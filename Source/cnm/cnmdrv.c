/*[]----------------------------------------------------------------------[]*/
/*| Coinmech and Notereader communication driver                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 95-07-28                                                 |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"cnm_def.h"
#include	"suica_def.h"


/*[]----------------------------------------------------------------------[]*/
/*| Optional Sirial Communication Port COMMAND Send Process                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rtn = CNM_CMD( buff)                                    |*/
/*| PARAMETER    : buff     : Send Data Buffer Address                     |*/
/*|                kn       : Send Data kind                               |*/
/*|                           =0:input command =1:output command           |*/
/*| RETURN VALUE : short   rtn;     : 0:OK  -1 : NG                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short CNM_CMD( const unsigned char *buf, short knd )
{
	short	ret;
	ret = -1;

	if( (SCI2.SSR.BIT.ORER==0) && (SCI2.SSR.BIT.FER==0) && (SCI2.SSR.BIT.PER==0) ){
		*TXBUFF = *buf;
		*(TXBUFF+1) = *(buf+1);
		TXCONT = 2;						/* Set send data size */
		TXRPTR = 0;						/* Read Pointer <- 0 */
		TXSDCM = knd;					/* Set Command kind */

		SYN_ENB();						/* SYN enable */
// MH810103 GG119202(S) JVMA通信のt3タイマ修正
//		wait2us( 30L );					/* wait 60us */
		wait2us( 60L );					/* wait 120us */
// MH810103 GG119202(E) JVMA通信のt3タイマ修正
		SCI2.TDR = TXBUFF[TXRPTR];	/* Write 1st charactor	*/
		TXRPTR++;						/* TX charactor count +1 */
		dummy_Read = SCI2.SSR.BYTE;	/* Dummy Read					*/

		CN_tsk_timer = 11;						/* (200ms)TX monitor timer set */
		for( ; ; ) {
			if( SCI2.SSR.BIT.TEND == 1 ){		/* 送信終了? */
				if( TXRPTR >= TXCONT ){			/* TX complete?	*/
					ret = 0;
					break;
				}
				wait2us( 50L );				/* wait 100us */
				SCI2.TDR = TXBUFF[TXRPTR];	/* Write 1st charactor	*/
				dummy_Read = SCI2.SSR.BYTE;	/* Dummy Read					*/
				TXRPTR++;		/* TX charactor count +1 */
			}
			if( CN_tsk_timer == 0 ){			/* TIme out? */
				ret = -1;						/* Set time out */
				break;
			}
		}
	}
	SCI2.SCR.BIT.TIE = 0;					/* 送信割り込みdisable */
	TXWAIT = -1;							/* Set send stop		*/
	CN_tsk_timer = -1;
	return( ret );

}

/*[]----------------------------------------------------------------------[]*/
/*| Optional Sirial Communication Port Send Process                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rtn = CNM_SND( buff, size );                            |*/
/*| PARAMETER    : char  *buff;     : Send Data Buffer Address             |*/
/*|                short  size;     : Send Data Size                       |*/
/*| RETURN VALUE : short   rtn;     : 0:OK  -1 : NG                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short CNM_SND( unsigned char *buf, short siz )
{

	short	ret;
	ret = -1;

	if( siz == 0 ){
		return( -1 );
	}
	
	if( (SCI2.SSR.BIT.ORER==0) && (SCI2.SSR.BIT.FER==0) && (SCI2.SSR.BIT.PER==0) ){
		TXCONT = siz;							/* Set send data size */
		TXRPTR = 0;								/* Read Pointer <- 1 */
		TXSDCM = -1;							/* Set not Command */
		memcpy( TXBUFF, buf, (size_t)siz );
		SCI2.TDR = TXBUFF[TXRPTR];	/* Write 1st charactor	*/
		TXRPTR++;						/* TX charactor count +1 */
		dummy_Read = SCI2.SSR.BYTE;	/* Dummy Read					*/
// MH321800(S) G.So ICクレジット対応
//		CN_tsk_timer = 12;						/* (200ms)TX monitor timer set */
// MH810103 GG119202(S) CN_tsk_timerをVP接続とそうでない場合で分ける
//		CN_tsk_timer = CN_TX_TIMEOUT;			/* TX monitor timer set */
		if(isEC_USE()) {
			CN_tsk_timer = CN_TX_TIMEOUT;		/* TX monitor timer set */
		}else{
			CN_tsk_timer = 12;					/* TX monitor timer set */
		}
// MH810103 GG119202(E) CN_tsk_timerをVP接続とそうでない場合で分ける
// MH321800(E) G.So ICクレジット対応

		for( ; ; ) {
			if( SCI2.SSR.BIT.TEND == 1 ){		/* 送信終了? */
				if( TXRPTR >= TXCONT ){			/* TX complete?	*/
					ret = 0;
					break;
				}
				wait2us( 50L );				/* wait 100us */
				SCI2.TDR = TXBUFF[TXRPTR];	/* Write 1st charactor	*/
				dummy_Read = SCI2.SSR.BYTE;	/* Dummy Read					*/
				TXRPTR++;		/* TX charactor count +1 */
			}
			if( CN_tsk_timer == 0 ){			/* TIme out? */
				ret = -1;						/* Set time out */
				break;
			}
		}
	}
	SCI2.SCR.BIT.TIE = 0;					/* 送信割り込みdisable */
	TXWAIT = -1;							/* Set send stop		*/
	CN_tsk_timer = -1;
	return( ret );

}

/*[]----------------------------------------------------------------------[]*/
/*| Optional Sirial Communication Port Receive Process                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rtn = CNM_RCV( buff, size );                            |*/
/*| PARAMETER    : char  *buff;     : Receive Data Buffer Address          |*/
/*|                short *size;     : Receive Data Size                    |*/
/*| RETURN VALUE : short rtn;       : 0:OK  9:ERROR   -1:TIME OUT          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short
CNM_RCV( buf, siz )
unsigned char	*buf;
short	*siz;
{
	short	ret;

	ret = 0;
	RXWAIT = RCV_MON_TIM;	/* timer start */
	CNMTSK_START = 1; 	/* TASK START */
						/* 既に受信済みが考えられるため、ﾀｽｸ起動をかけておく */
	taskchg( IDLETSKNO );	/* Change task to idletask */
	CNMTSK_START = 0; 	/* TASK STOP */
	for( ; ; ) {
		_di();
		if( RXRSLT.BIT.B7 != 0 ){	/* Recieve complete? */
			if( 0 != (RXRSLT.BYTE & 0x3C) ) { 	// PHYエラーあり
				goto CNM_RCV_10;
			}
			memcpy( buf, RXBUFF, (size_t)RXCONT );
			*siz = RXCONT;
			break;
		}
		if( RXWAIT == 0 ){		/* TIme out? */

			if( 0 != (RXRSLT.BYTE & 0x3C) ) { 	// PHYエラーあり
CNM_RCV_10:
				// エラー登録
				cmn_ComErrorRegist();
			}

			if( Suica_Rec.Data.BIT.EVENT )		/* Suica Event? */
				SUICA_RCLR();					/* Recieve Inf. Clear */
			else								/* Coin Note Event? */
			CN_RCLR();			/* RXRSLT,RXWAIT Clear */
			ret = -1;			/* Set time out */
			break;
		}
		_ei();
		taskchg( IDLETSKNO );	/* Change task to idletask */
		CNMTSK_START = 0; 	/* TASK STOP */
	}
	RXWAIT = -1;
	_ei();
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*|  Wait Current Task with Taskchg                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnwait( time )                                          |*/
/*| PARAMETER    : short time	: Wait time ( 20ms )                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void cnwait( short time )
{
	CN_tsk_timer = time;

	taskchg( IDLETSKNO );	/* Change task to idletask */
	CNMTSK_START = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*|  20ms interrupt                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnm_timer                                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
cnm_timer( void )
{
	if( CN_tsk_timer > 0 ){
		CN_tsk_timer--;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| RECEIVE FLG RESET                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CN_RCLR                                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
CN_RCLR( void )
{
	RXRSLT.BYTE = 0;
	RXCONT = 0;
	RXDTCT = 0;
	RXWAIT = -1;
	memset( crd_buf, 0, sizeof( crd_buf ) );
}

/*[]----------------------------------------------------------------------[]*/
/*| SYN signal control                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SYN_ENB                                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
SYN_ENB( void )
{
	CP_CN_SYN = 0;		/* SYM Enable	*/

	CMT0.CMCNT = 0;				// タイマカウンタクリア
	CMT.CMSTR0.BIT.STR0 = 1;	// CMT0 動作開始
}

/*[]----------------------------------------------------------------------[]*/
/*| Coin Mech and Note Reder Hard Reset                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CN_reset                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
CN_reset( void )
{
	CP_CN_SYN = 0;			/* SYM Enable	*/
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	CP_CN_SYN = 1;			/* SYM Disnable	*/
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	Cnm_Force_Reset = 1;


}

/*[]----------------------------------------------------------------------[]*/
/*|  CNM_Int_ERI                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|         SCI Receive Error interrupt routine							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		: Sugata(FSI)											   |*/
/*| Date		: 2011-10-04											   |*/
/*| Update      : 														   |*/
/*[]---------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	CNM_Int_ERI( void )
{
	volatile unsigned char	c;

	if( SCI2.SSR.BIT.PER ){			// Parity Err?
		//	SCI2_Handler内のError処理
		RXRSLT.BIT.B3 = 1;
	}
	if( SCI2.SSR.BIT.FER ){			// Flaming Err?
		//	SCI2_Handler内のError処理
// MH810105 GG119202(S) JVMA通信エラーコードが違う
//		RXRSLT.BIT.B3 = 1;
		RXRSLT.BIT.B5 = 1;
// MH810105 GG119202(E) JVMA通信エラーコードが違う
	}
	if( SCI2.SSR.BIT.ORER ){		// Overrun Err?
		//	SCI2_Handler内のError処理
// MH810105 GG119202(S) JVMA通信エラーコードが違う
//		RXRSLT.BIT.B3 = 1;
		RXRSLT.BIT.B4 = 1;
// MH810105 GG119202(E) JVMA通信エラーコードが違う
	}
	c = SCI2.RDR;			// Received Data
	SCI2.SSR.BIT.ORER = 0 ;			// ERROR clear
	SCI2.SSR.BIT.FER = 0 ;
	SCI2.SSR.BIT.PER = 0 ;
	dummy_Read = SCI2.SSR.BYTE;		// Dummy Read
	
}

