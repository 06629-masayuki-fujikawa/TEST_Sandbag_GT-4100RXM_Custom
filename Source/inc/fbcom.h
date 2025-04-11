#ifndef FB_COM_H
#define FB_COM_H
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		FB7000 communications heder file																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	fbcom.h																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      :	S.Takahashi(FSI)																		   |*/
/*| Date        :	201-10-04																				   |*/
/*| Update      :																							   |*/
/*|------------------------------------------------------------------------------------------------------------|*/
/*| �E���C���[�_�ɑ΂���ʐM�֘A�̕ϐ��Að��ْ�`�A�������ߐ錾												   |*/
/*|	�Esystem.h�̌�ɐ錾���邱��																			   |*/
/*[]--------------------------------------------------------------------- Copyright(C) 2011 AMANO Corp.-------[]*/

/*==============================================================================================================*/
/*				��̧�ݒ�`																						*/
/*==============================================================================================================*/
#define		FB_PKT_MAX				270											/* (126 + 9) * 2				*/
#define		FB_PKT_COMMAND			5											/* ��M�ް�����ވʒu			*/
#define		FB_PKT_MIN_SIZ			3											/* �ŏ��߹�Ļ��ށiSYN,ADR,xxx)	*/
#define		FB_ASCII_PKT_HEAD_SIZ	10											/* ASCII�߹��ͯ�ް����			*/
#define		FB_PKT_EOT_SIZ			9											/* EOT�߹�Ļ���					*/
#define		FB_PCK_SEQNO			3

/* ���葤���狭��ACK�����M����Ȃ��ꍇ�Ɏ��瑗�M���I�����邽�߂�	*/			/*								*/
/* ��ײ�񐔂��K�肷��(���è�@�\)									*/			/*								*/
#define		FB_DTSND_RETRY			20											/* ���M�ް��ɑ΂��鉞��NG����	*/
#define		FB_DTSND_REPEAT			3											/* �ʐM�ُ��̌J�Ԃ���		*/
#define		FB_DTRCV_RETRY			3											/* ��M�ް��ɑ΂��鉞���񐔶���	*/
#define		FB_NORSP_CNT			500											/* �������ɑ΂��鶳��			*/

#define		FB_S0					0											/* ƭ����						*/
#define		FB_S1					1											/* �߰�ݸތ�A������҂�			*/
#define		FB_S2					2											/* �ڸèݸތ�A�����҂�			*/
#define		FB_S3					3											/* �ް����M��A�����҂�			*/
#define		FB_S4					4											/* �ڸèݸތ�A�����҂�			*/

#define		FB_REQ_STX				0x02										/* ���亰��(STX)				*/
#define		FB_REQ_EOT				0x04										/* ���亰��(EOT)				*/
#define		FB_REQ_ENQ				0x05										/* ���亰��(ENQ)				*/
#define		FB_REQ_ACK				0x06										/* ���亰��(ACK)				*/
#define		FB_REQ_NAK				0x15										/* ���亰��(NAK)				*/
#define		FB_REQ_SYN				0x16										/* ���亰��(SYN)				*/

#define		FB_BCC_NG				5											/* BCC-NG return code			*/
#define		FB_SERIAL_NG			6											/* �V���A���G���[				*/

#define		FB_RCVBUF_EMPTY			0											/* ��M�ޯ̧��				*/
#define		FB_RCVBUF_FULL			1											/* ��M�ޯ̧FULL				*/

/*--------------*/
/*	���M�ް�ID	*/
/*--------------*/
#define		FB_SND_INIT_ID			0x00										/* �����ݒ��ް�					*/
#define		FB_SND_CTRL_ID			0x01										/* �����ް�						*/
#define		FB_SND_DATA_ID			0x02										/* �d��							*/

/*--------------*/
/*	��M�ް�ID	*/
/*--------------*/
#define		FB_RCV_VERS_ID			0x09										/* �o�[�W�����`�F�b�N�f�[�^		*/
#define		FB_RCV_READ_ID			0x44										/* ���[�h�f�[�^					*/
#define		FB_RCV_END_ID			0x45										/* �I���f�[�^					*/
#define		FB_RCV_SENSOR_ID		0x4c										/* �Z���T�[���x�`�F�b�N			*/
#define		FB_RCV_MNT_ID			0x72										/* �����e�i���X����				*/

// �G���[���e
#define		FB_ERR_NORESPONSE		0x00
#define		FB_ERR_NAKOVER			0x01
#define		FB_ERR_INVALIDDATA		0x02
#define		FB_ERR_DATANORESPONSE	0x03

#define 	FB_SENDDATA_QUEUE_MAX		4

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

#define		FB_SCI_SNDBUF_SIZE		(256)										/*								*/
#define		FB_SCI_RCVBUF_SIZE		(256)										/*								*/

/*==========================================================*/
/*						�\���̒�`							*/
/*==========================================================*/

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/

/** receive buffer control area **/
typedef struct {
    ushort  RcvCnt;           													/* received charcter count		*/
    ushort  ReadIndex;          												/* next read index				*/
    ushort  WriteIndex;         												/* next write index				*/
	ushort	OvfCount;															/* overflow occur count			*/
//	ushort	ComerrCount;														/* error occur count			*/
	ushort	ComerrStatus;														/*								*/
} t_FBSciRcvCtrl;																	/*								*/
																				/*								*/
extern	t_FBSciRcvCtrl	FB_RcvCtrl;												/*								*/


/** send buffer control area **/
typedef struct {
    ushort  SndReqCnt;															/* send request character count	*/
    ushort  SndCmpCnt;															/* send complete character count*/
    ushort  ReadIndex;															/* next send data (read) index	*/
    ushort  SndCmpFlg;															/* send complete flag (1=complete, 0=not yet) */
} t_FBSciSndCtrl;

extern	t_FBSciSndCtrl	FB_SndCtrl;												/*								*/

/*----------------------*/
/* �[�����ɊǗ�����ϐ� */
/*----------------------*/

/***** ���M�ް�����Ǘ�ð��� *****/
typedef struct {																/*								*/
	ushort	R_Index;															/* ذ���߲��					*/
	ushort	W_Index;															/* ײ��߲��						*/
	ushort	Count;																/* ����							*/
} t_FBcomQueCtrl;																/*								*/

typedef struct {
	ushort size;
	uchar buffer[250];
} t_FBsendDataBuffer;

/***** ����Ǘ� *****/
typedef struct {																/*								*/
	/*** ����Ǘ� ***/
	uchar	Matrix;																/* ��ظ�						*/
	uchar	RcvDtSeqNo;															/* �[������M�ް��O���SEQNo.	*/
	uchar	DtCrcNG;															/* �[�����ް�����CRC NG��		*/
	uchar	DtSndNG;															/* ���M�ް��ɑ΂���NAK��ײ����	*/
	uchar	DtRepNG;															/* �ʐM�ُ펞����ײ����			*/
	uchar	DtRcvNG;															/* ��M�ް��ɑ΂���NAK���M����	*/
	ushort	NoAnsCnt;															/* ���������̒[������ߗpPOL����	*/
	uchar	RcvResFlg;															/* ��M�׸�						*/
	uchar	SndDtID;															/* ���M�ް������̈�(ڽ��ݽ�����p)*/
																				/*								*/
	/*** �װ�׸� ***/
	union	{																	/*								*/
		struct	{																/*								*/
			uchar	CtCRC_NG : 1 ;												/* 1=POL/SEL���� CRC error		*/
			uchar	DtCRC_NG : 1 ;												/* 1=�ް�����    CRC error		*/
			uchar	Resp_NG	 : 1 ;												/* 1=ڽ��ݽ����(������)			*/
			uchar	bit_4	 : 1 ;												/* reserve						*/
			uchar	bit_3	 : 1 ;												/* reserve						*/
			uchar	bit_2	 : 1 ;												/* reserve						*/
			uchar	bit_1	 : 1 ;												/* reserve						*/
			uchar	bit_0	 : 1 ;												/* reserve						*/
		} bits;																	/*								*/
		uchar	byte;															/*								*/
	}ErrFlg;																	/*								*/
																				/*								*/
	t_FBcomQueCtrl	CtrlInfo;													/* ����ޯ̧���۰�				*/
	t_FBsendDataBuffer SendData[FB_SENDDATA_QUEUE_MAX];							/* �ް��i�[�ޯ̧				*/
																				/*								*/
	uchar	status[4];															/* �X�e�[�^�X���				*/

} t_FBcomTerm;																	/* 								*/
																				/*								*/
																				/*								*/
/*-----------------------------------*/
/* ��M�����ް���MAIN�֓n���ׂ��ޯ̧ */
/*-----------------------------------*/
#define		FB_RCV_BUF_SIZE		256												/* ��M�ް��i�[�ޯ̧����		*/
																				/*								*/
extern	uchar		FBcom_RcvData[FB_RCV_BUF_SIZE];								/*								*/
																				/*								*/
extern	uchar		FB_RcvBuf[ FB_SCI_RCVBUF_SIZE ];							/*								*/
																				/*								*/
extern	ushort		FBcom_Timer_2_Value;										/* ��ϰ2						*/
extern	ushort		FBcom_Timer_4_Value;										/* ��ϰ4						*/
extern	ushort		FBcom_Timer_5_Value;										/* ��ϰ5						*/
																				/*								*/
extern	uchar		FBcomdr_f_RcvCmp;											/* ������׸�					*/
extern	uchar		FBcomdr_SciErrorState;										/* �V���A���G���[���			*/
extern	ushort		FBcomdr_RcvLength;											/* ��M�ް�����(ܰ�)			*/
extern	uchar		FBcomdr_RcvData[FB_SCI_RCVBUF_SIZE];						/* ��M�ް��ޯ̧(ܰ�)			*/
extern	uchar		FB_SndBuf[FB_SCI_SNDBUF_SIZE];								/* ���M�ޯ̧					*/
																				/*								*/
																				/*								*/
/*==============================================================================================================*/
/*				�������ߐ錾																					*/
/*==============================================================================================================*/
																				/*								*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	mrdcom.c																										*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	void		FBcom_Init( void );											/*								*/
extern	void		FBcom_TimValInit( void );									/*								*/
extern	void		FBcom_Main( void );											/*								*/
extern	ushort		FBcom_SndDtPkt( t_FBcomTerm *, uchar, uchar, uchar );		/*								*/
extern	uchar		FBcom_WaitSciSendCmp( ushort );								/*								*/
extern	ushort		FBcom_SndDtDec( void );										/*								*/
extern	ushort		FBcom_AsciiToBinary(uchar* pAsciiData, ushort asciiSize, uchar* pBinData);
extern	void 		FBcom_SetReceiveData( void );

																				/*								*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	mrdcomTim.c																									*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	void		FBcom_2mTimStart( ushort );									/*								*/
extern	void		FBcom_2mTimStop( void );									/*								*/
extern	uchar		FBcom_2mTimeout( void );									/*								*/
																				/*								*/
extern	void		FBcom_20mTimStart( ushort );								/*								*/
extern	void		FBcom_20mTimStop( void );									/*								*/
extern	uchar		FBcom_20mTimeout( void );									/*								*/
																				/*								*/
extern	void		FBcom_20mTimStart2( ushort );								/*								*/
extern	void		FBcom_20mTimStop2( void );									/*								*/
extern	uchar		FBcom_20mTimeout2( void );									/*								*/
																				/*								*/
																				/*								*/
																				/*								*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	mrdcomdr.c																									*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	void		FBcomdr_Main( void );										/*								*/
extern	void		FBcomdr_RcvInit( void );									/*								*/
																				/*								*/

																				/*								*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	mrdsci.c																										*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	uchar		FBsci_GetChar( uchar *, uchar * );							/*								*/
extern	uchar		FBsci_SndReq(  ushort );									/*								*/
extern	uchar		FBsci_IsSndCmp( void );										/*								*/
extern	void		FBsci_Stop( void );											/*								*/
																				/*								*/
																				/*------------------------------*/
#endif // FB_COM_H
