//[]----------------------------------------------------------------------[]
///	@brief			
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

/*==============================================================================================================*/
/*				ﾃﾞﾌｧｲﾝ定義																						*/
/*==============================================================================================================*/

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

#define		BT_SCI_SNDBUF_SIZE		(256)
#define		BT_SCI_RCVBUF_SIZE		(256)
#define		PON_TIMEOUT				100											// 20ms単位(2s)
#define		RSP_TIMEOUT				50											// 20ms単位(1s)
#define		PORT_WAIT				5											// 20ms単位(100ms)

#define		BT_INIT_CMD_1_LEN	18
#define		BT_INIT_CMD_2_LEN	33
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
	ushort	ComerrStatus;
} t_BtSciRcvCtrl;
extern	t_BtSciRcvCtrl	Bt_RcvCtrl;


/** send buffer control area **/
typedef struct {
    ushort  SndReqCnt;															/* send request character count	*/
    ushort  SndCmpCnt;															/* send complete character count*/
    ushort  ReadIndex;															/* next send data (read) index	*/
    ushort  SndCmpFlg;															/* send complete flag (1=complete, 0=not yet) */
} t_BtSciSndCtrl;
extern	t_BtSciSndCtrl	Bt_SndCtrl;

extern	uchar			Bt_RcvBuf[ BT_SCI_RCVBUF_SIZE ];
extern	uchar			Bt_SndBuf[ BT_SCI_SNDBUF_SIZE ];
extern	uchar			Btcomdr_RcvData[BT_SCI_RCVBUF_SIZE];
extern	ushort			Btcom_Timer_1_Value;									// 文字間

extern	const char bt_init_cmd1[];
extern	const char bt_init_cmd2[];
extern	const char bt_ok[];

/*==============================================================================================================*/
/*				ﾌﾟﾛﾄﾀｲﾌﾟ宣言																					*/
/*==============================================================================================================*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	bluetooth.c																									*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	void	Bluetooth_unit(void);
extern	uchar	Btcom_EventCheck( void );
extern	void	Btcomdr_RcvInit( void );
extern	void	Btcomdr_SendInit( void );
extern	void	Btcom_Init( void );
extern	void	Btcom_1mTimStart( ushort );
extern	void	Btcom_1mTimStop( void );
extern	uchar	Btcom_1mTimeout( void );
extern	void	Btcom_20mTimStart( ushort );
extern	void	Btcom_20mTimStop( void );
extern	uchar	Btcom_20mTimeout( void );


/*--------------------------------------------------------------------------------------------------------------*/
/*	bluetoothsci.c																								*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	void	Bluetooth_Int_RXI( void );
extern	void	Bluetooth_Int_ERI( void );
extern	uchar	Bluetoothsci_SndReq( ushort );
extern	uchar	Bluetoothsci_IsSndCmp( void );
extern	void	Bluetooth_Int_TXI( void );
extern	void	Bluetooth_Int_TEI( void );
#endif	// _BLUETOOTH_H_
