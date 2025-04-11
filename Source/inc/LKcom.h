/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		IF board communications heder file																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	lkcom.h																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      :																							   |*/
/*| Date        :	2005-03-26																				   |*/
/*| Update      :																							   |*/
/*|------------------------------------------------------------------------------------------------------------|*/
/*| �EIF�Ղɑ΂���ʐM�֘A�̕ϐ��Að��ْ�`�A�������ߐ錾													   |*/
/*|	�Esystem.h�̌�ɐ錾���邱��																			   |*/
/*[]--------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.-------[]*/
#ifndef _LK_COM_H_
#define _LK_COM_H_

/*==============================================================================================================*/
/*				��̧�ݒ�`																						*/
/*==============================================================================================================*/
#define		LK_PKT_MAX				867	



/*--------------*/
/*	���M�ް�ID	*/
/*--------------*/
#define		LK_SND_INIT_ID			0x23										/* �����ݒ��ް�					*/
#define		LK_SND_CTRL_ID			0x21										/* ۯ����u�����ް�				*/
// ARC�߹�đ��M�v��
#define		ARC_SND_RRES_ID			0x52										/* ��M�ް�����					*/
#define		ARC_SND_TEST_ID			0x54										/* ýĺ����						*/

/*--------------*/
/*	��M�ް�ID	*/
/*--------------*/
#define		LK_RCV_REQU_ID			0x22										/* �v���ް�ID					*/
#define		LK_RCV_COND_ID			0x61										/* ����ް�ID					*/
#define		LK_RCV_TEST_ID			0x62										/* ý��ް�ID					*/
#define		LK_RCV_MENT_ID			0x63										/* ����ݽ�ް�ID					*/
#define		LK_RCV_VERS_ID			0x64										/* �ް�ޮ��ް�ID				*/
#define		LK_RCV_ERR_ID			0x65										/* �װ����ް�ID				*/
#define		LK_RCV_ECOD_ID			0x66										/* �װ�ް�ID					*/
#define		IBC_RCV_COND_ID			0x67
#define		IBC_RCV_REQU_ID			0x68
#define		CRR_RCV_TEST_ID			0x69										/* CRR�܂�Ԃ��e�X�g����		*/
#define		IBC_RCV_MENT_ID			0x70										/* ����ݽ�ް�ID					*/
// IBC�ŗL�̂���
// �ȉ��AARC�߹�ē��L�̂���
#define		IBC_RCV_VERS_ID			0x84										/* �ް�ޮ��ް�					*/
#define		IBC_RCV_RSLT_ID			0x80										/* ���M����ID					*/
#define		IBC_RCV_STAT_ID			0x81										/* ������ID					*/
#define		IBC_RCV_FINU_ID			0x82										/* �����ݒ芮��ID				*/
#define		IBC_RCV_TEND_ID			0x83										/* ýČ���ID					*/

#define		LK_RCV_ZAN				960											/* ��M�ޯ̧�󂫻���			*/
#define		LK_LOCK_MAX				50											/* 1�e�@ۯ����uMAX�ڑ��䐔		*/
																				/*								*/
#define		LK_CTRL_DT_SIZE			7											/* ۯ����u�����ް�����			*/
#define		LK_INIT_DT_SIZE			206											/* �����ݒ��ް�����				*/

#define	TERM_NO_MAX				FLAP_IF_MAX	// �ő������No

// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
#define MAX_AUTO_PAYMENT_NO		INT_CAR_LOCK_MAX	//(�����o�^���钓�ւ�20�Ԏ��Ȃ̂łƂ肠����20�Ƃ���) 
#define MAX_AUTO_PAYMENT_CAR	INT_CAR_LOCK_MAX-1	// �ׯ�� 0�`19
#define MAX_AUTO_PAYMENT_BIKE	BIKE_LOCK_MAX-1		// ۯ�   20�`69
enum {
	AUTOPAY_STS_IN = 1,
	AUTOPAY_STS_OUT,
	AUTOPAY_STS_UP,
	AUTOPAY_STS_DOWN,
};

#endif

#define LOCK_CTRL_BUFSIZE_MAX 74												// 9*6 + 20 CRB9��+�t���b�v20��
#define	FLAP_NUM_MAX			BIKE_START_INDEX	// �ő��ׯ��No(100) index��-1
#define	CRR_CTRL_START			INT_CAR_START_INDEX	// �ő�IF�ׯ��No(50) index��-1
#define	LOCK_REQ_ALL_TNO	0xFF					// ���b�N���u�S�Ăɑ΂���v���̌��ʂ�\���^�[�~�i��No(CRB I/F��CRA I/F�ϊ��p)
#define	FLAP_REQ_ALL_TNO	0xFE					// �t���b�v���u�S�Ăɑ΂���v���̌��ʂ�\���^�[�~�i��No(CRB I/F��CRA I/F�ϊ��p)
#define	FLAP_LOCK_TNO_MAX	31						// �t���b�v���b�N���u���킹���^�[�~�i��No�̍ő�l

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

#define		LK_SCI_SNDBUF_SIZE		(1024)										/*								*/
#define		LK_SCI_RCVBUF_SIZE		(1024)										/*								*/


/*----------------------------------*/
/* CRC-CCITT :  x^{16}+x^{12}+x^5+1 */
/*----------------------------------*/
#define		LK_CRCPOLY1				0x1021										/* ���V�t�g						*/
#define		LK_CRCPOLY2				0x8408  									/* �E�V�t�g						*/
#define		LK_CHAR_BIT				8											/* number of bits in a char		*/
#define		LK_L_SHIFT				0											/* ���V�t�g						*/
#define		LK_R_SHIFT				1											/* �E�V�t�g						*/

#define		LKCOM_TYPE_NONE			(uchar)0				// ���u����`
#define		LKCOM_TYPE_LOCK			(uchar)1				// ���b�N���u�i���ցj
#define		LKCOM_TYPE_FLAP			(uchar)2				// �t���b�v���u�i���ԁj


/*==========================================================*/
/*						�\���̒�`							*/
/*==========================================================*/

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/

/*---------------------*/
/***  ��M�ް�ð���  ***/
/*---------------------*/

/***** ۯ����u�ʏ����ް� *****/
/*------------------*/
/* �����ް��͉ϒ� */
/*------------------*/
typedef struct {																/*								*/
	uchar			lock_no;													/* ۯ����uNo.					*/
	uchar			car_cnd;													/* �ԗ����m���					*/
	uchar			lock_cnd;													/* ۯ����u���					*/
} t_LKcomSubLock;																/*								*/
																				/*								*/
typedef struct {																/*								*/
	uchar			did[4];														/* �ް�ID						*/
	uchar			kflg;														/* �ێ��׸�						*/
	uchar			sno;														/* �ް��ǔ�						*/
	uchar			dcnt;														/* �ް���						*/
} t_LKcomLock;																	/*								*/
																				/*								*/
																				/*								*/
/*--- ۯ����u�J��ýėv������ ---*/
typedef struct {																/*								*/
	uchar			did[4];														/* �ް�ID						*/
	uchar			kflg;														/* �ێ��׸�						*/
	uchar			sno;														/* �ް��ǔ�						*/
	uchar			lock_tst[90];												/* ۯ����u��ýČ���				*/
} t_LKcomTest;																	/*								*/
																				/*								*/
																				/*								*/
/***** ����ݽ���v������ *****/
typedef struct {																/*								*/
	uchar			auto_cnt[4];												/* ۯ����u���춳��				*/
	uchar			manu_cnt[4];												/* ۯ����u�蓮���춳��			*/
	uchar			trbl_cnt[4];												/* ۯ����u�̏��				*/
} t_LKcomSubMnt;																/*								*/
																				/*								*/
typedef struct {																/*								*/
	t_LKcomSubMnt	lock_mnt[72];												/* ۯ����u�����춳��			*/
} t_LKcomMnt;																	/*								*/

/***** ����ݽ���v������ *****/
typedef struct {																/*								*/
	uchar			auto_cnt[6];												/* �ׯ�ߑ��u���춳��			*/
	uchar			manu_cnt[4];												/* �ׯ�ߑ��u�蓮���춳��		*/
	uchar			trbl_cnt[4];												/* �ׯ�ߑ��u�̏��			*/
} t_FLcomSubMnt;																/*								*/
																				/*								*/
typedef struct {																/*								*/
	uchar			did[4];														/* �ް�ID						*/
	uchar			kflg;														/* �ێ��׸�						*/
	uchar			sno;														/* �ް��ǔ�						*/
	t_FLcomSubMnt	flap_mnt[72];												/* ۯ����u�����춳��			*/
} t_FLcomMnt;																	/*								*/
																				/*								*/
/***** �ް�ޮݗv������ *****/
typedef struct {																/*								*/
	uchar			s_ver[15][8];												/* IF�Վq�@�ް�ޮ�				*/
} t_LKcomVer;																	/*								*/
																				/*								*/
/***** �װ�v������ *****/
typedef struct {																/*								*/
	uchar			did[4];														/* �ް�ID						*/
	uchar			kflg;														/* �ێ��׸�						*/
	uchar			sno;														/*								*/
	ushort			m_err;														/* IF�Րe�@�װ					*/
	uchar			s_err[15];													/* IF�Վq�@�װ					*/
} t_LKcomErr;																	/*								*/

/***** IF�՗v���ް�	*****/
typedef struct {																/*								*/
	uchar			did[4];														/* �ް�ID						*/
	uchar			kflg;														/* �ێ��׸�						*/
	uchar			sno;														/* �ް��ǔ�						*/
	uchar			req;														/* �v��							*/
} t_LKcomReq;																	/*								*/
																				/*								*/
/***** �װ�ް�	*****/
typedef struct {																/*								*/
	uchar			did;														/* �ް�ID						*/
	uchar			kflg;														/* �ێ��׸�						*/
	uchar			sno;														/* �ް��ǔ�						*/
	uchar			tno;														/* �[�����(0=�e�@/1�`15:�q�@)	*/
	uchar			err;														/* �װNo.						*/
	uchar			occr;														/* ����/����/������������		*/
	uchar			dmy[2];														/* �\��							*/
} t_LKcomEcod;																	/*								*/
																				/*								*/
/***** ��M�ް�ͯ��	*****/
typedef struct {
	uchar			did[4];														/* �ް�ID						*/
	uchar			kflg;														/* �ێ��׸�						*/
	uchar			sno;														/* �ް��ǔ�						*/
} T_LKcomRcvHdrIbc;

/***** ���M�����ް�	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ͯ��							*/
	uchar				ssno;													/* ���M�ް��ǔ�					*/
	uchar				tno;													/* �[�����(0=�e�@/1�`15:�q�@)	*/
	uchar				rslt;													/* ����							*/
	uchar				dmy;													/* �\��							*/
} t_LKcomRsltIbc;																/*								*/

/***** �������ް�	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ͯ��							*/
	uchar				tsts[20];												/* �[�����(1�`20)				*/
} t_LKcomStatIbc;																/*								*/

/***** ýČ����ް�	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ͯ��							*/
	uchar				rslt;													/* ����							*/
	uchar				dmy;													/* �\��							*/
} t_LKcomTEndIbc;																/*								*/

/***** �����ݒ芮���ް�	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ͯ��							*/
	uchar				dmy[2];													/* �\��							*/
} t_LKcomFinuIbc;																/*								*/

/***** �ް�ޮ��ް�	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ͯ��							*/
	uchar				ver[10];												/* ��۸����ް�ޮ�				*/
	uchar				psum[2];												/* ��۸���������				*/
	uchar				mchk[4];												/* ���R/W��������				*/
	uchar				dmy[2];													/* �\��							*/
} t_LKcomVerIbc;																/*								*/

/*------------------------*/
/* �[������ŊǗ�����ϐ� */
/*------------------------*/
typedef	struct {																/*								*/
																				/*								*/
	uchar	PktSeqNo;															/* �߹�ļ��ݼ��No.				*/
	uchar	TnoNow;																/* ���݂̱����[���ԍ�			*/
	uchar	TnoMax;																/* �[���ڑ��䐔					*/
	uchar	MentFlg;															/* ����ݽ���׸�					*/
} t_LKcomLineCtrl;																/*								*/


/*----------------------*/
/* �[�����ɊǗ�����ϐ� */
/*----------------------*/

/***** ���M�ް�����Ǘ�ð��� *****/
typedef struct {																/*								*/
	ushort	R_Index;															/* ذ���߲��					*/
	ushort	W_Index;															/* ײ��߲��						*/
	ushort	Count;																/* ����							*/
} t_LKcomQueCtrl;																/*								*/


/***** �����ݒ��ް� *****/
typedef struct {																/*								*/
	uchar	did4;																/* �ް�ID4						*/
	uchar	kflg;																/* �ێ��׸�						*/
	uchar	sno;																/* ���ݼ��No.					*/
	uchar	lock[90];															/* ۯ����u�ڑ�					*/
	uchar	i_tm[15];															/* ���Ɏԗ����m��ϰ				*/
	uchar	o_tm[15];															/* �o�Ɏԗ����m��ϰ				*/
	uchar	r_cn[15];															/* ��ײ��						*/
	ushort	r_tm[15];															/* ��ײ�Ԋu						*/
	ushort	mst_tm1;															/* ��M�ް��ő厞��				*/
	ushort	mst_tm2;															/* ÷�đ��M��̉����҂�����		*/
	ushort	mst_tm3;															/* ���M�ް�Wait��ϰ				*/
	uchar	mst_ret1;															/* �ް����M��ײ��				*/
	uchar	mst_ret2;															/* NAK���M��ײ��				*/
	ushort	dummy1;																/* �\��							*/
	ushort	slv_tm1;															/* ��M�Ď���ϰ					*/
	ushort	slv_tm2;															/* POL�Ԋu						*/
	uchar	slv_ret1;															/* ��ײ��						*/
	uchar	slv_ret2;															/* �������װ�����			*/
	ushort	dummy2;																/* 								*/
	uchar	oc_tm[5][2];														/* open/close����				*/
	uchar	dummy3[10];															/*								*/
} t_LKcomInit;																	/*								*/

/***** ۯ����u�����ް� *****/
typedef struct {																/*								*/
	uchar	did4;																/* �ް�ID4						*/
	uchar	tno;																// CRB�C���^�[�t�F�[�X�̃^�[�~�i��No
	uchar	kind;																/* �����敪						*/
	uchar	lock;																/* ۯ����uNo.					*/
	uchar	clos_tm;															/* ����M���o�͎���			*/
	uchar	open_tm;															/* �J����M���o�͎���			*/
} t_LKcomCtrl;																	/*								*/


/***** ����Ǘ� *****/
typedef struct {																/*								*/
	/*** ����Ǘ� ***/
																				/*								*/
	/*** ۯ����u���� ***/
	t_LKcomQueCtrl	CtrlInfo;													/* ����ޯ̧���۰�				*/
	t_LKcomCtrl		CtrlData[LOCK_CTRL_BUFSIZE_MAX];							// ۯ����u�����ް��i�[�ޯ̧:74�� 9*6+20= CRB9��+�t���b�v20��
																				/*								*/
	// �ŏI�I�ɂ�1���ɂ���
																				/*								*/

} t_LKcomTerm;																	/* 								*/
																				/*								*/
extern	t_LKcomTerm		LKcomTerm;
/***** ���u��� *****/
extern	uchar			LKcom_Type[LOCK_IF_MAX];								// �[�����̑��u��ʁi���ԁE���ցj
																			/*								*/
/*-----------------------------------*/
/* ��M�����ް���MAIN�֓n���ׂ��ޯ̧ */
/*-----------------------------------*/
#define		LK_RCV_BUF_SIZE		20000											/* ��M�ް��i�[�ޯ̧����		*/
																				/*								*/
typedef	struct {																/*								*/
	uchar		dcnt;															/* �ް��i�[����					*/
	ushort		all_siz;														/* ���ް��i�[�޲Đ�				*/
	ushort		zan_siz;														/* �󂫴ر�޲Đ�				*/
	uchar		data[LK_RCV_BUF_SIZE];											/* �ް��i�[�ޯ̧				*/
																				/*								*/
} t_LK_RCV_INFO;																/*								*/
																				/*								*/
extern	t_LK_RCV_INFO		LKcom_RcvData;										/*								*/
																				/*								*/
extern	uchar		LKcom_InitFlg;												/* ۯ����u�ʐM�������׸�		*/
extern	uchar		LKcom_f_TaskStop;											/* ۯ����u�ʐM�����~�׸�		*/
																				/*								*/
extern	uchar		LKcom_RcvDtWork[LK_SCI_RCVBUF_SIZE];						/* ��M�ް���͗pܰ�			*/
																				/*								*/
																				/*								*/
extern	uchar		LK_Init;													/*								*/
extern	uchar		LKcom_f_SndReq;												/* POL/SEL���Mظ���				*/
extern				t_LKcomLineCtrl		LKcomLineCtrl;							/* ������۰�					*/
																				/*								*/
extern	uchar		LKcomdr_RcvData[LK_SCI_RCVBUF_SIZE];						/* ��M�ް��ޯ̧(ܰ�)			*/
extern	uchar		LK_SndBuf[LK_SCI_SNDBUF_SIZE];								/* ���M�ޯ̧					*/
																				/*								*/
extern	uchar		MntLockTest[LK_LOCK_MAX];									/*								*/
extern	ulong		MntLockDoCount[BIKE_LOCK_MAX][3];							/* LOCK���u���춳��				*/
extern	ulong		MntFlapDoCount[TOTAL_CAR_LOCK_MAX][3];						/* �ׯ��(�����{�O�t��)���u���춳��	*/
extern	ushort		MntLockWkTbl[LOCK_IF_REN_MAX];								/* ۯ����u���춳�Ď�M�p�ϊ�ð���	*/
extern	uchar		MntFlapTest[CAR_LOCK_MAX];
																				/*								*/
typedef struct {																/*								*/
	ushort			LockNoBuf;													/* �Ԏ���(1�`324)-1				*/
	uchar			car_cnd;													/* �ԗ����m���					*/
	uchar			lock_cnd;													/* ۯ����u���					*/
	uchar			dummy[2];													/* �\��1						*/
} t_LKBUF;																		/*								*/
extern	t_LKBUF		LockBuf[LK_LOCK_MAX];										/*								*/
extern	uchar		child_mk[LOCK_IF_MAX];												// �q�@��ۯ����uҰ���ҏW
																				/*								*/
extern	uchar	flp_m_mode[FLAP_IF_MAX];										/* �ׯ�ߑ��u�蓮Ӱ���׸�		*/
																				/*  ON = �蓮Ӱ�ޒ�				*/
extern	uchar	lok_m_mode[LOCK_IF_MAX];										/* ۯ����u�蓮Ӱ���׸�			*/
																				/*  ON = �蓮Ӱ�ޒ�				*/
																				/*								*/
																				/*								*/
/*==============================================================================================================*/
/*				�������ߐ錾																					*/
/*==============================================================================================================*/
																				/*								*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	LKcom.c																										*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	void		LKcom_Init( void );											/*								*/
extern	ushort		LKcom_SndDtDec( void );										/*								*/
extern	uchar		LKcom_RcvDataSave(uchar *data, ushort len );
extern	uchar		LKcom_RcvDataDel( void );									/*								*/
extern	void		LKcom_PassBreak( void );									/*								*/
extern	void		LKcom_RcvBuffReset( void );									/* ��M�ް��i�[�ޯ̧ؾ��		*/
																				/*								*/
																				/*								*/
																				/*								*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	LKcomApi.c																									*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	char		LKcom_SetDtPkt( uchar, uchar, ushort, uchar );				/*								*/
extern	uchar		LKcom_RcvDataGet( ushort * );								/*								*/
extern	uchar		LKcom_RcvDataAnalys( ushort );								/*								*/
extern	void		LKcom_SetLockMaker( void );									/*								*/
extern	short		LKopeGetLockNum( uchar, ushort, ushort * );					/*								*/
extern	void		LKopeApiLKCtrl( ushort, uchar );							/* ۯ����u�w��					*/
extern	void		LKopeApiLKCtrl_All(uchar, uchar);							/*								*/

extern	void		LKopeErrRegist( uchar, t_LKcomEcod* );						/*								*/
extern	short		LKopeLockErrCheck( short, uchar );							/*								*/
extern	void		ErrBinDatSet( ulong, short );								/*								*/
extern	void		lk_err_chk( ushort, char, char );							/*								*/
extern	void	LKope_ClearAllTermVersion(void);
extern	char	LKcom_Search_Ifno( uchar );
extern	void	LKcom_AllInfoReq( uchar );
extern	uchar	get_lktype(uchar lok_syu);
extern	void	LKcom_InitAccessTarminalType(void);

// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
extern	void LK_AutoPayment_Rcv(ushort index, uchar tno, uchar lkno, uchar kind);
#endif
extern 	ushort	LKcom_RoomNoToType( ulong roomNo );
extern	ushort	LKcom_GetAccessTarminalCount(void);
extern 	uchar	LKcom_GetAccessTarminalType(ushort tno);
// MH322914(S) K.Onodera 2016/09/07 AI-V�Ή��F�G���[�A���[��
extern	int		LKcom_RoomNoToIndex( ulong roomNo );
// MH322914(E) K.Onodera 2016/09/07 AI-V�Ή��F�G���[�A���[��

///*--------------------------------------------------------------------------------------------------------------*/
///*	LKsci.c																										*/
///*--------------------------------------------------------------------------------------------------------------*/

#endif	// _LK_COM_H_
