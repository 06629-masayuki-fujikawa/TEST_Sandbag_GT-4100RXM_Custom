/************************************************************************************************/
/*																								*/
/*	Ӽޭ�ٖ���	:mac.h				:---------------------------------------------------------: */
/*	Ӽޭ�ټ����	:					:��۰��ٕϐ��Q��										  : */
/*	���߲�		:Hitachi H8s,H8/300	:														  : */
/*	�ް�ޮ�		:6.0.0.3			:														  : */
/*	���ޯ�CPU	:H8S/2643F			:---------------------------------------------------------: */
/*	�Ή��@��	:MAC				: 														  : */
/*	�݌v		:���R(05/06/16)		:														  : */
/*	�쐬		:���R(05/06/16)		: 														  : */
/*									:---------------------------------------------------------: */
/*	�ύX����	:���R(05/12/14)		:�E���ڋ@�\�g�p���ʒǉ��y��Cappi�ł̎��т𔽉f			  : */
/*									:---------------------------------------------------------: */
/*																								*/
/************************************************************************************************/
#ifndef	_KSG_MAC_H_
#define	_KSG_MAC_H_

#include	"iodefine.h"


/* ���ޯ��p�錾																					*/
/* --- Note ----------------------------------------------------------------------------------- */
/* ���Ԍv�����s���ׂɁALED�̕\�����g�������ݸނ��o�͂��܂��B									*/
/* 	LED 0 : ��ϊ�����		��ϊ����ݒ��ɓ_��													*/
/* 	LED 1 : ���M������		���M�����ݒ��ɓ_��													*/
/* 	LED 2 : ��M������		��M�����ݒ��ɓ_��													*/
/* 	LED 3 : Ҳ�ٰ��			Ҳ�ٰ�݂̐擪��ĸ�ُo��												*/
/* ------------------------------------------------------------------------------------- End -- */
#define	DBG_LED		0												/* LED�ɂ�����ޯ���L��	*/
#define	DBG_SCI		1												/* �Ď���������� 1�`5		*/

#define _USE_SCR	0												/* ���ޔԍ��̽�����ِؑ�(1:ON 0;OFF)	*/

/* ����M��������																				*/
#define SOH			0x01											/* SOH						*/
#define STX			0x02											/* STX						*/
#define ETX			0x03											/* ETX						*/
#define EOT			0x04											/* EOT						*/
#define ENQ			0x05											/* ENQ						*/
#define ACK			0x06											/* ACK						*/
#define NAK			0x15											/* NAK						*/
#define SYN			0x16											/* SYN						*/

/* �^�錾 ------------------------------------------------------------------------------------- */
																	/* Byte-Bit�Q��				*/
typedef	volatile union {											/*   ���p��					*/
	struct {														/* 	  Bit�Q��				*/
		unsigned char	B7	: 1;									/*     Bit7					*/
		unsigned char	B6	: 1;									/*     Bit6					*/
		unsigned char	B5	: 1;									/*	   Bit5					*/
		unsigned char	B4	: 1;									/*     Bit4					*/
		unsigned char	B3	: 1;									/*     Bit3					*/
		unsigned char	B2	: 1;									/*     Bit2					*/
		unsigned char	B1	: 1;									/*	   Bit1					*/
		unsigned char	B0	: 1;									/*     Bit0					*/
	} BIT;															/*							*/
	unsigned char	BYTE;											/*    Byte�Q��				*/
} BITS;																/*							*/
																	/*							*/
typedef volatile union {											/* ���p��					*/
	struct	work_tag {												/* �\����					*/
		unsigned char	H;											/* Byte���					*/
		unsigned char	L;											/* Byte����					*/
	} BYTE;															/* 							*/
	unsigned short	WORD;											/* Word						*/
} HL;																/*							*/
																	/*							*/
struct	XINPUT {													/* �����߰�					*/
		BITS			REAL;										/* ����h�~��̓����߰�		*/
		BITS			EDGE;										/* �����߰Ă̴��ތ��o		*/
		BITS			MASK;										/* �����߰�Ͻ�				*/
};																	/*							*/
																	/*							*/
struct	XOUTPUT {													/* �o���߰�					*/
		BITS			REAL;										/* �ŏI�o�͌���				*/
		BITS			PULS;										/* ��ٽ�o�͗p				*/
		BITS			RQST;										/* ظ��ėp					*/
		BITS			THRU;										/* �ٰ�p					*/
};																	/*							*/
																	/*							*/
extern	volatile	struct XINPUT	XIP;							/* �����߰Ċ֌W				*/
extern	volatile	struct XOUTPUT	XOP, XMP;						/* �o���߰Ċ֌W				*/
extern	unsigned char	XIO_init_Req;								/* I/O��񏉊���ظ���		*/

/* DIP SW �֘A																					*/

union	DIPSW {														/* ����Ӱ�ސݒ� �� DIPSW	*/
	unsigned char	BYTE;											/* ��Byte����				*/
	struct {														/* ��Bit����				*/
		unsigned char	INIT	:2;									/* b7-8 �������׸�			*/
		unsigned char	TEST	:1;									/* b6 �����p 0:�ʏ�	1:����	*/
		unsigned char			:3;									/* b3-5	 ��				*/
		unsigned char	NODE	:2;									/* b1,b2 ɰ�ޱ��ڽ(224�`227)*/
	} BIT;															/*							*/
};																	/*							*/



																	/*							*/
extern	union			DIPSW	RunMode;							/* ����Ӱ��					*/

/* ROM�ް�ޮ� �֘A																				*/
extern	  signed char	ROMVER[];									/* ROM �ް�ޮݏ��			*/

/* 7SEG / LED �֘A																				*/
union ux7seg {														/* 7SEG ���p��				*/
	struct {														/*   �ޯĒP��				*/
		unsigned char	DOT:	1;									/*     �����_				*/
		unsigned char	SEG:	7;									/* 	   ������				*/
	} BIT;															/* 							*/
	unsigned char		BYTE;										/* �޲ĒP��					*/
};																	/*							*/
																	/*							*/
union	uxled {														/* LED ���p��				*/
	struct	{														/*   �ޯĒP��				*/
		unsigned char	dmy:4;										/*     ��а	(���ڑ�) 		*/
		unsigned char	B3:	1;										/*	   LED 3				*/
		unsigned char	B2:	1;										/*     LED 2				*/
		unsigned char	B1:	1;										/*     LED 1				*/
		unsigned char	B0:	1;										/*     LED 0				*/
	} BIT;															/*							*/
	unsigned char		BYTE;										/*   �޲ĒP��				*/
};																	/*							*/
																	/*							*/
#define z7SEG		(*(volatile union ux7seg 	*)0x400002)			/* 7SEG	Address				*/
#define zLED		(*(volatile union uxled 	*)0x400006)			/* LED	Address				*/
extern	union		ux7seg	x7SEG;									/*							*/
extern	union		uxled	xLED;									/*							*/
extern	union		uxled	fLED;									/* �_�ŗp					*/
extern	void 		digit_7seg( unsigned char , unsigned char );	/* 7SEG DEC��CODE�ϊ�		*/
extern	void		refresh_7seg( void );							/* 7SEG �ĕ`��				*/

/* ��� �֘A																						*/
extern	unsigned long	KSG_Now_t;									/* ���ݎ���(10ms�P��)		*/
extern	unsigned short	mSEC;										/* �ؕb�擾�p				*/
extern	unsigned long	KSG_GetInterval( unsigned long );			/* �o�ߎ��Ԃ̎擾(10ms�P��)	*/
extern	void			Wait_2us( unsigned long );					/* 2us�҂�					*/
																	
#ifdef 	_USE_RTC_																	
/* ر���Ѹۯ��֘A																				*/
extern	unsigned short	temps;										/* A/D�Ǎ��ݒl				*/
																	/*							*/
struct	RTCREG	{													/* �����\����				*/
	unsigned char	SEC;											/* 	�b	 00-59				*/
	unsigned char	MIN;											/*	��	 00-59				*/
	unsigned char	HUR;											/*  ��	 00-23				*/
	unsigned char	WEK;											/*  �j�� 0-6				*/
	unsigned char	DAY;											/*	��	 01-31				*/
	unsigned char	MON;											/*  ��	 01-12				*/
	unsigned char	YER;											/*  �N	 2001-2099 (01-99)	*/
	unsigned char	EDT;											/*	�����׸� 1:�����ݗv��	*/
	unsigned char	ADJ;											/*  �����l					*/
};																	/*							*/
																	/*							*/
union	RTCP1	{													/* ���ݒ�1���p��			*/
	unsigned char BYTE;												/*	Byte�����p				*/
	struct	{														/*  Bit�����p				*/
		unsigned char	WSLE:	1;									/*   �װ�W�@�\				*/
		unsigned char	DALE:	1;									/*   �װ�D�@�\				*/
		unsigned char	H24:	1;									/*   12/24���Ԑ��ؑ�		*/
		unsigned char	CLE2:	1;									/*   FOUT�o�͐���			*/
		unsigned char	TEST:	1;									/*	 �g�p�֎~				*/
		unsigned char	CT2:	1;									/*   /INT�����ݎ����ݒ�		*/
		unsigned char	CT1:	1;									/*   /INT�����ݎ����ݒ�		*/
		unsigned char	CT0:	1;									/*   /INT�����ݎ����ݒ�		*/
	} BIT;															/*							*/
};																	/*							*/
union	RTCP2	{													/* ���ݒ�2���p��			*/
	unsigned char BYTE;												/*	Byte�����p				*/
	struct	{														/*  Bit�����p				*/
		unsigned char	VDSL:	1;									/*   �d���ቺ���o��d��	*/
		unsigned char	VDET:	1;									/*   �d���ቺ���o����		*/
		unsigned char	XST:	1;									/*   ���U��~���o�@�\		*/
		unsigned char	PON:	1;									/*   ��ܰ��ؾ�Č��o			*/
		unsigned char	CLE1:	1;									/*   FOUT�o�͐���			*/
		unsigned char	CTFG:	1;									/*   ������������ݏ��		*/
		unsigned char	WAFG:	1;									/*   �װє���				*/
		unsigned char	DAFG:	1;									/*   �װє���				*/
	} BIT;															/*							*/
};																	/*							*/
																	/*							*/
extern	struct 			RTCREG	RTC;								/* �����ݒ�ϐ�				*/
extern	struct 			RTCREG	RTC_EDT;							/* �����ݒ�ϐ�				*/
extern	const struct 	RTCREG	RTC_INI;							/* �����ݒ�ϐ������l		*/
extern	union 			RTCP1	RTC_R1;								/* ���ݒ�1�ϐ�			*/
extern	union 			RTCP2	RTC_R2;								/* ���ݒ�2�ϐ�			*/
extern	  signed char	AdjPoint;									/* �����߲��				*/
																	/* 							*/
extern	void			RTC_SET_PARAM( void );						/* ���Ұ�����				*/
extern	void			RTC_GET_PARAM( void );						/* ���Ұ��Ǐo				*/
extern	void			RTC_SET_TIME( void );						/* ��������					*/
extern	void			RTC_GET_TIME( unsigned char );				/* �����Ǐo					*/
extern	void			RTC_ASC( signed char *, struct RTCREG *, unsigned char );
																	/* ASCII��					*/
extern	void			RTC_ASC_CAPPI( signed char *, struct RTCREG * );
																	/* ASCII��(for Cappi)		*/

#endif

#ifdef	_USE_ARC_
/* ArcNet�֘A																					*/
extern	void			ARC_Init();									/* 							*/
extern	void			ARC_Main();									/* 							*/
extern	unsigned char	ARC_RcvQueRead( unsigned char *, unsigned char *, unsigned char *, unsigned char *, signed char *, unsigned short * );
																	/* ��M�ޯ̧�Ǐo��			*/
extern	unsigned char	ARC_SndQueSet( unsigned char, unsigned char , unsigned char, signed char *, unsigned short );
																	/* ���M�ޯ̧���o��			*/
extern	unsigned char	ARC_Condition;								/* ���						*/
extern	unsigned char	ARC_NodeID;									/* ɰ��ID					*/
																	/*							*/
#endif

#define	SCI7_RST		PORT4.PODR.BIT.B1							/* ���uNT-NET FOMA(RESET����)	*/
#define	SCI7_RTS		PORT4.PODR.BIT.B3							/* ���uNT-NET FOMA(RTS����)		*/
#define	SCI7_DTR		PORT4.PODR.BIT.B2							/* ���uNT-NET FOMA(DTR����)		*/
#define SCI7_CTS		PORT4.PIDR.BIT.B7							/* ���uNT-NET FOMA(CTS���)		*/
#define SCI7_CD			PORT4.PIDR.BIT.B5							/* ���uNT-NET FOMA(CD ���)		*/
#define SCI7_DSR		PORT4.PIDR.BIT.B6							/* ���uNT-NET FOMA(DSR���)		*/
#define SCI7_CI			PORT4.PIDR.BIT.B4							/* ���uNT-NET FOMA(CI ���)		*/

#ifdef	_USE_SCI1_
/* SCI1�֘A																						*/
extern	void SCI1_Init( unsigned char, unsigned char, unsigned char, unsigned char);
																	/* ������					*/
extern	unsigned char	SCI1_SEND( signed char *, unsigned short );	/* ���M�v��					*/
extern	unsigned char	SCI1_READ( signed char * );					/* ��M�Ǐo					*/
extern	void			SCI1_CLR( void );							/* RING�ޯ̧�����ر			*/
extern	void			SCI1_BUFF_CLR( void );						/* RING�ޯ̧�ر				*/
																	/*							*/
#define	SCI1_RTS		P_PA.DR.BIT.B5								/* RTS����					*/
#define	SCI1_DTR		P_P8.DR.BIT.B0								/* DTR����					*/
#define SCI1_CTS		P_P7.PORT.BIT.B1							/* CTS���					*/
#define SCI1_CD			P_P8.PORT.BIT.B1							/* CD ���					*/
#define SCI1_DSR		P_P8.PORT.BIT.B2							/* DSR���					*/
#define SCI1_CI			P_P8.PORT.BIT.B3							/* CI ���					*/
																	/*							*/
extern	unsigned char	SCI1_SND_CMP;								/* 	���M����				*/
extern	unsigned char	SCI1_RCV_CMP;								/* 	��M����				*/
extern	signed short	SCI1_RCV_TIM;								/* 	��M�Ď����				*/
extern	signed short	SCI1_CHR_TIM;								/* 	�����ԊĎ����			*/
extern	signed short	SCI1_SND_TIM;								/* 	���M�Ď����				*/
extern	signed short	SCI1_RTS_TIM;								/* RTS�������				*/

#endif

#ifdef	_USE_SCI2_
/* SCI2�֘A																						*/
extern	void SCI2_Init( unsigned char, unsigned char, unsigned char, unsigned char);
																	/* ������					*/
extern	unsigned char	SCI2_SEND( char *, unsigned short );		/* ���M�v��					*/
extern	unsigned char	SCI2_READ( unsigned char * );				/* ��M�Ǐo					*/
extern	void			SCI2_CLR( void );							/* RING�ޯ̧�����ر			*/
extern	void			SCI2_BUFF_CLR( void );						/* RING�ޯ̧�ر				*/
extern	void			SCI2_PWR( unsigned char );					/* VCC ON/OFF				*/
																	/*							*/
extern	unsigned char	SCI2_SND_CMP;								/* 	���M����				*/
extern	unsigned char	SCI2_RCV_CMP;								/* 	��M����				*/
extern	unsigned char	SCI2_SND_REQ;								/*  ��M�����׸�		   	*/
extern	signed short	SCI2_RCV_TIM;								/* 	��M�Ď����				*/
extern	signed short	SCI2_CHR_TIM;								/* 	�����ԊĎ����			*/
extern	signed short	SCI2_SND_TIM;								/* 	���M�Ď����				*/
#endif

//#ifdef	_USE_SCI3_
/* SCI3�֘A																						*/
extern	void SCI3_Init( unsigned char, unsigned char, unsigned char, unsigned char);
																	/* ������					*/
extern	unsigned char	SCI3_SEND( char *, unsigned short );		/* ���M�v��					*/
extern	unsigned char	SCI3_READ( unsigned char * );				/* ��M�Ǐo					*/
extern	void			SCI3_CLR( void );							/* RING�ޯ̧�����ر			*/
extern	void			SCI3_BUFF_CLR( void );						/* RING�ޯ̧�ر				*/
																	/*							*/
#define	SCI3_RTS		P_PA.DR.BIT.B7								/* RTS����					*/
#define SCI3_CTS		P_P7.PORT.BIT.B3							/* CTS���					*/
																	/*							*/
extern	unsigned char	SCI3_SND_CMP;								/* 	���M����				*/
extern	unsigned char	SCI3_RCV_CMP;								/* 	��M����				*/
extern	signed short	SCI3_RCV_TIM;								/* 	��M�Ď����				*/
extern	signed short	SCI3_CHR_TIM;								/* 	�����ԊĎ����			*/
extern	signed short	SCI3_SND_TIM;								/* 	���M�Ď����				*/

//#ifdef	_USE_XPT_
/* XPort�֘A																					*/
																	/*							*/
typedef volatile union	{											/* XPort Connect Mode		*/
	unsigned char	BYTE;											/*							*/
	struct {														/*							*/
		unsigned char	IC	: 3;									/* Incoming Connection		*/
		unsigned char	RP	: 1;									/* Response					*/
		unsigned char	UP	: 1;									/* UDP						*/
		unsigned char	AC	: 3;									/* Active Setup				*/
	} BIT;															/*							*/
} XPT_CM;															/*							*/
																	/*							*/
typedef volatile union	{											/* XPort Pack Control		*/
	unsigned char	BYTE;											/*							*/
	struct {														/*							*/
		unsigned char	SC	: 4;									/* Send Charactors			*/
		unsigned char	TC	: 2;									/* Trailing Characters		*/
		unsigned char	IT	: 2;									/* Idle Time				*/
	} BIT;															/*							*/
} XPT_PC;															/*							*/
																	/*							*/
typedef volatile union	{											/* IPv4						*/
	unsigned char	SEG[4];											/*  �����ĕ�				*/
	unsigned long	FULL;											/*  4�޲�					*/
} IPv4;																/*							*/
																	/*							*/
extern	void			XPT_Init();									/* �������֐�				*/
extern	void			XPT_Main();									/* Ҳ݊֐�					*/
extern	unsigned char	XPT_RcvQueRead( unsigned char *, unsigned short * );
																	/* ���M����ǎ�				*/
extern	unsigned char	XPT_SndQueSet ( unsigned char *, unsigned short );
																	/* ���M�������				*/
																	/*							*/
#define XPT_CTS			P_P7.PORT.BIT.B3							/* CP1:CTS					*/
#define XPT_DCD			P_P7.PORT.BIT.B2							/* CP2:DCD					*/
#define	XPT_DTR			P_PA.DR.BIT.B7								/* CP3:DTR					*/
#define	XPT_RST			P_PA.DR.BIT.B6								/* RESET					*/
																	/*							*/
extern	unsigned char	XPT_Condition;								/* ���						*/
																	/*							*/
extern	IPv4			xpt_localIP;								/* ۰��IP					*/
extern	unsigned short	xpt_localPort;								/* ۰���߰�					*/
extern	IPv4			xpt_hostIP;									/* ν�IP					*/
extern	unsigned short	xpt_hostPort;								/* ν��߰�					*/
extern	IPv4			xpt_Gateway;								/* Gateway					*/
extern	XPT_CM			xpt_protocol;								/* 44h:TCP/IP 4Ch:UDP/IP	*/
extern	unsigned char	xpt_sndchr1;								/* ���M�ضޕ���				*/
extern	unsigned char	xpt_sndchr2;								/* ���M�ضޕ���				*/
extern	XPT_PC			xpt_pack;									/* ���M�߯�					*/
extern	unsigned char	xpt_conRetry;								/* �ȸ�����ײ��(0�`99��)	*/
extern	unsigned char	xpt_conWait;								/* �ȸ��ݑ҂�����(0�`99�b)	*/
extern	unsigned char	xpt_conSend;								/* �ȸ��ݑ��M�Ԋu(0�`99�b)	*/
extern	unsigned char	xpt_sndRetry;								/* ���M��ײ��(0�`99��)	*/
extern	unsigned char	xpt_sndWait;								/* ���M�҂�����(0�`99�b)	*/

//#endif

//#endif

#ifdef	_USE_SCI4_
/* SCI4�֘A																						*/
extern	void SCI4_Init( unsigned char, unsigned char, unsigned char, unsigned char);
																	/* ������					*/
extern	unsigned char	SCI4_SEND( signed char *, unsigned short );	/* ���M�v��					*/
extern	unsigned char	SCI4_READ( signed char * );					/* ��M�Ǐo					*/
extern	void			SCI4_CLR( void );							/* RING�ޯ̧�����ر			*/
extern	void			SCI4_BUFF_CLR( void );						/* RING�ޯ̧�ر				*/
extern	void			SCI4_PWR( unsigned char );					/* VCC ON/OFF				*/
																	/* ��M�Ǐo					*/
extern	unsigned char	SCI4_SND_CMP;								/* 	���M����				*/
extern	unsigned char	SCI4_RCV_CMP;								/* 	��M����				*/
extern	signed short	SCI4_RCV_TIM;								/* 	��M�Ď����				*/
extern	signed short	SCI4_CHR_TIM;								/* 	�����ԊĎ����			*/
extern	signed short	SCI4_SND_TIM;								/* 	���M�Ď����				*/
#endif

#ifdef	_USE_NMI_
/* WDT�֘A																						*/
extern	void			wdt_reset( void );							/* �����޸���� ؾ��			*/
#endif

/* ۸ފ֘A																						*/
#define					_PROLOG_USE				0			// �ذ����ɂ� 0 �ɂ���@//
extern	void			LogReset( void );							/* ۸ނ̋���ؾ��			*/
extern	void			LogClear( void );							/* ۸ނ̏�����				*/
extern	void			LogWrite( unsigned char, unsigned char, char * );
																	/* ۸ނ֏����o��			*/
extern	unsigned char	LogRead( unsigned short );					/* ۸ޓǏo��				*/
extern	void			LogInfo( void );							/* ۸ތ����\��				*/
extern	void			ProLog( unsigned char, signed char *, ... );
																	/* �ʐM۸ޑ��M(�����t)		*/
extern	  signed char	LOG_BUFF[];									/* ۸ސ��`��̕�����		*/

/* PC�ݒ�֘A																					*/
extern	void			PC_SET_COMM( unsigned char * );				/* ���ʐݒ菑���ݏ���		*/
extern	void			PC_GET_COMM( unsigned char * );				/* ���ʐݒ�Q�Ə���			*/
extern	void			PC_SET_HOST( unsigned char *, unsigned char );
																	/* HOST�ݒ菑���ݏ���		*/
extern	void			PC_GET_HOST( unsigned char *, unsigned char );
																	/* HOST�ݒ�Q�Ə���			*/

/* ���Ѵװ�֘A																					*/
//extern	unsigned char	SysErr[100];								/* ���Ѵװ�i�[�z��			*/
extern	void			SysErrSet( unsigned char, unsigned char );	/* ���Ѵװ����/�����֐�		*/


typedef struct tagPMCONBITS {
      unsigned RDSP:1;
      unsigned WRSP:1;
      unsigned BEP:1;
      unsigned CS1P:1;
      unsigned :1;
      unsigned ALP:1;
      unsigned CSF:2;
      unsigned CSF1:1;
      unsigned PTRDEN:1;
      unsigned PTWREN:1;
      unsigned PTBEEN:1;
      unsigned ADRMUX:2;
      unsigned PSIDL:1;
      unsigned :1;
      unsigned PMPEN:1;
} PMCONBITS;

typedef struct tagPMMODEBITS {
      unsigned WAITE:2;
      unsigned WAITM:4;
      unsigned WAITB:2;
      unsigned MODE:2;
      unsigned MODE16:1;
      unsigned INCM:2;
      unsigned IRQM:2;
      unsigned BUSY:1;
} PMMODEBITS;

typedef struct tagLATDBITS {
  unsigned LATD0:1;
  unsigned LATD1:1;
  unsigned LATD2:1;
  unsigned LATD3:1;
  unsigned LATD4:1;
  unsigned LATD5:1;
  unsigned LATD6:1;
  unsigned LATD7:1;
  unsigned LATD8:1;
  unsigned LATD9:1;
  unsigned LATD10:1;
  unsigned LATD11:1;
  unsigned LATD12:1;
  unsigned LATD13:1;
  unsigned LATD14:1;
  unsigned LATD15:1;
} LATDBITS;


typedef struct tagTRISDBITS {
  unsigned TRISD0:1;
  unsigned TRISD1:1;
  unsigned TRISD2:1;
  unsigned TRISD3:1;
  unsigned TRISD4:1;
  unsigned TRISD5:1;
  unsigned TRISD6:1;
  unsigned TRISD7:1;
  unsigned TRISD8:1;
  unsigned TRISD9:1;
  unsigned TRISD10:1;
  unsigned TRISD11:1;
  unsigned TRISD12:1;
  unsigned TRISD13:1;
  unsigned TRISD14:1;
  unsigned TRISD15:1;
} TRISDBITS;

typedef struct tagTRISBBITS {
  unsigned TRISB0:1;
  unsigned TRISB1:1;
  unsigned TRISB2:1;
  unsigned TRISB3:1;
  unsigned TRISB4:1;
  unsigned TRISB5:1;
  unsigned TRISB6:1;
  unsigned TRISB7:1;
  unsigned TRISB8:1;
  unsigned TRISB9:1;
  unsigned TRISB10:1;
  unsigned TRISB11:1;
  unsigned TRISB12:1;
  unsigned TRISB13:1;
  unsigned TRISB14:1;
  unsigned TRISB15:1;
} TRISBBITS;

typedef struct tagTRISEBITS {
  unsigned TRISE0:1;
  unsigned TRISE1:1;
  unsigned TRISE2:1;
  unsigned TRISE3:1;
  unsigned TRISE4:1;
  unsigned TRISE5:1;
  unsigned TRISE6:1;
  unsigned TRISE7:1;
  unsigned TRISE8:1;
  unsigned TRISE9:1;
  unsigned TRISE10:1;
  unsigned TRISE11:1;
  unsigned TRISE12:1;
  unsigned TRISE13:1;
  unsigned TRISE14:1;
  unsigned TRISE15:1;
} TRISEBITS;

typedef struct tagLATABITS {
  unsigned LATA0:1;
  unsigned LATA1:1;
  unsigned LATA2:1;
  unsigned LATA3:1;
  unsigned LATA4:1;
  unsigned LATA5:1;
  unsigned LATA6:1;
  unsigned LATA7:1;
  unsigned :1;
  unsigned LATA9:1;
  unsigned LATA10:1;
  unsigned :1;
  unsigned LATA12:1;
  unsigned LATA13:1;
  unsigned LATA14:1;
  unsigned LATA15:1;
} LATABITS;

typedef struct tagLATCBITS {
  unsigned :1;
  unsigned LATC1:1;
  unsigned LATC2:1;
  unsigned LATC3:1;
  unsigned LATC4:1;
  unsigned :7;
  unsigned LATC12:1;
  unsigned LATC13:1;
  unsigned LATC14:1;
  unsigned LATC15:1;
} LATCBITS;

typedef struct tagTRISABITS {
  unsigned TRISA0:1;
  unsigned TRISA1:1;
  unsigned TRISA2:1;
  unsigned TRISA3:1;
  unsigned TRISA4:1;
  unsigned TRISA5:1;
  unsigned TRISA6:1;
  unsigned TRISA7:1;
  unsigned :1;
  unsigned TRISA9:1;
  unsigned TRISA10:1;
  unsigned :1;
  unsigned TRISA12:1;
  unsigned TRISA13:1;
  unsigned TRISA14:1;
  unsigned TRISA15:1;
} TRISABITS;
typedef struct tagTRISCBITS {
  unsigned :1;
  unsigned TRISC1:1;
  unsigned TRISC2:1;
  unsigned TRISC3:1;
  unsigned TRISC4:1;
  unsigned :7;
  unsigned TRISC12:1;
  unsigned TRISC13:1;
  unsigned TRISC14:1;
  unsigned TRISC15:1;
} TRISCBITS;


extern volatile TRISCBITS TRISCbits;
extern volatile TRISABITS TRISAbits;
extern volatile LATABITS LATAbits;
extern volatile TRISBBITS TRISBbits;
extern volatile TRISEBITS TRISEbits;
extern volatile PMCONBITS PMCONbits;		// PMCON register
extern volatile PMMODEBITS PMMODEbits;		// PMMODE register
extern volatile LATDBITS LATDbits;
extern volatile TRISDBITS TRISDbits;
extern volatile unsigned int  PMAEN; 		// PMAEN address enables register

extern volatile unsigned int  PMADDR;		// PMP address increment
extern volatile unsigned int  PMDIN;

	
#define		UM03KO_PWR_PulseTime		11				// UM03-KO PWR SG �o�͎��ԁF1�b(+0.1)
#define		UM03KO_PowerOffTime			25				// UM03-KO CS&DR OFF�҂����ԁF2�b(+0.5)
#define		UM03KO_PowerOnDelay			635				// UM03-KO CS OFF��PWR ON�҂����ԁF63�b(+0.5)
#define		UM03KO_ER_DR_Int			165				// UM03-KO ER ON��DR ON�҂����ԁF16�b(+0.5)
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
#define		UM04KO_PowerOffTime			100				// AD-04S(UM04-KO) CS&DR OFF�҂����ԁF10�b
#define		UM04KO_PowerOnDelay			600				// AD-04S(UM04-KO) CS OFF��PWR ON�҂����ԁF60�b
#define		UM04KO_ER_DR_Int			265				// AD-04S(UM04-KO) (PWR ON)ER ON��DR ON�҂����ԁF26�b(+0.5)
#define		UM04KO_ER_ON				20				// AD-04S(UM04-KO) PWR ON��ER ON�҂����ԁF2�b
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������

#endif
