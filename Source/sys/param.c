/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		�ݒ荀�ڎQ��																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	param.c																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �E��������Ұ��̎Q��																						   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
																				/*								*/
#include 	<string.h>															/*								*/
																				/*								*/
#include	"system.h"															/*								*/
#include	"rkn_def.h"															/* OK/NG�̂���					*/
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
/*		�֐��錾															 									*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
long	prm_get( char, short, short, char, char );								/* ����/�ŗL���Ұ��̎擾		*/
long	prm_tim( char, short, short );											/* ����/�ŗL���Ұ����Ԃ̎擾	*/
void	prm_init( char );														/* ����/�ŗL���Ұ����ڽ���		*/
void	prm_clr( char, char, char );											/* ����/�ŗL���Ұ��ر/��̫��	*/
ulong	prm_svtim( ushort, uchar );												/* ���޽��ю擾					*/
																				/* 								*/
																				/* 								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���Ұ��ϊ��i�ėp�j																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_get(par1,par2,par3,par4,par5)															   |*/
/*| PARAMETER	: par1	: ���Ұ���� 0=����/1=�ŗL															   |*/
/*|				: par2	: ���Ұ������No.																	   |*/
/*| 			: par3	: ���ڽ																				   |*/
/*| 			: par4	: ����(�����擾���邩)																   |*/
/*| 			: par5	: �ʒu(�����ڂ���擾���邩 �ʒu��654321�Ƃ���)										   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
long	prm_get(																/*								*/
		char	kin,															/* ���Ұ���ށF0=����/1=��	*/
		short	ses,															/* �����No.						*/
		short	adr,															/* ���ڽNo.						*/
		char	len,															/* �擾����						*/
		char	pos	)															/* ���ʒu						*/
{																				/*								*/
	long	prm_wk;																/* ���Ұ��ϊ�ܰ�				*/
	long	result	= -1;														/*								*/
	char	chk		= NG;														/*								*/
																				/*								*/
	static	const long arr[] = { 0L,1L,10L,100L,1000L,10000L,100000L };			/* 								*/
																				/*								*/
	switch( len )																/* �擾�����ɂ�蕪��			*/
	{																			/*								*/
	case 1:																		/* 1���擾						*/
		if(pos>6){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 2:																		/* 2���擾						*/
		if(pos>5){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 3:																		/* 3���擾						*/
		if(pos>4){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 4:																		/* 4���擾						*/
		if(pos>3){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 5:																		/* 5���擾						*/
		if(pos>2){																/*								*/
			break;																/*								*/
		}																		/*								*/
		chk = OK;																/*								*/
		break;																	/*								*/
	case 6:																		/* 6���擾						*/
		if(pos>1){																/*								*/
			break;																/*								*/
		}																		/*								*/
		if( kin == PEC_PRM )													/* �ŗL���Ұ�					*/
			result = PPrmSS[ses][adr];											/*								*/
		else{																	/* �������Ұ�					*/
			result = CPrmSS[ses][adr];											/*								*/
			prm_invalid_change( ses, adr, &result );
		}
		break;																	/*								*/
	}																			/*								*/
	if( chk == OK )																/*								*/
	{																			/*								*/
		if( kin == PEC_PRM )													/* �ŗL���Ұ�					*/
			prm_wk = PPrmSS[ses][adr];											/*								*/
		else{																	/* �������Ұ�					*/
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
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
,	{ S_SCA, 4,       0}	// �������p�ݒ�͕ύX�����Ȃ�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
,	{ S_SCA, 5,	 200000}
,	{ S_SCA, 13,    255}
,	{ S_SCA, 14,      0}
,	{ S_SCA, 15,    101}
,	{ S_SCA, 16,    501}
};
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�Œ�l�ݒ�p���Ұ��ϊ�																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_invalid_chg(par1,par2)																   |*/
/*| PARAMETER	: par1	: ���Ұ������No.																	   |*/
/*|				: par2	: ���Ұ����ڽNo.																	   |*/
/*|				: par3	: �ݒ�l�i�[�߲��																	   |*/
/*| RETURN VALUE: ret	: �Œ�ݒ�l																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																					   |*/
/*| Date		: 2011-01-20																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2010 AMANO Corp.----------[]*/
void	prm_invalid_change(														/*								*/
		short	sec,															/* �����No.						*/
		short	adr,															/* ���ڽNo.						*/															/* ���ʒu						*/
		long	*dat)															/* �ݒ�l�i�[�߲��				*/															/* ���ʒu						*/
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
/*|	�Œ�l�ݒ�p���Ұ���������																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_invalid_check(par1,par2)																   |*/
/*| PARAMETER	: void	: 																					   |*/
/*| RETURN VALUE: ret	: 0���Œ�f�[�^�֏��������Ȃ�	1���Œ�f�[�^�֏�����������							   |*/
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
		if( !FLAGPT.event_CtrlBitData.BIT.PARAM_INVALID_CHK ){	// �N�����͎��v�m���Ƀ��O�o�^������̂ł����ł͂��Ȃ�
			SetSetDiff(SETDIFFLOG_SYU_INVALID);					// �ݒ�X�V�������O�o�^
		}else{
			FLAGPT.event_CtrlBitData.BIT.PARAM_LOG_REC = 1;		// �ݒ�X�V����o�^�\��
		}
	}
	return ret;
}
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���Ұ��ϊ�(Param Session�ŕϊ��������ڽ��n���ĕ��ɒ��������Ԃ𓾂�)									   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_tim(par1,par2,par3)																	   |*/
/*| PARAMETER	: par1	: ���Ұ���� 0=����/1=�ŗL															   |*/
/*| 			: par2	: ���Ұ������No.																	   |*/
/*|				: par3	: ���Ұ����ڽNo.																	   |*/
/*| RETURN VALUE: ret	: ���ԁi�����Z�j																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|�E����4�����g�p���Ď�������͂��Ă�����ڽ�ɑ΂��Ďg�p����֐�											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
long	prm_tim(																/*								*/
char	kin,																	/* ���Ұ���ށF0=����/1=�ŗL	*/
short	indat1,																	/* ����݇�						*/
short	indat2 	)																/* ���ڽ��						*/
{																				/*								*/
long	prm_wk;																	/*								*/
																				/*								*/
	if( kin == PEC_PRM )														/* �ŗL���Ұ�					*/
		prm_wk = PPrmSS[indat1][indat2]%10000;									/*								*/
	else																		/* �������Ұ�					*/
		prm_wk = CPrmSS[indat1][indat2]%10000;									/*								*/
																				/*								*/
	return((long)((prm_wk/100)*60+(prm_wk%100)));								/*								*/
}																				/*								*/
																				/*								*/
																				/*------------------------------*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	����/�����Ұ��̊i�[���ڽ���擾����																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_init(par1)																			   |*/
/*| PARAMETER	: par1	: ���Ұ���� 0=����/1=�ŗL															   |*/
/*| RETURN VALUE: non																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|�E																										   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	prm_init( char kind )													/* 0=����/1=�ŗL				*/
{																				/*								*/
																				/*								*/
																				/*								*/
	switch( kind )																/*								*/
	{																			/*								*/
	case	PEC_PRM:															/* �ŗL���Ұ�					*/
		PPrmSS[0]	= PParam.PParam00;											/*								*/
		PPrmSS[1]	= PParam.PParam01;											/*								*/
		PPrmSS[2]	= PParam.PParam02;											/*								*/
		PPrmSS[3]	= PParam.PParam03;											/*								*/
		break;																	/*								*/
	case	COM_PRM:															/* �������Ұ�					*/
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
/*|	����/�����Ұ��̸ر,��̫�Ēl��Ă���																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: prm_clr(par1,par2,par3)																	   |*/
/*| PARAMETER	: par1	: ���Ұ���� 0=����/1=�ŗL															   |*/
/*| PARAMETER	: par2	: �����v��	 0=�ر/1=�ر+��̫��														   |*/
/*| PARAMETER	: par3	: ����ݎw��  0=�S��/1�`XX=����ݔԍ�													   |*/
/*| RETURN VALUE: non																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|�E05.03.14���݂�RAM�����ޯĂƂ��Ă���																	   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	prm_clr(																/*								*/
char	kind,																	/* 0=����/1=�ŗL				*/
char	clr,																	/* 0=�ر/1=�ر+��̫��			*/
char	set )																	/* 0=ALL/1�`XX=����ݔԍ�		*/
{																				/*								*/
	ushort	prm_wk;																/*								*/
	short	i;																	/*								*/
																				/*								*/
	switch( kind )																/*								*/
	{																			/*								*/
	case	PEC_PRM:															/* �ŗL���Ұ�					*/
		if( set != 0 )															/* ����ݎw�肠��				*/
		{																		/*								*/
			memset( PPrmSS[set], 0, (ulong)(4*(PPrmCnt[set]+1)) );				/* �w�肳�ꂽ����݂�ر			*/
			if( clr == 0 )														/* �ر�̂ݎw��̎�				*/
				break;															/* �����𔲂���					*/
																				/*								*/
			for( i=0; 0xff != pprm_rec[i].ses; i++)								/* ��̫�Ēl��Č�����ٰ��		*/
			{																	/*								*/
				if( pprm_rec[i].ses == set )									/* �w�肳�ꂽ����ݔԍ�����v	*/
				{																/*								*/
					PPrmSS[set][pprm_rec[i].adr] = pprm_rec[i].dat;				/* �ް����						*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		else																	/* ����ݎw�薳��(�S��)			*/
		{																		/*								*/
			prm_wk = 0;															/*								*/
			for( i=0; i<P_PRM_SESCNT_MAX; i++ )									/* �S���Ұ��������Z�o			*/
			{																	/*								*/
				prm_wk += (ushort)(PPrmCnt[i]+1);								/*								*/
			}																	/*								*/
			memset( PPrmSS[0], 0, (ulong)(4*prm_wk) );							/* �S���Ұ��ر					*/
			if( clr == 0 )														/* �ر�̂ݎw��̎�				*/
				break;															/* �����𔲂���					*/
																				/*								*/
			for( i=0; 0xff != pprm_rec[i].ses; i++)								/* ��̫�Ēl��Č�����ٰ��		*/
			{																	/*								*/
				PPrmSS[pprm_rec[i].ses][pprm_rec[i].adr]						/*								*/
											= pprm_rec[i].dat;					/* �ް����						*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
																				/*								*/
	case	COM_PRM:															/* �������Ұ�					*/
		if( set != 0 )															/* ����ݎw�肠��				*/
		{																		/*								*/
			memset( CPrmSS[set], 0, (ulong)(4*(CPrmCnt[set]+1)) );				/* �w�肳�ꂽ����݂�ر			*/
			if( clr == 0 )														/* �ر�̂ݎw��̎�				*/
				break;															/* �����𔲂���					*/
																				/*								*/
			for( i=0; 0xff != cprm_rec[i].ses; i++)								/* ��̫�Ēl��Č�����ٰ��		*/
			{																	/*								*/
				if( cprm_rec[i].ses == set )									/* �w�肳�ꂽ����ݔԍ�����v	*/
				{																/*								*/
					CPrmSS[set][cprm_rec[i].adr] = cprm_rec[i].dat;				/* �ް����						*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		else																	/* ����ݎw�薳��(�S��)			*/
		{																		/*								*/
			prm_wk = 0;															/*								*/
			for( i=0; i<C_PRM_SESCNT_MAX; i++ )									/* �S���Ұ��������Z�o			*/
			{																	/*								*/
				prm_wk += (ushort)(CPrmCnt[i]+1);								/*								*/
			}																	/*								*/
			memset( CPrmSS[0], 0, (ulong)(4*prm_wk) );							/* �S���Ұ��ر					*/
			if( clr == 0 )														/* �ر�̂ݎw��̎�				*/
				break;															/* �����𔲂���					*/
																				/*								*/
			for( i=0; 0xff != cprm_rec[i].ses; i++)								/* ��̫�Ēl��Č�����ٰ��		*/
			{																	/*								*/
				CPrmSS[cprm_rec[i].ses][cprm_rec[i].adr]						/*								*/
											= cprm_rec[i].dat;					/* �ް����						*/
			}																	/*								*/
			wopelg( OPLOG_KYOTUPARADF, 0, 0 );									/* ���엚��o�^					*/
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
/*| �Ԏ��p�����[�^�[������                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockinfo_clr( clr )                                     |*/
/*| PARAMETER    : char clr : 0=�ر 1=�ر+��̫��                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART:ogura 05-07-25									   |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lockinfo_clr(char clr)
{
	short	i;
	uchar	CarParm = 0;

	memset(LockInfo, 0, sizeof(LockInfo));	/* �[���N���A */
	if (clr) {
		/* �f�t�H���g�l�ݒ� */
		// �Ԏ����Ұ������l��āi���ԁj
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
		if ( CarParm & 0x01 ) {	// ���֏ꕹ�݂���H
			// �Ԏ����Ұ������l��āi���ցj
			for (i = 0; i < LOCKINFO_REC_MAX2; i++) {
				LockInfo[lockinfo_rec2[i].adr] = lockinfo_rec2[i].dat;
			}
		}
		wopelg( OPLOG_SHASHITUPARADF, 0, 0 );		/* ���엚��o�^					*/
		memcpy( &bk_LockInfo, &LockInfo, sizeof( LockInfo ));
	}
	DataSumUpdate(OPE_DTNUM_LOCKINFO);
}

/*[]----------------------------------------------------------------------[]*/
/*| ���b�N��ʃp�����[�^�[������                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lockmaker_clr( clr )                                    |*/
/*| PARAMETER    : char clr : 0=�ر 1=�ر+��̫��                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART:ogura 05-07-25									   |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lockmaker_clr(char clr)
{
	short	i;

	memset(LockMaker, 0, sizeof(LockMaker));	/* �[���N���A */
	if (clr) {
		/* �f�t�H���g�l�ݒ� */
		for (i = 0; i < LOCKMAKER_REC_MAX; i++) {
			LockMaker[lockmaker_rec[i].adr] = lockmaker_rec[i].dat;
		}

		wopelg( OPLOG_ROCKPARADF, 0, 0 );		/* ���엚��o�^					*/
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�f�t�H���g�p�����[�^�[�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		ses		: �Z�b�V����No.
///	@param[in]		addr	: �A�h���XNo.
///	@param[in/out]	pos		: �ʒu
///	@return			data	: �f�t�H���g�f�[�^
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
/*|	�ʃp�����[�^�ǔԃ`�F�b�N																				   |*/
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

