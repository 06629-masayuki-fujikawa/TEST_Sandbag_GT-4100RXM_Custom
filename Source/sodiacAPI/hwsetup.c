/***********************************************************************/
/*                                                                     */
/*  FILE        :hwsetup.c                                             */
/*  DATE        :                                                      */
/*  DESCRIPTION :Hardware Setup file                                   */
/*  CPU TYPE    :RX630                                                 */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
/* June 07 , 2011  REA   For RSKRX630 with CAN API.*/

#include <machine.h>
#include "iodefine.h"

#ifdef __cplusplus
extern "C" {
#endif
extern void HardwareSetup(void);
#ifdef __cplusplus
}
#endif

static
void
RSPI_Setup( void )
{
	/* Disable interrupt requests */
	IEN(RSPI0,SPRI0)=0;
	IEN(RSPI0,SPTI0)=0;
	IEN(RSPI0,SPII0)=0;

	/* Enable SPI unit 0? */
	MSTP_RSPI0 = 0;

	/* Disable the channel */
	RSPI0.SPCR.BYTE = 0x00u;

	/* Load the channel registers */
	RSPI0.SPPCR.BYTE = 0x00u;

	RSPI0.SPBR.BYTE  = 0x00u;	/* 25Mbps */

	/* Buffer size 16bit */
	RSPI0.SPDCR.BYTE = 0x00u;
	RSPI0.SPDCR.BIT.SPLW = 0;

	RSPI0.SPCKD.BYTE = 0x00u;
	RSPI0.SSLND.BYTE = 0x00u;
	RSPI0.SPND.BYTE  = 0x00u;
	RSPI0.SPCR2.BYTE = 0x00u;
	RSPI0.SPSCR.BYTE = 0x00u;
	RSPI0.SSLP.BYTE  = 0x00u;

	/* Connection mode */
	RSPI0.SPCR.BIT.MSTR  = 1;
	RSPI0.SPCR.BIT.SPMS  = 1;
	RSPI0.SPCR.BIT.SPTIE = 0;
	RSPI0.SPCR.BIT.SPRIE = 1;	/* 受信割り込み有効 */

	/* SSL assertion */
	RSPI0.SPCMD0.BIT.SSLA = 0;	/* SSL0使用 */

	/* Frame data length */
	RSPI0.SPCMD0.BIT.SPB  = 7;	/* 8bit */
	RSPI0.SPCMD0.BIT.CPHA = 1;	/* SPI mode3 */
	RSPI0.SPCMD0.BIT.CPOL = 1;
	RSPI0.SPCMD0.BIT.BRDV = 0;	/* 25Mbps */

	/* SPIポートの設定 */
	/* 設定するポートをまず汎用入力に */
	PORTA.PMR.BIT.B6 = 0;
	PORTA.PMR.BIT.B7 = 0;

	MPC.PWPR.BIT.B0WI  = 0;		/* PFSWEの書き換え許可 */
	MPC.PWPR.BIT.PFSWE = 1;		/* PFSの書き換え許可 */
	MPC.PWPR.BIT.B0WI  = 1;		/* PFSWEの書き換え禁止 */

	/* 機能選択 */
	MPC.PA5PFS.BIT.PSEL = 0x0D;	/* PA5->RSPCKA */
	MPC.PA6PFS.BIT.PSEL = 0x0D;	/* PA6->MOSIA */
	MPC.PA7PFS.BIT.PSEL = 0x0D;	/* PA7->MISOA */

	MPC.PWPR.BIT.B0WI  = 0;		/* PFSWEの書き換え許可 */
	MPC.PWPR.BIT.PFSWE = 0;		/* PFSの書き換え禁止 */
	MPC.PWPR.BIT.B0WI  = 1;		/* PFSWEの書き換え禁止 */

	/* 機能実行 */
	PORTA.PMR.BIT.B5 = 1;		/* RSPCKA */
	PORTA.PMR.BIT.B6 = 1;		/* MOSIA */
	PORTA.PMR.BIT.B7 = 1;		/* MISOA */

	/* CS */
	PORTA.PDR.BIT.B4 = 1;		/* 出力 */
	PORTA.PMR.BIT.B4 = 0;
	PORTA.PODR.BIT.B4 = 1;

	IPR(RSPI0,SPRI0)=15;

	RSPI0.SPCR.BIT.SPE = 1;		/* 起動 */
}

/******************************************************************************
Function Name:       HardwareSetup
Parameters:          -
Return value:        -
Description:         IO and peripheral control.
******************************************************************************/
void HardwareSetup(void)
{
	RSPI_Setup();

}

/* EOF */
