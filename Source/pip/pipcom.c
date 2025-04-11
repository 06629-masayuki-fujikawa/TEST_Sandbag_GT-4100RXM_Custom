/*[]----------------------------------------------------------------------[]*/
/*| IFmodule対向 処理	(GW828700追加)                                     |*/
/*|																		   |*/
/*| Host 通信制御プログラム（Host.c の代行）				               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : M.Okuda                                                  |*/
/*| Date        : 2003.01.06                                               |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
#define		GLOBAL extern
#include	<string.h>

#include	"pip_def.h"
#include	"system.h"
#include	"ope_def.h"
#include	"mem_def.h"
#include	"ifm_ctrl.h"
#include	"prm_tbl.h"

/*==========================================================================*/

/** timer number **/
#define	TIM1_NUM		(unsigned char)1
#define	TIM2_NUM		(unsigned char)2
#define	TIM3_NUM		(unsigned char)3
#define	TIM4_NUM		(unsigned char)4
#define	TIM_ACC_PAY_NUM	(unsigned char)5	// 精算データ蓄積処理用
#define	TIM_ACC_ERR_NUM	(unsigned char)6	// エラーデータ蓄積処理用

/** timer value **/
#define	TIM1_VAL		(unsigned long)(30+1)	/* 300ms */
#define	TIM2_VAL		(unsigned long)(30+1)	/* 300ms */
#define	TIM3_VAL		(unsigned long)( 1+1)	/*  10ms */

#define	RCT1_COUNT	2						/* NAK retry */
#define	RCT2_COUNT	15						/* Pol retry */
#define	RCT3_COUNT	2						/* Sel retry */

#define	CHAR_ACK0	'0'
#define	CHAR_ACK1	'1'
#define	CHAR_DLE	0x10

#define	SOH	(unsigned char)( 0x01 )	// ヘッダー開始(Start Of Heading)
#define	STX	(unsigned char)( 0x02 )	// テキスト開始(Start Of Text)
#define	ETX	(unsigned char)( 0x03 )	// テキスト終結(End Of Text)
#define	EOT	(unsigned char)( 0x04 )	// 伝送終了(End Of Transmission)
#define	ENQ	(unsigned char)( 0x05 )	// 問い合わせ(Enquiry)
#define	ACK	(unsigned char)( 0x06 )	// 肯定応答(Acknowledge)
#define	NAK	(unsigned char)( 0x15 )	// 否定応答(Negative Acknowledge)
#define	SYN	(unsigned char)( 0x16 )	// テーブルデータ区切りコード
#define	ETB	(unsigned char)( 0x17 )	// 伝送ブロック終了


#define	S0_IDLE			0	// IDEL
#define	S1_Pol_Response	1	// Pol応答待ち
#define	S2_Receiv_TEXT	2	// EXT受信待ち
#define	S3_Receiv_BCC	3	// BCC受信待ち
#define	S4_Receiv_EOT	4	// ACK送信後の EOT受信待ち
#define	S5_Sel_Response	5	// Sel 送信後応答待ち
#define	S6_Send_TEXT	6	// Sel 電文送信後 応答待ち

/*==========================================================================*/
/*		エリア定義															*/
/*==========================================================================*/
// ==> PIPRAM (PIP_Ram.h)

/*==========================================================================*/
/*		プログラム															*/
/*==========================================================================*/

#pragma 	section

/* external */
extern	unsigned long	term_send_que_push( unsigned long );
extern	void	sci_tx_start_h( void );
extern	void	NH_ScuRestart( void );

/* plottype */
void			NH_Init(void);
void			NH_Main(void);
void			NH_S0(void);
void			NH_S1(void);
void			NH_S2(void);
void			NH_S3(void);
void			NH_S4(void);
void			NH_S5(void);
void			NH_S6(void);
void			NH_TimerSet(unsigned char f_Start);
void			NH_Send(unsigned char *pData, unsigned short Length);
unsigned short	NH_SData_Read(unsigned char *pSData);
unsigned short	NH_Is_SData(void);
void			NH_SData_Del(void);
void			NH_PolSel_Send(unsigned char f_Sel);
void			NH_CtrlCmd_Send(unsigned char CmdKind);
void			NH_RDataSet_toTemp(unsigned char Rchar);
void			NH_TimoutProc(unsigned char Phase);
void			NH_PolErrProc(unsigned char f_Occurred);
void			NH_SelErrProc(unsigned char f_Occurred, unsigned char *pSData);
unsigned char	NH_IsTimeout(unsigned char TimNum);
void			NH_SendReqCancel(void);

unsigned short PIP_SciSendQue_insert(unsigned char *pData, unsigned short length);

unsigned char	NH_RemoteErrProc(void);	// データ通信NGチェック

/*[]----------------------------------------------------------------------[]*/
/*| NH_Init		                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PowerON時毎にCallされるイニシャル処理  		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-06                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_Init( void )
{
	PIPRAM.NH_Rct1 = PIPRAM.NH_Rct2 = PIPRAM.NH_Rct3 = 0;	/* COM error counter */
	PIPRAM.NH_f_ComErr = 0;					/* 1= COM error */
	PIPRAM.NH_PrevRecvChar = 0;				/* prevous received character */

	NH_TimerSet( 1 );						/* timer start */
	PIPRAM.pip_uc_mode_h = S0_IDLE;			/* same as "S_H_IDLE" */
	PIPRAM.i_time_setting = ( unsigned char )prm_get(COM_PRM, S_CEN, 27, 2, 1);	// ポーリング間隔の設定
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_Main			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| IFmodule通信処理メイン（状態管理部）   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-06                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_Main( void )
{
	switch( PIPRAM.pip_uc_mode_h ){
	case	S0_IDLE:
	default:
		NH_S0();		//IDLE
		break;

	case	S1_Pol_Response:
		NH_S1();		//Pol応答待ち
		break;

	case	S2_Receiv_TEXT:	
		NH_S2();		//EXT受信待ち
		break;

	case	S3_Receiv_BCC:
		NH_S3();		//BCC受信待ち
		break;

	case	S4_Receiv_EOT:
		NH_S4();		//ACK送信後の EOT受信待ち
		break;

	case	S5_Sel_Response:
		NH_S5();		//Sel 送信後応答待ち
		break;

	case	S6_Send_TEXT:
		NH_S6();		//電文送信後 応答待ち
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S0			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| 状態０（アイドル）処理				   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S0( void )
{
	unsigned	char	f_Sel;						/* 0=Pol send, 1=Sel send */

	if( 0 == NH_IsTimeout( TIM1_NUM ) ){			/* T1 timeout (N) */
		if( 0 != NH_Is_SData() ){					/* send data exist (Y) */
			if( 1 == NH_IsTimeout( TIM4_NUM ) ){	/* T4 timeout (N) */
				goto NH_S0_10;
			}
		}
		return;
	}
NH_S0_10:

	_di();
	PIPRAM.ui_write_h = PIPRAM.ui_count_h = PIPRAM.ui_read_h = 0;	/* receive buffer clear */
	_ei();

	PIPRAM.NH_RbufCt = 0;							/* receive character for temporary buffer clear */
	PIPRAM.NH_PrevRecvChar = 0;						/* previous received character buffer clear */
	PIPRAM.NH_f_NxAck = CHAR_ACK0;					/* next send/receive ACK is ACK0 */

	f_Sel = 0;
	if( 1 != PIPRAM.NH_f_ComErr ){					/* comunication error is happening (N) */
		PIPRAM.NH_SbufLen = NH_SData_Read( &PIPRAM.NH_Sbuf[ 5 ] );	// シリアル送信キューから１パケット取り出し
		if( 0 != PIPRAM.NH_SbufLen ){				/* send data exist (Y) */
			f_Sel = 1;								/* Sel send req info set */
		}
	}

	NH_PolSel_Send( f_Sel );			/* Pol or Sel send */
	NH_TimerSet( 1 );					/* timer start */

	if( 1 == f_Sel ){					/* Sel sended (Y) */
		PIPRAM.pip_uc_mode_h = S5_Sel_Response;
	}
	else{								/* Pol sended */
		PIPRAM.pip_uc_mode_h = S1_Pol_Response;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S1			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| 状態１（Pol送信後 応答待ち）処理	   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S1( void )
{
	if( 1 == Mh_get_receive() ) {		/* character received (Y) */
		switch( PIPRAM.uc_rcv_byte_h ){		/* branch by received character */
		case	EOT:					/* frame none */
			PIPRAM.NH_f_ComErr = 0;			/* error repaier */
			PIPRAM.NH_Rct2 = 0;
			NH_PolErrProc( 0 );

			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// データ通信NGチェック
			return;

		case	SOH:					/* frame start */
			PIPRAM.NH_RbufCt = 0;				/* receive character for temporary buffer clear */
			NH_RDataSet_toTemp( PIPRAM.uc_rcv_byte_h );	/* received character save */
			NH_TimerSet( 1 );			/* T3 start */
			PIPRAM.pip_uc_mode_h = S2_Receiv_TEXT;
			return;
		}
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM2_NUM ) ){		/* T2 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S2			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| 状態２（電文受信中 ETX待ち）処理	   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S2( void )
{
	if( 1 == Mh_get_receive() ) {			/* character received (Y) */
		switch( PIPRAM.uc_rcv_byte_h ){		/* branch by received character */
		case	EOT:						/* frame none */
			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// データ通信NGチェック
			return;

		default:
			NH_RDataSet_toTemp( PIPRAM.uc_rcv_byte_h );	/* received character save */
			NH_TimerSet( 1 );			/* T3 start */

			if( ETX == PIPRAM.uc_rcv_byte_h ){	/* ETX received (Y) */
				PIPRAM.pip_uc_mode_h = S3_Receiv_BCC;
			}
			return;
		}
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM3_NUM ) ){		/* T3 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S3			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| 状態３（電文受信中 BCC待ち）処理	   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S3( void )
{
	unsigned char	Bcc;
	unsigned char	f_Ok;
	short			Length;
	unsigned char			ReqID;		// 要求データＩＤ
	unsigned char			ReqCnt;		// 要求件数
	#define	PIP_ID51_PKT_LEN	32		// 時計更新要求電文 PIPフレーム長

	if( 1 == Mh_get_receive() ) {		/* character received (Y) */

		NH_RDataSet_toTemp( PIPRAM.uc_rcv_byte_h );	/* received character save */
		f_Ok = 0;
		if( ( 13 < PIPRAM.NH_RbufCt ) &&		/* header & length OK (Y) */
			( 0 == (PIPRAM.NH_RbufCt & 7) )&&	/* multiple of 8 (also SOH+Header+STX+ETX+BCC are 8 bytes) */
			( '0' == PIPRAM.NH_Rbuf[1] ) &&
			( '0' == PIPRAM.NH_Rbuf[2] ) &&
			( '0' == PIPRAM.NH_Rbuf[3] ) &&
			( '1' == PIPRAM.NH_Rbuf[4] ) &&
			( STX == PIPRAM.NH_Rbuf[5] ) ){

			Bcc = Mt_BccCal( &PIPRAM.NH_Rbuf[6], (PIPRAM.NH_RbufCt - 6 - 1) );
			if( Bcc == PIPRAM.uc_rcv_byte_h ){		/* BCC OK (Y) */
				f_Ok = 1;
			}
		}
		if( 1 == NH_RemoteErrProc() ){
			f_Ok = 0;				// データ通信NGが発生している場合はNAKを返す
		}

		/** NG frame received process **/
		if( 0 == f_Ok ){							/* received data NG (Y) */
			NH_CtrlCmd_Send( NAK );					/* NAK send */
			NH_TimerSet( 1 );						/* T2 start */
			PIPRAM.pip_uc_mode_h = S1_Pol_Response;	/* frame receive wait again */
			return;
		}

		NH_CtrlCmd_Send( ACK );		/* ACK send (0/1) */
		PIPRAM.NH_f_ComErr = 0;		/* COM error repaier */
		PIPRAM.NH_Rct2 = 0;
		NH_PolErrProc( 0 );

		Length = PIPRAM.NH_RbufCt;	//Length = SOH～BCCのサイズ
		Length -= 8;				//SOH,HD1,HD2(1.2.3),STX, , ETX,BCC

		if( Length >= 6 ){
			switch( PIP_BcdAscToBin_2(&PIPRAM.NH_Rbuf[10]) ){	// データ種別
			case 31:	// データ要求２(フラップ・ロック式)
				ReqID = PIP_BcdAscToBin_2( &PIPRAM.NH_Rbuf[24] );		// 要求データＩＤ
				ReqCnt = PIP_BcdAscToBin_2( &PIPRAM.NH_Rbuf[26] );		// 要求件数
				if( (ReqCnt == 0)||(ReqCnt > 20) ){						// １～２０の範囲内でなければＮＧ
					ReqID = 0;
				}
				switch( ReqID ){
				case 67:	// 精算データ(フラップ・ロック式)
					IFM_Snd_AccumulatePayment( ReqCnt );
					break;
				case 68:	// エラーデータ(フラップ・ロック式)
					IFM_Snd_AccumulateError( ReqCnt );
					break;
				default:	// 結果:パラメータ不良("０２")
					memcpy(&PIPRAM.Err_sendbuf[0], &PIPRAM.NH_Rbuf[6], 40);
					PIPRAM.Err_sendbuf[4] = '6';
					PIPRAM.Err_sendbuf[5] = '1';		// ６１:データ要求２ＮＧ応答
					PIPRAM.Err_sendbuf[16] = '0';
					PIPRAM.Err_sendbuf[17] = '2';		// ０２:パラメータ不良
					PIP_SciSendQue_insert(&PIPRAM.Err_sendbuf[0], 40);	// シリアル送信キューへのデータ追加
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				}
				break;
			default:	// スルーデータ
				PIP_ChangeData(Length, &PIPRAM.NH_Rbuf[6]);	// シリアル受信データをIFMのバッファへ変換
				break;
			}
		}

		NH_TimerSet( 1 );			/* T2 start */
		PIPRAM.pip_uc_mode_h = S4_Receiv_EOT;
		return;
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM3_NUM ) ){		/* T3 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S4			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| 状態４（電文受信 ACK送信後 EOT待ち）処理	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S4( void )
{
	if( 1 == Mh_get_receive() ) {			/* character received (Y) */
		switch( PIPRAM.uc_rcv_byte_h ){		/* branch by received character */
		case	EOT:						/* frame none */
			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// データ通信NGチェック
			return;

		case	STX:
			PIPRAM.NH_RbufCt = 5;				/* data set point set */
			NH_RDataSet_toTemp( PIPRAM.uc_rcv_byte_h );	/* received character save */
			NH_TimerSet( 1 );			/* T3 start */
			PIPRAM.pip_uc_mode_h = S2_Receiv_TEXT;
			NH_RemoteErrProc();			// データ通信NGチェック
			return;
		}
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM2_NUM ) ){		/* T2 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S5			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| 状態５（Sel送信後 応答待ち）処理			                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S5( void )
{
	if( 1 == Mh_get_receive() ) {		/* character received (Y) */
		if( EOT == PIPRAM.uc_rcv_byte_h ){	/* EOT receive (Y) */
			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// データ通信NGチェック
			return;
		}

		if( NAK == PIPRAM.uc_rcv_byte_h ){		/* NAK receive (Y) */
			NH_TimoutProc( PIPRAM.pip_uc_mode_h );
			return;
		}

		if( (CHAR_DLE == PIPRAM.NH_PrevRecvChar) && (PIPRAM.NH_f_NxAck == PIPRAM.uc_rcv_byte_h) ){	/* OK ACK receive (Y) */
			PIPRAM.NH_Rct1 = 0;

			/* 送信する電文は "PIPRAM.NH_Sbuf[]" に入っている。但しヘッダー無し */
			PIPRAM.NH_Sbuf[ 0 ] = SOH;
			PIPRAM.NH_Sbuf[ 1 ] = '0';
			PIPRAM.NH_Sbuf[ 2 ] = '0';
			PIPRAM.NH_Sbuf[ 3 ] = '0';
			PIPRAM.NH_Sbuf[ 4 ] = '1';
			NH_Send( &PIPRAM.NH_Sbuf[ 0 ], (PIPRAM.NH_SbufLen + 5) );	/* send */

			PIPRAM.NH_f_NxAck ^= 1;
			PIPRAM.NH_PrevRecvChar = 0;

			NH_TimerSet( 1 );				/* T2 start */
			PIPRAM.pip_uc_mode_h = S6_Send_TEXT;
			NH_RemoteErrProc();			// データ通信NGチェック
			return;
		}

		PIPRAM.NH_PrevRecvChar = PIPRAM.uc_rcv_byte_h;	/* previous character update */
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM2_NUM ) ){		/* T2 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S6			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| 状態６（電文送信後 ACK待ち）処理			                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S6( void )
{
	if( 1 == Mh_get_receive() ) {		/* character received (Y) */
		if( EOT == PIPRAM.uc_rcv_byte_h ){		/* EOT receive (Y) */
			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// データ通信NGチェック
			return;
		}

		if( NAK == PIPRAM.uc_rcv_byte_h ){		/* NAK receive (Y) */
			if( RCT1_COUNT <= PIPRAM.NH_Rct1 ){			/* retry over (Y) */
				PIPRAM.NH_f_ComErr = 1;

				/* 送信しようとしたデータは "PIPRAM.NH_Sbuf[]" に入っている */
				NH_SelErrProc( 1, &PIPRAM.NH_Sbuf[0] );	/* error inform */
				NH_SendReqCancel();						/* send request cancel (for error) */

				PIPRAM.NH_Rct1 = 0;

				NH_CtrlCmd_Send( EOT );
				NH_TimerSet( 1 );					/* T1 start */
				PIPRAM.pip_uc_mode_h = S0_IDLE;		/* phase to IDLE */
			}
			else{
				++PIPRAM.NH_Rct1;
				NH_Send( &PIPRAM.NH_Sbuf[ 0 ], (PIPRAM.NH_SbufLen + 5) );	/* frame re-send */
				NH_TimerSet( 1 );					/* T2 start */
			}
			return;
		}

		if( (CHAR_DLE == PIPRAM.NH_PrevRecvChar) && (PIPRAM.NH_f_NxAck == PIPRAM.uc_rcv_byte_h) ){	/* OK ACK receive (Y) */

			/* 送信したデータは "PIPRAM.NH_Sbuf[]" に入っている */
			NH_SData_Del();								// シリアル送信キュー内の送信データポイント移動
			PIPRAM.NH_f_ComErr = 0;
			PIPRAM.NH_Rct1 = 0;							/* NAK received retry counter clear */
			PIPRAM.NH_Rct3 = 0;							/* Sel retry counter clear */
			NH_CtrlCmd_Send( EOT );
			NH_TimerSet( 1 );						/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;			/* phase to IDLE */
			NH_RemoteErrProc();			// データ通信NGチェック
			return;
		}

		PIPRAM.NH_PrevRecvChar = PIPRAM.uc_rcv_byte_h;		/* previous character update */
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM2_NUM ) ){		/* T2 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}



/*==========================================================================*/
/*		UTYLITY subroutines													*/
/*==========================================================================*/

/*[]----------------------------------------------------------------------[]*/
/*| NH_TimerSet		                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| timer start/stop control			  		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : f_Start : 1=Start, 0=Stop							   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-06                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_TimerSet( unsigned char f_Start )
{
	if( 1 == f_Start ){
		PIPRAM.NH_StartTime = LifeTimGet();
	}
	else{
		PIPRAM.NH_StartTime = 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_Send			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| data send to serial port.			  		                           |*/
/*| This return after send complete 									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : pData = pointer for send string						   |*/
/*|				   Length = send data length							   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-06                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_Send( unsigned char *pData, unsigned short Length )
{
	memcpy( &PIPRAM.huc_txdata_i_h[0], pData, Length );
	PIPRAM.ui_txdlength_i_h = Length;
	PIPRAM.ui_txpointer_h = 0;					/* send pointer clear */

	NH_TimerSet( 1 );						/* T1 start */
	do{
		taskchg( IDLETSKNO );
	}while( 0 == NH_IsTimeout( TIM3_NUM ) );

	sci_tx_start_h();					/* TxI enable (send start) */

	/* wait until send end */
	do{
		taskchg( IDLETSKNO );
	}while( 0 == PIPRAM.uc_send_end_h );
}

//==============================================================================
//	シリアル送信キューからのパケット取り出し
//
//	@argument	*pSData		ＰａｒｋｉＰＲＯに送信するパケット
//
//	@return		ＰａｒｋｉＰＲＯに送信するパケットの長さ(ＳＴＸ～ＢＣＣ)
//
//	@image	送信順
//			  │送信位置                  [シリアル送信キュー]
//			  │   │  ┌──┬──┬──┬──┬──┬─────┬──┬──┐┬
//			パ│   └→│長さ│STX │ID1 │ID2 │ID3 │…データ…│ETX │BCC ││
//			ケ│   │  ├──┼──┼──┼──┼──┼─────┼──┼──┤│Ｍ
//			ッ│   └→│長さ│STX │ID1 │ID2 │ID3 │…データ…│ETX │BCC ││Ａ
//			ト│   │  ├──┼──┼──┼──┼──┼─────┼──┼──┤│Ｘ
//			数│   └→│ ： │ ： │ ： │ ： │ ： │    ：    │ ： │ ： ││８
//			  │   │  ├──┼──┼──┼──┼──┼─────┼──┼──┤│０
//			  ↓   └→│長さ│STX │ID1 │ID2 │ID3 │…データ…│ETX │BCC ││件
//		STOP ─────┼──┼──┼──┼──┼──┼─────┼──┼──┤│
//			           │    │    │    │    │    │          │    │    ││
//			           └──┴──┴──┴──┴──┴─────┴──┴──┘┴
//			           ├────────  ２５９ｂｙｔｅ  ─────────┤
//	@end
//
//	@attention	COUNT 0, CIND 0 は送信キューが空の状態を意味する。
//
//	@note		シリアル送信キューに格納されている(ＳＴＸからＢＣＣまでの)
//				未送信パケットをキューの上方より探し出しパケットとパケット長を返す。
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
unsigned short NH_SData_Read(unsigned char *pSData)
{
	if( PIPRAM.SCI_S_QUE.COUNT == 0 )	return( 0 );			// シリアル送信キューにデータなし
	if( PIPRAM.SCI_S_QUE.CIND >= PIPRAM.SCI_S_QUE.COUNT ){		// 送信パケット位置がパケット数と同一になると全送信終了
		PIPRAM.SCI_S_QUE.COUNT = 0;								// パケット数０
		PIPRAM.SCI_S_QUE.CIND = 0;								// [メモ]送信パケット位置がインクリメントされるのは「正常送信時」
		return( 0 );											// シリアル送信キューにデータなし
	}

	memcpy( pSData, &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.CIND].BUFF[0], PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.CIND].LEN );
	return( PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.CIND].LEN );
}

//==============================================================================
//	シリアル送信キュー内の未送信パケット数取得
//
//	@argument	なし
//
//	@return		未送信パケット数
//
//	@attention	COUNT 0, CIND 0 は送信キューが空の状態を意味する。
//
//	@note		シリアル送信キューに格納されている(ＳＴＸからＢＣＣまでの)
//				未送信パケット数を返す。
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
unsigned short NH_Is_SData(void)
{
	if( PIPRAM.SCI_S_QUE.CIND > PIPRAM.SCI_S_QUE.COUNT ){	// 本来あり得ない
		PIPRAM.SCI_S_QUE.COUNT = 0;							// パケット数０
		PIPRAM.SCI_S_QUE.CIND = 0;							// [メモ]送信パケット位置がインクリメントされるのは「正常送信時」
	}
	return(unsigned short)( PIPRAM.SCI_S_QUE.COUNT - PIPRAM.SCI_S_QUE.CIND );
}

//==============================================================================
//	シリアル送信データポイント移動
//
//	@argument	なし
//
//	@return		なし
//
//	@attention	COUNT 0, CIND 0 は送信キューが空の状態を意味する。
//
//	@note		シリアル送信キューに格納されているパケット１件の送信が正常に
//				終了したときにコールされ、キュー内の次のデータをポイントする。
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void NH_SData_Del(void)
{
	if( PIPRAM.SCI_S_QUE.COUNT == 0 ){	// シリアル送信キューにはもともとデータが入ってない
		PIPRAM.SCI_S_QUE.CIND = 0;
		return;
	}

	if( PIPRAM.SCI_S_QUE.CIND > PIPRAM.SCI_S_QUE.COUNT ){		// 本来あり得ない
		PIPRAM.SCI_S_QUE.COUNT = 0;
		PIPRAM.SCI_S_QUE.CIND = 0;
		return;
	}

	if( PIPRAM.SCI_S_QUE.CIND == PIPRAM.SCI_S_QUE.COUNT ){				// 送信パケット位置がパケット数と同一になると全送信終了
	} else {
		PIPRAM.SCI_S_QUE.CIND++;				// 次のパケットをポイント
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_PolSel_Send		                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Pol or Sel send						  		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : f_Sel  0=Pol, 1=Sel									   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_PolSel_Send( unsigned char f_Sel )
{
	PIPRAM.wkuc[ 0 ] = EOT;
	PIPRAM.wkuc[ 1 ] = '0';
	PIPRAM.wkuc[ 2 ] = '0';
	PIPRAM.wkuc[ 3 ] = '0';
	PIPRAM.wkuc[ 4 ] = ( 1 == f_Sel ) ? '1' : '0';
	PIPRAM.wkuc[ 5 ] = ENQ;

	NH_Send( &PIPRAM.wkuc[ 0 ], (unsigned short)6 );
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_CtrlCmd_Send			                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| control character send 				  		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : CmdKind  send control character (see source)			   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_CtrlCmd_Send( unsigned char CmdKind )
{
	switch( CmdKind ){
	case	ACK:
		PIPRAM.wkuc[ 0 ] = CHAR_DLE;
		PIPRAM.wkuc[ 1 ] = PIPRAM.NH_f_NxAck;
		NH_Send( &PIPRAM.wkuc[ 0 ], (unsigned short)2 );
		PIPRAM.NH_f_NxAck ^= 1;
		break;

	case	NAK:
		PIPRAM.wkuc[ 0 ] = NAK;
		NH_Send( &PIPRAM.wkuc[ 0 ], (unsigned short)1 );
		break;

	case	EOT:
		PIPRAM.wkuc[ 0 ] = EOT;
		NH_Send( &PIPRAM.wkuc[ 0 ], (unsigned short)1 );
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_RDataSet_toTemp	                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| received character set to temporary buffer	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : Rchar  received character							   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_RDataSet_toTemp( unsigned char Rchar )
{
	if( NH_RBUF_MAX > PIPRAM.NH_RbufCt ){
		PIPRAM.NH_Rbuf[ PIPRAM.NH_RbufCt ] = Rchar;
		++PIPRAM.NH_RbufCt;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_TimoutProc		                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| timeout happened process					                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : Phase  Process phase when timeout occuerd 			   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_TimoutProc( unsigned char Phase )
{
	/** Pol phase process **/
	if( ( 1 <= Phase) && (Phase <= 3) ){			/* Pol phase (Y) */
		if( RCT2_COUNT <= PIPRAM.NH_Rct2 ){				/* retry over (Y) */
			NH_ScuRestart();						/* SCU restart */

			PIPRAM.NH_f_ComErr = 1;
			NH_PolErrProc( 1 );
			PIPRAM.NH_Rct2 = 0;
		}
		else{
			++PIPRAM.NH_Rct2;
		}
	}

	/** Sel phase process **/
	else if( ( 5 <= Phase) && (Phase <= 6) ){		/* Sel phase (Y) */
		if( RCT3_COUNT <= PIPRAM.NH_Rct3 ){				/* retry over (Y) */
			PIPRAM.NH_f_ComErr = 1;

			/* 送信しようとしたデータは "PIPRAM.NH_Sbuf[]" に入っている */
			NH_SelErrProc( 1, &PIPRAM.NH_Sbuf[0] );	/* error inform */
			NH_SendReqCancel();						/* send request cancel (for error) */

			PIPRAM.NH_Rct3 = 0;
		}
		else{
			++PIPRAM.NH_Rct3;
		}
	}

	NH_CtrlCmd_Send( EOT );
	NH_TimerSet( 1 );								/* T1 start */
	PIPRAM.pip_uc_mode_h = S0_IDLE;					/* phase to IDLE */
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_PolErrProc		                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| When Pol send retry over occurred or released process                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : f_Occerd  1=occurred, 0=released						   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_PolErrProc( unsigned char f_Occurred )
{
	if( 1 == f_Occurred ){
		PIP_ARCsB0Regist(1, 1);	// エラー通知:外部接続機器未応答(発生)
	}
	else{
		PIP_ARCsB0Regist(1, 0);	// エラー通知:外部接続機器未応答(解除)
	}
}

//==============================================================================
//	データ送信ＮＧ系のエラー通知電文生成
//
//	@argument	f_Occurred	エラー状況(0:解除, 1:発生)
//	@argument	*pSData		シリアル送信した/しようとしたパケットデータ
//	@return		なし
//
//	@note		ＳＥＬパケットの送信に対して応答(ＡＣＫ/ＮＡＫ)が３００ｍｓ以内に
//				受信できない、またはＮＡＫ応答が連続して３回続いたとき、および
//				ＴＥＸＴの送信に対して応答(ＡＣＫ)が３００ｍｓ以内に受信できないときに
//				コールされ、ＩＢＫが保持しているエラーの発生状況に変化があれば
//				エラーの発生または解除をＢＯで通知する。
//				今回、エラー発生解除同時なので、解除は不要だが、仕組みは残す
//
//	@author		2012/10/19 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void NH_SelErrProc( unsigned char f_Occurred, unsigned char *pSData )
{
	unsigned char	ErrCode;

	switch( PIP_BcdAscToBin_2(&pSData[10]) ){		// ＩＤ３(データ種別)
	case 67:	// 送信パケットは精算データ(フラップ・ロック式)
		ErrCode = 40;
		break;
	case 68:	// 送信パケットはエラーデータ(フラップ・ロック式)
		ErrCode = 41;
		break;
	case 60:
	case 61:
	case 62:
	case 63:
	case 64:
	case 65:
	case 66:
	case 70:
	case 71:
	case 72:
	case 73:
	case 74:
	case 75:
	case 76:
		ErrCode = 42;
		break;
	default:
		return;
	}

	if( 1 == f_Occurred ){
		PIP_ARCsB0Regist(ErrCode, 1);	// エラー通知:４０～４２(発生/発生解除同時)
	} else {
		PIP_ARCsB0Regist(ErrCode, 0);	// エラー通知:今回未使用(解除)
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_IsTimeout	                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| timeout check						   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : TimNum (1-3)											   |*/
/*| RETURN VALUE : 1=timeout, 0=doesn't timeout                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
unsigned char	NH_IsTimeout( unsigned char TimNum )
{
	unsigned char	TOsts;
	unsigned long	TimVal = 0;

	TOsts = 0;

	switch( TimNum ){
	case	TIM1_NUM:
		TimVal = TIM1_VAL + (unsigned long)(PIPRAM.i_time_setting * 10);
		break;
	case	TIM2_NUM:
		TimVal = TIM2_VAL;
		break;
	case	TIM3_NUM:
		TimVal = TIM3_VAL;
		break;
	case	TIM4_NUM:
		TimVal = TIM1_VAL;
		break;
	}
	if( LifePastTimGet( PIPRAM.NH_StartTime ) >= TimVal ){
		TOsts = 1;
	}
	return	TOsts;
}
//==============================================================================
//	シリアル送信データポイント移動
//
//	@argument	なし
//
//	@return		なし
//
//	@attention	COUNT 0, CIND 0 は送信キューが空の状態を意味する。
//
//	@note		ＳＥＬに対して無応答、またはＮＡＫ応答が連続して３回続いたときに
//				にコールされ、シリアル送信キューとスルーデータバッファのデータを
//				削除する。
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void NH_SendReqCancel(void)
{
	PIPRAM.SCI_S_QUE.COUNT = 0;		// 送信キュークリア
	PIPRAM.SCI_S_QUE.CIND = 0;
}

/****************************************************************/
/*	機能	=	BCC演算処理										*/
/*	return	=	BCC値											*/
/****************************************************************/
unsigned char	Mt_BccCal( unsigned char *pSet, unsigned short Length)
{
	unsigned char	bcc;
	unsigned short	i;

	bcc = 0;
	for ( i=0; i<Length; i++ ) {
		bcc = bcc ^ pSet[i];
	}
	return	bcc;
}
/****************************************************************/
/*	関数名	=	Mh_get_receive									*/
/*	機能	=	HOST側受信データ取得							*/
/*	備考	=	Hostからのキャラクタ受信有無をチェックし、		*/
/*				受信していれば uc_rcv_byte_h にセットする。		*/
/*	return	=	1=受信キャラクタあり、0=なし					*/
/****************************************************************/
unsigned short	Mh_get_receive(void)
{
	/* 新たな受信データがあるかチェック */
	if ( PIPRAM.ui_count_h == 0 )					/* データなし */
	{
		return((unsigned short)0);
	}
	else									/* データあり */
	{
		PIPRAM.uc_rcv_byte_h = PIPRAM.uc_r_buf_h[PIPRAM.ui_read_h];	/* 割込み処理の受信バッファから１キャラクタget */
		if ( PIPRAM.ui_read_h == R_BUF_MAX_H-1 )		/* 読み出しポインタ更新 */
		{
			PIPRAM.ui_read_h = 0;
		}
		else
		{
			PIPRAM.ui_read_h++;
		}
		_di();
		PIPRAM.ui_count_h--;
		_ei();
		return((unsigned short)1);
	}
}
//==============================================================================
//	データ通信NGチェック
//
//	@argument	なし
//	@argument	なし
//	@return		エラー発生状況 ： 0 = エラーなし、1 = エラー発生
//	@note		エラー割り込み発生フラグで判断し、それぞれのエラーを生成し、
//				エラー生成後、フラグを解除する
//
//	@author		2012/10/11 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
unsigned char NH_RemoteErrProc()
{
	unsigned char error_result;

	error_result = 0;
	if( 1 == pip_f_txerr_h_ovrun ){	// オーバーランエラー発生
		PIP_ARCsB0Regist(ERR_REMOTE_SCI_OVERRUN, 1);	// エラー通知 : 62 オーバーランエラー
		pip_f_txerr_h_ovrun = 0;						// エラー状態解除
		error_result = 1;
	}
	if( 1 == pip_f_txerr_h_fr ){	// フレーミングエラー発生
		PIP_ARCsB0Regist(ERR_REMOTE_SCI_FRAME, 1);		// エラー通知 : 61 フレーミングエラー
		pip_f_txerr_h_fr = 0;							// エラー状態解除
		error_result = 1;
	}
	if( 1 == pip_f_txerr_h_p ){		// パリティエラー発生
		PIP_ARCsB0Regist(ERR_REMOTE_SCI_PARITY, 1);		// エラー通知 : 60 パリティエラー
		pip_f_txerr_h_p = 0;							// エラー状態解除
		error_result = 1;
	}
	return error_result;
}
/****************************************************************/
/*	関数名		=	MH_ScuRestart								*/
/*	機能		=	HOST通信側　シリアルコントローラリスタート	*/
/*	備考		=												*/
/****************************************************************/
void	NH_ScuRestart( void )
{
	sci_stop_h();

	PIPRAM.ui_txdlength_i_h = 0;		/* 送信割込み処理用 残送信レングス クリア 	*/
	PIPRAM.ui_txpointer_h = 0;			/* 送信割込み処理用 次送信データIndexクリア */

	PIPRAM.pip_uc_mode_h = S0_IDLE;		/*	状態モードをニュートラルにする	*/

	sci_init_h();				/* ＨＯＳＴ側受信起動 */
}

/****************************************************************************************************/
/*	関数名		:  PIP_BcdAscToBin_2 ()																	*/
/*	機能		:  2byte BCDアスキーをバイナリーに変換する											*/
/****************************************************************************************************/
unsigned char	PIP_BcdAscToBin_2 ( unsigned char *asc_dat)
{
	unsigned char	wuc_bcd;
	unsigned char	i;

	wuc_bcd = 0;
	for( i=0; i < 2; i++ ){
		wuc_bcd *= 10;
		if ( asc_dat[i] >= '0' && asc_dat[i] <= '9' ){
			wuc_bcd += ( asc_dat[i] - '0' );
		}else{
			wuc_bcd = 0;
		}
	}
	return( wuc_bcd );
}
//--------------------------------------------------------------------------------
// バイナリーを２バイトBCDｱｽｷｰに変換する
//
// BinDat(in) : Binary Data
// TextDat(out):BCD ASCII
//--------------------------------------------------------------------------------
void	PIP_BinToBcdAsc_2 ( unsigned char BinDat, unsigned char * TextDat )
{
	unsigned char	uwi;
	uwi = BinDat / 10;
	TextDat[0] = uwi + 0x30;
	uwi = BinDat % 10;
	TextDat[1] = uwi + 0x30;
}
//==============================================================================
//	エラー通知電文の生成
//
//	@argument	err_code	エラーコード
//	@argument	on_off		発生/解除(1:発生, 0:解除)
//
//	@return		なし
//
//	@attention	フラップ・ロック式の場合エラーコード０１以外、
//				解除(０)ではエラー電文を生成しません。
//
//	@note		エラー通知電文を生成しＡＲＣＮＥＴ送信キューに格納する。
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void PIP_ARCsB0Regist(unsigned char err_code, unsigned char on_off)
{
	unsigned char	module_code;
	unsigned char	c_work[2];

	switch( err_code ){
	case 1:
		module_code = 55;
		if( PIPRAM.Ctrl_B0_54.bits.B0_01 == on_off ) return;		// 発生中の発生、解除中の解除は受け付けない
		PIPRAM.Ctrl_B0_54.bits.B0_01 = on_off;
		on_off = on_off?1:2;										// 1:発生，2:解除
		break;
	case 40:
	case 41:
	case 42:
		if( on_off == 0 )	return;			// 発生解除同時なので解除のときには生成しない。
		module_code = 55;
		on_off = 3;							// フラップ・ロック式の場合(3:発生解除同時)
		break;
	case ERR_REMOTE_SCI_PARITY:		// シリアルパリティエラー
	case ERR_REMOTE_SCI_FRAME:		// シリアルフレーミングエラー
	case ERR_REMOTE_SCI_OVERRUN:	// シリアルオーバーランエラー
		module_code = 55;
		break;
	default:
		return;
	}
	memset(&PIPRAM.ARCs_B0_54_SendEdit[0], 0x30, PIP_ERR_DATLEN_IDB0);
	memcpy(&PIPRAM.ARCs_B0_54_SendEdit[0],"0101B00100", 10);	// ＩＤ１～機種コード

	PIP_BinToBcdAsc_2(module_code, c_work);						// モジュールコード
	PIPRAM.ARCs_B0_54_SendEdit[10] = c_work[0];
	PIPRAM.ARCs_B0_54_SendEdit[11] = c_work[1];

	PIP_BinToBcdAsc_2(err_code, c_work);						// エラーコード
	PIPRAM.ARCs_B0_54_SendEdit[12] = c_work[0];
	PIPRAM.ARCs_B0_54_SendEdit[13] = c_work[1];

	PIP_BinToBcdAsc_2(on_off, c_work);							// 発生・解除
	PIPRAM.ARCs_B0_54_SendEdit[14] = c_work[0];
	PIPRAM.ARCs_B0_54_SendEdit[15] = c_work[1];

	PIP_ChangeData(16, &PIPRAM.ARCs_B0_54_SendEdit[0]);		// シリアル受信データをIFMのバッファへ変換
}

//==============================================================================
//	シリアル送信キューへのデータ追加
//
//	@argument	*pData		IFMで生成した応答データパケット(ＩＤ１～)
//	@argument	length		応答データパケットデータ長
//
//	@return		シリアル送信キューへ追加したＳＴＸからＢＣＣまでのデータ長
//
//	@attention	引数 length の値は PIP_HOSTSEND_LEN_MAX バイト以内であること。
//
//	@note		IFMで生成した応答データパケットをシリアル送信キューに入れる。
//
//	@author		2012/10/1 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
unsigned short PIP_SciSendQue_insert(unsigned char *pData, unsigned short length)
{
	typedef struct {
		unsigned char	_1;	// ＩＤ１(データブロック№)
		unsigned char	_2;	// ＩＤ２(ブロックステータス)
		unsigned char	_3;	// ＩＤ３(種別コード)
	} t_ID;
	t_ID ID_now;

	ID_now._1 = PIP_BcdAscToBin_2( &pData[0] );		// ＩＤ１
	ID_now._2 = PIP_BcdAscToBin_2( &pData[2] );		// ＩＤ２
	ID_now._3 = PIP_BcdAscToBin_2( &pData[4] );		// ＩＤ３

	if( 1 < ID_now._1 ) {								// 先頭ブロックでない場合、今までと同一の種別である必要がある。
		t_ID ID_before;
		ID_before._1 = PIP_BcdAscToBin_2( &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT - 1 ].BUFF[1] );	// 前回のＩＤ１
		ID_before._3 = PIP_BcdAscToBin_2( &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT - 1 ].BUFF[5] );	// 前回のＩＤ３
		if( ID_now._3 != ID_before._3){					// 別種別の途中ブロック
			return( 0 );								// 受信パケット廃棄
		}
		if( ID_now._1 != ID_before._1 + 1 ){			// データブロック№が連番である必要がある。
			return( 0 );								// 受信パケット廃棄
		}
	}
	if( (ID_now._2 == 0)&&(length != PIP_HOSTSEND_LEN_MAX) ){	// 途中ブロックの場合データ長は必ず最大でないといけない。
		return( 0 );									// 受信パケット廃棄
	}
	if( PIPRAM.SCI_S_QUE.COUNT >= 79 ){
		if( PIPRAM.SCI_S_QUE.COUNT == 79 ){				// 保存位置が８０パケット目
			if( ID_now._2 != 1 ){						// 必ず終端パケットでないといけない。
				return( 0 );							// 受信パケット廃棄
			}
		} else {
			return( 0 );								// 受信パケット廃棄
		}
	}

	memset( &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT], 0, sizeof( PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT] ) );					// 新データ形式分0クリア
	PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[0] = STX;								// テキストブロックの先頭 追加
	length = (length < PIP_HOSTSEND_LEN_MAX ? length : PIP_HOSTSEND_LEN_MAX);
	memcpy( &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[1], pData, (size_t)length );	// ＩＤ１から テキストブロックをコピー
	length++;																					// ＳＴＸ分プラス

	PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[length] = ETX;							// テキストブロックの末尾 追加
	PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[length+1] = Mt_BccCal( (unsigned char*)&PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[1], length );	// BCC計算し、BBCを追加
	length += 2;																				// ＥＴＸとＢＣＣ分プラス
	PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].LEN += length;								// ＳＴＸ～ＢＣＣまでの長さ
	PIPRAM.SCI_S_QUE.COUNT++;																	// パケット挿入位置変更

	return( length );
}

