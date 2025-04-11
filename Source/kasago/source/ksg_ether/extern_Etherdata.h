#include "Ether.h"
#include <trsocket.h>		// for kasago
/********************************************/
/*	Data definitions						*/
/********************************************/
extern	USHT 		wCurrentRxPacketPointer;// RXに対する現在のPacketPointer
											// Initial値 = 0x0000
											// MACGetHeader()関数で以下の処理を必ず行う
											//		wCurrentRxPacketPointer = wNextRxPacketPointer;
											//		WriteReg(REG_ERXRDPT, wCurrentRxPacketPointer);
extern	USHT 		wNextRxPacketPointer;	// RXに対する次のPacketPointer
											// Initial値 = ADDR_RXSTART;
											// MACGetHeader()関数で上記の処理後に以下の処理を必ず行う
											//		wNextRxPacketPointer = header.NextPacketPointer;(受信したData)
extern	ETHER_FLAGS			EtherFlags;				// 
extern	MAC_ADDR			MyMacAddr;				// 
extern	SEND_RECEV_INFO		SendRecvInfo;			// 
extern	ttUserInterface		OpenHandle;				// 
/********************************************/
extern	ETHER_RECV_INFO		EtheRecvInfo;			// 
/********************************************/
