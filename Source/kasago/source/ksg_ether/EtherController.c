/************************************************************************************************************************/
/*		EtherOpen				Open Function																			*/
/*		EtherClose				Close Function																			*/
/*		EtherSend				Send Function																			*/
/*		EtherReceive			Receive Function																		*/
/*		EtherFreeReceiveBuffer	Free Receive Buffer Function															*/
/*		EtherIoctl				Device IOCTL Function																	*/
/*		EtherGetPhyAddr;		Get Physical Address Function															*/
/************************************************************************************************************************/
/*		MACInit					Ether, Mac, Phy��RegisterInitialize����													*/
/************************************************************************************************************************/
/*		WritePHYReg				�w��PHY_Register(00 - 1F) <--- �w��Data(2Byte)��Write����								*/
/*		ReadPHYReg				�w��PHY_Register(00 - 1F)���� ---> Data(2Byte)��Read����								*/
/*		WriteReg				�w��SFR_Register(CPU_Access���Address) <--- �w��Data(2Byte)��Write����					*/
/*		ReadReg					�w��SFR_Register(CPU_Access���Address) ---> Data(2Byte)��Read����						*/
/*		BitSetReg				�w��SFR_Register(CPU_Access���Address)_�w��Bit <---	1��Write����					*/
/*		BitClearReg				�w��SFR_Register(CPU_Access���Address)_�w��Bit <---	0��Write����					*/
/************************************************************************************************************************/
/*		FillMemory																										*/
/*		WriteMemory				�w�肳�ꂽSRAMorSFRaddress(CPU_Access���Address)�Ɏw�肳�ꂽData��Byte����Write����	*/
/*		ReadMemory				�w�肳�ꂽSRAMorSFRaddress(CPU_Access���Address)���w�肳�ꂽData��Byte����Read����	*/
/*		WriteMemWindow			�w�肳�ꂽWindowRegister�ɑ΂��Ďw�肳�ꂽData���w��Byte����Write����					*/
/*								REG_EGPDATA���ByteRead����(REG_EGPDATA = EGPWRPT��Address)								*/
/*		ReadMemWindow			�w�肳�ꂽWindowRegister�ɑ΂��Ďw�肳�ꂽData���w��Byte����Read����					*/
/*								REG_ERXDATA���ByteRead����(REG_ERXDATA = REG_ERXRDPT��Address)							*/
/************************************************************************************************************************/
/*		SystemReset																										*/
/*		Ether_ReadAllRegster																							*/
/*		Ether_ReadSram																									*/
/*		Excep_IRQ15																										*/
/************************************************************************************************************************/
//#include "Ether.h"
#include "extern_Etherdata.h"
#include <stdlib.h>			// for srand, rand
//#include <trsocket.h>		// for kasago
#include	"system.h"
#include	"prm_tbl.h"	

/****************************************/
/*	ProtoType of Internal functions		*/
/****************************************/
/****************************************************/
int EtherOpen(ttUserInterface interfaceHandle);
int EtherClose(ttUserInterface interfaceHandle);
int EtherSend(ttUserInterface interfaceHandle, char TM_FAR *dataPtr, int dataLength, int flag);
int EtherReceive(ttUserInterface interfaceHandle, char TM_FAR **dataPtr, int TM_FAR *dataLength, ttUserBufferPtr userBufferHandlePtr);
int EtherFreeReceiveBuffer(ttUserInterface interfaceHandle, char TM_FAR *dataPtr);
int EtherIoctl(ttUserInterface interfaceHandle, int flag, void TM_FAR *optionPtr, int optionLen);
int EtherGetPhyAddr(ttUserInterface interfaceHandle, char TM_FAR *physicalAddress);
//USHT TestReadReg(USHT *pRegAdr);
/****************************************************/
void 	MACInit(void);
void 	Ether_DTinit(void);
/****************************************************/
void 	WritePHYReg(UCHAR Register, USHT Data);
USHT 	ReadPHYReg(UCHAR Register);
void 	WriteReg(ULNG RegAdr, USHT wValue);
USHT 	ReadReg(ULNG RegAdr);
void 	BitSetReg(ULNG RegAdr, USHT wBitMask);
void 	BitClearReg(ULNG RegAdr, USHT wBitMask);
/****************************************************/
void 	FillMemory(ULNG Address, UCHAR FillData, USHT wLength);
void 	WriteMemory(ULNG wAddress, UCHAR *vData, USHT wLength);
void 	ReadMemory(ULNG wAddress, UCHAR *vData, USHT wLength);
void 	WriteMemWindow(UCHAR vWindowNo, UCHAR *vData, USHT wLength);
void 	ReadMemWindow(UCHAR vWindow, UCHAR *vData, USHT wLength, USHT wEndian);
/****************************************************/
void 	SystemReset(void);
void 	Ether_ReadAllRegster(void);
void 	Ether_ReadSram(void);
/****************************************************************************/
/*	devPtr->devOpenFuncPtr = drvOpenFuncPtr;								*/
/*	return		errorCode													*/
/****************************************************************************/
int EtherOpen(	ttUserInterface		interfaceHandle)
{
	if (interfaceHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	SendRecvInfo.k_OpenCnt++;
	OpenHandle = interfaceHandle;
	MACInit();
#if 1
	/****************************************/
	/*	SramArea(24576Byte) <- Initialize	*/
	/****************************************/
	FillMemory(	TOP_ADDR_SRAM,	// StartAddress(CPU_Access���Address)
				0x11,			// FillData
				0x6000);		// Byte��
#endif
	return(TM_DEV_OKAY);
}
/****************************************************************************/
/*	devPtr->devCloseFuncPtr = drvCloseFuncPtr;								*/
/*	return		errorCode													*/
/****************************************************************************/
int EtherClose(	ttUserInterface		interfaceHandle)
{
	if (interfaceHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	if (OpenHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	SendRecvInfo.k_CloseCnt++;
	OpenHandle = NULL;
	return(TM_DEV_OKAY);
}
/****************************************************************************/
/*	devPtr->devSendFuncPtr = drvSendFuncPtr;								*/
/*	IRQ15��1Packet���M������	--->	tfNotifyInterfaceIsr()��call����	*/
/*	return		errorCode													*/
/****************************************************************************/
int EtherSend(	ttUserInterface		interfaceHandle,
				char TM_FAR 		*dataPtr,
				int					dataLength,
				int					flag)
{
	USHT	SendLen;
//	USHT	cnt;
//	UCHAR	*pSend;
	UCHAR	aDummy[128];
	USHT	wETXST, wETXSTAT, wETXWIRE, wECON1;
//	USHT	wEGPWRPT, wEGPRDPT, wETXLEN, wEIE, wEIR, wMACON2;
	USHT	wESTAT;

	if (interfaceHandle == NULL) {
		return(TM_DEV_ERROR);		// HandleError(NULL)
	}
	if (OpenHandle == NULL) {
		return(TM_DEV_ERROR);		// Not Opend yet
	}
	SendRecvInfo.k_SendCnt++;
	/****************/
	/*	���M����	*/
	/****************/
	SendRecvInfo.SendByteCnt = (USHT)dataLength;	// ���񑗐M����Byte��(IRQ15��kasago�ɕԂ�)
	SendLen = (USHT)dataLength;
	if ((SendLen > ETHER_RECV_PKT_MAX) || (SendLen == 0)) {		// 1Packet_Max = 1518B(0x5ee)
		return(TM_DEV_ERROR);		// SenddingLengthError
	}
#if 0
	//	�����ő��MData�𑗐MData�������Ă݂�!!!
	if (SendLen <= 128) {
		pSend = (UCHAR *)dataPtr;
		for (cnt = 0; cnt < SendLen; cnt++) {
			aDummy[cnt] = *pSend++;
		}
	} else {
		for (cnt = 0; cnt < 128; cnt++) {
			aDummy[cnt] = cnt;
		}
	}
	//	���M�O�ɑ��M�֘A��Register�̓��e�����Ă݂�!!!
	wETXST 		= ReadReg(REG_ETXST);		// 
	wETXSTAT 	= ReadReg(REG_ETXSTAT);		// 
	/************************************************************************************************************************/
	/*	ETXSTAT_LATECOL(b10)	1 = A collision occurred after transmitting more than MACLCONH + 8 bytes.					*/
	/*								The last transmission was aborted. 														*/
	/*	ETXSTAT_MAXCOL(b<9)		1 = MACLCONL + 1 collisions occurred while transmitting the last packet.					*/
	/*								The last transmission was aborted. 														*/
	/*	ETXSTAT_EXDEFER(b8)		1 = The medium was busy with traffic from other nodes for more than 24,288 bit times.		*/
	/*								The last transmission was aborted. 														*/
	/*	ETXSTAT_DEFER(b7)		1 = The medium was busy with traffic from other nodes,										*/
	/*								so the MAC was forced to temporarily defer transmission of the last packet 				*/
	/*	ETXSTAT_CRCBAD(b4)		1 = The FCS field of the last packet transmitted did not match the CRC internally generated	*/
	/*								by the MAC during transmission 															*/
	/*	ETXSTAT_COLCNT3_0(b3-b0)	Transmit Collision Count Status bits													*/
	/************************************************************************************************************************/
	wETXWIRE 	= ReadReg(REG_ETXWIRE);		// 
#endif
#if 0
	pSend = (UCHAR *)dataPtr;
	for (cnt=0;	cnt<40; cnt++) {
		pSend++;
	}
	*pSend++ = 0x01;
	*pSend++ = 0x04;
#endif
	/************************************************************/
	/*	REG_EGPWRPT�� <--- ADDR_TXSTART(���MBuffer�擪Address)	*/
	/************************************************************/
	WriteReg(REG_EGPWRPT, ADDR_TXSTART);
	/************************************************/
	/*	���M����Data��Length��������				*/
	/************************************************/
	WriteReg(REG_ETXLEN, SendLen);
	/************************************************/
	/*	���M����Data��SRAM��REG_EGPDATA�ɏ�����		*/
	/************************************************/
	WriteMemWindow(	GP_WINDOW,			// REG_EGPDATA(8bit)
					(UCHAR*)dataPtr,	// Data��Pointer
					SendLen);			// ���MByte��
#if 0
	//	���M�O�ɑ��M�ݒ肵��Register�̓��e�����Ă݂�!!!
	wEGPWRPT 	= ReadReg(REG_EGPWRPT);		// 
	wETXLEN 	= ReadReg(REG_ETXLEN);		// 

	/************************************************************************************************************/
	/*	TX�͑��M����Ȃ̂�REG_EGPWRPT���g�p����Write������s���݂̂���REG_EGPRDPT���g�p���đ��MData�̊m�F���s��	*/
	/*	RX�͎�M����Ȃ̂�REG_ERXRDPT���g�p����Read������s���݂̂�REG_ERXWRPT���g�p���ď����ގ��͂Ȃ�			*/
	/************************************************************************************************************/
	//	�����ł�REG_EGPWRPT���珑���񂾒l������̂ŏ�����StartAddress�ł���ADDR_TXSTART��������
	WriteReg(REG_EGPRDPT, ADDR_TXSTART);
	//	���M�O�ɑ��M�ݒ肵��Data�̓��e�����Ă݂�!!!
	ReadMemWindow(	GP_WINDOW,		// REG_EGPDATA(8bit) = EGPRDPT���g�p����
					&aDummy[0],		// Data��Pointer
					SendLen,		// ��MByte��
					0);				// 
	wEGPRDPT 	= ReadReg(REG_EGPRDPT);	// REG_EGPDATA��Read�łȂ��̂�++����Ȃ��͂�!!!
	/****************************************************************************************************************/
	/*	kasago����FCS(CRC)�͕t������Ă��Ȃ��悤�Ȃ̂Ŗ{Layer�ŕt�������ݒ�ł��鎖���m�F����						*/
	/*	MACON2_TXCRCEN(b4)	Transmit CRC Enable bit(RW:1)															*/
	/*						1 = MAC appends a valid CRC to all frames transmitted regardless of the PADCFG bits.	*/
	/*							TXCRCEN must be set if the PADCFG bits specify that a valid CRC will be appended. 	*/
	/****************************************************************************************************************/
	/*	kasago����zero-padded�͕t������Ă���悤�Ȃ̂Ŗ{Layer�ŕt�����Ȃ��ݒ�ł��鎖���m�F����(110)				*/
	/*	MACON2_PADCFG2_0(b7-b5)	110 = No automatic padding of short frames 											*/
	/*							101 = MAC automatically detects VLAN protocol frames which have a 8100h type field	*/
	/*								  and automatically pad to 64 bytes.											*/
	/*								  If the frame is not a VLAN frame,												*/
	/*									it will be padded to 60 bytes. After padding, a valid CRC is appended. 		*/
	/****************************************************************************************************************/
	wMACON2		= ReadReg(REG_MACON2);	// 0x40d2�̂͂�(TXCRCEN(b4))
	wEIE 		= ReadReg(REG_EIE);		// 0x8058�̂͂�(INTIE(b15), PKTIE(b6), TXIE(b3))
	wEIR		= ReadReg(REG_EIR);		// PKTIF(b6), TXIF(b3), TXABTIF(b2), RXABTIF(b1), PCFULIF(b0)
	wECON1		= ReadReg(REG_ECON1);	// PKTDEC(b8), TXRTS(b1), RXEN(b0)
#endif
	//	ENC624J600�ő��M�̐擪Address�͂ǂ�����Ēm��̂�?
	WriteReg(REG_EGPRDPT, ADDR_TXSTART);
	WriteReg(REG_EGPWRPT, ADDR_TXSTART);
	/********************************************************/
	/*	Ethernet��Link���m������Ă����瑗�M�J�n			*/
	/********************************************************/
	wESTAT = ReadReg(REG_ESTAT);	// INT(b15), PHYLNK(b8), PKTCNT7_0(b7-b0)
	if(wESTAT & ESTAT_PHYLNK) {
		/********************/
		/*	Data���M!!!!!!	*/
		/********************/
		/****************************************/
		/*	TXRTS has been cleared by hardware	*/
		/****************************************/
		BitSetReg(REG_ECON1, ECON1_TXRTS);		// ECON1_TXRTS <- 1
	/************************************************/
	/*	0 = Ethernet link has not been established	*/
	/*	with a remote Ethernet partner				*/
	/************************************************/
	} else {
		return(TM_DEV_ERROR);		// Phylink not established Error
	}
#if 1
	wECON1		= ReadReg(REG_ECON1);	// PKTDEC(b8), TXRTS(b1), RXEN(b0)
#endif
	if (wETXST != 0)	aDummy[0] = 1;
	if (wETXSTAT != 0)	aDummy[0] = 2;
	if (wETXWIRE != 0)	aDummy[0] = 3;
	if (wECON1 != 0)	aDummy[0] = 4;
	if (aDummy != 0)	aDummy[0] = 5;
	return(TM_DEV_OKAY);
}
/****************************************************************************/
/*	devPtr->devRecvFuncPtr = drvRecvFuncPtr;								*/
/*	IRQ15��1Packet��M������	--->	tfNotifyInterfaceIsr()��call����	*/
/*	tfNotifyInterfaceIsr()��call�����EtherReceive()��call�����			*/
/*	return		errorCode													*/
/*	�{�֐�������IRQ15�Ŏ�M����Data��kasago�ɕԂ�							*/
/****************************************************************************/
int EtherReceive(	ttUserInterface		interfaceHandle,
					char TM_FAR 		**dataPtr,
					int TM_FAR 			*dataLength,
					ttUserBufferPtr		userBufferHandlePtr)
{
	if (interfaceHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	if (OpenHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	SendRecvInfo.k_ReceiveCnt++;
#if 0		// ethertest
	TickCntStop(0);		// 0-7	TickCnt_RSLT_Value[][]	debug
#endif
	/************************************/
	/*	��M����Data��kasago�ɕԂ�����	*/
	/*	��M������IRQ15�ōs���Ă���		*/
	/************************************/
	*dataPtr 			= (char TM_FAR *)&EtheRecvInfo.Buff[(EtheRecvInfo.PktFirstSaveBoxNo)&ETHER_RECV_MASK][0];
	*dataLength 		= EtheRecvInfo.ByteCnt[(EtheRecvInfo.PktFirstSaveBoxNo)&ETHER_RECV_MASK];

	// TEST �f�o�b�O�p
	if(( EtheRecvInfo.PktCnt - EtheRecvInfo.PktFirstSaveBoxNo) > ETHER_RECV_MAX ){
		EtheRecvInfo.PktCnt = EtheRecvInfo.PktCnt;
	}
	// TEST �f�o�b�O�p
	*userBufferHandlePtr= NULL;
	EtheRecvInfo.PktFirstSaveBoxNo++;	// for MultiRecvPackets in 1_irq
	return(TM_DEV_OKAY);
}
/****************************************************************************/
/*	devPtr->devFreeRecvFuncPtr = drvFreeRecvFuncPtr;						*/
/*	return		errorCode													*/
/*	�{�֐���Option�̈�KSG_Initialize()�ɓo�^���鎞��NULL�œo�^����			*/
/****************************************************************************/
int EtherFreeReceiveBuffer(	ttUserInterface		interfaceHandle,
							char TM_FAR 		*dataPtr)
{
	if (interfaceHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	if (OpenHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	SendRecvInfo.k_FreeCnt++;
	return(TM_DEV_OKAY);
}
/****************************************************************************/
/*	devPtr->devIoctlFuncPtr = drvIoctlFuncPtr;								*/
/****************************************************************************/
int EtherIoctl	(	ttUserInterface		interfaceHandle,
					int					flag,
					void TM_FAR 		*optionPtr,
					int 				optionLen)
{
	if (interfaceHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	if (OpenHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	SendRecvInfo.k_IoctlCnt++;
	return(TM_DEV_OKAY);
}
/****************************************************************************/
/*	devPtr->devGetPhysAddrFuncPtr = drvGetPhysAddrFuncPtr;					*/
/****************************************************************************/
int EtherGetPhyAddr(ttUserInterface		interfaceHandle,
					char TM_FAR 		*physicalAddress)
{
	if (interfaceHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	if (OpenHandle == NULL) {
		return(TM_DEV_ERROR);
	}
	SendRecvInfo.k_GetPhyCnt++;
	/****************************/
	/*	MyMacAddr.v[0-5]��Ԃ�	*/
	/****************************/
	*physicalAddress++ = MyMacAddr.v[1];	// 0x04
	*physicalAddress++ = MyMacAddr.v[0];	// 0x00
	*physicalAddress++ = MyMacAddr.v[3];	// 0x50
	*physicalAddress++ = MyMacAddr.v[2];	// 0xA3
	*physicalAddress++ = MyMacAddr.v[5];	// 0xB1
	*physicalAddress++ = MyMacAddr.v[4];	// 0x0A
	return(TM_DEV_OKAY);
}
/************************************************************************************************/
/*	REG_ETXST		Ethernet_TX_Start_Address(set/clr)					0x0000					*/
/*	REG_ETXLEN		Ethernet_TX_Length(set/clr)							0x0000					*/
/*	REG_ERXST		Ethernet_RX_Buffer_Start_Address(set/clr)			0x5340					*/
/*	REG_ERXHEAD		Ethernet_RX_Head_Pointer(R)							0x5340					*/
/*	REG_ERXTAIL		Ethernet_RX_Tail_Pointer(set/clr)					0x5FFE					*/
/*	REG_ETXSTAT		Ethernet_TX_Status_Register(R)						0x0000=NotUsed			*/
/*	REG_ETXWIRE		Ethernet_TXByte_CountOnWire(R)						0x0000=NotUsed			*/
/*	REG_EUDAST		Ethernet_User_Defined_Area_Start_Pointer(set/clr)	0x0000					*/
/*	REG_EUDAND		Ethernet_User_Defined_Area_End_Pointer(set/clr)		0x5FFF					*/
/*		below is Window_Register																*/
/*	REG_EGPDATA		Ethernet_General_Purpose_Data_Window_Register_7_0	0x--??					*/
/*	REG_ERXDATA		Ethernet_RX_Data_Window_Register7_0					0x--??					*/
/*	REG_EUDADATA	Ethernet_User_Defined_Area_Data_Window_Register7_0	0x--??					*/
/*		below is Window_Pointer																	*/
/*	REG_EGPRDPT		Ethernet_General_Purpose_Window_Read_Pointer		0x05FA=NotUsed			*/
/*	REG_EGPWRPT		Ethernet_General_Purpose_Window_Write_Pointer		0x0000					*/
/*	REG_ERXRDPT		Ethernet_RX_Window_Read_Pointer						0x05FA					*/
/*	REG_ERXWRPT		Ethernet_RX_Window_Write_Pointer					0x0000=NotUsed			*/
/*	REG_EUDARDPT	Ethernet_UDA_Window_Read_Pointer					0x05FA=NotUsed			*/
/*	REG_EUDAWRPT	Ethernet_UDA_Window_Write_Pointer					0x0000=NotUsed			*/
/*----------------------------------------------------------------------------------------------*/
/*	REG_ESTAT		Ethernet_Status_Register(R)							0x0000					*/
/*	REG_ECON1		Ethernet_Control_Register1(set/clr)					0x0000					*/
/*	REG_ECON2		Ethernet_Control_Register2(set/clr)					0xCB00					*/
/*	REG_EPAUS		Ethernet_Pause_Timer_Value(set/clr)					0x1000=NotUsed			*/
/*	REG_ERXWM		Ethernet_RX_Watermark_register(set/clr)				0x100F=NotUsed			*/
/*	REG_EIE			Ethernet_Interrupt_Enable_Register(set/clr)			0x8010					*/
/*	REG_EIR			Ethernet_Intrrupt_Flag_Register(set/clr)			0x0A00					*/
/*	REG_EIDLED		Ethernet_ID_Status_Led_Control_Register(set/clr)	0x26??=NotUsed			*/
/*----------------------------------------------------------------------------------------------*/
/*	REG_EDMAST		Ethernet_DMA_Start_Address(set/clr)					0x0000					*/
/*	REG_EDMALEN		Ethernet_DMA_Length(set/clr)						0x0000					*/
/*	REG_EDMADST		Ethernet_DMA_Destination_Address(set/clr)			0x0000					*/
/*	REG_EDMACS		Ethernet_DMA_Checksum(set/clr)						0x0000					*/
/*----------------------------------------------------------------------------------------------*/
/*	REG_EHT1		Ethernet_Hash_Table_Filter15_0bit(set/clr)			0x0000=NotUsed			*/
/*	REG_EHT2		Ethernet_Hash_Table_Filter31_16bit(set/clr)			0x0000=NotUsed			*/
/*	REG_EHT3		Ethernet_Hash_Table_Filter47_32bit(set/clr)			0x0000=NotUsed			*/
/*	REG_EHT4		Ethernet_Hash_Table_Filter63_48bit(set/clr)			0x0000=NotUsed			*/
/*	REG_EPMM1		Ethernet_Pattern_Match_Filter_Mask1(set/clr)		0x0000=NotUsed			*/
/*	REG_EPMM2		Ethernet_Pattern_Match_Filter_Mask2(set/clr)		0x0000=NotUsed			*/
/*	REG_EPMM3		Ethernet_Pattern_Match_Filter_Mask3(set/clr)		0x0000=NotUsed			*/
/*	REG_EPMM4		Ethernet_Pattern_Match_Filter_Mask4(set/clr)		0x0000=NotUsed			*/
/*	REG_EPMCS		Ethernet_Pattern_Match_Filter_Checksum(set/clr)		0x0000=NotUsed			*/
/*	REG_EPMO		Ethernet_Pattern_Match_Filter_Offset(set/clr)		0x0000=NotUsed			*/
/*	REG_ERXFCON		Ethernet_RX_Filter_Control_Register(set/clr)		0x0059=NotUsed			*/
/*----------------------------------------------------------------------------------------------*/
/*	REG_MACON1		MAC_Control_Register1								0x?00D=NotUsed			*/
/*	REG_MACON2		MAC_Control_Register2								0x40B2					*/
/*	REG_MABBIPG		MAC_BackToBack_Inter_Packet_Gap_Register			0x0012					*/
/*	REG_MAIPG		MAC_Inter_Packet_Gap_Register						0x0C12=NotUsed			*/
/*	REG_MACLCON		MAC_Colision_Control_Register						0x370F=NotUsed			*/
/*	REG_MAMXFL		MAC_Maximum_Frame_Length							0x05EE=NotUsed			*/
/*	REG_MAADR3		MAC_Address_Byte6_15_0								0x????					*/
/*	REG_MAADR2		MAC_Address_Byte6_31_16								0x??A3					*/
/*	REG_MAADR1		MAC_Address_Byte6_47_32								0x0400					*/
/*----------------------------------------------------------------------------------------------*/
/*	REG_MICMD		MII_Management_Command_Register						0x--00					*/
/*	REG_MIREGADR	MII_Management_Address_Register(b8 = always_1)		0x0100					*/
/*	REG_MIWR		MII_Management_Write_Data							0x0000					*/
/*	REG_MIRD		MII_Management_Read_Data							0x0000					*/
/*	REG_MISTAT		MII_Management_Status_Register						0x--00					*/
/*	PHY_PHCON1		PHY_Control_Register1								0x1000					*/
/*	PHY_PHSTAT1		PHY_Status_Register1								0x7809					*/
/*	PHY_PHSTAT2		PHY_Status_Register2								0x??0?					*/
/*	PHY_PHSTAT3		PHY_Status_Register3								0x0040					*/
/*	PHY_PHANA		PHY_Auto_Negotiation_Advertisement_Register			0x01E1					*/
/*	PHY_PHANLPA		PHY_Auto_Negotiation_Link_Partner_Ability_Register	0x????					*/
/*	PHY_PHANE		PHY_Auto_Negotiation_Expansion_Register				0x0000					*/
/*	PHY_PHCON2		PHY_Control_Register2								0x0002					*/
/*	Ether, Mac, Phy��RegisterInitialize����														*/
/************************************************************************************************/
void MACInit(void)
{
	USHT 	wData;
	USHT 	wEIE, wMACON1, wMACON2;
	USHT	wESTAT;
	USHT 	cnt, wDummy;
	unsigned long	prm_41_0001_3;
	unsigned long	prm_41_0001_456;
	USHT	PhyPhcon1;		// 16-bit unsigned
	
#if 1
	Ether_ReadAllRegster();
#endif
	/********************************/
	/*	Perform a reliable reset	*/
	/*		ECON2_ETHRST <- 1		*/
	/********************************/
	SystemReset();
	/********************************************************************/
	/*	Initialize RX tracking variables and other control state flags	*/
	/********************************************************************/
	wNextRxPacketPointer 		= ADDR_RXSTART;	// �ŏ���Packet�͂��̈ʒu�����M���s��(ERXST�Ɠ���)
	wCurrentRxPacketPointer 	= 0x0000;
	EtherFlags.bWasDiscarded 	= 1;		// MACDiscardRx()��set�����, MACGetHeader()��0�Ȃ�MACDiscardRx()��call����
	EtherFlags.PoweredDown 		= 0x0000;	// MACPowerDown()��set�����
	EtherFlags.CryptoEnabled 	= 0x0000;	// set�Ȃ�
	for (cnt = 0; cnt < ETHER_RECV_MAX; cnt++) {
		EtheRecvInfo.ByteCnt[cnt] = 0;
	}
	EtheRecvInfo.PktCnt				= 0;
	EtheRecvInfo.PktFirstSaveBoxNo	= 0;		// for MultiRecvPackets in 1_irq
	//EtheRecvInfo.Buff[8][1518]
	/****************************************************************************************************************************************************/
	/*	EtherControler Initialize																														*/
	/****************************************************************************************************************************************************/
	/*	REG_ETXST		0x0000				Ethernet_TX_Start_Address																					*/
	/*	REG_ETXLEN		0x0000				Ethernet_TX_Length																							*/
	/*	REG_ERXST		0x5340				Ethernet_RX_Buffer_Start_Address																			*/
	/*	REG_ERXHEAD		0x5340				Ethernet_RX_Head_Pointer(R)																					*/
	/*	REG_ERXTAIL		0x5FFE				Ethernet_RX_Tail_Pointer																					*/
	/*	REG_ETXSTAT		0x0000				Ethernet_TX_Status_Register																					*/
	/*	0		ETXSTAT_LATECOL(b10)		1 = A collision occurred after transmitting more than MACLCONH + 8 bytes.The last transmission was aborted. */
	/*	0		ETXSTAT_MAXCOL(b9)(*1)		1 = MACLCONL + 1 collisions occurred while transmitting the last packet.The last transmission was aborted. 	*/
	/*	0		ETXSTAT_EXDEFER(b8(*1))		1 = The medium was busy with traffic from other nodes for more than 24288 bit times.						*/
	/*	0		ETXSTAT_DEFER(b7)(*1)		1 = The medium was busy with traffic from other nodes,														*/
	/*	0		ETXSTAT_CRCBAD(b4)			1 = The FCS field of the last packet transmitted did not match the CRC internally generated					*/
	/*											by the MAC during transmission																			*/
	/*	0000	ETXSTAT_COLCNT3_0(b3-b0)(*1)Number of collisions that occurred while transmitting the last packet. 										*/
	/*			*1: Applicable in Half-Duplex mode only; collisions and deferrals are not possible in Full-Duplex mode. 								*/
	/*	REG_ETXWIRE		0x0000				Ethernet_TXByte_CountOnWire																					*/
	/*	REG_EUDAST		0x0000				Ethernet_User_Defined_Area_Start_Pointer																	*/
	/*	REG_EUDAND		0x5FFF				Ethernet_User_Defined_Area_End_Pointer																		*/
	/*	REG_EGPDATA		0x--??				Ethernet_General_Purpose_Data_Window_Register_7_0															*/
	/*	REG_ERXDATA		0x--??				Ethernet_RX_Data_Window_Register7_0																			*/
	/*	REG_EUDADATA	0x--??				Ethernet_User_Defined_Area_Data_Window_Register7_0															*/
	/*	REG_EGPRDPT		0x05FA				Ethernet_General_Purpose_Window_Read_Pointer																*/
	/*	REG_EGPWRPT		0x0000				Ethernet_General_Purpose_Window_Write_Pointer																*/
	/*	REG_ERXRDPT		0x05FA				Ethernet_RX_Window_Read_Pointer																				*/
	/*	REG_ERXWRPT		0x0000				Ethernet_RX_Window_Write_Pointer																			*/
	/*	REG_EUDARDPT	0x05FA				Ethernet_UDA_Window_Read_Pointer																			*/
	/*	REG_EUDAWRPT	0x0000				Ethernet_UDA_Window_Write_Pointer																			*/
	/*	REG_ESTAT		0x0000				Ethernet_Status_Register																					*/
	/*	0		ESTAT_INT(b15)				1 = One of the EIR bits is set and enabled by the EIE register.												*/
	/*											If INTIE (EIE<15>) is set, the INT pin is also driven low. 												*/
	/*	0		ESTAT_FCIDLE(b14)			1 = Internal flow control state machine is Idle. It's safe to change FCOP(ECON1<7:6>), FULDPX(MACON2<0>)	*/
	/*	0		ESTAT_RXBUSY(b13)			1 = Receive logic is currently receiving a packet.															*/
	/*	0->1	ESTAT_CLKRDY(b12)			1/0 = Normal operation/Internal Ethernet clocks are not running and stable yet.								*/
	/*	0->1	ESTAT_RSTDONE(b11)			1 = maybe Reset done																						*/
	/*	0		ESTAT_PHYDPX(b10)			1/0 = PHY is operating in Full-Duplex mode/Half-Duplex mode 												*/
	/*	0->1	ESTAT_PHYRDY(b<9)			1 = maybe PhysicalLayer is ready																			*/
	/*	0		ESTAT_PHYLNK(b8)			1 = Ethernet link has been established with a remote Ethernet partner 										*/
	/*	0000000	ESTAT_PKTCNT7_0(b7-b0)		Number of complete packets that are saved in the RX buffer and ready for software processing.				*/
	/*										Set the PKTDEC(ECON1<8>) bit to decrement this field. 														*/
	/*	REG_ECON1		0x0000				Ethernet_Control_Register1																					*/
	/*	0		ECON1_MODEXST(b15)			1 = Modular exponentiation calculation started/busy; automatically cleared by hardware when done 			*/
	/*	0		ECON1_HASHEN(b14)			1 = MD5/SHA-1 hashing engine enabled. Data written to the hashing engine by the DMA is added to the hash. 	*/
	/*	0		ECON1_HASHOP(b13)			1 = MD5/SHA-1 hash engine loads the Initial Value (IV) from the hash memory.								*/
	/*	0		ECON1_HASHLST(b12)			1 = The next DMA transfer to the hash engine completes the hash.											*/
	/*	0		ECON1_AESST(b11)			1 = AES encrypt/decrypt operation started/busy; automatically cleared by hardware when done 				*/
	/*	0		ECON1_AESOP1_0(b10-b9)		AES Ope Control	10 = ECB/CBC decrypt, 01 = CBC/CFB encrypt, 00 = ECB/CFB/OFB encrypt or key initialization 	*/
	/*	0		ECON1_PKTDEC(b8)			1 = Decrement PKTCNT (ESTAT<7:0>) bits by one. Hardware immediately clears PKTDEC to '0',					*/
	/*											allowing back-to-back decrement operations. 															*/
	/*	00		ECON1_FCOP1_0(b7-b6)		00 = Flow control disabled/Idle(it's depend on FULDPX(MACON2<0>) = 1 or 0)									*/
	/*	0		ECON1_DMAST(b5)				1 = DMA is started/busy; automatically cleared by hardware when done 										*/
	/*	0		ECON1_DMACPY(b4)			1 = DMA copies data to memory location at EDMADST 															*/
	/*	0		ECON1_DMACSSD(b3)			1 = DMA checksum operations are initially seeded by the one's complement of the checksum contained in EDMACS*/
	/*	0		ECON1_DMANOCS(b2)			1 = DMA does not compute checksums; EDMACS remains unchanged 												*/
	/*	0		ECON1_TXRTS(b1)				1 = Transmit an Ethernet frame; automatically cleared by hardware when done 								*/
	/*	0		ECON1_RXEN(b0)				1 = Packets which pass the current RX filter configuration are written to the receive buffer 				*/
	/*	REG_ECON2		0xCB00				Ethernet_Control_Register2																					*/
	/*	1		ECON2_ETHEN(b15)			1 = Device enabled (normal operation) 																		*/
	/*	1		ECON2_STRCH(b14)			1 = Stretch transmit, receive and collision events on LEDA and LEDB to 50 ms 								*/
	/*	0		ECON2_TXMAC(b13)			1 = MAADR1-MAADR6 registers are automatically inserted														*/
	/*											into the source address field of all transmitted packets 												*/
	/*	0		ECON2_SHA1MD5(b12)			1 = Hashing engine computes a SHA-1 hash 																	*/
	/*	1011	ECON2_COCON3_0(b11-b8)		CLKOUT Frequency Control	1011 = 4.000 MHz nominal ((4 * FOSC)/25) 										*/
	/*	0		ECON2_AUTOFC(b7)			1 = Automatic flow control enabled 																			*/
	/*	0		ECON2_TXRST(b6)				1 = Transmit logic is held in Reset. TXRTS(ECON1<1>) is automatically cleared by hardware					*/
	/*	0		ECON2_RXRST(b5)				1 = Receive logic is held in Reset. RXEN(ECON1<0>) is automatically cleared by hardware						*/
	/*	0->1	ECON2_ETHRST(b4)			1 = All TX, RX, MAC, PHY, DMA, modular exponentiation, hashing and AES logic, registers are reset.			*/
	/*	00		ECON2_MODLEN1_0(b3-b2)		Modular Exponentiation Length	10 = 1024-bit modulus and operands, 01 = 768-bit , 00 = 512-bit				*/
	/*	00		ECON2_AESLEN1_0(b1-b0)		10 = 256-bit AES key, 01 = 192-bit AES key, 00 = 128-bit AES key											*/
	/*			*2: Reset value on POR events only. All other Resets leave these bits unchanged. 														*/
	/*	REG_EPAUS		0x1000				Ethernet_Pause_Timer_Value																					*/
	/*	REG_ERXWM		0x100F				Ethernet_RX_Watermark_register																				*/
	/*			ERXWM_RXFWM7_0(b15-b8)		When this number of 96-byte blocks is consumed in the receive buffer,										*/
	/*	00010000							the device considers its receive buffer to be full and initiates flow control.								*/
	/*										Use the default value of 16 if the full threshold is to be set at 1536 bytes. 								*/
	/*			ERXWM_RXEWM7_0(b7-b0)		When the number of occupied 96-byte blocks falls below this level, 											*/
	/*	00001111							the device considers its receive buffer to be empty enough to receive more data.							*/
	/*										Use the default value of 15 if the empty threshold is to be set at 1440 bytes. 								*/
	/*	REG_EIE			0x8010				Ethernet_Interrupt_Enable_Register																			*/
	/*	1		EIE_INTIE(b15)				1 = INT pin is controlled by the INT status bit (ESTAT<15>) 												*/
	/*	0		EIE_MODEXIE(b14)			Modular Exponentiation Interrupt Enable bit(RW:0)															*/
	/*	0		EIE_HASHIE(b13)				MD5/SHA-1 Hash Interrupt Enable bit(RW:0)																	*/
	/*	0		EIE_AESIE(b12)				AES Encrypt/Decrypt Interrupt Enable bit(RW:0)																*/
	/*	0		EIE_LINKIE(b11)				PHY Link Status Change Interrupt Enable bit(RW:0)															*/
	/*	0		EIE_PRDYIE(b10)				(?)																											*/
	/*	0		EIE_PKTIE(b6)				RX Packet Pending Interrupt Enable bit(RW:0)																*/
	/*	0		EIE_DMAIE(b5)				DMA Interrupt Enable bit(RW:0)																				*/
	/*	0		EIE_TXIE(b3)				Transmit Done Interrupt Enable bit(RW:0)																	*/
	/*	0		EIE_TXABTIE(b2)				Transmit Abort Interrupt Enable bit(RW:0)																	*/
	/*	0		EIE_RXABTIE(b1)				Receive Abort Interrupt Enable bit(RW:0)																	*/
	/*	0		EIE_PCFULIE(b0)				Packet Counter Full Interrupt Enable bit(RW:0)																*/
	/*	REG_EIR			0x0A00				Ethernet_Intrrupt_Flag_Register																				*/
	/*	0		EIR_CRYPTEN(b15)			1 = All cryptographic engine modules are enabled 															*/
	/*	0		EIR_MODEXIF(b14)			1 = Modular exponentiation calculation complete 															*/
	/*	0		EIR_HASHIF(b13)				1 = MD5/SHA-1 hash operation complete 																		*/
	/*	0		EIR_AESIF(b12)				1 = AES encrypt/decrypt operation complete 																	*/
	/*	1		EIR_LINKIF(b11)				1 = PHY Ethernet link status has changed. Read PHYLNK (ESTAT<8>) to determine the current state. 			*/
	/*	0		EIR_PRDYIF(b10)				(?)																											*/
	/*	0		EIR_PKTIF(b6)				1 = One or more RX packets have been saved and are ready for software processing.							*/
	/*										The PKTCNT<7:0> (ESTAT<7:0>) bits are non-zero. To clear this flag,											*/
	/*										decrement the PKTCNT bits to zero by setting PKTDEC (ECON1<8>). 											*/
	/*	0		EIR_DMAIF(b5)				1 = DMA copy or checksum operation complete 																*/
	/*	0		EIR_TXIF(b3)				1 = Packet transmission has completed. TXRTS (ECON1<1>) has been cleared by hardware. 						*/
	/*	0		EIR_TXABTIF(b2)				1 = Packet transmission has been aborted due to an error. Read the ETXSTAT register to determine the cause	*/
	/*											TXRTS (ECON1<1>) has been cleared by hardware. 															*/
	/*	0		EIR_RXABTIF(b1)				1 = An RX packet was dropped because there is insufficient space											*/
	/*											in the RX buffer to store the complete packet or the PKTCNT field is saturated at FFh 					*/
	/*	0		EIR_PCFULIF(b0)				1 = PKTCNT field has reached FFh.																			*/
	/*											Software must decrement the packet counter to prevent the next RX packet from being dropped. 			*/
	/*	REG_EIDLED		0x26??				Ethernet_ID_Status_Led_Control_Register																		*/
	/*	0010	EIDLED_LACFG3_0(b15-b12)	0010 = Display link state; pin is driven high when linked 													*/
	/*	0110	EIDLED_LBCFG3_0(b11-b8)		0110 = Display transmit and receive events;																	*/
	/*												pin is driven high while a packet is either being received or transmitted 							*/
	/*	???		EIDLED_DEVID2_0(b7-b5)		001 = ENC624J600 family device 																				*/
	/*	?????	EIDLED_REVID4_0(b4-b0)		Silicon Revision ID bits Indicates current silicon revision<4>:(R:?)										*/
	/*	REG_ERXFCON		0x0059				Ethernet_RX_Filter_Control_Register																			*/
	/*	0		ERXFCON_HTEN(b15)			1 = Accept packets having a hashed destination address that points to a set bit in the Hash Table(*1) 		*/
	/*	0		ERXFCON_MPEN(b14)			1 = Accept packets containing a Magic Packet pattern for the local MAC address(*1) 							*/
	/*	0		ERXFCON_NOTPM(b12)			1 = Pattern Match checksum mismatch required for a successful Pattern Match 								*/
	/*	0000	ERXFCON_PMEN3_0(b11-b8)		Pattern Match Collection Filter Enable bits<3>:(RW:0)	0000 = Filter disabled								*/
	/*	0		ERXFCON_CRCEEN(b7)			1 = Packets with an invalid CRC will be accepted, regardless of all other filter settings 					*/
	/*	1		ERXFCON_CRCEN(b6)			1 = Packets with an invalid CRC will be discarded(*2) 														*/
	/*	0		ERXFCON_RUNTEEN(b5)			1 = Accept packets that are 63 bytes or smaller, regardless of all other filter settings 					*/
	/*	1		ERXFCON_RUNTEN(b4)			1 = Discard packets that are 63 bytes or smaller(*2) 														*/
	/*	1		ERXFCON_UCEN(b3)			1 = Accept packets with a destination address matching the local MAC address(*1) 							*/
	/*	0		ERXFCON_NOTMEEN(b2)			1 = Accept packets with a Unicast destination address that does not match the local MAC address(*1) 		*/
	/*	0		ERXFCON_MCEN(b1)			1 = Accept packets with a Multicast destination address(*1) 												*/
	/*	1		ERXFCON_BCEN(b0)			1 = Accept packets with a Broadcast destination address of FF-FF-FF-FF-FF-FF(*1) 							*/
	/*			*1: This filtering decision can be overridden by the CRC Error Rejection filter															*/
	/*				and Runt Error Rejection filter decisions, if enabled, by CRCEN or RUNTEN. 															*/
	/*			*2: This filtering decision can be overridden by the CRC Error Collection filter														*/
	/*				and Runt Error Collection filter decisions, if enabled, by CRCEEN or RUNTEEN. 														*/
	/****************************************************************************************************************************************************/
	//	--->	EtherControler��Reset�l�ȊO�̒l��ݒ肷�鎞�͂����ɋL�q����
	/********************************************************************/
	/*	Set up TX/RX/UDA buffer addresses								*/
	/*	REG_ERXST�̐ݒ���s����REG_ERXWRPT�ɂ������l�������ݒ肳���	*/
	/*	REG_ERXRDPT��Soft�Őݒ肷��K�v������							*/
	/********************************************************************/
	WriteReg(REG_ETXST, 	ADDR_TXSTART);			// Top of Tx_Buffer
	WriteReg(REG_ERXST, 	ADDR_RXSTART);			// Top of Rx_Buffer(Open���ɂ̂ݐݒ肳���)
	WriteReg(REG_ERXTAIL, 	SIZE_SRAM_MAX - 2);		// 0x5ffe(MACDiscardRx�ōĐݒ肳���)
	WriteReg(REG_EUDAST, 	SIZE_SRAM_MAX);			// 0x6000
	WriteReg(REG_EUDAND, 	SIZE_SRAM_MAX + 1);		// 0x6000+1
	WriteReg(REG_EGPRDPT, 	0x05FA);				// PowerOnResetValue
	WriteReg(REG_ERXRDPT, 	0x05FA);				// PowerOnResetValue
	WriteReg(REG_EUDARDPT, 	0x05FA);				// PowerOnResetValue
	/************************************************************************************************************/
	/*	TX�͑��M����Ȃ̂�REG_EGPWRPT���g�p����Write������s���݂̂���REG_EGPRDPT���g�p���đ��MData�̊m�F���s��	*/
	/*	RX�͎�M����Ȃ̂�REG_ERXRDPT���g�p����Read������s���݂̂�REG_ERXWRPT���g�p���ď����ގ��͂Ȃ�			*/
	/************************************************************************************************************/
	// ---> ������REG_EGPRDPT��ݒ肵�Ă��ǂ������M�����Ŋm�F���鎞�ɐݒ肷�鎖�Ƃ���
	/****************************************************************************************************************************************************/
	/*	DMAControler Initialize																															*/
	/****************************************************************************************************************************************************/
	/*	REG_EDMAST		0x0000			Ethernet_DMA_Start_Address																						*/
	/*	REG_EDMALEN		0x0000			Ethernet_DMA_Length																								*/
	/*	REG_EDMADST		0x0000			Ethernet_DMA_Destination_Address																				*/
	/*	REG_EDMACS		0x0000			Ethernet_DMA_Checksum																							*/
	/****************************************************************************************************************************************************/
	//	--->	DMAControler��Reset�l�ȊO�̒l��ݒ肷�鎞�͂����ɋL�q����

	/****************************************************************************************************************************************************/
	/*	MacSubLayer Initialize																															*/
	/****************************************************************************************************************************************************/
	/*	REG_MACON1		0x?00D			MAC_Control_Register1																							*/
	/*	0		MACON1_LOOPBK(b4)		1 = Transmitted packets are looped back inside the MAC before reaching the PHY									*/
	/*	1		MACON1_RXPAUS(b2)		1 = Inhibit transmissions when pause control frames are received (normal operation)								*/
	/*	0		MACON1_PASSALL(b1)		1 = Control frames received by the MAC are written into the receive buffer if not filtered out 					*/
	/*	REG_MACON2		0x40B2			MAC_Control_Register2																							*/
	/*	1		MACON2_DEFER(b14)		1 = When the medium is occupied, the MAC will wait indefinitely for it to become free							*/
	/*										when attempting to transmit (use this setting for IEEE 802.3 compliance) 									*/
	/*	0		MACON2_BPEN(b13)		1 = After incidentally causing a collision during back pressure, the MAC immediately begins retransmitting 		*/
	/*	0		MACON2_NOBKOFF(b12)		1 = After any collision, the MAC immediately begins retransmitting 												*/
	/*	101		MACON2_PADCFG2_0(b7-b5)	101 = MAC automatically detects VLAN protocol frames which have a 8100h type field								*/
	/*										 and automatically pad to 64 bytes.																			*/
	/*										If the frame is not a VLAN frame, it will be padded to 60 bytes. After padding, a valid CRC is appended. 	*/
	/*	1		MACON2_TXCRCEN(b4)		1 = MAC appends a valid CRC to all frames transmitted regardless of the PADCFG bits.							*/
	/*										TXCRCEN must be set if the PADCFG bits specify that a valid CRC will be appended. 							*/
	/*	0		MACON2_PHDREN(b3)		1 = Frames presented to the MAC contain a 4-byte proprietary header which is not used, when calculating the CRC */
	/*	0		MACON2_HFRMEN(b2)		1 = Frames of any size will be allowed to be transmitted and received											*/
	/*	0		MACON2_FULDPX(b0)		1 = MAC operates in Full-Duplex mode. For proper operation, the PHY must also be set to Full-Duplex mode. 		*/
	/*	REG_MABBIPG		0x0012			MAC_BackToBack_Inter_Packet_Gap_Register																		*/
	/*	0010010	MABBIPG_BBIPG6_0(b6-b0)	Back-to-Back Inter-Packet Gap Delay Time Control bits(RW) = 0x12												*/
	/*	REG_MAIPG		0x0C12			MAC_Inter_Packet_Gap_Register																					*/
	/*	0010010	MAIPG_IPG6_0(b6-b0)		Non Back-to-Back Inter-Packet Gap Delay Time Control bits<6>:(RW:0) = 0x12										*/
	/*	REG_MACLCON		0x370F			MAC_Colision_Control_Register																					*/
	/*	1111	MACLCON_MAXRET3_0(b3-b0)Maximum retransmission attempts the MAC will make before aborting a packet due to excessive collisions. 		*/
	/*	REG_MAMXFL		0x05EE			MAC_Maximum_Frame_Length(1518B)																					*/
	/*	REG_MAADR3		0x????			MAC_Address_Byte6_15_0																							*/
	/*	REG_MAADR2		0x??A3			MAC_Address_Byte6_31_16																							*/
	/*	REG_MAADR1		0x0400			MAC_Address_Byte6_47_32																							*/
	/****************************************************************************************************************************************************/
	//	--->	MacSubLayer��Reset�l�ȊO�̒l��ݒ肷�鎞�͂����ɋL�q����
	/****************************************************************************************************************/
	/*	kasago����FCS(CRC)�͕t������Ă��Ȃ��悤�Ȃ̂Ŗ{Layer�ł͕t�������ݒ�										*/
	/*	MACON2_TXCRCEN(b4)	Transmit CRC Enable bit(RW:1)															*/
	/*						1 = MAC appends a valid CRC to all frames transmitted regardless of the PADCFG bits.	*/
	/*							TXCRCEN must be set if the PADCFG bits specify that a valid CRC will be appended. 	*/
	/****************************************************************************************************************/
	/*	kasago����zero-padded�͕t������Ă���悤�Ȃ̂Ŗ{Layer�ł͕t�����Ȃ��ݒ�(110)								*/
	/*	MACON2_PADCFG2_0(b7-b5)	110 = No automatic padding of short frames 											*/
	/*							101 = MAC automatically detects VLAN protocol frames which have a 8100h type field	*/
	/*								  and automatically pad to 64 bytes.											*/
	/*								  If the frame is not a VLAN frame,												*/
	/*									it will be padded to 60 bytes. After padding, a valid CRC is appended. 		*/
	/****************************************************************************************************************/
//	WriteReg(REG_MACON2, 	MACON2_DEFER	||
//							MACON2_PADCFG2	||
//							MACON2_PADCFG1	||
//							MACON2_TXCRCEN	||
//							0x0002	);
	WriteReg(REG_MACON2, 	0x40d3);
	wMACON2		= ReadReg(REG_MACON2);

#if 0
	/********************************************************************************/
	/*	Use ENCx24J600 preprogrammed MAC address, if AppConfig is not already set	*/
	/********************************************************************************/
	//	MacAddress�̐ݒ�(MicroChip�̏o��Data��������Ă���)
	if(((AppConfig.MyMACAddr.v[0] == 0x00u) &&
		(AppConfig.MyMACAddr.v[1] == 0x04u) &&
		(AppConfig.MyMACAddr.v[2] == 0xA3u) &&
		(AppConfig.MyMACAddr.v[3] == 0x00u) &&
		(AppConfig.MyMACAddr.v[4] == 0x00u) &&
		(AppConfig.MyMACAddr.v[5] == 0x00u)) ||
		((	AppConfig.MyMACAddr.v[0] |
			AppConfig.MyMACAddr.v[1] |
			AppConfig.MyMACAddr.v[2] |
			AppConfig.MyMACAddr.v[3] |
			AppConfig.MyMACAddr.v[4] |
			AppConfig.MyMACAddr.v[5]) == 0x00u))
	{			//	���݂�REG_MAADR1-3�ɐݒ肳��Ă���MacAddress��APL�̐ݒ�Buffer��set����
		wData = ReadReg(REG_MAADR1);
		AppConfig.MyMACAddr.v[0] = ((UCHAR*)&wData)[0];
		AppConfig.MyMACAddr.v[1] = ((UCHAR*)&wData)[1];
		wData = ReadReg(REG_MAADR2);
		AppConfig.MyMACAddr.v[2] = ((UCHAR*)&wData)[0];
		AppConfig.MyMACAddr.v[3] = ((UCHAR*)&wData)[1];
		wData = ReadReg(REG_MAADR3);
		AppConfig.MyMACAddr.v[4] = ((UCHAR*)&wData)[0];
		AppConfig.MyMACAddr.v[5] = ((UCHAR*)&wData)[1];
	} else {	//	APL�Őݒ肳�ꂽMacAddress��REG_MAADR1-3��set����
		((UCHAR*)&wData)[0] = AppConfig.MyMACAddr.v[0];
		((UCHAR*)&wData)[1] = AppConfig.MyMACAddr.v[1];
		WriteReg(REG_MAADR1, wData);
		((UCHAR*)&wData)[0] = AppConfig.MyMACAddr.v[2];
		((UCHAR*)&wData)[1] = AppConfig.MyMACAddr.v[3];
		WriteReg(REG_MAADR2, wData);
		((UCHAR*)&wData)[0] = AppConfig.MyMACAddr.v[4];
		((UCHAR*)&wData)[1] = AppConfig.MyMACAddr.v[5];
		WriteReg(REG_MAADR3, wData);
	}
#endif
#if 1
	/****************************************/
	/*	������MacAddress�� ---> Save����	*/
	/****************************************/
	wData = ReadReg(REG_MAADR1);
	MyMacAddr.v[0] = ((UCHAR*)&wData)[0];
	MyMacAddr.v[1] = ((UCHAR*)&wData)[1];
	wData = ReadReg(REG_MAADR2);
	MyMacAddr.v[2] = ((UCHAR*)&wData)[0];
	MyMacAddr.v[3] = ((UCHAR*)&wData)[1];
	wData = ReadReg(REG_MAADR3);
	MyMacAddr.v[4] = ((UCHAR*)&wData)[0];
	MyMacAddr.v[5] = ((UCHAR*)&wData)[1];
#endif
	
	/****************************************************************************************************************************************************/
	/*	PhysicalLayer Initialize																														*/
	/****************************************************************************************************************************************************/
	/*o	PHY_PHCON1		0x1000	---> 0x1000	PHY_CONTROL_REGISTER1																						*/
	/*	0		PHCON1_PRST(b15)			1/0 = Perform PHY Reset/PHY not in Reset																	*/
	/*	0		PHCON1_PLOOPBK(b14)			1/0 = Loopback enabled/Normal operation 																	*/
	/*	0->1	PHCON1_SPD100(b13)			1/0 = 100Mbps/10Mbps(*1)																					*/
	/*	1->0	PHCON1_ANEN(b12)			1/0 = Auto-negotiation enabled(SPD100 and PFULDPX are ignored)/disabled										*/
	/*	0		PHCON1_PSLEEP(b11)			1/0 = PHY powered down/Normal operation 																	*/
	/*	0		PHCON1_RENEG(b9)			1/0 = Restart the auto-negotiation process/Normal operation 												*/
	/*	0->1	PHCON1_PFULDPX(b8)			1/0 = Full duplex/Half duplex(*1)																			*/
	/*			*1: Applicable only when auto-negotiation is disabled(ANEN = 0). 																		*/
	/*	PHY_PHSTAT1		0x7809	---> 0x7809	PHY_STATUS_REGISTER1																						*/
	/*	1		PHSTAT1_FULL100(b14)		1 = 100Base-TX full-duplex operation(*1) 																	*/
	/*	1		PHSTAT1_HALF100(b13)		1 = 100Base-TX half-duplex operation(*1) 																	*/
	/*	1		PHSTAT1_FULL10(b12)			1 = 10Base-T full-duplex operation(*1) 																		*/
	/*	1		PHSTAT1_HALF10(b11)			1 = 10Base-T half-duplex operation(*1) 																		*/
	/*	0		PHSTAT1_ANDONE(b5)			1 = Auto-negotiation is complete 																			*/
	/*	0		PHSTAT1_LRFAULT(b4)			1 = Remote Fault condition has been detected																*/
	/*	1		PHSTAT1_ANABLE(b3)			1 = PHY is capable of auto-negotiation(*1)																	*/
	/*	0		PHSTAT1_LLSTAT(b2)			1 = Ethernet link is established and has stayed continuously established since the last read of PHSTAT1		*/
	/*	1		PHSTAT1_EXTREGS(b0)			1 = PHY has extended capability registers at addresses, 16 thru 31(*1) 										*/
	/*			*1: This is the only valid state for this bit; a'0'represents an invalid condition. 													*/
	/*	PHY_PHSTAT2		0x??0?	---> 0x??0?	PHY_STATUS_REGISTER2																						*/
	/*	0		PHSTAT2_PLRITY(b4)			1/0 = Wiring on TPIN+/- pins is reversed polarity/correct polarity											*/
	/*	PHY_PHSTAT3		0x0040	---> 0x0040	PHY_STATUS_REGISTER3																						*/
	/*	0		PHSTAT3_SPDDPX2_0(b4-b2)	110 = 100Mbps_full duplex/101 = 10 Mbps_full duplex/010 = 100Mbps_half duplex/001 = 10 Mbps_half duplex		*/
	/*o	PHY_PHANA		0x01E1	---> 0x01E1	PHY_AUTO_NEGOTIATION_ADVERTISEMENT_REGISTER																	*/
	/*	0		PHANA_ADNP(b15)				1/0 = Invalid/Local PHY does not support auto-negotiation next page abilities 								*/
	/*	0		PHANA_ADFAULT(b13)			1 = Local PHY has a Fault condition present																	*/
	/*	00->01	PHANA_ADPAUS1_0(b11-b10)	01 = Local device supports symmetric PAUSE only (Normal Flow Control mode) 									*/
	/*	1		PHANA_AD100FD(b8)			1 = Local PHY is capable of 100Base-TX full-duplex operation 												*/
	/*	1		PHANA_AD100(b7)				1 = Local PHY is capable of 100Base-TX half-duplex operation 												*/
	/*	1		PHANA_AD10FD(b6)			1 = Local PHY is capable of 10Base-T full-duplex operation 													*/
	/*	1		PHANA_AD10(b5)				1 = Local PHY is capable of 10Base-T half-duplex operation 													*/
	/*	00001	PHANA_ADIEEE4_0(b4-b0)		00001 = IEEE 802.3 Std. 																					*/
	/*	PHY_PHANLPA		0x????	---> 0x????	PHY_AUTO_NEGOTIATION_LINK_PARTNER_ABILITY_REGISTER															*/
	/*	?		PHANLPA_LPNP(b15)			1 = Link partner PHY supports auto-negotiation next page abilities 											*/
	/*	?		PHANLPA_LPACK(b14)			1 = Link partner PHY has successfully received the local PHY abilities saved in PHANA 						*/
	/*	?		PHANLPA_LPFAULT(b13)		1 = Link partner PHY has a Fault condition present 															*/
	/*	?		PHANLPA_LPPAUS1_0(b11-b10)	Link Partner PAUSE Flow Control Ability bits																*/
	/*	?		PHANLPA_LP100T4(b9)			1 = Link partner PHY is capable of operating in 100Base-T4 mode 											*/
	/*	?		PHANLPA_LP100FD(b8)			1 = Link partner PHY is capable of 100Base-TX full-duplex operation 										*/
	/*	?		PHANLPA_LP100(b7)			1 = Link partner PHY is capable of 100Base-TX half-duplex operation 										*/
	/*	?		PHANLPA_LP10FD(b6)			1 = Link partner PHY is capable of 10Base-T full-duplex operation 											*/
	/*	?		PHANLPA_LP10(b5)			1 = Link partner PHY is capable of 10Base-T half-duplex operation 											*/
	/*	?		PHANLPA_LPIEEE4_0(b4-b0)	00001 = IEEE 802.3 Std. 																					*/
	/*	PHY_PHANE		0x0000	---> 0x0000	PHY_AUTO_NEGOTIATION_EXPANSION_REGISTER																		*/
	/*	0		PHANE_PDFLT(b4)				1 = Parallel detection did not detect a valid link partner; automatically cleared when register is read 	*/
	/*	0		PHANE_LPARCD(b1)			1 = PHANLPA register has been written with a new value from the link partner;								*/
	/*	0		PHANA_LPANABL(b0)			1 = Link partner implements auto-negotiation 																*/
	/*	PHY_PHCON2		0x0002	---> 0x0002	PHY_CONTROL_REGISTER2																						*/
	/*	0		PHCON2_EDPWRDN(b13)			1 = Energy detect power-down enabled.																		*/
	/*	0		PHCON2_EDTHRES(b11)			1 = Less energy is required to wake the PHY from energy detect power-down 									*/
	/*	0		PHCON2_FRCLNK(b2)			1 = Force immediate link up, even when no link partner is present (100 Mbps operation only)(*1) 			*/
	/*	1		PHCON2_EDSTAT(b1)			1 = Energy detect circuit has detected energy on the TPIN+/- pins within the last 256ms						*/
	/*			*1: Intended for testing purposes only. Do not use in 10 Mbps operation. 																*/
	/****************************************************************************************************************************************************/
	WritePHYReg(PHY_PHANA, 	PHANA_ADPAUS0	|	//	01=Local device supports symmetric PAUSE only(Normal Flow Control mode)
							PHANA_AD10FD 	|	//	10Base-T full-duplex operation 
							PHANA_AD10 		|	//	10Base-T half-duplex operation
							PHANA_AD100FD 	|	//	100Base-TX full-duplex operation
							PHANA_AD100 	|	//	100Base-TX half-duplex operation
							PHANA_ADIEEE0);		//	00001=Always specify a selector value of '00001' for this device.
	/****************************************************/
	/*	Force operating mode, for debugging only.		*/
	/*	If none of these statements are executed,		*/
	/*	auto-negotiation/parallel detection is used		*/
	/*	which will always select the proper mode.		*/
	/****************************************************/
	/********************************************************/
	/*	41-0001												*/
	/*		�B	0/1/2 = Auto/�S��d/����d					*/
	/*	�C�D�E	000/010/100 = Auto/10MBase-T/100MBase-TX	*/
	/*		PHCON1_ANEN										*/
	/*		PHCON1_SPD100									*/
	/*		PHCON1_PFULDPX									*/
	/********************************************************/
	PhyPhcon1 = 0x0000;
	prm_41_0001_3 	= (unsigned long)prm_get(COM_PRM, S_MDL, 1, 1, 4);		//	41-0001-�B
	prm_41_0001_456 = (unsigned long)prm_get(COM_PRM, S_MDL, 1, 3, 1);		//	41-0001-�C�D�E
	switch (prm_41_0001_3) {
		case 0:
			PhyPhcon1 |= PHCON1_ANEN;
			break;	// Auto
		case 1:
			PhyPhcon1 |= PHCON1_PFULDPX;
			break;	// �S��d
		case 2:
			break;	// ����d
		default:
			break;	
	}
	switch (prm_41_0001_456) {
		case 0:
			PhyPhcon1 |= PHCON1_ANEN;
			break;	// Auto
		case 10:
//			ICU.IRQCR[15].BYTE = 0x04;
			break;	// 10MBase-T
		case 100:
//			ICU.IRQCR[15].BYTE = 0x00;
			PhyPhcon1 |= PHCON1_SPD100;
			break;	// 100MBase-TX
		default:
			break;	
	}
	WritePHYReg(PHY_PHCON1, PhyPhcon1);
	/****************************************************************************************************************************************************/
	/*	REG_EIE			0x8010				Ethernet_Interrupt_Enable_Register																			*/
	/*	1->1	EIE_INTIE(b15)				1 = INT pin is controlled by the INT status bit (ESTAT<15>) 												*/
	/*	0		EIE_MODEXIE(b14)			Modular Exponentiation Interrupt Enable bit(RW:0)															*/
	/*	0		EIE_HASHIE(b13)				MD5/SHA-1 Hash Interrupt Enable bit(RW:0)																	*/
	/*	0		EIE_AESIE(b12)				AES Encrypt/Decrypt Interrupt Enable bit(RW:0)																*/
	/*	0		EIE_LINKIE(b11)				PHY Link Status Change Interrupt Enable bit(RW:0)															*/
	/*	0		EIE_PRDYIE(b10)				(?)																											*/
	/*	0->1	EIE_PKTIE(b6)				RX Packet Pending Interrupt Enable bit(RW:0)																*/
	/*	0		EIE_DMAIE(b5)				DMA Interrupt Enable bit(RW:0)																				*/
	/*	0->1	EIE_TXIE(b3)				Transmit Done Interrupt Enable bit(RW:0)																	*/
	/*	0		EIE_TXABTIE(b2)				Transmit Abort Interrupt Enable bit(RW:0)																	*/
	/*	0		EIE_RXABTIE(b1)				Receive Abort Interrupt Enable bit(RW:0)																	*/
	/*	0		EIE_PCFULIE(b0)				Packet Counter Full Interrupt Enable bit(RW:0)																*/
	/*	REG_ECON1		0x0000				Ethernet_Control_Register1																					*/
	/*	0		ECON1_MODEXST(b15)			1 = Modular exponentiation calculation started/busy; automatically cleared by hardware when done 			*/
	/*	0		ECON1_HASHEN(b14)			1 = MD5/SHA-1 hashing engine enabled. Data written to the hashing engine by the DMA is added to the hash. 	*/
	/*	0		ECON1_HASHOP(b13)			1 = MD5/SHA-1 hash engine loads the Initial Value (IV) from the hash memory.								*/
	/*	0		ECON1_HASHLST(b12)			1 = The next DMA transfer to the hash engine completes the hash.											*/
	/*	0		ECON1_AESST(b11)			1 = AES encrypt/decrypt operation started/busy; automatically cleared by hardware when done 				*/
	/*	0		ECON1_AESOP1_0(b10-b9)		AES Ope Control	10 = ECB/CBC decrypt, 01 = CBC/CFB encrypt, 00 = ECB/CFB/OFB encrypt or key initialization 	*/
	/*	0		ECON1_PKTDEC(b8)			1 = Decrement PKTCNT (ESTAT<7:0>) bits by one. Hardware immediately clears PKTDEC to '0',					*/
	/*											allowing back-to-back decrement operations. 															*/
	/*	00		ECON1_FCOP1_0(b7-b6)		00 = Flow control disabled/Idle(it's depend on FULDPX(MACON2<0>) = 1 or 0)									*/
	/*	0		ECON1_DMAST(b5)				1 = DMA is started/busy; automatically cleared by hardware when done 										*/
	/*	0		ECON1_DMACPY(b4)			1 = DMA copies data to memory location at EDMADST 															*/
	/*	0		ECON1_DMACSSD(b3)			1 = DMA checksum operations are initially seeded by the one's complement of the checksum contained in EDMACS*/
	/*	0		ECON1_DMANOCS(b2)			1 = DMA does not compute checksums; EDMACS remains unchanged 												*/
	/*	0		ECON1_TXRTS(b1)				1 = Transmit an Ethernet frame; automatically cleared by hardware when done 								*/
	/*	0->1	ECON1_RXEN(b0)				1 = Packets which pass the current RX filter configuration are written to the receive buffer 				*/
	/****************************************************************************************************************************************************/
//	BitSetReg(REG_EIE, 		EIE_PKTIE	|	//	EIE_PKTIE(b6) 	<- 1
//							EIE_TXIE	|	//	EIE_TXIE(b3) 	<- 1
//							EIE_INTIE);		//	EIE_INTIE(b15) 	<- 1
//	WriteReg(REG_EIE, 	0x8050);	// EIE_INTIE(b15), EIE_PKTIE(b6)�̂ݗL��
//	WriteReg(REG_EIE, 	0x8018);	// EIE_INTIE(b15), EIE_TXIE(b3)�̂ݗL��
//	WriteReg(REG_EIE, 	0x8058);	// EIE_INTIE(b15), EIE_PKTIE(b6), EIE_TXIE(b3)�L��
	WriteReg(REG_EIE, 	0x8858);	// EIE_INTIE(b15), EIE_LINKIE(b11), EIE_PKTIE(b6), EIE_TXIE(b3)�L��
	BitSetReg(REG_ECON1, 	ECON1_RXEN);	//	ECON1_RXEN(b0) 	<- 1
#if 1
	//	�ݒ�l��Read���Ċm�F����
	wEIE 		= ReadReg(REG_EIE);
	wMACON1		= ReadReg(REG_MACON1);
	Ether_ReadAllRegster();
#endif
	wESTAT = ReadReg(REG_ESTAT);	// INT(b15), PHYLNK(b8), PKTCNT7_0(b7-b0)
	if(wESTAT & ESTAT_PHYLNK) {
		SendRecvInfo.PhylinkStatus = 1;
	} else {
		SendRecvInfo.PhylinkStatus = 0;
	}
	if (wEIE != 0)		wDummy = 1;
	if (wMACON1 != 0)	wDummy = 2;
	if (wMACON2 != 0)	wDummy = 3;
	if (wDummy != 0)	wDummy = 4;
}
/************************************************************************************************/
/*																								*/
/************************************************************************************************/
//	MACInit()����call�����Ƃ肠����opetask-I2C1_init()�O��_ei()�̑O����call����
void Ether_DTinit(void)
{
	USHT 	cnt;

	SendRecvInfo.I15cnt			= 0x0000;			// IRQ15�̔�����
	for (cnt = 0; cnt < SIZE_I15_EIR; cnt++) {
		SendRecvInfo.I15_EIR[cnt] = 0;				// IIRQ15��������������EIR�̏��
	}
	SendRecvInfo.I15cnt_BeforeOpen	= 0x0000;		// Open�O�ɔ�������IRQ15�̔�����(I15_EIR��Index)
	for (cnt = 0; cnt < 16; cnt++) {
		SendRecvInfo.I15_EIR_BeforeOpen[cnt] = 0;	// Open�O�ɔ�������IIRQ15��������������EIR�̏��
	}
	SendRecvInfo.PhylinkStatus	= 0;		// PHYLNK��Current���
	SendRecvInfo.SendByteCnt	= 0x0000;	// ���񑗐M����Byte��
	SendRecvInfo.SendPktInt		= 0x0000;	// IRQ15�ő��M�������������݂̔�����
	SendRecvInfo.RecvPktInt		= 0x0000;	// IRQ15�Ŏ�MPacket����M���������݂̔�����
	SendRecvInfo.RecvPktCnt		= 0x0000;	// IRQ15�Ŏ�MPacket��PKTCNT�̗݌v��
	SendRecvInfo.RecvPktNowBoxNo= 0;		// IRQ15�Ŏ�MPacket��Current��PKTCNT�̉񐔂�BoxNo
	for (cnt = 0; cnt < 4; cnt++) {
		SendRecvInfo.RecvPktNow[cnt] = 0;	// IRQ15�Ŏ�MPacket��Current��PKTCNT�̉�(EtherRecv�Ō��Z)
	}
	SendRecvInfo.RecvPktMax		= 0;		// IRQ15�Ŏ�MPacket��PKTCNT��Max�l
	SendRecvInfo.kasagoAPI_CallCnt = 0;		// tfNotifyInterfaceIsr()��call������
	SendRecvInfo.k_OpenCnt		= 0x0000;	// kasago���EtherOpen()���ꂽ������
	SendRecvInfo.k_CloseCnt		= 0x0000;	// kasago���EtherClose()���ꂽ������
	SendRecvInfo.k_SendCnt		= 0x0000;	// kasago���EtherSend()���ꂽ������
	SendRecvInfo.k_ReceiveCnt	= 0x0000;	// kasago���EtherReceive()���ꂽ������
	SendRecvInfo.k_FreeCnt		= 0x0000;	// kasago���EtherFreeReceiveBuffer()���ꂽ������
	SendRecvInfo.k_IoctlCnt		= 0x0000;	// kasago���EtherIoctl()���ꂽ������
	SendRecvInfo.k_GetPhyCnt	= 0x0000;	// kasago���EtherGetPhyAddr()���ꂽ������
	SendRecvInfo.ERR_EIR_TXABTIF = 0;		// 1 = Packet transmission has been aborted due to an error.
	SendRecvInfo.ERR_EIR_RXABTIF = 0;		// 1 = An RX packet was dropped because there is insufficient space
	SendRecvInfo.ERR_EIR_PCFULIF = 0;		// 1 = Packet Counter Full Interrupt Flag bit(RW:0)
	SendRecvInfo.ERR_ESTAT_RXBUSY = 0;		// 1 = Receive logic is currently receiving a packet.								*/
	SendRecvInfo.ERR_NextRxPtr_MAX = 0;
	SendRecvInfo.ERR_ByteCount_MAX = 0;
}
/************************************************************************************************/
/* PreCondition:    PSP bus must be initialized (done in MACInit()).							*/
/* Input:           Address of the PHY register to write to.									*/
/*					16 bits of data to write to PHY register.									*/
/* Overview:        WritePHYReg performs an MII write operation.  While in 						*/
/*					progress, it simply polls the MII BUSY bit wasting time.					*/
/*	�w��PHY_Register(00 - 1F) <--- �w��Data(2Byte)��Write����									*/
/************************************************************************************************/
void WritePHYReg(	UCHAR Register,	// PhyRegisterAddress(00 - 1F)
					USHT Data)		// WriteData(2Byte)
{
	/****************************************/
	/*	REG_MIREGADR <- PhyRegisterAddress	*/
	/*	b8 = always_1						*/
	/****************************************/
	WriteReg(REG_MIREGADR,  0x0100 | Register);	// WriteData = Write������PhyAddress
	/****************************/
	/*	REG_MIWR <- WriteData	*/
	/****************************/
	WriteReg(REG_MIWR, Data);
	/************************************************************************/
	/*	Loop to wait until the PHY register has been written through the MII*/
	/*	This requires 25.6us												*/
	/************************************************************************/
	while(ReadReg(REG_MISTAT) & MISTAT_BUSY);
}
/************************************************************************************************/
/* PreCondition:    PSP bus must be initialized (done in MACInit()).							*/
/* Input:           Address of the PHY register to read from.									*/
/* Output:          16 bits of data read from the PHY register.									*/
/* Overview:        ReadPHYReg performs an MII read operation.  While in 						*/
/*					progress, it simply polls the MII BUSY bit wasting time(25.6us).			*/
/*	�w��PHY_Register(00 - 1F)���� ---> Data(2Byte)��Read����									*/
/************************************************************************************************/
USHT ReadPHYReg(UCHAR Register)	// PhyRegisterAddress(00 - 1F)
{
	USHT 	wResult;
	/****************************************/
	/*	REG_MIREGADR <- PhyRegisterAddress	*/
	/*	b8 = always_1						*/
	/****************************************/
	WriteReg(REG_MIREGADR, 0x0100 | Register);	// WriteData = Read������PhyAddress
	/********************************/
	/*	MIIRD <- 1, MIISCAN <- 0	*/
	/********************************/
	WriteReg(REG_MICMD, MICMD_MIIRD);
	/************************************************************************/
	/*	Loop to wait until the PHY register has been read through the MII	*/
	/*	This requires 25.6us												*/
	/************************************************************************/
	while(ReadReg(REG_MISTAT) & MISTAT_BUSY);
	/********************************/
	/*	MIIRD <- 0, MIISCAN <- 0	*/
	/********************************/
	WriteReg(REG_MICMD, 0x0000);
	/************************/
	/*	wResult <- ReadData	*/
	/************************/
	wResult = ReadReg(REG_MIRD);
	return wResult;
}
/************************************************************************************************/
/* PreCondition:    PSP bus must be initialized (done in MACInit()).							*/
/* Input:           wAddress: Address of the SFR register to write to.  						*/
/*					16-bit USHT to be written into the register.								*/
/* Overview:        Selects the correct bank (if using the PSP and needed), and 				*/
/*					writes the corresponding 16-bit register with wValue.						*/
/* Note:            This routine cannot write to PHY registers.  Use the 						*/
/*					WritePHYReg() function for writing to PHY registers.						*/
/*	�w��SFR_Register(CPU_Access���Address) <--- �w��Data(2Byte)��Write����						*/
/************************************************************************************************/
void WriteReg(	ULNG 	RegAdr,		// SFR_RegisterAddress(CPU_Access���Address)
				USHT 	wValue)		// WriteData(2Byte)
{
	WriteMemory(RegAdr,				// SFR_RegisterAddress(CPU_Access���Address)
				(UCHAR*)&wValue,	// WriteDataAddress
				2);					// Byte��(2)
}
/************************************************************************************************/
/* PreCondition:    PSP bus must be initialized (done in MACInit()).							*/
/* Input:           wAddress: Address of SFR register to read from.  							*/
/*							  The LSb is ignored and treated as '0' always.						*/
/* Output:          USHT value of register contents												*/
/* Overview:        Selects the correct bank (if needed), and reads the 						*/
/*					corresponding 16-bit register												*/
/* Note:            This routine cannot be used to read PHY registers.  						*/
/*					Use the ReadPHYReg() function to read from PHY registers.					*/
/*	�w��SFR_Register(CPU_Access���Address) ---> Data(2Byte)��Read����							*/
/************************************************************************************************/
USHT ReadReg(ULNG RegAdr)			// SFR_RegisterAddress(CPU_Access���Address)
{
	USHT 	wData;					// Read����Data

	ReadMemory(	RegAdr,				// SFR_RegisterAddress(CPU_Access���Address)
				(UCHAR*)&wData,		// ReadDataAddress
				2);					// Byte��(2)
	return wData;
}
/************************************************************************************************/
/* Input:           7bit address of the register to write to.  									*/
/*					16-bit USHT bitmask to set/clear in the register.							*/
/* Overview:        Sets bits in Ethernet SFR registers											*/
/* Note:            These functions cannot be used to access ESFR registers.					*/
/*	�w��SFR_Register(CPU_Access���Address)_�w��Bit <---	1��Write����						*/
/************************************************************************************************/
void BitSetReg(	ULNG 	RegAdr,			// SFR_RegisterAddress(CPU_Access���Address)
				USHT 	wBitMask)		// Set����Bit(0-15)
{
	WriteMemory(RegAdr + SET_OFFSET,	// SFR_RegisterAddress(CPU_Access���Address)
				(UCHAR*)&wBitMask,		// BitData��Address <- 1
				2);						// Byte��(2)
}
/************************************************************************************************/
/* Input:           7bit address of the register to write to.  									*/
/*					16-bit USHT bitmask to set/clear in the register.							*/
/* Overview:        clears bits in Ethernet SFR registers										*/
/* Note:            These functions cannot be used to access ESFR registers.					*/
/*	�w��SFR_Register(CPU_Access���Address)_�w��Bit <---	0��Write����						*/
/************************************************************************************************/
void BitClearReg(	ULNG 	RegAdr,		// SFR_RegisterAddress(CPU_Access���Address)
					USHT 	wBitMask)	// Clear����Bit(0-15)
{
	WriteMemory(RegAdr + CLR_OFFSET,	// SFR_RegisterAddress(CPU_Access���Address)
				(UCHAR*)&wBitMask,		// BitData��Address <- 0
				2);						// Byte��(2)
}
/********************************************************************************************************/
/*																										*/
/********************************************************************************************************/
void FillMemory(	ULNG 	Address,	// StartAddress(CPU_Access���Address)
					UCHAR 	FillData,	// FillData
					USHT 	wLength)	// Byte��
{
	USHT 	wData;
	USHT 	cnt;
	USHT 	Wordlen;

//	return;

	if(wLength == 0u)		// WriteData���Ȃ���
		return;

	Wordlen = (wLength/2);

	((UCHAR*)&wData)[0] = FillData;
	((UCHAR*)&wData)[1] = FillData;
	for (cnt=0; cnt < Wordlen; cnt++) {
		*(volatile unsigned short *)Address = wData;
		Address++;			// 1,2,3.....
		Address++;			// 1,2,3.....
	}
}
/********************************************************************************************************/
/* Input:           wAddress: 	ENCX24J600 RAM or SFR address to write to								*/
/*					*vData: 	Pointer to local PIC RAM which needs to be written to the ENCX24J600	*/
/*					wLength: 	Number of bytes to copy from vData to wAddress							*/
/* Overview:        Copys 0 or more bytes to the ENCX24J600 RAM											*/
/* Note:            Can be used to access SFRs and ESFRs when using PSP									*/
/*	�w�肳�ꂽSRAMorSFRaddress(CPU_Access���Address)�Ɏw�肳�ꂽData���w�肳�ꂽByte����Write����		*/
/********************************************************************************************************/
void WriteMemory(	ULNG 	Address,	// SRAM or SFR address(CPU_Access���Address)
					UCHAR 	*vData,		// ������DataAddress
					USHT 	wLength)	// Byte��
{
	USHT 	wData;

	if(wLength == 0u)		// WriteData���Ȃ���
		return;
	/****************************************************************/
	/*	������StartAddress����̎�								*/
	/*	---> 16bit_AddresssBus, 16bit_DataAddress�Ȃ̂ōl�����Ȃ�	*/
	/****************************************************************/
//	if(wAddress & 0x1) {
//	}
	/********************************************************/
	/*	Data����0 or 1�܂ŏ����݂��s��						*/
	/********************************************************/
	while(wLength >= 2u) {
//		((UCHAR*)&wData)[1] = *vData++;	// ���Data����ʂ�set
//		((UCHAR*)&wData)[0] = *vData++;	// ����Data�����ʂ�set
		((UCHAR*)&wData)[0] = *vData++;	// ���Data�����ʂ�set
		((UCHAR*)&wData)[1] = *vData++;	// ����Data����ʂ�set
		*(volatile unsigned short *)Address = wData;
		Address++;			// 1,2,3.....
		Address++;			// 1,2,3.....
		wLength -= 2;
	}
	/********************************************************/
	/*	�cData����1�̎�										*/
	/********************************************************/
	if(wLength) {
		wData = *(volatile unsigned short *)Address;
//		wData &= 0xff00;				// ���Data��Mask
//		((UCHAR*)&wData)[0] = *vData++;	// ����Data��set
		wData &= 0x00ff;
		((UCHAR*)&wData)[1] = *vData++;
		*(volatile unsigned short *)Address = wData;
	}
}
/****************************************************************************************************/
/* Input:           wAddress: ENCX24J600 RAM or SFR address to read from							*/
/*					*vData: Pointer to local PIC RAM which will be written 							*/
/*							with data from the ENCX24J600.											*/
/*					wLength: Number of bytes to copy from wAddress to vData							*/
/* Overview:        Copys 0 or more bytes from the ENCX24J600 RAM									*/
/* Note:            Can be used to access SFRs and ESFRs when using PSP								*/
/*	�w�肳�ꂽSRAMorSFRaddress(CPU_Access���Address)���w�肳�ꂽData�Ɏw�肳�ꂽByte����Read����	*/
/****************************************************************************************************/
void ReadMemory(	ULNG 	Address,	// SRAM or SFR address(CPU_Access���Address)
					UCHAR 	*vData,		// �Ǎ���DataAddress
					USHT 	wLength)	// Byte��
{
	USHT 	wData;

	if(wLength == 0u)			// ReadData���Ȃ���
		return;
	/****************************************************************/
	/*	�Ǎ���StartAddress����̎�								*/
	/*	---> 16bit_AddresssBus, 16bit_DataAddress�Ȃ̂ōl�����Ȃ�	*/
	/****************************************************************/
//	if(wAddress & 0x1) {
//	}
	/********************************************************/
	/*	Data����0 or 1�܂œǍ��݂��s��						*/
	/********************************************************/
	while(wLength >= 2u) {
		wData = *(volatile unsigned short *)Address;
//		*vData++ = (UCHAR)(wData & 0x00ff);
//		*vData++ = (UCHAR)((wData & 0xff00)>>8);
		*vData++ = (UCHAR)((wData & 0xff00)>>8);
		*vData++ = (UCHAR)(wData & 0x00ff);
		Address++;			// 1,2,3.....
		Address++;			// 1,2,3.....
		wLength -= 2;
	}
	/********************************************************/
	/*	�cData����1�̎�										*/
	/********************************************************/
	if(wLength) {
		wData = *(volatile unsigned short *)Address;
//		*vData++ = (UCHAR)(wData & 0x00ff);			// ����Data��Mask
		*vData++ = (UCHAR)((wData & 0xff00)>>8);
	}
}
/************************************************************************************************/
/* Input:           vWindowNo: 	UDA_WINDOW, GP_WINDOW, or RX_WINDOW corresponding 				*/
/*							 	to the window register to write to								*/
/*					*vData: Pointer to local PIC RAM which contains the source data				*/
/*					wLength: Number of bytes to copy from vData to window						*/
/* Overview:        Copys 0 or more bytes from CPU RAM to the ENCX24J600 						*/
/*					Family RAM using one of the UDA, TX, or RX write window pointers.			*/
/*					This pointer is incremented by the number of bytes writen.					*/
/*	�w�肳�ꂽWindowRegister�ɑ΂��Ďw�肳�ꂽData���w��Byte����Write����						*/
/*	REG_EGPDATA���ByteRead����(REG_EGPDATA = EGPWRPT��Address)									*/
/************************************************************************************************/
void WriteMemWindow(	UCHAR 	vWindowNo,	// �K��GP_WINDOW�Ŏw�肳���
 						UCHAR 	*vData,		// Pointer to local PIC RAM which contains the source data
 						USHT 	wLength)	// Byte��
{
	ULNG 	WindowReg;						// CPU_Access���Address
	USHT	wData;

	if(wLength == 0u)
		return;
//////////////////////////////////////
	WindowReg = REG_EUDADATA;		// Ethernet_User_Defined_Area_Data_Window_Register7_0(Reset = 0x--??)
	if(vWindowNo & GP_WINDOW)
		WindowReg = REG_EGPDATA;	// Ethernet_General_Purpose_Data_Window_Register_7_0(Reset = 0x--??)
	if(vWindowNo & RX_WINDOW)
		WindowReg = REG_ERXDATA;	// Ethernet_RX_Data_Window_Register7_0(Reset = 0x--??)
//////////////////////////////////////
//	WindowReg = REG_EGPDATA;		// Write����Address��ݒ�
	/********************************************************/
	/*	Data����0 or 1�܂ŏ����݂��s��						*/
	/********************************************************/
	while(wLength >= 2u) {
		((UCHAR*)&wData)[0] = *vData++;	// ���Data�����ʂ�set
		((UCHAR*)&wData)[1] = *vData++;	// ����Data����ʂ�set
//		*(volatile unsigned short *)WindowReg = ((UCHAR*)&wData)[1];
//		*(volatile unsigned short *)WindowReg = ((UCHAR*)&wData)[0];
		*(volatile unsigned short *)WindowReg = ((UCHAR*)&wData)[0];
		*(volatile unsigned short *)WindowReg = ((UCHAR*)&wData)[1];
		wLength -= 2;
	}
	/********************************************************/
	/*	�cData����1�̎�										*/
	/********************************************************/
	if(wLength) {
		wData = *(volatile unsigned short *)WindowReg;
		wData &= 0x00ff;
		((UCHAR*)&wData)[1] = *vData++;
		*(volatile unsigned short *)WindowReg = wData;
	}
}
/************************************************************************************************/
/* Input:           vWindowNo: 	UDA_WINDOW, GP_WINDOW, or RX_WINDOW corresponding 				*/
/*							 	to the window register to read from								*/
/*					*vData: 	Pointer to local PIC RAM which will be written 					*/
/*								with data from the ENC624J600 Family.							*/
/*					wLength: 	Number of bytes to copy from window to vData					*/
/* Overview:        Copys 0 or more bytes from the ENC624J600 Family RAM using 					*/
/*					one of the UDA, TX, or RX read window pointers.								*/
/*					This pointer is incremented by the number of bytes read.  					*/
/*					However, if using a 16-bit parallel interface, the pointer 					*/
/*					will be incremented by 1 extra if the length parameter is 					*/
/*					odd to ensure 16-bit alignment.												*/
/*	�w�肳�ꂽWindowRegister�ɑ΂��Ďw�肳�ꂽData���w��Byte����Read����						*/
/*	REG_ERXDATA���ByteRead����(REG_ERXDATA = REG_ERXRDPT��Address)								*/
/************************************************************************************************/
void ReadMemWindow(	UCHAR 	vWindowNo,	// �K��RX_WINDOW�Ŏw�肳���
					UCHAR 	*vData,		// Pointer to local PIC RAM which will be written with data from the ENC624J600 Family
					USHT 	wLength,	// Byte��
					USHT 	wEndian)	// 
{
	ULNG 	WindowReg;					// CPU_Access���Address
	USHT	wData;

	if(wLength == 0u)
		return;
//////////////////////////////////////
	WindowReg = REG_EUDADATA;		// Ethernet_User_Defined_Area_Data_Window_Register7_0(Reset = 0x--??)
	if(vWindowNo & GP_WINDOW)
		WindowReg = REG_EGPDATA;	// Ethernet_General_Purpose_Data_Window_Register_7_0(Reset = 0x--??)
	if(vWindowNo & RX_WINDOW)
		WindowReg = REG_ERXDATA;	// Ethernet_RX_Data_Window_Register7_0(Reset = 0x--??)
//////////////////////////////////////
//	WindowReg = REG_ERXDATA;		// Read����Address��ݒ�
	/********************************************************/
	/*	Data����0 or 1�܂œǍ��݂��s��						*/
	/********************************************************/
	while(wLength >= 2u) {
		if (wEndian == 0) {
			((UCHAR*)&wData)[0] = *(volatile unsigned short *)WindowReg;
			((UCHAR*)&wData)[1] = *(volatile unsigned short *)WindowReg;
		} else {
			((UCHAR*)&wData)[1] = *(volatile unsigned short *)WindowReg;
			((UCHAR*)&wData)[0] = *(volatile unsigned short *)WindowReg;
		}
		*vData++ = ((UCHAR*)&wData)[0];
		*vData++ = ((UCHAR*)&wData)[1];
//		*vData++ = ((UCHAR*)&wData)[1];
//		*vData++ = ((UCHAR*)&wData)[0];
		wLength -= 2;
	}
	/********************************************************/
	/*	�cData����1�̎�										*/
	/********************************************************/
	if(wLength) {
		wData = *(volatile unsigned short *)WindowReg;
//		*vData++ = (UCHAR)(wData & 0x00ff);			// ����Data1Byte��Mask
		*vData++ = (UCHAR)((wData & 0xff00)>>8);	// ���Data1Byte��Mask
	}
}
/************************************************************************************************/
/* PreCondition:    PSP bus must be initialized (done in MACInit()).							*/
/* Overview:        SystemReset reliably resets the Ethernet controller.  						*/
/*					It resets all register contents (except for COCON bits of REG_ECON2)		*/
/*					and returns the device to the power on default state.						*/
/*					This function should be called instead of directly 							*/
/*					attempting to perform a reset via the REG_ECON2<ETHRST> bit. 				*/
/*					If using the PSP, SystemReset also does basic checks to 					*/
/*					look for unsoldered pins or solder bridges on the PSP pins.					*/
/* Note:            This function is a blocking function and will lock up the application		*/
/*					if a non-recoverable problem is present.  									*/
/*					Possible non-recoverable problems include:									*/
/*						- PSP module not configured correctly									*/
/*						- PMP module not configured correctly									*/
/*						- HardwareProfile pins not defined correctly							*/
/*						- Solder bridge on PSP/PMP lines										*/
/*						- Unsoldered pins on PSP/PMP lines										*/
/*						- 25MHz Ethernet oscillator not running									*/
/*						- Vdd lower than ENCX24J600 operating range								*/
/*						- I/O levels out of range (for example if the PIC is at 				*/
/*						  2V without level shifting)											*/
/*						- Bus contention on PSP/PMP lines with other slaves						*/
/*						- One or more Vdd or Vss pins are not connected.						*/
/*	call��		MACInit																			*/
/************************************************************************************************/
void SystemReset(void)
{
	USHT 	wOffset;
	USHT 	wEUDAST;
	USHT 	wESTAT, wECON1, wECON2;
	USHT 	wTestWriteData, wTestReadData;
	UCHAR 	WriteBuff[16], ReadBuff[16];
	USHT	wait, cnt, wDummy;

	/********************************************/
	/*	Perform a reset via the PSP interface	*/
	/********************************************/
//	do {
		/********************************************************************************/
		/*	Set and clear a few bits that clears themselves upon reset.					*/
		/*	If REG_EUDAST cannot be written to and your code gets stuck in this loop,	*/
		/*	you have a hardware problem of some sort									*/
		/*		1.	PMP not initialized correctly,										*/
		/*		2.	I/O pins aren't connected or										*/
		/*		3.	are shorted to something,											*/
		/*		4.	power isn't available, etc.											*/
		/********************************************************************************/
		do {
			WriteReg(REG_EUDAST, 0x1234);
		} while(ReadReg(REG_EUDAST) != 0x1234u);
		/************************************************************************************************************/
		/*	ECON2_ETHRST = Master Ethernet Reset bit(RW:0)															*/
		/*		All TX, RX, MAC, PHY, DMA, modular exponentiation, hashing and AES logic,							*/
		/*		and registers (excluding COCON) are reset.															*/
		/*		Hardware self-clears this bit to '0'.																*/
		/*		After setting this bit,																				*/
		/*		wait at least 25 ��s before attempting to read or write to the ENCX24J600 via the PSP interface. 	*/
		/************************************************************************************************************/
		BitSetReg(REG_ECON2, ECON2_ETHRST);			//	ECON2_ETHRST <- 1
		/************************************************************/
		/*	After Reset of TX, RX, MAC, PHY, DMA					*/
		/*	ESTAT_CLKRDY, ESTAT_RSTDONE, ESTAT_PHYRDY must be 1		*/
		/************************************************************/
		while((ReadReg(REG_ESTAT) & (ESTAT_CLKRDY | ESTAT_RSTDONE | ESTAT_PHYRDY)) !=
									(ESTAT_CLKRDY | ESTAT_RSTDONE | ESTAT_PHYRDY));
		//Delay10us(3);
		for (wait = 0; wait < 0x10; wait++) {
			cnt++;
		}
		/********************************************************************************************/
		/*	Check to see if the reset operation was successful by checking							*/
		/*	if REG_EUDAST went back to its reset default(0x0000u)									*/
		/*	This test should always pass, but certain special conditions might make this test fail,	*/
		/*	such as a PSP pin shorted to logic high.												*/
		/********************************************************************************************/
//	} while(wEUDAST = ReadReg(REG_EUDAST) != 0x0000u);
//	} while(wEUDAST = ReadReg(REG_EUDAST) != 0x0001u);
	/****************************************************************************/
	/*	Really ensure reset is done and give some time for power to be stable	*/
	/****************************************************************************/
	//DelayMs(1);
	for (wait = 0; wait < 0x600; wait++) {
		cnt++;
	}
#if 1
	wESTAT = ReadReg(REG_ESTAT);	// INT(b15), PHYLNK(b8), PKTCNT7_0(b7-b0)
	wECON1 = ReadReg(REG_ECON1);	// PKTDEC(b8), TXRTS(b1), RXEN(b0)
	wECON2 = ReadReg(REG_ECON2);	// ETHRST(b4)
#endif
	/****************************************************************************************************/
	/*	Initialize RAM contents with a random pattern and read back to verify							*/
	/*	This step is critical if using a PSP interface since some functionality 						*/
	/*	may appear to work while a solder bridge or disconnect will cause certain memory ranges to fail.*/
	/*	Generate and write random pattern																*/
	/****************************************************************************************************/
	/********************************************************/
	/*	SRAM(0x0000 - 0x2FFF) <- Data��2Byte�P�ʂŏ�����	*/
	/********************************************************/
	srand(600);
//	for(wOffset = 0; wOffset < SIZE_SRAM_MAX; wOffset += sizeof(wTestWriteData)) {
	for(wOffset = 0; wOffset < 8; wOffset += sizeof(wTestWriteData)) {
		wTestWriteData = rand() + rand();
		WriteMemory(	wOffset + TOP_ADDR_SRAM,	// SRAM_Address(CPU_Access���Address)
						(UCHAR*)&wTestWriteData,	// ������Data
						sizeof(wTestWriteData));	// Byte��(2)
		WriteBuff[wOffset+0] = (wTestWriteData &0x00ff);
		WriteBuff[wOffset+1] = ((wTestWriteData &0xff00)>>8);
	}
	/********************************************************/
	/*	SRAM(0x0000 - 0x2FFF) -> Data��2Byte�P�ʂœǍ���	*/
	/*	������Data : �Ǎ���Data��2Byte�P�ʂŔ�r����		*
	/********************************************************/
	srand(600);
//	for(wOffset = 0; wOffset < SIZE_SRAM_MAX; wOffset += sizeof(wTestWriteData)) {
	for(wOffset = 0; wOffset < 8; wOffset += sizeof(wTestWriteData)) {
		wTestWriteData = rand() + rand();
		ReadMemory(	wOffset + TOP_ADDR_SRAM,	// SRAM_Address(CPU_Access���Address)
					(UCHAR*)&wTestReadData,		// �Ǎ���Data
					sizeof(wTestReadData));		// Byte��(2)
		ReadBuff[wOffset+0] = (wTestReadData &0x00ff);
		ReadBuff[wOffset+1] = ((wTestReadData &0xff00)>>8);
		/****************************************************/
		/*	See if the data matches.						*/
		/*	If your application gets stuck here, 			*/
		/*	it means you have a hardware failure.			*/
		/*	Check all of your PSP address and data lines.	*/
		/****************************************************/
		if(wTestWriteData != wTestReadData)
			while(1);
	}
	if (wEUDAST != 0)		wDummy = 1;
	if (wESTAT != 0)		wDummy = 2;
	if (wECON1 != 0)		wDummy = 3;
	if (wECON2 != 0)		wDummy = 4;
	if (WriteBuff != 0)		wDummy = 5;
	if (ReadBuff != 0)		wDummy = 6;
	if (wDummy != 0)		wDummy = 6;
}
/************************************************************************************************/
/*																								*/
/************************************************************************************************/
void Ether_ReadAllRegster(void)
{
	UCHAR 	bPacketCount, bDummy;
	USHT 	wCurrentPacketAddress;		// not Regiseter Data
	USHT 	wETXST,	wETXLEN,wERXST,wERXHEAD,wERXTAIL,wETXSTAT,wETXWIRE,wEUDAST,wEUDAND;
	USHT 	wEGPDATA,wERXDATA,wEUDADATA,wEGPRDPT,wEGPWRPT,wERXRDPT,wERXWRPT,wEUDARDPT,wEUDAWRPT;
	USHT 	wESTAT,wECON1,wECON2,wEPAUS,wERXWM,wEIE,wEIR,wEIDLED;
	USHT 	wEDMAST,wEDMALEN,wEDMADST,wEDMACS;
	USHT 	wMACON1,wMACON2,wMABBIPG,wMAIPG,wMACLCON,wMAMXFL,wMAADR3,wMAADR2,wMAADR1;
	USHT 	wMICMD,wMIREGADR,wMIWR,wMIRD,wMISTAT;
	USHT 	wPHCON1,wPHSTAT1,wPHSTAT2,wPHSTAT3,wPHANA,wPHANLPA,wPHANE,wPHCON2;
//	USHT 	wEHT1,wEHT2,wEHT3,wEHT4,
//	USHT 	wEPMM1,wEPMM2,wEPMM3,wEPMM4,wEPMCS,wEPMO,wERXFCON,

	wETXST		= ReadReg(REG_ETXST);		//	0x0000
	wETXLEN		= ReadReg(REG_ETXLEN);		//	0x0000
	wERXST		= ReadReg(REG_ERXST);		//	0x5340
	wERXHEAD	= ReadReg(REG_ERXHEAD);		//	0x5340
	wERXTAIL	= ReadReg(REG_ERXTAIL);		//	0x5FFE
	wETXSTAT	= ReadReg(REG_ETXSTAT);		//	0x0000=NotUsed
	wETXWIRE	= ReadReg(REG_ETXWIRE);		//	0x0000=NotUsed
	wEUDAST		= ReadReg(REG_EUDAST);		//	0x0000
	wEUDAND		= ReadReg(REG_EUDAND);		//	0x5FFF
	wEGPDATA	= ReadReg(REG_EGPDATA);		//	0x--??
	wERXDATA	= ReadReg(REG_ERXDATA);		//	0x--??
	wEUDADATA	= ReadReg(REG_EUDADATA);	//	0x--??
	wEGPRDPT	= ReadReg(REG_EGPRDPT);		//	0x05FA=NotUsed
	wEGPWRPT	= ReadReg(REG_EGPWRPT);		//	0x0000
	wERXRDPT	= ReadReg(REG_ERXRDPT);		//	0x05FA
	wERXWRPT	= ReadReg(REG_ERXWRPT);		//	0x0000=NotUsed
	wEUDARDPT	= ReadReg(REG_EUDARDPT);	//	0x05FA=NotUsed
	wEUDAWRPT	= ReadReg(REG_EUDAWRPT);	//	0x0000=NotUsed
	//--------------------------------------------
	wESTAT		= ReadReg(REG_ESTAT);		//	0x0000
	wECON1		= ReadReg(REG_ECON1);		//	0x0000
	wECON2		= ReadReg(REG_ECON2);		//	0xCB00
	wEPAUS		= ReadReg(REG_EPAUS);		//	0x1000=NotUsed
	wERXWM		= ReadReg(REG_ERXWM);		//	0x100F=NotUsed
	wEIE		= ReadReg(REG_EIE);			//	0x8010
	wEIR		= ReadReg(REG_EIR);			//	0x0A00
	wEIDLED		= ReadReg(REG_EIDLED);		//	0x26??=NotUsed
	//--------------------------------------------
	wEDMAST		= ReadReg(REG_EDMAST);		//	0x0000
	wEDMALEN	= ReadReg(REG_EDMALEN);		//	0x0000
	wEDMADST	= ReadReg(REG_EDMADST);		//	0x0000
	wEDMACS		= ReadReg(REG_EDMACS);		//	0x0000
	//--------------------------------------------
//	wEHT1		= ReadReg(REG_EHT1);		//	0x0000=NotUsed
//	wEHT2		= ReadReg(REG_EHT2);		//	0x0000=NotUsed
//	wEHT3		= ReadReg(REG_EHT3);		//	0x0000=NotUsed
//	wEHT4		= ReadReg(REG_EHT4);		//	0x0000=NotUsed
//	wEPMM1		= ReadReg(REG_EPMM1);		//	0x0000=NotUsed
//	wEPMM2		= ReadReg(REG_EPMM2);		//	0x0000=NotUsed
//	wEPMM3		= ReadReg(REG_EPMM3);		//	0x0000=NotUsed
//	wEPMM4		= ReadReg(REG_EPMM4);		//	0x0000=NotUsed
//	wEPMCS		= ReadReg(REG_EPMCS);		//	0x0000=NotUsed
//	wEPMO		= ReadReg(REG_EPMO);		//	0x0000=NotUsed
//	wERXFCON	= ReadReg(REG_ERXFCON);		//	0x0059=NotUsed
	//--------------------------------------------
	wMACON1		= ReadReg(REG_MACON1);		//	0x?00D=NotUsed
	wMACON2		= ReadReg(REG_MACON2);		//	0x40B2
	wMABBIPG	= ReadReg(REG_MABBIPG);		//	0x0012
	wMAIPG		= ReadReg(REG_MAIPG);		//	0x0C12=NotUsed
	wMACLCON	= ReadReg(REG_MACLCON);		//	0x370F=NotUsed
	wMAMXFL		= ReadReg(REG_MAMXFL);		//	0x05EE=NotUsed
	wMAADR3		= ReadReg(REG_MAADR3);		//	0x????
	wMAADR2		= ReadReg(REG_MAADR2);		//	0x??A3
	wMAADR1		= ReadReg(REG_MAADR1);		//	0x0400
	//--------------------------------------------
	wMICMD		= ReadReg(REG_MICMD);		//	0x--00
	wMIREGADR	= ReadReg(REG_MIREGADR);	//	0x0100
	wMIWR		= ReadReg(REG_MIWR);		//	0x0000
	wMIRD		= ReadReg(REG_MIRD);		//	0x0000
	wMISTAT		= ReadReg(REG_MISTAT);		//	0x--00
	//--------------------------------------------
	bPacketCount= (UCHAR)wESTAT;
	wCurrentPacketAddress = wCurrentRxPacketPointer;
	//--------------------------------------------
	wPHCON1		= ReadPHYReg(PHY_PHCON1);	//	0x1000
	wPHSTAT1	= ReadPHYReg(PHY_PHSTAT1);	//	0x7809
	wPHSTAT2	= ReadPHYReg(PHY_PHSTAT2);	//	0x??0?
	wPHSTAT3	= ReadPHYReg(PHY_PHSTAT3);	//	0x0040
	wPHANA		= ReadPHYReg(PHY_PHANA);	//	0x01E1
	wPHANLPA	= ReadPHYReg(PHY_PHANLPA);	//	0x????
	wPHANE		= ReadPHYReg(PHY_PHANE);	//	0x0000
	wPHCON2		= ReadPHYReg(PHY_PHCON2);	//	0x0002

	if (bPacketCount != 0)			bDummy = 0xff;
	if (wCurrentPacketAddress != 0)	bDummy = 0xfe;
	if (wETXST != 0)				bDummy = 1;
	if (wETXLEN != 0)				bDummy = 2;
	if (wERXST != 0)				bDummy = 3;
	if (wERXHEAD != 0)				bDummy = 4;
	if (wERXTAIL != 0)				bDummy = 5;
	if (wETXSTAT != 0)				bDummy = 6;
	if (wETXWIRE != 0)				bDummy = 7;
	if (wEUDAST != 0)				bDummy = 8;
	if (wEUDAND != 0)				bDummy = 9;
	if (wEGPDATA != 0)				bDummy = 10;
	if (wERXDATA != 0)				bDummy = 11;
	if (wEUDADATA != 0)				bDummy = 12;
	if (wEGPRDPT != 0)				bDummy = 13;
	if (wEGPWRPT != 0)				bDummy = 14;
	if (wERXRDPT != 0)				bDummy = 15;
	if (wERXWRPT != 0)				bDummy = 16;
	if (wEUDARDPT != 0)				bDummy = 17;
	if (wEUDAWRPT != 0)				bDummy = 18;
	if (wESTAT != 0)				bDummy = 19;
	if (wECON1 != 0)				bDummy = 20;
	if (wECON2 != 0)				bDummy = 211;
	if (wEPAUS != 0)				bDummy = 22;
	if (wERXWM != 0)				bDummy = 23;
	if (wEIE != 0)					bDummy = 24;
	if (wEIR != 0)					bDummy = 25;
	if (wEIDLED != 0)				bDummy = 26;
	if (wEDMAST != 0)				bDummy = 27;
	if (wEDMALEN != 0)				bDummy = 28;
	if (wEDMADST != 0)				bDummy = 29;
	if (wEDMACS != 0)				bDummy = 30;
	if (wMACON1 != 0)				bDummy = 31;
	if (wMACON2 != 0)				bDummy = 32;
	if (wMABBIPG != 0)				bDummy = 33;
	if (wMAIPG != 0)				bDummy = 34;
	if (wMACLCON != 0)				bDummy = 35;
	if (wMAMXFL != 0)				bDummy = 36;
	if (wMAADR3 != 0)				bDummy = 37;
	if (wMAADR2 != 0)				bDummy = 38;
	if (wMAADR1 != 0)				bDummy = 39;
	if (wMICMD != 0)				bDummy = 40;
	if (wMIREGADR != 0)				bDummy = 41;
	if (wMIWR != 0)					bDummy = 42;
	if (wMIRD != 0)					bDummy = 43;
	if (wMISTAT != 0)				bDummy = 44;
	if (wPHCON1 != 0)				bDummy = 45;
	if (wPHSTAT1 != 0)				bDummy = 46;
	if (wPHSTAT2 != 0)				bDummy = 47;
	if (wPHSTAT3 != 0)				bDummy = 48;
	if (wPHANA != 0)				bDummy = 49;
	if (wPHANLPA != 0)				bDummy = 50;
	if (wPHANE != 0)				bDummy = 51;
	if (wPHCON2 != 0)				bDummy = 52;
	if (bDummy != 0)				bDummy = 53;
}
/************************************************************************************************/
/*	SRAM�̈�24KByte��Read���� = �A��16Byte�P��													*/
/*		1.	16Byte��bData[0-15]��Read															*/
/*		2.	bData[16-31]�ɕ\��Code�Ŋi�[����													*/
/************************************************************************************************/
void Ether_ReadSram(void)
{
	USHT 	wAddress;	// 0 - SIZE_SRAM_MAX
	USHT 	wLength;	// 16
	UCHAR 	bData[32];	// [0-15] = ReadData, [16-31] = �\��Code
	UCHAR 	loop;		// 0 - 15

	/************************************/
	/*	SRAM�̈�24KByte��Read����		*/
	/************************************/
	wLength = 16;
	for(wAddress = 0; wAddress < SIZE_SRAM_MAX; wAddress += wLength) {
		/********************************/
		/*	16Byte��bData[0-15]��Read	*/
		/********************************/
		ReadMemory(	wAddress + TOP_ADDR_SRAM,	// SRAM_Address(CPU_Access���Address)
					bData,						// bData[0] - bData[15]
					wLength);					// 16Byte
		/****************************************/
		/*	bData[16-31]�ɕ\��Code�Ŋi�[����	*/
		/****************************************/
		for(loop = 0; loop < wLength; loop++) {
			if((bData[loop] >= 0x20) && (bData[loop] <= 0x7E))
				bData[loop + wLength] = bData[loop];
			else
				bData[loop + wLength] = '.';
		}
	}
}
#if 0
//	�ȉ���Excep_IRQ15�͍쐬�� ---> int_ram.c�ֈړ�����
/****************************************************************************/
/*																			*/
/*	IRQ15																	*/
/*		VectorNo		79													*/
/*		Trigger			NegativeEdge/Ethernetcontroler������				*/
/*		�M����			#ETHER_IRQ											*/
/*		PinNo			98													*/
/*																			*/
/****************************************************************************/
#endif
