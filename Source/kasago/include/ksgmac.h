/************************************************************************************************/
/*																								*/
/*	ﾓｼﾞｭｰﾙ名称	:mac.h				:---------------------------------------------------------: */
/*	ﾓｼﾞｭｰﾙｼﾝﾎﾞﾙ	:					:ｸﾞﾛｰﾊﾞﾙ変数参照										  : */
/*	ｺﾝﾊﾟｲﾗ		:Hitachi H8s,H8/300	:														  : */
/*	ﾊﾞｰｼﾞｮﾝ		:6.0.0.3			:														  : */
/*	ﾀｰｹﾞｯﾄCPU	:H8S/2643F			:---------------------------------------------------------: */
/*	対応機種	:MAC				: 														  : */
/*	設計		:平山(05/06/16)		:														  : */
/*	作成		:平山(05/06/16)		: 														  : */
/*									:---------------------------------------------------------: */
/*	変更履歴	:平山(05/12/14)		:・搭載機能使用判別追加及びCappiでの実績を反映			  : */
/*									:---------------------------------------------------------: */
/*																								*/
/************************************************************************************************/
#ifndef	_KSG_MAC_H_
#define	_KSG_MAC_H_

#include	"iodefine.h"


/* ﾃﾞﾊﾞｯｸ用宣言																					*/
/* --- Note ----------------------------------------------------------------------------------- */
/* 時間計測を行う為に、LEDの表示を使ってﾀｲﾐﾝｸﾞを出力します。									*/
/* 	LED 0 : ﾀｲﾏ割込み		ﾀｲﾏ割込み中に点灯													*/
/* 	LED 1 : 送信割込み		送信割込み中に点灯													*/
/* 	LED 2 : 受信割込み		受信割込み中に点灯													*/
/* 	LED 3 : ﾒｲﾝﾙｰﾌﾟ			ﾒｲﾝﾙｰﾁﾝの先頭でﾄｸﾞﾙ出力												*/
/* ------------------------------------------------------------------------------------- End -- */
#define	DBG_LED		0												/* LEDによるﾃﾞﾊﾞｯｸを有効	*/
#define	DBG_SCI		1												/* 監視するﾁｬﾝﾈﾙ 1～5		*/

#define _USE_SCR	0												/* ｶｰﾄﾞ番号のｽｸﾗﾝﾌﾞﾙ切替(1:ON 0;OFF)	*/

/* 送受信文字ｺｰﾄﾞ																				*/
#define SOH			0x01											/* SOH						*/
#define STX			0x02											/* STX						*/
#define ETX			0x03											/* ETX						*/
#define EOT			0x04											/* EOT						*/
#define ENQ			0x05											/* ENQ						*/
#define ACK			0x06											/* ACK						*/
#define NAK			0x15											/* NAK						*/
#define SYN			0x16											/* SYN						*/

/* 型宣言 ------------------------------------------------------------------------------------- */
																	/* Byte-Bit参照				*/
typedef	volatile union {											/*   共用体					*/
	struct {														/* 	  Bit参照				*/
		unsigned char	B7	: 1;									/*     Bit7					*/
		unsigned char	B6	: 1;									/*     Bit6					*/
		unsigned char	B5	: 1;									/*	   Bit5					*/
		unsigned char	B4	: 1;									/*     Bit4					*/
		unsigned char	B3	: 1;									/*     Bit3					*/
		unsigned char	B2	: 1;									/*     Bit2					*/
		unsigned char	B1	: 1;									/*	   Bit1					*/
		unsigned char	B0	: 1;									/*     Bit0					*/
	} BIT;															/*							*/
	unsigned char	BYTE;											/*    Byte参照				*/
} BITS;																/*							*/
																	/*							*/
typedef volatile union {											/* 共用体					*/
	struct	work_tag {												/* 構造体					*/
		unsigned char	H;											/* Byte上位					*/
		unsigned char	L;											/* Byte下位					*/
	} BYTE;															/* 							*/
	unsigned short	WORD;											/* Word						*/
} HL;																/*							*/
																	/*							*/
struct	XINPUT {													/* 入力ﾎﾟｰﾄ					*/
		BITS			REAL;										/* ﾁｬﾀ防止後の入力ﾎﾟｰﾄ		*/
		BITS			EDGE;										/* 入力ﾎﾟｰﾄのｴｯｼﾞ検出		*/
		BITS			MASK;										/* 入力ﾎﾟｰﾄﾏｽｸ				*/
};																	/*							*/
																	/*							*/
struct	XOUTPUT {													/* 出力ﾎﾟｰﾄ					*/
		BITS			REAL;										/* 最終出力結果				*/
		BITS			PULS;										/* ﾊﾟﾙｽ出力用				*/
		BITS			RQST;										/* ﾘｸｴｽﾄ用					*/
		BITS			THRU;										/* ｽﾙｰ用					*/
};																	/*							*/
																	/*							*/
extern	volatile	struct XINPUT	XIP;							/* 入力ﾎﾟｰﾄ関係				*/
extern	volatile	struct XOUTPUT	XOP, XMP;						/* 出力ﾎﾟｰﾄ関係				*/
extern	unsigned char	XIO_init_Req;								/* I/O情報初期化ﾘｸｴｽﾄ		*/

/* DIP SW 関連																					*/

union	DIPSW {														/* 動作ﾓｰﾄﾞ設定 ⇒ DIPSW	*/
	unsigned char	BYTE;											/* ⇒Byteｱｸｾｽ				*/
	struct {														/* ⇒Bitｱｸｾｽ				*/
		unsigned char	INIT	:2;									/* b7-8 初期化ﾌﾗｸﾞ			*/
		unsigned char	TEST	:1;									/* b6 検査用 0:通常	1:検査	*/
		unsigned char			:3;									/* b3-5	 空き				*/
		unsigned char	NODE	:2;									/* b1,b2 ﾉｰﾄﾞｱﾄﾞﾚｽ(224～227)*/
	} BIT;															/*							*/
};																	/*							*/



																	/*							*/
extern	union			DIPSW	RunMode;							/* 動作ﾓｰﾄﾞ					*/

/* ROMﾊﾞｰｼﾞｮﾝ 関連																				*/
extern	  signed char	ROMVER[];									/* ROM ﾊﾞｰｼﾞｮﾝ情報			*/

/* 7SEG / LED 関連																				*/
union ux7seg {														/* 7SEG 共用体				*/
	struct {														/*   ﾋﾞｯﾄ単位				*/
		unsigned char	DOT:	1;									/*     小数点				*/
		unsigned char	SEG:	7;									/* 	   ｾｸﾞﾒﾝﾄ				*/
	} BIT;															/* 							*/
	unsigned char		BYTE;										/* ﾊﾞｲﾄ単位					*/
};																	/*							*/
																	/*							*/
union	uxled {														/* LED 共用体				*/
	struct	{														/*   ﾋﾞｯﾄ単位				*/
		unsigned char	dmy:4;										/*     ﾀﾞﾐｰ	(未接続) 		*/
		unsigned char	B3:	1;										/*	   LED 3				*/
		unsigned char	B2:	1;										/*     LED 2				*/
		unsigned char	B1:	1;										/*     LED 1				*/
		unsigned char	B0:	1;										/*     LED 0				*/
	} BIT;															/*							*/
	unsigned char		BYTE;										/*   ﾊﾞｲﾄ単位				*/
};																	/*							*/
																	/*							*/
#define z7SEG		(*(volatile union ux7seg 	*)0x400002)			/* 7SEG	Address				*/
#define zLED		(*(volatile union uxled 	*)0x400006)			/* LED	Address				*/
extern	union		ux7seg	x7SEG;									/*							*/
extern	union		uxled	xLED;									/*							*/
extern	union		uxled	fLED;									/* 点滅用					*/
extern	void 		digit_7seg( unsigned char , unsigned char );	/* 7SEG DEC⇒CODE変換		*/
extern	void		refresh_7seg( void );							/* 7SEG 再描画				*/

/* ﾀｲﾏ 関連																						*/
extern	unsigned long	KSG_Now_t;									/* 現在時刻(10ms単位)		*/
extern	unsigned short	mSEC;										/* ﾐﾘ秒取得用				*/
extern	unsigned long	KSG_GetInterval( unsigned long );			/* 経過時間の取得(10ms単位)	*/
extern	void			Wait_2us( unsigned long );					/* 2us待ち					*/
																	
#ifdef 	_USE_RTC_																	
/* ﾘｱﾙﾀｲﾑｸﾛｯｸ関連																				*/
extern	unsigned short	temps;										/* A/D読込み値				*/
																	/*							*/
struct	RTCREG	{													/* 日時構造体				*/
	unsigned char	SEC;											/* 	秒	 00-59				*/
	unsigned char	MIN;											/*	分	 00-59				*/
	unsigned char	HUR;											/*  時	 00-23				*/
	unsigned char	WEK;											/*  曜日 0-6				*/
	unsigned char	DAY;											/*	日	 01-31				*/
	unsigned char	MON;											/*  月	 01-12				*/
	unsigned char	YER;											/*  年	 2001-2099 (01-99)	*/
	unsigned char	EDT;											/*	調整ﾌﾗｸﾞ 1:書込み要求	*/
	unsigned char	ADJ;											/*  調整値					*/
};																	/*							*/
																	/*							*/
union	RTCP1	{													/* 環境設定1共用体			*/
	unsigned char BYTE;												/*	Byteｱｸｾｽ用				*/
	struct	{														/*  Bitｱｸｾｽ用				*/
		unsigned char	WSLE:	1;									/*   ｱﾗｰﾑW機能				*/
		unsigned char	DALE:	1;									/*   ｱﾗｰﾑD機能				*/
		unsigned char	H24:	1;									/*   12/24時間制切替		*/
		unsigned char	CLE2:	1;									/*   FOUT出力制御			*/
		unsigned char	TEST:	1;									/*	 使用禁止				*/
		unsigned char	CT2:	1;									/*   /INT割込み周期設定		*/
		unsigned char	CT1:	1;									/*   /INT割込み周期設定		*/
		unsigned char	CT0:	1;									/*   /INT割込み周期設定		*/
	} BIT;															/*							*/
};																	/*							*/
union	RTCP2	{													/* 環境設定2共用体			*/
	unsigned char BYTE;												/*	Byteｱｸｾｽ用				*/
	struct	{														/*  Bitｱｸｾｽ用				*/
		unsigned char	VDSL:	1;									/*   電圧低下検出基準電圧	*/
		unsigned char	VDET:	1;									/*   電圧低下検出結果		*/
		unsigned char	XST:	1;									/*   発振停止検出機能		*/
		unsigned char	PON:	1;									/*   ﾊﾟﾜｰｵﾝﾘｾｯﾄ検出			*/
		unsigned char	CLE1:	1;									/*   FOUT出力制御			*/
		unsigned char	CTFG:	1;									/*   定期周期割込み状態		*/
		unsigned char	WAFG:	1;									/*   ｱﾗｰﾑ発生				*/
		unsigned char	DAFG:	1;									/*   ｱﾗｰﾑ発生				*/
	} BIT;															/*							*/
};																	/*							*/
																	/*							*/
extern	struct 			RTCREG	RTC;								/* 日時設定変数				*/
extern	struct 			RTCREG	RTC_EDT;							/* 日時設定変数				*/
extern	const struct 	RTCREG	RTC_INI;							/* 日時設定変数初期値		*/
extern	union 			RTCP1	RTC_R1;								/* 環境設定1変数			*/
extern	union 			RTCP2	RTC_R2;								/* 環境設定2変数			*/
extern	  signed char	AdjPoint;									/* 調整ﾎﾟｲﾝﾄ				*/
																	/* 							*/
extern	void			RTC_SET_PARAM( void );						/* ﾊﾟﾗﾒｰﾀ書込				*/
extern	void			RTC_GET_PARAM( void );						/* ﾊﾟﾗﾒｰﾀ読出				*/
extern	void			RTC_SET_TIME( void );						/* 日時書込					*/
extern	void			RTC_GET_TIME( unsigned char );				/* 日時読出					*/
extern	void			RTC_ASC( signed char *, struct RTCREG *, unsigned char );
																	/* ASCII化					*/
extern	void			RTC_ASC_CAPPI( signed char *, struct RTCREG * );
																	/* ASCII化(for Cappi)		*/

#endif

#ifdef	_USE_ARC_
/* ArcNet関連																					*/
extern	void			ARC_Init();									/* 							*/
extern	void			ARC_Main();									/* 							*/
extern	unsigned char	ARC_RcvQueRead( unsigned char *, unsigned char *, unsigned char *, unsigned char *, signed char *, unsigned short * );
																	/* 受信ﾊﾞｯﾌｧ読出し			*/
extern	unsigned char	ARC_SndQueSet( unsigned char, unsigned char , unsigned char, signed char *, unsigned short );
																	/* 送信ﾊﾞｯﾌｧ書出し			*/
extern	unsigned char	ARC_Condition;								/* 状態						*/
extern	unsigned char	ARC_NodeID;									/* ﾉｰﾄﾞID					*/
																	/*							*/
#endif

#define	SCI7_RST		PORT4.PODR.BIT.B1							/* 遠隔NT-NET FOMA(RESET制御)	*/
#define	SCI7_RTS		PORT4.PODR.BIT.B3							/* 遠隔NT-NET FOMA(RTS制御)		*/
#define	SCI7_DTR		PORT4.PODR.BIT.B2							/* 遠隔NT-NET FOMA(DTR制御)		*/
#define SCI7_CTS		PORT4.PIDR.BIT.B7							/* 遠隔NT-NET FOMA(CTS状態)		*/
#define SCI7_CD			PORT4.PIDR.BIT.B5							/* 遠隔NT-NET FOMA(CD 状態)		*/
#define SCI7_DSR		PORT4.PIDR.BIT.B6							/* 遠隔NT-NET FOMA(DSR状態)		*/
#define SCI7_CI			PORT4.PIDR.BIT.B4							/* 遠隔NT-NET FOMA(CI 状態)		*/

#ifdef	_USE_SCI1_
/* SCI1関連																						*/
extern	void SCI1_Init( unsigned char, unsigned char, unsigned char, unsigned char);
																	/* 初期化					*/
extern	unsigned char	SCI1_SEND( signed char *, unsigned short );	/* 送信要求					*/
extern	unsigned char	SCI1_READ( signed char * );					/* 受信読出					*/
extern	void			SCI1_CLR( void );							/* RINGﾊﾞｯﾌｧｶｳﾝﾀｸﾘｱ			*/
extern	void			SCI1_BUFF_CLR( void );						/* RINGﾊﾞｯﾌｧｸﾘｱ				*/
																	/*							*/
#define	SCI1_RTS		P_PA.DR.BIT.B5								/* RTS制御					*/
#define	SCI1_DTR		P_P8.DR.BIT.B0								/* DTR制御					*/
#define SCI1_CTS		P_P7.PORT.BIT.B1							/* CTS状態					*/
#define SCI1_CD			P_P8.PORT.BIT.B1							/* CD 状態					*/
#define SCI1_DSR		P_P8.PORT.BIT.B2							/* DSR状態					*/
#define SCI1_CI			P_P8.PORT.BIT.B3							/* CI 状態					*/
																	/*							*/
extern	unsigned char	SCI1_SND_CMP;								/* 	送信完了				*/
extern	unsigned char	SCI1_RCV_CMP;								/* 	受信完了				*/
extern	signed short	SCI1_RCV_TIM;								/* 	受信監視ﾀｲﾏ				*/
extern	signed short	SCI1_CHR_TIM;								/* 	文字間監視ﾀｲﾏ			*/
extern	signed short	SCI1_SND_TIM;								/* 	送信監視ﾀｲﾏ				*/
extern	signed short	SCI1_RTS_TIM;								/* RTS制御ﾀｲﾏ				*/

#endif

#ifdef	_USE_SCI2_
/* SCI2関連																						*/
extern	void SCI2_Init( unsigned char, unsigned char, unsigned char, unsigned char);
																	/* 初期化					*/
extern	unsigned char	SCI2_SEND( char *, unsigned short );		/* 送信要求					*/
extern	unsigned char	SCI2_READ( unsigned char * );				/* 受信読出					*/
extern	void			SCI2_CLR( void );							/* RINGﾊﾞｯﾌｧｶｳﾝﾀｸﾘｱ			*/
extern	void			SCI2_BUFF_CLR( void );						/* RINGﾊﾞｯﾌｧｸﾘｱ				*/
extern	void			SCI2_PWR( unsigned char );					/* VCC ON/OFF				*/
																	/*							*/
extern	unsigned char	SCI2_SND_CMP;								/* 	送信完了				*/
extern	unsigned char	SCI2_RCV_CMP;								/* 	受信完了				*/
extern	unsigned char	SCI2_SND_REQ;								/*  受信許可ﾌﾗｸﾞ		   	*/
extern	signed short	SCI2_RCV_TIM;								/* 	受信監視ﾀｲﾏ				*/
extern	signed short	SCI2_CHR_TIM;								/* 	文字間監視ﾀｲﾏ			*/
extern	signed short	SCI2_SND_TIM;								/* 	送信監視ﾀｲﾏ				*/
#endif

//#ifdef	_USE_SCI3_
/* SCI3関連																						*/
extern	void SCI3_Init( unsigned char, unsigned char, unsigned char, unsigned char);
																	/* 初期化					*/
extern	unsigned char	SCI3_SEND( char *, unsigned short );		/* 送信要求					*/
extern	unsigned char	SCI3_READ( unsigned char * );				/* 受信読出					*/
extern	void			SCI3_CLR( void );							/* RINGﾊﾞｯﾌｧｶｳﾝﾀｸﾘｱ			*/
extern	void			SCI3_BUFF_CLR( void );						/* RINGﾊﾞｯﾌｧｸﾘｱ				*/
																	/*							*/
#define	SCI3_RTS		P_PA.DR.BIT.B7								/* RTS制御					*/
#define SCI3_CTS		P_P7.PORT.BIT.B3							/* CTS状態					*/
																	/*							*/
extern	unsigned char	SCI3_SND_CMP;								/* 	送信完了				*/
extern	unsigned char	SCI3_RCV_CMP;								/* 	受信完了				*/
extern	signed short	SCI3_RCV_TIM;								/* 	受信監視ﾀｲﾏ				*/
extern	signed short	SCI3_CHR_TIM;								/* 	文字間監視ﾀｲﾏ			*/
extern	signed short	SCI3_SND_TIM;								/* 	送信監視ﾀｲﾏ				*/

//#ifdef	_USE_XPT_
/* XPort関連																					*/
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
	unsigned char	SEG[4];											/*  ｾｸﾞﾒﾝﾄ別				*/
	unsigned long	FULL;											/*  4ﾊﾞｲﾄ					*/
} IPv4;																/*							*/
																	/*							*/
extern	void			XPT_Init();									/* 初期化関数				*/
extern	void			XPT_Main();									/* ﾒｲﾝ関数					*/
extern	unsigned char	XPT_RcvQueRead( unsigned char *, unsigned short * );
																	/* 送信ｷｭｰ読取				*/
extern	unsigned char	XPT_SndQueSet ( unsigned char *, unsigned short );
																	/* 送信ｷｭｰ書込				*/
																	/*							*/
#define XPT_CTS			P_P7.PORT.BIT.B3							/* CP1:CTS					*/
#define XPT_DCD			P_P7.PORT.BIT.B2							/* CP2:DCD					*/
#define	XPT_DTR			P_PA.DR.BIT.B7								/* CP3:DTR					*/
#define	XPT_RST			P_PA.DR.BIT.B6								/* RESET					*/
																	/*							*/
extern	unsigned char	XPT_Condition;								/* 状態						*/
																	/*							*/
extern	IPv4			xpt_localIP;								/* ﾛｰｶﾙIP					*/
extern	unsigned short	xpt_localPort;								/* ﾛｰｶﾙﾎﾟｰﾄ					*/
extern	IPv4			xpt_hostIP;									/* ﾎｽﾄIP					*/
extern	unsigned short	xpt_hostPort;								/* ﾎｽﾄﾎﾟｰﾄ					*/
extern	IPv4			xpt_Gateway;								/* Gateway					*/
extern	XPT_CM			xpt_protocol;								/* 44h:TCP/IP 4Ch:UDP/IP	*/
extern	unsigned char	xpt_sndchr1;								/* 送信ﾄﾘｶﾞ文字				*/
extern	unsigned char	xpt_sndchr2;								/* 送信ﾄﾘｶﾞ文字				*/
extern	XPT_PC			xpt_pack;									/* 送信ﾊﾟｯｸ					*/
extern	unsigned char	xpt_conRetry;								/* ｺﾈｸｼｮﾝﾘﾄﾗｲ回数(0～99回)	*/
extern	unsigned char	xpt_conWait;								/* ｺﾈｸｼｮﾝ待ち時間(0～99秒)	*/
extern	unsigned char	xpt_conSend;								/* ｺﾈｸｼｮﾝ送信間隔(0～99秒)	*/
extern	unsigned char	xpt_sndRetry;								/* 送信ﾘﾄﾗｲ回数(0～99回)	*/
extern	unsigned char	xpt_sndWait;								/* 送信待ち時間(0～99秒)	*/

//#endif

//#endif

#ifdef	_USE_SCI4_
/* SCI4関連																						*/
extern	void SCI4_Init( unsigned char, unsigned char, unsigned char, unsigned char);
																	/* 初期化					*/
extern	unsigned char	SCI4_SEND( signed char *, unsigned short );	/* 送信要求					*/
extern	unsigned char	SCI4_READ( signed char * );					/* 受信読出					*/
extern	void			SCI4_CLR( void );							/* RINGﾊﾞｯﾌｧｶｳﾝﾀｸﾘｱ			*/
extern	void			SCI4_BUFF_CLR( void );						/* RINGﾊﾞｯﾌｧｸﾘｱ				*/
extern	void			SCI4_PWR( unsigned char );					/* VCC ON/OFF				*/
																	/* 受信読出					*/
extern	unsigned char	SCI4_SND_CMP;								/* 	送信完了				*/
extern	unsigned char	SCI4_RCV_CMP;								/* 	受信完了				*/
extern	signed short	SCI4_RCV_TIM;								/* 	受信監視ﾀｲﾏ				*/
extern	signed short	SCI4_CHR_TIM;								/* 	文字間監視ﾀｲﾏ			*/
extern	signed short	SCI4_SND_TIM;								/* 	送信監視ﾀｲﾏ				*/
#endif

#ifdef	_USE_NMI_
/* WDT関連																						*/
extern	void			wdt_reset( void );							/* ｳｫｯﾁﾄﾞｸﾞﾀｲﾏ ﾘｾｯﾄ			*/
#endif

/* ﾛｸﾞ関連																						*/
#define					_PROLOG_USE				0			// ﾘﾘｰｽ時には 0 にする　//
extern	void			LogReset( void );							/* ﾛｸﾞの強制ﾘｾｯﾄ			*/
extern	void			LogClear( void );							/* ﾛｸﾞの初期化				*/
extern	void			LogWrite( unsigned char, unsigned char, char * );
																	/* ﾛｸﾞへ書き出し			*/
extern	unsigned char	LogRead( unsigned short );					/* ﾛｸﾞ読出し				*/
extern	void			LogInfo( void );							/* ﾛｸﾞ件数表示				*/
extern	void			ProLog( unsigned char, signed char *, ... );
																	/* 通信ﾛｸﾞ送信(書式付)		*/
extern	  signed char	LOG_BUFF[];									/* ﾛｸﾞ整形後の文字列		*/

/* PC設定関連																					*/
extern	void			PC_SET_COMM( unsigned char * );				/* 共通設定書込み処理		*/
extern	void			PC_GET_COMM( unsigned char * );				/* 共通設定参照処理			*/
extern	void			PC_SET_HOST( unsigned char *, unsigned char );
																	/* HOST設定書込み処理		*/
extern	void			PC_GET_HOST( unsigned char *, unsigned char );
																	/* HOST設定参照処理			*/

/* ｼｽﾃﾑｴﾗｰ関連																					*/
//extern	unsigned char	SysErr[100];								/* ｼｽﾃﾑｴﾗｰ格納配列			*/
extern	void			SysErrSet( unsigned char, unsigned char );	/* ｼｽﾃﾑｴﾗｰ発生/解除関数		*/


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

	
#define		UM03KO_PWR_PulseTime		11				// UM03-KO PWR SG 出力時間：1秒(+0.1)
#define		UM03KO_PowerOffTime			25				// UM03-KO CS&DR OFF待ち時間：2秒(+0.5)
#define		UM03KO_PowerOnDelay			635				// UM03-KO CS OFF→PWR ON待ち時間：63秒(+0.5)
#define		UM03KO_ER_DR_Int			165				// UM03-KO ER ON→DR ON待ち時間：16秒(+0.5)
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
#define		UM04KO_PowerOffTime			100				// AD-04S(UM04-KO) CS&DR OFF待ち時間：10秒
#define		UM04KO_PowerOnDelay			600				// AD-04S(UM04-KO) CS OFF→PWR ON待ち時間：60秒
#define		UM04KO_ER_DR_Int			265				// AD-04S(UM04-KO) (PWR ON)ER ON→DR ON待ち時間：26秒(+0.5)
#define		UM04KO_ER_ON				20				// AD-04S(UM04-KO) PWR ON→ER ON待ち時間：2秒
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御

#endif
