/*[]----------------------------------------------------------------------[]*/
/*|		RAM/SRAM Memory Data on toSlave task in New I/F(Master)			   |*/
/*|			・ここには、新I/F盤（親機）内の「対子機通信」タスクが		   |*/
/*|			  参照する領域を定義します。								   |*/
/*|			・CPU内臓RAMと、外付けS-RAMが混在しています。				   |*/
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
/* 通信状態 */
#define	toS_COM_INIT	0		/* イニシャル前 */
#define	toS_COM_IDOL	1		/* アイドル状態 */


/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/
/* 対子機通信プロトコル */
typedef struct {
	uchar	State;				/* リンクレベル伝送マトリックスの状態 */
								/*		0: (S0)ニュートラル           */
								/*		1: (S1)ヘッダ文字待ち         */
								/*		2: (S2)データサイズ待ち       */
								/*		3: (S3)データ種別待ち         */
								/*		4: (S4)ターミナルNo.待ち      */
								/*		5: (S5)データ待ち             */
								/*		6: (S6)BCC待ち                */
} t_toScom_Matrix;

/* タイマカウント値 */
typedef struct {
//	ushort	usBetweenChar;		/* 文字間監視タイマ */
	ushort	usSendWait;			/* 送信ウェイトタイマ */
	ushort	usLinkWatch;		/* 通信受信監視タイマ */
	ushort	usLineWatch;		/* 通信回線監視タイマ */
	ushort	usPolling;			/* 子機へのポーリング間隔 */
} t_toScom_Timer;

/* 子機ポーリング情報 */
typedef struct {
	uchar	ucOrder;			/* 状態要求する相手(ポーリングする)子機ターミナル */
	uchar	ucNow;				/* 現在の通信相手(状態要求のみならず)子機ターミナル */
	uchar	ucActionLock;		/* (状態書き込み対象の)ロック装置・・・使うか？*/
	uchar	bWaitAnser;			/* 応答待ち中フラグ */
								/*		1：上記「ucNow」子機に対して応答電文を待っている */
								/*		0：応答待ち無し（アイドル状態）*/
//（デバッグ用途）
	struct {
		uchar	LastCmd;		/* 応答待ち対象「コマンド(＝データ種別)」*/
		ushort	sndI49;			/* (49H)送信回数 */
		ushort	sndW57;			/* (57H)送信回数 */
		ushort	sndR52;			/* (52H)送信回数 */
		ushort	rcvA41;			/* (41H)受信回数 */
		ushort	rcvV56;			/* (56H)受信回数 */
		ushort	rcvACK;			/*   ANK受信回数 */
		ushort	rcvNAK;			/*   NAK受信回数 */
		ushort	rcvAny;			/*  異常受信回数 */
		ushort	no_ANS;			/*    無応答回数 */
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
extern	int		toScom_GetSlaveNow(void);		/* 今の通信相手を取得する */

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
