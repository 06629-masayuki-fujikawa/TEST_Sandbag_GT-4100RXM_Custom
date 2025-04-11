#include "iodefine.h"															/*								*/
#include "system.h"																/*								*/
#include "fbcom.h"																/*								*/
																				/*								*/
void		FB_Int_RXI( void );													/*								*/
void		FB_Int_ERI( void );													/*								*/
uchar		FBsci_SndReq( ushort );												/*								*/
uchar		FBsci_IsSndCmp( void );												/*								*/
void		FB_Int_TXI( void );													/*								*/
void		FB_Int_TEI( void );													/*								*/
void		FBsci_Stop( void );													/*								*/

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      FB_Int_RXI																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI Receive/Receive Error interrupt routine														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
/*----------------------*/														/*								*/
/*	文字間で見る方式	*/														/*								*/
/*----------------------*/														/*								*/
void	FB_Int_RXI( void )														/*								*/
{																				/*								*/
	volatile uchar   rcvdat;													/*								*/
																				/*								*/
	/************************************************************/
	/*	RX630のSSRにはRDRFがないので							*/
	/*	ORER(Overrun) FER(Framing) PER(Parity)のErrorがなければ	*/
	/*	ReadしてReadDataを格納する(RDRF <-0もなし)				*/
	/************************************************************/
	if((SCI3.SSR.BIT.ORER==0) && (SCI3.SSR.BIT.FER==0) && (SCI3.SSR.BIT.PER==0)) {
		rcvdat = SCI3.RDR;														/*								*/
																				/*								*/
FB_Sci_Rcv_10:																	/*								*/
		if( FB_SCI_RCVBUF_SIZE > FB_RcvCtrl.RcvCnt ){							/* receive buffer full (N)		*/
			FB_RcvBuf[ FB_RcvCtrl.WriteIndex ] = rcvdat;						/* received data set			*/
			++FB_RcvCtrl.RcvCnt;												/* counter up					*/
																				/*								*/
			if( FB_RcvCtrl.RcvCnt == 1 ){										/* 1文字目受信					*/
				FBcom_SetReceiveData();
				FBcom_20mTimStart2( FBcom_Timer_2_Value );						/* 文字受信監視開始				*/
				FBcom_20mTimStop();												/*								*/
			}																	/*								*/
																				/*								*/
			/* write index update */											/*								*/
			if( (FB_SCI_RCVBUF_SIZE - 1) > FB_RcvCtrl.WriteIndex ){				/*								*/
				++FB_RcvCtrl.WriteIndex;										/*								*/
			}																	/*								*/
			else {																/*								*/
				FB_RcvCtrl.WriteIndex = 0;										/*								*/
			}																	/*								*/
		}																		/*								*/
		else {		/* receive buffer overflow count update */					/*								*/
			FB_RcvCtrl.OvfCount = 1;											/*								*/
		}																		/*								*/
		SCI3.SSR.BYTE = 0;														// 割り込み要因のクリア
		dummy_Read = SCI3.SSR.BYTE;												/* Dummy Read					*/
		if( FB_RcvCtrl.OvfCount != 1 ){											/*								*/
			FBcom_2mTimStart( FBcom_Timer_5_Value );							/* 文字間ﾀｲﾏｰ起動				*/
		}																		/*								*/
	}																			/*								*/
	else{																		/* RDRF=0, ORER=1, FER=0, PER=0 */
		rcvdat = 0xff;															/*								*/
		goto FB_Sci_Rcv_10;														/*								*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      FB_Int_ERI																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI Receive Error interrupt routine																   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FB_Int_ERI(void)														/*								*/
{																				/*								*/
	volatile unsigned char	c;
																				/*								*/
	if( SCI3.SSR.BIT.ORER )														/* over run error ?				*/
	{																			/*								*/
		FB_RcvCtrl.ComerrStatus |= 0x01;										/* over run set					*/
	}																			/*								*/
	if( SCI3.SSR.BIT.FER )														/* frame error ?				*/
	{																			/*								*/
		FB_RcvCtrl.ComerrStatus |= 0x02;										/* frame error set				*/
	}																			/*								*/
	if( SCI3.SSR.BIT.PER )														/* parity error ?				*/
	{																			/*								*/
		FB_RcvCtrl.ComerrStatus |= 0x04;										/* parity error set				*/
	}																			/*								*/
	c = SCI3.RDR;																// Received Data
	SCI3.SSR.BIT.ORER = 0 ;														// ERROR clear
	SCI3.SSR.BIT.FER = 0 ;
	SCI3.SSR.BIT.PER = 0 ;
	SCI3.SSR.BYTE &= 0xc7;
	dummy_Read = SCI3.SSR.BYTE;													/* Dummy Read					*/
}																				/*								*/
																				/*								*/
/***************************/
/***	send process	 ***/
/***************************/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [API]      FBsci_SndReq																		 		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI character string send request																   |*/
/*|			You can check send complete ot not, that next routine.											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Input  : send string set in "IFC_SndBuf[]". (NULL need)													   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	FBsci_SndReq( ushort Length )											/*								*/
{																				/*								*/
																				/*								*/
	SCI3.SCR.BIT.TE = 1;
	if( (SCI3.SSR.BIT.ORER==0) && (SCI3.SSR.BIT.FER==0) && (SCI3.SSR.BIT.PER==0) ){	/* send enable (Y)			*/
		FB_SndCtrl.SndCmpFlg = 0;												/* send complete flag clear(OFF)*/
																				/*								*/
		FB_SndCtrl.SndReqCnt = Length;											/*								*/
		FB_SndCtrl.SndCmpCnt = 0;												/*								*/
		FB_SndCtrl.ReadIndex = 1;												/*								*/
																				/*								*/
		SCI3.TDR = FB_SndBuf[0];												/* send charcter set			*/
		SCI3.SSR.BIT.ORER= 0;													/* send start					*/
		SCI3.SSR.BIT.FER= 0;													/* 								*/
		SCI3.SSR.BIT.PER= 0;													/* 								*/
		dummy_Read = SCI3.SSR.BYTE;												/* Dummy Read					*/
																				/*								*/
		SCI3.SCR.BIT.TIE = 1;													/* interrupt enable				*/
		return (unsigned char)0;												/*								*/
	}																			/*								*/
	else {																		/* send invalid					*/
		FB_SndCtrl.SndCmpFlg = 1;												/* send complete flag set (ON)	*/
		SCI3.SCR.BIT.TEIE = 0;													/* interrupt enable (TEI)		*/
		SCI3.SCR.BIT.TIE = 0;													/* send interrupt disable		*/
		dummy_Read = SCI3.SSR.BYTE;												/* Dummy Read					*/
		return (unsigned char)1;												/*								*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [API]      FBsci_IsSndCmp																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI character string send complete check														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| return : 1=completed,  0=not complete yet.																   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	FBsci_IsSndCmp( void )													/*								*/
{																				/*								*/
	if( 1 == FB_SndCtrl.SndCmpFlg ){											/*								*/
		return (uchar)1;														/*								*/
	}																			/*								*/
	return (uchar)0;															/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      FB_Int_TXI																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI possible to send interrupt routine															   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| input  : Groval area use => IFC_SndBuf, IFC_SndCtrl														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FB_Int_TXI( void )														/*								*/
{																				/*								*/
																				/*								*/
	if((SCI3.SSR.BIT.ORER==0) && (SCI3.SSR.BIT.FER==0) && (SCI3.SSR.BIT.PER==0)){	/* send enable (Y)				*/
		++FB_SndCtrl.SndCmpCnt;													/* send completed count up		*/
		if( FB_SndCtrl.SndReqCnt > FB_SndCtrl.SndCmpCnt ){						/* not end yet (Y)				*/
			SCI3.TDR = FB_SndBuf[ FB_SndCtrl.ReadIndex++];						/* send charcter set			*/
			SCI3.SSR.BIT.ORER= 0;												/* send start					*/
			SCI3.SSR.BIT.FER= 0;												/* 								*/
			SCI3.SSR.BIT.PER= 0;												/* 								*/
			dummy_Read = SCI3.SSR.BYTE;											/* Dummy Read					*/
																				/*								*/
			SCI3.SCR.BIT.TIE = 1;												/* interrupt enable				*/
		}																		/*								*/
		else {																	/* all data send end			*/
			SCI3.SCR.BIT.TIE = 0;												/* send enable interrupt inhibit*/
			SCI3.SCR.BIT.TEIE = 1;												/*								*/
			FB_SndCtrl.SndCmpFlg = 1;											/* send complete flag set 		*/
		}																		/*								*/
	}																			/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      FB_Int_TEI																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI send end interrupt routine																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FB_Int_TEI( void )														/*								*/
{																				/*								*/
	/* I don't use this event */												/*								*/
	SCI3.SCR.BIT.TE = 0;
	SCI3.SCR.BIT.TEIE = 0;														/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      FBsci_Stop																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI send & receive stop																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・この関数は未使用																						   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	FBsci_Stop( void )														/*								*/
{																				/*								*/
	unsigned long	ist;														/* 現在の割込受付状態			*/
																				/*								*/
	ist = _di2();																/*								*/
																				/*								*/
	SCI3.SCR.BIT.TIE  = 0;														/* 送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ(TXI)割込み禁止*/
	SCI3.SCR.BIT.RIE  = 0;														/* 受信割込み禁止 				*/
	SCI3.SCR.BIT.TE   = 0;														/* 送信動作禁止 				*/
	SCI3.SCR.BIT.RE   = 0;														/* 受信動作禁止 				*/
	SCI3.SCR.BIT.TEIE = 0;														/* 送信終了(TEI)割り込み禁止 	*/
	dummy_Read = SCI3.SSR.BYTE;													/* Dummy Read					*/
																				/*								*/
	_ei2( ist );																/*								*/
																				/*								*/
}																				/*								*/
