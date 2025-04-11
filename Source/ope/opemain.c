/*[]----------------------------------------------------------------------[]*/
/*| operation main control                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-02-01                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
#include	<stddef.h>
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
#include	<stdio.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"pri_def.h"
#include	"tbl_rkn.h"
#include	"cnm_def.h"
#include	"flp_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"L_FLASHROM.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"mdl_def.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"prm_tbl.h"
#include	"mif.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"ope_ifm.h"
#include	"ifm_ctrl.h"

#include	"AppServ.h"
#include	"remote_dl.h"
#include	"I2c_driver.h"
#include	"rtc_readwrite.h"

#include	"lcd_def.h"
#include	"ntnetauto.h"
#include	"cre_ctrl.h"

// GG124100(S) R.Endo 2022/06/13 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/13 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
#include	"pkt_def.h"
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
#include	"aes_sub.h"
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
#include	"pktctrl.h"

extern uchar Is_in_lagtim( void );
extern short OnlineDiscount(void);
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
extern short GetTypeSwitch(ushort syu, ushort card_no);
extern short CheckDiscount(ushort syu, ushort card_no, ushort info);
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
extern BOOL IsDupSyubetuwari(void);
// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// extern void CopyDiscountCalcInfoToZaisha(void);
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

extern void Test_Ope_EnterLog(void);

uchar ryodsp;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//t_Edy_Dsp_erea	edy_dsp;					// ��ʕ\������p
//ulong	Edy_Pay_Work;						// ���Z�����i�[�p
//uchar	Dsp_Prm_Setting;					// �d�q�}�̎g�p�ݒ���
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
 
t_End_Status	cansel_status;				// ���Z���~�pEdy�ESuica�Ecmn�I���҂�
t_End_Status	CCT_Cansel_Status;			// �ڼޯĶ��ގg�p����Edy�ESuica��~�҂��p

char	TekKoshinCRE_flg;		//����X�V�ł̃N���W�b�g���Z�������t���O

char	TekKoshinCRE_faz;		// �X�V���Ӱ�ނł̸ڼޯĐ��Z�����׸�
								// 0= �ʏ�(F3��������Ȃ� or Suica�Ȃ��ݒ�)
								// 1= F3��������Suica��~�҂����
								// 2= Suica��~��ɸڼޯĐ��Z�ֈڍs������
uchar	Ope_Last_Use_Card;					// ���̃J�[�h�̂����Ō�Ɏg�p��������
											// 1=�v���y�C2=�񐔌��C3=�d�q���σJ�[�h
											// 0=��L�܂����g�p

short	tim1_mov;
static void	op_Cycle_BUZPIPI( void );
static void	op_IccLedOff( void );
static void	op_IccLedOff2( void );
static void	op_IccLedOnOff( void );
static void op_RestoreErrDsp( uchar dsp );

static void		SyuseiDataSet( ushort mot_lkno );
static short	SyuseiModosiPay( short rt );

char	First_Pay_Dsp( void );
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//uchar	FurikaeCancelFlg;		// �U�֐��Z���s�t���O
//uchar	FurikaeMotoSts;			// �U�֌���ԃt���O 0:�`�F�b�N�s�v / 1:�Ԃ��� / 2:�ԂȂ�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
uchar	MifStat;			// Mifare�������

uchar	Syusei_Select_Flg;
uchar	CreditReqResult;				// �N���W�b�g�J�[�h���Z�₢���킹���ʏ��
uchar CardStackStatus;			// �J�[�h�l�܂菈�����
static ulong EjectSafetyTime;			// �J�[�h�A���r�o�}�~�p
static uchar EjectActionCnt;			// �r�o�����
static uchar OutputRelease;				// �J�[�h�l�܂�����o��(0:�o�͂���/!0:�o�͂Ȃ�)
void StackCardEject(uchar req);			// �J�[�h�l�܂莞�̏����֐�
#define CARD_EJECT_AUTOLAG		(ushort)((prm_get(COM_PRM, S_SYS, 14, 2, 3) * 50) + 1)
#define CARD_EJECT_MANULAG		(ulong)((prm_get(COM_PRM, S_SYS, 14, 2, 5) * 100) + 1)
#define IS_ERR_RED_STACK		((ERR_CHK[ERRMDL_READER][ERR_RED_STACK_R]) || (ERR_CHK[ERRMDL_READER][ERR_RED_STACK_P]))
uchar	CardStackRetry;					// �r�o���쒆�׸�(���C���[�_����p)
										// �r�o���쒆�̓��[�h�R�}���h�𓊂��Ȃ�
static  long	decode_number=0;
static  long	op_Calc_Notice( ushort stimer, struct clk_rec *pDate, long sParkNo );
static  long	op_Calc_Ansyou( long lNoticeNo );
static	long	op_Calc_BekiJyo( long Val1, long Val2 );
static	void	teninb_pass2( ushort nu, ushort in_k, ushort lin, ushort col, uchar pas_cnt);
#define ONE_CHARACTER_CHECK(x,y) (((x/y)>10) ? ((x/y)%10):(x/y))	/* ��r�p�Ɉꌅ�ɕ�������}�N�� */
																	/* ��P�����ɈÏ؁E�o�ɔԍ����w�肵�A��Q�����ɕ���l���w�� */


#define SERCRET_MODE_OFF	0000
#define	DSP_DEFPOS	9
#define	GETA_10_6	1000000L
#define	GETA_10_5	100000L
#define	GETA_10_4	10000L
#define	GETA_10_3	1000L
#define	GETA_10_2	100L
#define	GETA_10_1	10L
#define	MAXVAL		9999L
#define	ZeroToMitsu	3L
char	shomei_errnum;
#define KEEP_EVT_MAX	10
ushort	Keep_Evt_Buf[KEEP_EVT_MAX];

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//void	Key_Event_Set( ushort msg );
//ushort	Key_Event_Get( void );
//ushort	key_event_keep[4];
//
//ushort	dspErrMsg;
//
//static	uchar	f_al_card_wait;			// 1=���C�J�[�h��������Aal_card() ���{�O�ɓd�q�}�l�[���[�_�Ȃǂ̒�~�҂������Ă���
// MH810100(S) 2020/09/09 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
//static struct clk_rec	PayStartTime;			// ���Z�J�n������(���Z���J�n���ꂽ���̌��ݎ���)
// MH810100(E) 2020/09/09 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
static DATE_YMDHMS		PayTargetTime;			// ���Z����(���Z�J�n�����B�������Z�ł͏o�Ɏ����ƂȂ�)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))

static	uchar	f_MandetMask;			// �l�̌��m�}�X�N�F0=�Ȃ� / 1=����
#define	TIMER_MANDETMASK	27			// �l�̌��m�}�X�N�^�C�}�[No.
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
uchar	coin_err_flg = 0;				// �d�ݎg�p�s���׸�
uchar	note_err_flg = 0;				// �����g�p�s���׸�
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH810105(S) MH364301 �C���{�C�X�Ή�
uchar	f_reci_ana;						// ������̗̎��؃{�^������
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
short	jnl_pri_wait_Timer;				// �W���[�i���󎚊����҂��^�C�}�[
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
ushort 	rpt_pri_wait_Timer;				// ���V�[�g�󎚊����҂��^�C�}�[
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j

extern	enum I2cResultFnc_e 	I2C_Request(I2C_REQUEST *pRequest, unsigned char ExeMode);

static	void	ryo_cal_sim(void);
static	void	init_ryocalsim(void);
static	void	set_tim_ryocalsim(ushort num, struct clk_rec *indate, struct clk_rec *outdate);
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή�
static void		op_mod01_Init( void );
static	int		Ope_PipRemoteCalcTimePre( void );
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//static	int		Ope_PipFurikaeCalc( void );
static	void	lcdbm_notice_opn( uchar	opn_cls,  uchar	opn_cls_reason );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή�
extern	uchar	SysMnt_Work[];			// ������� workarea (64KB)
// MH322914(S) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
short	IsMagReaderRunning();
// MH322914(E) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
// MH810100(S) K.Onodera 2019/12/27 �Ԕԃ`�P�b�g���X�i���ZID�Ή��j
static void op_SetSequenceID( ulong id );
static ulong op_GetSequenceID( void );
static void op_ClearSequenceID( void );
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)
//static BOOL isSeasonCardType( stMasterInfo_t* pstMasterInfo );
static BOOL isSeasonCardType( stMasterInfo_t* pstMasterInfo, stZaishaInfo_t* pstZaishaInfo );
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)

static eSEASON_CHK_RESULT CheckSeasonCardData( lcdbm_rsp_in_car_info_t* pstCardData );
static eSEASON_CHK_RESULT CheckSeasonCardData_main( lcdbm_rsp_in_car_info_t* pstCardData, ushort no );
static void SetPayTargetTime( DATE_YMDHMS *pTime );
static DATE_YMDHMS* GetPayTargetTime( void );
static short set_tim_only_out_card( short err_mode );
// MH810100(E) K.Onodera 2019/12/27 �Ԕԃ`�P�b�g���X�i���ZID�Ή��j
// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
static void SetPayStartTimeInCarInfo();
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// static short in_time_set();
static short in_time_set(uchar firstFlg);
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// MH810100(S) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j
void SetQRMediabyQueryResult();
// MH810100(E) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j

// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
extern void LongTermParkingRel( ulong LockNo , uchar knd, flp_com *flp);
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//extern	char	pcard_shtter_ctl;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH810100(S) Y.Yamauchi 2020/02/28 �Ԕԃ`�P�b�g���X(�����e�i���X)
extern	uchar	Anounce_volume	( uchar volume );
extern	uchar	Key_volume ( uchar volume, uchar kind);
// MH810100(E) Y.Yamauchi 2020/02/28 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#if (6 == AUTO_PAYMENT_PROGRAM)
static	ushort	auto_payment_locksts_set( void );
static	short	car_index = INT_CAR_START_INDEX;
static	uchar	is_auto_pay = 0;
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
static	void	lcd_Recv_FailureContactData_edit(void);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 �C���{�C�X�Ή�
static	void	ryo_reisu(void);
static 	void	receipt_output_error(void);
static	void	statusChange_DispUpdate(void);
// MH810105(E) MH364301 �C���{�C�X�Ή�

// MH810100(S) �e�X�g�p���Ƀ��O�쐬
#ifdef TEST_ENTER_LOG
void Test_Ope_EnterLog(void);
#endif
// MH810100(E) �e�X�g�p���Ƀ��O�쐬
// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
uchar autopay_coin_cnt = 0;

#define AP_CHARGE_INTERVAL_SEC		10		// �R�C�������Ԋu�b
#define AP_PARKING_FEE			   	200		// ���ԗ���
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z

// MH810101(S) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
static uchar season_chk_result;	// ����`�F�b�N����
// MH810101(E) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
static uchar cmp_send;
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
struct	stLaneDataInfo	m_stLaneWork;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102���p�j�iGM803003���p�j
uchar r_zero_call;							// 0�~���Z�ɂ��\���Z�k�t���O
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102���p�j�iGM803003���p�j

/*[]----------------------------------------------------------------------[]*/
/*| �ʏ퐸�Z                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpeMain( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpeMain( void )
{
	short	ret;

	ac_flg.cycl_fg = 0;												// 0:�ҋ@���
	if( 0 != OPECTL.Kakari_Num ){									// ����ݽ��ʂ���̖߂�
		if( OPECTL.Pay_mod != 2 ){									// �C�����Z�ȊO
		wopelg( OPLOG_MNT_END, 0L, (ulong)OPECTL.Kakari_Num );		// �W������I������۸ޓo�^
		OPECTL.Kakari_Num = 0;										// �W�����ر
		}
	}

	rd_shutter();													// ���Cذ�ް���������

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//	if(( CP_MODECHG == 0 )&&										// Mode change key ON?
//	   ( OPECTL.Pay_mod == 0 )){									// �ʏ퐸�Z
//		OPECTL.Mnt_mod = 1;											// ����ݽӰ�ޑI��������
//		Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//		return;
//	}
//
//	if( CP_MODECHG == 1 ){											// Mode change key OFF?
//		OPECTL.Pay_mod = 0;											// �ʏ퐸�Z
//	}
//
//	if( DoorCloseChk() >= 100 ){									// �ޱ���̓�����������
//		if( OPECTL.Pay_mod == 2 ){									// �C�����Z����?
//		}else{
//			OPECTL.Ope_mod = 0;										// �ҋ@�ɖ߂�
//			if( opncls() == 2 ){									// �x��?
//				OPECTL.Ope_mod = 100;								// �x�Ə�����
//			}
//		}
//	}else{
//		OPECTL.Ope_mod = 110;										// �ޱ���װѕ\��������
//	}
	// PowerOn�N�������ޱ�J��ԂɍS��炸 op_init00()�ֈڍs����
	if (OPECTL.Ope_mod != 255) {
		// �����e�L�[ON�H
		if(( CP_MODECHG == 0 )&&										// Mode change key ON?
		   ( OPECTL.Pay_mod == 0 )){									// �ʏ퐸�Z
			OPECTL.Mnt_mod = 1;											// ����ݽӰ�ޑI��������
		   	ret = -1;
		}
		// �����e�L�[OFF�H
		if( CP_MODECHG == 1 ){											// Mode change key OFF?
			OPECTL.Pay_mod = 0;											// �ʏ퐸�Z
		}
		if( DoorCloseChk() >= 100 ){									// �ޱ���̓�����������
			if( OPECTL.Pay_mod == 2 ){									// �C�����Z����?
			}else{
				if( opncls() == 2 ){									// �x��?
					OPECTL.Ope_mod = 100;								// �x�Ə�����
					// �O��c�Ƃő��M�ς݁H
					if( OPECTL.sended_opcls != 1 ){
						lcdbm_notice_opn( 1, (uchar)CLOSE_stat );		// �x�ƒʒm
						OPECTL.sended_opcls = 1;
					}
				}else{
					OPECTL.Ope_mod = 0;									// �ҋ@�ɖ߂�
					// �O��x�Ƃő��M�ς݁H
					if( OPECTL.sended_opcls != 0 ){
						lcdbm_notice_opn( 0, (uchar)CLOSE_stat );		// �c�ƒʒm
						OPECTL.sended_opcls = 0;
					}
				}

			}
		}else{
			OPECTL.Ope_mod = 110;										// �ޱ���װѕ\��������
		}
	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))

	knum_len = binlen( (ulong)CPrmSS[S_TYP][82] );					// ���тŎg�p���钓�Ԉʒu�̌���
	if( knum_len == 1 ){											// 1���̏ꍇ
		knum_ket = 1;
	}else if( knum_len == 2 ){										// 2���̏ꍇ
		knum_ket = 10;
	}else if( knum_len == 3 ){										// 3���̏ꍇ
		knum_ket = 100;
	}else{															// 4���ȏ�̏ꍇ
		knum_ket = 1000;
		knum_len = 4;
	}
	LCDNO = (ushort)-1;
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
	if( isEJA_USE() ){												// �d�q�W���[�i���ڑ�
		jnl_pri_wait_Timer = EJNL_PRI_WAIT_TIME;
	}
	else{
		jnl_pri_wait_Timer = JNL_PRI_WAIT_TIME;
	}
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
	Ec_check_PendingJvmaReset();
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
	// �ۗ����Ă����u�����h�l�S�V�G�[�V�������ĊJ����
	Ec_check_PendingBrandNego();
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X�iOPE�Ή��j
//	for( ret = 0; ret != -1; ){
	for( ret = 0; ; ){
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X�iOPE�Ή��j
		if( ret == 10 ){ 											// ���~?
// MH810100(S) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(���Z���~����������)
			if( 255 != OPECTL.Ope_mod ){	// ���Z���~����̋N���ҋ@�J�ڂł͂Ȃ�
// MH810100(E) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(���Z���~����������)
				OPECTL.Ope_mod = 0;									// �ҋ@�ɖ߂�
				ac_flg.cycl_fg = 0;									// 0:�ҋ@���
				OPECTL.nyukin_flg = 0;								// �����L���׸�OFF
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
				Ec_check_PendingJvmaReset();
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
				// �ۗ����Ă����u�����h�l�S�V�G�[�V�������ĊJ����
				Ec_check_PendingBrandNego();
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
				if( CP_MODECHG == 0 ){								// Mode change key ON?
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X�iOPE�Ή��j
//					if( OPECTL.Pay_mod == 0 ){						// �ʏ퐸�Z?
//						OPECTL.Mnt_mod = 1;							// ����ݽӰ�ޑI��������
//						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//						return;
//					}else if( OPECTL.Pay_mod == 2 ){				// �C�����Z?
//					}
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X�iOPE�Ή��j
				}else{
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
					OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}
				StackCardEject(1);										/* ���Z���~���̔r�o���� */
// MH810100(S) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(���Z���~����������)
			}
// MH810100(E) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(���Z���~����������)
		}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//		if( 0 == OPECTL.Ope_mod ){									// ���͑ҋ@
//			if( opncls() == 2 ){									// �x��?
//				OPECTL.Ope_mod = 100;								// �x�Ə�����
//			}
//		}
		if( 0 == OPECTL.Ope_mod ) {
			if( opncls() == 2 ){									// �x��?
				OPECTL.Ope_mod = 100;								// �x�Ə�����
				// �c�x�ƒʒm_�c�x��_�x�Ɨ��R�̑��M����
				lcdbm_notice_opn( 1, (uchar)CLOSE_stat );			// �x�ƒʒm
				OPECTL.sended_opcls = 1;
			}
		}
		else if( 100 == OPECTL.Ope_mod ) {
			if( opncls() == 1 ){									// �c��?
				OPECTL.Ope_mod = 0;									// �x�Ə�����
				// �c�x�ƒʒm_�c�x��_�x�Ɨ��R�̑��M����
				lcdbm_notice_opn( 0, (uchar)CLOSE_stat );			// �c�ƒʒm
				OPECTL.sended_opcls = 0;
			}
		}
		else if(( 255 == OPECTL.Ope_mod ) && ( ret != 10 ) && ( OPECTL.init_sts == 1 )){
			// ���Z���~����̋N���ҋ@�J�ڂł͂Ȃ��������������ς�
			if( opncls() == 1 ){									// �c��?
				OPECTL.Ope_mod = 0;									// �x�Ə�����
				// �c�x�ƒʒm_�c�x��_�x�Ɨ��R�̑��M����
				lcdbm_notice_opn( 0, (uchar)CLOSE_stat );			// �c�ƒʒm
				OPECTL.sended_opcls = 0;
			}
			else {													// �x��?(2)
				OPECTL.Ope_mod = 100;								// �x�Ə�����
				// �c�x�ƒʒm_�c�x��_�x�Ɨ��R�̑��M����
				lcdbm_notice_opn( 1, (uchar)CLOSE_stat );			// �x�ƒʒm
				OPECTL.sended_opcls = 1;
			}
		}

		// �����e�ڍs�������H
		if( OPECTL.Mnt_mod == 1 ){
			// �����e�J�n�����҂� => OK��M/���Z�J�n�v���Ń����e��
			if( MOD_CHG == op_wait_mnt_start() ){
				// �����e�i���X��
				Ope_KeyRepeatEnable( OPECTL.Mnt_mod );
				break;
			}else{
				// OPE��
				OPECTL.Pay_mod = 0;									// �ʏ퐸�Z
				OPECTL.Mnt_mod = 0;									// ���ڰ���Ӱ�ޏ�����
				Ope_KeyRepeatEnable( OPECTL.Mnt_mod );
			}
		}
		// �����e�ɂ͈ڍs���Ȃ���������ꍇ
		else{
			// op_init00()�̎����ޱ�J�������͍s��Ȃ�
			if( ret == -1 && OPECTL.init_sts == 1 ){
				if( CP_MODECHG == 0 ){								// Mode change key ON?
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
					continue;
				}else{
					// OPE��
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
					OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
					Ope_KeyRepeatEnable( OPECTL.Mnt_mod );
				}
				return;
			}
		}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		if( Suica_Rec.Data.BIT.LOG_DATA_SET == 1 &&					// ���ό����ް���M�ς݌��
		  ( OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100 || 		// �ҋ@���x�Ƃ�
		  	OPECTL.Ope_mod == 11 || OPECTL.Ope_mod == 21 )){		// �C�����Z���Ԉʒu�ԍ����͂�����X�V�I����ʕ\������
// MH321800(S) hosoda IC�N���W�b�g�Ή�
//			Settlement_rcv_faze_err((uchar*)&Settlement_Res, 4 );	// �G���[�o�^����
			if (isEC_USE()) {
				EcSettlementPhaseError((uchar*)&Ec_Settlement_Res, 4 ); // �G���[�o�^����
			} else {
				Settlement_rcv_faze_err((uchar*)&Settlement_Res, 4 );	// �G���[�o�^����
			}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
		}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//		if( OPECTL.Ope_mod == 100 || OPECTL.Ope_mod == 110 ){		// ���̉�ʑJ�ڂ��x�ƁE�x��A���[���̏ꍇ�́A�ێ����Ă���Ԏ��ԍ����J��
//			Key_Event_Get();										// ���̑��͑ҋ@���o�R����̂őҋ@�ŏ�������
//		}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))

		if (OPECTL.Ope_mod == 0 ||
			OPECTL.Ope_mod == 100) {
			TimeAdjustCtrl(2);			// ���������␳��~����
		}
		else {
			TimeAdjustCtrl(1);			// ���������␳��~
		}
		switch( OPECTL.Ope_mod ){
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			case 255:
				// ����ڽ�N���ʒm/�N�������ʒm��M
				ret = op_init00();
				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			case 0:
				ret = op_mod00();									// �ҋ@
				break;
			case 1:
				ret = op_mod01();									// ���Ԉʒu�ԍ����͏���
				break;
			case 2:
				NTNET_ClrSetup();
				ret = op_mod02();									// �����\��,��������
				break;
			case 3:
				ret = op_mod03();									// ���Z��������
				break;
			case 11:
				ret = op_mod11();									// �C�����Z���Ԉʒu�ԍ����͏���
				break;
			case 12:
				ret = op_mod12();									// �C�����Z�m�F�����i���F�C�����Z�����\���j
				break;
#if SYUSEI_PAYMENT
			case 13:
				ret = op_mod13();									// �C�����Z��������
				break;
			case 14:
				ret = op_mod14();									// �C�����Z��������
				break;
#endif		// SYUSEI_PAYMENT

#if	UPDATE_A_PASS
			case 21:
				ret = op_mod21();									// ������X�V ���}���҂�����
				break;
			case 22:
				ret = op_mod22();									// ������X�V �����\��,��������
				break;
			case 23:
				ret = op_mod23();									// ������X�V ���Z��������
				break;
#endif	// UPDATE_A_PASS
			case 70:
				ret = op_mod70();									// �߽ܰ�ޓ��͏���(���Z��)
				break;
			case 80:
				ret = op_mod80();									// �߽ܰ�ޓo�^����(���Ɏ�)
				break;
			case 90:
				ret = op_mod90();									// ��t�����s����
				break;
			case 100:
				ret = op_mod100();									// �x�Ə���
				break;
			case 110:
				ret = op_mod110();									// �ޱ���װѕ\������
				break;
			case 200:
				ret = op_mod200();									// �Ïؔԍ�(Btype)�o�ɑ���
				break;
			case 210:
				ret = op_mod210();									// �Ïؔԍ�(Btype)���͑���
				break;
			case 220:
				ret = op_mod220();									// ���O�^�C����������
				break;
			case 230:
				ret = op_mod230();									// ���Z�ς݈ē�����
				break;
			case 81:
			case 91:
			case 92:
				ret = op_mod81(OPECTL.Ope_mod);						// ��t�A�߽ܰ�ރG���[�\��
				break;
			default:
				ret = 10;
				break;
		}
	}
}

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//[]----------------------------------------------------------------------[]
///	@brief			LCDӼޭ�قɑ΂������Ұ�����۰�ނ��s��
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			result : ���s����
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
int lcdbm_setting_upload_FTP( void )
{
	int ret;
	int result = 1;	// 1=NG
	int retry = 0;

#if GT4100_NO_LCD
	return(OK);
#endif

// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
// GM760201(S) �Z���^�[�A�b�v���[�h�Ƃ̔r�������̉��P
//	if( TASK_START_FOR_DL() ){
	if( DOWNLOADING() ){
// GM760201(E) �Z���^�[�A�b�v���[�h�Ƃ̔r�������̉��P
		return 2;
	}
	OPECTL.lcd_prm_update = 1;	// LCD�ւ̃p�����[�^�A�b�v���[�h��
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)

	// original�܂߂�Max 4��s��
	for( retry = 0; retry < 4; retry++ ) {
		/****************************************************************/
		/* LCDӼޭ�قɑ΂������Ұ�����۰��(ftpctrl.result�̒l��Ԃ�)	*/
		/* 		while( ftpctrl.start )��loop	( FTP_IDLE == 0 )		*/
		/* 		TIMEOUT25 = FTP_Execute()�������s						*/
		/* 		TIMEOUT19 = ftp_send_quit(0)�������s					*/
		/****************************************************************/
		ret = ftp_auto_update_for_LCD();
				//	AU_DWL_SCR 			0
				//	AU_CONFIRM			1
				//	AU_EXEC				2
				//	AU_NORMAL_END		10
				//	AU_ABORT_END		11
				//	AU_NO_UPDATE		12
				//	AU_CANCEL			13
				//	AU_DISCONNECT		14
				//	AU_LOGIN_ERR		15
				//	AU_LOGOUT_ERR		16

		ret &= ~0x8000;
		// �����ްĂ͐���I���H
		if( ret == 10 ){	// AU_NORMAL_END
			result = 0;		// 0=OK
			break;
		}
	}
// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
	OPECTL.lcd_prm_update = 0;	// LCD�ւ̃p�����[�^�A�b�v���[�h������
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)

	// NG�H
	if( result ){
		// �G���[�o�^
		err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_PRM_UPLD_FL, 1, 2, 0, &ret );
	}

	return(result);
}

// MH810100(S) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(�ҋ@��ԑJ�ڏ������ʉ�)
//[]----------------------------------------------------------------------[]
///	@brief			�ҋ@��ԑJ�ڋ��ʏ���
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@note			�N�������ʒm����M���đҋ@��Ԃɖ߂�ۂ̋��ʏ���
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void	ope_idle_transit_common( void )
{
	uchar	result_PKTcmd;
// MH810100(S) Y.Yamauchi #3965 �u�U�[���ʂ��������Ȃ�Ȃ�
	uchar	volume;	// ����
// MH810100(E) Y.Yamauchi #3965 �u�U�[���ʂ��������Ȃ�Ȃ�

	// LCD�P�x�ݒ�	�P�x�l(0�`15) 0=�ޯ�ײ�OFF �i02-0028�D�E = �������̋P�x(�ҋ@��)�j
// MH810100(S) Y.Yamauchi #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�
	LcdBackLightCtrl(ON);
// MH810100(E) Y.Yamauchi #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�

	// �޻ް���ʐݒ�	���� = 0�`3(0=����,3=�ő剹��)
	LcdSend.lcdbm_BUZ_volume = (uchar)prm_get(COM_PRM, S_PAY, 29, 1, 2);	// 02-0029�D = 0=���Ȃ��^1=��^2=���^3=��
	volume = Key_volume( LcdSend.lcdbm_audio_volume, 1);					// �L�[����
	result_PKTcmd = PKTcmd_beep_volume( volume );							// �u�U�[�v���i�ݒ�j
	if (result_PKTcmd == FALSE) {
		// error
	}

	// �ųݽ����		0�`100(0=����)
	LcdSend.lcdbm_audio_volume = (ushort)prm_get(COM_PRM, S_SYS, 51, 2, 5);	// 01-0051	���ʐݒ�(ch0)	�@�A:���������1  01�`15/00=����
																			//							�B�C:���������2  01�`15/00=����
																			//							�D�E:���������3  01�`15/00=����	080000
// MH810100(S) Y.Yamauchi #3965 �u�U�[���ʂ��������Ȃ�Ȃ�
	volume = Anounce_volume( LcdSend.lcdbm_audio_volume );					// �A�i�E���X���ʎ擾
	result_PKTcmd = PKTcmd_audio_volume( volume );							// �A�i�E���X���ʂ�LCD�ɑ��M
// MH810100(E) Y.Yamauchi #3965 �u�U�[���ʂ��������Ȃ�Ȃ�

	// ���v
	PKTcmd_clock( 0 );														// RXM�̎��v�̒l�𑗐M
	if (result_PKTcmd == FALSE) {
		// error
	}

	// �N�������ʒm��M���̋N��������ԍX�V
	PktStartUpStsChg( 2 );
}

// MH810100(E) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(�ҋ@��ԑJ�ڏ������ʉ�)
/*[]----------------------------------------------------------------------[]*/
/*| ����ڽ�p�N���ҋ@                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_init00( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = �������֐؊�                              |*/
/*|                       : -1 = Ӱ�ސؑ�                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2019-11-15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short	op_init00( void )
{
	short	ret;
	ushort	msg = 0;
	int		result;
// MH810100(S) K.Onodera  2020/02/21 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
	ushort	sta_tim;
// MH810100(E) K.Onodera  2020/02/21 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
	uchar	req_ftp = OFF;
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)

// MH810100(S) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
	// LCD�Ƃ̐ڑ����؂ꂽ�̂ŁA�ێ����Ă��鐸�Z��Ԃ��N���A���Ă���
	clear_pay_status();
// MH810100(E) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)

// MH810100(S) K.Onodera  2020/03/23 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
	if(( OPECTL.init_sts == 0xff ) || ( OPECTL.init_sts == 0xfe )){	// �N���ʒm(0/1)��M��H
		if( OPECTL.init_sts == 0xff ){	// �N���ʒm(0)��M��H
			// �ݒ�A�b�v���[�h����(FTP)
			result = lcdbm_setting_upload_FTP();
// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
			if( result == 2 ){	// ���u�_�E�����[�h���H
				Lagtim( OPETCBNO, 1, 10*50 );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
			}else{
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
				// �ݒ�A�b�v���[�h�ʒm
				lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)result );
// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
			}
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
		}
// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
//		OPECTL.init_sts = 1;
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:221)�Ή�
		else {	// �N���ʒm(1)��M��H
			result = 0;
		}
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:221)�Ή�
		if( result != 2 ){
			OPECTL.init_sts = 1;
		}
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
	}
// GG120600(S) // Phase9 #6185 �ݒ�A�b�v���[�h���s�������ɃV�X�e���ؑւŋ��ʃp�����[�^�ύX����ƁA�u [�ݒ�A�b�v���[�h�@�\]-[�����J�n���茋��]-[����J�n](7000100)�v����������
	// GetMessage�O�Ɏ��{�i���i�����삷�邱�Ƃ�����̂Łj
	if(mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE){
		// parkingWeb�ڑ�����
		if(_is_ntnet_remote()) {
			// �[���Őݒ�p�����[�^�ύX
			rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
			// �[�����Őݒ肪�X�V���ꂽ���߁A�\�񂪓����Ă�����L�����Z�����s��
			remotedl_cancel_setting();
		}
		mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
	}
// GG120600(E) // Phase9 #6185 �ݒ�A�b�v���[�h���s�������ɃV�X�e���ؑւŋ��ʃp�����[�^�ύX����ƁA�u [�ݒ�A�b�v���[�h�@�\]-[�����J�n���茋��]-[����J�n](7000100)�v����������

	sta_tim = (ushort)prm_get( COM_PRM, S_PKT, 26, 3, 1 );
	if(( sta_tim < 1 ) || ( 999 < sta_tim )){
		sta_tim = 180;
	}
	// LCD�N���ʒm/�N�������ʒm�҂��^�C�}�J�n
	Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
// MH810100(E) K.Onodera  2020/03/23 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)

	// ret != 0 �܂Ŗ���loop
	for( ret = 0; ret == 0; ){
		msg = GetMessage();

		switch( msg ){

			// ����ʒm
			case LCD_OPERATION_NOTICE:
				// ����R�[�h
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {

					// �N���ʒm
					case LCDBM_OPCD_STA_NOT:
						// �N��Ӱ�� = �ݒ豯��۰�ދN��
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
//							// �ݒ�A�b�v���[�h����(FTP)
//							result = lcdbm_setting_upload_FTP();
//
//							// �ݒ�A�b�v���[�h�ʒm
//							lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)result );
//
//							OPECTL.init_sts = 1;
							req_ftp = ON;
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
						}
						// �N��Ӱ�� = �ʏ�N��
						else {
							OPECTL.init_sts = 1;	// �p�����[�^�A�b�v���[�h�s�v�Ȃ̂ŏ����������Ƃ���
// MH810100(S) K.Onodera  2020/02/21 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
							// LCD�N���ʒm/�N�������ʒm�҂��^�C�}�ĊJ
							Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
// MH810100(E) K.Onodera  2020/02/21 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
						}
						break;

					// �N�������ʒm
					case LCDBM_OPCD_STA_CMP_NOT:
// MH810100(S) K.Onodera  2020/03/13 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
						// LCD�N���ʒm/�N�������ʒm�҂��^�C�}��~
						Lagcan( OPETCBNO, TIMERNO_LCD_STA_WAIT );
						ope_idle_transit_common();	// �ҋ@��ԑJ�ڋ��ʏ���
						OPECTL.init_sts = 1;		// ����������
						ret = -1;
// MH810100(E) K.Onodera  2020/03/13 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
						break;
				}
				break;

// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
			// �ݒ�A�b�v���[�h�҂����킹�^�C���A�E�g
			case TIMEOUT1:
				req_ftp = ON;
				break;
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
// MH810100(S) K.Onodera  2020/02/21 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
				// LCD�N���ʒm/�N�������ʒm�҂��^�C�}�ĊJ
				Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
// MH810100(E) K.Onodera  2020/02/21 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
				OPECTL.init_sts = 0;	// �N���ʒm����҂�
				break;

// MH810100(S) K.Onodera  2020/02/21 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
			case TIMEOUT_LCD_STA_WAIT:	// LCD�N���ʒm/�N�������ʒm�҂��^�C�}�^�C���A�E�g
				PktResetReqFlgSet( 1 );	// LCD���Z�b�g�ʒm�v���v�ۃZ�b�g
				OPECTL.init_sts = 0;	// �N���ʒm����҂�
				break;

// MH810100(E) K.Onodera  2020/02/21 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
			default:
				break;
		}	// switch

// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
		if( req_ftp == ON ){
			req_ftp = OFF;

			Lagcan( OPETCBNO, TIMERNO_LCD_STA_WAIT );

			// �ݒ�A�b�v���[�h����(FTP)
			result = lcdbm_setting_upload_FTP();

			// ���u�_�E�����[�h���H
			if( result == 2 ){
				Lagtim( OPETCBNO, 1, 10*50 );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
				// LCD�N���ʒm/�N�������ʒm�҂��^�C�}���Z�b�g
				Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
			}
			// FTP���{
			else{
				// �ݒ�A�b�v���[�h�ʒm
				lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)result );

				OPECTL.init_sts = 1;
			}
			// LCD�N���ʒm/�N�������ʒm�҂��^�C�}�ĊJ
			Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
		}
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)

	}	// for( ret = 0; ret == 0; ){

// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
	Lagcan( OPETCBNO, 1 );						// ��ϰ1ؾ��(���ڰ��ݐ���p)
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
	Lagcan( OPETCBNO, TIMERNO_LCD_STA_WAIT );	// LCD�N���ʒm/�N�������ʒm�҂��^�C�}���Z�b�g
// GG120600(S) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
	if( DOWNLOADING() ){	
		// task_status�ɂ����f
		remotedl_status_set(remotedl_status_get());
	}
// GG120600(E) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�

	return( ret );
}

//[]----------------------------------------------------------------------[]
///	@brief			����ʒm���M
//[]----------------------------------------------------------------------[]
///	@param			ope_code�F����R�[�h
///					status	�F���
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_notice_ope( eOPE_NOTIF_CODE ope_code, ushort status )
{
	uchar	result_PKTcmd;

	LcdSend.lcdbm_cmd_notice_ope.ope_code 	= (uchar)ope_code;	// ����R�[�h
	LcdSend.lcdbm_cmd_notice_ope.status 	= status;			// ���

	// ����ʒm���M
	result_PKTcmd = PKTcmd_notice_ope( LcdSend.lcdbm_cmd_notice_ope.ope_code, LcdSend.lcdbm_cmd_notice_ope.status );
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			QR�ް�����(OK))���M
//[]----------------------------------------------------------------------[]
///	@param			result�F����(0:OK,1:NG(�r��),2:NG(�������))
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//	typedef	struct {
//		lcdbm_cmd_base_t		command;		///< ����ދ��ʏ��
//		unsigned long			id;				///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
//		unsigned char			result;			///< ����(0:OK,1:NG(�r��),2:NG(�������))
//		unsigned char			reserve;		///< �\��
//	} lcdbm_cmd_QR_data_res_t;
void	lcdbm_QR_data_res( uchar result )
{
	uchar	result_PKTcmd;

	// lcdbm_cmd_QR_data_res_t
	result_PKTcmd = PKTcmd_QR_data_res(
								op_GetSequenceID(),		// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
								result );				// ����(0:OK,1:NG(�r��),2:NG(�������))
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�U�[���v������
//[]----------------------------------------------------------------------[]
///	@param			beep	: 0=�߯���1=���ߨ���2=�����ߨ��
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void lcdbm_beep_start( uchar beep )
{
	uchar	result_PKTcmd;

	// �ʏ탂�[�h�H
	if( OPECTL.Mnt_mod == 0 ){
		return;		// �����e�i���X�ȊO�́ALCD���g�Ŗ炷
	}

	// ��� = 0=�߯���1=���ߨ���2=�����ߨ��
	LcdSend.lcdbm_BUZ_beep = beep;
	result_PKTcmd = PKTcmd_beep_start( LcdSend.lcdbm_BUZ_beep);
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			 �G���[�ʒm��M���̃G���[log�o�^����
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810100(S) S.Fujii 2020/08/24 #4609 SD�J�[�h�G���[�Ή�
//const static uchar LCD_SEC_TBL[] = { 28, 30, 37, 0 };
const static uchar LCD_SEC_TBL[] = { 28, 30, 37, 31, 0 };
// MH810100(E) S.Fujii 2020/08/24 #4609 SD�J�[�h�G���[�Ή�

void lcdbm_ErrorReceiveProc( void )
{
	date_time_rec date;
	char kind = 0, sts = 0;
	ushort sec, cnt;
	lcdbm_rsp_error_t* rcv = &LcdRecv.lcdbm_rsp_error;

	// ������������
	date.Year = rcv->year;
	date.Mon  = rcv->month;
	date.Day  = rcv->day;
	date.Hour = rcv->hour;
	date.Min  = rcv->minute;

	// ���݂̃��A���^�C���ʐM��Ԃ��擾
	sts = ERR_CHK[mod_realtime][1];

	// �S�����H
	if( rcv->kind == 255 ){
		for( sec=0; LCD_SEC_TBL[sec]; sec++ ){
			for( cnt=1; cnt<ERR_NUM_MAX; cnt++ ){
				WACDOG;
				// �G���[����
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:222)�Ή�
//				err_chk3( (char)LCD_SEC_TBL[sec], (char)cnt, 0, 0, 0, 0, &date );
				err_chk3( (char)LCD_SEC_TBL[sec], (char)cnt, 0, 0, 0, 0, 0, &date );
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:222)�Ή�
			}
		}
	}
	// �ʏ�
	else{
		switch( rcv->occur ){
			case 1:	// ����
				kind = 1;
				break;
			case 2:	// ����
				kind = 0;
				break;
			case 3:	// ����
				kind = 2;
				break;
			default:
				return;	// �ُ�l
				break;
		}

		// �G���[�o�^
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:222)�Ή�
//		err_chk3( rcv->kind, rcv->code, kind, 1, 0, rcv->info, &date );
		err_chk3( rcv->kind, rcv->code, kind, 1, 0, rcv->info, sizeof(rcv->info), &date );
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:222)�Ή�
	}

// MH810103(s) �d�q�}�l�[�Ή� #5583 �y���؉ێw�E�����z�u�������ܐ��Z�ł��܂���v�\�����Ɏc���Ɖ�ł���
	// ���A���^�C���ʐM�ُ킪�����H
	if( sts == 0 && 1 == ERR_CHK[mod_realtime][1] ){
		LcdBackLightCtrl(ON);
	}
// MH810103(e) �d�q�}�l�[�Ή� #5583 �y���؉ێw�E�����z�u�������ܐ��Z�ł��܂���v�\�����Ɏc���Ɖ�ł���
	// ���A���^�C���ʐM�ُ킪�������ꂽ�H
	if( sts && 0 == ERR_CHK[mod_realtime][1] ){
// MH810103(s) �d�q�}�l�[�Ή� #5583 �y���؉ێw�E�����z�u�������ܐ��Z�ł��܂���v�\�����Ɏc���Ɖ�ł���
		LcdBackLightCtrl(ON);
// MH810103(e) �d�q�}�l�[�Ή� #5583 �y���؉ێw�E�����z�u�������ܐ��Z�ł��܂���v�\�����Ɏc���Ɖ�ł���
		// ���A���^�C�����Z�f�[�^�Ȃ��H
		if( 0 == Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) ){
			// �����f�[�^�ʒm(�S�f�[�^���M����)
			PKTcmd_notice_ope( (uchar)LCDBM_OPCD_RESTORE_NOTIFY, (ushort)0 );
		}
		// ���Z�f�[�^����H
		else{
			// �����f�[�^�����MON
			pkt_set_restore_unsent_flg(2);
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���Z��Ԓʒm���M����
//[]----------------------------------------------------------------------[]
///	@param			pay_status : ���Z���
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//	b0:		�ޑK�s��
//	b1:		ڼ�ėp���s��
//	b2:		�ެ��ٗp���s��
//	b3:		Suica���p�s��
//	b4:		�ڼޯė��p�s��
//	b5:		Edy���p�s��
//	b6 �`15:�\��
void	lcdbm_notice_pay( ulong pay_status )
{
	uchar	result_PKTcmd;

	// ���Z��Ԓʒm
	result_PKTcmd = PKTcmd_notice_pay( pay_status );
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�c�x�ƒʒm
//[]----------------------------------------------------------------------[]
///	@param			opn_cls : �c�x�ƒʒm	0=�c�� / 1=�x��<br>
/// 				opn_cls_reason : �x�Ɨ��R����
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// ���x�Ɨ��R���ށ�
//	LCDBM_OPNCLS_RSN_FORCED				// �����c�x��
//	LCDBM_OPNCLS_RSN_OUT_OF_HOURS		// �c�Ǝ��ԊO
//	LCDBM_OPNCLS_RSN_BANKNOTE_FULL		// �������ɖ��t
//	LCDBM_OPNCLS_RSN_COIN_SAFE_FULL		// ��݋��ɖ��t
//	LCDBM_OPNCLS_RSN_BANKNOTE_DROP		// ����ذ�ް�E��
//	LCDBM_OPNCLS_RSN_NO_CHANGE			// �ޑK�؂�
//	LCDBM_OPNCLS_RSN_COINMECH_COMM_FAIL	// ���ү��ʐM�s��
//	LCDBM_OPNCLS_RSN_SEND_BUFF_FULL		// ���M�ޯ̧��
//	LCDBM_OPNCLS_RSN_SIGNAL				// �M���ɂ��x��
//	LCDBM_OPNCLS_RSN_COMM				// �ʐM�ɂ��x��
static void	lcdbm_notice_opn( uchar	opn_cls, uchar	opn_cls_reason )
{
	uchar	result_PKTcmd;

	// �c�x�ƒʒm
// MH810103(s) �d�q�}�l�[�Ή� �c�x�ƒʒm�C��
//	LcdSend.lcdbm_opn_cls 			= opn_cls;
//	LcdSend.lcdbm_opn_cls_reason 	= opn_cls_reason;
//	result_PKTcmd = PKTcmd_notice_opn( LcdSend.lcdbm_opn_cls, LcdSend.lcdbm_opn_cls_reason );
	LcdSend.lcdbm_opn_cls.opn_cls = opn_cls;
	LcdSend.lcdbm_opn_cls.reason = opn_cls_reason;
	result_PKTcmd = PKTcmd_notice_opn( LcdSend.lcdbm_opn_cls.opn_cls, LcdSend.lcdbm_opn_cls.reason );
// MH810103(e) �d�q�}�l�[�Ή� �c�x�ƒʒm�C��
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�x���ʒm��Ԏ擾
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			ret_warning: �x���ʒm���
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//		b0:		��݋��ɖ����
//		b1:		��ݶ��Ė����
//		b2:		���ү����{��
//		b3:		����ذ�ް�W�J
//		b4:		����ذ�ް���{��
//		b5:		ڼ��������W�J
//		b6:		�ެ���������W�J
//		b7:		RSW��0�ȊO = read_rotsw()
//		b8:		CAN�ʐM�s��
//		b9:		�C�j�V�����N��
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
////		b10�`15:	�\��
//		b10:	SD�J�[�h���p�s��
//		b11:	���V�[�g�v�����^�ݒ�s��v
//		b12�`15:	�\��
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
ushort	lcdbm_alarm_check( void )
{
	ushort	ret_warning = 0x0000;

	// b0
	if( FNT_CN_BOX_SW ){									// ��݋��ɾ��?
		ret_warning |= 0x0001;
	}
	// b1
	if( CN_RDAT.r_dat0c[1] & 0x40 ){						// ��ݶ��ĊJ
		ret_warning |= 0x0002;
	}
	// b2
	if( FNT_CN_DRSW ){										// ���ү��ޱ�����J(���ү��ޱ���J)?
		ret_warning |= 0x0004;
	}
	// b3
	if( NT_RDAT.r_dat1b[0] & 0x02 ){						// ������J?
		ret_warning |= 0x0008;
	}
	// b4
	if( FNT_NT_BOX_SW ){									// �������ɒE�������J?
		ret_warning |= 0x0010;
	}
	// b5
	if( 0 != ALM_CHK[ALMMDL_SUB][ALARM_RPAPEROPEN] ){		// ڼ��������W�J
		ret_warning |= 0x0020;
	}
	// b6
	if ( prm_get(COM_PRM, S_PAY, 21, 1, 1) == 1 ){			// �ެ������������
		if( 0 != ALM_CHK[ALMMDL_SUB][ALARM_JPAPEROPEN] ){		// �ެ���������W�J
			ret_warning |= 0x0040;
		}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		else if( 0 != IsErrorOccuerd(ERRMDL_EJA, ERR_EJA_SD_UNAVAILABLE) ){	// SD�J�[�h�g�p�s��
// MH810104 GG119201(S) �d�q�W���[�i���Ή� #5944 SD�J�[�h���p�s���̐��Z�@�̌x����ʂ��Ⴄ
//			ret_warning |= 0x0040;
			ret_warning |= 0x0400;
// MH810104 GG119201(E) �d�q�W���[�i���Ή� #5944 SD�J�[�h���p�s���̐��Z�@�̌x����ʂ��Ⴄ
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	}
	// b7
	if (read_rotsw() != 0) {
		ret_warning |= 0x0080;
	}
	// b8 = CAN �ʐM�s�� || CAN �X�^�b�t�G���[ || CAN �t�H�[���G���[
// MH810100(S) Y.Watanabe 2020/01/26 #3884 �x���ʒm_E8601�̖��\���s��C��
//	if( ERR_CHK[mod_can][ERR_CAN_COMFAIL] ){
	if(( ERR_CHK[mod_can][ERR_CAN_COMFAIL] )||( ERR_CHK[mod_can][ERR_CAN_STUFF_ERR] )||( ERR_CHK[mod_can][ERR_CAN_FORM_ERR] )){
// MH810100(E) Y.Watanabe 2020/01/26 #3884 �x���ʒm_E8601�̖��\���s��C��
		ret_warning |= 0x0100;
	}
	// b9
	if( RAMCLR ){
		ret_warning |= 0x0200;
	}
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	if( isModelMiss_R() ){
		ret_warning |= 0x0800;
	}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

	return( ret_warning );
}

//[]----------------------------------------------------------------------[]
///	@brief			�x���ʒm���M����
//[]----------------------------------------------------------------------[]
///	@param[in]		lcdbm_alarm_check()�̖߂�l = ������̌x���ʒm���
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_notice_alm( ushort warning )
{
	static ushort old_warning = 0;
	uchar	result_PKTcmd;

	// ���݂̌x���ʒm��Ԃ�������̌x���ʒm��Ԃɕω������������̂ݑ��M����
	if (old_warning != warning) {
		// �x���ʒm
		old_warning = warning;

		result_PKTcmd = PKTcmd_notice_alm( old_warning );

// MH810103(s) �d�q�}�l�[�Ή� #5413 ���Z�s��ʂ܂��͌x����ʕ\�����ɁAVP���c���Ɖ�{�^����\�����Ă��܂��Ă���
		// �ω������������ߒʒm
		LcdBackLightCtrl(ON);
// MH810103(e) �d�q�}�l�[�Ή� #5413 ���Z�s��ʂ܂��͌x����ʕ\�����ɁAVP���c���Ɖ�{�^����\�����Ă��܂��Ă���
		if (result_PKTcmd == FALSE) {
			// error
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�|�b�v�A�b�v�\���v��
//[]----------------------------------------------------------------------[]
///	@param[in]		DispCode�F�\���R�[�h<br>
///					DispStatus�F�\�����
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//	code;						status															����				ү����
//	1:�a��ؔ��s(���Z������)	0:�ޑK�Ȃ�, 1:�ޑK����, 2:�ޑK�Ȃ�(�̎���), 3:�ޑK����(�̎���)	0,2=35�^1,3��33		0,1=22/2,3=24
//	2:�a��ؔ��s(�����߂���)	0:�ޑK�Ȃ�, 1:�ޑK����, 2:�ޑK�Ȃ�(�̎���), 3:�ޑK����(�̎���)	0,2=35�^1,3��33		26(OPE_CHR_G_SALE_MSG)
//	3:���z���s(�d�qmony)		---																---					---
//	4:���x����(ICC)				---																---					29
//	5:�Ɖ�NG(ICC)				---																---					30
//	6:�ڼޯĻ��ވُ�			---																---					31
//	7:�c���s��(�d�qmony)		---																---					---
//	8:���Z�s��					---																15					13
//	9:QR���p�s��				0:�d���C1:�f�[�^�s���C2:���̑��ُ�								0=37�C1/2=26		0=14�C2/3=15
// 10:�������s��				0=���Z�����敪NG, 1=�������NG, 2=����NG, 3=�J�[�h�敪NG
// MH810105(S) MH364301 �C���{�C�X�Ή�
// 11:�̎��ؔ��s���s			---	
// MH810105(E) MH364301 �C���{�C�X�Ή�
void	lcdbm_notice_dsp( ePOP_DISP_KIND kind, uchar DispStatus )
{
	uchar	result_PKTcmd;
// MH810103(s) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
	if(DispStatus == 198){
		DispStatus = 98;
	}
// MH810103(e) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX

// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus );
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// 	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus, 0 );
	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus, 0, NULL, 0 );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)

	if (result_PKTcmd == FALSE) {
		// error
	}
}

// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//[]----------------------------------------------------------------------[]
///	@brief			�|�b�v�A�b�v�\���v��(�ǉ���񂠂��)
//[]----------------------------------------------------------------------[]
///	@param[in]		DispCode�F�\���R�[�h<br>
///					DispStatus�F�\�����<br>
///					add_info:�ǉ����
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/07/20<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
//	code;						status																	ү����							�ǉ����
//	1:�a��ؔ��s(���Z������)	0:�ޑK�Ȃ�, 1:�ޑK����, 2:�ޑK�Ȃ�(�̎���), 3:�ޑK����(�̎���)			0,1=2/2,3=2
//	2:�a��ؔ��s(�����߂���)	0:�ޑK�Ȃ�, 1:�ޑK����, 2:�ޑK�Ȃ�(�̎���), 3:�ޑK����(�̎���)			6
//	3:���z���s(�d�qmony)		---																		---
//	5:�Ɖ�NG(ICC)				1=�戵���s��/2=�����J�[�h/3=�g�p�s��/4=�����؂�/5=�ǂݎ��NG/6=�c���s��	1=11/2=19/3=10/4=20/5=21/6=22	6=�J�[�h�c��
//	8:���Z�s��					---																		13
//	9:QR���p�s��				0:�d���C1:�f�[�^�s���C2:���̑��ُ�										0=14/1,2=15(16,17,18)
// 10:�������s��				0=���Z�����敪NG, 1=�������NG, 2=����NG, 3=�J�[�h�敪NG				0=16/1,3=17/2=18
void	lcdbm_notice_dsp2( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info )
{
	uchar	result_PKTcmd;

// MH810103(s) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
	if(DispStatus == 198 ){
		DispStatus = 98;
	}
// MH810103(e) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// 	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus, add_info );
	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus, add_info, NULL, 0 );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	if (result_PKTcmd == FALSE) {
		// error
	}
}
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			�|�b�v�A�b�v�\���v��(�ǉ���񂠂��)
//[]----------------------------------------------------------------------[]
///	@param[in]		DispCode�F�\���R�[�h<br>
///					DispStatus�F�\�����<br>
///					add_info:�ǉ����<br>
///					str:�ǉ�������<br>
///					str_size:�ǉ����
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	lcdbm_notice_dsp3( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info, uchar *str, ulong str_size )
{
	if ( DispStatus == 198 ) {
		DispStatus = 98;
	}

	PKTcmd_notice_dsp((uchar)kind, DispStatus, add_info, str, str_size);
}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
//[]----------------------------------------------------------------------[]
///	@brief			�|�b�v�A�b�v�폜�v��
//[]----------------------------------------------------------------------[]
///	@param[in]		kind�F���<br>
///					status�F���<br>
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2023/05/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	lcdbm_notice_del( uchar kind, uchar status )
{
	uchar	result_PKTcmd;

	if (OPECTL.Ope_mod == 3) {
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 		if ( ( IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1)) &&
// 		                     (Ope_isJPrinterReady() && (paperchk2() != -1))) ||	
// 		     (!IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1))) ) {	// �C���{�C�X�ł͂Ȃ��ꍇ�̓��V�[�g�̂ݔ���
		// �C���{�C�X�ł͂Ȃ��ꍇ�̓��V�[�g�̂ݔ���A�d�q�̎��؂̏ꍇ�͏�Ƀ��V�[�g�o�͉�
		if ( ( IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1))) &&
		                                     (Ope_isJPrinterReady() && (paperchk2() != -1)) ) ||
		     (!IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1)))) ) {
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
			// ���V�[�g�E�W���[�i���󎚉\�ł���΁A�󎚐����Ɣ��f����
			result_PKTcmd = PKTcmd_notice_del( kind, status );
		}
		else {
			// ���V�[�g�E�W���[�i���̂ǂ��炩�󎚕s�ł���΁A�󎚎��s�Ɣ��f����
			// �󎚎��sPOP���o��̂ō폜�v���͑��M���Ȃ�
		}
	}

	if (result_PKTcmd == FALSE) {
		// error
	}
}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j

//[]----------------------------------------------------------------------[]
///	@brief			���Z�c���ω��ʒm
//[]----------------------------------------------------------------------[]
// GG124100(S) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
// ///	@param[in]		pay_sts�F���Z�X�e�[�^�X(0:���Z��,1:���Z����,2:���Z��,3:���Z�s��)
///	@param[in]		pay_sts�F���Z�X�e�[�^�X(0:���Z��,1:���Z����,2:���Z��,3:���Z�s��,4:���Z��(���~))
// GG124100(E) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_pay_rem_chg( uchar pay_sts )
{
	lcdbm_cmd_pay_rem_chg_t lcdbm_cmd_pay_rem_chg;
	DATE_YMDHMS *paytime;

	memset( &lcdbm_cmd_pay_rem_chg, 0, sizeof(lcdbm_cmd_pay_rem_chg) );

	// ����ދ��ʏ��_����ޒ�
	lcdbm_cmd_pay_rem_chg.command.length = sizeof(lcdbm_cmd_pay_rem_chg_t) - sizeof(unsigned short);

	// ����ދ��ʏ��_�����ID
	lcdbm_cmd_pay_rem_chg.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;

	// ����ދ��ʏ��_��޺����ID
	lcdbm_cmd_pay_rem_chg.command.subid = LCDBM_CMD_SUBID_PAY_REM_CHG;

	// ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	lcdbm_cmd_pay_rem_chg.id = op_GetSequenceID();

	// ���Z�X�e�[�^�X
	//		0:���Z��
	//		1:���Z����
	//		2:���Z��
	//		3:���Z�s��
// GG124100(S) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
// 	//      4:���Z����(�݂Ȃ�����)
	//      4:���Z��(���~)
// GG124100(E) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
	lcdbm_cmd_pay_rem_chg.pay_sts = pay_sts;
// MH810100(S) K.Onodera 2020/03/03 #3989 ���Ɏ����w��̉��u���Z�œ���������\������Ȃ�
	// ���u���Z(���Ɏ����w��)
	if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_TIME ){
		lcdbm_cmd_pay_rem_chg.pay_sts += 10;
	}
	// ���u���Z(���z�w��)
	else if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE ){
		lcdbm_cmd_pay_rem_chg.pay_sts += 20;
	}
// MH810100(E) K.Onodera 2020/03/03 #3989 ���Ɏ����w��̉��u���Z�œ���������\������Ȃ�

	// ���Z(����)�N���������b �����u���Z(���Ɏ����w��)�ł͓��Ɏ������Z�b�g����Ă���
	paytime = GetPayTargetTime();
	memcpy( &lcdbm_cmd_pay_rem_chg.pay_time, paytime, sizeof(lcdbm_cmd_pay_rem_chg.pay_time) );

// MH810101(S) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
	// ����`�F�b�N����
	lcdbm_cmd_pay_rem_chg.season_chk_result = season_chk_result;
// MH810101(E) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129000(S) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
//	if ( OPECTL.op_faz == 1 ){
//	if ( OPECTL.op_faz == 1 || OPECTL.op_faz == 2){	// �������܂��͐��Z����
	if ( ope_MakeLaneLog_Check(pay_sts) ){	// �������܂��͐��Z����	
// GG129000(E) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
		// ������
		// ���[�����j�^�f�[�^�o�^
		SetLaneFeeKind(ryo_buf.syubet+1);
		SetLaneFreeNum(ryo_buf.zankin);
		ope_MakeLaneLog(LM_PAY_MNY_IN);
	}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

	if( pay_sts < 2 ){
		// ���ԗ���
// MH810100(S) K.Onodera 2020/02/25 #3911 ����Ԃ̏ꍇ�ɗ�����ʂ��\������Ă��܂�
//		lcdbm_cmd_pay_rem_chg.prk_fee = ryo_buf.tyu_ryo;
		if( ryo_buf.ryo_flg < 2 ){
			lcdbm_cmd_pay_rem_chg.prk_fee = ryo_buf.tyu_ryo;
		}else{
			lcdbm_cmd_pay_rem_chg.prk_fee = ryo_buf.tei_ryo;
		}
// MH810100(E) K.Onodera 2020/02/25 #3911 ����Ԃ̏ꍇ�ɗ�����ʂ��\������Ă��܂�

		// ���Z�c�z
		lcdbm_cmd_pay_rem_chg.pay_rem = ryo_buf.zankin;

		// ���������z(�O�񐸎Z�ɂ��x��������ꍇ�͉��Z)
// MH810103(s) �d�q�}�l�[�Ή�
//		lcdbm_cmd_pay_rem_chg.cash_in = (ryo_buf.nyukin + ryo_buf.zenkai);
		// �����������v�z�AIC�J�[�h�^�N���W�b�g�ɂ�錈�ϋ��z
		if(PayData.Electron_data.Suica.e_pay_kind == EC_CREDIT_USED){
			// �N���W�b�g���́A�����݂̂ɁB
			lcdbm_cmd_pay_rem_chg.cash_in = (ryo_buf.nyukin +  ryo_buf.zenkai);
		}else if(PayData.Electron_data.Suica.e_pay_kind != 0){
// MH810103(s) �d�q�}�l�[�Ή� #5465 �Đ��Z�œd�q�}�l�[���g�p�������Z�������A���Z�@��ʏ�̈���������z�ɏ��񐸎Z�����܂�ł��܂�
//			lcdbm_cmd_pay_rem_chg.cash_in = (PayData.Electron_data.Suica.pay_ryo + ryo_buf.zenkai);
			lcdbm_cmd_pay_rem_chg.cash_in = ryo_buf.zenkai;		// �O��̂�
			lcdbm_cmd_pay_rem_chg.emoney_pay = PayData.Electron_data.Suica.pay_ryo;
// MH810103(e) �d�q�}�l�[�Ή� #5465 �Đ��Z�œd�q�}�l�[���g�p�������Z�������A���Z�@��ʏ�̈���������z�ɏ��񐸎Z�����܂�ł��܂�
		}else{
			lcdbm_cmd_pay_rem_chg.cash_in = (ryo_buf.nyukin + ryo_buf.zenkai);
		}
// MH810103(e) �d�q�}�l�[�Ή�

		// �ޑK�z
		lcdbm_cmd_pay_rem_chg.chg_amt = ryo_buf.turisen;

		// �����z
		// lcdbm_cmd_pay_rem_chg.disc_amt = ryo_buf_n.dis;
// MH810100(S) 2020/06/17 #4497�y�A���]���w�E�����zAi-�X�����Ɏ������芄������̉��u���Z���s���ƁA�s���ȁi�O�񐸎Z���ɓK�������j�������ēK�����Đ��Z���Ă��܂�
//// MH810100(S) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
////		lcdbm_cmd_pay_rem_chg.disc_amt = ryo_buf.waribik;
//		lcdbm_cmd_pay_rem_chg.disc_amt = ryo_buf.waribik - ryo_buf.zenkai;
//// MH810100(E) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
		if( ryo_buf.waribik > ryo_buf.zenkai){
			lcdbm_cmd_pay_rem_chg.disc_amt = ryo_buf.waribik - ryo_buf.zenkai;
		}else{
			lcdbm_cmd_pay_rem_chg.disc_amt = 0;
		}
// MH810100(E) 2020/06/17 #4497�y�A���]���w�E�����zAi-�X�����Ɏ������芄������̉��u���Z���s���ƁA�s���ȁi�O�񐸎Z���ɓK�������j�������ēK�����Đ��Z���Ă��܂�
// MH810103(s) �d�q�}�l�[�Ή�
		lcdbm_cmd_pay_rem_chg.method = 0;	// ����
		
		if(PayData.Electron_data.Suica.e_pay_kind == EC_CREDIT_USED){
			lcdbm_cmd_pay_rem_chg.method = 1; 												// �N���W�b�g���Z
		}else if( PayData.Electron_data.Suica.e_pay_kind != 0 ){							// �d�q���ώ��
			if (e_zandaka >= 0) {
				lcdbm_cmd_pay_rem_chg.method = 2; 												// �d�q�}�l�[
			}else{
// MH810105(S) 2022/01/13 iD�Ή� �|�X�g�y�C�^�ł݂Ȃ����ς̕\���ɂȂ��Ă��܂��s��C��
//				lcdbm_cmd_pay_rem_chg.method = 3; 												// �d�q�}�l�[(�݂Ȃ�����)
				// iD or QUICPay�ŁA�݂Ȃ��̏ꍇ�́A�t���O���݂�B����ȊO�́Ae_zandaka�Ń`�F�b�N
				if( PayData.Electron_data.Suica.e_pay_kind == EC_ID_USED || 
				PayData.Electron_data.Suica.e_pay_kind == EC_QUIC_PAY_USED ){
				    if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin != 0 &&
					PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0) {
						lcdbm_cmd_pay_rem_chg.method = 3; 												// �d�q�}�l�[(�݂Ȃ�����)
					}else{
						lcdbm_cmd_pay_rem_chg.method = 2; 												// �d�q�}�l�[
					}
				}else{
					lcdbm_cmd_pay_rem_chg.method = 3; 												// �d�q�}�l�[(�݂Ȃ�����)
				}
// MH810105(E) 2022/01/13 iD�Ή� �|�X�g�y�C�^�ł݂Ȃ����ς̕\���ɂȂ��Ă��܂��s��C��
			}
			lcdbm_cmd_pay_rem_chg.brand = PayData.Electron_data.Suica.e_pay_kind;			// �u�����h
			lcdbm_cmd_pay_rem_chg.emoney_balance = PayData.Electron_data.Suica.pay_after; 	// �d�q�}�l�[�c��
		}
// MH810103(e) �d�q�}�l�[�Ή�

// GG129000(S) R.Endo 2024/01/12 #7217 ���������v�\���Ή�
		// �������v
		lcdbm_cmd_pay_rem_chg.Kaimono = ryo_buf.shopping_total;
// GG129000(E) R.Endo 2024/01/12 #7217 ���������v�\���Ή�
// GG129004(S) M.Fujikawa 2024/10/22 �������z�������Ή�
		lcdbm_cmd_pay_rem_chg.Kaimono_info = ryo_buf.shopping_info;
// GG129004(E) M.Fujikawa 2024/10/22 �������z�������Ή�
	}

	// lcdbm_cmd_pay_rem_chg_t�^��I/F
	PKTcmd_pay_rem_chg( &lcdbm_cmd_pay_rem_chg );
}

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
//[]----------------------------------------------------------------------[]
///	@brief			���Ɏ����w�艓�u���Z�J�n
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void	lcdbm_remote_time_start( void )
{
	lcdbm_cmd_remote_time_start_t lcdbm_cmd_remote_time_start;

	// ������
	memset(&lcdbm_cmd_remote_time_start, 0, sizeof(lcdbm_cmd_remote_time_start_t));

	// ���ԏꇂ
	lcdbm_cmd_remote_time_start.ulPno = g_PipCtrl.stRemoteTime.ulPno;

	// �������
	lcdbm_cmd_remote_time_start.RyoSyu = g_PipCtrl.stRemoteTime.RyoSyu;

	// ���ɔN���������b
	lcdbm_cmd_remote_time_start.InTime.Year	 = g_PipCtrl.stRemoteTime.InTime.Year;
	lcdbm_cmd_remote_time_start.InTime.Mon	 = g_PipCtrl.stRemoteTime.InTime.Mon;
	lcdbm_cmd_remote_time_start.InTime.Day	 = g_PipCtrl.stRemoteTime.InTime.Day;
	lcdbm_cmd_remote_time_start.InTime.Hour	 = g_PipCtrl.stRemoteTime.InTime.Hour;
	lcdbm_cmd_remote_time_start.InTime.Min	 = g_PipCtrl.stRemoteTime.InTime.Min;
	lcdbm_cmd_remote_time_start.InTime.Sec	 = (uchar)g_PipCtrl.stRemoteTime.InTime.Sec;

	PKTcmd_remote_time_start(&lcdbm_cmd_remote_time_start);
}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

//[]----------------------------------------------------------------------[]
///	@brief			 RT���Z�f�[�^�̊�{���쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Set_All_RTPay_Data( void )
{
	uchar 	loop_cnt;
// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//	stDiscount2_t* pDisc2;
//	stDiscount_t*  pDisc3;
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�

	// ���A���^�C�����Z�f�[�^�p�o�b�t�@���N���A���Ă���
	memset( &RTPay_Data, 0, sizeof(RTPay_Data) );

// MH810100(S) K.Onodera 2020/03/05 #3912 �������Z�̐��Z�f�[�^���s��
	// ���O/�������
	RTPay_Data.shubetsu = lcdbm_rsp_in_car_info_main.shubetsu;
// MH810100(E) K.Onodera 2020/03/05 #3912 �������Z�̐��Z�f�[�^���s��

// MH810100(S) 2020/09/08 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
//	// �t�H�[�}�b�gRev.No.
//	RTPay_Data.crd_info.FormatNo = 0;
	// ID
	RTPay_Data.ID = lcdbm_rsp_in_car_info_main.id;

	// �J�[�h���Rev.No.
	RTPay_Data.crd_info_rev_no = lcdbm_rsp_in_car_info_main.crd_info_rev_no;

	// ̫�ϯ�Rev.No.
	RTPay_Data.crd_info.FormatNo = lcdbm_rsp_in_car_info_main.crd_info.FormatNo;
// MH810100(E) 2020/09/08 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j

// MH810100(S) 2020/07/27 #4557�y���؉ێw�E�����z ���A���^�C�����Z�f�[�^�̏����������s��(�`�~�F39)
// // �N���������b
// memcpy( &RTPay_Data.crd_info.dtTimeYtoSec,
// 		&lcdbm_rsp_in_car_info_main.crd_info.dtTimeYtoSec,
// 		sizeof(stDatetTimeYtoSec_t));
	// RTPay_LogRegist_AddOiban�ŏ����N����������
// MH810100(E) 2020/07/27 #4557�y���؉ێw�E�����z ���A���^�C�����Z�f�[�^�̏����������s��(�`�~�F39)

	// �J�[�h���
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// 	RTPay_Data.crd_info.dtCardInfo.RevNoInfo	= 10;		// Rev.No.					10�`65535
// GG129004(S) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
//	RTPay_Data.crd_info.dtCardInfo.RevNoInfo	= 11;		// Rev.No.					11�`65535
	if((char)prm_get(COM_PRM,S_LCD, 49, 2, 1) != 11){
		RTPay_Data.crd_info.dtCardInfo.RevNoInfo	= 12;		// Rev.No.					11�`65535
	}else{
		RTPay_Data.crd_info.dtCardInfo.RevNoInfo	= 11;		// Rev.No.					11�`65535
	}
// GG129004(E) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)

	RTPay_Data.crd_info.dtCardInfo.MasterOnOff	= 1;		// Ͻ�����  �L��			0:���1:�L
// MH810100(S) K.Onodera 2020/02/12 #3852 ���A���^�C�����Z�f�[�^�́u�J�[�h���@�\����v�Ɂu�L�v���Z�b�g���Ă��܂��Ă���
//	RTPay_Data.crd_info.dtCardInfo.ReserveOnOff	= 1;		// �\����  �L��			0:���1:�L
	RTPay_Data.crd_info.dtCardInfo.ReserveOnOff	= 0;		// �\����  �L��			0:���1:�L
// MH810100(E) K.Onodera 2020/02/12 #3852 ���A���^�C�����Z�f�[�^�́u�J�[�h���@�\����v�Ɂu�L�v���Z�b�g���Ă��܂��Ă���
	RTPay_Data.crd_info.dtCardInfo.PassOnOff	= 1;		// ����������  �L��		0:���1:�L
	RTPay_Data.crd_info.dtCardInfo.ZaishaOnOff	= 1;		// �ݎԏ��  �L��			0:���1:�L

	RTPay_Data.crd_info.dtCardInfo.MasterSize	= sizeof(stMasterInfo_t);	// �}�X�^�[���T�C�Y
	RTPay_Data.crd_info.dtCardInfo.ReserveSize	= sizeof(stReserveInfo_t);	// �\����T�C�Y
	RTPay_Data.crd_info.dtCardInfo.PassSize		= sizeof(stPassInfo_t);		// ����������T�C�Y
	RTPay_Data.crd_info.dtCardInfo.ZaishaSize	= sizeof(stZaishaInfo_t);	// �ݎԏ��T�C�Y

	// ============================================== //
	//		�}�X�^�[���
	// ============================================== //

	// �J�[�h1�`6(���ԏꇂ, ���, �ԍ�)
	for( loop_cnt = 0; loop_cnt < ONL_MAX_CARDNUM; loop_cnt++ ){
		memcpy( &RTPay_Data.crd_info.dtMasterInfo.stCardDataInfo[loop_cnt],
				&lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[loop_cnt],
				sizeof(stParkKindNum_t) );
	}

	// �\���p�J�[�h1�`6
	for( loop_cnt = 0; loop_cnt < ONL_MAX_CARDNUM; loop_cnt++) {
		memcpy( &RTPay_Data.crd_info.dtMasterInfo.stDispCardDataInfo[loop_cnt],
				&lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[loop_cnt],
				sizeof(stDispCardData_t) );
	}

	// �L���J�n�N����
	memcpy( &RTPay_Data.crd_info.dtMasterInfo.StartDate,
			&lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.StartDate,
			sizeof(stDate_YYMD_t));

	// �L���I���N����
	memcpy( &RTPay_Data.crd_info.dtMasterInfo.EndDate,
			&lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.EndDate,
			sizeof(stDate_YYMD_t));

	// ������(0�`15)
	RTPay_Data.crd_info.dtMasterInfo.SeasonKind = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.SeasonKind;

	// ������(0�`255)
	RTPay_Data.crd_info.dtMasterInfo.MemberKind = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.MemberKind;

	// ���o�ɃX�e�[�^�X 0�`99(0=�������(���o�ɉ�),1=�o�ɒ�(���ɉ�),2=���ɒ�(�o�ɉ�))
	RTPay_Data.crd_info.dtMasterInfo.InOutStatus = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.InOutStatus;

	// �x������i
	RTPay_Data.crd_info.dtMasterInfo.PayMethod = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.PayMethod;

	// ����m�F�t���O
	RTPay_Data.crd_info.dtMasterInfo.EntryConf = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.EntryConf;

	// �\��(0�Œ�)
	RTPay_Data.crd_info.dtMasterInfo.Reserve1 = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.Reserve1;


	// ============================================== //
	//		������
	// ============================================== //

	// �_��Ǘ��ԍ�		1�`16,000,000
	RTPay_Data.crd_info.dtPassInfo.ContractNo = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.ContractNo;

	// �̔��ð��		1=�̔��ς�/9=�̔��O
	RTPay_Data.crd_info.dtPassInfo.SaleStatus = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.SaleStatus;

	// ���޽ð��		0�`99 (1=�L���2=����)
	RTPay_Data.crd_info.dtPassInfo.CardStatus = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.CardStatus;

	// ����敪			0�`99
	RTPay_Data.crd_info.dtPassInfo.Classification = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.Classification;

	// �_����ԍ�		0�`26
	RTPay_Data.crd_info.dtPassInfo.DivisionNo = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.DivisionNo;

	// �_��Ԏ��ԍ�		1�`9999
	RTPay_Data.crd_info.dtPassInfo.CasingNo = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.CasingNo;

	// �_��ׯ����
	RTPay_Data.crd_info.dtPassInfo.RackInfo = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.RackInfo;

	// 4���ׂ��X�V����	0�`1(0=�����Ȃ�/1=������)
	RTPay_Data.crd_info.dtPassInfo.UpdatePermission = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.UpdatePermission;

	// ��߼�ݑ���׸އ@	�e�ޯđΉ�
	RTPay_Data.crd_info.dtPassInfo.Option1 = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.Option1;

	// ��߼�ݑ���׸އA	�e�ޯđΉ�
	RTPay_Data.crd_info.dtPassInfo.Option2 = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.Option2;

	// �Ĕ��s��		0�`255
	RTPay_Data.crd_info.dtPassInfo.ReWriteCount = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.ReWriteCount;

	// �\��				0�Œ�
	RTPay_Data.crd_info.dtPassInfo.Reserve1 = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.Reserve1;


	// ============================================== //
	//		�ݎԏ��
	// ============================================== //

	// �ݎԒ��̒��ԏꇂ(0�`999999)�ݎԂȂ��̏ꍇ�͢0�
	RTPay_Data.crd_info.dtZaishaInfo.ResParkingLotNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.ResParkingLotNo;

	// ���Z�����敪	0=�����Z/1=���Z/2=�Đ��Z/3=���Z���~/4=�Đ��Z���~
	RTPay_Data.crd_info.dtZaishaInfo.PaymentType = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType;

	// ���Z�o��		0=�ʏ퐸�Z/1=�����o��/(2=���Z�Ȃ��o��)/3=�s���o��/
	// 				9=�˔j�o��10=�ްĊJ��/97=ۯ��J��ׯ�ߏ㏸�O�����Z�o��/
	// 				98=׸���ѓ��o��/99=���޽��ѓ��o��
	RTPay_Data.crd_info.dtZaishaInfo.ExitPayment = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.ExitPayment;

	// ��������z(0�`999999)
	RTPay_Data.crd_info.dtZaishaInfo.GenkinFee = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.GenkinFee;

	// �ݎԏ��_�������(0�`99)
	RTPay_Data.crd_info.dtZaishaInfo.shFeeType = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType;

	// ���ɏ��
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtEntryDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime,
			sizeof(stDateParkTime_t));

	// ���Z���
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime,
			sizeof(stDateParkTime_t));

	// �o�ɏ��
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtExitDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime,
			sizeof(stDateParkTime_t));

	// �����ʉߏ��
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtEntranceDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntranceDateTime,
			sizeof(stDateParkTime_t));

	// �o���ʉߏ��
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtOutleteDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtOutleteDateTime,
			sizeof(stDateParkTime_t));

	// ���ԗ���(0�`999990(10�~�P��))
	RTPay_Data.crd_info.dtZaishaInfo.ParkingFee = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.ParkingFee;

	// ���Ԏ���(0�`999999(���P��))
	RTPay_Data.crd_info.dtZaishaInfo.ParkingTime = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.ParkingTime;

	// �ꎞ���p�t���O(0=������p,1=�ꎞ���p)
	RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.NormalFlag;

// GG129004(S) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
//	// �\��
//	RTPay_Data.crd_info.dtZaishaInfo.Reserve3 = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.Reserve3;
	// ���Ƀ��[�h
	RTPay_Data.crd_info.dtZaishaInfo.InCarMode = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.InCarMode;
// GG129004(E) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�

	// ���Z���01�`10
	for( loop_cnt = 0; loop_cnt < ONL_MAX_SETTLEMENT; loop_cnt++) {
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt],
				&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[loop_cnt],
				sizeof(stSettlement_t) );
	}

	// �Ïؔԍ�(�����@�ԍ�, �ԍ�)
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stPassword,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stPassword,
			sizeof(stPasswordNo_t) );

// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//	// ����01�`25
//	/************************************************************************************/
//	/* �ݎԏ��_����01�`25�͊����̐��Z�����I����(OnlineDiscount = ��ײ݊�������)��		*/
//	/* m_stDisc�����߰����̂ł��̊������Z����ް����g�p����							*/
//	/************************************************************************************/
//	taskchg(IDLETSKNO);
//	for( loop_cnt = 0; loop_cnt < ONL_MAX_DISC_NUM; loop_cnt++) {
//		pDisc3 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt];
//		pDisc2 = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[loop_cnt];
//
//		pDisc3->DiscParkNo		= pDisc2->DiscParkNo;			// ���� ���ԏꇂ(0�`999999)
//		pDisc3->DiscSyu			= pDisc2->DiscSyu;				// ���� ���(0�`9999)
//		pDisc3->DiscCardNo		= pDisc2->DiscCardNo;			// ���� ���ދ敪(0�`65000)
//		pDisc3->DiscNo			= pDisc2->DiscNo;				// ���� �敪(0�`9)
//		pDisc3->DiscCount		= pDisc2->DiscCount;			// ���� ����(0�`99)
//		pDisc3->DiscInfo		= pDisc2->DiscInfo;				// ���� �������(0�`65000)
//		pDisc3->DiscCorrType	= pDisc2->DiscCorrType;			// ���� �Ή����ގ��(0�`65000)
//		pDisc3->DiscStatus		= pDisc2->DiscStatus;			// ���� �ð��(0�`9)
//		pDisc3->DiscFlg			= pDisc2->DiscFlg;				// ���� ������(0�`9)
//		// ���Ԋ����H
//// MH810100(S) 2020/05/28 �Ԕԃ`�P�b�g���X(#4196)
////		if( pDisc2->DiscSyu >= NTNET_SVS_T ){
////			pDisc3->Discount		= pDisc2->DiscountT;		// ���� ����(0�`999999)
////		}else{
////			pDisc3->Discount		= pDisc2->DiscountM;		// ���� ���z(0�`999999)
////		}
//// MH810100(S) 2020/07/29 �Ԕԃ`�P�b�g���X(#4561 �Z���^�[�����M������������ύX����)
//// pDisc3->Discount		= pDisc2->Discount;				// ���z/����(0�`999999)
//		pDisc3->Discount		= pDisc2->UsedDisc;				// ���z/����(0�`999999)
//// MH810100(E) 2020/07/29 �Ԕԃ`�P�b�g���X(#4561 �Z���^�[�����M������������ύX����)
////		pDisc3->UsedDisc		= pDisc2->UsedDisc;				// �����������p���̎g�p�ς݊����i���z/���ԁj(0�`999999)
//// MH810100(E) 2020/05/28 �Ԕԃ`�P�b�g���X(#4196)
//	}
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�

}

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			RT�̎��؃f�[�^�̊�{���쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	Set_All_RTReceipt_Data( void )
{
	// RT�̎��؃f�[�^�p�o�b�t�@���N���A���Ă���
	memset(&RTReceipt_Data, 0, sizeof(RTReceipt_log));

	// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	RTReceipt_Data.ID = lcdbm_rsp_in_car_info_main.id;

	// RTReceipt_LogRegist_AddOiban �� �Z���^�[�ǔ� ������

	// RTReceipt_LogRegist_AddOiban �� �Z���^�[�ǔԕs���t���O ������

	// �t�H�[�}�b�gRev.No.
	RTReceipt_Data.receipt_info.FormatNo = 1;

	// ���Z�@ �@��R�[�h
	RTReceipt_Data.receipt_info.ModelCode = NTNET_MODEL_CODE;

	// RTReceipt_LogRegist_AddOiban �� ���� �N���������b ������

	// Set_Pay_RTReceipt_Data �Ŏc�������
}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

//[]----------------------------------------------------------------------[]
///	@brief			���ɏ��̃`�F�b�N�Ɛݒ菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void lcdbm_rsp_in_car_info_proc( void )
{
// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//	uchar 	loop_cnt, i, j;
	uchar 	i, j;
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
	stParkKindNum_t *pCard1 = NULL, *pCard2 = NULL;
	stParkKindNum_t stTmp;
// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//	stDiscount_t* pDisc1;
//	stDiscount2_t *pDisc2;
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// GG129000(S) T.Nagai 2023/02/13 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
	uchar	carno[36];
// GG129000(E) T.Nagai 2023/02/13 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j

	switch( OPECTL.Ope_mod ){
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
		// op_mod00
		case 0:
// GG129003(S) M.Fujikawa 2024/11/22 �W���s��C���@GM858400(S)[�}�[�W] �N���E�h�����v�Z�Ή�(���P�A��7�A8:���u���Z���̏��Z�b�g�R��)
			op_mod01_Init();												// op_mod01() ����߂̂��ߴر�̂ݸر
			memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET���Z�ް��t�����ر

			memcpy( &lcdbm_rsp_in_car_info_main, &lcdbm_rsp_in_car_info_recv, sizeof(lcdbm_rsp_in_car_info_main) );

			// ���Z�J�n�����ێ�
			SetPayStartTimeInCarInfo();

			Ope_Set_tyudata_Card();
			Set_All_RTPay_Data();

			set_tim_only_out_card(0);

			// �J�[�h���\�[�g
			taskchg(IDLETSKNO);
			for( i=0; i<ONL_MAX_CARDNUM; i++ ){
				pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
				for( j=i+1; j<ONL_MAX_CARDNUM; ++j ){
					pCard2 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[j];
					if( pCard2->CardType && (pCard1->CardType > pCard2->CardType) ){
						memcpy( &stTmp, pCard1, sizeof(stTmp) );
						memcpy( pCard1, pCard2, sizeof(stTmp) );
						memcpy( pCard2, &stTmp, sizeof(stTmp) );
					}
				}
			}
			
			// �\���J�[�h�ԍ��Z�b�g
			taskchg(IDLETSKNO);
			for( i=0; i<ONL_MAX_CARDNUM; i++ ){
				pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
				if( pCard1->CardType ){
					memcpy( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[i].byDispCardNo, pCard1->byCardNo, sizeof(stDispCardData_t) );
					if (pCard1->CardType == CARD_TYPE_PARKING_TKT) {
						SetLaneMedia(pCard1->ParkingLotNo,
										pCard1->CardType,
										pCard1->byCardNo);
					}
				}
			}
			// �Ԕԏ��iUTF-8�j�i�\���𔲂��ăR�s�[����j
			// ���^�x�ǖ��A���ޔԍ�
			memcpy(&carno[0], lcdbm_rsp_in_car_info_main.ShabanInfo.LandTransOfficeName, (size_t)(12+9));
			// �p�r����
			memcpy(&carno[(12+9)], lcdbm_rsp_in_car_info_main.ShabanInfo.UsageCharacter, (size_t)3);
			// ��A�ԍ�
			memcpy(&carno[(12+9+3)], lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber, (size_t)12);
			SetLaneFreeStr(carno, sizeof(carno));
			SetLaneFeeKind((ushort)lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.FeeType);
			SetLaneFreeNum(lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.ParkingFee);
			// ���u���Z�J�n��
			ope_MakeLaneLog(LM_PAY_CHANGE);

			// ���Z���ύX�f�[�^��M���͊����}�̏d���`�F�b�N�p�G���A���N���A
			memset( &g_UseMediaData, 0, sizeof( t_UseMedia ) );

			// ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)�̃Z�b�g
			op_SetSequenceID( lcdbm_rsp_in_car_info_main.id );
			
			break;
// GG129003(E) M.Fujikawa 2024/11/22 �W���s��C���@GM858400(S)[�}�[�W] �N���E�h�����v�Z�Ή�(���P�A��7�A8:���u���Z���̏��Z�b�g�R��)
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
		// op_mod01
		case 1:
			// ���ɏ��̑S�R�s�[
			memcpy( &lcdbm_rsp_in_car_info_main, &lcdbm_rsp_in_car_info_recv, sizeof(lcdbm_rsp_in_car_info_main) );

			// �J�[�h���\�[�g
			taskchg(IDLETSKNO);
			for( i=0; i<ONL_MAX_CARDNUM; i++ ){
				pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
				for( j=i+1; j<ONL_MAX_CARDNUM; ++j ){
					pCard2 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[j];
					if( pCard2->CardType && (pCard1->CardType > pCard2->CardType) ){
						memcpy( &stTmp, pCard1, sizeof(stTmp) );
						memcpy( pCard1, pCard2, sizeof(stTmp) );
						memcpy( pCard2, &stTmp, sizeof(stTmp) );
					}
				}
			}

			// �\���J�[�h�ԍ��Z�b�g
			taskchg(IDLETSKNO);
			for( i=0; i<ONL_MAX_CARDNUM; i++ ){
				pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
				if( pCard1->CardType ){
					memcpy( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[i].byDispCardNo, pCard1->byCardNo, sizeof(stDispCardData_t) );
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					if (pCard1->CardType == CARD_TYPE_PARKING_TKT) {
						SetLaneMedia(pCard1->ParkingLotNo,
										pCard1->CardType,
										pCard1->byCardNo);
					}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				}else{
					memset( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[i], 0, sizeof(stDispCardData_t) );
				}
			}

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
			if ( OPECTL.remote_wait_flg == 0 ) {	// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

			// ���Z�f�[�^�̊�{���쐬 = RTPay_Data �̍쐬
			Set_All_RTPay_Data();

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
			// �̎��؃f�[�^�̊�{���쐬 = RTReceipt_Data �̍쐬
			Set_All_RTReceipt_Data();
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

			// ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)�̃Z�b�g
			op_SetSequenceID( lcdbm_rsp_in_car_info_main.id );

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// 			// QR���p���̊����f�[�^��r�p�Ƀo�b�N�A�b�v
// 			memcpy( DiscountBackUpArea, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo, sizeof(DiscountBackUpArea) );
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
			}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

			break;

		// op_mod02
		case 2:
			// ID�̃`�F�b�N(���Ɂ`���Z�����܂ł̊Ǘ�ID) 
			if( lcdbm_rsp_in_car_info_main.id == lcdbm_rsp_in_car_info_recv.id ){

				// �����d���`�F�b�N����
				lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup = lcdbm_rsp_in_car_info_recv.crd_info.ValidCHK_Dup;

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// 				// �����v�Z���ʃR�s�[
// 				memcpy( &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo, &lcdbm_rsp_in_car_info_recv.crd_info.dtCalcInfo, sizeof( stPayResultInfo_t ) );
				if ( lcdbm_rsp_in_car_info_recv.crd_info.dtReqRslt.FeeCalcOnOff ) {			// �����v�Z���ʂ���
					// �����v�Z���ʃR�s�[
					memcpy(&lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo, &lcdbm_rsp_in_car_info_recv.crd_info.dtFeeCalcInfo, sizeof(stFeeCalcInfo_t));
				} else if ( lcdbm_rsp_in_car_info_recv.crd_info.dtReqRslt.PayResultInfo ) {	// ������񂠂�
					// �������R�s�[
					memcpy(&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo, &lcdbm_rsp_in_car_info_recv.crd_info.dtCalcInfo, sizeof(stPayResultInfo_t));
				}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// GG129000(S) H.Fujinaga 2023/01/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
				if (lcdbm_rsp_in_car_info_recv.shubetsu == 2) {
					// ���Z���ύX�f�[�^�K�p��

					lcdbm_rsp_in_car_info_main.shubetsu = lcdbm_rsp_in_car_info_recv.shubetsu;
					memcpy(&lcdbm_rsp_in_car_info_main.data.PayInfoChange,
							&lcdbm_rsp_in_car_info_recv.data.PayInfoChange,
							sizeof(lcdbm_rsp_in_car_info_main.data.PayInfoChange));
					if ( PayInfoChange_StateCheck() == 1 ){
						// ���u���Z����
						memcpy( &lcdbm_rsp_in_car_info_main, &lcdbm_rsp_in_car_info_recv, sizeof(lcdbm_rsp_in_car_info_main) );

						// ���Z�J�n�����ێ�
						SetPayStartTimeInCarInfo();

						Ope_Set_tyudata_Card();
						Set_All_RTPay_Data();
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
						Set_All_RTReceipt_Data();
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

						set_tim_only_out_card(0);

						taskchg(IDLETSKNO);
						for( i=0; i<ONL_MAX_CARDNUM; i++ ){
							pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
							if( pCard1->CardType ){
								memcpy( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[i].byDispCardNo, pCard1->byCardNo, sizeof(stDispCardData_t) );
								if (pCard1->CardType == CARD_TYPE_PARKING_TKT) {
									SetLaneMedia(pCard1->ParkingLotNo,
													pCard1->CardType,
													pCard1->byCardNo);
								}
							}
						}
						// �Ԕԏ��iUTF-8�j�i�\���𔲂��ăR�s�[����j
						// ���^�x�ǖ��A���ޔԍ�
						memcpy(&carno[0], lcdbm_rsp_in_car_info_main.ShabanInfo.LandTransOfficeName, (size_t)(12+9));
						// �p�r����
						memcpy(&carno[(12+9)], lcdbm_rsp_in_car_info_main.ShabanInfo.UsageCharacter, (size_t)3);
						// ��A�ԍ�
						memcpy(&carno[(12+9+3)], lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber, (size_t)12);
						SetLaneFreeStr(carno, sizeof(carno));
// GG129000(S) ���P�A��No.69,No.73 ���Z�ς݂̍ݎԂ��ݎԑ��M�������ɐ��Z���i���O���Z�F�����ς݁i�����܂ށj�j���iGM803002���p�j
						SetLaneFeeKind((ushort)lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.FeeType);
						SetLaneFreeNum(lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.ParkingFee);
						// ���u���Z�J�n��
						ope_MakeLaneLog(LM_PAY_CHANGE);
// GG129000(E) ���P�A��No.69,No.73 ���Z�ς݂̍ݎԂ��ݎԑ��M�������ɐ��Z���i���O���Z�F�����ς݁i�����܂ށj�j���iGM803002���p�j

						// ���Z���ύX�f�[�^��M���͊����}�̏d���`�F�b�N�p�G���A���N���A
						memset( &g_UseMediaData, 0, sizeof( t_UseMedia ) );
					}
				}
// GG129000(E) H.Fujinaga 2023/01/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j

// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//				// ���Z�ް�(���ޏ��)�̍ݎԏ��_�����̍쐬
//				taskchg(IDLETSKNO);
//				for( loop_cnt = 0; loop_cnt < ONL_MAX_DISC_NUM; loop_cnt++) {
//					pDisc1 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt];
//					pDisc2 = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[loop_cnt];
//					// �������̊������̂݃R�s�[����
//					if( pDisc1->DiscStatus ){	// �������łȂ�
//						continue;
//					}
//
//					pDisc1->DiscParkNo		= pDisc2->DiscParkNo;			// ���� ���ԏꇂ(0�`999999)
//					pDisc1->DiscSyu			= pDisc2->DiscSyu;				// ���� ���(0�`9999)
//					pDisc1->DiscCardNo		= pDisc2->DiscCardNo;			// ���� ���ދ敪(0�`65000)
//					pDisc1->DiscNo			= pDisc2->DiscNo;				// ���� �敪(0�`9)
//					pDisc1->DiscCount		= pDisc2->DiscCount;			// ���� ����(0�`99)
//					pDisc1->DiscInfo		= pDisc2->DiscInfo;				// ���� �������(0�`65000)
//					pDisc1->DiscCorrType	= pDisc2->DiscCorrType;			// ���� �Ή����ގ��(0�`65000)
//					pDisc1->DiscStatus		= pDisc2->DiscStatus;			// ���� �ð��(0�`9)
//					pDisc1->DiscFlg			= pDisc2->DiscFlg;				// ���� ������(0�`9)
//// MH810100(S) 2020/05/28 �Ԕԃ`�P�b�g���X(#4196)
////					// ���Ԋ����H
////					if( pDisc2->DiscSyu >= NTNET_SVS_T ){
////						pDisc1->Discount		= pDisc2->DiscountT;		// ���� ����(0�`999999)
////					}else{
////						pDisc1->Discount		= pDisc2->DiscountM;		// ���� ���z(0�`999999)
////					}
//// MH810100(S) 2020/07/29 �Ԕԃ`�P�b�g���X(#4561 �Z���^�[�����M������������ύX����)
////					pDisc1->Discount		= pDisc2->Discount;				// ���z/����(0�`999999)
//					pDisc1->Discount		= pDisc2->UsedDisc;				// ���z/����(0�`999999)
//// MH810100(E) 2020/07/29 �Ԕԃ`�P�b�g���X(#4561 �Z���^�[�����M������������ύX����)
////					pDisc1->UsedDisc		= pDisc2->UsedDisc;				// �����������p���̎g�p�ς݊����i���z/���ԁj(0�`999999)
//// MH810100(E) 2020/05/28 �Ԕԃ`�P�b�g���X(#4196)
//				}
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
			}
			// ID�s��v
			else {
				// �V�������ɏ��(���ޏ��)�Ƃ��Ĉ��킸���݂̐��Z��D�悷��̂ňȉ��ͺ��ı��
				// goto rsp_in_car_info_proc_NewData;
				// ���Z���~�v���̑��M
			}
			break;

		default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�o�[�R�[�h���
//[]----------------------------------------------------------------------[]
///	@param			pPrefix   : prefix
///					pMedia	  : �����}�̏��̃|�C���^
//[]----------------------------------------------------------------------[]
///	@return			TRUE : ��͐���
///					FALSE : ��͎��s
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
BOOL ope_ParseBarcodeData( tMediaDetail *pMedia )
{
	ushort	row_size = 0;
	lcdbm_rsp_QR_data_t *pQR = &lcdbm_rsp_QR_data_recv;

	// LCD�ŉ�͍ς݂̂���, ��̓f�[�^���R�s�[
	pMedia->Barcode.id = pQR->id;					// QR�R�[�hID
	pMedia->Barcode.rev = pQR->rev;					// QR�R�[�h�t�H�[�}�b�gRev.
	pMedia->Barcode.enc_type = pQR->enc_type;		// QR�R�[�h�G���R�[�h�^�C�v
	pMedia->Barcode.info_size = pQR->info_size;		// QR�f�[�^���(�p�[�X�f�[�^)�T�C�Y
	memcpy( &pMedia->Barcode.QR_data, &pQR->QR_data, sizeof(pMedia->Barcode.QR_data) );	// QR�p�[�X�f�[�^
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	pMedia->Barcode.qr_type = pQR->qr_type;		// QR�R�[�h�t�H�[�}�b�g�^�C�v
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j

	// ���f�[�^�R�s�[
	row_size = pQR->data_size;
	if( row_size > BAR_DATASIZE ){
		row_size = BAR_DATASIZE;
	}
	memcpy( pMedia->RowData, pQR->data, (size_t)row_size );
	pMedia->RowSize = row_size;

	return TRUE;
}

//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g�����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param			pData : �o�[�R�[�h���
//[]----------------------------------------------------------------------[]
///	@return			0=�����J�n�����O�C1=�L���������C2=�����I��������
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
//static uchar ope_CheckReceiptLimit( tBarcodeDetail* pData )
static uchar ope_CheckReceiptLimit( ushort id, QR_YMDData* pYmdData )
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
{
	ushort	startDay, endDay;			// �J�n��/�I����(normalize)
	ushort	validDay;
	ushort	BoundaryTime = 0;			// ���t�؊�����(normalize)
	ushort	BarMin = 0;
	uchar	ret = 1;
// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
//	QR_AmountInfo* pAmount = NULL;
//	QR_DiscountInfo* pDisc = NULL;
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
	struct clk_rec *pTime;

	// ���ݎ����̑���ɐ��Z�J�n�������g�p����
	pTime = GetPayStartTime();

	// QR���㌔
// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
//	if( pData->id == BAR_ID_AMOUNT ){
//		pAmount = &pData->QR_data.AmountType;
//
//		startDay = dnrmlzm( pAmount->IssueDate.Year, pAmount->IssueDate.Mon, pAmount->IssueDate.Day );	// �L���J�n��
	if( id == BAR_ID_AMOUNT ){
		startDay = dnrmlzm( pYmdData->IssueDate.Year, pYmdData->IssueDate.Mon, pYmdData->IssueDate.Day );	// �L���J�n��
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
		validDay = (ushort)prm_get(COM_PRM, S_SYS, 77, 2, 1);		// �L������
		if( validDay == 0 ){
			validDay = 99;
		}
		endDay = startDay + validDay - 1;																// �L���I����

		// ���t�؊������̎Z�o
		BoundaryTime = (ushort)prm_tim( COM_PRM, S_SYS, 76 );

// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
//		BarMin = (pAmount->IssueDate.Hour * 60) + pAmount->IssueDate.Min;
		BarMin = (pYmdData->IssueDate.Hour * 60) + pYmdData->IssueDate.Min;
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
		// �o�[�R�[�h���s�����t�؊����O�H
		if( BarMin < BoundaryTime ){
			// ���}�������t�؊����O�H
			if( pTime->nmin < BoundaryTime ){
				// ���t�؊����O�̃��V�[�g�́A���t�؊����O�͎g����
				;
			}
			// ���t�؊�����H
			else{
				// ���t�؊����O�̃��V�[�g�́A���t�؊�����͎g���Ȃ�(�L�������P���̏ꍇ)
				--startDay;
				--endDay;
			}
		}
		// �o�[�R�[�h���s�����t�؊�����H
		else{
			// ���}�������t�؊����O�H
			if( pTime->nmin < BoundaryTime ){
				++startDay;						// �J�n���ƏI�������{�P�����Ĕ͈̓`�F�b�N���s��
				++endDay;
			}
		}
	}
	// QR������
	else{
// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
//		pDisc = &pData->QR_data.DiscountType;
//
//		startDay = dnrmlzm( pDisc->StartDate.Year, pDisc->StartDate.Mon, pDisc->StartDate.Day );		// �L���J�n��
//		endDay   = dnrmlzm( pDisc->EndDate.Year, pDisc->EndDate.Mon, pDisc->EndDate.Day );				// �L���I����
		if( pYmdData->StartDate.Year == 0 && pYmdData->StartDate.Mon == 0 && pYmdData->StartDate.Day == 0 &&
			pYmdData->EndDate.Year == 99 && pYmdData->EndDate.Mon == 99 && pYmdData->EndDate.Day == 99 ){	// �������H

			// �L�������Ȃ��i�������j
			startDay = 0;
			endDay	 = 0xffff;
		}
		else{
			startDay = dnrmlzm( pYmdData->StartDate.Year, pYmdData->StartDate.Mon, pYmdData->StartDate.Day );	// �L���J�n��
			endDay   = dnrmlzm( pYmdData->EndDate.Year, pYmdData->EndDate.Mon, pYmdData->EndDate.Day );			// �L���I����
		}
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
	}

	// �͈�����
	if( pTime->ndat < startDay ) {		// �����O�i�Ώۓ����J�n���j
		ret = 0;
	}
	else if( endDay < pTime->ndat ) {	// ������i�I�������Ώۓ��j
		ret = 2;
	}

	return ret;
}
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j

//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g���s���`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param			pMedia : �����}�̏��
//[]----------------------------------------------------------------------[]
///	@return			TRUE : ���Ɏ�������
///					FALSE : ���Ɏ������O
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
static BOOL ope_CheckReceiptIssueDate( tBarcodeDetail* pData )
{
	ulong	BarTime;					// ���V�[�g���s��(normalize)
	ulong	InTime;						// ���Ɏ���(normalize)
	BOOL	bRet = FALSE;
	QR_AmountInfo* pAmount = &pData->QR_data.AmountType;
	QR_DiscountInfo* pDisc = &pData->QR_data.DiscountType;

	// ���������`�F�b�N
// MH810100(S) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4062 01-0078=�E0(���Ɏ����`�F�b�N����(��������))�������Ȃ�)
//	if( prm_get(COM_PRM, S_SYS, 78, 1, 1) ){
	if( prm_get(COM_PRM, S_SYS, 78, 1, 1) == 0 ){
// MH810100(E) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4062 01-0078=�E0(���Ɏ����`�F�b�N����(��������))�������Ȃ�)
		// QR���㌔
		if( pData->id == BAR_ID_AMOUNT ){
			// ���V�[�g���s��
			BarTime = dnrmlzm( pAmount->IssueDate.Year, pAmount->IssueDate.Mon, pAmount->IssueDate.Day );
			BarTime = (BarTime * T_DAY) + tnrmlz( 0, 0, pAmount->IssueDate.Hour, pAmount->IssueDate.Min );
		}
		// QR������
		else{
			// ���V�[�g���s��
			BarTime = dnrmlzm( pDisc->IssueDate.Year, pDisc->IssueDate.Mon, pDisc->IssueDate.Day );
			BarTime = (BarTime * T_DAY) + tnrmlz( 0, 0, pDisc->IssueDate.Hour, pDisc->IssueDate.Min );
		}

		// ���Ɏ���
		InTime = dnrmlzm( car_in_f.year, car_in_f.mon, car_in_f.day );
		InTime = (InTime * T_DAY) + tnrmlz(0, 0, car_in_f.hour, car_in_f.min);
	}
	// ���t�̂݃`�F�b�N
	else{
		// QR���㌔
		if( pData->id == BAR_ID_AMOUNT ){
			BarTime = dnrmlzm( pAmount->IssueDate.Year, pAmount->IssueDate.Mon, pAmount->IssueDate.Day );
		}
		// QR������
		else{
			BarTime = dnrmlzm( pDisc->IssueDate.Year, pDisc->IssueDate.Mon, pDisc->IssueDate.Day );
		}

		// ���ɔN����
		InTime = dnrmlzm( car_in_f.year, car_in_f.mon, car_in_f.day );
	}

	// ���Ɍ�ɔ��s�������V�[�g�H
	if( BarTime >= InTime ){
		bRet = TRUE;
	}

	return bRet;
}

// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
/*[]----------------------------------------------------------------------[]*/
/*| �e�튄�����Ƃ��Ẵ`�F�b�N                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ope_CanUseBarcode_sub( tBarcodeDetail* Barcode )        |*/
/*| PARAMETER    : tBarcodeDetail* Barcode : QR�ް��i�[�擪���ڽ           |*/
/*|              : QR_YMDData* pYmdData : QR���t�\���̐擪���ڽ            |*/
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
/*| Date         : 2020-05-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]*/
static short ope_CanUseBarcode_sub( tBarcodeDetail* Barcode, QR_YMDData* pYmdData )
{
	short	ret = 0;
	short	cardknd;
//	ushort	pkno_syu;
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	ushort	pkno_syu;
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
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
	ushort	std;			// Pass Start Day (BIN) 
	ushort	end;			// Pass End Day (BIN) 
	struct clk_rec		*pTime;	// ���Z�J�n����
	QR_DiscountInfo*	pDisc;	// QR�f�[�^��� QR������
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
	short	role;			// ����
	short	typeswitch;		// ��ʐ؊�
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)

	// ���ݎ����̑���ɐ��Z�J�n�������g�p����
	pTime = GetPayStartTime();

	// QR���㌔
	if( Barcode->id == BAR_ID_AMOUNT ){
		ret = 0;
	}
	// QR������
	else{
		pDisc = &Barcode->QR_data.DiscountType;		// QR�f�[�^��� QR�������̐擪�|�C���^

		if( chkdate( pDisc->StartDate.Year,
					(short)pDisc->StartDate.Mon,
					(short)pDisc->StartDate.Day ) ){	// �L���J�n������NG?
			return( 2 );					// �ް��ُ�
		}
		std = dnrmlzm( pDisc->StartDate.Year,
					  (short)pDisc->StartDate.Mon,
					  (short)pDisc->StartDate.Day );	// �L���J�n�����

		if( chkdate( pDisc->EndDate.Year,
					(short)pDisc->EndDate.Mon,
					(short)pDisc->EndDate.Day ) ){		// �L���J�n������NG?
			return( 2 );					// �ް��ُ�
		}
		end = dnrmlzm( pDisc->EndDate.Year,
					  (short)pDisc->EndDate.Mon,
					  (short)pDisc->EndDate.Day );		// �L���J�n�����

		ret = 0;
		for( ; ; ){
// GG124100(S) R.Endo 2022/08/03 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
				switch ( pDisc->DiscKind ) {	// �������
				case 1:		// �T�[�r�X���i���z�j
				case 101:	// �T�[�r�X���i���ԁj
					cardknd = 11;	// ���޽��
					break;
				case 2:		// �X�����i���z�j
				case 102:	// �X�����i���ԁj
				case 3:		// ���X�܊����i���z�j
				case 103:	// ���X�܊����i���ԁj
					cardknd = 12;	// �|����
					break;
				default:
					cardknd = 0;
					break;
				}
			} else {					// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/08/03 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			switch( pDisc->DiscKind ){			// �������
			case 1:		// �T�[�r�X���i���z�j
			case 101:	// �T�[�r�X���i���ԁj
				cardknd = 11;											// ���޽��
				break;
			case 2:		// �X�����i���z�j
			case 102:	// �X�����i���ԁj
				cardknd = 12;											// �|����
				break;
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
// 			case 4:		// �������i���z�j
// 			case 104:	// �������i���ԁj
// 				cardknd = 14;											// ������
// 				break;
			case 3:		// ���X�܊����i���z�j
			case 103:	// ���X�܊����i���ԁj
				if ( prm_get(COM_PRM, S_TAT, 1, 1, 1) == 1 ) {			// ���X�܊�������
					cardknd = 12;										// �|����
				} else {												// ���X�܊����Ȃ�
					ret = 13;											// ��ʋK��O
				}
				break;
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
			default:
				ret = 13;												// ��ʋK��O
				break;
			}
			if( ret ){
				break;													// �װ����
			}

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			if( 0L == pDisc->ParkingNo ){
				ret = 1;												// ���ԏꇂ�װ
				break;
			}

			if(( prm_get( COM_PRM, S_SYS, 71, 1, 6 ) == 1 )&&			// ��{�T�[�r�X���g�p��
			   ( CPrmSS[S_SYS][1] == pDisc->ParkingNo )){				// ��{���ԏꇂ?
//				pkno_syu = KIHON_PKNO;									// ��{
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
				pkno_syu = KIHON_PKNO;									// ��{
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			}
			else if(( prm_get( COM_PRM, S_SYS, 71, 1, 1 ) == 1 )&&		// �g��1�T�[�r�X���g�p��
					( CPrmSS[S_SYS][2] == pDisc->ParkingNo )){			// �g��1���ԏꇂ?
//				pkno_syu = KAKUCHOU_1;									// �g��1
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
				pkno_syu = KAKUCHOU_1;									// �g��1
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			}
			else if(( prm_get( COM_PRM, S_SYS, 71, 1, 2 ) == 1 )&&		// �g��2�T�[�r�X���g�p��
					( CPrmSS[S_SYS][3] == pDisc->ParkingNo )){			// �g��2���ԏꇂ?
//				pkno_syu = KAKUCHOU_2;									// �g��2
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
				pkno_syu = KAKUCHOU_2;									// �g��2
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			}
			else if(( prm_get( COM_PRM, S_SYS, 71, 1, 3 ) == 1 )&&		// �g��3�T�[�r�X���g�p��
					( CPrmSS[S_SYS][4] == pDisc->ParkingNo )){			// �g��3���ԏꇂ?
//				pkno_syu = KAKUCHOU_3;									// �g��3
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
				pkno_syu = KAKUCHOU_3;									// �g��3
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			}
			else{
				ret = 1;												// ���ԏꇂ�װ
				break;
			}

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			if ( !CLOUD_CALC_MODE ) {	// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

//			if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
//				if( CardSyuCntChk( pkno_syu, cardknd,
//					(short)pDisc->DiscClass, (short)pDisc->ShopNp, 0 ) ){	// 1���Z�̊�����ނ̌������ް?
//					ret = 14;											// ���x�������ް
//					break;
//				}
//			}
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			if ( prm_get(COM_PRM, S_DIS, 5, 1, 1) ) {	// ���ꊄ�����퐔�̐���(08-0005)�����Z���O�̐������z�����������󂯕t���Ȃ�
				if ( CardSyuCntChk(pkno_syu, cardknd, (short)pDisc->DiscClass, (short)pDisc->ShopNp, 0) ) {	// ���ꊄ�����퐔�I�[�o�[
					ret = 14;	// ���x�������ް
					break;
				}
			}
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			}
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			// �������ύX����
			chk_end1 = OFF;												// ���ް���v�׸ށFOFF

			for( tbl_no = 1 ; tbl_no <= 3 ; tbl_no++ ){					// �����ύXð��فi�P�`�R�j����

				chk_end2 = OFF;											// ��ʕs��v�׸ށFOFF

				tbl_syu = (short)CPrmSS[S_DIS][8+((tbl_no-1)*6)];		// �����ύXð��قɐݒ肳��Ă����ʂ��擾
				tbl_data= (short)CPrmSS[S_DIS][9+((tbl_no-1)*6)];		// �����ύXð��قɐݒ肳��Ă�����e���擾

				// ���������
				switch( tbl_syu ){										// �ݒ肳��Ă����ʂƌ��ް��̎�ʂ��r
				case	1:												// �ݒ��ʁ��T�[�r�X��
					if( cardknd != 11 ){								// ���ް���ʁ��T�[�r�X���H
						chk_end2 = ON;									// NO �� ��ʕs��v
					}
					else{
						if( tbl_data != 0 ){							// �ݒ���e���O�i�T�[�r�X���S�āj�ȊO�H
							if( tbl_data != (short)pDisc->DiscClass ){	// �T�[�r�X�����(A�`C)��v�H
								chk_end2 = ON;							// NO �� ��ʕs��v
							}
						}
					}
					break;
				case	2:												// �ݒ��ʁ��|����
					if( cardknd != 12 ){								// ���ް���ʁ��|�����H
						chk_end2 = ON;									// NO �� ��ʕs��v
					}
					else{
						if( tbl_data != 0 ){							// �ݒ���e���O�i�|�����S�āj�ȊO�H
							if( tbl_data != (short)pDisc->ShopNp ){		// �XNo.��v�H
								chk_end2 = ON;							// NO �� ��ʕs��v
							}
						}
					}
					break;
				case	3:												// �ݒ��ʁ�������
					if( cardknd != 14 ){								// ���ް���ʁ��������H
						chk_end2 = ON;									// NO �� ��ʕs��v
					}
					else{
						if( tbl_data != 0 ){							// �ݒ���e���O�i�������S�āj�ȊO�H
							if( tbl_data != (short)pDisc->ShopNp ){		// �XNo.��v�H
								chk_end2 = ON;							// NO �� ��ʕs��v
							}
						}
					}
					break;
				case	4:												// �ݒ��ʁ��S��
					break;												// �T�[�r�X���A�|�����A�������S�Ĉ�v�Ɣ��f
				case	0:												// �ݒ��ʁ��Ȃ�
				default:												// �ݒ��ʁ����̑�
					chk_end2 = ON;										// �� ��ʕs��v
					break;
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

				if( (std == tbl_sdate) && (end == tbl_edate) ){			// �J�n�����I������v�H
					chk_end1 = ON;										// YES �� ���ް���v
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

						tbl_sdate = dnrmlzm( tbl_syear, tbl_smon, tbl_sday );	// �L���J�n���ϊ�
						tbl_edate = dnrmlzm( tbl_eyear, tbl_emon, tbl_eday );	// �L���I�����ϊ�
					}
					std = tbl_sdate;											// �L���J�n����ύX��̊J�n���Ƃ���
					end = tbl_edate;											// �L���I������ύX��̏I�����Ƃ���

					pYmdData->StartDate.Year = tbl_syear;						// QR�L���J�n�N
					pYmdData->StartDate.Mon  = tbl_smon;						// QR�L���J�n��
					pYmdData->StartDate.Day  = tbl_sday;						// QR�L���J�n��

					pYmdData->EndDate.Year = tbl_eyear;							// QR�L���I���N
					pYmdData->EndDate.Mon  = tbl_emon;							// QR�L���I����
					pYmdData->EndDate.Day  = tbl_eday;							// QR�L���I����

					break;														// �� �����ύXð��فi�P�`�R�j�����I��
				}
			}

			if( std > pTime->ndat ){											// �����O
				ret = 6;														// �����O�װ
				break;
			}
			if( end < pTime->ndat ){											// �����؂�
				ret = 3;														// �����؂�װ
				break;
			}

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			if ( !CLOUD_CALC_MODE ) {	// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			if( cardknd == 12 ){
				/*** �|���� ***/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
// 				if( !rangechk( 1, 100, (short)pDisc->ShopNp ) ) {				// �X���͈͊O?
// 					ret = 13;													// ��ʋK��O
// 					break;
// 				}
// 				if (( CPrmSS[S_STO][1+3*((short)pDisc->ShopNp-1)] == 0L )&&
// 					( CPrmSS[S_STO][3+3*((short)pDisc->ShopNp-1)] == 0L ))
// 				{
				// ��ʋK��O(13)�Ɨ�����ʴװ(27)�̃`�F�b�N
				ret = CheckDiscount(pDisc->DiscKind, pDisc->ShopNp, pDisc->DiscClass);
				if ( ret ) {
					break;
				}

				// �������ݒ�(26)�̃`�F�b�N
				if ( (pDisc->DiscKind == NTNET_TKAK_M) || 						// ���X�܊����i���z�j
					 (pDisc->DiscKind == NTNET_TKAK_T) ) {						// ���X�܊����i���ԁj
					role = (short)prm_get(COM_PRM, S_TAT, (pDisc->DiscClass * 2), 1, 1);
				} else {														// �X����
					role = (short)prm_get(COM_PRM, S_STO, ((pDisc->ShopNp * 3) - 2), 1, 1);
				}
				typeswitch = GetTypeSwitch(pDisc->DiscKind, pDisc->ShopNp);
				if ( !role && !typeswitch ) {									// ����/��ʐؑւȂ�
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
					ret = 26;													// �������ݒ�
					break;
				}
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
// 				if( CPrmSS[S_STO][3+3*((short)pDisc->ShopNp-1)] ){				// ��ʐؑ֗L��?
// 					if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*((short)pDisc->ShopNp-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
// 						ret = 27;												// ������ʴװ
// 						break;
// 					}
// // MH810100(S) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// // // MH810100(S) 2020/06/19 #4503�y�A���]���w�E�����z�T�[�r�X��QR�K�p��̎�ʐؑւ̔F�؎���f�[�^�����M����Ă��Ȃ�
// // 					if(( ryo_buf.nyukin )||( ryo_buf.waribik )||			// �����ς�? or �����ς�?
// // 					   ( e_incnt > 0 )||									// or �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
// // 					   ( c_pay )) {											// or ����߲�޶��ގg�p����?
// // 						ret = 30;											// ���Z�����װ
// // 						break;
// // 					}
// // // MH810100(E) 2020/06/19 #4503�y�A���]���w�E�����z�T�[�r�X��QR�K�p��̎�ʐؑւ̔F�؎���f�[�^�����M����Ă��Ȃ�
// // MH810100(E) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// 				}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
//					if( CPrmSS[S_STO][1+3*((short)pDisc->ShopNp-1)] == 1L ){	// ���Ԋ���?
//						if( card_use[USE_PPC] || card_use[USE_NUM] ||			// ����߲�޶��� or �񐔌��g�p�ς�?
//							(e_incnt > 0))										// or �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
//						{
//							ret = 30;											// ���Z�����װ
//							break;
//						}
//					}
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
// 			}else if( cardknd == 14 ){
// 				/*** ������ ***/
// 				if(( CPrmSS[S_WAR][1] == 0 )||									// �������g�p���Ȃ��ݒ�?
// 				   ( !rangechk( 1, 100, (short)pDisc->DiscClass ) )||			// ������ʔ͈͊O?
// 				   (( CPrmSS[S_WAR][2+3*((short)pDisc->DiscClass-1)] == 0L )&&	// ���ݒ�
// 				    ( CPrmSS[S_WAR][4+3*((short)pDisc->DiscClass-1)] == 0L )))
// 				{
// 					ret = 13;													// ��ʋK��O
// 					break;
// 				}
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
			}else{
				/*** ���޽�� ***/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
// 				if( !rangechk( 1, SVS_MAX, (short)pDisc->DiscClass ) ){			// ���޽���͈͊O?
// 					ret = 13;													// ��ʋK��O
// 					break;
// 				}
// 				if (( CPrmSS[S_SER][1+3*((short)pDisc->DiscClass-1)] == 0L )&&
// 					( CPrmSS[S_SER][3+3*((short)pDisc->DiscClass-1)] == 0L ))
// 				{
				// ��ʋK��O(13)�Ɨ�����ʴװ(27)�̃`�F�b�N
// MH810104(S) R.Endo 2021/09/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6005 �y�A�������zQR�������̃T�[�r�X���œX�ԍ����u0�v�̏ꍇ�ɗ��p�ł��Ȃ�
// 				ret = CheckDiscount(pDisc->DiscKind, pDisc->ShopNp, pDisc->DiscClass);
				// �T�[�r�X���͊����敪���J�[�h�敪�A�X�ԍ����������ɓ���B
				ret = CheckDiscount(pDisc->DiscKind, pDisc->DiscClass, pDisc->ShopNp);
// MH810104(E) R.Endo 2021/09/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6005 �y�A�������zQR�������̃T�[�r�X���œX�ԍ����u0�v�̏ꍇ�ɗ��p�ł��Ȃ�
				if ( ret ) {
					break;
				}

				// �������ݒ�(26)�̃`�F�b�N
				role = (short)prm_get(COM_PRM, S_SER, ((pDisc->DiscClass * 3) - 2), 1, 1);
// MH810104(S) R.Endo 2021/10/06 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6067 �y�A�������z�T�[�r�X�������i��ʐ؊��j��QR�����������p�ł��Ȃ�
// 				typeswitch = GetTypeSwitch(pDisc->DiscKind, pDisc->ShopNp);
				typeswitch = GetTypeSwitch(pDisc->DiscKind, pDisc->DiscClass);
// MH810104(E) R.Endo 2021/10/06 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6067 �y�A�������z�T�[�r�X�������i��ʐ؊��j��QR�����������p�ł��Ȃ�
				if ( !role && !typeswitch ) {									// ����/��ʐؑւȂ�
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
					ret = 26;													// �������ݒ�
					break;
				}
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
// 				if( CPrmSS[S_SER][3+3*((short)pDisc->DiscClass-1)] ){			// ��ʐؑ֗L��?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*((short)pDisc->DiscClass-1)]-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�?
// 						ret = 27;												// ��ʐؑւȂ�
// 						break;
// 					}
// // MH810100(S) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// // // MH810100(S) 2020/06/19 #4503�y�A���]���w�E�����z�T�[�r�X��QR�K�p��̎�ʐؑւ̔F�؎���f�[�^�����M����Ă��Ȃ�
// // 					if(( ryo_buf.nyukin )||( ryo_buf.waribik )||				// �����ς�? or �����ς�?
// // 					   ( e_incnt > 0 )||										// or �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
// // 					   ( c_pay )) {												// or ����߲�޶��ގg�p����?
// // 						ret = 30;												// ���Z�����װ
// // 						break;
// // 					}
// // // MH810100(E) 2020/06/19 #4503�y�A���]���w�E�����z�T�[�r�X��QR�K�p��̎�ʐؑւ̔F�؎���f�[�^�����M����Ă��Ȃ�
// // MH810100(E) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// 				}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�e�튄�����`�F�b�N�̕ύX)
//				if( CPrmSS[S_SER][1+3*((short)pDisc->DiscClass-1)] == 1L ){		// ���Ԋ���?
//					if( card_use[USE_PPC] || card_use[USE_NUM] ||				// ����߲�޶��� or �񐔌��g�p�ς�?
//						(e_incnt > 0))											// or �d�q�}�l�[�g�p����?�i�����_�ł͕s�v�j
//					{
//						ret = 30;												// ���Z�����װ
//						break;
//					}
//				}
				if( ryo_buf.syubet < 6 ){
					// ���Z�Ώۂ̗������A�`F(0�`5)
					data_adr = 2*((short)pDisc->DiscClass-1)+76;				// �g�p�\������ʂ��ް����ڽ�擾
					data_pos = (char)(6-ryo_buf.syubet);						// �g�p�\������ʂ��ް��ʒu�擾
				}
				else{
					// ���Z�Ώۂ̗������G�`L(6�`11)
					data_adr = 2*((short)pDisc->DiscClass-1)+77;				// �g�p�\������ʂ��ް����ڽ�擾
					data_pos = (char)(12-ryo_buf.syubet);						// �g�p�\������ʂ��ް��ʒu�擾
				}
				if( prm_get( COM_PRM, S_SER, data_adr, 1, data_pos ) ){			// �g�p�s�ݒ�H
					ret = 25;													// ���̎Ԏ�̌�
					break;
				}
			}

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			break;
		}
	}

	return( ret );
}
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j

//[]----------------------------------------------------------------------[]
///	@brief			�o�[�R�[�h�\�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@return			RESULT_NO_ERROR : �g�p�\�ȃo�[�R�[�h
///					RESULT_QR_INQUIRYING�F�⍇����
///					RESULT_DISCOUNT_TIME_MAX : �������
///					RESULT_BAR_USED : �g�p�ς̃o�[�R�[�h
//					RESULT_BAR_EXPIRED: �����؂�o�[�R�[�h
///					RESULT_BAR_READ_MAX : �o�[�R�[�h�������
///					RESULT_BAR_FORMAT_ERR�F�t�H�[�}�b�g�G���[
///					RESULT_BAR_ID_ERR�F�ΏۊO
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
static OPE_RESULT ope_CanUseBarcode( tMediaDetail *pMedia )
{
	ushort	index = 0;
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// 	ulong	alm_info = 0;
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
	QR_AmountInfo* pAmount = NULL;
	QR_DiscountInfo* pDisc = NULL;
	QR_YMDData	YmdData;
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j

	// IC�N���W�b�g�ݒ肠��
// MH810103(s) �d�q�}�l�[�Ή�
//	if (prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1 ) {
	if(isEC_USE()) {
// MH810103(e) �d�q�}�l�[�Ή�
		// IC�N���W�b�g�⍇����
		if( OPECTL.InquiryFlg == 1 ){
			return RESULT_QR_INQUIRYING;		// �⍇����
		}
	}

	// 1�F�ؑ��쒆�̎g�p�σo�[�R�[�h�`�F�b�N
	for( index = 0; index < g_UseMediaData.bar_count; ++index ){
		WACDOG;
		if( 0 == memcmp(pMedia->RowData, &g_UseMediaData.data[index].RowData, sizeof(pMedia->RowData)) ){
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// 			alm_info = 1;	// 1�F�ؒ���2�񓯂��o�[�R�[�h��ǎ�肷��
// 			alm_chk2( ALMMDL_AUTH, ALARM_USED_RECEIPT, 2, 2, 1, (void *)&alm_info );	// �A���[���o�^(A0530)
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			return RESULT_BAR_USED;				// �g�p�ς̃o�[�R�[�h
		}
	}

	// ���V�[�g�����`�F�b�N
// MH810103(S) R.Endo 2021/05/31 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5669 QR���㌔���V�[�g�Ǎ��ݖ�������̔���s�
//	if( 0 != prm_get(COM_PRM, S_SYS, 75, 2, 1) && g_UseMediaData.bar_count >= prm_get(COM_PRM, S_SYS, 75, 2, 1) || g_UseMediaData.bar_count >= BARCODE_USE_MAX ){
	if ( (g_UseMediaData.bar_count >= BARCODE_USE_MAX) || (
		 (pMedia->Barcode.id == BAR_ID_AMOUNT) &&
		 (0 != prm_get(COM_PRM, S_SYS, 75, 2, 1)) &&
		 (g_UseMediaData.amount_count >= prm_get(COM_PRM, S_SYS, 75, 2, 1))) ) {
// MH810103(E) R.Endo 2021/05/31 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5669 QR���㌔���V�[�g�Ǎ��ݖ�������̔���s�
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// 		alm_info = 6;		// �Œ�
// 		alm_chk2( ALMMDL_AUTH, ALARM_USE_COUNT_OVER, 2, 2, 1, (void *)&alm_info );		// �A���[���o�^(A0533)
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
		return RESULT_BAR_READ_MAX;				// ���ɏ��
	}

	// ��Ή��o�[�R�[�h
	if( pMedia->Barcode.id != BAR_ID_AMOUNT && pMedia->Barcode.id != BAR_ID_DISCOUNT ){
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// 		alm_info = 1;		// �t�H�[�}�b�g�p�^�[���Ɉ�v���Ȃ��o�[�R�[�h��ǎ�肷��i
// 		alm_chk2( ALMMDL_AUTH, ALARM_NOT_USE_RECEIPT, 2, 2, 1, (void *)&alm_info );		// �A���[���o�^(A0531)
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
		return RESULT_BAR_ID_ERR;
	}

// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
	// �e�튄�����Ƃ��Ẵ`�F�b�N
	if( pMedia->Barcode.id == BAR_ID_AMOUNT ){
		// QR���㌔
		pAmount = &pMedia->Barcode.QR_data.AmountType;
		memcpy( &YmdData.IssueDate, &pAmount->IssueDate, sizeof(DATE_YMDHMS) );	// QR���s�N���������b
		memset( &YmdData.StartDate, 0, sizeof(DATE_YMD) );						// QR�L���J�n�N����
		memset( &YmdData.EndDate  , 0, sizeof(DATE_YMD) );						// QR�L���I���N����
	}else{
		// QR������
		pDisc = &pMedia->Barcode.QR_data.DiscountType;
		memset( &YmdData.IssueDate, 0, sizeof(DATE_YMDHMS) );					// QR���s�N���������b
		memcpy( &YmdData.StartDate, &pDisc->StartDate, sizeof(DATE_YMD) );		// QR�L���J�n�N����
		memcpy( &YmdData.EndDate  , &pDisc->EndDate  , sizeof(DATE_YMD) );		// QR�L���I���N����
	}
	switch( ope_CanUseBarcode_sub( &pMedia->Barcode, &YmdData ) ){
	case  0:	// OK
	default:
		break;
	case  1:	// ���ԏꇂ�װ
	case  2:	// �ް��ُ�
	case 13:	// ��ʋK��O
	case 25:	// �Ԏ�װ
	case 26:	// �ݒ�װ
	case 27:	// ������ʴװ
		return RESULT_BAR_ID_ERR;				// �ΏۊO
		break;
	case  3:	// �����؂�
	case  6:	// �����O
//		return RESULT_BAR_EXPIRED;				// �L�������O
		break;
//	case 14:	// ���x�������ް
//	case 28:	// ���x����0�װ
//		return RESULT_DISCOUNT_TIME_MAX;		// �������
//		break;
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	case 14:	// ���x�������ް
		return RESULT_DISCOUNT_TIME_MAX;		// �������
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(S) 2020/06/19 #4503�y�A���]���w�E�����z�T�[�r�X��QR�K�p��̎�ʐؑւ̔F�؎���f�[�^�����M����Ă��Ȃ�
	case 30:	// ���Z�����װ
		return RESULT_BAR_NOT_USE_CHANGEKIND;
		break;
// MH810100(E) 2020/06/19 #4503�y�A���]���w�E�����z�T�[�r�X��QR�K�p��̎�ʐؑւ̔F�؎���f�[�^�����M����Ă��Ȃ�
	}
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	if ( !CLOUD_CALC_MODE ) {	// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

	// �����؂�`�F�b�N
// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
//	if( 1 != ope_CheckReceiptLimit(&pMedia->Barcode) ){
	if( 1 != ope_CheckReceiptLimit( pMedia->Barcode.id, &YmdData ) ){
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// 		alm_info = 1;		// �L�������؂�̃o�[�R�[�h��ǎ�肷��
// 		alm_chk2( ALMMDL_AUTH, ALARM_EXPIRED_RECEIPT, 2, 2, 1, (void *)&alm_info );		// �A���[���o�^(A0532)
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
		return RESULT_BAR_EXPIRED;				// �L�������O
	}

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	}
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	// ���s���`�F�b�N
	if( 0 == prm_get(COM_PRM, S_SYS, 78, 1, 1) || 2 == prm_get(COM_PRM, S_SYS, 78, 1, 1) ){ 	// ���Ɏ����`�F�b�N����
		if( FALSE == ope_CheckReceiptIssueDate(&pMedia->Barcode) ){
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// 			alm_info = 2;	// ���Ɏ��������O�ɔ��s���ꂽ�o�[�R�[�h��ǎ�肷��
// 			alm_chk2( ALMMDL_AUTH, ALARM_EXPIRED_RECEIPT, 2, 2, 1, (void *)&alm_info );	// �A���[���o�^(A0532)
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			return RESULT_BAR_EXPIRED;			// �L�������O
		}
	}

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
		// QR�f�[�^�`�F�b�N
		return cal_cloud_qrdata_check(pMedia, &YmdData);
	}
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

	return RESULT_NO_ERROR;
}

// GG129000(S) H.Fujinaga 2023/02/15 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
//[]----------------------------------------------------------------------[]
///	@brief			QR���Ԍ��\�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@return			0 : �g�p�\
///					1 : �g�p�s��
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
static ushort ope_CanUseQRTicket( tMediaDetail *pMedia )
{
	long prm;
	ulong QrTime_all;
	ulong NowTime_all;
	struct clk_rec	QrTime;					// ���s����(normalize)
	struct clk_rec	NowTime;				// ���ݎ���(normalize)
	QR_TicketInfo* pTicket = &pMedia->Barcode.QR_data.TicketType;

	// ���ԏ�No = ��{���ԏ�No�H
	prm = CPrmSS[S_SYS][1];
	if(pTicket->ParkingNo != prm){
		return 1;
	}

	// ���Ɏ��� <= ���ݓ����H
	// ��������(���Ɏ���)
	QrTime.year = pTicket->IssueDate.Year;
	QrTime.mont = (short)pTicket->IssueDate.Mon;
	QrTime.date = (short)pTicket->IssueDate.Day;
	QrTime.hour = (short)pTicket->IssueDate.Hour;
	QrTime.minu = (short)pTicket->IssueDate.Min;
	QrTime.seco = (short)pTicket->IssueDate.Sec;
	QrTime.week	= 0;		// Day   (0:SUN-6:SAT)
	QrTime.ndat	= 0;		// Normalize Date
	QrTime.nmin	= 0;		// Normalize Minute
	QrTime_all = c_Normalize_sec(&QrTime);

	// ���ݎ���
	NowTime.year = CLK_REC.year;
	NowTime.mont = (short)CLK_REC.mont;
	NowTime.date = (short)CLK_REC.date;
	NowTime.hour = (short)CLK_REC.hour;
	NowTime.minu = (short)CLK_REC.minu;
	NowTime.seco = (short)CLK_REC.seco;
	NowTime.week = 0;		// Day   (0:SUN-6:SAT)
	NowTime.ndat = 0;		// Normalize Date
	NowTime.nmin = 0;		// Normalize Minute
	NowTime_all = c_Normalize_sec(&NowTime);

	if(QrTime_all > NowTime_all){
		return 1;
	}

	// ���Ɏ��������ݓ�����1�N�O�H
	// ��������(���Ɏ���+1�N)
	QrTime.year = (short)(pTicket->IssueDate.Year + 1);
	QrTime.mont = (short)pTicket->IssueDate.Mon;
	QrTime.date = (short)pTicket->IssueDate.Day;
	QrTime.hour = (short)pTicket->IssueDate.Hour;
	QrTime.minu = (short)pTicket->IssueDate.Min;
	QrTime.seco = (short)pTicket->IssueDate.Sec;
	QrTime.week	= 0;		// Day   (0:SUN-6:SAT)
	QrTime.ndat	= 0;		// Normalize Date
	QrTime.nmin	= 0;		// Normalize Minute
	QrTime_all = c_Normalize_sec(&QrTime);

	if(QrTime_all <= NowTime_all){
		return 1;
	}

	return 0;
}
// GG129000(E) H.Fujinaga 2023/02/15 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j

//[]----------------------------------------------------------------------[]
///	@brief			�g�p�ϊ����}�̓o�^
//[]----------------------------------------------------------------------[]
///	@return			TRUE : �o�^����
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
BOOL ope_AddUseMedia( tMediaDetail* pMedia )
{
	// �g�p�ϊ����}�̓o�^
	memcpy( &g_UseMediaData.data[g_UseMediaData.bar_count], pMedia, sizeof(g_UseMediaData.data[g_UseMediaData.bar_count]) );
	++g_UseMediaData.bar_count;

// MH810103(S) R.Endo 2021/05/31 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5669 QR���㌔���V�[�g�Ǎ��ݖ�������̔���s�
	if( pMedia->Barcode.id == BAR_ID_AMOUNT ) {
		++g_UseMediaData.amount_count;
	}
// MH810103(E) R.Endo 2021/05/31 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5669 QR���㌔���V�[�g�Ǎ��ݖ�������̔���s�

	return TRUE;
}

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//[]----------------------------------------------------------------------[]
///	@brief			���[�����j�^�f�[�^�o�^
//[]----------------------------------------------------------------------[]
///	@param			usStsNo	�F��Ԏ�ʁE�R�[�h
//[]----------------------------------------------------------------------[]
///	@return			TRUE : �o�^����
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
BOOL ope_MakeLaneLog( ushort usStsNo )
{
	memset( &DC_LANE_work, 0, sizeof(DC_LANE_work) );

	// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	DC_LANE_work.ID = op_GetSequenceID();

	// �Z���^�[�ǔ�
	DC_LANE_work.CenterOiban = DC_GetCenterSeqNo( DC_SEQNO_LANE );

	// �Z���^�[�ǔԕs���t���O
	if( DC_LANE_work.CenterOiban == 0 ){
		DC_UpdateCenterSeqNo( DC_SEQNO_LANE );
		DC_LANE_work.CenterOiban = 1;
		DC_LANE_work.CenterOibanFusei = 1;
	}

	// ���[�����j�^���Z�b�g ---------------------------------------------------------------
	// �t�H�[�}�b�gRev.��
	DC_LANE_work.LANE_Info.FormRev = 1;

	// ��������
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(�s�v�ȃL���X�g���C��)�iGM803000���p�j
//	memcpy( &DC_LANE_work.LANE_Info.ProcessDate, (date_time_rec2*)&CLK_REC, sizeof(DC_LANE_work.LANE_Info.ProcessDate) );
	memcpy( &DC_LANE_work.LANE_Info.ProcessDate, &CLK_REC, sizeof(DC_LANE_work.LANE_Info.ProcessDate) );
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(�s�v�ȃL���X�g���C��)�iGM803000���p�j

	// ��Ԉ�A�ԍ�
	DC_LANE_work.LANE_Info.StsSeqNo = LaneStsSeqNo;

	// �}�� ���ԏꇂ�A��ʁA�ԍ��A�������
	memcpy(&DC_LANE_work.LANE_Info.MediaParkNo, &m_stLaneWork, (size_t)offsetof(struct stLaneDataInfo, FreeNum));

	// ��Ԏ��
	DC_LANE_work.LANE_Info.StsSyu = (uchar)(usStsNo / 100);

	// ��ԃR�[�h
	DC_LANE_work.LANE_Info.StsCode = (uchar)(usStsNo % 100);

	// �t���[���l
	DC_LANE_work.LANE_Info.FreeNum = m_stLaneWork.FreeNum;

	// �t���[����
	memcpy(DC_LANE_work.LANE_Info.FreeStr, m_stLaneWork.FreeStr, sizeof(m_stLaneWork.FreeStr));

	// ��Ԗ��A��ԃ��b�Z�[�W�͐��Z�@�ł̓Z�b�g���Ȃ�

	Log_regist( LOG_DC_LANE );				// ���O�o�^
	DC_UpdateCenterSeqNo( DC_SEQNO_LANE );	// �Z���^�[�ǔԂ��J�E���g�A�b�v
// GG129000(S) M.Fujikawa 2023/11/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7211�Ή�
	DC_PopCenterSeqNo( DC_SEQNO_LANE );	// �Z���^�[�ǔԂ�Pop
// GG129000(S) M.Fujikawa 2023/11/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7211�Ή�

	return TRUE;
}
//[]----------------------------------------------------------------------[]
///	@brief			���[�����j�^�f�[�^�̔}�̃Z�b�g
//[]----------------------------------------------------------------------[]
/// @param		ulParkNo	�F���ԏ�No.
/// 			usMediaKind	�F�}�̎��
/// 			pMediaNo	�F�}�̔ԍ��i32���j
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void SetLaneMedia(ulong ulParkNo, ushort usMediaKind, uchar *pMediaNo)
{
	// ���ԏ�No.
	m_stLaneWork.MediaParkNo = ulParkNo;

	// �}�̎��
	m_stLaneWork.Mediasyu = usMediaKind;

	// �}�̔ԍ�
	memset(m_stLaneWork.MediaNo, 0, sizeof(m_stLaneWork.MediaNo));
	if (pMediaNo != NULL) {
		memcpy(m_stLaneWork.MediaNo, pMediaNo, sizeof(m_stLaneWork.MediaNo));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���[�����j�^�f�[�^�̗�����ʃZ�b�g
//[]----------------------------------------------------------------------[]
/// @param		usFeeKind	:�������
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void SetLaneFeeKind(ushort usFeeKind)
{
	// �������
	m_stLaneWork.PaySyu = usFeeKind;
}

//[]----------------------------------------------------------------------[]
///	@brief			���[�����j�^�f�[�^�̃t���[���l�Z�b�g
//[]----------------------------------------------------------------------[]
/// @param		ulNum	:�t���[���l
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void SetLaneFreeNum(ulong ulNum)
{
	// �t���[���l
	m_stLaneWork.FreeNum = ulNum;
}

//[]----------------------------------------------------------------------[]
///	@brief			���[�����j�^�f�[�^�̃t���[�����Z�b�g
//[]----------------------------------------------------------------------[]
/// @param		pStr		:�t���[�����i60byte�j
/// 			usStrSize	:�t���[�����T�C�Y
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void SetLaneFreeStr(uchar *pStr, uchar usStrSize)
{
	// �t���[����
	memset(m_stLaneWork.FreeStr, 0, sizeof(m_stLaneWork.FreeStr));
	if (pStr != NULL) {
		memcpy(m_stLaneWork.FreeStr, pStr, (size_t)usStrSize);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			��Ԉ�A�ԍ��X�V
//[]----------------------------------------------------------------------[]
/// @param		none
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void LaneStsSeqNoUpdate(void)
{
// GG129000(S) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
	if(OPECTL.f_req_paystop == 1){
//		OPECTL.f_req_paystop = 0;
		;	// �������Ȃ�
	}else{
// GG129000(E) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
	// ��Ԉ�A�ԍ��C���N�������g
	LaneStsSeqNo++;
	if( LaneStsSeqNo > 999999999L ){
		LaneStsSeqNo = 0;
	}
	memset( &m_stLaneWork, 0, sizeof(m_stLaneWork) );
// GG129000(S) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
	}
// GG129000(E) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
// GG129000(S) M.Fujikawa 2023/11/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7211�Ή�
	DC_PushCenterSeqNo( DC_SEQNO_LANE );	// �Z���^�[�ǔԂ�Push
// GG129000(S) M.Fujikawa 2023/11/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7211�Ή�
}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129000(S) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
//[]----------------------------------------------------------------------[]
///	@brief			���[�����j�^�o�^����(���Z��(�����ς�))
//[]----------------------------------------------------------------------[]
/// @param		status		:���Z�X�e�[�^�X
//[]----------------------------------------------------------------------[]
///	@return		TRUE		:�o�^����
///				FALSE		:�o�^���Ȃ�
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
BOOL ope_MakeLaneLog_Check( uchar pay_sts )
{
	if( pay_sts == 0 ){
		// ���Z��
		if( OPECTL.op_faz == 1 && !cal_cloud_discount_check_only() ){
			// 1:������(�����L�莞�ɾ��) ���� ���ɏ�񐳏�
			return TRUE;
		}
	}else if( pay_sts == 1 ){
		// ���Z����
		if( OPECTL.op_faz == 2 && OPECTL.Ope_mod == 3 ){
			// 2:���Z����(�����s���M�㾯�) ���� mod03
			return TRUE;
		}
	}
	return FALSE;
}
// GG129000(E) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j

//[]----------------------------------------------------------------------[]
///	@brief			�F�؃��O�쐬
//[]----------------------------------------------------------------------[]
///	@param			pTicketData : ���Ԍ����
///					pMedia : �����}�̏��
///					flg : �ŏI�t���O
///					kind : �����敪(0=�m��A1=���)
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void ope_MakeCertifLog( tMediaDetail *pMedia, uchar flg, eCertifKind kind )
{
	ushort cnt = 0, disc_cnt = 0;
	stDiscount_t* pDisc = NULL;

	memset( &DC_QR_work, 0, sizeof(DC_QR_work) );

	DC_QR_work.ID = op_GetSequenceID();							// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	DC_QR_work.CenterOiban = DC_GetCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔ�
	if( DC_QR_work.CenterOiban == 0 ){
		DC_UpdateCenterSeqNo( DC_SEQNO_QR );
		DC_QR_work.CenterOiban = 1;
		DC_QR_work.CenterOibanFusei = 1;						// �Z���^�[�ǔԕs���t���O
	}
	DC_QR_work.QR_Info_Rev = 1;									// QR���Rev.��

	// QR���Z�b�g ---------------------------------------------------------------
	DC_QR_work.QR_Info.FormRev = 1;								// �t�H�[�}�b�gRev.��
	memcpy( &DC_QR_work.QR_Info.CertDate, &g_UseMediaData.Certif_tim, sizeof(DC_QR_work.QR_Info.CertDate) );	// �F�؊m�莞��
	DC_QR_work.QR_Info.ProcessKind = kind;						// �����敪(0=�m��A1=���)
	// ���ɔ}��
// MH810100(S) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j
//// MH810100(S) 2020/06/12 #4227 �y�A���]���w�E�����zDC-NET�iDataID�F1101�@�����F�ؓo�^�E����v���f�[�^�j�̃J�[�h���̓��A���^�C���ʐM�̐��Z�}�̂Ɠ����ɂ��Ăق���
////	DC_QR_work.QR_Info.in_media.ParkingNo = lcdbm_rsp_in_car_info_main.crd_info.AskMediaParkNo;		// �⍇���}�� ���ԏꇂ		0�`999999
////	DC_QR_work.QR_Info.in_media.Kind = lcdbm_rsp_in_car_info_main.crd_info.AskMediaKind;			// �⍇���}�� ���			0�`65000
////	memcpy( DC_QR_work.QR_Info.in_media.CardNo, lcdbm_rsp_in_car_info_main.crd_info.byAskMediaNo, sizeof(DC_QR_work.QR_Info.in_media.CardNo) );
////																									// �⍇���}�� �ԍ�			�yASCII�z32���i���l�A�c���Null�Œ�j���{�����101�Ȃ�@�B��+���������A���ꂪ����H
//
//	// �⍇�������̊������擾���ʂ́u���Z�}�́v���Z�b�g����
//	DC_QR_work.QR_Info.in_media.ParkingNo = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;		// �⍇���}�� ���ԏꇂ		0�`999999
//	DC_QR_work.QR_Info.in_media.Kind = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;			// �⍇���}�� ���			0�`65000
//	memcpy( DC_QR_work.QR_Info.in_media.CardNo, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo, sizeof(DC_QR_work.QR_Info.in_media.CardNo) );
//																									// �⍇���}�� �ԍ�			�yASCII�z32���i���l�A�c���Null�Œ�j���{�����101�Ȃ�@�B��+���������A���ꂪ����H
//// MH810100(E) 2020/06/12 #4227 �y�A���]���w�E�����zDC-NET�iDataID�F1101�@�����F�ؓo�^�E����v���f�[�^�j�̃J�[�h���̓��A���^�C���ʐM�̐��Z�}�̂Ɠ����ɂ��Ăق���
//
//// MH810100(S) K.Onodera  2020/02/27 �Ԕԃ`�P�b�g���X(#3946 QR�R�[�h�Ŋ�������A���Z������Ƃ肯���ŏ�����ʂɖ߂�^�C�~���O�ōċN�����Ă��܂�)
//	DC_QR_work.QR_Info.Passkind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.SeasonKind;		// ������
//	DC_QR_work.QR_Info.MemberKind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.MemberKind;	// ������
//
//	// ���ɒ��ԏꇂ
//	DC_QR_work.QR_Info.InParkNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.ParkingNo;
//// MH810100(E) K.Onodera  2020/02/27 �Ԕԃ`�P�b�g���X(#3946 QR�R�[�h�Ŋ�������A���Z������Ƃ肯���ŏ�����ʂɖ߂�^�C�~���O�ōċN�����Ă��܂�)
//	// ���Ɏ���(�N���������b)
//	memcpy( &DC_QR_work.QR_Info.InDate, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec, sizeof(DC_QR_work.QR_Info.InDate) );

	// �⍇�������̊������擾���ʂ́u���Z�}�́v���Z�b�g����
	DC_QR_work.QR_Info.in_media.ParkingNo = g_UseMediaData.QueryResult.Query_ParkingNo;		// �⍇���}�� ���ԏꇂ		0�`999999
	DC_QR_work.QR_Info.in_media.Kind = g_UseMediaData.QueryResult.Query_Kind;			// �⍇���}�� ���			0�`65000
	memcpy( DC_QR_work.QR_Info.in_media.CardNo, g_UseMediaData.QueryResult.Query_CardNo, sizeof(DC_QR_work.QR_Info.in_media.CardNo) );
																									// �⍇���}�� �ԍ�			�yASCII�z32���i���l�A�c���Null�Œ�j���{�����101�Ȃ�@�B��+���������A���ꂪ����H

	DC_QR_work.QR_Info.Passkind	= g_UseMediaData.QueryResult.Passkind;		// ������
	DC_QR_work.QR_Info.MemberKind	= g_UseMediaData.QueryResult.MemberKind;	// ������
	// ���ɒ��ԏꇂ
	DC_QR_work.QR_Info.InParkNo = g_UseMediaData.QueryResult.InParkNo;
	// ���Ɏ���(�N���������b)
	memcpy( &DC_QR_work.QR_Info.InDate, &g_UseMediaData.QueryResult.InDate, sizeof(DC_QR_work.QR_Info.InDate) );
//// MH810100(E) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j

	DC_QR_work.QR_Info.ReciptCnt = g_UseMediaData.bar_count;							// ���V�[�g����
	DC_QR_work.QR_Info.Method = 0;														// �������@(0=�I�����C���f�[�^�A1=�I�t���C���f�[�^�A2=AR-100�^150 )
	DC_QR_work.QR_Info.Lastflg = flg;													// �ŏI�t���O(0=�F�؊J�n�f�[�^�A1=�F�ؓr���f�[�^�A2=�F�؍ŏI�f�[�^)

// MH810100(S) K.Onodera  2020/03/10 �Ԕԃ`�P�b�g���X(#3971 �����F�ؓo�^�E����v���f�[�^��QR�R�[�hID��ݒ肵�Ă��Ȃ�)
//	DC_QR_work.QR_Info.DiscountInfoSize = (sizeof(QR_Discount) * ONL_MAX_DISC_NUM) ;	// �����F�؏��f�[�^�T�C�Y
// MH810100(E) K.Onodera  2020/03/10 �Ԕԃ`�P�b�g���X(#3971 �����F�ؓo�^�E����v���f�[�^��QR�R�[�hID��ݒ肵�Ă��Ȃ�)

	// �����F�؏��
// MH810100(S) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(������Ɋ����F�؏����Z�b�g���Ă��܂�)
	if( kind == 0 ){	// �����敪(0=�m��A1=���)
// MH810100(E) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(������Ɋ����F�؏����Z�b�g���Ă��܂�)
		for( cnt=0; cnt<ONL_MAX_DISC_NUM; cnt++ ){
			pDisc = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt];
			// ������񂠂�H
			if( pDisc->DiscSyu || pDisc->DiscParkNo ){
				disc_cnt++;
				// ���Z�b�g
				DC_QR_work.QR_Info.DiscountInfo[cnt].ParkingNo = pDisc->DiscParkNo;		// ���ԏꇂ
				DC_QR_work.QR_Info.DiscountInfo[cnt].Kind = pDisc->DiscSyu;				// ���
				DC_QR_work.QR_Info.DiscountInfo[cnt].CardType = pDisc->DiscCardNo;		// �J�[�h�敪
				DC_QR_work.QR_Info.DiscountInfo[cnt].Type = pDisc->DiscNo;				// �敪
				DC_QR_work.QR_Info.DiscountInfo[cnt].DiscInfo = pDisc->DiscInfo;		// �������
			}
		}
// MH810100(S) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(������Ɋ����F�؏����Z�b�g���Ă��܂�)
	}
// MH810100(E) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(������Ɋ����F�؏����Z�b�g���Ă��܂�)
	DC_QR_work.QR_Info.CertCnt = disc_cnt;											// �����F�ؐ�(0�`25)
// MH810100(S) K.Onodera  2020/03/10 �Ԕԃ`�P�b�g���X(#3971 �����F�ؓo�^�E����v���f�[�^��QR�R�[�hID��ݒ肵�Ă��Ȃ�)
	DC_QR_work.QR_Info.DiscountInfoSize = (sizeof(QR_Discount) * disc_cnt) ;		// �����F�؏��f�[�^�T�C�Y
// MH810100(E) K.Onodera  2020/03/10 �Ԕԃ`�P�b�g���X(#3971 �����F�ؓo�^�E����v���f�[�^��QR�R�[�hID��ݒ肵�Ă��Ȃ�)

	DC_QR_work.QR_Info.QR_ID = pMedia->Barcode.id;						// QR�R�[�hID(���j�[�NID)
	DC_QR_work.QR_Info.QR_FormRev = pMedia->Barcode.rev;				// QR�R�[�h�t�H�[�}�b�gRev.��
	DC_QR_work.QR_Info.QR_type = (pMedia->Barcode.id - BAR_ID_AMOUNT);	// QR���(0=QR���グ/1=QR������)
// MH810100(S) K.Onodera  2020/03/10 �Ԕԃ`�P�b�g���X(#3971 �����F�ؓo�^�E����v���f�[�^��QR�R�[�hID��ݒ肵�Ă��Ȃ�)
//	DC_QR_work.QR_Info.QR_data_size = BAR_INFO_SIZE;					// QR���f�[�^�T�C�Y(�p�[�X�f�[�^)
//	DC_QR_work.QR_Info.QR_row_size = BAR_DATASIZE;						// QR�R�[�h�f�[�^�T�C�Y(���f�[�^)
	DC_QR_work.QR_Info.QR_data_size = pMedia->Barcode.info_size;		// QR���f�[�^�T�C�Y(�p�[�X�f�[�^)
	DC_QR_work.QR_Info.QR_row_size = pMedia->RowSize;					// QR�R�[�h�f�[�^�T�C�Y(���f�[�^)
// MH810100(E) K.Onodera  2020/03/10 �Ԕԃ`�P�b�g���X(#3971 �����F�ؓo�^�E����v���f�[�^��QR�R�[�hID��ݒ肵�Ă��Ȃ�)
	memcpy( DC_QR_work.QR_Info.QR_data, &pMedia->Barcode.QR_data, BAR_INFO_SIZE );
	memcpy( DC_QR_work.QR_Info.QR_row_data, pMedia->RowData, BAR_DATASIZE );

	Log_regist( LOG_DC_QR );				// ���O�o�^
	DC_UpdateCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔԂ��J�E���g�A�b�v
}

// MH810100(S) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
void ope_MakeCertifLog_quick( tMediaDetail *pMedia, uchar flg, eCertifKind kind )
{

	ushort cnt = 0, disc_cnt = 0;
	stDiscount_t* pDisc = NULL;
	DATE_YMDHMS		Certif_tim;
	// �F�؎������
	memcpy( &Certif_tim, &CLK_REC, sizeof(DATE_YMDHMS) );

	memset( &DC_QR_work, 0, sizeof(DC_QR_work) );

	DC_QR_work.ID = op_GetSequenceID();							// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	DC_QR_work.CenterOiban = DC_GetCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔ�
	if( DC_QR_work.CenterOiban == 0 ){
		DC_UpdateCenterSeqNo( DC_SEQNO_QR );
		DC_QR_work.CenterOiban = 1;
		DC_QR_work.CenterOibanFusei = 1;						// �Z���^�[�ǔԕs���t���O
	}
	DC_QR_work.QR_Info_Rev = 1;									// QR���Rev.��

	// QR���Z�b�g ---------------------------------------------------------------
	DC_QR_work.QR_Info.FormRev = 1;								// �t�H�[�}�b�gRev.��
	memcpy( &DC_QR_work.QR_Info.CertDate, &Certif_tim, sizeof(DC_QR_work.QR_Info.CertDate) );	// �F�؊m�莞��
	DC_QR_work.QR_Info.ProcessKind = kind;						// �����敪(0=�m��A1=���)
	// ���ɔ}��

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// 	// �⍇�������̊������擾���ʂ́u���Z�}�́v���Z�b�g����
// 	DC_QR_work.QR_Info.in_media.ParkingNo = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;		// �⍇���}�� ���ԏꇂ		0�`999999
// 	DC_QR_work.QR_Info.in_media.Kind = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;			// �⍇���}�� ���			0�`65000
// 	memcpy( DC_QR_work.QR_Info.in_media.CardNo, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo, sizeof(DC_QR_work.QR_Info.in_media.CardNo) );
// 																									// �⍇���}�� �ԍ�			�yASCII�z32���i���l�A�c���Null�Œ�j���{�����101�Ȃ�@�B��+���������A���ꂪ����H
	if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.FeeCalcOnOff ) {			// �����v�Z���ʂ���
		// �����v�Z����
		DC_QR_work.QR_Info.in_media.ParkingNo =
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.ParkingLotNo;	// ���Z�}�� ���ԏꇂ
		DC_QR_work.QR_Info.in_media.Kind =
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.CardType;		// ���Z�}�� ���
		memcpy(DC_QR_work.QR_Info.in_media.CardNo,
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.byCardNo,		// ���Z�}�� �ԍ�
			sizeof(DC_QR_work.QR_Info.in_media.CardNo));
	} else if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.PayResultInfo ) {	// ������񂠂�
		// �������
		DC_QR_work.QR_Info.in_media.ParkingNo =
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;	// ���Z�}�� ���ԏꇂ
		DC_QR_work.QR_Info.in_media.Kind =
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;		// ���Z�}�� ���
		memcpy(DC_QR_work.QR_Info.in_media.CardNo,
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo,		// ���Z�}�� �ԍ�
			sizeof(DC_QR_work.QR_Info.in_media.CardNo));
	}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

	DC_QR_work.QR_Info.Passkind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.SeasonKind;		// ������
	DC_QR_work.QR_Info.MemberKind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.MemberKind;	// ������

	// ���ɒ��ԏꇂ
	DC_QR_work.QR_Info.InParkNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.ParkingNo;
	// ���Ɏ���(�N���������b)
	memcpy( &DC_QR_work.QR_Info.InDate, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec, sizeof(DC_QR_work.QR_Info.InDate) );

	DC_QR_work.QR_Info.ReciptCnt = 1;			// ���V�[�g����1���Œ�i�����Ȃ̂Łj
	DC_QR_work.QR_Info.Method = 0;														// �������@(0=�I�����C���f�[�^�A1=�I�t���C���f�[�^�A2=AR-100�^150 )
	DC_QR_work.QR_Info.Lastflg = flg;													// �ŏI�t���O(0=�F�؊J�n�f�[�^�A1=�F�ؓr���f�[�^�A2=�F�؍ŏI�f�[�^)

	// �����F�؏��
	if( kind == 0 ){	// �����敪(0=�m��A1=���)
		for( cnt=0; cnt<ONL_MAX_DISC_NUM; cnt++ ){
			pDisc = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt];
			// ������񂠂�H
			if( pDisc->DiscSyu || pDisc->DiscParkNo ){
				disc_cnt++;
				// ���Z�b�g
				DC_QR_work.QR_Info.DiscountInfo[cnt].ParkingNo = pDisc->DiscParkNo;		// ���ԏꇂ
				DC_QR_work.QR_Info.DiscountInfo[cnt].Kind = pDisc->DiscSyu;				// ���
				DC_QR_work.QR_Info.DiscountInfo[cnt].CardType = pDisc->DiscCardNo;		// �J�[�h�敪
				DC_QR_work.QR_Info.DiscountInfo[cnt].Type = pDisc->DiscNo;				// �敪
				DC_QR_work.QR_Info.DiscountInfo[cnt].DiscInfo = pDisc->DiscInfo;		// �������
			}
		}
	}
	DC_QR_work.QR_Info.CertCnt = disc_cnt;											// �����F�ؐ�(0�`25)
	DC_QR_work.QR_Info.DiscountInfoSize = (sizeof(QR_Discount) * disc_cnt) ;		// �����F�؏��f�[�^�T�C�Y

	DC_QR_work.QR_Info.QR_ID = pMedia->Barcode.id;						// QR�R�[�hID(���j�[�NID)
	DC_QR_work.QR_Info.QR_FormRev = pMedia->Barcode.rev;				// QR�R�[�h�t�H�[�}�b�gRev.��
	DC_QR_work.QR_Info.QR_type = (pMedia->Barcode.id - BAR_ID_AMOUNT);	// QR���(0=QR���グ/1=QR������)
	DC_QR_work.QR_Info.QR_data_size = pMedia->Barcode.info_size;		// QR���f�[�^�T�C�Y(�p�[�X�f�[�^)
	DC_QR_work.QR_Info.QR_row_size = pMedia->RowSize;					// QR�R�[�h�f�[�^�T�C�Y(���f�[�^)
	memcpy( DC_QR_work.QR_Info.QR_data, &pMedia->Barcode.QR_data, BAR_INFO_SIZE );
	memcpy( DC_QR_work.QR_Info.QR_row_data, pMedia->RowData, BAR_DATASIZE );

	Log_regist( LOG_DC_QR );				// ���O�o�^
	DC_UpdateCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔԂ��J�E���g�A�b�v
}
// MH810100(E) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)

//[]----------------------------------------------------------------------[]
///	@brief			�I�t���C�����̔F�؃f�[�^�o�^
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
void ope_RegisterCertificateData( eCertifKind kind )
{
	ushort	index = 0;
	ushort	start = 0;
	uchar 	lastflg = 0;

	DC_PushCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔԂ�Push

	for( index = 0; index < g_UseMediaData.bar_count; ++index ){
		WACDOG;

		// �ŏI�t���O
		if( index == g_UseMediaData.bar_count-1 ){
			lastflg = 2;		// �ŏI
		}
		else if( start == 0 ){
			start = 1;			// �J�n
			lastflg = 0;
		}
		else{
			lastflg = 1;		// �r��
		}
		ope_MakeCertifLog( &g_UseMediaData.data[index], lastflg, kind );
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			�F�؊m��v�����M
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
BOOL ope_SendCertifCommit( void )
{
	// �F�؊m�莞��
	memcpy( &g_UseMediaData.Certif_tim, &CLK_REC, sizeof(DATE_YMDHMS) );

	ope_RegisterCertificateData( CERTIF_COMMIT );

	return TRUE;
}

//[]----------------------------------------------------------------------[]
///	@brief			�F�؎���v�����M
//[]----------------------------------------------------------------------[]
///	@param			bWaitRes : �����҂��t���O
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
//static BOOL ope_SendCertifCancel( void )
BOOL ope_SendCertifCancel( void )
{
	// �F�؎������
	memcpy( &g_UseMediaData.Certif_tim, &CLK_REC, sizeof(DATE_YMDHMS) );

	ope_RegisterCertificateData( CERTIF_ROLLBACK );

	return TRUE;
}

// MH810100(S) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
// �L�����Z���f�[�^�𑦎��o�^
void ope_CancelRegist( tMediaDetail* pMedia )
{
	ope_MakeCertifLog_quick( pMedia, 2, CERTIF_ROLLBACK );
}
// MH810100(E) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
// MH810100(S) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j
// �Z���^�[�₢���킹�����ʂ�DC�p�̏��ɃZ�b�g
void SetQRMediabyQueryResult()
{
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// 	// �⍇�������̊������擾���ʂ́u���Z�}�́v���Z�b�g����
// 	g_UseMediaData.QueryResult.Query_ParkingNo = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;		// �⍇���}�� ���ԏꇂ		0�`999999
// 	g_UseMediaData.QueryResult.Query_Kind = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;			// �⍇���}�� ���			0�`65000
// 	memcpy( g_UseMediaData.QueryResult.Query_CardNo, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo, sizeof(g_UseMediaData.QueryResult.Query_CardNo) );
// 																									// �⍇���}�� �ԍ�			�yASCII�z32���i���l�A�c���Null�Œ�j���{�����101�Ȃ�@�B��+���������A���ꂪ����H
	if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.FeeCalcOnOff ) {			// �����v�Z���ʂ���
		// �����v�Z����
		g_UseMediaData.QueryResult.Query_ParkingNo =
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.ParkingLotNo;	// ���Z�}�� ���ԏꇂ
		g_UseMediaData.QueryResult.Query_Kind =
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.CardType;		// ���Z�}�� ���
// GG124100(S) R.Endo 2022/08/24 �Ԕԃ`�P�b�g���X3.0 #6558 ���Z���~���A�����F�ؓo�^�E����v���f�[�^�̃J�[�h�ԍ��̒l���u0�v�ƂȂ��Ă���
// 		memcpy(DC_QR_work.QR_Info.in_media.CardNo,
// 			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.byCardNo,		// ���Z�}�� �ԍ�
// 			sizeof(DC_QR_work.QR_Info.in_media.CardNo));
		memcpy(g_UseMediaData.QueryResult.Query_CardNo,
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.byCardNo,		// ���Z�}�� �ԍ�
			sizeof(g_UseMediaData.QueryResult.Query_CardNo));
// GG124100(E) R.Endo 2022/08/24 �Ԕԃ`�P�b�g���X3.0 #6558 ���Z���~���A�����F�ؓo�^�E����v���f�[�^�̃J�[�h�ԍ��̒l���u0�v�ƂȂ��Ă���
	} else if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.PayResultInfo ) {	// ������񂠂�
		// �������
		g_UseMediaData.QueryResult.Query_ParkingNo =
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;	// ���Z�}�� ���ԏꇂ
		g_UseMediaData.QueryResult.Query_Kind =
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;		// ���Z�}�� ���
		memcpy(g_UseMediaData.QueryResult.Query_CardNo,
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo,		// ���Z�}�� �ԍ�
			sizeof(g_UseMediaData.QueryResult.Query_CardNo));
	}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
	g_UseMediaData.QueryResult.Passkind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.SeasonKind;		// ������
	g_UseMediaData.QueryResult.MemberKind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.MemberKind;	// ������
	// ���ɒ��ԏꇂ
	g_UseMediaData.QueryResult.InParkNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.ParkingNo;
	// ���Ɏ���(�N���������b)
	memcpy( &g_UseMediaData.QueryResult.InDate, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec, sizeof(g_UseMediaData.QueryResult.InDate) );
}
// MH810100(E) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j

//[]----------------------------------------------------------------------[]
///	@brief			���A���^�C�����Z���~���O�f�[�^�̍쐬
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void Set_Cancel_RTPay_Data(void)
{
	ushort	i,j;
// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// 	stDiscount2_t* pDisc2;
	stDiscount4_t* pDisc = NULL;
	stDiscount2_t* pDisc2 = NULL;
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
	stDiscount_t*  pDisc3;

	// ID
	RTPay_Data.ID = lcdbm_rsp_in_car_info_main.id;

	// �J�[�h���Rev.No.
	RTPay_Data.crd_info_rev_no = lcdbm_rsp_in_car_info_main.crd_info_rev_no;

	// ̫�ϯ�Rev.No.
	RTPay_Data.crd_info.FormatNo = lcdbm_rsp_in_car_info_main.crd_info.FormatNo;

	// �ݎԏ��_���Z�����敪�F���~�ɂ�����M�l�����̂܂ܕԂ�

	// �ݎԏ��_���Z_�N���������b(��������)�F��M�l�����̂܂ܕԂ�

	// �ݎԏ��_���ԗ���(0�`999990(10�~�P��))�F��M�l�����̂܂ܕԂ�

	// �ݎԏ��_���Ԏ���(0�`999999(���P��))�F��M�l�����̂܂ܕԂ�

// MH810101(S) R.Endo 2021/02/09 #5255 �y�A���]��NG�z�Z���^�[�⍇�����ʂ̈ꎞ���p�t���O���A���Z������ɐ��Z�@�ŏ����Ȃ��łق���
//	// �ݎԏ��_�ꎞ���p�t���O(0=������p�A1=�ꎞ���p)
//	if( ryo_buf.ryo_flg < 2 ){	// ���Ԍ����Z����
//		RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = 1;	// �ꎞ���p
//	}else{
//		RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = 0;	// ������p
//	}
	// �ݎԏ��_�ꎞ���p�t���O(0=������p�A1=�ꎞ���p)�F��M�l�����̂܂ܕԂ�
// MH810101(E) R.Endo 2021/02/09 #5255 �y�A���]��NG�z�Z���^�[�⍇�����ʂ̈ꎞ���p�t���O���A���Z������ɐ��Z�@�ŏ����Ȃ��łق���

	// �ݎԏ��_����01�`25�F��M�l���犄���ς݂̂��R�s�[��������
	// �������ςł����Ă��Đ��Z���Ɏc��̊������g�p���Ċ����z���X�V����Ă���\���������
	memset( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0, sizeof(stDiscount_t)*ONL_MAX_DISC_NUM );

// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
	if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.FeeCalcOnOff ) {			// �����v�Z���ʂ���
		for ( i = 0, j = 0; i < ONL_MAX_DISC_NUM; i++ ) {
			pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.stDiscountInfo[i];
			if ( pDisc->DiscStatus == 2 ) {	// ������
				pDisc3 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j];

				pDisc3->DiscParkNo		= pDisc->DiscParkNo;			// ���ԏꇂ(0�`999999)
				pDisc3->DiscSyu			= pDisc->DiscSyu;				// ���(0�`9999)
				pDisc3->DiscCardNo		= pDisc->DiscCardNo;			// ���ދ敪(0�`65000)
				pDisc3->DiscNo			= pDisc->DiscNo;				// �敪(0�`9)
				pDisc3->DiscCount		= pDisc->DiscCount;				// ����(0�`99)
				pDisc3->Discount		= pDisc->PrevUsageDiscAmount;	// ���z(0�`999999)
				pDisc3->DiscTime		= pDisc->PrevUsageDiscTime;		// ����(0�`999999)
				pDisc3->DiscInfo		= pDisc->DiscInfo;				// �������(0�`65000)
				pDisc3->DiscCorrType	= pDisc->DiscCorrType;			// �Ή����ގ��(0�`65000)
				pDisc3->DiscStatus		= pDisc->DiscStatus;			// �ð��(0�`9)
				pDisc3->DiscFlg			= pDisc->DiscFlg;				// ������(0�`9)

				j++;
			}
		}
	} else if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.PayResultInfo ) {	// ������񂠂�
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

	for( i = 0, j = 0; i < ONL_MAX_DISC_NUM; i++ ){
		pDisc2 = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];
		if( pDisc2->DiscStatus == 2 ){	// ������
			pDisc3 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j];

			pDisc3->DiscParkNo		= pDisc2->DiscParkNo;			// ���� ���ԏꇂ(0�`999999)
			pDisc3->DiscSyu			= pDisc2->DiscSyu;				// ���� ���(0�`9999)
			pDisc3->DiscCardNo		= pDisc2->DiscCardNo;			// ���� ���ދ敪(0�`65000)
			pDisc3->DiscNo			= pDisc2->DiscNo;				// ���� �敪(0�`9)
			pDisc3->DiscCount		= pDisc2->DiscCount;			// ���� ����(0�`99)
			pDisc3->DiscInfo		= pDisc2->DiscInfo;				// ���� �������(0�`65000)
			pDisc3->DiscCorrType	= pDisc2->DiscCorrType;			// ���� �Ή����ގ��(0�`65000)
			pDisc3->DiscStatus		= pDisc2->DiscStatus;			// ���� �ð��(0�`9)
			pDisc3->DiscFlg			= pDisc2->DiscFlg;				// ���� ������(0�`9)
// MH810100(S) 2020/05/28 �Ԕԃ`�P�b�g���X(#4196)
//			if( pDisc2->DiscSyu >= NTNET_SVS_T ){	// ���Ԋ����H
//				pDisc3->Discount		= pDisc2->DiscountT;		// ���� ���z�^����(���ԃG���A����Z�b�g)
//			}else{
//				pDisc3->Discount		= pDisc2->DiscountM;		// ���� ���z�^����(���z�G���A����Z�b�g)
//			}
// MH810100(S) 2020/07/29 �Ԕԃ`�P�b�g���X(#4561 �Z���^�[�����M������������ύX����)
//			pDisc3->Discount		= pDisc2->Discount;				// ���z/����(0�`999999)
			pDisc3->Discount		= pDisc2->UsedDisc;				// ���z/����(0�`999999)
// MH810100(E) 2020/07/29 �Ԕԃ`�P�b�g���X(#4561 �Z���^�[�����M������������ύX����)
//			pDisc3->UsedDisc		= pDisc2->UsedDisc;				// �����������p���̎g�p�ς݊����i���z/���ԁj(0�`999999)
// MH810100(E) 2020/05/28 �Ԕԃ`�P�b�g���X(#4196)
// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
			pDisc3->DiscTime		= 0;							// ����(0�`999999)
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)

			j++;
		}
	}

// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
	}
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

}

//[]----------------------------------------------------------------------[]
///	@brief			���A���^�C�����Z�������O�f�[�^�̍쐬
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
#define	EC_KOUTSUU_USED_TEMP	99
void Set_Pay_RTPay_Data(void)
{
	ushort	CardKind = 0;	// ���ϋ敪
	ulong	amount = 0L;	// ���Z���z
	ulong	indate;			// ���ɓ�
	ulong	outdate;		// �o�ɓ�
	ulong	intime;			// ���ɓ����i�����Z�j
	ulong	outtime;		// �o�ɓ����i�����Z�j
	ulong	parktime = 0L;	// ���Ԏ��ԁi�����Z�j
	ushort	loop_cnt;
// MH810100(S) 2020/08/31 #4787 ���A���^�C���ʐM�@�����Z�o�ɂ̐��Z�f�[�^�iID�F57�j�̐��Z�N���������b���o�ɔN���������b�Ɠ����ɂȂ��Ă���
	struct clk_rec *pTime;
// MH810100(E) 2020/08/31 #4787 ���A���^�C���ʐM�@�����Z�o�ɂ̐��Z�f�[�^�iID�F57�j�̐��Z�N���������b���o�ɔN���������b�Ɠ����ɂȂ��Ă���

// MH810100(S) 2020/09/08 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
// Set_All_RTPay_Data�ŃZ�b�g�ς�
//	// ID
//	RTPay_Data.ID = lcdbm_rsp_in_car_info_main.id;
//
//	// �J�[�h���Rev.No.
//	RTPay_Data.crd_info_rev_no = lcdbm_rsp_in_car_info_main.crd_info_rev_no;
//
//	// ̫�ϯ�Rev.No.
//	RTPay_Data.crd_info.FormatNo = lcdbm_rsp_in_car_info_main.crd_info.FormatNo;
// MH810100(E) 2020/09/08 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j

	// �ݎԏ��_���ԏꇂ(0�`999999)
	RTPay_Data.crd_info.dtZaishaInfo.ResParkingLotNo = prm_get( COM_PRM, S_SYS, 1, 6, 1);	// ���ԏꇂ

	// �ݎԏ��_���Z�����敪
	// �ʏ퐸�Z�H
	if( PayData.PayClass <= 3 ){ // PayData.PayClass = 0(���Z)/1(�Đ��Z)(2(���Z���~)/3(�Đ��Z���~)�͒ʂ�Ȃ�)
		RTPay_Data.crd_info.dtZaishaInfo.PaymentType = PayData.PayClass + 1;	// 1(���Z)/2(�Đ��Z)
	}
	// �N���W�b�g�H
	else if( PayData.PayClass <= 5 ){
		RTPay_Data.crd_info.dtZaishaInfo.PaymentType = (PayData.PayClass - 4) + 1;
	}
	// ���̑�
	else{
		RTPay_Data.crd_info.dtZaishaInfo.PaymentType = 1;	// �͈͊O�̏ꍇ�́u1(���Z)�v�Œ�
	}

	// �ݎԏ��_���Z�o��(0���ʏ퐸�Z�^1�������o�Ɂ^�i2�����Z�Ȃ��o�Ɂj�^3���s���o�Ɂ^9���˔j�o�Ɂ^10���Q�[�g�J���^
	//					 97�����b�N�J�E�t���b�v�㏸�O�����Z�o�Ɂ^98�����O�^�C�����o�Ɂ^99���T�[�r�X�^�C�����o��)
	RTPay_Data.crd_info.dtZaishaInfo.ExitPayment = PayData.OutKind;

	// �ݎԏ��_��������z(0�`999999)
	RTPay_Data.crd_info.dtZaishaInfo.GenkinFee = (ulong)(PayData.WTotalPrice + PayData.zenkai);	// ����̎��z�{�O��̎��z
// MH810100(S) S.Fujii 2020/08/26 #4747 �N���W�b�g���Z�ōĐ��Z���Ɍ�������z�������Ă���
	for( loop_cnt = 0; loop_cnt < ONL_MAX_SETTLEMENT; loop_cnt++ ){
		if( RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeType != 0L ) {
			// �O�񌈎Z�z������
			// CargeFee�ɂ͂��̎��_�ō��񕪂̌��ϊz�͂܂�������Ă��Ȃ��̂őO�񌈍ϊz�݂̂������Ă���
			RTPay_Data.crd_info.dtZaishaInfo.GenkinFee -= RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeFee;	// ���ϊz
		} else {
			break;
		}
	}
// MH810100(E) S.Fujii 2020/08/26 #4747 �N���W�b�g���Z�ōĐ��Z���Ɍ�������z�������Ă���

	// �ݎԏ��_�������(0�`99)
// MH810100(S) 2020/08/19 �Ԕԃ`�P�b�g���X(#4742 �y���؉ێw�E�����z�@��ʐؑ֌�̃��A���^�C�����Z�f�[�^�̗�����ʂ��ύX����Ă��Ȃ�)
	//RTPay_Data.crd_info.dtZaishaInfo.shFeeType = (ushort)PayData.syu;		// �u������ʁv�͎Q�Ƃ݂̂̂��߁A��ʐؑւ��������Ă��ύX���Ȃ�
	// �d�l�ύX�ɂ�蕜��
	RTPay_Data.crd_info.dtZaishaInfo.shFeeType = (ushort)PayData.syu;		
// MH810100(S) 2020/08/19 �Ԕԃ`�P�b�g���X(#4742 �y���؉ێw�E�����z�@��ʐؑ֌�̃��A���^�C�����Z�f�[�^�̗�����ʂ��ύX����Ă��Ȃ�)

	// �ݎԏ��_���Z_�N���������b(��������)
	RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.ParkingNo = prm_get( COM_PRM, S_SYS, 1, 6, 1);	// ���ԏꇂ(0�`999999)
	RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.MachineKind = NTNET_MODEL_CODE;					// �@����(0�`999)
	RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.MachineNo = prm_get( COM_PRM, S_PAY, 2, 2, 1);	// �@�B��(0�`999)
// MH810100(S) 2020/08/31 #4787 ���A���^�C���ʐM�@�����Z�o�ɂ̐��Z�f�[�^�iID�F57�j�̐��Z�N���������b���o�ɔN���������b�Ɠ����ɂȂ��Ă���
//// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:223,224)�Ή�
////	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear,					// �N���������b
////			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear,
////			sizeof( stDatetTimeYtoSec_t ));
//	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,							// �N���������b
//			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,
//			sizeof( stDatetTimeYtoSec_t ));
	if( lcdbm_rsp_in_car_info_main.shubetsu == 1 ){
		pTime = GetPayStartTime();

		// ���Z���Ԃɍ��̎��ԁA�o�Ɏ��ԂɁA���Z���ԁiset_tim_only_out_card�֐��ŁA�o�Ɏ��Ԃ𐸎Z���Ԃɂ���Ă���̂Łj�A���Z���Ԃ�����

		// �ݎԏ��_���Z �N���������b
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear		= pTime->year;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth		= pTime->mont;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay		= pTime->date;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours		= pTime->hour;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute	= pTime->minu;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond	= pTime->seco;

		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec,							// �N���������b
				&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,
				sizeof( stDatetTimeYtoSec_t ));

	}else{
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,							// �N���������b
				&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,
				sizeof( stDatetTimeYtoSec_t ));
	}
// MH810100(E) 2020/08/31 #4787 ���A���^�C���ʐM�@�����Z�o�ɂ̐��Z�f�[�^�iID�F57�j�̐��Z�N���������b���o�ɔN���������b�Ɠ����ɂȂ��Ă���

	// �ݎԏ��_���ԗ���(0�`999990(10�~�P��))
	RTPay_Data.crd_info.dtZaishaInfo.ParkingFee = PayData.WPrice;

	// �ݎԏ��_���Ԏ���(0�`999999(���P��)):���Ԏ��Ԉ��ް��ҏW����(RYOUSYUU_edit_parktime())�Q�l
	if( PayData.TInTime.Year != 0 ){		// ���Ɏ�������H
		indate	= dnrmlzm(					// ���ɓ��v�Z
							(short)PayData.TInTime.Year,
							(short)PayData.TInTime.Mon,
							(short)PayData.TInTime.Day
						);
		outdate	= dnrmlzm(					// �o�ɓ��v�Z
							(short)PayData.TOutTime.Year,
							(short)PayData.TOutTime.Mon,
							(short)PayData.TOutTime.Day
						);
		intime	= (indate*24*60) + (PayData.TInTime.Hour*60) + (PayData.TInTime.Min);		// ���ɓ����i�����Z�j�v�Z
		outtime	= (outdate*24*60) + (PayData.TOutTime.Hour*60) + (PayData.TOutTime.Min);	// �o�ɓ����i�����Z�j�v�Z
		if( outtime >= intime ){			// �o�ɓ��������ɓ���
			parktime  = outtime - intime;	// ���Ԏ��Ԏ擾(�����Z)
		}
	}
	RTPay_Data.crd_info.dtZaishaInfo.ParkingTime = parktime;

// MH810101(S) R.Endo 2021/02/09 #5255 �y�A���]��NG�z�Z���^�[�⍇�����ʂ̈ꎞ���p�t���O���A���Z������ɐ��Z�@�ŏ����Ȃ��łق���
//	// �ݎԏ��_�ꎞ���p�t���O(0=������p�A1=�ꎞ���p)
//	if( ryo_buf.ryo_flg < 2 ){	// ���Ԍ����Z����
//		RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = 1;	// �ꎞ���p
//	}else{
//		RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = 0;	// ������p
//	}
	// �ݎԏ��_�ꎞ���p�t���O(0=������p�A1=�ꎞ���p)�F��M�l�����̂܂ܕԂ�
// MH810101(E) R.Endo 2021/02/09 #5255 �y�A���]��NG�z�Z���^�[�⍇�����ʂ̈ꎞ���p�t���O���A���Z������ɐ��Z�@�ŏ����Ȃ��łق���

	// �ݎԏ��_���Z�敪/���Z���z
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// 	if( PayData.Electron_data.Suica.e_pay_kind != 0 ){	// �d�q���ώ��
// 		switch( PayData.Electron_data.Suica.e_pay_kind ){
// 		case EC_CREDIT_USED:	// �N���W�b�g
// 		case EC_HOUJIN_USED:	// �@�l�J�[�h
// 			CardKind = (ushort)1;
// 			break;
// 		case EC_EDY_USED:		// Edy
// 			CardKind = (ushort)3;
// 			break;
// // �uEC_PITAPA_USED�v�́uIC�N���W�b�g�Ή��v�ڐA�����`���Ȃ���(PiTaPa���Z���Ή��H)�A�R�����g�A�E�g
// //		case EC_PITAPA_USED:	// PiTaPa
// //			CardKind = (ushort)4;
// //			break;
// 		case EC_WAON_USED:		// WAON
// 			CardKind = (ushort)5;
// 			break;
// 		case EC_NANACO_USED:	// nanaco
// 			CardKind = (ushort)6;
// 			break;
// 		case EC_SAPICA_USED:	// SAPICA
// 			CardKind = (ushort)7;
// 			break;
// 		case EC_KOUTSUU_USED:	// ��ʌn�d�q�}�l�[
// 			CardKind = (ushort)EC_KOUTSUU_USED_TEMP;	// �����Ƌ�ʂ��邽�߂̒l���ꎞ�I�ɃZ�b�g
// 			break;
// // �uEC_ETC_USED/EC_ETC_EC_USED�v�́uIC�N���W�b�g�Ή��v�ڐA�����`���Ȃ���(ETC/ETC-EC���Z���Ή��H)�A�R�����g�A�E�g
// //		case EC_ETC_USED:
// //			CardKind = (ushort)10;
// //			break;
// //		case EC_ETC_EC_USED:
// //			CardKind = (ushort)11;
// //			break;
// // �uEC_ID_USED/EC_QUIC_PAY_USED�v�́u���u�ʐM�f�[�^ �J�[�h���d�l���v�ɋL�ڂ��Ȃ��ׁA�R�����g�A�E�g
// //		case EC_ID_USED:
// //			CardKind = (ushort)12;
// //			break;
// //		case EC_QUIC_PAY_USED:
// //			CardKind = (ushort)13;
// //			break;
// // MH810104(S) nanaco�EiD�EQUICPay�Ή�2 #5849
// 		case EC_ID_USED:
// 			CardKind = (ushort)12;
// 			break;
// 		case EC_QUIC_PAY_USED:
// 			CardKind = (ushort)13;
// 			break;
// // MH810104(E) nanaco�EiD�EQUICPay�Ή�2 #5849
// // MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6508 ���A���^�C���ʐM�̍ݎԏ��̌��Z�敪��QR���ς��Ȃ�
// 		case EC_QR_USED:					// [24]QR�R�[�h����
// 			CardKind = (ushort)16;
// 			break;
// 		case EC_PITAPA_USED:				// [25]PiTaPa
// 			CardKind = (ushort)4;
// 			break;
// // MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6508 ���A���^�C���ʐM�̍ݎԏ��̌��Z�敪��QR���ς��Ȃ�
// 		default:		// ��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
// 			CardKind = (ushort)2;
// 			break;
// 		}
// 	}
	CardKind = GetCardKind();
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	switch( CardKind ){	// ���ϋ敪
	case 1:		// �N���W�b�g
		amount = PayData.credit.pay_ryo;				// ���ϊz
		break;
	case 2:		// ��ʌn�d�q�}�l�[(SX-20:Suica,PASMO,ICOCA��)
		amount = PayData.Electron_data.Suica.pay_ryo;	// �d�q���ϐ��Z���@���ϊz
		break;
	case EC_KOUTSUU_USED_TEMP:	// ��ʌn (�����Ƌ�ʂ��邽�߈ꎞ�I�ɃZ�b�g�����l)
		CardKind = 2;
		// no break
	case 3:		// Edy
//	case 4:		// PiTaPa
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6508 ���A���^�C���ʐM�̍ݎԏ��̌��Z�敪��QR���ς��Ȃ�
	case 4:		// PiTaPa
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6508 ���A���^�C���ʐM�̍ݎԏ��̌��Z�敪��QR���ς��Ȃ�
	case 5:		// WAON
	case 6:		// nanaco
	case 7:		// SAPICA
//	case 12:	// iD
//	case 13:	// QUICPay
// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
	case 12:	// iD
	case 13:	// QUICPay
// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6508 ���A���^�C���ʐM�̍ݎԏ��̌��Z�敪��QR���ς��Ȃ�
	case 16:	// QR
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6508 ���A���^�C���ʐM�̍ݎԏ��̌��Z�敪��QR���ς��Ȃ�
		amount = PayData.Electron_data.Ec.pay_ryo;		// �d�q���ϐ��Z���@���ϊz
		break;
	default:
		break;
	}
// MH810100(S) S.Fujii 2020/07/06 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//	// �N���W�b�g/�d�q�}�l�[���Ɂu�g�p�����Z�����v�Ȃ̂ŕ����g�p�͂Ȃ�([0]�ɂ̂݃Z�b�g)
//	RTPay_Data.crd_info.dtZaishaInfo.stSettlement[0].CargeType = CardKind;	// ���ϋ敪
//	RTPay_Data.crd_info.dtZaishaInfo.stSettlement[0].CargeFee = amount;		// ���Z���z
	for( loop_cnt = 0; loop_cnt < ONL_MAX_SETTLEMENT; loop_cnt++ ){
		if(( RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeType == 0L ) &&
		   ( RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeFee == 0 )){
			// ����̌��σf�[�^��ǉ�
			RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeType = CardKind;	// ���ϋ敪
			RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeFee = amount;		// ���Z���z
			break;
		} else if ( RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeType == CardKind ) {
			// ���ɓ������ϋ敪�����݂���ꍇ�͌��ϋ��z�����Z����
			RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeFee += amount;		// ���Z���z
			break;
		}
	}
// MH810100(E) S.Fujii 2020/07/06 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)

	// �ő劄���f�[�^�������[�v(���񊄈��������ς�)
	for( loop_cnt = 0; loop_cnt < ONL_DISC_MAX; loop_cnt++ ){
		if(( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt].DiscParkNo == 0L ) &&
		   ( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt].DiscSyu == 0 )){
			// �����f�[�^���Ȃ���Δ�����
			break;
		}
		if( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt].DiscStatus == 1 ){	// ���񊄈��H
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt].DiscStatus = 2;	// �����ς�
		}
	}

}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

// GG129004(S) R.Endo 2024/12/10 �d�q�̎��ؑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			���ϋ敪�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			���ϋ敪
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
ushort	GetCardKind( void )
{
	ushort	CardKind = 0;	// ���ϋ敪

	if ( PayData.Electron_data.Suica.e_pay_kind != 0 ) {	// �d�q���ώ��
		switch ( PayData.Electron_data.Suica.e_pay_kind ) {
		case EC_CREDIT_USED:		// [20]�N���W�b�g
		case EC_HOUJIN_USED:		// [22]�@�l
			CardKind = (ushort)1;
			break;
		case EC_EDY_USED:			// [13]Edy
			CardKind = (ushort)3;
			break;
		case EC_WAON_USED:			// [15]WAON
			CardKind = (ushort)5;
			break;
		case EC_NANACO_USED:		// [14]nanaco
			CardKind = (ushort)6;
			break;
		case EC_SAPICA_USED:		// [16]SAPICA
			CardKind = (ushort)7;
			break;
		case EC_KOUTSUU_USED:		// [17]��ʌnIC�J�[�h
			CardKind = (ushort)EC_KOUTSUU_USED_TEMP;	// �����Ƌ�ʂ��邽�߂̒l���ꎞ�I�ɃZ�b�g
			break;
		case EC_ID_USED:			// [18]iD
			CardKind = (ushort)12;
			break;
		case EC_QUIC_PAY_USED:		// [19]QuicPay
			CardKind = (ushort)13;
			break;
		case EC_QR_USED:			// [24]QR�R�[�h����
			CardKind = (ushort)16;
			break;
		case EC_PITAPA_USED:		// [25]PiTaPa
			CardKind = (ushort)4;
			break;
//		case EC_TCARD_USED:			// [21]T�J�[�h
//		case EC_HOUSE_USED:			// [23]�n�E�X�J�[�h
		default:					// ��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
			CardKind = (ushort)2;
			break;
		}
	}

	return CardKind;
}

//[]----------------------------------------------------------------------[]
///	@brief			RT�̎��؃f�[�^�̍쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	Set_Pay_RTReceipt_Data( void )
{
	struct clk_rec wkClk;
	stReceiptInfo *pInfo = &RTReceipt_Data.receipt_info;	// �̎��؏��
	stMasterInfo_t *pMaster = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo;	// �}�X�^�[���
	stParkKindNum_t *pCard = NULL;
	ulonglong carDataId;
	ulong indate;	// ���ɓ�
	ulong outdate;	// �o�ɓ�
	ulong intime;	// ���ɓ����i�����Z�j
	ulong outtime;	// �o�ɓ����i�����Z�j
	ulong parktime;	// ���Ԏ��ԁi�����Z�j
	int i;
	int j;
// GG129004(S) R.Endo 2024/12/13 #7565 �����Ő��Z�������Ă��̎��؃f�[�^�̃J�[�h���ϓ����Ɂu1980/3/3 00:00:00�v���Z�b�g�����
	int isCard = 1;
// GG129004(E) R.Endo 2024/12/13 #7565 �����Ő��Z�������Ă��̎��؃f�[�^�̃J�[�h���ϓ����Ɂu1980/3/3 00:00:00�v���Z�b�g�����
	ushort CardKind;	// ���ϋ敪
	uchar buffer[30];

	// Set_All_RTReceipt_Data �ňꕔ�ݒ�ς�

	// ���Z��� RYOUSYUU_edit_seisanj
	pInfo->MachineNo          = PayData.Kikai_no;		// �@�B��
	pInfo->SettlementModeMode = PayData.Seisan_kind;	// ���Z���[�h
	pInfo->IndividualNo       = PayData.Oiban.i;		// ���Z�� �ʒǔ�
	pInfo->ConsecutiveNo      = PayData.Oiban.w;		// ���Z�� �ʂ��ǔ�

	// ���Ɏ��� RYOUSYUU_edit_intime
	if ( (PayData.TInTime.Year != 0) ||
	     (PayData.TInTime.Mon  != 0) ||
	     (PayData.TInTime.Day  != 0) ||
	     (PayData.TInTime.Hour != 0) ||
	     (PayData.TInTime.Min  != 0) ) {
		pInfo->dtEntry.shYear   = PayData.TInTime.Year;	// ���ɔN
		pInfo->dtEntry.byMonth  = PayData.TInTime.Mon ;	// ���Ɍ�
		pInfo->dtEntry.byDay    = PayData.TInTime.Day ;	// ���ɓ�
		pInfo->dtEntry.byHours  = PayData.TInTime.Hour;	// ���Ɏ�
		pInfo->dtEntry.byMinute = PayData.TInTime.Min ;	// ���ɕ�
	}
	if ( (PayData.BeforeTPayTime.Year != 0) ||
	     (PayData.BeforeTPayTime.Mon  != 0) ||
	     (PayData.BeforeTPayTime.Day  != 0) ||
	     (PayData.BeforeTPayTime.Hour != 0) ||
	     (PayData.BeforeTPayTime.Min  != 0) ) {
			pInfo->dtOldPayment.shYear   = PayData.BeforeTPayTime.Year;	// �O�񐸎Z�N
			pInfo->dtOldPayment.byMonth  = PayData.BeforeTPayTime.Mon ;	// �O�񐸎Z��
			pInfo->dtOldPayment.byDay    = PayData.BeforeTPayTime.Day ;	// �O�񐸎Z��
			pInfo->dtOldPayment.byHours  = PayData.BeforeTPayTime.Hour;	// �O�񐸎Z��
			pInfo->dtOldPayment.byMinute = PayData.BeforeTPayTime.Min ;	// �O�񐸎Z��
	}

	// �o��(���Z)���� RYOUSYUU_edit_outtime
	if ( PayData.shubetsu == 1 ) {	// �����Z�o�ɐ��Z
		if ( (PayData.TUnpaidPayTime.Year != 0) ||
		     (PayData.TUnpaidPayTime.Mon  != 0) ||
		     (PayData.TUnpaidPayTime.Day  != 0) ||
		     (PayData.TUnpaidPayTime.Hour != 0) ||
		     (PayData.TUnpaidPayTime.Min  != 0) ) {
			pInfo->dtPayment.shYear   = PayData.TUnpaidPayTime.Year;	// ���Z�N
			pInfo->dtPayment.byMonth  = PayData.TUnpaidPayTime.Mon ;	// ���Z��
			pInfo->dtPayment.byDay    = PayData.TUnpaidPayTime.Day ;	// ���Z��
			pInfo->dtPayment.byHours  = PayData.TUnpaidPayTime.Hour;	// ���Z��
			pInfo->dtPayment.byMinute = PayData.TUnpaidPayTime.Min ;	// ���Z��
		}
		if ( (PayData.TOutTime.Year != 0) ||
		     (PayData.TOutTime.Mon  != 0) ||
		     (PayData.TOutTime.Day  != 0) ||
		     (PayData.TOutTime.Hour != 0) ||
		     (PayData.TOutTime.Min  != 0) ) {
			pInfo->dtExitDateTime.shYear   = PayData.TOutTime.Year;	// �o�ɔN
			pInfo->dtExitDateTime.byMonth  = PayData.TOutTime.Mon ;	// �o�Ɍ�
			pInfo->dtExitDateTime.byDay    = PayData.TOutTime.Day ;	// �o�ɓ�
			pInfo->dtExitDateTime.byHours  = PayData.TOutTime.Hour;	// �o�Ɏ�
			pInfo->dtExitDateTime.byMinute = PayData.TOutTime.Min ;	// �o�ɕ�
		}
	} else {
		if ( (PayData.TOutTime.Year != 0) ||
		     (PayData.TOutTime.Mon  != 0) ||
		     (PayData.TOutTime.Day  != 0) ||
		     (PayData.TOutTime.Hour != 0) ||
		     (PayData.TOutTime.Min  != 0) ) {
			pInfo->dtPayment.shYear  	= PayData.TOutTime.Year;	// ���Z�N
			pInfo->dtPayment.byMonth 	= PayData.TOutTime.Mon ;	// ���Z��
			pInfo->dtPayment.byDay   	= PayData.TOutTime.Day ;	// ���Z��
			pInfo->dtPayment.byHours 	= PayData.TOutTime.Hour;	// ���Z��
			pInfo->dtPayment.byMinute	= PayData.TOutTime.Min ;	// ���Z��
		}
	}

	// ���Ԏ��� RYOUSYUU_edit_parktime
	if ( PayData.TInTime.Year != 0 ) {	// ���Ɏ�������
		// ���ɓ��v�Z
		indate	= dnrmlzm(
			(short)PayData.TInTime.Year,
			(short)PayData.TInTime.Mon,
			(short)PayData.TInTime.Day);

		// �o�ɓ��v�Z
		outdate	= dnrmlzm(
			(short)PayData.TOutTime.Year,
			(short)PayData.TOutTime.Mon,
			(short)PayData.TOutTime.Day);

		// ���ɓ����i�����Z�j�v�Z
		intime	= (indate *24*60) + (PayData.TInTime.Hour *60) + (PayData.TInTime.Min);

		// �o�ɓ����i�����Z�j�v�Z
		outtime	= (outdate*24*60) + (PayData.TOutTime.Hour*60) + (PayData.TOutTime.Min);

		// ���Ԏ��Ԏ擾�i�����Z�j
		if ( outtime >= intime ) {	// �o�ɓ��������ɓ���
			parktime = outtime - intime;
		} else {	// �o�ɓ��������ɓ����i���o�ɓ����ް��ُ�j
			parktime = 0;
		}
		pInfo->ParkingTime = parktime;	// ���Ԏ���
	}

	// ���ԗ��� RYOUSYUU_edit_churyo
	pInfo->FeeKind    = PayData.syu;	// �������
	pInfo->ParkingFee = PayData.WPrice;	// ���ԗ���

	// ��������, �_�� RYOUSYUU_edit_teiki
	if ( PayData.teiki.id != 0 ) {	// ������g�p
		for ( i = 0; i < ONL_MAX_CARDNUM; i++ ) {
			if( pMaster->stCardDataInfo[i].CardType == CARD_TYPE_PASS ) {	// ���
				pCard = &pMaster->stCardDataInfo[i];
				break;
			}
		}
		if ( pCard != NULL ) {
			pInfo->SeasonParkingLotNo = pCard->ParkingLotNo;	// ����� ���ԏꇂ
			pInfo->SeasonKind         = PayData.teiki.syu;	// ����� ���
			pInfo->SeasonId           = PayData.teiki.id;	// ����� ID
			if ( ((PayData.teiki.s_year !=  0) || (PayData.teiki.s_mon !=  0) || (PayData.teiki.s_day !=  0) ||
			      (PayData.teiki.e_year != 99) || (PayData.teiki.e_mon != 99) || (PayData.teiki.e_day != 99)) ) {
				// �d�q�̎��؂͐ݒ�ɂ��\���ؑւ͑S�ăf�t�H���g�Ƃ��邽��17-0002�i������L�������󎚁j�́u����v�Œ�
				pInfo->stSeasonValidStart.shYear  = PayData.teiki.s_year;	// ����� �L���J�n�N����
				pInfo->stSeasonValidStart.byMonth = PayData.teiki.s_mon;	// ����� �L���J�n�N����
				pInfo->stSeasonValidStart.byDay   = PayData.teiki.s_day;	// ����� �L���J�n�N����
				pInfo->stSeasonValidEnd.shYear    = PayData.teiki.e_year;	// ����� �L���I���N����
				pInfo->stSeasonValidEnd.byMonth   = PayData.teiki.e_mon;	// ����� �L���I���N����
				pInfo->stSeasonValidEnd.byDay     = PayData.teiki.e_day;	// ����� �L���I���N����
			}
		}
	}

	// �Ԕ�(���Z)
	memcpy(pInfo->VehicleNumberSerialNo,
		lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber,
		sizeof(pInfo->VehicleNumberSerialNo));	// �Ԕ� ��A�w��ԍ�

	// �Ԕԃf�[�^ID
	carDataId = atoll((const char *)PayData.CarDataID);
	if ( carDataId <= 0xFFFFFFFFFFFF ) {
        for ( i = 5; i >= 0; i-- ) {
            pInfo->UniqueID[i] = carDataId & 0xFF;
            carDataId >>= 8;
        }
	}

	// ��ېŔ}�̗��p�z RYOUSYUU_edit_taxableDiscount
	pInfo->PaidFeeAmount = PayData.zenkai;	// �x���ςݗ���

	// ���v���z(�K�p�ŗ��t) RYOUSYUU_edit_totalAmount_taxRate
	pInfo->Parking_Bil = PayData.WBillAmount;	// ���v���z�i�����z�j

	// ���l�߂ƃX�y�[�X�̒ǉ��ƃ}�X�N�̐ݒ�
	memset((char *)buffer, ' ', sizeof(buffer));
	CardKind = GetCardKind();
	switch ( CardKind ) {
	case 1:		// �N���W�b�g
		// 0123456789012345 -> 012345******2345
		for ( i = 0, j = 0; i < sizeof(PayData.credit.card_no); i++ ) {
			if ( PayData.credit.card_no[i] == '\0' || PayData.credit.card_no[i] == ' ' ) {
				continue;
			}
			if ( (i <= 5) || (12 <= i) ) {
				buffer[j] = PayData.credit.card_no[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case 2:		// ��ʌn�d�q�}�l�[(SX-20)
		// JE234567890123456 -> JE*** **** **** 3456
		for ( i = 0, j = 0; i < sizeof(PayData.Electron_data.Suica.Card_ID); i++ ) {
			if ( PayData.Electron_data.Suica.Card_ID[i] == '\0' || PayData.Electron_data.Suica.Card_ID[i] == ' ' ) {
				continue;
			}
			switch ( i ) {
			case 5:
			case 9:
			case 13:
				buffer[j] = ' ';
				j++;
			}
			if ( (i <= 2) || (13 <= i) ) {
				buffer[j] = PayData.Electron_data.Suica.Card_ID[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case EC_KOUTSUU_USED_TEMP:	// ��ʌn (�����Ƌ�ʂ��邽�߈ꎞ�I�ɃZ�b�g�����l)
	case 4:		// PiTaPa
		// JE234567890123456 -> JE*** **** **** 3456
		for ( i = 0, j = 0; i < sizeof(PayData.Electron_data.Ec.Card_ID); i++ ) {
			if ( PayData.Electron_data.Ec.Card_ID[i] == '\0' || PayData.Electron_data.Ec.Card_ID[i] == ' ' ) {
				continue;
			}
			switch ( i ) {
			case 5:
			case 9:
			case 13:
				buffer[j] = ' ';
				j++;
			}
			if ( (i <= 2) || (13 <= i) ) {
				buffer[j] = PayData.Electron_data.Ec.Card_ID[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case 3:		// Edy
	case 5:		// WAON
	case 6:		// nanaco
	case 7:		// SAPICA
	case 12:	// iD
		// 0123456789012345 -> **** **** **** 2345
		for ( i = 0, j = 0; i < sizeof(PayData.Electron_data.Ec.Card_ID); i++ ) {
			if ( PayData.Electron_data.Ec.Card_ID[i] == '\0' || PayData.Electron_data.Ec.Card_ID[i] == ' ' ) {
				continue;
			}
			switch ( i ) {
			case 4:
			case 8:
			case 12:
				buffer[j] = ' ';
				j++;
			}
			if ( 12 <= i ) {
				buffer[j] = PayData.Electron_data.Ec.Card_ID[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case 13:	// QUICPay
		// 01234567890123456789 -> **** **** **** **** 6789
		for ( i = 0, j = 0; i < sizeof(PayData.Electron_data.Ec.Card_ID); i++ ) {
			if ( PayData.Electron_data.Ec.Card_ID[i] == '\0' || PayData.Electron_data.Ec.Card_ID[i] == ' ' ) {
				continue;
			}
			switch ( i ) {
			case 4:
			case 8:
			case 12:
			case 16:
				buffer[j] = ' ';
				j++;
			}
			if ( 16 <= i ) {
				buffer[j] = PayData.Electron_data.Ec.Card_ID[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case 16:	// QR
	default:
		break;
	}

	// �J�[�h���� ������Ϗ��
	switch ( CardKind ) {
	case 1:		// �N���W�b�g
		// �J�[�h���� �[���敪
		pInfo->CardPaymentTerminalCategory = 1;

		// �J�[�h���� ����X�e�[�^�X
		if ( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ) {
			pInfo->CardPaymentTransactionStatus = 3;	// �����x���s��
		}
		else if ( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ) {
			pInfo->CardPaymentTransactionStatus = 1;	// �݂Ȃ�����
		}
		else {
			pInfo->CardPaymentTransactionStatus = 0;	// �x����
		}

		if ( PayData.PayClass == 2 || PayData.PayClass == 3 ) {	// ���~
			// ����ԍ�
			memset(pInfo->card.credit.card_no, 0x20, sizeof(pInfo->card.credit.card_no));

			// �[�����ʔԍ�
			memset(pInfo->card.credit.cct_num, 0x20, sizeof(pInfo->card.credit.cct_num));

			// KID�R�[�h
			memset(pInfo->card.credit.kid_code, 0x20, sizeof(pInfo->card.credit.kid_code));

			// �����X����ԍ�
			memset(pInfo->card.credit.ShopAccountNo, 0x20, sizeof(pInfo->card.credit.ShopAccountNo));
		}
		else {
			// ���p���z
			pInfo->card.credit.amount = PayData.credit.pay_ryo;

			// ����ԍ�
			if ( (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) &&
			     (PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0) ) {
				// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.��0x20h���߂���B
				// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
				memset(pInfo->card.credit.card_no, 0x20,
					sizeof(pInfo->card.credit.card_no));
			}
			else {
				memcpy(pInfo->card.credit.card_no, buffer, sizeof(pInfo->card.credit.card_no));
			}
// GG129004(S) R.Endo 2024/12/12 #7563 �N���W�b�g���ώ��̃��A���^�C���̎��؃f�[�^�̉���ԍ����u0�v�Ń}�X�N����Ă���
// 			// ��7���ځ`��5���ڈȊO��'*'���i�[����Ă�����'0'�ɒu��
// 			change_CharInArray(pInfo->card.credit.card_no, sizeof(pInfo->card.credit.card_no), 7, 5, '*', '0');
// GG129004(E) R.Endo 2024/12/12 #7563 �N���W�b�g���ώ��̃��A���^�C���̎��؃f�[�^�̉���ԍ����u0�v�Ń}�X�N����Ă���

			// �[�����ʔԍ�
			memcpyFlushLeft(pInfo->card.credit.cct_num,
				(uchar *)PayData.credit.CCT_Num,
				sizeof(pInfo->card.credit.cct_num),
				sizeof(PayData.credit.CCT_Num));

			// KID�R�[�h
			memcpyFlushLeft(pInfo->card.credit.kid_code,
				PayData.credit.kid_code,
				sizeof(pInfo->card.credit.kid_code),
				sizeof(PayData.credit.kid_code));

			// ���F�ԍ�
			pInfo->card.credit.app_no = PayData.credit.app_no;

			// �Z���^�[�����ʔ�
			pInfo->card.credit.center_oiban = 0;

			// �����X����ԍ�
//			memcpyFlushLeft(pInfo->card.credit.ShopAccountNo,
//				PayData.credit.ShopAccountNo,
//				sizeof(pInfo->card.credit.ShopAccountNo),
//				sizeof(PayData.credit.ShopAccountNo));
			memset(pInfo->card.credit.ShopAccountNo, 0x20, sizeof(pInfo->card.credit.ShopAccountNo));

			// �`�[�ԍ�
			pInfo->card.credit.slip_no = PayData.credit.slip_no;
		}

		break;
	case 2:		// ��ʌn�d�q�}�l�[(SX-20)
		// ���p���z
		pInfo->card.emoney.amount = PayData.Electron_data.Suica.pay_ryo;						

		// �J�[�h�ԍ�
		memcpy(pInfo->card.emoney.card_id, buffer, sizeof(pInfo->card.emoney.card_id));

		// �J�[�h�c�z
		pInfo->card.emoney.card_zangaku = PayData.Electron_data.Suica.pay_after;

		break;
	case EC_KOUTSUU_USED_TEMP:	// ��ʌn (�����Ƌ�ʂ��邽�߈ꎞ�I�ɃZ�b�g�����l)
		CardKind = 2;
		// no break
	case 3:		// Edy
	case 4:		// PiTaPa
	case 5:		// WAON
	case 6:		// nanaco
	case 7:		// SAPICA
	case 12:	// iD
	case 13:	// QUICPay
		// �J�[�h���� �[���敪
		pInfo->CardPaymentTerminalCategory = 1;

		// �J�[�h���� ����X�e�[�^�X
		if ( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ) {
			pInfo->CardPaymentTransactionStatus = 3;	// �����x���s��
		}
		else if ( PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END ) {
			pInfo->CardPaymentTransactionStatus = 2;	// �����x���ς�
		}
		else if ( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ) {
			pInfo->CardPaymentTransactionStatus = 1;	// �݂Ȃ�����
		}
		else {
			pInfo->CardPaymentTransactionStatus = 0;	// �x����
		}

		if ( PayData.PayClass == 2 || PayData.PayClass == 3 ) {	// ���~
			// �J�[�h�ԍ�
			memset(pInfo->card.emoney.card_id, 0x20, sizeof(pInfo->card.emoney.card_id));
			memset(pInfo->card.emoney.card_id, 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]));
			memset(pInfo->card.emoney.card_id, 'Z', 2);

			// �₢���킹�ԍ�
			memset(pInfo->card.emoney.inquiry_num, 0x20, sizeof(pInfo->card.emoney.inquiry_num));

			// ���̒[��ID
			memset(pInfo->card.emoney.terminal_id, 0x20, sizeof(pInfo->card.emoney.terminal_id));
		}
		else {
			// ���p���z
			pInfo->card.emoney.amount = PayData.Electron_data.Ec.pay_ryo;

			// �J�[�h�ԍ�
			if ( (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) &&
			     (PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0) ) {
				// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.���J�[�h������0x30���߂����l�߂�ZZ����B
				// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
				memset(pInfo->card.emoney.card_id, 0x20, sizeof(pInfo->card.emoney.card_id));
				memset(pInfo->card.emoney.card_id, 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]));
				memset(pInfo->card.emoney.card_id, 'Z', 2);
			} else {
				memcpy(pInfo->card.emoney.card_id, buffer, sizeof(pInfo->card.emoney.card_id));
			}

			// �J�[�h�c�z
			pInfo->card.emoney.card_zangaku = PayData.Electron_data.Ec.pay_after;

			// �₢���킹�ԍ�
			memcpyFlushLeft(pInfo->card.emoney.inquiry_num,
				PayData.Electron_data.Ec.inquiry_num,
				sizeof(pInfo->card.emoney.inquiry_num),
				sizeof(PayData.Electron_data.Ec.inquiry_num));

			// ���F�ԍ�
			switch ( PayData.Electron_data.Ec.e_pay_kind ) {
			case EC_ID_USED:
				pInfo->card.emoney.approbal_no = astoinl(PayData.Electron_data.Ec.Brand.Id.Approval_No, 7);
				break;
			case EC_QUIC_PAY_USED:
				pInfo->card.emoney.approbal_no = astoinl(PayData.Electron_data.Ec.Brand.Quickpay.Approval_No, 7);
				break;
			case EC_PITAPA_USED:
				// �I�t���C�����F���ώ��̓I�[��'*'�̂��߁A'0'�֕ϊ�����
				if ( PayData.Electron_data.Ec.Brand.Pitapa.Approval_No[0] == '*' ) {
					pInfo->card.emoney.approbal_no = 0;
				}
				else {
					pInfo->card.emoney.approbal_no = astoinl(PayData.Electron_data.Ec.Brand.Pitapa.Approval_No, 8);
				}
				break;
			default:
				pInfo->card.emoney.approbal_no = 0;
				break;
			}

			// ���̒[��ID
			switch ( PayData.Electron_data.Ec.e_pay_kind ) {
			case EC_WAON_USED:
				memcpyFlushLeft(pInfo->card.emoney.terminal_id,
					PayData.Electron_data.Ec.Brand.Waon.SPRW_ID,
					sizeof(pInfo->card.emoney.terminal_id),
					sizeof(PayData.Electron_data.Ec.Brand.Waon.SPRW_ID));
				break;
			default:
				memset(pInfo->card.emoney.terminal_id, 0x20, sizeof(pInfo->card.emoney.terminal_id));
				break;
			}
		}

		break;
	case 16:	// QR
		// �J�[�h���� �[���敪
		pInfo->CardPaymentTerminalCategory = 1;

		// �J�[�h���� ����X�e�[�^�X
		if ( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ) {
			pInfo->CardPaymentTransactionStatus = 3;	// �����x���s��
		}
		else if ( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ) {
			pInfo->CardPaymentTransactionStatus = 1;	// �݂Ȃ�����
		}
		else {
			pInfo->CardPaymentTransactionStatus = 0;	// �x����
		}

		if ( PayData.PayClass == 2 || PayData.PayClass == 3 ) {	// ���~
			// Mch����ԍ�
			memset(pInfo->card.qr.MchTradeNo, 0x20, sizeof(pInfo->card.qr.MchTradeNo));

			if ( PayData.Electron_data.Ec.E_Flag.BIT.deemSettleCancal == 1 ) {
				// ���~�ł��邪���ό��ʂ���M�ł��Ȃ������̂�
				// ���ꂼ��̌�����0x20���߂��A�x���[��ID��13���A����ԍ���15���A0x30���߂���

				// �x���[��ID
				memset(pInfo->card.qr.PayTerminalNo, 0x20, sizeof(pInfo->card.qr.PayTerminalNo));
				memset(pInfo->card.qr.PayTerminalNo, 0x30, 13);

				// ����ԍ�
				memset(pInfo->card.qr.DealNo, 0x20, sizeof(pInfo->card.qr.DealNo));
				memset(pInfo->card.qr.DealNo, 0x30, 15);
			}
			else {
				// �x���[��ID
				memcpyFlushLeft(pInfo->card.qr.PayTerminalNo,
					PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo,
					sizeof(pInfo->card.qr.PayTerminalNo),
					sizeof(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo));

				// ����ԍ�
				memcpyFlushLeft(pInfo->card.qr.DealNo,
					PayData.Electron_data.Ec.inquiry_num,
					sizeof(pInfo->card.qr.DealNo),
					sizeof(PayData.Electron_data.Ec.inquiry_num));
			}
		}
		else {
			// ���p���z
			pInfo->card.qr.amount = PayData.Electron_data.Ec.pay_ryo;

			if ( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ) {
				// Mch����ԍ�
				memset(pInfo->card.qr.MchTradeNo, 0x20, sizeof(pInfo->card.qr.MchTradeNo));

				// �x���[��ID
				memcpyFlushLeft(pInfo->card.qr.PayTerminalNo,
					PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo,
					sizeof(pInfo->card.qr.PayTerminalNo),
					sizeof(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo));

				// ����ԍ�
				memcpyFlushLeft(pInfo->card.qr.DealNo,
					PayData.Electron_data.Ec.inquiry_num,
					sizeof(pInfo->card.qr.DealNo),
					sizeof(PayData.Electron_data.Ec.inquiry_num));
			}
			else {
				// Mch����ԍ�
				memcpyFlushLeft(pInfo->card.qr.MchTradeNo,
					PayData.Electron_data.Ec.Brand.Qr.MchTradeNo,
					sizeof(pInfo->card.qr.MchTradeNo),
					sizeof(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo));

				// �x���[��ID
				memset(pInfo->card.qr.PayTerminalNo, 0x20, sizeof(pInfo->card.qr.PayTerminalNo));

				// ����ԍ�
				memset(pInfo->card.qr.DealNo, 0x20, sizeof(pInfo->card.qr.DealNo));
			}
		}

		// ���σu�����h
		pInfo->card.qr.PayKind = PayData.Electron_data.Ec.Brand.Qr.PayKind;

		break;
	default:
// GG129004(S) R.Endo 2024/12/13 #7565 �����Ő��Z�������Ă��̎��؃f�[�^�̃J�[�h���ϓ����Ɂu1980/3/3 00:00:00�v���Z�b�g�����
		isCard = 0;
// GG129004(E) R.Endo 2024/12/13 #7565 �����Ő��Z�������Ă��̎��؃f�[�^�̃J�[�h���ϓ����Ɂu1980/3/3 00:00:00�v���Z�b�g�����
		break;
	}

	// �J�[�h���� �敪
	pInfo->CardPaymentCategory = CardKind;

	// �J�[�h���� �N���������b
// GG129004(S) R.Endo 2024/12/13 #7565 �����Ő��Z�������Ă��̎��؃f�[�^�̃J�[�h���ϓ����Ɂu1980/3/3 00:00:00�v���Z�b�g�����
	if ( isCard ) {
// GG129004(E) R.Endo 2024/12/13 #7565 �����Ő��Z�������Ă��̎��؃f�[�^�̃J�[�h���ϓ����Ɂu1980/3/3 00:00:00�v���Z�b�g�����
	c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &wkClk);
	pInfo->dtCardPayment.shYear   = wkClk.year;
	pInfo->dtCardPayment.byMonth  = wkClk.mont;
	pInfo->dtCardPayment.byDay    = wkClk.date;
	pInfo->dtCardPayment.byHours  = wkClk.hour;
	pInfo->dtCardPayment.byMinute = wkClk.minu;
	pInfo->dtCardPayment.bySecond = wkClk.seco;
// GG129004(S) R.Endo 2024/12/13 #7565 �����Ő��Z�������Ă��̎��؃f�[�^�̃J�[�h���ϓ����Ɂu1980/3/3 00:00:00�v���Z�b�g�����
	}
// GG129004(E) R.Endo 2024/12/13 #7565 �����Ő��Z�������Ă��̎��؃f�[�^�̃J�[�h���ϓ����Ɂu1980/3/3 00:00:00�v���Z�b�g�����

	// �K�i������ ���L��
	pInfo->EligibleInvoiceHasInfo = 1;

	// �K�i������ �o�^�ԍ�
	memcpy(pInfo->EligibleInvoiceRegistrationNo, PayData.RegistNum, sizeof(PayData.RegistNum));

	// �K�i������ �ېőΏۊz(�ō���)
	pInfo->EligibleInvoiceTaxableAmount = PayData.WTaxPrice;

	// �K�i������ �K�p�ŗ�
	pInfo->EligibleInvoiceTaxRate = PayData.WTaxRate;

	// ������� RYOUSYUU_edit_hakkenj
	pInfo->EntryMachineNo = PayData.CMachineNo;	// �����@�ԍ� [Rev.No.1�ǉ�]
	pInfo->ParkingTicketNo = PayData.TicketNum;	// ���Ԍ��ԍ� [Rev.No.1�ǉ�]

	// �����̎��z RYOUSYUU_edit_content_sub
	if ( (PayData.WInPrice - PayData.WChgPrice) > 0 ) {
		pInfo->CashReceiptAmount = PayData.WInPrice - PayData.WChgPrice;	// �����̎��z [Rev.No.1�ǉ�]
	}

	// �a����z RYOUSYUU_edit_content_sub
	pInfo->WInPrice = PayData.WInPrice;	// �a����z [Rev.No.1�ǉ�]

	// �ނ�K�z RYOUSYUU_edit_turisen
	pInfo->WChgPrice = PayData.WChgPrice;	// �ނ�K�z [Rev.No.1�ǉ�]

	// �ނ�؂��� RYOUSYUU_edit_turikire
	pInfo->WFusoku = PayData.WFusoku;	// ���ߕs�����z [Rev.No.1�ǉ�]
}

//[]----------------------------------------------------------------------[]
///	@brief			QR�̎��؃f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[out]		data	QR�̎��؃f�[�^�̊i�[��
///	@param[in]		size	QR�̎��؃f�[�^�̊i�[��̃T�C�Y
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	MakeQRCodeReceipt( char* data, size_t size )
{
	char encrypt[QRCODE_RECEIPT_ENCRYPT_SIZE + 1];
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	char buffer[QRCODE_RECEIPT_ENCRYPT_SIZE * 2];
	char buffer[15];	// �N���������b������T�C�Y
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	char crc_wk[2];
	int i;

	// QR�̎��؃f�[�^�{CRC���i�[�ł��邩�`�F�b�N
	if ( size < (QRCODE_RECEIPT_SIZE + 4) ) {
		return;
	}

	// �N���A
	memset(data, '\0', size);

	// ��Í����̈�
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(data, "https://payment.parkingweb.jp/000000000000000000000000000000000000000000000000000000000000000000000/");	// URL
	if ( prm_get(COM_PRM, S_RTP, 58, 1, 6) == 1 ) {	// 17-0058�@ �d�q�̎���URL�ύX�i�f�o�b�O�p�j
		strcat(data, "https://payment.verif.parkingdev.com/00000000000000000000000000000000000000000000000000000000000000/");	// URL
	} else {
		strcat(data, "https://payment.parkingweb.jp/000000000000000000000000000000000000000000000000000000000000000000000/");	// URL
	}
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	strcat(data, "DCENTER_QR");	// ���ʕ���
	strcat(data, "00101");		// QR�R�[�hID
	strcat(data, "00001");		// QR�R�[�h�t�H�[�}�b�gRev.No.
	strcat(data, "000001");		// �����NID
	strcat(data, "1");			// �Í�������
	strcat(data, "0");			// �\��

	// �Í����̈�
	memset(encrypt, '\0', sizeof(encrypt));

	sprintf(buffer, "%04d%02d%02d%02d%02d%02d",
		CLK_REC.year,
		CLK_REC.mont,
		CLK_REC.date,
		CLK_REC.hour,
		CLK_REC.minu,
		CLK_REC.seco);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// QR���s�N���������b
// 	strcat(encrypt, "0");		// �\��
	strcat(data, buffer);	// QR���s�N���������b
	strcat(data, "0");		// �\��
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%03d", CPrmSS[S_PKT][1]);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// �[��IP�A�h���X ��1�I�N�e�b�g
	strcat(data, buffer);	// �[��IP�A�h���X ��1�I�N�e�b�g
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%03d", CPrmSS[S_PKT][2]);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// �[��IP�A�h���X ��2�I�N�e�b�g
	strcat(data, buffer);	// �[��IP�A�h���X ��2�I�N�e�b�g
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%03d", CPrmSS[S_PKT][3]);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// �[��IP�A�h���X ��3�I�N�e�b�g
	strcat(data, buffer);	// �[��IP�A�h���X ��3�I�N�e�b�g
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%03d", CPrmSS[S_PKT][4]);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// �[��IP�A�h���X ��4�I�N�e�b�g
	strcat(data, buffer);	// �[��IP�A�h���X ��4�I�N�e�b�g
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%06d", CPrmSS[S_SYS][1]);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// ���ԏꇂ
	strcat(data, buffer);	// ���ԏꇂ
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%03d", RTReceipt_Data.receipt_info.ModelCode);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// �@��R�[�h
	strcat(data, buffer);	// �@��R�[�h
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%04d", RTReceipt_Data.receipt_info.MachineNo);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// �@�B��
	strcat(data, buffer);	// �@�B��
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%010d", RTReceipt_Data.CenterOiban);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// �Z���^�[�ǔ�
	strcat(data, buffer);	// �Z���^�[�ǔ�
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	sprintf(buffer, "%04d%02d%02d%02d%02d%02d",
		RTReceipt_Data.receipt_info.dtTimeYtoSec.shYear,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.byMonth,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.byDay,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.byHours,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.byMinute,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.bySecond);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	strcat(encrypt, buffer);	// �����N���������b
// 	strcat(encrypt, "000000000000000");	// �Í��������o�C�g
	strcat(data, buffer);	// �����N���������b
	strcat(data, "0000000000000000");	// �Í��������o�C�g
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�

	// �Í���
	EncryptWithKeyAndIV_NoPadding(
		CRYPT_KEY_QRRECEIPT,
		(uchar *)&data[QRCODE_RECEIPT_HEADER_SIZE],
		(uchar *)encrypt,
		sizeof(encrypt));

	// ASCII�ϊ�
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	for ( i = 0; i < QRCODE_RECEIPT_ENCRYPT_SIZE; i++ ) {
// 		hxtoas((uchar*)&buffer[i*2], (uchar)encrypt[i]);
// 	}
// 	memcpy(&data[QRCODE_RECEIPT_HEADER_SIZE], buffer, (QRCODE_RECEIPT_ENCRYPT_SIZE * 2));
	for ( i = 0; i < QRCODE_RECEIPT_ENCRYPT_SIZE; i++ ) {
		hxtoas((uchar*)&data[QRCODE_RECEIPT_HEADER_SIZE+(i*2)], (uchar)encrypt[i]);
	}
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�

	// CRC�Z�o
	crc_ccitt((ushort)QRCODE_RECEIPT_SIZE, (uchar *)data, (uchar *)crc_wk, (uchar)R_SHIFT);
// GG129004(S) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
// 	sprintf(buffer, "%02x%02x", crc_wk[1], crc_wk[0]);
	sprintf(buffer, "%02X%02X", crc_wk[1], crc_wk[0]);
// GG129004(E) R.Endo 2024/12/17 #7567 �d�q�̎��؂̈Í��������ɉ�����񂪃Z�b�g����Ă��Ȃ�
	memcpy(&data[QRCODE_RECEIPT_SIZE], buffer, 4);
}
// GG129004(E) R.Endo 2024/12/10 �d�q�̎��ؑΉ�

// MH810100(S) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
//[]----------------------------------------------------------------------[]
///	@brief			���ɏ��Ŏ�M�������Z���Z�b�g����
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void	Set_PayInfo_from_InCarInfo( void )
{
	ushort	cnt;
	uchar	card_info[16];
// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
	stMasterInfo_t* pMaster;
	stZaishaInfo_t* pZaisha;
	ushort	index;
// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX

	// ���Ԍ��@�B���Z�b�g
	OpeNtnetAddedInfo.CMachineNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.MachineNo;	// ���Ԍ��@�B�� = ���ɒ[���@�B��
	// NT-NET�d����A���Ԍ��@�B����1Byte�Ȃ̂�256�ȏ��0�Ƃ���
	if( OpeNtnetAddedInfo.CMachineNo <= 255 ){
		PayData.CMachineNo	= (uchar)OpeNtnetAddedInfo.CMachineNo;	// ���Ԍ��@�B��
	}else{
		PayData.CMachineNo	= 0;									// ���Ԍ��@�B��
	}
	NTNET_Data152Save((void *)&PayData.CMachineNo, NTNET_152_CMACHINENO);

	PayData.MediaKind1 = 0;	// ���Z�}�̎�ʃN���A
	memset( PayData.MediaCardNo1, 0, sizeof(PayData.MediaCardNo1) );
	memset( card_info, 0, sizeof(card_info) );
// GG129000(S) T.Nagai 2023/01/16 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
	PayData.TicketNum = 0;
	for( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ){
		if( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType == CARD_TYPE_PARKING_TKT ){	// ���Z�}�̎�� = ���Ԍ�
			PayData.MediaKind1 = CARD_TYPE_PARKING_TKT;
			memcpy( PayData.MediaCardNo1, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo, sizeof(PayData.MediaCardNo1) );
			PayData.TicketNum = astoinl(&PayData.MediaCardNo1[3], 6);
// GG129000(S) H.Fujinaga 2023/02/18 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
			memcpy( PayData.QrTicketInfo.EntryMachineNo, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo, sizeof(PayData.QrTicketInfo.EntryMachineNo) );
			memcpy( PayData.QrTicketInfo.ParkingTicketNo, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo[sizeof(PayData.QrTicketInfo.EntryMachineNo)], sizeof(PayData.QrTicketInfo.ParkingTicketNo) );
			PayData.QrTicketInfo.TicketLessMode[0] = 0;
// GG129000(E) H.Fujinaga 2023/02/18 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
			break;
		}
	}
// GG129000(E) T.Nagai 2023/01/16 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
	if( PayData.teiki.id ){	// ��������Z
		// ������֘A�}�̏��Z�b�g
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:225)�Ή�
//		for( cnt = 0; cnt < ONL_MAX_MEDIA_NUM; cnt++ ){
		for( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ){
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:225)�Ή�
			if( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType == CARD_TYPE_PARKING_TKT ){	// ���Z�}�̎�� = ���Ԍ�
				PayData.MediaKind1 = CARD_TYPE_PASS;
// GG129000(S) T.Nagai 2023/01/16 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
				memset( PayData.MediaCardNo1, 0, sizeof(PayData.MediaCardNo1) );
// GG129000(E) T.Nagai 2023/01/16 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
				intoasl( PayData.MediaCardNo1, PayData.teiki.id, 5 );	// �J�[�h�ԍ�(�����ID(5���A0�`12000))
				intoas( card_info, PayData.teiki.syu, 2 );				// �J�[�h���(��������(2���A1�`15))
				break;
			}
		}
// GG129000(S) T.Nagai 2023/01/16 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
//	}else{
//		// ���Ԍ��֘A�}�̏��Z�b�g
//// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:226,227)�Ή�
////		for( cnt = 0; cnt < ONL_MAX_MEDIA_NUM; cnt++ ){
//		for( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ){
//// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:226,227)�Ή�
//			if( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType == CARD_TYPE_PARKING_TKT ){	// ���Z�}�̎�� = ���Ԍ�
//				PayData.MediaKind1 = CARD_TYPE_PARKING_TKT;
//				memcpy( PayData.MediaCardNo1, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo, sizeof(PayData.MediaCardNo1) );
//				break;
//			}
//		}
// GG129000(E) T.Nagai 2023/01/16 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
	}
	NTNET_Data152Save( (void *)&PayData.MediaKind1, NTNET_152_MEDIAKIND1 );
	NTNET_Data152Save( (void *)PayData.MediaCardNo1, NTNET_152_MEDIACARDNO1 );
	NTNET_Data152Save( (void *)card_info, NTNET_152_MEDIACARDINFO1 );
	// �ԔԊ֘A�}�̏��Z�b�g
	PayData.MediaKind2 = 0;	// ���Z�}�̎�ʃN���A
	memset( PayData.MediaCardNo2, 0, sizeof(PayData.MediaCardNo2) );
	memset( card_info, 0, sizeof(card_info) );
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:228,229)�Ή�
//	for( cnt = 0; cnt < ONL_MAX_MEDIA_NUM; cnt++ ){
	for( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ){
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:228,229)�Ή�
		if( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType == CARD_TYPE_CAR_NUM ){	// ���Z�}�̎�� = �Ԕ�
			PayData.MediaKind2 = CARD_TYPE_CAR_NUM;
			NTNET_Data152Save( (void *)&PayData.MediaKind2, NTNET_152_MEDIAKIND2 );
			memcpy( PayData.MediaCardNo2, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo, sizeof(PayData.MediaCardNo2) );
			NTNET_Data152Save( (void *)PayData.MediaCardNo2, NTNET_152_MEDIACARDNO2 );
			NTNET_Data152Save( (void *)card_info, NTNET_152_MEDIACARDINFO2 );
			break;
		}
	}

// GG124100(S) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
	// �Ԕԏ��
	memcpy(&PayData.ShabanInfo, &lcdbm_rsp_in_car_info_main.ShabanInfo, sizeof(SHABAN_INFO));
// GG124100(E) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g

// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
	// NT-NET���Z�f�[�^�̒��Ԍ�/������͈ꎞ���p�t���O�ɏ�������ׁA
	// �K�v�ȃJ�[�h������ɕێ����Ă����B
	pMaster = &(lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo);
	pZaisha = &(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo);

	// �ꎞ���p�t���O
	ntNet_56_SaveData.NormalFlag = pZaisha->NormalFlag;

	// ���Ԍ��J�[�h���
	for ( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ) {
		if ( pMaster->stCardDataInfo[cnt].CardType == CARD_TYPE_PARKING_TKT ) {	// ���Ԍ�
			// ���Ԍ��@�B��(���ɋ@�B��)(���3���A0�`255)�A���Ԍ��ԍ�(����6���A0�`999999)
			memcpy(ntNet_56_SaveData.MachineTktNo,
				&(pMaster->stCardDataInfo[cnt].byCardNo),
				sizeof(ntNet_56_SaveData.MachineTktNo));

			break;
		}
	}

	// ������J�[�h���
	for ( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ) {
		if ( (pMaster->stCardDataInfo[cnt].CardType == CARD_TYPE_PASS) &&		// �����
			 (pMaster->stCardDataInfo[cnt].ParkingLotNo) ) {					// ��������ԏꇂ����
			for ( index = 0; index < 4; index++ ) {
				// ��������ԏꇂ����{/�g�����ԏꇂ�ƈ�v���邩�m�F����B
				if ( pMaster->stCardDataInfo[cnt].ParkingLotNo == CPrmSS[S_SYS][index+1] ) {
					// ��������ԏꇂ
					ntNet_56_SaveData.ParkingNo = pMaster->stCardDataInfo[cnt].ParkingLotNo;

					// �����ID(5���A0�`12000)
					ntNet_56_SaveData.id = astoin(pMaster->stCardDataInfo[cnt].byCardNo, 5);

					// ��������(2���A1�`15)
					ntNet_56_SaveData.syu = pMaster->SeasonKind;

					break;
				}
			}

			break;
		}
	}
// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX

// GG132000(S) ���Z�����^���~����NT-NET�ʐM�̐��Z�f�[�^�Ƀf�[�^���ڂ�ǉ�
	ntNet_56_SaveData.MaxFeeApplyFlg	= lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.MaxFeeApplyFlg;
	ntNet_56_SaveData.MaxFeeApplyCnt	= lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.MaxFeeApplyCnt;
	ntNet_56_SaveData.MaxFeeSettingFlg	= lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.MaxFeeSettingFlg;
// GG132000(E) ���Z�����^���~����NT-NET�ʐM�̐��Z�f�[�^�Ƀf�[�^���ڂ�ǉ�
}
// MH810100(E) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)

/*[]----------------------------------------------------------------------[]*/
/*| �ҋ@                                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod00( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = �������֐؊�                              |*/
/*|                       : -1 = Ӱ�ސؑ�                                  |*/
/*|                       : 10 = ���(���~)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod00( void )
{
	short	ret = 0;
	ushort	msg = 0;
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	uchar	key_chk;
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	short	tim2_mov = 0;
	uchar	uc_buf_size;
	uchar	reci_sw_flg = 0;
// MH810103(s) �d�q�}�l�[�Ή� �ÓI���
//	ushort	e_pram_set;												// �d�q�Ȱ�g�p�ݒ�
	ushort	e_pram_set = 0;												// �d�q�Ȱ�g�p�ݒ�
// MH810103(e) �d�q�}�l�[�Ή� �ÓI���
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	uchar	MntLevel = 0;
//	uchar	PassLevel = 0;
//	uchar	KakariNo = 0;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
	short	ans;
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	short	wk_lev;
//	char	f_Card = 0;												// 0:�s���W������ 1:�W�����ގ� -1:�W������NG
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	uchar	rd_err_count = 0;
	ushort	tim_mandet;												// �l�̌��o�M���}�X�N����
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	ushort	lcdGuardTm = 0;
//	ushort	lcdGuardInterval = prm_get(COM_PRM, S_PAY, 62, 2, 1);	// �X�N���[���Z�[�o���s�Ԋu(��)
//	ushort	lcdGuardSpan = 25;										// �X�N���[���Z�[�o���s��500ms(x20ms)
	ushort	res_wait_time;

	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// �f�[�^��M�Ď��^�C�}
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// �͈͊O��5�b�Ƃ���
	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#if (6 == AUTO_PAYMENT_PROGRAM)
	ushort	auto_pay_timer = (ushort)prm_get(COM_PRM, S_ECR, 100, 3, 1);
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

	OPECTL.ChkPassSyu = 0;											// ������⍇��������ر
	RD_Credit_kep = 0;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// �����^�C���A�E�g�t���O�N���A
	OPECTL.op_faz = 0;												// ���ڰ���̪���
	an_vol_flg = 0;
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH810105(S) MH364301 �C���{�C�X�Ή�
	f_reci_ana = 0;
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
	GyouCnt_All_r = 0;
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	GyouCnt_All_j = 0;
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129001(S) �C���{�C�X�Ή��i�t���O�̃N���A�����j
	OPECTL.f_RctErrDisp = 0;
// GG129001(E) �C���{�C�X�Ή��i�t���O�̃N���A�����j
	if( OPECTL.Pay_mod != 2 ){										// �C�����Z����?
		OpeLcd( 1 );												// �ҋ@��ʕ\��
	}
	rd_shutter();													// ���Cذ�ް���������
	OPE_red = 2;													// ذ�ް�����r�o
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
	OPECTL.remote_wait_flg = 0;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
	OPECTL.f_rtm_remote_pay_flg = 0;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	LaneStsSeqNoUpdate();
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

	Lagtim( OPETCBNO, 18, RD_ERR_INTERVAL );						// ��ϰ18(10s)������ُ픻��p��ϰ

#if (4 == AUTO_PAYMENT_PROGRAM)
	OPECTL.Seisan_Chk_mod = ON;
#endif

	if( OPECTL.Seisan_Chk_mod == ON ){								// ���Z����Ӱ�ށH
// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
	// ���ɏ�Ԃɂ��Ȃ�
#else
		op_indata_set();											// ���ɏ�Ծ��
#endif
	}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#if (6 == AUTO_PAYMENT_PROGRAM)
	is_auto_pay = 0;
	// �������Z�J�n�^�C�}�[
	if(auto_pay_timer == 0) {
		auto_pay_timer = 5;
	}
	auto_pay_timer *= 50;
	Lagtim( OPETCBNO, TIMERNO_AUTO_PAYMENT_TIMEOUT, auto_pay_timer );
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ذ�޺����(���Ԍ��ۗ���)
	}

	if( MIFARE_CARD_DoesUse ){										// Mifare���L���ȏꍇ
		op_MifareStop_with_LED();								 	// Mifare���~����
	}

	if( OPECTL.NtnetTimRec ){										// NT-NET���v��M�׸�ON
		NTNET_Snd_Data104(NTNET_MANDATA_CLOCK);						// NT-NET�Ǘ��ް��v���쐬
		OPECTL.NtnetTimRec = 0;										// NT-NET���v��M�׸�OFF
	}

	key_num = 0;													// �����͒l�ر
	if( CPrmSS[S_TYP][81] ){										// ���g�p�ݒ�L��?
		key_sec = 1;												// ����[�`]�Ƃ���(���g�p����)
	}else{
		key_sec = 0;												// �����͒l�ر(���g�p���Ȃ�)
	}

	Lagtim( OPETCBNO, 1, 10*50 );									// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
	tim1_mov = 1;													// ��ϰ1�N����
	if(( OPECTL.RECI_SW == (char)-1 )&&( RECI_SW_Tim == 0 )){		// �ҋ@���̎������݉�?
		RECI_SW_Tim = (30 * 2) + 1;									// �̎��؎�t���Ԃ�30s�Œ�(500ms�ɂĶ���)
	}
	Lagtim( OPETCBNO, 4, 1*50 );									// ��ϰ4(1s)�N��(�s�������o�ɊĎ��p)����̂�1s�Ƃ���
	Lagtim( OPETCBNO, 26, 3*50 );									// ���د��\���p��ϰ(3s)START
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	dspErrMsg = 0;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

	OPECTL.Ent_Key_Sts = OFF;										// �o�^��������ԁF�n�e�e
	OPECTL.Can_Key_Sts = OFF;										// �����������ԁF�n�e�e
	OPECTL.EntCan_Sts  = OFF;										// �o�^�������������������ԁF�n�e�e
	OPECTL.Apass_off_seisan = 0;									// ��������߽OFF�ł̒�����p�׸�OFF

	Chu_Syu_Status = 0;
	memset( DspWorkerea, 0, sizeof( DspWorkerea ));							// ܰ��ر�̏�����
	memset( DspChangeTime, 0, sizeof( DspChangeTime ));						// ܰ��ر�̏�����
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	memset( &g_PipCtrl, 0, sizeof(g_PipCtrl) );
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
// MH810100(S) 2020/06/17 #4497�y�A���]���w�E�����zAi-�X�����Ɏ������芄������̉��u���Z���s���ƁA�s���ȁi�O�񐸎Z���ɓK�������j�������ēK�����Đ��Z���Ă��܂�
	memset( &lcdbm_rsp_in_car_info_main, 0, sizeof(lcdbm_rsp_in_car_info_t) );
// MH810100(E) 2020/06/17 #4497�y�A���]���w�E�����zAi-�X�����Ɏ������芄������̉��u���Z���s���ƁA�s���ȁi�O�񐸎Z���ɓK�������j�������ēK�����Đ��Z���Ă��܂�
// MH810105(S) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
	memset( &RTPay_Data, 0, sizeof(RTPay_log) );					// RT���Z�f�[�^�N���A
// MH810105(E) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	memset( &RTReceipt_Data, 0, sizeof(RTReceipt_log) );			// RT�̎��؃f�[�^�N���A
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

// MH810103(s) �d�q�}�l�[�Ή� �s�v�@�\�폜
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////	Card_Status_Prev = 0;
////	if( ((e_pram_set = (ushort)prm_get(COM_PRM, S_PAY, 24, 1, 3)) == 1 ) ||
////		((e_pram_set = (ushort)prm_get(COM_PRM, S_PAY, 24, 1, 4)) == 1 )) {
//	if( (e_pram_set = (ushort)prm_get(COM_PRM, S_PAY, 24, 1, 3)) == 1 ) {
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		Ope_EleUseDsp();
//	}
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
//	else if(isEC_USE()) {
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100
	e_pram_set = 0;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100
	if(isEC_USE()) {
// MH810103(e) �d�q�}�l�[�Ή� �s�v�@�\�폜
		e_pram_set = 1;
		Ope_EcEleUseDsp();
// MH810103(s) �d�q�}�l�[�Ή� #5413 ���Z�s��ʂ܂��͌x����ʕ\�����ɁAVP���c���Ɖ�{�^����\�����Ă��܂��Ă���
		// mod00���ߒʒm
		LcdBackLightCtrl(ON);
// MH810103(e) �d�q�}�l�[�Ή� #5413 ���Z�s��ʂ܂��͌x����ʕ\�����ɁAVP���c���Ɖ�{�^����\�����Ă��܂��Ă���
	}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810104(S) MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
	// �܂����O����f�[�^���������Ă��Ȃ���Ώ�������
	if( ac_flg.ec_recv_deemed_fg != 0 ){
		EcRecvDeemed_RegistPri();
	}
// MH810104(E) MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
	if( OPECTL.Pay_mod == 2 ){									// �C�����Z����?
		OPECTL.Ope_mod = 11;									// �C�����Z���Ԉʒu�ԍ����͏���
		return(1);
	}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//	if(( key_num = Key_Event_Get() )){
//		queset( OPETCBNO, DUMMY_KEY_EVENT, 0, NULL );
//	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
	// NOTE:�l�̌��m�M�����o������������ꍇ�͂���ȍ~�ɉ����邱��
	tim_mandet = (ushort)prm_get(COM_PRM, S_PAY, 32, 2, 3);			// �l�̌��o�M���}�X�N����
	if(tim_mandet != 0 && f_MandetMask == 1 ){
		Lagtim( OPETCBNO, TIMER_MANDETMASK, (ushort)(tim_mandet*50) );	// �l�̌��m�Z���T�[�}�X�N�^�C�}�[�J�n
	}else{
		f_MandetMask = 0;
	}
	
	if(0 == f_MandetMask && 1 == FNT_MAN_DET) {					// �}�X�NOFF���l�̌��m ON
		goto	op_mod00_MANDET_ON;
	}
// MH810100(S) K.Onodera 2020/04/09 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
	// ���Z��Ԓʒm
	dspCyclicErrMsgRewrite();
// MH810100(E) K.Onodera 2020/04/09 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
//// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//	// �܂����O����f�[�^���������Ă��Ȃ���Ώ�������
//	if( ac_flg.ec_recv_deemed_fg != 0 ){
//		EcRecvDeemed_RegistPri();
//	}
//// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
// GG129000(S) H.Fujinaga 2022/12/09 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i#6411 �d�q�}�l�[�ݒ�Ȃ��̐ݒ�ŃT�[�r�X��(QR)�Ő��Z�������ɗ̎��ؔ��s�{�^�����^�b�`�\�ɂȂ��Ă���j
	// �d�q�W���[�i���ڑ���Ԃ�ʒm
	lcdbm_notice_ope(LCDBM_OPCD_EJA_USE,isEJA_USE());
// GG129000(E) H.Fujinaga 2022/12/09 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i#6411 �d�q�}�l�[�ݒ�Ȃ��̐ݒ�ŃT�[�r�X��(QR)�Ő��Z�������ɗ̎��ؔ��s�{�^�����^�b�`�\�ɂȂ��Ă���j

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		if( Op_Event_Disable( msg ) )								// �C�x���g�}�~�����`�F�b�N
			continue;
		Ope_ArmClearCheck( msg );

		switch( msg ){
			case KEY_DOOR1:											// Door1 key event
			case KEY_DOOR1_DUMMY:									// Edy�����Z���^�[�ʐM���̃h�A�C�x���g
				if( OPECTL.on_off == 0 ){							// OFF(Door Close)
					if( DoorCloseChk() < 100 ){						// �ޱ������ NG
						ret = -1;
						break;
					}
				}
				break;
			case CLOCK_CHG:											// ���v�X�V
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( edy_auto_com ){
//					queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL ); // �c�x�ƕω��ʒm�o�^
//				}else{
				{
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					// �C���f���g�����ׁ̈A�ȉ��̏����Ƀ^�u������B
					if( opncls() == 2 ){								// �x��?
						ret = -1;
						break;
					}
				}
				dspclk(0, COLOR_DARKSLATEBLUE);						// ���v�\��
				if( tim1_mov == 0 ){								// ��ϰ1���N��
					LcdBackLightCtrl( OFF );						// back light OFF
				}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//				if(lcdGuardInterval != 0){
//					lcdGuardTm++;
//					if( lcdGuardTm >= lcdGuardInterval ){			// �X�N���[���Z�[�o���s�Ԋu
//						dispclr();
//						lcdGuardTm = 0;
//						Lagtim( OPETCBNO, 28, lcdGuardSpan );		// �X�N���[���Z�[�o���s��
//					}
//				}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
				break;

			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}else{
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
				}
				ret = -1;
				fus_subcal_flg = 0;
				break;

			case OPE_OPNCLS_EVT:									// �����c�x�� event
				if( opncls() == 2 ){								// �x��?
					ret = -1;
				}
// MH810105(S) MH364301 �C���{�C�X�Ή�
				if (f_reci_ana) {
					// �̎��؃{�^�������ς݂ł���΁A
					// ���V�[�g�󎚎��s������s��
					op_rct_failchk();
				}
// MH810105(E) MH364301 �C���{�C�X�Ή�
				break;

			case MID_STSCHG_CHGEND:									// �ނ�؂�װѕω�
			case MID_STSCHG_R_PNEND:								// ڼ�Ď��؂�ω�
			case MID_STSCHG_J_PNEND:								// �ެ��َ��؂�ω�
				if( DspSts == LCD_WMSG_OFF ){						// ܰ�ݸ�ү���ޕ\����Ԃ��n�e�e
					dspCyclicErrMsgRewrite();
				}
			case IBK_LPR_ERR_REC:									// ����������װ�ް���M
				LCDNO = (ushort)-1;
				OpeLcd( 1 );										// �ҋ@��ʍĕ\��
				break;

// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			case KEY_TEN0:											// �ݷ�[0] �����
//			case KEY_TEN1:											// �ݷ�[1] �����
//			case KEY_TEN2:											// �ݷ�[2] �����
//			case KEY_TEN3:											// �ݷ�[3] �����
//			case KEY_TEN4:											// �ݷ�[4] �����
//			case KEY_TEN5:											// �ݷ�[5] �����
//			case KEY_TEN6:											// �ݷ�[6] �����
//			case KEY_TEN7:											// �ݷ�[7] �����
//			case KEY_TEN8:											// �ݷ�[8] �����
//			case KEY_TEN9:											// �ݷ�[9] �����
//			case DUMMY_KEY_EVENT:									// ���Z������ʂ���̃L�[����
//				if (tim2_mov) {
//					tim2_mov = 0;									// F1���������ꂽ���׸ނ�ر
//					Lagcan( OPETCBNO, 2 );							// F1�������Ď����
//				}
//				if( auto_syu_prn == 2 || 							// �����W�v���H
//// MH322914 (s) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
////					(AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) ){	// ���Z���O�������ݒ�
//					(AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) || // ���Z���O�������ݒ�
//					( ryo_inji != 0 ) ){	 						// �̎��؈󎚒�
//// MH322914 (e) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
//					// �����W�v���͐��Z����s�Ƃ���ׁA�ݷ��̓��͂��K������
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				LcdBackLightCtrl( ON );								// back light ON
//				if( msg != DUMMY_KEY_EVENT )
//				key_num = msg - KEY_TEN;
//				OPECTL.Ope_mod = 1;									// ���Ԉʒu�ԍ����͏�����
//				ret = 1;											// ������
//				break;
//
//			case KEY_TEN_CL:										// �ݷ�[C] �����
//				if( auto_syu_prn == 2 || 							// �����W�v��
//					OPECTL.Pay_mod == 1 ||							// �Ïؔԍ��Y�ꐸ�ZӰ�ގ�
//// MH322914 (s) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
////					(AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) ){	// ���Z���O�������ݒ�
//					(AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) || // ���Z���O�������ݒ�
//					( ryo_inji != 0 ) ){	 						// �̎��؈󎚒�
//// MH322914 (e) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
//					// �����W�v���͐��Z����s�Ƃ���ׁA�ݷ��̓��͂��K������
//					BUZPIPI();
//					break;
//				}
//				if( key_sec ){										// ���g�p����?
//					OPECTL.Ope_mod = 1;								// ���Ԉʒu�ԍ����͏�����
//					BUZPI();
//					LcdBackLightCtrl( ON );							// back light ON
//					ret = 1;										// ������
//					break;
//				}
//				if ((CPrmSS[S_PAY][41] % 10) != 0) {
//					if( tim2_mov == 1 ){
//						BUZPI();
//						LcdBackLightCtrl( ON );						// back light ON
//						OPECTL.Ope_mod = 200;						// �Ïؔԍ�(Btype)���͑����
//						ret = 1;
//						break;
//					}
//				}
//
//#if	UPDATE_A_PASS
//				if( CPrmSS[S_KOU][1] ){								// ������X�V�@�\����?
//					if( !( LprnErrChk( 1 ) && prm_get( COM_PRM,S_KOU,26,1,1 ) ) ){
//						BUZPI();
//						LcdBackLightCtrl( ON );						// back light ON
//						// �װ�Ȃ����װ���ł����Z�̐ݒ�̎��̂ݒ�����X�V��������
//						OPECTL.Ope_mod = 21;						// ������X�V ���}��������
//						ret = 1;									// ������
//						break;
//					}
//				}
//#endif	// UPDATE_A_PASS
//				BUZPIPI();
//				LcdBackLightCtrl( ON );								// back light ON
//				Lagtim( OPETCBNO, 1, 10*50 );						// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//				tim1_mov = 1;										// ��ϰ1�N����
//				break;
//
//			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
//				if (tim2_mov) {
//					tim2_mov = 0;									// F1���������ꂽ���׸ނ�ر
//					Lagcan( OPETCBNO, 2 );							// F1�������Ď����
//				}
//				Lagtim( OPETCBNO, 1, 10*50 );						// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//				tim1_mov = 1;										// ��ϰ1�N����
//				if( OPECTL.RECI_SW == (char)-1 ){					// �ҋ@���̎������݉�?
//					BUZPI();
//					LcdBackLightCtrl( ON );								// back light ON
//					ryo_isu( 0 );									// �̎��ؔ��s
//					OPECTL.RECI_SW = 0;								// �̎������ݖ��g�p
//
//					RECI_SW_Tim = 0;								// �̎��؎�t���Ըر
//					reci_sw_flg = 1;								// �̎������s�s��
//// MH810105(S) MH364301 �C���{�C�X�Ή�
//					f_reci_ana = 1;									// �������ɗ̎��ؔ��s
//// MH810105(E) MH364301 �C���{�C�X�Ή�
//				}
//				else {
//					BUZPIPI();
//					LcdBackLightCtrl( ON );							// back light ON
//				}
//				break;
//			case KEY_TEN_F1:										// �ݷ�[���Z] �����
//			case KEY_TEN_F3:										// �ݷ�[��t��] �����
//			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
//				if (tim2_mov) {
//					tim2_mov = 0;									// F1���������ꂽ���׸ނ�ر
//					Lagcan( OPETCBNO, 2 );							// F1�������Ď����
//				}
//				if ((CPrmSS[S_PAY][41] % 10) != 0) {
//					if( msg == KEY_TEN_F5 ){		// �ݷ�[�o�^] �����
//						//�Ïؔԍ�(Btype)�o�ɂ��g�p����ݒ�ł������Ͻ���
//						Lagtim( OPETCBNO, 2, 3*50 );					// ��ϰ2(3s)�N��(F1��3s�����Ď��p)
//						tim2_mov = 1;
//					}
//				}
//				BUZPIPI();
//				LcdBackLightCtrl( ON );								// back light ON
//				Lagtim( OPETCBNO, 1, 10*50 );						// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//				tim1_mov = 1;										// ��ϰ1�N����
//				break;
//			case KEY_TEN_F4:										// �ݷ�[���] �����
//				fus_subcal_flg = 0;
//				if (tim2_mov) {
//					tim2_mov = 0;									// F1���������ꂽ���׸ނ�ر
//					Lagcan( OPETCBNO, 2 );							// F1�������Ď����
//				}
//				if( OPECTL.Pay_mod == 1 ){							// �߽ܰ�ޖY�ꏈ��?
//					BUZPI();
//					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
//					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
//					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					ret = -1;
//				}else{
//					BUZPIPI();
//					LcdBackLightCtrl( ON );							// back light ON
//					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//					tim1_mov = 1;									// ��ϰ1�N����
//				}
//				StackCardEject(3);									/* �J�[�h�l�܂莞�̔r�o���� */
//				break;
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�s�v�����폜)

// MH810100(S) Y.Yamauchi 2020/03/05 #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�
			case KEY_TEN_CL:
				if( OPECTL.on_off == 1 ){
					LcdBackLightCtrl( ON );
					Lagtim( OPETCBNO, 1, 10*50 );						// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					tim1_mov = 1;										// ��ϰ1�N����
				}
				break;
// MH810100(E) Y.Yamauchi 2020/03/05 #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�

			case KEY_MANDET:										// �l�̌��m�ݻ����� �����
				if( f_MandetMask ){
					break;											// �}�X�N���̓C�x���g����
				} else if(tim_mandet != 0){// �l�̌��m�Z���T�[�}�X�N���ԗL
					Lagtim( OPETCBNO, TIMER_MANDETMASK, (ushort)(tim_mandet*50) );	// �l�̌��m�Z���T�[�}�X�N�^�C�}�[�J�n
					f_MandetMask = 1;
				}
op_mod00_MANDET_ON:
				LcdBackLightCtrl( ON );								// back light ON
				Lagtim( OPETCBNO, 1, 10*50 );						// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
				tim1_mov = 1;										// ��ϰ1�N����
				ope_anm( AVM_AUX );									// �l�̌��m��(�ݷ��ȊO��������)�ųݽ
				break;

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//			case ARC_CR_R_EVT:										// ����IN
//				cr_service_holding = 0;								// ���޽���ۗ̕�����
//				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
//				if (((m_kakari *)&MAGred[MAG_ID_CODE])->kkr_type == 0x20) {		// �W���J�[�h
//				f_Card = Kakariin_card();
//				if( 1 == f_Card ) {
//					if(CPrmSS[S_PAY][17]) {							// �d�����b�N����H
//						Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
//						// �d�����b�N�������x���ȏ�̌W���J�[�h�}�����̓��b�N����
//						if(PassLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
//							LcdBackLightCtrl( ON );					// �ޯ�ײ� ON
//							OPE_SIG_OUT_DOORNOBU_LOCK(0);			// �ޱ��ނ�ۯ�����
//							wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ����o�^(�A�����b�N)
//							Ope2_WarningDisp( 6*2, sercret_Str[3]);	// ��ۯ��޲��ݽ�\��
//							if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// �ޱ�J��� ��ۯ�������ϰ�l���L���͈͓�
//								DoorLockTimer = 1;
//								Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// �ޱ�J��� ��ۯ�������ϰ����
//							}
//						}
//						// �d�����b�N�������x�������̏ꍇ�͉������Ȃ��őҋ@��ʂ̂܂�
//						continue;
//					}
//				}
//				else if( 0 == f_Card ) {							// �s���J�[�h�i���o�^�j
//					Ope2_WarningDisp( 6*2, ERR_CHR[3]);				// �s���޲��ݽ�\��
//				}
//				else {												// �J�[�hNG
//					Ope2_WarningDisp( 6*2, ERR_CHR[1]);				// �s���޲��ݽ�\��
//				}
//				}
//				else {
//				wk_lev = hojuu_card();
//				if( NG != wk_lev && UsMnt_mnyctl_chk() ){			// ��[��������
//					// ��[���ނ̏ꍇ
//					LcdBackLightCtrl( ON );							// �ޯ�ײ� ON
//					OPECTL.Mnt_mod = 4;								// ����ݽӰ�ށ���[��������ݽӰ��
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					OPECTL.Mnt_lev = (char)wk_lev;
//					Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
//					OPECTL.Kakari_Num = KakariNo;					// �W��No.���Z�b�g
//					ret = -1;										// Ӱ�ސؑ�
//				}
//				}
//				break;
//
//			case ARC_CR_EOT_EVT:									// ���ޔ������
//				Ope2_WarningDispEnd();								// �޲��ݽ����
//				if( RD_mod < 9 ){
//					rd_shutter();									// 500msec��ڲ��ɼ��������
//				}
//				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

			case TIMEOUT1:											// ��ϰ1��ѱ��(���ڰ��ݐ���p)
				fus_subcal_flg = 0;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( edy_dsp.BIT.edy_zangaku_dsp )					// �c�z�\�����̏ꍇ
//					break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				tim1_mov = 0;										// ��ϰ1���N��
				LcdBackLightCtrl( OFF );							// back light OFF
				LedReq( CN_TRAYLED, LED_OFF );						// ��ݎ�o�����޲��LED����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( Edy_Rec.edy_status.BIT.ZAN_SW ){
//					Edy_StopAndLedOff();							// ���ތ��m��~��LED OFF
//					Edy_Rec.edy_status.BIT.ZAN_SW = 0;				// �c�z�Ɖ����݉����ς�ؾ��
//					DspWorkerea[3] = 0;								// 6�s�ڂɋ󔒕\������
//					grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]] );	// 6�s�ڂɉ�ʐؑ֗p�\��������
//				}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				break;
			case TIMEOUT2:										// ��ϰ2��ѱ��(�̎��ؖt��t�Ď��p)
				if ((CPrmSS[S_PAY][41] % 10) != 0) {					// �����ԍ�����
					//�Ïؔԍ�(Btype)�o�ɂ��g�p����ݒ�ł������Ͻ���
					if (tim2_mov == 1) {
						// F1�Ď���ϋN�����ł���΍ċN��
						Lagtim( OPETCBNO, 2, 3*50 );					// ��ϰ2(3s)�N��(F1��3s�����Ď��p)
					}
				}
				break;
			case MID_RECI_SW_TIMOUT:								// �̎��ؔ��s�\��ϰ��ѱ�Ēʒm
				OPECTL.RECI_SW = 0;									// �̎������ݖ��g�p
				reci_sw_flg = 1;									// �̎������s�s��
				break;

			case TIMEOUT4:											// ��ϰ4��ѱ��
				Lagtim( OPETCBNO, 4, 5*50 );						// ��ϰ4(5s)�N��(�s�������o�ɊĎ��p)
				break;

// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			case TIMEOUT6:											// ��ϰ6��ѱ�āF�u�o�^�v�����u����v������������ԊĎ���ϰ
//
//				if( READ_SHT_flg == 1 ){							// �������ԁH
//					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
//						// ���C���[�_�Ȃ��̏ꍇ�̓V���b�^�[�J���Ȃ�
//						break;
//					}
//					Lagtim( OPETCBNO, 7, 30*50 );					// ��ϰ7�i������J��Ԍp���Ď���ϰ�j�N���i�R�O�b�j
//					read_sht_opn();									// �̏ꍇ�F�J����
//				}
//				else{
//					read_sht_cls();									// �J�̏ꍇ�F����
//				}
//				break;
//			case TIMEOUT7:											// ��ϰ7��ѱ�āF������J��Ԍp���Ď���ϰ
//				rd_shutter();										// ���Cذ�ް���������
//				break;
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			case TIMEOUT11:														// Suica�p��ϰ
				Ope_TimeOut_11( OPECTL.Ope_mod,  e_pram_set );
				break;

			case TIMEOUT10:														// ��ʐؑ֗p��ϰ�Ƃ��Ďg�p
				Ope_TimeOut_10( OPECTL.Ope_mod,  e_pram_set );
				break;

// MH321800(S) G.So IC�N���W�b�g�Ή�
			case EC_CYCLIC_DISP_TIMEOUT:										// ��ʐؑ֗p��ϰ�Ƃ��Ďg�p
				Ope_TimeOut_Cyclic_Disp( OPECTL.Ope_mod,  e_pram_set );
				break;

			case EC_RECEPT_SEND_TIMEOUT:										// ec�p��ϰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,  e_pram_set );
				break;
// MH321800(E) G.So IC�N���W�b�g�Ή�

			case KEY_RESSW1:															// �c�z�Ɖ����ݲ����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( !Edy_Rec.edy_status.BIT.ZAN_SW ){
//					if( Dsp_Prm_Setting == 10 ){										// Edy�ݒ�݂̂���Edy�֘A�װ�������̏ꍇ
//						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ��ϰ1(03-0067)�����N��(���ڰ��ݐ���p) // 07-01-19�ύX //
//						tim1_mov = 1;													// ��ϰ1�N���� // 07-01-16�ǉ� //
//						LcdBackLightCtrl( ON );											// back light ON
//						Edy_Rec.edy_status.BIT.ZAN_SW = 1;								// �c�z�Ɖ����݉����ςݾ��
//						Edy_SndData01();												// ���ތ��m�J�n�w�����M
//						op_mod01_dsp_sub();												// �c�z�Ɖ�\�}�̕\������
//						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]] );					// 6�s�ڂɉ�ʐؑ֗p�\��������
//						DspWorkerea[0] = 1;
//					}
//					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );				// ��ʐؑ֗p��ϰ�N��(Timer10)
//				}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			case IBK_EDY_RCV:														// EdyӼޭ�ق���̎�M�ް�����
//				ret = Ope_Edy_Event( msg, OPECTL.Ope_mod );
//				break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			case TIMEOUT18:
				shutter_err_chk( &rd_err_count );										// ���Cذ�ް�ُ�������
				break;
			case TIMEOUT26:
				if( DspSts == LCD_WMSG_OFF ){						// ܰ�ݸ�ү���ޕ\����Ԃ��n�e�e
					dspCyclicErrMsgRewrite();
				}
				Lagtim( OPETCBNO, 26, 3*50 );						// ���د��\���p��ϰ(3s)
				break;
			case TIMEOUT27:
				f_MandetMask = 0;
				break;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//			case TIMEOUT28:
//				OpeLcd( 1 );								// �ҋ@��ʕ\��
//				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			case OPE_REQ_CALC_FEE:							// �����v�Z�v��
				// �����v�Z
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);		// �S�Ԏ����e�[�u�����M
				break;

// MH322914(S) K.Onodera 2016/08/18 AI-V�Ή��F�U�֐��Z
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//			// �U�֑Ώۏ��擾�v��
//			case OPE_REQ_FURIKAE_TARGET_INFO:
//				ans = carchk( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, 0 );	// ���Ԉʒu�ԍ��`�F�b�N
//				if( ans != 0 ){
//					// �o�ɏ�� or NG
//					if( ans != 1 && ans != 4 ){
//						// NG����
//						NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_DST );	// �U�֐�����G���[
//						break;
//					}
//				}
//				// �t���O�N���A
//				cm27();
//				init_ryocalsim();
//				// �����v�Z
//				ans = (short)Ope_PipFurikaeCalc();
//				ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
//				NTNET_Snd_Data16_04( (ushort)ans );
//				break;
//
//			// �U�֗v��
//			case OPE_REQ_FURIKAE_GO:
//				// �U�֌��Ԏ����O�^�C�}��~
//				if( ope_Furikae_start(vl_frs.lockno) ){
//					;
//				} else {
//					// NG����
//					NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_SRC );	// �U�֌������G���[
//					break;
//				}
//				ans = carchk( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, 0 );	// ���Ԉʒu�ԍ��`�F�b�N
//				if( ans != 0 ){
//					// �o�ɏ�� or NG
//					if( ans != 1 && ans != 4 ){
//						// NG����
//						NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_DST );	// �U�֐�����G���[
//						break;
//					}
//				}
//				op_mod01_Init();								// op_mod01() ����߂̂��ߴر�̂ݸر
//				OPECTL.Ope_mod = 2;								// �����\����ʂ�
//				PiP_FurikaeStart();
//				memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET���Z�ް��t�����ر
//				cm27();											// �׸޸ر
//				LcdBackLightCtrl( ON );
//				ret = 1;										// ������
//				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH322914(E) K.Onodera 2016/08/18 AI-V�Ή��F�U�֐��Z
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)
			// ���u���Z�����v�Z�v��
			case OPE_REQ_REMOTE_CALC_TIME_PRE:

// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z�i���A���^�C���j�J�n��t�ς�
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
				// �N���W�b�gHOST�ڑ����H
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_BUSY );		// NG����
					break;
				}

				// �t���O�N���A
				cm27();
				init_ryocalsim();
				// �����v�Z
				ans = (short)Ope_PipRemoteCalcTimePre();
				NTNET_Snd_Data16_08( (ushort)ans );
				break;

			// ���u���Z���Ɏ����w�萸�Z�v��
			case OPE_REQ_REMOTE_CALC_TIME:

// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z�i���A���^�C���j�J�n��t�ς�
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
				// �N���W�b�gHOST�ڑ����H
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_BUSY );		// NG����
					break;
				}
// MH810100(S) K.Onodera  2019/12/25 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//				ans = carchk( g_PipCtrl.stRemoteTime.Area, g_PipCtrl.stRemoteTime.ulNo, 0 );	// ���Ԉʒu�ԍ��`�F�b�N
//				if( ans > 2 ){
//					// NG����
//					switch( ans ){
//						case 3:			// 3 = �x�Ə��(���ү��A����ذ�ް�ʐM�ُ�)
//						case 4:			// 4 = ���O�^�C���� 
//						case 10:		// 10 = NG(ۯ����u��Ԃ��K��O��) 
//							NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_DENY);
//							break;
//						default:
//							NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_NO_CAR);
//							break;
//					}
//					break;
//				}
// MH810100(E) K.Onodera  2019/12/25 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//				Ope_Set_tyudata();							// �����v�Z�p�ɒ��Ԍ��ް����Z�b�g����B
//				read_sht_cls();									// ذ�ް������ELED OFF
//				op_mod01_Init();								// op_mod01() ����߂̂��ߴر�̂ݸر
//				memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET���Z�ް��t�����ر
//				OPECTL.Ope_mod = 2;								// �����\����ʂ�
//				OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;	// �����v�Z�J�n�v��=���Ԍ��ް���M
//				cm27();											// �׸޸ر
//				vl_now = V_CHM;
//				LcdBackLightCtrl( ON );
//				ret = 1;										// ������
//				break;
				// ���u���Z�v���̉����҂�����Ȃ��H
				if( OPECTL.remote_wait_flg == 0 ){
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
					if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
						// ���Ɏ����w�艓�u���Z�J�n
						lcdbm_remote_time_start();
					} else {					// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

					// ���u���Z�J�n���M
					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
					}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

					// ���u���Z�J�n�����҂��t���OON
					OPECTL.remote_wait_flg = 1;
					// �����҂��^�C�}�X�^�[�g
					Lagtim( OPETCBNO, TIMERNO_REMOTE_PAY_RESP, (res_wait_time * 50) );
				}
				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)

// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
			case OPE_REQ_REMOTE_CALC_FEE:							// ���u���Z���z�w�萸�Z�v��

// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z�i���A���^�C���j�J�n��t�ς�
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
				// �N���W�b�gHOST�ڑ����H
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG����
					break;
				}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//				// ������Z
//				if( !g_PipCtrl.stRemoteFee.Type ){
//// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//// MH810100(S) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iParkiPRO�Ή��j
////					ans = carchk( g_PipCtrl.stRemoteFee.Area, g_PipCtrl.stRemoteFee.ulNo, 0 );	// ���Ԉʒu�ԍ��`�F�b�N
////					if( ans > 2 ){
////						// NG����
////						switch( ans ){
////							case 3:			// 3 = �x�Ə��(���ү��A����ذ�ް�ʐM�ُ�)
////							case 4:			// 4 = ���O�^�C���� 
////							case 10:		// 10 = NG(ۯ����u��Ԃ��K��O��) 
////								NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_DENY);
////								break;
////							default:
////								NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_NO_CAR);
////								break;
////						}
////						break;
////					}
//					OPECTL.Pr_LokNo = 1;
//// MH810100(E) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iParkiPRO�Ή��j
//// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//				}
//				// �o�ɐ��Z
//				else{
//					OPECTL.Pr_LokNo = 0;
//// MH810100(S) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iParkiPRO�Ή��j
////					OPECTL.Op_LokNo = (g_PipCtrl.stRemoteFee.Area*10000L + g_PipCtrl.stRemoteFee.ulNo);
//// MH810100(E) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iParkiPRO�Ή��j
//				}
//// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//
//				// �������Z�`�F�b�N
//				if( Ope_FunCheck(1) ){
//// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//					// ����ʒm(���u���Z�J�n)���M	0�Œ�
//					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );
//// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//
//					read_sht_cls();									// ذ�ް������ELED OFF
//					op_mod01_Init();								// op_mod01() ����߂̂��ߴر�̂ݸر
//					memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET���Z�ް��t�����ر
//					OPECTL.Ope_mod = 2;								// �����\����ʂ�
//					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_FEE;		// �����v�Z�J�n�v��=�������Z
//					cm27();											// �׸޸ر
//					vl_now = V_FUN;
//					LcdBackLightCtrl( ON );
//					ret = 1;										// ������
//				}
				// ���u���Z�v���̉����҂�����Ȃ��H
				if( OPECTL.remote_wait_flg == 0 ){
					// ���u���Z�J�n���M
					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );
					// ���u���Z�J�n�����҂��t���OON
					OPECTL.remote_wait_flg = 2;
					// �����҂��^�C�}�X�^�[�g
					Lagtim( OPETCBNO, TIMERNO_REMOTE_PAY_RESP, (res_wait_time * 50) );
				}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
				break;
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH810105(S) MH364301 �C���{�C�X�Ή�
			case PRIEND_PREQ_RYOUSYUU:
				if (IS_INVOICE) {
					if (OPECTL.Pri_Kind == J_PRI) {
						// �W���[�i����
						if (OPECTL.f_ReIsuType) {
							// �Z���̎��؁i�������Z�Ȃǁj�󎚂ň󎚒��Ƀt�^���J�����ꍇ�A
							// �󎚊������󎚎��s�Ƃ������Œʒm�����ꍇ������
							// ���̂��߁A�󎚊�����M���Wait�^�C�}�𓮍삳����
							// �^�C�}���쒆�Ɉ󎚎��s����M�����ꍇ�͈󎚎��s�Ƃ��Ĉ���
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
//							LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, JNL_PRI_WAIT_TIME, op_jnl_failchk);
							LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, jnl_pri_wait_Timer, op_jnl_failchk);
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
						}
					}
				}
				else {
					if (f_reci_ana && OPECTL.Pri_Result == PRI_NML_END) {
						f_reci_ana = 0;
					}
				}
				break;
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#if (6 == AUTO_PAYMENT_PROGRAM)
			case AUTO_PAYMENT_TIMEOUT:
				BUZPI();
				LcdBackLightCtrl( ON );
				is_auto_pay = 1;
				key_num = auto_payment_locksts_set();
				OPECTL.Ope_mod = 1;
				ret = 1;
				break;
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
			// ���ɏ��
			case LCD_IN_CAR_INFO:
// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
//				if ( OPECTL.remote_wait_flg == 1 ) {	// ���u���Z(���Ɏ����w��)
				if ( OPECTL.remote_wait_flg == 1 ||		// ���u���Z(���Ɏ����w��)
					 OPECTL.f_rtm_remote_pay_flg != 0) {	// ���u���Z�i���A���^�C���j
// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
					// �^�C�}��~
					Lagcan(OPETCBNO, TIMERNO_REMOTE_PAY_RESP);

					// ���ɏ��(���ޏ��)�������Ɛݒ菈��
					lcdbm_rsp_in_car_info_proc();

					// ���ڰ����������1���Z�I����ōs���׸޸ر
					cm27();

					// �����v�Z�p�ɒ��Ԍ��f�[�^���Z�b�g����
// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
//					Ope_Set_tyudata();
					if (OPECTL.remote_wait_flg == 1) {	// ���u���Z(���Ɏ����w��)
// GG129004(S) M.Fujikawa 2024/12/09 ��ROMDB�w�E�����Ή��i2024/11/27 ��؁j
//						OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;
						Ope_Set_tyudata();
// GG129004(E) M.Fujikawa 2024/12/09 ��ROMDB�w�E�����Ή��i2024/11/27 ��؁j
					}
					else {								// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
// GG129004(S) M.Fujikawa 2024/12/09 ��ROMDB�w�E�����Ή��i2024/11/27 ��؁j
//						OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_LCD_IN_TIME;
						Ope_Set_tyudata_Card();
// GG129004(E) M.Fujikawa 2024/12/09 ��ROMDB�w�E�����Ή��i2024/11/27 ��؁j
					}
// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j

					// op_mod02(�����\���A��������)
					OPECTL.Ope_mod = 2;

					// �����v�Z�p�ɃZ�b�g
					OPECTL.Pr_LokNo = 1;

					// ���Z�J�n�g���K�[�Z�b�g
// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
//					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;
					if (OPECTL.remote_wait_flg == 1) {	// ���u���Z(���Ɏ����w��)
						OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;
					}
					else {								// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
						OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_LCD_IN_TIME;
					}
					OPECTL.f_rtm_remote_pay_flg = 0;
// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j

					// ����VL�J�[�h�f�[�^ = ���Z�O��
					vl_now = V_CHM;
// GG129003(S) M.Fujikawa 2024/11/22 �W���s��C���@GM858400(S)[�}�[�W] �N���E�h�����v�Z�Ή�(���P�A��7�A8:���u���Z���̏��Z�b�g�R��)
					LcdBackLightCtrl( ON );
// GG129003(E) M.Fujikawa 2024/11/22 �W���s��C���@GM858400(S)[�}�[�W] �N���E�h�����v�Z�Ή�(���P�A��7�A8:���u���Z���̏��Z�b�g�R��)

					ret = 1;
				}

				break;
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			// QR�f�[�^
			case LCD_QR_DATA:
				// QR�����f�[�^
				lcdbm_QR_data_res( 1 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
				break;

			// ����ʒm
			case LCD_OPERATION_NOTICE:
				// ����R�[�h
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// ���Z���~�v��
					case LCDBM_OPCD_PAY_STP:
// GG129004(S) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
//// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
//						if (OPECTL.f_rtm_remote_pay_flg != 0) {
//							// ���u���Z�i���A���^�C���j�J�n��t�ς�
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
//							break;
//						}
//// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
						if(OPECTL.f_rtm_remote_pay_flg != 0){
							OPECTL.f_rtm_remote_pay_flg = 0;
						}
// GG129004(E) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
						// ����ʒm(���Z���~����(OK))���M	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
						break;

					// ���Z�J�n�v��
					case LCDBM_OPCD_PAY_STA:
						// �����W�v���H or ���Z���O�������ݒ� or �̎��؈󎚒� or ���u���Z�v���̉����҂��H
// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
//						if( auto_syu_prn == 2 || (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) || ( ryo_inji != 0 ) || OPECTL.remote_wait_flg ){
						if( auto_syu_prn == 2 || (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) ||
							( ryo_inji != 0 ) || OPECTL.remote_wait_flg ||
							OPECTL.f_rtm_remote_pay_flg != 0 ){
// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );	// NG
// MH810100(S) Y.Yamauchi 2020/03/05 #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�
							LcdBackLightCtrl( ON );
							Lagtim( OPETCBNO, 1, 10*50 );						// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
							tim1_mov = 1;										// ��ϰ1�N����
// MH810100(E) Y.Yamauchi 2020/03/05 #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�
						}else{
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );	// OK
							OPECTL.Ope_mod = 1;		// op_mod01(�Ԕԓ���) = ���ޏ��҂�
							ret = 1;
// MH810100(S) Y.Yamauchi 2020/03/13 #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�
							LcdBackLightCtrl( ON );
// MH810100(E) Y.Yamauchi 2020/03/13 #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�
						}
						break;

					// ���u���Z�J�n����
					case LCDBM_OPCD_RMT_PAY_STA_RES:
						// �^�C�}��~
						Lagcan( OPETCBNO, TIMERNO_REMOTE_PAY_RESP );

						// �n�j�H
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							// ���Ɏ����w��
							if( OPECTL.remote_wait_flg == 1 ){
								OPECTL.Pr_LokNo = 1;
								cm27();											// �׸޸ر
								Ope_Set_tyudata();								// �����v�Z�p�ɒ��Ԍ��ް����Z�b�g����B
								op_mod01_Init();								// op_mod01() ����߂̂��ߴر�̂ݸر
								memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET���Z�ް��t�����ر
								OPECTL.Ope_mod = 2;								// �����\����ʂ�
								OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;	// �����v�Z�J�n�v��=���Ԍ��ް���M
								vl_now = V_CHM;
								LcdBackLightCtrl( ON );
								ret = 1;										// ������
							}
							// ���z�w��
							else if( OPECTL.remote_wait_flg == 2 ){
								// �o�ɐ��Z
								if( !g_PipCtrl.stRemoteFee.Type ){
									OPECTL.Pr_LokNo = 1;
								}
								// ������Z
								else{
									OPECTL.Pr_LokNo = 0;
								}
								Ope_FunCheck(1);
								op_mod01_Init();								// op_mod01() ����߂̂��ߴر�̂ݸر
								memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET���Z�ް��t�����ر
								OPECTL.Ope_mod = 2;								// �����\����ʂ�
								OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_FEE;		// �����v�Z�J�n�v��=�������Z
								cm27();											// �׸޸ر
								vl_now = V_FUN;
								LcdBackLightCtrl( ON );
								ret = 1;										// ������
							}
						}
						// �m�f�H
						else{
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// 							OPECTL.remote_wait_flg = 0;	// �⍇�����t���O�N���A
// 							NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG����
							if ( OPECTL.remote_wait_flg == 1 ) {		// ���u���Z(���Ɏ����w��)
								OPECTL.remote_wait_flg = 0;									// �⍇�����t���O�N���A
								NTNET_Snd_Data16_08(LcdRecv.lcdbm_rsp_notice_ope.status);	// NG����
							} else if( OPECTL.remote_wait_flg == 2 ) {	// ���u���Z(���z�w��)
								OPECTL.remote_wait_flg = 0;									// �⍇�����t���O�N���A
								NTNET_Snd_Data16_09(LcdRecv.lcdbm_rsp_notice_ope.status);	// NG����
							}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
						}
						break;
// GG129000(S) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j
					// ���u���Z�i���A���^�C���j�J�n�v��
					case LCDBM_OPCD_RTM_REMOTE_PAY_REQ:
// GG129004(S) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
//						if (OPECTL.remote_wait_flg != 0 ||
//							OPECTL.f_rtm_remote_pay_flg != 0) {
						if (OPECTL.remote_wait_flg != 0) {
// GG129004(E) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
							// ���u���Z�iPIP�j�J�n�v����
							// ���u���Z�i���A���^�C���j�J�nNG
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 1 );
						}
						else {
							OPECTL.f_rtm_remote_pay_flg = 1;
							// ���u���Z�i���A���^�C���j�J�nOK
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
						}
						break;
					// ���u���Z�i���A���^�C���j��t���~
					case LCDBM_OPCD_RTM_REMOTE_PAY_CAN:
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							OPECTL.f_rtm_remote_pay_flg = 0;
						}
						break;
// GG129000(E) T.Nagai 2023/10/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i�ҋ@��Ԃł���t����j

					// �N���ʒm
					case LCDBM_OPCD_STA_NOT:
						// �N��Ӱ�� = �ݒ豯��۰�ދN��
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(�N����)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// �N��Ӱ�� = �ʏ�N��
						else {
							// op_mod00(�ҋ@)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;
// MH810100(S) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(�N�������ʒm��M�l���R��C��)
					// �N�������ʒm
					case LCDBM_OPCD_STA_CMP_NOT:
						ope_idle_transit_common();	// �ҋ@��ԑJ�ڋ��ʏ���
						OPECTL.init_sts = 1;
						OPECTL.Ope_mod = 0;
						ret = -1;
						break;

					default:
						break;
// MH810100(E) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(�N�������ʒm��M�l���R��C��)
				}
				break;

			// ���u���Z�J�n�����҂��^�C���A�E�g
			case TIMEOUT_REMOTE_PAY_RESP:
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// 				OPECTL.remote_wait_flg = 0;	// �⍇�����t���O�N���A
// 				NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG����
				if ( OPECTL.remote_wait_flg == 1 ) {		// ���u���Z(���Ɏ����w��)
					OPECTL.remote_wait_flg = 0;						// �⍇�����t���O�N���A
					NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_OTHER);	// NG����
				} else if( OPECTL.remote_wait_flg == 2 ) {	// ���u���Z(���z�w��)
					OPECTL.remote_wait_flg = 0;						// �⍇�����t���O�N���A
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_OTHER);	// NG����
				}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(�N����)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// ��������������ԂƂ���
				ret = -1;
				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

			default:
				break;
		}

// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//		key_chk = op_key_chek();									// �u�o�^�v�L�[���u����v�L�[�̓���������ԊĎ�
//		switch( key_chk ){
//			case	1:												// ����������ԊJ�n
//				Lagtim( OPETCBNO, 6, 3*50 );						// ��ϰ6�i���������Ď���ϰ�j�N���i�R�b�j
//				break;
//			case	2:												// ����������ԉ���
//				Lagcan( OPETCBNO, 6 );								// ��ϰ6�i���������Ď���ϰ�jؾ��
//				break;
//		}
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�s�v�����폜)
		if (tim2_mov) {
			if (OPECTL.on_off == 0) {
				// F1����������
				Lagcan( OPETCBNO, 2 );								// ��ϰ2�iF1�������Ď���ϰ�jؾ��
				tim2_mov = 0;
			}
		}
		lto_syuko();												// �ׯ�ߏ㏸�A���b�N����ϓ��o��
		if(	fus_subcal_flg == 0 ){
			if( fus_kyo() == 2 ){									// �s���E�����W�v����(1=�����o��,2=�s���o��)
				if( prm_get(COM_PRM,S_NTN,63,1,5) == 0 ){	// �������M���Ȃ��ݒ�̏ꍇ
					// ���̎��_��FROM�ւ̏����݂͏I����Ă��� or �����ݗv���Ȃ�
					ntautoReqToSend( NTNET_BUFCTRL_REQ_SALE );	// ���Z�f�[�^���M�v��
				}
			}
		}
		mc10exec();
	}

	if((reci_sw_flg) || ( OPECTL.RECI_SW == (char)-1 )){
		if(Flap_Sub_Flg == 1){
			if(Flap_Sub_Num < 9){
				uc_buf_size = (uchar)(9 - Flap_Sub_Num);
				memset(&FLAPDT_SUB[Flap_Sub_Num],0,sizeof(flp_com_sub));			// �ڍג��~�ر(���Z����)�ر
				memmove(&FLAPDT_SUB[Flap_Sub_Num],&FLAPDT_SUB[Flap_Sub_Num+1],sizeof(flp_com_sub)*uc_buf_size);
			}
			if(Flap_Sub_Num == 10){
				memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));							// �I�[�ڍג��~�ر�ر
			}else{
				memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// �I�[�ڍג��~�ر�ر
			}
			Flap_Sub_Flg = 0;														// �ڍג��~�ر�g�p�t���OOFF
			OPECTL.RECI_SW = 0;														// �ҋ@��ʂ𔲂�����̎������s�s��
		}
	}

	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 2 );											// F1�������Ď����-
	Lagcan( OPETCBNO, 4 );											// ��ϰ4ؾ��(�s�������o�ɊĎ��p)
	Lagcan( OPETCBNO, 6 );											// ��ϰ6ؾ��
	Lagcan( OPETCBNO, 7 );											// ��ϰ7ؾ��
	Lagcan( OPETCBNO, 10 );											// Suica�c���\���p��ϰؾ��
	Lagcan( OPETCBNO, 11 );											// ��ʐؑ֗p��ϰؾ��
	Lagcan( OPETCBNO, 18 );											// ��ϰ18(10s)������ُ픻��p��ϰSTOP
	Lagcan( OPETCBNO, 26 );											// ��ϰ26(3s)�ޑK�s���A�p���s���̻��د���ϰSTOP
	Lagcan( OPETCBNO, TIMER_MANDETMASK );							// �l�̌��m�}�X�N���ԃ��Z�b�g (27)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	LagCan500ms( LAG500_EDY_LED_RESET_TIMER );						// Edy�֘A��ϰ����
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) G.So IC�N���W�b�g�Ή�
	Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );					// ec���T�C�N���\���p��ϰؾ��
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	Lagcan( OPETCBNO, TIMERNO_REMOTE_PAY_RESP );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810105(S) MH364301 �C���{�C�X�Ή�
	LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);							// �W���[�i���󎚊����҂��^�C�}
	LagCan500ms(LAG500_RECEIPT_MISS_DISP_TIMER);					// �̎��؎��s�\���^�C�}
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#if (6 == AUTO_PAYMENT_PROGRAM)
	Lagcan( OPETCBNO, TIMERNO_AUTO_PAYMENT_TIMEOUT );
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_StopAndLedOff();											// ���ތ��m��~�w���J�n��LEDOFF
//	Edy_Rec.edy_status.BIT.ZAN_SW = 0;								// �c�z�Ɖ����݉����ς�ؾ��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	op_SuicaFusokuOff();											// 6�E7�s�ڂ̱װт�����
	ope_anm( AVM_STOP );

	if( OPECTL.Mnt_mod != 4 ){										// �u��[���ގ�t�����v�ȊO�H
		rd_shutter();												// ���Cذ�ް���������
	}

	LedReq( CN_TRAYLED, LED_OFF );									// ��ݎ�o�����޲��LED����
	f_MandetMask = 0;

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Ԉʒu�ԍ����͏���                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod01( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = �������֐؊�                              |*/
/*|                       : -1 = Ӱ�ސؑ�                                  |*/
/*|                       : 10 = ���(���~)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod01( void )
{
	short	ret;
	short	ans;
	ushort	msg = 0;
// GG129000(S) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
	ushort result = 0;
// GG129000(E) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	stParkKindNum_t	stQRTicket;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//// MH322914(S) K.Onodera 2016/12/22 [�ÓI���-583]��������
////	char	setwk;
//	char	setwk = 0;
//// MH322914(E) K.Onodera 2016/12/22 [�ÓI���-583]��������
//	char	pri_time_flg;
//	ushort	key_num_wk = key_num;			// �Ïؔԍ��o�ɗp�L�[���͒l(4��)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	OpeLcd( 2 );													// ���Ԉʒu�ԍ��\��
//
//	ope_anm( AVM_ICHISEL );											// �Ԏ��ԍ��I�����ųݽ
	ushort	res_wait_time;

	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// �f�[�^��M�Ď��^�C�}
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// �͈͊O��5�b�Ƃ���
	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

// MH322914(S) K.Onodera 2016/09/21 AI-V�Ή�
//	OPECTL.multi_lk = 0L;											// ������Z�⍇�����Ԏ���
//	OPECTL.ChkPassSyu = 0;											// ������⍇��������ر
//	OPECTL.ChkPassPkno = 0L;										// ������⍇�������ԏꇂ�ر
//	OPECTL.ChkPassID = 0L;											// ������⍇���������ID�ر
//// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
//	OPECTL.CalStartTrigger = 0;
//// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
	op_mod01_Init();
// MH322914(E) K.Onodera 2016/09/21 AI-V�Ή�

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#if (6 == AUTO_PAYMENT_PROGRAM)
	if(is_auto_pay) {
		msg = KEY_TEN_F1;
		goto AUTO_PAYMENT_PROGRAM_START;
	}
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X�iOPE�Ή��j
//	Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X�iOPE�Ή��j

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( Edy_Rec.edy_status.BIT.INITIALIZE ){						// EdyӼޭ�ق̏��������������Ă���H
//		Edy_StopAndLedOff();										// ���ތ��m��~�w���J�n��LEDOFF
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#if (6 == AUTO_PAYMENT_PROGRAM)
AUTO_PAYMENT_PROGRAM_START:
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
		switch( msg ){

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//			// op_mod01()��loop��������KEY_MODECHG������ݽ�Ɉڍs���Ȃ��悤�ɂ���
//			case KEY_MODECHG:										// Mode change key
//				if( OPECTL.on_off == 1 ){							// key ON
//					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
//					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
//					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//				}else{
//					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
//					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
//				}
//				ret = -1;
//				break;
//
//			case KEY_TEN0:											// �ݷ�[0] �����
//			case KEY_TEN1:											// �ݷ�[1] �����
//			case KEY_TEN2:											// �ݷ�[2] �����
//			case KEY_TEN3:											// �ݷ�[3] �����
//			case KEY_TEN4:											// �ݷ�[4] �����
//			case KEY_TEN5:											// �ݷ�[5] �����
//			case KEY_TEN6:											// �ݷ�[6] �����
//			case KEY_TEN7:											// �ݷ�[7] �����
//			case KEY_TEN8:											// �ݷ�[8] �����
//			case KEY_TEN9:											// �ݷ�[9] �����
//				if( OPECTL.multi_lk ){								// ������Z�⍇�����ͷ��͗����Ȃ�
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				key_num_wk = (( key_num_wk % 1000 ) * 10 )+( msg - KEY_TEN );
//				key_num = (( key_num % knum_ket ) * 10 )+( msg - KEY_TEN );
//				teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
//				break;
//			case KEY_TEN_CL:										// �ݷ�[C] �����
//				if( OPECTL.multi_lk ){								// ������Z�⍇�����ͷ��͗����Ȃ�
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				key_num = 0;
//				key_num_wk = 0;
//				teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
//				break;
//			case KEY_TEN_F1:										// �ݷ�[���Z] �����
//				if( OPECTL.multi_lk ){								// ������Z�⍇�����ͷ��͗����Ȃ�
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
//				if(key_num_wk != 0 && key_num_wk == CPrmSS[39][6]){
//					OPECTL.Ope_mod = 200;
//					key_num = 0;
//					key_num_wk = 0;									// �Ïؔԍ��o�ɗp�L�[���͒l
//					ret = 1;
//					break;
//				}
//				ans = carchk( key_sec, key_num, 0 );				// ���Ԉʒu�ԍ�����
//				setwk = (char)Carkind_Param(FLP_ROCK_INTIME, (char)(LockInfo[OPECTL.Pr_LokNo-1].ryo_syu),1,1);
//				if(( ans == -1 )||( ans == 10 )||( ans == 2 )||		// ���Z�s�\ or ���ݒ� or �o�ɏ��
//				   (( ans == 1 )&&( setwk == 0 ))){					// ۯ����u�J�n�҂� and ۯ����u�J�n��ϰ�N�����ɐ��Z���Ȃ��ݒ�
//					if( ans == -1						&&				// ���Z�s�\
//						( OPECTL.Pay_mod != 1 ) 		&&				// �p�X���[�h�Y��o�ɂł͖���
//						_is_ntnet_normal()				&&				// NT-NET�ڑ��H
//						prm_get( COM_PRM,S_NTN,26,1,1 )) 				// �Ԏ��⍇������?
//					{
//						if( !ERR_CHK[mod_ntibk][1] ){				// NTNET IBK �ʐM����?
//							OPECTL.multi_lk = (ulong)(( key_sec * 10000L ) + key_num );
//							NTNET_Snd_Data01( OPECTL.multi_lk );	// NTNET�Ԏ��⍇���ް��쐬
//							grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[70]);		// "      �����A�⍇�����ł�      "
//							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);		// "                              "
//							blink_reg(6, 5, 20, 0, COLOR_DARKSLATEBLUE, &OPE_CHR[7][5] );		// "     ���΂炭���҂�������     "
//							grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);		// "                              "
//							Lagtim( OPETCBNO, 2, (ushort)(prm_get( COM_PRM,S_NTN,30,2,3 )*50+1) );	// ��ϰ2(XXs)�N��(������Z�⍇���Ď�)
//						}else{
//							ope_anm( AVM_BTN_NG );					// �Ԏ��ԍ��ԈႢ���ųݽ
//							key_num = 0;
//							key_num_wk = 0;							// �Ïؔԍ��o�ɗp�L�[���͒l
//							teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
//							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );		// "                              "
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[7] );		// "  ���̐��Z�@�Ő��Z���ĉ�����  "
//							grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );		// "                              "
//							Lagtim( OPETCBNO, 4, 5*50 );			// ��ϰ4(5s)�N��(�װ�\���p)
//						}
//					}else{
//						ope_anm( AVM_BTN_NG );											// �Ԏ��ԍ��ԈႢ���ųݽ
//						key_num = 0;
//						teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
//						key_num_wk = 0;													// �Ïؔԍ��o�ɗp�L�[���͒l
//					}
//					break;
//				}
//				if(( ans == 0 )||									// OK or �x�Ə��(ү�,����ذ�ް�ʐM�ُ�) or
//				   (( ans == 1 )&&( setwk ))){						// ۯ����u�J�n�҂� and ۯ����u�J�n��ϰ�N�����ɐ��Z����ݒ�
//					if(( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd != 0 )&&	// �߽ܰ�ޗL��?
//					   ( OPECTL.Pay_mod != 1 )){					// �߽ܰ�ޖY�ꏈ���ȊO?
//						/*** �߽ܰ�ޓo�^�L��(�ʏ퐸�Z�̂�) ***/
//						OPECTL.Ope_mod = 70;						// �߽ܰ�ޓ��͏���(���Z��)��
//					}else{
//						/*** �߽ܰ�ޓo�^���� ***/
//						if( OPECTL.InLagTimeMode ) {				// ���O�^�C�������������s�H
//							OPECTL.Ope_mod = 220;					// ���O�^�C������������
//						}else{
//							OPECTL.Ope_mod = 2;						// �����\��,����������
//						}
//					}
//					cm27();											// �׸޸ر
//					ret = 1;
//				}
//				if( ans == 4  )										// ���O�^�C����
//				{
//					OPECTL.Ope_mod = 230;							// ���Z�ς݈ē�������
//					cm27();											// �׸޸ر
//					ret = 1;
//				}
//				break;
//
//			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
//				BUZPIPI();
//				break;
//			case KEY_TEN_F3:										// �ݷ�[��t��] �����
//				if( OPECTL.multi_lk ){								// ������Z�⍇�����ͷ��͗����Ȃ�
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
//				if( prm_get( COM_PRM,S_TYP,62,1,3 ) ){				// ���ԏؖ������s�\�����ݒ肠��
//					setwk = (char)prm_get( COM_PRM,S_TYP,68,1,2 );	// ۯ����u�J�n��ϰ�N�����Ɏ�t�����s���Ȃ�/����
//					ans = carchk( key_sec, key_num, 12);			// ���Ԉʒu�ԍ�����
//					if(( ans == -1 )||( ans == 10 )||( ans == 2 )||	// ���Z�s�\ or ���ݒ� or �o�ɏ��
//					   (( ans == 1 )&&( setwk == 0 ))){				// ۯ����u�J�n�҂� and ۯ����u�J�n��ϰ�N�����Ɏ�t�����s���Ȃ��ݒ�
//						ope_anm( AVM_BTN_NG );						// �Ԏ��ԍ��ԈႢ���ųݽ
//						key_num = 0;
//						teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
//						key_num_wk = 0;											// �Ïؔԍ��o�ɗp�L�[���͒l
//						break;
//					}
//				pri_time_flg = pri_time_chk(OPECTL.Pr_LokNo,0);
//				if(pri_time_flg == 0){
//					//�G���[���b�Z�[�W
//					BUZPIPI();
//					OPECTL.Ope_mod = 91;						// ��t�����s������
//					ret = 1;
//					break;
//				}
//				else if(Ope_isPrinterReady() == 0){
//					BUZPIPI();
//					OPECTL.Ope_mod = 92;						// ��t�����s������
//					shomei_errnum = 1;
//					ret = 1;
//					break;
//				}else{
//					if(( ans == 0 )||( ans == 3 )||					// OK or �x�Ə��(ү�,����ذ�ް�ʐM�ُ�) or
//					   (( ans == 1 )&&( setwk ))){					// ۯ����u�J�n�҂� and ۯ����u�J�n��ϰ�N�����Ɏ�t�����s����ݒ�
//						OPECTL.Ope_mod = 90;						// ��t�����s������
//						ret = 1;
//					}
//				}
//				}
//				break;
//			case KEY_TEN_F4:										// �ݷ�[���] �����
//				BUZPI();
//				ret = 10;											// �ҋ@�֖߂�
//				break;
//			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
//				if( OPECTL.multi_lk || OPECTL.Pay_mod == 1 ){		// ������Z�⍇�����܂��͈Ïؔԍ��Y�ꐸ�ZӰ�ގ��ͷ��͖����Ƃ���
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
//				if( prm_get( COM_PRM,S_TYP,63,1,1 ) ){
//					ans = carchk( key_sec, key_num, 3 );			// ���Ԉʒu�ԍ�����
//					if(( ans == -1 )||( ans == 10 )||( ans == 2 )){	// ���Z�s�\ or ���ݒ� or �o�ɏ��
//						ope_anm( AVM_BTN_NG );						// �Ԏ��ԍ��ԈႢ���ųݽ
//						key_num = 0;
//						teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
//						key_num_wk = 0;											// �Ïؔԍ��o�ɗp�L�[���͒l
//						break;
//					}
//				pri_time_flg = pri_time_chk(OPECTL.Pr_LokNo,1);
//				if(pri_time_flg == 0){
//					//�G���[���b�Z�[�W
//					BUZPIPI();
//					OPECTL.Ope_mod = 81;						// ��t�����s������
//					ret = 1;
//					break;
//				}else{
//					if(( ans == 0 )||( ans == 1 )||( ans == 3 )){	// OK or ۯ����u�J�n�҂� or �x�Ə��(ү�,����ذ�ް�ʐM�ُ�)
//						OPECTL.Ope_mod = 80;						// �߽ܰ�ޓo�^����(���Ɏ�)��
//						ret = 1;
//					}
//				}
//				}
//				break;
//
//			case ARC_CR_R_EVT:										// ����IN
//				cr_service_holding = 0;								// ���޽���ۗ̕�����
//				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
//				break;
//			case ARC_CR_EOT_EVT:									// ���ޔ������
//				if( RD_mod < 9 ){
//					Ope2_WarningDispEnd();							// �װ�\������
//					rd_shutter();									// 500msec��ڲ��ɼ��������
//				}
//				break;
//
//			case TIMEOUT1:											// ��ϰ1��ѱ��
//				if( OPECTL.multi_lk ){								// ������Z�⍇����?
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
//				}else{
//					ret = 10;
//				}
//				break;
//
//			case IBK_NTNET_LOCKMULTI:								// NTNET ������Z�p�ް�(�Ԏ��⍇�������ް�)��M
//				if( LOCKMULTI.Answer == 0 && OPECTL.multi_lk )		// ���ʂ�����
//				{
//					ans = carchk_ml( key_sec, key_num );			// ���Ԉʒu�ԍ�����
//					if(( ans == 0 )||								// OK or �x�Ə��(ү�,����ذ�ް�ʐM�ُ�) or
//					   (( ans == 1 )&&( setwk ))){					// ۯ����u�J�n�҂� and ۯ����u�J�n��ϰ�N�����ɐ��Z����ݒ�
//						if(( LOCKMULTI.lock_mlt.passwd != 0 )&&		// �߽ܰ�ޗL��?
//						   ( OPECTL.Pay_mod != 1 )){				// �߽ܰ�ޖY�ꏈ���ȊO?
//							/*** �߽ܰ�ޓo�^�L��(�ʏ퐸�Z�̂�) ***/
//							OPECTL.Ope_mod = 70;					// �߽ܰ�ޓ��͏���(���Z��)��
//						}else{
//							/*** �߽ܰ�ޓo�^���� ***/
//							OPECTL.Ope_mod = 2;						// �����\��,����������
//						}
//						OPECTL.Op_LokNo = LOCKMULTI.LockNo;
//						OPECTL.Pr_LokNo = 0xffff;
//						cm27();										// �׸޸ر
//						ret = 1;
//						break;
//					}
//					else{
//						LOCKMULTI.Answer = 1;						// ���ʁ�����^�Ԏ���ԁ����Ԓ��ȊO
//					}
//				}
//				else{
//					if( LOCKMULTI.Answer == 1 ){					// ID=02�i�����ް��j��M	���ʁ����ڑ��i�Y���Ȃ��j�H
//						LOCKMULTI.Answer = 2;						// ���ʁ����ڑ��i�Y���Ȃ��j
//					}
//				}
//
//				// no break
//			case TIMEOUT2:											// ��ϰ2��ѱ��
//				if( LOCKMULTI.Answer == 0 ){						// �Ԏ��⍇�������ް��҂���ѱ�āH
//					LOCKMULTI.Answer = 3;							// Yes�F�ʐM��Q��
//				}
//				if( OPECTL.multi_lk ){
//					OPECTL.multi_lk = 0;
//					ope_anm( AVM_BTN_NG );							// �Ԏ��ԍ��ԈႢ���ųݽ
//
//					key_num = 0;
//					blink_end();									// �_�ŏI��
//					teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
//					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );				// "                              "
//					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );				// "                              "
//					key_num_wk = 0;																	// �Ïؔԍ��o�ɗp�L�[���͒l
//
//					switch( LOCKMULTI.Answer ){
//						case	1:									// �Ԏ���ԁ����Ԓ��ȊO
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[6] );		// "  �Ԏ��ԍ�������������܂���  "
//							break;
//						case	2:									// ���ڑ�
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[12] );		// "   �Y������Ԏ��͂���܂���   "
//							break;
//						case	3:									// �ʐM��Q��
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[7] );		// "  ���̐��Z�@�Ő��Z���ĉ�����  "
//							break;
//						case	4:									// �蓮Ӱ�ޒ�
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[13] );		// "     �W���ɘA�����ĉ�����     "
//							break;
//						default:									// ���̑�
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[7] );		// "  ���̐��Z�@�Ő��Z���ĉ�����  "
//// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
//							break;
//// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
//					}
//					Lagtim( OPETCBNO, 4, 5*50 );					// ��ϰ4(5s)�N��(�װ�\���p)
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
//				}
//				break;
//
//			case TIMEOUT4:											// ��ϰ4��ѱ��
//				if( OPECTL.multi_lk == 0 ){
//					LCDNO = (ushort)-1;
//					OpeLcd( 2 );									// ���Ԉʒu�ԍ��\��(�ĕ\��)
//				}
//				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

			case MID_RECI_SW_TIMOUT:								// �̎��ؔ��s�\��ϰ��ѱ�Ēʒm
				OPECTL.RECI_SW = 0;									// �̎������ݖ��g�p
				break;

			case OPE_REQ_CALC_FEE:									// �����v�Z�v��
				// �����v�Z
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);				// �S�Ԏ����e�[�u�����M
				break;

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
//
//			// �U�֑Ώۏ��擾�v��
//			case OPE_REQ_FURIKAE_TARGET_INFO:
//				ans = carchk( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, 0 );	// ���Ԉʒu�ԍ��`�F�b�N
//				if( ans != 0 ){
//					// �o�ɏ�� or NG
//					if( ans != 1 && ans != 4 ){
//						// NG����
//						NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_SRC );	// �U�֌������G���[
//						break;
//					}
//				}
//				// �t���O�N���A
//				cm27();
//				init_ryocalsim();
//				// �����v�Z
//				ans = (short)Ope_PipFurikaeCalc();
//				ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
//				NTNET_Snd_Data16_04( (ushort)ans );
//				break;
//
//			// �U�֗v��
//			case OPE_REQ_FURIKAE_GO:
//				// �U�֌��Ԏ����O�^�C�}��~
//				if( ope_Furikae_start(vl_frs.lockno) ){
//					;
//				} else {
//					// NG����
//					NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_SRC );	// �U�֌������G���[
//					break;
//				}
//				ans = carchk( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, 0 );	// ���Ԉʒu�ԍ��`�F�b�N
//				if( ans != 0 ){
//					// �o�ɏ�� or NG
//					if( ans != 1 && ans != 4 ){
//						// NG����
//						NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_DST );	// �U�֐�����G���[
//						break;
//					}
//				}
//				OPECTL.Ope_mod = 2;								// �����\����ʂ�
//				PiP_FurikaeStart();
//				cm27();											// �׸޸ر
//				LcdBackLightCtrl( ON );
//				ret = 1;										// ������
//				break;
//// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F���u���Z
			// ���u���Z�����v�Z�v��
			case OPE_REQ_REMOTE_CALC_TIME_PRE:

// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z�i���A���^�C���j�J�n��t�ς�
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				// �N���W�b�gHOST�ڑ����H
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_BUSY );		// NG����
					break;
				}

				// �t���O�N���A
				cm27();
				init_ryocalsim();
				// �����v�Z
				ans = (short)Ope_PipRemoteCalcTimePre();
				NTNET_Snd_Data16_08( (ushort)ans );
				break;

			// ���u���Z���Ɏ����w�萸�Z�v��
			case OPE_REQ_REMOTE_CALC_TIME:

				// �N���W�b�gHOST�ڑ����H
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_BUSY );		// NG����
					break;
				}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//				ans = carchk( g_PipCtrl.stRemoteTime.Area, g_PipCtrl.stRemoteTime.ulNo, 0 );	// ���Ԉʒu�ԍ��`�F�b�N
//				if( ans > 2 ){
//					// NG����
//					switch( ans ){
//						case 3:			// 3 = �x�Ə��(���ү��A����ذ�ް�ʐM�ُ�)
//						case 4:			// 4 = ���O�^�C���� 
//						case 10:		// 10 = NG(ۯ����u��Ԃ��K��O��) 
//							NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_DENY);
//							break;
//						default:
//							NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_NO_CAR);
//							break;
//					}
//					break;
//				}
//				Ope_Set_tyudata();							// �����v�Z�p�ɒ��Ԍ��ް����Z�b�g����B
//				OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;			// �����v�Z�J�n�v��=���Ԍ��ް���M
//				read_sht_cls();									// ذ�ް������ELED OFF
//				OPECTL.Ope_mod = 2;
//				cm27();											// �׸޸ر
//				vl_now = V_CHM;
//				LcdBackLightCtrl( ON );
//				ret = 1;										// ������
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z�i���A���^�C���j�J�n��t�ς�
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				// ���u���Z�v���̉����҂�����Ȃ��H
				if( OPECTL.remote_wait_flg == 0 ){
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
					if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
						// ���Ɏ����w�艓�u���Z�J�n
						lcdbm_remote_time_start();
					} else {					// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

					// ���u���Z�J�n���M
					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
					}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

					// ���u���Z�J�n�����҂��t���OON
					OPECTL.remote_wait_flg = 1;
					// �����҂��^�C�}�X�^�[�g
					Lagtim( OPETCBNO, TIMERNO_REMOTE_PAY_RESP, (res_wait_time * 50) );
				}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
				break;
// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F���u���Z
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
			// ���u���Z���z�w�萸�Z�v��
			case OPE_REQ_REMOTE_CALC_FEE:
			
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z�i���A���^�C���j�J�n��t�ς�
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if( OPECTL.InquiryFlg ){								// �N���W�b�gHOST�ڑ����͎󂯕t���Ȃ�
					// NG����
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//				if( !g_PipCtrl.stRemoteFee.Type ){
//// MH810100(S) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iParkiPRO�Ή��j
////					ans = carchk( g_PipCtrl.stRemoteFee.Area, g_PipCtrl.stRemoteFee.ulNo, 0 );	// ���Ԉʒu�ԍ�����
////					if( ans > 2 ){
////						// NG����
////						switch(ans){
////						case 3:			// 3 = �x�Ə��(���ү��A����ذ�ް�ʐM�ُ�)
////						case 4:			// 4 = ���O�^�C���� 
////						case 10:		// 10 = NG(ۯ����u��Ԃ��K��O��) 
////							NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_DENY);
////							break;
////						default:
////							NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_NO_CAR);
////							break;
////						}
////						break;
////					}
//					OPECTL.Pr_LokNo = 1;
//// MH810100(E) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iParkiPRO�Ή��j
//				}else{
//					OPECTL.Pr_LokNo = 0;
//// MH810100(S) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iParkiPRO�Ή��j
////					OPECTL.Op_LokNo = (g_PipCtrl.stRemoteFee.Area*10000L + g_PipCtrl.stRemoteFee.ulNo);
//// MH810100(E) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iParkiPRO�Ή��j
//				}
//				if( Ope_FunCheck(1) ){								// �������Z���� (OK)
//					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_FEE;			// �����v�Z�J�n�v��=�������Z
//					read_sht_cls();								// ذ�ް������ELED OFF
//					OPECTL.Ope_mod = 2;								// �����\��,����������
//					cm27();											// �׸޸ر
//					vl_now = V_FUN;
//					LcdBackLightCtrl( ON );
//					ret = 1;										// ������
//				}
				// ���u���Z�v���̉����҂�����Ȃ��H
				if( OPECTL.remote_wait_flg == 0 ){
					// ���u���Z�J�n���M
					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );
					// ���u���Z�J�n�����҂��t���OON
					OPECTL.remote_wait_flg = 2;
					// �����҂��^�C�}�X�^�[�g
					Lagtim( OPETCBNO, TIMERNO_REMOTE_PAY_RESP, (res_wait_time * 50) );
				}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
				break;
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH321800(S) ��t�������M����Ȃ��s��C��
			case EC_RECEPT_SEND_TIMEOUT:							// ec�p��ϰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,0 );
				break;
// MH321800(E) ��t�������M����Ȃ��s��C��
// MH810105(S) MH364301 �C���{�C�X�Ή�
			case PRIEND_PREQ_RYOUSYUU:
				if (IS_INVOICE) {
					if (OPECTL.Pri_Kind == J_PRI) {
						// �W���[�i����
						if (OPECTL.f_ReIsuType) {
							// �Z���̎��؁i�������Z�Ȃǁj�󎚂ň󎚒��Ƀt�^���J�����ꍇ�A
							// �󎚊������󎚎��s�Ƃ������Œʒm�����ꍇ������
							// ���̂��߁A�󎚊�����M���Wait�^�C�}�𓮍삳����
							// �^�C�}���쒆�Ɉ󎚎��s����M�����ꍇ�͈󎚎��s�Ƃ��Ĉ���
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
//							LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, JNL_PRI_WAIT_TIME, op_jnl_failchk);
							LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, jnl_pri_wait_Timer, op_jnl_failchk);
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
						}
					}
				}
				else {
					if (f_reci_ana && OPECTL.Pri_Result == PRI_NML_END) {
						f_reci_ana = 0;
					}
				}
				break;
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

			// ���ɏ��
			case LCD_IN_CAR_INFO:

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
				if ( OPECTL.remote_wait_flg == 1 ) {	// ���u���Z(���Ɏ����w��)
					// �^�C�}��~
					Lagcan(OPETCBNO, TIMERNO_REMOTE_PAY_RESP);
				}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

				// ���ɏ��(���ޏ��)�������Ɛݒ菈��
				lcdbm_rsp_in_car_info_proc();

				// ���ڰ����������1���Z�I����ōs���׸޸ر
				cm27();

				// �����v�Z�p�ɒ��Ԍ��f�[�^���Z�b�g����
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// 				Ope_Set_tyudata_Card();
				if ( OPECTL.remote_wait_flg == 1 ) {	// ���u���Z(���Ɏ����w��)
					Ope_Set_tyudata();
				} else {								// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
					Ope_Set_tyudata_Card();
				}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

				// op_mod02(�����\���A��������)
				OPECTL.Ope_mod = 2;

				// �����v�Z�p�ɃZ�b�g
				OPECTL.Pr_LokNo = 1;

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// 				// ���Z�J�n�g���K�[�Z�b�g = �Ԕԃ`�P�b�g���X���Z
// 				OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_LCD_IN_TIME;
				// ���Z�J�n�g���K�[�Z�b�g
				if ( OPECTL.remote_wait_flg == 1 ) {	// ���u���Z(���Ɏ����w��)
					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;
				} else {								// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_LCD_IN_TIME;
				}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				OPECTL.f_rtm_remote_pay_flg = 0;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j

				// ����VL�J�[�h�f�[�^ = ���Z�O��
				vl_now = V_CHM;
				ret = 1;
				break;

			// QR�f�[�^
			case LCD_QR_DATA:
// GG129000(S) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
//				// QR�����f�[�^
//				lcdbm_QR_data_res( 1 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
//				break;
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z�i���A���^�C���j�J�n��t�ς�
					lcdbm_QR_data_res( 1 );	// ����(1:NG(�r��))
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				// mod01�ł�QR���Ԍ��̂ݎ�M
				if( ope_ParseBarcodeData( &MediaDetail ) ){
					result = ope_CanUseQRTicket( &MediaDetail );
					if( result == 1 ){
						lcdbm_QR_data_res( 4 );	// ����(4:�t�H�[�}�b�g�s��)
						break;
					}
				}
				// �ŏI�ǎ�QR�̕ێ�(�����e�i���X�p)
				push_ticket( &MediaDetail,(ushort)result);

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				// QR���Ԍ��̒��Ԍ��ԍ���ێ�
				memset( &stQRTicket, 0, sizeof(stQRTicket));
				stQRTicket.ParkingLotNo = MediaDetail.Barcode.QR_data.TicketType.ParkingNo;
				stQRTicket.CardType = CARD_TYPE_PARKING_TKT;
				intoas(&stQRTicket.byCardNo[0], MediaDetail.Barcode.QR_data.TicketType.EntryMachineNo, 3);
				intoasl(&stQRTicket.byCardNo[3], MediaDetail.Barcode.QR_data.TicketType.ParkingTicketNo, 6);
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

				// QR�����f�[�^
				lcdbm_QR_data_res( 0 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
				break;
// GG129000(E) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j

			// ����ʒm
			case LCD_OPERATION_NOTICE:

				// ����R�[�h
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// ���Z���~�v��
					case LCDBM_OPCD_PAY_STP:
// GG129004(S) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
//// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
//						if (OPECTL.f_rtm_remote_pay_flg != 0) {
//							// ���u���Z�i���A���^�C���j�J�n��t�ς�
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
//							break;
//						}
//// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
// GG129004(E) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
						// ���Z���~����(OK)���M
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
						// ���[�����j�^�f�[�^�o�^(���Z���~)
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.51)�iGM803000���p�j
						if(OPECTL.f_searchtype != 0xff) {				// �����^�C�v�Z�b�g�ォ
							ope_MakeLaneLog(LM_PAY_STOP);
						}
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.51)�iGM803000���p�j
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

						// op_mod00(�ҋ@)
						OPECTL.Ope_mod = 0;
						ret = -1;
						break;

					// ���Z�J�n�v��
					case LCDBM_OPCD_PAY_STA:
						// ���u���Z�v���̉����҂��H
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
//						if( OPECTL.remote_wait_flg ){
						if( OPECTL.remote_wait_flg || OPECTL.f_rtm_remote_pay_flg != 0 ){
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );	// NG
						}else{
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );	// OK
						}
						break;

					// ���u���Z�J�n����
					case LCDBM_OPCD_RMT_PAY_STA_RES:
						// �^�C�}��~
						Lagcan( OPETCBNO, TIMERNO_REMOTE_PAY_RESP );

						// �n�j�H
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							// ���Ɏ����w��
							if( OPECTL.remote_wait_flg == 1 ){
								OPECTL.Pr_LokNo = 1;
								cm27();										// �׸޸ر
								Ope_Set_tyudata();							// �����v�Z�p�ɒ��Ԍ��ް����Z�b�g����B
								OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;			// �����v�Z�J�n�v��=���Ԍ��ް���M
								OPECTL.Ope_mod = 2;
								vl_now = V_CHM;
								LcdBackLightCtrl( ON );
								ret = 1;										// ������
							}
							// ���z�w��
							else if( OPECTL.remote_wait_flg == 2 ){
								// �o�ɐ��Z
								if( !g_PipCtrl.stRemoteFee.Type ){
									OPECTL.Pr_LokNo = 1;
								}
								// ������Z
								else{
									OPECTL.Pr_LokNo = 0;
								}
								Ope_FunCheck(1);
								OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_FEE;			// �����v�Z�J�n�v��=�������Z
								OPECTL.Ope_mod = 2;								// �����\��,����������
								cm27();											// �׸޸ر
								vl_now = V_FUN;
								LcdBackLightCtrl( ON );
								ret = 1;										// ������
							}
						}
						// �m�f�H
						else{
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// 							OPECTL.remote_wait_flg = 0;	// �⍇�����t���O�N���A
// 							NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG����
							if ( OPECTL.remote_wait_flg == 1 ) {		// ���u���Z(���Ɏ����w��)
								OPECTL.remote_wait_flg = 0;									// �⍇�����t���O�N���A
								NTNET_Snd_Data16_08(LcdRecv.lcdbm_rsp_notice_ope.status);	// NG����
							} else if( OPECTL.remote_wait_flg == 2 ) {	// ���u���Z(���z�w��)
								OPECTL.remote_wait_flg = 0;									// �⍇�����t���O�N���A
								NTNET_Snd_Data16_09(LcdRecv.lcdbm_rsp_notice_ope.status);	// NG����
							}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
						}
						break;

					// �N���ʒm
					case LCDBM_OPCD_STA_NOT:
						// �N��Ӱ�� = �ݒ豯��۰�ދN��
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(�N����)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// �N��Ӱ�� = �ʏ�N��
						else {
							// op_mod00(�ҋ@)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					// �����^�C�v�ʒm
					case LCDBM_OPCD_SEARCH_TYPE:
						// �����^�C�v�Z�b�g
						OPECTL.f_searchtype = LcdRecv.lcdbm_rsp_notice_ope.status;

						// ���[�����j�^�f�[�^���M(�⍇����)
						switch(LcdRecv.lcdbm_rsp_notice_ope.status){
							// �ԔԌ���
							case SEARCH_TYPE_NO:
								// ���[�����j�^�f�[�^�o�^
								ope_MakeLaneLog(LM_INQ_NO);
								break;

							// ��������
							case SEARCH_TYPE_TIME:
								// ���[�����j�^�f�[�^�o�^
								ope_MakeLaneLog(LM_INQ_TIME);
								break;

							// QR����
							case SEARCH_TYPE_QR_TICKET:
								SetLaneMedia(stQRTicket.ParkingLotNo,
												stQRTicket.CardType,
												stQRTicket.byCardNo);

								// ���[�����j�^�f�[�^�o�^
								ope_MakeLaneLog(LM_INQ_QR_TICKET);
								break;
							default:
								break;
						}
						break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
					// ���u���Z�i���A���^�C���j�J�n�v��
					case LCDBM_OPCD_RTM_REMOTE_PAY_REQ:
// GG129004(S) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
//						if (OPECTL.remote_wait_flg != 0 ||
//							OPECTL.f_rtm_remote_pay_flg != 0) {
						if (OPECTL.remote_wait_flg != 0) {
// GG129004(E) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
							// ���u���Z�iPIP�j�J�n�v����
							// ���u���Z�i���A���^�C���j�J�n�v����t�ς�
							// ���u���Z�i���A���^�C���j�J�nNG
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 1 );
						}
						else {
							OPECTL.f_rtm_remote_pay_flg = 1;
							// ���u���Z�i���A���^�C���j�J�nOK
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
						}
						break;
					// ���u���Z�i���A���^�C���j��t���~
					case LCDBM_OPCD_RTM_REMOTE_PAY_CAN:
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							OPECTL.f_rtm_remote_pay_flg = 0;
						}
						break;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				}
				break;

			// ���u���Z�J�n�����҂��^�C���A�E�g
			case TIMEOUT_REMOTE_PAY_RESP:
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// 				OPECTL.remote_wait_flg = 0;	// �⍇�����t���O�N���A
// 				NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG����
				if ( OPECTL.remote_wait_flg == 1 ) {		// ���u���Z(���Ɏ����w��)
					OPECTL.remote_wait_flg = 0;						// �⍇�����t���O�N���A
					NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_OTHER);	// NG����
				} else if( OPECTL.remote_wait_flg == 2 ) {	// ���u���Z(���z�w��)
					OPECTL.remote_wait_flg = 0;						// �⍇�����t���O�N���A
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_OTHER);	// NG����
				}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(�N����)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// ��������������ԂƂ���
				ret = -1;
				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

			default:
				break;
		}
		mc10exec();
	}
   	if(OPECTL.Ope_mod != 2){					// �����Z�t�F�[�Y����Ȃ�
   		ope_anm( AVM_STOP );
   	}

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)
//	Lagcan( OPETCBNO, 2 );											// ��ϰ2ؾ��(������Z�⍇���Ď�)
//	Lagcan( OPETCBNO, 4 );											// ��ϰ4ؾ��(�װ�\���p)
//	blink_end();													// �_�ŏI��
	Lagcan( OPETCBNO, TIMERNO_REMOTE_PAY_RESP );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810105(S) MH364301 �C���{�C�X�Ή�
	LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);							// �W���[�i���󎚊����҂��^�C�}
	LagCan500ms(LAG500_RECEIPT_MISS_DISP_TIMER);					// �̎��؎��s�\���^�C�}
// MH810105(E) MH364301 �C���{�C�X�Ή�

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �����\��,��������                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod02( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret(OPECTL.Fin_mod)                                     |*/
/*|                  1 = ���Z����(�ނ薳��) ���Z���������֐؊�             |*/
/*|                  2 = ���Z����(�ނ�L��) ���Z���������֐؊�             |*/
/*|                  3 = ү��װ����         ���Z���������֐؊�             |*/
/*|                 10 = ���Z���~           �ҋ@�֖߂�                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 2007-02-26                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod02( void )
{
	short	ret;
	short	r_zero = -1;
	ushort	msg = 0;
	ushort	wk_MsgNo;
	uchar	Mifare_LastReadCardID[4];			// �Ō�ɓǂ񂾶���ID���L�����Ă������궰�ގ��͏������Ȃ��l�ɂ���
	uchar	syu_bk;

// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	short	credit_result = 0;
//	short	credit_turi;
//	short	credit_nyukin = 0;
//	uchar	credit_error_ari = 0;
//	uchar	credit_Timeout8Flag = 0;	// �����J�n����A��莞�ԍēx�����������ꍇ�A�������~����ꍇ�̃N���W�b�g�Ƃ̂����
//										// �����r���ł����Ă��A�N���W�b�g�₢���킹���̃^�C���A�b�v�͖������A
//										// �₢���킹�m�f�ŁA�^�C���A�b�v�ς̏ꍇ�ɏ]���̏������s�����߂̃t���O.
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	uchar	set03_98;					// �C�����Z�@�\�ݒ�
//	char	pri_ann_flg = 0;
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	uchar	ryo_announce_flg = 0;	// �����Ǐグ�ς݃t���O
	uchar	first_announce_flag = 0;
	uchar	nyukin_flag = 0;
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	uchar	announce_flag = 0;			// ����������̶��ޔ������ųݽ���t���O
	uchar	announce_end = 0;			// ����������̶��ޔ������ųݽ���ރt���O
	ushort	ec_removal_wait_time;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	ushort cnt = 0;
	uchar type = 0;
	OPE_RESULT result = RESULT_NO_ERROR;
	ushort	res_wait_time;	// �f�[�^��M�Ď��^�C�}
// MH810100(S) K.Onodera 2020/2/18 #3870 ������ʂɖ߂�܂Ő��Z�ł��Ȃ��Ȃ��Ă��܂�
	uchar require_cancel_res = 0;
// MH810100(E) K.Onodera 2020/2/18 #3870 ������ʂɖ߂�܂Ő��Z�ł��Ȃ��Ȃ��Ă��܂�
// MH810100(S) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
	short retOnline = 0;
// MH810100(E) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	uchar	carno[36];
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

// MH810100(S) K.Onodera  2020/03/06 �Ԕԃ`�P�b�g���X(�C�x���g�����̓x�Ɏc���ω��ʒm/����ʒm�����M�����s��C��)
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
//	uchar cmp_send = 0;
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
// MH810100(E) K.Onodera  2020/03/06 �Ԕԃ`�P�b�g���X(�C�x���g�����̓x�Ɏc���ω��ʒm/����ʒm�����M�����s��C��)
// MH810102(S) �d�q�}�l�[�Ή� #5476 �y�s��s��z���Z�{�^���������Ă��u���Z���J�n�ł��܂���ł����v�ƂȂ�J�n�ł��Ȃ�
	uchar flag_stop = 0;		// 1:���~��
// MH810102(E) �d�q�}�l�[�Ή� #5476 �y�s��s��z���Z�{�^���������Ă��u���Z���J�n�ł��܂���ł����v�ƂȂ�J�n�ł��Ȃ�
// MH810105(S) MH364301 �C���{�C�X�Ή�
	OPECTL.f_DelayRyoIsu = 0;
	OPECTL.f_CrErrDisp = 0;
// MH810105(E) MH364301 �C���{�C�X�Ή�

// MH810101(S) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
	season_chk_result = 0;
// MH810101(E) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
	cmp_send = 0;
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102���p�j�iGM803003���p�j
	r_zero_call = 0;												// 0�~���Z�ɂ��\���Z�k�t���O������
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102���p�j�iGM803003���p�j

	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// �f�[�^��M�Ď��^�C�}
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// �͈͊O��5�b�Ƃ���
	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	FurikaeCancelFlg = 0;		// �U�֐��Z���s�t���O
//	FurikaeMotoSts = 0;		// �U�֌���ԃt���O 0:�`�F�b�N�s�v / 1:�Ԃ��� / 2:�ԂȂ�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	OPECTL.InquiryFlg = 0;		// �O���Ɖ�t���OOFF
	MifStat = MIF_WAITING;				// Mifare�������

	creInfoInit();		// �N���W�b�g���N���A

	delay_count = 0;												// ������������ү���޶��Ă̸ر
	memset( nyukin_delay,0,sizeof( nyukin_delay ));					// ������������ү���ނ�ێ��̈�̸ر
	memset( &Settlement_Res,0,sizeof( Settlement_Res ));
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	coin_err_flg = 0;
	note_err_flg = 0;
	ec_flag_clear(0);												// ���σ��[�_�֘A�t���O�N���A
// MH321800(E) hosoda IC�N���W�b�g�Ή�

    edy_dsp.BYTE = 0;												// Edy��ʕ\���p�̈揉����
	cansel_status.BYTE = 0;											// ������̓d�q�}�̒�~��ԏ�����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Pay_Work = 0;												// Edy���ϊzܰ��̈�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	CCT_Cansel_Status.BYTE = 0;										// �ڼޯĶ��ގg�p���̊e�d�q�}�̂̒�~��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	time_out = 0;
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// �����^�C���A�E�g�t���O�N���A
	Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;

	Suica_Rec.Data.BIT.PAY_CTRL = 0;								// �d�q�}�̌��ύςݔ����׸޸ر���O�̂���
	Suica_rcv_split_flag_clear();

	Flap_Sub_Flg = 0;												// �ڍג��~�G���A�g�p�t���OOFF
	Pay_Flg = 0;		//�v���y�C�h�A�񐔌��g�p�׸޸ر

	NgCard = 0;														// NG�J�[�h�ǎ���e�N���A

	CreditReqResult = 0;											// �N���W�b�g�J�[�h�₢���킹���ʏ��ϐ�������
	OPECTL.credit_in_mony = 0;
// MH810100(S) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	f_al_card_wait = 0;												// �d�q�}�̒�~���̃J�[�h�}���t���O�N���A
// MH810100(E) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 0;							// ��������������t���O�N���A
	Product_Select_Data = 0;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Rec.edy_status.BIT.PRI_NG = 0;								// �W���[�i���v�����^�g�p�s���׸޸ر
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	Suica_Rec.Data.BIT.PRI_NG = 0;									// �W���[�i���v�����^�g�p�s���׸޸ر

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	ec_removal_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 13, 3, 1);
	if(ec_removal_wait_time == 0) {									// 0�̎��A360���
		ec_removal_wait_time = 360;
	} else if( ec_removal_wait_time < 30 ){
		ec_removal_wait_time = 30;
	}
	ec_removal_wait_time *= 50;										// Xs * 1000ms / 20ms
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

// MH810100(S) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4607 ���Z���Ɍ��σ��[�_�̃G���[���������Ă����Z��ʂɁu�N���W�b�g���p�s�v���\������Ȃ�)
	Lagtim( OPETCBNO, 26, 3*50 );									// ���د��\���p��ϰ(3s)START
// MH810100(E) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4607 ���Z���Ɍ��σ��[�_�̃G���[���������Ă����Z��ʂɁu�N���W�b�g���p�s�v���\������Ȃ�)

	if( f_NTNET_RCV_MC10_EXEC ){									// mc10()�X�V���K�v�Ȑݒ肪�ʐM�ōX�V���ꂽ
		mc10();
		f_NTNET_RCV_MC10_EXEC = 0;
	}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	ryo_buf.lag_tim_over = 0;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) 2020/06/17 #4497�y�A���]���w�E�����zAi-�X�����Ɏ������芄������̉��u���Z���s���ƁA�s���ȁi�O�񐸎Z���ɓK�������j�������ēK�����Đ��Z���Ă��܂�
	FLAPDT.flp_data[0].lag_to_in.BIT.LAGIN = 0x00;
// MH810100(E) 2020/06/17 #4497�y�A���]���w�E�����zAi-�X�����Ɏ������芄������̉��u���Z���s���ƁA�s���ȁi�O�񐸎Z���ɓK�������j�������ēK�����Đ��Z���Ă��܂�
// MH810100(S) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
	lcdbm_Flag_QRuse_ICCuse = 0;	// QR�t���O�N���A
// MH810100(E) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
	lcdbm_Flag_RemotePay_ICCuse = 0;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j

// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
//	/*** ���o�Ɏ��������� ***/
//	if( set_tim( OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){				// ���o�Ɏ���NG?(Y)
//		ope_anm(AVM_STOP);											// ������~
//		BUZPIPI();
//		return( 10 );												// ���Z���~(���Z�s��)�Ƃ���B�ҋ@�֖߂�
//	}
//	ryo_buf.credit.pay_ryo = 0;
//
//	ryo_cal( 0, OPECTL.Pr_LokNo );									// �����v�Z

// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
// // MH810105(S) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
// 	// ���Z�J�n�����ێ�
// 	SetPayStartTime( &CLK_REC );
// // MH810105(E) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

// GG124100(S) R.Endo 2022/08/03 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
		ret = 0;

		switch ( OPECTL.CalStartTrigger ) {
		case CAL_TRIGGER_LCD_IN_TIME:		// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
			// ���Z�J�n�����ێ�
			SetPayStartTimeInCarInfo();
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

			// ���ɏ��`�F�b�N(����)
			retOnline = cal_cloud_fee_check();

			// ���Ɏ����A�o�Ɏ���(���ݎ���)�A�Čv�Z�p���Ԏ������Z�b�g
			if ( !retOnline ) {
				if ( set_tim_only_out_card(1) ) {
					retOnline = 1;	// ���Z�s��
				}
			}

			// ���ʔ���
			switch ( retOnline ) {
			case 0:		// ��������
				// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)�ݒ�
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// 				in_time_set();
				in_time_set(1);	// ����
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�

				// ���͂��ꂽ���Ԉʒu�ԍ�
				key_num = (ushort)OPECTL.Op_LokNo;

				break;
			case 2:		// ���Z�ς�
				lcdbm_pay_rem_chg(2);	// ���Z�c���ω��ʒm(���Z�ς�)���M
				OPECTL.Ope_mod = 230;	// ���Z�ς݈ē�����
				return 0;	// �p��
// GG124100(S) R.Endo 2022/09/28 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
			case 3:		// ���Z�ς�(���~)
				lcdbm_pay_rem_chg(4);	// ���Z�c���ω��ʒm(���Z��(���~))���M
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.51(�ďC��))�iGM803000���p�j
				// ���Z���̃��[�����j�^���o�͂��Ă��Ȃ��̂ŁA�����ŎԔԂ��Z�b�g����
				// �Ԕԏ��iUTF-8�j�i�\���𔲂��ăR�s�[����j
				// ���^�x�ǖ��A���ޔԍ�
				memcpy(&carno[0], lcdbm_rsp_in_car_info_main.ShabanInfo.LandTransOfficeName, (size_t)(12+9));
				// �p�r����
				memcpy(&carno[(12+9)], lcdbm_rsp_in_car_info_main.ShabanInfo.UsageCharacter, (size_t)3);
				// ��A�ԍ�
				memcpy(&carno[(12+9+3)], lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber, (size_t)12);
				SetLaneFreeStr(carno, sizeof(carno));
				ope_MakeLaneLog(LM_PAY_STOP);
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.51(�ďC��))�iGM803000���p�j
				return 10;	// ���Z���~(�ҋ@�֖߂�)
// GG124100(E) R.Endo 2022/09/28 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
			case 1:		// ���Z�s��
			default:	// ���̑�
				lcdbm_pay_rem_chg(3);	// ���Z�c���ω��ʒm(���Z�s��)���M
				return 10;	// ���Z���~(�ҋ@�֖߂�)
			}

			break;
		case CAL_TRIGGER_REMOTECALC_FEE:	// ���u���Z(���z�w��)
// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
			// ���Z�J�n�����ێ�
			SetPayStartTime(&CLK_REC);
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
			// ���Ɏ����A�o�Ɏ���(���ݎ���)�A�Čv�Z�p���Ԏ������Z�b�g
			set_tim_Lost(OPECTL.CalStartTrigger, OPECTL.Pr_LokNo, GetPayStartTime(), 0);	// �G���[�͂Ȃ�

			// ���u���Z����o�^
			wopelg(OPLOG_PARKI_ENKAKU, 0, 0);

			// �����ݒ�
			cal_cloud_fee_set_remote_fee();

			// �����ݒ�
			if ( g_PipCtrl.stRemoteFee.Discount ) {	// �������z����
				cal_cloud_discount_set_remote_fee();
			}

			// ���͂��ꂽ���Ԉʒu�ԍ�
			key_num = (ushort)OPECTL.Op_LokNo;

			// ���u���Z���z�w�艞��
			NTNET_Snd_Data16_09(PIP_RES_RESULT_OK);

			break;
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
		case CAL_TRIGGER_REMOTECALC_TIME:	// ���u���Z(���Ɏ����w��)
// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
			// ���Z�J�n�����ێ�
			SetPayStartTimeInCarInfo();
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
			// ���Ɏ����A�o�Ɏ���(���ݎ���)�A�Čv�Z�p���Ԏ������Z�b�g
			if ( set_tim_only_out(OPECTL.Pr_LokNo, GetPayStartTime(), 1) == 1 ) {
				// ������~
				ope_anm(AVM_STOP);

				// �u�U�[
				BUZPIPI();

				// ���u���Z���Ɏ����w��w�艞��
				NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_OTHER);

				// ���Z�c���ω��ʒm(���Z�s��)���M
				lcdbm_pay_rem_chg(3);

				return 10;	// ���Z���~(�ҋ@�֖߂�)
			}

			// ���u���Z����o�^
			wopelg(OPLOG_PARKI_ENKAKU, 0, 0);

			// �����ݒ�
			cal_cloud_fee_set_remote_time();

			// �����ݒ�
			if ( g_PipCtrl.stRemoteTime.Discount ) {	// �������z����
				cal_cloud_discount_set_remote_time();
			}

			// ���͂��ꂽ���Ԉʒu�ԍ�
			key_num = (ushort)OPECTL.Op_LokNo;

			// ���u���Z���Ɏ����w��w�艞��
			NTNET_Snd_Data16_08(PIP_RES_RESULT_OK);

			break;
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
		default:
			break;
		}
	} else {					// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/08/03 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

	// ���u���Z(���z�w��)
	if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE ){
// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
		// ���Z�J�n�����ێ�
		SetPayStartTime(&CLK_REC);
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

		// ���o�Ɏ����𗝃`�F�b�N
		if( set_tim_Lost(OPECTL.CalStartTrigger, OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){		// ���o�Ɏ���NG?(Y)
			ope_anm( AVM_STOP );			// ������~
			BUZPIPI();
			NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_CHANGE_NEXT );
			return( 10 );					// ���Z���~(���Z�s��)�Ƃ���B�ҋ@�֖߂�
		}
		wopelg(OPLOG_PARKI_ENKAKU, 0, 0);	// ����o�^
		ryo_buf.credit.pay_ryo = 0;
		ryo_cal( 7, OPECTL.Pr_LokNo );		// �����v�Z
		// ������񂠂�H
		if( g_PipCtrl.stRemoteFee.Discount ){
			// ����>�c�z
			if( g_PipCtrl.stRemoteFee.Discount > ryo_buf.zankin ){
				g_PipCtrl.stRemoteFee.Discount = ryo_buf.zankin;
			}
			vl_now = V_DIS_FEE;
			ryo_cal( 3, OPECTL.Pr_LokNo );		// �T�[�r�X���Ƃ��Čv�Z
			ret = in_mony(OPE_REQ_REMOTE_CALC_FEE, 0);
		}
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//		OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z
		// �o�ɐ��Z�H
		if( !g_PipCtrl.stRemoteFee.Type ){
			OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z
		}
		// ������Z�H
		else{
			OpeNtnetAddedInfo.PayMethod = 13;	// ������Z
		}
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		OpeNtnetAddedInfo.PayMode = 4;
// �s��C��(S) K.Onodera 2016/10/12 #1518 �U�֐��Z���ƒʏ퐸�Z���ŗ����A�i�E���X�̕������قȂ�
		key_num = OPECTL.Op_LokNo;
// �s��C��(E) K.Onodera 2016/10/12 #1518 �U�֐��Z���ƒʏ퐸�Z���ŗ����A�i�E���X�̕������قȂ�
// �d�l�ύX(S) K.Onodera 2016/11/07 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//		g_PipCtrl.stRemoteFee.RyoSyu = ryo_buf.syubet + 1;
// �d�l�ύX(E) K.Onodera 2016/11/07 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		NTNET_Snd_Data16_09( PIP_RES_RESULT_OK );
	}
	// ���u���Z(���Ɏ����w��)
	else if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_TIME ){
// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
		// ���Z�J�n�����ێ�
		SetPayStartTime(&CLK_REC);
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

		// ���o�Ɏ����𗝃`�F�b�N
		if( set_tim_only_out(OPECTL.Pr_LokNo, &CLK_REC, 1) == 1 ){
			ope_anm( AVM_STOP );			// ������~
			BUZPIPI();
			NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_CHANGE_NEXT );
			return( 10 );					// ���Z���~(���Z�s��)�Ƃ���B�ҋ@�֖߂�
		}
		wopelg(OPLOG_PARKI_ENKAKU, 0, 0);	// ����o�^
		ryo_buf.credit.pay_ryo = 0;
		ryo_cal( 0, OPECTL.Pr_LokNo );		// �����v�Z
		if( g_PipCtrl.stRemoteTime.Discount ){
			if( g_PipCtrl.stRemoteTime.Discount > ryo_buf.zankin ){
				g_PipCtrl.stRemoteTime.Discount = ryo_buf.zankin;
			}
			vl_now = V_DIS_TIM;
			ryo_cal( 3, OPECTL.Pr_LokNo );		// �T�[�r�X���Ƃ��Čv�Z
			ret = in_mony(OPE_REQ_REMOTE_CALC_TIME, 0);
		}
		OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z
		OpeNtnetAddedInfo.PayMode = 4;
// �s��C��(S) K.Onodera 2016/10/12 #1518 �U�֐��Z���ƒʏ퐸�Z���ŗ����A�i�E���X�̕������قȂ�
		key_num = OPECTL.Op_LokNo;
// �s��C��(E) K.Onodera 2016/10/12 #1518 �U�֐��Z���ƒʏ퐸�Z���ŗ����A�i�E���X�̕������قȂ�
		// �ԐM�p���Z�b�g
		g_PipCtrl.stRemoteTime.RyoSyu = ryo_buf.syubet + 1;
		g_PipCtrl.stRemoteTime.Price = ryo_buf.tyu_ryo;
		NTNET_Snd_Data16_08( PIP_RES_RESULT_OK );
	}

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_for_LCD_IN_CAR_INFO_�����v�Z
	// ====================== //
	// LCD������ɏ���M
	// ====================== //
	else if( OPECTL.CalStartTrigger == CAL_TRIGGER_LCD_IN_TIME ){
// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
		// ���Z�J�n�����ێ�
		SetPayStartTimeInCarInfo();
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

		// ���Z�����敪�`�F�b�N
		FLAPDT.flp_data[0].lag_to_in.BIT.LAGIN = 0x00;

		if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1) ||		// 1=���Z
			(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2) ){		// 2=�Đ��Z
			// ���Z������������H
			if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth > 0) &&
				(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay > 0) ){

// MH810100(S) 2020/06/16 #4231 �y�A���]���w�E�����z���O�^�C���I�[�o��ɏo�ɂ��Ă��邪�A�����Z�o�ɂ̍Đ��Z�������ɂȂ��Ă��܂�(No.02-0027)
//				// ���O�^�C�����H�i���Z���� + ׸���ю��� >= ���ݎ����j
//// MH810100(S) K.Onodera 2020/03/05 #3962/#3912 �������o�Ɍ�A�Ė������o�ɂ��悤�Ƃ����Ƃ��ɗL���ɂȂ�
////				if( Is_in_lagtim() ){
//				// ���O�^�C�����H or ���������Z�H
//				if( Is_in_lagtim() || lcdbm_rsp_in_car_info_main.shubetsu == 1 ){
//// MH810100(E) K.Onodera 2020/03/05 #3962/#3912 �������o�Ɍ�A�Ė������o�ɂ��悤�Ƃ����Ƃ��ɗL���ɂȂ�
//// MH810100(S) K.Onodera 2020/2/18 #3869 ���O�^�C�����̍Đ��Z���{���A���Z�ς݈ē��\�����ꂸ�ɏ�����ʂɑJ�ڂ��Ă��܂�
//					lcdbm_pay_rem_chg(2);							// �c���ω��ʒm�i���Z�ς݁j���M
//// MH810100(S) K.Onodera  2020/03/05 �Ԕԃ`�P�b�g���X(���Z������ʕ\�����Ԃ��ݒ�l�ɏ]��Ȃ��s��C��)
////					lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 2 );	// ���Z�����ʒm(���Z�ς݈ē�)
//// MH810100(E) K.Onodera  2020/03/05 �Ԕԃ`�P�b�g���X(���Z������ʕ\�����Ԃ��ݒ�l�ɏ]��Ȃ��s��C��)
//// MH810100(E) K.Onodera 2020/2/18 #3869 ���O�^�C�����̍Đ��Z���{���A���Z�ς݈ē��\�����ꂸ�ɏ�����ʂɑJ�ڂ��Ă��܂�
//					// op_mod230(���Z�ς݈ē�)
//					OPECTL.Ope_mod = 230;
//					return(0);
//				}
//				// ���O�^�C���I�[�o�[�H�i���Z���� + ׸���ю��� < ���ݎ����j
//				else {
//					// ryo_cal()����
//					//		if( FLAPDT.flp_data[num-1].lag_to_in.BIT.LAGIN == ON ){ ryo_buf.lag_tim_over = 1; }���Ă���̂�
//					//		�ȉ���bit��set����(num == OPECTL.Pr_LokNo�̒l)
//					FLAPDT.flp_data[0].lag_to_in.BIT.LAGIN = 0x01;
//				}

				// ���O�^�C�����H�i���Z����(�o�Ɏ���������ꍇ�͏o�Ɏ����Ɣ�r����) + ׸���ю��� >= ���ݎ����j
				if( Is_in_lagtim() ){
					lcdbm_pay_rem_chg(2);							// �c���ω��ʒm�i���Z�ς݁j���M
					// op_mod230(���Z�ς݈ē�)
					OPECTL.Ope_mod = 230;
					return(0);
				}
				// ���O�^�C���I�[�o�[�H�i���Z����(�o�Ɏ���������ꍇ�͏o�Ɏ����Ɣ�r����)+ ׸���ю��� < ���ݎ����j
				else {
					// ryo_cal()����
					//		if( FLAPDT.flp_data[num-1].lag_to_in.BIT.LAGIN == ON ){ ryo_buf.lag_tim_over = 1; }���Ă���̂�
					//		�ȉ���bit��set����(num == OPECTL.Pr_LokNo�̒l)
					FLAPDT.flp_data[0].lag_to_in.BIT.LAGIN = 0x01;
				}
// MH810100(E) 2020/06/16 #4231 �y�A���]���w�E�����z���O�^�C���I�[�o��ɏo�ɂ��Ă��邪�A�����Z�o�ɂ̍Đ��Z�������ɂȂ��Ă��܂�(No.02-0027)
			}
		}

// MH810105(S) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
// 		// ���Z�J�n�����ێ�
// 		SetPayStartTime( &CLK_REC );
// MH810105(E) R.Endo 2021/11/04 �Ԕԃ`�P�b�g���X3.0 #6147 �o�ɍςݐ��Z��ɉ��u���Z�����ꍇ�ANT-NET���Z�f�[�^�̏��������Ɠ��ɓ������s���ɂȂ�
// MH810100(S) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
// MH810100(S) 2020/08/19 �Ԕԃ`�P�b�g���X(#4744 �y���؉ێw�E�����zQR������(1h)������������̎�ʐؑւŖ����ɂȂ�(No56))
		// ����ryo_cal�ŃN���A����邪�����ł����Ă����BIsDupSyubetuwari�ŎQ�Ƃ��邽��
		ryo_buf.nyukin = 0;
// MH810100(E) 2020/08/19 �Ԕԃ`�P�b�g���X(#4744 �y���؉ێw�E�����zQR������(1h)������������̎�ʐؑւŖ����ɂȂ�(No56))
// MH810100(S) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
		g_checkKirikae = 0;								// ��ʐؑփ`�F�b�N�ς݃t���O
// MH810100(E) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
		// �����̎�ʊ��������邩�ǂ����`�F�b�N
		if( IsDupSyubetuwari() == TRUE){
			// error��
			lcdbm_pay_rem_chg(3);		// �c���ω��ʒm�i���Z�s�j���M
			lcdbm_notice_dsp( POP_UNABLE_PAY, 0 );			// 8:���Z�s��
			return (10);
		}
// MH810100(E) 2020/08/03 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�

		// ���Ɏ���, �o�Ɏ���(���ݎ���), �Čv�Z�p���Ԏ������Z�b�g
		if( set_tim_only_out_card( 1 ) == 1 ){
			// error��
			lcdbm_pay_rem_chg(3);		// �c���ω��ʒm�i���Z�s�j���M
			lcdbm_notice_dsp( POP_UNABLE_PAY, 0 );			// 8:���Z�s��
			return (10);
		}

		// ryo_cal()���ŎԔԐ��Z�̎��ɎQ�Ƃ���
		ryo_buf.credit.pay_ryo = 0;

		// ----------------------------------------------- //
		// �������v�Z
		//   ryo_cal()�ł͎ԔԐ��Z��
		//				ec09()	req_rkn.param = RY_TSA
		//				et02() -> et40()		�̏��Ԃ�call����
		// ----------------------------------------------- //
		// ryo_cal()���ňȉ���׸���ѵ��ނ��������s��
		//		if( FLAPDT.flp_data[num-1].lag_to_in.BIT.LAGIN == ON ){ ryo_buf.lag_tim_over = 1; }

		// �ԔԐ��Z�Ƃ��Čv�Z
		ryo_cal( 8, OPECTL.Pr_LokNo );		// Pr_LokNo : ���������p���Ԉʒu�ԍ�(1�`324))
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:�Ԏ�؊���̐��Z���~���A���ԗ���/������ʂ�؊��O�ɖ߂�)
		RTPay_Data.crd_info.dtZaishaInfo.ParkingFee = ryo_buf.tyu_ryo;	// ���񗿋��v�Z���̒��ԗ������i�[
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:�Ԏ�؊���̐��Z���~���A���ԗ���/������ʂ�؊��O�ɖ߂�)
		ret = in_mony( OPE_REQ_LCD_CALC_IN_TIME, 0 );

		// ryo_cal()���Ń��O�^�C���I�[�o�[���o�H
		if( ryo_buf.lag_tim_over == 1 ){
			// ���O�^�C���I�[�o�[�㒓�ԗ��� > ��������z
			if( ryo_buf.tyu_ryo > lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.GenkinFee ){
				// ���Ɏ�������Čv�Z�������ԗ�������A��������z������
				ryo_buf.zankin = ryo_buf.tyu_ryo - lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.GenkinFee;
				// ����ɁA���ϋ��z������
				for( cnt=0; cnt<ONL_MAX_SETTLEMENT; cnt++ ){
					// ���ϋ敪����H
					if( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeType ){
						if( ryo_buf.zankin > lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeFee ){
							ryo_buf.zankin -= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeFee;
						}else{
							ryo_buf.zankin = 0;
							break;
						}
					}
				}
			}else{
				ryo_buf.zankin = 0;
			}
			// �\�������ƉېőΏۊz�ɂ����l�̋��z���Z�b�g
			ryo_buf.dsp_ryo = ryo_buf.kazei = ryo_buf.zankin;
// MH810100(S) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
//			// �O�񐸎Z�Ŏx����������/���ϊz���Z�b�g
//			ryo_buf.zenkai = (ryo_buf.tyu_ryo - ryo_buf.dsp_ryo);
// MH810100(E) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
		}

		// �����Z�o�ɐ��Z�H
		if( lcdbm_rsp_in_car_info_main.shubetsu == 1 ){
			OpeNtnetAddedInfo.PayMethod = 13;	// ������Z
		}
		// ���O���Z�H
		else{
			OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z
		}

		OpeNtnetAddedInfo.PayMode = 0;		// ���Z���[�h = �������Z
		if(( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1 ) ||	// ���Z
		   ( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2 ) ||	// �Đ��Z
		   ( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 4 )){	// �Đ��Z���~
			OpeNtnetAddedInfo.PayClass = 1;	// �����敪 = �Đ��Z
		}

		// �\����������H
// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//		if( ryo_buf.dsp_ryo ){
		if( 1 ){
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�

			// ----------------------------------------------- //
			// ���
			// ----------------------------------------------- //
			// ����ԁH
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)
//			if( isSeasonCardType( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo ) ){
			if( isSeasonCardType( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo ) ){
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)
				// ��M����f�[�^�`�F�b�N�iCRD_DAT�ւ̒ǉ����s���j
// MH810101(S) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
//				if( CheckSeasonCardData( &lcdbm_rsp_in_car_info_main ) == SEASON_CHK_OK ){
				season_chk_result = CheckSeasonCardData( &lcdbm_rsp_in_car_info_main );
				if( season_chk_result == SEASON_CHK_OK ){
// MH810101(E) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
					al_pasck_set();							// PayData�ɒ��������������
// MH810101(S) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
					if ( OPECTL.PassNearEnd == 1 ) {
						season_chk_result = 100;
					}
// MH810101(E) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�

					ryo_cal( 1, OPECTL.Pr_LokNo );			// �����v�Z
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:�Ԏ�؊���̐��Z���~���A���ԗ���/������ʂ�؊��O�ɖ߂�)
					RTPay_Data.crd_info.dtZaishaInfo.ParkingFee = ryo_buf.tei_ryo;	// �i�[�������񗿋��v�Z���̒��ԗ������������ŏ㏑��
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:�Ԏ�؊���̐��Z���~���A���ԗ���/������ʂ�؊��O�ɖ߂�)

					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;					// ������
					}

					ac_flg.cycl_fg = 10;					// ����

					ret = in_mony( OPE_REQ_LCD_CALC_IN_TIME, 0 );
				}
			}
		}
// MH810100(S) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
		Set_PayInfo_from_InCarInfo();	// ���ɏ��Ŏ�M�������Z�����Z�b�g
// MH810100(E) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)

		// �\����������H
// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//		if( ryo_buf.dsp_ryo ){
		// �T�[�r�X�^�C�����ł�����������Ă�����K��������
		if( 1 ){
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�

			// ----------------------------------------------- //
			// ����
			// ----------------------------------------------- //
			OnlineDiscount();

			key_num = OPECTL.Op_LokNo;			// �����͒l <- ���Ԉʒu�ԍ�
		}
// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
		// �T�[�r�X�^�C�����ł��悹��̂ł����̓R�����g�A�E�gOnlineDiscount��COPY�����
//		else{
//			// ������COPY���Ă����i����OnlineDiscount��COPY����Ȃ��Ƃ��Ɂj
//			CopyDiscountCalcInfoToZaisha();
//		}
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_for_LCD_IN_CAR_INFO_�����v�Z

// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	// �U�֐��Z
//	else if( PiP_GetFurikaeSts() ){
//		// ���o�Ɏ����𗝃`�F�b�N
//		if( set_tim( OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){
//			// ���o�Ɏ���NG?(Y)
//			ope_anm( AVM_STOP );			// ������~
//			BUZPIPI();
//			NTNET_Snd_Data16_04( PIP_RES_RESULT_OK_STOP );
//			return( 10 );					// ���Z���~(���Z�s��)�Ƃ���B�ҋ@�֖߂�
//		}
//		wopelg(OPLOG_PARKI_FURIKAE, 0, 0);	// ����o�^
//		ryo_buf.credit.pay_ryo = 0;
//		ryo_cal( 0, OPECTL.Pr_LokNo );		// �����v�Z
//		// �U�֊J�n����
//		FurikaeMotoSts = 1;					// �U�֌���ԃt���O 1:�Ԃ���
//		OpeNtnetAddedInfo.PayMethod = 12;	// ���Z���@=12:�U�֐��Z
//		// ����L/������
//		if( vl_frs.antipassoff_req ){
//			// ������p�L��Ȃ���������
//			ryo_buf.apass_off = 1;
//		}else{
//			ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
//			vl_now = V_FRK;						// �U�֐��Z
//			ryo_cal( 3, OPECTL.Pr_LokNo );		// �T�[�r�X���Ƃ��Čv�Z
//		}
//// �s��C��(S) K.Onodera 2016/10/12 #1518 �U�֐��Z���ƒʏ퐸�Z���ŗ����A�i�E���X�̕������قȂ�
//		key_num = OPECTL.Op_LokNo;
//// �s��C��(E) K.Onodera 2016/10/12 #1518 �U�֐��Z���ƒʏ퐸�Z���ŗ����A�i�E���X�̕������قȂ�
//		// �ԐM�p���Z�b�g
//		g_PipCtrl.stFurikaeInfo.DestFeeKind = ryo_buf.syubet + 1;
//		g_PipCtrl.stFurikaeInfo.DestFee = ryo_buf.tyu_ryo;
//		// ���퉞��
//		NTNET_Snd_Data16_04( PIP_RES_RESULT_OK );
//	}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	// ���̑�
	else{
// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
		// ���Z�J�n�����ێ�
		SetPayStartTime(&CLK_REC);
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

		// ���o�Ɏ�����`�F�b�N
		if( set_tim( OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){									// ���o�Ɏ���NG?(Y)
			ope_anm( AVM_STOP );			// ������~
			BUZPIPI();
			return( 10 );					// ���Z���~(���Z�s��)�Ƃ���B�ҋ@�֖߂�
		}
		ryo_buf.credit.pay_ryo = 0;
		ryo_cal( 0, OPECTL.Pr_LokNo );		// �����v�Z
	}
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
	if (OPECTL.Seisan_Chk_mod == ON) {			// ���Z����Ӱ��ON
		// ���Z�z�������I�ɕS�~�ɂ���
// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
//		ryo_buf.dsp_ryo = 100;
		autopay_coin_cnt = 0;					// 100�~���������N���A
		if( ryo_buf.svs_tim == 1 ){		// �T�[�r�X�^�C�����̏ꍇ
			ryo_buf.dsp_ryo = 0;
		}
		else{
			ryo_buf.dsp_ryo = AP_PARKING_FEE;
			Lagtim( OPETCBNO, 28, AP_CHARGE_INTERVAL_SEC * 50);			// 100�~�����^�C�}
		}
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z
	}
#endif

	ac_flg.cycl_fg = 1;												// 1:�����\����
	OPECTL.op_faz = 0;												// ���ڰ���̪���
	OPECTL.RECI_SW = 0;												// �̎������ݖ��g�p
	RECI_SW_Tim = 0;												// �̎��؎�t���Ըر
	OPECTL.CAN_SW = 0;												// �������
	OPECTL.PassNearEnd = 0;											// �����؂�ԋ��׸�
	OPECTL.LastUsePCardBal = 0L;									// �Ō�ɗ��p��������߲�޶��ނ̎c�z
	OPECTL.PriUsed = 0;
	ryodsp = 0;
	OPECTL.f_KanSyuu_Cmp = 0;										// �����W�v�����{
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	OPECTL.f_eReceiptReserve = 0;									// �d�q�̎��ؗ\��N���A
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	OPECTL.InLagTimeMode = 0;

// �d�l�ύX(S) K.Onodera 2016/11/04 �U�֐��Z���̗̎��؎������s�̐ݒ��݂���
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
////	if( OPECTL.Pay_mod != 2 ){
////		ope_imf_Start();
////	}
//	// ParkingWeb�o�R�̐U�֐��Z���H
//	if( PiP_GetFurikaeSts() ){
//		// �̎��� �������s����H
//		if( prm_get( COM_PRM, S_CEN, 33, 1, 1 ) == 1 ){
//			// �̎������ݖ��g�p�H
//			if( OPECTL.RECI_SW == 0 ){
//				OPECTL.RECI_SW = 1;						// �̎������ݎg�p
//			}
//		}
//	}
//	// ParkingWeb�o�R�̐U�֐��Z���łȂ��H
//	else{
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	if( OPECTL.Pay_mod != 2 ){
		ope_imf_Start();
	}
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// �d�l�ύX(E) K.Onodera 2016/11/04 �U�֐��Z���̗̎��؎������s�̐ݒ��݂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
	// ���u���Z�H
	if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE || 
		OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_TIME ){
		// �̎��� �������s����H
		if( prm_get( COM_PRM, S_CEN, 34, 1, 1 ) == 1 ){
			// �̎������ݖ��g�p�H
			if( OPECTL.RECI_SW == 0 ){
				OPECTL.RECI_SW = 1;						// �̎������ݎg�p
// MH810100(S) S.Fujii 2020/07/01 �Ԕԃ`�P�b�g���X(���u���Z���̗̎��؎������s)
				PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );	// ����ʒm���M
// MH810100(E) S.Fujii 2020/07/01 �Ԕԃ`�P�b�g���X(���u���Z���̗̎��؎������s)
			}
		}
	}
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[

	/* ���֐��Z�J�n�o�^ */
	IoLog_write(IOLOG_EVNT_AJAST_STA, (ushort)LockInfo[OPECTL.Pr_LokNo - 1].posi, 0, 0);

	Ope_Last_Use_Card = 0;											// �Ō�ɗ��p�����J�[�h������
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Card_Status_Prev = 0;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	dsp_change = 0;
	w_settlement = 0;


#if (4 == AUTO_PAYMENT_PROGRAM)						// �����p�ɗ�����ύX����
	if( CPrmSS[S_SYS][5] != 0 )
	ryo_buf.tyu_ryo = ryo_buf.dsp_ryo = ryo_buf.zankin = ryo_buf.kazei = CPrmSS[S_SYS][5];
#endif																	// ���~���� �T���E�|���� �����z get

	if( ryo_buf.pkiti != 0xffff ){
		syu_bk = (uchar)FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].bk_syu;
	}else{
		syu_bk = (uchar)LOCKMULTI.lock_mlt.bk_syu;
	}
// MH322914(S) K.Onodera 2016/09/13 AI-V�Ή� �U�֐��Z
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	// �V�X�e��ID16�F�U�֐��Z�H ���A�U�֊z�����ԗ����ȏ�H
//	if( PiP_GetFurikaeSts() && ryo_buf.dsp_ryo == 0 ){
//		ret = in_mony(OPE_REQ_FURIKAE_GO, 0);
//	}
//	else{
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH322914(E) K.Onodera 2016/09/13 AI-V�Ή� �U�֐��Z
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	// �Ԕԏ��iUTF-8�j�i�\���𔲂��ăR�s�[����j
	// ���^�x�ǖ��A���ޔԍ�
	memcpy(&carno[0], lcdbm_rsp_in_car_info_main.ShabanInfo.LandTransOfficeName, (size_t)(12+9));
	// �p�r����
	memcpy(&carno[(12+9)], lcdbm_rsp_in_car_info_main.ShabanInfo.UsageCharacter, (size_t)3);
	// ��A�ԍ�
	memcpy(&carno[(12+9+3)], lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber, (size_t)12);
	SetLaneFreeStr(carno, sizeof(carno));
	SetLaneFeeKind((ushort)(ryo_buf.syubet+1));
// GG129000(S) ���P�A��No.01 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
//	SetLaneFreeNum(ryo_buf.dsp_ryo);
	SetLaneFreeNum(lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.ParkingFee);
// GG129000(E) ���P�A��No.01 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
// GG129000(S) H.Fujinaga 2023/01/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
	if(lcdbm_rsp_in_car_info_main.shubetsu != 2){
// GG129000(E) H.Fujinaga 2023/01/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
	switch( OPECTL.f_searchtype ){
	case SEARCH_TYPE_NO:		// �ԔԌ���
	case SEARCH_TYPE_TIME:		// ��������
		// ���[�����j�^�f�[�^�o�^
		ope_MakeLaneLog(LM_PAY_NO_TIME);
		break;
	case SEARCH_TYPE_QR_TICKET:	// QR����
		// ���[�����j�^�f�[�^�o�^
		ope_MakeLaneLog(LM_PAY_QR_TICKET);
		break;
	default:				
		break;
	}
// GG129000(S) H.Fujinaga 2023/01/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
	}
	else {
		// ���Z���ύX�f�[�^�K�p��
		ope_MakeLaneLog(LM_PAY_CHANGE);
	}
// GG129000(E) H.Fujinaga 2023/01/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	if( ryo_buf.nyukin || ryo_buf.waribik || c_pay || syu_bk || e_pay){

		// ���Z��������invcrd()�Ɠ��l�̏����Ƃ���
		if( ryo_buf.nyukin < ryo_buf.dsp_ryo ){						// �����z < ���ԗ���?
			ryo_buf.zankin = ryo_buf.dsp_ryo - ryo_buf.nyukin;
		}
// MH321800(S) G.So IC�N���W�b�g�Ή�
		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// ���p�\�}�̻��د��\���p��ϰSTOP
		} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
			Lagcan( OPETCBNO, 10 );									// ���p�\�}�̻��د��\���p��ϰSTOP
// MH321800(S) G.So IC�N���W�b�g�Ή�
		}
// MH321800(E) G.So IC�N���W�b�g�Ή�
		ryodsp = 1;													// �����\��
		OpeLcd( 4 );												// �����\��
	}else{
		suica_fusiku_flg = 0;										// ���p�\�}�̻��د��\���L��
// MH322914 (s) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
		PrinterCheckForSuica();
// MH322914 (e) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
// MH810103 GG119202(S) �R�C���E�����̃G���[�`�F�b�N�ǉ�
		if (cn_errst[0] != 0) {
			coin_err_flg = 1;
		}
		if (cn_errst[1] != 0) {
			note_err_flg = 1;
		}
// MH810103 GG119202(E) �R�C���E�����̃G���[�`�F�b�N�ǉ�
		OpeLcd( 3 );												// �����\��(����)
		// �T�C�N���b�N�\���^�C�}�N��
		if (dspIsCyclicMsg()) {		// �T�C�N���b�N����/������
			DspChangeTime[0] = (ushort)(prm_get(COM_PRM, S_DSP, 30, 1, 1)); // ���p�\�}�̻��د��\���p��ϰ�l�擾
			if(DspChangeTime[0] <= 0) {
				DspChangeTime[0] = 2;
			}
// MH321800(S) G.So IC�N���W�b�g�Ή�
			if (isEC_USE() != 0) {
				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
			} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
				mode_Lagtim10 = 0;										// Lagtimer10�̎g�p��� 0:���p�\�}�̻��د��\��
				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
// MH321800(S) G.So IC�N���W�b�g�Ή�
			}
// MH321800(E) G.So IC�N���W�b�g�Ή�
		}
	}

// GG129000(S) ���P�A��No.01 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
		if( ryo_buf.nyukin || ryo_buf.waribik || c_pay || e_pay ){
			// ���ɏ��Ŏ��O���Z���ɂ�銄����O��̎��z������ꍇ�́u���Z���i���O���Z�F�����ς݁i�����܂ށj�j�v�𑗐M����
			SetLaneFeeKind(ryo_buf.syubet+1);
			SetLaneFreeNum(ryo_buf.zankin);
			ope_MakeLaneLog(LM_PAY_MNY_IN);
		}
// GG129000(E) ���P�A��No.01 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j

	// 0�~���Z
	if( ryo_buf.dsp_ryo == 0 ){
		r_zero = 0;													// 0�~���Z
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102���p�j�iGM803003���p�j
		r_zero_call = 1;											// 0�~���Z�ɂ��\���Z�k�t���OON
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102���p�j�iGM803003���p�j
		ope_anm(AVM_STOP);											// ������~
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i��ʕ\�����ԒZ�k�d�l�̋����j�iGM804102���p�j�iGM803003���p�j
//		Lagtim( OPETCBNO, 1, 3*50 );								// ��ϰ1(3s)�N��(���ڰ��ݐ���p)
		Lagtim( OPETCBNO, 1, 1*50 );								// ��ϰ1(1s)�N��(���ڰ��ݐ���p)
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i��ʕ\�����ԒZ�k�d�l�̋����j�iGM804102���p�j�iGM803003���p�j
		OPE_red = 4;												// ���Z����
// MH810100(S) S.Takahashi 2020/03/02 �����z���x��ĕ\�������
		lcdbm_pay_rem_chg(1);							// �c���ω��ʒm(���Z����)
// MH810100(E) S.Takahashi 2020/03/02 �����z���x��ĕ\�������
// GG129000(S) M.Fujikawa 2023/09/11 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�����̎�0�~���͗̎��؂𔭍s���Ȃ��j
		QRIssueChk(0);
// GG129000(E) M.Fujikawa 2023/09/11 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�����̎�0�~���͗̎��؂𔭍s���Ȃ��j
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 0 );				// ���Z�����ʒm(0�~�\��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	}
	// 0�~�ȊO���Z
	else{
		r_zero = -1;
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102���p�j�iGM803003���p�j
		r_zero_call = 0;											// 0�~���Z�ɂ��\���Z�k�t���OOFF
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102���p�j�iGM803003���p�j

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		// ���Z�c���ω��ʒm�̑��M����
		lcdbm_pay_rem_chg(0);		// ���Z�ð��(0:���Z��,1:���Z����,2:���Z��,3:���Z�s��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// GG129000(S) ���P�A��No.01 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
//// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.53)�iGM803000���p�j
//		if( ryo_buf.nyukin || ryo_buf.waribik || c_pay || e_pay ){
//			// ���ɏ��Ŏ��O���Z���ɂ�銄����O��̎��z������ꍇ�́u���Z���i���O���Z�F�����ς݁i�����܂ށj�j�v�𑗐M����
//			SetLaneFeeKind(ryo_buf.syubet+1);
//			SetLaneFreeNum(ryo_buf.zankin);
//			ope_MakeLaneLog(LM_PAY_MNY_IN);
//		}
//// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.53)�iGM803000���p�j
// GG129000(E) ���P�A��No.01 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j

		if( !SyuseiEndKeyChk() ){
			cn_stat( 1, 2 );									// ���� & COIN������
		}

		nmicler( &SFV_DAT.refval, 8 );								// �s���z�ر

		OPE_red = 3;												// ���Z��
		if(( RD_mod != 10 )&&( RD_mod != 11 )){
			opr_snd( 3 );											// ذ�޺����(���Ԍ��ۗ���)
		}

		if( SyuseiEndKeyChk() != 2 ){
			if( MIFARE_CARD_DoesUse ){									// Mifare���L���ȏꍇ
				op_MifareStart();										// Mifare��L����
			}
		}


		if( SyuseiEndKeyChk() != 2 ){
			read_sht_opn();											// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
		}


		if( !SyuseiEndKeyChk() ){

// MH322914 (s) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
//				if( Ope_isJPrinterReady() != 1 || IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)){		// �W���[�i���g�p�s��
//						Suica_Rec.Data.BIT.PRI_NG = 1;
//						Edy_Rec.edy_status.BIT.PRI_NG = 1;
//				}
			PrinterCheckForSuica();
// MH322914 (e) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
			Suica_Rec.Data.BIT.ADJUSTOR_START = 1;						// ���Z�J�n�t���O�Z�b�g
			if( !Suica_Rec.Data.BIT.CTRL ){								// ��t�s��ԁH
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
				if (isSX10_USE()) {
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
				Suica_Ctrl( S_CNTL_DATA, 0x01 );										// Suica���p���ɂ���
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
				}
				else {
					Ope_EcPayStart();									// ���σ��[�_���p���ɂ���
				}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
			}else{
// MH321800(S) G.So IC�N���W�b�g�Ή�
				if( isSX10_USE() ) {									// SX-10�ڑ�����
// MH321800(E) G.So IC�N���W�b�g�Ή�
				Suica_Ctrl( S_SELECT_DATA, 0 );							// ���ԗ��������i�I���ް��Ƃ��đ��M����
// MH321800(S) G.So IC�N���W�b�g�Ή�
				}else{													// EC�L�� and �������ϗL��
					Ope_EcPayStart();									// ���σ��[�_���p���ɂ���
				}
// MH321800(E) G.So IC�N���W�b�g�Ή�
			}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( Edy_Rec.edy_status.BIT.INITIALIZE ){				// EdyӼޭ�ق̏��������������Ă���H
//					Edy_SndData01();									// ���ތ��m�J�n
//				}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		}

		w_settlement = 0;											// Suica�c���s�����̐��Z�z�i�O���͎c���s���łȂ����Ӗ�����j
		ope_anm(AVM_SHASHITU);										// �Ԏ��ǂݏグ
		// �����ǂݏグ����
		if(ryo_announce_flg == 0){
			announceFee = ryo_buf.dsp_ryo;
			ope_anm(AVM_RYOUKIN);									// �u�����́������������~�ł��v
			ryo_announce_flg = 1;									// �����Ǐグ
		}
		if( OPECTL.Pay_mod != 2 ){									// �C�����Z�łȂ�
			if( Ex_portFlag[EXPORT_CHGNEND] == 0 ){						// 0:�ޑK�؂�\������/1:�ޑK�؂�\���L��
				ope_anm( AVM_TURIARI );									// �����\����(�ޑK������)�ųݽ
			}else{
				ope_anm( AVM_TURINASI );								// �����\����(�ޑK�Ȃ����)�ųݽ
			}
		}
// MH810103 GG119202(S) �d�q�}�l�[�V���O���ݒ�ňē��������s��
		op_EcEmoney_Anm();
// MH810103 GG119202(E) �d�q�}�l�[�V���O���ݒ�ňē��������s��
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�폜
//			Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][65]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�폜
	}
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH322914(S) K.Onodera 2016/09/13 AI-V�Ή� �U�֐��Z
//	}
//// MH322914(E) K.Onodera 2016/09/13 AI-V�Ή� �U�֐��Z
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	OPECTL.coin_syukei = 0;											// ��݋��ɏW�v���o�͂Ƃ���
	OPECTL.note_syukei = 0;											// �������ɏW�v���o�͂Ƃ���

	dsp_fusoku = 0;													// �װ�ү���ޕ\���׸�ؾ��
	suica_fusiku_flg = 0;											//
// MH810100(S) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
	lcdbm_Flag_ResetRequest = 0;
// MH810100(E) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
// MH810103(s) �d�q�}�l�[�Ή� #5424 ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���
	// ���Z��Ԓʒm(Ope_mod�ɂ��؂�ւ��̂ŁAmod02���_�ő��M���Ă���)Ope_EcPayStart�̌�łȂ��ƁA���p�s�ƂȂ��Ă��܂�
	dspCyclicErrMsgRewrite();
// MH810103(e) �d�q�}�l�[�Ή� #5424���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���


	for( ret = 0; ; ){
		msg = GetMessage();
		if( Ope_MiryoEventCheck( msg ) == 1 )
			continue;

		if( Ope_ArmClearCheck( msg ) == 1 )
			continue;
_MSG_RETRY:

		switch( msg ){
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			case KEY_TEN_F3:											// [�������Z����]�����
//				if(	OPECTL.Pay_mod == 2 ){								// �C�����Z�̎�
//					if( prm_get( COM_PRM,S_TYP,98,1,3 ) ){				// �������Z�������L��
//						BUZPI();
//						cn_stat( 2, 2 );								// �����s��
//						PayData.MMTwari = ryo_buf.dsp_ryo;
//						PayData.Zengakufg |= 0x02;						// �x�����|������
//						ret = 1;
//					}
//				}
//				break;
//			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
//// MH810105(S) MH364301 �C���{�C�X�Ή�
////				if(Ope_isPrinterReady() == 0){
//				if (Ope_isPrinterReady() == 0 ||
//					(IS_INVOICE && Ope_isJPrinterReady() == 0)) {		// �C���{�C�X�ݒ莞�̓��V�[�g�ƃW���[�i�����Е��ł������Ȃ������ꍇ
//// MH810105(E) MH364301 �C���{�C�X�Ή�
//					if( pri_ann_flg == 0 && OPECTL.InquiryFlg == 0){		// �₢���킹���̓A�i�E���X���Ȃ�
//						pri_ann_flg = 1;
//						ope_anm( AVM_RYOUSYUU_NG );				// �����A�̎��؂̔��s�͏o���܂���
//					}
//					break;
//				}
//// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//				if( isEC_USE() && ECCTL.Ec_FailureContact_Sts != 0 ){	// 1:��Q�A���[���s�������^2:��Q�A���[���s��
//					if (ECCTL.Ec_FailureContact_Sts == 2) {
//						// ��Q�A���[���s�ς݂̏ꍇ�A�̎��؃{�^�����󂯕t���Ȃ�
//						break;
//					}
//					EcFailureContactPri();								// ��Q�A���[���s �����V�[�g�̂�
//					op_SuicaFusokuOff();
//					// 6�b�ԕ\������
//					lcd_wmsg_dsp_elec(1, ERR_CHR[78], ERR_CHR[79], 0, 0, COLOR_RED, LCD_BLINK_OFF);
//					LagTim500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER, 13, lcd_Recv_FailureContactData_edit);
//					ECCTL.Ec_FailureContact_Sts = 2;					// 2:��Q�A���[���s��
//					if( OPECTL.RECI_SW == 0 ){							// �̎������ݖ��g�p?
//						OPECTL.RECI_SW = 1;								// �̎������ݎg�p
//					}
//					break;
//				}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
//				if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
//				// �����m���Ȃ�󂯕t���Ȃ�
//					break;
//				}
//				if( announce_flag ){
//				// ���ޔ������ųݽ���Ȃ�󂯕t���Ȃ�
//					break;
//				}
//// MH321800(E) hosoda IC�N���W�b�g�Ή�
//				if( ReceiptChk() == 1 ){							// ڼ�Ĕ��s�s�v�ݒ莞
//					break;
//				}
//				if(	(OPECTL.Pay_mod == 2)&&							// �C�����Z
//					(!prm_get(COM_PRM, S_PRN, 32, 1, 6))){			// �̎��؂̔��s�Ȃ�
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				if( OPECTL.RECI_SW == 0 ){							// �̎������ݖ��g�p?
//					if((( OPECTL.ChkPassSyu == 0 )&&				// �⍇�����ł͂Ȃ�?
//					   ( OPECTL.op_faz != 0 || ryodsp ))			// �ҋ@��ԈȊO?
//					   &&
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
////					   (ryo_buf.credit.pay_ryo == 0))				// �N���W�b�g�₢���킹��?
//							ec_MessagePtnNum == 0 ) 					// ���σ��[�_�֘A�����\�����ł͂Ȃ��H
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//					{
//						wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
//						grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      �̎��؂𔭍s���܂�      "
//// MH810105(S) MH364301 �C���{�C�X�Ή�
//						OPECTL.f_CrErrDisp = 0;						// �G���[��\��
//// MH810105(E) MH364301 �C���{�C�X�Ή�
//					}
//					OPECTL.RECI_SW = 1;								// �̎������ݎg�p
//				}
//				break;
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			case COIN_EVT:											// Coin Mech event
				if( OPECTL.op_faz == 4 ){							// ��ѱ��
					if(( OPECTL.CN_QSIG == 7 )&&( ryo_buf.nyukin == 0 )){	// ���o�������z���Ȃ�������
						if( !Op_Cansel_Wait_sub( 0xff ) )				// �S�f�o�C�X�̃L�����Z���ς݁H
							Op_Cansel_Wait_sub( 2 );					// ���b�N�̒�~
						break;
					}else{
						OPECTL.op_faz = 0;
						cn_stat( 3, 2 );							// ������
						Lagcan( OPETCBNO, 2 );						// ү��Ď���ϰ��ݾ�
					}
				}
				if (( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 ) ){
					nyukin_flag = 1;
				}
			case NOTE_EVT:											// Note Reader event
				if(( r_zero == 0 )&&( OPECTL.cnsend_flg == 1 )){	// 0�~���Z�������\���
					r_zero = -1;
					OPECTL.op_faz = 1;								// ������
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
				}
				if ( (msg == NOTE_EVT )&&( OPECTL.NT_QSIG == 1 ) ){
					nyukin_flag = 1;
				}
			case COIN_RJ_EVT:										// ���Z���~
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_ؼު����ް_COIN_RJ_EVT
// MH810100(S) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
//				// ؼު����ް_COIN_RJ_EVT
//				if( msg == COIN_RJ_EVT ){
			case TIMEOUT8:
				// ���W�F�N�g���o�[ or ������߂蔻��^�C���A�E�g 
				if( msg == COIN_RJ_EVT || msg == TIMEOUT8 ){
// MH810100(E) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
// MH810100(S) K.Onodera  2020/02/28 #3974 ���W�F�N�g���o�[�Œ��~��A���Z�J�n�ł��Ȃ�
//					// ���σ��[�_�̃J�[�h�������H
//					if( isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ){
//						break;
//					}
//// MH810100(S) K.Onodera  2020/02/27 #3945 ���z���̃��W�F�N�g���o�[����ŏ�����ʂ֑J�ڂ��Ă��܂�
//					if( OPECTL.op_faz < 2 ){
//// MH810100(E) K.Onodera  2020/02/27 #3945 ���z���̃��W�F�N�g���o�[����ŏ�����ʂ֑J�ڂ��Ă��܂�
//						// ���~�����҂�����Ȃ��H
//						if( stp_wait_flg == 0 ){
//							// ���Z���~����OK�҂���ײ����
//							lcdbm_Counter_PayStopRetry = 0;
//							// ���Z���~�v�����M
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP, 0 );
//							// ���Z���~�����҂��^�C�}
//							Lagtim( OPETCBNO, TIMERNO_PAY_STOP_RESP, (res_wait_time * 50) );
//							stp_wait_flg = 1;
//						}
//// MH810100(S) K.Onodera  2020/02/27 #3945 ���z���̃��W�F�N�g���o�[����ŏ�����ʂ֑J�ڂ��Ă��܂�
//					}
//// MH810100(E) K.Onodera  2020/02/27 #3945 ���z���̃��W�F�N�g���o�[����ŏ�����ʂ֑J�ڂ��Ă��܂�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6398 �d�q�}�l�[�AQR�R�[�h�ǎ掞�̏������|�b�v�A�b�v�����Ԍo�߂łƂ��Ă��܂�
//					// ���W�F�N�g���o�[�ʒm
//					PKTcmd_notice_ope( LCDBM_OPCD_REJECT_LEVER, (ushort)0 );
//// MH810100(E) K.Onodera  2020/02/28 #3974 ���W�F�N�g���o�[�Œ��~��A���Z�J�n�ł��Ȃ�
//					break;
					if( msg == TIMEOUT8 ){								// ������߂蔻����ϰ��ѱ��?
						if(OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8 ){ 			//����ʐM�`�F�b�N�����d�q�}�̒�~��
							Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
							break;											//�L�����Z������
						}
						if(ECCTL.Ec_FailureContact_Sts == 1){
							// ��Q�A���[���s���͂Ƃ肯��������󂯕t���Ȃ�
							break;
						}
						// ���σ��[�_�̃J�[�h�������͎��������󂯕t���Ȃ�
						if(isEC_STS_CARD_PROC()) {
							break;
						}
						if (isEC_MIRYO_TIMEOUT()) {
							// �����c���Ɖ�^�C���A�E�g��͖�����^�C���A�E�g���󂯕t���Ȃ�
							break;
						}
// MH810105(S) QR�R�[�h���ϑΉ� GT-4100 #6591 ���Z���A�ҋ@��ʕ��A�^�C�}�[�o�ߌ��LCD��ʂ������\�������܂܂ƂȂ�
//						msg = KEY_TEN_F4;							// �����I�ɐ��Z���~�Ƃ���
//						PayInfo_Class = 2;							// ���Z���f�[�^�̏����敪�𐸎Z���~�ɂ���
//						SetNoPayData();								// ���Z���~�f�[�^��ݒ肷��
//						Cansel_Ele_Start();
//						time_out = 1;									// ��ѱ�Ĕ����׸�ON 07-01-22�ǉ�
//					}else{
//						// ���W�F�N�g���o�[�ʒm
//						PKTcmd_notice_ope( LCDBM_OPCD_REJECT_LEVER, (ushort)0 );
//						break;
//					}
						// ���W�F�N�g���o�[�ʒm
						PKTcmd_notice_ope( LCDBM_OPCD_REJECT_LEVER, (ushort)0 );
						break;
					}else{
						if(OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8 ){ 			//����ʐM�`�F�b�N�����d�q�}�̒�~��
							break;											//�L�����Z������
						}
						if( OPECTL.op_faz == 9 ) {
							break;
						}
						if(ECCTL.Ec_FailureContact_Sts == 1){
							// ��Q�A���[���s���͂Ƃ肯��������󂯕t���Ȃ�
							break;
						}
						// ���σ��[�_�̃J�[�h�������͎��������󂯕t���Ȃ�
						if(isEC_STS_CARD_PROC()) {
							break;
						}
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							break;
						}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
							// ���W�F�N�g���o�[�ʒm
						PKTcmd_notice_ope( LCDBM_OPCD_REJECT_LEVER, (ushort)0 );
						break;
					}
// MH810105(E) QR�R�[�h���ϑΉ� GT-4100 #6591 ���Z���A�ҋ@��ʕ��A�^�C�}�[�o�ߌ��LCD��ʂ������\�������܂܂ƂȂ�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6398 �d�q�}�l�[�AQR�R�[�h�ǎ掞�̏������|�b�v�A�b�v�����Ԍo�߂łƂ��Ă��܂�
				}
				// no break;

			// op_Payment_Cancel(�V�K�쐬rev.1100)�̏������s��Ȃ��ŏ]���̏������s��
			case LCD_LCDBM_OPCD_PAY_STP:			// ���Z���~�v����M
			case LCD_LCDBM_OPCD_STA_NOT:			// �N���ʒm��M��
			case LCD_LCD_DISCONNECT:				// ���Z�b�g�ʒm/�ؒf���m
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_ؼު����ް_COIN_RJ_EVT
// MH810100(S) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
//			case TIMEOUT8:											// ������߂蔻����ϰ��ѱ��
// MH810100(E) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
			case KEY_TEN_F4:										// �ݷ�[���] �����
// MH810100(S) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
//			case CANCEL_EVENT:										// �L�����Z������
// MH810100(E) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
				fus_subcal_flg = 1;
// MH322914 (s) kasiyama 2016/07/11 �����{�N���W�b�g�̎���ŉ�ʃ��b�N�΍�[���ʃo�ONo.1002](MH324709)
//				if( msg == KEY_TEN_F4 && creSeisanInfo.amount != 0L){// ������� ON and �N���W�b�g���Z��
//					break;
//				}
// MH322914 (e) kasiyama 2016/07/11 �����{�N���W�b�g�̎���ŉ�ʃ��b�N�΍�[���ʃo�ONo.1002](MH324709)
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//				if( msg == KEY_TEN_F4 || msg == CANCEL_EVENT|| msg == COIN_RJ_EVT){	// �������?(Y) or �L�����Z�� or ���Z���~
// MH810100(S) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
//				if((  msg == KEY_TEN_F4 || msg == CANCEL_EVENT|| msg == COIN_RJ_EVT) ||
				if( ( msg == KEY_TEN_F4 || msg == COIN_RJ_EVT) ||
// MH810100(E) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
					( msg == LCD_LCDBM_OPCD_PAY_STP ) 		||		// ���Z���~�v����M��
					( msg == LCD_LCDBM_OPCD_STA_NOT ) 		||		// �N���ʒm��M��
					( msg == LCD_LCD_DISCONNECT ) ){				// ���Z�b�g�ʒm/�ؒf���m
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH322914 (s) kasiyama 2016/07/11 �����{�N���W�b�g�̎���ŉ�ʃ��b�N�΍�[���ʃo�ONo.1002](MH324709)
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//					if( creSeisanInfo.amount != 0L){// �N���W�b�g���Z��
//						break;
//					}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
					if(ECCTL.Ec_FailureContact_Sts == 1){
						// ��Q�A���[���s���͂Ƃ肯��������󂯕t���Ȃ�
						break;
					}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
					// ���σ��[�_�̃J�[�h�������͎��������󂯕t���Ȃ�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//					if(isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC) {
					if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
						break;
					}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH322914 (e) kasiyama 2016/07/11 �����{�N���W�b�g�̎���ŉ�ʃ��b�N�΍�[���ʃo�ONo.1002](MH324709)
					if(OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8){ 		//����ʐM�`�F�b�N�����d�q�}�̒�~��
						break;											//�L�����Z������
					}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					if( dsp_fusoku || edy_dsp.BIT.edy_dsp_Warning ){
					if( dsp_fusoku ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) G.So IC�N���W�b�g�Ή�
						if (isEC_USE() != 0) {
							Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(���ʉ��PNo.1362/���Z���~�Ɠ����ɓd�q�}�l�[�^�b�`�ŁA�J�[�h�����肷�邪���Z���~���Ă��܂��s��΍�)
							Target_MsgGet_delete1( OPETCBNO, EC_CYCLIC_DISP_TIMEOUT );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(���ʉ��PNo.1362/���Z���~�Ɠ����ɓd�q�}�l�[�^�b�`�ŁA�J�[�h�����肷�邪���Z���~���Ă��܂��s��΍�)
						} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
							Lagcan( OPETCBNO, 10 );
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(���ʉ��PNo.1362/���Z���~�Ɠ����ɓd�q�}�l�[�^�b�`�ŁA�J�[�h�����肷�邪���Z���~���Ă��܂��s��΍�)
							Target_MsgGet_delete1( OPETCBNO, TIMEOUT10 );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(���ʉ��PNo.1362/���Z���~�Ɠ����ɓd�q�}�l�[�^�b�`�ŁA�J�[�h�����肷�邪���Z���~���Ă��܂��s��΍�)
// MH321800(S) G.So IC�N���W�b�g�Ή�
						}
// MH321800(E) G.So IC�N���W�b�g�Ή�
					}
					if( OPECTL.op_faz == 9 ) {
						break;
					}
					if (msg == KEY_TEN_F4) {
						BUZPI();
					} else {
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//						// �U�֌��ԏo�ɂ̏ꍇ�A�ȉ��̏����͎���{�^�������Ɠ���
//						msg = KEY_TEN_F4;
						if (( msg != LCD_LCDBM_OPCD_PAY_STP ) 		&&		// ���Z���~�v����M��
							( msg != LCD_LCDBM_OPCD_STA_NOT ) 		&&		// �N���ʒm��M��
							( msg != LCD_LCD_DISCONNECT ) ){ 				// ���Z�b�g�ʒm/�ؒf���m
								// �U�֌��ԏo�ɂ̏ꍇ�A�ȉ��̏����͎���{�^�������Ɠ���
								msg = KEY_TEN_F4;
						}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
					}
					PayInfo_Class = 2;								// ���Z���f�[�^�̏����敪�𐸎Z���~�ɂ���
					SetNoPayData();								// ���Z���~�f�[�^��ݒ肷��
					ope_anm( AVM_STOP );							// ������~�ųݽ
					Cansel_Ele_Start();
				}
// MH810100(S) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
//				if( msg == TIMEOUT8 ){								// ������߂蔻����ϰ��ѱ��?
//					if(OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8 ){ 			//����ʐM�`�F�b�N�����d�q�}�̒�~��
//						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
//						break;											//�L�����Z������
//					}
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
////					if ( creSeisanInfo.amount != 0L ){ 				// ���Ȃ݂ɁA�N���W�b�g���Z�J�n���͓����s�ɂ��邱�ƂŁA
////						credit_Timeout8Flag = 1;					// NOTE_EVT ����������̂ŁA CPrmSS[S_TYP][66] �b Lagtim�����.
////					}else{
//					{
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//						msg = KEY_TEN_F4;							// �����I�ɐ��Z���~�Ƃ���
//						PayInfo_Class = 2;							// ���Z���f�[�^�̏����敪�𐸎Z���~�ɂ���
//						SetNoPayData();								// ���Z���~�f�[�^��ݒ肷��
//					}
//					Cansel_Ele_Start();
//					time_out = 1;									// ��ѱ�Ĕ����׸�ON 07-01-22�ǉ�
//				}
// MH810100(E) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
				if( r_zero == 0 ){
					if( OPECTL.Pay_mod != 2 ){
						break;
					}
				}
				if( (first_announce_flag == 0)&&(nyukin_flag == 1)) { // ���A�i�E���X�A��������
					first_announce_flag = 1;

					if (OPECTL.op_faz == 0){
						OPECTL.op_faz = 1;								// ������
					}
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
////					if( OPECTL.Pay_mod != 2 ){						// �C�����Z�łȂ�
//					if ((OPECTL.Pay_mod != 2)						// �C�����Z�łȂ�
//					&&	(Suica_Rec.Data.BIT.MIRYO_TIMEOUT == 0)) {	// �����m�肵�Ă��Ȃ�
//// MH321800(E) hosoda IC�N���W�b�g�Ή�
//							ope_anm( AVM_RYOUSYUU );						// ������t���y�ї̎��رųݽ
//					}											// �C�����Z�łȂ�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
				}
// MH810104 GG119202(S) �݂Ȃ��x�ƂŎ����Ƃ肯������Ȃ�
				if (msg == KEY_TEN_F4 || msg == COIN_RJ_EVT) {
// MH810104 GG119202(E) �݂Ȃ��x�ƂŎ����Ƃ肯������Ȃ�
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�A���[�����)
				LagCan500ms(LAG500_EC_AUTO_CANCEL_TIMER);		// �A���[�������̐��Z�����L�����Z���^�C�}���Z�b�g
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�A���[�����)
// MH810104 GG119202(S) �݂Ȃ��x�ƂŎ����Ƃ肯������Ȃ�
				}
// MH810104 GG119202(E) �݂Ȃ��x�ƂŎ����Ƃ肯������Ȃ�

				/*** ������XXs�Ő��Z���~�Ƃ��� ***/
				Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
				ret = in_mony( msg, 0 );							// �����������

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_���_��������
				switch(msg) {
					// ���Z���~�v����M��
					case LCD_LCDBM_OPCD_PAY_STP:
// MH810100(S) K.Onodera 2020/2/18 #3870 ������ʂɖ߂�܂Ő��Z�ł��Ȃ��Ȃ��Ă��܂�
//						// ���~��
//						if (OPECTL.op_faz == 3) {
//							// ����ʒm(���Z���~����(OK))���M	0=OK/1=NG
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
//						}
//						// OPECTL.op_faz != 3
//						else {
//							// ����ʒm(���Z���~����(NG))���M	0=OK/1=NG
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
//						}
						require_cancel_res = 1;
// MH810100(E) K.Onodera 2020/2/18 #3870 ������ʂɖ߂�܂Ő��Z�ł��Ȃ��Ȃ��Ă��܂�
						break;

					// �N���ʒm��M��
					case LCD_LCDBM_OPCD_STA_NOT:
						// ���~��
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
//// MH810100(S) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
////						if (OPECTL.op_faz == 3) {
//						if((OPECTL.op_faz == 3) && ( OPECTL.Ope_mod != 0 )){	// ���Z���~��������op_mod00�ւ̑J�ڑ҂��ł͂Ȃ��H
//// MH810100(E) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
//							// 0=�N��
//							if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
//								OPECTL.init_sts = 0xff;	// ������������(�N���ʒm��M�ς�)��ԂƂ���
//							}
//// MH810100(S) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
//							// 1=�ʏ�
//							else{
//								OPECTL.init_sts = 0xfe;	// �����������ς�(�N���ʒm��M�ς�)��ԂƂ���
//							}
//// MH810100(E) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
						if((OPECTL.op_faz == 3) && ( OPECTL.init_sts != 1 )){	// ���Z���~�������ŏ����������ς݂ł͂Ȃ��H
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
							OPECTL.Ope_mod = 255;
						}
						break;

					// LCD_DISCONNECT��M����IC�ڼޯĔ�⍇�����̎��ɔ��s
					case LCD_LCD_DISCONNECT:
						// ���~��
						if( OPECTL.op_faz == 3 ){
							// op_init00(�N����)
							OPECTL.Ope_mod = 255;
							OPECTL.init_sts = 0;	// ��������������ԂƂ���
						}
						break;

					default:
						break;
				}		// switch(msg) {
				// �ȉ��͋��ʏ���

				// ���_��������
				if (( msg == COIN_EVT ) || ( msg == NOTE_EVT )) {
					// ���Z�����H
					if( !ryo_buf.zankin ){
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
//// MH810100(S) K.Onodera  2020/03/06 �Ԕԃ`�P�b�g���X(�C�x���g�����̓x�Ɏc���ω��ʒm/����ʒm�����M�����s��C��)
//						if( !cmp_send ){	// �c���ω��ʒm(���Z����)/����ʒm(���Z�����ʒm(���Z�ς݈ē�))�����M
//// MH810100(E) K.Onodera  2020/03/06 �Ԕԃ`�P�b�g���X(�C�x���g�����̓x�Ɏc���ω��ʒm/����ʒm�����M�����s��C��)
//// MH810100(S) S.Takahashi 2020/03/02 �����z���x��ĕ\�������
//							lcdbm_pay_rem_chg(1);							// �c���ω��ʒm(���Z����)
//// MH810100(E) S.Takahashi 2020/03/02 �����z���x��ĕ\�������
//							// ����ʒm(���Z�����ʒm(���Z�ς݈ē�))���M
//							// 0=0�~�\��/1=׸���щ������/2=���Z�ς݈ē�
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 2 );
//// MH810100(S) K.Onodera  2020/03/06 �Ԕԃ`�P�b�g���X(�C�x���g�����̓x�Ɏc���ω��ʒm/����ʒm�����M�����s��C��)
//							cmp_send = 1;
//						}
//// MH810100(E) K.Onodera  2020/03/06 �Ԕԃ`�P�b�g���X(�C�x���g�����̓x�Ɏc���ω��ʒm/����ʒm�����M�����s��C��)
						if ( cmp_send != 1 ) {		// ����̎c���ω��ʒm(���Z����)�����M
							lcdbm_pay_rem_chg(1);	// �c���ω��ʒm(���Z����)
							cmp_send = 1;
						}
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
					}else{
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(#3995 ���Z���~����ƁA���Z�m�F��ʂ̉������b�Z�[�W���Đ�����Ă��܂�)
// MH810100(S) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4067 �����}���Ŏc���ω��ʒm�����M����Ȃ�)
//						if( ( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 ) ){
						if( (( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 ))  ||
							( (OPECTL.NT_QSIG == 1 )||( OPECTL.NT_QSIG == 5 )) ){
// MH810100(E) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4067 �����}���Ŏc���ω��ʒm�����M����Ȃ�)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(#3995 ���Z���~����ƁA���Z�m�F��ʂ̉������b�Z�[�W���Đ�����Ă��܂�)
							lcdbm_pay_rem_chg(0);		// �c���ω��ʒm(���Z��)
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(#3995 ���Z���~����ƁA���Z�m�F��ʂ̉������b�Z�[�W���Đ�����Ă��܂�)
// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
							Lagtim( OPETCBNO, 28, AP_CHARGE_INTERVAL_SEC * 50);	// 100�~�����^�C�}
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z
						}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(#3995 ���Z���~����ƁA���Z�m�F��ʂ̉������b�Z�[�W���Đ�����Ă��܂�)
					}
				}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_���_��������
				break;

// MH321800(S) hosoda IC�N���W�b�g�Ή� (�A���[�����)
			case AUTO_CANCEL:
// MH810100(S) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
				if( lcdbm_Flag_ResetRequest ){	// ���Z���~�v�������H
					// ���Z���~������A�N���ҋ@��ԂɑJ�ڂ���(OPECTL.init_sts�͗v���ɉ������l�ɕύX�ς�)
					// �A���A�Ӑ}�I�ɑҋ@��ԂɑJ�ڂ�����ꍇ�͋N���ҋ@��Ԃɂ͑J�ڂ��Ȃ�
					if( OPECTL.Ope_mod != 0 ){
						OPECTL.Ope_mod = 255;
					}
				}
// MH810100(E) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
				msg = KEY_TEN_F4;
				goto _MSG_RETRY;
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�A���[�����)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			case IBK_EDY_RCV:															// EdyӼޭ�ق���̎�M�ް��H
//				Ope_Edy_Event( msg, OPECTL.Ope_mod );
//				break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			case ELE_EVT_STOP:											// �d�q�}�̒�~������M
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( edy_dsp.BIT.edy_Miryo_Loss || 
//				  ( Suica_Rec.Data.BIT.MIRYO_ARM_DISP && !SUICA_CM_BV_RD_STOP ) ) // �����莸�s���ү���ޕ\�����H
				if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP && !SUICA_CM_BV_RD_STOP ) // �����莸�s���ү���ޕ\�����H
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					op_SuicaFusokuOff();								// �u�����N�\���Ƃ��Ԃ�̂�ү���ޏ���
				switch( CCT_Cansel_Status.BIT.STOP_REASON ){			// ��~�v��
					case REASON_PAY_END:								// ���Z�������̓d�q�}�̒�~�҂����킹
// �d�l�ύX(S) K.Onodera 2016/10/28 �U�։ߕ������ƒޑK�؂蕪��
//						if ( ryo_buf.turisen == 0 ){
						if ( ryo_buf.turisen == 0 && ryo_buf.kabarai == 0 ){
// �d�l�ύX(E) K.Onodera 2016/10/28 �U�։ߕ������ƒޑK�؂蕪��
							ret = 1;												// ���Z�I��(�ޖ���)
						}else{
							ret = 2;												// ���Z�I��(�ޗL��)
						}
// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
//						lcdbm_notice_ope(LCDBM_OPCD_PAY_CMP_NOT, 2);	// 0=0�~�\��/1=׸���щ������/2=���Z�ς݈ē�
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
// MH810103(e) �d�q�}�l�[�Ή� �����ʒm
						#if (3 == AUTO_PAYMENT_PROGRAM)
						BUZPIPIPI();
						#endif
						break;
					case REASON_CARD_READ:								// �J�[�h�}�����̑҂����킹������
						if( (Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
								  OPECTL.op_faz == 3 || 
								  ryo_buf.zankin == 0 ){
							opr_snd( 13 );						// �O�r�o
						}else{
							msg = ARC_CR_R_EVT;
							CCT_Cansel_Status.BYTE = 0;						// �d�q�}�̒�~�Ǘ��̈�̏�����
							goto OPMOD_ARC_CR_R_EVT_DUMMY;
						}
						break;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
					// QR�ް���M����IC�ڼޯĒ�~�������Ƃ������R
					case REASON_QR_USE:
// MH810103(s) �d�q�}�l�[�Ή�
//						// ��IC�ڼޯĐݒ肠��
//						// 02-0024 �@:EC����ذ��	0=�ڑ��Ȃ�/1=�ڑ�����
//						if (prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1 ) {
						if(isEC_USE()) {
// MH810103(e) �d�q�}�l�[�Ή�
							// IC�ڼޯĒ�~�҂���ϒ�~
							Lagcan(OPETCBNO, TIMERNO_ICCREDIT_STOP);

							// QR�׸�== ���p�m�F��	(0 = idle, 1 = ���p�m�F��, 2 = ���p��)
							if (lcdbm_Flag_QRuse_ICCuse == 1) {
								// QR�ް�����(OK))���M	����(0:OK,1:NG(�r��),2:NG(�������))
								lcdbm_QR_data_res( 0 );							// ����(0:OK,1:NG(�r��),2:NG(�������))

// MH810100(S) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
								// QR�ް���M & QR�ް�����(OK)���M���LCD_IN_CAR_INFO�҂�
								lcdbm_QRans_InCar_status = 1;
// MH810100(E) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�

								// QR�׸ށ����p��
								lcdbm_Flag_QRuse_ICCuse = 2;
							}
						}
						// IC�ڼޯĐݒ�Ȃ�
						else {
							// nothing todo
						}
						// REASON_CARD_READ�Ɠ����̏������s��
						if( (Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
								  OPECTL.op_faz == 3 || 
								  ryo_buf.zankin == 0 ){
							opr_snd( 13 );						// �O�r�o
						}else{
// MH810100(S) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
//							msg = ARC_CR_R_EVT;
//							CCT_Cansel_Status.BYTE = 0;						// �d�q�}�̒�~�Ǘ��̈�̏�����
//							goto OPMOD_ARC_CR_R_EVT_DUMMY;
							CCT_Cansel_Status.BYTE = 0;						// �d�q�}�̒�~�Ǘ��̈�̏�����
// MH810100(E) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
						}
						break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
					case REASON_RTM_REMOTE_PAY:							// ���u���Z�J�n
						// IC�ڼޯĒ�~�҂���ϒ�~
						Lagcan(OPETCBNO, TIMERNO_ICCREDIT_STOP);
						// ���σ��[�_��~�m�F��
						if (lcdbm_Flag_RemotePay_ICCuse == 1) {
							// ��t�ς�
							OPECTL.f_rtm_remote_pay_flg = 1;
							// ���u���Z�i���A���^�C���j�J�nOK
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
							// ���u���Z�J�nOK
							lcdbm_Flag_RemotePay_ICCuse = 2;
						}
						// REASON_CARD_READ�Ɠ����̏������s��
						if( (Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
								OPECTL.op_faz == 3 || 
								ryo_buf.zankin == 0 ){
						}else{
							CCT_Cansel_Status.BYTE = 0;					// �d�q�}�̒�~�Ǘ��̈�̏�����
						}
						break;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
					case REASON_MIF_READ:								// Mifare�^�b�`���̑҂����킹������
						if( (Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
								  OPECTL.op_faz == 3 || 
								  ryo_buf.zankin == 0 ){
							op_MifareStop_with_LED();					// Mifare��~
						}else{
							msg = IBK_MIF_A2_OK_EVT;
							CCT_Cansel_Status.BYTE = 0;						// �d�q�}�̒�~�Ǘ��̈�̏�����
							goto OPMOD_MIF_A2_OK_DUMMY;
						}
						break;
					case REASON_MIF_WRITE_LOSS:							// Mifare�����ݎ��s
						OpMif_snd( 0xA2, 0 );							// Mifare���޾ݽ&�ް��Ǐo���ėv��
						LagTim500ms( LAG500_MIF_LED_ONOFF, 1, op_IccLedOnOff );
						read_sht_cls();									// �����I�ɼ���������
						cn_stat( 2, 1 );								// ���������s��
						break;
					default:
						break;
				}
				CCT_Cansel_Status.BYTE = 0;						// �d�q�}�̒�~�Ǘ��̈�̏�����
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����

// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
				// ���σ��[�_�֘A�����\�����̓J�[�h�f���o��
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if( ec_MessagePtnNum != 0 ){
				if( isEC_MSG_DISP() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					if( OPECTL.other_machine_card ) {
						opr_snd( 13 );								// �ۗ��ʒu����ł��߂�
					}
					else {
						opr_snd( 2 );								// �O�r�o
					}
					break;
				}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
				if( OPECTL.op_faz > 2 ){							// ���Z���ł͖���?
					if( OPE_red != 2 ){
						if( OPECTL.other_machine_card )
							opr_snd( 13 );									// �ۗ��ʒu����ł��߂�
						else
							opr_snd( 2 );									// �O�r�o				
					}
				}else if( ryo_buf.zankin == 0 ){					// �c�z0�~
					if( OPE_red != 2 ){
						if( OPECTL.other_machine_card )
							opr_snd( 13 );									// �ۗ��ʒu����ł��߂�
						else
							opr_snd( 2 );									// �O�r�o				
					}
				}else{
					if(( OPECTL.op_faz == 0 )&&( r_zero != 0 )){	//
						Lagcan( OPETCBNO, 1 );						// ��ϰ1ؾ��(���ڰ��ݐ���p)
					}
					Lagcan( OPETCBNO, 7 );							// ��ϰ7ؾ��(�ǎ�װ�\���p��ϰ)
					Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
					if( !Op_StopModuleWait( REASON_CARD_READ ) ){
// MH810100(S) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//						f_al_card_wait = 1;							// 1=���C�J�[�h��������Aal_card() ���{�O�ɓd�q�}�l�[���[�_�Ȃǂ̒�~�҂������Ă���
// MH810100(E) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
						break;
					}
OPMOD_ARC_CR_R_EVT_DUMMY:
// MH810100(S) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//					f_al_card_wait = 0;								// al_card() ���{�̂��߃N���A
// MH810100(E) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
					ret = al_card( msg , 0 );						// ���ޏ���
					if (ret >= 0x100) {
						ret = 0;		// ���̑��̏����ۗ����
						break;
					}
				}
				break;

			case ELE_EVENT_CANSEL:									// �S�f�o�C�X�̒�~�����C�x���g
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				if( !announce_flag ) {								// ���ޔ������ųݽ���ł���Ε�����~�͂��Ȃ�
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//					ope_anm( AVM_STOP );							// ������~�ųݽ
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
				ret = 10;											// �ҋ@�ɖ߂�
				break;

			case ARC_CR_E_EVT:										// �I������ގ�M
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				break;
			case TIMEOUT7:											// �ǎ�װ�\���p��ϰ��ѱ��
			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( msg == ARC_CR_EOT_EVT ){
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
					// ���σ��[�_�֘A�����\�����̓J�[�h�f���o��
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//					if( ec_MessagePtnNum != 0 ){
					if( isEC_MSG_DISP() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810100(S) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//						f_al_card_wait = 0;							// al_card()���{�O�Ɍ��ԋp�������߃N���A
// MH810100(E) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						if( ec_MessagePtnNum == 98 &&				// ����NG���b�Z�[�W�\����
//							!Suica_Rec.Data.BIT.CTRL_CARD ){		// ���ޔ����ς�
						if( isEC_NG_MSG_DISP() &&					// ����NG���b�Z�[�W�\����
							!isEC_CARD_INSERT() ){					// ���ޔ����ς�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							// ����NG���b�Z�[�W�\�����ɃN���W�b�g�J�[�h�����Ă���
							// ���C�J�[�h������������Ǝ��C���[�_�[�V���b�^�[��
							// �߂Ă��܂����߁A�����Ŕ�����
							break;
						}
						read_sht_cls();
						break;
					}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//					if( !Suica_Rec.Data.BIT.MIRYO_TIMEOUT){
//						creMessageAnaOnOff( 0, 0 );						// �װ�\������
//					}
//					if ( credit_error_ari != 0 ){
//						credit_error_ari = 0;
//						if( OPECTL.op_faz != 3 ){
//							cn_stat( 1, 1 );							// ����������
//							cn_stat( 3, 0);								// �d�ݓ�����
//						}
//						else {											// ���
//							cn_stat( 2, 2 );							// �����s��
//						}
//					}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
					Ope2_WarningDispEnd();							// �װ�\������
				}
				if( Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_MIRYO_AFTER_STOP ){	// ������ѱ�Č��Suica��~�ݒ�̏ꍇ�͕\���������Ȃ�
					if( SUICA_CM_BV_RD_STOP && !(( RD_mod >= 10 )&&( RD_mod <= 13 )) ){// ������ѱ�Č�͑S�Ă̔}�̂��g�p�s�Ƃ���ꍇ�ňړ����E�������҂��ł͂Ȃ��ꍇ
						read_sht_cls();
					}
				}else if( OPECTL.ChkPassSyu ){										// ���(Mifare)�̖₢���킹���̃J�[�h�f���o��
					read_sht_cls();
				}
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				// ���σ��[�_�֘A�����\�����ł͂Ȃ��H
//				if( ec_MessagePtnNum == 0 ){
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//				// �̎��؍ĕ\��
//				if( (OPECTL.PriUsed == 0 || OPECTL.CR_ERR_DSP != 0) && OPECTL.ChkPassSyu == 0 ){
//					// ����ߖ��g�p �� �װ�\�����s���Ă��Ȃ� ���� ������₢���킹���ł͂Ȃ�
//					if( ryodsp ){
//						Lcd_Receipt_disp();
//					}else{
//						dsp_intime( 7, OPECTL.Pr_LokNo );			// ���Ɏ����\��
//					}
//// MH810105(S) MH364301 �C���{�C�X�Ή�
//					OPECTL.f_CrErrDisp = 0;							// �G���[��\��
//// MH810105(E) MH364301 �C���{�C�X�Ή�
//				}
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
				OPECTL.PassNearEnd = 0;								// �����؂�ԋ�
				if( SyuseiEndKeyChk() == 2 ){
					read_sht_cls();										/* ���Cذ�ް������۰�� */
				}
				break;
			case IBK_MIF_A2_OK_EVT:									// Mifare�ް��Ǐo������
				if( OPECTL.InquiryFlg )								// �N���W�b�gHOST�ڑ����͎󂯕t���Ȃ�
					break;
				if(( OPECTL.op_faz > 2 )||( ryo_buf.zankin == 0 )){	// ���Z���ł͖��� or �c�z0�~
					break;
				}else{
					if(( OPECTL.op_faz == 0 )&&( r_zero != 0 )){	//
						Lagcan( OPETCBNO, 1 );						// ��ϰ1ؾ��(���ڰ��ݐ���p)
					}
					Lagcan( OPETCBNO, 7 );							// ��ϰ7ؾ��(�ǎ�װ�\���p��ϰ)
					Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
					if( !Op_StopModuleWait( REASON_MIF_READ ) )
						break;
OPMOD_MIF_A2_OK_DUMMY:
					// Mifare�����ݏ�ԃ`�F�b�N
					if( ( MifStat == MIF_WRITING) ||					// �����݊����҂�
						( MifStat == MIF_WROTE_CMPLT) ||				// �����݊���OK��i���ޒu�����ςȂ��j
						( MifStat == MIF_DATA_ERR) ){					// �f�[�^�G���[
						break;
					}
					else if (MifStat == MIF_WROTE_FAIL) {				// ������NG��A���궰������҂�
						if( memcmp( &MIF_LastReadCardData.Sid,  &MIF_CARD_SID[0], 4 ) != 0 ){
							break;										// ���궰�ނł͂Ȃ��ꍇ�A���ނ������Ȃ�̂�҂�
						}
						// �e��񏉊����i���߂ēǏo�����ꍇ�Ɠ����̑���ɂ���j
						LagCan500ms( LAG500_MIF_WRITING_FAIL_BUZ );		// NG�޻ް��~
						// �װ����
						op_RestoreErrDsp(ryodsp);
					}

					memcpy( &Mifare_LastReadCardID[0], &MIF_CARD_SID[0], 4 );	// ����ID�ۑ�
					LagTim500ms( LAG500_MIF_LED_ONOFF, (1*2), op_IccLedOff );	// 1�b�����

					ret = al_iccard( msg , 0 );							// IC���ޏ���

					if( MifStat == MIF_WROTE_FAIL )					// �����ݎ��s����ēx�^�b�`���ꂽ�ꍇ
						MifStat = MIF_WAITING;					// ���ނ��������s�Ȃ�����ŃX�e�[�^�X��߂�

					if (OPECTL.MIF_CR_ERR) {							// Mifare���ް��װ
						MifStat = MIF_DATA_ERR;							// status = data error
						OPECTL.MIF_CR_ERR = 0;							// �װ�׸�OFF

						LagTim500ms( LAG500_MIF_LED_ONOFF, (1*2), op_IccLedOff2 );	// ICC LED�ē_��

					}
					else {
						MifStat = MIF_WRITING;
					}
				}
				break;
			case	TIMEOUT12:
				if (MIF_ENDSTS.sts1 == 0)
					break;			// �J�[�h�̃G���[�����o���Ă��Ȃ���Ή������Ȃ�
		// NTNET�₢���킹���ɃJ�[�h�����ɂȂ�ƁA�J�[�h���o���ꎞ��~����̂�
		// ���̃^�C�}�[��IBK_MIF_A2_EN_EVT�̑�p������

			case IBK_MIF_A2_NG_EVT:									// Mifare�ް��Ǐo��NG
				switch (MifStat) {
				case MIF_DATA_ERR:
					if( MIF_ENDSTS.sts1 == 0x30 ){					// ���ޖ���
						op_RestoreErrDsp(ryodsp);					// �̎��؍ĕ\��
						OPECTL.PassNearEnd = 0;						// �����؂�ԋ��׸�
						MifStat = MIF_WAITING;
					}
				case MIF_WAITING:
				case MIF_WROTE_FAIL:
					if (OPECTL.ChkPassSyu == 0) {					// �⍇�����͖���
				/* 07.01.31 NOTE
					 �⍇�����ɃJ�[�h��������Ă��A���J�[�h��OK�Ȃ琸�Z�ł��Ă��܂��̂�
					 �⍇�������܂ŁA�J�[�h���o���~����
						����OK��A4���M�ŃG���[�ƂȂ�i���̌�Č��o�J�n�j
						����NG���J�[�h�G���[�\����A�^�C�}�[12��A2 NG���������s
				*/
						OpMif_snd( 0xA2, 0 );						// A2�đ��M
					}
					break;
				case MIF_WRITING:
				case MIF_WROTE_CMPLT:
					break;
				default:
					break;
				}
				break;
			case IBK_MIF_A4_OK_EVT:									// Mifare�ް��������݊���
				if( MifStat == MIF_WRITING ){						// Mifare�����݊����҂�
					BUZPI();										// OK��
					op_RestoreErrDsp(ryodsp);						// �G���[����
					
					// ���ޏ����ݎ��s�ر�ɓo�^����Ă���΍폜
					Mifare_WrtNgDataUpdate( 2, (void *)&MIF_LastReadCardData );
					ret = al_iccard( msg , 0 );						// �����v�Z����
					op_MifareStop();

					MifStat = MIF_WROTE_CMPLT;						// Mifare�����݊���
				}
				break;
			
			case IBK_MIF_A4_NG_EVT:									// Mifare�ް���������NG
				if( MifStat == MIF_WRITING ){						// Mifare�����݊����҂�
					BUZPIPI();										// NG��
					grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[14] );				// " �ēx�J�[�h���^�b�`���ĉ����� "

					LagTim500ms( LAG500_MIF_WRITING_FAIL_BUZ, (1*2), op_Cycle_BUZPIPI );	// 1�b�� �Ĵװ�޻ް��
					MifStat = MIF_WROTE_FAIL;						// �����ݎ��s�i��ݾفj

					/* ���ޏ����ݎ��s�ر�� Read�����ް���o�^ */
					Mifare_WrtNgDataUpdate( 3, (void *)&MIF_LastReadCardData );	// �o�^

					if( Op_StopModuleWait( REASON_MIF_WRITE_LOSS ) == 1 ){
					OpMif_snd( 0xA2, 0 );							// Mifare���޾ݽ&�ް��Ǐo���ėv��
					LagTim500ms( LAG500_MIF_LED_ONOFF, 1, op_IccLedOnOff );
																	// ICC LED�ē_��
																	// ����1�b�� ICC LED�����w�����Ă���̂ŏ㏑������
					read_sht_cls();									// �����I�ɼ���������
					cn_stat( 2, 1 );								// ���������s��
					}
				}
				break;

			case IBK_NTNET_CHKPASS:									// ����⍇�������ް���M
			case TIMEOUT6:											// ������⍇���Ď���ϰ��ѱ��
				if(( OPECTL.op_faz > 2 )||( ryo_buf.zankin == 0 )){	// ���Z���ł͖��� or �c�z0�~
					break;
				}else{
					if( OPECTL.ChkPassSyu == 1 ){					// APS�����
						if( msg == TIMEOUT6 ){
							OPECTL.ChkPassSyu = 0xff;
						}
						msg = ARC_CR_R_EVT;
						ret = al_card( msg , 0 );					// ���ޏ���

						if( OPECTL.CR_ERR_DSP ){
						read_sht_opn();								// ���Cذ�ް���������� 
						if( ryo_buf.zankin != 0 ) {						// �c�z�������
							cn_stat( 3, 2 );						// ���� & COIN�ē�����
							if( OPECTL.CR_ERR_DSP == 4 )
								read_sht_opn();
						}
						}
					}
					else if( OPECTL.ChkPassSyu == 2 ){				// Mifare�����
						if( msg == TIMEOUT6 ){
							OPECTL.ChkPassSyu = 0xff;
						}
						msg = IBK_MIF_A2_OK_EVT;
						ret = al_iccard( msg , 0 );					// IC���ޏ���
						if ((OPECTL.MIF_CR_ERR) && 					// Mifare���ް��װ
							(OPECTL.CR_ERR_DSP != 99)) {			// �⍇�����łȂ�
							MifStat = MIF_DATA_ERR;					// status = data error
							OPECTL.MIF_CR_ERR = 0;					// �װ�׸�OFF

							LagTim500ms( LAG500_MIF_LED_ONOFF, (1*2), op_IccLedOff2 );		// ICC LED�ē_��
							read_sht_opn();
							cn_stat( 3, 2 );						// ���� & COIN�ē�����
						}
						else {
							MifStat = MIF_WRITING;
						}
					}
				}
				break;

			case TIMEOUT1:
				if( r_zero == 0 ){									// 0�~���Z
					cn_stat( 2, 2 );								// �����s��
					ret = 1;										// ���Z����������
					break;
				}
				fus_subcal_flg = 1;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( ( OPECTL.op_faz == 0 && Suica_Rec.Data.BIT.CTRL || Edy_Rec.edy_status.BIT.CTRL) ){
				if( ( OPECTL.op_faz == 0 && Suica_Rec.Data.BIT.CTRL) ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					time_out = 1;									// ��ѱ�Ĕ����׸�ON
				}
				cansel_status.BIT.INITIALIZE = 1;					// �d�q�}�̒�~����

				if(( OPECTL.op_faz == 9 || OPECTL.op_faz == 3 || ( OPECTL.op_faz == 0 && time_out == 1))){		// ������������s���̏ꍇ
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
					if( Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL ){	// Suica���L�����Ō�ɑ��M�����̂���t���̏ꍇ
						Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
					}
					else{													// Suica�����łɎ�t�s��ԁi�ʏ킠�肦�Ȃ��j
						if( !Suica_Rec.Data.BIT.CTRL )
							Op_Cansel_Wait_sub( 0 );						// Suica��~�ς��׸ނ𗧂Ă�
					}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					if( Edy_Rec.edy_status.BIT.CTRL )
//						Edy_StopAndLedOff();
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

					LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 20, Op_ReturnLastTimer );	// �c�z�\���p��ϰ���ꎞ�I�Ɏg�p����
				}

				if( OPECTL.op_faz >= 3 ){
					if( OPECTL.op_faz == 8 && CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){
						cn_stat( 2, 2 );									// �����s��
						Lagcan( OPETCBNO, 1 );								// ��ϰ1ؾ��(���ڰ��ݐ���p)
						Lagtim( OPETCBNO, 2, 10*50 );						// ү��Ď���ϰ�N��							
						break;
					}

					if( !Op_Cansel_Wait_sub( 0xff ) )				// �S�f�o�C�X�̃L�����Z���ς݁H
						Op_Cansel_Wait_sub( 2 );					// ���b�N�̒�~
					break;
				}
				cn_stat( 2, 2 );									// �����s��
				PayInfo_Class = 2;									// ���Z���f�[�^�̏����敪�𐸎Z���~�ɂ���
				SetNoPayData();										// ���Z���~�f�[�^��ݒ肷��
				OPECTL.op_faz = 4;									// ���ڰ���̪��� ��ѱ��
				Lagcan( OPETCBNO, 1 );								// ��ϰ1ؾ��(���ڰ��ݐ���p)
				Lagtim( OPETCBNO, 2, 10*50 );						// ү��Ď���ϰ�N��
				break;

			case TIMEOUT2:											// ү��Ď���ϰ��ѱ��
				switch( OPECTL.op_faz ){
					case 2:											// ����
						if( ryo_buf.turisen ){						//
							SFV_DAT.reffal = ryo_buf.turisen;		//
							ret = 3;								// ���Z����������
						}else{
							ret = 1;								// ���Z����������
						}
						break;
					case 3:											// ���~
					case 4:											// ��ѱ��
					case 9:											// ��ѱ��
						if( ryo_buf.nyukin ){						//
							ryo_buf.turisen = ryo_buf.nyukin;		//
							if( SFV_DAT.reffal ){					// �s������?
								ryo_buf.fusoku += SFV_DAT.reffal;	// �x�����s���z
							}
							if( SFV_DAT.nt_escrow ){				// �����߂��װ
								ryo_buf.fusoku += 1000L;			// �x�����s���z
							}
							chu_isu();								// ���~�W�v
							Lagtim( OPETCBNO, 1, 10*50 );			// ��ϰ1�N��(���ڰ��ݐ���p)
						}else{
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
//							if( carduse() || Suica_Rec.Data.BIT.PAY_CTRL ){	// ���ގg�p ��Suica�g�p��?
							if( carduse() || Suica_Rec.Data.BIT.PAY_CTRL ||	// ���ގg�p ��Suica�g�p��?
								isEC_PAY_CANCEL() ){				// ���σ��[�_�̌��ϐ��Z���~�H
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
								chu_isu();							// ���~����
								svs_renzoku();						// ���޽���A���}��
							}
							if( !Suica_Rec.suica_err_event.BYTE ){					// Suica�װ���������Ă��Ȃ�
								if( Suica_Rec.Data.BIT.CTRL ){
									Suica_Ctrl( S_CNTL_DATA, 0 );				// Suica���p��s�ɂ���	
									OPECTL.op_faz = 9;
									LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 20, Op_ReturnLastTimer );	// �c�z�\���p��ϰ���ꎞ�I�Ɏg�p����
									break;
								}
							}
							// �������Ȃ��A�����������ꍇ�́A�P�b��TIMEOUT1�𔭐�������
							// �i�ԋ������Ȃ��̂ŁA��L�̂悤�ɂP�O�b���҂K�v���Ȃ��j
							Lagtim( OPETCBNO, 1, 50 );				// ��ϰ1�N��(���ڰ��ݐ���p)
						}
						break;
					case 8:											// ���Z�����҂��H
						if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// ���Z�������̓d�q�}�̒�~�҂����킹
							// ���ү���������Ԃ��Ȃ��ꍇ�ASuica���������Ȃ��\�������邽�߁A
							// ��~�v������20�b�o���Ă��I�����Ȃ��ꍇ�́ASuica�E���ү��Ƃ��Ɋ��������Ƃ݂Ȃ�
							if( !CCT_Cansel_Status.BIT.SUICA_END ){	// Suica���I�����Ă��Ȃ��ꍇ
								CCT_Cansel_Status.BIT.SUICA_END = 1;// Suica�I��
							}
							if( !Op_StopModuleWait_sub( 0xff ) ){
								Op_StopModuleWait_sub( 3 );				// ���ү��I��
							}
						}
						break;
					default:
						ret = 10;									// �ҋ@�ɖ߂�
						break;
				}
				break;

			case TIMEOUT11:											// Suica�u���ޔF�شװ/���Z�����v�����҂�����ѱ�Ēʒm
				Ope_TimeOut_11( OPECTL.Ope_mod,  0 );
				break;

			case TIMEOUT10:											// �uSuica��~��̎�t�����M�҂��vor�u������ʕ\���ؑցv����ѱ�Ēʒm
				Ope_TimeOut_10( OPECTL.Ope_mod,  0 );
				break;
// MH321800(S) G.So IC�N���W�b�g�Ή�
			case EC_CYCLIC_DISP_TIMEOUT:							// �uec��~��̎�t�����M�҂��vor�u������ʕ\���ؑցv����ѱ�Ēʒm
				Ope_TimeOut_Cyclic_Disp( OPECTL.Ope_mod,  0 );
				break;

			case EC_RECEPT_SEND_TIMEOUT:							// ec�u���ޔF�شװ/���Z�����v�����҂�����ѱ�Ēʒm�
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,  0 );
				break;
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//			case	TIMEOUT9:
//				if( cre_ctl.Status == CRE_STS_IDLE ){				// �����҂���ѱ��
//					if( cre_ctl.Timeout_Announce != 0 ){			// 04, 06 �^�C���A�E�g�̂Ƃ������A�i�E���X����
//						//�đ��M���s.or.���g���C�s�\�� TimeOut�������ꍇ
//						credit_result = -2;	//�����A����舵�����ł��܂���
//					}
//				}
//				break;
//			case	CRE_EVT_04_OK:
//				vl_now = V_CRE;	//�����v�Z�ŎQ��.
//				// vl_now : ���Q�Ƃ��ď��� > Ryo_Cal
//				ryo_cal( 0, OPECTL.Pr_LokNo );
//
//				if( prm_get( COM_PRM, S_CRE, 3, 1, 1 ) == 1 ){		//���p���׏�/�̎��� �������s����
//					if( OPECTL.RECI_SW == 0 ){						// �̎������ݖ��g�p?
//						OPECTL.RECI_SW = 1;							// �̎������ݎg�p
//					}
//				}
//
//				ac_flg.cycl_fg = 10;								// ����
//				in_mony( msg, 0 );
//
//				credit_result = 1;
//
//				// �{���� in_mony �̐�� invcrd �̖߂�l���g���������A���̃C�x���g�Ƌ��ʉ����Ă��邽��
//				// �e�����o�Ȃ��l�Ɂ��ł�����x�`�F�b�N����. �N���W�b�g�ŁA B3����̏ꍇ�͕K���A���Z�͊�������.
//				if ( ryo_buf.turisen == 0 ){
//					credit_turi = 1;
//				}else{
//					credit_turi = 2;
//				}
//				if (ryo_buf.nyukin != 0){
//					credit_nyukin = 1;								// �N���W�b�g���Z�O�̓�������
//				}
//
//				OPECTL.InquiryFlg = 0;
//
//				cre_ctl.SalesKind = CRE_KIND_PAYMENT;				// ���M�v���ɐ��Z�����ݒ�
//
//				if( creCtrl( CRE_EVT_SEND_SALES ) != 0 ){			// ����˗��f�[�^(05)�̏��񑗐M
//					;	// �Y�������Ȃ�
//				}else{
//					;	// �Y�������Ȃ�
//				}
//				break;
//			case	CRE_EVT_02_OK:
//			case	CRE_EVT_06_OK:
//			case	CRE_EVT_08_OK:
//			case	CRE_EVT_0A_OK:
//				{
//					;	// �Y�������Ȃ�
//				}
//				break;
//			case	CRE_EVT_02_NG:
//			case	CRE_EVT_08_NG:
//			case	CRE_EVT_06_TOUT:
//				//�m�f������M
//				if(OPECTL.InquiryFlg == 1) {						// �^�M�₢������
//					credit_result = -2;										//  �����A����舵�����ł��܂��� 
//					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ
//
//					OPECTL.InquiryFlg = 0;
//					creSeisanInfo.amount = 0;
//					ryo_buf.credit.pay_ryo = 0;
//				}
//				break;
//			case	CRE_EVT_0A_NG:
//				{
//					;	// �Y�������Ȃ�
//				}
//				break;
//			case	CRE_EVT_04_NG1:
//			case	CRE_EVT_04_NG2:
//				//�m�f������M
//				if( msg == CRE_EVT_04_NG1 ){
//					credit_result = -3;									// ���̃J�[�h�͎g���܂���
//				}else if( msg == CRE_EVT_04_NG2 ){
//					credit_result = -2;									// �����A����舵�����ł��܂���
//				}
//				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ
//
//				OPECTL.InquiryFlg = 0;
//				creSeisanInfo.amount = 0;
//				ryo_buf.credit.pay_ryo = 0;
//				break;
//			case	CRE_EVT_06_NG:
//				//�m�f������M
//				;	// �����ł͊Y�������Ȃ�
//				break;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH321800(S) G.So IC�N���W�b�g�Ή�
			case EC_EVT_CANCEL_PAY_OFF:							// ���Z�L�����Z���g���K����
// MH810103 GG119202(S) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
//			case EC_INQUIRY_WAIT_TIMEOUT:						// �⍇��(������)�҂��^�C���A�E�g
// MH810103 GG119202(E) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
			// �⍇���^�C���A�E�g�Ő��Z���L�����Z������B�i�݂Ȃ����ςƂ͂��Ȃ��j
			// �i���Z�L�����Z���̓^�C���A�E�g�Ɠ�������������j
				OPECTL.InquiryFlg = 0;							// �J�[�h�������t���OOFF
				Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 0;		// ���Ϗ�������M�t���O������
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				Ec_Settlement_Sts = EC_SETT_STS_NONE;			// ���Ϗ�Ԃ�OFF�ɂ���
				ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;		// ���Ϗ�Ԃ�OFF�ɂ���
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
				ec_MessageAnaOnOff( 0, 0 );						// �������̕��������ɖ߂�
				ope_anm(AVM_STOP);								// ���΂炭���҂����������̉�����~
// MH810103 GG119202(S) �d�q�}�l�[���ώ��̌��ό��ʎ�M�҂��^�C�}�ǉ�
				ac_flg.ec_deemed_fg = 0;						// �݂Ȃ����ϕ��d�p�t���OOFF
// MH810103 GG119202(E) �d�q�}�l�[���ώ��̌��ό��ʎ�M�҂��^�C�}�ǉ�
				// no break
			case	EC_EVT_CRE_PAID_NG:
			// �O���Ɖ�NG���A�����ɂ���B�J�[�h��������Ƃ���B
// MH810100(S) S.Nishimoto 2020/08/19 #4606 �Ԕԃ`�P�b�g���X(���p�ȃ|�b�v�A�b�v���\������Ă��܂�)
//// MH810100(S) K.Onodera 2020/3/10 �Ԕԃ`�P�b�g���X(�|�b�v�A�b�v)
//				lcdbm_notice_dsp( POP_INQUIRY_NG, 0 );	// �Ɖ�NG�|�b�v�A�b�v
//// MH810100(E) K.Onodera 2020/3/10 �Ԕԃ`�P�b�g���X(�|�b�v�A�b�v)
// MH810100(E) S.Nishimoto 2020/08/19 #4606 �Ԕԃ`�P�b�g���X(���p�ȃ|�b�v�A�b�v���\������Ă��܂�)
				if( ryo_buf.nyukin && OPECTL.credit_in_mony ){	// ��������ŸڼޯĂƂ̂���Ⴂ�����̏ꍇ
					// in_mony()����cn_crdt()�����s����ryo_buf.nyukin���X�V���Ă�K�v�Z���s��
					// ��x�Acn_crdt()�����s����΁A���������������Z�����̂�
					// in_mony(COIN_EVT)���������s����
					in_mony ( COIN_EVT, 0 );					// ��ݲ���ē���������āA��������(ryo_buf�ر�̍X�V)���s��
					OPECTL.credit_in_mony = 0;					// ����Ⴂ�׸޸ر
// MH810100(S) K.Onodera 2020/2/27 �Ԕԃ`�P�b�g���X(�c���ω��ʒm)
					if( ryo_buf.zankin ){
						lcdbm_pay_rem_chg(0);					// �c���ω��ʒm(���Z��)
					}
// MH810100(E) K.Onodera 2020/2/27 �Ԕԃ`�P�b�g���X(�c���ω��ʒm)
				}

				// �c�肪0�~�łȂ���
				if( ryo_buf.zankin ){
					OPECTL.holdPayoutForEcStop = 0;						// �ۗ���������
// MH810100(S) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
					if( lcdbm_Flag_ResetRequest ){	// ���Z���~�v�������H
						// ���Z���~������A�N���ҋ@��ԂɑJ�ڂ���(OPECTL.init_sts�͗v���ɉ������l�ɕύX�ς�)
						// �A���A�Ӑ}�I�ɑҋ@��ԂɑJ�ڂ�����ꍇ�͋N���ҋ@��Ԃɂ͑J�ڂ��Ȃ�
						if( OPECTL.Ope_mod != 0 ){
							OPECTL.Ope_mod = 255;
						}
						msg = KEY_TEN_F4;
						goto _MSG_RETRY;
					}
// MH810100(E) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
					// EC_EVT_CRE_PAID_NG�A�ē����\�ɂ���
					if( msg == EC_EVT_CRE_PAID_NG ){
						// ���ލ����Ȃ��H �܂��́A���σ��[�_�֘A�����\�����ł͂Ȃ��H
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						if( !Suica_Rec.Data.BIT.CTRL_CARD || ec_MessagePtnNum == 0 ){
						if( !isEC_CARD_INSERT() || !isEC_MSG_DISP() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							read_sht_opn();
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
							if (check_enable_multisettle() <= 1 ||
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//								!isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
//								// �}���`�u�����h�ȊO�A�܂��́A�d�q�}�l�[�u�����h���I���̏ꍇ��
//								// �ē����Ƃ���
								!EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
								// �V���O���u�����h�A�܂��́A�d�q�}�l�[�AQR�R�[�h���ψȊO�̏ꍇ��
								// �ē����Ƃ���
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
							cn_stat( 3, 2 );								// ���� & COIN�ē�����
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
							}
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
							if( MIFARE_CARD_DoesUse ){						// Mifare���L���ȏꍇ
								op_MifareStart();							// Mifare�L��
							}
						}
					}
					// EC_INQUIRY_WAIT_TIMEOUT�AEC_EVT_CANCEL_PAY_OFF�A�����������
					else {
						// �������(����{�^��)�g���K����
// MH810100(S) S.Nishimoto 2020/07/27 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//						msg = KEY_TEN_F4;
//						goto _MSG_RETRY;
// MH810100(S) S.Nishimoto 2020/07/29 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
						// ����ʒm(�J�[�h���Ϗ�Ԓʒm)
// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
//						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x07);	// �N���W�b�g�J�[�h�L�����Z��
						if(isEcBrandNoEMoney(RecvBrandResTbl.no, 0) ){
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x08);	// �d�q�}�l�[�L�����Z��
						}else{
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x07);	// �N���W�b�g�J�[�h�L�����Z��
						}
// MH810103(e) �d�q�}�l�[�Ή� �����ʒm
// MH810100(E) S.Nishimoto 2020/07/29 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
						// ���W�F�N�g���o�[�ʒm
						PKTcmd_notice_ope(LCDBM_OPCD_REJECT_LEVER, (ushort)0);
// MH810100(E) S.Nishimoto 2020/07/27 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
					}
				}
				else {
			//��zankin�Ȃ��i���Z�����j���̂���Ⴂ�ɑ΂��Ă�credit_in_mony�ŏ�������邽�߁A���o���͖������ėǂ�
					OPECTL.holdPayoutForEcStop = 0;						// �ۗ���������
				}
				break;
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			case	IFMPAY_GENGAKU:
//			// �c�z����v���邱�� && ���z���c�z�ł��邱��
//				if (vl_ggs.zangaku == ryo_buf.zankin &&
//					!OPECTL.InquiryFlg &&	// �ڼޯ�HOST�ڑ����y�ѓd�q���ς̖�������1�ɂȂ�
//					vl_ggs.waribikigaku <= ryo_buf.zankin) {
//					ope_imf_Answer(1);					// ����ʒm
//					wopelg(OPLOG_PARKI_GENGAKU, 0, vl_ggs.waribikigaku);	// ����o�^
//					vl_now = V_GNG;
//					ryo_cal(3, OPECTL.Pr_LokNo);		// �T�[�r�X���Ƃ��Čv�Z
//					ret = in_mony(msg, 0);
//				}
//				else {
//					ope_imf_Answer(0);					// ���ےʒm
//				}
//				break;
//
//			case	IFMPAY_FURIKAE:
//				// �U�֌��Ԏ����O�^�C�}��~
//				if (ope_Furikae_start(vl_frs.lockno) &&
//					!OPECTL.InquiryFlg ){	// �ڼޯ�HOST�ڑ����y�ѓd�q���ς̖�������1�ɂȂ�
//					ope_ifm_GetFurikaeGaku(&vl_frs);
//					ope_imf_Answer(1);				// ����ʒm
//				} else {
//					ope_imf_Answer(0);				// �ُ�ʒm
//					break;
//				}
//				wopelg(OPLOG_PARKI_FURIKAE, 0, 0);	// ����o�^
//				FurikaeMotoSts = 1;			// �U�֌���ԃt���O 1:�Ԃ���
//				OpeNtnetAddedInfo.PayMethod = 11;	// ���Z���@=11:�C�����Z
//				// ����L/������
//				if (vl_frs.antipassoff_req) {
//					// ������p�L��Ȃ���������
//					ryo_buf.apass_off = 1;
//				} else {
//					// ��������Ȃ�U�֎��s
//					ope_ifm_FurikaeCalc( 0 );				// ������Z�ŐU�֊����Ȃ�U�֊z���O�~
//					ret = in_mony(IFMPAY_FURIKAE, 0);
//				}
//				break;
//
//			case	SYU_SEI_SEISAN:
//				OpeNtnetAddedInfo.PayMethod = 11;	// ���Z���@=11:�C�����Z
//				// ����L/������
//				if (vl_frs.antipassoff_req) {		// �C�����Œ�����g�p
//					// ������p�L��Ȃ���������
//					ryo_buf.apass_off = 1;			// �����I�ɃA���`�p�X��OFF�ɂ���
//					
//					set03_98 = (uchar)prm_get(COM_PRM, S_TYP, 98, 1, 4);
//					if( (set03_98 == 2)||(set03_98 == 3)){		// �C�����Ŏg������������ɂ�萸�Z
//						if(	!al_pasck_syusei() ){				// ������g�p�\?
//							ryo_cal( 1, OPECTL.Pr_LokNo );
//						}
//					}
//				}
//				ope_ifm_FurikaeCalc( 1 );			// ������Z�ŐU�֊����Ȃ�U�֊z���O�~
//				ret = in_mony(IFMPAY_FURIKAE, 0);
//				break;
//
//			case CAR_FURIKAE_OUT:									// �U�֌��̎Ԃ��o��
//				if (FurikaeMotoSts != 0) {
//					// �U�֌���ԃt���O 0:�`�F�b�N�s�v �łȂ����"2:�ԂȂ�"��
//					FurikaeMotoSts = 2;			// �U�֌���ԃt���O 2:�ԂȂ�
//				}
//				break;
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

			case OPE_REQ_CALC_FEE:						// �����v�Z�v��
				// �����v�Z
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);	// �S�Ԏ����e�[�u�����M
				break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
			case EC_INQUIRY_WAIT_TIMEOUT:							// �⍇��(������)�҂��^�C���A�E�g
				err_chk( (char)jvma_setup.mdl, ERR_EC_RCV_SETT_FAIL, 1, 0, 0 );
				// ���ό��ʎ�M���s
				Suica_Rec.suica_err_event.BIT.SETT_RCV_FAIL = 1;
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
				jvma_trb(ERR_SUICA_RECEIVE);
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
// MH810103 GG119202(S) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
// MH810103 GG119201(S) �n�E�X�J�[�h�u�����h���菈���s�
//				if (!IsSettlementBrand(RecvBrandResTbl.no)) {
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//				if (!isEcBrandNoEMoney(RecvBrandResTbl.no, 0) &&
				if (!EcUseKindCheck(convert_brandno(RecvBrandResTbl.no)) &&
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
					RecvBrandResTbl.no != BRANDNO_CREDIT) {
// MH810103 GG119201(E) �n�E�X�J�[�h�u�����h���菈���s�
					// ���ςȂ��u�����h�^�㌈�σu�����h�͐��Z���~�Ƃ���
					queset(OPETCBNO, EC_EVT_CANCEL_PAY_OFF, 0, 0);
					break;
				}
// MH810103 GG119202(E) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
				// no break
// MH810103 GG119202(E) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
			case EC_EVT_DEEMED_SETTLEMENT:
				// �݂Ȃ����σg���K����
				EcDeemedSettlement(&OPECTL.Ope_mod);
				break;
			case EC_EVT_CARD_STS_UPDATE:
				// �J�[�h�������Ȃ�j��
				if( OPECTL.InquiryFlg != 0 ){
					break;
				}
				LagCan500ms(LAG500_EC_START_REMOVAL_TIMER);			// ���ޔ������^�C�}�N���^�C�}�[��~
				// ����ʒm(�J�[�h�����ʒm)
				lcdbm_notice_ope(LCDBM_OPCD_CARD_EJECTED, 0x00);
				// ����������̶��ޔ������ųݽ���H
				if ( announce_flag ) {
				// announce_end�̓Z�b�g�����ASuica_Rec.Data.BIT.CTRL_CARD����Ń��g���C����
					ret = 10;										// �ҋ@��ʂ�
				}
				else {
// MH321800(S) �J�[�h������ԍX�V�ʒm�ƂƂ肯���̋����ɂ��a����؂����s�����s��C��
					// ���ό���NG��̶��ޔ������҂����Hor���ό���NG�\�����H
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//					if( ec_MessagePtnNum == 4 || ec_MessagePtnNum == 98 ){
					if( isEC_REMOVE_MSG_DISP() || isEC_NG_MSG_DISP() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
						queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);	// ope�֒ʒm(�ēx����NG��M�̏������s���B�������ĊJ������)
					}
// MH321800(E) �J�[�h������ԍX�V�ʒm�ƂƂ肯���̋����ɂ��a����؂����s�����s��C��
					// ���ό���NG��̶��ޔ������҂����H
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//					if( ec_MessagePtnNum == 4 ){
					if( isEC_REMOVE_MSG_DISP() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
						ope_anm( AVM_STOP );						// ������~�ųݽ
						ec_MessageAnaOnOff( 0, 0 );					// �u�J�[�h������艺�����v�\�������Ƃɖ߂�
					}
// MH321800(S) �J�[�h������ԍX�V�ʒm�ƂƂ肯���̋����ɂ��a����؂����s�����s��C��
//					queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);		// ope�֒ʒm(�ēx����NG��M�̏������s���B�������ĊJ������)
// MH321800(E) �J�[�h������ԍX�V�ʒm�ƂƂ肯���̋����ɂ��a����؂����s�����s��C��
				}
				break;
			case EC_REMOVAL_WAIT_TIMEOUT:
				// ����������̶��ޔ������ųݽ���H
				if ( announce_flag ) {
				// �Ȍ�Suica_Rec.Data.BIT.CTRL_CARD�͌����ɏI��
					announce_end = 1;								// �ųݽ���ރt���OON
					ret = 10;										// �ҋ@��ʂ�
				}
				// ���ό���NG��̶��ޔ������҂����H
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				else if( ec_MessagePtnNum == 4 ){
				else if( isEC_REMOVE_MSG_DISP() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);		// ope�֒ʒm(�ēx����NG��M�̏������s���B�������ĊJ������)
				}
				ope_anm( AVM_STOP );								// ������~�ųݽ
				ec_MessageAnaOnOff( 0, 0 );							// �u�J�[�h������艺�����v�\�������Ƃɖ߂�
				break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		// ���σ��[�_�[�ł̃u�����h�I���ɑΉ�
//			case EC_BRAND_RESULT_TCARD:		// ���σ��[�_�Ή� �u�����h�I������=T�J�[�h
//				AcceptTCard();
//				EnableNyukin();
//				break;				
//			case EC_BRAND_RESULT_HOUJIN:	// ���σ��[�_�Ή� �u�����h�I������=�@�l�J�[�h
//				if(InquireHoujin()) {		// �@�l�J�[�h�₢���킹
//					// �@�l�J�[�hNG
//					EnableNyukin();		
//				}
//				break;				
			case EC_BRAND_RESULT_UNSELECTED:
				BrandResultUnselected();
			// no break
			case EC_BRAND_RESULT_CANCELED:
				Ope_EcPayStart();			// �u�����h�I���f�[�^
// MH810103(s) �d�q�}�l�[�Ή� �d�q�}�l�[�u�����h�I�����̈ē�����(�L�����Z��)
				lcdbm_notice_ope(LCDBM_OPCD_BRAND_SELECT_CANCEL,0);
// MH810103(e) �d�q�}�l�[�Ή� �d�q�}�l�[�u�����h�I�����̈ē�����(�L�����Z��)
				break;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
			case EC_EVT_ENABLE_NYUKIN:
				if (isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
					// �d�q�}�l�[�u�����h�I����Ԃ̏ꍇ�͍ē����������Ȃ�
					break;
				}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				if( RecvBrandResTbl.no == BRANDNO_QR ){
					// QR�R�[�h�I����Ԃ̏ꍇ�͍ē����������Ȃ�
					break;
				}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				if (Suica_Rec.suica_err_event.BIT.SETT_RCV_FAIL != 0) {
					// ���ό��ʎ�M���s���݂͂Ȃ����ςƂȂ邽�߁A�ē����Ƃ��Ȃ�
					break;
				}

				cn_stat( 3, 2 );			// ���� & COIN�ē�����
				if (LCDNO == 3) {
					// �u�d�݂܂��͎����Ő��Z���ĉ������v�ĕ\���̂��ߍĕ`��
					OpeLcd(3);
				}
				break;
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
// MH810105(S) MH364301 �C���{�C�X�Ή�
			case OPE_OPNCLS_EVT:							// �����c�x�� event
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//				if (!IS_INVOICE) {
//					break;
//				}
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j

				statusChange_DispUpdate();					// ��ʍX�V
				break;
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�_�ύX))
			/****************************************************************/
			/* ���ɏ��(���ޏ��) = op_mod02								*/
			/*		  1 = ���Z����(�ނ薳��) 	���Z��������(op_mod03)��	*/
			/*		  2 = ���Z����(�ނ�L��) 	���Z��������(op_mod03)��	*/
			/*		  3 = ү��װ����         	���Z��������(op_mod03)��	*/
			/*		 10 = ���Z���~           	�ҋ@(op_mod00)�֖߂�		*/
			/****************************************************************/
			case LCD_IN_CAR_INFO:

// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
				Lagcan(OPETCBNO, 28);				//100�~�����^�C�}
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z

				// ���ɏ��(���ޏ��)�������Ɛݒ菈��
				lcdbm_rsp_in_car_info_proc();

				// �J�[�h���҂����
				switch( lcdbm_QRans_InCar_status ){
					// QR�f�[�^����M
					case 0:
						break;

					// QR�f�[�^����(OK)���M��̃J�[�h���҂�
					case 1:
						type = lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup;
						// �����d�����p�`�F�b�N�n�j�H
						if( type < 2 ){
							lcdbm_QRans_InCar_status = 0;
						}else{
							// �����d������
							if( type == 2 ){
								lcdbm_notice_dsp( POP_UNABLE_QR, 0 );	// �d��
							}
							// �f�[�^�s��
							else if( type == 3 ){
								lcdbm_notice_dsp( POP_UNABLE_QR, 1 );	// �f�[�^�s��
							}
							// ���̑��ُ�
							else{
								lcdbm_notice_dsp( POP_UNABLE_QR, 2 );	// ���̑��ُ�
							}

// GG124100(S) R.Endo 2022/08/25 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// // MH810100(S) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
// 							// ���ɏ��NG����
// 							lcdbm_notice_ope( LCDBM_OPCD_IN_INFO_NG, (ushort)0 );
// // MH810100(E) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
// GG124100(E) R.Endo 2022/08/25 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
						}
						break;

					// �J�[�h���҂����Ɍ�����������
					case 2:
						// �������Z�̍Čv�Z���s��
						lcdbm_QRans_InCar_status = 3;	// �������Z�� 0�ɂ���
						break;

					default:
						break;
				}
// MH810100(S) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j
				// �Z���^�[�₢���킹���ʂ�DC�p�̏��ɃZ�b�g
				SetQRMediabyQueryResult();
// MH810100(E) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
				if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
					// ���ɏ��`�F�b�N(����)
					retOnline = cal_cloud_discount_check();

					// ���ʔ���
					switch ( retOnline ) {
					case 0:		// ��������
						// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)�ݒ�
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// 						in_time_set();
						in_time_set(0);	// ���ڈȍ~
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�

// GG129000(S) H.Fujinaga 2023/02/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
						if (PayInfoChange_StateCheck() == 1){
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
							// ��������Ă���ꍇ���l�����čČv�Z
							cn_crdt();
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
							// ���u���Z�J�n���ł���Δ�����
							break;
						}
// GG129000(E) H.Fujinaga 2023/02/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
						// �g�p�ϔ}�̓o�^
						ope_AddUseMedia(&MediaDetail);

						// QR����OK����
						lcdbm_notice_ope(LCDBM_OPCD_QR_DISCOUNT_OK, 0);

						break;
					case 2:		// ���ɏ��NG(QR����Ȃ�)
						// ���ɏ��NG����
						lcdbm_notice_ope(LCDBM_OPCD_IN_INFO_NG, 0);

						break;
					case 1:		// ���ɏ��NG
					default:	// ���̑�
						// QR����f�[�^�����o�^
						ope_CancelRegist(&MediaDetail);

						// ���ɏ��NG����
						lcdbm_notice_ope(LCDBM_OPCD_IN_INFO_NG, 0);

						break;
					}
				} else {					// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

				// �����������s
// MH810100(S) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
//				if( !OnlineDiscount() ){
				retOnline = OnlineDiscount();
				if(!retOnline){
// MH810100(E) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
					// �g�p�ϔ}�̓o�^
					ope_AddUseMedia( &MediaDetail );
					// QR����OK����
					lcdbm_notice_ope( LCDBM_OPCD_QR_DISCOUNT_OK, (ushort)0 );
				}
// MH810100(S) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
				// �����������s�H ���O�񂩂�̍��������s��������
				else{
					// ���ɏ��NG����
// MH810100(S) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
//					lcdbm_notice_ope( LCDBM_OPCD_IN_INFO_NG, (ushort)0 );
					lcdbm_notice_ope( LCDBM_OPCD_IN_INFO_NG, (ushort)retOnline );
					if( retOnline < 100){
						// �Z���^�[�����NG�ɂȂ������̈ȊO�iRXM��NG�Ƃ�������)�̓L�����Z���f�[�^�𑗂�
						ope_CancelRegist(&MediaDetail );
					}
// MH810100(E) 2020/07/11 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
				}
// MH810100(E) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
				}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
				// �����Ő��Z���������𖞂������������҂��Ő��Z�������Ɣ��肵���ꍇ�̂��߂�
				// ����NG�ł���Γ��������T�u���ĂэĔ�����s��
				if ( retOnline ) {
					// ���������T�u�F�Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
					in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);
				}
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041

// MH810100(S) K.Onodera  2020/04/10 #4127 �Ԕԃ`�P�b�g���X(�ēxQR�R�[�h���Ȃ����ꍇ�ɕ\������Ȃ�)
//				// QR���p���̊����f�[�^��r�p�Ƀo�b�N�A�b�v
//				memcpy( DiscountBackUpArea, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo, sizeof(DiscountBackUpArea) );
// MH810100(E) K.Onodera  2020/04/10 #4127 �Ԕԃ`�P�b�g���X(�ēxQR�R�[�h���Ȃ����ꍇ�ɕ\������Ȃ�)

				key_num = OPECTL.Op_LokNo;			// �����͒l <- ���Ԉʒu�ԍ�
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					OPECTL.f_rtm_remote_pay_flg = 0;
					if (ryo_buf.nyukin) {
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
						OPECTL.CN_QSIG = 1;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
						in_mony(COIN_EVT, 0);			// �����Čv�Z
					}
					if (OPECTL.op_faz < 2) {
						cn_stat( 3, 2 );				// ���� & COIN�ē�����
					}
				}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j

				// �������z + �������z >= ���ԗ���
				if( (ryo_buf.waribik + ryo_buf.nyukin) >= ryo_buf.tyu_ryo ){
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
//// MH810100(S) S.Takahashi 2020/03/02 �����z���x��ĕ\�������
//					lcdbm_pay_rem_chg(1);							// �c���ω��ʒm(���Z����)
//// MH810100(E) S.Takahashi 2020/03/02 �����z���x��ĕ\�������
//					lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT,2 );	// ���Z�����ʒm
					if ( cmp_send != 2 ) {		// ����̎c���ω��ʒm(���Z����)�����M
						lcdbm_pay_rem_chg(1);	// �c���ω��ʒm(���Z����)
						cmp_send = 2;
					}
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041

// MH810100(S) K.Onodera 2020/03/23 #4043 �Ԕԃ`�P�b�g���X(�����Ɠ��������Ŋ����������Z�������A�������������o����Ȃ��s��C��)
//					// �ނ�K�Ȃ�
//					if (ryo_buf.turisen <= 0) {
//						ret = 1;				// ���Z����(�ނ薳��) 	���Z��������(op_mod03)��
//					}
//					// �ނ�K����(�������ꂽ��̊������z >= ���ԗ��� = �����͑S�z���߂�)
//					else {
// MH810100(E) K.Onodera 2020/03/23 #4043 �Ԕԃ`�P�b�g���X(�����Ɠ��������Ŋ����������Z�������A�������������o����Ȃ��s��C��)
// MH810103(s) �d�q�}�l�[�Ή� QR�T�[�r�X���Ő��Z���������ꍇ�ɁA�̎��؂̔��s���x���Ȃ�s��C��
						// OnlineDiscount����in_mony�œ������Ƃ����{���Ă��邵�A���σ��[�_������ꍇ��op_faz��8�ɂ��Ē�~�҂����邽�߂����ŏ��������Ă̓_��
//						ac_flg.cycl_fg = 10;				// ������
//						cn_crdt();
//						ac_flg.cycl_fg = 11;				// ���Z����
//						if( OPECTL.op_faz != 2 ){
//							svs_renzoku();					// ���޽���A���}��
//							cn_stat( 2, 2 );				// �����s��
//							Lagtim( OPETCBNO, 2, 10*50 );	// ү��Ď�10s = TIMEOUT2:
//							OPECTL.op_faz = 2;
//						}
// MH810103(e) �d�q�}�l�[�Ή� QR�T�[�r�X���Ő��Z���������ꍇ�ɁA�̎��؂̔��s���x���Ȃ�s��C��
// MH810100(S) K.Onodera 2020/03/23 #4043 �Ԕԃ`�P�b�g���X(�����Ɠ��������Ŋ����������Z�������A�������������o����Ȃ��s��C��)
//					}
// MH810100(E) K.Onodera 2020/03/23 #4043 �Ԕԃ`�P�b�g���X(�����Ɠ��������Ŋ����������Z�������A�������������o����Ȃ��s��C��)
				}
				// �������z + �������z < ���ԗ��� = �����㗿��������
				else{
					lcdbm_pay_rem_chg(0);							// �c���ω��ʒm(���Z��)

					// QR�t���O�== ���p��	(0 = idle, 1 = ���p�m�F��, 2 = ���p��)
					if (lcdbm_Flag_QRuse_ICCuse == 2) {
						// IC�ڼޯĊJ�n�v��
// MH810103(s) �d�q�}�l�[�Ή� #5403 �N���W�b�g���Z���x�z����QR�����Ō��x�z�ȉ��ɂȂ����ꍇ�A�N���W�b�g���Z���s����悤�ɂȂ�Ȃ�
//						Suica_Ctrl( S_CNTL_DATA, 0x01 );
						// ���σ��[�_���z�ύX
						if( isEC_USE()){						// ���σ��[�_��
							if(!Suica_Rec.Data.BIT.PAY_CTRL	&&	// ���ό��ʃf�[�^����M���Ă��炸		// �����Ӄ��r���[��
							Suica_Rec.Data.BIT.CTRL   			// ���σ��[�_�L����
							) {
								if (EcCheckBrandSelectTable(TRUE)){			// �u�����h�I���f�[�^�ɕύX������(�u�����h�I�𒆂łȂ��ꍇ�͏��TRUE)
									Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// ��t�֎~���M(���z�ύX)
								}
							}

							// ��t�����M�۔���͂����ōs��
							if(( OPECTL.op_faz < 2 ) &&						// ���Z������
							( !Suica_Rec.Data.BIT.PAY_CTRL ) &&			// ���ό��ʃf�[�^����M���Ă��炸
							( !Suica_Rec.Data.BIT.CTRL )){				// ���σ��[�_����
								EcSendCtrlEnableData();						// ��t�����M
							}
						}
// MH810103(s) �d�q�}�l�[�Ή� #5403 �N���W�b�g���Z���x�z����QR�����Ō��x�z�ȉ��ɂȂ����ꍇ�A�N���W�b�g���Z���s����悤�ɂȂ�Ȃ�
						// QR�t���O�N���A
						lcdbm_Flag_QRuse_ICCuse = 0;
					}
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
					if (lcdbm_Flag_RemotePay_ICCuse == 2) {
						// ���σ��[�_�Ċ�����
						Op_StopModuleWait_sub(2);
					}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				}
				lcdbm_QRans_InCar_status = 0;
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				lcdbm_Flag_RemotePay_ICCuse = 0;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
				Lagtim( OPETCBNO, 28, AP_CHARGE_INTERVAL_SEC * 50);	// 100�~�����^�C�}
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z
				break;

			// QR�m��E����f�[�^����(lcdbm_rsp_QR_conf_can_res_t)
			case LCD_QR_CONF_CAN_RES:
				break;

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			// ���[�����j�^�f�[�^����(lcdbm_rsp_LANE_res_t)
			case LCD_LANE_DATA_RES:
				break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

			// QR�f�[�^
			case LCD_QR_DATA:
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z�i���A���^�C���j�J�n��t�ς�
					lcdbm_QR_data_res( 1 );	// ����(1:NG(�r��))
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j

				if( ope_ParseBarcodeData( &MediaDetail ) ){
					result = ope_CanUseBarcode( &MediaDetail );
				}
				else{
					result = RESULT_BAR_FORMAT_ERR;
				}
				// �ŏI�ǎ�QR�̕ێ�(�����e�i���X�p)
				push_ticket( &MediaDetail,(ushort)result);

				// �G���[����H
				if( result != RESULT_NO_ERROR ){
					switch( result ){
						case RESULT_QR_INQUIRYING:			// �⍇����
							type = 1;	// �r��
							break;
						case RESULT_DISCOUNT_TIME_MAX:		// �������
// MH810100(S) K.Onodera 2020/03/04 #3994 ����QR�R�[�h���Ȃ��ƁuQR�̗��p�����𒴂��Ă��܂��B�v�ƈē�����
							type = 6;	// �������
							break;
// MH810100(E) K.Onodera 2020/03/04 #3994 ����QR�R�[�h���Ȃ��ƁuQR�̗��p�����𒴂��Ă��܂��B�v�ƈē�����
						case RESULT_BAR_USED:				// �g�p�ς̃o�[�R�[�h
// MH810100(S) K.Onodera 2020/03/04 #3994 ����QR�R�[�h���Ȃ��ƁuQR�̗��p�����𒴂��Ă��܂��B�v�ƈē�����
							type = 7;	// �g�p�ς̃o�[�R�[�h
							break;
// MH810100(E) K.Onodera 2020/03/04 #3994 ����QR�R�[�h���Ȃ��ƁuQR�̗��p�����𒴂��Ă��܂��B�v�ƈē�����
						case RESULT_BAR_READ_MAX:			// �o�[�R�[�h�������
							type = 2;	// �������
							break;
						case RESULT_BAR_EXPIRED:			// �����؂�o�[�R�[�h
							type = 3;	// �L�������؂�
							break;
						case RESULT_BAR_FORMAT_ERR:			// �t�H�[�}�b�g�G���[
							type = 4;	// �t�H�[�}�b�g�s��
							break;
						case RESULT_BAR_NOT_USE_CHANGEKIND:	// �����E�����ς݌�̎Ԏ�؊�
							type = 8;	// �����E�����ς݌�̎Ԏ�؊�
							break;
						case RESULT_BAR_ID_ERR:				// ID�G���[
						default:
							type = 5;	// �ΏۊO
							break;
					}
					// QR�����f�[�^
					lcdbm_QR_data_res( type );	// ����(0:OK,1:NG(�r��),2:NG(�������))
					break;
				}
// MH810102(S) �d�q�}�l�[�Ή� #5476 �y�s��s��z���Z�{�^���������Ă��u���Z���J�n�ł��܂���ł����v�ƂȂ�J�n�ł��Ȃ�
				if(	flag_stop){		// 1:���~��
					// QR�����f�[�^
					lcdbm_QR_data_res( 1 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
					break;
				}
// MH810102(E) �d�q�}�l�[�Ή� #5476 �y�s��s��z���Z�{�^���������Ă��u���Z���J�n�ł��܂���ł����v�ƂȂ�J�n�ł��Ȃ�

				// IC�ڼޯĐݒ肠��
// MH810103(s) �d�q�}�l�[�Ή�
//				if( prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1  ){
				if(isEC_USE()) {
// MH810103(e) �d�q�}�l�[�Ή�
// MH810100(S) S.Fujii 2020/08/27 #4764 �N���W�b�g���p�s�̏�Ԃ�QR�R�[�h�����p�ł��Ȃ�
//					// IC�ڼޯĒ�~�v��
//					Op_StopModuleWait( REASON_QR_USE );
//					// IC�ڼޯĒ�~�҂���ϊJ�n		48-0021	�ް���M�Ď����	(�⍇��)	�D�E:��M�Ď����(�⍇��) 1�`99=1�b�`99�b
//					Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, (res_wait_time * 50) );
//					// QR�׸ށ����p�m�F��	0 = idle, 1 = ���p�m�F��, 2 = ���p��
//					lcdbm_Flag_QRuse_ICCuse = 1;
// MH810100(S) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
//					if( Suica_Rec.Data.BIT.CTRL &&				// ��t��
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//					if( Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ) {	// �J�[�h�������H
					if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
						// QR�����f�[�^
						lcdbm_QR_data_res( 1 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
					} else if( Suica_Rec.Data.BIT.CTRL &&				// ��t��
// MH810100(E) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
						!Suica_Rec.suica_err_event.BYTE) {		// �G���[�Ȃ�
						// IC�ڼޯĒ�~�v��
// MH810103(s) �d�q�}�l�[�Ή� QR���p
//						Op_StopModuleWait( REASON_QR_USE );
//// MH810100(S) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
////						// IC�ڼޯĒ�~�҂���ϊJ�n		48-0021	�ް���M�Ď����	(�⍇��)	�D�E:��M�Ď����(�⍇��) 1�`99=1�b�`99�b
////						Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, (res_wait_time * 50) );
//						// IC�ڼޯĒ�~�҂���ϊJ�n		��t�s��M�҂��^�C�}�[(5�b) + 1�b
//						Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, ((EC_DISABLE_WAIT_TIME+1) * 50) );
//// MH810100(E) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
//						// QR�׸ށ����p�m�F��	0 = idle, 1 = ���p�m�F��, 2 = ���p��
//						lcdbm_Flag_QRuse_ICCuse = 1;
						if(Op_StopModuleWait( REASON_QR_USE )){
							// QR�����f�[�^
							lcdbm_QR_data_res( 0 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
							// QR�ް���M & QR�ް�����(OK)���M���LCD_IN_CAR_INFO�҂�
							lcdbm_QRans_InCar_status = 1;
							// QR�׸ށ����p��
							lcdbm_Flag_QRuse_ICCuse = 2;
						}else{
							// IC�ڼޯĒ�~�҂���ϊJ�n		��t�s��M�҂��^�C�}�[(5�b) + 1�b
							Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, ((EC_DISABLE_WAIT_TIME+1) * 50) );
							// QR�׸ށ����p�m�F��	0 = idle, 1 = ���p�m�F��, 2 = ���p��
							lcdbm_Flag_QRuse_ICCuse = 1;
						}
// MH810103(e) �d�q�}�l�[�Ή� QR���p
					} else {
						// QR�����f�[�^
						lcdbm_QR_data_res( 0 );	// ����(0:OK,1:NG(�r��),2:NG(�������))

						// QR�ް���M & QR�ް�����(OK)���M���LCD_IN_CAR_INFO�҂�
						lcdbm_QRans_InCar_status = 1;

						// QR�׸ށ����p��
						lcdbm_Flag_QRuse_ICCuse = 2;
					}
// MH810100(E) S.Fujii 2020/08/27 #4764 �N���W�b�g���p�s�̏�Ԃ�QR�R�[�h�����p�ł��Ȃ�
// MH810100(S) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
//				}
//				// QR�����f�[�^
//				lcdbm_QR_data_res( 0 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
//				
//				// QR�ް���M & QR�ް�����(OK)���M���LCD_IN_CAR_INFO�҂�
//				lcdbm_QRans_InCar_status = 1;
				} else {
					// QR�����f�[�^
					lcdbm_QR_data_res( 0 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
				
					// QR�ް���M & QR�ް�����(OK)���M���LCD_IN_CAR_INFO�҂�
					lcdbm_QRans_InCar_status = 1;
				}
// MH810100(E) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
				break;

			// ����ʒm(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// ����ʒm		���캰��
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// ���Z���~�v��
					case LCDBM_OPCD_PAY_STP:
						// ���σ��[�_�̃J�[�h�������H
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						if( isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ){
// MH810105(S) QR�R�[�h���ϑΉ� GT-4100 #6591 ���Z���A�ҋ@��ʕ��A�^�C�}�[�o�ߌ��LCD��ʂ������\�������܂܂ƂȂ�
//						if(isEC_STS_CARD_PROC()) {
						if(isEC_STS_CARD_PROC() || 								// ���σ��[�_�̃J�[�h�������͎��������󂯕t���Ȃ�
						(ECCTL.Ec_FailureContact_Sts == 1) ||					// ��Q�A���[���s���͂Ƃ肯��������󂯕t���Ȃ�
// GG129004(S) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
//// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
//						(OPECTL.f_rtm_remote_pay_flg != 0) ||					// ���u���Z�J�n��t�ς�
//// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
// GG129004(E) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
						 (OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8)){ 		// ����ʐM�`�F�b�N�����d�q�}�̒�~��
// MH810105(E) QR�R�[�h���ϑΉ� GT-4100 #6591 ���Z���A�ҋ@��ʕ��A�^�C�}�[�o�ߌ��LCD��ʂ������\�������܂܂ƂȂ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							// ����ʒm(���Z���~����(NG))���M	0=OK/1=NG
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						}
						// IC�ڼޯĔ�⍇����
						else {
// MH810102(S) �d�q�}�l�[�Ή� #5476 �y�s��s��z���Z�{�^���������Ă��u���Z���J�n�ł��܂���ł����v�ƂȂ�J�n�ł��Ȃ�
//							// ���~�C�x���g���M
//							queset( OPETCBNO, LCD_LCDBM_OPCD_PAY_STP, 0, NULL );
							if(lcdbm_QRans_InCar_status){
								// �J�[�h���҂���ԂȂ̂ŁA���̃^�C�~���O�ŗ�����NG����
								// ����ʒm(���Z���~����(NG))���M	0=OK/1=NG
								lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
							}else{
								flag_stop = 1;		// 1:���~��
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
								// ���[�����j�^�f�[�^�o�^(���Z���~)
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.54)�iGM803000���p�j
//								SetLaneFreeNum(ryo_buf.zankin);
								SetLaneFreeNum(0);								// �c�z�N���A
								SetLaneFeeKind(0);								// ������ʃN���A
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.54)�iGM803000���p�j
								ope_MakeLaneLog(LM_PAY_STOP);
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
								// ���~�C�x���g���M
								queset( OPETCBNO, LCD_LCDBM_OPCD_PAY_STP, 0, NULL );
							}
// MH810102(E) �d�q�}�l�[�Ή� #5476 �y�s��s��z���Z�{�^���������Ă��u���Z���J�n�ł��܂���ł����v�ƂȂ�J�n�ł��Ȃ�
						}
						break;

					// ���Z�J�n�v��
					case LCDBM_OPCD_PAY_STA:
						// ����ʒm(���Z�J�n����(NG))���M	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
// MH810102(S) �d�q�}�l�[�Ή� #5476 �y�s��s��z���Z�{�^���������Ă��u���Z���J�n�ł��܂���ł����v�ƂȂ�J�n�ł��Ȃ�
						// �t�F�[���Z�[�t����(op_mod02�ɊJ�n�ʒm���󂯂鎞�_�ŁA�t�F�[�Y���ꂪ�����Ă���B���~���Ă���͂�������������łЂ������Ă���ꍇ)
						if(flag_stop){
							// �ҋ@�ɖ߂�
							queset( OPETCBNO, ELE_EVENT_CANSEL, 0, NULL );					// �I�y�ɓd�q�}�̂̒�~��ʒm					
						}
// MH810102(E) �d�q�}�l�[�Ή� #5476 �y�s��s��z���Z�{�^���������Ă��u���Z���J�n�ł��܂���ł����v�ƂȂ�J�n�ł��Ȃ�
						break;

					// �̎��ؔ��s
					case LCDBM_OPCD_RCT_ISU:
// GG129000(S) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
						// �̎��ؔ��s�{�^��������̗̎��؈󎚕s�ŕK���̎��ؔ��s���s�\�����邽�ߕK���t���O�𗧂Ă�
						while ( OPECTL.RECI_SW == 0 ) {							// �̎������ݖ��g�p?
							OPECTL.RECI_SW = 1;									// �̎������ݎg�p
// GG129000(E) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
// MH810105(S) MH364301 �C���{�C�X�Ή� GT-4100
//						if( Ope_isPrinterReady() == 0 ){
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 						if (Ope_isPrinterReady() == 0 ||
						if ((!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) ||
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// GG129000(S) M.Fujikawa 2023/11/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7151
//							(IS_INVOICE && Ope_isJPrinterReady() == 0)) {		// �C���{�C�X�ݒ莞�̓��V�[�g�ƃW���[�i�����Е��ł������Ȃ������ꍇ
							(IS_INVOICE && Ope_isJPrinterReady() == 0 && ECCTL.Ec_FailureContact_Sts == 0)) {		// �C���{�C�X�ݒ莞�̓��V�[�g�ƃW���[�i�����Е��ł������Ȃ������ꍇ
// GG129000(E) M.Fujikawa 2023/11/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7151
// MH810105(E) MH364301 �C���{�C�X�Ή� GT-4100
							break;
						}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100
						if( isEC_USE() && ECCTL.Ec_FailureContact_Sts != 0 ){	// 1:��Q�A���[���s�������^2:��Q�A���[���s��
							if (ECCTL.Ec_FailureContact_Sts == 2) {
								// ��Q�A���[���s�ς݂̏ꍇ�A�̎��؃{�^�����󂯕t���Ȃ�
								break;
							}
							EcFailureContactPri();								// ��Q�A���[���s �����V�[�g�̂�
							op_SuicaFusokuOff();
							// 6�b�ԕ\������
							lcd_wmsg_dsp_elec(1, ERR_CHR[78], ERR_CHR[79], 0, 0, COLOR_RED, LCD_BLINK_OFF);
							LagTim500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER, 13, lcd_Recv_FailureContactData_edit);
							// LCD�ɒʒm
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0E);	// 14=��Q�A���[���s�ς�(�R�[�h����)
							
							ECCTL.Ec_FailureContact_Sts = 2;					// 2:��Q�A���[���s��
							if( OPECTL.RECI_SW == 0 ){							// �̎������ݖ��g�p?
								OPECTL.RECI_SW = 1;								// �̎������ݎg�p
							}
							break;
						}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100
						if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
						// �����m���Ȃ�󂯕t���Ȃ�
							break;
						}
						if( announce_flag ){
						// ���ޔ������ųݽ���Ȃ�󂯕t���Ȃ�
							break;
						}
						if( ReceiptChk() == 1 ){							// ڼ�Ĕ��s�s�v�ݒ莞
							break;
						}
						if(	(OPECTL.Pay_mod == 2)&&							// �C�����Z
							(!prm_get(COM_PRM, S_PRN, 32, 1, 6))){			// �̎��؂̔��s�Ȃ�
							BUZPIPI();
							break;
						}
						BUZPI();
// GG129000(S) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
// 						if( OPECTL.RECI_SW == 0 ){							// �̎������ݖ��g�p?
// GG129000(E) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
							if((( OPECTL.ChkPassSyu == 0 )&&				// �⍇�����ł͂Ȃ�?
							   ( OPECTL.op_faz != 0 || ryodsp ))			// �ҋ@��ԈȊO?
							   &&
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//								Ec_Settlement_Sts != EC_SETT_STS_CARD_PROC ) // �N���W�b�g�������H
								!isEC_STS_CARD_PROC() )
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							{
								wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
								grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      �̎��؂𔭍s���܂�      "
// MH810105(S) MH364301 �C���{�C�X�Ή�
								OPECTL.f_CrErrDisp = 0;						// �G���[��\��
// MH810105(E) MH364301 �C���{�C�X�Ή�
							}
// GG129000(S) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
// 							OPECTL.RECI_SW = 1;								// �̎������ݎg�p
// GG129000(E) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
						}
						break;

					// �N���ʒm
					case LCDBM_OPCD_STA_NOT:
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
						// ���Z�����������{��AOPECTL.init_sts�ɏ]����op_init00()�֑J��
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							OPECTL.init_sts = 0xff;	// ������������(�N���ʒm��M�ς�)��ԂƂ���
						}
						// 1=�ʏ�
						else{
							OPECTL.init_sts = 0xfe;	// �����������ς�(�N���ʒm��M�ς�)��ԂƂ���
						}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
						// IC�N���W�b�g�⍇����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						if( isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ){
						if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							// ���Z�b�g�v���L���t���OON
							lcdbm_Flag_ResetRequest = 2;
							// op_mod03(���Z����)�ڍs����loop�O�� -> op_init00(�N����)�ֈڍs����
						}
						// IC�N���W�b�g��⍇����
						else {
							// ���Z���~�������b�Z�[�W���M
							queset( OPETCBNO, LCD_LCDBM_OPCD_STA_NOT, 0, NULL );
						}
						break;

					// �N�������ʒm
					case LCDBM_OPCD_STA_CMP_NOT:
// MH810100(S) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
//						if(( OPECTL.Ope_mod == 255 ) && ( OPECTL.init_sts == 1 )){	// ������������ԂŋN���ҋ@�ɑJ�ځH
//							// �N���ʒm(1)��M��A���Z���~���������O�ɋN�������ʒm����M�����ꍇ
//							// FTP�͕s�v�Ȃ̂Ő��Z���~����������������op_mod00()�ɖ߂�
// MH810100(S) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
//						if( OPECTL.Ope_mod == 255 ){	// �N���ҋ@�ɑJ�ځH
						if(( OPECTL.init_sts == 0xfe ) || ( OPECTL.init_sts == 0xff )){	// �N���ʒm��M�ς݁H
// MH810100(E) K.Onodera 2020/03/11 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
							// op_mod02()�̏�Ԃ�FTP���Ă��Ȃ��̂ɋN�������ʒm����M����Ƃ������Ƃ�
							// LCD��FTP�s�v�Ƃ̔��f�Ȃ̂Ő��Z���~����������������op_mod00()�ɖ߂�
							OPECTL.init_sts = 1;
// MH810100(S) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(�ҋ@��ԑJ�ڏ������ʉ�)
//							// �N�������ʒm��M���̋N��������ԍX�V
//							PktStartUpStsChg( 2 );
// MH810100(E) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(�ҋ@��ԑJ�ڏ������ʉ�)
// MH810100(E) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�N���ҋ@��Ԗ������[�v�h�~)
							OPECTL.Ope_mod = 0;
						}
						break;

					// QR��~����			0=OK�^1=NG
					// QR�J�n����			0=OK�^1=NG
					case LCDBM_OPCD_QR_STP_RES:
					case LCDBM_OPCD_QR_STA_RES:
						// ��U�����҂��^�C�}��~
						Lagcan( OPETCBNO, TIMERNO_QR_START_RESP );
						Lagcan( OPETCBNO, TIMERNO_QR_STOP_RESP );

						// 0=OK
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
						}
						// 1=NG
						else {
							// QRذ�ފJ�n/��~����OK�҂���ײ����
							lcdbm_Counter_QR_StartStopRetry++;

							// ���g���C�񐔓��H
							if( lcdbm_Counter_QR_StartStopRetry < 3 ){
								if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_QR_STA_RES ){
									// QR�J�n�v�����M
									lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
									// �����҂��^�C�}�[�X�^�[�g
									Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
								}else{
									// QR��~�v�����M
									lcdbm_notice_ope( LCDBM_OPCD_QR_STP_REQ, 0 );
									// �����҂��^�C�}�[�X�^�[�g
									Lagtim( OPETCBNO, TIMERNO_QR_STOP_RESP, (res_wait_time * 50) );
								}
							}
							// ���g���C�I�[�o�[�H
							else {
								lcdbm_Counter_QR_StartStopRetry = 0;
								// ���Z�p��
							}
						}
						break;
// MH810100(S) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
					// �ҋ@��ʒʒm
					case LCDBM_OPCD_CHG_IDLE_DISP_NOT:
						OPECTL.chg_idle_disp_rec = 1;
						break;
// MH810100(E) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
// MH810100(S) 2020/06/08 #4205�y�A���]���w�E�����z�Z���^�[�Ƃ̒ʐM�f����QR�Ǎ������Z��������Ɖ�ʂ��ł܂�(No.02-0009)
					case LCDBM_OPCD_QR_NO_RESPONSE:			// 37:QR������̓��ɏ��͗��Ȃ��ʒm
					 	// QR�ް�����M���Ă��Ȃ���Ԃɖ߂�
					 	lcdbm_QRans_InCar_status = 0;
// MH810100(S) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
						if (isEC_USE()) {
							// QR�t���O�N���A
							lcdbm_Flag_QRuse_ICCuse = 0;
							
							// ��t�����M
							EcSendCtrlEnableData();
						}
// MH810100(E) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
						// �����Ő��Z���������𖞂������������҂��Ő��Z�������Ɣ��肵���ꍇ�̂��߂�
						// �Z���^�[�⍇��NG�ł���Γ��������T�u���ĂэĔ�����s��
						// ���������T�u�F�Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
						in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);

						// QR����f�[�^�����o�^
						ope_CancelRegist(&MediaDetail);
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
// GG129000(S) T.Nagai 2023/10/17 #7173 QR�̃Z���^�[�⍇���^�C���A�E�g����Ɛ��Z�����ł��Ȃ��Ȃ�
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							OPECTL.f_rtm_remote_pay_flg = 0;
							if (ryo_buf.nyukin) {
								in_mony(COIN_EVT, 0);			// �����Čv�Z
							}
							if (OPECTL.op_faz < 2) {
								cn_stat( 3, 2 );				// ���� & COIN�ē�����
							}
							lcdbm_Flag_RemotePay_ICCuse = 0;
						}
// GG129000(E) T.Nagai 2023/10/17 #7173 QR�̃Z���^�[�⍇���^�C���A�E�g����Ɛ��Z�����ł��Ȃ��Ȃ�
					 	break;
// MH810100(E) 2020/06/08 #4205�y�A���]���w�E�����z�Z���^�[�Ƃ̒ʐM�f����QR�Ǎ������Z��������Ɖ�ʂ��ł܂�(No.02-0009)
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
					// ���u���Z�i���A���^�C���j�J�n�v��
					case LCDBM_OPCD_RTM_REMOTE_PAY_REQ:
// GG129004(S) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
//						if (OPECTL.f_rtm_remote_pay_flg != 0 ||		// ���u���Z�i���A���^�C���j�J�n��t�ς�
//							OPECTL.op_faz > 1 ||					// ������
//							card_use[USE_N_SVC] > 0 ||				// ���񊄈�����
//							isEC_PAY_CANCEL() ||					// ���ϐ��Z���~��
//							ryo_buf.nyukin > 0 ||					// ������
//							Suica_Rec.Data.BIT.CTRL_MIRYO != 0 ||	// ������
//							Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0){	// �����c���Ɖ�^�C���A�E�g
						if (OPECTL.op_faz > 1 ||					// ������
							card_use[USE_N_SVC] > 0 ||				// ���񊄈�����
							isEC_PAY_CANCEL() ||					// ���ϐ��Z���~��
							ryo_buf.nyukin > 0 ||					// ������
							Suica_Rec.Data.BIT.CTRL_MIRYO != 0 ||	// ������
							Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0){	// �����c���Ɖ�^�C���A�E�g
// GG129004(E) M.Fujikawa 2024/11/13 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
							// ���u���Z�i���A���^�C���j�J�nNG
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 1 );
							break;
						}

						if (isEC_USE()) {
							// ���σ��[�_����

							if (isEC_STS_CARD_PROC()) {
								// ���u���Z�i���A���^�C���j�J�nNG
								lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 1 );
							}
							else {
								// �����֎~
								cn_stat(2, 2);

								if (Op_StopModuleWait(REASON_RTM_REMOTE_PAY)) {
									// ��t�ς�
									OPECTL.f_rtm_remote_pay_flg = 1;
									// ���u���Z�i���A���^�C���j�J�nOK
									lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
									// ���u���Z�J�nOK
									lcdbm_Flag_RemotePay_ICCuse = 2;
								}
								else {
									// IC�ڼޯĒ�~�҂���ϊJ�n		��t�s��M�҂��^�C�}�[(5�b) + 1�b
									Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, ((EC_DISABLE_WAIT_TIME+1) * 50) );
									// ��~�m�F��
									lcdbm_Flag_RemotePay_ICCuse = 1;
								}
							}
						}
						else {
							// ���σ��[�_�Ȃ�

							// ��t�ς�
							OPECTL.f_rtm_remote_pay_flg = 1;
							// �����֎~
							cn_stat(2, 2);
							// ���u���Z�i���A���^�C���j�J�nOK
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
						}
						break;
					// ���u���Z�i���A���^�C���j��t���~
					case LCDBM_OPCD_RTM_REMOTE_PAY_CAN:
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							OPECTL.f_rtm_remote_pay_flg = 0;
// GG129004(S) M.Fujikawa 2024/11/19 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
//							if (ryo_buf.nyukin) {
//								in_mony(COIN_EVT, 0);			// �����Čv�Z
//							}
//							if (OPECTL.op_faz < 2) {
//								cn_stat( 3, 2 );				// ���� & COIN�ē�����
//							}
//							if (lcdbm_Flag_RemotePay_ICCuse == 2) {
//								// ���σ��[�_�Ċ�����
//								Op_StopModuleWait_sub(2);
//							}
//							lcdbm_Flag_RemotePay_ICCuse = 0;
// GG129004(E) M.Fujikawa 2024/11/19 AMS���P�v�] ���u���Z�J�n���Ɏԗ����m�F������
						}
						break;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				}
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// IC�N���W�b�g�⍇����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if( isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ){
				if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					// ���Z�b�g�v���L���t���OON
					lcdbm_Flag_ResetRequest = 1;
					// op_mod03(���Z����)�ڍs����loop�O�� -> op_init00(�N����)�ֈڍs����
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
					OPECTL.init_sts = 0;	// ��������������ԂƂ���
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
				}
				// IC�N���W�b�g��⍇����
				else {
					// ���Z���~�������b�Z�[�W���M
					queset( OPETCBNO, LCD_LCD_DISCONNECT, 0, NULL );
				}
				break;

			// IC�ڼޯ�_��t�s��ELE_EVT_STOP(�d�q�}�̒�~������M)����Ď�M����
			// REASON_QR_USE(QR�ް���M����IC�ڼޯĒ�~�������Ƃ������R)��M�̉ӏ��ŏ������s��

			// IC�ڼޯ�_������(�����J�n)
			// ���Ϗ���ް�(From_ICC)(lcdbm_ICC_Settlement_Status)
			case LCD_ICC_SETTLEMENT_STS:
				// ���ޏ�����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if( lcdbm_ICC_Settlement_Status == EC_SETT_STS_CARD_PROC ){
				if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					lcdbm_Counter_QR_StartStopRetry = 0;
					// ����ʒm(QR��~�v��)���M	0�Œ�
					lcdbm_notice_ope( LCDBM_OPCD_QR_STP_REQ, 0 );
// MH810100(S) S.Nishimoto 2020/07/14 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
					// ����ʒm(�J�[�h���Ϗ�Ԓʒm)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100
//// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
////					lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x01);	// �N���W�b�g�J�[�h������
//					if(isEcBrandNoEMoney(RecvBrandResTbl.no, 0) ){
					if (EcUseKindCheck(convert_brandno((ushort)RecvBrandResTbl.no))) {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100
// MH810105(S) 2021/12/03 iD�Ή�
//						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x02);	// �d�q�}�l�[������
						if(RecvBrandResTbl.no == BRANDNO_ID){
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0b);	// �d�q�}�l�[������(iD)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100
						}else if(RecvBrandResTbl.no ==BRANDNO_QR){
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0b);	// �d�q�}�l�[������(QR��iD�Ɠ���)
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100
						}else{
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x02);	// �d�q�}�l�[������(iD�ȊO)
						}
// MH810105(E) 2021/12/03 iD�Ή�
					}else{
						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x01);	// �N���W�b�g�J�[�h������
					}
// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
// MH810100(E) S.Nishimoto 2020/07/14 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
// MH810100(S) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
					// QR�׸�== ���p�m�F��	(0 = idle, 1 = ���p�m�F��, 2 = ���p��)
					if( lcdbm_Flag_QRuse_ICCuse == 1 ){
						// IC�N���W�b�g��~�҂��^�C�}��~
						Lagcan( OPETCBNO, TIMERNO_ICCREDIT_STOP );
						// QR�ް�����(NG))���M	����(0:OK,1:NG(�r��),2:NG(�������))
						lcdbm_QR_data_res( 1 );		// ����(0:OK,1:NG(�r��),2:NG(�������))
						// QR�t���O�N���A
						lcdbm_Flag_QRuse_ICCuse = 0;
					}
// MH810100(E) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
					// LCDBM_OPCD_QR_STP_REQ(QR��~�v��), LCDBM_OPCD_QR_STA_REQ(QR�J�n�v��)�̉����҂����
					// QRذ�ފJ�n/��~����OK�҂����	48-0021	�ް���M�Ď����	(�⍇��)	�D�E:��M�Ď����(�⍇��) 1�`99=1�b�`99�b
					Lagtim( OPETCBNO, TIMERNO_QR_STOP_RESP, (res_wait_time * 50) );
				}
				break;

			// IC�N���W�b�g���ό��ʃf�[�^(����/���s)
			case LCD_ICC_SETTLEMENT_RSLT:
				// IC�N���W�b�g�ڑ�����H
// MH810103(s) �d�q�}�l�[�Ή�
//				if( prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1 ){
				if(isEC_USE()) {
// MH810103(e) �d�q�}�l�[�Ή�
// MH810100(S) S.Nishimoto 2020/08/18 #4608 �Ԕԃ`�P�b�g���X(�N���W�b�g���ό�A�J�[�h�����^�C���A�E�g���O�ɑҋ@�ɖ߂��Ă��܂�)
//// MH810100(S) S.Nishimoto 2020/07/14 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//					// ����ʒm(�J�[�h���Ϗ�Ԓʒm)
//					lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x00);	// �|�b�v�A�b�v����(����I��)
//// MH810100(E) S.Nishimoto 2020/07/14 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
// MH810100(E) S.Nishimoto 2020/08/18 #4608 �Ԕԃ`�P�b�g���X(�N���W�b�g���ό�A�J�[�h�����^�C���A�E�g���O�ɑҋ@�ɖ߂��Ă��܂�)
// MH810103(s) �d�q�}�l�[�Ή� #5469 E3262(���ϕs��(����w���z�Ǝ���z�ɍ��ق���))�����Ő��Z�������Ă��܂�
					if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR){
						// ��M�������ϊz�Ǝ�M�����I�����i�f�[�^�Ƃō��ق�������
						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=����(�Đ��Z�ē�)
					}else
// MH810103(e) �d�q�}�l�[�Ή� #5469 E3262(���ϕs��(����w���z�Ǝ���z�ɍ��ق���))�����Ő��Z�������Ă��܂�
					// ���ό���OK
// MH810103(s) �d�q�}�l�[�Ή�
//					if( (lcdbm_ICC_Settlement_Result == EPAY_RESULT_OK) ||				// OK
//						(lcdbm_ICC_Settlement_Result == EPAY_RESULT_MIRYO_AFTER_OK) ){	// �����m��㌈��OK	
					if( (lcdbm_ICC_Settlement_Result == EPAY_RESULT_OK)  ){				// OK
// MH810103(e) �d�q�}�l�[�Ή�
// MH810100(S) S.Nishimoto 2020/08/18 #4608 �Ԕԃ`�P�b�g���X(�N���W�b�g���ό�A�J�[�h�����^�C���A�E�g���O�ɑҋ@�ɖ߂��Ă��܂�)
						// ����ʒm(�J�[�h���Ϗ�Ԓʒm)
						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x00);	// �|�b�v�A�b�v����(����OK)
// MH810100(E) S.Nishimoto 2020/08/18 #4608 �Ԕԃ`�P�b�g���X(�N���W�b�g���ό�A�J�[�h�����^�C���A�E�g���O�ɑҋ@�ɖ߂��Ă��܂�)
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
//						lcdbm_pay_rem_chg(1);							// �c���ω��ʒm(���Z����)
//// MH810100(E) S.Takahashi 2020/03/02 �����z���x��ĕ\�������
//						// ����ʒm(���Z�����ʒm(���Z�ς݈ē�))���M
//						// 0=0�~�\��/1=׸���щ������/2=���Z�ς݈ē�
//						lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 2 );
						if ( cmp_send != 3 ) {		// ����̎c���ω��ʒm(���Z����)�����M
							lcdbm_pay_rem_chg(1);	// �c���ω��ʒm(���Z����)
							cmp_send = 3;
						}
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041

// MH810105(S) #6469 �R�C�����b�N�ُ펞�ɁA���Z��������ƃt�F�[�Y���ꂪ�N����
//// MH810100(S) Y.Watanabe 2020/03/25 �Ԕԃ`�P�b�g���X_IC�N���W�b�g���ςƌ��������̓��������̏C��
////						// �ނ�K(�ޑK�z)
////						if( ryo_buf.turisen == 0 ){
////							ret = 1;				// ���Z����(�ނ薳��) 	���Z��������(op_mod03)��
////						}
////						else {
////							ret = 2;				// ���Z����(�ނ�L��) 	���Z��������(op_mod03)��
////						}
						// OnlineDiscount����in_mony�œ������Ƃ����{���Ă��邵�A���σ��[�_������ꍇ��op_faz��8�ɂ��Ē�~�҂����邽�߂����ŏ��������Ă̓_��
//						ac_flg.cycl_fg = 10;				// ������
//						cn_crdt();
//						ac_flg.cycl_fg = 11;				// ���Z����
//						if( OPECTL.op_faz != 2 ){
//							cn_stat( 2, 2 );				// �����s��
//							Lagtim( OPETCBNO, 2, 10*50 );	// ү��Ď�10s
//							OPECTL.op_faz = 2;
//						}
//// MH810100(E) Y.Watanabe 2020/03/25 �Ԕԃ`�P�b�g���X_IC�N���W�b�g���ςƌ��������̓��������̏C��
// MH810105(E) #6469 �R�C�����b�N�ُ펞�ɁA���Z��������ƃt�F�[�Y���ꂪ�N����
					}
					// ���ό���NG
					else {
// MH810100(S) S.Nishimoto 2020/08/18 #4608 �Ԕԃ`�P�b�g���X(�N���W�b�g���ό�A�J�[�h�����^�C���A�E�g���O�ɑҋ@�ɖ߂��Ă��܂�)
						// ����ʒm(�J�[�h���Ϗ�Ԓʒm)
						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x09);	// �|�b�v�A�b�v����(����NG)
// MH810100(E) S.Nishimoto 2020/08/18 #4608 �Ԕԃ`�P�b�g���X(�N���W�b�g���ό�A�J�[�h�����^�C���A�E�g���O�ɑҋ@�ɖ߂��Ă��܂�)
						lcdbm_Counter_QR_StartStopRetry = 0;
// MH810105(S) #6275 ���������c���Ɖ�^�C���A�E�g���QR�R�[�h���[�_�[���ǂݎ���ԂɂȂ�B
//						// ����ʒm(QR�J�n�v��)	���M	0�Œ�
//						lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
//						// LCDBM_OPCD_QR_STP_REQ(QR��~�v��), LCDBM_OPCD_QR_STA_REQ(QR�J�n�v��)�̉����҂����
//						// QRذ�ފJ�n/��~����OK�҂����	48-0021	�ް���M�Ď����	(�⍇��)	�D�E:��M�Ď����(�⍇��) 1�`99=1�b�`99�b
//						Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
						// ���������ꍇ�́AQR���J�n���Ȃ�
						switch(lcdbm_ICC_Settlement_Result){
						case EPAY_RESULT_MIRYO:				// �����m��
						case EPAY_RESULT_MIRYO_ZANDAKA_END:	// �����c���Ɖ��
						case EPAY_RESULT_PAY_MIRYO:			// �x�������i����OK�j
							break;
						default:
							// ����ʒm(QR�J�n�v��)	���M	0�Œ�
							lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
							// LCDBM_OPCD_QR_STP_REQ(QR��~�v��), LCDBM_OPCD_QR_STA_REQ(QR�J�n�v��)�̉����҂����
							// QRذ�ފJ�n/��~����OK�҂����	48-0021	�ް���M�Ď����	(�⍇��)	�D�E:��M�Ď����(�⍇��) 1�`99=1�b�`99�b
							Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
							break;
						}
// MH810105(E) #6275 ���������c���Ɖ�^�C���A�E�g���QR�R�[�h���[�_�[���ǂݎ���ԂɂȂ�B

						// IC�N���W�b�g�⍇�����ɂ��A���Z���~��ۗ����Ă����ꍇ�ɂ����ŗv�����s��
						if( lcdbm_Flag_ResetRequest == 1 ){
							// �ؒf�ɂ�钆�~�v��
							queset( OPETCBNO, LCD_LCD_DISCONNECT, 0, NULL );
						}else if( lcdbm_Flag_ResetRequest == 2 ){
							// �������ʒm�ɂ�钆�~�v��
							queset( OPETCBNO, LCD_LCDBM_OPCD_STA_NOT, 0, NULL );
						}
					}
				}
				// IC�ڼޯĐݒ�Ȃ�
				else {
					// nothing todo
				}
				break;

			// IC�N���W�b�g��~�҂��^�C���A�E�g
			case TIMEOUT_ICCREDIT_STOP:
				// IC�N���W�b�g�ڑ�����
// MH810103(s) �d�q�}�l�[�Ή�
//				if( prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1 ){
				if(isEC_USE()) {
// MH810103(e) �d�q�}�l�[�Ή�
					// IC�N���W�b�g��~�҂��^�C�}��~
					Lagcan( OPETCBNO, TIMERNO_ICCREDIT_STOP );

					// QR�׸�== ���p�m�F��	(0 = idle, 1 = ���p�m�F��, 2 = ���p��)
					if( lcdbm_Flag_QRuse_ICCuse == 1 ){
							// QR�ް�����(NG))���M	����(0:OK,1:NG(�r��),2:NG(�������))
							lcdbm_QR_data_res( 1 );		// ����(0:OK,1:NG(�r��),2:NG(�������))

// MH810100(S) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
						// QR�t���O�N���A
						lcdbm_Flag_QRuse_ICCuse = 0;
// MH810100(E) S.Fujii 2020/08/31 #4778 QR���p�m�F�^�C���A�E�g�C��
					}
				}
				// IC�ڼޯĐݒ�Ȃ�
				else {
					// nothing todo
				}
 				break;

			// QRذ�ފJ�n/��~����OK�҂����
			case TIMEOUT_QR_START_RESP:
			case TIMEOUT_QR_STOP_RESP:
				// QRذ�ފJ�n/��~����OK�҂���ײ����
				lcdbm_Counter_QR_StartStopRetry++;

				// ���g���C�񐔓��H
				if( lcdbm_Counter_QR_StartStopRetry < 3 ){
					if( msg == TIMEOUT_QR_START_RESP ){
						// QR�J�n�v�����M
						lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
						// �����҂��^�C�}�[�X�^�[�g
						Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
					}else{
						// QR��~�v�����M
						lcdbm_notice_ope( LCDBM_OPCD_QR_STP_REQ, 0 );
						// �����҂��^�C�}�[�X�^�[�g
						Lagtim( OPETCBNO, TIMERNO_QR_STOP_RESP, (res_wait_time * 50) );
					}
				}
				// ���g���C�I�[�o�[�H
				else {
					lcdbm_Counter_QR_StartStopRetry = 0;
					// ���Z�p��
				}
				break;
// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
			case TIMEOUT28:
				ryo_buf.in_coin[2] = ++autopay_coin_cnt;
				queset( OPETCBNO, COIN_IN_EVT, 0, NULL );	// ��������
				queset( OPETCBNO, COIN_EN_EVT, 0, NULL );	// ���o�����
				Lagcan(OPETCBNO, 28);
				break;
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z

// MH810100(S) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4607 ���Z���Ɍ��σ��[�_�̃G���[���������Ă����Z��ʂɁu�N���W�b�g���p�s�v���\������Ȃ�)
			case TIMEOUT26:
				if( DspSts == LCD_WMSG_OFF ){						// ܰ�ݸ�ү���ޕ\����Ԃ��n�e�e
					dspCyclicErrMsgRewrite();
				}
				Lagtim( OPETCBNO, 26, 3*50 );						// ���د��\���p��ϰ(3s)
				break;
// MH810100(E) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4607 ���Z���Ɍ��σ��[�_�̃G���[���������Ă����Z��ʂɁu�N���W�b�g���p�s�v���\������Ȃ�)

			default:
				break;
		}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�_�ύX))

		if( ret == 10 ){											// ���(���~)?
// MH321800(S) hosoda IC�N���W�b�g�Ή�
			// �ųݽ���ރt���OOFF�̎�
			if( !announce_end ){
				// ���ލ�����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if( Suica_Rec.Data.BIT.CTRL_CARD ){
				if( isEC_CARD_INSERT() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					// �ųݽ���łȂ����A���ޔ������ųݽON
					if (!announce_flag) {
						// ���������ϰ�N��
						Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );
						announce_flag = 1;		// �ųݽ�t���OON
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//						// �u�J�[�h������艺�����v�\���ƃA�i�E���X������
//						ec_MessageAnaOnOff( 1, 3 );
//						// ���C���[�_�ɉ���������΃V���b�^�[����
//						if( RD_mod < 9 ){
//							rd_shutter();
//						}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
					}
					// �J�[�h�������A�܂��́A�^�C���A�E�g��҂��߁Aret���N���A����
					ret = 0;
					continue;
				}
				// ���ޔ��������
				else {
					ope_anm( AVM_STOP );						// ������~�ųݽ
					// �u�J�[�h������艺�����v�\�������Ƃɖ߂�
					ec_MessageAnaOnOff( 0, 0 );
					announce_end = 1;							// �ųݽ���ރt���OON
					announce_flag = 0;							// �ųݽ�t���OOFF
				}
			}
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
//			if (ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0) {	// WAON���ψُ킪�������Ă���
			if (ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0 ||	// WAON���ψُ킪�������Ă���
				ERR_CHK[mod_ec][ERR_EC_SETTLE_ABORT] != 0) {		// ���Ϗ������s���������Ă���
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
				// ���̓��[�_�[���ɕۗ�����̂Ŕ������̑҂����킹�͂��Ȃ�
				// �����^�C���A�E�g�ɂ��x�Ƃ̏ꍇ�A����߂��Ȃ�
				if (RD_mod == 8) {
					opr_snd(91);		// read���Ȃ��~
				}
			}
// MH810100(S) K.Onodera 2020/2/18 #3870 ������ʂɖ߂�܂Ő��Z�ł��Ȃ��Ȃ��Ă��܂�
			// ���Z���̉������K�v�H
			if( require_cancel_res ){
				// ���Z���~����(OK)
				lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
			}
// MH810100(E) K.Onodera 2020/2/18 #3870 ������ʂɖ߂�܂Ő��Z�ł��Ȃ��Ȃ��Ă��܂�
// MH321800(E) hosoda IC�N���W�b�g�Ή�
			_di();
			CN_escr_timer = 0; 										// ������ϰ��ݾ�
			_ei();
			safecl( 7 );											// ���ɖ����Z�o
// MH810100(S) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(�ҋ@��ԑJ�ڏ������ʉ�)
			if(( OPECTL.Ope_mod == 0 ) && ( OPECTL.init_sts == 1 )){
				// �N�������ʒm��M�őҋ@��ԂɑJ��
				ope_idle_transit_common();	// �ҋ@��ԑJ�ڋ��ʏ���
			}
// MH810100(E) K.Onodera 2020/03/13 �Ԕԃ`�P�b�g���X(�ҋ@��ԑJ�ڏ������ʉ�)
		}
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//		if ( credit_result < 0 )	//���Z�ُ�
//		{
//			creInfoInit();			// �N���W�b�g���N���A
//			memset ( &ryo_buf.credit, 0, sizeof(credit_use));
//			read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
//			opr_snd( 2 );										// ���ޔr�o
//			creMessageAnaOnOff( 1, (short)(credit_result * -1) );
//			// �N���W�b�g�J�[�h�G���[���A�T�C�N���b�N�\���L���Ȃ�ĊJ
//			if (dspIsCyclicMsg()) {		// �T�C�N���b�N����
//				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
//			}
//			credit_result = 0;
//			credit_error_ari = 1;
//			if ( credit_Timeout8Flag == 1 ){
//				in_mony ( KEY_TEN_F4, 0 );
//				credit_result = 999;								// ����
//			}
//			OPECTL.InquiryFlg = 0;		// �O���Ɖ�t���OOFF
//			Suica_Ctrl( S_CNTL_DATA, 0x01 );							// Suica���p��s�ɂ���	
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////			Edy_SndData01();								// Edy���p�����i�ݒ肠��΁j
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( MIFARE_CARD_DoesUse ){						// Mifare���L���ȏꍇ
//				op_MifareStart();							// Mifare�𗘗p�����i�ĊJ�j
//			}
//			if( ryo_buf.nyukin && OPECTL.credit_in_mony ){	// ��������ŸڼޯĂƂ̂���Ⴂ�����̏ꍇ
//				in_mony ( COIN_EVT, 0 );					// ��ݲ���ē���������āA��������(ryo_buf�ر�̍X�V)���s��
//				OPECTL.credit_in_mony = 0;					// ����Ⴂ�׸޸ر
//			}
//		}
//		else if ( credit_result == 1 )	//���Z����
//		{
//			read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
//			opr_snd( 2 );										//���ޔr�o
//			creMessageAnaOnOff( 0, 0 );
//			credit_result = 999;								// ����
//			FurikaeMotoSts = 0;			// �U�֌���ԃt���O 0:�`�F�b�N�s�v
//		}
//		// �N���W�b�g�֘A�C�x���g�҂����ɂ���Ⴂ�œ����������������ݸނ����邽��
//		// 
//		if( ryo_buf.credit.pay_ryo != 0L && ryo_buf.credit.cre_type == CREDIT_CARD){
//			if( credit_result == 999 && ret != 0 ){
//				if (credit_nyukin == 1){					// �N���W�b�g���Z�O�̓�������
//					ret = credit_turi;
//				}
//				else{
//					if(ryo_buf.nyukin != 0){				// �N���W�b�g���Z��̓�������
//						ryo_buf.turisen = ryo_buf.nyukin - ryo_buf.dsp_ryo;
//						ret = 2;
//					}
//					else{
//						ret = credit_turi;
//					}
//				}
//				break;
//			}
//		}else{
//			if( ret != 0 ){
//				// �ڼޯĶ��ޑ}���ς݁����ʂ�����ȊO���ڼޯ�HOST�₢���킹����J�n���d�q�}�̂ł̐��Z���Ȃ�
//				if( RD_Credit_kep && credit_result <= 0 && OPECTL.InquiryFlg && !e_incnt ){
//					opr_snd( 2 );										//���ޔr�o					
//				}
//				break;
//			}
//		}
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
		if ( ((ret == 1) || (ret == 2)) && (r_zero != 0) ) {	// ���Z������0�~�ȊO���Z
			lcdbm_notice_ope(LCDBM_OPCD_PAY_CMP_NOT, 2);	// ���Z�����ʒm
		}
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
		// ���Z����/�Ƃ肯��(1=�ޑK�Ȃ�,2=�ޑK����,3=ү��װ����,10=�Ƃ肯��)
		if (ret != 0) {
			break;
		}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
		if (OPECTL.InquiryFlg == 0) {		// �O���Ɖ�t���OOFF
// MH810100(S) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
//			if (FurikaeMotoSts == 2) {		// �U�֌���ԃt���O 2:�ԂȂ�
//				msg = CANCEL_EVENT;
//				FurikaeMotoSts = 0;
//				FurikaeCancelFlg = 1;
//				goto _MSG_RETRY;	// ����ɃL�����Z���������s���������߁Aqueset�ł͂Ȃ�goto����
//			}
// MH810100(E) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
		}
	}


	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 2 );											// ��ϰ2ؾ��(ү��Ď��p)
	Lagcan( OPETCBNO, 6 );											// ��ϰ6ؾ��(������⍇���Ď���ϰ)
	Lagcan( OPETCBNO, 7 );											// ��ϰ7ؾ��(�ǎ�װ�\���p��ϰ)
	Lagcan( OPETCBNO, 8 );											// ��ϰ8ؾ��(������߂蔻����ϰ)
	Lagcan( OPETCBNO, 10 );											// ���p�\�}�̻��د��\���p��ϰSTOP
	Lagcan( OPETCBNO, 11 );										
	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );				// Suica�֘A��ϰ�ر
	LagCan500ms( LAG500_MIF_WRITING_FAIL_BUZ );						// NG�޻ް
// MH321800(S) G.So IC�N���W�b�g�Ή�
	LagCan500ms(LAG500_EC_START_REMOVAL_TIMER);						// ���ό���NG��M���̶��ޔ������^�C�}�N����ϰؾ��
	Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );						// ec���T�C�N���\���p��ϰؾ��
	Lagcan( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT );					// �J�[�h�������҂���ϰؾ��
// MH810100(S) S.Takahashi 2020/03/18 �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
	Lagcan(OPETCBNO, 28);											//100�~�����^�C�}
#endif	// (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 �����A�����Z

	// ���σ��[�_�֘A�������܂��\������Ă����猳�ɖ߂�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if( ec_MessagePtnNum != 0){
//		if( ec_MessagePtnNum == 98 ){
	if( isEC_MSG_DISP() ){
		if( isEC_NG_MSG_DISP() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			Ope2_WarningDispEnd();
		} else {
			ec_MessageAnaOnOff( 0, 0 );
		}
	}
// MH321800(E) G.So IC�N���W�b�g�Ή�
	if( MifStat != MIF_WROTE_CMPLT) {								// ��~�ς݂łȂ����
		op_MifareStop();
	}
	OPECTL.InquiryFlg = 0;		// �O���Ɖ�t���OOFF

	OPECTL.Fin_mod = ret;
	if( ret != 10 ){
		OPECTL.Ope_mod = 3;											// ���Z����������
// MH810100(S) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//		if( f_al_card_wait == 1 ){		// 1=���C�J�[�h��������Aal_card() ���{�O�ɓd�q�}�l�[���[�_�Ȃǂ̒�~�҂������Ă��鎞�ɐ��Z�����ɂȂ���
//			// �O���r�o���̃R�}���h���M����RD_mod = 10�ƂȂ�A���M���������[�_�[�����M����ƁARD_mod = 11�ƂȂ�B
//			// 10,11���́A���Ŕr�o�v�������Ă���\��������̂ŁA�����ł͖߂��Ȃ��i����΍�j
//			if((RD_mod != 10) && (RD_mod != 11) ){
//				opr_snd( 13 );			// �ۗ��ʒu�ɂ���J�[�h�ł��O���r�o
//				if( pas_kep == (char)-1 ){
//					// ��L�Ŗ߂��̂�pas_kep=2�Ƃ��āAop_mod03�ŃJ�[�h�߂����肵�Ȃ��悤�ɂ���B
//					pas_kep = 2;		// ����ԋp��
//				}
//			}
//		}else{
//			if( pas_kep && ( RD_mod != 10 && RD_mod != 11 )){		// ���Z�������Ă���ɂ��ւ�炸�A������ۗ��ʒu�Ɏc���Ă���ꍇ
//				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
//				opr_snd( 2 );		//�O���r�o
//			}
//		}
// MH810100(E) K.Onodera 2020/02/18 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	}
	else{
		cn_stat( 8, 0 );	// �ۗL�������M
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//		Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );	// ���~���̋��K�Ǘ����O�f�[�^�쐬
//		Log_regist( LOG_MONEYMANAGE_NT );							// ���K�Ǘ����O�o�^
		if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {	// ���~���̋��K�Ǘ����O�f�[�^�쐬
			Log_regist( LOG_MONEYMANAGE_NT );							// ���K�Ǘ����O�o�^
		}
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
		LagCan500ms( LAG500_MIF_LED_ONOFF );							// ICC �޲��LED�_��
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// �s��C��(S) K.Onodera 2016/10/13 #1521 �U�֌��̃t���b�v�����~�����܂܂ɂȂ��Ă��܂�
////		if (ope_imf_GetStatus() == 2) {
//		if( ope_imf_GetStatus() == 2 || PiP_GetFurikaeSts() ){
//// �s��C��(E) K.Onodera 2016/10/13 #1521 �U�֌��̃t���b�v�����~�����܂܂ɂȂ��Ă��܂�
//			ope_Furikae_stop(vl_frs.lockno, 0);			// �U�֌��Ԏ��������~
//			if (FurikaeCancelFlg) {
//			// �Ԃ��Ȃ��Ȃ������Ƃɂ�鐸�Z���~�Ȃ�A�U�֐��Z���s��ʒm
//				ope_imf_Answer(0);			// �U�֐��Z���s
//			}
//		}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
		if(OPECTL.ChkPassSyu)
			blink_end();									// �_�ŏI��
		if(OPECTL.Pay_mod == 2){										// ���Z�̎��(�ʏ퐸�Z)
			OPECTL.Pay_mod = 0;
			wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
			OPECTL.Kakari_Num = 0;										// �W�����ر
		}
			f_MandetMask = 1;
	}
	ope_imf_End();
// MH322914(S) K.Onodera 2016/09/12 AI-V�Ή�
	PiP_FurikaeEnd();
// MH322914(E) K.Onodera 2016/09/12 AI-V�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	Suica_Rec.Data.BIT.ADJUSTOR_START = 0;							// ���Z�J�n�t���O�Z�b�g
	PayDataErrDispClr();

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �c�z�s��ү���ޕ\��                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_SuicaFusoku_Msg			                           |*/
/*| PARAMETER    : dat	�F�\�����z	                                       |*/
/*| 			 : buff	�F�\���ް��i�[�̈�                                 |*/
/*| RETURN VALUE : void				�@                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : suzuki                                                 |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	op_SuicaFusoku_Msg( ulong dat,	uchar *buff, uchar use_kind )
{
	unsigned char	w_buf[5];
	unsigned char	d_buf[10];
	unsigned char	w_Msg[30];
	short			i;
	unsigned long	bb;
	ushort siz = 5;

	if( siz != 0 ) {
		for( bb = 1l,i = 0; i < siz; i++ ) {
			bb = bb * 10l;
		}
	}
	dat = ( dat % bb );
	bb = bb / 10l;
	for( i = 0;i < siz; i++ ) {
		w_buf[i] = (unsigned char)(( dat / bb ) | 0x30);
		dat = ( dat % bb );
		bb = bb / 10l;
	}
	zrschr( w_buf, (uchar)siz );
	as1chg( w_buf, d_buf, (uchar)siz );
	if( use_kind == 0 )
		memcpy(&w_Msg[0], "  �c�z�s���i�c�z", 16);
	else
		memcpy(&w_Msg[0], "  �c���s���i�c��", 16);
	memcpy(&w_Msg[16], d_buf, 10);
	memcpy(&w_Msg[26], "�~�j", 4);
	memcpy(buff, w_Msg, sizeof(w_Msg));
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	// �|�b�v�A�b�v�\���v��(�c���s��)
// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//	lcdbm_notice_dsp( POP_SHORT_BALANCE, 0 );
	lcdbm_notice_dsp2( POP_INQUIRY_NG, 6, dat );
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| 6�E7�s�ڂɕ\�������װя���                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_SuicaFusokuOff			                           |*/
/*| PARAMETER    : void				                                       |*/
/*| RETURN VALUE : void				�@                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : suzuki                                                 |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	op_SuicaFusokuOff( void )
{
	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );	// ��ϰ�ر
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( edy_dsp.BIT.edy_Pay_Retry || edy_dsp.BIT.edy_Pay_Retry_Sub ){ // �c�z�s����ʕ\�����i������)������׸ނ���čς݁H
//		if( !PayData.Electron_data.Edy.pay_ryo )		// Edy������
//			Edy_StopAndStart();							// ���ތ��m��~���J�n�w�����M
//		edy_dsp.BIT.edy_Pay_Retry_Sub = 0;				// �c�z��������ʕ\������׸ނ�ر
//	}
//	if( edy_dsp.BIT.edy_dsp_change || edy_dsp.BIT.edy_dsp_Warning ){// �װ�ү���ނ��\�������܂܂̏ꍇ
//		edy_dsp.BIT.edy_dsp_change = 0;					// �װ�ү�����׸ނ�������
//		edy_dsp.BIT.edy_dsp_Warning = 0;				// �װ�ү�����׸ނ�������
//		if(!edy_dsp.BIT.edy_Miryo_Loss){				// Edy���Z���s��ѱ�Č�̱װѕ\�����H
//			if( PayData.Electron_data.Edy.pay_ryo != EDY_USED && OPECTL.op_faz != 8 && // 07-01-22�ύX //
//				(OPECTL.op_faz != 3 || OPECTL.op_faz != 4 ))	// �������ѱ�Ď��Ͷ��ތ��m�J�n�w�����o���Ȃ� 07-01-22�ǉ� //
//				Edy_StopAndStart();						// ���ތ��m��~���J�n�w�����M
//		}
//		if( OPECTL.Ope_mod == 0 && !Edy_Rec.edy_status.BIT.ZAN_SW )	// ���Ԍ��}���҂��H
//			Lagtim( OPETCBNO, 11, 2 );					// �c���Ɖ��ʐؑ���ϰ���Ľ���
//		else if( (OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 13 || OPECTL.Ope_mod == 22 ) && OPECTL.op_faz != 8 )
//			Lagtim( OPETCBNO, 10, 2 );					// ���د��\����ϰ���Ľ���
//	}
//	else if( edy_dsp.BIT.edy_Miryo_Loss ){				// Edy���Z���s��ѱ�Č�̱װѕ\�����H
//		edy_dsp.BIT.edy_Miryo_Loss = 0;					// Edy���Z���s��ѱ�Č�̱װѕ\���׸޸ر
//	}
//	else if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){		// Suica���Z���s��ѱ�Č�̱װѕ\�����H
	if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){			// Suica���Z���s��ѱ�Č�̱װѕ\�����H
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) hosoda IC�N���W�b�g�Ή�
		Ope2_ErrChrCyclicDispStop();					// ���[�j���O �T�C�N���b�N�\����~
// MH321800(E) hosoda IC�N���W�b�g�Ή�
		Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;			// �װѕ\���׸޸ر
	}
	Lcd_WmsgDisp_OFF2();
	Lcd_WmsgDisp_OFF();
	dsp_fusoku = 0;
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//// GG116200(S) D.Inaba IC�N���W�b�g�Ή�
//	ec_MessagePtnNum = 0;								// LCD�\���p�^�[���N���A
//// GG116200(E) D.Inaba IC�N���W�b�g�Ή�
	ECCTL.ec_MessagePtnNum = 0;							// LCD�\���p�^�[���N���A
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
}

/*[]----------------------------------------------------------------------[]*/
/*| �ۗ����̲���č�Post�֐�                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nyukin_delay_check			                           |*/
/*| PARAMETER    : 					                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2006-10-17                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	nyukin_delay_check( ushort *postdata, uchar count )
{
	uchar i;
	for( i=0; i<count; i++ ){
		if( postdata[i] ){
			queset( OPETCBNO, postdata[i], 0, NULL );		// �ۗ����̎�������Ă�ʒm
		}
	}
	memset( nyukin_delay,0,sizeof( nyukin_delay ));			// ����Ĵر�̏�����
	delay_count = 0;										// ���Đ��̏�����
}

/*[]----------------------------------------------------------------------[]*/
/*| �ҋ@���A�p�ŏI��ϰ                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_return_last_timer			                           |*/
/*| PARAMETER    : 					                                       |*/
/*| ���l         : �ҋ@��ʂɑJ�ڂ����邽�߂̍ŏI��ϰ�Ŗ{�֐�����ق��ꂽ�@ |*/
/*| 			 : �ꍇ��Suica�̏�ԂɊւ�炸�ҋ@��ʂɑJ�ڂ���        �@ |*/
/*| 			 : �ҋ@�ɖ߂邽�߂�ۼޯ��Ƃ��Ă�TIMEOUT2���g�p����         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2006-10-11                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	Op_ReturnLastTimer( void )
{
	Suica_Rec.Data.BIT.CTRL = 0;			// �ҋ@�p��Suica��s��Ԃɐݒ肵�Ă���
	Lagtim( OPETCBNO, 2, 25 );				// ��ϰ2���(���ڰ��ݐ���p)(500ms)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ){	// Edy�ݒ肪�����
//		Op_Cansel_Wait_sub( 1 );			// Edy�I���ς��׸ނ�Ă���
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
}

/*[]----------------------------------------------------------------------[]*/
/*| �����������                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : in_mony( msg )                                          |*/
/*| PARAMETER    : msg    : ү����                                         |*/
/*|                paymod :  0 = �ʏ�, 1 = �C��                            |*/
/*| RETURN VALUE : ret    :  0 = �p��                                      |*/
/*|                       :  1 = ���Z����(�ނ薳��)                        |*/
/*|                       :  2 = ���Z����(�ނ�L��)                        |*/
/*|                       : 10 = ���Z���~(�ҋ@�֖߂�)                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	in_mony( ushort msg, ushort paymod )
{
	short	ret = 0;
	char	w_op_faz = OPECTL.op_faz;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
	ushort	turi_wk = 0;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

	switch( msg ){
		case KEY_TEN_F4:											// �ݷ�[���] �����
			if( OPECTL.op_faz == 2 ){
				break;
			}
			if( OPECTL.CAN_SW == 0 ){								// �ē��͂łȂ�
				OPECTL.CAN_SW = 1;
				cn_stat( 2, 2 );									// �����s��
				Lagtim( OPETCBNO, 2, 10*50 );						// ү��Ď�10s
				OPECTL.op_faz = 3;									// ���~��
				OPE_red = 5;										// ���Z���~
				if( ryo_buf.tei_ryo ){								// ����g�p(������g�p��Ɏc�z�L��)?
					PassIdBackupTim = 0;							// ���n����������������
				}
			}
			break;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		case LCD_LCDBM_OPCD_PAY_STP:			// ���Z���~�v����M��
		case LCD_LCDBM_OPCD_STA_NOT:			// �N���ʒm��M��
		case LCD_LCD_DISCONNECT:				// ���Z�b�g�ʒm/�ؒf���m
			// invcrd( paymod )��ɐ��Z�����Ȃ� ---> op_faz <- 2�Ő��Z����
			if( OPECTL.op_faz == 2 ){
				break;
			}
			// ���Z������ �� ���Z��������
			else {
// MH810100(S) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�S�z���o����ɗa��؂����s�����s��C��)
				if( OPECTL.op_faz != 3 ){	// ���Z���~���ł͂Ȃ�
// MH810100(E) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�S�z���o����ɗa��؂����s�����s��C��)
					cn_stat( 2, 2 );								// �����s��
					Lagtim( OPETCBNO, 2, 10*50 );					// ү��Ď�10s
					OPECTL.op_faz = 3;								// ���~��
					OPE_red = 5;									// ���Z���~
// MH810100(S) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�S�z���o����ɗa��؂����s�����s��C��)
				}
// MH810100(E) K.Onodera  2020/02/25 #3858 �Ԕԃ`�P�b�g���X(�S�z���o����ɗa��؂����s�����s��C��)
			}
			break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

		case NOTE_EVT:												// Note Reader event
			if( OPECTL.NT_QSIG == 1 ){
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
				if(isEC_USE()) {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//					if( !Suica_Rec.Data.BIT.PAY_CTRL &&							// ���ό��ʃf�[�^����M���Ă��炸
//						Suica_Rec.Data.BIT.CTRL &&								// ���σ��[�_�L����
//						in_credit_check() ){									// �c�z������ꍇ
//						// �����ɂ���t�֎~�̏ꍇ�͋��z�ύX�r�b�g���Z�b�g����
//						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// ��t�֎~���M(���z�ύX)
//					}
					// ��t�֎~(���z�ύX)�̓u�����h�I���f�[�^�̕ύX����ɍX�V���ryo_buf.nyukin���g�p���邽��cn_crdt()��Ɉړ�
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
				} else
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
				if( !Suica_Rec.Data.BIT.PAY_CTRL && !ryo_buf.nyukin &&	// ���Z�J�n��ŏ��̌����������͎�t�s�𑗐M����
					Suica_Rec.Data.BIT.CTRL && in_credit_check() ){		// Suica�L���̎��Ŏc�z������ꍇ
					Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
				}					
				ac_flg.cycl_fg = 10;								// ������
				cn_crdt();
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
				// ���σ��[�_���z�ύX
				if( isEC_USE()						&&	// ���σ��[�_��
				    !Suica_Rec.Data.BIT.PAY_CTRL	&&	// ���ό��ʃf�[�^����M���Ă��炸		// �����Ӄ��r���[��
				    Suica_Rec.Data.BIT.CTRL   		&&	// ���σ��[�_�L����
					(ryo_buf.nyukin < ryo_buf.dsp_ryo)) {	// �c�z����
				    if (EcCheckBrandSelectTable(TRUE)){			// �u�����h�I���f�[�^�ɕύX������(�u�����h�I�𒆂łȂ��ꍇ�͏��TRUE)
						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// ��t�֎~���M(���z�ύX)
					}
				}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH321800(S) hosoda IC�N���W�b�g�Ή�
				if ((Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)
				||	(Suica_Rec.Data.BIT.CTRL_MIRYO != 0)) {
				// ���������`�m��܂ł̓����͏������Ȃ�(OPECTL.op_faz:2�`3�̊ԂȂ̂�)
				// ������L�[�����ŕԋ�����
					break;											// �����m���?
				}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z��t�Ƃ̂���Ⴂ�������͓��ɏ�����M����܂œ����������Ȃ�
					// ���ɏ���M��ɍČv�Z���邽��
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if( OPECTL.op_faz == 3 ){
					break;											// ���~?
				}
				else if( OPECTL.op_faz == 4 ){
					OPECTL.op_faz = 3;								// ���Z���~�������s���B
					break;
				}
				if( invcrd( paymod ) != 0 ){						// ���Z����?
					ac_flg.cycl_fg = 11;							// ���Z����
					if( OPECTL.op_faz != 2 ){						//
						svs_renzoku();								// ���޽���A���}��
						cn_stat( 2, 2 );							// �����s��
						Lagtim( OPETCBNO, 2, 10*50 ); 				// ү��Ď�10s
						OPECTL.op_faz = 2;
					}
				}
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
				else{												// ���Z������
					// ryo_buf.zankin��invcrd()�ōX�V�����̂�
					// ��t�����M�۔���͂����ōs��
					if( isEC_USE() ){								// ���σ��[�_�ڑ��ݒ肠��
						if(( !Suica_Rec.Data.BIT.PAY_CTRL ) &&		// ���ό��ʃf�[�^����M���Ă��炸
						   ( !Suica_Rec.Data.BIT.CTRL )){			// ���σ��[�_����
							EcSendCtrlEnableData();					// ��t�����M
						}
					}
				}
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
			}
			if( OPECTL.NT_QSIG == 5 ){								// �㑱��������?
				cn_stat( 1, 1 );									// ��������
			}
			break;

		case COIN_EVT:												// Coin Mech event
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//			if(( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 )){	// �����
			if(( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 )||	// �����
			   ( OPECTL.credit_in_mony != 0 )){
				if(isEC_USE()) {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//					if( !Suica_Rec.Data.BIT.PAY_CTRL &&							// ���ό��ʃf�[�^����M���Ă��炸
//						Suica_Rec.Data.BIT.CTRL &&								// ���σ��[�_�L����
//						in_credit_check() ){									// �c�z������ꍇ
//						// �����ɂ���t�֎~�̏ꍇ�͋��z�ύX�r�b�g���Z�b�g����
//						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// ��t�֎~���M(���z�ύX)
//					}
					// ��t�֎~(���z�ύX)�̓u�����h�I���f�[�^�̕ύX����ɍX�V���ryo_buf.nyukin���g�p���邽��cn_crdt()��Ɉړ�
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
				} else
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
				if( !Suica_Rec.Data.BIT.PAY_CTRL && !ryo_buf.nyukin &&	// ���Z�J�n��ŏ��̌����������͎�t�s�𑗐M����
					Suica_Rec.Data.BIT.CTRL && in_credit_check() ){		// Suica�L���̎��Ŏc�z������ꍇ
					Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
				}					
				ac_flg.cycl_fg = 10;								// ������
				cn_crdt();
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
				// ���σ��[�_���z�ύX
				if( isEC_USE()						&&	// ���σ��[�_��
				    !Suica_Rec.Data.BIT.PAY_CTRL	&&	// ���ό��ʃf�[�^����M���Ă��炸		// �����Ӄ��r���[��
				    Suica_Rec.Data.BIT.CTRL   		&&	// ���σ��[�_�L����
					(ryo_buf.nyukin < ryo_buf.dsp_ryo)) {	// �c�z����
				    if (EcCheckBrandSelectTable(TRUE)){			// �u�����h�I���f�[�^�ɕύX������(�u�����h�I�𒆂łȂ��ꍇ�͏��TRUE)
						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// ��t�֎~���M(���z�ύX)
					}
				}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH321800(S) hosoda IC�N���W�b�g�Ή�
				if ((Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)
				||	(Suica_Rec.Data.BIT.CTRL_MIRYO != 0)) {
				// ���������`�m��܂ł̓����͏������Ȃ�(OPECTL.op_faz:2�`3�̊ԂȂ̂�)
				// ������L�[�����ŕԋ�����
// MH810105 MH321800(S) ���������Ɠ����ɃJ�[�h�^�b�`���ă^�b�`�҂����݂̂Ȃ����ώ�M�ŉ�ʂ����b�N����
//					break;											// �����m���?
					if( OPECTL.credit_in_mony == 0 ){
					// ���������Ƌ����ł���ꍇ�͓������������邽��break���Ȃ�
						break;										// �����m���?
					}
// MH810105 MH321800(E) ���������Ɠ����ɃJ�[�h�^�b�`���ă^�b�`�҂����݂̂Ȃ����ώ�M�ŉ�ʂ����b�N����
				}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810104 GG119201(S) �u�J�[�h�������ł��v�\�����������
				if ((OPECTL.credit_in_mony == 1) && isEC_STS_CARD_PROC()) {
					// �J�[�h�������Ƃ̂���Ⴂ�������͌��ό��ʎ�M����܂œ����������Ȃ�
					// ���ό��ʎ�M��ɍČv�Z���邽��
					break;
				}
// MH810104 GG119201(E) �u�J�[�h�������ł��v�\�����������
				if( OPECTL.op_faz == 3 ){
					break;											// ���~?
				}
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// ���u���Z��t�Ƃ̂���Ⴂ�������͓��ɏ�����M����܂œ����������Ȃ�
					// ���ɏ���M��ɍČv�Z���邽��
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
				if( invcrd( paymod ) != 0 ){						// ���Z�I��?
					ac_flg.cycl_fg = 11;							// ���Z����
					if( OPECTL.op_faz != 2 ){						//
						svs_renzoku();								// ���޽���A���}��
						cn_stat( 2, 2 );							// �����s��
						Lagtim( OPETCBNO, 2, 10*50 );				// ү��Ď�10s
						OPECTL.op_faz = 2;							//
					}
				}else if( OPECTL.CN_QSIG == 5 ){ 					// Coin MAX ? NJ
					alm_chk( ALMMDL_SUB, ALARM_CNM_IN_AMOUNT_MAX, 2 );
					cn_stat( 5, 0 ); 								// Continue
				}
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
				if( isEC_USE() ){									// ���σ��[�_�ڑ��ݒ肠��
					// ryo_buf.zankin��invcrd()�ōX�V�����̂�
					// ��t�����M�۔���͂����ōs��
					if(( OPECTL.op_faz < 2 ) &&						// ���Z������
					   ( !Suica_Rec.Data.BIT.PAY_CTRL ) &&			// ���ό��ʃf�[�^����M���Ă��炸
					   ( !Suica_Rec.Data.BIT.CTRL )){				// ���σ��[�_����
						EcSendCtrlEnableData();						// ��t�����M
					}
				}
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
			}
			else if( OPECTL.CN_QSIG == 7 ){							// ���o��(�����s���)
				Lagcan( OPETCBNO, 2 );
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if (Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC) {	// �J�[�h�������H
				if (isEC_STS_CARD_PROC()) {							// �J�[�h�������H
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					OPECTL.holdPayoutForEcStop = 1;					// ���o�C�x���g��ۗ�����
					break;
				}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
				switch( OPECTL.op_faz ){
					case 3:											// ���~
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						if (isEC_USE() &&
//							Suica_Rec.Data.BIT.CTRL &&				// ��t��
//							!Suica_Rec.suica_err_event.BYTE) {		// �G���[�Ȃ�
						if (isEC_CTRL_ENABLE()) {					// ���σ��[�_����t��
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							OPECTL.holdPayoutForEcStop = 1;			// ���o�C�x���g��ۗ�����
							break;
						}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
						if( ryo_buf.nyukin ){						// �����
							ac_flg.cycl_fg = 21;					// ���Z���~
							if( refund( modoshi() ) == 0 ){
								/*** ��ݕ��o�� ***/
								LedReq( CN_TRAYLED, LED_ON );		// ��ݎ�o�����޲��LED�_��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//								Lagtim( OPETCBNO, 2, (ushort)((60*12)*50) );	// ү��Ď�12min
								// ���o���z����ү��Ď����Ԃ��Z�o�i�ő�12min�j
								if(((ryo_buf.nyukin / 10) + 5) > 720) {
									turi_wk = 720;
								} else {
									turi_wk = (ushort)((ryo_buf.nyukin / 10) + 5);
								}
								Lagtim( OPETCBNO, 2, (ushort)(turi_wk*50) );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
							}else{
								Lagtim( OPETCBNO, 2, 10*50 );		// ү��Ď�10s
							}
							svs_renzoku();							// ���޽���A���}��
						}else{
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
//							if( carduse() ){						// ���ގg�p?
							if( carduse() ||						// ���ގg�p?
								isEC_PAY_CANCEL() ){				// ���σ��[�_�̌��ϐ��Z���~�H
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
								chu_isu();							// ���~
								svs_renzoku();						// ���޽���A���}��
							}
// MH810100(S) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
							else{
								chu_isu();							// ���~
							}
// MH810100(E) 2020/07/20 �Ԕԃ`�P�b�g���X(�d�l�ύX #4541 RXM�ŃZ���^�[�⍇����ɁANG�Ƃ���QR�f�[�^�̎���f�[�^�iDC-NET�j���M)
							Lagtim( OPETCBNO, 1, 1 );				// T1out����ҋ@�ֈڍs����
						}
						break;
					case 2:											// ����
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						if (isEC_USE() &&
//							Suica_Rec.Data.BIT.CTRL &&				// ��t��
//							!Suica_Rec.suica_err_event.BYTE) {		// �G���[�Ȃ�
						if (isEC_CTRL_ENABLE()) {					// ���σ��[�_����t��
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							OPECTL.holdPayoutForEcStop = 1;			// ���o�C�x���g��ۗ�����
							break;
						}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
						ret = invcrd( paymod );						// ���Z��������
						Op_StopModuleWait_sub(3);					// ���Z�����i���Z���j�b�g��~�����j���� 
						ret = 0;									// "ELE_EVT_STOP"���[���Œʒm����
						break;
					case 8:												// �d�q�}�̒�~�������H
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						if (isEC_USE() &&
//							Suica_Rec.Data.BIT.CTRL &&				// ��t��
//							!Suica_Rec.suica_err_event.BYTE) {		// �G���[�Ȃ�
						if (isEC_CTRL_ENABLE()) {					// ���σ��[�_����t��
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
							OPECTL.holdPayoutForEcStop = 1;			// ���o�C�x���g��ۗ�����
							break;
						}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
						if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// ���Z�������̓d�q���ϏI���҂����H
							Op_StopModuleWait_sub(3);					// ���ү���~�������{
							ret = 0;
						}
						break;
					default:
						break;
				}
			}
			else if( OPECTL.CN_QSIG == 2 ){ 						// ���o����
				Lagcan( OPETCBNO, 2 );								// ү��Ď���ϰ��ݾ�
				Op_Cansel_Wait_sub( 2 );
				if(( SFV_DAT.reffal )||( ryo_buf.fusoku )){			//
					ryo_buf.fusoku += SFV_DAT.reffal;				// �x�����s���z
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					ret = 0;
				}
				ac_flg.cycl_fg = 23;								// ���Z���~�����o������
				chu_isu();											// ���~�W�v
			}
			else if( OPECTL.CN_QSIG == 9 ){							// ���o�װ
				if( OPECTL.op_faz == 3 ){							// ���~?
					ac_flg.cycl_fg = 23;							// ���Z���~�����o������
					refalt();	 									// �s�����Z�o
					ryo_buf.fusoku += SFV_DAT.reffal;				// �x�����s���z
					chu_isu();										// ���~�W�v
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					set_escrowtimer();
				}
			}
			break;

		case ARC_CR_R_EVT:											// ����IN
		case IBK_MIF_A2_OK_EVT:										// Mifare�ް��Ǐo������
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//		case CRE_EVT_04_OK:											// �N���W�b�g�^�M�⍇�����ʂn�j
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH321800(S) hosoda IC�N���W�b�g�Ή�
		case EC_EVT_CRE_PAID:										// ���σ��[�_�ł̃N���W�b�g����OK
// MH321800(E) hosoda IC�N���W�b�g�Ή�
			if( paymod == 0 ){
			}else{
				if( ryo_buf.dsp_ryo > ryo_buf.mis_ryo ){
					ryo_buf.dsp_ryo -= ryo_buf.mis_ryo;
				}else{
					ryo_buf.dsp_ryo = 0;
				}
			}
			if( invcrd( paymod ) != 0 ){							// ���Z�I��? NJ
				if( paymod ){										// �C�����Z?
					ac_flg.cycl_fg = 71;							// �C�����Z����
				}else{
					ac_flg.cycl_fg = 11;							// ���Z����
				}
				if( OPECTL.op_faz != 2 ){
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//					if( msg == CRE_EVT_04_OK ) {					// �N���W�b�g���Z�i�^�M�₢����OK�j
//						cre_uriage.UmuFlag	= ON;					// ����˗��f�[�^���M�׸�ON�i06��M��OFF�j
//					}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
					if(msg == EC_EVT_CRE_PAID) {
						if( OPECTL.op_faz == 8 && 
							CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ ){	// STOP_REASON���J�[�h�}���̏ꍇ�Ͷ��ނ̓f���߂����s��
							opr_snd( 13 );							// �O�r�o(�v�����^�u���b�N����ł��f���o��)
						}
						Op_StopModuleWait_sub(4);					// ���σ��[�_���猋�ʂ�������Ă��邽�ߒ�~�ς݂ɂ���
						OPECTL.credit_in_mony = 0;
					}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
					OPE_red = 4;									// ���Z����
					cn_stat( 2, 2 );								// �����s��
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					OPECTL.op_faz = 2;
					if( msg == ARC_CR_R_EVT ){						// ����IN
						if( MAGred[MAG_ID_CODE] == 0x2d ){					// ���޽��? NJ
							rd_shutter();							// ���Cذ�ް���������
						}
						if( (msg == ARC_CR_R_EVT) &&					// �J�[�h�C�x���g�̂Ƃ�
							(MAGred[MAG_ID_CODE] == 0x0e) ||						// ����߲�޶���
							(MAGred[MAG_ID_CODE] == 0x2c) ){						// �񐔌�
							OPECTL.PriUsed = 1;
						}
					}
				}
			}else{
				if( msg == ARC_CR_R_EVT && MAGred[MAG_ID_CODE] == 0x0e ){		// ����IN & ����߲�޶���? NJ
					OPECTL.PriUsed = 2;
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[68] );				// "        �i�c�z            �~�j"
					opedpl( 7, 16, OPECTL.LastUsePCardBal, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
				}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// �u�����h���ɕω����������ꍇ�̓u�����h�I�����s��
// ���σ��[�_���z�ύX
				if( isEC_USE()	&&						// ���σ��[�_��
					Suica_Rec.Data.BIT.CTRL ) {  		// ��t�i���[�_�[��~���Ă��Ȃ��Ƃ��j
				    if (EcCheckBrandSelectTable(FALSE)){			// �u�����h�I���f�[�^�ɕύX������(�u�����h�I�𒆂łȂ��ꍇ�͏��TRUE)
						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// ��t�֎~���M(���z�ύX)
						break;
				    }
				}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
				Op_StopModuleWait_sub( 2 );
			}
			break;
		case SUICA_EVT:													// Suica(Sx-10)����̎�M�ް�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case EC_EVT_QR_PAID:											// ���σ��[�_�ł�QR�R�[�h����OK
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			if( PayData.Electron_data.Suica.pay_ryo ) {					// Suica���ό����ް���M 
				if( paymod != 0 ){											// �C�����Z����
					if( ryo_buf.dsp_ryo > ryo_buf.mis_ryo ){				// ������ł���΁i�s������j
						ryo_buf.dsp_ryo -= ryo_buf.mis_ryo;					// ��Ŏx��������������
					}else{													// ��<or=��ł���΁i�s���Ȃ��j
						ryo_buf.dsp_ryo = 0;								// 0��set
					}
				}
				if( invcrd( paymod ) != 0 ){								// ���Z�I��? NJ
					if( OPECTL.op_faz == 8 && 
						CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ ){	// STOP_REASON���J�[�h�}���̏ꍇ�Ͷ��ނ̓f���߂����s��
							opr_snd( 13 );											// �O�r�o(�v�����^�u���b�N����ł��f���o��)
					}
					Op_StopModuleWait_sub(4);									
// MH810103 GG119202(S) ���������Ɠ����Ɍ�ʌn�̃J�[�h��������M�����Ƃ��A���Z������A�������߂�Ȃ��B
					if (isEC_USE() ){
						OPECTL.credit_in_mony = 0;
					}
// MH810103 GG119202(E) ���������Ɠ����Ɍ�ʌn�̃J�[�h��������M�����Ƃ��A���Z������A�������߂�Ȃ��B
					ac_flg.cycl_fg = 11;									// ���Z����
					OPE_red = 4;											// ���Z����
					if( OPECTL.PriUsed != 1 && RD_Credit_kep )
					svs_renzoku();											// ���޽���A���}��
					cn_stat( 2, 2 );										// �����s��
					Lagtim( OPETCBNO, 1, 10*50 );							// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					OPECTL.op_faz = 2;										// ���Z����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					Edy_StopAndLedOff();							// ���ތ��m��~��UI LED�����w�����M
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				}
				else{
					if( OPECTL.Ope_mod != 13 ){								// �C�����Z���͎c���\���͂��Ȃ�
						grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[68] );						// "      �i�c�z            �~�j  "
						opedpl( 7, 14, (ulong)e_zandaka, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
					}
				}
				Suica_Rec.Data.BIT.PAY_CTRL = 1;
				if( OPECTL.op_faz == 9 ){								// ���Z�Ǝ��������݂̓��������Ŏ��������݂��ۗ����̏ꍇ
					OPECTL.op_faz = 1;									// ����̪��ނɖ߂��Ă���
				}
			}
			break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			case IBK_EDY_RCV:											// Edy�i���ό��ʁF�����ް���M�̂݌ďo�����j
//				if(Edy_Rec.rcv_kind == R_SUBTRACTION) {	// Suica���ό����ް���M 
//					if( paymod != 0 ){											// �C�����Z����
//						if( ryo_buf.dsp_ryo > ryo_buf.mis_ryo ){				// ������ł���΁i�s������j
//							ryo_buf.dsp_ryo -= ryo_buf.mis_ryo;					// ��Ŏx��������������
//						}else{													// ��<or=��ł���΁i�s���Ȃ��j
//							ryo_buf.dsp_ryo = 0;								// 0��set
//						}
//					}
//					if( invcrd( paymod ) != 0 ){								// ���Z�I��? NJ
//						if( OPECTL.op_faz == 8 && 
//							CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ ){	// STOP_REASON���J�[�h�}���̏ꍇ�Ͷ��ނ̓f���߂����s��
//								opr_snd( 13 );											// �O�r�o
//						}
//						Op_StopModuleWait_sub(5);								// Edy���ώ�(���ό�͕K��Edy����~���)
//						ac_flg.cycl_fg = 11;									// ���Z����
//						OPE_red = 4;											// ���Z����
//						if( OPECTL.PriUsed != 1 && RD_Credit_kep )
//						svs_renzoku();											// ���޽���A���}��
//						cn_stat( 2, 2 );										// �����s��
//						Lagtim( OPETCBNO, 1, 10*50 );							// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//						OPECTL.op_faz = 2;										// ���Z����
//					}
//					Edy_SndData04();									// EM�ð���Ǐo���̂��߂ɐ�s���Ķ��ތ��m�I������ޑ��M
//					Edy_SndData11();									// EM�ð���Ǐo���w�����M
//					if( !Ex_portFlag[EXPORT_R_PEND] && OPECTL.RECI_SW == 0 && prm_get(COM_PRM, S_SCA, 54, 1, 1) == 1 )
//						OPECTL.RECI_SW = 1;								// ���Ȃ��i�����E�蓮�j���́Aڼ�Ĉ�
//				}
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		case	IFMPAY_GENGAKU:
		case	IFMPAY_FURIKAE:
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
		case OPE_REQ_REMOTE_CALC_FEE:
		case OPE_REQ_REMOTE_CALC_TIME:
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�_for_LCD_IN_CAR_INFO_op_mod00))
		case OPE_REQ_LCD_CALC_IN_TIME:			// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�_for_LCD_IN_CAR_INFO_op_mod00))

// MH322914(S) K.Onodera 2016/09/05 AI-V�Ή��F�U�֐��Z
		case OPE_REQ_FURIKAE_GO:
// MH322914(E) K.Onodera 2016/09/05 AI-V�Ή��F�U�֐��Z
			if( invcrd( paymod ) != 0 ){						// ���Z����?
				ac_flg.cycl_fg = 11;							// ���Z����
				if( OPECTL.op_faz != 2 ){
					svs_renzoku();								// ���޽���A���}��
					cn_stat( 2, 2 );							// �����s��
					Lagtim( OPETCBNO, 2, 10*50 ); 				// ү��Ď�10s
					OPECTL.op_faz = 2;
				}
			}
			else{
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
				// ���σ��[�_�͐���f�[�^(���p�s��)�𑗐M����
				if( isEC_USE() ){
// MH810100(S) ���u���Z�ŃN���W�b�g���Z���ł��Ȃ�
                    // �I�����i�f�[�^���M�ς݂ł���΁A��t�֎~�𑗐M����
                    if (Suica_Rec.Data.BIT.ADJUSTOR_NOW) {
// MH810100(E) ���u���Z�ŃN���W�b�g���Z���ł��Ȃ�
						Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );
// MH810100(S) ���u���Z�ŃN���W�b�g���Z���ł��Ȃ�
                    }
// MH810100(E) ���u���Z�ŃN���W�b�g���Z���ł��Ȃ�
				} else {
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
				if( !ryo_buf.nyukin ){
					Suica_Ctrl( S_SELECT_DATA, 0 );							// ���ԗ��������i�I���ް��Ƃ��đ��M����
				}
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
				}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
			}
			break;
		default:
			break;
	}

	if( OPECTL.op_faz == 2 && (OpeImfStatus != OPEIMF_STS_GGK_DONE && OpeImfStatus != OPEIMF_STS_FRK_DONE ) ){
		switch(ope_imf_GetStatus()) {
		case	1:
			ope_imf_Answer(1);			// ���z���Z����
			break;
		case	2:
			if( is_paid_remote(&PayData) < 0 ){
			// �U�֐��Z�����{�i������p�w�����ꂽ���������Z�j�Ȃ炱���ŐU�֊z�̂݌v��
				ope_ifm_FurikaeCalc( 0 );
			}
			ope_Furikae_stop(vl_frs.lockno, 1);			// �U�֌��Ԏ��������s
			ope_imf_Answer(1);			// �U�֐��Z����
			break;
		default:
			break;
		}
	}
// MH322914(S) K.Onodera 2016/09/05 AI-V�Ή��F�U�֐��Z
	if( OPECTL.op_faz == 2 && PiP_GetFurikaeSts() ){
		if (is_ParkingWebFurikae(&PayData) < 0) {
			ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
			vl_now = V_FRK;						// �U�֐��Z
			ryo_cal( 3, OPECTL.Pr_LokNo );		// �T�[�r�X���Ƃ��Čv�Z
		}
		ope_Furikae_stop( vl_frs.lockno, 1 );			// �U�֌��Ԏ��������s
		PiP_FurikaeEnd();
	}
// MH322914(E) K.Onodera 2016/09/05 AI-V�Ή��F�U�֐��Z

	if( OPECTL.op_faz == 2 ){ 		// ���Z������
		// ���Z�������ɑ��̗v���ɂ��d�q�}�̒�~���̏ꍇ�́A���Z�����̒�~�v����D�悳����
		// �����ł́AINITIALIZE���N���A���AOp_StopModuleWait()���Ő��Z�����v���̃t���O���㏑������
		if( CCT_Cansel_Status.BIT.STOP_REASON != REASON_PAY_END ){
			CCT_Cansel_Status.BIT.INITIALIZE = 0;
		}
		Op_StopModuleWait( REASON_PAY_END );		// �d�q�}�̒�~����
		if(OPECTL.ChkPassSyu)
			blink_end();												// �_�ŏI��
	}
	if( w_op_faz == 8 && ( w_op_faz != OPECTL.op_faz && StopStatusGet( 0 ) != 0x07 )){	// �d�q�}�̒�~�҂����킹���Ńt�F�[�Y���ς��悤�ȏꍇ
		OPECTL.op_faz = w_op_faz;						// �ēx�A�d�q�}�̒�~�҂����킹�t�F�[�Y�ɖ߂�
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z����                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod03( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = ���튮��                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod03( void )
{
	short	ret;
	short	ryo_en;
	short	safe_que = 0;
	ushort	msg = 0;
	short	ret_bak = 0;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	ushort	wk_MsgNo;
	uchar	Discon_flg = 0;
	ushort	res_wait_time;
	uchar	idle_req_flg = 0;	// 0=���Z�ē��� 1=���Z�ē��I�������҂� 2=���Z�ē��I��������M�ς�
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	uchar	buf_size;
	uchar	ryo_isu_zumi=0;
	ulong	turi_wk;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
	uchar	print_wait_return =0;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	uchar	announce_flag = 0;			//  ���ޔ������ųݽ���t���O
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	ushort	ec_removal_wait_time;
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//// MH322914(S) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//	short	nDelayAnnounce = 0;
//// MH322914(E) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
	uchar	dummyNotce_Wait = 0;		// �󎚏��������҂��t���O
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
	Cnm_End_PayTimOut		= 0;
// MH810105(S) MH364301 �C���{�C�X�Ή�
	OPECTL.f_RctErrDisp = 0;
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	ryo_stock = 0;						// �X�g�b�N�Ȃ�
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	ec_removal_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 13, 3, 1);
	if(ec_removal_wait_time == 0) {
		ec_removal_wait_time = 360;
	} else if( ec_removal_wait_time < 30 ){
		ec_removal_wait_time = 30;
	}
	ec_removal_wait_time *= 50;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X�i���Z�I�������C���j
	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// �f�[�^��M�Ď��^�C�}
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// �͈͊O��5�b
	}
// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X�i���Z�I�������C���j
// GG129000(S) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
	OPECTL.f_req_paystop = 0xff;
// GG129000(E) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52

	// �N���W�b�g�J�[�h���グ���M

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( edy_dsp.BIT.edy_Miryo_Loss || edy_dsp.BIT.edy_dsp_change || dsp_change || Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
	if( dsp_change || Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		op_SuicaFusokuOff();
	}

	if( Suica_Rec.Data.BIT.OPE_CTRL == 1 ){							// �Ō��Ope�����M�����f�[�^����t�̏ꍇ��
		// �{�����́A���Z�Ȃ��o�ɋy�сA�O�~���Z���ɂ͐��Z�������o���ɐ��Z����̪��ނɈڍs���邽�߁A
		// ������Suica�̒�~���s��
		Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
	}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( PayData.Electron_data.Edy.e_pay_kind != EDY_USED ){
//		Edy_StopAndLedOff();
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	memset( DspChangeTime, 0, sizeof( DspChangeTime ));				// ܰ��ر�̏�����
// MH810103(s) �d�q�}�l�[�Ή� ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���
	// ���Z��Ԓʒm(�̎��؂��g����悤�ɂȂ��Ă���\�������邽�߁A�O�̂���)
	dspCyclicErrMsgRewrite();
// MH810103(e) �d�q�}�l�[�Ή� ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���

// MH810100(S) K.Onodera 2020/01/06 �Ԕԃ`�P�b�g���X�iQR�m��E����f�[�^�Ή��j
//	IoLog_wrrite(IOLOG_EVNT_AJAST_FIN, (ushort)LockInfo[OPECTL.Pr_LokNo - 1].posi, 0, 0);
//	OPE_red = 2;													// ذ�ް�����r�o
//
//	if( OPECTL.PriUsed != 1 && RD_Credit_kep == 0){
//		rd_shutter();												// ���Cذ�ް���������
//	}
// MH810100(E) K.Onodera 2020/01/06 �Ԕԃ`�P�b�g���X�iQR�m��E����f�[�^�Ή��j

// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//	if( !OPECTL.multi_lk ){											// ������Z�⍇�����ȊO?
	if( !OPECTL.multi_lk && (!g_PipCtrl.stRemoteFee.Type || g_PipCtrl.stRemoteFee.ReqKind != UNI_KIND_REMOTE_FEE) ){	// ������Z�⍇�����ł��A������Z�ł��Ȃ��H
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
//		queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof( OPECTL.Pr_LokNo ), &OPECTL.Pr_LokNo );	// ���Z����������(�ׯ�߰���~)
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_year = car_ot_f.year;// ���Z����	�i�N�j���
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_mont = car_ot_f.mon;	// 			�i���j���
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_date = car_ot_f.day;	// 			�i���j���
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_hour = car_ot_f.hour;// 			�i���j���
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_minu = car_ot_f.min;	// 			�i���j���
// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//		LongTermParkingRel( OPECTL.Pr_LokNo );						// �������ԉ���(���Z)
		LongTermParkingRel( OPECTL.Pr_LokNo, LONGPARK_LOG_PAY ,&FLAPDT.flp_data[OPECTL.Pr_LokNo - 1]);	// �������ԉ���(���Z)
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�

// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
//		if( OPECTL.Pay_mod == 2 ){
//			queset( FLPTCBNO, FLAP_UP_SND_SS, sizeof(OPECTL.MPr_LokNo), &OPECTL.MPr_LokNo );		// �C�����ׯ�ߏ㏸
//		}
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// �s��C��(S) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
//		// �U�֐��Z�H
//		if( g_PipCtrl.stFurikaeInfo.ReqKind == UNI_KIND_FURIKAE ){
//			// �U�֐斢���~�H
//			if( FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].nstat.bits.b01 == 1 ){
//				FurikaeDestFlapNo = OPECTL.Pr_LokNo;
//			}
//		}
//// �s��C��(E) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	}
	PayEndSig();													// ���Z�����M���o��
// GG129000(S) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5684�F�̎��ؕK�����s�ݒ�ŗ̎��ؔ��s�����ꍇ��NT-NET���Z�f�[�^�̗̎��ؔ��s�L����0���̎��؂Ȃ��ɂȂ��Ă���j
	QRIssueChk(0);
// GG129000(E) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5684�F�̎��ؕK�����s�ݒ�ŗ̎��ؔ��s�����ꍇ��NT-NET���Z�f�[�^�̗̎��ؔ��s�L����0���̎��؂Ȃ��ɂȂ��Ă���j

	ryo_en = 0;														// �����ݕs��
	switch( OPECTL.Fin_mod ){
		case 1:														// �ނ薳��?
		default:
			ac_flg.cycl_fg = 13;									// �����o������(��)
			cn_stat( 6, 0 );										// ������ر(���ۺ�݂𗎂Ƃ�)
			ryo_en = 1;												// �����݉�
			if( OPECTL.RECI_SW == 1 ){								// �̎������ݎg�p?
				ryo_isu( 0 );										// �̎���(�ޑK�s������)���s
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
				print_wait_return = 1;								// �̎��ؔ��s����
				if (IS_INVOICE &&
					OPECTL.f_DelayRyoIsu != 2) {
				}else{
					ryo_isu_zumi = 1;								// �̎��ؔ��s�ς�
				}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
// GG129000(S) R.Endo 2023/02/22 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
				// �̎��ؔ��s�{�^��������̗̎��؈󎚕s�ŗ̎��ؔ��s���s�\�����邽��
				// �̎��؈󎚕s���̃��V�[�g�󎚎��s����������ł��s��
				op_rct_failchk();
// GG129000(E) R.Endo 2023/02/22 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
			}
// MH810105(S) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
//			if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )			// ���Z�����ʐM�d�����M�^�C�~���O=���Z��������
			// �Q�[�g���Ɠ��l�ɐ��Z�����W�v�̃^�C�~���O�𑁂߂�
// MH810105(E) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
			{
				if( !OPECTL.f_KanSyuu_Cmp ){						// �����W�v�����{
					PayData_set( 0, 0 );							// 1���Z����
					kan_syuu();										// ���Z�����W�v
				}
			}
			break;

		case 2:														// �ނ�L��?
			turi_wk = 0;
			if( ryo_buf.turisen != 0 ){
				turi_wk = ryo_buf.turisen;
			}else if( ryo_buf. mis_tyu != 0 ){
				turi_wk = ryo_buf.mis_tyu;
			}
// �d�l�ύX(S) K.Onodera 2016/10/28 �U�։ߕ������ƒޑK�؂蕪��
			if( ryo_buf.kabarai != 0 ){
				turi_wk += ryo_buf.kabarai;
			}
// �d�l�ύX(E) K.Onodera 2016/10/28 �U�։ߕ������ƒޑK�؂蕪��
			if( refund( (long)turi_wk ) != 0 ){				// ���o�N��
				/*** ���oNG ***/
				ac_flg.cycl_fg = 13;								// �����o������(��)
				ryo_en = 1;											// �����݉�
				ryo_buf.fusoku = SFV_DAT.reffal;					// �x�����s���z
				set_escrowtimer();
				Print_Condition_select();							// �̎���(�ޑK�s���L��) or �a��ؔ��s
// MH810105(S) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
//				if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )		// ���Z�����ʐM�d�����M�^�C�~���O=���Z��������
				// �Q�[�g���Ɠ��l�ɐ��Z�����W�v�̃^�C�~���O�𑁂߂�
// MH810105(E) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
				{
					if( !OPECTL.f_KanSyuu_Cmp ){					// �����W�v�����{
						PayData_set( 0, 0 );						// 1���Z����
						kan_syuu();									// ���Z�����W�v
					}
				}
			}else{
				if( (CN_refund & 0x01) == 0 ){
					// ��݂̕����o�����h�Ȃ��h�̂Ƃ�
					cn_stat( 6, 0 );									// ������ر(���ۺ�݂𗎂Ƃ�)
				}
				else{												// �����o������
					Lagtim( OPETCBNO, 20, (ushort)(((turi_wk/10)+5)*50));		// ��ү������o�������҂���ϰSTART
					Cnm_End_PayTimOut = 1;							// �����o���Ď�����
				}
			}
			LedReq( CN_TRAYLED, LED_ON );							// ��ݎ�o�����޲��LED�_��
			break;

		case 3:														// ү��װ(�ނ�L��)?
			ac_flg.cycl_fg = 13;									// �����o������(��)
			ryo_en = 1;												// �����݉�
			ryo_buf.fusoku = SFV_DAT.reffal;						// �x�����s���z
			set_escrowtimer();
			Print_Condition_select();								// �̎���(�ޑK�s���L��) or �a��ؔ��s
// MH810105(S) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
//			if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )			// ���Z�����ʐM�d�����M�^�C�~���O=���Z��������
			// �Q�[�g���Ɠ��l�ɐ��Z�����W�v�̃^�C�~���O�𑁂߂�
// MH810105(E) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
			{
				if( !OPECTL.f_KanSyuu_Cmp ){						// �����W�v�����{
					PayData_set( 0, 0 );							// 1���Z����
					kan_syuu();										// ���Z�����W�v
				}
			}
			break;
	}
// GG129000(S) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
//// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	// ���[�����j�^�f�[�^�o�^
//	SetLaneFreeNum(0);
//// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.54)�iGM803000���p�j
//	SetLaneFeeKind(0);								// ������ʃN���A
//// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.54)�iGM803000���p�j
//	ope_MakeLaneLog(LM_PAY_CMP);
//// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129000(E) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
// GG129000(S) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5684�F�̎��ؕK�����s�ݒ�ŗ̎��ؔ��s�����ꍇ��NT-NET���Z�f�[�^�̗̎��ؔ��s�L����0���̎��؂Ȃ��ɂȂ��Ă���j
//// GG129000(S) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
//	QRIssueChk(0);
//// GG129000(E) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129000(E) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5684�F�̎��ؕK�����s�ݒ�ŗ̎��ؔ��s�����ꍇ��NT-NET���Z�f�[�^�̗̎��ؔ��s�L����0���̎��؂Ȃ��ɂȂ��Ă���j
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	DspChangeTime[0] = (ushort)(prm_get(COM_PRM, S_DSP, 30, 1, 1)); // ���p�\�}�̻��د��\���p��ϰ�l�擾
//	if(DspChangeTime[0] <= 0) {
//		DspChangeTime[0] = 2;
//	}
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//	if (isEC_USE() != 0) {
//		Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
//	} else {
//// MH321800(E) G.So IC�N���W�b�g�Ή�
//		mode_Lagtim10 = 0;											// Lagtimer10�̎g�p��� 0:���p�\�}�̻��د��\��
//		suica_fusiku_flg = 0;										// ���p�\�}�̻��د��\���L��
//		Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );		// ���p�\�}�̻��د��\���p��ϰSTART
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//	}
//// MH321800(E) G.So IC�N���W�b�g�Ή�
//	OpeLcd( 5 );													// ���Z�����\��

// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
//	// ���Z�c���ω��ʒm�̑��M����
//	lcdbm_pay_rem_chg(1);	// 1=���Z����
	if ( !cmp_send ) {			// �c���ω��ʒm(���Z����)�����M
		lcdbm_pay_rem_chg(1);	// �c���ω��ʒm(���Z����)
		cmp_send = 0;
	}
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

// GG129000(S) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
	// ���[�����j�^�f�[�^�o�^
	SetLaneFreeNum(0);
	ope_MakeLaneLog(LM_PAY_CMP);
	SetLaneFeeKind(0);								// ������ʃN���A
// GG129000(E) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j

	cn_stat( 4, 1 );												// �����捞

	PayDataErrDisp();
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH322914(S) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//	if( (cr_dat_n == RID_SAV) && ( 1 == IsMagReaderRunning() ) ){	// �T�[�r�X���g�p��Ń��[�_�[�������H
//		nDelayAnnounce = 1;
//	}
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//	else if( Suica_Rec.Data.BIT.CTRL_CARD ) {						// ���σ��[�_�ڑ��ŃJ�[�h������Ԃ���
//		// �J�[�h�������҂��^�C�}�[�N��
//		Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );
//		announce_flag = 1;
//		// �u�J�[�h������艺�����v�\���ƃA�i�E���X������
//		ec_MessageAnaOnOff( 1, 3 );
//	}
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
//	else{
//// MH322914(E) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//	if( !OPECTL.multi_lk ){											// ������Z�⍇�����ȊO?
//		ope_anm( AVM_BGM );
//		if( OPECTL.Pr_LokNo >= LOCK_START_NO ){
//			ope_anm( AVM_KANRYO );									// ���Z�������ųݽ(���b�N)
//		}else{
//			ope_anm( AVM_KANRYO2 );									// ���Z�������ųݽ(�t���b�v)
//		}
//	}
//	else{	// �}���`���Z���̓��b�N���̉����𗬂��idefault�ł́u���肪�Ƃ��������܂����v�j
//		ope_anm( AVM_KANRYO );										// ���Z�������ųݽ(���b�N)
//	}
//// MH322914(S) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//	}
//// MH322914(E) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
	// ���σ��[�_�ڑ��ŃJ�[�h������Ԃ���
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if( Suica_Rec.Data.BIT.CTRL_CARD ) {						// ���σ��[�_�ڑ��ŃJ�[�h������Ԃ���
	if( isEC_CARD_INSERT() ) {									// ���σ��[�_�ڑ��ŃJ�[�h������Ԃ���
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		// �J�[�h�������҂��^�C�}�[�N��
		Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );
	}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	// ���σ��[�_�ڑ��ݒ�łȂ�||�J�[�h�𔲂�����Ă�������ϰ1(10s)�N��
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if( !isEC_USE() || !Suica_Rec.Data.BIT.CTRL_CARD ){
	if( !isEC_CARD_INSERT() ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
//	Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
	if( r_zero_call ){
		Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );		// ��ϰ1(10s)�N��(���ڰ��ݐ���p)�F�Z�k�p
	}else{
		Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );		// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
	}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
	Lagcan( OPETCBNO, 2 );											// Timer Cancel
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
	if( OPECTL.init_sts != 1 ){	// �������������H(op_mod02()�ŋN���ҋ@��ԑJ�ڃC�x���g�����H)
		Discon_flg = 1;
	}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
	if( OPECTL.RECI_SW == 1 && print_wait_return == 0){		// �̎������݉����ς݁H
		print_wait_return = 1;								// �̎��ؔ��s����
		OPECTL.RECI_SW = 0;								// ��������߂��ă��b�Z�[�W�ʒm
		queset( OPETCBNO, OPE_DELAY_RYO_PRINT, 0, NULL );
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
// MH810100(S) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
	if( OPECTL.chg_idle_disp_rec ){	// �ҋ@��ʒʒm��M�ς�
		queset( OPETCBNO, LCD_CHG_IDLE_DISP, 0, NULL );
	}
// MH810100(E) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case COIN_EVT:											// Coin Mech event
				ryo_en = 1;											// �����݉�
				if( OPECTL.CN_QSIG == 2 || OPECTL.CN_QSIG == 9 ){
					Lagcan( OPETCBNO, 20 );
					Cnm_End_PayTimOut = 0;
				}
				if( OPECTL.CN_QSIG == 2 ){							// ���o����
					ac_flg.cycl_fg = 13;							// �����o������
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );		// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					if( r_zero_call ){
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)�F�Z�k�p
					}else{
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
					if( SFV_DAT.reffal ){
						ryo_buf.fusoku += SFV_DAT.reffal;			// �x�����s���z
						Print_Condition_select();					// �̎���(�ޑK�s���L��) or �a��ؔ��s
					}
					else if( OPECTL.RECI_SW ){						// �̎������ݏ��
						if( !ryo_isu_zumi ){
						ryo_isu( 0 );								// �̎���(�ޑK�s������)���s
					    OPECTL.RECI_SW = 1;							// �̎������ݎg�p
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
						print_wait_return = 1;								// �̎��ؔ��s����
						if (IS_INVOICE &&
							OPECTL.f_DelayRyoIsu != 2) {
						}else{
							ryo_isu_zumi = 1;								// �̎��ؔ��s�ς�
						}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
						}
					}
// MH810105(S) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
//					if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )	// ���Z�����ʐM�d�����M�^�C�~���O=���Z��������
					// �Q�[�g���Ɠ��l�ɐ��Z�����W�v�̃^�C�~���O�𑁂߂�
// MH810105(E) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
					{
						if( !OPECTL.f_KanSyuu_Cmp ){				// �����W�v�����{
							PayData_set( 0, 0 );					// 1���Z����
							kan_syuu();								// ���Z�����W�v
						}
					}
				}
				else if( OPECTL.CN_QSIG == 9 ){						// ���o�װ
					ac_flg.cycl_fg = 13;							// �����o������(��)
					refalt();										// �s�����Z�o
					ryo_buf.fusoku += SFV_DAT.reffal;				// �x�����s���z
					Print_Condition_select();						// �̎���(�ޑK�s���L��) or �a��ؔ��s
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					if( r_zero_call ){
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)�F�Z�k�p
					}else{
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
					set_escrowtimer();
// MH810105(S) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
//					if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )	// ���Z�����ʐM�d�����M�^�C�~���O=���Z��������
					// �Q�[�g���Ɠ��l�ɐ��Z�����W�v�̃^�C�~���O�𑁂߂�
// MH810105(E) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
					{
						if( !OPECTL.f_KanSyuu_Cmp ){				// �����W�v�����{
							PayData_set( 0, 0 );					// 1���Z����
							kan_syuu();								// ���Z�����W�v
						}
					}
				}
// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X�i���Z�I�������C���j
//				if( ret_bak ) ret = ret_bak;
// MH810100(S) K.Onodera 2020/03/02 #3964 �̎��؂��󎚂��Ȃ�
//				ret = 10;
				if( ret_bak ){
					ret = ret_bak;
				}
// MH810100(E) K.Onodera 2020/03/02 #3964 �̎��؂��󎚂��Ȃ�
				// �ؒf���Ă�����A��������M�������Ƃɂ���
				if( Discon_flg ){
					idle_req_flg = 2;
				}
// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X�i���Z�I�������C���j
				break;

// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			case KEY_TEN0:											// �ݷ�[0] �����
//			case KEY_TEN1:											// �ݷ�[1] �����
//			case KEY_TEN2:											// �ݷ�[2] �����
//			case KEY_TEN3:											// �ݷ�[3] �����
//			case KEY_TEN4:											// �ݷ�[4] �����
//			case KEY_TEN5:											// �ݷ�[5] �����
//			case KEY_TEN6:											// �ݷ�[6] �����
//			case KEY_TEN7:											// �ݷ�[7] �����
//			case KEY_TEN8:											// �ݷ�[8] �����
//			case KEY_TEN9:											// �ݷ�[9] �����
//// MH322914(S) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//				if( 1 == IsMagReaderRunning() ){					// ���[�_�[�������H
//					break;
//				}
//// MH322914(E) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//				if( ryo_en ){
//					Key_Event_Set( msg );
//				}
//			case KEY_TEN_CL:										// �ݷ�[���] �����
//// MH322914(S) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//				if( 1 == IsMagReaderRunning() ){					// ���[�_�[�������H
//					break;
//				}
//// MH322914(E) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//				BUZPI();
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				// �J�[�h�������A�i�E���X����[KEY_TEN]�C�x���g�𔭐������Ȃ�
//				if( announce_flag == 1 ){
//					break;
//				}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//				// NOTE:�L�[���������_�ŉ������~�߁A���Z�f�[�^7�����Ƃ�FROM�Ƀ��C�g�A�N�Z�X����̂ł��̎��Ԃ𑁂߂Ɋm�ۂ����Z����̒x�����y������
//				ope_anm( AVM_STOP );								// ������~�ųݽ
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			case TIMEOUT1:											// Time Over ?
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				// �J�[�h�������A�i�E���X����[TIMEOUT1]�C�x���g�𔭐������Ȃ�
//				if( announce_flag == 1 ){
//					break;
//				}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
				if( ryo_en == 0 ){
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					if( r_zero_call ){
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );		// ��ϰ1(10s)�N��(���ڰ��ݐ���p)�F�Z�k�p
					}else{
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );		// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
					break;
				}
				if( SFV_DAT.safe_cal_do != 0 ){						// ���Ɍv�Z���I��?
					if( msg == TIMEOUT1 ){
						safe_que++;
						if( safe_que < 10 ){
							Lagtim( OPETCBNO, 1, 1*50 );				// ��ϰ1(1s)�N��(���ڰ��ݐ���p)
							break;
						}
						ex_errlg( ERRMDL_MAIN, ERR_MAIN_NOSAFECLOPERATE, 2, 0 );	// ���ɖ����Z�o�Ȃ��ɔ�����
					}else{
						break;
					}
				}
				ret = 10;
				break;

// MH810105(S) MH364301 �C���{�C�X�Ή�
			case OPE_DELAY_RYO_PRINT:								// op_jnl_failchk��̗̎��ز����
				goto _LCDBM_OPCD_RCT_ISU;	// ����ʒm�̗̎��ؔ��s�ɔ�΂�
				break;
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
//				if( ReceiptChk() == 1 || Ope_isPrinterReady() == 0){	// ڼ�Ĕ��s�s�v�ݒ莞
//					break;
//				}
//				if(	(OPECTL.Pay_mod == 2)&&
//					(!prm_get(COM_PRM, S_PRN, 32, 1, 6))){
//					BUZPIPI();
//					break;
//				}
//
//				BUZPI();
//				if( OPECTL.RECI_SW == 0 ){							// �̎������ݖ��g�p?
//					OPECTL.RECI_SW = 1;								// �̎������ݎg�p
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//					// �J�[�h�������A�i�E���X����[F2]������u�̎��؂�����艺�����v��\�������Ȃ�
//					if( announce_flag != 1 ){
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//					wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
//					grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);		// "     �̎��؂�����艺����     "
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//					}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//					if( ryo_en == 0 ){
//						break;
//					}
//					ryo_isu( 0 );									// �̎���(�ޑK�s������)���s
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//					OPECTL.PassNearEnd = 0;							// �����؂�ԋ�
//					ryo_isu_zumi = 1;								// �̎��ؔ��s�ς�
//				}
//				break;
//
//			case KEY_TEN_F4:										// �ݷ�[���] �����
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				// �J�[�h�������A�i�E���X����[���]�C�x���g�𔭐������Ȃ�
//				if( announce_flag == 1 ){
//					break;
//				}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//				BUZPI();
//				// NOTE:�L�[���������_�ŉ������~�߁A���Z�f�[�^7�����Ƃ�FROM�Ƀ��C�g�A�N�Z�X����̂ł��̎��Ԃ𑁂߂Ɋm�ۂ����Z����̒x�����y������
//				ope_anm( AVM_STOP );								// ������~�ųݽ
//				if(( ryo_en != 0 )&&( SFV_DAT.safe_cal_do == 0 )){	// ���o�����ŋ��Ɍv�Z�I���Ȃ�
//					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
//					ret = 10;
//				}
//				break;
//
//			case ARC_CR_R_EVT:										// ����IN
//				cr_service_holding = 0;								// ���޽���ۗ̕�����
//				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
//				break;
//
//			case ARC_CR_EOT_EVT:									// ���ޔ������
//				if( RD_mod < 9 ){
//					rd_shutter();									// 500msec��ڲ��ɼ��������
//				}
//				OPECTL.PassNearEnd = 0;								// �����؂�ԋ�
//				break;
//
//			case IBK_MIF_A2_NG_EVT:									// Mifare�ް��Ǐo��NG
//				if( MIF_ENDSTS.sts1 == 0x30 ){						// ���ޖ���
//					OPECTL.PassNearEnd = 0;							// �����؂�ԋ��׸�
//				}
//				break;
//
//// MH322914(S) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//			case ARC_CR_E_EVT:
//				if( (1 == nDelayAnnounce) && ( !IsMagReaderRunning() ) ){
//					nDelayAnnounce = 0;
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//					// �J�[�h�𔲂�����Ă�������ϰ1(10s)�N��
//					if( !Suica_Rec.Data.BIT.CTRL_CARD ){
//						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//					}
//					else {
//						// �J�[�h�������҂��^�C�}�[�N��
//						Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );
//						announce_flag = 1;
//						// �u�J�[�h������艺�����v�\���ƃA�i�E���X������
//						ec_MessageAnaOnOff( 1, 3 );
//						break;
//					}
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
//					if( !OPECTL.multi_lk ){							// ������Z�⍇�����ȊO?
//						ope_anm( AVM_BGM );
//						if( OPECTL.Pr_LokNo >= LOCK_START_NO ){
//							ope_anm( AVM_KANRYO );					// ���Z�������ųݽ(���b�N)
//						}else{
//							ope_anm( AVM_KANRYO2 );					// ���Z�������ųݽ(�t���b�v)
//						}
//					}
//					else{	// �}���`���Z���̓��b�N���̉����𗬂��idefault�ł́u���肪�Ƃ��������܂����v�j
//						ope_anm( AVM_KANRYO );						// ���Z�������ųݽ(���b�N)
//					}
//				}
//				break;
//// MH322914(E) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
//
//			case TIMEOUT10:											// �u������ʕ\���ؑցv����ѱ�Ēʒm
//				dspCyclicMsgRewrite(1);	// �T�C�N���b�N�\���X�V
//				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
//				break;
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//			case EC_CYCLIC_DISP_TIMEOUT:							// �u������ʕ\���ؑցv����ѱ�Ēʒm
//				dspCyclicMsgRewrite(1);	// �T�C�N���b�N�\���X�V
//				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
//				break;
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			case TIMEOUT20:											// �����o���\BITON�҂���ѱ�Ēʒm
				Cnm_End_PayTimOut = 0x02;
				break;
			case OPE_REQ_CALC_FEE:									// �����v�Z�v��
				// �����v�Z
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);				// �S�Ԏ����e�[�u�����M
				break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
			case EC_REMOVAL_WAIT_TIMEOUT:
				// �J�[�h�������҂���ϰ����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				Suica_Rec.Data.BIT.CTRL_CARD = 0;					// �J�[�h���������ꂽ���ɂ���
				if (isEC_USE()) {
					Suica_Rec.Data.BIT.CTRL_CARD = 0;				// �J�[�h���������ꂽ���ɂ���
				}
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
				// no break
			case EC_EVT_CARD_STS_UPDATE:
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//				// ���ޔ������ųݽ��
//				if( announce_flag ){
//					// �J�[�h�������҂���ϰؾ��
//					Lagcan( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT );
//					ec_MessageAnaOnOff( 0, 0 );
//					if(!Suica_Rec.Data.BIT.CTRL_CARD) {				// �J�[�h�����Ȃ�
//						if(!nDelayAnnounce) {
//							Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );			// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//							if( !OPECTL.multi_lk ){											// ������Z�⍇�����ȊO?
//								ope_anm( AVM_BGM );
//								if( OPECTL.Pr_LokNo >= LOCK_START_NO ){
//									ope_anm( AVM_KANRYO );									// ���Z�������ųݽ(���b�N)
//								}else{
//									ope_anm( AVM_KANRYO2 );									// ���Z�������ųݽ(�t���b�v)
//								}
//							}
//							else{	// �}���`���Z���̓��b�N���̉����𗬂��idefault�ł́u���肪�Ƃ��������܂����v�j
//								ope_anm( AVM_KANRYO );										// ���Z�������ųݽ(���b�N)
//							}
//							announce_flag = 0;
//						}
//					}
//				}
				// �J�[�h�������҂��^�C�}�[���Z�b�g
				Lagcan( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT );
				// �J�[�h�����Ȃ�
				if( !Suica_Rec.Data.BIT.CTRL_CARD ){
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );			// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					if( r_zero_call ){
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)�F�Z�k�p
					}else{
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
				}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
				break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			// QR�f�[�^
			case LCD_QR_DATA:
				// QR�����f�[�^
				lcdbm_QR_data_res( 1 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
				break;

			// ����ʒm(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// ����ʒm		���캰��
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// ���Z���~�v��
					case LCDBM_OPCD_PAY_STP:
						// ����ʒm(���Z���~����(NG))���M	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						break;

					// ���Z�J�n�v��
					case LCDBM_OPCD_PAY_STA:
// MH810100(S) K.Onodera 2020/02/18 #3883 �u�ʎԗ��̐��Z�v��������ƎԔԌ�����ʂɑJ�ڂ��Ă��܂�
//						// ����ʒm(���Z�J�n����(OK))���M	0=OK/1=NG
//						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );
//
//						// �ϐ��������R��h�~�ׁ̈Aop_mod00()��ʂ���op_mod01()�ɑJ�ڂ���
//						OPECTL.Ope_mod = 0;			// op_mod00(�ҋ@)
//						OPECTL.rec_pay_start = 1;	// ���Z�J�n�v����M�t���OON
//						ret = 1;
						// ���Z�J�n�������M(NG)
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
						break;

					// �ҋ@��ʒʒm
					case LCDBM_OPCD_CHG_IDLE_DISP_NOT:
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
//						ret = 10;
						if(print_wait_return){								// �̎��ؔ��s����
							queset( OPETCBNO, LCD_CHG_IDLE_DISP, 0, NULL );
						}else{
							ret = 10;
						}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
// MH810100(E) K.Onodera 2020/02/18 #3883 �u�ʎԗ��̐��Z�v��������ƎԔԌ�����ʂɑJ�ڂ��Ă��܂�
						break;

// MH810105(S) MH364301 �C���{�C�X�Ή� GT-4100
_LCDBM_OPCD_RCT_ISU:
// MH810105(E) MH364301 �C���{�C�X�Ή� GT-4100
					// �̎��ؔ��s
					case LCDBM_OPCD_RCT_ISU:
// GG129000(S) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
						// �̎��ؔ��s�{�^��������̗̎��؈󎚕s�ŕK���̎��ؔ��s���s�\�����邽�ߕK���t���O�𗧂Ă�
						// ���e�����̃G���[����break��LCDBM_OPCD_RCT_ISU�̃P�[�X�𔲂�����
						// �@���V�[�g�󎚎��s������s���ׁAif���ł͂Ȃ�while����p����B
						while ( OPECTL.RECI_SW == 0 ) {						// �̎������ݖ��g�p?
							OPECTL.RECI_SW = 1;								// �̎������ݎg�p
// GG129000(E) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
// GG129000(S) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
//						if( ReceiptChk() == 1 || Ope_isPrinterReady() == 0){	// ڼ�Ĕ��s�s�v�ݒ莞
						if( (QRIssueChk(1) == 0 && ReceiptChk() == 1) ||	// QR���Ԍ����s�Ȃ��A���A���V�[�g���s�s�ݒ�
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 							Ope_isPrinterReady() == 0){						// ڼ�Ĕ��s�s��
							(!IS_ERECEIPT && (Ope_isPrinterReady() == 0))){	// ڼ�Ĕ��s�s��
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// GG129000(E) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
							break;
						}
// MH810105(S) MH364301 �C���{�C�X�Ή�
						if (IS_INVOICE && Ope_isJPrinterReady() == 0) {		// ���޲��ݒ莞�ͼެ��ٔ��s�s�ł�
							break;											// �󎚂��Ȃ�
						}
// MH810105(E) MH364301 �C���{�C�X�Ή�
						if(	(OPECTL.Pay_mod == 2)&&
							(!prm_get(COM_PRM, S_PRN, 32, 1, 6))){
							BUZPIPI();
							break;
						}
// GG129000(S) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
// 						if( OPECTL.RECI_SW == 0 ){							// �̎������ݖ��g�p?
// 							OPECTL.RECI_SW = 1;								// �̎������ݎg�p
// GG129000(E) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
							if( ryo_en == 0 ){
								break;
							}
// MH810105(S) MH364301 �C���{�C�X�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
							print_wait_return = 1;								// �̎��ؔ��s����
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
							if (IS_INVOICE &&
								OPECTL.f_DelayRyoIsu != 2) {
								break;
							}
							if (ryo_isu_zumi) {
								// �̎��ؔ��s�ς�
								break;
							}
// MH810105(E) MH364301 �C���{�C�X�Ή�
							ryo_isu( 0 );									// �̎���(�ޑK�s������)���s
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
//							Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
							if( r_zero_call ){
								Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)�F�Z�k�p
							}else{
								Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
							}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
							OPECTL.PassNearEnd = 0;							// �����؂�ԋ�
							ryo_isu_zumi = 1;								// �̎��ؔ��s�ς�
						}
// GG129000(S) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
						// �̎��؈󎚕s��̗̎��ؔ��s�{�^�������ŗ̎��ؔ��s���s�\�����邽��
						// �̎��؈󎚕s���̃��V�[�g�󎚎��s����������ł��s��
						op_rct_failchk();
// GG129000(E) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
						break;

					// �N���ʒm
					case LCDBM_OPCD_STA_NOT:
						Discon_flg = 1;		// �I����A�N�����ɑJ�ڂ���
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							OPECTL.init_sts = 0xff;	// ������������(�N���ʒm��M�ς�)��ԂƂ���
						}
						// 1=�ʏ�
						else{
							OPECTL.init_sts = 0xfe;	// �����������ς�(�N���ʒm��M�ς�)��ԂƂ���
						}
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
						break;

// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
					// �N�������ʒm
					case LCDBM_OPCD_STA_CMP_NOT:
						if( OPECTL.init_sts != 1 ){	// �������������H
							// op_mod03()�̏�Ԃ�FTP���Ă��Ȃ��̂ɋN�������ʒm����M����Ƃ������Ƃ�
							// LCD��FTP�s�v�Ƃ̔��f�Ȃ̂Ő��Z�����������op_mod00()�ɖ߂�
							OPECTL.init_sts = 1;
							OPECTL.Ope_mod = 0;
							Discon_flg = 2;			// �I����A�ҋ@�ɑJ�ڂ���
						}
						break;

// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
					// ���Z�ē��I������
					case LCDBM_OPCD_PAY_GUIDE_END_RES:
						Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );
						// ������M�t���OON
						idle_req_flg = 2;
						break;
				}
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				Discon_flg = 1;		// �I����A�N�����ɑJ�ڂ���
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
				OPECTL.init_sts = 0;	// ��������������ԂƂ���
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(IC�N���W�b�g�⍇�����̋N���ʒm/�ؒf�ʒm����������)
				// ���o�������H
				if( ryo_en != 0 ){
					idle_req_flg = 2;	// ��������M�������Ƃɂ��Ă���
					ret = 10;
				}
				break;

			// ���Z�ē��I�������^�C���A�E�g
			case TIMEOUT_MNT_RESTART:
				// ���Z�ē��I���ʒm
				lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
				// �����҂��^�C�}�[�ĊJ
				Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

// MH810100(S) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
			// �ҋ@��ʒʒm��M�ς�
			case LCD_CHG_IDLE_DISP:
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
//				OPECTL.chg_idle_disp_rec = 0;
//				ret = 10;
				if(print_wait_return && ryo_isu_zumi == 0){					// �̎��ؔ��s����̏ꍇ�͂܂�����
					queset( OPETCBNO, LCD_CHG_IDLE_DISP, 0, NULL );
				}else{
					OPECTL.chg_idle_disp_rec = 0;
					ret = 10;
				}
// GG129000(S) A.Shirai 2023/10/2 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5688�F�ޑK�؂�Ő��Z������g�b�v�ɖ߂�{�^�����������Ă��g�b�v��ʂɖ߂�Ȃ��j
				if (Ope_isJPrinterReady() == 0 || (paperchk2() != 0)) {
					OPECTL.chg_idle_disp_rec = 0;
					ret = 10;
				}
// GG129000(E) A.Shirai 2023/10/2 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5688�F�ޑK�؂�Ő��Z������g�b�v�ɖ߂�{�^�����������Ă��g�b�v��ʂɖ߂�Ȃ��j
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6424 ���Z������A�����ɐ��Z������ʂ���ҋ@��ʂɑJ�ڂ���Ɨ̎��؂��󎚂���Ȃ�
				break;
// MH810100(E) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
// MH810104(S) MH321800(S) ����f�[�^�i��t���j���M�^�C�}���쒆�t���O�N���A�s�
			case EC_RECEPT_SEND_TIMEOUT:
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,  0 );
				break;
// MH810104(E) MH321800(E) ����f�[�^�i��t���j���M�^�C�}���쒆�t���O�N���A�s�
// MH810105(S) MH364301 �C���{�C�X�Ή�
			case PRIEND_PREQ_RYOUSYUU:
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//				if (!IS_INVOICE) {
//// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
//					if (OPECTL.Pri_Kind == R_PRI) {
//						// ���V�[�g�󎚊����҂��^�C�}�[�N��
//						rpt_pri_wait_Timer = GyouCnt_All_r + GyouCnt_r;							// ���M�ς݈��ް��S�s���擾
//						Lagtim(OPETCBNO, TIMERNO_RPT_PRINT_WAIT, (rpt_pri_wait_Timer * 5));		// 1�s/100ms(20ms*5)
//					}
//// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
//					break;
//				}
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j

				if (OPECTL.Pri_Kind == J_PRI) {
					// �W���[�i����

// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//					// �Z���̎��؁i�������Z�Ȃǁj�󎚂ň󎚒��Ƀt�^���J�����ꍇ�A
//					// �󎚊������󎚎��s�Ƃ������Œʒm�����ꍇ������
//					// ���̂��߁A�󎚊�����M���Wait�^�C�}�𓮍삳����
//					// �^�C�}���쒆�Ɉ󎚎��s����M�����ꍇ�͈󎚎��s�Ƃ��Ĉ���
//// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
////					LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, JNL_PRI_WAIT_TIME, op_jnl_failchk);
//// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
////					LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, jnl_pri_wait_Timer, op_jnl_failchk);
//					jnl_pri_wait_Timer = GyouCnt_All_j + GyouCnt_j;								// ���M�ς݈��ް��S�s���擾
//// GG129000(S) M.Fujikawa 2023/10/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7170
//					if(jnl_pri_wait_Timer == 0){
//						jnl_pri_wait_Timer = 1;
//					}
//// GG129000(E) M.Fujikawa 2023/10/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7170
//// GG129000(S) M.Fujikawa 2023/10/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�󎚊����^�C�~���O�𑁂߂�
////					LagTim20ms(LAG20_JNL_PRI_WAIT_TIMER, (jnl_pri_wait_Timer *5), op_jnl_failchk);		// 1�s/100ms(20ms*5)
//					LagTim20ms(LAG20_JNL_PRI_WAIT_TIMER, (jnl_pri_wait_Timer *3), op_jnl_failchk);		// 1�s/60ms(20ms*3)
//// GG129000(E) M.Fujikawa 2023/10/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�󎚊����^�C�~���O�𑁂߂�
//// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
//// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�̎��ؔ��s�^�C�~���O�ύX�j
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
				}
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
				else {
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//					if( OPECTL.f_DelayRyoIsu == 2 && ryo_isu_zumi == 1 ){
					if( ryo_isu_zumi == 1 ){
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
						// ���V�[�g�󎚊����҂��^�C�}�[�N��
						rpt_pri_wait_Timer = GyouCnt_All_r + GyouCnt_r;							// ���M�ς݈��ް��S�s���擾
						Lagtim(OPETCBNO, TIMERNO_RPT_PRINT_WAIT, (rpt_pri_wait_Timer * 5));		// 1�s/100ms(20ms*5)
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
						ryo_isu_zumi = 0xff;						// ���V�[�g�󎚊���
						if( dummyNotce_Wait == 1 ) {				// ���łɈ󎚏��������҂�
							// ��а�ʒm�őҋ@�֑J�ڂ���
							// �󎚏��������҂��ł͂Ȃ��ꍇ�A[�̎���] ����Ă���ϰ1�őҋ@�֑J�ڂ���
							queset(OPETCBNO, OPE_DUMMY_NOTICE, 0, 0);
						}
					}
				}
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
				break;
			case OPE_OPNCLS_EVT:					// �����c�x�� event
// GG129000(S) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
// 				if (!IS_INVOICE) {
// 					break;
// 				}
// 
// 				statusChange_DispUpdate();			// ��ʍX�V
				if ( IS_INVOICE ) {
					statusChange_DispUpdate();		// ��ʍX�V
				}
// GG129000(E) R.Endo 2023/02/20 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�

				if (OPECTL.RECI_SW == 1) {
					// �̎��؃{�^�������ς݂ł���΁A
					// ���V�[�g�󎚎��s������s��
					op_rct_failchk();
				}
// GG129000(S) D.Inaba 2023/10/03 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
				else{
					// �����ς݂ł͂Ȃ��ꍇ�̓t���O��OFF����
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 					if( (Ope_isPrinterReady() == 0) ||					// ڼ�Ĕ��s�s��
					if( (!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) ||	// ڼ�Ĕ��s�s��
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
						(IS_INVOICE && Ope_isJPrinterReady() == 0) ){	// ���޲��ݒ莞�ͼެ��ٔ��s�s��
						OPECTL.f_DelayRyoIsu = 0;
						if( ryo_stock == 1 ){							// �󎚃f�[�^�X�g�b�N��
							MsgSndFrmPrn(PREQ_STOCK_CLEAR, R_PRI, 0);	// �󎚃f�[�^�N���A
							ryo_stock = 2;								// �X�g�b�N�f�[�^�j��
						}
					}
				}
// GG129000(E) D.Inaba 2023/10/03 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
				break;
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
			case RPT_PRINT_WAIT_TIMEOUT:							// ڼ�Ĉ󎚊����҂���ϰ
					// �̎���POP�폜�v��
					lcdbm_notice_del(1,1);
				break;
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
			case TIMEOUT_QR_RECEIPT_DISP:							// QR�R�[�h���s�ē��\���^�C�}�[
				lcdbm_notice_del(1, 1);	// �̎���POP, �󎚊���
				break;
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���Z�������̗̎��ؔ��s���ɑҋ@��Ԃɖ߂�Ȃ��悤�ɂ���j
			case OPE_DUMMY_NOTICE:									// ��а�ʒm
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//				if( !IS_INVOICE ){									// �C���{�C�X�łȂ�
//					break;
//				}
//				if( OPECTL.f_DelayRyoIsu == 1 ){					// �W���[�i���󎚒�
//					break;
//				}
//				if( OPECTL.f_DelayRyoIsu == 2 && ryo_isu_zumi == 1 ){ // ���V�[�g�󎚒�
//					break;
//				}
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
				if( OPECTL.f_RctErrDisp == 1 ){						// �̎��ؔ��s���s�ē��\����
					break;
				}
				if( dummyNotce_Wait == 1 ) {						// �󎚏��������҂�
					ret = 10;										// �ҋ@��
				}
				break;
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���Z�������̗̎��ؔ��s���ɑҋ@��Ԃɖ߂�Ȃ��悤�ɂ���j
			default:
				break;
		}
// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X�i���Z�I�������C���j
//		// �ޑK�̕��o�����������Ŕ����悤�Ƃ�����A���o������������܂ő҂�
//		if(( ryo_en == 0 )&&( ret )){								// ���o������?
//			ret_bak = ret;											// Yes..Save
//			ret = 0;
//		}
		if( ret ){
			if( ryo_en != 0 ){
				if( idle_req_flg == 0 ){
					// ���Z�ē��I���ʒm
					lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
					// ���Z�ē��I�������҂�
					idle_req_flg = 1;
					// �����҂��^�C�}�[�J�n
					Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				}
			}
// MH810100(S) K.Onodera 2020/03/02 #3964 �̎��؂��󎚂��Ȃ�
			else{
				ret_bak = ret;
			}
// MH810100(E) K.Onodera 2020/03/02 #3964 �̎��؂��󎚂��Ȃ�
			ret = 0;
		}
		// ���Z�ē��I��������M�ς�
		if( idle_req_flg == 2 ){
			ret = 10;	// �I����
		}
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���Z�������̗̎��ؔ��s���ɑҋ@��Ԃɖ߂�Ȃ��悤�ɂ���j
		// �󎚊֘A�������Ŕ����悤�Ƃ�����A��������܂ő҂�
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//		if( ret == 10 && IS_INVOICE ){								// �C���{�C�X
		if( ret == 10 ){
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
			if( OPECTL.f_DelayRyoIsu == 1 ||						// �W���[�i���󎚒�
				OPECTL.f_RctErrDisp == 1 ||							// �̎��ؔ��s���s�ē��\����
				(OPECTL.f_DelayRyoIsu == 2 && ryo_isu_zumi == 1) ){ // ���V�[�g�󎚒�
				dummyNotce_Wait = 1;								// �󎚏��������҂�
				ret = 0;											// �����p��
			}
		}
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���Z�������̗̎��ؔ��s���ɑҋ@��Ԃɖ߂�Ȃ��悤�ɂ���j
// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X�i���Z�I�������C���j
	}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
	ope_anm( AVM_STOP );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
	Lagcan( OPETCBNO, 10 );											// ���p�\�}�̻��د��\���p��ϰSTOP
// MH321800(S) G.So IC�N���W�b�g�Ή�
	Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );						// ec���T�C�N���\���p��ϰؾ��
	Lagcan( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT );					// �J�[�h�������҂���ϰؾ��
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(S) MH364301 �C���{�C�X�Ή�
	LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);							// �W���[�i���󎚊����҂��^�C�}
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
	Lagcan( OPETCBNO, TIMERNO_RPT_PRINT_WAIT );						// ���V�[�g�󎚊����҂��^�C�}���Z�b�g
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
	LagCan500ms(LAG500_RECEIPT_MISS_DISP_TIMER);					// �̎��؎��s�\���^�C�}
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X�i���Z�I�������C���j
	Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );
// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X�i���Z�I�������C���j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	LagCan20ms( LAG20_JNL_PRI_WAIT_TIMER );							// �W���[�i���󎚊����҂��^�C�}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
	Lagcan(OPETCBNO, TIMERNO_QR_RECEIPT_DISP);						// QR�R�[�h���s�ē��\���^�C�}�[
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
	if( OPECTL.RECI_SW == 1 ){										// �̎������ݎg�p?
		OPECTL.RECI_SW = 0;											// �̎������ݖ��g�p
	}else{
// MH810105(S) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
		// �Q�[�g���Ɠ��l�ɐ��Z�����W�v�̃^�C�~���O�𑁂߂����߁A
		// �����ł̐��Z�����W�v��������߂�
//		if( !(uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )				// ���Z�����ʐM�d�����M�^�C�~���O=�ҋ@�֖߂鎞
//		{
//			PayData_set( 0, 0 );									// 1���Z����
//			kan_syuu();												// ���Z�����W�v
//		}
// MH810105(E) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
		OPECTL.RECI_SW = (char)-1;									// �ҋ@���̎������݉�
		if(OPECTL.Pay_mod == 2){									// �C�����Z��
			OPECTL.RECI_SW = 0;
		}
	}
	// ���Ɍv�Z�������i�^�C���A�E�g�j�Ŕ������ꍇ�Ȃǂ̏W�v
// MH810105(S) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
//	if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )					// ���Z�����ʐM�d�����M�^�C�~���O=���Z��������
// MH810105(E) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
	{
		if( !OPECTL.f_KanSyuu_Cmp ){								// �����W�v�����{
			PayData_set( 0, 0 );									// 1���Z����
			kan_syuu();												// ���Z�����W�v
		}
	}
	// FT�͓r���̉�ʂ̍X�V���͂Ȃ��̂ŁA��ʂ𔲂���Ōゾ���\���`�F�b�N����
	PayDataErrDispClr();
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//	Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );					// ���K�Ǘ����O�f�[�^�쐬
//	Log_regist( LOG_MONEYMANAGE_NT );								// ���K�Ǘ����O�o�^
	if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {			// ���K�Ǘ����O�f�[�^�쐬
		Log_regist( LOG_MONEYMANAGE_NT );							// ���K�Ǘ����O�o�^
	}
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)

	if(OPECTL.RECI_SW != (char)-1){
	if(Flap_Sub_Flg == 1){
		if(Flap_Sub_Num < 9){
			buf_size = (uchar)(9 - Flap_Sub_Num);
			memset(&FLAPDT_SUB[Flap_Sub_Num],0,sizeof(flp_com_sub));			// �ڍג��~�ر(���Z����)�ر
			memmove(&FLAPDT_SUB[Flap_Sub_Num],&FLAPDT_SUB[Flap_Sub_Num+1],sizeof(flp_com_sub)*buf_size);
		}
		if(Flap_Sub_Num == 10){
			memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));							// �I�[�ڍג��~�ر�ر
		}else{
			memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// �I�[�ڍג��~�ر�ر
		}
		Flap_Sub_Flg = 0;														// �ڍג��~�ر�g�p�t���OOFF
	}
	}

	cn_stat( 8, 0 );	// �ۗL�������M

	f_MandetMask = 1;

	e_inflg = 0;													// ����̓����́u�d�q�}�l�[�v1:�ł���0:�Ȃ� ���N���A
	e_incnt = 0;													// ���Z���́u�d�q�}�l�[�v�g�p�񐔁i�܂ޒ��~�j���N���A
	e_pay = 0;														// �d�q�}�l�[���ϊz���N���A
	e_zandaka = 0;													// �d�q�}�l�[�c�����N���A
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	q_inflg = 0;													// ����̓����́uQR�R�[�h�v1:�ł���0:�Ȃ� ���N���A
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	if( ryo_stock == 1 ){											// �󎚃f�[�^�X�g�b�N��
		MsgSndFrmPrn(PREQ_STOCK_CLEAR, R_PRI, 0);					// �󎚃f�[�^�N���A
	}
	ryo_stock = 0;													// �X�g�b�N�Ȃ�
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( Led_status[0] || Led_status[1] || Led_status [2] )			// LED�̂ǂꂩ���_���܂��͓_�ł��Ă����ꍇ
//		EDY_LED_OFF();												// LED����
//	if( Edy_Rec.edy_status.BIT.CTRL )								// Edy�̏�Ԃ����޾ݽ�\��ԂȂ�
//		Edy_SndData04();											// ���ތ��m��~�w�����M
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	if(OPECTL.Pay_mod == 2){										// �C�����Z��
		OPECTL.Pay_mod = 0;
		wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
		OPECTL.Kakari_Num = 0;										// �W�����ر
	}
	// �O�̈׃t���O�N���A���������Ă���
	Cnm_End_PayTimOut = 0;

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	// ���Z������A�N�����ɖ߂��H
	if( Discon_flg == 1 ){			// �I����A�N�����ɑJ��
		OPECTL.Ope_mod = 255;		// op_init00(�N����)
		ret = -1;
	}else if( Discon_flg == 2 ){	// �I����A�ҋ@�ɑJ�ځH
		ope_idle_transit_common();	// �ҋ@��ԑJ�ڋ��ʏ���
		ret = -1;
	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �C�����Z���Ԉʒu�ԍ����͏���                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod11( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = �������֐؊�                              |*/
/*|                       : -1 = Ӱ�ސؑ�                                  |*/
/*|                       : 10 = ���(���~)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod11( void )
{
	short	ret;
	short	ans;
	ushort	msg = 0;
	ushort	inp_lin = 2;											// ���,���Ԉʒu�ԍ��̓��͍s
	char	setwk;

	Suica_Rec.Data.BIT.PAY_CTRL = 0;
	OPECTL.MOp_LokNo = 0L;											// �ԈႦ���ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)�ر
	OPECTL.MPr_LokNo = 0;											// �ԈႦ�����������p���Ԉʒu�ԍ�(1�`324)�ر
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// �����^�C���A�E�g�t���O�N���A
	Suica_rcv_split_flag_clear();

	Syusei_Select_Flg = 0;

	key_num = 0;													// �����͒l�ر
	if( CPrmSS[S_TYP][81] ){										// ���g�p�ݒ�L��?
		key_sec = 1;												// ����[�`]�Ƃ���(���g�p����)
	}else{
		key_sec = 0;												// �����͒l�ر(���g�p���Ȃ�)
	}
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	OpeLcd( 10 );													// �C�����Z���Ԉʒu�ԍ����͉��

	rd_shutter();													// ���Cذ�ް���������
	OPE_red = 2;													// ذ�ް�����r�o

	ope_anm( AVM_STOP );											// ������~�ųݽ

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( Edy_Rec.edy_status.BIT.INITIALIZE ){						// EdyӼޭ�ق̏��������������Ă���H
//		Edy_StopAndLedOff();										// ���ތ��m��~��LEDOFF�J�n
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 0 ){							// key OFF
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					OPECTL.Pay_mod = 0;								// ���Z�̎��(�ʏ퐸�Z)
					wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
					OPECTL.Kakari_Num = 0;									// �W�����ر
					ret = -1;
				}
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
				BUZPI();
				key_num = (( key_num % knum_ket ) * 10 )+( msg - KEY_TEN );
				opedsp( inp_lin, 22, key_num, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// ���Ԉʒu�ԍ��\��������
				break;
			case KEY_TEN_CL:										// �ݷ�[���] �����
				BUZPI();
#if	UPDATE_A_PASS
				if( !CPrmSS[S_KOU][1] ){							// ������X�V���Z�Ȃ�?
#endif	// UPDATE_A_PASS
					if( key_sec == 0 )	break;
					if( key_sec > 0 )	key_sec++;
					if( key_sec > CPrmSS[S_TYP][81] )	key_sec = 1;
					grachr( inp_lin, 20, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );	// ���\���؊�
#if	UPDATE_A_PASS
				}
#endif	// UPDATE_A_PASS
				break;
			case KEY_TEN_F1:										// �ݷ�[���Z] �����
				BUZPI();
				if( inp_lin == 2 ){									// �ԈႦ�����Ԉʒu�ԍ�����
					ans = carchk( key_sec, key_num, 1 );			// ���Ԉʒu�ԍ�����
					if(( ans != 0 )||								// OK�ȊO or
					   ( OPECTL.MOp_LokNo == OPECTL.Op_LokNo )){	// ���͂����ڋq�p���Ԉʒu�ԍ�������
						ope_anm( AVM_BTN_NG );						// �Ԏ��ԍ��ԈႢ���ųݽ
						key_num = 0;
						opedsp( inp_lin, 22, key_num, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	// ���Ԉʒu�ԍ��\��������
						break;
					}
					if( ans == 0 ){									// OK
						OPECTL.MOp_LokNo = OPECTL.Op_LokNo;			// �ԈႦ���ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)���
						OPECTL.MPr_LokNo = OPECTL.Pr_LokNo;			// �ԈႦ�����������p���Ԉʒu�ԍ�(1�`324)���
						grachr( inp_lin, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );	// �Ԉ�������\���؊�
						opedsp( inp_lin, 22, key_num, knum_len, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );			// �Ԉ�������Ԉʒu�ԍ��\��������
						inp_lin = 3;								// ���������Ԉʒu�ԍ��֐؊�
						if( CPrmSS[S_TYP][81] ){					// ���g�p�ݒ�L��?
							key_sec = 1;							// ����[�`]�Ƃ���(���g�p����)
							grachr( inp_lin, 20, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );// ���������\���؊�
						}else{
							key_sec = 0;							// �����͒l�ر(���g�p���Ȃ�)
						}
						key_num = 0;
						opedsp( inp_lin, 22, key_num, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );			// ���������Ԉʒu�ԍ��\��������
					}
				}else{
					ans = carchk( key_sec, key_num, 0 );			// ���Ԉʒu�ԍ�����
					setwk = (char)Carkind_Param(FLP_ROCK_INTIME, (char)(LockInfo[OPECTL.Pr_LokNo-1].ryo_syu),1,1);
					if(( ans == -1 )||( ans == 10 )||				// ���Z�s�\ or ���ݒ� or
					   ( ans == 2 )||(( ans == 1 ) && ( setwk == 0 ))||	// �o�ɏ�� or ۯ����u�J�n�҂� and ۯ����u�J�n��ϰ�N�����ɐ��Z���Ȃ��ݒ� or
					   ( OPECTL.MOp_LokNo == OPECTL.Op_LokNo )){	// ���͂����ڋq�p���Ԉʒu�ԍ�������
						ope_anm( AVM_BTN_NG );						// �Ԏ��ԍ��ԈႢ���ųݽ
						key_num = 0;
						opedsp( inp_lin, 22, key_num, knum_len, 1, 1 , COLOR_BLACK, LCD_BLINK_OFF);	// ���Ԉʒu�ԍ��\��������
						break;
					}
					/*** ���o�Ɏ��������� ***/
					if( set_tim( OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){	// ���o�Ɏ���NG?(Y)
						ope_anm( AVM_BTN_NG );						// �Ԏ��ԍ��ԈႢ���ųݽ
						key_num = 0;
						opedsp( inp_lin, 22, key_num, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	// ���Ԉʒu�ԍ��\��������
						break;
					}
					OPECTL.Ope_mod = 12;								// �C�����Z�m�F�\����
					ret = 1;
				}
				break;
			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				if(( key_num == 0 )&&( inp_lin == 2 )){
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
					OPECTL.Kakari_Num = 0;										// �W�����ر
					ret = -1;
					break;
				}
				inp_lin = 2;										// �ԈႦ�����Ԉʒu�ԍ�����
				OPECTL.MOp_LokNo = 0L;								// �ԈႦ���ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)�ر
				OPECTL.MPr_LokNo = 0;								// �ԈႦ�����������p���Ԉʒu�ԍ�(1�`324)�ر
				key_num = 0;										// �����͒l�ر
				if( CPrmSS[S_TYP][81] ){							// ���g�p�ݒ�L��?
					key_sec = 1;									// ����[�`]�Ƃ���(���g�p����)
					grachr( 2, 20, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );	// ���\��
					opedsp( 2, 22, 0, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );				// ���Ԉʒu�ԍ�0�\��
					grachr( 3, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ 1 * 2 ] );		// ���\��
					opedsp( 3, 22, 0, knum_len, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );				// ���Ԉʒu�ԍ�0�\��
				}else{
					key_sec = 0;									// �����͒l�ر(���g�p���Ȃ�)
					opedsp( 2, 22, 0, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );				// ���Ԉʒu�ԍ�0�\��
					opedsp( 3, 22, 0, knum_len, 1, 0 , COLOR_BLACK, LCD_BLINK_OFF);				// ���Ԉʒu�ԍ�0�\��
				}
				break;
			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
			case KEY_TEN_F3:										// �ݷ�[��t��] �����
			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
				BUZPI();
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					Ope2_WarningDispEnd();							// �װ�\������
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			default:
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �C�����Z�����\��(�Ԉ�������Ԉʒu�̗��� > ���������Ԉʒu�̗���)        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod12( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 10=���~  1=�߂��Ȃ�  2=�d�ݖ߂�                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*[]----------------------------------------------------------------------[]*/
/*| �C�����Z�����\��(�m�F�̂��ߏC�����Ԏ��̏���\��)                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod12( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret :                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

short	op_mod12( void )
{
	short	ret;
	ushort	msg = 0;
	if( prm_get(COM_PRM, S_TYP, 98, 1, 1) == 2 ){					// �C�������������z�̕��ߑI��
		Syusei_Select_Flg = 1;
	}

	OpeLcd( 11 );													// �C�����Z�����\��(�၄��)

	OPE_red = 2;													// ذ�ް�����r�o

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 0 ){							// key OFF
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					OPECTL.Pay_mod = 0;								// ���Z�̎��(�ʏ퐸�Z)
					wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
					OPECTL.Kakari_Num = 0;										// �W�����ر
					ret = -1;
				}
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
			case KEY_TEN_F3:										// �ݷ�[��t��] �����
			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
			case KEY_TEN_CL:										// �ݷ�[C] �����
				BUZPI();
				break;
			case KEY_TEN_F1:										// �ݷ�[���Z] �����
				BUZPI();
				if( prm_get(COM_PRM, S_TYP, 98, 1, 1) == 2 ){		// �C�������������z�̕��ߑI��
					Syusei_Select_Flg = 2;							// ���߂�����
				}
				SyuseiDataSet( OPECTL.MPr_LokNo );					// 
				ope_SyuseiStart( OPECTL.Pr_LokNo );					// �V�K�֐�
				queset(OPETCBNO, SYU_SEI_SEISAN, 0, NULL);			// �C�����Z�J�n�C�x���g�Z�b�g
				OPECTL.Ope_mod = 2;									// �����E����������
				ret = 1;
				break;
			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				ret = 10;
				break;
			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
				BUZPI();
				if( prm_get(COM_PRM, S_TYP, 98, 1, 1) == 2 ){		// �C�������������z�̕��ߑI��
					SyuseiDataSet( OPECTL.MPr_LokNo );				// 
					ope_SyuseiStart( OPECTL.Pr_LokNo );				// �V�K�֐�
					queset(OPETCBNO, SYU_SEI_SEISAN, 0, NULL);		// �C�����Z�J�n�C�x���g�Z�b�g
					Syusei_Select_Flg = 1;							// ���߂��Ȃ�
					OPECTL.Ope_mod = 2;								// �����E����������
					ret = 1;
				}
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			default:
				break;
		}
	}
	return( ret );
}



#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
//----------------------------------------------------------------------------
// �V�C�����Z�@�\�ǉ�(�V�C�����Z�ł͂��̊֐��͎g��Ȃ�)
//----------------------------------------------------------------------------
/*[]----------------------------------------------------------------------[]*/
/*| �C�����Z��������(�Ԉ�������Ԉʒu�̗��� < ���������Ԉʒu�̗���)        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod13( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 10=���~  1=�߂��Ȃ�  2=�d�ݖ߂�                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod13( void )
{
	short	ret;
	ushort	msg = 0;
	ushort	loklen, Mloklen;
	ushort	wk_MsgNo;
	delay_count = 0;												// ������������ү���޶��Ă̸ر
	memset( nyukin_delay,0,sizeof( nyukin_delay ));					// ������������ү���ނ�ێ��̈�̸ر
	dsp_change = 0;
    edy_dsp.BYTE = 0;												// Edy��ʕ\���p�̈揉����
	cansel_status.BYTE = 0;											// ������̓d�q�}�̒�~��ԏ�����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Pay_Work = 0;												// Edy���ϊzܰ��̈�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	CCT_Cansel_Status.BYTE = 0;										// �ڼޯĶ��ގg�p���̊e�d�q�}�̂̒�~��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// �����^�C���A�E�g�t���O�N���A
	Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	time_out = 0;
	Ope_Last_Use_Card = 0;											// �Ō�ɗ��p�����J�[�h������
	w_settlement = 0;											// Suica�c���s�����̐��Z�z�i�O���͎c���s���łȂ����Ӗ�����j

	OpeLcd( 12 );													// �C�����Z�����\��(�၃��)

	OPECTL.op_faz = 0;												// ���ڰ���̪���
	OPECTL.RECI_SW = 0;												// �̎������ݖ��g�p
	RECI_SW_Tim = 0;												// �̎��؎�t���Ըر
	OPECTL.CAN_SW = 0;												// �������
	ryo_buf.dsp_ryo -= ryo_buf.mis_ryo;								// �\�����������Z���ߕ\���������ԈႢ�������������z�Ƃ���B

	cn_stat( 1, 2 );												// ���� & COIN������

	nmicler( &SFV_DAT.refval, 8 );									// �s���z�ر

	OPE_red = 3;													// ���Z��
	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ذ�޺����(���Ԍ��ۗ���)
	}

	if( MIFARE_CARD_DoesUse ){										// Mifare���L���ȏꍇ
		op_MifareStart();											// Mifare��L����
	}

	read_sht_opn();													// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
	OPECTL.coin_syukei = 0;											// ��݋��ɏW�v���o�͂Ƃ���
	OPECTL.note_syukei = 0;											// �������ɏW�v���o�͂Ƃ���

	dsp_fusoku = 0;
	suica_fusiku_flg = 0;									

	for( ret = 0; ret == 0; ){
		msg = GetMessage();

		if( Ope_MiryoEventCheck( msg ) == 1 )
			continue;

		if( Ope_ArmClearCheck( msg ) == 1 )
			continue;

		switch( msg ){
			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
				BUZPI();
				break;

			case COIN_EVT:											// Coin Mech event
				if( OPECTL.op_faz == 4 ){							// ��ѱ��
					if( OPECTL.CN_QSIG == 7 ){						// ���o��
//						ret = 10;									// �ҋ@�ɖ߂�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						if( cansel_status.BYTE == 0x0d ){				// �I���ð���FSuica&Edy�I���ς݂̏ꍇ
						if( cansel_status.BYTE == 0x05 ){				// �I���ð���FSuica�I���ς݂̏ꍇ
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
							ret = 10;									// �ҋ@�ɖ߂�
						}else{											// Suica���܂��I�����Ă��Ȃ��ꍇ
							if( SUICA_USE_ERR ){
								cansel_status.BIT.SUICA_END = 1;		// Suica�I���ς��׸ނ��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//								if( cansel_status.BYTE == 0x0d ){		// �I���ð���FSuica&Edy�I���ς݂̏ꍇ	
								if( cansel_status.BYTE == 0x05 ){		// �I���ð���FSuica�I���ς݂̏ꍇ	
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
									ret = 10;							// �ҋ@�ɖ߂�
									break;
								}
							}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//							if( EDY_USE_ERR ){							// Edy�֘A�̴װ��������
//								cansel_status.BIT.EDY_END = 1;			// Edy�I���ς��׸ނ��
//								if( cansel_status.BYTE == 0x0d ){		// �I���ð���FSuica&Edy�I���ς݂̏ꍇ
//									ret = 10;							// �ҋ@�ɖ߂�
//									break;
//								}
//							}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
							cansel_status.BIT.CMN_END = 1;				// ү��I���ς��׸ނ��
						}
						break;
					}else{
						OPECTL.op_faz = 0;
						cn_stat( 3, 2 );							// ������
					}
				}
			case NOTE_EVT:											// Note Reader event
			case KEY_TEN_F4:										// �ݷ�[���] �����
				if( msg == KEY_TEN_F4 ){							// �������?(Y)
					if( !Suica_Rec.suica_err_event.BYTE && (!Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL) ) 	// �����ް����M��A�׸ނ��X�V�����O�Ɏ�����L�[�������ꂽ�ꍇ
						break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					if( dsp_fusoku || edy_dsp.BIT.edy_dsp_Warning ){
					if( dsp_fusoku ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) G.So IC�N���W�b�g�Ή�
						if (isEC_USE() != 0) {
							Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );
						} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
						Lagcan( OPETCBNO, 10 );
// MH321800(S) G.So IC�N���W�b�g�Ή�
						}
// MH321800(E) G.So IC�N���W�b�g�Ή�
					}
					if( OPECTL.op_faz == 9 ) {
						break;
					}
					BUZPI();
					Cansel_Ele_Start();
				}
				if(( OPECTL.op_faz == 0 )&&( msg != KEY_TEN_F4 )){
					OPECTL.op_faz = 1;								// ������
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
				}
				ret = in_mony( msg, 1 );							// �����������
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				if( OPECTL.op_faz > 2 ){							// ���Z���� Ų?
					opr_snd( 2 );									// �O�r�o
				}else if( ryo_buf.zankin == 0 ){					//
					break;
				}else{
//					if(( OPECTL.op_faz == 0 )&&( r_zero != 0 )){	//
					if( OPECTL.op_faz == 0 ){
						Lagcan( OPETCBNO, 1 );						// ��ϰ1ؾ��(���ڰ��ݐ���p)
					}
					ret = al_card( msg , 1 );						// ���ޏ���
				}
				break;

			case ARC_CR_E_EVT:										// �I������ގ�M
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������

				// �̎��؍ĕ\��
				if( OPECTL.RECI_SW == 1 ){							// �̎������ݎg�p?
					wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
					grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);		// "      �̎��؂𔭍s���܂�      "
				}else{
					grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);			// " �K�v�ȕ��͗̎��������ĉ����� "
				}
				break;

			case IBK_MIF_A2_OK_EVT:									// Mifare�ް��Ǐo������
				if(( OPECTL.op_faz > 2 )||( ryo_buf.zankin == 0 )){	// ���Z���ł͖��� or �c�z0�~
					break;
				}else{
					if( OPECTL.op_faz == 0 ){						//
						Lagcan( OPETCBNO, 1 );						// ��ϰ1ؾ��(���ڰ��ݐ���p)
					}
					ret = al_iccard( msg , 0 );						// IC���ޏ���
				}
				break;

			case IBK_MIF_A2_NG_EVT:									// Mifare�ް��Ǐo��NG
				if( MIF_ENDSTS.sts1 == 0x30 ){						// ���ޖ���
					// �̎��؍ĕ\��
					if( OPECTL.RECI_SW == 1 ){						// �̎������ݎg�p?
						wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
						grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      �̎��؂𔭍s���܂�      "
					}else{
						grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);		// " �K�v�ȕ��͗̎��������ĉ����� "
					}
				}
				break;

			case TIMEOUT1:
				time_out = 1;									// ��ѱ�Ĕ����׸�ON
				cansel_status.BIT.INITIALIZE = 1;				// �d�q�}�̒�~����

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( OPECTL.op_faz == 9 || Suica_Rec.Data.BIT.CTRL || Edy_Rec.edy_status.BIT.CTRL ){		// ������������s���̏ꍇ
				if( OPECTL.op_faz == 9 || Suica_Rec.Data.BIT.CTRL ){		// ������������s���̏ꍇ
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
					if( Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL ){	// Suica���L�����Ō�ɑ��M�����̂���t���̏ꍇ
						Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
					}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					if( Edy_Rec.edy_status.BIT.CTRL )
//						Edy_StopAndLedOff();
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

					LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 20, Op_ReturnLastTimer );	// �c�z�\���p��ϰ���ꎞ�I�Ɏg�p����
				}
				if( OPECTL.op_faz >= 3 ){
					if( carduse() ){								// ���ގg�p?
						chu_isu();									// ���~����
						svs_renzoku();								// ���޽���A���}��
					}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					if( cansel_status.BYTE == 0x0d ){				// �I���ð���FSuica&Edy�I���ς݂̏ꍇ
					if( cansel_status.BYTE == 0x05 ){				// �I���ð���FSuica�I���ς݂̏ꍇ
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
						ret = 10;									// �ҋ@�ɖ߂�
					}else{											// Suica���܂��I�����Ă��Ȃ��ꍇ
						if( SUICA_USE_ERR ){
							cansel_status.BIT.SUICA_END = 1;		// Suica�I���ς��׸ނ��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//							if( cansel_status.BYTE == 0x0d ){		// �I���ð���FSuica&Edy�I���ς݂̏ꍇ	
							if( cansel_status.BYTE == 0x05 ){		// �I���ð���FSuica�I���ς݂̏ꍇ	
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
								ret = 10;							// �ҋ@�ɖ߂�
								break;
							}
						}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						if( EDY_USE_ERR ){							// Edy�֘A�̴װ��������
//							cansel_status.BIT.EDY_END = 1;			// Edy�I���ς��׸ނ��
//							if( cansel_status.BYTE == 0x0d ){		// �I���ð���FSuica&Edy�I���ς݂̏ꍇ
//								ret = 10;							// �ҋ@�ɖ߂�
//								break;
//							}
//						}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
						cansel_status.BIT.CMN_END = 1;				// ү��I���ς��׸ނ��
					}
					break;
				}
				cn_stat( 2, 2 );									// �����s��
				OPECTL.op_faz = 4;									// ���ڰ���̪��� ��ѱ��
				Lagcan( OPETCBNO, 1 );								// ��ϰ1ؾ��(���ڰ��ݐ���p)
				Lagtim( OPETCBNO, 2, 10*50 );						// ү��Ď���ϰ�N��
				break;

			case TIMEOUT2:											// ү��Ď���ϰ��ѱ��
				switch( OPECTL.op_faz ){
					case 2:											// ����
						if( ryo_buf.turisen ){						//
							SFV_DAT.reffal = ryo_buf.turisen;		//
							ret = 3;								// ���Z����������
						}else{
							ret = 1;								// ���Z����������
						}
						break;
					case 3:											// ���~
					case 4:											// ��ѱ��
					case 9:											// ��ѱ��
						if( ryo_buf.nyukin ){						//
							ryo_buf.turisen = ryo_buf.nyukin;		//
							if( SFV_DAT.reffal ){					// �s������?
								ryo_buf.fusoku += SFV_DAT.reffal;	// �x�����s���z
							}
							if( SFV_DAT.nt_escrow ){				// �����߂��װ
								ryo_buf.fusoku += 1000L;			// �x�����s���z
							}
							chu_isu();								// ���~�W�v
							Lagtim( OPETCBNO, 1, 10*50 );			// ��ϰ1�N��(���ڰ��ݐ���p)
						}else{
							if( !Suica_Rec.suica_err_event.BYTE ){					// Suica�װ���������Ă��Ȃ�
								if( Suica_Rec.Data.BIT.CTRL ){
									Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
									OPECTL.op_faz = 9;
									LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 20, Op_ReturnLastTimer );	// �c�z�\���p��ϰ���ꎞ�I�Ɏg�p����
									break;
								}
							}
							ret = 10;								// �ҋ@�ɖ߂�
						}
						break;
					default:
						ret = 10;									// �ҋ@�ɖ߂�
						break;
				}
				break;
			case TIMEOUT11:											// Suica�u���ޔF�شװ/���Z�����v�����҂�����ѱ�Ēʒm
				Ope_TimeOut_11( OPECTL.Ope_mod,  0 );
				break;

			case TIMEOUT10:											// �uSuica��~��̎�t�����M�҂��vor�u������ʕ\���ؑցv����ѱ�Ēʒm
				Ope_TimeOut_10( OPECTL.Ope_mod,  0 );
				break;

// MH321800(S) G.So IC�N���W�b�g�Ή�
			case EC_CYCLIC_DISP_TIMEOUT:							// �uec��~��̎�t�����M�҂��vor�u������ʕ\���ؑցv����ѱ�Ēʒm
				Ope_TimeOut_Cyclic_Disp( OPECTL.Ope_mod,  0 );
				break;

			case EC_RECEPT_SEND_TIMEOUT:							// ec�u���ޔF�شװ/���Z�����v�����҂�����ѱ�Ēʒm�
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,  0 );
				break;
// MH321800(E) G.So IC�N���W�b�g�Ή�

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			case IBK_EDY_RCV:															// EdyӼޭ�ق���̎�M�ް��H
//				ret = Ope_Edy_Event( msg, OPECTL.Ope_mod );
//				break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			default:
				break;
		}
	}

	OPECTL.Fin_mod = ret;
	if( ret == 10 ){
		_di();
		CN_escr_timer = 0;											// ������ϰ��ݾ�
		_ei();
		safecl( 7 );												// ���ɖ����Z�o
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//		Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );	// ���~���̋��K�Ǘ����O�f�[�^�쐬
//		Log_regist( LOG_MONEYMANAGE_NT );							// ���K�Ǘ����O�o�^
		if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {	// ���~���̋��K�Ǘ����O�f�[�^�쐬
			Log_regist( LOG_MONEYMANAGE_NT );						// ���K�Ǘ����O�o�^
		}
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
//		if( FLAPDT.flp_data[OPECTL.MPr_LokNo-1].mode != FLAP_CTRL_MODE4 ){		// ���Ԓ��łȂ�?
//			queset( FLPTCBNO, FLAP_DOWN_SND_SS, sizeof( OPECTL.MPr_LokNo ), &OPECTL.MPr_LokNo );	// �C�����Z�pۯ����u�J(�ׯ�߉��~)
//		}
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
	}else{
		OPECTL.Ope_mod = 14;										// �C�����Z���Z������
	}

	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 2 );											// ��ϰ2ؾ��(ү��Ď��p)
	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );				// Suica�֘A��ϰ�ر

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �C�����Z����                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod14( void                                          |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = ���튮��                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod14( void )
{
	short	ret;
	short	ryo_en;
	short	safe_que = 0;
	ushort	msg = 0;

	Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( PayData.Electron_data.Edy.e_pay_kind != EDY_USED ){
//		Edy_StopAndLedOff();
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	if( MIFARE_CARD_DoesUse ){										// Mifare���L���ȏꍇ
		op_MifareStop_with_LED();								 	// Mifare���~����
	}
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
//	queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof( OPECTL.Pr_LokNo ), &OPECTL.Pr_LokNo );	// ���Z����������(�ׯ�߰���~)
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
	IoLog_write(IOLOG_EVNT_AJAST_FIN, (ushort)LockInfo[OPECTL.Pr_LokNo - 1].posi, 0, 0);

	OPE_red = 2;													// ذ�ް�����r�o

	rd_shutter();													// ���Cذ�ް���������

	ryo_en = 0;														// �����ݕs��

	switch( OPECTL.Fin_mod ){
		case 0:														// ���z?
			ac_flg.cycl_fg = 73; 									// �i���j�����o������
			ryo_en = 1;
			OpeLcd( 13 );											// �C�����Z�����\��(���z)
			break;

		case 1:														// �ނ薳��?
		default:
			ac_flg.cycl_fg = 73; 									// �i���j�����o������
			ryo_en = 1;
			OpeLcd( 14 );											// �C�����Z�����\��(�ޖ���)
			break;

		case 2:														// �ނ�L��?
		case 3:														// ү��װ(�ނ�L��)?
			OpeLcd( 15 );											// �C�����Z�����\��(�ޗL��)
			if(( OPECTL.Fin_mod == 3 )||
			   (( OPECTL.Fin_mod == 2 )&&
			    ( refund( (long)ryo_buf.turisen ) != 0 ))){			// ���o�N��
				/*** ���oNG ***/
				ac_flg.cycl_fg = 73;								// �i���j�����o������
				ryo_en = 1;											// �����݉�
				ryo_buf.fusoku = SFV_DAT.reffal;					// �x�����s���z
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[29] );					// "���ߕs���z                  �~"
				opedpl( 3, 16, ryo_buf.fusoku, 6, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
			}
			else{
				if( (CN_refund & 0x01) == 0 ){
					// ��݂̕����o�����h�Ȃ��h�̂Ƃ�
					cn_stat( 6, 0 );									// ������ر(���ۺ�݂𗎂Ƃ�)
				}
			}
			LedReq( CN_TRAYLED, LED_ON );							// ��ݎ�o�����޲��LED�_��
			break;
	}

	cn_stat( 4, 1 );												// �����捞

	Lagtim( OPETCBNO, 1, 10*50 );									// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 2 );											// Timer Cancel

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case COIN_EVT:											// Coin Mech event
				ryo_en = 1;											// �����݉�
				if( OPECTL.CN_QSIG == 2 ){							// ���o����
					ac_flg.cycl_fg = 73;							// �����o������
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					if( SFV_DAT.reffal ){
						ryo_buf.fusoku += SFV_DAT.reffal;			// �x�����s���z
					}
				}
				else if( OPECTL.CN_QSIG == 9 ){						// ���o�װ
					ac_flg.cycl_fg = 73;							// �����o������(��)
					refalt();										// �s�����Z�o
					ryo_buf.fusoku += SFV_DAT.reffal;				// �x�����s���z
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
				}
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
				BUZPI();
			case TIMEOUT1:											// Time Over ?
				if( ryo_en == 0 ){
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
//					ryo_en = 1;	���o��10�b�ȏォ����Ɣ����Ă��܂�
					break;
				}
				if( SFV_DAT.safe_cal_do != 0 ){						// ���Ɍv�Z���I��?
					if( msg == TIMEOUT1 ){
					safe_que++;
					if( safe_que < 10 ){
						Lagtim( OPETCBNO, 1, 1*50 );				// ��ϰ1(1s)�N��(���ڰ��ݐ���p)
						break;
					}
					ex_errlg( ERRMDL_MAIN, ERR_MAIN_NOSAFECLOPERATE, 2, 0 );	// ���ɖ����Z�o�Ȃ��ɔ�����
					}else{
						break;
					}
				}
				ret = 10;
				break;

			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
				BUZPI();
				break;

			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				if(( ryo_en != 0 )&&( SFV_DAT.safe_cal_do == 0 )){	// ���o�����ŋ��Ɍv�Z�I���Ȃ�
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
					ret = 10;
				}
				break;

			default:
				break;
		}
	}
	syu_syuu();														// �C�����Z�����W�v
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//	Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );					// ���K�Ǘ����O�f�[�^�쐬
//	Log_regist( LOG_MONEYMANAGE_NT );								// ���K�Ǘ����O�o�^
	if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {			// ���K�Ǘ����O�f�[�^�쐬
		Log_regist( LOG_MONEYMANAGE_NT );							// ���K�Ǘ����O�o�^
	}
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
	e_inflg = 0;													// ����̓����́u�d�q�}�l�[�v1:�ł���0:�Ȃ� ���N���A
	e_incnt = 0;													// ���Z���́u�d�q�}�l�[�v�g�p�񐔁i�܂ޒ��~�j���N���A
	e_pay = 0;														// �d�q�}�l�[���ϊz���N���A
	e_zandaka = 0;													// �d�q�}�l�[�c�����N���A

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( Led_status[0] || Led_status[1] || Led_status [2] )			// LED�̂ǂꂩ���_���܂��͓_�ł��Ă����ꍇ
//		EDY_LED_OFF();												// LED����
//	if( Edy_Rec.edy_status.BIT.CTRL )								// Edy�̏�Ԃ����޾ݽ�\��ԂȂ�
//		Edy_SndData04();											// ���ތ��m��~�w�����M
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	return( ret );
}

#endif		// SYUSEI_PAYMENT


#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| ������X�V ���}���҂�����                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod21( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = �������֐؊�                              |*/
/*|                       : -1 = Ӱ�ސؑ�                                  |*/
/*|                       : 10 = ���(���~)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	op_mod21( void )
{
	short	ret;
	short	ans;
	ushort	msg = 0;
	char	setwk;

	OpeLcd( 20 );													// ������X�V ���}���҂���ʕ\��

	OPECTL.multi_lk = 0L;											// ������Z�⍇�����Ԏ���
	OPECTL.ChkPassSyu = 0;											// ������⍇��������ر
	OPECTL.ChkPassPkno = 0L;										// ������⍇�������ԏꇂ�ر
	OPECTL.ChkPassID = 0L;											// ������⍇���������ID�ر
	OPECTL.PassNearEnd = 0;											// �����؂�ԋ��׸�
	OPECTL.PriUsed = 0;
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	OPE_red = 3;													// ���Z��
	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ذ�޺����(���Ԍ��ۗ���)
	}
	read_sht_opn();													// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��

	Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}else{
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
				}
				ret = -1;
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
			case KEY_TEN_F1:										// �ݷ�[���Z] �����
			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
			case KEY_TEN_F3:										// �ݷ�[��t��] �����
			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
				BUZPI();
				break;
			case KEY_TEN_CL:										// �ݷ�[�X�V] �����
				BUZPI();
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
				break;
			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				ret = 10;											// �ҋ@�֖߂�
				break;

			case ARC_CR_R_EVT:										// ����IN
				memset( &PayData, 0, sizeof( Receipt_data ) );		// ���Z���,�̎��؈��ް�
// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
				memset( &ntNet_56_SaveData, 0, sizeof(ntNet_56_SaveData) );	// ���Z�f�[�^�ێ�
// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
				memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET���Z�ް��t�����ر
				memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );	// ���Z�ް��������쐬�ر�ر
				memset( card_use, 0, sizeof( card_use ) );			// 1���Z���ޖ��g�p����ð���
				CardUseSyu = 0;										// 1���Z�̊�����ނ̌����ر
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
				if( ret = al_card_renewal( msg , 0 ) ){				// ���ޔ���
					OPECTL.Ope_mod = 22;							// ������X�V �����\��,����������
					set_tim2( &CLK_REC );							// ���Z�������
				}
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				// �װ�\���폜
				grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );					// "                              "
				OPECTL.PassNearEnd = 0;								// �����؂�ԋ�
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��
				ret = 10;
				break;

			case MID_RECI_SW_TIMOUT:								// �̎��ؔ��s�\��ϰ��ѱ�Ēʒm
				OPECTL.RECI_SW = 0;									// �̎������ݖ��g�p
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�V �����\��,��������                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod22( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret(OPECTL.Fin_mod)                                     |*/
/*|                  1 = ���Z����(�ނ薳��) ���Z���������֐؊�             |*/
/*|                  2 = ���Z����(�ނ�L��) ���Z���������֐؊�             |*/
/*|                  3 = ү��װ����         ���Z���������֐؊�             |*/
/*|                 10 = ���Z���~           �ҋ@�֖߂�                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	op_mod22( void )
{
	short	ret;
	short	r_zero = -1;
	ushort	msg = 0;
	short	err = 0;
	ushort	loklen;

	ac_flg.cycl_fg = 1;												// 1:�����\����
	OPECTL.op_faz = 0;												// ���ڰ���̪���
	OPECTL.RECI_SW = 0;												// �̎������ݖ��g�p
	RECI_SW_Tim = 0;												// �̎��؎�t���Ըر
	OPECTL.CAN_SW = 0;												// �������
	OPECTL.LastUsePCardBal = 0L;									// �Ō�ɗ��p��������߲�޶��ނ̎c�z
	OPECTL.PriUsed = 0;
	ryodsp = 0;
	OPECTL.f_KanSyuu_Cmp = 0;										// �����W�v�����{
	OpeNtnetAddedInfo.PayMethod = 5;								// ���Z���@=5:������X�V
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// �����^�C���A�E�g�t���O�N���A
	Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;
	Suica_Rec.Data.BIT.PAY_CTRL = 0;								// �d�q�}�̌��ύςݔ����׸޸ر���O�̂���
	Suica_rcv_split_flag_clear();

	cm27();															// �׸޸ر
	memset( &ryo_buf, 0, sizeof( struct RYO_BUF ) );				// �����ޯ̧�ر�
	ryo_buf.pkiti = 9999;											// �Ԏ�������а�Ƃ���9999�����Ă���
	ryo_buf.tyu_ryo = 
	ryo_buf.dsp_ryo = RenewalFee;									// �X�V����

	if( ryo_buf.nyukin ){
		OpeLcd( 4 );												// �����\��(�������͊�����)
	}else{
		OpeLcd( 21 );												// ������X�V �����\��(����)
	}
	if( ryo_buf.dsp_ryo == 0 ){										// �\������0�~
		r_zero = 0;													// 0�~���Z
		Lagtim( OPETCBNO, 1, 3*50 );								// ��ϰ1(3s)�N��(���ڰ��ݐ���p)
		OPE_red = 4;												// ���Z����
	}else{
		r_zero = -1;												// 0�~�ȊO���Z
				cn_stat( 1, 2 );									// ���� & COIN������
		nmicler( &SFV_DAT.refval, 8 );								// �s���z�ر

		rd_shutter();												// ���Cذ�ް���������
		Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][65]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
	}

	OPECTL.coin_syukei = 0;											// ��݋��ɏW�v���o�͂Ƃ���
	OPECTL.note_syukei = 0;											// �������ɏW�v���o�͂Ƃ���

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
				BUZPI();
				if( OPECTL.RECI_SW == 0 ){							// �̎������ݖ��g�p?
					if(( OPECTL.ChkPassSyu == 0 )&&					// �⍇�����ł͂Ȃ�?
					   ( OPECTL.op_faz != 0 || ryodsp )){			// �ҋ@��ԈȊO?
						wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
						grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      �̎��؂𔭍s���܂�      "
					}
					OPECTL.RECI_SW = 1;								// �̎������ݎg�p
				}
				break;

			case COIN_EVT:											// Coin Mech event
				if( OPECTL.op_faz == 4 ){							// ��ѱ��
					if( OPECTL.CN_QSIG == 7 ){ 						// ���o��
						ret = 10;									// �ҋ@�ɖ߂�
						break;
					}else{
						OPECTL.op_faz = 0;
						cn_stat( 3, 2 );							// ������
					}
				}
			case NOTE_EVT:											// Note Reader event
				if(( r_zero == 0 )&&( OPECTL.cnsend_flg == 1 )){	// 0�~���Z�������\���
					r_zero = -1;
					OPECTL.op_faz = 1;								// ������
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
				}
			case TIMEOUT8:											// ������߂蔻����ϰ��ѱ��
			case KEY_TEN_F4:										// �ݷ�[���] �����
				if( msg == KEY_TEN_F4 ){							// �������?(Y)
					BUZPI();
					ope_anm( AVM_STOP );							// ������~�ųݽ
				}
				if( msg == TIMEOUT8 ){								// ������߂蔻����ϰ��ѱ��?
					msg = KEY_TEN_F4;								// �����I�ɐ��Z���~�Ƃ���
				}
				if( r_zero == 0 ){
					break;
				}
				if(( OPECTL.op_faz == 0 )&&( msg != KEY_TEN_F4 )){
					OPECTL.op_faz = 1;								// ������
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
				}

				/*** ������XXs�Ő��Z���~�Ƃ��� ***/
				Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
				ret = in_mony( msg, 0 );							// �����������
				break;

			case ARC_CR_R_EVT:										// ����IN
			case ARC_CR_E_EVT:										// �I������ގ�M
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				break;

			case TIMEOUT1:
				if( r_zero == 0 ){									// 0�~���Z
					cn_stat( 2, 2 );								// �����s��
					ret = 1;										// ���Z����������
					break;
				}
				if( OPECTL.op_faz >= 3 ){
					ret = 10;										// �ҋ@�ɖ߂�
					break;
				}
				cn_stat( 2, 2 );									// �����s��
				OPECTL.op_faz = 4;									// ���ڰ���̪��� ��ѱ��
				Lagcan( OPETCBNO, 1 );								// ��ϰ1ؾ��(���ڰ��ݐ���p)
				Lagtim( OPETCBNO, 2, 10*50 );						// ү��Ď���ϰ�N��
				break;

			case TIMEOUT2:											// ү��Ď���ϰ��ѱ��
				switch( OPECTL.op_faz ){
					case 2:											// ����
						if( ryo_buf.turisen ){						//
							SFV_DAT.reffal = ryo_buf.turisen;		//
							ret = 3;								// ���Z����������
						}else{
							ret = 1;								// ���Z����������
						}
						break;
					case 3:											// ���~
					case 4:											// ��ѱ��
						if( ryo_buf.nyukin ){						//
							ryo_buf.turisen = ryo_buf.nyukin;		//
							if( SFV_DAT.reffal ){					// �s������?
								ryo_buf.fusoku += SFV_DAT.reffal;	// �x�����s���z
							}
							if( SFV_DAT.nt_escrow ){				// �����߂��װ
								ryo_buf.fusoku += 1000L;			// �x�����s���z
							}
							chu_isu();								// ���~�W�v
							Lagtim( OPETCBNO, 1, 10*50 );			// ��ϰ1�N��(���ڰ��ݐ���p)
						}else{
							if( carduse() ){						// ���ގg�p?
								chu_isu();							// ���~����
								svs_renzoku();						// ���޽���A���}��
							}
							ret = 10;								// �ҋ@�ɖ߂�
						}
						break;
					default:
						ret = 10;									// �ҋ@�ɖ߂�
						break;
				}
				break;

			default:
				break;
		}
		if( ret == 10 ){											// ���(���~)?
			_di();
			CN_escr_timer = 0; 										// ������ϰ��ݾ�
			_ei();
			safecl( 7 );											// ���ɖ����Z�o
		}
	}

	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 2 );											// ��ϰ2ؾ��(ү��Ď��p)
	Lagcan( OPETCBNO, 8 );											// ��ϰ8ؾ��(������߂蔻����ϰ)

	OPECTL.Fin_mod = ret;
	if( ret != 10 ){
		OPECTL.Ope_mod = 23;										// ������X�V ���Z����������
	}
	else{
		cn_stat( 8, 0 );	// �ۗL�������M
		read_sht_opn();		// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
		opr_snd( 13 );		// ����o���ړ�
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//		Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );	// ���~���̋��K�Ǘ����O�f�[�^�쐬
//		Log_regist( LOG_MONEYMANAGE_NT );							// ���K�Ǘ����O�o�^
		if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {	// ���~���̋��K�Ǘ����O�f�[�^�쐬
			Log_regist( LOG_MONEYMANAGE_NT );						// ���K�Ǘ����O�o�^
		}
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�V ���Z����                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod23( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = ���튮��                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	op_mod23( void )
{
	short	ret;
	short	ryo_en;
	short	safe_que = 0;
	ushort	msg = 0;
	short	ret_bak = 0;
	ushort	wk_MsgNo;

	read_sht_opn();													// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��

	al_mkpas_renewal( (m_apspas *)MAGred, &CLK_REC );

	if( CPrmSS[S_TIK][10] == 0 ){									// ��������̧����Ȃ��ݒ�?
		opr_snd( 16 );												// ������ް�ײ�(���̧�����)
	}else{
		opr_snd( 9 );												// ������ް�ײ�(���̧��L��)
	}


	if( prm_get( COM_PRM,S_PAY,25,1,2 ) && !LprnErrChk( 1 ) ){		// ���ِݒ肠������ي֘A�װ�Ȃ����̂����ٔ��s����
		LedReq( LB_TRAYLED, LED_ON );								// ���َ�o�����޲��LED�_��

		PayData.TOutTime.Year = car_ot_f.year;						// �o�� �N
		PayData.TOutTime.Mon  = car_ot_f.mon;						//      ��
		PayData.TOutTime.Day  = car_ot_f.day;						//      ��

		OpLpr_snd( &PayData, 0 );									// ���ُo��
		LabelCountDown();											// ���ٔ��s�������Z
	}

	ryo_en = 0;														// �����ݕs��
	switch( OPECTL.Fin_mod ){
		case 1:														// �ނ薳��?
		default:
			ac_flg.cycl_fg = 13;									// �����o������(��)
			cn_stat( 6, 0 );										// ������ر(���ۺ�݂𗎂Ƃ�)
			ryo_en = 1;												// �����݉�
			if( OPECTL.RECI_SW == 1 ){								// �̎������ݎg�p?
				ryo_isu( 0 );										// �̎���(�ޑK�s������)���s
			}
			break;

		case 2:														// �ނ�L��?
			if( refund( (long)ryo_buf.turisen ) != 0 ){				// ���o�N��
				/*** ���oNG ***/
				ac_flg.cycl_fg = 13;								// �����o������(��)
				ryo_en = 1;											// �����݉�
				ryo_buf.fusoku = SFV_DAT.reffal;					// �x�����s���z
				set_escrowtimer();									// ������ϰ���
				Print_Condition_select();							// �̎���(�ޑK�s���L��) or �a��ؔ��s
			}else{
				if( (CN_refund & 0x01) == 0 ){
					// ��݂̕����o�����h�Ȃ��h�̂Ƃ�
					cn_stat( 6, 0 );									// ������ر(���ۺ�݂𗎂Ƃ�)
				}
			}
			LedReq( CN_TRAYLED, LED_ON );							// ��ݎ�o�����޲��LED�_��
			break;

		case 3:														// ү��װ(�ނ�L��)?
			ac_flg.cycl_fg = 13;									// �����o������(��)
			ryo_en = 1;												// �����݉�
			ryo_buf.fusoku = SFV_DAT.reffal;						// �x�����s���z
			set_escrowtimer();										// ������ϰ���
			Print_Condition_select();								// �̎���(�ޑK�s���L��) or �a��ؔ��s
			break;
	}

	OpeLcd( 23 );													// ������X�V ���Z�����\��

	cn_stat( 4, 1 );												// �����捞

	Lagtim( OPETCBNO, 1, 10*50 );									// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 2 );											// Timer Cancel

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case COIN_EVT:											// Coin Mech event
				ryo_en = 1;											// �����݉�
				if( OPECTL.CN_QSIG == 2 ){							// ���o����
					ac_flg.cycl_fg = 13;							// �����o������
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					if( SFV_DAT.reffal ){
						ryo_buf.fusoku += SFV_DAT.reffal;			// �x�����s���z
						Print_Condition_select();					// �̎���(�ޑK�s���L��) or �a��ؔ��s
					}
					else if( OPECTL.RECI_SW ){						// �̎������ݏ��
						ryo_isu( 0 );								// �̎���(�ޑK�s������)���s
					    OPECTL.RECI_SW = 1;							// �̎������ݎg�p
					}
				}
				else if( OPECTL.CN_QSIG == 9 ){						// ���o�װ
					ac_flg.cycl_fg = 13;							// �����o������(��)
					refalt();										// �s�����Z�o
					ryo_buf.fusoku += SFV_DAT.reffal;				// �x�����s���z
					Print_Condition_select();						// �̎���(�ޑK�s���L��) or �a��ؔ��s
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					set_escrowtimer();								// ������ϰ���
				}
				if( ret_bak ) ret = ret_bak;
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
			case KEY_TEN_CL:										// �ݷ�[���] �����
				BUZPI();
			case TIMEOUT1:											// Time Over ?
				if( ryo_en == 0 ){
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					break;
				}
				if( SFV_DAT.safe_cal_do != 0 ){						// ���Ɍv�Z���I��?
					if( msg == TIMEOUT1 ){
					safe_que++;
					if( safe_que < 10 ){
						Lagtim( OPETCBNO, 1, 1*50 );				// ��ϰ1(1s)�N��(���ڰ��ݐ���p)
						break;
					}
					ex_errlg( ERRMDL_MAIN, ERR_MAIN_NOSAFECLOPERATE, 2, 0 );	// ���ɖ����Z�o�Ȃ��ɔ�����
					}else{
						break;
					}
				}
				ret = 10;
				break;

			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
				BUZPI();
				if( OPECTL.RECI_SW == 0 ){							// �̎������ݖ��g�p?
					OPECTL.RECI_SW = 1;								// �̎������ݎg�p
					wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
					grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "     �̎��؂�����艺����     "
					if( ryo_en == 0 ){
						break;
					}
					ryo_isu( 0 );									// �̎���(�ޑK�s������)���s
					Lagtim( OPETCBNO, 1, 10*50 );					// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
					OPECTL.PassNearEnd = 0;							// �����؂�ԋ�
				}
				break;

			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				if(( ryo_en != 0 )&&( SFV_DAT.safe_cal_do == 0 )){	// ���o�����ŋ��Ɍv�Z�I���Ȃ�
					Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
					ret = 10;
				}
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				OPECTL.PassNearEnd = 0;								// �����؂�ԋ�
				break;

			case IBK_MIF_A2_NG_EVT:									// Mifare�ް��Ǐo��NG
				if( MIF_ENDSTS.sts1 == 0x30 ){						// ���ޖ���
					OPECTL.PassNearEnd = 0;							// �����؂�ԋ��׸�
				}
				break;

			case IBK_LPR_B1_REC_NG:									// ��������Ď��s
				grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR2[8] );					// "���x���̔��s���o���܂���ł���"���]�\��
				// no break
			case IBK_LPR_B1_REC_OK:									// ��������Đ���
				break;

			default:
				break;
		}
		// �ޑK�̕��o�����������Ŕ����悤�Ƃ�����A���o������������܂ő҂�
		if(( ryo_en == 0 )&&( ret )){								// ���o������?
			ret_bak = ret;											// Yes..Save
			ret = 0;
		}
	}
	if( OPECTL.RECI_SW == 1 ){										// �̎������ݎg�p?
		OPECTL.RECI_SW = 0;											// �̎������ݖ��g�p
	}else{
		PayData_set( 0, 0 );										// 1���Z����
		kan_syuu();													// ���Z�����W�v
		OPECTL.RECI_SW = -1;										// �ҋ@���̎������݉�
	}
	cn_stat( 8, 0 );	// �ۗL�������M
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//	Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );					// ���K�Ǘ����O�f�[�^�쐬
//	Log_regist( LOG_MONEYMANAGE_NT );								// ���K�Ǘ����O�o�^
	if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {			// ���K�Ǘ����O�f�[�^�쐬
		Log_regist( LOG_MONEYMANAGE_NT );							// ���K�Ǘ����O�o�^
	}
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)

	return( ret );
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| ���Ԉʒu�̏������                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : carchk( snum, pnum, paymod )                            |*/
/*| PARAMETER    : snum   : ���                                           |*/
/*|                pnum   : �ڋq�p���Ԉʒu�ԍ�                             |*/
/*|                paymod : Ӱ�� 0=�ʏ� 1=�C��                             |*/
/*|                		  :      2=��t�����s�� 3=�Ïؔԍ��o�^��           |*/
/*| RETURN VALUE : ret    :  0 = OK                                        |*/
/*|                       :  1 = ۯ����u�J�n�҂�,ۯ����u�҂�           |*/
/*|                       :  2 = �o�ɏ��                                  |*/
/*|                       :  3 = �x�Ə��(���ү��A����ذ�ް�ʐM�ُ�)       |*/
/*|                       :  4 = ���O�^�C����                              |*/
/*|                       : 10 = NG(ۯ����u��Ԃ��K��O��)                 |*/
/*|                       : -1 = NG(���ݒ蓙�ɂ��{���Z�@�ł͐��Z�s�\)  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	carchk( uchar snum, ushort pnum, uchar paymod )
{
	short	ret;
	ushort	posi;

	ret = -1;
	OPECTL.InLagTimeMode = OFF;										// ���O�^�C���������[�h�n�e�e

	if(( snum > 26 )||
	   (( pnum < 1 )||( pnum > 9999 ))){							// ���,���Ԉʒu�ԍ����ΏۊO?
		return( 10 );
	}

	if( LKopeGetLockNum( snum, pnum, &posi ) ){
		OPECTL.Op_LokNo = (ulong)(( snum * 10000L ) + pnum );		// �ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)
		OPECTL.Pr_LokNo = posi;										// ���������p���Ԉʒu�ԍ�(1�`324)
		pnum = posi-1;
		ret = 10;
	}

	if( ret == 10 ){
		for( ; ; ){
			if( FlpSetChk( pnum ) != 1 ){							// Flap Settei Check
				break;
			}

			if( paymod == 1 ){										// �C�����Z?
				if( syusei[pnum].sei == 1 ){
					ret = 0;
				}
				break;
			}
			else if(paymod / 10)									// 10�̌���0�ȊO(���ԏؖ������s�L��)
			{
				paymod = (uchar)(paymod % 10);						// 1�̌��̂ݎc��
			}

			switch( FLAPDT.flp_data[pnum].mode ){
				case FLAP_CTRL_MODE2:								// ۯ����u�J�n�҂�
				case FLAP_CTRL_MODE3:								// ۯ����u�����҂�
					ret = 1;
					break;
				case FLAP_CTRL_MODE4:								// ���Ԓ�
					ret = 0;
					break;
				case FLAP_CTRL_MODE1:								// �ҋ@���
				case FLAP_CTRL_MODE5:								// ۯ����u�J�����҂�
				case FLAP_CTRL_MODE6:								// �o�ɑ҂�
					if (Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[pnum].ryo_syu, 1, 4)) {		// ���O�^�C��������������
						if ((FLAPDT.flp_data[pnum].mode == FLAP_CTRL_MODE5 || FLAPDT.flp_data[pnum].mode == FLAP_CTRL_MODE6) &&	// ���b�N���u�J�����҂� or �o�ɑ҂���
							(FLAPDT.flp_data[pnum].timer > 0) &&																// ���O�^�C�}�[���쒆�H
							(OPECTL.Ope_mod == 1) &&																			// ���Ԉʒu�ԍ����͏�����
							(paymod < 2)) {																						// �e���L�[[���Z] �C�x���g(�ꉞ�C�����Z�����\�ɂ���)
							if (!prm_get(COM_PRM, S_TYP, 80, 1, 2) ||		// ���O�^�C�����������Ȃ�
								!prm_get(COM_PRM, S_TYP, 80, 1, 1) ||		// ���O�^�C����������Ȃ�
								Flp_LagExtCnt[pnum] < prm_get(COM_PRM, S_TYP, 80, 1, 1)) {
																			// �����񐔓�
								OPECTL.InLagTimeMode = ON;					// ���O�^�C���������[�h�n�m
								ret = 0;
								break;
							} else {
								if (prm_get(COM_PRM, S_DSP, 20, 1, 1)) {	// ���O�^�C�����Ԏ��̐ڋq�ē�����
									ret = 4;
									break;
								}
							}
						}
					} else {
						if ((prm_get(COM_PRM, S_DSP, 20, 1, 1)) &&			// ���O�^�C�����Ԏ��̐ڋq�ē�����
							(FLAPDT.flp_data[pnum].mode == FLAP_CTRL_MODE5 || FLAPDT.flp_data[pnum].mode == FLAP_CTRL_MODE6) &&	// ���b�N���u�J�����҂� or �o�ɑ҂���
							(FLAPDT.flp_data[pnum].timer > 0) &&																// ���O�^�C�}�[���쒆�H
							(OPECTL.Ope_mod == 1) &&																			// ���Ԉʒu�ԍ����͏�����
							(paymod < 2)) {																						// �e���L�[[���Z] �C�x���g(�ꉞ�C�����Z�����\�ɂ���)
							ret = 4;
							break;
						}
					}
					ret = 2;
					break;
				default:
					ex_errlg( ERRMDL_MAIN, ERR_MAIN_LOCKSTATUSNG, 2, 0 );	// ۯ����u��ԋK��O
					break;
			}
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��ԍ������ׯ�ߐݒ���������鏈��                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FlpSetChk( pnum )                                       |*/
/*| PARAMETER    : num   : �Ԏ���-1                                        |*/
/*| RETURN VALUE : ret   : 0 = �ݒ薳�� 1 = �ݒ�L��                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	FlpSetChk( ushort pnum )
{
	if(( LockInfo[pnum].lok_syu == 0 )||							// Ұ���ݒ薳��
	   ( LockInfo[pnum].ryo_syu == 0 )){							// ������ʖ���
		return( 0 );												// �L���ݒ薳��
	}
	return( 1 );													// �L���ݒ�L��
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Ԉʒu�̏������                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : carchk_ml( snum, pnum )                                 |*/
/*| PARAMETER    : snum   : ���                                           |*/
/*|                pnum   : �ڋq�p���Ԉʒu�ԍ�                             |*/
/*| RETURN VALUE : ret    :  0 = OK                                        |*/
/*|                       :  1 = ۯ����u�J�n�҂�,ۯ����u�҂�           |*/
/*|                       :  2 = �o�ɏ��                                  |*/
/*|                       :  3 = �x�Ə��(���ү��A����ذ�ް�ʐM�ُ�)       |*/
/*|                       : 10 = NG(ۯ����u��Ԃ��K��O��)                 |*/
/*|                       : -1 = NG(���ݒ蓙�ɂ��{���Z�@�ł͐��Z�s�\)  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	carchk_ml( uchar snum, ushort pnum )
{
	short	ret;

	ret = -1;

	if(( snum > 26 )||
	   (( pnum < 1 )||( pnum > 9999 ))){							// ���,���Ԉʒu�ԍ����ΏۊO?
		return( 10 );
	}

	for( ; ; ){
		if( prm_get( COM_PRM,S_SHA,(short)(1+6*(LOCKMULTI.lock_mlt.ryo_syu-1)),2,5 ) == 0 ){
			// �����v�Z�����ʂ��ݒ肳��Ă��Ȃ��B
			break;
		}
		if( LOCKMULTI.lock_mlt.bk_syu ){
			if( prm_get( COM_PRM,S_SHA,(short)(1+6*(LOCKMULTI.lock_mlt.bk_syu-1)),2,5 ) == 0 ){
				// �����v�Z�����ʂ��ݒ肳��Ă��Ȃ��B
				break;
			}
		}

		switch( LOCKMULTI.lock_mlt.mode ){
			case 1:												// ۯ����u�J�n�҂�
			case 2:												// ۯ����u�����҂�
				ret = 1;
				break;
			case 3:												// ���Ԓ�
				ret = 0;
				break;
			case 0:												// �ҋ@���
			case 4:												// ۯ����u�J�����҂�
			case 5:												// �o�ɑ҂�
				ret = 2;
				break;
			default:
				ex_errlg( ERRMDL_MAIN, ERR_MAIN_LOCKSTATUSNG, 2, 0 );	// ۯ����u��ԋK��O
				break;
		}
		break;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���o�Ɏ������                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : set_tim( num, clk, err_mode )                           |*/
/*| PARAMETER    : num      : �Ԏ���                                       |*/
/*|              : clk      : ���ݎ���                                     |*/
/*|              : err_mode : �װ�L�� 0:���� / 1:�L��                      |*/
/*| RETURN VALUE : ret      : ���o�Ɏ�������(1980�`2079�N�ȓ���?)0:OK 1:NG |*/
/*[]----------------------------------------------------------------------[]*/
/*| car_in, car_ot, car_in_f, car_ot_f�ɓ��o�Ɏ������Z�b�g����             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	set_tim( ushort num, struct clk_rec *clk_para, short err_mode )
{
	flp_com	*frp;
	ushort	in_tim, out_tim;

	if( OPECTL.multi_lk ){											// ������Z�⍇����?
		frp = &LOCKMULTI.lock_mlt;
	}else{
		frp = &FLAPDT.flp_data[num-1];
	}

	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					// ���Ԏ����ر
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// �o�Ԏ����ر
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// ���Ԏ���Fix�ر
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// �o�Ԏ���Fix�ر
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// �Čv�Z�p���Ԏ����ر

	car_in.year = frp->year;										// ���� �N
	car_in.mon  = frp->mont;										//      ��
	car_in.day  = frp->date;										//      ��
	car_in.hour = frp->hour;										//      ��
	car_in.min  = frp->minu;										//      ��
	car_in.week = (char)youbiget( car_in.year,						//      �j��
								(short)car_in.mon,
								(short)car_in.day );
// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
	// �V�X�e��ID16�F�U�֐��Z�H
	if( PiP_GetFurikaeSts() ){
		// ���Ɏ����ێ�
		g_PipCtrl.stFurikaeInfo.DestInTime.Year		= car_in.year;	// Year
		g_PipCtrl.stFurikaeInfo.DestInTime.Mon		= car_in.mon;	// Month
		g_PipCtrl.stFurikaeInfo.DestInTime.Day		= car_in.day;	// Day
		g_PipCtrl.stFurikaeInfo.DestInTime.Hour		= car_in.hour;	// Hour
		g_PipCtrl.stFurikaeInfo.DestInTime.Min		= car_in.min;	// Minute
	}
// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z

	car_ot.year = clk_para->year;									// �o�� �N
	car_ot.mon  = clk_para->mont;									//      ��
	car_ot.day  = clk_para->date;									//      ��
	car_ot.hour = clk_para->hour;									//      ��
	car_ot.min  = clk_para->minu;									//      ��
	car_ot.week = (char)youbiget( car_ot.year,						//      �j��
								(short)car_ot.mon,
								(short)car_ot.day );
// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
	// �V�X�e��ID16�F�U�֐��Z�H
	if( PiP_GetFurikaeSts() ){
		// �����v�Z�����ێ�
		g_PipCtrl.stFurikaeInfo.DestPayTime.Year	= clk_para->year;	// Year
		g_PipCtrl.stFurikaeInfo.DestPayTime.Mon		= clk_para->mont;	// Month
		g_PipCtrl.stFurikaeInfo.DestPayTime.Day		= clk_para->date;	// Day
		g_PipCtrl.stFurikaeInfo.DestPayTime.Hour	= clk_para->hour;	// Hour
		g_PipCtrl.stFurikaeInfo.DestPayTime.Min		= clk_para->minu;	// Minute
		g_PipCtrl.stFurikaeInfo.DestPayTime.Sec		= clk_para->seco;	// Second
	}
// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z

	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// ���Ԏ���Fix
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			// �o�Ԏ���Fix
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		// �Čv�Z�p���Ԏ���Fix

	if( err_mode == 0 ){
		return( 0 );
	}

	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// ���Ɏ����K��O?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// �o�Ɏ����K��O?
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTTIMENG, 2, 0 );		// ���o�Ɏ����K��O
		return( 1 );
	}
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					// (���ɓ�+1�N)normlize
					(short)car_in.mon,
					(short)car_in.day );
	out_tim = dnrmlzm( car_ot.year,									// �o�ɓ�normlize
					(short)car_ot.mon,
					(short)car_ot.day );
	if( in_tim <= out_tim ){
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_OVER1YEARCAL, 2, 0 );		// 1�N�ȏ�̗����v�Z���s����
	}
	in_tim = dnrmlzm((short)( car_in.year ),						/* (���ɓ�)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){											/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTGYAKU, 2, 1 );			/* ���o�ɋt�]					*/
		return( 1 );												/*								*/
	}																/*								*/
	return( 0 );
}

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| ���Z����(���o�Ɏ���)���                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : set_tim2(  clk_para )                                   |*/
/*| PARAMETER    : clk_para : ���ݎ���                                     |*/
/*| RETURN VALUE : ret      : ���o�Ɏ�������(1980�`2079�N�ȓ���?)0:OK 1:NG |*/
/*[]----------------------------------------------------------------------[]*/
/*| car_in, car_ot, car_in_f, car_ot_f�ɓ��o�Ɏ������Z�b�g����             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	set_tim2( struct clk_rec *clk_para )
{
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					// ���Ԏ����ر
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// �o�Ԏ����ر
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// ���Ԏ���Fix�ر
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// �o�Ԏ���Fix�ر
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// �Čv�Z�p���Ԏ����ر

	car_in.year = clk_para->year;									// ���� �N
	car_in.mon  = clk_para->mont;									//      ��
	car_in.day  = clk_para->date;									//      ��
	car_in.hour = clk_para->hour;									//      ��
	car_in.min  = clk_para->minu;									//      ��
	car_in.week = (char)youbiget( car_in.year,						//      �j��
								(short)car_in.mon,
								(short)car_in.day );

	memcpy( &car_ot, &car_in, sizeof( struct CAR_TIM ) );			// �o�Ԏ���
	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// ���Ԏ���Fix
	memcpy( &car_ot_f, &car_in, sizeof( struct CAR_TIM ) );			// �o�Ԏ���Fix
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		// �Čv�Z�p���Ԏ���Fix

	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// ���Ɏ����K��O?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// �o�Ɏ����K��O?
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTTIMENG, 2, 0 );		// ���o�Ɏ����K��O
		return( 1 );
	}
	return( 0 );
}
#endif	// UPDATE_A_PASS
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : set_tim( num, clk, err_mode )                           |*/
/*| PARAMETER    : trig     : �ضް                                       |*/
/*|              : num      : �Ԏ���                                       |*/
/*|              : clk      : ���ݎ���                                     |*/
/*|              : err_mode : �װ�L�� 0:���� / 1:�L��                      |*/
/*| RETURN VALUE : ret      : ���o�Ɏ�������(1980�`2079�N�ȓ���?)0:OK 1:NG |*/
/*[]----------------------------------------------------------------------[]*/
/*| car_in, car_ot, car_in_f, car_ot_f�ɓ��o�Ɏ������Z�b�g����             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	set_tim_Lost( ushort trig, ushort num, struct clk_rec *clk_para, short err_mode )
{
	flp_com	*frp;

	if( OPECTL.multi_lk ){											// ������Z�⍇����?
		frp = &LOCKMULTI.lock_mlt;
	}else{
		frp = &FLAPDT.flp_data[num-1];
	}
	// �N���A
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					// ���Ԏ����ر
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// �o�Ԏ����ر
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// ���Ԏ���Fix�ر
	NTNET_Data152Save( (void *)&car_in_f, NTNET_152_CARINTIME );
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// �o�Ԏ���Fix�ر
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// �Čv�Z�p���Ԏ����ر

	// ���z�w�艓�u���Z�H
	if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE ){
		// ���Ɏ���������H
		if( frp->mode > FLAP_CTRL_MODE3 ){
			car_in.year = frp->year;								// ���� �N
			car_in.mon  = frp->mont;								//      ��
			car_in.day  = frp->date;								//      ��
			car_in.hour = frp->hour;								//      ��
			car_in.min  = frp->minu;								//      ��
			car_in.week = (char)youbiget( car_in.year,				//      �j��
										(short)car_in.mon,
										(short)car_in.day );
		}
		// ���Ɏ����Ȃ��H
		else{
			// ���Z�������Z�b�g����
			car_in.year = clk_para->year;							// ���� �N
			car_in.mon  = clk_para->mont;							//      ��
			car_in.day  = clk_para->date;							//      ��
			car_in.hour = clk_para->hour;							//      ��
			car_in.min  = clk_para->minu;							//      ��
			car_in.week = (char)youbiget( car_in.year,				//      �j��
										(short)car_in.mon,
										(short)car_in.day );
		}
	}

	ryo_buf.fumei_fg = 2;											// ���Ԏ����s�����set
	// ������or�����{�^��
	ryo_buf.fumei_reason = fumei_LOST;								// �s���ڍח��R:�������Z

	car_ot.year = clk_para->year;									// �o�� �N
	car_ot.mon  = clk_para->mont;									//      ��
	car_ot.day  = clk_para->date;									//      ��
	car_ot.hour = clk_para->hour;									//      ��
	car_ot.min  = clk_para->minu;									//      ��
	car_ot.week = (char)youbiget( car_ot.year,						//      �j��
								(short)car_ot.mon,
								(short)car_ot.day );

	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// ���Ԏ���Fix
	NTNET_Data152Save( (void *)&car_in_f, NTNET_152_CARINTIME );
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			// �o�Ԏ���Fix
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		// �Čv�Z�p���Ԏ���Fix

	return( 0 );
}
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)
//[]----------------------------------------------------------------------[]
///	@brief		���Ɏ��������Z�b�g�ςݎ��̓��o�Ɏ����Z�b�g
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/03<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
short set_tim_only_out( ushort num, struct clk_rec *clk_para, short err_mode )
{
	ushort	in_tim, out_tim;
// MH810100(S) K.Onodera 2020/03/03 #3989 ���Ɏ����w��̉��u���Z�œ���������\������Ȃ�
	DATE_YMDHMS paytime;
// MH810100(E) K.Onodera 2020/03/03 #3989 ���Ɏ����w��̉��u���Z�œ���������\������Ȃ�

	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// �o�Ԏ����ر
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// ���Ԏ���Fix�ر
	NTNET_Data152Save( (void *)&car_in_f, NTNET_152_CARINTIME );
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// �o�Ԏ���Fix�ر
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// �Čv�Z�p���Ԏ����ر

	car_ot.year = clk_para->year;									// �o�� �N
	car_ot.mon  = clk_para->mont;									//      ��
	car_ot.day  = clk_para->date;									//      ��
	car_ot.hour = clk_para->hour;									//      ��
	car_ot.min  = clk_para->minu;									//      ��
	car_ot.week = (char)youbiget( car_ot.year,						//      �j��
								(short)car_ot.mon,
								(short)car_ot.day );
	// �����v�Z�����ێ�
	g_PipCtrl.stRemoteTime.OutTime.Year	= clk_para->year;			// Year
	g_PipCtrl.stRemoteTime.OutTime.Mon	= clk_para->mont;			// Month
	g_PipCtrl.stRemoteTime.OutTime.Day	= clk_para->date;			// Day
	g_PipCtrl.stRemoteTime.OutTime.Hour	= clk_para->hour;			// Hour
	g_PipCtrl.stRemoteTime.OutTime.Min	= clk_para->minu;			// Minute
	g_PipCtrl.stRemoteTime.OutTime.Sec	= clk_para->seco;			// Second

// MH810100(S) K.Onodera 2020/03/03 #3989 ���Ɏ����w��̉��u���Z�œ���������\������Ȃ�
	// LCD�ɒʒm���邽�߂ɐ��Z������ێ�����(�c���ω��ʒm�ɂđ��M����)
	memset( &paytime, 0, sizeof(paytime) );
	paytime.Year = car_in.year;
	paytime.Mon  = car_in.mon;
	paytime.Day  = car_in.day;
	paytime.Hour = car_in.hour;
	paytime.Min  = car_in.min;
	paytime.Sec  = (uchar)g_PipCtrl.stRemoteTime.InTime.Sec;

	SetPayTargetTime( &paytime );
// MH810100(E) K.Onodera 2020/03/03 #3989 ���Ɏ����w��̉��u���Z�œ���������\������Ȃ�

	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// ���Ԏ���Fix
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			// �o�Ԏ���Fix
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		// �Čv�Z�p���Ԏ���Fix

	if( err_mode == 0 ){
		return( 0 );
	}

	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// ���Ɏ����K��O?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// �o�Ɏ����K��O?
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTTIMENG, 2, 0 );		// ���o�Ɏ����K��O
		return( 1 );
	}
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					// (���ɓ�+1�N)normlize
					(short)car_in.mon,
					(short)car_in.day );
	out_tim = dnrmlzm( car_ot.year,									// �o�ɓ�normlize
					(short)car_ot.mon,
					(short)car_ot.day );
	if( in_tim <= out_tim ){
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_OVER1YEARCAL, 2, 0 );		// 1�N�ȏ�̗����v�Z���s����
	}
	in_tim = dnrmlzm((short)( car_in.year ),						/* (���ɓ�)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){											/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTGYAKU, 2, 1 );			/* ���o�ɋt�]					*/
		return( 1 );												/*								*/
	}																/*								*/
	return( 0 );
}

// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//[]----------------------------------------------------------------------[]
///	@brief		���o�Ɏ����Z�b�g
//[]----------------------------------------------------------------------[]
///	@param		err_mode : error�����̗L��
///	@return		0=����/1=�ُ�
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2019/11/15<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static short set_tim_only_out_card( short err_mode )
{
	ushort	in_tim, out_tim;
	stDatetTimeYtoSec_t* pExitDate = NULL;
	stDateParkTime_t* pExit = NULL;
	DATE_YMDHMS paytime;
	struct clk_rec *pTime;

	// �o�Ɏ���
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
//	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );
	memset( &car_ot_f, 0, sizeof( car_ot_f ) );
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j

	// ���Ɏ���
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
//	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );
	memset( &car_in_f, 0, sizeof( car_in_f ) );
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j

	NTNET_Data152Save( (void *)&car_in_f, NTNET_152_CARINTIME );

	// �Čv�Z�p���Ԏ���
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );

	// �������Z�`�F�b�N
	pExit = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime;

	// �����Z�o�ɐ��Z�H
	if( lcdbm_rsp_in_car_info_main.shubetsu == 1 ){
		if( pExit->ParkingNo && pExit->MachineKind && 
			pExit->dtTimeYtoSec.shYear && pExit->dtTimeYtoSec.byMonth && pExit->dtTimeYtoSec.byDay ){
			pExitDate = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec;
			// �o�Ɏ������ <- �ݎԏ��_�o�Ɏ���
			car_ot.year = pExitDate->shYear;		// �o��_�N	2000�`2099(��������)
			car_ot.mon  = pExitDate->byMonth;		// �o��_��	1�`12(��������)
			car_ot.day  = pExitDate->byDay;			// �o��_��	1�`31(��������)
			car_ot.hour = pExitDate->byHours;		// �o��_��	0�`23(��������)
			car_ot.min  = pExitDate->byMinute;		// �o��_��	0�`59(��������)
			// �ݎԏ��_���Z �N���������b
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear		= pExitDate->shYear;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth		= pExitDate->byMonth;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay		= pExitDate->byDay;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours		= pExitDate->byHours;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute	= pExitDate->byMinute;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond	= pExitDate->bySecond;
		}
		else{
			// �����Z�o�ɐ��Z�����A�o�ɏ�񂪐������Z�b�g����Ă��Ȃ�
			return( 1 );
		}
	}
	// ���O���Z�H
	else {
		// ���Z�J�n�������擾
		pTime = GetPayStartTime();

		// �o�Ɏ������ <- parameter�̌��ݎ���
		car_ot.year = pTime->year;									// �o�� �N
		car_ot.mon  = pTime->mont;									//      ��
		car_ot.day  = pTime->date;									//      ��
		car_ot.hour = pTime->hour;									//      ��
		car_ot.min  = pTime->minu;									//      ��
		// �ݎԏ��_���Z �N���������b
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear		= pTime->year;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth		= pTime->mont;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay		= pTime->date;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours		= pTime->hour;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute	= pTime->minu;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond	= pTime->seco;
	}
	// �j���擾
	car_ot.week = (char)youbiget( car_ot.year, (short)car_ot.mon, (short)car_ot.day );

	// LCD�ɒʒm���邽�߂ɐ��Z������ێ�����(�c���ω��ʒm�ɂđ��M����)
	memcpy( &paytime, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec, sizeof(paytime) );
	SetPayTargetTime( &paytime );

	// ���Ԏ���Fix <- �{�֐�call�O�ɐݒ肵�����Ɏ������
	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
	car_in_f.sec = (char)lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.bySecond;
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j

	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);

	// �o�Ԏ���Fix <- �o�Ɏ������ <- parameter�̌��ݎ���
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
	car_ot_f.sec = (char)lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond;
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j

	// �Čv�Z�p���Ԏ���Fix <- �{�֐�call�O�ɐݒ肵�����Ɏ������
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );

	// error�����̗L�� = ��
	if( err_mode == 0 ){
		return( 0 );
	}

	// err_mode == 1�Ȃ̂�error�������s��
	// car_in��� = Ope_Set_tyudata_Card()��set	--->	vl_lcd_tikchg();
	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// ���Ɏ����K��O?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// �o�Ɏ����K��O?
			ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTTIMENG, 2, 0 );		// ���o�Ɏ����K��O
			return( 1 );
	}

	// (���ɓ�+1�N)normlize
	in_tim = dnrmlzm( (short)( car_in.year + 1 ), (short)car_in.mon, (short)car_in.day );

	// �o�ɓ�normlize
	out_tim = dnrmlzm( car_ot.year, (short)car_ot.mon, (short)car_ot.day );

	// (���ɓ�+1�N)normlize <= �o�ɓ�normlize
	if( in_tim <= out_tim ){
		// 1�N�ȏ�̗����v�Z���s����
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_OVER1YEARCAL, 2, 0 );
		return( 1 );
	}

	// (���ɓ�)normlize
	in_tim = dnrmlzm( (short)( car_in.year ), (short)car_in.mon, (short)car_in.day );

	// ���� > �o�� �� ���o�ɋt�]
	if( in_tim > out_tim ){
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTGYAKU, 2, 1 );
		return( 1 );
	}

	return( 0 );
}

//[]----------------------------------------------------------------------[]
///	@brief		���Z�J�n�������ݒ�
//[]----------------------------------------------------------------------[]
///	@return		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void SetPayStartTime( struct clk_rec *pTime )
{
// MH810100(S) 2020/09/09 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
//	memcpy( &PayStartTime, pTime, sizeof(*pTime) );
	memcpy( &g_PayStartTime, pTime, sizeof(*pTime) );
// MH810100(E) 2020/09/09 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
}

//[]----------------------------------------------------------------------[]
///	@brief		���Z�J�n�����擾
//[]----------------------------------------------------------------------[]
///	@return		���}���������|�C���^
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
struct clk_rec* GetPayStartTime( void )
{
// MH810100(S) 2020/09/09 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
//	return &PayStartTime;
	return &g_PayStartTime;
// MH810100(E) 2020/09/09 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
}

//[]----------------------------------------------------------------------[]
///	@brief		���Z�����ݒ�
//[]----------------------------------------------------------------------[]
///	@return		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void SetPayTargetTime( DATE_YMDHMS *pTime )
{
	memcpy( &PayTargetTime, pTime, sizeof(*pTime) );
}

//[]----------------------------------------------------------------------[]
///	@brief		���Z�����擾
//[]----------------------------------------------------------------------[]
///	@return		���}���������|�C���^
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static DATE_YMDHMS* GetPayTargetTime( void )
{
	return &PayTargetTime;
}

/********************************************************************/
/* ��QR/ICC/�����̓����g�p											*/
/*		lcdbm_QRans_InCar_status									*/
/* 0 =	op_mod2()��loop��������QR�ް�����M���Ă��Ȃ����			*/
/* 1 =	QR�ް���M & QR�ް�����(OK)���M���LCD_IN_CAR_INFO�҂�		*/
/* 2 =	1�̏�ԂŌ���(COIN_EVT: NOTE_EVT:)���������				*/
/*		�����v�Z���s��ryo_buf�̍X�V�͍s�������Z�����Ƃ��Ȃ����		*/
/*		invcrd()��ret == 1 or 2	�Ő��Z�����Ƃ��鎞					*/
/*		���Z�����Ƃ��Ȃ���(ret = 0)	�� =op_mod02�𔲂��Ȃ�			*/
/*		lcdbm_QRans_InCar_status <- 2�Ƃ���							*/
/* 3	2�̏�Ԃ�op_mod02��LCD_IN_CAR_INFO��M�������				*/
/*		���̏�ԂŊ������Z�̍Čv�Z���s��							*/
/*		�������Z�� <- 0�ɂ���										*/
/********************************************************************/
// invcrd()����return����O��call�����invcrd()��ret�ϐ���check����
// invcrd()�ł͐��Z�����̗L���̂ݔ��肵�Ă���̂�invcrd()��call����in_mony()�ł͂Ȃ�invcrd()�ōs��
// ret == 1(���Z�I��(�ޖ���)) or 2(���Z�I��(�ޗL��)) && lcdbm_QRans_InCar_status == 1�Ȃ�
// ret <- 0 & lcdbm_QRans_InCar_status <- 2�ɂ��Đ��Z�����Ƃ��Ȃ�
void lcdbm_QRans_InCar_DoNotPayCompCheck(short *ret)
{
	// ���炩�̓��������鎞
	if (ryo_buf.nyukin != 0) {
		// QR�ް���M & QR�ް�����(OK)���M���LCD_IN_CAR_INFO�҂�
		if (lcdbm_QRans_InCar_status == 1) {
			// LCD_IN_CAR_INFO��M�҂�
			// ���̏�ԂŊ������Z�̍Čv�Z���s��
			// �������Z�� <- 0�ɂ���
			lcdbm_QRans_InCar_status = 2;
		}

// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
//		if (lcdbm_QRans_InCar_status > 1) {
		if ( lcdbm_QRans_InCar_status == 2 ) {
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�����̃Z���^�[�⍇�����Ɍ����Ő��Z��������ƃt�F�[�Y���ꂪ��������@�s�#7041
			// 1(���Z�I��(�ޖ���)) or 2(���Z�I��(�ޗL��))
			if ((*ret == 1) || (*ret == 2)) {
				// ���Z���I�� <- 1(���Z�I��(�ޖ���)) or 2(���Z�I��(�ޗL��)
				*ret = 0;
			}
		}
	}
}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

/*[]----------------------------------------------------------------------[]*/
/*| ���Z��������                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : invcrd( void )                                          |*/
/*| PARAMETER    : paymod : 0 = �ʏ�, 1 = �C��                             |*/
/*| RETURN VALUE : ret    : 0 = ���Z���I��                                 |*/
/*|                       : 1 = ���Z�I��(�ޖ���)                           |*/
/*|                       : 2 = ���Z�I��(�ޗL��)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	invcrd( ushort paymod )
{
	short	ret;

	ret = 0;
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//	if( ryo_buf.credit.pay_ryo != 0 ){
//		if ( ryo_buf.zankin == 0 ){
//			if ( ryo_buf.turisen == 0 ){
//				ret = 1;											// ���Z�I��(�ޖ���)
//			}else{
//				ret = 2;											// ���Z�I��(�ޗL��)
//			}
//		}
//		if( ryo_buf.credit.pay_ryo != 0 ){
//			creMessageAnaOnOff( 0, 0 );
//		}
//
//		Lagcan( OPETCBNO, 10 );										// ���p�\�}�̻��د��\���p��ϰSTOP
//		OpeLcd( 4 );												// �����\��(�������͊�����)
//
//		if( ret == 0 ){
//			creMessageAnaOnOff( 1, 1 );
//			//�N���W�b�g�J�[�h�}����ɂ����������A����Ⴂ�̏ꍇ.
//			OPECTL.credit_in_mony = 1;
//		}
//		return ret;
//	}
	// �J�[�h������
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if (Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC) {
	if (isEC_USE()) {
	if (isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		ec_MessageAnaOnOff( 0, 0 );

		OpeLcd( 4 );												// �����\��(�������͊�����)

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX
//		if(EcDeemedLog.EcDeemedBrandNo == BRANDNO_CREDIT) {
		if(RecvBrandResTbl.no == BRANDNO_CREDIT) {
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX
			ec_MessageAnaOnOff( 1, 1 );			// �N���W�b�g�J�[�h������
		} else {
			ec_MessageAnaOnOff( 1, 2 );			// �J�[�h������
		}
		//�N���W�b�g�J�[�h�}����ɂ����������A����Ⴂ�̏ꍇ.
		OPECTL.credit_in_mony = 1;

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	// lcdbm_QRans_InCar_status�̍X�Vcheck
	// invcrd()����return����O��call�����invcrd()��ret�ϐ���check����
	lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		return ret;
	}
	// �N���W�b�g���ό�
	else if (ryo_buf.credit.pay_ryo != 0) {
		if ( ryo_buf.turisen == 0 ){
			ret = 1;												// ���Z�I��(�ޖ���)
		}else{
			ret = 2;												// ���Z�I��(�ޗL��)
		}

		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// ���p�\�}�̻��د��\���p��ϰSTOP
		} else {
			Lagcan( OPETCBNO, 10 );									// ���p�\�}�̻��د��\���p��ϰSTOP
		}
		OpeLcd( 4 );												// �����\��(�������͊�����)

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		// lcdbm_QRans_InCar_status�̍X�Vcheck
		// invcrd()����return����O��call�����invcrd()��ret�ϐ���check����
		lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		return ret;
	}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���������Ɠ����Ɍ�ʌn�̃J�[�h��������M�����Ƃ��A���Z������A�������߂�Ȃ��B
	else if (PayData.Electron_data.Ec.pay_ryo != 0) {
		if ( ryo_buf.turisen == 0 ){
			ret = 1;											// ���Z�I��(�ޖ���)
		}else{
			ret = 2;											// ���Z�I��(�ޗL��)
		}

		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );			// ���p�\�}�̻��د��\���p��ϰSTOP
		} else {
			Lagcan( OPETCBNO, 10 );								// ���p�\�}�̻��د��\���p��ϰSTOP
		}
		OpeLcd( 4 );											// �����\��(�������͊�����)
// MH810103(s) �d�q�}�l�[�Ή� 
		// lcdbm_QRans_InCar_status�̍X�Vcheck
		// invcrd()����return����O��call�����invcrd()��ret�ϐ���check����
		lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810103(e) �d�q�}�l�[�Ή� 
		return ret;
	}
// MH810103 GG119202(E) ���������Ɠ����Ɍ�ʌn�̃J�[�h��������M�����Ƃ��A���Z������A�������߂�Ȃ��B
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
	}
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������

	// ��ryo_buf.dsp_ryo �� ryo_buf.zenkai ���l���i�v�Z�j�ς�
	if( ryo_buf.nyukin < ryo_buf.dsp_ryo ){ 						// �����z < ���ԗ���?
		ryo_buf.zankin = ryo_buf.dsp_ryo - ryo_buf.nyukin;
	}else{
		ryo_buf.zankin = 0;
// MH322914(S) K.Onodera 2016/09/16 AI-V�Ή��F�U�֐��Z
// �s��C��(S) K.Onodera 2016/10/13 #1514 �U�֐�Œ���𗘗p����ƒʒm������Œ�����g�p�����ɐ��Z�������̒ޑK���������Ȃ�
//		if( PiP_GetFurikaeSts() && ryo_buf.dsp_ryo == 0 ){
		if( PiP_GetFurikaeSts() && (ryo_buf.dsp_ryo == 0 ) ){
// �s��C��(E) K.Onodera 2016/10/13 #1514 �U�֐�Œ���𗘗p����ƒʒm������Œ�����g�p�����ɐ��Z�������̒ޑK���������Ȃ�
			ret = 2;												// ���Z�I��(�ޗL��)���ޑK��ryo_cal�ŎZ�o
// MH321800(S) G.So IC�N���W�b�g�Ή�
			if (isEC_USE() != 0) {
				Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );
			} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
			Lagcan( OPETCBNO, 10 );
// MH321800(S) G.So IC�N���W�b�g�Ή�
			}
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			// lcdbm_QRans_InCar_status�̍X�Vcheck
			// invcrd()����return����O��call�����invcrd()��ret�ϐ���check����
			lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			return ret;
// MH810102(S) �d�q�}�l�[�Ή� #5500 ���������Ɠ�����QR�����Ő��Z��������ƌ������ԋp����Ȃ�
//// MH810100(S) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
//		}else if( ryo_buf.dsp_ryo == 0 ){
//			ret = 2;												// ���Z�I��(�ޗL��)���ޑK��ryo_cal�ŎZ�o
//			if (isEC_USE() != 0) {
//				Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );
//			} else {
//				Lagcan( OPETCBNO, 10 );
//			}
//			lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
//			return ret;
//// MH810100(E) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
// MH810102(E) �d�q�}�l�[�Ή� #5500 ���������Ɠ�����QR�����Ő��Z��������ƌ������ԋp����Ȃ�
		}else{
// MH322914(E) K.Onodera 2016/09/16 AI-V�Ή��F�U�֐��Z
		if(( ryo_buf.turisen = ryo_buf.nyukin - ryo_buf.dsp_ryo ) == 0 ){
			ret = 1;												// ���Z�I��(�ޖ���)
			if( OPECTL.Pay_mod == 2 ){								// �C�����Z�̎�
				ret = SyuseiModosiPay( ret );
			}
		}else{
			ret = 2;												// ���Z�I��(�ޗL��)
		}
// MH322914(S) K.Onodera 2016/09/16 AI-V�Ή��F�U�֐��Z
		}
// MH322914(E) K.Onodera 2016/09/16 AI-V�Ή��F�U�֐��Z
	}
	// ���ԗ���(�c�z)�\��
	if( paymod == 0 ){												// �ʏ퐸�Z
// MH321800(S) G.So IC�N���W�b�g�Ή�
		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// ���p�\�}�̻��د��\���p��ϰSTOP
		} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
		Lagcan( OPETCBNO, 10 );										// ���p�\�}�̻��د��\���p��ϰSTOP
// MH321800(S) G.So IC�N���W�b�g�Ή�
		}
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//		if( ryo_buf.credit.pay_ryo != 0 ){
//			creMessageAnaOnOff( 0, 0 );
//		}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

// MH321800(S) G.So IC�N���W�b�g�Ή�
		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// ���p�\�}�̻��د��\���p��ϰSTOP
		} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
		Lagcan( OPETCBNO, 10 );										// ���p�\�}�̻��د��\���p��ϰSTOP
// MH321800(S) G.So IC�N���W�b�g�Ή�
		}
// MH321800(E) G.So IC�N���W�b�g�Ή�
		OpeLcd( 4 );												// �����\��(�������͊�����)

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//		if( ryo_buf.credit.pay_ryo != 0 ){
//			creMessageAnaOnOff( 1, 1 );
//		}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
		ryodsp = 1;
	}else{																				// �C�����Z
		opedpl( 5, 16, ryo_buf.nyukin, 6, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �����z�\��
		opedpl( 6, 16, ryo_buf.zankin, 6, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// �c�z�\��
	}

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	// lcdbm_QRans_InCar_status�̍X�Vcheck
	// invcrd()����return����O��call�����invcrd()��ret�ϐ���check����
	lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���~���A�߂����z�̎Z�o                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : modoshi( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : �߂����z                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
long	modoshi( void )
{
	ulong	set, not_vl, ret;

	ret = ryo_buf.turisen = ryo_buf.nyukin;

	set = CPrmSS[S_SYS][46];										// �����ɑ΂���ő啥�ߊz

	if( set ){
		if( ryo_buf.in_coin[4] != 0 ){								// ���������L��?
			not_vl = ryo_buf.in_coin[4] * 1000L;
			if( not_vl > set ){
				ryo_buf.fusoku = not_vl - set;
				ret = ryo_buf.nyukin - ryo_buf.fusoku;
			}
		}
	}
	ryo_buf.modoshi = ret;											// ���~�߂��z���
	return( (long)ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �߽ܰ�ޓ��͏���(���Z��)                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod70( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  :  1 = ������(�����\��,��������)�֐؊�             |*/
/*|                     : 10 = ���(���~),�ҋ@�֖߂�                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const uchar		KuroMaru[3] = { "��" };

short	op_mod70( void )
{
	short	ret;
	ushort	msg = 0;
	ushort	us_pass = 0;
	uchar	pushcnt = 0;
	ushort	Lk_pass;


	OpeLcd( 9 );													// �Ïؔԍ����͉�ʕ\��

	ope_anm( AVM_P_INPUT );											// �Ïؔԍ����͎��ųݽ

	if( OPECTL.multi_lk ){											// ������Z�⍇����?
		Lk_pass = LOCKMULTI.lock_mlt.passwd;
	}else{
		Lk_pass = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd;
	}

	Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
				BUZPI();
				us_pass = (( us_pass % 1000 ) * 10 )+( msg - KEY_TEN );
				if( pushcnt == 0 ){									// 1�����ړ���
					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[4] );				// "      �Ïؔԍ�  ��������      "
				}
				if( pushcnt < 4 ){
					grachr(5, (ushort)(16+(pushcnt*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ���\��
					pushcnt++;										// �߽ܰ�ޕ\�����͌���(MAX4��)
				}
				if( pushcnt == 4 ){									// 4�����ړ���
					if( Lk_pass == us_pass ){						// �߽ܰ�ނ͐�����?
						if( OPECTL.InLagTimeMode ) {				// ���O�^�C�������������s�H
							OPECTL.Ope_mod = 220;					// ���O�^�C������������
						}else{
							ope_anm(AVM_SHASHITU);					// �Ԏ��ǂݏグ
							OPECTL.Ope_mod = 2;						// �����\��,����������
						}
						ret = 1;
						break;
					}else{
						pushcnt = 0;
						grachr(7, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[42]);			// "   �Ïؔԍ����Ԉ���Ă��܂�   "
						ope_anm( AVM_P_INPNG );						// �Ïؔԍ��ԈႢ���ųݽ
						Lagtim( OPETCBNO, 2, 3*50 );				// ��ϰ2(3s)�N��(�װ�\���p)
					}
				}
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
				break;

			case KEY_TEN_F1:										// �ݷ�[���Z] �����
			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
			case KEY_TEN_F3:										// �ݷ�[��t��] �����
			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
				BUZPIPI();
				break;

			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				ret = 10;											// �ҋ@�֖߂�
				break;

			case KEY_TEN_CL:										// �ݷ�[C] �����
				BUZPI();
				if( us_pass ){
					us_pass = 0;
					pushcnt = 0;
					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[4]);				// "      �Ïؔԍ�  ��������      "
				}
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��
				ret = 10;											// �ҋ@�֖߂�
				break;

			case TIMEOUT2:											// ��ϰ2��ѱ��
				displclr( 7 );										// Display Line Clear
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 2 );											// ��ϰ2ؾ��(�װ�\���p)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �߽ܰ�ޓo�^����(���Ɏ�)                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod80( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = ���(���~),�ҋ@�֖߂�                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod80( void )
{
	short	ret;
	ushort	msg = 0;
	ushort	us_pass = 0;
	uchar	pushcnt = 0;
	uchar	set;
	char	pri_time_flg;

	if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd == 0 ){			// �߽ܰ�ޖ��o�^
		OpeLcd( 8 );												// �߽ܰ�ޓo�^��ʕ\��
		ope_anm( AVM_P_ENTRY );										// �Ïؔԍ��o�^���ųݽ
		Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
	}else{															// �߽ܰ�ޓo�^�ς�
		OpeLcd( 19 );												// �߽ܰ�ޓo�^�ς݉�ʕ\��
		ope_anm( AVM_STOP );										// ������~�ųݽ
		Lagtim( OPETCBNO, 1, 5*50 );								// ��ϰ1(5s)�N��(���ڰ��ݗp)
	}

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
				if(pushcnt <= 3 ){
					BUZPI();
					if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd == 0 ){		// �߽ܰ�ޖ��o�^
						us_pass = (( us_pass % 1000 ) * 10 )+( msg - KEY_TEN );
						pushcnt++;
						set = (uchar)prm_get( COM_PRM,S_TYP,63,1,1 );
						if( ( set == 3 )||( set == 4 ) ){
							blindpasword( us_pass, 4, 3, 16 ,pushcnt);					// �߽ܰ�ޔ�\��
						}else{
							teninb_pass(us_pass, 4, 3, 16 ,pushcnt, COLOR_FIREBRICK);	// �߽ܰ�ޕ\��
						}
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
					}
					break;
				}else{
					BUZPIPI();
					break;
				}

			case KEY_TEN_F3:										// �ݷ�[��t��] �����
				if(pushcnt < 4 ){					//�e���L�[��4�񉟂���Ȃ��Ǝ�t���͔��s���܂���
					BUZPIPI();
					break;
				}
				set = (uchar)prm_get( COM_PRM,S_TYP,62,1,3 );		// ���ԏؖ������s�\�����擾
				pri_time_flg = pri_time_chk(OPECTL.Pr_LokNo,0);
				if( ( us_pass == 0 ) || ( set == 0 ) ||	( pri_time_flg == 0 ) ){
					// �߽ܰ�ޖ���(0000)�@�܂��́@��t�����s�@�\�Ȃ��ݒ�@�܂��͎�t�����s���ԃI�[�o�[�̏ꍇ�A�o�^�����s�s�� 
					BUZPIPI();
					break;
				}

				/*** �߽ܰ�ޓo�^ & ��t�����s ***/
				BUZPI();

				if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd == 0 ){	// �߽ܰ�ޖ��o�^
					FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd = us_pass;// �߽ܰ�ފi�[

					displclr( 6 );									// Line Clear
					if( PriRctCheck() != OK ) {						// ڼ�Ĉ󎚏����^�s������
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[143] );	// "�����A��t���͔��s�ł��܂���  "
					} else {
					if( uke_isu( OPECTL.Op_LokNo, OPECTL.Pr_LokNo, OFF ) == 0 ){	// ��t�����s�i�ʏ�j
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[74] );			// " ��t��������艺����"
						ope_anm( AVM_UKE_PRN );						// ��t�����s�ųݽ
					}else{
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[73] );			// " ��t���͔��s�ς݂ł�"
						ope_anm( AVM_UKE_SUMI );					// ��t�����s�ςݱųݽ
					}
					}
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[39] );				// "  �Ïؔԍ��o�^���������܂���  "
					Lagtim( OPETCBNO, 1, 5*50 );					// ��ϰ1(5s)�N��(���ڰ��ݗp)
				}
				break;

			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
				if(pushcnt < 4 ){					//�e���L�[��4�񉟂���Ȃ��ƃp�X���[�h�̓o�^�͂��܂���
					BUZPIPI();
					break;
				}
				if( us_pass == 0 ){									// �߽ܰ�ޖ���(0000)�͓o�^�s��
					BUZPIPI();
					break;
				}

				/*** �߽ܰ�ޓo�^ ***/
				BUZPI();

				if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd == 0 ){	// �߽ܰ�ޖ��o�^
					FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd = us_pass;// �߽ܰ�ފi�[

					displclr( 6 );									// Line Clear
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[39] );				// "  �Ïؔԍ��o�^���������܂���  "
					Lagtim( OPETCBNO, 1, 5*50 );					// ��ϰ1(5s)�N��(���ڰ��ݗp)
				}
				break;

			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				ret = 10;											// �ҋ@�֖߂�
				break;
			case KEY_TEN_CL:										// �ݷ�[C] �����
				BUZPI();
				if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd != 0 ){	// �߽ܰ�ޓo�^����
					break;
				}
				if( us_pass == 0 && pushcnt == 0){
				}else{
					pushcnt = 0;									//push�J�E���^�[
					us_pass = 0;									// �߽ܰ�޸ر
					teninb_pass(us_pass, 4, 3, 16, pushcnt, COLOR_FIREBRICK);		// �߽ܰ�ޕ\��
				}
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��
				ret = 10;											// �ҋ@�֖߂�
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ��t�����s����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod90( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = ���(���~),�ҋ@�֖߂�                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod90( void )
{
	short	ret;
	ushort	msg = 0;

	if( PriRctCheck() != OK ) {										// ڼ�Ĉ󎚏����^�s������
		OpeLcd( 29 );												// ��t�����sNG
	} else {
	if( uke_isu( OPECTL.Op_LokNo, OPECTL.Pr_LokNo, OFF ) ){			// ��t�����s�i�ʏ�j
		/*** ���s�ς�NG ***/
		OpeLcd( 7 );												// ��t�����sNG
		ope_anm( AVM_UKE_SUMI );									// ��t�����s�ςݱųݽ
	}else{
		/*** ���sOK ***/
		OpeLcd( 6 );												// ��t�����sOK
		ope_anm( AVM_UKE_PRN );										// ��t�����s�ųݽ
	}
	}

	Lagtim( OPETCBNO, 1, 5*50 );									// ��ϰ1(5s)�N��(���ڰ��ݗp)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN_F4:										// �ݷ�[���] �����
				ret = 10;											// �ҋ@�֖߂�
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��
				ret = 10;											// �ҋ@�֖߂�
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)

	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| ��t�����s,�߽ܰ�ޓo�^NG�\��                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod81( void )                                        |*/
/*| PARAMETER    : mod : 81 = �Ïؔԍ�, 91 = ��t��                        |*/
/*| RETURN VALUE : ret  : 10 = ���(���~),�ҋ@�֖߂�                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.Sekiguchi                                             |*/
/*| Date         : 2006-09-26                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod81( uchar mod )
{
	short	ret;
	ushort	msg = 0;

	if( mod == 81 ){			// ��t�����s�i�ʏ�j
		/*** ���s�ς�NG ***/
		OpeLcd( 26 );												// �߽ܰ�ޓo�^NG
	}else if(mod == 91){
		/*** ���sOK ***/
		OpeLcd( 27 );												// ��t�����sNG
	}
	else if(mod == 92){
		OpeLcd( 28 );
	}

	Lagtim( OPETCBNO, 1, 5*50 );									// ��ϰ1(5s)�N��(���ڰ��ݗp)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN_F4:										// �ݷ�[���] �����
				ret = 10;											// �ҋ@�֖߂�
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��
				ret = 10;											// �ҋ@�֖߂�
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)

	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| ���ގg�p�L�����菈��                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : carduse( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 0 = �g�p����                                     |*/
/*|                     : 1 = �g�p�L��                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	carduse( void )
{
	short	ret;

	ret = 0;
	if( card_use[USE_PAS] +											// ������g�p����
	    card_use[USE_PPC] +											// ����߲�޶��ގg�p����
	    card_use[USE_NUM] +											// �񐔌��g�p����
	    card_use[USE_N_SVC] )										// �V�K���޽��,�|����,�������g�p����
	{
		ret = 1;
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���޽���A���}����������                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : svs_renzoku( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ���޽���A�������̂��߁A�ۗ����Ă��錔(SNS6,7)���A���Z�����܂���        |*/
/*| ���Z���~���ɁA����p�����A�����r�oӰ�ނ֑J�ڂ����邽�߂̏���           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nakayama, modified by Hara                            |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	svs_renzoku( void )
{
	if(( CPrmSS[S_DIS][3] == 1 )&&									// ���޽����荞��
	   ( CPrmSS[S_DIS][2] == 0 )){									// �p��ϰ��󎚂��Ȃ�
		/*** �A���}�������� ***/
		if( cr_service_holding == 1 ){								// ���޽���ۗ̕�����
			opr_snd( 10 );											// ���ތ���r�o
			cr_service_holding = 0;									// ���޽���A������ �ۗ�����
		}
	}else{
		/*** �A���}�������Ȃ� ***/
		OPE_red = 2;												// ذ�ް�����r�o
	}
	rd_shutter();													// ���Cذ�ް���������
}

/*[]----------------------------------------------------------------------[]*/
/*| �x�Ə���                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod100( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : -1 = Ӱ�ސؑ�                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod100( void )
{
	short	ret;
	ushort	msg = 0;
	uchar	key_chk;

	short	wk_lev;
	char	f_Card = 0;												// 0:�s���W������ 1:�W�����ގ� -1:�W������NG
	uchar	MntLevel = 0;
	uchar	PassLevel = 0;
	uchar	KakariNo = 0;
	uchar	rd_err_count = 0;

// GG129001(S) �C���{�C�X�Ή��i�t���O�̃N���A�����j
	f_reci_ana = 0;
	OPECTL.f_RctErrDisp = 0;
// GG129001(E) �C���{�C�X�Ή��i�t���O�̃N���A�����j
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
	if (isEC_USE()) {
		Ope_EcEleUseDsp();
	}
	else {
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
	if( Suica_Rec.Data.BIT.CTRL ){								// ��t��ԁH
		Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
	}
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
	}
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( Edy_Rec.edy_status.BIT.INITIALIZE ){					// EdyӼޭ�ق̏��������������Ă���H
//		Edy_StopAndLedOff();									// ���ތ��m��~�w���J�n��LEDOFF
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	memset( DspChangeTime, 0, sizeof( DspChangeTime ));				// ܰ��ر�̏�����
//	
//	DspChangeTime[0] = (ushort)(prm_get(COM_PRM, S_DSP, 30, 1, 1)); // ���p�\�}�̻��د��\���p��ϰ�l�擾
//	if(DspChangeTime[0] <= 0) {
//		DspChangeTime[0] = 2;
//	}
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//	if (isEC_USE() != 0) {
//		Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰSTART
//	} else {
//// MH321800(E) G.So IC�N���W�b�g�Ή�
//	mode_Lagtim10 = 0;										// Lagtimer10�̎g�p��� 0:���p�\�}�̻��د��\��
//	suica_fusiku_flg = 0;									// ���p�\�}�̻��د��\���L��
//	Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );		// ���p�\�}�̻��د��\���p��ϰSTART <--��06/09/15 Suzuki�i���د��\�����Ȃ��j
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//	}
//// MH321800(E) G.So IC�N���W�b�g�Ή�
//
//	OpeLcd( 17 );													// �x�ƒ���ʕ\��
//
//	ope_anm( AVM_STOP );											// ������~�ųݽ
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	if( OPECTL.NtnetTimRec ){										// NT-NET���v��M�׸�ON
		NTNET_Snd_Data104(NTNET_MANDATA_CLOCK);						// NT-NET�Ǘ��ް��v���쐬
		OPECTL.NtnetTimRec = 0;										// NT-NET���v��M�׸�OFF
	}
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	rd_shutter();													// ���Cذ�ް���������
//	OPE_red = 2;													// ذ�ް�����r�o
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	Lagtim( OPETCBNO, 18, RD_ERR_INTERVAL );						// ��ϰ18(10s)������ُ픻��p��ϰ

	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ذ�޺����(���Ԍ��ۗ���)
	}

	if( MIFARE_CARD_DoesUse ){										// Mifare���L���ȏꍇ
		op_MifareStop_with_LED();								 	// Mifare���~����
	}

	Lagtim( OPETCBNO, 1, 10*50 );									// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
	tim1_mov = 1;													// ��ϰ1�N����
	if(( OPECTL.RECI_SW == (char)-1 )&&( RECI_SW_Tim == 0 )){		// �ҋ@���̎������݉�?
		RECI_SW_Tim = (30 * 2) + 1;									// �̎��؎�t���Ԃ�30s�Œ�(500ms�ɂĶ���)
	}
	Lagtim( OPETCBNO, 4, 1*50 );									// ��ϰ4(1s)�N��(�s�������o�ɊĎ��p)����̂�1s�Ƃ���

	OPECTL.Ent_Key_Sts = OFF;										// �o�^��������ԁF�n�e�e
	OPECTL.Can_Key_Sts = OFF;										// �����������ԁF�n�e�e
	OPECTL.EntCan_Sts  = OFF;										// �o�^�������������������ԁF�n�e�e

// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
	// �܂����O����f�[�^���������Ă��Ȃ���Ώ�������
	if( ac_flg.ec_recv_deemed_fg != 0 ){
		EcRecvDeemed_RegistPri();
	}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		if( Op_Event_Disable(msg) )									// �C�x���g�}�~�`�F�b�N
			continue;												// �����ɂ���
		switch( msg ){
			case CLOCK_CHG:											// ���v�X�V
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( edy_auto_com ){
//					queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL ); // �c�x�ƕω��ʒm�o�^
//				}else{
				{
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					// �C���f���g�����ׁ̈A�ȉ��̏����Ƀ^�u������B
					if( opncls() == 1 ){								// �c��?
						ret = -1;
						break;
					}
					else {
						// �x�Ɨ��R�ĕ\��
						if( (0 == (uchar)prm_get(COM_PRM, S_DSP, 35, 1, 1)) ||	// �x�ƕ\������ or 
							(2 == CLOSE_stat) ){								// �c�Ǝ��ԊO(����͏�ɕ\��)
							grachr(0, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, CLSMSG[CLOSE_stat]);	// �x�Ɨ��R�\��
						}
					}
				}
				dspclk(7, COLOR_WHITE);								// ���v�\��
				if( tim1_mov == 0 ){								// ��ϰ1���N��
					LcdBackLightCtrl( OFF );						// back light OFF
				}
				break;

			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�폜
//					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�폜
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					ret = -1;
				}
				break;

			case OPE_OPNCLS_EVT:									// �����c�x�� event
				if( opncls() == 1 ){								// �c��?
					ret = -1;
				} else {
					// �x�Ɨ��R�ĕ\��
					if( (0 == (uchar)prm_get(COM_PRM, S_DSP, 35, 1, 1)) ||	// �x�ƕ\������ or 
						(2 == CLOSE_stat) ){								// �c�Ǝ��ԊO(����͏�ɕ\��)
						grachr(0, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, CLSMSG[CLOSE_stat]);	// �x�Ɨ��R�\��
					}
				}
// MH810105(S) MH364301 �C���{�C�X�Ή�
				if (f_reci_ana) {
					// �̎��؃{�^�������ς݂ł���΁A
					// ���V�[�g�󎚎��s������s��
					op_rct_failchk();
				}
// MH810105(E) MH364301 �C���{�C�X�Ή�
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
			case KEY_TEN_F1:										// �ݷ�[���Z] �����
			case KEY_TEN_F3:										// �ݷ�[��t��] �����
			case KEY_TEN_F4:										// �ݷ�[���] �����
			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
			case KEY_TEN_CL:										// �ݷ�[C] �����
				BUZPIPI();
				LcdBackLightCtrl( ON );								// back light ON
				Lagtim( OPETCBNO, 1, 10*50 );						// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
				tim1_mov = 1;										// ��ϰ1�N����
				break;

			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
				LcdBackLightCtrl( ON );								// back light ON
				Lagtim( OPETCBNO, 1, 10*50 );						// ��ϰ1(10s)�N��(���ڰ��ݐ���p)
				tim1_mov = 1;										// ��ϰ1�N����
				if( OPECTL.RECI_SW == (char)-1 ){					// �ҋ@���̎������݉�?
					BUZPI();
					ryo_isu( 0 );									// �̎��ؔ��s
					OPECTL.RECI_SW = 0;								// �̎������ݖ��g�p
					RECI_SW_Tim = 0;								// �̎��؎�t���Ըر
// MH810105(S) MH364301 �C���{�C�X�Ή�
					f_reci_ana = 1;
// MH810105(E) MH364301 �C���{�C�X�Ή�
				}else{
					BUZPIPI();
				}
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				if (((m_kakari *)&MAGred[MAG_ID_CODE])->kkr_type == 0x20) {		// �W���J�[�h
					f_Card = Kakariin_card();
					if( 1 == f_Card ) {
						if(CPrmSS[S_PAY][17]) {							// �d�����b�N����H
							Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
							// �d�����b�N�������x���ȏ�̌W���J�[�h�}�����̓��b�N����
							if(PassLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
								LcdBackLightCtrl( ON );					// �ޯ�ײ� ON
								OPE_SIG_OUT_DOORNOBU_LOCK(0);			// �ޱ��ނ�ۯ�����
								wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ����o�^(�A�����b�N)
								Ope2_WarningDisp( 6*2, sercret_Str[3]);	// ��ۯ��޲��ݽ�\��
								if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// �ޱ�J��� ��ۯ�������ϰ�l���L���͈͓�
									DoorLockTimer = 1;
									Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// �ޱ�J��� ��ۯ�������ϰ����
								}
							}
						}
					}
					else if( 0 == f_Card ) {							// �s���J�[�h�i���o�^�j
						Ope2_WarningDisp( 6*2, ERR_CHR[3]);				// �s���޲��ݽ�\��
					}
					else {												// �J�[�hNG
						Ope2_WarningDisp( 6*2, ERR_CHR[1]);				// �s���޲��ݽ�\��
					}
				}
				else {
				wk_lev = hojuu_card();
				if( NG != wk_lev && UsMnt_mnyctl_chk() ){			// ��[��������
					// ��[���ނ̏ꍇ
					LcdBackLightCtrl( ON );							// �ޯ�ײ� ON
					OPECTL.Mnt_mod = 4;								// ����ݽӰ�ށ���[��������ݽӰ��
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					OPECTL.Mnt_lev = (char)wk_lev;
					Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
					OPECTL.Kakari_Num = KakariNo;					// �W��No.���Z�b�g
					ret = -1;										// Ӱ�ސؑ�
				}
				}
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					Ope2_WarningDispEnd();							// �װ�\������
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��(���ڰ��ݐ���p)
				tim1_mov = 0;										// ��ϰ1���N��
				LcdBackLightCtrl( OFF );							// back light OFF
				LedReq( CN_TRAYLED, LED_OFF );						// ��ݎ�o�����޲��LED����
				break;

			case MID_RECI_SW_TIMOUT:								// �̎��ؔ��s�\��ϰ��ѱ�Ēʒm
				OPECTL.RECI_SW = 0;									// �̎������ݖ��g�p
				break;

			case TIMEOUT4:											// ��ϰ4��ѱ��
				Lagtim( OPETCBNO, 4, 5*50 );						// ��ϰ4(5s)�N��(�s�������o�ɊĎ��p)
				break;

			case TIMEOUT6:											// ��ϰ6��ѱ�āF�u�o�^�v�����u����v������������ԊĎ���ϰ

				if( READ_SHT_flg == 1 ){							// �������ԁH
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// ���C���[�_�Ȃ��̏ꍇ�̓V���b�^�[�J���Ȃ�
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ��ϰ7�i������J��Ԍp���Ď���ϰ�j�N���i�R�O�b�j
					read_sht_opn();									// �̏ꍇ�F�J����
				}
				else{
					read_sht_cls();									// �J�̏ꍇ�F����
				}
				break;

			case TIMEOUT7:											// ��ϰ7��ѱ�āF������J��Ԍp���Ď���ϰ

				rd_shutter();										// ���Cذ�ް���������
				break;
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			case TIMEOUT10:								// �uSuica��~��̎�t�����M�҂��vor�u������ʕ\���ؑցv����ѱ�Ēʒm
//				dspCyclicMsgRewrite(18);	// �T�C�N���b�N�\���X�V
//				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
//				break;
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//			case EC_CYCLIC_DISP_TIMEOUT:								// �uec��~��̎�t�����M�҂��vor�u������ʕ\���ؑցv����ѱ�Ēʒm
//				dspCyclicMsgRewrite(18);								// �T�C�N���b�N�\���X�V
//				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
//				break;
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			case TIMEOUT18:
				shutter_err_chk( &rd_err_count );					// ���Cذ�ް�ُ�������
				break;
			case OPE_REQ_CALC_FEE:									// �����v�Z�v��
				// �����v�Z
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);				// �S�Ԏ����e�[�u�����M
				break;

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			// QR�m��E����f�[�^����(lcdbm_rsp_QR_conf_can_res_t)
			case LCD_QR_CONF_CAN_RES:
				break;

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			// ���[�����j�^�f�[�^����(lcdbm_rsp_LANE_res_t)
			case LCD_LANE_DATA_RES:
				break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

			// QR�f�[�^
			case LCD_QR_DATA:
				// QR�����f�[�^
				lcdbm_QR_data_res( 1 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
				break;

			// ���Ϗ���ް�(lcdbm_ICC_Settlement_Status)
			case LCD_ICC_SETTLEMENT_STS:
				break;

			// ���ό��ʏ��(lcdbm_ICC_Settlement_Result)
			case LCD_ICC_SETTLEMENT_RSLT:
				break;

			// ����ʒm(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// ����ʒm		���캰��
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// ���Z���~�v��
					case LCDBM_OPCD_PAY_STP:
						// ����ʒm(���Z���~����(NG))���M	0=OK/1=NG
// MH810103(s) �d�q�}�l�[�Ή� �x�Ǝ��͒��~�v����OK��ԋp����
//						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
// MH810103(e) �d�q�}�l�[�Ή� �x�Ǝ��͒��~�v����OK��ԋp����
						break;

					// ���Z���~����		0=OK/1=NG
					case LCDBM_OPCD_PAY_STP_RES:
						break;

					// ���Z�J�n�v��			0�Œ�
					case LCDBM_OPCD_PAY_STA:
						// ����ʒm(���Z�J�n����(NG))���M	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
						break;

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// 					// ���u���Z�J�n����		0=OK/1=NG
// 					case LCDBM_OPCD_RMT_PAY_STA_RES:
// 						break;
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

					// �N���ʒm
					case LCDBM_OPCD_STA_NOT:
						// �N��Ӱ�� = �ݒ豯��۰�ދN��
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(�N����)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// �N��Ӱ�� = �ʏ�N��
						else {
							// op_mod00(�ҋ@)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;
				}
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(�N����)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// ��������������ԂƂ���
				ret = -1;
				break;

			// IC�ڼޯĒ�~�҂����
			case TIMEOUT_ICCREDIT_STOP:		// TIMERNO_ICCREDIT_STOP
				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
			case EC_RECEPT_SEND_TIMEOUT:							// ec�p��ϰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,0 );
				break;
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
// MH810105(S) MH364301 �C���{�C�X�Ή�
			case PRIEND_PREQ_RYOUSYUU:
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
// // GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
// //			if (IS_INVOICE) {
// 				if (!IS_INVOICE) {
// // GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//					break;
//				}
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
				if (f_reci_ana && OPECTL.Pri_Result == PRI_NML_END) {
					f_reci_ana = 0;
				}
				break;
// MH810105(E) MH364301 �C���{�C�X�Ή�
			default:
				break;
		}
		key_chk = op_key_chek();									// �u�o�^�v�L�[���u����v�L�[�̓���������ԊĎ�
		switch( key_chk ){
			case	1:												// ����������ԊJ�n
				Lagtim( OPETCBNO, 6, 3*50 );						// ��ϰ6�i���������Ď���ϰ�j�N���i�R�b�j
				break;
			case	2:												// ����������ԉ���
				Lagcan( OPETCBNO, 6 );								// ��ϰ6�i���������Ď���ϰ�jؾ��
				break;
		}

		lto_syuko();												// �ׯ�ߏ㏸�A���b�N����ϓ��o��
		fus_kyo();													// �s���E�����W�v����
		mc10exec();
	}

	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 4 );											// ��ϰ4ؾ��(�s�������o�ɊĎ��p)
	Lagcan( OPETCBNO, 6 );											// ��ϰ6ؾ��
	Lagcan( OPETCBNO, 7 );											// ��ϰ7ؾ��
	Lagcan( OPETCBNO, 10 );											// ���p�\�}�̻��د��\���p��ϰSTOP
	Lagcan( OPETCBNO, 18 );											// ��ϰ18(10s)������ُ픻��p��ϰSTOP
// MH321800(S) G.So IC�N���W�b�g�Ή�
	Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );					// ec���T�C�N���\���p��ϰؾ��
// MH321800(E) G.So IC�N���W�b�g�Ή�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
	LagCan500ms(LAG500_RECEIPT_MISS_DISP_TIMER);					// �̎��؎��s�\���^�C�}
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j

	if( OPECTL.Mnt_mod != 4 ){										// �u��[���ގ�t�����v�ȊO�H
		rd_shutter();												// ���Cذ�ް���������
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �ޱ���װѕ\������                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod110( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : -1 = Ӱ�ސؑ�                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod110( void )
{
	short	ret;
	ushort	msg = 0;
	char	f_Card = 0;												// 0:�s���W������ 1:�W�����ގ� -1:�W������NG
	uchar	MntLevel = 0;
	uchar	PassLevel = 0;
	uchar	KakariNo = 0;
	uchar	key_chk;
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	LcdBackLightCtrl( ON );											// back light ON
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	Suica_Ctrl( S_CNTL_DATA, 0x00 );								// Suica���p��s�ɂ���
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	OpeLcd( 18 );													// �ޱ���̱װѕ\��
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	if( OPECTL.NtnetTimRec ){										// NT-NET���v��M�׸�ON
		NTNET_Snd_Data104(NTNET_MANDATA_CLOCK);						// NT-NET�Ǘ��ް��v���쐬
		OPECTL.NtnetTimRec = 0;										// NT-NET���v��M�׸�OFF
	}
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	OPE_red = 2;													// ذ�ް�����r�o
//	if(( RD_mod != 10 )&&( RD_mod != 11 )){
//		opr_snd( 3 );												// ذ�޺����(���Ԍ��ۗ���)
//	}
//	ope_anm( AVM_STOP );											// ������~�ųݽ
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	Lagtim( OPETCBNO, 1, 2*50 );									// ��ϰ1(2s)�N��(���ڰ��ݐ���p)
	BUZPIPIPI();

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					ret = -1;
				}
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
			case KEY_TEN_F1:										// �ݷ�[���Z] �����
			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
			case KEY_TEN_F3:										// �ݷ�[��t��] �����
			case KEY_TEN_F4:										// �ݷ�[���] �����
			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
			case KEY_TEN_CL:										// �ݷ�[C] �����
				BUZPIPI();
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				f_Card = Kakariin_card();
				if( 1 == f_Card ) {
					if(CPrmSS[S_PAY][17]) {							// �d�����b�N����H
						Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
						// �d�����b�N�������x���ȏ�̌W���J�[�h�}�����̓��b�N����
						if(PassLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
							LcdBackLightCtrl( ON );					// �ޯ�ײ� ON
							OPE_SIG_OUT_DOORNOBU_LOCK(0);			// �ޱ��ނ�ۯ�����
							wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ����o�^(�A�����b�N)
							Ope2_WarningDisp( 6*2, sercret_Str[3]);	// ��ۯ��޲��ݽ�\��
							if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// �ޱ�J��� ��ۯ�������ϰ�l���L���͈͓�
								DoorLockTimer = 1;
								Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// �ޱ�J��� ��ۯ�������ϰ����
							}
						}
					}
				}
				else if( 0 == f_Card ) {							// �s���J�[�h�i���o�^�j
					Ope2_WarningDisp( 6*2, ERR_CHR[3]);				// �s���޲��ݽ�\��
				}
				else {												// �J�[�hNG
					Ope2_WarningDisp( 6*2, ERR_CHR[1]);				// �s���޲��ݽ�\��
				}
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					Ope2_WarningDispEnd();							// �װ�\������
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			case KEY_DOOR1:
				if( OPECTL.on_off == 0 ){							// OFF(Door Close)
					if( DoorCloseChk() == 100 ){
						OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
						ret = -1;
						break;
					}
					OpeLcd( 18 );									// �ޱ���̱װэĕ\��
				}
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��(���ڰ��ݐ���p)
				if( DoorCloseChk() == 100 ){						// �ޱ������ OK(�ُ한��)
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					ret = -1;
					break;
				}
				Lagtim( OPETCBNO, 1, 2*50 );						// ��ϰ1(2s)�N��(���ڰ��ݐ���p)
				BUZPIPIPI();
				break;
			case TIMEOUT6:											// ��ϰ6��ѱ�āF�u�o�^�v�����u����v������������ԊĎ���ϰ

				if( READ_SHT_flg == 1 ){							// �������ԁH
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// ���C���[�_�Ȃ��̏ꍇ�̓V���b�^�[�J���Ȃ�
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ��ϰ7�i������J��Ԍp���Ď���ϰ�j�N���i�R�O�b�j
					read_sht_opn();									// �̏ꍇ�F�J����
				}
				else{
					read_sht_cls();									// �J�̏ꍇ�F����
				}
				break;
			case TIMEOUT7:											// ��ϰ7��ѱ�āF������J��Ԍp���Ď���ϰ

				rd_shutter();										// ���Cذ�ް���������
				break;
			case OPE_REQ_CALC_FEE:									// �����v�Z�v��
				// �����v�Z
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);				// �S�Ԏ����e�[�u�����M
				break;

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			// QR�m��E����f�[�^����(lcdbm_rsp_QR_conf_can_res_t)
			case LCD_QR_CONF_CAN_RES:
				break;

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			// ���[�����j�^�f�[�^����(lcdbm_rsp_LANE_res_t)
			case LCD_LANE_DATA_RES:
				break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

			// QR�f�[�^
			case LCD_QR_DATA:
				// QR�����f�[�^
				lcdbm_QR_data_res( 1 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
				break;

			// ���Ϗ���ް�(lcdbm_ICC_Settlement_Status)
			case LCD_ICC_SETTLEMENT_STS:
				break;

			// ���ό��ʏ��(lcdbm_ICC_Settlement_Result)
			case LCD_ICC_SETTLEMENT_RSLT:
				break;

			// ����ʒm(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// ����ʒm		���캰��
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// ���Z���~�v��
					case LCDBM_OPCD_PAY_STP:
						// ����ʒm(���Z���~����(NG))���M	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						break;

					// ���Z�J�n�v��			0�Œ�
					case LCDBM_OPCD_PAY_STA:
						// ����ʒm(���Z�J�n����(NG))���M
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
						break;

					// �N���ʒm
					case LCDBM_OPCD_STA_NOT:
						// �N��Ӱ�� = �ݒ豯��۰�ދN��
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(�N����)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// �N��Ӱ�� = �ʏ�N��
						else {
							// op_mod00(�ҋ@)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;
				}
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(�N����)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// ��������������ԂƂ���
				ret = -1;
				break;

			// IC�ڼޯĒ�~�҂����
			case TIMEOUT_ICCREDIT_STOP:		// TIMERNO_ICCREDIT_STOP
				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

			default:
				break;
		}
		key_chk = op_key_chek();									// �u�o�^�v�L�[���u����v�L�[�̓���������ԊĎ�
		switch( key_chk ){
			case	1:												// ����������ԊJ�n
				Lagtim( OPETCBNO, 6, 3*50 );						// ��ϰ6�i���������Ď���ϰ�j�N���i�R�b�j
				break;
			case	2:												// ����������ԉ���
				Lagcan( OPETCBNO, 6 );								// ��ϰ6�i���������Ď���ϰ�jؾ��
				break;
		}
	}

	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ïؔԍ�(Btype)�o�ɑ���                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod200( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : -1 = Ӱ�ސؑ�                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Nishizato                                             |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod200( void )
{
	short	ret;
	ushort	msg = 0;
	short	ans;
	uchar	key_chk;

	LcdBackLightCtrl( ON );											// back light ON
	OpeLcd( 24 );													// �Ïؔԍ��o�ɑ��� �Ԏ����͉�� �\��

	OPECTL.multi_lk = 0L;											// ������Z�⍇�����Ԏ���
	OPECTL.ChkPassSyu = 0;											// ������⍇��������ر
	OPECTL.ChkPassPkno = 0L;										// ������⍇�������ԏꇂ�ر
	OPECTL.ChkPassID = 0L;											// ������⍇���������ID�ر

	// TODO:�Ïؔԍ������o�ɑҋ@��ʕ��A�^�C�}�[�͕W���p�̋��ʃp�����[�^���擾���邱��
	Lagtim( OPETCBNO, 1, 60*50+1 );								// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.multi_lk ){								// ������Z�⍇�����ͷ��͗����Ȃ�
					BUZPIPI();
					break;
				}
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}else{
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
				}
				ret = -1;
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
				if( OPECTL.multi_lk ){								// ������Z�⍇�����ͷ��͗����Ȃ�
					BUZPIPI();
					break;
				}
				BUZPI();
				key_num = (( key_num % knum_ket ) * 10 )+( msg - KEY_TEN );
				teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
				break;

			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
			case KEY_TEN_F3:										// �ݷ�[��t��] �����
			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
			case KEY_TEN_CL:										// �ݷ�[C] �����
				BUZPIPI();
				break;

			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				if( key_num == 0 ){
					ret = 10;										// �ҋ@�֖߂�
				}else{
					key_num = 0;									// ���Ԉʒu�ԍ��ر
					teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
				}
				break;

			case KEY_TEN_F1:										// �ݷ�[�m�F] �����
				if( OPECTL.multi_lk ){								// ������Z�⍇�����ͷ��͗����Ȃ�
					BUZPIPI();
					break;
				}
				BUZPI();
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)

				ans = carchk( key_sec, key_num, 0 );				// ���Ԉʒu�ԍ�����
				if (((ans == 10) || (ans == -1)) ||
					 ((ans == 2) && (FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mode == FLAP_CTRL_MODE1))) {	// �K��O or ���Z�s�\ or (�o�ɏ�� and �ҋ@���)
					key_num = 0;
					teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��������
					break;
				}

				//�Ïؔԍ�(Btype)���͑���֑J��
				OPECTL.Ope_mod = 210;								// �Ïؔԍ�(Btype)���͑����
				ret = 1;

				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��
				if( OPECTL.multi_lk ){								// ������Z�⍇����?
					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
				}else{
					ret = 10;
				}
				break;

			case IBK_NTNET_LOCKMULTI:								// NTNET ������Z�p�ް�(�Ԏ��⍇�������ް�)��M
				if( LOCKMULTI.Answer == 0 && OPECTL.multi_lk )		// ���ʂ�����
				{
					ans = carchk_ml( key_sec, key_num );			// ���Ԉʒu�ԍ�����
					if ((ans != 10) && (ans != -1)) {	// �K��O �ȊO and ���Z�s�� �ȊO
						OPECTL.Ope_mod = 210;						// �Ïؔԍ�(Btype)���͑����
						ret = 1;
						break;
					}
				}
				// no break
			case TIMEOUT2:											// ��ϰ2��ѱ��
				if( OPECTL.multi_lk ){
					OPECTL.multi_lk = 0;

					key_num = 0;
					blink_end();									// �_�ŏI��
					teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��
					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );				// "                              "
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );				// "                              "
					if( LOCKMULTI.Answer ){							// ���ʂ����ڑ�(�Y������)
						grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[6] );			// "  �Ԏ��ԍ�������������܂���  "
					}else{
						grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR3[7] );			// "  ���̐��Z�@�ő��삵�ĉ�����  "
					}
					Lagtim( OPETCBNO, 4, 5*50 );					// ��ϰ4(5s)�N��(�װ�\���p)
					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
				}
				break;

			case TIMEOUT4:											// ��ϰ4��ѱ��
				if( OPECTL.multi_lk == 0 ){
					LCDNO = (ushort)-1;
					OpeLcd( 2 );									// ���Ԉʒu�ԍ��\��(�ĕ\��)
				}
				break;
			case TIMEOUT6:											// ��ϰ6��ѱ�āF�u�o�^�v�����u����v������������ԊĎ���ϰ

				if( READ_SHT_flg == 1 ){							// �������ԁH
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// ���C���[�_�Ȃ��̏ꍇ�̓V���b�^�[�J���Ȃ�
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ��ϰ7�i������J��Ԍp���Ď���ϰ�j�N���i�R�O�b�j
					read_sht_opn();									// �̏ꍇ�F�J����
				}
				else{
					read_sht_cls();									// �J�̏ꍇ�F����
				}
				break;
			case TIMEOUT7:											// ��ϰ7��ѱ�āF������J��Ԍp���Ď���ϰ

				rd_shutter();										// ���Cذ�ް���������
				break;

			default:
				break;
		}
		key_chk = op_key_chek();									// �u�o�^�v�L�[���u����v�L�[�̓���������ԊĎ�
		switch( key_chk ){
			case	1:												// ����������ԊJ�n
				Lagtim( OPETCBNO, 6, 3*50 );						// ��ϰ6�i���������Ď���ϰ�j�N���i�R�b�j
				break;
			case	2:												// ����������ԉ���
				Lagcan( OPETCBNO, 6 );								// ��ϰ6�i���������Ď���ϰ�jؾ��
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(���ڰ��ݐ���p)
	Lagcan( OPETCBNO, 2 );											// ��ϰ2ؾ��(������Z�⍇���Ď�)
	Lagcan( OPETCBNO, 4 );											// ��ϰ4ؾ��(�װ�\���p)
	blink_end();													// �_�ŏI��

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ïؔԍ�(Btype)���͑���                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod210( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : -1 = Ӱ�ސؑ�                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Nishizato                                             |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod210( void )
{
	uchar	visitor_code[6];		//���q�l���ފi�[�ر
	short	ret;
	ushort	loklen;
	ushort	msg = 0;
	ushort	pass_num = 0;
	ushort	work = 0;
	uchar	pushcnt = 0;
	uchar	endflg = 0;
	uchar	key_chk;
	struct CAR_TIM wk_tm1,wk_tm2;

	lcd_backlight( ON );											// back light ON
	OpeLcd( 25 );													// �Ïؔԍ��o�ɑ��� �Ïؔԍ����͉�� �\��

	//�Ԏ��ԍ��\��
	loklen = binlen((ulong)key_num);
	opedsp(1, 12, key_num, loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);	// �Ԏ��ԍ��\��

	//���q�l���ލ쐬�A�\���A�\��������Ͼ��
	MakeVisitorCode(visitor_code);									// ���q�l���ލ쐬(4��+1��)
																	// visitor_code[0]�`[3]�F����
																	//            [4]�F��
																	//            [5]�F\0
	grachr( 6, 16, 5, 0, COLOR_BLACK, LCD_BLINK_OFF, visitor_code);	// "  ���q�l�R�[�h nnnnn          "
	//���q�l���ޕ\�����Ԏ擾
	work = (ushort)CPrmSS[S_PAY][41];
	work = (work % 10000) / 10;
	if (work != 0) {
		Lagtim( OPETCBNO, 1,  (ushort)(work*50));	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
	}

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
					OPECTL.Mnt_mod = 1;								// ����ݽӰ�ޑI��������
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}else{
					OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
				}
				ret = -1;
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
				if(pushcnt <= 3 ){
					BUZPI();
					pass_num = (( pass_num % 1000 ) * 10 )+( msg - KEY_TEN );
					pushcnt++;	
					teninb_pass(pass_num, 4, 3, 16 ,pushcnt, COLOR_FIREBRICK);	// �߽ܰ�ޕ\��
					if (work != 0) {
						Lagtim( OPETCBNO, 1,  (ushort)(work*50));	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
					}
					break;
				}else{
					BUZPIPI();
					break;
				}
			case KEY_TEN_F2:										// �ݷ�[�̎���] �����
			case KEY_TEN_F3:										// �ݷ�[��t��] �����
			case KEY_TEN_F5:										// �ݷ�[�o�^] �����
				BUZPIPI();
				break;
			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				if (endflg == 1) {									// �����o�ɏ���������������
					endflg = 0;										// ���������t���O�N���A
				}
				ret = 10;											// �ҋ@�֖߂�
				break;
			case KEY_TEN_CL:										// �ݷ�[���] �����
				BUZPI();
				if( endflg == 1 ){									// �Ïؔԍ��ر�A�����o�ɏ���������������
					break;
				}
				if( pass_num == 0 && pushcnt == 0){
				}else{
					pass_num = 0;									// �Ïؔԍ��ر
					pushcnt = 0;									//push�J�E���^�[
					teninb_pass(0, 4, 3, 16, 0, COLOR_FIREBRICK);
					if (work != 0) {
						Lagtim( OPETCBNO, 1,  (ushort)(work*50));	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
					}
				}
				break;
			case KEY_TEN_F1:										// �ݷ�[�m�F] �����
				//�Ïؔԍ�(Btype)����
				if (CheckVisitorCode(visitor_code, pass_num) == 0) {
					//�Ïؔԍ�(Btype)���������Ȃ�������
					BUZPIPI();
					pass_num = 0;									// �Ïؔԍ��ر
					pushcnt = 0;									//push�J�E���^�[
					teninb_pass(0, 4, 3, 16, 0, COLOR_FIREBRICK);
					if (work != 0) {
						Lagtim( OPETCBNO, 1,  (ushort)(work*50));	// ��ϰ1(XXs)�N��(���ڰ��ݐ���p)
					}
					break;
				}
				BUZPI();
				work = 0;												// ���Z�ē�
				if ((prm_get( COM_PRM,S_TYP,63,1,1 ))&&					// �Ïؔԍ��o�^���� ����
					(FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd != 0))	// �Ïؔԍ����o�^����Ă���
				{
					if(prm_get( COM_PRM,S_PAY,41,1,5 ) == 1 ){			// �����������ԍ������o��
						work = 1;										// �����o��
					}else{
						work = 2;										// �Ïؔԍ�����
					}
					endflg = 1;											// �Ïؔԍ��ر�����ʒm
				}
				else
				{
					if(prm_get( COM_PRM,S_PAY,41,1,5 ) == 1 ){			// �����������ԍ������o��
						work = 1;										// �����o��
					}else{
						wk_tm1.year = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].year;
						wk_tm1.mon  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mont;
						wk_tm1.day  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].date;
						wk_tm1.hour = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].hour;
						wk_tm1.min  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].minu;

						wk_tm2.year = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_year;
						wk_tm2.mon  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_mont;
						wk_tm2.day  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_date;
						wk_tm2.hour = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_hour;
						wk_tm2.min  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_minu;
						if(ec64(&wk_tm1,&wk_tm2) != -1 ){				// ���Ɏ������o�Ɏ���
						// ���Z�ς̏ꍇ->�����o��
							work = 1;									// �����o��
						}
					}
					endflg = 1;											// �����o�Ɋ����ʒm
				}
				if( work == 1 ){	// �����o��
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
//					queset( FLPTCBNO, FLAP_DOWN_SND, sizeof( OPECTL.Pr_LokNo ), &OPECTL.Pr_LokNo );	// �C�����Z�pۯ����u�J(�ׯ�߉��~)
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[3] );					// "  �����o�ɂ��s���܂�          "
					// ���b�N���u
					wopelg(OPLOG_LOCK_OPEN, 0, (ulong)OPECTL.Pr_LokNo);
					endflg = 1;										// �����o�Ɋ����ʒm
				}else if( work == 2 ){	// �Ïؔԍ��N���A�`���Z�ē�
					FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd = 0;	// �Ïؔԍ�(Atype)�ر
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[4] );					// "  ���Z���Ă�������            "
					wopelg(OPLOG_ANSHOU_B_CLR, 0, (ulong)OPECTL.Pr_LokNo);
					endflg = 1;										// �����o�Ɋ����ʒm
				}else{		// ���Z�ē�
					// �����Z�̏ꍇ
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[4] );					// "  ���Z���Ă�������            "
					endflg = 1;										// �����o�Ɋ����ʒm
				}
				//5�b�őҋ@��Ԃ֑J��
				Lagtim( OPETCBNO, 2, 5*50 );						// ��ϰ2(5s)�N��(���ڰ��ݐ���p)
				pass_num = 0;										//����CL�������ł����ɑҋ@��Ԃ֖߂邽�߂̏���
				break;
			case TIMEOUT6:											// ��ϰ6��ѱ�āF�u�o�^�v�����u����v������������ԊĎ���ϰ
				if( READ_SHT_flg == 1 ){							// �������ԁH
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// ���C���[�_�Ȃ��̏ꍇ�̓V���b�^�[�J���Ȃ�
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ��ϰ7�i������J��Ԍp���Ď���ϰ�j�N���i�R�O�b�j
					read_sht_opn();									// �̏ꍇ�F�J����
				}
				else{
					read_sht_cls();									// �J�̏ꍇ�F����
				}
				break;
			case TIMEOUT7:											// ��ϰ7��ѱ�āF������J��Ԍp���Ď���ϰ
				rd_shutter();										// ���Cذ�ް���������
				break;
			case TIMEOUT1:											// ��ϰ1��ѱ��
			case TIMEOUT2:											// ��ϰ2��ѱ��
				ret = 10;											// �ҋ@�֖߂�
				break;
			default:
				break;
		}
		key_chk = op_key_chek();									// �u�o�^�v�L�[���u����v�L�[�̓���������ԊĎ�
		switch( key_chk ){
			case	1:												// ����������ԊJ�n
				Lagtim( OPETCBNO, 6, 3*50 );						// ��ϰ6�i���������Ď���ϰ�j�N���i�R�b�j
				break;
			case	2:												// ����������ԉ���
				Lagcan( OPETCBNO, 6 );								// ��ϰ6�i���������Ď���ϰ�jؾ��
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��
	Lagcan( OPETCBNO, 2 );											// ��ϰ2ؾ��

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���O�^�C����������                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod220( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = ���(���~),�ҋ@�֖߂�                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Nagashima                                             |*/
/*| Date         : 2010-10-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
short	op_mod220( void )
{
	short	ret;
	ushort	msg = 0;
	uchar	timeout1 = OFF;

	FLAPDT.flp_data[OPECTL.Pr_LokNo-1].timer = LagTimer( OPECTL.Pr_LokNo );
																	// Lag time restart

	if( prm_get(COM_PRM, S_TYP, 68, 1, 3) == 2 &&					// ���O�^�C���^�C���A�b�v��̍ē��Ɏ��������Z��������
	    prm_get(COM_PRM, S_TYP, 80, 1, 3) == 1 ){					// �����X�V����
		// ���O�^�C���������쎞���i���ݎ����j�𐸎Z�����ɐݒ肷��B
		memcpy( &FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_year, &CLK_REC, 6 );
	}

	Flp_LagExtCnt[OPECTL.Pr_LokNo-1] += 1;							// �Ԏ������O�^�C�������񐔁{�P

//	wptoff();
	sky.tsyuk.Lag_extension_cnt += 1;								// �W�v�|���O�^�C�������񐔁{�P
//	wpton();

	wmonlg(OPMON_LAG_EXT, NULL, OPECTL.Op_LokNo);					// ���j�^�o�^�i���O�^�C�������FR0504-xxxx�j

	if( prm_get(COM_PRM, S_TYP, 80, 1, 2) ){						// ���O�^�C��������������
		if( Flp_LagExtCnt[OPECTL.Pr_LokNo-1] >= prm_get(COM_PRM, S_TYP, 80, 1, 1)){	// ���O�^�C����������񐔓��B
			alm_chk2(ALMMDL_MAIN, ALARM_LAG_EXT_OVER, 2, 2, 1, &OPECTL.Op_LokNo);	// �A���[���o�^�i���O�^�C����������K��񐔃I�[�o�[�FA0030-xxxx�j
		}
	}else{															// ���O�^�C�����������Ȃ�
		if( prm_get(COM_PRM, S_TYP, 80, 1, 1) &&					// ���O�^�C���������肠��
		    Flp_LagExtCnt[OPECTL.Pr_LokNo-1] >= prm_get(COM_PRM, S_TYP, 80, 1, 1)){	// ���O�^�C����������񐔓��B
			alm_chk2(ALMMDL_MAIN, ALARM_LAG_EXT_OVER, 2, 2, 1, &OPECTL.Op_LokNo);	// �A���[���o�^�i���O�^�C����������K��񐔃I�[�o�[�FA0030-xxxx�j
			Flp_LagExtCnt[OPECTL.Pr_LokNo-1] = 0;					// ���O�^�C�������񐔃N���A
		}
	}

	OpeLcd( 30 );													// �o�Ɉē�
	if( OPECTL.Pr_LokNo >= LOCK_START_NO ){
		ope_anm( AVM_KANRYO );										// ���Z�������A�i�E���X�i���b�N�j
	}else{
		ope_anm( AVM_KANRYO2 );										// ���Z�������A�i�E���X�i�t���b�v�j
	}

	// �ȉ��A�u��t�����s����(op_mod90())�v��藬�p
	Lagtim( OPETCBNO, 1, 5*50 );									// ��ϰ1(5s)�N��(�ݷ���t�s���ԗp)
	Lagtim( OPETCBNO, 2, 10*50 );									// ��ϰ2(10s)�N��(�o�Ɉē��\���p)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
			case KEY_TEN_CL:										// �ݷ�[���] �����
				if( timeout1 == OFF ) {								// ��ϰ1��N�����H
					break;											// �ݷ���t�s��
				}
				// no break

			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();
				ret = 10;											// �ҋ@�֖߂�
				break;

			case TIMEOUT1:											// ��ϰ1��ѱ��
				timeout1 = ON;
				break;

			case TIMEOUT2:											// ��ϰ2��ѱ��
				ret = 10;											// �ҋ@�֖߂�
				break;

			case ARC_CR_R_EVT:										// ����IN
				cr_service_holding = 0;								// ���޽���ۗ̕�����
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				break;

			case ARC_CR_EOT_EVT:									// ���ޔ������
				if( RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(�ݷ���t�s���ԗp)
	Lagcan( OPETCBNO, 2 );											// ��ϰ2ؾ��(�o�Ɉē��\���p)

	OPECTL.InLagTimeMode = OFF;										// ���O�^�C���������[�h�n�e�e

	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| ���Z�ς݈ē�����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod230( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = �ҋ@�֖߂�                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.Suzuki                                                |*/
/*| Date         : 2011-06-24                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
short	op_mod230( void )
{
	short	ret;
	ushort	msg = 0;
// MH810100(S) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
	ushort	res_wait_time;
	uchar	end_req = 0;
// MH810100(E) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
// MH810100(S) K.Onodera  2020/03/05 �Ԕԃ`�P�b�g���X(���Z������ʕ\�����Ԃ��ݒ�l�ɏ]��Ȃ��s��C��)
	ushort	dsp_tm;
// MH810100(E) K.Onodera  2020/03/05 �Ԕԃ`�P�b�g���X(���Z������ʕ\�����Ԃ��ݒ�l�ɏ]��Ȃ��s��C��)

// MH810100(S) K.Onodera  2020/03/05 �Ԕԃ`�P�b�g���X(���Z������ʕ\�����Ԃ��ݒ�l�ɏ]��Ȃ��s��C��)
//	OpeLcd( 31 );													// ���Z�ς݈ē�
	lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 2 );					// ���Z�����ʒm(���Z�ς݈ē�)
// MH810100(E) K.Onodera  2020/03/05 �Ԕԃ`�P�b�g���X(���Z������ʕ\�����Ԃ��ݒ�l�ɏ]��Ȃ��s��C��)

// MH810100(S) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// �f�[�^��M�Ď��^�C�}
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// �͈͊O��5�b
	}
// MH810100(E) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
// MH810100(S) K.Onodera  2020/03/05 �Ԕԃ`�P�b�g���X(���Z������ʕ\�����Ԃ��ݒ�l�ɏ]��Ȃ��s��C��)
//	Lagtim( OPETCBNO, 1, 5*50 );									// ��ϰ1(5s)�N��(���ڰ��ݐ���p)
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
//	dsp_tm = (ushort)prm_get( COM_PRM, S_TYP, 128, 4, 1 );			// ���Z�����ē��^�C�}�[
	// �{�����̓��O�^�C�������Z��p=0�~���Z�m��̂��߁A�^�C�}�[�ɂ͕K���ʂ̐ݒ�l���Z�b�g����
	dsp_tm = (ushort)prm_get( COM_PRM, S_TYP, 129, 2, 2 );			// ���Z�����ē��^�C�}�[
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i0�~���Z���̐��Z������ʕ\�����Ԃ�Z������iGM804102�Q�l�j�iGM803003���p�j
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i��ʕ\�����ԒZ�k�d�l�̋����j�iGM804102���p�j�iGM803003���p�j
	// ���O�^�C�������Z���͌Œ�^�C�}�[(1s)���쓮���Ȃ����߁A���̑���0�~���Z���ƕ\�����Ԃ������ɂȂ�悤�^�C�}�[�l��␳����B
	dsp_tm = dsp_tm + 1;
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i��ʕ\�����ԒZ�k�d�l�̋����j�iGM804102���p�j�iGM803003���p�j
	Lagtim( OPETCBNO, 1, dsp_tm*50 );								// ��ϰ1�N��(���ڰ��ݐ���p)
// MH810100(E) K.Onodera  2020/03/05 �Ԕԃ`�P�b�g���X(���Z������ʕ\�����Ԃ��ݒ�l�ɏ]��Ȃ��s��C��)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN_F4:										// �ݷ�[���] �����
				BUZPI();

			case TIMEOUT1:											// ��ϰ1��ѱ��
// MH810100(S) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
//				ret = 10;											// �ҋ@�֖߂�
				// ���Z�ē��I���ʒm
				lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
				// �����҂��^�C�}�[�J�n
				Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				end_req = 1;
// MH810100(E) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
				break;

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

			// QR�f�[�^
			case LCD_QR_DATA:
				// QR�����f�[�^
				lcdbm_QR_data_res( 1 );	// ����(0:OK,1:NG(�r��),2:NG(�������))
				break;

			// ����ʒm(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// ����ʒm		���캰��
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// ���Z���~�v��
					case LCDBM_OPCD_PAY_STP:
						// ����ʒm(���Z���~����(NG))���M	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						break;

					// ���Z�J�n�v��			0�Œ�
					case LCDBM_OPCD_PAY_STA:
// MH810100(S) K.Onodera 2020/02/18 #3883 �u�ʎԗ��̐��Z�v��������ƎԔԌ�����ʂɑJ�ڂ��Ă��܂�
//						// ����ʒm(���Z�J�n����(OK))���M
//						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );
//
//						// op_mod01(�Ԕԓ���) = ���ޏ��҂�
//						OPECTL.Ope_mod = 1;
//						ret = -1;
						// ����ʒm(���Z�J�n����(NG)
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
						break;

					// �ҋ@��ʒʒm
					case LCDBM_OPCD_CHG_IDLE_DISP_NOT:
// MH810100(S) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
//						ret = 10;
						if( !end_req ){
							// ���Z�ē��I���ʒm
							lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
							// �����҂��^�C�}�[�J�n
							Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
							end_req = 1;
						}
// MH810100(E) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
// MH810100(E) K.Onodera 2020/02/18 #3883 �u�ʎԗ��̐��Z�v��������ƎԔԌ�����ʂɑJ�ڂ��Ă��܂�
						break;

					// �N���ʒm
					case LCDBM_OPCD_STA_NOT:
						// �N��Ӱ�� = �ݒ豯��۰�ދN��
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(�N����)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// �N��Ӱ�� = �ʏ�N��
						else {
							// op_mod00(�ҋ@)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;

// MH810100(S) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
					// ���Z�ē��I������
					case LCDBM_OPCD_PAY_GUIDE_END_RES:
						ret = 10;	// �ҋ@�֖߂�
						break;
// MH810100(E) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
				}
				break;

			// ���Z�ē��I�������^�C���A�E�g
			case TIMEOUT_MNT_RESTART:
				// ���Z�ē��I���ʒm
				lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
				// �����҂��^�C�}�[�ĊJ
				Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(�N����)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// ��������������ԂƂ���
				ret = -1;
				break;

			// IC�ڼޯĒ�~�҂����
			case TIMEOUT_ICCREDIT_STOP:		// TIMERNO_ICCREDIT_STOP
				break;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

			default:
				break;
		}
	}

	Lagcan( OPETCBNO, 1 );											// ��ϰ1ؾ��(�ݷ���t�s���ԗp)
// MH810100(S) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)
	Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );						// ��ϰ1ؾ��(�ݷ���t�s���ԗp)
// MH810100(E) K.Onodera  2020/03/05 #3931 �Ԕԃ`�P�b�g���X(���O�^�C����0�~���Z���A�g�b�v�ɖ߂鉟���őҋ@�ɖ߂�Ȃ��s��C��)

	OPECTL.InLagTimeMode = OFF;										// ���O�^�C���������[�h�n�e�e

	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| ����ݽӰ�ޑI������                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MntMain( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	MntMain( void )
{
	uchar	pushcnt = 0;
	uchar	wpass[9];
	ushort	msg = 0;
	ushort	usMtSelEvent;
	char	f_Card = 0;												// 0:�s���W������ 1:�W�����ގ� -1:�W������NG
	uchar	menu_page = 0;											// �\�����t�@���N�V�����y�[�W
	uchar	key_chk;
	uchar	tenkey_f5=0;
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar	ucReq = 0;	// 0=OK/1=NG/2=OK(�ċN���s�v)
	ushort	mode = 0;
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)


	TimeAdjustCtrl(2);			// ���������␳��~����

	DP_CP[0] = DP_CP[1] = 0;

	OPE_red = 2;													// ذ�ް�����r�o

	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ذ�޺����(���Ԍ��ۗ���)
	}

// GG124100(S) R.Endo 2022/09/28 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
// // GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
// 	if(mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE){
// 		// parkingWeb�ڑ�����
// 		if(_is_ntnet_remote()) {
// 			// �[���Őݒ�p�����[�^�ύX
// 			rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
// 			// �[�����Őݒ肪�X�V���ꂽ���߁A�\�񂪓����Ă�����L�����Z�����s��
// 			remotedl_cancel_setting();
// 		}
// 		mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
// 	}
// // GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
// GG124100(E) R.Endo 2022/09/28 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]		
	memset( wpass, 0x00, sizeof( wpass ) );							// Password clear
	LcdBackLightCtrl( ON );								// back light ON
	if( DOWNLOADING() ){
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( remotedl_disp() == MOD_CHG ){
//			return;
//		}
// GG120600(S) // Phase9 #5040 �v���O�����_�E�����[�h���{��(FTP�ڑ�������M��)�ɐ��Z�@�̓d��OFF/ON����ƁA�ʐM���̉�ʂ��\������ă����e�i���X��ʂɑJ�ڂł��Ȃ��Ȃ�
		// task_status�ɂ����f
		remotedl_status_set(remotedl_status_get());
// GG120600(E) // Phase9 #5040 �v���O�����_�E�����[�h���{��(FTP�ڑ�������M��)�ɐ��Z�@�̓d��OFF/ON����ƁA�ʐM���̉�ʂ��\������ă����e�i���X��ʂɑJ�ڂł��Ȃ��Ȃ�
		mode = remotedl_disp();
		if( mode == MOD_CHG || mode == MOD_CUT ){
			return;
		}
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�����e�i���X)
	}
// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
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
		lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)ucReq );	// �ݒ�A�b�v���[�h�ʒm���M
		mnt_SetFtpFlag( FTP_REQ_NONE );								// FTP�X�V�t���O�i�X�V�I���j���Z�b�g
	}
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
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

	dsp_background_color(COLOR_WHITE);
	dispclr();														// Display All Clear

	if( OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4){										// ����ݽ����3(�Z�p������)?
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[33] );							// "�������e�i���X�I����          "

		usMtSelEvent = Menu_Slt( SLMENU, MNT_SEL_TBL, (char)MNT_SEL_MAX, (char)1 );
																	// "�P�D���[�U�[�����e�i���X      "
																	// "�Q�D�V�X�e�������e�i���X      "
		if( usMtSelEvent == USR_MNT ){
			OPECTL.Mnt_mod = 2;										// �W��,�Ǘ�������ݽӰ�ނֈڍs
		}else if( usMtSelEvent == SYS_MNT ){
			OPECTL.Mnt_mod = 3;										// �Z�p������ݽӰ�ނֈڍs
		}else if( usMtSelEvent == MOD_EXT ){						// F5 Key ( End Key )
			OPECTL.Mnt_lev = (char)-1;								// ����ݽ�����߽ܰ�ޖ���
			OPECTL.PasswordLevel = (char)-1;
		}else if( usMtSelEvent == MOD_CHG ){						// Mode change key
// MH810100(S) K.Onodera 2020/2/19 #3887 �����e�i���X�I����ʂɖ߂��Ă���h�A�ɂ���ƁA������ʂ֑J�ڂł��Ȃ��Ȃ�
//			OPECTL.Mnt_mod = 0;										// ���ڰ���Ӱ�ޏ�����
//			OPECTL.Mnt_lev = (char)-1;								// ����ݽ�����߽ܰ�ޖ���
//			OPECTL.PasswordLevel = (char)-1;
//		}
			op_wait_mnt_close();									// �����e�i���X�I������
		}
		else if( usMtSelEvent == MOD_CUT ){
			OPECTL.Ope_mod = 255;									// ��������Ԃ�
			OPECTL.init_sts = 0;									// ��������������ԂƂ���
			OPECTL.Pay_mod = 0;										// �ʏ퐸�Z
			OPECTL.Mnt_mod = 0;										// ���ڰ���Ӱ�ޏ�����
			OPECTL.Mnt_lev = (char)-1;								// ����ݽ�����߽ܰ�ޖ���
			OPECTL.PasswordLevel = (char)-1;
		}
// MH810100(E) K.Onodera 2020/2/19 #3887 �����e�i���X�I����ʂɖ߂��Ă���h�A�ɂ���ƁA������ʂ֑J�ڂł��Ȃ��Ȃ�
		Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
		return;
	}else{
		/* ����ݽ��ʂ���߂��Ă����ꍇ����ÏI������LOG�o�^ */
		if( 0 != OPECTL.Kakari_Num ){									// ����ݽ��ʂ���̖߂�
			wopelg( OPLOG_MNT_END, 0L, (ulong)OPECTL.Kakari_Num );		// �W������I������۸ޓo�^
			OPECTL.Kakari_Num = 0;										// �W�����ر
			OPECTL.PasswordLevel = (char)-1;
		}
		else{
			msg = 0xffff;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//			Key_Event_Get();
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
		}
		OPECTL.Mnt_lev = (char)-1;									// ����ݽ�����߽ܰ�ޖ���
		OPECTL.PasswordLevel = (char)-1;
		if( Err_onf ){
			grachr( 0, 8, 16, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[58]+8 );					// "       �|�G���[�������|       "
		}else if( Alm_onf ){
			grachr( 0, 6, 18, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[59]+6 );					// "     �|�A���[���������|       "
		}
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[144] );						// "  �p�X���[�h����͂��ĉ�����  "
		grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );							// "     �p�X���[�h  ��������     "
		if( prm_get( COM_PRM,S_TYP,63,1,1 ) ){						// �Ïؔԍ��o�^����?
			Fun_Dsp(FUNMSG[107]);
		}
		else{														// �Ïؔԍ��o�^�Ȃ�
			Fun_Dsp(FUNMSG[106]);
		}
		if( msg == 0xffff ){
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
			if (isEC_USE()) {
				Ope_EcEleUseDsp();
// MH810104 GG119201(S) ���Ԍ��}���҂��������e��ʂŒ��O������e���󎚂���Ȃ�
				// �����e�i���X��ʂ֑J�ڂ���ۂ�
				// �܂����O����f�[�^���������Ă��Ȃ���Ώ�������
				if( ac_flg.ec_recv_deemed_fg != 0 ){
					EcRecvDeemed_RegistPri();
				}
// MH810104 GG119201(E) ���Ԍ��}���҂��������e��ʂŒ��O������e���󎚂���Ȃ�
			}
			else {
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
			Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica���p��s�ɂ���	
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
			}
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( Edy_Rec.edy_status.BIT.INITIALIZE ){			// Edy�̏����ݒ肪�����ς݁H
//				Edy_StopAndLedOff();							// ���ތ��m��~��UI LED�����w�����M  
//			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			msg = 0;
		}
	}

	OPE_red = 2;													// ذ�ް�����r�o

	for( ;; ){
		msg = GetMessage();
		switch( msg ){
// MH810100(S) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i�ؒf���m�j
			case LCD_DISCONNECT:
				OPECTL.Ope_mod = 255;							// ��������Ԃ�
				OPECTL.init_sts = 0;							// ��������������ԂƂ���
				OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
				OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
				OPECTL.PasswordLevel = (char)-1;
				return;
				break;
// MH810100(E) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i�ؒf���m�j

			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 0 ){							// key OFF
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X�i�����e�i���X�I�������j
//					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
//					OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
//					OPECTL.PasswordLevel = (char)-1;
//					return;
					// �����e�i���X�I������
					mode = op_wait_mnt_close();
					if( mode == MOD_CHG || mode == MOD_CUT ){
						return;
					}
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X�i�����e�i���X�I�������j
				}
				break;

			case KEY_TEN0:											// �ݷ�[0] �����
			case KEY_TEN1:											// �ݷ�[1] �����
			case KEY_TEN2:											// �ݷ�[2] �����
			case KEY_TEN3:											// �ݷ�[3] �����
			case KEY_TEN4:											// �ݷ�[4] �����
			case KEY_TEN5:											// �ݷ�[5] �����
			case KEY_TEN6:											// �ݷ�[6] �����
			case KEY_TEN7:											// �ݷ�[7] �����
			case KEY_TEN8:											// �ݷ�[8] �����
			case KEY_TEN9:											// �ݷ�[9] �����
				BUZPI();
				if(f_Card) {
					// �W���J�[�h�}����A�p�X���[�h��ʂ̂܂܃e���L�[�����͂��ꂽ�ꍇ�̓N���A����
					OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
					OPECTL.PasswordLevel = (char)-1;
					f_Card = 0;
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );		// "     �p�X���[�h  ��������     "
				}
				if( OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4){										// ����ݽ����3(�Z�p������)?
					;
				}else{
					/*--------------------------------------------------------------*/
					/* ���͍�����\�����邪�A�����߽ܰ�ނ͉E�l�߂��ޯ̧�ݸނ���B	*/
					/* ����1�޲ļ�Ă��Ă���V�������͒l���i�[����B					*/
					/* �ݒ�Ɣ�r�����̂�wpass[4]�`wpass[7]��4�޲ĂƂȂ�B		*/
					/* wpass[0]�`wpass[7]��8�޲Ă�[C4011441]�p�B					*/
					/*--------------------------------------------------------------*/
					if( pushcnt < 4 ){
						grachr( 4, (ushort)(17+(pushcnt*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru );	// ���\��
						pushcnt++;									// �߽ܰ�ޕ\�����͌���(MAX4��)
					}
					memmove( &wpass[0], &wpass[1], 8 );				// ���͒l������1�޲Ĉړ�
					wpass[8] = (uchar)( msg - KEY_TEN );
				}
				break;

			case KEY_TEN_F1:										// �ݷ�[���Z] �����
				if( !prm_get( COM_PRM,S_TYP,62,1,2 ) ) {
					BUZPIPI();
					break;
				}
				if (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0){	// ���Z���O�������ݒ�
					BUZPIPI();
					break;
				}
				if( ope_imf_GetStatus() ){ 
					BUZPIPI();
					break;
				}
				if( prm_get(COM_PRM, S_TYP, 98, 1, 5) ){
					if( mtpass_get( pushcnt, wpass ) < 1 ){				// ����ݽ�̖���(����)�擾
						BUZPIPI();
						break;
					}
				}
				wopelg( OPLOG_SYUSEI_START, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
				BUZPI();
				OPECTL.Pay_mod = 2;									// �C�����Z����
				OPECTL.Mnt_mod = 0;									// ���ڰ���Ӱ�ޏ�����
				OPECTL.PasswordLevel = (char)-1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;							// ����ݽ�����߽ܰ�ޖ���
				OPECTL.PasswordLevel = (char)-1;
				return;

			case KEY_TEN_F4:										// �ݷ�[���] �����
				/*** �߽ܰ�ފm�菈�� ***/
				if( (OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4) && f_Card != 1){			// ����ݽ����3(�Z�p������)?
					;
				}else{
					if(f_Card != 1) {
						OPECTL.Mnt_lev = mtpass_get( pushcnt, wpass );	// ����ݽ�̖���(����)�擾
					}
					else {
						f_Card = 0;
					}

// MH810104(S) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�
					// �����v�Z�e�X�g�̒��Ӊ�ʂ�\������
					OPECTL.RT_show_attention = 1;
// MH810104(E) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�

					switch( OPECTL.Mnt_lev ){
						case 1:										// �W������
						case 2:										// �Ǘ��ґ���
							if( read_rotsw() != 0 ) {
								BUZPIPI();
								grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ۰�ؽ�����0�ɖ߂��ĉ�����   "
								OPECTL.Mnt_lev = (char)-1;			// ����ݽ�����߽ܰ�ޖ���
								OPECTL.PasswordLevel = (char)-1;
								break;
							}
							BUZPI();
							OPECTL.Mnt_mod = 2;						// �W��,�Ǘ�������ݽӰ�ނֈڍs
							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
							wopelg( OPLOG_MNT_START_PASS, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
							if(CPrmSS[S_PAY][17]) {							// �d�����b�N����H
								// �d�����b�N�������x���ȏ�̌W���J�[�h�}�����̓��b�N����
								if(OPECTL.PasswordLevel != (char)-1 && OPECTL.PasswordLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
									if(SD_EXIOPORT & 0x8000) {
									OPE_SIG_OUT_DOORNOBU_LOCK(0);			// �ޱ��ނ�ۯ�����
									wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ����o�^(�A�����b�N)
									if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// �ޱ�J��� ��ۯ�������ϰ�l���L���͈͓�
										DoorLockTimer = 1;
										Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// �ޱ�J��� ��ۯ�������ϰ����
									}
									}
								}
							}
							return;

						case 3:										// �Z�p������
						case 4:										// �J���ґ���
							if( read_rotsw() != 0 ) {
								BUZPIPI();
								grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ۰�ؽ�����0�ɖ߂��ĉ�����   "
								OPECTL.Mnt_lev = (char)-1;			// ����ݽ�����߽ܰ�ޖ���
								OPECTL.PasswordLevel = (char)-1;
								break;
							}
							BUZPI();
							wopelg( OPLOG_MNT_START_PASS, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
							if(CPrmSS[S_PAY][17]) {							// �d�����b�N����H
								// �d�����b�N�������x���ȏ�̌W���J�[�h�}�����̓��b�N����
								if(OPECTL.PasswordLevel != (char)-1 && OPECTL.PasswordLevel >=  (char)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
									if(SD_EXIOPORT & 0x8000) {
									OPE_SIG_OUT_DOORNOBU_LOCK(0);			// �ޱ��ނ�ۯ�����
									wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ����o�^(�A�����b�N)
									if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// �ޱ�J��� ��ۯ�������ϰ�l���L���͈͓�
										DoorLockTimer = 1;
										Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// �ޱ�J��� ��ۯ�������ϰ����
									}
									}
								}
							}
							dispclr();								// Display All Clear
							grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[33] );		// "�������e�i���X�I����          "

							usMtSelEvent = Menu_Slt( SLMENU, MNT_SEL_TBL, (char)MNT_SEL_MAX, (char)1 );
																	// "�P�D���[�U�[�����e�i���X      "
																	// "�Q�D�V�X�e�������e�i���X      "
							if( usMtSelEvent == USR_MNT ){
								OPECTL.Mnt_mod = 2;					// �W��,�Ǘ�������ݽӰ�ނֈڍs
							}else if( usMtSelEvent == SYS_MNT ){
								OPECTL.Mnt_mod = 3;					// �Z�p������ݽӰ�ނֈڍs
							}else if( usMtSelEvent == MOD_EXT ){	// F5 Key ( End Key )
								OPECTL.Mnt_lev = (char)-1;			// ����ݽ�����߽ܰ�ޖ���
								OPECTL.PasswordLevel = (char)-1;
							}else if( usMtSelEvent == MOD_CHG ){	// Mode change key
// MH810100(S) K.Onodera 2020/01/17 �Ԕԃ`�P�b�g���X�i�����e�i���X�I�������j
//								OPECTL.Mnt_mod = 0;					// ���ڰ���Ӱ�ޏ�����
//								OPECTL.Mnt_lev = (char)-1;			// ����ݽ�����߽ܰ�ޖ���
//								OPECTL.PasswordLevel = (char)-1;
//							}
								op_wait_mnt_close();				// �����e�i���X�I������
							}
							else if( usMtSelEvent == MOD_CUT ){
								OPECTL.Ope_mod = 255;				// ��������Ԃ�
								OPECTL.init_sts = 0;				// ��������������ԂƂ���
								OPECTL.Pay_mod = 0;					// �ʏ퐸�Z
								OPECTL.Mnt_mod = 0;					// ���ڰ���Ӱ�ޏ�����
								OPECTL.Mnt_lev = (char)-1;			// ����ݽ�����߽ܰ�ޖ���
								OPECTL.PasswordLevel = (char)-1;
							}
// MH810100(E) K.Onodera 2020/01/17 �Ԕԃ`�P�b�g���X�i�����e�i���X�I�������j
							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
							return;

						default:
							BUZPIPI();
							OPECTL.Mnt_lev = (char)-1;				// ����ݽ�����߽ܰ�ޖ���
							OPECTL.PasswordLevel = (char)-1;
							OPECTL.Kakari_Num = 0;					// �W�����ر
							pushcnt = 0;							// �߽ܰ�ޕ\�����͌����ر
							memset( wpass, 0x00, sizeof( wpass ) );	// Password clear
							grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );		// "     �p�X���[�h  ��������     "
							break;
					}
				}
				break;

			case KEY_TEN_F2:										// �ݷ�[�o�^] �����
				if( tenkey_f5 == 1 && menu_page == 1 ){					// �����J�̃e���L�[�@�\�͂Q�ʖڌŒ�
					if(( OPECTL.Mnt_lev = mtpass_get( pushcnt, wpass )) >= 1){	// ����ݽ�̖���(����)�擾(�W���ȏ�)
						if( read_rotsw() != 0 ) {
							BUZPIPI();
							grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ۰�ؽ�����0�ɖ߂��ĉ�����   "
							break;
						}
						OPECTL.Mnt_mod = 2;								// �W��,�Ǘ�������ݽӰ�ނֈڍs
						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
						wopelg( OPLOG_MNT_START_PASS, 0L, (ulong)OPECTL.Kakari_Num );	// �߽ܰ�ތW������J�n����۸ޓo�^
						
						if( Cardress_DoorOpen() == MOD_EXT ){
							if( OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4 ){							// ����ݽ����3(�Z�p������)?
								if( read_rotsw() != 0 ) {
									BUZPIPI();
									grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ۰�ؽ�����0�ɖ߂��ĉ�����   "
									break;
								}
								BUZPI();
								OPECTL.Mnt_mod = 1;						// �����e�i���X�I����ʂ�
							}else{
								if( read_rotsw() != 0 ) {
									BUZPIPI();
									grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ۰�ؽ�����0�ɖ߂��ĉ�����   "
									break;
								}
								BUZPI();
								OPECTL.Mnt_mod = 2;						// �߽ܰ�މ�ʂֈڍs
							}
						}else{
							/* MOD_CHG */
							if( OPECTL.on_off == 0 ){					// key OFF
								BUZPI();
								OPECTL.Pay_mod = 0;						// �ʏ퐸�Z
								OPECTL.Mnt_mod = 0;						// ���ڰ���Ӱ�ޏ�����
								OPECTL.Mnt_lev = (char)-1;				// ����ݽ�����߽ܰ�ޖ���
								OPECTL.PasswordLevel = (char)-1;
							}else{										// key ON
								if( read_rotsw() != 0 ) {
									BUZPIPI();
									grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ۰�ؽ�����0�ɖ߂��ĉ�����   "
									break;
								}
								BUZPI();
								OPECTL.Pay_mod = 0;						// �ʏ퐸�Z
								OPECTL.Mnt_mod = 1;						// ����ݽӰ�ޑI��������
								OPECTL.Mnt_lev = (char)-1;				// ����ݽ�����߽ܰ�ޖ���
								OPECTL.PasswordLevel = (char)-1;
							}
						}
						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
						return;
					}else{
						BUZPIPI();
						OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
						OPECTL.PasswordLevel = (char)-1;
						OPECTL.Kakari_Num = 0;							// �W�����ر
						pushcnt = 0;									// �߽ܰ�ޕ\�����͌����ر
						memset( wpass, 0x00, sizeof( wpass ) );			// Password clear
						grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );				// "     �p�X���[�h  ��������     "
					}
					break;
				}
				if( prm_get( COM_PRM,S_TYP,63,1,1 ) ){				// �Ïؔԍ��o�^����?
					BUZPI();
					OPECTL.Pay_mod = 1;								// �߽ܰ�ޖY�ꏈ��
					OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
					OPECTL.PasswordLevel = (char)-1;
					return;
				}else{
					BUZPIPI();
				}
				break;

			case KEY_TEN_CL:										// �ݷ�CL �����
					BUZPI();
					pushcnt = 0;									// �߽ܰ�ޕ\�����͌����ر
					memset( wpass, 0x00, sizeof( wpass ) );			// Password clear
					wpass[8] = 'C';
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );				// "     �p�X���[�h  ��������     "
					grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );	// 	<--- Clear
					OPECTL.Mnt_lev = (char)-1;									// ����ݽ�����߽ܰ�ޖ���
					OPECTL.PasswordLevel = (char)-1;
					break;
			case KEY_TEN_F3:										// �ݷ�F3 �����
// �e�X�g�p���O�쐬(S)
#ifdef TEST_LOGFULL
				BUZPI();
				grachr( 6, 10, 10, 0, COLOR_RED, LCD_BLINK_ON, "���O�o�͒�" );
				taskchg(IDLETSKNO);
				Test_Ope_LogFull();
				grachr( 6, 10, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, "�@�@�@�@�@" );
#endif	// TEST_LOGFULL
// �e�X�g�p���O�쐬(E)
// MH810100(S) �e�X�g�p���Ƀ��O�쐬
#ifdef TEST_ENTER_LOG
			BUZPI();
			Test_Ope_EnterLog();
#endif
// MH810100(E) �e�X�g�p���Ƀ��O�쐬
			case KEY_TEN_F5:										// �ݷ�F5 �����
				BUZPIPI();
				break;
			case MID_STSCHG_ANY_ERROR:								// �װ �L���ɕω�����
			case MID_STSCHG_ANY_ALARM:								// �װїL���ɕω�����
				if( Err_onf ){
					displclr( 0 );									// Line Clear
					grachr( 0, 8, 16, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[58]+8 );			// "       �|�G���[�������|       "
				}else if( Alm_onf ){
					displclr( 0 );									// Line Clear
					grachr( 0, 6, 18, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[59]+6 );			// "     �|�A���[���������|       "
				}else{
					displclr( 0 );									// Line Clear
				}
				break;
			case ARC_CR_R_EVT:										// ����IN
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				f_Card = Kakariin_card();
				if( 1 == f_Card ) {

					BUZPI();
					// ���x���N���A
					OPECTL.Mnt_lev = (char)-1;
					OPECTL.PasswordLevel = (char)-1;

					f_Card = 1;
					grachr( 4, (ushort)17, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ���\��
					grachr( 4, (ushort)19, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ���\��
					grachr( 4, (ushort)21, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ���\��
					grachr( 4, (ushort)23, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ���\��
					Ope_KakariCardInfoGet((uchar*)&OPECTL.Mnt_lev, (uchar*)&OPECTL.PasswordLevel, &OPECTL.Kakari_Num);
					if(CPrmSS[S_PAY][17]) {							// �d�����b�N����H
						// �d�����b�N�������x���ȏ�̌W���J�[�h�}�����̓��b�N����
						if(OPECTL.PasswordLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
							OPE_SIG_OUT_DOORNOBU_LOCK(0);			// �ޱ��ނ�ۯ�����
							wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ����o�^(�A�����b�N)
							if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){	// �ޱ�J��� ��ۯ�������ϰ�ݒ肠��
								DoorLockTimer = 1;
								Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// �ޱ�J��� ��ۯ�������ϰ����
							}
							grachr(6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, sercret_Str[3]);				// " <<�h�A���b�N���������܂���>> "
						}
					}
				}
				else if( 0 == f_Card ) {							// �s���J�[�h�i���o�^�j
					grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[3] );	// ���̃J�[�h�͖����ł�
				}
				else {												// �J�[�hNG
					grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[1] );	// ���̃J�[�h�͎g���܂���
				}
				break;
			case TIMEOUT4:
			case ARC_CR_EOT_EVT:
				displclr(6);
				if( msg == ARC_CR_EOT_EVT && RD_mod < 9 ){
					rd_shutter();									// 500msec��ڲ��ɼ��������
				}
				break;
			case TIMEOUT6:											// ��ϰ6��ѱ�āF�u�o�^�v�����u����v������������ԊĎ���ϰ

				if( READ_SHT_flg == 1 ){							// �������ԁH
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// ���C���[�_�Ȃ��̏ꍇ�̓V���b�^�[�J���Ȃ�
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ��ϰ7�i������J��Ԍp���Ď���ϰ�j�N���i�R�O�b�j
					read_sht_opn();									// �̏ꍇ�F�J����
				}
				else{
					read_sht_cls();									// �J�̏ꍇ�F����
				}
				break;
			case TIMEOUT7:											// ��ϰ7��ѱ�āF������J��Ԍp���Ď���ϰ

				rd_shutter();										// ���Cذ�ް���������
				break;
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
			case EC_RECEPT_SEND_TIMEOUT:							// ec�p��ϰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,0 );
				break;
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
			default:
				break;
		}
		key_chk = op_key_chek();									// �u�o�^�v�L�[���u����v�L�[�̓���������ԊĎ�
		switch( key_chk ){
			case	1:												// ����������ԊJ�n
				Lagtim( OPETCBNO, 6, 3*50 );						// ��ϰ6�i���������Ď���ϰ�j�N���i�R�b�j
				break;
			case	2:												// ����������ԉ���
				Lagcan( OPETCBNO, 6 );								// ��ϰ6�i���������Ď���ϰ�jؾ��
				break;
		}
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z���������p���ɏ�Ծ�ď���                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_indata_set( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*|                                                                        |*/
/*|  �ڑ��ݒ肳��Ă���ۯ����u�̒��Ԉʒu����ɏ�ԂƂ���                   |*/
/*|  �i���ɓ����́A���ݔN�����̂T�F�O�O�Ƃ���j                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-10-04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	op_indata_set( void )
{
	ushort	i;
	uchar	work;
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	ushort	normalize;
	short	date[3];
// MH321800(E) hosoda IC�N���W�b�g�Ή�

#if (4 == AUTO_PAYMENT_PROGRAM)
	for( i=0 ; i < 24 ; i++ ){

			FLAPDT.flp_data[i].mode = 3;						// ���ɏ�Ծ��
			FLAPDT.flp_data[i].year = (ushort)CLK_REC.year;		// ���݁i�N�j���
			FLAPDT.flp_data[i].mont = (uchar)CLK_REC.mont;		// ���݁i���j���
			FLAPDT.flp_data[i].date = (uchar)CLK_REC.date;		// ���݁i���j���
			FLAPDT.flp_data[i].hour = 5;						// �O�T��
			FLAPDT.flp_data[i].minu = 0;						// �O�O��
			memcpy( &LockInfo[i+1],&LockInfo[i],sizeof( LockInfo[0] ));
			LockInfo[i+1].posi = (LockInfo[i].posi+1);
			LockInfo[i+1].lok_no = (uchar)(LockInfo[i].lok_no+1);
			IoLog_write(IOLOG_EVNT_ENT_GARAGE, i, 0, (ulong)FLAPDT.flp_data[i].hour);	/* MH702200-T00 Y.Ise ���O���L�^�f�o�b�O�p */
	}

#else
	work = GetCarInfoParam();
	for( i=0 ; i < LOCK_MAX ; i++ ){
		WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( i < INT_CAR_START_INDEX ) {
			if( !( work & 0x02 ) ) {
				continue;
			}
		} else if( i < BIKE_START_INDEX ) {
			if( !( work & 0x04 ) ) {
				continue;
			}
		} else {
			if( !( work & 0x01 ) ) {
				continue;
			}
		}

		if( LockInfo[i].lok_syu != 0 ){
			// �ڑ��ݒ肠��
			FLAPDT.flp_data[i].mode = 3;						// ���ɏ�Ծ��
			if (_is_pip()) {
				if ((i+1) == ryo_buf.pkiti) {
					FLAPDT.flp_data[i].mode = 5;					// ���ɏ�Ծ��
				}
			}
			FLAPDT.flp_data[i].year = (ushort)CLK_REC.year;		// ���݁i�N�j���
			FLAPDT.flp_data[i].mont = (uchar)CLK_REC.mont;		// ���݁i���j���
			FLAPDT.flp_data[i].date = (uchar)CLK_REC.date;		// ���݁i���j���
			if (_is_pip()) {
				if (i % 2) {
					FLAPDT.flp_data[i].hour = 9;					// 09��
				} else {
					FLAPDT.flp_data[i].hour = 5;					// �O�T��
				}
			} else {
				FLAPDT.flp_data[i].hour = 5;						// �O�T��
			}
// MH321800(S) hosoda IC�N���W�b�g�Ή�
			if (CLK_REC.hour <= FLAPDT.flp_data[i].hour) {
			// ���ݎ��� <= �T���i�X���j�����ɂ���O���ɂ���
				memset(date, 0, sizeof(date));
				date[0] = FLAPDT.flp_data[i].year;
				date[1] = FLAPDT.flp_data[i].mont;
				date[2] = FLAPDT.flp_data[i].date;
				normalize = dnrmlzm(date[0], date[1], date[2]);
				normalize--;
				idnrmlzm(normalize, &date[0], &date[1], &date[2]);
				FLAPDT.flp_data[i].year = (ushort)date[0];
				FLAPDT.flp_data[i].mont = (uchar)date[1];
				FLAPDT.flp_data[i].date = (uchar)date[2];
			}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
			FLAPDT.flp_data[i].minu = 0;						// �O�O��
			IoLog_write(IOLOG_EVNT_ENT_GARAGE, i, 0, (ulong)FLAPDT.flp_data[i].hour);	/* MH702200-T00 Y.Ise ���O���L�^�f�o�b�O�p */
		}
	}
#endif
}

/*[]----------------------------------------------------------------------[]*/
/*| �o�^���A������̉��������������                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_key_chek( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : uchar ret = �����                                      |*/
/*|                          0:�����i����ԕω��Ȃ��j                      |*/
/*|                          1:�o�^���A��������������J�n                  |*/
/*|                          2:�o�^���A�����������������                  |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2005-09-13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	op_key_chek( void )
{
	uchar	ret = 0;	// �߂�l

	if( PPrmSS[S_P01][1] == 0 ){

		// �V���b�^�[�J�����[�h�łȂ��ꍇ

		if( (OPECTL.Comd_knd == 0x01) && (OPECTL.Comd_cod == (0x00FF & KEY_TEN_F5)) ){
			// �u�o�^�L�[�v�C�x���g
			if( OPECTL.on_off == 0 ){
				// �n�e�e
				OPECTL.Ent_Key_Sts = OFF;				// �o�^����ԁF�n�e�e
				if( OPECTL.EntCan_Sts == ON ){			// ����������Ԃ������H
					OPECTL.EntCan_Sts = OFF;			// ����������ԁF�n�e�e
					ret = 2;							// ����������ԉ���
				}
			}
			else{
				// �n�m
				OPECTL.Ent_Key_Sts = ON;				// �o�^����ԁF�n�m
				if( OPECTL.Can_Key_Sts == ON ){			// �������Ԃn�m�H
					OPECTL.EntCan_Sts = ON;				// ����������ԁF�n�m
					ret = 1;							// ����������ԊJ�n
				}
			}
		}
		else if( (OPECTL.Comd_knd == 0x01) && (OPECTL.Comd_cod == (0x00FF & KEY_TEN_F4)) ){
			// �u����L�[�v�C�x���g
			if( OPECTL.on_off == 0 ){
				// �n�e�e
				OPECTL.Can_Key_Sts = OFF;				// �������ԁF�n�e�e
				if( OPECTL.EntCan_Sts == ON ){			// ����������Ԃ������H
					OPECTL.EntCan_Sts = OFF;			// ����������ԁF�n�e�e
					ret = 2;							// ����������ԉ���
				}
			}
			else{
				// �n�m
				OPECTL.Can_Key_Sts = ON;				// �������ԁF�n�m
				if( OPECTL.Ent_Key_Sts == ON ){			// �o�^����Ԃn�m�H
					OPECTL.EntCan_Sts = ON;				// ����������ԁF�n�m
					ret = 1;							// ����������ԊJ�n
				}
			}
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*|  �p�����[�^�ɂ���ėa��؁E�̎����̈󎚂��s��						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Print_Condition_select( void )		                   |*/
/*| PARAMETER    : NONE													   |*/
/*| RETURN VALUE : NONE													   |*/
/*| �����l��                                                               |*/
/*|    �������Ұ�01-0044�ɐݒ肳��Ă���l���Q�Ƃ��A�󎚏�����ύX����     |*/
/*|    0�F�a��؂��������s								                   |*/
/*|    1�F�̎����ɕ��o�s���z���󎚂��Ď������s					           |*/
/*|    2�F�������s���Ȃ�								                   |*/
/*|    ���F����̑Ή��ł͗̎����{�^�����������Ă���ꍇ�́AOPECTL.RECI_SW  |*/
/*|    ���P�ɐݒ肳��Ă���F���ł���A�ݒ���@���ς�����ꍇ�͏C���̕K�v��|*/
/*|    ����܂��B										                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.NAMIOKA                                               |*/
/*| Date         : 2006-02-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void Print_Condition_select( void )
{
	switch(CPrmSS[S_SYS][44]){							// 0:�����I�ɗa��؂̔��s 1:�����I�ɗ̎����ɗa����e���󎚂��Ĕ��s 2:�������s���Ȃ�
		case 0:
			chu_isu();									// �a�菑���s
			if(OPECTL.RECI_SW == 1)
				ryo_isu( 1 );							// �̎���(�a����e����)���s
			break;
		case 1:
// MH810100(S) K.Onodera 2020/03/31 �Ԕԃ`�P�b�g���X(#3941 �̎��؎������s��A�{�^���������ł��Ă��܂�)
			PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );	// ����ʒm���M
// MH810100(E) K.Onodera 2020/03/31 �Ԕԃ`�P�b�g���X(#3941 �̎��؎������s��A�{�^���������ł��Ă��܂�)
			ryo_isu( 1 );								// �̎���(�a����e����)���s
// MH810100(S) 2020/05/27 �Ԕԃ`�P�b�g���X(#4181)
			if(OPECTL.op_faz == 3){											// ����{�^�����������ꂽ�ꍇ
				azukari_popup(1);
			}else{
				azukari_popup(0);
			}
// MH810100(E) 2020/05/27 �Ԕԃ`�P�b�g���X(#4181)
			OPECTL.RECI_SW = 1;							// �̎������ݎg�p�s���
			break;
		case 2:
			if(OPECTL.RECI_SW == 1)
				ryo_isu( 1 );							// �̎���(�a����e����)���s
			break;
		default:
			break;
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ���q�l���ލ쐬                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MakeVisitorCode(void)                                   |*/
/*| PARAMETER    : code ���q�l���ފi�[�ر�ւ��߲��                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Nishizato                                             |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void MakeVisitorCode(uchar *code)
{
	ushort wk, month, denom;
	short i;

	static unsigned int seed = 0;
	if (seed++ >= 0xefff) seed = 1;		// seed == 0 ���Ɨ�������������Ȃ�
	srand((unsigned int)CLK_REC.minu + seed);	//���ݕ���Seed�Ƃ��ė�������
	wk = (ushort)rand();
	wk %= 10000;

	//�������
	denom = 1000;
	for (i = 3; i >= 0; i--) {
		*code = (uchar)((wk / denom) + 0x30);
		wk %= denom;
		code++;
		denom /= 10;
	}

	//�����
	if ((CPrmSS[S_PAY][41] % 10) == 1) {
		// �Ïؔԍ�(Btype)�����\�J�n���̍X�V�͍s��Ȃ�
		wk = 1;
	} else {
		// �J�n���v�Z�ɓ����̉�1�����g�p����
		month = (ushort)CLK_REC.mont;
		wk = month % 10;
	}
	*code = (uchar)(wk + 0x30);
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ïؔԍ�(Btype)����                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CheckVisitorCode(void)                                  |*/
/*| PARAMETER    : code ���q�l���ފi�[�ر�ւ��߲��                         |*/
/*|                val  ���͈Ïؔԍ�                                       |*/
/*| RETURN VALUE : 0:�Ïؔԍ��s��v / 1:�Ïؔԍ���v                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Nishizato                                             |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort CheckVisitorCode(uchar *code, ushort val)
{
	ulong pass1, pass2, denom;
	ushort wk, kt;
	short i, j;
	ulong pass[10];

	//�����\�擾
	pass1 = (ulong)CPrmSS[S_PAY][42];
	pass2 = (ulong)CPrmSS[S_PAY][43];
	denom = 100000;
	for (i = 0; i < 10; i ++) {
		if (i < 6) {
			pass[i] = pass1 / denom;
			if (i == 5) {
				denom = 100000*10;
			}
			pass1 %= denom;
		} else {
			pass[i] = pass2 / denom;
			pass2 %= denom;
		}
		denom /= 10;
	}

	//�J�n���̎擾
	kt = code[4] - 0x30;
	if (kt == 0) kt = 10;

	//�l�擾
	wk = 0;
	denom = 1000;
	for (i = 3; i >= 0; i--) {
		j = (kt-1)+(code[3-i]-0x30);
		if (j == 0) j = 10;	// 0(����)�Ȃ�10���ڂ��Q�Ƃ���
		if (j > 10) j -= 10;
		wk += (ushort)(pass[j-1] * denom);
		denom /= 10;
	}
	//�l��v����
	if (wk == val) {
		return 1;
	} else {
		return 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�̈�ւ��ް��̊i�[		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ElectronSet_PayData						               |*/
/*| PARAMETER    : ppc_tiket *buf : �ް��i�[�o�b�t�@                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2007-02-26                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	ElectronSet_PayData( void *buf, uchar data_kind )
{

	#if (3 == AUTO_PAYMENT_PROGRAM)									// �����p�ɃJ�[�h�c�z�����܂����ꍇ�iýėp�j
		ulong	wkul;
	#endif

	if( SuicaUseKindCheck( data_kind ) ){									// ���Z��suica�ōs��ꂽ�ꍇ
		PayData.Electron_data.Suica.e_pay_kind = data_kind;	// ���Z��ʂ��
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.e_pay_kind), NTNET_152_ECARDKIND);
		memcpy( &PayData.Electron_data.Suica.pay_ryo,&((SUICA_SETTLEMENT_RES*)buf)->settlement_data, 28); // ���ϋ��z�E�c���E����ID���
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.pay_ryo), NTNET_152_EPAYRYO);
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.Card_ID[0]), NTNET_152_ECARDID);
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.pay_after), NTNET_152_EPAYAFTER);

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	}else if( data_kind == EDY_USED ){						// ���Z��Edy�ōs��ꂽ�ꍇ
//		memset( &PayData.Electron_data.Edy, 0, sizeof( PayData.Electron_data.Edy ));
//		PayData.Electron_data.Edy.e_pay_kind = data_kind;	// ���Z��ʂ��
//		// Edy���̾�Ă��ȉ��ōs��
//		PayData.Electron_data.Edy.pay_ryo		=	PayChengeToBIN(((R_SUBTRACTION_DATA*)buf)->Pay_Ryo);    	// ���ϋ��z���
//		PayData.Electron_data.Edy.pay_befor		=	PayChengeToBIN(((R_SUBTRACTION_DATA*)buf)->Pay_Befor);		// ���Z�O�c�z���
//		PayData.Electron_data.Edy.pay_after		=	PayChengeToBIN(((R_SUBTRACTION_DATA*)buf)->Pay_After);		// ���Z��c�z���
//
//		/** ýėp ���ގc�z���܂������� **/
//		#if (3 == AUTO_PAYMENT_PROGRAM)									// �����p�ɃJ�[�h�c�z�����܂����ꍇ�iýėp�j
//			if( 0L != (unsigned long)CPrmSS[S_SYS][5] ){				// �c�z���܂����ݒ肠���
//				wkul = CPrmSS[S_SYS][5];								// ���ޓ��c�z�ύX
//				if( PayData.Electron_data.Edy.pay_befor > wkul ) {		// �����ގc�z > ���܂����z�i�����ގc�z���傫�Ȓl�ɂ͂��Ȃ��j
//					PayData.Electron_data.Edy.pay_befor = wkul;			// ���Z�O���ގc�z���܂���
//					PayData.Electron_data.Edy.pay_after = PayData.Electron_data.Edy.pay_befor - PayData.Electron_data.Edy.pay_ryo;
//																		// ���Z�㶰�ގc�z���܂���
//				}
//			}
//		#endif
//
//		if( prm_get(COM_PRM, S_SCA, 55, 1, 1) != 1 ){															// ýėp���ގg�p�ݒ莞
//			PayData.Electron_data.Edy.deal_no		=	((R_SUBTRACTION_DATA*)buf)->Edy_Deal_num;				// Edy����ʔԂ��
//			PayData.Electron_data.Edy.card_deal_no	=	((R_SUBTRACTION_DATA*)buf)->Card_Deal_num;				// ���ގ���ʔԂ��
//			memcpy( PayData.Electron_data.Edy.Terminal_ID, &((R_SUBTRACTION_DATA*)buf)->High_Terminal_ID,4);
//			memcpy( PayData.Electron_data.Edy.Card_ID, ((R_SUBTRACTION_DATA*)buf)->Edy_No, sizeof( PayData.Electron_data.Edy.Card_ID ));
//		}
//#endif
//	}else if( data_kind == 99 ){							// ���Z��Edyý�Ӱ�ނōs��ꂽ�ꍇ
//		memset( &PayData.Electron_data.Edy, 0, sizeof( PayData.Electron_data.Edy ));
//		PayData.Electron_data.Edy.e_pay_kind = EDY_USED;													// ���Z��ʂ��
//		PayData.Electron_data.Edy.pay_befor		=	Edy_Settlement_Res.Pay_Befor;							// ���Z�O�c�z���
//		PayData.Electron_data.Edy.pay_ryo		=	Edy_Settlement_Res.Pay_Ryo;								// ���ϋ��z���
//		PayData.Electron_data.Edy.pay_after		=	Edy_Settlement_Res.Pay_Befor;							// ���Z�O�c�z���
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	}else{
		;													// �߽ȯđΉ��\�莞��ۼޯ����쐬����
	}

}

// MH321800(S) hosoda IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
const unsigned char EcBrandEmoney_Digit[] = {
	// �d�q�}�l�[�̃u�����h�̃J�[�h�ԍ�����
	// EC_KOUTSUU_USED���̏��Ԃɍ��킹��
	0,		// EC_UNKNOWN_USED [11]�s���u�����h
	0,		// EC_ZERO_USED [12]�u�����h�ԍ�0
	16,		// EC_EDY_USED [13]Edy
	16,		// EC_NANACO_USED [14]nanaco
// MHUT40XX(S) Edy�EWAON�Ή�
//	17,		// EC_WAON_USED [15]WAON
	16,		// EC_WAON_USED [15]WAON
// MHUT40XX(E) Edy�EWAON�Ή�
	0,		// EC_SAPICA_USED [16]SAPICA
	17,		// EC_KOUTSUU_USED [17]��ʌnIC�J�[�h
	16,		// EC_ID_USED [18]iD
	20,		// EC_QUIC_PAY_USED [19]QuicPay
	0,		// EC_CREDIT_USED [20]�N���W�b�g
	0,		// EC_HOUJIN_USED [21]�@�l
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	0,		// EC_HOUJIN_USED [22]�@�l
	0,		// EC_HOUSE_USED [23]�n�E�X�J�[�h
	0,		// EC_QR_USED [24]QR�R�[�h����
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
	17,		// EC_PITAPA_USED [25]PiTaPa
// MH810105(E) MH364301 PiTaPa�Ή�
};
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
/*[]----------------------------------------------------------------------[]*/
/*| ���Z�̈�ւ��ް��̊i�[ (���σ��[�_�ł̓d�q�}�l�[����)                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcElectronSet_PayData					               |*/
/*| PARAMETER    : *buf		: ����ް�				                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : hosoda                                                  |*/
/*| Date         : 2019-02-06                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	EcElectronSet_PayData( void *buf )
{
	EC_SETTLEMENT_RES	*p;
// MH810103 GG119202(S) �����c���Ɖ�����̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
	ushort	i, max;
// MH810103 GG119202(E) �����c���Ɖ�����̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//	uchar	dummy_Card_ID[20];
//	uchar	dummy_inquiry_num[16];
//// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	p = (EC_SETTLEMENT_RES*)buf;
	if(p->Column_No == 0xFF) {
		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;									// �݂Ȃ�����
	}
	PayData.Electron_data.Ec.pay_ryo	= p->settlement_data;											// ���ϋ��z
	PayData.Electron_data.Ec.pay_befor	= p->settlement_data_before;									// ���ϑO�c��
	PayData.Electron_data.Ec.pay_after	= p->settlement_data_after;										// ���ό�c��
	memcpy(PayData.Electron_data.Ec.Card_ID, p->Card_ID, sizeof(PayData.Electron_data.Ec.Card_ID));		// ����ID
	memcpy(PayData.Electron_data.Ec.inquiry_num, p->inquiry_num, sizeof(PayData.Electron_data.Ec.inquiry_num));// �⍇���ԍ�
	PayData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);						// ���ϓ���(Normalize)
// MH810105 GG119202(S) ������������W�v�d�l���P
	PayData.EcResult = p->Result;																		// ���ό���
// MH810105 GG119202(E) ������������W�v�d�l���P
// MH810105(S) MH364301 �s�v�ϐ��폜
//// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
//	memcpy(PayData.Electron_data.Ec.Termserial_No, p->Termserial_No, sizeof(PayData.Electron_data.Ec.Termserial_No));// �[�����ʔԍ�
//// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
// MH810105(E) MH364301 �s�v�ϐ��폜

	switch (p->brand_no) {
	case	BRANDNO_KOUTSUU:
		PayData.Electron_data.Ec.e_pay_kind					= EC_KOUTSUU_USED;							// ���Z���
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		memcpy(PayData.Electron_data.Ec.Brand.Koutsuu.SPRW_ID, p->Brand.Koutsuu.SPRW_ID,
				sizeof(PayData.Electron_data.Ec.Brand.Koutsuu.SPRW_ID));								// SPRWID
		memcpy(PayData.Electron_data.Ec.Brand.Koutsuu.Kamei, p->Brand.Koutsuu.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Koutsuu.Kamei));									// �����X��
		PayData.Electron_data.Ec.Brand.Koutsuu.TradeKind = p->Brand.Koutsuu.TradeKind;					// ������
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		break;
	case	BRANDNO_EDY:
		PayData.Electron_data.Ec.e_pay_kind					= EC_EDY_USED;								// ���Z���
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//		PayData.Electron_data.Ec.Brand.Edy.DealNo			= p->Brand.Edy.DealNo;						// ����ʔ�
//		PayData.Electron_data.Ec.Brand.Edy.CardDealNo		= p->Brand.Edy.CardDealNo;					// �J�[�h����ʔ�
		memcpy(PayData.Electron_data.Ec.Brand.Edy.DealNo, &p->Brand.Edy.DealNo, 10);					// Edy����ʔ�
		memcpy(PayData.Electron_data.Ec.Brand.Edy.CardDealNo, &p->Brand.Edy.CardDealNo, 5);				// �J�[�h����ʔ�
		memcpy(PayData.Electron_data.Ec.Brand.Edy.Kamei, p->Brand.Edy.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Edy.Kamei));										// �����X��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
		memcpy(PayData.Electron_data.Ec.Brand.Edy.TerminalNo, &p->Brand.Edy.TerminalNo, 8);				// �[���ԍ�
		break;
	case	BRANDNO_NANACO:
		PayData.Electron_data.Ec.e_pay_kind					= EC_NANACO_USED;							// ���Z���
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//		PayData.Electron_data.Ec.Brand.Nanaco.DealNo		= p->Brand.Nanaco.DealNo;					// ��ʎ���ʔ�
//		memcpy(PayData.Electron_data.Ec.Brand.Nanaco.TerminalNo, &p->Brand.Nanaco.TerminalNo, 12);		// ��ʒ[��ID
		memcpy(PayData.Electron_data.Ec.Brand.Nanaco.Kamei, p->Brand.Nanaco.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Nanaco.Kamei));									// �����X��
		memcpy(PayData.Electron_data.Ec.Brand.Nanaco.DealNo, &p->Brand.Nanaco.DealNo, 6);				// �[������ʔ�
		memcpy(PayData.Electron_data.Ec.Brand.Nanaco.TerminalNo, &p->Brand.Nanaco.TerminalNo, 20);		// ��ʒ[��ID
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
		break;
	case	BRANDNO_WAON:
		PayData.Electron_data.Ec.e_pay_kind					= EC_WAON_USED;								// ���Z���
		PayData.Electron_data.Ec.Brand.Waon.point_status	= p->Brand.Waon.PointStatus;				// ���p��
		PayData.Electron_data.Ec.Brand.Waon.point			= p->Brand.Waon.GetPoint;					// ����|�C���g
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//		PayData.Electron_data.Ec.Brand.Waon.point_total		= p->Brand.Waon.TotalPoint;					// �݌v�|�C���g
//		PayData.Electron_data.Ec.Brand.Waon.period_point	= p->Brand.Waon.PeriodPoint;				// �Q�N�O�܂łɊl�������|�C���g
//		PayData.Electron_data.Ec.Brand.Waon.period			= p->Brand.Waon.Period;						// �Q�N�O�܂łɊl�������|�C���g�̗L������
//		PayData.Electron_data.Ec.Brand.Waon.card_type		= p->Brand.Waon.CardType;					// �J�[�h����
//		PayData.Electron_data.Ec.Brand.Waon.deal_code		= p->Brand.Waon.DealCode;					// �����ʃR�[�h
		memcpy(PayData.Electron_data.Ec.Brand.Waon.point_total, p->Brand.Waon.TotalPoint,
				sizeof(PayData.Electron_data.Ec.Brand.Waon.point_total));								// �݌v�|�C���g
		memcpy(PayData.Electron_data.Ec.Brand.Waon.Kamei, p->Brand.Waon.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Waon.Kamei));										// �����X��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
		memcpy(PayData.Electron_data.Ec.Brand.Waon.SPRW_ID, &p->Brand.Waon.SPRW_ID, sizeof(PayData.Electron_data.Ec.Brand.Waon.SPRW_ID));	// SPRWID
		break;
	case	BRANDNO_SAPICA:
		PayData.Electron_data.Ec.e_pay_kind					= EC_SAPICA_USED;							// ���Z���
		PayData.Electron_data.Ec.Brand.Sapica.Details_ID	= p->Brand.Sapica.Details_ID;				// ����ID
		memcpy(PayData.Electron_data.Ec.Brand.Sapica.Terminal_ID, &p->Brand.Sapica.Terminal_ID, sizeof(PayData.Electron_data.Ec.Brand.Sapica.Terminal_ID));	// ���̒[��ID
		break;
	case	BRANDNO_ID:
		PayData.Electron_data.Ec.e_pay_kind					= EC_ID_USED;								// ���Z���
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//		PayData.Electron_data.Ec.Brand.Id.Details_ID
//		PayData.Electron_data.Ec.Brand.Id.Terminal_ID
		memcpy(PayData.Electron_data.Ec.Brand.Id.Kamei, p->Brand.Id.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Id.Kamei));										// �����X��
		memcpy(PayData.Electron_data.Ec.Brand.Id.Approval_No, &p->Brand.Id.Approval_No, 7);				// ���F�ԍ�
		memcpy(PayData.Electron_data.Ec.Brand.Id.TerminalNo, &p->Brand.Id.TerminalNo, 13);				// ��ʒ[��ID
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
		break;
	case	BRANDNO_QUIC_PAY:
		PayData.Electron_data.Ec.e_pay_kind					= EC_QUIC_PAY_USED;							// ���Z���
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//		PayData.Electron_data.Ec.Brand.Id.Details_ID
//		PayData.Electron_data.Ec.Brand.Id.Terminal_ID
		memcpy(PayData.Electron_data.Ec.Brand.Quickpay.Kamei, p->Brand.QuicPay.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Quickpay.Kamei));									// �����X��
		memcpy(PayData.Electron_data.Ec.Brand.Quickpay.Approval_No, &p->Brand.QuicPay.Approval_No, 7);	// ���F�ԍ�
		memcpy(PayData.Electron_data.Ec.Brand.Quickpay.TerminalNo, &p->Brand.QuicPay.TerminalNo, 13);	// ��ʒ[��ID
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
		break;
// MH810105(S) MH364301 PiTaPa�Ή�
	case	BRANDNO_PITAPA:
		PayData.Electron_data.Ec.e_pay_kind					= EC_PITAPA_USED;							// ���Z���
		memcpy(PayData.Electron_data.Ec.Brand.Pitapa.Kamei, p->Brand.Pitapa.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Pitapa.Kamei));									// �����X��
		memcpy(PayData.Electron_data.Ec.Brand.Pitapa.Slip_No, &p->Brand.Pitapa.Slip_No, 5);				// �`�[�ԍ�
		memcpy(PayData.Electron_data.Ec.Brand.Pitapa.TerminalNo, &p->Brand.Pitapa.TerminalNo, 13);		// ��ʒ[��ID
		memcpy(PayData.Electron_data.Ec.Brand.Pitapa.Approval_No, &p->Brand.Pitapa.Approval_No, 8);		// ���F�ԍ�
		break;
// MH810105(E) MH364301 PiTaPa�Ή�
	default:
		break;
	}

// MH810103 GG119202(S) �d�q�}�l�[�Ή�
	// �݂Ȃ����ρH
	if( PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 1 ){
		// 20byte��0x20����
		memset( &PayData.Electron_data.Ec.Card_ID[0], 0x20, sizeof(PayData.Electron_data.Ec.Card_ID) );
		if( EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED] ) {
			// ���ꂼ��̃J�[�h������0x30����
			memset( &PayData.Electron_data.Ec.Card_ID[0], 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]) );
			// ���l�߂�ZZ
			memset( &PayData.Electron_data.Ec.Card_ID[0], 'Z', 2 );
		}
	}
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
// MH810103 GG119202(S) �����c���Ɖ�����̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
	else if (p->Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
		// �����c���Ɖ�����̌��ό��ʃf�[�^�ɂ̓}�X�N����Ȃ��J�[�h�ԍ����ʒm����邽�߁A
		// �����ŃJ�[�h�ԍ����}�X�N����
		switch (p->brand_no) {
		case BRANDNO_KOUTSUU:
			max = ECARDID_SIZE_KOUTSUU - 4;
			for (i = 2; i < max; i++) {
				PayData.Electron_data.Ec.Card_ID[i] = '*';		// ���2���A����4���ȊO���}�X�N
			}
			break;
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
		case BRANDNO_EDY:
// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
//// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//		case BRANDNO_NANACO:
//		case BRANDNO_ID:
//// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
			memset(&PayData.Electron_data.Ec.Card_ID[0], '*', 12);	// ����4���ȊO���}�X�N
			break;
		case BRANDNO_WAON:
			// WAON�̓}�X�N����Ēʒm�����
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//			// �����c���Ɖ�����̓|�C���g���b�Z�[�W���󎚂��Ȃ��d�l�̂��߁A
//			// 0xFF���Z�b�g���Ă���
//			PayData.Electron_data.Ec.Brand.Waon.point_status = 0xFF;
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810104 GG119202(S) nanaco�EiD�EQUICPay�Ή�
		case BRANDNO_NANACO:
			// nanaco�̓}�X�N����Ēʒm�����
// MH810104 GG119202(E) nanaco�EiD�EQUICPay�Ή�
			break;
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
//// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//		case BRANDNO_QUIC_PAY:
//			memset(&PayData.Electron_data.Ec.Card_ID[0], '*', 16);	// ����4���ȊO���}�X�N
//			break;
//// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
		case BRANDNO_ID:
		case BRANDNO_QUIC_PAY:
			// iD, QUICPay�͈󎚎��ɃJ�[�h�ԍ����}�X�N����
			break;
// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
		default:
			break;
		}
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
		// ���ό��ʃf�[�^��ۑ�����
		memcpy(&EcAlarm.Ec_Res, p, sizeof(*p));
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	}
// MH810103 GG119202(E) �����c���Ɖ�����̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
	if (p->Result == EPAY_RESULT_MIRYO ||
		p->Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
		// �����m��A�����c���Ɖ��
// MH810105 GG119202(S) ������������W�v�d�l���P
//		if (p->brand_no != BRANDNO_ID && p->brand_no != BRANDNO_QUIC_PAY) {
//			PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
//		}
// MH810105 GG119202(E) ������������W�v�d�l���P
		if (p->brand_no == BRANDNO_WAON) {
			// �����m�莞�̓|�C���g���b�Z�[�W���󎚂��Ȃ����߁A0xFF���Z�b�g���Ă���
			PayData.Electron_data.Ec.Brand.Waon.point_status = 0xFF;
		}
	}
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
	if (PayData.EcResult == EPAY_RESULT_MIRYO ||
		PayData.EcResult == EPAY_RESULT_NG) {
		// ���ό��ʁ������m��A����NG�̏ꍇ�͐��Z���~�f�[�^���M�ΏۂƂ���
		PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.e_pay_kind), NTNET_152_ECARDKIND);
//	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.pay_ryo), NTNET_152_EPAYRYO);
//// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
////	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.Card_ID[0]), NTNET_152_ECARDID);
////	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
//	if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
//		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
//		// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.���J�[�h������0x30���߂����l�߂�ZZ����B
//		// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
//		memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
//		// ���ꂼ��̃J�[�h������0x30����
//		memset( &dummy_Card_ID[0], 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]) );
//		// ���l�߂�ZZ
//		memset( &dummy_Card_ID[0], 'Z', 2 );
//		NTNET_Data152Save((void *)(&dummy_Card_ID[0]), NTNET_152_ECARDID);
//
//		// �⍇���ԍ���0x20h���߂���B
//		memset( &dummy_inquiry_num[0], 0x20, sizeof(dummy_inquiry_num) );
//		NTNET_Data152Save((void *)(&dummy_inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
//	}
//	else {
//		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.Card_ID[0]), NTNET_152_ECARDID);
//		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
//	}
//// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.pay_after), NTNET_152_EPAYAFTER);
	Ec_Data152Save();
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�̈�ւ��ް��̊i�[ (���σ��[�_�ł̃N���W�b�g����)                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcCreUpdatePayData					                   |*/
/*| PARAMETER    : *buf		: ����ް�				                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : hosoda                                                  |*/
/*| Date         : 2019-02-18                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	EcCreUpdatePayData( void *buf )
{
	EC_SETTLEMENT_RES	*p;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//	char	dummy_card_no[20];
//// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	p = (EC_SETTLEMENT_RES*)buf;

	//�W�v�E�̎��� �p (�N���W�b�g)
	memset( &ryo_buf.credit, 0, sizeof(credit_use) );

	memcpy( &ryo_buf.credit.card_name[0], &p->Brand.Credit.Credit_Company[0], sizeof(ryo_buf.credit.card_name) );	// �ڼޯĶ��މ�З���
	memcpy( &ryo_buf.credit.card_no[0], &p->Card_ID[0], sizeof(ryo_buf.credit.card_no) );	// �ڼޯĶ��މ���ԍ��i�E�l�߁j
	ryo_buf.credit.slip_no				= (ulong)p->Brand.Credit.Slip_No;					// �ڼޯĶ��ޓ`�[�ԍ�
	ryo_buf.credit.pay_ryo				= (ulong)p->settlement_data;						// �N���W�b�g���ϊz
	memset( &ryo_buf.credit.ShopAccountNo[0], 0x20, sizeof(ryo_buf.credit.ShopAccountNo) );	// �����X����ԍ�, ���σ��[�_�ł͖���

	ryo_buf.credit.app_no = astoinl(&p->Brand.Credit.Approval_No[0], sizeof(p->Brand.Credit.Approval_No));	// ���F�ԍ�

	memcpy( &ryo_buf.credit.CCT_Num[0], &p->Brand.Credit.Id_No[0], sizeof(p->Brand.Credit.Id_No) );			// �[�����ʔԍ�
	memcpy( &ryo_buf.credit.kid_code[0], &p->Brand.Credit.KID_Code[0], sizeof(ryo_buf.credit.kid_code) );	// KID �R�[�h
	ryo_buf.credit.Identity_Ptrn = p->Brand.Credit.Identity_Ptrn;											// �{�l�m�F�p�^�[��

	//�W�v�E�̎��� �p (�d�q����)
	if(p->Column_No == 0xFF) {
		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;									// �݂Ȃ�����
	}
	PayData.Electron_data.Ec.e_pay_kind	= EC_CREDIT_USED;												// ���Z���
	memcpy(PayData.Electron_data.Ec.Card_ID, p->Card_ID, sizeof(PayData.Electron_data.Ec.Card_ID));		// ����ID
	PayData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);						// ���ϓ���(Normalize)
// MH810105 GG119202(S) ������������W�v�d�l���P
	PayData.EcResult = p->Result;																		// ���ό���
// MH810105 GG119202(E) ������������W�v�d�l���P
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
	if (PayData.EcResult == EPAY_RESULT_NG) {
		// ���ό��ʁ�����NG�̏ꍇ�͐��Z���~�f�[�^���M�ΏۂƂ���
		PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	//���Z���f�[�^ �p
//	NTNET_Data152Save((void *)(&ryo_buf.credit.pay_ryo), NTNET_152_CPAYRYO);				// �N���W�b�g���ϊz
//// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
////	NTNET_Data152Save((void *)(&ryo_buf.credit.card_no[0]), NTNET_152_CCARDNO);				// �N���W�b�g�J�[�h����ԍ�
//	if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
//		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
//		// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.��0x20h���߂���B
//		// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
//		memset( &dummy_card_no[0], 0x20, sizeof(dummy_card_no) );
//		NTNET_Data152Save((void *)(&dummy_card_no[0]), NTNET_152_CCARDNO);					// �N���W�b�g�J�[�h����ԍ�
//	}
//	else {
//		NTNET_Data152Save((void *)(&ryo_buf.credit.card_no[0]), NTNET_152_CCARDNO);			// �N���W�b�g�J�[�h����ԍ�
//	}
//// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//	NTNET_Data152Save((void *)(&ryo_buf.credit.CCT_Num[0]), NTNET_152_CCCTNUM);				// �[�����ʔԍ�
//	NTNET_Data152Save((void *)(&ryo_buf.credit.kid_code[0]), NTNET_152_CKID);				// �j�h�c�R�[�h
//	NTNET_Data152Save((void *)(&ryo_buf.credit.app_no), NTNET_152_CAPPNO);					// ���F�ԍ�
//	NTNET_Data152Save((void *)(&ryo_buf.credit.ShopAccountNo[0]), NTNET_152_CTRADENO);		// �����X����ԍ�, ���σ��[�_�ł͖���
//	NTNET_Data152Save((void *)(&ryo_buf.credit.slip_no), NTNET_152_SLIPNO);					// �`�[�ԍ�
	Ec_Data152Save();
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
}
// MH321800(E) hosoda IC�N���W�b�g�Ή�

/*[]----------------------------------------------------------------------[]*/
/*|	Mifare ���ތ��m�J�n													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2005-11-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	op_MifareStart( void )
{
	if( (MIFARE_CARD_DoesUse) &&									// Mifare�g���ݒ�
		(!IsErrorOccuerd(ERRMDL_MIFARE,1)) &&
		( OPECTL.Ope_mod != 22 )){									// ����X�V���Z�ȊO
		OpMif_snd( 0xA2, 0 );										// Mifare���޾ݽ&�ް��Ǐo���J�n
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|	Mifare�������~�i��~�j												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2005-11-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	op_MifareStop( void )
{
	OpMif_snd( 0xA3, 0 );											// Mifare�������~
}

void	op_MifareStop_with_LED( void )
{
	OpMif_snd( 0xA3, 0 );											// Mifare�������~
}

void	op_Cycle_BUZPIPI( void )
{
	BUZPIPI();
	LagTim500ms( LAG500_MIF_WRITING_FAIL_BUZ, (1*2), op_Cycle_BUZPIPI );	// 1�b�� �Ĵװ�޻ް��
}

void	op_IccLedOff( void )
{
}

void	op_IccLedOff2( void )
{
	queset(OPETCBNO, TIMEOUT12, 0, NULL);
	LagTim500ms( LAG500_MIF_LED_ONOFF, (2*2), op_IccLedOnOff );		// 2�b�� ICC LED�ē_��
}

void	op_IccLedOnOff( void )
{
}

// �G���[�\�������̕\���ɖ߂�
void op_RestoreErrDsp( uchar dsp ) {
	if( dsp ){
		Lcd_Receipt_disp();
	}else{
		dsp_intime( 7, OPECTL.Pr_LokNo );			// ���Ɏ����\��
	}
// MH810105(S) MH364301 �C���{�C�X�Ή�
	OPECTL.f_CrErrDisp = 0;							// �G���[��\��
// MH810105(E) MH364301 �C���{�C�X�Ή�
}

/*[]----------------------------------------------------------------------[]*/
/*| �V�C�����Z�isyusei�ޯ̧�Ɋi�[����Ă����ް���vl_frs�ɾ�Ă���j         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SyuseiDataSet(void)                                     |*/
/*| PARAMETER    : mot_lkno : �C�����t���b�v��                             |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Akiba                                                 |*/
/*| Date         : 2008-03-07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SyuseiDataSet( ushort mot_lkno )
{
	cm27();
	memset(&vl_frs, 0, sizeof(vl_frs));

	vl_frs.lockno	= mot_lkno;										// �C�����E�t���b�v�^���b�N���uNo.(1-324)
	vl_frs.price	= syusei[mot_lkno-1].ryo;						// �C�����E�x���z�i�����{�O�Łj
	vl_frs.in_price = syusei[mot_lkno-1].gen  						// �C�����E����
								+ syusei[mot_lkno-1].sy_wari;		// �C�����E��������,�v���y�C�h,�񐔌�,�N���W�b�g,E�}�l�[�����z
	vl_frs.syubetu	= syusei[mot_lkno-1].syubetu;					// �C�����������

	vl_frs.seisan_oiban = syusei[mot_lkno-1].oiban;					// �C�������Z�ǂ���
	memcpy( &vl_frs.seisan_time.Year, &syusei[mot_lkno-1].oyear, 6 );

	// ��������g��ꂽ�������邽�߂����Ń`�F�b�N����B
	if( syusei[mot_lkno-1].tei_syu ){
		vl_frs.antipassoff_req = 1;									// �A���`�p�X����OFF�v��
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �V�C�����Z�i�C�������������z�̕��ߔ���j                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SyuseiModosiPay(void)                                   |*/
/*| PARAMETER    : rt  �F�߂�l                                            |*/
/*| RETURN VALUE : ret �F2=�ޑK����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Akiba                                                 |*/
/*| Date         : 2008-03-07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	SyuseiModosiPay( short rt )
{
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//short	ret;
//ulong	tyu_zan,tmp_ryo;
//ulong	disp_waribik;
//uchar	end_flg = 0;
//
//	ret = rt;
//	if( (ryo_buf.nyukin == 0)&&(ryo_buf.mis_ryo != 0) ){		// �����Ȃ��ŏC����������������
//		ryo_buf.mis_tyu = 0;
//		tmp_ryo = ryo_buf.tyu_ryo;
//		if( tki_flg ){
//			tmp_ryo = ryo_buf.tei_ryo;
//		}
//
//		disp_waribik = ryo_buf.dis_fee + ryo_buf.dis_tim;
//		if( tmp_ryo < disp_waribik ){
//			end_flg = 1;
//		}else{
//			tmp_ryo -= disp_waribik;
//		}
//		if( tmp_ryo < ryo_buf.mis_wari ){						// ���ԗ���>�C���������z
//			end_flg = 1;
//		}
//
//		if( !end_flg ){
//			tyu_zan = tmp_ryo - ryo_buf.mis_wari;				// ���ԗ���-�C���������z�i�c�蒓�ԗ����j
//			if( tyu_zan <= ryo_buf.mis_ryo ){					// �c�蒓�ԗ���<�C�������������z
//				PayData.MMTwari = ryo_buf.mis_ryo - tyu_zan;
//				if( (prm_get(COM_PRM, S_TYP, 98, 1, 1) == 1)||	// �C�������������z�̕��߂���ݒ�
//					(Syusei_Select_Flg == 2) ){					// �C�������������z�̕��߂���I��
//					ryo_buf.mis_tyu = ryo_buf.mis_ryo - tyu_zan;// �C�������������z-�c�蒓�ԗ����i�C�����ߊz�j
//					ret = 2;									// ���Z�I��(�ޗL��)
//				}else{
//					PayData.Zengakufg |= 0x04;
//				}
//			}
//		}else{
//			PayData.MMTwari = ryo_buf.mis_ryo;
//			if( (prm_get(COM_PRM, S_TYP, 98, 1, 1) == 1)||		// �C�������������z�̕��߂���ݒ�
//				(Syusei_Select_Flg == 2) ){						// �C�������������z�̕��߂���I��
//				ryo_buf.mis_tyu = ryo_buf.mis_ryo;
//				ret = 2;
//			}else{
//				PayData.Zengakufg |= 0x04;
//			}
//		}
//	}
//	return ret;
	return rt;
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
}

uchar	SyuseiEndKeyChk( void )
{
uchar	ret,set;

	ret = 0;
	set = (uchar)prm_get(COM_PRM, S_TYP, 98, 1, 3);
	if( set ){													// ��������������
		if( OPECTL.Pay_mod == 2 ){								// �C�����Z��
			ret = 2;											// ��݁A�����A�d�q���ρA���Cذ�ށAMifare����
			if( (set == 1)&&									// ����̑}������
				(vl_frs.antipassoff_req)&&(tki_flg==OFF) ){		// �C�����Œ�����g�p���͒�����g�p�ς�
				ret = 1;										// ��݁A�����A�d�q���ς���
			}
		}
	}
	return(ret);
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �ʒm�ԍ����v�Z����																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: op_Calc_Notice( ushort stimer, struct clk_rec *pDate, short sParkNo )			   |*/
/*| PARAMETER	: ushort stimer,		: 10ms���Ƃ̃^�C�}�[�l									   |*/
/*|               struct clk_rec *pDate : ����													   |*/
/*|               short sParkNo         : ���ԏ�ԍ�											   |*/
/*| RETURN VALUE: ret                   : �ʒm�ԍ�												   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																		   |*/
/*| Date		: 2006-02-21																	   |*/
/*| Update		:																				   |*/
/*| ���l		:NT4500EX����̗��p���W�b�N�i�ύX����j 										   |*/
/*[]------------------------------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static long	op_Calc_Notice( ushort stimer, struct clk_rec *pDate, long sParkNo )/*								*/
{																				/*								*/
	long motodat1, motodat2, worknum, hash;										/*								*/
	long nen, tuki, hi, ji, fun;												/*								*/
	long chunum, timer;															/*								*/
																				/*								*/
	nen = (long)( pDate->year % GETA_10_2 );									/*								*/
	tuki = (long)pDate->mont;													/*								*/
	hi = (long)pDate->date;														/*								*/
	ji = (long)pDate->hour;														/*								*/
	fun = (long)pDate->minu;													/*								*/
	chunum = (long)sParkNo;														/*								*/
	timer  = (long)stimer;														/*								*/
																				/*								*/
    motodat1 = (nen * GETA_10_6 + timer * GETA_10_3 + chunum) ;								/* '8���f�[�^1�쐬	*/
    motodat2 = ((hi + fun) * GETA_10_6 + ji * GETA_10_5 + tuki * GETA_10_2 + timer + fun) ;	/* '8���f�[�^2�쐬	*/
																				/*								*/
    worknum = (motodat1 ^ motodat2);											/*								*/
    hash = (worknum % MAXVAL);													/*								*/
																				/*								*/
	return( hash );																/*								*/
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �Ïؔԍ����v�Z����																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: op_Calc_Ansyou( long lNoticeNo )												   |*/
/*| PARAMETER	: long lNoticeNo : �ʒm�ԍ�														   |*/
/*| RETURN VALUE: ret            : �Ïؔԍ�														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																		   |*/
/*| Date		: 2006-02-21																	   |*/
/*| Update		:																				   |*/
/*| ���l		:NT4500EX����̗��p���W�b�N�i�ύX����j	     									   |*/
/*[]------------------------------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static long	op_Calc_Ansyou( long lNoticeNo )									/*								*/
{																				/*								*/
	long ansyo, val_1, Setting[4];												/*								*/
	long seed1, seed2, seed3, shift1, shift2, shift3;							/*								*/
	ushort temp=GETA_10_3,i;
	memset(Setting,0,sizeof(Setting));											/*								*/
	val_1 = lNoticeNo;

	for(i=0;i<4;i++){
		Setting[i] = ONE_CHARACTER_CHECK(decode_number,temp);
		temp/=10;
	}																			/*								*/
																				/*								*/
	shift1 = (long)Setting[0];													/* '�P���ڃf�[�^				*/
	shift2 = (long)Setting[1];													/* '�Q���ڃf�[�^				*/
																				/*								*/
	shift3 = (shift1 + shift2);													/* �f�[�^����					*/
																				/*								*/
	seed1 = (long)Setting[2];													/* '�R���ڃf�[�^				*/
	seed2 = (long)Setting[3];													/* '�S���ڃf�[�^				*/

	seed3 = (seed1 + seed2);													/* �f�[�^����					*/

	shift1 = ( ( shift1 % 4L ) + 1L );											/* '�f�[�^���I�[�o�[�t���[���Ȃ��悤�ɕϐ���1�`4�̃f�[�^�ɕϊ� */
	shift2 = ( ( shift2 % 4L ) + 1L );											/*								*/
	shift3 = ( ( shift3 % 4L ) + 1L );											/*								*/
	seed1 = ( ( seed1 % 4L ) + 1L );											/*								*/
	seed2 = ( ( seed2 % 4L ) + 1L );											/*								*/
	seed3 = ( ( seed3 % 4L ) + 1L );											/*								*/
																				/*								*/ 
	ansyo = ( ( ( val_1 << shift1 ) * op_Calc_BekiJyo( 10, seed1 ) ) ^ ( ( val_1 << shift2 ) * op_Calc_BekiJyo( 10, seed2 ) ) ^ ( ( val_1 << shift3 ) * op_Calc_BekiJyo( 10, seed3 ) ) ^
	          ( ( val_1 << 4 ) * GETA_10_4 ) ^ ( ( val_1 << 1 ) * GETA_10_3 ) ^ ( ( val_1 << 3 ) * GETA_10_2 ) );


	ansyo = ( ( ( val_1 << shift1 ) * op_Calc_BekiJyo( 10, seed1 ) ) ^ ( ( val_1 << shift2 ) * op_Calc_BekiJyo( 10, seed2 ) ) ^ ( ( val_1 << shift3 ) * op_Calc_BekiJyo( 10, seed3 ) ) ^
	          ( ( val_1 << 4 ) * GETA_10_4 ) ^ ( ( val_1 << 1 ) * GETA_10_3 ) ^ ( ( val_1 << 3 ) * GETA_10_2 ) );

																				/* '�ʒm�ԍ����Ïؔԍ��ɉ��H	*/
																				/*								*/
																				/*								*/
	ansyo = (ansyo % MAXVAL);													/* '�Ïؔԍ���4���ɕϊ�			*/
																				/*								*/
																				/*								*/
	seed1 = ansyo / GETA_10_3;													/* 1000�̌�						*/
																				/*								*/
	seed2 = (ansyo / GETA_10_2 ) % GETA_10_1;									/* 100�̌�						*/
																				/*								*/
	seed3 = (ansyo / GETA_10_1 ) % GETA_10_1;									/* 10�̌�						*/
																				/*								*/
	shift1 = ansyo % GETA_10_1;													/* 10�̌�						*/
																				/*								*/
	if( seed1 == 0){															/* 0?(Y)						*/
		seed1 = ZeroToMitsu;													/* 0->3							*/
	}																			/*								*/
	if( seed2 == 0){															/* 0?(Y)						*/
		seed2 = ZeroToMitsu;													/* 0->3							*/
	}																			/*								*/
	if( seed3 == 0){															/* 0?(Y)						*/
		seed3 = ZeroToMitsu;													/* 0->3							*/
	}																			/*								*/
	if( shift1 == 0){															/* 0?(Y)						*/
		shift1 = ZeroToMitsu;													/* 0->3							*/
	}																			/*								*/
																				/*								*/
	ansyo = seed1*GETA_10_3 + seed2*GETA_10_2 + seed3*GETA_10_1 + shift1;		/*	�Ïؔԍ������@�@			*/
																				/*								*/
	return( ansyo );															/*								*/
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �ׂ�����v�Z����																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: op_Calc_BekiJyo( long Val1, long Val2 )										   |*/
/*| PARAMETER	: long Val1 : �ׂ��悳���l													   |*/
/*|				: long Val2 : �ׂ���															   |*/
/*| RETURN VALUE: ret       : ����																   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																		   |*/
/*| Date		: 2006-02-21																	   |*/
/*| Update		:																				   |*/
/*| ���l		:NT4500EX����̗��p���W�b�N														   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static long	op_Calc_BekiJyo( long Val1, long Val2 )								/*								*/
{																				/*								*/
	long Result;																/*								*/
	long i;																		/*								*/
																				/*								*/
	if ( Val2 <= 0 ) {															/*								*/
		Result = 1L;															/* �H�̂O��͂P�Ȃ̂ŁA�P�ŏ����� */
	} else {																	/*								*/
		Result = Val1;															/*								*/
		for ( i = 1L ; i < Val2 ; i++ ) {										/*								*/
			Result = ( Result * Val1 );											/*								*/
		}																		/*								*/
	}																			/*								*/
	return Result;																/*								*/
}
unsigned short	Cardress_DoorOpen( void )
{
	long advice_No,ansyou_No;
	short	Save_OPE_red;
	ushort	ret;
	ushort	msg=0;
	ushort	input=0;
	uchar	input_cnt = 0;
	char	exitflag=OFF;
	
	dispclr();												/* Display All Clear */

	// TODO:��ǔԍ��͕W���p�̋��ʃp�����[�^���擾���邱��
	decode_number = 4134;

	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[0] );				/* ���d�����b�N������ */
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[1] );				/* "   �h�A�J�ԍ�����͂��A       " */
// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ�������
//	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[2] );				/* "   �Ō�ɐ��Z�������ĉ�����   " */
//	Fun_Dsp(FUNMSG[8]);										// "�@                       �I�� "
	grachr( 4, 0, 9, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[2] );				/* "   �Ō��                     " */
	PKTcmd_text_1_backgroundcolor( COLOR_F1BLUE );
	grachr( 4, 9, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[4] );				/* " �m��                         " */
	PKTcmd_text_1_backgroundcolor( COLOR_WHITE );
	grachr( 4, 15, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[5] );				/* "�������ĉ�����                " */
	Fun_Dsp(FUNMSG[125]);									// " �m��                    �I�� "
// MH810100(E) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ�������
	advice_No = (op_Calc_Notice((ushort)LifeTimGet(),&CLK_REC,(long)CPrmSS[S_SYS][1])%GETA_10_4);		/* �ʒm�ԍ��̐��� */
	ansyou_No = op_Calc_Ansyou(advice_No);						/* �Ïؔԍ��̐��� */
	
// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
//	teninb((ushort)advice_No,4,1,9, COLOR_BLACK);			/* �ʒm�ԍ��̕\�� */
	teninb((ushort)advice_No,4,1,11, COLOR_BLACK);			/* �ʒm�ԍ��̕\�� */
// MH810100(E) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j

	Save_OPE_red = OPE_red;
	OPE_red = 2;										// ذ�ް�����r�o

	for ( ret = 0; ret == 0; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG){
			BUZPI();
			ret = MOD_CHG;
		}
		switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:														/* �I��(F5) */
				if(exitflag == OFF){
					BUZPI();
					ret = MOD_EXT;
				}
				break;
			case KEY_TEN_F3:														/* ����(F4) */
			case KEY_TEN_F2:														/* �|(F2) */
				break;																
			case KEY_TEN_F1:														/* �{(F1) */
				if (ansyou_No != input) {
					//�Ïؔԍ�(Btype)���������Ȃ�������
					BUZPIPI();
					input = 0;
					input_cnt = 0;
// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
//					teninb_pass2( input, 4, 5, 9, input_cnt );		// �ԍ��\��
					teninb_pass2( input, 4, 5, 11, input_cnt );		// �ԍ��\��
// MH810100(E) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
					break;
				}
				BUZPI();
				Lagcan( OPETCBNO, 16 );									// �ޱ�J��� ��ۯ���ϰؾ��
				Lagcan( OPETCBNO, 17 );									// �ޱ����ޕ� ���ۯ��҂���ϰؾ��
				grachr(3, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, sercret_Str[3]);					/* "<<�h�A���b�N���������܂���>>" */
				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLSMSG[0]);						/* "                            " */
				Lagtim( OPETCBNO, 6, 3*50 );								/* �^�C�}�|�ݒ� */
				exitflag = ON;
				OPE_SIG_OUT_DOORNOBU_LOCK(0);								// �ޱ��ނ�ۯ�����
				wmonlg( OPMON_LOCK_OPEN, 0, 0 );							// ����o�^
				attend_no = 0;
				wopelg2( OPLOG_NO_CARD_OPEN, 0L, (ulong)attend_no );		// ���엚��o�^(�����J��)
				if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// �ޱ�J��� ��ۯ�������ϰ�l���L���͈͓�
					DoorLockTimer = 1;
					Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// �ޱ�J��� ��ۯ�������ϰ����
				}
				break;

			case KEY_TEN:			/* ����(�e���L�[) */
				BUZPI();
				input =  ((input%1000) * 10 )+( msg - KEY_TEN );
				input_cnt++;
				if(input_cnt > 4){
					input_cnt = 4;
				}
// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
//				teninb_pass2( input, 4, 5, 9, input_cnt);	// �\��������
				teninb_pass2( input, 4, 5, 11, input_cnt);	// �\��������
// MH810100(E) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
				break;

			case KEY_TEN_CL:		// �ݷ�[C] �����
				BUZPI();
				input = 0;
				input_cnt = 0;
// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
//				teninb_pass2( input, 4, 5, 9, input_cnt);	// ���Ԉʒu�ԍ��\��
				teninb_pass2( input, 4, 5, 11, input_cnt);	// ���Ԉʒu�ԍ��\��
// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
				break;
			case TIMEOUT6:															/* 4�����͌ォ��ʑJ�ڗp��ѱ�� */
//				return 0;
				ret = MOD_EXT;
			default:
				break;
		}
	}

	OPE_red = Save_OPE_red;
	return	ret;
}

static void	teninb_pass2( ushort nu, ushort in_k, ushort lin, ushort col, uchar pas_cnt)
{
	uchar	c_bf[4];
	ushort	wcol;
	short	i;
	uchar	p_cnt;

	nu %= 10000;
	if( in_k > 4 )	in_k = 4;

	intoas( c_bf, nu, in_k );

// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
//	for( i=0, wcol=0; i<in_k; i++, wcol+=3 ){
	for( i=0, wcol=0; i<in_k; i++, wcol+=2 ){
// MH810100(E) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
		if(pas_cnt == 0){
			//�󔒕\��
			p_cnt = 0;
			c_bf[p_cnt] = ' ';
			numchr(lin, (ushort)(col+wcol), COLOR_FIREBRICK, LCD_BLINK_OFF, c_bf[p_cnt]);
		}else{
			p_cnt = (uchar)(4 - pas_cnt);
			numchr(lin, (ushort)(col+wcol), COLOR_FIREBRICK, LCD_BLINK_OFF, c_bf[p_cnt]);
			pas_cnt--;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*| ���C���[�_���̃J�[�h�l�܂蔭������										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : req = �v������											|*/
/*|							0:����������									|*/
/*|							1:���Z����or���Z���~���̏���					|*/
/*|							2:�r�o�����܂��l�܂��Ă�����^�C�}�J�n		|*/
/*|							3:�A������}�~���Ԍo��							|*/
/*|							4:�������{�^���ɂ��J�[�h�r�o����			|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Ise                                                      |*/
/*| Date         : 2009-06-18                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void StackCardEject(uchar req)
{
	uchar RetryMax;																/* �����r�o�񐔐ݒ�l */

	uchar retry_wk = CardStackRetry;
	RetryMax = (uchar)(prm_get(COM_PRM, S_SYS, 14, 2, 1));						/* �����r�o�񐔐ݒ�擾 */
	switch(req)																	/* �v�����ɏ����𕪊� */
	{
	case 1:																		/* ���Z���~or���Z���� */
		if(IS_ERR_RED_STACK && RetryMax != 0){
			if(OutputRelease != 0 || RetryMax <= EjectActionCnt){
				EjectActionCnt = 0;
				CardStackStatus = 0;
			if(!rd_mov(1))														/* �J�[�h�r�o�v�� */
			{
				wmonlg(OPMON_CARDSTACK_OCC, 0, 0);								/* �r�o����J�n���j�^���O�o�^ */
				OutputRelease = 0;												/* ���̃G���[�������O�o�^���� */
				CardStackRetry = 1;												/* �r�o����J�n */
			}
			}
		}
		else if(!IS_ERR_RED_STACK && CardStackRetry)																	/* �J�[�h�l�܂������ */
		{
			if(!OutputRelease)													/* �G���[�����ɂ�郍�O�o�^���� */
			{
				wmonlg(OPMON_CARDSTACK_REL, 0, 0);								/* �r�o���슮�����j�^���O�o�^ */
				OutputRelease = 1;												/* ������G���[�������O�o�^�Ȃ� */
			}
			else																/* �G���[�������O�o�^�Ȃ� */
			{
				OutputRelease = 2;												/* ���O�^�X�N����̃J�[�h�l�܂�ʒm�҂� */
			}
			CardStackRetry = 0;													/* �r�o���쓮��I�� */
		}
		break;																	/* ���򏈗��I�� */
	case 2:																		/* �r�o�����̎��C���[�_�ʒm */
		if(!CardStackRetry){
			/* �r�o���쒆�ȊO�̓��슮���̓��[�h�R�}���h�o���Ȃ� */
			break;
		}
		if(IS_ERR_RED_STACK)													/* �J�[�h�l�܂蔭���� */
		{
			if(!CardStackStatus){											// ��ϰ�N�����Ă��Ȃ�
				EjectActionCnt++;
			if((RetryMax == 99) || (EjectActionCnt < RetryMax))					/* �����r�o����ݒ�񐔕��J��Ԃ� */
			{
				Lagtim(OPETCBNO, 3, CARD_EJECT_AUTOLAG);						/* �����r�o����^�C�}�[�J�n */
			}
			else																/* �����r�o���삪�ݒ�񐔎��s�ς� */
			{
				EjectSafetyTime = LifeTimGet();									/* �������{�^���ł̔r�o����J�n */
				CardStackRetry = 0;												/* �r�o����I��*/
			}
			CardStackStatus = 1;												/* �r�o�L���^�C�}�J�n */
			}
		}
		else																	/* �J�[�h�l�܂肪���� */
		{
			EjectActionCnt = 0;													/* �r�o����񐔏����� */
			CardStackStatus = 0;												/* ����r�o����\ */
			CardStackRetry = 0;													/* �r�o����I�� */
			if(!OutputRelease)													/* �G���[�����ɂ�郍�O�o�^���� */
			{
				wmonlg(OPMON_CARDSTACK_REL, 0, 0);								/* �r�o���슮�����j�^���O�o�^ */
				OutputRelease = 1;												/* ������G���[�������O�o�^�Ȃ� */
			}
		}
		break;																	/* ���򏈗��I�� */
	case 3:																		/* �J�[�h�l�܂蒆�̎������{�^�� */
		if(CardStackStatus == 1)												/* �r�o����L���^�C�}���쒆 */
		{
			if((EjectActionCnt >= RetryMax) && (RetryMax != 99 && RetryMax != 0))	/* �ݒ�񐔕��̎����r�o�����s���� */
			{
				if(CARD_EJECT_MANULAG <= LifePastTimGet(EjectSafetyTime))		/* �r�o���얳�����Ԍo�� */
				{
					if(!rd_mov(1))												/* �J�[�h�r�o�v�� */
					{
						CardStackStatus = 0;									/* �r�o����v���ς� */
						wmonlg(OPMON_CARDSTACK_OCC, 0, 0);								/* �r�o����J�n���j�^���O�o�^ */
						OutputRelease = 0;												/* ���̃G���[�������O�o�^���� */
						EjectActionCnt = 0;
						CardStackRetry = 1;										/* �r�o����J�n */
					}
				}
			}
		}
		break;																	/* ���򏈗��I�� */
	case 4:																		/* �����r�o����^�C�}���荞�� */
		if(CardStackStatus == 1)												/* �r�o����L���^�C�}���쒆 */
		{
			if((EjectActionCnt < RetryMax) || (RetryMax == 99))					/* �����r�o��ݒ�񐔕��J��Ԃ� */
			{
				if(!rd_mov(1))													/* �J�[�h�r�o�v�� */
				{
					CardStackStatus = 0;										/* �r�o����v���ς� */
				}
			}
		}
		break;																	/* ���򏈗��I�� */
	case 5:																		/* ���O�^�X�N���J�[�h�l�܂�ʒm */
		if(IS_ERR_RED_STACK && (RetryMax > EjectActionCnt)){					/* �J�[�h�l�܂蔭���� */
			if(EjectActionCnt == 0 && CardStackRetry == 0){
				wmonlg(OPMON_CARDSTACK_OCC, 0, 0);								/* �r�o����J�n���j�^���O�o�^ */
				if(!rd_mov(1))													/* �J�[�h�r�o�v�� */
				{
					CardStackRetry = 1;											/* �r�o���쒆*/
					OutputRelease = 0;											/* ���̃G���[�������O�o�^���� */
				}
			}
		}
		else if(!OutputRelease)													/* �G���[�����ɂ�郍�O�o�^���� */
		{
			if(!IS_ERR_RED_STACK){
				EjectActionCnt = 0;													/* �r�o����񐔏����� */
				CardStackStatus = 0;												/* ����r�o����\ */
				wmonlg(OPMON_CARDSTACK_REL, 0, 0);								/* �r�o���슮�����j�^���O�o�^ */
				OutputRelease = 1;												/* ������G���[�������O�o�^�Ȃ� */
				CardStackRetry = 0;
			}
		}
		break;																	/* ���򏈗��I�� */
	default:																	/* ��L�ȊO */
		break;																	/* �K��O�͂Ȃɂ����Ȃ� */
	}
	if(retry_wk && !CardStackRetry && !IS_ERR_RED_STACK){					// ����J�[�h�l�܂肪��������
		opr_snd( 3 );
	}

}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
///*[]----------------------------------------------------------------------[]*/
///*| �e���L�[���͕ێ��֐�                                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : Key_Event_Set( msg )         	                       |*/
///*| PARAMETER    : msg:����Ď��                                           |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       : namioka                                                 |*/
///*| Date         : 2009-10-26                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
//void	Key_Event_Set( ushort msg )
//{
//	uchar	i;
//	uchar	param;
//	
//	param = (uchar)prm_get(COM_PRM, S_TYP, 97, 1, 1);
//	
//	switch( param ){
//		case	1:
//			break;
//		case	2:
//			if( SFV_DAT.safe_cal_do != 0 )
//				return;
//			break;
//		case	0:
//		default:
//			return;
//	}
//		
//	if( KEY_TEN0to9(msg) == KEY_TEN ){
//		for( i=0; i < TBL_CNT(key_event_keep); i++ ){
//			if( !key_event_keep[i] ){
//				key_event_keep[i] = msg;
//				break;
//			}
//		}
//		
//		if( i == TBL_CNT(key_event_keep) ){
//			memmove( &key_event_keep[0], &key_event_keep[1], (size_t)(sizeof( key_event_keep )-1));
//			key_event_keep[i-1] = msg;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*| �e���L�[���͕����֐�                                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : Key_Event_Get( msg )         	                       |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN		 : ret:�ϊ���̎Ԏ��ԍ�                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       : namioka                                                 |*/
///*| Date         : 2009-10-26                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
//ushort	Key_Event_Get( void )
//{
//	uchar	i;
//	ushort	ret=0;
//	
//	for( i=0; i < TBL_CNT(key_event_keep); i++ ){
//		if( key_event_keep[i] ){
//			ret = (ushort)(( ret % knum_ket ) * 10 )+( key_event_keep[i] - KEY_TEN );
//		}
//	}
//	memset( key_event_keep, 0, sizeof( key_event_keep ));
//	return ret;
//}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_

//[]----------------------------------------------------------------------[]
///	@brief			�₢���킹���Ȃǂ̃C�x���g�}�~�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@return			ret 0:�}�~�Ȃ� 1:�}�~
///	@author			
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/10/27<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char	Op_Event_Disable( ushort msg )
{
	char ret=0;
	char i;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( edy_auto_com ){
//		switch( msg ){
//			case CLOCK_CHG:						// ���v�͗�O�Ŏ����̍X�V���s�킹��ׂɏ������s��
//				break;
//			case ARC_CR_R_EVT:					// ����IN
//				read_sht_opn();					// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
//				if( OPE_red != 2 ){				// �����r�o���[�h�ł͂Ȃ��ꍇ
//					opr_snd( 13 );				// �ۗ��ʒu�����o������
//				}
//				ope_anm( AVM_CARD_ERR1 );	// ��������̃J�[�h�͎g���܂���
//				ret = 2;
//				break;
//			case ARC_CR_E_EVT:					
//				rd_shutter();					// �V���b�^�[��߂�
//				break;
//			case KEY_DOOR1:				// �h�A�J�ʒm�i�h�A���J���Ă���̂��A�܂��Ă���̂���DoorCloseChk()�Ŕ��f����j
//				msg = KEY_DOOR1_DUMMY;
//				ret = 1;						// �C�x���g�ێ����āA�Z���^�[�ʐM��ɍs��
//				break;
//
//			case TIMEOUT1:						// �o�b�N���C�g�A��ʐ���p�^�C�}�[�i���ړI�^�C�}�[�j
//			case TIMEOUT4:						// �s�������o�ɊĎ��p�^�C�}�[
//			case TIMEOUT7:						// �V���b�^�[�J��Ԍp���Ď��^�C�}�[
//			case TIMEOUT27:						// �l�̌��o�M���}�X�N����
//			case OPE_OPNCLS_EVT:				// �x�Ə�ԕω��ʒm
//			case MID_RECI_SW_TIMOUT:			// �̎��ؔ��s�\��ϰ��ѱ�Ēʒm
//			// KEY_INSIG_LC1��KEY_MODECHG���͒ʐM����ON��OFF�AOFF��ON�����\��������̂�
//			// �C�x���g�ێ������A�^�C���A�E�g��ɏ�Ԃ����Ĕ��f����
//				ret = 1;						// �C�x���g�ێ����āA�Z���^�[�ʐM��ɍs��
//				break;
//			default:
//				ret = 2;						// ���̑��̃C�x���g�͎󂯎̂Ă�
//				break;
//		}
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	if( ret == 1 ){											// ����ēo�^�����H
		for( i=0;i<KEEP_EVT_MAX && Keep_Evt_Buf[i];i++ );
		if(i >= KEEP_EVT_MAX){
			i = KEEP_EVT_MAX -1;// �͈͊O�ւ̃A�N�Z�X�u���b�N
		}
		Keep_Evt_Buf[i] = msg;								// ����ĕێ�
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�Z���^�[�ʐM���Ȃǂɗ}�~�����C�x���g�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@return			0:�v���Ȃ� 1:��ʑJ�ڏ�������
///	@author			T.Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/12/08<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char	Op_Event_enable( void )
{
	ushort evt=0;
	char i;
	
	while( 1 ){

		if( CP_MODECHG == 1 ){
			evt = KEY_MODECHG;							// ��Ì� ON
			break;
		}
		// ���̏�����t��������ꍇ�͂����ɋL�ځ�

		// ���̏�����t��������ꍇ�͂����ɋL�ځ�
		break;
	}
	
	if( evt ){
		OPECTL.on_off = 1;
		queset( OPETCBNO, evt, sizeof(OPECTL.on_off), &OPECTL.on_off );		// �ۗ����̲���Ă�ʒm
	}
	
	for( i=0; i<KEEP_EVT_MAX; i++ ){
		if( Keep_Evt_Buf[i] ){
			queset( OPETCBNO, Keep_Evt_Buf[i], 0, 0 );		// �ۗ����̲���Ă�ʒm
		}else{
			break;
		}
	}
	memset( Keep_Evt_Buf, 0, sizeof( Keep_Evt_Buf ));
	
	return(char)(evt != 0);
}

/*[]----------------------------------------------------------------------[]*/
/*| RTC�ւ̃t���O���W�X�^�擾�v���֐�                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_RtcFlagRegisterRead     	  	                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN		 : void 				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : tanaka                                                  |*/
/*| Date         : 2012-1-13                                               |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	op_RtcFlagRegisterRead( void )
{
	I2C_REQUEST 			request;
	ushort					ret;

	request.TaskNoTo	 			= I2C_TASK_OPE;							// ope�^�X�N��I2CREQUEST����������ʒm
	request.DeviceNo 				= I2C_DN_RTC;							// RTC�ւ̗v��
	request.RequestCode    			= I2C_RC_RTC_STATUS_RCV;				// 
	request.I2cReqDataInfo.RWCnt	= RTC_FR_SIZE;							// 
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.flag_reg;					// 
	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );					// 
	if( ret != RIIC_RSLT_OK ){
		// �����ŃG���[�o�^���͂��܂����ˁH
	}

	if( (RTC_CLOCK.flag_reg & 0x03) != 0x00 )
	{
		//VLF(���U��H�d���ቺ)�̏ꍇ
		if((RTC_CLOCK.flag_reg & 0x02) == 0x02)
		{
			err_chk( ERRMDL_MAIN, ERR_MAIN_RTCCPU_VOLDWN, 2, 0, 0);			/* VLF(���U��H�d���ቺ	*/
		}
		//VDET(���x�⏞��H�d���ቺ)�̏ꍇ
		if(( RTC_CLOCK.flag_reg & 0x01) == 0x01 )
		{
			err_chk( ERRMDL_MAIN, ERR_MAIN_RTCTMPREC_VOLDWN, 2, 0, 0);		/* VDET(���x�⏞��H�d���ቺ) */
		}
		
		request.RequestCode 			= I2C_RC_RTC_EX_FLAG_CTL_SND;		// ���W�X�^�i3��ށj������
		request.I2cReqDataInfo.RWCnt	= RTC_EX_FR_CR_SIZE;				// Write/Read����Byte��(3)
		RTC_CLOCK.ex_reg				= RTC_EX_INIT;						// TEST		WADA	USEL	TE		FSEL1	FSEL0	TSEL1	TSEL0(0x20)
		RTC_CLOCK.flag_reg	 			= RTC_FR_INIT;						// ---		---		UF		TF		AF		EVF		VLF		VDET(0x00)
		RTC_CLOCK.ctrl_reg	 			= RTC_CR_INIT;						// CSEL1	CSEL0	UIE		TIE		AIE		EIE		---		RESET(0x60)
		request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.ex_reg;				// Write/Read����Buffer�ւ�Pointer
	}else{
		request.RequestCode    			=	I2C_RC_RTC_FLAG_SND;			// flag ���W�X�^������
		request.I2cReqDataInfo.RWCnt	=	RTC_FR_SIZE;					// Write/Read����Byte��(1)
		RTC_CLOCK.flag_reg				=	RTC_FR_INIT;					// flag register
		request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.flag_reg;			// Write/Read����Buffer�ւ�Pointer
	}

	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );					// 
	if( ret != RIIC_RSLT_OK ){
		// �����ŃG���[�o�^���͂��܂����ˁH
	}
	
	return;
}
/*[]----------------------------------------------------------------------[]*/
/*| RTC�̃��W�X�^�������֐�  		                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_RtcRegisterInit     	    	                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN		 : void 				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : tanaka                                                  |*/
/*| Date         : 2012-1-13                                               |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	op_RtcRegisterInit( void )
{
	I2C_REQUEST 			request;
	ushort					ret;

	request.TaskNoTo	 			= I2C_TASK_OPE;					// ope�^�X�N��I2CREQUEST����������ʒm
	request.DeviceNo 				= I2C_DN_RTC;					// RTC�ւ̗v��
	request.RequestCode    			= I2C_RC_RTC_STATUS_RCV;		// 
	request.I2cReqDataInfo.RWCnt	= RTC_FR_SIZE;					// 
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.flag_reg;			// 

	//FLAG���W�X�^�G���[���菈��( VLF(���U��H�d���ቺ) orVDET(���x�⏞��H�d���ቺ))�̏ꍇ
	if( (RTC_CLOCK.flag_reg & 0x03) != 0x00 )
	{
		//VLF(���U��H�d���ቺ)�̏ꍇ
		if((RTC_CLOCK.flag_reg & 0x02) == 0x02)
		{
			err_chk( ERRMDL_MAIN, ERR_MAIN_RTCCPU_VOLDWN, 2, 0, 0);				/* VLF(���U��H�d���ቺ	*/
		}
		//VDET(���x�⏞��H�d���ቺ)�̏ꍇ
		if(( RTC_CLOCK.flag_reg & 0x01) == 0x01 )
		{
			err_chk( ERRMDL_MAIN, ERR_MAIN_RTCTMPREC_VOLDWN, 2, 0, 0);			/* VDET(���x�⏞��H�d���ቺ) */
		}		
		request.RequestCode   			=	I2C_RC_RTC_EX_FLAG_CTL_SND;		//���W�X�^�i3��ށj������
		request.I2cReqDataInfo.RWCnt	=	RTC_EX_FR_CR_SIZE;				// Write/Read����Byte��(1)
		RTC_CLOCK.ex_reg				=	RTC_EX_INIT;					// ex register
		RTC_CLOCK.flag_reg	 			=	RTC_FR_INIT;					// flag register
		RTC_CLOCK.ctrl_reg	 			=	RTC_CR_INIT;					// ctrl register
		request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.ex_reg;				// Write/Read����Buffer�ւ�Pointer
	}
	//�G���[�Ȃ��̏ꍇ
	else
	{
		request.RequestCode    			=	I2C_RC_RTC_FLAG_SND;			// flag ���W�X�^������
		request.I2cReqDataInfo.RWCnt	=	RTC_FR_SIZE;					// Write/Read����Byte��(1)
		RTC_CLOCK.flag_reg				=	RTC_FR_INIT;					// flag register
		request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.flag_reg;				// Write/Read����Buffer�ւ�Pointer
	}
	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );					// 

	if( ret != RIIC_RSLT_OK ){
		// �����ŃG���[�o�^���͂��܂����ˁH
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*|            timset( struct clk_rec * )                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|            set time                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Argument : struct clk_rec *                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Return : 0: OK / -1: NG                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Y.Nonaka                                                |*/
/*| Date        :  2000- 7-29                                              |*/
/*| Update      :  2001-11-27 Y.Takahashi A1H                              |*/
/*[]------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/

unsigned char	timset( struct clk_rec *irq1_clk )
{

	I2C_REQUEST 			request;
	ushort					ret;
	uchar					year;
	
	request.TaskNoTo	 			= I2C_TASK_OPE;					// ope�^�X�N��I2CREQUEST����������ʒm
	request.DeviceNo 				= I2C_DN_RTC;					// RTC�ւ̗v��
	request.RequestCode    			= I2C_RC_RTC_TIME_SND;			// 
	request.I2cReqDataInfo.RWCnt	= (RTC_RW_SIZE - 1);			// 

	RTC_CLOCK.freq100sec	= 0;
	RTC_CLOCK.sec    = binbcd( irq1_clk->seco );					// Second (BCD)
	RTC_CLOCK.min    = binbcd( irq1_clk->minu );					// Minute (BCD)
	RTC_CLOCK.hour   = binbcd( irq1_clk->hour );					// Hour (BCD)
	RTC_CLOCK.week   = 0;
	RTC_CLOCK.day    = binbcd( irq1_clk->date );					// Day (BCD)
	RTC_CLOCK.month  = binbcd( irq1_clk->mont );					// Month (BCD)
	year = (unsigned char)( irq1_clk->year - 2000 );				//00�`99�N�܂ł����ݒ�ł��Ȃ��̂�-2000�N
	RTC_CLOCK.year   = binbcd( year );								// Year (BCD)
	request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.sec;				// Write/Read����Buffer�ւ�Pointer

	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );			// 

	if( ret != RIIC_RSLT_OK ){
		// �����ŃG���[�o�^���͂��܂����ˁH
	}
	
	// �e�탌�W�X�^�̏������E���Z�b�g
	request.RequestCode    			=	I2C_RC_RTC_FLAG_SND;	//�t���O���W�X�^�������ݗv��
	RTC_CLOCK.flag_reg				=	RTC_FR_INIT;			//�t���O���W�X�^������
	RTC_CLOCK.ctrl_reg				=	(RTC_CR_INIT | RTC_CR_RESET);			//CTRL���W�X�^RESET
	request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.flag_reg;		
	request.I2cReqDataInfo.RWCnt	=	RTC_FR_CR_SIZE;			// Write/Read����Byte��(2)
	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );			// 

	if( ret != RIIC_RSLT_OK ){
		// �����ŃG���[�o�^���͂��܂����ˁH
	}

	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*|            timerd( void )                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Argument : struct clk_rec *                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Return : 0: OK / -1: NG                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Y.Nonaka                                                |*/
/*| Date        :  2000- 7-29                                              |*/
/*| Update      :  2001-11-27 Y.Takahashi A2H -> D2H                       |*/
/*[]------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/
unsigned char	timerd( struct clk_rec *irq1_clk )
{
	return (uchar)(RTC_ClkDataReadReq( RTC_READ_NON_QUE ));	// ���v�f�[�^�ǂݍ��ݗv��
}

/*[]----------------------------------------------------------------------[]*/
/*| �I�����C������̗v����M���̎��s�\��ԃ`�F�b�N					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : CheckOpeCondit( void )								   |*/
/*| RETURN VALUE : 0=OK�A1=NG											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : imai													   |*/
/*| Date		 : 2014-11-12											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
int CheckOpeCondit(void)
{
	if (OPECTL.Mnt_mod != 0) {
	// �����e�i���X��
		return 1;
	}
	if ((OPECTL.Ope_mod != 0)
	&&	(OPECTL.Ope_mod != 1)
	&&	(OPECTL.Ope_mod != 2)
	&&	(OPECTL.Ope_mod != 3)
	&&	(OPECTL.Ope_mod != 100)
	&&	(OPECTL.Ope_mod != 110)) {
	// �ҋ@or���Ԉʒu�ԍ�����or���Z��or���Z�����ł͂Ȃ�
	// �x��or�ޱ���װѕ\�������łȂ�
		return 1;
	}
	if (auto_syu_prn == 2) {
	// �ҋ@ && �����W�v���s��
		return 1;
	}
	if (DOWNLOADING()) {
	// ���u�_�E�����[�h��
		return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| �I�����C���ł̗����v�Z�v����M���̎Ԏ��`�F�b�N						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : CheckReqCalcData( void )								   |*/
/*| RETURN VALUE : 0=OK�A0!=NG											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : T.Nagai												   |*/
/*| Date		 : 2015-02-26											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
int CheckReqCalcData(void)
{
	int				i;
	int				j = 0;
	int				ret = 0;
	flp_com			*flp;
	RYO_INFO		*ryo_info;
	struct clk_rec	wk_CLK_REC;
	ushort			wk_CLK_REC_msec;
	ushort	sts_msk = 0xFFFF;
	extern	uchar	SetCarInfoSelect( short index );

	// �����v�Z�\��Ԃ��H
	if (CheckOpeCondit()) {
		// �����v�Z�s��
		ret = 1;
	}


	// �o�Ɏ��Ԏ擾
	c_Now_CLK_REC_ms_Read(&wk_CLK_REC, &wk_CLK_REC_msec);

	ryo_info = &RyoCalSim.RyoInfo[0];
	for (i = 0; i < OLD_LOCK_MAX; i++) {
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s

		if (FlpSetChk((ushort)i)) {
			// �L���ȎԎ����̂݃Z�b�g
			if( !SetCarInfoSelect(i) ){
				continue;
			}
			if( j >= OLD_LOCK_MAX ){
				break;
			}
			//  �����Z�b�g
			ryo_info[j].op_lokno = (ulong)((LockInfo[i].area * 10000L) + LockInfo[i].posi);	// �����
			ryo_info[j].pr_lokno = i+1;

			// ������ʃZ�b�g
			ryo_info[j].kind = LockInfo[i].ryo_syu;			// �������

			flp = &FLAPDT.flp_data[i];
			// �ԗ����� and (�t���b�v�㏸�� or �t���b�v�㏸���� or ���Ԓ�)
			if (flp->nstat.bits.b00 == 1 && (flp->mode == FLAP_CTRL_MODE2 ||
				flp->mode == FLAP_CTRL_MODE3 || flp->mode == FLAP_CTRL_MODE4)) {
				ryo_info[j].indate.year	= flp->year;		// ���ɓ���
				ryo_info[j].indate.mont	= flp->mont;
				ryo_info[j].indate.date	= flp->date;
				ryo_info[j].indate.hour	= flp->hour;
				ryo_info[j].indate.minu	= flp->minu;
				ryo_info[j].indate.week	= (uchar)youbiget((short)ryo_info[j].indate.year,	// �j��
													(short)ryo_info[j].indate.mont,
													(short)ryo_info[j].indate.date);
			}
			else {
				// ���Ɏ�����0�N���A����
				memset(&ryo_info[j].indate, 0, sizeof(ryo_info[j].indate));
			}

			// �o�Ɏ����Z�b�g
			memcpy(&ryo_info[j].outdate, &wk_CLK_REC, sizeof(wk_CLK_REC));

			// ���݃X�e�[�^�X
			ryo_info[j].nstat = flp->nstat.word & sts_msk;
			if (ret == 1) {
				// ���ԗ����s��(bit10)���Z�b�g
				ryo_info[j].nstat |= (1 << 10);
			}

			j++;
		}
	}


	// �����v�Z�s���
	if (ret == 1) {
		// �S�Ԏ����e�[�u���𑗐M
		NTNET_Snd_Data245(RyoCalSim.GroupNo);
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| �����v�Z����														   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : ryo_cal_sim( void )									   |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : T.Nagai												   |*/
/*| Date		 : 2015-02-26											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
static void	ryo_cal_sim(void)
{
	int			i;
	ulong		wk_Credit;		// �N���W�b�g���p�z
	uchar		wk_key_sec;		// �����͒l
	uchar		ryo_syu;		// �������(A=0�E�E�E)
	short		ans;
	ushort		wk_Pr_LokNo;	// ���������p���Ԉʒu�ԍ�(1�`324)
	ulong		wk_Op_LokNo;	// �ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)
	uchar		wk_InLagTime;	// ���O�^�C�������������s
	RYO_INFO	*ryo_info;
	flp_com		*flp;

	// �̎��ؔ��s�����p�ɁA���[�N�G���A��PayData�APPC_Data_Detail�A�N���W�b�g���p�z��ޔ�������
	memcpy(&SysMnt_Work[0], &PayData, sizeof(PayData));
	memcpy(&SysMnt_Work[sizeof(PayData)], &PPC_Data_Detail, sizeof(PPC_Data_Detail));
	wk_Credit = ryo_buf.credit.pay_ryo;
	// ���ݐ��Z���̏ꍇ�ɃZ�b�g�ς݂̃f�[�^��ޔ�������
	wk_Op_LokNo = OPECTL.Op_LokNo;							// �ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)
	wk_Pr_LokNo = OPECTL.Pr_LokNo;							// ���������p���Ԉʒu�ԍ�(1�`324)
	wk_InLagTime = OPECTL.InLagTimeMode;					// ���O�^�C�������������s

	if( CPrmSS[S_TYP][81] ){								// ���g�p�ݒ�L��?
		wk_key_sec = 1;										// ����[�`]�Ƃ���(���g�p����)
	}else{
		wk_key_sec = 0;										// �����͒l�ر(���g�p���Ȃ�)
	}

	// �L���Ԏ������[�v����
	i = 0;
	ryo_info = &RyoCalSim.RyoInfo[0];
	while (ryo_info[i].pr_lokno) {
		WACDOG;											// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s

		ans = carchk(wk_key_sec, ryo_info[i].op_lokno, 0);	// ���Ԉʒu�ԍ�����
		if ((ans == -1)										// ���Z�s�\
		||	(ans == 10)) {									// ���ݒ�
		// �Ԏ����G���[
			i++;
			continue;
		}

		ryo_syu = ryo_info[i].kind - 1;
		if (prm_get(COM_PRM, S_SHA, (short)(1+ryo_syu*6), 2, 5) == 0) {
		// �Ԏ����̎Ԏ햢�g�p�ݒ�
			i++;
			continue;
		}
		flp = &FLAPDT.flp_data[ryo_info[i].pr_lokno-1];
		// ���Z��or���Z�����H
		if (OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3) {
			// ���Z���̎Ԏ��̂ݒ��ԗ������Z�b�g����
			if (ryo_info[i].pr_lokno == wk_Pr_LokNo) {
				// �t���b�v��ԃ`�F�b�N
				switch (flp->mode) {
				case FLAP_CTRL_MODE1:						// �ҋ@���
					ryo_info[i].nstat |= (1 << 10);			// ���ԗ����s��(bit10)���Z�b�g
					// no break
				case FLAP_CTRL_MODE5:						// ���b�N���u�J�����҂�
				case FLAP_CTRL_MODE6:						// �o�ɑ҂���
					ryo_info[i].fee = 0;					// 0�~���Z�b�g
					break;
				default:
					// ���Z�����H
					if (OPECTL.Ope_mod == 2) {
						ryo_info[i].fee = ryo_buf.tyu_ryo;	// �v�Z�ς݂̒��ԗ������Z�b�g
					}
					else {
						ryo_info[i].fee = 0;				// 0�~���Z�b�g
						ryo_info[i].nstat |= (1 << 10);		// ���ԗ����s��(bit10)���Z�b�g
					}
					break;
				}
			}
			else {
				ryo_info[i].fee = 0;						// 0�~���Z�b�g
				ryo_info[i].nstat |= (1 << 10);				// ���ԗ����s��(bit10)���Z�b�g
			}
		}
		// �o�ɏ�Ԃł͂Ȃ� or ���~�ς�(�ԗ��Ȃ�)�ł͂Ȃ�
		else if (!chkdate2(ryo_info[i].indate.year, (short)ryo_info[i].indate.mont, (short)ryo_info[i].indate.date)) {
			// �����v�Z����
			cm27();												// �׸޸ر
			init_ryocalsim();									// �׸޸ر
			set_tim_ryocalsim(ryo_info[i].pr_lokno, &ryo_info[i].indate, &ryo_info[i].outdate);	// ���o�Ɏ����Z�b�g
			ryo_cal(110, ryo_info[i].pr_lokno);					// �����v�Z(�����v�Z�V�~�����[�^�p)
			ryo_info[i].fee = ryo_buf.dsp_ryo;					// �Z�o����
		}
		else {
			ryo_info[i].fee = 0;						// 0�~���Z�b�g
			ryo_info[i].nstat |= (1 << 10);				// ���ԗ����s��(bit10)���Z�b�g
		}

		i++;
		if (i >= OLD_LOCK_MAX) {
			break;
		}
	}

	// �ޔ�������PayData�APPC_Data_Detail�A�N���W�b�g���p�z��߂�
	memcpy(&PayData, &SysMnt_Work[0], sizeof(PayData));
	memcpy(&PPC_Data_Detail, &SysMnt_Work[sizeof(PayData)], sizeof(PPC_Data_Detail));
	ryo_buf.credit.pay_ryo = wk_Credit;
	// �ޔ������l��߂�
	OPECTL.Op_LokNo = wk_Op_LokNo;
	OPECTL.Pr_LokNo = wk_Pr_LokNo;
	OPECTL.InLagTimeMode = wk_InLagTime;
}

/*[]----------------------------------------------------------------------[]*/
/*| �����v�Z�t���O�N���A												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : init_ryocalsim( void )								   |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : imai													   |*/
/*| Date		 : 2014-10-10											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
static void	init_ryocalsim(void)
{
	Flap_Sub_Flg = 0;											// �ڍג��~�G���A�g�p�t���OOFF

	if (f_NTNET_RCV_MC10_EXEC) {								// mc10()�X�V���K�v�Ȑݒ肪�ʐM�ōX�V���ꂽ
		mc10();
		f_NTNET_RCV_MC10_EXEC = 0;
	}

	ryo_buf.credit.pay_ryo = 0;

	OPECTL.InLagTimeMode = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���o�Ɏ������														   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : set_tim_ryocalsim( num, *indate, *outdate )			   |*/
/*| PARAMETER	 : num	 	: �Ԏ���									   |*/
/*| PARAMETER	 : *indate	: ���ɗ\�����								   |*/
/*| PARAMETER	 : *outdate	: �o�ɗ\�����								   |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| car_in, car_ot, car_in_f, car_ot_f�ɓ��o�Ɏ������Z�b�g����             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : imai													   |*/
/*| Date		 : 2014-10-10											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
static void	set_tim_ryocalsim(ushort num, struct clk_rec *indate, struct clk_rec *outdate)
{
	memset(&car_in, 		0, sizeof(struct CAR_TIM));				// ���Ԏ����ر
	memset(&car_ot, 		0, sizeof(struct CAR_TIM));				// �o�Ԏ����ر
	memset(&car_in_f,		0, sizeof(struct CAR_TIM));				// ���Ԏ���Fix�ر
	memset(&car_ot_f,		0, sizeof(struct CAR_TIM));				// �o�Ԏ���Fix�ر
	memset(&recalc_carin,	0, sizeof(struct CAR_TIM));				// �Čv�Z�p���Ԏ����ر

	car_in.year = indate->year;										// ���� �N
	car_in.mon  = indate->mont;										//      ��
	car_in.day  = indate->date;										//      ��
	car_in.hour = indate->hour;										//      ��
	car_in.min  = indate->minu;										//      ��
	car_in.week = (char)youbiget( car_in.year,						//      �j��
								(short)car_in.mon,
								(short)car_in.day );
	car_ot.year = outdate->year;									// �o�� �N
	car_ot.mon  = outdate->mont;									//      ��
	car_ot.day  = outdate->date;									//      ��
	car_ot.hour = outdate->hour;									//      ��
	car_ot.min  = outdate->minu;									//      ��
	car_ot.week = (char)youbiget( car_ot.year,						//      �j��
								(short)car_ot.mon,
								(short)car_ot.day );

	memcpy(&car_in_f,		&car_in, sizeof(struct CAR_TIM));		// ���Ԏ���Fix
	memcpy(&car_ot_f,		&car_ot, sizeof(struct CAR_TIM));		// �o�Ԏ���Fix
	memcpy(&recalc_carin,	&car_in, sizeof(struct CAR_TIM));		// �Čv�Z�p���Ԏ���Fix
}
// MH322914(S) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
short	IsMagReaderRunning()
{
	short nRet = 0;
	
	if(( RD_mod == 12 )||( RD_mod == 13 )){							// ���[�_�[���쒆�H
		nRet = 1;
	}
	
	return nRet;
}
// MH322914(E) K.Onodera 2016/12/07 �T�[�r�X���g�p���ɓ���s����ƂȂ邱�Ƃ̑΍�[���ʃo�ONo.1341](GG103200)
// MH322914(S) K.Onodera 2016/09/21 AI-V�Ή�
//[]----------------------------------------------------------------------[]
///	@brief		OPE01�J�n���N���A����
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/09/21<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void op_mod01_Init( void )
{
	OPECTL.multi_lk = 0L;							// ������Z�⍇�����Ԏ���
	OPECTL.ChkPassSyu = 0;							// ������⍇��������ر
	OPECTL.ChkPassPkno = 0L;						// ������⍇�������ԏꇂ�ر
	OPECTL.ChkPassID = 0L;							// ������⍇���������ID�ر
	OPECTL.CalStartTrigger = 0;

// MH810100(S) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
	OPECTL.chg_idle_disp_rec = 0;
// MH810100(E) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
// MH810100(S) K.Onodera 2019/12/27 �Ԕԃ`�P�b�g���X�iID�Ή��j
	OPECTL.remote_wait_flg = 0;
	op_ClearSequenceID();
// MH810100(E) K.Onodera 2019/12/27 �Ԕԃ`�P�b�g���X�iID�Ή��j
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
	OPECTL.f_rtm_remote_pay_flg = 0;
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
// MH810100(S) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
	memset( &g_UseMediaData, 0, sizeof( t_UseMedia ) );
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// 	memset( DiscountBackUpArea, 0, sizeof(DiscountBackUpArea) );
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// MH810100(E) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.51)�iGM803000���p�j
// GG129000(S) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
//	OPECTL.f_searchtype = 0xff;						// 0�ɈӖ�������̂�0xff�ŏ�����
	if(OPECTL.f_req_paystop == 0){
		OPECTL.f_searchtype = 0xff;						// 0�ɈӖ�������̂�0xff�ŏ�����
	}
// GG129000(E) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j(���P�A��No.51)�iGM803000���p�j
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
	GyouCnt_All_r = 0;
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	GyouCnt_All_j = 0;
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
}
// MH322914(E) K.Onodera 2016/09/21 AI-V�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F�U�֐��Z
////[]----------------------------------------------------------------------[]
/////	@brief		�U�֐旿���v�Z
////[]----------------------------------------------------------------------[]
/////	@param		none
/////	@return		0=OK/!0=NG
////[]----------------------------------------------------------------------[]
/////	@date		Create	: 2016/08/25<br>
/////				Update	: 
////[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
//static int Ope_PipFurikaeCalc( void )
//{
//	ushort			posi;
//	flp_com			*flp;
//	RYO_INFO		*ryo_info;
//	struct clk_rec	wk_CLK_REC;
//	ushort			wk_CLK_REC_msec;
//	ushort	sts_msk = 0xFFFF;
//	extern	uchar	SetCarInfoSelect( short index );
//
//	// ���[�N�֏o�Ɏ��Ԏ擾
//	c_Now_CLK_REC_ms_Read( &wk_CLK_REC, &wk_CLK_REC_msec );
//
//	ryo_info = &RyoCalSim.RyoInfo[0];
//
//	// �Y���Ԏ�����H
//	if( LKopeGetLockNum( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, &posi ) ){
//
//		// ���݃X�e�[�^�X
//		flp = &FLAPDT.flp_data[posi-1];
//		ryo_info[0].nstat = flp->nstat.word & sts_msk;
//		g_PipCtrl.stFurikaeInfo.DestStatus = ryo_info[0].nstat;
//
//		//  �����Z�b�g
//		ryo_info[0].op_lokno = (ulong)( (g_PipCtrl.stFurikaeInfo.DestArea * 10000L) + g_PipCtrl.stFurikaeInfo.DestNo );
//		ryo_info[0].pr_lokno = posi;
//
//		// ������ʃZ�b�g
//		ryo_info[0].kind = LockInfo[posi-1].ryo_syu;
//		g_PipCtrl.stFurikaeInfo.DestFeeKind = ryo_info[0].kind;
//
//		// ���Ɏ����Z�b�g
//		ryo_info[0].indate.year = flp->year;										// ���� �N
//		ryo_info[0].indate.mont = flp->mont;										//      ��
//		ryo_info[0].indate.date = flp->date;										//      ��
//		ryo_info[0].indate.hour = flp->hour;										//      ��
//		ryo_info[0].indate.minu = flp->minu;										//      ��
//		ryo_info[0].indate.week = (char)youbiget( ryo_info[0].indate.year,			//      �j��
//									(short)ryo_info[0].indate.mont,
//									(short)ryo_info[0].indate.date );
//
//		// �����p�f�[�^�ɂ��Z�b�g
//		g_PipCtrl.stFurikaeInfo.DestInTime.Year = ryo_info[0].indate.year;
//        g_PipCtrl.stFurikaeInfo.DestInTime.Mon	= ryo_info[0].indate.mont;
//		g_PipCtrl.stFurikaeInfo.DestInTime.Day	= ryo_info[0].indate.date;
//		g_PipCtrl.stFurikaeInfo.DestInTime.Hour	= ryo_info[0].indate.hour;
//		g_PipCtrl.stFurikaeInfo.DestInTime.Min	= ryo_info[0].indate.minu;
//		g_PipCtrl.stFurikaeInfo.DestInTime.Sec	= ryo_info[0].indate.seco;
//
//		// �o�Ɏ����Z�b�g
//		memcpy( &ryo_info[0].outdate, &wk_CLK_REC, sizeof(wk_CLK_REC) );
//		// �����p�f�[�^�ɂ��Z�b�g
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Year	= ryo_info[0].outdate.year;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Mon		= ryo_info[0].outdate.mont;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Day		= ryo_info[0].outdate.date;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Hour	= ryo_info[0].outdate.hour;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Min		= ryo_info[0].outdate.minu;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Sec		= ryo_info[0].outdate.seco;
//
//	}else{
//		return PIP_RES_RESULT_NG_NO_CAR;
//	}
//
//	set_tim_ryocalsim( ryo_info[0].pr_lokno, &ryo_info[0].indate, &ryo_info[0].outdate );	// ���o�Ɏ����Z�b�g
//	ryo_cal( 111, ryo_info[0].pr_lokno );				// �����v�Z(�����v�Z�V�~�����[�^�p)
//	ryo_info[0].fee = ryo_buf.dsp_ryo;					// �Z�o����
//	g_PipCtrl.stFurikaeInfo.DestFee = ryo_info[0].fee;
//	g_PipCtrl.stFurikaeInfo.DestFeeDiscount = ryo_buf.waribik;
//
//	return PIP_RES_RESULT_OK;
//}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_PiP_GetFurikaeGaku
 *[]----------------------------------------------------------------------[]
 *| summary	: �U�֋��z�v�Z
 *| return	: void
 *| NOTE	: �U�֐��Z���f�[�^�|�C���^
 *[]----------------------------------------------------------------------[]*/
void	ope_PiP_GetFurikaeGaku( PIP_FURIKAE_INFO *st )
{
	uchar	wari_tim;

	wari_tim = (uchar)prm_get(COM_PRM, S_CEN, 32, 1, 1);

	vl_frs.zangaku = ryo_buf.zankin;
	// �U�֌��ƐU�֐�̗�����ʂ����� or ������ʂ��قȂ��Ă��T�[�r�X��/�|���茔�U�ւ���H
	if( st->SrcFeeKind == (char)(ryo_buf.syubet+1) ||	wari_tim == 1 ){
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//		st->DestFeeDiscount = vl_frs.price;
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
		st->Remain = ryo_buf.zankin - vl_frs.price;
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//		vl_frs.furikaegaku = vl_frs.price;
		vl_frs.wari_furikaegaku = ( vl_frs.price - (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku) );
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
	}else{
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//		st->DestFeeDiscount = vl_frs.in_price;
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
		st->Remain = ryo_buf.zankin - vl_frs.in_price;
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//		vl_frs.furikaegaku = vl_frs.in_price;
		vl_frs.wari_furikaegaku = ( vl_frs.in_price - (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku) );
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
	}
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
//	// furikaegaku�́A�U�։\�Ȋz�̂�(�ނ�Ƃ��镪�͊܂܂Ȃ�)
//	if( vl_frs.furikaegaku > vl_frs.zangaku ){
//		vl_frs.furikaegaku = vl_frs.zangaku;
//	}
	// �U�֊������c�z�ȏ�H���U�֊����͎��ۂɓK�p�����z�̂���
	if( vl_frs.wari_furikaegaku > vl_frs.zangaku ){
		vl_frs.wari_furikaegaku = vl_frs.zangaku;
	}
	// �U�֊z = �U�֊z(�����A�����A�J�[�h���ς̍��Z)
	vl_frs.furikaegaku = (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku + vl_frs.wari_furikaegaku);
	if( vl_frs.furikaegaku > vl_frs.zangaku ){
		vl_frs.kabarai = (vl_frs.furikaegaku - vl_frs.zangaku);
	}
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
}
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F�U�֐��Z
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)
//[]----------------------------------------------------------------------[]
///	@brief		���u���Z�p�����v�Z
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		0=OK/!0=NG
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static int Ope_PipRemoteCalcTimePre( void )
{
	ushort			posi;
	flp_com			*flp;
	RYO_INFO		*ryo_info;
	struct clk_rec	wk_CLK_REC;
	ushort			wk_CLK_REC_msec;
	ushort	sts_msk = 0xFFFF;
	extern	uchar	SetCarInfoSelect( short index );

	// ���[�N�֏o�Ɏ��Ԏ擾
	c_Now_CLK_REC_ms_Read( &wk_CLK_REC, &wk_CLK_REC_msec );
	// �����v�Z�����ێ�
	g_PipCtrl.stRemoteTime.OutTime.Year	= wk_CLK_REC.year;		// Year
	g_PipCtrl.stRemoteTime.OutTime.Mon	= wk_CLK_REC.mont;		// Month
	g_PipCtrl.stRemoteTime.OutTime.Day	= wk_CLK_REC.date;		// Day
	g_PipCtrl.stRemoteTime.OutTime.Hour	= wk_CLK_REC.hour;		// Hour
	g_PipCtrl.stRemoteTime.OutTime.Min	= wk_CLK_REC.minu;		// Minute
	g_PipCtrl.stRemoteTime.OutTime.Sec	= wk_CLK_REC.seco;		// Second

	ryo_info = &RyoCalSim.RyoInfo[0];

	// �Y���Ԏ�����H
	if( LKopeGetLockNum( g_PipCtrl.stRemoteTime.Area, g_PipCtrl.stRemoteTime.ulNo, &posi ) ){
		//  �����Z�b�g
		ryo_info[0].op_lokno = (ulong)( (g_PipCtrl.stRemoteTime.Area * 10000L) + g_PipCtrl.stRemoteTime.ulNo );
		ryo_info[0].pr_lokno = posi;

		// ������ʃZ�b�g
		ryo_info[0].kind = LockInfo[posi-1].ryo_syu;			// �������
		g_PipCtrl.stRemoteTime.RyoSyu = ryo_info[0].kind;

		// ���ɓ���
		ryo_info[0].indate.year	= g_PipCtrl.stRemoteTime.InTime.Year;
		ryo_info[0].indate.mont	= g_PipCtrl.stRemoteTime.InTime.Mon;
		ryo_info[0].indate.date	= g_PipCtrl.stRemoteTime.InTime.Day;
		ryo_info[0].indate.hour	= g_PipCtrl.stRemoteTime.InTime.Hour;
		ryo_info[0].indate.minu	= g_PipCtrl.stRemoteTime.InTime.Min;
		ryo_info[0].indate.week	= (uchar)youbiget((short)ryo_info[0].indate.year,	// �j��
												  (short)ryo_info[0].indate.mont,
												  (short)ryo_info[0].indate.date);

		// �o�Ɏ����Z�b�g
		memcpy( &ryo_info[0].outdate, &wk_CLK_REC, sizeof(wk_CLK_REC) );
	}else{
// MH810100(S) 2020/06/11 #4222�y�A���]���w�E�����z���u���Z�v���f�[�^�u�v����ʁF1=�����v�Z�v��M���ɉ��u���Z���Ɏ����w�萸�Z�����f�[�^�̌��ʂ��u13=���s�i�w��Ԏ������j�v�ƂȂ��Ă��܂�
//		return PIP_RES_RESULT_NG_NO_CAR;

		syashu = g_PipCtrl.stRemoteTime.RyoSyu;				// �������

		ryo_info[0].pr_lokno = 1;							// 1�Œ�

		// ���ɓ���
		ryo_info[0].indate.year	= g_PipCtrl.stRemoteTime.InTime.Year;
		ryo_info[0].indate.mont	= g_PipCtrl.stRemoteTime.InTime.Mon;
		ryo_info[0].indate.date	= g_PipCtrl.stRemoteTime.InTime.Day;
		ryo_info[0].indate.hour	= g_PipCtrl.stRemoteTime.InTime.Hour;
		ryo_info[0].indate.minu	= g_PipCtrl.stRemoteTime.InTime.Min;
		ryo_info[0].indate.week	= (uchar)youbiget((short)ryo_info[0].indate.year,	// �j��
												  (short)ryo_info[0].indate.mont,
												  (short)ryo_info[0].indate.date);

		// �o�Ɏ����Z�b�g
		memcpy( &ryo_info[0].outdate, &wk_CLK_REC, sizeof(wk_CLK_REC) );
// MH810100(E) 2020/06/11 #4222�y�A���]���w�E�����z���u���Z�v���f�[�^�u�v����ʁF1=�����v�Z�v��M���ɉ��u���Z���Ɏ����w�萸�Z�����f�[�^�̌��ʂ��u13=���s�i�w��Ԏ������j�v�ƂȂ��Ă��܂�
	}

	// ���݃X�e�[�^�X
	flp = &FLAPDT.flp_data[0];
	ryo_info[0].nstat = flp->nstat.word & sts_msk;

	set_tim_ryocalsim( ryo_info[0].pr_lokno, &ryo_info[0].indate, &ryo_info[0].outdate );	// ���o�Ɏ����Z�b�g
	ryo_cal( 111, ryo_info[0].pr_lokno );				// �����v�Z(�����v�Z�V�~�����[�^�p)
	ryo_info[0].fee = ryo_buf.dsp_ryo;					// �Z�o����
	// �����z���Z�o�����𒴂���ꍇ�A�����z�͗����ɍ��킹��
	if( ryo_buf.dsp_ryo < g_PipCtrl.stRemoteTime.Discount ){
		g_PipCtrl.stRemoteTime.Discount = ryo_buf.dsp_ryo;
	}
	g_PipCtrl.stRemoteTime.Price = ryo_info[0].fee;

	return PIP_RES_RESULT_OK;
}
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
/*[]----------------------------------------------------------------------[]*/
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
///*| ���Z�̈�ւ��ް��̊i�[ (���σ��[�_�ŕ�����)              		       |*/
/*| ���Z�̈�ւ��ް��̊i�[ (���σ��[�_�Œ��O����f�[�^��M)              		       |*/
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcElectronSet_DeemedData					               |*/
/*| PARAMETER    : *buf		: ����ް�				                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Inaba                                                   |*/
/*| Date         : 2019-02-06                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	EcElectronSet_DeemedData( void *buf )
{
	EC_SETTLEMENT_RES	*p;
	p = (EC_SETTLEMENT_RES*)buf;
	
	//�W�v�E�̎��� �p (�N���W�b�g) ������
	memset( &EcRecvDeemedData, 0, sizeof(Receipt_data) );

	// �����σf�[�^��ID��t�^
	EcRecvDeemedData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		EcRecvDeemedData.ID = 56;
	}

	// �݂Ȃ����ό��ʃf�[�^��M�t���O
	EcRecvDeemedData.Electron_data.Ec.E_Status.BIT.deemed_receive = 1;
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//	// ���ώ������Z�b�g
//	EcRecvDeemedData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);						// ���ϓ���(Normalize)
//	// �o�Ɏ��������σ��[�_���瑗���Ă����������Z�b�g
//	memcpy(&EcRecvDeemedData.TOutTime, &p->settlement_time, sizeof(date_time_rec));
	// ���ώ������Z�b�g
	if( datetimecheck(&p->settlement_time) == 0){
		// �s�𗝃`�F�b�N��NG�ł���Ό��ώ������Z�b�g���Ȃ�
		EcRecvDeemedData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);						// ���ϓ���(Normalize)
	}
	// ���ό��ʂ��Z�b�g
	EcRecvDeemedData.EcResult = (uchar)p->Result;
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

	// �ڼޯ��ް����
	if( p->brand_no == BRANDNO_CREDIT ){
		memcpy( &EcRecvDeemedData.credit.card_name[0], &p->Brand.Credit.Credit_Company[0], sizeof(EcRecvDeemedData.credit.card_name) );	// �ڼޯĶ��މ�З���
		memcpy( &EcRecvDeemedData.credit.card_no[0], &p->Card_ID[0], sizeof(EcRecvDeemedData.credit.card_no) );							// �ڼޯĶ��މ���ԍ��i�E�l�߁j
		EcRecvDeemedData.credit.slip_no				= (ulong)p->Brand.Credit.Slip_No;													// �ڼޯĶ��ޓ`�[�ԍ�
		EcRecvDeemedData.credit.pay_ryo				= (ulong)p->settlement_data;														// �N���W�b�g���ϊz
		memset( &EcRecvDeemedData.credit.ShopAccountNo[0], 0x20, sizeof(EcRecvDeemedData.credit.ShopAccountNo) );						// �����X����ԍ�, ���σ��[�_�ł͖���

		EcRecvDeemedData.credit.app_no = astoinl(&p->Brand.Credit.Approval_No[0], sizeof(p->Brand.Credit.Approval_No));					// ���F�ԍ�
		memcpy( &EcRecvDeemedData.credit.CCT_Num[0], &p->Brand.Credit.Id_No[0], sizeof(p->Brand.Credit.Id_No) );						// �[�����ʔԍ�
		memcpy( &EcRecvDeemedData.credit.kid_code[0], &p->Brand.Credit.KID_Code[0], sizeof(EcRecvDeemedData.credit.kid_code) );			// KID �R�[�h

		EcRecvDeemedData.Electron_data.Ec.e_pay_kind	= EC_CREDIT_USED;																// ���Z���
		memcpy(EcRecvDeemedData.Electron_data.Ec.Card_ID, p->Card_ID, sizeof(EcRecvDeemedData.Electron_data.Ec.Card_ID));				// ����ID
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	else if( p->brand_no == BRANDNO_QR ){
		EcRecvDeemedData.Electron_data.Ec.pay_ryo	= p->settlement_data;											// ���ϋ��z
		EcRecvDeemedData.Electron_data.Ec.pay_after	= p->settlement_data_after;										// �c��

		EcRecvDeemedData.Electron_data.Ec.Brand.Qr.PayKind = p->Brand.Qr.PayKind;									// �x�����
		EcRecvDeemedData.Electron_data.Ec.e_pay_kind	= EC_QR_USED;												// ���Z���

		memcpy(EcRecvDeemedData.Electron_data.Ec.QR_Kamei, p->QR_Kamei,
				sizeof(EcRecvDeemedData.Electron_data.Ec.QR_Kamei));												// �����X��
		memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Qr.PayTerminalNo, p->Brand.Qr.PayTerminalNo,
				sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Qr.PayTerminalNo));									// �x���[��ID
		memcpy(EcRecvDeemedData.Electron_data.Ec.inquiry_num, p->inquiry_num,
				sizeof(EcRecvDeemedData.Electron_data.Ec.inquiry_num));												// ����ԍ�
		memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Qr.MchTradeNo, p->Brand.Qr.MchTradeNo,
				sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Qr.MchTradeNo));										// Mch����ԍ�
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	// �d�q�Ȱ�ް����
	else {
		EcRecvDeemedData.Electron_data.Ec.pay_ryo	= p->settlement_data;											// ���ϋ��z
		EcRecvDeemedData.Electron_data.Ec.pay_befor	= p->settlement_data_before;									// ���ϑO�c��
		EcRecvDeemedData.Electron_data.Ec.pay_after	= p->settlement_data_after;										// ���ό�c��
		memcpy(EcRecvDeemedData.Electron_data.Ec.Card_ID, p->Card_ID, sizeof(EcRecvDeemedData.Electron_data.Ec.Card_ID));		// ����ID
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		memcpy(EcRecvDeemedData.Electron_data.Ec.inquiry_num, p->inquiry_num, sizeof(EcRecvDeemedData.Electron_data.Ec.inquiry_num)); //�₢���킹�ԍ�
// MH810105(S) MH364301 �s�v�ϐ��폜
//		memcpy(EcRecvDeemedData.Electron_data.Ec.Termserial_No, p->Termserial_No, sizeof(EcRecvDeemedData.Electron_data.Ec.Termserial_No)); //�[�����ʔԍ�
// MH810105(E) MH364301 �s�v�ϐ��폜
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��

		switch (p->brand_no) {
		case	BRANDNO_KOUTSUU:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_KOUTSUU_USED;						// ���Z���
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.SPRW_ID, p->Brand.Koutsuu.SPRW_ID,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.SPRW_ID));								// SPRWID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.Kamei, p->Brand.Koutsuu.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.Kamei));									// �����X��
			EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.TradeKind = p->Brand.Koutsuu.TradeKind;					// ������
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
			break;
		case	BRANDNO_EDY:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_EDY_USED;							// ���Z���
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//			EcRecvDeemedData.Electron_data.Ec.Brand.Edy.DealNo			= p->Brand.Edy.DealNo;						// ����ʔ�
//			EcRecvDeemedData.Electron_data.Ec.Brand.Edy.CardDealNo		= p->Brand.Edy.CardDealNo;					// �J�[�h����ʔ�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.DealNo, &p->Brand.Edy.DealNo, 10);					// Edy����ʔ�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.CardDealNo, &p->Brand.Edy.CardDealNo, 5);			// �J�[�h����ʔ�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.Kamei, p->Brand.Edy.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.Kamei));										// �����X��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.TerminalNo, &p->Brand.Edy.TerminalNo, 8);				// �[���ԍ�
			break;
		case	BRANDNO_NANACO:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_NANACO_USED;						// ���Z���
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//			EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.DealNo		= p->Brand.Nanaco.DealNo;					// ��ʎ���ʔ�
//			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.TerminalNo, &p->Brand.Nanaco.TerminalNo, 12);		// ��ʒ[��ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.Kamei, p->Brand.Nanaco.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.Kamei));									// �����X��
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.DealNo, &p->Brand.Nanaco.DealNo, 6);				// �[������ʔ�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.TerminalNo, &p->Brand.Nanaco.TerminalNo, 20);		// ��ʒ[��ID
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
			break;
		case	BRANDNO_WAON:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_WAON_USED;							// ���Z���
			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point_status	= p->Brand.Waon.PointStatus;				// ���p��
			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point			= p->Brand.Waon.GetPoint;					// ����|�C���g
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point_total		= p->Brand.Waon.TotalPoint;				// �݌v�|�C���g
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.period_point	= p->Brand.Waon.PeriodPoint;				// �Q�N�O�܂łɊl�������|�C���g
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.period			= p->Brand.Waon.Period;						// �Q�N�O�܂łɊl�������|�C���g�̗L������
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.card_type		= p->Brand.Waon.CardType;					// �J�[�h����
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.deal_code		= p->Brand.Waon.DealCode;					// �����ʃR�[�h
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point_total, p->Brand.Waon.TotalPoint,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point_total));								// �݌v�|�C���g
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.Kamei, p->Brand.Waon.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.Kamei));									// �����X��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.SPRW_ID, &p->Brand.Waon.SPRW_ID, sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.SPRW_ID));	// SPRWID
			break;
		case	BRANDNO_SAPICA:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_SAPICA_USED;						// ���Z���
			EcRecvDeemedData.Electron_data.Ec.Brand.Sapica.Details_ID	= p->Brand.Sapica.Details_ID;				// ����ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Sapica.Terminal_ID, &p->Brand.Sapica.Terminal_ID, sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Sapica.Terminal_ID));	// ���̒[��ID
			break;
		case	BRANDNO_ID:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_ID_USED;							// ���Z���
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//			PayData.Electron_data.Ec.Brand.Id.Details_ID
//			PayData.Electron_data.Ec.Brand.Id.Terminal_ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Id.Kamei, p->Brand.Id.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Id.Kamei));										// �����X��
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Id.Approval_No, &p->Brand.Id.Approval_No, 7);			// ���F�ԍ�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Id.TerminalNo, &p->Brand.Id.TerminalNo, 13);				// ��ʒ[��ID
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
			break;
		case	BRANDNO_QUIC_PAY:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_QUIC_PAY_USED;						// ���Z���
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//			PayData.Electron_data.Ec.Brand.Id.Details_ID
//			PayData.Electron_data.Ec.Brand.Id.Terminal_ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Quickpay.Kamei, p->Brand.QuicPay.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Quickpay.Kamei));								// �����X��
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Quickpay.Approval_No, &p->Brand.QuicPay.Approval_No, 7);	// ���F�ԍ�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Quickpay.TerminalNo, &p->Brand.QuicPay.TerminalNo, 13);	// ��ʒ[��ID
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
			break;
// MH810105(S) MH364301 PiTaPa�Ή�
		case	BRANDNO_PITAPA:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_PITAPA_USED;						// ���Z���
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.Kamei, p->Brand.Pitapa.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.Kamei));									// �����X��
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.Slip_No, &p->Brand.Pitapa.Slip_No, 5);			// �`�[�ԍ�
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.TerminalNo, &p->Brand.Pitapa.TerminalNo, 13);		// ��ʒ[��ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.Approval_No, &p->Brand.Pitapa.Approval_No, 8);	// ���F�ԍ�
			break;
// MH810105(E) MH364301 PiTaPa�Ή�
		default:	// �s���u�����h
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_UNKNOWN_USED;
			break;
		}
	}
}

#if (6 == AUTO_PAYMENT_PROGRAM)
ushort	auto_payment_locksts_set( void )
{
	ulong	work_data_nml;
	short	car_index_bk = car_index;
	short	i;
	short	end = INT_CAR_START_INDEX + INT_CAR_LOCK_MAX;
	date_time_rec work_date;
	memset(&work_date, 0, sizeof(work_date));

	// ���Ɏ������Z�o
	work_date.Year = (ushort)CLK_REC.year;
	work_date.Mon = (uchar)CLK_REC.mont;
	work_date.Day = (uchar)CLK_REC.date;
	work_date.Hour = (uchar)CLK_REC.hour;
	work_date.Min = (uchar)CLK_REC.minu;
	work_data_nml = Nrm_YMDHM(&work_date);
	work_data_nml -= 0x10000;
	UnNrm_YMDHM(&work_date, work_data_nml);

	// �Ԏ���Ԃ��Z�b�g
	FLAPDT.flp_data[car_index].mode = 3;					// ����
	FLAPDT.flp_data[car_index].year = work_date.Year;		// �N
	FLAPDT.flp_data[car_index].mont = work_date.Mon;		// ��
	FLAPDT.flp_data[car_index].date = work_date.Day;		// ��
	FLAPDT.flp_data[car_index].hour = work_date.Hour;		// ��
	FLAPDT.flp_data[car_index].minu = work_date.Min;		// ��
	FLAPDT.flp_data[car_index].nstat.bits.b00 = 1;			// �ԗ�����
	FLAPDT.flp_data[car_index].nstat.bits.b01 = 1;			// �㏸�ς�
	FLAPDT.flp_data[car_index].nstat.bits.b02 = 1;			// �㏸����

	// ���̎Ԏ��̃C���f�b�N�X���Z�o
	for(i = car_index+1; i < end; i++) {
		WACDOG;
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			car_index = i;
			break;
		}
	}
	if(car_index == car_index_bk ) {
		car_index = INT_CAR_START_INDEX;
	}
	return (ushort)LockInfo[car_index_bk].posi;
}
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

// MH810103 GG119202(S) �d�q�}�l�[�V���O���ݒ�ňē��������s��
//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_�̈ē���������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/04/26
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void op_EcEmoney_Anm(void)
{
	if (!isEC_USE() ||
		OPECTL.op_faz >= 2) {
		return;
	}

	if (check_enable_multisettle() == 1) {
		// �d�q�}�l�[�V���O���ݒ�̏ꍇ�͈ē��������s��
		if (isEcEmoneyEnabled(1, 0) &&
			ECCTL.anm_ec_emoney != 0xFF) {
			ope_anm(AVM_SELECT_EMONEY);			// �u������܂ł�������^�b�`���Ă��������v
			ECCTL.anm_ec_emoney = 0xFF;			// �����ς�
// MH810103(s) �d�q�}�l�[�Ή� #5555�yWAON�V���O���Ή��z���Z�J�n�Łu������܂ł�������^�b�`���ĉ������v���ē���������
			lcdbm_notice_ope(LCDBM_OPCD_EMONEY_SINGLE,0);
// MH810103(e) �d�q�}�l�[�Ή� #5555�yWAON�V���O���Ή��z���Z�J�n�Łu������܂ł�������^�b�`���ĉ������v���ē���������
		}
		else if (!SUICA_USE_ERR && Suica_Rec.Data.BIT.CTRL == 0 &&
				 ECCTL.anm_ec_emoney != 0xFF) {
			// ��t�s��Ԃ̏ꍇ�͎�t��M���Ɉē���������
			ECCTL.anm_ec_emoney = 1;			// ��t�҂�
		}
	}
}
// MH810103 GG119202(E) �d�q�}�l�[�V���O���ݒ�ňē��������s��
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//// MH810100(S) �e�X�g�p���Ƀ��O�쐬
//#ifdef TEST_ENTER_LOG
//void Test_Ope_EnterLog(void)
//{
//	FLAPDT.flp_data[1].year = CLK_REC.year;
//	FLAPDT.flp_data[1].mont = CLK_REC.mont;
//	FLAPDT.flp_data[1].date = CLK_REC.date;
//	FLAPDT.flp_data[1].hour = CLK_REC.hour;
//	FLAPDT.flp_data[1].minu = CLK_REC.minu;
//	Make_Log_Enter(1);
//	Log_regist(LOG_ENTER);
//	taskchg(IDLETSKNO);
//}
//#endif
//// MH810100(E) �e�X�g�p���Ƀ��O�쐬
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)

// MH810100(S) K.Onodera 2019/11/18 �Ԕԃ`�P�b�g���X�i�����e�i���X�I�������j
//[]----------------------------------------------------------------------[]
///	@brief			�����e�i���X�J�n�����҂�
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			MOD_EXT / MOD_CHG
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort op_wait_mnt_start( void )
{
	ushort	msg = 0;
	ushort	res_wait_time = prm_get(COM_PRM, S_PKT, 21, 2, 1);

	if( !res_wait_time ){
		res_wait_time = 5;	// �͈͊O��5�b
	}

	// ����ʒm(�����e�i���X�J�n�v��)���M(��LCD)
	lcdbm_notice_ope( LCDBM_OPCD_MNT_STA_REQ, 0 );
	// �����e�i���X�I�������҂��^�C�}�[�J�n
	Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );

	while(1){
		msg = GetMessage();
		switch( msg ){

			case KEY_MODECHG:			// Mode change key
				if( !OPECTL.on_off ){	// key OFF
					// �����e�i���X�J�n�����҂��^�C�}�[��~
					Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );
					return (MOD_EXT);	// OPE�p��
				}
				break;

			case LCD_DISCONNECT:
				OPECTL.Ope_mod = 255;							// ��������Ԃ�
				OPECTL.init_sts = 0;							// ��������������ԂƂ���
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
				OPECTL.PasswordLevel = (char)-1;
				return MOD_CUT;
				break;

			// �����e�i���X�J�n�����҂��^�C���A�E�g
			case TIMEOUT_MNT_RESTART:
				 // �h�A�J�H
				if( !CP_MODECHG ){
					// ����ʒm(�����e�i���X�J�n�v��)���M(��LCD)
					lcdbm_notice_ope( LCDBM_OPCD_MNT_STA_REQ, 0 );
					// �����e�i���X�J�n�����҂��^�C�}�[�ĊJ
					Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				}
				break;

			// ����ʒm
			case LCD_OPERATION_NOTICE:
				// �����e�i���X�J�n����
				if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_MNT_STA_RES ){
					 // �h�A�J�H
					if( !CP_MODECHG ){
						// 0=OK�H
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							OPECTL.Ope_mod = 0;								// ���ڰ���Ӱ�޸ر
							OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
							OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
							OPECTL.PasswordLevel = (char)-1;
							return (MOD_CHG);	// �����e�i���X�I��
						}
						// 1=NG�H
						else{
							// ���g���C�I�[�o�[���݂���ƃt�F�[�Y���ꂪ��������̂Ŗ������g���C
							// ����ʒm(�����e�i���X�J�n�ʒm)�đ��M(��LCD)
							lcdbm_notice_ope( LCDBM_OPCD_MNT_STA_REQ, 0 );
							// �����e�i���X�J�n�����҂��^�C�}�[�ĊJ
							Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
						}
					}
				}
				// ���Z�J�n�v��
				else if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_PAY_STA ){
					// ����ʒm(���Z�J�n����(OK))���M
					lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );
					// ����ݽ�J�n�đ���ϒ�~
					Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );
					// op_mod01(�Ԕԓ���) = ���ޏ��҂�
					OPECTL.Ope_mod = 1;
					return (MOD_EXT);	// OPE�p��
				}
				break;

			// ���ɏ��(���ޏ��)
			case LCD_IN_CAR_INFO:
				// TODO_Onodera�F�K�v�H�H
				break;

			// ���u���Z���h�A�J�Ȃ���Ȃ��̂�OK
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�����e�i���X�I������
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			MOD_EXT / MOD_CHG
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort op_wait_mnt_close( void )
{
	ushort	msg = 0;
	ushort	res_wait_time = prm_get(COM_PRM, S_PKT, 21, 2, 1);

	if( !res_wait_time ){
		res_wait_time = 5;	// �͈͊O��5�b
	}

	// ����ʒm(�����e�i���X�I���ʒm)���M(��LCD)
	lcdbm_notice_ope( LCDBM_OPCD_MNT_END_REQ, lcdbm_alarm_check() );
	// �����e�i���X�I�������҂��^�C�}�[�J�n
	Lagtim( OPETCBNO, TIMERNO_MNT_RESTOP, (res_wait_time * 50) );

	while(1){
		msg = GetMessage();
		switch( msg ){

			case KEY_MODECHG:			// Mode change key
				if( OPECTL.on_off ){	// key ON
					// �����e�i���X�I�������҂��^�C�}�[��~
					Lagcan( OPETCBNO, TIMERNO_MNT_RESTOP );
					return (MOD_EXT);	// �����e�i���X�p��
				}
				break;

			case LCD_DISCONNECT:
				OPECTL.Ope_mod = 255;							// ��������Ԃ�
				OPECTL.init_sts = 0;							// ��������������ԂƂ���
				OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
				OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
				OPECTL.PasswordLevel = (char)-1;
				return MOD_CUT;
				break;

			// �����e�i���X�I�������҂��^�C���A�E�g
			case TIMEOUT_MNT_RESTOP:
				 // �h�A�H
				if( CP_MODECHG ){
					// ����ʒm(�����e�i���X�I���ʒm)�đ��M(��LCD)
					lcdbm_notice_ope( LCDBM_OPCD_MNT_END_REQ, lcdbm_alarm_check() );
					// �����e�i���X�I�������҂��^�C�}�[�ĊJ
					Lagtim( OPETCBNO, TIMERNO_MNT_RESTOP, (res_wait_time * 50) );
				}
				break;

			// ����ʒm
			case LCD_OPERATION_NOTICE:
				// �����e�i���X�I������
				if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_MNT_END_RES ){
					 // �h�A�H
					if( CP_MODECHG ){
						// 0=OK�H
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
							OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
							OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
							OPECTL.PasswordLevel = (char)-1;
							return (MOD_CHG);	// �����e�i���X�I��
						}
						// 1=NG�H
						else{
							// ���g���C�I�[�o�[���݂���ƃt�F�[�Y���ꂪ��������̂Ŗ������g���C
							// ����ʒm(�����e�i���X�I���ʒm)�đ��M(��LCD)
							lcdbm_notice_ope( LCDBM_OPCD_MNT_END_REQ, lcdbm_alarm_check() );
							// �����e�i���X�I�������҂��^�C�}�[�ĊJ
							Lagtim( OPETCBNO, TIMERNO_MNT_RESTOP, (res_wait_time * 50) );
						}
					}
				}
				break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)�Z�b�g
//[]----------------------------------------------------------------------[]
///	@param			id�F���ɂ��琸�Z�����܂ł��Ǘ�����ID
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void op_SetSequenceID( ulong id )
{
	g_OpeSequenceID = id;
}

//[]----------------------------------------------------------------------[]
///	@brief			ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)�Q�b�g
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			id�F���ɂ��琸�Z�����܂ł��Ǘ�����ID
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ulong op_GetSequenceID( void )
{
	return g_OpeSequenceID;
}

//[]----------------------------------------------------------------------[]
///	@brief			ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)�N���A
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void op_ClearSequenceID( void )
{
	g_OpeSequenceID = 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����ʂ��Z�b�g����Ă��邩���`�F�b�N����
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)
//static BOOL isSeasonCardType( stMasterInfo_t* pstMasterInfo )
static BOOL isSeasonCardType( stMasterInfo_t* pstMasterInfo, stZaishaInfo_t* pstZaishaInfo )
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)
{
	ushort	cnt;
	BOOL	bRet = FALSE;

// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)
	if( pstZaishaInfo && pstZaishaInfo->NormalFlag != 0 ) {		// ������p�ȊO
		return bRet;
	}
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)

	// �����ʂ���H
	if( pstMasterInfo->SeasonKind ){

		for( cnt=0; cnt<ONL_MAX_CARDNUM; cnt++ ){
			// �J�[�h�ԍ���ʂ�������H
			if( pstMasterInfo->stCardDataInfo[cnt].CardType == CARD_TYPE_PASS ){
				bRet = TRUE;
				break;
			}
		}
	}

	return bRet;
}

//[]----------------------------------------------------------------------[]
///	@brief			�J�[�h���Ƃ��Ď�M����������̃`�F�b�N(������)
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static eSEASON_CHK_RESULT CheckSeasonCardData_main( lcdbm_rsp_in_car_info_t* pstCardData, ushort no )
{
	char	c_prm;
	short	wk;
// MH810100(S) 2020/06/01 �����؂������Ő��Z���̗������A���ɓ������琸�Z�����Ōv�Z����Ă��܂�
	short	wk2;
	uchar	KigenCheckResult2;
// MH810100(E) 2020/06/01 �����؂������Ő��Z���̗������A���ɓ������琸�Z�����Ōv�Z����Ă��܂�
	short	w_syasyu;
	long	div_id;
	ushort	index = 0;
	ushort	pass_id;
	ushort	us_day;
// MH810100(S) m.saito 2020/05/22 �Ԕԃ`�P�b�g���X�i#4179 ������y�уT�[�r�X���̎�ʖ��̎g�p�s�ݒ肪�K������Ȃ��j
	short	data_adr;		// �g�p�\������ʂ��ް����ڽ
	char	data_pos;		// �g�p�\������ʂ��ް��ʒu
// MH810100(E) m.saito 2020/05/22 �Ԕԃ`�P�b�g���X�i#4179 ������y�уT�[�r�X���̎�ʖ��̎g�p�s�ݒ肪�K������Ȃ��j

	stDatetTimeYtoSec_t processTime;
	stMasterInfo_t* pMaster = &pstCardData->crd_info.dtMasterInfo;
	stZaishaInfo_t* pZaisha = &pstCardData->crd_info.dtZaishaInfo;
	stPassInfo_t* pPass 	= &pstCardData->crd_info.dtPassInfo;
	stParkKindNum_t* pCard 	= &pMaster->stCardDataInfo[no];

	short	wksy, wkey;
	uchar	uc_GtAlmPrm[ALM_LOG_ASC_DAT_SIZE];
	uchar	KigenCheckResult;

	memset( &CRD_DAT.PAS, 0, sizeof(pas_rcc) );
	memset( uc_GtAlmPrm, 0x00, ALM_LOG_ASC_DAT_SIZE );	// 0�N���A
	memcpy( &uc_GtAlmPrm[0], &pCard->ParkingLotNo, 4 );	// ���ԏ�No�Z�b�g
	pass_id = astoin( pCard->byCardNo, 5 );
	memcpy( &uc_GtAlmPrm[4], &pass_id, 2 );				// �����ID�Z�b�g

	// ------------------- //
	// ���ԏꇂ�`�F�b�N
	// ------------------- //
	if( pCard->ParkingLotNo ){
		for( index=0; index<4; index++ ){
			// �[���ƃJ�[�h���̒��ԏꇂ��v�H
			if( pCard->ParkingLotNo == prm_get( COM_PRM, S_SYS, (1 + index), 6, 1 ) ){
				break;
			}
		}
		if( index >= 4 ){
			alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			return SEASON_CHK_INVALID_PARKINGLOT;	// ���ԏꇂ�G���[
		}
	}

	// ------------------- //
	// �J�[�h�ԍ��`�F�b�N
	// ------------------- //
	if( pass_id < 1 ){
		return SEASON_CHK_INVALID_PASS_ID;		// ���ID�G���[
	}
	if( (ulong)CPrmSS[S_SYS][65] == pCard->ParkingLotNo ){
		if( CPrmSS[S_SYS][61] ){	// ����e�[�u�������@�ݒ肠��
			div_id = CPrmSS[S_SYS][61];
		}else{
			div_id = PAS_MAX + 1;
		}
		if( pass_id > (ushort)( div_id - 1 ) ){							// ð��ه@�͈̔͊O?
			return SEASON_CHK_INVALID_PASS_ID;	// ���ID�G���[
		}
	}
	else if( (ulong)CPrmSS[S_SYS][66] == pCard->ParkingLotNo ){
		if( CPrmSS[S_SYS][62] ){	// ����e�[�u�������A�ݒ肠��
			div_id = CPrmSS[S_SYS][62];
		}else{
			div_id = PAS_MAX + 1;
		}
		if( pass_id > (ushort)( div_id - CPrmSS[S_SYS][61] ) ){			// ð��هA�͈̔͊O?
			return SEASON_CHK_INVALID_PASS_ID;	// ���ID�G���[
		}
	}
	else if( (ulong)CPrmSS[S_SYS][67] == pCard->ParkingLotNo ){
		if( CPrmSS[S_SYS][63] ){	// ����e�[�u�������B�ݒ肠��
			div_id = CPrmSS[S_SYS][63];
		}else{
			div_id = PAS_MAX + 1;
		}
		if( pass_id > (ushort)( div_id - CPrmSS[S_SYS][62] ) ){			// ð��هB�͈̔͊O?
			return SEASON_CHK_INVALID_PASS_ID;	// ���ID�G���[
		}
	}
	else if( (ulong)CPrmSS[S_SYS][68] == pCard->ParkingLotNo ){
		if( pass_id > (ushort)( PAS_MAX + 1 - CPrmSS[S_SYS][63] ) ){	// ð��هC�͈̔͊O?
			return SEASON_CHK_INVALID_PASS_ID;	// ���ID�G���[
		}
	}else{
		return SEASON_CHK_NO_MASTER_PARKINGLOT;	// �e�@�̒��ԏꇂ��v�Ȃ��G���[
	}

	// ------------------- //
	// �L�������`�F�b�N
	// ------------------- //
	if( pPass->CardStatus == 2 ){
		alm_chk2( ALMMDL_SUB2, ALARM_GT_MUKOU_PASS_USE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
		return SEASON_CHK_INVALID_SEASONDATA;		// ��������G���[
	}

	// ------------------- //
	// �������`�F�b�N
	// ------------------- //
	// �������ǂ܂Ȃ��H
	if( prm_get( COM_PRM, S_TIK, 8, 1, 1 ) == 0 ){
		// ���o�ɃX�e�[�^�X = 0:������ԁH
		if( pMaster->InOutStatus == 0 ){
			return SEASON_CHK_FIRST_NG;		// �������G���[
		}
	}

	// ------------------- //
	// �̔��X�e�[�^�X�`�F�b�N
	// ------------------- //
	if( pPass->SaleStatus == 9 ){
		return SEASON_CHK_PRE_SALE_NG;		// �̔��O����G���[
	}

	// ------------------- //
	// �����ʔ͈̓`�F�b�N
	// ------------------- //
	if( pMaster->SeasonKind < 1 || pMaster->SeasonKind > 15 ){
		alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
		return SEASON_CHK_INVALID_TYPE;		// �����ʔ͈͊O�G���[
	}

// MH810100(S) m.saito 2020/05/22 �Ԕԃ`�P�b�g���X�i#4179 ������y�уT�[�r�X���̎�ʖ��̎g�p�s�ݒ肪�K������Ȃ��j
	if( ryo_buf.syubet < 6 ){
		// ���Z�Ώۂ̗������A�`F(0�`5)
		data_adr = 10*(pMaster->SeasonKind-1)+9;				// �g�p�\������ʂ��ް����ڽ�擾
		data_pos = (char)(6-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
	}
	else{
		// ���Z�Ώۂ̗������G�`L(6�`11)
		data_adr = 10*(pMaster->SeasonKind-1)+10;				// �g�p�\������ʂ��ް����ڽ�擾
		data_pos = (char)(12-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
	}
	if( prm_get( COM_PRM, S_PAS, data_adr, 1, data_pos ) ){		// �g�p�s�ݒ�H
		return SEASON_CHK_UNUSED_RYO_SYU;	// ������ʖ��g�p�G���[
	}
// MH810100(E) m.saito 2020/05/22 �Ԕԃ`�P�b�g���X�i#4179 ������y�уT�[�r�X���̎�ʖ��̎g�p�s�ݒ肪�K������Ȃ��j

	// ------------------- //
	// �����ʎg�p�ۃ`�F�b�N
	// ------------------- //
	w_syasyu = 0;
	c_prm = (char)prm_get( COM_PRM, S_PAS, 1 + (10 * (pMaster->SeasonKind - 1)), 2, 1 );	// �g�p�ړI�ݒ�Get
	if( !rangechk( 1, 14, c_prm ) ){			// ����g�p�ړI�ݒ�͈͊O
		if( c_prm == 0 ){
			return SEASON_CHK_UNUSED_TYPE;		// �����ʖ��g�p�G���[
		}else{
			return SEASON_CHK_OUT_OF_RNG_TYPE;	// ����g�p�ړI�ݒ�͈͊O�G���[
		}
	}
	if( rangechk( 3, 14, c_prm ) ){													// ����Ԏ�؊�?
		w_syasyu = c_prm - 2;														// �Ԏ�؊��p�Ԏ�Z�b�g
		if ( prm_get( COM_PRM, S_SHA, (short)(1+6*(w_syasyu-1)), 2, 5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�
			return SEASON_CHK_UNUSED_RYO_SYU;										// ������ʖ��g�p�G���[
		}
		if(( ryo_buf.waribik )|| ( ryo_buf.zankin == 0 )){							// �����ς�or�c��0�~?
			return SEASON_CHK_ORDER_OF_USE;											// ���Z���ԃG���[
		}
	}

	// ------------------- //
	// �L�������`�F�b�N
	// ------------------- //
	CRD_DAT.PAS.knd = pMaster->SeasonKind;		// ��ʾ��

	if( (short)prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,1 ) == 1 ){	// �����؂ꎞ��t����ݒ�
		c_prm = (char)prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,2 );	// �������ݒ�Get
	}else{
		c_prm = 0;
	}

	wksy = (short)pMaster->StartDate.shYear;
	if( c_prm == 1 || c_prm == 2 ){									// ������ or �J�n��������
		CRD_DAT.PAS.std = dnrmlzm( 1980, 3, 1 );					// 1980�N3��1��
	}else{
		CRD_DAT.PAS.std = 											// �L���J�n�����
		dnrmlzm( wksy, (short)pMaster->StartDate.byMonth, (short)pMaster->StartDate.byDay );
	}

	CRD_DAT.PAS.std_end[0] = (char)(pMaster->StartDate.shYear % 100);
	CRD_DAT.PAS.std_end[1] = pMaster->StartDate.byMonth;;
	CRD_DAT.PAS.std_end[2] = pMaster->StartDate.byDay;

	wkey = (short)pMaster->EndDate.shYear;
	if( c_prm == 1 || c_prm == 3 ){									// ������ or �I����������
		CRD_DAT.PAS.end = dnrmlzm( 2079, 12, 31 );					// 2079�N12��31��
	}else{
		CRD_DAT.PAS.end = 											// �L���I�������
		dnrmlzm( wkey, (short)pMaster->EndDate.byMonth, (short)pMaster->EndDate.byDay );
	}

	CRD_DAT.PAS.std_end[3] = (char)(pMaster->EndDate.shYear % 100);
	CRD_DAT.PAS.std_end[4] = pMaster->EndDate.byMonth;
	CRD_DAT.PAS.std_end[5] = pMaster->EndDate.byDay;

// MH810100(S) 2020/06/01 �����؂������Ő��Z���̗������A���ɓ������琸�Z�����Ōv�Z����Ă��܂�
//	KigenCheckResult = Ope_PasKigenCheck( CRD_DAT.PAS.std, CRD_DAT.PAS.end, CRD_DAT.PAS.knd, CLK_REC.ndat, CLK_REC.nmin );
//										// ����L�������`�F�b�N�i�߂�F0=�����J�n�����O�C1=�L���������C2=�����I��������j
//
//	if(( 1 != KigenCheckResult ) &&								// �����؂�
//	   ( prm_get( COM_PRM,S_PAS, (short)(5+10*(CRD_DAT.PAS.knd-1)), 1, 1 ) == 0 )){	// �����؂ꎞ��t���Ȃ��ݒ�?
//
//		/*** �����؂�Ŋ����؂�����t���Ȃ��ݒ�̎� ***/
//		if( 0 == KigenCheckResult ){							// �����O
//			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
//			return SEASON_CHK_BEFORE_VALID;						// �����O�G���[
//		}
//		else{													// �����؂�
//			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
//			return SEASON_CHK_AFTER_VALID;						// �����؂�G���[
//		}
//	}

	if( prm_get( COM_PRM,S_PAS, (short)(5+10*(CRD_DAT.PAS.knd-1)), 1, 1 ) == 0 ){	// �����؂ꎞ��t���Ȃ��ݒ�?
		
		//                   *-----------* ����L������
		//                                    *-------- ���ɂ��`�F�b�N
		// ���Ɏ������`�F�b�N
		wk = dnrmlzm( car_in_f.year, (short)car_in_f.mon, (short)car_in_f.day );	// ���ɔN�����m�[�}���C�Y
		wk2 = tnrmlz( 0, 0, (short)car_in_f.hour, (short)car_in_f.min );			// ���Ɏ��ԃm�[�}���C�Y
	
		KigenCheckResult = Ope_PasKigenCheck( CRD_DAT.PAS.std, CRD_DAT.PAS.end, CRD_DAT.PAS.knd, wk, wk2 );
												// ����L�������`�F�b�N�i�߂�F0=�����J�n�����O�C1=�L���������C2=�����I��������j
		
// MH810101 �t�F�[�Y2 (S) Takei 2021/01/20 �@�����؂ꎞ��t���Ȃ��ݒ�̏ꍇ�ɗL���������r�����[�ȏꍇ��NG�Ƃ���B									�@
//		if( KigenCheckResult == 2){
//			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
//			return SEASON_CHK_AFTER_VALID;						// �����؂�G���[
//		}										

		if( KigenCheckResult == 2){
			//���Ɏ������L�������I��������
			//���ɓ�������̗L������B��肠�Ɓ@�@		A��-----------------��B�@	���ɓ����@�@������
			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			return SEASON_CHK_AFTER_VALID;						// �����؂�G���[
		}
		else if( KigenCheckResult == 0){
			//���Ɏ������L�������J�n�����O�@	���Ɏ������@���Ȃ��Ē����NG�Œe���[�����Ȃ����߁A���Z���Ƀ`�F�b�N����K�v������
			//���ɓ����L���J�nA���O�@�@		���ɓ���	A��------�����Z��-----------��B�@���i���Z���������ł��悢�j	������
			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			return SEASON_CHK_BEFORE_VALID;					// �����O�G���[
		}
// MH810101 �t�F�[�Y2 (E) Takei 2021/01/20 �@�����؂ꎞ��t���Ȃ��ݒ�̏ꍇ�ɗL���������r�����[�ȏꍇ��NG�Ƃ���B	

		//                                *-----------* ����L������
		//       ���Z���`�F�b�N    ---*
		// ���Z�������`�F�b�N
		wk = dnrmlzm( car_ot_f.year, (short)car_ot_f.mon, (short)car_ot_f.day );	// ���Z�N�����m�[�}���C�Y
		wk2 = tnrmlz( 0, 0, (short)car_ot_f.hour, (short)car_ot_f.min );			// ���Z�����m�[�}���C�Y
	
		KigenCheckResult2 = Ope_PasKigenCheck( CRD_DAT.PAS.std, CRD_DAT.PAS.end, CRD_DAT.PAS.knd, wk, wk2 );
											// ����L�������`�F�b�N�i�߂�F0=�����J�n�����O�C1=�L���������C2=�����I��������j
											
// MH810101 �t�F�[�Y2 (S) Takei 2021/01/20 �@�����؂ꎞ��t���Ȃ��ݒ�̏ꍇ�ɗL���������r�����[�ȏꍇ��NG�Ƃ���B
//		if( KigenCheckResult2 == 0){
//			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
//			return SEASON_CHK_BEFORE_VALID;						// �����؂�G���[
//		}
		
		if( KigenCheckResult2 == 0){
			//���Z�������L�������J�n�����O
			//���Z�����L���J�nA���O�@�@		�����ɓ� �����Z��	A��-----------------��B�@������
			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			return SEASON_CHK_BEFORE_VALID;						// �����؂�G���[
// MH810101 �t�F�[�Y2 (S) Endo 2021/01/22 �@�����؂ꎞ��t���Ȃ��ݒ�̏ꍇ�ɗL���������r�����[�ȏꍇ��NG�Ƃ���B
//		}else if( KigenCheckResult == 2){
		}else if( KigenCheckResult2 == 2){
// MH810101 �t�F�[�Y2 (E) Endo 2021/01/22 �@�����؂ꎞ��t���Ȃ��ݒ�̏ꍇ�ɗL���������r�����[�ȏꍇ��NG�Ƃ���B
			//���Z�������L�������I��������
			//���Z�����L���J�nB���O�@�@		A��-------�����ɓ�----------��B�@�����Z��	������
			//�����OK�ɂ������̂Ȃ�΁A07-00X5 = 1 ���X �������^�����؂��t��1�ɂ����OK�B
			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			return SEASON_CHK_AFTER_VALID;						// �����؂�G���[
		}
// MH810101 �t�F�[�Y2 (E) Takei 2021/01/20 �@�����؂ꎞ��t���Ȃ��ݒ�̏ꍇ�ɗL���������r�����[�ȏꍇ��NG�Ƃ���B

	}

// MH810100(E) 2020/06/01 �����؂������Ő��Z���̗������A���ɓ������琸�Z�����Ōv�Z����Ă��܂�

	// ------------------- //
	// �A���`�p�X�`�F�b�N
	// ------------------- //
	memset( &processTime, 0, sizeof(processTime) );
	switch( pMaster->InOutStatus ){
		// �o�ɒ�
		case 1:
			memcpy( &processTime, &pZaisha->dtPaymentDateTime.dtTimeYtoSec, sizeof(processTime) );
			break;
		// ���ɒ�
		case 2:
			memcpy( &processTime, &pZaisha->dtEntryDateTime.dtTimeYtoSec, sizeof(processTime) );
			break;
		default:
			break;
	}

	if( prm_get( COM_PRM, S_PAS, (short)(2+10*(pMaster->SeasonKind - 1)), 1, 1 ) ){	// ���o�`�F�b�N����ݒ�H
		us_day = dnrmlzm( (short)processTime.shYear, (short)processTime.byMonth, (short)processTime.byDay );	// �J�[�h���_�����N�����m�[�}���C�Y
		wk = dnrmlzm( car_in_f.year, (short)car_in_f.mon, (short)car_in_f.day );								// ���ɔN�����m�[�}���C�Y
		// ���ތ��� > ���Ɍ���?�H
		if( us_day > wk ){
			return SEASON_CHK_NTIPASS_ERROR;			// ���o�G���[
		}
		if( us_day == wk ){
			wk = tnrmlz( 0, 0, (short)car_in_f.hour, (short)car_in_f.min );
			us_day = tnrmlz( 0, 0, (short)processTime.byHours, (short)processTime.byMinute );
			if( us_day > wk ){
				return SEASON_CHK_NTIPASS_ERROR;		// ���o�G���[
			}
		}
	}

	if( w_syasyu ){
		vl_now = V_SYU;													// ��ʐ؊�
		syashu = (char)w_syasyu;										// �Ԏ�
	}else{																// �Ԏ�؊��łȂ�
		vl_now = V_TSC;													// �����(���Ԍ����p�L��)
	}

	// ����f�[�^�����ʃG���A�ɃZ�[�u�ivl_paschg()�֐����R�[�����邽�߁j
	CRD_DAT.PAS.pno = (long)pCard->ParkingLotNo;						// ���ԏꇂ
	CRD_DAT.PAS.GT_flg = 1;												// GT�t�H�[�}�b�g�t���O
	CRD_DAT.PAS.cod = astoin( pCard->byCardNo, 5 );						// ���ID
	CRD_DAT.PAS.sts = pMaster->InOutStatus;								// ���o�ɃX�e�[�^�X
	CRD_DAT.PAS.trz[0] = processTime.byMonth;							// ������
	CRD_DAT.PAS.trz[1] = processTime.byDay;								// ������
	CRD_DAT.PAS.trz[2] = processTime.byHours;							// ������
	CRD_DAT.PAS.trz[3] = processTime.byMinute;							// ������
	CRD_DAT.PAS.typ = index;											// ����敪

	InTeiki_PayData_Tmp.syu 		 = (uchar)CRD_DAT.PAS.knd;			// ��������
	InTeiki_PayData_Tmp.status 		 = (uchar)CRD_DAT.PAS.sts;			// ������ð��(�ǎ掞)
	InTeiki_PayData_Tmp.id 			 = CRD_DAT.PAS.cod;					// �����id
	InTeiki_PayData_Tmp.pkno_syu 	 = CRD_DAT.PAS.typ;					// ��������ԏ�m���D��� (0-3:��{,�g��1-3)
	InTeiki_PayData_Tmp.update_mon 	 = 0;								// �X�V����
	InTeiki_PayData_Tmp.s_year 		 = pMaster->StartDate.shYear;		// �L�������i�J�n�F�N�j
	InTeiki_PayData_Tmp.s_mon 		 = pMaster->StartDate.byMonth;		// �L�������i�J�n�F���j
	InTeiki_PayData_Tmp.s_day 		 = pMaster->StartDate.byDay;		// �L�������i�J�n�F���j
	InTeiki_PayData_Tmp.e_year 		 = pMaster->EndDate.shYear;			// �L�������i�I���F�N�j
	InTeiki_PayData_Tmp.e_mon 		 = pMaster->EndDate.byMonth;		// �L�������i�I���F���j
	InTeiki_PayData_Tmp.e_day 		 = pMaster->EndDate.byDay;			// �L�������i�I���F���j
	memcpy( InTeiki_PayData_Tmp.t_tim, CRD_DAT.PAS.trz, 4 );			// ������������
	InTeiki_PayData_Tmp.update_rslt1 = 0;								// ����X�V���Z���̍X�V����			�i�@OK�F�X�V�����@�^�@NG�F�X�V���s�@�j
	InTeiki_PayData_Tmp.update_rslt2 = 0;								// ����X�V���Z���̃��x�����s����	�i�@OK�F���픭�s�@�^�@NG�F���s�s�ǁ@�j

	OpeNtnetAddedInfo.PayMethod = 2;	// ���Z���@=2:��������Z

	return SEASON_CHK_OK;	// �G���[�Ȃ�
}

//[]----------------------------------------------------------------------[]
///	@brief			�J�[�h���Ƃ��Ď�M����������̃`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static eSEASON_CHK_RESULT CheckSeasonCardData( lcdbm_rsp_in_car_info_t* pstCardData )
{
	ushort	cnt;
	eSEASON_CHK_RESULT	result = SEASON_CHK_OK;

	for( cnt=0; cnt<ONL_MAX_CARDNUM; cnt++ ){

		if( pstCardData->crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType != CARD_TYPE_PASS ){	// �}�̎�ʂ�����łȂ�
			continue;
		}
		// �`�F�b�N������
		result = CheckSeasonCardData_main( pstCardData, cnt );
		break;
	}

	return result;
}

// MH810100(E) K.Onodera 2019/11/18 �Ԕԃ`�P�b�g���X�i�����e�i���X�I�������j

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
/*[]----------------------------------------------------------------------[]*/
/*| ���Z�̈�ւ��ް��̊i�[ (���σ��[�_�ł�QR�R�[�h����)			               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcQrSet_PayData					     	              |*/
/*| PARAMETER    : *buf		: ����ް�				                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2021-07-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]*/
void	EcQrSet_PayData( void *buf )
{
	EC_SETTLEMENT_RES	*p;

	p = (EC_SETTLEMENT_RES*)buf;
	if(p->Column_No == 0xFF) {
		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;								// �݂Ȃ�����
	}
	PayData.Electron_data.Ec.pay_ryo	= p->settlement_data;										// ���ϋ��z
	PayData.Electron_data.Ec.pay_after	= p->settlement_data_after;									// �c��
	PayData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);					// ���ϓ���(Normalize)

	PayData.Electron_data.Ec.Brand.Qr.PayKind = p->Brand.Qr.PayKind;								// �x�����
	PayData.Electron_data.Ec.e_pay_kind	= EC_QR_USED;												// ���Z���
	PayData.EcResult = p->Result;

	memcpy(PayData.Electron_data.Ec.QR_Kamei, &p->QR_Kamei,
			sizeof(PayData.Electron_data.Ec.QR_Kamei));												// �����X��
	memcpy(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo, &p->Brand.Qr.PayTerminalNo,
			sizeof(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo));								// �x���[��ID
	memcpy(PayData.Electron_data.Ec.inquiry_num, &p->inquiry_num,
			sizeof(PayData.Electron_data.Ec.inquiry_num));											// ����ԍ�
	memcpy(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo, &p->Brand.Qr.MchTradeNo,
			sizeof(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo));									// Mch����ԍ�

	// �݂Ȃ����ρH
	if (PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 1) {
		// �x���[��ID��'0'���߁i13���j
		memset(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo, 0x20,
				sizeof(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo));
		memset(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo, 0x30, 13);
		// ����ԍ���'0'���߁i15���j
		memset(PayData.Electron_data.Ec.inquiry_num, 0x30,
				sizeof(PayData.Electron_data.Ec.inquiry_num));
		// Mch����ԍ����X�y�[�X���߁i32���j
		memset(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo, 0x20,
				sizeof(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo));
	}

// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
	if (PayData.EcResult == EPAY_RESULT_NG) {
		// ���ό��ʁ�����NG�̏ꍇ�͐��Z���~�f�[�^���M�ΏۂƂ���
		PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
		// ���Z���~�׸ނ�deemSettleCancal�ɾ�āi���Z�f�[�^���M���ɂ����׸ނ��g�p����j
		// ���Ϗ�ԁ��u�݂Ȃ����ρv�� deemSettleCancal=0, ����ȊO��deemSettleCancal=1�ƂȂ�B
		PayData.Electron_data.Ec.E_Flag.BIT.deemSettleCancal = p->E_Flag.BIT.deemSettleCancal;
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
	Ec_Data152Save();
}

//[]----------------------------------------------------------------------[]
///	@brief			��Q�A���[���������m�N��LCD�\���ؑ�
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			t.sato
///	@note			��Q�A���[���������m�N��LCD�\���ؑ�<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/01/28<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void lcd_Recv_FailureContactData_edit(void)
{
	lcd_wmsg_dsp_elec(1, ERR_CHR[78], ERR_CHR[85], 0, 0, COLOR_RED, LCD_BLINK_OFF);
}

//[]----------------------------------------------------------------------[]
///	@brief			ID152���Z���f�[�^�̓d�q�[�����Ϗ��ۑ�
//[]----------------------------------------------------------------------[]
///	@param[in]		wk_paydata 	: PayData
///	@return			void
///	@note			�g�p�����d�q�[���J�[�h����ID152�֕ۑ�<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/21<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void Ec_Data152Save(void)
{
	ulong	zangaku;
	ulong	app_no;
	ulong	slip_no;
	uchar	trans_sts;
	char	dummy_kid[6];
	char	dummy_CCTNum[13];
	uchar	dummy_Card_ID[20];
	uchar	dummy_inquiry_num[16];
	uchar	dummy_MchTradeNo[32];
	uchar	dummy_PayTermID[16];

	// �e�u�����h����ID152�փf�[�^��ۑ�����
	switch (PayData.Electron_data.Ec.e_pay_kind) {
	// �N���W�b�g
	case EC_CREDIT_USED:

		if( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ){
			// ����ԍ�
			memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
			// �[�����ʔԍ�
			memset( &dummy_CCTNum[0], 0x20, sizeof(dummy_CCTNum) );
			// KID�R�[�h
			memset( &dummy_kid[0], 0x20, sizeof(dummy_kid) );
			// ���F�ԍ�
			app_no = 0;
			// �`�[�ԍ�
			slip_no = 0;
		}
		else{
			// ����ԍ�
			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
				PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
				// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.��0x20h���߂���B
				// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
				memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
			}
			else {
				memcpy( &dummy_Card_ID[0], &ryo_buf.credit.card_no[0], sizeof(ryo_buf.credit.card_no) );
			}
			// �[�����ʔԍ�
			memcpy( &dummy_CCTNum[0], &ryo_buf.credit.CCT_Num[0], sizeof(dummy_CCTNum) );
			// KID�R�[�h
			memcpy( &dummy_kid[0], &ryo_buf.credit.kid_code, sizeof(dummy_kid) );
			// ���F�ԍ�
			app_no = ryo_buf.credit.app_no;
			// �`�[�ԍ�
			slip_no = ryo_buf.credit.slip_no;
		}
		// ���p���z
		NTNET_Data152Save((void *)(&ryo_buf.credit.pay_ryo), NTNET_152_CPAYRYO);
		// ����ԍ�
		NTNET_Data152Save((void *)(&dummy_Card_ID[0]), NTNET_152_CCARDNO);
		// �[�����ʔԍ�
		NTNET_Data152Save((void *)(&dummy_CCTNum[0]), NTNET_152_CCCTNUM);
		// KID�R�[�h
		NTNET_Data152Save((void *)(&dummy_kid[0]), NTNET_152_CKID);
		// ���F�ԍ�
		NTNET_Data152Save((void *)(&app_no), NTNET_152_CAPPNO);
		// �`�[�ԍ�
		NTNET_Data152Save((void *)(&slip_no), NTNET_152_SLIPNO);
		break;

	// QR
	case EC_QR_USED:

		if( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ){
			// Mch����ԍ�
			memset(&dummy_MchTradeNo[0], 0x20, sizeof(dummy_MchTradeNo));

			if (PayData.Electron_data.Ec.E_Flag.BIT.deemSettleCancal == 1) {
				// ���ό��ʂ���M�ł��Ȃ������̂�
				// ���ꂼ��̌�����0x20���߂��A�x���[��ID��13���A����ԍ���15���A0x30���߂���
				// �x���[��ID
				memset(&dummy_PayTermID, 0x20, sizeof(dummy_PayTermID));
				memset(&dummy_PayTermID, 0x30, 13);
				// ����ԍ�
				memset(&dummy_inquiry_num, 0x20, sizeof(dummy_inquiry_num));
				memset(&dummy_inquiry_num, 0x30, 15);
			}
			else {
				// �x���[��ID
				memcpy(&dummy_PayTermID[0], &PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo[0], 16);
				// ����ԍ�
				memset(&dummy_inquiry_num, 0x20, sizeof(dummy_inquiry_num));
				memcpy(&dummy_inquiry_num[0], &PayData.Electron_data.Ec.inquiry_num[0], 15);
			}
		}
		else{
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// Mch����ԍ�
				memset(&dummy_MchTradeNo[0], 0x20, sizeof(dummy_MchTradeNo));
				// �x���[��ID
				memcpy(&dummy_PayTermID[0], &PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo[0], 16);
				// ����ԍ�
				memset(&dummy_inquiry_num, 0x20, sizeof(dummy_inquiry_num));
				memcpy(&dummy_inquiry_num[0], &PayData.Electron_data.Ec.inquiry_num[0], 15);
			}
			else{
				// Mch����ԍ�
				memcpy(&dummy_MchTradeNo[0], &PayData.Electron_data.Ec.Brand.Qr.MchTradeNo[0], sizeof(dummy_MchTradeNo));
				// �x���[��ID
				memset(&dummy_PayTermID, 0x20, sizeof(dummy_PayTermID));
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6381 QR�R�[�h���ςŒʏ퐸�Z���̐��Z�f�[�^�Ō��Ϗ��̎x���[��ID�A����ԍ��� 0�ƂȂ��Ă��܂�
//				memset(&dummy_PayTermID, 0x30, 13);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6381 QR�R�[�h���ςŒʏ퐸�Z���̐��Z�f�[�^�Ō��Ϗ��̎x���[��ID�A����ԍ��� 0�ƂȂ��Ă��܂�
				// ����ԍ�
				memset(&dummy_inquiry_num, 0x20, sizeof(dummy_inquiry_num));
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6381 QR�R�[�h���ςŒʏ퐸�Z���̐��Z�f�[�^�Ō��Ϗ��̎x���[��ID�A����ԍ��� 0�ƂȂ��Ă��܂�
//				memset(&dummy_inquiry_num, 0x30, 15);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6381 QR�R�[�h���ςŒʏ퐸�Z���̐��Z�f�[�^�Ō��Ϗ��̎x���[��ID�A����ԍ��� 0�ƂȂ��Ă��܂�
			}
		}
		// �J�[�h���ϋ敪
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.e_pay_kind), NTNET_152_ECARDKIND);
		// ���p���z
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.pay_ryo), NTNET_152_EPAYRYO);
		// Mch����ԍ�
		NTNET_Data152Save((void *)(&dummy_MchTradeNo[0]), NTNET_152_QR_MCH_TRADE_NO);
		// �x���[��ID
		NTNET_Data152Save((void *)(&dummy_PayTermID[0]), NTNET_152_QR_PAY_TERM_ID);
		// ����ԍ�
		NTNET_Data152Save((void *)(&dummy_inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
		// ���σu�����h
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.Brand.Qr.PayKind), NTNET_152_QR_PAYKIND);
		break;

// MH810105(S) MH364301 SX20�����Ή�
	case SUICA_USED:
		if( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ){
			// �J�[�h�ԍ�
			memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
			memset( &dummy_Card_ID[0], 0x30, ECARDID_SIZE_SUICA );
			memset( &dummy_Card_ID[0], 'Z', 2 );
			// �J�[�h�c�z
			zangaku = 0;
		}
		else{
			// �J�[�h�ԍ�
			memcpyFlushLeft(&dummy_Card_ID[0],
							&PayData.Electron_data.Suica.Card_ID[0],
							sizeof(dummy_Card_ID),
							sizeof(PayData.Electron_data.Suica.Card_ID));
			// �J�[�h�c�z
			zangaku = PayData.Electron_data.Suica.pay_after;
		}
		// �₢���킹�ԍ�
		memset( &dummy_inquiry_num[0], 0x20, sizeof(dummy_inquiry_num) );
		// ���F�ԍ�
		app_no = 0;

		// �J�[�h���ϋ敪
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.e_pay_kind), NTNET_152_ECARDKIND);
		// ���p���z
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.pay_ryo), NTNET_152_EPAYRYO);
		// �J�[�h�ԍ�
		NTNET_Data152Save((void *)(&dummy_Card_ID[0]), NTNET_152_ECARDID);
		// �⍇���ԍ�
		NTNET_Data152Save((void *)(&dummy_inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
		// �J�[�h�c�z
		NTNET_Data152Save((void *)(&zangaku), NTNET_152_EPAYAFTER);
		// ���F�ԍ�
		NTNET_Data152Save((void*)(&app_no), NTNET_152_CAPPNO);
		break;
// MH810105(E) MH364301 SX20�����Ή�

	// ���̑��i�d�q�}�l�[�j
	default:

		if( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ){
			// �J�[�h�ԍ�
			memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
			memset( &dummy_Card_ID[0], 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]) );
			memset( &dummy_Card_ID[0], 'Z', 2 );
			// �J�[�h�c�z
			zangaku = 0;
			// �₢���킹�ԍ�
			memset( &dummy_inquiry_num[0], 0x20, sizeof(dummy_inquiry_num) );
			// ���F�ԍ�
			app_no = 0;
		}
		else{
			// �J�[�h�ԍ�
			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
				PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
				// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.���J�[�h������0x30���߂����l�߂�ZZ����B
				// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
				memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
				memset( &dummy_Card_ID[0], 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]) );
				memset( &dummy_Card_ID[0], 'Z', 2 );
			}
			else{
				memcpy( &dummy_Card_ID[0], &PayData.Electron_data.Ec.Card_ID[0], sizeof(dummy_Card_ID) );
			}
			// �J�[�h�c�z
			zangaku = PayData.Electron_data.Ec.pay_after;
			// �₢���킹�ԍ�
			memcpy( &dummy_inquiry_num[0], &PayData.Electron_data.Ec.inquiry_num[0], sizeof(PayData.Electron_data.Ec.inquiry_num) );
			// ���F�ԍ�
			switch(PayData.Electron_data.Ec.e_pay_kind){
				case EC_ID_USED:
					app_no = astoinl(PayData.Electron_data.Ec.Brand.Id.Approval_No, 7);
					break;
				case EC_QUIC_PAY_USED:
					app_no = astoinl(PayData.Electron_data.Ec.Brand.Quickpay.Approval_No, 7);
					break;
				case EC_PITAPA_USED:
					app_no = astoinl(PayData.Electron_data.Ec.Brand.Pitapa.Approval_No, 8);
					break;
				default:
					app_no = 0;
					break;
			}
		}
		// �J�[�h���ϋ敪
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.e_pay_kind), NTNET_152_ECARDKIND);
		// ���p���z
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.pay_ryo), NTNET_152_EPAYRYO);
		// �J�[�h�ԍ�
		NTNET_Data152Save((void *)(&dummy_Card_ID[0]), NTNET_152_ECARDID);
		// �⍇���ԍ�
		NTNET_Data152Save((void *)(&dummy_inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
		// �J�[�h�c�z
		NTNET_Data152Save((void *)(&zangaku), NTNET_152_EPAYAFTER);
		// ���F�ԍ�
		NTNET_Data152Save((void*)(&app_no), NTNET_152_CAPPNO);
		break;
	}

	// ����X�e�[�^�X
	if (PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
		trans_sts = 3;		// �����x���s��
	}
	else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
		trans_sts = 2;		// �����x���ς�
	}
	else if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
		trans_sts = 1;		// �݂Ȃ�����
	}
	else {
		trans_sts = 0;		// �x����
	}
	NTNET_Data152Save((void*)(&trans_sts), NTNET_152_TRANS_STS);
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810105(S) MH364301 �C���{�C�X�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�̎��؍Ĕ��s�i�C���{�C�X�Ή��j
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/22
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void ryo_reisu(void)
{
	T_FrmReceipt	rec_data;

	memset(&rec_data, 0, sizeof(rec_data));
	rec_data.prn_kind = R_PRI;										// �������ʁFڼ��
	rec_data.prn_data = &Cancel_pri_work;							// �̎��؈��ް����߲�����
	if (OPECTL.f_ReIsuType == 1) {
		rec_data.kakari_no = 99;									// �W��No.99�i�Œ�j
	}
	else {
		rec_data.kakari_no = 0;										// �W��No.0�i�Œ�j
	}
	rec_data.reprint = ON;											// �Ĕ��s�׸޾�āi�Ĕ��s�j
	memcpy( &rec_data.PriTime, &CLK_REC, sizeof(date_time_rec) );	// �Ĕ��s�����i���ݓ����j���
	queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
	wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );						// ���엚��o�^
	LedReq( CN_TRAYLED, LED_ON );									// ��ݎ�o�����޲��LED ON
	LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );

	OPECTL.f_ReIsuType = 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g�󎚎��s����
//[]----------------------------------------------------------------------[]
///	@param[in]		prm	: ����
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/05/06
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void op_rct_failchk(void)
{
	if (OPECTL.Ope_mod == 3) {
// GG129000(S) R.Endo 2023/02/22 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
// 		if (Ope_isPrinterReady() &&
// 			paperchk() != -1 &&
// 			Ope_isJPrinterReady() &&
// 			paperchk2() != -1) {
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 		if ( ( IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1)) &&
// 		                     (Ope_isJPrinterReady() && (paperchk2() != -1))) ||	
// 		     (!IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1))) ) {	// �C���{�C�X�ł͂Ȃ��ꍇ�̓��V�[�g�̂ݔ���
		// �C���{�C�X�ł͂Ȃ��ꍇ�̓��V�[�g�̂ݔ���A�d�q�̎��؂̏ꍇ�͏�Ƀ��V�[�g�o�͉�
		if ( ( IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1))) &&
		                     (                Ope_isJPrinterReady() && (paperchk2() != -1) )) ||	
		     (!IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1)))) ) {
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// GG129000(E) R.Endo 2023/02/22 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
			// ���V�[�g�E�W���[�i���󎚉\�ł���΁A�󎚐����Ɣ��f����
		}
		else {
			// ���V�[�g�E�W���[�i���̂ǂ��炩�󎚕s�ł���΁A�󎚎��s�Ɣ��f����

			LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);
			OPECTL.f_DelayRyoIsu = 0;
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
			if( ryo_stock == 1 ){							// �󎚃f�[�^�X�g�b�N��
				MsgSndFrmPrn(PREQ_STOCK_CLEAR, R_PRI, 0);	// �󎚃f�[�^�N���A
				ryo_stock = 2;								// �X�g�b�N�f�[�^�j��
			}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

			// �̎��؃{�^��LED�����Ȃ�
			op_ReciptLedOff();
			// �̎��ؔ��s���s�\��
			receipt_output_error();
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
			// ���V�[�g�󎚊����҂��^�C�}���Z�b�g
			Lagcan( OPETCBNO, TIMERNO_RPT_PRINT_WAIT );
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
		}
	}
	else {
		if (f_reci_ana) {
			// ������̗̎��؃{�^������
// GG129000(S) R.Endo 2023/02/22 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
// 			if (Ope_isPrinterReady() &&
// 				paperchk() != -1 &&
// 				Ope_isJPrinterReady() &&
// 				paperchk2() != -1) {
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 			if ( ( IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1)) &&
// 			                     (Ope_isJPrinterReady() && (paperchk2() != -1))) ||	
// 			     (!IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1))) ) {	// �C���{�C�X�ł͂Ȃ��ꍇ�̓��V�[�g�̂ݔ���
			// �C���{�C�X�ł͂Ȃ��ꍇ�̓��V�[�g�̂ݔ���A�d�q�̎��؂̏ꍇ�͏�Ƀ��V�[�g�o�͉�
			if ( ( IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1))) &&
			                                     (Ope_isJPrinterReady() && (paperchk2() != -1)) ) ||
			     (!IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1)))) ) {
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// GG129000(E) R.Endo 2023/02/22 �Ԕԃ`�P�b�g���X4.0 #6927 ���Z������ʂŃW���[�i�����؂�A���V�[�g���؂�𔭐������Ă��̎��؃{�^�������\�̂܂܂ɂȂ�
				// ���V�[�g�E�W���[�i���󎚉\�ł���΁A�󎚐����Ɣ��f����

				// ������̗̎��؁i���V�[�g�j�󎚊���
				f_reci_ana = 0;
			}
			else {
				// ���V�[�g�E�W���[�i���̂ǂ��炩�󎚕s�ł���΁A�󎚎��s�Ɣ��f����

				// �̎��؃{�^��LED�����Ȃ�
				LagCan500ms(LAG500_RECEIPT_LEDOFF_DELAY);
				op_ReciptLedOff();
				// �̎��ؔ��s���s�\��
				receipt_output_error();
				f_reci_ana = 0;
			}
		}
		else {
			// �̎��؍Ĕ��s�v����M
			LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);
			OPECTL.f_ReIsuType = 0;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�W���[�i���󎚎��s����
//[]----------------------------------------------------------------------[]
///	@param[in]		prm	: ����
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/05/06
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void op_jnl_failchk(void)
{

	if (OPECTL.Ope_mod == 3) {
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 		if (Ope_isPrinterReady() &&
// 			paperchk() != -1 &&
		if ((IS_ERECEIPT || (Ope_isPrinterReady() && (paperchk() != -1))) &&
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
			Ope_isJPrinterReady() &&
			paperchk2() != -1) {
			// ���V�[�g�E�W���[�i���󎚉\�ł���΁A�󎚐����Ɣ��f����

			// �̎��؁i�W���[�i���j�󎚊���
			OPECTL.f_DelayRyoIsu = 2;

			if (OPECTL.RECI_SW == 1) {
				// �̎��؁i���V�[�g�j���󎚂���
				OPECTL.RECI_SW = 0;
				queset(OPETCBNO, OPE_DELAY_RYO_PRINT, 0, NULL);
			}
		}
		else {
			// ���V�[�g�E�W���[�i���̂ǂ��炩�󎚕s�ł���΁A�󎚎��s�Ɣ��f����

			OPECTL.f_DelayRyoIsu = 0;
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
			if( ryo_stock == 1 ){							// �󎚃f�[�^�X�g�b�N��
				MsgSndFrmPrn(PREQ_STOCK_CLEAR, R_PRI, 0);	// �󎚃f�[�^�N���A
				ryo_stock = 2;								// �X�g�b�N�f�[�^�j��
			}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

			// �̎��؃{�^��LED�����Ȃ�
			op_ReciptLedOff();

			// ��ʍX�V
			statusChange_DispUpdate();

			if (OPECTL.RECI_SW == 1) {
				// �̎��ؔ��s���s�\��
				receipt_output_error();
			}
		}
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���Z�������̗̎��ؔ��s���ɑҋ@��Ԃɖ߂�Ȃ��悤�ɂ���j
		// ��а�ɂļެ��و󎚊����i���s�j��ʒm
		queset(OPETCBNO, OPE_DUMMY_NOTICE, 0, 0);
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���Z�������̗̎��ؔ��s���ɑҋ@��Ԃɖ߂�Ȃ��悤�ɂ���j
	}
	else {
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 		if (Ope_isPrinterReady() &&
// 			paperchk() != -1 &&
		if ((IS_ERECEIPT || (Ope_isPrinterReady() && (paperchk() != -1))) &&
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
			Ope_isJPrinterReady() &&
			paperchk2() != -1) {
			// ���V�[�g�E�W���[�i���󎚉\�ł���΁A�󎚐����Ɣ��f����

			if (OPECTL.f_ReIsuType) {
				// �̎��؍Ĕ��s�v����M��̗̎��؁i�W���[�i���j�󎚊���
				ryo_reisu();
			}
		}
		else {
			// ���V�[�g�E�W���[�i���̂ǂ��炩�󎚕s�ł���΁A�󎚎��s�Ɣ��f����
			OPECTL.f_ReIsuType = 0;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�̎��؏o�̓G���[���|�b�v�A�b�v
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/26
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void receipt_output_error(void)
{

	short	time = 6*2;										// �̎��؎��s�\������
	uchar	wk = 0;

	// �^�C�}�[�N�����H
	if( LagTim500ms_Is_Counting(LAG500_RECEIPT_MISS_DISP_TIMER) ){
		return;
	}

// MH810105(S) MH364301 �C���{�C�X�Ή� GT-4100
//	wk = 86;												// "�@�̎��؂̔��s�Ɏ��s���܂����@"
//	if (prm_get(COM_PRM, S_RTP, 2, 1, 3) == 1) {			// �̎��ؕ\���(2)(17-0002�C)
//		wk = 87;											// "�@�̎����̔��s�Ɏ��s���܂����@"
//	}
// MH810105(E) MH364301 �C���{�C�X�Ή� GT-4100

	switch (OPECTL.f_RctErrDisp)
	{
		case 0:												// �̎��؎��sү���ޖ��\��
			if( f_reci_ana ){								// �̎��؉������H
				Ope2_WarningDisp( 6*2, ERR_CHR[wk] );		// �̎��؏o�̓��[�j���O�\�� (6sec)
			}
			else{
				grachr( 7, 0, 30, 1, COLOR_RED,  LCD_BLINK_OFF, ERR_CHR[wk] );
				// �̎��ؔ��s���s�\���^�C�}�[�N��
				LagTim500ms( LAG500_RECEIPT_MISS_DISP_TIMER, (short)(time + 1), receipt_output_error );
// MH810105(S) MH364301 �C���{�C�X�Ή� GT-4100
				lcdbm_notice_dsp( POP_RECIPT_OUT_ERROR, 0 );	// 11:�̎��ؔ��s���s
// MH810105(E) MH364301 �C���{�C�X�Ή� GT-4100
			}
			OPECTL.f_RctErrDisp = 1;						// �̎��؎��sү���ޕ\����
			break;
		case 1:												// �̎��؎��sү���ޕ\�����H
			Lcd_Receipt_disp();

			if (OPECTL.Ope_mod == 3 && isEC_CARD_INSERT()) {
				// �J�[�h�������҂��p�����ł���΁A�ĕ\������
				ec_MessageAnaOnOff( 1, 3 );
			}
			OPECTL.f_RctErrDisp = 0xff;						// �̎��؎��sү���ޕ\���I��
			break;
		default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			��ԕω��ɂ���ʍX�V����
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/05/16
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void statusChange_DispUpdate(void)
{
	ushort	wk_ec_MessagePtnNum = 0;

	switch (OPECTL.Ope_mod)
	{
	case 2:											// ���Z��
		// ���G���[�\�����H
		if( OPECTL.f_CrErrDisp != 0 ){
			// ����������ɂV�s�ڂ��ĕ\������̂ł����ł͕\�����Ȃ�
			break;
		}

		// �����\��(����)�łȂ�
		if( ryodsp ){
			// (�װ�ȊO��)EC�֘Aү���ޕ\�����H
			if( isEC_MSG_DISP() && ECCTL.ec_MessagePtnNum != 98 ){
				// ү���ޔԍ�����ү���ޔԍ��֕ێ�
				wk_ec_MessagePtnNum = ECCTL.ec_MessagePtnNum;
				ec_MessageAnaOnOff( 0, 0 );
			}
			else{
				Lcd_Receipt_disp();
			}

			// ��ү���ޔԍ��ɒl��ێ����Ă�����ēxү���ނ�\������
			if( wk_ec_MessagePtnNum != 0 ){
				// ���ޔ������\���H
				ec_MessageAnaOnOff( 1, (short)wk_ec_MessagePtnNum );
			}
		}
		break;
	case 3:											// ���Z����
		// ���s�\�����͖���
		if( OPECTL.f_RctErrDisp == 1 ){
			break;
		}
		// �\���������ec�֘Aү���ޕ\�����͖����iec�֘Aү���ޗD��j
		if( isEC_MSG_DISP() && OPECTL.f_RctErrDisp == 0xff ){
			break;
		}

		// (�װ�ȊO��)EC�֘Aү���ޕ\�����H
		if( isEC_MSG_DISP() ){
			// ү���ޔԍ�����ү���ޔԍ��֕ێ�
			wk_ec_MessagePtnNum = ECCTL.ec_MessagePtnNum;
			ec_MessageAnaOnOff( 0, 0 );
		}
		else{
			Lcd_Receipt_disp();
		}

		// ��ү���ޔԍ��ɒl��ێ����Ă�����ĕ\������
		if( wk_ec_MessagePtnNum != 0 ){
			// �̎��؃{�^���������H
			if( OPECTL.RECI_SW == 0 ){
				ec_MessageAnaOnOff( 1, (short)wk_ec_MessagePtnNum );
			}
			else{
				// �̎��؃{�^�������ς̏ꍇ��receipt_output_error�ŉ�ʕ\������
				// �����ł͉������Ȃ�
			}
		}
		break;
	default:
		break;
	}
}
// MH810105(E) MH364301 �C���{�C�X�Ή�

// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
//[]----------------------------------------------------------------------[]
///	@brief			���Z�J�n�������ݒ�(���ɏ�񂩂�ݒ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void SetPayStartTimeInCarInfo()
{
	struct clk_rec PayDateTime;

	PayDateTime.year = lcdbm_rsp_in_car_info_main.PayDateTime.Year;
	PayDateTime.mont = lcdbm_rsp_in_car_info_main.PayDateTime.Mon;
	PayDateTime.date = lcdbm_rsp_in_car_info_main.PayDateTime.Day;
	PayDateTime.hour = lcdbm_rsp_in_car_info_main.PayDateTime.Hour;
	PayDateTime.minu = lcdbm_rsp_in_car_info_main.PayDateTime.Min;
	PayDateTime.seco = lcdbm_rsp_in_car_info_main.PayDateTime.Sec;
	PayDateTime.ndat = dnrmlzm(
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Year,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Mon,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Day);
	PayDateTime.nmin = tnrmlz((short)0, (short)0,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Hour,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Min);
	PayDateTime.week = (uchar)youbiget(
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Year,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Mon,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Day);
	SetPayStartTime(&PayDateTime);
}
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)�ݒ�
//[]----------------------------------------------------------------------[]
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// ///	@param[in]		void
///	@param[in]		uchar	: ����t���O<br>
///							  0 = ���ڈȍ~<br>
///							  1 = ����<br>
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
///	@return			ret		: �����ݒ茋��<br>
///							  0 = ��������<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// static short in_time_set() {
static short in_time_set(uchar firstFlg) {
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
	short ret;

	// �����ݒ�
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// 	ret = cal_cloud_fee_set();
	ret = cal_cloud_fee_set(firstFlg);
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
	if ( ret != 0 ) {
		return ret;
	}

	// ����L���`�F�b�N
// GG129000(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)
//	if ( isSeasonCardType( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo ) ) {
	if ( isSeasonCardType( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo ) ) {
// GG129000(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���P�A��No.79)
		// ����`�F�b�N
		season_chk_result = (uchar)cal_cloud_season_check();
		if( season_chk_result == SEASON_CHK_OK ){
			// ����ݒ�
			ret = cal_cloud_season_set();
			if ( ret != 0 ) {
				return ret;
			}
			if ( OPECTL.PassNearEnd == 1 ) {
				season_chk_result = 100;
			}
		}
	}

	// ���ɏ��Ŏ�M�������Z�����Z�b�g
	Set_PayInfo_from_InCarInfo();

	// �����ݒ�
	ret = cal_cloud_discount_set();

	return ret;
}
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

// GG129000(S) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
/**
 * @brief QR���Ԍ��t���̎��؂̔��s�����`�F�b�N
 * 
 * @param[in]		check_only	: 1=�`�F�b�N�̂�
 * @return 0:QR���Ԍ����s�Ȃ� 1:QR���Ԍ��������s 2:QR���Ԍ����s���� 3:QR���Ԍ��������s(����)
 * 
 * @date			2023/02/10
 */
uchar QRIssueChk(uchar check_only)
{
	uchar	ret = 0;
	uchar	qriss_flg1, qriss_flg2;
	long	prm;

	// �̎��؂͋������s����ݒ�?
	qriss_flg1 = 0;
// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//	if( prm_get(COM_PRM, S_LCD, 139, 1, 2) != 0 )
	if( prm_get(COM_PRM, S_RTP, 57, 1, 2) != 0 )
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
	{
		// �������s���Ȃ�
		qriss_flg1 = 0;
	}
	else
	{
// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//		if( prm_get(COM_PRM, S_LCD, 139, 1, 1) != 3 )
		if( prm_get(COM_PRM, S_RTP, 57, 1, 1) != 3 )
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
		{
			// QR�R�[�h�̎��ؔ��s���Ȃ��ݒ�ȊO�̎�
			// �������s����
			qriss_flg1 = 1;
		}
	}

	// ����̐��Z��QR���Ԍ����s�����ɓ��ěƂ܂������m�F����B
	qriss_flg2 = 0;
	// �̎��ؔ��s����
// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//	if( prm_get(COM_PRM, S_LCD, 139, 1, 1) == 0 )
	if( prm_get(COM_PRM, S_RTP, 57, 1, 1) == 0 )
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
	{
		qriss_flg2 = 2;
	}

// GG129000(S) A.Shirai 2023/9/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5696�F#7043���Ɏ��������Ő��Z�������Ă��̎��؂��������s����Ȃ��j
// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//	if (prm_get(COM_PRM, S_LCD, 139, 1, 1) == 1)
	if (prm_get(COM_PRM, S_RTP, 57, 1, 1) == 1)
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
	{
		if (OPECTL.f_searchtype == SEARCH_TYPE_TIME) {
			qriss_flg2 = 1;
		}
	}
// GG129000(E) A.Shirai 2023/9/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5696�F#7043���Ɏ��������Ő��Z�������Ă��̎��؂��������s����Ȃ��j
	// �ԔԔF�����s�����ԗ��Ő��Z���������ꍇ��QR���s���s���B
// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//	if( prm_get(COM_PRM, S_LCD, 139, 1, 1) == 2 )
	if( prm_get(COM_PRM, S_RTP, 57, 1, 1) == 2 )
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
	{
		// ���Ɏ��Ԕԏ��擾�ɐ������Ă����ꍇ�AQR�R�[�h���󎚂��Ȃ�
		qriss_flg2 = CarNumChk();
	}

	// QR���Ԍ��g�p����QR�󎚑ΏۊO
	if( PayData.CarSearchFlg == 2 &&
		lcdbm_rsp_in_car_info_main.shubetsu == 0)
	{
		qriss_flg2 = 0;
	}

	// �ݒ�͈͊O���������ꍇ��QR�󎚑ΏۊO
	// �̎��؂̎������s
// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//	prm = prm_get(COM_PRM, S_LCD, 139, 1, 2);
	prm = prm_get(COM_PRM, S_RTP, 57, 1, 2);
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
	if ( 1 < prm )
	{
		// �͈͊O
		qriss_flg2 = 0;
		ret = 0;
	}

	// �̎��؂̔��s����
// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//	prm = prm_get(COM_PRM, S_LCD, 139, 1, 1);
	prm = prm_get(COM_PRM, S_RTP, 57, 1, 1);
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
// GG129000(S) A.Shirai 2023/9/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5696�F#7043���Ɏ��������Ő��Z�������Ă��̎��؂��������s����Ȃ��j
//	if ( (3 < prm) || (prm == 1) )
	if (3 < prm)
// GG129000(E) A.Shirai 2023/9/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5696�F#7043���Ɏ��������Ő��Z�������Ă��̎��؂��������s����Ȃ��j
	{
		// �͈͊O
		qriss_flg2 = 0;
		ret = 0;
	}

	// �������s�ݒ肠��&QR���s�������v
	if( (qriss_flg1 == 1) && (qriss_flg2 == 1) )
	{
		ret = 1;
	}
	// �������s�ݒ�Ȃ�&QR���s�������v
	else if( (qriss_flg1 == 0) && (qriss_flg2 == 1) )
	{
		ret = 2;
	}
	// �������s�ݒ肠��&QR���s�������v
	else if( (qriss_flg1 == 1) && (qriss_flg2 == 2) )
	{
		ret = 3;
	}

	if (check_only) {
		return ret;
	}

	switch (ret) {
	case 1:
		// �̎��؃{�^����������Ă��Ȃ���΋������s�������s��
// GG129000(S) M.Fujikawa 2023/10/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7199�Ή�
// GG129000(S) M.Fujikawa 2023/10/20 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7170
////		if( OPECTL.RECI_SW == 0 ){											// �̎������ݖ��g�p?
//		if( OPECTL.RECI_SW == 0 && 
//		  ((!IS_INVOICE && Ope_isPrinterReady() == 1) || 
//		  (IS_INVOICE && (Ope_isJPrinterReady() == 1 && paperchk2() == 0 && Ope_isPrinterReady() == 1)))){
// GG129000(E) M.Fujikawa 2023/10/20 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7170
		if( OPECTL.RECI_SW == 0 ){											// �̎������ݖ��g�p?
// GG129000(E) M.Fujikawa 2023/10/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7199�Ή�
			OPECTL.RECI_SW = 1;												// �̎������ݎg�p
// GG129000(S) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5684�F�̎��ؕK�����s�ݒ�ŗ̎��ؔ��s�����ꍇ��NT-NET���Z�f�[�^�̗̎��ؔ��s�L����0���̎��؂Ȃ��ɂȂ��Ă���j
			ryo_buf.ryos_fg = 1;
// GG129000(E) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5684�F�̎��ؕK�����s�ݒ�ŗ̎��ؔ��s�����ꍇ��NT-NET���Z�f�[�^�̗̎��ؔ��s�L����0���̎��؂Ȃ��ɂȂ��Ă���j
			PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );		// ����ʒm���M
		}
		// �̎��؎󂯎��|�b�v�A�b�v�͕K���\������
		lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQR, 0 );						// �̎��؎󂯎��\��(���s�{�^������)
		break;
	case 2:
		// �{�^����������Ă��Ȃ��ꍇ�́A�{�^���t���|�b�v�A�b�v��\��������B
		if( OPECTL.RECI_SW == 0 ){											// �̎������ݖ��g�p?
			lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQRBUTTON, 0 );				// �̎��؎󂯎��\��(���s�{�^���L��)
		}
		else
		{
			// ���Ƀ{�^����������Ă���ꍇ�̓{�^�������̃|�b�v�A�b�v��\������B
			lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQR, 0 );					// �̎��؎󂯎��\��(���s�{�^������)
		}
		break;
	case 3:
		// �̎��؃{�^����������Ă��Ȃ���΋������s�������s��
// GG129000(S) M.Fujikawa 2023/10/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7199�Ή�
// GG129000(S) M.Fujikawa 2023/10/20 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7170
////		if( OPECTL.RECI_SW == 0 ){											// �̎������ݖ��g�p?
//		if( OPECTL.RECI_SW == 0 && 
//		  ((!IS_INVOICE && Ope_isPrinterReady() == 1) || 
//		  (IS_INVOICE && (Ope_isJPrinterReady() == 1 && paperchk2() == 0 && Ope_isPrinterReady() == 1)))){
// GG129000(E) M.Fujikawa 2023/10/20 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7170
		if( OPECTL.RECI_SW == 0 ){											// �̎������ݖ��g�p?
// GG129000(E) M.Fujikawa 2023/10/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7199�Ή�
			OPECTL.RECI_SW = 1;												// �̎������ݎg�p
// GG129000(S) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5684�F�̎��ؕK�����s�ݒ�ŗ̎��ؔ��s�����ꍇ��NT-NET���Z�f�[�^�̗̎��ؔ��s�L����0���̎��؂Ȃ��ɂȂ��Ă���j
			ryo_buf.ryos_fg = 1;
// GG129000(E) A.Shirai 2023/9/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5684�F�̎��ؕK�����s�ݒ�ŗ̎��ؔ��s�����ꍇ��NT-NET���Z�f�[�^�̗̎��ؔ��s�L����0���̎��؂Ȃ��ɂȂ��Ă���j
			PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );		// ����ʒm���M
		}
		// �̎��؎󂯎��|�b�v�A�b�v�͕K���\������
		// �Ԕԏ��擾�L��
		if ( CarNumChk() ){
			// �擾NG
			lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQR, 0 );					// �̎��؎󂯎��\��(���s�{�^������)
		}else{
			// �擾OK
			lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQRNORMAL, 0 );				// �̎��؎󂯎��\��(����)
		}
		break;
	case 0:
		// QR���s�����𖞂����Ă��Ȃ����߁A�\���Ȃ���ʒm����(�ē������p�^�[�������Ɏg�p)
		lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQRNONE, 0 );					// �̎��؎󂯎��\�����Ȃ�
		break;
	}

	return ret;
}

uchar	CarNumChk(void)
{
	uchar	ret = 0;	// 0:�擾OK,1:�擾NG

	// ���Ɏ��Ԕԏ��擾�ɐ������Ă����ꍇ�AQR�R�[�h���󎚂��Ȃ�
	if( PayData.MediaKind2 == CARD_TYPE_CAR_NUM )
	{
		// �Ԕԏ�񂠂�̏ꍇ�A�e���ڂ̒l��0�ɂȂ��Ă��Ȃ����m�F����
		// ���ׂĒl������Ώ��擾�����Ɣ��f���A�󎚂��Ȃ�
		if( ((PayData.MediaCardNo2[0] != 0x30) || (PayData.MediaCardNo2[1] != 0x30)) &&	// ���^�x�ǖ�
			((PayData.MediaCardNo2[2] != 0x30) || (PayData.MediaCardNo2[3] != 0x30) ||	// ���ޔԍ�
			 (PayData.MediaCardNo2[4] != 0x30)) &&
			((PayData.MediaCardNo2[5] != 0x30) || (PayData.MediaCardNo2[6] != 0x30)) && // �p�r����
			((PayData.MediaCardNo2[7] != 0x30) || (PayData.MediaCardNo2[8] != 0x30) ||	// ��A�ԍ�
			 (PayData.MediaCardNo2[9] != 0x30) || (PayData.MediaCardNo2[10] != 0x30)) )
		{
			ret = 0;
		}
		else
		{
			ret = 1;
		}
	}
	else
	{
		ret = 1;
	}
	
	return ret;
}
// GG129000(E) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j

// GG129000(S) H.Fujinaga 2023/02/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//[]----------------------------------------------------------------------[]
///	@brief			���u���Z(���Z���ύX)
//[]----------------------------------------------------------------------[]
///	@param[in]		void		: 
///	@return			ret			: 0=�ΏۊO(���u���Z�ł͂Ȃ�)
///								: 1=����(���u���Z�J�n)
///								: 2=2��ڈȍ~(QR����)
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2023/02/24
///					Update	:
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
uchar	PayInfoChange_StateCheck(void)
{
	uchar ret = 0;
	uchar shubetsu = lcdbm_rsp_in_car_info_main.shubetsu;
	uchar state = lcdbm_rsp_in_car_info_main.data.PayInfoChange.state;

	if(shubetsu == 2 && state == 1){
		// ���u���Z������
		ret = 1;
	}else if(shubetsu == 2 && state == 2){
		// ���u���Z����2��ڈȍ~(QR����)
		ret = 2;
	}

	return ret;
}
// GG129000(E) H.Fujinaga 2023/02/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j