#ifndef	_IFM_CTRL_H_
#define	_IFM_CTRL_H_

#include	"mem_def.h"

/*[]----------------------------------------------------------------------[]*/
/*| PARKiPRO�Ή�                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : ART                                                      |*/
/*| Date        : 2007-02-28                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/

#define IFM_DATA_MAX		250

#pragma pack
/*--------------------------------------------------------------------------*/
/*	IFM�d����{�`��															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort	mlen;
	uchar	ID[2];
	uchar	data[1];
} tIFMMSG;

/*--------------------------------------------------------------------------*/
/*	��{�ް�(���ԏꇂ�`�\��)												*/
/*--------------------------------------------------------------------------*/
// MH810100(S) K.Onodera 2020/01/07 �Ԕԃ`�P�b�g���X(�@��R�[�h)
//#define MODEL_CODE_FT4000	"0220"
#define MODEL_CODE_GT4100	"0581"
// MH810100(E) K.Onodera 2020/01/07 �Ԕԃ`�P�b�g���X(�@��R�[�h)

typedef	struct {
	uchar			ParkingNo[4];		// ���ԏꇂ
	uchar			MachineNo[2];		// �@�B��
	uchar			ModelCode[4];		// �@��R�[�h
	uchar			Rslt[2];			// �\��(�������̂�)
} tIFMMSG_BASIC;

// �Ԏ����
typedef struct {
	uchar			Area[2];			// ���
	uchar			No[4];				// �Ԏ��ԍ�
} t_ShedInfo;

/*----------*/
/*	�v���p�@*/
/*----------*/

/*--------------------------------------------------------------------------*/
/*	�f�[�^�v��(�f�[�^���30/31)												*/
/*--------------------------------------------------------------------------*/
/*--------------------*/
/*	�v���n�T�u�\���́@*/
/*--------------------*/
// �f�[�^�v���P
typedef struct {
	uchar			ReqID[2];			// �v���f�[�^ID
	t_ShedInfo		CarInfo;			// �Ԏ����
	uchar			Reserve[14];		// �\��
} tREQ30;

// ���ݎ����ύX�v��
typedef struct {
	uchar			Year[4];			// �N
	uchar			Mon[2];				// ��
	uchar			Day[2];				// ��
	uchar			Hour[2];			// ��
	uchar			Min[2];				// ��
	uchar			Sec[2];				// �b
	uchar			Reserve[8];			// �\��
} tREQ40;

// �t���b�v�E���b�N����v��
typedef struct {
	uchar			Reserve1[2];		// �\���P
	t_ShedInfo		CarInfo;			// �Ԏ����
	uchar			ProcMode[2];		// �����敪
	uchar			Reserve2[12];		// �\���Q
} tREQ41;

// ��t�����s�v��
typedef struct {
	uchar			Reserve1[2];		// �\���P
	t_ShedInfo		CarInfo;			// �Ԏ����
	uchar			Reserve2[14];		// �\���Q
} tREQ42;

// �U�֐��Z���s�\�`�F�b�N�E�U�֐��Z�v��
typedef struct {
	uchar			Reserve1[2];		// �\���P
	t_ShedInfo		CarInfo1;			// �U�֌��Ԏ����
	t_ShedInfo		CarInfo2;			// �U�֐�Ԏ����
	uchar			Reserve2[1];		// �\��
	uchar			PassUse;			// ������p�L���iID45�ɂ͑��݂��Ȃ��j
	uchar			Reserve3[6];		// �\��
} tREQ46;

typedef	struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
										// �������͊�{�f�[�^�̗\�񕔂Ɍ��ʂ�����
	union {
		tREQ30		r30;
		tREQ40		r40;
		tREQ41		r41;
		tREQ42		r42;
		tREQ46		r45;
		tREQ46		r46;
	} req;
} tIFMMSG_REQ;

typedef	tIFMMSG_REQ	tIFMMSG30;
typedef	tIFMMSG_REQ	tIFMMSG40;
typedef	tIFMMSG_REQ	tIFMMSG41;
typedef	tIFMMSG_REQ	tIFMMSG42;
typedef	tIFMMSG_REQ	tIFMMSG45;
typedef	tIFMMSG_REQ	tIFMMSG46;

/*--------------------------------------------------------------------------*/
/*	�̎��؍Ĕ��s�v��(�f�[�^���43)											*/
/*--------------------------------------------------------------------------*/
// ���Z���
typedef struct {
	uchar			Method[2];			// ���Z���@
	uchar			PayClass[2];		// �����敪
	uchar			Mode[2];			// ���Z���[�h
	uchar			OutKind[2];			// ���Z�o��
	uchar			Receipt;			// �̎��؏o��
	uchar			Empty;				// ��
	uchar			PayCount[6];		// ���Z�^���Z���~�ǔ�
	uchar			tmPay[8];			// ���Z�����iMMDDHHMM�j
	t_ShedInfo		CarInfo;			// �Ԏ����
	uchar			ParkingNo[4];		// ��������ԏ�ԍ�
	uchar			PassKind[2];		// ��������
	uchar			PassID[6];			// �����ID�i�_��ԍ��j
	uchar			AntiPassChk;		// �A���`�p�X�`�F�b�N
	uchar			StockCnt;			// �݌ɃJ�E���g
	uchar			FareKind[2];		// �������
	uchar			tmEnter[8];			// ���Ɏ���
	uchar			Fare[6];			// ���ԗ���
} t_PayInfo;

// ���z���
typedef struct {
	uchar			Cash[6];			// �������グ�z
	uchar			Entered[6];			// �������z
	uchar			Change[6];			// �ޑK�z
	uchar			Shortage[6];		// ���o���s���z
	uchar			CreditCard[6];		// �N���W�b�g�J�[�h���p���z
} t_SumInfo;

typedef	struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	uchar			Serial[2];			// �f�[�^�ǔ�
	t_PayInfo		PayInfo;			// ���Z���
	t_SumInfo		SumInfo;			// ���z���
	uchar			dummy[10];			// �\�񁕗\��
} tIFMMSG43;

/*--------------------------------------------------------------------------*/
/*	���z���Z�v��(�f�[�^���44)												*/
/*--------------------------------------------------------------------------*/
// ���z���Q
typedef struct {
	uchar			Rest[6];			// �x���c�z
	uchar			Entered[6];			// �������z
	uchar			Change[6];			// �ޑK�z
	uchar			Shortage[6];		// ���o���s���z
	uchar			Discount[6];		// ���ԗ��������z
} t_SumInfo2;

typedef	struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	uchar			SerNo[2];			// �f�[�^�ǔ�
	t_PayInfo		PayInfo;			// ���Z���
	t_SumInfo2		SumInfo;			// ���z���
	uchar			dummy[10];			// �\�񁕗\��
} tIFMMSG44;

/*--------------------------------------------------------------------------*/
/*	IBK�G���[�ʒm(�f�[�^���B0)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			mlen;
	uchar			ID[2];

	uchar			MachineNo[2];		// �@�B��		(="01")
	uchar			ModelCode[2];		// �@��R�[�h	(="00")
	uchar			ModuleCode[2];		// ���W���[���R�[�h	(="55" or "12")
	uchar			ErrorCode[2];		// �G���[�R�[�h	(="01"�`"99")
	uchar			ErrorOccur[2];		// �����^����	(="01"�`"03")
	uchar			dummy[8];
} tIFMMSGB0;

/*----------*/
/*	�����p	*/
/*----------*/

/*--------------------------------------------------------------------------*/
/*	IBK�������N���A�v��(�f�[�^���57)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			mlen;
	uchar			ID[2];

	uchar			dummy1[3];			// �\��
	uchar			req;				// �ʒm�i='2'�`IBK�C�j�V�����C�Y�̂݁j
	uchar			dummy2;				// �\��
} tIFMMSG57;

/*--------------------------------------------------------------------------*/
/*	�f�[�^�v��NG(�f�[�^���60/61)											*/
/*--------------------------------------------------------------------------*/
typedef	tIFMMSG_REQ	tIFMMSG60;

enum {
	_IFM_ANS_SUCCESS,
	_IFM_ANS_REFUSED,
	_IFM_ANS_INVALID_PARAM,
	_IFM_ANS_FROM_NG,
	_IFM_ANS_TO_NG,
	_IFM_ANS_NOT_FOUND,
	_IFM_ANS_IMPOSSIBLE,
	_IFM_ANS_NOW_BUSY,
	_IFM_ANS_reserved2,
	_IFM_ANS_OTHER_ERR,
	_IFM_ANS_DONE = 20,
	_IFM_ANS_CANCEL,
	_IFM_ANS_ABNORMAL
};

/*--------------------------------------------------------------------------*/
/*	���ԏ���(�f�[�^���62)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			CurTime[14];		// ���Z�@���ݎ����f�[�^
	uchar			Stat;				// �c�x�Ə�
	uchar			Mode;				// ���Z�@���샂�[�h��
	uchar			Err;				// �G���[������
	uchar			Alarm;				// �A���[��������
	uchar			Full1;				// ���ԂP��
	uchar			Full2;				// ���ԂQ��
	uchar			Full3;				// ���ԂR��
	uchar			Reserve1;			// �\���P
	uchar			Reserve2[7];		// �\���Q
	uchar			ShedNum[3];			// �Ԏ���
} tM_STATUS;

typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	tM_STATUS		Stat;				// ���Z�@���
	t_ShedInfo		CarInfo[324];		// �Ԏ����
	uchar			Reserve[6];			// �\��
} tIFMMSG62;

/*--------------------------------------------------------------------------*/
/*	���ԏ���E��(�f�[�^���63)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	tM_STATUS		Stat;				// ���Z�@���
	uchar			Reserve[6];			// �\��
} tIFMMSG63;

/*--------------------------------------------------------------------------*/
/*	�S�Ԏ���(�f�[�^���64)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			Area[2];			// ���
	uchar			No[4];				// �Ԏ��ԍ�
	uchar			Mode;				// �������[�h
	uchar			Stat1;				// �X�e�[�^�X�P
	uchar			Stat2;				// �X�e�[�^�X�Q
	uchar			Stat3;				// �X�e�[�^�X�R
	uchar			ParkTime[8];		// ���Ɏ����iMMDDHHMM�j
} t_ShedStat;

typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	tM_STATUS		Stat;				// ���Z�@���
	t_ShedStat		ShedStat[324];		// �Ԏ���
	uchar			Reserve[6];			// �\��
} tIFMMSG64;

/*--------------------------------------------------------------------------*/
/*	�P�Ԏ���(�f�[�^���65)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	tM_STATUS		Stat;				// ���Z�@���
	t_ShedStat		ShedStat;			// �Ԏ���
	uchar			Reserve[12];		// �\��
} tIFMMSG65;

/*--------------------------------------------------------------------------*/
/*	���Z�󋵁E���Z�f�[�^(�f�[�^���66/67)									*/
/*--------------------------------------------------------------------------*/
// �����ڍ�
typedef struct {
	uchar			ParkingNo[3];		// ���ԏꇂ
	uchar			kind[3];			// �������
	uchar			div[3];				// �����敪
	uchar			used[3];			// �g�p����
	uchar			sum[6];				// �����z
	uchar			inf1[6];			// �������P
	uchar			inf2[6];			// �������Q
} t_DiscountInfo;

typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	uchar			Serial[2];			// ���Z������ԁ^�f�[�^�ǔ�
	t_PayInfo		PayInfo;			// ���Z���
	t_SumInfo		SumInfo;			// ���z���
	t_DiscountInfo	DscntInfo[25];		// �����ڍ�
	uchar			Reserve[4];			// �\��
} tIFMMSG_PAY;

typedef	tIFMMSG_PAY	tIFMMSG66;
typedef	tIFMMSG_PAY	tIFMMSG67;

/*--------------------------------------------------------------------------*/
/*	�G���[�f�[�^(�f�[�^���68)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {	// error data
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	uchar			Reserve1[2];		// �\���P
	uchar			ModuleCode[4];		// ���W���[���R�[�h
	uchar			ErrCode[4];			// �G���[�R�[�h
	uchar			Stat[2];			// ����������
	uchar			Time[8];			// ��������
	uchar			Reserve2[2];		// �\���Q
} tIFMMSG68;

/*--------------------------------------------------------------------------*/
/*	�����ėp(�f�[�^���70�`74)												*/
/*--------------------------------------------------------------------------*/
typedef	tIFMMSG40	tIFMMSG70;
typedef	tIFMMSG41	tIFMMSG71;
typedef	tIFMMSG42	tIFMMSG72;
typedef	tIFMMSG43	tIFMMSG73;
typedef	tIFMMSG44	tIFMMSG74;

/*--------------------------------------------------------------------------*/
/*	�U�֐��Z�`�F�b�N(�f�[�^���75/76)										*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			FailedPaid[6];		// �U�֌��x���z
	uchar			Switchable[6];		// �U�։\�z
	uchar			OrgFare[6];			// �U�֐撓�ԗ���
	uchar			OrgArrear[6];		// �U�֐�x���c�z
	uchar			ActArrear[6];		// �U�֌�x���c�z
	
} tSWP_INFO;

typedef	struct {	// error data
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// ��{�f�[�^
	uchar			PayMode[2];			// ���Z�������
	t_ShedInfo		CarInfo1;			// �U�֌��Ԏ����
	t_ShedInfo		CarInfo2;			// �U�֐�Ԏ����
	uchar			Reserve1;			// �\��
	uchar			PassUse;			// ������p�L��
	tSWP_INFO		SwInfo;				// �U�֋��z���
	uchar			Reserve2[8];		// �\��
} tIFMMSG_SWP;

typedef	tIFMMSG_SWP	tIFMMSG75;
typedef	tIFMMSG_SWP	tIFMMSG76;


#pragma unpack

/*--------------------------------------------------------------------------*/
/*	��M�o�b�t�@															*/
/*--------------------------------------------------------------------------*/
typedef	union {
	tIFMMSG		buff;
	tIFMMSG30	msg30;
	tIFMMSG40	msg40;
	tIFMMSG41	msg41;
	tIFMMSG42	msg42;
	tIFMMSG43	msg43;
	tIFMMSG44	msg44;
	tIFMMSG45	msg45;
	tIFMMSG46	msg46;
	tIFMMSGB0	msgB0;
	char		dummy[sizeof(tIFMMSG)-1+IFM_DATA_MAX];
} tIFM_RCV_BUF;

/*--------------------------------------------------------------------------*/
/*	���M�L���[																*/
/*--------------------------------------------------------------------------*/
typedef	union {
	tIFMMSG		buff;
	tIFMMSG67	msg67;			// ���Z�f�[�^
	tIFMMSG68	msg68;			// �G���[�f�[�^
	tIFMMSG57	msg57;
	tIFMMSG74	msg74;			// ���z���Z�v������
	tIFMMSG76	msg76;			// �U�֐��Z����
	tIFMMSG60	msg60;			// �f�[�^�v��NG
	tIFMMSG62	msg62;			// ���ԏ���
	tIFMMSG63	msg63;			// ���ԏ���i���j
	tIFMMSG64	msg64;			// �S�Ԏ����
	tIFMMSG65	msg65;			// �P�Ԏ����
	tIFMMSG66	msg66;			// ���Z������
	tIFMMSG70	msg70;			// ���ݎ����ύX����
	tIFMMSG71	msg71;			// �t���b�v���b�N���䉞��
	tIFMMSG72	msg72;			// ��t�����s����
	tIFMMSG73	msg73;			// �̎��ؔ��s����
	tIFMMSG75	msg75;			// �U�֐��Z���O�`�F�b�N����
} tIFM_SND_BUF;

typedef	struct {
	ushort		blk_num;			// �u���b�N��
	ushort		last_blk_size;		// �ŏI�u���b�N�T�C�Y
	ushort		lb_revised_size;	// �ŏI�u���b�N�␳�T�C�Y
	ushort		snd_blk;
//	ushort		src_que;
} tIFM_SND_CTR;

/*--------------------------------------------------------------------------*/
/*	���Z�f�[�^�e�[�u��														*/
/*--------------------------------------------------------------------------*/
typedef struct {
	unsigned char	COUNT;						// �d����(�O�`�Q�O)
	unsigned char	RIND;						// �ŌÓd���ʒu(�O�`�P�X)
	unsigned char	CIND;						// �������ݓd���ʒu(�O�`�P�X)
	struct {
		unsigned short	LEN;					// �d����(�r�s�w�`�a�b�b)
		unsigned char	BUFF[259 * 4];			// �f�[�^�o�b�t�@(�Q�T�X�~�S)
	} DATA[20];									// �Q�O��
} t_PIP_PayTable;								// ���Z�f�[�^�e�[�u��

/*--------------------------------------------------------------------------*/
/*	�G���[�f�[�^�e�[�u��													*/
/*--------------------------------------------------------------------------*/
typedef struct {
	unsigned char	COUNT;						// �d����(�O�`�Q�O)
	unsigned char	RIND;						// �ŌÓd���ʒu(�O�`�P�X)
	unsigned char	CIND;						// �������ݓd���ʒu(�O�`�P�X)
	struct {
		unsigned short	LEN;					// �d����(�r�s�w�`�a�b�b)
		unsigned char	BUFF[67];				// �f�[�^�o�b�t�@
	} ERROR[20];								// �t���b�v�E���b�N��(�Q�O��)
} t_PIP_ErrTable;								// �G���[�f�[�^�e�[�u��
extern	t_PIP_PayTable	PAYTBL;		// ���Z�f�[�^�e�[�u��
extern	t_PIP_ErrTable	ERRTBL;		// �G���[�f�[�^�e�[�u��

/*--------------------------------------------------------------------------*/
/*	�v���g�^�C�v															*/
/*--------------------------------------------------------------------------*/
void	IFM_Init(int clr);
void	IFM_Snd_ErrorClear(void);
void	IFM_Snd_Error(void);
extern	void	IFM_Snd_Payment(const Receipt_data *dat);
extern	void	IFM_Accumulate_Payment_Init();
extern	void	IFM_Accumulate_Error_Init();
void	IFM_Snd_AccumulateError(uchar count);
void	IFM_Snd_AccumulatePayment(uchar count);
void	IFM_Snd_GengakuAns(int sts);
void	IFM_Snd_FurikaeAns(int sts, struct VL_FRS *frs);

extern	ushort	pipcomm;
#define	_is_pip()		(pipcomm)
#define	_is_not_pip()	(!pipcomm)
// MH322914(S) K.Onodera 2016/10/12 AI-V�Ή�
#define	_is_Normal_pip()		(pipcomm == 1)
#define	_is_ParkingWeb_pip()	(pipcomm == 2)
// MH322914(E) K.Onodera 2016/10/12 AI-V�Ή�

#endif
