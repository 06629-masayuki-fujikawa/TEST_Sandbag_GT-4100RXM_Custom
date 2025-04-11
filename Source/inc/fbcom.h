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
/*| ・磁気リーダに対する通信関連の変数、ﾃｰﾌﾞﾙ定義、ﾌﾟﾛﾄﾀｲﾌﾟ宣言												   |*/
/*|	・system.hの後に宣言すること																			   |*/
/*[]--------------------------------------------------------------------- Copyright(C) 2011 AMANO Corp.-------[]*/

/*==============================================================================================================*/
/*				ﾃﾞﾌｧｲﾝ定義																						*/
/*==============================================================================================================*/
#define		FB_PKT_MAX				270											/* (126 + 9) * 2				*/
#define		FB_PKT_COMMAND			5											/* 受信ﾃﾞｰﾀｺﾏﾝﾄﾞ位置			*/
#define		FB_PKT_MIN_SIZ			3											/* 最小ﾊﾟｹｯﾄｻｲｽﾞ（SYN,ADR,xxx)	*/
#define		FB_ASCII_PKT_HEAD_SIZ	10											/* ASCIIﾊﾟｹｯﾄﾍｯﾀﾞｰｻｲｽﾞ			*/
#define		FB_PKT_EOT_SIZ			9											/* EOTﾊﾟｹｯﾄｻｲｽﾞ					*/
#define		FB_PCK_SEQNO			3

/* 相手側から強制ACKが送信されない場合に自ら送信を終了するための	*/			/*								*/
/* ﾘﾄﾗｲ回数を規定する(ｾｰﾌﾃｨ機能)									*/			/*								*/
#define		FB_DTSND_RETRY			20											/* 送信ﾃﾞｰﾀに対する応答NGｶｳﾝﾄ	*/
#define		FB_DTSND_REPEAT			3											/* 通信異常後の繰返し回数		*/
#define		FB_DTRCV_RETRY			3											/* 受信ﾃﾞｰﾀに対する応答回数ｶｳﾝﾄ	*/
#define		FB_NORSP_CNT			500											/* 無応答に対するｶｳﾝﾄ			*/

#define		FB_S0					0											/* ﾆｭｰﾄﾗﾙ						*/
#define		FB_S1					1											/* ﾎﾟｰﾘﾝｸﾞ後、応答ﾀ待ち			*/
#define		FB_S2					2											/* ｾﾚｸﾃｨﾝｸﾞ後、応答待ち			*/
#define		FB_S3					3											/* ﾃﾞｰﾀ送信後、応答待ち			*/
#define		FB_S4					4											/* ｾﾚｸﾃｨﾝｸﾞ後、応答待ち			*/

#define		FB_REQ_STX				0x02										/* 制御ｺｰﾄﾞ(STX)				*/
#define		FB_REQ_EOT				0x04										/* 制御ｺｰﾄﾞ(EOT)				*/
#define		FB_REQ_ENQ				0x05										/* 制御ｺｰﾄﾞ(ENQ)				*/
#define		FB_REQ_ACK				0x06										/* 制御ｺｰﾄﾞ(ACK)				*/
#define		FB_REQ_NAK				0x15										/* 制御ｺｰﾄﾞ(NAK)				*/
#define		FB_REQ_SYN				0x16										/* 制御ｺｰﾄﾞ(SYN)				*/

#define		FB_BCC_NG				5											/* BCC-NG return code			*/
#define		FB_SERIAL_NG			6											/* シリアルエラー				*/

#define		FB_RCVBUF_EMPTY			0											/* 受信ﾊﾞｯﾌｧ空き				*/
#define		FB_RCVBUF_FULL			1											/* 受信ﾊﾞｯﾌｧFULL				*/

/*--------------*/
/*	送信ﾃﾞｰﾀID	*/
/*--------------*/
#define		FB_SND_INIT_ID			0x00										/* 初期設定ﾃﾞｰﾀ					*/
#define		FB_SND_CTRL_ID			0x01										/* 制御ﾃﾞｰﾀ						*/
#define		FB_SND_DATA_ID			0x02										/* 電文							*/

/*--------------*/
/*	受信ﾃﾞｰﾀID	*/
/*--------------*/
#define		FB_RCV_VERS_ID			0x09										/* バージョンチェックデータ		*/
#define		FB_RCV_READ_ID			0x44										/* リードデータ					*/
#define		FB_RCV_END_ID			0x45										/* 終了データ					*/
#define		FB_RCV_SENSOR_ID		0x4c										/* センサー感度チェック			*/
#define		FB_RCV_MNT_ID			0x72										/* メンテナンス応答				*/

// エラー内容
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
/*						構造体定義							*/
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
/* 端末毎に管理する変数 */
/*----------------------*/

/***** 送信ﾃﾞｰﾀｷｭｰ管理ﾃｰﾌﾞﾙ *****/
typedef struct {																/*								*/
	ushort	R_Index;															/* ﾘｰﾄﾞﾎﾟｲﾝﾀ					*/
	ushort	W_Index;															/* ﾗｲﾄﾎﾟｲﾝﾀ						*/
	ushort	Count;																/* 件数							*/
} t_FBcomQueCtrl;																/*								*/

typedef struct {
	ushort size;
	uchar buffer[250];
} t_FBsendDataBuffer;

/***** 回線管理 *****/
typedef struct {																/*								*/
	/*** 回線管理 ***/
	uchar	Matrix;																/* ﾏﾄﾘｸｽ						*/
	uchar	RcvDtSeqNo;															/* 端末毎受信ﾃﾞｰﾀ前回のSEQNo.	*/
	uchar	DtCrcNG;															/* 端末毎ﾃﾞｰﾀ応答CRC NG回数		*/
	uchar	DtSndNG;															/* 送信ﾃﾞｰﾀに対するNAKﾘﾄﾗｲｶｳﾝﾄ	*/
	uchar	DtRepNG;															/* 通信異常時のﾘﾄﾗｲｶｳﾝﾄ			*/
	uchar	DtRcvNG;															/* 受信ﾃﾞｰﾀに対するNAK送信ｶｳﾝﾄ	*/
	ushort	NoAnsCnt;															/* 無応答時の端末ｽｷｯﾌﾟ用POLｶｳﾝﾀ	*/
	uchar	RcvResFlg;															/* 受信ﾌﾗｸﾞ						*/
	uchar	SndDtID;															/* 送信ﾃﾞｰﾀ消去の為(ﾚｽﾎﾟﾝｽﾁｪｯｸ用)*/
																				/*								*/
	/*** ｴﾗｰﾌﾗｸﾞ ***/
	union	{																	/*								*/
		struct	{																/*								*/
			uchar	CtCRC_NG : 1 ;												/* 1=POL/SEL応答 CRC error		*/
			uchar	DtCRC_NG : 1 ;												/* 1=ﾃﾞｰﾀ応答    CRC error		*/
			uchar	Resp_NG	 : 1 ;												/* 1=ﾚｽﾎﾟﾝｽ無し(無応答)			*/
			uchar	bit_4	 : 1 ;												/* reserve						*/
			uchar	bit_3	 : 1 ;												/* reserve						*/
			uchar	bit_2	 : 1 ;												/* reserve						*/
			uchar	bit_1	 : 1 ;												/* reserve						*/
			uchar	bit_0	 : 1 ;												/* reserve						*/
		} bits;																	/*								*/
		uchar	byte;															/*								*/
	}ErrFlg;																	/*								*/
																				/*								*/
	t_FBcomQueCtrl	CtrlInfo;													/* ｷｭｰﾊﾞｯﾌｧｺﾝﾄﾛｰﾙ				*/
	t_FBsendDataBuffer SendData[FB_SENDDATA_QUEUE_MAX];							/* ﾃﾞｰﾀ格納ﾊﾞｯﾌｧ				*/
																				/*								*/
	uchar	status[4];															/* ステータス情報				*/

} t_FBcomTerm;																	/* 								*/
																				/*								*/
																				/*								*/
/*-----------------------------------*/
/* 受信したﾃﾞｰﾀをMAINへ渡す為のﾊﾞｯﾌｧ */
/*-----------------------------------*/
#define		FB_RCV_BUF_SIZE		256												/* 受信ﾃﾞｰﾀ格納ﾊﾞｯﾌｧｻｲｽﾞ		*/
																				/*								*/
extern	uchar		FBcom_RcvData[FB_RCV_BUF_SIZE];								/*								*/
																				/*								*/
extern	uchar		FB_RcvBuf[ FB_SCI_RCVBUF_SIZE ];							/*								*/
																				/*								*/
extern	ushort		FBcom_Timer_2_Value;										/* ﾀｲﾏｰ2						*/
extern	ushort		FBcom_Timer_4_Value;										/* ﾀｲﾏｰ4						*/
extern	ushort		FBcom_Timer_5_Value;										/* ﾀｲﾏｰ5						*/
																				/*								*/
extern	uchar		FBcomdr_f_RcvCmp;											/* ｲﾍﾞﾝﾄﾌﾗｸﾞ					*/
extern	uchar		FBcomdr_SciErrorState;										/* シリアルエラー状態			*/
extern	ushort		FBcomdr_RcvLength;											/* 受信ﾃﾞｰﾀｻｲｽﾞ(ﾜｰｸ)			*/
extern	uchar		FBcomdr_RcvData[FB_SCI_RCVBUF_SIZE];						/* 受信ﾃﾞｰﾀﾊﾞｯﾌｧ(ﾜｰｸ)			*/
extern	uchar		FB_SndBuf[FB_SCI_SNDBUF_SIZE];								/* 送信ﾊﾞｯﾌｧ					*/
																				/*								*/
																				/*								*/
/*==============================================================================================================*/
/*				ﾌﾟﾛﾄﾀｲﾌﾟ宣言																					*/
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
