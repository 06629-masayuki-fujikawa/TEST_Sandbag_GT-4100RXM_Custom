/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		SCI通信ドライバー関数																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	ksg_RauSci.c																			   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・SCI通信ドライバ																						   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/

#include "system.h"																/*								*/
#include "prm_tbl.h"
#include "iodefine.h"
#include "ksgmac.h"
#include "ksgRauSci.h"															/*								*/
#include "ksg_def.h"
#include "ksgRauTable.h"
#include "ksgRauModemData.h"													/* ﾓﾃﾞﾑ関連ﾃﾞｰﾀ定義				*/
#include "ksgRauModem.h"

void		SCI0_rcv_block( void );												/*								*/

t_RauSciRcvCtrl	KSG_RauRcvCtrl;													/*								*/
t_RauSciSndCtrl	KSG_RauSndCtrl;													/*								*/
uchar			KSG_RauSndBuf[ KSG_RAU_SCI_SNDBUF_SIZE ];						/*								*/
																				/*								*/
//uchar			KSG_RauComRcvBuffFull;											/* 受信ﾊﾞｯﾌｧFULL				*/
																				/*								*/
uchar			KSG_RauFrameFlag;												// フレーム検出フラグ
uchar			KSG_RauPrevHDLCData;											// 1つ前のHDLCデータ

extern t_RauComQueCtrl				KSG_RauComdrDataQueueCtrl;
extern t_KSG_RauComdrDataQueue		KSG_RauComdrDataQueue[KSG_RauComdr_QUEUE_COUNT];
t_KSG_RauComdrDataQueue*			KSG_RauComdrpQueue;
																				/*								*/
uchar			KSG_RauComdr_f_RcvCmp;											/*								*/

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      KSG_RauSCI_Init																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI初期化																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2012-09-10																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void KSG_RauSCI_Init( void )
{																				/*								*/
	unsigned long	exrreg;														/* 								*/
	volatile unsigned char	dummy;												/*								*/
	exrreg = _di2();

	// SCI7 module stop mode release(SYSTEM.MSTPCRB.BIT.MSTPB24)
	MSTP_SCI7 = 0;
	SCI7.SEMR.BIT.ABCS = 0x01;													/* 調歩同期基本クロックセレクトBIT	*/
	SCI7.SCR.BYTE = 0x00;														/* 送受信禁止/割込み禁止		*/
	SCI7.SMR.BIT.CM	= 0;														/* 調歩同期式					*/
	SCI7.SMR.BIT.MP	= 0;														/* ﾏﾙﾁﾌﾟﾛｾｯｻ未使用				*/
																				/*								*/
	_ei2( exrreg );

	SCI7.SMR.BIT.CHR = 0;														/* Data bits  : 0=8bit, 1=7bit	*/
	switch(KSG_RauConf.parity_bit){
		case 0:																	/* パリティbit なし				*/
			SCI7.SMR.BIT.PE = 0;												/* Parity有無 : 0=None, 1=Use	*/
			SCI7.SMR.BIT.PM = 0;												/* ParityType : 0=Even, 1=Odd	*/
			break;
		case 1:																	/* パリティbit 奇数				*/
			SCI7.SMR.BIT.PE = 1;												/* Parity有無 : 0=None, 1=Use	*/
			SCI7.SMR.BIT.PM = 0;												/* ParityType : 0=Even, 1=Odd	*/
			break;
		case 2:																	/* パリティbit 偶数				*/
			SCI7.SMR.BIT.PE = 1;												/* Parity有無 : 0=None, 1=Use	*/
			SCI7.SMR.BIT.PM = 1;												/* ParityType : 0=Even, 1=Odd	*/
			break;
		default:
			SCI7.SMR.BIT.PE = 0;												/* Parity有無 : 0=None, 1=Use	*/
			SCI7.SMR.BIT.PM = 0;												/* ParityType : 0=Even, 1=Odd	*/
			break;
	}
	SCI7.SMR.BIT.STOP = KSG_RauConf.stop_bit;									/* ｽﾄｯﾌﾟﾋﾞｯﾄ					*/

	switch( prm_get(COM_PRM, S_CEN, 74, 1,4) ) {
		case 0:																	/* 38400bps						*/
			SCI7.SMR.BIT.CKS = 0;
			SCI7.BRR = SCI_38400BPS_n0;
			break;
		case 1:																	/* 19200bps						*/
			SCI7.SMR.BIT.CKS = 1;
			SCI7.BRR = SCI_19200BPS_n1;
			break;
		case 2:																	/*  9600bps						*/
			SCI7.SMR.BIT.CKS = 1;
			SCI7.BRR = SCI_9600BPS_n1;
			break;
		case 3:																	/*  4800bps						*/
			SCI7.SMR.BIT.CKS = 1;
			SCI7.BRR = SCI_4800BPS_n1;
			break;
		case 5:																	/*  57600bps					*/
			SCI7.SMR.BIT.CKS = 0;
			SCI7.BRR = SCI_57600BPS_n0;
			break;
		case 6:																	/*  115200bps					*/
			SCI7.SMR.BIT.CKS = 0;
			SCI7.BRR = SCI_115200BPS_n0;
			break;
		case 7:																	/*  230400bps					*/
			SCI7.SMR.BIT.CKS = 0;
			SCI7.BRR = SCI_230400BPS_n0;
			break;
		default:																/* 19200 ← ﾃﾞﾌｫﾙﾄ				*/
			SCI7.SMR.BIT.CKS = 1;
			SCI7.BRR = SCI_19200BPS_n1;
			break;
	}

	wait2us( 125L );															/* 1bit time over wait			*/
	dummy = SCI7.SSR.BYTE;														/*								*/
	SCI7.SSR.BYTE = 0x00;														/* ｽﾃｰﾀｽｸﾘｱ						*/
	SCI7.SSR.BYTE = 0xC4;														/* Status register <- 0			*/
	SCI7.SCR.BYTE = 0x70;														/* 送受信許可/割込み許可		*/
	KSG_ptr_rx_save = &KSG_mdm_r_buf[0];										/* 受信ﾃﾞｰﾀ格納					*/
	KSG_rx_save_len = 0;														/* 受信ﾃﾞｰﾀ格納数ｸﾘｱ			*/
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      KSG_Rau_Int_RXI																				   |*/
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
void	KSG_Rau_Int_RXI( void )													/*								*/
{																				/*								*/
	uchar   rcvdat;																/*								*/
																				/*								*/
	if( KSG_RauModemOnline != 1 ) {												/* ﾎｽﾄ回線未接続?				*/
		rcvdat = SCI7.RDR;														/* → 文字読取り				*/
		*KSG_ptr_rx_save = rcvdat;												/*	  受信ﾃﾞｰﾀ転送				*/
		if( KSG_rx_save_len < KSG_RCV_MAX_SCI7 ) {								/*    ﾊﾞｯﾌｧ空きあり?			*/
			KSG_ptr_rx_save++;	KSG_rx_save_len++;								/*	  ﾎﾟｲﾝﾀ加算					*/
		}																		/*								*/
		if( KSG_code_sts == 0x00 ) {											/*	  CR検出待ち?				*/
			if( rcvdat == KSG_CR ) KSG_code_sts = 0x01;							/*        LF検出待ち			*/
		}																		/*								*/
		else if( KSG_code_sts == 0x01 ) {										/*	  LF検出待ち?				*/
			if( rcvdat == KSG_LF ) {											/*      LF検出?					*/
				KSG_RauMdmResultCheck();										/*        ﾘｻﾞﾙﾄｺｰﾄﾞ解析			*/
				KSG_ptr_rx_save = &KSG_mdm_r_buf[0];							/*		  先頭へ移動			*/
				KSG_rx_save_len = 0x00;											/*        ｶｳﾝﾀｸﾘｱ				*/
			}																	/*		  						*/
			if( rcvdat != KSG_CR ) KSG_code_sts = 0x00;							/*        CR検出待ち			*/
		}																		/*								*/
		rcvdat = SCI7.SSR.BYTE;
		SCI7.SSR.BIT.ORER = 0;													/*    ﾌﾗｸﾞｸﾘｱ					*/
		SCI7.SSR.BIT.FER = 0;													/*    ﾌﾗｸﾞｸﾘｱ					*/
		SCI7.SSR.BIT.PER = 0;													/*    ﾌﾗｸﾞｸﾘｱ					*/
	}else{
		/* ホスト回線接続状態(PPP接続) */
		rcvdat = SCI7.RDR;														/*								*/
		if( KSG_RAU_SCI_RCVBUF_SIZE > KSG_RauRcvCtrl.RcvCnt ){					/* receive buffer full (N)		*/
			KSG_RauComdrpQueue = &KSG_RauComdrDataQueue[KSG_RauComdrDataQueueCtrl.W_Index];
			if(KSG_RauFrameFlag == 0) {											// フレーム開始を検出
				if(rcvdat == KSG_RAU_FLAG_SEQUENCE) {							// フラグシーケンス検出
					KSG_RauRcvCtrl.RcvCnt = 0;
					KSG_RauComdrpQueue->RcvData[KSG_RauRcvCtrl.RcvCnt] = rcvdat;
					KSG_RauPrevHDLCData = rcvdat;
				}
				else if(rcvdat == 0xff &&										// 直前がフラグシーケンスで0xffならアドレスとする
						KSG_RauPrevHDLCData == KSG_RAU_FLAG_SEQUENCE) {
					KSG_RauFrameFlag = 1;										// フレームの開始
					KSG_RauComdrpQueue->RcvData[KSG_RauRcvCtrl.RcvCnt] = rcvdat;
					KSG_RauPrevHDLCData = rcvdat;
				}
				else {															// フレームを検出していない場合は初期化する
					KSG_RauPrevHDLCData = 0;
					KSG_RauRcvCtrl.RcvCnt = 0;
				}
			}
			else if(KSG_RauFrameFlag == 1) {									// フレーム検出中
				KSG_RauComdrpQueue->RcvData[KSG_RauRcvCtrl.RcvCnt] = rcvdat;
				if(rcvdat == KSG_RAU_FLAG_SEQUENCE) {							// フレームの終端を検出
					KSG_RauComdrpQueue->RcvLength = KSG_RauRcvCtrl.RcvCnt + 1;	// 1フレームのサイズを設定
					KSG_RauFrameFlag = 0;
					++KSG_RauComdrDataQueueCtrl.W_Index;
					++KSG_RauComdrDataQueueCtrl.Count;
					
					if(KSG_RauComdrDataQueueCtrl.W_Index >= KSG_RauComdr_QUEUE_COUNT) {
						KSG_RauComdrDataQueueCtrl.W_Index = 0;
					}
					if(KSG_RauComdrDataQueueCtrl.Count > KSG_RauComdr_QUEUE_COUNT) {
						// オーバーフロー発生
						// 
						// ここではデータを格納していないので、まだデータを破壊していない
						// この先で受信パケットの処理をしていない最古のデータを上書きする
						//
						WACDOG;		// デバッガでbreakさせるため
						KSG_RauComdrDataQueueCtrl.Count = KSG_RauComdr_QUEUE_COUNT;
					}
					KSG_RauPrevHDLCData = 0x00;									// 0x7e, 0xff以外でクリア
				}
				else {
					KSG_RauPrevHDLCData = rcvdat;
				}
			}
			++KSG_RauRcvCtrl.RcvCnt;											/* counter up					*/
		}																		/*								*/
		else {																	/*								*/
			KSG_RauRcvCtrl.OvfCount = 1;										/* receive buffer overflow count update */
		}																		/*								*/
		SCI7.SSR.BIT.ORER = 0;													/*    ﾌﾗｸﾞｸﾘｱ					*/
		SCI7.SSR.BIT.FER = 0;													/*    ﾌﾗｸﾞｸﾘｱ					*/
		SCI7.SSR.BIT.PER = 0;													/*    ﾌﾗｸﾞｸﾘｱ					*/
		dummy_Read = SCI7.SSR.BYTE;												/* Dummy Read					*/
		KSG_RauComdr_f_RcvCmp = 0;
	}
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      KSG_Rau_Int_ERI																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI Receive Error interrupt routine																   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	KSG_Rau_Int_ERI(void)													/*								*/
{																				/*								*/
																				/*								*/
	if( SCI7.SSR.BIT.ORER )														/* over run error ?				*/
	{																			/*								*/
		KSG_RauRcvCtrl.ComerrStatus |= 0x01;									/* over run set					*/
	}																			/*								*/
	if( SCI7.SSR.BIT.FER )														/* frame error ?				*/
	{																			/*								*/
		KSG_RauRcvCtrl.ComerrStatus |= 0x02;									/* frame error set				*/
	}																			/*								*/
	if( SCI7.SSR.BIT.PER )														/* parity error ?				*/
	{																			/*								*/
		KSG_RauRcvCtrl.ComerrStatus |= 0x04;									/* parity error set				*/
	}																			/*								*/
	dummy_Read = SCI7.RDR;														/* Received Data				*/
	SCI7.SSR.BIT.ORER = 0 ;														/* ERROR clear					*/
	SCI7.SSR.BIT.FER = 0 ;
	SCI7.SSR.BIT.PER = 0 ;
	dummy_Read = SCI7.SSR.BYTE;													/* Dummy Read					*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [API]      KSG_RauSciSndReq																	 		   |*/
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
uchar	KSG_RauSciSndReq( ushort Length )										/*								*/
{																				/*								*/
																				/*								*/
	if( KSG_RauModemOnline == 1 ) {												/* ﾎｽﾄ回線接続中?				*/
		KSG_RauSndCtrl.SndCmpFlg = 0;											/* send complete flag clear(OFF)*/
		KSG_RauSndCtrl.SndReqCnt = Length;										/*								*/
		KSG_RauSndCtrl.SndCmpCnt = 0;											/*								*/
		KSG_RauSndCtrl.ReadIndex = 1;											/*								*/
																				/*								*/
		SCI7.TDR = KSG_RauSndBuf[0];											/* send charcter set			*/
		SCI7.SCR.BIT.TIE = 1;													/* interrupt enable				*/
		return 0;
	}
	else {
		return 1;
	}
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [API]      KSG_RauSciIsSndCmp																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI character string send complete check														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| return : 1=completed,  0=not complete yet.																   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	KSG_RauSciIsSndCmp( void )												/*								*/
{																				/*								*/
	if( 1 == KSG_RauSndCtrl.SndCmpFlg ){										/*								*/
		return (uchar)1;														/*								*/
	}																			/*								*/
	return (uchar)0;															/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      KSG_Rau_Int_TXI																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI possible to send interrupt routine															   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| input  : Groval area use => IFC_SndBuf, IFC_SndCtrl														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	KSG_Rau_Int_TXI( void )													/*								*/
{																				/*								*/
	if( KSG_RauModemOnline != 1 ) {
		/* ﾎｽﾄ回線未接続状態(ATコマンド発行) */
		if( KSG_mdm_cmd_len != 0 ) {											/* ⇒ATｺﾏﾝﾄﾞあり?				*/
			if( ( KSG_mdm_cmd_ptr < KSG_mdm_cmd_len )&&							/*   ⇒送信ﾃﾞｰﾀあり?			*/
			    ( ( KSG_line_stat_h & KSG_CS_SIG ) != 0 ) ) {					/*   ⇒CS ON?					*/
				SCI7.TDR = KSG_mdm_cmd_buf[KSG_mdm_cmd_ptr];					/*     ⇒1ﾊﾞｲﾄ送信				*/
				KSG_mdm_cmd_ptr++;												/*       ｶｳﾝﾀ加算				*/
				if( SCI7.SSR.BIT.TEND == 1 ) {									/*       送信ﾃﾞｰﾀ空?			*/
					SCI7.SSR.BIT.TEND = 0;										/*       ⇒TDREｸﾘｱ				*/
				}																/*								*/
			}																	/*								*/
			else {																/*   ⇒その他					*/
				SCI7.SCR.BIT.TIE = 0;											/*     ⇒TXI割込み禁止			*/
				if( KSG_mdm_cmd_ptr < KSG_mdm_cmd_len ) {						/*       ⇒送信ﾃﾞｰﾀあり?		*/
					KSG_wait_CS = 1;											/*         ⇒CS ON待ち			*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		else {																	/* ⇒その他						*/
			SCI7.SCR.BIT.TIE = 0;												/*   ⇒TXI割込み禁止			*/
		}																		/*								*/
	}																			/*								*/
	else {
		/* ﾎｽﾄ回線接続中(PPP接続) */
		if( KSG_RauSndCtrl.SndReqCnt != 0 ){									/* send enable (Y)				*/
			++KSG_RauSndCtrl.SndCmpCnt;											/* send completed count up		*/
			if( KSG_RauSndCtrl.SndReqCnt > KSG_RauSndCtrl.SndCmpCnt ){			/* not end yet (Y)				*/
				SCI7.TDR = KSG_RauSndBuf[KSG_RauSndCtrl.ReadIndex];				/* send charcter set			*/
				KSG_RauSndCtrl.ReadIndex++;
				if(SCI7.SSR.BIT.TEND == 1) {
					SCI7.SSR.BIT.TEND= 0;										/*								*/
				}
			}																	/*								*/
			else {																/* all data send end			*/
				SCI7.SCR.BIT.TIE = 0;											/* send enable interrupt inhibit*/
				KSG_RauSndCtrl.SndCmpFlg = 1;									/* send complete flag set 		*/
			}																	/*								*/
		}
		else {
			SCI7.SCR.BIT.TIE = 0;												/*   ⇒TXI割込み禁止			*/
		}
	}
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      KSG_Rau_Int_TEI																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI send end interrupt routine																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	KSG_Rau_Int_TEI( void )													/*								*/
{																				/*								*/
	/* I don't use this event */												/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|  [INT]      KSG_RauSciStop																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|         SCI send & receive stop																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      : 																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・この関数は未使用																						   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	KSG_RauSciStop( void )													/*								*/
{																				/*								*/
	unsigned long	ist;														/* 現在の割込受付状態			*/
																				/*								*/
	ist = _di2();																/*								*/
																				/*								*/
	SCI7.SCR.BIT.TIE  = 0;														/* 送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ(TXI)割込み禁止*/
	SCI7.SCR.BIT.RIE  = 0;														/* 受信割込み禁止 				*/
	SCI7.SCR.BIT.TE   = 0;														/* 送信動作禁止 				*/
	SCI7.SCR.BIT.RE   = 0;														/* 受信動作禁止 				*/
	SCI7.SCR.BIT.TEIE = 0;														/* 送信終了(TEI)割り込み禁止 	*/
	dummy_Read = SCI7.SSR.BYTE;													/* Dummy Read					*/
																				/*								*/
	_ei2( ist );																/*								*/
																				/*								*/
}																				/*								*/


/*[]------------------------------------------------------------------------------------------[]*/
/*|	SCI0通信制御信号処理																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauSciLineCheck();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauSciLineCheck( void )												/*								*/
{
	unsigned char	wDSR,wCTS,wCD,wCI;

	// ToDo:H/Wに合わせる必要あり
	wDSR = SCI7_DSR;
	wCTS = SCI7_CTS;
	wCD  = SCI7_CD;
	wCI  = SCI7_CI;

	wait2us(5);						// 10us wait //
	wDSR = (wDSR << 1) | SCI7_DSR;
	wCTS = (wCTS << 1) | SCI7_CTS;
	wCD  = (wCD << 1) | SCI7_CD;
	wCI  = (wCI << 1) | SCI7_CI;

	wait2us(5);						// 10us wait //
	wDSR = (wDSR << 1) | SCI7_DSR;
	wCTS = (wCTS << 1) | SCI7_CTS;
	wCD  = (wCD << 1) | SCI7_CD;
	wCI  = (wCI << 1) | SCI7_CI;

	//----- DSR(DR) -----//
	if( wDSR == 0x07 ){
		KSG_Chatt_DSR = (KSG_Chatt_DSR << 1) | 1;
		if( (KSG_Chatt_DSR & KSG_RSSIG_CHT_BITCNT) == KSG_RSSIG_CHT_BITCNT ){
			KSG_DSR = 1;
		}
	}
	else if( wDSR == 0){
		KSG_Chatt_DSR <<= 1;
		if( (KSG_Chatt_DSR & KSG_RSSIG_CHT_BITCNT) == 0 ){
			KSG_DSR = 0;
		}
	}
	//----- CTS(CS) -----//
	if( wCTS == 0x07 ){
		KSG_Chatt_CTS = (KSG_Chatt_CTS << 1) | 1;
		if( (KSG_Chatt_CTS & KSG_RSSIG_CHT_BITCNT) == KSG_RSSIG_CHT_BITCNT ){
			KSG_CTS = 1;
		}
	}
	else if( wCTS == 0){
		KSG_Chatt_CTS <<= 1;
		if( (KSG_Chatt_CTS & KSG_RSSIG_CHT_BITCNT) == 0 ){
			KSG_CTS = 0;
		}
	}
	//----- DCD(CD) -----//
	if( wCD == 0x07 ){
		KSG_Chatt_CD = (KSG_Chatt_CD << 1) | 1;
		if( (KSG_Chatt_CD & KSG_RSSIG_CHT_BITCNT) == KSG_RSSIG_CHT_BITCNT ){
			KSG_CD = 1;
		}
	}
	else if( wCD == 0){
		KSG_Chatt_CD <<= 1;
		if( (KSG_Chatt_CD & KSG_RSSIG_CHT_BITCNT) == 0 ){
			KSG_CD = 0;
		}
	}
	//----- RI(CI) -----//
	if( wCI == 0x07 ){
		KSG_Chatt_CI = (KSG_Chatt_CI << 1) | 1;
		if( (KSG_Chatt_CI & KSG_RSSIG_CHT_BITCNT) == KSG_RSSIG_CHT_BITCNT ){
			KSG_CI = 1;
		}
	}
	else if( wCI == 0){
		KSG_Chatt_CI <<= 1;
		if( (KSG_Chatt_CI & KSG_RSSIG_CHT_BITCNT) == 0 ){
			KSG_CI = 0;
		}
	}
	/* --- CTS ---------------------------------------------------------------------------------*/
	if( KSG_CTS ) {																/* CTS OFF? 					*/
		if( KSG_CS_ON() ) {														/*								*/
			KSG_line_stat_h &= ~KSG_CS_SIG;										/*								*/
		}																		/*								*/
	} else {																	/* CTS ON?						*/
		if( KSG_CS_ON() == 0 ) {												/* ⇒ONに変化? 					*/
			KSG_line_stat_h |= KSG_CS_SIG;										/*								*/
			if( !SCI7.SCR.BIT.TIE && KSG_wait_CS ) {							/* ⇒送信停止中 & CS_ON待ち		*/
				SCI7.SCR.BIT.TIE = 1;											/* ⇒TXI割込み許可				*/
				KSG_wait_CS = 0;												/*								*/
			}																	/*								*/
		}																		/*								*/
		if(( !SCI7.SCR.BIT.TIE ) && ( SCI7.SSR.BIT.TEND )) {					/* ⇒送信停止中 & 送信完了		*/
			if( KSG_mdm_cmd_len != 0 ) {										/* ⇒送信待ちあり?				*/
				if( KSG_mdm_cmd_ptr >= KSG_mdm_cmd_len ) KSG_mdm_cmd_len = 0;	/* ⇒送信ﾃﾞｰﾀ長ｸﾘｱ				*/
			}																	/*								*/
		}																		/*								*/
	}																			/*								*/
	/* --- CD ----------------------------------------------------------------------------------*/
	if( KSG_CD ) {																/* CD OFF? 						*/
		if( KSG_CD_ON() ) {														/* ⇒ON→OFF? 					*/
			KSG_line_stat_h &= ~KSG_CD_SIG;										/*								*/
			KSG_RauModemOnline = 0;												/* ﾎｽﾄ回線状態ﾌﾗｸﾞ 切断			*/
			KSG_dials.BIT.out = 0;	KSG_dials.BIT.in = 0;						/* ﾀﾞｲｱﾙｽﾃｰﾀｽ ﾘｾｯﾄ				*/
		}																		/*								*/
	}																			/*								*/
	else {																		/* CD ON?						*/
		if( KSG_CD_ON() == 0 )	{												/* ⇒OFF→ON?	 				*/
			KSG_line_stat_h |= KSG_CD_SIG;										/*   							*/
		}																		/*								*/
	}																			/*								*/
	/* --- DSR ---------------------------------------------------------------------------------*/
	if( KSG_DSR ) {																/* DSR OFF? 					*/
		if( KSG_line_stat_h & KSG_DR_SIG ) {									/* ⇒ON→OFF?					*/
			KSG_line_stat_h &= ~KSG_DR_SIG;										/*   							*/
		}																		/*								*/
	}																			/*								*/
	else {																		/* DSR ON?						*/
		if( ( KSG_line_stat_h & KSG_DR_SIG ) == 0 ) {							/* ⇒OFF→ON?					*/
			KSG_line_stat_h |= KSG_DR_SIG;										/*								*/
		}																		/*								*/
	}																			/*								*/
	/* --- CI(RI)--------------------------------------------------------------------------------*/
	if ( KSG_CI ){																/* RI OFF? 						*/
		if ( KSG_line_stat_h & KSG_CI_SIG ){									/* ⇒ON→OFF?					*/
			KSG_line_stat_h &= ~KSG_CI_SIG;										/*   							*/
		}																		/*								*/
	}else{																		/*								*/
		if( ( KSG_line_stat_h & KSG_CI_SIG ) == 0 ){							/* RI ON?						*/
			KSG_line_stat_h |= KSG_CI_SIG;										/* ⇒OFF→ON?					*/
		}																		/*								*/
	}																			/*								*/
}

unsigned short	KSG_CS_ON( void )												/*								*/
{																				/*								*/
	return	(unsigned short)(KSG_line_stat_h & KSG_CS_SIG);						/*								*/
}																				/*								*/

unsigned short	KSG_CD_ON( void )												/*								*/
{																				/*								*/
	return	(unsigned short)(KSG_line_stat_h & KSG_CD_SIG);						/*								*/
}																				/*								*/
