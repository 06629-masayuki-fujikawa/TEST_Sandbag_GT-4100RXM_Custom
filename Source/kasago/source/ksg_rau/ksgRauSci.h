#ifndef RAU_SIC_H
#define RAU_SIC_H

#include	"trsystem.h"
#include	"trmacro.h"

/*==============================================================================================================*/
/*				define定義																						*/
/*==============================================================================================================*/

#define 	KSG_RauComdr_QUEUE_COUNT	16

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

#define		KSG_RAU_SCI_SNDBUF_SIZE			(4096)										/*								*/
#define		KSG_RAU_SCI_RCVBUF_SIZE			(4096)										/*								*/

#define		KSG_RAU_FLAG_SEQUENCE			(uchar)0x7e									// フラグシーケンス

/*==========================================================*/
/*						構造体定義							*/
/*==========================================================*/

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/

/** receive buffer control area **/
typedef struct {
	ushort	RcvCnt;																		/* received charcter count		*/
	ushort	ReadIndex;																	/* next read index				*/
	ushort	WriteIndex;																	/* next write index				*/
	ushort	OvfCount;																	/* overflow occur count			*/
	ushort	ComerrStatus;																/*								*/
} t_RauSciRcvCtrl;																		/*								*/
																						/*								*/
extern	t_RauSciRcvCtrl	KSG_RauRcvCtrl;													/*								*/


/** send buffer control area **/
typedef struct {
	ushort	SndReqCnt;																	/* send request character count	*/
	ushort	SndCmpCnt;																	/* send complete character count*/
	ushort	ReadIndex;																	/* next send data (read) index	*/
	ushort	SndCmpFlg;																	/* send complete flag (1=complete, 0=not yet) */
} t_RauSciSndCtrl;

extern	t_RauSciSndCtrl	KSG_RauSndCtrl;													/*								*/

/*----------------------*/
/* 端末毎に管理する変数 */
/*----------------------*/

/***** 送信ﾃﾞｰﾀｷｭｰ管理ﾃｰﾌﾞﾙ *****/
typedef struct {																		/*								*/
	ushort	R_Index;																	/* ﾘｰﾄﾞﾎﾟｲﾝﾀ					*/
	ushort	W_Index;																	/* ﾗｲﾄﾎﾟｲﾝﾀ						*/
	ushort	Count;																		/* 件数							*/
} t_RauComQueCtrl;																		/*								*/

typedef struct {
	ushort		RcvLength;
	uchar		RcvData[KSG_RAU_SCI_RCVBUF_SIZE];
} t_KSG_RauComdrDataQueue;
																						/*								*/
/*-----------------------------------*/
/* 受信したﾃﾞｰﾀをMAINへ渡す為のﾊﾞｯﾌｧ */
/*-----------------------------------*/
//extern	ushort		KSG_rismcom_Timer_6_Value;											/* ﾀｲﾏｰ6						*/
//extern	uchar		KSG_rismcomRcvBuffFull;												/* 受信ﾊﾞｯﾌｧFULL				*/
extern	uchar		KSG_RauComdr_f_RcvCmp;												/* ｲﾍﾞﾝﾄﾌﾗｸﾞ					*/
extern	uchar		KSG_RauSndBuf[KSG_RAU_SCI_SNDBUF_SIZE];								/* 送信ﾊﾞｯﾌｧ					*/
																						/*								*/
/*==============================================================================================================*/
/*				ﾌﾟﾛﾄﾀｲﾌﾟ宣言																					*/
/*==============================================================================================================*/
/*																												*/
extern	void		KSG_RauComInit( void );												/*								*/
extern	void		KSG_RauComMain( void );												/*								*/

extern	void		KSG_RauComdrMain( void );											/*								*/
extern	void		KSG_RauComdrRcvInit( void );										/*								*/
																						/*								*/
extern	uchar		KSG_RauSciSndReq(  ushort );										/*								*/
extern	uchar		KSG_RauSciIsSndCmp( void );											/*								*/

#endif // RAU_SIC_H
