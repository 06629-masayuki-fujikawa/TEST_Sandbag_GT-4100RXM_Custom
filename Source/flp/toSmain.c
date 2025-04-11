/*[]----------------------------------------------------------------------[]*/
/*|		New I/F to Salve Commnucation task on Master 					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"toS.h"
#include	"toSdata.h"


/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	toS_init( void );


/*----------------------------------*/
/*		external function define	*/
/*----------------------------------*/


/*[]----------------------------------------------------------------------[]*/
/*|             toS_init()  	                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|          toS task initial routine                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_init( void )
{
	/* �^�C�}�[��J�E���^�N���A */
	toS_TimerInit();				/* toStime.c �ɂ���B*/

	/* �ʐM�|�[�g�̏����� */
	toScom_Init();					/* toScom.c �ɂ���B*/

	/* ����ŃC���^�[�o���^�C�}���J�E���g���n�߂�B*/
	toS_f_TmStart = 1;				/* �J�E���g�J�n�t���O */
}

