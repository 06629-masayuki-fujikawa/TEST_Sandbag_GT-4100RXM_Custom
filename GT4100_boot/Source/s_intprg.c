//[]----------------------------------------------------------------------[]
///	@file		s_intprg.c
///	@brief		割り込み関数
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/

#include <machine.h>
#include "s_vect.h"
#pragma section IntPRG

/****************************************************/
/*  未使用時のダミー関数定義                        */
/****************************************************/
// BRK
void Excep_BRK(void){ wait(); }


// Exception(Supervisor Instruction)
#pragma section SuperViser 
void Excep_SuperVisorInst(void){/* brk(){  } */}

// Exception(Undefined Instruction)
#pragma section Undefine
void Excep_UndefinedInst(void){/* brk(){  } */}

// Exception(Floating Point)
#pragma section Floating
void Excep_FloatingPoint(void){/* brk(){  } */}

// NMI
#pragma section NmiPRG
void NonMaskableInterrupt(void){/* brk(){  } */}

// Dummy
#pragma section Dummyprg
void Dummy(void){/* brk(){  } */}

