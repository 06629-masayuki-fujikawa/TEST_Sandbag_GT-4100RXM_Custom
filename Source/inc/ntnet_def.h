#ifndef	___NTNET_DEFH___
#define	___NTNET_DEFH___
/*[]----------------------------------------------------------------------[]*/
/*| NT-NET�֘A�ް���`                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  R.Hara                                                  |*/
/*| Date        :  2005-08-05                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include "mem_def.h"
#include "rkn_cal.h"
#include "ope_def.h"
#include "ntnet.h"

#pragma pack

// MH810100(S) K.Onodera 2019/12/25 �Ԕԃ`�P�b�g���X(��)
//#define		NTNET_MODEL_CODE		220
#define		NTNET_MODEL_CODE		581
// MH810100(E) K.Onodera 2019/12/25 �Ԕԃ`�P�b�g���X(��)

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#define	NTBUF_NEARFULL_COUNT			6
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/*--------------------------------------------------------------------------*/
/*	�Ԏ��⍇���ް�(�ް����01)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			LockNo;											// �����
} DATA_KIND_01;


/*--------------------------------------------------------------------------*/
/*	�Ԏ��⍇�������ް�(�ް����02)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			LockNo;											// �����
	ushort			Answer;											// ����
//	flp_com			LkInfo;											// �Ԏ����
	uchar			LkInfo[48];										// �Ԏ����(flp_com�\���̂���Ō�̎��Ԃ����ް4�޲ĕ��������l)
	ushort			Yobi;
	ulong			PpcData;
	ushort			SvsTiket[15];
	ushort			KakeTiket[10];
	ushort			Rsv[20];										// �\��
} DATA_KIND_02;


/*--------------------------------------------------------------------------*/
/*	�Ԏ��⍇������NG�ް�(�ް����03)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			LockNo;											// �����
	ushort			Answer;											// ����
} DATA_KIND_03;


/*--------------------------------------------------------------------------*/
/*	ۯ������ް�(�ް����04)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			LockNo;											// �����
	ushort			MoveMode;										// �����敪
} DATA_KIND_04;


/*--------------------------------------------------------------------------*/
/*	ۯ����䌋���ް�(�ް����05)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			LockNo;											// �����
	ushort			LockSense;										// �ԗ����m���
	ushort			LockState;										// ۯ����u���
} DATA_KIND_05;


/*--------------------------------------------------------------------------*/
/*	�ȈՎԎ����ð���(�ް����12)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ulong			LockNo;											// �����
	ushort			NowState;										// ���ݽð��
	ushort			Year;											// ���ɔN
	uchar			Mont;											// ���Ɍ�
	uchar			Date;											// ���ɓ�
	uchar			Hour;											// ���Ɏ�
	uchar			Minu;											// ���ɕ�
	ushort			Syubet;											// �������
	ulong			TyuRyo;											// ���ԗ���
} LOCK_STATE;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	LOCK_STATE		LockState[OLD_LOCK_MAX];						// �Ԏ����
} DATA_KIND_12;


/*--------------------------------------------------------------------------*/
/*	�����ް�(�ް����20)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ulong			ParkingNo;										// ��������ԏꇂ
	ulong			PassID;											// �����ID
	uchar			Contract[8];									// ������_��
	ushort			Syubet;											// ��������
	ushort			State;											// ������ð��
	ulong			Reserve1;										// �\��
	ulong			Reserve2;										// �\��
	ushort			MoveMode;										// ���������^�ԋp
	ushort			ReadMode;										// �����ذ��ײā^ذ�޵�ذ
	ushort			SYear;											// ������J�n�N
	uchar			SMon;											// ������J�n��
	uchar			SDate;											// ������J�n��
	ushort			EYear;											// ������I���N
	uchar			EMon;											// ������I����
	uchar			EDate;											// ������I����
} PASS_DATA;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			InCount;										// ���ɒǂ���
	ushort			Syubet;											// �����敪
	ushort			InMode;											// ����Ӱ��
	ulong			LockNo;											// �����
	ushort			CardType;										// ���Ԍ�����
	ushort			CMachineNo;										// ���Ԍ��@�B��
	ulong			CardNo;											// ���Ԍ��ԍ�(�����ǂ���)
	date_time_rec2	InTime;											// ���ɔN���������b
	ushort			PassCheck;										// ����߽����
	ushort			CountSet;										// �ݎԶ���
	PASS_DATA		PassData;										// ������ް�
	ushort			Dummy[32];										// ��а
} DATA_KIND_20;


/*--------------------------------------------------------------------------*/
/*	���Z�ް�(�ް����22,23)													*/
/*--------------------------------------------------------------------------*/
#define		NTNET_DIC_MAX	25										// ������
#define		NTNET_SVS_M		1										// ������� ���޽��(����)
#define		NTNET_SVS_T		101										// ������� ���޽��(����)
#define		NTNET_KAK_M		2										// ������� �X����(����)
#define		NTNET_KAK_T		102										// ������� �X����(����)
#define		NTNET_TKAK_M	3										// ������� ���X��(����)
#define		NTNET_TKAK_T	103										// ������� ���X��(����)
#define		NTNET_WRI_M		4										// ������� ������(����)
#define		NTNET_WRI_T		104										// ������� ������(����)
#define		NTNET_FRE		5										// ������� �񐔌�
#define		NTNET_PRI_W		11										// ������� ��������߲�޶���
#define		NTNET_PRI_S		12										// ������� �̔�����߲�޶���
#define		NTNET_WRIKEY	40										// ������� ������
#define		NTNET_KWRIKEY	41										// ������� �X������
#define		NTNET_SYUBET	50										// ������� ��ʊ���
#define		NTNET_SUICA_1	31										// ������� Suica���ޔԍ�
#define		NTNET_SUICA_2	32										// ������� Suica�x���z�A�c�z
#define		NTNET_PASMO_0	33										// ������� PASMO���ρi�W�v�p�j
#define		NTNET_PASMO_1	35										// ������� PASMO���ޔԍ�
#define		NTNET_PASMO_2	36										// ������� PASMO�x���z�A�c�z
#define		NTNET_EDY_0		32										// ������� Edy�W�v�ް�
#define		NTNET_EDY_1		33										// ������� Edy���ޔԍ�
#define		NTNET_EDY_2		34										// ������� Edy�x���z�A�c�z
#define		NTNET_GENGAKU	90										// ������� ���z���Z
#define		NTNET_FURIKAE	91										// ������� �U�֐��Z
#define		NTNET_ICOCA_0	35										// ������� ICOCA���ޔԍ�(�W�v�p)
#define		NTNET_ICOCA_1	61										// ������� ICOCA���ޔԍ�
#define		NTNET_ICOCA_2	62										// ������� ICOCA�x���z�A�c�z
#define		NTNET_ICCARD_0	36										// ������� IC-Card���ޔԍ�(�W�v�p)
#define		NTNET_ICCARD_1	63										// ������� IC-Card���ޔԍ�
#define		NTNET_ICCARD_2	64										// ������� IC-Card�x���z�A�c�z
#define		NTNET_TCARD_0	38
#define		NTNET_TCARD_1	67
#define		NTNET_TCARD_2	68
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		NTNET_WAON_0	37										// ������� WAON�i�W�v�p�j
#define		NTNET_WAON_1	65										// ������� WAON���ޔԍ�
#define		NTNET_WAON_2	66										// ������� WAON�x���z�A�c�z

#define		NTNET_SAPICA_0	82										// ������� SAPICA�i�W�v�p�j
#define		NTNET_SAPICA_1	83										// ������� SAPICA���ޔԍ�
#define		NTNET_SAPICA_2	84										// ������� SAPICA�x���z�A�c�z

#define		NTNET_NANACO_0	80										// ������� nanaco�i�W�v�p�j
#define		NTNET_NANACO_1	85										// ������� nanaco���ޔԍ�
#define		NTNET_NANACO_2	86										// ������� nanaco�x���z�A�c�z

#define		NTNET_ID_0		34										// ������� iD�i�W�v�p�j
#define		NTNET_ID_1		37										// ������� iD���ޔԍ�
#define		NTNET_ID_2		38										// ������� iD�x���z�A�c�z

#define		NTNET_QUICPAY_0	83										// ������� QUICPay�i�W�v�p�j
#define		NTNET_QUICPAY_1	81										// ������� QUICPay���ޔԍ�
#define		NTNET_QUICPAY_2	82										// ������� QUICPay�x���z�A�c�z
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(S) MH364301 PiTaPa�Ή�
#define		NTNET_PITAPA_0	81										// ������� PiTaPa�i�W�v�p�j
#define		NTNET_PITAPA_1	87										// ������� PiTaPa���ޔԍ�
#define		NTNET_PITAPA_2	88										// ������� PiTaPa�x���z
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define 	NTNET_QR		110										// ������� QR����
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

#define		NTNET_SYUSEI_1	92										// ������� �C�����Z
#define		NTNET_SYUSEI_2	93										// ������� �C�����Z
#define		NTNET_SYUSEI_3	94										// ������� �C�����Z
#define		NTNET_CSVS_M	21										// ������� ���Z���~���޽��(����)
#define		NTNET_CSVS_T	22										// ������� ���Z���~���޽��(����)
#define		NTNET_CKAK_M	23										// ������� ���Z���~�X����(����)
#define		NTNET_CKAK_T	24										// ������� ���Z���~�X����(����)
#define		NTNET_CWRI_M	25										// ������� ���Z���~������(����)
#define		NTNET_CWRI_T	26										// ������� ���Z���~������(����)
#define		NTNET_CFRE		27										// ������� ���Z���~�񐔌�

// �s��C��(S) K.Onodera 2016/12/08 #1585 �U�֌��������z�ɖ��֌W�̒l���Z�b�g�����
#define		NTNET_SECTION_WARI_MAX	999
// �s��C��(E) K.Onodera 2016/12/08 #1585 �U�֌��������z�ɖ��֌W�̒l���Z�b�g�����
// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define		NTNET_DEEMED_PAY		1021							// ������� �݂Ȃ�����
#define		NTNET_MIRYO_PAY			1022							// ������� �����x���ς�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
#define		NTNET_KABARAI			2030							// ������� �ߕ���(�����ȊO)
#define		NTNET_FURIKAE_2			2031							// ������� �U�֐��Z
#define		NTNET_FURIKAE_DETAIL	2032
#define		NTNET_FUTURE			2033							// ������� ������Z�\��z
#define		NTNET_AFTER_PAY			2034							// ������� ������Z�z(����������z)
// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
#define		NTNET_KAIMONO_GOUKEI	2001	// ������� �������z���v
#define		NTNET_SYUBET_TIME		150		// ��ʊ���(����)
#define		NTNET_SHOP_DISC_AMT		6		// ��������(���z)
#define		NTNET_SHOP_DISC_TIME	106		// ��������(����)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// GG129004(S) M.Fujikawa 2024/10/22 �������z�������Ή�
#define		NTNET_KAIMONO_INFO		2002	// �������
// GG129004(E) M.Fujikawa 2024/10/22 �������z�������Ή�
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
#define		NTNET_INQUIRY_NUM		8000							// ������� ���σ��[�_�⍇���ԍ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define		NTNET_MCH_NO_TOP_HALF		8001						// ������� QR�R�[�h���� Mch����ԍ���ʔ���
#define		NTNET_MCH_NO_LOWER_HALF		8002						// ������� QR�R�[�h���� Mch����ԍ����ʔ���
#define		NTNET_PAYTERMINAL_NO		8003						// ������� QR�R�[�h���� �x���[��ID
#define		NTNET_DEAL_NO				8004						// ������� QR�R�[�h���� ����ԍ�
#define		NTNET_MIRYO_UNKNOWN			8021						// ������� �����x���s��
#define		NTNET_ID_APPROVAL_NO		8101						// ������� ID���F�ԍ�
#define		NTNET_QUIC_PAY_APPROVAL_NO	8102						// ������� QUICPAY���F�ԍ�
#define		NTNET_PITAPA_APPROVAL_NO	8103						// ������� PITAPA���F�ԍ�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j
#define		NTNET_MEITETU_IN_MONEY		9200							// ���S�ʊ�����ʁF����
#define		NTNET_MEITETU_OUT_MONEY		9201							// ���S�ʊ�����ʁF�o��

#define		NTNET_SHABANINFO_1			60100						// �Ԕԏ��1
#define		NTNET_SHABANINFO_2			60101						// �Ԕԏ��2
#define		NTNET_SHABANINFO_3			60102						// �Ԕԏ��3
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j

typedef	struct {
	ulong			ParkingNo;										// ���ԏꇂ
	ushort			DiscSyu;										// �������
	uchar			CardNo[20];										// �d�q�}�l�[�J�[�hNo.
} DISCOUNT_DATA2;
extern	DISCOUNT_DATA2	wk_DicData;

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
typedef struct {
	ulong			ParkingNo;			// ���ԏꇂ
	ushort			DiscSyu;			// �������
	ushort			DiscNo;				// �����敪
	ushort			DiscCount;			// ����
	ulong			Discount;			// �����z(��������)
	ulong			DiscInfo1;			// �������1
	union {
		struct {						// �v���y�C�h�J�[�h�ȊO�̍\����
			ulong			DiscInfo2;	// �������2
			ushort			MoveMode;	// ����^�ԋp
			ushort			DiscFlg;	// �����ς�
		} common;
		struct {						// �v���y�C�h�J�[�h�p�\����
			ulong			pay_befor;	// �x���O�c�z
			ushort			kigen_year;	// �L�������N
			uchar			kigen_mon;	// �L��������
			uchar			kigen_day;	// �L��������
		} ppc_data;						// NT-NET���Z�f�[�^���M�O�ɃN���A����
	} uDiscData;
} t_SeisanDiscountOld;
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			PayCount;										// ���Z�ǂ���
	ushort			PayMethod;										// ���Z���@
	ushort			PayClass;										// �����敪
	ushort			PayMode;										// ���ZӰ��
	ulong			LockNo;											// �����
	ushort			CardType;										// ���Ԍ�����
	ushort			CMachineNo;										// ���Ԍ��@�B��
	ulong			CardNo;											// ���Ԍ��ԍ�(�����ǂ���)
	date_time_rec2	OutTime;										// �o�ɔN���������b
	ushort			KakariNo;										// �W����
	ushort			OutKind;										// ���Z�o��
	ushort			CountSet;										// �ݎԶ���
	ushort			Before_pst;										// �O�񁓊�����
	ulong			BeforePwari;									// �O�񁓊������z
	ulong			BeforeTime;										// �O�񊄈����Ԑ�
	ulong			BeforeTwari;									// �O�񎞊Ԋ������z
	ulong			BeforeRwari;									// �O�񗿋��������z
	ushort			ReceiptIssue;									// �̎��ؔ��s�L��
	date_time_rec2	InTime;											// ���ɔN���������b
	date_time_rec2	PayTime;										// ���O���Z�N���������b(CPS���Z�����)
	ulong			TaxPrice;										// �ېőΏۊz
	ulong			TotalPrice;										// ���v���z
	ulong			Tax;											// ����Ŋz
	ushort			Syubet;											// �������
	ulong			Price;											// ���ԗ���
	ulong			TotalDiscount;									// �������z
	ulong			CashPrice;										// ��������
	ulong			InPrice;										// �������z
	ulong			ChgPrice;										// �ޑK���z
	ulong			Fusoku;											// �ޑK���o�s�����z
	ushort			FusokuFlg;										// �ޑK���o�s�������׸�
	ushort			PayObsFlg;										// ���Z����Q�����׸�
	ushort			ChgOverFlg;										// ���ߏ���z���ް�����׸�
	ushort			PassCheck;										// ����߽����
	PASS_DATA		PassData;										// ������ް�
	ulong			PassRenewalPric;								// ������X�V����
	ushort			PassRenewalCondition;							// ������X�V����
	ushort			PassRenewalPeriod;								// ������X�V����
	date_time_rec2	BeforePayTime;									// �O�񎖑O���Z�N���������b�i���Z�㌔�Ő��Z���j
	uchar			MatchIP[8];										// ϯ�ݸ�IP���ڽ
	uchar			MatchVTD[8];									// ϯ�ݸ�VTD/�Ԕ�
	uchar			CreditCardNo[20];								// �ڼޯĶ��މ����
	ulong			Credit_ryo;										// �ڼޯĶ��ޗ��p���z
	ulong			CreditSlipNo;									// �ڼޯĶ��ޓ`�[�ԍ�
	ulong			CreditAppNo;									// �ڼޯĶ��ޏ��F�ԍ�
	uchar			CreditName[10];									// �ڼޯĶ��މ�Ж�
	uchar			CreditDate[2];									// �ڼޯĶ��ޗL������(�N��)
	ulong			CreditProcessNo;								// �ڼޯĶ��޾�������ǂ���
	ulong			CreditReserve1;									// �ڼޯĶ��ޗ\��1
	ulong			CreditReserve2;									// �ڼޯĶ��ޗ\��2
	uchar			PayCalMax;										// �ő嗿���K�p�L��
	uchar			Reserve1[3];									// �\��1
	ulong			Reserve2;										// �\��2
	ulong			Reserve3;										// �\��3
	ulong			Reserve4;										// �\��4
	ulong			Reserve5;										// �\��5
	ulong			Reserve6;										// �\��6
	ulong			Reserve7;										// �\��7
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	DISCOUNT_DATA	DiscountData[NTNET_DIC_MAX];					// �����֘A
	t_SeisanDiscountOld	DiscountData[NTNET_DIC_MAX];				// �����֘A
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
} DATA_KIND_22;

typedef	struct {
	ushort			FullNo1;										// ���ԑ䐔�P
	ushort			CarCnt1;										// ���ݑ䐔�P
	ushort			FullNo2;										// ���ԑ䐔�Q
	ushort			CarCnt2;										// ���ݑ䐔�Q
	ushort			FullNo3;										// ���ԑ䐔�R
	ushort			CarCnt3;										// ���ݑ䐔�R
	ushort			PascarCnt;										// ����ԗ��J�E���g
	ushort			Full[3];										// ����1�`3������
} PARKCAR_DATA1;
typedef	struct {
	ushort			GroupNo;										// ��ٰ�ߔԍ�
	ushort			State;											// �p�r�ʒ��ԑ䐔�ݒ�
	ushort			Full[3];										// ����1�`3������
	ushort			EmptyNo1;										// ��ԑ䐔�P
	ushort			FullNo1;										// ���ԑ䐔�P
	ushort			EmptyNo2;										// ��ԑ䐔�Q
	ushort			FullNo2;										// ���ԑ䐔�Q
	ushort			EmptyNo3;										// ��ԑ䐔�R
	ushort			FullNo3;										// ���ԑ䐔�R
} PARKCAR_DATA2;

typedef	struct {
	ushort			FullNo1;										// ���ԑ䐔�P
	ushort			CarCnt1;										// ���ݑ䐔�P
	ushort			FullNo2;										// ���ԑ䐔�Q
	ushort			CarCnt2;										// ���ݑ䐔�Q
	ushort			FullNo3;										// ���ԑ䐔�R
	ushort			CarCnt3;										// ���ݑ䐔�R
	uchar			CarFullFlag;									// �䐔�E�����ԃt���O
	uchar			PascarCnt;										// ����ԗ��J�E���g
	ushort			Full[3];										// ����1�`3������
} PARKCAR_DATA11;
typedef	struct {
	ushort			FullSts1;										// ���ԂP������
	ushort			EmptyNo1;										// ��ԑ䐔�P
	ushort			FullNo1;										// ���ԑ䐔�P
	ushort			FullSts2;										// ���ԂQ������
	ushort			EmptyNo2;										// ��ԑ䐔�Q
	ushort			FullNo2;										// ���ԑ䐔�Q
	ushort			FullSts3;										// ���ԂR������
	ushort			EmptyNo3;										// ��ԑ䐔�R
	ushort			FullNo3;										// ���ԑ䐔�R
} PARKCAR_DATA21;
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
typedef	struct {
	ushort			FullNo1;										// ���ԑ䐔�P
	ushort			CarCnt1;										// ���ݑ䐔�P
	ushort			FullNo2;										// ���ԑ䐔�Q
	ushort			CarCnt2;										// ���ݑ䐔�Q
	ushort			FullNo3;										// ���ԑ䐔�R
	ushort			CarCnt3;										// ���ݑ䐔�R
	uchar			CarFullFlag;									// �䐔�E�����ԃt���O
	uchar			PascarCnt;										// ����ԗ��J�E���g
	ushort			Full[3];										// ����1�`3������
// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//	struct{	// �䐔�Ǘ��ǔ�
//		uchar		CarCntYear;										// �N
//		uchar		CarCntMon;										// ��
//		uchar		CarCntDay;										// ��
//		uchar		CarCntHour;										// ��
//		uchar		CarCntMin;										// ��
//		ushort		CarCntSeq;										// �ǔ�
//		uchar		Reserve1;										// �\��
//	} CarCntInfo;
	CAR_CNT_INFO	CarCntInfo;											// �䐔�Ǘ��ǔ�
// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
} PARKCAR_DATA14;
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
/*--------------------------------------------------------------------------*/
/*	�����ް�(�ް����54)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA1	ParkData;										// ���ԑ䐔
	ulong			InCount;										// ���ɒǂ���
	ushort			Syubet;											// �����敪
	ushort			InMode;											// ����Ӱ��
	ulong			LockNo;											// �����
	ushort			CardType;										// ���Ԍ�����
	ushort			CMachineNo;										// ���Ԍ��@�B��
	ulong			CardNo;											// ���Ԍ��ԍ�(�����ǂ���)
	date_time_rec2	InTime;											// ���ɔN���������b
	ushort			PassCheck;										// ����߽����
	ushort			CountSet;										// �ݎԶ���
	PASS_DATA		PassData;										// ������ް�
	ushort			Dummy[32];										// ��а
} DATA_KIND_54;
typedef struct {
	ushort			MediaKind;										// ���(�}��)	0�`99
	uchar			MediaCardNo[30];								// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			MediaCardInfo[16];								// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
} t_MediaInfo;		// ���Ɂ^���Z�@�}�̏��

// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
typedef struct {
	ushort			MediaKind;										// ���(�}��)	0�`99
	uchar			MediaCardNo[30];								// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			MediaCardInfo[16];								// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	ushort			Reserve1;										// �\��(�T�C�Y)
} t_MediaInfo2;		// ���Ɂ^���Z�@�}�̏��
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA11	ParkData;										// ���ԑ䐔
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			InCount;										// ���ɒǂ���
	uchar			Syubet;											// �����敪
	uchar			InMode;											// ����Ӱ��
	uchar			CMachineNo;										// ���Ԍ��@�B��
	uchar			Reserve;										// �\���i�T�C�Y�����p�j
	ushort			FlapArea;										// �t���b�v�V�X�e��	���		0�`99
	ushort			FlapParkNo;										// 					�Ԏ��ԍ�	0�`9999
	uchar			PassCheck;										// ����߽����
	uchar			CountSet;										// �ݎԶ���
	ulong			ParkingNo;										// ������@���ԏꇂ	0�`999999
	t_MediaInfo		Media[4];										// ���ɔ}�̏��1�`4
} DATA_KIND_54_r10;
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA14	ParkData;										// ���ԑ䐔
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			InCount;										// ���ɒǂ���
	uchar			Syubet;											// �����敪
	uchar			InMode;											// ����Ӱ��
	uchar			CMachineNo;										// ���Ԍ��@�B��
	uchar			Reserve;										// �\���i�T�C�Y�����p�j
	ushort			FlapArea;										// �t���b�v�V�X�e��	���		0�`99
	ushort			FlapParkNo;										// 					�Ԏ��ԍ�	0�`9999
	uchar			PassCheck;										// ����߽����
	uchar			CountSet;										// �ݎԶ���
	ulong			ParkingNo;										// ������@���ԏꇂ	0�`999999
// �s��C��(S) K.Onodera 2016/12/09 �A���]���w�E(0�J�b�g�O�̃T�C�Y������Ă���)
//	t_MediaInfo		Media[4];										// ���ɔ}�̏��1�`4
	t_MediaInfo2	Media[4];										// ���ɔ}�̏��1�`4
	ushort			DepositKind;									// �ۏ؋��敪
	ulong			DepositMoney;									// �ۏ؋����z
	ulong			InMoney;										// �������z
	ulong			ChgMoney;										// �ޑK�z
// �s��C��(E) K.Onodera 2016/12/09 �A���]���w�E(0�J�b�g�O�̃T�C�Y������Ă���)
} DATA_KIND_54_r13;
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�

/*--------------------------------------------------------------------------*/
/*	���Z�ް�(�ް����56,57)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA1	ParkData;										// ���ԑ䐔
	ulong			PayCount;										// ���Z�ǂ���
	ushort			PayMethod;										// ���Z���@
	ushort			PayClass;										// �����敪
	ushort			PayMode;										// ���ZӰ��
	ulong			LockNo;											// �����
	ushort			CardType;										// ���Ԍ�����
	ushort			CMachineNo;										// ���Ԍ��@�B��
	ulong			CardNo;											// ���Ԍ��ԍ�(�����ǂ���)
	date_time_rec2	OutTime;										// �o�ɔN���������b
	ushort			KakariNo;										// �W����
	ushort			OutKind;										// ���Z�o��
	ushort			CountSet;										// �ݎԶ���
	ushort			Before_pst;										// �O�񁓊�����
	ulong			BeforePwari;									// �O�񁓊������z
	ulong			BeforeTime;										// �O�񊄈����Ԑ�
	ulong			BeforeTwari;									// �O�񎞊Ԋ������z
	ulong			BeforeRwari;									// �O�񗿋��������z
	ushort			ReceiptIssue;									// �̎��ؔ��s�L��
	date_time_rec2	InTime;											// ���ɔN���������b
	date_time_rec2	PayTime;										// ���O���Z�N���������b(CPS���Z�����)
	ulong			TaxPrice;										// �ېőΏۊz
	ulong			TotalPrice;										// ���v���z
	ulong			Tax;											// ����Ŋz
	ushort			Syubet;											// �������
	ulong			Price;											// ���ԗ���
	ulong			TotalDiscount;									// �������z
	ulong			CashPrice;										// ��������
	ulong			InPrice;										// �������z
	ulong			ChgPrice;										// �ޑK���z
	ulong			Fusoku;											// �ޑK���o�s�����z
	ushort			FusokuFlg;										// �ޑK���o�s�������׸�
	ushort			PayObsFlg;										// ���Z����Q�����׸�
	ushort			ChgOverFlg;										// ���ߏ���z���ް�����׸�
	ushort			PassCheck;										// ����߽����
	PASS_DATA		PassData;										// ������ް�
	ulong			PassRenewalPric;								// ������X�V����
	ushort			PassRenewalCondition;							// ������X�V����
	ushort			PassRenewalPeriod;								// ������X�V����
	date_time_rec2	BeforePayTime;									// �O�񎖑O���Z�N���������b�i���Z�㌔�Ő��Z���j

	uchar			MatchIP[8];										// ϯ�ݸ�IP���ڽ
	uchar			MatchVTD[8];									// ϯ�ݸ�VTD/�Ԕ�
	uchar			CreditCardNo[20];								// �ڼޯĶ��މ����
	ulong			Credit_ryo;										// �ڼޯĶ��ޗ��p���z
	ulong			CreditSlipNo;									// �ڼޯĶ��ޓ`�[�ԍ�
	ulong			CreditAppNo;									// �ڼޯĶ��ޏ��F�ԍ�
	uchar			CreditName[10];									// �ڼޯĶ��މ�Ж�
	uchar			CreditDate[2];									// �ڼޯĶ��ޗL������(�N��)
	ulong			CreditProcessNo;								// �ڼޯĶ��޾�������ǂ���
	ulong			CreditReserve1;									// �ڼޯĶ��ޗ\��1
	ulong			CreditReserve2;									// �ڼޯĶ��ޗ\��2
	ulong			Reserve1;										// �\��1
	ulong			Reserve2;										// �\��2
	ulong			Reserve3;										// �\��3
	ulong			Reserve4;										// �\��4
	ulong			Reserve5;										// �\��5
	ulong			Reserve6;										// �\��6
	ulong			Reserve7;										// �\��7
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	DISCOUNT_DATA	DiscountData[NTNET_DIC_MAX];					// �����֘A
	t_SeisanDiscountOld	DiscountData[NTNET_DIC_MAX];				// �����֘A
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
} DATA_KIND_56;
typedef	struct {
	uchar		In_Year;			// ����	�N		00�`99
	uchar		In_Mon;				// 		��		01�`12
	uchar		In_Day;				// 		��		01�`31
	uchar		In_Hour;			// 		��		00�`23
	uchar		In_Min;				// 		��		00�`59
	uchar		In_Sec;				// 		�b		00�`59(���C�J�[�h��0�Œ�)
	uchar		Prev_Year;			// �O�񐸎Z	�N	00�`99 2000�`2099
	uchar		Prev_Mon;			// 			��	01�`12
	uchar		Prev_Day;			// 			��	01�`31
	uchar		Prev_Hour;			// 			��	00�`23
	uchar		Prev_Min;			// 			��	00�`59
	uchar		Prev_Sec;			// 			�b	00�`59(���C�J�[�h��0�Œ�)
} t_InPrevYMDHMS;		// ����/�O�񐸎Z_YMDHMS

typedef	struct {
	uchar			MoneyKind_In;	// ����L��	���ڋ���̗L��
									//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
									//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
	uchar			In_10_cnt;		// ����������(10�~)		0�`255
	uchar			In_50_cnt;		// ����������(50�~)		0�`255
	uchar			In_100_cnt;		// ����������(100�~)	0�`255
	uchar			In_500_cnt;		// ����������(500�~)	0�`255
	uchar			In_1000_cnt;	// ����������(1000�~)	0�`255
	uchar			In_2000_cnt;	// ����������(2000�~)	0�`255
	uchar			In_5000_cnt;	// ����������(5000�~)	0�`255
	uchar			In_10000_cnt;	// ����������(10000�~)	0�`255

	uchar			MoneyKind_Out;	// ����L��	���ڋ���̗L��
									//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
									//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
	uchar			Out_10_cnt;		// ���o������(10�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
	uchar			Out_50_cnt;		// ���o������(50�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
	uchar			Out_100_cnt;	// ���o������(100�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
	uchar			Out_500_cnt;	// ���o������(500�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
	uchar			Out_1000_cnt;	// ���o������(1000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
	uchar			Out_2000cnt;	// ���o������(2000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
	uchar			Out_5000_cnt;	// ���o������(5000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
	uchar			Out_10000_cnt;	// ���o������(10000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
} t_MoneyInOut;		// ���K���(����������/���o������)

typedef struct {
	ulong			ParkingNo;		// ���ԏ�No.
	ushort			Kind;			// �������
	ushort			Group;			// �����敪
	ushort			Callback;		// �������
	ulong			Amount;			// �����z
	ulong			Info1;			// �������1
	ulong			Info2;			// �������2
} t_SeisanDiscount;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA11	ParkData;										// ���ԑ䐔
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			PayCount;										// ���Z�ǔ�(0�`99999)
	uchar			PayMethod;										// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
	uchar			PayClass;										// �����敪(0�����Z/1���Đ��Z/2�����Z���~/3���Đ��Z���~, 8=���Z�O, 9=���Z�r��
	uchar			PayMode;										// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
																	//				20��Mifare�v���y�C�h���Z
	uchar			CMachineNo;										// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
	ushort			FlapArea;										// �t���b�v�V�X�e��	���		0�`99
	ushort			FlapParkNo;										// 					�Ԏ��ԍ�	0�`9999
	ushort			KakariNo;										// �W����	0�`9999
	uchar			OutKind;										// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
																	//				10���Q�[�g�J��/20��Mifare�v���y�C�h���Z�o��
																	//				97�����b�N�J�E�t���b�v�㏸�O�����Z�o��/98�����O�^�C�����o��
																	//				99���T�[�r�X�^�C�����o��
	uchar			CountSet;										// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
																	// (�f�[�^��ʂ��u22�F���O���Z�v�̎��́u1=���Ȃ��v�ƂȂ�)
	t_InPrevYMDHMS	InPrev_ymdhms;									// ����/�O�񐸎Z_YMDHMS
	uchar			ReceiptIssue;									// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
	uchar			Syubet;											// �������			1�`
	ulong			Price;											// ���ԗ���			0�`
	ulong			CashPrice;										// ��������			0�`
	ulong			InPrice;										// �������z			0�`
	ushort			ChgPrice;										// �ޑK���z			0�`9999
	t_MoneyInOut	MoneyInOut;										// ���K���i�������z�^���ߋ��z�j
	ushort			HaraiModoshiFusoku;								// ���ߕs���z	0�`9999
	uchar			Reserve1;										// �\��(�T�C�Y�����p)	0
	uchar			PassCheck;										// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�N�n�e�e/2�������n�e�e
																	//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�N�n�e�e�v�Œ�
	ulong			ParkingNo;										// ������@���ԏꇂ	0�`999999
	t_MediaInfo		Media[2];										// ���Z�}�̏��1, 2
	ushort			CardKind;										// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
																	//					3=Edy�A4=PiTaPa�A5=WAON�@�@90�ȍ~=�ʑΉ�
	uchar			settlement[64];									// ���Ϗ��			(��3)�@�Í����Ώ�
	ulong			MoneyIn;										// �����������z
	ulong			MoneyOut;										// �o���������z
	ulong			MoneyBack;										// ���ߋ��z
	ulong			MoneyFusoku;									// ����s�����z
	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];					// ����
} DATA_KIND_56_r10;

// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA14	ParkData;										// ���ԑ䐔
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			PayCount;										// ���Z�ǔ�(0�`99999)
	uchar			PayMethod;										// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z/5��������X�V
																	//   10���C�����Z�i�C�����j/11���C�����Z�i�C���j/12=�U�֐��Z�i�U�֐�j/13=������Z/20�����Ɏ����Z/30=����i��t�j�����Z)
	uchar			PayClass;										// �����敪(0�����Z/1���Đ��Z/2�����Z���~/3���Đ��Z���~, 8=���Z�O, 9=���Z�r��
	uchar			PayMode;										// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
																	//				20��Mifare�v���y�C�h���Z
	uchar			CMachineNo;										// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
	ushort			FlapArea;										// �t���b�v�V�X�e��	���		0�`99
	ushort			FlapParkNo;										// 					�Ԏ��ԍ�	0�`9999
	ushort			KakariNo;										// �W����	0�`9999
	uchar			OutKind;										// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
																	//				10���Q�[�g�J��/20��Mifare�v���y�C�h���Z�o��
																	//				97�����b�N�J�E�t���b�v�㏸�O�����Z�o��/98�����O�^�C�����o��
																	//				99���T�[�r�X�^�C�����o��
	uchar			CountSet;										// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
																	// (�f�[�^��ʂ��u22�F���O���Z�v�̎��́u1=���Ȃ��v�ƂȂ�)
	t_InPrevYMDHMS	InPrev_ymdhms;									// ����/�O�񐸎Z_YMDHMS
	uchar			ReceiptIssue;									// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
	uchar			Syubet;											// �������			1�`
	ulong			Price;											// ���ԗ���			0�`
	long			CashPrice;										// ��������			0�`
	ulong			InPrice;										// �������z			0�`
	ushort			ChgPrice;										// �ޑK���z			0�`9999
	t_MoneyInOut	MoneyInOut;										// ���K���i�������z�^���ߋ��z�j
	ushort			HaraiModoshiFusoku;								// ���ߕs���z(����)	0�`9999
	ushort			CardFusokuType;									// ���o�s���}�̎��(�����ȊO)
	ushort			CardFusokuTotal;								// ���o�s���z(�����ȊO)
	ulong			SaleParkingNo;									// ����撓�ԏꇂ(0�`999999   �������p�i�W���ł͊�{���ԏꇂ���Z�b�g�j
	uchar			Reserve1;										// �\��(�T�C�Y�����p)	0
	uchar			PassCheck;										// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�N�n�e�e/2�������n�e�e
																	//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�N�n�e�e�v�Œ�
	ulong			ParkingNo;										// ������@���ԏꇂ	0�`999999
	t_MediaInfo2	Media[2];										// ���Z�}�̏��1, 2
	ushort			CardKind;										// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
																	//					3=Edy�A4=PiTaPa�A5=WAON�@�@90�ȍ~=�ʑΉ�
	uchar			settlement[80];									// ���Ϗ��			(��3)�@�Í����Ώ�
	ulong			TotalSale;										// ���v���z(����)
	ulong			DeleteSeq;										// �폜�f�[�^�ǔ�(1�`FFFFFFFFH�@�폜���鐸�Z�f�[�^�ɃZ�b�g�����Z���^�[�ǔ�)
	ulong			Reserve2;										// �\��
	ulong			Reserve3;										// �\��
	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];					// ����
} DATA_KIND_56_r14;
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA14	ParkData;										// ���ԑ䐔
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			PayCount;										// ���Z�ǔ�(0�`99999)
	uchar			PayMethod;										// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z/5��������X�V
																	//   10���C�����Z�i�C�����j/11���C�����Z�i�C���j/12=�U�֐��Z�i�U�֐�j/13=������Z/20�����Ɏ����Z/30=����i��t�j�����Z)
	uchar			PayClass;										// �����敪(0�����Z/1���Đ��Z/2�����Z���~/3���Đ��Z���~, 8=���Z�O, 9=���Z�r��
	uchar			PayMode;										// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
																	//				20��Mifare�v���y�C�h���Z
	uchar			CMachineNo;										// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
	ushort			FlapArea;										// �t���b�v�V�X�e��	���		0�`99
	ushort			FlapParkNo;										// 					�Ԏ��ԍ�	0�`9999
	ushort			KakariNo;										// �W����	0�`9999
	uchar			OutKind;										// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
																	//				10���Q�[�g�J��/20��Mifare�v���y�C�h���Z�o��
																	//				97�����b�N�J�E�t���b�v�㏸�O�����Z�o��/98�����O�^�C�����o��
																	//				99���T�[�r�X�^�C�����o��
	uchar			CountSet;										// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
																	// (�f�[�^��ʂ��u22�F���O���Z�v�̎��́u1=���Ȃ��v�ƂȂ�)
	t_InPrevYMDHMS	InPrev_ymdhms;									// ����/�O�񐸎Z_YMDHMS
	ulong			ParkTime;										// ���Ԏ���
	uchar			ReceiptIssue;									// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
	uchar			Syubet;											// �������			1�`
	ulong			Price;											// ���ԗ���			0�`
	long			CashPrice;										// ��������			0�`
	ulong			InPrice;										// �������z			0�`
	ushort			ChgPrice;										// �ޑK���z			0�`9999
	t_MoneyInOut	MoneyInOut;										// ���K���i�������z�^���ߋ��z�j
	ushort			HaraiModoshiFusoku;								// ���ߕs���z(����)	0�`9999
	ushort			CardFusokuType;									// ���o�s���}�̎��(�����ȊO)
	ushort			CardFusokuTotal;								// ���o�s���z(�����ȊO)
	ulong			SaleParkingNo;									// ����撓�ԏꇂ(0�`999999   �������p�i�W���ł͊�{���ԏꇂ���Z�b�g�j
	ushort			MaxChargeApplyFlg;								// �ő嗿���K�p�t���O
	ushort			MaxChargeApplyCnt;								// �ő嗿���K�p��
	uchar			MaxChargeSettingFlg;							// �ő嗿���ݒ�L��
	uchar			PassCheck;										// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�N�n�e�e/2�������n�e�e
																	//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�N�n�e�e�v�Œ�
	ulong			ParkingNo;										// ������@���ԏꇂ	0�`999999
	t_MediaInfo2	Media[2];										// ���Z�}�̏��1, 2
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�K�i���������L�����Z�b�g����j
	ushort			Invoice;										// �K�i���������L��
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�K�i���������L�����Z�b�g����j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
	uchar			RegistNum[14];									// �o�^�ԍ��i�擪1����+13���̐����j
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
	ulong			TaxPrice;										// �ېőΏۊz
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�K�p�ŗ����Z�b�g����j
	ushort			TaxRate;										// �K�p�ŗ�
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�K�p�ŗ����Z�b�g����j
	ushort			CardKind;										// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
																	//					3=Edy�A4=PiTaPa�A5=WAON�@�@90�ȍ~=�ʑΉ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	uchar			PayTerminalClass;								// ���ϒ[���敪
	uchar			Transactiontatus;								// ����X�e�[�^�X
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	uchar			settlement[80];									// ���Ϗ��			(��3)�@�Í����Ώ�
	ulong			TotalSale;										// ���v���z(����)
	ulong			DeleteSeq;										// �폜�f�[�^�ǔ�(1�`FFFFFFFFH�@�폜���鐸�Z�f�[�^�ɃZ�b�g�����Z���^�[�ǔ�)
	uchar			Reserve;										// �\��
	uchar			CarNumInfoFlg;									// �Ԕԏ��L��
// GG124100(S) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
// 	uchar			LandTransOfficeName[12];						// �Ԕԁ@���^�x�ǖ�
// 	uchar			ClassNum[9];									// �Ԕԁ@���ޔԍ�
// 	uchar			Reserve1;										// �\��
// 	uchar			UsageCharacter[3];								// �Ԕԁ@�p�r����
// 	uchar			Reserve2;										// �\��
// 	uchar			SeqDesignNumber[12];							// �Ԕԁ@��A�w��ԍ�
// 	uchar			Reserve3[10];									// �\��
	SHABAN_INFO		ShabanInfo;										// �Ԕԏ��
// GG124100(E) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];					// ����
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//} DATA_KIND_56_r17;
} DATA_KIND_56_rXX;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
// ���Z�f�[�^�ێ��p
#define MACHINE_TKT_NO_SIZE (9)	// ���Ԍ��@�B��(���ɋ@�B��)�A���Ԍ��ԍ��T�C�Y

typedef struct {
	uchar			NormalFlag;										// �ꎞ���p�t���O(0=������p,1=�ꎞ���p)
	uchar			MachineTktNo[MACHINE_TKT_NO_SIZE];				// ���Ԍ��@�B��(���ɋ@�B��)(���3���A0�`255)�A���Ԍ��ԍ�(����6���A0�`999999)
	ulong			ParkingNo;										// ��������ԏꇂ
	ulong			id;												// �����ID(5���A0�`12000)
	uchar			syu;											// ��������(2���A1�`15)
	uchar			pkno_syu;										// ��������ԏꇂ���(0=��{,1-3=�g��)
// GG132000(S) ���Z�����^���~����NT-NET�ʐM�̐��Z�f�[�^�Ƀf�[�^���ڂ�ǉ�
	ushort			MaxFeeApplyFlg;									// �ő嗿���K�p�t���O
	ushort			MaxFeeApplyCnt;									// �ő嗿���K�p��
	uchar			MaxFeeSettingFlg;								// �ő嗿���ݒ�L��
// GG132000(E) ���Z�����^���~����NT-NET�ʐM�̐��Z�f�[�^�Ƀf�[�^���ڂ�ǉ�
} ntNet_56_saveInf;

extern ntNet_56_saveInf ntNet_56_SaveData;
// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX

/*--------------------------------------------------------------------------*/
/*	���Ϗ��i�d�q�}�l�[�j
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			amount;
	uchar			card_id[20];
	ulong			card_zangaku;
	uchar			reserve[36];
} EMONEY;
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
typedef struct {
	ulong			amount;
	uchar			card_id[20];
	ulong			card_zangaku;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//	uchar			reserve[52];
	uchar			inquiry_num[16];
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	uchar			reserve[36];
	ulong			approbal_no;		// ���F�ԍ�
	uchar			reserve[32];
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
} EMONEY_r14;

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
/*--------------------------------------------------------------------------*/
/*	���Ϗ��iQR�R�[�h���ρj
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			amount;				// ���p���z
	uchar			MchTradeNo[32];		// Mch����ԍ�
	uchar			PayTerminalNo[16];	// �x���[��ID
	uchar			DealNo[16];			// ����ԍ�
	uchar			PayKind;			// ���σu�����h
	uchar			dummy;
	uchar			reserve[10];
} QRCODE_rXX;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
/*--------------------------------------------------------------------------*/
/*	���Ϗ��i�N���W�b�g�j
/*--------------------------------------------------------------------------*/
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//typedef struct {
//	ulong			amount;
//	uchar			card_no[20];
//	uchar			cct_num[16];
//	uchar			kid_code[6];
//	ulong			app_no;
//	ulong			center_oiban;
//	uchar			reserve[10];
//} CREINFO;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
typedef struct {
	ulong			amount;				// ���p���z
	uchar			card_no[20];		// ����ԍ�
	uchar			cct_num[16];		// �[�����ʔԍ�
	uchar			kid_code[6];		// KID�R�[�h
	ulong			app_no;				// ���F�ԍ�
	ulong			center_oiban;		// �Z���^�����ʔ�
	uchar			ShopAccountNo[20];	// �����X����ԍ�
	ulong			slip_no;			// �`�[�ԍ�
	uchar			reserve[2];
} CREINFO_r14;
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
typedef struct {	// �N���W�b�g�iID56/57:���Z�f�[�^�p�j
	uchar			CreditCardNo[20];								// �ڼޯĶ��މ����
	ulong			Credit_ryo;										// �ڼޯĶ��ޗ��p���z
	ulong			CreditSlipNo;									// �ڼޯĶ��ޓ`�[�ԍ�
	ulong			CreditAppNo;									// �ڼޯĶ��ޏ��F�ԍ�
	uchar			CreditName[10];									// �ڼޯĶ��މ�Ж�
	uchar			CreditDate[2];									// �ڼޯĶ��ޗL������(�N��)
	ulong			CreditProcessNo;								// �ڼޯĶ��޾�������ǂ���
	uchar			term_id[16];									// �[�����ʔԍ�
} SETTLEMENT_CREDIT;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

typedef struct {	// ���Z�}�́iID152:���Z���f�[�^�p�j
	uchar			card_id[30];									// �J�[�h�ԍ�
	uchar			card_info[16];									// �J�[�h���
	uchar			reserve[2];										// ��
} SETTLEMENT_OUTMEDIA;
typedef union {
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	SETTLEMENT_CREDIT	credit;										// �N���W�b�g�iID56/57:���Z�f�[�^�p�j
	uchar				cardno[16];									// �d�q�}�l�[�iID56/57:���Z�f�[�^�p�j
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	SETTLEMENT_OUTMEDIA	media;
} SETTLEMENT_INFO;
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
/*--------------------------------------------------------------------------*/
/*	�N���W�b�g�J�[�h�f�[�^
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			JIS2Data[69];									// JIS2�����ް�
	uchar			JIS1Data[37];									// JIS1�����ް�
	ushort			PayMethod;										// �x�����@
	ulong			DivCount;										// ������
} CARDDAT;

/*--------------------------------------------------------------------------*/
/*	���ԑ䐔�f�[�^(�ް����58/59)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA2	ParkData;										// ���ԑ䐔
	ulong			Reserve[4];										// �\��
} DATA_KIND_58;
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	PARKCAR_DATA21	ParkData;										// ���ԑ䐔
	ulong			Reserve[4];										// �\��
} DATA_KIND_58_r10;
/*--------------------------------------------------------------------------*/
/*	�Z���^�[�p�[������ް�(�ް����65)										*/
/*--------------------------------------------------------------------------*/
typedef union {
	struct {
		ushort	b15	: 1 ;
		ushort	b14	: 1 ;
		ushort	b13	: 1 ;
		ushort	b12	: 1 ;
		ushort	b11	: 1 ;
		ushort	b10	: 1 ;
		ushort	b09	: 1 ;
		ushort	b08	: 1 ;
		ushort	b07	: 1 ;
		ushort	b06	: 1 ;
		ushort	b05	: 1 ;
		ushort	b04	: 1 ;
		ushort	b03	: 1 ;
		ushort	b02	: 1 ;
		ushort	b01	: 1 ;
		ushort	b00	: 1 ;
	} bits;
	ushort	word;
} REQ_BIT;
// GG120600(S) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
typedef struct {
	uchar		DataProto;			// �ʐM�v���g�R��(1=���uNT-NET/2=RealTime/3=DC-NET)
	uchar		DataSysID;			// �V�X�e��ID
	ushort		DataID;				// ��M�d���f�[�^���
	ushort		DataRev;			// ��M�d�����r�W����
}DATA_REV;
// GG120600(E) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;										// ���M��[���@�B��
	ushort			Result;											// �v������
	REQ_BIT			AcceptReq;										// ��t�\�v��
	uchar			ProgramVer[12];									// ��۸����ް�ޮ�
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb�t�F�[�Y9�Ή�)
	uchar			BaseProgramVer[12];								// �ް���۸����ް�ޮ�
	uchar			RyoCalDllVer[12];								// �����v�Zdll�ް�ޮ�
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb�t�F�[�Y9�Ή�)
	uchar			ModuleVer[15][12];								// Ӽޭ���ް�ޮ�
// GG120600(S) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
	DATA_REV		DataRevs[100];									// ��M�d�����r�W����
// GG120600(E) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
} DATA_KIND_65;

/*--------------------------------------------------------------------------*/
/*	�����ް�����(�ް����100,101)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			ControlData[70];								// �e�퐧��
} DATA_KIND_100;


/*--------------------------------------------------------------------------*/
/*	�Ǘ��ް��v��(�ް����103,104)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			ControlData[70];								// �e�퐧��
} DATA_KIND_103;

#define	NTNET_MANDATA_CLOCK			0x00000001	// ���v�f�[�^
#define	NTNET_MANDATA_CTRL			0x00000002	// ����f�[�^
#define	NTNET_MANDATA_COMMON		0x00000004	// ���ʐݒ�f�[�^
#define	NTNET_MANDATA_INVTKT		0x00000008	// �������Ԍ��f�[�^
#define	NTNET_MANDATA_PASSSTOP		0x00000010	// ��������Z���~�f�[�^
#define	NTNET_MANDATA_PASSSTS		0x00000020	// ������X�e�[�^�X
#define	NTNET_MANDATA_PASSCHANGE	0x00000040	// ������X�V�X�e�[�^�X
#define	NTNET_MANDATA_PASSEXIT		0x00000080	// ������o�Ɏ����e�[�u��
#define	NTNET_MANDATA_LOCKINFO		0x00000100	// �Ԏ��p�����[�^
#define	NTNET_MANDATA_LOCKMARKER	0x00000200	// ���b�N���u�p�����[�^
#define NTNET_MANDATA_SPECIALDAY	0x00000400	// ���ʓ��ݒ�f�[�^
#define NTNET_MANDATA_PARKNUMCTL	0x00000800	// ���ԑ䐔�Ǘ��f�[�^
#define	NTNET_MANDATA_PASSWORD		0x00002000	// �p�X���[�h�f�[�^
#define	NTNET_MANDATA_NGKAKARIID	0x00004000	// �����W��ID�e�[�u��

/*--------------------------------------------------------------------------*/
/*	�ް��v��2(�ް����109)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			ControlData[20];								// �e�퐧��
} DATA_KIND_109;

#define	NTNET_DATAREQ2_TSYOUKEI		0x00000001	// T���v�f�[�^
#define	NTNET_DATAREQ2_GTSYOUKEI	0x00000002	// GT���v�f�[�^
#define	NTNET_DATAREQ2_MTSYOUKEI	0x00000004	// MT���v�f�[�^
#define	NTNET_DATAREQ2_MSYOUKEI		0x00000008	// �������v�f�[�^
#define	NTNET_DATAREQ2_MGOUKEI		0x00000010	// �������v�f�[�^
#define	NTNET_DATAREQ2_COIN			0x00000020	// �R�C�����ɏ��v�f�[�^
#define	NTNET_DATAREQ2_NOTE			0x00000040	// �������ɏ��v�f�[�^
#define	NTNET_DATAREQ2_COUNT		0x00000080	// ����J�E���g�f�[�^
#define	NTNET_DATAREQ2_TERMINFO		0x00000100	// �[�����f�[�^
#define	NTNET_DATAREQ2_TERMSTS		0x00000200	// �[����ԃf�[�^
#define	NTNET_DATAREQ2_MONEY		0x00000400	// ���K�Ǘ��f�[�^
#define	NTNET_DATAREQ2_SYASITU		0x00000800	// �ȈՎԎ����e�[�u��
#define	NTNET_DATAREQ2_NMSYOUKEI	0x00001000	// ���ݕ������v�f�[�^

/*--------------------------------------------------------------------------*/
/*	�Z���^�p�f�[�^�v��(�ް����154)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {						// 
	DATA_BASIC		DataBasic;			// ��{�ް�
	ushort			FmtRev;				// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;			// ���M��[���@�B��
	uchar			SeisanJyoutai_Req;	// ���Z��ԃf�[�^�v��(0���v���Ȃ��^1���v������)
	uchar			RealTime_Req;		// ���A���^�C�����v��(0���v���Ȃ��^1���v������)
	uchar			Reserve1;			// �\��1(0���v���Ȃ��^1���v������)
	uchar			Reserve2;			// �\��2(0���v���Ȃ��^1���v������)
	uchar			Reserve3;			// �\��3(0���v���Ȃ��^1���v������)
	uchar			Reserve4;			// �\��4(0���v���Ȃ��^1���v������)
	uchar			Reserve5;			// �\��5(0���v���Ȃ��^1���v������)
	uchar			Reserve6;			// �\��6(0���v���Ȃ��^1���v������)
	uchar			SynchroTime_Req;	// ���������f�[�^�v��(0���v���Ȃ��^1���v������)
	uchar			TermInfo_Req;		// �[�����f�[�^�v��(0���v���Ȃ��^1���v������)
} DATA_KIND_154;

/*--------------------------------------------------------------------------*/
/*	���v�ް�(�ް����119,229)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Year;											// Year
	uchar			Mon;											// Month
	uchar			Day;											// Day
	uchar			Hour;											// Hour
	uchar			Min;											// Minute
	ushort			Sec;											// Second
	ushort			MSec;											// Millisecond
	ulong			HOSEI_MSec;										// �␳�l
	ulong			HOSEI_wk;										// �␳�l ��ƈ�
} DATA_KIND_119;


/*--------------------------------------------------------------------------*/
/*	�װ�ް�(�ް����120)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	uchar			Errsyu;											// �װ���
	uchar			Errcod;											// �װ����
	uchar			Errdtc;											// �װ�ð��(����/����)
	uchar			Errlev;											// �װ����
	uchar			Errdat1[10];									// �װ���(�޲��)
	uchar			Errdat2[160];									// �װ���(acsii����)
} DATA_KIND_120;


/*--------------------------------------------------------------------------*/
/*	�װ��ް�(�ް����121)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	uchar			Armsyu;											// �װю��
	uchar			Armcod;											// �װѺ���
	uchar			Armdtc;											// �װѽð��(����/����)
	uchar			Armlev;											// �װ�����
	uchar			Armdat1[10];									// �װя��(�޲��)
	uchar			Armdat2[160];									// �װя��(acsii����)
} DATA_KIND_121;

/*--------------------------------------------------------------------------*/
/*	�װ�ް�(�ް����63)														*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	uchar			Errsyu;											// �װ���
	uchar			Errcod;											// �װ����
	uchar			Errdtc;											// �װ�ð��(����/����)
	uchar			Errlev;											// �װ����
	uchar			ErrDoor;										// �ޱ���(0:close,1:open)
	uchar			Errdat[10];										// �װ���(�޲��)
} DATA_KIND_63;


/*--------------------------------------------------------------------------*/
/*	�װ��ް�(�ް����64)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	uchar			Armsyu;											// �װю��
	uchar			Armcod;											// �װѺ���
	uchar			Armdtc;											// �װѽð��(����/����)
	uchar			Armlev;											// �װ�����
	uchar			ArmDoor;										// �ޱ���(0:close,1:open)
	uchar			Armdat[10];										// �װя��(�޲��)
} DATA_KIND_64;


/*--------------------------------------------------------------------------*/
/*	���K�Ǘ��ް�(�ް����126)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			Mai;											// �ۗL����
	ushort			Money;											// ���z
} MONEY_DATA;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	ulong			PayCount;										// ���Z�ǂ���
	ushort			PayClass;										// �����敪
	date_time_rec2	PayTime;										// ���Z�N���������b
	ushort			KakariNo;										// �W����
	MONEY_DATA		CoinSf[8];										// ��݋����ް�
	ulong			CoinSfTotal;									// ��݋��ɑ��z
	MONEY_DATA		NoteSf[8];										// ���������ް�
	ulong			NoteSfTotal;									// �������ɑ��z
	MONEY_DATA		Coin[8];										// ��ݏz���ް�
	MONEY_DATA		CoinYotiku[2];									// ��ݗ\�~�ް�
	MONEY_DATA		NoteChange[8];									// �������o���ް�
} DATA_KIND_126;

/*--------------------------------------------------------------------------*/
/*	�ޑK�Ǘ��W�v�ް�(�ް����135)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			Kind;											// ����
	ushort			gen_mai;										// ���݁i�ύX�O�j�ۗL����
	ushort			zen_mai;										// �O��ۗL����
	ushort			sei_nyu;										// ���Z����������
	ushort			sei_syu;										// ���Z���o������
	ushort			jyun_syu;										// �z�o������
	ushort			hojyu;											// �ޑK��[����
	ushort			hojyu_safe;										// �ޑK��[�����ɔ�������
	ushort			turi_kyosei;									// �������o����(�ޑK��)
	ushort			kin_kyosei;										// �������o����(����)
	ushort			sin_mai;										// �V�K�ݒ薇��
} TURI_DATA;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			Oiban;											// ���K�Ǘ����v�ǔ�
	ushort			PayClass;										// �����敪
	ushort			KakariNo;										// �W����
	TURI_DATA		turi_dat[4];									// ���K�ް�(4���핪)
	TURI_DATA		yturi_dat[5];									// ���K�ް�(5�\�~��)
} DATA_KIND_135;

/*--------------------------------------------------------------------------*/
/*	������⍇���ް�(�ް����142)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			ParkingNo;										// ���ԏꇂ
	ulong			PassID;											// �����ID
} DATA_KIND_142;


/*--------------------------------------------------------------------------*/
/*	������⍇�������ް�(�ް����143)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Reserve;										// �\��
	ulong			ParkingNo;										// ���ԏꇂ
	ulong			PassID;											// �����ID
	uchar			PassState;										// ������ð��
	uchar			RenewalState;									// �X�V�ð��
	date_time_rec	OutTime;										// �o�ɔN��������
	ulong			UseParkingNo;									// ���p�����ԏꇂ
} DATA_KIND_143;


/*--------------------------------------------------------------------------*/
/*	�[������ް�(�ް����230)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			ProgramVer[12];									// ��۸����ް�ޮ�
	uchar			ModuleVer[10][12];								// Ӽޭ���ް�ޮ�
} DATA_KIND_230;


/*--------------------------------------------------------------------------*/
/*	�[������ް�(�ް����231)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			TerminalSt;										// �[�����
	uchar			OpenClose;										// �c�x��
	uchar			LC1_State;										// LC1���
	uchar			LC2_State;										// LC2���
	uchar			LC3_State;										// LC3���
	uchar			Gate_State;										// �ްď��
	uchar			CardNGType;										// NG���ޓǎ���e
} DATA_KIND_231;


/*--------------------------------------------------------------------------*/
/*	���ʐݒ��ް�(�ް����80,208)											*/
/*--------------------------------------------------------------------------*/
#define	NTNET_PARAMDATA_MAX				998
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	ushort			uMode;											// �X�VӰ��
	ulong			ModelCode;										// �ݒ�f�[�^�̋@��R�[�h
	ushort			Status;											// �ް��ð��
	ushort			Segment;										// �ݒ辸����
	ushort			TopAddr;										// �J�n���ڽ
	ushort			DataCount;										// �ݒ��ް���
	ulong			Data[NTNET_PARAMDATA_MAX];						// �ݒ��ް�
} DATA_KIND_80;

enum {
	_RPKERR_INVALID_MACHINENO = 140,
	_RPKERR_INVALID_MODELCODE,
	_RPKERR_INVALID_PARA_ADDR,
	_RPKERR_NO_PREPARE_COMMAND,
	_RPKERR_COMMAND_REJECT,

	_RPKERR_MAX
};

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			ProcMode;										// �����敪
	uchar			dummy_1;
	ulong			dummy_4;										// (�ݒ�f�[�^�̋@��R�[�h)
	ushort			Status;											// �ް��ð��
	ushort			Segment;										// �ݒ辸����
	ushort			TopAddr;										// �J�n���ڽ
	ushort			DataCount;										// �ݒ��ް���
	long			Data[NTNET_PARAMDATA_MAX];						// �ݒ��ް�
} RP_DATA_KIND_80;

#define	RP_PARAMDATA_MAX	30
#define	RP_DATA80_LEN		_offsetof(RP_DATA_KIND_80, Data[RP_PARAMDATA_MAX])

/*--------------------------------------------------------------------------*/
/*	�ݒ��ް��v��(�ް����78,90)												*/
/*--------------------------------------------------------------------------*/
// ���p�[�N��p�t�H�[�}�b�g�Ƃ���
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ulong			SMachineNo;								// ���M��[���@�B��
	ulong			dummy_4;								// (�ݒ�f�[�^�̋@��R�[�h)
	ushort			dummy_2;								// (���Ұ��敪)
	ushort			Segment;								// �ݒ辸����
	ushort			TopAddr;								// �J�n���ڽ
	ushort			DataCount;								// �ݒ��ް���
	uchar			Reserve[72];							// �\��
} RP_DATA_KIND_78;


/*--------------------------------------------------------------------------*/
/*	������Z���~�e�[�u��(�ް����91,219)									*/
/*--------------------------------------------------------------------------*/

typedef	struct {
	ulong			ParkingNo;										// ���ԏꇂ
	ulong			PassID;											// �����ID
	uchar			PassKind;										// ��������
	uchar			ChargeType;										// �������
	ushort			Year;											// ���ɔN
	uchar			Month;											// ���Ɍ�
	uchar			Day;											// ���ɓ�
	uchar			Hour;											// ���Ɏ�
	uchar			Min;											// ���ɕ�
	ushort			Sec;											// ���ɕb
	ulong			CouponRyo;										// �񐔌��̎��z
	ushort			ShopNo;											// �X�ԍ�
	uchar			ServiceTicketA;									// �T�[�r�X���`
	uchar			ServiceTicketB;									// �T�[�r�X���a
	uchar			ServiceTicketC;									// �T�[�r�X���b
	uchar			UseCount;										// �g�p����
	ulong			DiscountMoney;									// �������z
	ulong			DiscountTime;									// ��������
	ushort			DiscountRate;									// ������
} TEIKI_CHUSI;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Dummy;											// ���M��[���@�B��
	ushort			ProcMode;										// �����敪
	TEIKI_CHUSI		TeikiChusi[TKI_CYUSI_MAX];						// �e�퐧��
} DATA_KIND_91, DATA_KIND_219;

/*--------------------------------------------------------------------------*/
/*	������ð��ð���(�ް����93,221)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	PAS_TBL			PassTable[PAS_MAX];								// ������ð��ð���
} DATA_KIND_93;

/*--------------------------------------------------------------------------*/
/*	������X�Vð���(�ް����94,222)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	PAS_RENEWAL		PassRenewal[PAS_MAX/4];							// ������X�Vð���
} DATA_KIND_94;

/*--------------------------------------------------------------------------*/
/*	������o�Ɏ���ð���(�ް����95,223)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	PASS_EXTBL		PassExTable[PASS_EXTIMTBL_MAX];					// ������o�Ɏ���ð���
} DATA_KIND_95;

/*--------------------------------------------------------------------------*/
/*	�Ԏ��p�����[�^��M(�ް����97,225)										*/
/*--------------------------------------------------------------------------*/

typedef	struct {
	ushort		lok_syu;											// ۯ����u��ʁi0:�����@�P�`�U�j
	ushort		ryo_syu;											// �������	�i0:�����@�P�`�P�Q�j
	ushort		area;												// ���(1�`26)
	ulong		posi;												// ���Ԉʒu��(1�`9999)
	ushort		if_oya;												// �eIF�Շ��i�P�`�P�O�j
	ushort		lok_no;												// �eIF�Ղɑ΂��鑕�u�A�ԁi�P�`�X�O�j
} LOCK_INFO;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	ulong			ModelCode;										// �ݒ�f�[�^�̋@��R�[�h
	LOCK_INFO		LockInfo[OLD_LOCK_MAX];							// �e�퐧��
} DATA_KIND_97;

typedef DATA_KIND_97	DATA_KIND_225;

/*--------------------------------------------------------------------------*/
/*	�ݒ��M�����ʒm�ް�(�ް����99)										*/
/*--------------------------------------------------------------------------*/
// �ݒ芮�������ް�
enum {
	NTNET_COMPLETE_TIME=0,								// ���v�ް�
	NTNET_COMPLETE_CTRL,								// �����ް�
	NTNET_COMPLETE_CPRM,								// ���ʐݒ��ް�
	NTNET_COMPLETE_PPRM,								// �ʐݒ��ް�
	NTNET_COMPLETE_UPRM,								// հ�ް���Ұ��ް�
	NTNET_COMPLETE_RSV1,								// (���ݒ��ԑ䐔�ް�)
	NTNET_COMPLETE_RSV2,								// (���ԑ䐔�ް�)
	NTNET_COMPLETE_HEADER,								// �̎���ͯ�ް�ް�
	NTNET_COMPLETE_FOOTER,								// �̎���̯���ް�
	NTNET_COMPLETE_LOGO,								// ۺވ��ް�
	NTNET_COMPLETE_BMP,									// �ޯ�ϯ���ް�
	NTNET_COMPLETE_MUKOU,								// �������Ԍ�ð����ް�
	NTNET_COMPLETE_RSV3,								// �\��
	NTNET_COMPLETE_TEIKI_TYUSHI,						// ��������Z���~ð����ް�
	NTNET_COMPLETE_RSV4,								// �\��
	NTNET_COMPLETE_TEIKI_STS,							// ������ð��ð���
	NTNET_COMPLETE_TEIKI_UPDATE,						// ������X�Vð���
	NTNET_COMPLETE_TEIKI_CAROUT,						// ������o�Ɏ���ð���
	NTNET_COMPLETE_SPDAY,								// ���ʓ��ݒ��ް�
	NTNET_COMPLETE_SHASHITSU_PRM,						// �Ԏ����Ұ��ݒ�
	NTNET_COMPLETE_LOCK_PRM,							// ۯ����u���Ұ��ݒ�
	NTNET_COMPLETE_USRSET,								// հ�ް�ݒ��ް�
	NTNET_COMPLETE_REQ_TCKT,							// (�����v��)
	NTNET_COMPLETE_RSV5,								// (��斞�󐧌��ް�)
	NTNET_COMPLETE_RSV6,								// (���c�x�Ɛ����ް�)
	NTNET_COMPLETE_PRKCTRL,								// ���ݑ䐔�Ǘ��ް�
	NTNET_COMPLETE_RSV7,								// �\��

	NTNET_COMPLETE_MAX=70
};

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			CompleteInfo[NTNET_COMPLETE_MAX];				// �ݒ芮�����
} DATA_KIND_99;

/*--------------------------------------------------------------------------*/
/*	�ް��v��2����NG(�ް����110)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;
	ulong			SMachineNo;
	uchar			ControlData[20];
	ushort			Result;
	ulong			ErrTerminal[32];
} DATA_KIND_110;

/*--------------------------------------------------------------------------*/
/*	�Z���^�p�f�[�^�v������NG(�ް����155)									*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;
	ushort			FmtRev;				// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;
	uchar			SeisanJyoutai_Req;	// ���Z��ԃf�[�^�v��(0���v���Ȃ��^1���v������)
	uchar			RealTime_Req;		// ���A���^�C�����v��(0���v���Ȃ��^1���v������)
	uchar			Reserve1;			// �\��1(0���v���Ȃ��^1���v������)
	uchar			Reserve2;			// �\��2(0���v���Ȃ��^1���v������)
	uchar			Reserve3;			// �\��3(0���v���Ȃ��^1���v������)
	uchar			Reserve4;			// �\��4(0���v���Ȃ��^1���v������)
	uchar			Reserve5;			// �\��5(0���v���Ȃ��^1���v������)
	uchar			Reserve6;			// �\��6(0���v���Ȃ��^1���v������)
	uchar			SynchroTime_Req;	// ���������f�[�^�v��(0���v���Ȃ��^1���v������)
	uchar			TermInfo_Req;		// �[�����f�[�^�v��(0���v���Ȃ��^1���v������)
	ushort			Result;				// ����(9=�p�����[�^NG)
	ulong			ErrTerminal[32];
} DATA_KIND_155;

/*--------------------------------------------------------------------------*/
/*	������f�[�^�X�V�@������`�F�b�N�f�[�^(�ް����116,117)					*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;
	ushort			ProcMode;										// �����敪
	ulong			ParkingNo;										// ���ԏꇂ
	ulong			PassID;											// �����ID
	uchar			Status;											// �X�e�[�^�X
	uchar			UpdateStatus;									// �X�V�X�e�[�^�X
	ushort			OutYear;										// �o�ɔN
	uchar			OutMonth;										// �o�Ɍ�
	uchar			OutDay;											// �o�ɓ�
	uchar			OutHour;										// �o�Ɏ�
	uchar			OutMin;											// �o�ɕ�
	ulong			UsingParkingNo;									// �g�p�����ԏꇂ
} DATA_KIND_116, DATA_KIND_117;

#define	NTNET_PASSUPDATE_STATUS			0x00000001
#define	NTNET_PASSUPDATE_UPDATESTATUS	0x00000002
#define	NTNET_PASSUPDATE_OUTTIME		0x00000004

enum {
	NTNET_PASSUPDATE_INITIAL = 0,									// �������		0
	NTNET_PASSUPDATE_OUT,											// �o�ɒ�		1
	NTNET_PASSUPDATE_IN,											// ���ɒ�		2
	NTNET_PASSUPDATE_INVALID,										// ����			3
	NTNET_PASSUPDATE_ALL_INITIAL,									// �S������		4
	NTNET_PASSUPDATE_ALL_IN,										// �S���ɒ�		5
	NTNET_PASSUPDATE_ALL_OUT,										// �S�o�ɒ�		6
	NTNET_PASSUPDATE_ALL_OUT_FREE,									// �S�o�Ƀt���[	7
	NTNET_PASSUPDATE_ALL_IN_FREE,									// �S���Ƀt���[	8
	NTNET_PASSUPDATE_ALL_INVALID,									// �S����		9
	NTNET_PASSUPDATE_ALL_VALID,										// �S�L��		10
	NTNET_PASSUPDATE_ALL_INITIAL2									// �S�������i�L������̂݁j11
};

/*--------------------------------------------------------------------------*/
/*	���j�^�f�[�^(�ް����122)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	uchar			Monsyu;											// ���j�^���
	uchar			Moncod;											// ���j�^�R�[�h
	uchar			dummy;											// �\��
	uchar			Monlev;											// ���j�^���x��
	uchar			Mondat1[10];									// ���j�^���
	uchar			Mondat2[160];									// ���j�^���b�Z�[�W�i�\��j
} DATA_KIND_122;


/*--------------------------------------------------------------------------*/
/*	���샂�j�^�f�[�^(�ް����123)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	uchar			OpeMonsyu;										// ���샂�j�^���
	uchar			OpeMoncod;										// ���샂�j�^�R�[�h
	uchar			OpeMonlev;										// ���샂�j�^���x��
	uchar			OpeMondat1[4];									// �ύX�O�f�[�^
	uchar			OpeMondat2[4];									// �ύX��f�[�^
	uchar			OpeMondat3[160];								// ���샂�j�^���b�Z�[�W�i�\��j
} DATA_KIND_123;

/*--------------------------------------------------------------------------*/
/*	ۯ����u���Ұ�(�ް����98/226)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			in_tm;											/* ���Ɏԗ����m��ϰ				*/
	ushort			ot_tm;											/* �o�Ɏԗ����m��ϰ				*/
	ushort			r_cnt;											/* ۯ����u��ײ��				*/
	ushort			r_tim;											/* ۯ����u��ײ�Ԋu				*/
	ushort			open_tm;										/* �J����M���o�͎���			*/
	ushort			clse_tm;										/* ����M���o�͎���			*/
} t_LockMaker_NtNet;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	ulong			ModelCode;										// �ݒ�f�[�^�̋@��R�[�h
	t_LockMaker_NtNet	LockMaker[6];								// ۯ����u���Ұ�
} DATA_KIND_98;

typedef DATA_KIND_98 DATA_KIND_226;

/*--------------------------------------------------------------------------*/
/*	�W�v��{�ް�(�ް����30/42/54/66/158/170/182/194)						*/
/*--------------------------------------------------------------------------*/

typedef struct {
	ulong			Num;											// ��
	ulong			Amount;											// ���z
} t_SyuSub;

typedef struct {
	ushort			Num;											// ��
	ulong			Amount;											// ���z
} t_SyuSub2;

typedef struct {
	t_SyuSub		CarOutIllegal;									// �s���o��
	t_SyuSub		CarOutForce;									// �����o��
	ulong			AcceptTicket;									// ��t�����s��
	t_SyuSub		ModifySettle;									// �C�����Z
	ulong			Rsv[2];											// �\��1�`2
} t_SyuSub_AllSystem;

typedef struct {
	t_SyuSub		CarOutIllegal;									// �s���o��
	t_SyuSub		CarOutForce;									// �����o��
	ulong			AcceptTicket;									// ��t�����s��
	t_SyuSub		ModifySettle;									// �C�����Z
} t_SyuSub_AllSystem2;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	date_time_rec2	NowTime;										// ����W�v
	date_time_rec2	LastTime;										// �O��W�v
	ulong			SettleNum;										// �����Z��
	ulong			Kakeuri;										// ���|���z
	ulong			Cash;											// ����������z
	ulong			Uriage;											// ������z
	ulong			Tax;											// ������Ŋz
	ulong			Charge;											// �ޑK���ߊz
	ulong			CoinTotalNum;									// �R�C�����ɍ��v��
	ulong			NoteTotalNum;									// �������ɍ��v��
	ulong			CyclicCoinTotalNum;								// �z�R�C�����v��
	ulong			NoteOutTotalNum;								// �������o�@���v��
	ulong			Rsv1[3];										// �\��
	ulong			CarOutWithoutPay;								// ���Z�Ȃ����[�h���o�ɑ䐔
	ulong			SettleNumServiceTime;							// �T�[�r�X�^�C�������Z��
	ulong			CarOutLagTime;									// ���O�^�C�����o�ɉ�
	t_SyuSub		Shortage;										// ���o�s��
	t_SyuSub		Cancel;											// ���Z���~
	t_SyuSub		AutoSettle;										// ���������Z
	t_SyuSub		ManualSettle;									// �}�j���A�����Z
	ulong			AntiPassOffSettle;								// �A���`�p�XOFF���Z��
	ulong			CarOutGateOpen;									// �Q�[�g�J�����o�ɑ䐔
	ulong			CarOutForce;									// �����o�ɑ䐔
	t_SyuSub		LostSettle;										// �������Z
	ulong			ReceiptCallback;								// �̎��؉������
	ulong			ReceiptIssue;									// �̎��ؔ��s����
	ulong			WarrantIssue;									// �a��ؔ��s����
	t_SyuSub_AllSystem	AllSystem;									// �S���u
	ulong			CarInTotal;										// �����ɑ䐔
	ulong			CarOutTotal;									// ���o�ɑ䐔
	ulong			CarIn1;											// ����1���ɑ䐔
	ulong			CarOut1;										// �o��1�o�ɑ䐔
	ulong			CarIn2;											// ����2���ɑ䐔
	ulong			CarOut2;										// �o��2�o�ɑ䐔
	ulong			CarIn3;											// ����3���ɑ䐔
	ulong			CarOut3;										// �o��3�o�ɑ䐔
	ulong			MiyoCount;										// ��������
	ulong			MiroMoney;										// �������z
	ulong			LagExtensionCnt;								// ���O�^�C��������
	ulong			Rsv2[11];										// �\��1�`11
} DATA_KIND_30;


typedef DATA_KIND_30 DATA_KIND_66;
typedef DATA_KIND_30 DATA_KIND_158;
typedef DATA_KIND_30 DATA_KIND_170;
typedef DATA_KIND_30 DATA_KIND_182;
typedef DATA_KIND_30 DATA_KIND_194;

/*--------------------------------------------------------------------------*/
/*	������ʖ��W�v�ް�(�ް����31/43/55/67/159/171/183/195)					*/
/*--------------------------------------------------------------------------*/

typedef struct {
	ushort			Kind;											// �������
	t_SyuSub		Settle;											// ���Z����
	t_SyuSub		Discount;										// ����
} t_SyuSub_RyokinKind;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	t_SyuSub_RyokinKind	Kind[50];									// ���01�`50
} DATA_KIND_31;

typedef DATA_KIND_31 DATA_KIND_55;
typedef DATA_KIND_31 DATA_KIND_67;
typedef DATA_KIND_31 DATA_KIND_159;
typedef DATA_KIND_31 DATA_KIND_171;
typedef DATA_KIND_31 DATA_KIND_183;
typedef DATA_KIND_31 DATA_KIND_195;

/*--------------------------------------------------------------------------*/
/*	���ޏW�v�ް�(�ް����32/44/56/68/160/172/184/196)						*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	ulong			Kind;											// ���ԕ��ޏW�v�̎��
	t_SyuSub		Group[BUNRUI_CNT];								// ����1�`48
	t_SyuSub		GroupTotal;										// ���ވȏ�
	ulong			LostSettle;										// �������Z �䐔
	ulong			AntiPassOffSettle;								// �A���`�p�XOFF���Z �䐔
	ulong			CarOutForce;									// �����o�� �䐔
	ulong			FirstTimePass;									// ������ �䐔
	t_SyuSub		Unknown;										// ���ޕs��
	uchar			Rsv[32];										// �\��
	ulong			Kind2;											// ���ԕ��ޏW�v�̎��_2
	t_SyuSub		Group2[BUNRUI_CNT];								// ����1�`48_2
	t_SyuSub		GroupTotal2;										// ���ވȏ�_2
	ulong			LostSettle2;										// �������Z �䐔_2
	ulong			AntiPassOffSettle2;								// �A���`�p�XOFF���Z �䐔_2
	ulong			CarOutForce2;									// �����o�� �䐔_2
	ulong			FirstTimePass2;									// ������ �䐔_2
	t_SyuSub		Unknown2;										// ���ޕs��_2
	uchar			Rsv2[32];										// �\��_2
	ulong			Kind3;											// ���ԕ��ޏW�v�̎��_3
	t_SyuSub		Group3[BUNRUI_CNT];								// ����1�`48_3
	t_SyuSub		GroupTotal3;										// ���ވȏ�_3
	ulong			LostSettle3;										// �������Z �䐔_3
	ulong			AntiPassOffSettle3;								// �A���`�p�XOFF���Z �䐔_3
	ulong			CarOutForce3;									// �����o�� �䐔_3
	ulong			FirstTimePass3;									// ������ �䐔_3
	t_SyuSub		Unknown3;										// ���ޕs��_3
	uchar			Rsv3[32];										// �\��_3
} DATA_KIND_32;

typedef DATA_KIND_32 DATA_KIND_68;
typedef DATA_KIND_32 DATA_KIND_160;
typedef DATA_KIND_32 DATA_KIND_172;
typedef DATA_KIND_32 DATA_KIND_196;

/*--------------------------------------------------------------------------*/
/*	�����W�v�ް�(�ް����33/45/57/69/161/173/185/197)						*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;										// ���ԏ�No.
	ushort			Kind;											// �������
	ushort			Group;											// �����敪
	ulong			Num;											// ������
	ulong			Callback;										// �������
	ulong			Amount;											// �����z
	ulong			Info;											// �������
	ulong			Rsv;											// �\��
} t_SyuSub_Discount;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	t_SyuSub_Discount	Discount[500];								// ���� 001�`500
} DATA_KIND_33;

typedef DATA_KIND_33 DATA_KIND_57;
typedef DATA_KIND_33 DATA_KIND_69;
typedef DATA_KIND_33 DATA_KIND_161;
typedef DATA_KIND_33 DATA_KIND_173;
typedef DATA_KIND_33 DATA_KIND_197;

/*--------------------------------------------------------------------------*/
/*	����W�v�ް�(�ް����34/46/58/70/162/174/186/198)						*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;										// ���ԏ�No.
	ushort			Kind;											// ���
	ulong			Num;											// ��
	ulong			Callback;										// �������
	t_SyuSub		Update;											// �X�V
	ulong			Rsv;											// �\��
} t_SyuSub_Pass;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	t_SyuSub_Pass	Pass[100];										// ����� 001�`100
} DATA_KIND_34;

typedef DATA_KIND_34 DATA_KIND_70;
typedef DATA_KIND_34 DATA_KIND_162;
typedef DATA_KIND_34 DATA_KIND_174;
typedef DATA_KIND_34 DATA_KIND_198;

/*--------------------------------------------------------------------------*/
/*	�Ԏ����W�v�ް�(�ް����35/47/59/71/163/175/187/199)						*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			LockNo;											// �����
	ulong			CashAmount;										// ��������
	ulong			Settle;											// ���Z��
	t_SyuSub		CarOutIllegal;									// �s���o��
	t_SyuSub		CarOutForce;									// �����o��
	ulong			AcceptTicket;									// ��t�����s��
	t_SyuSub		ModifySettle;									// �C�����Z
	ulong			Rsv[2];											// �\��1�`2
} t_SyuSub_Lock;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	t_SyuSub_Lock	Lock[OLD_LOCK_MAX];								// �Ԏ�001�`324
} DATA_KIND_35;

typedef DATA_KIND_35 DATA_KIND_47;
typedef DATA_KIND_35 DATA_KIND_59;
typedef DATA_KIND_35 DATA_KIND_71;
typedef DATA_KIND_35 DATA_KIND_163;
typedef DATA_KIND_35 DATA_KIND_175;
typedef DATA_KIND_35 DATA_KIND_199;

/*--------------------------------------------------------------------------*/
/*	���K�W�v�ް�(�ް����36/48/60/72/164/176/188/200)						*/
/*--------------------------------------------------------------------------*/

typedef struct {
	ushort			Kind;											// ����
	ulong			Num;											// ����
} t_SyuSub_Coin;

typedef struct {
	ushort			Kind;											// ����
	ushort			Num;											// ����
} t_SyuSub_Coin2;

typedef struct {
	ushort			Kind;											// ����
	ulong			Num1;											// ����1
	ulong			Num2;											// ����2
} t_SyuSub_Note;

typedef struct {
	ushort			Kind;											// ����
	ushort			Num1;											// ����1
	ushort			Num2;											// ����2
} t_SyuSub_Note2;

typedef struct {
	ushort			CoinKind;										// �R�C������
	ulong			Accept;											// ��������
	ulong			Pay;											// �o������
	ulong			ChargeSupply;									// �ޑK��[����
	ulong			SlotInventory;									// �C���x���g������(��o��)
	ulong			CashBoxInventory;								// �C���x���g������(����)
	ulong			Hold;											// �ۗL����
} t_SyuSub_Cycle;

typedef struct {
	ushort			CoinKind;										// �R�C������
	ulong			Pay;											// �o������
	ulong			ChargeSupply;									// �ޑK��[����
	ulong			SlotInventory;									// �C���x���g������(��o��)
	ulong			CashBoxInventory;								// �C���x���g������(����)
	ulong			Hold;											// �ۗL����
} t_SyuSub_Stock;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	ulong			Total;											// ���ɑ������z
	ulong			NoteTotal;										// �������ɑ������z
	ulong			CoinTotal;										// �R�C�����ɑ������z
	t_SyuSub_Coin	Coin[4];										// �R�C��1�`4
	t_SyuSub_Coin	CoinRsv[4];										// �R�C���\��1�`4
	t_SyuSub_Note	Note[4];										// ����1�`4
	t_SyuSub_Note	NoteRsv[4];										// �����\��1�`4
	ulong			CycleAccept;									// �z���������z
	ulong			CyclePay;										// �z�����o���z
	ulong			NoteAcceptTotal;								// �����������z
	ulong			NotePayTotal;									// ���������o���z
	ulong			StockPayTotal;									// �\�~�����o���z
	t_SyuSub_Cycle	Cycle[4];										// �z��1�`4
	t_SyuSub_Cycle	CycleRsv[4];									// �z�\��1�`4
	t_SyuSub_Stock	Stock[2];										// �\�~��1�`2
	t_SyuSub_Note	NotePay[4];										// �������o1�`4
	t_SyuSub_Note	NotePayRsv[4];									// �������o�\��1�`4
} DATA_KIND_36;

typedef DATA_KIND_36 DATA_KIND_60;
typedef DATA_KIND_36 DATA_KIND_72;
typedef DATA_KIND_36 DATA_KIND_164;
typedef DATA_KIND_36 DATA_KIND_176;
typedef DATA_KIND_36 DATA_KIND_200;

/*--------------------------------------------------------------------------*/
/*	�W�v�I���ʒm�ް�(�ް����41/53/65/77/169/181/193/205)					*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	ulong			MachineNo[32];									// �����W�v�����[���̋@��No.-001�`-032
} DATA_KIND_41;

typedef DATA_KIND_41 DATA_KIND_77;
typedef DATA_KIND_41 DATA_KIND_169;
typedef DATA_KIND_41 DATA_KIND_181;
typedef DATA_KIND_41 DATA_KIND_193;
typedef DATA_KIND_41 DATA_KIND_205;

// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
///*--------------------------------------------------------------------------*/
///*	�Z���^�[Phase2�W�v��{�ް�(�ް����42)									*/
///*--------------------------------------------------------------------------*/
//typedef struct {
//	DATA_BASIC		DataBasic;										// ��{�ް�
//	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
//	ushort			Type;											// �W�v�^�C�v
//	ushort			KakariNo;										// �W��No.
//	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
//	ulong			SeqNo;											// �W�v�ǔ�
//	ulong			StartSeqNo;										// �J�n�ǔ�
//	ulong			EndSeqNo;										// �I���ǔ�
//	date_time_rec3	LastTime;										// �O��W�v
//	ulong			SettleNum;										// �����Z��
//	ulong			Kakeuri;										// ���|���z
//	ulong			Cash;											// ����������z
//	ulong			Uriage;											// ������z
//	ulong			Tax;											// ������Ŋz
//	ulong			Charge;											// �ޑK���ߊz
//	ushort			CoinTotalNum;									// �R�C�����ɍ��v��
//	ushort			NoteTotalNum;									// �������ɍ��v��
//	ushort			CyclicCoinTotalNum;								// �z�R�C�����v��
//	ushort			NoteOutTotalNum;								// �������o�@���v��
//	ulong			CarOutWithoutPay;								// ���Z�Ȃ����[�h���o�ɑ䐔
//	ulong			SettleNumServiceTime;							// �T�[�r�X�^�C�������Z��
//	ulong			CarOutLagTime;									// ���O�^�C�����o�ɉ�
//	t_SyuSub		Shortage;										// ���o�s�� �񐔁^���z
//	t_SyuSub		Cancel;											// ���Z���~ �񐔁^���z
//	t_SyuSub		AutoSettle;										// ���������Z �񐔁^���z
//	t_SyuSub		ManualSettle;									// �}�j���A�����Z �񐔁^���z
//	ulong			AntiPassOffSettle;								// �A���`�p�XOFF���Z��
//	ulong			CarOutGateOpen;									// �Q�[�g�J�����o�ɑ䐔
//	ulong			CarOutForce;									// �����o�ɑ䐔
//	t_SyuSub		LostSettle;										// �������Z �񐔁^���z
//	ulong			ReceiptCallback;								// �̎��؉������
//	ulong			ReceiptIssue;									// �̎��ؔ��s����
//	ulong			WarrantIssue;									// �a��ؔ��s����
//	t_SyuSub_AllSystem2	AllSystem;									// �S���u
//	ulong			CarInTotal;										// �����ɑ䐔
//	ulong			CarOutTotal;									// ���o�ɑ䐔
//	ulong			CarIn1;											// ����1���ɑ䐔
//	ulong			CarOut1;										// �o��1�o�ɑ䐔
//	ulong			CarIn2;											// ����2���ɑ䐔
//	ulong			CarOut2;										// �o��2�o�ɑ䐔
//	ulong			CarIn3;											// ����3���ɑ䐔
//	ulong			CarOut3;										// �o��3�o�ɑ䐔
//	ulong			MiyoCount;										// ��������
//	ulong			MiroMoney;										// �������z
//	ulong			LagExtensionCnt;								// ���O�^�C��������
//	ulong			Total;											// ���ɑ������z
//	ulong			NoteTotal;										// �������ɑ������z
//	ulong			CoinTotal;										// �R�C�����ɑ������z
//	t_SyuSub_Coin2	Coin[4];										// �R�C��1�`4
//	t_SyuSub_Note2	Note[4];										// ����1�`4
//} DATA_KIND_42;
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(S) K.Onodera 2016/11/04 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
/*--------------------------------------------------------------------------*/
/*	�Z���^�[Phase7�W�v��{�ް�(�ް����42)									*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	date_time_rec3	LastTime;										// �O��W�v
	ulong			SettleNum;										// �����Z��
	ulong			Kakeuri;										// ���|���z
	ulong			Cash;											// ����������z
	ulong			Uriage;											// ������z
	ulong			Tax;											// ������Ŋz
	ulong			Excluded;										// ������ΏۊO���z
	ulong			Charge;											// �ޑK���ߊz
	ushort			CoinTotalNum;									// �R�C�����ɍ��v��
	ushort			NoteTotalNum;									// �������ɍ��v��
	ushort			CyclicCoinTotalNum;								// �z�R�C�����v��
	ushort			NoteOutTotalNum;								// �������o�@���v��
	ulong			CarOutWithoutPay;								// ���Z�Ȃ����[�h���o�ɑ䐔
	ulong			SettleNumServiceTime;							// �T�[�r�X�^�C�������Z��
	ulong			CarOutLagTime;									// ���O�^�C�����o�ɉ�
	t_SyuSub		Shortage;										// ���o�s�� �񐔁^���z
	t_SyuSub		Cancel;											// ���Z���~ �񐔁^���z
	t_SyuSub		AutoSettle;										// ���������Z �񐔁^���z
	t_SyuSub		ManualSettle;									// �}�j���A�����Z �񐔁^���z
	ulong			AntiPassOffSettle;								// �A���`�p�XOFF���Z��
	ulong			CarOutGateOpen;									// �Q�[�g�J�����o�ɑ䐔
	ulong			CarOutForce;									// �����o�ɑ䐔
	t_SyuSub		LostSettle;										// �������Z �񐔁^���z
	ulong			ReceiptCallback;								// �̎��؉������
	ulong			ReceiptIssue;									// �̎��ؔ��s����
	ulong			WarrantIssue;									// �a��ؔ��s����
	t_SyuSub_AllSystem2	AllSystem;									// �S���u
	ulong			CarInTotal;										// �����ɑ䐔
	ulong			CarOutTotal;									// ���o�ɑ䐔
	ulong			CarIn1;											// ����1���ɑ䐔
	ulong			CarOut1;										// �o��1�o�ɑ䐔
	ulong			CarIn2;											// ����2���ɑ䐔
	ulong			CarOut2;										// �o��2�o�ɑ䐔
	ulong			CarIn3;											// ����3���ɑ䐔
	ulong			CarOut3;										// �o��3�o�ɑ䐔
	ulong			MiyoCount;										// ��������
	ulong			MiroMoney;										// �������z
	ulong			LagExtensionCnt;								// ���O�^�C��������
	ulong			SaleParkingNo;									// ����撓�ԏꇂ(0�`999999   �������p�i�W���ł͊�{���ԏꇂ���Z�b�g�j
	ulong			FurikaeCnt;										// �U�։�
	ulong			FurikaeTotal;									// �U�֊z
	ulong			RemoteCnt;										// ���u���Z��
	ulong			RemoteTotal;									// ���u���Z���z
	ulong			Total;											// ���ɑ������z
	ulong			NoteTotal;										// �������ɑ������z
	ulong			CoinTotal;										// �R�C�����ɑ������z
	t_SyuSub_Coin2	Coin[4];										// �R�C��1�`4
	t_SyuSub_Note2	Note[4];										// ����1�`4
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//} DATA_KIND_42_r13;
} DATA_KIND_42;
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/11/04 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�

/*--------------------------------------------------------------------------*/
/*	�Z���^�[Phase2������ʖ��W�v�ް�(�ް����43)							*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	t_SyuSub_RyokinKind	Kind[50];									// ���01�`50
} DATA_KIND_43;

/*--------------------------------------------------------------------------*/
/*	�Z���^�[Phase2�����W�v�ް�(�ް����45)									*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;										// ���ԏ�No.
	ushort			Kind;											// �������
	ushort			Group;											// �����敪
	ulong			Num;											// ������
	ulong			Callback;										// �������
	ulong			Amount;											// �����z
	ulong			Info;											// �������
} t_SyuSub_Discount2;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	t_SyuSub_Discount2	Discount[500];								// ���� 001�`500
} DATA_KIND_45;

/*--------------------------------------------------------------------------*/
/*	�Z���^�[Phase2����W�v�ް�(�ް����46)									*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;										// ���ԏ�No.
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	ushort			Kind;											// ���
	uchar			Mode;											// �����敪
	uchar			Kind;											// ���
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
	ulong			Num;											// ��
	ulong			Callback;										// �������
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	t_SyuSub		Update;											// �X�V
	t_SyuSub2		Update;											// �X�V
	t_SyuSub2		Sale;											// �̔�
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
} t_SyuSub_Pass2_Pay;

// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
typedef struct{
	uchar			syu;											// ���
	ulong			num;											// ��
	ulong			uri;											// ������z
} t_Teiki_Option;
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
	t_Teiki_Option	Option[6];										// �I�v�V�������
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
	t_SyuSub_Pass2_Pay	Pass[100];									// ����� 001�`100
} DATA_KIND_46;

/*--------------------------------------------------------------------------*/
/*	�Z���^�[Phase2�W�v�I���ʒm�ް�(�ް����53)								*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
} DATA_KIND_53;

/*--------------------------------------------------------------------------*/
/*	�߽ܰ���ް�(�ް����84/212)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			PassWord[4];							// �߽ܰ��
	uchar			PassWord2[4];							// �߽ܰ��2
	uchar			Reserve[4];								// �\��
} DATA_KIND_84_DATA;
typedef	struct {
	DATA_BASIC			DataBasic;							// ��{�ް�
	DATA_KIND_84_DATA	Data;								// �߽ܰ���ް�
} DATA_KIND_84;

typedef DATA_KIND_84	DATA_KIND_212;

/*--------------------------------------------------------------------------*/
/*	�W��ID��ԃf�[�^�A�W��ID�⍇���f�[�^�A�W���ێ�f�[�^(�ް����254/255/151/108)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			Kubun;									// �����敪 (���g�p)
	ulong			ParkingNo;								// ���ԏꇂ
	ushort			KakariNo;								// �W��No.
	uchar			Status;									// �X�e�[�^�X�A�\��
} DATA_KIND_254;

typedef DATA_KIND_254	DATA_KIND_255;
typedef DATA_KIND_254	DATA_KIND_151;
typedef DATA_KIND_254	DATA_KIND_108;

/*--------------------------------------------------------------------------*/
/*	���ʓ��ݒ�f�[�^(�ް����83/211)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			sta_Mont;								// �J�n��
	ushort			sta_Date;								// �J�n��
	ushort			end_Mont;								// �I����
	ushort			end_Date;								// �I����
} t_splday_kikan;

typedef	struct {
	ushort			mont;									// ��
	ushort			day;									// ��
} t_splday_date;

typedef	struct {
	ushort			mont;									// ��
	ushort			week;									// �T
	ushort			yobi;									// �j��
} t_splday_yobi;

typedef	struct {
	ushort			year;									// �N
	ushort			mont;									// ��
	ushort			day;									// ��
} t_splday_year;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			rsv1;									// �\���P
	ushort			rsv2;									// �\���Q
	t_splday_kikan	kikan[3];								// ���ʊ��ԂP�`�R
	t_splday_date	date[31];								// ���ʓ��P�`�R�P
	ushort			Shift[31];								// �V�t�g�P�`�R�P
	t_splday_yobi	yobi[12];								// ���ʗj���P�`�P�Q
	ushort			spl56;									// 5�^6����ʓ��Ƃ���
	t_splday_year	year[6];								// ���ʔN�����P�`�U
} DATA_KIND_83;

typedef DATA_KIND_83	DATA_KIND_211;

#define		NTNET_SPLTRM_START		 1		// ���ʊ��Ԃ̊J�n�ʒu
#define		NTNET_SPLTRM_END		 6		// ���ʊ��Ԃ̏I���ʒu
#define		NTNET_SPLDAY_START		 9		// ���ʓ��̊J�n�ʒu
#define		NTNET_SPLDAY_END		39		// ���ʓ��̏I���ʒu
#define		NTNET_HMON_START		42		// ʯ�߰���ް�̊J�n�ʒu
#define		NTNET_HMON_END			47		// ʯ�߰���ް�̏I���ʒu
#define		NTNET_5_6				49		// �u5��6������ʓ��Ƃ���v�̈ʒu
#define		NTNET_SPYEAR_START		50		// ���ʔN�����̊J�n�ʒu
#define		NTNET_SPYEAR_END		55		// ���ʔN�����̏I���ʒu
#define		NTNET_SPYOBI_START		56		// ���ʗj���̊J�n�ʒu
#define		NTNET_SPYOBI_END		67		// ���ʗj���̏I���ʒu

/*--------------------------------------------------------------------------*/
/*	�ݒ��ް��v��(�ް����78,90)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ulong			SMachineNo;								// ���M��[���@�B��
	ulong			ModelCode;								// �ݒ�f�[�^�̋@��R�[�h
	ushort			PrmDiv;									// ���Ұ��敪
	ushort			Segment;								// �ݒ辸����
	ushort			TopAddr;								// �J�n���ڽ
	ushort			DataCount;								// �ݒ��ް���
	uchar			Reserve[72];							// �\��
} DATA_KIND_78;

/*--------------------------------------------------------------------------*/
/*	�ݒ��ް����M�v��(�ް����114)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ulong			SMachineNo;								// ���M��[���@�B��
	SETUP_NTNETDATA	Req;									// �ݒ��ް�
	uchar			Reserve[60];							// �\��
} DATA_KIND_114;

// ���p�[�N��p�t�H�[�}�b�g�Ƃ���
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			reqSend;			// ���M�v���F0(�Ȃ��j�A1�i����j�A2�i�L�����Z���j
	uchar			reqExec;			// ���s�v���F0(�Ȃ��j�A1�i����j
	uchar			reserve[7];			// �\��
	uchar			reserve2[60];		// �\��
} RP_DATA_KIND_114;

/*--------------------------------------------------------------------------*/
/*	�ݒ��ް����M����(�ް����115)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ulong			SMachineNo;								// ���M���[���@�B��
	uchar			ProcMode;								// �����敪
	uchar			Reserve[8];								// �\��1�`8
	uchar			Reserve2[60];							// �\��
} DATA_KIND_115;

// ���p�[�N��p�t�H�[�}�b�g�Ƃ���
typedef	DATA_KIND_115	RP_DATA_KIND_115;

/*--------------------------------------------------------------------------*/
/*	�R�C���E�������ɏW�v�f�[�^�i�f�[�^���130/131/132/133)					*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	short			Money;									// ���z
	long			Mai;									// �ۗL����
} MONEY_DATA2;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�f�[�^
	ushort			Type;									// �W�v�^�C�v
	ushort			KakariNo;								// �W��No.
	ulong			SeqNo;									// �W�v�ǔ�
	ulong			StartSeqNo;								// �J�n�ǔ�
	ulong			EndSeqNo;								// �I���ǔ�
	union {
		MONEY_DATA2		Coin[8];							// �R�C�����Ƀf�[�^
		MONEY_DATA2		Note[8];							// �������Ƀf�[�^
	} Sf;
	ulong			SfTotal;								// ���ɑ������z
} DATA_KIND_130;

typedef DATA_KIND_130	DATA_KIND_132;

/*--------------------------------------------------------------------------*/
/*	����J�E���g�f�[�^�i�f�[�^���228)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�f�[�^
	ulong			MachineNo;								// ���M���[���@�B��
	ulong			Count[100];								// ����J�E���g
} DATA_KIND_228;

/*--------------------------------------------------------------------------*/
/*	���ԑ䐔�Ǘ��f�[�^(�ް����234/235)										*/
/*--------------------------------------------------------------------------*/
typedef struct {
	long			mode;									// ���������Ӱ��
	long			car_cnt;								// ���ݑ䐔
	long			ful_cnt;								// ���ԑ䐔
	long			kai_cnt;								// ���ԉ����䐔
} PARK_NUM_CTRL_Sub;

typedef	struct {
	ulong			CurNum;									// ���ݒ��ԑ䐔
	ulong			NoFullNum;								// ���ԉ����䐔
	ulong			FullNum;								// ���ԑ䐔
} PARK_NUM_CTRL;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			GroupNo;								// ��ٰ�ߔԍ�
	ulong			Reserve1;								// �\���P
	ulong			State;									// �p�r�ʒ��ԑ䐔�ݒ�
	PARK_NUM_CTRL	Data[3];								// ���ԑ䐔�Ǘ��ް�01�`03
	ulong			Reserve[27];							// �\��
} DATA_KIND_234;

typedef DATA_KIND_234	DATA_KIND_235;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			GroupNo;								// ��ٰ�ߔԍ�
	ulong			CurNum;									// ���ݒ��ԑ䐔
	ulong			TkCurNum[15];							// ������1�`15���ݒ��ԑ䐔
	ulong			RyCurNum[20];							// �������1�`20���ݒ��ԑ䐔
	ulong			State;									// �p�r�ʒ��ԑ䐔�ݒ�
	ulong			Full[3];								// ����1�`3������
	ulong			EmptyNo1;								// ��ԑ䐔�P
	ulong			FullNo1;								// ���ԑ䐔�P
	ulong			EmptyNo2;								// ��ԑ䐔�Q
	ulong			FullNo2;								// ���ԑ䐔�Q
	ulong			EmptyNo3;								// ��ԑ䐔�R
	ulong			FullNo3;								// ���ԑ䐔�R
	ulong			Reserve[4];								// �\��
} DATA_KIND_236;

/*--------------------------------------------------------------------------*/
/*	���u������ð��ð���(�ް����92)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			ProcMode;								// �����敪
	PAS_TBL			PassTable[PAS_MAX];						// ������ð��ð���
} DATA_KIND_92;

/*--------------------------------------------------------------------------*/
/*	�Ǘ��f�[�^�v��NG(�ް����105)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ulong			TermNo;									// ���M�������No
	ushort			Result;									// ����
} DATA_KIND_105;

/*--------------------------------------------------------------------------*/
/*	���u�����ް�(�ް����240)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			GroupNo;								// �O���[�vNo
	uchar			ControlData[12];						// �e�퐧��
} DATA_KIND_240;

/*--------------------------------------------------------------------------*/
/*	���u�ް��v��(�ް����243)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			GroupNo;								// �O���[�vNo
	uchar			ControlData[20];						// �e��v��
} DATA_KIND_243;

/*--------------------------------------------------------------------------*/
/*	���u�f�[�^�v������NG(�ް����244)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			GroupNo;								// �O���[�vNo
	uchar			ControlData[20];						// �e��v��
	ushort			Result;									// ����
	ulong			MachineNo[32];							// ��Q�[���̃^�[�~�i��No01�`32�@�BNo
} DATA_KIND_244;

/*--------------------------------------------------------------------------*/
/*	�S�Ԏ����f�[�^(�ް����245)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			GroupNo;								// �O���[�vNo
	LOCK_STATE		LockState[OLD_LOCK_MAX];				// �Ԏ����
} DATA_KIND_245;

/*--------------------------------------------------------------------------*/
/*	���u�ŗL�d���p��{�ް�2													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			SystemID;										// ����ID
	uchar			DataKind;										// �ް����
	uchar			DataKeep;										// �ް��ێ��׸�
} DATA_BASIC_R;

/*--------------------------------------------------------------------------*/
/*	���uIBK�����ް�(ID60/80)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC_R	DataBasicR;								// ��{�ް�
	uchar			Data[18];								// �v���ް�
} DATA_KIND_60_R;

/*--------------------------------------------------------------------------*/
/*	���M�E�ر�v���ް�(ID61�E62/81�E82)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC_R	DataBasicR;								// ��{�ް�
	uchar			Data[20];								// �v���ް�
} DATA_KIND_61_R;

/*--------------------------------------------------------------------------*/
/*	ð��ٌ����ް�(ID63/83)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC_R	DataBasicR;								// ��{�ް�
	ushort			Data[19];								// ð��ٌ������\��
} DATA_KIND_83_R;

/*--------------------------------------------------------------------------*/
/*	�ʐM�����v���^�����ް�(ID100/101)										*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;								// ���ԏꇂ
	ushort			ModelCode;								// �@����
	ulong			SMachineNo;								// ���M����
	uchar			SerialNo[6];							// �[���ره�
	uchar			Year;									// �N
	uchar			Mon;									// ��
	uchar			Day;									// ��
	uchar			Hour;									// ��
	uchar			Min;									// ��
	uchar			Sec;									// �b
	uchar			ChkNo;									// �ʐM����No.
} DATA_KIND_100R_sub;

typedef	struct {
	DATA_BASIC_R		DataBasicR;							// ��{�ް�
	uchar				SeqNo;								// ���ݼ�ه�
	DATA_KIND_100R_sub	sub;								// ���ʗp����ް�
	uchar				Data[26];							// �v���F�v�����(1byte)
															//       ��ѱ��(1byte)
															//       �\��(24byte)
															// ���ʁF���ʺ���(1byte)
															//       �\��(25byte)
} DATA_KIND_100_R;

/*--------------------------------------------------------------------------*/
/*	�j�A�t���ʒm�f�[�^(ID90)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC_R	DataBasicR;								// ��{�ް�
	uchar			Data[20];								// �j�A�t���ʒm(13byte)
															//       �\��(7byte)
} DATA_KIND_90_R;

typedef	struct {
	uchar		card_use;				// ���p�E�s��
	ushort		Base_Amount;
	ushort		Base_Point;
	ushort		Time_Amount;
	ushort		Time_Point;
	t_TIME_INFO Start;
	t_TIME_INFO End;
	uchar		Reserve2[181];
} DATA_KIND_184_01;

typedef	struct {
	uchar		Reserve2[200];
} DATA_KIND_184_02;

typedef	struct {
	uchar		Id_BusCode[2];
	uchar		Company[4];
	uchar 		MemberNo_S[BIN_KETA_MAX];
	uchar 		MemberNo_E[BIN_KETA_MAX];
	uchar		Field;
	uchar		Partner_Code2[4];				// ��g��R�[�h�Q
	uchar		Ptr2_Chk;						// ��g��R�[�h�Q�������
	uchar		Member_Chk;						// ����ԍ��������
	uchar		Reserve[10];					// �\���i�����g���\��j
} W_CARD_INFO;

typedef	struct {
	uchar		kind;				// �J�[�h���
	uchar		kubun;				// ����w��
	W_CARD_INFO	W_Card;				// �J�[�h�f�[�^
	uchar		Reserve2[143];
} DATA_KIND_184_03;

typedef	struct {
	uchar		kind;				// �J�[�h���
	uchar		BlockNo;
	uchar		Reserve2[198];
} DATA_KIND_184_04;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			ReqKind;
	ushort			Reserve;
	union{
		DATA_KIND_184_01	Data01;
		DATA_KIND_184_02	Data02;
		DATA_KIND_184_03	Data03;
		DATA_KIND_184_04	Data04;
	}RcvData;
} DATA_KIND_184;


typedef	struct {
	uchar		Reserve[200];
} DATA_KIND_185_31;

typedef	struct {
	uchar		card_use;				// ���p�E�s��
	ushort		Base_Amount;
	ushort		Base_Point;
	ushort		Time_Amount;
	ushort		Time_Point;
	t_TIME_INFO Start;
	t_TIME_INFO End;
	uchar		Reserve2[181];
} DATA_KIND_185_32;

typedef	struct {
	uchar		kind;				// �J�[�h���
	uchar		Reserve[199];
} DATA_KIND_185_33;

typedef	struct {
	uchar		kind;				// �J�[�h���
	uchar		BlockNo;
	W_CARD_INFO	W_Card[BLOCK_MAX];
	uchar		Reserve2[88];
} DATA_KIND_185_34;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	ushort			ReqKind;
	ushort			Result;
	union{
		DATA_KIND_185_31	Data01;
		DATA_KIND_185_32	Data02;
		DATA_KIND_185_33	Data03;
		DATA_KIND_185_34	Data04;
	}SndData;
} DATA_KIND_185;

typedef	struct {
	ulong		LockNo;
	t_TIME_INFO	InCarTime;
	uchar		Reserve;
} PARKING_INFO;

typedef	struct {
	DATA_BASIC		DataBasic;					// ��{�ް�
	ushort			Kind;
	ushort			ParkingTerm;
	ushort			ParkingNum;
	PARKING_INFO	ParkInfo[8];
	uchar			Reserve[10];
} DATA_KIND_14;

/*--------------------------------------------------------------------------*/
/*	�ڼޯĖ⍇���ް�(ID148/149)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	uchar			DataKind;								// �d�����
	uchar			DataOiban;								// �����ǔ�
	ulong			SaleOiban;								// ���Z�ǔ�
	ushort			Year;									// ���Z�N
	uchar			Mon;									//     ��
	uchar			Day;									//     ��
	uchar			Hour;									//     ��
	uchar			Min;									//     ��
	ushort			Sec;									//     �b
	ulong			Slip_No;								// �`�[�ԍ�
	ulong			sale_ryo;								// ������z
	ushort			pay_kind;								// �x�����@
	ushort			share_count;							// ������
	uchar			yobi[12];								// �\��
	uchar			jis2_data[69];							// JIS2�����ް�
	uchar			jis1_data[37];							// JIS1�����ް�
	uchar			card_no[16];							// �@�l�J�[�h�ԍ�
	uchar			use_limit[4];							// �L������
	uchar			Type;									// �@�\���
	uchar			Partner_Code2[4];						// ��g��R�[�h�Q
	uchar			Ptr2_Chk;								// ��g��R�[�h�Q�������
	uchar			Reserve[3];								// �\��(�����g���\��)
	uchar			yobi2[4];								// �\��
} DATA_KIND_148;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	uchar			DataKind;								// �d�����
	uchar			DataOiban;								// �����ǔ�
	ulong			SaleOiban;								// ���Z�ǔ�
	ushort			Year;									// ���Z�N
	uchar			Mon;									//     ��
	uchar			Day;									//     ��
	uchar			Hour;									//     ��
	uchar			Min;									//     ��
	ushort			Sec;									//     �b
	ulong			slip_no;								// �`�[�ԍ�
	ulong			sale_ryo;								// ������z
	ushort			pay_kind;								// �x�����@
	ushort			share_count;							// ������
	uchar			inq_result1[2];							// �Ɖ�ʇ@
	uchar			inq_result2[2];							// �Ɖ�ʇA
	ulong			app_no;									// ���F�ԍ�
	uchar			card_kind[2];							// ���ގ��
	uchar			company_code[4];						// ��к���
	uchar			company_name[10];						// ��Ж�
	uchar			member_code[20];						// ����ԍ�
	uchar			member_name[10];						// �����
	uchar			card_use_limmit_y;						// ���ޗL�������i�N�j	
	uchar			card_use_limmit_m;						// ���ޗL�������i���j
	uchar			Tcard_use;								// �s�J�[�h���p
	uchar			yobi2[86];								// �\��
} DATA_KIND_149;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	uchar			ReqKind;								// �v�����
	uchar			DL_Year;								// �_�E�����[�h�����i�N�j
	uchar			DL_Mon;									// �_�E�����[�h�����i���j
	uchar			DL_Day;									// �_�E�����[�h�����i���j
	uchar			DL_Hour;								// �_�E�����[�h�����i���j
	uchar			DL_Min;									// �_�E�����[�h�����i���j
	uchar			SW_Year;								// �v���O�����X�V�����i�N�j
	uchar			SW_Mon;									// �v���O�����X�V�����i���j
	uchar			SW_Day;									// �v���O�����X�V�����i���j
	uchar			SW_Hour;								// �v���O�����X�V�����i���j
	uchar			SW_Min;									// �v���O�����X�V�����i���j
	uchar			Prog_Info[15];							// ���C���v���O�������
	uchar			Yobi[5];								// �\���G���A
} DATA_KIND_188;

typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	uchar			ErrCode;								// �G���[�R�[�h
	uchar			ReqKind;								// �v�����
	uchar			yobi[5];								// �\��
} DATA_KIND_189;


typedef	struct {
	DATA_BASIC		DataBasic;								// ��{�ް�
	uchar			Series_Name[12];						// �V���[�Y����
	uchar			Version[9];								// ���Z�@�@�\�o�[�W����
	uchar			MachineNo;								// ���Z�@�@�BNo
	ulong			DeviceID1;								// ���Z�@�f�o�C�XID1�i��\�j
	ulong			DeviceID2;								// ���Z�@�f�o�C�XID2
	ulong			DeviceID3;								// ���Z�@�f�o�C�XID3
	uchar			T_Card;									// T�J�[�h
	uchar			Credit;									// �N���W�b�g�J�[�h
	uchar			Web_Money;								// �d�q�}�l�[
	uchar			Corporate;								// �@�l�J�[�h
	uchar			Reserve[30];							// �\���i�����g���\��j
} DATA_KIND_190;




/*--------------------------------------------------------------------------*/
/*	���A���^�C�����f�[�^(�ް����153)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {	// ���Z�@���K���
	ushort			ModelCode;		// �@��R�[�h(001�`999) 0=���Ȃ�
	ushort			MachineNo;		// ���M���[���@�B��(01�`99)  0=���Ȃ�
	ushort			Result;			// �v������(0������A1���ʐM��Q���A2���^�C���A�E�g)
	ulong			Uriage;			// ������z					0�`999999
	ulong			Cash;			// ����������z				0�`999999
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	uchar			Date_Uriage_use;// ���t�ؑ֎���� �g�p�L�� 0�����g�p�A1���g�p
	uchar			Reserve1;		// �\��(�T�C�Y�����p)
	ulong			Date_Uriage;	// ���t�ؑ֎���� ������z      0�`999999
	ulong			Date_Cash;		// ���t�ؑ֎���� ����������z  0�`999999
	uchar			Date_hour;		// ���t�ؑ֎���� �ؑ֎� 0�`23
	uchar			Date_min;		// ���t�ؑ֎���� �ؑ֕� 0�`12
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	ulong			CoinTotal;		// �R�C�����ɑ������z			0�`999999
	ulong			NoteTotal;		// �������ɑ������z			0�`999999
	uchar			CoinReceive;	// �R�C�����Ɏ��[��(%)
	uchar			Reserve;		// �\��(�T�C�Y�����p)
	ushort			NoteReceive;	// �������Ɏ��[����
	uchar			KinsenKanriFlag;// ���K�Ǘ�����/�Ȃ�			0������/1���Ȃ�
	uchar			TuriStatus;		// �ޑK�؂���	0���ޑK����/1���ޑK�؂�
									// 	1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
									// 	5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~
	ushort			TuriMai_10;		// �ޑK���	10�~�ۗL����	0�`9999
	ushort			TuriMai_50;		// 			50�~�ۗL����	
	ushort			TuriMai_100;	// 			100�~�ۗL����	
	ushort			TuriMai_500;	// 			500�~�ۗL����	
	ushort			Yochiku1_shu;	// �\�~1(SUB��)	������z	0���񓋍ځ@10/50/100/500
	ushort			Yochiku1_mai;	// 				�ۗL����	0�`9999
	ushort			Yochiku2_shu;	// �\�~2		������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
	ushort			Yochiku2_mai;	// 				�ۗL����	0�`9999
	ushort			Yochiku3_shu;	// �\�~3		������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
	ushort			Yochiku3_mai;	// 				�ۗL����	0�`9999
	ushort			Yochiku4_shu;	// �\�~4		������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
	ushort			Yochiku4_mai;	// 				�ۗL����	0�`9999
	ushort			Yochiku5_shu;	// �\�~5		������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
	ushort			Yochiku5_mai;	// 				�ۗL����	0�`9999
} t_MacMoneyInfo;
typedef	struct {
	DATA_BASIC		DataBasic;		// ��{�ް�
	ushort			FmtRev;			// �t�H�[�}�b�gRev.��
	PARKCAR_DATA11	ParkData;		// ���ԑ䐔
	t_MacMoneyInfo	MoneyInfo[32];	// ���Z�@���K���
} DATA_KIND_153;	// ���A���^�C�����f�[�^
/*--------------------------------------------------------------------------*/
/*	���Z����ް�(�ް����152)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar		KyugyoStatus;		//	�@����_�c�x��		0�F�c��/1�F�x��
	uchar		Lc1Status;			//	�@����_�k�b1���	0�F�n�e�e/1�F�n�m
	uchar		Lc2Status;			//	�@����_�k�b2���	0�F�n�e�e/1�F�n�m
	uchar		Lc3Status;			//	�@����_�k�b3���	0�F�n�e�e/1�F�n�m
	uchar		GateStatus;			//	�@����_�Q�[�g���	0�F��/1�F�J
	uchar		NgCardRead;			//	NG�J�[�h�ǎ���e	1	30		0���Ȃ�, 1�`255��NT-NET�̃A���[�����02�̃R�[�h���e�Ɠ���
} t_KikiStatus;

typedef	struct {
	uchar		ErrCoinmech;		//	�R�C�����b�N	�G���[���03�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	uchar		ErrNoteReader;		//	�������[�_�[	�G���[���04�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	uchar		ErrGikiReader;		//	���C���[�_�[	�G���[���01�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	uchar		ErrNoteHarai;		//	�������o�@		�G���[���05�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	uchar		ErrCoinJyunkan;		//	�R�C���z��		�G���[���06�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	uchar		ErrNoteJyunkan;		//	�����z��		�G���[���14�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
} t_Erroring;		// �������G���[
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
//typedef	struct {
//	DATA_BASIC		DataBasic;			// ��{�ް�
//	ushort			FmtRev;				// �t�H�[�}�b�gRev.��
//	t_KikiStatus	Kiki;				// �@����
//	t_Erroring		Err;				// �������G���[
//	ulong			PayCount;			// ���Z�ǔ�(0�`99999)
//	uchar			PayMethod;			// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
//	uchar			PayClass;			// �����敪(0�����Z/1���Đ��Z/2�����Z���~/3���Đ��Z���~, 8=���Z�O, 9=���Z�r��
//	uchar			PayMode;			// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
//										//				20��Mifare�v���y�C�h���Z
//	uchar			CMachineNo;			// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
//	ushort			FlapArea;			// �t���b�v�V�X�e��	���		0�`99
//	ushort			FlapParkNo;			// 					�Ԏ��ԍ�	0�`9999
//	ushort			KakariNo;			// �W����	0�`9999
//	uchar			OutKind;			// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
//										//				10���Q�[�g�J��/20��Mifare�v���y�C�h���Z�o��
//										//				97�����b�N�J�E�t���b�v�㏸�O�����Z�o��/98�����O�^�C�����o��
//										//				99���T�[�r�X�^�C�����o��
//	uchar			CountSet;			// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
//										// (�f�[�^��ʂ��u22�F���O���Z�v�̎��́u1=���Ȃ��v�ƂȂ�)
//	t_InPrevYMDHMS	InPrev_ymdhms;		// ����/�O�񐸎Z_YMDHMS
//	uchar			ReceiptIssue;		// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
//	uchar			Syubet;				// �������			1�`
//	ulong			Price;				// ���ԗ���			0�`
//	ulong			CashPrice;			// ��������			0�`
//	ulong			InPrice;			// �������z			0�`
//	ushort			ChgPrice;			// �ޑK���z			0�`9999
//	t_MoneyInOut	MoneyInOut;			// ���K���(����������/���o������)
//	ushort			HaraiModoshiFusoku;	// ���ߕs���z	0�`9999
//	uchar			Reserve1;			// �\��(�T�C�Y�����p)	0
//	uchar			AntiPassCheck;		// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�N�n�e�e/2�������n�e�e
//										//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�N�n�e�e�v�Œ�
//	ulong			ParkNoInPass;		// ������@���ԏꇂ	0�`999999
//	t_MediaInfo		Media[2];			// ���Z�}�̏��1, 2
//	ushort			CardKind;			// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//										//					3=Edy�A4=PiTaPa�A5=WAON�@�@90�ȍ~=�ʑΉ�
//	uchar			settlement[64];		// ���Ϗ��			(��3)�@�Í����Ώ�
//	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];	// ����
//} DATA_KIND_152;	// ���Z����ް�
typedef	struct {
	DATA_BASIC		DataBasic;			// ��{�ް�
	ushort			FmtRev;				// �t�H�[�}�b�gRev.��
	t_KikiStatus	Kiki;				// �@����
	t_Erroring		Err;				// �������G���[
	ulong			PayCount;			// ���Z�ǔ�(0�`99999)
	uchar			PayMethod;			// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
	uchar			PayClass;			// �����敪(0�����Z/1���Đ��Z/2�����Z���~/3���Đ��Z���~, 8=���Z�O, 9=���Z�r��
	uchar			PayMode;			// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
										//				20��Mifare�v���y�C�h���Z
	uchar			CMachineNo;			// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
	ushort			FlapArea;			// �t���b�v�V�X�e��	���		0�`99
	ushort			FlapParkNo;			// 					�Ԏ��ԍ�	0�`9999
	ushort			KakariNo;			// �W����	0�`9999
	uchar			OutKind;			// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
										//				10���Q�[�g�J��/20��Mifare�v���y�C�h���Z�o��
										//				97�����b�N�J�E�t���b�v�㏸�O�����Z�o��/98�����O�^�C�����o��
										//				99���T�[�r�X�^�C�����o��
	uchar			CountSet;			// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
										// (�f�[�^��ʂ��u22�F���O���Z�v�̎��́u1=���Ȃ��v�ƂȂ�)
	t_InPrevYMDHMS	InPrev_ymdhms;		// ����/�O�񐸎Z_YMDHMS
	uchar			ReceiptIssue;		// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
	uchar			Syubet;				// �������			1�`
	ulong			Price;				// ���ԗ���			0�`
	ulong			CashPrice;			// ��������			0�`
	ulong			InPrice;			// �������z			0�`
	ushort			ChgPrice;			// �ޑK���z			0�`9999
	t_MoneyInOut	MoneyInOut;			// ���K���(����������/���o������)
	ushort			HaraiModoshiFusoku;	// ���ߕs���z	0�`9999
	ushort			CardFusokuType;		// ���o�s���}�̎��(�����ȊO)
	ushort			CardFusokuTotal;	// ���o�s���z(�����ȊO)
	uchar			Reserve1;			// �\��(�T�C�Y�����p)	0
	uchar			AntiPassCheck;		// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�N�n�e�e/2�������n�e�e
										//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�N�n�e�e�v�Œ�
	ulong			ParkNoInPass;		// ������@���ԏꇂ	0�`999999
	t_MediaInfo2	Media[2];			// ���Z�}�̏��1, 2
	ushort			CardKind;			// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
										//					3=Edy�A4=PiTaPa�A5=WAON�@�@90�ȍ~=�ʑΉ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	uchar			PayTerminalClass;	// ���ϒ[���敪
	uchar			Transactiontatus;	// ����X�e�[�^�X
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	uchar			settlement[80];		// ���Ϗ��			(��3)�@�Í����Ώ�
// �s��C��(S) K.Onodera 2016/11/24 #1580 ���Z���f�[�^�̓��e��NT-NET�t�H�[�}�b�g�d�l���ƈ�v���Ă��Ȃ�
//	ulong			TotalSale;			// ���v���z(����)
//	ulong			DeleteSeq;			// �폜�f�[�^�ǔ�(1�`FFFFFFFFH�@�폜���鐸�Z�f�[�^�ɃZ�b�g�����Z���^�[�ǔ�)
//	ulong			Reserve2;			// �\��
//	ulong			Reserve3;			// �\��
// �s��C��(E) K.Onodera 2016/11/24 #1580 ���Z���f�[�^�̓��e��NT-NET�t�H�[�}�b�g�d�l���ƈ�v���Ă��Ȃ�
	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];	// ����
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//} DATA_KIND_152_r12;	// ���Z����ް�
} DATA_KIND_152_rXX;	// ���Z����ް�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
#define	NTNET_SYNCHRO_GET	0	// ���������Q��
#define	NTNET_SYNCHRO_SET	1	// ���������ݒ�
/*--------------------------------------------------------------------------*/
/*	���������ݒ�v��(�ް����156)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {						// 
	DATA_BASIC		DataBasic;			// ��{�ް�
	ushort			FmtRev;				// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;			// ���M��[���@�B��
	uchar			Time_synchro_Req;	// ���������ݒ�v��(0���v���Ȃ��^1���v������)
	uchar			synchro_hour;		// ��������(��)
	uchar			synchro_minu;		// ��������(��)
	uchar			Reserve;			// �\��
	uchar			Reserve2[12];		// �\��2
} DATA_KIND_156;
/*--------------------------------------------------------------------------*/
/*	���������ݒ�v�� ����(�ް����157)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {						// 
	DATA_BASIC		DataBasic;			// ��{�ް�
	ushort			FmtRev;				// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;			// ���M��[���@�B��
	uchar			Time_synchro_Req;	// ���������ݒ�v��(0���v���Ȃ��^1���v������)
	uchar			synchro_hour;		// ��������(��)
	uchar			synchro_minu;		// ��������(��)
	uchar			Reserve;			// �\��
	uchar			Reserve2[12];		// �\��2
	ushort			Result;				// ����(9=�p�����[�^NG)
} DATA_KIND_157;


/*--------------------------------------------------------------------------*/
/*	�ڼޯ�:����M�d�����ʕ�(�ް����136,137,138,139����)					*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ushort			DestInfo;										// ���M����
	uchar			DataIdCode1;									// �d�����ʃR�[�h�@
	uchar			DataIdCode2;									// �d�����ʃR�[�h�A
} CREDIT_COMMON;

/*==========================================================================*/
/*	�ڼޯ�:���M�d��															*/
/*==========================================================================*/
/*--------------------------------------------------------------------------*/
/*	�J�Ǻ����:��ײ�ý�														*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	date_time_rec2	Proc;											// �����N���������b
} DATA_KIND_136_01;

/*--------------------------------------------------------------------------*/
/*	�ڼޯė^�M�⍇���ް�													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	date_time_rec2	Pay;											// ���Z�N���������b
	ulong			SlipNo;											// �[�������ʔԁi�`�[���j
	ulong			Amount;											// ������z
	uchar			AppNo[6];										// ���F�ԍ�
	struct {
		uchar		JIS2Data[69];									// JIS2�����ް�
		uchar		JIS1Data[37];									// JIS1�����ް�
		ushort		PayMethod;										// �x�����@
		ulong		DivCount;										// ������
	} Crypt;
} DATA_KIND_136_03;

/*--------------------------------------------------------------------------*/
/*	�ڼޯĔ���˗��ް�														*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			MediaCardNo[30];								// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			MediaCardInfo[16];								// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			reserve[2];										// �\��
} t_MediaInfoCre;	// ���Ɂ^���Z�@�}�̏��
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	date_time_rec2	Pay;											// ���Z�N���������b
	ulong			SlipNo;											// �[�������ʔԁi�`�[���j
	ulong			Amount;											// ������z
	uchar			AppNo[6];										// ���F�ԍ�
	uchar			ShopAccountNo[20];								// �����X����ԍ�
	ushort			PayMethod;										// �x�����@
	ulong			DivCount;										// ������
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
	t_MediaInfoCre	Media;											// ���Z�}�̏��1
} DATA_KIND_136_05;

/*--------------------------------------------------------------------------*/
/*	ýĺ����																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	uchar			TestKind;										// ýĎ��
} DATA_KIND_136_07;

/*--------------------------------------------------------------------------*/
/*	�ڼޯĕԕi�⍇���ް�													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	date_time_rec2	Proc;											// �����N���������b
	ulong			SlipNo;											// �[�������ʔԁi�`�[���j
	uchar			AppNo[6];										// �ԕi�Ώ� ���F�ԍ�
	uchar			ShopAccountNo[20];								// �ԕi�Ώ� �����X����ԍ�
} DATA_KIND_136_09;

/*==========================================================================*/
/*	�ڼޯ�:��M�d��															*/
/*==========================================================================*/
/*--------------------------------------------------------------------------*/
/*	�J�Ǻ���މ���:��ײ�ý�													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	date_time_rec2	Proc;											// �����N���������b
	ushort			Result1;										// �������ʇ@
	ushort			Result2;										// �������ʇA
	ulong			SrcParkingNo;									// �v�������ԏꇂ
	ushort			SrcModelCode;									// �v�����@����
	ulong			SrcMachineNo;									// �v�����[���@�B��
} DATA_KIND_137_02;

/*--------------------------------------------------------------------------*/
/*	�ڼޯė^�M�⍇�������ް�												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	date_time_rec2	Pay;											// ���Z�N���������b
	ulong			SlipNo;											// �[�������ʔԁi�`�[���j
	ulong			Amount;											// ������z
	ushort			PayMethod;										// �x�����@
	ulong			DivCount;										// ������
	ushort			Result1;										// �������ʇ@
	ushort			Result2;										// �������ʇA
	uchar			AppNo[6];										// ���F�ԍ�
	uchar			ShopAccountNo[20];								// �����X����ԍ�
	uchar			KidCode[6];										// KID����
	uchar			CardNo[16];										// �ڼޯĶ��އ�
	uchar			TerminalId[13];									// �[�����ʔԍ�
	uchar			CompanyName[12];								// �ڼޯĶ��މ�Ж�
} DATA_KIND_137_04;

/*--------------------------------------------------------------------------*/
/*	�ڼޯĔ���˗������ް�													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	date_time_rec2	Pay;											// ���Z�N���������b
	ulong			SlipNo;											// �[�������ʔԁi�`�[���j
	ulong			Amount;											// ������z
	uchar			AppNo[6];										// ���F�ԍ�
	uchar			ShopAccountNo[20];								// �����X����ԍ�
	ushort			Result1;										// �������ʇ@
	ushort			Result2;										// �������ʇA
} DATA_KIND_137_06;

/*--------------------------------------------------------------------------*/
/*	ýĺ���މ���															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	uchar			TestKind;										// ýĎ��
	ushort			Result1;										// �������ʇ@
	ushort			Result2;										// �������ʇA
} DATA_KIND_137_08;

/*--------------------------------------------------------------------------*/
/*	�ڼޯĕԕi�⍇�������ް�												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	CREDIT_COMMON	Common;											// �ڼޯċ��ʕ�
	ushort			SeqNo;											// �����ǂ���
	date_time_rec2	Proc;											// �����N���������b
	ulong			SlipNo;											// �[�������ʔԁi�`�[���j
	uchar			AppNo[6];										// �ԕi�Ώ� ���F�ԍ�
	uchar			ShopAccountNo[20];								// �ԕi�Ώ� �����X����ԍ�
	ushort			Result1;										// �������ʇ@
	ushort			Result2;										// �������ʇA
} DATA_KIND_137_0A;

// GG120600(S) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�

// �V�X�e��ID�F40
#define REMOTE_MNT_REQ								111				// ���u�����e�i���X�v��

// GG120600(E) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�

typedef struct {
	ulong			InstNo1;										// ���ߔԍ�1
	ulong			InstNo2;										// ���ߔԍ�2
	uchar			ReqKind;										// �v�����
	uchar			ProcKind;										// �������
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
	uchar			RmonFrom;										// ���ߗv�����i0���Z���^�[/1=�[���j
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
	uchar			DL_Year;										// �_�E�����[�h�����i�N�j
	uchar			DL_Mon;											// �_�E�����[�h�����i���j
	uchar			DL_Day;											// �_�E�����[�h�����i���j
	uchar			DL_Hour;										// �_�E�����[�h�����i���j
	uchar			DL_Min;											// �_�E�����[�h�����i���j
	uchar			SW_Year;										// �v���O�����X�V�����i�N�j
	uchar			SW_Mon;											// �v���O�����X�V�����i���j
	uchar			SW_Day;											// �v���O�����X�V�����i���j
	uchar			SW_Hour;										// �v���O�����X�V�����i���j
	uchar			SW_Min;											// �v���O�����X�V�����i���j
	uchar			Prog_No[8];										// �v���O��������
	uchar			Prog_Ver[2];									// �v���O�����o�[�W����
} t_ProgDlReq;

typedef struct {
	uchar			DestIP[12];										// FTP�T�[�oIP�A�h���X
	uchar			FTPCtrlPort[10];								// FTP����|�[�g
	uchar			FTPUser[20];									// FTP���[�U
	uchar			FTPPasswd[20];									// FTP�p�X���[�h
	uchar			Reserve[2];
} t_FtpInfo;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;										// ���M��^�[�~�i����
	t_ProgDlReq		ProgDlReq;
	t_FtpInfo		FtpInfo;
	uchar			Reserve[7];
} DATA_KIND_111_DL;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;										// ���M���^�[�~�i����
	t_ProgDlReq		ProgDlReq;
	uchar			Reserve[8];
} DATA_KIND_118_DL;

typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;										// ���M���^�[�~�i����
	ulong			InstNo1;										// ���ߔԍ�1
	ulong			InstNo2;										// ���ߔԍ�2
	ushort			FuncNo;											// �[���@�\�ԍ�
	ushort			ProcNo;											// �����ԍ�
	ushort			ProcInfoNo;										// �������ԍ�
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//	uchar			Reserve;
	uchar			RmonFrom;										// ���ߗv����0=�Z���^�[/1=�[��
	uchar			RmonUploadReq;									// �ݒ�A�b�v���[�h�v��
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
	uchar			StatusNo;										// �󋵔ԍ�
	ushort			StatusDetailNo;									// �󋵏ڍהԍ�
	ushort			MonitorInfo;									// �[���Ď����
	uchar			Start_Year1;									// �\��J�n����1�i�N�j
	uchar			Start_Mon1;										// �\��J�n����1�i���j
	uchar			Start_Day1;										// �\��J�n����1�i���j
	uchar			Start_Hour1;									// �\��J�n����1�i���j
	uchar			Start_Min1;										// �\��J�n����1�i���j
	uchar			Start_Year2;									// �\��J�n����2�i�N�j
	uchar			Start_Mon2;										// �\��J�n����2�i���j
	uchar			Start_Day2;										// �\��J�n����2�i���j
	uchar			Start_Hour2;									// �\��J�n����2�i���j
	uchar			Start_Min2;										// �\��J�n����2�i���j
	uchar			Prog_No[8];										// �v���O��������
	uchar			Prog_Ver[2];									// �v���O�����o�[�W����
} DATA_KIND_125_DL;

// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// ============================================================== //
//
//	�[���ԒʐM�p�t�H�[�}�b�g
//
// ============================================================== //

/*--------------------------------------------------------------------------*/
/*	���Z�ް�(�ް����56,57)													*/
/*--------------------------------------------------------------------------*/
// GM849100(S) M.Fujikawa 2025/01/09 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
//#define		NTNET_DIC_MAX2	20										// �������i�f�[�^���56/57�j
#define		NTNET_MNY_MAX	2										// �����^���o���z
#define		NTNET_DIC_MAX2	98										// �������i�f�[�^���56/57�j
// GM849100(E) M.Fujikawa 2025/01/09 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j

typedef struct {
	uchar			no;												// �I�v�V��������ԍ�
	uchar			reserve;										// �\���i�����p�j
	ulong			ryo;											// �I�v�V����������z
} t_opt_price;

typedef struct {
	t_MoneyInOut	MoneyInOut;										// ���K���i�������z�^���ߋ��z�j
// �s��C��(S) K.Onodera 2016/12/20 #1684 �[���ԒʐM�ł̐��Z�f�[�^(ID:57)�Ŕ���撓�ԏꇂ���������ʒu�ɃZ�b�g����Ă��Ȃ�
//	ulong			TotalSale;										// ���v���z(����)
//	ulong			DeleteSeq;										// �폜�f�[�^�ǔ�
//	ulong			SalesParkingNo;									// ����撓�ԏ�No.
	ulong			SalesParkingNo;									// ����撓�ԏ�No.
	ulong			TotalSale;										// ���v���z(����)
	ulong			DeleteSeq;										// �폜�f�[�^�ǔ�
// �s��C��(E) K.Onodera 2016/12/20 #1684 �[���ԒʐM�ł̐��Z�f�[�^(ID:57)�Ŕ���撓�ԏꇂ���������ʒu�ɃZ�b�g����Ă��Ȃ�
	ushort			FusokuCardKind;									// ���o���s���}�̎��(�����ȊO)
	ushort			FusokuCard;										// ���o���s���z(�����ȊO)
// GM849100(S) M.Fujikawa 2025/01/09 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
//	uchar			Reserve1[12];									// �\��1
	ulong			ParkingTime;									// ���Ԏ���
	ushort			MaxFeeFlag;										// �ő�K�p�t���O
	ushort			MaxFeeCount;									// �ő�K�p��
	uchar			MaxFeeSetting;									// �ő嗿���ݒ�L��
	uchar			Reserve1[3];									// �\��1
// GM849100(E) M.Fujikawa 2025/01/09 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
	ulong			PayCount;										// ���Z�ǂ���
	ushort			PayMethod;										// ���Z���@
	ushort			PayClass;										// �����敪
	ushort			PayMode;										// ���ZӰ��
	ulong			LockNo;											// �����
	ushort			CardType;										// ���Ԍ�����
	ushort			CMachineNo;										// ���Ԍ��@�B��
	ulong			CardNo;											// ���Ԍ��ԍ�(�����ǂ���)
	date_time_rec2	OutTime;										// �o�ɔN���������b
	ushort			KakariNo;										// �W����
	ushort			OutKind;										// ���Z�o��
	ushort			CountSet;										// �ݎԶ���
	ushort			Before_pst;										// �O�񁓊�����
	ulong			BeforePwari;									// �O�񁓊������z
	ulong			BeforeTime;										// �O�񊄈����Ԑ�
	ulong			BeforeTwari;									// �O�񎞊Ԋ������z
	ulong			BeforeRwari;									// �O�񗿋��������z
// GM849100(S) M.Fujikawa 2025/01/09 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
//	ushort			ReceiptIssue;									// �̎��ؔ��s�L��
	uchar			SealIssue;										//�V�[�����s�L��
	uchar			ReceiptIssue;									// �̎��ؔ��s�L��
// GM849100(E) M.Fujikawa 2025/01/09 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
	date_time_rec2	InTime;											// ���ɔN���������b
	date_time_rec2	PayTime;										// ���O���Z�N���������b(CPS���Z�����)
	ulong			TaxPrice;										// �ېőΏۊz
	ulong			TotalPrice;										// ���v���z
	ulong			Tax;											// ����Ŋz
	ushort			Syubet;											// �������
	ulong			Price;											// ���ԗ���
	ulong			TotalDiscount;									// �������z
	long			CashPrice;										// ��������
	ulong			InPrice;										// �������z
	ulong			ChgPrice;										// �ޑK���z
	ulong			Fusoku;											// �ޑK���o�s�����z
	ushort			FusokuFlg;										// �ޑK���o�s�������׸�
	ushort			PayObsFlg;										// ���Z����Q�����׸�
	ushort			ChgOverFlg;										// ���ߏ���z���ް�����׸�
	ushort			PassCheck;										// ����߽����
	PASS_DATA		PassData;										// ������ް�
	ulong			PassRenewalPric;								// ������X�V����
	ushort			PassRenewalCondition;							// ������X�V����
	ushort			PassRenewalPeriod;								// ������X�V����
	uchar			UpCount;										// �X�V��
	uchar			ReIssueCount;									// �Ĕ��s��
	t_opt_price		Opt_Price[4];									// �I�v�V�������
	date_time_rec2	BeforePayTime;									// �O�񎖑O���Z�N���������b�i���Z�㌔�Ő��Z���j
	uchar			MatchIP[8];										// ϯ�ݸ�IP���ڽ
	uchar			MatchVTD[8];									// ϯ�ݸ�VTD/�Ԕ�
	uchar			CreditIssue;									// �ڼޯĶ��ތ��ϗL��
	uchar			CredirReserve1;									// �ڼޯĶ��ޗ\��1�i�����p�j
	uchar			CreditCardNo[20];								// �ڼޯĶ��މ����
	ulong			Credit_ryo;										// �ڼޯĶ��ޗ��p���z
	ulong			CreditSlipNo;									// �ڼޯĶ��ޓ`�[�ԍ�
	ulong			CreditAppNo;									// �ڼޯĶ��ޏ��F�ԍ�
	uchar			CreditName[10];									// �ڼޯĶ��މ�Ж�
	uchar			CreditDate[2];									// �ڼޯĶ��ޗL������(�N��)
	ulong			CreditProcessNo;								// �ڼޯĶ��޾�������ǂ���
	uchar			term_id[16];									// �[�����ʔԍ�
	uchar			kid_code[6];									// KID����
	ulong			Reserve2;										// �\��2
// MH364304(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
//	ulong			Reserve3;										// �\��3
//	ulong			Reserve4;										// �\��4
	ulong			RegistNum1;										// �o�^�ԍ�1�i����9�� BIN�j
	ushort			RegistNum2;										// �o�^�ԍ�2�i���4�� BIN�j
	uchar			RegistNum3;										// �o�^�ԍ�3�i�擪1�����j
// MH364304(S) �f�[�^�ۊǃT�[�r�X�Ή��i�K�p�ŗ����Z�b�g����j
	uchar			TaxRate;										// �K�p�ŗ�
// MH364304(E) �f�[�^�ۊǃT�[�r�X�Ή��i�K�p�ŗ����Z�b�g����j
// MH364304(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
// MH364304(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۂ��Z�b�g����j
//	ulong			Reserve5;										// �\��5
	ulong			TaxableDiscount;								// �ېőΏہib0�`b19�j
// MH364304(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۂ��Z�b�g����j
// MH364304(S) �f�[�^�ۊǃT�[�r�X�Ή��i�K�i���������L�����Z�b�g����j
//	ulong			Reserve6;										// �\��6
	ushort			Invoice;										// �K�i���������L��
	ushort			Reserve6;
// MH364304(E) �f�[�^�ۊǃT�[�r�X�Ή��i�K�i���������L�����Z�b�g����j
	ulong			Reserve7;										// �\��7
	ulong			Reserve8;										// �\��8
	t_MediaInfo2	Media[1];										// �o�ɔ}�̎��
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j�iGT-7700:GM747904�Q�l�j
	date_time_rec2	Before_Ts_Time;									// �O��s���v�̎���		�v8byte
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j�iGT-7700:GM747904�Q�l�j
} t_SeisanData_T;

typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			CenterSeqNo;									// �����ǔ�
	t_SeisanData_T	SeisanData;										// ���Z�f�[�^�{��
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j�iGT-7700:GM747904�Q�l�j
//	DISCOUNT_DATA	DiscountData[NTNET_DIC_MAX2];					// �����֘A
	IN_OUT_MONEY	MoneyData[NTNET_MNY_MAX];						// �����E���o���z
	t_SeisanDiscountOld	DiscountData[NTNET_DIC_MAX2];				// �����֘A
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�ύX�j�iGT-7700:GM747904�Q�l�j
} DATA_KIND_56_T;

/*--------------------------------------------------------------------------*/
/*	�W�v��{�ް�(�ް����42)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			CenterSeqNo;									// �����ǔ�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	date_time_rec2	NowTime;										// ����W�v
	date_time_rec2	LastTime;										// �O��W�v
	ulong			SettleNum;										// �����Z��
	ulong			Kakeuri;										// ���|���z
	ulong			Cash;											// ����������z
	ulong			Uriage;											// ������z
	ulong			Tax;											// ������Ŋz
	ulong			Charge;											// �ޑK���ߊz
	ulong			CoinTotalNum;									// �R�C�����ɍ��v��
	ulong			NoteTotalNum;									// �������ɍ��v��
	ulong			CyclicCoinTotalNum;								// �z�b�p�[���v��
	ulong			NoteOutTotalNum;								// �������o�@���v��
	ulong			Uri_Tryo_Gai;									// ������O�z
	ulong			SalesParkingNo;									// ����撓�ԏ�No.
	ulong			Rsv1;											// �\��
	ulong			CarOutWithoutPay;								// ���Z�Ȃ����[�h���o�ɑ䐔
	ulong			SettleNumServiceTime;							// �T�[�r�X�^�C�������Z��
	ulong			CarOutLagTime;									// ���O�^�C�����o�ɉ�
	t_SyuSub		Shortage;										// ���o�s��
	t_SyuSub		Cancel;											// ���Z���~
	t_SyuSub		AutoSettle;										// ���������Z
	t_SyuSub		ManualSettle;									// �}�j���A�����Z
	ulong			AntiPassOffSettle;								// �A���`�p�XOFF���Z��
	ulong			CarOutGateOpen;									// �Q�[�g�J�����o�ɑ䐔
	ulong			CarOutForce;									// �����o�ɑ䐔
	t_SyuSub		LostSettle;										// �������Z
	ulong			ReceiptCallback;								// �̎��؉������
	ulong			ReceiptIssue;									// �̎��ؔ��s����
	ulong			WarrantIssue;									// �a��ؔ��s����
	t_SyuSub_AllSystem	AllSystem;									// �S���u
	ulong			CarInTotal;										// �����ɑ䐔
	ulong			CarOutTotal;									// ���o�ɑ䐔
	ulong			CarIn1;											// ����1���ɑ䐔
	ulong			CarOut1;										// �o��1�o�ɑ䐔
	ulong			CarIn2;											// ����2���ɑ䐔
	ulong			CarOut2;										// �o��2�o�ɑ䐔
	ulong			CarIn3;											// ����3���ɑ䐔
	ulong			CarOut3;										// �o��3�o�ɑ䐔
	ulong			MiyoCount;										// ��������
	ulong			MiroMoney;										// �������z
	ulong			LagExtensionCnt;								// ���O�^�C��������
	ulong			FurikaeCnt;										// �U�։�
	ulong			FurikaeTotal;									// �U�֊z
	ulong			RemoteCnt;										// ���u���Z��
	ulong			RemoteTotal;									// ���u���Z���z
	ulong			Rsv2[6];										// �\��8�`13
} DATA_KIND_42_T;

/*--------------------------------------------------------------------------*/
/*	������ʖ��W�v�ް�(�ް����43)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			CenterSeqNo;									// �����ǔ�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	t_SyuSub_RyokinKind	Kind[50];									// ���01�`50
} DATA_KIND_43_T;

/*--------------------------------------------------------------------------*/
/*	���ޏW�v�ް�(�ް����44)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			CenterSeqNo;									// �����ǔ�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	ulong			Kind;											// ���ԕ��ޏW�v�̎��
	t_SyuSub		Group[BUNRUI_CNT];								// ����1�`48
	t_SyuSub		GroupTotal;										// ���ވȏ�
	ulong			LostSettle;										// �������Z �䐔
	ulong			AntiPassOffSettle;								// �A���`�p�XOFF���Z �䐔
	ulong			CarOutForce;									// �����o�� �䐔
	ulong			FirstTimePass;									// ������ �䐔
	t_SyuSub		Unknown;										// ���ޕs��
	uchar			Rsv[32];										// �\��
	ulong			Kind2;											// ���ԕ��ޏW�v�̎��_2
	t_SyuSub		Group2[BUNRUI_CNT];								// ����1�`48_2
	t_SyuSub		GroupTotal2;									// ���ވȏ�_2
	ulong			LostSettle2;									// �������Z �䐔_2
	ulong			AntiPassOffSettle2;								// �A���`�p�XOFF���Z �䐔_2
	ulong			CarOutForce2;									// �����o�� �䐔_2
	ulong			FirstTimePass2;									// ������ �䐔_2
	t_SyuSub		Unknown2;										// ���ޕs��_2
	uchar			Rsv2[32];										// �\��_2
	ulong			Kind3;											// ���ԕ��ޏW�v�̎��_3
	t_SyuSub		Group3[BUNRUI_CNT];								// ����1�`48_3
	t_SyuSub		GroupTotal3;									// ���ވȏ�_3
	ulong			LostSettle3;									// �������Z �䐔_3
	ulong			AntiPassOffSettle3;								// �A���`�p�XOFF���Z �䐔_3
	ulong			CarOutForce3;									// �����o�� �䐔_3
	ulong			FirstTimePass3;									// ������ �䐔_3
	t_SyuSub		Unknown3;										// ���ޕs��_3
	uchar			Rsv3[32];										// �\��_3
} DATA_KIND_44_T;

/*--------------------------------------------------------------------------*/
/*	�����W�v�ް�(�ް����45)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			CenterSeqNo;									// �����ǔ�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	t_SyuSub_Discount	Discount[500];								// ���� 001�`500
} DATA_KIND_45_T;

/*--------------------------------------------------------------------------*/
/*	����W�v�ް�(�ް����46)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			kind;											// �I�v�V����������
	ulong			count;											// ��
	ulong			sales;											// ������z
} t_SyuSub_Opt_Price;
typedef struct {
	DATA_BASIC			DataBasic;									// ��{�ް�
	ulong				CenterSeqNo;								// �����ǔ�
	ushort				Type;										// �W�v�^�C�v
	ushort				KakariNo;									// �W��No.
	ulong				SeqNo;										// �W�v�ǔ�
	ulong				StartSeqNo;									// �J�n�ǔ�
	ulong				EndSeqNo;									// �I���ǔ�
	t_SyuSub_Opt_Price	Opt_Price[6];								// �I�v�V�������
	t_SyuSub_Pass		Pass[100];									// ����� 001�`100
} DATA_KIND_46_T;

/*--------------------------------------------------------------------------*/
/*	���K�W�v�ް�(�ް����48)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			CenterSeqNo;									// �����ǔ�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	ulong			Total;											// ���ɑ������z
	ulong			NoteTotal;										// �������ɑ������z
	ulong			CoinTotal;										// �R�C�����ɑ������z
	t_SyuSub_Coin	Coin[4];										// �R�C��1�`4
	t_SyuSub_Coin	CoinRsv[4];										// �R�C���\��1�`4
	t_SyuSub_Note	Note[4];										// ����1�`4
	t_SyuSub_Note	NoteRsv[4];										// �����\��1�`4
	ulong			CycleAccept;									// �z���������z
	ulong			CyclePay;										// �z�����o���z
	ulong			NoteAcceptTotal;								// �����������z
	ulong			NotePayTotal;									// ���������o���z
	ulong			StockPayTotal;									// �\�~�����o���z
	t_SyuSub_Cycle	Cycle[4];										// �z��1�`4
	t_SyuSub_Cycle	CycleRsv[4];									// �z�\��1�`4
	t_SyuSub_Stock	Stock[2];										// �\�~��1�`2
	t_SyuSub_Note	NotePay[4];										// �������o1�`4
	t_SyuSub_Note	NotePayRsv[4];									// �������o�\��1�`4
} DATA_KIND_48_T;

/*--------------------------------------------------------------------------*/
/*	�W�v�I���ʒm�ް�(�ް����53)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			CenterSeqNo;									// �����ǔ�
	ushort			Type;											// �W�v�^�C�v
	ushort			KakariNo;										// �W��No.
	ulong			SeqNo;											// �W�v�ǔ�
	ulong			StartSeqNo;										// �J�n�ǔ�
	ulong			EndSeqNo;										// �I���ǔ�
	ulong			MachineNo[32];									// �����W�v�����[���̋@�BNo.-001�`-032
} DATA_KIND_53_T;

/*	�װ�ް�(�ް����63)														*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	uchar			Errsyu;											// �װ���
	uchar			Errcod;											// �װ����
	uchar			Errdtc;											// �װ�ð��(����/����)
	uchar			Errlev;											// �װ����
	uchar			ErrDoor;										// �ޱ���(0:close,1:open)
	uchar			Errdat1[10];									// �װ���(�޲��)
	uchar			Errdat2[160];									// �װ���(acsii����)
} DATA_KIND_63_T;

/*--------------------------------------------------------------------------*/
/*	�װ��ް�(�ް����64)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	uchar			Armsyu;											// �װю��
	uchar			Armcod;											// �װѺ���
	uchar			Armdtc;											// �װѽð��(����/����)
	uchar			Armlev;											// �װ�����
	uchar			ArmDoor;										// �ޱ���(0:close,1:open)
	uchar			Armdat1[10];									// �װя��(�޲��)
	uchar			Armdat2[160];									// �װя��(acsii����)
} DATA_KIND_64_T;

/*--------------------------------------------------------------------------*/
/*	�ޑK�Ǘ��W�v�ް�(�ް����135)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			CenterSeqNo;									// �Z���^�[�ǔ�
	ulong			Oiban;											// ���K�Ǘ����v�ǔ�
	ushort			PayClass;										// �����敪
	ushort			KakariNo;										// �W����
	TURI_DATA		turi_dat[4];									// ���K�ް�(4���핪)
	TURI_DATA		yturi_dat[5];									// ���K�ް�(5�\�~��)
	uchar			Reserve[32];									// �\��
} DATA_KIND_135_T;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// GM849100(S) M.Fujikawa 2025/01/10 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
/*--------------------------------------------------------------------------*/
/*	�[������ް�(�ް����230)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			Status;								// ���j�b�g���
	uchar			Err_md;								// �G���[��ʁi���W���[���m���j
	uchar			Err_no;								// �G���[�ԍ�
} UNIT_DATA;

/*--------------------------------------------------------------------------*/
/*	�[������ް�(�ް����230)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ulong			SMachineNo;										// ���M��[���@�B��
	uchar			ProgramVer[12];									// ��۸����ް�ޮ�
	UNIT_DATA		UnitInfo[10];
																	// [0]: ���[�_�[�P
																	// [1]: ���[�_�[�Q
																	// [2]: �������j�b�g�i�Ȃ��j
																	// [3]: �W���[�i���v�����^
																	// [4]: ���V�[�g�v�����^
																	// [5]: �R�C�����b�N
																	// [6]: �������[�_�[
																	// [7]: �Q�[�g���u
																	// [8]: �I�v�V�������j�b�g
	uchar			Dummy[80];
} DATA_KIND_230_T;
// GM849100(E) M.Fujikawa 2025/01/10 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j

// �V�X�e��ID�F16
#define PIP_REQ_KIND_RECEIPT_AGAIN			1			// �̎����Ĕ��s�v��
#define PIP_RES_KIND_RECEIPT_AGAIN			2			// �̎��؍Ĕ��s����

#define PIP_REQ_KIND_FURIKAE				3			// �U�֐��Z�v��
#define PIP_RES_KIND_FURIKAE				4			// �U�֐��Z����

#define PIP_REQ_KIND_RECEIVE_TKT			5			// ��t�����s�v��
#define PIP_RES_KIND_RECEIVE_TKT			6			// ��t�����s����

#define PIP_REQ_KIND_REMOTE_CALC			7			// ���u���Z�v��
#define PIP_RES_KIND_REMOTE_CALC_ENTRY		8			// ���u���Z���Ɏ����w�萸�Z����
#define PIP_RES_KIND_REMOTE_CALC_FEE		9			// ���u���Z���z�w�萸�Z����

#define PIP_REQ_KIND_OCCUR_ERRALM			10			// �������G���[�A���[���v��
#define PIP_RES_KIND_OCCUR_ERRALM			11			// �������G���[�A���[������

// ���u���Z�v�����
#define	PIP_REMOTE_TYPE_CALC				1
#define	PIP_REMOTE_TYPE_CALC_TIME			2
#define	PIP_REMOTE_TYPE_CALC_FEE			3

// �U�֐��Z�v�����
#define	PIP_FURIKAE_TYPE_INFO				1
#define	PIP_FURIKAE_TYPE_CHK				2
#define	PIP_FURIKAE_TYPE_GO					3

// �U�֐��Z�X�e�[�^�X
enum {
	PIP_FRK_STS_IDLE = 0,							// �A�C�h�����
	PIP_FRK_STS_ACP,								// �U�֐��Z ��t
	PIP_FRK_STS_MAX,
};

// ����
#define PIP_RES_RESULT_OK					0			// ����
#define PIP_RES_RESULT_NG_DENY				11			// ��t���ہi��t�s��ԁj
#define PIP_RES_RESULT_NG_PARAM				12			// �p�����[�^�s��
#define PIP_RES_RESULT_NG_NO_CAR			13			// �w��Ԏ�����
#define PIP_RES_RESULT_NG_BUSY				14			// �r�W�[�i�N���W�b�g�₢���킹���ȂǁA�ꎞ�I�Ɏ󂯕t�����Ȃ��ꍇ
#define PIP_RES_RESULT_NG_OTHER				19			// ���̑��G���[
#define PIP_RES_RESULT_NG_CHANGE_NEXT		21			// �v����t�������A���̌�̏����ւ̈ڍs�����s
#define PIP_RES_RESULT_NG_CHANGE_PAY		22			// �����v�Z������A���Z����ԂɈڍs���s
#define PIP_RES_RESULT_OK_STOP_USER			23			// �v�����s�����i���p�҂ɂ�钆�~�I���j
#define PIP_RES_RESULT_OK_STOP				24			// �v�����s�����i���Z�@�����s�s�������m�ɂ�钆�~�I���j
#define PIP_RES_RESULT_NG_FURIKAE_SRC		25			// �U�֌������G���[
#define PIP_RES_RESULT_NG_FURIKAE_DST		26			// �U�֐�����G���[

// UNION PIP_CTRL�̎��
enum{
	UNI_KIND_FURIKAE = 1,
	UNI_KIND_REMOTE_TIME,
	UNI_KIND_REMOTE_FEE,
};
// ============================================================ //
//			�\���̒�`
// ============================================================ //
// PiP���ʕ���
typedef	struct {
	ushort			FmtRev;					// �t�H�[�}�b�gRev.��
	ulong			SMachineNo;				// ���M��^�[�~�i����
	ulong			CenterSeqNo;			// �Z���^�[�ǔ�
} PIP_COMMON;

// �������
typedef struct{
	ulong			ParkNo;					// �������ԏꇂ
	ushort			DiscSyubet;				// �������
	ushort			DiscType;				// �����敪
	ushort			DiscCount;				// �����g�p����
	ulong			DiscPrice;				// �������z
	ulong			DiscInfo1;				// �������P
	ulong			DiscInfo2;				// �������Q
} PIP_Discount;

// �}�̏��
typedef struct{
	ushort		Kind;						// ���
	uchar		CardNo[30];					// �J�[�h�ԍ�
	uchar		CardInfo[16];				// �J�[�h���
	ushort		Reserve;					// �\��(�T�C�Y�����p)
} PIP_Media;

#define	PIP_DISCOUNT_MAX					25

// �Ԏ����
typedef struct{
	ushort			SrcArea;				// ���
	ulong			SrcNo;					// �Ԏ�
	date_time_rec2	Date1;					// ���Z/���ɔN���������b
	date_time_rec2	Date2;					// ����/�����v�Z�N���������b
	ushort			SrcFeeKind;				// �������
	ulong			SrcFee;					// ���ԗ���
	ulong			SrcFeeDiscount;			// �������z
	ushort			SrcStatus;				// �X�e�[�^�X
} PIP_FlpInfo;

// �I�v�V�������
typedef struct{
	ushort		kind;						// ���
	ulong		Amount;						// ���z
} PIP_Option;

#define	PIP_OPTION_MAX						4
// ============================================================ //
//			�v���R�}���h�\���̒�`
// ============================================================ //

// �U�֐��Z�v��
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			ReqKind;				// �v�����
	ushort			SrcArea;				// �U�֌����
	ulong			SrcNo;					// �U�֌��Ԏ��ԍ�
	ushort			DestArea;				// �U�֐���
	ulong			DestNo;					// �U�֐�Ԏ��ԍ�
	ushort			SrcPassUse;				// �U�֌�������p�L��
} DATA_KIND_16_03;

// ���u���Z�v��
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	uchar			ReqKind;				// �v�����
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//	uchar			Syubet;					// �������
	uchar			Type;					// ���u���Z���(0=�o�ɐ��Z/1=������Z)
	ushort			Syubet;					// �������
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ushort			Area;					// ���
	ulong			ulNo;					// �Ԏ��ԍ�
	date_time_rec2	InTime;					// ���ɔN���������b
	ulong			Price;					// ���ԗ���			0�`
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ulong			FutureFee;				// ������Z�\��z
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	PIP_Discount	stDiscount;				// �������
} DATA_KIND_16_07;


// ��t�����s�v��
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			Area;					// ���
	ulong			ulNo;					// �Ԏ��ԍ�
} DATA_KIND_16_05;

// �̎��؍Ĕ��s�v��
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			PayInfoKind;			// ���Z�����
	ulong			PayCount;				// ���Z�ǔ�
	uchar			PayMethod;				// ���Z���@
	uchar			PayClass;				// �����敪
	uchar			PayMode;				// ���ZӰ��
	uchar			CMachineNo;				// ���Ԍ��@�B��(���ɋ@�B�����t���b�v���̓[��)
	ushort			Area;					// ���
	ushort			No;						// �Ԏ��ԍ�
	ushort			KakariNo;				// �W����
	uchar			OutKind;				// ���Z�o��
	uchar			CountSet;				// �ݎԶ���
	uchar			OutTime[6];				// ���Z�N���������b
	uchar			InTime[6];				// ���ɔN���������b
	uchar			PreOutTime[6];			// �O�񐸎Z�N���������b
	uchar			ReceiptIssue;			// �̎��ؔ��s�L��
	uchar			Syubet;					// �������
	ulong			TotalFee;				// ���ԗ���/���v���z
	ulong			TotalMoney;				// ��������f�[�^
	ulong			InMoney;				// �������z
	ushort			Change;					// �ޑK�z
	struct{	// ���K���;
		uchar		InMoneyBit;				// ����������L��
		uchar		InMoneyCnt[8];
	} InMoneyInfo;
	struct{	// ���o���;
		uchar		OutMoneyBit;			// ���o������L��
		uchar		OutMoneyCnt[8];
	} OutMoneyInfo;
	ushort			Fusoku;					// ���߂��s���z
// �d�l�ύX(S) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	ushort			CardFusokuType;			// ���߂��s���}��(�����ȊO)
	ushort			CardFusoku;				// ���߂��s���z(�����ȊO)
// �d�l�ύX(E) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	ushort			Antipas;				// �A���`�p�X�`�F�b�N(0=�`�F�b�NON/1=�`�F�b�NOFF/2=����OFF)������g�p�Ȃ���1�Œ�
	PIP_Media		MainMedia;				// ���C���}�̏��
	PIP_Media		SubMedia;				// �T�u�}�̏��
	ushort			CardPaymentKind;		// �J�[�h���ϋ敪(0=�Ȃ�/1=�N���W�b�g/2=��ʌn/3=Edy/4=PiTaPa/5=WAON/6=nanaco/7=SPICA)
// �d�l�ύX(S) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
//	uchar			CardPaymentInfo[64];	// ���Ϗ�񁦈Í����Ώ�
//	struct{	// �^�C�����W���
//		ulong		InMoney;				// �����������z
//		ulong		OutMoney;				// �o���������z
//		ulong		ReturnMoney;			// ���߂����z
//		ulong		FusokuMoney;			// �󂯎��s�����z
//	} TimeRegInfo;
	uchar			CardPaymentInfo[80];	// ���Ϗ�񁦈Í����Ώ�
	ulong			TotalSale;				// ���v���z(����)
	ulong			DeleteSeq;				// �폜�f�[�^�ǔ�
// �d�l�ύX(E) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	ulong			SalesDestination;		// ����撓�ԏꇂ�������p�ŁA�W���͊�{���ԏꇂ���Z�b�g
	struct{	// ��������
		ulong		ParkNo;					// ���ԏꇂ
		ulong		Id;						// ID
		ushort		Kind;					// ���
		uchar		Status;					// �X�e�[�^�X
		uchar		ReadOnly;				// ���[�h�I�����[(0=���[�h���C�g/1=���[�h���C�g)
		uchar		ReturnKind;				// �ԋp/���
		uchar		UpdataStartDate[3];		// �X�V��̗L���J�n�N����
		uchar		StartDate[3];			// �L���J�n�N����
		uchar		EndDate[3];				// �L���I���N����
		ulong		UpdateFee;				// �X�V����
		uchar		UpdateFlag;				// 0=�V�K/1=�X�V
		uchar		UpdateCnt;				// �X�V��
		uchar		UpdateTerm;				// �X�V����
	} PassInfo;
	uchar			ReissueCnt;				// �Ĕ��s��
// �d�l�ύX(S) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	uchar			card_name[12];			// �N���W�b�g�J�[�h��Ж�
// �d�l�ύX(E) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	PIP_Option		stOption[PIP_OPTION_MAX];
	PIP_Discount	stDiscount[PIP_DISCOUNT_MAX];
} DATA_KIND_16_01;

// �������G���[�A���[���v��
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			ReqFlg;					// �v���G���[�A���[���t���O(0=�G���[/1=�A���[��)
} DATA_KIND_16_10;


// ============================================================ //
//			�����R�}���h�\���̒�`
// ============================================================ //

// ��t�����s����
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			Result;					// ����
	ushort			Area;					// ���
	ulong			ulNo;					// �Ԏ��ԍ�
} DATA_KIND_16_06;

// �̎��؍Ĕ��s����
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			Type;					// ���Z�����(0=���Z/1=����X�V)
	ushort			Result;					// ����
	ulong			PayCount;				// ���Z�ǂ���
	uchar			PayMethod;				// ���Z���@
	uchar			PayClass;				// �����敪
	uchar			PayMode;				// ���ZӰ��
	uchar			CMachineNo;				// ���Ԍ��@�B��
	ushort			Area;					// ���
	ushort			No;						// �Ԏ��ԍ�
	ushort			KakariNo;				// �W����
// MH810105(S) MH364301 �̎��؍Ĕ��s�����̃t�H�[�}�b�g�ԈႢ���C��
//	ushort			OutKind;				// ���Z�o��
//	ushort			CountSet;				// �ݎԶ���
	uchar			OutKind;				// ���Z�o��
	uchar			CountSet;				// �ݎԶ���
// MH810105(E) MH364301 �̎��؍Ĕ��s�����̃t�H�[�}�b�g�ԈႢ���C��
	uchar			OutTime[6];				// ���Z�N���������b
	uchar			InTime[6];				// ���ɔN���������b
	uchar			PreOutTime[6];			// �O�񐸎Z�N���������b
// MH810105(S) MH364301 �̎��؍Ĕ��s�����̃t�H�[�}�b�g�ԈႢ���C��
//	ushort			ReceiptIssue;			// �̎��ؔ��s�L��
	uchar			ReceiptIssue;			// �̎��ؔ��s�L��
// MH810105(E) MH364301 �̎��؍Ĕ��s�����̃t�H�[�}�b�g�ԈႢ���C��
	uchar			Syubet;					// �������
	ulong			TotalFee;				// ���ԗ���/���v���z
	ulong			TotalMoney;				// ��������f�[�^
	ulong			InMoney;				// �������z
	ushort			Change;					// �ޑK�z
	struct{	// ���K���;
		uchar		InMoneyBit;				// ����������L��
		uchar		InMoneyCnt[8];
	} InMoneyInfo;
	struct{	// ���o���;
		uchar		OutMoneyBit;			// ����������L��
		uchar		OutMoneyCnt[8];
	} OutMoneyInfo;
	ushort			Fusoku;					// ���߂��s���z
// �d�l�ύX(S) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	ushort			CardFusokuType;			// ���߂��s���}��(�����ȊO)
	ushort			CardFusoku;				// ���߂��s���z(�����ȊO)
// �d�l�ύX(E) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	ushort			Antipas;				// �A���`�p�X�`�F�b�N(0=�`�F�b�NON/1=�`�F�b�NOFF/2=����OFF)������g�p�Ȃ���1�Œ�
	PIP_Media		MainMedia;				// ���C���}�̏��
	PIP_Media		SubMedia;				// �T�u�}�̏��
	ushort			CardPaymentKind;		// �J�[�h���ϋ敪(0=�Ȃ�/1=�N���W�b�g/2=��ʌn/3=Edy/4=PiTaPa/5=WAON/6=nanaco/7=SPICA)
// �d�l�ύX(S) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
//	uchar			CardPaymentInfo[64];	// ���Ϗ�񁦈Í����Ώ�
//	struct{	// �^�C�����W���
//		ulong		InMoney;				// �����������z
//		ulong		OutMoney;				// �o���������z
//		ulong		ReturnMoney;			// ���߂����z
//		ulong		FusokuMoney;			// �󂯎��s�����z
//	} TimeRegInfo;
	uchar			CardPaymentInfo[80];	// ���Ϗ�񁦈Í����Ώ�
	ulong			TotalSale;				// ���v���z(����)
	ulong			DeleteSeq;				// �폜�f�[�^�ǔ�
// �d�l�ύX(E) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	ulong			SalesDestination;		// ����撓�ԏꇂ�������p�ŁA�W���͊�{���ԏꇂ���Z�b�g
	struct{	// ��������
		ulong		ParkNo;					// ���ԏꇂ
		ulong		Id;						// ID
		ushort		Kind;					// ���
		uchar		Status;					// �X�e�[�^�X
		uchar		ReadOnly;				// ���[�h�I�����[(0=���[�h���C�g/1=���[�h���C�g)
		uchar		ReturnKind;				// �ԋp/���
		uchar		UpdataStartDate[3];		// �X�V��̗L���J�n�N����
		uchar		StartDate[3];			// �L���J�n�N����
		uchar		EndDate[3];				// �L���I���N����
		ulong		UpdateFee;				// �X�V����
		uchar		UpdateFlag;				// 0=�V�K/1=�X�V
		uchar		UpdateCnt;				// �X�V��
		uchar		UpdateTerm;				// �X�V����
	} PassInfo;
	uchar			ReissueCnt;				// �Ĕ��s��
// �d�l�ύX(S) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	uchar			card_name[12];			// �N���W�b�g�J�[�h��Ж�
// �d�l�ύX(E) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
	PIP_Option		stOption[PIP_OPTION_MAX];
	PIP_Discount	stDiscount[PIP_DISCOUNT_MAX];
	
} DATA_KIND_16_02;

typedef struct {
	uchar			ucKind;					// ���
	uchar			ucCoce;					// �R�[�h
	uchar			ucLevel;				// ���x��
	uchar			ucDoor;					// �h�A���
	uchar			ucInfo[10];				// �G���[�A���[�����
}PIP_ErrAlarm;

#define PIP_OCCUR_ERROR_ALARM_MAX			60

// �������G���[�����e�[�u��
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			Result;					// ����
	ushort			Kind;					// �����G���[�A���[���t���O�i0=�G���[�^1=�A���[���j
	ushort			Count;					// �������G���[�A���[������
	PIP_ErrAlarm	stErrAlm[PIP_OCCUR_ERROR_ALARM_MAX];
} DATA_KIND_16_11;

// �U�֐��Z����
typedef struct{
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			RcvKind;				// �������
	ushort			Result;					// ����
	uchar			OpenClose;				// �c�x�Ə�
	uchar			MntMode;				// ���Z�@���샂�[�h��(0=�ʏ�/1=�����e)
	uchar			ErrOccur;				// �G���[������(0=�Ȃ�/1=����)
	uchar			AlmOccur;				// �A���[��������(0=�Ȃ�/1=����)
	ushort			PayState;				// ���Z�������
	ushort			PassUse;				// �U�֌�������p�L��
	PIP_FlpInfo		stSrcInfo;				// �U�֌��Ԏ����
	PIP_FlpInfo		stDestInfo;				// �U�֐�Ԏ����
	ulong			PassParkingNo;			// ��������ԏ�ԍ�
	ulong			PassID;					// �����ID
	ushort			PassKind;				// ��������
	long			Remain;					// �x���c�z
} DATA_KIND_16_04;

// ���u���Z���Ɏ����w�萸�Z����
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			RcvKind;				// �������
	ushort			Result;					// ����
	ushort			Area;					// ���
	ulong			ulNo;					// �Ԏ��ԍ�
	date_time_rec2	PayTime;				// �����v�Z�N���������b
	date_time_rec2	InTime;					// ���ɔN���������b
	uchar			ParkTimeDay;			// ���Ԏ��ԓ�
	uchar			ParkTimeHour;			// ���Ԏ��Ԏ�
	uchar			ParkTimeMin;			// ���Ԏ��ԕ�
	uchar			ParkTimeSec;			// ���Ԏ��ԕb
	ulong			ParkTotalSec;			// ���ԑ����ԕb��
	ushort			Syubet;					// �������
	ulong			Price;					// ���ԗ���			0�`
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ulong			FutureFee;				// ������Z�\��z
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	PIP_Discount	stDiscount;				// �������
} DATA_KIND_16_08;

// ���u���Z���z�w�萸�Z����
typedef struct {
	DATA_BASIC		DataBasic;				// ��{�ް�
	PIP_COMMON		Common;					// Park I Pro���ʃf�[�^
	ushort			Result;					// ����
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ushort			Type;					// ���u���Z���
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ushort			Area;					// ���
	ulong			ulNo;					// �Ԏ��ԍ�
	ushort			Syubet;					// �������
	ulong			Price;					// ���ԗ���			0�`
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ulong			FutureFee;			// ������Z�\��z
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	PIP_Discount	stDiscount;				// �������
} DATA_KIND_16_09;

// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
/*--------------------------------------------------------------------------*/
/*	�������ԏ��f�[�^(�ް����61)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// ��{�ް�
	ushort			FmtRev;											// �t�H�[�}�b�gRev.��
	uchar			dtc;											// �X�e�[�^�X(����/����/�S����)
	uchar			Reserve;										// �\���i�T�C�Y�����p�j
	proc_date		InTime;											// ���ɓ���
	proc_date		OutTime;										// �o�ɓ���
	ulong			Ck_Time;										// �������Ԍ��o����
	t_MediaInfo2	Media[4];										// ���ɔ}�̏��1�`4
} DATA_KIND_61_r10;

#define LONGPARK_LOG_SET		0// ����
#define LONGPARK_LOG_RESET		1// ����
#define LONGPARK_LOG_ALL_RESET	2// �S����

#define LONGPARK_LOG_NON		0// �w��Ȃ�
#define LONGPARK_LOG_PAY		1// ���Z
#define LONGPARK_LOG_KYOUSEI_FUSEI	2// ����/�s��
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
#pragma unpack

/*--------------------------------------------------------------------------*/
/*	Free�ް�																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			FreeData[500];									// FreeData
} DATA_KIND_Free;


typedef	union {
	DATA_BASIC		DataBasic;
	DATA_KIND_01	SData01;
	DATA_KIND_02	SData02;
	DATA_KIND_05	SData05;
	DATA_KIND_12	SData12;
	DATA_KIND_20	SData20;
	DATA_KIND_22	SData22;
	DATA_KIND_54	SData54;
	DATA_KIND_56	SData56;
	DATA_KIND_58	SData58;
	DATA_KIND_58_r10	SData58_r10;
	DATA_KIND_100	SData101;
	DATA_KIND_103	SData104;
	DATA_KIND_120	SData120;
	DATA_KIND_121	SData121;
	DATA_KIND_126	SData126;
	DATA_KIND_142	SData142;
	DATA_KIND_119	SData229;
	DATA_KIND_230	SData230;
	DATA_KIND_231	SData231;
	DATA_KIND_99	SData99;
	DATA_KIND_109	SData109;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
//	DATA_KIND_152	SData152;	// ���Z���f�[�^
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	DATA_KIND_152_r12	SData152_r12;	// ���Z���f�[�^
	DATA_KIND_152_rXX	SData152_r12;	// ���Z���f�[�^
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	DATA_KIND_153	SData153;	// ���A���^�C�����
	DATA_KIND_154	SData154;	// �Z���^�[�p�f�[�^�v��
	DATA_KIND_116	SData116;
	DATA_KIND_122	SData122;
	DATA_KIND_123	SData123;
	DATA_KIND_80	SData208;
	RP_DATA_KIND_80	RP_SData208;
	DATA_KIND_219	SData219;
	DATA_KIND_93	SData221;
	DATA_KIND_94	SData222;
	DATA_KIND_95	SData223;
	DATA_KIND_225	SData225;
	DATA_KIND_226	SData226;
	DATA_KIND_30	SData30;
	DATA_KIND_31	SData31;
	DATA_KIND_32	SData32;
	DATA_KIND_33	SData33;
	DATA_KIND_34	SData34;
	DATA_KIND_35	SData35;
	DATA_KIND_36	SData36;
	DATA_KIND_41	SData41;
	DATA_KIND_Free	SDataFree;
	RP_DATA_KIND_115	SData115;
	DATA_KIND_130	SData130;
	DATA_KIND_132	SData132;
	DATA_KIND_211	SData211;
	DATA_KIND_228	SData228;
	DATA_KIND_235	SData235;

	DATA_KIND_78	SData90;

	DATA_KIND_236	SData236;
	DATA_KIND_105	SData105;
	DATA_KIND_110	SData110;
	DATA_KIND_155	SData155;
	DATA_KIND_157	SData157;
	DATA_KIND_244	SData244;
	DATA_KIND_245	SData245;
	DATA_BASIC_R	DataBasicR;
	DATA_KIND_60_R	SData60_R;
	DATA_KIND_61_R	SData61_R;
	DATA_KIND_83_R	SData63_R;
	DATA_KIND_100_R	SData100_R;
	DATA_KIND_143	SData143;
	DATA_KIND_212	SData212;
	DATA_KIND_255	SData255;
	DATA_KIND_108	SData108;
	DATA_KIND_185	SData185;
	DATA_KIND_14	SData14;
	DATA_KIND_148	SData148;
	DATA_KIND_189	SData189;
	DATA_KIND_190	SData190;
	DATA_KIND_135	SData135;
	DATA_KIND_42	SData42;
	DATA_KIND_43	SData43;
	DATA_KIND_45	SData45;
	DATA_KIND_46	SData46;
	DATA_KIND_53	SData53;
	DATA_KIND_65	SData65;

	DATA_KIND_136_01	SData136_01;	// �J�Ǻ����:��ײ�ý�
	DATA_KIND_136_03	SData136_03;	// �ڼޯė^�M�⍇���ް�
	DATA_KIND_136_05	SData136_05;	// �ڼޯĔ���˗��ް�
	DATA_KIND_136_07	SData136_07;	// ýĺ����
	DATA_KIND_136_09	SData136_09;	// �ڼޯĕԕi�⍇���ް�
	DATA_KIND_118_DL	SData118_DL;
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
	DATA_KIND_16_02		SData16_02;		// �̎��؍Ĕ��s����
	DATA_KIND_16_04		SData16_04;		// �U�֐��Z����
	DATA_KIND_16_06		SData16_06;		// ��t�����s����
	DATA_KIND_16_08		SData16_08;		// ���u���Z���Ɏ����w�萸�Z����
	DATA_KIND_16_09		SData16_09;		// ���u���Z���z�w�萸�Z����
	DATA_KIND_16_11		SData16_11;		// �������G���[�����e�[�u��
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	DATA_KIND_61_r10	SData61_r10;
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	DATA_KIND_56_T	SData56_T;
	DATA_KIND_63_T	SData63_T;
	DATA_KIND_64_T	SData64_T;
	DATA_KIND_230_T	SData230_T;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

} SEND_NTNET_DT;

extern	SEND_NTNET_DT	SendNtnetDt;								// NE-NET���M�ޯ̧(ܰ�)


typedef	union {
	DATA_BASIC		DataBasic;
	DATA_KIND_01	RData01;
	DATA_KIND_02	RData02;
	DATA_KIND_03	RData03;
	DATA_KIND_04	RData04;
	DATA_KIND_22	RData22;
	DATA_KIND_100	RData100;
	DATA_KIND_103	RData103;
	DATA_KIND_109	RData109;
	DATA_KIND_154	RData154;
	DATA_KIND_156	RData156;
	DATA_KIND_110	RData110;
	DATA_KIND_155	RData155;
	DATA_KIND_116	RData116;
	DATA_KIND_117	RData117;
	DATA_KIND_119	RData119;
	DATA_KIND_143	RData143;
	DATA_KIND_80	RData80;
	RP_DATA_KIND_80	RP_RData80;
	RP_DATA_KIND_78	RP_RData78;
	DATA_KIND_91	RData91;
	DATA_KIND_93	RData93;
	DATA_KIND_94	RData94;
	DATA_KIND_95	RData95;
	DATA_KIND_97	RData97;
	DATA_KIND_98	RData98;
	DATA_KIND_66	RData66;
	DATA_KIND_67	RData67;
	DATA_KIND_68	RData68;
	DATA_KIND_69	RData69;
	DATA_KIND_70	RData70;
	DATA_KIND_72	RData72;
	DATA_KIND_77	RData77;
	DATA_KIND_78	RData78;
	DATA_KIND_83	RData83;
	RP_DATA_KIND_114	RData114;
	DATA_KIND_234	RData234;
	DATA_KIND_236	RData236;
	DATA_KIND_92	RData92;
	DATA_KIND_240	RData240;
	DATA_KIND_243	RData243;
	DATA_BASIC_R	DataBasicR;
	DATA_KIND_83_R	RData83_R;
	DATA_KIND_100_R	RData101_R;
	DATA_KIND_60_R	RData60_R;
	DATA_KIND_90_R	RData90_R;
	DATA_KIND_142	RData142;
	DATA_KIND_84	RData84;
	DATA_KIND_151	RData151;
	DATA_KIND_254	RData254;
	DATA_KIND_184	RData184;
	DATA_KIND_149	RData149;
	DATA_KIND_188	RData188;

	DATA_KIND_137_02	RData137_02;	// �J�Ǻ���މ���:��ײ�ý�
	DATA_KIND_137_04	RData137_04;	// �ڼޯė^�M�⍇�������ް�
	DATA_KIND_137_06	RData137_06;	// �ڼޯĔ���˗������ް�
	DATA_KIND_137_08	RData137_08;	// ýĺ���މ���
	DATA_KIND_137_0A	RData137_0A;	// �ڼޯĕԕi�⍇�������ް�
	DATA_KIND_111_DL	RData111_DL;
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
	DATA_KIND_16_01		RData16_01;		// �̎��؍Ĕ��s�v��
	DATA_KIND_16_03		RData16_03;		// �U�֐��Z�v��
	DATA_KIND_16_05		RData16_05;		// ��t�����s�v��
	DATA_KIND_16_07		RData16_07;		// ���u���Z�v��
	DATA_KIND_16_10		RData16_10;		// �������G���[�A���[���v��
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�

} RECV_NTNET_DT;

extern	RECV_NTNET_DT	RecvNtnetDt;								// NE-NET��M�ޯ̧(ܰ�)

extern	DATA_KIND_58	SData58_bk;

extern	short		NTNetID100ResetFlag;							// ����f�[�^���Z�b�g�w�����s�t���O
extern	short		NTNetTotalEndFlag;								// �W�v�f�[�^�ŏI�d����M�t���O
extern	short		NTNetTotalEndError;								// �f�[�^�v���QNG��M
extern	NTNETCTRL_FUKUDEN_PARAM	NtNet_FukudenParam;					// �Ԏ��p�����[�^/���b�N��ʃp�����[�^ �s�i�p

extern	ulong		NTNetDataCont[4];								// �e�ް��ǂ���
																	// 0=���ɒǂ���
																	// 1=�o�ɒǂ���
																	// 2=���Z�ǂ���
																	// 3=���Z���~

typedef struct {
	ulong			MachineNo;										// �O���M�����@�B��
	ulong			PayCount;										// �O���M�������Z�ǂ���
	ushort			PayMethod;										// �O���M�������Z���@
	ushort			PayClass;										// �O���M���������敪
	ushort			PayMode;										// �O���M�������ZӰ��
	ulong			LockNo;											// �O���M���������
	date_time_rec2	OutTime;										// �O���M�������Z�N���������b
} RECV_MULTI_DT22;

extern	RECV_MULTI_DT22	RcvDt22;
extern	date_time_rec2	NTNetTTotalTime;
extern	date_time_rec2	NTNetTime_152;
extern	date_time_rec2	NTNetTime_152_wk;

// ���Z���f�[�^�ێ��p
typedef struct	{								// ���ɓ���
	short	year;
	char	mon;
	char	day;
	char 	week;
	char	hour;
	char	min;
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
	char	sec;
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
} NTNET_CAR_IN_TIM;																	/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/

typedef	struct {
	ulong			w;			// whole
	ulong			i;			// individual
} NTNET152_ST_OIBAN;

enum {
	NTNET_152_OIBAN = 0,								// ���Z�ǔ�
	NTNET_152_PAYMETHOD,								// ���Z���@
	NTNET_152_PAYMODE,									// ���Z���[�h
// MH810100(S) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	NTNET_152_CMACHINENO,								// ���Ԍ��@�B��(���ɋ@�B��)
// MH810100(E) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	NTNET_152_WPLACE,									// ���Ԉʒu�ް�
	NTNET_152_KAKARINO,									// �W����
	NTNET_152_OUTKIND,									// ���Z�o��
	NTNET_152_COUNTSET,									// �ݎԃJ�E���g
	NTNET_152_CARINTIME,								// ���ɓ���
	NTNET_152_RECEIPTISSUE,								// �̎��ؔ��s�L��
	NTNET_152_SYUBET,									// �������
	NTNET_152_PRICE,									// ���ԗ���
	NTNET_152_CASHPRICE,								// ��������
	NTNET_152_INPRICE,									// �������z
	NTNET_152_CHGPRICE,									// �ޑK���z
// �s��C��(S) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
	NTNET_152_KABARAI,									// �U�։ߕ�����(�����߂��z)
// �s��C��(E) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
	NTNET_152_INCOIN,									// ���했��������
	NTNET_152_OUTCOIN,									// ���했�o������
	NTNET_152_FESCROW,									// ���Z�f�[�^�A���Z���f�[�^��1000�~�̕��߂��������Z�b�g����t���O
	NTNET_152_HARAIMODOSHIFUSOKU,						// ���ߕs���z
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	NTNET_152_CARD_FUSOKU_TYPE,							// ���ߕs���}��
	NTNET_152_CARD_FUSOKU,								// ���ߕs���z(�����ȊO)
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	NTNET_152_ANTIPASSCHECK,							// �A���`�p�X�`�F�b�N
	NTNET_152_PARKNOINPASS,								// ����� ���ԏꇂ
	NTNET_152_PKNOSYU,									// ��������ԏ�m���D���
	NTNET_152_TEIKIID,									// �����id
	NTNET_152_TEIKISYU,									// ��������
// MH810100(S) K.Onodera 2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	NTNET_152_MEDIAKIND1,								// ���(�}��)
	NTNET_152_MEDIACARDNO1,								// �J�[�h�ԍ�
	NTNET_152_MEDIACARDINFO1,							// �J�[�h�ԍ�
	NTNET_152_MEDIAKIND2,								// ���(�}��)
	NTNET_152_MEDIACARDNO2,								// �J�[�h�ԍ�
	NTNET_152_MEDIACARDINFO2,							// �J�[�h�ԍ�
// MH810100(E) K.Onodera 2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	NTNET_152_ECARDKIND,								// �J�[�h���ϋ敪
	NTNET_152_EPAYRYO,									// �d�q�}�l�[���ϊz
	NTNET_152_ECARDID,									// ����ID
	NTNET_152_EPAYAFTER,								// ���ό�Suica�c��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	NTNET_152_ECINQUIRYNUM,								// �⍇���ԍ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
	NTNET_152_CPAYRYO,									// �N���W�b�g���ϊz
	NTNET_152_CCARDNO,									// �N���W�b�g�J�[�h����ԍ�
	NTNET_152_CCCTNUM,									// �[�����ʔԍ�
	NTNET_152_CKID,										// �j�h�c�R�[�h
	NTNET_152_CAPPNO,									// ���F�ԍ�
	NTNET_152_CCENTEROIBAN,								// �Z���^�[�����ʔ�
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	NTNET_152_CTRADENO,									// �����X����ԍ�
	NTNET_152_SLIPNO,									// �`�[�ԍ�
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	NTNET_152_DPARKINGNO,								// ���� ���ԏꇂ
	NTNET_152_DSYU,										// ���� �������
	NTNET_152_DNO,										// ���� �����敪
	NTNET_152_DCOUNT,									// ���� ����
	NTNET_152_DISCOUNT,									// ���� �����z(��������)
	NTNET_152_DINFO1,									// ���� �������1
	NTNET_152_DINFO2,									// ���� �������2
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	NTNET_152_TRANS_STS,								// ����X�e�[�^�X
	NTNET_152_QR_MCH_TRADE_NO,							// Mch����ԍ�
	NTNET_152_QR_PAY_TERM_ID,							// �x���[��ID
	NTNET_152_QR_PAYKIND,								// QR���σu�����h
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	NTNET_152_PREVDISCOUNT,								// ���� ����g�p�����O�񐸎Z�܂ł̊������z
	NTNET_152_PREVUSAGEDISCOUNT,						// ���� �O�񐸎Z�܂ł̎g�p�ς݊������z
	NTNET_152_PREVUSAGEDCOUNT,							// ���� �O�񐸎Z�܂ł̎g�p�ςݖ���
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// GG124100(S) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
	NTNET_152_FEEKINDSWITCHSETTING,						// ���� ��ʐ؊���Ԏ�
	NTNET_152_DROLE,									// ���� ��������
// GG124100(E) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
};

typedef struct {
	NTNET152_ST_OIBAN	Oiban;		// ���Z�ǔ�(0�`99999)
	uchar		PayMethod;			// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
	uchar		PayMode;			// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
// MH810100(S) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	uchar		CMachineNo;			// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
// MH810100(E) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	ulong		WPlace;				// ���Ԉʒu�ް�
// MH810100(E) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	uchar		KakariNo;			// �W����	0�`9999
	uchar		OutKind;			// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
	uchar		CountSet;			// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
	NTNET_CAR_IN_TIM	carInTime;	// ���ɓ���
	uchar			ReceiptIssue;		// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
	uchar			Syubet;				// �������			1�`
	ulong			Price;				// ���ԗ���			0�`
	ulong			CashPrice;			// ��������			0�`
	ulong			InPrice;			// �������z			0�`
	ulong			ChgPrice;			// �ޑK���z			0�`9999
// �s��C��(S) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
	ushort			FrkReturn;			// �U�։ߕ�����(�����߂��z)
// �s��C��(E) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
	uchar			in_coin[5];											// ���했��������(10,50,100,500,1000)
	uchar			out_coin[5];										// ���했�o������(10,50,100,500,1000)
	uchar			f_escrow;			// ���Z�f�[�^�A���Z���f�[�^��1000�~�̕��߂��������Z�b�g����t���O
	ulong			HaraiModoshiFusoku;	// ���ߕs���z	0�`9999
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	ushort			CardFusokuType;		// ���o�s���}�̎��(�����ȊO)
	ushort			CardFusokuTotal;	// ���o�s���z(�����ȊO)
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	uchar			AntiPassCheck;		// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�N�n�e�e/2�������n�e�e
	ulong			ParkNoInPass;		// ������@���ԏꇂ	0�`999999
	uchar			pkno_syu;			// ��������ԏ�m���D���
	ulong			teiki_id;			// �����id
	uchar			teiki_syu;			// ��������
// MH810100(S) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	ushort			MediaKind1;			// ���(�}��)	0�`99
	uchar			MediaCardNo1[30];	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			MediaCardInfo1[16];	// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	ushort			MediaKind2;			// ���(�}��)	0�`99
	uchar			MediaCardNo2[30];	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			MediaCardInfo2[16];	// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
// MH810100(E) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	uchar			e_pay_kind;			// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
	ulong			e_pay_ryo;			// �d�q�}�l�[���ϊz
// MH321800(S) G.So IC�N���W�b�g�Ή�
//	uchar			e_Card_ID[16];		// ����ID (Ascii 16��)
	uchar			e_Card_ID[20];		// ����ID (Ascii 20��)
// MH321800(E) G.So IC�N���W�b�g�Ή�
	ulong			e_pay_after;		// ���ό�Suica�c��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	uchar			e_inquiry_num[16];	// �⍇���ԍ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
	uchar			c_Card_No[20];		// �ڼޯĶ��މ���ԍ�
	ulong			c_pay_ryo;			// �ڼޯĶ��ޗ��p���z
	uchar			c_cct_num[16];		// �ڼޯĶ��ޒ[�����ʔԍ�
	uchar			c_kid_code[6];		// KID����
	ulong			c_app_no;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	uchar			c_trade_no[20];		// �����X����ԍ�
	ulong			c_slipNo;			// �`�[�ԍ�
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	DISCOUNT_DATA	DiscountData[WTIK_USEMAX];						// �����ް�
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	DETAIL_DATA		DetailData[DETAIL_SYU_MAX];						// ���׃f�[�^
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	uchar			e_Transactiontatus;	// ����X�e�[�^�X
	uchar			qr_MchTradeNo[32];	// Mch����ԍ�
	uchar			qr_PayTermID[16];	// �x���[��ID
	uchar			qr_PayKind;			// QR���σu�����h
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
} ntNet_152_saveInf;

typedef union {
	NTNET152_ST_OIBAN	Oiban;		// ���Z�ǔ�(0�`99999)
	uchar		PayMethod;			// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
	uchar		PayMode;			// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
// MH810100(S) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	uchar		CMachineNo;			// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
// MH810100(E) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	ulong		WPlace;				// ���Ԉʒu�ް�
// MH810100(E) K.Onodera 2020/02/05 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	uchar		KakariNo;			// �W����	0�`9999
	uchar		OutKind;			// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
	uchar		CountSet;			// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
	NTNET_CAR_IN_TIM	carInTime;	// ���ɓ���
	uchar			ReceiptIssue;		// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
	uchar			Syubet;				// �������			1�`
	ulong			Price;				// ���ԗ���			0�`
	ulong			CashPrice;			// ��������			0�`
	ulong			InPrice;			// �������z			0�`
	ulong			ChgPrice;			// �ޑK���z			0�`9999
// �s��C��(S) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
	ushort			FrkReturn;			// �U�։ߕ�����(�����߂��z)
// �s��C��(E) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
	uchar			in_coin[5];			// ���했��������(10,50,100,500,1000)
	uchar			out_coin[5];			// ���했�o������(10,50,100,500,1000)
	uchar			f_escrow;			// ���Z�f�[�^�A���Z���f�[�^��1000�~�̕��߂��������Z�b�g����t���O
	ulong			HaraiModoshiFusoku;	// ���ߕs���z	0�`9999
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	ushort			CardFusokuType;		// ���o�s���}�̎��(�����ȊO)
	ushort			CardFusokuTotal;	// ���o�s���z(�����ȊO)
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	uchar			AntiPassCheck;		// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�N�n�e�e/2�������n�e�e
	ulong			ParkNoInPass;		// ������@���ԏꇂ	0�`999999
	uchar			pkno_syu;			// ��������ԏ�m���D���
	ulong			teiki_id;			// �����id
	uchar			teiki_syu;			// ��������
// MH810100(S) K.Onodera 2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	ushort			MediaKind1;			// ���(�}��)	0�`99
	uchar			MediaCardNo1[30];	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			MediaCardInfo1[16];	// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	ushort			MediaKind2;			// ���(�}��)	0�`99
	uchar			MediaCardNo2[30];	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	uchar			MediaCardInfo2[16];	// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
// MH810100(E) K.Onodera 2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	uchar			e_pay_kind;			// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
	ulong			e_pay_ryo;			// �d�q�}�l�[���ϊz
// MH321800(S) G.So IC�N���W�b�g�Ή�
//	uchar			e_Card_ID[16];		// ����ID (Ascii 16��)
	uchar			e_Card_ID[20];		// ����ID (Ascii 20��)
// MH321800(E) G.So IC�N���W�b�g�Ή�
	ulong			e_pay_after;		// ���ό�Suica�c��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	uchar			e_inquiry_num[16];	// �⍇���ԍ�
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
	uchar			c_Card_No[20];		// �ڼޯĶ��މ���ԍ�
	ulong			c_pay_ryo;			// �ڼޯĶ��ޗ��p���z
	uchar			c_cct_num[16];		// �ڼޯĶ��ޒ[�����ʔԍ�
	uchar			c_kid_code[6];		// KID����
	ulong			c_app_no;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	uchar			c_trade_no[20];		// �����X����ԍ�
	ulong			c_slipNo;			// �`�[�ԍ�
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	DISCOUNT_DATA	DiscountData;		// �����ް�
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	DETAIL_DATA		DetailData;			// ���׃f�[�^
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	uchar			e_Transactiontatus;	// ����X�e�[�^�X
	uchar			qr_MchTradeNo[32];	// Mch����ԍ�
	uchar			qr_PayTermID[16];	// �x���[��ID
	uchar			qr_PayKind;			// QR���σu�����h
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
} NTNET_152_U_SAVEINF;

extern ntNet_152_saveInf	ntNet_152_SaveData;

/*--------------------------------------------------------------------------*/
/*	�v���ް���M���ޯ����ߴر												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			ReceiveFlg;										// �v����M�ς��׸�
	ulong			ParkingNo;										// ���ԏꇂ
	ushort			ModelCode;										// �@����
	ulong			MachineNo;										// �@�B��
	uchar			SerialNo[6];									// �[���ره�
	uchar			Year;											// �N
	uchar			Mon;											// ��
	uchar			Day;											// ��
	uchar			Hour;											// ��
	uchar			Min;											// ��
	uchar			Sec;											// �b
} RECEIVE_BACKUP;

extern	RECEIVE_BACKUP	RecvBackUp;									// NE-NET�v���ް���M���ޯ����ߴر

#define	REMOTE_BUFF_SIZE	1800				// ���u�ݒ�ύX��M�ޯ̧����

typedef struct {
	int		prepare;		// =1�F�����w�ߎ�M
	int		bufcnt;			// ���o�b�t�@�ςݐݒ�f�[�^��
	int		bufofs;			// ����M�o�b�t�@�擪
	char	buffer[REMOTE_BUFF_SIZE];	// ��M�o�b�t�@
} NTNET_TMP_BUFFER;

extern	NTNET_TMP_BUFFER tmp_buffer;


extern	DATA_KIND_126	Ntnet_Prev_SData126;						// �O�񑗐M ���K�Ǘ��f�[�^
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
extern	TURI_KAN	turi_kan_bk;									// �O�񃍃O�쐬 ���K�Ǘ��f�[�^
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
// ���S�����R�[���Z���^�[�Ή��Ŏg�p����[���ԒʐM����M�f�[�^
// RECV_NTNET_DT��[���ԒʐM�p�ɒ�`����ƃ��������s�����邽�߁A���S�����R�[���Z���^�[�Ή���
// ��M����f�[�^ID�̂ݒ�`����
typedef	union {
	DATA_BASIC		DataBasic;
	DATA_KIND_109	RData109;
} RECV_NTNET_TERM_DT;

extern RECV_NTNET_TERM_DT	RecvNtnetTermDt;						// �[���ԒʐM�p��M�f�[�^�o�b�t�@
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
extern	int		ntnet_decision_credit(credit_use *p);
extern	ushort	NTNET_Edit_Data22( Receipt_data *p_RcptDat, DATA_KIND_22 *p_NtDat );
extern	ushort	NTNET_Edit_Data22_SK( Receipt_data *p_RcptDat, DATA_KIND_22 *p_NtDat );
extern	void	NTNET_Snd_Data22_FusDel( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind );
extern void	Make_Log_Enter( unsigned short );
extern void	Make_Log_Enter_frs( unsigned short , void * );
// MH810100(S) K.Onodera 2020/01/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//extern unsigned short	NTNET_Edit_Data20( enter_log *, DATA_KIND_20 * );
// MH810100(S) K.Onodera 2020/01/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
extern unsigned short	NTNET_Edit_Data120( Err_log *, DATA_KIND_120 * );
extern unsigned short	NTNET_Edit_Data121( Arm_log *, DATA_KIND_121 * );
extern unsigned short	NTNET_Edit_Data63( Err_log *, DATA_KIND_63 * );
extern unsigned short	NTNET_Edit_Data64( Arm_log *, DATA_KIND_64 * );
extern unsigned short	NTNET_Edit_Data122( Mon_log *, DATA_KIND_122 * );
extern unsigned short	NTNET_Edit_Data123( Ope_log *, DATA_KIND_123 * );
extern unsigned short	NTNET_Edit_Data131( COIN_SYU *, DATA_KIND_130 * );
extern unsigned short	NTNET_Edit_Data133( NOTE_SYU *, DATA_KIND_132 * );
extern void	Make_Log_ParkCarNum( void );
extern unsigned short	NTNET_Edit_Data236( ParkCar_log *, DATA_KIND_236 * );
extern unsigned short	NTNETDOPA_Edit_Data14( LongPark_log *, DATA_KIND_14 * );
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//extern void	Make_Log_MnyMng( ushort );
extern BOOL	Make_Log_MnyMng( ushort );
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
extern unsigned short	NTNET_Edit_Data126( TURI_KAN *, DATA_KIND_126 * );
extern unsigned short	NTNET_Edit_Data135( TURI_KAN *, DATA_KIND_135 * );
extern	void	Make_Log_TGOUKEI( void );
#define	NTNET_Edit_Data30( syu, ntdat )		NTNET_Edit_SyukeiKihon( syu, 30, ntdat )
#define	NTNET_Edit_Data31( syu, ntdat )		NTNET_Edit_SyukeiRyokinMai( syu, 31, ntdat )
#define	NTNET_Edit_Data32( syu, ntdat )		NTNET_Edit_SyukeiBunrui( syu, 32, ntdat )
#define	NTNET_Edit_Data33( syu, ntdat )		NTNET_Edit_SyukeiWaribiki( syu, 33, ntdat )
#define	NTNET_Edit_Data34( syu, ntdat )		NTNET_Edit_SyukeiTeiki( syu, 34, ntdat )
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// #define	NTNET_Edit_Data35( syu, ntdat )		NTNET_Edit_SyukeiShashitsuMai( syu, 35, ntdat )
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
#define	NTNET_Edit_Data36( syu, ntdat )		NTNET_Edit_SyukeiKinsen( syu, 36, ntdat )
#define	NTNET_Edit_Data41( syu, ntdat )		NTNET_Edit_SyukeiSyuryo( syu, 41, ntdat )
#define	NTNET_Edit_Data158( syu, ntdat )	NTNET_Edit_SyukeiKihon( syu, 158, ntdat )
#define	NTNET_Edit_Data159( syu, ntdat )	NTNET_Edit_SyukeiRyokinMai( syu, 159, ntdat )
#define	NTNET_Edit_Data160( syu, ntdat )	NTNET_Edit_SyukeiBunrui( syu, 160, ntdat )
#define	NTNET_Edit_Data161( syu, ntdat )	NTNET_Edit_SyukeiWaribiki( syu, 161, ntdat )
#define	NTNET_Edit_Data162( syu, ntdat )	NTNET_Edit_SyukeiTeiki( syu, 162, ntdat )
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// #define	NTNET_Edit_Data163( syu, ntdat )	NTNET_Edit_SyukeiShashitsuMai( syu, 163, ntdat )
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
#define	NTNET_Edit_Data164( syu, ntdat )	NTNET_Edit_SyukeiKinsen( syu, 164, ntdat )
#define	NTNET_Edit_Data169( syu, ntdat )	NTNET_Edit_SyukeiSyuryo( syu, 169, ntdat )
extern	unsigned short	NTNET_Edit_SyukeiKihon( SYUKEI *, ushort, DATA_KIND_30 * );
extern	unsigned short	NTNET_Edit_SyukeiRyokinMai( SYUKEI *, ushort, DATA_KIND_31 * );
extern	unsigned short	NTNET_Edit_SyukeiBunrui( SYUKEI *, ushort, DATA_KIND_32 * );
extern	unsigned short	NTNET_Edit_SyukeiWaribiki( SYUKEI *, ushort, DATA_KIND_33 * );
extern	unsigned short	NTNET_Edit_SyukeiTeiki( SYUKEI *, ushort, DATA_KIND_34 * );
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// extern	unsigned short	NTNET_Edit_SyukeiShashitsuMai( SYUKEI *, ushort, DATA_KIND_35 * );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
extern	unsigned short	NTNET_Edit_SyukeiKinsen( SYUKEI *, ushort, DATA_KIND_36 * );
extern	unsigned short	NTNET_Edit_SyukeiSyuryo( SYUKEI *, ushort, DATA_KIND_41 * );
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
//extern	unsigned char	NTNET_Edit_isData20_54(enter_log 		*p_RcptDat);	//	���Ƀt�H�[�}�b�g����(20/54)
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
extern	unsigned short	NTNET_Edit_SyukeiKihon_T( SYUKEI *, ushort, DATA_KIND_42_T * );
extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_T( SYUKEI *, ushort, DATA_KIND_43_T * );
extern	unsigned short	NTNET_Edit_SyukeiBunrui_T( SYUKEI *, ushort, DATA_KIND_44_T * );
extern	unsigned short	NTNET_Edit_SyukeiWaribiki_T( SYUKEI *, ushort, DATA_KIND_45_T * );
extern	unsigned short	NTNET_Edit_SyukeiTeiki_T( SYUKEI *, ushort, DATA_KIND_46_T * );
extern	unsigned short	NTNET_Edit_SyukeiKinsen_T( SYUKEI *, ushort, DATA_KIND_48_T * );
extern	unsigned short	NTNET_Edit_SyukeiSyuryo_T( SYUKEI *, ushort, DATA_KIND_53_T * );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

extern	unsigned char	NTNET_Edit_isData22_56(Receipt_data 	*p_RcptDat);	//	���Z�t�H�[�}�b�g����(22/56)
extern	unsigned char	NTNET_Edit_isData236_58(ParkCar_log 	*p_RcptDat);	//	���ԑ䐔�t�H�[�}�b�g����(236/58)
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
////	�V����NT-NET�t�H�[�}�b�g�ݒ�(20̫�ϯĂ�FmtRev, PARKCAR_DATA1��t��)
//extern	unsigned short	NTNET_Edit_Data54(enter_log *p_RcptDat,	DATA_KIND_54 	*p_NtDat );
////	�V����NT-NET�t�H�[�}�b�g�ݒ�(20̫�ϯĂ�FmtRev, PARKCAR_DATA11��t��)
//extern	unsigned short	NTNET_Edit_Data54_r10(enter_log *p_RcptDat,	DATA_KIND_54_r10	*p_NtDat );
//// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//extern	unsigned short	NTNET_Edit_Data54_r13(enter_log *p_RcptDat,	DATA_KIND_54_r13	*p_NtDat );
//// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
extern	unsigned short	NTNET_Edit_Data56_T( Receipt_data *p_RcptDat, DATA_KIND_56_T *p_NtDat );
extern	unsigned short	NTNET_Edit_Data63_T( Err_log *, DATA_KIND_63_T * );
extern	unsigned short	NTNET_Edit_Data64_T( Arm_log *, DATA_KIND_64_T * );
extern unsigned short	NTNET_Edit_Data135_T( TURI_KAN *, DATA_KIND_135_T * );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
////	�V���ZNT-NET�t�H�[�}�b�g�ݒ�(22̫�ϯĂ�FmtRev, PARKCAR_DATA1��t��)	// �Q�� = NTNET_Edit_Data22
//extern	unsigned short	NTNET_Edit_Data56(Receipt_data *p_RcptDat,	DATA_KIND_56 	*p_NtDat );
////	�V���ZNT-NET�t�H�[�}�b�g�ݒ�(22̫�ϯĂ�FmtRev, PARKCAR_DATA1��t��)	// �Q�� = NTNET_Edit_Data22_SK
//extern	unsigned short	NTNET_Edit_Data56_SK(Receipt_data 	*p_RcptDat,	DATA_KIND_56 	*p_NtDat );
////	�V���ZNT-NET�t�H�[�}�b�g�ݒ�(22̫�ϯĂ�FmtRev, PARKCAR_DATA11��t��)	// �Q�� = NTNET_Edit_Data22
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//extern	unsigned short	NTNET_Edit_Data56_r10(Receipt_data *p_RcptDat,	DATA_KIND_56_r10	*p_NtDat );
//// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//extern	unsigned short	NTNET_Edit_Data56_r14( Receipt_data *p_RcptDat, DATA_KIND_56_r14 *p_NtDat );
//// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//extern	unsigned short	NTNET_Edit_Data56_r17( Receipt_data *p_RcptDat, DATA_KIND_56_r17 *p_NtDat );
extern	unsigned short	NTNET_Edit_Data56_rXX( Receipt_data *p_RcptDat, DATA_KIND_56_rXX *p_NtDat );
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
//	WEB�p���ԑ䐔�pNT-NET�t�H�[�}�b�g�ݒ�(236̫�ϯĂƂ͕ʂ�58̫�ϯĂ��쐬)	// �Q�� = NTNET_Edit_Data236
extern	unsigned short	NTNET_Edit_Data58(ParkCar_log		*p_RcptDat,	DATA_KIND_58 	*p_NtDat );
//	WEB�p���ԑ䐔�pNT-NET�t�H�[�}�b�g�ݒ�(236̫�ϯĂƂ͕ʂ�58_r10̫�ϯĂ��쐬)	// �Q�� = NTNET_Edit_Data236
extern	unsigned short	NTNET_Edit_Data58_r10(ParkCar_log	*p_RcptDat,	DATA_KIND_58_r10	*p_NtDat_r10 );
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//extern	unsigned short	NTNET_Edit_SyukeiKihon_r10( SYUKEI *syukei, ushort Type, DATA_KIND_42 *p_NtDat );
//// �d�l�ύX(S) K.Onodera 2016/11/04 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//extern	unsigned short	NTNET_Edit_SyukeiKihon_r13( SYUKEI *syukei, ushort Type, DATA_KIND_42_r13 *p_NtDat );
//// �d�l�ύX(E) K.Onodera 2016/11/04 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
////extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_r10( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat );
////extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_r10( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat );
////extern	unsigned short	NTNET_Edit_SyukeiWaribiki_r10( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat );
////extern	unsigned short	NTNET_Edit_SyukeiTeiki_r10( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat );
////extern	unsigned short	NTNET_Edit_SyukeiSyuryo_r10( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat );
//extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_r13( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat );
//extern	unsigned short	NTNET_Edit_SyukeiWaribiki_r13( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat );
//extern	unsigned short	NTNET_Edit_SyukeiTeiki_r13( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat );
//extern	unsigned short	NTNET_Edit_SyukeiSyuryo_r13( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat );
//// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
extern	unsigned short	NTNET_Edit_SyukeiKihon_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_42 *p_NtDat );
extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat );
extern	unsigned short	NTNET_Edit_SyukeiWaribiki_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat );
extern	unsigned short	NTNET_Edit_SyukeiTeiki_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat );
extern	unsigned short	NTNET_Edit_SyukeiSyuryo_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat );
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
extern	void	NTNET_Data152Save(void *saveData, uchar saveDataCategory);
extern	void NTNET_Data152_DiscDataSave(void *saveData, uchar saveDataCategory, uchar saveIndex);
extern	void NTNET_Data152_DiscDataClear(void);
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
extern	void NTNET_Data152_DetailDataSave(void *saveData, uchar saveDataCategory, uchar saveIndex);
extern	void NTNET_Data152_DetailDataClear(void);
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
extern	void NTNET_Data152_SaveDataClear(void);
extern	void	NTNET_CtrlRecvData( void );
extern	void NTNET_Data152_SaveDataUpdate(void);
uchar	NTNET_Snd_Data136_01( uchar ReSend );
uchar	NTNET_Snd_Data136_03( void );
uchar	NTNET_Snd_Data136_05( uchar ReSend );
uchar	NTNET_Snd_Data136_07( uchar ReSend );
uchar	NTNET_Snd_Data136_09( uchar ReSend );
extern	void NTNET_CtrlRecvData_DL( void );
extern	void NTNET_RevData111_DL(void);
extern	void NTNET_Snd_Data118_DL(t_ProgDlReq *pDlReq);
extern	unsigned short NTNET_Edit_Data125_DL(Rmon_log *p_RcptDat, DATA_KIND_125_DL *p_NtDat);
// MH322914(S) K.Onodera 2016/09/16 AI-V�Ή��F�U�֐��Z
extern	Receipt_data* GetFurikaeSrcReciptData( void );
// MH322914(E) K.Onodera 2016/09/16 AI-V�Ή��F�U�֐��Z
// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
extern void SetVehicleCountDate( ulong val );
extern ulong GetVehicleCountDate( void );
extern void AddVehicleCountSeqNo( void );
extern void ClrVehicleCountSeqNo( void );
extern ushort GetVehicleCountSeqNo( void );
// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
extern void Make_Log_LongParking_Pweb( unsigned long pr_lokno, unsigned short time, uchar knd, uchar knd2);
extern unsigned short NTNET_Edit_Data61( LongPark_log_Pweb *p_RcptDat, DATA_KIND_61_r10 *p_NtDat );
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
#endif	/* ___NTNET_DEFH___ */
