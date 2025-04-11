//[]----------------------------------------------------------------------[]
///	@file		FT4000_boot.c
///	@brief		全割り込みベクタの参照関数定義 
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

#pragma interrupt (Excep_SuperVisorInst)			// Exception(Supervisor Instruction)
#pragma interrupt (Excep_UndefinedInst)				// Exception(Undefined Instruction)
#pragma interrupt (Excep_FloatingPoint)				// Exception(Floating Point)
#pragma interrupt (NonMaskableInterrupt)			// NMI
#pragma interrupt (Dummy)							// Dummy
#pragma interrupt (Excep_BRK(vect=0))				// BRK

/*****************************************************
/*  プロトタイプ宣言                                 *
/****************************************************/
void Excep_SuperVisorInst(void);
void Excep_UndefinedInst(void);
void Excep_FloatingPoint(void);
void NonMaskableInterrupt(void);
void Dummy(void);
void Excep_BRK(void);

//;<<VECTOR DATA START (POWER ON RESET)>>
//;Power On Reset PC
extern void PowerON_Reset_PC(void);                                                                                                                
//;<<VECTOR DATA END (POWER ON RESET)>>

