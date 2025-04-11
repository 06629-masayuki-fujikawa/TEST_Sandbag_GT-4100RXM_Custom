//[]----------------------------------------------------------------------[]
///	@file		s_cpu_init.c
///	@brief		CPU初期設定処理
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/
#include	"iodefine.h"
#include	"s_vect.h"
#include	<machine.h>
#include	"typedefine.h"
//#include	"stacksct.h"

/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void cpu_init( void );
void port_init( void );
void syscr_init( void );

/*--- Define --------------------*/

/*--- Gloval Value --------------*/

/*--- Extern --------------------*/

/****************************************************************************/
/*            オプション設定メモリの設定
/****************************************************************************/
// RX630ではエンディアンの設定を内部プログラムROMに書き込む必要があるのでその処置をここで行なう
#pragma address OFS1_REG = 0xFFFFFF88			// OFS1 register
const unsigned long OFS1_REG = 0xFFFFFFFF;		// デフォルト

#pragma address OFS0_REG = 0xFFFFFF8C			// OFS0 register
const unsigned long OFS0_REG = 0xFFFFFFFF;		// デフォルト

#pragma address MDEreg = 0xFFFFFF80				// MDE register (Single Chip Mode)
const unsigned long MDEreg = 0xFFFFFFF8;		// big

/****************************************************************************/
/*																			*/
/*	hardware initialization													*/
/*																			*/
/****************************************************************************/
void cpu_init( void )
{
	volatile unsigned char  dummy;
	volatile unsigned char  cnt;
	volatile unsigned short  short_dummy;

	/********************************************************************************/
	/*		SYSTEM																	*/
	/*			Mode設定															*/
	/*				reset解除時はSingleChipModeにて起動し(☆1の設定)				*/
	/*				初期設定完了後、内蔵ROM有効拡張Modeに移行する(☆2設定)			*/
	/*				MD端子 = H(SingleChipMode起動)									*/
	/*			運用Mode設定														*/
	/*				1.	MDEB.MDE[2:0]又はMDES.MDE[2:0]								*/
	/*						000b		Big				o							*/
	/*						111b		Little			x							*/
	/*			動作Mode *1=SYSCR0													*/
	/*	--------------------------------------------------------------				*/
	/*			MD端子	ROME*1 	EXBE*1	動作Mode				内蔵ROM/外部Bus		*/
	/*			0 		1 		0 		SCIブートMode			有効/無効			*/
	/*			0 		1 		0 		USB/USERブートMode		有効/無効			*/
	/*			1 		1 		0 		シングルチップMode		有効/無効	<-	☆1	*/
	/*	--------------------------------------------------------------				*/
	/*					ROME*1 	EXBE*1	動作Mode				内蔵ROM/外部Bus		*/
	/*					0 		0		シングルチップMode		無効/無効			*/
	/*					1 		0 		シングルチップMode		有効/無効	<-	☆1	*/
	/*					0 		1 		内蔵ROM無効拡張Mode		無効/有効			*/
	/*					1 		1 		内蔵ROM有効拡張Mode		有効/有効	<-	☆2	*/
	/********************************************************************************/
	/********************************************************/
	/*	PowerOnReset時のRegisterの値						*/
	/*		R0(SP) - R15	(不定?)							*/
	/*		ISP 	(H'00000000)							*/
	/*		USP 	(H'00000000)							*/
	/*		INTB	(不定)									*/
	/*		PC 		(resetVectorの値)						*/
	/*		PSW 	(H'00000000)							*/
	/*			I(b16)				INT(0/1 = DI/EI)		*/
	/*			U(b17)				0/1 = ISP/USP			*/
	/*			PM(b20)				0/1 = supperuser/user	*/
	/*			IPL[3:0](b24-b27)	INT priority level		*/
	/********************************************************/

	/****************************************/
	/*		9	Clock発生回路				*/
	/****************************************/
	SYSTEM.SCKCR.LONG = 0x21821211;	//	FCK:4(0010b:4分周), ICK:4(0001b:2分周), PSTOP1:1(1), b22-b20(000b), BCK:4(0010b:4分周)
									//	b15-b12(0001b), PCKB:4(0010b:4分周), b7-b4(0001b), b3-b0(0001b)
	SYSTEM.SCKCR2.WORD = 0x0011;	//	b15-b8(00000000b), UCK:4(0001b), EBCK:4(0001b:2分周)
	SYSTEM.SCKCR3.WORD = 0x0400;	//	b15-b11(00000b), CKSEL:3(100b=PLL回路選択), IEBCK:8(00000000b)
	// メインクロック発振安定待機時間が20mS以上(ハードウェアマニュアル45.4クロックタイミング)より
	// 水晶の周波数12.5MHzで(b4-0:01110)262144サイクルとすると20.97ms
	SYSTEM.MOSCWTCR.BYTE = 0x0E;	//	メインクロック発振器ウェイトコントロールレジスタ
	SYSTEM.MOSCCR.BYTE = 0x00;		//	b7-b1(0000000b), MOSTP:1(0=MainClock発振器動作)
	for(;;){
		dummy = SYSTEM.MOSCCR.BYTE;
		if(0x00 == dummy){
			break;
		}
	}
	wait2us(15000L);				// 30msウェイト(20.97に対してマージンをとって)
	SYSTEM.PLLCR.WORD = 0x0F00;		//	b15-b14(00b), STC:6(001111b=x16), b7-b2(000000b), PLIDIV:2(00b:1分周)
	// PLLクロック発振安定待機時間が1.5mS以上(ハードウェアマニュアル45.4クロックタイミング)より
	// PLLの発振周波数200MHzで(b4-0:01100)524288サイクルとすると2.62ms
	SYSTEM.PLLWTCR.BYTE = 0x0C;		//	PLLウェイトコントロールレジスタ
	SYSTEM.PLLCR2.BYTE = 0x00;		//	b7-b1(0000000b), PLLEN:1(0=PLL動作ON)
	SYSTEM.BCKCR.BYTE = 0x01;		//	b7-b1(0000000b), BCLKDIV:1(1=BCLKの2分周)
	wait2us(1400L);					// 2.8msウェイト(2.62に対してマージンをとって)
	SYSTEM.SOSCCR.BYTE = 0x01;		//	b7-b1(0000000b), SOSTP:1(1=SubClock発振器停止)
	SYSTEM.LOCOCR.BYTE = 0x01;		//	b7-b1(0000000b), LCSTP:1(1=LOCO動作停止)
	SYSTEM.ILOCOCR.BYTE = 0x00;		//	b7-b1(0000000b), ILCSTP:1(0=IWDT専用OnChipOscrator動作ON)
	SYSTEM.HOCOCR.BYTE = 0x01;		//	b7-b1(0000000b), HCSTP:1(1=HOCO動作停止)
	SYSTEM.OSTDCR.BYTE = 0x00;		//	OSTDE:1, b6-b1(000000b), OSTDIE:1
	SYSTEM.OSTDSR.BYTE = 0x00;		//	b7-b1(0000000b), OSTDF:1(0=MainClock発振停止を未検出)
	SYSTEM.MOFCR.BYTE = 0x00;		//	b7-b1(0000000b), MOFXIN:1
	SYSTEM.HOCOPCR.BYTE = 0x01;		//	b7-b1(0000000b), HOCOPCNT:1(1=HOCOの電源OFF)

	//	初期設定完了後の内蔵ROM有効拡張Modeの設定処理
	SYSTEM.SYSCR0.WORD = 0x5A03;	//	KEY:8, b7-b2, EXBE:1(1), ROME:1(1)
	for(;;){
		short_dummy = SYSTEM.SYSCR0.WORD & 0x00FF;
		if(0x0003 == short_dummy){
			break;
		}
	}

	/************************/
	/*	CS1					*/
	/************************/
	BSC.CS1REC.WORD = 0x0101;		//	b15-b12, WRCV:4(0001=WriteRecovery1Cycle), b7-b4, RRCV:4(0001=ReadRecovery1Cycle)
	BSC.CS1MOD.WORD = 0x0301;		//	PRMOD:1(0=NormalAccess互換Mode), b14-b10, PWENB:1(1=PageWriteAccess許可), PRENB:1(1=PageReadAccess許可)
									//	b7-b4, EWENB:1(0=外部Wait禁止), b2-b1, WRMOD:1(1=1WriteStrobeMode))
	BSC.CS1WCR1.LONG = 0x04040202;	//	b31-b29, CSRWAIT:5(00100=NormalReadCycleWait4Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
									//	b15-b11, CSPRWAIT:3(010=PageReadCycleWait 2cycle), b7-b3, CSPWWAIT:3(010=PageWriteCycleWait 2cycle)
	BSC.CS1WCR2.LONG = 0x01110122;	//	b31, CSON:3(000=CSAssertWait挿入しない), b27, WDON:3(001=WriteData出力Wait1Cycle), b23, WRON:3(001=WRAssertWait1Cycle), b19, RDON:3(001=RDAssertWait1Cycle)
									//	b15-b14, AWAIT:2(00=AddressCycleWait挿入しない), b11, WDOFF:3(001=WriteData出力延長1Cycle), b7, CSWOFF:3(010=Write時CS延長2Cycle), b3, CSROFF:3(010=Read時CS延長2Cycle)
	BSC.CS1CR.WORD = 0x0001;		//	b15-b13, MPXEN:1(0=SeparateBusIF/Address,DataMultiPlexI/OIF), b11-b9, EMODE:1(0=Endianは動作ModeのEndianと同じ) 
									//	b7-b6, BSIZE:2(00=16BitBus空間に設定), b3-b1, EXENB:1(1=動作許可)
	/************************/
	/*	CS2					*/
	/************************/
	BSC.CS2REC.WORD = 0x0101;		//	b15-b12, WRCV:4(0001=WriteRecovery1Cycle), b7-b4, RRCV:4(0001=ReadRecovery1Cycle)
	BSC.CS2MOD.WORD = 0x0301;		//	PRMOD:1(0=NormalAccess互換Mode), b14-b10, PWENB:1(1=PageWriteAccess許可), PRENB:1(1=PageReadAccess許可)
									//	b7-b4, EWENB:1(0=外部Wait禁止), b2-b1, WRMOD:1(1=1WriteStrobeMode))
	BSC.CS2WCR1.LONG = 0x04040202;	//	b31-b29, CSRWAIT:5(00100=NormalReadCycleWait4Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
									//	b15-b11, CSPRWAIT:3(010=PageReadCycleWait 2cycle), b7-b3, CSPWWAIT:3(010=PageWriteCycleWait 2cycle)
	BSC.CS2WCR2.LONG = 0x01110122;	//	b31, CSON:3(000=CSAssertWait挿入しない), b27, WDON:3(001=WriteData出力Wait1Cycle), b23, WRON:3(001=WRAssertWait1Cycle), b19, RDON:3(001=RDAssertWait1Cycle)
									//	b15-b14, AWAIT:2(00=AddressCycleWait挿入しない), b11, WDOFF:3(001=WriteData出力延長1Cycle), b7, CSWOFF:3(010=Write時CS延長2Cycle), b3, CSROFF:3(010=Read時CS延長2Cycle)
	BSC.CS2CR.WORD = 0x0001;		//	b15-b13, MPXEN:1(0=SeparateBusIF/Address,DataMultiPlexI/OIF), b11-b9, EMODE:1(0=Endianは動作ModeのEndianと同じ) 
									//	b7-b6, BSIZE:2(00=16BitBus空間に設定), b3-b1, EXENB:1(1=動作許可)
	/************************/
	/*	CS3					*/
	/************************/
	BSC.CS3REC.WORD = 0x0101;		//	b15-b12, WRCV:4(0001=WriteRecovery1Cycle), b7-b4, RRCV:4(0001=ReadRecovery1Cycle)
	BSC.CS3MOD.WORD = 0x0301;		//	PRMOD:1(0=NormalAccess互換Mode), b14-b10, PWENB:1(1=PageWriteAccess許可), PRENB:1(1=PageReadAccess許可)
									//	b7-b4, EWENB:1(0=外部Wait禁止), b2-b1, WRMOD:1(1=1WriteStrobeMode))
	BSC.CS3WCR1.LONG = 0x06040202;	//	b31-b29, CSRWAIT:5(00110=NormalReadCycleWait6Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
									//	b15-b11, CSPRWAIT:3(010=PageReadCycleWait 2cycle), b7-b3, CSPWWAIT:3(010=PageWriteCycleWait 2cycle)
									//	b15-b14, AWAIT:2(00=AddressCycleWait挿入しない), b11, WDOFF:3(010=WriteData出力延長2Cycle), b7, CSWOFF:3(011=Write時CS延長3Cycle), b3, CSROFF:3(010=Read時CS延長2Cycle)
	BSC.CS3WCR2.LONG = 0x01310232;	// 2012/03/01 修正依頼により変更

	BSC.CS3CR.WORD = 0x0001;		//	b15-b13, MPXEN:1(0=SeparateBusIF/Address,DataMultiPlexI/OIF), b11-b9, EMODE:1(0=Endianは動作ModeのEndianと同じ) 
									//	b7-b6, BSIZE:2(00=16BitBus空間に設定), b3-b1, EXENB:1(1=動作許可)
	/************************/
	/*	CS4					*/
	/************************/
	BSC.CS4REC.WORD = 0x0101;		//	b15-b12, WRCV:4(0001=WriteRecovery1Cycle), b7-b4, RRCV:4(0001=ReadRecovery1Cycle)
	BSC.CS4MOD.WORD = 0x0001;		//	PRMOD:1(0=NormalAccess互換Mode), b14-b10, PWENB:1(0=PageWriteAccess禁止), PRENB:1(0=PageReadAccess禁止)
									//	b7-b4, EWENB:1(0=外部Wait禁止), b2-b1, WRMOD:1(1=1WriteStrobeMode))
	BSC.CS4WCR1.LONG = 0x03020000;	//	b31-b29, CSRWAIT:5(00011=NormalReadCycleWait3Cycle), b23-b21, CSWWAIT:5(00010=NormalWriteCycleWait2Cycle)
									//	b15-b11, CSPRWAIT:3(000=PageReadCycleWait挿入しない), b7-b3, CSPWWAIT:3(000=PageWriteCycleWait挿入しない)
	BSC.CS4WCR2.LONG = 0x01110122;	//	b31, CSON:3(000=CSAssertWait挿入しない), b27, WDON:3(001=WriteData出力Wait1Cycle), b23, WRON:3(001=WRAssertWait1Cycle), b19, RDON:3(001=RDAssertWait1Cycle)
									//	b15-b14, AWAIT:2(00=AddressCycleWait挿入しない), b11, WDOFF:3(001=WriteData出力延長1Cycle), b7, CSWOFF:3(010=Write時CS延長2Cycle), b3, CSROFF:3(010=Read時CS延長2Cycle)
	BSC.CS4CR.WORD = 0x0001;		//	b15-b13, MPXEN:1(0=SeparateBusIF/Address,DataMultiPlexI/OIF), b11-b9, EMODE:1(0=Endianは動作ModeのEndianと同じ) 
									//	b7-b6, BSIZE:2(00=16BitBus空間に設定), b3-b1, EXENB:1(1=動作許可)
	/************************/
	/*	共通				*/
	/************************/
	BSC.CSRECEN.WORD = 0x3EFE;		//	RCVENM7:1 - RCVENM0:1(00111110b), RCVEN7:1 - RCVEN0:1(11111110b)
	BSC.BEREN.BYTE = 0x03;			//	b7-b2(000000b), TOEN:1(1=BusTimeout検出許可), IGAEN:1(1=不正AddressAccess検出許可)


}
/****************************************************************************/
/*																			*/
/*	Port initialization														*/
/*																			*/
/****************************************************************************/
void port_init( void )
{
	/****************************************************************/
	/*	MPC(MultiFunctionPinControl)								*/
	/*	CS1(P71=01)	MainMemory1			SRAM_2MB(Backup有)			*/
	/*	CS2(P72=01)	MainMemory2			SRAM_1MB(Backup有)			*/
	/*	CS3(P63=00)	EthernetControler	???							*/
	/*	CS4(P64=00)	拡張I/O				入力:16Port，出力:16Port	*/
	/*	A1 - A19, A22												*/
	/****************************************************************/
	MPC.PFCSS0.BYTE = 0x14;		//	CS3S:2, CS2S:2, CS1S:2, b1, CS0S:1(CS出力端子選択Register0)
	MPC.PFCSS1.BYTE = 0x00;		//	CS7S:2, CS6S:2, CS5S:2, CS4S:2(CS出力端子選択Register1)
	MPC.PFAOE0.BYTE = 0xFF;		//	A15E:1 - A8E:1(Address出力許可Register0)
	MPC.PFAOE1.BYTE = 0x4F;		//	A23E:1 - A16E:1(Address出力許可Register1)
	MPC.PFBCR0.BYTE = 0x51;		//	WR32BC32E:1, WR1BC1E:1, DH32E:1, DHE:1, b3-b2, ADRHMS:1, ADRLE:1(外部Bus制御Register0:20.2.25)
	MPC.PFBCR1.BYTE = 0x00;		//	b7-b3, ALEOE:1(P54=IO), WAITS:2(WAIT=?)(外部Bus制御Register1:20.2.26)
	MPC.PFCSE.BYTE  = 0x1E;		//	CS7E:1 - CS0E:1(CS出力許可Register:20.2.20) = CS1,2,3,4のみ出力許可
	MPC.PWPR.BYTE   = 0x00;		//	B0WI:0, PFSWE:0(書込みProtectRegister:20.2.1), b5-b0
	MPC.PWPR.BYTE   = 0x40;		//	B0WI:0, PFSWE:1(書込みProtectRegister:20.2.1), b5-b0
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P00		TXD6	O		BT_TXD		1			0		Bluetooth		Bluetooth通信Data送信信号		8		*/
	/*	P01		RXD6	I		BT_RXD		1			0		Bluetooth		Bluetooth通信Data受信信号		7		*/
	/*	P02		P02		O(L)	#MUTE		通常		MUTE	音声合成		デジタルアンプMUTE信号			6		*/
	/*	P03		P03		O(L)	FBSH_O		Open		通常	磁気Reader制御	磁気ReaderシャッタOpen信号		4		*/
	/*	P05		P05		O(L)	FBSH_C		Close		通常	磁気Reader制御	磁気ReaderシャッタClose信号		2		*/
	/********************************************************************************************************************/
	MPC.P00PFS.BYTE = 0x0A;	//	ASEL:1(Analog), ISEL:1(IRQ), b5, PSEL:5(Port)
	MPC.P01PFS.BYTE = 0x0A;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.P02PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.P03PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.P05PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.P07PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P12		RXD2	I		#JVMA_RXD	0			1		JVMA			JVMA通信Data受信信号			45		*/
	/*	P13		TXD2	O		#JVMA_TXD	0			1		JVMA			JVMA通信Data送信信号			44		*/
	/*	P14		IRQ4	I		#RTC_IRQ	通常		H/W-IRQ	RTC				RTC時刻同期信号(1min間隔)		43		*/
	/*	P15		IRQ5	I		#EX_IRQ1	通常		H/W-IRQ	CPU(IIC)		UARTcontroler1割込み信号		42		*/
	/*	P16		MOSIA	O		MISO		1			0		CPU(SPI)		SPImasterData					40		*/
	/*	P17		MISOA	I		MOSI		1			0		CPU(SPI)		SPIslaveData					38		*/
	/********************************************************************************************************************/
	MPC.P12PFS.BYTE = 0x0A;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P13PFS.BYTE = 0x0A;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P14PFS.BYTE = 0x40;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P15PFS.BYTE = 0x40;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P16PFS.BYTE = 0x0D;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P17PFS.BYTE = 0x0D;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P20		SDA1	I/O		SDA			1			0		CPU(IIC)		I2CData							37		*/
	/*	P21		SCL1	O		SCL			-			-		CPU(IIC)		I2CClock						36		*/
	/*	P22		P22		O(H)	#FB_MD2		通常		書込み	磁気Reader制御	FB-7000bootMode選択信号MD2		35		*/
	/*	P23		TXD3	O		#FB_TXD		1			0		磁気Reader制御	FB-7000Data送信信号				34		*/
	/*	P24		P24		O(L)	FB_FW		書込み		通常	磁気Reader制御	FB-7000bootMode選択信号FW		33		*/
	/*	P25		RXD3	I		#FB_RXD		1			0		磁気Reader制御	FB-7000Data受信信号				32		*/
	/*	P26		TDO		O		TDO			1			0		Debug/Program	Debuger通信TXD(WriterTXDと共有)	31		*/
	/*	P27		TCK		I		TCK			-			-		Debug/Program	Debuger通信CLK(WriterCLKと共有)	30		*/
	/********************************************************************************************************************/
	MPC.P20PFS.BYTE = 0x0F;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P21PFS.BYTE = 0x0F;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P22PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P23PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.P24PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P25PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.P26PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P27PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P30		TDI		I		TDI			1			0		Debug/Program	Debuger通信RXD(WriterRXDと共有)	29		*/
	/*	P31		TMS		I		TMS								Debug/Program									28		*/
	/*	P32		P32		O(H)	#FB_RES		通常		reset	磁気Reader制御	FB-7000reset信号				27		*/
	/*	P33		TIOCD0	O		SOUND		-			-		音声合成		音声出力						26		*/
	/*	P34		#TRST	I		＃TRST		通常		reset	Debug/Program									25		*/
	/*	P35		NMI		I		#NMI		通常		NMI		電源/reset		NMI信号監視						24		*/
	/*	P36		EXTAL	I		EXTAL		-			-		CPU				外部水晶振動子接続				22		*/
	/*	P37		XTAL	O		XTAL		-			-		CPU				外部水晶振動子接続				20		*/
	/********************************************************************************************************************/
	//	表20.10
	MPC.P30PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P31PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P32PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P33PFS.BYTE = 0x03;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P34PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P40		IRQ8-DS	I		#EX_IRQ2	通常		H/W-IRQ	CPU(IIC)		UARTcontroler2割込み信号		141		*/
	/*	P41		P41		O(L)	SF_RES		reset要求	通常	serverFOMA		serverFOMA_reset信号			139		*/
	/*	P42		P42		O(L)	#SF_DTR		disable		enable	serverFOMA		serverFOMA_ER信号				138		*/
	/*	P43		P43		O(L)	#SF_RTS		disable		enable	serverFOMA		serverFOMA_RS信号				137		*/
	/*	P44		P44		I		#SF_RI		disable		enable	serverFOMA		serverFOMA_CI信号				136		*/
	/*	P45		P45		I		#SF_DCD		disable		enable	serverFOMA		serverFOMA_CD信号				135		*/
	/*	P46		P46		I		#SF_DSR		disable		enable	serverFOMA		serverFOMA_DR信号				134		*/
	/*	P47		P47		I		#SF_CTS		disable		enable	serverFOMA		serverFOMA_CS信号				133		*/
	/********************************************************************************************************************/
	MPC.P40PFS.BYTE = 0x40;	//	ASEL:1, ISEL:1, b5-b0
	MPC.P41PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5-b0
	MPC.P42PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5-b0
	MPC.P43PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5-b0
	MPC.P44PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5-b0
	MPC.P45PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5-b0
	MPC.P46PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5-b0
	MPC.P47PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5-b0
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P50		#WR0	O		#WR			disable		enable	CPU				ライトenable信号				56		*/
	/*	P51		#BC1	O		#BC1		disable		enable	CPU				address選択(UB)					55		*/
	/*	P52		#RD		O		#RD			disable		enable	CPU				リード信号						54		*/
	/*	P53		BCLK	O		BCLK		1			0		CPU				未使用(Bus選択時はP51使用不可)	53		*/
	/*	P54		CTX1	O		CTX			1			0		機器内通信		CAN_TXD信号						52		*/
	/*	P55		CRX1	I		CRX			1			0		機器内通信		CAN_RXD信号						51		*/
	/*	P56		P56		O(L)	#CTERM		終端無し	終端有り機器内通信		CAN_終端抵抗制御信号(LV14で反転)50		*/
	/********************************************************************************************************************/
	//	表20.12
	MPC.P50PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P51PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P52PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P54PFS.BYTE = 0x10;	//	b7-b5, PSEL:5
	MPC.P55PFS.BYTE = 0x10;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P56PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P60		P60		O(H)	#NT_TEN		disable		enable	NT-NET通信		NT-NET通信送信enable信号		117		*/
	/*	P61		P61		O(L)	FAN_SW	I	ON			OFF		FAN制御			FAN制御SW信号					115		*/
	/*	P62		P62		I		#DRSW		ドア開		ドア閉	各種センサ監視	ドア開センサ信号				114		*/
	/*	P63		#CS3	O		#CS3		disable		enable	Ethernet		LANcontrolerChipSelect信号		113		*/
	/*	P64		#CS4	O		#CS4		disable		enable	CPU				I/O拡張ChipSelect信号			112		*/
	/*	P65		P65		O(L)	#OUT_EN	I	disable		enable	CPU				I/O拡張空間Outputenable信号		100		*/
	/*	P66		P66		O(L)	CAN_RES	I	reset		通常	機器内通信		機器内通信reset信号				99		*/
	/*	P67		IRQ15	I		#ETHER_IRQ	通常		H/W-IRQ	Ethernet		Ethernetcontroler割込み信号		98		*/
	/********************************************************************************************************************/
	MPC.P60PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P61PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P66PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P67PFS.BYTE = 0x40;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P70		P70		O(H)	#FL_TEN		disable		enable	Flap制御		Flap通信_送信enable信号			104		*/
	/*	P71		#CS1	O		#CS1		disable		enable	CPU				SRAM1ChipSelect信号				86		*/
	/*	P72		#CS2	O		#CS2		disable		enable	CPU				SRAM2ChipSelect信号				85		*/
	/*	P73		P73		I		#MRW_CTS	disable		enable	MultiICcardR/W	MultiICcardR/W_CS信号			77		*/
	/*	P74		P74		O(H)	#MRW_DTR	disable		enable	MultiICcardR/W	MultiICcardR/W_ER信号			72		*/
	/*	P75		P75		O(H)	#MRW_RTS	disable		enable	MultiICcardR/W	MultiICcardR/W_RS信号			71		*/
	/*	P76		RXD11	I		#MRW_RXD	0			1		MultiICcardR/W	MultiICcardR/WData受信信号		69		*/
	/*	P77		TXD11	O		#MRW_TXD	0			1		MultiICcardR/W	MultiICcardR/WData送信信号		68		*/
	/********************************************************************************************************************/
	MPC.P70PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P73PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P74PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P75PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P76PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.P77PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P80		P80		I		#MRW_RI		disable		enable	MultiICcardR/W	MultiICcardR/W_CI信号			65		*/
	/*	P81		RXD10	I		#RW_RXD		0			1		非接触IC		非接触IC R/WData受信信号		64		*/
	/*	P82		TXD10	O		#RW_TXD		0			1		非接触IC		非接触IC R/WData送信信号		63		*/
	/*	P83		P83		O(L)	RW_RES		reset		通常	非接触IC		非接触IC reset信号				58		*/
	/*	P86		P86		O(H)	#JVMA_SYC	disable		enable	JVMA			JVMA通信同期信号				41		*/
	/*	P87		P87		O(L)	FB_LD		点灯		消灯	磁気Reader制御	磁気ReaderシャッタLED制御信号	39		*/
	/********************************************************************************************************************/
	MPC.P80PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P81PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.P82PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.P83PFS.BYTE = 0x00;	//	b7-b5, SEL:5
	MPC.P86PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P87PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P90		TXD7	O		#SF_TXD		0			1		serverFOMA		serverFOMAData送信信号			131		*/
	/*	P91		P91		O(H)	BT_MODE		Auto		Command	Bluetooth		BluetoothMode選択信号			129		*/
	/*	P92		RXD7	I		#SF_RXD		0			1		serverFOMA		serverFOMAData受信信号			128		*/
	/*	P93		P93		O(L)	BT_ROLE		master		slave	Bluetooth		Bluetoothmaster/slave選択信号	127		*/
	/********************************************************************************************************************/
	MPC.P90PFS.BYTE = 0x0A;	//	ASEL:1, b6-b5, PSEL:5
	MPC.P91PFS.BYTE = 0x00;	//	ASEL:1, b6-b5, PSEL:5
	MPC.P92PFS.BYTE = 0x0A;	//	ASEL:1, b6-b5, PSEL:5
	MPC.P93PFS.BYTE = 0x00;	//	ASEL:1, b6-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PA0		#BC0	O		#BC0		disable		enable	CPU				address選択(LB)					97		*/
	/*	PA1		A1		O		AB1			1			0		CPU				外部Bus_address					96		*/
	/*	PA2		A2		O		AB2			1			0		CPU				外部Bus_address					95		*/
	/*	PA3		A3		O		AB3			1			0		CPU				外部Bus_address					94		*/
	/*	PA4		A4		O		AB4			1			0		CPU				外部Bus_address					92		*/
	/*	PA5		A5		O		AB5			1			0		CPU				外部Bus_address					90		*/
	/*	PA6		A6		O		AB6			1			0		CPU				外部Bus_address					89		*/
	/*	PA7		A7		O		AB7			1			0		CPU				外部Bus_address					88		*/
	/********************************************************************************************************************/
	//	表20.19
	MPC.PA0PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PA1PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.PA2PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PA3PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.PA4PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.PA5PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PA6PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PA7PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PB0		A8		O		AB8			1			0		CPU				外部Bus_address					87		*/
	/*	PB1		A9		O		AB9			1			0		CPU				外部Bus_address					84		*/
	/*	PB2		A10		O		AB10		1			0		CPU				外部Bus_address					83		*/
	/*	PB3		A11		O		AB11		1			0		CPU				外部Bus_address					82		*/
	/*	PB4		A12		O		AB12		1			0		CPU				外部Bus_address					81		*/
	/*	PB5		A13		O		AB13		1			0		CPU				外部Bus_address					80		*/
	/*	PB6		A14		O		AB14		1			0		CPU				外部Bus_address					79		*/
	/*	PB7		A15		O		AB15		1			0		CPU				外部Bus_address					78		*/
	/********************************************************************************************************************/
	//	表20.21
	MPC.PB0PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.PB1PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.PB2PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PB3PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PB4PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PB5PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PB6PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PB7PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PC0		A16		O		AB16		1			0		CPU				外部Busaddress					75		*/
	/*	PC1		A17		O		AB17		1			0		CPU				外部Busaddress					73		*/
	/*	PC2		A18		O		AB18		1			0		CPU				外部Busaddress					70		*/
	/*	PC3		A19		O		AB19		1			0		CPU				外部Busaddress					67		*/
	/*	PC4		SSLA0	O		#SSL0		disable		enable	CPU(SPI)		SPI-FROMChipSelect信号			66		*/
	/*	PC5		RSPCK	I		RSPCK		-			-		CPU(SPI)		SPIClock信号					62		*/
	/*	PC6		A22		O		A22			1			0		CPU				外部Busaddress					61		*/
	/*	PC7		PC7		I		MODE		Userboot	SCIboot	Debug/Program	bootMode切替					60		*/
	/********************************************************************************************************************/
	//	表20.23
	MPC.PC0PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.PC1PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.PC2PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PC3PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.PC4PFS.BYTE = 0x0D;	//	b7-b5, PSEL:5
	MPC.PC5PFS.BYTE = 0x0D;	//	b7-b5, PSEL:5
	MPC.PC6PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.PC7PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PD0		D0[A0/D0]	I/O	DB0			1			0		CPU				外部Bus_Data					126		*/
	/*	PD1		D1[A1/D1]	I/O	DB1			1			0		CPU				外部Bus_Data					125		*/
	/*	PD2		D2[A2/D2]	I/O	DB2			1			0		CPU				外部Bus_Data					124		*/
	/*	PD3		D3[A3/D3]	I/O	DB3			1			0		CPU				外部Bus_Data					123		*/
	/*	PD4		D4[A4/D4]	I/O	DB4			1			0		CPU				外部Bus_Data					122		*/
	/*	PD5		D5[A5/D5]	I/O	DB5			1			0		CPU				外部Bus_Data					121		*/
	/*	PD6		D6[A6/D6]	I/O	DB6			1			0		CPU				外部Bus_Data					120		*/
	/*	PD7		D7[A7/D7]	I/O	DB7			1			0		CPU				外部Bus_Data					119		*/
	/********************************************************************************************************************/
	//	表20.26
	MPC.PD0PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PD1PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PD2PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PD3PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PD4PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PD5PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PD6PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PD7PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PE0		D8[A8/D8]	I/O	DB8			1			0		CPU				外部Bus_Data					111		*/
	/*	PE1		D9[A9/D9]	I/O	DB9			1			0		CPU				外部Bus_Data					110		*/
	/*	PE2		D10[A10/D10]I/O	DB10		1			0		CPU				外部Bus_Data					109		*/
	/*	PE3		D11[A11/D11]I/O	DB11		1			0		CPU				外部Bus_Data					108		*/
	/*	PE4		D12[A12/D12]I/O	DB12		1			0		CPU				外部Bus_Data					107		*/
	/*	PE5		D13[A13/D13]I/O	DB13		1			0		CPU				外部Bus_Data					106		*/
	/*	PE6		D14[A14/D14]I/O	DB14		1			0		CPU				外部Bus_Data					102		*/
	/*	PE7		D15[A15/D15]I/O	DB15		1			0		CPU				外部Bus_Data					101		*/
	/********************************************************************************************************************/
	//	表20.29
	MPC.PE0PFS.BYTE = 0x00;	//	ASEL:1, b6-b5, PSEL:5
	MPC.PE1PFS.BYTE = 0x00;	//	ASEL:1, b6-b5, PSEL:5
	MPC.PE2PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PE3PFS.BYTE = 0x00;	//	ASEL:1, b6-b5, PSEL:5
	MPC.PE4PFS.BYTE = 0x00;	//	ASEL:1, b6-b5, PSEL:5
	MPC.PE5PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PE6PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.PE7PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PF5		PF5		O(L)	BT_RES		reset		通常	Bluetooth		BluetoothMojulereset信号		9		*/
	/********************************************************************************************************************/
	MPC.PF5PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PJ3		PJ3		O(H)	#BT_RTS		disable		enable	Bluetooth		BluetoothMojule送信Clear信号	13		*/
	/*	PJ5		PJ5		I		#BT_CTS		disable		enable	Bluetooth		BluetoothMojule送信Request信号	11		*/
	/********************************************************************************************************************/
	MPC.PJ3PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PK2		TXD9	O		#NT_TXD		0			1		NT-NET通信		NT-NETData送信信号				118		*/
	/*	PK3		RXD9	I		#NT_RXD		0			1		NT-NET通信		NT-NETData受信信号				116		*/
	/*	PK4		RXD4	I		#FL_RXD		0			1		Flap制御		Flap通信Data受信信号			105		*/
	/*	PK5		TXD4	O		#FL_TXD		0			1		Flap制御		Flap通信Data送信信号			103		*/
	/********************************************************************************************************************/
	MPC.PK2PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.PK3PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.PK4PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.PK5PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.PWPR.BYTE   = 0x00;		//	B0WI:0, PFSWE:0(書込みProtectRegister:20.2.1), b5-b0
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	機能1(Init値)	信号名		Data(H)		(L)		Block			内容							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PL0		PL0		I		#MRW_DCD	disable		enable	MultiICcardR/W	MultiICcardR/W_CD信号			76		*/
	/*	PL1		PL1		I		#MRW_DSR	disable		enable	MultiICcardR/W	MultiICcardR/W_DR信号			74		*/
	/********************************************************************************************************************/
	//	No Setting
	/********************************************/
	/*	PORT(PDR_PMR_ODR_PCR_DSCR_Setting)		*/
	/********************************************/
	/*	Port0									*/
	/*		P00	TXD6	O		BT_TXD			*/
	/*		P01	RXD6	I		BT_RXD			*/
	/*		P02	P02		O(L)	#MUTE			*/
	/*		P03	P03		O(L)	FBSH_O			*/
	/*		P05	P05		O(L)	FBSH_C			*/
	/*		P07	P07		O(L)	BT_RNSW			*/
	/********************************************/
	PORT0.PMR.BYTE  = 0x03;		//	B7(0), B6(-), B5(0), B4(-), B3(0), B2(0), B1(1), B0(1)
	PORT0.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT0.ODR1.BYTE = 0x00;		//	B6, B2
	PORT0.PCR.BYTE  = 0x00;		//	B7, B5, B3, B2, B1, B0
	PORT0.DSCR.BYTE = 0x00;		//	B2, B1, B0
	PORT0.PODR.BYTE = 0x00;		//	B7, B5, B3, B2, B1, B0
	PORT0.PDR.BYTE  = 0xAD;		//	B7(1), B6(-), B5(1), B4(-), B3(1), B2(1), B1(0), B0(1)
	/********************************************/
	/*	Port1									*/
	/*		P12	RXD2	I		#JVMA_RXD		*/
	/*		P13	TXD2	O		#JVMA_TXD		*/
	/*		P14	IRQ4	I		#RTC_IRQ		*/
	/*		P15	IRQ5	I		#EX_IRQ1		*/
	/*		P16	MOSIA	O		MISO			*/
	/*		P17	MISOA	I		MOSI			*/
	/********************************************/
	PORT1.PMR.BYTE  = 0xCC;		//	B7(1), B6(1), B5(0), B4(0), B3(1), B2(1), B1(-), B0(-)
	PORT1.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT1.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT1.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT1.PDR.BYTE  = 0x48;		//	B7(0), B6(1), B5(0), B4(0), B3(1), B2(0), B1(-), B0(-)
	/********************************************/
	/*	Port2									*/
	/*		P20	SDA1	I		SDA				*/
	/*		P21	SCL1	I		SCL				*/
	/*		P22	P22		O(H)	#FB_MD2			*/
	/*		P23	TXD3	O		#FB_TXD			*/
	/*		P24	P24		O(L)	FB_FW			*/
	/*		P25	RXD3	I		#FB_RXD			*/
	/*		P26	TDO		I		TDO				*/
	/*		P27	TCK		I		TCK				*/
	/********************************************/
	PORT2.PMR.BYTE  = 0x2B;		//	B7(0), B6(0), B5(1), B4(0), B3(1), B2(0), B1(1), B0(1)
	PORT2.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT2.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT2.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT2.DSCR.BYTE = 0x00;		//	B7
	PORT2.PODR.BYTE = 0x04;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT2.PDR.BYTE  = 0x1C;		//	B7(0), B6(0), B5(0), B4(1), B3(1), B2(1), B1(0), B0(0)
	/********************************************/
	/*	Port3									*/
	/*		P30	TDI		I		TDI				*/
	/*		P31	TMS		I		TMS				*/
	/*		P32	P32		O(H)	#FB_RES			*/
	/*		P33	TIOCD0	O		SOUND			*/
	/*		P34	#TRST	I		＃TRST			*/
	/*		P35	NMI		I		#NMI			*/
	/*		P36	EXTAL	0		EXTAL(入力扱いにすること)	*/
	/*		P37	XTAL	O		XTAL(入力扱いにすること)	*/
	/********************************************/
	PORT3.PMR.BYTE  = 0x08;		//	B7(0), B6(0), B5(0), B4(0), B3(1), B2(0), B1(0), B0(0)
	PORT3.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT3.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT3.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT3.PODR.BYTE = 0x04;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT3.PDR.BYTE  = 0x0C;		//	B7(0), B6(0), B5(0), B4(0), B3(1), B2(1), B1(0), B0(0)
	/********************************************/
	/*	Port4									*/
	/*		P40	IRQ8-DS	I		#EX_IRQ2		*/
	/*		P41	P41		O(L)	SF_RES			*/
	/*		P42	P42		O(H)	#SF_DTR			*/
	/*		P43	P43		O(H)	#SF_RTS			*/
	/*		P44	P44		I		#SF_RI			*/
	/*		P45	P45		I		#SF_DCD			*/
	/*		P46	P46		I		#SF_DSR			*/
	/*		P47	P47		I		#SF_CTS			*/
	/********************************************/
	PORT4.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORT4.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT4.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT4.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT4.PODR.BYTE = 0x0C;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT4.PDR.BYTE  = 0x0E;		//	B7(0), B6(0), B5(0), B4(0), B3(1), B2(1), B1(1), B0(0)
	/********************************************/
	/*	Port5									*/
	/*		P50	#WR0	I		#WR(入力扱いにすること)	*/
	/*		P51	#BC1	I		#BC1(入力扱いにすること)*/
	/*		P52	#RD		I		#RD(入力扱いにすること)	*/
	/*		P53	BCLK	I		BCLK			*/
	/*		P54	CTX1	O		CTX				*/
	/*		P55	CRX1	I		CRX				*/
	/*		P56	P56		O(L)	#CTERM			*/
	/********************************************/
	PORT5.PMR.BYTE  = 0x30;		//	B7(-), B6(0), B5(1), B4(1), B3(0), B2(0), B1(0), B0(0)
	PORT5.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT5.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT5.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT5.DSCR.BYTE = 0x03;		//	B7(-), B6(0),B5-3(-), B2(0), B1(1), B0(1)
	PORT5.PODR.BYTE = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT5.PDR.BYTE  = 0x50;		//	B7(-), B6(1), B5(0), B4(1), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	Port6									*/
	/*		P60	P60		O(H)	#NT_TEN			*/
	/*		P61	P61		O(L)	FAN_SW			*/
	/*		P62	P62		I		#DRSW			*/
	/*		P63	#CS3	I		#CS3(入力扱いにすること)*/
	/*		P64	#CS4	I		#CS4(入力扱いにすること)*/
	/*		P65	P65		O(H)	#OUT_EN			*/
	/*		P66	P66		O(H)	CAN_RES			*/
	/*		P67	IRQ15	I		#ETHER_IRQ		*/
	/********************************************/
	PORT6.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORT6.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT6.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT6.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT6.DSCR.BYTE = 0x30;		//	B7(0), B6(0), B5(1), B4(1), B3(0), B2(0), B1(0), B0(0)
	PORT6.PODR.BYTE = 0x41;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT6.PDR.BYTE  = 0x63;		//	B7(0), B6(1), B5(1), B4(0), B3(0), B2(0), B1(1), B0(1)
	/********************************************/
	/*	Port7									*/
	/*		P70	P70		O(H)	#FL_TEN			*/
	/*		P71	#CS1	O		#CS1(入力扱いにすること)*/
	/*		P72	#CS2	O		#CS2(入力扱いにすること)*/
	/*		P73	P73		I		#MRW_CTS		*/
	/*		P74	P74		O(H)	#MRW_DTR		*/
	/*		P75	P75		O(H)	#MRW_RTS		*/
	/*		P76	RXD11	I		#MRW_RXD		*/
	/*		P77	TXD11	O		#MRW_TXD		*/
	/********************************************/
	PORT7.PMR.BYTE  = 0xC0;		//	B7(1), B6(1), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORT7.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT7.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT7.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT7.DSCR.BYTE = 0x06;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(1), B1(1), B0(0)
	PORT7.PODR.BYTE = 0x31;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT7.PDR.BYTE  = 0xB1;		//	B7(1), B6(0), B5(1), B4(1), B3(0), B2(0), B1(0), B0(1)
	/********************************************/
	/*	Port8									*/
	/*		P80	P80		I		#MRW_RI			*/
	/*		P81	RXD10	I		#RW_RXD			*/
	/*		P82	TXD10	O		#RW_TXD			*/
	/*		P83	P83		O(L)	RW_RES			*/
	/*		P86	P86		O(H)	#JVMA_SYC		*/
	/*		P87	P87		O(L)	FB_LD			*/
	/********************************************/
	PORT8.PMR.BYTE  = 0x06;		//	B7(0), B6(0), B5(-), B4(-), B3(0), B2(1), B1(1), B0(0)
	PORT8.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT8.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT8.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT8.PODR.BYTE = 0x40;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT8.PDR.BYTE  = 0xCC;		//	B7(1), B6(1), B5(-), B4(-), B3(1), B2(1), B1(0), B0(0)
	/********************************************/
	/*	Port9									*/
	/*		P90	TXD7	O		#SF_TXD			*/
	/*		P91	P91		O(L)	BT_MODE			*/
	/*		P92	RXD7	I		#SF_RXD			*/
	/*		P93	P93		O(L)	BT_ROLE			*/
	/********************************************/
	PORT9.PMR.BYTE  = 0x05;		//	B7(-), B6(-), B5(-), B4(-), B3(0), B2(1), B1(0), B0(1)
	PORT9.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT9.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORT9.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT9.DSCR.BYTE = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT9.PODR.BYTE = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORT9.PDR.BYTE  = 0x0B;		//	B7(-), B6(-), B5(-), B4(-), B3(1), B2(0), B1(1), B0(1)
	/********************************************/
	/*	PortA									*/
	/*		PA0	#BC0	O		#BC0(入力扱いにすること)*/
	/*		PA1	A1		O		AB1(入力扱いにすること)	*/
	/*		PA2	A2		O		AB2(入力扱いにすること)	*/
	/*		PA3	A3		O		AB3(入力扱いにすること)	*/
	/*		PA4	A4		O		AB4(入力扱いにすること)	*/
	/*		PA5	A5		O		AB5(入力扱いにすること)	*/
	/*		PA6	A6		O		AB6(入力扱いにすること)	*/
	/*		PA7	A7		O		AB7(入力扱いにすること)	*/
	/********************************************/
	PORTA.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORTA.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTA.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTA.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTA.DSCR.BYTE = 0xFF;		//	B7(1), B6(1), B5(1), B4(1), B3(1), B2(1), B1(1), B0(1)
	PORTA.PDR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	PortB									*/
	/*		PB0	A8		O		AB8(入力扱いにすること)	*/
	/*		PB1	A9		O		AB9(入力扱いにすること)	*/
	/*		PB2	A10		O		AB10(入力扱いにすること)*/
	/*		PB3	A11		O		AB11(入力扱いにすること)*/
	/*		PB4	A12		O		AB12(入力扱いにすること)*/
	/*		PB5	A13		O		AB13(入力扱いにすること)*/
	/*		PB6	A14		O		AB14(入力扱いにすること)*/
	/*		PB7	A15		O		AB15(入力扱いにすること)*/
	/********************************************/
	PORTB.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORTB.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTB.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTB.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTB.DSCR.BYTE = 0xFF;		//	B7(1), B6(1), B5(1), B4(1), B3(1), B2(1), B1(1), B0(1)
	PORTB.PDR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	PortC									*/
	/*		PC0	A16		O		AB16(入力扱いにすること)*/
	/*		PC1	A17		O		AB17(入力扱いにすること)*/
	/*		PC2	A18		O		AB18(入力扱いにすること)*/
	/*		PC3	A19		O		AB19(入力扱いにすること)*/
	/*		PC4	SSLA0	O		#SSL0			*/
	/*		PC5	RSPCK	I		RSPCK			*/
	/*		PC6	A22		O		A22(入力扱いにすること)*/
	/*		PC7	PC7		I		MODE			*/
	/********************************************/
	PORTC.PMR.BYTE  = 0x30;		//	B7(0), B6(0), B5(1), B4(1), B3(0), B2(0), B1(0), B0(0)
	PORTC.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTC.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTC.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTC.DSCR.BYTE = 0x4F;		//	B7(0), B6(1), B5(0), B4(0), B3(1), B2(1), B1(1), B0(1)
	PORTC.PDR.BYTE  = 0x30;		//	B7(0), B6(0), B5(1), B4(1), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	PortD									*/
	/*		PD0	D0[A0/D0]	I/O	DB0(入力扱いにすること)	*/
	/*		PD1	D1[A1/D1]	I/O	DB1(入力扱いにすること)	*/
	/*		PD2	D2[A2/D2]	I/O	DB2(入力扱いにすること)	*/
	/*		PD3	D3[A3/D3]	I/O	DB3(入力扱いにすること)	*/
	/*		PD4	D4[A4/D4]	I/O	DB4(入力扱いにすること)	*/
	/*		PD5	D5[A5/D5]	I/O	DB5(入力扱いにすること)	*/
	/*		PD6	D6[A6/D6]	I/O	DB6(入力扱いにすること)	*/
	/*		PD7	D7[A7/D7]	I/O	DB7(入力扱いにすること)	*/
	/********************************************/
	PORTD.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORTD.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTD.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTD.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTD.DSCR.BYTE = 0xFF;		//	B7(1), B6(1), B5(1), B4(1), B3(1), B2(1), B1(1), B0(1)
	PORTD.PDR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	PortE									*/
	/*		PE0	D8[A8/D8]	I/O	DB8(入力扱いにすること)	*/
	/*		PE1	D9[A9/D9]	I/O	DB9(入力扱いにすること)	*/
	/*		PE2	D10[A10/D10]I/O	DB10(入力扱いにすること)	*/
	/*		PE3	D11[A11/D11]I/O	DB11(入力扱いにすること)	*/
	/*		PE4	D12[A12/D12]I/O	DB12(入力扱いにすること)	*/
	/*		PE5	D13[A13/D13]I/O	DB13(入力扱いにすること)	*/
	/*		PE6	D14[A14/D14]I/O	DB14(入力扱いにすること)	*/
	/*		PE7	D15[A15/D15]I/O	DB15(入力扱いにすること)	*/
	/********************************************/
	PORTE.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORTE.ODR0.BYTE = 0x00;		//	B6, B4, B3, B2, B0
	PORTE.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTE.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTE.DSCR.BYTE = 0xFF;		//	B7(1), B6(1), B5(1), B4(1), B3(1), B2(1), B1(1), B0(1)
	PORTE.PDR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	PortF									*/
	/*		PF5	PF5		O(L)	BT_RES			*/
	/********************************************/
	PORTF.PMR.BYTE  = 0x00;		//	B7(-), B6(-), B5(0), B4(-), B3(-), B2(-), B1(-), B0(-)
	PORTF.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTF.ODR1.BYTE = 0x00;		//	B2, B0
	PORTF.PCR.BYTE  = 0x00;		//	B5, B4, B3, B2, B1, B0
	PORTF.PODR.BYTE = 0x00;		//	B5, B4, B3, B2, B1, B0
	PORTF.PDR.BYTE  = 0x20;		//	B7(-), B6(-), B5(1), B4(-), B3(-), B2(-), B1(-), B0(-)
	//	PORTF.PIDR.BYTE;		//	B5, B4, B3, B2, B1, B0
	/********************************************/
	/*	PortG, H								*/
	/********************************************/
	/********************************************/
	/*	PortJ									*/
	/*		PJ3	PJ3		O(H)	#BT_RTS			*/
	/*		PJ5	PJ5		I		#BT_CTS			*/
	/********************************************/
	PORTJ.PMR.BYTE  = 0x00;		//	B7(-), B6(-), B5(0), B4(-), B3(0), B2(-), B1(-), B0(-)
	PORTJ.ODR0.BYTE = 0x00;		//	B6
	PORTJ.ODR1.BYTE = 0x00;		//	B2
	PORTJ.PCR.BYTE  = 0x00;		//	B5, B3
	PORTJ.PODR.BYTE = 0x08;		//	B5, B3
	PORTJ.PDR.BYTE  = 0x08;		//	B7(-), B6(-), B5(0), B4(-), B3(1), B2(-), B1(-), B0(-)
	/********************************************/
	/*	PortK									*/
	/*		PK2	TXD9	O		#NT_TXD			*/
	/*		PK3	RXD9	I		#NT_RXD			*/
	/*		PK4	RXD4	I		#FL_RXD			*/
	/*		PK5	TXD4	O		#FL_TXD			*/
	/********************************************/
	PORTK.PMR.BYTE  = 0x3C;		//	B7(-), B6(-), B5(1), B4(1), B3(1), B2(1), B1(-), B0(-)
	PORTK.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTK.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTK.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTK.PDR.BYTE  = 0x24;		//	B7(-), B6(-), B5(1), B4(0), B3(0), B2(1), B1(-), B0(-)
	/********************************************/
	/*	PortL									*/
	/*		PL0	PL0		I		#MRW_DCD		*/
	/*		PL1	PL1		I		#MRW_DSR		*/
	/********************************************/
	PORTL.PMR.BYTE  = 0x00;		//	B7(-), B6(-), B5(-), B4(-), B3(-), B2(-), B1(0), B0(0)
	PORTL.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTL.ODR1.BYTE = 0x00;		//	B0
	PORTL.PCR.BYTE  = 0x00;		//	B4, B3, B2, B1, B0
	PORTL.PDR.BYTE  = 0x00;		//	B7(-), B6(-), B5(-), B4(-), B3(-), B2(-), B1(0), B0(0)
}

/****************************************************************************/
/*																			*/
/*	syscr Register Initialize												*/
/*		call元 = 															*/
/****************************************************************************/
void syscr_init( void )
{
	unsigned short short_dummy;
	//プロテクトレジスタ解除(クロック発生関連レジスタ、動作モード関連レジスタの書き込みを許可する)
	SYSTEM.PRCR.WORD = 0xA503;		// KEY:8,b7-4,PRC3(0),b2,PRC1(1),PRC0(1)
	SYSTEM.SYSCR0.WORD = 0x5A01;	//	KEY:8, b7-b2, EXBE:1(0), ROME:1(1)
	for(;;){//書き換わったことを確認する
		short_dummy = SYSTEM.SYSCR0.WORD & 0x00FF;
		if(0x0001 == short_dummy){
			break;
		}
	}
	SYSTEM.SYSCR1.WORD = 0x0001;	//	b15-b1, RAME:1(1=default:内蔵RAM有効)
}
