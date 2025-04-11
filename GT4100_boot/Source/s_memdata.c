//[]----------------------------------------------------------------------[]
///	@file		s_memdata.c
///	@brief		�u�[�g�̈惁������`
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
// CS2:�A�h���X���0x06000000-0x060000FF
// ���̗̈�̓��C���v���O�����Ɠ����ŁA�ȉ��̍\���̂��������̂Ƃ���
SWITCH_DATA	BootInfo;						// �N�����ʑI�����

