#ifndef _VECT_H_
#define _VECT_H_
/****************************************************************************/
/*																			*/
/*	Fixed Interrupt Vector													*/
/*																			*/
/****************************************************************************/
#pragma interrupt (Excep_SuperVisorInst)
void Excep_SuperVisorInst(void);
#pragma interrupt (Excep_UndefinedInst)
void Excep_UndefinedInst(void);
#pragma interrupt (Excep_FloatingPoint)
void Excep_FloatingPoint(void);
#pragma interrupt (NonMaskableInterrupt)
void NonMaskableInterrupt(void);
#pragma interrupt (Dummy)
void Dummy(void);
/****************************************************************************/
/*																			*/
/*	Variable Interrupt Vector												*/
/*																			*/
/****************************************************************************/
#pragma interrupt (Excep_IRQ4(vect=68))
void Excep_IRQ4(void);							//	#RTC_IRQ/RTC時刻同期信号(1min間隔)	Pin43
#pragma interrupt (Excep_IRQ5(vect=69))
void Excep_IRQ5(void);							//	#EX_IRQ1/UARTcontroler1割込み		Pin42
#pragma interrupt (Excep_IRQ8(vect=72))
void Excep_IRQ8(void);							//	#EX_IRQ2/UARTcontroler2割込み		Pin141
#pragma interrupt (Excep_IRQ15(vect=79))
void Excep_IRQ15(void);							//	#ETHER_IRQ/Ethernetcontroler割込み	Pin98
/****************************************************************************/
#pragma interrupt (Excep_TPU0_TGI0A(vect=126))
void Excep_TPU0_TGI0A(void);					//	20msTimer割込み
#pragma interrupt (Excep_TPU1_TGI1A(vect=130))
void Excep_TPU1_TGI1A(void);					//	2msTimer割込み
#pragma interrupt (Excep_TPU2_TGI2A(vect=132))
void Excep_TPU2_TGI2A(void);					//	1msTimer割込み
#pragma interrupt (Excep_TPU3_TGI3A(vect=134))
void Excep_TPU3_TGI3A(void);					//	20msTimer割込み

/****************************************************************************/
#pragma interrupt (Excep_CAN1_RXF1(vect=52))
void Excep_CAN1_RXF1(void);						//	機器内通信		1Mbps
#pragma interrupt (Excep_CAN1_TXM1(vect=55))
void Excep_CAN1_TXM1(void);						//	機器内通信		1Mbps
#pragma interrupt (Excep_CANi_ERSi(vect=106))	//	機器内通信		1Mbps
void Excep_CANi_ERSi(void);
/****************************************************************************/
#pragma interrupt (Excep_RIIC1_EEI1(vect=186))
void Excep_RIIC1_EEI1(void);					//	RS232C通信Device制御	400kbps
#pragma interrupt (Excep_RIIC1_RXI1(vect=187))
void Excep_RIIC1_RXI1(void);					//	RS232C通信Device制御	400kbps
#pragma interrupt (Excep_RIIC1_TXI1(vect=188))
void Excep_RIIC1_TXI1(void);					//	RS232C通信Device制御	400kbps
#pragma interrupt (Excep_RIIC1_TEI1(vect=189))
void Excep_RIIC1_TEI1(void);					//	RS232C通信Device制御	400kbps
/****************************************************************************/
#pragma interrupt (Excep_SCI2_RXI2(vect=220))
void Excep_SCI2_RXI2(void);						//	JVMA(ｺｲﾝﾒｯｸ,紙幣ﾘｰﾀﾞ)	4,800bps
#pragma interrupt (Excep_SCI2_TXI2(vect=221))
void Excep_SCI2_TXI2(void);						//	JVMA(ｺｲﾝﾒｯｸ,紙幣ﾘｰﾀﾞ)	4,800bps
#pragma interrupt (Excep_SCI2_TEI2(vect=222))
void Excep_SCI2_TEI2(void);						//	JVMA(ｺｲﾝﾒｯｸ,紙幣ﾘｰﾀﾞ)	4,800bps
//#pragma interrupt (Excep_SCI2_ERI2(vect=xxx))
//void Excep_SCI2_ERI2(void);					//	JVMA(ｺｲﾝﾒｯｸ,紙幣ﾘｰﾀﾞ)	4,800bps
/****************************************************************************/
#pragma interrupt (Excep_SCI3_RXI3(vect=223))
void Excep_SCI3_RXI3(void);						//	磁気Reader制御			38,400bps
#pragma interrupt (Excep_SCI3_TXI3(vect=224))
void Excep_SCI3_TXI3(void);						//	磁気Reader制御			38,400bps
#pragma interrupt (Excep_SCI3_TEI3(vect=225))
void Excep_SCI3_TEI3(void);						//	磁気Reader制御			38,400bps
//#pragma interrupt (Excep_SCI3_ERI3(vect=xxx))
//void Excep_SCI3_ERI3(void);					//	磁気Reader制御			38,400bps
/****************************************************************************/
#pragma interrupt (Excep_SCI4_RXI4(vect=226))
void Excep_SCI4_RXI4(void);						//	Flap制御				38,400bps
#pragma interrupt (Excep_SCI4_TXI4(vect=227))
void Excep_SCI4_TXI4(void);						//	Flap制御				38,400bps
#pragma interrupt (Excep_SCI4_TEI4(vect=228))
void Excep_SCI4_TEI4(void);						//	Flap制御				38,400bps
//#pragma interrupt (Excep_SCI4_ERI4(vect=xxx))
//void Excep_SCI4_ERI4(void);					//	Flap制御				38,400bps
/****************************************************************************/
#pragma interrupt (Excep_SCI6_RXI6(vect=232))
void Excep_SCI6_RXI6(void);						//	Bluetooth				115,200bps
#pragma interrupt (Excep_SCI6_TXI6(vect=233))
void Excep_SCI6_TXI6(void);						//	Bluetooth				115,200bps
#pragma interrupt (Excep_SCI6_TEI6(vect=234))
void Excep_SCI6_TEI6(void);						//	Bluetooth				115,200bps
//#pragma interrupt (Excep_SCI6_ERI6(vect=xxx))
//void Excep_SCI6_ERI6(void);					//	Bluetooth				115,200bps
/****************************************************************************/
#pragma interrupt (Excep_SCI7_RXI7(vect=235))
void Excep_SCI7_RXI7(void);						//	serverFOMA_Rism			38.400bps
#pragma interrupt (Excep_SCI7_TXI7(vect=236))
void Excep_SCI7_TXI7(void);						//	serverFOMA_Rism			38.400bps
#pragma interrupt (Excep_SCI7_TEI7(vect=237))
void Excep_SCI7_TEI7(void);						//	serverFOMA_Rism			38.400bps
//#pragma interrupt (Excep_SCI7_ERI7(vect=xxx))
//void Excep_SCI7_ERI7(void);					//	serverFOMA_Rism			38.400bps
/****************************************************************************/
#pragma interrupt (Excep_SCI9_RXI9(vect=241))
void Excep_SCI9_RXI9(void);						//	NT-NET通信				38,400bps
#pragma interrupt (Excep_SCI9_TXI9(vect=242))
void Excep_SCI9_TXI9(void);						//	NT-NET通信				38,400bps
#pragma interrupt (Excep_SCI9_TEI9(vect=243))
void Excep_SCI9_TEI9(void);						//	NT-NET通信				38,400bps
//#pragma interrupt (Excep_SCI9_ERI9(vect=xxx))
//void Excep_SCI9_ERI9(void);					//	NT-NET通信				38,400bps
/****************************************************************************/
#pragma interrupt (Excep_SCI10_RXI10(vect=244))
void Excep_SCI10_RXI10(void);					//	非接触IC				38,400bps
#pragma interrupt (Excep_SCI10_TXI10(vect=245))
void Excep_SCI10_TXI10(void);					//	非接触IC				38,400bps
#pragma interrupt (Excep_SCI10_TEI10(vect=246))
void Excep_SCI10_TEI10(void);					//	非接触IC				38,400bps
//#pragma interrupt (Excep_SCI10_ERI10(vect=xxx))
//void Excep_SCI10_ERI10(void);					//	非接触IC				38,400bps
/****************************************************************************/
#pragma interrupt (Excep_SCI11_RXI11(vect=247))
void Excep_SCI11_RXI11(void);					//	MultiICcardR/W			38,400bps
#pragma interrupt (Excep_SCI11_TXI11(vect=248))
void Excep_SCI11_TXI11(void);					//	MultiICcardR/W			38,400bps
#pragma interrupt (Excep_SCI11_TEI11(vect=249))
void Excep_SCI11_TEI11(void);					//	MultiICcardR/W			38,400bps
//#pragma interrupt (Excep_SCI11_ERI11(vect=xxx))
//void Excep_SCI11_ERI11(void);					//	MultiICcardR/W			38,400bps
/****************************************************************************/
extern void PowerON_Reset_PC(void);				//	PowerON_Reset_PC
/****************************************************************************/
/*																			*/
/*	Unused Variable Interrupt Vector										*/
/*																			*/
/****************************************************************************/
#pragma interrupt (Excep_BRK(vect=0))
void Excep_BRK(void);
// vector  1 reserved
// vector  2 reserved
// vector  3 reserved
// vector  4 reserved
// vector  5 reserved
// vector  6 reserved
// vector  7 reserved
// vector  8 reserved
// vector  9 reserved
// vector 10 reserved
// vector 11 reserved
// vector 12 reserved
// vector 13 reserved
// vector 14 reserved
// vector 15 reserved
#pragma interrupt (Excep_BUSERR(vect=16))
void Excep_BUSERR(void);
// vector 17 reserved
// vector 18 reserved
// vector 19 reserved
// vector 20 reserved
#pragma interrupt (Excep_FCU_FCUERR(vect=21))
void Excep_FCU_FCUERR(void);
// vector 22 reserved
#pragma interrupt (Excep_FCU_FRDYI(vect=23))
void Excep_FCU_FRDYI(void);
// vector 24 reserved
// vector 25 reserved
// vector 26 reserved
#pragma interrupt (Excep_ICU_SWINT(vect=27))
void Excep_ICU_SWINT(void);
#pragma interrupt (Excep_CMTU0_CMT0(vect=28))
void Excep_CMTU0_CMT0(void);
#pragma interrupt (Excep_CMTU0_CMT1(vect=29))
void Excep_CMTU0_CMT1(void);
#pragma interrupt (Excep_CMTU1_CMT2(vect=30))
void Excep_CMTU1_CMT2(void);
#pragma interrupt (Excep_CMTU1_CMT3(vect=31))
void Excep_CMTU1_CMT3(void);
// vector 32 reserved
#pragma interrupt (Excep_USB0_D0FIFO0(vect=33))
void Excep_USB0_D0FIFO0(void);
#pragma interrupt (Excep_USB0_D1FIFO0(vect=34))
void Excep_USB0_D1FIFO0(void);
#pragma interrupt (Excep_USB0_USBI0(vect=35))
void Excep_USB0_USBI0(void);
// vector 36 reserved
// vector 37 reserved
// vector 38 reserved
#pragma interrupt (Excep_RSPI0_SPRI0(vect=39))
void Excep_RSPI0_SPRI0(void);
#pragma interrupt (Excep_RSPI0_SPTI0(vect=40))
void Excep_RSPI0_SPTI0(void);
#pragma interrupt (Excep_RSPI0_SPII0(vect=41))
void Excep_RSPI0_SPII0(void);
#pragma interrupt (Excep_RSPI1_SPRI1(vect=42))
void Excep_RSPI1_SPRI1(void);
#pragma interrupt (Excep_RSPI1_SPTI1(vect=43))
void Excep_RSPI1_SPTI1(void);
#pragma interrupt (Excep_RSPI1_SPII1(vect=44))
void Excep_RSPI1_SPII1(void);
#pragma interrupt (Excep_RSPI2_SPRI2(vect=45))
void Excep_RSPI2_SPRI2(void);
#pragma interrupt (Excep_RSPI2_SPTI2(vect=46))
void Excep_RSPI2_SPTI2(void);
#pragma interrupt (Excep_RSPI2_SPII2(vect=47))
void Excep_RSPI2_SPII2(void);
#pragma interrupt (Excep_CAN0_RXF0(vect=48))
void Excep_CAN0_RXF0(void);
#pragma interrupt (Excep_CAN0_TXF0(vect=49))
void Excep_CAN0_TXF0(void);
#pragma interrupt (Excep_CAN0_RXM0(vect=50))
void Excep_CAN0_RXM0(void);
#pragma interrupt (Excep_CAN0_TXM0(vect=51))
void Excep_CAN0_TXM0(void);
#pragma interrupt (Excep_CAN1_TXF1(vect=53))
void Excep_CAN1_TXF1(void);
#pragma interrupt (Excep_CAN1_RXM1(vect=54))
void Excep_CAN1_RXM1(void);
#pragma interrupt (Excep_CAN2_RXF2(vect=56))
void Excep_CAN2_RXF2(void);
#pragma interrupt (Excep_CAN2_TXF2(vect=57))
void Excep_CAN2_TXF2(void);
#pragma interrupt (Excep_CAN2_RXM2(vect=58))
void Excep_CAN2_RXM2(void);
#pragma interrupt (Excep_CAN2_TXM2(vect=59))
void Excep_CAN2_TXM2(void);
// vector 60 reserved
// vector 61 reserved
#pragma interrupt (Excep_RTC_COUNTUP(vect=62))
void Excep_RTC_COUNTUP(void);
// vector 63 reserved
#pragma interrupt (Excep_IRQ0(vect=64))
void Excep_IRQ0(void);
#pragma interrupt (Excep_IRQ1(vect=65))
void Excep_IRQ1(void);
#pragma interrupt (Excep_IRQ2(vect=66))
void Excep_IRQ2(void);
#pragma interrupt (Excep_IRQ3(vect=67))
void Excep_IRQ3(void);
#pragma interrupt (Excep_IRQ6(vect=70))
void Excep_IRQ6(void);
#pragma interrupt (Excep_IRQ7(vect=71))
void Excep_IRQ7(void);
#pragma interrupt (Excep_IRQ9(vect=73))
void Excep_IRQ9(void);
#pragma interrupt (Excep_IRQ10(vect=74))
void Excep_IRQ10(void);
#pragma interrupt (Excep_IRQ11(vect=75))
void Excep_IRQ11(void);
#pragma interrupt (Excep_IRQ12(vect=76))
void Excep_IRQ12(void);
#pragma interrupt (Excep_IRQ13(vect=77))
void Excep_IRQ13(void);
#pragma interrupt (Excep_IRQ14(vect=78))
void Excep_IRQ14(void);
// vector 80 reserved
// vector 81 reserved
// vector 82 reserved
// vector 83 reserved
// vector 84 reserved
// vector 85 reserved
// vector 86 reserved
// vector 87 reserved
// vector 88 reserved
// vector 89 reserved
#pragma interrupt (Excep_USB_USBR0(vect=90))
void Excep_USB_USBR0(void);
// vector 91 reserved
#pragma interrupt (Excep_RTC_ALARM(vect=92))
void Excep_RTC_ALARM(void);
#pragma interrupt (Excep_RTC_SLEEP(vect=93))
void Excep_RTC_SLEEP(void);
// vector 94 reserved
// vector 95 reserved
// vector 96 reserved
// vector 97 reserved
#pragma interrupt (Excep_AD0_ADI0(vect=98))
void Excep_AD0_ADI0(void);
// vector 99 reserved
// vector 100 reserved
// vector 101 reserved
#pragma interrupt (Excep_S12AD0_S12ADI0(vect=102))
void Excep_S12AD0_S12ADI0(void);
// vector 103 reserved
// vector 104 reserved
// vector 105 reserved
#pragma interrupt (Excep_ICU_GE1(vect=107))
void Excep_ICU_GE1(void);
#pragma interrupt (Excep_ICU_GE2(vect=108))
void Excep_ICU_GE2(void);
#pragma interrupt (Excep_ICU_GE3(vect=109))
void Excep_ICU_GE3(void);
#pragma interrupt (Excep_ICU_GE4(vect=110))
void Excep_ICU_GE4(void);
#pragma interrupt (Excep_ICU_GE5(vect=111))
void Excep_ICU_GE5(void);
#pragma interrupt (Excep_ICU_GE6(vect=112))
void Excep_ICU_GE6(void);
// vector 113 reserved
#pragma interrupt (Excep_ICU_GL0(vect=114))
void Excep_ICU_GL0(void);
// vector 115 reserved
// vector 116 reserved
// vector 117 reserved
// vector 118 reserved
// vector 119 reserved
// vector 120 reserved
// vector 121 reserved
#pragma interrupt (Excep_SCIX_SCIX0(vect=122))
void Excep_SCIX_SCIX0(void);
#pragma interrupt (Excep_SCIX_SCIX1(vect=123))
void Excep_SCIX_SCIX1(void);
#pragma interrupt (Excep_SCIX_SCIX2(vect=124))
void Excep_SCIX_SCIX2(void);
#pragma interrupt (Excep_SCIX_SCIX3(vect=125))
void Excep_SCIX_SCIX3(void);
#pragma interrupt (Excep_TPU0_TGI0B(vect=127))
void Excep_TPU0_TGI0B(void);
#pragma interrupt (Excep_TPU0_TGI0C(vect=128))
void Excep_TPU0_TGI0C(void);
#pragma interrupt (Excep_TPU0_TGI0D(vect=129))
void Excep_TPU0_TGI0D(void);
#pragma interrupt (Excep_TPU1_TGI1B(vect=131))
void Excep_TPU1_TGI1B(void);
#pragma interrupt (Excep_TPU2_TGI2B(vect=133))
void Excep_TPU2_TGI2B(void);
#pragma interrupt (Excep_TPU3_TGI3B(vect=135))
void Excep_TPU3_TGI3B(void);
#pragma interrupt (Excep_TPU3_TGI3C(vect=136))
void Excep_TPU3_TGI3C(void);
#pragma interrupt (Excep_TPU3_TGI3D(vect=137))
void Excep_TPU3_TGI3D(void);
#pragma interrupt (Excep_TPU4_TGI4A(vect=138))
void Excep_TPU4_TGI4A(void);
#pragma interrupt (Excep_TPU4_TGI4B(vect=139))
void Excep_TPU4_TGI4B(void);
#pragma interrupt (Excep_TPU5_TGI5A(vect=140))
void Excep_TPU5_TGI5A(void);
#pragma interrupt (Excep_TPU5_TGI5B(vect=141))
void Excep_TPU5_TGI5B(void);
#pragma interrupt (Excep_TPU6_TGI6A(vect=142))
void Excep_TPU6_TGI6A(void);
//#pragma interrupt (Excep_MTU0_TGIA0(vect=142))
//void Excep_MTU0_TGIA0(void);
#pragma interrupt (Excep_TPU6_TGI6B(vect=143))
void Excep_TPU6_TGI6B(void);
//#pragma interrupt (Excep_MTU0_TGIB0(vect=143))
//void Excep_MTU0_TGIB0(void);
#pragma interrupt (Excep_TPU6_TGI6C(vect=144))
void Excep_TPU6_TGI6C(void);
//#pragma interrupt (Excep_MTU0_TGIC0(vect=144))
//void Excep_MTU0_TGIC0(void);
#pragma interrupt (Excep_TPU6_TGI6D(vect=145))
void Excep_TPU6_TGI6D(void);
//#pragma interrupt (Excep_MTU0_TGID0(vect=145))
//void Excep_MTU0_TGID0(void);
#pragma interrupt (Excep_MTU0_TGIE0(vect=146))
void Excep_MTU0_TGIE0(void);
#pragma interrupt (Excep_MTU0_TGIF0(vect=147))
void Excep_MTU0_TGIF0(void);
#pragma interrupt (Excep_TPU7_TGI7A(vect=148))
void Excep_TPU7_TGI7A(void);
//#pragma interrupt (Excep_MTU1_TGIA1(vect=148))
//void Excep_MTU1_TGIA1(void);
#pragma interrupt (Excep_TPU7_TGI7B(vect=149))
void Excep_TPU7_TGI7B(void);
//#pragma interrupt (Excep_MTU1_TGIB1(vect=149))
//void Excep_MTU1_TGIB1(void);
#pragma interrupt (Excep_TPU8_TGI8A(vect=150))
void Excep_TPU8_TGI8A(void);
//#pragma interrupt (Excep_MTU2_TGIA2(vect=150))
//void Excep_MTU2_TGIA2(void);
#pragma interrupt (Excep_TPU8_TGI8B(vect=151))
void Excep_TPU8_TGI8B(void);
//#pragma interrupt (Excep_MTU2_TGIB2(vect=151))
//void Excep_MTU2_TGIB2(void);
#pragma interrupt (Excep_TPU9_TGI9A(vect=152))
void Excep_TPU9_TGI9A(void);
//#pragma interrupt (Excep_MTU3_TGIA3(vect=152))
//void Excep_MTU3_TGIA3(void);
#pragma interrupt (Excep_TPU9_TGI9B(vect=153))
void Excep_TPU9_TGI9B(void);
//#pragma interrupt (Excep_MTU3_TGIB3(vect=153))
//void Excep_MTU3_TGIB3(void);
#pragma interrupt (Excep_TPU9_TGI9C(vect=154))
void Excep_TPU9_TGI9C(void);
//#pragma interrupt (Excep_MTU3_TGIC3(vect=154))
//void Excep_MTU3_TGIC3(void);
#pragma interrupt (Excep_TPU9_TGI9D(vect=155))
void Excep_TPU9_TGI9D(void);
//#pragma interrupt (Excep_MTU3_TGID3(vect=155))
//void Excep_MTU3_TGID3(void);
#pragma interrupt (Excep_TPU10_TGI10A(vect=156))
void Excep_TPU10_TGI10A(void);
//#pragma interrupt (Excep_MTU4_TGIA4(vect=156))
//void Excep_MTU4_TGIA4(void);
#pragma interrupt (Excep_TPU10_TGI10B(vect=157))
void Excep_TPU10_TGI10B(void);
//#pragma interrupt (Excep_MTU4_TGIB4(vect=157))
//void Excep_MTU4_TGIB4(void);
#pragma interrupt (Excep_MTU4_TGIC4(vect=158))
void Excep_MTU4_TGIC4(void);
#pragma interrupt (Excep_MTU4_TGID4(vect=159))
void Excep_MTU4_TGID4(void);
#pragma interrupt (Excep_MTU4_TGIV4(vect=160))
void Excep_MTU4_TGIV4(void);
#pragma interrupt (Excep_MTU5_TGIU5(vect=161))
void Excep_MTU5_TGIU5(void);
#pragma interrupt (Excep_MTU5_TGIV5(vect=162))
void Excep_MTU5_TGIV5(void);
#pragma interrupt (Excep_MTU5_TGIW5(vect=163))
void Excep_MTU5_TGIW5(void);
#pragma interrupt (Excep_TPU11_TGI11A(vect=164))
void Excep_TPU11_TGI11A(void);
#pragma interrupt (Excep_TPU11_TGI11B(vect=165))
void Excep_TPU11_TGI11B(void);
#pragma interrupt (Excep_POE_OEI1(vect=166))
void Excep_POE_OEI1(void);
#pragma interrupt (Excep_POE_OEI2(vect=167))
void Excep_POE_OEI2(void);
// vector 168 reserved
// vector 169 reserved
#pragma interrupt (Excep_TMR0_CMIA0(vect=170))
void Excep_TMR0_CMIA0(void);
#pragma interrupt (Excep_TMR0_CMIB0(vect=171))
void Excep_TMR0_CMIB0(void);
#pragma interrupt (Excep_TMR0_OVI0(vect=172))
void Excep_TMR0_OVI0(void);
#pragma interrupt (Excep_TMR1_CMIA1(vect=173))
void Excep_TMR1_CMIA1(void);
#pragma interrupt (Excep_TMR1_CMIB1(vect=174))
void Excep_TMR1_CMIB1(void);
#pragma interrupt (Excep_TMR1_OVI1(vect=175))
void Excep_TMR1_OVI1(void);
#pragma interrupt (Excep_TMR2_CMIA2(vect=176))
void Excep_TMR2_CMIA2(void);
#pragma interrupt (Excep_TMR2_CMIB2(vect=177))
void Excep_TMR2_CMIB2(void);
#pragma interrupt (Excep_TMR2_OVI2(vect=178))
void Excep_TMR2_OVI2(void);
#pragma interrupt (Excep_TMR3_CMIA3(vect=179))
void Excep_TMR3_CMIA3(void);
#pragma interrupt (Excep_TMR3_CMIB3(vect=180))
void Excep_TMR3_CMIB3(void);
#pragma interrupt (Excep_TMR3_OVI3(vect=181))
void Excep_TMR3_OVI3(void);
#pragma interrupt (Excep_RIIC0_EEI0(vect=182))
void Excep_RIIC0_EEI0(void);
#pragma interrupt (Excep_RIIC0_RXI0(vect=183))
void Excep_RIIC0_RXI0(void);
#pragma interrupt (Excep_RIIC0_TXI0(vect=184))
void Excep_RIIC0_TXI0(void);
#pragma interrupt (Excep_RIIC0_TEI0(vect=185))
void Excep_RIIC0_TEI0(void);
#pragma interrupt (Excep_RIIC2_EEI2(vect=190))
void Excep_RIIC2_EEI2(void);
#pragma interrupt (Excep_RIIC2_RXI2(vect=191))
void Excep_RIIC2_RXI2(void);
#pragma interrupt (Excep_RIIC2_TXI2(vect=192))
void Excep_RIIC2_TXI2(void);
#pragma interrupt (Excep_RIIC2_TEI2(vect=193))
void Excep_RIIC2_TEI2(void);
#pragma interrupt (Excep_RIIC3_EEI3(vect=194))
void Excep_RIIC3_EEI3(void);
#pragma interrupt (Excep_RIIC3_RXI3(vect=195))
void Excep_RIIC3_RXI3(void);
#pragma interrupt (Excep_RIIC3_TXI3(vect=196))
void Excep_RIIC3_TXI3(void);
#pragma interrupt (Excep_RIIC3_TEI3(vect=197))
void Excep_RIIC3_TEI3(void);
#pragma interrupt (Excep_DMAC_DMAC0I(vect=198))
void Excep_DMAC_DMAC0I(void);
#pragma interrupt (Excep_DMAC_DMAC1I(vect=199))
void Excep_DMAC_DMAC1I(void);
#pragma interrupt (Excep_DMAC_DMAC2I(vect=200))
void Excep_DMAC_DMAC2I(void);
#pragma interrupt (Excep_DMAC_DMAC3I(vect=201))
void Excep_DMAC_DMAC3I(void);
// vector 202 reserved
// vector 203 reserved
// vector 204 reserved
// vector 205 reserved
// vector 206 reserved
// vector 207 reserved
// vector 208 reserved
// vector 209 reserved
// vector 210 reserved
// vector 211 reserved
// vector 212 reserved
// vector 213 reserved
#pragma interrupt (Excep_SCI0_RXI0(vect=214))
void Excep_SCI0_RXI0(void);
#pragma interrupt (Excep_SCI0_TXI0(vect=215))
void Excep_SCI0_TXI0(void);
#pragma interrupt (Excep_SCI0_TEI0(vect=216))
void Excep_SCI0_TEI0(void);
#pragma interrupt (Excep_SCI1_RXI1(vect=217))
void Excep_SCI1_RXI1(void);
#pragma interrupt (Excep_SCI1_TXI1(vect=218))
void Excep_SCI1_TXI1(void);
#pragma interrupt (Excep_SCI1_TEI1(vect=219))
void Excep_SCI1_TEI1(void);
#pragma interrupt (Excep_SCI5_RXI5(vect=229))
void Excep_SCI5_RXI5(void);
#pragma interrupt (Excep_SCI5_TXI5(vect=230))
void Excep_SCI5_TXI5(void);
#pragma interrupt (Excep_SCI5_TEI5(vect=231))
void Excep_SCI5_TEI5(void);
#pragma interrupt (Excep_SCI8_RXI8(vect=238))
void Excep_SCI8_RXI8(void);
#pragma interrupt (Excep_SCI8_TXI8(vect=239))
void Excep_SCI8_TXI8(void);
#pragma interrupt (Excep_SCI8_TEI8(vect=240))
void Excep_SCI8_TEI8(void);
#pragma interrupt (Excep_SCI12_RXI12(vect=250))
void Excep_SCI12_RXI12(void);
#pragma interrupt (Excep_SCI12_TXI12(vect=251))
void Excep_SCI12_TXI12(void);
#pragma interrupt (Excep_SCI12_TEI12(vect=252))
void Excep_SCI12_TEI12(void);
#pragma interrupt (Excep_IEB_IEBINT(vect=253))
void Excep_IEB_IEBINT(void);
// vector 254 reserved
// vector 255 reserved
/****************************************************************************/
#endif	// _VECT_H_
