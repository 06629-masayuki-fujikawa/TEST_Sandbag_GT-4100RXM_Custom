/*[]----------------------------------------------------------------------[]*/
/*| Coinmech and Notereader control                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 95-07-28                                                 |*/
/*| Update      : 2001-11-01                                               |*/
/*| Update      : 2004-06-30 T.Nakayama                                    |*/
/*|  V25->H8S convert                                                      |*/
/*|  Coinmech and notereader are changed the one manufactured by CONLAX.   |*/
/*[]----------------------------------------------------------------------[]*/
/*|  2004.6.30 T.Nakayama										           |*/
/*|  ｴｽｸﾛﾀｲﾑｱｳﾄは4秒(CN_escr_timer)のままにしなければならない              |*/
/*|  20msに変更したところ、金庫計算中となり、op_mod02でｴﾗｰとなる為。       |*/
/*|  2006/5/8 MH544402 紙幣金庫合計が狂う対策							   |*/
/*|    (GT4700 FCR A060080のUT8500へ反映) 								   |*/
/*|    紙幣ｴｽｸﾛさせる。ｺｲﾝ投入し精算完了とほぼ同時にｴｽｸﾛ紙幣を別紙幣で押し込む */
/*|    紙幣金庫枚数がｶｳﾝﾄしない。										   |*/
/*|    紙幣金庫のｶｳﾝﾄを取り込み後ではなくﾘｰﾄﾞしたときに行うように修正	   |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"Message.h"								/* Message				*/
#include	"tbl_rkn.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"mnt_def.h"
#include	"suica_def.h"
#include	"ntnet_def.h"
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern ushort	lcdbm_alarm_check(void);
extern void		lcdbm_notice_alm(ushort	warning);
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

static	char 	cnm_sig = 0;	/* Send Command Kind */
static	short	cn_faze = 1;	/* Communication Faze */
static	char cn_err[4] = { 0 };	/* Communication  Status */
// MH321800(S) T.Nagai ICクレジット対応
static	uchar	cn_reset_reserved = 0;
static	uchar	cn_reset_count = 0;
// MH321800(E) T.Nagai ICクレジット対応

static void	cnmtrb( short );
static void	count_set_sub( unsigned char * );

static short	cnm_01( short );
static short	cnm_02( void );
static short	cnm_03( void );
static short	cnm_04( void );
static void		cmn_Note_Refund_Chk( uchar * );


/*[]----------------------------------------------------------------------[]*/
/*| Coin-mech and Note-reader Task Main                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnmtask                                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
cnmtask( void )
{
	short	tn;	/* Terminal No. */
	unsigned char	cn_errst_bak[2];

// MH321800(S) G.So ICクレジット対応
	jvma_init();							/* Suica機能初期化処理 */
// MH321800(E) G.So ICクレジット対応
	cn_init();	/* Coin and Note Initial */
	cn_errst[0] = 0; /* Error status Clear */
	cn_errst[1] = 0; /* Error status Clear */

// MH321800(S) G.So ICクレジット対応
//	suica_init();							/* Suica機能初期化処理 */
	jvma_setup.init();
// MH321800(E) G.So ICクレジット対応
	for( tn=0 ; ;  ){
		if( Suica_Rec.Data.BIT.EVENT ){		/* Suica Event発生? */
// MH321800(S) G.So ICクレジット対応
//			suica_event();					/* Suica Main処理 */
			jvma_event();
// MH321800(E) G.So ICクレジット対応
		}else{
		if( tn ){
#ifdef	SUICA_EAON_DEBUG_MODE
			goto SuicaDebugJump;
#endif
			ntr_set();
		} else {
			cnm_set();
		}
		switch( cnmact( tn )){
			case 1: /* OK */
				break;
			case 4: /* ACK4 */
				if( tn ){
					ex_errlg( ERRMDL_NOTE, ERR_NOTE_ACK4RECEIVE, 2, 0 ); /* ｴﾗｰﾛｸﾞ登録(ACK4受信) */
				}else{
					ex_errlg( ERRMDL_COIM, ERR_COIN_ACK4RECEIVE, 2, 0 ); /* ｴﾗｰﾛｸﾞ登録(ACK4受信) */
				}
				cn_errst[tn] = cn_err[0];
				cnwait( 50 );		/* 1sec wait */
				cnm_mem( 1 );
				cn_init();			/* Excute Initial */
				continue;
				break;
			case -1: /* Receive Time Out */
			case -9: /* Send Time Out */
				if( (cn_errst[tn] & 0xc0 ) == 0 ){
							/* Already Error? YJ */
// MH321800(S) T.Nagai ICクレジット対応
//					cn_errst[tn] = cn_err[0];
//					cn_errst_bak[0] = cn_errst[0];
//					cn_errst_bak[1] = cn_errst[1];
// MH321800(E) T.Nagai ICクレジット対応
					if( tn ){
						err_chk( ERRMDL_NOTE, ERR_NOTE_COMFAIL, 1, 0, 0 ); /* ｴﾗｰﾛｸﾞ登録 */
					}else{
						err_chk( ERRMDL_COIM, ERR_COIN_COMFAIL, 1, 0, 0 ); /* ｴﾗｰﾛｸﾞ登録 */
					}
// MH321800(S) T.Nagai ICクレジット対応
//					_di();
//					Sci2Init();
//					//SCI1_init(); /* Initial Sirial IC */
//					_ei();
//					cnm_mem( 1 );
//					CN_reset();	/* Mech Hard Reset */
//					cn_init();	/* Excute Initial */
//					cn_errst[0] = cn_errst_bak[0];
//					cn_errst[1] = cn_errst_bak[1];
//					continue;
					cn_reset_reserved = 1;
// MH321800(E) T.Nagai ICクレジット対応
				}
				break;
			default:
				break;
		}
		if( cn_err[0] ){
			cnmtrb( tn );		/* Trouble Check */
		}
		else { 
			if( cn_errst[tn] != 0 ){
				if( tn ){
					;
				} else {
					cnm_set();
				}
			}
// MH321800(S) T.Nagai ICクレジット対応
			cn_reset_count = 0;
// MH321800(E) T.Nagai ICクレジット対応
		}
		cn_errst[tn] = cn_err[0];
#ifdef	SUICA_EAON_DEBUG_MODE
SuicaDebugJump:
#endif
// MH321800(S) T.Nagai ICクレジット対応
		if (!cn_reset_count &&					// 初期化未実行？
			cn_reset_reserved &&				// 初期化保留中？
// MH810103 GG119202(S) JVMAリセット処理変更
			OPECTL.Ope_mod != 1 &&				// 車室番号入力中ではない？
// MH810103 GG119202(E) JVMAリセット処理変更
			OPECTL.Ope_mod != 2 &&				// 精算中状態ではない？
			OPECTL.Ope_mod != 3) {				// 精算完了状態ではない？
			cn_errst_bak[0] = cn_errst[0];
			cn_errst_bak[1] = cn_errst[1];

			_di();
			Sci2Init();
			_ei();
			cnm_mem( 1 );
			CN_reset();	/* Mech Hard Reset */
			cn_init();	/* Excute Initial */
			cn_errst[0] = cn_errst_bak[0];
			cn_errst[1] = cn_errst_bak[1];

			cn_reset_count = 1;					// 初期化済みﾌﾗｸﾞｾｯﾄ
			cn_reset_reserved = 0;				// 初期化保留中OFF
			continue;
		}
// MH321800(E) T.Nagai ICクレジット対応
		tn^=0x01;
		}										/* Coin-Noteﾘｰﾀﾞｰ切替 */
// MH321800(S) G.So ICクレジット対応
//		if( prm_get(COM_PRM, S_PAY, 24, 1, 3) == 1){ 
		if (isEMoneyReader() != 0) {			// 電子マネーカードリーダー接続あり
// MH321800(E) G.So ICクレジット対応

			if( Suica_Rec.Data.BIT.EVENT ){				/* Suicaｲﾍﾞﾝﾄ処理中？ */
				if( suica_errst ){						/* ｴﾗｰ発生？ */
// MH321800(S) G.So ICクレジット対応
//					suicatrb( ERR_SUICA_COMFAIL );		/* Trouble Check */
					jvma_trb( ERR_SUICA_COMFAIL );		/* Trouble Check */
// MH810103 GG119202(S) JVMAリセット処理変更
//// MH321800(E) G.So ICクレジット対応
//// MH321800(S) 切り離しエラー発生後の待機状態でJVMAリセットを行う
//// GG116202(S) JVMAリセット回数制限対応
////				}else if( (Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) != 0 ){
//				}else if( (Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) != 0 &&	// 決済リーダ切り離しエラー発生(通信不良除く)
//							isEC_JVMA_RESET_CNT_LESS_THAN ){								// JVMAリセット回数未満
//// GG116202(E) JVMAリセット回数制限対応
//					if(	OPECTL.Ope_mod != 2 &&					// 精算中状態ではない？
//						OPECTL.Ope_mod != 3 ){					// 精算完了状態ではない？
//						// 決済リーダ切り離しエラー発生で初期化予約する
//						jvma_trb( 0 );
//						// 決済リーダ切り離しエラーのフラグをクリアする
//						Suica_Rec.suica_err_event.BYTE &= ~EC_NOT_AVAILABLE_ERR;
//					}
//// MH321800(E) 切り離しエラー発生後の待機状態でJVMAリセットを行う
// MH810103 GG119202(S) JVMAリセット処理不具合
//// GG119200(S) JVMAリセット処理変更
////				}else if( (Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) != 0 ){	// 決済リーダ切り離しエラー発生(通信不良除く)
//				}else if( (Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) != 0 &&	// 決済リーダ切り離しエラー発生(通信不良除く)
//						  (Suica_Rec.Data.BIT.INITIALIZE != 0) ){							// 初期化済み
//// GG119200(E) JVMAリセット処理変更
//					// 決済リーダ切り離しエラー発生を通知する
//					jvma_trb( ERR_SUICA_RECEIVE );
//// GG119200(E) JVMAリセット処理変更
// MH810103 GG119202(E) JVMAリセット処理不具合
				}else{
					Suica_Rec.Data.BIT.RESET_COUNT = 0; /* 初期化ｶｳﾝﾀ０ｸﾘｱ */
// MH810103 GG119202(S) JVMAリセット処理変更
//// MH321800(S) G.So ICクレジット対応
//					Suica_Rec.Data.BIT.RESET_RESERVED = 0;	// 初期化保留中OFF
//// MH321800(E) G.So ICクレジット対応
// MH810103 GG119202(E) JVMAリセット処理変更
					err_suica_chk(  &suica_errst ,&suica_errst_bak,ERR_SUICA_COMFAIL );	/* ｴﾗｰ解除 */
					suica_errst_bak = suica_errst;			/* ｴﾗｰ状態をﾊﾞｯｸｱｯﾌﾟ */
				}

// MH321800(S) Y.Tanizaki ICクレジット対応
				if( !Suica_Rec.Data.BIT.RESET_COUNT &&			// 初期化未実行？
					Suica_Rec.Data.BIT.RESET_RESERVED &&		// 初期化保留中？
// MH810103 GG119202(S) JVMAリセット処理変更
					OPECTL.Ope_mod != 1 &&						// 車室番号入力中ではない？
// MH810103 GG119202(E) JVMAリセット処理変更
					OPECTL.Ope_mod != 2 &&						// 精算中状態ではない？
					OPECTL.Ope_mod != 3 ){						// 精算完了状態ではない？
// GG116202(S) JVMAリセット回数制限対応
// MH810103 GG119202(S) JVMAリセット処理変更
//					if (!hasEC_JVMA_RESET_CNT_EXCEEDED) {		// JVMAリセットリトライ回数以下
//						Ec_Jvma_Reset_Count++;					// JVMAリセットリトライ回数インクリメント
//					}
// MH810103 GG119202(E) JVMAリセット処理変更
					if (!hasEC_JVMA_RESET_CNT_EXCEEDED) {		// JVMAリセットリトライ回数以下
// GG116202(S) JVMAリセットをエラー登録する
						err_chk( (char)jvma_setup.mdl, ERR_EC_JVMA_RESET, 2, 0, 0 );
// GG116202(E) JVMAリセットをエラー登録する
// GG116202(E) JVMAリセット回数制限対応
					_di();										// 初期化処理実行
					Sci2Init();
					_ei();
					Suica_reset();
					jvma_setup.init();
// GG116202(S) JVMAリセット回数制限対応
					}
// GG116202(E) JVMAリセット回数制限対応
					Suica_Rec.Data.BIT.RESET_COUNT = 1;			// 初期化済みﾌﾗｸﾞｾｯﾄ
					Suica_Rec.Data.BIT.RESET_RESERVED = 0;		// 初期化保留中OFF
					Suica_Rec.Data.BIT.EVENT ^= 0x01;			// Suicaｲﾍﾞﾝﾄ切り替え(ｺｲﾝﾒｯｸﾀｽｸ処理移行)
				}
// MH321800(E) Y.Tanizaki ICクレジット対応
			}
			if( Suica_Rec.Status == DATA_RCV || suica_errst ) /* ﾃﾞｰﾀ受信完了及びｴﾗｰ発生中 */
				Suica_Rec.Data.BIT.EVENT ^= 0x01;             /* ｲﾍﾞﾝﾄ切替 */
			else
				Suica_Rec.Data.BIT.EVENT = 0x01;			  /* suicaｲﾍﾞﾝﾄ継続 */

#ifdef	SUICA_EAON_DEBUG_MODE
			cnwait( 10 );	/* 100msec wait */			
#else
// MH321800(S) G.So ICクレジット対応
			if(	Suica_Rec.Data.BIT.RESET_RESERVED &&
				(OPECTL.Ope_mod == 2 ||	OPECTL.Ope_mod == 3) ) {
				// 精算中、かつ、初期化保留中は現金精算に影響があるためSuicaイベントを処理しない
				Suica_Rec.Data.BIT.EVENT = 0x00;
			}
			if(isEC_USE()) {
				// 決済リーダを使用している場合はインターバルを短くする
				cnwait( 2 );	/* 40msec wait */
			} else {
// MH321800(E) G.So ICクレジット対応
			cnwait( 5 );	/* 100msec wait */
// MH321800(S) G.So ICクレジット対応
			}
// MH321800(E) G.So ICクレジット対応
#endif
		}
		else{
			Suica_Rec.Data.BIT.EVENT = 0;			  /* suicaｲﾍﾞﾝﾄ無効 */
		cnwait( 10 );	/* 200msec wait */
		}
	}

}

/*[]----------------------------------------------------------------------[]*/
/*| Coin-mech ans Notereader Setup                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cn_init                                                 |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
cn_init( void )
{
	short	ct, tn;

// MH810103 GG119202(S) JVMAリセット処理変更
	// cn_com, nt_comをバックアップする
	cn_com_bak = cn_com;
	nt_com_bak = nt_com;
// MH810103 GG119202(E) JVMAリセット処理変更

	cnm_sts = 0;	/* Send Status */
	ntr_sts = 0;	/* Send Status */
	cn_com = 0;	/* Flg Output data to Coin Mech */
	nt_com = 0;	/* Flg Output data to Note Reader */
	cnm_ope = 0;	/* Init Coinmech & Note Reader */
	Cnm_Force_Reset = 0;

	for( ct=0,tn=0; ct<9; ct++,tn^=0x01){
		switch( ct ){
		    case 0: /* coin */
		    case 1: /* note */
				cnm_sig = 0;	/* Stand-by */
				break;
		    case 2: /* coin */
				cn_com = 0x80;
				CN_SDAT.s_dat00 &= 0xec;
				CN_SDAT.s_dat00 |= 0x06; /* CREAR,CREM OFF インベントリボタン禁止*/
				cnm_set();
				break;
		    case 3: /* note */
				nt_com = 0x80;
				NT_SDAT.s_dat10[0] = 0x22; /* 1000YEN,CREM OFF */
				NT_SDAT.s_dat10[1] = 0x00;
				ntr_set();
				break;
		    case 4: /* coin */
				memcpy( &CN_SDAT.s_dat04[0], &SFV_DAT.r_dat07[0], 12 );
				memcpy( &CN_RDAT.r_dat07[0], &SFV_DAT.r_dat07[0], 12 );
				cn_com = 0x80 + 14; /* Tube Counts Initialize */
				cnm_set();
				break;
		    case 5: /* note */
				cnm_sig = 1;	/* All req. */
				break;
		    case 6: /* coin */
				count_set( 0 );
				cn_com = 0x80 + 13; /* Change Counter Settings */
				cnm_set();
				break;
		    case 7: /* note */
				cnm_sig = 2;	/* Input req. */
				break;
		    case 8: /* coin */
				cnm_sig = 1;	/* All req. */
				break;
		    default:
				break;
		}
		cnmact( tn );
		cn_errst[tn] = cn_err[0];
		cnwait( 5 );	/* 100msec wait */
	}
	Cnm_Force_Reset = 0xff;
	cnm_ope = 1;		/* Finished Initial */
// MH810103 GG119202(S) JVMAリセット処理変更
	// バックアップしたcn_com, nt_comを戻す
	// CNMタスクで一度、処理した場合はOPEからの再要求扱いとするため、最上位ビットを立てる
	cn_com = cn_com_bak;
	if ((cn_com & 0x80) == 0) {
		cn_com |= 0x80;
	}
	nt_com = nt_com_bak;
	if ((nt_com & 0x80) == 0) {
		nt_com |= 0x80;
	}
// MH810103 GG119202(E) JVMAリセット処理変更
}

/*[]----------------------------------------------------------------------[]*/
/*| Coin-mech Command Set                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnm_set                                                 |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
cnm_set( void )
{
	short	snd_nu;
	unsigned char	c_0b, mod = 0x7f;
	unsigned char	w_s_dat00 = CN_SDAT.s_dat00;		// 最後に送信した制御データ情報をセット

	c_0b = CN_RDAT.r_dat0b;
	for( ; ; ){
		if( sfstat & 0x0004 ){	/* Out Casette ? NJ */
			sfstat &= (~0x0004);
			queset( OPETCBNO, (unsigned short)(COIN_CASSET), 0, NULL ); /* ｶｾｯﾄ着ｲﾍﾞﾝﾄ送信 */
			if((CPrmSS[S_KAN][2] == 1)&&
			  (turi_kan_f_defset_wait == TURIKAN_DEFAULT_NOWAIT)){//デフォルトセット中でない時のみ受付可能
				turi_kan_f_defset_wait = TURIKAN_DEFAULT_WAIT;//デフォルトセット待ちフラグセット
				count_set( 1 ); 			/* s03,s04ﾃﾞｰﾀｾｯﾄ 	*/
				turikan_ini(); 				/* 釣銭管理,新規設定枚数のｾｯﾄ */
				mod = 12;
				turikan_prn( MNY_COIN_CASETTE ); /* 釣銭管理印字		*/
				break;
			}
		}
		if( sfstat & 0x0008 ){ /* 保有送信後、CREM ON? */
			sfstat &= (~0x0008);
			CN_SDAT.s_dat00 = 0x47; /* CREM ON + ｸﾘｱ + ｲﾝﾍﾞﾝﾄﾘ禁止 */
			mod = 0; /* 制御ﾃﾞｰﾀ送信 */
			break;
		}
		if( sfstat & 0x0400 ){ /* CREM OFF待ち? */
			sfstat &= (~0x0400);
			queset( OPETCBNO, (unsigned short)(COIN_CNT_SND), 0, NULL ); /* 0x0535 */
			mod = 14; /* 保有枚数送信 */
			break;
		}
		if( sfstat & 0x0800 ){ /* 設定枚数送信完了待ち? */
			sfstat &= (~0x0800); /* 保有枚数送信 */
			mod = 14;
			break;
		}
		if( sfstat & 0x0100 ){ /* 入力要求送信まち? */
			if( cn_polcnt >= 2 ){
				sfstat &= (~0x0100);
				if( CN_refund == 1 ){
					SFV_SNDSTS.BIT.coin = 0;
					queset( OPETCBNO, (unsigned short)(COIN_OT_EVT), 0, NULL );
				}else{
					CN_refund &= (~0x01);
				}
				mod = 30;
				break;
			}
		}
		if( sfstat & 0x1000 ){ // 設定枚数送信完了待ち?
			sfstat &= (~0x1000);
			mod = 15;			// 釣り合わせ実行
			break;
		}
		if( cnm_sts & 0x01 ){	/* All Req. Wait */
			if( sfstat & 0x0001 ){	/* All Req. Recieved ? */
				sfstat &= (~0x0001);
				cnm_sts &= (~0x01);
			}
		}
		if( cnm_sts & 0x04 ){	/* Waitting CREM OFF & clear */
			if( c_0b & 0x20 ){
				cnm_sts &= (~0x04);
				sfstat &= (~0x00f0);
				break;
			}
		}
		if( cnm_sts & 0x10 ){	/* 払出し可状態待ち? */
			if( c_0b & 0x04 ){
				safecl( 1 ); /* 金庫入金後処理 */
				queset( OPETCBNO, (unsigned short)(COIN_EN_EVT), 0, NULL ); /* 0x0507 */
				cnm_sts &= (~0x10);
			}
		}
		if( cnm_sts & 0x20 ){	/* 払出し完了待ち? */
			if( c_0b & 0x10 ){  /* 払出し完了? */
				cnm_sts &= (~0x20);
				mod = 11;	/* Clear for RE-Refund Set */
				break;
			}
		}
		if( cnm_sts & 0x40 ){  /* 払出し可状態待ち? */
			if( c_0b & 0x04 ){
				cnm_sts &= (~0x40);
				mod = 10;	/* RE-Refund Set */
				break;
			}
		}
		if( cnm_sts & 0x80 ){ /* 払出し完了待ち? */
			if( c_0b & 0x10 ){
				SFV_SNDSTS.BIT.coin = 0;
				queset( OPETCBNO, (unsigned short)(COIN_OT_EVT), 0, NULL ); /* 0x0502 */
				cnm_sts &= (unsigned char)(~0x80);
			}
		}
		cnm_sig = 2; /* 入力要求の送信 */
		mod = cn_com;
		if( !( mod & 0x80) ) return; /* ｺﾏﾝﾄﾞ送信要求? NR(入力要求送信) */
		cn_com &= 0x7f;
		break;
	}
	cnm_sts = 0x00;
	switch( mod & 0x7f ){
		case 0: /* Control */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			break;
		case 1: /* CREM ON */
			csd_buf[0] = 13;
			csd_buf[1] = 4;
			count_set( 10 );	/* s_dat04 = all0 */
			memcpy( &csd_buf[2], CN_SDAT.s_dat04, 12 );
			snd_nu = 14;
			sfstat &= 0x0004; /* ここでｸﾘｱしておく */
			sfstat |= 0x0008; /* 保有枚数送信後にCREM ONを送信する */
			safecl( 0 ); /* 金庫入金前処理 */
			// safeclで0ｸﾘｱするので不要
			//SFV_DAT.safe_inout = 0; /* 払出しなしである(金庫枚数算出用) */
			break;
		case 2: /* CREM OFF */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xec;	/* CREM OFF,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			cnm_sts = 0x10;	/* 払出し可状態待ち */
			break;
		case 3: /* CREM ON （料金表示でﾀｲﾑｱｳﾄ直前に入金があった場合に入金可にする）*/
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xe8;
			CN_SDAT.s_dat00 |= 0x05;	/* CREM ON, ｲﾝﾍﾞﾝﾄﾘ禁止 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			break;
		case 4: /* CREM OFF */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xe8;	/* CREM OFF, ｲﾝﾍﾞﾝﾄﾘ可  */
			CN_SDAT.s_dat00 |= 0x06;	/* Clear,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			SFV_DAT.cn_escrow = 0; /* ｴｽｸﾛ枚数ｸﾘｱ */
			break;
		case 5: /* CREM ON,Clear(入金継続) */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xe8;
			CN_SDAT.s_dat00 |= 0x07;	/* CREM ON,clear, ｲﾝﾍﾞﾝﾄﾘ禁止 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			sfstat &= (~0x00f0 );
			SFV_DAT.cn_escrow = 0; /* ｴｽｸﾛ枚数ｸﾘｱ */
			break;
		case 6: /* CREM OFF,Clear（精算終了でｴｽｸﾛｳｴｲﾄ） */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xe8;	/* CREM OFF, ｲﾝﾍﾞﾝﾄﾘ禁止 */
			CN_SDAT.s_dat00 |= 0x06;	/* Clear,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			set_escrowtimer();
			SFV_DAT.safe_inout = 0; /* 払出しなしである */
			SFV_DAT.cn_escrow = 0; /* ｴｽｸﾛ枚数ｸﾘｱ */
			break;
		case 7:
			cnm_sig = 1;	/* Set All req. */
			cn_com &= 0x7f;
			return;
		case 8: /* CREM OFF */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xec;	/* CREM OFF,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			count_set( 21 );	/* s_dat04に最大枚数をｾｯﾄ */
			sfstat |= 0x0400; /* 送信後に保有枚数を送信する */
			break;
		case 9:  /* 払出し前の保有枚数送信 */
			csd_buf[0] = 13;
			csd_buf[1] = 4;
			count_set( 11 );	/* s_dat04に最大枚数をｾｯﾄ */
			memcpy( &csd_buf[2], CN_SDAT.s_dat04, 12 );
			snd_nu = 14;
			sfstat |= 0x0002; /* 保有枚数送信後に払出し指示を送信する */
			SFV_DAT.safe_inout = 1; /* 払出し後である(金庫枚数算出用) */
			break;
		case 10: /* Refund */
			switch( refcal()){ /* Refund Amount ZERO ? NJ */
			default:
			case 0:	
				cnm_sts = 0x00;
				cn_com = 0;

				sfstat |= 0x0100;			/* 払出し完了後、入力要求送信待ち */
				cn_polcnt = 0;
				cnm_sig = 2; 				/* 入力要求の送信 */
				return;
			case 1:						// 金種毎に枚数指定して払い出し
				csd_buf[0] = 5;
				csd_buf[1] = 2;
				memcpy( &csd_buf[2], CN_SDAT.s_dat02, 4 );
				snd_nu = 6;
				cnm_sts = 0x20; /* Set Waitting Refund finished */
				CN_RDAT.r_dat0b &= (~0x10); 
					/* 追加払出しのとき、この時点では払出し終了ﾋﾞｯﾄONのままなのでOFFする */
				break;
			case -1:
				csd_buf[0] = 4;
				csd_buf[1] = 1;
				memcpy( &csd_buf[2], CN_SDAT.s_dat01, 3 );
				snd_nu = 5;
				cnm_sts = 0x20; /* Set Waitting Refund finished */
				CN_RDAT.r_dat0b &= (~0x10); 
					/* 追加払出しのとき、この時点では払出し終了ﾋﾞｯﾄONのままなのでOFFする */
				break;
			}
			break;
		case 11: /* CREM OFF（追加払出し） */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xec;	/* CREM OFF,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			cnm_sts = 0x40;
			break;
		case 12: /* Change Counter Settings */
			csd_buf[0] = 13;
			csd_buf[1] = 3;
			memcpy( &csd_buf[2], CN_SDAT.s_dat03, 12 );
			snd_nu = 14;
			sfstat |= 0x0800;
			break;
		case 13: /* Change Counter Settings */
			csd_buf[0] = 13;
			csd_buf[1] = 3;
			memcpy( &csd_buf[2], CN_SDAT.s_dat03, 12 );
			snd_nu = 14;
			break;
		case 14: /* 保有枚数送信 */
			csd_buf[0] = 13;
			csd_buf[1] = 4;
			memcpy( &csd_buf[2], CN_SDAT.s_dat04, 12 );
			snd_nu = 14;
			break;
		case 15: /* Issue Auto Change Setting */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xfd; /* Clear Flag off */
			CN_SDAT.s_dat00 |= 0x50; /* Set Flag */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			cnm_sts = 0x80; /* Set Waitting Refund finished */
			break;
		case 21: /* CREM ON */
			csd_buf[0] = 13;
			csd_buf[1] = 4;
			count_set( 21 );	/* s_dat04 = all0 */
			memcpy( &csd_buf[2], CN_SDAT.s_dat04, 12 );
			snd_nu = 14;
			sfstat &= 0x0004; /* ここでｸﾘｱしておく */
			sfstat |= 0x0008; /* 保有枚数送信後にCREM ONを送信する */
			break;
		case 22: /* CREM OFF, Clear */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xe8;	/* CREM OFF,ｲﾝﾍﾞﾝﾄﾘ禁止 */
			CN_SDAT.s_dat00 |= 0x06;	/* Clear,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			SFV_DAT.cn_escrow = 0; /* ｴｽｸﾛ枚数ｸﾘｱ */
			break;
		case 24: /* CREM OFF */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xe8;	/* CREM OFF */
			CN_SDAT.s_dat00 |= 0x04;	/* ｲﾝﾍﾞﾝﾄﾘ禁止 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			break;
		case 30: /* 払出し完了後、入力要求を送信したところ */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xe8;	/* CREM OFF, ｲﾝﾍﾞﾝﾄﾘ不可 */
			CN_SDAT.s_dat00 |= 0x06;	/* Clear,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			if(( turi_dat.turi_in == 0 )&&( OPECTL.Mnt_mod != 3 )){		// 釣銭管理中でなく、ｼｽﾃﾑﾒﾝﾃ中でない
				set_escrowtimer();
			}
			SFV_DAT.cn_escrow = 0; /* ｴｽｸﾛ枚数ｸﾘｱ */
			break;
		case 32: /* CREM OFF */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xec; /* CREM OFF,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			count_set( 32 );	/* s_dat04に最大枚数をｾｯﾄ */
			sfstat |= 0x0400; /* 送信後に保有枚数を送信する */
			break;
		case 33: /* CREM ON, Clear */
			csd_buf[0] = 2;
			csd_buf[1] = 0;
			CN_SDAT.s_dat00 &= 0xe8;	/* CREM ON,ｲﾝﾍﾞﾝﾄﾘ可 */
			CN_SDAT.s_dat00 |= 0x07;	/* Clear ,ｲﾝﾍﾞﾝﾄﾘ不可 */
			csd_buf[2] = CN_SDAT.s_dat00;
			snd_nu = 3;
			break;
		case 34: // 設定枚数送信後に釣り合わせを実行する
			csd_buf[0] = 13;
			csd_buf[1] = 3;
			count_set_sub( CN_SDAT.s_dat03 );	/* s_dat03に初期枚数をｾｯﾄ */
			memcpy( &csd_buf[2], CN_SDAT.s_dat03, 12 );
			snd_nu = 14;
			sfstat |= 0x1000;
			break;
		default:
			cnm_sig = 2;
			cnm_sts = 0x00;
			cn_com = 0;
			return;
			break;
	}
	cnm_sig = 3;
	csd_buf[snd_nu] = bcccal( (char*)csd_buf, snd_nu );
	csd_siz = snd_nu+1;	/* Set SEND DATA SIZE */
	
	if( (( CN_RDAT.r_dat0b & 0x01 ) == 1 || 					// 最後に受信したメックの状態がCREM ONか、
		( w_s_dat00 & 0x01 ) == 1 ) &&							// 最後に送信した制御データがCREM ONの場合、
		(( CN_SDAT.s_dat00 & 0x01 ) == 0 && snd_nu == 3 )){		// CREM OFF且つ制御データ送信時(送信データサイズで判定)
		CN_RDAT.r_dat0b &= 0xFB;								// 状態データの釣銭払い出し可能ビットをクリアする
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| Note-Reader Command Set                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntr_set                                                 |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
ntr_set( void )
{
	short	snd_nu;
	unsigned char	mod;

	for( ; ; ){
		if( ntr_sts & 0x10 ){	/* Waitting Available Return ? */
			if( NT_RDAT.r_dat1b[0] & 0x04 ){
				queset( OPETCBNO, (unsigned short)(NOTE_EN_EVT), 0, NULL ); /* 0x0807 */
				ntr_sts = 0x00;
			}
		}
		if( ntr_sts & 0x20 ){	/* Waitting Return finished ? */
			if( NT_RDAT.r_dat1b[0] & 0x08 ){
				if( OPECTL.Ope_Mnt_flg != 4 && OPECTL.Mnt_mod != 5){	/* ｼｽﾃﾑﾒﾝﾃﾅﾝｽ-動作ﾁｪｯｸ-Noteﾁｪｯｸ */
					if( ryo_buf.in_coin[4] ) ryo_buf.in_coin[4]--;
					if( SFV_DAT.nt_safe_dt ) SFV_DAT.nt_safe_dt--; // 紙幣金庫合計+1
				}
				SFV_SNDSTS.BIT.note = 0;
				SFV_DAT.nt_escrow = 0;	// 紙幣ｴｽｸﾛで中止し電源offonすると金庫ｶｳﾝﾄしてしまう対策
				PayData.f_escrow = 1;	// 1000円札のエスクロ有フラグセット
				// NOTE：このフラグはryo_cal()の最初でPayDataごとクリアされる
				NTNET_Data152Save((void *)(&PayData.f_escrow), NTNET_152_FESCROW);
				if( OPECTL.Ope_Mnt_flg == 4 || OPECTL.Mnt_mod == 5){	// 紙幣ﾘｰﾀﾞｰﾁｪｯｸ中
					queset( OPETCBNO, (unsigned short)(NOTE_OT_EVT), 0, NULL ); /* 0x0802 */
				}
				if( CN_refund == 2 ){
					queset( OPETCBNO, (unsigned short)(COIN_OT_EVT), 0, NULL ); /* 0x0502 */
				}else{
					CN_refund &= (~0x02);
				}
				ntr_sts = 0x00;
			}
		}
		if( ntr_sts & 0x40 ){	/* Waitting End of Get Note ? */
			if( NT_RDAT.r_dat1b[0] & 0x40 ){
				ntr_sts = 0x80;
			}
		}
		if( ntr_sts & 0x80 ){	/* Waitting End of Get Note ? */
			if( (NT_RDAT.r_dat1b[0] & 0x40)==0 ){
				SFV_DAT.nt_escrow = 0;
				queset( OPETCBNO, (unsigned short)(NOTE_IM_EVT), 0, NULL );
				ntr_sts = 0x00;
			}
		}
		cnm_sig = 2;
		mod = nt_com;
		if( !(mod & 0x80) ) return;
		nt_com &= 0x7f;
		break;
	}
	ntr_sts = 0x00;
	switch( mod & 0x7f ){
		case 0:
			csd_buf[0] = 3;
			csd_buf[1] = 0x10;
			csd_buf[2] = NT_SDAT.s_dat10[0];
			csd_buf[3] = NT_SDAT.s_dat10[1];
			snd_nu = 4;
			break;
		case 1: /* CREM ON,clear */
			csd_buf[0] = 3;
			csd_buf[1] = 0x10;
			csd_buf[2] = 0x23; /* CREM ON, Clear */
			csd_buf[3] = 0x00;
			snd_nu = 4;
			if( NT_RDAT.r_dat18[0] != 0 ){	/* In Escrow ? */
				ntr_sts = 0x40;
			}
			break;
		case 2: /* CREM OFF */
			csd_buf[0] = 3;
			csd_buf[1] = 0x10;
			csd_buf[2] = 0x20; /* CREM OFF */
			csd_buf[3] = 0x00;
			snd_nu = 4;
			ntr_sts = 0x10;
			break;
		case 3: /* CREM ON */
			csd_buf[0] = 3;
			csd_buf[1] = 0x10;
			csd_buf[2] = 0x21; /* CREM ON */
			csd_buf[3] = 0x00;
			snd_nu = 4;
			break;
		case 4: /* CREM OFF,Clear */
			csd_buf[0] = 3;
			csd_buf[1] = 0x10;
			csd_buf[2] = 0x22; /* CREM OFF */
			csd_buf[3] = 0x00;
			snd_nu = 4;
			if( NT_RDAT.r_dat18[0] != 0 ){	/* In Escrow ? */
				ntr_sts = 0x40;
			}
			break;
		case 10: /* Return Note */
			csd_buf[0] = 5;
			csd_buf[1] = 0x11;
			csd_buf[2] = 0x00;
			csd_buf[3] = 0x10;
			csd_buf[4] = 0x00;
			csd_buf[5] = 0x00;
			snd_nu = 6;
			ntr_sts = 0x20;
			break;
		case 7:
			cnm_sig = 1;	/* Set All req. */
			nt_com &= 0x7f;
			return;
		default:
			cnm_sig = 2;
			ntr_sts = 0x00;
			nt_com = 0;
			return;
			break;
	}
	cnm_sig = 3;
	csd_buf[snd_nu] = bcccal( (char*)csd_buf, snd_nu );
	csd_siz = snd_nu+1;	/* Set SEND DATA SIZE */
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 金庫計算ﾃﾞｰﾀ初期化、ｺｲﾝﾒｯｸへ送信する保有枚数をｾｯﾄする                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : count_set                                               |*/
/*| PARAMETER	 : prm = 0:入金前, 1:入金完了でｸﾘｱ前, 2:ｶｾｯﾄ着時           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void count_set( short prm )
{
	short	i;
	short nu, max;

	switch( prm ){

		case 0: /* 電源投入時の初期化 */
			Tubu_cnt_set = 0;
			memcpy( CN_SDAT.s_dat03, CN_RDAT.r_dat06, sizeof( CN_SDAT.s_dat03 ) );
			memcpy( CN_SDAT.s_dat04, CN_RDAT.r_dat07, sizeof( CN_SDAT.s_dat04 ) );
			break;
		case 1: /* ｶｾｯﾄ着時の初期化 */
			if( CPrmSS[S_KAN][2] == 0 ){
				Tubu_cnt_set = 0;
				memcpy( CN_SDAT.s_dat03, CN_RDAT.r_dat06, sizeof( CN_SDAT.s_dat03 ) );
				memcpy( CN_SDAT.s_dat04, CN_RDAT.r_dat07, sizeof( CN_SDAT.s_dat04 ) );
			}else{
				Tubu_cnt_set = -1;
				count_set_sub( CN_SDAT.s_dat03 ); // ﾃﾞﾌｫﾙﾄ枚数をｾｯﾄする
				memcpy( CN_SDAT.s_dat04, CN_SDAT.s_dat03, sizeof( CN_SDAT.s_dat04 ) );
			}
			memcpy( CN_RDAT.r_dat07, CN_SDAT.s_dat04, sizeof( CN_RDAT.r_dat07 ) );
			break;
 		case 10: /* 入金前の初期化 */
			memset( CN_RDAT.r_dat08, 0, sizeof( CN_RDAT.r_dat08 ) );
			memset( CN_SDAT.s_dat04, 0, sizeof( CN_SDAT.s_dat04 ) );
			memcpy( CN_SDAT.s_dat04+4, CN_RDAT.r_dat06+4, 8 );
			memcpy( CN_RDAT.r_dat07, CN_SDAT.s_dat04, sizeof( CN_RDAT.r_dat07 ) );
			break;
		case 11: /* 払出前の初期化 */
			memcpy( CN_SDAT.s_dat04, CN_RDAT.r_dat06, sizeof( CN_SDAT.s_dat04 ) );
			memcpy( CN_SDAT.s_dat04, sv_coin_max, 4 );
			memcpy( CN_RDAT.r_dat07, CN_SDAT.s_dat04, sizeof( CN_RDAT.r_dat07 ) );
			memset( CN_SDAT.s_dat02, 0, sizeof( CN_SDAT.s_dat04 ) );
			break;
		case 21: /* 釣銭補充で入金前の初期処理 、待機状態での保有枚数送信 */
				/* s04へ現在保有枚数をｾｯﾄする */
			if( CPrmSS[S_KAN][1] == 0 ){
				memcpy( CN_SDAT.s_dat04, CN_RDAT.r_dat06, sizeof( CN_SDAT.s_dat04 ) );
			}else{
				for( i = 0; i < 4; i++ ){
					CN_SDAT.s_dat04[i] = binbcd( (unsigned char)turi_kan.turi_dat[i].gen_mai );
				}
				nu = (short)( turi_kan.turi_dat[0].ygen_mai % 1000 );
				max = (short)bcdbin( CN_RDAT.r_dat06[4] );
				max += (short)( bcdbin( CN_RDAT.r_dat06[5] ) * 100 );
				if( nu > max )	nu = max;
				CN_SDAT.s_dat04[4] = binbcd( (unsigned char)(nu % 100));	/* 10円補助筒保有枚数	*/
				CN_SDAT.s_dat04[5] = binbcd( (unsigned char)(nu / 100));	/* 10円補助筒保有枚数	*/
	
				nu = (short)( turi_kan.turi_dat[1].ygen_mai % 1000 );
				max = (short)bcdbin( CN_RDAT.r_dat06[6] );
				max += (short)( bcdbin( CN_RDAT.r_dat06[7] ) * 100 );
				if( nu > max ) nu = max;
				CN_SDAT.s_dat04[6] = binbcd( (unsigned char)(nu % 100));	/* 50円補助筒保有枚数	*/
				CN_SDAT.s_dat04[7] = binbcd( (unsigned char)(nu / 100));	/* 50円補助筒保有枚数	*/

				nu = (short)( turi_kan.turi_dat[2].ygen_mai % 1000 );
				max = (short)bcdbin( CN_RDAT.r_dat06[8] );
				max += (short)( bcdbin( CN_RDAT.r_dat06[9] ) * 100 );
				if( nu > max ) nu = max;
				CN_SDAT.s_dat04[8] = binbcd( (unsigned char)(nu % 100));	/* 100円補助筒保有枚数	*/
				CN_SDAT.s_dat04[9] = binbcd( (unsigned char)(nu / 100));	/* 100円補助筒保有枚数	*/
			}
			memcpy( CN_RDAT.r_dat07, CN_SDAT.s_dat04, sizeof( CN_RDAT.r_dat07 ) );
			SFV_DAT.cn_escrow = 0;
			break;
		case 32: // 現在枚数を変更したときに保有枚数を送信する
				/* s04へ現在保有枚数をｾｯﾄする */
			if( CPrmSS[S_KAN][1] == 0 ){
				memcpy( CN_SDAT.s_dat04, CN_RDAT.r_dat06, sizeof( CN_SDAT.s_dat04 ) );
			}else{
				for( i = 0; i < 4; i++ ){
					CN_SDAT.s_dat04[i] = binbcd( (unsigned char)turi_dat.coin_sin[i] );
				}
				nu = (short)( turi_dat.coin_sin[4] % 1000 );
				max = (short)bcdbin( CN_RDAT.r_dat06[4] );
				max += (short)( bcdbin( CN_RDAT.r_dat06[5] ) * 100 );
				if( nu > max )	nu = max;
				CN_SDAT.s_dat04[4] = binbcd( (unsigned char)(nu % 100));	/* 10円補助筒保有枚数	*/
				CN_SDAT.s_dat04[5] = binbcd( (unsigned char)(nu / 100));	/* 10円補助筒保有枚数	*/
	

				// 50円をturi_dat.coin_sin[5]、100円をturi_dat.coin_sin[6]に変更する
				nu = (short)( turi_dat.coin_sin[5] % 1000 );
				max = (short)bcdbin( CN_RDAT.r_dat06[6] );
				max += (short)( bcdbin( CN_RDAT.r_dat06[7] ) * 100 );
				if( nu > max ) nu = max;
				CN_SDAT.s_dat04[6] = binbcd( (unsigned char)(nu % 100));	/* 50円補助筒保有枚数	*/
				CN_SDAT.s_dat04[7] = binbcd( (unsigned char)(nu / 100));	/* 50円補助筒保有枚数	*/
				
				nu = (short)( turi_dat.coin_sin[6] % 1000 );
				max = (short)bcdbin( CN_RDAT.r_dat06[8] );
				max += (short)( bcdbin( CN_RDAT.r_dat06[9] ) * 100 );
				if( nu > max ) nu = max;
				CN_SDAT.s_dat04[8] = binbcd( (unsigned char)(nu % 100));	/* 100円補助筒保有枚数	*/
				CN_SDAT.s_dat04[9] = binbcd( (unsigned char)(nu / 100));	/* 100円補助筒保有枚数	*/
			}
			memcpy( CN_RDAT.r_dat07, CN_SDAT.s_dat04, sizeof( CN_RDAT.r_dat07 ) );
			SFV_DAT.cn_escrow = 0;
			break;
		default:
			break;
	}
}

static void
count_set_sub( wk )
unsigned char *wk;
{
// MH321800(S) Y.Tanizaki ICクレジット対応(共通改善No.1445/全ての初期設定枚数が0枚の場合に釣合せ実行でロックしたように見える)
//	unsigned char nu, max;
//
//	nu = (uchar)prm_get( 0,S_KAN,3,3,1 );
//	if( nu > TUB_MAX10 ) nu = TUB_MAX10;
//	*wk = binbcd( nu );							// 10円保有枚数
//
//	nu = (uchar)prm_get( 0,S_KAN,9,3,1 );
//	if( nu > TUB_MAX50 ) nu = TUB_MAX50;
//	*(wk+1) = binbcd( nu );						// 10円保有枚数
//
//	nu = (uchar)prm_get( 0,S_KAN,15,3,1 );
//	if( nu > TUB_MAX100 ) nu = TUB_MAX100;
//	*(wk+2) = binbcd( nu );						// 10円保有枚数
//
//	nu = (uchar)prm_get( 0,S_KAN,21,3,1 );
//	if( nu > TUB_MAX500 ) nu = TUB_MAX500;
//	*(wk+3) = binbcd( nu );						// 10円保有枚数
//
//	nu = (uchar)prm_get( 0,S_KAN,27,3,1 );
//	max = bcdbin( CN_RDAT.r_dat06[4] );
//	max += (unsigned char)( bcdbin( CN_RDAT.r_dat06[5] ) * 100 );
//	if( nu > max )	nu = max;
//	*(wk+4) = binbcd( (unsigned char)(nu % 100));	// 10円補助筒保有枚数
//	*(wk+5) = binbcd( (unsigned char)(nu / 100));	// 10円補助筒保有枚数
//
//	nu = (uchar)prm_get( COM_PRM,S_KAN,30,3,1 );	// 50円は予蓄２のみ
//	max = bcdbin( CN_RDAT.r_dat06[6] );
//	max += (unsigned char)( bcdbin( CN_RDAT.r_dat06[7] ) * 100 );
//	if( nu > max )	nu = max;
//	*(wk+6) = binbcd( (unsigned char)(nu % 100));	// 50円補助筒保有枚数
//	*(wk+7) = binbcd( (unsigned char)(nu / 100));	// 50円補助筒保有枚数
//
//	nu = (uchar)prm_get( 0,S_KAN,33,3,1 );
//	max = bcdbin( CN_RDAT.r_dat06[8] );
//	max += (unsigned char)( bcdbin( CN_RDAT.r_dat06[9] ) * 100 );
//	if( nu > max )	nu = max;
//	*(wk+8) = binbcd( (unsigned char)(nu % 100));	// 100円補助筒保有枚数
//	*(wk+9) = binbcd( (unsigned char)(nu / 100));	// 100円補助筒保有枚数
	unsigned char nu, max;
	unsigned char all_zero = 1;

	nu = (uchar)prm_get( 0,S_KAN,3,3,1 );
	if( nu > TUB_MAX10 ) nu = TUB_MAX10;
	if( nu != 0 ) all_zero = 0;
	*wk = binbcd( nu );							// 10円保有枚数

	nu = (uchar)prm_get( 0,S_KAN,9,3,1 );
	if( nu > TUB_MAX50 ) nu = TUB_MAX50;
	if( nu != 0 ) all_zero = 0;
	*(wk+1) = binbcd( nu );						// 50円保有枚数

	nu = (uchar)prm_get( 0,S_KAN,15,3,1 );
	if( nu > TUB_MAX100 ) nu = TUB_MAX100;
	if( nu != 0 ) all_zero = 0;
	*(wk+2) = binbcd( nu );						// 100円保有枚数

	nu = (uchar)prm_get( 0,S_KAN,21,3,1 );
	if( nu > TUB_MAX500 ) nu = TUB_MAX500;
	if( nu != 0 ) all_zero = 0;
	*(wk+3) = binbcd( nu );						// 500円保有枚数

	nu = (uchar)prm_get( 0,S_KAN,27,3,1 );
	max = bcdbin( CN_RDAT.r_dat06[4] );
	max += (unsigned char)( bcdbin( CN_RDAT.r_dat06[5] ) * 100 );
	if( nu > max )	nu = max;
	if( nu == 0 && all_zero != 0) nu = 10;			// 全て0の場合ロックするためダミーとして10枚を設定
	*(wk+4) = binbcd( (unsigned char)(nu % 100));	// 10円補助筒保有枚数
	*(wk+5) = binbcd( (unsigned char)(nu / 100));	// 10円補助筒保有枚数

	nu = (uchar)prm_get( COM_PRM,S_KAN,30,3,1 );	// 50円は予蓄２のみ
	max = bcdbin( CN_RDAT.r_dat06[6] );
	max += (unsigned char)( bcdbin( CN_RDAT.r_dat06[7] ) * 100 );
	if( nu > max )	nu = max;
	if( nu == 0 && all_zero != 0) nu = 10;			// 全て0の場合ロックするためダミーとして10枚を設定
	*(wk+6) = binbcd( (unsigned char)(nu % 100));	// 50円補助筒保有枚数
	*(wk+7) = binbcd( (unsigned char)(nu / 100));	// 50円補助筒保有枚数

	nu = (uchar)prm_get( 0,S_KAN,33,3,1 );
	max = bcdbin( CN_RDAT.r_dat06[8] );
	max += (unsigned char)( bcdbin( CN_RDAT.r_dat06[9] ) * 100 );
	if( nu > max )	nu = max;
	if( nu == 0 && all_zero != 0) nu = 10;			// 全て0の場合ロックするためダミーとして10枚を設定
	*(wk+8) = binbcd( (unsigned char)(nu % 100));	// 100円補助筒保有枚数
	*(wk+9) = binbcd( (unsigned char)(nu / 100));	// 100円補助筒保有枚数
// MH321800(E) Y.Tanizaki ICクレジット対応(共通改善No.1445/全ての初期設定枚数が0枚の場合に釣合せ実行でロックしたように見える)
}

/*[]----------------------------------------------------------------------[]*/
/*| Coin-mech and Note-reader Trouble Check	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnmtrb( tn )                                            |*/
/*| PARAMETER	 : short	tn	| 0:Coin-Mech 1:Note-Reader                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static void
cnmtrb( short tn )
{
	if( cn_errst[tn] == 0 ){ /* Already Trouble? YJ */
		switch( tn ){
		    case 0:
				if( cnm_sts & (0x20 | 0x80 )){	/* 払い出し中 又は 釣り合わせ中? */
					queset( OPETCBNO, (unsigned short)(COIN_ER_EVT), 0, NULL ); /* 0x0509 */
					cnm_sts = 0x00;
					cn_stat( 30, 0 );
				}
				break;
		    case 1:
				if( ntr_sts & 0x03 ){	/* Run Return? */
					queset( OPETCBNO, (unsigned short)(NOTE_EM_EVT), 0, NULL ); /* 0x0809 */
					ntr_sts = 0x00;
				}else if( ntr_sts & 0xc0 ){ /* Run Get? */
					queset( OPETCBNO, (unsigned short)(NOTE_ER_EVT), 0, NULL ); /* 0x080A */
					ntr_sts = 0x00;
					nt_com = 0;
				}
			break;
		    	default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Coin-mech and Note-reader Communication Main                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnmact( tn )                                            |*/
/*| PARAMETER	 : short	tn	| 0:Coin-Mech 1:Note-Reader                |*/
/*| RETURN VALUE : short		| 1:OK	<>1:Error                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short
cnmact(
short	tn )
{
	short	rtn;

	tn_save = tn;
	rtn = 0;
								/* cn_err[0]-[4] Clear */
	cn_err[0] =	cn_err[1] =	cn_err[2] = cn_err[3] = 0;
	cn_faze = 1;
	for( ; rtn==0 ; ){
		switch( cn_faze ){
			case 1:		/* Newtral */
				rtn = cnm_01( tn );
				break;
			case 2:		/* Wait after Input Command */
				rtn = cnm_02();
				break;
			case 3:		/* Wait after Output Command */
				rtn = cnm_03();
				break;
			case 4:		/* Wait after Data Send */
				rtn = cnm_04();
				break;
			default:
				break;
		}
		switch( rtn ) {
			case 0:	 /* Continue */
				continue;
				break;
			case 4:  /* ACK4 */
				break;
			case -9: /* Send Time Out */
				cn_err[0] |= 0x80;
				break;
			case -1: /* Receive Time Out */
				if( cn_errst[tn]&0x40 ){ /* Already Timeout? NJ */
					cn_err[0] |= 0x40;
					break;
				}
				cn_err[1]++;	/* Retry Counter +1 */
				if( cn_err[1] <= T_RTRY ){
					cn_faze = 1;
					rtn = 0;
					wait2us( 2500L ); /* 5ms wait */
				}else{
					cn_err[0] |= 0x40;
				}
				break;
			default:
				break;
		}
	}
	if( tn ){	/* Note Reader ?  NJ */
		if( (char)(NT_RDAT.r_dat1c[0] & 0x03) == 0x01 ){
			cn_err[0] |= 0x01;	/* Set Trouble */
		}
	}else{		/* Coin Mech */
		if( (char)(CN_RDAT.r_dat0c[0] & 0x03) == 0x01 ){
			cn_err[0] |= 0x01;	/* Set Trouble */
		}
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 1...Send Command                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnm_01( tn )                                            |*/
/*| PARAMETER	 : short tn	| 0:Coin-Mech 1:Note-Reader                    |*/
/*| RETURN VALUE : short 	| 0:Continue -9:Send Timeout                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static short
cnm_01(short tn )
{
	short	sig;
	const unsigned char	*scp;

	sig = cnm_sig;
	if( tn ){	/* Note Reader? yes */
		scp = &cnt_tbl[sig][0];
	} else {	/* Coin Mech? yes */
		scp = &com_tbl[sig][0];
	}
	CN_RCLR();					// 受信バッファクリア
	if( CNM_CMD( scp, sig ) ) {	/* Send COMMAND */
		return( -9 );		/* TIME OUT */
	}
	switch( sig ){
		case 2:
			if( sfstat & 0x0100 ){ /* 入力要求送信まち? */
				cn_polcnt++;
			}
		case 0:
		case 1:
		case 4:
			cn_faze = 2;
			break;
		case 3:
		case 5:
			cn_faze = 3;
			break;
		default:
			break;
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 2...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnm_02                                                  |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short    | 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static short
cnm_02( void )
{
	short	rtn;

	if( (rtn = CNM_RCV( crd_buf, &crd_siz )) != 0 ){
		cnm_sig = 4;//入力再要求
		cn_faze = 1;//cnm_01()へ
		return( rtn );
	}
	switch( rtn = cnm_recv( crd_buf, crd_siz ) ){
		case 1: /* ACK1 */
		case 2: /* ACK2 */
		case 3: /* ACK3 */
			if( tn_save ){
				err_chk( ERRMDL_NOTE, ERR_NOTE_COMFAIL, 0, 0, 0 ); /* ｴﾗｰﾛｸﾞ登録（解除）*/
			}else{
				err_chk( ERRMDL_COIM, ERR_COIN_COMFAIL, 0, 0, 0 ); /* ｴﾗｰﾛｸﾞ登録（解除） */
			}
			if( cnm_sig == 0 )
				cnm_ctrl_info.BIT.standby_snd_after = 1;
		case 6: /* NAK */
			rtn = 1;
			break;
		case 4: /* ACK4 */
			break;
		case 5: /* ACK5 */
		case 7: /* ETC. */
		case 9:	/* LRC ERROR */
			cn_err[3]++;
			if( cn_err[3] < R_RTRY ){
				cnm_sig = 4;
				cn_faze = 1;
				rtn = 0;	/* Retry */
			}else{
				cn_err[0] |= 0x10;
				rtn = 1;
			}
			break;
		default:
			break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 3...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnm_03                                                  |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short	| 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static short
cnm_03( void )
{
	short	rtn;

	if( (rtn = CNM_RCV( crd_buf, &crd_siz )) != 0 ){
		return( rtn );
	}
	switch( rtn = cnm_recv( crd_buf, crd_siz ) ){
		case 1: /* ACK1 */
			if( CNM_SND(csd_buf,csd_siz) == 0 ){ /* SEND DATA */
				cn_faze = 4;
				rtn = 0;
				if(csd_buf[1] == 0x11){
					SFV_SNDSTS.BIT.note = 1;		// 紙幣払出し要求
				}
				else if(csd_buf[1] == 0x01){
					SFV_SNDSTS.BIT.coin = 1;
				}
				break;
			}
		case 2: /* ACK2 */
		case 3: /* ACK3 */
		case 7: /* ETC. */
		case 6: /* NAK */
			cn_err[2]++;
			if( cn_err[2] < S_RTRY ){
				cn_faze = 1;
				rtn = 0;
				wait2us( 2500L ); /* 5ms wait */
			}else{
				cn_err[0] |= 0x08;
				rtn = 1;
			}
			break;
		case 4: /* ACK4 */
			break;
		case 5: /* ACK5 */
			rtn = 1;	/* OK */
			break;
		default:
			break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 4...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnm_04                                                  |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short | 0:Continue 1:OK ETC:Error                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static short
cnm_04( void )
{
	short	rtn;

	if( (rtn = CNM_RCV( crd_buf, &crd_siz )) != 0 ){
		cn_err[2]++;
		if( cn_err[2] < S_RTRY ){
			cnm_sig = 5;
			cn_faze = 1;
			rtn = 0;
		}else{
			cn_err[0] |= 0x08;
			rtn = 1;
		}
		return( rtn );
	}
	switch( rtn = cnm_recv( crd_buf, crd_siz ) ){
		case 1: /* ACK1 */
			rtn = 1; /* OK */
			if( sfstat & 0x0008 ){ /* 保有枚数送信待ち? */
				cnm_set();
				cn_faze = 1;
				rtn = 0;
			}
			if( sfstat & 0x0002 ){ /* 保有枚数送信完了待ち? */
				sfstat &= (~0x0002);
				cn_com = 0x80 + 10;
				cnm_set();		/* 払出しｺﾏﾝﾄﾞ編集 */
				cn_faze = 1;
				rtn = 0;
			}
			if( sfstat & 0x0400 ){ /* CREM OFF送信待ち? */
				cnm_set(); /* 保有枚数送信 */
				cnm_sig = 3;
				cn_faze = 1;
				rtn = 0;
				break;
			}
			if( sfstat & 0x0800 ){ /* 設定枚数送信完了待ち? */
				cnm_set(); /* 保有枚数送信 */
				cnm_sig = 3;
				cn_faze = 1;
				rtn = 0;
				break;
			}
			if( sfstat & 0x1000 ){ // 設定枚数送信完了待ち?
				cnm_set(); // 釣り合わせ実行
				cnm_sig = 3;
				cn_faze = 1;
				rtn = 0;
				break;
			}
			break;
		case 2: /* ACK2 */
		case 3: /* ACK3 */
		case 5: /* ACK5 */
		case 7: /* ACK5 */
			cn_err[2]++;
			if( cn_err[2] < S_RTRY ){
				cnm_sig = 5;
				cn_faze = 1;
				rtn = 0;	/* Retry */
			}else{
				cn_err[0] |= 0x08;
				rtn = 1;
			}
			break;
		case 6: /* NAK */
			if( tn_save ){
				ex_errlg( ERRMDL_NOTE, ERR_NOTE_NAKRECEIVE, 2, 0 ); /* ｴﾗｰﾛｸﾞ登録(NAK受信) */
			}else{
				ex_errlg( ERRMDL_COIM, ERR_COIN_NAKRECEIVE, 2, 0 ); /* ｴﾗｰﾛｸﾞ登録(NAK受信) */
			}
			rtn = 1;
			break;
		case 4: /* ACK4 */
			break;
		default:
			break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Get Receive Charactor Inf.                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnm_recv                                                |*/
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
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short
cnm_recv( unsigned char *buf,short siz )
{
	short	i, rtn, bc, cnt;
	char	*ack;
	uchar	md;

	md = ERRMDL_COIM;
	if( tn_save != 0 ) md = ERRMDL_NOTE;


	rtn = 0;
	ack = (char*)ack_tbl;
	for( i=1; i<7; i++,ack++){
		if(*buf == *ack) break;
	}
	if(( siz > 1 ) && ( i < 4 )) {
		buf++;
		bc = (short)(*buf);	/* bc <- Total Byte */
		for( ; ; ){
		    if( siz != (bc+3) ){	/* Recieve Size OK ? YJ */
				ex_errlg( md, ERR_COIN_RECEIVESIZENG, 2, 0 );
				i = 9;			/* Size Error 	*/
				break;
		    }
		    if( (unsigned char)(*(buf+bc+1)) != bcccal( (char*)buf, (short)(bc+1) ) ){ /* LRC Ok ? YJ */
				ex_errlg( md, ERR_COIN_RECEIVEBCCNG, 2, 0 );
				i = 9;			/* LRC Error */
				break;
		    }
		    if( i==2 ){	/* ACK2 ? NJ */
				buf++;
				cnm_save( buf ); /* Copy buf to CN_RDAT */
				break;
		    }
		    if( (i==3 )||(i==1) ){ /* ACK3 or ACK1 ? */
				buf++;
				for( cnt=0 ;cnt < bc; ){
					cnt += (*buf + 1);
					buf++;
					cnm_save( buf ); /* Copy buf to CN_RDAT */
					buf += *(buf-1);
				}
				if( i == 1 ){
					if( cnm_sts & 0x01 ){ /* Req. wait */
						sfstat |= 0x0001;	/* Set Recieve All Req. */
					}
				}
		    }
		    break;
		}
	}
	CN_RCLR();	/* Recieve Inf. Clear */
	rtn = i;
	return( (short)rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| 受信ﾃﾞｰﾀをｾｰﾌﾞし、ﾒｯｾｰｼﾞｷｭｰを登録する                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnm_save                                                |*/
/*| PARAMETER	 : char	*buf | Address of Recieve Data                     |*/
/*| RETURN VALUE : short     | 0:Continue 1:OK ETC:Error                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*| Update       : 2002-03-26 Y.Takahashi								   |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
cnm_save( unsigned char *buf )
{
	short	i;
	unsigned char sf_wk;
	unsigned char	*pt;
	unsigned short	que;

	pt = buf + 1;	/* ﾃﾞｰﾀ部の先頭ｱﾄﾞﾚｽ(種別の次) */
	switch( *buf ){
		case 0x06:	/* つり銭最大枚数 */
			memcpy( CN_RDAT.r_dat06, pt, 12 );
			CN_SUB_SET[0] = (short)bcdbin( CN_RDAT.r_dat06[5] ) * (short)100
								+ (short)bcdbin( CN_RDAT.r_dat06[4] );
			// 10円をCN_SUB_SET[0]、50円をCN_SUB_SET[1]、100円をCN_SUB_SET[2]とする。
			CN_SUB_SET[1] = (short)bcdbin( CN_RDAT.r_dat06[7] ) * (short)100
								+ (short)bcdbin( CN_RDAT.r_dat06[6] );
			CN_SUB_SET[2] = (short)bcdbin( CN_RDAT.r_dat06[9] ) * (short)100
								+ (short)bcdbin( CN_RDAT.r_dat06[8] );
			break;
		case 0x07:	/* ﾁｭｰﾌﾞ保有枚数 */
			coin_inout( 3, pt ); /* 予蓄払い出し枚数ｶｳﾝﾄ */
			memcpy( CN_RDAT.r_dat07, pt, 12 );
			nmisave( SFV_DAT.r_dat07, pt, 12 );
			break;
		case 0x08: /* 入金ﾃﾞｰﾀ(Credit Value Data) */
			coin_inout( 1, pt ); /* ﾄｰﾀﾙ入金枚数加算 */
			memcpy( CN_RDAT.r_dat08, pt, 4 );
			if( cnm_sig == 1 ) break;	/* 一括要求であれば抜ける */

			if( *pt | *(pt+1) | *(pt+2) | *(pt+3) ){ /* 入金ありか? */
				if( (*(pt+2)) != 0 ){ /* 100yen入金あり? */
				}
				SFV_DAT.powoff = 1;
				if( OPECTL.Mnt_mod != 3 ){	/* ｼｽﾃﾑﾒﾝﾃ中でない */
					SFV_DAT.safe_cal_do = -1;	/* 金庫枚数計算指示ｾｯﾄ */
				}
				que = COIN_IN_EVT;

				sf_wk = 0x10;
				for( i=0; i<4; i++, sf_wk<<=1 ){
					if( *(pt+i) >= co_max[i] ){
						if(( sfstat & 0x00f0 ) == 0 ){ /* 既にmaxでない? */
							que = COIN_IM_EVT;
						}
						sfstat |= (short)sf_wk;	/* Coin Max Set */
					}
				}

				queset( OPETCBNO, que, 0, NULL ); /* Set que Input Coin from C/M */
			}
			break;
		case 0x09: /* 払出し可能枚数 */
			memcpy( CN_RDAT.r_dat09, pt, 4 );
			if( cnm_ctrl_info.BIT.standby_snd_after && cnm_sig == 1 ){ // スタンバイ送信後の一括要求時
				cnm_ctrl_info.BIT.r_dat09_rcv_wait = 1;		// 釣銭切れチェック待ちフラグをセット
				cnm_ctrl_info.BIT.standby_snd_after = 0;	// スタンバイ送信後フラグはOFF
			}else if( cnm_ctrl_info.BIT.r_dat09_rcv_wait ){	// 一括応答後の受信時
				cnm_ctrl_info.BIT.r_dat09_rcv_wait = 0;		// クリアする
			}
			break;
		case 0x0a: /* 払出し枚数 */
			coin_inout( 2, pt ); /* ﾄｰﾀﾙ出金枚数加算 */
			memcpy( CN_RDAT.r_dat0a, pt, 4 );
			nmisave( SFV_DAT.r_dat0a, pt, 4 );
			if( cnm_sig == 1 ) break;	/* 一括要求であれば抜ける */

			if(( OPECTL.Mnt_mod != 3 )&&( turi_dat.turi_in == 0 ))
			{	/* ｼｽﾃﾑﾒﾝﾃ中でない && ｲﾝﾍﾞﾝﾄﾘ操作中でない */
				SFV_DAT.safe_cal_do = -1;	/* 金庫枚数計算指示ｾｯﾄ */
			}
			if( (*(pt+2)) != 0 ){ /* 100yen払出し? */
			}
			break;
	    case 0x0b: /* Contorol Data */
			if( ( CN_RDAT.r_dat0b & 0x02 ) != 0 ){ /* ｲﾝﾍﾞﾝﾄﾘ操作? */
				if( ( (*pt) & 0x02 ) == 0 ){ /* 終了? */
					queset( OPETCBNO, (unsigned short)(COIN_INVENTRY), 0, NULL );
					turikan_inventry( 1 ); 	// T集計に加算
				}
			}
			else if( ( (*pt) & 0x02 ) != 0 ){ /* 開始? */
				memset( &turi_dat.forceout, 0, sizeof( turi_dat.forceout ) );
			}
			else if( ( CN_RDAT.r_dat0b & 0x02 ) == 0 ){ /* ｲﾝﾍﾞﾝﾄﾘ動作以外 */
				if( ( (*pt) & 0x08 ) != 0 )	/* 返却スイッチＯＮの場合	*/
				{
					queset( OPETCBNO, (unsigned short)(COIN_RJ_EVT), 0, NULL );		/* 精算中止	*/
				}
			}
			if( cnm_sts & 0x40 ){  					/* 払出し可状態待ち? 	 	*/
				if( (*pt & 0x04) == 0){				/* 払出し可以外の場合		*/
					switch( Cnm_End_PayTimOut ){	// 
						case 0x00:					// 払い出し監視期間外
							break;					// 何もしない
						case 0x01:					// 監視中
							cn_com = 0x80 + 11;		/* 再度CREM OFFを送信する	*/
							break;
						case 0x02:
							sfstat |= 0x0100;		// 入力要求待ち状態とする
							Cnm_End_PayTimOut = 0;	// 監視終了
							break;
						default:
							break;
					}
				}
			}

			CN_RDAT.r_dat0b = (*pt);
			break;
		case 0x0c: /* Coinmech Trouble Data */
			err_cn_chk( pt ,&CN_RDAT.r_dat0c[0] );	/* ｴﾗｰﾁｪｯｸ */
			if( ( CN_RDAT.r_dat0c[1] & 0x40 ) != 0 ){
				if( ( *(pt+1) & 0x40 ) == 0 ){	/* Out Casette? */
					sfstat |= 0x0004;	/* Casette out set */
				}
			}
			memcpy( CN_RDAT.r_dat0c, pt, 3 );
					    /* Error check & Error Prshort */
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
// 警告通知(lcdbm_notice_alm)のcall元の追加
			// 警告通知	事象	0=解除､1=発生(警告通知が変化した時のみ送信する)
			//		ｺｲﾝｶｾｯﾄ		CN_RDAT.r_dat0c[1] & 0x40
			lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
			break;
		case 0x0d:
			memcpy( CN_RDAT.r_dat0d, pt, 2 );
			break;
	    case 0x18: /* 入金ﾃﾞｰﾀ */
			if( cnm_sig == 1 ){ 	/* 一括要求? YB */
				break;
			}
			memcpy( NT_RDAT.r_dat18, pt, 4 );
			if( *pt ){
				if( OPECTL.Ope_Mnt_flg != 4 ){	/* ｼｽﾃﾑﾒﾝﾃﾅﾝｽ-動作ﾁｪｯｸ-Noteﾁｪｯｸ */
					SFV_DAT.nt_escrow++; /* 紙幣ｴｽｸﾛ中ｾｯﾄ */
					SFV_DAT.nt_safe_dt++; // 紙幣金庫合計+1
					ryo_buf.in_coin[4]++;
				}
				SFV_DAT.powoff = 1;

				OPECTL.nyukin_flg = 1;

				queset( OPETCBNO, (unsigned short)(NOTE_IN_EVT), 0, NULL ); /* from N/R */
				inc_dct( MOV_NOTE_RED, 1 );		/* 動作回数ｶｳﾝﾄ */
			}
			break;
		case 0x19:
			memcpy( NT_RDAT.r_dat19, pt, 4 );
			break;
		case 0x1a:
			memcpy( NT_RDAT.r_dat1a, pt, 4 );
			break;
		case 0x1b:
			if(( ( *pt ^ NT_RDAT.r_dat1b[0] ) & 0x02 ) != 0 ){ /* ｽﾀｯｶｰﾋﾞｯﾄ変化? NJ */
				if( ( *pt & 0x02 ) != 0 ){ /* ｽﾀｯｶｰ開? NJ */
					if( cnm_ope != 0 ){ /* 初期化中? YJ */
						sf_wk = 1;					/* ｽﾀｯｶｰ開 */
						queset( OPETCBNO, NOTE_SO_EVT, 1, &sf_wk );
					}
				}else{
					sf_wk = 0;						/* ｽﾀｯｶｰ閉 */
					queset( OPETCBNO, NOTE_SO_EVT, 1, &sf_wk );
				}
			}
			if(( *pt & 0x20 )&&( NT_RDAT.r_dat19[0] )){ /* 紙幣識別中かつｴｽｸﾛ中? */
								/* 後続入金ありのｲﾍﾞﾝﾄｾｯﾄ */
				queset( OPETCBNO, NOTE_I2_EVT, 0, NULL ); /* from N/R */
			}
			memcpy( NT_RDAT.r_dat1b, pt, 3 );
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
// 警告通知(lcdbm_notice_alm)のcall元の追加
			// 警告通知	事象	0=解除､1=発生(警告通知が変化した時のみ送信する)
			//		紙幣ﾘｰﾀﾞｰのﾌﾀ	NT_RDAT.r_dat1b[0] & 0x02
			lcdbm_notice_alm(
						lcdbm_alarm_check());
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
			break;
	    case 0x1c: /* Notereader Trouble Data */
			err_nt_chk( pt ,&NT_RDAT.r_dat1c[0] );	/* ｴﾗｰﾁｪｯｸ */
			cmn_Note_Refund_Chk( pt );
			memcpy( NT_RDAT.r_dat1c, pt, 2 );
				    /* Error check & Error Prshort */
			break;
		case 0x1d:
			memcpy( NT_RDAT.r_dat1d, pt, 2 );
			break;
		case 0x1e:
			memcpy( NT_RDAT.r_dat1e, pt, 5 );
			break;
	    default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ｺｲﾝの入金および出金ﾃﾞｰﾀの受信でそれぞれの枚数をｶｳﾝﾄする                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : void = coin_inout( void )                               |*/
/*| PARAMETER    : mod : =0:Save Area Clear                                |*/
/*|                      =1:入金枚数ｶｳﾝﾄ                                   |*/
/*|                      =2:出金枚数ｶｳﾝﾄ                                   |*/
/*|                      =3:出金枚数ｶｳﾝﾄ（予蓄）                           |*/
/*|              : buf : =recieve buffer address                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
coin_inout( short mod, unsigned char *buf )
{
	short	i;
	unsigned char	*r_dat;
	short	c, *r_dat2;
	short	c1;
	short	c2;

	switch( mod ){
		case 0: /* clear */
			nmicler( SFV_DAT.r_add08, sizeof( SFV_DAT.r_add08 ) );
			break;
		case 1: /* in coin */
			r_dat = CN_RDAT.r_dat08;
			r_dat2 = SFV_DAT.r_add08;
			for( i = 0; i < 4; i++, r_dat++, r_dat2++ ){
				if( *(buf+i) != 0 ){
					c1=bcdbin( *(buf+i) );		/* 今回入金値*/
					c2=bcdbin( *r_dat );		/* 前回入金値*/
					if( c1 > c2 ){
						c= c1 -c2;
					}else if( c1 < c2 ){		/* 前回最大枚数のため、コインメックが0枚から数えなおした*/
												/* ソフトがメックに固定される為、あえて最大枚数は使用しない*/
						c=  c1;
					}else{						/* 前回=今回の場合、何もしない*/
						continue;
					}
					
					(*r_dat2) += c;			/* 入金枚数＋＝今回入金ﾃﾞｰﾀ－前回入金ﾃﾞｰﾀ */

					ryo_buf.in_coin[i] += c; /* 精算ごと入金枚数ｶｳﾝﾄ */
					OPECTL.nyukin_flg = 1;
					inc_dct( MOV_COIN_RED, (short)c );		/* 動作回数ｶｳﾝﾄ */
				}
			}
			break;
		case 2: /* out coin */
			r_dat = CN_RDAT.r_dat0a;
			r_dat2 = SFV_DAT.r_add0a;
			for( i = 0; i < 4; i++, r_dat++, r_dat2++ ){
				if( *(buf+i) != 0 ){
					c = (short)( bcdbin( *(buf+i) ) - bcdbin(*r_dat) );
					if( c > 0 ){
						(*r_dat2) += c;			/* 出金枚数＋＝今回出金ﾃﾞｰﾀ－前回出金ﾃﾞｰﾀ */
						ryo_buf.out_coin[i] += c; 	/* 精算ごと出金枚数ｶｳﾝﾄ */
						inc_dct( MOV_COIN_OUT, (short)c );		/* 動作回数ｶｳﾝﾄ */
					}
				}
			}
			break;
		case 3: /* 保有枚数の変化 */
			r_dat = CN_RDAT.r_dat07;
			for( i = 0; i < 4; i++, r_dat++ ){
				c1 = (short)( bcdbin( *r_dat ) );
				c2 = (short)( bcdbin( *(buf+i) ));
				if( c1 > c2 ){
					// 前回枚数>今回枚数 の場合(枚数減少)
					if( ( (c1 - c2) >= 10 ) ){
						// 減少枚数が10枚以上の場合(例：99->0)
						c2 += 100;
						//釣銭管理枚数不良ｱﾗｰﾑｾｯﾄ(発生/解除)
						alm_chk( ALMMDL_SUB, ALARM_TURIKAN_REV, 2 );			// ｱﾗｰﾑ登録
					}
				}
				c = c1 - c2;
				if( c > 0 ){
					if( CN_RDAT.r_dat0b & 0x02 ){ 	/* ｲﾝﾍﾞﾝﾄﾘｰ中? */
						if( OPECTL.Mnt_mod != 3 ){	/* ｼｽﾃﾑﾒﾝﾃ中でない */
							turi_dat.forceout[i] += c;			// 強制払出枚数ｶｳﾝﾄ
							inc_dct( MOV_COIN_OUT, (short)c );	/* 動作回数ｶｳﾝﾄ */
						}
					}
					else if( turi_dat.turi_in != 0 )
					{ /* 釣銭補充操作中である */
						turi_dat.outcount[i] += c;
					}
				}else if( c < 0 ){
					if( turi_dat.turi_in != 0 ){ /* 釣銭補充操作中である */
						turi_dat.incount[i] += ((~c)+1);
						queset( OPETCBNO, (unsigned short)(COIN_IH_EVT), 0, NULL ); /* Set que Input Coin from C/M */
					}
				}
			}
			r_dat = CN_RDAT.r_dat07 + 4;
			c = (short)( bcdbin3( r_dat ) - bcdbin3( buf+4 ) );
			if( c > 0 ){
				ryo_buf.out_coin[4] += c; 	/* 精算ごと出金枚数ｶｳﾝﾄ */
				ryo_buf.out_coin[0] -= c; 	/* 循環分からは引いてやる */
				if( CN_RDAT.r_dat0b & 0x02 ){ 		/* ｲﾝﾍﾞﾝﾄﾘｰ中? */
					if( OPECTL.Mnt_mod != 3 ){ 		/* ｼｽﾃﾑﾒﾝﾃ中でない */
						turi_dat.forceout[4] += c;			// 強制払出枚数ｶｳﾝﾄ
						inc_dct( MOV_COIN_OUT, (short)c );	/* 動作回数ｶｳﾝﾄ */
					}
				}
				else if( turi_dat.turi_in != 0 )
				{ /* 釣銭補充操作中である */
					turi_dat.outcount[4] += c;
				}
			}

			r_dat = CN_RDAT.r_dat07 + 6;	// 50円予蓄
			c = (short)( bcdbin3( r_dat ) - bcdbin3( buf+6 ) );
			if( c > 0 ){
				ryo_buf.out_coin[5] += c; 	/* 精算ごと出金枚数ｶｳﾝﾄ */
				ryo_buf.out_coin[1] -= c; 	/* 循環分からは引いてやる */
				if( CN_RDAT.r_dat0b & 0x02 ){ 		/* ｲﾝﾍﾞﾝﾄﾘｰ中? */
					if( OPECTL.Mnt_mod != 3 ){ 		/* ｼｽﾃﾑﾒﾝﾃ中でない */
						turi_dat.forceout[5] += c;			// 強制払出枚数ｶｳﾝﾄ
						inc_dct( MOV_COIN_OUT, (short)c );	/* 動作回数ｶｳﾝﾄ */
					}
				}
				else if( turi_dat.turi_in != 0 )
				{ /* 釣銭補充操作中である */
					turi_dat.outcount[5] += c;
				}
			}

			r_dat = CN_RDAT.r_dat07 + 8;	// 100円予蓄
			c = (short)( bcdbin3( r_dat ) - bcdbin3( buf+8 ) );
			if( c > 0 ){
				ryo_buf.out_coin[6] += c; 	/* 精算ごと出金枚数ｶｳﾝﾄ */
				ryo_buf.out_coin[2] -= c; 	/* 循環分からは引いてやる */
				if( CN_RDAT.r_dat0b & 0x02 ){ 	/* ｲﾝﾍﾞﾝﾄﾘｰ中? */
					if( OPECTL.Mnt_mod != 3 ){ 	/* ｼｽﾃﾑﾒﾝﾃ中でない */
						turi_dat.forceout[6] += c;			// 強制払出枚数ｶｳﾝﾄ
						inc_dct( MOV_COIN_OUT, (short)c );	/* 動作回数ｶｳﾝﾄ */
					}
				}
				else if( turi_dat.turi_in != 0 )
				{ /* 釣銭補充操作中である */
					turi_dat.outcount[6] += c;
				}
			}
			break;
		default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Refund Amount Cal.                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : refcal                                                  |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short    |0:Non Amount  -1:Refund Amount                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short
refcal( void )
{
	short	i, ret;
	long	val, wlg;
	long	ka[4], de[4];
	unsigned char	*s_dat;
	unsigned char	r_kind = 0;
	SFV_DAT.refval = 0l;
	for(i=0;i<4;i++){
		if(ref_coinsyu[i] != 0){
			r_kind = 1;
		}
	}
	if( (wlg=SFV_DAT.reffal) == 0 && r_kind == 0){
		return( 0 );
	}
	if(r_kind){
		val = 0;
		for(i=0;i<4;i++){
			if(ref_coinsyu[i] > bcdbin(CN_RDAT.r_dat09[i])){
				CN_SDAT.s_dat02[i] = CN_RDAT.r_dat09[i];
				ref_coinsyu[i]-=bcdbin(CN_RDAT.r_dat09[i]);
			}
			else{
				CN_SDAT.s_dat02[i] = binbcd(ref_coinsyu[i]);
				ref_coinsyu[i] = 0;
			}
			val += (coin_vl[i] * bcdbin(CN_SDAT.s_dat02[i]));
		}
		if(val){
			SFV_DAT.refval = (long)val;					/* 払出額				*/
			SFV_DAT.reffal -= (long)val;
			ret = 1;
		}
		else{
			memset(&ref_coinsyu, 0, sizeof(ref_coinsyu));
			SFV_DAT.reffal = 0;
			ret = 0;
		}
		return ret;
	}
	s_dat = CN_RDAT.r_dat09;			/* 釣銭払出可能枚数ﾃﾞｰﾀ		*/
	val = (long)( bcdbin(*(s_dat+3)) ) * 500l;	/* 500円払出可能枚数＊500	*/
	if( !((wlg >= val)&&(val >= 2000l)) ){		/* 釣銭≧val≧2500でない？	*/
		for( i = 3; i>=0; i-- ){
			val = (long)(wlg % coin_vl[i]);
			de[i] = (long)(wlg - val);	/* 各硬貨で払出したい金額(期待値)	*/
			wlg = val;
		}
		for( i = 0; i<4; i++ ){
			ka[i] = (long)( bcdbin( *(s_dat+i) ) * coin_vl[i]);
		}					/* 各硬貨払出可能枚数＊硬貨金額	*/
		if( de[0] > ka[0] ){
			de[0] = ka[0];
		}
		ka[0] -= de[0];
		val = ka[0] + ka[1];
		if( de[1] > val ){
			de[1] = val;
		}
		val -= de[1];
		val += ka[2];
		if( de[2] > val ){
			de[2] = val;
		}
		val -= de[2];
		val += ka[3];
		if( de[3] > val ){
			de[3] = val;
		}
		val = 0;
		for( i = 0; i<4 ; i++ ){
			val += de[i];							/* Cal. Refund Amount	*/
		}
	}
	if( val ){
		SFV_DAT.refval = (long)val;					/* 払出額				*/
		SFV_DAT.reffal -= (long)val;				/* 残り金額				*/
		s_dat = CN_SDAT.s_dat01;
		for( i=0; i<3; i++,s_dat++) {
			*s_dat = binbcd( (unsigned char)(val%100l) );	/* 払出金額送信ﾃﾞｰﾀｾｯﾄ	*/
			val /= 100l;
		}
		ret = -1;
	}else{
		SFV_DAT.refval = 0l;
		ret = 0;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 通信エラー登録	　                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cmn_ComErrorRegist                                      |*/
/*| PARAMETER	 : void                                                    |*/
/*|				   RXRSLTの通信エラー情報を見て登録する					   |*/
/*| RETURN VALUE : none													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2008/03/17                                              |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void	cmn_ComErrorRegist( void )
{
	uchar	md;

	if( 0 != (RXRSLT.BYTE & 0x3C) ) { 		// PHYエラーあり

		if( Suica_Rec.Data.BIT.EVENT ){		// Suicaの通信Phase
// MH321800(S) G.So ICクレジット対応
//			md = ERRMDL_SUICA;
			md = jvma_setup.mdl;
// MH321800(E) G.So ICクレジット対応
		}
		else{
			if( tn_save == 0 ){				// CMの通信Phase
				md = ERRMDL_COIM;
			}else{
				md = ERRMDL_NOTE;			// BVの通信Phase
			}
		}

// MH321800(S) T.Nagai ICクレジット対応
		if (md == ERRMDL_EC) {
			if( RXRSLT.BIT.B2 != 0 ){ /* 受信ﾊﾞｯﾌｧﾌﾙ */
				ex_errlg( md, ERR_EC_BUF_FULL, 2, 0 );
			}
			if( RXRSLT.BIT.B3 != 0 ){ /* ﾊﾟﾘﾃｨｴﾗｰ */
				ex_errlg( md, ERR_EC_PARITY, 2, 0 );
			}
			if( RXRSLT.BIT.B4 != 0 ){ /* ｵｰﾊﾞｰﾗﾝｴﾗｰ */
				ex_errlg( md, ERR_EC_OVERRUN, 2, 0 );
			}
			if( RXRSLT.BIT.B5 != 0 ){ /* ﾌﾚ-ﾐﾝｸﾞｴﾗｰ */
				ex_errlg( md, ERR_EC_FLAMING, 2, 0 );
			}
		}
		else {
// MH321800(E) T.Nagai ICクレジット対応
		if( RXRSLT.BIT.B2 != 0 ){ /* 受信ﾊﾞｯﾌｧﾌﾙ */
			ex_errlg( md, ERR_COIN_RECEIVEBUFFULL, 2, 0 );
		}
		if( RXRSLT.BIT.B3 != 0 ){ /* ﾊﾟﾘﾃｨｴﾗｰ */
			ex_errlg( md, ERR_COIN_PARITYERROR, 2, 0 );
		}
		if( RXRSLT.BIT.B4 != 0 ){ /* ｵｰﾊﾞｰﾗﾝｴﾗｰ */
			ex_errlg( md, ERR_COIN_OVERRUNERROR, 2, 0 );
		}
		if( RXRSLT.BIT.B5 != 0 ){ /* ﾌﾚ-ﾐﾝｸﾞｴﾗｰ */
			ex_errlg( md, ERR_COIN_FLAMERROR, 2, 0 );
		}
// MH321800(S) T.Nagai ICクレジット対応
		}
// MH321800(E) T.Nagai ICクレジット対応
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ビルバリデータ異常受信時の払い出し中チェック                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cmn_Note_Refund_Chk                                      |*/
/*| PARAMETER	 : *new                                                    |*/
/*| RETURN VALUE : none													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2009/11/06                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	cmn_Note_Refund_Chk( uchar *new )
{
	uchar	wNew,wOld;
	
	wNew = ((*new&0x20)==0x20);
	wOld = ((NT_RDAT.r_dat1c[0]&0x20)==0x20);
	
	if( wNew ){							// 払い出し異常が発生？
		if( wNew != wOld ){				// 今までは発生していない場合
			if( ntr_sts & 0x20 ){		// 払い出し完了待ち？
				if( CN_refund & 0x02 ){	// 
					queset( OPETCBNO, (unsigned short)(TIMEOUT2), 0, NULL ); /* 0x0302 */
				}
			}
		}
	}

}
