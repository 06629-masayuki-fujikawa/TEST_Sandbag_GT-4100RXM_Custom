/*[]----------------------------------------------------------------------[]*/
/*|		RAM/SRAM Memory Data on toSlave task in New I/F(Master)			   |*/
/*|			�E�����ɂ́A�VI/F�Ձi�e�@�j���́u�Ύq�@�ʐM�v�^�X�N��		   |*/
/*|			  �Q�Ƃ���̈���`���܂��B								   |*/
/*|			�ECPU����RAM�ƁA�O�t��S-RAM�����݂��Ă��܂��B				   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _TO_S_H_
#define _TO_S_H_

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/
/* �ʐM��� */
#define	toS_COM_INIT	0		/* �C�j�V�����O */
#define	toS_COM_IDOL	1		/* �A�C�h����� */


/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/
/* �Ύq�@�ʐM�v���g�R�� */
typedef struct {
	uchar	State;				/* �����N���x���`���}�g���b�N�X�̏�� */
								/*		0: (S0)�j���[�g����           */
								/*		1: (S1)�w�b�_�����҂�         */
								/*		2: (S2)�f�[�^�T�C�Y�҂�       */
								/*		3: (S3)�f�[�^��ʑ҂�         */
								/*		4: (S4)�^�[�~�i��No.�҂�      */
								/*		5: (S5)�f�[�^�҂�             */
								/*		6: (S6)BCC�҂�                */
} t_toScom_Matrix;

/* �^�C�}�J�E���g�l */
typedef struct {
//	ushort	usBetweenChar;		/* �����ԊĎ��^�C�} */
	ushort	usSendWait;			/* ���M�E�F�C�g�^�C�} */
	ushort	usLinkWatch;		/* �ʐM��M�Ď��^�C�} */
	ushort	usLineWatch;		/* �ʐM����Ď��^�C�} */
	ushort	usPolling;			/* �q�@�ւ̃|�[�����O�Ԋu */
} t_toScom_Timer;

/* �q�@�|�[�����O��� */
typedef struct {
	uchar	ucOrder;			/* ��ԗv�����鑊��(�|�[�����O����)�q�@�^�[�~�i�� */
	uchar	ucNow;				/* ���݂̒ʐM����(��ԗv���݂̂Ȃ炸)�q�@�^�[�~�i�� */
	uchar	ucActionLock;		/* (��ԏ������ݑΏۂ�)���b�N���u�E�E�E�g�����H*/
	uchar	bWaitAnser;			/* �����҂����t���O */
								/*		1�F��L�uucNow�v�q�@�ɑ΂��ĉ����d����҂��Ă��� */
								/*		0�F�����҂������i�A�C�h����ԁj*/
//�i�f�o�b�O�p�r�j
	struct {
		uchar	LastCmd;		/* �����҂��Ώہu�R�}���h(���f�[�^���)�v*/
		ushort	sndI49;			/* (49H)���M�� */
		ushort	sndW57;			/* (57H)���M�� */
		ushort	sndR52;			/* (52H)���M�� */
		ushort	rcvA41;			/* (41H)��M�� */
		ushort	rcvV56;			/* (56H)��M�� */
		ushort	rcvACK;			/*   ANK��M�� */
		ushort	rcvNAK;			/*   NAK��M�� */
		ushort	rcvAny;			/*  �ُ��M�� */
		ushort	no_ANS;			/*    �������� */
	} dbg;
} t_toScom_Polling;


/*==================================*/
/*		InRAM area define			*/
/*==================================*/


/*----------------------------------*/
/*		function external			*/
/*----------------------------------*/
/* IFMmain.c */

/* IFMsub.c */
extern	void	toS_EarlyDataInit( void );
extern	uchar	toS_GetBPSforSalve(void);

/* IFMcom.c */
extern	void	toScom_Init( void );
extern	void	toScom_Main( void );
extern	int		toScom_GetSlaveNow(void);		/* ���̒ʐM������擾���� */

/* IFMcomdr.c */
extern	void	toScomdr_Init( void );
extern	void	toScomdr_Main( void );

/* IFMsci.c */
extern	void	toSsci_Init(uchar Speed, uchar Dbits, uchar Sbits, uchar Pbit);
extern	uchar	toSsci_SndReq( unsigned short Length );
extern	uchar	toSsci_IsSndCmp( void );
extern	void	toSsci_Stop( void );
extern	void	toScom_RtsTimStart( ushort TimVal );
extern	void	toScom_RtsTimStop( void );
extern	void	toS_Enable_RS485Driver(uchar Boolean);
extern	struct	t_IF_Queue*	toScom_DeQueueUnusedSelectingData(void);
extern  struct	t_IF_Queue*	toScom_DeQueueTerminalNo(uchar tno);

/* IFMtime.c */
extern	void	toS_TimerInit( void );
extern	void	toScom_2msTimerStart( ushort TimVal );
extern	void	toScom_2msTimerStop( void );
extern	uchar	toScom_2msTimeout( void );
extern	void	toScom_LinkTimerStart(ushort TimVal);
extern	void	toScom_LinkTimerStop(void);
extern	uchar	toScom_LinkTimeout(void);
extern	void	toScom_LineTimerStart(ushort TimVal);
extern	void	toScom_LineTimerStop(void);
extern	uchar	toScom_LineTimeout(void);
extern	uchar	toScom_Is_LineTimerStop(void);

extern	void	toS_2msInt(void);
extern	void	toS_10msInt(void);

// toSmain.c
extern	void	toS_init( void );

// toSsci.c
extern	void	toSsci_Init(unsigned char Speed, unsigned char Dbits, unsigned char Sbits, unsigned char Pbit);
extern	void	toS_Int_RXI2( void );
extern	void	toS_Int_ERI2(void);
extern	uchar	toSsci_SndReq( unsigned short Length );
extern	void	toS_Int_TXI2( void );
extern	void	toS_Int_TEI2( void );
extern	unsigned char	toSsci_IsSndCmp( void );
extern	void	toSsci_Stop( void );

#endif	// _TO_S_H_
