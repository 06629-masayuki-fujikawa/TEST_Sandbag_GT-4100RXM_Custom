/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		���C���[�_�^�X�N																					   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	fbcom.c																					   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �EFB7000�Ƃ̒ʐM�y��MAIN����Ƃ̲���̪��																	   |*/
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
		uchar	CommErr		: 1 ;												/* 1=���ް�ݤ�ڰ�ݸޤ���è �װ	*/
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
t_FBcomTerm		FBcomTerm;													/* �[�����						*/
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
uchar		FBcom_RcvDataSave( uchar*, uchar );									/* ��M�����ް����捞			*/
																				/*								*/
void		FBcom_SetSendData(uchar* pSendData, ushort size);
ushort		FBcom_BinaryToAscii(uchar* pBinData, ushort binSize, uchar* pAsciiData);
ushort		FBcom_AsciiToBinary(uchar* pAsciiData, ushort asciiSize, uchar* pBinData);
uchar*		FBcom_GetSyncDataIndex( uchar* pData, ushort size, ushort* pDatSize );
void 		FBcom_InitMatrix( void );
void 		FBcom_SetReceiveData( void );


/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF�ՒʐM������																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Init()																				   |*/
/*| PARAMETER	: par1	: 																					   |*/
/*|				: par2	: 																					   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
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
	FBcom_TimValInit();															/* ��ϰ&��ײ���ľ��				*/
	FBcom_2mTimStop();															/* Timer stop					*/
	FBcom_20mTimStop();															/* Timer stop					*/
	FBcom_20mTimStop2();															/* Timer stop					*/
																				/*								*/
	memset( &FBcomTerm, 0x00, sizeof(FBcomTerm) );

	FBcomTerm.Matrix = FB_S0;													/* ��ظ�						*/
	memset(FBcomTerm.status, 0xff, sizeof(FBcomTerm.status));

	FBcom_20mTimStart(FBcom_Timer_4_Value);										/*								*/
}																				/*								*/
																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF�ՒʐM�֘A��ϰ������																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_TimValInit()																		   |*/
/*| PARAMETER	: par1	: 																					   |*/
/*|				: par2	: 																					   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
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
	bps = 0;																	/* �ްڰľ�� 38400				*/
																				/*								*/
	FBcom_Timer_4_Value = 60 / 20;												/* ENQ�������    20ms��ϰ		*/
																				/*								*/
	/*** �e��ʐM��ϰ��� ***/													/*								*/
	switch( bps ){																/*								*/
	case 0:																		/* 38400 bps					*/
		FBcom_Timer_5_Value = 2;												/* �����ԊĎ���ϰ 2*2=4 msec	*/
		FBcom_Timer_2_Value = 50;												/* ��M�ް���ϰ	20*50=1000 msec	*/
		break;																	/*								*/
	case 1:																		/* 19200 bps					*/
		FBcom_Timer_5_Value = 2;												/* �����ԊĎ���ϰ 2*2=4 msec	*/
		FBcom_Timer_2_Value = 50;												/* ��M�ް���ϰ	20*50=1000 msec	*/
		break;																	/*								*/
	case 2:																		/* 9600 bps						*/
		FBcom_Timer_5_Value = 3;												/* �����ԊĎ���ϰ 2*3=6 msec	*/
		FBcom_Timer_2_Value = 80;												/* ��M�ް���ϰ	20*80=1600 msec	*/
		break;																	/*								*/
	case 3:																		/* 4800 bps						*/
		FBcom_Timer_5_Value = 5;												/* �����ԊĎ���ϰ 2*5=10 msec	*/
		FBcom_Timer_2_Value = 130;												/* ��M�ް���ϰ	20*130=2600 msec*/
		break;																	/*								*/
	case 4:																		/* 2400 bps						*/
		FBcom_Timer_5_Value = 10;												/* �����ԊĎ���ϰ 2*10=20 msec	*/
		FBcom_Timer_2_Value = 230;												/* ��M�ް���ϰ	20*230=4600 msec*/
		break;																	/*								*/
	default:																	/* ���̑��� 38400bps			*/
		bps = 0;																/* �ްڰĂ�������0�ɂ���		*/
		FBcom_Timer_5_Value = 2;												/* �����ԊĎ���ϰ 2*2=4 msec	*/
		FBcom_Timer_2_Value = 30;												/* ��M�ް���ϰ	20*50=1000 msec	*/
		break;																	/*								*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF�ՒʐMMAIN																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Main()																				   |*/
/*| PARAMETER	: par1	: 																					   |*/
/*|				: par2	: 																					   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
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
	case	1:																	/* 1= ��M����					*/
		FBcom_Event1();															/*								*/
		break;																	/*								*/
	case	2:																	/* 2= �ʐM�װ					*/
		FBcom_Event2();															/*								*/
		break;																	/*								*/
	case	3:																	/* 3= �ʐM��ѱ��				*/
		FBcom_Event3();															/*								*/
		break;																	/*								*/
	case	4:																	/* 4= �ʐMײ���ѱ��				*/
		FBcom_Event4();															/*								*/
		break;																	/*								*/
	case	5:																	/* 5= ��M�ޯ̧���ް�۰			*/
		FBcom_Event5();															/*								*/
		break;																	/*								*/
	default:																	/*								*/
		break;																	/*								*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	����Ĕ�������																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SubEventCheck()																		   |*/
/*| PARAMETER	: par1	: 																					   |*/
/*|				: par2	: 																					   |*/
/*| 			: par3	: 																					   |*/
/*| 			: par4	: 																					   |*/
/*| 			: par5	: 																					   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	FBcom_SubEventCheck( void )												/*								*/
{																				/*								*/
	if( 0 != FBcomdr_f_RcvCmp ){												/* ��M����Ă���				*/
		return (uchar)FBcomdr_f_RcvCmp;											/*								*/
	}																			/*								*/
																				/*								*/
	if( FBcom_20mTimeout() != 0 ){												/* ��ѱ�Ĳ����(ENQ�Ԋu)			*/
		return (uchar)4;														/*								*/
	}																			/*								*/
	return (uchar)0;															/* ����Ĕ�������				*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	��M�ް�����																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_Event1()																			   |*/
/*| PARAMETER	: none 																						   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
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
		FBcomdr_f_RcvCmp = 0;													/* ����ĸر						*/
		FBcom_20mTimStart(FBcom_Timer_4_Value);									/*								*/
		return;
	}
	
	switch(	(retc = FBcom_SubRcvAnalysis(pData, length)) )						/*								*/
	{																			/* ��M�ް��̉��				*/
																				/*								*/
	case 1:																		/* �ް�����-NG					*/
	case 4:																		/* ADR�Ȃ�						*/
	case 6:																		/* ���亰��-NG					*/
	case 7:																		/* �ް�ID-NG					*/
	case 9:																		/* TEXT���TEXT					*/
		// NOTE:FT-4000�V���[�Y�̏ꍇ�v�����^�󎚂��s���Ă��鎞�Ƀv�����^�ʐM��I2C�o�X�o�R�̂���FB�^�X�N���x���ł�
		// �p�P�b�g��M�������ǂ����Ȃ��Ȃ�ꍇ������̂ŁA�G���[�̔����͘A��3��ȏ�Ƃ����背�x����������
		com_err_cnt++;
		if(com_err_cnt >= 3){
			com_err_cnt = 0;
			err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 1, 0, 0 );
		}
		FBcom_20mTimStart(FBcom_Timer_4_Value);									/*								*/
		break;																	/* ����							*/
																				/*								*/
	case 0:																		/* �����ް���M					*/
		com_err_cnt = 0;
		err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 0, 0, 0 );						/* 00:ذ�ޒʐM�ُ����			*/
	case 5:																		/* CRC16-NG						*/
	case 8:																		/* Length����					*/
		FBcom_20mTimStop();														/*								*/
		FBcom_MatrixCheck( retc, &FBcomTerm, pData );							/* ��ظ�����					*/
		break;																	/*								*/
	}																			/*								*/
	FBcomdr_f_RcvCmp = 0;														/* ����ĸر						*/
// MH810103 GG119202(S) IC�N���W�b�g�Ή��iPCI-DSS�j
	memset(FBcomdr_RcvData, 0, sizeof(FBcomdr_RcvData));
// MH810103 GG119202(E) IC�N���W�b�g�Ή��iPCI-DSS�j
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�رٺЭƹ���ݴװ����																					   |*/
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
	if(FBcomdr_SciErrorState & 0x01) {											// �I�[�o�[�����G���[
		err_chk( ERRMDL_READER, ERR_RED_SCI_OVERRUN, 2, 0, 0 );					// ��������
	}
	else if(FBcomdr_SciErrorState & 0x02) {										// �t���[�~���O�G���[
		err_chk( ERRMDL_READER, ERR_RED_SCI_FRAME, 2, 0, 0 );					// ��������
	}
	else if(FBcomdr_SciErrorState & 0x04) {										// �p���e�B�G���[
		err_chk( ERRMDL_READER, ERR_RED_SCI_PARITY, 2, 0, 0 );					// ��������
	}
	FBcomdr_SciErrorState = 0;
	pData = FBcom_GetSyncDataIndex(&FBcomdr_RcvData[0], FBcomdr_RcvLength, &length);
	if(pData) {
		if(length >= 3 && FB_REQ_STX == *(pData + 2)) {
			// �f�[�^��M(STX)�̃G���[�Ȃ�NAK�𑗐M����
			FBcom_MatrixCheck( FB_REQ_NAK, &FBcomTerm, pData );					/* ��ظ�����					*/
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
	FBcom_SndCtrlPkt( FB_REQ_ENQ, 1, 0 );										/*�yENQ�z���M					*/
	FBcom_InitMatrix();
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	POL/SEL����																								   |*/
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
	FBcomdr_f_RcvCmp = 0;														/* ����ĸر						*/

	if(FB_MainErr.bits.CommErr == 0 && (FBcomTerm.DtSndNG > 0 || FBcomTerm.DtRepNG > 0)) {
		// �ʐM�G���[���������Ă��Ȃ���ԂŁANAK��M�ɂ��đ�����ENQ�𑗐M���Ȃ�
		return;
	}
	
	if(FBcomTerm.RcvResFlg == 1) {
		// ����������񐔃N���A
		FBcomTerm.NoAnsCnt = 0;
	}
	else {
		// ENQ�Ԋu���ɉ����Ȃ�
		++FBcomTerm.NoAnsCnt;
		if(FBcomTerm.NoAnsCnt >= FB_NORSP_CNT) {
			// ����������񐔒���
			err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 1, 0, 0 );					/* �װ����						*/
			FBcomTerm.NoAnsCnt = 0;
		}
	}
	FBcomTerm.RcvResFlg = 0;

	FBcomTerm.Matrix = FB_S1;													/* �߰�ݸތ�̉����҂�(EMPTY)	*/
	FBcom_SndCtrlPkt(FB_REQ_ENQ, 0, 0);											/* �����ް����M					*/
}																				/*								*/

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�رٺЭƹ���ݴװ����																					   |*/
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
	FBcom_SndCtrlPkt( FB_REQ_ENQ, 1, 0 );										/*�yENQ�z���M					*/
	FBcom_InitMatrix();
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	��M�ް����																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SubRcvAnalysis()																	   |*/
/*| PARAMETER	: pData	: ��M�f�[�^																		   |*/
/*|				: size	: ��M�f�[�^�T�C�Y																	   |*/
/*| RETURN VALUE: ret	: 0		: �f�[�^OK																	   |*/
/*						: 1		: �f�[�^�T�C�YNG															   |*/
/*						: 4		: �f�[�^�s���iADR�Ȃ��j														   |*/
/*						: 5		: �f�[�^�s���iBCC�G���[�j													   |*/
/*						: 6		: �f�[�^�s���i����R�[�hNG�j												   |*/
/*						: 7		: �f�[�^�s���i��MIDNG�j													   |*/
/*						: 8		: �f�[�^�s���i�f�[�^���s��v�j												   |*/
/*						: 9		: �f�[�^�s���i�}�g���N�X�ُ�j												   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�E��M�ް��ɑ΂��Ė������ׂ��ް��Ɖ������ׂ��ް��̔�����s��											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	FBcom_SubRcvAnalysis( uchar *pData, ushort size )						/*								*/
{																				/*								*/
																				/*								*/
ushort			ctrlCode;
ushort			dataSize;
uchar			bcc;
																				/*								*/
	/*** �߹�ļ��ݼ�ه����� ***/												/*								*/
	if( pData[0] != 0x32) {														/* ADR?					 		*/
		return 4;;
	}

	/*** �ް�ID&���亰�ޔ��� ***/												/*								*/
	ctrlCode = *(pData + 1);
	switch( ctrlCode ){															/* ���亰������					*/
	case FB_REQ_ACK:															/* [ACK]						*/
	case FB_REQ_NAK:															/* [NAK]						*/
		if(size != 2) {
			return 1;															/* �ް�����NG					*/
		}
		break;
	case FB_REQ_EOT:															/* [EOT]						*/
		if(size != 6) {
			return 1;															/* �ް�����NG					*/
		}
		break;																	/*								*/
	default:
		switch(*(pData + 2)) {
		case FB_REQ_STX:														/*								*/
			if(	(pData[FB_PKT_COMMAND] == FB_RCV_VERS_ID) ||					/* �ް�ޮ����					*/
				(pData[FB_PKT_COMMAND] == FB_RCV_READ_ID) ||					/* ذ���ް�						*/
				(pData[FB_PKT_COMMAND] == FB_RCV_END_ID) ||						/* �I���ް�						*/
				(pData[FB_PKT_COMMAND] == FB_RCV_SENSOR_ID) ||					/* �ݻ����						*/
				(pData[FB_PKT_COMMAND] == FB_RCV_MNT_ID) ) {					/* ����ݽ����					*/
																				/*								*/
				dataSize = (ushort)pData[4];
				if( dataSize + 7 > size ){										/* �߹���ް�������				*/
					return( 8 );												/*								*/
				}																/*								*/
				if( FBcomTerm.Matrix == FB_S3 ){								/* �߹�đ��M��(����)			*/
					return( 9 );												/*								*/
				}																/*								*/
			
				// BCC�`�F�b�N
				bcc = (uchar)~bcccal( (char*)&pData[3], (ushort)(dataSize + 3) );
				if(bcc != pData[5 + dataSize + 1]) {
					return( 5 );												/*								*/
				}																/*								*/
			}else{																/*								*/
				return( 7 );													/* ��MID-NG(����)				*/
			}																	/*								*/
			break;
		default:
			return( 6 );															/* ���亰��NG(����)				*/
		}
		break;																	/*								*/
	}																			/*								*/
																				/*								*/
	return( 0 );																/* �ް�����OK					*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	��M�ް���ظ�����																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_MatrixCheck(par1,par2,par3)															   |*/
/*| PARAMETER	: result	: �f�[�^��͌���																   |*/
/*|				: dp		: �ʐM���\����																   |*/
/*| 			: pData		: ��M�f�[�^																	   |*/
/*| RETURN VALUE: none																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�E�������ׂ��ް��ɑ΂�����ظ�����������s��																   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
void	FBcom_MatrixCheck(														/*								*/
		uchar 		result,														/*								*/
		t_FBcomTerm *dp,														/*								*/
		uchar		*pData )													/* ��M�ް��ޯ̧				*/
{																				/*								*/
ushort	ret;																	/*								*/
																				/*								*/
	switch( *(pData + 1) )														/* ���䕶���̎擾				*/
	{																			/*								*/
	/***  EOT��M  ***/
	case FB_REQ_EOT:															/* [EOT]��M					*/
		switch( dp->Matrix )													/*								*/
		{																		/*								*/
		case FB_S0:		/* ƭ����	*/											/* ����ԁFƭ����				*/
			break;																/*								*/
																				/*								*/
		case FB_S1:		/* ENQ���M��	*/										/* ����ԁFENQ���M��			*/
			dp->Matrix	= FB_S0;												/* ƭ���قֈړ�					*/
			dp->ErrFlg.bits.CtCRC_NG = 0;										/* �װ�׸�ON					*/
			
			if(FBcomTerm.CtrlInfo.Count > 0) {
				// ���M�f�[�^������Α��M�������s��
				FBcom_SndDtPkt( &FBcomTerm, FB_REQ_STX, 0x01, 0 );
				FBcomTerm.Matrix = FB_S3;										/* �߹�đ��M�㉞���҂���		*/
			}
			
			if(memcmp(dp->status, pData + 2, sizeof(dp->status))) {
				// �O��̃X�e�[�^�X�Ɣ�r���āA�قȂ�ꍇ�͒ʒm����
				memcpy(dp->status, pData + 2, sizeof(dp->status));
				ret = FBcom_RcvDataSave(pData, 1 );								/* ��M�����ް����捞			*/
				cr_rcv();
			}
			break;																/*								*/
																				/*								*/
		case FB_S3:		/* �ް����M��	*/										/* ����ԁFTEXT�㉞��			*/
			dp->Matrix	= FB_S0;												/* ƭ���قֈړ�					*/
			dp->ErrFlg.bits.CtCRC_NG = 0;										/* �װ�׸�ON					*/
			break;																/*								*/
		}																		/*								*/
		break;																	/*								*/
																				/*								*/
	/***  ACK��M  ***/															/*								*/
	case FB_REQ_ACK:															/* [ACK]��M					*/
		switch( dp->Matrix )													/*								*/
		{																		/*								*/
		case FB_S0:		/* ƭ����	*/											/* ����ԁFƭ����				*/
			break;																/*								*/
																				/*								*/
		case FB_S1:		/* ENQ���M��	*/										/* ����ԁFENQ���M��			*/
			dp->Matrix	= FB_S0;												/* ƭ���قֈړ�					*/
			dp->SndDtID = 0;
			dp->ErrFlg.bits.CtCRC_NG = 0;										/* �װ�׸�ON					*/
			break;																/*								*/
																				/*								*/
		case FB_S3:		/* �ް����M��	*/										/* ����ԁFTEXT�㉞��			*/
			FBcom_SndDtDec();													/* �߹�Ĕj��(���M����)			*/
			dp->Matrix	= FB_S0;												/* ƭ���ق�						*/
			dp->DtSndNG = 0;
			dp->DtRepNG = 0;
			dp->SndDtID = 0;
			dp->ErrFlg.bits.CtCRC_NG = 0;										/* �װ�׸�						*/
			break;																/*								*/
		}																		/*								*/
		break;																	/*								*/
																				/*								*/
	/***  NAK��M�̎�  ***/
	case FB_REQ_NAK:															/*								*/
		switch( dp->Matrix ){													/*								*/
		case FB_S0:																/* ����ԁFƭ����				*/
		case FB_S1:																/* ����ԁFENQ���M��			*/
			break;																/*								*/
																				/*								*/
		case FB_S3:		/* �ް����M��	*/										/* ����ԁFTEXT���M��			*/
			if( dp->DtSndNG > (uchar)FB_DTSND_RETRY )							/* �ް��đ��񐔵��ް			*/
			{
				if(dp->DtRepNG <= (uchar)FB_DTSND_REPEAT) {
					// ���g���C�񐔂𒴂����ꍇ�́AENQ�𑗂�
					dp->Matrix	= FB_S0;										/* ƭ���ق�						*/
					dp->DtCrcNG	= 0;											/*								*/
					dp->DtSndNG = 0;											/*								*/
					dp->SndDtID = 0;
					
					//�ʐM�ُ펞�̃��g���C�J�E���^���X�V
					++dp->DtRepNG;

					FBcom_SndCtrlPkt( FB_REQ_ENQ, 1, 0 );						/*�yENQ�z���M					*/
					FBcomTerm.Matrix = FB_S1;									/* �߰�ݸތ�̉����҂�(EMPTY)	*/
				}
				else {
					// �ʐM�ُ펞�̃��g���C�񐔃I�[�o�[�ōđ���������ߑ����M�f�[�^�폜
					dp->Matrix	= FB_S0;										/* ƭ���ق�						*/
					dp->DtCrcNG	= 0;											/*								*/
					dp->DtSndNG = 0;											/*								*/
					dp->DtRepNG = 0;
					dp->SndDtID = 0;
					FBcom_SndDtDec();											/* �߹�ď���					*/
					err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 1, 0, 0 );			/* �װ����/�����ʒm				*/
				}
			}																	/*								*/
			else
			{
				// ���g���C�񐔖����͍đ�����
				FBcom_SndDtPkt( dp, FB_REQ_STX, 0x00, dp->SndDtID);				/* �߹�đ��M					*/
				dp->DtSndNG ++;													/* NAK��ײ���M����Up			*/
				dp->Matrix = FB_S3;												/* �߹�ĉ����҂��ֈړ�			*/
			}																	/*								*/
			break;																/*								*/
		}																		/*								*/
		break;																	/*								*/
	default:
		switch(*(pData + 2)) {
		/***  DATA��M�̎�  ***/
		case FB_REQ_STX:														/*								*/
			switch( dp->Matrix )												/*								*/
			{																	/*								*/
			case FB_S0:															/* ����ԁFƭ����				*/
				break;															/*								*/
																				/*								*/
			case FB_S1:		/* ENQ���M��	*/									/* ����ԁFENQ���M��			*/
				switch( result )												/* ���茋��=BCC-NG				*/
				{																/*								*/
				case FB_BCC_NG:													/* ���茋��=BCC-NG				*/
				case FB_SERIAL_NG:												/* �رٴװ����					*/
					if( dp->DtRcvNG > FB_DTRCV_RETRY )							/* BCC-NG��ײ���ް				*/
					{															/*								*/
						FBcom_SndCtrlPkt(FB_REQ_ACK, 1, 0 );					/* ����ACK�I���v��				*/
						dp->DtRcvNG	= 0;										/*								*/
						dp->Matrix	= FB_S0;									/* ƭ���قֈړ�					*/
						/*--------------------------------------------------*/
						/* ��M�ް�CRC-NG����ײ�񐔵��ް�F����ACK���M		*/
						/*--------------------------------------------------*/
						err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 1, 0, 0 );		/* �װ����/�����ʒm				*/
					}															/*								*/
					else														/*								*/
					{															/*								*/
						FBcom_SndCtrlPkt(FB_REQ_NAK, 1, 0);						/*�yNAK�zBCC-NG 80H				*/
						dp->Matrix = FB_S1;										/* POL����ް��҂�				*/
						dp->DtRcvNG ++;											/* BCC-NG						*/
					}															/*								*/
					break;														/*								*/
				default:														/* �����ް���M					*/
					ret = FBcom_RcvDataSave(pData, 0 );							/* ��M�����ް����捞			*/
					if( ret == 1 ){												/*								*/
						FBcom_SndCtrlPkt(FB_REQ_EOT, 1, 1);						/*�yEOT�z���M 80H:�ޯ̧FULL		*/
					}else{														/*								*/
						FBcom_SndCtrlPkt(FB_REQ_ACK, 1, 0);						/*�yACK�z���M					*/
					}															/*								*/
					dp->DtRcvNG	= 0;											/*								*/
					dp->Matrix = FB_S0;											/* ƭ���قֈړ�					*/
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
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}																			/*								*/
	if( dp->Matrix == FB_S0 ){													/*								*/
		FBcom_20mTimStart( FBcom_Timer_4_Value );								/* POL/SEL���Mظ���				*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�����߹�đ��M�v��																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SndCtrlPkt(par1,par2,par3,par4)													   |*/
/*| PARAMETER	: ctrl	: ���亰�ގ�� 0=ENQ/1=ACK/2=NAK													   |*/
/*|				: blk	: �]����ۯ���(ENQ�̎��̂ݗL��)														   |*/
/*|				: t_st	: �[���ð��																			   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�EAKC/NAK/ENQ���̐����߹�Ă�SCI�ޯ̧�־�Ă����M����														   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBcom_SndCtrlPkt(														/*								*/
uchar		ctrl,																/* ���亰��						*/
uchar		blk,																/* ��ۯ���(ENQ�̎��̂ݗL��)		*/
uchar		t_st )																/* �[���ð��(FULL/EMPTY)		*/
{																				/*								*/
	ushort	Length;																/* �ް���						*/
																				/*								*/
	memset( &FB_SndBuf, 0x00, (uchar)(FB_ASCII_PKT_HEAD_SIZ+2) );				/* ���M�ް��ޯ̧�ر				*/
																				/*								*/
	FB_SndBuf[0] = '1';
	FB_SndBuf[1] = '6';
	FB_SndBuf[2] = '1';
	FB_SndBuf[3] = '6';
	FB_SndBuf[4] = '1';
	FB_SndBuf[5] = '6';
	FB_SndBuf[6] = '3';
	FB_SndBuf[7] = '2';
	switch( ctrl ){																/* ���亰��						*/
	case FB_REQ_ACK:															/* �yACK�z						*/
		FB_SndBuf[8]	= '0';
		FB_SndBuf[9]	= '6';
		break;																	/*								*/
	case FB_REQ_NAK:															/* �yNAK�z						*/
		FB_SndBuf[8]	= '1';
		FB_SndBuf[9]	= '5';
		break;																	/*								*/
	case FB_REQ_ENQ:															/* �yENQ�z						*/
		FB_SndBuf[8]	= '0';
		FB_SndBuf[9]	= '5';
		break;																	/*								*/
	}																			/*								*/
																				/*								*/
	Length = (ushort)FB_ASCII_PKT_HEAD_SIZ;										/* ͯ�ް������					*/
																				/* 								*/
	FBcomTerm.SndDtID = FB_SND_CTRL_ID;											/*								*/
	FBcom_WaitSciSendCmp( Length );												/* send complete wait			*/
																				/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�ް��߹�đ��M�v�����																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SndDtPkt(par1,par2,par3)															   |*/
/*| PARAMETER	: dp	: �[��No.�ʉ�����																	   |*/
/*|				: code	: 0=MAIN����̗v���ް�/���M�ް�ID=���M�w�肷���ް�ID								   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�E���ع���݂���n���ꂽ���M�ް���SCI���M�ޯ̧�־�Ă����M����											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
ushort	FBcom_SndDtPkt( 														/*								*/
t_FBcomTerm		*dp,															/* ����Ǘ�						*/
uchar	code,																	/* ���亰��						*/
uchar	stat,																	/* ����ð��					*/
uchar	typ																		/* 0=�V�K/1=�đ�				*/
)																				/*								*/
{																				/*								*/
	ushort		Length;															/*								*/
	ushort		ret;
	t_FBsendDataBuffer* pSendData;
																				/*								*/
	if( typ == 0 ){																/*								*/
		if(	dp->CtrlInfo.Count == 0 ){											/* �ް�����						*/
			return( 0 );														/*								*/
		}																		/*								*/
	}																			/*								*/
	memset( &FB_SndBuf[0], 0x00, FB_SCI_SNDBUF_SIZE );							/* ���M�ޯ̧�ر					*/
																				/*								*/
	FB_SndBuf[0] = '1';		// SYN
	FB_SndBuf[1] = '6';
	FB_SndBuf[2] = '1';		// SYN
	FB_SndBuf[3] = '6';
	Length = 4;
	
	// ASCII�ϊ���ɑ��M�o�b�t�@�Ɋi�[
	pSendData =	&FBcomTerm.SendData[FBcomTerm.CtrlInfo.R_Index];
	Length += FBcom_BinaryToAscii(pSendData->buffer, pSendData->size, &FB_SndBuf[4]);
	dp->SndDtID = FB_SND_DATA_ID;												/*								*/
																				/*								*/
	ret = (ushort)FBcom_WaitSciSendCmp( Length );								/*								*/
	
	return	ret;																/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�ް��߹�đ��M�v��																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_WaitSciSendCmp()																	   |*/
/*| PARAMETER	: Length	: ���M�f�[�^��																	   |*/
/*| RETURN VALUE: ret	: 0 : ���M����																		   |*/
/*| 			: 		: 1 : ���M�f�[�^���� or �^�C���A�E�g												   |*/
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
		taskchg( IDLETSKNO );													/* �^�X�N�ؑ�					*/
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
/*|	���M�ς��ް�����																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_SndDtDec(void)																		   |*/
/*| PARAMETER	: Non	: 																					   |*/
/*| RETURN VALUE: ret	: ���M�ް�����Ǘ�ð��ق̌���														   |*/
/*| 			: 		: 0xFFFF = �����װ																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�EIF�Ղ֑��M�ς݂��ް��߹�Ă���������																	   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
ushort	FBcom_SndDtDec( void )													/*								*/
{																				/*								*/
	ushort	ret=0xFFFF;

	/* �������ް��������ݒ��ް��̎� */											/* 								*/
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
/*|	��M�ް��߹�ľ���																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: FBcom_RcvDataSave()																		   |*/
/*| PARAMETER	: pData	: ��M�f�[�^																		   |*/
/*|				: typ	: �i�[����	0:������ް��̂ݱ����ޯ̧�֊i�[											   |*/
/*|									1:������ް����ꎞ�ޯ̧�֊i�[											   |*/
/*|									2:����ƈꎞ�ޯ̧���ް�������ޯ̧�֊i�[								   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�E0:����M�����ް�����ع���݂ւ̎�M�ް��ޯ̧�֊i�[													   |*/
/*|     FBcomdr_RcvBuf�Ɋi�[����Ă����ް�������ޯ̧FBcom_RcvData�֊i�[����								   |*/
/*| �E1:�����M����1��ۯ��ް����ꎞ�ۗ��ޯ̧�֎w�肵����ۯ��ԍ��̈ʒu�Ɋi�[����(�����F������ۯ��p)			   |*/
/*| �E2:�����M�����ް��ƈꎞ�ޯ̧���ް�������ޯ̧�֊i�[����(�����F������ۯ��p)							   |*/
/*| �y�\���̍\���z																							   |*/
/*|		�ް�����	2 byte(2+1+�ް�)																		   |*/
/*|		�ް�ID4		1 byte																					   |*/
/*|		�ް�		**** byte(ID1�`CRC�̎�O)																   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
																				/*								*/
uchar	FBcom_RcvDataSave(														/* ��M�����ް����捞			*/
uchar*	pData,
uchar	typ )																	/* �i�[����						*/
{																				/*								*/
	switch( typ ){																/*								*/
																				/*								*/
	case 0:		// �f�[�^
		// ToDo: �f�[�^��M��ɂ����ɒʒm����̂ŁA�o�b�t�@��1�f�[�^���Ƃ���
		//		 �����f�[�^��ێ�����ꍇ�̓t���b�v�̏������Q�l�ɂ���
		memcpy( &FBcom_RcvData, pData, (size_t)pData[4] + 5 );					/* ADR�����ް�����				*/
		break;																	/*								*/
																				/*								*/
	case 1:		// EOT
		memcpy( &FBcom_RcvData, pData, 6);										/* ADR�����ް�����				*/
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
/*|		���M�f�[�^�i�[																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	FBcom_SetSendData(uchar* pSendData, ushort size)
{
	if( FB_SENDDATA_QUEUE_MAX <= FBcomTerm.CtrlInfo.Count ){					/* �ޯ̧FULL���				*/
		++FBcomTerm.CtrlInfo.R_Index;											/* �Â��ް�������				*/
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
/*|		�o�C�i����ASCII�ϊ�																					   |*/
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
/*|		ASCII�ϊ����o�C�i��																					   |*/
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
		
		if(asciiIndex & 0x0001) {			// ���
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
/*|		SYN��T��SYN�ȊO�̃f�[�^�̃|�C���^��Ԃ�															   |*/
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
					// �Ō��SYN����SYN�ȊO�|�C���^��Ԃ�
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
/*|		�����}�g���b�N�X����������ENQ���M�҂��Ƃ���															   |*/
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
	FBcomdr_f_RcvCmp = 0;														/* ����ĸر						*/
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcom_SetReceiveData()																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		�f�[�^����M�������Ƃ��L�^����																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void 		FBcom_SetReceiveData( void )
{
	FBcomTerm.RcvResFlg = 1;
}
