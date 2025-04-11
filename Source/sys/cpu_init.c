/********************************************************************************************/
/*																							*/
/*		rx630_cpu_init.c																	*/
/*		cpu_init		hardware initialization												*/
/*						����Mode															*/
/*						Clock������H														*/
/*						����d�͒ጸ�@�\													*/
/*						Rest��H															*/
/*						�d�����o��H														*/
/*						���g������@�\														*/
/*						RegisterWriteProtection�@�\ 										*/
/*						BSC(BusControl)														*/
/*						ICU(InterruptControl)												*/
/*						(P33=TIOCD0)														*/
/*		port_init		I/O Port init														*/
/*						MPC(MultiFunctionPinControl)										*/
/*						PORT(PDR_PMR_ODR_PCR_DSCR_Setting)									*/
/*		port_nmi		I/O Port NMI init													*/
/*		dog_init		WATCHDOG timer start												*/
/*		dog_reset		WATCHDOG counter reset												*/
/*		syscr_init		syscr Register Initialize											*/
/*		nmiint_set		NMI���m����=NMI�׸ނ�Ă�RAM��ײ����øĂ�������					*/
/*	x	scpu_reset		���CPUؾ��															*/
/*		System_reset	���u(MainCPU)ؾ�� 													*/
/********************************************************************************************/
/*		TPU0_init		20msTimer�ݒ�(MHz)													*/
/*		TPU1_init		 2msTimer�ݒ�(MHz)													*/
/*		TPU2_init		 1msTimer�ݒ�(MHz)													*/
/*		CAN1_init		�@����ʐM															*/
/*		I2C1_init		RS232C�ʐMDevice����												*/
/*						RTC_init															*/
/*						SC16_I2CDEV2_1_init													*/
/*						SC16_I2CDEV2_2_init													*/
/*						SC16_RP_init														*/
/*						SC16_JP_init														*/
/*		Sci2Init		JVMA(���ү�,����ذ��)	4,800bps	RXI2, TXI2, TEI2, ERI2			*/
/*		Sci3Init		���CReader����			38,400bps	RXI3, TXI3, TEI3, ERI3			*/
/*		Sci4Init		Flap����				38,400bps	RXI4, TXI4, TEI4, ERI4			*/
/*		Sci6Init		Bluetooth				115,200bps	RXI6, TXI6, TEI6, ERI6			*/
/*		Sci7Init		serverFOMA_Rism			38.400bps	RXI7, TXI7, TEI7, ERI7			*/
/*		Sci9Init		NT-NET�ʐM				38,400bps	RXI9, TXI9, TEI9, ERI9			*/
/*		Sci10Init		��ڐGIC				38,400bps	RXI10, TXI10, TEI10, ERI10		*/
/*		Sci11Init		MultiICcardR/W			38,400bps	RXI11, TXI11, TEI11, ERI11		*/
/*		Unkown_vect		���o�^�޸��������̴װ����											*/
/*		Unkown_vect4																		*/
/*		Unkown_vect12																		*/
/*		Unkown_vect13																		*/
/********************************************************************************************/
#include	"iodefine.h"
#include	"vect.h"
#include	"extern_Debugdata.h"
#include 	"extern_I2Cdata.h"
#include	"rtc_readwrite.h"
#include	<machine.h>
#include	<_h_c_lib.h>
#include	"typedefine.h"
/****************************************************************************/
#include	"system.h"
#include	"mem_def.h"
#include	"prm_tbl.h"
#include	"IF.h"
#include	"can_api.h"
#include	"can_def.h"
// Sc16RecvInSending_IRQ5_Level_L start
extern	unsigned char 	I2cSendDataBuff[256];
extern	unsigned short 	I2cSendDataCnt;
extern	unsigned char 	I2cSendSlaveAddr;
extern	unsigned char 	I2cSendLsiAddr;
// Sc16RecvInSending_IRQ5_Level_L end
/****************************************************************************/
extern	void	KSG_RauSCI_Init( void );

/****************************/
/*	ProtoType Definitions	*/
/****************************/
void SC16_DataInit(void);
int SC16_CheckSPRregister(int DevName);
int SC16_RegDataGet(int DevName);
int SC16_I2CDEV2_2_init(void);
int SC16_RP_init(void);
int SC16_JP_init(void);
void RTC_init(void);
void I2cAM_AllClear(void);
int ExSRam_clear( void );
int SramWrite(	volatile unsigned long 	*src, unsigned long len_byte );
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

	set_intb(__sectop("C$VECT"));
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
	wait2us(1400L);				// 2.8ms�E�F�C�g(2.62�ɑ΂��ă}�[�W�����Ƃ���)
	SYSTEM.SOSCCR.BYTE = 0x01;		//	b7-b1(0000000b), SOSTP:1(1=SubClock���U���~)
	SYSTEM.LOCOCR.BYTE = 0x01;		//	b7-b1(0000000b), LCSTP:1(1=LOCO�����~)
	SYSTEM.ILOCOCR.BYTE = 0x00;		//	b7-b1(0000000b), ILCSTP:1(0=IWDT��pOnChipOscrator����ON)
	SYSTEM.HOCOCR.BYTE = 0x01;		//	b7-b1(0000000b), HCSTP:1(1=HOCO�����~)
	SYSTEM.OSTDCR.BYTE = 0x00;		//	OSTDE:1, b6-b1(000000b), OSTDIE:1
	SYSTEM.OSTDSR.BYTE = 0x00;		//	b7-b1(0000000b), OSTDF:1(0=MainClock���U��~�𖢌��o)
	SYSTEM.MOFCR.BYTE = 0x00;		//	b7-b1(0000000b), MOFXIN:1
	SYSTEM.HOCOPCR.BYTE = 0x01;		//	b7-b1(0000000b), HOCOPCNT:1(1=HOCO�̓d��OFF)
	/********************************************************************************/
	/*	PowerOnDefault�l���g�p ---> �ݒ�K�v���̈�Commentout ---> �d�l�m���폜	*/
	/********************************************************************************/

	//	�����ݒ芮����̓���ROM�L���g��Mode�̐ݒ菈��
	SYSTEM.SYSCR0.WORD = 0x5A03;	//	KEY:8, b7-b2, EXBE:1(1), ROME:1(1)
	for(;;){
		short_dummy = SYSTEM.SYSCR0.WORD & 0x00FF;
		if(0x0003 == short_dummy){
			break;
		}
	}

	/****************************************************************************/
	/*		BSC(BusControl)														*/
	/*			���	�@�\					���l							*/
	/*			---------------------------------------------					*/
	/*			CS0		����ROM					�����ݒ�̂܂�					*/
	/*			CS1		MainMemory1				SRAM_2MB(Backup�L)				*/
	/*			CS2		MainMemory2				SRAM_1MB(Backup�L)				*/
	/*			CS3		EthernetControler		???								*/
	/*			CS4		�g��I/O					����:16Port�C�o��:16Port		*/
	/*			CS5�`CS7�͖��g�p				�����ݒ�̂܂�					*/
	/****************************************************************************/
	/************************/
	/*	CS1					*/
	/************************/
	BSC.CS1REC.WORD = 0x0101;		//	b15-b12, WRCV:4(0001=WriteRecovery1Cycle), b7-b4, RRCV:4(0001=ReadRecovery1Cycle)
	BSC.CS1MOD.WORD = 0x0301;		//	PRMOD:1(0=NormalAccess�݊�Mode), b14-b10, PWENB:1(1=PageWriteAccess����), PRENB:1(1=PageReadAccess����)
									//	b7-b4, EWENB:1(0=�O��Wait�֎~), b2-b1, WRMOD:1(1=1WriteStrobeMode))
// MH322915(S) K.Akiba 2017/05/22 ���No.5176 SRAM��ւɔ����o�X�E�F�C�g�̏C��
//	BSC.CS1WCR1.LONG = 0x04040202;	//	b31-b29, CSRWAIT:5(00100=NormalReadCycleWait4Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
	BSC.CS1WCR1.LONG = 0x03030303;	//	b31-b29, CSRWAIT:5(00100=NormalReadCycleWait4Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
// MH322915(E) K.Akiba 2017/05/22 ���No.5176 SRAM��ւɔ����o�X�E�F�C�g�̏C��
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
// MH322915(S) K.Akiba 2017/05/22 ���No.5176 SRAM��ւɔ����o�X�E�F�C�g�̏C��
//	BSC.CS2WCR1.LONG = 0x04040202;	//	b31-b29, CSRWAIT:5(00100=NormalReadCycleWait4Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
	BSC.CS2WCR1.LONG = 0x03030303;	//	b31-b29, CSRWAIT:5(00100=NormalReadCycleWait4Cycle), b23-b21, CSWWAIT:5(00100=NormalWriteCycleWait4Cycle)
// MH322915(E) K.Akiba 2017/05/22 ���No.5176 SRAM��ւɔ����o�X�E�F�C�g�̏C��
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
	/****************************************************************************/
	/*		WDTI/IWDTI															*/
	/*			ICU.NMIER��1�񂵂�Write�ł��Ȃ��̂�ICU�̑O�ɐݒ肷��			*/
	/*			---> 	dog_init()��INITAL()����call�����̂�					*/
	/*					�����ł͏���������dog_init()�ōs��						*/
	/****************************************************************************/

	/****************************************************************************/
	/*		ICU(InterruptControl)												*/
	/****************************************************************************/
	/********************************************************/
	/*	�����ݗv�� <- 0(�����Ȃ�)							*/
	/********************************************************/
	for( cnt=0; cnt<254; cnt++ ){	// InterruptTable��
		ICU.IR[cnt].BYTE = 0;		// ICU.IR[cnt].BIT.IR
	}
	/********************************************************/
	/*	�����ݗv��Priority = Level�ݒ�(Level0=�����݋֎~)	*/
	/*		IPR:4(Level0 - 15)								*/
	/********************************************************/
	//	Level5����ɐݒ肷��
	ICU.IPR[ 68].BYTE = 0x0A;	//	ICU_IRQ4	= #RTC_IRQ/RTC���������M��(1min�Ԋu)	NegativeEdge	Pin43
	ICU.IPR[ 69].BYTE = 0x05;	//	ICU_IRQ5	= #EX_IRQ1/UARTcontroler1������			NegativeEdge	Pin42
	ICU.IPR[ 72].BYTE = 0x05;	//	ICU_IRQ8	= #EX_IRQ2/UARTcontroler2������			NegativeEdge	Pin141
	ICU.IPR[ 79].BYTE = 0x06;	//	ICU_IRQ15	= #ETHER_IRQ/Ethernetcontroler������	NegativeEdge	Pin98
	ICU.IPR[VECT_ICU_GROUPL0].BYTE = 0x07;		//	���荞�ݗv���O���[�v12
	ICU.IPR[220].BYTE = 0x07;	//	SCI2_RXI2	= JVMA(���ү�,����ذ��):��M			4,800bps
	ICU.IPR[221].BYTE = 0x07;	//	SCI2_TXI2	= JVMA(���ү�,����ذ��):���M			4,800bps
	ICU.IPR[222].BYTE = 0x07;	//	SCI2_TEI2	= JVMA(���ү�,����ذ��):Error			4,800bps
	ICU.IPR[223].BYTE = 0x07;	//	SCI3_RXI3	= ���CReader����:��M					38,400bps
	ICU.IPR[224].BYTE = 0x07;	//	SCI3_TXI3	= ���CReader����:���M					38,400bps
	ICU.IPR[225].BYTE = 0x07;	//	SCI3_TEI3	= ���CReader����:Error					38,400bps
	ICU.IPR[226].BYTE = 0x07;	//	SCI4_RXI4	= Flap����:��M							38,400bps
	ICU.IPR[227].BYTE = 0x07;	//	SCI4_TXI4	= Flap����:���M							38,400bps
	ICU.IPR[228].BYTE = 0x07;	//	SCI4_TEI4	= Flap����:Error						38,400bps
	ICU.IPR[232].BYTE = 0x07;	//	SCI6_RXI6	= Bluetooth:��M						115,200bps
	ICU.IPR[233].BYTE = 0x07;	//	SCI6_TXI6	= Bluetooth:���M						115,200bps
	ICU.IPR[234].BYTE = 0x07;	//	SCI6_TEI6	= Bluetooth:Error						115,200bps
	ICU.IPR[235].BYTE = 0x08;	//	SCI7_RXI7	= ���uNT-NET�pFOMA:��M					38.400bps
	ICU.IPR[236].BYTE = 0x08;	//	SCI7_TXI7	= ���uNT-NET�pFOMA:���M					38.400bps
	ICU.IPR[237].BYTE = 0x08;	//	SCI7_TEI7	= ���uNT-NET�pFOMA:Error				38.400bps
	ICU.IPR[241].BYTE = 0x07;	//	SCI9_RXI9	= NT-NET�ʐM:��M						38,400bps
	ICU.IPR[242].BYTE = 0x07;	//	SCI9_TXI9	= NT-NET�ʐM:���M						38,400bps
	ICU.IPR[243].BYTE = 0x07;	//	SCI9_TEI9	= NT-NET�ʐM:Error						38,400bps
	ICU.IPR[244].BYTE = 0x07;	//	SCI10_RXI10	= ��ڐGIC:��M							38,400bps
	ICU.IPR[245].BYTE = 0x07;	//	SCI10_TXI10	= ��ڐGIC:���M							38,400bps
	ICU.IPR[246].BYTE = 0x07;	//	SCI10_TEI10	= ��ڐGIC:Error						38,400bps
	ICU.IPR[247].BYTE = 0x07;	//	SCI11_RXI11	= MultiICcardR/W:��M					38,400bps
	ICU.IPR[248].BYTE = 0x07;	//	SCI11_TXI11	= MultiICcardR/W:���M					38,400bps
	ICU.IPR[249].BYTE = 0x07;	//	SCI11_TEI11	= MultiICcardR/W:Error					38,400bps
	ICU.IPR[126].BYTE = 0x05;	//	TPU0_TGI0A	= 20msTimer������
	ICU.IPR[130].BYTE = 0x05;	//	TPU1_TGI1A	=  2msTimer������
	ICU.IPR[132].BYTE = 0x05;	//	TPU2_TGI2A	=  1msTimer������
	ICU.IPR[134].BYTE = 0x05;	//	TPU3_TGI3A	= 20msTimer������(TPU0��Sodiac�Ŏg�p����̂ŁA�^�C�}�[��ǉ�)
	/****************************/
// Sc16RecvInSending_IRQ5_Level_L
	ICU.IPR[186].BYTE = 0x0B;	//	RIIC1_EEI1	= RS232C�ʐMDevice����(IC1,2,3):Error	400kbps
//	ICU.IPR[186].BYTE = 0x0C;	//	RIIC1_EEI1	= RS232C�ʐMDevice����(IC1,2,3):Error	400kbps
	ICU.IPR[187].BYTE = 0x0B;	//	RIIC1_RXI1	= RS232C�ʐMDevice����(IC1,2,3):��M	400kbps
	ICU.IPR[188].BYTE = 0x0B;	//	RIIC1_TXI1	= RS232C�ʐMDevice����(IC1,2,3):���M	400kbps
	ICU.IPR[189].BYTE = 0x0B;	//	RIIC1_TEI1	= RS232C�ʐMDevice����(IC1,2,3):Error	400kbps
	ICU.IPR[VECT_RSPI0_SPRI0].BYTE = 0x0E;	//	RSPI0_SPRI0	= SPI ��M���荞��(FROM)
// NOTE:���Ԃ̐��x���m�ۂ��邱�ƁA�V���A�����荞�݂���p�x�A�������Ԃ��ɒ[�ɒZ���̂ŗD�揇�ʂ͍��߂ɐݒ肵�Ă���܂�
	ICU.IPR[  4].BYTE = 0x08;	//	CMTU0_CMT0	= 800us 1shot�^�C�} JVMA SYN�M������p
	/********************************************************/
	/*	IRQControlRegister(������Edge�w��:b7-b3,IRQMD:2, b0)*/
	/*		00/01/10/11 = L/Neg/Pos/Neg+Pos					*/
	/********************************************************/
	ICU.IRQCR[ 4].BYTE = 0x04;	//	IRQ4 = #RTC_IRQ/RTC���������M��(1min�Ԋu)	Pin43
	ICU.IRQCR[ 5].BYTE = 0x04;	//	IRQ5 = #EX_IRQ1/UARTcontroler1������		Pin42
	ICU.IRQCR[ 8].BYTE = 0x04;	//	IRQ8 = #EX_IRQ2/UARTcontroler2������		Pin141
//	EtherControlLSI����̑��M����/Packet��M�����݂�NegativeEdge�Ŕ������邪
//	RX630-H/W�Ō��m�ł��Ȃ���������(�����ݽ�����Q��)�̂�LowLevel�̌��m�ɕύX����B
//	�A��IRQ15��Line��L�̂܂܂�PowerOn�����H�ɂȂ�܂ŉi�v��IRQ15����������
//	System�������オ��Ȃ��̂�kasago����Open�����O�ɔ�������IRQ15��
//	LSI����SPacket��Read����IRQ15��Line��H�ɂ���B
	ICU.IRQCR[15].BYTE = 0x00;	//	IRQ15= #ETHER_IRQ/Ethernetcontroler������	Pin98
	/************************************************/
	/*	NMI�[�q������ControlRegister) = 0/1=Neg/Pos	*/
	/************************************************/
	ICU.NMICR.BYTE = 0x00;		//	NMI = b7-b3,NMIMD:1, b1-b0
	/********************************************************/
	/*	ICU.NMISR�̓��YBit��Clear����(Clear = write1)		*/
	/*		LVD2ST, LVD1ST1, IWDTST, WDTST, OSTST, NMIST	*/
	/********************************************************/
	ICU.NMICLR.BYTE = 0xFF;		//	b7-b6, LVD2CLR:1, LVD1CLR:1, IWDTCLR:1, WDTCLR:1, OSTCLR:1, NMICLR:1(NonMaskable������ClearRegister)

	/****************************************************/
	/*	NonMaskable�����݋���Register(0/1 = DI/EI)		*/
	/****************************************************/
	//	1�񂵂�Write�ł��Ȃ��̂łƂ肠����, WDTEN, NMIEN <- EI����
	ICU.NMIER.BYTE = 0x05;		//	b7-b6, LVD2EN:1, LVD1EN:1, IWDTEN:1, WDTEN:1, OSTEN:1, NMIEN:1(NonMaskable�����݋���Register)
	/****************************************************/
	/*	�����݋���Register(0/1 = DI/EI)					*/
	/****************************************************/
	/********************************/
	/*	ICU_IRQ4	= IER08.IEN4	*/
	/*	ICU_IRQ5	= IER08.IEN5	*/
	/********************************/
	ICU.IER[8].BYTE = 0x30;
	/********************************/
	/*	ICU_IRQ8	= IER09.IEN0	*/
	/*	ICU_IRQ15	= IER09.IEN7	*/
	/********************************/
	ICU.IER[9].BYTE = 0x81;
	/********************************/
	/*	ICU_GROUPL0	= IER0E.IEN2	*/
	/********************************/
	ICU.IER[IER_ICU_GROUPL0].BYTE = 0x04;	// �O���[�v12���荞�݋���(SCI�̃G���[���荞��)
	/********************************/
	/*	SCI2_RXI2	= IER1B.IEN4	*/
	/*	SCI2_TXI2	= IER1B.IEN5	*/
	/*	SCI2_TEI2	= IER1B.IEN6	*/
	/*	SCI3_RXI3	= IER1B.IEN7	*/
	/********************************/
	ICU.IER[27].BYTE = 0xF0;
	/********************************/
	/*	SCI3_TXI3	= IER1C.IEN0	*/
	/*	SCI3_TEI3	= IER1C.IEN1	*/
	/*	SCI4_RXI4	= IER1C.IEN2	*/
	/*	SCI4_TXI4	= IER1C.IEN3	*/
	/*	SCI4_TEI4	= IER1C.IEN4	*/
	/********************************/
	ICU.IER[28].BYTE = 0x1F;
	/********************************/
	/*	SCI6_RXI6	= IER1D.IEN0	*/
	/*	SCI6_TXI6	= IER1D.IEN1	*/
	/*	SCI6_TEI6	= IER1D.IEN2	*/
	/*	SCI7_RXI7	= IER1D.IEN3	*/
	/*	SCI7_TXI7	= IER1D.IEN4	*/
	/*	SCI7_TEI7	= IER1D.IEN5	*/
	/********************************/
	ICU.IER[29].BYTE = 0x3F;
	/********************************/
	/*	SCI9_RXI9	= IER1E.IEN1	*/
	/*	SCI9_TXI9	= IER1E.IEN2	*/
	/*	SCI9_TEI9	= IER1E.IEN3	*/
	/*	SCI10_RXI10	= IER1E.IEN4	*/
	/*	SCI10_TXI10	= IER1E.IEN5	*/
	/*	SCI10_TEI10	= IER1E.IEN6	*/
	/*	SCI11_RXI11	= IER1E.IEN7	*/
	/********************************/
	ICU.IER[30].BYTE = 0xFE;
	/********************************/
	/*	SCI11_TXI11	= IER1F.IEN0	*/
	/*	SCI11_TEI11	= IER1F.IEN1	*/
	/********************************/
	ICU.IER[31].BYTE = 0x03;
	/********************************/
	/*	TPU0_TGI0A	= IER0F.IEN6	*/
	/********************************/
	ICU.IER[15].BYTE = 0x40;
	/********************************/
	/*	TPU1_TGI1A	= IER10.IEN2	*/
	/*	TPU2_TGI2A	= IER10.IEN4	*/
	/********************************/
	/*	TPU3_TGI3A	= IER10.IEN6	*/
	ICU.IER[16].BYTE = 0x54;
	/********************************/
	/*	TPU3_TGI3A	= IER10.IEN6	*/
	/********************************/

	/********************************/
	/*	CAN0_RXF0	= IER06.IEN0	*/
	/*	CAN0_TXF0	= IER06.IEN1	*/
	/*	CAN0_RXM0	= IER06.IEN2	*/
	/*	CAN0_TXM0	= IER06.IEN3	*/
	/********************************/
	ICU.IER[6].BYTE = 0x0F;
	/********************************/
	/*	RIIC1_EEI1	= IER17.IEN2	*/
	/*	RIIC1_RXI1	= IER17.IEN3	*/
	/*	RIIC1_TXI1	= IER17.IEN4	*/
	/*	RIIC1_TEI1	= IER17.IEN5	*/
	/********************************/
	ICU.IER[23].BYTE = 0x3C;
	// �O���[�v���荞�݋���(SCI�̃G���[���荞�݂̋���)
	ICU.GEN[GEN_SCI2_ERI2].BIT.EN2 = 1;		// ERI2 �iSCI2��M�G���[�j ���荞�݋���
	ICU.GEN[GEN_SCI3_ERI3].BIT.EN3 = 1;		// ERI3 �iSCI3��M�G���[�j ���荞�݋���
	ICU.GEN[GEN_SCI4_ERI4].BIT.EN4 = 1;		// ERI4 �iSCI4��M�G���[�j ���荞�݋���
	ICU.GEN[GEN_SCI6_ERI6].BIT.EN6 = 1;		// ERI6 �iSCI6��M�G���[�j ���荞�݋���
	ICU.GEN[GEN_SCI7_ERI7].BIT.EN7 = 1;		// ERI7 �iSCI7��M�G���[�j ���荞�݋���
	ICU.GEN[GEN_SCI9_ERI9].BIT.EN9 = 1;		// ERI9 �iSCI9��M�G���[�j ���荞�݋���
	ICU.GEN[GEN_SCI10_ERI10].BIT.EN10 = 1;	// ERI10�iSCI10��M�G���[�j���荞�݋���
	ICU.GEN[GEN_SCI11_ERI11].BIT.EN11 = 1;	// ERI11�iSCI11��M�G���[�j���荞�݋���
	/********************************/
	/*	RSPI0_SPRI0	= IER04.IEN7	*/
	/********************************/
	ICU.IER[4].BYTE = 0x80;
	/********************************/
	/*	CMTU0_CMT0	= IER03.IEN4	*/
	/********************************/
	ICU.IER[3].BYTE = 0x10;
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
	PORT3.PODR.BYTE = 0x00;		//	B7, B6, B5, B4, B3, B2, B1, B0
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
	/****************************************************************************************************************/
	/*	�g���|�[�g(CS4���)																							*/
	/*	CS4 = 04000000 - 04FFFFFF(16M)���																			*/
	/*	���o��	Bits	�M����(Init�l)	Data(H)		L			Block			���e								*/
	/*	--------------------------------------------------------------------------------------------				*/
	/*	�o��	b15		LOCK(L)			Lock		Open		�O��I/O			RXI-1��d�����b�N����				*/
	/*			b14		FULL1(L)		����		���		�O��I/O			RXI-1����o��1						*/
	/*			b13		FULL2(L)		����		���		�O��I/O			RXI-1����o��2						*/
	/*			b12		#DORE(L)		����		�x��o��	�O��I/O			RXI-1�h�A�x��o��(CPU�N�����������)*/
	/*			b11		RXI_OUT4(L)		ON			OFF			�O��I/O			RXI-1�ėp�o�̓|�[�g4				*/
	/*			b10		RXI_OUT5(L)		ON			OFF			�O��I/O			RXI-1�ėp�o�̓|�[�g5				*/
	/*			b9		RXI_OUT5(L)		ON			OFF			�O��I/O			RXI-1�ėp�o�̓|�[�g5				*/
	/*			b8		RXI_OUT7(L)		ON			OFF			�O��I/O			RXI-1�ėp�o�̓|�[�g7				*/
	/*			b7		M_OUT(L)		ON			OFF			�O��I/O			RXM-1��\���o�̓|�[�g				*/
	/*			b6		#URES(L)		�ʏ�		reset�v��	CPU				CPUmoniter�pLED						*/
	/*			b5		CF_RES(L)		reset�v��	�ʏ�		CredgitFOMA		�N���W�b�gFOMAreset�M��				*/
	/*			b4		JP_RES(L)		reset�v��	�ʏ�		Jarnal			JarnalPrinterreset�M��				*/
	/*			b3		RW_RES(L)		reset�v��	�ʏ�		��ڐGIC		��ڐGICReaderreset�M��				*/
	/*			b2		M_LD2(L)		�_��		����		CPU				StatusLED2(������)					*/
	/*			b1		M_LD1(L)		�_��		����		CPU				StatusLED1							*/
	/*			b0		M_LD0(L)		�_��		����		CPU				StatusLED0							*/
	/*	����	b15		#RXI_IN			OFF			ON			�O��I/O			RXI-1��ėp���̓|�[�g				*/
	/*			b14		#M_IN			OFF			ON			�O��I/O			RXM-1��\�����̓|�[�g				*/
	/*			b13		#M_KEY			�J			��			�e��Sensor�Ď�	�h�A�m�u�L�[��ԐM��				*/
	/*			b12		#C_KEY			�J			��			�e��Sensor�Ď�	�ݒ�L�[��ԐM��(�\��)				*/
	/*			b11		#TH_ON			-			FAN�쓮		FAN����			FAN�쓮�J�n���x���o�M��				*/
	/*			b10		TH_OFF			FAN��~		-			FAN����			FAN��~���x���o�M��					*/
	/*			b9		���g�p																						*/
	/*			b8		���g�p																						*/
	/*			b7		#DPSW3			OFF			ON			CPU				�ݒ�SW_�\��							*/
	/*			b6		#DPSW2			OFF			ON			CPU				�ݒ�SW_�\��							*/
	/*			b5		#DPSW1			OFF			ON			CPU				�ݒ�SW_�\��							*/
	/*			b4		#DPSW0			OFF(�I�[�L)	ON(�I�[��)	CPU				�ݒ�SW_CAN�I�[�ݒ�					*/
	/*			b3		#RTSW3			OFF			ON			CPU				Mode�ݒ�SW							*/
	/*			b2		#RTSW2			OFF			ON			CPU				Mode�ݒ�SW							*/
	/*			b1		#RTSW1			OFF			ON			CPU				Mode�ݒ�SW							*/
	/*			b0		#RTSW0			OFF			ON			CPU				Mode�ݒ�SW							*/
	/****************************************************************************************************************/
}
void C_INITSCT(void)
{
	_INITSCT();		// �]����initial.src�ɔz�u���Ă�����RX�V���[�Y�ł̓r���h���ʂ�Ȃ��̂Ŕz�u���������{����
}
/****************************************************************************/
/*																			*/
/*	I/O Port NMI init														*/
/*		call�� = NMIINT, System_reset()										*/
/****************************************************************************/
void	port_nmi( void )
{

// MH322915(S) K.Onodera 2017/05/19 �d�q�}�l�[���Z�b�g�Ή�
	CP_CN_SYN = 0;									// /* SYM Enable	*/
// MH322915(E) K.Onodera 2017/05/19 �d�q�}�l�[���Z�b�g�Ή�
	
// SPI���ʐM���̉\��������̂�SPCR.BIT.SPE�����Z�b�g���ē����V�[�P���X������������
	RSPI0.SPCR.BYTE = 0x08;		// SPI���Z�b�g
	nop();
	nop();
	nop();
	nop();
	nop();
	RSPI0.SPCR.BYTE = 0x48;		// SPE�Z�b�g
// FROM�A�N�Z�X���ł��邱�Ƃ��l�����ă��Z�b�g�V�[�P���X�����s���ċ��������~�Ƃ���
// ���Z�b�g�V�[�P���X�̎��s���Ԃ͌v����2.2uS
	SetRsten();
	SetRst();

	_di();
	// �����l��L���Ƃ���
	CP_OUTREG_EN = 0;	// �o�̓|�[�g�C�l�[�u���M�� �L��

//	�g�pPort��S��in�ݒ肷��
	PORT0.PDR.BYTE  = 0x00;
	PORT1.PDR.BYTE  = 0x00;
	PORT2.PDR.BYTE  = 0x00;
	PORT3.PDR.BYTE  = 0x00;
	PORT4.PDR.BYTE  = 0x00;
	PORT5.PDR.BYTE  = 0x00;
// GG129004(S) M.Fujikawa 2024/11/08 ���u�[�g���̌x��Z�k
//	PORT6.PDR.BYTE  = 0x00;
	PORT6.PDR.BYTE  = 0x20;
// GG129004(E) M.Fujikawa 2024/11/08 ���u�[�g���̌x��Z�k
	PORT7.PDR.BYTE  = 0x00;
// MH322915(S) K.Onodera 2017/05/19 �d�q�}�l�[���Z�b�g�Ή�
//	PORT8.PDR.BYTE  = 0x00;
	PORT8.PDR.BYTE  = 0x40;		// �d�q�}�l�[���Z�b�g�̂���PORT8.PDR.BIT.B6�͏o�͂̂܂܂ɂ���
// MH322915(E) K.Onodera 2017/05/19 �d�q�}�l�[���Z�b�g�Ή�
	PORT9.PDR.BYTE  = 0x00;
	PORTA.PDR.BYTE  = 0x00;
	PORTB.PDR.BYTE  = 0x00;
	PORTC.PDR.BYTE  = 0x00;
	PORTD.PDR.BYTE  = 0x00;
	PORTE.PDR.BYTE  = 0x00;
	PORTF.PDR.BYTE  = 0x00;
	PORTJ.PDR.BYTE  = 0x00;
	PORTK.PDR.BYTE  = 0x00;
	PORTL.PDR.BYTE  = 0x00;
}
/****************************************************************************************/
/*																						*/
/*	WATCHDOG timer start																*/
/*		call�� = INITAL, NMIINT, System_reset(), Apri									*/
/*		WatchDogTimer = 1300ms															*/
/*		OFS0	(Option�@�\�I��Register0)(7.2.1) = ReadOnlyROM(Write with ProgramCode)	*/
/*				BlankROM = 0xFFFFFFFF(InitialValue)										*/
/****************************************************************************************/
void dog_init( void )
{
	// Timeout���ԑI�� :8192Cycle
	// �����I��: Clock50MHz/8192
	// 8192*8192/50,000,000 = 1.342(s)	Timeout����
	// Window�I���ʒu�F�w��Ȃ�
	// Window�J�n�ʒu�F�w��Ȃ�
	WDT.WDTCR.WORD = 0x3382;// WDTCR�͋N����1�񂵂��������߂Ȃ��d�l�Ȃ���WORD�P�ʂŏ�������

	WDT.WDTRCR.BIT.RSTIRQS	= 1;	// ���Z�b�g�o��
//	WDT.WDTRCR.BIT.RSTIRQS	= 0;	// NMI�o��
	WDT.WDTRR	= 0x00;				//
	WDT.WDTRR	= 0xFF;				// refresh
}
/****************************************************************************/
/*																			*/
/*	WATCHDOG counter reset													*/
/*		call�� = �Ȃ�														*/
/*		RX630 = �폜���邪call���Ƃ̊֌W������̂łƂ肠����stab			*/
/****************************************************************************/
void dog_reset( void )
{
	//	Refresh = Write0x00 then Write0xFF(WDTRefreshRegister)
	_di();
	WDT.WDTRR	= 0x00;				//
	WDT.WDTRR	= 0xFF;				// refresh
	_ei();
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
	/********************************************************/
	/*	�����ݒ芮����̓���ROM�L���g��Mode�̐ݒ菈����		*/
	/*	CS�̐ݒ�̒��O(BSC�ݒ�)�ɍs��(�ǂ��̂��Ȃ�!?)		*/
	/*	SYSTEM.SYSCR0.WORD = 0x0003;						*/
	/********************************************************/
}
///****************************************************************************/
///*																			*/
///*	NMI���m����: NMI�׸ނ�Ă�RAM��ײ����øĂ�������						*/
///*		call�� = NMIINT														*/
///****************************************************************************/
void nmiint_set( void )
{
	FLAGPT.nmi_flg = 1;
}
/****************************************************************************/
/*																			*/
/*	���u(MainCPU)ؾ��														*/
/*		call�� = ����														*/
/****************************************************************************/
void	System_reset( void )
{
// MH810100(S) K.Onodera  2020/01/17 �Ԕԃ`�P�b�g���X(LCD���Z�b�g�ʒm�Ή�)
	SystemResetFlg = 1;					// �V�X�e�����Z�b�g���s�t���OON
// MH810100(E) K.Onodera  2020/01/17 �Ԕԃ`�P�b�g���X(LCD���Z�b�g�ʒm�Ή�)
	_di();								// �����݋֎~
	port_nmi();							// All port <- DI
	dog_init();							// watch dog timer start
	for( ;; );

}
/****************************************************************************/
/*																			*/
/*	TPU0  		20ms_Timer													*/
/*				INT:		Excep_TPU0_TGI0A								*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void	TPU0_init( void )
{
	volatile unsigned long	ist;			// ���݂̊�����t���
	volatile unsigned char	wkb;

	MSTP_TPU0 = 0;					// TPU0 module stop release(SYSTEM.MSTPCRA.BIT.MSTPA13)
	wait2us(5L);
	ist = _di2();
	TPUA.TSTR.BIT.CST0 = 0;			// ���Ē�~(b7, b6, CST5, CST4, CST3, CST2, CST1, CST0)
	TPUA.TSYR.BIT.SYNC0 = 0;		// �Ɨ�����(b7, b6, SYNC5, SYNC4, SYNC3, SYNC2, SYNC1, SYNC0)
	_ei2( ist );
	TPU0.TCR.BIT.CCLR = 1;			// TGRA compare match(CCLR[2:0], CKEG[1:0], TPSC[2:0])
	TPU0.TCR.BIT.CKEG = 0;			// up_edge(CCLR[2:0], CKEG[1:0], TPSC[2:0])
	TPU0.TCR.BIT.TPSC = 3;			// PCLK/64(CCLR[2:0], CKEG[1:0], TPSC[2:0])
	TPU0.TMDR.BIT.MD = 0;			// normal mode
	TPU0.TIORH.BIT.IOB = 0;			// TGRB output <- DI(IOB[3:0], IOA[3:0])
	TPU0.TIORH.BIT.IOA = 0;			// TGRA output <- DI(IOB[3:0], IOA[3:0])
	TPU0.TIORL.BIT.IOD = 0;			// TGRD output <- DI(IOD[3:0], IOC[3:0])
	TPU0.TIORL.BIT.IOC = 0;			// TGRC output <- DI(IOD[3:0], IOC[3:0])
	TPU0.TGRA = 15625;				// 1/50MHz*64*15625=20000us(:16bit)
	TPU0.TCNT = 0;					// up counter <- 0(16bit)
	ist = _di2();
	wkb = TPU0.TSR.BYTE;			// (TCFD, b6, TCFU, TCFV, TGFD, TGFC, TGFB, TGFA)
	TPU0.TSR.BYTE = 0;				// status <- 0(TCFD, b6, TCFU, TCFV, TGFD, TGFC, TGFB, TGFA)
	dummy_Read = TPU0.TSR.BYTE;		// Dummy Read
	TPUA.TSTR.BIT.CST0 = 1;			// ���ĊJ�n(b7, b6, CST5, CST4, CST3, CST2, CST1, CST0)
	_ei2( ist );
	TPU0.TIER.BIT.TGIEA = 1;		// TGIA������ <- EI(TTGE, b6, TCIEU, TCIEV, TGIED, TGIEC, TGIEB, TGIEA)
}
/****************************************************************************/
/*																			*/
/*	TPU1  		2ms_Timer													*/
/*				INT:		Excep_TPU1_TGI1A								*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void	TPU1_init( void )
{
	volatile unsigned long	ist;			// ���݂̊�����t���
	volatile unsigned char	wkb;


	MSTP_TPU1 = 0;					// TPU1 module stop release(SYSTEM.MSTPCRA.BIT.MSTPA13)
	wait2us(5L);
	ist = _di2();
	TPUA.TSTR.BIT.CST1 = 0;			// ���Ē�~
	TPUA.TSYR.BIT.SYNC1 = 0;		// �Ɨ�����
	_ei2( ist );
	TPU1.TCR.BIT.CCLR = 1;			// TGRA compare match
	TPU1.TCR.BIT.CKEG = 0;			// up_edge
	TPU1.TCR.BIT.TPSC = 3;			// PCLK/64
	TPU1.TMDR.BIT.MD = 0;			// normal mode
	TPU1.TIOR.BIT.IOB = 0;			// TGRB output <- DI
	TPU1.TIOR.BIT.IOA = 0;			// TGRA output <- DI
	TPU1.TGRA = 1562;				// 1/50MHz*64*1562=2000us(:16bit)
	TPU1.TCNT = 0;					// up counter <- 0
	ist = _di2();
	wkb = TPU1.TSR.BYTE;
	TPU1.TSR.BYTE = 0;				// all flag <- 0
	dummy_Read = TPU1.TSR.BYTE;		// Dummy Read
	TPUA.TSTR.BIT.CST1 = 1;			// ���ĊJ�n
	_ei2( ist );
	TPU1.TIER.BIT.TGIEA = 1;		// TGIA������ <- EI
}
/****************************************************************************/
/*																			*/
/*	TPU2  		1ms_Timer													*/
/*				INT:		Excep_TPU2_TG21A								*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void	TPU2_init( void )
{
	volatile unsigned long	ist;			// ���݂̊�����t���
	volatile unsigned char	wkb;

	MSTP_TPU2 = 0;					// TPU2 module stop release(SYSTEM.MSTPCRA.BIT.MSTPA13)
	wait2us(5L);
	ist = _di2();
	TPUA.TSTR.BIT.CST2 = 0;			// ���Ē�~
	TPUA.TSYR.BIT.SYNC2 = 0;		// �Ɨ�����
	_ei2( ist );
	TPU2.TCR.BIT.CCLR = 1;			// TGRA compare match
	TPU2.TCR.BIT.CKEG = 0;			// up_edge
	TPU2.TCR.BIT.TPSC = 2;			// PCLK/16(CCLR[2:0], CKEG[1:0], TPSC[2:0])
	TPU2.TMDR.BIT.MD = 0;			// normal mode
	TPU2.TIOR.BIT.IOB = 0;			// TGRB output <- DI
	TPU2.TIOR.BIT.IOA = 0;			// TGRA output <- DI
	TPU2.TGRA = 3125;				// 1/50MHz*16*3125=1000us(:16bit)
	TPU2.TCNT = 0;					// up counter <- 0
	ist = _di2();
	wkb = TPU2.TSR.BYTE;
	TPU2.TSR.BYTE = 0;				// all flag <- 0
	dummy_Read = TPU2.TSR.BYTE;		// Dummy Read
	TPUA.TSTR.BIT.CST2 = 1;			// ���ĊJ�n
	_ei2( ist );
	TPU2.TIER.BIT.TGIEA = 1;		// TGIA������ <- EI
}

/****************************************************************************/
/*																			*/
/*	TPU3  		20ms_Timer													*/
/*				INT:		Excep_TPU3_TGI0A								*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void	TPU3_init( void )
{
	volatile unsigned long	ist;			// ���݂̊�����t���
	volatile unsigned char	wkb;

	MSTP_TPU3 = 0;					// TPU3 module stop release(SYSTEM.MSTPCRA.BIT.MSTPA13)
	wait2us(5L);
	ist = _di2();
	TPUA.TSTR.BIT.CST3 = 0;			// ���Ē�~
	TPUA.TSYR.BIT.SYNC3 = 0;		// �Ɨ�����
	_ei2( ist );
	TPU3.TCR.BIT.CCLR = 1;			// TGRA compare match(CCLR[2:0], CKEG[1:0], TPSC[2:0])
	TPU3.TCR.BIT.CKEG = 0;			// up_edge(CCLR[2:0], CKEG[1:0], TPSC[2:0])
	TPU3.TCR.BIT.TPSC = 3;			// PCLK/64(CCLR[2:0], CKEG[1:0], TPSC[2:0])
	TPU3.TMDR.BIT.MD = 0;			// normal mode
	TPU3.TIORH.BIT.IOB = 0;			// TGRB output <- DI(IOB[3:0], IOA[3:0])
	TPU3.TIORH.BIT.IOA = 0;			// TGRA output <- DI(IOB[3:0], IOA[3:0])
	TPU3.TIORL.BIT.IOD = 0;			// TGRD output <- DI(IOD[3:0], IOC[3:0])
	TPU3.TIORL.BIT.IOC = 0;			// TGRC output <- DI(IOD[3:0], IOC[3:0])
	TPU3.TGRA = 15625;				// 1/50MHz*64*15625=20000us(:16bit)
	TPU3.TCNT = 0;					// up counter <- 0(16bit)
	ist = _di2();
	wkb = TPU3.TSR.BYTE;			// (TCFD, b6, TCFU, TCFV, TGFD, TGFC, TGFB, TGFA)
	TPU3.TSR.BYTE = 0;				// status <- 0(TCFD, b6, TCFU, TCFV, TGFD, TGFC, TGFB, TGFA)
	dummy_Read = TPU3.TSR.BYTE;		// Dummy Read
	TPUA.TSTR.BIT.CST3 = 1;			// ���ĊJ�n(b7, b6, CST5, CST4, CST3, CST2, CST1, CST0)
	_ei2( ist );
	TPU3.TIER.BIT.TGIEA = 1;		// TGIA������ <- EI(TTGE, b6, TCIEU, TCIEV, TGIED, TGIEC, TGIEB, TGIEA)
	Tim500msCount = 0;				// 500ms�ώZ���� <- 0
	TIM500_START = 0;				// 500ms interval�o���׸� <- 0
	Tim1sCount = 0;					// 1s�ώZ�J�E���^�N���A
}

/****************************************************************************/
/*																			*/
/*	CAN0		�@����ʐM													*/
/*				Baudrate: 	1Mbps											*/
/*				INT:		Excep_CAN0_RXF0									*/
/*                          Excep_CAN0_TXF0									*/
/*                          Excep_CAN0_RXM0									*/
/*                          Excep_CAN0_TXM0									*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void CAN1_init( void )
{
	caninit();
}
/****************************************************************************/
/*	I2C1																	*/
/*	RIIC1	RS232C�ʐMDevice����	400kbps		EEI1, RXI1, TXI1, TEI1		*/
/*			Baudrate: 	400Kbps												*/
/*			INT:		Excep_RIIC1_EEI1	TMOF, AL, SP, Nack, ST			*/
/*            	        Excep_RIIC1_RXI1	��M							*/
/*            	        Excep_RIIC1_TXI1	���M							*/
/*            	        Excep_RIIC1_TEI1	Transmitt End					*/
/*			PCLK = 50MHz(20nsec/1clock)										*/
/*	(type1=i2c0:���g�p, type2=i2c1,2,3)										*/
/*	�ڑ�IC																	*/
/*		Ref.	�i��			�^��			W_ADDR	R_ADDR	���x		*/
/*		IC2		RTCMojule		RX-8803LC(UB)	0x62	0x63	400kbps		*/
/*		IC3		UARTControler	SC16IS752IPW	0x90	0x91	400kbps		*/
/*		IC4		UARTControler	SC16IS752IPW	0x92	0x93	400kbps		*/
/*	�]�����x	������g�� = PCLK											*/
/*				30MHz				33MHz				50MHz				*/
/*				*1		*2	*3		*1		*2	*3		*1		*2	*3		*/
/*	----------------------------------------------------------------------	*/
/*	400kbps		001b	8	19		001b	9	21		010b	7	16		*/
/*	----------------------------------------------------------------------	*/
/*	*1:	ICMR1-CKS[2:0]														*/
/*	*2:	ICBRL-BRL[4:0]														*/
/*	*3:	ICBRH-BRH[4:0]														*/
/*	RX630�g�p����ݒ�l														*/
/*		�]�����x=400kbps, ������g��PCLK=50MHz	------->	010b, 7, 16		*/
/****************************************************************************/
/*	SC16IS752IPW															*/
/*		�����U���q			14.7456MH��										*/
/*		MCR_Register_bit7	0 												*/
/****************************************************************************/
void I2C1_init( void )
{
	int result;
	int loopcnt;

	I2cAM.I2c_ProcStatus = 0;		// 0229
	/********************************************/
	/*	When clear ICE and set IICRST, 			*/
	/*	all RIIC registers are reset.			*/
	/********************************************/
	MSTP_RIIC1 = 0;					// �������ǉ�����K�v������B
	RIIC1.ICCR1.BIT.ICE 	= 0;	// RIIC disable for RIIC initial
	RIIC1.ICCR1.BIT.IICRST 	= 1;	// RIIC all reset
	RIIC1.ICCR1.BIT.IICRST 	= 0;	// Clear reset
	/********************************************************/
	/*	Set baudrate is 1Mbps (when input = 12.5MHz).		*/
	/*	IIC PCLK = 50MHz (1bit = 20ns).						*/
	/*	Baudrate = 400Kbps(2.5us/1bit)						*/
	/*	Section 32.2.13										*/
	/********************************************************/
	// Baudrate = 400Kbps(2.5us/1bit)
	RIIC1.ICMR1.BIT.CKS = 2;		// IIC phi = PCLK clock
	RIIC1.ICBRL.BYTE = 0xF0;
	RIIC1.ICBRH.BYTE = 0xE7;
	/********************************/
	/*	Time out setting			*/
	/********************************/
	RIIC1.ICMR2.BIT.TMOL = 0;		// Disable SCL=Low time out
	RIIC1.ICMR2.BIT.TMOH = 1;		// Enable SCL=High time out
	RIIC1.ICMR2.BIT.TMOS = 0;		// Long mode (16bits counter)
									// PCLK=48MHz, IIC phi=48MHz,
									// Long mode is about 1.365 ms
	RIIC1.ICFER.BIT.TMOE = 1;		// Enable time out detection
	/************************************/
	/*	Disable all address detection	*/
	/************************************/
	RIIC1.ICSER.BYTE = 0x00;
	/********************************/
	/*	Disable protect for ACKBT	*/
	/********************************/
	RIIC1.ICMR3.BIT.ACKWP = 1;
	/************************************************************/
	/*	Enable/Disable each interrupt							*/
	/*		b0:1 Time Out interrupt <- DI 						*/
	/*		b1:1 Arbitration Lost interrupt <- EI 				*/
	/*		b2:0 Start Condition Detection Interrupt <- DI		*/
	/*		b3:1 Stop condition detection interrupt <- EI 		*/
	/*		b4:1 NACK reception interrupt <- EI 				*/
	/*		b5:1 Receive Data Full Interrupt <- EI	 			*/
	/*		b6:0 Transmit End Interrupt <- DI	 				*/
	/*		b7:1 Transmit Data Empty Interrupt <- EI			*/
	/************************************************************/
	RIIC1.ICIER.BYTE = 0xBB;
	/********************************/
	/*	Initialize ram for RIIC		*/
	/********************************/
	//	I2C_MESSAGE 	iic_ActiveBuff;
	iic_mode 	= IIC_MODE_I2C_IDLE;
	iic_status 	= RIIC_STATUS_IDLE;
	iic_trm_cnt = 0;
	iic_rcv_cnt = 0;
	/********************/
	/*	Enable RIIC		*/
	/********************/
	RIIC1.ICCR1.BIT.ICE = 1;
	/********************************************************/
	/*	RTC Initialize										*/
	/********************************************************/
	RTC_init();
	/********************************************************/
	/*	I2C_Baudrate: 	400Kbps(2.5us/1bit)					*/
	/*	SC16��PowerOn����Initial�l��Read����				*/
	/*	RX630��I2C�ݒ�͊��ɍs���Ă���						*/
	/********************************************************/
	SC16_DataInit();
	/********************************************************/
	/*	�����U���q		14.7456MH��							*/
	/*	MCR_bit7		0									*/
	/*	BaudRate	N(�v�Z�l)	N(�ݒ�l)	�덷��(%)		*/
	/*	19200		48.000 		48 			0.00		o	*/
	/*	I2C_Baudrate: 	400Kbps(2.5us/1bit)					*/
	/********************************************************/
	/*	Don't change below CallingSequence					*/
	/*	setting ActiveDevice for Initialize					*/
	/********************************************************/
	I2cAM.ActiveDevice = I2C_DN_NON;
	result = SC16_RP_init();
	if (result != 0) 			{	return;		}
	result = SC16_JP_init();
	if (result != 0) 			{	return;		}

	I2cAM.I2c_ProcStatus = 1;		// 0229
	/********************************************************/
	/*	I2c_Arbitration Manager Class Initialize			*/
	/********************************************************/
	I2cAM_AllClear();
	I2C_BusError_Info.USHORT = 0;
	I2cErrEvent.SendRP = 0;
	I2cErrEvent.RecvRP = 0;
	I2cErrEvent.SendJP = 0;
	I2cErrEvent.RecvJP = 0;
	I2cErrStatus.SendRP = 0;
	I2cErrStatus.RecvRP = 0;
	I2cErrStatus.SendJP = 0;
	I2cErrStatus.RecvJP = 0;
	/************************************************/
	IicMilestone.Wcnt = 0;							// = DebugInfo
	IicMilestone.Rcnt = 0;							// = DebugInfo
	IicMilestone.I5cnt = 0;							// = DebugInfo
	IicMilestone.RTCcnt = 0;						// = DebugInfo
	IicMilestone.ForceReqStatus = 0;				// 0x00 = �������M�v���Ȃ�
	IicMilestone.I5_RP_THRcnt = 0;					// = DebugInfo
	IicMilestone.I5_JP_THRcnt = 0;					// = DebugInfo
	IicMilestone.I5_RP_RHRcnt = 0;					// = DebugInfo
	IicMilestone.I5_JP_RHRcnt = 0;					// = DebugInfo
	for( loopcnt=0; loopcnt<16; loopcnt++ ){
		IicMilestone.I5_RP_RHRbuf[loopcnt] = 0;		// = DebugInfo
		IicMilestone.I5_JP_RHRbuf[loopcnt] = 0;		// = DebugInfo
	}
	IicMilestone.I8cnt = 0;							// = DebugInfo
	IicMilestone.I8_I2CDEV2_1_THRcnt = 0;				// = DebugInfo
	IicMilestone.I8_I2CDEV2_1_RHRcnt = 0;				// = DebugInfo
	IicMilestone.RP_THR_HistCnt = 0;				// = DebugInfo
	for( loopcnt=0; loopcnt<256; loopcnt++ ){
		IicMilestone.RP_THR_History[loopcnt] = 0;	// = DebugInfo
	}
	IicMilestone.JP_THR_HistCnt = 0;				// = DebugInfo
	for( loopcnt=0; loopcnt<256; loopcnt++ ){
		IicMilestone.JP_THR_History[loopcnt] = 0;	// = DebugInfo
	}
	IicMilestone.I2cSP_Sending = 0;
// Sc16RecvInSending_IRQ5_Level_L start
	I2cSendDataCnt = 0;
	I2cSendSlaveAddr = 0;
	I2cSendLsiAddr = 0;
	for( loopcnt=0; loopcnt<256; loopcnt++ ){
		I2cSendDataBuff[loopcnt] = 0;
	}
// Sc16RecvInSending_IRQ5_Level_L end
}
/****************************************************************************/
/*																			*/
/*	SCI2		JVMA(���ү�,����ذ��)										*/
/*				Baudrate: 	4,800bps										*/
/*				INT:		Excep_SCI2_RXI2									*/
/*							Excep_SCI2_TXI2									*/
/*							Excep_SCI2_TEI2									*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void Sci2Init( void )
{
	volatile unsigned char	i;
	volatile unsigned long	ist;			// ���݂̊�����t���

	ist = _di2();
	MSTP_SCI2 = 0;					// SCI2 module stop mode release(SYSTEM.MSTPCRB.BIT.MSTPB29)
	SCI2.SEMR.BIT.ABCS = 0x01;		/* ����������{�N���b�N�Z���N�gBIT	*/
	SCI2.SCR.BYTE = 0x00;			// SCI2 <- 0(�����݋֎~)(TIE RIE TE RE MPIE TEIE CKE[1:0])
	_ei2( ist );
	SCI2.SMR.BIT.CM = 0;			// ��������Mode�œ���(CM CHR PE PM STOP MP CKS[1:0])
	SCI2.SMR.BIT.MP = 0;			// Multi pro func : 0=Nouse, 1=Use
	SCI2.SMR.BIT.CHR = 0;			// Data bits  : 0=8bit, 1=7bit
	SCI2.SMR.BIT.PE = 1;			// Parity�L�� : 0=None, 1=Use
	SCI2.SMR.BIT.PM = 0;			// ParityType : 0=Even, 1=Odd
	SCI2.SMR.BIT.STOP = 0;			// Stop bits  : 0=1bit, 1=2bit
	SCI2.SMR.BIT.CKS = 1;			// Clock select : 0=�����Ȃ�, 1=����/4
	SCI2.BRR = SCI_4800BPS_n1;		// (BitRateRegister)
	wait2us( 125L );				// 1bit time over wait
	i = SCI2.SSR.BYTE;				// Dummy Read(b7, b6, ORER FER PER TEND MPB MPBT)
	SCI2.SSR.BYTE = 0xC4;			// Status register <- 0
	dummy_Read = SCI2.SSR.BYTE;		// Dummy Read
	SCI2.SCR.BYTE = 0x70;			// RIE, TE, RE <- 1(EI)(TIE RIE TE RE MPIE TEIE CKE[1:0])
}
/****************************************************************************/
/*																			*/
/*	SCI3		���CReader����												*/
/*				Baudrate: 	38,400bps										*/
/*				INT:		Excep_SCI3_RXI3									*/
/*							Excep_SCI3_TXI3									*/
/*							Excep_SCI3_TEI3									*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void Sci3Init( void )
{
	volatile unsigned char	i;
	volatile unsigned long	ist;			// ���݂̊�����t���

	ist = _di2();
	MSTP_SCI3 = 0;					// SCI3 module stop mode release(SYSTEM.MSTPCRB.BIT.MSTPB28)
	SCI3.SEMR.BIT.ABCS = 0x01;		/* ����������{�N���b�N�Z���N�gBIT	*/
	SCI3.SCR.BYTE = 0x00;			// SCI3 <- 0(�����݋֎~)(TIE RIE TE RE MPIE TEIE CKE[1:0])
	_ei2( ist );
	SCI3.SMR.BIT.CM = 0;			// ��������Mode�œ���(CM CHR PE PM STOP MP CKS[1:0])
	SCI3.SMR.BIT.MP = 0;			// Multi pro func : 0=Nouse, 1=Use
	SCI3.SMR.BIT.CHR = 0;			// Data bits  : 0=8bit, 1=7bit
	SCI3.SMR.BIT.PE = 1;			// Parity�L�� : 0=None, 1=Use
	SCI3.SMR.BIT.PM = 1;			// ParityType : 0=Even, 1=Odd
	SCI3.SMR.BIT.STOP = 0;			// Stop bits  : 0=1bit, 1=2bit
	SCI3.SMR.BIT.CKS = 0;			// Clock select : 0=�����Ȃ�, 1=����/4
	SCI3.BRR = SCI_38400BPS_n0;		// (BitRateRegister)
	wait2us( 125L );				// 1bit time over wait
	i = SCI3.SSR.BYTE;				// Dummy Read(b7, b6, ORER FER PER TEND MPB MPBT)
	SCI3.SSR.BYTE = 0xC4;			// Status register <- 0
	dummy_Read = SCI3.SSR.BYTE;		// Dummy Read
	SCI3.SCR.BYTE = 0x70;			// RIE, TE, RE <- 1(EI)(TIE RIE TE RE MPIE TEIE CKE[1:0])
}
/****************************************************************************/
/*																			*/
/*	SCI4		Flap����													*/
/*				Baudrate: 	38,400bps										*/
/*				INT:		Excep_SCI4_RXI4									*/
/*							Excep_SCI4_TXI4									*/
/*							Excep_SCI4_TEI4									*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void Sci4Init( void )
{
	uchar	bps;
	bps = (uchar)prm_get( COM_PRM,S_TYP,75,1,1 );// 03-0075�E �{�[���[�g
	toSsci_Init(bps, IF_SCI_SET_DBITS, IF_SCI_SET_SBITS, IF_SCI_SET_PBIT);
}
/****************************************************************************/
/*																			*/
/*	SCI6		Bluetooth													*/
/*				Baudrate: 	115,200bps										*/
/*				INT:		Excep_SCI6_RXI6									*/
/*							Excep_SCI6_TXI6									*/
/*							Excep_SCI6_TEI6									*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void Sci6Init( void )
{
	volatile unsigned char	i;
	volatile unsigned long	ist;			// ���݂̊�����t���

	ist = _di2();
	MSTP_SCI6 = 0;					// SCI6 module stop mode release(SYSTEM.MSTPCRB.BIT.MSTPB25)
	SCI6.SEMR.BIT.ABCS = 0x01;		/* ����������{�N���b�N�Z���N�gBIT	*/
	SCI6.SCR.BYTE = 0x00;			// SCI6 <- 0(�����݋֎~)(TIE RIE TE RE MPIE TEIE CKE[1:0])
	_ei2( ist );
	SCI6.SMR.BIT.CM = 0;			// ��������Mode�œ���(CM CHR PE PM STOP MP CKS[1:0])
	SCI6.SMR.BIT.MP = 0;			// Multi pro func : 0=Nouse, 1=Use
	SCI6.SMR.BIT.CHR = 0;			// Data bits  : 0=8bit, 1=7bit
	SCI6.SMR.BIT.PE = 0;			// Parity�L�� : 0=None, 1=Use
	SCI6.SMR.BIT.PM = 0;			// ParityType : 0=Even, 1=Odd
	SCI6.SMR.BIT.STOP = 0;			// Stop bits  : 0=1bit, 1=2bit
	SCI6.SMR.BIT.CKS = 0;			// Clock select : 0=�����Ȃ�, 1=����/4
	SCI6.BRR = SCI_115200BPS_n0;	// (BitRateRegister)
	wait2us( 125L );				// 1bit time over wait
	i = SCI6.SSR.BYTE;				// Dummy Read(b7, b6, ORER FER PER TEND MPB MPBT)
	SCI6.SSR.BYTE = 0xC4;			// Status register <- 0
	dummy_Read = SCI6.SSR.BYTE;		// Dummy Read
	SCI6.SCR.BYTE = 0x70;			// RIE, TE, RE <- 1(EI)(TIE RIE TE RE MPIE TEIE CKE[1:0])
}
/****************************************************************************/
/*																			*/
/*	SCI7		serverFOMA_Rism												*/
/*				Baudrate: 	38,400bps										*/
/*				INT:		Excep_SCI7_RXI7									*/
/*							Excep_SCI7_TXI7									*/
/*							Excep_SCI7_TEI7									*/
/*					PCLK = 50MHz(20nsec/1clock)								*/
/*																			*/
/****************************************************************************/
void Sci7Init( void )
{
	KSG_RauSCI_Init();
}
/****************************************************************************/
/*																			*/
/*	SCI9		NT-NET�ʐM													*/
/*				Baudrate: 	38,400bps										*/
/*				INT:		Excep_SCI9_RXI9									*/
/*							Excep_SCI9_TXI9									*/
/*							Excep_SCI9_TEI9									*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void Sci9Init( void )
{
	volatile unsigned char	i;
	volatile unsigned long	ist;			// ���݂̊�����t���

	ist = _di2();
	MSTP_SCI9 = 0;					// SCI9 module stop mode release(SYSTEM.MSTPCRC.BIT.MSTPC26)
	SCI9.SEMR.BIT.ABCS = 0x01;		/* ����������{�N���b�N�Z���N�gBIT	*/
	SCI9.SCR.BYTE = 0x00;			// SCI9 <- 0(�����݋֎~)(TIE RIE TE RE MPIE TEIE CKE[1:0])
	_ei2( ist );
	SCI9.SMR.BIT.CM = 0;			// ��������Mode�œ���(CM CHR PE PM STOP MP CKS[1:0])
	SCI9.SMR.BIT.MP = 0;			// Multi pro func : 0=Nouse, 1=Use
	SCI9.SMR.BIT.CHR = 0;			// Data bits  : 0=8bit, 1=7bit
	SCI9.SMR.BIT.PE = 1;			// Parity�L�� : 0=None, 1=Use
	SCI9.SMR.BIT.PM = 0;			// ParityType : 0=Even, 1=Odd
	SCI9.SMR.BIT.STOP = 0;			// Stop bits  : 0=1bit, 1=2bit
	SCI9.SMR.BIT.CKS = 0;			// Clock select : 0=�����Ȃ�, 1=����/4
	SCI9.BRR = SCI_38400BPS_n0;		// (BitRateRegister)
	wait2us( 125L );				// 1bit time over wait
	i = SCI9.SSR.BYTE;				// Dummy Read(b7, b6, ORER FER PER TEND MPB MPBT)
	SCI9.SSR.BYTE = 0xC4;			// Status register <- 0
	dummy_Read = SCI9.SSR.BYTE;		// Dummy Read
	SCI9.SCR.BYTE = 0x70;			// RIE, TE, RE <- 1(EI)(TIE RIE TE RE MPIE TEIE CKE[1:0])
}

/****************************************************************************/
/*																			*/
/*	SCI10		ParkiPro													*/
/*				Baudrate: 	9,600bps										*/
/*				INT:		Excep_SCI10_RXI10								*/
/*							Excep_SCI10_TXI10								*/
/*							Excep_SCI10_TEI10								*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void Sci10Init( void )
{
	volatile unsigned char	i;
	volatile unsigned long	ist;			// ���݂̊�����t���

	ist = _di2();
	MSTP_SCI10 = 0;					// SCI10 module stop mode release(SYSTEM.MSTPCRC.BIT.MSTPC25)
	SCI10.SEMR.BIT.ABCS = 0x01;		/* ����������{�N���b�N�Z���N�gBIT	*/
	SCI10.SCR.BYTE = 0x00;			// SCI10 <- 0(�����݋֎~)(TIE RIE TE RE MPIE TEIE CKE[1:0])
	_ei2( ist );
	SCI10.SMR.BIT.CM = 0;			// ��������Mode�œ���(CM CHR PE PM STOP MP CKS[1:0])
	SCI10.SMR.BIT.MP = 0;			// Multi pro func : 0=Nouse, 1=Use
	SCI10.SMR.BIT.CHR = prm_get(COM_PRM, S_CEN, 24, 1, 3);	// Data bits  : 0=8bit, 1=7bit
	switch( prm_get(COM_PRM, S_CEN, 24, 1, 1) ){
	case 0:	// �p���e�B�r�b�g �Ȃ�
		SCI10.SMR.BIT.PE = 0;			// Parity�L�� : 0=None, 1=Use
		break;
	case 1:	// �
		SCI10.SMR.BIT.PE = 1;			// Parity�L�� : 0=None, 1=Use
		SCI10.SMR.BIT.PM = 1;			// ParityType : 0=Even, 1=Odd
		break;
	case 2:	// ����
	default:
		SCI10.SMR.BIT.PE = 1;			// Parity�L�� : 0=None, 1=Use
		SCI10.SMR.BIT.PM = 0;			// ParityType : 0=Even, 1=Odd
		break;
	}
	SCI10.SMR.BIT.STOP = prm_get(COM_PRM, S_CEN, 24, 1, 2);			// Stop bits  : 0=1bit, 1=2bit
	switch( prm_get(COM_PRM, S_CEN, 24, 1, 4) ){	// �ʐM���x
	case 0:	// 38400bps
		SCI10.SMR.BIT.CKS = 0;			// Clock select : 0=�����Ȃ�, 1=����/4
		SCI10.BRR = SCI_38400BPS_n0;	// (BitRateRegister)
		break;
	case 1:	// 19200bps
		SCI10.SMR.BIT.CKS = 1;			// Clock select : 0=�����Ȃ�, 1=����/4
		SCI10.BRR = SCI_19200BPS_n1;	// (BitRateRegister)
		break;
	case 2:	// 9600bps
		SCI10.SMR.BIT.CKS = 1;			// Clock select : 0=�����Ȃ�, 1=����/4
		SCI10.BRR = SCI_9600BPS_n1;		// (BitRateRegister)
		break;
	case 3:	// 4800bps
		SCI10.SMR.BIT.CKS = 1;			// Clock select : 0=�����Ȃ�, 1=����/4
		SCI10.BRR = SCI_4800BPS_n1;		// (BitRateRegister)
		break;
	case 4:	// 2400bps
	default:
		SCI10.SMR.BIT.CKS = 1;			// Clock select : 0=�����Ȃ�, 1=����/4
		SCI10.BRR = SCI_9600BPS_n1;		// (BitRateRegister)
		break;
	}
	wait2us( 125L );				// 1bit time over wait
	i = SCI10.SSR.BYTE;				// Dummy Read(b7, b6, ORER FER PER TEND MPB MPBT)
	SCI10.SSR.BYTE = 0xC4;			// Status register <- 0
	dummy_Read = SCI10.SSR.BYTE;	// Dummy Read
	SCI10.SCR.BYTE = 0x70;			// RIE, TE, RE <- 1(EI)(TIE RIE TE RE MPIE TEIE CKE[1:0])
}

/****************************************************************************/
/*																			*/
/*	SCI11		MultiICcardR/W												*/
/*				Baudrate: 	38,400bps										*/
/*				INT:		Excep_SCI11_RXI11								*/
/*							Excep_SCI11_TXI11								*/
/*							Excep_SCI11_TEI11								*/
/*				PCLK = 50MHz(20nsec/1clock)									*/
/*																			*/
/****************************************************************************/
void Sci11Init( void )
{
}
/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
void RTC_init(void)
{
	int						ret;
	I2C_REQUEST 			request;
	extern	void ClkrecUpdateFromRTC( struct	RTC_rec *Time );

	request.TaskNoTo	 			= I2C_TASK_OPE;						// ope�^�X�N��I2CREQUEST����������ʒm
	request.DeviceNo 				= I2C_DN_RTC;						// RTC�ւ̗v��
	request.RequestCode    			= I2C_RC_RTC_STATUS_RCV;			// RTC FLAG ���W�X�^���[�h
	request.I2cReqDataInfo.RWCnt	= RTC_FR_SIZE;						// Write/Read����Byte��(1)
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.flag_reg;				// Write/Read����Buffer�ւ�Pointer
	ret = I2C_Request( &request, EXE_MODE_FORCE );						// RTC FLAG���W�X�^���[�h
	if( ret != RIIC_RSLT_OK ){
		// �G���[�������s�����H
	}
	//FLAG���W�X�^�G���[���菈��( VLF(���U��H�d���ቺ) orVDET(���x�⏞��H�d���ቺ))�̏ꍇ
	if( (RTC_CLOCK.flag_reg & 0x03) != 0x00 )
	{
		//VLF(���U��H�d���ቺ)�̏ꍇ
		if((RTC_CLOCK.flag_reg & 0x02) == 0x02)
		{
			RTC_Err.BIT.RTC_VLF = 1;
		}
		//VDET(���x�⏞��H�d���ቺ)�̏ꍇ
		if(( RTC_CLOCK.flag_reg & 0x01) == 0x01 )
		{
			RTC_Err.BIT.RTC_VDET = 1;
		}
	}
	//�R���g���[�����W�X�^�E�t���O���W�X�^�E�G�N�X�e���V�������W�X�^������������
	request.TaskNoTo				= I2C_TASK_OPE;					// ope�^�X�N��I2CREQUEST����������ʒm
	request.DeviceNo 				= I2C_DN_RTC;					// RTC�ւ̗v��
	request.RequestCode 			= I2C_RC_RTC_EX_FLAG_CTL_SND;	// ���W�X�^�i3��ށj������
	request.I2cReqDataInfo.RWCnt	= RTC_EX_FR_CR_SIZE;			// Write/Read����Byte��(3)
	RTC_CLOCK.ex_reg				= RTC_EX_INIT;					// TEST		WADA	USEL	TE		FSEL1	FSEL0	TSEL1	TSEL0(0x20)
//	RTC_CLOCK.ex_reg				= 0x00;
	RTC_CLOCK.flag_reg	 			= RTC_FR_INIT;					// ---		---		UF		TF		AF		EVF		VLF		VDET(0x00)
	RTC_CLOCK.ctrl_reg	 			= RTC_CR_INIT;					// CSEL1	CSEL0	UIE		TIE		AIE		EIE		---		RESET(0x60)
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.ex_reg;			// Write/Read����Buffer�ւ�Pointer
	ret = I2C_Request( &request, EXE_MODE_FORCE );						// RTC ���W�X�^������
	if( ret != RIIC_RSLT_OK ){
		// �G���[�������s�����H
	}

	/*** ��d�������� ***/
	if(0 != chkdate2((short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date) ||
			(CLK_REC.hour >= 24) || (CLK_REC.minu >= 60) || (CLK_REC.seco >= 60)) {
		// ���ݎ����������ȏꍇ��2012/01/01 00:00:00��ݒ肷��
		CLK_REC.year = 2012;
		CLK_REC.mont = 1;
		CLK_REC.date = 1;
		CLK_REC.hour = 0;
		CLK_REC.minu = 0;
		CLK_REC.seco = 0;
		CLK_REC.ndat = dnrmlzm((short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date);
		CLK_REC.nmin = tnrmlz ((short)0, (short)0, (short)CLK_REC.hour, (short)CLK_REC.minu);
		CLK_REC.week = (unsigned char)((CLK_REC.ndat + 6) % 7);
	}
	memcpy(&clk_save, &CLK_REC, sizeof(struct clk_rec));		// ��d�����ۑ�

	//���Ԏ擾�v��
	request.TaskNoTo	 			= I2C_TASK_OPE;				// ope�^�X�N��I2CREQUEST����������ʒm
	request.DeviceNo 	 			= I2C_DN_RTC;				// RTC�ւ̗v��
	request.RequestCode    			= I2C_RC_RTC_TIME_RCV;		// ���Ԏ擾�v��
	request.I2cReqDataInfo.RWCnt	= RTC_RW_SIZE;				// Write/Read����Byte��(8)
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.freq100sec;	// Write/Read����Buffer�ւ�Pointer
	ret = I2C_Request( &request, EXE_MODE_FORCE );
	
	if( ret == RIIC_RSLT_OK ){
		ClkrecUpdateFromRTC( &RTC_CLOCK );						// ���v�̍X�V�����s�Ȃ�
	}
	else{
		// �G���[�������s�����H
	}
	
}
/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
void SC16_DataInit(void)
{
	unsigned char cnt;
	/********************************************************/
	/*	I2C_SC16_REGS	I2cSc16Regs[SC16_IC_CH_NUM] <--- 0	*/
	/********************************************************/
	for( cnt=SC16_I2CDEV2_1; cnt < SC16_IC_CH_NUM; cnt++ ){
//		PowerOn_Rest value						address	7	6	5	4	3	2	1	0
		I2cSc16Regs[cnt].sc16_THR_W = 0;		// 	0	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_RHR_R = 0;		// 	0	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_IER_RW = 0;		// 	1	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_FCR_W = 0;		// 	2	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_IIR_R = 0;		// 	2	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_LCR_RW = 0;		// 	3	0	0	0	1	1	1	0	1
		I2cSc16Regs[cnt].sc16_MCR_RW = 0;		// 	4	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_LSR_R = 0;		// 	5	0	1	1	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_MSR_R = 0;		// 	6	-	-	-	-	0	0	0	0(-: input)
		I2cSc16Regs[cnt].sc16_SPR_RW = 0;		// 	7	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_TCR_RW = 0;		// 	6	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_TLR_RW = 0;		// 	7	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_TXLVL_R = 0;		// 	8	0	1	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_RXLVL_R = 0;		// 	9	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_IODIR_RW = 0;		// 	A	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_IOSTATE_RW = 0;	// 	B	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_IOINTENA_RW = 0;	// 	C	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_IOCONTROL_RW = 0;	// 	E	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_EFCR_RW = 0;		// 	F	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_DLL_SP_RW = 0;	// 	0	not reset
		I2cSc16Regs[cnt].sc16_DLH_SP_RW = 0;	// 	1	not reset
		I2cSc16Regs[cnt].sc16_EFR_EH_RW = 0;	// 	2	0	0	0	0	0	0	0	0
		I2cSc16Regs[cnt].sc16_XON1_EH_RW = 0;	// 	4	not reset
		I2cSc16Regs[cnt].sc16_XON2_EH_RW = 0;	// 	5	not reset
		I2cSc16Regs[cnt].sc16_XOFF1_EH_RW = 0;	// 	6	not reset
		I2cSc16Regs[cnt].sc16_XOFF2_EH_RW = 0;	// 	7	not reset
	}
}
/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
int SC16_CheckSPRregister(int DevName)
{
	int 			ret;
	unsigned char 	ReadNum;
	unsigned char 	WriteNum;
	unsigned char 	ReadData[2];
	unsigned char 	WriteData[2];

	ReadNum = 1;
	WriteNum = 1;
	/************************************/
	/*	Check User Temporary Register	*/
	/************************************/
	ret = readRegSC16(DevName, SC16_SPR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	ReadData[1] = I2cSc16Regs[DevName].sc16_SPR_RW = ReadData[0];
	if (ret != RIIC_RSLT_OK) 	{	return 1;	}

	WriteData[0] = 0x55;
	ret = writeRegSC16(DevName, SC16_SPR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) 	{	return 2;	}
	ret = readRegSC16(DevName, SC16_SPR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	ReadData[1] = I2cSc16Regs[DevName].sc16_SPR_RW = ReadData[0];

	WriteData[0] = 0xAA;
	ret = writeRegSC16(DevName, SC16_SPR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) 	{	return 2;	}
	ret = readRegSC16(DevName, SC16_SPR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	ReadData[1] = I2cSc16Regs[DevName].sc16_SPR_RW = ReadData[0];

	WriteData[0] = 0x99;
	ret = writeRegSC16(DevName, SC16_SPR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) 	{	return 2;	}
	ret = readRegSC16(DevName, SC16_SPR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	ReadData[1] = I2cSc16Regs[DevName].sc16_SPR_RW = ReadData[0];

	WriteData[0] = 0x66;
	ret = writeRegSC16(DevName, SC16_SPR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) 	{	return 2;	}
	ret = readRegSC16(DevName, SC16_SPR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	ReadData[1] = I2cSc16Regs[DevName].sc16_SPR_RW = ReadData[0];

	WriteData[0] = 0x69;
	ret = writeRegSC16(SC16_I2CDEV2_1, SC16_SPR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) 	{	return 2;	}
	WriteData[0] = 0xA5;
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_SPR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) 	{	return 2;	}

	ret = readRegSC16(SC16_I2CDEV2_1, SC16_SPR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	ReadData[1] = I2cSc16Regs[DevName].sc16_SPR_RW = ReadData[0];
	ret = readRegSC16(SC16_I2CDEV2_2, SC16_SPR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	ReadData[1] = I2cSc16Regs[DevName].sc16_SPR_RW = ReadData[0];
	return 0;

}
/****************************************************************************/
/*	I2C_Baudrate: 	400Kbps(2.5us/1bit)										*/
/*	1.	I2C_SC16_REGS	I2cSc16Regs[SC16_IC_CH_NUM] <--- 0					*/
/*	2.	R/W & R Register Get												*/
/*	3.	I2cSc16Regs[SC16_IC_CH_NUM] <- set									*/
/****************************************************************************/
/*		sc16_THR_W	 		// transmit FIFO, write only					*/
/*		sc16_RHR_R			// receive FIFO, read oly						*/
/*		sc16_IER_RW			// interrup enable reg., R/W					*/
/*		sc16_FCR_W			// FIFO control, write only						*/
/*		sc16_IIR_R			// interrupt status, read only					*/
/*		sc16_LCR_RW			// line control, R/W							*/
/*		sc16_MCR_RW			// modem control, R/W							*/
/*		sc16_LSR_R			// Line status, R								*/
/*		sc16_MSR_R			// Modem status, R								*/
/*		sc16_SPR_RW			// scratch pad, R/W								*/
/*=======TCR, TLR = sc16_EFR_EH_RW[BIT_ENHANCED_FUNC_EI_EFR:4] = 1 and		*/
/*			 		sc16_MCR_RW[BIT_TCR_TLR_EI_MCR:2] = 1, accessable		*/
/*		sc16_TCR_RW			// sc16_EFR_EH_RW[4] = 1 and sc16_MCR_RW[2] = 1	*/
/*		sc16_TLR_RW			// sc16_EFR_EH_RW[4] = 1 and sc16_MCR_RW[2] = 1	*/
/*		sc16_TXLVL_R		// TX FIFO, R									*/
/*		sc16_RXLVL_R		// RX FIFO, R									*/
/*		sc16_IODIR_RW		// IO Direction Control R/W						*/
/*		sc16_IOSTATE_RW		// IO State R/W									*/
/*		sc16_IOINTENA_RW	// IO Interrupt Mask R/W						*/
/*		sc16_IOCONTROL_RW	// IO Control R/W								*/
/*		sc16_EFCR_RW		//  Enhancede Function Reg R/W					*/
/*=======sc16_LCR_RW[BIT_DIV_LATCH_EI_LCR:7] = 1 and sc16_LCR_RW is not 0xBF*/
/*		sc16_DLL_SP_RW		// baud rate divisor, LSB, R/W					*/
/*		sc16_DLH_SP_RW		// baud rate divisor, MSB, R/W					*/
/*=======sc16_LCR_RW = 0xBF. 												*/
/*		sc16_EFR_EH_RW		// enhanced register, R/W						*/
/*		sc16_XON1_EH_RW		// flow control R/W								*/
/*		sc16_XON2_EH_RW		// flow control R/W								*/
/*		sc16_XOFF1_EH_RW	// flow control R/W								*/
/*		sc16_XOFF2_EH_RW	// flow control R/W								*/
/*	see -------> I2c_driver.h												*/
/****************************************************************************/
/********************************************************/
/*	*2 = when sc16_EFR_EH_RW[BIT_ENHANCED_FUNC_EI_EFR:4]*/
/*			 = EI, can be modified						*/
/*		CTS_INT_EI_IER(0x80)	// *2					*/
/*		RTS_INT_EI_IER(0x40)	// *2					*/
/*		XOFF_INT_EI_IER(0x20)	// *2					*/
/*		SLEEP_INT_EI_IER(0x10)	// *2					*/
/*		TXFF_TRG_LVL_FCR(0x30)	// *2					*/
/*		INT_ENCODED_IIR(0x3e)	// *2(bit4,5)			*/
/*		CLOCK_DIV_MCR(0x80)		// *2					*/
/*		IRDA_MODE_EI_MCR(0x40)	// *2					*/
/*		XON_ANY_EI_MCR(0x20)	// *2					*/
/*		TCR_TLR_EI_MCR(0x04)	// *2					*/
/********************************************************/
int SC16_RegDataGet(int DevName)
{
	unsigned char ReadNum;
	unsigned char ReadData[2];
	unsigned char WriteNum;
	unsigned char WriteData[2];
	int			  ret;

	ReadNum = 1;
	WriteNum = 1;
	/********************************************************/
	/*	R/W & R Register Get								*/
	/*	I2cSc16Regs[SC16_IC_CH_NUM] <- set					*/
	/*		I2cSc16Regs[DevName].sc16_THR_W = Write only	*/
	/*		I2cSc16Regs[DevName].sc16_FCR_W = Write only	*/
	/********************************************************/
	/************************************************************/
	/*	Special Register set access = SC16_LCR_RW <- 0x80		*/
	/*	for read BaudRate Setting register						*/
	/************************************************************/
	WriteData[0] = 0x80;
	ret = writeRegSC16(DevName, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 13;		}
	/***********/
	/***********/
	// BaudRateLower						<---	accessible only when sc16_LCR_RW[BIT_DIV_LATCH_EI_LCR:7] = 1 and sc16_LCR_RW is not 0xBF
	ret = readRegSC16(DevName, SC16_DLL_SP_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 14;		}
	I2cSc16Regs[DevName].sc16_DLL_SP_RW = ReadData[0];
	// BaudRateUpper						<---	accessible only when sc16_LCR_RW[BIT_DIV_LATCH_EI_LCR:7] = 1 and sc16_LCR_RW is not 0xBF
	ret = readRegSC16(DevName, SC16_DLH_SP_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 14;		}
	I2cSc16Regs[DevName].sc16_DLH_SP_RW = ReadData[0];
	/************************************************************/
	/*	Enhanced Register set access = SC16_LCR_RW <- 0xBF		*/
	/*	for read below register									*/
	/************************************************************/
	WriteData[0] = 0xBF;
	ret = writeRegSC16(DevName, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 7;		}
	/***********/
	/***********/
	// CTS_FLOW_CTR_EI(0x80)RTS_FLOW_CTR_EI(0x40)SPCL_CHAR_DETECT(0x20)ENHANCED_FUNC_EI(0x10)SOFT_FLOW_CTR(0x0f)
	// 										<---		only accessible when sc16_LCR_RW = 0xBF
	ret = readRegSC16(DevName, SC16_EFR_EH_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 8;		}
	I2cSc16Regs[DevName].sc16_EFR_EH_RW = ReadData[0];
	ret = readRegSC16(DevName, SC16_XON1_EH_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 9;		}
	I2cSc16Regs[DevName].sc16_XON1_EH_RW = ReadData[0];
	ret = readRegSC16(DevName, SC16_XON2_EH_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 10;		}
	I2cSc16Regs[DevName].sc16_XON2_EH_RW = ReadData[0];
	ret = readRegSC16(DevName, SC16_XOFF1_EH_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 11;		}
	I2cSc16Regs[DevName].sc16_XOFF1_EH_RW = ReadData[0];
	ret = readRegSC16(DevName, SC16_XOFF2_EH_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 12;		}
	I2cSc16Regs[DevName].sc16_XOFF2_EH_RW = ReadData[0];
	/************************************************************/
	/*	General Register set access = SC16_LCR_RW-bit7 <- 0		*/
	/*	for read below register									*/
	/************************************************************/
	WriteData[0] = 0x17;	// 8 data bit, 2 stop bit, parity = DI
	ret = writeRegSC16(DevName, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 13;		}
	/***********/
	ret = readRegSC16(DevName, SC16_RHR_R, ReadNum, ReadData, BUSFREE_WAIT);
	/***********/
	ret = readRegSC16(DevName, SC16_RHR_R, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 16;		}
	I2cSc16Regs[DevName].sc16_RHR_R = ReadData[0];
	// CTS_INT_EI(0x80)RTS_INT_EI(0x40)XOFF_INT_EI(0x20)SLEEP_INT_EI(0x10)MODEM_INT_EI(0x08)RCV_LINE_INT_EI(0x04)THR_EMPTY_INT_EI(0x02)RHR_RCV_INT_EI(0x01)
	ret = readRegSC16(DevName, SC16_IER_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 17;		}
	I2cSc16Regs[DevName].sc16_IER_RW = ReadData[0];
	// RXFF_TRG_LVL(0xc0)TXFF_TRG_LVL(0x30)TXFF_RESET(0x04)RXFF_RESET(0x02)FIFO_TRG_EI(0x01)
	// MIRR_FIFO_TRG_EI(0xc0)INT_ENCODED(0x3e)INT_STATUS(0x01)
	ret = readRegSC16(DevName, SC16_IIR_R, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 18;		}
	I2cSc16Regs[DevName].sc16_IIR_R = ReadData[0];
	// DIV_LATCH_EI(0x80)BREAK_CTR(0x40)FORCE_PARITY(0x20)EVEN_PARITY(0x10)PARITY_EI(0x08)NUM_STOP_BIT(0x04)WORD_LENGTH(0x03)
	ret = readRegSC16(DevName, SC16_LCR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 19;		}
	I2cSc16Regs[DevName].sc16_LCR_RW = ReadData[0];
	// CLOCK_DIV(0x80)IRDA_MODE_EI(0x40)XON_ANY_EI(0x20)LOOPBACK_EI(0x10)TCR_TLR_EI(0x04)RTS_FORCE_SET(0x02)DTR_FORCE_SET(0x01)
	ret = readRegSC16(DevName, SC16_MCR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 20;		}
	I2cSc16Regs[DevName].sc16_MCR_RW = ReadData[0];
	// FIFO_DATA_ERR(0x80)THR_TSR_EMPTY(0x40)THR_EMPTY(0x20)BREAK_INT(0x10)FRAMING_ERR(0x08)PARITY_ERR(0x04)OVERRUN_ERR(0x02)DATA_RECEIVED(0x01)
	ret = readRegSC16(DevName, SC16_LSR_R, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 21;		}
	I2cSc16Regs[DevName].sc16_LSR_R = ReadData[0];
	// PIN_STATE_CD(0x80)PIN_STATE_RI(0x40)PIN_STATE_DSR(0x20)PIN_STATE_CTS(0x10)PIN_CHANGE_CD(0x08)PIN_CHANGE_RI(0x04)PIN_CHANGE_DSR(0x02)PIN_CHANGE_CTS(0x01)
	ret = readRegSC16(DevName, SC16_MSR_R, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 22;		}
	I2cSc16Regs[DevName].sc16_MSR_R = ReadData[0];
	// User Temporary Register
	ret = readRegSC16(DevName, SC16_SPR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 23;		}
	I2cSc16Regs[DevName].sc16_SPR_RW = ReadData[0];
	// NUM_TX_FIFO(0x7f) number of spaces available in TX FIFO
	ret = readRegSC16(DevName, SC16_TXLVL_R, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 24;		}
	I2cSc16Regs[DevName].sc16_TXLVL_R = ReadData[0];
	// NUM_RX_FIFO(0x7f) number of characters stored in RX FIFO
	ret = readRegSC16(DevName, SC16_RXLVL_R, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 25;		}
	I2cSc16Regs[DevName].sc16_RXLVL_R = ReadData[0];
	// GPIO0_IN_OUT(0x01) - GPIO7_IN_OUT(0x80)
	ret = readRegSC16(DevName, SC16_IODIR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 26;		}
	I2cSc16Regs[DevName].sc16_IODIR_RW = ReadData[0];
	// GPIO0_DSRB(0x01)GPIO1_DTRB(0x02)GPIO2_CDB(0x04)GPIO3_RIB(0x08)GPIO4_DSRA(0x10)GPIO5_DTRA(0x20)GPIO6_CDA(0x40)GPIO7_RIA(0x80)
	ret = readRegSC16(DevName, SC16_IOSTATE_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 27;		}
	I2cSc16Regs[DevName].sc16_IOSTATE_RW = ReadData[0];
	// GPIO0_INT_EI(0x01) - GPIO7_INT_EI(0x80)
	ret = readRegSC16(DevName, SC16_IOINTENA_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 28;		}
	I2cSc16Regs[DevName].sc16_IOINTENA_RW = ReadData[0];
	// SOFT_RESET(0x08)GPIO0_3_SEL(0x04)GPIO4_7_SEL(0x02)IOLATCH_EI(0x01)
	ret = readRegSC16(DevName, SC16_IOCONTROL_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 29;		}
	I2cSc16Regs[DevName].sc16_IOCONTROL_RW = ReadData[0];
	// IRDA_MODE(0x80)RTS_INVERT(0x20)RTS_CTR_EI(0x10)TX_SEND_DI(0x04)RX_RECV_DI(0x02)SEL_232_485(0x01)
	ret = readRegSC16(DevName, SC16_EFCR_RW, ReadNum, ReadData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 30;		}
	I2cSc16Regs[DevName].sc16_EFCR_RW = ReadData[0];
	/************************************************************/
	/*	TCR, TLC can access, when EFR[4] = 1 & MCR[2] = 1		*/
	/************************************************************/
	return 0;
}
/****************************************************************************/
/*	H/W����																	*/
/*		�{LSI��Channel���ɒʐM���x��ݒ�ł��Ȃ���							*/
/*		2Channel�Ƃ������ʐM���x�̂��̂�ڑ����鎖							*/
/*		I2C_Baudrate: 	400Kbps(2.5us/1bit)									*/
/*	�����U���q		14.7456MH��												*/
/*	MCR_bit7		0														*/
/*	BaudRate	N(�v�Z�l)	N(�ݒ�l)	�덷��(%)							*/
/*	19200		48.000 		48 			0.00		o						*/
/*	------------------------------------------------------------------		*/
/*	ReceiptPrinter:															*/
/*		TXD		O		Data���M											*/
/*		RXD		I		Data��M											*/
/*		RTS		O		HOST_BUSY�M��										*/
/*		CTS		I		Printer_BUSY�M��									*/
/*		*2 = when EFR[4] = EI, can be modified								*/
/*			Data bits	8bit												*/
/*			Parity�L��	None												*/
/*			ParityType	Even												*/
/*			Stop bits	2bit												*/
/****************************************************************************/
int SC16_RP_init(void)
{
	unsigned char WriteNum;
	unsigned char WriteData[2];
	int 		  ret;
	WriteNum = 1;
	/************************************************************/
	/*	Special Register set access = SC16_LCR_RW <- 0x80		*/
	/*	for BaudRate Setting									*/
	/************************************************************/
	WriteData[0] = 0x80;
	ret = writeRegSC16(SC16_RP, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 1;		}
	//	19200bps�̐ݒ�
	WriteData[0] = I2C_DEV_BAUD_RATE_L_19200;
	ret = writeRegSC16(SC16_RP, SC16_DLL_SP_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 2;		}
	WriteData[0] = I2C_DEV_BAUD_RATE_H_19200;
	ret = writeRegSC16(SC16_RP, SC16_DLH_SP_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 3;		}
	/************************************************************/
	/*	Enhanced Register set access = SC16_LCR_RW <- 0xBF		*/
	/*	for enable enhanced function Setting					*/
	/************************************************************/
	WriteData[0] = 0xBF;
	ret = writeRegSC16(SC16_RP, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 4;		}
	//	ENHANCED_FUNC_EI <- 1 for bit modify
	//	CTS���͐M����RTS�o�͐M����SC16��HW��Auto�ōs���ׂɈȉ���EI����!?
	//		BIT_AUTO_CTS_CTR_EI_EFR(0x80)	// 1 = CTS flow control is EI(AutoCTS)
	//		BIT_AUTO_RTS_CTR_EI_EFR(0x40)	// 1 = RTS flow control is EI(AutoRTS)
	WriteData[0] = (BIT_ENHANCED_FUNC_EI_EFR |
					BIT_AUTO_CTS_CTR_EI_EFR  |
					BIT_AUTO_RTS_CTR_EI_EFR);
	ret = writeRegSC16(SC16_RP, SC16_EFR_EH_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 5;		}
	/************************************************************/
	/*	General Register set access = SC16_LCR_RW-bit7 <- 0		*/
	/************************************************************/
	//	DIV_LATCH_EI:7		<- 0	//	General Register set access Mode
	//	BREAK_CTR:6			<- 0	//	no TX break condition
	//	FORCE_PARITY:5		<- 0	//	not force parity
	//	EVEN_PARITY:4		<- 1	//	parity = EVEN
	//	PARITY_EI:3			<- 0	//	Parity Enable <- DI
	//	NUM_STOP_BIT:2		<- 1	//	stop = 2
	//	WORD_LENGTH_LCR:1,0	<- 3	//	data = 8bit
	WriteData[0] = 0x17;	// 8 data bit, 2 stop bit, parity = DI
	ret = writeRegSC16(SC16_RP, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 6;		}
	/********************************************/
	/*	Enable transmitter controls RTS pin		*/
	/********************************************/
	//	IRDA_MODE  (0x80)	<- 0 = Unuse IrDAMode
	//	RTS_INVERT (0x20)	<- 0 = Not Invert RTS signal
	//	RTS_CTR_EI (0x10)	<- 1 = Enable transmitter controls RTS pin
	//	TX_SEND_DI (0x04)	<- 0 = transmitter is EI
	//	RX_RECV_DI (0x02)	<- 0 = receiver is EI
	//	SEL_232_485(0x01)	<- 0 = normal RS-232 mode
	WriteData[0] = 0x30;
	ret = writeRegSC16(SC16_RP, SC16_EFCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 11;		}
	/************************************/
	/*	TX_FIFO, RX_FIFO <- Flush		*/
	/************************************/
	//	RXFF_TRG_LVL	<- 0	//	00 = 8character
	//	TXFF_TRG_LVL	<- 0	//	00 = 8character(*2)
	//	TXFF_RESET		<- 1	//	clear TX_FIFO & TX_FIFO_LEVEL
	//	RXFF_RESET		<- 1	//	clear RX_FIFO & RX_FIFO_LEVEL
	//	FIFO_TRG_EI		<- 0	//	TX_FIFO & RX_FIFO <- DI
	WriteData[0] = 0x06;		// reset TXFIFO, reset RXFIFO, non FIFO mode
	ret = writeRegSC16(SC16_RP, SC16_FCR_W, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 10;		}
	/************************************/
	/*	TX_FIFO, RX_FIFO <- Use EI		*/
	/*	TX_FIFO_Size = 8char			*/
	/*	RX_FIFO_Size = 8char			*/
	/************************************/
	//	FIFO_TRG_EI		<- 1	//	TX_FIFO & RX_FIFO <- EI(FIFO == 8character)
	WriteData[0] = 0x01;		// enable FIFO mode
	ret = writeRegSC16(SC16_RP, SC16_FCR_W, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 11;		}
	/************************************/
	/*	���YDevice�̊����ݔ����v���ݒ�	*/
	/************************************/
	//	CTS_INT_EI		<- 0	//	CTS INT <- DI(*2)
	//	RTS_INT_EI		<- 0	//	RTS INT <- DI(*2)
	//	XOFF_INT_EI		<- 0	//	XOFF INT <- DI(*2)
	//	SLEEP_INT_EI	<- 0	//	SLEEP INT <- DI(*2)
	//	MODEM_INT_EI	<- 0	//	MODEM INT <- DI
	//	RCV_LINE_INT_EI	<- 1	//	RCV_LINE INT <- EI
	//	THR_EMPTY_INT_EI<- 1	//	THR transmitt INT <- EI
	//	--->	1ByteWrite����̂�75us������̂�
	//			��͂�writeRegSC16/readRegSC16���鎞��
	//			��{�I��BUSFREE_IGNO�ōs��Ȃ��Ƃ����Ȃ��B
	//			(THR_EMPTY_INT_EI��ByteHandling����)
	//	RHR_RCV_INT_EI	<- 1	//	RHR receive INT <- EI
	WriteData[0] = 0x01;		// RHR_RCV <- EI
	ret = writeRegSC16(SC16_RP, SC16_IER_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 12;		}
	return 0;
}
/****************************************************************************/
/*	H/W����																	*/
/*		�{LSI��Channel���ɒʐM���x��ݒ�ł��Ȃ���							*/
/*		2Channel�Ƃ������ʐM���x�̂��̂�ڑ����鎖							*/
/*		I2C_Baudrate: 	400Kbps(2.5us/1bit)									*/
/*	�����U���q		14.7456MH��												*/
/*	MCR_bit7		0														*/
/*	BaudRate	N(�v�Z�l)	N(�ݒ�l)	�덷��(%)							*/
/*	19200		48.000 		48 			0.00		o						*/
/*	------------------------------------------------------------------		*/
/*	JernalPrinter:															*/
/*		TXD		O		Data���M											*/
/*		RXD		I		Data��M											*/
/*		RTS		O		HOST_BUSY�M��										*/
/*		CTS		I		Printer_BUSY�M��									*/
/*		RESET	O		reset = �@��ɂ���Ă͕s�v							*/
/*		*2 = when EFR[4] = EI, can be modified								*/
/*			Data bits	8bit												*/
/*			Parity�L��	None												*/
/*			ParityType	Even												*/
/*			Stop bits	2bit												*/
/****************************************************************************/
int SC16_JP_init(void)
{
	unsigned char WriteNum;
	unsigned char WriteData[2];
	int 		  ret;
	WriteNum = 1;
	/************************************************************/
	/*	Special Register set access = SC16_LCR_RW <- 0x80		*/
	/*	for BaudRate Setting									*/
	/************************************************************/
	WriteData[0] = 0x80;
	ret = writeRegSC16(SC16_JP, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 1;		}
	//	19200bps�̐ݒ�
	WriteData[0] = I2C_DEV_BAUD_RATE_L_19200;
	ret = writeRegSC16(SC16_JP, SC16_DLL_SP_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 2;		}
	WriteData[0] = I2C_DEV_BAUD_RATE_H_19200;
	ret = writeRegSC16(SC16_JP, SC16_DLH_SP_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 3;		}
	/************************************************************/
	/*	Enhanced Register set access = SC16_LCR_RW <- 0xBF		*/
	/*	for enable enhanced function Setting					*/
	/************************************************************/
	WriteData[0] = 0xBF;
	ret = writeRegSC16(SC16_JP, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 4;		}
	//	ENHANCED_FUNC_EI <- 1 for bit modify
	//	CTS���͐M����RTS�o�͐M����SC16��HW��Auto�ōs���ׂɈȉ���EI����!?
	//		BIT_AUTO_CTS_CTR_EI_EFR(0x80)	// 1 = CTS flow control is EI(AutoCTS)
	//		BIT_AUTO_RTS_CTR_EI_EFR(0x40)	// 1 = RTS flow control is EI(AutoRTS)
	WriteData[0] = (BIT_ENHANCED_FUNC_EI_EFR |
					BIT_AUTO_CTS_CTR_EI_EFR  |
					BIT_AUTO_RTS_CTR_EI_EFR);
	ret = writeRegSC16(SC16_JP, SC16_EFR_EH_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 5;		}
	/************************************************************/
	/*	General Register set access = SC16_LCR_RW-bit7 <- 0		*/
	/************************************************************/
	//	DIV_LATCH_EI:7		<- 0	//	General Register set access Mode
	//	BREAK_CTR:6			<- 0	//	no TX break condition
	//	FORCE_PARITY:5		<- 0	//	not force parity
	//	EVEN_PARITY:4		<- 1	//	parity = EVEN
	//	PARITY_EI:3			<- 0	//	Parity Enable <- DI
	//	NUM_STOP_BIT:2		<- 1	//	stop = 2
	//	WORD_LENGTH_LCR:1,0	<- 3	//	data = 8bit
	WriteData[0] = 0x17;	// 8 data bit, 2 stop bit, parity = DI
	ret = writeRegSC16(SC16_JP, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 6;		}
	/********************************************/
	/*	Enable transmitter controls RTS pin		*/
	/********************************************/
	//	IRDA_MODE  (0x80)	<- 0 = Unuse IrDAMode
	//	RTS_INVERT (0x20)	<- 0 = Not Invert RTS signal
	//	RTS_CTR_EI (0x10)	<- 1 = Enable transmitter controls RTS pin
	//	TX_SEND_DI (0x04)	<- 0 = transmitter is EI
	//	RX_RECV_DI (0x02)	<- 0 = receiver is EI
	//	SEL_232_485(0x01)	<- 0 = normal RS-232 mode
	WriteData[0] = 0x30;
	ret = writeRegSC16(SC16_JP, SC16_EFCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 11;		}
	/************************************/
	/*	TX_FIFO, RX_FIFO <- Flush		*/
	/************************************/
	//	RXFF_TRG_LVL	<- 0	//	00 = 8character
	//	TXFF_TRG_LVL	<- 0	//	00 = 8character(*2)
	//	TXFF_RESET		<- 1	//	clear TX_FIFO & TX_FIFO_LEVEL
	//	RXFF_RESET		<- 1	//	clear RX_FIFO & RX_FIFO_LEVEL
	//	FIFO_TRG_EI		<- 0	//	TX_FIFO & RX_FIFO <- DI
	WriteData[0] = 0x06;		// reset TXFIFO, reset RXFIFO, non FIFO mode
	ret = writeRegSC16(SC16_JP, SC16_FCR_W, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 10;		}
	/************************************/
	/*	TX_FIFO, RX_FIFO <- Use EI		*/
	/*	TX_FIFO_Size = 8char			*/
	/*	RX_FIFO_Size = 8char			*/
	/************************************/
	//	FIFO_TRG_EI		<- 1	//	TX_FIFO & RX_FIFO <- EI(FIFO == 8character)
	WriteData[0] = 0x01;		// enable FIFO mode
	ret = writeRegSC16(SC16_JP, SC16_FCR_W, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 11;		}
	/************************************/
	/*	���YDevice�̊����ݔ����v���ݒ�	*/
	/************************************/
	//	CTS_INT_EI		<- 0	//	CTS INT <- DI(*2)
	//	RTS_INT_EI		<- 0	//	RTS INT <- DI(*2)
	//	XOFF_INT_EI		<- 0	//	XOFF INT <- DI(*2)
	//	SLEEP_INT_EI	<- 0	//	SLEEP INT <- DI(*2)
	//	MODEM_INT_EI	<- 0	//	MODEM INT <- DI
	//	RCV_LINE_INT_EI	<- 1	//	RCV_LINE INT <- EI
	//	THR_EMPTY_INT_EI<- 1	//	THR transmitt INT <- EI
	//	--->	1ByteWrite����̂�75us������̂�
	//			��͂�writeRegSC16/readRegSC16���鎞��
	//			��{�I��BUSFREE_IGNO�ōs��Ȃ��Ƃ����Ȃ��B
	//			(THR_EMPTY_INT_EI��ByteHandling����)
	//	RHR_RCV_INT_EI	<- 1	//	RHR receive INT <- EI
	WriteData[0] = 0x01;		// THR_EMPTY, RHR_RCV <- EI
	ret = writeRegSC16(SC16_JP, SC16_IER_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 12;		}
	return 0;
}

/****************************************************************************/
/*																			*/
/*	H/W����																	*/
/*		�{LSI��Channel���ɒʐM���x��ݒ�ł��Ȃ���							*/
/*		2Channel�Ƃ������ʐM���x�̂��̂�ڑ����鎖							*/
/*		I2C_Baudrate: 	400Kbps(2.5us/1bit)									*/
/*	�����U���q		14.7456MH��												*/
/*	MCR_bit7		0														*/
/*	BaudRate	N(�v�Z�l)	N(�ݒ�l)	�덷��(%)							*/
/*	19200		48.000 		48 			0.00		o						*/
/*	------------------------------------------------------------------		*/
/*	I2CDEV2_2:																	*/
/*			TXD		O		Data���M										*/
/*			RXD		I		Data��M										*/
/*			RTS		O		���MRequest�M��									*/
/*			CTS		I		���M�M��										*/
/*		*2 = when EFR[4] = EI, can be modified								*/
/****************************************************************************/
int SC16_I2CDEV2_2_init(void)
{
	unsigned char WriteNum;
	unsigned char WriteData[2];
	int 		  ret;
	WriteNum = 1;
	/************************************************************/
	/*	Special Register set access = SC16_LCR_RW <- 0x80		*/
	/*	for BaudRate Setting									*/
	/************************************************************/
	WriteData[0] = 0x80;
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 1;		}
	//	19200bps�̐ݒ�
	WriteData[0] = I2C_DEV_BAUD_RATE_L_19200;
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_DLL_SP_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 2;		}
	WriteData[0] = I2C_DEV_BAUD_RATE_H_19200;
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_DLH_SP_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 3;		}
	/************************************************************/
	/*	Enhanced Register set access = SC16_LCR_RW <- 0xBF		*/
	/*	for enable enhanced function Setting					*/
	/************************************************************/
	WriteData[0] = 0xBF;
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 4;		}
	//	ENHANCED_FUNC_EI <- 1 for bit modify
	//	CTS���͐M����RTS�o�͐M����SC16��HW��Auto�ōs���ׂɈȉ���EI����!?
	//		BIT_AUTO_CTS_CTR_EI_EFR(0x80)	// 1 = CTS flow control is EI(AutoCTS)
	//		BIT_AUTO_RTS_CTR_EI_EFR(0x40)	// 1 = RTS flow control is EI(AutoRTS)
	WriteData[0] = (BIT_ENHANCED_FUNC_EI_EFR |
					BIT_AUTO_CTS_CTR_EI_EFR  |
					BIT_AUTO_RTS_CTR_EI_EFR);
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_EFR_EH_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 5;		}
	/************************************************************/
	/*	General Register set access = SC16_LCR_RW-bit7 <- 0		*/
	/************************************************************/
	//	DIV_LATCH_EI:7		<- 0	//	General Register set access Mode
	//	BREAK_CTR:6			<- 0	//	no TX break condition
	//	FORCE_PARITY:5		<- 0	//	not force parity
	//	EVEN_PARITY:4		<- 1	//	parity = EVEN
	//	PARITY_EI:3			<- 0	//	Parity Enable <- DI
	//	NUM_STOP_BIT:2		<- 1	//	stop = 2
	//	WORD_LENGTH_LCR:1,0	<- 3	//	data = 8bit
	WriteData[0] = 0x17;	// 8 data bit, 2 stop bit, parity = DI
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_LCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 6;		}
	/********************************************/
	/*	Enable transmitter controls RTS pin		*/
	/********************************************/
	//	IRDA_MODE  (0x80)	<- 0 = Unuse IrDAMode
	//	RTS_INVERT (0x20)	<- 0 = Not Invert RTS signal
	//	RTS_CTR_EI (0x10)	<- 1 = Enable transmitter controls RTS pin
	//	TX_SEND_DI (0x04)	<- 0 = transmitter is EI
	//	RX_RECV_DI (0x02)	<- 0 = receiver is EI
	//	SEL_232_485(0x01)	<- 0 = normal RS-232 mode
	WriteData[0] = 0x30;
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_EFCR_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 11;		}
	/************************************/
	/*	TX_FIFO, RX_FIFO <- Flush		*/
	/************************************/
	//	RXFF_TRG_LVL	<- 0	//	00 = 8character
	//	TXFF_TRG_LVL	<- 0	//	00 = 8character(*2)
	//	TXFF_RESET		<- 1	//	clear TX_FIFO & TX_FIFO_LEVEL
	//	RXFF_RESET		<- 1	//	clear RX_FIFO & RX_FIFO_LEVEL
	//	FIFO_TRG_EI		<- 0	//	TX_FIFO & RX_FIFO <- DI
	WriteData[0] = 0x06;		// reset TXFIFO, reset RXFIFO, non FIFO mode
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_FCR_W, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 10;		}
	/************************************/
	/*	TX_FIFO, RX_FIFO <- Use EI		*/
	/*	TX_FIFO_Size = 8char			*/
	/*	RX_FIFO_Size = 8char			*/
	/************************************/
	//	FIFO_TRG_EI		<- 1	//	TX_FIFO & RX_FIFO <- EI(FIFO == 8character)
	WriteData[0] = 0x01;		// enable FIFO mode
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_FCR_W, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 11;		}
	/************************************/
	/*	���YDevice�̊����ݔ����v���ݒ�	*/
	/************************************/
	//	CTS_INT_EI		<- 0	//	CTS INT <- DI(*2)
	//	RTS_INT_EI		<- 0	//	RTS INT <- DI(*2)
	//	XOFF_INT_EI		<- 0	//	XOFF INT <- DI(*2)
	//	SLEEP_INT_EI	<- 0	//	SLEEP INT <- DI(*2)
	//	MODEM_INT_EI	<- 0	//	MODEM INT <- DI
	//	RCV_LINE_INT_EI	<- 1	//	RCV_LINE INT <- EI
	//	THR_EMPTY_INT_EI<- 1	//	THR transmitt INT <- EI
	//	--->	1ByteWrite����̂�75us������̂�
	//			��͂�writeRegSC16/readRegSC16���鎞��
	//			��{�I��BUSFREE_IGNO�ōs��Ȃ��Ƃ����Ȃ��B
	//			(THR_EMPTY_INT_EI��ByteHandling����)
	//	RHR_RCV_INT_EI	<- 1	//	RHR receive INT <- EI
	WriteData[0] = 0x01;		// THR_EMPTY, RHR_RCV <- EI
	ret = writeRegSC16(SC16_I2CDEV2_2, SC16_IER_RW, WriteNum, WriteData, BUSFREE_WAIT);
	if (ret != RIIC_RSLT_OK) {	return 12;		}
	return 0;
}
/********************************************/
/*	PCLK = 50MHz(20nsec)					*/
/*	TCNT.1clock = 1.280usec(20 * 64)		*/
/*	Tcnt.max = 83.886080ms(1280 * 65535)	*/
/********************************************/
void	TPU5_init( void )	// TPU0_init
{

	MSTP_TPU0 = 0;					// TPU5 module stop release(SYSTEM.MSTPCRA.BIT.MSTPA13)
	wait2us(5L);
	TPU5.TIOR.BIT.IOB = 0;			// TGRB output <- DI(IOB[3:0], IOA[3:0])
	TPU5.TIOR.BIT.IOA = 0;			// TGRA output <- DI(IOB[3:0], IOA[3:0])
	TPU5.TCR.BIT.TPSC = 3;			// PCLK/64(CCLR[2:0], CKEG[1:0], TPSC[2:0])
	TPU5.TCNT = 0;					// up counter <- 0(16bit)
	TPU5.TSR.BYTE = 0;				// status <- 0(TCFD, b6, TCFU, TCFV, TGFD, TGFC, TGFB, TGFA)
	dummy_Read = TPU5.TSR.BYTE;		// Dummy Read
	TPUA.TSTR.BIT.CST5 = 1;			// ���ĊJ�n(b7, b6, CST5, CST4, CST3, CST2, CST1, CST0)
}
/********************************************/
/*	PCLK = 50MHz(20nsec)					*/
/*	TCNT.1clock = 1.280usec(20 * 64)		*/
/*	Tcnt.max = 83.886080ms(1280 * 65535)	*/
/********************************************/
void	TickCntInit( void )
{
	unsigned short cnt, i;

	for(cnt=0; cnt < 8; cnt++) {
		TickCnt_Start[cnt] = 0;
		TickCnt_Stop[cnt] = 0;
		TickCnt_RSLT_Cnt[cnt] = 0;
		for(i=0; i < 128; i++) {
			TickCnt_RSLT_Value[cnt][i] = 0;
		}
	}
	for(cnt=0; cnt < 8; cnt++) {
		TickCntBoxNo[cnt] = 0;
		for(i=0; i < 32; i++) {
			TickCntValue[cnt][i] = 0;
		}
	}
	DebugSignalOut = 0;
	TPU5_init();
}
/********************************************/
/*	PCLK = 50MHz(20nsec)					*/
/*	TCNT.1clock = 1.280usec(20 * 64)		*/
/*	Tcnt.max = 83.886080ms(1280 * 65535)	*/
/********************************************/
void	TickCntStart( unsigned short BoxNo )	// 0-7
{
	unsigned short TickCntValue_Start;

	if (BoxNo >= 8) {
		return;
	}
	TickCntValue_Start = TPU5.TCNT;		// up counter
	TickCnt_Start[BoxNo] = TickCntValue_Start;
	TickCnt_Stop[BoxNo] = 0;
}
/********************************************/
/*	PCLK = 50MHz(20nsec)					*/
/*	TCNT.1clock = 1.280usec(20 * 64)		*/
/*	Tcnt.max = 83.886080ms(1280 * 65535)	*/
/********************************************/
unsigned short	TickCntStop( unsigned short BoxNo )	// 0-7
{
	unsigned short TickCntValue_Stop;
	unsigned short TickCntValue_cal;
	unsigned long  TickCntValue_long;

	if (BoxNo >= 8) {
		return(0);
	}
	TickCntValue_Stop = TPU5.TCNT;		// up counter
	TickCnt_Stop[BoxNo] = TickCntValue_Stop;
	if (TickCnt_Stop[BoxNo] >= TickCnt_Start[BoxNo]) {
		TickCntValue_cal = (TickCnt_Stop[BoxNo] - TickCnt_Start[BoxNo]);
	} else {
		TickCntValue_cal = (0xffff - TickCnt_Start[BoxNo]) + TickCnt_Stop[BoxNo] + 1;
	}
	TickCntValue_long = (unsigned long)TickCntValue_cal;
	TickCntValue_long = (TickCntValue_long * 128)/100;
	TickCntValue_cal = (unsigned short)TickCntValue_long;
	TickCnt_RSLT_Value[BoxNo][TickCnt_RSLT_Cnt[BoxNo] & 0x7f] = TickCntValue_cal;
	TickCnt_RSLT_Cnt[BoxNo]++;
	return(TickCntValue_cal);
	
}
/********************************************/
/*	PCLK = 50MHz(20nsec)					*/
/*	TCNT.1clock = 1.280usec(20 * 64)		*/
/*	Tcnt.max = 83.886080ms(1280 * 65535)	*/
/********************************************/
void	TickCntBuffSet( unsigned short BoxNo )	// 0-7
{
	unsigned short TickCntValue_cal;
	unsigned long  TickCntValue_long;

	if (BoxNo >= 8) {
		return;
	}
	TickCntValue_long = (unsigned long)TPU5.TCNT;		// up counter
	TickCntValue_long = (TickCntValue_long * 128)/100;
	TickCntValue_cal = (unsigned short)TickCntValue_long;
	TickCntValue[BoxNo][TickCntBoxNo[BoxNo] & 0x001f] = TickCntValue_cal;
	TickCntBoxNo[BoxNo]++;

}
/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
void I2cAM_AllClear(void)
{
	int		cnt;

	I2cAM.ActiveDevice = 0;
	I2cAM.ForceRequest = 0;
	I2cAM.NextDeviceProcNo = 0;
	for(cnt=0; cnt < SIZE_APL_I2CDEV2_1_MAX * 4; cnt++)		{	I2cAM.AplBufI2cDev2_1[cnt] = 0;	}
	for(cnt=0; cnt < SIZE_APL_I2CDEV2_2_MAX * 4; cnt++)		{	I2cAM.AplBufI2cDev2_2[cnt] = 0;		}
	for(cnt=0; cnt < SIZE_APL_RP_MAX * 4; cnt++)		{	I2cAM.AplBufRp[cnt] = 0;		}
	for(cnt=0; cnt < SIZE_APL_JP_MAX * 4; cnt++)		{	I2cAM.AplBufJp[cnt] = 0;		}
	for(cnt=0; cnt < SIZE_APL_RTC_MAX * 4; cnt++)		{	I2cAM.AplBufRtc[cnt] = 0;		}
	for(cnt=0; cnt < SIZE_DEV_I2CDEV2_1_MAX; cnt++)			{	I2cAM.DevBufI2cDev2_1[cnt] = 0;	}
	for(cnt=0; cnt < SIZE_DEV_I2CDEV2_2_MAX; cnt++)			{	I2cAM.DevBufI2cDev2_2[cnt] = 0;		}
	for(cnt=0; cnt < SIZE_DEV_RP_MAX; cnt++)			{	I2cAM.DevBufRp[cnt] = 0;		}
	for(cnt=0; cnt < SIZE_DEV_JP_MAX; cnt++)			{	I2cAM.DevBufJp[cnt] = 0;		}
	/************************/
	/*	I2cAM.CtrI2cDev2_1.	*/
	/************************/
	I2cAM.CtrI2cDev2_1.Status = DEVICE_STS_IDLE;
	I2cAM.CtrI2cDev2_1.AplWriteP = 0;
	I2cAM.CtrI2cDev2_1.AplReadP = 0;
	I2cAM.CtrI2cDev2_1.AplRegistedNum = 0;
	I2cAM.CtrI2cDev2_1.AplRegistBoxIndex = 0;
	I2cAM.CtrI2cDev2_1.AplActiveBoxIndex = 0;
	I2cAM.CtrI2cDev2_1.AplActiveRWCnt = 0;
	I2cAM.CtrI2cDev2_1.DevRecvP = 0;
	I2cAM.CtrI2cDev2_1.SendData[0] = 0;
	I2cAM.CtrI2cDev2_1.SendData[1] = 0;
	for(cnt=0; cnt < 4; cnt++) {
		I2cAM.CtrI2cDev2_1.AplReqBox[cnt].TaskNoTo = 0;
		I2cAM.CtrI2cDev2_1.AplReqBox[cnt].DeviceNo = 0;
		I2cAM.CtrI2cDev2_1.AplReqBox[cnt].RequestCode = 0;
		I2cAM.CtrI2cDev2_1.AplReqBox[cnt].I2cReqDataInfo.RWCnt = 0;
		I2cAM.CtrI2cDev2_1.AplReqBox[cnt].I2cReqDataInfo.pRWData = 0;
	}
	/************************/
	/*	I2cAM.CtrI2cDev2_2	*/
	/************************/
	I2cAM.CtrI2cDev2_2.Status = DEVICE_STS_IDLE;
	I2cAM.CtrI2cDev2_2.AplWriteP = 0;
	I2cAM.CtrI2cDev2_2.AplReadP = 0;
	I2cAM.CtrI2cDev2_2.AplRegistedNum = 0;
	I2cAM.CtrI2cDev2_2.AplRegistBoxIndex = 0;
	I2cAM.CtrI2cDev2_2.AplActiveBoxIndex = 0;
	I2cAM.CtrI2cDev2_2.AplActiveRWCnt = 0;
	I2cAM.CtrI2cDev2_2.DevRecvP = 0;
	I2cAM.CtrI2cDev2_2.SendData[0] = 0;
	I2cAM.CtrI2cDev2_2.SendData[1] = 0;
	for(cnt=0; cnt < 4; cnt++) {
		I2cAM.CtrI2cDev2_2.AplReqBox[cnt].TaskNoTo = 0;
		I2cAM.CtrI2cDev2_2.AplReqBox[cnt].DeviceNo = 0;
		I2cAM.CtrI2cDev2_2.AplReqBox[cnt].RequestCode = 0;
		I2cAM.CtrI2cDev2_2.AplReqBox[cnt].I2cReqDataInfo.RWCnt = 0;
		I2cAM.CtrI2cDev2_2.AplReqBox[cnt].I2cReqDataInfo.pRWData = 0;
	}
	/************************/
	/*	I2cAM.CtrRp.		*/
	/************************/
	I2cAM.CtrRp.Status = DEVICE_STS_IDLE;
	I2cAM.CtrRp.AplWriteP = 0;
	I2cAM.CtrRp.AplReadP = 0;
	I2cAM.CtrRp.AplRegistedNum = 0;
	I2cAM.CtrRp.AplRegistBoxIndex = 0;
	I2cAM.CtrRp.AplActiveBoxIndex = 0;
	I2cAM.CtrRp.AplActiveRWCnt = 0;
	I2cAM.CtrRp.DevRecvP = 0;
	I2cAM.CtrRp.SendData[0] = 0;
	I2cAM.CtrRp.SendData[1] = 0;
	for(cnt=0; cnt < 4; cnt++) {
		I2cAM.CtrRp.AplReqBox[cnt].TaskNoTo = 0;
		I2cAM.CtrRp.AplReqBox[cnt].DeviceNo = 0;
		I2cAM.CtrRp.AplReqBox[cnt].RequestCode = 0;
		I2cAM.CtrRp.AplReqBox[cnt].I2cReqDataInfo.RWCnt = 0;
		I2cAM.CtrRp.AplReqBox[cnt].I2cReqDataInfo.pRWData = 0;
	}
	/************************/
	/*	I2cAM.CtrJp.		*/
	/************************/
	I2cAM.CtrJp.Status = DEVICE_STS_IDLE;
	I2cAM.CtrJp.AplWriteP = 0;
	I2cAM.CtrJp.AplReadP = 0;
	I2cAM.CtrJp.AplRegistedNum = 0;
	I2cAM.CtrJp.AplRegistBoxIndex = 0;
	I2cAM.CtrJp.AplActiveBoxIndex = 0;
	I2cAM.CtrJp.AplActiveRWCnt = 0;
	I2cAM.CtrJp.DevRecvP = 0;
	I2cAM.CtrJp.SendData[0] = 0;
	I2cAM.CtrJp.SendData[1] = 0;
	for(cnt=0; cnt < 4; cnt++) {
		I2cAM.CtrJp.AplReqBox[cnt].TaskNoTo = 0;
		I2cAM.CtrJp.AplReqBox[cnt].DeviceNo = 0;
		I2cAM.CtrJp.AplReqBox[cnt].RequestCode = 0;
		I2cAM.CtrJp.AplReqBox[cnt].I2cReqDataInfo.RWCnt = 0;
		I2cAM.CtrJp.AplReqBox[cnt].I2cReqDataInfo.pRWData = 0;
	}
	/************************/
	/*	I2cAM.CtrRtc.		*/
	/************************/
	I2cAM.CtrRtc.Status = DEVICE_STS_IDLE;
	I2cAM.CtrRtc.AplWriteP = 0;
	I2cAM.CtrRtc.AplReadP = 0;
	I2cAM.CtrRtc.AplRegistedNum = 0;
	I2cAM.CtrRtc.AplRegistBoxIndex = 0;
	I2cAM.CtrRtc.AplActiveBoxIndex = 0;
	I2cAM.CtrRtc.AplActiveRWCnt = 0;
	I2cAM.CtrRtc.DevRecvP = 0;
	I2cAM.CtrRtc.SendData[0] = 0;
	I2cAM.CtrRtc.SendData[1] = 0;
	for(cnt=0; cnt < 4; cnt++) {
		I2cAM.CtrRtc.AplReqBox[cnt].TaskNoTo = 0;
		I2cAM.CtrRtc.AplReqBox[cnt].DeviceNo = 0;
		I2cAM.CtrRtc.AplReqBox[cnt].RequestCode = 0;
		I2cAM.CtrRtc.AplReqBox[cnt].I2cReqDataInfo.RWCnt = 0;
		I2cAM.CtrRtc.AplReqBox[cnt].I2cReqDataInfo.pRWData = 0;
	}
	TickCntInit();
}
/********************************************************************************/
/*	�O��SRAM��Clear																*/
/*		����:			�O��SRAM�̑S�̈��0�N���A����							*/
/*		����:			���[�^���[�X�C�b�` = �S�̎�								*/
/*		�O��SRAM�̈�	1: CS1:	0x07000000 - 0x070fffff(1MB)					*/
/*						2: CS1:	0x07400000 - 0x074fffff(1MB)					*/
/*						3: CS2:	0x06000000 - 0x060fffff(1MB)					*/
/*						* 4Byte�P�ʂ�0clear										*/
/********************************************************************************/
int ExSRam_clear( void )
{
	volatile unsigned long 	*start;
	unsigned long 			len_byte;
	int	ret = 0;			// OK
							// ff = rsw != 4, 1 = CS1_1_NG, 2 = CS1_2_NG, 3 = CS2_NG
	/************************/
	/*	rsw��check			*/
	/************************/
// MH810100(S) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iLCD�f�t�H���g�N�����[�h�Ή��j
//	if( (read_rotsw() != 4)&&(read_rotsw() != 5) ){// �X�[�p�[�C�j�V�����A�����C���X�g�[�����[�h�ȊO�͎��{���Ȃ�
	if( read_rotsw() != 4 ){	// �X�[�p�[�C�j�V�������[�h�ȊO�͎��{���Ȃ�
// MH810100(E) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iLCD�f�t�H���g�N�����[�h�Ή��j
		return 0xff;
	}
	len_byte	= 0x100000;		// 1MB
	/********************************************/
	/*	CS1:	0x07000000 - 0x070fffff(1MB)	*/
	/********************************************/
	start	= (unsigned long *)0x07000000UL;
	ret = SramWrite(start, len_byte);
	if (ret != 0) {
		return 1;
	}
	/********************************************/
	/*	CS1:	0x07400000 - 0x074fffff(1MB)	*/
	/********************************************/
	start	= (unsigned long *)0x07400000UL;
	ret = SramWrite(start, len_byte);
	if (ret != 0) {
		return 2;
	}
	/********************************************/
	/*	CS2:	0x06000000 - 0x060fffff(1MB)	*/
	/********************************************/
	start	= (unsigned long *)0x06000000UL;
	ret = SramWrite(start, len_byte);
	if (ret != 0) {
		return 3;
	}
	return ret;
}
/************************************************************************/
/*	MemoryClear(4byte)													*/
/*		src			Clear����̈�ւ̃|�C���^							*/
/*		len_byte	Clear����̈��Byte��								*/
/*		return		0		����I��									*/
/*					�ȊO	Error����									*/
/************************************************************************/
int SramWrite(	volatile unsigned long 	*src,
				unsigned long 			len_byte )
{
	static const unsigned long	verify_data[] = {
		0x55555555,		// 0
		0xaaaaaaaa,		// 1
		0xffffffff,		// 2
		0x00000000,		// 3
	};
//	volatile unsigned long	byte_save;
	volatile unsigned long	byte_read;
	unsigned int			loop, i, j, err;
/**********************************************/
//	volatile unsigned long	*read_src;
//	read_src = src;
//	*read_src++ = verify_data[0];		// Write
//	*read_src++ = verify_data[0];		// Write
//	*read_src++ = verify_data[0];		// Write
//	*read_src++ = verify_data[0];		// Write
//	read_src = 0x07000080;
//	byte_read = *read_src++;		// Read
//	byte_read = *read_src++;		// Read
//	byte_read = *read_src++;		// Read
//	byte_read = *read_src++;		// Read
/**********************************************/
	err = 0;
	loop = len_byte / sizeof(unsigned long);
	for ( i = 0; i < loop; i++ ) {
//		byte_save = *src;				// Write�O��DataSave
		for ( j = 0; j < 4; j++ ) {
			*src = verify_data[j];		// Write
			byte_read = *src;			// Read
			if ( byte_read != verify_data[j] ) {
				err = (verify_data[j] + 1);
				break;
			}
		}
		if (err != 0) {
			break;
		}
//		*src = byte_save;				// Write�O��DataLoad
		src++;							// Address�X�V
	}
	return(err);
}
//[]----------------------------------------------------------------------[]
///	@brief			CMTU0_CMT0 �^�C�}�̏�����
//[]----------------------------------------------------------------------[]
///	@return			ret		: void
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2013/07/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	CMTU0_CMT0_init( void )
{
	MSTP_CMT0 = 0;						// CMT0�X�g�b�v��ԉ���

	CMT.CMSTR0.WORD = 0x0000;			// CMT0�J�E���g�����~
	CMT0.CMCR.WORD = 0x00C0;			// �N���b�N�I���FPCLK(50MHz)/8 = 6.25MHz (T �� 0.16us)
										// �R���y�A�E�}�b�`���荞�݋���
	CMT0.CMCOR = 5000 - 1;				// �R���y�A�l�F 800us = 0.16us * 5000
}
