//[]----------------------------------------------------------------------[]
///	@file		s_resetprg.c
///	@brief		パワーオン・リセット処理
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/
#include	<machine.h>
#include	<_h_c_lib.h>
#include	"typedefine.h"		// Define Types

/*--- Pragma --------------------*/
#pragma stacksize su=0x300      	/* ユーザスタックサイズ設定 */
#pragma stacksize si=0x100  		/* 割り込みスタックサイズ設定 */

/*--- Prototype -----------------*/
void PowerON_Reset_PC(void);

/*--- Define --------------------*/
#define PSW_init  0x00010000	// PSW bit pattern
#define FPSW_init 0x00000000	// FPSW bit base pattern
#define _DENOM 0x00000100		// Let FPSW DNbit=1 (denormal as zero)

/*--- Gloval Value --------------*/

/*--- Extern --------------------*/
extern void cpu_init( void );
extern void port_init( void );
extern void syscr_init( void );
extern void main(void);


#pragma section ResetPRG		// output PowerON_Reset to PResetPRG section

#pragma entry PowerON_Reset_PC

void PowerON_Reset_PC(void)
{ 

	set_fpsw(FPSW_init | _DENOM);

	_INITSCT();
	syscr_init();
	cpu_init();
	port_init();
	Rspi_from_Init();

    nop();

	set_psw(PSW_init);				// Set Ubit & Ibit for PSW
	main();

	brk();
}

