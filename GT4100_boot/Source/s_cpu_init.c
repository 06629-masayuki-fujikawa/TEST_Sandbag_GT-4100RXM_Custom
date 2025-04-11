//[]----------------------------------------------------------------------[]
///	@file		s_cpu_init.c
///	@brief		CPU�����ݒ菈��
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
/*            �I�v�V�����ݒ胁�����̐ݒ�
/****************************************************************************/
// RX630�ł̓G���f�B�A���̐ݒ������v���O����ROM�ɏ������ޕK�v������̂ł��̏��u�������ōs�Ȃ�
#pragma address OFS1_REG = 0xFFFFFF88			// OFS1 register
const unsigned long OFS1_REG = 0xFFFFFFFF;		// �f�t�H���g

#pragma address OFS0_REG = 0xFFFFFF8C			// OFS0 register
const unsigned long OFS0_REG = 0xFFFFFFFF;		// �f�t�H���g

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
	/*			Mode�ݒ�															*/
	/*				reset��������SingleChipMode�ɂċN����(��1�̐ݒ�)				*/
	/*				�����ݒ芮����A����ROM�L���g��Mode�Ɉڍs����(��2�ݒ�)			*/
	/*				MD�[�q = H(SingleChipMode�N��)									*/
	/*			�^�pMode�ݒ�														*/
	/*				1.	MDEB.MDE[2:0]����MDES.MDE[2:0]								*/
	/*						000b		Big				o							*/
	/*						111b		Little			x							*/
	/*			����Mode *1=SYSCR0													*/
	/*	--------------------------------------------------------------				*/
	/*			MD�[�q	ROME*1 	EXBE*1	����Mode				����ROM/�O��Bus		*/
	/*			0 		1 		0 		SCI�u�[�gMode			�L��/����			*/
	/*			0 		1 		0 		USB/USER�u�[�gMode		�L��/����			*/
	/*			1 		1 		0 		�V���O���`�b�vMode		�L��/����	<-	��1	*/
	/*	--------------------------------------------------------------				*/
	/*					ROME*1 	EXBE*1	����Mode				����ROM/�O��Bus		*/
	/*					0 		0		�V���O���`�b�vMode		����/����			*/
	/*					1 		0 		�V���O���`�b�vMode		�L��/����	<-	��1	*/
	/*					0 		1 		����ROM�����g��Mode		����/�L��			*/
	/*					1 		1 		����ROM�L���g��Mode		�L��/�L��	<-	��2	*/
	/********************************************************************************/
	/********************************************************/
	/*	PowerOnReset����Register�̒l						*/
	/*		R0(SP) - R15	(�s��?)							*/
	/*		ISP 	(H'00000000)							*/
	/*		USP 	(H'00000000)							*/
	/*		INTB	(�s��)									*/
	/*		PC 		(resetVector�̒l)						*/
	/*		PSW 	(H'00000000)							*/
	/*			I(b16)				INT(0/1 = DI/EI)		*/
	/*			U(b17)				0/1 = ISP/USP			*/
	/*			PM(b20)				0/1 = supperuser/user	*/
	/*			IPL[3:0](b24-b27)	INT priority level		*/
	/********************************************************/

	/****************************************/
	/*		9	Clock������H				*/
	/****************************************/
	SYSTEM.SCKCR.LONG = 0x21821211;	//	FCK:4(0010b:4����), ICK:4(0001b:2����), PSTOP1:1(1), b22-b20(000b), BCK:4(0010b:4����)
									//	b15-b12(0001b), PCKB:4(0010b:4����), b7-b4(0001b), b3-b0(0001b)
	SYSTEM.SCKCR2.WORD = 0x0011;	//	b15-b8(00000000b), UCK:4(0001b), EBCK:4(0001b:2����)
	SYSTEM.SCKCR3.WORD = 0x0400;	//	b15-b11(00000b), CKSEL:3(100b=PLL��H�I��), IEBCK:8(00000000b)
	// ���C���N���b�N���U����ҋ@���Ԃ�20mS�ȏ�(�n�[�h�E�F�A�}�j���A��45.4�N���b�N�^�C�~���O)���
	// �����̎��g��12.5MHz��(b4-0:01110)262144�T�C�N���Ƃ����20.97ms
	SYSTEM.MOSCWTCR.BYTE = 0x0E;	//	���C���N���b�N���U��E�F�C�g�R���g���[�����W�X�^
	SYSTEM.MOSCCR.BYTE = 0x00;		//	b7-b1(0000000b), MOSTP:1(0=MainClock���U�퓮��)
	for(;;){
		dummy = SYSTEM.MOSCCR.BYTE;
		if(0x00 == dummy){
			break;
		}
	}
	wait2us(15000L);				// 30ms�E�F�C�g(20.97�ɑ΂��ă}�[�W�����Ƃ���)
	SYSTEM.PLLCR.WORD = 0x0F00;		//	b15-b14(00b), STC:6(001111b=x16), b7-b2(000000b), PLIDIV:2(00b:1����)
	// PLL�N���b�N���U����ҋ@���Ԃ�1.5mS�ȏ�(�n�[�h�E�F�A�}�j���A��45.4�N���b�N�^�C�~���O)���
	// PLL�̔��U���g��200MHz��(b4-0:01100)524288�T�C�N���Ƃ����2.62ms
	SYSTEM.PLLWTCR.BYTE = 0x0C;		//	PLL�E�F�C�g�R���g���[�����W�X�^
	SYSTEM.PLLCR2.BYTE = 0x00;		//	b7-b1(0000000b), PLLEN:1(0=PLL����ON)
	SYSTEM.BCKCR.BYTE = 0x01;		//	b7-b1(0000000b), BCLKDIV:1(1=BCLK��2����)
	wait2us(1400L);					// 2.8ms�E�F�C�g(2.62�ɑ΂��ă}�[�W�����Ƃ���)
	SYSTEM.SOSCCR.BYTE = 0x01;		//	b7-b1(0000000b), SOSTP:1(1=SubClock���U���~)
	SYSTEM.LOCOCR.BYTE = 0x01;		//	b7-b1(0000000b), LCSTP:1(1=LOCO�����~)
	SYSTEM.ILOCOCR.BYTE = 0x00;		//	b7-b1(0000000b), ILCSTP:1(0=IWDT��pOnChipOscrator����ON)
	SYSTEM.HOCOCR.BYTE = 0x01;		//	b7-b1(0000000b), HCSTP:1(1=HOCO�����~)
	SYSTEM.OSTDCR.BYTE = 0x00;		//	OSTDE:1, b6-b1(000000b), OSTDIE:1
	SYSTEM.OSTDSR.BYTE = 0x00;		//	b7-b1(0000000b), OSTDF:1(0=MainClock���U��~�𖢌��o)
	SYSTEM.MOFCR.BYTE = 0x00;		//	b7-b1(0000000b), MOFXIN:1
	SYSTEM.HOCOPCR.BYTE = 0x01;		//	b7-b1(0000000b), HOCOPCNT:1(1=HOCO�̓d��OFF)

	//	�����ݒ芮����̓���ROM�L���g��Mode�̐ݒ菈��
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
	BSC.CS1MOD.WORD = 0x0301;		//	PRMOD:1(0=NormalAccess�݊�Mode), b14-b10, PWENB:1(1=PageWriteAccess����), PRENB:1(1=PageReadAccess����)
									//	b7-b4, EWENB:1(0=�O��Wait�֎~), b2-b1, WRMOD:1(1=1WriteStrobeMode))
	BSC.CS1WCR1.LONG = 0x04040202;	//	b31-b29, CSRWAIT:5(00100=NormalReadCycleWait4Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
									//	b15-b11, CSPRWAIT:3(010=PageReadCycleWait 2cycle), b7-b3, CSPWWAIT:3(010=PageWriteCycleWait 2cycle)
	BSC.CS1WCR2.LONG = 0x01110122;	//	b31, CSON:3(000=CSAssertWait�}�����Ȃ�), b27, WDON:3(001=WriteData�o��Wait1Cycle), b23, WRON:3(001=WRAssertWait1Cycle), b19, RDON:3(001=RDAssertWait1Cycle)
									//	b15-b14, AWAIT:2(00=AddressCycleWait�}�����Ȃ�), b11, WDOFF:3(001=WriteData�o�͉���1Cycle), b7, CSWOFF:3(010=Write��CS����2Cycle), b3, CSROFF:3(010=Read��CS����2Cycle)
	BSC.CS1CR.WORD = 0x0001;		//	b15-b13, MPXEN:1(0=SeparateBusIF/Address,DataMultiPlexI/OIF), b11-b9, EMODE:1(0=Endian�͓���Mode��Endian�Ɠ���) 
									//	b7-b6, BSIZE:2(00=16BitBus��Ԃɐݒ�), b3-b1, EXENB:1(1=���싖��)
	/************************/
	/*	CS2					*/
	/************************/
	BSC.CS2REC.WORD = 0x0101;		//	b15-b12, WRCV:4(0001=WriteRecovery1Cycle), b7-b4, RRCV:4(0001=ReadRecovery1Cycle)
	BSC.CS2MOD.WORD = 0x0301;		//	PRMOD:1(0=NormalAccess�݊�Mode), b14-b10, PWENB:1(1=PageWriteAccess����), PRENB:1(1=PageReadAccess����)
									//	b7-b4, EWENB:1(0=�O��Wait�֎~), b2-b1, WRMOD:1(1=1WriteStrobeMode))
	BSC.CS2WCR1.LONG = 0x04040202;	//	b31-b29, CSRWAIT:5(00100=NormalReadCycleWait4Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
									//	b15-b11, CSPRWAIT:3(010=PageReadCycleWait 2cycle), b7-b3, CSPWWAIT:3(010=PageWriteCycleWait 2cycle)
	BSC.CS2WCR2.LONG = 0x01110122;	//	b31, CSON:3(000=CSAssertWait�}�����Ȃ�), b27, WDON:3(001=WriteData�o��Wait1Cycle), b23, WRON:3(001=WRAssertWait1Cycle), b19, RDON:3(001=RDAssertWait1Cycle)
									//	b15-b14, AWAIT:2(00=AddressCycleWait�}�����Ȃ�), b11, WDOFF:3(001=WriteData�o�͉���1Cycle), b7, CSWOFF:3(010=Write��CS����2Cycle), b3, CSROFF:3(010=Read��CS����2Cycle)
	BSC.CS2CR.WORD = 0x0001;		//	b15-b13, MPXEN:1(0=SeparateBusIF/Address,DataMultiPlexI/OIF), b11-b9, EMODE:1(0=Endian�͓���Mode��Endian�Ɠ���) 
									//	b7-b6, BSIZE:2(00=16BitBus��Ԃɐݒ�), b3-b1, EXENB:1(1=���싖��)
	/************************/
	/*	CS3					*/
	/************************/
	BSC.CS3REC.WORD = 0x0101;		//	b15-b12, WRCV:4(0001=WriteRecovery1Cycle), b7-b4, RRCV:4(0001=ReadRecovery1Cycle)
	BSC.CS3MOD.WORD = 0x0301;		//	PRMOD:1(0=NormalAccess�݊�Mode), b14-b10, PWENB:1(1=PageWriteAccess����), PRENB:1(1=PageReadAccess����)
									//	b7-b4, EWENB:1(0=�O��Wait�֎~), b2-b1, WRMOD:1(1=1WriteStrobeMode))
	BSC.CS3WCR1.LONG = 0x06040202;	//	b31-b29, CSRWAIT:5(00110=NormalReadCycleWait6Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
									//	b15-b11, CSPRWAIT:3(010=PageReadCycleWait 2cycle), b7-b3, CSPWWAIT:3(010=PageWriteCycleWait 2cycle)
									//	b15-b14, AWAIT:2(00=AddressCycleWait�}�����Ȃ�), b11, WDOFF:3(010=WriteData�o�͉���2Cycle), b7, CSWOFF:3(011=Write��CS����3Cycle), b3, CSROFF:3(010=Read��CS����2Cycle)
	BSC.CS3WCR2.LONG = 0x01310232;	// 2012/03/01 �C���˗��ɂ��ύX

	BSC.CS3CR.WORD = 0x0001;		//	b15-b13, MPXEN:1(0=SeparateBusIF/Address,DataMultiPlexI/OIF), b11-b9, EMODE:1(0=Endian�͓���Mode��Endian�Ɠ���) 
									//	b7-b6, BSIZE:2(00=16BitBus��Ԃɐݒ�), b3-b1, EXENB:1(1=���싖��)
	/************************/
	/*	CS4					*/
	/************************/
	BSC.CS4REC.WORD = 0x0101;		//	b15-b12, WRCV:4(0001=WriteRecovery1Cycle), b7-b4, RRCV:4(0001=ReadRecovery1Cycle)
	BSC.CS4MOD.WORD = 0x0001;		//	PRMOD:1(0=NormalAccess�݊�Mode), b14-b10, PWENB:1(0=PageWriteAccess�֎~), PRENB:1(0=PageReadAccess�֎~)
									//	b7-b4, EWENB:1(0=�O��Wait�֎~), b2-b1, WRMOD:1(1=1WriteStrobeMode))
	BSC.CS4WCR1.LONG = 0x03020000;	//	b31-b29, CSRWAIT:5(00011=NormalReadCycleWait3Cycle), b23-b21, CSWWAIT:5(00010=NormalWriteCycleWait2Cycle)
									//	b15-b11, CSPRWAIT:3(000=PageReadCycleWait�}�����Ȃ�), b7-b3, CSPWWAIT:3(000=PageWriteCycleWait�}�����Ȃ�)
	BSC.CS4WCR2.LONG = 0x01110122;	//	b31, CSON:3(000=CSAssertWait�}�����Ȃ�), b27, WDON:3(001=WriteData�o��Wait1Cycle), b23, WRON:3(001=WRAssertWait1Cycle), b19, RDON:3(001=RDAssertWait1Cycle)
									//	b15-b14, AWAIT:2(00=AddressCycleWait�}�����Ȃ�), b11, WDOFF:3(001=WriteData�o�͉���1Cycle), b7, CSWOFF:3(010=Write��CS����2Cycle), b3, CSROFF:3(010=Read��CS����2Cycle)
	BSC.CS4CR.WORD = 0x0001;		//	b15-b13, MPXEN:1(0=SeparateBusIF/Address,DataMultiPlexI/OIF), b11-b9, EMODE:1(0=Endian�͓���Mode��Endian�Ɠ���) 
									//	b7-b6, BSIZE:2(00=16BitBus��Ԃɐݒ�), b3-b1, EXENB:1(1=���싖��)
	/************************/
	/*	����				*/
	/************************/
	BSC.CSRECEN.WORD = 0x3EFE;		//	RCVENM7:1 - RCVENM0:1(00111110b), RCVEN7:1 - RCVEN0:1(11111110b)
	BSC.BEREN.BYTE = 0x03;			//	b7-b2(000000b), TOEN:1(1=BusTimeout���o����), IGAEN:1(1=�s��AddressAccess���o����)


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
	/*	CS1(P71=01)	MainMemory1			SRAM_2MB(Backup�L)			*/
	/*	CS2(P72=01)	MainMemory2			SRAM_1MB(Backup�L)			*/
	/*	CS3(P63=00)	EthernetControler	???							*/
	/*	CS4(P64=00)	�g��I/O				����:16Port�C�o��:16Port	*/
	/*	A1 - A19, A22												*/
	/****************************************************************/
	MPC.PFCSS0.BYTE = 0x14;		//	CS3S:2, CS2S:2, CS1S:2, b1, CS0S:1(CS�o�͒[�q�I��Register0)
	MPC.PFCSS1.BYTE = 0x00;		//	CS7S:2, CS6S:2, CS5S:2, CS4S:2(CS�o�͒[�q�I��Register1)
	MPC.PFAOE0.BYTE = 0xFF;		//	A15E:1 - A8E:1(Address�o�͋���Register0)
	MPC.PFAOE1.BYTE = 0x4F;		//	A23E:1 - A16E:1(Address�o�͋���Register1)
	MPC.PFBCR0.BYTE = 0x51;		//	WR32BC32E:1, WR1BC1E:1, DH32E:1, DHE:1, b3-b2, ADRHMS:1, ADRLE:1(�O��Bus����Register0:20.2.25)
	MPC.PFBCR1.BYTE = 0x00;		//	b7-b3, ALEOE:1(P54=IO), WAITS:2(WAIT=?)(�O��Bus����Register1:20.2.26)
	MPC.PFCSE.BYTE  = 0x1E;		//	CS7E:1 - CS0E:1(CS�o�͋���Register:20.2.20) = CS1,2,3,4�̂ݏo�͋���
	MPC.PWPR.BYTE   = 0x00;		//	B0WI:0, PFSWE:0(������ProtectRegister:20.2.1), b5-b0
	MPC.PWPR.BYTE   = 0x40;		//	B0WI:0, PFSWE:1(������ProtectRegister:20.2.1), b5-b0
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P00		TXD6	O		BT_TXD		1			0		Bluetooth		Bluetooth�ʐMData���M�M��		8		*/
	/*	P01		RXD6	I		BT_RXD		1			0		Bluetooth		Bluetooth�ʐMData��M�M��		7		*/
	/*	P02		P02		O(L)	#MUTE		�ʏ�		MUTE	��������		�f�W�^���A���vMUTE�M��			6		*/
	/*	P03		P03		O(L)	FBSH_O		Open		�ʏ�	���CReader����	���CReader�V���b�^Open�M��		4		*/
	/*	P05		P05		O(L)	FBSH_C		Close		�ʏ�	���CReader����	���CReader�V���b�^Close�M��		2		*/
	/********************************************************************************************************************/
	MPC.P00PFS.BYTE = 0x0A;	//	ASEL:1(Analog), ISEL:1(IRQ), b5, PSEL:5(Port)
	MPC.P01PFS.BYTE = 0x0A;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.P02PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.P03PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.P05PFS.BYTE = 0x00;	//	ASEL:1, ISEL:1, b5, PSEL:5
	MPC.P07PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P12		RXD2	I		#JVMA_RXD	0			1		JVMA			JVMA�ʐMData��M�M��			45		*/
	/*	P13		TXD2	O		#JVMA_TXD	0			1		JVMA			JVMA�ʐMData���M�M��			44		*/
	/*	P14		IRQ4	I		#RTC_IRQ	�ʏ�		H/W-IRQ	RTC				RTC���������M��(1min�Ԋu)		43		*/
	/*	P15		IRQ5	I		#EX_IRQ1	�ʏ�		H/W-IRQ	CPU(IIC)		UARTcontroler1�����ݐM��		42		*/
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
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P20		SDA1	I/O		SDA			1			0		CPU(IIC)		I2CData							37		*/
	/*	P21		SCL1	O		SCL			-			-		CPU(IIC)		I2CClock						36		*/
	/*	P22		P22		O(H)	#FB_MD2		�ʏ�		������	���CReader����	FB-7000bootMode�I��M��MD2		35		*/
	/*	P23		TXD3	O		#FB_TXD		1			0		���CReader����	FB-7000Data���M�M��				34		*/
	/*	P24		P24		O(L)	FB_FW		������		�ʏ�	���CReader����	FB-7000bootMode�I��M��FW		33		*/
	/*	P25		RXD3	I		#FB_RXD		1			0		���CReader����	FB-7000Data��M�M��				32		*/
	/*	P26		TDO		O		TDO			1			0		Debug/Program	Debuger�ʐMTXD(WriterTXD�Ƌ��L)	31		*/
	/*	P27		TCK		I		TCK			-			-		Debug/Program	Debuger�ʐMCLK(WriterCLK�Ƌ��L)	30		*/
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
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P30		TDI		I		TDI			1			0		Debug/Program	Debuger�ʐMRXD(WriterRXD�Ƌ��L)	29		*/
	/*	P31		TMS		I		TMS								Debug/Program									28		*/
	/*	P32		P32		O(H)	#FB_RES		�ʏ�		reset	���CReader����	FB-7000reset�M��				27		*/
	/*	P33		TIOCD0	O		SOUND		-			-		��������		�����o��						26		*/
	/*	P34		#TRST	I		��TRST		�ʏ�		reset	Debug/Program									25		*/
	/*	P35		NMI		I		#NMI		�ʏ�		NMI		�d��/reset		NMI�M���Ď�						24		*/
	/*	P36		EXTAL	I		EXTAL		-			-		CPU				�O�������U���q�ڑ�				22		*/
	/*	P37		XTAL	O		XTAL		-			-		CPU				�O�������U���q�ڑ�				20		*/
	/********************************************************************************************************************/
	//	�\20.10
	MPC.P30PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P31PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P32PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P33PFS.BYTE = 0x03;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P34PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P40		IRQ8-DS	I		#EX_IRQ2	�ʏ�		H/W-IRQ	CPU(IIC)		UARTcontroler2�����ݐM��		141		*/
	/*	P41		P41		O(L)	SF_RES		reset�v��	�ʏ�	serverFOMA		serverFOMA_reset�M��			139		*/
	/*	P42		P42		O(L)	#SF_DTR		disable		enable	serverFOMA		serverFOMA_ER�M��				138		*/
	/*	P43		P43		O(L)	#SF_RTS		disable		enable	serverFOMA		serverFOMA_RS�M��				137		*/
	/*	P44		P44		I		#SF_RI		disable		enable	serverFOMA		serverFOMA_CI�M��				136		*/
	/*	P45		P45		I		#SF_DCD		disable		enable	serverFOMA		serverFOMA_CD�M��				135		*/
	/*	P46		P46		I		#SF_DSR		disable		enable	serverFOMA		serverFOMA_DR�M��				134		*/
	/*	P47		P47		I		#SF_CTS		disable		enable	serverFOMA		serverFOMA_CS�M��				133		*/
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
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P50		#WR0	O		#WR			disable		enable	CPU				���C�genable�M��				56		*/
	/*	P51		#BC1	O		#BC1		disable		enable	CPU				address�I��(UB)					55		*/
	/*	P52		#RD		O		#RD			disable		enable	CPU				���[�h�M��						54		*/
	/*	P53		BCLK	O		BCLK		1			0		CPU				���g�p(Bus�I������P51�g�p�s��)	53		*/
	/*	P54		CTX1	O		CTX			1			0		�@����ʐM		CAN_TXD�M��						52		*/
	/*	P55		CRX1	I		CRX			1			0		�@����ʐM		CAN_RXD�M��						51		*/
	/*	P56		P56		O(L)	#CTERM		�I�[����	�I�[�L��@����ʐM		CAN_�I�[��R����M��(LV14�Ŕ��])50		*/
	/********************************************************************************************************************/
	//	�\20.12
	MPC.P50PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P51PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P52PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P54PFS.BYTE = 0x10;	//	b7-b5, PSEL:5
	MPC.P55PFS.BYTE = 0x10;	//	b7, ISEL:1, b5, PSEL:5
	MPC.P56PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P60		P60		O(H)	#NT_TEN		disable		enable	NT-NET�ʐM		NT-NET�ʐM���Menable�M��		117		*/
	/*	P61		P61		O(L)	FAN_SW	I	ON			OFF		FAN����			FAN����SW�M��					115		*/
	/*	P62		P62		I		#DRSW		�h�A�J		�h�A��	�e��Z���T�Ď�	�h�A�J�Z���T�M��				114		*/
	/*	P63		#CS3	O		#CS3		disable		enable	Ethernet		LANcontrolerChipSelect�M��		113		*/
	/*	P64		#CS4	O		#CS4		disable		enable	CPU				I/O�g��ChipSelect�M��			112		*/
	/*	P65		P65		O(L)	#OUT_EN	I	disable		enable	CPU				I/O�g�����Outputenable�M��		100		*/
	/*	P66		P66		O(L)	CAN_RES	I	reset		�ʏ�	�@����ʐM		�@����ʐMreset�M��				99		*/
	/*	P67		IRQ15	I		#ETHER_IRQ	�ʏ�		H/W-IRQ	Ethernet		Ethernetcontroler�����ݐM��		98		*/
	/********************************************************************************************************************/
	MPC.P60PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P61PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P66PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P67PFS.BYTE = 0x40;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P70		P70		O(H)	#FL_TEN		disable		enable	Flap����		Flap�ʐM_���Menable�M��			104		*/
	/*	P71		#CS1	O		#CS1		disable		enable	CPU				SRAM1ChipSelect�M��				86		*/
	/*	P72		#CS2	O		#CS2		disable		enable	CPU				SRAM2ChipSelect�M��				85		*/
	/*	P73		P73		I		#MRW_CTS	disable		enable	MultiICcardR/W	MultiICcardR/W_CS�M��			77		*/
	/*	P74		P74		O(H)	#MRW_DTR	disable		enable	MultiICcardR/W	MultiICcardR/W_ER�M��			72		*/
	/*	P75		P75		O(H)	#MRW_RTS	disable		enable	MultiICcardR/W	MultiICcardR/W_RS�M��			71		*/
	/*	P76		RXD11	I		#MRW_RXD	0			1		MultiICcardR/W	MultiICcardR/WData��M�M��		69		*/
	/*	P77		TXD11	O		#MRW_TXD	0			1		MultiICcardR/W	MultiICcardR/WData���M�M��		68		*/
	/********************************************************************************************************************/
	MPC.P70PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P73PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P74PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P75PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P76PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.P77PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P80		P80		I		#MRW_RI		disable		enable	MultiICcardR/W	MultiICcardR/W_CI�M��			65		*/
	/*	P81		RXD10	I		#RW_RXD		0			1		��ڐGIC		��ڐGIC R/WData��M�M��		64		*/
	/*	P82		TXD10	O		#RW_TXD		0			1		��ڐGIC		��ڐGIC R/WData���M�M��		63		*/
	/*	P83		P83		O(L)	RW_RES		reset		�ʏ�	��ڐGIC		��ڐGIC reset�M��				58		*/
	/*	P86		P86		O(H)	#JVMA_SYC	disable		enable	JVMA			JVMA�ʐM�����M��				41		*/
	/*	P87		P87		O(L)	FB_LD		�_��		����	���CReader����	���CReader�V���b�^LED����M��	39		*/
	/********************************************************************************************************************/
	MPC.P80PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P81PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.P82PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.P83PFS.BYTE = 0x00;	//	b7-b5, SEL:5
	MPC.P86PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	MPC.P87PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	P90		TXD7	O		#SF_TXD		0			1		serverFOMA		serverFOMAData���M�M��			131		*/
	/*	P91		P91		O(H)	BT_MODE		Auto		Command	Bluetooth		BluetoothMode�I��M��			129		*/
	/*	P92		RXD7	I		#SF_RXD		0			1		serverFOMA		serverFOMAData��M�M��			128		*/
	/*	P93		P93		O(L)	BT_ROLE		master		slave	Bluetooth		Bluetoothmaster/slave�I��M��	127		*/
	/********************************************************************************************************************/
	MPC.P90PFS.BYTE = 0x0A;	//	ASEL:1, b6-b5, PSEL:5
	MPC.P91PFS.BYTE = 0x00;	//	ASEL:1, b6-b5, PSEL:5
	MPC.P92PFS.BYTE = 0x0A;	//	ASEL:1, b6-b5, PSEL:5
	MPC.P93PFS.BYTE = 0x00;	//	ASEL:1, b6-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PA0		#BC0	O		#BC0		disable		enable	CPU				address�I��(LB)					97		*/
	/*	PA1		A1		O		AB1			1			0		CPU				�O��Bus_address					96		*/
	/*	PA2		A2		O		AB2			1			0		CPU				�O��Bus_address					95		*/
	/*	PA3		A3		O		AB3			1			0		CPU				�O��Bus_address					94		*/
	/*	PA4		A4		O		AB4			1			0		CPU				�O��Bus_address					92		*/
	/*	PA5		A5		O		AB5			1			0		CPU				�O��Bus_address					90		*/
	/*	PA6		A6		O		AB6			1			0		CPU				�O��Bus_address					89		*/
	/*	PA7		A7		O		AB7			1			0		CPU				�O��Bus_address					88		*/
	/********************************************************************************************************************/
	//	�\20.19
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
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PB0		A8		O		AB8			1			0		CPU				�O��Bus_address					87		*/
	/*	PB1		A9		O		AB9			1			0		CPU				�O��Bus_address					84		*/
	/*	PB2		A10		O		AB10		1			0		CPU				�O��Bus_address					83		*/
	/*	PB3		A11		O		AB11		1			0		CPU				�O��Bus_address					82		*/
	/*	PB4		A12		O		AB12		1			0		CPU				�O��Bus_address					81		*/
	/*	PB5		A13		O		AB13		1			0		CPU				�O��Bus_address					80		*/
	/*	PB6		A14		O		AB14		1			0		CPU				�O��Bus_address					79		*/
	/*	PB7		A15		O		AB15		1			0		CPU				�O��Bus_address					78		*/
	/********************************************************************************************************************/
	//	�\20.21
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
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PC0		A16		O		AB16		1			0		CPU				�O��Busaddress					75		*/
	/*	PC1		A17		O		AB17		1			0		CPU				�O��Busaddress					73		*/
	/*	PC2		A18		O		AB18		1			0		CPU				�O��Busaddress					70		*/
	/*	PC3		A19		O		AB19		1			0		CPU				�O��Busaddress					67		*/
	/*	PC4		SSLA0	O		#SSL0		disable		enable	CPU(SPI)		SPI-FROMChipSelect�M��			66		*/
	/*	PC5		RSPCK	I		RSPCK		-			-		CPU(SPI)		SPIClock�M��					62		*/
	/*	PC6		A22		O		A22			1			0		CPU				�O��Busaddress					61		*/
	/*	PC7		PC7		I		MODE		Userboot	SCIboot	Debug/Program	bootMode�ؑ�					60		*/
	/********************************************************************************************************************/
	//	�\20.23
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
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PD0		D0[A0/D0]	I/O	DB0			1			0		CPU				�O��Bus_Data					126		*/
	/*	PD1		D1[A1/D1]	I/O	DB1			1			0		CPU				�O��Bus_Data					125		*/
	/*	PD2		D2[A2/D2]	I/O	DB2			1			0		CPU				�O��Bus_Data					124		*/
	/*	PD3		D3[A3/D3]	I/O	DB3			1			0		CPU				�O��Bus_Data					123		*/
	/*	PD4		D4[A4/D4]	I/O	DB4			1			0		CPU				�O��Bus_Data					122		*/
	/*	PD5		D5[A5/D5]	I/O	DB5			1			0		CPU				�O��Bus_Data					121		*/
	/*	PD6		D6[A6/D6]	I/O	DB6			1			0		CPU				�O��Bus_Data					120		*/
	/*	PD7		D7[A7/D7]	I/O	DB7			1			0		CPU				�O��Bus_Data					119		*/
	/********************************************************************************************************************/
	//	�\20.26
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
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PE0		D8[A8/D8]	I/O	DB8			1			0		CPU				�O��Bus_Data					111		*/
	/*	PE1		D9[A9/D9]	I/O	DB9			1			0		CPU				�O��Bus_Data					110		*/
	/*	PE2		D10[A10/D10]I/O	DB10		1			0		CPU				�O��Bus_Data					109		*/
	/*	PE3		D11[A11/D11]I/O	DB11		1			0		CPU				�O��Bus_Data					108		*/
	/*	PE4		D12[A12/D12]I/O	DB12		1			0		CPU				�O��Bus_Data					107		*/
	/*	PE5		D13[A13/D13]I/O	DB13		1			0		CPU				�O��Bus_Data					106		*/
	/*	PE6		D14[A14/D14]I/O	DB14		1			0		CPU				�O��Bus_Data					102		*/
	/*	PE7		D15[A15/D15]I/O	DB15		1			0		CPU				�O��Bus_Data					101		*/
	/********************************************************************************************************************/
	//	�\20.29
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
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PF5		PF5		O(L)	BT_RES		reset		�ʏ�	Bluetooth		BluetoothMojulereset�M��		9		*/
	/********************************************************************************************************************/
	MPC.PF5PFS.BYTE = 0x00;	//	b7, ISEL:1, b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PJ3		PJ3		O(H)	#BT_RTS		disable		enable	Bluetooth		BluetoothMojule���MClear�M��	13		*/
	/*	PJ5		PJ5		I		#BT_CTS		disable		enable	Bluetooth		BluetoothMojule���MRequest�M��	11		*/
	/********************************************************************************************************************/
	MPC.PJ3PFS.BYTE = 0x00;	//	b7-b5, PSEL:5
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PK2		TXD9	O		#NT_TXD		0			1		NT-NET�ʐM		NT-NETData���M�M��				118		*/
	/*	PK3		RXD9	I		#NT_RXD		0			1		NT-NET�ʐM		NT-NETData��M�M��				116		*/
	/*	PK4		RXD4	I		#FL_RXD		0			1		Flap����		Flap�ʐMData��M�M��			105		*/
	/*	PK5		TXD4	O		#FL_TXD		0			1		Flap����		Flap�ʐMData���M�M��			103		*/
	/********************************************************************************************************************/
	MPC.PK2PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.PK3PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.PK4PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.PK5PFS.BYTE = 0x0A;	//	b7-b5, PSEL:5
	MPC.PWPR.BYTE   = 0x00;		//	B0WI:0, PFSWE:0(������ProtectRegister:20.2.1), b5-b0
	/********************************************************************************************************************/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	Port	�@�\1(Init�l)	�M����		Data(H)		(L)		Block			���e							PinNo	*/
	/*	---------------------------------------------------------------------------------------------------------------	*/
	/*	PL0		PL0		I		#MRW_DCD	disable		enable	MultiICcardR/W	MultiICcardR/W_CD�M��			76		*/
	/*	PL1		PL1		I		#MRW_DSR	disable		enable	MultiICcardR/W	MultiICcardR/W_DR�M��			74		*/
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
	/*		P34	#TRST	I		��TRST			*/
	/*		P35	NMI		I		#NMI			*/
	/*		P36	EXTAL	0		EXTAL(���͈����ɂ��邱��)	*/
	/*		P37	XTAL	O		XTAL(���͈����ɂ��邱��)	*/
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
	/*		P50	#WR0	I		#WR(���͈����ɂ��邱��)	*/
	/*		P51	#BC1	I		#BC1(���͈����ɂ��邱��)*/
	/*		P52	#RD		I		#RD(���͈����ɂ��邱��)	*/
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
	/*		P63	#CS3	I		#CS3(���͈����ɂ��邱��)*/
	/*		P64	#CS4	I		#CS4(���͈����ɂ��邱��)*/
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
	/*		P71	#CS1	O		#CS1(���͈����ɂ��邱��)*/
	/*		P72	#CS2	O		#CS2(���͈����ɂ��邱��)*/
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
	/*		PA0	#BC0	O		#BC0(���͈����ɂ��邱��)*/
	/*		PA1	A1		O		AB1(���͈����ɂ��邱��)	*/
	/*		PA2	A2		O		AB2(���͈����ɂ��邱��)	*/
	/*		PA3	A3		O		AB3(���͈����ɂ��邱��)	*/
	/*		PA4	A4		O		AB4(���͈����ɂ��邱��)	*/
	/*		PA5	A5		O		AB5(���͈����ɂ��邱��)	*/
	/*		PA6	A6		O		AB6(���͈����ɂ��邱��)	*/
	/*		PA7	A7		O		AB7(���͈����ɂ��邱��)	*/
	/********************************************/
	PORTA.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORTA.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTA.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTA.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTA.DSCR.BYTE = 0xFF;		//	B7(1), B6(1), B5(1), B4(1), B3(1), B2(1), B1(1), B0(1)
	PORTA.PDR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	PortB									*/
	/*		PB0	A8		O		AB8(���͈����ɂ��邱��)	*/
	/*		PB1	A9		O		AB9(���͈����ɂ��邱��)	*/
	/*		PB2	A10		O		AB10(���͈����ɂ��邱��)*/
	/*		PB3	A11		O		AB11(���͈����ɂ��邱��)*/
	/*		PB4	A12		O		AB12(���͈����ɂ��邱��)*/
	/*		PB5	A13		O		AB13(���͈����ɂ��邱��)*/
	/*		PB6	A14		O		AB14(���͈����ɂ��邱��)*/
	/*		PB7	A15		O		AB15(���͈����ɂ��邱��)*/
	/********************************************/
	PORTB.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORTB.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTB.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTB.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTB.DSCR.BYTE = 0xFF;		//	B7(1), B6(1), B5(1), B4(1), B3(1), B2(1), B1(1), B0(1)
	PORTB.PDR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	PortC									*/
	/*		PC0	A16		O		AB16(���͈����ɂ��邱��)*/
	/*		PC1	A17		O		AB17(���͈����ɂ��邱��)*/
	/*		PC2	A18		O		AB18(���͈����ɂ��邱��)*/
	/*		PC3	A19		O		AB19(���͈����ɂ��邱��)*/
	/*		PC4	SSLA0	O		#SSL0			*/
	/*		PC5	RSPCK	I		RSPCK			*/
	/*		PC6	A22		O		A22(���͈����ɂ��邱��)*/
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
	/*		PD0	D0[A0/D0]	I/O	DB0(���͈����ɂ��邱��)	*/
	/*		PD1	D1[A1/D1]	I/O	DB1(���͈����ɂ��邱��)	*/
	/*		PD2	D2[A2/D2]	I/O	DB2(���͈����ɂ��邱��)	*/
	/*		PD3	D3[A3/D3]	I/O	DB3(���͈����ɂ��邱��)	*/
	/*		PD4	D4[A4/D4]	I/O	DB4(���͈����ɂ��邱��)	*/
	/*		PD5	D5[A5/D5]	I/O	DB5(���͈����ɂ��邱��)	*/
	/*		PD6	D6[A6/D6]	I/O	DB6(���͈����ɂ��邱��)	*/
	/*		PD7	D7[A7/D7]	I/O	DB7(���͈����ɂ��邱��)	*/
	/********************************************/
	PORTD.PMR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	PORTD.ODR0.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTD.ODR1.BYTE = 0x00;		//	B6, B4, B2, B0
	PORTD.PCR.BYTE  = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
	PORTD.DSCR.BYTE = 0xFF;		//	B7(1), B6(1), B5(1), B4(1), B3(1), B2(1), B1(1), B0(1)
	PORTD.PDR.BYTE  = 0x00;		//	B7(0), B6(0), B5(0), B4(0), B3(0), B2(0), B1(0), B0(0)
	/********************************************/
	/*	PortE									*/
	/*		PE0	D8[A8/D8]	I/O	DB8(���͈����ɂ��邱��)	*/
	/*		PE1	D9[A9/D9]	I/O	DB9(���͈����ɂ��邱��)	*/
	/*		PE2	D10[A10/D10]I/O	DB10(���͈����ɂ��邱��)	*/
	/*		PE3	D11[A11/D11]I/O	DB11(���͈����ɂ��邱��)	*/
	/*		PE4	D12[A12/D12]I/O	DB12(���͈����ɂ��邱��)	*/
	/*		PE5	D13[A13/D13]I/O	DB13(���͈����ɂ��邱��)	*/
	/*		PE6	D14[A14/D14]I/O	DB14(���͈����ɂ��邱��)	*/
	/*		PE7	D15[A15/D15]I/O	DB15(���͈����ɂ��邱��)	*/
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
/*		call�� = 															*/
/****************************************************************************/
void syscr_init( void )
{
	unsigned short short_dummy;
	//�v���e�N�g���W�X�^����(�N���b�N�����֘A���W�X�^�A���샂�[�h�֘A���W�X�^�̏������݂�������)
	SYSTEM.PRCR.WORD = 0xA503;		// KEY:8,b7-4,PRC3(0),b2,PRC1(1),PRC0(1)
	SYSTEM.SYSCR0.WORD = 0x5A01;	//	KEY:8, b7-b2, EXBE:1(0), ROME:1(1)
	for(;;){//��������������Ƃ��m�F����
		short_dummy = SYSTEM.SYSCR0.WORD & 0x00FF;
		if(0x0001 == short_dummy){
			break;
		}
	}
	SYSTEM.SYSCR1.WORD = 0x0001;	//	b15-b1, RAME:1(1=default:����RAM�L��)
}
