#include "Ether.h"
#include <trsocket.h>		// for kasago
/********************************************/
/*	Data definitions						*/
/********************************************/
USHT 				wCurrentRxPacketPointer;// RXに対する現在のPacketPointer
											// Initial値 = 0x0000
											// MACGetHeader()関数で以下の処理を必ず行う
											//		wCurrentRxPacketPointer = wNextRxPacketPointer;
											//		WriteReg(REG_ERXRDPT, wCurrentRxPacketPointer);
USHT 				wNextRxPacketPointer;	// RXに対する次のPacketPointer
											// Initial値 = ADDR_RXSTART;
											// MACGetHeader()関数で上記の処理後に以下の処理を必ず行う
											//		wNextRxPacketPointer = header.NextPacketPointer;(受信したData)
ETHER_FLAGS			EtherFlags;				// 
MAC_ADDR			MyMacAddr;				// 
SEND_RECEV_INFO		SendRecvInfo;			// 
ttUserInterface		OpenHandle;				// 
/********************************************/
ETHER_RECV_INFO		EtheRecvInfo;
/********************************************/
