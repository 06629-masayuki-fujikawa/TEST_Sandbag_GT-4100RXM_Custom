/*[]--------------------------------------------------------------------------------------------------[]*/
/*|		設定ﾃｰﾌﾞﾙ定義																				   |*/
/*[]--------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	prm_tbl.c																		   |*/
/*[]--------------------------------------------------------------------------------------------------[]*/
/*| Author      :																					   |*/
/*| Date        :	2005-03-10																		   |*/
/*| Update      :																					   |*/
/*|----------------------------------------------------------------------------------------------------|*/
/*| ・設定ﾃｰﾌﾞﾙ宣言及び設定値取得に関連するﾌﾟﾛﾄﾀｲﾌﾟ宣言												   |*/
/*|																									   |*/
/*[]------------------------------------------------------- Copyright(C) 2005 AMANO Corp.-------------[]*/
																		/*------------------------------*/
#include	"system.h"													/*								*/
																		/*								*/
#include	"prm_tbl.h"													/*								*/
																		/*								*/
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/*	共通ﾊﾟﾗﾒｰﾀ																							*/
/*------------------------------------------------------------------------------------------------------*/
																		/*								*/
const ushort	CPrmCnt[C_PRM_SESCNT_MAX] = 							/* ﾊﾟﾗﾒｰﾀｾｯｼｮﾝ数格納配列		*/
{																		/* ※ﾊﾟﾗﾒｰﾀｾｯｼｮﾝ毎の件数を定義	*/
	0,																	/*								*/
	C_PRM_SES01,														/*								*/
	C_PRM_SES02,														/*								*/
	C_PRM_SES03,														/*								*/
	C_PRM_SES04,														/*								*/
	C_PRM_SES05,														/*								*/
	C_PRM_SES06,														/*								*/
	C_PRM_SES07,														/*								*/
	C_PRM_SES08,														/*								*/
	C_PRM_SES09,														/*								*/
	C_PRM_SES10,														/*								*/
	C_PRM_SES11,														/*								*/
	C_PRM_SES12,														/*								*/
	C_PRM_SES13,														/*								*/
	C_PRM_SES14,														/*								*/
	C_PRM_SES15,														/*								*/
	C_PRM_SES16,														/*								*/
	C_PRM_SES17,														/*								*/
	C_PRM_SES18,														/*								*/
	C_PRM_SES19,														/*								*/
	C_PRM_SES20,														/*								*/
	C_PRM_SES21,														/*								*/
	C_PRM_SES22,														/*								*/
	C_PRM_SES23,														/*								*/
	C_PRM_SES24,														/*								*/
	C_PRM_SES25,														/*								*/
	C_PRM_SES26,														/*								*/
	C_PRM_SES27,														/*								*/
	C_PRM_SES28,														/*								*/
	C_PRM_SES29,														/*								*/
	C_PRM_SES30,														/*								*/
	C_PRM_SES31,														/*								*/
	C_PRM_SES32,														/*								*/
	C_PRM_SES33,														/*								*/
	C_PRM_SES34,														/*								*/
	C_PRM_SES35,														/*								*/
	C_PRM_SES36,														/*								*/
	C_PRM_SES37,														/*								*/
	C_PRM_SES38,														/*								*/
	C_PRM_SES39															/*								*/
	,	C_PRM_SES40
	,	C_PRM_SES41
	,	C_PRM_SES42														/*								*/
	,	C_PRM_SES43														/*								*/
	,	C_PRM_SES44														/*								*/
	,	C_PRM_SES45														/*								*/
	,	C_PRM_SES46														/*								*/
	,	C_PRM_SES47														/*								*/
	,	C_PRM_SES48														/*								*/
	,	C_PRM_SES49														/*								*/
	,	C_PRM_SES50														/*								*/
	,	C_PRM_SES51														/*								*/
	,	C_PRM_SES52														/*								*/
	,	C_PRM_SES53														/*								*/
	,	C_PRM_SES54														/*								*/
	,	C_PRM_SES55														/*								*/
	,	C_PRM_SES56														/*								*/
	,	C_PRM_SES57														/*								*/
	,	C_PRM_SES58														/*								*/
	,	C_PRM_SES59														/*								*/
	,	C_PRM_SES60														/*								*/
};																		/*								*/
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/*	固有ﾊﾟﾗﾒｰﾀ																							*/
/*------------------------------------------------------------------------------------------------------*/
																		/*								*/
const short	PPrmCnt[P_PRM_SESCNT_MAX] = 								/* ﾊﾟﾗﾒｰﾀｾｯｼｮﾝ数格納配列		*/
{																		/*								*/
	0,																	/*								*/
	P_PRM_SES01,														/*								*/
	P_PRM_SES02,														/*								*/
	P_PRM_SES03															/*								*/
};																		/*								*/
																		/*								*/
																		/*------------------------------*/
