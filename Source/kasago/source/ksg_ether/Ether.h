/************************************************************************************************/
/*																								*/
/*			Generic Type Definitions															*/
/*																								*/
/*	DataType_definitions																		*/
/*	Byte_Value_union_definitions																*/
/*	Word_Value_union_definitions																*/
/*	WindowNo_EtherType_LayerInfo_definitions													*/
/*	SetClearArea_offset_Value																	*/
/*	SRAM_Size_Address_definitions																*/
/*	SFR_register_addresses_difinitions															*/
/*	PHY_register_addresses_difinitions															*/
/*	SFR_register_bit_difinitions																*/
/*	structure_of_MAC_address																	*/
/*	Structure_of_RXSTATUS																		*/
/*	Structure_of_ETHER_PREAMBLE																	*/
/*	Structure_of_ETHER_FLAGS																	*/
/*	Structure_of_ETHER_HEADER																	*/
/*	Structure_of_ETHER_TOP_8BYTE																*/
/*	Structure_of_ETHER_RECV_INFO																*/
/*	Structure_of_SEND_RECEV_INFO																*/
/*																								*/
/************************************************************************************************/
//#include "iodefine.h"		// for EXIO_CS3_ETHER_CTR
/************************************************/
/*		DataType_definitions					*/
/************************************************/
//typedef enum _BOOL { FALSE = 0, TRUE } BOOL;	// Undefined size
//#define NULL    0		//((void *)0)
#define UCHAR	unsigned char		// 8-bit unsigned
#define USHT	unsigned short int	// 16-bit unsigned
#define ULNG	unsigned long		// 32-bit unsigned
#define VOID	void                // 
/************************************************/
/*		Byte_Value_union_definitions			*/
/************************************************/
typedef union 
{
    UCHAR 	Val;
    struct
    {
        unsigned char b0:1;
        unsigned char b1:1;
        unsigned char b2:1;
        unsigned char b3:1;
        unsigned char b4:1;
        unsigned char b5:1;
        unsigned char b6:1;
        unsigned char b7:1;
    } bits;
} UCHAR_VAL;
/************************************************/
/*		Word_Value_union_definitions			*/
/************************************************/
typedef union _USHT_VAL
{
    USHT Val;
    UCHAR v[2];
    struct
    {
        UCHAR LB;
        UCHAR HB;
    } byte;
    struct
    {
        unsigned char b0:1;
        unsigned char b1:1;
        unsigned char b2:1;
        unsigned char b3:1;
        unsigned char b4:1;
        unsigned char b5:1;
        unsigned char b6:1;
        unsigned char b7:1;
        unsigned char b8:1;
        unsigned char b9:1;
        unsigned char b10:1;
        unsigned char b11:1;
        unsigned char b12:1;
        unsigned char b13:1;
        unsigned char b14:1;
        unsigned char b15:1;
    } bits;
} USHT_VAL;
/************************************************/
/*	WindowNo_EtherType_LayerInfo_definitions	*/
/************************************************/
#define UDA_WINDOW		(0x1)
#define GP_WINDOW		(0x2)
#define RX_WINDOW		(0x4)
#define ETHER_IP		((USHT)0x00)
#define ETHER_ARP		((USHT)0x06)
#define MAC_IP      	(0x00u)
#define MAC_ARP     	(0x06u)
#define MAC_UNKNOWN 	(0xFFu)
/********************************/
/*	SetClearArea_offset_Value	*/
/********************************/
#define SET_OFFSET				(0x0080u<<1)	// 3F80 - 
#define CLR_OFFSET				(0x00c0u<<1)	// 3FC0 - 
/************************************/
/*	SRAM_Size_Address_definitions	*/
/************************************/
#define SIZE_SRAM_MAX			(24 * 1024ul)	// 4096(FFF) * 6 = 24576Byte(0x6000)
#define SIZE_TCP_ETH	 		0ul
#define SIZE_RESERVED_HTTP 		0ul
#define SIZE_RESERVED_SSL	 	0ul
#define SIZE_RESERVED_CRYPTO	0ul				// (128ul)
#define SIZE_TX_AREA			(1518ul)
#define ADDR_TXSTART 			(0x0000ul)
#define ADDR_RXSTART 			((ADDR_TXSTART + \
								SIZE_TX_AREA + \
								SIZE_TCP_ETH + \
								SIZE_RESERVED_HTTP + \
								SIZE_RESERVED_SSL + \
								SIZE_RESERVED_CRYPTO + \
								1ul) & 0xFFFE)
#define	ADDR_RXSTOP				(SIZE_SRAM_MAX - 1ul)
#define SIZE_RX_BUFF			(ADDR_RXSTOP - ADDR_RXSTART + 1ul)
//////////////////////////////////////////////////////
//#define BASE_TX_ADDR			(ADDR_TXSTART)
//#define BASE_TCB_ADDR			(BASE_TX_ADDR + 1518ul)
//#define BASE_HTTPB_ADDR 		(BASE_TCB_ADDR + SIZE_TCP_ETH)
//#define BASE_SSLB_ADDR		(BASE_HTTPB_ADDR + SIZE_RESERVED_HTTP)
//#define BASE_CRYPTOB_ADDR		(BASE_SSLB_ADDR + SIZE_RESERVED_SSL)
/****************************************************/
/*	SFR_register_addresses_difinitions				*/
/*		NotUsed = not used in EtherMacPhyControl.c	*/
/****************************************************/
#define TOP_ADDR_SRAM	(0x05000000 + (0x0000u<<1))	//	Top address of SRAM that CPU_Access
/****************************************************/															// PowerOn	// AfterInit
#define REG_ETXST		(0x05000000 + (0x3f00u<<1))	//	TX_Start_Address(set/clr)				(Reset = 0x0000)		0x0000	0x0000
#define REG_ETXLEN		(0x05000000 + (0x3f01u<<1))	//	TX_Length(set/clr)						(Reset = 0x0000)		0x0000	0x0000
#define REG_ERXST		(0x05000000 + (0x3f02u<<1))	//	RX_Buffer_Start_Address(set/clr)		(Reset = 0x5340)		0x5340	0x05ee
#define REG_ERXHEAD		(0x05000000 + (0x3f04u<<1))	//	RX_Head_Pointer(R)						(Reset = 0x5340)		0x5340	0x05ee
#define REG_ERXTAIL		(0x05000000 + (0x3f03u<<1))	//	RX_Tail_Pointer(set/clr)				(Reset = 0x5FFE)		0x5ffe	0x5ffe
#define REG_ETXSTAT		(0x05000000 + (0x3f09u<<1))	//	TX_Status_Reg(R)						(Reset = 0x0000)NotUsed	0x0000	0x0000
#define REG_ETXWIRE		(0x05000000 + (0x3f0au<<1))	//	TXByte_CountOnWire(R)					(Reset = 0x0000)NotUsed	0x0000	0x0000
#define REG_EUDAST		(0x05000000 + (0x3f0bu<<1))	//	User_Defined_Area_Start_Pointer(set/clr)(Reset = 0x0000)		0x0000	0x6000
#define REG_EUDAND		(0x05000000 + (0x3f0cu<<1))	//	User_Defined_Area_End_Pointer(set/clr)	(Reset = 0x5FFF)		0x5fff	0x6001
//	below is Window_Reg
#define REG_EGPDATA		(0x05000000 + (0x3f40u<<1))	//	General_Purpose_Data_Window_Reg_7_0		(Reset = 0x--??)		0x5f5f	0x2222
#define REG_ERXDATA		(0x05000000 + (0x3f41u<<1))	//	RX_Data_Window_Reg7_0					(Reset = 0x--??)		0x5f5f	0x2222
#define REG_EUDADATA	(0x05000000 + (0x3f42u<<1))	//	User_Defined_Area_Data_Window_Reg7_0	(Reset = 0x--??)		0x5f5f	0x2222
//	below is Window_Pointer
#define REG_EGPRDPT		(0x05000000 + (0x3f43u<<1))	//	General_Purpose_Window_Read_Pointer		(Reset = 0x05FA)NotUsed	0x0aa4	0x05fc
#define REG_EGPWRPT		(0x05000000 + (0x3f44u<<1))	//	General_Purpose_Window_Write_Pointer	(Reset = 0x0000)		0x0000	0x0000
#define REG_ERXRDPT		(0x05000000 + (0x3f45u<<1))	//	RX_Window_Read_Pointer					(Reset = 0x05FA)		0x0aa4	0x05fc
#define REG_ERXWRPT		(0x05000000 + (0x3f46u<<1))	//	RX_Window_Write_Pointer					(Reset = 0x0000)NotUsed	0x0000	0x0000
#define REG_EUDARDPT	(0x05000000 + (0x3f47u<<1))	//	UDA_Window_Read_Pointer					(Reset = 0x05FA)NotUsed	0x0aa4	0x05fc
#define REG_EUDAWRPT	(0x05000000 + (0x3f48u<<1))	//	UDA_Window_Write_Pointer				(Reset = 0x0000)NotUsed	0x0000	0x0000
//----------------------------------------------------------------------------------------------------------------------
#define REG_ESTAT		(0x05000000 + (0x3f0du<<1))	//	Status_Reg(R)							(Reset = 0x0000)		0x5f00	0x5f00
#define REG_ECON1		(0x05000000 + (0x3f0fu<<1))	//	Control_Reg1(set/clr)					(Reset = 0x0000)		0x0000	0x0000
#define REG_ECON2		(0x05000000 + (0x3f37u<<1))	//	Control_Reg2(set/clr)					(Reset = 0xCB00)		0xcb00	0xcb00
#define REG_EPAUS		(0x05000000 + (0x3f36u<<1))	//	Pause_Timer_Value(set/clr)				(Reset = 0x1000)NotUsed	0x1000	0x1000
#define REG_ERXWM		(0x05000000 + (0x3f38u<<1))	//	RX_Watermark_Reg(set/clr)				(Reset = 0x100F)NotUsed	0x100f	0x100f
#define REG_EIE			(0x05000000 + (0x3f39u<<1))	//	Interrupt_Enable_Reg(set/clr)			(Reset = 0x8010)		0x8010	0x8010
#define REG_EIR			(0x05000000 + (0x3f0eu<<1))	//	Intrrupt_Flag_Reg(set/clr)				(Reset = 0x0A00)		0x0e00	0x0f00
#define REG_EIDLED		(0x05000000 + (0x3f3au<<1))	//	ID_Status_Led_Control_Reg(set/clr)		(Reset = 0x26??)NotUsed	0x2621	0x2621
//----------------------------------------------------------------------------------------------------------------------
#define REG_EDMAST		(0x05000000 + (0x3f05u<<1))	//	DMA_Start_Address(set/clr)				(Reset = 0x0000)		0x0000	0x0000
#define REG_EDMALEN		(0x05000000 + (0x3f06u<<1))	//	DMA_Length(set/clr)						(Reset = 0x0000)		0x0000	0x0000
#define REG_EDMADST		(0x05000000 + (0x3f07u<<1))	//	DMA_Destination_Address(set/clr)		(Reset = 0x0000)		0x0000	0x0000
#define REG_EDMACS		(0x05000000 + (0x3f08u<<1))	//	DMA_Checksum(set/clr)					(Reset = 0x0000)		0x0000	0x0000
//----------------------------------------------------------------------------------------------------------------------
#define REG_EHT1		(0x05000000 + (0x3f10u<<1))	//	Hash_Table_Filter15_0bit(set/clr)		(Reset = 0x0000)NotUsed
#define REG_EHT2		(0x05000000 + (0x3f11u<<1))	//	Hash_Table_Filter31_16bit(set/clr)		(Reset = 0x0000)NotUsed
#define REG_EHT3		(0x05000000 + (0x3f12u<<1))	//	Hash_Table_Filter47_32bit(set/clr)		(Reset = 0x0000)NotUsed
#define REG_EHT4		(0x05000000 + (0x3f13u<<1))	//	Hash_Table_Filter63_48bit(set/clr)		(Reset = 0x0000)NotUsed
#define REG_EPMM1		(0x05000000 + (0x3f14u<<1))	//	Pattern_Match_Filter_Mask1(set/clr)		(Reset = 0x0000)NotUsed
#define REG_EPMM2		(0x05000000 + (0x3f15u<<1))	//	Pattern_Match_Filter_Mask2(set/clr)		(Reset = 0x0000)NotUsed
#define REG_EPMM3		(0x05000000 + (0x3f16u<<1))	//	Pattern_Match_Filter_Mask3(set/clr)		(Reset = 0x0000)NotUsed
#define REG_EPMM4		(0x05000000 + (0x3f17u<<1))	//	Pattern_Match_Filter_Mask4(set/clr)		(Reset = 0x0000)NotUsed
#define REG_EPMCS		(0x05000000 + (0x3f18u<<1))	//	Pattern_Match_Filter_Checksum(set/clr)	(Reset = 0x0000)NotUsed
#define REG_EPMO		(0x05000000 + (0x3f19u<<1))	//	Pattern_Match_Filter_Offset(set/clr)	(Reset = 0x0000)NotUsed
#define REG_ERXFCON		(0x05000000 + (0x3f1au<<1))	//	RX_Filter_Control_Reg(set/clr)			(Reset = 0x0059)NotUsed
//----------------------------------------------------------------------------------------------------------------------
#define REG_MACON1		(0x05000000 + (0x3f20u<<1))	//	MAC_Control_Reg1						(Reset = 0x?00D)NotUsed	0x800d	0x800d
#define REG_MACON2		(0x05000000 + (0x3f21u<<1))	//	MAC_Control_Reg2						(Reset = 0x40B2)		0x40b2	0x40b2
#define REG_MABBIPG		(0x05000000 + (0x3f22u<<1))	//	MAC_BackToBack_Inter_Packet_Gap_Reg		(Reset = 0x0012)		0x0012	0x0012
#define REG_MAIPG		(0x05000000 + (0x3f23u<<1))	//	MAC_Inter_Packet_Gap_Reg				(Reset = 0x0C12)NotUsed	0x0c12	0x0c12
#define REG_MACLCON		(0x05000000 + (0x3f24u<<1))	//	MAC_Colision_Control_Reg				(Reset = 0x370F)NotUsed	0x370f	0x370f
#define REG_MAMXFL		(0x05000000 + (0x3f25u<<1))	//	MAC_Maximum_Frame_Length				(Reset = 0x05EE)NotUsed	0x05ee	0x05ee
#define REG_MAADR3		(0x05000000 + (0x3f30u<<1))	//	MAC_Address_Byte6_15_0					(Reset = 0x????)		0xb10a	0xb10a
#define REG_MAADR2		(0x05000000 + (0x3f31u<<1))	//	MAC_Address_Byte6_31_16					(Reset = 0x??A3)		0x50a3	0x50a3
#define REG_MAADR1		(0x05000000 + (0x3f32u<<1))	//	MAC_Address_Byte6_47_32					(Reset = 0x0400)		0x0400	0x0400
//----------------------------------------------------------------------------------------------------------------------
#define REG_MICMD		(0x05000000 + (0x3f29u<<1))	//	MII_Command_Reg							(Reset = 0x--00)		0x0000	0x0000
#define REG_MIREGADR	(0x05000000 + (0x3f2au<<1))	//	MII_Address_Reg(b8 = always_1)			(Reset = 0x0100)		0x1101	0x0100
#define REG_MIWR		(0x05000000 + (0x3f33u<<1))	//	MII_Write_Data							(Reset = 0x0000)		0x0000	0x2100
#define REG_MIRD		(0x05000000 + (0x3f34u<<1))	//	MII_Read_Data							(Reset = 0x0000)		0x0000	0x0002
#define REG_MISTAT		(0x05000000 + (0x3f35u<<1))	//	MII_Status_Reg							(Reset = 0x--00)		0x0000	0x0008
/****************************************************/
/*	PHY_register_addresses_difinitions				*/
/****************************************************/
#define PHY_PHCON1	0x00u	//	PHY_CONTROL_REGISTER1								(Reset = 0x1000)	0x1000	0x2100
#define PHY_PHSTAT1	0x01u	//	PHY_STATUS_REGISTER1								(Reset = 0x7809)	0x7829	0x780d
#define PHY_PHSTAT2	0x1Bu	//	PHY_STATUS_REGISTER2								(Reset = 0x??0?)	0x001a	0x001b
#define PHY_PHSTAT3	0x1Fu	//	PHY_STATUS_REGISTER3								(Reset = 0x0040)	0x1058	0x0058
#define PHY_PHANA	0x04u	//	PHY_AUTO_NEGOTIATION_ADVERTISEMENT_REGISTER			(Reset = 0x01E1)	0x01e1	0x05e1
#define PHY_PHANLPA	0x05u	//	PHY_AUTO_NEGOTIATION_LINK_PARTNER_ABILITY_REGISTER	(Reset = 0x????)	0x05e1	0x45e1
#define PHY_PHANE	0x06u	//	PHY_AUTO_NEGOTIATION_EXPANSION_REGISTER				(Reset = 0x0000)	0x0003	0x0000
#define PHY_PHCON2	0x11u	//	PHY_CONTROL_REGISTER2								(Reset = 0x0002)	0x0002	0x0002
/****************************************************/
/*	SFR_register_bit_difinitions					*/
/****************************************************/
// -------------------------------------------------
//	REG_ETXSTAT		Reaset = 0x0000
#define ETXSTAT_LATECOL	((USHT)1<<10)	//	Transmit Late Collision Status bit(*1)(R:0)
										//		1 = A collision occurred after transmitting more than MACLCONH + 8 bytes.
										//			The last transmission was aborted. 
										//		0 = No late collision occurred during the last transmission 
#define ETXSTAT_MAXCOL	((USHT)1<<9)	//	Transmit Maximum Collisions Status bit(*1)(R:0)
										//		1 = MACLCONL + 1 collisions occurred while transmitting the last packet.
										//			The last transmission was aborted. 
										//		0 = MACLCONL or less collisions occurred while transmitting the last packet 
#define ETXSTAT_EXDEFER	((USHT)1<<8)	//	Transmit Excessive Defer Status bit(*1)(R:0)
										//		1 = The medium was busy with traffic from other nodes for more than 24,288 bit times.
										//			The last transmission was aborted. 
										//		0 = The MAC deferred for less than 24,288 bit times while transmitting the last packet 
#define ETXSTAT_DEFER	(1<<7)			//	Transmit Defer Status bit(*1)(R:0)
										//		1 = The medium was busy with traffic from other nodes,
										//			so the MAC was forced to temporarily defer transmission of the last packet 
										//		0 = No transmit deferral or an excessive deferral occurred while attempting to transmit the last packet 
#define ETXSTAT_CRCBAD	(1<<4)			//	Transmit CRC Incorrect Status bit(R:0)
										//		1 = The FCS field of the last packet transmitted did not match the CRC internally generated by the MAC during transmission 
										//		0 = The FCS field of the last packet transmitted was correct or the MAC is configured to append an internally generated CRC 
#define ETXSTAT_COLCNT3 (1<<3)			//	Transmit Collision Count Status bits<3>: (*1)(R:0)
#define ETXSTAT_COLCNT2 (1<<2)			//	<2>: (*1)(R:0)
#define ETXSTAT_COLCNT1 (1<<1)			//	<1>: (*1)(R:0)
#define ETXSTAT_COLCNT0 (1)				//	<0>: (*1)(R:0)
										//		Number of collisions that occurred while transmitting the last packet. 
//	*1: Applicable in Half-Duplex mode only; collisions and deferrals are not possible in Full-Duplex mode. 
// -------------------------------------------------
//	REG_ESTAT		Reaset = 0x0000
#define ESTAT_INT		((USHT)1<<15)	//	Interrupt Pending Status bit(R:0)
										//		1 = One of the EIR bits is set and enabled by the EIE register.
										//			If INTIE (EIE<15>) is set, the INT pin is also driven low. 
										//		0 = No enabled interrupts are currently pending. The INT pin is being driven high. 
#define ESTAT_FCIDLE	((USHT)1<<14)	//	Flow Control Idle Status bit(R:0)
										//		1 = Internal flow control state machine is Idle. It is safe to change the FCOP (ECON1<7:6>) and FULDPX (MACON2<0>) bits. 
										//		0 = Internal flow control state machine is busy. Do not modify the FCOP (ECON1<7:6>) or FULDPX (MACON2<0>) bits. 
#define ESTAT_RXBUSY	((USHT)1<<13)	//	Receive Logic Active Status bit(R:0)
										//		1 = Receive logic is currently receiving a packet.
										//			This packet may be discarded in the future if an RX buffer overflow occurs or a receive filter rejects it,
										//			so this bit does not necessarily indicate that an RX packet pending interrupt will occur. 
										//		0 = Receive logic is Idle 
#define ESTAT_CLKRDY	((USHT)1<<12)	//	Clock Ready Status bit(R:0)
										//		1 = Normal operation 
										//		0 = Internal Ethernet clocks are not running and stable yet. Only the ESTAT and EUDAST registers should be accessed. 
#define ESTAT_RSTDONE	((USHT)1<<11)	//	
#define ESTAT_PHYDPX	((USHT)1<<10)	//	PHY Full Duplex Status bit(R:0)
										//		1 = PHY is operating in Full-Duplex mode 
										//		0 = PHY is operating in Half-Duplex mode 
#define ESTAT_PHYRDY	((USHT)1<<9)	//	(R:0)
#define ESTAT_PHYLNK	((USHT)1<<8)	//	PHY Linked Status bit(R:0)
										//		1 = Ethernet link has been established with a remote Ethernet partner
										//		0 = No Ethernet link present 
#define ESTAT_PKTCNT7	(1<<7)			//	Receive Packet Count bits<7>: (R:0)
#define ESTAT_PKTCNT6	(1<<6)			//	<6>: (R:0)
#define ESTAT_PKTCNT5	(1<<5)			//	<5>: (R:0)
#define ESTAT_PKTCNT4	(1<<4)			//	<4>: (R:0)
#define ESTAT_PKTCNT3	(1<<3)			//	<3>: (R:0)
#define ESTAT_PKTCNT2	(1<<2)			//	<2>: (R:0)
#define ESTAT_PKTCNT1	(1<<1)			//	<1>: (R:0)
#define ESTAT_PKTCNT0	(1)				//	<0>: (R:0)
										//		Number of complete packets that are saved in the RX buffer and ready for software processing.
										//		Set the PKTDEC (ECON1<8>) bit to decrement this field. 
// -------------------------------------------------
//	REG_ECON1		Reaset = 0x0000
#define ECON1_MODEXST	((USHT)1<<15)	//	Modular Exponentiation Start bit(RW:0)
										//		1 = Modular exponentiation calculation started/busy; automatically cleared by hardware when done 
										//		0 = Modular exponentiation calculation done/Idle 
#define ECON1_HASHEN	((USHT)1<<14)	//	MD5/SHA-1 Hash Enable bit(RW:0)
										//		1 = MD5/SHA-1 hashing engine enabled. Data written to the hashing engine by the DMA is added to the hash. 
										//		0 = MD5/SHA-1 hashing engine disabled 
#define ECON1_HASHOP	((USHT)1<<13)	//	MD5/SHA-1 Hash Operation Control bit(RW:0)
										//		1 = MD5/SHA-1 hash engine loads the Initial Value (IV) from the hash memory.
										//			This mode is typically used for HMAC hash operations. 
										//		0 = Normal MD5/SHA-1 hash operation 
#define ECON1_HASHLST	((USHT)1<<12)	//	MD5/SHA-1 Hash Last Block Control bit(RW:0)
										//		1 = The next DMA transfer to the hash engine completes the hash.
										//			If needed, padding is automatically generated and added to the hash. 
										//		0 = The next DMA transfer to the hash engine adds data to the hash.
										//			Further data additions to the hash are still possible. 
#define ECON1_AESST		((USHT)1<<11)	//	AES Encrypt/Decrypt Start bit(RW:0)
										//		1 = AES encrypt/decrypt operation started/busy; automatically cleared by hardware when done 
										//		0 = AES encrypt/decrypt operation done/Idle 
#define ECON1_AESOP1	((USHT)1<<10)	//	AES Operation Control bits<1>:(RW:0)
#define ECON1_AESOP0	((USHT)1<<9)	//	<0>:(RW:0)
										//		10 = ECB/CBC decrypt 
										//		01 = CBC/CFB encrypt 
										//		00 = ECB/CFB/OFB encrypt or key initialization 
#define ECON1_PKTDEC	((USHT)1<<8)	//	RX Packet Counter Decrement Control bit(RW:0)
										//		1 = Decrement PKTCNT (ESTAT<7:0>) bits by one. Hardware immediately clears PKTDEC to '0', 
										//			allowing back-to-back decrement operations. 
										//		0 = Leave PKTCNT bits unchanged 
#define ECON1_FCOP1		(1<<7)			//	Flow Control Operation Control/Status bits<1>:(RW:0)
#define ECON1_FCOP0		(1<<6)			//	<0>:(RW:0)
										//		When FULDPX (MACON2<0>) = 1: 
										//			11 = End flow control by sending a pause frame with 0000h pause timer value;
										//					automatically cleared by hardware when done 
										//			10 = Enable flow control by periodically sending pause frames with a pause timer defined by EPAUS 
										//			01 = Transmit single pause frame defined by EPAUS; automatically cleared by hardware when done 
										//			00 = Flow control disabled/Idle 
										//		When FULDPX (MACON2<0>) = 0: 
										//			1x, 01 = Enable flow control by continuously asserting backpressure (transmitting preamble) 
										//			00 = Flow control disabled/Idle 
#define ECON1_DMAST		(1<<5)			//	DMA Start bit(RW:0)
										//		1 = DMA is started/busy; automatically cleared by hardware when done 
										//		0 = DMA is done/Idle 
#define ECON1_DMACPY	(1<<4)			//	DMA Copy Control bit(RW:0)
										//		1 = DMA copies data to memory location at EDMADST 
										//		0 = DMA does not copy data; EDMADST is ignored 
#define ECON1_DMACSSD	(1<<3)			//	DMA Checksum Seed Control bit(RW:0)
										//		1 = DMA checksum operations are initially seeded by the one's complement of the checksum contained in EDMACS 
										//		0 = DMA checksum operations are initially seeded by 0000h 
#define ECON1_DMANOCS	(1<<2)			//	DMA No Checksum Control bit(RW:0)
										//		1 = DMA does not compute checksums; EDMACS remains unchanged 
										//		0 = DMA computes checksums; hardware updates EDMACS at the completion of all DMA operations 
#define ECON1_TXRTS		(1<<1)			//	Transmit Request to Send Status/Control bit(RW:0)
										//		1 = Transmit an Ethernet frame; automatically cleared by hardware when done
										//		0 = Transmit logic done/Idle 
#define ECON1_RXEN		(1)				//	Receive Enable bit(RW:0)
										//		1 = Packets which pass the current RX filter configuration are written to the receive buffer 
										//		0 = All packets received are ignored 
// -------------------------------------------------
//	REG_ECON2		Reaset = 0xCB00
#define ECON2_ETHEN		((USHT)1<<15)	//	Ethernet Enable bit(RW:1)
										//		1 = Device enabled (normal operation) 
										//		0 = Device disabled (reduced power) 
#define ECON2_STRCH		((USHT)1<<14)	//	LED Stretching Enable bit(RW:1)
										//		1 = Stretch transmit, receive and collision events on LEDA and LEDB to 50 ms 
										//		0 = LEDA and LEDB outputs show real-time status without stretching 
#define ECON2_TXMAC		((USHT)1<<13)	//	Automatically Transmit MAC Address Enable bit(RW:0)
										//		1 = MAADR1-MAADR6 registers are automatically inserted
										//			into the source address field of all transmitted packets 
										//		0 = No automatic source address insertion 
#define ECON2_SHA1MD5	((USHT)1<<12)	//	SHA-1/MD5 Hash Control bit(RW:0)
										//		1 = Hashing engine computes a SHA-1 hash 
										//		0 = Hashing engine computes an MD5 hash 
#define ECON2_COCON3	((USHT)1<<11)	//	CLKOUT Frequency Control bits<3>: (*2)(RW:1)
#define ECON2_COCON2	((USHT)1<<10)	//	<2>: (*2)(RW:0)
#define ECON2_COCON1	((USHT)1<<9)	//	<1>: (*2)(RW:1)
#define ECON2_COCON0	((USHT)1<<8)	//	<0>: (*2)(RW:1)
										//		1111 = 50 kHz nominal ((4 * FOSC)/2000) 
										//		1110 = 100 kHz nominal ((4 * FOSC)/1000) 
										//		1101 = No output (DC sinking to VSS) 
										//		1100 = 3.125 MHz nominal ((4 * FOSC)/32) 
										//	o	1011 = 4.000 MHz nominal ((4 * FOSC)/25) 
										//		1010 = 5.000 MHz nominal ((4 * FOSC)/20) 
										//		1001 = 6.250 MHz nominal ((4 * FOSC)/16) 
										//		1000 = 8.000 MHz nominal ((4 * FOSC)/12.5); duty cycle is not 50% 
										//		0111 = 8.333 MHz nominal ((4 * FOSC)/12) 
										//		0110 = 10.00 MHz nominal ((4 * FOSC)/10) 
										//		0101 = 12.50 MHz nominal ((4 * FOSC)/8) 
										//		0100 = 16.67 MHz nominal ((4 * FOSC)/6) 
										//		0011 = 20.00 MHz nominal ((4 * FOSC)/5) 
										//		0010 = 25.00 MHz nominal ((4 * FOSC)/4) 
										//		0001 = 33.33 MHz nominal ((4 * FOSC)/3) 
										//		0000 = No output (DC sinking to VSS) 
#define ECON2_AUTOFC	(1<<7)			//	Automatic Flow Control Enable bit(RW:0)
										//		1 = Automatic flow control enabled 
										//		0 = Automatic flow control disabled 
#define ECON2_TXRST		(1<<6)			//	Transmit Logic Reset bit(RW:0)
										//		1 = Transmit logic is held in Reset. TXRTS (ECON1<1>) is automatically cleared by hardware when this bit is set. 
										//		0 = Transmit logic is not in Reset (normal operation) 
										//		Note 1: Reset value on POR events only. All other Resets leave these bits unchanged. 
#define ECON2_RXRST		(1<<5)			//	Receive Logic Reset bit(RW:0)
										//		1 = Receive logic is held in Reset. RXEN (ECON1<0>) is automatically cleared by hardware when this bit is set. 
										//		0 = Receive logic is not in Reset (normal operation) 
#define ECON2_ETHRST	(1<<4)			//	Master Ethernet Reset bit(RW:0)
										//		1 = All TX, RX, MAC, PHY, DMA, modular exponentiation, hashing and AES logic,
										//			and registers (excluding COCON) are reset.
										//			Hardware self-clears this bit to '0'.
										//			After setting this bit,
										//			wait at least 25 μs before attempting to read or write to the ENCX24J600 via the SPI or PSP interface. 
										//		0 = Device not in Reset (normal operation) 
#define ECON2_MODLEN1	(1<<3)			//	Modular Exponentiation Length Control bits<1>:(RW:0)
#define ECON2_MODLEN0	(1<<2)			//	<0>:(RW:0)
										//		10 = 1024-bit modulus and operands 
										//		01 = 768-bit modulus and operands 
										//		00 = 512-bit modulus and operands 
#define ECON2_AESLEN1	(1<<1)			//	AES Key Length Control bits<1>:(RW:0)
#define ECON2_AESLEN0	(1)				//	<0>:(RW:0)
										//		10 = 256-bit key 
										//		01 = 192-bit key 
										//		00 = 128-bit key 
//	*2: Reset value on POR events only. All other Resets leave these bits unchanged. 
// -------------------------------------------------
//	REG_ERXWM		Reaset = 0x100F
#define ERXWM_RXFWM7	((USHT)1<<15)	//	Set RXFWM<7> (ERXWM<15:8>) to indicate when flow control is to begin.(RW:0)
#define ERXWM_RXFWM6	((USHT)1<<14)	//	<6> (RW:0)
#define ERXWM_RXFWM5	((USHT)1<<13)	//	<5> (RW:0)
#define ERXWM_RXFWM4	((USHT)1<<12)	//	<4> (RW:1)
#define ERXWM_RXFWM3	((USHT)1<<11)	//	<3> (RW:0)
#define ERXWM_RXFWM2	((USHT)1<<10)	//	<2> (RW:0)
#define ERXWM_RXFWM1	((USHT)1<<9)	//	<1> (RW:0)
#define ERXWM_RXFWM0	((USHT)1<<8)	//	<0> (RW:0)
										//	When this number of 96-byte blocks is consumed in the receive buffer,
										//	the device considers its receive buffer to be full and initiates flow control.
										//	Use the default value of 16 if the full threshold is to be set at 1536 bytes. 
#define ERXWM_RXEWM7	(1<<7)			//	Set RXEWM<7> (ERXWM<7:0>) to indicate when flow control is to end.(RW:0)
#define ERXWM_RXEWM6	(1<<6)			//	<6> (RW:0)
#define ERXWM_RXEWM5	(1<<5)			//	<5> (RW:0)
#define ERXWM_RXEWM4	(1<<4)			//	<4> (RW:0)
#define ERXWM_RXEWM3	(1<<3)			//	<3> (RW:1)
#define ERXWM_RXEWM2	(1<<2)			//	<2> (RW:1)
#define ERXWM_RXEWM1	(1<<1)			//	<1> (RW:1)
#define ERXWM_RXEWM0	(1)				//	<0> (RW:1)
										//	When the number of occupied 96-byte blocks falls below this level, 
										//	the device considers its receive buffer to be empty enough to receive more data.
										//	Use the default value of 15 if the empty threshold is to be set at 1440 bytes. 
// -------------------------------------------------
//	REG_EIE			Reaset = 0x8010
#define EIE_INTIE		((USHT)1<<15)	//	INT Global Interrupt Enable bit(RW:1)
										//		1 = INT pin is controlled by the INT status bit (ESTAT<15>) 
										//		0 = INT pin is driven high 
#define EIE_MODEXIE		((USHT)1<<14)	//	Modular Exponentiation Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_HASHIE		((USHT)1<<13)	//	MD5/SHA-1 Hash Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_AESIE		((USHT)1<<12)	//	AES Encrypt/Decrypt Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_LINKIE		((USHT)1<<11)	//	PHY Link Status Change Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_PRDYIE		((USHT)1<<10)	//	(?)
#define EIE_PKTIE		(1<<6)			//	RX Packet Pending Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_DMAIE		(1<<5)			//	DMA Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_TXIE		(1<<3)			//	Transmit Done Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_TXABTIE		(1<<2)			//	Transmit Abort Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_RXABTIE		(1<<1)			//	Receive Abort Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
#define EIE_PCFULIE		(1)				//	Packet Counter Full Interrupt Enable bit(RW:0)
										//		1 = Enabled 
										//		0 = Disabled 
// -------------------------------------------------
//	REG_EIR			Reaset = 0x0A00
#define EIR_CRYPTEN		((USHT)1<<15)	//	Modular Exponentiation and AES Cryptographic Modules Enable bit(RW:0)
										//		1 = All cryptographic engine modules are enabled 
										//		0 = Modular exponentiation and AES modules are disabled and powered down; MD5/SHA-1 hashing is still available 
#define EIR_MODEXIF		((USHT)1<<14)	//	Modular Exponentiation Interrupt Flag bit(RW:0)
										//		1 = Modular exponentiation calculation complete 
										//		0 = No interrupt pending 
#define EIR_HASHIF		((USHT)1<<13)	//	MD5/SHA-1 Hash Interrupt Flag bit(RW:0)
										//		1 = MD5/SHA-1 hash operation complete 
										//		0 = No interrupt pending 
#define EIR_AESIF		((USHT)1<<12)	//	AES Encrypt/Decrypt Interrupt Flag bit(RW:0)
										//		1 = AES encrypt/decrypt operation complete 
										//		0 = No interrupt pending 
#define EIR_LINKIF		((USHT)1<<11)	//	PHY Link Status Change Interrupt Flag bit(RW:1)
										//		1 = PHY Ethernet link status has changed. Read PHYLNK (ESTAT<8>) to determine the current state. 
										//		0 = No interrupt pending
#define EIR_PRDYIF		((USHT)1<<10)	//	(?:?)
#define EIR_PKTIF		(1<<6)			//	RX Packet Pending Interrupt Flag bit(R:0)
										//		1 = One or more RX packets have been saved and are ready for software processing.
										//			The PKTCNT<7:0> (ESTAT<7:0>) bits are non-zero. To clear this flag,
										//			decrement the PKTCNT bits to zero by setting PKTDEC (ECON1<8>). 
										//		0 = No RX packets are pending
#define EIR_DMAIF		(1<<5)			//	DMA Interrupt Flag bit(RW:0)
										//		1 = DMA copy or checksum operation complete 
										//		0 = No interrupt pending 
#define EIR_TXIF		(1<<3)			//	Transmit Done Interrupt Flag bit(RW:0)
										//		1 = Packet transmission has completed. TXRTS (ECON1<1>) has been cleared by hardware. 
										//		0 = No interrupt pending 
#define EIR_TXABTIF		(1<<2)			//	Transmit Abort Interrupt Flag bit(RW:0)
										//		1 = Packet transmission has been aborted due to an error. Read the ETXSTAT register to determine the cause.
										//			TXRTS (ECON1<1>) has been cleared by hardware. 
										//		0 = No interrupt pending 
#define EIR_RXABTIF		(1<<1)			//	Receive Abort Interrupt Flag bit(RW:0)
										//		1 = An RX packet was dropped because there is insufficient space
										//			in the RX buffer to store the complete packet or the PKTCNT field is saturated at FFh 
										//		0 = No interrupt pending 
#define EIR_PCFULIF		(1)				//	Packet Counter Full Interrupt Flag bit(RW:0)
										//		1 = PKTCNT field has reached FFh.
										//			Software must decrement the packet counter to prevent the next RX packet from being dropped. 
										//		0 = No interrupt pending 
// -------------------------------------------------
//	REG_EIDLED		Reaset = 0x26??
#define EIDLED_LACFG3	((USHT)1<<15)	//	LEDA Configuration bits<3>:(RW:0)
#define EIDLED_LACFG2	((USHT)1<<14)	//	LEDA <2>:(RW:0)
#define EIDLED_LACFG1	((USHT)1<<13)	//	LEDA <1>:(RW:1)
#define EIDLED_LACFG0	((USHT)1<<12)	//	LEDA <0>:(RW:0)
#define EIDLED_LBCFG3	((USHT)1<<11)	//	LEDB Configuration bits<3>:(RW:0)
#define EIDLED_LBCFG2	((USHT)1<<10)	//	LEDB <2>:(RW:1)
#define EIDLED_LBCFG1	((USHT)1<<9)	//	LEDB <1>:(RW:1)
#define EIDLED_LBCFG0	((USHT)1<<8)	//	LEDB <0>:(RW:0)
										//		1111 = Display link and speed state, transmit and receive events(1) 
										//		1110 = Display link and duplex state, transmit and receive events(1) 
										//		1100 = Display link state, collision events;
										//				pin is driven high when a link is present and driven low temporarily when a collision occurs 
										//		1011 = Display link state, transmit and receive events;
										//				pin is driven high when a link is present and driven low while a packet is being received or transmitted 
										//		1010 = Display link state, receive events;
										//				pin is driven high when a link is present and driven low while a packet is being received 
										//		1001 = Display link state, transmit events;
										//				pin is driven high when a link is present and driven low while a packet is being transmitted 
										//		1000 = Display speed state; pin is driven high when in 100 Mbps mode and a link is present 
										//		0111 = Display duplex state;
										//				pin is driven high when the PHY is in full duplex (PHYDPX (ESTAT<10>) is '1') and a link is present 
										//	B	0110 = Display transmit and receive events;
										//				pin is driven high while a packet is either being received or transmitted 
										//		0101 = Display receive events; pin is driven high while a packet is being received 
										//		0100 = Display transmit events; pin is driven high while a packet is being transmitted 
										//		0011 = Display collision events; pin is temporarily driven high when a collision occurs 
										//	A	0010 = Display link state; pin is driven high when linked 
										//		0001 = On (pin is driven high) 
										//		0000 = Off (pin is driven low) 
#define EIDLED_DEVID2	(1<<7)			//	Device ID bits<2>:(R:?)
#define EIDLED_DEVID1	(1<<6)			//	<1>:(R:?)
#define EIDLED_DEVID0	(1<<5)			//	<0>:(R:?)
										//		001 = ENC624J600 family device 
#define EIDLED_REVID4	(1<<4)			//	Silicon Revision ID bits Indicates current silicon revision<4>:(R:?)
#define EIDLED_REVID3	(1<<3)			//	<3>:(R:?)
#define EIDLED_REVID2	(1<<2)			//	<2>:(R:?)
#define EIDLED_REVID1	(1<<1)			//	<1>:(R:?)
#define EIDLED_REVID0	(1)				//	<0>:(R:?)
										//		These configurations require that a bi-color LED be connected between the LEDA and LEDB pins,
										//		and that LACFG<3:0> and LBCFG<3:0> be set to the same value.
										//		See Section 2.5.1 “Using Bi-Color LEDs” for detailed information. 
// -------------------------------------------------
//	REG_ERXFCON		Reaset = 0x0059
#define ERXFCON_HTEN	((USHT)1<<15)	//	Hash Table Collection Filter Enable bit(RW:0)
										//		1 = Accept packets having a hashed destination address that points to a set bit in the Hash Table(*1) 
										//		0 = Filter disabled 
#define ERXFCON_MPEN	((USHT)1<<14)	//	Magic Packet. Collection Filter Enable it(RW:0)
										//		1 = Accept packets containing a Magic Packet pattern for the local MAC address(*1) 
										//		0 = Filter disabled 
#define ERXFCON_NOTPM	((USHT)1<<12)	//	Pattern Match Inversion Control bit(RW:0)
										//		1 = Pattern Match checksum mismatch required for a successful Pattern Match 
										//		0 = Pattern Match checksum match required for a successful Pattern Match 
#define ERXFCON_PMEN3	((USHT)1<<11)	//	Pattern Match Collection Filter Enable bits<3>:(RW:0)
#define ERXFCON_PMEN2	((USHT)1<<10)	//	<2>:(RW:0)
#define ERXFCON_PMEN1	((USHT)1<<9)	//	<1>:(RW:0)
#define ERXFCON_PMEN0	((USHT)1<<8)	//	<0>:(RW:0)
										//		When NOTPM = 0: 
										//			A packet is accepted by the filter if the pattern checksum matches AND the selected mode's condition is true. 
										//		When NOTPM = 1:
										//			A packet is accepted by the filter if pattern checksum does not match AND the selected mode's conditionis true.
										//		1001 = Magic Packet for local Unicast address(*1)
										//		1000 = Hashed packet destination points to a bit in the Hash Table registers that is set(*1)
										//		0111 = Packet destination is not the Broadcast address(*1)
										//		0110 = Packet destination is the Broadcast address(*1)
										//		0101 = Packet destination is not a Multicast address(*1)
										//		0100 = Packet destination is a Multicast address(*1)
										//		0011 = Packet destination is not local Unicast address(*1)
										//		0010 = Packet destination is local Unicast address(*1)
										//		0001 = Accept all packets with checksum match defined by NOTPM(*1)
										//	o	0000 = Filter disabled
#define ERXFCON_CRCEEN	(1<<7)			//	CRC Error Collection Filter Enable bit(RW:0)
										//		1 = Packets with an invalid CRC will be accepted, regardless of all other filter settings 
										//		0 = Filter disabled 
#define ERXFCON_CRCEN	(1<<6)			//	CRC Error Rejection Filter Enable bit(RW:1)
										//		1 = Packets with an invalid CRC will be discarded(*2) 
										//		0 = Filter disabled 
#define ERXFCON_RUNTEEN	(1<<5)			//	Runt Error Collection Filter Enable bit(RW:0)
										//		1 = Accept packets that are 63 bytes or smaller, regardless of all other filter settings 
										//		0 = Filter disabled 
#define ERXFCON_RUNTEN	(1<<4)			//	Runt Error Rejection Filter Enable bit(RW:1)
										//		1 = Discard packets that are 63 bytes or smaller(*2) 
										//		0 = Filter disabled 
#define ERXFCON_UCEN	(1<<3)			//	Unicast Destination Collection Filter Enable bit(RW:1)
										//		1 = Accept packets with a destination address matching the local MAC address(*1) 
										//		0 = Filter disabled 
#define ERXFCON_NOTMEEN	(1<<2)			//	Not-Me Unicast Destination Collection Filter Enable bit(RW:0)
										//		1 = Accept packets with a Unicast destination address that does not match the local MAC address(*1) 
										//		0 = Filter disabled 
#define ERXFCON_MCEN	(1<<1)			//	Multicast Destination Collection Filter Enable bit(RW:0)
										//		1 = Accept packets with a Multicast destination address(*1) 
										//		0 = Filter disabled 
#define ERXFCON_BCEN	(1)				//	Broadcast Destination Collection Filter Enable bit(RW:1)
										//		1 = Accept packets with a Broadcast destination address of FF-FF-FF-FF-FF-FF(*1) 
										//		0 = Filter disabled 
//	*1: This filtering decision can be overridden by the CRC Error Rejection filter
//		and Runt Error Rejection filter decisions, if enabled, by CRCEN or RUNTEN. 
//	*2: This filtering decision can be overridden by the CRC Error Collection filter
//		and Runt Error Collection filter decisions, if enabled, by CRCEEN or RUNTEEN. 
// -------------------------------------------------
//	REG_MACON1		Reaset = 0x?00D
#define MACON1_LOOPBK	(1<<4)			//	MAC Loopback Enable bit(RW:0)
										//		1 = Transmitted packets are looped back inside the MAC before reaching the PHY
										//		0 = Normal operation
#define	MACON1_RXPAUS	(1<<2)			//	Pause Control Frame Reception Enable bit(RW:1)
										//		1 = Inhibit transmissions when pause control frames are received (normal operation)
										//		0 = Ignore pause control frames which are received
#define	MACON1_PASSALL	(1<<1)			//	Pass All Received Frames Enable bit(RW:0)
										//		1 = Control frames received by the MAC are written into the receive buffer if not filtered out 
										//		0 = Control frames are discarded after being processed by the MAC (normal operation) 
// -------------------------------------------------
//	REG_MACON2		Reaset = 0x40B2
#define	MACON2_DEFER	((USHT)1<<14)	//	Defer Transmission Enable bit (applies to half duplex only)(RW:1)
										//		1 = When the medium is occupied, the MAC will wait indefinitely for it to become free
										//			when attempting to transmit (use this setting for IEEE 802.3 compliance) 
										//		0 = When the medium is occupied, the MAC will abort the transmission
										//			after the excessive deferral limit is reached (24,288 bit times) 
#define	MACON2_BPEN		((USHT)1<<13)	//	No Backoff During Back Pressure Enable bit (applies to half duplex only)(RW:0)
										//		1 = After incidentally causing a collision during back pressure, the MAC immediately begins retransmitting 
										//		0 = After incidentally causing a collision during backpressure, the MAC delays using the binary 
										//			exponential backoff algorithm before attempting to retransmit (normal operation) 
#define	MACON2_NOBKOFF	((USHT)1<<12)	//	No Backoff Enable bit (applies to half duplex only)(RW:0)
										//		1 = After any collision, the MAC immediately begins retransmitting 
										//		0 = After any collision, the MAC delays using the binary exponential backoff algorithm
										//			before attempting to retransmit (normal operation) 
#define	MACON2_PADCFG2	(1<<7)			//	Automatic Pad and CRC Configuration bits<2>:(RW:1)
#define	MACON2_PADCFG1	(1<<6)			//	<1>:(RW:0)
#define	MACON2_PADCFG0	(1<<5)			//	<0>:(RW:1)
										//		111 = All short frames are zero-padded to 64 bytes and a valid CRC is then appended 
										//		110 = No automatic padding of short frames 
										//	o	101 = MAC automatically detects VLAN protocol frames which have a 8100h type field and automatically pad to 64 bytes.
										//				If the frame is not a VLAN frame, it will be padded to 60 bytes. After padding, a valid CRC is appended. 
										//		100 = No automatic padding of short frames 
										//		011 = All short frames are zero-padded to 64 bytes and a valid CRC is then appended 
										//		010 = No automatic padding of short frames 
										//		001 = All short frames will be zero-padded to 60 bytes and a valid CRC is then appended 
										//		000 = No automatic padding of short frames 
#define	MACON2_TXCRCEN	(1<<4)			//	Transmit CRC Enable bit(RW:1)
										//		1 = MAC appends a valid CRC to all frames transmitted regardless of the PADCFG bits.
										//			TXCRCEN must be set if the PADCFG bits specify that a valid CRC will be appended. 
										//		0 = MAC does not append a CRC.
										//			The last 4 bytes are checked and if it is an invalid CRC,
										//			it is to be reported by setting CRCBAD (ETXSTAT<4>). 
#define	MACON2_PHDREN	(1<<3)			//	Proprietary Header Enable bit(RW:0)
										//		1 = Frames presented to the MAC contain a 4-byte proprietary header which is not used
										//			when calculating the CRC 
										//		0 = No proprietary header is present; the CRC covers all data (normal operation) 
#define	MACON2_HFRMEN	(1<<2)			//	Huge Frame Enable bit(RW:0)
										//		1 = Frames of any size will be allowed to be transmitted and received
										//		0 = Frames bigger than MAMXFL will be aborted when transmitted or received 
#define	MACON2_FULDPX	(1)				//	MAC Full-Duplex Enable bit(RW:0)
										//		1 = MAC operates in Full-Duplex mode. For proper operation, the PHY must also be set to Full-Duplex mode. 
										//		0 = MAC operates in Half-Duplex mode. For proper operation, the PHY must also be set to Half-Duplex mode. 
// -------------------------------------------------
//	REG_MABBIPG		Reaset = 0x0012
#define MABBIPG_BBIPG6	(1<<6)			//	Back-to-Back Inter-Packet Gap Delay Time Control bits <6>:(RW:0) = 0x12
#define MABBIPG_BBIPG5	(1<<5)			//	<5>:(RW:0)
#define MABBIPG_BBIPG4	(1<<4)			//	<4>:(RW:1)
#define MABBIPG_BBIPG3	(1<<3)			//	<3>:(RW:0)
#define MABBIPG_BBIPG2	(1<<2)			//	<2>:(RW:0)
#define MABBIPG_BBIPG1	(1<<1)			//	<1>:(RW:1)
#define MABBIPG_BBIPG0	(1)				//	<0>:(RW:0)
										//		When FULDPX (MACON2<0>) = 1: 
										//			Nibble time offset delay between the end of one transmission and the beginning of the next in a back-to-back sequence.
										//			The register value should be programmed to the desired period in nibble times minus 3.
										//			The recommended setting is 15h
										//			which represents the minimum IEEE specified Inter-Packet Gap (IPG) of 0.96 μs (at 100 Mb/s) or 9.6 μs (at 10 Mb/s). 
										//		When FULDPX (MACON2<0>) = 0: 
										//			Nibble time offset delay between the end of one transmission and the beginning of the next in a back-to-back sequence.
										//			The register value should be programmed to the desired period in nibble times minus 6.
										//			The recommended setting is 12h
										//			which represents the minimum IEEE specified Inter-Packet Gap (IPG) of 0.96 μs (at 100 Mb/s) or 9.6 μs (at 10 Mb/s). 
// -------------------------------------------------
//	REG_MAIPG		Reaset = 0x0C12
#define MAIPG_IPG6		(1<<6)			//	Non Back-to-Back Inter-Packet Gap Delay Time Control bits<6>:(RW:0) = 0x12
#define MAIPG_IPG5		(1<<5)			//	<5>:(RW:0)
#define MAIPG_IPG4		(1<<4)			//	<4>:(RW:1)
#define MAIPG_IPG3		(1<<3)			//	<3>:(RW:0)
#define MAIPG_IPG2		(1<<2)			//	<2>:(RW:0)
#define MAIPG_IPG1		(1<<1)			//	<1>:(RW:1)
#define MAIPG_IPG0		(1)				//	<0>:(RW:0)
										//		Inter-Packet Gap (IPG) between the end of one packet received or transmitted and the start of the next packet transmitted.
										//		For maximum performance while meeting IEEE 802.3 compliance,
										//		leave this field set to 12h, which represents an Inter-Packet Gap time of 0.96 μs (at 100 Mb/s) or 9.6 μs (at 10 Mb/s). 
// -------------------------------------------------
//	REG_MACLCON		Reaset = 0x370F
#define MACLCON_MAXRET3	(1<<3)			//	Maximum Retransmissions Control bits<3>:(half duplex only)(RW:1)
#define MACLCON_MAXRET2	(1<<2)			//	<2>:(half duplex only)(RW:1)
#define MACLCON_MAXRET1	(1<<1)			//	<1>:(half duplex only)(RW:1)
#define MACLCON_MAXRET0	(1)				//	<0>:(half duplex only)(RW:1)
										//		Maximum retransmission attempts the MAC will make before aborting a packet due to excessive collisions. 
// -------------------------------------------------
//	REG_MICMD		Reaset = 0x--00
#define	MICMD_MIISCAN	(1<<1)			//	MII Scan Enable bit(RW:0)
										//		1 = PHY register designated by MIREGADR<4:0> is continuously read and the data is copied to MIRD 
										//		0 = No MII Management scan operation is in progress 
#define	MICMD_MIIRD		(1)				//	MII Read Enable bit(RW:0)
										//		1 = PHY register designated by MIREGADR<4:0> is read once and the data is copied to MIRD
										//		0 = No MII Management read operation is in progress
// -------------------------------------------------
//	REG_MIREGADR	Reaset = 0x0100
#define MIREGADR_PHREG4	(1<<4)			//	MII Management PHY Register Address Select bits<4>:(RW:0)
#define MIREGADR_PHREG3	(1<<3)			//	<3>:(RW:0)
#define MIREGADR_PHREG2	(1<<2)			//	<2>:(RW:0)
#define MIREGADR_PHREG1	(1<<1)			//	<1>:(RW:0)
#define MIREGADR_PHREG0	(1)				//	<0>:(RW:0)
										//		Address of the PHY register which MII Management read and write operations will apply to. 
// -------------------------------------------------
//	REG_MISTAT		Reaset = 0x--00
#define	MISTAT_NVALID	(1<<2)			//	MII Management Read Data Not Valid Status bit(R:0)
										//		1 = The contents of MIRD are not valid yet 
										//		0 = The MII Management read cycle has completed and MIRD has been updated 
#define	MISTAT_SCAN		(1<<1)			//	MII Management Scan Status bit(R:0)
										//		1 = MII Management scan operation is in progress 
										//		0 = No MII Management scan operation is in progress 
#define	MISTAT_BUSY		(1)				//	MII Management Busy Status bit(R:0)
										//		1 = A PHY register is currently being read or written to 
										//		0 = The MII Management interface is Idle 
// -------------------------------------------------
//	PHY_PHCON1	Reset = 0x1000
#define PHCON1_PRST		((USHT)1<<15)	//	PHY Reset bit(RW:0)
										//		1 = Perform PHY Reset.
										//			Hardware automatically clears this bit to '0' when the Reset is complete. 
										//		0 = PHY not in Reset (normal operation) 
#define PHCON1_PLOOPBK	((USHT)1<<14)	//	PHY Loopback Enable bit(RW:0)
										//		1 = Loopback enabled
										//		0 = Normal operation
#define PHCON1_SPD100	((USHT)1<<13)	//	PHY Speed Select Control bit(*1)(RW:0)
										//		1 = 100Mbps
										//		0 = 10Mbps
#define PHCON1_ANEN		((USHT)1<<12)	//	PHY Auto-Negotiation Enable bit(RW:1)
										//		1 = Auto-negotiation enabled.
										//			SPD100 and PFULDPX are ignored.
										//		0 = Auto-negotiation disabled.
										//			SPD100 and PFULDPX control the operating speed and duplex. 
#define PHCON1_PSLEEP	((USHT)1<<11)	//	PHY Sleep Enable bit(RW:0)
										//		1 = PHY powered down
										//		0 = Normal operation
#define PHCON1_RENEG	((USHT)1<<9)	//	Restart Auto-Negotiation Control bit(RW:0)
										//		1 = Restart the auto-negotiation process.
										//			Hardware automatically clears this bit to '0'
										//			when the auto-negotiation process starts.
										//		0 = Normal operation
#define PHCON1_PFULDPX	((USHT)1<<8)	//	PHY Duplex Select Control bit(*1)(RW:0)
										//		1 = Full duplex
										//		0 = Half duplex
//	*1: Applicable only when auto-negotiation is disabled (ANEN = 0). 
// -------------------------------------------------
//	PHY_PHSTAT1	Reset = 0x7809
#define PHSTAT1_FULL100	((USHT)1<<14)	//	100Base-TX Full-Duplex Ability Status bit(R:1)
										//		1 = PHY is capable of 100Base-TX full-duplex operation(*1) 
#define PHSTAT1_HALF100	((USHT)1<<13)	//	100Base-TX Half-Duplex Ability Status bit(R:1)
										//		1 = PHY is capable of 100Base-TX half-duplex operation(*1) 
#define PHSTAT1_FULL10	((USHT)1<<12)	//	10Base-T Full-Duplex Ability Status bit(R:1)
										//		1 = PHY is capable of 10Base-T full-duplex operation(*1) 
#define PHSTAT1_HALF10	((USHT)1<<11)	//	10Base-T Half-Duplex Ability Status bit(R:1)
										//		1 = PHY is capable of 10Base-T half-duplex operation(*1) 
#define PHSTAT1_ANDONE	(1<<5)			//	Auto-Negotiation Done Status bit(R:0)
										//		1 = Auto-negotiation is complete 
										//		0 = Auto-negotiation is disabled or still in progress 
#define PHSTAT1_LRFAULT	(1<<4)			//	Latching Remote Fault Condition Status bit(R:0)
										//		1 = Remote Fault condition has been detected. This bit latches high
										//			and automatically returns to '0' after PHSTAT1 is read. 
										//		0 = No remote Fault has been detected since the last read of PHSTAT1 
#define PHSTAT1_ANABLE	(1<<3)			//	Auto-Negotiation Ability Status bit(R:1)
										//		1 = PHY is capable of auto-negotiation(*1)
#define PHSTAT1_LLSTAT	(1<<2)			//	Latching Link Status bit(R:0)
										//		1 = Ethernet link is established and has stayed continuously established since the last read of PHSTAT1 
										//		0 = Ethernet link is not established or was not established for a period since the last read of PHSTAT1 
#define PHSTAT1_EXTREGS	(1)				//	Extended Capabilities Registers Present Status bit(R:1)
										//		1 = PHY has extended capability registers at addresses, 16 thru 31(*1) 
//	*1: This is the only valid state for this bit; a '0' represents an invalid condition. 
// -------------------------------------------------
//	PHY_PHSTAT2	Reset = 0x??0?
#define PHSTAT2_PLRITY	(1<<4)			//	TPIN+/- Polarity Status bit (applies to 10Base-T only)(R:0)
										//		1 = Wiring on TPIN+/- pins is reversed polarity.
										//			PHY internally swaps the TPIN+/- signals to get the correct polarity. 
										//		0 = Wiring on TPIN+/- is correct polarity 
// -------------------------------------------------
//	PHY_PHSTAT3	Reset = 0x0040
#define PHSTAT3_SPDDPX2	(1<<4)			//	Current Operating Speed and Duplex Status bits<2>:(R:0)
#define PHSTAT3_SPDDPX1	(1<<3)			//	<1>:(R:0)
#define PHSTAT3_SPDDPX0	(1<<2)			//	<0>:(R:0)
										//		110 = 100Mbps, full duplex 
										//		101 = 10 Mbps, full duplex 
										//		010 = 100Mbps, half duplex 
										//		001 = 10 Mbps, half duplex 
// -------------------------------------------------
//	PHY_PHANA	Reset = 0x01E1
#define PHANA_ADNP		((USHT)1<<15)	//	Advertise Next Page Ability bit(RW:0)
										//		1 = Invalid 
										//		0 = Local PHY does not support auto-negotiation next page abilities 
#define PHANA_ADFAULT	((USHT)1<<13)	//	Advertise Remote Fault Condition bit(RW:0)
										//		1 = Local PHY has a Fault condition present
										//		0 = Local PHY does not have a Fault condition present
#define PHANA_ADPAUS1	((USHT)1<<11)	//	Advertise PAUSE Flow Control Ability bits<1>:(RW:0)
#define PHANA_ADPAUS0	((USHT)1<<10)	//	Advertise PAUSE Flow Control Ability bits<0>:(RW:0)
										//		11 = Local device supports both symmetric PAUSE and asymmetric PAUSE toward local device 
										//		10 = Local device supports asymmetric PAUSE toward link partner only 
										//		01 = Local device supports symmetric PAUSE only (Normal Flow Control mode) 
										//		00 = Local device does not support PAUSE flow control 
#define PHANA_AD100FD	((USHT)1<<8)	//	Advertise 100Base-TX Full-Duplex Ability bit(RW:1)
										//		1 = Local PHY is capable of 100Base-TX full-duplex operation 
										//		0 = Local PHY is incapable of 100Base-TX full-duplex operation 
#define PHANA_AD100		(1<<7)			//	Advertise 100Base-TX Half-Duplex Ability bit(RW:1)
										//		1 = Local PHY is capable of 100Base-TX half-duplex operation 
										//		0 = Local PHY is incapable of 100Base-TX half-duplex operation 
#define PHANA_AD10FD	(1<<6)			//	Advertise 10Base-T Full-Duplex Ability bit(RW:1)
										//		1 = Local PHY is capable of 10Base-T full-duplex operation 
										//		0 = Local PHY is incapable of 10Base-T full-duplex operation 
#define PHANA_AD10		(1<<5)			//	Advertise 10Base-T Half-Duplex Ability bit(RW:1)
										//		1 = Local PHY is capable of 10Base-T half-duplex operation 
										//		0 = Local PHY is incapable of 10Base-T half-duplex operation 
#define PHANA_ADIEEE4	(1<<4)			//	Advertise IEEE Standard Selector Field bits<4>:(RW:0)
#define PHANA_ADIEEE3	(1<<3)			//	<3>:(RW:0)
#define PHANA_ADIEEE2	(1<<2)			//	<2>:(RW:0)
#define PHANA_ADIEEE1	(1<<1)			//	<1>:(RW:0)
#define PHANA_ADIEEE0	(1)				//	<0>:(RW:1)
										//		00001 = IEEE 802.3 Std. 
										//		All other values reserved by IEEE.
										//		Always specify a selector value of '00001' for this device. 
// -------------------------------------------------
//	PHY_PHANLPA	Reset = 0x????
#define PHANLPA_LPNP	((USHT)1<<15)	//	Link Partner Next Page Ability bit(R:?)
										//		1 = Link partner PHY supports auto-negotiation next page abilities 
										//		0 = Link partner PHY does not support auto-negotiation next page abilities 
#define PHANLPA_LPACK	((USHT)1<<14)	//	Link Partner Acknowledge Local PHY Code Word Status bit(R:?)
										//		1 = Link partner PHY has successfully received the local PHY abilities saved in PHANA 
										//		0 = Link partner PHY has not received the local PHY abilities saved in PHANA 
#define PHANLPA_LPFAULT	((USHT)1<<13)	//	Link Partner Remote Fault Condition bit(R:?)
										//		1 = Link partner PHY has a Fault condition present 
										//		0 = Link partner PHY does not have a Fault condition present 
#define PHANLPA_LPPAUS1	((USHT)1<<11)	//	Link Partner PAUSE Flow Control Ability bits<1>:(R:?)
#define PHANLPA_LPPAUS0	((USHT)1<<10)	//	<0>:(R:?)
										//		11 = Link partner supports both symmetric PAUSE and asymmetric PAUSE toward local device.
										//			Link partner generates and responds to PAUSE control frames.
										//			Alternatively, if the local device only supports asymmetric PAUSE,
										//			the link partner will respond to PAUSE control frames, but not generate any. 
										//		10 = Link partner supports asymmetric PAUSE toward local device only;
										//			it can transmit PAUSE control frames, but cannot act upon PAUSE frames sent to it 
										//		01 = Link partner supports symmetric PAUSE only, and generates and responds to PAUSE control frames 
										//		00 = Link partner does not support PAUSE flow control 
#define PHANLPA_LP100T4	((USHT)1<<9)	//	Link Partner 100Base-T4 Ability bit(R:?)
										//		1 = Link partner PHY is capable of operating in 100Base-T4 mode 
										//		0 = Link partner PHY is incapable of operating in 100Base-T4 mode 
#define PHANLPA_LP100FD	((USHT)1<<8)	//	Link Partner 100Base-TX Full-Duplex Ability bit(R:?)
										//		1 = Link partner PHY is capable of 100Base-TX full-duplex operation 
										//		0 = Link partner PHY is incapable of 100Base-TX full-duplex operation 
#define PHANLPA_LP100	(1<<7)			//	Link Partner 100Base-TX Half-Duplex Ability bit(R:?)
										//		1 = Link partner PHY is capable of 100Base-TX half-duplex operation 
										//		0 = Link partner PHY is incapable of 100Base-TX half-duplex operation 
#define PHANLPA_LP10FD	(1<<6)			//	Link Partner 10Base-T Full-Duplex Ability bit(R:?)
										//		1 = Link partner PHY is capable of 10Base-T full-duplex operation 
										//		0 = Link partner PHY is incapable of 10Base-T full-duplex operation 
#define PHANLPA_LP10	(1<<5)			//	Link Partner 10Base-T Half-Duplex Ability bit(R:?)
										//		1 = Link partner PHY is capable of 10Base-T half-duplex operation 
										//		0 = Link partner PHY is incapable of 10Base-T half-duplex operation 
#define PHANLPA_LPIEEE4	(1<<4)			//	Link Partner IEEE Standard Selector Field bits<4>:(R:?)
#define PHANLPA_LPIEEE3	(1<<3)			//	<3>:(R:?)
#define PHANLPA_LPIEEE2	(1<<2)			//	<2>:(R:?)
#define PHANLPA_LPIEEE1	(1<<1)			//	<1>:(R:?)
#define PHANLPA_LPIEEE0	(1)				//	<0>:(R:?)
										//		00001 = IEEE 802.3 Std. 
										//		All other values are reserved by IEEE.
										//		Remote node should also specify this as the selector value. 
// -------------------------------------------------
//	PHY_PHANE	Reset = 0x0000
#define PHANE_PDFLT		(1<<4)			//	Parallel Detection Fault Status bit(R:0)
										//		1 = Parallel detection did not detect a valid link partner; automatically cleared when register is read 
										//		0 = Parallel detection is still in progress or a valid link partner is connected 
#define PHANE_LPARCD	(1<<1)			//	Link Partner Abilities Received Status bit(R:0)
										//		1 = PHANLPA register has been written with a new value from the link partner;
										//			automatically cleared when register is read 
										//		0 = PHANLPA contents have not changed since the last read of PHANE 
#define PHANA_LPANABL	(1)				//	Link Partner Auto-Negotiation Able Status bit(R:0)
										//		1 = Link partner implements auto-negotiation 
										//		0 = Link partner does not implement auto-negotiation 
// -------------------------------------------------
//	PHY_PHCON2	Reset = 0x0002
#define PHCON2_EDPWRDN	((USHT)1<<13)	//	Energy Detect Power-Down Enable bit(RW:0)
										//		1 = Energy detect power-down enabled.
										//			PHY automatically powers up and down based on the state of EDSTAT. 
										//		0 = Energy detect power-down disabled.
										//			Use this setting for maximal compatibility. 
#define PHCON2_EDTHRES	((USHT)1<<11)	//	Energy Detect Threshold Control bit(RW:0)
										//		1 = Less energy is required to wake the PHY from energy detect power-down 
										//		0 = Normal energy detect threshold 
#define PHCON2_FRCLNK	(1<<2)			//	Force Link Control bit(RW:0)
										//		1 = Force immediate link up, even when no link partner is present (100 Mbps operation only)(*1) 
										//		0 = Normal operation 
#define PHCON2_EDSTAT	(1<<1)			//	Energy Detect Status bit(R:1)
										//		1 = Energy detect circuit has detected energy on the TPIN+/- pins within the last 256 ms 
										//		0 = No energy has been detected on the TPIN+/- pins within the last 256 ms 
//	*1: Intended for testing purposes only. Do not use in 10 Mbps operation. 
//-----------------------------------------------------------------------------------------------------------------------
/********************************/
/*	structure_of_MAC_address	*/
/********************************/
typedef struct {
	UCHAR	v[6];
} MAC_ADDR;						// MyMacAddr
/********************************/
/*	Structure_of_RXSTATUS		*/
/********************************/
typedef union _t_RXSTATUS {
	UCHAR v[6];
	struct {
		USHT	 		ByteCount;					// 16bit
		unsigned char	PreviouslyIgnored:1;		// 0
		unsigned char	RXDCPreviouslySeen:1;		// 1
		unsigned char	CarrierPreviouslySeen:1;	// 2
		unsigned char	CodeViolation:1;			// 3
		unsigned char	CRCError:1;					// 4
		unsigned char	LengthCheckError:1;			// 5
		unsigned char	LengthOutOfRange:1;			// 6
		unsigned char	ReceiveOk:1;				// 7
		unsigned char	Multicast:1;				// 8
		unsigned char	Broadcast:1;				// 9
		unsigned char	DribbleNibble:1;			// 10
		unsigned char	ControlFrame:1;				// 11
		unsigned char	PauseControlFrame:1;		// 12
		unsigned char	UnsupportedOpcode:1;		// 13
		unsigned char	VLANType:1;					// 14
		unsigned char	RuntMatch:1;				// 15
		unsigned char	filler:1;					// 16
		unsigned char	HashMatch:1;				// 17
		unsigned char	MagicPacketMatch:1;			// 18
		unsigned char	PatternMatch:1;				// 19
		unsigned char	UnicastMatch:1;				// 20
		unsigned char	BroadcastMatch:1;			// 21
		unsigned char	MulticastMatch:1;			// 22
		unsigned char	ZeroH:1;					// 23
		unsigned char	Zero:8;						// 24-31
	} bits;
} RXSTATUS;
/************************************/
/*	Structure_of_ETHER_PREAMBLE		*/
/************************************/
typedef struct  _t_ETHER_PREAMBLE {
	USHT			NextPacketPointer;	// 
	RXSTATUS		StatusVector;		// 
	MAC_ADDR		DestMACAddr;		// 
	MAC_ADDR		SourceMACAddr;		// 
	USHT_VAL		Type;				// 
} ETHER_PREAMBLE;
/************************************/
/*	Structure_of_ETHER_FLAGS		*/
/************************************/
typedef struct _t_ETHER_FLAGS {
	unsigned char bWasDiscarded:1;	// Status tracking bit indicating(初期値 = 1)
									// if the last received packet has been discarded via MACDiscardRx() or not.
	unsigned char PoweredDown:1;	// Local cached bit indicating CRYPTEN == ETHEN == 0 and PSLEEP == 1(初期値 = 0)
	unsigned char CryptoEnabled:1;	// Local cached bit indicating CRYPTEN == 1(初期値 = 0)
	unsigned char bDummy:1;			// 
} ETHER_FLAGS;						// EtherFlags
/************************************/
/*	Structure_of_ETHER_HEADER		*/
/************************************/
typedef struct {
	MAC_ADDR        DestMACAddr;
	MAC_ADDR        SourceMACAddr;
	USHT_VAL        Type;
} ETHER_HEADER;
/************************************/
/*	Structure_of_ETHER_TOP_8BYTE	*/
/************************************/
typedef struct {
	USHT			NextPacketPointer;	// 
	RXSTATUS		StatusVector;		// 
} ETHER_TOP_8BYTE;						// EncPreamble
/************************************/
/*	Structure_of_ETHER_RECV_INFO	*/
/************************************/
#define					ETHER_RECV_MAX	0x20
#define					ETHER_RECV_MASK	(ETHER_RECV_MAX-1)
#define					ETHER_RECV_PKT_MAX	1518

typedef struct {
	USHT				ByteCnt[ETHER_RECV_MAX];			// 
	USHT				PktCnt;				// 
	USHT				PktFirstSaveBoxNo;	// 
	UCHAR				Buff[ETHER_RECV_MAX][ETHER_RECV_PKT_MAX];		// for MultiRecvPackets in 1_irq
} ETHER_RECV_INFO;							// EtheRecvInfo

/************************************/
/*	Structure_of_SEND_RECEV_INFO	*/
/************************************/
#define SIZE_I15_EIR	256
typedef struct {
	USHT	I15cnt;					// IRQ15の発生回数(I15_EIRのIndex)
	USHT	I15_EIR[SIZE_I15_EIR];	// IIRQ15が発生した時のEIRの状態
	USHT	PhylinkStatus;			// PHYLNKのCurrent状態
	USHT	SendByteCnt;			// 今回送信したByte数
	USHT	SendPktInt;				// IRQ15で送信完了した割込みの発生回数
	USHT	RecvPktInt;				// IRQ15で受信Packetを受信した割込みの発生回数
	USHT	RecvPktCnt;				// IRQ15で受信PacketのPKTCNTの累計回数
	USHT	RecvPktNowBoxNo;		// IRQ15で受信PacketのCurrentのPKTCNTの回数のBoxNo
	USHT	RecvPktNow[4];			// IRQ15で受信PacketのCurrentのPKTCNTの回数(EtherRecvで減算)
	USHT	RecvPktMax;				// IRQ15で受信PacketのPKTCNTのMax値
	USHT	kasagoAPI_CallCnt;		// tfNotifyInterfaceIsr()をcallした数
	USHT	k_OpenCnt;				// kasagoよりEtherOpen()された発生回数
	USHT	k_CloseCnt;				// kasagoよりEtherClose()された発生回数
	USHT	k_SendCnt;				// kasagoよりEtherSend()された発生回数
	USHT	k_ReceiveCnt;			// kasagoよりEtherReceive()された発生回数
	USHT	k_FreeCnt;				// kasagoよりEtherFreeReceiveBuffer()された発生回数
	USHT	k_IoctlCnt;				// kasagoよりEtherIoctlFunc()された発生回数
	USHT	k_GetPhyCnt;			// kasagoよりEtherGetPhyAddr()された発生回数
	USHT	ERR_EIR_TXABTIF;		// 1 = Packet transmission has been aborted due to an error.
	USHT	ERR_EIR_RXABTIF;		// 1 = An RX packet was dropped because there is insufficient space
	USHT	ERR_EIR_PCFULIF;		// 1 = Packet Counter Full Interrupt Flag bit(RW:0)
	USHT	ERR_ESTAT_RXBUSY;		// 1 = Receive logic is currently receiving a packet.(R:0)
	USHT	ERR_NextRxPtr_MAX;		// 
	USHT	ERR_ByteCount_MAX;		// 
	USHT	I15cnt_BeforeOpen;		// Open前に発生したIRQ15の発生回数(I15_EIRのIndex)
	USHT	I15_EIR_BeforeOpen[16];	// Open前に発生したIIRQ15が発生した時のEIRの状態
} SEND_RECEV_INFO;					// SendRecvInfo
