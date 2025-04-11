//[]----------------------------------------------------------------------[]
///	@file		s_vecttbl.c
///	@brief		固定ベクタ・テーブル
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/
#include "s_vect.h"

/*--- Pragma --------------------*/
#pragma section C FIXEDVECT

void (*const Fixed_Vectors[])(void) = {
    Excep_SuperVisorInst,			//;0xffffffd0  Exception(Supervisor Instruction)
    Dummy,							//;0xffffffd4  Reserved
    Dummy,							//;0xffffffd8  Reserved
    Excep_UndefinedInst,			//;0xffffffdc  Exception(Undefined Instruction)
    Dummy,							//;0xffffffe0  Reserved
    Excep_FloatingPoint,			//;0xffffffe4  Exception(Floating Point)
    Dummy,							//;0xffffffe8  Reserved
    Dummy,							//;0xffffffec  Reserved
    Dummy,							//;0xfffffff0  Reserved
    Dummy,							//;0xfffffff4  Reserved
    NonMaskableInterrupt,			//;0xfffffff8  NMI
//;<<VECTOR DATA START (POWER ON RESET)>>
    PowerON_Reset_PC				//;0xfffffffc  RESET (Power On Reset PC)
//;<<VECTOR DATA END (POWER ON RESET)>>
};

