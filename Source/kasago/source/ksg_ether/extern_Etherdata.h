#include "Ether.h"
#include <trsocket.h>		// for kasago
/********************************************/
/*	Data definitions						*/
/********************************************/
extern	USHT 		wCurrentRxPacketPointer;// RX�ɑ΂��錻�݂�PacketPointer
											// Initial�l = 0x0000
											// MACGetHeader()�֐��ňȉ��̏�����K���s��
											//		wCurrentRxPacketPointer = wNextRxPacketPointer;
											//		WriteReg(REG_ERXRDPT, wCurrentRxPacketPointer);
extern	USHT 		wNextRxPacketPointer;	// RX�ɑ΂��鎟��PacketPointer
											// Initial�l = ADDR_RXSTART;
											// MACGetHeader()�֐��ŏ�L�̏�����Ɉȉ��̏�����K���s��
											//		wNextRxPacketPointer = header.NextPacketPointer;(��M����Data)
extern	ETHER_FLAGS			EtherFlags;				// 
extern	MAC_ADDR			MyMacAddr;				// 
extern	SEND_RECEV_INFO		SendRecvInfo;			// 
extern	ttUserInterface		OpenHandle;				// 
/********************************************/
extern	ETHER_RECV_INFO		EtheRecvInfo;			// 
/********************************************/
