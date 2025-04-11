/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		�ʐM�Ď��p�^�C�}�[�֐�																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	ksgRauTime.c																			   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �E�ʐM�Ď��p�^�C�}�[																					   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include	<string.h>															/*								*/
#include	<stdlib.h>															/*								*/
																				/*								*/
#include	"system.h"															/*								*/
#include	"ksgRauSci.h"														/*								*/
#include 	"ksg_def.h"
#include	"ksgRauModemData.h"													/* ���ъ֘A�ް���`			*/
																				
static	unsigned long	KSG_IRTime, KSG_MAXL;									/*							*/
static	unsigned int	KSG_x10ms_count;
static	unsigned int	KSG_x100ms_count;

/*[]-- �v���g�^�C�v�錾 --------------------------------------------------------------------------------------[]*/
extern void	KSG_RauMdm10msInt( void );
extern void	KSG_RauMdm100msInt( void );

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���Ԍo�ߏ���																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : KSG_GetInterval();														   |*/
/*| PARAMETER1 	 : unsigned	long	tim	:	��r����										   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| RETURN VALUE : unsigned long	�o�ߎ��� 1ms�P��										   |*/
/*[]------------------------------------------------------------------------------------------[]*/
unsigned long	KSG_GetInterval( unsigned long tim )
{																	/*							*/
	KSG_IRTime = KSG_Now_t;	KSG_MAXL = 1000000000L;					/* ���ݎ������ޯ�����		*/
	if( KSG_IRTime < tim ) 	return((  KSG_MAXL - tim ) + KSG_IRTime);		/* �J�n���������݂��傫��	*/
	else				return( KSG_IRTime - tim );					/* ����ȊO					*/
}																	/*							*/

void	KSG_RauIntervalTimer( void )								/*							*/
{																	/*							*/
	KSG_x10ms_count++;												/* 100ms�p����				*/
	if( ++KSG_Now_t > 999999999 ) 	KSG_Now_t = 0;					/* ���ݎ����̉��Z			*/
	if( KSG_x10ms_count >= 10 )										/* ���ı���?				*/
	{
		KSG_RauMdm10msInt();										/* 10ms��Ϗ���				*/
																	/*							*/
		KSG_x100ms_count++;											/* 100ms�p����				*/
		if( KSG_x100ms_count >= 10 )								/* ���ı���?				*/
		{															/*							*/
			KSG_RauMdm100msInt();									/* 100ms��Ϗ���				*/
			KSG_x100ms_count = 0;									/* �˶���ؾ��				*/
		}															/*							*/
		KSG_x10ms_count = 0;										/* �˶���ؾ��				*/
	}
}
