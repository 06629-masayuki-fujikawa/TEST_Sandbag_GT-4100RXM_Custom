/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		磁気リーダタスク																					   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	fbcom.c																					   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・FB7000との通信及びMAINﾀｽｸとのｲﾝﾀｰﾌｪｰｽ																	   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include	<string.h>															/*								*/
#include	<stdlib.h>															/*								*/
#include	"iodefine.h"														/*								*/
#include	"system.h"															/*								*/
#include	"Message.h"															/*								*/
#include	"prm_tbl.h"															/*								*/
#include	"mdl_def.h"
#include	"fb_def.h"															/*								*/
#include	"fbcom.h"															/*								*/
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
																				/*								*/
																				/*								*/
union	{																		/*								*/
	struct	{																	/*								*/
		uchar	CommErr		: 1 ;												/* 1=ｵｰﾊﾞｰﾗﾝ､ﾌﾚｰﾐﾝｸﾞ､ﾊﾟﾘﾃｨ ｴﾗｰ	*/
		uchar	bit_6		: 1 ;												/* reserve						*/
		uchar	bit_5		: 1 ;												/* reserve						*/
		uchar	bit_4		: 1 ;												/* reserve						*/
		uchar	bit_3		: 1 ;												/* reserve						*/
		uchar	bit_2		: 1 ;												/* reserve						*/
		uchar	bit_1		: 1 ;												/* reserve						*/
		uchar	bit_0		: 1 ;												/* reserve						*/
	} bits;																		/*								*/
	uchar	byte;																/*								*/
} FB_MainErr;																	/*								*/
																				/*								*/
																				/*								*/
t_FBcomTerm		FBcomTerm;													/* 端末情報						*/
																				/*								*/
void		FBcom_Init( void );													/*								*/
void		FBcom_TimValInit( void );											/*								*/
void		FBcom_Main( void );													/*								*/
void		FBcom_Event1( void );												/*								*/
void		FBcom_Event2( void );												/*								*/
void		FBcom_Event3( void );												/*								*/
void		FBcom_Event4( void );												/*								*/
void		FBcom_Event5( void );												/*								*/
uchar		FBcom_SubEventCheck( void );										/*								*/
uchar		FBcom_SubRcvAnalysis( uchar *, ushort );							/*								*/
void		FBcom_MatrixCheck( uchar, t_FBcomTerm *, uchar *  );				/*								*/
void		FBcom_SndCtrlPkt( uchar, uchar, uchar );							/*								*/
																				/*								*/
ushort		FBcom_SndDtPkt( t_FBcomTerm *, uchar, uchar, uchar );				/*								*/
uchar		FBcom_WaitSciSendCmp( ushort );										/*								*/
ushort		FBcom_SndDtDec( void );												/*								*/
uchar		FBcom_RcvDataSave( uchar*, uchar );									/* 受信したﾃﾞｰﾀを取込			*/
																				/*								*/
void		FBcom_SetSendData(uchar* pSendData, ushort size);
ushort		FBcom_BinaryToAscii(uchar* pBinData, ushort binSize, uchar* pAsciiData);
ushort		FBcom_AsciiToBinary(uchar* pAsciiData, ushort asciiSize, uchar* pBinData);
uchar*		FBcom_GetSyncDataIndex( uchar* pData, ushort size, ushort* pDatSize );
void 		FBcom_InitMatrix( void );
void 		FBcom_SetReceiveData( void );


/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF盤通信初期化																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Init()																				   |*/
/*| PARAMETER	: par1	: 																					   |*/
/*|				: par2	: 																					   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_Init( void )														/*								*/
{																				/*								*/
																				/*								*/
	FBcom_TimValInit();															/* ﾀｲﾏｰ&ﾘﾄﾗｲｶｳﾝﾄｾｯﾄ				*/
	FBcom_2mTimStop();															/* Timer stop					*/
	FBcom_20mTimStop();															/* Timer stop					*/
	FBcom_20mTimStop2();															/* Timer stop					*/
																				/*								*/
	memset( &FBcomTerm, 0x00, sizeof(FBcomTerm) );

	FBcomTerm.Matrix = FB_S0;													/* ﾏﾄﾘｸｽ						*/
	memset(FBcomTerm.status, 0xff, sizeof(FBcomTerm.status));

	FBcom_20mTimStart(FBcom_Timer_4_Value);										/*								*/
}																				/*								*/
																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF盤通信関連ﾀｲﾏｰ初期化																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_TimValInit()																		   |*/
/*| PARAMETER	: par1	: 																					   |*/
/*|				: par2	: 																					   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_TimValInit( void )												/*								*/
{																				/*								*/
	uchar	bps;																/*								*/
																				/*								*/
	bps = 0;																	/* ﾎﾞｰﾚｰﾄｾｯﾄ 38400				*/
																				/*								*/
	FBcom_Timer_4_Value = 60 / 20;												/* ENQｲﾝﾀｰﾊﾞﾙ    20msﾀｲﾏｰ		*/
																				/*								*/
	/*** 各種通信ﾀｲﾏｰｾｯﾄ ***/													/*								*/
	switch( bps ){																/*								*/
	case 0:																		/* 38400 bps					*/
		FBcom_Timer_5_Value = 2;												/* 文字間監視ﾀｲﾏｰ 2*2=4 msec	*/
		FBcom_Timer_2_Value = 50;												/* 受信ﾃﾞｰﾀﾀｲﾏｰ	20*50=1000 msec	*/
		break;																	/*								*/
	case 1:																		/* 19200 bps					*/
		FBcom_Timer_5_Value = 2;												/* 文字間監視ﾀｲﾏｰ 2*2=4 msec	*/
		FBcom_Timer_2_Value = 50;												/* 受信ﾃﾞｰﾀﾀｲﾏｰ	20*50=1000 msec	*/
		break;																	/*								*/
	case 2:																		/* 9600 bps						*/
		FBcom_Timer_5_Value = 3;												/* 文字間監視ﾀｲﾏｰ 2*3=6 msec	*/
		FBcom_Timer_2_Value = 80;												/* 受信ﾃﾞｰﾀﾀｲﾏｰ	20*80=1600 msec	*/
		break;																	/*								*/
	case 3:																		/* 4800 bps						*/
		FBcom_Timer_5_Value = 5;												/* 文字間監視ﾀｲﾏｰ 2*5=10 msec	*/
		FBcom_Timer_2_Value = 130;												/* 受信ﾃﾞｰﾀﾀｲﾏｰ	20*130=2600 msec*/
		break;																	/*								*/
	case 4:																		/* 2400 bps						*/
		FBcom_Timer_5_Value = 10;												/* 文字間監視ﾀｲﾏｰ 2*10=20 msec	*/
		FBcom_Timer_2_Value = 230;												/* 受信ﾃﾞｰﾀﾀｲﾏｰ	20*230=4600 msec*/
		break;																	/*								*/
	default:																	/* その他は 38400bps			*/
		bps = 0;																/* ﾎﾞｰﾚｰﾄを強制で0にする		*/
		FBcom_Timer_5_Value = 2;												/* 文字間監視ﾀｲﾏｰ 2*2=4 msec	*/
		FBcom_Timer_2_Value = 30;												/* 受信ﾃﾞｰﾀﾀｲﾏｰ	20*50=1000 msec	*/
		break;																	/*								*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF盤通信MAIN																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Main()																				   |*/
/*| PARAMETER	: par1	: 																					   |*/
/*|				: par2	: 																					   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_Main( void )														/*								*/
{																				/*								*/
																				/*								*/
	switch( FBcom_SubEventCheck() ){											/*								*/
	case	1:																	/* 1= 受信完了					*/
		FBcom_Event1();															/*								*/
		break;																	/*								*/
	case	2:																	/* 2= 通信ｴﾗｰ					*/
		FBcom_Event2();															/*								*/
		break;																	/*								*/
	case	3:																	/* 3= 通信ﾀｲﾑｱｳﾄ				*/
		FBcom_Event3();															/*								*/
		break;																	/*								*/
	case	4:																	/* 4= 通信ﾗｲﾝﾀｲﾑｱｳﾄ				*/
		FBcom_Event4();															/*								*/
		break;																	/*								*/
	case	5:																	/* 5= 受信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰ			*/
		FBcom_Event5();															/*								*/
		break;																	/*								*/
	default:																	/*								*/
		break;																	/*								*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ｲﾍﾞﾝﾄ発生ﾁｪｯｸ																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SubEventCheck()																		   |*/
/*| PARAMETER	: par1	: 																					   |*/
/*|				: par2	: 																					   |*/
/*| 			: par3	: 																					   |*/
/*| 			: par4	: 																					   |*/
/*| 			: par5	: 																					   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	FBcom_SubEventCheck( void )												/*								*/
{																				/*								*/
	if( 0 != FBcomdr_f_RcvCmp ){												/* 受信ｲﾍﾞﾝﾄあり				*/
		return (uchar)FBcomdr_f_RcvCmp;											/*								*/
	}																			/*								*/
																				/*								*/
	if( FBcom_20mTimeout() != 0 ){												/* ﾀｲﾑｱｳﾄｲﾍﾞﾝﾄ(ENQ間隔)			*/
		return (uchar)4;														/*								*/
	}																			/*								*/
	return (uchar)0;															/* ｲﾍﾞﾝﾄ発生無し				*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	受信ﾃﾞｰﾀ処理																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Event1()																			   |*/
/*| PARAMETER	: none 																						   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_Event1( void )													/*								*/
{																				/*								*/
	uchar	retc;																/*								*/
	uchar*	pData;
	ushort	length;
	static uchar com_err_cnt = 0;
																				/*								*/
	FB_MainErr.bits.CommErr = 0;												/*								*/
																				/*								*/
	pData = FBcom_GetSyncDataIndex(&FBcomdr_RcvData[0], FBcomdr_RcvLength, &length);
	if(pData == NULL) {
		FBcomdr_f_RcvCmp = 0;													/* ｲﾍﾞﾝﾄｸﾘｱ						*/
		FBcom_20mTimStart(FBcom_Timer_4_Value);									/*								*/
		return;
	}
	
	switch(	(retc = FBcom_SubRcvAnalysis(pData, length)) )						/*								*/
	{																			/* 受信ﾃﾞｰﾀの解析				*/
																				/*								*/
	case 1:																		/* ﾃﾞｰﾀｻｲｽﾞ-NG					*/
	case 4:																		/* ADRなし						*/
	case 6:																		/* 制御ｺｰﾄﾞ-NG					*/
	case 7:																		/* ﾃﾞｰﾀID-NG					*/
	case 9:																		/* TEXT後のTEXT					*/
		// NOTE:FT-4000シリーズの場合プリンタ印字を行っている時にプリンタ通信がI2Cバス経由のためFBタスクレベルでの
		// パケット受信処理が追いつかなくなる場合があるので、エラーの発生は連続3回以上とし判定レベルを下げる
		com_err_cnt++;
		if(com_err_cnt >= 3){
			com_err_cnt = 0;
			err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 1, 0, 0 );
		}
		FBcom_20mTimStart(FBcom_Timer_4_Value);									/*								*/
		break;																	/* 無視							*/
																				/*								*/
	case 0:																		/* 正常ﾃﾞｰﾀ受信					*/
		com_err_cnt = 0;
		err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 0, 0, 0 );						/* 00:ﾘｰﾀﾞ通信異常解除			*/
	case 5:																		/* CRC16-NG						*/
	case 8:																		/* Lengthﾁｪｯｸ					*/
		FBcom_20mTimStop();														/*								*/
		FBcom_MatrixCheck( retc, &FBcomTerm, pData );							/* ﾏﾄﾘｸｽﾁｪｯｸ					*/
		break;																	/*								*/
	}																			/*								*/
	FBcomdr_f_RcvCmp = 0;														/* ｲﾍﾞﾝﾄｸﾘｱ						*/
// MH810103 GG119202(S) ICクレジット対応（PCI-DSS）
	memset(FBcomdr_RcvData, 0, sizeof(FBcomdr_RcvData));
// MH810103 GG119202(E) ICクレジット対応（PCI-DSS）
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ｼﾘｱﾙｺﾐｭﾆｹｰｼｮﾝｴﾗｰ発生																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Event2()																			   |*/
/*| PARAMETER	: none																						   |*/
/*| RETURN VALUE: none																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_Event2( void )													/*								*/
{																				/*								*/
	ushort length;
	uchar* pData;
	
	FBcom_2mTimStop();															/*								*/
	FBcom_20mTimStop();															/*								*/
																				/*								*/
	if(FBcomdr_SciErrorState & 0x01) {											// オーバーランエラー
		err_chk( ERRMDL_READER, ERR_RED_SCI_OVERRUN, 2, 0, 0 );					// 発生解除
	}
	else if(FBcomdr_SciErrorState & 0x02) {										// フレーミングエラー
		err_chk( ERRMDL_READER, ERR_RED_SCI_FRAME, 2, 0, 0 );					// 発生解除
	}
	else if(FBcomdr_SciErrorState & 0x04) {										// パリティエラー
		err_chk( ERRMDL_READER, ERR_RED_SCI_PARITY, 2, 0, 0 );					// 発生解除
	}
	FBcomdr_SciErrorState = 0;
	pData = FBcom_GetSyncDataIndex(&FBcomdr_RcvData[0], FBcomdr_RcvLength, &length);
	if(pData) {
		if(length >= 3 && FB_REQ_STX == *(pData + 2)) {
			// データ受信(STX)のエラーならNAKを送信する
			FBcom_MatrixCheck( FB_REQ_NAK, &FBcomTerm, pData );					/* ﾏﾄﾘｸｽﾁｪｯｸ					*/
			memset(FBcomdr_RcvData, 0, sizeof(FBcomdr_RcvData));
		}
	}
	FBcom_InitMatrix();
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcom_Event3()																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		Communication time-out occurred																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	FBcom_Event3( void )													/*								*/
{																				/*								*/
	FBcom_2mTimStop();															/*								*/
	FBcom_20mTimStop();															/*								*/
																				/*								*/
	FBcom_SndCtrlPkt( FB_REQ_ENQ, 1, 0 );										/*【ENQ】送信					*/
	FBcom_InitMatrix();
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	POL/SEL判定																								   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Event4()																			   |*/
/*| PARAMETER	: none																						   |*/
/*| RETURN VALUE: none																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_Event4( void )													/*								*/
{																				/*								*/
	FBcom_20mTimStop();															/* Timer stop					*/
	FBcom_20mTimStop2();														/*								*/
	FBcom_2mTimStop();															/*								*/
																				/*								*/
	FBcomdr_f_RcvCmp = 0;														/* ｲﾍﾞﾝﾄｸﾘｱ						*/

	if(FB_MainErr.bits.CommErr == 0 && (FBcomTerm.DtSndNG > 0 || FBcomTerm.DtRepNG > 0)) {
		// 通信エラーが発生していない状態で、NAK受信による再送中はENQを送信しない
		return;
	}
	
	if(FBcomTerm.RcvResFlg == 1) {
		// 無応答判定回数クリア
		FBcomTerm.NoAnsCnt = 0;
	}
	else {
		// ENQ間隔中に応答なし
		++FBcomTerm.NoAnsCnt;
		if(FBcomTerm.NoAnsCnt >= FB_NORSP_CNT) {
			// 無応答判定回数超過
			err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 1, 0, 0 );					/* ｴﾗｰ発生						*/
			FBcomTerm.NoAnsCnt = 0;
		}
	}
	FBcomTerm.RcvResFlg = 0;

	FBcomTerm.Matrix = FB_S1;													/* ﾎﾟｰﾘﾝｸﾞ後の応答待ち(EMPTY)	*/
	FBcom_SndCtrlPkt(FB_REQ_ENQ, 0, 0);											/* 制御ﾃﾞｰﾀ送信					*/
}																				/*								*/

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ｼﾘｱﾙｺﾐｭﾆｹｰｼｮﾝｴﾗｰ発生																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Event5()																			   |*/
/*| PARAMETER	: none																						   |*/
/*| RETURN VALUE: none																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_Event5( void )													/*								*/
{																				/*								*/
	FBcom_2mTimStop();															/*								*/
	FBcom_20mTimStop();															/*								*/
																				/*								*/
	FBcom_SndCtrlPkt( FB_REQ_ENQ, 1, 0 );										/*【ENQ】送信					*/
	FBcom_InitMatrix();
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	受信ﾃﾞｰﾀ解析																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SubRcvAnalysis()																	   |*/
/*| PARAMETER	: pData	: 受信データ																		   |*/
/*|				: size	: 受信データサイズ																	   |*/
/*| RETURN VALUE: ret	: 0		: データOK																	   |*/
/*						: 1		: データサイズNG															   |*/
/*						: 4		: データ不正（ADRなし）														   |*/
/*						: 5		: データ不正（BCCエラー）													   |*/
/*						: 6		: データ不正（制御コードNG）												   |*/
/*						: 7		: データ不正（受信IDNG）													   |*/
/*						: 8		: データ不正（データ長不一致）												   |*/
/*						: 9		: データ不正（マトリクス異常）												   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・受信ﾃﾞｰﾀに対して無視すべきﾃﾞｰﾀと応答すべきﾃﾞｰﾀの判定を行う											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	FBcom_SubRcvAnalysis( uchar *pData, ushort size )						/*								*/
{																				/*								*/
																				/*								*/
ushort			ctrlCode;
ushort			dataSize;
uchar			bcc;
																				/*								*/
	/*** ﾊﾟｹｯﾄｼｰｹﾝｼｬﾙ№判定 ***/												/*								*/
	if( pData[0] != 0x32) {														/* ADR?					 		*/
		return 4;;
	}

	/*** ﾃﾞｰﾀID&制御ｺｰﾄﾞ判定 ***/												/*								*/
	ctrlCode = *(pData + 1);
	switch( ctrlCode ){															/* 制御ｺｰﾄﾞﾁｪｯｸ					*/
	case FB_REQ_ACK:															/* [ACK]						*/
	case FB_REQ_NAK:															/* [NAK]						*/
		if(size != 2) {
			return 1;															/* ﾃﾞｰﾀｻｲｽﾞNG					*/
		}
		break;
	case FB_REQ_EOT:															/* [EOT]						*/
		if(size != 6) {
			return 1;															/* ﾃﾞｰﾀｻｲｽﾞNG					*/
		}
		break;																	/*								*/
	default:
		switch(*(pData + 2)) {
		case FB_REQ_STX:														/*								*/
			if(	(pData[FB_PKT_COMMAND] == FB_RCV_VERS_ID) ||					/* ﾊﾞｰｼﾞｮﾝﾁｪｸ					*/
				(pData[FB_PKT_COMMAND] == FB_RCV_READ_ID) ||					/* ﾘｰﾄﾞﾃﾞｰﾀ						*/
				(pData[FB_PKT_COMMAND] == FB_RCV_END_ID) ||						/* 終了ﾃﾞｰﾀ						*/
				(pData[FB_PKT_COMMAND] == FB_RCV_SENSOR_ID) ||					/* ｾﾝｻｰ状態						*/
				(pData[FB_PKT_COMMAND] == FB_RCV_MNT_ID) ) {					/* ﾒﾝﾃﾅﾝｽ応答					*/
																				/*								*/
				dataSize = (ushort)pData[4];
				if( dataSize + 7 > size ){										/* ﾊﾟｹｯﾄﾃﾞｰﾀ長ﾁｪｯｸ				*/
					return( 8 );												/*								*/
				}																/*								*/
				if( FBcomTerm.Matrix == FB_S3 ){								/* ﾊﾟｹｯﾄ送信後(無視)			*/
					return( 9 );												/*								*/
				}																/*								*/
			
				// BCCチェック
				bcc = (uchar)~bcccal( (char*)&pData[3], (ushort)(dataSize + 3) );
				if(bcc != pData[5 + dataSize + 1]) {
					return( 5 );												/*								*/
				}																/*								*/
			}else{																/*								*/
				return( 7 );													/* 受信ID-NG(無視)				*/
			}																	/*								*/
			break;
		default:
			return( 6 );															/* 制御ｺｰﾄﾞNG(無視)				*/
		}
		break;																	/*								*/
	}																			/*								*/
																				/*								*/
	return( 0 );																/* ﾃﾞｰﾀ判定OK					*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	受信ﾃﾞｰﾀﾏﾄﾘｸｽﾁｪｯｸ																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_MatrixCheck(par1,par2,par3)															   |*/
/*| PARAMETER	: result	: データ解析結果																   |*/
/*|				: dp		: 通信情報構造体																   |*/
/*| 			: pData		: 受信データ																	   |*/
/*| RETURN VALUE: none																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・応答すべきﾃﾞｰﾀに対してﾏﾄﾘｸｽ上のﾁｪｯｸを行う																   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
void	FBcom_MatrixCheck(														/*								*/
		uchar 		result,														/*								*/
		t_FBcomTerm *dp,														/*								*/
		uchar		*pData )													/* 受信ﾃﾞｰﾀﾊﾞｯﾌｧ				*/
{																				/*								*/
ushort	ret;																	/*								*/
																				/*								*/
	switch( *(pData + 1) )														/* 制御文字の取得				*/
	{																			/*								*/
	/***  EOT受信  ***/
	case FB_REQ_EOT:															/* [EOT]受信					*/
		switch( dp->Matrix )													/*								*/
		{																		/*								*/
		case FB_S0:		/* ﾆｭｰﾄﾗﾙ	*/											/* 現状態：ﾆｭｰﾄﾗﾙ				*/
			break;																/*								*/
																				/*								*/
		case FB_S1:		/* ENQ送信後	*/										/* 現状態：ENQ送信後			*/
			dp->Matrix	= FB_S0;												/* ﾆｭｰﾄﾗﾙへ移動					*/
			dp->ErrFlg.bits.CtCRC_NG = 0;										/* ｴﾗｰﾌﾗｸﾞON					*/
			
			if(FBcomTerm.CtrlInfo.Count > 0) {
				// 送信データがあれば送信処理を行う
				FBcom_SndDtPkt( &FBcomTerm, FB_REQ_STX, 0x01, 0 );
				FBcomTerm.Matrix = FB_S3;										/* ﾊﾟｹｯﾄ送信後応答待ちへ		*/
			}
			
			if(memcmp(dp->status, pData + 2, sizeof(dp->status))) {
				// 前回のステータスと比較して、異なる場合は通知する
				memcpy(dp->status, pData + 2, sizeof(dp->status));
				ret = FBcom_RcvDataSave(pData, 1 );								/* 受信したﾃﾞｰﾀを取込			*/
				cr_rcv();
			}
			break;																/*								*/
																				/*								*/
		case FB_S3:		/* ﾃﾞｰﾀ送信後	*/										/* 現状態：TEXT後応答			*/
			dp->Matrix	= FB_S0;												/* ﾆｭｰﾄﾗﾙへ移動					*/
			dp->ErrFlg.bits.CtCRC_NG = 0;										/* ｴﾗｰﾌﾗｸﾞON					*/
			break;																/*								*/
		}																		/*								*/
		break;																	/*								*/
																				/*								*/
	/***  ACK受信  ***/															/*								*/
	case FB_REQ_ACK:															/* [ACK]受信					*/
		switch( dp->Matrix )													/*								*/
		{																		/*								*/
		case FB_S0:		/* ﾆｭｰﾄﾗﾙ	*/											/* 現状態：ﾆｭｰﾄﾗﾙ				*/
			break;																/*								*/
																				/*								*/
		case FB_S1:		/* ENQ送信後	*/										/* 現状態：ENQ送信後			*/
			dp->Matrix	= FB_S0;												/* ﾆｭｰﾄﾗﾙへ移動					*/
			dp->SndDtID = 0;
			dp->ErrFlg.bits.CtCRC_NG = 0;										/* ｴﾗｰﾌﾗｸﾞON					*/
			break;																/*								*/
																				/*								*/
		case FB_S3:		/* ﾃﾞｰﾀ送信後	*/										/* 現状態：TEXT後応答			*/
			FBcom_SndDtDec();													/* ﾊﾟｹｯﾄ破棄(送信完了)			*/
			dp->Matrix	= FB_S0;												/* ﾆｭｰﾄﾗﾙへ						*/
			dp->DtSndNG = 0;
			dp->DtRepNG = 0;
			dp->SndDtID = 0;
			dp->ErrFlg.bits.CtCRC_NG = 0;										/* ｴﾗｰﾌﾗｸﾞ						*/
			break;																/*								*/
		}																		/*								*/
		break;																	/*								*/
																				/*								*/
	/***  NAK受信の時  ***/
	case FB_REQ_NAK:															/*								*/
		switch( dp->Matrix ){													/*								*/
		case FB_S0:																/* 現状態：ﾆｭｰﾄﾗﾙ				*/
		case FB_S1:																/* 現状態：ENQ送信後			*/
			break;																/*								*/
																				/*								*/
		case FB_S3:		/* ﾃﾞｰﾀ送信後	*/										/* 現状態：TEXT送信後			*/
			if( dp->DtSndNG > (uchar)FB_DTSND_RETRY )							/* ﾃﾞｰﾀ再送回数ｵｰﾊﾞｰ			*/
			{
				if(dp->DtRepNG <= (uchar)FB_DTSND_REPEAT) {
					// リトライ回数を超えた場合は、ENQを送る
					dp->Matrix	= FB_S0;										/* ﾆｭｰﾄﾗﾙへ						*/
					dp->DtCrcNG	= 0;											/*								*/
					dp->DtSndNG = 0;											/*								*/
					dp->SndDtID = 0;
					
					//通信異常時のリトライカウンタを更新
					++dp->DtRepNG;

					FBcom_SndCtrlPkt( FB_REQ_ENQ, 1, 0 );						/*【ENQ】送信					*/
					FBcomTerm.Matrix = FB_S1;									/* ﾎﾟｰﾘﾝｸﾞ後の応答待ち(EMPTY)	*/
				}
				else {
					// 通信異常時のリトライ回数オーバーで再送をあきらめ総送信データ削除
					dp->Matrix	= FB_S0;										/* ﾆｭｰﾄﾗﾙへ						*/
					dp->DtCrcNG	= 0;											/*								*/
					dp->DtSndNG = 0;											/*								*/
					dp->DtRepNG = 0;
					dp->SndDtID = 0;
					FBcom_SndDtDec();											/* ﾊﾟｹｯﾄ消去					*/
					err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 1, 0, 0 );			/* ｴﾗｰ発生/解除通知				*/
				}
			}																	/*								*/
			else
			{
				// リトライ回数未満は再送する
				FBcom_SndDtPkt( dp, FB_REQ_STX, 0x00, dp->SndDtID);				/* ﾊﾟｹｯﾄ送信					*/
				dp->DtSndNG ++;													/* NAKﾘﾄﾗｲ送信ｶｳﾝﾄUp			*/
				dp->Matrix = FB_S3;												/* ﾊﾟｹｯﾄ応答待ちへ移動			*/
			}																	/*								*/
			break;																/*								*/
		}																		/*								*/
		break;																	/*								*/
	default:
		switch(*(pData + 2)) {
		/***  DATA受信の時  ***/
		case FB_REQ_STX:														/*								*/
			switch( dp->Matrix )												/*								*/
			{																	/*								*/
			case FB_S0:															/* 現状態：ﾆｭｰﾄﾗﾙ				*/
				break;															/*								*/
																				/*								*/
			case FB_S1:		/* ENQ送信後	*/									/* 現状態：ENQ送信後			*/
				switch( result )												/* 判定結果=BCC-NG				*/
				{																/*								*/
				case FB_BCC_NG:													/* 判定結果=BCC-NG				*/
				case FB_SERIAL_NG:												/* ｼﾘｱﾙｴﾗｰ発生					*/
					if( dp->DtRcvNG > FB_DTRCV_RETRY )							/* BCC-NGﾘﾄﾗｲｵｰﾊﾞｰ				*/
					{															/*								*/
						FBcom_SndCtrlPkt(FB_REQ_ACK, 1, 0 );					/* 強制ACK終了要求				*/
						dp->DtRcvNG	= 0;										/*								*/
						dp->Matrix	= FB_S0;									/* ﾆｭｰﾄﾗﾙへ移動					*/
						/*--------------------------------------------------*/
						/* 受信ﾃﾞｰﾀCRC-NGでﾘﾄﾗｲ回数ｵｰﾊﾞｰ：強制ACK送信		*/
						/*--------------------------------------------------*/
						err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 1, 0, 0 );		/* ｴﾗｰ発生/解除通知				*/
					}															/*								*/
					else														/*								*/
					{															/*								*/
						FBcom_SndCtrlPkt(FB_REQ_NAK, 1, 0);						/*【NAK】BCC-NG 80H				*/
						dp->Matrix = FB_S1;										/* POL後のﾃﾞｰﾀ待ち				*/
						dp->DtRcvNG ++;											/* BCC-NG						*/
					}															/*								*/
					break;														/*								*/
				default:														/* 正常ﾃﾞｰﾀ受信					*/
					ret = FBcom_RcvDataSave(pData, 0 );							/* 受信したﾃﾞｰﾀを取込			*/
					if( ret == 1 ){												/*								*/
						FBcom_SndCtrlPkt(FB_REQ_EOT, 1, 1);						/*【EOT】送信 80H:ﾊﾞｯﾌｧFULL		*/
					}else{														/*								*/
						FBcom_SndCtrlPkt(FB_REQ_ACK, 1, 0);						/*【ACK】送信					*/
					}															/*								*/
					dp->DtRcvNG	= 0;											/*								*/
					dp->Matrix = FB_S0;											/* ﾆｭｰﾄﾗﾙへ移動					*/
					cr_rcv();
					break;														/*								*/
				}																/*								*/
				break;															/*								*/
																				/*								*/
			case FB_S3:															/* TEXT							*/
				break;															/*								*/
			}																	/*								*/
			break;																/*								*/
		default:
			break;
		}
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}																			/*								*/
	if( dp->Matrix == FB_S0 ){													/*								*/
		FBcom_20mTimStart( FBcom_Timer_4_Value );								/* POL/SEL送信ﾘｸｴｽﾄ				*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	制御ﾊﾟｹｯﾄ送信要求																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SndCtrlPkt(par1,par2,par3,par4)													   |*/
/*| PARAMETER	: ctrl	: 制御ｺｰﾄﾞ種類 0=ENQ/1=ACK/2=NAK													   |*/
/*|				: blk	: 転送ﾌﾞﾛｯｸ数(ENQの時のみ有効)														   |*/
/*|				: t_st	: 端末ｽﾃｰﾀｽ																			   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・AKC/NAK/ENQ等の制御ﾊﾟｹｯﾄをSCIﾊﾞｯﾌｧへｾｯﾄし送信する														   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_SndCtrlPkt(														/*								*/
uchar		ctrl,																/* 制御ｺｰﾄﾞ						*/
uchar		blk,																/* ﾌﾞﾛｯｸ数(ENQの時のみ有効)		*/
uchar		t_st )																/* 端末ｽﾃｰﾀｽ(FULL/EMPTY)		*/
{																				/*								*/
	ushort	Length;																/* ﾃﾞｰﾀ長						*/
																				/*								*/
	memset( &FB_SndBuf, 0x00, (uchar)(FB_ASCII_PKT_HEAD_SIZ+2) );				/* 送信ﾃﾞｰﾀﾊﾞｯﾌｧｸﾘｱ				*/
																				/*								*/
	FB_SndBuf[0] = '1';
	FB_SndBuf[1] = '6';
	FB_SndBuf[2] = '1';
	FB_SndBuf[3] = '6';
	FB_SndBuf[4] = '1';
	FB_SndBuf[5] = '6';
	FB_SndBuf[6] = '3';
	FB_SndBuf[7] = '2';
	switch( ctrl ){																/* 制御ｺｰﾄﾞ						*/
	case FB_REQ_ACK:															/* 【ACK】						*/
		FB_SndBuf[8]	= '0';
		FB_SndBuf[9]	= '6';
		break;																	/*								*/
	case FB_REQ_NAK:															/* 【NAK】						*/
		FB_SndBuf[8]	= '1';
		FB_SndBuf[9]	= '5';
		break;																	/*								*/
	case FB_REQ_ENQ:															/* 【ENQ】						*/
		FB_SndBuf[8]	= '0';
		FB_SndBuf[9]	= '5';
		break;																	/*								*/
	}																			/*								*/
																				/*								*/
	Length = (ushort)FB_ASCII_PKT_HEAD_SIZ;										/* ﾍｯﾀﾞｰ部ｻｲｽﾞ					*/
																				/* 								*/
	FBcomTerm.SndDtID = FB_SND_CTRL_ID;											/*								*/
	FBcom_WaitSciSendCmp( Length );												/* send complete wait			*/
																				/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ﾃﾞｰﾀﾊﾟｹｯﾄ送信要求ｾｯﾄ																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SndDtPkt(par1,par2,par3)															   |*/
/*| PARAMETER	: dp	: 端末No.別回線情報																	   |*/
/*|				: code	: 0=MAINからの要求ﾃﾞｰﾀ/送信ﾃﾞｰﾀID=送信指定するﾃﾞｰﾀID								   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・ｱﾌﾟﾘｹｰｼｮﾝから渡された送信ﾃﾞｰﾀをSCI送信ﾊﾞｯﾌｧへｾｯﾄし送信する											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
ushort	FBcom_SndDtPkt( 														/*								*/
t_FBcomTerm		*dp,															/* 回線管理						*/
uchar	code,																	/* 制御ｺｰﾄﾞ						*/
uchar	stat,																	/* 制御ｽﾃｰﾀｽ					*/
uchar	typ																		/* 0=新規/1=再送				*/
)																				/*								*/
{																				/*								*/
	ushort		Length;															/*								*/
	ushort		ret;
	t_FBsendDataBuffer* pSendData;
																				/*								*/
	if( typ == 0 ){																/*								*/
		if(	dp->CtrlInfo.Count == 0 ){											/* ﾃﾞｰﾀ無し						*/
			return( 0 );														/*								*/
		}																		/*								*/
	}																			/*								*/
	memset( &FB_SndBuf[0], 0x00, FB_SCI_SNDBUF_SIZE );							/* 送信ﾊﾞｯﾌｧｸﾘｱ					*/
																				/*								*/
	FB_SndBuf[0] = '1';		// SYN
	FB_SndBuf[1] = '6';
	FB_SndBuf[2] = '1';		// SYN
	FB_SndBuf[3] = '6';
	Length = 4;
	
	// ASCII変換後に送信バッファに格納
	pSendData =	&FBcomTerm.SendData[FBcomTerm.CtrlInfo.R_Index];
	Length += FBcom_BinaryToAscii(pSendData->buffer, pSendData->size, &FB_SndBuf[4]);
	dp->SndDtID = FB_SND_DATA_ID;												/*								*/
																				/*								*/
	ret = (ushort)FBcom_WaitSciSendCmp( Length );								/*								*/
	
	return	ret;																/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ﾃﾞｰﾀﾊﾟｹｯﾄ送信要求																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_WaitSciSendCmp()																	   |*/
/*| PARAMETER	: Length	: 送信データ長																	   |*/
/*| RETURN VALUE: ret	: 0 : 送信完了																		   |*/
/*| 			: 		: 1 : 送信データ無し or タイムアウト												   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	FBcom_WaitSciSendCmp(													/*								*/
ushort	Length )																/*								*/
{																				/*								*/
	ulong 	StartTime;															/*								*/
																				/*								*/
																				/*								*/
	if( FBsci_SndReq( Length ) != 0 ){											/* send NG ?					*/
		return	(uchar)1;														/*								*/
	}																			/*								*/
	StartTime = LifeTim2msGet();												/* now life time get			*/
																				/*								*/
	while( 0 == LifePastTim2msGet(StartTime, 600L) ){							/* till timeout (1sec)			*/
		taskchg( IDLETSKNO );													/* タスク切替					*/
																				/*								*/
		if( 1 == FBsci_IsSndCmp() ){											/* send complete (Y) 			*/
			FBcom_20mTimStart( FBcom_Timer_4_Value );							/* recv wait timer start 		*/
			return	(uchar)0;													/*								*/
		}																		/*								*/
	}																			/*								*/
	FBcom_20mTimStart( FBcom_Timer_4_Value );							/* recv wait timer start 		*/
	return	(uchar)1;															/* This case will be only flow control useing. 	*/
																				/* in this time, flow control doesn't use.		*/
																				/* Therefore I must return OK status.			*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	送信済みﾃﾞｰﾀ消去																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SndDtDec(void)																		   |*/
/*| PARAMETER	: Non	: 																					   |*/
/*| RETURN VALUE: ret	: 送信ﾃﾞｰﾀｷｭｰ管理ﾃｰﾌﾞﾙの件数														   |*/
/*| 			: 		: 0xFFFF = 引数ｴﾗｰ																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・IF盤へ送信済みのﾃﾞｰﾀﾊﾟｹｯﾄを消去する																	   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
ushort	FBcom_SndDtDec( void )													/*								*/
{																				/*								*/
	ushort	ret=0xFFFF;

	/* 送ったﾃﾞｰﾀが初期設定ﾃﾞｰﾀの時 */											/* 								*/
	if( 0 == FBcomTerm.CtrlInfo.Count ){										/* 								*/
		return 0;																/* 								*/
	}																			/* 								*/
																				/* 								*/
	++FBcomTerm.CtrlInfo.R_Index;												/* 								*/
	if( FB_SENDDATA_QUEUE_MAX <= FBcomTerm.CtrlInfo.R_Index ){					/* 								*/
		FBcomTerm.CtrlInfo.R_Index = 0;										/* 								*/
	}																			/* 								*/
	--FBcomTerm.CtrlInfo.Count;												/* 								*/
	ret = FBcomTerm.CtrlInfo.Count;

	return(ret);
}																				/* 								*/
																				/* 								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	受信ﾃﾞｰﾀﾊﾟｹｯﾄｾｰﾌﾞ																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_RcvDataSave()																		   |*/
/*| PARAMETER	: pData	: 受信データ																		   |*/
/*|				: typ	: 格納ﾀｲﾌﾟ	0:今回のﾃﾞｰﾀのみｱﾌﾟﾘﾊﾞｯﾌｧへ格納											   |*/
/*|									1:今回のﾃﾞｰﾀを一時ﾊﾞｯﾌｧへ格納											   |*/
/*|									2:今回と一時ﾊﾞｯﾌｧのﾃﾞｰﾀをｱﾌﾟﾘﾊﾞｯﾌｧへ格納								   |*/
/*| RETURN VALUE: ret	: 設定値																			   |*/
/*| 			: 		: -1 = 引数ｴﾗｰ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・0:今受信したﾃﾞｰﾀをｱﾌﾟﾘｹｰｼｮﾝへの受信ﾃﾞｰﾀﾊﾞｯﾌｧへ格納													   |*/
/*|     FBcomdr_RcvBufに格納されているﾃﾞｰﾀをｱﾌﾟﾘﾊﾞｯﾌｧFBcom_RcvDataへ格納する								   |*/
/*| ・1:今回受信した1ﾌﾞﾛｯｸﾃﾞｰﾀを一時保留ﾊﾞｯﾌｧへ指定したﾌﾞﾛｯｸ番号の位置に格納する(未完：複数ﾌﾞﾛｯｸ用)			   |*/
/*| ・2:今回受信したﾃﾞｰﾀと一時ﾊﾞｯﾌｧのﾃﾞｰﾀをｱﾌﾟﾘﾊﾞｯﾌｧへ格納する(未完：複数ﾌﾞﾛｯｸ用)							   |*/
/*| 【構造体構成】																							   |*/
/*|		ﾃﾞｰﾀｻｲｽﾞ	2 byte(2+1+ﾃﾞｰﾀ)																		   |*/
/*|		ﾃﾞｰﾀID4		1 byte																					   |*/
/*|		ﾃﾞｰﾀ		**** byte(ID1～CRCの手前)																   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
																				/*								*/
uchar	FBcom_RcvDataSave(														/* 受信したﾃﾞｰﾀを取込			*/
uchar*	pData,
uchar	typ )																	/* 格納ﾀｲﾌﾟ						*/
{																				/*								*/
	switch( typ ){																/*								*/
																				/*								*/
	case 0:		// データ
		// ToDo: データ受信後にすぐに通知するので、バッファは1データ分とする
		//		 複数データを保持する場合はフラップの処理を参考にする
		memcpy( &FBcom_RcvData, pData, (size_t)pData[4] + 5 );					/* ADRからﾃﾞｰﾀｾｰﾌﾞ				*/
		break;																	/*								*/
																				/*								*/
	case 1:		// EOT
		memcpy( &FBcom_RcvData, pData, 6);										/* ADRからﾃﾞｰﾀｾｰﾌﾞ				*/
		break;																	/*								*/
																				/*								*/
	case 2:																		/*								*/
		break;																	/*								*/
																				/*								*/
	}																			/*								*/
	return( 0 );																/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcom_SetSendData()																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		送信データ格納																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	FBcom_SetSendData(uchar* pSendData, ushort size)
{
	if( FB_SENDDATA_QUEUE_MAX <= FBcomTerm.CtrlInfo.Count ){					/* ﾊﾞｯﾌｧFULL状態				*/
		++FBcomTerm.CtrlInfo.R_Index;											/* 古いﾃﾞｰﾀを消去				*/
		if( FB_SENDDATA_QUEUE_MAX <= FBcomTerm.CtrlInfo.R_Index ){				/* 								*/
			FBcomTerm.CtrlInfo.R_Index = 0;									/*								*/
		}																		/* 								*/
		--FBcomTerm.CtrlInfo.Count;											/* 								*/
	}																			/*								*/
	
	FBcomTerm.SendData[FBcomTerm.CtrlInfo.W_Index].size = size;
	memcpy(FBcomTerm.SendData[FBcomTerm.CtrlInfo.W_Index].buffer, pSendData, (size_t)size);
	
	++FBcomTerm.CtrlInfo.W_Index;												/*								*/
	if( FB_SENDDATA_QUEUE_MAX <= FBcomTerm.CtrlInfo.W_Index ){					/*								*/
		FBcomTerm.CtrlInfo.W_Index = 0;										/*								*/
	}																			/*								*/
	++FBcomTerm.CtrlInfo.Count;												/*								*/
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcom_BinaryToAscii()																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		バイナリ→ASCII変換																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
ushort	FBcom_BinaryToAscii(uchar* pBinData, ushort binSize, uchar* pAsciiData)
{
	ushort binIndex;
	ushort asciiIndex;
	uchar* pBuffer;
	uchar data;
	
	pBuffer = pAsciiData;
	asciiIndex = 0;
	for(binIndex = 0; binIndex < binSize; ++binIndex) {
		data = (uchar)((*(pBinData + binIndex) >> 4) & 0x0f);
		if(data <= 9) {
			pBuffer[asciiIndex] = (uchar)('0' + data);
		}
		else {
			pBuffer[asciiIndex] = (uchar)('A' + (data - 10));
		}
		++asciiIndex;
		
		data = (uchar)((*(pBinData + binIndex)) & 0x0f);
		if(data <= 9) {
			pBuffer[asciiIndex] = (uchar)('0' + data);
		}
		else {
			pBuffer[asciiIndex] = (uchar)('A' + (data - 10));
		}
		++asciiIndex;
	}
	
	return asciiIndex;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcom_AsciiToBinary()																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		ASCII変換→バイナリ																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
ushort	FBcom_AsciiToBinary(uchar* pAsciiData, ushort asciiSize, uchar* pBinData)
{
	uchar* pBuffer;
	ushort binIndex;
	ushort asciiIndex;
	uchar data;
	uchar asciiChar;
	
	pBuffer = pBinData;
	binIndex = 0;
	for(asciiIndex = 0; asciiIndex < asciiSize; ++asciiIndex) {
		asciiChar = pAsciiData[asciiIndex];
		if(asciiChar <= '9') {
			data = (uchar)(asciiChar - '0');
		}
		else {
			data = (uchar)(asciiChar - 'A' + 10);
		}
		
		if(asciiIndex & 0x0001) {			// 奇数か
			pBuffer[binIndex] |= data;
			++binIndex;
		}
		else {
			pBuffer[binIndex] = (uchar)(data << 4);
		}
	}

	return binIndex;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcom_GetSyncDataIndex()																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		SYNを探しSYN以外のデータのポインタを返す															   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
uchar*		FBcom_GetSyncDataIndex( uchar* pData, ushort size, ushort* pDataSize )
{
	uchar	synflg;
	ushort	index;

	synflg = 0;
	if(pData && pData[0] == FB_REQ_SYN ) {
		for(index = 0; index < size; ++index) {
			if(pData[index] == FB_REQ_SYN) {
				synflg = 1;
			}
			else {
				if(synflg == 1) {
					// 最後のSYNからSYN以外ポインタを返す
					*pDataSize = size - index;
					return pData + index;
				}
				else {
					break;
				}
			}
		}
	}
	*pDataSize = 0;
	return NULL;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcom_InitMatrix()																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		処理マトリックスを初期化しENQ送信待ちとする															   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void 		FBcom_InitMatrix( void )
{
	FBcomTerm.Matrix = FB_S0;													/*								*/
	FBcomTerm.DtCrcNG = 0;														/*								*/
																				/*								*/
	FBcom_20mTimStart(FBcom_Timer_4_Value);										/*								*/
	FBcomdr_f_RcvCmp = 0;														/* ｲﾍﾞﾝﾄｸﾘｱ						*/
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcom_SetReceiveData()																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		データを受信したことを記録する																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void 		FBcom_SetReceiveData( void )
{
	FBcomTerm.RcvResFlg = 1;
}
