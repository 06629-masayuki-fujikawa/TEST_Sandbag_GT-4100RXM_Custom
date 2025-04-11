/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
#include	"vect.h"
#include	"system.h"
#include	"mem_def.h"

extern void PowerON_Reset_PC(void);
extern void NonMaskableInterrupt(void);
extern void SodiacInterPWM(void);

void Unkown_vect( void );
/****************************************************************************/
/*																			*/
/*	Unused_Variable_Interrupt												*/
/*																			*/
/****************************************************************************/
void Unkown_vect( void )
{
	FLAGPT.event_CtrlBitData.BIT.UNKOWN_VECT = 1;
}

//#pragma interrupt (Excep_BRK)
void Excep_BRK( void )
{ 
	Unkown_vect();
}
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
//#pragma interrupt (Excep_BUSERR))
void Excep_BUSERR( void )
{ 
	Unkown_vect();
}
// vector 17 reserved
// vector 18 reserved
// vector 19 reserved
// vector 20 reserved
//#pragma interrupt (Excep_FCU_FCUERR)
void Excep_FCU_FCUERR( void )
{ 
	Unkown_vect();
}
// vector 22 reserved
//#pragma interrupt (Excep_FCU_FRDYI)
void Excep_FCU_FRDYI( void )
{ 
	Unkown_vect();
}
// vector 24 reserved
// vector 25 reserved
// vector 26 reserved
//#pragma interrupt (Excep_ICU_SWINT)
void Excep_ICU_SWINT( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CMTU0_CMT0)

//#pragma interrupt (Excep_CMTU0_CMT1)
void Excep_CMTU0_CMT1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CMTU1_CMT2)
void Excep_CMTU1_CMT2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CMTU1_CMT3)
void Excep_CMTU1_CMT3( void )
{ 
	Unkown_vect();
}
// vector 32 reserved
//#pragma interrupt (Excep_USB0_D0FIFO0)
void Excep_USB0_D0FIFO0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_USB0_D1FIFO0)
void Excep_USB0_D1FIFO0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_USB0_USBI0)
void Excep_USB0_USBI0( void )
{ 
	Unkown_vect();
}
// vector 36 reserved
// vector 37 reserved
// vector 38 reserved
//#pragma interrupt (Excep_RSPI0_SPTI0)
void Excep_RSPI0_SPTI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RSPI0_SPII0)
void Excep_RSPI0_SPII0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RSPI1_SPRI1)
void Excep_RSPI1_SPRI1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RSPI1_SPTI1)
void Excep_RSPI1_SPTI1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RSPI1_SPII1)
void Excep_RSPI1_SPII1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RSPI2_SPRI2)
void Excep_RSPI2_SPRI2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RSPI2_SPTI2)
void Excep_RSPI2_SPTI2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RSPI2_SPII2)
void Excep_RSPI2_SPII2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN0_RXF0)
void Excep_CAN0_RXF0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN0_TXF0)
void Excep_CAN0_TXF0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN0_RXM0)
void Excep_CAN0_RXM0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN0_TXM0)
void Excep_CAN0_TXM0( void )
{ 
	Unkown_vect();
}
void Excep_CAN1_TXF1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN1_RXM1)
void Excep_CAN1_RXM1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN2_RXF2)
void Excep_CAN2_RXF2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN2_TXF2)
void Excep_CAN2_TXF2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN2_RXM2)
void Excep_CAN2_RXM2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_CAN2_TXM2)
void Excep_CAN2_TXM2( void )
{ 
	Unkown_vect();
}
// vector 60 reserved
// vector 61 reserved
//#pragma interrupt (Excep_RTC_COUNTUP)
void Excep_RTC_COUNTUP( void )
{ 
	Unkown_vect();
}
// vector 63 reserved
//#pragma interrupt (Excep_IRQ0)
void Excep_IRQ0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ1)
void Excep_IRQ1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ2)
void Excep_IRQ2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ3)
void Excep_IRQ3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ6)
void Excep_IRQ6( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ7)
void Excep_IRQ7( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ9)
void Excep_IRQ9( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ10)
void Excep_IRQ10( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ11)
void Excep_IRQ11( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ12)
void Excep_IRQ12( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ13)
void Excep_IRQ13( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IRQ14)
void Excep_IRQ14( void )
{ 
	Unkown_vect();
}
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
//#pragma interrupt (Excep_USB_USBR0)
void Excep_USB_USBR0( void )
{ 
	Unkown_vect();
}
// vector 91 reserved
//#pragma interrupt (Excep_RTC_ALARM)
void Excep_RTC_ALARM( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RTC_SLEEP)
void Excep_RTC_SLEEP( void )
{ 
	Unkown_vect();
}
// vector 94 reserved
// vector 95 reserved
// vector 96 reserved
// vector 97 reserved
//#pragma interrupt (Excep_AD0_ADI0)
void Excep_AD0_ADI0( void )
{ 
	Unkown_vect();
}
// vector 99 reserved
// vector 100 reserved
// vector 101 reserved
//#pragma interrupt (Excep_S12AD0_S12ADI0)
void Excep_S12AD0_S12ADI0( void )
{ 
	Unkown_vect();
}
// vector 103 reserved
// vector 104 reserved
// vector 105 reserved
//#pragma interrupt (Excep_ICU_GE1)
void Excep_ICU_GE1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_ICU_GE2)
void Excep_ICU_GE2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_ICU_GE3)
void Excep_ICU_GE3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_ICU_GE4)
void Excep_ICU_GE4( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_ICU_GE5)
void Excep_ICU_GE5( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_ICU_GE6)
void Excep_ICU_GE6( void )
{ 
	Unkown_vect();
}
// vector 113 reserved
//#pragma interrupt (Excep_ICU_GL0)
// vector 115 reserved
// vector 116 reserved
// vector 117 reserved
// vector 118 reserved
// vector 119 reserved
// vector 120 reserved
// vector 121 reserved
//#pragma interrupt (Excep_SCIX_SCIX0)
void Excep_SCIX_SCIX0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCIX_SCIX1)
void Excep_SCIX_SCIX1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCIX_SCIX2)
void Excep_SCIX_SCIX2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCIX_SCIX3)
void Excep_SCIX_SCIX3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU0_TGI0B)
void Excep_TPU0_TGI0B( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU0_TGI0C)
void Excep_TPU0_TGI0C( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU0_TGI0D)
void Excep_TPU0_TGI0D( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU1_TGI1B)
void Excep_TPU1_TGI1B( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU2_TGI2B)
void Excep_TPU2_TGI2B( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU3_TGI3B)
void Excep_TPU3_TGI3B( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU3_TGI3C)
void Excep_TPU3_TGI3C( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU3_TGI3D)
void Excep_TPU3_TGI3D( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU4_TGI4A)
void Excep_TPU4_TGI4A( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU4_TGI4B)
void Excep_TPU4_TGI4B( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU5_TGI5A)
void Excep_TPU5_TGI5A( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU5_TGI5B)
void Excep_TPU5_TGI5B( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU6_TGI6A)
void Excep_TPU6_TGI6A( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU0_TGIA0)
//void Excep_MTU0_TGIA0( void ) { }
//#pragma interrupt (Excep_TPU6_TGI6B)
void Excep_TPU6_TGI6B( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU0_TGIB0)
//void Excep_MTU0_TGIB0( void ) { }
//#pragma interrupt (Excep_TPU6_TGI6C)
void Excep_TPU6_TGI6C( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU0_TGIC0)
//void Excep_MTU0_TGIC0( void ) { }
//#pragma interrupt (Excep_TPU6_TGI6D)
void Excep_TPU6_TGI6D( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU0_TGID0)
//void Excep_MTU0_TGID0( void ) { }
//#pragma interrupt (Excep_MTU0_TGIE0)
void Excep_MTU0_TGIE0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_MTU0_TGIF0)
void Excep_MTU0_TGIF0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU7_TGI7A)
void Excep_TPU7_TGI7A( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU1_TGIA1)
//void Excep_MTU1_TGIA1( void ) { }
//#pragma interrupt (Excep_TPU7_TGI7B)
void Excep_TPU7_TGI7B( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU1_TGIB1)
//void Excep_MTU1_TGIB1( void ) { }
//#pragma interrupt (Excep_TPU8_TGI8A)
void Excep_TPU8_TGI8A( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU2_TGIA2)
//void Excep_MTU2_TGIA2( void ) { }
//#pragma interrupt (Excep_TPU8_TGI8B)
void Excep_TPU8_TGI8B( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU2_TGIB2)
//void Excep_MTU2_TGIB2( void ) { }
//#pragma interrupt (Excep_TPU9_TGI9A)
void Excep_TPU9_TGI9A( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU3_TGIA3)
//void Excep_MTU3_TGIA3( void ) { }
//#pragma interrupt (Excep_TPU9_TGI9B)
void Excep_TPU9_TGI9B( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU3_TGIB3)
//void Excep_MTU3_TGIB3( void ) { }
//#pragma interrupt (Excep_TPU9_TGI9C)
void Excep_TPU9_TGI9C( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU3_TGIC3)
//void Excep_MTU3_TGIC3( void ) { }
//#pragma interrupt (Excep_TPU9_TGI9D)
void Excep_TPU9_TGI9D( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU3_TGID3)
//void Excep_MTU3_TGID3( void ) { }
//#pragma interrupt (Excep_TPU10_TGI10A)
void Excep_TPU10_TGI10A( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU4_TGIA4)
//void Excep_MTU4_TGIA4( void ) { }
//#pragma interrupt (Excep_TPU10_TGI10B)
void Excep_TPU10_TGI10B( void )
{ 
	Unkown_vect();
}
////#pragma interrupt (Excep_MTU4_TGIB4)
//void Excep_MTU4_TGIB4( void ) { }
//#pragma interrupt (Excep_MTU4_TGIC4)
void Excep_MTU4_TGIC4( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_MTU4_TGID4)
void Excep_MTU4_TGID4( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_MTU4_TGIV4)
void Excep_MTU4_TGIV4( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_MTU5_TGIU5)
void Excep_MTU5_TGIU5( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_MTU5_TGIV5)
void Excep_MTU5_TGIV5( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_MTU5_TGIW5)
void Excep_MTU5_TGIW5( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU11_TGI11A)
void Excep_TPU11_TGI11A( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TPU11_TGI11B)
void Excep_TPU11_TGI11B( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_POE_OEI1)
void Excep_POE_OEI1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_POE_OEI2)
void Excep_POE_OEI2( void )
{ 
	Unkown_vect();
}
// vector 168 reserved
// vector 169 reserved
//#pragma interrupt (Excep_TMR0_CMIA0)
void Excep_TMR0_CMIA0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR0_CMIB0)
void Excep_TMR0_CMIB0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR0_OVI0)
void Excep_TMR0_OVI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR1_CMIA1)
void Excep_TMR1_CMIA1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR1_CMIB1)
void Excep_TMR1_CMIB1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR1_OVI1)
void Excep_TMR1_OVI1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR2_CMIA2)
void Excep_TMR2_CMIA2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR2_CMIB2)
void Excep_TMR2_CMIB2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR2_OVI2)
void Excep_TMR2_OVI2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR3_CMIA3)
void Excep_TMR3_CMIA3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR3_CMIB3)
void Excep_TMR3_CMIB3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_TMR3_OVI3)
void Excep_TMR3_OVI3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC0_EEI0)
void Excep_RIIC0_EEI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC0_RXI0)
void Excep_RIIC0_RXI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC0_TXI0)
void Excep_RIIC0_TXI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC0_TEI0)
void Excep_RIIC0_TEI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC2_EEI2)
void Excep_RIIC2_EEI2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC2_RXI2)
void Excep_RIIC2_RXI2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC2_TXI2)
void Excep_RIIC2_TXI2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC2_TEI2)
void Excep_RIIC2_TEI2( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC3_EEI3)
void Excep_RIIC3_EEI3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC3_RXI3)
void Excep_RIIC3_RXI3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC3_TXI3)
void Excep_RIIC3_TXI3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_RIIC3_TEI3)
void Excep_RIIC3_TEI3( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_DMAC_DMAC0I)
void Excep_DMAC_DMAC0I( void )
{ 
	SodiacInterPWM();
}
//#pragma interrupt (Excep_DMAC_DMAC1I)
void Excep_DMAC_DMAC1I( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_DMAC_DMAC2I)
void Excep_DMAC_DMAC2I( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_DMAC_DMAC3I)
void Excep_DMAC_DMAC3I( void )
{ 
	Unkown_vect();
}
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
//#pragma interrupt (Excep_SCI0_RXI0)
void Excep_SCI0_RXI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI0_TXI0)
void Excep_SCI0_TXI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI0_TEI0)
void Excep_SCI0_TEI0( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI1_RXI1)
void Excep_SCI1_RXI1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI1_TXI1)
void Excep_SCI1_TXI1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI1_TEI1)
void Excep_SCI1_TEI1( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI5_RXI5)
void Excep_SCI5_RXI5( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI5_TXI5)
void Excep_SCI5_TXI5( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI5_TEI5)
void Excep_SCI5_TEI5( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI8_RXI8)
void Excep_SCI8_RXI8( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI8_TXI8)
void Excep_SCI8_TXI8( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI8_TEI8)
void Excep_SCI8_TEI8( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI12_RXI12)
void Excep_SCI12_RXI12( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI12_TXI12)
void Excep_SCI12_TXI12( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_SCI12_TEI12)
void Excep_SCI12_TEI12( void )
{ 
	Unkown_vect();
}
//#pragma interrupt (Excep_IEB_IEBINT)
void Excep_IEB_IEBINT( void )
{ 
	Unkown_vect();
}
// vector 254 reserved
// vector 255 reserved

// 固定ベクタテーブルに配置されるプログラムをブートプログラムと同じ配置にするためにここで定義する
#pragma section SuperViser 
#pragma interrupt (Excep_SuperVisorInst)
void Excep_SuperVisorInst(void)
{
	FLAGPT.event_CtrlBitData.BIT.ILLEGAL_SUPERVISOR_INST = 1;
}

#pragma section Undefine
#pragma interrupt (Excep_UndefinedInst)
void Excep_UndefinedInst(void)
{
	FLAGPT.event_CtrlBitData.BIT.ILLEGAL_INSTRUCTION = 1;
}

#pragma section Floating
#pragma interrupt (Excep_FloatingPoint)
void Excep_FloatingPoint(void)
{
	FLAGPT.event_CtrlBitData.BIT.ILLEGAL_FLOATINGPOINT = 1;
}

#pragma section Dummyprg
#pragma interrupt (Dummy)
void Dummy(void){/* brk(); */}

#pragma section C FIXEDVECT
/****************************************************************************/
/*																			*/
/*	Interrupt Fixed Vector Table											*/
/*																			*/
/****************************************************************************/
void* const Fixed_Vectors[] = {
    (void*) Excep_SuperVisorInst,	//;0xffffffd0  Exception(Supervisor Instruction)
    Dummy,							//;0xffffffd4  Reserved
    Dummy,							//;0xffffffd8  Reserved
    (void*) Excep_UndefinedInst,	//;0xffffffdc  Exception(Undefined Instruction)
    Dummy,							//;0xffffffe0  Reserved
    (void*) Excep_FloatingPoint,	//;0xffffffe4  Exception(Floating Point)
    Dummy,							//;0xffffffe8  Reserved
    Dummy,							//;0xffffffec  Reserved
    Dummy,							//;0xfffffff0  Reserved
    Dummy,							//;0xfffffff4  Reserved
    (void*) NonMaskableInterrupt,	//;0xfffffff8  NMI
	PowerON_Reset_PC				//;0xfffffffc  RESET
};
