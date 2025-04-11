/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ۯ����u����֘Aͯ�ް̧��																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      : K.Akiba																					   |*/
/*| Date        : 2005-04-28																				   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#ifndef _FLP_DEF_H_
#define _FLP_DEF_H_

// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#include	"system.h"
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#include	"mem_def.h"
																				/*								*/
#define		LK_TYPE_AIDA1		1												/* �p�c���]��					*/
#define		LK_TYPE_AIDA2		2												/* �p�c�޲�						*/
#define		LK_TYPE_YOSHI		3												/* �g��							*/
#define		LK_TYPE_KOMUZ		4												/* �ѽ�							*/
#define		LK_TYPE_HID			5												/* HID							*/
																				/* �ׯ�ߑ��u���				*/
#define		LK_TYPE_AIDA_FLP	11												/* �p�c���ׯ��					*/
#define		LK_TYPE_HOUWA_FLP	12												/* �M�a���ׯ��					*/
#define		LK_TYPE_EIKOU_FLP	13												/* �h�W���ׯ��					*/
#define		LK_TYPE_MEITO_FLP	14												/* ҲĐ��ׯ��					*/
#define		LK_TYPE_SANICA_FLAP		15											/* �T�j�J�t���b�v				*/
#define		LK_TYPE_CONTACT_FLAP	16											/* �ړ_�t���b�v					*/
#define		LK_TYPE_AIDA_FLP_CUSTOM			31									/* �p�c���ׯ��(��~3�b)			*/
#define		LK_TYPE_AIDA_FLP_CUSTOM_COLD	35									/* �p�c���ׯ��(��~3�b�F����n�d�l)		*/

																				/* ���u���						*/
#define		LK_KIND_ERR			0												/* �s��							*/
#define		LK_KIND_FLAP		1												/* �ׯ�ߑ��u					*/
#define		LK_KIND_LOCK		2												/* ۯ����u						*/
#define		LK_KIND_INT_FLAP	3												/* �����ׯ�ߑ��u				*/

																				/* ���uNo.�͈�					*/
#define		FLAP_START_NO		CAR_START_INDEX+1								/* �ׯ��	�J�nNo.				*/
#define		FLAP_END_NO			CAR_LOCK_MAX									/* �ׯ��	�I��No.				*/
#define		LOCK_START_NO		BIKE_START_INDEX+1								/* ۯ����u	�J�nNo.				*/
#define		LOCK_END_NO			BIKE_START_INDEX+BIKE_LOCK_MAX					/* ۯ����u	�I��No.				*/
#define		INT_FLAP_START_NO	INT_CAR_START_INDEX+1							/* �ׯ��	�J�nNo.				*/
#define		INT_FLAP_END_NO		INT_CAR_START_INDEX + INT_CAR_LOCK_MAX			/* �ׯ��	�I��No.				*/
																				/*								*/
#define		LOCK_MAKER_CNT		6												/* ۯ����u�̎�ސ�				*/
																				/*								*/
// �t���b�v��ԃf�[�^
enum {								// ���b�N���
	FLP_LOCK_WAIT = '0',			// �ҋ@��
	FLP_LOCK_DOWN,					// ���~��
	FLP_LOCK_UP,					// �㏸��
	FLP_LOCK_DOWN_ERR,				// ���~�G���[
	FLP_LOCK_UP_ERR,				// �㏸�G���[
	FLP_LOCK_FORCE_DOWN,			// �������~
	FLP_LOCK_FORCE_UP,				// �����㏸
	FLP_LOCK_RESERVED,				// �\��i���g�p�j
	FLP_LOCK_INVALID,				// �s�����b�N
};

// �t���b�v�������݃f�[�^
enum {								// �������݃f�[�^
	FLP_COMMAND_FORCE_OFF = 1,		// ����OFF
	FLP_COMMAND_FORCE_ON = 2,		// ����ONJ
	FLP_COMMAND_UP = 4,				// �t���b�v�㏸
	FLP_COMMAND_DOWN = 5,			// �t���b�v���~
};
union	bit_reg {																/*								*/
	uchar	BYTE;																/*								*/
	struct	bt_tag {															/*								*/
		uchar	YOBI07	: 1 ;													/*								*/
		uchar	SYUUS	: 1 ;													/*								*/
		uchar	FURIK	: 1 ;													/*								*/
		uchar	FUKUG	: 1 ;													/*								*/
		uchar	YOBI03	: 1 ;													/*								*/
		uchar	YOBI02	: 1 ;													/*								*/
		uchar	YOBI01	: 1 ;													/*								*/
		uchar	LAGIN   : 1 ;													/*								*/
	} BIT;																		/*								*/
};																				/*								*/
typedef union bit_reg	FLP_BIT;												/*								*/

union	bits_reg {																/*								*/
	struct	bit_tag {															/*								*/
		ushort	b15	: 1 ;														/*								*/
		ushort	b14	: 1 ;														/*								*/
		ushort	b13	: 1 ;														/*								*/
		ushort	b12	: 1 ;														/*								*/
		ushort	b11	: 1 ;														/*								*/
		ushort	b10	: 1 ;														/*								*/
		ushort	b09	: 1 ;														/*								*/
		ushort	b08	: 1 ;														/*								*/
		ushort	b07	: 1 ;														/*								*/
		ushort	b06	: 1 ;														/*								*/
		ushort	b05	: 1 ;														/*								*/
		ushort	b04	: 1 ;														/*								*/
		ushort	b03	: 1 ;														/*								*/
		ushort	b02	: 1 ;														/*								*/
		ushort	b01	: 1 ;														/*								*/
		ushort	b00	: 1 ;														/*								*/
	} bits;																		/*								*/
	ushort	word;																/*								*/
};																				/*								*/
typedef union bits_reg	BITS;													/*								*/
																				/*								*/
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
union	bits_reg_byte {															/*								*/
	struct	bit_tag_byte {															/*								*/
		uchar	b07	: 1 ;														/*								*/
		uchar	b06	: 1 ;														/*								*/
		uchar	b05	: 1 ;														/*								*/
		uchar	b04	: 1 ;														/*								*/
		uchar	b03	: 1 ;														/*								*/
		uchar	b02	: 1 ;														/*								*/
		uchar	b01	: 1 ;														/*								*/
		uchar	b00	: 1 ;														/*								*/
	} BIT;																		/*								*/
	uchar	byte;																/*								*/
};																				/*								*/
typedef union bits_reg_byte	BIT;												/*								*/
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
/* �p�c���t���b�v ���~���b�N�G���[�������̃p�^�p�^��ԕی쏈���p(n��m�񃊃g���C�p) */
typedef union{
	uchar  BYTE;
	struct{
		uchar	EXEC:1;					// Bit7�@�F1=���g���C�������s��
		uchar	TO_MSG:1;				// Bit6  �F1=�^�C���A�E�g���b�Z�[�W���M�ς݁i�^�C�}�J�n����0�j
		uchar	YOBI:4;					// Bit2-5�F�\��
		uchar	RETRY_START_ERR:1;		// Bit1�@�F1=E1141�i���g���C�����J�n�G���[�j������
		uchar	RETRY_OVER_ERR:1;		// Bit0�@�F1=E1140�i���g���C�I�[�o�[�G���[�j������
	}BIT;								// ��E1140,E1141�����󋵂͂����ɂ����o�^����B
										//   �\���͖����� ERR_CHK[]�ɂ͔��f���Ȃ��B
} flp_FLAG_INFO;

typedef struct{
	ushort			TimerCount;			// ���~�w�����s�C���^�[�o�����ԁin���Fx500ms)�Ff_info.BIT.EXEC==1 ���̂ݗL��
	uchar  			RetryCount;			// ���g���C���s�񐔁i���~�w�����M�񐔁j�im��F0 �` 03-0074�D�E�j
	flp_FLAG_INFO 	f_info;
} t_flp_DownLockErrInfo;
extern	t_flp_DownLockErrInfo	flp_DownLockErrInfo[LOCK_MAX];

enum{
	FLAP_CTRL_MODE1=0,	// ���~�ς�(�ԗ��Ȃ�)�̂Ƃ��̲���ď���
	FLAP_CTRL_MODE2,	// ���~�ς�(�ԗ�����FT���Ē�)�̂Ƃ��̲���ď���
	FLAP_CTRL_MODE3,	// �㏸���쒆�̂Ƃ��̲���ď���
	FLAP_CTRL_MODE4,	// �㏸�ς�(���Ԓ�)�̂Ƃ��̲���ď���
	FLAP_CTRL_MODE5,	// ���~���쒆�̂Ƃ��̲���ď���
	FLAP_CTRL_MODE6,	// ���~�ς�(�ԗ�����LT���Ē�)�̂Ƃ��̲���ď���
};
																				/*------------------------------*/
typedef	struct	flp_dt_com {													/* Flap Car Data				*/
																				/*------------------------------*/
	ushort		mode;															/* Flap Mode					*/
																				/*------------------------------*/
	BITS		nstat;															/*								*/
	BITS		ostat;															/*								*/
//																				/* Bit 15 reserve				*/
//																				/* Bit 14 reserve				*/
// ����ǉ�����14�E15�̈����Ƃ��ẮABIT15��BIT9�Ɠ����Ӗ������������׸ނɂȂ��Ă��܂����A
// BIT9���ׯ��(ۯ�)�����ް�����M����ƁA�l��ر���Ă��܂��̂ŁA���u����̋����o�ɂ����������ǂ�����
// ���肷�邽�߂ɒǉ��B
// BIT14�͉��u����̗v����ۯ����䌋�ʂ𑗐M����K�v���������ꍇ��MAF�ɑ΂��đ��M����̂��A
// NT-NET�ɑ΂��đ��M����̂��𔻒肷�邽�߂Ɏg�p����B
																				/* Bit 15 ���u����̃t���b�v����v��	*/
																				/* Bit 14 �����o�Ɏ���ۯ����䌋�ʃf�[�^���M�p */
																				/* Bit 13 ���~�ُ��׸�			*/
																				/* Bit 12 �㏸�ُ��׸�			*/
																				/* Bit 11 reserve				*/
																				/* Bit 10 reserve				*/
																				/* Bit  9 0:�ʏ� 1:NT-NET		*/
																				/* Bit  8 0:�ڑ����� 1:�ڑ��Ȃ�	*/
																				/* Bit  7 0:�ʏ� 1:����ݽ		*/
																				/* Bit  6 0:�s���ر 1:����		*/
																				/* Bit  5 0:���~ۯ����� 1:����	*/
																				/* Bit  4 0:�㏸ۯ����� 1:����	*/
																				/* Bit  3 0:�����o�ɂȂ� 1:����	*/
																				/* Bit  2 0:���~���� 1:�㏸����	*/
																				/* Bit  1 0:���~�ς� 1:�㏸�ς�	*/
																				/* Bit  0 0:�ԗ��Ȃ� 1:�ԗ�����	*/
																				/*------------------------------*/
	ushort		ryo_syu;														/* �������(������Z�p)			*/
																				/*------------------------------*/
	ushort		year;															/* ���ɔN						*/
																				/*------------------------------*/
	uchar		mont;															/* ���Ɍ�						*/
																				/*------------------------------*/
	uchar		date;															/* ���ɓ�						*/
																				/*------------------------------*/
	uchar		hour;															/* ���Ɏ�						*/
																				/*------------------------------*/
	uchar		minu;															/* ���ɕ�						*/
																				/*------------------------------*/
	ushort		passwd;															/* �߽ܰ��						*/
																				/*------------------------------*/
	ushort		uketuke;														/* ��t�����s�׸�				*/
																				/*------------------------------*/
	ushort		u_year;															/* ��t�����s�N					*/
																				/*------------------------------*/
	uchar		u_mont;															/* ��t�����s��					*/
																				/*------------------------------*/
	uchar		u_date;															/* ��t�����s��					*/
																				/*------------------------------*/
	uchar		u_hour;															/* ��t�����s��					*/
																				/*------------------------------*/
	uchar		u_minu;															/* ��t�����s��					*/
																				/*------------------------------*/
	ushort		bk_syu;															/* ���(���~,�C���p)			*/
																				/*------------------------------*/
	ushort		s_year;															/* ���Z�����N					*/
																				/*------------------------------*/
	uchar		s_mont;															/* ���Z������					*/
																				/*------------------------------*/
	uchar		s_date;															/* ���Z������					*/
																				/*------------------------------*/
	uchar		s_hour;															/* ���Z������					*/
																				/*------------------------------*/
	uchar		s_minu;															/* ���Z������					*/
																				/*------------------------------*/
	FLP_BIT		lag_to_in;														/* ׸���ϰ��ѱ�Ăɂ��ē����׸�*/
																				/*------------------------------*/
	uchar		issue_cnt;														/* ���ԏؖ������s��			*/
																				/*------------------------------*/
	ushort		bk_wmai;														/* �g�p����(���~,�C���p)		*/
																				/*------------------------------*/
	ulong		bk_wari;														/* �������z(���~,�C���p)		*/
																				/*------------------------------*/
	ulong		bk_time;														/* �������Ԑ�(���~,�C���p)		*/
																				/*------------------------------*/
	ushort		bk_pst;															/* ��������(���~,�C���p)		*/
																				/*------------------------------*/
	short		in_chk_cnt;														/* ���ɔ���J�E���^				*/
																				/*------------------------------*/
	long		timer;															/* Timer( 500ms unit )			*/
	uchar		car_fail;														/* �Ԏ��̏�						*/
																				/* 0:�Ԏ��̏ᖳ�����			*/
																				/* 1:�Ԏ��̏Ⴀ����			*/
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
																				/* Bit  7 �\��					*/
																				/* Bit  6 �\��					*/
																				/* Bit  5 �\��					*/
																				/* Bit  4 �\��					*/
																				/* Bit  3 �\��					*/
																				/* Bit  2 �\��					*/
																				/* Bit  1 ��������2 0:�Ȃ� 1:����*/
	BIT			flp_state;														/* Bit  0 ��������1 0:�Ȃ� 1:����*/
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
	//	   �����o�͍Ō���ɒǉ����A�r���Ƀ����o��}�����Ȃ�����
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//	ulong		Dummy[10];														/* �����g���p�̗\���G���A   	*/
	uchar		Dummy1[3];														/* �����g���p�̗\���G���A   	*/
	ulong		Dummy[9];														/* �����g���p�̗\���G���A   	*/
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
																				/*------------------------------*/
}	flp_com;																	/* 13 Byte						*/
																				/*------------------------------*/
																				/*								*/
struct	FLPCTL_rec {															/*								*/
																				/*------------------------------*/
	uchar		Comd_knd;														/* ү���޺���ގ��				*/
	uchar		Comd_cod;														/* ү���޺���޺���				*/
	ushort		Room_no;														/* �Ԏ���						*/
	ushort		Ment_flg;														/* 0=����ݽ, 1=NT-NET			*/
	short		Flp_mv_tm[LOCK_MAX];											/* �ׯ�ߔ�(ۯ����u)����Ď���ϰ	*/
	short		Flp_uperr_tm[LOCK_MAX];											/* �㏸ۯ�(�ُ�)�װ������ϰ	*/
	short		Flp_dwerr_tm[LOCK_MAX];											/* ���~ۯ�(�J�ُ�)�װ������ϰ	*/
	flp_com		flp_work;														/* Write Data Work				*/
																				/*------------------------------*/
};																				/*								*/
#define	_FLP_LAGIN		0x0f
#define	_FLP_FUKUGEN	0x10
#define	_FLP_FURIKAE	0x20
																				/*								*/
extern	struct FLPCTL_rec	FLPCTL;												/*								*/
																				/*								*/
typedef	struct	flp_dt_rec {													/*								*/
																				/*------------------------------*/
	flp_com		flp_data[LOCK_MAX];												/* Flap Car Data				*/
																				/*------------------------------*/
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
	//	   �����o�͍Ō���ɒǉ����A�r���Ƀ����o��}�����Ȃ�����
	ushort		yobi;															/* �\��							*/
																				/*------------------------------*/
}	flp_rec;																	/*								*/
																				/*								*/
extern	flp_rec	FLAPDT;															/*								*/

//�|�����ް�̫�ϯ�(���Z���~�p)
typedef struct {
	ushort			mise_no;										// �X�m���D
	uchar			maisuu;											// ����
} kake_tiket;

//���Z���~��� �G���A�a
typedef struct {
	date_time_rec	TInTime;										// ���ɓ���
	ulong			WPlace;											// �t���b�v�ԍ�		2Byte��4Byte�ɕύX
	uchar			sev_tik[15];									// �T�[�r�X�����했�̎g�p����
	kake_tiket		kake_data[5];									// �|�����XNo,�g�p����
	ulong			ppc_chusi_ryo;									// ���Z���~����߲�޶��ޗ���
	uchar			syu;											// ���
	uchar			yobi[30];										// �\��		32��30�ύX
} flp_com_sub;

extern flp_com_sub FLAPDT_SUB[11];												/*0�`9�F���Z���~�ر�@10�F������Z�p���~�ر*/
extern	uchar	Flap_Sub_Flg;													/* B�G���A�t���O				*/
extern	uchar	Flap_Sub_Num;													/* B�G���A�v�f��				*/
																				/*								*/
typedef	struct {																/*								*/
	ulong		LockNo;															/* �����						*/
	ushort		Answer;															/* ����(0=����,1=���ڑ�)		*/
	flp_com		lock_mlt;														/* �Ԏ����						*/
	ulong			ppc_chusi_ryo_mlt;											/* ���Z���~����߲�޶��ޗ���		*/
	uchar			sev_tik_mlt[15];											/* �T�[�r�X�����했�̎g�p����	*/
	kake_tiket		kake_data_mlt[5];											/* �|�����XNo,�g�p����			*/
} lock_multi;																	/*								*/
																				/*								*/
extern	lock_multi	LOCKMULTI;													/* Flap Car Data(������Z�p)		*/
																				/*								*/
extern	ulong	UketukeNoBackup[LOCK_MAX];										/* ��t�����s�ǔ��ޯ�����		*/
																				/*								*/
extern	uchar	SvsTime_Syu[LOCK_MAX];											/* ���Z���̗�����ʁi��ʖ����޽��т̑I���Ɏg�p�j	*/

extern	uchar	Lock_Kind;														/* ۯ����u��ʁi�ׯ�߁^ۯ����u�j*/
																				/*								*/
extern	uchar	DownLockFlag[LOCK_MAX];

// �s��C��(S) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
extern	ushort	FurikaeDestFlapNo;												// �U�֐�t���b�v��(�U�֐�t���b�v���~�w������d�ɑΉ�)
// �s��C��(E) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
extern	uchar	LongParkingFlag[LOCK_MAX];										//�������ԏ�ԊǗ��t���O 0:�������ԏ�ԂȂ��A1:�������ԏ�Ԃ���
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
/*** function prototype ***/													/*								*/
																				/*								*/
/* fcmain.c */																	/*								*/
extern	void	fcmain( void );													/*								*/
extern	char	flp_faz1( ushort );												/*								*/
extern	char	flp_faz2( ushort );												/*								*/
extern	char	flp_faz3( ushort );												/*								*/
extern	char	flp_faz4( ushort );												/*								*/
extern	char	flp_faz5( ushort );												/*								*/
extern	char	flp_faz6( ushort );												/*								*/
extern	ushort	FlpMsg( void );													/*								*/
extern	void	FlpSet( ushort, uchar );										/*								*/
extern	void	flpst_pc( char, char );											/*								*/
extern	void	CarTimer( char, char );											/*								*/
extern	short	InChkTimer( void );
extern	long	LockTimer( void );												/*								*/
extern	long	LagTimer( ushort );												/*								*/
extern	void	FmvTimer( ushort, short );										/*								*/
extern	char	Sens_Chk( char );												/*								*/
extern	char	CarSen_Ck( char );												/*								*/
extern	void	CarTimMng( void );												/*								*/
extern	void	SrvTimMng( void );												/*								*/
extern	void	LkErrTimMng( void );											/*								*/
extern	void	LkTimChk( void );												/*								*/
extern	void	NTNET_Snd_Data05_Sub( void );									/*								*/
extern	uchar	LkKind_Get( ushort );											/*								*/
extern	uchar	m_mode_chk_psl( ushort no );
extern	uchar	m_mode_chk_all( uchar kind );
extern	void Flapdt_sub_clear(ushort no);
char	ope_Furikae_start(ushort no);
char	ope_Furikae_stop(ushort no, char bCancel);
void	ope_Furikae_fukuden(void);
extern	void	ope_SyuseiStart(ushort no);

extern	uchar	flp_DownLock_DownSendEnableCheck( void );
extern	uchar	flp_DownLock_DownSendEnableCheck2( ushort no );
extern	void	flp_DownLock_ErrSet( ushort no, char ErrNo, char knd );
extern	ulong	flp_ErrBinDataEdit( ushort no );
extern	void	flp_DownLock_Initial( void );
extern	void	flp_DownLock_RetryTimerStart( ushort no );
extern	void	flp_DownLock_RetryStop( ushort no );
extern	uchar	flp_DownLock_DownSend( ushort no );
extern	void	flp_DownLock_FlpSet( ushort no );
extern	void	flp_DownLock_lk_err_chk( ushort no, char ErrNo, char kind );
extern	uchar	flp_err_search( ushort, ulong* );
#endif	// _FLP_DEF_H_
