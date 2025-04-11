//[]----------------------------------------------------------------------[]
///	@file		s_memdata.c
///	@brief		ブート領域メモリ定義
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#include <machine.h>
#include "iodefine.h" 
#include "s_memdata.h"

const unsigned char prog_update_flag[4] = {'P','R','O','G'};

/*--- Pragma --------------------*/
/*------------------------------------------------------------------------------*/
#pragma	section	_BOOTINFO		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:アドレス空間0x06000000-0x060000FF
// この領域はメインプログラムと同じで、以下の構造体も同じものとする
SWITCH_DATA	BootInfo;						// 起動時面選択情報

