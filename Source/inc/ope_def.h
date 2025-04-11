#ifndef	_OPE_DEF_H_
#define	_OPE_DEF_H_
/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ���������ʒ�`                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-02-01                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	"common.h"
#include	"pri_def.h"
#include	"rkn_cal.h"

/*** structure ***/

/*------------------*/
/* ���ڰ��݊֘A��` */
/*------------------*/

typedef union{
	uchar	BYTE;											// Byte
	struct{
		uchar	MC10_EXEC:1;								// Bit 7 = 1:mc10()���{�v������
		uchar	YOBI2:1;									// Bit 6 = �\��
		uchar	MNTTIMCHG:1;								// Bit 5 = ����ݽ���ݎ����ύX
		uchar	TEIKIEXTIM:1;								// Bit 4 = ������o�Ɏ����ް�
		uchar	TEIKITHUSI:1;								// Bit 3 = ��������Z���~�ݒ��ް�
		uchar	KENKIGEN:1;									// Bit 2 = �������ݒ��ް�
		uchar	SPLDATE:1;									// Bit 1 = ���ʓ��ݒ��ް�
		uchar	USERSET:1;									// Bit 0 = հ���ݒ��ް�
	} BIT;
} t_COM_UPDATE_DATA;

#define	f_NTNET_RCV_MC10_EXEC	(OPECTL.NTNET_RCV_DATA.BIT.MC10_EXEC)	// mc10()���{�v�������׸�
#define	f_NTNET_RCV_MNTTIMCHG	(OPECTL.NTNET_RCV_DATA.BIT.MNTTIMCHG)	// ����ݽ���ݎ����ύX�׸�
#define	f_NTNET_RCV_KENKIGEN	(OPECTL.NTNET_RCV_DATA.BIT.KENKIGEN)	// �������ݒ��ް��X�V�׸�
#define	f_NTNET_RCV_SPLDATE		(OPECTL.NTNET_RCV_DATA.BIT.SPLDATE)		// ���ʓ��ݒ��ް��X�V�׸�
#define	f_NTNET_RCV_USERSET		(OPECTL.NTNET_RCV_DATA.BIT.USERSET)		// հ���ݒ��ް��X�V�׸�
#define	f_NTNET_RCV_TEIKITHUSI	(OPECTL.NTNET_RCV_DATA.BIT.TEIKITHUSI)	// ��������Z���~�ݒ��ް��X�V�׸�
#define	f_NTNET_RCV_TEIKIEXTIM	(OPECTL.NTNET_RCV_DATA.BIT.TEIKIEXTIM)	// ������o�Ɏ����ް��X�V�׸�


/*** ���ڰ��ݐ���֘A��` ***/
struct	OPECTL_rec {
	char	Mnt_mod;										// 0:�ʏ�(�ҋ@,���Z)
															// 1:�߽ܰ��,�C��,�߽ܰ�ޖY��̑I��
															// 2:�W��,�Ǘ�������ݽ
															// 3:�Z�p������ݽ
															// 4:��[��������ݽӰ��
															// 5:�H�ꌟ��Ӱ��
	char	Mnt_lev;										//  1:����ݽ����1(�W������)
															//  2:����ݽ����2(�Ǘ��ґ���)
															//  3:����ݽ����3(�Z�p������)
															// -1:����ݽ�����߽ܰ�ޖ���
	uchar	Kakari_Num;										// ����ݽ�W���ԍ�(0=�s���A1�`10=�߽ܰ�ޓ��͂����W���ԍ��A99=��ɋZ�p��)
	uchar	Ope_mod;										// ���ڰ��ݏ�ԇ� OpeMain()�Q��
	uchar	Pay_mod;										// 0:�ʏ퐸�Z
															// 1:�߽ܰ�ޖY�ꐸ�Z
															// 2:�C�����Z
	uchar	Comd_knd;										// ү���޺���ގ��
	uchar	Comd_cod;										// ү���޺���޺���
	char	on_off;											// 0:KEY OFF
															// 1:KEY ON
	ulong	Op_LokNo;										// �ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)
	ushort	Pr_LokNo;										// ���������p���Ԉʒu�ԍ�(1�`324)
	ulong	MOp_LokNo;										// �ԈႦ���ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)
	ushort	MPr_LokNo;										// �ԈႦ�����������p���Ԉʒu�ԍ�(1�`324)
	char	op_faz;											// ���ڰ���̪���
															// 0:�������M
															// 1:������(�����L�莞�ɾ��)
															// 2:���Z����(�����s���M�㾯�)
															// 3:���~��(�����ɓ����s���M�㾯�
															// 4:��ѱ��(��ѱ�Ă������s���M�㾯�
															// 8:�d�q�}�̒�~�҂����킹̪��ށi�ڍׂ�STOP_REASON�Ō����j
															// 9:���Z���~����Suica��~�҂����킹̪���(����ɒ�~���Ȃ������ꍇ�p)
															//10:Edy���ώ���Suica��~�����҂�̪���
	char	RECI_SW;										//  0:�̎������ݖ��g�p
															//  1:�̎������ݎg�p
															// -1:�ҋ@���̎������݉�
	char	CAN_SW;											// 0:���~����OFF
															// 1:���~����ON
	uchar	opncls_eigyo;									// �����c�ƐM�� 0:�V�O�i��OFF  1:�V�O�i��ON
	uchar	opncls_kyugyo;									// �����x�ƐM�� 0:�V�O�i��OFF  1:�V�O�i��ON
	uchar	CN_QSIG;										// ���ү������
	uchar	NT_QSIG;										// ����ذ�ް�����
	char	Ope_Mnt_flg;									// ���ڰ�������ݽ�׸�
															//  1:����������
															//  2:LCD����
															//  3:LED����
															//  4:����ذ�ް����
															//  5:���ү�����
															//  6:SW����
															//  7:���������
															//  8:���o�͐M������
															//  9:���������
															// 10:���Cذ�ް����
															// 11:�ųݽ����
															// 12:��ذ����
															// 13:IF������
															// 14:ARCNET����
															// 15:NTNET����
	char	Ope_err;										// ���ڰ��ݴװ�׸�
	char	coin_syukei;									// ��݋��ɏW�v�׸�
	char	note_syukei;									// �������ɏW�v�׸�
	char	nyukin_flg;										// ��ݎ��������׸�
	char	cnsend_flg;										// ��ݎ����������M�ς��׸�
	char	flp_recover;									// ���ɏ�񕜋A�׸�
	short	Fin_mod;										// 1:���Z�����ނ薳��
															// 2:���Z�����ނ�L��
															// 3:ү��װ����
	uchar	LastCard;										// �Ō�Ɏg�p���ꂽ�J�[�h
															// (OPE_LAST_RCARD_NONE/OPE_LAST_RCARD_MAG/OPE_LAST_RCARD_MIF)
	short	LastCardInfo;									// ���ǂݎ����(���ް��\���p)
	short	CR_ERR_DSP;										// ���װ���
	short	MIF_CR_ERR;										// Mifare���װ
	T_FrmEnd	PriEndMsg[2];								// �v�����^�ُ펞�\���p
	uchar	Pri_Result;										// �󎚏I�����̈󎚌���	�i�󎚏I��ү���ށFBMode���i�[�j
	uchar	Pri_Kind;										// �󎚂��I�����������	�i�󎚏I��ү���ށFBPrikind���i�[�j
	char	NtnetTimRec;									// NT-NET���v��M�׸�
	uchar	Seisan_Chk_mod;									// ���Z����Ӱ�ށiOFF:�ʏ�^ON:���Z����Ӱ�ށj
	uchar	Ent_Key_Sts;									// �o�^��������ԁiOFF�^ON�j
	uchar	Can_Key_Sts;									// �����������ԁiOFF�^ON�j
	uchar	EntCan_Sts;										// �o�^�������������������ԁiOFF�^ON�j
	uchar	PriUsed;										// <>0:����߲�޶��ނ��g�p
	ulong	LastUsePCardBal;								// �Ō�ɗ��p��������߲�޶��ނ̎c�z
	ulong	multi_lk;										// ������Z�⍇�����Ԏ���
	uchar	Apass_off_seisan;								// 1=��������߽OFF�ݒ�i������p���j
	short	ChkPassSyu;										// ������⍇��������0=�⍇���Ȃ�,1=APS�����,2=Mifare,0xfe=�ʐM�s�ǎ�,0xff=�⍇����ѱ��
	ulong	ChkPassPkno;									// ������⍇�������ԏꇂ
	ulong	ChkPassID;										// ������⍇���������ID
	short	PassNearEnd;									// 1=�����؂�ԋ�
	uchar	f_KanSyuu_Cmp;									// 1=�����W�v���{�ς�
	ushort	op_mod02_dispCnt[3];							// ������ʂł̗��p�\�}�̐ؑ֕\���p�J�E���^
	t_COM_UPDATE_DATA	NTNET_RCV_DATA;						// �ʐM���ް��X�V�������
	uchar	InquiryFlg;		// �O���Ɖ�t���O
	uchar	credit_in_mony;									// �ڼޯĂ���Ⴂ��������
	uchar	other_machine_card;								// ���̐��Z�@�̶���
	uchar	InLagTimeMode;									// ���O�^�C�������������s
	char	PasswordLevel;									// �p�X���[�h or �W���J�[�h���x�� 0�`4:���x��, -1:�p�X���[�h���ݒ�
	uchar	f_DoorSts;										// �h�A��ԁi0:�A1:�J�j
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
	uchar	CalStartTrigger;								// �����v�Z�ضް
															// 0x14: �������Z(����)�ɂ�鐸�Z	�i�������Z�j
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
	uchar	holdPayoutForEcStop;							// ���o�ۗ��t���O
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2019/12/09 �Ԕԃ`�P�b�g���X(�N��������)
	uchar	init_sts;										// �N���X�e�[�^�X
	char	sended_opcls;									// ���M�ς݂̉c�x�Ə��
	uchar	remote_wait_flg;								// ���u���Z�v����̉����҂��t���O
// MH810100(E) K.Onodera 2019/12/09 �Ԕԃ`�P�b�g���X(�N��������)
// MH810100(S) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
	uchar	chg_idle_disp_rec;								// �ҋ@��ʒʒm��M�t���O
// MH810100(E) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
	uchar	lcd_prm_update;									// LCD�ւ̃p�����[�^�A�b�v���[�h��
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
// MH810103(s) �d�q�}�l�[�Ή� �ҋ@��ʂł̎c���Ɖ��
	uchar	lcd_query_onoff;								// �ҋ@��ʂł̎c���Ɖ��(0:NG/1:OK)
// MH810103(e) �d�q�}�l�[�Ή� �ҋ@��ʂł̎c���Ɖ��
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	uchar	EJAClkSetReq;									// �d�q�W���[�i���ւ̎��v�ݒ�v��
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810104(S) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�
	uchar	RT_show_attention;								// �����v�Z�e�X�g�̒��Ӊ�ʕ\��(0:�\�����Ȃ�/1:�\������)
// MH810104(E) R.Endo 2021/09/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6031 �����v�Z�e�X�g�̒��Ӊ�ʒǉ�
// MH810105(S) MH364301 �C���{�C�X�Ή�
	uchar	f_DelayRyoIsu;									// ���V�[�g�󎚒x���t���O 1:�W���[�i���󎚊����҂��A2:�W���[�i���󎚊���
	uchar	f_ReIsuType;									// �Ĕ��s��� 1:PWeb, 2:PIP
	uchar	f_RctErrDisp;									// �̎��ؔ��s���s�\���t���O
	uchar	f_CrErrDisp;									// ���G���[�\�����t���O 0:��\��, 1:�\����
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	ushort	f_searchtype;									// �����^�C�v�t���O 0:�ԔԌ���,1:��������,2:QR����
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
// GG129000(S) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
	uchar	f_req_paystop;									// LCD����̐��Z���~�v��(0:�Ƃ肯���{�^���A1:�߂�{�^��)
// GG129000(E) M.Fujikawa 2023/09/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���P�A��No.52
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
	uchar	f_rtm_remote_pay_flg;							// ���u���Z�i���A���^�C���j�J�n��t�t���O
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	uchar	f_eReceiptReserve;								// �d�q�̎��ؗ\��t���O
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
};
extern	struct OPECTL_rec	OPECTL;

// MH322914(S) K.Onodera 2016/10/05 AI-V�Ή�
#define		CAL_TRIGGER_REMOTECALC_TIME		0x55			// ���u���Z(���Ɏ����w��)
#define		CAL_TRIGGER_REMOTECALC_FEE		0x56			// ���u���Z(���z�w��)
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�)_for_LCD_IN_CAR_INFO_op_mod00))
// ���Z�Ƃ��Ă͓��Ɏ����w��Œ�Ő��Z���s��
#define		CAL_TRIGGER_LCD_IN_TIME			0x65			// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�)_for_LCD_IN_CAR_INFO_op_mod00))
// MH322914(E) K.Onodera 2016/10/05 AI-V�Ή�

typedef struct {
	ulong			op_lokno;								// �ڋq�p���Ԉʒu�ԍ�(���2��A�`Z,����4��1�`9999,�v6��)
	ushort			pr_lokno;								// ���������p���Ԉʒu�ԍ�(1�`324)
	uchar			kind;									// �������(�Ԏ�)
	uchar			resv;
	ulong			fee;									// �����v�Z����
	ushort			nstat;									// ���݃X�e�[�^�X
	struct clk_rec	indate;									// ���ɓ���
	struct clk_rec	outdate;								// �o�ɓ���
} RYO_INFO;

typedef struct {
	ushort			GroupNo;								// �O���[�vNo
	RYO_INFO		RyoInfo[OLD_LOCK_MAX];					// �������
} RYOCALSIM_CTRL;
extern	RYOCALSIM_CTRL		RyoCalSim;

// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
#pragma pack
typedef struct{
	uchar			ReqKind;
	uchar			OpenClose;				// �c�x�Ə�
	uchar			MntMode;				// ���Z�@���샂�[�h��(0=�ʏ�/1=�����e)
	uchar			ErrOccur;				// �G���[������(0=�Ȃ�/1=����)
	uchar			AlmOccur;				// �A���[��������(0=�Ȃ�/1=����)
	ushort			PayState;				// ���Z�������
	ushort			PassUse;				// �U�֌�������p�L��
	ushort			SrcArea;				// �U�֌����
	ulong			SrcNo;					// �U�֌��Ԏ�
	date_time_rec2	SrcPayTime;				// �U�֌����Z�N���������b
	date_time_rec2	SrcInTime;				// �U�֌����ɔN���������b
	ushort			SrcFeeKind;				// �U�֌��������
	ulong			SrcFee;					// �U�֌����ԗ���
	ulong			SrcFeeDiscount;			// �U�֌��������z
	ushort			SrcStatus;				// �U�֌��X�e�[�^�X
	ushort			DestArea;				// �U�֐���
	ulong			DestNo;					// �U�֐�Ԏ�
	date_time_rec2	DestInTime;				// �U�֐���ɔN���������b
	date_time_rec2	DestPayTime;			// �U�֐搸�Z�N���������b
	ushort			DestFeeKind;			// �U�֐旿�����
	ulong			DestFee;				// �U�֐撓�ԗ���
	ulong			DestFeeDiscount;		// �U�֐抄�����z
	ushort			DestStatus;				// �U�֐�X�e�[�^�X
	ulong			PassParkingNo;			// ��������ԏ�ԍ�
	ulong			PassID;					// �����ID
	ushort			PassKind;				// ��������
	long			Remain;					// �x���c�z
}PIP_FURIKAE_INFO;
typedef struct{
	uchar			ReqKind;
	ushort			Area;					// ���
	ulong			ulNo;					// �Ԏ��ԍ�
	date_time_rec2	InTime;					// ���ɔN���������b
	date_time_rec2	OutTime;				// �����v�Z�N���������b
	ulong			ulPno;					// ���ԏꇂ
	ushort			RyoSyu;					// �������
	ulong			Price;					// ���ԏ��
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ulong			FutureFee;				// ������Z�\��z
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ushort			DiscountKind;			// �������
	ushort			DiscountType;			// �����敪
	ushort			DiscountCnt;			// �����g�p����
	ulong			Discount;				// �������z
	ulong			DiscountInfo1;			// �������P
	ulong			DiscountInfo2;			// �������Q
}PIP_REMOTE_TIME;
typedef struct{
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	uchar			ReqKind;
	uchar			Type;					// ���u���Z���
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ushort			Area;					// ���
	ulong			ulNo;					// �Ԏ��ԍ�
	ulong			Price;					// ���ԗ���			0�`
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ulong			FutureFee;				// ������Z�\��z
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
	ushort			RyoSyu;					// �������
	ushort			DiscountKind;			// �������
	ushort			DiscountType;			// �����敪
	ushort			DiscountCnt;			// �����g�p����
	ulong			Discount;				// �������z
	ulong			DiscountInfo1;			// �������P
	ulong			DiscountInfo2;			// �������Q
}PIP_REMOTE_FEE;

typedef union {
	PIP_FURIKAE_INFO	stFurikaeInfo;						// �U�֑Ώۏ��
	PIP_REMOTE_TIME		stRemoteTime;						// ���u���Z�����w��
	PIP_REMOTE_FEE		stRemoteFee;						// ���u���Z���z�w��
} PIP_CTRL;
extern	PIP_CTRL		g_PipCtrl;
#pragma unpack
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�

enum {
	OPE_LAST_RCARD_NONE = 0,
	OPE_LAST_RCARD_MAG,		// ���C�J�[�h
};

/*** ���������֘A��` ***/
typedef union{
	uchar	BYTE;											// Byte
	struct{
		uchar	YOBI2:2;									// Bit 6,7 = �\��
		uchar	YOBI1:2;									// Bit 4,5 = �\��
		uchar	COIN:2;										// Bit 2,3 = ��ݓ��������������ظ���(0=ظ��Ė���,1=�������,2=������J)
		uchar	READ:2;										// Bit 0,1 = ���Cذ�ް���������ظ���(0=ظ��Ė���,1=�������,2=������J)
	} BIT;
} SHT_RQ;

extern	SHT_RQ			SHT_REQ;							// ���������
extern	SHT_RQ			SHT_CTRL;							// ��������쒆�׸�
extern	char			READ_SHT_flg;						// 0=����,1=�������,2=������J
extern	char			COIN_SHT_flg;						// 0=����,1=�������,2=������J

extern	uchar	Kakari_Numu[LOCK_MAX];											// ����ݽ�W���ԍ�(0=�s���A1�`10=�߽ܰ�ޓ��͂����W���ԍ��A99=��ɋZ�p��)
extern	uchar	Flp_LagExtCnt[LOCK_MAX];					/* ���O�^�C��������			*/

#define RECODE_SIZE		(0x1000-6)	// �ꎟ�i�[�T�C�Y/����(FLT_LOGOFS_RECORD=5)
#define RECODE_SIZE2	(0x2000-6)	// �ꎟ�i�[�T�C�Y/����(FLT_LOGOFS_RECORD=5)

/*** ۸ފ֘A��` ***/
enum {	// ���O���
	eLOG_PAYMENT = 0,		// ���Z
// MH810100(S) K.Onodera 2019/12/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	eLOG_ENTER,				// ����
	eLOG_RTPAY,				// RT���Z�f�[�^
// MH810100(E) K.Onodera 2019/12/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	eLOG_TTOTAL,			// �W�v
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	eLOG_LCKTTL,			// �Ԏ����W�v
	eLOG_RTRECEIPT,			// RT�̎��؃f�[�^
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	eLOG_ERROR,				// �G���[
	eLOG_ALARM,				// �A���[��
	eLOG_OPERATE,			// ����
	eLOG_MONITOR,			// ���j�^
	eLOG_ABNORMAL,			// �s�������o��
	eLOG_MONEYMANAGE,		// �ޑK�Ǘ�
	eLOG_PARKING,			// ���ԑ䐔�f�[�^
	eLOG_NGLOG,				// �s�����
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//	eLOG_IOLOG,				// ���o�ɗ���
	eLOG_DC_QR,				// DC-NET QR�m��E����f�[�^
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
	eLOG_CREUSE,			// �N���W�b�g���p
	eLOG_iDUSE,				// ID���p
	eLOG_HOJIN_USE,			// �@�l�J�[�h���p
	eLOG_REMOTE_SET,		// ���u�����ݒ�
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//	eLOG_LONGPARK,			// ��������
	eLOG_LONGPARK_PWEB,		// ��������(ParkingWeb�p)
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	eLOG_RISMEVENT,			// RISM�C�x���g
	eLOG_DC_LANE,			// DC-NET ���[�����j�^�f�[�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	eLOG_GTTOTAL,			// GT���v
	eLOG_REMOTE_MONITOR,	// ���u�Ď�
	eLOG_COINBOX,			// �R�C�����ɏW�v(ram�̂�)
	eLOG_NOTEBOX,			// �������ɏW�v(ram�̂�)
	eLOG_EDYARM,			// Edy�A���[��(ram�̂�)
	eLOG_EDYSHIME,			// Edy����(ram�̂�)
	eLOG_POWERON,			// ���d(ram�̂�)
	eLOG_MNYMNG_SRAM,		// ���K�Ǘ�(SRAM)
	eLOG_MAX				// terminator
};

enum {	// ���O�^�[�Q�b�g���
	eLOG_TARGET_NTNET = 0,	// NT-NET
	eLOG_TARGET_RISM,		// RISM
	eLOG_TARGET_PARKIPRO,	// Park i pro
	eLOG_TARGET_REMOTE,		// ���uNT-NET
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	eLOG_TARGET_YOBI,		// �\��
	eLOG_TARGET_LCD,		// LCD�ʐM
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	eLOG_TARGET_MAX			// terminator
};

typedef	struct {						// �j�A�t����ԊǗ�
	ushort	NearFullMaximum;			// �j�A�t���A���[������l(����)
	ushort	NearFullMinimum;			// �j�A�t���A���[�������l(����)
	ushort	NearFullStatus;				// �j�A�t���������
	ushort	NearFullStatusBefore;		// �j�A�t��������ԁi�O��j
} near_full;

// MH322917(S) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
// ���ӎ����Flog_record�\���̂�RAM��ɕێ����Ă��郍�O�̊Ǘ��̈�̂��߃����o��ύX���Ȃ�����
// �ύX���K�v�ȏꍇ�̓������z�u������邽�߃o�[�W�����A�b�v�Ń��O�����O�o�[�W�����uLOG_VERSION�v���A�b�v�f�[�g���A
// opetask()�̒���LOG_VERSION�ɂ������S���O�N���A������ǉ����邱��
// MH322917(E) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
struct log_record {	// �������R�[�h
	short	kind;						// ���(0/1��)
	short	count[2];					// ����(RAM)
	ulong	wtp;						// ײ��߲��(RAM)
	union{								// �ޯ̧(RAM)
		uchar	s1[2][RECODE_SIZE];
		uchar	s2[RECODE_SIZE2];
	} dat;
	short	Fcount;						// FLASH�����ݍςݾ������
	short	Fwtp;						// FLASH�����ݾ����
	ushort	unread[eLOG_TARGET_MAX];	// ���L����Q��
	ushort	f_unread[eLOG_TARGET_MAX];	// ���ǃ��R�[�h��(RAM�݂̂Ƀf�[�^�����ꍇ�g�p)
	ushort	void_read[eLOG_TARGET_MAX];	// ��ǂ݃��R�[�h����
	near_full	nearFull[eLOG_TARGET_MAX];	// �j�A�t�����
	ushort	overWriteUnreadCount[eLOG_TARGET_MAX];	// �o�b�t�@�t�������ɂ��FROM�����ݎ��ɏ㏑�����������M�f�[�^��
	uchar	writeLogFlag[eLOG_TARGET_MAX];	// ���O�����݃t���O
	uchar	writeFullFlag[eLOG_TARGET_MAX];	// �f�[�^�t��(FROM�Z�N�^������)�t���O
};
extern struct log_record	LOG_DAT[eLOG_MAX];
// MH322917(S) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
// ���ӎ����Flog_recover�\���̂̓��O�������݂̒�d���J�o���p�G���A�̂��߃����o��ύX���Ȃ�����
// (���O�������ݒ��Ƀo�[�W�����A�b�v�𔺂���d����ƃf�[�^�����邽��)
// �ύX���K�v�ȏꍇ�̓������z�u������邽�߃o�[�W�����A�b�v�Ń��O�����O�o�[�W�����uLOG_VERSION�v���A�b�v�f�[�g���A
// opetask()�̒���LOG_VERSION�ɂ������S���O�N���A������ǉ����邱��
// MH322917(E) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
struct log_recover {// ���O�Ǘ����d�����̍\����
	ushort	f_recover;		// ���݂̏������
	short	Lno;			// LOG���
	short	stat_kind;		// ���(0/1��)
	short	stat_count;		//  ����(RAM)
	ulong	stat_wtp;		// ײ��߲��(RAM)
	void	*dat_p;			// LOG�f�[�^�̃|�C���^
	BOOL	strage;			// SRAM�t���O
	ushort	f_unread[eLOG_TARGET_MAX];// ���ǃ��R�[�h��(RAM�݂̂Ƀf�[�^�����ꍇ�g�p)
	uchar	dummy[20];		// �\���̈�
};
extern struct log_recover	log_bakup;// ���O�Ǘ����d�����̑ޔ��G���A
extern	const ushort LogDatMax[][2];
#define LOG_SECORNUM(x)	(LogDatMax[(x)][1]/LogDatMax[(x)][0])

enum {	// ���O���
	LOG_PAYMENT = 0,											// �ʐ��Z
	LOG_PAYSTOP,												// ���Z���~
	LOG_ABNORMAL,												// �s���E�����o��
	LOG_TTOTAL,													// �s���v
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	LOG_LCKTTL,													// �Ԏ����W�v
	LOG_RTRECEIPT,												// RT�̎��؃f�[�^
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	LOG_MONEYMANAGE,											// ���K�Ǘ�
	LOG_COINBOX,												// COIN���ɏW�v
	LOG_NOTEBOX,												// �������ɏW�v
	LOG_POWERON,												// �╜�d
	LOG_ERROR,													// �װ
	LOG_ALARM,													// �װ�
	LOG_OPERATE,												// ���엚��
	LOG_MONITOR,												// ���j�^
	LOG_CREUSE,													// �N���W�b�g���p���׃��O
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	LOG_EDYARM,													// Edy�A���[��������O
//	LOG_EDYSHIME,												// Edy���ߋL�^���O
//	LOG_EDYMEISAI,												// �J�[�h���Z(�d����)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	LOG_SCAMEISAI,												// �J�[�h���Z(�r��������)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	LOG_EDYSYUUKEI,												// �J�[�h���Z(�d����)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	LOG_SCASYUUKEI,												// �J�[�h���Z(�r��������)
	LOG_NGLOG,													// �s�����O
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή��j
//	LOG_IOLOG,													// ���o�Ƀ��O
	LOG_DC_QR,													// QR�m��E����f�[�^
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή��j
	LOG_HOJIN_USE,												// �@�l�J�[�h���p
	LOG_REMOTE_SET,												// ���u�����ݒ�
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	LOG_ENTER,													// ����
	LOG_RTPAY,													// RT���Z�f�[�^
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	LOG_PARKING,												// ���ԑ䐔�f�[�^
	LOG_LONGPARK,												// ��������
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	LOG_RISMEVENT,												// RISM�C�x���g
	LOG_DC_LANE,												// ���[�����j�^�f�[�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	LOG_MONEYMANAGE_NT,											// ���K�Ǘ��f�[�^(NT-NET)
	LOG_MNYMNG_SRAM,											// ���K�Ǘ�(SRAM)
	LOG_RTPAYMENT,												// �����e�X�g
	LOG_GTTOTAL,												// GT���v
	LOG_REMOTE_MONITOR,											// ���u�Ď�
// MH321800(S) G.So IC�N���W�b�g�Ή�
	LOG_ECSYUUKEI,												// �J�[�h���Z(���σ��[�_)
	LOG_ECMEISAI,												// �J�[�h���Z(���σ��[�_)
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	LOG_ECMINASHI,												// �݂Ȃ�����(���σ��[�_)
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// MH810105(S) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
	LOG_PAYSTOP_FU,												// ���d���̒��~�f�[�^
// MH810105(E) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
	LOG_DEFINE_MAX												// terminator
};

union log_rec {												// �o�^�pܰ��ر
	Pon_log			pon;
	Err_log			err;
	Arm_log			arm;
	Ope_log			ope;
	flp_log			flp;
//	Pay_log			pay;
//	Can_log			can;
	Mny_log			mny;
//	Fryo_log		Fryo;
	Receipt_data	receipt;
};

extern	union log_rec	logwork;

struct logwork_rec {
	short		wor_count;									// ����ܰ�
	short		wor_wtp;									// ײ��߲��ܰ�
};
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// �o�[�R�[�h����
typedef enum {
	RESULT_NO_ERROR,						// �g�p�\�ȃo�[�R�[�h
	RESULT_QR_INQUIRYING,					// �⍇����
	RESULT_DISCOUNT_TIME_MAX,				// �������
	RESULT_BAR_USED,						// �g�p�ς̃o�[�R�[�h
	RESULT_BAR_EXPIRED,						// �����؂�o�[�R�[�h
	RESULT_BAR_READ_MAX,					// �o�[�R�[�h�������
	RESULT_BAR_FORMAT_ERR,					// �t�H�[�}�b�g�G���[
	RESULT_BAR_ID_ERR,						// �ΏۊO
// MH810100(S) 2020/06/19 #4503�y�A���]���w�E�����z�T�[�r�X��QR�K�p��̎�ʐؑւ̔F�؎���f�[�^�����M����Ă��Ȃ�
	RESULT_BAR_NOT_USE_CHANGEKIND,			// �����E�����ς݌�̎Ԏ�؊�
// MH810100(E) 2020/06/19 #4503�y�A���]���w�E�����z�T�[�r�X��QR�K�p��̎�ʐؑւ̔F�؎���f�[�^�����M����Ă��Ȃ�
} OPE_RESULT;

// QR�m��E��������敪
typedef enum {
	CERTIF_COMMIT,							// �m��
	CERTIF_ROLLBACK,						// ���
} eCertifKind;
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)

/*** ������⍇�����ʒ�` ***/
typedef	struct {
	// NTNET��DATA_KIND_143�Ɠ��l�Ƃ��鎖�B
	ushort			Reserve;								// �\��
	ulong			ParkingNo;								// ���ԏꇂ
	ulong			PassID;									// �����ID
	uchar			PassState;								// ������ð��
	uchar			RenewalState;							// �X�V�ð��
	date_time_rec	OutTime;								// �o�ɔN��������
	ulong			UseParkingNo;							// ���p�����ԏꇂ
} PASSCHK;

extern	PASSCHK		PassChk;								// ������⍇�������ް�
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// �Z���^�[�₢���킹�����̒�����`�F�b�N����
typedef enum{
	SEASON_CHK_OK,					// �G���[�Ȃ�
	SEASON_CHK_INVALID_PARKINGLOT,	// ���ԏꇂ�G���[
	SEASON_CHK_INVALID_PASS_ID,		// ���ID�G���[
	SEASON_CHK_NO_MASTER_PARKINGLOT,// �e�@�̒��ԏꇂ��v�Ȃ��G���[
	SEASON_CHK_INVALID_SEASONDATA,	// ��������G���[
	SEASON_CHK_FIRST_NG,			// �������G���[
	SEASON_CHK_PRE_SALE_NG,			// �̔��O����G���[
	SEASON_CHK_INVALID_TYPE,		// �����ʔ͈͊O�G���[
	SEASON_CHK_UNUSED_TYPE,			// �����ʖ��g�p�G���[
	SEASON_CHK_OUT_OF_RNG_TYPE,		// ����g�p�ړI�ݒ�͈͊O�G���[
	SEASON_CHK_UNUSED_RYO_SYU,		// ������ʖ��g�p�G���[
	SEASON_CHK_ORDER_OF_USE,		// ���Z���ԃG���[
	SEASON_CHK_BEFORE_VALID,		// �����O�G���[
	SEASON_CHK_AFTER_VALID,			// �����؂�G���[
	SEASON_CHK_NTIPASS_ERROR,		// �A���`�p�X�G���[
} eSEASON_CHK_RESULT;
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)

/*** LED�֘A��` ***/

/*** LedReq() �ւ̑�1���� ***/
// MH810100(S) Y.Yamauchi 2019/10/04 �Ԕԃ`�P�b�g���X(�����e�i���X)
//#define		RD_SHUTLED			0							// ���C���[�_�[�V���b�^LED
//#define		CN_TRAYLED			1							// �ޑK��o����LED
//#define		LEDCT_MAX			2							// ��L�����̌�
#define		CN_TRAYLED			0							// �ޑK��o����LED
#define		LEDCT_MAX			1							// ��L�����̌�
// MH810100(E) Y.Yamauchi 2019/10/04 �Ԕԃ`�P�b�g���X(�����e�i���X)

/*** LedReq() �ւ̑�2���� ***/
#define		LED_OFF				0							// OFF
#define		LED_ON				1							// ON
#define		LED_ONOFF			2							// �_��

/*** LedCtrl.Phase ***/
#define		LED_ON_TIME			25							// ON ���� (500ms : 20ms*25)
#define		LED_OFF_TIME		25							// OFF���� (500ms : 20ms*25)

/*** LED����p ***/
typedef struct {
	unsigned char	Request[LEDCT_MAX];						// ����v��
	unsigned char	Phase;									// �_��̪��� (0=���ݓ_�ŗv���Ȃ�, 1=����)
	unsigned char	OnOff;									// �_��̪��� (0=����OFF, 1=����ON)
	unsigned char	Count;									// �_�Ŏ��Զ���
} t_LedCtrl;

extern	t_LedCtrl	LedCtrl;								// LED����

/*** �ݒ�֘A ***/


/*** LCD�֘A ***/
extern	ushort	LCDNO;										// ���ݕ\�����̉��

extern	ushort	PowonSts_Param;								// �N�����̃p�����[�^�f�[�^��
extern	ushort	PowonSts_LockParam;							// �N�����̎Ԏ��p�����[�^�f�[�^��

/*** ���o�͐M������֘A��` ***/
#define		SIG_OFF				0							// OFF
#define		SIG_ON				1							// ON
#define		SIG_ONOFF			2							// ON-OFF(�ݼ���)

#define		OutPortMax			7							// �o�͐M����
// MH322914 (s) kasiyama 2016/07/11 �^�p�ݒ�v�����g�̏o�͐M�����C��[���ʃo�ONo.1266](MH341106)
#define		InPortMax			1							// ���͐M����
// MH322914 (e) kasiyama 2016/07/11 �^�p�ݒ�v�����g�̏o�͐M�����C��[���ʃo�ONo.1266](MH341106)
#define		INOUTMax			5							// �ݼ��ĐM����(�\��3��)

#define		SIG_OUTCNT1			1							// ���Z�����M��1(�o�ɐM��1)
#define		SIG_OUTCNT2			2							// ���Z�����M��2(�o�ɐM��2)
#define		SIG_GATEOPN			3							// �ްĊJ�M��
#define		SIG_GATECLS			4							// �ްĕM��
#define		SIG_J_PEND			10							// �ެ��َ��؂�
#define		SIG_J_PNEND			11							// �ެ��َ��؂�\��
#define		SIG_R_PEND			12							// ڼ�Ď��؂�
#define		SIG_R_PNEND			13							// ڼ�Ď��؂�\��
#define		SIG_JR_PEND			14							// �ެ��ٖ���ڼ�Ď��؂�
#define		SIG_JR_PNEND		15							// �ެ��ٖ���ڼ�Ď��؂�\��
#define		SIG_TROUBLE			16							// �����
#define		SIG_PAYMENT			17							// ���Z��
#define		SIG_CARFUL1			18							// ����1
#define		SIG_CARFUL2			19							// ����2
#define		SIG_CARNFUL1		20							// �قږ���1
#define		SIG_CARNFUL2		21							// �قږ���2
#define		SIG_OUTALM			22							// �o�Ɍx��
#define		SIG_CLOSE			24							// �x�ƒ�
#define		SIG_SECURITY		25							// �h�ƃZ���T�[
#define		SIG_COINNFUL		26							// ��݋��ɖ��t�\��
#define		SIG_NOTENFUL		27							// �������ɖ��t�\��
#define		SIG_SAFENFUL		28							// ��݋��ɖ��͎������ɖ��t�\��
#define		SIG_CHGEND			49							// �ޑK�؂�
#define		SIG_CHGNEND			50							// �ޑK�؂�\��
#define		SIG_COINFUL			60							// ��݋��ɖ��t
#define		SIG_NOTEFUL			61							// �������ɖ��t
#define		SIG_SAFEFUL			62							// ��݋��ɖ��͎������ɖ��t
#define		SIG_LPR_PEND		68							// ���َ��؂ꖔ�����ݐ؂�
#define		SIG_LPR_PNEND		69							// ���َ��؂ꖔ�����ݐ؂�\��
#define		SIG_CARFUL3			73							// ����3
#define		SIG_CAREMPTY1		74							// ���1
#define		SIG_CAREMPTY2		75							// ���2
#define		SIG_CAREMPTY3		76							// ���3

extern	unsigned char	sig_port[INOUTMax];					// �ݼ��ďo�͂��Ă���M���̏o���߰Ă�ێ�
															// �o�͐M���̐���Ɏg�p

#define		EXPORTSIGNALNUM		27

#define		EXPORT_J_PEND		0							// �ެ��َ��؂�
#define		EXPORT_J_PNEND		1							// �ެ��َ��؂�\��
#define		EXPORT_R_PEND		2							// ڼ�Ď��؂�
#define		EXPORT_R_PNEND		3							// ڼ�Ď��؂�\��
#define		EXPORT_JR_PEND		4							// �ެ��ٖ���ڼ�Ď��؂�
#define		EXPORT_JR_PNEND		5							// �ެ��ٖ���ڼ�Ď��؂�\��
#define		EXPORT_TROUBLE		6							// �����
#define		EXPORT_CARFUL1		7							// ����1
#define		EXPORT_CARFUL2		8							// ����2
#define		EXPORT_CLOSE		9							// �x�ƒ�
#define		EXPORT_COINNFUL		10							// ��݋��ɖ��t�\��
#define		EXPORT_NOTENFUL		11							// �������ɖ��t�\��
#define		EXPORT_SAFENFUL		12							// ��݋��ɖ��͎������ɖ��t�\��
#define		EXPORT_COINFUL		13							// ��݋��ɖ��t
#define		EXPORT_NOTEFUL		14							// �������ɖ��t
#define		EXPORT_SAFEFUL		15							// ��݋��ɖ��͎������ɖ��t
#define		EXPORT_CHGEND		16							// �ޑK�؂�
#define		EXPORT_CHGNEND		17							// �ޑK�؂�\��
#define		EXPORT_LPR_PEND		18							// ���َ��؂ꖔ�����ݐ؂�
#define		EXPORT_LPR_PNEND	19							// ���َ��؂ꖔ�����ݐ؂�\��
#define		EXPORT_SECURITY		20							// �h�ƃZ���T�[
#define		EXPORT_CARFUL3		21							// ����3
#define		EXPORT_CAREMPTY1	22							// ���1
#define		EXPORT_CAREMPTY2	23							// ���2
#define		EXPORT_CAREMPTY3	24							// ���3

#define		EXPORT_M_LD0		0							// �ð��LED0
#define		EXPORT_M_LD1		1							// �ð��LED1
#define		EXPORT_M_LD2		2							// �ð��LED2
#define		EXPORT_M_LD3		3							// �ð��LED3
#define		EXPORT_JP_RES		4							// �ެ��������ؾ�ĐM��
#define		EXPORT_CF_RES		5							// �ڼޯ�FOMAؾ�ĐM��
#define		EXPORT_URES			6							// CPU����pLED
#define		EXPORT_M_OUT		7							// RXM-1��\���o���߰�
#define		EXPORT_RXI_OUT7		8							// RXI-1�ėp�o���߰�7
#define		EXPORT_RXI_OUT6		9							// RXI-1�ėp�o���߰�6
#define		EXPORT_RXI_OUT5		10							// RXI-1�ėp�o���߰�5
#define		EXPORT_RXI_OUT4		11							// RXI-1�ėp�o���߰�4
#define		EXPORT_RXI_OUT3		12							// RXI-1�ėp�o���߰�3
#define		EXPORT_RXI_OUT2		13							// RXI-1�ėp�o���߰�2
#define		EXPORT_RXI_OUT1		14							// RXI-1�ėp�o���߰�1
#define		EXPORT_LOCK			15							// RXI-1��d��ۯ�����

extern	unsigned char	Ex_portFlag[EXPORTSIGNALNUM];		// ܰ�
extern	unsigned char	Ex_OutFlag[EXPORTSIGNALNUM];		// �o�͏�� 0:������,1:������
															//  0:�ެ��َ��؂�
															//  1:�ެ��َ��؂�\��
															//  2:ڼ�Ď��؂�
															//  3:ڼ�Ď��؂�\��
															//  4:�ެ��ٖ���ڼ�Ď��؂�
															//  5:�ެ��ٖ���ڼ�Ď��؂�\��
															//  6:�����
															//  7:����1
															//  8:����2
															//  9:�x�ƒ�
															// 10:��݋��ɖ��t�\��
															// 11:�������ɖ��t�\��
															// 12:��ݖ��͎������ɖ��t�\��
															// 13:��݋��ɖ��t
															// 14:�������ɖ��t
															// 15:��ݖ��͎������ɖ��t
															// 16:�ޑK�؂�
															// 17:�ޑK�؂�\��
															// 18:���َ��؂ꖔ�����ݐ؂�
															// 19:���َ��؂ꖔ�����ݐ؂�\��
															// 20:�h�ƃZ���T�[
															// 21:����3
															// 22:���1
															// 23:���2
															// 24:���3


extern	uchar	f_OpeSig_1shot;								// �o�͐M�� 1shot�o�͗v���׸ށi1=�v������j
extern	ushort	OpeSig_1shotInfo[OutPortMax];				// �o�͐M�� 1shot�o�͗v�����i�cON���ԁFx20ms�l�j

#define		INSIG_GATEST		1							// �ްĕ��
#define		INSIG_INCNT1		2							// ����1
#define		INSIG_INCNT2		3							// ����2
#define		INSIG_OUTCNT1		4							// �o��1
#define		INSIG_OUTCNT2		5							// �o��2
#define		INSIG_OPOPEN		6							// �����c��
#define		INSIG_OPCLOSE		7							// �����x��
#define		INSIG_SYUCHG1		8							// ��ʐؑ֐M��1
#define		INSIG_SYUCHG2		9							// ��ʐؑ֐M��2
#define		INSIG_FRCEXE		10							// �����o�ɐM��

#define		INSIG_RTSW0			0							// ���[�h�ݒ�SW
#define		INSIG_RTSW1			1							// ���[�h�ݒ�SW
#define		INSIG_RTSW2			2							// ���[�h�ݒ�SW
#define		INSIG_RTSW3			3							// ���[�h�ݒ�SW
#define		INSIG_DPSW0			4							// Rism, Cappi�ʐMEthernet�ݒ�
#define		INSIG_DPSW1			5							// �ݒ�SW_�\��
#define		INSIG_DPSW2			6							// �ݒ�SW_�\��
#define		INSIG_DPSW3			7							// �ݒ�SW_CAN�I�[�ݒ�
#define		INSIG_DUMMY1		8							// ���g�p
#define		INSIG_DUMMY2		9							// ���g�p
#define		INSIG_FANSTOP		10							// FAN��~���x���o�M��
#define		INSIG_FANSTART		11							// FAN�쓮�J�n���x���o�M��
#define		INSIG_SWMODE		12							// �ݒ�L�[��ԐM��(�\��)
#define		INSIG_DOOR			13							// �h�A�m�u�L�[��ԐM��
#define		INSIG_RXMIN			14							// RXM-1��\�����̓|�[�g
#define		INSIG_RXIIN			15							// RXI-1��ėp���̓|�[�g

#define		ALL_COUNT			1							// �ʂ��ǔ�
#define		PAYMENT_COUNT		2							// ���Z�ǔ�
#define		CANCEL_COUNT		3							// ���Z���~�ǔ�
#define		DEPOSIT_COUNT		4							// �a��ؒǔ�
#define		T_TOTAL_COUNT		5							// T���v�ǔ�
#define		GT_TOTAL_COUNT		6							// GT���v�ǔ�
#define		F_TOTAL_COUNT		7							// ����T���v�ǔ�
#define		COINMECK_COUNT		8							// ���ү��ǔ�
#define		TURIKAN_COUNT		9							// �ޑK�Ǘ����v�ǔ�
#define		COIN_SAFE_COUNT		10							// ��݋��ɒǔ�
#define		NOTE_SAFE_COUNT		11							// �������ɒǔ�
#define		LOST_TIK_COUNT		12							// ���������s�ǔ�
#define		INFO_COUNT			13							// ��t�����s�ǔ�
#define		KIYOUSEI_COUNT		14							// �����o�ɒǔ�
#define		FUSEI_COUNT			15							// �s���o�ɒǔ�

enum{
	__Prev = FUSEI_COUNT,
	CRE_CAPPI_COUNT,
	SET_PRI_COUNT,											// �^�p�ݒ蔭�s�ǔ�
	COUNT_MAX
	
};

#define		CLEAR_COUNT_ALL		0xF0						// �ǔԸر(�S��)
#define		CLEAR_COUNT_T		0xF1						// �ǔԸر(T���v������)
#define		CLEAR_COUNT_GT		0xF2						// �ǔԸر(GT���v������)
#define		CLEAR_COUNT_MT		0xF3						// �ǔԸر(MT���v������)


/*** ���ގg�p���� ***/
#define		USE_TIK				0							// ���Ԍ�
#define		USE_PAS				1							// �����
#define		USE_PPC				2							// ����߲�޶���
#define		USE_NUM				3							// �񐔌�
#define		USE_SVC				4							// ���޽��,�|����,������(���~���Ɋo���Ă����������܂�)
#define		USE_N_SVC			5							// ���޽��,�|����,������(�V�K�g�p��������)
#define		USE_MAX				6							// ���ޖ��g�p����ð��ِ�MAX

extern	uchar	card_use[USE_MAX];							// ���ޖ��g�p����ð���
extern	uchar	CardUseSyu;									// 1���Z�̊�����ނ̌���
extern	uchar	card_use2[15];							// ���޽�����g�p����ð���

extern	const unsigned char		OPE_CHR[][31];
extern	const unsigned char		OPE_CHR2[][31];
extern	const unsigned char		OPE_CHR3[][31];
extern	const unsigned char		OPE_OKNG[][3];
extern	const unsigned char		OPE_CHR_CRE[][31];
extern	const unsigned char		ERR_CHR[][31];
extern	const unsigned char		BIG_CHR[][21];
extern	const unsigned char		BIG_OPE[];
extern	const unsigned char		OPE_ENG[];
extern	const unsigned char		SKEY_CHR[][7];
extern	const unsigned char		CLSMSG[][31];
extern	const unsigned char		DRCLS_ALM[][31];
extern	const unsigned short	WAKU_DSP[][30];
#define	MAX_MSG		40
// MH321800(S) ������̒ǉ��ӏ���ύX
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
////#define	MAX_EXMSG	4
//#define	MAX_EXMSG	5
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
#define	MAX_EXMSG	4
// MH321800(E) ������̒ǉ��ӏ���ύX
extern const unsigned char		OPE_CHR_G_SALE_MSG[MAX_MSG+MAX_EXMSG][31];
extern const unsigned char		OPE_CHR_CYCLIC_MSG[][17];
extern const unsigned char MAGREADERRMSG[][5];
extern  const unsigned char	sercret_Str[][31];
extern const unsigned char shomei_errstr[][31];

extern	ushort			key_num;							// ���͂��ꂽ���Ԉʒu�ԍ�
extern	uchar			key_sec;							// �����͒l
extern	ushort			knum_len;							// ���͉\�Ȓ��Ԉʒu�ԍ��̌���
extern	ushort			knum_ket;							// ���͉\�Ȓ��Ԉʒu�ԍ��̌�
extern	struct logwork_rec	Log_Work;
extern	char			pas_NG;
extern	char			Err_onf;							// ���ݔ����װ 0=�����A1=�L��
extern	char			Alm_onf;							// ���ݔ����װ� 0=�����A1=�L��
extern	char			Err_trb;							// ����ِM�� 0=�����A1=�L��(�װ�p)
extern	char			Alm_trb;							// ����ِM�� 0=�����A1=�L��(�װїp)
extern	char			Err_cls;							// �x�Ɨv���װ  0=�����A1=�L��
extern	char			Alm_cls;							// �x�Ɨv���װ� 0=�����A1=�L��
extern	char			Security_out;						// �h�ƃZ���T 0=�����A1=�L��
extern	char			NgCard;								// NG�J�[�h�ǎ���e
extern	unsigned short	bundat1[24];
extern	unsigned short	bundat2[24];
extern	unsigned short	bunbak1[24];
extern	unsigned short	bunbak2[24];
extern	char			bundat_onf;
extern	struct clk_rec	CLK_BAK;
extern	char			CLOSE_stat;							// �x�ƽð��
extern	char			OPEN_stat;							// �c�ƽð��
#define		OPEN_NTNET		1
#define		OPEN_TIMESET	2
extern	uchar			auto_syu_prn;						// �����W�v���
extern	ushort			auto_syu_ndat;						// �����W�v�J�n����
extern	uchar			coin_kinko_evt;						// �R�C�����ɃC�x���g
extern	uchar			note_kinko_evt;						// �������ɃC�x���g
extern	uchar			CARFULL_stat[4];					// ����ð��(Auto) 0=��� 1=����
extern	ulong			PassPkNoBackup;						// ��������ԏ�ԍ��ޯ�����(n�������p)
extern	ushort			PassIdBackup;						// �����id�ޯ�����(n�������p)
extern	ulong			PassIdBackupTim;					// n�������p��ϰ
extern	unsigned short	StartingFunctionKey;				// �N�����ɉ�����Ă����t�@���N�V�����L�[
extern	ushort			RECI_SW_Tim;						// �̎��ؔ��s�\��ϰ

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//typedef	union {
//	ushort		SHORT;
//	struct {
//		uchar		yobi		:6;			// �\��
//		uchar		blink_status:1;			// �u�����N�X�e�[�^�X
//		uchar		comm_type	:1;			// ���ߗL���t���O
//		uchar		disp_type	:2;			// ��ʕ\���L���t���O
//		uchar		exec_status	:2;			// �ʐM���s���X�e�[�^�X
//											// 1�F�Z���^�[�ʐM�J�n
//											// 2�F�Z���^�[�ʐM�J�n������M(�ʐM��)
//											// 3�F�Z���^�[�ʐM����
//		uchar		status		:2;			// auto_cnt_prn(1-3)
//											// 	0:�󂫁i�J�n�����҂��j
//											// 	1:�ʐM�J�n�҂�
//											// 	2:�ʐM�󎚒�
//											// 	3:�ʐM��������
//		uchar		wait_kind	:2;			// ����ʐM���
//											// 1�F�Z���^�[�ʐM�̂�
//											// 2�F����T���v��̒��߁{�Z���^�[�ʐM
//	} BIT;
//}EDY_CNT_CTRL;
//extern	EDY_CNT_CTRL	edy_cnt_ctrl;
//#define	auto_cnt_prn	edy_cnt_ctrl.BIT.status
//#define	edy_auto_com	edy_cnt_ctrl.BIT.exec_status
//extern	ulong 			auto_cnt_ndat;						// �ŏI�Z���^�[�ʐM�J�n����
//extern	Receipt_data	PayData_save;						// ��d�O�̐��Z���,�̎��؈��ް�
//extern	uchar			fg_autocnt_fstchk;					/* ���グ����Edy�����Z���^�[�ʐM�v�ۊm�F�t���O�i0:��/1:�ρj */
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

extern	char			RAMCLR;								// RAM�ر���{�׸�

extern	teiki_use		InTeiki_PayData_Tmp;				// �}������ް��ꎞ�ۑ��ر
// ���[�U�[�����e�i���X�Őݒ�ύX����
typedef union{
	unsigned char	BYTE;
	struct{
		unsigned char yobi:3;			// B5-7 = �\��
		unsigned char cpara:1;			// B4 = �Ԏ��p�����[�^
		unsigned char bpara:1;			// B3 = ���ʃp�����[�^
		unsigned char splday:1;			// B2 = ���ʓ��^���ʊ���
		unsigned char tickvalid:1;		// B1 = ������
		unsigned char other:1;			// B0 = ���̑�
	}BIT;
} f_PARAUPDATE;
extern	f_PARAUPDATE	f_ParaUpdate;

// ���[�U�[�����e�i���X�̌������̕ҏW�Ŏg�p���郏�[�N�G���A
typedef struct {
	long	kind;				// ���
	long	data;				// ���e
	long	date[4];			// [0]=�ύX�O�J�n�N�����A[1]=�ύX�O�I���N�����A[2]=�ύX��J�n�N�����A[3]=�ύX��I���N����
} TIC_PRM;

typedef struct {
	char	no;					// 1=�������P��ҏW�A2=�������Q��ҏW�A3=�������R��ҏW
	TIC_PRM	tic_prm;			// �������̕ҏW�f�[�^
} TICK_VALID_DATA;
extern	TICK_VALID_DATA	tick_valid_data;
extern	ushort	sntp_prm_before;

/* NT-NET���Z�ް� �⑫��� */
typedef struct {
	ushort	PayMethod;										// ���Z���@�i0=���������Z�A1=���Ԍ����Z�A2=��������Z�A3=��������p���Z�A4=�������Z�j
	ushort	PayClass;										// �����敪�i0=���Z�A1=�Đ��Z�A2=���Z���~�A3=�Đ��Z���~�j
	ushort	PayMode;										// ���ZӰ�ށi0=�������Z�A1=���������Z�A2=�蓮���Z�A3=���Z�Ȃ��j
	ushort	CardType;										// ���Ԍ����߁i���C�ް�ID�j
	ushort	CMachineNo;										// ���Ԍ������@�@�B���i0�`20�j
	ulong	CardNo;											// ���Ԍ��ԍ�(�����ǂ���)�i0�`99999�j
	ushort	KakariNo;										// �W�����i���0�j
	ushort	CountSet;										// �ݎԶ��āi0=����+1�A1=���Ȃ��A2=����-1�j
	ushort	PassCheck;										// ����߽�����i0=����ON,1=����OFF,2=����OFF�j
	ushort	ReadMode;										// �����ײ�Ӱ�ށi0=ذ��ײāA1=ذ�޵�ذ�j
	uchar	f_ChuusiCard;									// ���~���׸ށi1=APS�݊����~��, 2=������̫�ϯĒ��~���j
	uchar	f_ChgOver;										// ���ߏ���z���ް�����׸�(1=����)
	ulong	BeforeTwariTime;								// �O�񊄈����ԁi���j
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	uchar	CreditDate[2];									// �N���W�b�g�J�[�h�L������ [0]=�N [1]=��
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
} t_OpeNtnetAddedInfo;
extern	t_OpeNtnetAddedInfo	OpeNtnetAddedInfo;
extern	const uchar		Shunbun_Day[100];
extern	const uchar		Shuubun_Day[100];
extern	ushort			RenewalMonth;						// ������X�V����
extern	ulong			RenewalFee;							// ������X�V����
extern	uchar	Syusei_Select_Flg;
#define		APSF_PKNO_LOWER				1					// APS�t�H�[�}�b�g ���ԏꇂ����
#define		APSF_PKNO_UPPER				999					// APS�t�H�[�}�b�g ���ԏꇂ���
#define		GTF_PKNO_LOWER				1001				// GT�t�H�[�}�b�g ���ԏꇂ����
#define		GTF_PKNO_UPPER				260001L				// GT�t�H�[�}�b�g ���ԏꇂ���
extern	char	an_vol_flg;
extern	char	shomei_errnum;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//extern	ushort	dspErrMsg;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
extern	unsigned char	Pri_Open_Status_R;			// �v�����^�J�o�[���
extern	unsigned char	Pri_Open_Status_J;			// �v�����^�J�o�[���

/*[]----------------------------------------------------------------------[]*/
/*| �װ����                                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| �װ���ޕ\ Ӽޭ�ٺ��ޒ�`                                               |*/
/*| err_chk(md,,)�̑�P����md�Ƃ��Ďg�p����                                |*/
/*| moj_no[modno_note] == mod_note�ƂȂ�                                   |*/
/*[]----------------------------------------------------------------------[]*/
#define		ERRMDL_MAIN					0					/* Ҳݐ���Ӽޭ�ٺ���			*/
#define		ERRMDL_READER				1					/* ���Cذ�ްӼޭ�ٺ���			*/
#define		ERRMDL_PRINTER				2					/* ڼ��,�ެ���Ӽޭ�ٺ���		*/
#define		ERRMDL_COIM					3					/* ��ݎ��ʏz��Ӽޭ�ٺ���		*/
#define		ERRMDL_NOTE					4					/* ����ذ�ްӼޭ�ٺ���			*/
#define		ERRMDL_IFROCK				10					/* IF��,ۯ����uӼޭ�ٺ���		*/
#define		ERRMDL_IFFLAP				11					/* IF��,�ׯ�ߑ��uӼޭ�ٺ���		*/
#define		ERRMDL_CRRFLAP				15					/* CRR���,�ׯ�ߑ��uӼޭ�ٺ���	*/
#define		ERRMDL_FLAP_CRB				16					// �ȈՃt���b�v�^���b�N���u�i�t���b�v���u�ACRB��j���W���[���R�[�h
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		ERRMDL_EJA					22					// �d�q�W���[�i��
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#define		ERRMDL_CAL_CLOUD			24					// �N���E�h�ŗ����v�Z�G���W��
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#define		ERRMDL_SODIAC				25					/* Sodiac�i���������jӼޭ�ٺ��	*/
// MH810100(S) K.Onodera 2019/12/12 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
#define		ERRMDL_TKLSLCD				26					/* �Ԕԃ`�P�b�g���XLCDӼޭ�ٺ���		*/
#define		ERRMDL_DC_NET				28					// DC-NET�ʐM
#define		ERRMDL_BARCODE				30					// �o�[�R�[�h���[�_
// MH810103(s) �d�q�}�l�[�Ή� �đ����̃G���[��ʂ����������s��C��
#define		ERRMDL_SD					31					// SD�J�[�h
// MH810103(e) �d�q�}�l�[�Ή� �đ����̃G���[��ʂ����������s��C��
#define		ERRMDL_REALTIME				37					// ���A���^�C���ʐM
// MH810100(E) K.Onodera 2019/12/12 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
#define		ERRMDL_I2C					40					/* I2C Error���	*/
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//#define		ERRMDL_CCT					54					/* IBK(CCT)�֘A�G���[ */
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
#define		ERRMDL_REMOTE				55					/* ���uӼޭ�ٺ���				*/
#define		ERRMDL_MIFARE				59					/* MifearӼޭ�ٺ���				*/
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		ERRMDL_EDY					62					/* EdyӼޭ�ٺ���				*/
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		ERRMDL_NTNETIBK				63					/* NT-NET IBKӼޭ�ٺ���			*/
#define		ERRMDL_LABEL				64					/* ���������Ӽޭ�ٺ���			*/
#define		ERRMDL_NTNETDOPAMAF			70					/* NT-NET-DOPA MAFӼޭ�ٺ���	*/
#define		ERRMDL_NTNETDOPA			ERRMDL_NTNETDOPAMAF		/* NT-NET-DOPA Ӽޭ�ٺ���		*/
#define		ERRMDL_NTNET				66					/* NT-NETӼޭ�ٺ���				*/
#define		ERRMDL_SUICA				69					/* SuicaӼޭ�ٺ���				*/

#define		ERRMDL_APSNETIBC			ERRMDL_NTNETIBK		/* APS-NET IBCӼޭ�ٺ���		*/
#define		ERRMDL_APSNET				ERRMDL_NTNET		/* APS-NETӼޭ�ٺ���			*/
#define		ERRMDL_LANIBK				72					/* ���u�F�Q�|�[�gLAN�|IBK		*/
#define		ERRMDL_DOPAIBK				73					/* ���u�F�Q�|�[�gDoPa�|IBK		*/
#define		ERRMDL_CAPPI				74					/* Cappi�֘A�G���[ */
#define		ERRMDL_IDC					75					/* iD�֘A�G���[ */
#define		ERRMDL_CARD					76					/* ����J�[�h�G���[ */
#define		ERRMDL_FOMAIBK				77					/* ���u�FFOMA�|IBK		*/
#define		ERRMDL_SUBCPU				80					/* ���CPUӼޭ�ٺ���				*/
#define		ERRMDL_CAN					86					/* CANӼޭ�ٺ���		*/
#define		ERRMDL_CREDIT				88					/* �N���W�b�g		*/
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		ERRMDL_EC					32					/* ���σ��[�_Ӽޭ�ٺ���	*/
// MH321800(E) G.So IC�N���W�b�g�Ή�

//--------------------------------------------------------------------------
// �eӼޭ�ق��Ƃ̴װ����
//--------------------------------------------------------------------------

// Ҳ�Ӽޭ��
#define		ERR_MAIN_MEMORY				1					// Memory  Error 'AMANO' Clash
#define		ERR_MAIN_BOOTINFO_MAIN		2					// Ҳ�CPU�^�p�ʈُ�
#define		ERR_MAIN_BOOTINFO_SUB		3					// ���CPU�^�p�ʈُ�
#define		ERR_MAIN_SETTEIRAM			5					// RAM��̐ݒ�ر�ُ�		�i����/�����j
#define		ERR_MAIN_SETTEIFLASH		6					// FLASH��̐ݒ�ر�ُ�		�i����/�����j
#define		ERR_MAIN_CLOCKFAIL			7					// ���v�ް��ُ�
#define		ERR_MAIN_SUBCPU				8					// �R���� �����i�Ȃ�
#define		ERR_MAIN_ARCNETBUFFULL		10					// ARCNET�ޯ̧��
#define		ERR_MAIN_MESSAGEFULL		11					// ү���޷����				�i�����Ȃ��j
#define		ERR_MAIN_FLASH_ERASE_ERR	13					// FlashROM�������s
#define		ERR_MAIN_FLASH_WRITE_ERR	14					// FlashROM�������ݎ��s
#define		ERR_MAIN_LOG_WRITE_LOCK		15					// ���O�����݂̃��b�N���s
#define		ERR_MAIN_ARCNETICFAILUER	16					// ARCNET��ײ�ވُ�i���������s�j
#define		ERR_MAIN_READER_ARCNETFAIL	17					// ���Cذ�ް�ʐM�s��(ARCNET)
#define		ERR_MAIN_ANOUNCE_ARCNETFAIL	18					// �ųݽϼݒʐM�s��(ARCNET)
#define		ERR_MAIN_IBC_ARCNETFAIL		23					// APSNET���W���[���ʐM�s��
#define		ERR_MAIN_IBKCOMFAIL_213		25					// IBKӼޭ�ْʐM�s�ǁiARCNET Node�F213�j
#define		ERR_MAIN_IBKCOMFAIL_214		26					// IBKӼޭ�ْʐM�s�ǁiARCNET Node�F214�j
#define		ERR_MAIN_IBKCOMFAIL_215		27					// IBKӼޭ�ْʐM�s�ǁiARCNET Node�F215�j
#define		ERR_MAIN_IBKCOMFAIL_216		28					// IBKӼޭ�ْʐM�s�ǁiARCNET Node�F216�j
#define		ERR_MAIN_ANNOUNCECOMFAIL	31					// �ųݽϼݒʐM�s��
#define		ERR_MAIN_FLAPIBCCOM			35					// �ׯ�ߗpIBCӼޭ�ْʐM�s��
#define		ERR_MAIN_BASICPARAMETER		40					// �ݒ�l�ُ�
#define		ERR_MAIN_CALPARAMETER_NG	50					// �^�p�ݒ�v�����g�ُ�
#define		ERR_MAIN_CALPARAMETER_OK	51					// �^�p�ݒ�v�����g����
#define		ERR_MAIN_ILLEGAL_INSTRUCTION 52					// ����`���ߗ�O���荞�ݔ���
#define		ERR_MAIN_ILLEGAL_SUPERVISOR_INST	53			// �������ߗ�O����
#define		ERR_MAIN_ILLEGAL_FLOATINGPOINT		54			// ���������_��O����
#define		ERR_MAIN_UNKOWNVECT			55					// ���o�^�޸�����
#define		ERR_MAIN_CLOCKREADFAIL		57					// ���v�ǂݍ��ݎ��s
#define		ERR_MAIN_MACHINEREADFAIL	58					// �@����ǂݍ��ݎ��s
// MH322914 (s) kasiyama 2016/07/13 ���O�ۑ��ł��Ȃ���Ԃ����P[���ʃo�ONo.1225](MH341106)
#define		ERR_MAIN_SRAM_LOG_BUFFER_FULL	59				// SRAM���O�o�b�t�@�t��
// MH322914 (e) kasiyama 2016/07/13 ���O�ۑ��ł��Ȃ���Ԃ����P[���ʃo�ONo.1225](MH341106)
#define		ERR_MAIN_OVER1YEARCAL		60					// 1�N�ȏ�̗����v�Z���s����
#define		ERR_MAIN_INOUTTIMENG		61					// ���o�Ɏ����K��O
#define		ERR_MAIN_LOCKSTATUSNG		63					// ۯ����u��ԋK��O
#define		ERR_MAIN_LOCKNUMNG			65					// �s��������ۯ����u�ԍ����K��O
#define		ERR_MAIN_FKOVER1YEARCAL		66					// 1�N�ȏ�̗����v�Z���s����(�s���E����)
#define		ERR_MAIN_FKINOUTTIMENG		67					// ���o�Ɏ����K��O(�s���E����)
#define		ERR_MAIN_NOSAFECLOPERATE	68					// ���ɖ����Z�o�Ȃ��Ŕ�����
#define		ERR_MAIN_INOUTGYAKU			71					// ���o�ɋt�]�ŗ����v�Z���s�����Ƃ���
#define		ERR_MAIN_FKINOUTGYAKU		72					// ���o�ɋt�]�ŗ����v�Z���s�����Ƃ���(�s���E����)
#define		ERR_MAIN_FUSEI_RYOCAL		73					// �����v�Z�Ŗ���ٰ��		�i����/�����j
#define		ERR_MAIN_SNTP_FAIL			83					// �Z���^�[�����������s
#define		ERR_MAIN_RTCCPU_VOLDWN		85					// ���vIC���M��H�d���ቺ�G���[		�i����/�����j
#define		ERR_MAIN_RTCTMPREC_VOLDWN	86					// ���vIC���x�⏞��H�d���ቺ�G���[	�i����/�����j
#define		ERR_MAIN_BOOTINFO_WAVE		87					// �����f�[�^�^�p�ʈُ�
#define		ERR_MAIN_BOOTINFO_PARM		88					// ���ʃp�����[�^�^�p�ʈُ�
#define		ERR_MAIN_MAFCOMFAIL_224		96					// MAFӼޭ�ْʐM�s�ǁiARCNET Node�F224�j
#define		ERR_MAIN_MAFCOMFAIL_225		97					// MAFӼޭ�ْʐM�s�ǁiARCNET Node�F225�j
#define		ERR_MAIN_MAFCOMFAIL_226		98					// MAFӼޭ�ْʐM�s�ǁiARCNET Node�F226�j�\��
#define		ERR_MAIN_MAFCOMFAIL_227		99					// MAFӼޭ�ْʐM�s�ǁiARCNET Node�F227�j�\��

// ���Cذ�ްӼޭ��
#define		ERR_RED_COMFAIL				1					// �ʐM�s��
#define		ERR_RED_MAGTYPE				2					// ���C���[�_�[�^�C�v�ُ�
#define		ERR_RED_STACK_R				10					// ���ދl�܂�(ذ�ް��)
#define		ERR_RED_STACK_P				11					// ���ދl�܂�(�������)
#define		ERR_RED_LOCSENSOR1			13					// �ʒu�ݻ�1�s��
#define		ERR_RED_LOCSENSOR2			14					// �ʒu�ݻ�2�s��
#define		ERR_RED_LOCSENSOR3			15					// �ʒu�ݻ�3�s��
#define		ERR_RED_LOCSENSOR4			16					// �ʒu�ݻ�4�s��
#define		ERR_RED_LOCSENSOR5			17					// �ʒu�ݻ�5�s��
#define		ERR_RED_LOCSENSOR6			18					// �ʒu�ݻ�6�s��
#define		ERR_RED_LOCSENSOR7			19					// �ʒu�ݻ�7�s��
#define		ERR_RED_PRNLOCSENSOR		20					// ������ʒu�ݻ��s��
#define		ERR_RED_START				21					// �X�^�[�g�����G���[
#define		ERR_RED_DATA				22					// �f�[�^�G���[
#define		ERR_RED_PARITY				23					// �p���e�B�G���[
#define		ERR_RED_HPSENSOR1			24					// HP�ݻ�1�s��
#define		ERR_RED_HPSENSOR2			25					// HP�ݻ�2�s��
#define		ERR_RED_AFTERWRITE			30					// ������̴װ
#define		ERR_RED_OTHERS				31					// ���̑��̴װ
#define		ERR_RED_MEMORY				32					// ��ؕs��
#define		ERR_RED_VERIFY				33					// ���̧��װ
#define		ERR_RED_SCI_PARITY			40					// �V���A���p���e�B�G���[
#define		ERR_RED_SCI_FRAME			41					// �V���A���t���[�~���O�G���[
#define		ERR_RED_SCI_OVERRUN			42					// �V���A���I�[�o�[�����G���[

// ������
#define		ERR_PRNT_R_PRINTCOM			1					// Printer Error
#define		ERR_PRNT_R_HEADHEET			3					// Head Heet Up Error
#define		ERR_PRNT_R_CUTTER			4					// Cutter       Error
#define		ERR_PRNT_J_PRINTCOM			20					// Printer Error
#define		ERR_PRNT_J_HEADHEET			22					// Head Heet Up Error
#define		ERR_PRNT_DATA_ERR			51					// �󎚗v���ް��װ
#define		ERR_PRNT_LOG_ERR			52					// ۸��ް��װ
#define		ERR_PRNT_YEAR_ERR			53					// ���t�i�N�j�ް��װ
#define		ERR_PRNT_MON_ERR			54					// ���t�i���j�ް��װ
#define		ERR_PRNT_DAY_ERR			55					// ���t�i���j�ް��װ
#define		ERR_PRNT_HOUR_ERR			56					// ���t�i���j�ް��װ
#define		ERR_PRNT_MIN_ERR			57					// ���t�i���j�ް��װ
#define		ERR_PRNT_ERR_IRQ			61					// ������װ�����ݔ���
#define		ERR_PRNT_BUFF_FULL			62					// ��������M�ޯ̧���ް�۰����
#define		ERR_PRNT_INIT_ERR			63					// ��������������s
#define		ERR_PRNT_PARITY_ERR_R		80					// ڼ����������è�װ
#define		ERR_PRNT_FRAMING_ERR_R		81					// ڼ��������ڰ�ݸ޴װ
#define		ERR_PRNT_OVERRUN_ERR_R		82					// ڼ����������ް�ݴװ
#define		ERR_PRNT_PARITY_ERR_J		83					// �ެ�����������è�װ
#define		ERR_PRNT_FRAMING_ERR_J		84					// �ެ���������ڰ�ݸ޴װ
#define		ERR_PRNT_OVERRUN_ERR_J		85					// �ެ�����������ް�ݴװ

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		ERR_EJA_COMFAIL				1					// [01]�ʐM�s��
#define		ERR_EJA_SD_UNAVAILABLE		2					// [02]SD�J�[�h�g�p�s��
#define		ERR_EJA_WRITE_ERR			3					// [03]�����ݎ��s
#define		ERR_EJA_UNCONNECTED			4					// [04]���ڑ�
#define		ERR_EJA_ERR_IRQ				40					// [40]�G���[�����ݔ���(FT-4000�ł͔������Ȃ�)
#define		ERR_EJA_PARITY_ERR			41					// [41]�p���e�B�G���[
#define		ERR_EJA_FRAMING_ERR			42					// [42]�t���[�~���O�G���[
#define		ERR_EJA_OVERRUN_ERR			43					// [43]�I�[�o�[�����G���[
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

// ���ү�
#define		ERR_COIN_COMFAIL			1					// �ʐM�s��
#define		ERR_COIN_ACCEPTER			5					// ������  Error
#define		ERR_COIN_REJECTSW			6					// ؼު��  Error
#define		ERR_COIN_DISPENCEFAIL		7					// ���o  Error
#define		ERR_COIN_10EMPTYSW			12					//  10 ����èSW Error
#define		ERR_COIN_50EMPTYSW			13					//  50 ����èSW Error
#define		ERR_COIN_100EMPTYSW			14					// 100 ����èSW Error
#define		ERR_COIN_500EMPTYSW			15					// 500 ����èSW Error
#define		ERR_COIN_10OVFSENSOR		18					//  10 ���ް�۰�ݻ� Error
#define		ERR_COIN_50VFSENSOR			19					//  50 ���ް�۰�ݻ� Error
#define		ERR_COIN_100VFSENSOR		20					// 100 ���ް�۰�ݻ� Error
#define		ERR_COIN_500VFSENSOR		21					// 500 ���ް�۰�ݻ� Error
#define		ERR_COIN_ACK4RECEIVE		51					// ACK4��M
#define		ERR_COIN_NAKRECEIVE			52					// NAK��M
#define		ERR_COIN_RECEIVEBUFFULL		55					// ��M�ޯ̧��
#define		ERR_COIN_PARITYERROR		56					// ���è�װ
#define		ERR_COIN_OVERRUNERROR		57					// ���ް�ݴװ
#define		ERR_COIN_FLAMERROR			58					// ��-�ݸ޴װ
#define		ERR_COIN_RECEIVESIZENG		59					// ��M�������s��v
#define		ERR_COIN_RECEIVEBCCNG		60					// ��M�ް�BCC�װ
#define		ERR_COIN_CASETTOUT			99					// Casett    OUT
#define		ERR_COIN_SAFE				30

// ����ذ�ް
#define		ERR_NOTE_COMFAIL			1					// �ʐM�s��
#define		ERR_NOTE_JAM				5					// Note Jam  Error
#define		ERR_NOTE_SHIKIBETU			6					// Read Div  Error
#define		ERR_NOTE_STACKER			7					// Stacker   Error
#define		ERR_NOTE_REJECT				8					// Reject    Error
#define		ERR_NOTE_ACK4RECEIVE		51					// ACK4��M
#define		ERR_NOTE_NAKRECEIVE			52					// NAK��M
#define		ERR_NOTE_SAFE				30

// IF�ՁAۯ����u
#define		ERR_IFMASTERCOMFAIL			1					// IF�Րe�@�ʐM�s��
#define		ERR_IFSLAVECOMFAIL			20					// IF�Վq�@�ʐM�s��
#define		ERR_LOCKCLOSEFAIL			38					// ۯ����u��ۯ�
#define		ERR_LOCKOPENFAIL			39					// ۯ����u�Jۯ�
#define		ERR_LOCK_STATUSNG			52					// ۯ����u��Ԃ��K��O
#define		ERR_LOCK_EVENTOUTOFRULE		53					// ����Ĕԍ��K��O
#define		ERR_FLAPIFMASTERCOMFAIL		1					// IF�Րe�@�ʐM�s��
#define		ERR_FLAPCLOSEFAIL			38					// �ׯ�ߑ��u��ۯ�
#define		ERR_FLAPOPENFAIL			39					// �ׯ�ߑ��u�Jۯ�
#define		ERR_FLAPDOWNRETRYOVER		40					// ����������ײ���ް
#define		ERR_FLAPDOWNRETRY			41					// ����������ײ����
#define		ERR_FLAP_NOTPROCESSED		50					// IBC�O���ް�������
#define		ERR_FLAP_STATUSNG			52					// �ׯ�ߑ��u��Ԃ��K��O
#define		ERR_FLAP_EVENTOUTOFRULE		53					// ����Ĕԍ��K��O
#define		ERR_FLAP_AUTO_UPRETRYOVER	45					// �t���b�v�㏸���[�^�[���䃊�g���C�I�[�o�[
#define		ERR_FLAP_AUTO_DOWNRETRYOVER	46					// �t���b�v���~���[�^�[���䃊�g���C�I�[�o�[

// Mifare
#define		ERR_MIF_WRT_VERIFY			10					// Write Verify NG
#define		ERR_MIF_WRT_NG				11					// Write NG(Verify�ȊO)

// NT-NET �q�@	(66)
// ���ԓ���ւ��A�R�����g�ǋL
#define		ERR_NTNET_PRIOR_RCVBUF		6					// �D��f�[�^��M�o�b�t�@�t���ɂ��f�[�^�폜����
#define		ERR_NTNET_NORMAL_RCVBUF		7					// �ʏ�f�[�^��M�o�b�t�@�t���ɂ��f�[�^�폜����
#define		ERR_NTNET_BROADCAST_RCVBUF	8					// ����f�[�^��M�o�b�t�@�t���ɂ��f�[�^�폜����
#define		ERR_NTNET_FREE_RCVBUF		9					// FREE�f�[�^��M�o�b�t�@�t���ɂ��f�[�^�폜����
#define		ERR_NTNET_OVERWRITE_SENDBUF	12					// ���M�o�b�t�@�㏑��
#define		ERR_NTNET_DISCARD_SENDDATA	18					// �u���b�N�ԍ��ُ�Ńf�[�^�j��
#define		ERR_NTNET_NTBLKNO_VALID		19					// �u���b�N�ԍ��ُ�(����f�[�^�L��)
#define		ERR_NTNET_NTBLKNO_INVALID	20					// �u���b�N�ԍ��ُ�(����f�[�^����)
#define		ERR_NTNET_ID22_BUFNFULL		24					// ���Z�f�[�^���M�o�b�t�@�j�A�t��
#define		ERR_NTNET_ID22_BUFFULL		25					// ���Z�f�[�^���M�o�b�t�@�t��
#define		ERR_NTNET_ID20_BUFFULL		26					// ���Ƀf�[�^���M�o�b�t�@�t��
#define		ERR_NTNET_ID21_BUFFULL		27					// �o�Ƀf�[�^���M�o�b�t�@�t��			�����ݏ����Ȃ�
#define		ERR_NTNET_TTOTAL_BUFFULL	28					// T���v�f�[�^���M�o�b�t�@�t��			�����ݏ����Ȃ�
#define		ERR_NTNET_ID22_SENDBUF		30					// ���Z�f�[�^���M�o�b�t�@�㏑��
#define		ERR_NTNET_ID20_SENDBUF		31					// ���Ƀf�[�^���M�o�b�t�@�㏑��
#define		ERR_NTNET_ID21_SENDBUF		32					// �o�Ƀf�[�^���M�o�b�t�@�㏑��			�����ݏ����Ȃ�
#define		ERR_NTNET_TTOTAL_SENDBUF	33					// T���v�f�[�^���M�o�b�t�@�㏑��		�����ݏ����Ȃ�
#define		ERR_NTNET_ARCBLKNO_INVALID	48					// �r���u���b�N�ُ�
#define		ERR_NTNET_RECV_BLK_OVER		50					// Arcnet��M�u���b�N�I�[�o�[
#define		ERR_NTNET_BUFFULL_RETRY		60					// �o�b�t�@FULL�ɂ��đ��񐔃I�[�o�[
#define		ERR_NTNETDOPA_SAVE_SENDBUF_FULL			85		// �ێ����M�f�[�^�o�b�t�@�t���ɂ��f�[�^�폜����

#define		ERR_NTNETDOPA_SAVE_SENDBUF_NFULL		30		// �ێ����M�f�[�^�o�b�t�@�j�A�t��					�����g�p(�ԍ��͉��l)
#define		ERR_NTNETDOPA_NOT_SAVE_SENDBUF_FULL		31		// ��ێ����M�f�[�^�o�b�t�@�t���ɂ��f�[�^�폜����	�����g�p(�ԍ��͉��l)
#define		ERR_NTNETDOPA_NOT_SAVE_SENDBUF_NFULL	32		// ��ێ����M�f�[�^�o�b�t�@�j�A�t��					�����g�p(�ԍ��͉��l)
#define		ERR_NTNETDOPA_RECVBUF_FULL				33		// ��M�f�[�^�o�b�t�@�t���ɂ��f�[�^�폜����		�����g�p(�ԍ��͉��l)
#define		ERR_NTNETDOPA_RECVBUF_NFULL				34		// ��M�f�[�^�o�b�t�@�j�A�t��						�����g�p(�ԍ��͉��l)
#define		ERR_NTNETDOPA_PARITY_ERR		40				// �p���e�B�G���[
#define		ERR_NTNETDOPA_FRAMING_ERR		41				// �t���[�~���O�G���[
#define		ERR_NTNETDOPA_OVERRUN_ERR		42				// �I�[�o�[�����G���[

#define		ERR_NTNETDOPA_DISCARD_SENDDATA	86				// �u���b�N�ԍ��ُ�Ńf�[�^�j��
#define		ERR_NTNETDOPA_NTBLKNO_VALID		87				// �u���b�N�ԍ��ُ�(����f�[�^�L��)
#define		ERR_NTNETDOPA_ARCBLKNO_INVALID	88				// �r���u���b�N�ُ�
#define		ERR_NTNETDOPA_BUFFULL_RETRY		89				// �o�b�t�@FULL�ɂ��đ��񐔃I�[�o�[

#define		ERR_APSNET_ARCBLKNO_INVALID_RCV	48				// 6348�FAPS��M�d�� �u���b�N�ԍ��ُ� 					�i1shot�j
#define		ERR_APSNET_COMMFAIL				1				// 6301�F�O�񑗐M�f�[�^���������� �đ����ް

// ���������()
#define		ERR_LPRN_COMFAIL			1					// �ʐM�^�C���A�E�g
#define		ERR_LPRN_CUTTER				10					// �J�b�^�[�G���[
#define		ERR_LPRN_HEADOPEN			11					// �w�b�h�I�[�v��
#define		ERR_LPRN_HEAD				12					// �w�b�h�G���[
#define		ERR_LPRN_SENSOR				13					// �Z���T�G���[
#define		ERR_LPRN_BUFF_FULL			14					// �o�b�t�@�I�[�o�[
#define		ERR_LPRN_MEDIA				15					// ���f�B�A�G���[
#define		ERR_LPRN_OTHER				16					// ���̑��̃G���[
#define		ERR_LPRN_OFFLINE			20					// �ʐM�I�t���C��

#define		ERR_SUICA_COMFAIL			1					// �ʐM�s��
#define		ERR_SUICA_RECEIVE			2					// �ُ��ް���M
#define		ERR_SUICA_OPEN				4					// �J�Ǐ����ُ�
#define		ERR_SUICA_MIRYO				5					// �������������ѱ��
#define		ERR_SUICA_PAY_NG			6					// Suica���ZNG

#define		ERR_SUICA_ACK4RECEIVE		51					// ACK4��M
#define		ERR_SUICA_NAKRECEIVE		52					// NAK��M

#define		ERR_SUICA_REINPUT			62					// ���͍ėv�����M
#define		ERR_SUICA_REOUTPUT			63					// �o�͍Ďw�ߑ��M
#define		ERR_SUICA_MIRYO_START		64					// ��������
#define		ERR_SUICA_PAY_ERR			65					// �d�q�}�l�[���[�_�[���ϕs��

// MH321800(S) G.So IC�N���W�b�g�Ή�
// ���σ��[�_
#define		ERR_EC_COMFAIL				ERR_SUICA_COMFAIL	// [01]�ʐM�s��
#define		ERR_EC_ERR_HARD				2					// [02]���[�_�n�[�h�E�F�A�ُ�(�ُ�f�[�^��M��)
#define		ERR_EC_ERR_DRIVER			3					// [03]JVMA�h���C�o�ُ�(�ُ�f�[�^��M��)
#define		ERR_EC_ERR_SYSTEM			4					// [04]�\�t�g�E�F�A�ُ�(�ُ�f�[�^��M��)
#define		ERR_EC_ERR_ENCRYPTION		5					// [05]�Í������[�h�ݒ莸�s(�ُ�f�[�^��M��)
#define		ERR_EC_ERR_CONNECTION		6					// [06]���[�_�ڑ��ُ�(�ُ�f�[�^��M��)
#define		ERR_EC_OPEN					9					// [09]���[�_NotReady
#define		ERR_EC_UNINITIALIZED		10					// [10]���σ��[�_�N��������
// MH810103 GG119202(S) �ُ�f�[�^�t�H�[�}�b�g�ύX
#define		ERR_EC_ERR_LTE_FAIL			11					// [11]���σ��[�_�N����LTE�ڑ����s
#define		ERR_EC_ERR_SETTING_FAIL		12					// [12]���σ��[�_�N�����ݒ�ُ�
// MH810103 GG119202(E) �ُ�f�[�^�t�H�[�}�b�g�ύX
// MH810103 GG119202(S) ���σ��[�_����̍ċN���v�����L�^����
#define		ERR_EC_REBOOT				13					// [13]���σ��[�_�ċN��
// MH810103 GG119202(E) ���σ��[�_����̍ċN���v�����L�^����
#define		ERR_EC_RCV_SETT_ERR_100		20					// [20]���σ��[�_�n�ڍ׃G���[�R�[�h100��M(�n�[�h�E�F�A�ُ�)
#define		ERR_EC_RCV_SETT_ERR_101		21					// [21]���σ��[�_�n�ڍ׃G���[�R�[�h101��M(JVMA�h���C�o�ُ�)
#define		ERR_EC_RCV_SETT_ERR_200		22					// [22]���σ��[�_�n�ڍ׃G���[�R�[�h200��M(���σ��[�_H/W�ُ�)
#define		ERR_EC_RCV_SETT_ERR_201		23					// [23]���σ��[�_�n�ڍ׃G���[�R�[�h201��M(�I�����i�f�[�^�t�H�[�}�b�g�s����)
#define		ERR_EC_RCV_SETT_ERR_300		24					// [24]���σ��[�_�n�ڍ׃G���[�R�[�h300��M(�\�E�g�E�F�A�ُ�)
#define		ERR_EC_RCV_SETT_ERR_301		25					// [25]���σ��[�_�n�ڍ׃G���[�R�[�h301��M(�Z���^�[�ʐM�ُ�)
#define		ERR_EC_RCV_SETT_ERR_304		26					// [26]���σ��[�_�n�ڍ׃G���[�R�[�h304��M(���[�_�ڑ����s)
#define		ERR_EC_RCV_SETT_ERR_305		27					// [27]���σ��[�_�n�ڍ׃G���[�R�[�h305��M(�Í������[�h�ݒ莸�s)
#define		ERR_EC_RCV_SETT_ERR_306		28					// [28]���σ��[�_�n�ڍ׃G���[�R�[�h306��M(�J�[�h�ǂݎ�莸�s)
#define		ERR_EC_RCV_SETT_ERR_307		29					// [29]���σ��[�_�n�ڍ׃G���[�R�[�h307��M(IC�J�[�h�ǂݎ�莸�s)
#define		ERR_EC_RCV_SETT_ERR_308		30					// [30]���σ��[�_�n�ڍ׃G���[�R�[�h308��M(�J�[�h�ǂݎ��^�C���A�E�g)
#define		ERR_EC_RCV_SETT_ERR_309		31					// [31]���σ��[�_�n�ڍ׃G���[�R�[�h309��M(�J�[�h�������)
#define		ERR_EC_RCV_SETT_ERR_311		33					// [33]���σ��[�_�n�ڍ׃G���[�R�[�h311��M(���ϊ����O�J�[�h��������)
// MH810103 GG119202(S) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
#define		ERR_EC_RCV_SETT_FAIL		34					// [34]���ό��ʎ�M���s
// MH810103 GG119202(E) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
// MH810105(S) MH364301 �݂Ȃ����σf�[�^��M�G���[�ǉ�
#define		ERR_EC_RCV_DEEMED_SETTLEMENT	35				// [35]�݂Ȃ����σf�[�^��M
// MH810105(E) MH364301 �݂Ȃ����σf�[�^��M�G���[�ǉ�
#define		ERR_EC_RCV_SETT_ERR_DES		40					// [40]�Z���^�[�n�ڍ׃G���[�R�[�hDES��M
#define		ERR_EC_RCV_SETT_ERR_CKP		41					// [41]�Z���^�[�n�ڍ׃G���[�R�[�hCKP��M
#define		ERR_EC_RCV_SETT_ERR_G		42					// [42]�Z���^�[�n�ڍ׃G���[�R�[�hG��M
#define		ERR_EC_RCV_SETT_ERR_UNREG	43					// [43]���o�^�ڍ׃G���[�R�[�h��M
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
#define		ERR_EC_RCV_EMONEY_CRITICAL_ERR	44				// [44]�Z���^�[�E�d�q�}�l�[�n�d��ڍ׃G���[�R�[�h��M
#define		ERR_EC_RCV_EMONEY_ERR		45					// [45]�Z���^�[�E�d�q�}�l�[�n�ڍ׃G���[�R�[�h��M
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�	
// MH810103 GG119202(S) ���ϒ�~�G���[�̓o�^
#define		ERR_EC_PAY_STOP				46					// [46]���ϒ�~
// MH810103 GG119202(E) ���ϒ�~�G���[�̓o�^
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
#define		ERR_EC_SETTLE_ABORT			47					// [47]���Ϗ������s
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
// MH810103 GG119202(S) JVMA���Z�b�g���G���[�o�^����
#define		ERR_EC_JVMA_RESET			48					// [48]JVMA���Z�b�g
// MH810103 GG119202(E) JVMA���Z�b�g���G���[�o�^����
// MH321800(S) �t�F�[�Y�G���[�������Ɍ���OK�ȊO�̓��[�_�[�؂藣�����Ȃ�
// MH810105(S) MH364301 E3249�𔭐��^�����ʂɕύX
//#define		ERR_EC_NOTOK_SETTLE_ERR		49					// [49]���ό��ʎ�M�G���[
#define		ERR_EC_NOTOK_SETTLE_ERR		49					// [49]���ό��ʎ�M�G���[�P
// MH810105(E) MH364301 E3249�𔭐��^�����ʂɕύX
// MH321800(E) �t�F�[�Y�G���[�������Ɍ���OK�ȊO�̓��[�_�[�؂藣�����Ȃ�
// MH321800(S) �݂Ȃ����ϔ������ɃG���[�o�^����
#define		ERR_EC_DEEMED_SETTLEMENT	50					// [50]�݂Ȃ�����
// MH321800(E) �݂Ȃ����ϔ������ɃG���[�o�^����
#define		ERR_EC_ACK4RECEIVE			51					// [51]ACK4��M
#define		ERR_EC_NAKRECEIVE			52					// [52]NAK��M
#define		ERR_EC_BUF_FULL				53					// [53]��M�o�b�t�@�t��
#define		ERR_EC_PARITY				54					// [54]��M�f�[�^�p���e�B�G���[
#define		ERR_EC_OVERRUN				55					// [55]��M�f�[�^�I�[�o�[�����G���[
#define		ERR_EC_FLAMING				56					// [56]��M�f�[�^�t���[�~���O�G���[
#define		ERR_EC_DATANUM				57					// [57]��M�������s��v�G���[
#define		ERR_EC_DATABCC				58					// [58]��M�f�[�^BCC�`�F�b�N�G���[
// MH810105(S) MH364301 E3249�𔭐��^�����ʂɕύX
//#define		ERR_EC_SETTLE_ERR			59					// [59]���ψُ픭��
#define		ERR_EC_SETTLE_ERR			59					// [59]���ό��ʎ�M�G���[�Q
// MH810105(E) MH364301 E3249�𔭐��^�����ʂɕύX
#define		ERR_EC_REINPUT				60					// [60]���͍ėv�����M
#define		ERR_EC_REOUTPUT				61					// [61]�o�͍Ďw�ߑ��M
#define		ERR_EC_PAY_ERR				62					// [62]���σ��[�_���ϕs��
#define		ERR_EC_PAYBRAND_ERR			63					// [63]���ݒ�u�����h����f�[�^��M
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//#define		ERR_EC_SETTEI_BRAND_NG		64					// [64]���σ��[�_�ݒ�s�ǁi�g�p�}�l�[�I���j
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
#define		ERR_EC_BRANDNO_ZERO			65					// [65]�u�����h�ԍ�0�f�[�^��M
#define		ERR_EC_ALARM_TRADE			66					// [66]�������菈�������^�C���A�E�g����(�S��J�[�h����)
#define		ERR_EC_ALARM_TRADE_WAON		67					// [67]�������菈�������^�C���A�E�g����(WAON)
#define		ERR_EC_SND_QUE_GAP			68					// [68]���M�L���[�̎Q�ƁE�����݃C���f�b�N�X����
#define		ERR_EC_RCV_IGL_STS_DATA		69					// [69]�z��O��MVT��ԃf�[�^��M
#define		ERR_EC_KOUTU_ABNORMAL		70					// [70]�u�����h�ُ�(��ʌn�d�q�}�l�[)
#define		ERR_EC_EDY_ABNORAML			71					// [71]�u�����h�ُ�(Edy)
#define		ERR_EC_WAON_ABNORMAL		72					// [72]�u�����h�ُ�(WAON)
#define		ERR_EC_NANACO_ABNORMAL		73					// [73]�u�����h�ُ�(nanaco)
#define		ERR_EC_ID_ABNORMAL			74					// [74]�u�����h�ُ�(iD)
#define		ERR_EC_QUICPAY_ABNORMAL		75					// [75]�u�����h�ُ�(QuicPay)
#define		ERR_EC_CREDIT_ABNORMAL		76					// [76]�u�����h�ُ�(�N���W�b�g)
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(S) MH364301 PiTaPa�Ή�
#define		ERR_EC_PITAPA_ABNORMAL		77					// [77]�u�����h�ُ�(PiTaPa)
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define		ERR_EC_QR_ABNORMAL			78					// [78]�u�����h�ُ�(QR�R�[�h)
#define		ERR_EC_SUB_BRAND_FAILURE	79					// [79]�T�u�u�����h�ُ�
#define		ERR_EC_PROCESS_FAILURE		80					// [80]���Ϗ������s
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
#define		ERR_EC_HOUSE_ABNORAML		0					// �G���[�ɂȂ邱�Ƃ��Ȃ�
#define		ERR_EC_TCARD_ABNORAML		0					// �G���[�ɂȂ邱�Ƃ��Ȃ�
#define		ERR_EC_HOUJIN_ABNORAML		0					// �G���[�ɂȂ邱�Ƃ��Ȃ�
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

#define		ERR_SUBCPU_CPUERR			1					// 8001:�ʐMCPU�ُ�
#define		ERR_SUBCPU_HIFSENDERR		2					// 8002:HIF�f�[�^���M�G���[
#define		ERR_SUBCPU_HIFRECVDELAY		3					// 8003:HIF�f�[�^�x����M
#define		ERR_SUBCPU_LOADERR			4					// 8004:���[�_�[���[�h�G���[
#define		ERR_SUBCPU_APPENVERR		5					// 8005:�A�v�������[�h�G���[
#define		ERR_SUBCPU_APPERR			6					// 8006:�A�v���P�[�V�������[�h�G���[
#define		ERR_SUBCPU_CHECKSUMERR		7					// 8007:�`�F�b�N�T���G���[
#define		ERR_SUBCPU_RTRYERR			8					// 8008:���[�h���g���C�G���[
#define		ERR_SUBCPU_BKMEMERR			9					// 8009:�o�b�N�A�b�v�������ُ�
#define		ERR_SUBCPU_FTP_COM_TIMEOUT	15					// 8015:FTP�ʐM�^�C���A�E�g����
#define		ERR_SUBCPU_NTPNOANS			20					// 8020:NTP�T�[�o�[�ʐM�s��
#define		ERR_SUBCPU_SNTPTIMOUT		21					// 8021:SNTP���N�G�X�g�^�C���A�E�g
#define		ERR_SUBCPU_FTPSVRFAIL		30					// 8030:FTP�T�[�o�[�ڑ����s
#define		ERR_CARD_T					2					// �s�J�[�h�G���[
#define		ERR_CARD_H					3					// �@�l�J�[�h�G���[
#define		ERR_FLASH_WRITE				4					// �t���b�V���������݃G���[
#define		ERR_TCARD_CHK_NG			5					// T�J�[�h����NG
#define		ERR_CAN_COMFAIL				1					// E8601:CAN �ʐM�s��
#define		ERR_CAN_STUFF_ERR			5					// E8605:CAN �X�^�b�t�G���[
#define		ERR_CAN_FORM_ERR			6					// E8606:CAN �t�H�[���G���[
#define		ERR_CAN_CRC_ERR				7					// E8607:CAN CRC�G���[
#define		ERR_CAN_LOST_ERR			8					// E8608:CAN �I�[�o�[���[�h�i���b�Z�[�W���X�g�j�����G���[
#define		ERR_CAN_NOANSWER_02			12					// E8612:CAN �m�[�h2�����Ȃ�

#define		ERR_FLAPLOCK_COMFAIL		1					// ���M���g���C�I�[�o�[�i�ʐM�ُ�j
#define		ERR_FLAPLOCK_LOCKCLOSEFAIL	38					// ���b�N���u�^�t���b�v�㏸����Ń��b�N
#define		ERR_FLAPLOCK_LOCKOPENFAIL	39					// ���b�N���u�J�^�t���b�v���~����Ń��b�N
#define		ERR_FLAPLOCK_DOWNRETRYOVER	40					// �t���b�v���~����Ń��b�N���̕ی쏈�����g���C�I�[�o�[
#define		ERR_FLAPLOCK_DOWNRETRY		41					// �t���b�v���~����Ń��b�N���̕ی쏈���J�n
#define		ERR_FLAPLOCK_STATENG		52					// �t���b�v�^���b�N��ԋK��O
#define		ERR_FLAPLOCK_EVENTOUTOFRULE	53					// �t���b�v�^���b�N�C�x���g�K��O
#define		ERR_FLAPLOCK_PARITYERROR	60					// �p���e�B�G���[
#define		ERR_FLAPLOCK_FLAMERROR		61					// �t���[�~���O�G���[
#define		ERR_FLAPLOCK_OVERRUNERROR	62					// �I�[�o�[�����G���[
#define		ERR_FLAPLOCK_HEADERERROR	63					// �d���ُ�i�w�b�_�����j
#define		ERR_FLAPLOCK_LENGTHERROR	64					// �d���ُ�i�f�[�^��NG�j
#define		ERR_FLAPLOCK_IDERROR		65					// �d���ُ�i�f�[�^IDNG�j
#define		ERR_FLAPLOCK_NAKRYOVER		66					// NAK���g���C�I�[�o�[
#define		ERR_FLAPLOCK_CRC_ERR		67					// CRC�ُ�
#define		ERR_FLAPLOCK_NOHEADER		68					// �d���ُ�i�w�b�_������M�j
#define		ERR_FLAPLOCK_PLOTOCOLERR	69					// �v���g�R���ُ�
#define		ERR_FLAPLOCK_RESERR			70					// ����Ⴂ
#define		ERR_FLAPLOCK_QUEFULL1		71					// �L���[�t���i�q�@��ԃ��X�g�j
#define		ERR_FLAPLOCK_QUEFULL2		72					// �L���[�t���i���b�N����҂����X�g�j
#define		ERR_FLAPLOCK_QUEFULL3		73					// �L���[�t���i�q�@�Z���N�e�B���O�҂����X�g�j
#define		ERR_FLAPLOCK_DATAERR		74					// ����f�[�^�ُ�
#define		ERR_FLAPLOCK_TYPEDISCODE	75					// ���u��ʕs��v

// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#define		ERR_CC_PAYDATE_INVALID			21				// ���Z��������
#define		ERR_CC_ENTRYDATE_INVALID		22				// ���ɓ��������i����j
#define		ERR_CC_ENTRYDATE_INVALID_OLD	23				// ���ɓ��������i�Đ��Z�j
#define		ERR_CC_PAYDATE_INVALID_OLD		24				// �O�񐸎Z���������i�Đ��Z�j
#define		ERR_CC_OVER1YEARCAL				41				// 1�N�ȏ㗿���v�Z�i���Ɂj
#define		ERR_CC_OVER1YEARCAL_OLD			42				// 1�N�ȏ㗿���v�Z�i�O�񐸎Z�j
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

#define		ERR_SODIAC_STOP_ERR			5					//�ُ�I��
#define		ERR_SODIAC_DATA_ERR			6					//�f�[�^�ُ�
#define		ERR_SODIAC_MEM_ERR			7					//�������ُ�
#define		ERR_SODIAC_PRM_ERR			8					//�p�����[�^�ُ�
#define		ERR_SODIAC_STS_ERR			9					//��Ԉُ�
#define		ERR_SODIAC_NOTSP_ERR		10					//���T�|�[�g�ُ�
#define		ERR_SODIAC_OTHER_ERR		11					//���̑��ُ�
#define		ERR_SODIAC_RETRY_ERR		15					//���M���g���C�I�[�o�[				
//Sodiac notice id
#define		ERR_SOIDAC_STOP_ID			0					//�Đ���~
#define		ERR_SOIDAC_NEXT_ID			1					//����t���[�Y�v��
#define		ERR_SOIDAC_ERROR_ID			2					// �G���[
#define		ERR_SOIDAC_PWMON_ID 		3					// PWM �o�͊J�n
#define		ERR_SOIDAC_PWMOFF_ID 		4					// PWM �o�͒�~
#define		ERR_SOIDAC_MUTEOFF_ID 		5					// �~���[�g����
#define		ERR_SOIDAC_MUTEON_ID 		6					// �~���[�g�J�n
#define		ERR_SOIDAC_NOT_ID 			10					// �ʒm�Ȃ�
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
#define		ERR_TKLSLCD_COMM_FAIL		1					// ���Z�@�|LCD�ԒʐM�s��(�����F�R�l�N�V�����ؒf�^�����F�R�l�N�V�����ڑ�)
#define		ERR_TKLSLCD_RMT_RST			2					// ���肩��̃��Z�b�g(�����FTM_CB_RESET���o�^�����F�R�l�N�V�����ڑ�)
#define		ERR_TKLSLCD_RMT_DSC			3					// ���肩��̐ؒf(�����FTM_CB_REMOTE_CLOSE���o�^�����F�R�l�N�V�����ڑ�)
#define		ERR_TKLSLCD_ERR_OCC			4					// �G���[����(�����FTM_CB_SOCKET_ERROR���o�^�����F�R�l�N�V�����ڑ�)
#define		ERR_TKLSLCD_CON_RES_TO		5					// �R�l�N�V���������҂��^�C���A�E�g(�����F�R�l�N�V���������҂��^�C���A�E�g���o�^�����F�R�l�N�V�����ڑ�)
#define		ERR_TKLSLCD_KPALV_RTY_OV	6					// �L�[�v�A���C�u���g���C�I�[�o�[(�����F�L�[�v�A���C�u���g���C�I�[�o�[�����^�����F�R�l�N�V�����ڑ�)
#define		ERR_TKLSLCD_KSG_SOC			10					// kasago�G���[(socket())(�����^��������)
#define		ERR_TKLSLCD_KSG_BLK_ST		11					// kasago�G���[(tfBlockingState())(�����^��������)
#define		ERR_TKLSLCD_KSG_SOC_OP		12					// kasago�G���[(setsockopt())(�����^��������)
#define		ERR_TKLSLCD_KSG_SOC_CB		13					// kasago�G���[(tfRegisterSocketCB())(�����^��������)
#define		ERR_TKLSLCD_KSG_BND			14					// kasago�G���[(bind())(�����^��������)
#define		ERR_TKLSLCD_KSG_CON			15					// kasago�G���[(Connect())(�����^��������)
#define		ERR_TKLSLCD_KSG_SND			16					// kasago�G���[(���M�G���[(send())(�����^��������)
#define		ERR_TKLSLCD_KSG_RCV			17					// kasago�G���[(��M�G���[(recv())(�����^��������)
#define		ERR_TKLSLCD_KSG_CLS			18					// kasago�G���[(tfClose()�j(�����^��������)
#define		ERR_TKLSLCD_SND_FORCED_ACK	30					// ����ACK���M(��M�f�[�^�j��(�����^��������)
#define		ERR_TKLSLCD_SND_RTRY_OV		31					// �f�[�^�đ����g���C�I�[�o�[(�����^��������)
#define		ERR_TKLSLCD_SND_BUF_RPT		32					// ���M�o�b�t�@���[�h�|�C���^�ُ�(�����^��������)
#define		ERR_TKLSLCD_SND_BUF_WPT		33					// ���M�o�b�t�@���C�g�|�C���^�ُ�(�����^��������)
#define		ERR_TKLSLCD_SND_BUF_FUL		34					// ���M�o�b�t�@�t��(�����^��������)
#define		ERR_TKLSLCD_PAY_RES_TO		40					// ���Z�����f�[�^�҂��^�C���A�E�g(�����^��������)
#define		ERR_TKLSLCD_QR_RES_TO		41					// QR�m��E��������f�[�^�҂��^�C���A�E�g(�����^��������)
#define		ERR_TKLSLCD_RCV_FORCED_ACK	50					// ����ACK��M(���M�f�[�^�j��(�����^��������)
#define		ERR_TKLSLCD_RCV_HDR			51					// ��M�f�[�^�w�b�_�G���[(�����^��������)
#define		ERR_TKLSLCD_RCV_CRC			52					// ��M�f�[�^CRC�G���[(�����^��������)
#define		ERR_TKLSLCD_RCV_LENGTH		53					// ��M�f�[�^���G���[(�����^��������)
#define		ERR_TKLSLCD_RCV_SEQ_NO		54					// ��M�f�[�^�V�[�P���X���G���[(�����^��������)
#define		ERR_TKLSLCD_RCV_DCRYPT_FL	55					// ��M�f�[�^�������G���[(�����^��������)
#define		ERR_TKLSLCD_PRM_UPLD_FL		70					// �p�����[�^�A�b�v���[�h���s

// DC-NET�ʐM(28)
#define		ERR_BATCH_LAN_UP_HOST_COMM_ERROR		1		// �y������zHOST���ʐM�G���[
#define		ERR_BATCH_LAN_UP_REMOTE_CRC16_ERROR		10		// �y������z��M�f�[�^�ُ�
#define		ERR_BATCH_LAN_UP_SEND_RETRY_OVER		11		// �y������z�p�P�b�g���M���g���C�I�[�o�[
#define		ERR_BATCH_LAN_UP_NTNET_BLOCK_ERROR		13		// �y������z�m�s�|�m�d�s�u���b�N�ԍ��ُ�

// �o�[�R�[�h(30)
#define		ERR_BARCODE_COMM_FAIL		5					// �o�[�R�[�h�X�L���i�ʐM�s��
#define		ERR_BARCODE_PARAM_FAIL		10					// �p�����[�^�ݒ莸�s
#define		ERR_BARCODE_OVER_RUN		15					// �V���A���ʐM�F�I�[�o�[�����G���[(�����^��������)
#define		ERR_BARCODE_FLAMING_ERR		16					// �V���A���ʐM�F�t���[�~���O�G���[(�����^��������)
#define		ERR_BARCODE_PARITY_ERR		17					// �V���A���ʐM�F�p���e�B�G���[(�����^��������)
#define		ERR_BARCODE_BUFF_OVER_FLOW	18					// �V���A���ʐM�F�o�b�t�@�I�[�o�[�t���[(�����^��������)

// ���A���^�C���ʐM(37)
#define		ERR_RELT_UP_COM_FAIL				1			//�y������zHOST���ʐM�G���[
#define		ERR_RELT_UP_RECV_NACK99				3			//�y������z�f�[�^�폜������M
#define		ERR_RELT_UP_NACK10_RETRY_OVER		4			//�y������z�f�[�^�폜�������M�iNACK10���g���C�I�[�o�[�j
#define		ERR_RELT_UP_NACK12_RETRY_OVER		5			//�y������z�f�[�^�폜�������M�iNACK12���g���C�I�[�o�[�j
#define		ERR_RELT_UP_NACK13_RETRY_OVER		6			//�y������z�f�[�^�폜�������M�iNACK13���g���C�I�[�o�[�j
#define		ERR_RELT_UP_SEND_RETRY_OVER			14			//�y������z�p�P�b�g���M���g���C�I�[�o�[
#define		ERR_RELT_DOWN_NO_ACT_TIMEOUT		51			//�y�������z��M�Ď��^�C���A�E�g
#define		ERR_RELT_DOWN_RECV_NACK99			52			//�y�������z�f�[�^�폜������M
#define		ERR_RELT_DOWN_NACK10_RETRY_OVER		53			//�y�������z�f�[�^�폜�������M�iNACK10���g���C�I�[�o�[�j
#define		ERR_RELT_DOWN_NACK12_RETRY_OVER		54			//�y�������z�f�[�^�폜�������M�iNACK12���g���C�I�[�o�[�j
#define		ERR_RELT_DOWN_NACK13_RETRY_OVER		55			//�y�������z�f�[�^�폜�������M�iNACK13���g���C�I�[�o�[�j
#define		ERR_RELT_DOWN_SEND_RETRY_OVER		58			//�y�������z�p�P�b�g���M���g���C�I�[�o�[

// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
#define		ERR_I2C_R_W_MODE			4		// Read/Write���[�h�G���[
#define		ERR_I2C_CLOCK_LEVEL			5		// �N���b�N���x���G���[
#define		ERR_I2C_SEND_DATA_LEVEL		6		// ���M�f�[�^���x���G���[
#define		ERR_I2C_BUS_BUSY			7		// �o�XBUSY
#define		ERR_I2C_PREV_TOUT			8		// �O��X�e�[�^�X�ω��҂��^�C���A�E�g
//-----------------------------------------------
#define		ERR_I2C_BFULL_SEND_RP		10		// ���V�[�g�v�����^���M�o�b�t�@�t��
#define		ERR_I2C_BFULL_RECV_RP		11		// ���V�[�g�v�����^��M�o�b�t�@�t��
#define		ERR_I2C_BFULL_SEND_JP		12		// �W���[�i���v�����^���M�o�b�t�@�t��
#define		ERR_I2C_BFULL_RECV_JP		13		// �W���[�i���v�����^��M�o�b�t�@�t��
#define		ERR_I2C_BFULL_SEND_CAPPI	14		// �\��G���A�iCAPPI���M�o�b�t�@�t��
#define		ERR_I2C_BFULL_RECV_CAPPI	15		// �\��G���A�iCAPPI��M�o�b�t�@�t��
#define		ERR_I2C_BFULL_SEND_PIP		16		// �\��G���A�iPIP���M�o�b�t�@�t���j
#define		ERR_I2C_BFULL_RECV_PIP		17		// �\��G���A�iPIP��M�o�b�t�@�t���j

#define		ERR_CAPPI_PARITY_ERR		50		// �p���e�B�G���[
#define		ERR_CAPPI_FRAMING_ERR		51		// �t���[�~���O�G���[
#define		ERR_CAPPI_OVERRUN_ERR		52		// �I�[�o�[�����G���[

#define		ERR_REMOTE_SCI_PARITY		60		// �V���A���p���e�B�G���[
#define		ERR_REMOTE_SCI_FRAME		61		// �V���A���t���[�~���O�G���[
#define		ERR_REMOTE_SCI_OVERRUN		62		// �V���A���I�[�o�[�����G���[

//RAU
#define		ERR_RAU_HOST_COMMUNICATION		1				// �G���[�R�[�h�O�P(������ʐM�G���[)
#define		ERR_RAU_RCV_SEQ_TIM_OVER		10				// �G���[�R�[�h�P�O(��M�V�[�P���X�t���O���X�|���X�^�C���A�E�g)
#define		ERR_RAU_REQ_COUNT_TIMEOUT		24				// ���ݒ��ԑ䐔�v���^�C���A�E�g(HOST�ւ̏�񑗐M�Ɏg�p)
#define		ERR_RAU_FMA_ANTENALOW1			31				// (31)�ڑ����̱������ق� 1
#define		ERR_RAU_FMA_ANTENALOW2			32				// (32)�ڑ����̱������ق� 2
#define		ERR_RAU_DPA_RECV_CRC			54				// �G���[�R�[�h�T�S(�������M�f�[�^�b�q�b�G���[)
#define		ERR_RAU_DPA_RECV_LONGER			55				// �G���[�R�[�h�T�T(�������M�f�[�^���ُ�)
#define		ERR_RAU_DPA_RECV_SHORTER		56				// �G���[�R�[�h�T�U(�������M�f�[�^���ُ�)
#define		ERR_RAU_TCP_CONNECT				91				// �G���[�R�[�h�X�P(������s�b�o�R�l�N�V�����^�C���A�E�g)
#define		ERR_RAU_DPA_SEND_LEN			92				// �G���[�R�[�h�X�Q(��������M�f�[�^���I�[�o�[)
#define		ERR_RAU_DPA_RECV_LEN			93				// �G���[�R�[�h�X�R(�������M�f�[�^���I�[�o�[)
#define		ERR_RAU_DPA_CALL_RETRY_OVER		94				// �G���[�R�[�h�X�S(�Ĕ��ĉ񐔃I�[�o�[)
#define		ERR_RAU_DPA_SEND_RETRY_OVER		95				// �G���[�R�[�h�X�T(������f�[�^�đ��񐔃I�[�o�[)
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
#define		ERR_RAU_LONGPARK_BUFERFULL		57				// �������ԃf�[�^�o�b�t�@�t��
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
#define		ERR_RAU_TURI_BUFERFULL			58				// �ޑK�Ǘ��W�v�f�[�^�o�b�t�@�t��
#define		ERR_RAU_GTTOTAL_BUFERFULL		59				// GT���v�f�[�^�o�b�t�@�t��
#define		ERR_RAU_ENTRY_BUFERFULL			60				// ���Ƀf�[�^�o�b�t�@�t��
#define		ERR_RAU_PAYMENT_BUFERFULL		63				// ���Z�i�o���j�f�[�^�o�b�t�@�t��
#define		ERR_RAU_TOTAL_BUFERFULL			64				// T���v�f�[�^�o�b�t�@�t��
#define		ERR_RAU_ERROR_BUFERFULL			65				// �G���[�f�[�^�o�b�t�@�t��
#define		ERR_RAU_ALARM_BUFERFULL			66				// �A���[���f�[�^�o�b�t�@�t��
#define		ERR_RAU_MONITOR_BUFERFULL		67				// ���j�^�f�[�^�o�b�t�@�t��
#define		ERR_RAU_OPE_MONITOR_BUFERFULL	68				// ���샂�j�^�f�[�^�o�b�t�@�t��
#define		ERR_RAU_COIN_BUFERFULL			69				// �R�C�����ɌɃf�[�^�o�b�t�@�t��
#define		ERR_RAU_NOTE_BUFERFULL			70				// �������Ƀf�[�^�o�b�t�@�t��
#define		ERR_RAU_PARK_CNT_BUFERFULL		71				// ���ԑ䐔�f�[�^�o�b�t�@�t��
#define		ERR_RAU_RMON_BUFERFULL			72				// ���u�Ď��f�[�^�o�b�t�@�t��
#define		ERR_RAU_FMA_MODEMERR_DR			74				// (74)DR�M���� 3�b�o�߂��Ă�ON�ɂȂ�Ȃ�(MODEM ON��) //
#define		ERR_RAU_FMA_MODEMERR_CD			75				// (75)CD�M����60�b�o�߂��Ă�OFF�ɂȂ�Ȃ�(MODEM OFF��) //
#define		ERR_RAU_FMA_PPP_TIMEOUT 		76				// (76)PPP�R�l�N�V�����^�C���A�E�g //
#define		ERR_RAU_FMA_RESTRICTION			77				// (77)RESTRICTION(�K����)����M //
#define		ERR_RAU_FMA_MODEMPOWOFF			78				// (78)MODEM �d��OFF //
#define		ERR_RAU_FMA_ANTENALOW			79				// (79)�ڑ����̱������ق� 0 //
#define		ERR_RAU_DPA_RECV_NAK99			97				// �G���[�R�[�h97(��������M�f�[�^������~��M)
// MH322914 (s) kasiyama 2016/07/12 ���uNT-NETLAN�ڑ����̃G���[�R�[�h��FOMA�ڑ����̃G���[�R�[�h�ɍ��킹��[���ʃo�ONo.1200](MH341106)
//#define		ERR_RAU_DPA_BLK_VALID_D			151				// �G���[�R�[�h151(��������M�f�[�^�u���b�N�ԍ��ُ�[����L��])
//#define		ERR_RAU_DPA_BLK_INVALID_D		152				// �G���[�R�[�h152(��������M�f�[�^�u���b�N�ԍ��ُ�[���񖳌�])
//#define		ERR_RAU_DPA_RECV_CRC_D			154				// �G���[�R�[�h154(��������M�f�[�^�b�q�b�G���[)
//#define		ERR_RAU_DPA_RECV_LONGER_D		155				// �G���[�R�[�h155(��������M�f�[�^���ُ�)
//#define		ERR_RAU_DPA_RECV_SHORTER_D		156				// �G���[�R�[�h156(��������M�f�[�^���ُ�)
//#define		ERR_RAU_DPA_SEND_LEN_D			192				// �G���[�R�[�h192(���������M�f�[�^���I�[�o�[)
//#define		ERR_RAU_DPA_RECV_LEN_D			193				// �G���[�R�[�h193(��������M�f�[�^���I�[�o�[)
//#define		ERR_RAU_DPA_SEND_RETRY_OVER_D	195				// �G���[�R�[�h195(�������f�[�^�đ��񐔃I�[�o�[)
//#define		ERR_RAU_DPA_RECV_NAK99_D		197				// �G���[�R�[�h197(���������M�f�[�^������~��M)
//#define		ERR_RAU_FMA_BLK_VALID_D			80				// FOMA�G���[�R�[�h�W�O(��������M�f�[�^�u���b�N�ԍ��ُ�[����L��])
//#define		ERR_RAU_FMA_BLK_INVALID_D		81				// FOMA�G���[�R�[�h�W�P(��������M�f�[�^�u���b�N�ԍ��ُ�[���񖳌�])
//#define		ERR_RAU_FMA_RECV_CRC_D			82				// FOMA�G���[�R�[�h�W�Q(��������M�f�[�^�b�q�b�G���[)
//#define		ERR_RAU_FMA_RECV_LONGER_D		83				// FOMA�G���[�R�[�h�W�R(��������M�f�[�^���ُ�)
//#define		ERR_RAU_FMA_RECV_SHORTER_D		84				// FOMA�G���[�R�[�h�W�S(��������M�f�[�^���ُ�)
//#define		ERR_RAU_FMA_SEND_LEN_D			85				// FOMA�G���[�R�[�h�W�T(���������M�f�[�^���I�[�o�[)
//#define		ERR_RAU_FMA_RECV_LEN_D			86				// FOMA�G���[�R�[�h�W�U(��������M�f�[�^���I�[�o�[)
//#define		ERR_RAU_FMA_SEND_RETRY_OVER_D	87				// FOMA�G���[�R�[�h�W�V(�������f�[�^�đ��񐔃I�[�o�[)
//#define		ERR_RAU_FMA_RECV_NAK99_D		88				// FOMA�G���[�R�[�h�W�W(���������M�f�[�^������~��M)
//#define		ERR_RAU_FMA_RECV_NAK99			97				// FOMA�G���[�R�[�h�X�V(��������M�f�[�^������~��M)
#define		ERR_RAU_DPA_BLK_VALID_D			80				// �G���[�R�[�h�W�O(��������M�f�[�^�u���b�N�ԍ��ُ�[����L��])
#define		ERR_RAU_DPA_BLK_INVALID_D		81				// �G���[�R�[�h�W�P(��������M�f�[�^�u���b�N�ԍ��ُ�[���񖳌�])
#define		ERR_RAU_DPA_RECV_CRC_D			82				// �G���[�R�[�h�W�Q(��������M�f�[�^�b�q�b�G���[)
#define		ERR_RAU_DPA_RECV_LONGER_D		83				// �G���[�R�[�h�W�R(��������M�f�[�^���ُ�)
#define		ERR_RAU_DPA_RECV_SHORTER_D		84				// �G���[�R�[�h�W�S(��������M�f�[�^���ُ�)
#define		ERR_RAU_DPA_SEND_LEN_D			85				// �G���[�R�[�h�W�T(���������M�f�[�^���I�[�o�[)
#define		ERR_RAU_DPA_RECV_LEN_D			86				// �G���[�R�[�h�W�U(��������M�f�[�^���I�[�o�[)
#define		ERR_RAU_DPA_SEND_RETRY_OVER_D	87				// �G���[�R�[�h�W�V(�������f�[�^�đ��񐔃I�[�o�[)
#define		ERR_RAU_DPA_RECV_NAK99_D		88				// �G���[�R�[�h�W�W(���������M�f�[�^������~��M)
// MH322914 (e) kasiyama 2016/07/12 ���uNT-NETLAN�ڑ����̃G���[�R�[�h��FOMA�ڑ����̃G���[�R�[�h�ɍ��킹��[���ʃo�ONo.1200](MH341106)
#define		ERR_CREDIT_HOST_COMMUNICATION		1			// �N���W�b�g����ʐM�G���[(HOST�ُ�)
#define		ERR_CREDIT_SND_BUFFERFULL			57			// ���M�f�[�^�o�b�t�@�t��
#define		ERR_CREDIT_OPEN_ERR					60			// �J�ǈُ�
#define		ERR_CREDIT_NO_ACCEPT				61			// ���㋑�ۃf�[�^
#define		ERR_CREDIT_CONNECTCHECK_ERR			62			// �����m�F���s
#define		ERR_CREDIT_DPA_RECV_CRC				82			// �G���[�R�[�h�T�S(�����M�f�[�^�b�q�b�G���[)
#define		ERR_CREDIT_DPA_RECV_LONGER			83			// �G���[�R�[�h�T�T(�����M�f�[�^���ُ�)
#define		ERR_CREDIT_DPA_RECV_SHORTER			84			// �G���[�R�[�h�T�U(�����M�f�[�^���ُ�)
#define		ERR_CREDIT_FMA_SEND_LEN				85			// FOMA�G���[�R�[�h�W�T(������M�f�[�^���I�[�o�[)
#define		ERR_CREDIT_FMA_RECV_LEN				86			// FOMA�G���[�R�[�h�W�U(�����M�f�[�^���I�[�o�[)
#define		ERR_CREDIT_FMA_SEND_RETRY_OVER		87			// FOMA�G���[�R�[�h�W�V(����f�[�^�đ��񐔃I�[�o�[)
#define		ERR_CREDIT_FMA_RECV_NAK99			88			// FOMA�G���[�R�[�h�W�W(������M�f�[�^������~��M)
#define		ERR_CREDIT_NAK_RETRYOVER			89			// NAK��M�ɂ�郊�g���C�I�[�o�[
#define		ERR_CREDIT_TCP_CONNECT				91			// �G���[�R�[�h�X�P(������s�b�o�R�l�N�V�����^�C���A�E�g)


/*[]----------------------------------------------------------------------[]*/
/*| �װ���ޕ\Ӽޭ�ٺ��ނɑΉ��������ޯ��                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Used By static const char moj_no[99] & ERR_CHK[ Index No. ]            |*/
/*|                                                                        |*/
/*| ERR_CHK[mod_note][]                                                    |*/
/*| mod_note�� moj_no[�װ���ޕ\Ӽޭ�ٺ���]�Ɋi�[����Ă���                 |*/
/*|                                                                        |*/
/*| �װ���ޕ\Ӽޭ�ٺ��ނ���Ή�����ERR_CHK�𓱂��ꍇ                       |*/
/*| ERR_CHK[moj_no[�װ���ޕ\Ӽޭ�ٺ���]][]�ƂȂ�                           |*/
/*[]----------------------------------------------------------------------[]*/
#define		mod_main		0								/* Ҳݐ���						*/
#define		mod_read		1								/* ���Cذ�ް					*/
#define		mod_prnt		2								/* ڼ��,�ެ���					*/
#define		mod_coin		3								/* ��ݎ��ʏz��					*/
#define		mod_note		4								/* ����ذ�ް					*/
#define		mod_ifrock		5								/* IF�եۯ����u					*/
#define		mod_mifr		6								/* Mifear						*/
#define		mod_ntnet		7								/* NT-NET						*/
#define		mod_ntibk		8								/* NT-NET IBK					*/
#define		mod_lprn		9								/* ���������					*/
#define		mod_ifflap		10								/* IF�ե�ׯ�ߑ��u					*/
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//#define		mod_cct			11								/* IBK(CCT)	(ModCode=54)*/
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
enum {
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//	mod_lanibk = mod_cct+1,							/* IBK(LAN) (ModCode=72)*/
	mod_lanibk = mod_ifflap+1,						/* IBK(LAN) (ModCode=72)*/
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
	mod_dopaibk,									/* IBK(Dopa)(ModCode=73)*/
	// ���� �V���W���[���͂����ɒ�`��ǉ�
	mod_Suica,										/* Suica (ModCode=69)		*/
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	mod_Edy,  										/* Edy (ModCode=62)			*/
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	mod_remote,
	mod_fomaibk,									/* IBK(FOMA)(ModCode=77)*/

	mod_ntmf,										/* NTNET-DOPA�q�@MAF(ModCode=65)*/
	mod_ntdp,										/* NTNET-DOPA�q�@	(ModCode=67)*/
	mod_cappi,										/* Cappi	(ModCode=74)*/
	mod_idc,										/* iD		(ModCode=75)*/
	mod_crrflap,									/* CRR��E�t���b�v���u(ModCode=15)*/
	mod_card,
	mod_flapcrb,
	mod_can,								// CAN�ʐM
	mod_sodiac,								// ���������isodiac)
	mod_I2c,
// MH321800(S) G.So IC�N���W�b�g�Ή�
	mod_ec,									// ���σ��[�_
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2019/12/12 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	mod_tklslcd,							// �Ԕԃ`�P�b�g���XLCD�ʐM
	mod_barcode,							// �o�[�R�[�h
	mod_dc_net,								// DC-NET�ʐM
	mod_realtime,							// ���A���^�C���ʐM
// MH810100(E) K.Onodera 2019/12/12 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) S.Fujii 2020/08/24 #4609 SD�J�[�h�G���[�Ή�
	mod_sd,
// MH810100(E) S.Fujii 2020/08/24 #4609 SD�J�[�h�G���[�Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	mod_eja,								// �d�q�W���[�i��
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	mod_cc,									// �N���E�h�ŗ����v�Z�G���W��
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

	mod_SubCPU,								// ���CPU�@�\
	mod_Credit,
	ERR_MOD_ALL,
	mod_lanibk2 = ERR_MOD_ALL,
	mod_dopaibk2,
	ERR_MOD_MAX,
};

#define		ERR_NUM_MAX		100								/* �װ���ސ�MAX					*/
#define		HASSEI_MAX		33

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�)_�ύX)))
//#define		ALM_MOD_MAX		3								/* �װ�Ӽޭ�ِ�MAX				*/
enum{
	alm_mod_main,
	alm_mod_sub1,
	alm_mod_sub2,
	alm_mod_auth,
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	alm_mod_cc,
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// GG124100(S) R.Endo 2022/08/29 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
#ifdef DEBUG_ALARM_ACT
	alm_mod_debug,
#endif
// GG124100(E) R.Endo 2022/08/29 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
	ALM_MOD_MAX
};
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�)_�ύX))
#define		ALM_NUM_MAX		100								/* �װѺ��ސ�MAX				*/

extern	char			ERR_CHK[ERR_MOD_MAX][ERR_NUM_MAX];	// �װ��
extern	char			ALM_CHK[ALM_MOD_MAX][ALM_NUM_MAX];	// �װя�

typedef struct{													/* �G���[�E�A���[���Ώۗp�\���� */
	int		kind;												/* �G���[�E�A���[����� */
	uchar	code;												/* ��ʖ��R�[�h */
}CERTIFY_ERR_ARM;

// MH322914(S) K.Onodera 2016/09/07 AI-V�Ή��F�G���[�A���[��
extern ulong	ERR_INFO[ERR_MOD_MAX][ERR_NUM_MAX];		// �������G���[���
extern ulong	ALM_INFO[ALM_MOD_MAX][ALM_NUM_MAX];		// �������A���[�����
extern ulong	ERR_LOCK_INFO[LOCK_MAX][4];				// �㏸/���~�G���[���;
// MH322914(E) K.Onodera 2016/09/07 AI-V�Ή��F�G���[�A���[��

/*[]----------------------------------------------------------------------[]*/
/*| �װѐ���                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| �װѺ��ޕ\ Ӽޭ�ٺ��ޒ�`                                              |*/
/*| alm_chk(md,,)�̑�P����md�Ƃ��Ďg�p����                                |*/
/*[]----------------------------------------------------------------------[]*/
#define		ALMMDL_MAIN					0					/* Ҳݐ���Ӽޭ�ٺ���			*/
#define		ALMMDL_SUB					1					/* ��ސ���Ӽޭ�ٺ���			*/
#define		ALMMDL_SUB2					2					/* ��ސ���Ӽޭ�ٺ���2			*/
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// // MH810100(S)
// #define		ALMMDL_AUTH					5					// �F�؋@�֘A
// // MH810100(E)
#define		ALMMDL_CAL_CLOUD			8					// �N���E�h�ŗ����v�Z�G���W��
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

//------------------
// �װъ֘A��`
//------------------

// Used by Function alm_chk( #define, ...

// Ӽޭ�ٺ���0
#define		ALARM_DOORALARM			1						// �ޱ�x��
#define		ALARM_VIBSNS			5						// �U���ݻ
#define		ALARM_NOTERE_REMOVE		6						// ����ذ�ް�E���m
#define		ALARM_LAG_EXT_OVER		30						// ���O�^�C����������K��񐔃I�[�o�[
// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
#define		ALARM_LONG_PARKING		31						// �������Ԍ��o
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�

// Ӽޭ�ٺ���1
// ���בւ��ƏC��
#define		ALARM_JPAPEREND			1						// �ެ���������p������
#define		ALARM_RPAPEREND			2						// ڼ��������p������
//#define	ALARM_RPAPEROPEN		3						// ڼ������������
#define		ALARM_NOTESAFEFULL		6						// �������ɖ��t
#define		ALARM_COINSAFEFULL		7						// ��݋��ɖ��t
//#define	ALARM_DOOROPEN			9						// �ޱ�J
//#define	ALARM_NOTESAFEOPEN		11						// �������ɊJ
//#define	ALARM_COINSAFEOPEN		12						// ��݋��ɊJ
#define		ALARM_500COIN_EMPTY		15						// 500�~�ސ؂�
#define		ALARM_100COIN_EMPTY		16						// 100�~�ސ؂�
#define		ALARM_50COIN_EMPTY		17						// 50�~�ސ؂�
#define		ALARM_10COIN_EMPTY		18						// 10�~�ސ؂�
#define		ALARM_500COIN_NEMPTY	21						// 500�~�ސ؂�\��
#define		ALARM_100COIN_NEMPTY	22						// 100�~�ސ؂�\��
#define		ALARM_50COIN_NEMPTY		23						// 50�~�ސ؂�\��
#define		ALARM_10COIN_NEMPTY		24						// 10�~�ސ؂�\��
#define		ALARM_TURIKAN_REV		26						// �ޑK�Ǘ������␳
#define		ALARM_JPAPERNEAREND		28						// �ެ���������p��Ʊ����
#define		ALARM_RPAPERNEAREND		29						// ڼ��������p��Ʊ����
#define		ALARM_COINSAFENFULL		31						// ��݋��ɖ��t�\��
#define		ALARM_NOTESAFENFULL		32						// �������ɖ��t�\��
#define		ALARM_SALELOG_FULL		43						// ���Z۸�FULL�ɂ��ް��㏑��	�����ݏ����Ȃ��i���񖢑Ή��j
#define		ALARM_TOTALLOG_FULL		44						// �W�v۸�FULL�ɂ��ް��㏑��	�����ݏ����Ȃ��i���񖢑Ή��j
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		ALARM_SD_EXPIRATION		45						// SD�J�[�h�g�p��������
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
#define		ALARM_READERSENSOR		53						// �J�[�h���[�_�Z���T�i���x����j
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		ALARM_SD_END			55						// SD�J�[�h�G���h
#define		ALARM_SD_NEAR_END		56						// SD�J�[�h�j�A�G���h
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
#define		ALARM_LBPAPEREND		46						// ������������؂�
#define		ALARM_LBRIBBONEND		47						// ������������ݐ؂�
#define		ALARM_LBPAPERNEAREND	48						// ������������؂�\��
#define		ALARM_LBRIBBONNEAREND	49						// ������������ݐ؂�\��
#define		ALARM_RPAPEROPEN		60						// ڼ������������		(�o�^���邪Alarm���ڂƂ��Ȃ�)
#define		ALARM_JPAPEROPEN		61						// �ެ�������������		(�o�^���邪Alarm���ڂƂ��Ȃ�)
// MH810105 GG119202(S) ���[�_���璼�撆�̏�ԃf�[�^��M�����ۂ̐V�K�A���[����݂���
#define		ALARM_EC_LASTSETTERUN	84						// ���σ��[�_���p�s��
// MH810105 GG119202(E) ���[�_���璼�撆�̏�ԃf�[�^��M�����ۂ̐V�K�A���[����݂���
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#define		ALARM_EC_SETTLEMENT_TIME_GAP	89				// ���ώ����Ɛ��Z�@�̎�����1���ȏ�̃Y��������
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
#define		ALARM_CNM_IN_AMOUNT_MAX	90						// �������ő喇���I�[�o�[���̓����z�N���A���{
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		ALARM_EC_OPEN			93						// ���[�_��t���䎸�s
// MH321800(E) G.So IC�N���W�b�g�Ή�

// Ӽޭ�ٺ���2
#define		ALARM_MUKOU_PASS_USE	1						// �����o�^������g�p
#define		ALARM_PARKING_NO_NG		2						// ���ԏ�NG�����
#define		ALARM_ANTI_PASS_NG		3						// �A���`�p�XNG�����
#define		ALARM_VALIDITY_TERM_OUT	4						// �L�����ԊO�̒����
#define		ALARM_NOT_USE_TICKET	7						// �g�p�s�����
#define		ALARM_N_MINUTE_RULE		10						// n�����[�������
#define		ALARM_GT_MUKOU_PASS_USE		40						// �����o�^������g�p
#define		ALARM_GT_PARKING_NO_NG		41						// ���ԏ�NG�����
#define		ALARM_GT_ANTI_PASS_NG		42						// �A���`�p�XNG�����
#define		ALARM_GT_VALIDITY_TERM_OUT	43						// �L�����ԊO�̒����
#define		ALARM_GT_NOT_USE_TICKET		46						// �g�p�s�����
#define		ALARM_GT_N_MINUTE_RULE		49						// n�����[�������
#define		ALARM_GT_MIFARE_READ_CRC_ERR	54					// Mifare�f�[�^�s�ǁi�Ǎ���CRC�G���[�j
#define		ALARM_OUTSIDE_PERIOD_PRC	85						// �L�����ԊO������߲�޶���
#define		ALARM_RED_AFTERWRITE_PRC	86						// ����߲�޶��ޏ����ݕs��
#define		ALARM_WRITE_CRC_ERR			97						// ���C�f�[�^������CRC�G���[
#define		ALARM_READ_CRC_ERR			98						// ���C�f�[�^�Ǎ���CRC�G���[

// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// // Ӽޭ�ٺ���5
// #define		ALARM_USED_RECEIPT			30						// �������p�ς݃��V�[�g
// #define		ALARM_NOT_USE_RECEIPT		31						// �ΏۊO���V�[�g
// #define		ALARM_EXPIRED_RECEIPT		32						// �L�������؂ꃌ�V�[�g
// #define		ALARM_USE_COUNT_OVER		33						// ���V�[�g�ǂݍ��ݏ��
// #define		ALARM_DIS_PRM_NOT_SET		34						// �ΏۊO�o�[�R�[�h
// Ӽޭ�ٺ���8
#define		ALMMDL_CC_PAY_SET_ERR			1				// �����ݒ�ُ�
#define		ALMMDL_CC_PASS_SET_ERR			2				// ������ݒ�ُ�
#define		ALMMDL_CC_PAYPATTERN_SET_ERR	3				// �����p�^�[���ݒ�ُ�
#define		ALMMDL_CC_MAXCHARGE_SET_ERR		4				// ���Ԏ��ԍő嗿���ݒ�ُ�
#define		ALMMDL_CC_TIMEZONE_SET_ERR		5				// ���ԑѐݒ�ُ�
#define		ALMMDL_CC_UNITTIME_SET_ERR		6				// �P�ʎ��Ԑݒ�ُ�
#define		ALMMDL_CC_SVS_SET_ERR			7				// �T�[�r�X���ݒ�ُ�
#define		ALMMDL_CC_KAK_SET_ERR			8				// �X�����ݒ�ُ�
#define		ALMMDL_CC_TKAK_SET_ERR			9				// ���X�܊����ݒ�ُ�
#define		ALMMDL_CC_PMCSVS_SET_ERR		10				// PMC�T�[�r�X���ݒ�ُ�
#define		ALMMDL_CC_SYUBET_SET_ERR		11				// �Ԏ튄���ݒ�ُ�
#define		ALMMDL_CC_WRI_SET_ERR			12				// �������ݒ�ُ�
#define		ALMMDL_CC_UNSUPPORTED_DISC_USE	64				// ���Ή������g�p
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

//------------------
// ���엚���֘A��`
//------------------
#define		OPLOG_ALL_INITIAL		1						// �S�C�j�V�����C�Y�N��(�X�[�p�[�C�j�V����)
#define		OPLOG_DATA_INITIAL		2						// �f�[�^�C�j�V�����N��(���O�C�j�V����)
#define		OPLOG_DOOROPEN			101						// �h�A�J���엚��o�^
#define		OPLOG_DOORCLOSE			102						// �h�A���엚��o�^
#define		OPLOG_COINMECHSWOPEN	103						// �R�C�����b�NSW�J�̑��엚��o�^
#define		OPLOG_COINMECHSWCLOSE	104						// �R�C�����b�NSW�̑��엚��o�^
#define		OPLOG_NOTERDSWOPEN		105						// ����ذ�ްSW�J�̑��엚��o�^
#define		OPLOG_NOTERDSWCLOSE		106						// ����ذ�ްSW�̑��엚��o�^
#define		OPLOG_COINKINKO			107						// �R�C�����ɊJ�̑��엚��o�^
#define		OPLOG_NOTEKINKO			108						// �������ɊJ�̑��엚��o�^
#define		OPLOG_MNT_START_PASS	111						// �W������J�n�i�p�X���[�h�ɂ��J�n�j
#define		OPLOG_MNT_START_CARD	112						// �W������J�n�i�W���J�[�h�ɂ��J�n�j
#define		OPLOG_MNT_END			115						// �W������I��
#define		OPLOG_ATTRNDANT_CARD	120						// ��߰��W�����ނ��g�p���ޱۯ�����
#define		OPLOG_NO_CARD_OPEN		121						// �����J�@�\���g�p���ޱۯ�����
#define		OPLOG_DOORNOBU_LOCK		122						// �ޱ���ۯ�
#define		OPLOG_SYUSEI_START		128						// �C�����Z����J�n
#define		OPLOG_SYUSEI_END		129						// �C�����Z����I��

#define		OPLOG_TURIHOJU			1101					// ��[����L�^
#define		OPLOG_TURIHENKO			1102					// �����ύX����L�^
#define		OPLOG_TURIAUTO			1103					// AUTO����L�^
#define		OPLOG_TURICASETTE		1104					// ���Ē�����L�^
#define		OPLOG_INVENTRY			1105					// ������ؑ���L�^
#define		OPLOG_INVBUTTON			1106					// ������ؑ���L�^(�����������)
#define		OPLOG_SHOKIMAISUHENKO	1107					// ���������ύX����L�^
#define		OPLOG_TURIKAN_SHOKEI	1108					// �ޑK�Ǘ����v���엚��o�^

#define		OPLOG_T_SHOKEI			1201					// �s���v���엚��o�^
#define		OPLOG_T_GOKEI			1202					// �s���v���엚��o�^
#define		OPLOG_T_ZENGOKEI		1203					// �s���v���엚��o�^�i�Ĕ��s�j
#define		OPLOG_GT_SHOKEI			1204					// �f�s���v���엚��o�^
#define		OPLOG_GT_GOKEI			1205					// �f�s���v���엚��o�^
#define		OPLOG_GT_ZENGOKEI		1206					// �f�s���v���엚��o�^�i�Ĕ��s�j
#define		OPLOG_MT_SHOKEI			1207					// �l�s���v���엚��o�^
#define		OPLOG_MT_GOKEI			1208					// �l�s���v���엚��o�^
#define		OPLOG_MT_ZENGOKEI		1209					// �l�s���v���엚��o�^�i�Ĕ��s�j
#define		OPLOG_F_SHOKEI			1210					// �������v���엚��o�^
#define		OPLOG_F_GOKEI			1211					// �������v���엚��o�^
#define		OPLOG_F_ZENGOKEI		1212					// �������v���엚��o�^�i�Ĕ��s�j
#define		OPLOG_COIN_SHOKEI		1220					// �R�C�����ɏ��v���엚��o�^
#define		OPLOG_COIN_ZENGOUKEI	1221					// �R�C�����ɍ��v���엚��o�^�i�Ĕ��s�j
#define		OPLOG_NOTE_SHOKEI		1222					// �������ɏ��v���엚��o�^
#define		OPLOG_NOTE_ZENGOUKEI	1223					// �������ɍ��v���엚��o�^�i�Ĕ��s�j

#define		OPLOG_CAN_T_SUM			1240					// T���v���~
#define		OPLOG_CAN_GT_SUM		1241					// GT���v���~
#define		OPLOG_CAN_MT_SUM		1242					// MT���v���~

#define		OPLOG_TOKEISET			1301					// ���v���엚��o�^
#define		OPLOG_TOKEISET2			1302					// ���v���킹�i�蓮�j
#define		OPLOG_LOCK_OPEN			1501					// ���b�N���u�J
#define		OPLOG_A_LOCK_OPEN		1502					// �S���b�N���u�J
#define		OPLOG_FLAP_DOWN			1511					// �t���b�v���~
#define		OPLOG_A_FLAP_DOWN		1512					// �S�t���b�v���~
#define		OPLOG_FLAP_SENSOR_OFF	1521					// �t���b�v�@�ԗ����m�Z���T�[����OFF

#define		OPLOG_LOCK_CLOSE		1601					// ���b�N���u��
#define		OPLOG_A_LOCK_CLOSE		1602					// �S���b�N���u��
#define		OPLOG_FLAP_UP			1611					// �t���b�v�㏸
#define		OPLOG_A_FLAP_UP			1612					// �S�t���b�v�㏸
#define		OPLOG_FLAP_SENSOR_ON	1621					// �t���b�v�@�ԗ����m�Z���T�[����ON

#define		OPLOG_CHUSHADAISU		1701					// ���ԑ䐔����o�^
#define		OPLOG_MANSYACONTROL		1702					// ���ԃR���g���[������o�^
#define		OPLOG_RYOSHUSAIHAKKO	1703					// �̎��؍Ĕ��s����o�^
#define		OPLOG_UKETUKESAIHAKKO	1704					// ��t���Ĕ��s����o�^
#define		OPLOG_EIKYUGYOKIRIKAE	1705					// �c�x�Ɛ؊�����o�^
#define		OPLOG_SHUTTERKAIHO		1706					// �V���b�^�[�J������o�^
#define		OPLOG_SERVICETIME		1707					// �T�[�r�X�^�C������o�^
#define		OPLOG_ROCKTIMER			1708					// ���b�N���u�^�C�}�[����o�^
#define		OPLOG_TEIKICHKSURU		1710					// ���엚��o�^(����^���o�̓`�F�b�N����)
#define		OPLOG_TEIKICHKSHINAI	1711					// ���엚��o�^(����^���o�̓`�F�b�N���Ȃ�)
#define		OPLOG_TEIKIYUKOMUKO		1712					// ���엚��o�^(����^����L���E����)
#define		OPLOG_TEIKIZENYUKO		1713					// ���엚��o�^(����^�S����L��)
#define		OPLOG_TEIKIZENMUKO		1714					// ���엚��o�^(����^�S�������)
#define		OPLOG_TEIKIINOUT		1715					// ���엚��o�^(����^���o��)
#define		OPLOG_TEIKIZENSHOKI		1716					// ���엚��o�^(����^�S����)
#define		OPLOG_TEIKIMUKOYOUBI	1717					// ��������j�����엚��o�^
#define		OPLOG_TOKUBETUBI		1718					// ���ʓ����엚��o�^
#define		OPLOG_TOKUBETUKIKAN		1719					// ���ʊ��ԑ��엚��o�^
#define		OPLOG_TOKUBETUHAPPY		1720					// �n�b�s�[�}���f�[���엚��o�^
#define		OPLOG_KAKARIINPASS		1721					// �W���p�X���[�h���엚��o�^
#define		OPLOG_BACKLIGHT			1722					// �o�b�N���C�g�_�����엚��o�^
#define		OPLOG_CONTRAST			1723					// �R���g���X�g�������엚��o�^
#define		OPLOG_KEYVOLUME			1724					// �L�[���ʒ������엚��o�^
#define		OPLOG_KENKIGEN			1725					// ���������엚��o�^
#define		OPLOG_EIGYOKAISHI		1726					// �c�ƊJ�n���엚��o�^
#define		OPLOG_TEIKICHUSI_DEL	1750					// ��������~�f�[�^�P���폜 ���엚��o�^
#define		OPLOG_TEIKICHUSI_ALLDEL	1751					// ��������~�f�[�^�S�폜 ���엚��o�^
#define		OPLOG_PEXTIM_DEL		1752					// ������o�Ɏ����f�[�^�P���폜 ���엚��o�^
#define		OPLOG_PEXTIM_ALLDEL		1753					// ������o�Ɏ����f�[�^�S�폜   ���엚��o�^
#define		OPLOG_TOKUBETUWEEKLY	1767					// ���ʗj���ݒ�(Y.Ise:�b��)
#define		OPLOG_LPRNSAIHAKKO		1768					// ���x���Ĕ��s���엚��o�^
#define		OPLOG_LPRPAPERSET		1769					// ���x���c�������엚��o�^
#define		OPLOG_LPRKOSINARI		1770					// ���x�����s�s���̍X�V���葀��o�^
#define		OPLOG_LPRKOSINNASI		1771					// ���x�����s�s���̍X�V�Ȃ�����o�^
#define		OPLOG_CRE_CAN_DEL		1772					// �N���W�b�g������E�f�[�^�폜
#define		OPLOG_CRE_CAN_PRN		1773					// ������E���
#define		OPLOG_CRE_CONECT		1774					// �ڑ��m�F
#define		OPLOG_CRE_USE_DEL		1775					// ���p���ׁE�폜

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		OPLOG_EDY_ARM_PRN		1776					// Edy�װю����������
//#define		OPLOG_EDY_ARM_CLR		1777					// Edy�װю�����ر
//#define		OPLOG_EDY_PRM_CHG		1778					// �֘A�ݒ�ύX
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

#define		OPLOG_VLSWTIME			1779					// ���ʐؑ֎�������o�^
#define		OPLOG_TOKUBETUYOBI		1780					// ���ʗj�����엚��o�^
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		OPLOG_EDY_SHIME_PRN		1781					// Edy���ߋL�^��������
//#define		OPLOG_EDY_SHIME_CLR		1782					// Edy���ߋL�^���ر
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		OPLOG_CARDINFO_UPDATE	1783					// �J�[�h���f�[�^�v��	
#define		OPLOG_PRG_DL			1790					// �v���O�����_�E�����[�h���{
#define		OPLOG_CPARAM_DL			1791					// ���ʃp�����[�^�_�E�����[�h���{
#define		OPLOG_MAINPRG_RESERVE	1792					// ���C���v���O�����^�p�ʐؑ֗\����{
#define		OPLOG_SUBPRG_RESERVE	1793					// �T�u�v���O�����^�p�ʐؑ֗\����{
#define		OPLOG_CPARAM_RESERVE	1794					// ���ʃp�����[�^�[�^�p�ʐؑ֗\����{
#define		OPLOG_CHARGE_PRINT		1795					// �����ݒ�v�����g
#define		OPLOG_SCASYU_CLR		1797					// Suica�����W�v����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		OPLOG_EDYSYU_CLR		1798					// Edy�����W�v����
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		OPLOG_HOJIN_USEDEL		1799

#define		OPLOG_SAIDAIRYOKIN		1801					// �ő嗿�����엚��o�^
#define		OPLOG_SERVICEYAKUWARI	1802					// �T�[�r�X���������엚��o�^
#define		OPLOG_MISEKUWARI		1803					// �X�������엚��o�^
#define		OPLOG_SHOHIZEI			1804					// ����ł̐ŗ����엚��o�^
#define		OPLOG_ALYAKUWARI		1805					// �`�`�k��������엚��o�^
#define		OPLOG_TANIJIKANRYO		1806					// �P�ʎ��ԗ������엚��o�^
#define		OPLOG_CAN_SET_PRI		1830					// �^�p�ݒ�v�����g���~���엚��o�^

#define		OPLOG_KYOTUPARAWT		2001					// �������Ұ�����엚��o�^
#define		OPLOG_SHASHITUPARAWT	2002					// �Ԏ����Ұ�����엚��o�^
#define		OPLOG_ROCKPARAWT		2003					// ۯ����Ұ�����엚��o�^
#define		OPLOG_KYOTUPARADF		2010					// �������Ұ�����엚��o�^
#define		OPLOG_SHASHITUPARADF	2011					// �Ԏ����Ұ�����엚��o�^
#define		OPLOG_ROCKPARADF		2012					// ۯ����Ұ�����엚��o�^
#define		OPLOG_T_HOUJIN_DEF		2020					// T�E�@�l�J�[�h�f�t�H���g�e�[�u���Z�b�g

#define		OPLOG_SWCHK				2301					// ��������(SW����)����o�^
#define		OPLOG_KEYCHK			2302					// ��������(������)����o�^
#define		OPLOG_LCDCHK			2303					// ��������(LCD����)����o�^
#define		OPLOG_LEDCHK			2304					// ��������(LED����)����o�^
#define		OPLOG_SHUTTERCHK		2305					// ��������(���������)����o�^
#define		OPLOG_SIGNALCHK			2306					// ��������(���o�͐M������)����o�^
#define		OPLOG_COINMECHCHK		2307					// ��������(���ү�����)����o�^
#define		OPLOG_NOTEREADERCHK		2308					// ��������(����ذ�ް����)����o�^
#define		OPLOG_PRINTERCHK		2309					// ��������(����������)����o�^
#define		OPLOG_ANNAUNCECHK		2310					// ��������(�ųݽ����)����o�^
#define		OPLOG_MEMORYCHK			2311					// ��������(�������)����o�^
#define		OPLOG_IFBOARDCHK		2312					// ��������(IF������)����o�^
#define		OPLOG_ARCNETCHK			2313					// ��������(ARCNET����)����o�^
#define		OPLOG_READERCHK			2314					// ��������(���Cذ�ް����)����o�^
#define		OPLOG_NYUKINCHK			2318					// ��������(��ݓ�������)����o�^
#define		OPLOG_HARAIDASHICHK		2319					// ��������(��ݏo������)����o�^
#define		OPLOG_NOTENYUCHK		2320					// ��������(������������)����o�^
#define		OPLOG_DOUSACOUNT		2323					// ��������(���춳��)����o�^
#define		OPLOG_DOUSACOUNTCLR		2324					// ��������(���춳��-�ر)����o�^
#define		OPLOG_LOCKDCNT			2325					// ��������(ۯ����u���춳��)	����o�^
#define		OPLOG_LOCKDCNTCLR		2347					// ��������(ۯ����u���춳��-�ر)����o�^
#define		OPLOG_FLAPDCNT			2348					// ��������(�ׯ�ߓ��춳��)		����o�^
#define		OPLOG_FLAPDCNTCLR		2349					// ��������(�ׯ�ߓ��춳��-�ر)	����o�^
#define		OPLOG_ALLBACKUP			2326					// �S�ް��ޯ����ߗ���o�^
#define		OPLOG_ALLRESTORE		2327					// �S�ް�ؽı����o�^
#define		OPLOG_SHASHITUDTSET		2328					// �Ԏ���񒲐�
#define		OPLOG_NTNETCHK			2338					// ��������(NTNET����)����o�^
#define		OPLOG_VERSIONCHK		2399					// ��������(�ް�ޮ�����)����o�^
#define		OPLOG_MIFARECHK			2339					// ��������(MIFARE����)����o�^
#define		OPLOG_CCOMCHK			2346					// ��������(�����ʐM����)����o�^
#define		OPLOG_SUICACHK			2344					// ��������(Suica����)����o�^ 
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		OPLOG_EDYCHK			2345					// ��������(Edy����)����o�^ 
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		OPLOG_PIPCHK			2342					// ��������(ParkiPro����)����o�^
#define		OPLOG_MAFCHK			2343					// ��������(MAF����)����o�^
#define		OPLOG_CAPPICHK			2350					// ��������(Cappi����)����o�^
#define		OPLOG_SCICHK			2360					// ��������(�ėp�ʐMײ�����)����o�^ 
#define		OPLOG_CCOMCHK_APN		2361					// ��������(�����ʐM���� APN�ύX)����o�^
#define		OPLOG_CCOMCHK_ANT		2362					// ��������(�����ʐM���� �d�g��M��Ԋm�F)����o�^
#define		OPLOG_CCRVERCHK			2370					// ��������(CRR�o�[�W�����`�F�b�N)����o�^
#define		OPLOG_CCRCOMCHK			2371					// ��������(CRR�܂�Ԃ��e�X�g)����o�^
#define		OPLOG_RISMCOMCHK		2372					// ��������(Rism�T�[�o�[�ڑ��m�F)����o�^
#define		OPLOG_CAR_FUNC_SELECT	2373					// ��������(�Ԏ��@�\�I��)����o�^
#define		OPLOG_FTP_PARAM_UPLD	2374					// ��������(FTP�ڑ��m�F �p�����[�^�A�b�v���[�h���s)����o�^
#define		OPLOG_FTP_CONNECT_CHK	2375					// ��������(FTP�ڑ��m�F FTP�ڑ��m�F���s)����o�^
#define		OPLOG_FLA_LOOP_DATA		2376					// �t���b�v���[�v�f�[�^�m�F
#define		OPLOG_FLA_SENSOR_CTRL	2377					// �t���b�v�ԗ����m�Z���T�[����
#define		OPLOG_FANOPE_CHK		2378					// FAN����`�F�b�N����o�^
#define		OPLOG_LAN_CONNECT_CHK	2379					// ��������(LAN�ڑ��m�F)����o�^
#define		OPLOG_CHK_PRINT			2380					// ��������(�H��m�F�v�����g)����o�^
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		OPLOG_CHK_EC			2383					// ��������(���σ��[�_�`�F�b�N)����o�^
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(S) Y.Yamauchi 20191015 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define		OPLOG_QRCHK				2440					// ��������(�p�q���[�_�[�`�F�b�N)����o�^
#define		OPLOG_REALCHK			2441					// ��������(���A���^�C���ʐM�`�F�b�N)����o�^
#define		OPLOG_DCLCHK			2442					// ��������(DC-NET�ʐM�`�F�b�N)����o�^
// MH810100(E) Y.Yamauchi 20191015 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define		OPLOG_WAVEDATA_RESERVE	2502					// �����f�[�^�^�p�ʐؑ֗\����{
#define		OPLOG_CARINFO_DL		2520					// �Ԏ����_�E�����[�h���{
#define		OPLOG_CCRFAIL_ON		2521					// �Ԏ��̏�(�Ȃ��˂���)����o�^
#define		OPLOG_CCRFAIL_OFF		2522					// �Ԏ��̏�(����˂Ȃ�)����o�^
#define		OPLOG_SHOUMEISETTEI		2523					// ���ԏؖ������s�ݒ�
#define		OPLOG_CRE_UNSEND_SEND	2570					// �����M����đ�
#define		OPLOG_CRE_UNSEND_DEL	2571					// �����M����폜
#define		OPLOG_CRE_SALE_NG_DEL	2572					// ���㋑�ۃf�[�^�폜
#define		OPLOG_CRE_CON_CHECK		2573					// �ڑ��m�F

/* �����[�g�����e�i���X����֘A */
#define		OPLOG_SET_TIME				8001				// ���v�Z�b�g
#define		OPLOG_SET_MANSYACTRL		8002				// ���ԃR���g���[���ύX
#define		OPLOG_SET_EIKYUGYO			8005				// �c�x�Ə�ԕύX
#define		OPLOG_SET_SHUTTER			8006				// �V���b�^�[��ԕύX		�����ݏ����Ȃ��i���񖢑Ή��j
#define		OPLOG_TELNET_LOGIN			8009				// TELNET���O�C��
#define		OPLOG_SET_NYUSYUTUCHK_ON	8010				// ������o�̓`�F�b�N�u����v
#define		OPLOG_SET_NYUSYUTUCHK_OFF	8011				// ������o�̓`�F�b�N�u���Ȃ��v
#define		OPLOG_SET_TEIKI_YUKOMUKO	8012				// ����L��/�����o�^
#define		OPLOG_SET_TEIKI_ALLYUKO		8013				// �S����L���o�^
#define		OPLOG_SET_TEIKI_ALLMUKO		8014				// �S��������o�^
#define		OPLOG_GT_SET_TEIKI_YUKOMUKO	8016				// ����L��/�����o�^(GT�t�H�[�}�b�g)
#define		OPLOG_GT_SET_STATUS_CHENGE	8017				// ������X�e�[�^�X�ύX(GT�t�H�[�}�b�g)
#define		OPLOG_SET_TEIKI_TYUSI		8028				// ������~�f�[�^
#define		OPLOG_SET_LOCK_OPEN			8040				// ���b�N���u�J
#define		OPLOG_SET_LOCK_CLOSE		8041				// ���b�N���u��
#define		OPLOG_SET_FLAP_DOWN			8042				// �t���b�v���~
#define		OPLOG_SET_FLAP_UP			8043				// �t���b�v�㏸
#define		OPLOG_SET_A_LOCK_OPEN		8044				// �S���b�N���u�J
#define		OPLOG_SET_A_LOCK_CLOSE		8045				// �S���b�N���u��
#define		OPLOG_SET_A_FLAP_DOWN		8046				// �S�t���b�v���~
#define		OPLOG_SET_A_FLAP_UP			8047				// �S�t���b�v�㏸
#define		OPLOG_ANSHOU_B_CLR			8048				// �Ïؔԍ�����
#define		OPLOG_SET_CTRL_RESET		8049				// ����f�[�^���Z�b�g�w��
#define		OPLOG_PARKI_GENGAKU			8051				// ���z���Z
#define		OPLOG_PARKI_FURIKAE			8052				// �U�֐��Z
#define		OPLOG_PARKI_UKETUKEHAKKO	8053				// ��t�����s
#define		OPLOG_PARKI_RYOSHUSAIHAKKO	8054				// �̎����Ĕ��s
// MH322914(S) K.Onodera 2016/09/16 AI-V�Ή��F���u���Z
#define		OPLOG_PARKI_ENKAKU			8055				// ���u���Z
// MH322914(E) K.Onodera 2016/09/16 AI-V�Ή��F���u���Z
#define		OPLOG_SET_STATUS_CHENGE		8015				// ������X�e�[�^�X�ύX
#define		OPLOG_SET_STATUS_TBL_UPDATE	8030				// ������X�e�[�^�X�e�[�u���X�V
#define		OPLOG_SET_TOKUBETU_UPDATE	8031				// ���ʓ��ݒ�f�[�^�X�V
#define		OPLOG_SET_TIME2_COM			8033				// NT-NET/APS-NET�ɂ�鎞�v�Z�b�g
#define		OPLOG_SET_TIME2_PIP			8034				// ParkiPRO
#define		OPLOG_SET_TIME2_SNTP		8035				// SNTP �������v�X�V�@�\�ɂ�鎞�v�Z�b�g
#define		OPLOG_SET_SNTP_SYNCHROTIME	8036				// SNTP ���������ύX(���u)
#define		OPLOG_REMOTE_DL_REQ			8084				// ���u�_�E�����[�h�v������
#define		OPLOG_REMOTE_DISCONNECT		8085				// Rism�ʐM�ؒf
#define		OPLOG_REMOTE_DL_END			8086				// ���u�_�E�����[�h�I��
#define		OPLOG_REMOTE_SW_END			8087				// �v���O�����X�V�I��
#define		OPLOG_REMOTE_RES_UPLD		8088				// ���u�_�E�����[�h�E�v���O�����X�V���ʃA�b�v���[�h�I��
#define		OPLOG_REMOTE_CONNECT		8089				// Rism�ʐM����
#define		OPLOG_SET_ATE_INV			8090				// �W�������f�[�^
#define		OPLOG_SET_PASSWORD			8091				// �W�������f�[�^
#define		OPLOG_SET_W_BINCODE			8092				// W�J�[�hBIN�R�[�h�ݒ�
#define		OPLOG_SET_H_BINCODE			8093				// �@�l�J�[�hBIN�R�[�h�ݒ�
#define		OPLOG_SET_TPOINT_BASE		8094				// �s�J�[�h��{���[�g�ݒ�
#define		OPLOG_SET_TPOINT_TIME		8095				// �s�J�[�h���ԃ��[�g�ݒ�
#define		OPLOG_SET_TCARD_USE			8096				// �s�J�[�h�g�p�ېݒ�
#define		OPLOG_SET_ATE_INV_MAX		8097				// �W�������f�[�^�o�^���

/* �����[�g�p�����[�^�ݒ�֘A */
#define		OPLOG_SET_KYOTUPARAM		8101				// ���ʃp�����[�^�ύX
#define		OPLOG_SET_SYASITUPARAM		8102				// �Ԏ��p�����[�^�ύX
#define		OPLOG_SET_LOCKPARAM			8103				// ���b�N��ʃp�����[�^�ύX

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//------------------
// ���[�����j�^�֘A��`
//------------------
// ���[�����j�^�i��Ԏ�ʁE�R�[�h�j
// �⍇���t�F�[�Y
#define LM_INQ_NO							2421	// �ԔԌ����ōݎԖ⍇����
#define LM_INQ_TIME							2422	// ���������ōݎԖ⍇����
#define LM_INQ_QR_TICKET					2423	// QR���Ԍ��ōݎԖ⍇����

// ���Z�t�F�[�Y
#define LM_PAY_NO_TIME						2721	// ���Z��(�Ԕ�/��������)
#define LM_PAY_QR_TICKET					2722	// ���Z��(QR���Ԍ�)
#define LM_PAY_CHANGE						2724	// ���Z��(���Z���ύX)
#define LM_PAY_MNY_IN						2725	// ���Z��(�����ς�(�����܂�))
#define LM_PAY_CMP							2726	// ���Z����
#define	LM_PAY_STOP							2727	// ���Z���~

// ���[�����j�^�i�����^�C�v�j
enum {
	SEARCH_TYPE_NO = 0,								// �ԔԌ���
	SEARCH_TYPE_TIME,								// ��������
	SEARCH_TYPE_QR_TICKET,							// QR����
};

// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

//------------------
// ���j�^�֘A��`
//------------------
enum {
	// ���u�ݒ� -->
		OPMON_RSETUP_REFOK		= 80,			// �ݒ�Q��OK
		OPMON_RSETUP_REFNG,						// ��NG
		OPMON_RSETUP_PREOK,						// �ݒ�ύX����OK
		OPMON_RSETUP_PRENG,						// ��NG
		OPMON_RSETUP_RCVOK,						// �ݒ�f�[�^��MOK
		OPMON_RSETUP_RCVNG,						// ��NG
		OPMON_RSETUP_EXEOK,						// �ݒ���sOK
		OPMON_RSETUP_EXENG,						// ��NG
		OPMON_RSETUP_CHGOK,						// �ݒ�ύX�I��OK
		OPMON_RSETUP_CHGNG,						// �ݒ�ύX�I��NG
		OPMON_RSETUP_CANOK,						// �ݒ�ύX�L�����Z��OK
		OPMON_RSETUP_CANNG,						// ��NG
		OPMON_RSETUP_NOREQ,						// �ݒ�ύX�v���Ȃ�
		_OPMON_RSETUP_MAX
};

//------------------
// ���j�^�֘A��`
//------------------
#define		OPMON_DOOROPEN				101					// �h�A�J
#define		OPMON_DOORCLOSE				102					// �h�A��
#define		OPMON_COINMECHSWOPEN		103					// �R�C�����b�NSW�J
#define		OPMON_COINMECHSWCLOSE		104					// �R�C�����b�NSW��
#define		OPMON_NOTERDSWOPEN			105					// ����ذ�ްSW�J
#define		OPMON_NOTERDSWCLOSE			106					// ����ذ�ްSW��
#define		OPMON_LOCK_OPEN				107					// ���b�N�J
#define		OPMON_LOCK_CLOSE			108					// ���b�N��
#define		OPMON_COINKINKO_UNSET		111					// �R�C�����ɖ��Z�b�g
#define		OPMON_COINKINKO_SET			112					// �R�C�����ɃZ�b�g
#define		OPMON_NOTEKINKO_UNSET		113					// �������ɊJ
#define		OPMON_NOTEKINKO_SET			114					// �������ɕ�
#define		OPMON_OPEN					121					// �c�ƒ�
#define		OPMON_CLOSW					122					// �x�ƒ�
#define		OPMON_CAR1_UNFULL			123					// ��Ԓ��P
#define		OPMON_CAR1_FULL				124					// ���Ԓ��P
#define		OPMON_CAR2_UNFULL			125					// ��Ԓ��Q
#define		OPMON_CAR2_FULL				126					// ���Ԓ��Q
#define		OPMON_CAR3_UNFULL			127					// ��Ԓ��R
#define		OPMON_CAR3_FULL				128					// ���Ԓ��R

#define		OPMON_CARDSTACK_OCC			145					// ���C���[�_�[�J�[�h�l�܂蔭��
#define		OPMON_CARDSTACK_REL			146					// ���C���[�_�[�J�[�h�l�܂����

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		OPMON_EDY_FSTCON_START		150					// ����ʐM�J�n
//#define		OPMON_EDY_FSTCON_STOP		151					// ����ʐM�I��
//#define		OPMON_EDY_MT_CEN_START		152					// �蓮�s���v�ɂ��Z���^�[�ʐM�J�n
//#define		OPMON_EDY_AT_CEN_START		153					// �����s���v�ɂ��Z���^�[�ʐM�J�n
//#define		OPMON_EDY_M_CEN_START		154					// �蓮�Z���^�[�ʐM�J�n�i�����e����j
//#define		OPMON_EDY_A_CEN_START		155					// �����Z���^�[�ʐM�J�n
//#define		OPMON_EDY_CEN_STOP			156					// �Z���^�[�ʐM�I��
//#define		OPMON_EDY_SNCKEY			157					// ���[�_�[���ݔF�،��X�V
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		OPMON_TIME_AUTO_REVISE		170					// ���������␳
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
#define		OPMON_SYUUKEI_IRGCLR1		171					// �W�v�N���A�ُ�(�@�󎚏W�v�̃w�b�_�[�ƑO��W�v�̃w�b�_�[�̔�r)
#define		OPMON_SYUUKEI_IRGCLR2		172					// �W�v�N���A�ُ�(�A���ݏW�v�̒ǔԁA����W�v�����A�O��W�v������ �󎚏W�v�̒ǔԁA����W�v�����A�O��W�v�����̔�r)
#define		OPMON_SYUUKEI_IRGCLR3		173					// �W�v�N���A�ُ�(�B���ݏW�v���ڂ̒l�ƈ󎚏W�v���ڂ̒l�̔�r)
#define		OPMON_SYUUKEI_IRGCLR4		174					// �W�v�N���A�ُ�(�C���ݏW�v�̍���W�v������0�N���A����Ă��邩�m�F)
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		OPMON_EC_MIRYO_START		175					// �d�q�}�l�[������������
#define		OPMON_EC_MIRYO_TIMEOUT		176					// �d�q�}�l�[���������^�C���A�E�g
// MH321800(E) G.So IC�N���W�b�g�Ή�
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
#define		OPMON_TURIKAN_IRGCLR1		177					// �ޑK�Ǘ��N���A�ُ�(�C���ݏW�v�̍���W�v������0�N���A����Ă��邩�m�F)
#define		OPMON_TURIKAN_IRGCLR2		178					// �ޑK�Ǘ��N���A�ُ�(�D���݂̒ǔԁA�W�����A�@�B����0�N���A����Ă���ꍇ�̓N���A�L�����Z��)
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
#define		OPMON_CRE_SEND_COMMAND		185					// �N���W�b�g�R�}���h���M
#define		OPMON_CRE_RECV_RESPONSE		186					// �N���W�b�g�R�}���h������M
#define		OPMON_CRE_COMMAND_TIMEOUT	187					// �N���W�b�g�R�}���h�^�C���A�E�g
#define		OPMON_SUICA_SETTLEMENT		250
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		OPMON_EC_SETTLEMENT			252					// �d�q�}�l�[�����茋�ʓd����M
#define		OPMON_EC_CRE_SETTLEMENT		253					// �N���W�b�g���ό��ʓd����M
// MH321800(E) G.So IC�N���W�b�g�Ή�
#define		OPMON_FLAPDOWN				510					// �ׯ�ߑ��u�J
#define		OPMON_FLAPUP				511					// �ׯ�ߑ��u��
#define		OPMON_LOCKDOWN				505					// ۯ����u�J
#define		OPMON_LOCKUP				506					// ۯ����u��
#define		OPMON_FLAP_MMODE_ON			515					// �ׯ�ߑ��u�蓮Ӱ�ޔ���
#define		OPMON_FLAP_MMODE_OFF		516					// �ׯ�ߑ��u�蓮Ӱ�މ���
#define		OPMON_LOCK_MMODE_ON			517					// ۯ����u�蓮Ӱ�ޔ���
#define		OPMON_LOCK_MMODE_OFF		518					// ۯ����u�蓮Ӱ�މ���
#define		OPMON_CAR_IN				520					// ����
#define		OPMON_CAR_OUT				521					// �o��
#define		OPMON_LAG_EXT				504					// ���O�^�C������

// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
#define		OPMON_TEST					9901				// �����p
#endif
// �����p (e)
// �[���@�\�ԍ�
#define		RMON_FUNC_REMOTEDL					 1			// ���u�����e�i���X�@�\

// �����ԍ��E�󋵔ԍ�
#define		RMON_PRG_DL_START_OK				 1000100		// �_�E�����[�h�J�n����
#define		RMON_PRG_DL_START_MNT_NG			 1000901		// �����e�i���X��NG
#define		RMON_PRG_DL_START_RETRY_OVER		 1000201		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
#define		RMON_PRG_DL_START_OVER_ELAPSED_TIME	 1000202		// ��莞�Ԍo��
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�

#define		RMON_PRG_DL_END_OK					 1010100		// �_�E�����[�h�I������
#define		RMON_PRG_DL_END_COMM_NG				 1010201		// �ʐM�ُ�
#define		RMON_PRG_DL_END_FILE_NG				 1010202		// �v���O�����t�@�C���G���[
#define		RMON_PRG_DL_END_FLASH_WRITE_NG		 1010203		// Flash�����ݎ��s
#define		RMON_PRG_DL_END_PROG_FILE_NONE		 1010204		// �v���O�����t�@�C���Ȃ�
#define		RMON_PRG_DL_END_SUM_NG				 1010205		// �v���O����SUM�l�G���[
#define		RMON_PRG_DL_END_RETRY_OVER			 1010206		// ���g���C�I�[�o�[

#define		RMON_PRG_SW_START_OK				 2000100		// �X�V�J�n����
#define		RMON_PRG_SW_START_STATUS_NG			 2000901		// ���NG
#define		RMON_PRG_SW_START_COMM_NG			 2000902		// �ʐM��
#define		RMON_PRG_SW_START_FLAP_NG			 2000903		// �t���b�v���쒆
#define		RMON_PRG_SW_START_DOOR_OPEN_NG		 2000904		// �h�A�J
#define		RMON_PRG_SW_START_RETRY_OVER		 2000201		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
#define		RMON_PRG_SW_START_OVER_ELAPSED_TIME	 2000202		// ��莞�Ԍo��
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�

#define		RMON_PRG_SW_END_OK					 2010100		// �X�V����
#define		RMON_PRG_SW_END_BACKUP_NG			 2010201		// �S�o�b�N�A�b�v���s
#define		RMON_PRG_SW_END_PROG_FILE_NONE		 2010202		// �ҋ@�ʂɃv���O�����t�@�C���Ȃ�
#define		RMON_PRG_SW_END_FLASH_WRITE_NG		 2010203		// �v���O���������ݎ��s
#define		RMON_PRG_SW_END_RESTORE_NG			 2010204		// ���X�g�A���s
#define		RMON_PRG_SW_END_RESET_NG			 2010205		// �X�V���Ƀ��Z�b�g
#define		RMON_PRG_SW_END_UNSENT_DATA_NG		 2010206		// �����M�f�[�^�̑��M���s

#define		RMON_FTP_LOGIN_OK					 3000100		// ���O�C������
#define		RMON_FTP_LOGIN_ID_PW_NG				 3000201		// ID or PW�s��
#define		RMON_FTP_LOGIN_TIMEOUT_NG			 3000202		// �ڑ��^�C���A�E�g
#define		RMON_FTP_FILE_TRANS_NG				 3000203		// �t�@�C���]�����s
#define		RMON_FTP_FILE_DEL_NG				 3000204		// �폜���s
#define		RMON_FTP_LOGIN_RETRY_OVER			 3000205		// ���g���C�I�[�o�[

#define		RMON_FTP_LOGOUT_OK					 4010100		// ���O�A�E�g����
#define		RMON_FTP_LOGOUT_NG					 4010200		// ���O�A�E�g���s

#define		RMON_PRM_DL_START_OK				 5000100		// �_�E�����[�h�J�n����
#define		RMON_PRM_DL_START_MNT_NG			 5000901		// �����e�i���X��NG
#define		RMON_PRM_DL_START_RETRY_OVER		 5000201		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
#define		RMON_PRM_DL_START_OVER_ELAPSED_TIME	 5000202		// ��莞�Ԍo��
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_DL_START_CANCEL			 5000301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

#define		RMON_PRM_DL_END_OK					 5010100		// �_�E�����[�h�I������
#define		RMON_PRM_DL_END_COMM_NG				 5010201		// �ʐM�ُ�
#define		RMON_PRM_DL_END_FILE_NG				 5010202		// �ݒ�t�@�C���G���[
#define		RMON_PRM_DL_END_FLASH_WRITE_NG		 5010203		// Flash�����ݎ��s
#define		RMON_PRM_DL_END_PARAM_FILE_NONE		 5010204		// �ݒ�t�@�C���Ȃ�
#define		RMON_PRM_DL_END_RETRY_OVER			 5010205		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_DL_END_CANCEL			     5010301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

#define		RMON_PRM_SW_START_OK				 6000100		// �X�V�J�n����
#define		RMON_PRM_SW_START_STATUS_NG			 6000901		// ���NG
#define		RMON_PRM_SW_START_COMM_NG			 6000902		// �ʐM��
#define		RMON_PRM_SW_START_FLAP_NG			 6000903		// �t���b�v���쒆
#define		RMON_PRM_SW_START_DOOR_OPEN_NG		 6000904		// �h�A�J
#define		RMON_PRM_SW_START_RETRY_OVER		 6000201		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
#define		RMON_PRM_SW_START_OVER_ELAPSED_TIME	 6000202		// ��莞�Ԍo��
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_SW_START_CANCEL			 6000301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

#define		RMON_PRM_SW_END_OK					 6010100		// �X�V����
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_SW_END_OK_MNT				 6010101		// �X�V����(�[���Őݒ�p�����[�^�ύX)
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_SW_END_PARAM_FILE_NONE		 6010201		// �ҋ@�ʂɐݒ�t�@�C���Ȃ�
#define		RMON_PRM_SW_END_FLASH_WRITE_NG		 6010202		// �ݒ菑���ݎ��s
#define		RMON_PRM_SW_END_RESET_NG			 6010203		// �X�V���Ƀ��Z�b�g
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_SW_END_CANCEL				 6010301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

#define		RMON_PRM_UP_START_OK				 7000100		// �ݒ�v���J�n����
#define		RMON_PRM_UP_START_MNT_NG			 7000901		// �����e�i���X��NG
#define		RMON_PRM_UP_START_RETRY_OVER		 7000201		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
#define		RMON_PRM_UP_START_OVER_ELAPSED_TIME	 7000202		// ��莞�Ԍo��
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_UP_START_CANCEL			 7000301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

#define		RMON_PRM_UP_END_OK					 7010100		// �ݒ�v���I������
#define		RMON_PRM_UP_END_COMM_NG				 7010201		// �ʐM�ُ�
#define		RMON_PRM_UP_END_UNCOMP_NG			 7010202		// ���k���s
#define		RMON_PRM_UP_END_RETRY_OVER			 7010203		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_UP_END_CANCEL				 7010301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

#define		RMON_RESET_START_1MIN_BEFORE		 8000101		// 1���O�ʒm
#define		RMON_RESET_START_OK					 8000100		// ���Z�b�g�J�n����
#define		RMON_RESET_START_STATUS_NG			 8000901		// ���NG
#define		RMON_RESET_START_COMM_NG			 8000902		// �ʐM��
#define		RMON_RESET_START_FLAP_NG			 8000903		// �t���b�v���쒆
#define		RMON_RESET_START_DOOR_OPEN_NG		 8000904		// �h�A�J
#define		RMON_RESET_START_RETRY_OVER			 8000201		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
#define		RMON_RESET_START_OVER_ELAPSED_TIME	 8000202		// ��莞�Ԍo��
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�

#define		RMON_RESET_END_OK					 8010100		// ���Z�b�g�I������
	
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
#define		RMON_PRM_DL_DIF_START_OK			 9000100		// �_�E�����[�h�J�n����
#define		RMON_PRM_DL_DIF_START_MNT_NG		 9000901		// �����e�i���X��NG
#define		RMON_PRM_DL_DIF_START_RETRY_OVER	 9000201		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
#define		RMON_PRM_DL_DIF_START_OVER_ELAPSED_TIME	 9000202		// ��莞�Ԍo��
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_DL_DIF_START_CANCEL		 9000301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

#define		RMON_PRM_DL_DIF_END_OK				 9010100		// �_�E�����[�h�I������
#define		RMON_PRM_DL_DIF_END_COMM_NG			 9010201		// �ʐM�ُ�
#define		RMON_PRM_DL_DIF_END_FILE_NG			 9010202		// �ݒ�t�@�C���G���[
#define		RMON_PRM_DL_DIF_END_FLASH_WRITE_NG	 9010203		// Flash�����ݎ��s
#define		RMON_PRM_DL_DIF_END_PARAM_FILE_NONE	 9010204		// �ݒ�t�@�C���Ȃ�
#define		RMON_PRM_DL_DIF_END_RETRY_OVER		 9010205		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
#define		RMON_PRM_DL_DIF_END_CANCEL			 9010301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

#define		RMON_PRM_SW_DIF_START_OK			10000100		// �X�V�J�n����
#define		RMON_PRM_SW_DIF_START_STATUS_NG		10000901		// ���NG
#define		RMON_PRM_SW_DIF_START_COMM_NG		10000902		// �ʐM��
#define		RMON_PRM_SW_DIF_START_FLAP_NG		10000903		// �t���b�v���쒆
#define		RMON_PRM_SW_DIF_START_DOOR_OPEN_NG	10000904		// �h�A�J
#define		RMON_PRM_SW_DIF_START_RETRY_OVER	10000201		// ���g���C�I�[�o�[
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
#define		RMON_PRM_SW_DIF_START_OVER_ELAPSED_TIME	 10000202		// ��莞�Ԍo��
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// GG120600(S) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���
#define		RMON_PRM_SW_DIF_START_CANCEL		10000301		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX�j
// GG120600(E) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���

#define		RMON_PRM_SW_DIF_END_OK				10010100		// �X�V����
#define		RMON_PRM_SW_DIF_END_PARAM_FILE_NONE	10010201		// �ҋ@�ʂɐݒ�t�@�C���Ȃ�
#define		RMON_PRM_SW_DIF_END_FLASH_WRITE_NG	10010202		// �ݒ菑���ݎ��s
#define		RMON_PRM_SW_DIF_END_RESET_NG		10010203		// �X�V���Ƀ��Z�b�g
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)

#define		RMON_PRG_DL_REQ_OK					20000100		// �\��o�^����
#define		RMON_PRG_DL_REQ_NG					20000200		// �\��o�^���s
#define		RMON_PRG_DL_REQ_PRGNO_NG			20000201		// �v���O�������Ԉُ�
#define		RMON_PRG_DL_CANCEL_OK				20010100		// ���~����
#define		RMON_PRG_DL_CANCEL_NG				20010200		// ���~���s
#define		RMON_PRG_DL_CHECK_OK				20020100		// �\��m�F����
#define		RMON_PRG_DL_CHECK_NG				20020200		// �\��m�F���s
#define		RMON_PRG_DL_REQ_RT_OK				20030100		// �������s�o�^����
#define		RMON_PRG_DL_REQ_RT_NG				20030200		// �������s�o�^���s
#define		RMON_PRG_DL_REQ_RT_PRGNO_NG			20030201		// �v���O�������Ԉُ�

#define		RMON_PRM_DL_REQ_OK					20040100		// �\��o�^����
#define		RMON_PRM_DL_REQ_NG					20040200		// �\��o�^���s
#define		RMON_PRM_DL_CANCEL_OK				20050100		// ���~����
#define		RMON_PRM_DL_CANCEL_NG				20050200		// ���~���s
#define		RMON_PRM_DL_CHECK_OK				20060100		// �\��m�F����
#define		RMON_PRM_DL_CHECK_NG				20060200		// �\��m�F���s
#define		RMON_PRM_DL_REQ_RT_OK				20070100		// �������s�o�^����
#define		RMON_PRM_DL_REQ_RT_NG				20070200		// �������s�o�^���s

#define		RMON_PRM_UP_REQ_OK					20080100		// �\��o�^����
#define		RMON_PRM_UP_REQ_NG					20080200		// �\��o�^���s
#define		RMON_PRM_UP_CANCEL_OK				20090100		// ���~����
#define		RMON_PRM_UP_CANCEL_NG				20090200		// ���~���s
#define		RMON_PRM_UP_CHECK_OK				20100100		// �\��m�F����
#define		RMON_PRM_UP_CHECK_NG				20100200		// �\��m�F���s
#define		RMON_PRM_UP_REQ_RT_OK				20110100		// �������s�o�^����
#define		RMON_PRM_UP_REQ_RT_NG				20110200		// �������s�o�^���s

#define		RMON_RESET_REQ_OK					20120100		// �\��o�^����
#define		RMON_RESET_REQ_NG					20120200		// �\��o�^���s
#define		RMON_RESET_CANCEL_OK				20130100		// ���~����
#define		RMON_RESET_CANCEL_NG				20130200		// ���~���s
#define		RMON_RESET_CHECK_OK					20140100		// �\��m�F����
#define		RMON_RESET_CHECK_NG					20140200		// �\��m�F���s
#define		RMON_RESET_REQ_RT_OK				20150100		// �������s�o�^����
#define		RMON_RESET_REQ_RT_NG				20150200		// �������s�o�^���s

#define		RMON_PRG_SW_REQ_OK					20160100		// �\��o�^����
#define		RMON_PRG_SW_REQ_NG					20160200		// �\��o�^���s
#define		RMON_PRG_SW_REQ_PRG_NONE_NG			20160201		// �v���O�����f�[�^�Ȃ�
#define		RMON_PRG_SW_REQ_PRGNO_NG			20160202		// ���ԕs��v
#define		RMON_PRG_SW_CANCEL_OK				20170100		// ���~����
#define		RMON_PRG_SW_CANCEL_NG				20170200		// ���~���s
#define		RMON_PRG_SW_CHECK_OK				20180100		// �\��m�F����
#define		RMON_PRG_SW_CHECK_NG				20180200		// �\��m�F���s
#define		RMON_PRG_SW_REQ_RT_OK				20190100		// �������s�o�^����
#define		RMON_PRG_SW_REQ_RT_NG				20190200		// �������s�o�^���s
#define		RMON_PRG_SW_REQ_RT_PRG_NONE_NG		20190201		// �v���O�����f�[�^�Ȃ�
#define		RMON_PRG_SW_REQ_RT_PRGNO_NG			20190202		// ���ԕs��v

#define		RMON_FTP_CHG_OK						20200100		// ��t����
#define		RMON_FTP_CHG_NG						20200200		// ��t�s��

#define		RMON_FTP_TEST_RES_OK				20210100		// �e�X�g����
#define		RMON_FTP_TEST_RES_NG				20210200		// �e�X�g���s
#define		RMON_FTP_TEST_NG					20210201		// ��t�s��

#define		RMON_ILLEGAL_REQ_KIND				20220200		// �s���v�����
#define		RMON_ILLEGAL_PROC_KIND				20230200		// �s���������

// MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
#define		RMON_PRM_DIF_DL_REQ_OK				20240100		// �\��o�^����
#define		RMON_PRM_DIF_DL_REQ_NG				20240200		// �\��o�^���s
#define		RMON_PRM_DIF_DL_CANCEL_OK			20250100		// ���~����
#define		RMON_PRM_DIF_DL_CANCEL_NG			20250200		// ���~���s
#define		RMON_PRM_DIF_DL_CHECK_OK			20260100		// �\��m�F����
#define		RMON_PRM_DIF_DL_CHECK_NG			20260200		// �\��m�F���s
#define		RMON_PRM_DIF_DL_REQ_RT_OK			20270100		// �������s�o�^����
#define		RMON_PRM_DIF_DL_REQ_RT_NG			20270200		// �������s�o�^���s
// MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�

//-------------------------------
// �o�b�N�A�b�v�^���X�g�A�֘A��`
//-------------------------------

/*** �f�[�^��ʒ�` ***/
enum {
	OPE_DTNUM_COMPARA = 0,									// ���ʃp�����[�^
	OPE_DTNUM_PERPARA,										// �ʃp�����[�^
	OPE_DTNUM_LOCKINFO,										// �Ԏ��ݒ�
	OPE_DTNUM_LOCKMAKER,									// ���b�N���u�ݒ�
	OPE_DTNUM_PCADPT,										// ����e�[�u��
	OPE_DTNUM_FLAPDT,										// ���Ɂi���Ԉʒu�j���
	OPE_DTNUM_MOVCNT,										// ����J�E���g
	OPE_DTNUM_NTNET,										// NT-NET�f�[�^
	OPE_DTNUM_COINSYU,										// �R�C�����ɏW�v
	OPE_DTNUM_NOTESYU,										// �������ɏW�v
	OPE_DTNUM_TURIKAN,										// ���K�Ǘ�
	OPE_DTNUM_SKY,											// �W�v�iT,GT,������j
	OPE_DTNUM_SKYBK,										// �O��W�v
	OPE_DTNUM_LOKTL,										// ���Ԉʒu�ʏW�v
	OPE_DTNUM_LOKTLBK,										// �O�񒓎Ԉʒu�ʏW�v
	OPE_DTNUM_CARCOUNT,										// ���o�ɑ䐔
	OPE_DTNUM_PRCADPT,										// ����e�[�u��
	OPE_DTNUM_COIN_BK,										// �R�C�����ɍ��v�i�O�񕪁j
	OPE_DTNUM_NOTE_BK,										// �������ɍ��v�i�O�񕪁j
	OPE_DTNUM_CRE_CAN,										// �N���W�b�g�����񃍃O
	OPE_DTNUM_CRE_URI,										// �N���W�b�g������W�`�F�b�N�G���A
	OPE_DTNUM_MIF_NG,										// Mifare�����ݎ��s�����ް�	(Gate)
	OPE_DTNUM_SUICA_ERR_LOG,								// Suica�ُ�ʐM���O
	OPE_DTNUM_LOG_DCSYUUKEI,								// Edy,Suica�W�v���
	OPE_DTNUM_ATEINV_LOG,									// �����W���ް�
	OPE_DTNUM_IO_LOG,										// ���o�Ƀ��O
	OPE_DTNUM_BKLOCK_INFO,									// �Ԏ��ݒ�o�b�N�A�b�v�G���A�i�Ԏ��̏�p�j
	OPE_DTNUM_SETPRM_LOG,									// �ݒ�X�V����
	OPE_DTNUM_LONG_TERM_PARKING,							// �������ԃf�[�^�O�񔭕񎞊�
	OPE_DTNUM_CENTER_SEQNO,									// �Z���^�[�ǔ�
	OPE_DTNUM_CREDIT_SLIPNO,								// �N���W�b�g�[�������ʔ�
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	OPE_DTNUM_DATE_SYOUKEI,									//���t�ؑ֊�̏��v(���A���^�C���f�[�^�p)
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// MH321800(S) G.So IC�N���W�b�g�Ή�
	OPE_DTNUM_EDY_TERMINALNO,								// Edy��ʒ[��ID
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
//	OPE_DTNUM_EC_ALARM_TRADE_LOG,							// ���σ��[�_ �A���[��������O
	OPE_DTNUM_DUMMY,										// ���Ɉړ����邽�߃_�~�[
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
	OPE_DTNUM_DC_NET_SEQNO,									// DC-NET�ʐM�p�Z���^�[�ǔ�
	OPE_DTNUM_REAL_SEQNO,									// ���A���^�C���ʐM�p�Z���^�[�ǔ�
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
	OPE_DTNUM_EC_ALARM_TRADE_LOG,							// ���σ��[�_ �A���[��������O
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
	OPE_DTNUM_EC_BRAND_TBL,									// ���σ��[�_�����M�����u�����h�e�[�u��
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
	OPE_DTNUM_LOG_DCSYUUKEI_EC,								// SX,���σ��[�_�W�v���
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
	OPE_DTNUM_MAX
};
extern	ulong	Ope_f_Sem_DataAccess;						// �f�[�^�A�N�Z�X�Z�}�t�H�[�t���O
															// bit���ɈӖ������� 1=�g�p���C0=���g�p
															// bit�ƃf�[�^��̊֌W�͏�L�V���{���̒ʂ�
															// ��jb0=���ʃp�����[�^

//------------------
// ��݊֘A��`
//------------------
#define		COIN_FULL			CPrmSS[S_MON][27]			// ��݋��ɖ��t�߲��
#define		COIN_NEAR_FULL		CPrmSS[S_MON][26]			// ��݋��ɖ��t�\���߲��
#define		NOTE_NEAR_FULL		CPrmSS[S_MON][28]			// �������ɖ��t�\���߲��

#define		POINT_10			CPrmSS[S_MON][20]			// 10�~���߲�Đ�
#define		POINT_50			CPrmSS[S_MON][21]			// 50�~���߲�Đ�
#define		POINT_100			CPrmSS[S_MON][22]			// 100�~���߲�Đ�
#define		POINT_500   		CPrmSS[S_MON][23]			// 500�~���߲�Đ�

/* ��ʕ\���p�@���Z�}�̎�� */
#define		OPE_DISP_MEDIA_TYPE_SUICA		1
#define		OPE_DISP_MEDIA_TYPE_PASMO		2
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		OPE_DISP_MEDIA_TYPE_EDY			3
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		OPE_DISP_MEDIA_TYPE_ICOCA		4
#define		OPE_DISP_MEDIA_TYPE_eMONEY		5
#define		OPE_DISP_MEDIA_TYPE_ICCARD		10
// MH321800(S) hosoda IC�N���W�b�g�Ή�
#define		OPE_DISP_MEDIA_TYPE_EC			11
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		EDY_USE_TIMER					6 // 500ms��ϰ�p(3�b)
//
//extern  const unsigned char EdyTestModeStr[][19];
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

//---------------------------------------
//	Macro
//---------------------------------------
/* ���u��ʁE�^�p�`�ԂȂ� */
#define		MIFARE_CARD_DoesUse			( ((uchar)prm_get(COM_PRM, S_PAY, 25,1, 1)) == 1 && !card_use[USE_PAS] )		// 1=Mifare use & ������ł̐��Z���s���Ă��Ȃ��ꍇ

#define		Is_SUICA_STYLE_OLD			((uchar)( prm_get(COM_PRM, S_SCA, 1, 1, 6) == 0 ))	// ������w�蔻��iMH706305�ȑO�̓���j

#define		SUICA_MIRYO_AFTER_STOP		((uchar)( prm_get(COM_PRM, S_SCA, 5, 1, 6) >= 1 ))	// Suica��������������͎g�p�s�Ƃ���ݒ�
#define		SUICA_ONLY_STOP				((uchar)( prm_get(COM_PRM, S_SCA, 5, 1, 6) == 1 ))	// Suica���������������Suica�̂ݎg�p�s�Ƃ���ݒ�
#define		SUICA_CM_BV_RD_STOP			((uchar)( prm_get(COM_PRM, S_SCA, 5, 1, 6) == 2 ))	// Suica���������������Suica/CM/BV/RD���g�p�s�Ƃ���ݒ�
#define		OPE_SIG_DOOR_Is_OPEN			(1 == doorDat)							// 1= �O�ʔ��J
#define		OPE_SIG_DOOR_and_NOBU_are_CLOSE	(!OPE_SIG_DOOR_Is_OPEN && !OPE_SIG_DOORNOBU_Is_OPEN)
#define		OPE_SIG_DOORNOBU_Is_OPEN		doornobuDat								// 1= �O�ʔ��h�A�m�u�J
#define		OPE_SIG_OUT_DOORNOBU_LOCK(a)	(ExIOSignalwt( EXPORT_LOCK, (uchar)a ))	// �ޱ���ۯ���ɲ�ސ���ia:1=Lock, 0=UnLock�j
#define		OPE_DOOR_KNOB_ALARM_START_TIMER		(ushort)(50 * prm_get(COM_PRM, S_PAY, 39, 3, 1))
#define		OPE_DOOR_KNOB_ALARM_STOP_TIMER		(ushort)(50 * prm_get(COM_PRM, S_PAY, 39, 3, 4))
#define		OPE_IS_EBABLE_MAG_LOCK		((uchar)((CPrmSS[S_PAY][17]) != 0))			// 1=���b�N���� 0=���b�N���Ȃ�
#define		DATE_CHK_ERR(y,m,d)	(( !y || !m || !d || chkdate( y, m, d ) != 0 ) ? 1 : 0)	// 1=Error
#define		OPE_Is_DOOR_ERRARM			((uchar)( prm_get(COM_PRM, S_CEN, 96, 1, 6) == 0 ))
// MH810100(S) Y.Watanabe 2020/02/13 �Ԕԃ`�P�b�g���X
#define		QR_READER_USE				((uchar)( prm_get(COM_PRM, S_PAY, 25, 1, 5) == 1))
// MH810100(E) Y.Watanabe 2020/02/13 �Ԕԃ`�P�b�g���X
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//// MH810105(S) MH364301 �C���{�C�X�Ή� ��Ή�
////// MH810105(S) MH364301 �C���{�C�X�Ή�
////#define		IS_INVOICE					((uchar)( prm_get(COM_PRM, S_SYS, 16, 1, 6) == 0 ))
////// MH810105(E) MH364301 �C���{�C�X�Ή�
//#define		IS_INVOICE					((uchar)( 0 ))
//// MH810105(E) MH364301 �C���{�C�X�Ή� ��Ή�
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//#define		IS_INVOICE					((uchar)( prm_get(COM_PRM, S_SYS, 16, 1, 6) == 0 ))
#define		IS_INVOICE					(0)
// 01-0016�@���Q�Ƃ���ꍇ�͉��L�̃}�N�����g�p���邱��
#define		IS_INVOICE_PRM				((uchar)( prm_get(COM_PRM, S_SYS, 16, 1, 6) == 0 ))
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
#define		IS_ERECEIPT					(prm_get(COM_PRM, S_RTP, 58, 1, 1) == 1)
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�

//---------------------------------------
//	���̑�
//---------------------------------------
// MH810105(S) MH364301 �C���{�C�X�Ή�
#define		PRIEND_PREQ_RYOUSYUU		(PREQ_RYOUSYUU | INNJI_ENDMASK)				// �̎������s���b�Z�[�W
#define		JNL_PRI_WAIT_TIME			(3*2+1)										// �W���[�i���󎚊����҂��^�C�}�i3�b�j�i500ms�P�ʁj
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
#define		EJNL_PRI_WAIT_TIME			2											// �d�q�W���[�i���󎚊����҂��^�C�}�i1�b�j�i500ms�P�ʁj
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j

//---------------------------------
// �f�[�^SUM(���󋤒ʃp�����[�^�ƎԎ��p�����[�^�̂ݎg�p)
//---------------------------------

typedef struct {
	ulong	Sum;
	ulong	Len;
}t_DataSum;
extern	t_DataSum	DataSum[OPE_DTNUM_MAX];

#define		FORCE_FULL		0x11	// ��������(�{��)
#define		FORCE_VACANCY	0x21	// �������(�{��)
#define		NTNET_FULL		0x12	// ��������(�ʐM)
#define		NTNET_VACANCY	0x22	// �������(�ʐM)
#define		AUTO_FULL		0x14	// ����(����)
#define		AUTO_VACANCY	0x24	// ���(����)
// MH322917(S) A.Iiizumi 2018/11/27 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
#define		DATE_URIAGE_NO_CHANGE		0
#define		DATE_URIAGE_CLR_TODAY		1
#define		DATE_URIAGE_CLR_BEFORE		2
#define		DATE_URIAGE_PRMCNG_TODAY	3
#define		DATE_URIAGE_PRMCNG_BEFORE	4

#define		DATE_URIAGE_PRMCNG_NOCLR	0
#define		DATE_URIAGE_PRMCNG_CLR		1
// MH322917(E) A.Iiizumi 2018/11/27 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
/*** function prototype ***/

/* opetask.c */
extern  void			opetask( void );
extern	void			mojule_wait( void );
extern	char			memclr( char );
extern	unsigned short	GetMessage( void );
extern	void			Ope_ErrAlmClear( void );
extern	void			auto_syuukei( void );
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	uchar			auto_centercomm( uchar execkind );
//extern	uchar			autocnt_execchk( uchar type );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

extern	void			Ope_MsgBoxClear( void );
extern	void			Ope_InSigSts_Initial( void );

extern	void	ck_syuukei_ptr_zai( ushort LogKind );
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// �A�C�h���T�u�^�X�N�Ɉړ�
//-// MH322916(S) A.Iiizumi 2018/07/26 �������Ԍ��o�@�\�Ή�
//extern	void	LongTermParkingCheck_Resend( void );
//-// MH322916(E) A.Iiizumi 2018/07/26 �������Ԍ��o�@�\�Ή�
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
extern	void			EJA_TimeAdjust(void);
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

#define	_AteGetDataTop()		(int)_offset(Attend_Invalid_table.wtp, _countof(Attend_Invalid_table.aino) - Attend_Invalid_table.count, _countof(Attend_Invalid_table.aino))
#define	_AteGetDataPos(ofs)		_offset(_AteGetDataTop(), ofs, _countof(Attend_Invalid_table.aino))
#define	_AteIdx2Ofs(idx)		_offset(Attend_Invalid_table.wtp, ADN_VAILD_MAX-Attend_Invalid_table.count+idx, ADN_VAILD_MAX)
#define	dispPos(top,i)			(((top+_AteGetDataPos(i)) > (ADN_VAILD_MAX-1)) ? ((top+_AteGetDataPos(i))-ADN_VAILD_MAX):(top+_AteGetDataPos(i)))
extern ushort	DoorLockTimer;
extern ulong	attend_no;							// Repark���ނ���ǂݎ�����W��No
extern uchar	Repark_card_use;					// �P�F�g�p�������ނ�Repark���� �O�F�ʂ̶���

/* logctrl.c */
extern	void			Log_regist( short );
extern	void			log_init( void );
extern	void			Log_Write(short Lno, void *dat, BOOL strage);
extern	void Log_Write_Pon(void);
extern	void Log_clear_log_bakupflag(void);
extern	void			Log_CheckBufferFull(BOOL occur, short Lno, short target);
extern	ushort			Ope_Log_TotalCountGet(short id);
extern	ushort			Ope_Log_UnreadCountGet(short id, short target);
extern	ushort			Ope_Log_UnreadToRead(short id, short target);
extern	ushort			Ope_Log_CheckNearFull(short id, short target);
extern	ushort			Ope_Log_GetNearFull( short id, short target );
extern	ushort			Ope_Log_GetNearFullCount( short id );
extern	uchar			Ope_Log_1DataGet(short id, ushort Index, void *pSetBuf);
extern	uchar			Ope_Log_TargetDataGet(short id, void *pSetBuf, short target, BOOL update);
extern	uchar			Ope_Log_TargetDataVoidRead(short id, void *pSetBuf, short target, BOOL start);
extern	void 			Ope_Log_UndoTargetDataVoidRead( short id, short target );
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// extern	uchar			Ope_Log_ShashitsuLogGet( SYUKEI *syukei, LCKTTL_LOG *lckttl );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
extern	void			Ope_Log_TargetReadPointerUpdate(short id, short target);
extern	ushort			Ope_Log_TargetVoidReadPointerUpdate(short id, short target);
extern	uchar			Log_SemGet( uchar DtNum );
extern	void			Log_SemFre( uchar DtNum );
extern	void			NgLog_write( ushort card_type, uchar *cardno, ushort cardno_len );
extern	void			IoLog_write( ushort event_type, ushort sub_type, ulong ticketnum, ulong flg );

/* Ledctrl.c */
extern	void			LedInit( void );
extern	void    		LedReq( unsigned char , unsigned char );
extern	unsigned char	IsLedReq( unsigned char );
extern	void			LedOnOffCtrl( void );

/* opemain.c */
extern	void			OpeMain( void );
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// �V�K �N���������̒ǉ�
extern	short			op_init00( void );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
extern	short			op_mod00( void );
extern	short			op_mod01( void );
extern	short			op_mod02( void );
extern	short			op_mod03( void );
extern	short			op_mod11( void );
extern	short			op_mod12( void );
extern	short			op_mod13( void );
extern	short			op_mod14( void );
extern	short			op_mod21( void );
extern	short			op_mod22( void );
extern	short			op_mod23( void );
extern	short			carchk( uchar, ushort, uchar );
extern	short			carchk_ml( uchar, ushort );
extern	short			invcrd( ushort );
extern	short			in_mony( ushort, ushort );
extern	long			modoshi( void );
extern	short			set_tim( unsigned short, struct clk_rec *, short );
extern	short			set_tim2( struct clk_rec * );
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
extern	short			set_tim_Lost( ushort trig,ushort num, struct clk_rec *clk_para, short err_mode );
extern	short			set_tim_only_out( ushort num, struct clk_rec *clk_para, short err_mode );
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
extern	void			cl_7seg( char );
extern	void			Tim_7seg( void );
extern	char 			FlpSetChk( unsigned short );
extern	short			op_mod70( void );
extern	short			op_mod80( void );
extern	short			op_mod90( void );
extern	short			carduse( void );
extern	void			svs_renzoku( void );
extern	short			op_mod100( void );
extern	short			op_mod110( void );
extern	void			MntMain( void );
extern	void			op_indata_set( void );
extern	uchar			op_key_chek( void );
extern	void			Print_Condition_select( void );
extern	short			op_mod81( uchar );

extern	short			op_mod200( void );
extern	short			op_mod210( void );
extern	void			MakeVisitorCode(uchar *code);				//���q�l���ލ쐬
extern	ushort			CheckVisitorCode(uchar *code, ushort val);	//���͈Ïؔԍ�(Btype)����
extern	short			op_mod220( void );	// ���O�^�C����������
extern	short			op_mod230( void );	// ���Z�ς݈ē�����
// MH810100(S)
extern	void			SetPayStartTime( struct clk_rec *pTime );
extern	struct clk_rec*	GetPayStartTime( void );
// MH810100(E)

extern	char	save_op_faz;
extern	void	op_MifareStart( void );
extern	void	op_MifareStop( void );
extern	void	op_MifareStop_with_LED( void );

extern	void	ElectronSet_PayData( void *buf, uchar data_kind );
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern	void	EcElectronSet_PayData( void *buf );
extern	void	EcCreUpdatePayData( void *buf );
extern	void	EcElectronSet_DeemedData( void *buf );
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �d�q�}�l�[�V���O���ݒ�ňē��������s��
extern	void	op_EcEmoney_Anm(void);
// MH810103 GG119202(E) �d�q�}�l�[�V���O���ݒ�ňē��������s��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern void		EcQrSet_PayData( void *buf );
extern void		Ec_Data152Save( void );
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 �C���{�C�X�Ή�
extern	void	op_rct_failchk(void);
extern	void	op_jnl_failchk(void);
// MH810105(E) MH364301 �C���{�C�X�Ή�

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
extern	void	Set_Pay_RTPay_Data( void );
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
extern	ushort	GetCardKind( void );
extern	void	Set_Pay_RTReceipt_Data( void );
extern	void	MakeQRCodeReceipt( char* data, size_t size );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
extern	void	Set_Cancel_RTPay_Data( void );
extern	BOOL	ope_SendCertifCommit( void );
extern	BOOL	ope_SendCertifCancel( void );
extern	ushort	op_wait_mnt_start( void );
extern	ushort	op_wait_mnt_close( void );
extern	ushort	lcdbm_alarm_check( void );
extern	void	lcdbm_notice_alm( ushort warning );
extern	void	lcdbm_ErrorReceiveProc( void );
extern	void	lcdbm_notice_ope( eOPE_NOTIF_CODE ope_code, ushort status );
extern	BOOL	ope_AddUseMedia( tMediaDetail* pMedia );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
extern	BOOL	ope_MakeLaneLog( ushort usStsNo );
extern	void	SetLaneMedia(ulong ulParkNo, ushort usMediaKind, uchar *pMediaNo);
extern	void	SetLaneFeeKind(ushort usFeeKind);
extern	void	SetLaneFreeNum(ulong ulNum);
extern	void	SetLaneFreeStr(uchar *pStr, uchar usStrSize);
extern	void	LaneStsSeqNoUpdate(void);
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129000(S) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
extern	BOOL	ope_MakeLaneLog_Check(uchar pay_sts);
// GG129000(E) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
// GG129000(S) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
extern	uchar	QRIssueChk(uchar check_only);
extern	uchar	CarNumChk(void);
// GG129000(E) T.Nagai 2023/02/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j


typedef union{
	unsigned char	BYTE;					// Byte
	struct{
		unsigned char   STOP_REASON:3;      // Bit 4-5 = ��~���R
											//     0�F�N���W�b�g
											//     1�F����⍇��(���C)
											//     3�F����⍇��(Mifare)
											//     5�F���Z�������ɓd�q�}�̂̒�~��҂����킹��
											//   6-7�F�\��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		unsigned char	YOBI:1;				// Bit 4 = �\��
//		unsigned char	EDY_END:1;			// Bit 3 = Edy�I��
		unsigned char	YOBI:2;				// Bit 3-4 = �\��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		unsigned char	SUICA_END:1;		// Bit 2 = Suica�I��
		unsigned char	CMN_END:1;			// Bit 1 = ү��I��
		unsigned char	INITIALIZE:1;		// Bit 0 = �������
	} BIT;
} t_End_Status;

extern	t_End_Status	cansel_status;				// ���Z���~�pEdy�ESuica�Ecmn�I���҂�
extern	t_End_Status	CCT_Cansel_Status;			// �ڼޯĶ��ގg�p����Edy�ESuica��~�҂��p
enum{
	REASON_PAY_END = 1,
	REASON_CARD_READ,
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// QR�ް���M����IC�ڼޯĒ�~�������Ƃ������R��V�K�ǉ�
	REASON_QR_USE,
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
	REASON_MIF_READ,
	REASON_MIF_WRITE_LOSS,
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
	REASON_RTM_REMOTE_PAY,					// ���u���Z�J�n
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
};

extern	uchar	Ope_Last_Use_Card;			// ���̃J�[�h�̂����Ō�Ɏg�p��������
											// 1=�v���y�C2=�񐔌��C3=�d�q���σJ�[�h
											// 0=��L�܂����g�p

extern	short	tim1_mov;					// ��ϰ1�N���׸�

extern	uchar	Chu_Syu_Status;				// ���~�W�v���{�ð�� 0�F�����{ 1�F���{�ς�
extern	Receipt_data	PayInfoData_SK;		// ���Z���f�[�^�p�����E�s���o�Ƀf�[�^
extern	uchar	PayInfo_Class;				// ���Z���f�[�^�p�����敪

extern	uchar	SyuseiEndKeyChk( void );
extern void StackCardEject(uchar req);
extern uchar CardStackStatus;			// �J�[�h�l�܂菈�����
extern uchar CardStackRetry;					// �r�o���쒆�׸�
extern	char	Op_Event_Disable( ushort msg );
extern	char	Op_Event_enable( void );
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
extern	uchar	coin_err_flg;
extern	uchar	note_err_flg;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
extern const unsigned char EcBrandEmoney_Digit[];
// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�

/* fus_kyo.c */
extern	short			fus_kyo( void );

/* tokubet.c */
extern	char			tokubet( short, short, short );
extern	const uchar	Shunbun_Day[100];
extern  const uchar	Shuubun_Day[100];
extern  const uchar	MAY_6[100];

/* syuukei.c */
extern	void			kan_syuu( void );
extern	char			DoBunruiSyu( char Syubetu );
extern	void			cyu_syuu( void );
extern	void			mulkan_syu( ushort );
extern	void			mulchu_syu( ushort );
extern	short			fus_syuu( void );
extern	void			syu_syuu( void );
extern	void			lto_syuu(void);
extern	void			lto_syuko( void );
extern	void			toty_syu( void );
extern	SYUKEI			*syuukei_prn( int preq, SYUKEI *src );
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
extern	char			Check_syuukei_clr( char no );
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
extern	void			syuukei_clr( char );
extern	void			kinko_syu( char, char );
extern	void			kinko_clr( char );
extern	void			syu_init( void );
extern	void			disc_wari_conv( DISCOUNT_DATA *, wari_tiket * );
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
extern	void			disc_wari_conv_prev( DISCOUNT_DATA *, wari_tiket * );
extern	void			disc_wari_conv_all( DISCOUNT_DATA *, wari_tiket * );
// GG124100(S) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
extern	void			disc_wari_conv_new( DISCOUNT_DATA *, wari_tiket * );
// GG124100(E) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
extern	void			disc_Detail_conv( DETAIL_DATA *, wari_tiket * );
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
extern	uchar	date_uriage_use_chk( void );
extern	void	date_uriage_syoukei_judge( void );
extern	uchar	date_uriage_update_chk( void );
extern	void	date_uriage_syoukei_clr( uchar flg);
extern	void	date_uriage_prmcng_judge( void );
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//extern	void			ec_alarm_syuu( uchar brand, ulong ryo );
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
// MH810105 GG119202(S) T���v�A���󎚑Ή�
extern	void			ec_linked_total_print(ushort pri_req, T_FrmSyuukei *pFrmSyuukei);
// MH810105 GG119202(E) T���v�A���󎚑Ή�

/* turikan.c */
extern	void			turikan_proc( short );
extern	void			turikan_gen( void );
extern	void			turikan_pay( void );
extern	void			turikan_ini( void );
extern	void			turikan_prn( short );
extern	void			turikan_sfv( void );
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
extern	char			Check_turikan_clr( void );
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
extern	void			turikan_clr( void );
extern	void			turikan_clr_sub2( void );
extern	void			turikan_clr_sub( void );
extern	void			turikan_inventry( short );
extern	void			turikan_fuk( void );
extern	void			turiadd_hojyu_safe(void);
extern	void			turikan_subtube_set(void);

/* fukuden.c */
extern	void			fukuden( void );
extern	void			flp_infuku( void );

/* SystemMnt.c */
extern	void			SysMntMain( void );

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// sysmnt2.c
extern int ftp_auto_update_for_LCD( void );		// LCDӼޭ�قɑ΂������Ұ�����۰��
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))

/* UserMnt.c */
extern	void			UserMntMain( void );
// MH810100(S) Y.Yamauchi 20191212 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//extern	void 			CardErrDsp( void );
extern	void 			CardErrDsp( short err );
extern	void			push_ticket( tMediaDetail *pMedia, ushort err );
// MH810100(S) Y.Yamauchi 20191212 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
/* shtctrl.c */
extern	void			shtctrl( void );
extern	void			rd_shutter( void );
extern	void			read_sht_opn( void );
extern	void			read_sht_cls( void );
extern	void			TpuReadShut( void );
extern	void			TpuCoinShut( void );
extern	void			shutter_err_chk( uchar * );
extern	void			start_rdsht_cls( void );

#define	RD_ERR_INTERVAL		(50*10)			// 10�b�i������ُ��ԊĎ��Ԋu�j
#define	RD_SOLENOID_WAIT	(50*1)			// 1�b�i�������ɲ�ޓ���҂���ϰ�j
#define	RD_ERR_COUNT_MAX	3				// 3��i������ُ��ԊĎ��񐔁j

/* SubCpuMnt.c */
extern	void			SubCpuMnt( void );

/* DspTask.c */
extern	void			DspTask( void );

/* opered.c */
extern	void			red_int( void );
extern	void			opr_ctl( ushort );
extern	void			opr_rcv( ushort );
extern	short			opr_snd( short );
extern	void			opr_int( void );
extern	short			al_svsck( m_gtservic * );
extern	ushort			CardSyuCntChk( ushort, short, short, short, short );
extern	short			al_pasck( m_gtapspas * );
extern	short			al_pasck_renewal( m_apspas * );
extern	void			al_mkpas_renewal( m_apspas *, struct clk_rec * );
extern	short			al_preck( m_gtprepid * );
extern	short			al_kasck( m_gtservic *mag );
extern	void			al_mkpas( m_gtapspas *, struct clk_rec * );
extern	void			al_mkpre( m_gtprepid *, pre_rec * );
extern	void			al_mkkas( m_gtkaisuutik *inpp, svs_rec *pre, struct clk_rec *ck );

extern	uchar			Read_Tik_Chk( uchar *, uchar *, uchar );
extern	ushort			PkNo_get( uchar *, uchar );
extern	void			NgPkNo_set( uchar *, uchar );
extern	uchar chk_for_inquiry(uchar type);
extern	uchar	Ope_PasKigenCheck( ushort StartKigen_ndat, ushort EndKigen_ndat, short TeikiSyu, 
						   ushort Target_ndat, ushort Target_nmin );

extern	void			al_pasck_set( void );
extern	short			al_pasck_syusei( void );

extern	uchar GT_Settei_flg;					// GT̫�ϯĐݒ�NG�׸�

/* opesub.c */
extern	short			al_card( ushort, ushort );
extern	short			al_card_renewal( ushort, ushort );
extern	void			date_renewal( ushort, ushort *, uchar *, uchar * );
extern	char			LprnErrChk( char );
extern	void			LabelCountDown( void );
extern	short			hojuu_card( void );
extern	short			al_iccard( ushort, ushort );
extern	void			PayData_set( uchar, uchar );
extern	void			SetAddPayData( void );
extern	void			PayData_set_SK( ushort, ushort, ushort, ushort, uchar );
extern	void			PayData_set_LO( ushort, ushort, ushort, ushort);
extern	short			Kakariin_card( void );
extern	void			Ope_KakariCardInfoGet( uchar *pMntLevel, uchar* pPassLevel, uchar *pKakariNum );
extern	void			ryo_isu( char );
extern	uchar			ReceiptChk( void );
extern	void			chu_isu( void );
extern	void			azukari_isu(uchar cancel);
extern	char			uke_isu( ulong, ushort, uchar );
extern	char			pri_time_chk( unsigned short pr_no, uchar type );
extern	char			mtpass_get( uchar, uchar* );
extern	char			PassExitTimeTblRead( ulong, ushort, date_time_rec * );
extern	void			PassExitTimeTblWrite( ulong, ushort, date_time_rec * );
extern	void			PassExitTimeTblDelete( ulong, ushort );
extern	short			opncls( void );
extern	void			PayEndSig( void );
extern	void			CountUp_Individual( uchar kind );
extern	ulong			CountRead_Individual( uchar kind );
extern	PAS_TBL	*GetPassData( ulong ParkingNo, ushort PassId);
extern	void			WritePassTbl( ulong ParkingNo, ushort PassId, ushort Status, ulong UseParking, ushort Valid );
extern	void			GetPassArea( ulong ParkingNo, ushort *Start, ushort *End);
extern	void			FillPassTbl( ulong ParkingNo, ushort Status, ulong UseParking, ushort Valid, ushort FillType );
extern	short			ReadPassTbl( ulong ParkingNo, ushort PassId, ushort *PassData );
extern	uchar			ck_jis_credit ( uchar , char * );

extern	PAS_RENEWAL    *GetPassRenewalData( ulong, ushort, char * );
extern	short			ReadPassRenewalTbl( ulong, ushort, ushort * );
extern	void			WritePassRenewalTbl( ulong, ushort, ushort );
extern	short			GetPassRenewalArea( ulong, ushort *, ushort *, ushort *, ushort * );
extern	void			FillPassRenewalTbl( ulong, uchar );

extern	void			Ope2_Log_NewestOldestDateGet( ushort LogSyu, date_time_rec *NewestDate, date_time_rec *OldestDate, ushort *RegCount );
extern	unsigned long	Nrm_YMDHM( date_time_rec *wk_CLK_REC );
extern	void			UnNrm_YMDHM( date_time_rec *wk_CLK_REC, ulong Nrm_YMDHM );
extern	ushort			Ope2_Log_CountGet_inDate( ushort LogSyu, ushort *Date, ushort *id );
extern	BOOL			Ope2_Log_Get_inDate( ushort LogSyu, ulong Nrm_Date, ushort *id, void *pBuf );
extern	ushort			Ope2_Log_CountGet( ushort LogSyu );
extern	void			DiditalCashe_NewestOldestDateGet( ushort LogSyu, date_time_rec *NewestDate, date_time_rec *OldestDate, ushort *RegCount );
extern	short			chkdate2(short yyyy, short mm, short dd);
extern	ulong 			time_nrmlz ( ushort norm_day, ushort norm_min );
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
extern	uchar			Ope_FunCheck( char f_Button );
extern	void			Ope_Set_tyudata( void );
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
extern	void			Ope_Set_tyudata_Card( void );
extern	ulong			DC_GetCenterSeqNo( DC_SEQNO_TYPE type );
extern	void			DC_UpdateCenterSeqNo( DC_SEQNO_TYPE type );
extern	void			DC_PushCenterSeqNo( DC_SEQNO_TYPE type );
extern	void			DC_PopCenterSeqNo( DC_SEQNO_TYPE type );
extern	void			RTPay_LogRegist_AddOiban(void);			// ر���ѐ��Z�ް��̾����ǔԂ��X�V����log�ɏ������ޏ���
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
extern	void			RTReceipt_LogRegist_AddOiban(void);
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// MH810100(S) 2020/09/02 �Đ��Z���̕������Ή�
//// MH810100(S) Y.Watanabe2020/02/12 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//extern	ulong			Ope_GetLastDisc( void );
//// MH810100(E) Y.Watanabe 2020/02/12 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
extern	ulong			Ope_GetLastDisc( long lDiscount );
// MH810100(E) 2020/09/02 �Đ��Z���̕������Ή�
// MH810100(S) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
extern	ulong			Ope_GetLastDiscOrg();
// MH810100(E) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
extern BOOL Ope_SyubetuWariCheck(DISCOUNT_DATA* pDisc);
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j

// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
extern	void			CheckShoppingDisc( ulong pno, ushort *p_kind, ushort mno, ushort *p_info );
// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)

// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//extern	void			LongTermParkingRel( ulong LockNo );
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH810100(S) 2020/05/27 �Ԕԃ`�P�b�g���X(#4181)
extern void				azukari_popup(uchar cancel);
// MH810100(E) 2020/05/27 �Ԕԃ`�P�b�g���X(#4181)
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
extern void				pri_ryo_stock(uchar kind);
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
extern void				QrReciptDisp( void );
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j

#define PASS_DATA_FILL_ALL			0
#define PASS_DATA_FILL_VALID		1
#define PASS_DATA_FILL_IN_FREE		2
#define PASS_DATA_FILL_OUT_FREE		3

extern	char			AteVaild_Update(ushort id, ushort status);
extern	short			AteVaild_Check(ushort id);
extern	char			MAFParamUpdate(ushort seg, ushort add, long data);
extern	void			Ope2_WarningDisp( short time, const uchar *pMessage );
extern	void			Ope2_WarningDispEnd( void );
// MH321800(S) G.So IC�N���W�b�g�Ή�
typedef struct {			// ���[�j���O �T�C�N���b�N�\���Ǘ��\����
	short	interval;
	uchar	page;			// 0:���g�p�A1�`:�\���y�[�W
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
	uchar	mode;			// 0:�ʏ�A1:���]
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
	uchar	err_page[2][2];
} t_CyclicDisp;
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
//extern	void			Ope2_ErrChrCyclicDispStart( short interval, const uchar err_num[] );
extern	void			Ope2_ErrChrCyclicDispStart( short interval, const uchar err_num[], uchar mode );
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
extern	void			Ope2_ErrChrCyclicDispStop( void );
extern	void			Ope2_ErrChrCyclicDisp( void );
// MH321800(E) G.So IC�N���W�b�g�Ή�
extern	void			AteVaild_table_update( ushort *data, ushort size);
extern	void			SetNoPayData( void );
extern	int				subtube_use_check( int mny_type );
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
extern	void	EnableNyukin(void);
extern	void	BrandResultUnselected(void);
extern	ushort	GetBrandReasonCode(ushort msg);
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	void			EcFailureContactPri( void );
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 �C���{�C�X�Ή�
extern	long			invoice_prt_param_check( long param_jadge );
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
extern	uchar			cancelReceipt_chk( void );
extern	uchar			cancelReceipt_Waridata_chk( wari_tiket* );
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
extern	void			setting_taxableAdd( ulong*, uchar );
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
// GG129001(S) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
extern	ulong			Billingcalculation(Receipt_data *data);
// GG129001(E) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
extern	ulong			TaxAmountcalculation(Receipt_data *data , uchar typ);
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j

/* Lcdctrl.c */
extern	void			opedsp( ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort );
extern	void			opedsp3( ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort );
extern	void			opedpl( ushort, ushort, ulong, ushort, ushort, ushort, ushort, ushort );
extern	void			opedpl3( ushort, ushort, ulong, ushort, ushort, ushort, ushort, ushort );
extern	void			opedpl2( ushort, ushort, ulong, ushort, ushort, ushort, ushort, ushort );
extern	void			opedsp5( ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort);
extern	void			opedsp6( ushort, ushort, ushort, ushort, ushort, ushort, ushort );
extern	void			pntchr( uchar *, uchar );
extern	void			as1chg( const uchar *, uchar *, uchar );
extern	void			wekchg( uchar, uchar * );
extern	void			dsp_background_color( ushort rgb );
extern	void			dsp_intime( ushort, ushort );
extern	void			zrschr( uchar *, uchar );
extern	ushort			teninb( ushort, ushort, ushort, ushort, ushort );
extern	ushort			teninb_pass( ushort, ushort, ushort, ushort, uchar, ushort );
extern	void			feedsp( ushort, ulong );
extern	void			LcdBackLightCtrl( char );
extern	void			OpeLcd( ushort );
extern	ushort			blindpasword( ushort, ushort, ushort, ushort ,uchar);
extern	void			MagReadErrCodeDisp(uchar ercd);
extern	ulong			teninb2( ulong, ushort, ushort, ushort, ushort );
extern	void			Lcd_Receipt_disp(void);
extern	ushort			Ope_Disp_LokNo_GetFirstCol(void);
// MH810100(S) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
extern	void			clear_pay_status( void );
// MH810100(E) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)

/* werrlg.c */
extern	void			err_chk( char, char, char, char, char );
extern	void			err_chk2( char md, char no, char knd, char f_data, char err_ctl, void *pData );
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:222)�Ή�
//extern	void			err_chk3( char md, char no, char knd, char f_data, char err_ctl, void *pData, date_time_rec* date );
extern	void			err_chk3( char md, char no, char knd, char f_data, char err_ctl, void *pData, unsigned int length, date_time_rec* date );
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:222)�Ή�
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
extern	void			alm_chk( char, char, char );
extern	void			alm_chk2( char md, char no, char knd, char f_data, char err_ctl, void *pData );
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern	void			err_ec_chk(unsigned char *NEW, unsigned char *OLD);
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH321800(S) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
extern	void			err_ec_clear(void);
// MH321800(E) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
// MH810103 GG119202(S) �ُ�f�[�^�t�H�[�}�b�g�ύX
extern	void			err_ec_chk2(unsigned char *NEW, unsigned char *OLD);
// MH810103 GG119202(E) �ُ�f�[�^�t�H�[�}�b�g�ύX
extern	void			err_cn_chk( uchar *, uchar * );
extern	char			err_cnm_chk(void);
extern	void			err_nt_chk( uchar *, uchar * );
extern	void			err_pr_chk( uchar, uchar );
extern	void			err_pr_chk2( uchar, uchar );
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
extern	void			err_eja_chk(uchar NEW, uchar OLD);
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
extern	void			memorychk( void );
extern	void			sig_chk( void );
extern	void			fulchk( void );
extern	void			wopelg( ushort no, ulong before, ulong after );
#define	wopelg2	wopelg
extern	void			wmonlg(ushort no, void *p_info, ulong ul_info);
extern	short			turick( void );
extern	void			Ex_outport( void );
extern	void			ExOutSignal( uchar, uchar );
// MH810105(S) MH364301 �C���{�C�X�Ή�
extern	short			paperchk( void );
extern	short			paperchk2( void );
// MH810105(E) MH364301 �C���{�C�X�Ή�
extern	short			cinful( void );
extern	short			notful( void );
extern	void			err_mod_chk( unsigned char *, unsigned char *, short );
extern	short			LabelPaperchk( void );
extern	char			DoorCloseChk( void );
extern	void			ex_errlg( uchar, uchar, uchar, uchar );
extern	uchar			IsErrorOccuerd( char md, char no );
extern	ushort			IsErrorInfoGet( char type, char md, char no );
extern	ushort	GetErrorOccuerdLevel( char md, char no );
extern	void	Ope_CenterDoorResend( void );
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//extern	void	wrmonlg(ushort FuncNo, ulong Code, ushort MonInfo, u_inst_no *pInstNo, date_time_rec *pTime1, date_time_rec *pTime2, uchar *pProgNo);
extern	void	wrmonlg(ushort FuncNo, ulong Code, ushort MonInfo, u_inst_no *pInstNo, date_time_rec *pTime1, date_time_rec *pTime2, uchar *pProgNo,uchar fromflag,uchar upflag);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

enum{
	ERR_INFO_ALL = 0,
	ERR_INFO_LOGREGIST,
	ERR_INFO_JOURNAL,
	ERR_INFO_CLOSE,
	ERR_INFO_NOTICETOHOST,
	ERR_INFO_TROUBLESIGOUT,
	ERR_INFO_BIGHORN,
	ERR_INFO_MUKOU,
	ERR_INFO_LEVEL,
};

extern char				getFullFactor(unsigned char);
// MH322914(S) K.Onodera 2016/08/30 AI-V�Ή��F�G���[�E�A���[��
extern ushort			getErrLevel( ushort no, ushort code );
extern ushort			getAlmLevel( ushort no, ushort code );
// MH322914(E) K.Onodera 2016/08/30 AI-V�Ή��F�G���[�E�A���[��
// �s��C��(S) K.Onodera 2016/10/13 #1505 �A���[��01-61(5000�~�D�ނ�؂�)�������܂ރf�[�^����������Ă��܂�
extern BOOL 			isDefToErrAlmTbl( uchar type, uchar no, uchar code );
// �s��C��(E) K.Onodera 2016/10/13 #1505 �A���[��01-61(5000�~�D�ނ�؂�)�������܂ރf�[�^����������Ă��܂�

/* sigctrl.c */
extern	void			Out1shotSig_Interval( void );
extern	void			OutSignalCtrl( uchar, uchar );
extern	uchar			InSignalCtrl( ushort );
extern	void			ExIOSignalwt( uchar, uchar );			// �g��I/O�|�[�g�o��
extern	ushort			ExIOSignalrd( uchar );					// �g��I/O�|�[�g����
extern	unsigned char	read_rotsw(void);

/* opemif.c */
extern	void			OpMif_ctl( ushort );
extern	void			OpMif_snd( uchar, uchar );
extern	void			OpMif_snd2( uchar cmdid, uchar stat );
extern	short			MifareDataChk( void );
extern	void			MifareDataWrt( void );
extern	void			MifareDataWrt2( void );
extern	void			Mifare_WrtNgDataUpdate( uchar Request, void *pData );
extern	void			Mifare_WrtNgDataUpdate_Exec( void );
extern	ushort			Mifare_WrtNgDataSearch( uchar pSid[] );

/* bunrui.c */
extern	void			bunrui( ushort, ulong ,char);
// �d�l�ύX(S) K.Onodera 2016/12/13 #1674 �W�v�̐��Z�����ɑ΂���U�֕��̉��Z���@��ύX����
extern	void			bunrui_Erace( Receipt_data* pay, ulong back, char syu );
// �d�l�ύX(E) K.Onodera 2016/12/13 #1674 �W�v�̐��Z�����ɑ΂���U�֕��̉��Z���@��ύX����
extern	void			bunrui_syusei( ushort pos, ulong ryo );

/* opelpr.c */
extern	void			OpLpr_init(void);
extern	ushort			OpLpr_ctl(ushort msg, uchar *data);
extern	void			OpLpr_snd( Receipt_data *lpn, char test_flg );

/* ope_setup.c */
typedef struct {
	uchar		ProcMode;										// �����敪
	uchar		Cmnprm;											// ���ʐݒ��ް�
	uchar		Psnprm;											// �ʐݒ��ް�
	uchar		Usrprm;											// հ�ް���Ұ��ް�
	uchar		PriHdr;											// �̎���ͯ�ް�ް�
	uchar		PriPrm;											// �̎���̯���ް�
	uchar		PriLg;											// ۺވ��ް�
	uchar		FlpPrm;											// �Ԏ����Ұ��ݒ�
	uchar		LckPrm;											// ۯ����u���Ұ��ݒ�
} SETUP_NTNETDATA;

extern	uchar	OPESETUP_StartSetup(SETUP_NTNETDATA *Data);
extern	void	OPESETUP_SetupDataCancel(SETUP_NTNETDATA *Data);
extern	uchar	OPESETUP_SetupDataChk(uchar ID, ushort Status);

extern	short	AN_buf;

extern	void	dspCyclicMsgInit(void);
extern	uchar	dspIsCyclicMsg(void);
extern	void	dspCyclicMsgRewrite(uchar);
extern	void	dspCyclicErrMsgRewrite();

extern	void	err_suica_chk( uchar *, uchar *, uchar );
extern	short	al_emony( ushort msg, ushort paymod );
// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//extern	short	al_emony_sousai(void);
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
extern	ushort	Ope_Disp_Media_Getsub( uchar mode );
extern	ushort	Ope_Disp_Media_GetMsgNo( uchar mode, uchar type1, uchar type2 );
extern	ushort	Ope_Disp_Receipt_GetMsgNo(void);
extern	ushort	SysMnt_Log_CntDsp2( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd, ushort *NewOldDate );
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	void	Edy_CentComm_sub( uchar disp_type, uchar comm_type );		// �Z���^�[�ʐM���{���[�`��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define	OPE_RECIPT_LED_DELAY	(10*2)								// �̎��ؔ��s�����E���~�̎��؋@�\�ł̗̎��ؔ��s���̎�o�����_�Ŏ���(10S,*500msec)
extern	void	op_ReciptLedOff( void );
extern void		add_month ( struct clk_rec *clk_date, unsigned short add_val );
extern int		al_preck_Kigen ( m_gtprepid* mag, struct clk_rec *clk_Kigen );
extern void		GetPrcKigenStr  ( char *PccUkoKigenStr );

extern  uchar	MifStat;
// Mifare�����ݏ��
enum {
	MIF_WAITING = 0,		// 0=�����݂��Ă��Ȃ�
	MIF_WRITING,			// 1=�������ݏ�����
	MIF_WROTE_CMPLT,		// 2=�����݊����i����j
	MIF_DATA_ERR,			// 3=�f�[�^�G���[
	MIF_WROTE_FAIL,			// 4=�����ݎ��s�i���궰������҂��j
};

extern	ushort	Ope2_Log_CountGet_inDateTime( ushort LogSyu, ushort *NewDate, ushort *OldDate, ushort NewTime, ushort OldTime );
extern	ushort	SysMnt_Log_CntDsp3(ushort LogSyu, const uchar *title, ushort PreqCmd);
extern	ushort	Ope2_Log_CountGet_inDateTime_AttachOffset( ushort LogSyu, ushort *NewDate, ushort *OldDate, ushort NewTime, ushort OldTime, ushort *FirstIndex, ushort *LastIndex );
extern	ushort	Ope2_Log_NewestOldestDateGet_AttachOffset(ushort LogSyu, date_time_rec *NewestDate, date_time_rec *OldestDate, ushort *FirstIndex, ushort *LastIndex );
extern	int		Ope_isPrinterReady(void);
extern	int		Ope_isJPrinterReady(void);
extern	unsigned short	Cardress_DoorOpen( void );

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	void	Edy_CentComm_Recv( uchar type );
//extern	void	Edy_CentComm_Blink( void );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

/* oiban.c */
extern	ulong	CountSel( ST_OIBAN *oiban );
extern	int		CountCheck( void );
extern	void	CountFree( uchar kind );

/* define */
#define		SETDIFFLOG_BAK				200		// �P�������Ɋi�[�\�ȍ����f�[�^���i255 �ȉ��̂���)
#define		SETDIFFLOG_MAX				1010	// �����o�b�t�@�S�̐� ( ��ͯ�ް����ѽ���߂��܂܂��A����Ă��̒l���i�[�\�S�����ł͂Ȃ� )
#define		REM_SET_CALSCH_ADDRLEN		3		// ���u�����ݒ�@�ݒ�\�A�h���X�w��̐ݒ�l�̐��i�J�n�A�I���P�Z�b�g�Ƃ����Z�b�g��)

/* enum */
// �f�[�^�ʃw�b�_�[�A�ύX���
enum{
	SETDIFFLOG_SYU_NONE = 0,					// �ύX��ʂȂ�
	SETDIFFLOG_SYU_DEFLOAD,						// �f�t�H���g���[�h
	SETDIFFLOG_SYU_SYSMNT,						// ��������ݽ�ɂĕύX
	SETDIFFLOG_SYU_USRMNT,						// հ�ް����ݽ�ɂĕύX
	SETDIFFLOG_SYU_PCTOOL,						// PC�ݒ�°قɂĕύX
	SETDIFFLOG_SYU_REMSET,						// ���u�ݒ�ɂė����E�p�X���[�h�ύX
	SETDIFFLOG_SYU_ROLLBACK,					// �O��^�p�l�߂��ɂĕύX
	SETDIFFLOG_SYU_FCSCH,						// �����ݒ�\��iSysMnt)�ɂĕύX
	SETDIFFLOG_SYU_PCDEFLOAD,					// PC�ݒ�°ق�����̫��۰�ގw��
	SETDIFFLOG_SYU_RBACKSCH,					// �O��^�p�l�߂��ɂĕύX(SysMnt)
	SETDIFFLOG_SYU_INVALID,						// �Œ�f�[�^�ύX�ɂċ����߂�
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X (#4014 RSW=5���ݒ�X�V���󎚂ɋL�^����Ȃ�)
	SETDIFFLOG_SYU_IP_DEF,						// RSW=5�N���ɂ��IP�C�|�[�g�f�t�H���g�Z�b�g
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X (#4014 RSW=5���ݒ�X�V���󎚂ɋL�^����Ȃ�)
	SETDIFFLOG_SYU_MAXV							// SetDiffPrnStr_1�̔z��(���O��ʐ�)
};
// ����̪����׸�
enum {
	SETDIFFLOG_CYCL_IDLE = 0,					// IDLE / �i�[�I��
	SETDIFFLOG_CYCL_MIRROR,						// ���ʃp�����[�^�̃~���[�̈敡��
	SETDIFFLOG_CYCL_ACTIVE,						// �w�b�_�[�i�[��
	SETDIFFLOG_CYCL_DIFFING,					// �������i�[��
	SETDIFFLOG_CYCL_BOTTOM
};

/* struct */
// �o�b�t�@�w�b�_�[
typedef struct {
	unsigned short		UseInfoCnt;				// �g�p�σf�[�^�� Info �̎g�p��
	unsigned short		InPon;					// ���̓|�C���^�[ Info[n]�� n
	unsigned short		OlderPon;				// �ŌẪ|�C���^�[ Info[n] �� n
} TD_SETDIFFLOG_INDX;
// �ʃf�[�^�w�b�_�[(�P���� : 12Byte)
typedef struct {
	unsigned char	hed_mark[2];				// ͯ�ްϰ� 'H','D'
	unsigned short	zen_cnt;					// ������񌏐�
	unsigned char	count;						// �����f�[�^�i�[����( 0 -- SETDIFFLOG_BAK_MAX ):����ͯ�ް�,��ѽ�����,̸Ͻ�
												// �G���A����`�̊i�[�������ޯ̧�S������菬�����ꍇ�� zen_cnt > count ���L�蓾��.
	unsigned char	syu;						// ���
	unsigned short	year;						// �������(�N)
	unsigned char	mont;						// �������(��)
	unsigned char	date;						// �������(��)
	unsigned char	hour;						// �������(��)
	unsigned char	minu;						// �������(��)
} TD_SETDIFFLOG_HEAD;
// �ʃf�[�^�������(�Q�`������ : 12Byte)
typedef struct {
	short	ses;								// ���ʃp�����[�^(�Z�b�V����)
	short	add;								// ���ʃp�����[�^(�A�h���X)
	long	bef;								// �ύX�O�ݒ�l
	long	aft;								// �ύX��ݒ�l
} TD_SETDIFFLOG_DIFF;
// ������� UNION(�S�� 12 Byte�ł��邱��)
typedef union {
	TD_SETDIFFLOG_HEAD	head;					// �ʃf�[�^�w�b�_�[
	TD_SETDIFFLOG_DIFF	diff;					// �ʃf�[�^�������
}TD_SETDIFFLOG_INFO;
// �ݒ�ύX�����o�b�t�@
typedef struct {
	TD_SETDIFFLOG_INDX	Header;					// ���O�C���f�b�N�X
	TD_SETDIFFLOG_INFO	Info[SETDIFFLOG_MAX];	// �ʃf�[�^([0]=�w�b�_�[/[1�`m]=�������)
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
} TD_SETDIFFLOG;
// ���d�����p�ۑ�
typedef struct {
	unsigned char		cycl;					// see 'SETDIFFLOG_CYCL_XXX' 
	TD_SETDIFFLOG_INDX	Header;					// ���O�C���f�b�N�X
	TD_SETDIFFLOG_INFO	Info[SETDIFFLOG_BAK+1];	// 1=header
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͍Ō���ɒǉ����邱��!!!
	//	   �r���Ƀ����o��}�����Ȃ�����
} TD_BCKDIFFLOG;

/* extern */
extern TD_SETDIFFLOG	SetDiffLogBuff;			// �ݒ�l�ύX�����o�b�t�@
extern TD_BCKDIFFLOG	SetDiffLogBack;			// ���d��ض��ؗp

/* function program / subroutine prottype */
extern void SetSetDiff ( unsigned char syubetu  );
extern void SetDiffLogBackUp ( void );
extern void SetDiffLogRecoverCheck_Go ( void );
extern void SetDiffLogReset( uchar ope_mode );
extern unsigned short SetDiff_UpdatePon ( unsigned short OldPon, unsigned short AddPon );

/* ---------------------------------------------------------------------------------------------------------
   �ݒ�l�ύX�����o�b�t�@���C�A�E�g�S�̑�                          7                  0
                                                               +-- +------------------+
  7                           0                                |   |ͯ�ްϰ� 1 'H'    | �Œ蕶��
  +---------------------------+                                |   +------------------+
  | �g�p�Ϗ��              |                                |   |ͯ�ްϰ� 2 'D'    | �Œ蕶��
  +--                       --+                                |   +------------------+
  | 0 -- SETDIFFLOG_MAX       |                                |   |�����f�[�^�S����  |
  +---------------------------+                                |   +--              --+
  | ���ݓ��̓|�C���^          |                                |   | 0 -- 1699        |
  +--                       --+                                |   +------------------+
  | 0 -- SETDIFFLOG_MAX-1     |                                |   |�����ް��i�[����  | 0 -- 200
  +---------------------------+        7               0       |   +------------------+
  | �ŌẪf�[�^�|�C���^      |    +-- +---------------+       |   |�ύX���          | SETDIFFLOG_SYU_XXXXXXX
  +--                       --+    |   | �ʃw�b�_�[  | ------+-- +------------------+
  | 0 -- SETDIFFLOG_MAX-1     |    |   +---------------+       |   | ����N           |
  +---------------------------+    |   | �����ް��P����| --+   |   +--              --+
  |  ������� ���̂P          |    |   +---------------+   |   |   |                  |
  | (�ʃw�b�_�[ + ��ѽ����) | ---+   |               |   |   |   +------------------+
  | + (12 Byte x ��)          |    |  ==       ��      ==  |   |   | ��               |
  +---------------------------+    |   |               |   |   |   +------------------+
  |                           |    |   +---------------+   |   |   | ��               |
  ==           ��            ===   |   | �����ް�������|   |   |   +------------------+
  |                           |    +-- +---------------+   |   |   | ��               |
  +---------------------------+                            |   |   +------------------+
  |  ������� ���̂m          |                            |   |   | ��               |
 =|                           |                            |   +-- +------------------+
  |                           |                            |                           
  +---------------------------+                            |       7                  0
                                                           +---+-- +------------------+
                                                               |   | �ݒ�A�h���X     |
                                                               |   +--              --+
                                                               |   |                  |
                                                               |   +------------------+
                                                               |   | �ύX�O�f�[�^     |
                                                               |   +--              --+
                                                               |   |                  |
                                                               |   +------------------+
                                                               |   | �ύX��f�[�^     |
                                                               |   +--              --+
                                                               |   |                  |
�E�g�p�ςݏ�񐔂� 6Byte ��-��g�p��                            +-- +------------------+
�E�������̍ŏ��͌ʃw�b�_�[���A�ŏ��{�P����ѽ���ߌŒ�(�Œ�12Byte)
�E���ݓ��̓|�C���^�A�ŌÃ|�C���^�͂��ꂼ��̌ʃw�b�_�[�̈ʒu������
�E���ꂼ��̗������( 12 x m byte )�̃T�C�Y�� ���������ް��i�[���� x 6 + (12 x 2)
�E�ޯ̧�����т܂Ŏg�p�����ꍇ�͐擪�ɂ܂�肱�ށi�ŌÏ�񐔕�������)=(ͯ�ް + �����ް���������)
�E�ύX��ʂɂ���ẮA�͍����ް��Ȃ��Ō�ͯ�ް�݂̂����肦��
�E�i�[�����ƑS������ != �̏ꍇ�����肦��A�i�[�� ����Ő؂�
--------------------------------------------------------------------------------------------------------- */

#define		PRC_GENDOGAKU_MIN		10			// �v���y�C�h�J�[�h���Z���x�z�P�O�~.
extern  void	op_RtcFlagRegisterRead( void );
extern  void	op_RtcRegisterInit( void );
extern	uchar	timset( struct clk_rec * );
extern	uchar	timerd( struct clk_rec * );
extern	ushort	RTC_ClkDataReadReq( short );	// ���v�f�[�^�ǂݍ��ݗv��
extern	void	I2C_Event_Analize( void );	// ���v�f�[�^�ǂݍ��ݗv��
extern	void	I2C_BusError_Analize( void );
typedef struct {
	ushort		key;							// ���s�[�g�L�[
	uchar		enable;							// ���s�[�g�L��/����
	uchar		repeat;
} t_OpeKeyRepeat;

extern	void	Ope_KeyRepeatEnable(char mnt_mode);
extern	void 	ChekModeMain( void );
extern 	void	Ope_StartDoorKnobTimer(void);
extern 	void	Ope_StopDoorKnobTimer(void);
extern	void	Ope_EnableDoorKnobChime(void);
extern	void	Ope_DisableDoorKnobChime(void);
extern	uchar	Ope_IsEnableDoorKnobChime(void);
extern	void	Ope_CenterDoorTimer( char );

// �����v�Z�V�~�����[�^
extern	int		CheckOpeCondit(void);
extern	int		CheckReqCalcData(void);

// MH322914(S) K.Onodera 2016/09/14 AI-V�Ή��F�U�֐��Z
extern void	ope_PiP_GetFurikaeGaku( PIP_FURIKAE_INFO *st );
// MH322914(E) K.Onodera 2016/09/14 AI-V�Ή��F�U�֐��Z

// MH810103 GG119202 (s) #5320 ���j�^�R�[�h�́uR0252�v�A�uR0253�v���ʐM�f�[�^�ő��M����Ă��܂�					
extern uchar chk_mon_send_ntnet( uchar MonKind,uchar MonCode);
extern uchar chk_opemon_send_ntnet( uchar OpeMonKind,uchar OpeMonCode);
// MH810103 GG119202 (e) #5320 ���j�^�R�[�h�́uR0252�v�A�uR0253�v���ʐM�f�[�^�ő��M����Ă��܂�					
// MH810103(s) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�
extern void RTPay_pre_sousai(void);
extern void RTPay_set_sousai(void);
// MH810103(e) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�

// �e�X�g�p���O�쐬(S)
//#define TEST_LOGFULL
#ifdef TEST_LOGFULL
extern void Test_Ope_LogFull(void);
#endif	// TEST_LOGFULL
// �e�X�g�p���O�쐬(E)

#endif	// _OPE_DEF_H_

