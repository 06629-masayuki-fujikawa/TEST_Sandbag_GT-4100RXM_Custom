/*[]----------------------------------------------------------------------[]*/
/*|		RAM/SRAM Memory Data on Lock task in New I/F(Master)			   |*/
/*|			�E�����ɂ́A�VI/F�Ձi�e�@�j���́u���b�N���u�Ǘ��v�^�X�N��	   |*/
/*|			  �Q�Ƃ���̈���`���܂��B								   |*/
/*|			�ECPU����RAM�ƁA�O�t��S-RAM�����݂��Ă��܂��B				   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _LCM_H_
#define _LCM_H_

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/


/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/


/*----------------------------------*/
/*		function external			*/
/*----------------------------------*/
/* LCMmain.c */
extern	void	LCM_init(void);
extern	void	LCM_main(void);
extern	void	PAYcom_InitDataSave(void);



/* LCMtime.c */
extern	void	LCM_TimerInit(void);
extern	void	LCM_10msInt(void);
extern	void	LCM_1secInt(void);
extern	void	LCMtim_10msTimerStart(int iTerm, int iLock, ushort TimVal);
extern	void	LCMtim_10msTimerStop(int iTerm, int iLock);
extern	uchar	LCMtim_10msTimeout(int iTerm, int iLock);
extern	void	LCMtim_1secTimerStart(int iTerm, int iLock, ushort TimVal);
extern	void	LCMtim_1secTimerStop(int iTerm, int iLock);
extern	uchar	LCMtim_1secTimeout(int iTerm, int iLock);
extern	void	LCMtim_1secWaitStart(ushort TimVal);
extern	void	LCMtim_1secWaitStop(void);
extern	uchar	LCMtim_1secWaitTimeout(void);
extern	BOOL	IFM_Can_Action(char type);
extern	BOOL	LCM_CanFlapCommand(ushort index, uchar direction);

#endif	// _LCM_H_

