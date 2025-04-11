#ifndef _CONFIG_CAN_API_
#define _CONFIG_CAN_API_
//[]----------------------------------------------------------------------[]
///	@file		config_can_api.h
///	@brief		CAN APIの設定定義
///	@date		2012/01/11
///	@author		m.onouchi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

/*** CAN interrupt ***/
#define CAN1_INT_LVL			2	// 割り込み優先レベル2
#define CAN1_INT_LVL_RCV			7	// 割り込み優先レベル7

/*** Board specific ports ***/
#define CAN1_TERM_PORT		5		// P56:CANトランシーバの終端抵抗
#define CAN1_TERM_PIN		6		// 
#define CAN1_TERM_LVL		0		// Low = terminate.

/*** Baudrate settings ***/
//	Calculation of baudrate:
//	*********************************
//	*	PCLK = 50 MHz = fcan.		*
//	*	fcanclk = fcan/prescale		*
//	*********************************
//	Desired baudrate 1 Mbps.
//	Selecting prescale to 5.
//	fcanclk = 50/5[MHz]
//	fcanclk = 10[MHz]
//	Bitrate = fcanclk/Tqtot
//
//	Tqtot = fcanclk/bitrate
//	Tqtot = 10M/1M
//	Tqtot = 10/1 = 10
//	Tqtot = TSEG1 + TSEG2 + SS
//	Using TSEG1 = 6 Tq
//		  TSEG2 = 3 Tq
//		  SS = 1 Tq always
//		  Re-synchronization Control (SJW) should be 1-4 Tq (must be <=TSEG2).
	#define CAN_BRP		5
	#define CAN_TSEG1	6
	#define CAN_TSEG2	3
	#define CAN_SJW		1

/*** Other settings  ***/
#define MAX_CAN_REG_POLLTIME	4

#endif
