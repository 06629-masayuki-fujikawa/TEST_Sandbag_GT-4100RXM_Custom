/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		設定項目参照																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	param.c																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・ｾｯｼｮﾝﾊﾟﾗﾒｰﾀの参照																						   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
																				/*								*/
#include 	<string.h>															/*								*/
																				/*								*/
#include	"system.h"															/*								*/
#include	"rkn_def.h"															/* OK/NGのため					*/
#include	"prm_tbl.h"															/*								*/
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"LKmain.h"
#include	"AppServ.h"
#include	"mnt_def.h"
#include	"suica_def.h"
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*		関数宣言															 									*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
long	prm_get( char, short, short, char, char );								/* 共通/固有ﾊﾟﾗﾒｰﾀの取得		*/
long	prm_tim( char, short, short );											/* 共通/固有ﾊﾟﾗﾒｰﾀ時間の取得	*/
void	prm_init( char );														/* 共通/固有ﾊﾟﾗﾒｰﾀｱﾄﾞﾚｽｾｯﾄ		*/
void	prm_clr( char, char, char );											/* 共通/固有ﾊﾟﾗﾒｰﾀｸﾘｱ/ﾃﾞﾌｫﾙﾄ	*/
ulong	prm_svtim( ushort, uchar );												/* ｻｰﾋﾞｽﾀｲﾑ取得					*/
																				/* 								*/
																				/* 								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ﾊﾟﾗﾒｰﾀ変換（汎用）																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_get(par1,par2,par3,par4,par5)															   |*/
/*| PARAMETER	: par1	: ﾊﾟﾗﾒｰﾀ種類 0=共通/1=固有															   |*/
/*|				: par2	: ﾊﾟﾗﾒｰﾀｾｯｼｮﾝNo.																	   |*/
/*| 			: par3	: ｱﾄﾞﾚｽ																				   |*/
/*| 			: par4	: 桁数(何桁取得するか)																   |*/
/*| 			: par5	: 位置(何桁目から取得するか 位置は654321とする)										   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
long	prm_get(																/*								*/
		char	kin,															/* ﾊﾟﾗﾒｰﾀ種類：0=共通/1=個別	*/
		short	ses,															/* ｾｯｼｮﾝNo.						*/
		short	adr,															/* ｱﾄﾞﾚｽNo.						*/
		char	len,															/* 取得桁数						*/
		char	pos	)															/* 桁位置						*/
{																				/*								*/
	long	prm_wk;																/* ﾊﾟﾗﾒｰﾀ変換ﾜｰｸ				*/
	long	result	= -1;														/*								*/
	char	chk		= NG;														/*								*/
																				/*								*/
	static	const long arr[] = { 0L,1L,10L,100L,1000L,10000L,100000L };			/* 								*/
																				/*								*/
	switch( len )																/* 取得桁数により分岐			*/
	{																			/*								*/
	case 1:																		/* 1桁取得						*/
		if(pos>6){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 2:																		/* 2桁取得						*/
		if(pos>5){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 3:																		/* 3桁取得						*/
		if(pos>4){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 4:																		/* 4桁取得						*/
		if(pos>3){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 5:																		/* 5桁取得						*/
		if(pos>2){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 6:																		/* 6桁取得						*/
		if(pos>1){																/*								*/
			break;																/*								*/
		}																		/*								*/
		if( kin == PEC_PRM )													/* 固有ﾊﾟﾗﾒｰﾀ					*/
			result = PPrmSS[ses][adr];											/*								*/
		else{																	/* 共通ﾊﾟﾗﾒｰﾀ					*/
			result = CPrmSS[ses][adr];											/*								*/
			prm_invalid_change( ses, adr, &result );
		}
		break;																	/*								*/
	}																			/*								*/
	if( chk == OK )																/*								*/
	{																			/*								*/
		if( kin == PEC_PRM )													/* 固有ﾊﾟﾗﾒｰﾀ					*/
			prm_wk = PPrmSS[ses][adr];											/*								*/
		else{																	/* 共通ﾊﾟﾗﾒｰﾀ					*/
			prm_wk = CPrmSS[ses][adr];											/*								*/
			prm_invalid_change( ses, adr, &prm_wk );
		}
																				/*								*/
		result = prm_wk/arr[pos]%arr[len+1];									/*								*/
	}																			/*								*/
	return( result );															/*								*/
}																				/*								*/
																				/*								*/
typedef struct{
	short	sec;
	short	adr;
	long	data;
}t_param_inv;

const	t_param_inv	param_inv_table[] = {
	{ S_SCA, 1,	 100000}
// MH321800(S) Y.Tanizaki ICクレジット対応
,	{ S_SCA, 4,       0}	// 現金併用設定は変更させない
// MH321800(E) Y.Tanizaki ICクレジット対応
,	{ S_SCA, 5,	 200000}
,	{ S_SCA, 13,    255}
,	{ S_SCA, 14,      0}
,	{ S_SCA, 15,    101}
,	{ S_SCA, 16,    501}
};
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	固定値設定用ﾊﾟﾗﾒｰﾀ変換																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_invalid_chg(par1,par2)																   |*/
/*| PARAMETER	: par1	: ﾊﾟﾗﾒｰﾀｾｯｼｮﾝNo.																	   |*/
/*|				: par2	: ﾊﾟﾗﾒｰﾀｱﾄﾞﾚｽNo.																	   |*/
/*|				: par3	: 設定値格納ﾎﾟｲﾝﾀ																	   |*/
/*| RETURN VALUE: ret	: 固定設定値																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																					   |*/
/*| Date		: 2011-01-20																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2010 AMANO Corp.----------[]*/
void	prm_invalid_change(														/*								*/
		short	sec,															/* ｾｯｼｮﾝNo.						*/
		short	adr,															/* ｱﾄﾞﾚｽNo.						*/															/* 桁位置						*/
		long	*dat)															/* 設定値格納ﾎﾟｲﾝﾀ				*/															/* 桁位置						*/
{

	uchar	i;
	const t_param_inv	*p;

	WACDOG;
	for( i=0;i<TBL_CNT(param_inv_table);i++ ){
		p = &param_inv_table[i];
		if( sec == p->sec && adr == p->adr ){
			*dat = p->data;
			break;
		}
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	固定値設定用ﾊﾟﾗﾒｰﾀﾁｪｯｸ処理																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_invalid_check(par1,par2)																   |*/
/*| PARAMETER	: void	: 																					   |*/
/*| RETURN VALUE: ret	: 0→固定データへ書き換えなし	1→固定データへ書き換えあり							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																					   |*/
/*| Date		: 2011-01-20																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2010 AMANO Corp.----------[]*/
uchar	prm_invalid_check( void )												/*								*/
{

	uchar	i,ret;
	const t_param_inv	*p;
	
	ret = 0;
	WACDOG;
	for( i=0;i<TBL_CNT(param_inv_table);i++ ){
		p = &param_inv_table[i];
		if( CPrmSS[p->sec][p->adr] != p->data ){
			CPrmSS[p->sec][p->adr] = p->data;
			ret = 1;
		}
	}
	
	if( ret ){
		if( !FLAGPT.event_CtrlBitData.BIT.PARAM_INVALID_CHK ){	// 起動時は時計確定後にログ登録をするのでここではやらない
			SetSetDiff(SETDIFFLOG_SYU_INVALID);					// 設定更新履歴ログ登録
		}else{
			FLAGPT.event_CtrlBitData.BIT.PARAM_LOG_REC = 1;		// 設定更新履歴登録予約
		}
	}
	return ret;
}
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ﾊﾟﾗﾒｰﾀ変換(Param Sessionで変換したｱﾄﾞﾚｽを渡して分に直した時間を得る)									   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_tim(par1,par2,par3)																	   |*/
/*| PARAMETER	: par1	: ﾊﾟﾗﾒｰﾀ種類 0=共通/1=固有															   |*/
/*| 			: par2	: ﾊﾟﾗﾒｰﾀｾｯｼｮﾝNo.																	   |*/
/*|				: par3	: ﾊﾟﾗﾒｰﾀｱﾄﾞﾚｽNo.																	   |*/
/*| RETURN VALUE: ret	: 時間（分換算）																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|・下位4桁を使用して時分を入力しているｱﾄﾞﾚｽに対して使用する関数											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
long	prm_tim(																/*								*/
char	kin,																	/* ﾊﾟﾗﾒｰﾀ種類：0=共通/1=固有	*/
short	indat1,																	/* ｾｯｼｮﾝ№						*/
short	indat2 	)																/* ｱﾄﾞﾚｽ№						*/
{																				/*								*/
long	prm_wk;																	/*								*/
																				/*								*/
	if( kin == PEC_PRM )														/* 固有ﾊﾟﾗﾒｰﾀ					*/
		prm_wk = PPrmSS[indat1][indat2]%10000;									/*								*/
	else																		/* 共通ﾊﾟﾗﾒｰﾀ					*/
		prm_wk = CPrmSS[indat1][indat2]%10000;									/*								*/
																				/*								*/
	return((long)((prm_wk/100)*60+(prm_wk%100)));								/*								*/
}																				/*								*/
																				/*								*/
																				/*------------------------------*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	共通/個別ﾊﾟﾗﾒｰﾀの格納ｱﾄﾞﾚｽを取得する																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_init(par1)																			   |*/
/*| PARAMETER	: par1	: ﾊﾟﾗﾒｰﾀ種類 0=共通/1=固有															   |*/
/*| RETURN VALUE: non																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|・																										   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	prm_init( char kind )													/* 0=共通/1=固有				*/
{																				/*								*/
																				/*								*/
																				/*								*/
	switch( kind )																/*								*/
	{																			/*								*/
	case	PEC_PRM:															/* 固有ﾊﾟﾗﾒｰﾀ					*/
		PPrmSS[0]	= PParam.PParam00;											/*								*/
		PPrmSS[1]	= PParam.PParam01;											/*								*/
		PPrmSS[2]	= PParam.PParam02;											/*								*/
		PPrmSS[3]	= PParam.PParam03;											/*								*/
		break;																	/*								*/
	case	COM_PRM:															/* 共通ﾊﾟﾗﾒｰﾀ					*/
		CPrmSS[0]	= CParam.CParam00;											/*								*/
		CPrmSS[1]	= CParam.CParam01;											/*								*/
		CPrmSS[2]	= CParam.CParam02;											/*								*/
		CPrmSS[3]	= CParam.CParam03;											/*								*/
		CPrmSS[4]	= CParam.CParam04;											/*								*/
		CPrmSS[5]	= CParam.CParam05;											/*								*/
		CPrmSS[6]	= CParam.CParam06;											/*								*/
		CPrmSS[7]	= CParam.CParam07;											/*								*/
		CPrmSS[8]	= CParam.CParam08;											/*								*/
		CPrmSS[9]	= CParam.CParam09;											/*								*/
		CPrmSS[10]	= CParam.CParam10;											/*								*/
		CPrmSS[11]	= CParam.CParam11;											/*								*/
		CPrmSS[12]	= CParam.CParam12;											/*								*/
		CPrmSS[13]	= CParam.CParam13;											/*								*/
		CPrmSS[14]	= CParam.CParam14;											/*								*/
		CPrmSS[15]	= CParam.CParam15;											/*								*/
		CPrmSS[16]	= CParam.CParam16;											/*								*/
		CPrmSS[17]	= CParam.CParam17;											/*								*/
		CPrmSS[18]	= CParam.CParam18;											/*								*/
		CPrmSS[19]	= CParam.CParam19;											/*								*/
		CPrmSS[20]	= CParam.CParam20;											/*								*/
		CPrmSS[21]	= CParam.CParam21;											/*								*/
		CPrmSS[22]	= CParam.CParam22;											/*								*/
		CPrmSS[23]	= CParam.CParam23;											/*								*/
		CPrmSS[24]	= CParam.CParam24;											/*								*/
		CPrmSS[25]	= CParam.CParam25;											/*								*/
		CPrmSS[26]	= CParam.CParam26;											/*								*/
		CPrmSS[27]	= CParam.CParam27;											/*								*/
		CPrmSS[28]	= CParam.CParam28;											/*								*/
		CPrmSS[29]	= CParam.CParam29;											/*								*/
		CPrmSS[30]	= CParam.CParam30;											/*								*/
		CPrmSS[31]	= CParam.CParam31;											/*								*/
		CPrmSS[32]	= CParam.CParam32;											/*								*/
		CPrmSS[33]	= CParam.CParam33;											/*								*/
		CPrmSS[34]	= CParam.CParam34;											/*								*/
		CPrmSS[35]	= CParam.CParam35;											/*								*/
		CPrmSS[36]	= CParam.CParam36;											/*								*/
		CPrmSS[37]	= CParam.CParam37;											/*								*/
		CPrmSS[38]	= CParam.CParam38;											/*								*/
		CPrmSS[39]	= CParam.CParam39;											/*								*/
		CPrmSS[40]	= CParam.CParam40;
		CPrmSS[41]	= CParam.CParam41;
		CPrmSS[42]	= CParam.CParam42;											/*								*/
		CPrmSS[43]	= CParam.CParam43;											/*								*/
		CPrmSS[44]	= CParam.CParam44;											/*								*/
		CPrmSS[45]	= CParam.CParam45;											/*								*/
		CPrmSS[46]	= CParam.CParam46;											/*								*/
		CPrmSS[47]	= CParam.CParam47;											/*								*/
		CPrmSS[48]	= CParam.CParam48;											/*								*/
		CPrmSS[49]	= CParam.CParam49;											/*								*/
		CPrmSS[50]	= CParam.CParam50;											/*								*/
		CPrmSS[51]	= CParam.CParam51;											/*								*/
		CPrmSS[52]	= CParam.CParam52;											/*								*/
		CPrmSS[53]	= CParam.CParam53;											/*								*/
		CPrmSS[54]	= CParam.CParam54;											/*								*/
		CPrmSS[55]	= CParam.CParam55;											/*								*/
		CPrmSS[56]	= CParam.CParam56;											/*								*/
		CPrmSS[57]	= CParam.CParam57;											/*								*/
		CPrmSS[58]	= CParam.CParam58;											/*								*/
		CPrmSS[59]	= CParam.CParam59;											/*								*/
		CPrmSS[60]	= CParam.CParam60;											/*								*/
		break;																	/*								*/
	}																			/*								*/
	return;																		/*								*/
}																				/*								*/
																				/*------------------------------*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	共通/個別ﾊﾟﾗﾒｰﾀのｸﾘｱ,ﾃﾞﾌｫﾙﾄ値をｾｯﾄする																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_clr(par1,par2,par3)																	   |*/
/*| PARAMETER	: par1	: ﾊﾟﾗﾒｰﾀ種類 0=共通/1=固有															   |*/
/*| PARAMETER	: par2	: 処理要求	 0=ｸﾘｱ/1=ｸﾘｱ+ﾃﾞﾌｫﾙﾄ														   |*/
/*| PARAMETER	: par3	: ｾｯｼｮﾝ指定  0=全て/1～XX=ｾｯｼｮﾝ番号													   |*/
/*| RETURN VALUE: non																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|・05.03.14現在はRAMをﾀｰｹﾞｯﾄとしている																	   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	prm_clr(																/*								*/
char	kind,																	/* 0=共通/1=固有				*/
char	clr,																	/* 0=ｸﾘｱ/1=ｸﾘｱ+ﾃﾞﾌｫﾙﾄ			*/
char	set )																	/* 0=ALL/1～XX=ｾｯｼｮﾝ番号		*/
{																				/*								*/
	ushort	prm_wk;																/*								*/
	short	i;																	/*								*/
																				/*								*/
	switch( kind )																/*								*/
	{																			/*								*/
	case	PEC_PRM:															/* 固有ﾊﾟﾗﾒｰﾀ					*/
		if( set != 0 )															/* ｾｯｼｮﾝ指定あり				*/
		{																		/*								*/
			memset( PPrmSS[set], 0, (ulong)(4*(PPrmCnt[set]+1)) );				/* 指定されたｾｯｼｮﾝをｸﾘｱ			*/
			if( clr == 0 )														/* ｸﾘｱのみ指定の時				*/
				break;															/* 処理を抜ける					*/
																				/*								*/
			for( i=0; 0xff != pprm_rec[i].ses; i++)								/* ﾃﾞﾌｫﾙﾄ値ｾｯﾄ件数分ﾙｰﾌﾟ		*/
			{																	/*								*/
				if( pprm_rec[i].ses == set )									/* 指定されたｾｯｼｮﾝ番号が一致	*/
				{																/*								*/
					PPrmSS[set][pprm_rec[i].adr] = pprm_rec[i].dat;				/* ﾃﾞｰﾀｾｯﾄ						*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		else																	/* ｾｯｼｮﾝ指定無し(全て)			*/
		{																		/*								*/
			prm_wk = 0;															/*								*/
			for( i=0; i<P_PRM_SESCNT_MAX; i++ )									/* 全ﾊﾟﾗﾒｰﾀ件数を算出			*/
			{																	/*								*/
				prm_wk += (ushort)(PPrmCnt[i]+1);								/*								*/
			}																	/*								*/
			memset( PPrmSS[0], 0, (ulong)(4*prm_wk) );							/* 全ﾊﾟﾗﾒｰﾀｸﾘｱ					*/
			if( clr == 0 )														/* ｸﾘｱのみ指定の時				*/
				break;															/* 処理を抜ける					*/
																				/*								*/
			for( i=0; 0xff != pprm_rec[i].ses; i++)								/* ﾃﾞﾌｫﾙﾄ値ｾｯﾄ件数分ﾙｰﾌﾟ		*/
			{																	/*								*/
				PPrmSS[pprm_rec[i].ses][pprm_rec[i].adr]						/*								*/
											= pprm_rec[i].dat;					/* ﾃﾞｰﾀｾｯﾄ						*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
																				/*								*/
	case	COM_PRM:															/* 共通ﾊﾟﾗﾒｰﾀ					*/
		if( set != 0 )															/* ｾｯｼｮﾝ指定あり				*/
		{																		/*								*/
			memset( CPrmSS[set], 0, (ulong)(4*(CPrmCnt[set]+1)) );				/* 指定されたｾｯｼｮﾝをｸﾘｱ			*/
			if( clr == 0 )														/* ｸﾘｱのみ指定の時				*/
				break;															/* 処理を抜ける					*/
																				/*								*/
			for( i=0; 0xff != cprm_rec[i].ses; i++)								/* ﾃﾞﾌｫﾙﾄ値ｾｯﾄ件数分ﾙｰﾌﾟ		*/
			{																	/*								*/
				if( cprm_rec[i].ses == set )									/* 指定されたｾｯｼｮﾝ番号が一致	*/
				{																/*								*/
					CPrmSS[set][cprm_rec[i].adr] = cprm_rec[i].dat;				/* ﾃﾞｰﾀｾｯﾄ						*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		else																	/* ｾｯｼｮﾝ指定無し(全て)			*/
		{																		/*								*/
			prm_wk = 0;															/*								*/
			for( i=0; i<C_PRM_SESCNT_MAX; i++ )									/* 全ﾊﾟﾗﾒｰﾀ件数を算出			*/
			{																	/*								*/
				prm_wk += (ushort)(CPrmCnt[i]+1);								/*								*/
			}																	/*								*/
			memset( CPrmSS[0], 0, (ulong)(4*prm_wk) );							/* 全ﾊﾟﾗﾒｰﾀｸﾘｱ					*/
			if( clr == 0 )														/* ｸﾘｱのみ指定の時				*/
				break;															/* 処理を抜ける					*/
																				/*								*/
			for( i=0; 0xff != cprm_rec[i].ses; i++)								/* ﾃﾞﾌｫﾙﾄ値ｾｯﾄ件数分ﾙｰﾌﾟ		*/
			{																	/*								*/
				CPrmSS[cprm_rec[i].ses][cprm_rec[i].adr]						/*								*/
											= cprm_rec[i].dat;					/* ﾃﾞｰﾀｾｯﾄ						*/
			}																	/*								*/
			wopelg( OPLOG_KYOTUPARADF, 0, 0 );									/* 操作履歴登録					*/
		}																		/*								*/
		break;																	/*								*/
	}																			/*								*/

	if( COM_PRM == kind ){
		DataSumUpdate(OPE_DTNUM_COMPARA);										/* update parameter sum on ram	*/
	}
	return;																		/*								*/
}																				/*								*/
																				/*------------------------------*/

/*[]----------------------------------------------------------------------[]*/
/*| 車室パラメーター初期化                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockinfo_clr( clr )                                     |*/
/*| PARAMETER    : char clr : 0=ｸﾘｱ 1=ｸﾘｱ+ﾃﾞﾌｫﾙﾄ                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART:ogura 05-07-25									   |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lockinfo_clr(char clr)
{
	short	i;
	uchar	CarParm = 0;

	memset(LockInfo, 0, sizeof(LockInfo));	/* ゼロクリア */
	if (clr) {
		/* デフォルト値設定 */
		// 車室ﾊﾟﾗﾒｰﾀ初期値ｾｯﾄ（駐車）
		CarParm = GetCarInfoParam();
		if( CarParm & 0x02 ){
			for (i = 0; i < LOCKINFO_REC_MAX1; i++) {
				LockInfo[lockinfo_rec1[i].adr] = lockinfo_rec1[i].dat;
			}
		}
		
		if( CarParm & 0x04 ){
			for (i = 0; i < LOCKINFO_REC_MAX3; i++) {
				LockInfo[lockinfo_rec3[i].adr] = lockinfo_rec3[i].dat;
			}
		}
		if ( CarParm & 0x01 ) {	// 駐輪場併設あり？
			// 車室ﾊﾟﾗﾒｰﾀ初期値ｾｯﾄ（駐輪）
			for (i = 0; i < LOCKINFO_REC_MAX2; i++) {
				LockInfo[lockinfo_rec2[i].adr] = lockinfo_rec2[i].dat;
			}
		}
		wopelg( OPLOG_SHASHITUPARADF, 0, 0 );		/* 操作履歴登録					*/
		memcpy( &bk_LockInfo, &LockInfo, sizeof( LockInfo ));
	}
	DataSumUpdate(OPE_DTNUM_LOCKINFO);
}

/*[]----------------------------------------------------------------------[]*/
/*| ロック種別パラメーター初期化                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockmaker_clr( clr )                                    |*/
/*| PARAMETER    : char clr : 0=ｸﾘｱ 1=ｸﾘｱ+ﾃﾞﾌｫﾙﾄ                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART:ogura 05-07-25									   |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lockmaker_clr(char clr)
{
	short	i;

	memset(LockMaker, 0, sizeof(LockMaker));	/* ゼロクリア */
	if (clr) {
		/* デフォルト値設定 */
		for (i = 0; i < LOCKMAKER_REC_MAX; i++) {
			LockMaker[lockmaker_rec[i].adr] = lockmaker_rec[i].dat;
		}

		wopelg( OPLOG_ROCKPARADF, 0, 0 );		/* 操作履歴登録					*/
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			デフォルトパラメーター取得
//[]----------------------------------------------------------------------[]
///	@param[in]		ses		: セッションNo.
///	@param[in]		addr	: アドレスNo.
///	@param[in/out]	pos		: 位置
///	@return			data	: デフォルトデータ
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/04/17<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
long	default_prm_get(uchar ses, ushort addr, ushort *pos)
{
	ushort	i;

	i = *pos;
	while (cprm_rec[i].ses < ses) {
		i++;
	}
	if (cprm_rec[i].ses == ses) {
		do {
			if (cprm_rec[i].adr == addr) {
				*pos = i;
				(*pos)++;
				return((long)cprm_rec[i].dat);
			}
			i++;
		} while (cprm_rec[i].ses == ses);
	}
	return(0);
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	個別パラメータ追番チェック																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_oiban_chk( void)																		   |*/
/*| PARAMETER	: void	: 																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: H.Sekiguchi																				   |*/
/*| Date		: 2011-08-18																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 201 AMANO Corp.----------[]*/
void prm_oiban_chk(void)
{
	uchar cnt;
	
	for(cnt = 1 ; cnt < COUNT_MAX; cnt++){
		if(PPrmSS[S_P03][cnt] == 0){
			PPrmSS[S_P03][cnt] = 1;
		}
	}
}

