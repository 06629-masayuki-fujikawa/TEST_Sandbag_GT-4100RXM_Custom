/*[]------------------------------------------------------------------------------[]*/
/*|	����			:Ӽޭ�يԒʐM����@��֘A�Ŏg�p�����ް����̒�`���s��		   |*/
/*|	�t�@�C����		:mdl_def.h								 					   |*/
/*|	�^�[�Q�b�gCPU	:H8S/2352								 					   |*/
/*|	Author    		:Tadashi_nakayama	Amano Co.,Ltd.		 					   |*/
/*|	Date    		:2004.5.19								 					   |*/
/*[]------------------------------------------------------------------------------[]*/
/*|	specifications											 					   |*/
/*|		keymag.h�ް���`�����ڐA���܂�						 					   |*/
/*|		MDLSUB.h �֐��^�錾���ڐA���܂�						 					   |*/
/*[]------------------------------------------------------------------------------[]*/
/*|		total_def.h , memdef.h�̌�ɐ錾���Ďg�p���܂�		 					   |*/
/*[]------------------------------------------------------------------------------[]*/
#ifndef _MDL_DEF_H_
#define _MDL_DEF_H_

#include "SodiacAPI.h"

/*[]------------------------------------------------------------------------------[]*/
/*|		Reader , Anounce Machine												   |*/
/*[]------------------------------------------------------------------------------[]*/
#define		MTS_RED			0						/* Ӽޭ�يԒʐM���Cذ��			*/
#define		MTS_AVM			1						/* Ӽޭ�يԒʐM�����ē�			*/
#define		MTS_MAX			(MTS_AVM+1)				/* Ӽޭ�يԒʐM�ő�ڑ���		*/
													/*								*/
													/*								*/
													/*------------------------------*/
struct	red_rec	{									/* Reader Data					*/
													/* ---------------------------- */
	unsigned char	idc1;							/* ID 1							*/
	unsigned char	idc2;							/* ID 2							*/
	unsigned char	rdat[250];						/* Read Data					*/
													/*------------------------------*/
};													/*								*/
													/*------------------------------*/
extern	struct	red_rec	RDT_REC;					/* Reader Read Data Buffer		*/
													/*------------------------------*/
													/*------------------------------*/
struct	rfn_rec	{									/* Reader End Data				*/
													/* ---------------------------- */
	unsigned char	idc1;							/* ID 1							*/
	unsigned char	idc2;							/* ID 2							*/
	unsigned char	ecod;							/* Kind							*/
	unsigned char	ercd;							/* Error Code					*/
	unsigned char	posi[2];						/* Head Position				*/
													/*------------------------------*/
};													/*								*/
													/*------------------------------*/
extern	struct	rfn_rec	RED_REC;					/* Reader End Command Buffer	*/
													/*------------------------------*/
													/*------------------------------*/
struct	rvr_rec	{									/* Module Version Data			*/
													/* ---------------------------- */
	unsigned char	idc1;							/* ID 1							*/
	unsigned char	idc2;							/* ID 2							*/
	unsigned char	kind;							/* Kind							*/
	unsigned char	vers[8];						/* Version No.					*/
													/*------------------------------*/
};													/*								*/
													/*------------------------------*/
extern	struct	rvr_rec	RVD_REC;					/* Reader Version No.			*/
													/*------------------------------*/
													/*------------------------------*/
extern	struct	rvr_rec	ANM_REC;					/* Anounce Machine Version No.	*/
													/*------------------------------*/
													/*								*/
struct	rds_rec	{									/* Dip Switch Data				*/
													/* ---------------------------- */
	unsigned char	result;							/* Result						*/
	unsigned char	state[4];						/* status						*/
													/*------------------------------*/
};													/*								*/
extern	struct	rds_rec	RDS_REC;					/* Dip Switch Data				*/
													/*------------------------------*/
struct	mdl_dat_rec {								/* Module Data Send Buffer		*/
													/*------------------------------*/
	short		mdl_size;							/* Send Data Size				*/
													/*------------------------------*/
	struct	red_rec	mdl_data;						/* Send Data Buffer				*/
													/*------------------------------*/
	short		mdl_endf;							/* ETX/ETB Flag					*/
													/*------------------------------*/
};													/*								*/
													/*								*/
extern	struct	mdl_dat_rec	MDLdata[MTS_MAX];		/*								*/
													/*								*/
													/*								*/
/*----------------------------------------------------------------------------------*/
/* Reader 																			*/
/*----------------------------------------------------------------------------------*/
													/*								*/
													/*								*/
extern char		MAGred[256];						/* Card Read Data Buffer		*/
													/*								*/
													/*								*/
extern char		MDW_buf[96];						/* Card Write Data Buffer		*/
													/*								*/
													/*								*/
extern short	MDW_siz;							/* Size							*/
													/*								*/
													/*								*/
extern char		MDP_buf[128];						/* Card Print&Write Data Buffer	*/
													/*								*/
													/*								*/
extern short	MDP_siz;							/* Size							*/
													/*								*/
													/*								*/
extern unsigned short	MDP_mag;					/* Write Data Size				*/
													/*								*/
													/*								*/
extern short	PAS_okng;							/* Pass Word OK/NG Flag			*/
													/* 0:OK / 1:NG					*/
													/*								*/
extern short	OPE_red;							/* ذ�ް����					*/
													/*								*/
#define		TIK_SYU_MAX		16						/* ����@�ő�l					*/
#define		ID_SYU_MAX		6						/* ID��@�ő�l					*/
#define		MAG_GT_APS_TYPE	0						/* GT/APS�t�H�[�}�b�g�i�[�ʒu	*/
#define		MAG_EX_GT_PKNO	1						/* GT�p�ɒǉ����ꂽP10-P17�i�[�ʒu*/
#define		MAG_ID_CODE		2						/* ���C�f�[�^�擪�Ԓn			*/

extern	const	uchar	tik_id_tbl[TIK_SYU_MAX+1][ID_SYU_MAX];

typedef	struct {									/* ���했�̍ŏI�ǎ挔ID��ۑ���	*/
	uchar	pk_tik;									/* ���Ԍ�						*/
	uchar	teiki;									/* �����						*/
	uchar	pripay;									/* �v���y�C�h�J�[�h				*/
	uchar	kaisuu;									/* �񐔌�						*/
	uchar	svs_tik;								/* �T�[�r�X���i�|�����E�������j	*/
	uchar	kakari;									/* �W���J�[�h					*/
} t_ReadIdSyu;

extern	t_ReadIdSyu	ReadIdSyu;						/* ���했�̍ŏI�ǎ挔ID��		*/
extern	uchar	MRD_VERSION[16];					// ���C���[�_�[�o�[�W����

/*----------------------------------------------------------------------------------*/
/* Anounce Machine																	*/
/*----------------------------------------------------------------------------------*/
struct	an_msg_def {								/* Anounce Machine				*/
	char	cnt;									/*								*/
	char	mod;									/*								*/
	char	msg[10];								/*								*/
};													/*								*/
													/*								*/
extern	struct an_msg_def MSG_def[5];				/*								*/
													/*								*/
													/*								*/
extern	short	MSG_str[9][4];						/*								*/
													/*								*/
													/*								*/
/*----------------------------------------------------------------------------------*/
/* Operation Data & Flag															*/
/*----------------------------------------------------------------------------------*/
													/*								*/
extern	unsigned short 	RD_mod ;					/* Reader Mode					*/
													/*								*/
													/*								*/
extern	unsigned char	RD_pos ;					/* Reader Head Position			*/
													/*								*/
													/*								*/
													/*								*/
													/*								*/
extern	char	Is_AN_reset;						/* AVM��Ճ��Z�b�g(����=1)		*/
													/*								*/
													/*								*/
extern	short	OP_MODUL;							/* Module Next Send Wait		*/
													/*								*/
extern	unsigned char	cr_service_holding;			/* 								*/
													/*								*/
extern	unsigned char AvmBusyFg;					/*								*/
													/*								*/
extern	char	rd_tik;								/* = 0 : ż						*/
													/* = 1 : ���Ԍ��ۗ�				*/
													/* = 2 : �̔��s�����҂�			*/
													/*								*/
extern	char	pas_kep;							/* ����ۗ��׸�					*/
													/*								*/
extern	short	opr_bak;							/* ���M���e�ޯ�����				*/
													/*								*/
extern	short	rd_faz;								/* ذ�ް̪���					*/
													/*								*/
extern	uchar	WaitForTicektRemove;				/* �������҂��׸�				*/
													/*								*/
extern	uchar	RD_SendCommand;						/* ���M����޾��ޗp				*/
													/*								*/
extern	uchar	Mag_LastSendCmd;					/* ���Cذ�ނ֍Ō�ɑ��M���������*/
extern	uchar	RD_PrcWriteFlag;							/* �v���y�C�h�J�[�h�������ݒ��t���O */
extern	uchar	RD_Credit_kep;						/* 1=�ڼޯĶ��ނ�ذ�ް���ɂ���*/
extern	char	avm_alarm_flg;
extern	char pre_volume[2];
extern	ulong		announceFee;					// ���Z�����i�����ǂݏグ���Ɏg�p�j
#define	AN_MSGBUF_SIZE	10
extern	ushort		an_msgbuf[AN_MSGBUF_SIZE];					// �����ǂݏグ�p���b�Z�[�W�ҏW�o�b�t�@
#define	SODIAC_ERR_NONE					0x00			/*	�G���[����				*/
#define	SODIAC_INIT_ERR					0x01			/*	Sodiac�������G���[		*/
#define	SODIAC_AMP_ERR					0x02			/*	AMP�o�^�G���[			*/
#define	SODIAC_EVEHOOK_ERR				0x04			/*	�C�x���g�t�b�N�o�^�G���[*/
#define	SODIAC_NOTSTOP_ERR				0x08			/*	�Đ��I���ʒm�����G���[�@*/

#define	AVM_REQ_BUFF_SIZE				20				/*	AVM�v���o�b�t�@�T�C�Y	*/
#define	AVM_REQ_RETRY_MAX				2				/*  �v�����g���C�ő��	*/

typedef struct _AVM_SODIAC_REQ
{
	st_sodiac_param			req_prm;					/*	sodiac�����Đ��v���p	*/
	unsigned char			resend_count;				/*	�J��Ԃ���			*/
	unsigned char			wait;						/*	�C���^�[�o��			*/
	unsigned char			message_num;				/*	���b�Z�[�W�ԍ�			*/
	unsigned char			dummy;						/*	�_�~�[					*/
}AVM_SODIAC_REQ;

typedef struct _AVM_SODIAC_CTRL
{
	AVM_SODIAC_REQ	sd_req[AVM_REQ_BUFF_SIZE];			/*  sodiac�v��						*/
	unsigned char	write_ptr;							/*	AVM�v���o�b�t�@���C�g�|�C���^	*/
	unsigned char	read_ptr;							/*	AVM�v���o�b�t�@���[�h�|�C���^	*/
	unsigned char	retry_count;						/*  �đ��M�J�E���^					*/
	unsigned char	resend_count_now;					/*	�c��J��Ԃ���				*/
	unsigned char	play_cmp;							/*  �Đ��I���t���O					*/
	unsigned char	message_num;						/*	���b�Z�[�W�ԍ�					*/
	unsigned char	stop;
	unsigned char	play_message_cmp;					/* 1���b�Z�[�W�Đ��I���t���O		*/
}AVM_SODIAC_CTRL;

extern		AVM_SODIAC_CTRL			AVM_Sodiac_Ctrl[2];
extern		st_sodiac_version_info 	Avm_Sodiac_Info;
extern		unsigned char			Avm_Sodiac_Info_end;
extern		unsigned char			Avm_Sodiac_Err_flg;
extern		unsigned char			Sodiac_FROM_Get;

/*----------------------------------------------------------------------------------*/
/* Module Communication Flag														*/
/*----------------------------------------------------------------------------------*/
extern short	mts_req;							/* �ް����M�v��������			*/
													/*								*/
/*----------------------------------------------------------------------------------*/
/* Module Status Buffer																*/
/*----------------------------------------------------------------------------------*/
													/*								*/
extern	unsigned char	RED_stat[4];				/* Reader Status				*/
													/*								*/
extern	unsigned char	w_stat1[3];					/* Reader EOT Status			*/
extern	unsigned char	w_stat2;					/* Reader EOT Status			*/
													/*								*/
/*----------------------------------------------------------------------------------*/
/* Module Status Buffer																*/
/*----------------------------------------------------------------------------------*/
													/*								*/
#define		M_R_INIT		0x49					/* Reader Initial Command		*/
#define		M_R_FONT		0x53					/*        Font					*/
#define		M_R_TEST		0x54					/*        Test					*/
#define		M_R_MOVE		0x4D					/*	  Card Move					*/
#define		M_R_READ		0x52					/*	  Read						*/
#define		M_R_WRIT		0x57					/*	  Write						*/
#define		M_R_PRNT		0x50					/*	  Print						*/
#define		M_R_PRWT		0x58					/*	  Print & Write				*/
#define		M_R_RECV		0x44					/*	  Read Data					*/
#define		M_R_FINE		0x45					/*	  Read Data					*/
#define		M_R_ANSW		0x09					/*	  Read Data					*/
													/*								*/
#define		M_A_MSAG		0x03					/* Anounce Machine Message		*/
#define		M_A_STOP		0x02					/*		   Stop					*/
#define		M_A_DEFN		0x05					/*		   Define				*/
#define		M_A_INIT		0x07					/*		   Initial				*/
#define		M_A_TEST		0x09					/*		   Test					*/
#define		M_A_ANSW		0x09					/*		   Test					*/
													/*								*/
#define		ANN_END			0						// �����I��
/*----------------------------------------------------------------------------------*/
/* OPERED.C 																		*/
/*----------------------------------------------------------------------------------*/
#define		E_VERIFY		0x27					/*								*/
#define		E_START			0x21					/*								*/
#define		E_DATA			0x22					/*								*/
#define		E_PARITY		0x23					/*								*/
#define		E_VERIFY_30		30						/*								*/
/*----------------------------------------------------------------------------------*/
/* OPEANM.C 																		*/
/*----------------------------------------------------------------------------------*/
extern	void	ope_anm	( short );					/*								*/
extern	void	opa_ctl	( ushort );					/*								*/
													/*								*/
#define		AVM_TEST		0						/* ýĺ����4�� �ް�ޮݗv��		*/
#define		AVM_STOP		1						/* ������~						*/
#define		AVM_AUX			2						/* �l�̌��m��					*/
#define		AVM_ICHISEL		3						/* �Ԏ��ԍ��I����				*/
#define		AVM_TURIARI		4						/* �����\����(�ޑK������)		*/
#define		AVM_TURINASI	5						/* �����\����(�ޑK�Ȃ����)		*/
#define		AVM_RYOUSYUU	6						/* ������t���y�ї̎��ؕ���		*/
#define		AVM_KANRYO		7						/* ���Z������					*/
#define		AVM_BTN_NG		8						/* �Ԏ��ԍ��ԈႢ��				*/
#define		AVM_P_INPUT		9						/* �Ïؔԍ����͎�				*/
#define		AVM_P_INPNG		10						/* �Ïؔԍ��ԈႢ��				*/
#define		AVM_P_ENTRY		11						/* �Ïؔԍ��o�^��				*/
#define		AVM_UKE_PRN		12						/* ��t�����s��					*/
#define		AVM_UKE_SUMI	13						/* ��t�����s�ςݎ�				*/
#define		AVM_KANRYO2		14						/* ���Z������(�t���b�v)			*/
enum{
	_pre = AVM_KANRYO2,
	AVM_CREWAIT,			/* 15		�ڼޯĐ��Z�����⍇����ү���� */
	AVM_MAGCARD_ERR,		/* 16		���C�J�[�h�G���[�A�i�E���X */
	AVM_BGM,				/* 17		���p�[�N�a�f�l */
	AVM_HOJIN,				/* 18		�@�l�J�[�h  */
	AVM_FORCE_ROCK,			/* 19		�����o�Ɂi���b�N�j */
	AVM_FORCE_FLAP,			/* 20		�����o�Ɂi�t���b�v�j */
// MH810103 GG119202(S) �ē������̓d�q�}�l�[�Ή�
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//	AVM_CARD_ERR6,			// 21		�J�[�h�̓ǂݎ��Ɏ��s���܂����B
//	AVM_CARD_ERR7,			// 22		�c���s���ł��B
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�	
// MH810103 GG119202(E) �ē������̓d�q�}�l�[�Ή�

	// 27	��
	AVM_RYOUKIN = 28,		// �����ǂݏグ(��)
	AVM_SHASHITU = 29,		// �Ԏ��ǂݏグ(��)
	
	AVM_CREJIT_NG = 32,		// 32		�����A�N���W�b�g�J�[�h�͂���舵���o���܂���B
	AVM_CREJIT_MAXOVER,		// 33		�N���W�b�g�J�[�h�Ő��Z�ł�����x�z�𒴂��Ă��邽�߃N���W�b�g�J�[�h�ɂ�鐸�Z�͏o���܂���B
	AVM_CARD_RETRY,			// 34		�ēx���̕����ɓ���ĉ������B
	AVM_TCARD_NG,			// 35		�����AT�J�[�h�͂���舵���o���܂���B
	AVM_TCARD_OK,			// 36		T�J�[�h���󂯕t���܂����B
	AVM_TCARD_DOUBLE,		// 37		T�J�[�h�͎�t�ς݂ł��B
	AVM_HOJIN_MAXOVER,		// 38		�@�l�J�[�h�Ő��Z�ł�����x�z�𒴂��Ă��邽�ߖ@�l�J�[�h�ɂ�鐸�Z�͏o���܂���B
	AVM_CARD_ERR5,			// 39		�����A����舵�����o���܂���B
	AVM_RYOUSYUU_NG,		// 40		�����A�̎����̔��s���ł��܂���
	AVM_KIGEN_END,			// 41		���̃J�[�h�͊����؂�ł��B
	AVM_CARD_ERR3,			// 42		���̃J�[�h�͖����ł��B
	AVM_CARD_ERR1,			// 43		���̃J�[�h�͎g���܂���B
	AVM_CARD_ERR2,			// 44		���̃J�[�h�͓ǂ߂܂���B
	AVM_CARD_ERR4,			// 45		���̃J�[�h�͂���舵���o���܂���B
	AVM_KIGEN_NEAR_END,		// 46		���̃J�[�h�͂܂��Ȃ������؂�ƂȂ�܂��B
// MH810103 GG119202(S) �ē������̓d�q�}�l�[�Ή�
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//	AVM_ICCARD_ERR1,		// 47		�J�[�h������肭�������B
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
	AVM_ICCARD_ERR1,		// 47		�J�[�h�𔲂��Ă��������B
	AVM_CARD_ERR6,			// 48		�J�[�h�̓ǂݎ��Ɏ��s���܂����B
	AVM_CARD_ERR7,			// 49		�c���s���ł��B
// MH810103 GG119202(E) �ē������̓d�q�}�l�[�Ή�
// MH810103 GG119202(S) ���C���[�_�[�ɃN���W�b�g�J�[�h�}�������Ƃ��̈ē������E�\��
	AVM_CARD_ERR8,			// 50		���σ��[�_�[�Ő��Z���Ă�������
// MH810103 GG119202(E) ���C���[�_�[�ɃN���W�b�g�J�[�h�}�������Ƃ��̈ē������E�\��
// MH810103 GG119202(S) �d�q�}�l�[�u�����h�I�����̈ē�����
	AVM_SELECT_EMONEY,		// 51		�d�q�}�l�[�u�����h�I�����ē�
// MH810103 GG119202(E) �d�q�}�l�[�u�����h�I�����̈ē�����
};

#define		AVM_RYOUKIN_HA		102			// �u�����́v�̃��b�Z�[�W��
#define		AVM_SHASHITU_BAN	105			// �u�Ԏ��ԍ��v�̃��b�Z�[�W��
#define		AVM_SHASHITU_HA		107			// �u�Ԏ��ԍ��́v�̃��b�Z�[�W��
#define		AVM_BANGOU_1		106			// �u�Ԃ̗����́v(��ɗ����ǂݏグ��)
#define		AVM_BANGOU_2		108			// ���q���N,�o���f�X�ƕ�������ꍇ��"�o���f�X"

/* ���G���[�A�i�E���X */
/* �ݒ�Ɏ����Ȃ��A�i�E���X�ԍ�(���ʉ�) */
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		AVM_Edy_OK		120					/* Edy OK �� */
//#define		AVM_Edy_NG		121					/* Edy NG �� */
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		AVM_BOO			122					/* �u�U�[��		  */
#define		AVM_IC_OK		123					/* IC�J�[�h���m�� */
#define		AVM_IC_NG1		124					/* NG�� */
#define		AVM_IC_NG2		125					/* NG�� */
#define		AVM_BGM_MSG		126					/* ���p�[�NBGM */

#define		AVM_AN_TEST		490						// �ē������e�X�g

													/*								*/
/* ����������0.5�b�Ԃ̓R�}���h�𑗐M���Ȃ����ƂƂ���								*/
/* �����ē�������0.5�b�ȓ�															*/
/* �����ē������Ȃ��܂��́A0.5�b�o��												*/
#define		AVM_BUSY		1						/*								*/
#define		AVM_READY		0						/*								*/
#define		AVM_TIMER		5						/*								*/
#define		AVM_VOLMAX		31						/* ����MAX�l 					*/
extern unsigned short avm_test_no[2];
extern char	avm_test_ch;
extern short	avm_test_cnt;

/* ���O�^�C���Ǐグ�p */
#define		AVM_FLAP_BAN	185	//   "�t���b�v�������������Ƃ��m�F��A"
#define		AVM_LOCK_BAN	186	//   "���b�N�������������Ƃ��m�F��A"
#define		AVM_1MINUTE		87	//   "1���ȓ���"
#define		AVM_SYUKKO		109	//   "�o�ɂ��Ă�������"

/* �Ԏ��E�����ǂݏグ�p */
#define		ANN_YEN_DESU	103	//   "�G���f�X"
#define		ANN_JUU			10	//   "�W���E"
#define		ANN_HYAKU		37	//   "�q���N"
#define		ANN_SEN			50	//   "�Z��"
#define		ANN_MAN			67	//   "�}��"

// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�����e�i���X)
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
////#define		ANN_MSG_MAX		254	//   ���b�Z�[�W�ԍ�MAX���ڐ�
//#define		ANN_MSG_MAX		299	//   ���b�Z�[�W�ԍ�MAX���ڐ�
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
#define		ANN_MSG_MAX		999	//   ���b�Z�[�W�ԍ�MAX���ڐ�
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�����e�i���X)

/*----------------------------------------------------------------------------------*/
/* MDLSUB.C 																		*/
/*----------------------------------------------------------------------------------*/
extern	short	rd_mov( short );					/*								*/
extern	short	rd_init( short );					/*								*/
extern	short	rd_read( short );					/*								*/
extern	short	rd_FB7000_MntCommandSend( uchar, uchar, uchar );
extern	short	rd_test( short );					/*								*/
extern	short	rd_font( void );					/*								*/
extern	short	an_init( char );					/*								*/
extern	short	an_test( char );					/*								*/
extern	short	an_boo( void );						/*								*/
extern	short	an_defn( char, char, char * );		/*								*/
extern	void	md_pari( uchar *, ushort, char );	/*								*/
extern	void	md_pari2( uchar *, ushort, char );	/*								*/
extern	char	Is_AVM_connect(void);
extern	void	an_reset(void);
extern	short	an_msag( short *an_msgno, short msg_cnt, short cnt, short wat, char ch);
extern	void 	an_stop(char ch);
extern	short	msg_set( short *an_msgno, short prm );
extern	short	an_boo2(short	seg);
extern	void opa_chime(char onoff);
extern	short an_msag_edit_rxm(short mode);
extern	void	AVM_Sodiac_Init( void );			/*	Sodiac����������			*/
extern	void	AVM_Sodiac_Execute( unsigned short ch );	/*  	*/
extern	void	AVM_Sodiac_Play_Wait_Tim( unsigned short ch );
extern	void	AVM_Sodiac_Err_Chk( D_SODIAC_E err, unsigned char notice_id );
extern	unsigned char	AVM_Sodiac_NextMessageNum( unsigned short ch,	unsigned char ptr);
extern	unsigned char	AVM_Sodiac_EqualReadPtr( unsigned short ch,	unsigned char  ptr, unsigned char message_num );
extern	void	AVM_Sodiac_EqualMsgBuffClear( unsigned short ch,	unsigned char  ptr, unsigned char message_num );
extern  void	AVM_Sodiac_ReadPtrInc( unsigned short ch,	unsigned char ptr);
extern	void	AVM_Sodiac_Play_WaitReq( unsigned char ch,  unsigned char ptr );
extern	void	AVM_Sodiac_TimeOut( void );
extern	void	AVM_Sodiac_SemFree( void );

#define AVM_NOT_CONNECT		0
#define AVM_CONNECT			1

/*------------------------------------------------------------------------------*/
/* CR_SND.C																		*/
/*------------------------------------------------------------------------------*/
extern 	void	cr_snd( void );							/* ذ���ް����M			*/
extern 	void	avm_snd( void );						/* AVM�ް����M			*/
														/*						*/
#define		MTS_BCR1		0x8000						/* �f�[�^���M�v���r�b�g	*/
#define		MTS_BCRWT1		0x0080						/* �}�������ނȂ��҂��v���r�b�g	*/
														/*						*/
#define		MTS_BAVM1		0x0800						/* AVM���M�v���r�b�g	*/
														/*						*/
#define		TICKET_MOUTH	0x10						/* �}����������r�b�g	*/
#define		R_CMD_RCV		0x20						/* ذ�޺���ގ�M�ς݃r�b�g	*/
														/*						*/
/*------------------------------------------------------------------------------*/
/* CR_RCV.C																		*/
/*------------------------------------------------------------------------------*/
void	cr_rcv( void );									/*						*/
void	avm_rcv( void );								/*						*/
char	avm_queset( char);								/*						*/
														/*						*/
extern	char get_timeptn(void);
extern	char get_anavolume(char,char);

#endif	// _MDL_DEF_H_
