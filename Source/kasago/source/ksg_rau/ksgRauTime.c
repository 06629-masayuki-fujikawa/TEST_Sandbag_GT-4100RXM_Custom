/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		通信監視用タイマー関数																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	ksgRauTime.c																			   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・通信監視用タイマー																					   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include	<string.h>															/*								*/
#include	<stdlib.h>															/*								*/
																				/*								*/
#include	"system.h"															/*								*/
#include	"ksgRauSci.h"														/*								*/
#include 	"ksg_def.h"
#include	"ksgRauModemData.h"													/* ﾓﾃﾞﾑ関連ﾃﾞｰﾀ定義			*/
																				
static	unsigned long	KSG_IRTime, KSG_MAXL;									/*							*/
static	unsigned int	KSG_x10ms_count;
static	unsigned int	KSG_x100ms_count;

/*[]-- プロトタイプ宣言 --------------------------------------------------------------------------------------[]*/
extern void	KSG_RauMdm10msInt( void );
extern void	KSG_RauMdm100msInt( void );

/*[]------------------------------------------------------------------------------------------[]*/
/*|	時間経過処理																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : KSG_GetInterval();														   |*/
/*| PARAMETER1 	 : unsigned	long	tim	:	比較時刻										   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| RETURN VALUE : unsigned long	経過時間 1ms単位										   |*/
/*[]------------------------------------------------------------------------------------------[]*/
unsigned long	KSG_GetInterval( unsigned long tim )
{																	/*							*/
	KSG_IRTime = KSG_Now_t;	KSG_MAXL = 1000000000L;					/* 現在時刻のﾊﾞｯｸｱｯﾌﾟ		*/
	if( KSG_IRTime < tim ) 	return((  KSG_MAXL - tim ) + KSG_IRTime);		/* 開始時刻が現在より大きい	*/
	else				return( KSG_IRTime - tim );					/* それ以外					*/
}																	/*							*/

void	KSG_RauIntervalTimer( void )								/*							*/
{																	/*							*/
	KSG_x10ms_count++;												/* 100ms用ｶｳﾝﾄ				*/
	if( ++KSG_Now_t > 999999999 ) 	KSG_Now_t = 0;					/* 現在時刻の加算			*/
	if( KSG_x10ms_count >= 10 )										/* ｶｳﾝﾄｱｯﾌﾟ?				*/
	{
		KSG_RauMdm10msInt();										/* 10msﾀｲﾏ処理				*/
																	/*							*/
		KSG_x100ms_count++;											/* 100ms用ｶｳﾝﾄ				*/
		if( KSG_x100ms_count >= 10 )								/* ｶｳﾝﾄｱｯﾌﾟ?				*/
		{															/*							*/
			KSG_RauMdm100msInt();									/* 100msﾀｲﾏ処理				*/
			KSG_x100ms_count = 0;									/* ⇒ｶｳﾝﾄﾘｾｯﾄ				*/
		}															/*							*/
		KSG_x10ms_count = 0;										/* ⇒ｶｳﾝﾄﾘｾｯﾄ				*/
	}
}
