#include "extern_Debugdata.h"

unsigned short		DBG_cs4_exio_in_data;
////////////////////////////////////////////////
unsigned short		DBG_nmi_cnt;
////////////////////////////////////////////////
unsigned short		DBG_tpu0_cnt;
unsigned short		DBG_tpu1_cnt;
unsigned short		DBG_tpu2_cnt;
////////////////////////////////////////////////
unsigned short		DBG_irq4_cnt;
unsigned short		DBG_irq5_cnt;
unsigned short		DBG_irq8_cnt;
unsigned short		DBG_irq15_cnt;
////////////////////////////////////////////////
unsigned short		DBG_can1_RXM1_cnt;
unsigned short		DBG_can1_TXM1_cnt;
unsigned short		DBG_cani_ERSi_cnt;
////////////////////////////////////////////////
unsigned short		DBG_i2c_EEI1_cnt;
unsigned short		DBG_i2c_RXI1_cnt;
unsigned short		DBG_i2c_TXI1_cnt;
unsigned short		DBG_i2c_TEI1_cnt;
////////////////////////////////////////////////
unsigned short		DBG_sci2_RXI2_cnt;
unsigned short		DBG_sci2_TXI2_cnt;
unsigned short		DBG_sci2_TEI2_cnt;
unsigned short		DBG_sci3_RXI3_cnt;
unsigned short		DBG_sci3_TXI3_cnt;
unsigned short		DBG_sci3_TEI3_cnt;
unsigned short		DBG_sci4_RXI4_cnt;
unsigned short		DBG_sci4_TXI4_cnt;
unsigned short		DBG_sci4_TEI4_cnt;
unsigned short		DBG_sci6_RXI6_cnt;
unsigned short		DBG_sci6_TXI6_cnt;
unsigned short		DBG_sci6_TEI6_cnt;
unsigned short		DBG_sci7_RXI7_cnt;
unsigned short		DBG_sci7_TXI7_cnt;
unsigned short		DBG_sci7_TEI7_cnt;
unsigned short		DBG_sci9_RXI9_cnt;
unsigned short		DBG_sci9_TXI9_cnt;
unsigned short		DBG_sci9_TEI9_cnt;
unsigned short		DBG_sci10_RXI10_cnt;
unsigned short		DBG_sci10_TXI10_cnt;
unsigned short		DBG_sci10_TEI10_cnt;
unsigned short		DBG_sci11_RXI11_cnt;
unsigned short		DBG_sci11_TXI11_cnt;
unsigned short		DBG_sci11_TEI11_cnt;
/****************************************************************************/
/*	I2C1 Debug Information after Initialize									*/
/****************************************************************************/
//-------------------------------------------------------------------------------------------
//										7		6		5		4		3		2		1		0
//-------------------------------------------------------------------------------------------
unsigned char	DBG_I2C_ICCR1;		//	ICE 	IICRST 	CLO 	SOWP 	SCLO 	SDAO 	SCLI:R 	SDAI:R	ControlRegister1
unsigned char	DBG_I2C_ICCR2;		//	BBSY:R 	MST 	TRS 	Å| 		SP 		RS 		ST 		Å|		ControlRegister2
unsigned char	DBG_I2C_ICMR1;		//	MTWP 	CKS[2:0] 				BCWP 	BC[2:0]					ModeRegister1
unsigned char	DBG_I2C_ICMR2;		//	DLCS 	SDDL[2:0] 				Å| 		TMOH 	TMOL 	TMOS	ModeRegister2
unsigned char	DBG_I2C_ICMR3;		//	SMBS 	WAIT 	RDRFS 	ACKWP:W	ACKBT 	ACKBR:R	NF[1:0]			ModeRegister3
unsigned char	DBG_I2C_ICFER;		//	FMPE 	SCLE 	NFE 	NACKE 	SALE 	NALE 	MALE 	TMOE	FunctionEnableRegister
unsigned char	DBG_I2C_ICSER;		//	HOAE 	Å| 		DIDE 	Å| 		GCAE 	SAR2E 	SAR1E 	SAR0E	StatusEnableRegister
unsigned char	DBG_I2C_ICIER;		//	TIE 	TEIE 	RIE 	NAKIE 	SPIE 	STIE 	ALIE 	TMOIE	InterruptEnableRegister
unsigned char	DBG_I2C_ICSR1;		//	HOA 	Å| 		DID 	Å| 		GCA 	AAS2 	AAS1 	AAS0	StatusRegister1
unsigned char	DBG_I2C_ICSR2;		//	TDRE:R 	TEND 	RDRF 	NACKF 	STOP 	START 	AL 		TMOF	StatusRegister2
unsigned char	DBG_I2C_SARL0;		//	SVA[6:0] 												SVA0	SlaveAddressRegisterL0
unsigned char	DBG_I2C_SARU0;		//	Å| 		Å| 		Å| 		Å| 		Å| 		SVA[1:0] 		FS		SlaveAddressRegisterU0
unsigned char	DBG_I2C_SARL1;		//	SVA[6:0] 												SVA0	SlaveAddressRegisterL1
unsigned char	DBG_I2C_SARU1;		//	Å| 		Å| 		Å| 		Å| 		Å| 		SVA[1:0] 		FS		SlaveAddressRegisterU1
unsigned char	DBG_I2C_SARL2;		//	SVA[6:0] 												SVA0	SlaveAddressRegisterL2
unsigned char	DBG_I2C_SARU2;		//	Å| 		Å| 		Å| 		Å| 		Å| 		SVA[1:0] 		FS		SlaveAddressRegisterU2
unsigned char	DBG_I2C_ICBRL;		//	Å| 		Å| 		Å| 		BRL[4:0]								BitRateLowLevelRegister
unsigned char	DBG_I2C_ICBRH;		//	Å| 		Å| 		Å| 		BRH[4:0]								BitRateHighLevelRegister
unsigned char	DBG_I2C_ICDRT;		//	SEND_DATA[7:0]													ëóêMDataRegister
unsigned char	DBG_I2C_ICDRR;		//	RECV_DATA[7:0]:R												éÛêMDataRegister	/********************************************/
	/*	Port0									*/
	/*		P00	TXD6	O		BT_TXD			*/
	/*		P01	RXD6	I		BT_RXD			*/
	/*		P02	P02		O(L)	#MUTE			*/
	/*		P03	P03		O(L)	FBSH_O			*/
	/*		P05	P05		O(L)	FBSH_C			*/
	/*		P07	P07		O(L)	BT_RNSW			*/
	/********************************************/
unsigned char	DBG_port0_in_data;		//	PORT0.PIDR.BYTE
	/********************************************/
	/*	Port1									*/
	/*		P12	RXD2	I		#JVMA_RXD		*/
	/*		P13	TXD2	O		#JVMA_TXD		*/
	/*		P14	IRQ4	I		#RTC_IRQ		*/
	/*		P15	IRQ5	I		#EX_IRQ1		*/
	/*		P16	MOSIA	O		MISO			*/
	/*		P17	MISOA	I		MOSI			*/
	/********************************************/
unsigned char	DBG_port1_in_data;		//	PORT1.PIDR.BYTE
	/********************************************/
	/*	Port2									*/
	/*		P20	SDA1	I/O		SDA				*/
	/*		P21	SCL1	O		SCL				*/
	/*		P22	P22		O(H)	#FB_MD2			*/
	/*		P23	TXD3	O		#FB_TXD			*/
	/*		P24	P24		O(L)	FB_FW			*/
	/*		P25	RXD3	I		#FB_RXD			*/
	/*		P26	TDO		O		TDO				*/
	/*		P27	TCK		I		TCK				*/
	/********************************************/
unsigned char	DBG_port2_in_data;		//	PORT2.PIDR.BYTE
	/********************************************/
	/*	Port3									*/
	/*		P30	TDI		I		TDI				*/
	/*		P31	TMS		I		TMS				*/
	/*		P32	P32		O(H)	#FB_RES			*/
	/*		P33	PO11	O		SOUND			*/
	/*		P34	#TRST	I		ÅîTRST			*/
	/*		P35	NMI		I		#NMI			*/
	/*		P36	EXTAL	I		EXTAL			*/
	/*		P37	XTAL	O		XTAL			*/
	/********************************************/
unsigned char	DBG_port3_in_data;		//	PORT3.PIDR.BYTE
	/********************************************/
	/*	Port4									*/
	/*		P40	IRQ8-DS	I		#EX_IRQ2		*/
	/*		P41	P41		O(L)	SF_RES			*/
	/*		P42	P42		O(L)	#SF_DTR			*/
	/*		P43	P43		O(L)	#SF_RTS			*/
	/*		P44	P44		I		#SF_RI			*/
	/*		P45	P45		I		#SF_DCD			*/
	/*		P46	P46		I		#SF_DSR			*/
	/*		P47	P47		I		#SF_CTS			*/
	/********************************************/
unsigned char	DBG_port4_in_data;		//	PORT4.PIDR.BYTE
	/********************************************/
	/*	Port5									*/
	/*		P50	#WR0	O		#WR				*/
	/*		P51	#BC1	O		#BC1			*/
	/*		P52	#RD		O		#RD				*/
	/*		P53	BCLK	O		BCLK			*/
	/*		P54	CTX1	O		CTX				*/
	/*		P55	CRX1	I		CRX				*/
	/*		P56	P56		O(L)	#CTERM			*/
	/********************************************/
unsigned char	DBG_port5_in_data;		//	PORT5.PIDR.BYTE
	/********************************************/
	/*	Port6									*/
	/*		P60	P60		O(H)	#NT_TEN			*/
	/*		P61	P61		O(L)	FAN_SW			*/
	/*		P62	P62		I		#DRSW			*/
	/*		P63	#CS3	O		#CS3			*/
	/*		P64	#CS4	O		#CS4			*/
	/*		P65	P65		O(L)	#OUT_EN			*/
	/*		P66	P66		O(L)	CAN_RES			*/
	/*		P67	IRQ15	I		#ETHER_IRQ		*/
	/********************************************/
unsigned char	DBG_port6_in_data;		//	PORT6.PIDR.BYTE
	/********************************************/
	/*	Port7									*/
	/*		P70	P70		O(H)	#FL_TEN			*/
	/*		P71	#CS1	O		#CS1			*/
	/*		P72	#CS2	O		#CS2			*/
	/*		P73	P73		I		#MRW_CTS		*/
	/*		P74	P74		O(H)	#MRW_DTR		*/
	/*		P75	P75		O(H)	#MRW_RTS		*/
	/*		P76	RXD11	I		#MRW_RXD		*/
	/*		P77	TXD11	O		#MRW_TXD		*/
	/********************************************/
unsigned char	DBG_port7_in_data;		//	PORT7.PIDR.BYTE
	/********************************************/
	/*	Port8									*/
	/*		P80	P80		I		#MRW_RI			*/
	/*		P81	RXD10	I		#RW_RXD			*/
	/*		P82	TXD10	O		#RW_TXD			*/
	/*		P83	P83		O(L)	RW_RES			*/
	/*		P86	P86		O(H)	#JVMA_SYC		*/
	/*		P87	P87		O(L)	FB_LD			*/
	/********************************************/
unsigned char	DBG_port8_in_data;		//	PORT8.PIDR.BYTE
	/********************************************/
	/*	Port9									*/
	/*		P90	TXD7	O		#SF_TXD			*/
	/*		P91	P91		O(H)	BT_MODE			*/
	/*		P92	RXD7	I		#SF_RXD			*/
	/*		P93	P93		O(L)	BT_ROLE			*/
	/********************************************/
unsigned char	DBG_port9_in_data;		//	PORT9.PIDR.BYTE
	/********************************************/
	/*	PortA									*/
	/*		PA0	#BC0	O		#BC0			*/
	/*		PA1	A1		O		AB1				*/
	/*		PA2	A2		O		AB2				*/
	/*		PA3	A3		O		AB3				*/
	/*		PA4	A4		O		AB4				*/
	/*		PA5	A5		O		AB5				*/
	/*		PA6	A6		O		AB6				*/
	/*		PA7	A7		O		AB7				*/
	/********************************************/
unsigned char	DBG_portA_in_data;		//	PORTA.PIDR.BYTE
	/********************************************/
	/*	PortB									*/
	/*		PB0	A8		O		AB8				*/
	/*		PB1	A9		O		AB9				*/
	/*		PB2	A10		O		AB10			*/
	/*		PB3	A11		O		AB11			*/
	/*		PB4	A12		O		AB12			*/
	/*		PB5	A13		O		AB13			*/
	/*		PB6	A14		O		AB14			*/
	/*		PB7	A15		O		AB15			*/
	/********************************************/
unsigned char	DBG_portB_in_data;		//	PORTB.PIDR.BYTE
	/********************************************/
	/*	PortC									*/
	/*		PC0	A16		O		AB16			*/
	/*		PC1	A17		O		AB17			*/
	/*		PC2	A18		O		AB18			*/
	/*		PC3	A19		O		AB19			*/
	/*		PC4	SSLA0	O		#SSL0			*/
	/*		PC5	RSPCK	I		RSPCK			*/
	/*		PC6	A22		O		A22				*/
	/*		PC7	PC7		I		MODE			*/
	/********************************************/
unsigned char	DBG_portC_in_data;		//	PORTC.PIDR.BYTE
	/********************************************/
	/*	PortD									*/
	/*		PD0	D0[A0/D0]	I/O	DB0				*/
	/*		PD1	D1[A1/D1]	I/O	DB1				*/
	/*		PD2	D2[A2/D2]	I/O	DB2				*/
	/*		PD3	D3[A3/D3]	I/O	DB3				*/
	/*		PD4	D4[A4/D4]	I/O	DB4				*/
	/*		PD5	D5[A5/D5]	I/O	DB5				*/
	/*		PD6	D6[A6/D6]	I/O	DB6				*/
	/*		PD7	D7[A7/D7]	I/O	DB7				*/
	/********************************************/
unsigned char	DBG_portD_in_data;		//	PORTD.PIDR.BYTE
	/********************************************/
	/*	PortE									*/
	/*		PE0	D8[A8/D8]	I/O	DB8				*/
	/*		PE1	D9[A9/D9]	I/O	DB9				*/
	/*		PE2	D10[A10/D10]I/O	DB10			*/
	/*		PE3	D11[A11/D11]I/O	DB11			*/
	/*		PE4	D12[A12/D12]I/O	DB12			*/
	/*		PE5	D13[A13/D13]I/O	DB13			*/
	/*		PE6	D14[A14/D14]I/O	DB14			*/
	/*		PE7	D15[A15/D15]I/O	DB15			*/
	/********************************************/
unsigned char	DBG_portE_in_data;		//	PORTE.PIDR.BYTE
	/********************************************/
	/*	PortF									*/
	/*		PF5	PF5		O(L)	BT_RES			*/
	/********************************************/
unsigned char	DBG_portF_in_data;		//	PORTF.PIDR.BYTE
	/********************************************/
	/*	PortJ									*/
	/*		PJ3	PJ3		O(H)	#BT_RTS			*/
	/*		PJ5	PJ5		I		#BT_CTS			*/
	/********************************************/
unsigned char	DBG_portJ_in_data;		//	PORTJ.PIDR.BYTE
	/********************************************/
	/*	PortK									*/
	/*		PK2	TXD9	O		#NT_TXD			*/
	/*		PK3	RXD9	I		#NT_RXD			*/
	/*		PK4	RXD4	I		#FL_RXD			*/
	/*		PK5	TXD4	O		#FL_TXD			*/
	/********************************************/
unsigned char	DBG_portK_in_data;		//	PORTK.PIDR.BYTE
	/********************************************/
	/*	PortL									*/
	/*		PL0	PL0		I		#MRW_DCD		*/
	/*		PL1	PL1		I		#MRW_DSR		*/
	/********************************************/
unsigned char	DBG_portL_in_data;		//	PORT0.PIDR.BYTE




