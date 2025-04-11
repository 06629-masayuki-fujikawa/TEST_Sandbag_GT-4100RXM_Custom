/*[]----------------------------------------------------------------------[]*/
/*| Memory Area Definition                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hashimoto                                             |*/
/*| Date        :  2001-10-26                                              |*/
/*[]------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/
#ifndef	___MEM_DEFH___
#define	___MEM_DEFH___

#include	"flashdef.h"
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//#include	"suica_def.h"
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
#include	"pkt_def.h"
#include	"pktctrl.h"
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
extern	SWITCH_DATA	BootInfo;					// �N�����ʑI�����

/*--------------------------------------------------------------------------*/
/*	�ݒ��ް�																*/
/*--------------------------------------------------------------------------*/
struct	Machine_rec {
	uchar			read_dat[12+9];									// ���ذ���ް�
	uchar			sirial_no[12];									// �ره�
	uchar			board_no[12];									// ����
};

extern	struct	Machine_rec	MACHN_INF;								// �@����


/*--------------------------------------------------------------------------*/
/*	��ĳ���ް�ޮ�															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			ver_devi;										// Device  Name
	uchar			ver_file;										// Machine Name
	uchar			ver_part[6];									// Parts No.
	uchar			ver_romn[2];									// Software Version No.00
} ver_rec;

extern	const	ver_rec		VERSNO;									// ROM Table
// GG120600(S) // Phase9 Version�𕪂���
extern	const	ver_rec		VERSNO_BASE;							// Base Table
extern	const	ver_rec		VERSNO_RYOCAL;							// CalDll Table
// GG120600(E) // Phase9 Version�𕪂���


/*--------------------------------------------------------------------------*/
/*	�߽ܰ��ð���															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			pas_word[5];									// RAM  Password
	uchar			pas_trap;										// Trap Flag   00H : OK
																	//            0FFH : NG
	ushort			pas_onda;										// Power On Time Date
	ushort			pas_onhm;										//               hh:mm
	ushort			pas_ofda;										// Power Off Time Date
	ushort			pas_ofhm;										//                hh:mm
} pas_rec;

extern	const pas_rec		PASSDF;									// ROM Address
extern	pas_rec		PASSPT;											// RAM Address


/*--------------------------------------------------------------------------*/
/*	�����׸�																*/
/*--------------------------------------------------------------------------*/
typedef union{
	unsigned char	BYTE;								// Byte
	struct{
		unsigned char	YOBI:1;					// Bit 7	= �\��
		unsigned char	MESSAGEFULL:1;			// Bit 6	= ���b�Z�[�W�o�b�t�@FULL
		unsigned char	PARAM_INVALID_CHK:1;			// Bit 5	= �N�����̃p�����[�^�ύX�������{�t���O 0:�N�����ȊO��������� 1:�N����������
		unsigned char	PARAM_LOG_REC:1;				// Bit 4	= �N�����̐ݒ�X�V����o�^�����t���O 0:�o�^�\��Ȃ� 1:�o�^�\�񂠂�
		unsigned char	ILLEGAL_FLOATINGPOINT:1;		// Bit 3	= Excep_FloatingPoint�p(���������_��O)
		unsigned char	ILLEGAL_SUPERVISOR_INST:1;		// Bit 2	= Excep_SuperVisorInst�p(�������ߗ�O)
		unsigned char	ILLEGAL_INSTRUCTION:1;			// Bit 1	= Excep_UndefinedInst�p (����`���ߗ�O)
		unsigned char	UNKOWN_VECT:1;					// Bit 0	= UnKown_Vect��\�p�̈�
	} BIT;
} t_event_CtrlBitData;

#define		NMI_DAT_MAX	2048

typedef struct {
	ushort		flg_ocd;										// Open Close Receive Date
	ushort		flg_oct;										// Open Close Receive Time
	ushort		flg_ock;										// Open Close Kind (0=���� 1=�c�� 2=�x��)
} RECEIVE_REC;

typedef	struct {
	RECEIVE_REC		receive_rec;
	ushort			nmi_mod;										// nmisave()/nmicler() Mode
	ushort			nmi_siz;										// Size
	void			*nmi_adr;										// Address
	uchar			nmi_dat[NMI_DAT_MAX];							// Save Data
	ushort			nmi_flg;										// nmi�����׸�
	uchar			car_full[4];										// NT-NET�p����ð��

	ushort			memflg;
	t_event_CtrlBitData 	event_CtrlBitData;						// ���荞�ݐ���p�G���[�o�^
																	// BIT 0  �FUnKown_Vect��\�p�̈�
																	// BIT 1  �FExcep_UndefinedInst�p (����`���ߗ�O)
																	// BIT 2  �FExcep_SuperVisorInst�p(�������ߗ�O)
																	// BIT 3  �FExcep_FloatingPoint�p(���������_��O)
} flg_rec;

extern	flg_rec		FLAGPT;

#define	_MEMFLG_VAL		((ushort)0x2b54)			// '+T'

extern	ushort		clr_req;
#define		_CLR_HIFMEM		0x0001
#define		_CLR_CRNMEM		0x0010
#define		_CLR_ALLMEM		(_CLR_HIFMEM|_CLR_CRNMEM)

/*--------------------------------------------------------------------------*/
/* ��ԊĎ�																	*/
/*--------------------------------------------------------------------------*/
struct	AC_FLG {
	char			cycl_fg;										// ���Z�����׸�
	char			syusyu;											// �W�v�����׸�
	char			turi_syu;										// �ޑK�Ǘ������׸�(��d�p)
// MH321800(S) G.So IC�N���W�b�g�Ή�
	char			ec_alarm;										// ���σ��[�_���d�p�t���O
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
	uchar			ec_deemed_fg;									// �݂Ȃ����ϕ��d�p�t���O
// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
	uchar			ec_recv_deemed_fg;								// ���O����f�[�^��M�t���O�i�����t���O�j
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
};

extern	struct	AC_FLG	ac_flg;


/*--------------------------------------------------------------------------*/
/* �����ð���																*/
/*--------------------------------------------------------------------------*/
#define		PAS_MAX		12000										// Pass Card MAX

typedef union{
	uchar	BYTE;													// Byte
	struct{
		uchar	INV:1;												// Bit 7		0:�L��, 1:����
		uchar	PKN:2;												// Bit 5�`6		���p�����ԏ� 0=��{, 1=�g��1, 2=�g��2, 3=�g��3
		uchar	RSV:1;												// Bit 4		�\��
		uchar	STS:4;												// Bit 0�`3		0:�������, 1:�o�ɒ�, 2,���ɒ�
	} BIT;
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
} PAS_TBL;

// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//extern	PAS_TBL		pas_tbl[PAS_MAX];								// ������ð��ð���(12000)
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

/*--------------------------------------------------------------------------*/
/* ������X�Vð���															*/
/*--------------------------------------------------------------------------*/
typedef union{
	uchar	BYTE;													// Byte
	struct{
		uchar	Bt67:2;												// Bit 6,7		���4  0:�X�V����, 1:�X�V�s��
		uchar	Bt45:2;												// Bit 4,5		���3  0:�X�V����, 1:�X�V�s��
		uchar	Bt23:2;												// Bit 2,3		���2  0:�X�V����, 1:�X�V�s��
		uchar	Bt01:2;												// Bit 0,1		���1  0:�X�V����, 1:�X�V�s��
	} BIT;
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
} PAS_RENEWAL;

// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//extern	PAS_RENEWAL	pas_renewal[PAS_MAX/4];							// ������X�Vð���(12000)
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

/*--------------------------------------------------------------------------*/
/* ������o�Ɏ���ð���														*/
/*--------------------------------------------------------------------------*/
#define		PASS_EXTIMTBL_MAX	1000								// Pass Exit Time Tbl MAX

#pragma pack

typedef struct {
	ushort			PassId;											// �����ID
	ulong			ParkNo;											// ���ԏꇂ
	date_time_rec	PassTime;										// �o�Ɏ���
} PASS_EXTBL;
#pragma unpack

typedef struct {
	ushort			Count;											// ������o�Ɏ���ð��ٓo�^����
	PASS_EXTBL		PassExTbl[PASS_EXTIMTBL_MAX];
} PASS_EXTIMTBL;

// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//extern	PASS_EXTIMTBL	pas_extimtbl;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

/*--------------------------------------------------------------------------*/
/* ���춳��	 																*/
/*--------------------------------------------------------------------------*/
enum {
	MOV_PDWN_CNT = 0,													// ���d��
	MOV_PRNT_CNT,														// ������󎚍s��
	MOV_CUT_CNT,														// ��������������
	MOV_JPNT_CNT,														// �ެ���������󎚍s��
	MOV_COIN_RED,														// �d�ݓǎ文��
	MOV_COIN_OUT,														// �d�ݕ��o����
	MOV_NOTE_RED,														// �����ǎ文��
	MOV_NOTE_BOX,														// �������ɔ���� MH544401 GW844900 �������ɃX�^�b�J�J�ʒm�ύX
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//	READ_SHUT_CT,														// ���Cذ�ް����������
//	READ_DO,															// ���Cذ�ް�����
//	READ_YO,															// �ǎ擮���
//	READ_ER,															// �ǎ�װ��
//	READ_WR,															// �������݉�
//	READ_VN,															// ���̧�NG��
//	VPRT_DO,															// ���Cذ�ް����������
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
	LCD_LIGTH_CNT,														// LCD�ޯ�ײē_�����쎞��
// MH810100(S) Y.Yamauchi 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	QR_READER_CNT,														// QRذ�ް���쎞��
// MH810100(E) Y.Yamauchi 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	MOV_CNT_MAX															// ���춳�Đ�
};
extern	ulong	Mov_cnt_dat[MOV_CNT_MAX];								// ���춳��

#pragma pack
/*--------------------------------------------------------------------------*/
/* �ǂ���																	*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ulong			w;			// whole
	ulong			i;			// individual
} ST_OIBAN;
#pragma unpack
// IOLOG_DATA��pack����ST_OIBAN���g�p����ƁA�A���C�����g2�o�C�g�Ɣ��f�����悤�ɂȂ�
// �G���[���o�Ă��܂����߁AIOLOG_DATA�ł�pack���Ă��Ȃ������\����ST_OIBAN2���g�p����
typedef	struct {
	ulong			w;			// whole
	ulong			i;			// individual
} ST_OIBAN2;

/*--------------------------------------------------------------------------*/
/* �C�����Z�p�ް��ر														*/
/*--------------------------------------------------------------------------*/
struct	SYUSEI{														// �C���p�ް�
	uchar			sei;											// �C�����Z��� 0=����/1=�L��/2=����(���޽��эl��)
	ulong			tryo;											// ���X�̒��ԗ���(����܂܂�)
	ulong			ryo;											// ���ԗ���(�������)
	ulong			gen;											// �����̎��z
	ulong			tax;											// ����Ŋz
	ushort			iyear;											// ���Ԏ���
	uchar			imont;											//
	uchar			idate;											//
	uchar			ihour;											//
	uchar			iminu;											//
	ushort			oyear;											// �o�Ԏ���
	uchar			omont;											//
	uchar			odate;											//
	uchar			ohour;											//
	uchar			ominu;											//
	uchar			ot_car;											// 0=�ް��Ȃ�
																	// 1=�ׯ�߉��~׸���ѓ�
																	// 2=�o��
																	// 3=���ԗ������ׯ�ߏ㏸��ѓ�
																	// 4=���ԗ������ׯ�ߏ㏸
																	// 5=׸���ѱ��ߌ��ׯ�ߏ㏸(�ē���)
	uchar			infofg;											// ** �C�����Z����׸� **
																	// b8=(���g�p)			0x80
																	// b7=�̎��ؔ��s		0x40
																	// b6=����߽OFF			0x20
																	// b5=���޽��ѓ��o��	0x10
																	// b4=(���g�p)			0x08
																	// b3=(���g�p)			0x04
																	// b2=(���g�p)			0x02
																	// b1=�S�z����			0x01

	ulong			sy_wari;										// �����������z
	ulong			sy_time;										// ���Ԋ������z
	uchar			sy_wmai;										// �g�p����
	uchar			tei_syu;										// ������(1�`15)
																	// b1�`b6:��������
																	// b7,b8 :���ԏꇂ(0=��{�A1�`3=�g��1�`3)
	ushort			tei_id;											// �l����(1�`12000)
	ushort			tei_sd;											// �L���J�n�N����ɰ�ײ��
	ushort			tei_ed;											// �L���I���N����ɰ�ײ��
	uchar			syubetu;										// �������
	ST_OIBAN		oiban;											// ���Z�ǔ�(�C�����̐��Z�ǔ�)
	ushort			bun_syu1;
	ushort			bun_syu2;										// 1�̈�		0:�Ȃ�/1:�ʏ�/2:�ȏ�/3:�s��
																	// 10�̈�		0:�Ȃ�/1:�ر1/2:�ر2/3:�ر3
																	// 100�`�̈�	0:�Ȃ�/1�`48:1�`48���ޖ�
};
extern	struct	SYUSEI	syusei[LOCK_MAX];

#define		SSS_ZENWARI		0x01									// �S�z����
#define		SSS_SVTIME		0x10									// ���޽��ѓ����Z
#define		SSS_ANTIOFF		0x20									// ����߽OFF
#define		SSS_RYOUSYU		0x40									// �̎��ؔ��s

/*--------------------------------------------------------------------------*/
/* �s���o�ɗp�ް��ر														*/
/*--------------------------------------------------------------------------*/
struct	FUSEI_D {													// �s���o�Ɏԓ��o���ް�
	ushort			iyear;											// ���ԔN
	uchar			imont;											// ���Ԍ�
	uchar			idate;											// ���ԓ�
	uchar			ihour;											// ���Ԏ�
	uchar			iminu;											// ���ԕ�
	ushort			oyear;											// �o�ԔN
	uchar			omont;											// �o�Ԍ�
	uchar			odate;											// �o�ԓ�
	uchar			ohour;											// �o�Ԏ�
	uchar			ominu;											// �o�ԕ�
	ushort			t_iti;											// ���Ԉʒu
	uchar			kyousei;										// �����o���׸�
																	// 0=�ʏ�̕s���o��
																	// 1=�ʏ�̋����o��
																	// 2=�C�����Z�̕s���o��(�䐔���ĂȂ��A�C�����̏C���ޯ̧�̓��o�Ɏ���)
																	// 3=�C�����Z�̕s���o��(�䐔���Ă���A�C�����̌��݂̓��o��)
};

struct	FUSEI_SD {
	struct	FUSEI_D		fus_d[LOCK_MAX];							// �s���ް�LOCK_MAX����
	unsigned short		kensuu;										// ����
};

extern	struct	FUSEI_SD	fusei;
extern	struct	FUSEI_SD	wkfus;									// �Ҕ�ܰ��ر
extern	struct	FUSEI_SD	TempFus;								// �s���E�����o���ް��X�V�pܰ��ر�i�d���f����ۑ��j
extern	struct	FUSEI_SD	locktimeout;							// ���b�N�E�ׯ�ߏ㏸�^�C�}���o�Ƀf�[�^�i�[
extern	struct	FUSEI_SD	wklocktimeout;							// �Ҕ�ܰ��ر


/*--------------------------------------------------------------------------*/
/* �e��W�v�� 																*/
/*--------------------------------------------------------------------------*/
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// #define		WTIK_USEMAX			10									// ���޽��,�|����,������,�񐔌��g�p�\�ő喇��
#define		WTIK_USEMAX			11									// �������g�p�\�ő��ʐ��i1�g�͔��㌔�p�̗\���j
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
#define		PKICHI_KU_CNT		26									// ���Ԉʒu��搔�i1�`26=A�`Z�j
#define		RYOUKIN_SYU_CNT		12									// ������ʐ�
#define		TEIKI_SYU_CNT		15									// �������ʐ�
#define		PKNO_SYU_CNT		4									// ���ԏ�m���D��ʐ�
#define		PKNO_WARI_CNT		2									// �X�m���D�G���A�m��
#define		SERVICE_SYU_CNT		15									// ���޽����ʐ�
#define		MISE_NO_CNT			100									// �X�m���D��
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
#define		T_MISE_NO_CNT		999									// ���X�܊��� �X�m���D��
#define		T_MISE_SYU_CNT		15									// ���X�܊��� ��ʐ�
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
#define		WARIBIKI_SYU_CNT	100									// ������ʐ��i�������j
#define		SIHEI_SYU_CNT		4									// ������ʐ�
#define		COIN_SYU_CNT		4									// ��ݎ�ʐ�
#define		IN_SIG_CNT			3									// ���ɐM�����i�M���P�C�Q�A�R�j
#define		BUNRUI_CNT			48									// ���ޏW�v�敪��
#define		SYUUKEI_SYU_CNT		8									// �W�v��ʐ�
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
#define		DETAIL_SYU_MAX		2									// ���׏��Z�b�g�\��
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
#define		KAIMONO_WARISYU_MAX	10									// ����������ʍő吔
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
#define		EC_BRAND_MAX		10									// �u�����h���ő�l
#define		EC_BRAND_TOTAL_MAX	(EC_BRAND_MAX*2)					// ���u�����h���ő�l
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���

/*--------------------------------------------------------------------------*/
/* ��݋��ɏW�v																*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	ulong			Tryo;											// ��݋��ɑ������z
	ulong			ryo[COIN_SYU_CNT];								// ��݋��ɓ����z�i���했�j
	ulong			cnt[COIN_SYU_CNT];								// ��݋��ɓ������i���했�j
} COIN_SYU_V01;
typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	ulong			Tryo;											// ��݋��ɑ������z
	ulong			ryo[COIN_SYU_CNT];								// ��݋��ɓ����z�i���했�j
	ulong			cnt[COIN_SYU_CNT];								// ��݋��ɓ������i���했�j
	uchar			SeqNo;											// �V�[�P���V����No.
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
} COIN_SYU_V04;
typedef COIN_SYU_V04	COIN_SYU;

extern	COIN_SYU	coin_syu;
extern	COIN_SYU	cobk_syu;

/*--------------------------------------------------------------------------*/
/* �������ɏW�v																*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	ulong			Tryo;											// �������ɑ������z
	ulong			ryo[SIHEI_SYU_CNT];								// �������ɓ����z�i���했�j
	ulong			cnt[SIHEI_SYU_CNT];								// �������ɓ������i���했�j
} NOTE_SYU_V01;
typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	ulong			Tryo;											// �������ɑ������z
	ulong			ryo[SIHEI_SYU_CNT];								// �������ɓ����z�i���했�j
	ulong			cnt[SIHEI_SYU_CNT];								// �������ɓ������i���했�j
	uchar			SeqNo;											// �V�[�P���V����No.
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
} NOTE_SYU_V04;
typedef NOTE_SYU_V04	NOTE_SYU;

extern	NOTE_SYU	note_syu;
extern	NOTE_SYU	nobk_syu;


#pragma pack
/*--------------------------------------------------------------------------*/
/* �ޑK�Ǘ��W�v																*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			Year;											// �N
	uchar			Mon;											// ��
	uchar			Day;											// ��
	uchar			Hour;											// ��
	uchar			Min;											// ��
	uchar			Sec;											// �b
} proc_date;

typedef struct {
// ���ȉ���ushort�l
	ushort			gen_mai;										// ���݁i�ύX�O�j�ۗL����
	ushort			ygen_mai;										// ���݁i�ύX�O�j�ۗL����(�\�~)
	ushort			zen_mai;										// �O��ۗL����
	ushort			yzen_mai;										// �O��ۗL����(�\�~)
	ushort			sin_mai;										// �V�K�ݒ薇��
	ushort			ysin_mai;										// �V�K�ݒ薇��(�\�~)
// ���Ȍ�͐ώZ����̂�ulong�l
	ulong			sei_nyu;										// ���Z����������
	ulong			sei_syu;										// ���Z���o������
	ulong			ysei_syu;										// ���Z���o������(�\�~)
	ulong			hojyu;											// �ޑK��[����
	ulong			hojyu_safe;										// �ޑK��[�����ɔ�������
	ulong			kyosei;											// �������o����
	ulong			ykyosei;										// �������o����(�\�~)
} TURI_kan_rec_V01;

typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	proc_date		ProcDate;										// ��������
	date_time_rec	PayDate;										// ���Z����
	ulong			PayCount;										// ���Z�ǂ���
	uchar			PayClass;										// �����敪
	short			safe_dt[4];										// Coin Safe Count(�R�C�����ɖ���)
	short			nt_safe_dt;										// Note Safe Count(�������ɖ���)
	TURI_kan_rec_V01	turi_dat[4];									// ���K�ް�(4���핪)
	ushort			sub_tube;										// �T�u�`���[�u����
} TURI_KAN_V01;

typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	proc_date		ProcDate;										// ��������
	date_time_rec	PayDate;										// ���Z����
	ulong			PayCount;										// ���Z�ǂ���
	uchar			PayClass;										// �����敪
	short			safe_dt[4];										// Coin Safe Count(�R�C�����ɖ���)
	short			nt_safe_dt;										// Note Safe Count(�������ɖ���)
	TURI_kan_rec_V01	turi_dat[4];								// ���K�ް�(4���핪)
	ushort			sub_tube;										// �T�u�`���[�u����
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			dummy;											// �\��
} TURI_KAN_V02;
typedef struct {
// ���ȉ���ushort�l
	ushort			gen_mai;										// ���݁i�ύX�O�j�ۗL����
	ushort			ygen_mai;										// ���݁i�ύX�O�j�ۗL����(�\�~)
	ushort			zen_mai;										// �O��ۗL����
	ushort			yzen_mai;										// �O��ۗL����(�\�~)
	ushort			sin_mai;										// �V�K�ݒ薇��
	ushort			ysin_mai;										// �V�K�ݒ薇��(�\�~)
// ���Ȍ�͐ώZ����̂�ulong�l
	ulong			sei_nyu;										// ���Z����������
	ulong			sei_syu;										// ���Z���o������
	ulong			ysei_syu;										// ���Z���o������(�\�~)
	ulong			hojyu;											// �ޑK��[����
	ulong			hojyu_safe;										// �ޑK��[�����ɔ�������
	ulong			kyosei;											// �������o����
	ulong			ykyosei;										// �������o����(�\�~)
	uchar			SeqNo;											// �V�[�P���V����No.
} TURI_kan_rec_V04;
typedef TURI_kan_rec_V04	TURI_kan_rec;


typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	proc_date		ProcDate;										// ��������
	date_time_rec	PayDate;										// ���Z����
	ulong			PayCount;										// ���Z�ǂ���
	uchar			PayClass;										// �����敪
	short			safe_dt[4];										// Coin Safe Count(�R�C�����ɖ���)
	short			nt_safe_dt;										// Note Safe Count(�������ɖ���)
	TURI_kan_rec	turi_dat[4];									// ���K�ް�(4���핪)
	ushort			sub_tube;										// �T�u�`���[�u����
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	uchar			SeqNo;											// �V�[�P���V����No.
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
	//	   �����o�͍Ō���ɒǉ����A�r���Ƀ����o��}�����Ȃ�����
	uchar			dummy[3];										// �\��
} TURI_KAN_V04;
typedef TURI_KAN_V04	TURI_KAN;

extern	TURI_KAN	turi_kan;
extern	TURI_KAN	turi_kwk;										// �ҏW�pܰ��ر
extern	TURI_KAN	turi_log_wk;									// ���K�Ǘ����O�Ǐo�����[�N�G���A

struct	turi_dat_rec{												// �ޑK�Ǘ�ܰ�
	ushort			turi_in;										// �ޑK��[����L���׸�
	short			coin_dsp[7];									// �\���pܰ�
	short			dsp_ini[7];										// �\���pܰ������l
	short			coin_sin[7];									// �V�K�����ݒ�ܰ�
	short			incount[4];										// ���າݖ���ܰ�
	short			outcount[7];									// �o�າݖ���ܰ�
	short			pay_safe[4];									// ���Z���̋��ɓ�������
	short			forceout[7];									// �������o����ܰ�
};

extern	struct	turi_dat_rec	turi_dat;

#define		TURIKAN_DEFAULT_NOWAIT	0	// �f�t�H���g�Z�b�g�҂�����
#define		TURIKAN_DEFAULT_WAIT	1	// �f�t�H���g�Z�b�g�҂�
extern	uchar	turi_kan_f_defset_wait;	
#define		TURIKAN_AUTOSTART_NOEXE	0	// �ލ��������s
#define		TURIKAN_AUTOSTART_EXE	1	// �ލ������s�ς�
extern	uchar	turi_kan_f_exe_autostart;	/* �ލ������s�t���O 1�F���s�ς݁A0�F�����s*/

#pragma unpack

#pragma pack
/*--------------------------------------------------------------------------*/
/* ���Ԉʒu�ʏW�v�ر														*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			Genuri_ryo;										// ��������z
	ushort			Seisan_cnt;										// ���Z��
	ushort			Husei_out_cnt;									// �s���o�ɉ�
	ulong			Husei_out_ryo;									// �s���o�ɋ��z
	ushort			Kyousei_out_cnt;								// �����o�ɉ�
	ulong			Kyousei_out_ryo;								// �����o�ɋ��z
	ushort			Uketuke_pri_cnt;								// ��t�����s��
	ushort			Syuusei_seisan_cnt;								// �C�����Z��
	ulong			Syuusei_seisan_ryo;								// �C�����Z���ߋ��z
} LOKTOTAL_DAT;

extern	LOKTOTAL_DAT	wkloktotal;									// ���Ԉʒu�ʏW�v�Ҕ��Eܰ��ر

typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	LOKTOTAL_DAT	loktldat[LOCK_MAX];								// ���Ԉʒu�ʏW�v(���Ԉʒu��)
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
} LOKTOTAL;

struct	LOKTL{
	LOKTOTAL		tloktl;											// ���Ԉʒu�ʏW�v(T�W�v�p)
	LOKTOTAL		gloktl;											// ���Ԉʒu�ʏW�v(GT�W�v�p)
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͍Ō���ɒǉ����邱��!!!
	//	   �r���Ƀ����o��}�����Ȃ�����
};

extern	struct	LOKTL	loktl;
extern	struct	LOKTL	loktlbk;									// ���Ԉʒu�ʏW�v�O��
// MH321800(S) G.So IC�N���W�b�g�Ή�
/*--------------------------------------------------------------------------*/
/* ���σ��[�_�u�����h���W�v�ر 												  */
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			sei_cnt;										// ���Z��������
	ulong			sei_ryo;										//     �����z��
	ulong			alm_cnt;										// �A���[�������������
	ulong			alm_ryo;										//             �����z��
} EC_SYU_DATA;
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
/*--------------------------------------------------------------------------*/
/* �W�v		 																*/
/*--------------------------------------------------------------------------*/
// �s���t���O�ڍח��R
enum {
	fumei_LOST = 0,			// 0:�������Z
//	fumei_FIRST_PASS,		// 1:������
//	fumei_ANTIPASS_OFF,		// 2:�A���`�p�XOFF
//	fumei_FORCE_OUT,		// 3:�����o��
//	fumei_CARD_NG,			// 4:�J�[�h�ُ�
	fumei_UNKNOWN_MAX,		// �s���ڍח��R��
};
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
/*--------------------------------------------------------------------------*/
/* �W�v		 																*/
/*--------------------------------------------------------------------------*/
enum {
	_OBN_WHOLE,				// �ʂ��ǂ���
	_OBN_PAYMENT,			// ���Z�ǂ���
	_OBN_DEPOSIT,			// �a��ؒǂ���
	_OBN_CANCEL,			// ���Z���~�ǂ���
	_OBN_TURIKAN,			// �ޑK�Ǘ����v�ǂ���
	_OBN_COIN_SAFE,			// �R�C�����ɍ��v�ǂ���
	_OBN_NOTE_SAFE,			// �������ɍ��v�ǂ���
	_OBN_KIYOUSEI,			// �����o�ɒǔ�	
	_OBN_FUSEI,				// �s���o�ɒǔ�	
	_OBN_reserve,
	_OBN_T_TOTAL,			// T���v�ǂ���
	_OBN_GT_TOTAL,			// GT���v�ǂ���

	_OBN_AREA_MAX
};

// �d�l�ύX(S) K.Onodera 2016/11/02 �����W�v�f�[�^�t�H�[�}�b�g�Ή�
// ���F�J�[�h���ϋ敪�ƍ��킹�邱��
enum{
	MOD_TYPE_MONEY,
	MOD_TYPE_CREDIT,
	MOD_TYPE_EMONEY,
	MOD_TYPE_MAX
};
// �d�l�ύX(E) K.Onodera 2016/11/02 �����W�v�f�[�^�t�H�[�}�b�g�Ή�

typedef struct {
	ushort			Kikai_no;										// �@�B��
	ushort			Kakari_no;										// �W����
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����
	ulong			oiban_range[_OBN_AREA_MAX][2];					// �e��ǂ��Ԕ͈�
	ulong			Uri_Tryo;										// ������z
	ulong			Genuri_Tryo;									// ����������z
	ulong			Kakeuri_Tryo;									// ���|���z
	ulong			Tax_Tryo;										// ���ŋ��z
	ulong			Seisan_Tcnt;									// �����Z��
	ulong			In_car_Tcnt;									// �����ɑ䐔
	ulong			In_car_cnt[IN_SIG_CNT];							// ���ɑ䐔�i���ɐM���P�A�Q�j
	ulong			Out_car_Tcnt;									// ���o�ɑ䐔
	ulong			Out_car_cnt[IN_SIG_CNT];						// �o�ɑ䐔�i���ɐM���P�A�Q�j
	ulong			Now_car_Tcnt;									// �����ݒ��ԑ䐔
	ulong			Now_car_cnt[IN_SIG_CNT];						// ���ݒ��ԑ䐔�i���ɐM���P�A�Q�j

	ulong			Hunsei_Tcnt;									// �������Z���񐔁�
	ulong			Hunsei_Tryo;									// �@�@�@�@�����z��

	ulong			Rsei_cnt[RYOUKIN_SYU_CNT];						// ������ʖ����Z���񐔁��i������ʂ`�`�k�j
	ulong			Rsei_ryo[RYOUKIN_SYU_CNT];						// �@�@�@�@�@�@�@�����z���i������ʂ`�`�k�j

	ulong			Rtwari_cnt[RYOUKIN_SYU_CNT];					// ������ʖ����Ԋ������񐔁��i������ʂ`�`�k�j
	ulong			Rtwari_ryo[RYOUKIN_SYU_CNT];					// �@�@�@�@�@�@�@�@�@�����z���i������ʂ`�`�k�j

	ulong			Ccrd_sei_cnt;									// �ڼޯĶ��ސ��Z���񐔁�
	ulong			Ccrd_sei_ryo;									// �@�@�@�@�@�@�@�����z��

	ulong			Pcrd_use_cnt[PKNO_SYU_CNT];						// ����߲�޶��ގg�p���񐔁��i��{�A�g���P�C�Q�C�R�j
	ulong			Pcrd_use_ryo[PKNO_SYU_CNT];						// �@�@�@�@�@�@�@�@�����z���i��{�A�g���P�C�Q�C�R�j

	ulong			Ktik_use_cnt[PKNO_SYU_CNT];						// �񐔌��g�p���񐔁��i��{�A�g���P�C�Q�C�R�j
	ulong			Ktik_use_ryo[PKNO_SYU_CNT];						// �@�@�@�@�@�����z���i��{�A�g���P�C�Q�C�R�j

	ulong			Stik_use_cnt[PKNO_SYU_CNT][SERVICE_SYU_CNT];	// ���޽����ʖ��g�p���������i��{�A�g���P�C�Q�C�R�j�i���޽���`�C�a�C�b�j
	ulong			Stik_use_ryo[PKNO_SYU_CNT][SERVICE_SYU_CNT];	// �@�@�@�@�@�@�@�@	�����z���i��{�A�g���P�C�Q�C�R�j�i���޽���`�C�a�C�b�j

	ulong			Mno_use_Tcnt;									// �X�m���D������İ�ف��񐔁�
	ulong			Mno_use_Tryo;									// �@�@�@�@�@�@�@�@�@�����z��
	ulong			Mno_use_cnt1[PKNO_SYU_CNT];						// �X�m���D����İ�ف��񐔁��i��{�A�g���P�C�Q�C�R�j
	ulong			Mno_use_ryo1[PKNO_SYU_CNT];						// �@�@�@�@�@�@�@�@�����z���i��{�A�g���P�C�Q�C�R�j
	ulong			Mno_use_cnt2[PKNO_SYU_CNT];						// �X�m���D1�`100	İ�ي������񐔁��i��{�A�g���P�C�Q�C�R�j
	ulong			Mno_use_ryo2[PKNO_SYU_CNT];						// �@�@�@�@�@�@�@	�@�@�@�@�����z���i��{�A�g���P�C�Q�C�R�j
	ulong			Mno_use_cnt3[PKNO_SYU_CNT];						// �X�m���D101�`255	İ�ي������񐔁��i��{�A�g���P�C�Q�C�R�j
	ulong			Mno_use_ryo3[PKNO_SYU_CNT];						// �@�@�@�@�@�@�@�@	�@�@�@�@�����z���i��{�A�g���P�C�Q�C�R�j
	ulong			Mno_use_cnt4[PKNO_WARI_CNT][MISE_NO_CNT];		// �X�m���D���������񐔁��i��{�A�g���P�j�i�X�m���D�P�`�P�O�O�j
	ulong			Mno_use_ryo4[PKNO_WARI_CNT][MISE_NO_CNT];		// �@�@�@�@�@�@�@�����z���i��{�A�g���P�j�i�X�m���D�P�`�P�O�O�j

	ulong			Wtik_use_Tcnt;									// �������g�p��İ�ف��񐔁�
	ulong			Wtik_use_Tryo;									// �@�@�@�@�@�@�@�@�����z��
	ulong			Wtik_use_cnt1[PKNO_SYU_CNT];					// �������g�pİ�ف��񐔁��i��{�A�g���P�C�Q�C�R�j
	ulong			Wtik_use_ryo1[PKNO_SYU_CNT];					// �@�@�@�@�@�@�@�����z���i��{�A�g���P�C�Q�C�R�j
	ulong			Wtik_use_cnt2[PKNO_WARI_CNT][WARIBIKI_SYU_CNT];	// ������������ʖ��g�p���񐔁��i��{�A�g���P�j�i������ʂP�`�P�O�O�j
	ulong			Wtik_use_ryo2[PKNO_WARI_CNT][WARIBIKI_SYU_CNT];	// �@�@�@�@�@�@�@�@�@�@�����z���i��{�A�g���P�j�i������ʂP�`�P�O�O�j

	ulong			Teiki_use_cnt[PKNO_SYU_CNT][TEIKI_SYU_CNT];		// �������ʖ��g�p���񐔁��i��{�A�g���P�C�Q�C�R�j�i����P�`�P�T�j
	ulong			Teiki_kou_cnt[PKNO_SYU_CNT][TEIKI_SYU_CNT];		// �������ʖ��X�V���񐔁��i��{�A�g���P�C�Q�C�R�j�i����P�`�P�T�j
	ulong			Teiki_kou_ryo[PKNO_SYU_CNT][TEIKI_SYU_CNT];		// �@�@�@�@�@�@�@�@�����z���i��{�A�g���P�C�Q�C�R�j�i����P�`�P�T�j

	ulong			Ryosyuu_pri_cnt;								// �̎��ؔ��s��
	ulong			Azukari_pri_cnt;								// �a��ؔ��s��
	ulong			Uketuke_pri_Tcnt;								// ��t�����s��

	ulong			In_svst_seisan;									// �T�[�r�X�^�C�������Z��
	ulong			Apass_off_seisan;								// �A���`�p�X�n�e�e���Z��

	ulong			Harai_husoku_cnt;								// ���o�s�����񐔁�
	ulong			Harai_husoku_ryo;								// �@�@�@�@�����z��

	ulong			Kinko_Tryo;										// ���ɑ������z
	ulong			Note_Tryo;										// �������ɑ������z
	ulong			Note_ryo[SIHEI_SYU_CNT];						// �������ɓ����z�i���했�j
	ulong			Note_cnt[SIHEI_SYU_CNT];						// �������ɓ������i���했�j

	ulong			Coin_Tryo;										// ��݋��ɑ������z
	ulong			Coin_ryo[COIN_SYU_CNT];							// ��݋��ɓ����z�i���했�j
	ulong			Coin_cnt[COIN_SYU_CNT];							// ��݋��ɓ������i���했�j
																	// �����@���K�f�[�^�@����
	ulong			tounyu;											// ���Z�������z�@���z
	ulong			tou[5];											// ���Z�������z�@�e���했
	ulong			hojyu;											// �ޑK��[���z�@���z
	ulong			hoj[4];											// �ޑK��[���z�@�e���했
	ulong			seisan;											// ���Z���o���z�@���z
	ulong			sei[4];											// ���Z���o���z�@�e���했
	ulong			kyosei;											// �������o���z�@���z
	ulong			kyo[4];											// �������o����@�e���했

	ulong			Sihei_out_Tryo;									// �������o���z
	ulong			Sihei_out_cnt[SIHEI_SYU_CNT];					// �@�@�@�@�����i���했�j
	ulong			Sihei_now_Tryo;									// �������o�@���ݕۗL���z
	ulong			Sihei_now_cnt[SIHEI_SYU_CNT];					// �@�@�@�@�@�@�@�@�@�����i���했�j

	ulong			Junkan_goukei_cnt;								// �z�R�C�����v��
	ulong			Siheih_goukei_cnt;								// �������o�@���v��
	ulong			Ckinko_goukei_cnt;								// �R�C�����ɍ��v��
	ulong			Skinko_goukei_cnt;								// �������ɍ��v��

	ulong			Turi_modosi_ryo;								// �ޑK���ߊz

	ulong			Seisan_chusi_cnt;								// ���Z���~���񐔁�
	ulong			Seisan_chusi_ryo;								// �@�@�@�@�����z��

	ulong			Husei_out_Tcnt;									// �s���o�ɉ�
	ulong			Husei_out_Tryo;									// �s���o�ɋ��z

	ulong			Kyousei_out_Tcnt;								// �����o�ɉ�
	ulong			Kyousei_out_Tryo;								// �����o�ɋ��z

	ulong			Syuusei_seisan_Tcnt;							// �C�����Z��
	ulong			Syuusei_seisan_Tryo;							// �C�����Z���ߋ��z

//�u���ޏW�v�f�[�^�v3���ޑΉ�
	ulong			Bunrui1_cnt[3][BUNRUI_CNT];						// ���ޏW�v�P�F�S�W�敪�̑䐔�i�u���ɁE���Z �����ʏW�v�v���F���ɑ䐔�j
																	// �@�@�@�@�@�@�@�@�@�@�@�@�@�i�u���ɁE�o�� �����ʏW�v�v���F���ɑ䐔�j
	ulong			Bunrui1_ryo[3][BUNRUI_CNT];						// ���ޏW�v�P�F�S�W�敪�̗����i�u���ɁE���Z �����ʏW�v�v���F���Z�䐔�j
																	// �@�@�@�@�@�@�@�@�@�@�@�@�@�i�u���ɁE�o�� �����ʏW�v�v���F�o�ɑ䐔�j
	ulong			Bunrui1_cnt1[3];								// ���ޏW�v�P�F�敪�ȏ�̑䐔
	ulong			Bunrui1_ryo1[3];								// ���ޏW�v�P�F�敪�ȏ�̗���
	ulong			Bunrui1_cnt2[3];								// ���ޏW�v�P�F�敪�s���̑䐔�i�u���ɁE���Z �����ʏW�v�v���F���ɑ䐔�j
																	// �@�@�@�@�@�@�@�@�@�@�@�@�@�i�u���ɁE�o�� �����ʏW�v�v���F���ɑ䐔�j
	ulong			Bunrui1_ryo2[3];								// ���ޏW�v�P�F�敪�s���̗����i�u���ɁE���Z �����ʏW�v�v���F���Z�䐔�j
																	// �@�@�@�@�@�@�@�@�@�@�@�@�@�i�u���ɁE�o�� �����ʏW�v�v���F�o�ɑ䐔�j
	ulong			Electron_sei_cnt;								// Suica���Z���񐔁�
	ulong			Electron_sei_ryo;								// �@�@�@�@ �����z��
	ulong			Electron_psm_cnt;								// PASMO���Z���񐔁�
	ulong			Electron_psm_ryo;								// �@�@�@�@ �����z��
	ulong			Electron_edy_cnt;								// Edy���Z  ���񐔁�
	ulong			Electron_edy_ryo;								// �@�@�@�@ �����z��
	ulong			Electron_Arm_cnt;								// Edy�װю��  ���񐔁�
	ulong			Electron_Arm_ryo;								// �@�@�@�@     �����z��

	ulong			Gengaku_seisan_cnt;								// ���z���Z��
	ulong			Gengaku_seisan_ryo;								// ���z���Z���z
	ulong			Furikae_seisan_cnt;								// �U�֐��Z��
	ulong			Furikae_seisan_ryo;								// �U�֐��Z���z

	ulong			Electron_ico_cnt;								// ICOCA���Z���񐔁�
	ulong			Electron_ico_ryo;								// �@�@�@�@ �����z��

	ulong			Electron_icd_cnt;								// IC-CARD���Z���񐔁�
	ulong			Electron_icd_ryo;								// �@�@�@�@ �����z��
	
	
	ulong			Syuusei_seisan_Mcnt;							// �C�����Z��������
	ulong			Syuusei_seisan_Mryo;							// �C�����Z�������z
	ulong			Lag_extension_cnt;								// ���O�^�C��������
	LOKTOTAL_DAT	loktldat[SYU_LOCK_MAX];							// ���Ԉʒu�ʏW�v(���Ԉʒu��:�擪����R�O�Ԏ���)
	ulong			CenterSeqNo;									// �Z���^�[�ǔԁi�W�v�j
	// note: �W�v�f�[�^���畡����NT-NET�f�[�^���쐬����邽�߁A�K�v�ȃV�[�P���V����No.��\�ߎ擾���Ă���
	//		 FT-4000�ł͈ȉ��̃f�[�^���쐬����
	//			�f�[�^��				ntnet,���uPhase1/���uPhase2
	//			�W�v��{�f�[�^			ID:30/42
	//			������ʖ��W�v�f�[�^	ID:31/43
	//			���ޏW�v�f�[�^			ID:32/44
	//			�����W�v�f�[�^			ID:33/45
	//			����W�v�f�[�^			ID:34/46
	//			�Ԏ����W�v�f�[�^		ID:35/--
	//			���K�W�v�f�[�^			ID:36/--
	//			�W�v�I���f�[�^			ID:41/53
	uchar			SeqNo[8];										// �V�[�P���V����No.
// MH322914(S) K.Onodera 2016/10/11 AI-V�Ή��F�U�֐��Z
//	ulong			dummy[13];										// �\���G���A(dummy�͍\���̂̍Ō�ɔz�u���邱�ƁI)
	ulong			Mno_use_cnt5;									// �X�m���D���������񐔁��i��{�A�g���P�j�i�X�m���D�P�`�P�O�O�j
	ulong			Mno_use_ryo5;									// �@�@�@�@�@�@�@�����z���i��{�A�g���P�j�i�X�m���D�P�`�P�O�O�j
	ulong			Furikae_seisan_cnt2;							// �U�֐��Z��(ParkingWeb��)
	ulong			Furikae_seisan_ryo2;							// �U�֐��Z���z(ParkingWeb��)
	ulong			Remote_seisan_cnt;								// ���u���Z��(ParkingWeb��)
	ulong			Remote_seisan_ryo;								// ���u���Z���z(ParkingWeb��)
	ulong			Furikae_Card_cnt[MOD_TYPE_MAX];					// �U�֐��Z�ɂ����錻���ȊO�̉ߕ�����
	ulong			Furikae_CardKabarai[MOD_TYPE_MAX];				// �U�֐��Z�ɂ����錻���ȊO�̉ߕ����z
// MH321800(S) G.So IC�N���W�b�g�Ή�
//�����σ��[�_�������ł������܂ŏW�v�G���A��ǉ����܂��B
	EC_SYU_DATA		Ec_syu[EC_BRAND_TOTAL_MAX];							// ���σ��[�_
// MH321800(E) G.So IC�N���W�b�g�Ή�
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
	//	   �����o�͍Ō���ɒǉ����A�r���Ƀ����o��}�����Ȃ�����
// MH321800(S) G.So IC�N���W�b�g�Ή�
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
	ulong			Kaimono_use_cnt[KAIMONO_WARISYU_MAX+1];			// ���������@�`�I + 臒l��񂪂Ȃ���������	���񐔁�
	ulong			Kaimono_use_ryo[KAIMONO_WARISYU_MAX+1];			// 				 							�����z��
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	ulong			Ec_minashi_cnt;									// �݂Ȃ����ϐ��Z���񐔁�
	ulong			Ec_minashi_ryo;									// �@�@�@�@ �����z��
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// ���W�v�̈�g���̋@��ɗ\���̈��100���m�ۂ���B�i�v�F8120Byte�j
//	ulong			dummy[1];										// �\���G���A(dummy�͍\���̂̍Ō�ɔz�u���邱�ƁI)
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
////	ulong			dummy[100];										// �\���G���A(dummy�͍\���̂̍Ō�ɔz�u���邱�ƁI)
//	ulong			dummy[78];										// �\���G���A(dummy�͍\���̂̍Ō�ɔz�u���邱�ƁI)
//// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
	ulong			dummy[76];										// �\���G���A(dummy�͍\���̂̍Ō�ɔz�u���邱�ƁI)
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH322914(E) K.Onodera 2016/10/11 AI-V�Ή��F�U�֐��Z
} SYUKEI;

// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//#define	nanaco_sei_cnt		Ec_syu[0].sei_cnt
//#define	nanaco_sei_ryo		Ec_syu[0].sei_ryo
//#define	nanaco_alm_cnt		Ec_syu[0].alm_cnt
//#define	nanaco_alm_ryo		Ec_syu[0].alm_ryo
//#define	waon_sei_cnt		Ec_syu[1].sei_cnt
//#define	waon_sei_ryo		Ec_syu[1].sei_ryo
//#define	waon_alm_cnt		Ec_syu[1].alm_cnt
//#define	waon_alm_ryo		Ec_syu[1].alm_ryo
//#define	sapica_sei_cnt		Ec_syu[2].sei_cnt
//#define	sapica_sei_ryo		Ec_syu[2].sei_ryo
//#define	sapica_alm_cnt		Ec_syu[2].alm_cnt
//#define	sapica_alm_ryo		Ec_syu[2].alm_ryo
//#define	koutsuu_sei_cnt		Ec_syu[3].sei_cnt
//#define	koutsuu_sei_ryo		Ec_syu[3].sei_ryo
//#define	koutsuu_alm_cnt		Ec_syu[3].alm_cnt
//#define	koutsuu_alm_ryo		Ec_syu[3].alm_ryo
//#define	id_sei_cnt			Ec_syu[4].sei_cnt
//#define	id_sei_ryo			Ec_syu[4].sei_ryo
//#define	id_alm_cnt			Ec_syu[4].alm_cnt
//#define	id_alm_ryo			Ec_syu[4].alm_ryo
//#define	quicpay_sei_cnt		Ec_syu[5].sei_cnt
//#define	quicpay_sei_ryo		Ec_syu[5].sei_ryo
//#define	quicpay_alm_cnt		Ec_syu[5].alm_cnt
//#define	quicpay_alm_ryo		Ec_syu[5].alm_ryo
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// �u�����h���̎x���񐔁E���z
#define	nanaco_sei_cnt		Ec_syu[0].sei_cnt
#define	nanaco_sei_ryo		Ec_syu[0].sei_ryo
#define	waon_sei_cnt		Ec_syu[1].sei_cnt
#define	waon_sei_ryo		Ec_syu[1].sei_ryo
#define	sapica_sei_cnt		Ec_syu[2].sei_cnt
#define	sapica_sei_ryo		Ec_syu[2].sei_ryo
#define	koutsuu_sei_cnt		Ec_syu[3].sei_cnt
#define	koutsuu_sei_ryo		Ec_syu[3].sei_ryo
#define	id_sei_cnt			Ec_syu[4].sei_cnt
#define	id_sei_ryo			Ec_syu[4].sei_ryo
#define	quicpay_sei_cnt		Ec_syu[5].sei_cnt
#define	quicpay_sei_ryo		Ec_syu[5].sei_ryo
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define qr_sei_cnt			Ec_syu[6].sei_cnt
#define qr_sei_ryo			Ec_syu[6].sei_ryo
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
#define	pitapa_sei_cnt		Ec_syu[7].sei_cnt
#define	pitapa_sei_ryo		Ec_syu[7].sei_ryo
// MH810105(E) MH364301 PiTaPa�Ή�

// ������������W�v�iEc_syu��0,1,3�`5��alm_cnt,alm_ryo�͎g�p���Ȃ����Ɓj
#define	miryo_pay_ok_cnt	Ec_syu[2].alm_cnt	// �x���ς݉�
#define	miryo_pay_ok_ryo	Ec_syu[2].alm_ryo	// �x���ς݋��z
#define	miryo_unknown_cnt	Ec_syu[6].alm_cnt	// �x���s����
#define	miryo_unknown_ryo	Ec_syu[6].alm_ryo	// �x���s�����z
// MH810105 GG119202(E) ������������W�v�d�l���P

#define	_SYU_HDR_SIZ	((ulong)&((SYUKEI*)NULL)->Uri_Tryo)
#define	_SYU_DAT_CNT	((sizeof(SYUKEI)-_SYU_HDR_SIZ)/sizeof(ulong))
extern	SYUKEI		skyprn;											// �W�v�󎚃G���A
extern	SYUKEI		wksky;											// �W�v�Ҕ��Eܰ��ر

struct	SKY{														// �W�v�ر
	SYUKEI			tsyuk;											// T�W�v
	SYUKEI			gsyuk;											// GT�W�v
	SYUKEI			msyuk;											// MT�W�v
	SYUKEI			fsyuk;											// �����䐔�W�v
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͍Ō���ɒǉ����邱��!!!
	//	   �r���Ƀ����o��}�����Ȃ�����
};

extern	struct		SKY		sky;
extern	struct		SKY		skybk;									// �W�v�ر(�O��)

typedef struct {
	ulong			In_car_Tcnt;									// �����ɑ䐔
	ulong			In_car_cnt[IN_SIG_CNT];							// ���ɑ䐔�i���ɐM���P�A�Q�j
	ulong			Out_car_Tcnt;									// ���o�ɑ䐔
	ulong			Out_car_cnt[IN_SIG_CNT];						// �o�ɑ䐔�i���ɐM���P�A�Q�j
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
} CAR_COUNT;

extern	CAR_COUNT		CarCount;									// ���o�ɑ䐔���Ĵر
extern	CAR_COUNT		CarCount_W;									// ���o�ɑ䐔����ܰ��ر

// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// ���A���^�C�����p�̓��t�ؑ֊�̑�����z�A����������z
typedef struct {
	ulong			Uri_Tryo;		// ������z
	ulong			Genuri_Tryo;	// ����������z
	date_time_rec	Date_Time;		// ���t�ؑ֊�����i���݂̏��v����j
	ulong			reserve[8];		// �\��
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
}	DATE_SYOUKEI;

extern	DATE_SYOUKEI	Date_Syoukei;								//���t�ؑ֊�̏��v
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
#pragma unpack


/*--------------------------------------------------------------------------*/
/* ���엚���i�[�ر															*/
/*--------------------------------------------------------------------------*/
#define		ERR_LOG_CNT			10									// �װ۸ތ���
// MH322916(S) A.Iiizumi 2018/08/20 �������Ԍ��o�@�\�Ή�
//#define		ARM_LOG_CNT			10									// �װ�۸ތ���
#define		ARM_LOG_CNT			80									// �װ�۸ތ���
// MH322916(E) A.Iiizumi 2018/08/20 �������Ԍ��o�@�\�Ή�
#define		OPE_LOG_CNT			10									// ���엚������
#define		MON_LOG_CNT			10									// ���j�^��������
#define		RMON_LOG_CNT		10									// ���u�Ď�����
#define		FLP_LOG_CNT			10									// �s���E������������
#define		SYUUKEI_DAY			101									// �r���������E�d�����W�v�󎚓����̍ő�����i�{ work�p�i�����N���A���j�j
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		SYUUKEI_DAY_EC		65									// ���σ��[�_�W�v�󎚓����̍ő�����i�{ work�p�i�����N���A���j�j
// MH321800(E) G.So IC�N���W�b�g�Ή�
#define		IO_EVE_CNT			20									// ���o�Ɏ��ی���
#define		IO_LOG_CNT			70									// ���o�Ƀ��O���ݏ��̌���
#define		RT_PAY_LOG_CNT		100									// �����V�~�����[�^���Z��������

/*--------------------------------------------------------------------------*/
/* ���d����																	*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec	Pdw_Date;										// ��d����
	date_time_rec	Pon_Date;										// ���d����
	unsigned char	Pon_nmi;										// NMI�׸�
	unsigned char	Pon_kind;										// ���d���
} Pon_log;

extern	Pon_log		Pon_log_work;									// �╜�d���O���[�N�o�b�t�@

/*--------------------------------------------------------------------------*/
/* �װ����																	*/
/*--------------------------------------------------------------------------*/
#define	ERR_LOG_ASC_DAT_SIZE	36									// 35�����i������P�s�j+ 1
#define	ERR_LOG_DOOR_STS_F		0x01								// �h�A�J�r�b�g
#define	ERR_LOG_RESEND_F		0x02								// �h�A�đ��r�b�g

typedef	struct {
	date_time_rec	Date_Time;										// ��������
	uchar			Errsyu;											// �װ���
	uchar			Errcod;											// �װ����
	uchar			Errdtc;											// �װ����/����
	uchar			Errlev;											// �װ����
	uchar			ErrDoor;										// b0:0=�h�A��,1=�J�^b1:�đ� 0=�łȂ�,1=�ł���
	uchar			Errinf;											// �װ���L��(0:�Ȃ��A1:acsii�A2:bin)
	ulong			ErrBinDat;										// �װ���(bin����)
	uchar			Errdat[ERR_LOG_ASC_DAT_SIZE];					// �װ���(acsii����)
	uchar			ErrSeqNo;										// �V�[�P���V����No.
} Err_log;

struct	Err_log_rec {
	short			Err_wtp;										// �G���[���O�󎚃��C�g�|�C���^
	Err_log			Err_log_dat[ERR_LOG_CNT];						// �G���[���O�󎚃o�b�t�@
};
extern	struct	Err_log_rec		ERR_LOG_DAT;
extern	Err_log		Err_work;										// �G���[���O���[�N�o�b�t�@

/*--------------------------------------------------------------------------*/
/* �װї���																	*/
/*--------------------------------------------------------------------------*/
#define	ALM_LOG_ASC_DAT_SIZE	36									// 35�����i������P�s�j+ 1

typedef	struct {
	date_time_rec	Date_Time;										// ��������
	uchar			Armsyu;											// �װю��
	uchar			Armcod;											// �װѺ���
	uchar			Armdtc;											// �װє���/����
	uchar			Armlev;											// �װ�����
	uchar			ArmDoor;										// b0:0=�h�A��,1=�J�^b1:�đ� 0=�łȂ�,1=�ł���
	uchar			Arminf;											// �װя��L��
	ulong			ArmBinDat;										// �װя��(bin����)
	uchar			Armdat[ALM_LOG_ASC_DAT_SIZE];					// �װя��(acsii����)
	uchar			ArmSeqNo;										// �V�[�P���V����No.
} Arm_log;

struct	Arm_log_rec {
	short			Arm_wtp;										// �A���[�����O�󎚃��C�g�|�C���^
	Arm_log			Arm_log_dat[ARM_LOG_CNT];						// �A���[�����O�󎚃o�b�t�@
};
extern	struct	Arm_log_rec		ARM_LOG_DAT;
extern	Arm_log	Arm_work;											// �A���[�����O���[�N�o�b�t�@


/*--------------------------------------------------------------------------*/
/* ���엚��																	*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec	Date_Time;										// ��������
	uchar			OpeKind;										// ������
	uchar			OpeCode;										// ����R�[�h
	uchar			OpeLevel;										// ���샌�x��
	ulong			OpeBefore;										// �ύX�O�f�[�^
	ulong			OpeAfter;										// �ύX��f�[�^
	uchar			OpeSeqNo;										// �V�[�P���V����No.
} Ope_log;

struct	Ope_log_rec {
	short			Ope_count;										// ���엚������
	short			Ope_wtp;										// ���엚��ײ��߲��
	Ope_log			Ope_work;										// ���엚��ܰ��ޯ̧
	Ope_log			Ope_log_dat[OPE_LOG_CNT];						// ���엚���ޯ̧
};

extern	struct	Ope_log_rec		OPE_LOG_DAT;
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//extern	Ope_log 		Ope_work;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�

/*--------------------------------------------------------------------------*/
/* ���j�^����																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec	Date_Time;										// ��������
	uchar			MonKind;										// ���j�^���
	uchar			MonCode;										// ���j�^�R�[�h
	uchar			Rsv;											// �\��
	uchar			MonLevel;										// ���j�^���x��
	uchar			MonInfo[10];									// ���j�^���
	uchar			MonSeqNo;										// �V�[�P���V����No.
} Mon_log;

struct	Mon_log_rec {
	short			Mon_count;										// ���j�^��������
	short			Mon_wtp;										// ���j�^����ײ��߲��
	Mon_log			Mon_work;										// ���j�^����ܰ��ޯ̧
	Mon_log			Mon_log_dat[MON_LOG_CNT];						// ���j�^�����ޯ̧
};

extern	struct	Mon_log_rec		MON_LOG_DAT;
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//extern	Mon_log			Mon_work;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�

/*--------------------------------------------------------------------------*/
/* �s����������																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec	Date_Time;										// ��������
	date_time_rec	In_Time;										// ���ɓ���
	ulong			WPlace;											// ���Ԉʒu�ް�
	uchar			Lok_inf;										// ������ 0:�����o��
																	//          1:�s���o��
																	//          2:�s���o��(�C�����Z) GM333200
	uchar			ryo_syu;										// �������
	long			fusei_fee;										// �s���o�ɒ��ԗ���
	short			dumy;											// not used
	ushort			Lok_No;											// ���b�N���uNo
	ST_OIBAN		count;											// �ǂ���
	ushort			Kikai_no;										// �@�B�m��
	uchar			Seisan_kind;									// ���Z���
}	flp_log;

struct	Flp_log_rec {
	short			Flp_wtp;										// �s������ײ��߲��
	flp_log			Flp_log_dat[FLP_LOG_CNT];						// �s�������ޯ̧
};

extern	struct	Flp_log_rec		FLP_LOG_DAT;

/*--------------------------------------------------------------------------*/
/* ���u�Ď�																	*/
/*--------------------------------------------------------------------------*/
typedef union {
	ulonglong		ullinstNo;
	ulong			ulinstNo[2];
} u_inst_no;

typedef	struct {
	date_time_rec2	Date_Time;										// ��������
	u_inst_no		RmonInstNo;										// ���ߔԍ�
	ushort			RmonFuncNo;										// �[���@�\�ԍ�
	ushort			RmonProcNo;										// �����ԍ�
	ushort			RmonProcInfoNo;									// �������ԍ�
	ushort			RmonStatusDetailNo;								// �󋵏ڍהԍ�
	ushort			RmonInfo;										// �[���Ď����
	uchar			RmonStatusNo;									// �󋵔ԍ�
	uchar			RmonSeqNo;										// �V�[�P���V����No.
	date_time_rec	RmonStartTime1;									// �\��J�n����1
	date_time_rec	RmonStartTime2;									// �\��J�n����2
	uchar			RmonProgNo[12];									// �v���O�������ԁE�o�[�W����
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
	uchar			RmonFrom;										// ���ߗv�����i0���Z���^�[/1=�[���j
	uchar			RmonUploadReq;									// �ݒ�A�b�v���[�h�v��
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
} Rmon_log;

struct	Rmon_log_rec {
	short			Rmon_count;										// ���u�Ď�����
	short			Rmon_wtp;										// ���u�Ď�ײ��߲��
	Rmon_log		Rmon_work;										// ���u�Ď�ܰ��ޯ̧
	Rmon_log		Rmon_log_dat[RMON_LOG_CNT];						// ���u�Ď��ޯ̧
};

extern	struct	Rmon_log_rec	RMON_LOG_DAT;

#pragma pack
/*--------------------------------------------------------------------------*/
/* ���Z����(�ʐ��Z���)													*/
/*--------------------------------------------------------------------------*/
// ������ް�̫�ϯ�
typedef struct {
	uchar			syu;											// ��������
	uchar			status;											// ������ð��(�ǎ掞)
	ulong			id;												// �����id
	uchar			pkno_syu;										// ��������ԏ�m���D���
	uchar			update_mon;										// �X�V����
	ushort			s_year;											// �L�������i�J�n�F�N�j
	uchar			s_mon;											// �L�������i�J�n�F���j
	uchar			s_day;											// �L�������i�J�n�F���j
	ushort			e_year;											// �L�������i�I���F�N�j
	uchar			e_mon;											// �L�������i�I���F���j
	uchar			e_day;											// �L�������i�I���F���j
	uchar			t_tim[4];										// ������������
	uchar			update_rslt1;									// ����X�V���Z���̍X�V����			�i�@OK�F�X�V�����@�^�@NG�F�X�V���s�@�j
	uchar			update_rslt2;									// ����X�V���Z���̃��x�����s����	�i�@OK�F���픭�s�@�^�@NG�F���s�s�ǁ@�j
	uchar			Apass_off_seisan;								// 1=��������߽OFF�ݒ����������Ȃ�����
	ulong			ParkingNo;										// ���ԏ�m���D
} teiki_use;

// ���޽��,�|����,������,�񐔌��ް�̫�ϯ�
typedef struct {
	uchar			tik_syu;										// ���������
																	// 0x01=���޽��
																	// 0x02=�|����
																	// 0x03=�񐔌�
																	// 0x04=������
																	// 0x10=�������i���z���Z���Ɏg�p�j
																	// 0x11=�U�֊z�i�U�֐��Z���Ɏg�p�j
	uchar			pkno_syu;										// ���ԏ�m���D���
// MH322914(S) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
//	uchar			syubetu;										// ���
	ushort			syubetu;										// ���
// MH322914(E) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
	uchar			maisuu;											// ����
	ushort			mise_no;										// �X�m���D�i��������ʁ��������̏ꍇ�A���ް��̓X�m���D���i�[�j
	ulong			ryokin;											// ��������
	ulong			minute;											// ��������(UT8500���g�p)
} wari_tiket;

// ����߲�޶����ް�̫�ϯ�
typedef struct {
	ulong			ppc_id;											// ����߲�޶��ނm���D
	ulong			pay_befor;										// �x���O�c�z
	ulong			pay_after;										// �x����c�z
	ushort			kigen_year;										// �L�������N
	uchar			kigen_mon;										// �L��������
	uchar			kigen_day;										// �L��������
} ppc_tiket;

// �ڼޯĶ����ް�̫�ϯ�
typedef struct {
// MH321800(S) G.So IC�N���W�b�g�Ή�(�N���W�b�g�J�[�h��Ж��iShift JIS�j24���Ή�)
//	char			card_name[12];									// �ڼޯĶ��މ�З���
	char			card_name[24];									// �ڼޯĶ��މ�З���
// MH321800(E) G.So IC�N���W�b�g�Ή�(�N���W�b�g�J�[�h��Ж��iShift JIS�j24���Ή�)
	char			card_no[20];									// �ڼޯĶ��މ���ԍ�
	ulong			slip_no;										// �ڼޯĶ��ޓ`�[�ԍ�
	ulong			app_no;											// �ڼޯĶ��ޏ��F�ԍ�
	ulong			pay_ryo;										// �ڼޯĶ��ޗ��p���z
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	date_time_rec2	CenterProcTime;									// �ڼޯĶ��޾������������i�b����j
//	ulong			CenterProcOiBan;								// �ڼޯĶ��޾��������ǔ�
//	char			CCT_Num[16];									// �ڼޯĶ��ޒ[�����ʔԍ�
	char			CCT_Num[13];									// �ڼޯĶ��ޒ[�����ʔԍ�
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	uchar			cre_type;										// 0=�ڼޯĐ��Z 1=iD���Z
																	// 2=�@�l����
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	uchar			CreditDate[2];									// �ڼޯĶ��ޗL������ [0]=�N [1]=��
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	uchar			kid_code[6];									// KID����
	char			ShopAccountNo[20];								// �ڼޯĶ��މ����X����ԍ�
	char			AppNoChar[6];									// ���F�ԍ�(�p����)
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	uchar			Identity_Ptrn;									// �{�l�m�F�p�^�[��
// MH810105(S) MH364301 �C���{�C�X�Ή�
//	uchar			dummy1;
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
} credit_use;

enum {
	CREDIT_CARD = 0,
	CREDIT_HOJIN,
	_POSTPAY_MAX
};


#define	SUICA_USED	1
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define	EDY_USED	2
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define	PASMO_USED	3
#define	ICOCA_USED	4
#define	ICCARD_USED	5
// MH321800(S) G.So IC�N���W�b�g�Ή�
enum {
// �ȉ����σ��[�_
	EC_USED = 11,				// ���σ��[�_�g�p
	EC_UNKNOWN_USED = EC_USED,	// [11]�s���u�����h
	EC_ZERO_USED,				// [12]�u�����h�ԍ�0
	EC_EDY_USED,				// [13]Edy
	EC_NANACO_USED,				// [14]nanaco
	EC_WAON_USED,				// [15]WAON
	EC_SAPICA_USED,				// [16]SAPICA
	EC_KOUTSUU_USED,			// [17]��ʌnIC�J�[�h
	EC_ID_USED,					// [18]iD
	EC_QUIC_PAY_USED,			// [19]QuicPay
	EC_CREDIT_USED,				// [20]�N���W�b�g
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	EC_TCARD_USED,				// [21]T�J�[�h
	EC_HOUJIN_USED,				// [22]�@�l
	EC_HOUSE_USED,				// [23]�n�E�X�J�[�h
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	EC_QR_USED,					// [24]QR�R�[�h����
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
	EC_PITAPA_USED,				// [25]PiTaPa
// MH810105(E) MH364301 PiTaPa�Ή�
};
// MH321800(E) G.So IC�N���W�b�g�Ή�

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
// QR�u�����h�̎x�����
enum {
	QR_DUMMY,		// [00]
	ALIPAY,			// [01]
	DPAY,			// [02]
	LINEPAY,		// [03]
	PAYPAY,			// [04]
	RAKUTENPAY,		// [05]
	MICROPAY,		// [06]
	UPICPM,			// [07]
	AUPAY,			// [08]
	MERPAY,			// [09]
	ORIGAMIPAY,		// [10]
	GINKOPAY,		// [11]
	QUOPAY,			// [12]
	SGTDASH,		// [13]
	SGTGLB,			// [14]
	SMARTCODE,		// [15]
	JCOINPAY,		// [16]
	AMAZONPAY,		// [17]
	JKOPAY,			// [18]
	GLNPAY,			// [19]
	BANKPAY,		// [20]
	FAMIPAY,		// [21]
	ATONEPAY,		// [22]
	QR_PAY_KIND_MAX
};
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

typedef union{
	uchar	e_status;
	struct {
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//		uchar YOBI				:7;		/* B1-7 = �\�� */				
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//		uchar YOBI				:4;		/* B4-7 = �\�� */
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//		uchar YOBI				:3;		/* B5-7 = �\�� */
// MH810105 GG119202(S) GT4710FX�Ή�(���σ��[�_�[�Ή�/�����c���Ɖ�����L�^����)(MH615702���p)
//		uchar YOBI				:2;		/* B6-7 = �\�� */
		uchar reserve			:1;		/* B7 : �\��(GT4710FX�Ŏg�p)	*/
		uchar YOBI				:1;		/* B6 = �\�� */
// MH810105 GG119202(E) GT4710FX�Ή�(���σ��[�_�[�Ή�/�����c���Ɖ�����L�^����)(MH615702���p)
		uchar deemed_sett_fin	:1;		/* B5 = �݂Ȃ����ςɂ�鐸�Z����*/
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
		uchar miryo_confirm		:1;		/* B4 = �����m�� */
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
		uchar brand_mismatch	:1;		/* B3 = �u�����h�s��v */
		uchar deemed_receive	:1;		/* B2 = ���ϐ��Z���~(������) */			
		uchar deemed_settlement	:1;		/* B1 = �݂Ȃ����� */
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
		uchar pay_cansel_work	:1;		/* B0 = �d�q�}�l�[�����ς��׸ޕێ��̈�(�󎚗p) */
	} BIT;
}t_E_STATUS;

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
typedef union{
	uchar	e_flag;
	struct {
		uchar YOBI				:6;		/* B1-6 = �\�� */
		uchar minashi_tim_pri	:1;		/* B4 = �݂Ȃ��W�v���ɔ����������׸� */
		uchar deemSettleCancal	:1;		/* B0 = �����x���s���i���ό��� 1= ����M 0= ��M�j�׸� */
	} BIT;
}t_E_FLAG;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//typedef struct {
//	uchar	e_pay_kind;				// �d�q���ώ�� Edy:2
//	t_E_STATUS E_Status;			// �d�q���ϗp�e��ð��
//	ulong	pay_ryo;				// ���ϊz
//	ulong	pay_befor;				// ���Z�OSuica�c��
//	ulong	pay_after;				// ���ό�Suica�c��
//	uchar	Card_ID[8];				// ����ID (BCD 16��)
//	ulong	deal_no;				// ����ʔ�(Hex)
//	ushort	card_deal_no;			// ���ގ���ʔ�(Hex)
//	uchar	Terminal_ID[4];			// ��ʒ[��ID (BCD 8��)
//	ushort	IDm;					// Felica card ID
//} Edy_info;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

typedef struct {
	uchar	e_pay_kind;				// �d�q���ώ�� Suica:1
	t_E_STATUS E_Status;			// �d�q���ϗp�e��ð��
	ulong	pay_ryo;				// ���ϊz
	ulong	pay_befor;				// ���Z�OSuica�c��
	ulong	pay_after;				// ���ό�Suica�c��
	uchar	Card_ID[16];			// ����ID (Ascii 16��)
	uchar	dmy2[4];				// ��
} Suica_info;

// MH321800(S) G.So IC�N���W�b�g�Ή�
// ����f�[�^
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
typedef struct {
// ��ʌnIC
	uchar	SPRW_ID[13];			// SPRWID
	uchar	dmy1;
	uchar	Kamei[30];				// �����X��
	uchar	TradeKind;				// ������
	uchar	dmy2;
} Ec_info_Koutsuu;
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��

typedef struct {
// Edy
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//	ulong	DealNo;					// ����ʔ�
//	ulong	CardDealNo;				// �J�[�h����ʔ�
//	ulong	TerminalNo[2];			// �[���ԍ�
	uchar	DealNo[10];				// Edy����ʔ�
	uchar	CardDealNo[5];			// �J�[�h����ʔ�
	uchar	dmy;
	uchar	TerminalNo[8];			// ��ʒ[��ID
	uchar	Kamei[30];				// �����X��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
} Ec_info_Edy;

typedef struct {
// nanaco
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	ulong	TerminalNo[3];			// ��ʒ[��ID:10�i 20��
//	ulong	DealNo;					// ��ʎ���ʔ�
	uchar	Kamei[30];				// �����X��
	uchar	DealNo[6];				// �[������ʔ�
	uchar	TerminalNo[20];			// ��ʒ[��ID
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
} Ec_info_Nanaco;

typedef struct {
// WAON
	ulong	point;					// ����|�C���g
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//	ulong	point_total;			// �݌v�|�C���g
//	long	card_type;				// �J�[�h����
//	ulong	period_point;			// �Q�N�O�܂łɊl�������|�C���g
//	ulong	period;					// �Q�N�O�܂łɊl�������|�C���g�̗L������
//	uchar	SPRW_ID[14];			// SPRWID(���ۂ�Ascii 13��)
//	uchar	point_status;			// ���p��
//	uchar	deal_code;				// �����ʃR�[�h
	ulong	point_total[2];			// �݌v�|�C���g
	uchar	SPRW_ID[13];			// SPRWID
	uchar	point_status;			// �݌v�|�C���g���b�Z�[�W
									// 0=�u�݌vWAON�|�C���g���́A���߂���WAON�X�e�[�V�����Ŋm�F���������B�v
									// 1=�u����WAON�J�[�h�̓|�C���g�ΏۊO�ł��B�v
									// 2=�uJMB WAON�̓}�C�������܂��B�v
	uchar	Kamei[30];				// �����X��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
} Ec_info_Waon;

typedef struct {
// SAPICA
	uchar	Terminal_ID[8];			// �[��ID (���ۂ�Ascii  7��)
	ulong	Details_ID;				// ����ID
} Ec_info_Sapica;

typedef struct {
// iD
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	uchar	Terminal_ID[8];			// �[��ID (���ۂ�Ascii  7��)
//	ulong	Details_ID;				// ����ID
	uchar	Kamei[30];				// �����X��
	uchar	Approval_No[7];			// ���F�ԍ�
	uchar	dmy1;
	uchar	TerminalNo[13];			// ��ʒ[��ID
	uchar	dmy2;
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
} Ec_info_Id;

typedef struct {
// QUICPay
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	uchar	Terminal_ID[8];			// �[��ID (���ۂ�Ascii  7��)
//	ulong	Details_ID;				// ����ID
	uchar	Kamei[30];				// �����X��
	uchar	Approval_No[7];			// ���F�ԍ�
	uchar	dmy1;
	uchar	TerminalNo[13];			// ��ʒ[��ID
	uchar	dmy2;
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
} Ec_info_Quickpay;

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
typedef struct {
// QR
	uchar	PayKind;				// �x�����
	uchar	dmy1;
	uchar	PayTerminalNo[20];		// �x���[��ID
	uchar	MchTradeNo[32];			// Mch����ԍ�
} Ec_info_Qr;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810105(S) MH364301 PiTaPa�Ή�
typedef	struct {
// PiTaPa
	uchar	Kamei[30];				// �����X��
	uchar	Approval_No[8];			// ���F�ԍ�
	uchar	TerminalNo[13];			// ��ʒ[��ID
	uchar	Slip_No[5];				// �`�[�ԍ�
} Ec_info_Pitapa;
// MH810105(E) MH364301 PiTaPa�Ή�

typedef struct {
	uchar	e_pay_kind;				// �d�q���ώ�� Sanden:11�`
	t_E_STATUS E_Status;			// �d�q���ϗp�e��ð��
	ulong	pay_ryo;				// ���ϊz
	ulong	pay_befor;				// ���Z�O�c��
	ulong	pay_after;				// ���ό�c��
	uchar	Card_ID[20];			// ����ID (���ۂ�Ascii 16or17��)	��Card_ID��wk_DicData.CardNo�̃T�C�Y�ɍ��킹��
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
//	uchar	inquiry_num[16];		// �⍇���ԍ�
	uchar	inquiry_num[15];		// �⍇���ԍ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	uchar	dummy;					// �₢���킹�ԍ��ƍ��킹��16byte
	t_E_FLAG	E_Flag;				// �׸ފǗ�(uchar)
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
	ulong	pay_datetime;			// ���ϓ���(Normalize)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
//	uchar	Termserial_No[30];		// �[�����ʔԍ�(30��)
//// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
	uchar	QR_Kamei[30];			// �����X���iQR�R�[�h���ρj
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	union brand {					// �u�����h�ʎ���f�[�^
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		Ec_info_Koutsuu		Koutsuu;// ��ʌnIC
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		Ec_info_Edy			Edy;	// Edy
		Ec_info_Nanaco		Nanaco;	// nanaco
		Ec_info_Waon		Waon;	// WAON
		Ec_info_Sapica		Sapica;	// SAPICA
		Ec_info_Id			Id;		// iD
		Ec_info_Quickpay	Quickpay;	// QUICPay
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		Ec_info_Qr			Qr;		// QR
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
		Ec_info_Pitapa		Pitapa;	// PiTaPa
// MH810105(E) MH364301 PiTaPa�Ή�
	} Brand;
} Ec_info;
// MH321800(E) G.So IC�N���W�b�g�Ή�

typedef union {
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_info	Edy;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	Suica_info	Suica;
// MH321800(S) G.So IC�N���W�b�g�Ή�
	Ec_info		Ec;
// MH321800(E) G.So IC�N���W�b�g�Ή�
} Electron_info;


typedef union{
	uchar	lw_status;
	struct {
		uchar YOBI		:2;		// B6,7	�\��
		uchar PWARI		:2;		// B4,5	�O�񁓊���
		uchar RWARI		:2;		// B2,3	�O�񗿋�����
		uchar TWARI		:2;		// B0,1	�O�񎞊Ԋ���
	} BIT;
}t_LW_STATUS;
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//typedef union{
//	unsigned char	BYTE;					// Byte
//	struct{
//		unsigned char	YOBI:6;				// �\��
//		unsigned char	T_Cancel:1;			// 1:�s�J�[�h�L�����Z������
//		unsigned char	T_Rate:1;			// 1:���ʊ��ԃ��[�g�K�p
//	} BIT;
//} Card_Receipt;
//typedef struct {
//	uchar			T_card_num[16];
//	ulong			T_point;
//	Card_Receipt	T_status;
//} T_CARD;
//
//#define T_POINT_MASK( a ) (a&0x7fffffff)
//#define T_CARD_USE( a ) (a&0x80000000)
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
typedef	struct {
	uchar			SystemID;										// ����ID
	uchar			DataKind;										// �ް����
	uchar			DataKeep;										// �ް��ێ��׸�
	uchar			SeqNo;											// ���ݼ�ه�
	ulong			ParkingNo;										// ���ԏꇂ
	ushort			ModelCode;										// �@����
	ulong			MachineNo;										// �@�B��
	union {
		uchar		SerialNo[6];									// �[���ره�
		struct {
			ushort	encryptMode;									// �Í�������(0=�Í��Ȃ� 1=AES����)
			ushort	encryptKey;										// �Í����ԍ�
			uchar	Reserve[2];										// �\��
		} DT_BASIC;
	} CMN_DT;
	uchar			Year;											// �N
	uchar			Mon;											// ��
	uchar			Day;											// ��
	uchar			Hour;											// ��
	uchar			Min;											// ��
	uchar			Sec;											// �b
} DATA_BASIC;

// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// typedef	struct {
// 	ulong			ParkingNo;										// ���ԏꇂ
// 	ushort			DiscSyu;										// �������
// 	ushort			DiscNo;											// �����敪
// 	ushort			DiscCount;										// ����
// 	ulong			Discount;										// �����z(��������)
// 	ulong			DiscInfo1;										// �������1
// 	union {
// 		struct {													// �v���y�C�h�J�[�h�ȊO�̍\����
// 			ulong			DiscInfo2;								// �������2
// 			ushort			MoveMode;								// ����^�ԋp
// 			ushort			DiscFlg;								// �����ς�
// 		} common;
// 		struct {													// �v���y�C�h�J�[�h�p�\����
// 			ulong			pay_befor;								// �x���O�c�z
// 			ushort			kigen_year;								// �L�������N
// 			uchar			kigen_mon;								// �L��������
// 			uchar			kigen_day;								// �L��������
// 		} ppc_data;													// NT-NET���Z�f�[�^���M�O�ɃN���A����
// 	} uDiscData;
// } DISCOUNT_DATA;
typedef	struct {
	ulong			ParkingNo;				// ���ԏꇂ
	ushort			DiscSyu;				// �������
	ushort			DiscNo;					// �����敪
	ulong			Discount;				// ����g�p�����������z
	ulong			DiscInfo1;				// �������1
	union {
		struct {							// �v���y�C�h�J�[�h�ȊO�̍\����
			ulong	DiscInfo2;				// �������2
			ulong	PrevDiscount;			// ����g�p�����O�񐸎Z�܂ł̊������z
			ulong	PrevUsageDiscount;		// �O�񐸎Z�܂ł̎g�p�ς݊������z
			uchar	PrevUsageDiscCount;		// �O�񐸎Z�܂ł̎g�p�ςݖ���
// GG124100(S) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
			uchar	FeeKindSwitchSetting;	// ��ʐ؊���Ԏ�
			uchar	DiscRole;				// ��������
// GG124100(E) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
		} common;
		struct {							// �v���y�C�h�J�[�h�p�\����
			ulong	pay_befor;				// �x���O�c�z
			ushort	kigen_year;				// �L�������N
			uchar	kigen_mon;				// �L��������
			uchar	kigen_day;				// �L��������
// GG124100(S) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
// 			uchar	Reserve[5];				// �\��
			uchar	Reserve[7];				// �\��
// GG124100(E) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
		} ppc_data;							// NT-NET���Z�f�[�^���M�O�ɃN���A����
	} uDiscData;
	uchar			DiscCount;				// ����g�p��������
} DISCOUNT_DATA;
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-7700:GM747904�Q�l�j
typedef	struct {
	ulong			ParkingNo;										// ���ԏꇂ�i��{���ԏ�m���Œ�j
	ushort			DiscSyu;										// ������ʁi�Q�O�O�F�������A�Q�O�P�F���o���j
	ushort			InOut10000;										// �P�O�O�O�O�~����or���o����
	ushort			InOut5000;										// �T�O�O�O�~����or���o���z
	ushort			InOut2000;										// �Q�O�O�O�~����or���o���z
	ushort			InOut1000;										// �P�O�O�O�~����or���o���z
	ushort			InOut500;										// �@�T�O�O�~����or���o���z
	ushort			InOut100;										// �@�P�O�O�~����or���o���z
	ushort			InOut50;										// �@�@�T�O�~����or���o���z
	ushort			InOut10;										// �@�@�P�O�~����or���o���z
	ushort			MoveMode;										// ����^�ԋp�@�i����̓_�~�[�j
	ushort			DiscFlg;										// �����ς݁@�i����̓_�~�[�j
} IN_OUT_MONEY;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-7700:GM747904�Q�l�j

// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// �����ڍ�(���׏��)
typedef struct{
	ulong			ParkingNo;										// ���ԏꇂ
	ushort			DiscSyu;										// ���
	union {
		struct{
			ushort	DiscNo;											// �����敪
			ushort	DiscCount;										// ����
			ulong	Discount;										// �����z(��������)
			ulong	DiscInfo1;										// �������1
			ulong	DiscInfo2;										// �������2
		} Common;
		struct{
			ulong	Pos;											// �����
			ulong	OrgFee;											// �U�֌����ԗ���
			ulong	Total;											// �U�֊z
			ulong	Oiban;											// �U�֌����Z�ǔ�
		} Furikae;			// 2031(�U�֐��Z)
		struct{
			ushort	Mod;											// �}�̎��(�ݒ�\�Ȓl�͐��Z�f�[�^�̃J�[�h���ϋ敪�Ɠ���)
			ushort	Reserve1;										// ���g�p
			ulong	FrkMoney;										// �U�֊z(����)
			ulong	FrkCard;										// �U�֊z(�����ȊO)
			ulong	FrkDiscount;									// �U�֊z(����)
		} FurikaeDetail;	// 2032(�U�֐��Z����)
		struct{
			ushort	Reserve1;										// ���g�p
			ushort	Reserve2;										// ���g�p
			ulong	Total;											// �x���\��z
			ulong	Reserve3;										// ���g�p
			ulong	Reserve4;										// ���g�p
		} Future;			// 2033(������Z�\��z)
		struct{
			ushort	Reserve1;										// ���g�p
			ushort	Reserve2;										// ���g�p
			ulong	Total;											// �x���\��z
			ulong	Reserve3;										// ���g�p
			ulong	Reserve4;										// ���g�p
		} AfterPay;			// 2034(������Z�z(����������z))
	} uDetail;
} DETAIL_DATA;
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
typedef struct{	
	uchar		CarCntYear;											// �N
	uchar		CarCntMon;											// ��
	uchar		CarCntDay;											// ��
	uchar		CarCntHour;											// ��
	uchar		CarCntMin;											// ��
	ushort		CarCntSeq;											// �ǔ�
	uchar		Reserve1;											// �\��
} CAR_CNT_INFO;	// �䐔�Ǘ��ǔ�
// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
// GG129000(S) H.Fujinaga 2023/02/18 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
typedef struct{	
	uchar		EntryMachineNo[3];									// �����@�ԍ�
	uchar		ParkingTicketNo[6];									// ���Ԍ��ԍ�
	uchar		TicketLessMode[1];									// �Ԕԃ`�P�b�g���X�A���L��
} QRTICKET_INFO;	// QR���Ԍ����
// GG129000(E) H.Fujinaga 2023/02/18 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j

// 1���Z����ް�̫�ϯ�
typedef struct {
	uchar			WFlag;											// �󎚎�ʂP�i0:�ʏ� 1:���d�j
	uchar			chuusi;											// �󎚎�ʂQ�i0:�̎��� 1:���Z���~ 2:���g�p 3:�C�����Z�̎��؁j
	ulong			WPlace;											// ���Ԉʒu�ް�
	ushort			Kikai_no;										// �@�B�m��
	uchar			Seisan_kind;									// ���Z���
	ST_OIBAN		Oiban;											// �ǔ�
	date_time_rec	TInTime;										// ���ɓ���
	date_time_rec	TOutTime;										// �o�ɓ����i�a��؂̏ꍇ�A���s�����j
	ulong			WPrice;											// ���ԗ����i����X�V�̎��؂̏ꍇ�A�X�V�����j
	uchar			syu;											// ���
	teiki_use		teiki;											// ����ް�
	ulong			ppc_chusi_ryo;									// ���Z���~����߲�޶��ޗ���
	ulong			BeforeTwari;									// �O�񎞊Ԋ������z
	ulong			BeforeRwari;									// �O�񗿋��������z
	ulong			BeforePwari;									// �O�񁓊������z
	ulong			Wtax;											// �ŋ�
	ulong			WInPrice;										// �����z
	ulong			WChgPrice;										// �̎��؂̏ꍇ		�F�ޑK���z
																	// ���Z���~�̏ꍇ	�F���ߋ��z
// �d�l�ύX(S) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
//	ulong			WTotalPrice;									// �̎����z
	long			WTotalPrice;									// �̎����z
// �d�l�ύX(E) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
	ulong			WFusoku;										// �̎��؂̏ꍇ		�F�ޑK���o�s�����z
																	// ���Z���~�̏ꍇ	�F���ߕs�����z
	credit_use		credit;											// �ڼޯĶ����ް�
// MH810105(S) MH364301 �C���{�C�X�Ή�
	uchar			WTaxRate;										// �K�p�ŗ�
// MH810105(E) MH364301 �C���{�C�X�Ή�
	uchar			testflag;										// �e�X�g�󎚗p�t���O
																	// GM333200(�C�����Z�ϐ��̖����ύX)
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	ulong			MMPlace;										// �ԈႢ���Z�F���Ԉʒu		���V�C�����Z�F�C�����Ԏ��ԍ�
//	ulong			MMPrice;										// �ԈႢ���Z�F���ԗ���		���V�C�����Z�F�C�������ԗ���(�U�֊z)
//	ulong			MMInPrice;										// �ԈႢ���Z�F�����z		���V�C�����Z�F�C���������z
//	ulong			MMTwari;										// �ԈႢ���Z�F���Ԋ������z	���V�C�����Z�F�C�������߁^�x���|
//	ulong			MMRwari;										// �ԈႢ���Z�F�����������z	���V�C�����Z�F�C���������z(����+����)
//	ST_OIBAN		MMPwari;										// �C�������Z�ǔ�
//	uchar			MMSyubetu;										// �C�����������
//	uchar			Zengakufg;										// �ԈႢ���Z�F�S�z�����׸�	���V�C�����Z�F����b0bit�F�C�����S�z����(0:�Ȃ��A1:����)
//																	//										�F����b1bit�F�C���a��^�|��(0:�a��A1:�|��)
//																	//										�F����b2��t�F������(0:�Ȃ��A1:����)
	ulong			zenkai;											// �O��̎��z
	uchar			CarSearchFlg;									// �Ԕ�/�����������
	uchar			CarSearchData[6];								// �Ԕ�/���������f�[�^
																	// �ԔԌ����̏ꍇ�F0�`3�ɎԔԁA4,5��0�Œ�
																	// ���������̏ꍇ�F�N�Q�o�C�g�A����ȊO�͂P�o�C�g
	uchar			CarNumber[4];									// �Z���^�[�₢���킹���̎Ԕ�
	uchar			CarDataID[32];									// �Ԕԃf�[�^ID(�⍇���Ɏg�p����ID)
	date_time_rec	BeforeTPayTime;									// �O�񐸎Z����
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
// �d�l�ύX(S) K.Onodera 2016/10/28 �̎��؃t�H�[�}�b�g
	ushort			FRK_RetMod;										// �U�֐��Z(ParkingWeb�o�R)�U�։ߕ����}��
	ushort			FRK_Return;										// �U�֐��Z(ParkingWeb�o�R)�U�։ߕ�����(���߂�)
// �d�l�ύX(E) K.Onodera 2016/10/28 �̎��؃t�H�[�}�b�g
	Electron_info 	Electron_data;									// �d�q���ϐ��Z���
	t_LW_STATUS		PRTwari;										// �O�񁓁A�����A���Ԋ����t���O
	ulong			ppc_chusi;										// ���Z���~�v���y�C�h�f�[�^�i�[
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	T_CARD			T_card;
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	uchar			PayCalMax;										// �ő嗿���z������
	uchar			ReceiptIssue;									// �̎��ؔ��s�L��
	uchar			PayMethod;										// ���Z���@
	uchar			PayClass;										// �����敪
	uchar			PayMode;										// ���ZӰ��
// MH810100(S) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	uchar			CMachineNo;										// ���Ԍ��@�B��(���ɋ@�B��)
// MH810100(S) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	uchar			KakariNo;										// �W����
	uchar			OutKind;										// ���Z�o��
	uchar			CountSet;										// �ݎԶ���
	uchar			PassCheck;										// ����߽����
	DATA_BASIC		DataBasic;										// NT-NET��{�ް�
	DISCOUNT_DATA	DiscountData[WTIK_USEMAX];						// �����ް�
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
	DETAIL_DATA		DetailData[DETAIL_SYU_MAX];						// �����f�[�^(���׏��)
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// 	ulong			SyuWariRyo;										// ��ʊ�������
	// �N���E�h�����v�Z�Ή��Ŏ�ʊ����𑼂̊����Ɠ��l�Ɉ��������ʂƂ��ĕs�v�ƂȂ����ׁA
	// DiscountData�̗v�f��1���₷���ߍ팸�B
	// �����̍팸�͒ʏ헿���v�Z�̎�ʊ����ɉe�����邪�A�O�q�̃N���E�h�����v�Z�Ή�����
	// �@�ʏ헿���v�Z�̓����e�s�v�Ƃ��ĕύX����Ȃ������ׁA���Ƃ��Ɛ��퓮�삵�Ă��Ȃ��B
	// �@�Ăђʏ헿���v�Z���g�p����ꍇ�A�ʏ헿���v�Z�ł��u��ʊ����𑼂̊����Ɠ��l�Ɉ����v�Ή����K�v�B
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
	ushort			ID;													// 22/56
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	ushort			FullNo1;											// ���ԑ䐔�P
//	ushort			CarCnt1;											// ���ݑ䐔�P
//	ushort			FullNo2;											// ���ԑ䐔�Q
//	ushort			CarCnt2;											// ���ݑ䐔�Q
//	ushort			FullNo3;											// ���ԑ䐔�R
//	ushort			CarCnt3;											// ���ݑ䐔�R
//	ushort			PascarCnt;											// ����ԗ��J�E���g
//	ushort			Full[3];											// ����1�`3������
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//	CAR_CNT_INFO	CarCntInfo;											// �䐔�Ǘ��ǔ�
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	uchar			in_coin[5];											// ���했��������(10,50,100,500,1000)
	uchar			out_coin[5];										// ���했�o������(10,50,100,500,1000)
	ulong			CenterSeqNo;										// �Z���^�[�ǔԁi���Z�j
	uchar			f_escrow;											// ���Z�f�[�^�A���Z���f�[�^��1000�~�̕��߂��������Z�b�g����t���O
	uchar			SeqNo;											// �V�[�P���V����No.
// MH322914(S) K.Onodera 2016/11/25 AI-V�Ή�
//	uchar			reserve[48];									// �\��
// MH322914(E) K.Onodera 2016/11/25 AI-V�Ή�
// MH810100(S) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	ushort			MediaKind1;										// ���(�}��)	0�`99
	uchar			MediaCardNo1[32];								// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	ushort			MediaKind2;										// ���(�}��)	0�`99
	uchar			MediaCardNo2[32];								// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
// MH810100(E) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	// LOG_SECORNUM(0) = LogDatMax[0][1]/LogDatMax[0][0] = RECODE_SIZE/sizeof(Receipt_data) = (0x1000-6)/816 = 5�c10
	// �x�[�X�t�@�C���Ɠ��l��1�Z�N�^������5�̌ʐ��Z��񂪊i�[�o����悤16Byte�P�ʂ̍ő�l�u816Byte�v�̗\�����m��
// MH810100(S) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
//	uchar			reserve[32];									// �\��
	uchar			shubetsu;										// ���	(0=���O���Z�^1�������Z�o�ɐ��Z)
	date_time_rec	TUnpaidPayTime;									// ���Z�����i�����Z�o�ɗp�j
// MH810100(S) 2020/05/29 �Ԕԃ`�P�b�g���X(#4196)
//	uchar			reserve[25];									// �\��
// MH810100(e) 2020/05/29 �Ԕԃ`�P�b�g���X(#4196)
// MH810100(E) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
// MH810100(S) 2020/05/29 �Ԕԃ`�P�b�g���X(#4196)
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// 	ulong			KaiWariDiff;									// ���������̑O�񂩂�̍���
	// ���g�p�ׁ̈ADiscountData�̗v�f��1���₷���ߍ팸�B
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// MH810100(S) 2020/07/09 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX �䐔�Ǘ��ǔԂ�䐔�Ǘ������Ȃ��Ă��t�^����)
//	uchar			reserve[21];									// �\��
	CAR_CNT_INFO	CarCntInfo;											// �䐔�Ǘ��ǔ�
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//	uchar			reserve[13];									// �\��
	uchar			EcResult;										// ���ό���
	uchar			EcErrCode[3];									// �ڍ׃G���[�R�[�h
// GG124100(S) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
// 	uchar			reserve[9];
	SHABAN_INFO		ShabanInfo;										// �Ԕԏ��
	// LOG_SECORNUM(0) = LogDatMax[0][1]/LogDatMax[0][0] = RECODE_SIZE/sizeof(Receipt_data) = (0x1000-6)/1008 = 4�c58
	// 1�Z�N�^������4�̌ʐ��Z��񂪊i�[�o����悤16Byte�P�ʂ̍ő�l�u1008Byte�v�̗\�����m��
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	uchar			reserve[103];
// GG124100(S) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
// 	uchar			reserve[63];
// GG129000(S) T.Nagai 2023/01/16 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
//	uchar			reserve[43];
	ulong			TicketNum;										// ���Ԍ��ԍ��i1�`99999�B0�̏ꍇ�l�����j
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
//	uchar			reserve[39];
	uchar			TInTime_Sec;									// ���ɕb
	uchar			TOutTime_Sec;									// �o�ɕb
	uchar			BeforeTPayTime_Sec;								// �O�񐸎Z�b
	uchar			TUnpaidPayTime_Sec;								// �����Z�b
// GG129000(S) H.Fujinaga 2023/02/18 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
//	uchar			reserve[35];
	QRTICKET_INFO	QrTicketInfo;
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// 	uchar			reserve[25];
	uchar			reserve[1];
// GG129000(E) R.Endo 2023/05/30 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// GG129000(E) H.Fujinaga 2023/02/18 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
// GG129000(E) T.Nagai 2023/01/16 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
// GG124100(E) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// GG124100(E) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// MH810100(E) 2020/07/09 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX �䐔�Ǘ��ǔԂ�䐔�Ǘ������Ȃ��Ă��t�^����)
// MH810100(e) 2020/05/29 �Ԕԃ`�P�b�g���X(#4196)
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
	ulong			WTaxPrice;										// �ېőΏۊz
	ulong			WBillAmount;									// �����z
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
	uchar			RegistNum[14];									// �o�^�ԍ�
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j
	date_time_rec2	Before_Ts_Time;									// �O��s���v�̎���	
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j
} Receipt_data;
#pragma unpack

#define	_PAY_SEEK_LEN(a)	((ulong)&((Receipt_data*)NULL)->a)
#define	_PAY_DATA_LEN(a)	((ulong)sizeof(((Receipt_data*)NULL)->a))

extern	Receipt_data	PayData;									// 1���Z���,�̎��؈��ް�
extern	Receipt_data	PayDataBack;								// 1���Z���,�̎��؈��ް��ޯ�����
extern	Receipt_data	LabelPayData;								// 1���Z���,���و��ް�(ܰ�)
struct Receipt_Data_Sub{
	wari_tiket		wari_data[25];							// ���޽���A�|�����A�������f�[�^
	ulong			pay_ryo;										// �񐔌��A�v���y�C�h�g�p���z
};
extern struct Receipt_Data_Sub		PayData_Sub;					// �󎚗p�ڍג��~�G���A
extern	uchar	Pay_Flg;											// ���Z���~�t���O


typedef union{
	unsigned char	BYTE;					// Byte
	struct{
		unsigned char	YOBI:4;				// Bit 4-7 = �\��
		unsigned char	offset:4;			// Bit 0-3 = �̾��
	} BIT;
} t_PPC_Sub;

typedef struct {
	t_PPC_Sub	ppc_sub_data;
	ppc_tiket	ppc_data_detail[WTIK_USEMAX];
} t_PPC_DATA_DETAIL;

extern t_PPC_DATA_DETAIL	PPC_Data_Detail;
typedef	struct {
	uchar	year;
	uchar	mon;
	uchar	day;
	uchar	hour;
	uchar	min;
} t_TIME_INFO;
typedef struct {
	unsigned short 	Amount;				// �P�ʋ��z
	unsigned short 	Point;				// �P�ʃ|�C���g
	t_TIME_INFO		start;				// �^�p�J�n
	t_TIME_INFO		end;				// �^�p�I��
} t_T_Point_Time;

typedef struct {
	unsigned short Amount;				// �P�ʋ��z
	unsigned short Point;				// �P�ʃ|�C���g
} t_T_Point_Base;
typedef struct {
	unsigned char  Tokubet;				// ���ԃ��[�g�׸�
	unsigned short Amount;				// �P�ʋ��z
	unsigned short Point;				// �P�ʃ|�C���g
	unsigned short disable;				// T�J�[�h����
} t_T_Point_Now;


typedef struct {
	t_T_Point_Time	T_Rate_time;				// ���ԃ��[�g
	t_T_Point_Base	T_Rate_base;				// ��{���[�g
} t_T_POINT_DATA;

#define BIN_KETA_MAX		16

typedef struct{
	uchar		Id_BusCode[2];			// ID�}�[�N/�Ƒԃ}�[�N
	uchar		Company[4];				// ��ЃR�[�h
	uchar 		BIN_Code[20];			// BIN�̌n
	uchar		keta;					// BIN�̌n����
} t_BIN_DEF;

//----------------------------------------------------------------------//
// T�E�@�l�J�[�h�̃G���A�g���E�ǉ��E�폜���s�Ȃ��ꍇ��FLASH�������ݎ���	//
// �T�C�Y�w������Ă���AFLT_TCARD_DATA_SIZE�AFLT_HCARD_DATA_SIZE��		//
// �C�����K�v�ɂȂ�܂��B�ڂ�����fla_def.h���Q�Ƃ��邱��				//
//----------------------------------------------------------------------//
typedef struct {
	uchar		Id_BusCode[2];			// ID�}�[�N/�Ƒԃ}�[�N
	uchar		Company[4];				// ��ЃR�[�h
	uchar 		BIN_start[BIN_KETA_MAX];			// BIN�̌n�J�n
	uchar		BIN_end[BIN_KETA_MAX];			// BIN�̌n�I��
	uchar		Field;					// �C�Ӄt�B�[���h
	uchar		Partner_Code2[4];		// ��g��R�[�h�Q
	uchar		Ptr2_Chk;				// ��g��R�[�h�Q�������
	uchar		Member_Chk;				// ����ԍ��������
	uchar		Reserve[10];			// �\���i�����g���\��j
	uchar		keta;					// BIN�̌n����
} t_BIN_ADD;	

#define BIN_ADD_MAX			20
#define	BLOCK_MAX			2

// T�y�і@�l�J�[�h�̍\���̂�ύX����ꍇ�ɂ́A�ȉ��̃o�[�W���������X�V���邱��
// FLASH���Ƀo�[�W�������Ƃ��ĕێ����܂��B
#define	T_MEMBER_VERTION			1	// T�J�[�h�o�[�W�������
#define	H_MEMBER_VERTION			1	// �@�l�J�[�h�o�[�W�������

extern	Receipt_data	RT_PayData;									// 1���Z���,�̎��؈��ް�

struct	RT_pay_log_rec {
	short			RT_pay_count;									// ���Z��������(RAM)
	short			RT_pay_wtp;										// ���Z����ײ��߲��(RAM)
	Receipt_data	RT_pay_log_dat[RT_PAY_LOG_CNT];					// ���Z�����ޯ̧(RAM)
};

extern	struct	RT_pay_log_rec		RT_PAY_LOG_DAT;

/*--------------------------------------------------------------------------*/
/* �Z���^�[�ǔ�																*/
/*--------------------------------------------------------------------------*/
typedef enum {
	RAU_SEQNO_ENTER,									// �Z���^�[�ǔԁi���Ɂj
	RAU_SEQNO_PAY,										// �Z���^�[�ǔԁi���Z�j
	RAU_SEQNO_TOTAL,									// �Z���^�[�ǔԁi�W�v�j
	RAU_SEQNO_CHANGE,									// �Z���^�[�ǔԁi�ޑK�Ǘ��W�v�j

	RAU_SEQNO_TYPE_COUNT,								// �Z���^�[�ǔԎ�ʐ�
} RAU_SEQNO_TYPE;

extern	ulong	Rau_SeqNo[RAU_SEQNO_TYPE_COUNT];	// �Z���^�[�ǔ�

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
/*--------------------------------------------------------------------------*/
/* �Z���^�[�ǔ�(DC-NET�ʐM													*/
/*--------------------------------------------------------------------------*/
typedef enum {
	DC_SEQNO_QR,										// QR�m��E����f�[�^
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	DC_SEQNO_LANE,										// ���[�����j�^�f�[�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

	// �����@�ǉ��͂�������ɍs��
	DC_SEQNO_TYPE_COUNT,								// �Z���^�[�ǔԎ�ʐ�
} DC_SEQNO_TYPE;

extern	ulong	DC_SeqNo[DC_SEQNO_TYPE_COUNT];		// �Z���^�[�ǔ�
extern	ulong	DC_SeqNo_wk[DC_SEQNO_TYPE_COUNT];	// �Z���^�[�ǔԃ��[�N�G���A

/*--------------------------------------------------------------------------*/
/* �Z���^�[�ǔ�(���A���^�C���ʐM											*/
/*--------------------------------------------------------------------------*/
typedef enum {
	REAL_SEQNO_PAY,										// ���Z�f�[�^
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
	REAL_SEQNO_PAY_AFTER,								// ���Z�f�[�^(������Z)
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	REAL_SEQNO_RECEIPT,									// �̎��؃f�[�^
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	// �����@�ǉ��͂�������ɍs��
	REAL_SEQNO_TYPE_COUNT,								// �Z���^�[�ǔԎ�ʐ�
} REAL_SEQNO_TYPE;

extern	ulong	REAL_SeqNo[REAL_SEQNO_TYPE_COUNT];	// �Z���^�[�ǔ�
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

/*--------------------------------------------------------------------------*/
/* ���Z���~����																*/
/*--------------------------------------------------------------------------*/
extern	Receipt_data	Cancel_pri_work;

#pragma pack
/*--------------------------------------------------------------------------*/
/* �W�v����																	*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	SYUKEI			syukei_log;										// �W�v�ر
	short			dumy;											// not used
} Syu_log;

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// typedef struct {
// 	ushort			Kikai_no;										// �@�B��
// 	ushort			Kakari_no;										// �W����
// 	ST_OIBAN		Oiban;											// �ǔ�
// 	date_time_rec	NowTime;										// ����W�v����
// 	date_time_rec	OldTime;										// �O��W�v����
// 	LOKTOTAL_DAT	loktldat[LOCK_MAX-SYU_LOCK_MAX];				// ���Ԉʒu�ʏW�v(���Ԉʒu��:�R�P�Ԏ��ȍ~)
// } LCKTTL_LOG;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

extern	SYUKEI		TSYU_LOG_WK;									// �s���v���O(30�Ԏ����̏W�v���܂�)�o�^�p���[�N�G���A
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// extern	LCKTTL_LOG	LCKT_LOG_WK;									// �Ԏ����W�v���O(31�Ԏ��ȍ~)�o�^�p���[�N�G���A
// extern	LCKTTL_LOG	lckttl_wk;										// �Ԏ����W�v���O(31�Ԏ��ȍ~)�ҏW�p���[�N�G���A
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

#pragma unpack

/*--------------------------------------------------------------------------*/
/* �ޑK�Ǘ�����																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	TURI_KAN		Mny;											// �ޑK�Ǘ�
	short			dumy;											// not used
} Mny_log;

/*--------------------------------------------------------------------------*/
/* ��݋��ɏW�v����															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	COIN_SYU		cin_mem;										// ��݋��ɏW�v
	short			dumy;											// not used
} coin_log;

/*--------------------------------------------------------------------------*/
/* �������ɏW�v����															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	NOTE_SYU		note_mem;										// �������ɏW�v
	short			dumy;											// not used
} note_log;

// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
///*--------------------------------------------------------------------------*/
///* ���ɗ���																	*/
///*--------------------------------------------------------------------------*/
//#pragma pack
//typedef	struct {
//	ulong			ParkingNo;										// ��������ԏꇂ
//	ulong			PassID;											// �����ID
//	uchar			Contract[8];									// ������_��
//	ushort			Syubet;											// ��������
//	ushort			State;											// ������ð��
//	ulong			Reserve1;										// �\��
//	ulong			Reserve2;										// �\��
//	ushort			MoveMode;										// ���������^�ԋp
//	ushort			ReadMode;										// �����ذ��ײā^ذ�޵�ذ
//	ushort			SYear;											// ������J�n�N
//	uchar			SMon;											// ������J�n��
//	uchar			SDate;											// ������J�n��
//	ushort			EYear;											// ������I���N
//	uchar			EMon;											// ������I����
//	uchar			EDate;											// ������I����
//} PASS_DATA_LOG;
//
//
//typedef	struct {
//	ulong			InCount;										// ���ɒǂ���
//	ushort			Syubet;											// �����敪
//	ushort			InMode;											// ����Ӱ��
//	ulong			LockNo;											// �����
//	ushort			CardType;										// ���Ԍ�����
//	ushort			CMachineNo;										// ���Ԍ��@�B��
//	ulong			CardNo;											// ���Ԍ��ԍ�(�����ǂ���)
//	date_time_rec2	InTime;											// ���ɔN���������b
//	ushort			PassCheck;										// ����߽����
//	ushort			CountSet;										// �ݎԶ���
//	PASS_DATA_LOG	PassData;										// ������ް�
//	union {
//		ushort			Dummy[32];									// ��а
//		struct {
//			ushort	ID;				// 20/54
//			ushort	FullNo1;		// ���ԑ䐔�P
//			ushort	CarCnt1;		// ���ݑ䐔�P
//			ushort	FullNo2;		// ���ԑ䐔�Q
//			ushort	CarCnt2;		// ���ݑ䐔�Q
//			ushort	FullNo3;		// ���ԑ䐔�R
//			ushort	CarCnt3;		// ���ݑ䐔�R
//			ushort	PascarCnt;		// ����ԗ��J�E���g
//			ushort	Full[3];		// ����1�`3������
//			long	CenterSeqNo;	// �Z���^�[�ǔԁi���Ɂj
//			uchar	SeqNo;			// �V�[�P���V����No.
//			uchar	ReserveByte;	// �\��
//			proc_date	ProcDate;	// ��������
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
////			ushort	Reserve[15];	// �\��
//			CAR_CNT_INFO CarCntInfo;// �䐔�Ǘ��ǔ�
//			ushort	Reserve[11];	// �\��
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//		} DT_54;					// 
//	} CMN_DT;
//} enter_log;
//#pragma unpack
//extern	enter_log	Enter_data;
// MH810100(S) 2020/2/18 #3852 RT���Z�f�[�^�̃t�H�[�}�b�g�������(�ĕ]��)
#pragma pack
// MH810100(E) 2020/2/18 #3852 RT���Z�f�[�^�̃t�H�[�}�b�g�������(�ĕ]��)
/*--------------------------------------------------------------------------*/
/* RT���Z�f�[�^																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	unsigned long			ID;					// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	unsigned long			CenterOiban;		// �Z���^�[�ǔ�
	unsigned short			CenterOibanFusei;	// �Z���^�[�ǔԕs���׸�
	uchar					shubetsu;			// ���Z���
	unsigned short			crd_info_rev_no;	// �J�[�h���Rev.No.
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// 	stSendCardInfo_Rev10	crd_info;			// �J�[�h���
// GG129004(S) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
//	stSendCardInfo_Rev11	crd_info;			// �J�[�h���
	stSendCardInfo_RevXX	crd_info;			// �J�[�h���
// GG129004(E) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	// �{�\���̂�ύX����ꍇ�͕K�� RTPay_log_date ���C������
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
} RTPay_log;
extern	RTPay_log		RTPay_Data;			// RT���Z�f�[�^���O

// LOG���R�[�h���t�p
typedef	struct {
	unsigned long		ID;					// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	unsigned long		CenterOiban;		// �Z���^�[�ǔ�
	unsigned short		CenterOibanFusei;	// �Z���^�[�ǔԕs���׸�
	uchar				shubetsu;			// ���Z���
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	unsigned short		crd_info_rev_no;	///< �J�[�h���Rev.No.
	uchar				FormatNo[2];		// ̫�ϯ�Rev.No.	0�`65535
	stDatetTimeYtoSec_t	dtTimeYtoSec;		// �N���������b
} RTPay_log_date;

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
/*--------------------------------------------------------------------------*/
/* RT�̎��؃f�[�^															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	unsigned long			ID;					// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	unsigned long			CenterOiban;		// �Z���^�[�ǔ�
	unsigned short			CenterOibanFusei;	// �Z���^�[�ǔԕs���t���O
	stReceiptInfo			receipt_info;		// �̎��؏��

	// �{�\���̂�ύX����ꍇ�͕K�� RTReceipt_log_date ���C������
} RTReceipt_log;
extern	RTReceipt_log		RTReceipt_Data;		// RT�̎��؃f�[�^���O

// LOG���R�[�h���t�p
typedef	struct {
	unsigned long		ID;					// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	unsigned long		CenterOiban;		// �Z���^�[�ǔ�
	unsigned short		CenterOibanFusei;	// �Z���^�[�ǔԕs���t���O
	ushort				FormatNo;			// �t�H�[�}�b�gRev.No.
	stDatetTimeYtoSec_t	dtTimeYtoSec;		// ���� �N���������b
} RTReceipt_log_date;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

// MH810100(S) 2020/2/18 #3852 RT���Z�f�[�^�̃t�H�[�}�b�g�������(�ĕ]��)
#pragma unpack
// MH810100(E) 2020/2/18 #3852 RT���Z�f�[�^�̃t�H�[�}�b�g�������(�ĕ]��)
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
/*--------------------------------------------------------------------------*/
/* ���ԑ䐔�f�[�^															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec2	Time;			// ��������					(236)
	ushort			GroupNo;		// ��ٰ�ߔԍ�				(236, 58)
	ulong			CurNum;			// ���ݒ��ԑ䐔				(236, 58)
	ulong			TkCurNum[15];	// ������1�`15���ݒ��ԑ䐔(236)
	ulong			RyCurNum[20];	// �������1�`20���ݒ��ԑ䐔(236)
	ulong			State;			// �p�r�ʒ��ԑ䐔�ݒ�		(236, 58)
	ulong			Full[3];		// ����1�`3������			(236, 58)
	union {
		ulong		Reserve[10];	// �\��(236)
		struct {
			ushort	ID;				// 236/58
			ushort	FmtRev;			// �t�H�[�}�b�gRev.��
			ushort	EmptyNo1;		// ��ԑ䐔�P
			ushort	FullNo1;		// ���ԑ䐔�P
			ushort	EmptyNo2;		// ��ԑ䐔�Q
			ushort	FullNo2;		// ���ԑ䐔�Q
			ushort	EmptyNo3;		// ��ԑ䐔�R
			ushort	FullNo3;		// ���ԑ䐔�R
			uchar	SeqNo;			// �V�[�P���V����No.
			uchar	ReserveByte;	// �\��
			ushort	Reserve[11];	// �\��
		} DT_58;					// (58)
	} CMN_DT;
} ParkCar_log;
extern	ParkCar_log		ParkCar_data;								// ���ԑ䐔�f�[�^
extern	ParkCar_log		ParkCar_data_Bk;							// ���ԑ䐔�f�[�^�i�O�񑗐M�f�[�^�j
/*--------------------------------------------------------------------------*/
/* �������ԃf�[�^															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ulong		LockNo;
	t_TIME_INFO	InCarTime;
	uchar		Reserve;
} parking_info;
typedef	struct {
	date_time_rec2	Time;											// ��������
	ushort			DeviceID_BK;
	ushort			Kind;
	ushort			ParkingTerm;
	ushort			ParkingNum;
	parking_info	ParkInfo[8];
	uchar			Reserve[10];
} LongPark_log;
extern	LongPark_log	LongParking_data;
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
typedef	struct {
	date_time_rec2	InTime;		// ���ɓ���
	date_time_rec2	OutTime;	// �o�ɓ���
	ulong			LockNo;		// ���������p���Ԉʒu�ԍ�
	date_time_rec2	ProcDate;	// ��������
	ushort			Ck_Time;	// ���o����
	uchar			Knd;		// ����/����/�S����
	uchar			SeqNo;		// �V�[�P���V����No(NT-NET�d���ɐݒ肷��)
	uchar			Reserve[2];
} LongPark_log_Pweb;
extern	LongPark_log_Pweb	LongParking_data_Pweb;
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
///*--------------------------------------------------------------------------*/
///* Rism�C�x���g�f�[�^														*/
///*--------------------------------------------------------------------------*/
//typedef struct {					// �C�x���g���O�f�[�^
//	date_time_rec	Time;			// ��������
//	unsigned long	Code;			// �C�x���g�R�[�h
//} RismEvent_log;
//extern	RismEvent_log	RismEvent_data;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

///*--------------------------------------------------------------------------*/
///* �W�������f�[�^   	    												*/
///*--------------------------------------------------------------------------*/
//
//#define	ADN_INV_MAX		1000										// �W�������ް�MAX��
//
//
//typedef struct {
//	short		count;												// �o�^����
//	short		wtp;												// �������݈ʒu
//	ushort		aino[ADN_INV_MAX];									// �W��No
//} t_InvalidWriteData;
//
//extern t_InvalidWriteData Attend_Invalid_table;	
/*--------------------------------------------------------------------------*/
/* �W���L���f�[�^   	    												*/
/*--------------------------------------------------------------------------*/

#define	ADN_VAILD_MAX		1000										// �W���L���ް�MAX��

typedef struct {
	short		count;												// �o�^����
	short		wtp;												// �������݈ʒu
	ushort		aino[ADN_VAILD_MAX];									// �W��No
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
} t_ValidWriteData;

extern t_ValidWriteData Attend_Invalid_table;	



/*--------------------------------------------------------------------------*/
/* �J�[�h��� 																*/
/*--------------------------------------------------------------------------*/
#define		NG_CARD_PASS			1								// ���
#define		NG_CARD_TICKET			2								// ���Ԍ�
#define		NG_CARD_CREDIT			3								// �N���W�b�g�J�[�h
#define		NG_CARD_BUSINESS		4								// �r�W�l�X�J�[�h
#define		NG_CARD_AMANO			5								// �A�}�m�W���J�[�h�s��
#define		OK_CARD_AMANO			6								// �A�}�m�W���J�[�h����

/*--------------------------------------------------------------------------*/
/* �s�����O�����f�[�^�i�P�����j												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			Card_type;										// �J�[�h���
	union card_no {													// �J�[�h�ԍ�
		ulong		card_s;											// ����A���Ԍ��A�W������									
		uchar		card_c[10];										// �ڼޯāA�޼�Ƚ����(BCD�E�l)
	}Card_No;
	date_time_rec	NowTime;										// ���o����
} NGLOG_DATA;
extern	NGLOG_DATA	nglog_data;
/*--------------------------------------------------------------------------*/
/* �s�����O����																*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* ���o�ɃC�x���g��� 														*/
/*--------------------------------------------------------------------------*/
enum{																/* �C�x���g��� */
	IOLOG_EVNT_NONE = 0,											//  0:�C�x���g������
	IOLOG_EVNT_FLAP_UP_STA,											//  1:�t���b�v�㏸�J�n
	IOLOG_EVNT_FLAP_UP_FIN,											//  2:�t���b�v�㏸����
	IOLOG_EVNT_FLAP_DW_STA,											//  3:�t���b�v���~�J�n
	IOLOG_EVNT_FLAP_DW_FIN,											//  4:�t���b�v���~����
	IOLOG_EVNT_FLAP_UP_UFN,											//  5:�t���b�v�㏸����
	IOLOG_EVNT_FLAP_DW_UFN,											//  6:�t���b�v���~����
	IOLOG_EVNT_LOCK_CL_STA,											//  7:���փ��b�N�J�n
	IOLOG_EVNT_LOCK_CL_FIN,											//  8:���փ��b�N����
	IOLOG_EVNT_LOCK_OP_STA,											//  9:���փ��b�N�J�J�n
	IOLOG_EVNT_LOCK_OP_FIN,											// 10:���փ��b�N�J����
	IOLOG_EVNT_LOCK_CL_UFN,											// 11:���փ��b�N����
	IOLOG_EVNT_LOCK_OP_UFN,											// 12:���փ��b�N�J����
	IOLOG_EVNT_ENT_GARAGE,											// 13:����
	IOLOG_EVNT_OUT_GARAGE,											// 14:�o��
	IOLOG_EVNT_RENT_GARAGE,											// 15:�ē���
	IOLOG_EVNT_FORCE_STA,											// 16:�����o�ɊJ�n
	IOLOG_EVNT_FORCE_FIN,											// 17:�����o�Ɋ���
	IOLOG_EVNT_OUT_ILLEGAL,											// 18:�s���o��
	IOLOG_EVNT_AJAST_STA,											// 19:���Z�J�n
	IOLOG_EVNT_AJAST_FIN,											// 20:���Z����
	IOLOG_EVNT_AJAST_STP,											// 21:���Z���~
	IOLOG_EVNT_OUT_ILLEGAL_START,									// 22:�s���o�ɔ���
	IOLOG_EVNT_ERR_REDSTACK_R_Ge,									// 23:���C���[�_ ���[�_���J�[�h�l�܂� ����
	IOLOG_EVNT_ERR_REDSTACK_R_Re,									// 24:���C���[�_ ���[�_���J�[�h�l�܂� ����
	IOLOG_EVNT_ERR_REDSTACK_P_Ge,									// 25:���C���[�_ �v�����^�[���J�[�h�l�܂� ����
	IOLOG_EVNT_ERR_REDSTACK_P_Re,									// 26:���C���[�_ �v�����^�[���J�[�h�l�܂� ����
	IOLOG_EVNT_ERR_COINDISPENCEFAIL_Ge,								// 27:�R�C�����o�ُ� ����
	IOLOG_EVNT_ERR_COINDISPENCEFAIL_Re,								// 28:�R�C�����o�ُ� ����
	IOLOG_EVNT_ERR_COINSAFE,										// 29:�d�ݕԋp�R�}���h���M��̒╜�d����
	IOLOG_EVNT_ERR_NOTEJAM_Ge,										// 30:�����l�� ����
	IOLOG_EVNT_ERR_NOTEJAM_Re,										// 31:�����l�� ����
	IOLOG_EVNT_ERR_NOTEREJECT_Ge,									// 32:�������o���ُ� ����
	IOLOG_EVNT_ERR_NOTEREJECT_Re,									// 33:�������o���ُ� ����
	IOLOG_EVNT_ERR_NOTESAFE,										// 34:�����ԋp�R�}���h���M��̒╜�d����
	IOLOG_EVNT_MAX													// xx:�C�x���g��
};

// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
///*--------------------------------------------------------------------------*/
///* ���o�Ƀ��O�����f�[�^�i�P�����j											*/
///*--------------------------------------------------------------------------*/
//typedef struct {
//	date_time_rec	NowTime;										// ��������
//	ushort			Event_type;										// ���o�ɃC�x���g���
//	union io_inf {													// ���o�ɏڍ׏��
//		uchar		io_c[4];										// ���g�p
//		ushort		io_s[2];										// ���g�p
//		ST_OIBAN2	io_l;											// �ǂ��ԁi���Z���~�A�����j�^���ԍ��i���Z�J�n�j
//	}IO_INF;
//} IOLOG_DATA;
//
///*--------------------------------------------------------------------------*/
///* ���o�Ƀ��O����															*/
///*--------------------------------------------------------------------------*/
//typedef	struct {
//	IOLOG_DATA		iolog_data[IO_EVE_CNT];							// ���o�Ƀ��O�����f�[�^�i20���j
//	ushort			is_rec:2;										// ���o�Ƀ��O�L�^��(0:�����{/1:�L�^��/2:�L�^����)
//	ushort			room:14;										// ���o�Ƀ��O�Ԏ��ԍ�
//} IoLog_Data;
//
//// MH322917(S) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
//// ���ӎ����FIo_log_rec�\���͓̂��o�Ƀ��O�������݂�SRAM�̈��胁���o�ύX���Ȃ�����
//// �{�G���A�̕ύX���K�v�ȏꍇ�̓o�[�W�����A�b�v�Ń��O�����O�o�[�W�����uLOG_VERSION�v���A�b�v�f�[�g���A
//// opetask()�̒���LOG_VERSION�ɂ������S���O�N���A������ǉ����邱��
//// MH322917(E) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
//struct	Io_log_rec {
//	ushort			iolog_count;									// ���o�Ƀ��O��������
//	ushort			iolog_wtp;										// ���o�Ƀ��O����ײ��߲��
//	ushort			LockIndex;
//	IoLog_Data		iolog_list[IO_LOG_CNT];							// ���o�Ƀ��O�����ޯ̧
//};
//extern	struct	Io_log_rec	IO_LOG_REC;
//extern	IoLog_Data	Io_log_wk;
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
/*--------------------------------------------------------------------------*/
/* Suica�ʐM����															*/
/*--------------------------------------------------------------------------*/
// �o�b�t�@�T�C�Y�ύX
#define	SUICA_LOG_MAXSIZE			48000	// �ʐM۸��ޯ̧����
#define	SUICA_LOG_NEAR_PRINT_SIZE	4000	// ���߂̃��O�̈󎚃T�C�Y

typedef union{
	unsigned char	BYTE;					// Byte
	struct{
		unsigned char	YOBI:6;				// Bit 2-7 = �\��
		unsigned char	log_Write_Start:1;	// Bit 1 = ۸ފJ�n�׸�
		unsigned char	write_flag:1;		// Bit 0 = �������׸�
	} BIT;
} t_Suica_log;

struct suica_log_rec{
	ushort	log_wpt;						// log�������߲���
	short	log_time_old;					// �ŌÂ̏������ް��ێ��̈�
	char	log_Buf[SUICA_LOG_MAXSIZE];		// LOG�ް��i�[�̈�
	char	cNull;
	t_Suica_log	Suica_log_event;			// suica۸޲����
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
	//	   �r���Ƀ����o��}�����Ȃ�����
};

extern	struct	suica_log_rec	SUICA_LOG_REC;
extern	struct	suica_log_rec	SUICA_LOG_REC_FOR_ERR;	// ���ψُ킪�����������̃��O���

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern ulong	Edy_OpeDealNo;				// Main�Ǘ��pEdy����ʔ�
//
//#define	EDY_ARMLOG_MAX	20
//
//typedef struct {
//	uchar				Kikai_no;									// �@�B�m���i���ԍ�0,1-20�j
//	uchar				TD_Kikai_no;								// �����@ �@�B�ԍ��i1�`20�B0�̏ꍇ�l�����j
//	ulong				TicketNum;									// ���Ԍ��ԍ��i1�`99999�B0�̏ꍇ�l�����j
//	uchar				syubet;										// ���
//	teiki_use			teiki;										// ����ް�
//	date_time_rec		TOutTime;									// ���s����
//	uchar				Edy_CardNo[8];								// Suica ID
//	ulong				Pay_Ryo;									// ���ۂ̌��ϊz
//	ulong				Pay_Befor;									// ���Z�OSuica�c��
//	ulong				Pay_After;									// ���ό�Suica�c��
//	ulong				Edy_Deal_num;								// ����ʔ�
//	ushort				Card_Deal_num;								// ���ގ���ʔ�
//	uchar				High_Terminal_ID[4];						// ��ʒ[��ID
//} edy_arm_log;
//
//struct Edy_Arm_Log{
//	short		log_cnt;											// ���O��������
//	ushort		log_wpt;											// log�������߲���
//	edy_arm_log	log_Buf[EDY_ARMLOG_MAX];							// LOG�ް��i�[�̈�
//};
//extern	edy_arm_log		Edy_Arm_log_work;									// �╜�d���O���[�N�o�b�t�@
//
//
//#define	EDY_SHIMELOG_MAX	40
//
//typedef struct {
//	date_time_rec		ShimeTime;									// ���ߎ��{����
//	uchar				High_Terminal_ID[4];						// ��ʒ[��ID
//	ulong				Electron_edy_cnt;							// Edy���Z  ���񐔁�
//	ulong				Electron_edy_ryo;							// �@�@�@�@ �����z��
//	ulong				Electron_Arm_cnt;							// Edy�װю��  ���񐔁�
//	ulong				Electron_Arm_ryo;							// �@�@�@�@     �����z��
//	uchar				Shime_OKorNG;								// ���ߌ��ʁi0:OK/1:NG�j
//	uchar				fg_DataAri;									// EDY_SHIME_LOG_NOW �ł̂ݎg�p�����ް������׸ށi1=����j
//																	// �i��d��LOG Edy����LOG�o�^�p�j
//																	// EM���琸�Z��&���z��M�`LOG�o�^�܂ł̊�1�ƂȂ�B
//} edy_shime_log;
//
//struct Edy_Shime_Log{
//	short		log_cnt;											// ���O��������
//	ushort		log_wpt;											// log�������߲���
//	edy_shime_log	log_Buf[EDY_SHIMELOG_MAX];						// LOG�ް��i�[�̈�
//};
//
//extern	edy_shime_log	EDY_SHIME_LOG_NOW;
//
//extern	uchar	Edy_High_Terminal_ID[4];							// ��ʒ[��ID�ۑ��ر
//
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		CRE_SALENG_MAX			10				/* �N���W�b�g���㋑�ۍő匏��		*/
#define		CRE_SHOP_ACCOUNTBAN_MAX	20				/* �N���W�b�g����ԍ��ő啶����		*/
typedef struct {
	uchar			MediaCardNo[30];								// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			MediaCardInfo[16];								// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			reserve[2];										// �\��

} t_MediaInfoCre2;	// ���Ɂ^���Z�@�}�̏��
struct	DATA_BK{									/*									*/
	unsigned long	ryo;							/* �ޯ����ߗ���						*/
	date_time_rec	time;							/* ���Z����							*/
	ulong			slip_no;						/* �ڼޯĶ��ޓ`�[�ԍ�				*/
	char			AppNoChar[6];					// �ڼޯĶ��ޏ��F�ԍ�(�p����)
	char			shop_account_no[20];			/* �ڼޯĶ��މ����X����ԍ�			*/
	uchar			CMachineNo;										// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
	uchar			PayMethod2;										// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
	uchar			PayClass;										// �����敪(0�����Z/1���Đ��Z/2�����Z���~/3���Đ��Z���~, 8=���Z�O, 9=���Z�r��
	uchar			PayMode;										// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
	ushort			FlapArea;										// �t���b�v�V�X�e��	���		0�`99
	ushort			FlapParkNo;										// 					�Ԏ��ԍ�	0�`9999
	ulong			Price;											// ���ԗ���			0�`
	ulong			OptionPrice;									// ���̑�����		0�`
	ulong			Discount;										// �����z(���Z)
	ulong			CashPrice;										// ��������			0�`
	ulong			PayCount;										// ���Z�ǔ�(0�`99999)
	ushort			MediaKind;										// ���(�}��)	0�`99
	t_MediaInfoCre2	Media;											// ���Z�}�̏��1
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
	//	   �r���Ƀ����o��}�����Ȃ�����
};													/*									*/
													/*									*/
typedef struct {									/* �װ�����ް� �ر					*/
	char	ken;									/* ���㋑�ی���						*/
	struct	DATA_BK back[CRE_SALENG_MAX];			/*									*/
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
	//	   �r���Ƀ����o��}�����Ȃ�����
} CRE_SALENG;										/* �@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@ */
extern	CRE_SALENG	cre_saleng;						// ���㋑�ۃf�[�^�P�O����
													/*									*/
//
// ������W�`�F�b�N����p�G���A
//
typedef struct {
	short			UmuFlag;						// ����ρH�t���O(A3���M����ON)
	struct	DATA_BK	back;							// ������
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͗\���̈���g�p���邱��!!!
	//	   �r���Ƀ����o��}�����Ȃ�����
} CRE_URIAGE;
extern CRE_URIAGE	cre_uriage;
extern	ulong		cre_slipno;						// �N���W�b�g�[�������ʔ�

// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//extern struct clk_rec	ClkLastSend;				/* �J�[�h���v���ŏI���M����	*/
//
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//
// �N���W�b�g���p���חp�G���A
//
typedef struct {
	date_time_rec	PayTime;										// ���Z����
	char			card_name[12];									// �ڼޯĶ��މ�З���
	char			card_no[20];									// �ڼޯĶ��މ���ԍ�
	ulong			slip_no;										// �ڼޯĶ��ޓ`�[�ԍ�
	char			AppNoChar[6];									// �ڼޯĶ��ޏ��F�ԍ�(�p����)
	char			shop_account_no[20];							// �ڼޯĶ��މ����X����ԍ�
	ulong			pay_ryo;										// �ڼޯĶ��ޗ��p���z
} meisai_log;
extern meisai_log	meisai_work;		// ���p���׃��[�N

typedef	struct {
	ushort		event1;			/* control event */
	ushort		status1;		/* control status */
	ulong		event2;			/* data event to send */
	ulong		status2;		/* data status to send */
} NTNET_AUTO_TRANCTRL;

extern	NTNET_AUTO_TRANCTRL	ntTranCtrl;

typedef	struct {
	struct	clk_rec		last0;		/* last batch send time */
	struct	clk_rec		last1;		/* last transaction(number of parked cars) time */
} NTNET_AUTO_TRANTIME;

extern	NTNET_AUTO_TRANTIME	ntTranTime;

extern	short	NT_pcars_timer;

typedef struct {
	date_time_rec	SyuTime;										// �W�v����
	ushort			cnt;											// �����p��
	ulong			ryo;											// �����p���z
} syu_dat;

typedef struct {
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����

	uchar			ptr;											// �ŐV�̊i�[�ʒu
	uchar			cnt;											// Suica/Edy�W�v�̊i�[��

	syu_dat			now;											// �����̏W�v
	syu_dat			bun[SYUUKEI_DAY];								// �ߋ�40�����̏W�v�i�{ work�p�i�����N���A���j�j
} syuukei_info;

// MH321800(S) G.So IC�N���W�b�g�Ή�
typedef struct {
	date_time_rec	SyuTime;										// �W�v����
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//	// cnt[EC_BRAND_TOTAL_MAX - 1] �� ryo[EC_BRAND_TOTAL_MAX - 1]�ɂ݂Ȃ����ς̉񐔂Ƌ��z���i�[����
//// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// MH810105 GG119202(E) ������������W�v�d�l���P
	ushort			cnt[EC_BRAND_TOTAL_MAX];						// �����p��
	ulong			ryo[EC_BRAND_TOTAL_MAX];						// �����p���z
	ushort			cnt2[EC_BRAND_TOTAL_MAX];						// ���A���[����
	ulong			ryo2[EC_BRAND_TOTAL_MAX];						// ���A���[�����z
	ulong			unknownTimeCnt;									// ���ϓ����s���W�v��
} syu_dat2;

// MH810105 GG119202(S) ������������W�v�d�l���P
// �݂Ȃ����ϏW�v
#define	sp_minashi_cnt			cnt2[EC_SAPICA_USED-EC_EDY_USED]
#define	sp_minashi_ryo			ryo2[EC_SAPICA_USED-EC_EDY_USED]

// ������������W�v�i0�`2,4�`6��cnt2,ryo2�͎g�p���Ȃ����Ɓj
#define	sp_miryo_pay_ok_cnt		cnt2[EC_CREDIT_USED-EC_EDY_USED]
#define	sp_miryo_pay_ok_ryo		ryo2[EC_CREDIT_USED-EC_EDY_USED]
#define	sp_miryo_unknown_cnt	cnt2[EC_TCARD_USED-EC_EDY_USED]
#define	sp_miryo_unknown_ryo	ryo2[EC_TCARD_USED-EC_EDY_USED]
// MH810105 GG119202(E) ������������W�v�d�l���P
typedef struct {
	date_time_rec	NowTime;										// ����W�v����
	date_time_rec	OldTime;										// �O��W�v����

	uchar			ptr;											// �ŐV�̊i�[�ʒu
	uchar			cnt;											// ���σ��[�_�W�v�̊i�[��

	syu_dat2		next;											// ����f�[�^			�F�����̏W�v
	syu_dat2		now;											// ����f�[�^			�F�����̏W�v
	syu_dat2		bun[SYUUKEI_DAY_EC];							// ����f�[�^			�F�ߋ�64�����̏W�v�i�{ work�p�i�����N���A���j�j
} syuukei_info2;
// MH321800(E) G.So IC�N���W�b�g�Ή�

struct Syuukei_log_rec {
	syuukei_info	sca_inf;										// �r���������W�v���
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	syuukei_info	edy_inf;										// �d�����W�v���
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) G.So IC�N���W�b�g�Ή�
	syuukei_info2	ec_inf;											// �d�b�W�v���
// MH321800(E) G.So IC�N���W�b�g�Ή�
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
};

extern	struct Syuukei_log_rec	Syuukei_sp;
extern	syu_dat bk_syu_dat;
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern	syu_dat2 bk_syu_dat_ec;
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//typedef struct {
//	uchar			Brand;											// ���d�p�A���[������u�����h
//	ulong			Ryo;											// ���d�p�A���[������z
//} EMoneyAlarmFukuden;
//extern	EMoneyAlarmFukuden	EcAlarm;								// ���σ��[�_���d�p�A���[������f�[�^
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
typedef struct {
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//	ulong				WPlace;										// �Ԏ��ԍ�
//// MH810100(S) S.Nishimoto 2020/08/20 �Ԕԃ`�P�b�g���X(#4602:���ϐ��Z���~�i���j�ŎԔԏ�񂪈������Ȃ�)
//	uchar				CarSearchFlg;								// �Ԕ�/�����������
//	uchar				CarSearchData[6];							// �Ԕ�/���������f�[�^
//	uchar				CarNumber[4];								// �Ԕ�(���Z)
//	uchar				CarDataID[32];								// �Ԕԃf�[�^ID
//// MH810100(E) S.Nishimoto 2020/08/20 �Ԕԃ`�P�b�g���X(#4602:���ϐ��Z���~�i���j�ŎԔԏ�񂪈������Ȃ�)
//	uchar				update_mon;									// �X�V����
//	uchar				syu;										// �Ԏ�
//	ulong				WPrice;										// ���ԗ����i����X�V�̎��؂̏ꍇ�A�X�V�����j
//	ulong				PayPrice;									// �݂Ȃ����ϊz
//	date_time_rec		TInTime;									// ���ɓ���
//	ushort				EcDeemedBrandNo;							// �݂Ȃ����ϔ����u�����hNo
//// GG119202(S) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
////	date_time_rec		EcDeemedDate;								// �݂Ȃ����ϔ�������
//	date_time_rec		TOutTime;									// �o�ɓ���
//// GG119202(E) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
// MH810100(S) S.Nishimoto 2020/08/20 �Ԕԃ`�P�b�g���X(#4602:���ϐ��Z���~�i���j�ŎԔԏ�񂪈������Ȃ�)
	uchar				CarSearchFlg;								// �Ԕ�/�����������
	uchar				CarSearchData[6];							// �Ԕ�/���������f�[�^
	uchar				CarNumber[4];								// �Ԕ�(���Z)
	uchar				CarDataID[32];								// �Ԕԃf�[�^ID
// MH810100(E) S.Nishimoto 2020/08/20 �Ԕԃ`�P�b�g���X(#4602:���ϐ��Z���~�i���j�ŎԔԏ�񂪈������Ȃ�)
	ulong				PayPrice;									// �݂Ȃ����ϊz
	ushort				EcDeemedBrandNo;							// �݂Ȃ����ϔ����u�����hNo
	uchar				MiryoFlg;									// 1:������, 2:�����c���Ɖ
	uchar				dummy[25];
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
} EcDeemedFukudenLog;
extern	EcDeemedFukudenLog	EcDeemedLog;							// �݂Ȃ����ϕ��d���O�f�[�^
extern	Receipt_data	EcRecvDeemedData;							// ���ϐ��Z���~(������)�f�[�^
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105 GG119202(S) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
extern	uchar			EcReaderVerUpFlg;							// ���σ��[�_�o�[�W�����A�b�v���t���O
// MH810105 GG119202(E) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�

extern	const	uchar	Mif_Read_Sector[8];							/* �Ǐo����w��ر */
extern	const	uchar	Mif_Read_AccessBlock[8];					/* ������ۯ��w��ر	*/
extern	const	uchar	Mif_W_DataSpecify[3][8];					/* �������ް��w��ر */
extern	const	uchar	Mif_W_Verify[3][8]; 						/* ���������̧��ر */
extern	const	uchar	Mif_AccessKey[8]; 							/* �������ر */
#define	SUICA_QUE_MAX_SIZE 256
#define	SUICA_QUE_MAX_COUNT 10

extern	ushort		CLK_REC_msec;		// ���v�~���b�l

typedef struct{
	unsigned short	readpt;
	unsigned short	writept;
	unsigned short	count;
	unsigned char	rcvdata[SUICA_QUE_MAX_COUNT][SUICA_QUE_MAX_SIZE];
}t_SUICA_RCV_QUE;

extern t_SUICA_RCV_QUE	suica_rcv_que;

extern	ushort	LongTermParkingPrevTime[2];
/* ���u�����ݒ胍�O */
#define	REMOTELOG_COUNT_MAX		1000
typedef	struct {
	date_time_rec	rcv_Time;									// �ݒ����
	unsigned short	addr;										// �ݒ�A�h���X
	unsigned long	rcv_data;									// �ݒ�f�[�^
} t_Change_data;

extern t_Change_data	remote_Change_data;

/* ��ʖ��ɐݒ�𕪂��鍀��(46-0001�E46-0002) */
enum {
	ROCK_CLOSE_TIMER = 0,
	FLAP_UP_TIMER,
	FLP_ROCK_INTIME,			// ���b�N�E�t���b�v�㏸�E���O�^�C��������
	LAG_PAY_PTN,				// ���O�^�C���A�b�v��̗���
	WARI_PAY_PTN,				// ���Ԋ���/�����̗���
	TYUU_DAISUU_MODE,			// ���ԑ䐔�Ǘ�����
	IN_CHK_TIME,				// ���ɔ���^�C��
	
	SYUSET_MAX,
};

/* �������[�h���i�[�G���A */
typedef struct{
	unsigned char	dir_chkresult[5];	// �f�B���N�g��
	unsigned char	fn_chkresult[30]; 	// ���ʏ��t�@�C����
	unsigned char	pt_no;				// ���[�h�i���o�[
	unsigned char	Chk_mod;			// �`�F�b�N���̍���
	unsigned char	Chk_str1[15];		// �Œ蕶����@
	unsigned char	Chk_str2[15];		// �Œ蕶����A
} t_Chk_info;
extern	t_Chk_info Chk_info;
/* �������ʊi�[�G���A */
typedef struct{
	unsigned short	Kakari_no;
	unsigned long	Chk_no;
	unsigned short	Card_no;
	date_time_rec	Chk_date;
	unsigned short	ptn;
	unsigned char set[8];
	unsigned char moj[16];
	unsigned char key_disp[2];
	unsigned char mag[4];
	unsigned char led_shut[2];
	unsigned char sw[8];
	unsigned char r_print[6];
	unsigned char j_print[5];
	unsigned char ann[3];
	unsigned char sig[1];
	unsigned char coin[3];
	unsigned char note[3];
} t_Chk_result;
extern t_Chk_result Chk_result;
#define		CHK_RESULT01		6								/* ��������01�ݒ萔			*/
#define		CHK_RESULT02		3								/* ��������02�ݒ萔			*/
#define		CHK_RESULT03		4								/* ��������03�ݒ萔			*/
#define		CHK_RESULT04		15								/* ��������04�ݒ萔			*/
#define		CHK_RESULT05		8								/* ��������05�ݒ萔			*/
#define		CHK_RESULT06		7								/* ��������06�ݒ萔			*/
#define		CHK_RESULT07		5								/* ��������07�ݒ萔			*/
#define		CHK_RESULT08		6								/* ��������08�ݒ萔			*/
#define		CHK_RESULT09		3								/* ��������09�ݒ萔			*/
#define		CHK_RESULT10		8								/* ��������10�ݒ萔			*/
#define		CHK_RESULT11		8								/* ��������11�ݒ萔			*/
#define		CHK_RESULT12		2								/* ��������12�ݒ萔			*/

#define		CHK_RESULT_NUMMAX	13
typedef struct{
	date_time_rec	Chk_date;		// ������
	unsigned short	Kakari_no;		// �Ј��ԍ�
	unsigned char	Machine_No[6];		// ���i���@
	unsigned char	Model[8];			// ���f���H
	unsigned char	System;			// �V�X�e���\��
	unsigned char	Sub_Money;		// �T�u�`���[�u����
} t_Chk_Res01;

typedef struct{
	// �\�t�g�E�F�A�o�[�W����
	unsigned char Version[3][8];
} t_Chk_Res02;

typedef struct{
	unsigned char	Dip_sw[3][4];
	unsigned char	Dip_sw6;
} t_Chk_Res03;

typedef struct{
	unsigned char	res_dat[15];
} t_Chk_Res04;

typedef struct{
	unsigned char	Mojule[27][8];				// ���W���[���i���o�[
} t_Chk_Res05;

typedef struct{
	unsigned char	res_dat[7];
} t_Chk_Res06;

typedef struct{
	unsigned char	res_dat[5];
} t_Chk_Res07;

typedef struct{
	unsigned char	res_dat[6];
} t_Chk_Res08;

typedef struct{
	unsigned char	res_dat[3];
} t_Chk_Res09;

typedef struct{
	unsigned char	res_dat[8];
} t_Chk_Res10;

typedef struct{
	unsigned char	res_dat[8];
} t_Chk_Res11;

typedef struct{
	unsigned char	res_dat[2];
} t_Chk_Res12;

typedef struct{
	char	Chk_Res00;
	t_Chk_Res01		Chk_Res01;
	t_Chk_Res02		Chk_Res02;
	t_Chk_Res03		Chk_Res03;
	t_Chk_Res04		Chk_Res04;
	t_Chk_Res05		Chk_Res05;
	t_Chk_Res06		Chk_Res06;
	t_Chk_Res07		Chk_Res07;
	t_Chk_Res08		Chk_Res08;
	t_Chk_Res09		Chk_Res09;
	t_Chk_Res10		Chk_Res10;
	t_Chk_Res11		Chk_Res11;
	t_Chk_Res12		Chk_Res12;
} t_Chk_res_ftp;

extern t_Chk_res_ftp Chk_res_ftp;
extern	char	*Chk_result_p[];												/* ���Ұ������ð���				*/
/*--------------------------------------------------------------------------*/
/* FTP���[�N�G���A															*/
/*--------------------------------------------------------------------------*/
extern	char	ETC_cache[65536];
extern	char	FTP_buff[];
extern	char	SCR_buff[8192];
extern	ushort		xSTACK_TBL[TSKMAX];							/* �^�X�N���̃X�^�b�N�T�C�Y�e�[�u�� */
extern	ulong		xSTK_RANGE_BTM;								/* �X�^�b�N�I���A�h���X */

#define	ENCKEY_NUM	(1)											/* �Í����L�[�̌� */
extern	uchar	Encryption_Key[ENCKEY_NUM][16];					/* �Í����L�[ */
extern	uchar	ryo_test_flag;




// SNTP�Œ�p�����[�^
#define	_SNTP_PORTNO			123
#define	_SNTP_TOUT_IBK			5		// 5 sec
#define	_SNTP_RESPONSE_TOUT		10		// 10 sec
#define	_SNTP_WAI_MAX			5		// 1+5 times
#define	_SNTP_RETRY_MAX			2		// 1+2 times
#define	_SNTP_REQ_MAX			3		// 3 times
#define	_SNTP_RETRY_INT			60L		// 60 min
#define	_SNTP_d_LIMIT			1600	// 1600 msec (���[�g�x��臒l)
#define	_SNTP_UPPER_LIMIT		600		// 600 sec (�덷�������)
#define	_SNTP_LOWER_LIMIT		500		// 500 msec (�덷��������)


/*--------------------------------------------------------------------------*/
/* ���u�_�E�����[�h���[�N�G���A												*/
/*--------------------------------------------------------------------------*/
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//#define TEMP_BUFF_MAX			65536		// 64K
#define TEMP_BUFF_MAX			(128*1024)		// 128K
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
extern uchar	g_TempUse_Buffer[TEMP_BUFF_MAX];
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
extern uchar	g_TempUse_Buffer2[TEMP_BUFF_MAX];
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
typedef enum{
	FTP_REQ_NONE,			// 0:�v���Ȃ�
	FTP_REQ_NORMAL,			// 1:FTP�v������
	FTP_REQ_WITH_POWEROFF,	// 2:FTP�v������(�d�f�v)
	PRM_CHG_REQ_NONE = 10,		// 0:�ύX�Ȃ�
	PRM_CHG_REQ_NORMAL,			// 1:�ύX����
} eFTP_REQ_TYPE;
extern eFTP_REQ_TYPE 	g_PrmChgFlg;						// �p�����[�^�ύX�L��
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

/*--------------------------------------------------------------------------*/
/* ���������␳��񗚗�														*/
/*--------------------------------------------------------------------------*/
enum {
	_T_DELAY,						// �␳�x����
	_T_WATCH,						// �␳�����Ď���
	_T_BUSY,						// SNTP�����҂�
	_T_LAG,							// msec������
};

enum {
	_SNTP_RESULT_CODE = 0x5300,
	SNTP_LIMIT_OVER,				// �␳�������				�F��
	SNTP_NORMAL_REVISE,				// �����␳���{				�F��
	SNTP_NORMAL_SKIP,				// �����␳�����{			�F��
	SNTP_DELAYED_REVISE,			// �x������E�����␳���{	�F��
	SNTP_DELAYED_SKIP,				// �x������E�����␳�����{	�F��
	SNTP_DELAYED_RETRY,				// �x������E�ď���			�F��
	SNTP_BADDATA,					// �����f�[�^				�F�~
	SNTP_NOTREADY_WAIT,				// �ʐM�ҋ@					�F�{
	SNTP_GUARD_TIME,				// �[���������X�V�֎~���ԑ�	�F��
	SNTP_TIMEOUT,					// �����^�C���A�E�g			�F�|
};

typedef struct {
	uchar		type;				// ���{�^�C�v
	uchar		reqno;				// ���{��
	ushort		fExec;				// ���{����
	date_time_rec2	Req_Date;		// ���������␳�v������
	date_time_rec2	Rsp_Date;		// ���������␳���s����
	ulong		d;					// ���[�g�x��(ms)
	long		t;					// �[������(ms)
} NTP_log;

typedef struct {
	uchar		mode;				// 1=�L��, 0=����
	uchar		state;				// ���
	uchar		stop;				// 1=��~
	uchar		req_adj;			// 1=�␳�v��
	uchar		method;				// �����擾����={0:�����A1:����}
	uchar		waicnt;				// PPP�ڑ��҂���
	uchar		trycnt;				// �g���C��
	uchar		reqcnt;				// �v����(���[�g�x��NG�p)
	uchar		skip_adj;			// �␳�ȗ�
	ulong		delay;				// �x������
	NTP_log		log;				// 1�������O
	struct clk_rec	tmpClk;
	ushort		tmpMsec;
	short		lag_count;			// �����␳�����J�E���^(msec)
	char		tmp[38];
} t_SYSTIME_ADJ;
extern t_SYSTIME_ADJ	SysTimeAdj;
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
/*--------------------------------------------------------------------------*/
/* �������Ԍ��o�@�\�֘A														*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort		time1;// ���o����1 �ݒ���
	ushort		time2;// ���o����2 �ݒ���
	ushort		cng_count;// �Ԏ������p�̃J�E���^
	uchar		f_prm_cng;// �ݒ�ύX���o�t���O
	
} t_longpark_prm_cng;
// �������ԃf�[�^�g�p���A�r���Őݒ肪�ύX���ꂽ���A���ݔ������Ă��钷�����Ԃ���U������
// �V�����ݒ�ł̒������ԏ�ԂɍX�V���邽�߂̏��
extern t_longpark_prm_cng LongPark_Prmcng;
#define	LONG_PARK_TIME1_DEF		48	// ���o����1�̃f�t�H���g(03-0135�B�C�D�E=0000�̎�048�œ��삳���邽�߂̐ݒ�l)
// �������ԃ`�F�b�N(�A���[���f�[�^���o)�Ńh�A���ɃZ���^�[�ɑ΂��đ��M���鏈���̔���p
extern ushort LongParkCheck_resend_count;// �Ԏ������p�̃J�E���^
extern uchar f_LongParkCheck_resend;// ����t���O
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
extern uchar f_ec_reader_waiting_wakeup;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
extern void mnt_SetFtpFlag( eFTP_REQ_TYPE req );
extern eFTP_REQ_TYPE mnt_GetRemoteFtpFlag( void );
extern	uchar	remotedl_work_update_get( void );
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
typedef struct {
	ushort				id;					// QR�R�[�hID
	ushort				rev;				// QR�R�[�h�t�H�[�}�b�gRev.
	ushort				enc_type;			// QR�R�[�h�G���R�[�h�^�C�v
	ushort				info_size;			// QR�f�[�^���(�p�[�X�f�[�^)�T�C�Y
	lcdbm_rsp_QR_com_u	QR_data;			// �������z�`�\��
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	ushort				qr_type;			// QR�R�[�h�t�H�[�}�b�g�^�C�v
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
} tBarcodeDetail;

typedef struct {
	tBarcodeDetail		Barcode;			// �o�[�R�[�h���
	ushort	RowSize;						// ���f�[�^�T�C�Y
	char	RowData[BAR_DATASIZE];			// �ǎ悵���o�[�R�[�h���f�[�^
} tMediaDetail;
// MH810100(S) Y.Yamaichi 2020/1/23 �Ԕԃ`�P�b�g���X(�����e�i���X�j
extern	tMediaDetail	MediaDetail;			// �����}�̏��
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// extern	stDiscount2_t	DiscountBackUpArea[ONL_MAX_DISC_NUM];
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
extern	void	lcdbm_QR_data_res( uchar result );
// MH810100(E) Y.Yamaichi 2020/1/23 �Ԕԃ`�P�b�g���X(�����e�i���X�j
	
#define BARCODE_USE_MAX			99			// 1�F�ؑ��쒆�Ɏg�p�\�ȃo�[�R�[�h����
typedef	struct	{							// 1���쒆�̊����}�̏d���`�F�b�N�p�f�[�^
//	stDiscount		stDis[5];				// �|�����
	DATE_YMDHMS		Certif_tim;				// �F�؊m�莞��/�������
//	ulong	ulAmount;						// �ݐϔ������z
//	uchar	ucDiscountMethod;				// �������@
	uchar	bar_count;						// �o�[�R�[�h�g�p����
// MH810103(S) R.Endo 2021/05/31 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5669 QR���㌔���V�[�g�Ǎ��ݖ�������̔���s�
	uchar	amount_count;					// QR���㌔�g�p����
// MH810103(E) R.Endo 2021/05/31 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5669 QR���㌔���V�[�g�Ǎ��ݖ�������̔���s�
//	uchar	media_count;					// �����}�̎g�p����
	tMediaDetail	data[BARCODE_USE_MAX];	// 1�F�ؑ���Ŏg�p����������
// MH810100(S) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j
	struct{
		ulong			Query_ParkingNo;		// �₢���킹�J�[�h ���ԏꇂ
		ushort			Query_Kind;				// �₢���킹�J�[�h ���
		uchar			Query_CardNo[32];		// �₢���킹�J�[�h �J�[�h�ԍ�
		uchar			Passkind;				// ������
		uchar			MemberKind;				// ������
		ulong			InParkNo;				// ���ɒ��ԏꇂ
		DATE_YMDHMS		InDate;					// ���ɔN���������b
	} QueryResult;
// MH810100(E) 2020/08/31 #4776 �y�A���]���w�E�����z���Z���̒�d��A���d���ɑ���QR����f�[�^�̒l���ꕔ�������āu0�v�ƂȂ��Ă���iNo.02-0052�j
} t_UseMedia;
extern	t_UseMedia	g_UseMediaData;

// ��LCD_IF_��M�ް����̒�`				// _INTINRAM�ɔz�u����̂�initialize�͕s�v
// MH810100(S) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X(#4019 QR���[�_�o�[�W�����m�F��ʂŁu���v��������ƁA��ʂ�؂�ւ��邽�тɎ擾�����l��������)
//typedef union {
typedef struct {
// MH810100(E) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X(#4019 QR���[�_�o�[�W�����m�F��ʂŁu���v��������ƁA��ʂ�؂�ւ��邽�тɎ擾�����l��������)
	lcdbm_rsp_error_t			lcdbm_rsp_error;			// �G���[�ʒm
	lcdbm_rsp_notice_ope_t		lcdbm_rsp_notice_ope;		// ����ʒm
	lcdbm_rsp_QR_ctrl_res_t		lcdbm_rsp_QR_ctrl_res;		// QRذ�ސ��䉞��(����ݽ)
	lcdbm_rsp_QR_rd_rslt_t lcdbm_rsp_QR_rd_rslt;			// QR�ǎ挋��(����ݽ)
	lcdbm_rsp_rt_con_rslt_t		lcdbm_rsp_rt_con_rslt;		// ر���ђʐM�a�ʌ���(����ݽ)
	lcdbm_rsp_dc_con_rslt_t		lcdbm_rsp_dc_con_rslt;		// DC-NET�ʐM�a�ʌ���(����ݽ)
	lcdbm_lcd_disconnect_t		lcdbm_lcd_disconnect;		// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING��M���̋���ү����	LCD_DISCONNECT
} LcdRecv_t;

// ��LCD_IF_���M�ް����̒�`				// _INTINRAM�ɔz�u����̂�initialize�͕s�v
typedef union {
	ushort 						lcdbm_LCDbrightness;		// LCD�P�x�ݒ�					PKTcmd_brightness()
	uchar 						lcdbm_audio_volume;			// �ųݽ���ʐݒ�				PKTcmd_audio_volume()
	lcdbm_cmd_audio_start_t 	lcdbm_cmd_audio_start;		// �ųݽ�J�n�v��				PKTcmd_audio_start()
	uchar 						lcdbm_audio_stop_channel;	// �ųݽ�I���v��(�����I������)	PKTcmd_audio_end()
	uchar 						lcdbm_audio_stop_method;	// �ųݽ�I���v��(���f���@)		PKTcmd_audio_end()
	uchar 						lcdbm_BUZ_volume;			// �޻ް���ʐݒ�				PKTcmd_beep_volume()
	uchar 						lcdbm_BUZ_beep;				// �޻ް���v��				PKTcmd_beep_start()
// MH810103(s) �d�q�}�l�[�Ή� �c�x�ƒʒm�C��
//	uchar 						lcdbm_opn_cls;				// �c�x�ƒʒm					PKTcmd_notice_opn()
//	uchar 						lcdbm_opn_cls_reason;		// �x�Ɨ��R����					PKTcmd_notice_opn()
	lcdbm_cmd_notice_opn_t		lcdbm_opn_cls;				// �c�x�ƒʒm					PKTcmd_notice_opn()
// MH810103(e) �d�q�}�l�[�Ή� �c�x�ƒʒm�C��
	lcdbm_rsp_notice_ope_t		lcdbm_cmd_notice_ope;		// ����ʒm						PKTcmd_notice_ope()
} LcdSendUnion_t;

extern	LcdRecv_t				LcdRecv;
extern	LcdSendUnion_t			LcdSend;

extern	lcdbm_rsp_in_car_info_t	lcdbm_rsp_in_car_info_recv;			// ���ɏ��(���ޏ��)PKTtask���ް���set����
extern	lcdbm_rsp_in_car_info_t	lcdbm_rsp_in_car_info_main;			// ���ɏ��(���ޏ��)
extern	lcdbm_rsp_QR_data_t		lcdbm_rsp_QR_data_recv;				// QR�ް�=PKTtask���ް���set����	LCD_QR_DATA
extern	unsigned char			lcdbm_ICC_Settlement_Status;		// ���Ϗ���ް�					LCD_ICC_SETTLEMENT_STS
extern	unsigned char			lcdbm_ICC_Settlement_Result;		// ���ό��ʏ��					LCD_ICC_SETTLEMENT_RSLT
extern	uchar					lcdbm_Flag_ResetRequest;			// ؾ�ėv���L�������׸�(op_mod02�̏�ԂŃ��Z�b�g�ʒm/�N���ʒm��<- ON)
extern	uchar					lcdbm_Flag_QRuse_ICCuse;			// QR_���p��, ���p�m�F���׸�(QRذ�ޗ��p, IC�ڼޯė��p�̔r������)
extern	uchar					lcdbm_Counter_QR_StartStopRetry;	// QRذ�ފJ�n/��~����OK�҂���ײ����
extern	uchar					lcdbm_Last_QR_StartStopInfo;		// QRذ�ފJ�n/��~�����̍Ō�ɑ��M��������(�J�n����=0/��~����=1)
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
extern	uchar					lcdbm_Flag_RemotePay_ICCuse;		// ���σ��[�_��~�m�F�t���O(���u���Z)
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
//////////////////////////////////////////////////////////////////////////
extern	stDiscount_t			m_stDisc;							// DISCOUNT_DATA3 + DiscParkNo
extern	short					m_nDiscPage;						// stDiscountInfo[]��index(0-24)
extern	uchar					lcdbm_QRans_InCar_status;			// 
extern	ulong					g_OpeSequenceID;					// ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
extern	uchar					g_checkKirikae;								// ��ʐؑփ`�F�b�N�ς݃t���O
// MH810100(E) 2020/08/28 #4780 ��ʐؑ�QR�K�p��Ɍ����𓊓����AQR��������ǂݎ��Ɗ������K�p����Ȃ�
// MH810100(S) 2020/09/09 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
extern struct clk_rec			g_PayStartTime;			// ���Z�J�n������(���Z���J�n���ꂽ���̌��ݎ���)
// MH810100(E) 2020/09/09 #4820 �y�A���]���w�E�����zIC�N���W�b�g���Z��������ɒ�d�����d�������ꍇ�A���A���^�C�����Z�f�[�^�̐��Z�����̔N���������b�����f���ꂸ�ɑ��M�����iNo.83�j
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
extern	uchar					g_calcMode;							// �����v�Z���[�h(0:�ʏ헿���v�Z���[�h/1:�N���E�h�����v�Z���[�h)
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
extern	ulong					LaneStsSeqNo;						// ���[�����j�^�f�[�^�̏�Ԉ�A�ԍ�
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129000(S) H.Fujinaga 2023/02/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
extern	uchar	PayInfoChange_StateCheck(void);
// GG129000(E) H.Fujinaga 2023/02/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���u�ƒ[���ԒʐM�̃o�b�t�@��ʂɂ���j
// �W���̒[���ԒʐM�ł͉��u�̃o�b�t�@���g�p���Ă��邪�A���p�\�ɂ��邽�߃o�b�t�@��ʂɂ���
// �T�C�Y�͉��u�Ɠ����Ƃ���
#define	NTNET_LOG_WORK_SIZE		20000							// NT-NET�[���ԒʐM�p���O�f�[�^
extern uchar	NTNET_LogData[NTNET_LOG_WORK_SIZE];
extern uchar	NTNET_NtLogData[NTNET_LOG_WORK_SIZE];
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���u�ƒ[���ԒʐM�̃o�b�t�@��ʂɂ���j

#endif	/* ___MEM_DEFH___ */

