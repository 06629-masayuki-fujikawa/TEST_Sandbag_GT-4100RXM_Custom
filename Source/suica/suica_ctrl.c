/*[]----------------------------------------------------------------------[]*/
/*| Suica control                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Namioka                                                |*/
/*| Date        : 06-07-12                                                 |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"irq1.h"
#include	"suica_def.h"
#include	"tbl_rkn.h"
#include	"lcd_def.h"
#include	"mdl_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

static short	suica_01( void );
static short	suica_02( void );
static short	suica_03( void );
static short	suica_04( void );
static uchar 	Suica_que_check( void );

static void 	Suica_recv_data_regist( uchar* , ushort );

// ◆◆◆◆◆　【SX-10制御部】(start)　◆◆◆◆◆ 
/*[]----------------------------------------------------------------------[]*/
/*| SX-10制御部		                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|・交通系ICカードリーダー（SX-10）の制御部で部版を取る。  	           |*/
/*|　"【SX-10制御部】" (start)～(end) までの間がSX-10制御部である。 　　　 |*/
/*|・この間のプログラムを変更した場合は、JREM様へ申請を行うこととする。    |*/
/*|・SX-10制御部として部版を持ち、変更の有無を明確にする。                 |*/
/*|・差異明確化のため、この間のコメント文は整理する。					   |*/
/*[]----------------------------------------------------------------------[]*/

/*[]----------------------------------------------------------------------[]*/
/*| SX-10制御部 部番                                                       |*/
/*[]----------------------------------------------------------------------[]*/
const	ver_rec		VERSNO_SX10CTRL = {
	'0',' ','M','H','3','2','8','8','0','0'		// 電子決済対応部番(Suica恒久対策版)FT4000用
};

/*[]----------------------------------------------------------------------[]*/
/*| Suica Initialize                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_init                                              |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void suica_init( void )
{
	short	ct;
	
	if( SuicaParamGet() ){		// if( prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1) を判定
		return;					// 設定異常
	}

	Suica_Rec.Data.BIT.INITIALIZE = 0;				/* Init Suica Reader */
	suica_errst = 0;								
	memset( suica_err,0,sizeof(suica_err) );
	memset( &suica_work_time, 0, sizeof( suica_work_time ));

    Suica_Rec.snd_kind = 0;
	Ope_Suica_Status = 0;							// Ope側からの送信ｽﾃｰﾀｽ初期化
	memset( &Suica_Snd_Buf,0,sizeof( SUICA_SEND_BUFF ));

	for( ct=0;ct<5; ct++){
		switch( ct ){
		    case 0: 								/* Stand-by */
				Suica_Rec.Com_kind = 0;					
				Suica_Rec.Status = STANDBY_BEFORE;
				break;
		    case 1: 								/* All req. */
				Suica_Rec.Com_kind = 1;					
				Suica_Rec.Status = STANDBY_SND_AFTER;
				break;
		    case 3:									/* Out req. */
				if( Suica_Rec.Status == DATA_RCV ){
					Suica_Data_Snd( S_INDIVIDUAL_DATA,&CLK_REC );
					suica_command_set();
					Suica_Rec.Com_kind = 3;
				}
				break;
		    case 2: 								/* Input req. */
		    case 4:
				Suica_Rec.Com_kind = 2;	
				break;

		    default:
				break;
		}
		if( 4 == suicaact() ){						// ACK4 receive
			ex_errlg( ERRMDL_SUICA, ERR_SUICA_ACK4RECEIVE, 2, 0 ); /* ｴﾗｰﾛｸﾞ登録(ACK4受信) */
			return;									// 次のPOL契機でスタンバイ送信(イニシャル未完)
		}
		suica_errst = suica_err[0];
		if( suica_errst )
			return;
		cnwait( 5 );								/* 100msec wait */
	}
	Suica_Rec.Data.BIT.INITIALIZE = 1;				/* Finished Initial */
	
	queset( OPETCBNO, SUICA_INITIAL_END, 0, NULL );	// OpeTaskにSuicaの初期化完了を通知
	
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Command Set                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_event                                             |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void suica_event( void )
{
		if( !Suica_Rec.Data.BIT.INITIALIZE ){
			suica_init();
			return;
		}
		suica_command_set();							/* 実行するｺﾏﾝﾄﾞを選択する */
		switch( suicaact()){							/* Suicaﾒｲﾝﾛｼﾞｯｸ */
			case 1: /* OK */
				break;
			case 2: 									/* Retry Err */
				suica_errst = suica_err[0];
				break;
			case 4: /* ACK4 */
				ex_errlg( ERRMDL_SUICA, ERR_SUICA_ACK4RECEIVE, 2, 0 ); /* ｴﾗｰﾛｸﾞ登録(ACK4受信) */
				suica_errst = suica_err[0];
				cnwait( 50 );							/* 1sec wait */
				suica_init();							/* Excute Initial */
				break;
			case -1: 									/* Receive Time Out */
			case -9: 									/* Send Time Out */
				if( (suica_errst & 0xc0 ) == 0 ){		/* Already Error? YJ */
					suica_errst = suica_err[0];
				}
				break;
			default:
				break;
		}
			
		return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Command Set                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_command_set                                       |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2007-02-19                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void suica_command_set( void )
{

	uchar	snd_kind = 0;
	memset( suica_work_buf,0,sizeof( suica_work_buf ));
	Suica_Snd_regist( &snd_kind , suica_work_buf );
	Suica_Rec.snd_kind = snd_kind;

	switch( Suica_Rec.snd_kind ){
		case S_CNTL_DATA: 					/* 制御ﾃﾞｰﾀの送信要求 */
			Suica_Rec.Snd_Size = 3;
			break;
		case S_SELECT_DATA:					/* 商品選択ﾃﾞｰﾀの送信要求 */
			Suica_Rec.Snd_Size = 9;
			break;
		case S_INDIVIDUAL_DATA:				/* 個別ﾃﾞｰﾀの送信要求 */
			Suica_Rec.Snd_Size = 25;
			break;
		case S_PAY_DATA:					/* 精算ﾃﾞｰﾀの送信要求 */
			Suica_Rec.Snd_Size = 10;
			break;
		default:							/* ｺﾏﾝﾄﾞ送信要求なし */
			Suica_Rec.Com_kind = 2;			/* Polling実施 */
			return;
			break;
	}

	Suica_Rec.Snd_Buf[0] = (unsigned char)(Suica_Rec.Snd_Size-1);	/* 送信ﾃﾞｰﾀ数＋種別の合計ｻｲｽﾞ */
	Suica_Rec.Snd_Buf[1] = Suica_Rec.snd_kind;						/* 送信種別 */
	Suica_Rec.Com_kind = 3;												/* ﾃﾞｰﾀ送信要求 */
	memcpy( &Suica_Rec.Snd_Buf[2],suica_work_buf,((sizeof(Suica_Rec.Snd_Buf[2]))*(Suica_Rec.Snd_Size-2)));	/* 送信ﾊﾞｯﾌｧにﾃﾞｰﾀをｾｯﾄ */
	Suica_Rec.Snd_Buf[Suica_Rec.Snd_Size] = bcccal( (char*)Suica_Rec.Snd_Buf, Suica_Rec.Snd_Size );		/* BCCﾃﾞｰﾀ付加 */
	Suica_Rec.Snd_Size = Suica_Rec.Snd_Size+1;															/* 送信ﾃﾞｰﾀのﾄｰﾀﾙｻｲｽﾞｾｯﾄ */

}


/*[]----------------------------------------------------------------------[]*/
/*| Suica-reader Communication Main  				                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suicaact                                                |*/
/*| PARAMETER	 : None										               |*/
/*| RETURN VALUE : short		| 1:OK	<>1:Error                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka	                                           |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short suicaact( void )
{
	short	rtn;

	rtn = 0;
	
	suica_errst = 0;								
	memset(suica_err,0,sizeof(suica_err));			/* ｴﾗｰ格納ｴﾘｱの初期化 */
	Suica_Rec.faze = 1;								/* ｺﾏﾝﾄﾞ送信ﾌｪｰｽﾞ移行 */

	for( ; rtn==0 ; ){
		switch( Suica_Rec.faze ){
			case 1:									/* ｺﾏﾝﾄﾞ送信 */
				rtn = suica_01();
				break;
			case 2:									/* ｺﾏﾝﾄﾞ送信結果取得ﾌｪｰｽﾞ */
				rtn = suica_02();
				break;
			case 3:									/* ﾃﾞｰﾀ送信ﾌｪｰｽﾞ */
				rtn = suica_03();
				break;
			case 4:									/* ﾃﾞｰﾀ送信結果受信ﾌｪｰｽﾞ */
				rtn = suica_04();
				break;
			default:
				break;
		}
		switch( rtn ) {
			case 0:	 								/* Continue */
				continue;
				break;
			case 4:  								/* ACK4 */
				break;
			case -9: 								/* 送信TimeOut */
				if( suica_errst & 0x80 ){  			/* Already Timeout?  */
					suica_err[0] |= 0x80;			
					break;
				}
				suica_err[2]++;						/* Time Out Retry Counter +1 */
				if( suica_err[2] <= SUICA_RTRY_COUNT_31 ){  /* ﾘﾄﾗｲが31未満の場合 */
					Suica_Rec.faze = 1;                     /* ﾘﾄﾗｲを実施 */
					Suica_Rec.Com_kind = 5;
					rtn = 0;
				}else{
					suica_err[0] |= 0x80;			/*ｴﾗｰをｾｯﾄ */		
				}
				break;
			case -1: 								/* 受信TimeOut */
				if( Suica_Rec.Status == STANDBY_SND_AFTER ){ /* ｽﾃｰﾀｽがｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信後 */
					Suica_Rec.Com_kind = 0;					 /* 再度、ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞを送信する */
					Suica_Rec.faze = 1;
					suica_err[0] |= 0x40;			/*ｴﾗｰをｾｯﾄ */
					break;
				}
					
				if( suica_errst & 0x40 ){  			/* Already Timeout?  */
					suica_err[0] |= 0x40;			
					break;
				}
				suica_err[1]++;						/* Time Out Retry Counter +1 */
				if( suica_err[1] <= SUICA_RTRY_COUNT_31 ){  /* ﾘﾄﾗｲが31未満の場合 */
					Suica_Rec.faze = 1;                     /* ﾘﾄﾗｲを実施 */
					rtn = 0;
				}else{
					suica_err[0] |= 0x40;			/*ｴﾗｰをｾｯﾄ */		
				}
				break;
			default:
				break;
		}
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 1...Send Command                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_01		                                           |*/
/*| PARAMETER	 : None									                   |*/
/*| RETURN VALUE : short 	| 0:Continue -9:Send Timeout                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static short suica_01()
{
	short	sig;
	unsigned char	*scp;

	sig = Suica_Rec.Com_kind;			/* 送信ｺﾏﾝﾄﾞをﾜｰｸｴﾘｱに設定 */
	scp = (unsigned char *)&suica_tbl[sig][0];			/* 送信ｺﾏﾝﾄﾞIDを設定 */

	SUICA_RCLR();						// 受信バッファクリア
	if( CNM_CMD( scp, sig ) ) {			/* Send COMMAND */
		return( -9 );					/* TIME OUT */
	}
	Suica_Rec.Status = (unsigned char)(sig+1); /* 現在の状態を保存 */

	switch( sig ){
		case 2:
		case 0:
		case 1:
		case 4:
			Suica_Rec.faze = 2;			/* ｺﾏﾝﾄﾞ送信結果取得ﾌｪｰｽﾞに移行 */
			break;
		case 3:
		case 5:
			Suica_Rec.faze = 3;			/* ﾃﾞｰﾀ送信ﾌｪｰｽﾞに移行 */
			break;
		default:
			break;
	}
	if( sig == 4 ){
		err_chk( ERRMDL_SUICA, ERR_SUICA_REINPUT, 2, 0, 0 );		
	}else if( sig == 5 ){
		err_chk( ERRMDL_SUICA, ERR_SUICA_REOUTPUT, 2, 0, 0 );
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 2...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_02                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short    | 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka	                                           |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static short suica_02( void )
{
	short	rtn;
	unsigned char	status;
	
	status = Suica_Rec.Status;												/* ｽﾃｰﾀｽの保持 */

	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* ﾃﾞｰﾀ受信 */
		// 0以外は-1（タイムアウト）
suica_02_10:
		rtn = -1;															// タイムアウトで処理する
		if( 2 == Suica_Rec.Com_kind ){										// 入力要求送信後の無応答
			Suica_Rec.Com_kind = 4;											// 入力再要求コマンド送信
		}																	// 一括要求の無応答は再度一括要求を行うこととする
		return( rtn );
	}

	switch( rtn = suica_recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){	/* 受信ﾃﾞｰﾀ解析処理 */
		case 1: /* ACK1 */
			if( ( status == STANDBY_SND_AFTER ||							/* ｺﾏﾝﾄﾞ送信後、ﾃﾞｰﾀ送信要求がある場合、   */
				  status == INPUT_SND_AFTER   ||							/* 出力要求ｺﾏﾝﾄﾞ送信後、ﾃﾞｰﾀ送信を行う */
				  status == REINPUT_SND_AFTER )) {
				  if( Suica_que_check() ){
					suica_command_set();
					Suica_Rec.faze = 1;
					Suica_Rec.Com_kind = 3;										/* 出力要求ｺﾏﾝﾄﾞ送信処理を行う */
					rtn = 0;
					break;
				  }
			}
			rtn = 1;														/* 入力要求を行う */
			break;
		case 2: /* ACK2 */
		case 3: /* ACK3 */
			if( (Suica_Rec.Com_kind == 0) ||								// ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信後
				(Suica_Rec.Com_kind == 1) ){								// 一括要求送信後
				goto suica_02_10;											// 無視
			}
			rtn = 1;														
			break;
		case 6: /* NAK */
			if( (Suica_Rec.Com_kind == 0) ||								// ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信後
				(Suica_Rec.Com_kind == 1) ){								// 一括要求送信後
				goto suica_02_10;											// 再送
			}
			rtn = 1;
			Suica_Rec.Status = DATA_RCV;									// 他UNITへのPOLに移る
			break;
		case 4: /* ACK4 */
			Suica_Rec.Com_kind = 0;											/* ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信 */
			Suica_Rec.faze = 1;
			Suica_Rec.Status = DATA_RCV;									// 初期ｼｰｹﾝｽ中であれば、他UNITへのPOLに移り次はスタンバイ
																			// 初期ｼｰｹﾝｽ中でなければ、即時スタンバイ送信
			break;
		case 5: /* ACK5 */
			goto suica_02_10;
			break;

		case 9:	/* LRC ERROR */												// PHY error、レングス異常を含む
			if( Suica_Rec.Com_kind == 0 ){									// ｽﾀﾝﾊﾞｲ送信後
				goto suica_02_10;											// 無視（ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞの再送）
			}
			if( suica_errst & 0x10 ){  										/* Already RetryErr?  */
				suica_err[0] |= 0x10;
				rtn = 2;
				break;
			}
			suica_err[4]++;													/* ﾘﾄﾗｲ回数をｶｳﾝﾄｱｯﾌﾟ */
			if( suica_err[4] <= SUICA_RTRY_COUNT_31 ){						// ﾘﾄﾗｲｵｰﾊﾞｰでない
				if( 2 == Suica_Rec.Com_kind ){								// 入力要求送信後の再送（一括要求時は再送も一括）
					Suica_Rec.Com_kind = 4;									// 入力再要求コマンド送信
				}
				Suica_Rec.faze = 1;
				rtn = 0;	/* Retry */
			}else{															/* ﾘﾄﾗｲ回数ｵｰﾊﾞｰ */
				suica_err[0] |= 0x10;										/* ｴﾗｰｾｯﾄ */
				rtn = 2;
			}
			break;
		default:
			goto suica_02_10;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 3...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_03                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short	| 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static short suica_03( void )
{
	short	rtn;
	unsigned char	status;
	
	status = Suica_Rec.Status;												/* ｽﾃｰﾀｽの保持 */

	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* ﾃﾞｰﾀ受信 */
suica_03_10:
		rtn = -1;															// タイムアウトで処理する
		return( rtn );
	}

	switch( rtn = suica_recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){	/* 受信ﾃﾞｰﾀ解析処理 */
		case 1: /* ACK1 */
			if( CNM_SND(Suica_Rec.Snd_Buf, Suica_Rec.Snd_Size) == 0 ){ 		/* ﾃﾞｰﾀ送信 */
				Suica_Rec.snd_kind = 0;
				Suica_Log_regist( &Suica_Rec.Snd_Buf[1],(ushort)(Suica_Rec.Snd_Size-2), 1 );	/* 通信ログ登録 */	
				
				#if (4 == AUTO_PAYMENT_PROGRAM)								// 試験用に通信ログ取得方式を変更する
				Suica_Log_regist_for_debug( &Suica_Rec.Snd_Buf[1],(ushort)(Suica_Rec.Snd_Size-2), 1 );	/* 通信ログ登録 */	
				#endif
				

				Suica_Rec.faze = 4;								/* ﾃﾞｰﾀ送信後、送信結果ﾌｪｰｽﾞに移行 */
				rtn = 0;
				break;
			}else{
				rtn = -9;
				break;
			}
		case 2: /* ACK2 */
		case 3: /* ACK3 */
			rtn = 1;											/* OK */
			break;
		case 4: /* ACK4 */										/* 出力要求・出力再要求ｺﾏﾝﾄﾞ送信後にACK4受信時 */				
			Suica_Rec.faze = 1;								    /* ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信 */ 			
			Suica_Rec.Com_kind = 0;												
			break;
		case 5: /* ACK5 */
			if( status == REOUTPUT_SND_AFTER ){					/* 出力再指令送信後 */
				Suica_Rec.faze = 4;								// 送信出来たと判断し、送信データ消去
				if( Suica_que_check() ){							/* ｺﾏﾝﾄﾞ送信要求がある場合 */
					suica_command_set();
					Suica_Rec.faze = 1;								
					Suica_Rec.Com_kind = 3;							/* 出力指令送信 */
					rtn = 0;		
				}
				else{
					Suica_Rec.faze = 3;							// 送信データ消去のために変えた値を元に戻す
					rtn = 1;									// 次の周期に出力要求する(ACK5時はデータの再送をしない)
				}
			}
			else{												// 出力指令
				goto suica_03_10;								// 再送
			}
			break;
		case 6: /* NAK */
			ex_errlg( ERRMDL_SUICA, ERR_SUICA_NAKRECEIVE, 2, 0 ); /* ｴﾗｰﾛｸﾞ登録(NAK受信) */
			if( suica_errst & 0x08 ){  							/* Already RetryErr?  */
				suica_err[0] |= 0x08;							/* ｴﾗｰをｾｯﾄ  */
				rtn = 2;
				break;
			}
			suica_err[3]++;										/* ﾘﾄﾗｲ回数をｶｳﾝﾄｱｯﾌﾟ */
			if( suica_err[3] <= SUICA_RTRY_COUNT_21 ){			/* ﾘﾄﾗｲ回数が21回未満 */
				Suica_Rec.faze = 1;								/* ﾘﾄﾗｲ実施 */
				wait2us( 2500L ); 								/* 5ms wait */
				rtn = 0;
			}else{
				suica_err[0] |= 0x08;							/* ｴﾗｰをｾｯﾄ */
				rtn = 2;
			}
			break;
		default:
			break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 4...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_04                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short | 0:Continue 1:OK ETC:Error                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka	                                           |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static short suica_04( void )
{
	short	rtn;
	
	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* ﾃﾞｰﾀ受信 */
		rtn = -1;															// タイムアウトで処理する
		Suica_Rec.Com_kind = 5;												// 次は出力再指令送信
		return( rtn );
	}
	switch( rtn = suica_recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){
		case 1: /* ACK1 */
			if( Suica_Rec.Snd_Buf[1] == S_SELECT_DATA ){
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 1;						// Suica精算中フラグON！
				Suica_Rec.Data.BIT.SEND_CTRL80 = 0;
			}
			else if(( Suica_Rec.Snd_Buf[1] == S_CNTL_DATA && Suica_Rec.Snd_Buf[2] == 0x80 ) && 
					  !Suica_Rec.Data.BIT.SEND_CTRL80 ){
				Suica_Rec.Data.BIT.SEND_CTRL80 = 1;
			}
		case 5: /* ACK5 */
			if( Suica_que_check() ){
				suica_command_set();
				Suica_Rec.faze = 1;								     
				rtn = 0;		
			}else 	rtn = 1;		

			break;
		case 2: /* ACK2 */
		case 3: /* ACK3 */
			rtn = 1; /* OK */
			break;
		case 4: /* ACK4 */										/* 出力要求・出力再要求ｺﾏﾝﾄﾞ送信後にACK4受信時 */				
			Suica_Rec.faze = 1;								    /* ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信 */ 			
			Suica_Rec.Com_kind = 0;												
			break;
		case 6: /* NAK */
			ex_errlg( ERRMDL_SUICA, ERR_SUICA_NAKRECEIVE, 2, 0 ); 			/* ｴﾗｰﾛｸﾞ登録(NAK受信) */
			if( suica_errst & 0x08 ){  										/* Already RetryErr?  */
				suica_err[0] |= 0x08;
				rtn = 2;
				break;
			}
			suica_err[4]++;
			if( suica_err[4] <= SUICA_RTRY_COUNT_3 ){
				Suica_Rec.faze = 1;
				Suica_Rec.Com_kind = 5;							// 出力再指令
				rtn = 0;
			}else{
				suica_err[0] |= 0x08;
				rtn = 2;
			}
			break;

		case 9:	/* LRC ERROR */
		default:
			if( suica_errst & 0x10 ){  										/* Already RetryErr?  */
				suica_err[0] |= 0x10;
				rtn = 2;
				break;
			}
			suica_err[4]++;
			if( suica_err[4] <= SUICA_RTRY_COUNT_3 ){
				Suica_Rec.Com_kind = 5;										// 次は出力再指令
				Suica_Rec.faze = 1;
				rtn = 0;	/* Retry */
			}else{
				suica_err[0] |= 0x10;
				rtn = 2;
			}
			break;
	}
	return( rtn );
}


/*[]----------------------------------------------------------------------[]*/
/*| Suica Snd Data que Check                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Suica_que_check                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-09-29                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar Suica_que_check( void )
{
	uchar i;

	if( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind && Suica_Rec.faze == 4 ){
		Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind = 0;

		if( Suica_Snd_Buf.read_wpt > 3 )
			Suica_Snd_Buf.read_wpt = 0;
		else
			Suica_Snd_Buf.read_wpt++;
	}

	for( i=0;i<5;i++ ){
		if( Suica_Snd_Buf.Suica_Snd_q[i].snd_kind ){
			return 1;
		}
	}
	if( Suica_Snd_Buf.read_wpt != Suica_Snd_Buf.write_wpt )		// 未送信ﾃﾞｰﾀがなく、RptとWptがずれている場合
		Suica_Snd_Buf.read_wpt = Suica_Snd_Buf.write_wpt;		// RptをWptにあわせ、送信の矛盾をなくす

	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| Get Receive Charactor Inf.                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_recv                                              |*/
/*| PARAMETER	 : char *buf    : Receive Buffer Address                   |*/
/*|              : short   siz    : number of Receive Charactor            |*/
/*| RETURN VALUE : short ret;     : 1:ACK1+DATA                            |*/
/*|              : 2:ACK2+DATA                                             |*/
/*|              : 3:ACK3+DATA                                             |*/
/*|              : 4:ACK4                                                  |*/
/*|              : 5:ACK5                                                  |*/
/*|              : 6:NAK                                                   |*/
/*|              : 7:ETC. Char.                                            |*/
/*|              : 9:LRC ERROR                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short suica_recv( unsigned char *buf,short siz )
{
	short	i, rtn, bc;
	unsigned char	*ack;

	// 決済結果データだけは処理済で無い限り受信情報を消さない
	if( (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 		// 決済結果ﾃﾞｰﾀを受信し
		(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) )		// 決済結果ﾃﾞｰﾀ未処理の場合
	{
		Suica_Rec.suica_rcv_event.BYTE = 0;
		Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;				// 決済結果データだけは消さない
	}
	else{
		Suica_Rec.suica_rcv_event.BYTE = 0;
	}
	Suica_Rec.suica_rcv_event_sumi.BYTE = 0;

	rtn = 0;
	ack = (unsigned char *)ack_tbl;
	for( i=1; i<7; i++,ack++){
		if(*buf == *ack) break;
	}

	if(( siz > 1 ) && ( i < 4 )) {		// ACK2,3 + データ電文受信
		buf++;
		bc = (short)(*buf);	/* bc <- Total Byte */
		for( ; ; ){
		    if( siz != (bc+3) ){	/* Recieve Size OK ? YJ */
				ex_errlg( ERRMDL_SUICA, ERR_COIN_RECEIVESIZENG, 2, 0 );	// Recv packet size error
				i = 9;			/* Size Error 	*/
				break;
		    }
		    if( (unsigned char)(*(buf+bc+1)) != bcccal( (char*)buf, (short)(bc+1) ) ){ /* LRC Ok ? YJ */
				ex_errlg( ERRMDL_SUICA, ERR_COIN_RECEIVEBCCNG, 2, 0 );	// BCC error
				i = 9;			/* LRC Error */
				break;
		    }

			if( Suica_Rec.Status == STANDBY_SND_AFTER ){		// ｽﾀﾝﾊﾞｲ送信後のﾃﾞｰﾀ受信のためﾃﾞｰﾀ破棄
				SUICA_RCLR();	/* Recieve Inf. Clear */
				return (i);
			}

			if( i >= 1 || i <= 3 ){
				Suica_Log_regist( (uchar*)(buf+(i!=2?1:0)),(ushort)(i!=2?bc:bc+1), 0 );
				buf--;											// 受信データの先頭バイトを指定
				Suica_recv_data_regist( buf,(ushort)(bc+2) );	// 受信キューへ格納
				queset( OPETCBNO, SUICA_EVT, 0, NULL );			// OpeTaskへ受信通知
				Suica_Rec.Status = DATA_RCV;
			}

		    break;
		}
	}else if( siz == 1 ){
		if( *buf == 0x11 )
			Suica_Rec.Status = DATA_RCV;
	}
	SUICA_RCLR();	/* Recieve Inf. Clear */
	rtn = i;
	return( (short)rtn );
}


/*[]----------------------------------------------------------------------[]*/
/*| Suica Trouble   	                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suicatrb                                                |*/
/*| PARAMETER	 : err_kind:ｴﾗｰ種別							               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void suicatrb( unsigned char err_kind )
{

	if( err_kind == ERR_SUICA_COMFAIL ){									/* ｴﾗｰ種別が通信不良の場合 */
		err_suica_chk(  &suica_errst ,&suica_errst_bak,ERR_SUICA_COMFAIL );	/* ｴﾗｰﾁｪｯｸ */
		Suica_Rec.suica_err_event.BIT.COMFAIL = 1;							/* 通信不良ｴﾗｰﾌﾗｸﾞON */
		suica_errst_bak = suica_errst;										/* ｴﾗｰ状態をﾊﾞｯｸｱｯﾌﾟ */
	}

	if( !Suica_Rec.Data.BIT.RESET_COUNT ){									/* 初期化未実行？ */
		_di();																/* 初期化処理実行 */
		Sci2Init();
		_ei();
		Suica_reset();
		suica_init();
		Suica_Rec.Data.BIT.RESET_COUNT = 1;									/* 初期化済みﾌﾗｸﾞｾｯﾄ */
		Suica_Rec.Data.BIT.EVENT ^= 0x01;									/* Suicaｲﾍﾞﾝﾄ切り替え(ｺｲﾝﾒｯｸﾀｽｸ処理移行) */

	}

}

/*[]----------------------------------------------------------------------[]*/
/*| RECEIVE FLG RESET                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SUICA_RCLR                                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void SUICA_RCLR( void )
{
	RXRSLT.BYTE = 0;
	RXCONT = 0;
	RXDTCT = 0;
	RXWAIT = -1;
	memset( Suica_Rec.Rcv_Buf, 0, sizeof( Suica_Rec.Rcv_Buf ) );
}

/*[]----------------------------------------------------------------------[]*/
/*| SUICA Reder Hard Reset                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CN_reset                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void Suica_reset( void )
{
	CP_CN_SYN = 0;			/* SYM Enable	*/
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	CP_CN_SYN = 1;			/* SYM Disnable	*/
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
}


/*[]----------------------------------------------------------------------[]*/
/*| snd_ctrl_timeout                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : snd_ctrl_timeout                                        |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2007-02-26                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void snd_ctrl_timeout( void )
{
	unsigned char wks = 0;

	wks = (uchar)prm_get(COM_PRM, S_SCA, 14, 2, 1);		/* 再初期化ﾘﾄﾗｲ回数取得 */	
	LagCan500ms( LAG500_SUICA_STATUS_TIMER );			/* SX-10状態監視ﾀｲﾏﾘｾｯﾄ */
	if( Retrycount < wks ){								/* 再初期化ﾘﾄﾗｲ回数ｵｰﾊﾞｰ？ */
		suicatrb( ERR_SUICA_RECEIVE );					/* 初期化処理実行 */
		Retrycount++;									/* ﾘﾄﾗｲｶｳﾝﾀの更新 */
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			受信ﾃﾞｰﾀ登録処理
//[]----------------------------------------------------------------------[]
///	@param[in]		buf		:登録するﾃﾞｰﾀﾎﾟｲﾝﾀ
///	@param[in]		size	:登録するﾃﾞｰﾀｻｲｽﾞ<br>
///	@return			void		:
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 2008/07/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void 	Suica_recv_data_regist( uchar *buf , ushort size )
{
	uchar	*wbuf = suica_rcv_que.rcvdata[suica_rcv_que.writept];					// 今回書込む受信ﾊﾞｯﾌｧﾎﾟｲﾝﾀをｾｯﾄ
	uchar	wsize;																	// 検索用ｴﾘｱ
	ushort	i;																		// ﾙｰﾌﾟ用変数
	ushort	w_counter[3];															// ｵﾌｾｯﾄ編集ｴﾘｱ

	memcpy( w_counter, &suica_rcv_que.readpt, sizeof( w_counter ) );				// ｵﾌｾｯﾄをﾜｰｸ領域に展開
	if( (w_counter[2] == SUICA_QUE_MAX_COUNT) && (w_counter[1] == w_counter[0]) ){	// ﾊﾞｯﾌｧがﾌﾙ状態の場合

		for( i=0; (i<6 && *wbuf != ack_tbl[i]); i++);								// 受信種別(ACK1～NACK)検索

		if( i == 1 ){																// 単体ﾃﾞｰﾀの場合
			if( *(wbuf+2) == S_SETTLEMENT_DATA && *(wbuf+3) == 0x01 ){				// 有効な決済結果ﾃﾞｰﾀの場合
				Settlement_rcv_faze_err( wbuf+3, 6 );								// ｴﾗｰ登録(E6961)
			}			
		}else{																		// 複数ﾃﾞｰﾀの場合
			wsize = *(wbuf+1);														// ﾃﾞｰﾀｻｲｽﾞを設定
			wbuf++;																	// ﾃﾞｰﾀﾎﾟｲﾝﾀを更新
			for( i=0; i<wsize; ){													// 全ﾃﾞｰﾀを検索するまでﾙｰﾌﾟ
				if( *(wbuf+2) == S_SETTLEMENT_DATA && *(wbuf+3) == 0x01 ){			// 有効な決済結果ﾃﾞｰﾀ
					Settlement_rcv_faze_err( wbuf+3, 6 );							// ｴﾗｰ登録(E6961)
				}
				i+=*(wbuf+1)+1;														// 検索済みのﾃﾞｰﾀｻｲｽﾞを更新
				wbuf+=*(wbuf+1)+1;													// ﾃﾞｰﾀﾎﾟｲﾝﾀを更新し、次のﾃﾞｰﾀを参照
			}			
		}

		if( ++w_counter[0] >= SUICA_QUE_MAX_COUNT ){								// 読込みｵﾌｾｯﾄ更新＆次に読込むｵﾌｾｯﾄがMAX越えの場合
			w_counter[0] = 0;														// ｵﾌｾｯﾄを先頭に移動
		}
	}
	memcpy( suica_rcv_que.rcvdata[w_counter[1]], buf, (size_t)size );				// 受信してきたﾃﾞｰﾀを受信ｷｭｰに登録
	if( ++w_counter[1] >= SUICA_QUE_MAX_COUNT ){									// 書込みｵﾌｾｯﾄ更新＆次に読込むｵﾌｾｯﾄがMAX越えの場合
		w_counter[1] = 0;															// ｵﾌｾｯﾄを先頭に移動
	}
	
	if( w_counter[2] < SUICA_QUE_MAX_COUNT ){										// ﾃﾞｰﾀｶｳﾝﾄがMAXでなければ
		w_counter[2]++;																// ﾃﾞｰﾀｶｳﾝﾄをｲﾝｸﾘﾒﾝﾄ
	}
	nmisave( &suica_rcv_que.readpt, w_counter, sizeof(w_counter));					// ｵﾌｾｯﾄを停電保障で更新

}
