#include "Ether.h"
#include <trsocket.h>		// for kasago
/********************************************/
/*	Data definitions						*/
/********************************************/
USHT 				wCurrentRxPacketPointer;// RX�ɑ΂��錻�݂�PacketPointer
											// Initial�l = 0x0000
											// MACGetHeader()�֐��ňȉ��̏�����K���s��
											//		wCurrentRxPacketPointer = wNextRxPacketPointer;
											//		WriteReg(REG_ERXRDPT, wCurrentRxPacketPointer);
USHT 				wNextRxPacketPointer;	// RX�ɑ΂��鎟��PacketPointer
											// Initial�l = ADDR_RXSTART;
											// MACGetHeader()�֐��ŏ�L�̏�����Ɉȉ��̏�����K���s��
											//		wNextRxPacketPointer = header.NextPacketPointer;(��M����Data)
ETHER_FLAGS			EtherFlags;				// 
MAC_ADDR			MyMacAddr;				// 
SEND_RECEV_INFO		SendRecvInfo;			// 
ttUserInterface		OpenHandle;				// 
/********************************************/
ETHER_RECV_INFO		EtheRecvInfo;
/********************************************/
