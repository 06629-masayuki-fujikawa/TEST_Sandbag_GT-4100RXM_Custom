#ifndef RAU_SIC_H
#define RAU_SIC_H

#include	"trsystem.h"
#include	"trmacro.h"

/*==============================================================================================================*/
/*				define��`																						*/
/*==============================================================================================================*/

#define 	KSG_RauComdr_QUEUE_COUNT	16

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

#define		KSG_RAU_SCI_SNDBUF_SIZE			(4096)										/*								*/
#define		KSG_RAU_SCI_RCVBUF_SIZE			(4096)										/*								*/

#define		KSG_RAU_FLAG_SEQUENCE			(uchar)0x7e									// �t���O�V�[�P���X

/*==========================================================*/
/*						�\���̒�`							*/
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
/* �[�����ɊǗ�����ϐ� */
/*----------------------*/

/***** ���M�ް�����Ǘ�ð��� *****/
typedef struct {																		/*								*/
	ushort	R_Index;																	/* ذ���߲��					*/
	ushort	W_Index;																	/* ײ��߲��						*/
	ushort	Count;																		/* ����							*/
} t_RauComQueCtrl;																		/*								*/

typedef struct {
	ushort		RcvLength;
	uchar		RcvData[KSG_RAU_SCI_RCVBUF_SIZE];
} t_KSG_RauComdrDataQueue;
																						/*								*/
/*-----------------------------------*/
/* ��M�����ް���MAIN�֓n���ׂ��ޯ̧ */
/*-----------------------------------*/
//extern	ushort		KSG_rismcom_Timer_6_Value;											/* ��ϰ6						*/
//extern	uchar		KSG_rismcomRcvBuffFull;												/* ��M�ޯ̧FULL				*/
extern	uchar		KSG_RauComdr_f_RcvCmp;												/* ������׸�					*/
extern	uchar		KSG_RauSndBuf[KSG_RAU_SCI_SNDBUF_SIZE];								/* ���M�ޯ̧					*/
																						/*								*/
/*==============================================================================================================*/
/*				�������ߐ錾																					*/
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
