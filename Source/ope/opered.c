/*[]----------------------------------------------------------------------[]*/
/*| 磁気ﾘｰﾀﾞｰ関連制御                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"iodefine.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"prm_tbl.h"
#include	"lcd_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"flp_def.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"ntnet_def.h"
#include	"cre_ctrl.h"

#define		R_TIM_OUT	(15*50)										// 15sｳｪｲﾄ

static short Ope_GT_Settei_Check( uchar gt_flg, ushort pno_syu );
uchar GT_Settei_flg;					// GTﾌｫｰﾏｯﾄ設定NGﾌﾗｸﾞ
extern void StackCardEject(uchar req);
uchar	chk_JIS2( void );
char	MAGred_HOUJIN[sizeof(MAGred)];					// 法人カードチェック用バッファ
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//extern	char	pcard_shtter_ctl;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

/*[]----------------------------------------------------------------------[]*/
/*| 磁気ﾘｰﾀﾞｰ初期化待ち処理                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : red_int( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	red_int( void )
{
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（磁気リーダー非対応化）
//	unsigned short	msg = 0;
//	MsgBuf		*msb;		// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
//	int		loop;
//	short	rd_snd;
//	char	ret, i;
//
//	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
//
//	if( prm_get( COM_PRM,S_PAY,21,1,3 ) == 0 ){						// 磁気ﾘｰﾀﾞｰ使用不可?
//		return;
//	}
//	OPE_red = 0;													// ﾘｰﾀﾞｰ自動排出としない
//
//	Lagtim( OPETCBNO, 3, R_TIM_OUT );								// ﾀｲﾏｰ3(15s)起動
//
//	rd_snd = i = 0;
//	for( ret = 0; ret == 0; ){
//		if( i == 0 ){
//			switch( RD_mod ){
//				case 0:
//					opr_snd( rd_snd = 0 );
//					i = 1;
//					break;
//				case 6:
//				case 8:
//				case 20:
//					opr_snd( rd_snd = 94 );							// バージョン要求
//					i = 1;
//					break;
//				case 19:
//					opr_snd( rd_snd = 90 );							// 状態要求
//					i = 1;
//					break;
//				case 10:
//				case 11:
//					Lagcan( OPETCBNO, 3 );							// Timer Cancel
//					return;
//				default:
//					i = 1;
//					break;
//			}
//		}
//
//		// 受信したいﾒｯｾｰｼﾞIDを準備
//		Target_WaitMsgID.Count = 3;
//		Target_WaitMsgID.Command[0] = TIMEOUT3;
//		Target_WaitMsgID.Command[1] = ARC_CR_SND_EVT;
//		Target_WaitMsgID.Command[2] = ARC_CR_E_EVT;
//		for( loop=1; loop ; ){
//			taskchg( IDLETSKNO );									// Change task to idletask
//			msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID );		// 期待するﾒｰﾙだけ受信（他は溜めたまま）
//			if( NULL == msb ){										// 期待するﾒｰﾙなし（未受信）
//				continue;
//			}
//
//			msg = msb->msg.command;
//			switch( msg ){
//				case TIMEOUT3:
//					loop = 0;
//					break;
//				case ARC_CR_SND_EVT:								// 送信完了
//				case ARC_CR_E_EVT:									// 終了ｺﾏﾝﾄﾞ受信
//					loop = 0;
//					opr_ctl( msg );									// message分析処理
//					break;
//				default:
//					break;
//			}
//			if( !loop ){
//				FreeBuf( msb );										// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放
//			}
//		}
//
//		switch( msg ){
//			case TIMEOUT3:											// ﾀｲﾏｰ3ﾀｲﾑｱｳﾄ
//				/*** ﾘｰﾀﾞ正常ではない ***/
//				opr_snd( 0 );
//				ret = 1;
//				break;
//			case ARC_CR_SND_EVT:									// 送信完了
//			case ARC_CR_E_EVT:										// 終了ｺﾏﾝﾄﾞ受信
//				if(( rd_snd == 90 )&&( msg == ARC_CR_E_EVT )){
//					if( prm_get( COM_PRM,S_PAY,21,1,3 ) > 2 ){
//						/*** Vﾌﾟﾘﾝﾀ無し ***/
//					}
//					ret = 1;
//				}else{
//					i = 0;											// 再試行
//				}
//				break;
//			default:
//				break;
//		}
//	}
//	w_stat2 = 0;													// Rｺﾏﾝﾄﾞ受信 New Status
//	Lagcan( OPETCBNO, 3 );											// Timer Cancel

	// 内部で待ち合わせ処理を行ってしまうため、関数スタブ化
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（磁気リーダー非対応化）
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 磁気ﾘｰﾀﾞｰ制御処理                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opr_ctl( ret )                                          |*/
/*| PARAMETER    : ret  : Message                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opr_ctl( unsigned short ret )
{
	short	ans;

	ans = 0;														// ｺﾏﾝﾄﾞ送信指示
																	// 0:未送信
	opr_rcv( ret );													// ﾘｰﾀﾞｰ制御
	switch( ret ){
		/*--------------------------------------------------------------*/
		/* ﾘｰﾄﾞﾃﾞｰﾀ受信時、ﾘｰﾀﾞｰ動作許可されていない場合自動排出する。	*/
		/* 常にﾘｰﾄﾞｺﾏﾝﾄﾞを受信状態の為。								*/
		/*--------------------------------------------------------------*/
		case ARC_CR_R_EVT:											// ﾘｰﾄﾞﾃﾞｰﾀ受信
			if( OPE_red == 2 ){										// 自動排出?
				ans = 13;											// Yes
			}
			break;

		/*----------------------------------------------*/
		/* ｶｰﾄﾞ引き抜きｲﾍﾞﾝﾄ							*/
		/* ﾘｰﾀﾞｰ動作許可されていない時はｼｬｯﾀｰを閉する	*/
		/* 初期化済み状態の場合、ﾘｰﾄﾞｺﾏﾝﾄﾞ送信する。	*/
		/*----------------------------------------------*/
		case ARC_CR_EOT_EVT:										// ｶｰﾄﾞ抜き取り
			if( RD_mod == 6 ){										// ｲﾆｼｬﾙ完了?
				if(( OPE_red == 2 )||( OPE_red == 3 )){				// 自動排出?
					ans = 3;										// Yes..ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
				}
				else if( OPE_red == 1 ){							// 駐車券待ち? NJ
					ans = 3;										// Yes..ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
				}
				if(( OPE_red < 3 )&&( rd_tik == 1 )){				// 駐車券ｱﾘ?
					ans = 5;										// Yes..取込
				}
			}
			break;

		/*--------------------------*/
		/* ﾘｰﾄﾞｺﾏﾝﾄﾞ受信済みｲﾍﾞﾝﾄ	*/
		/* なにもしない				*/
		/*--------------------------*/
		case ARC_CR_EOT_RCMD:										// ﾘｰﾄﾞｺﾏﾝﾄﾞ受信済み
			break;

		/*--------------------------------------------------*/
		/* ﾘｰﾄﾞｺﾏﾝﾄﾞ受信待ちｲﾍﾞﾝﾄ							*/
		/* ﾘｰﾄﾞﾃﾞｰﾀ待ち状態でこのｲﾍﾞﾝﾄ発生は条理NGの為、	*/
		/* ﾘｰﾄﾞｺﾏﾝﾄﾞを送信する。							*/
		/*--------------------------------------------------*/
		case ARC_CR_EOT_RCMD_WT:									// ﾘｰﾄﾞｺﾏﾝﾄﾞ受信待ち
			if( RD_mod == 8 ){										// ﾘｰﾄﾞﾃﾞｰﾀ送信済み?
				ans = 3;											// Yes..ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
			}
			break;
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//		case ARC_CR_EOT_MOVE_TKT:									// 
//			if( pcard_shtter_ctl == 1 ){							// ベリファイ時にシャッター閉した
//				read_sht_opn();
//				pcard_shtter_ctl = 0;
//			}
//			break;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

		/*------------------------------*/
		/* 送信完了ｲﾍﾞﾝﾄ、送信ｴﾗｰｲﾍﾞﾝﾄ	*/
		/* ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ受信				*/
		/* なにもしない					*/
		/*------------------------------*/
		case ARC_CR_SND_EVT:										// 送信完了
		case ARC_CR_SER_EVT:										// 送信ｴﾗｰ
			WaitForTicektRemove = 0;								// 0ｸﾘｱはここだけで行うこと
			if( ( (RD_SendCommand) == 0x06 )||						// 取出口移動か？
				( (RD_SendCommand) == 0x0A ) )
			{
				WaitForTicektRemove = 1;
			}
		case ARC_CR_VER_EVT:										// ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ受信
			break;

		/*----------------------------------------------*/
		/* 動作完了ｲﾍﾞﾝﾄ								*/
		/* 初期化済み状態の場合、ﾘｰﾄﾞｺﾏﾝﾄﾞ送信する。	*/
		/*----------------------------------------------*/
		case ARC_CR_E_EVT:											// 終了ｺﾏﾝﾄﾞ受信(動作完了)
			if( WaitForTicektRemove ){								// ｶｰﾄﾞ抜き取り
				queset( OPETCBNO, ARC_CR_EOT_EVT, 0, NULL );
			}
			if( RD_mod == 6 ){										// ｲﾆｼｬﾙ完了?
				if( opr_bak != -1 ){								// ｺﾏﾝﾄﾞ保留中? NJ
					ans = opr_bak;
					opr_bak = -1;
				}else{
					if(( OPE_red == 2 )||( OPE_red == 3 )){			// 自動排出? NJ
						ans = 3;									// Yes..ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
					}
					else if( OPE_red == 1 ){						// 駐車券待ち? NJ
						ans = 3;									// Yes..ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
					}
					if((OPE_red < 3 )&&( rd_tik == 1 )){			// 駐車券ｱﾘ?
						ans = 5;									// Yes..取込
					}
				}
			}
			StackCardEject(2);									/* 連続排出動作防止タイマ開始 */
			break;

		default:
			if( RD_mod == 6 ){										// ｲﾆｼｬﾙ完了?
				ans = 3;											// Yes..ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
			}
			break;
	}
	/*--------------*/
	/* ｺﾏﾝﾄﾞ送信	*/
	/*--------------*/
	if( ans ){
		if( opr_snd( ans ) != 0 ) {
			opr_snd( 0 );
		}
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 磁気ﾘｰﾀﾞｰ受信ﾃﾞｰﾀ処理                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opr_rcv( ret )                                          |*/
/*| PARAMETER    : ret  : Message                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opr_rcv( unsigned short ret )
{
	unsigned short	i;
	uchar	chk;			// 券ﾃﾞｰﾀﾁｪｯｸ結果
	uchar	pk_syu;			// 駐車場種別
	unsigned short	cnt;
	uchar	disp_card_chk=0;
	uchar set;

	switch( ret ){
		/*----------------------------------------------*/
		/* 送信完了ｲﾍﾞﾝﾄ、送信ｴﾗｰｲﾍﾞﾝﾄ					*/
		/* 初期化済み状態の場合、ﾘｰﾄﾞｺﾏﾝﾄﾞ送信する。	*/
		/*----------------------------------------------*/
		case ARC_CR_SND_EVT:										// 送信完了
			switch( RD_mod ){
				case 1:
				case 3:
				case 7:
				case 10:
					RD_mod++;
					break;
			}
			break;

		/*--------------------------*/
		/* 送信ｴﾗｰｲﾍﾞﾝﾄ				*/
		/* ｲﾆｼｬﾙｺﾏﾝﾄﾞ 前方排出送信	*/
		/*--------------------------*/
		case ARC_CR_SER_EVT:										// 送信ｴﾗｰ
			set = (uchar)prm_get( COM_PRM,S_PAY,21,1,3 );
			if(set == 1 || set == 2) {
				i = rd_init( 2 );
			}
			else {
				i = rd_init( 4 );									// プリンタなし
			}
			if( i == 0 ){											// 正常?(Y)
				RD_mod = 1;
			}else{													// 異常?(Y)
				RD_mod = 0;
			}
			break;

		/*------------------*/
		/* 動作完了ｲﾍﾞﾝﾄ	*/
		/*------------------*/
		case ARC_CR_E_EVT:											// 終了ｺﾏﾝﾄﾞ受信(動作完了)
			if( RED_REC.ercd != 0 ){								// ｴﾗｰあり
				switch( RED_REC.ercd ){
					case	E_VERIFY:
						err_chk( ERRMDL_READER, ERR_RED_VERIFY, 2, 0, 0 );	// ｴﾗｰ登録：ﾍﾞﾘﾌｧｲｴﾗｰ
						break;

					case	E_START:	// スタート符号エラー
					case	E_DATA:		// データエラー
					case	E_PARITY:	// パリティエラー
						err_chk( ERRMDL_READER, (char)(ERR_RED_START+(RED_REC.ercd-E_START)), 2, 0, 0 );	
						break;
						
					default:
						if( (M_R_WRIT == Mag_LastSendCmd) ||			// 書込みｺﾏﾝﾄﾞ
						    (M_R_PRWT == Mag_LastSendCmd) )				// 印字・書込み
						{
							if( RED_REC.ercd == 0x24 ){	//書込みCRCエラー
								alm_chk( ALMMDL_SUB2, ALARM_WRITE_CRC_ERR, 2 );/* ｱﾗｰﾑﾛｸﾞ登録解除		*/
							}else{
								err_chk( ERRMDL_READER, ERR_RED_AFTERWRITE, 2, 0, 0 );	// ｴﾗｰ登録：書込み後ｴﾗｰ発生
							}
						}
						else{
							if( RED_REC.ercd == 0x25 ){	//読込みCRCエラー
								alm_chk( ALMMDL_SUB2, ALARM_READ_CRC_ERR, 2 );/* ｱﾗｰﾑﾛｸﾞ登録解除		*/
							}else{
								err_chk( ERRMDL_READER, ERR_RED_OTHERS, 2, 0, 0 );	// ｴﾗｰ登録：その他ｴﾗｰ発生
							}
						}
						break;
					
				}
				if ( RD_PrcWriteFlag != 0 ){
					alm_chk( ALMMDL_SUB2, ALARM_RED_AFTERWRITE_PRC, 2 );	/* ｱﾗｰﾑﾛｸﾞ登録 発生・解除	*/
					RD_PrcWriteFlag = 0;
				}
			}
			if ( RED_REC.ercd == 0 ){
				if( (M_R_WRIT == Mag_LastSendCmd) || (M_R_PRWT == Mag_LastSendCmd) ){// 書込み / 印字・書込み
					RD_PrcWriteFlag = 0;
				}
			}
			Mag_LastSendCmd = '\0';
			switch( RD_mod ){
				case 2:												// ｲﾆｼｬﾙ完了?(Y)
					if( (prm_get( COM_PRM,S_SYS,11,1,1 ) == 1 ) && ( (RED_REC.posi[1] & 0x08) == 0 )){
						err_chk( ERRMDL_READER, ERR_RED_MAGTYPE, 1, 0, 0 );// 磁気リーダータイプ異常発生
						GT_Settei_flg = 1;
					}
					if( rd_font() == 0 ){
						RD_mod++;
					}else{
						RD_mod = 0;
					}
					break;
				case 4:
				case 13:											// 動作完了
					RD_mod = 6;
				case 6:
					break;
				case 7:
					break;
				case 8:												// 券ﾘｰﾄﾞ
					RD_mod = 6;										// ｲﾆｼｬﾙ完了
					if( RED_REC.ercd != 0 ){
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//						inc_dct( READ_ER, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
						MAGred[MAG_ID_CODE] = 0xff;
						OPECTL.LastCard = OPE_LAST_RCARD_MAG;
						OPECTL.LastCardInfo = (ushort)MAGred[MAG_ID_CODE];
						MagReadErrCodeDisp(RED_REC.ercd);			// 磁気データ読取りエラー詳細表示
// MH321800(S) D.Inaba ICクレジット対応
						// 決済リーダ関連文言表示中ならメッセージは表示させない
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						if( ec_MessagePtnNum != 0 ){
						if( isEC_MSG_DISP() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							break;
						}
// MH321800(E) D.Inaba ICクレジット対応
						if( OPE_red == 3 ){							// 精算中?
							ope_anm(AVM_MAGCARD_ERR);
							grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[9] );		// "    このカードは読めません    "ｴﾗｰ表示は反転文字にて表示
							Lagtim( OPETCBNO, 7, 5*50 );			// ﾀｲﾏｰ7(5s)起動(読取ｴﾗｰ表示用)
						}
					}
					break;
				case 11:											// 券抜き取り待ち
					RD_mod = 6;
					if( RED_REC.ercd == E_VERIFY ){					// ﾍﾞﾘﾌｧｲｴﾗｰ
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//						inc_dct( READ_VN, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
						RED_REC.ercd = 0;
					}
					if( rd_tik == 2 ){								// 駐車券領収書抜き待ち?
						rd_tik = 0;
					}
					if( RD_Credit_kep ){
						RD_Credit_kep = 0;
					}
					if(( OPE_red != 3 )&&( OPE_red != 4 )){
						switch( rd_tik ){
							case 1:									// 駐車券保留
								opr_snd( 5 );						// 駐車券取込
								break;
							case 3:									// 駐車券戻し待ち
								opr_snd( 13 );						// 券取出口移動
								break;
							case 4:									// 駐車券書込待ち
								opr_snd( 7 );						// 中止券ﾗｲﾄ
								break;
							case 5:									// 駐車券戻し待ち
								opr_snd( 15 );						// 預かり印字
								break;
							case 6:									// 駐車券書込待ち
								opr_snd( 14 );						// 預かり中止
								break;
						}
					}
					if( pas_kep ){									// 定期保留?
						pas_kep = 0;
					}
					break;
				case 12:											// ﾘｰﾀﾞ動作中
					rd_faz++;
					if( rd_mov( rd_faz ) != 0 ){
						RD_mod = 0;
					}
					break;
				case 20:											// ﾊﾞｰｼﾞｮﾝ要求出力後
				case 15:											// 状態要求出力後
					RD_mod = 20;
					break;
				default:
					RD_mod = 0;
					break;
			}
			break;

		/*--------------------------------------------------------------*/
		/* ﾘｰﾄﾞﾃﾞｰﾀ受信時、ﾘｰﾀﾞｰ動作許可されていない場合自動排出する。	*/
		/* 常にﾘｰﾄﾞｺﾏﾝﾄﾞを受信状態の為。								*/
		/*--------------------------------------------------------------*/
		case ARC_CR_R_EVT:											// ﾘｰﾄﾞﾃﾞｰﾀ受信
			RD_PrcWriteFlag = 0;
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//			inc_dct( READ_YO, 1 );
//			inc_dct( READ_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
			RD_pos = RDT_REC.rdat[1];
			//※ al_preck内で、RD_pos > 1の裏表判定をしているため、今回のＧＴ判別ビットをマスクする
			if( prm_get( COM_PRM,S_SYS,11,1,1 ) == 1 ){
				RD_pos &= 0x03;
			}
			if( RD_mod == 8 ){
				RD_mod = 9;
				memset( MAGred, '\0', sizeof(MAGred) );
				if( RDT_REC.idc2 >= 3 ){
					// 読み込んだカードのGTフォーマットチェック
					if(( RDT_REC.rdat[1] & 0x80) == 0x80){
						MAGred[MAG_GT_APS_TYPE] = 1;						// ＧＴ情報にﾌｫｰﾏｯﾄﾌﾗｸﾞをｾｯﾄ
						for( i = MAG_EX_GT_PKNO; i < 9; i++ ){				// 拡張した駐車場Noを格納する
							MAGred[MAG_EX_GT_PKNO] |= (char)(( RDT_REC.rdat[i+3] & 0x80 ) >> (8 - i));
						}
					}
					
					//全てのパリティビットを外す
					for( i = MAG_ID_CODE,cnt = 0; cnt < RDT_REC.idc2 - 3; i++,cnt++ ){
						MAGred[i] = (char)( RDT_REC.rdat[cnt+3] & 0x7f );
					}
					chk = NG;
						// 拡張IDチェック：APSのみ処理を行う
						if(MAGred[MAG_GT_APS_TYPE] == 0){
							chk = Read_Tik_Chk( (uchar *)&MAGred[MAG_ID_CODE], &pk_syu, (uchar)(RDT_REC.idc2 - 3) );	// 券ﾃﾞｰﾀﾁｪｯｸ＆ID変換
						}

						if( chk != OK && ck_jis_credit ( (uchar)(RDT_REC.idc2 - 3), (char*)&RDT_REC.rdat[3] ) != 0 ){
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//							if( CREDIT_ENABLED() ){
//								RD_Credit_kep = 1;
//							}
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
							disp_card_chk = 1;
						}else if( MAGred[MAG_ID_CODE] == 0x1a ){
							pas_kep = (char)-1;							// 定期保留ｾｯﾄ
						}
					OPECTL.LastCard = OPE_LAST_RCARD_MAG;
					OPECTL.LastCardInfo = (ushort)MAGred[MAG_ID_CODE];
					if( disp_card_chk == 1 ){							// 券データを表示しない？
						OPECTL.LastCardInfo |= 0x8000;
// MH321800(S) T.Nagai ICクレジット対応
						// クレジットのJISデータは内部メモリに保持しない
						memset( MAGred, '\0', sizeof(MAGred) );
						memset( RDT_REC.rdat, '\0', sizeof(RDT_REC.rdat) );
// MH321800(E) T.Nagai ICクレジット対応
					}
					OPECTL.CR_ERR_DSP = 0;
					OPECTL.other_machine_card = 0;
					if( (MAGred[MAG_ID_CODE] >= 0x21 && MAGred[MAG_ID_CODE] <= 0x2B) ){
						OPECTL.other_machine_card = 1;
					}
				}
			}else{
				MAGred[MAG_ID_CODE] = 0xff;
				OPECTL.LastCardInfo = (ushort)0;
				OPECTL.CR_ERR_DSP = 0;
			}
			break;

		/*------------------*/
		/* ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ受信	*/
		/*------------------*/
		case ARC_CR_VER_EVT:										// ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ受信
			RD_mod = 20;
			break;

		/*----------------------*/
		/* ｶｰﾄﾞ引き抜きｲﾍﾞﾝﾄ	*/
		/*----------------------*/
		case ARC_CR_EOT_EVT:										// ｶｰﾄﾞ抜き取り
			if( RD_mod == 11 ){
				if( OPE_red != 5 ){
					RD_mod = 6;
					if( RED_REC.ercd == E_VERIFY ){					// ﾍﾞﾘﾌｧｲｴﾗｰ
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//						inc_dct( READ_VN, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
					}
				}
				RED_REC.ercd = 0;
				if( rd_tik == 2 ){									// 駐車券領収書抜き待ち?
					rd_tik = 0;
				}
				if( pas_kep ){										// 定期保留?
					pas_kep = 0;
				}
			}
			break;

		default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 磁気ﾘｰﾀﾞｰｺﾏﾝﾄﾞ送信処理                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opr_snd( mod )                                          |*/
/*| PARAMETER    : mod  : 送信内容                                         |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	opr_snd( short mod )
{
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（磁気リーダー非対応化）
//	short	ans, cmd;
//	short 	ret;
//	uchar	set;
//
//	if( prm_get( COM_PRM,S_PAY,21,1,3 ) == 0 ){						// 磁気ﾘｰﾀﾞｰ使用不可?
//		return( 0 );
//	}
//// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
//	if( ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0 ){
//		switch(mod) {
//		case 90:	// 状態要求
//		case 91:	// バージョン要求
//		case 94:	// ROM部番
//		case 95:	// DIPSW状態
//			break;
//		default:
//			return( 0 );
//		}
//	}
//// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
//
//	if(( RD_mod == 12 )||( RD_mod == 13 )){							// ﾘｰﾀﾞｰ動作中?
//		opr_bak = mod;												// Yes
//		return( 0 );
//	}
//
//	RD_SendCommand = 0;
//	switch( mod ){
//		case 2:					// リーダー内から取出し口へ移動
//		case 13:				// 保留位置から取出し口へ移動
//		    // --------------------------------------------------------------------------------------------- //
//			// read_sht_opn関数内でｵｰﾌﾟﾝ済みの場合は何もしない為、既存の箇所ですでにシャッター開要求を       //
//			// 掛けていても問題ないので、ｵｰﾌﾟﾝ漏れを防ぐ為に磁気リーダーへ戻し要求を掛ける直前に開要求する   //
//		    // --------------------------------------------------------------------------------------------- //
//			if( !OPECTL.Mnt_mod )	// 精算モードのときのみ
//				read_sht_opn();
//			break;
//		default:
//			break;
//	}
//	ans = 0;
//	switch( mod ){
//		case 0:														// Initial Command
//			set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				ret = rd_init( 2 );
//			}
//			else {
//				ret = rd_init( 4 );									// プリンタなし
//			}
//			if(ret == 0) {
//				RD_mod = 1;
//// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
////				inc_dct( READ_DO, 1 );
//// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//			} else {
//				ans = -1;
//			}
//			break;
//		case 1:														// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ﾅｼ)
//			if(( RD_mod == 11 )||(( RD_mod == 8 )&&( w_stat2 ))) break;
//			set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				ret = rd_read( 0 );
//			}
//			else {
//				ret = rd_read( 2 );
//			}
//			if( ret == 0 ) {
//				RD_mod = 7;
//			} else {
//				ans = -1;
//			}
//			break;
//		case 2:														// 取出し口移動
//			if( rd_mov( rd_faz = 0 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 3:														// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
//			if(( RD_mod == 11 )||(( RD_mod == 8 )&&( w_stat2 )) || CardStackRetry) break;		// 排出動作中はリードコマンド出さない
//			set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				ret = rd_read( 1 );
//			}
//			else {
//				ret = rd_read( 2 );
//			}
//			if( ret == 0 ) {
//				RD_mod = 7;
//			} else {
//				ans = -1;
//			}
//			break;
//		case 4:														// 駐車券(領収証)取込(後排出)
//			if( rd_mov( rd_faz = 12 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 5:														// 駐車券取込(後排出)
//			set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				rd_faz = 2;
//			}
//			else {
//				rd_faz = 33;
//			}
//
//			if( rd_mov( rd_faz ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 6:														// 駐車券領収証
////			RD_pos = tikchu.pos;									// Head posｾｯﾄ
//			if( rd_mov( rd_faz = 4 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 7:														// 精算中止券書込(ﾍﾞﾘﾌｧｲ有り)
////			RD_pos = tikchu.pos;									// Head posｾｯﾄ
//			if( rd_mov( rd_faz = 13 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 8:														// 駐車券印字取込
//			if( rd_mov( rd_faz = 14 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 110:													// 駐車券印字取込
//			if( rd_mov( rd_faz = 21 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 9:														// 定期書込(ﾍﾞﾘﾌｧｲ有り)
//			if( rd_mov( rd_faz = 5 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 10:													// ｻｰﾋﾞｽ券取込(後排出)
//		case 11:													// ｻｰﾋﾞｽ券取込(前移動+後排出)
//			memset( &MDP_buf[0], ' ', 30 );
//			if( CPrmSS[S_DIS][2] ){									// 廃券ﾏｰｸ印字する
//				MDP_buf[29] = 0x58;									// 'X'
//			}
//			MDP_siz = 30;
//			set = (uchar)prm_get( COM_PRM,S_PAY,21,1,3 );
//			if(set == 1 || set == 2) {
//				cmd = 6;
//				if( mod == 11 ) cmd = 7;
//			}
//			else {
//				cmd = 33;
//			}
//			if( rd_mov( rd_faz = cmd ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 12:													// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ書込(ﾍﾞﾘﾌｧｲ有り)
//			if( rd_mov( rd_faz = 10 ) != 0 ){
//				ans = -1;
//			}
//			break;
//		case 13:													// 券取り出し口移動(駐車券戻し)
//			if( rd_mov( rd_faz = 1 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 14:													// 駐車券預かり証(中止)
////			RD_pos = tikchu.pos;									// Head posｾｯﾄ
//			if( rd_mov( rd_faz = 17 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 15:													// 駐車券預かり証(中止)
////			RD_pos = tikchu.pos;									// Head posｾｯﾄ
//			if( rd_mov( rd_faz = 16 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 16:													// 定期書込(ﾍﾞﾘﾌｧｲ無し)
//			if( rd_mov( rd_faz = 22 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 17:													// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ書込(ﾍﾞﾘﾌｧｲ無し)
//			if( rd_mov( rd_faz = 26 ) != 0 ){
//				ans = -1;
//			}
//			break;
//		case 18:													// 精算中止券書込(ﾍﾞﾘﾌｧｲなし)
////			RD_pos = (uchar)CRD_DAT.TIK.pos;						// Head posｾｯﾄ
//			if( rd_mov( rd_faz = 28 ) == 0 ) {
//				rd_tik = 2;											// 駐車券戻し 抜き待ち
//			}else{
//				ans = -1;
//			}
//			break;
//		case 19:													// 回数券書込(ﾍﾞﾘﾌｧｲなし)
//			if( rd_mov( rd_faz = 29 ) != 0 ){
//				ans = -1;
//			}
//			break;
//		case 20:													// 回数券書込(ﾍﾞﾘﾌｧｲあり)
//			if( rd_mov( rd_faz = 31 ) != 0 ){
//				ans = -1;
//			}
//			break;
//		case 90:													// 状態要求
//			if( rd_test( 3 ) == 0 ) {
//				RD_mod = 15;
//			} else {
//				ans = -1;
//			}
//			break;
//		case 91:													// ﾊﾞｰｼﾞｮﾝ要求
//			if( rd_test( 4 ) == 0 ) {
//				RD_mod = 16;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 92:													// ﾘｰﾄﾞﾗｲﾄﾃｽﾄ要求
//			if( rd_test( 1 ) == 0 ){
//// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
////				inc_dct( READ_DO, 1 );
//// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//				RD_mod = 17;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 93:													// 印字ﾃｽﾄ要求
//			if( rd_test( 2 ) == 0 ){
//// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
////				inc_dct( READ_DO, 1 );
//// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//				RD_mod = 18;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 94:													// ROM部番読出し要求
//			// FB7000 ﾒﾝﾃﾅﾝｽ Command送信
//			if( rd_FB7000_MntCommandSend( 1, 0, 0 ) == 0 ){			// ROM部番読出し要求
//				RD_mod = 19;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 95:													// ROM部番読出し要求
//			// FB7000 ﾒﾝﾃﾅﾝｽ Command送信
//			if( rd_FB7000_MntCommandSend( 9, 0, 0 ) == 0 ){			// ディップスイッチ状態取得要求
//				RD_mod = 19;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 100:													// 駐車券書込（発券）
//			if( rd_mov( rd_faz = 18 ) == 0 ) {
//				rd_tik = 2;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 101:													// 廃券
//			if( rd_mov( rd_faz = 19 ) == 0 ) {
//				rd_tik = 0;
//			}else{
//				ans = -1;
//			}
//			break;
//		case 102:											// 廃券書き込み
//			memset( &MDP_buf[0], ' ', 30 );
//			MDP_siz = 30;
//			if( rd_mov( rd_faz = 36 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 200:													// ｶｰﾄﾞ取り込み
//			if( rd_mov( rd_faz = 23 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//		case 201:													// 定期書込(ｶｰﾄﾞ発行用)
//			if( rd_mov( rd_faz = 24 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//
//		case 202:													// 係員ｶｰﾄﾞ書込(ｶｰﾄﾞ発行用)
//			if( rd_mov( rd_faz = 25 ) != 0 ) {
//				ans = -1;
//			}
//			break;
//
//		default:
//			break;
//	}
//	if( ans == -1 ){
//		RD_mod = 0;
//	}
//	return( ans );
	// 内部で待ち合わせ処理を行ってしまうため、関数スタブ化
	return -1;
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（磁気リーダー非対応化）
}

/*[]----------------------------------------------------------------------[]*/
/*| 磁気ﾘｰﾀﾞｰ初期化処理                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opr_int( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opr_int( void )
{
	if( RD_mod == 0 ){
		opr_snd( 0 );
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ｻｰﾋﾞｽ券ﾘｰﾄﾞ                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_svsck( m_servic *mag )                               |*/
/*| PARAMETER    : m_servic *mag : 磁気ﾃﾞｰﾀ格納先頭ｱﾄﾞﾚｽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = 駐車場№ｴﾗｰ                                  |*/
/*|                       2 = ﾃﾞｰﾀ異常                                     |*/
/*|                       3 = 期限切れ                                     |*/
/*|                       6 = 期限前                                       |*/
/*|                      13 = 種別規定外                                   |*/
/*|                      14 = 限度枚数ｵｰﾊﾞｰ                                |*/
/*|                      25 = 車種ｴﾗｰ                                      |*/
/*|                      26 = 設定ｴﾗｰ                                      |*/
/*|                      27 = 料金種別ｴﾗｰ                                  |*/
/*|                      28 = 限度枚数0ｴﾗｰ                                 |*/
/*|                      30 = 精算順序ｴﾗｰ                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const short pno_dt[8] = { 0x0000, 0x0200, 0x0100, 0x0300, 0x0080, 0x0280, 0x0180, 0x0380 };
const char ck_dat[6] = { 0, 0, 0, 99, 99, 99 };
const char prm_pos[4] = { 0, 5, 3, 1 };
const char magtype[4] = {6,1,2,3};

short	al_svsck( m_gtservic *mag )
{
	short	ret, wk;
	short	cardknd;
	ushort	pkno_syu;
	uchar	chk_end1;		// 券ﾃﾞｰﾀ一致ﾌﾗｸﾞ	（OFF=不一致／ON=一致）
	uchar	chk_end2;		// 券種別不一致ﾌﾗｸﾞ	（OFF=一致／ON=不一致）
	uchar	tbl_no;			// 期限変更ﾃｰﾌﾞﾙNo.
	short	tbl_syu;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：種別
	short	tbl_data;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：内容
	short	tbl_syear;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：開始年ﾃﾞｰﾀ
	short	tbl_smon;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：開始月ﾃﾞｰﾀ
	short	tbl_sday;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：開始日ﾃﾞｰﾀ
	short	tbl_eyear;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：終了年ﾃﾞｰﾀ
	short	tbl_emon;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：終了月ﾃﾞｰﾀ
	short	tbl_eday;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：終了日ﾃﾞｰﾀ
	ushort	tbl_sdate;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：開始年月日（dnrmlzm変換ﾃﾞｰﾀ）
	ushort	tbl_edate;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：終了年月日（dnrmlzm変換ﾃﾞｰﾀ）
	short	data_adr;		// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ
	char	data_pos;		// 使用可能料金種別のﾃﾞｰﾀ位置
	int		i;
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
	long	role = 0;		// 役割
	long	chng = 0;		// 車種切換
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）

	CRD_DAT.SVS.pno = 												// 駐車場№ｾｯﾄ
		(long)mag->servic.svc_pno[0] + (long)pno_dt[mag->servic.svc_pno[1]>>4];
	if(mag->magformat.type == 1){//GTフォーマット
		CRD_DAT.SVS.pno |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17格納
		CRD_DAT.SVS.GT_flg = 1;										//GTフォーマットフラグセット
	}else{
		CRD_DAT.SVS.GT_flg = 0;										//GTフォーマットフラグセット	
	}

	CRD_DAT.SVS.knd = (short)( mag->servic.svc_pno[1] & 0x0f );		// 種別ｾｯﾄ
	CRD_DAT.SVS.cod =												// 店№ｾｯﾄ
		(short)mag->servic.svc_sno[1] + (((short)mag->servic.svc_sno[0])<<7);
	CRD_DAT.SVS.sts = (short)( mag->servic.svc_sts );				// ｽﾃｰﾀｽ

	if( memcmp( mag->servic.svc_sta, ck_dat, 6 ) == 0 ){			// 期限ｱﾘ?
		CRD_DAT.SVS.std = 0;										// 有効開始日ｾｯﾄ
		CRD_DAT.SVS.end = 0xffff;									// 有効終了日ｾｯﾄ
	}else{
		wk = (short)(mag->servic.svc_sta[0]);
		if( wk >= 80 ){
			wk += 1900;
		}else{
			wk += 2000;
		}
		if( chkdate( wk, (short)mag->servic.svc_sta[1], (short)mag->servic.svc_sta[2] ) ){	// 有効開始日ﾁｪｯｸNG?
			return( 2 );											// ﾃﾞｰﾀ異常
		}
		CRD_DAT.SVS.std = 											// 有効開始日ｾｯﾄ
			dnrmlzm( wk,(short)mag->servic.svc_sta[1],(short)mag->servic.svc_sta[2] );

		wk = (short)mag->servic.svc_end[0];
		if( wk >= 80 ){
			wk += 1900;
		}else{
			wk += 2000;
		}
		if( chkdate( wk, (short)mag->servic.svc_end[1], (short)mag->servic.svc_end[2] ) ){	// 有効終了日ﾁｪｯｸNG?
			return( 2 );											// ﾃﾞｰﾀ異常
		}
		CRD_DAT.SVS.end = 											// 有効終了日ｾｯﾄ
			dnrmlzm( wk,(short)mag->servic.svc_end[1],(short)mag->servic.svc_end[2] );
	}

	ret = 0;
	for( ; ; ){
		if( CRD_DAT.SVS.knd == 0 ){
			if( CRD_DAT.SVS.sts == 0 ){
				cardknd = 12;										// 掛売券
			}else{
				cardknd = 14;										// 割引券
			}
		}else{
			cardknd = 11;											// ｻｰﾋﾞｽ券
		}

		if( 0L == (ulong)CRD_DAT.SVS.pno ){
			ret = 1;												// 駐車場№ｴﾗｰ
			break;
		}

		// 減額精算後のサービス券類は無効
		i = is_paid_remote(&PayData);
		if (i >= 0 && PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU) {
			ret = 8;
			break;
		}

		if(( prm_get( COM_PRM, S_SYS, 71, 1, 6 ) == 1 )&&			// 基本サービス券使用可
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.SVS.pno )){ 		// 基本駐車場№?
			pkno_syu = KIHON_PKNO;									// 基本
		}
		else if(( prm_get( COM_PRM, S_SYS, 71, 1, 1 ) == 1 )&&		// 拡張1サービス券使用可
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.SVS.pno )){	// 拡張1駐車場№?
			pkno_syu = KAKUCHOU_1;									// 拡張1
		}
		else if(( prm_get( COM_PRM, S_SYS, 71, 1, 2 ) == 1 )&&		// 拡張2サービス券使用可
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.SVS.pno )){	// 拡張2駐車場№?
			pkno_syu = KAKUCHOU_2;									// 拡張2
		}
		else if(( prm_get( COM_PRM, S_SYS, 71, 1, 3 ) == 1 )&&		// 拡張3サービス券使用可
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.SVS.pno )){	// 拡張3駐車場№?
			pkno_syu = KAKUCHOU_3;									// 拡張3
		}
		else{
			ret = 1;												// 駐車場№ｴﾗｰ
			break;
		}

		if( Ope_GT_Settei_Check( CRD_DAT.SVS.GT_flg, pkno_syu ) == 1 ){		// 設定NGチェック
			ret = 33;
			break;
		}

		if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){
			if( ((CRD_DAT.SVS.GT_flg == 1) && ((GTF_PKNO_LOWER > CRD_DAT.SVS.pno) || (CRD_DAT.SVS.pno > GTF_PKNO_UPPER))) ||
				((CRD_DAT.SVS.GT_flg == 0) && ((APSF_PKNO_LOWER > CRD_DAT.SVS.pno) || (CRD_DAT.SVS.pno > APSF_PKNO_UPPER))) ){		//駐車場No範囲チェック
				ret = 1;												// 駐車場№ｴﾗｰ
				break;
			}
		}

		wk = (short)prm_get( COM_PRM, S_DIS, 1, 2, 1 );				// 使用限度枚数設定
		if(wk == 0){
			ret = 28;												// 使用限度枚数設定0枚
			break;
		}
		if( card_use[USE_SVC] >= wk ){								// ｻｰﾋﾞｽ券,掛売券,割引券使用枚数
			ret = 14;												// 限度枚数ｵｰﾊﾞｰ
			break;
		}
		if(cardknd == 11){
			if( 1 == (short)prm_get( COM_PRM, S_DIS, 1, 1, 6 ) ){		// 種別使用限度枚数設定？
				if( CRD_DAT.SVS.knd <= 3 ){
					data_adr = 106;											// 使用限度枚数別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
					data_pos = (char)prm_pos[CRD_DAT.SVS.knd];				// 使用限度枚数のﾃﾞｰﾀ位置取得
				}else if( CRD_DAT.SVS.knd <= 6 ){
					data_adr = 107;											// 使用限度枚数のﾃﾞｰﾀｱﾄﾞﾚｽ取得
					data_pos = (char)prm_pos[(CRD_DAT.SVS.knd-3)];			// 使用限度枚数のﾃﾞｰﾀ位置取得
				}else if( CRD_DAT.SVS.knd <= 9 ){
					data_adr = 108;											// 使用限度枚数のﾃﾞｰﾀｱﾄﾞﾚｽ取得
					data_pos = (char)prm_pos[(CRD_DAT.SVS.knd-6)];			// 使用限度枚数のﾃﾞｰﾀ位置取得
				}else if( CRD_DAT.SVS.knd <= 12 ){
					data_adr = 109;											// 使用限度枚数のﾃﾞｰﾀｱﾄﾞﾚｽ取得
					data_pos = (char)prm_pos[(CRD_DAT.SVS.knd-9)];			// 使用限度枚数のﾃﾞｰﾀ位置取得
				}else if( CRD_DAT.SVS.knd <= 15 ){
					data_adr = 110;											// 使用限度枚数のﾃﾞｰﾀｱﾄﾞﾚｽ取得
					data_pos = (char)prm_pos[(CRD_DAT.SVS.knd-12)];			// 使用限度枚数のﾃﾞｰﾀ位置取得
				}
				wk = (short)prm_get( COM_PRM, S_SER, data_adr, 2, data_pos );// 使用限度枚数設定
				if(wk == 0 ){
					ret = 28;												// 使用限度枚数設定0枚
					break;
				}
				if(card_use2[CRD_DAT.SVS.knd-1] >= wk ){
					ret = 14;												// 限度枚数ｵｰﾊﾞｰ
					break;
				}
			}
		}

		if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
			if( CardSyuCntChk( pkno_syu, cardknd, CRD_DAT.SVS.knd, CRD_DAT.SVS.cod, CRD_DAT.SVS.sts ) ){	// 1精算の割引種類の件数ｵｰﾊﾞｰ?
				ret = 14;											// 限度枚数ｵｰﾊﾞｰ
				break;
			}
		}

		// 券期限変更処理
		chk_end1 = OFF;												// 券ﾃﾞｰﾀ一致ﾌﾗｸﾞ：OFF

		for( tbl_no = 1 ; tbl_no <= 3 ; tbl_no++ ){					// 期限変更ﾃｰﾌﾞﾙ（１～３）検索

			chk_end2 = OFF;											// 種別不一致ﾌﾗｸﾞ：OFF

			tbl_syu = (short)CPrmSS[S_DIS][8+((tbl_no-1)*6)];		// 期限変更ﾃｰﾌﾞﾙに設定されている種別を取得
			tbl_data= (short)CPrmSS[S_DIS][9+((tbl_no-1)*6)];		// 期限変更ﾃｰﾌﾞﾙに設定されている内容を取得

			// 券種別ﾁｪｯｸ
			switch( tbl_syu ){										// 設定されている種別と券ﾃﾞｰﾀの種別を比較

				case	1:											// 設定種別＝サービス券

					if( cardknd != 11 ){							// 券ﾃﾞｰﾀ種別＝サービス券？
						chk_end2 = ON;								// NO → 種別不一致
					}
					else{
						if( tbl_data != 0 ){						// 設定内容＝０（サービス券全て）以外？
							if( tbl_data != CRD_DAT.SVS.knd ){		// サービス券種別(A～C)一致？
								chk_end2 = ON;						// NO → 種別不一致
							}
						}
					}
					break;

				case	2:											// 設定種別＝掛売券
					if( cardknd != 12 ){							// 券ﾃﾞｰﾀ種別＝掛売券？
						chk_end2 = ON;								// NO → 種別不一致
					}
					else{
						if( tbl_data != 0 ){						// 設定内容＝０（掛売券全て）以外？
							if( tbl_data != CRD_DAT.SVS.cod ){		// 店No.一致？
								chk_end2 = ON;						// NO → 種別不一致
							}
						}
					}
					break;

				case	3:											// 設定種別＝割引券
					if( cardknd != 14 ){							// 券ﾃﾞｰﾀ種別＝割引券？
						chk_end2 = ON;								// NO → 種別不一致
					}
					else{
						if( tbl_data != 0 ){						// 設定内容＝０（割引券全て）以外？
							if( tbl_data != CRD_DAT.SVS.cod ){		// 店No.一致？
								chk_end2 = ON;						// NO → 種別不一致
							}
						}
					}
					break;

				case	4:											// 設定種別＝全て
					break;											// サービス券、掛売券、割引券全て一致と判断

				case	0:											// 設定種別＝なし
				default:											// 設定種別＝その他
					chk_end2 = ON;									// → 種別不一致
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			}

			if( chk_end2 == ON ){									// 設定されている種別と券ﾃﾞｰﾀの種別不一致？
				continue;											// YES → 判定中の期限変更ﾃｰﾌﾞﾙ検索終了
			}

			// 有効期限ﾁｪｯｸ

			tbl_syear = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 5 );	// 変更前の開始（年）取得
			tbl_smon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 3 );	// 変更前の開始（月）取得
			tbl_sday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 1 );	// 変更前の開始（日）取得

			tbl_eyear = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 5 );	// 変更前の終了（年）取得
			tbl_emon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 3 );	// 変更前の終了（月）取得
			tbl_eday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 1 );	// 変更前の終了（日）取得

			if( (CRD_DAT.SVS.std == 0) && (CRD_DAT.SVS.end == 0xffff) ){					// 券ﾃﾞｰﾀの有効期限？
				// 券ﾃﾞｰﾀ＝無期限
				if(	(tbl_syear == 0) && (tbl_smon == 0) && (tbl_sday == 0) &&
					(tbl_eyear == 99) && (tbl_emon == 99) && (tbl_eday == 99) ){			// 期限変更ﾃｰﾌﾞﾙに設定されている変更前有効期限？

					chk_end1 = ON;															// 無期限 → 券ﾃﾞｰﾀ一致
				}
			}
			else{
				// 券ﾃﾞｰﾀ＝有効期限あり
				if( tbl_syear >= 80 ){		// 開始年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
					tbl_syear += 1900;
				}else{
					tbl_syear += 2000;
				}
				if( tbl_eyear >= 80 ){		// 終了年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
					tbl_eyear += 1900;
				}else{
					tbl_eyear += 2000;
				}
				tbl_sdate = dnrmlzm( tbl_syear, tbl_smon, tbl_sday );	// 有効開始日変換
				tbl_edate = dnrmlzm( tbl_eyear, tbl_emon, tbl_eday );	// 有効終了日変換

				if( (CRD_DAT.SVS.std == tbl_sdate) && (CRD_DAT.SVS.end == tbl_edate) ){		// 開始日＆終了日一致？

					chk_end1 = ON;															// YES → 券ﾃﾞｰﾀ一致
				}
			}

			if( chk_end1 == ON ){									// 券ﾃﾞｰﾀ一致？

				// 券ﾃﾞｰﾀ（種別＆有効期限）と期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ（種別＆変更前有効期限）が一致した場合

				tbl_syear = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 5 );	// 変更後の開始（年）取得
				tbl_smon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 3 );	// 変更後の開始（月）取得
				tbl_sday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 1 );	// 変更後の開始（日）取得

				tbl_eyear = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 5 );	// 変更後の終了（年）取得
				tbl_emon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 3 );	// 変更後の終了（月）取得
				tbl_eday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 1 );	// 変更後の終了（日）取得

				if(	(tbl_syear == 0) && (tbl_smon == 0) && (tbl_sday == 0) &&
					(tbl_eyear == 99) && (tbl_emon == 99) && (tbl_eday == 99) ){	// 変更後有効期限？

					// 有効期限なし（無期限）
					tbl_sdate = 0;
					tbl_edate = 0xffff;
				}
				else{
					if( tbl_eyear >= 80 ){
						// 年が80以上入力されていたら調整する。
						tbl_eyear = 79; tbl_emon = 12; tbl_eday = 31;
	 				}
					// 有効期限あり
					if( tbl_syear >= 80 ){		// 開始年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
						tbl_syear += 1900;
					}else{
						tbl_syear += 2000;
					}
					if( tbl_eyear >= 80 ){		// 終了年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
						tbl_eyear += 1900;
					}else{
						tbl_eyear += 2000;
					}

					tbl_sdate = dnrmlzm( tbl_syear, tbl_smon, tbl_sday );		// 有効開始日変換
					tbl_edate = dnrmlzm( tbl_eyear, tbl_emon, tbl_eday );		// 有効終了日変換
				}
				CRD_DAT.SVS.std = tbl_sdate;									// 有効開始日を変更後の開始日とする
				CRD_DAT.SVS.end = tbl_edate;									// 有効終了日を変更後の終了日とする

				break;															// → 期限変更ﾃｰﾌﾞﾙ（１～３）検索終了
			}
		}

		if( CRD_DAT.SVS.std > CLK_REC.ndat ){						// 期限前
			ret = 6;												// 期限前ｴﾗｰ
			break;
		}
		if( CRD_DAT.SVS.end < CLK_REC.ndat ){						// 期限切れ
			ret = 3;												// 期限切れｴﾗｰ
			break;
		}

		if( cardknd == 12 ){
			/*** 掛売券 ***/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
			role = prm_get( COM_PRM, S_STO, (short)(1+3*(CRD_DAT.SVS.cod-1)), 1, 1 );
			chng = prm_get( COM_PRM, S_STO, (short)(3+3*(CRD_DAT.SVS.cod-1)), 2, 1 );
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
			if( !rangechk( 1, 100, CRD_DAT.SVS.cod ) ) {			// 店№範囲外?
				ret = 13;											// 種別規定外
				break;
			}
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//			if (( CPrmSS[S_STO][1+3*(CRD_DAT.SVS.cod-1)] == 0L )&&
//				( CPrmSS[S_STO][3+3*(CRD_DAT.SVS.cod-1)] == 0L ))
			if (( role == 0L )&&
			    ( chng == 0L ))
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
			{
				ret = 26;											// 役割未設定
				break;
			}

// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//			if( CPrmSS[S_STO][3+3*(CRD_DAT.SVS.cod-1)] ){			// 種別切替有り?
//				if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*(CRD_DAT.SVS.cod-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
			if( chng ){												// 種別切替有り?
				if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(chng-1)),2,5 ) == 0L ) {									// 車種設定なし?
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					ret = 27;										// 料金種別ｴﾗｰ
					break;
				}
				if(( ryo_buf.nyukin )||( ryo_buf.waribik )||		// 入金済み? or 割引済み?
				   ( e_incnt > 0 )||								// or 電子マネー使用あり?（現時点では不要）
				   ( c_pay )) {										// or ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用あり?
					ret = 30;										// 精算順序ｴﾗｰ
					break;
				}
			}
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//			if( CPrmSS[S_STO][1+3*(CRD_DAT.SVS.cod-1)] == 1L ){		// 時間割引?
			if( role == 1L ){										// 時間割引?
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
				if( card_use[USE_PPC] || card_use[USE_NUM] ||		// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ or 回数券使用済み?
					(e_incnt > 0))									// or 電子マネー使用あり?（現時点では不要）
				{
					ret = 30;										// 精算順序ｴﾗｰ
					break;
				}
			}
		}else if( cardknd == 14 ){
			/*** 割引券 ***/
			if(( CPrmSS[S_WAR][1] == 0 )||							// 割引券使用しない設定?
			   ( !rangechk( 1, 100, CRD_DAT.SVS.sts ) )||			// 割引種別範囲外?
			   (( CPrmSS[S_WAR][2+3*(CRD_DAT.SVS.sts-1)] == 0L )&&	// 未設定
			    ( CPrmSS[S_WAR][4+3*(CRD_DAT.SVS.sts-1)] == 0L )))
			{
				ret = 13;											// 種別規定外
				break;
			}
		}else{
			/*** ｻｰﾋﾞｽ券 ***/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
			role = prm_get( COM_PRM, S_SER, (short)(1+3*(CRD_DAT.SVS.knd-1)), 1, 1 );
			chng = prm_get( COM_PRM, S_SER, (short)(3+3*(CRD_DAT.SVS.knd-1)), 2, 1 );
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
			if( !rangechk( 1, SVS_MAX, CRD_DAT.SVS.knd ) ){		// ｻｰﾋﾞｽ券範囲外?
				ret = 13;										// 種別規定外
				break;
			}
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//			if (( CPrmSS[S_SER][1+3*(CRD_DAT.SVS.knd-1)] == 0L )&&
//				( CPrmSS[S_SER][3+3*(CRD_DAT.SVS.knd-1)] == 0L ))
			if (( role == 0L ) &&
				( chng == 0L ))
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
			{
				ret = 26;											// 役割未設定
				break;
			}

// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//			if( CPrmSS[S_SER][3+3*(CRD_DAT.SVS.knd-1)] ){			// 種別切替有り?
//				if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*(CRD_DAT.SVS.knd-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
			if( chng ){												// 種別切替有り?
				if( prm_get( COM_PRM,S_SHA,(short)(1+6*(chng-1)),2,5 ) == 0L ) {									// 車種設定なし?
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					ret = 27;										// 種別切替なし
					break;
				}
				if(( ryo_buf.nyukin )||( ryo_buf.waribik )||		// 入金済み? or 割引済み?
				   ( e_incnt > 0 )||								// or 電子マネー使用あり?（現時点では不要）
				   ( c_pay )) {										// or ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用あり?
					ret = 30;										// 精算順序ｴﾗｰ
					break;
				}
			}
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//			if( CPrmSS[S_SER][1+3*(CRD_DAT.SVS.knd-1)] == 1L ){		// 時間割引?
			if( role == 1L ){										// 時間割引?
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
				if( card_use[USE_PPC] || card_use[USE_NUM] ||		// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ or 回数券使用済み?
					(e_incnt > 0))									// or 電子マネー使用あり?（現時点では不要）
				{
					ret = 30;										// 精算順序ｴﾗｰ
					break;
				}
			}
			if( ryo_buf.syubet < 6 ){
				// 精算対象の料金種別A～F(0～5)
				data_adr = 2*(CRD_DAT.SVS.knd-1)+76;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)(6-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
			}
			else{
				// 精算対象の料金種別G～L(6～11)
				data_adr = 2*(CRD_DAT.SVS.knd-1)+77;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)(12-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
			}
			if( prm_get( COM_PRM, S_SER, data_adr, 1, data_pos ) ){		// 使用不可設定？
				ret = 25;												// 他の車種の券
				break;
			}
		}
		vl_now = V_SAK;												// ｻｰﾋﾞｽ券(時間/料金割引)
		break;
	}
	if( ret == 0 )
	{
		(void)vl_svschk( (m_gtservic*)mag );							// 料金計算用ｻｰﾋﾞｽ券ﾃﾞｰﾀｾｯﾄ
		card_use[USE_SVC] += 1;										// ｻｰﾋﾞｽ券,掛売券,割引券枚数+1
		card_use[USE_N_SVC] += 1;									// 新規ｻｰﾋﾞｽ券,掛売券,割引券枚数+1
		if(cardknd == 11){//サービス券
			card_use2[CRD_DAT.SVS.knd-1] += 1;									// ｻｰﾋﾞｽ券+1
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 1精算に使用したｻｰﾋﾞｽ券,掛売券,割引券の種類が超えていないかﾁｪｯｸする     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CardSyuCntChk()                                         |*/
/*| PARAMETER    : pksy  : 駐車場№                                        |*/
/*|              : cdknd : 11=ｻｰﾋﾞｽ券, 12=掛売券, 13=回数券，14=割引券     |*/
/*|              : knd   : ｻｰﾋﾞｽ券A～O (1-15, ｻｰﾋﾞｽ券以外は0)              |*/
/*|              : cod   : 店№(ｻｰﾋﾞｽ券の場合は掛売先ｺｰﾄﾞ)                 |*/
/*|              : sts   : 割引券の割引種別(割引券以外は0)                 |*/
/*| RETURN VALUE : ret   : 0 = OK                                          |*/
/*|                        1 = 1精算限度種類を超えている                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-11-29                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	CardSyuCntChk( ushort pksy, short cdknd, short knd, short cod, short sts )
{
	ushort	ret;
	uchar	i;
	uchar	tik_syu;
	ulong	ParkingNo;

	ret = 1;

// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 	if( CardUseSyu >= WTIK_USEMAX ){
	if ( CardUseSyu >= (WTIK_USEMAX - 1) ) {	// 同一割引券種数が最大値以上(制限の判定時は買物金額合計の分を除外する)
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する

		if( cdknd == 12 ){											// 掛売券
			cdknd = KAKEURI;
		}
		else if( cdknd == 13 ){										// 回数券
			cdknd = KAISUU;
		}
		else if( cdknd == 14 ){										// 割引券
			cdknd = WARIBIKI;
		}
		else if( cdknd == 15 ){										// 割引券
			cdknd = PREPAID;
		}
		else{														// ｻｰﾋﾞｽ券
			cdknd = SERVICE;
		}

		for( i=0; i<WTIK_USEMAX; i++ ){
			switch( PayData.DiscountData[i].DiscSyu ){
				case NTNET_SVS_T:
				case NTNET_SVS_M:
					tik_syu = SERVICE;
					break;
				case NTNET_KAK_T:
				case NTNET_KAK_M:
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引種類上限チェックの変更)
				case NTNET_TKAK_T:
				case NTNET_TKAK_M:
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引種類上限チェックの変更)
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// // MH810100(S) K.Onodera  2020/04/08 #4128 車番チケットレス(10種以上の読取に店割引が適用されない)
// 				case NTNET_SHOP_DISC_AMT:
// 				case NTNET_SHOP_DISC_TIME:
// // MH810100(E) K.Onodera  2020/04/08 #4128 車番チケットレス(10種以上の読取に店割引が適用されない)
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
					tik_syu = KAKEURI;
					break;
				case NTNET_FRE:
					tik_syu = KAISUU;
					break;
				case NTNET_WRI_M:
				case NTNET_WRI_T:
					tik_syu = WARIBIKI;
					break;
				case NTNET_PRI_W:
					tik_syu = PREPAID;
					break;
				default :
					tik_syu = 0xff;
					break;
			}
			ParkingNo = CPrmSS[S_SYS][pksy+1];
			if(( tik_syu == cdknd ) &&								// 券種が同じ?
			   ( PayData.DiscountData[i].ParkingNo == ParkingNo )){	// 駐車場№が同じ?
				if( cdknd == KAKEURI ){								// 掛売券
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 					if( PayData.DiscountData[i].DiscNo == (ushort)cod ){	// 店№同じ?
					if( (PayData.DiscountData[i].DiscNo == (ushort)cod) &&	// 店№
						(PayData.DiscountData[i].DiscInfo1 == (ulong)knd) ){// 割引種類/割引種別
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
						ret = 0;
						break;
					}
				}
				else if( cdknd == KAISUU ){							// 回数券
					ret = 1;
				}
				else if( cdknd == WARIBIKI ){						// 割引券
					ret = 1;
				}
				else if( cdknd == PREPAID ){						// プリペイドカード
					ret = 1;
				}
				else{												// ｻｰﾋﾞｽ券
					if( (PayData.DiscountData[i].DiscNo == (ushort)knd) &&	// A～Oが同じ
						(PayData.DiscountData[i].DiscInfo1 == (ulong)cod) ){// 掛売先ｺｰﾄﾞも同じ
						ret = 0;
						break;
					}
				}
			}
		}
	}else{
		ret = 0;
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ﾘｰﾄﾞ                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_pasck( m_servic *mag )                               |*/
/*| PARAMETER    : m_servic *mag : 磁気ﾃﾞｰﾀ格納先頭ｱﾄﾞﾚｽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = 駐車場№ｴﾗｰ                                  |*/
/*|                       2 = ﾃﾞｰﾀ異常                                     |*/
/*|                       3 = 期限切れ                                     |*/
/*|                       4 = 入出庫ｴﾗｰ                                    |*/
/*|                       5 = 無効定期                                     |*/
/*|                       6 = 期限前                                       |*/
/*|                       9 = 二重使用ｴﾗｰ                                  |*/
/*|                      13 = 種別規定外                                   |*/
/*|                      25 = 車種ｴﾗｰ                                      |*/
/*|                      26 = 設定ｴﾗｰ                                      |*/
/*|                      27 = 料金種別ｴﾗｰ                                  |*/
/*|                      29 = 問合せｴﾗｰ                                    |*/
/*|                      30 = 精算順序ｴﾗｰ                                  |*/
/*|                      99 = HOSTへ問合せ                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_pasck( m_gtapspas* mag )
{
	short	ret;
	short	wk, wksy, wkey;
	short	w_syasyu;
	ushort	us_day;
	short	s_ptbl;
	ushort	us_psts[3];
	char	c_prm;
	ulong	ul_AlmPrm;
	uchar	uc_GtAlmPrm[10];
	short	data_adr;		// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ
	char	data_pos;		// 使用可能料金種別のﾃﾞｰﾀ位置
	uchar	KigenCheckResult;
	int		i;
	char	w_cbuff[16];

	CRD_DAT.PAS.pno = 												// 駐車場№ｾｯﾄ
		(long)mag->apspas.aps_pno[0] + (long)pno_dt[mag->apspas.aps_pno[1]>>4];
	if(mag->magformat.type == 1){//GTフォーマット
		CRD_DAT.PAS.pno |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17格納
		CRD_DAT.PAS.GT_flg = 1;													//GTフォーマットフラグセット
	}else{
		CRD_DAT.PAS.GT_flg = 0;
	}
	CRD_DAT.PAS.knd = (short)(mag->apspas.aps_pno[1]&0x0f);			// 種別ｾｯﾄ
	CRD_DAT.PAS.cod =												// 個人ｺｰﾄﾞｾｯﾄ
		(short)mag->apspas.aps_pcd[1] + (((short)(mag->apspas.aps_pcd[0]))<<7);

	if( (short)prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,1 ) == 1 ){	// 期限切れ時受付する設定
		c_prm = (char)prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,2 );	// 無期限設定Get
	}else{
		c_prm = 0;
	}

	wksy = (short)mag->apspas.aps_sta[0];
	if( wksy >= 80 ){
		wksy += 1900;
	}else{
		wksy += 2000;
	}

	if( c_prm == 1 || c_prm == 2 ){									// 無期限 or 開始日無期限
		CRD_DAT.PAS.std = dnrmlzm( 1980, 3, 1 );					// 1980年3月1日
	}else{
		if( chkdate( wksy, (short)mag->apspas.aps_sta[1], (short)mag->apspas.aps_sta[2] ) ){	// 有効開始日ﾁｪｯｸNG?
			return( 2 );											// ﾃﾞｰﾀ異常
		}
		CRD_DAT.PAS.std = 											// 有効開始日ｾｯﾄ
			dnrmlzm( wksy,(short)mag->apspas.aps_sta[1],(short)mag->apspas.aps_sta[2] );
	}

	CRD_DAT.PAS.std_end[0] = mag->apspas.aps_sta[0];
	CRD_DAT.PAS.std_end[1] = mag->apspas.aps_sta[1];
	CRD_DAT.PAS.std_end[2] = mag->apspas.aps_sta[2];

	wkey = (short)mag->apspas.aps_end[0];
	if( wkey >= 80 ){
		wkey += 1900;
	}else{
		wkey += 2000;
	}
	if( c_prm == 1 || c_prm == 3 ){									// 無期限 or 終了日無期限
		CRD_DAT.PAS.end = dnrmlzm( 2079, 12, 31 );					// 2079年12月31日
	}else{
		if( chkdate( wkey, (short)mag->apspas.aps_end[1], (short)mag->apspas.aps_end[2] ) ){	// 有効終了日ﾁｪｯｸNG?
			return( 2 );											// ﾃﾞｰﾀ異常
		}
		CRD_DAT.PAS.end = 											// 有効終了日ｾｯﾄ
			dnrmlzm( wkey,(short)mag->apspas.aps_end[1],(short)mag->apspas.aps_end[2] );
	}

	CRD_DAT.PAS.std_end[3] = mag->apspas.aps_end[0];
	CRD_DAT.PAS.std_end[4] = mag->apspas.aps_end[1];
	CRD_DAT.PAS.std_end[5] = mag->apspas.aps_end[2];

	CRD_DAT.PAS.sts = mag->apspas.aps_sts;									// ｽﾃｰﾀｽｾｯﾄ
	*((long*)CRD_DAT.PAS.trz) = *((long*)mag->apspas.aps_wrt);				// 処理月日時分ｾｯﾄ

	ret = 0;
	for( ; ; ){
		// 定期券ｱﾗｰﾑ時の駐車場番号ﾃﾞｰﾀ 設定形式変更(NT-NET電文変更に伴う)
		if(mag->magformat.type == 0){
			ul_AlmPrm = (ulong)CRD_DAT.PAS.pno;
			ul_AlmPrm *= 100000L;									// 定期券駐車場set
			ul_AlmPrm += (ulong)CRD_DAT.PAS.cod;					// 定期券IDset(1～12000)
		}else{
			memset(uc_GtAlmPrm,0x00,10);						// 0クリア

			memcpy(&uc_GtAlmPrm[0],&CRD_DAT.PAS.pno,4);			// 駐車場Noセット
			memcpy(&uc_GtAlmPrm[4],&CRD_DAT.PAS.cod,2);			// 定期券IDセット
		}
		if( 0L == (ulong)CRD_DAT.PAS.pno ){
			ret = 1;												// 駐車場№ｴﾗｰ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}

		if(( prm_get( COM_PRM, S_SYS, 70, 1, 6 ) == 1 )&&					// 基本定期券使用可
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.PAS.pno )){ 				// 基本駐車場№?
			CRD_DAT.PAS.typ = KIHON_PKNO;									// 基本
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 1 ) == 1 )&&				// 拡張1定期券使用可
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.PAS.pno )){			// 拡張1駐車場№?
			CRD_DAT.PAS.typ = KAKUCHOU_1;									// 拡張1
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 2 ) == 1 )&&				// 拡張2定期券使用可
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.PAS.pno )){			// 拡張2駐車場№?
			CRD_DAT.PAS.typ = KAKUCHOU_2;									// 拡張2
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 3 ) == 1 )&&				// 拡張3定期券使用可
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.PAS.pno )){			// 拡張3駐車場№?
			CRD_DAT.PAS.typ = KAKUCHOU_3;									// 拡張3
		}
		else{
			ret = 1;														// 駐車場№ｴﾗｰ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}

		if( Ope_GT_Settei_Check( CRD_DAT.PAS.GT_flg, (ushort)CRD_DAT.PAS.typ ) == 1 ){		// 設定NGﾁｪｯｸ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_NOT_USE_TICKET, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			ret = 33;
			break;
		}
		if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){
			if( ((CRD_DAT.PAS.GT_flg == 1) && ((GTF_PKNO_LOWER > CRD_DAT.PAS.pno) || (CRD_DAT.PAS.pno > GTF_PKNO_UPPER))) ||
				((CRD_DAT.PAS.GT_flg == 0) && ((APSF_PKNO_LOWER > CRD_DAT.PAS.pno) || (CRD_DAT.PAS.pno > APSF_PKNO_UPPER))) ){	//駐車場No範囲チェック
			
				ret = 1;												// 駐車場№ｴﾗｰ
				if(mag->magformat.type == 0){
					alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
				}else{
					alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
				}
				break;
			}
		}
		s_ptbl = ReadPassTbl( (ulong)CRD_DAT.PAS.pno, CRD_DAT.PAS.cod, us_psts );
		if( s_ptbl == -1 ){											// ｴﾗｰ(数値異常)
			ret = 13;												// 種別規定外ｴﾗｰ
			break;
		}

		if( us_psts[0] ){											// 無効登録
			ret = 5;												// 無効ｴﾗｰ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_MUKOU_PASS_USE, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_MUKOU_PASS_USE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			memset(w_cbuff, 0, sizeof(w_cbuff));
			sprintf(w_cbuff, "%05d", CRD_DAT.PAS.cod );
			NgLog_write( NG_CARD_PASS, (uchar *)w_cbuff, 5 );
			break;
		}

		if( !rangechk( 1, 15, CRD_DAT.PAS.knd ) ||					// 定期種別範囲外
			!rangechk( 0, 3, CRD_DAT.PAS.sts ) )					// ｽﾃｰﾀｽ範囲外
		{
			ret = 13;												// 種別規定外ｴﾗｰ
			break;
		}
		if( ryo_buf.syubet < 6 ){
			// 精算対象の料金種別A～F(0～5)
			data_adr = 10*(CRD_DAT.PAS.knd-1)+9;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
			data_pos = (char)(6-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
		}
		else{
			// 精算対象の料金種別G～L(6～11)
			data_adr = 10*(CRD_DAT.PAS.knd-1)+10;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
			data_pos = (char)(12-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
		}
		if( prm_get( COM_PRM, S_PAS, data_adr, 1, data_pos ) ){		// 使用不可設定？
			ret = 25;												// 他の車種の券
			break;
		}

		if( PayData.teiki.id != 0 ){								// 二重使用?
			ret = 9;												// 二重使用ｴﾗｰ
			break;
		}

		if( CPrmSS[S_TIK][9] ){										// n分ﾁｪｯｸ有り?
			if( (PassIdBackupTim) &&								// n分ﾁｪｯｸ起動中?
				(PassPkNoBackup == (ulong)CRD_DAT.PAS.pno) &&
				(PassIdBackup   == (ushort)CRD_DAT.PAS.cod) ){		// 前回と同一ｶｰﾄﾞ
				ret = 9;											// 二重使用ｴﾗｰ(n分ﾁｪｯｸｴﾗｰ)
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_N_MINUTE_RULE, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_N_MINUTE_RULE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
				break;
			}
		}

		if(( mag->apspas.aps_sts == 0 )&&									// ﾊﾞｰｼﾞﾝ
		   ( CPrmSS[S_TIK][8] == 0 )){								// 初回定期読まない設定?
			ret = 26;												// 設定ｴﾗｰ
			break;
		}

		if( CRD_DAT.PAS.std > CRD_DAT.PAS.end ){					// 有効期限ﾃﾞｰﾀ異常(開始＞終了)
			ret = 13;												// 種別規定外ｴﾗｰ
			break;
		}

		/** 定期有効期限チェック処理(チェック方式をNT-7700に合わせる) **/
		KigenCheckResult = Ope_PasKigenCheck( CRD_DAT.PAS.std, CRD_DAT.PAS.end, CRD_DAT.PAS.knd, CLK_REC.ndat, CLK_REC.nmin );
											// 定期有効期限チェック（戻り：0=期限開始日より前，1=有効期限内，2=期限終了日より後）

		if( (1 != KigenCheckResult) &&								// 期限切れ
			(prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,1 ) == 0 ) ){	// 期限切れ時受付しない設定?

			/*** 期限切れで期限切れ定期受付しない設定の時 ***/
			if( 0 == KigenCheckResult ){							// 期限前
				ret = 6;											// 期限前ｴﾗｰ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_VALIDITY_TERM_OUT, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
				break;
			}
			else{													// 期限切れ
				ret = 3;											// 期限切れｴﾗｰ
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_VALIDITY_TERM_OUT, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
				break;
			}
			break;
		}

		w_syasyu = 0;
		c_prm = (char)CPrmSS[S_PAS][1+10*(CRD_DAT.PAS.knd-1)];		// 使用目的設定Get
		if( !rangechk( 1, 14, c_prm ) ){							// 定期使用目的設定範囲外
			if (c_prm == 0) {										// 使用目的：未使用
				ret = 26;											// 設定ｴﾗｰ
			}
			else {
				ret = 13;											// 種別規定外
			}
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_NOT_USE_TICKET, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}
		if( rangechk( 3, 14, c_prm ) ){								// 定期車種切換?
			w_syasyu = c_prm - 2;									// 車種切換用車種ｾｯﾄ
			if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(w_syasyu-1)),2,5 ) == 0L ) {	// 車種設定なし
				ret = 27;											// 料金種別ｴﾗｰ
				break;
			}
			if(( ryo_buf.waribik )||								// 割引済み?
			   ( ryo_buf.zankin == 0 )||							// 残金0円の時
			   ( e_incnt > 0 ))										// 電子マネー使用あり?（現時点では不要）
			{
				ret = 30;											// 精算順番ｴﾗｰ
				break;
			}
		}
		// 減額精算後の定期券は無効
		i = is_paid_remote(&PayData);
		if (i >= 0 &&
		   ( PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU || 
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
//// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
//		   ( PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE_2 && !vl_frs.antipassoff_req) || 
//// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応
		   ( PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE && !vl_frs.antipassoff_req)) ) {
			ret = 8;
			break;
		}
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
		i = is_ParkingWebFurikae( &PayData );
		if (i >= 0 &&  ( PayData.DetailData[i].DiscSyu == NTNET_FURIKAE_2 && !vl_frs.antipassoff_req) ){
			ret = 8;
			break;
		}
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応

		if(	(OPECTL.Pay_mod == 2)&&						// 修正精算
			(vl_frs.antipassoff_req)&&					// 修正元定期券使用
			(prm_get(COM_PRM, S_TYP, 98, 1, 4)==1)){	// 同一定期ﾁｪｯｸする
			if( syusei[vl_frs.lockno-1].tei_id != CRD_DAT.PAS.cod ){
				ret = 13;											// 種別規定外
				break;
			}
		}

		ryo_buf.pass_zero = 0;										// 定期券強制0円精算要求なし

		if( chk_for_inquiry(1) ) {									// 問合せる?
			if( !ERR_CHK[mod_ntibk][1] ){							// NTNET IBK 通信正常?
				OPECTL.ChkPassSyu = 1;								// 定期券問合せ中券種1=APS定期券
				OPECTL.ChkPassPkno = (ulong)CRD_DAT.PAS.pno;		// 定期券問合せ中駐車場№
				OPECTL.ChkPassID = CRD_DAT.PAS.cod;					// 定期券問合せ中定期券ID
				ret = 99;
				break;
			}else{
				OPECTL.ChkPassSyu = 0xfe;							// 通信不良
				wk = (short)prm_get( COM_PRM,S_NTN,36,1,2 );		// 通信不良時の設定
				if( wk == 0 ){										// NGで使用不可
					ret = 29;										// 通信不良
					break;
				}else if( wk == 1 ){								// 強制0円
					ryo_buf.pass_zero = 1;							// 定期券強制0円精算要求あり
				}
			}
		}

		if( OPECTL.ChkPassSyu == 1 ){								// 定期券問合せ後の問合せ結果受信
			memcpy( CRD_DAT.PAS.trz, &PassChk.OutTime.Mon, 4 );		// 処理月日時分を受信ﾃﾞｰﾀに更新
		}
		else if( OPECTL.ChkPassSyu == 0xff ){						// 定期券問合せ後の問合せ結果受信ﾀｲﾑｱｳﾄ
			wk = (short)prm_get( COM_PRM,S_NTN,36,1,1 );			// 問合せ応答ﾀｲﾑｱｳﾄ時の設定
			if( wk == 0 ){											// NGで使用不可
				ret = 29;										// 応答ﾀｲﾑｱｳﾄ
				break;
			}else if( wk == 1 ){									// 強制0円
				ryo_buf.pass_zero = 1;								// 定期券強制0円精算要求あり
			}
		}

		if(( DO_APASS_CHK )&&
		   ( prm_get( COM_PRM,S_PAS,(short)(2+10*(CRD_DAT.PAS.knd-1)),1,1 ) )&&	// 入出ﾁｪｯｸする設定?
		   ( OPECTL.ChkPassSyu != 0xff )&&							// 定期券問合せ後の問合せ結果受信ﾀｲﾑｱｳﾄ以外
		   ( OPECTL.ChkPassSyu != 0xfe )){							// 通信不良
			if( mag->apspas.aps_sts != 0 ){								// ﾊﾞｰｼﾞﾝでない?
				wk = CLK_REC.year;
				us_day = dnrmlzm( (short)wk,(short)CRD_DAT.PAS.trz[0],(short)CRD_DAT.PAS.trz[1] );	// ｶｰﾄﾞ月日ﾉｰﾏﾗｲｽﾞ

				if( us_day > CLK_REC.ndat ){
					wk--;											// 年ﾏｲﾅｽ
					us_day = dnrmlzm( (short)wk,(short)CRD_DAT.PAS.trz[0],(short)CRD_DAT.PAS.trz[1] );	// ｶｰﾄﾞ月日ﾉｰﾏﾗｲｽﾞ
				}
				wk = dnrmlzm( car_in_f.year, (short)car_in_f.mon, (short)car_in_f.day );

				if( us_day > wk ){									// ｶｰﾄﾞ月日 > 入庫月日?
					ret = 4;										// 入出ｴﾗｰ
					break;
				}
				if( us_day == wk ){
					wk = tnrmlz( 0, 0, (short)car_in_f.hour, (short)car_in_f.min );

					us_day = tnrmlz( 0, 0, (short)CRD_DAT.PAS.trz[2], (short)CRD_DAT.PAS.trz[3] );
					if( us_day > wk ){
						ret = 4;									// 入出ｴﾗｰ
						break;
					}
				}
			}
		}
		if( KaisuuWaribikiGoukei || c_pay || PayData_Sub.pay_ryo ){ // ﾌﾟﾘﾍﾟ・回数券使用時は定期券の使用を不可とする
			ret = 9;												// 二重使用ｴﾗｰ
		}
		break;
	}

	if( ret == 4 ){													// 入出ｴﾗｰ?
		for( wk=0; wk<TKI_CYUSI_MAX; wk++ ){
			if( CRD_DAT.PAS.cod == tki_cyusi.dt[wk].no &&			// 中止ﾃﾞｰﾀと一致?
				(ulong)CRD_DAT.PAS.pno == tki_cyusi.dt[wk].pk ){
				tkcyu_ichi = (char)(wk + 1);						// 中止読取位置
				ret = 0;											// ﾁｪｯｸOK
				break;
			}
		}
		if( ret == 4 ){
			if( chk_for_inquiry(2) ) {								// ｱﾝﾁﾊﾟｽNGの時のみ問合せる?
				if( !ERR_CHK[mod_ntibk][1] ){						// NTNET IBK 通信正常?
					OPECTL.ChkPassSyu = 1;							// 定期券問合せ中券種1=APS定期券
					OPECTL.ChkPassPkno = (ulong)CRD_DAT.PAS.pno;	// 定期券問合せ中駐車場№
					OPECTL.ChkPassID = CRD_DAT.PAS.cod;				// 定期券問合せ中定期券ID
					// ここでは送信要求をかけない。opemainのELE_EVT_STOPで電子媒体の停止を待ってから送信する
					ret = 99;
				}else{
					wk = (short)prm_get( COM_PRM,S_NTN,36,1,2 );	// 通信不良時の設定
					if( wk == 0 ){									// NGで使用不可
						ret = 29;									// 通信不良
					}else if( wk == 1 ){							// 強制0円
						ryo_buf.pass_zero = 1;						// 定期券強制0円精算要求あり
						ret = 0;
					}else if( wk == 2 ){							// ｶｰﾄﾞ時刻で精算
						ret = 0;
					}
				}
			}
			if( ret == 4){		//入出ｴﾗｰ?
			if(mag->magformat.type == 0){
				alm_chk2( ALMMDL_SUB2, ALARM_ANTI_PASS_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_ANTI_PASS_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			}
		}
	}

	if( ret != 99 ){												// 問合せしていない?
		OPECTL.ChkPassSyu = 0;										// 定期券問合せ中券種ｸﾘｱ
		OPECTL.ChkPassPkno = 0L;									// 定期券問合せ中駐車場№ｸﾘｱ
		OPECTL.ChkPassID = 0L;										// 定期券問合せ中定期券IDｸﾘｱ
		blink_end();												// 点滅終了
	}

	if( ret == 0 ){													// ｶｰﾄﾞOK?
		if( w_syasyu ){
			vl_now = V_SYU;											// 種別切換
			syashu = (char)w_syasyu;								// 車種
		}else{														// 車種切換でない
			vl_now = V_TSC;											// 定期券(駐車券併用有り)
		}
		vl_paschg();

		InTeiki_PayData_Tmp.syu 		 = (uchar)CRD_DAT.PAS.knd;	// 定期券種別
		InTeiki_PayData_Tmp.status 		 = (uchar)CRD_DAT.PAS.sts;	// 定期券ｽﾃｰﾀｽ(読取時)
		InTeiki_PayData_Tmp.id 			 = CRD_DAT.PAS.cod;			// 定期券id
		InTeiki_PayData_Tmp.pkno_syu 	 = CRD_DAT.PAS.typ;			// 定期券駐車場Ｎｏ．種別 (0-3:基本,拡張1-3)
		InTeiki_PayData_Tmp.update_mon 	 = 0;						// 更新月数
		InTeiki_PayData_Tmp.s_year 		 = wksy;					// 有効期限（開始：年）

		InTeiki_PayData_Tmp.s_mon 		 = mag->apspas.aps_sta[1];			// 有効期限（開始：月）
		InTeiki_PayData_Tmp.s_day 		 = mag->apspas.aps_sta[2];			// 有効期限（開始：日）
		InTeiki_PayData_Tmp.e_year 		 = wkey;					// 有効期限（終了：年）
		InTeiki_PayData_Tmp.e_mon 		 = mag->apspas.aps_end[1];			// 有効期限（終了：月）
		InTeiki_PayData_Tmp.e_day 		 = mag->apspas.aps_end[2];			// 有効期限（終了：日）
		memcpy( InTeiki_PayData_Tmp.t_tim, CRD_DAT.PAS.trz, 4 );	// 処理月日時分
		InTeiki_PayData_Tmp.update_rslt1 = 0;						// 定期更新精算時の更新結果			（　OK：更新成功　／　NG：更新失敗　）
		InTeiki_PayData_Tmp.update_rslt2 = 0;						// 定期更新精算時のラベル発行結果	（　OK：正常発行　／　NG：発行不良　）
	}
	return( ret );

}

/*[]----------------------------------------------------------------------[]*/
/*| 定期ﾃﾞｰﾀを精算ﾃﾞｰﾀｴﾘｱにｾｯﾄ                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*|	al_pasck() で 有効定期と判断した後にCallすること					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_pasck_set()			                               |*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : 100 = OK（期限切れ間近：3日以内）					   |*/
/*|					 0 = OK  期限切れ間近ではない						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2005-11-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_pasck_set( void )
{

	memcpy( &PayData.teiki, &InTeiki_PayData_Tmp, sizeof(teiki_use) );
	NTNET_Data152Save((void *)(&PayData.teiki.id), NTNET_152_TEIKIID);
	NTNET_Data152Save((void *)(&PayData.teiki.ParkingNo), NTNET_152_PARKNOINPASS);
	NTNET_Data152Save((void *)(&PayData.teiki.pkno_syu), NTNET_152_PKNOSYU);
	NTNET_Data152Save((void *)(&PayData.teiki.syu), NTNET_152_TEIKISYU);
	PassIdBackup = (ushort)PayData.teiki.id;							// 定期券idﾊﾞｯｸｱｯﾌﾟ(n分ﾁｪｯｸ用)
	PassIdBackupTim = (ulong)( CPrmSS[S_TIK][9] * 60 * 2 );			// n分ﾁｪｯｸ用ﾀｲﾏｰ起動(500ms Timer)
	PassPkNoBackup = (ulong)CRD_DAT.PAS.pno;						// 定期券駐車場番号ﾊﾞｯｸｱｯﾌﾟ(n分ﾁｪｯｸ用)
	OPECTL.PassNearEnd = vl_paschg();
	card_use[USE_PAS] += 1;											// 定期券枚数+1
	return;
}

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| 定期券ﾘｰﾄﾞ(更新用)                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_pasck_renewal( m_servic *mag )                       |*/
/*| PARAMETER    : m_servic *mag : 磁気ﾃﾞｰﾀ格納先頭ｱﾄﾞﾚｽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = 駐車場№ｴﾗｰ                                  |*/
/*|                       2 = ﾃﾞｰﾀ異常                                     |*/
/*|                       3 = 期限切れ                                     |*/
/*|                       5 = 無効定期                                     |*/
/*|                       6 = 期限前                                       |*/
/*|                      13 = 種別規定外                                   |*/
/*|                      50 = 更新期間外ｴﾗｰ                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	al_pasck_renewal( m_apspas* mag )
{
	short	ret;
	short	wksy, wkey;
	ushort	us_day;
	short	s_ptbl;
	ushort	us_psts[3];
	ushort	us_prm;

	uchar	r_sts;
	ushort	e_ymd[3];


	CRD_DAT.PAS.pno = 												// 駐車場№ｾｯﾄ
		(short)mag->aps_pno[0] + (short)pno_dt[mag->aps_pno[1]>>4];
	CRD_DAT.PAS.knd = (short)(mag->aps_pno[1]&0x0f);				// 種別ｾｯﾄ
	CRD_DAT.PAS.cod =												// 個人ｺｰﾄﾞｾｯﾄ
		(short)mag->aps_pcd[1] + (((short)(mag->aps_pcd[0]))<<7);


	wksy = (short)mag->aps_sta[0];
	if( wksy >= 80 ){
		wksy += 1900;
	}else{
		wksy += 2000;
	}
	if( chkdate( wksy, (short)mag->aps_sta[1], (short)mag->aps_sta[2] ) ){	// 有効開始日ﾁｪｯｸNG?
		return( 2 );												// ﾃﾞｰﾀ異常
	}
	CRD_DAT.PAS.std = dnrmlzm( wksy,(short)mag->aps_sta[1],(short)mag->aps_sta[2] );	// 有効開始日ｾｯﾄ

	CRD_DAT.PAS.std_end[0] = mag->aps_sta[0];
	CRD_DAT.PAS.std_end[1] = mag->aps_sta[1];
	CRD_DAT.PAS.std_end[2] = mag->aps_sta[2];

	wkey = (short)mag->aps_end[0];
	if( wkey >= 80 ){
		wkey += 1900;
	}else{
		wkey += 2000;
	}
	if( chkdate( wkey, (short)mag->aps_end[1], (short)mag->aps_end[2] ) ){	// 有効終了日ﾁｪｯｸNG?
		return( 2 );												// ﾃﾞｰﾀ異常
	}

	e_ymd[0] = wkey;
	e_ymd[1] = (ushort)mag->aps_end[1];
	e_ymd[2] = (ushort)mag->aps_end[2];

	CRD_DAT.PAS.end = dnrmlzm( wkey,(short)mag->aps_end[1],(short)mag->aps_end[2] );	// 有効終了日ｾｯﾄ

	CRD_DAT.PAS.std_end[3] = mag->aps_end[0];
	CRD_DAT.PAS.std_end[4] = mag->aps_end[1];
	CRD_DAT.PAS.std_end[5] = mag->aps_end[2];

	CRD_DAT.PAS.sts = mag->aps_sts;									// ｽﾃｰﾀｽｾｯﾄ
	*((long*)CRD_DAT.PAS.trz) = *((long*)mag->aps_wrt);				// 処理月日時分ｾｯﾄ

	ret = 0;
	for( ; ; ){
		if( 0L == (ulong)CRD_DAT.PAS.pno ){
			ret = 1;												// 駐車場№ｴﾗｰ
			break;
		}

		if(( prm_get( COM_PRM, S_SYS, 70, 1, 6 ) == 1 )&&					// 基本定期券使用可
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.PAS.pno )){ 				// 基本駐車場№?
			CRD_DAT.PAS.typ = 0;											// 基本
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 1 ) == 1 )&&				// 拡張1定期券使用可
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.PAS.pno )){			// 拡張1駐車場№?
			CRD_DAT.PAS.typ = 1;											// 拡張1
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 2 ) == 1 )&&				// 拡張2定期券使用可
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.PAS.pno )){			// 拡張2駐車場№?
			CRD_DAT.PAS.typ = 2;											// 拡張2
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 3 ) == 1 )&&				// 拡張3定期券使用可
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.PAS.pno )){			// 拡張3駐車場№?
			CRD_DAT.PAS.typ = 3;											// 拡張3
		}
		else{
			ret = 1;														// 駐車場№ｴﾗｰ
			break;
		}

		if( prm_get( COM_PRM,S_KOU,4,1,1 ) ){						// 無効登録ﾁｪｯｸ(定期ｽﾃｰﾀｽﾃｰﾌﾞﾙ)する?
			s_ptbl = ReadPassTbl( (ushort)CRD_DAT.PAS.pno, CRD_DAT.PAS.cod, us_psts );
			if( s_ptbl == -1 ){										// ｴﾗｰ(数値異常)
				ret = 13;											// 種別規定外ｴﾗｰ
				break;
			}
			if( us_psts[0] ){										// 無効登録
				ret = 5;											// 無効ｴﾗｰ
				break;
			}
		}

		if( !rangechk( 1, 15, CRD_DAT.PAS.knd ) )					// 定期種別範囲外
		{
			ret = 13;												// 種別規定外ｴﾗｰ
			break;
		}

		r_sts = (uchar)prm_get( COM_PRM,S_KOU,5,2,1 );
		if( rangechk( 0, 3, r_sts ) ){								// 読取ｽﾃｰﾀｽ設定が0～3：例外ｽﾃｰﾀｽなし
			if( !rangechk( 0, 3, CRD_DAT.PAS.sts ) ){				// ｽﾃｰﾀｽ範囲外
				ret = 13;											// 種別規定外ｴﾗｰ
				break;
			}
		}else{														// 読取ｽﾃｰﾀｽ設定が0～3以外：例外あり
			if( !rangechk( 0, 3, CRD_DAT.PAS.sts ) &&				// ｽﾃｰﾀｽ範囲外
				( r_sts != CRD_DAT.PAS.sts ) )						// 設定した例外ｽﾃｰﾀｽと異なる
			{
				ret = 13;											// 種別規定外ｴﾗｰ
				break;
			}
		}

		if( CRD_DAT.PAS.std > CRD_DAT.PAS.end ){					// 有効期限ﾃﾞｰﾀ異常(開始＞終了)
			ret = 13;												// 種別規定外ｴﾗｰ
			break;
		}

		if( prm_get( COM_PRM,S_KOU,3,2,5 ) ){						// 更新期間の方法
			// 指定日から定期終了日＋x日
			us_prm = (ushort)prm_get( COM_PRM,S_KOU,3,2,3 );		// 指定日
			us_day = dnrmlzm( (short)CLK_REC.year,(short)CLK_REC.mont,(short)us_prm );	// 指定日をﾉｰﾏﾗｲｽﾞ

			if( CLK_REC.ndat < CRD_DAT.PAS.end ){					// 現在日が有効終了日より前

				// 更新期限前ﾁｪｯｸ
				if( CLK_REC.year != e_ymd[0] ||						// 現在年月と有効終了年月が異なる
					CLK_REC.mont != e_ymd[1] ){
					ret = 50;										// 更新期間外ｴﾗｰ
					break;
				}

				// 更新期限前ﾁｪｯｸ
				if( CRD_DAT.PAS.end < us_day ){						// 指定日より終了日が後なら更新可能
					ret = 50;										// 更新期間外ｴﾗｰ
					break;
				}

				// 更新期限前ﾁｪｯｸ
				if( CLK_REC.ndat < us_day ){						// 有効終了日からxx日前なら更新可能
					ret = 50;										// 更新期間外ｴﾗｰ
					break;
				}
			}
		}else{
			// 定期券終了日から前後の指定した日数
			us_prm = (ushort)prm_get( COM_PRM,S_KOU,3,2,3 );		// 更新可能日数(前)
			if( CRD_DAT.PAS.end >= us_prm ){
				us_day = CRD_DAT.PAS.end - us_prm;
			}else{
				us_day = 0;
			}

			// 更新期限前ﾁｪｯｸ
			if( CLK_REC.ndat < us_day ){							// 有効終了日からxx日前なら更新可能
				ret = 50;											// 更新期間外ｴﾗｰ
				break;
			}
		}

		if(( CRD_DAT.PAS.std > CLK_REC.ndat )||						// 期限切れ(開始＞現在)
		   ( CRD_DAT.PAS.end < CLK_REC.ndat )){						// 期限切れ(終了＜現在)
			/*** 期限切れ定期受付しない ***/
			if( CRD_DAT.PAS.std > CLK_REC.ndat ){					// 期限前
				ret = 6;											// 期限前ｴﾗｰ
				break;
			}
			if( CRD_DAT.PAS.end < CLK_REC.ndat ){					// 期限切れ
				// 更新期限後ﾁｪｯｸ
				us_day = CRD_DAT.PAS.end + (ushort)prm_get( COM_PRM,S_KOU,3,2,1 );	// 更新可能日数(後) or 猶予日数

				if( CLK_REC.ndat > us_day ){						// 有効終了日からxx日後なら更新可能
					ret = 6;										// 更新期限外ｴﾗｰ
					break;
				}
			}
		}

		if( !CPrmSS[S_KOU][7] ){									// 更新ﾎﾞﾀﾝを使用しない
			us_prm = (ushort)CPrmSS[S_KOU][34+10*(CRD_DAT.PAS.knd-1)];	// 使用目的設定Get
			if( us_prm == 0 ){										// 更新設定 未設定
				ret = 13;											// 種別規定外
				break;
			}
		}

		ryo_buf.pass_zero = 0;										// 定期券強制0円精算要求なし

		break;
	}

	if( ret == 0 ){													// ｶｰﾄﾞOK?
		OPECTL.PassNearEnd = 0;
		vl_paschg();
		RenewalMonth = (ushort)CPrmSS[S_KOU][34+10*(CRD_DAT.PAS.knd-1)];	// 定期券更新月数ｾｯﾄ
		RenewalFee = (ulong)CPrmSS[S_KOU][30+10*(CRD_DAT.PAS.knd-1)];	// 定期券更新料金ｾｯﾄ
		PayData.teiki.syu = (uchar)CRD_DAT.PAS.knd;					// 定期券種別
		PayData.teiki.status = (uchar)CRD_DAT.PAS.sts;				// 定期券ｽﾃｰﾀｽ(読取時)
		PayData.teiki.id = CRD_DAT.PAS.cod;							// 定期券id
		PayData.teiki.pkno_syu = CRD_DAT.PAS.typ;					// 定期券駐車場Ｎｏ．種別
		PayData.teiki.update_mon = (uchar)RenewalMonth;				// 更新月数
		PayData.teiki.s_year = wksy;								// 有効期限（開始：年）
		PayData.teiki.s_mon = mag->aps_sta[1];						// 有効期限（開始：月）
		PayData.teiki.s_day = mag->aps_sta[2];						// 有効期限（開始：日）
		PayData.teiki.e_year = wkey;								// 有効期限（終了：年）
		PayData.teiki.e_mon = mag->aps_end[1];						// 有効期限（終了：月）
		PayData.teiki.e_day = mag->aps_end[2];						// 有効期限（終了：日）
		date_renewal( RenewalMonth, &PayData.teiki.e_year, &PayData.teiki.e_mon, &PayData.teiki.e_day );	// 有効期限（終了日）更新
		memcpy( PayData.teiki.t_tim, CRD_DAT.PAS.trz, 4 );			// 処理月日時分
		PayData.teiki.update_rslt1 = 0;								// 定期更新精算時の更新結果			（　OK：更新成功　／　NG：更新失敗　）
		PayData.teiki.update_rslt2 = 0;								// 定期更新精算時のラベル発行結果	（　OK：正常発行　／　NG：発行不良　）
	}
	return( ret );

}

/*[]----------------------------------------------------------------------[]*/
/*| 日をXXヶ月更新処理                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : date_renewal()                                          |*/
/*| PARAMETER    : renw : 更新する月数                                     |*/
/*|                us_y : 更新する年のﾎﾟｲﾝﾀ                                |*/
/*|                uc_m : 更新する月のﾎﾟｲﾝﾀ                                |*/
/*|                uc_d : 更新する日のﾎﾟｲﾝﾀ                                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	date_renewal( ushort renw, ushort *us_y, uchar *uc_m, uchar *uc_d )
{
	ushort	us_wk;
	short	s_wky, s_wkm, s_wkd, s_wkd2;

	s_wky = (short)*us_y;
	s_wkm = (short)*uc_m;
	s_wkd = (short)*uc_d;


	if( (ushort)CPrmSS[S_KOU][2] == 0 ){							// 定期券更新計算方法
		// 定期券更新計算方法：有効終了日 ＋ １日 ＋ 更新月数 － １日
		us_wk = dnrmlzm( s_wky, s_wkm, s_wkd );						// 有効終了日ﾉｰﾏﾗｲｽﾞ
		us_wk++;													// +1日更新
		idnrmlzm( us_wk, &s_wky, &s_wkm, &s_wkd );					// 逆ﾉｰﾏﾗｲｽﾞし1日更新した日付を求める

		// 月を更新
		if( ( s_wkm + renw ) > 12 ){
			// 更新後12月を超えてしまう場合は年も更新する
			s_wkm = ( s_wkm + renw ) - 12;							// 月更新
			s_wky++;												// 年更新
		}else{
			s_wkm += renw;											// 月更新
		}

		s_wkd2 = medget( s_wky, s_wkm );							// 月の最終日ﾁｪｯｸ
		if( s_wkd2 < s_wkd ){
			// 1ヶ月更新した日付がその月でありえない日付（その月の最終日を越えている）の時
			// 更新日はその月の最終日とする。
			s_wkd = s_wkd2;
		}else{
			// 1ヶ月更新した日付が正しければ、本来の計算式（-1日）を継続して行い、更新日を算出する
			us_wk = dnrmlzm( s_wky, s_wkm, s_wkd );					// 月を更新したものを再度ﾉｰﾏﾗｲｽﾞ
			us_wk--;												// -1日更新

			idnrmlzm( us_wk, &s_wky, &s_wkm, &s_wkd );				// 逆ﾉｰﾏﾗｲｽﾞし-1日更新した日付を求める
		}
	}else{
		// 定期券更新計算方法：有効終了日 ＋ 更新月数

		// 月を更新
		if( ( s_wkm + renw ) > 12 ){
			// 更新後12月を超えてしまう場合は年も更新する
			s_wkm = ( s_wkm + renw ) - 12;							// 月更新
			s_wky++;												// 年更新
		}else{
			s_wkm += renw;											// 月更新
		}
		if( (ushort)CPrmSS[S_KOU][2] == 1 ){
			// あまった日は翌月へ繰越設定

			// 一回ﾉｰﾏﾗｲｽﾞして戻すことによりあまった日付を補正する
			us_wk = dnrmlzm( s_wky, s_wkm, s_wkd );					// ﾉｰﾏﾗｲｽﾞ
			idnrmlzm( us_wk, &s_wky, &s_wkm, &s_wkd );				// 逆ﾉｰﾏﾗｲｽﾞ
		}else{
			// あまった日は翌月へ繰越さない設定

			// 更新後の日が更新後の月の最終日を超えている場合は日を月の最終日に補正する。
			s_wkd2 = medget( s_wky, s_wkm );						// 月の最終日ﾁｪｯｸ
			if( s_wkd > s_wkd2 ){
				s_wkd = s_wkd2;
			}
		}
	}

	*us_y = (ushort)s_wky;
	*uc_m = (uchar)s_wkm;
	*uc_d = (uchar)s_wkd;
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetPrcKigenStr										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : "有効期限　　ＸＸＸＸ年ＸＸ月ＸＸ日"　文字列作成        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : mag(in) プリペイド券データ                              |*/
/*|				   PccUkoKigenStr(out) 文字列<NULL> 31バイト以上のｴﾘｱがある前提 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Okamoto(COSMO)                                        |*/
/*| Date         : 2011-07-20                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	GetPrcKigenStr  ( char *PccUkoKigenStr )
{
	char	swork[10];

	memcpy ( PccUkoKigenStr, OPE_CHR[142], 31 );
	sprintf( swork, "%04d", CRD_DAT.PRE.kigen_year );
	as1chg ( (unsigned char*)swork, (unsigned char*)&PccUkoKigenStr[8], 4 );	// "有効期限２０１１年    月    日"
	sprintf( swork, "%2d", CRD_DAT.PRE.kigen_mon );
	as1chg ( (unsigned char*)swork, (unsigned char*)&PccUkoKigenStr[18], 2 );	// "有効期限２０１１年　７月    日"
	sprintf( swork, "%2d", CRD_DAT.PRE.kigen_day );
	as1chg ( (unsigned char*)swork, (unsigned char*)&PccUkoKigenStr[24], 2 );	// "有効期限２０１１年　７月２０日"
}
//--------------------------------------------------------------------------------------------------------
// プリペイドカード磁気データの販売日に、有効期間（ｎヶ月）を加算し－１日した年月日(有効期限日）
// を算出し、出力する.
//
// 算出例:有効期間が１ヶ月の場合
//
//　　　販売日：2011年3月1日		有効期限日：2011年3月31日
//　　　販売日：2011年3月15日		有効期限日：2011年4月14日
//　　　販売日：2011年2月28日		有効期限日：2011年3月27日
//　　　販売日：2011年1月31日		有効期限日：2011年2月28日
//　　　販売日：2012年1月31日		有効期限日：2012年2月29日
//
// Param:
//  mag(in) : ﾘｰﾄﾞ磁気ﾃﾞｰﾀ
//  clk_Kigen(out) : 有効期限日
//
// Return: 0
//
//--------------------------------------------------------------------------------------------------------
int		al_preck_Kigen ( m_gtprepid* mag, struct clk_rec *clk_Kigen )
{
	struct	clk_rec		clk_date;
	unsigned short		us1;

	us1 = (unsigned short)prm_get( COM_PRM,S_PRP,11,2,1 );

	clk_date.year = astoin( &mag->prepid.pre_sta[0], 2 );
	clk_date.mont = (unsigned char)astoin( &mag->prepid.pre_sta[2], 2 );
	clk_date.date = (unsigned char)astoin( &mag->prepid.pre_sta[4], 2 );
	clk_date.hour = 0;
	clk_date.minu = 0;

	if ( clk_date.year >= 80 ){
		clk_date.year += 1900;
	}else{
		clk_date.year += 2000;
	}

	if ( us1 == 0 ){
		us1 = 6;			//6ヶ月.
	}
	add_month ( &clk_date, us1 );	//nヶ月加算 -1日//

	memcpy ( clk_Kigen, &clk_date, sizeof(struct clk_rec));

	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞﾘｰﾄﾞ                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_preck( m_servic *mag )                               |*/
/*| PARAMETER    : m_servic *mag : 磁気ﾃﾞｰﾀ格納先頭ｱﾄﾞﾚｽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = 駐車場№ｴﾗｰ                                  |*/
/*|                       2 = ﾃﾞｰﾀ異常                                     |*/
/*|						  3 = 有効期限切れ								   |*/
/*|                       7 = 残額ｾﾞﾛ                                      |*/
/*|                      10 = 表裏ｴﾗｰ                                      |*/
/*|                      13 = 種別規定外                                   |*/
/*|                      26 = 設定ｴﾗｰ                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_preck( m_gtprepid* mag )
{
	short	ret, wk;
	short	data[3];
	long temp;
	ushort	pkno_syu;
	struct	clk_rec		clk_wok;
	unsigned short		us1;

	if(mag->magformat.type == 1){
		temp  =  (long)mag->prepid.pre_pno[0] & 0x0000003F;
		temp |= ((long)mag->prepid.pre_pno[1] & 0x0000003F) << 6;
		temp |= ((long)mag->prepid.pre_pno[2] & 0x0000003F) << 12;
		CRD_DAT.PRE.pno = temp;											// 駐車場№ｾｯﾄ
		CRD_DAT.PRE.GT_flg = 1;
	}else{
		wk = astoin( mag->prepid.pre_pno, 2 );
		wk += ( mag->prepid.pre_pno[2]&0x0f ) * 100;
		CRD_DAT.PRE.pno = (long)wk;										// 駐車場№ｾｯﾄ
		CRD_DAT.PRE.GT_flg = 0;
	}
	CRD_DAT.PRE.mno = astoin( mag->prepid.pre_mno, 2 );					// 販売機№ｾｯﾄ
	wk = (short)mag->prepid.pre_amo;
	if( wk < 0x40 ){												// "0"-"9" ?
		wk -= 0x31;
		wk += 1;
	}else if( wk < 0x5b ){											// "A"-"Z" ?
		wk -= 0x41;
		wk += 10;
	}else{															// "a"-"z"
		wk -= 0x61;
		wk += 36;
	}
	CRD_DAT.PRE.amo = wk * 1000L;									// 販売金額ｾｯﾄ
	CRD_DAT.PRE.ram = (long)astoinl( mag->prepid.pre_ram, 5 );		// 残額ｾｯﾄ
	CRD_DAT.PRE.plm = PRC_GENDOGAKU_MIN;							// 限度額ｾｯﾄ
	CRD_DAT.PRE.cno = (long)astoinl( mag->prepid.pre_cno, 5 );		// ｶｰﾄﾞ№
	if ( prm_get( COM_PRM,S_PRP,11,2,1 ) != 99 ){
		al_preck_Kigen ( (m_gtprepid*)mag, &clk_wok );		// clk_wok <-- 有効期限日
		CRD_DAT.PRE.kigen_year = clk_wok.year;
		CRD_DAT.PRE.kigen_mon  = clk_wok.mont;
		CRD_DAT.PRE.kigen_day  = clk_wok.date;
	}else{
		CRD_DAT.PRE.kigen_year = 0;
		CRD_DAT.PRE.kigen_mon  = 0;
		CRD_DAT.PRE.kigen_day  = 0;
	}
	ret = 0;
	for( ; ; ){
		if( 0L == (ulong)CRD_DAT.PRE.pno ){
			ret = 1;												// 駐車場№ｴﾗｰ
			break;
		}

		if(( prm_get( COM_PRM, S_SYS, 72, 1, 6 ) == 1 )&&					// 基本プリペイド使用可
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.PRE.pno )){ 				// 基本駐車場№?
			pkno_syu = KIHON_PKNO;									// 基本
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 1 ) == 1 )&&				// 拡張1プリペイド使用可
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.PRE.pno )){			// 拡張1駐車場№?
			pkno_syu = KAKUCHOU_1;									// 拡張1
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 2 ) == 1 )&&				// 拡張2プリペイド使用可
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.PRE.pno )){			// 拡張2駐車場№?
			pkno_syu = KAKUCHOU_2;									// 拡張2
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 3 ) == 1 )&&				// 拡張3プリペイド使用可
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.PRE.pno )){			// 拡張3駐車場№?
			pkno_syu = KAKUCHOU_3;									// 拡張3
		}
		else{
			ret = 1;														// 駐車場№ｴﾗｰ
			break;
		}
		
		if( Ope_GT_Settei_Check( CRD_DAT.PRE.GT_flg, pkno_syu ) == 1 ){		// 設定NGﾁｪｯｸ
			ret = 33;
			break;
		}

		if( CPrmSS[S_PRP][1] != 1 ){								// ﾌﾟﾘﾍﾟｲﾄﾞ使用不可設定
			ret = 26;												// 設定ｴﾗｰ
			break;
		}
		
		if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){
			if( ((CRD_DAT.PRE.GT_flg == 1) && ((GTF_PKNO_LOWER > CRD_DAT.PRE.pno) || (CRD_DAT.PRE.pno > GTF_PKNO_UPPER))) ||
				((CRD_DAT.PRE.GT_flg == 0) && ((APSF_PKNO_LOWER > CRD_DAT.PRE.pno) || (CRD_DAT.PRE.pno > APSF_PKNO_UPPER))) ){			//駐車場No範囲チェック
				ret = 1;												// 駐車場№ｴﾗｰ
				break;
			}
		}

		if( CRD_DAT.PRE.ram == 0L ){
			ret = 7;												// 残額ｾﾞﾛ
			break;
		}

		if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
			if( CardSyuCntChk( pkno_syu, 15, 0, 0, 0 ) ){			// 1精算の割引種類の件数ｵｰﾊﾞｰ?
				ret = 14;											// 限度枚数ｵｰﾊﾞｰ
				break;
			}
		}

		if( RD_pos > 1 ){											// 表裏ｴﾗｰ?
			ret = 10;												// 表裏ｴﾗｰ
			break;
		}

		data[0] = astoin( &mag->prepid.pre_sta[0], 2 );				// 販売年
		if( data[0] >= 80 ){
			data[0] += 1900;
		}else{
			data[0] += 2000;
		}
		data[1] = astoin( &mag->prepid.pre_sta[2], 2 );				// 販売月
		data[2] = astoin( &mag->prepid.pre_sta[4], 2 );				// 販売日
		if( chkdate( data[0], data[1], data[2] ) ){					// 販売日ﾁｪｯｸNG?
			ret = 2;												// ﾃﾞｰﾀ異常
			break;
		}
		if( dnrmlzm( data[0], data[1], data[2] ) > CLK_REC.ndat ){	// 販売日 > 現在日
			ret = 13;												// 種別規定外
			break;
		}
		vl_now = V_PRI;												// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ
		break;
	}
	if ( (ret == 0) &&
		 (prm_get( COM_PRM,S_PRP,11,2,1 ) != 99) ){

		us1 = dnrmlzm( (short)CRD_DAT.PRE.kigen_year, (short)CRD_DAT.PRE.kigen_mon, (short)CRD_DAT.PRE.kigen_day );
		if ( us1 < CLK_REC.ndat ){
			ret = 3;
		}
	}
	if( ret == 0 )
	{
		(void)vl_prechg( (m_gtprepid*)mag );
		card_use[USE_PPC] += 1;										// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ枚数+1
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 回数券ﾘｰﾄﾞ＆チェック                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_kasck( m_servic *mag )                               |*/
/*| PARAMETER    : m_servic *mag : 磁気ﾃﾞｰﾀ格納先頭ｱﾄﾞﾚｽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = 駐車場№ｴﾗｰ                                  |*/
/*|                       2 = 種別ｴﾗｰ                                      |*/
/*|                       3 = 期限切れ                                     |*/
/*|                       7 = 残額ｾﾞﾛ                                      |*/
/*|                      10 = 表裏ｴﾗｰ                                      |*/
/*|                      13 = 種別規定外                                   |*/
/*|                      14 = 限度枚数ｵｰﾊﾞｰ                                |*/
/*|                      26 = 設定ｴﾗｰ                                      |*/
/*|						100 = 期限切れ間近（3日以内）					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_kasck( m_gtservic *mag )
{
	short	ret, wk;
	ushort	pkno_syu;

	CRD_DAT.SVS.pno = 												// 駐車場№ｾｯﾄ
		(long)mag->servic.svc_pno[0] + (long)pno_dt[mag->servic.svc_pno[1]>>4];
	if(mag->magformat.type == 1){//GTフォーマット
		CRD_DAT.SVS.pno |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17格納
		CRD_DAT.SVS.GT_flg = 1;													//GTフォーマットフラグセット
	}else{
		CRD_DAT.SVS.GT_flg = 0;													//GTフォーマットフラグセット	
	}
	CRD_DAT.SVS.knd = (short)( mag->servic.svc_pno[1] & 0x0f );			// 制限度数ｾｯﾄ
	CRD_DAT.SVS.cod =												// 単位金額ｾｯﾄ
		(short)mag->servic.svc_sno[1] + (((short)mag->servic.svc_sno[0])<<7);
	CRD_DAT.SVS.sts = (short)( mag->servic.svc_sts );						// 残り回数

	if( memcmp( mag->servic.svc_sta, ck_dat, 6 ) == 0 ){					// 無期限
		CRD_DAT.SVS.std = 0;										// 有効開始日ｾｯﾄ
		CRD_DAT.SVS.end = 0xffff;									// 有効終了日ｾｯﾄ
	}else{
		wk = (short)(mag->servic.svc_sta[0]);
		if( wk >= 80 ){
			wk += 1900;
		}else{
			wk += 2000;
		}
		if( chkdate( wk, (short)mag->servic.svc_sta[1], (short)mag->servic.svc_sta[2] ) ){	// 有効開始日ﾁｪｯｸNG?
			return( 2 );											// ﾃﾞｰﾀ異常
		}
		CRD_DAT.SVS.std = 											// 有効開始日ｾｯﾄ
			dnrmlzm( wk,(short)mag->servic.svc_sta[1],(short)mag->servic.svc_sta[2] );

		wk = (short)mag->servic.svc_end[0];
		if( wk >= 80 ){
			wk += 1900;
		}else{
			wk += 2000;
		}
		if( chkdate( wk, (short)mag->servic.svc_end[1], (short)mag->servic.svc_end[2] ) ){	// 有効終了日ﾁｪｯｸNG?
			return( 2 );											// ﾃﾞｰﾀ異常
		}
		CRD_DAT.SVS.end = 											// 有効終了日ｾｯﾄ
			dnrmlzm( wk,(short)mag->servic.svc_end[1],(short)mag->servic.svc_end[2] );
	}

	/* TF-7700のチェック項目 */
	/* 	無効ﾃﾞｰﾀﾁｪｯｸ														*/
	//		if( ! rangechk(1,11,vl_kas.rim ) ||			/* 制限度数範囲外 　*/
	//			! rangechk(1,11 ,vl_kas.nno) ||			/* 残回数1-11以外   */
	//			! rangechk(0,9990,vl_kas.tnk) ) 		/* 単位金額範囲外 　*/
	/* 	駐車場NO.ﾁｪｯｸ														*/
	/* 	不正券ﾁｪｯｸ	(回数券設定有無)										*/
	/* 	挿入方向ﾁｪｯｸ														*/
	/* 	期限切れﾁｪｯｸ														*/
	/* 	期限切れ間近ﾁｪｯｸ													*/

	ret = 0;
	for( ; ; ){

		/* 駐車場番号ﾁｪｯｸ */
		if( 0L               == (ulong)CRD_DAT.SVS.pno){
			ret = 1;												// 駐車場№ｴﾗｰ
			break;
		}
		if(( prm_get( COM_PRM, S_SYS, 72, 1, 6 ) == 1 )&&			// 基本回数券券使用可
		   ( CPrmSS[S_SYS][1] == (ulong)CRD_DAT.SVS.pno )){ 		// 基本駐車場№?
			pkno_syu = KIHON_PKNO;									// 基本
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 1 ) == 1 )&&		// 拡張1回数券券使用可
				( CPrmSS[S_SYS][2] == (ulong)CRD_DAT.SVS.pno )){	// 拡張1駐車場№?
			pkno_syu = KAKUCHOU_1;									// 拡張1
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 2 ) == 1 )&&		// 拡張2回数券券使用可
				( CPrmSS[S_SYS][3] == (ulong)CRD_DAT.SVS.pno )){	// 拡張2駐車場№?
			pkno_syu = KAKUCHOU_2;									// 拡張2
		}
		else if(( prm_get( COM_PRM, S_SYS, 72, 1, 3 ) == 1 )&&		// 拡張3回数券券使用可
				( CPrmSS[S_SYS][4] == (ulong)CRD_DAT.SVS.pno )){	// 拡張3駐車場№?
			pkno_syu = KAKUCHOU_3;									// 拡張3
		}
		else{
			ret = 1;												// 駐車場№ｴﾗｰ
			break;
		}

		if( Ope_GT_Settei_Check( CRD_DAT.SVS.GT_flg, pkno_syu ) == 1 ){	// 設定NGﾁｪｯｸ
			ret = 33;
			break;
		}

		if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){
			if( ((CRD_DAT.SVS.GT_flg == 1) && ((GTF_PKNO_LOWER > CRD_DAT.SVS.pno) || (CRD_DAT.SVS.pno > GTF_PKNO_UPPER))) ||
				((CRD_DAT.SVS.GT_flg == 0) && ((APSF_PKNO_LOWER > CRD_DAT.SVS.pno) || (CRD_DAT.SVS.pno > APSF_PKNO_UPPER))) ){//駐車場No範囲チェック
				ret = 1;												// 駐車場№ｴﾗｰ
				break;
			}
		}

		/* 不正券ﾁｪｯｸ (回数券設定有無) */
		if( CPrmSS[S_PRP][1] != 2 ){								// 回数券使用不可設定
			ret = 26;												// 設定ｴﾗｰ
			break;
		}

		/* 挿入方向ﾁｪｯｸ */
		if( RD_pos > 1 ){											// 表裏ｴﾗｰ?
			ret = 10;												// 表裏ｴﾗｰ
			break;
		}

		/* 期限切れﾁｪｯｸ */
			if( CRD_DAT.SVS.std > CLK_REC.ndat ){					// 期限前
				ret = 6;
				break;
			}
			if( CRD_DAT.SVS.end < CLK_REC.ndat ){					// 期限切れ
				ret = 3;
				break;
			}

		/* 残回数 */
		if( CRD_DAT.SVS.sts == 0 ){									// 
			ret = 7;												// 残額ｾﾞﾛ
			break;
		}

		if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
			if( CardSyuCntChk( pkno_syu, 13, CRD_DAT.SVS.knd, CRD_DAT.SVS.cod, CRD_DAT.SVS.sts ) ){	// 1精算の割引種類の件数ｵｰﾊﾞｰ?
				ret = 14;											// 限度枚数ｵｰﾊﾞｰ
				break;
			}
		}

		/* 無効ﾃﾞｰﾀﾁｪｯｸ */
		if( ! rangechk( 1, 11, CRD_DAT.SVS.knd ) ||					// 制限度数範囲外
			! rangechk( 1, 11, CRD_DAT.SVS.sts ) ||					// 残回数1-11以外
			! rangechk( 0, 9990, CRD_DAT.SVS.cod) ){ 				// 単位金額範囲外
			ret = 13;												// 種別規定外
		}

		/* 料金計算要求 情報ｾｯﾄ */
		if( !CRD_DAT.SVS.cod )
			vl_now = V_KAM;											// 回数券（無料）
		else
			vl_now = V_KAG;											// 回数券（割引）
		break;
	}
	if( ret == 0 )
	{
		ret = vl_kaschg( (m_gtservic*)mag );							// 料金計算用 回数券ﾃﾞｰﾀｾｯﾄ
		card_use[USE_NUM] += 1;										// 回数券枚数+1
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券ﾗｲﾄﾃﾞｰﾀ作成                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_mkpas( inpp, ck )                                    |*/
/*| PARAMETER    : m_apspas *inpp : Read Pass Card Data                    |*/
/*|                clk_rec  *ck   : Dispence Time                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_mkpas( m_gtapspas *inpp, struct clk_rec *ck )
{
	m_apspas *outp = (m_apspas*)MDP_buf;

	memcpy( outp, &inpp->apspas.aps_idc, sizeof( m_apspas ) );
	outp->aps_sts = 1;												// 出庫中
	outp->aps_wrt[0] = ck->mont;
	outp->aps_wrt[1] = ck->date;
	outp->aps_wrt[2] = ck->hour;
	outp->aps_wrt[3] = ck->minu;
	MDP_siz = sizeof( m_apspas );
	MDP_mag = sizeof( m_apspas );
	if(inpp->magformat.type == 0){
		md_pari( &(uchar)MDP_buf[0], (ushort)MDP_siz, 0 );				// Odd Parity Set
		MDP_buf[127] = 0;		//GTフォーマット作成フラグ
	}else{
		outp->aps_pno[0] |= (uchar)((inpp->magformat.ex_pkno & 0x01) << 7);
		outp->aps_pno[1] |= (uchar)((inpp->magformat.ex_pkno & 0x02) << 6);
		outp->aps_pcd[0] |= (uchar)((inpp->magformat.ex_pkno & 0x04) << 5);
		outp->aps_pcd[1] |= (uchar)((inpp->magformat.ex_pkno & 0x08) << 4);
		outp->aps_sta[0] |= (uchar)((inpp->magformat.ex_pkno & 0x10) << 3);
		outp->aps_sta[1] |= (uchar)((inpp->magformat.ex_pkno & 0x20) << 2);
		outp->aps_sta[2] |= (uchar)((inpp->magformat.ex_pkno & 0x40) << 1);
		outp->aps_end[0] |= (uchar)(inpp->magformat.ex_pkno & 0x80);
		md_pari2( &(uchar)MDP_buf[0], (ushort)1, 1 );					// IDコードEven Parity Set
		MDP_buf[127] = 1;		//GTフォーマット作成フラグ
	}
}

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| 定期券ﾗｲﾄﾃﾞｰﾀ作成(定期券更新用)                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_mkpas_renewal( inpp, ck )                            |*/
/*| PARAMETER    : m_apspas *inpp : Read Pass Card Data                    |*/
/*|                clk_rec  *ck   : Dispence Time                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_mkpas_renewal( m_apspas *inpp, struct clk_rec *ck )
{
uchar	r_sts;

	m_apspas *outp = (m_apspas*)MDP_buf;

	memcpy( outp, inpp, sizeof( m_apspas ) );

	outp->aps_end[0] = (uchar)(PayData.teiki.e_year % 100);			// 有効期限（終了：年）
	outp->aps_end[1] = PayData.teiki.e_mon;							// 有効期限（終了：月）
	outp->aps_end[2] = PayData.teiki.e_day;							// 有効期限（終了：日）

//	outp->aps_sts = 0;												// 初回

	if( rangechk( 0, 3, outp->aps_sts ) ){							// 定期券読取時のｽﾃｰﾀｽが0～3の範囲内
		if( !prm_get( COM_PRM,S_KOU,6,2,5 ) ){						// 全て初回ｽﾃｰﾀｽに書換える
			outp->aps_sts = 0;										// 初回
		}
		// それ以外は読取時の定期券ｽﾃｰﾀｽのまま
	}else{															// 定期券読取時のｽﾃｰﾀｽが0～3以外
		r_sts = (uchar)prm_get( COM_PRM,S_KOU,5,2,1 );
		if( rangechk( 0, 3, r_sts ) ){								// 読取ｽﾃｰﾀｽ設定が0～3：例外ｽﾃｰﾀｽなし
			r_sts = 0;												// 書込ｽﾃｰﾀｽ設定での書込なしとする
		}
		if( r_sts && ( r_sts == outp->aps_sts ) ){					// 例外ありで例外ｽﾃｰﾀｽ＝定期券読取時
			outp->aps_sts = (uchar)prm_get( COM_PRM,S_KOU,6,2,1 );	// 例外ｽﾃｰﾀｽの時の書込ｽﾃｰﾀｽに変更する
		}else{
			outp->aps_sts = 0;										// 初回
		}
	}

	outp->aps_wrt[0] = ck->mont;
	outp->aps_wrt[1] = ck->date;
	outp->aps_wrt[2] = ck->hour;
	outp->aps_wrt[3] = ck->minu;
	MDP_siz = sizeof( m_apspas );
	md_pari( &(uchar)MDP_buf[0], (ushort)MDP_siz, 0 );				// Odd Parity Set
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾟﾘﾍﾟｲﾄﾞﾗｲﾄﾃﾞｰﾀ作成                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_mkpre( inpp, pre )                                   |*/
/*| PARAMETER    : m_prepid *inpp : ｶｰﾄﾞﾘｰﾄﾞﾃﾞｰﾀ                           |*/
/*|                pre_rec  *pre  : ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞﾗｲﾄ内容                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_mkpre( m_gtprepid *inpp, pre_rec *pre )
{
	short	i;
	long	set;
	unsigned long	setzan, zenzan;
	m_prepid	*tic = ( m_prepid*)&MDP_buf[31];

	memset( MDP_buf, 0x20, 30 );
	zenzan = (long)astoinl( inpp->prepid.pre_ram, 5 );						// 前回残額ｾｯﾄ
	for( i=7; i>0; i-- ){
		setzan = (unsigned long)( CPrmSS[S_PRP][2-1+i] );
		if( pre->ram < setzan ){									// 残金額 < 設定金額
			if( zenzan >= setzan ){									// 前残額 >= 設定金額
				MDP_buf[i*2+5] = 0x2a;								// 残額ﾏｰｸ'*'ｾｯﾄ
			}
		}
	}
	if(( pre->mno < 50 )&&( CPrmSS[S_PRP][9] )){					// 初回 & 初回印字する?
		pre->mno += 50;
		for( i=7; i>0; i-- ){
			if(( set = CPrmSS[S_PRP][2-1+i] ) == 0 ){				// 設定ｱﾘ?
				continue;											// No..continue
			}
			if( set <= pre->amo ){
				MDP_buf[i*2+7] = 0x2a;								// 初回ﾏｰｸ'*'ｾｯﾄ
				break;
			}
		}
	}
	if( pre->ram == 0 ){
		MDP_buf[5] = 0x2a;											// 残額0円
	}

	MDP_buf[30] = 0x09;												// 区切りﾏｰｸ
	memcpy( tic, &inpp->prepid.pre_idc, sizeof( m_prepid ) );
	intoas( tic->pre_mno, (unsigned short)pre->mno, 2 );			// 販売機№ｾｯﾄ
	intoasl( tic->pre_ram, (unsigned long)pre->ram, 5 );			// 残額ｾｯﾄ
	MDP_siz = 31 + sizeof( m_prepid );
	MDP_mag = sizeof( m_prepid );
	if(inpp->magformat.type == 0){
		md_pari( &(uchar)MDP_buf[31], sizeof( m_prepid ), 1 );			// Even Parity Set
		MDP_buf[127] = 0;		//GTフォーマット作成フラグ
	}else{
		md_pari2( &(uchar)MDP_buf[31], sizeof( m_prepid ), 1 );			// Even Parity Set
		MDP_buf[127] = 1;		//GTフォーマット作成フラグ
	}
	RD_PrcWriteFlag = 1;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 回数券ﾗｲﾄﾃﾞｰﾀ作成                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_mkkas( inpp, pre )                                   |*/
/*| PARAMETER    : m_prepid *inpp : ｶｰﾄﾞﾘｰﾄﾞﾃﾞｰﾀ                           |*/
/*|                pre_rec  *pre  : 回数券ﾗｲﾄ内容    		               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2005-10-28                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	al_mkkas( m_gtkaisuutik *inpp, svs_rec *pre, struct clk_rec *ck )
{
	short	i, j;
	uchar	BeforeDosuu, AfterDosuu;
	m_kaisuutik	*tic = ( m_kaisuutik*)&MDP_buf[31];

	memset( MDP_buf, 0x20, 30 );

	/* 券面印字作成 */
		BeforeDosuu = (uchar)CRD_DAT.SVS.sts;
		AfterDosuu  = KaisuuAfterDosuu;

		j = (short)(BeforeDosuu - AfterDosuu);						// 今回利用回数
		if ( j >= 2 ){												// 2回以上
			for ( i = 0 ; i < (j-1) ; i ++ ){
				BeforeDosuu --;		  								// 利用回数ﾃﾞｸﾘﾒﾝﾄ
				MDP_buf[BeforeDosuu*2+5] = 0x2a;					// '＊' ﾃﾞｰﾀ
			}
		}
		MDP_buf[AfterDosuu*2+5] = 0x2a;								// '＊' ﾃﾞｰﾀ

	/* 磁気ﾃﾞｰﾀｾｯﾄ */
	MDP_buf[30] = 0x09;												// 区切りﾏｰｸ
	memcpy( tic, &inpp->kaisuutik.kaitik_idc, sizeof( m_kaisuutik ) );

	tic->kaitik_kai = (uchar)KaisuuAfterDosuu;						// 残回数ｾｯﾄ

	tic->kaitik_wrt[0] = ck->mont;									// 処理年月日ｾｯﾄ
	tic->kaitik_wrt[1] = ck->date;
	tic->kaitik_wrt[2] = ck->hour;
	tic->kaitik_wrt[3] = ck->minu;

	MDP_siz = 31 + sizeof( m_kaisuutik );
	MDP_mag = sizeof( m_kaisuutik );
	if(inpp->magformat.type == 0){
		md_pari( &(uchar)MDP_buf[31], sizeof( m_kaisuutik ), 0 );		// Odd Parity Set
		MDP_buf[127] = 0;		//GTフォーマット作成フラグ
	}else{
		tic->kaitik_pno[0] |= (uchar)((inpp->magformat.ex_pkno & 0x01) << 7);
		tic->kaitik_pno[1] |= (uchar)((inpp->magformat.ex_pkno & 0x02) << 6);
		tic->kaitik_tan[0] |= (uchar)((inpp->magformat.ex_pkno & 0x04) << 5);
		tic->kaitik_tan[1] |= (uchar)((inpp->magformat.ex_pkno & 0x08) << 4);
		tic->kaitik_sta[0] |= (uchar)((inpp->magformat.ex_pkno & 0x10) << 3);
		tic->kaitik_sta[1] |= (uchar)((inpp->magformat.ex_pkno & 0x20) << 2);
		tic->kaitik_sta[2] |= (uchar)((inpp->magformat.ex_pkno & 0x40) << 1);
		tic->kaitik_end[0] |= (uchar)(inpp->magformat.ex_pkno & 0x80);
		md_pari2( &(uchar)MDP_buf[31], (ushort)1, 1 );					// IDコードEven Parity Set
		MDP_buf[127] = 1;		//GTフォーマット作成フラグ
	}
	return;
}


// 券IDﾃﾞｰﾀﾃｰﾌﾞﾙ
const	uchar	tik_id_tbl[TIK_SYU_MAX+1][ID_SYU_MAX] =
	{
//		ID1		ID2		ID3		ID4		ID5		ID6
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	// 券種００：ダミー
		0x21,	0x45,	0x50,	0x00,	0x00,	0x06,	// 券種０１：駐車券（ＡＲ掛売券）
		0x22,	0x46,	0x54,	0x00,	0x00,	0x20,	// 券種０２：駐車券（ＡＲ-サ券中止券）
		0x23,	0x47,	0x56,	0x00,	0x00,	0x2E,	// 券種０３：駐車券（ＡＲ-Ｐ回中止券）
		0x24,	0x48,	0x57,	0x00,	0x00,	0x30,	// 券種０４：駐車券（精算前）
		0x25,	0x49,	0x58,	0x00,	0x00,	0x39,	// 券種０５：駐車券（精算後）
		0x26,	0x4A,	0x59,	0x00,	0x00,	0x3C,	// 券種０６：駐車券（中止券）
		0x27,	0x4B,	0x5A,	0x00,	0x00,	0x3D,	// 券種０７：駐車券（掛売券）
		0x28,	0x4C,	0x5B,	0x00,	0x00,	0x3E,	// 券種０８：駐車券（再精算中止券）
		0x29,	0x4D,	0x5C,	0x00,	0x00,	0x3F,	// 券種０９：駐車券（精算済み券）
		0x2A,	0x4E,	0x5D,	0x00,	0x00,	0x00,	// 券種１０：再発行券
		0x2B,	0x4F,	0x5F,	0x00,	0x00,	0x43,	// 券種１１：紛失券
		0x1A,	0x64,	0x6A,	0x70,	0x76,	0x44,	// 券種１２：ＡＰＳ定期券
		0x0E,	0x1B,	0x1C,	0x1D,	0x1E,	0x1F,	// 券種１３：プリペイドカード
		0x2C,	0x65,	0x6B,	0x71,	0x77,	0x62,	// 券種１４：回数券
		0x2D,	0x66,	0x6C,	0x72,	0x78,	0x63,	// 券種１５：サービス券（掛売券・割引券）
		0x41,	0x69,	0x6F,	0x75,	0x7B,	0x7E,	// 券種１６：係員カード

	};

// 券種毎使用ID設定ｱﾄﾞﾚｽﾃｰﾌﾞﾙ１（券種毎の設定ﾃﾞｰﾀｱﾄﾞﾚｽ）
const	uchar	id_para_tbl1[TIK_SYU_MAX+1][4] =
	{
//		基本	拡張１	拡張２	拡張３
		0,		0,		0,		0,					// 券種００：ダミー
		10,		0,		0,		0,					// 券種０１：駐車券（ＡＲ掛売券）
		10,		0,		0,		0,					// 券種０２：駐車券（ＡＲ-サ券中止券）
		10,		0,		0,		0,					// 券種０３：駐車券（ＡＲ-Ｐ回中止券）
		10,		0,		0,		0,					// 券種０４：駐車券（精算前）
		10,		0,		0,		0,					// 券種０５：駐車券（精算後）
		10,		0,		0,		0,					// 券種０６：駐車券（中止券）
		10,		0,		0,		0,					// 券種０７：駐車券（掛売券）
		10,		0,		0,		0,					// 券種０８：駐車券（再精算中止券）
		10,		0,		0,		0,					// 券種０９：駐車券（精算済み券）
		10,		0,		0,		0,					// 券種１０：再発行券
		10,		0,		0,		0,					// 券種１１：紛失券
		11,		12,		13,		14,					// 券種１２：ＡＰＳ定期券
		11,		12,		13,		14,					// 券種１３：プリペイドカード
		11,		12,		13,		14,					// 券種１４：回数券
		11,		12,		13,		14,					// 券種１５：サービス券（掛売券・割引券）
		10,		0,		0,		0,					// 券種１６：係員カード
	};

// 券種毎使用ID設定ｱﾄﾞﾚｽﾃｰﾌﾞﾙ２（券種毎の設定ﾃﾞｰﾀ読出し位置）
const	char	id_para_tbl2[TIK_SYU_MAX+1] =
	{
		0,			// 券種００：ダミー
		1,			// 券種０１：駐車券（ＡＲ掛売券）
		1,			// 券種０２：駐車券（ＡＲ-サ券中止券）
		1,			// 券種０３：駐車券（ＡＲ-Ｐ回中止券）
		1,			// 券種０４：駐車券（精算前）
		1,			// 券種０５：駐車券（精算後）
		1,			// 券種０６：駐車券（中止券）
		1,			// 券種０７：駐車券（掛売券）
		1,			// 券種０８：駐車券（再精算中止券）
		1,			// 券種０９：駐車券（精算済み券）
		1,			// 券種１０：再発行券
		1,			// 券種１１：紛失券
		1,			// 券種１２：ＡＰＳ定期券
		3,			// 券種１３：プリペイドカード
		4,			// 券種１４：回数券
		2,			// 券種１５：サービス券（掛売券・割引券）
		1,			// 券種１６：係員カード
	};

/*[]---------------------------------------------------------------------------[]*/
/*|		券ﾃﾞｰﾀﾁｪｯｸ＆ID変換処理													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Read_Tik_Chk( *tik_data, *pk_syu )						|*/
/*|																				|*/
/*|	PARAMETER		:	uchar	*tik_data	=	券ﾃﾞｰﾀﾎﾟｲﾝﾀ						|*/
/*|																				|*/
/*|						uchar	*pk_syu		=	駐車場種別ﾃﾞｰﾀ格納ﾎﾟｲﾝﾀ			|*/
/*|													1=基本						|*/
/*|													2=拡張1						|*/
/*|													3=拡張2						|*/
/*|													4=拡張3						|*/
/*|																				|*/
/*|						※新ｶｰﾄﾞIDを使用する設定でﾁｪｯｸ結果がＯＫの場合のみ有効	|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret = ﾁｪｯｸ結果									|*/
/*|										OK ：券ﾃﾞｰﾀ有効							|*/
/*|										NG ：券ﾃﾞｰﾀ無効							|*/
/*|										 2 : 新ｶｰﾄﾞID使用しない (OK=0/NG=0xFF)	|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-11-22														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	Read_Tik_Chk( uchar *tik_data, uchar *pk_syu, uchar tik_data_length )
{
	uchar	ret = OK;			// 戻り値（ﾁｪｯｸ結果）
	uchar	tik_id;				// 券ＩＤ
	uchar	tik_syu;			// 券種
	uchar	id_syu;				// ID種
	uchar	cmp_end;			// ID比較結果
	ushort	pk_no;				// 駐車場No.
	uchar	pk_syu_wk;			// 駐車場種別（ﾜｰｸ）
	uchar	ng_pkno	= OFF;		// ＮＧ駐車場Ｎｏ．ｾｯﾄﾌﾗｸﾞ
	uchar	idchg	= OFF;		// ID1変換ﾌﾗｸﾞ
	uchar	id_syu_settei;		// 使用券ID

	if( prm_get( COM_PRM,S_PAY,10,1,4 ) != 0 ){						// 新カードＩＤ使用する？

		// 新カードＩＤを使用する場合

		// 券ﾃﾞｰﾀのIDから券種を検索する
		tik_id = tik_data[0];										// 券ID取得
		cmp_end = OFF;
		for( tik_syu = 1 ; tik_syu <= TIK_SYU_MAX ; tik_syu++ ){	// 券種毎のID比較ﾙｰﾌﾟ

			for( id_syu = 0 ; id_syu < ID_SYU_MAX ; id_syu++ ){		// ID種(１～５)のID比較ﾙｰﾌﾟ

				if( tik_id == tik_id_tbl[tik_syu][id_syu] ){		// ID一致？
					cmp_end = ON;									// YES
					break;
				}
			}
			if( cmp_end == ON ){									// ID一致？
				break;												// YES ->
			}
		}
		// 拡張ID以外のカードであればはじく
		if( cmp_end == ON ){
			if( tik_data_length > 60 ){								// 60byteより長いデータは拡張ID対応カードには無い
																	// 対応カードの最長は29byteだが、磁気リーダーの
																	// クレジットカード判定と同じ長さで比較する
				cmp_end = OFF;
				if( tik_data[10] == 0x53 ){							// データID＝53H
					if( (MAGred[MAG_GT_APS_TYPE] == 0) &&			// APSフォーマットでPAK
						((tik_data[11] == 0x50) && (tik_data[12] == 0x41) && (tik_data[13] == 0x4b)) ){
						cmp_end = ON;								// アマノ係員カード
					}
					if( (MAGred[MAG_GT_APS_TYPE] == 1) &&			// GTフォーマットでPGT
						((tik_data[11] == 0x50) && (tik_data[12] == 0x47) && (tik_data[13] == 0x54)) ){
						cmp_end = ON;								// アマノ係員カード
					}
				}
			}
		}
		if( cmp_end == ON ){

			// 券ＩＤ＝対応範囲内の場合

			pk_no = PkNo_get( tik_data, tik_syu  );					// 券ﾃﾞｰﾀから駐車場No.取得

			// 駐車場種別検索
			if( CPrmSS[S_SYS][1] == (ulong)pk_no ){					// 基本？
				pk_syu_wk = 1;
			}
			else if( CPrmSS[S_SYS][2] == (ulong)pk_no ){			// 拡張１？
				pk_syu_wk = 2;
			}
			else if( CPrmSS[S_SYS][3] == (ulong)pk_no ){			// 拡張２？
				pk_syu_wk = 3;
			}
			else if( CPrmSS[S_SYS][4] == (ulong)pk_no ){			// 拡張３？
				pk_syu_wk = 4;
			}
			else{													// 駐車場No.不一致
				pk_syu_wk = 0;
			}

			// 券種毎に使用するID種を検索
			switch( pk_syu_wk ){									// 駐車場種別？

				case	1:											// 駐車場種別＝基本

					id_syu_settei = (uchar)prm_get(					// 使用券ID種設定ﾊﾟﾗﾒｰﾀ取得
													COM_PRM,
													S_PAY,
													(short)id_para_tbl1[tik_syu][pk_syu_wk-1],
													1,
													id_para_tbl2[tik_syu]
												);
					break;

				case	2:											// 駐車場種別＝拡張１
				case	3:											// 駐車場種別＝拡張２
				case	4:											// 駐車場種別＝拡張３

					if(
						tik_syu == 1	||							// 券種別＝駐車券（ＡＲ掛売券）
						tik_syu == 2	||							// 券種別＝駐車券（ＡＲ-サ券中止券）
						tik_syu == 3	||							// 券種別＝駐車券（ＡＲ-Ｐ回中止券）
						tik_syu == 4	||							// 券種別＝駐車券（精算前）
						tik_syu == 5	||							// 券種別＝駐車券（精算後）
						tik_syu == 6	||							// 券種別＝駐車券（中止券）
						tik_syu == 7	||							// 券種別＝駐車券（掛売券）
						tik_syu == 8	||							// 券種別＝駐車券（再精算中止券）
						tik_syu == 9	||							// 券種別＝駐車券（精算済み券）
						tik_syu == 10	||							// 券種別＝再発行券
						tik_syu == 11	||							// 券種別＝紛失券
						tik_syu == 16								// 券種別＝係員カード
					){
						// 券種別が駐車券または係員カードの場合（拡張駐車場No.はＮＧ）
						ret		= NG;
						ng_pkno	= ON;
						idchg	= ON;
					}
					else{
						// 券種別が駐車券または係員カード以外の場合
						id_syu_settei = (uchar)prm_get(				// 使用券ID種設定ﾊﾟﾗﾒｰﾀ取得
														COM_PRM,
														S_PAY,
														(short)id_para_tbl1[tik_syu][pk_syu_wk-1],
														1,
														id_para_tbl2[tik_syu]
													);
					}
					break;

				default:											// 駐車場種別＝駐車場Ｎｏ．不一致
					ret		= NG;
					ng_pkno	= ON;
					idchg	= ON;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			}

			if( ret == OK ){

				// 駐車場Ｎｏ．がＯＫの場合

				switch( id_syu_settei ){							// 使用券ID種設定？

					case	1:										// ID1
					case	2:										// ID2
					case	3:										// ID3
					case	4:										// ID4
					case	5:										// ID5

						if( tik_id != tik_id_tbl[tik_syu][id_syu_settei-1] ){
							// 使用券ID種のIDと不一致
							ret		= NG;
							ng_pkno	= ON;
							idchg	= ON;
						}
						break;

					case	6:										// ID1とID2

						if( ( tik_id != tik_id_tbl[tik_syu][0] ) && ( tik_id != tik_id_tbl[tik_syu][1] ) ){
							// 使用券ID種のIDと不一致
							ret		= NG;
							ng_pkno	= ON;
							idchg	= ON;
						}
						break;

					case	7:										// ID1とID3

						if( ( tik_id != tik_id_tbl[tik_syu][0] ) && ( tik_id != tik_id_tbl[tik_syu][2] ) ){
							// 使用券ID種のIDと不一致
							ret		= NG;
							ng_pkno	= ON;
							idchg	= ON;
						}
						break;

					case	8:										// ID6
						if( tik_id != tik_id_tbl[tik_syu][5] ){
							// 使用券ID種のIDと不一致
							ret		= NG;
							ng_pkno	= ON;
							idchg	= ON;
						}
						break;
					case	0:										// 読まない
					default:										// その他（設定エラー）
						ret		= NG;
						ng_pkno	= ON;
						idchg	= ON;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				}

				if( ret == OK ){

					// 使用券ID種のIDと券ﾃﾞｰﾀのIDが一致

					tik_data[0]	= tik_id_tbl[tik_syu][0];			// 券ﾃﾞｰﾀのIDをID1（従来型）のIDに変更
					*pk_syu		= pk_syu_wk;						// 駐車場種別（出力ﾊﾟﾗﾒｰﾀ）ｾｯﾄ

					switch( tik_syu ){								// 券種毎の最終読取券ID種を保存

						case	1:									// 駐車券（ＡＲ掛売券）
						case	2:									// 駐車券（ＡＲ-サ券中止券）
						case	3:									// 駐車券（ＡＲ-Ｐ回中止券）
						case	4:									// 駐車券（精算前）
						case	5:									// 駐車券（精算後）
						case	6:									// 駐車券（中止券）
						case	7:									// 駐車券（掛売券）
						case	8:									// 駐車券（再精算中止券）
						case	9:									// 駐車券（精算済み券）
						case	10:									// 再発行券
						case	11:									// 紛失券

							ReadIdSyu.pk_tik = (uchar)(id_syu+1);
							break;

						case	12:									// ＡＰＳ定期券

							ReadIdSyu.teiki = (uchar)(id_syu+1);
							break;

						case	13:									// プリペイドカード

							ReadIdSyu.pripay = (uchar)(id_syu+1);
							break;

						case	14:									// 回数券

							ReadIdSyu.kaisuu = (uchar)(id_syu+1);
							break;

						case	15:									// サービス券（掛売券・割引券）

							ReadIdSyu.svs_tik = (uchar)(id_syu+1);
							break;

						case	16:									// 係員カード

							ReadIdSyu.kakari = (uchar)(id_syu+1);
							break;
					}
				}
			}
		}
		else{
			// 券ＩＤ＝対応範囲外（券IDﾃﾞｰﾀﾃｰﾌﾞﾙに存在しない）
			ret = NG;
		}
	}
	else{
		ret = 2;
	}

	if( ret == NG ){

		// 券ﾃﾞｰﾀが無効な場合

		if( ng_pkno == ON ){
			NgPkNo_set( tik_data, tik_syu );						// 規定外の駐車場No.書込み
		}
		if( idchg == ON ){
			tik_data[0]	= tik_id_tbl[tik_syu][0];					// 券ﾃﾞｰﾀのIDをID1（従来型）のIDに変更
		}
	}

	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		駐車場No.取得処理														|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PkNo_get( *tik_data, tik_syu )							|*/
/*|																				|*/
/*|	PARAMETER		:	uchar	*tik_data	=	券ﾃﾞｰﾀﾎﾟｲﾝﾀ						|*/
/*|																				|*/
/*|						uchar	tik_syu		=	券種別							|*/
/*|																				|*/
/*|	RETURN VALUE	:	ushort	pk_no = 駐車場No.								|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-11-22														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
ushort	PkNo_get( uchar *tik_data, uchar tik_syu )
{
	ushort	pk_no = 0;							// 駐車場No.

	switch( tik_syu ){							// 券種？

//		case	1:								// 駐車券（ＡＲ掛売券）
//		case	2:								// 駐車券（ＡＲ-サ券中止券）
//		case	3:								// 駐車券（ＡＲ-Ｐ回中止券）
//		case	4:								// 駐車券（精算前）
//		case	5:								// 駐車券（精算後）
//		case	6:								// 駐車券（中止券）
//		case	7:								// 駐車券（掛売券）
//		case	8:								// 駐車券（再精算中止券）
//		case	9:								// 駐車券（精算済み券）
//		case	10:								// 再発行券
//		case	11:								// 紛失券
//
//			pk_no  = (ushort)tik_data[1];
//			pk_no += (ushort)pno_d1[tik_data[2]>>5];
//			pk_no += (((ushort)tik_data[8])<<3 )&0x0200;
//			break;

		case	12:								// ＡＰＳ定期券
		case	14:								// 回数券
		case	15:								// サービス券（掛売券・割引券）

			pk_no  = (ushort)tik_data[1];
			pk_no += (ushort)pno_dt[tik_data[2]>>4];
			break;

		case	13:								// プリペイドカード

			pk_no  = astoin( &tik_data[1], 2 );
			pk_no += ( tik_data[3]&0x0f ) * 100;
			break;

		case	16:								// 係員カード

			pk_no = (	( (tik_data[27] & 0x0f) * 1000 ) +
						( (tik_data[28] & 0x0f) * 100 )  +
						( (tik_data[29] & 0x0f) * 10 )   +
						( (tik_data[30] & 0x0f) * 1 )
					);
			break;

	}
	return( pk_no );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		規定外の駐車場No.書込み処理												|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	NgPkNo_set( *tik_data, tik_syu  )						|*/
/*|																				|*/
/*|	PARAMETER		:	uchar	*tik_data	=	券ﾃﾞｰﾀﾎﾟｲﾝﾀ						|*/
/*|																				|*/
/*|						uchar	tik_syu		=	券種別							|*/
/*|																				|*/
/*|	RETURN VALUE	:	void													|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-11-22														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
void	NgPkNo_set( uchar *tik_data, uchar tik_syu )
{

	switch( tik_syu ){							// 券種？

		case	1:								// 駐車券（ＡＲ掛売券）
		case	2:								// 駐車券（ＡＲ-サ券中止券）
		case	3:								// 駐車券（ＡＲ-Ｐ回中止券）
		case	4:								// 駐車券（精算前）
		case	5:								// 駐車券（精算後）
		case	6:								// 駐車券（中止券）
		case	7:								// 駐車券（掛売券）
		case	8:								// 駐車券（再精算中止券）
		case	9:								// 駐車券（精算済み券）
		case	10:								// 再発行券
		case	11:								// 紛失券

			// 駐車場No.1023をｾｯﾄ
			tik_data[1] = (uchar)(tik_data[1] | 0x7f);
			tik_data[2] = (uchar)(tik_data[2] | 0x60);
			tik_data[8] = (uchar)(tik_data[8] | 0x40);
			break;

		case	12:								// ＡＰＳ定期券
		case	14:								// 回数券
		case	15:								// サービス券（掛売券・割引券）

			// 駐車場No.1023をｾｯﾄ
			tik_data[1] = (uchar)(tik_data[1] | 0x7f);
			tik_data[2] = (uchar)(tik_data[2] | 0x70);
			break;

		case	13:								// プリペイドカード

			// 駐車場No.1023をｾｯﾄ
			tik_data[1] = 0x32;
			tik_data[2] = 0x33;
			tik_data[3] = 0x0A;
			break;

		case	16:								// 係員カード

			// 駐車場No.1023をｾｯﾄ
			tik_data[27] = 0x31;
			tik_data[28] = 0x30;
			tik_data[29] = 0x32;
			tik_data[30] = 0x33;
			break;

	}
}
/*[]--------------------------------------------- AMANO Corporation (R) --[]*/
/* MODULE NAME : ck_jis_credit												*/
/* カードがクレジットカードか否かチェックする								*/
/* Parameter:																*/
/*		MagDataSize  : リードデータ長										*/
/*		card_data(i) : リードした生データ									*/
/* Return:																	*/
/*		0 : ちがう															*/
/*		1 : JIS1/JIS2 カード												*/
/*[]--------------------------------------------- AMANO Corporation (R) --[]*/
uchar	ck_jis_credit( uchar MagDataSize, char *card_data )
{
	uchar	cWok;
	int		i;
	int		b_cnt = 0;								//　ビットカウンタ

	// ID の ビット数 => cnt
	cWok = (uchar)card_data[0];
	for( i=0; i < 8; i++ ){
		if(cWok & 0x01) b_cnt++;
		cWok >>= 1;
	}

	// JIS1 判定
	if( ((uchar)card_data[0] == 0xff) &&			// ISOｸﾚｼﾞｯﾄ種別(0xff)
	    	((uchar)card_data[1] == 0xbb) )			// 開始符合(0xbb)
	{
		MAGred[MAG_ID_CODE] = 0x7F;					// 内部コードに変換
		return 1;
	}

	// JIS2 判定
	if( ( b_cnt % 2 ) ||							// パリティーは奇数（アマノカード）
		( 69 > MagDataSize ) ||						// 磁気ﾃﾞｰﾀｻｲｽﾞ 69byte未満 (JIS2でない)
													// FBは開始符合の次（IDﾏｰｸ）～終了符号まで読み出すので
													// JIS2カードを読むと Length=70byteとなる
		( (0x41 == card_data[0]) && (0x53 == card_data[10]) ) )	// ｱﾏﾉ係員ｶｰﾄﾞ
																// ﾗﾍﾞﾙID = 41h かつ ﾃﾞｰﾀID = 53hでｱﾏﾉ係員ｶｰﾄﾞと判断する
	{
		return 0;
	}

	return 1;										// JIS2 かもしれない
}

/*[]--------------------------------------------- AMANO Corporation (R) --[]*/
/* MODULE NAME : chk_for_inquiry											*/
/* 定期券問合せチェック														*/
/* Parameter:																*/
/*		type : 																*/
/*			1 : 問合せ														*/
/*			2 : ｱﾝﾁﾊﾟｽNGの時のみ問合せ										*/
/* Return:																	*/
/*		0 : 問合せしない													*/
/*		1 : 問合せする														*/
/*[]--------------------------------------------- AMANO Corporation (R) --[]*/
uchar chk_for_inquiry(uchar type)
{
	if( _is_ntnet_normal()			&&						// NT-NET接続あり？
		(OPECTL.ChkPassSyu == 0)	&&						// 問合せ中でない
		(MifStat != MIF_WROTE_FAIL) &&						// 書き込みNG後の再タッチでない
		(uchar)prm_get( COM_PRM,S_NTN,26,1,4 ) == type) 	// 定期券問合せする?
	{
		return 1;
	}
	
	return 0;
}
/*[]-----------------------------------------------------------------------[]*/
/*| 定期有効期限チェック処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| NT-7700のチェックロジックに合わせる。									|*/
/*|	① ﾘｰﾄﾞｵﾝﾘｰ設定の場合、日付切換時刻を 0:00 としてﾁｪｯｸする。				|*/
/*|	② ﾘｰﾄﾞ&ﾗｲﾄ設定の場合、第１料金体系開始時刻(31-0004)時刻を日付切換時刻	|*/
/*|    としてﾁｪｯｸする。														|*/
/*|	※UTはﾘｰﾄﾞ&ﾗｲﾄ固定設定のため、常に②となる。							|*/
/*|																			|*/
/*| MODULE NAME  : Ope_PasKigenCheck()                                      |*/
/*| PARAMETER    : StartKigen_ndat = 定期券内の有効期限開始日(ﾉｰﾏﾗｲｽﾞ値)	|*/
/*|				   EndKigen_ndat   = 定期券内の有効期限終了日(ﾉｰﾏﾗｲｽﾞ値)	|*/
/*|				   TeikiSyu        = 定期種別(1～15)						|*/
/*|				   Target_ndat     = ﾁｪｯｸ対象年月日(ﾉｰﾏﾗｲｽﾞ値)				|*/
/*|				   Target_nmin     = ﾁｪｯｸ対象時分　(ﾉｰﾏﾗｲｽﾞ値)				|*/
/*| RETURN VALUE : 0=期限開始日より前，1=有効期限内，2=期限終了日より後     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : okuda			                                        |*/
/*| Date         : 2007/02/14                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
uchar	Ope_PasKigenCheck( ushort StartKigen_ndat, ushort EndKigen_ndat, short TeikiSyu, 
						   ushort Target_ndat, ushort Target_nmin )
{
	ushort	Kirikae_nmin;						// 日付切換時間(ﾉｰﾏﾗｲｽﾞ値)
	uchar	ret=1;

	/** 日付切換時刻get **/

		Kirikae_nmin = (ushort)prm_tim( COM_PRM, S_RAT, (short)4 );	// 日付切換時刻は料金体系開始時刻(ﾉｰﾏﾗｲｽﾞ値)

	/** 有効期間の値を加工して比較を行う **/

	if( Target_nmin < Kirikae_nmin ){			// まだ日付切換時刻になっていない（対象時間＜切換時刻）
		++StartKigen_ndat;						// 開始日と終了日を＋１日して範囲チェックを行う
		++EndKigen_ndat;
	}

	/** 範囲ﾁｪｯｸ **/

	if( Target_ndat < StartKigen_ndat )			// 期限前（対象日＜開始日）
		ret = 0;
	else if( EndKigen_ndat < Target_ndat )		// 期限後（終了日＜対象日）
		ret = 2;

	return ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*| GTﾌｫｰﾏｯﾄ設定ﾁｪｯｸ処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ope_GT_Settei_Check()                                    |*/
/*| PARAMETER    : gt_flg = 読み取った券がGTorAPSﾌｫｰﾏｯﾄ						|*/
/*|				   pno_syu= 基本,拡張1～3の番号								|*/
/*| RETURN VALUE : 0=設定OK,1=設定NG									    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : akiba			                                        |*/
/*| Date         : 2008/06/13                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
static short Ope_GT_Settei_Check( uchar gt_flg, ushort pno_syu )
{
short	ret;

	ret = 0;
	if(prm_get( COM_PRM,S_SYS,11,1,1) == 1){									// APS/GT設定
		if( gt_flg == 1 ){														// ﾘｰﾄﾞﾃﾞｰﾀがGTﾌｫｰﾏｯﾄ
			if( !prm_get( COM_PRM,S_SYS,12,1,magtype[pno_syu] ) ){				// 利用駐車場№がAPS
				ret = 1;
			}
		}else{																	// ﾘｰﾄﾞﾃﾞｰﾀがAPSﾌｫｰﾏｯﾄ
			if( prm_get( COM_PRM,S_SYS,12,1,magtype[pno_syu] ) ){					// 利用駐車場№がGT
				ret = 1;
			}
		}
	}else{																		// APSデータ
		if( gt_flg == 0 ){														// ﾘｰﾄﾞﾃﾞｰﾀがAPSﾌｫｰﾏｯﾄ
			if( prm_get( COM_PRM,S_SYS,12,1,magtype[pno_syu] ) ){					// 利用駐車場№がGT
				ret = 1;
			}
		}
	}
	if( GT_Settei_flg == 1 ){
		ret = 1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 修正精算用定期券ﾘｰﾄﾞ(修正元で使用した定期を挿入なしで使用可とする)     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_pasck_syusei( void )                                 |*/
/*| PARAMETER    : m_servic *mag : 磁気ﾃﾞｰﾀ格納先頭ｱﾄﾞﾚｽ                   |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                      25 = 車種ｴﾗｰ                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	al_pasck_syusei( void )
{
	short	ret;
	short	w_syasyu;
	short	data_adr;		// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ
	char	data_pos;		// 使用可能料金種別のﾃﾞｰﾀ位置

	vl_passet( vl_frs.lockno );
	w_syasyu = 0;

	ret = 0;
	if( prm_get(COM_PRM, S_TYP, 98, 1, 4) == 3 ){
		for( ; ; ){
			if( ryo_buf.syubet < 6 ){
				// 精算対象の料金種別A～F(0～5)
				data_adr = 10*(tsn_tki.kind-1)+9;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)(6-ryo_buf.syubet);				// 使用可能料金種別のﾃﾞｰﾀ位置取得
			}else{
				// 精算対象の料金種別G～L(6～11)
				data_adr = 10*(tsn_tki.kind-1)+10;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
				data_pos = (char)(12-ryo_buf.syubet);				// 使用可能料金種別のﾃﾞｰﾀ位置取得
			}
			if( prm_get( COM_PRM, S_PAS, data_adr, 1, data_pos ) ){	// 使用不可設定？
				ret = 25;											// 他の車種の券
				break;
			}
			break;
		}
	}
	if( ret == 0 ){													// ｶｰﾄﾞOK?
		if( w_syasyu ){
			vl_now = V_SYU;											// 種別切換
			syashu = (char)w_syasyu;								// 車種
		}else{														// 車種切換でない
			vl_now = V_TSC;											// 定期券(駐車券併用有り)
		}
		card_use[USE_PAS] += 1;										// 定期券枚数+1
		PayData.teiki.syu = (uchar)tsn_tki.kind;					// 定期券種別
		NTNET_Data152Save((void *)(&PayData.teiki.syu), NTNET_152_TEIKISYU);
		PayData.teiki.status = (uchar)tsn_tki.status;				// 定期券ｽﾃｰﾀｽ(読取時)
		PayData.teiki.id = tsn_tki.code;							// 定期券id
		NTNET_Data152Save((void *)(&PayData.teiki.id), NTNET_152_TEIKIID);
		PayData.teiki.pkno_syu = (uchar)tsn_tki.pkno;				// 定期券駐車場Ｎｏ．種別
		NTNET_Data152Save((void *)(&PayData.teiki.pkno_syu), NTNET_152_PKNOSYU);
		PayData.teiki.update_mon = 0;								// 更新月数
		PayData.teiki.s_year = tsn_tki.data[0];						// 有効期限(開始：年)
		PayData.teiki.s_mon = tsn_tki.data[1];						// 有効期限(開始：月)
		PayData.teiki.s_day = tsn_tki.data[2];						// 有効期限(開始：日)
		PayData.teiki.e_year = tsn_tki.data[3];						// 有効期限(終了：年)
		PayData.teiki.e_mon = tsn_tki.data[4];						// 有効期限(終了：月)
		PayData.teiki.e_day = tsn_tki.data[5];						// 有効期限(終了：日)
		PayData.teiki.update_rslt1 = 0;								// 定期更新精算時の更新結果			(OK：更新成功／NG：更新失敗)
		PayData.teiki.update_rslt2 = 0;								// 定期更新精算時のラベル発行結果	(OK：正常発行／NG：発行不良)
		PassPkNoBackup = 0;											// 定期券駐車場番号ﾊﾞｯｸｱｯﾌﾟ(n分ﾁｪｯｸ用)
		PassIdBackup = (ushort)tsn_tki.code;						// 定期券idﾊﾞｯｸｱｯﾌﾟ(n分ﾁｪｯｸ用)
		PassIdBackupTim = (ulong)( CPrmSS[S_TIK][9] * 60 * 2 );		// n分ﾁｪｯｸ用ﾀｲﾏｰ起動(500ms Timer)

		if( SKIP_APASS_CHK ){										// 強制ｱﾝﾁﾊﾟｽOFF設定(定期券ﾁｪｯｸ＝しない)？
			OPECTL.Apass_off_seisan = 1;							// 強制ｱﾝﾁﾊﾟｽOFF設定状態で定期使用
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| JIS2判定関数														   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : chk_JIS2(void)			                               |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : result :  OK = JIS2                                     |*/
/*|                          NG = JIS2以外                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2009-11-11                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	chk_JIS2( void )
{
	uchar i;
	uchar count = 0;
	uchar p_law = RDT_REC.rdat[3];
	uchar result = NG;
	
	for( i=0; i < 8; i++ ){										// パリティを算出
		if(p_law & 0x01) count++;
		p_law >>= 1;
	}
	if( !((count % 2 == 0) && ((RDT_REC.idc2 - 3) >= 69)) ){	// JIS2形式ではない
		return result;											// NGとする。
	}
	return (result = OK);
}

