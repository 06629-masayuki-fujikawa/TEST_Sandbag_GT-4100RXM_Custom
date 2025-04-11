/*[]----------------------------------------------------------------------------------------------[]*/
/*| fukuden.c																					   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2001-12-27																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"cnm_def.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"mnt_def.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"LKmain.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"suica_def.h"
#include	"flp_def.h"
#include	"ntnetauto.h"
#include	"AppServ.h"
#include	"LKmain.h"
#include	"oiban.h"
#include	"raudef.h"
#include	"cre_ctrl.h"

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 集計・印字中の停電の復電処理																   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fukuden( void )																   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: void																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-12-27																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void fukuden( void )
{
	char	cy, sy;													/*								*/
	SYUKEI	*ts, *gs, *ms, *ws, *ds;								/* ms(MT合計),ds(T合計)を追加	*/
	unsigned long	*ss, *as;										/*								*/
	LOKTOTAL_DAT	*lkts;											/*								*/
	int		i;														/*								*/
	ushort	syasitu;												/*								*/
	uchar	f_SameData;												/* 1=定期中止ﾃｰﾌﾞﾙ内に目的ﾃﾞｰﾀあり */
	uchar	uc_update = PayData.teiki.update_mon;					/* 更新月格納					*/
	T_FrmReceipt	ReceiptPreqData;								// ＜領収証印字要求＞				ﾒｯｾｰｼﾞﾃﾞｰﾀ
	T_FrmFusKyo		FusKyoPreqData;									// ＜不正・強制出庫情報印字要求＞	ﾒｯｾｰｼﾞﾃﾞｰﾀ
	uchar			jnl_pri = OFF;									// ｼﾞｬｰﾅﾙ印字有無
	ushort	num;
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
	long			jnl_pri_jadge;
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	char	qrcode[QRCODE_RECEIPT_SIZE + 4];
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
																	/*								*/
	cy = ac_flg.cycl_fg;											/* 精算ｻｲｸﾙﾌﾗｸﾞ					*/
	sy = ac_flg.syusyu;												/* 集計ｻｲｸﾙﾌﾗｸﾞ					*/
	ts = &skyprn;													/*								*/
	gs = &sky.gsyuk;												/* ＧＴ集計ｴﾘｱ					*/
	ms = &sky.msyuk;												/* ＭＴ集計ｴﾘｱ					*/
	ws = &wksky;													/* 集計ﾜｰｸｴﾘｱ					*/
																	/*								*/
																	/*								*/
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//// MH321800(S) G.So ICクレジット対応
//	if ( EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {
//		Ec_Settlement_Res.settlement_data = PayData.Electron_data.Ec.pay_ryo;
//		Ec_Settlement_Res.settlement_data_before = PayData.Electron_data.Ec.pay_befor;
//		Ec_Settlement_Res.settlement_data_after = PayData.Electron_data.Ec.pay_after;
//		memcpy( &Ec_Settlement_Res.Card_ID,PayData.Electron_data.Ec.Card_ID,sizeof(Ec_Settlement_Res.Card_ID));
//// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
//		c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &Ec_Settlement_Res.settlement_time);					// 精算日時
//		memcpy(&Ec_Settlement_Res.inquiry_num,PayData.Electron_data.Ec.inquiry_num,sizeof(Ec_Settlement_Res.inquiry_num));// 問合せ番号
//		memcpy(&Ec_Settlement_Res.Termserial_No,PayData.Electron_data.Ec.Termserial_No,sizeof(Ec_Settlement_Res.Termserial_No));// 端末識別番号
//// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
//	}
//	else
//// MH321800(E) G.So ICクレジット対応
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind )){
		Settlement_Res.settlement_data = PayData.Electron_data.Suica.pay_ryo;
		Settlement_Res.settlement_data_before = PayData.Electron_data.Suica.pay_befor;
		Settlement_Res.settlement_data_after = PayData.Electron_data.Suica.pay_after;
		memcpy( &Settlement_Res.Suica_ID,PayData.Electron_data.Suica.Card_ID,sizeof(Settlement_Res.Suica_ID));
	}

// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//// MH321800(S) G.So ICクレジット対応
//	switch (ac_flg.ec_alarm) {
//	case 2:														/* ﾜｰｸｴﾘｱから回復 */
//		memcpy( &sky.tsyuk, ws, sizeof( sky.tsyuk ) );			/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
//		DailyAggregateDataBKorRES( 1 );							/* ﾊﾞｯｸｱｯﾌﾟﾃﾞｰﾀのﾘｽﾄｱ			*/
//		// no break
//	case 1:														/* 決済リーダ アラーム取引ﾃﾞｰﾀ登録 */
//		ec_alarm_syuu(EcAlarm.Brand, EcAlarm.Ryo);				/* アラーム取引登録				*/
//		break;
//// MH810103 GG119202(S) 決済精算中止印字処理修正
////	case 3:
////		// みなし決済発生印字
////		ReqEcDeemedJnlPrint();
////		ac_flg.ec_alarm = 0;
////		break;
//// MH810103 GG119202(E) 決済精算中止印字処理修正
//	default :
//		break;
//	}
//// MH321800(E) G.So ICクレジット対応
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
	// まだ直前取引データ処理をしていなければ処理する
	if( ac_flg.ec_recv_deemed_fg != 0 ){
		EcRecvDeemed_RegistPri();
	}
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// MH810103 GG119202(S) 決済精算中止印字処理修正
	if (ac_flg.ec_deemed_fg != 0) {
		// みなし決済発生印字
		ReqEcDeemedJnlPrint();
		ac_flg.ec_deemed_fg = 0;
		// 入金中に変更して精算中止ログを残す
		cy = 10;
	}
// MH810103 GG119202(E) 決済精算中止印字処理修正

	if( cy != 0 ){													/* 入金～集計加算中の停電有り？	*/
		switch( cy ){												/*								*/
																	/* ***** 精算 *****				*/
			case 10:												/* 入金中						*/
				safecl( 1 );										/* 入金完了処理(金庫枚数算出用)	*/
				safecl( 7 );										/* 金庫枚数算出					*/
				ac_flg.cycl_fg = 50;								/* 入金中停電復帰集計開始		*/
				toty_syu();											/* 精算途中停電復帰集計			*/
				break;												/*								*/
																	/*								*/
			case 11:												/* 入金完（表示０円）			*/
				ryo_buf.fusoku = ryo_buf.turisen;					/* 支払い不足額ｾｯﾄ				*/
			case 12:												/* 釣り銭払い出し起動			*/
			case 13:												/* 釣り銭払い出し完了			*/
				if( ac_flg.cycl_fg == 12 ){							/*								*/
					refalt();										/* 不足分算出					*/
					ryo_buf.fusoku += SFV_DAT.reffal;				/* 支払い不足額ｾｯﾄ				*/
				}													/*								*/
				if(( ryo_buf.pkiti != 0xffff )&&					/* ﾏﾙﾁ精算以外?					*/
				   ( uc_update == 0 )){								/* 定期券更新精算以外			*/
					syasitu = ryo_buf.pkiti;						/*								*/
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
//					queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof(syasitu), &syasitu );	/* 精算完了ｷｭｰｾｯﾄ(ﾌﾗｯﾊﾟｰ下降)	*/
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
					FLAPDT.flp_data[syasitu - 1].s_year = car_ot_f.year;// 精算時刻	（年）をｾｯﾄ
					FLAPDT.flp_data[syasitu - 1].s_mont = car_ot_f.mon;	// 			（月）をｾｯﾄ
					FLAPDT.flp_data[syasitu - 1].s_date = car_ot_f.day;	// 			（日）をｾｯﾄ
					FLAPDT.flp_data[syasitu - 1].s_hour = car_ot_f.hour;// 			（時）をｾｯﾄ
					FLAPDT.flp_data[syasitu - 1].s_minu = car_ot_f.min;	// 			（分）をｾｯﾄ
					IoLog_write(IOLOG_EVNT_AJAST_FIN, (ushort)LockInfo[syasitu - 1].posi, 0, 1);
				}													/*								*/
				ac_flg.cycl_fg = 13;								/*								*/
				PayData_set( 1, 0 );								/* 1精算情報ｾｯﾄ					*/
				kan_syuu();											/*								*/
				safecl( 7 );										/* 金庫枚数算出、釣銭管理集計	*/
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//				Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );		/* 金銭管理ログデータ作成		*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* 金銭管理ログ登録				*/
				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass ) ){	/* 金銭管理ログデータ作成		*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
																	/*								*/
																	/*								*/
				break;												/*								*/
																	/*								*/
			case 14:												/* Ｔ集計をﾜｰｸｴﾘｱへ転送完了		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
				if(( ryo_buf.pkiti != 0xffff )&&					/* ﾏﾙﾁ精算以外?					*/
				   ( uc_update == 0 )){								/* 定期券更新精算以外			*/
					syasitu = ryo_buf.pkiti - 1;					/*								*/
					lkts = &loktl.tloktl.loktldat[syasitu];			/*								*/
					memcpy( lkts, &wkloktotal, sizeof( LOKTOTAL_DAT ) );/* ﾜｰｸｴﾘｱをＴ集計へ(停電対策)*/
				}													/*								*/
				DailyAggregateDataBKorRES( 1 );						/* ﾊﾞｯｸｱｯﾌﾟﾃﾞｰﾀのﾘｽﾄｱ			*/
				ac_flg.cycl_fg = 13;								/*								*/
				PayData_set( 1, 0 );								/* 1精算情報ｾｯﾄ					*/
				kan_syuu();											/*								*/
				safecl( 7 );										/* 金庫枚数算出、釣銭管理集計	*/
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//				Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );		/* 金銭管理ログデータ作成		*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* 金銭管理ログ登録				*/
				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass ) ) {	/* 金銭管理ログデータ作成		*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
				break;												/*								*/
																	/*								*/
			case 16:												/* 個別精算ﾃﾞｰﾀﾜｰｸ登録完了		*/
				if( PrnJnlCheck() == ON ){
					//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//					switch(CPrmSS[S_RTP][1]){	// ｼﾞｬｰﾅﾙ印字条件？
					jnl_pri_jadge = prm_get( COM_PRM, S_RTP, 1, 2, 1 );			// 17-0001の値を変数に保存

					jnl_pri_jadge = invoice_prt_param_check( jnl_pri_jadge );	// インボイス時パラメータ変換

					switch(jnl_pri_jadge){	// ｼﾞｬｰﾅﾙ印字条件？
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
						case	0:
						case	1:
						case	2:
							// ｼﾞｬｰﾅﾙは０円時も記録
							jnl_pri = ON;
							break;
						case	3:
						case	4:
						case	5:
							// ｼﾞｬｰﾅﾙは駐車料金０円時以外記録
							if( PayData.WPrice != 0 ){
								//	駐車料金≠０
								jnl_pri = ON;
							}
							break;
						case	6:
						case	7:
						case	8:
							// ｼﾞｬｰﾅﾙは現金領収０円時以外記録
							if( PayData.WTotalPrice != 0 ){
								//	現金領収額≠０
								jnl_pri = ON;
							}
							break;
						case	9:
						case   10:
						case   11:
// MH321800(S) Y.Tanizaki ICクレジット対応
//							if( PayData.WTotalPrice != 0 || PayData.Electron_data.Suica.e_pay_kind ){
							if( PayData.WTotalPrice != 0 ||
								SuicaUseKindCheck(PayData.Electron_data.Suica.e_pay_kind) ||
								EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ){
// MH321800(E) Y.Tanizaki ICクレジット対応
								//	現金領収額≠０ OR 電子媒体による精算が行われた場合
								jnl_pri = ON;
							}
							break;
						default:
							jnl_pri = OFF;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
							break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
					}

					if( jnl_pri == ON ){
						//	ｼﾞｬｰﾅﾙ印字条件を満たす場合
						ReceiptPreqData.prn_kind = J_PRI;											// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
// MH810104 GG119201(S) 電子ジャーナル対応
//						Pri_Work[1].Receipt_data = PayData;
//						ReceiptPreqData.prn_data = &Pri_Work[1].Receipt_data;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
						memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
						ReceiptPreqData.prn_data = &Cancel_pri_work;			// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
// MH810104 GG119201(E) 電子ジャーナル対応
						ReceiptPreqData.reprint = OFF;												// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）
						queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
					}
				}
// MH810100(S) K.Onodera  2019/12/18 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//				NTNET_Snd_Data22_FusDel( ryo_buf.pkiti, (ushort)PayData.PayMethod, (ushort)PayData.PayClass, (ushort)PayData.OutKind );	// 不正出庫情報あれば消す
// MH810100(E) K.Onodera  2019/12/18 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
				goto	_after_pay_log_regist;
																	/*								*/
// MH810100(S) K.Onodera  2019/12/18 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//			case 122:
//				memcpy( &fusei, &wkfus, sizeof( struct FUSEI_SD ) );	/* 不正ﾃﾞｰﾀ戻し			*/
//				NTNET_Snd_Data22_FusDel( ryo_buf.pkiti, (ushort)PayData.PayMethod, (ushort)PayData.PayClass, (ushort)PayData.OutKind );	// 不正出庫情報あれば消す
//				goto	_after_pay_log_regist;
// MH810100(E) K.Onodera  2019/12/18 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
			case 15:												/* Ｔｴﾘｱ集計加算完了			*/
				PayData_set( 1, 0 );								/* 1精算情報ｾｯﾄ					*/
				Log_regist( LOG_PAYMENT );							/* 個別精算情報登録				*/
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
				if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					Set_Pay_RTPay_Data();	// 精算完了logﾃﾞｰﾀの作成
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
					Set_Pay_RTReceipt_Data();
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					// ﾘｱﾙﾀｲﾑ精算ﾃﾞｰﾀのｾﾝﾀｰ追番を更新してlogに書き込む処理
					RTPay_LogRegist_AddOiban();
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
					// RT領収証データ登録
					RTReceipt_LogRegist_AddOiban();
					// QR領収証データ作成
					MakeQRCodeReceipt(qrcode, sizeof(qrcode));
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					// QR確定・取消データ登録
					ope_SendCertifCommit();
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
				}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
_after_pay_log_regist:
				ac_flg.cycl_fg = 19;								/*								*/
			case 19:
				if( ryo_buf.ryo_flg >= 2 )							/* 定期使用?					*/
				{													/*								*/
					memcpy( &tki_cyusi_wk,							/*								*/
							&tki_cyusi,								/*								*/
							sizeof(t_TKI_CYUSI) );					/* 中止定期ﾃﾞｰﾀ退避				*/
																	/*								*/
					ac_flg.cycl_fg = 20;							/*								*/
					for( i=0; i<TKI_CYUSI_MAX; i++ )				/*								*/
					{												/*								*/
						if( tki_cyusi.dt[i].no == PayData.teiki.id &&/* 中止ﾊﾞｯﾌｧに使用定期ﾃﾞｰﾀ有り?*/
							tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*		*/
						{											/*								*/
							NTNET_Snd_Data219(1, &tki_cyusi.dt[i]);	/* 1件削除通知送信				*/
							TKI_Delete((short)i);					/* 中止定期1件ｸﾘｱ				*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* 定期券ﾃｰﾌﾞﾙ更新(出庫)	*/
								  (ushort)PayData.teiki.id,			/*								*/
								  1,								/*								*/
								  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*				*/
								  0xffff );							/*								*/
				}													/*								*/
																	/*								*/
				CountUp( PAYMENT_COUNT );							/* 精算追番+1					*/
				if( ryo_buf.fusoku != 0 ){							/* 預り証発行?					*/
					CountUp(DEPOSIT_COUNT);							/* 預り証追番+1					*/
				}
				if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
					RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
				}
// MH810100(S) K.Onodera  2020/01/23 車番チケットレス(QR確定・取消データ対応)
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
				if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					DC_PopCenterSeqNo( DC_SEQNO_QR );	// センター追番をPop
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
				}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) K.Onodera  2020/01/23 車番チケットレス(QR確定・取消データ対応)
																	/*								*/
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//				if( PayData.credit.pay_ryo ){						/* ｸﾚｼﾞｯﾄｶｰﾄﾞ使用				*/
//					ac_flg.cycl_fg = 80;							// クレジット利用明細登録開始
//					Log_regist( LOG_CREUSE );						/* クレジット利用明細登録		*/
//				}													/*								*/
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
				if (EcUseEMoneyKindCheck(PayData.Electron_data.Ec.e_pay_kind) &&
					PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
					// 引き去り済みを確認した場合も処理未了取引記録を登録する
					ac_flg.cycl_fg = 90;							// 処理未了取引記録登録開始
					EcAlarmLog_Regist(&EcAlarm.Ec_Res);
					ac_flg.cycl_fg = 91;							// 処理未了取引記録登録完了
					// 処理未了取引記録の再精算情報を登録する
					EcAlarmLog_RepayLogRegist(&PayData);
				}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
				ac_flg.cycl_fg = 17;								/* 17:個別精算ﾃﾞｰﾀ登録完了		*/
																	/*								*/
				// no break											/*								*/
																	/*								*/
			case 17:												/* 個別精算ﾃﾞｰﾀ登録完了			*/
				safecl( 7 );										/* 金庫枚数算出					*/
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//				Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );		/* 金銭管理ログデータ作成		*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* 金銭管理ログ登録				*/
				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {	/* 金銭管理ログデータ作成		*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
				break;												/*								*/
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
			case 90:												// 処理未了取引記録登録開始
				// 引き去り済みを確認した場合も処理未了取引記録を登録する
				EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				ac_flg.cycl_fg = 91;								// 処理未了取引記録登録完了
				// no break
			case 91:
				// 処理未了取引記録の再精算情報を登録する
				EcAlarmLog_RepayLogRegist(&PayData);
				ac_flg.cycl_fg = 17;								/* 17:個別精算ﾃﾞｰﾀ登録完了		*/

				safecl( 7 );										/* 金庫枚数算出					*/
				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {	/* 金銭管理ログデータ作成		*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
				}													/*								*/
				break;
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//			case 80:												// クレジット利用明細登録開始
//				Log_regist( LOG_CREUSE );							/* クレジット利用明細登録		*/
//				ac_flg.cycl_fg = 17;								/* 17:個別精算ﾃﾞｰﾀ登録完了		*/
//				safecl( 7 );										/* 金庫枚数算出					*/
//// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
////				Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );		/* 金銭管理ログデータ作成		*/
////				Log_regist( LOG_MONEYMANAGE_NT );					/* 金銭管理ログ登録				*/
//				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {	/* 金銭管理ログデータ作成		*/
//					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
//				}													/*								*/
//// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//				break;												/*								*/
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
			case 20:												/* 定期中止データ更新中			*/
				memcpy( &tki_cyusi, &tki_cyusi_wk,					/*								*/
						sizeof(t_TKI_CYUSI) );						/* 退避中止定期ﾃﾞｰﾀを元に戻す	*/
				goto	_after_pay_log_regist;
																	/*								*/
																	/* ***** 中止 *****				*/
			case 21:												/* 入金完（表示０円）			*/
				ryo_buf.fusoku = ryo_buf.turisen = ryo_buf.nyukin;	/* 支払い不足額ｾｯﾄ				*/
			case 22:												/* 釣り銭払い出し起動			*/
			case 23:												/* 釣り銭払い出し完了			*/
				if( ac_flg.cycl_fg == 22 ){							/*								*/
					refalt();										/* 不足分算出					*/
					ryo_buf.fusoku += SFV_DAT.reffal;				/* 支払い不足額ｾｯﾄ				*/
				}													/*								*/
				ac_flg.cycl_fg = 23;								/*  							*/
				PayData_set( 1, 1 );								/* 1精算情報ｾｯﾄ					*/
				cyu_syuu();											/* 精算中止集計					*/
				safecl( 7 );										/* 金庫枚数算出、釣銭管理集計	*/
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//				Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );/* 中止時の金銭管理ログデータ作成	*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* 金銭管理ログ登録				*/
				if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {/* 中止時の金銭管理ログデータ作成	*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
				break;												/*								*/
																	/*								*/
			case 24:												/* Ｔ集計をﾜｰｸｴﾘｱへ転送完了		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
				DailyAggregateDataBKorRES( 1 );						/* ﾊﾞｯｸｱｯﾌﾟﾃﾞｰﾀのﾘｽﾄｱ			*/
				ac_flg.cycl_fg = 23;								/*								*/
				PayData_set( 1, 1 );								/* 1精算情報ｾｯﾄ					*/
				cyu_syuu();											/*								*/
				safecl( 7 );										/* 金庫枚数算出、釣銭管理集計	*/
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//				Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );/* 中止時の金銭管理ログデータ作成	*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* 金銭管理ログ登録				*/
				if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {/* 中止時の金銭管理ログデータ作成	*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
				break;												/*								*/
																	/*								*/
			case 26:												/* 精算中止ﾃﾞｰﾀﾜｰｸ登録完了		*/
				memcpy( &tki_cyusi, &tki_cyusi_wk,					/*								*/
						sizeof(t_TKI_CYUSI) );						/* 退避中止定期ﾃﾞｰﾀを元に戻す	*/
				if( PrnJnlCheck() == ON ){
					//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
					ReceiptPreqData.prn_kind = J_PRI;				// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
					ReceiptPreqData.prn_data = &Cancel_pri_work;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
					ReceiptPreqData.reprint = OFF;					// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
				}
				IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
				goto	_after_can_log1_regist;
			case 28:												/* 精算中止ﾃﾞｰﾀﾜｰｸ登録完了		*/
				if( PrnJnlCheck() == ON ){
					//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
					ReceiptPreqData.prn_kind = J_PRI;				// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
					ReceiptPreqData.prn_data = &Cancel_pri_work;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
					ReceiptPreqData.reprint = OFF;					// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
				}
				IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
				goto	_after_can_log2_regist;
																	/*								*/
			case 25:												/* Ｔｴﾘｱ集計加算完了			*/
				PayData_set( 1, 1 );								/* 1精算情報ｾｯﾄ					*/
				if( ryo_buf.ryo_flg >= 2 )							/* 定期使用?					*/
				{													/*								*/
					memcpy( &tki_cyusi_wk,							/*								*/
							&tki_cyusi,								/*								*/
							sizeof(t_TKI_CYUSI) );					/* 中止定期ﾃﾞｰﾀ退避				*/
																	/*								*/
					Log_regist( LOG_PAYSTOP );						/* 精算中止情報登録				*/
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime精算中止データ送信する
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
							Set_Cancel_RTPay_Data();
							// ﾘｱﾙﾀｲﾑ精算ﾃﾞｰﾀのｾﾝﾀｰ追番を更新してlogに書き込む処理
							RTPay_LogRegist_AddOiban();
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						}
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						// QR確定・取消データ登録
						ope_SendCertifCancel();
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
_after_can_log1_regist:
																	/*								*/
					f_SameData = 0;									/*								*/
																	/*								*/
					if( TKI_CYUSI_MAX < tki_cyusi.count )			/* fail safe					*/
						tki_cyusi.count = TKI_CYUSI_MAX;			/*								*/
																	/*								*/
					if( TKI_CYUSI_MAX <= tki_cyusi.wtp )			/* fail safe					*/
						tki_cyusi.wtp = tki_cyusi.count - 1;		/*								*/
																	/*								*/
					for( i=0; i<tki_cyusi.count; i++ )				/*								*/
					{												/*								*/
						if( 0L == tki_cyusi.dt[i].pk ){				/* ﾃｰﾌﾞﾙにﾃﾞｰﾀなし				*/
							break;									/* i=登録件数					*/
						}											/*								*/
						if( tki_cyusi.dt[i].no == PayData.teiki.id &&/* 再中止?						*/
							tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*		*/
						{											/*								*/
							f_SameData = 1;							/*								*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					if( 0 == f_SameData )							/* 同一ﾃﾞｰﾀなし(新規登録必要)	*/
					{												/*								*/
						if( i == TKI_CYUSI_MAX ){					/* 登録件数Full					*/
							NTNET_Snd_Data219(1, &tki_cyusi.dt[0]);	/* 1件削除通知送信				*/
							TKI_Delete(0);							/* 最古ﾃﾞｰﾀ削除					*/
							nmisave( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* 中止定期ﾃﾞｰﾀ退避		*/
						}											/*								*/
						tki_cyusi.dt[tki_cyusi.wtp].syubetu =		/*								*/
													PayData.syu;	/* 料金種別(A～L:1～12)			*/
						tki_cyusi.dt[tki_cyusi.wtp].pk =			/*								*/
							CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];/* 駐車場№						*/
						tki_cyusi.dt[tki_cyusi.wtp].no =			/*								*/
												PayData.teiki.id;	/* 個人ｺｰﾄﾞ(1～12000)			*/
						tki_cyusi.dt[tki_cyusi.wtp].tksy =			/*								*/
												PayData.teiki.syu;	/* 定期種別(1～15)				*/
						tki_cyusi.dt[tki_cyusi.wtp].year =			/* 処理年						*/
											PayData.TInTime.Year;	/*								*/
						memcpy( &tki_cyusi.dt[tki_cyusi.wtp].mon,	/*								*/
								PayData.teiki.t_tim, 4 );			/* 処理月日時分					*/
						tki_cyusi.dt[tki_cyusi.wtp].sec = 0;		/* 処理秒						*/
																	/*								*/
						NTNET_Snd_Data219(0, &tki_cyusi.dt[tki_cyusi.wtp]);	/* 1件更新通知送信		*/
																	/*								*/
						if( tki_cyusi.count < TKI_CYUSI_MAX )		/*								*/
						{											/*								*/
							tki_cyusi.count++;						/* 登録件数を+1					*/
						}											/*								*/
																	/*								*/
						tki_cyusi.wtp++;							/* ﾗｲﾄﾎﾟｲﾝﾀ+1					*/
						if( tki_cyusi.wtp >= TKI_CYUSI_MAX )		/*								*/
						{											/*								*/
							tki_cyusi.wtp = 0;						/*								*/
						}											/*								*/
					}												/*								*/
					WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* 定期券ﾃｰﾌﾞﾙ更新(出庫)	*/
								  (ushort)PayData.teiki.id,			/*								*/
								  1,								/*								*/
								  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*				*/
								  0xffff );							/*								*/
				}													/*								*/
				else{												/*								*/
					Log_regist( LOG_PAYSTOP );						/* 精算中止情報登録				*/
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime精算中止データ送信する
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
							Set_Cancel_RTPay_Data();
							// ﾘｱﾙﾀｲﾑ精算ﾃﾞｰﾀのｾﾝﾀｰ追番を更新してlogに書き込む処理
							RTPay_LogRegist_AddOiban();
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						}
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						// QR確定・取消データ登録
						ope_SendCertifCancel();
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
				}													/*								*/
_after_can_log2_regist:
																	/*								*/
				CountUp( CANCEL_COUNT );							/* 精算中止追番+1				*/
				if( ryo_buf.fusoku != 0 ){							/* 預り証発行?					*/
					CountUp(DEPOSIT_COUNT);							/* 預り証追番+1					*/
				}
				if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
					RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
				}
// MH810100(S) K.Onodera  2020/01/23 車番チケットレス(QR確定・取消データ対応)
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
				if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					DC_PopCenterSeqNo( DC_SEQNO_QR );	// センター追番をPop
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
				}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) K.Onodera  2020/01/23 車番チケットレス(QR確定・取消データ対応)
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
				if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
					PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
					(PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0)) {
					// 決済中に障害発生、または、未了残高照会タイムアウトしたため、
					// 処理未了取引記録を登録する
					ac_flg.cycl_fg = 100;							// 処理未了取引記録登録開始
					EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
																	/*								*/
				ac_flg.cycl_fg = 27;								/* 27:精算中止ﾃﾞｰﾀ登録完了		*/
			case 27:												/* 精算中止ﾃﾞｰﾀ登録完了			*/
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
_after_can_log3_regist:
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
				safecl( 7 );										/* 金庫枚数算出、釣銭管理集計	*/
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//				Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );/* 中止時の金銭管理ログデータ作成	*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* 金銭管理ログ登録				*/
				if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {/* 中止時の金銭管理ログデータ作成	*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
				break;												/*								*/
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
			case 100:												// 処理未了取引記録登録開始
				// 決済中に障害発生、または、未了残高照会タイムアウトしたため、
				// 処理未了取引記録を登録する
				EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				ac_flg.cycl_fg = 27;								/* 27:精算中止ﾃﾞｰﾀ登録完了		*/
				goto	_after_can_log3_regist;
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
																	/* ***** 不正・強制 *****		*/
			case 32:												/* Ｔ集計をﾜｰｸｴﾘｱへ転送完了		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
				syasitu = fusei.fus_d[0].t_iti - 1;					/*								*/
				lkts = &loktl.tloktl.loktldat[syasitu];				/*								*/
				memcpy( lkts, &wkloktotal, sizeof( LOKTOTAL_DAT ) );/* ﾜｰｸｴﾘｱをＴ集計へ(停電対策)	*/
																	/*								*/
				ac_flg.cycl_fg = 31;								/*								*/
			case 31:
				if( fus_syuu() == 2 ){								/*								*/
					if( prm_get(COM_PRM,S_NTN,63,1,5) == 0 ){		/* 即時送信しない設定の場合		*/
						ntautoReqToSend( NTNET_BUFCTRL_REQ_SALE );	/* 精算データ送信要求			*/
					}												/*								*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			case 81:
				logwork.flp.Kikai_no = (uchar)CPrmSS[S_PAY][2];		// 機械No.

				logwork.flp.In_Time.Year = car_in_f.year;				/* 入庫年月日時分 	*/
				logwork.flp.In_Time.Mon  = car_in_f.mon;
				logwork.flp.In_Time.Day  = car_in_f.day;
				logwork.flp.In_Time.Hour = car_in_f.hour;
				logwork.flp.In_Time.Min  = car_in_f.min;
				logwork.flp.Date_Time.Year = car_ot_f.year;				/* 精算年月日時分 	*/
				logwork.flp.Date_Time.Mon  = car_ot_f.mon;
				logwork.flp.Date_Time.Day  = car_ot_f.day;
				logwork.flp.Date_Time.Hour = car_ot_f.hour;
				logwork.flp.Date_Time.Min  = car_ot_f.min;

				num = ryo_buf.pkiti - 1;
				logwork.flp.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	/* 駐車位置 		*/

				logwork.flp.Lok_No = num +1;

				if( fusei.fus_d[0].kyousei == 1 || fusei.fus_d[0].kyousei == 11 ){
					CountGet( KIYOUSEI_COUNT, &logwork.flp.count );
					logwork.flp.Lok_inf = 0;							/* 強制 			*/
				}
				else{					// 0：通常不正、2：修正不正(料金あり)、3：修正不正(料金なし)
					CountGet( FUSEI_COUNT, &logwork.flp.count );
					logwork.flp.Lok_inf = 1;							/* 不正 			*/
					if( fusei.fus_d[0].kyousei != 0 ){
						logwork.flp.Lok_inf = 2;						/* 修正精算による不正 */
					}
				}
				if( fusei.fus_d[0].kyousei == 3 ){
					logwork.flp.fusei_fee = 0;						/* 駐車料金 		*/
				}else{
					logwork.flp.fusei_fee = ryo_buf.dsp_ryo;		/* 駐車料金 		*/
				}
				logwork.flp.ryo_syu = ryo_buf.tik_syu;				/* 料金種別 		*/

				memcpy( &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp], &logwork.flp, sizeof( flp_log ) );
				Log_Write(eLOG_ABNORMAL, &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp], TRUE);// 不正・強制出庫ログ登録
			case 34:												/* 不正・強制出庫情報ﾜｰｸ登録完了*/
				if( PrnJnlCheck() == ON ){
					//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
					FusKyoPreqData.prn_kind = J_PRI;											// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
					FusKyoPreqData.prn_data = &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp];	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
					queset( PRNTCBNO, PREQ_FUSKYO_JOU, sizeof(T_FrmFusKyo), &FusKyoPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
				}
				if(logwork.flp.Lok_inf == 0){
					IoLog_write(IOLOG_EVNT_FORCE_FIN, (ushort)FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].WPlace, 0, 0);
					CountUp( KIYOUSEI_COUNT );													//強制出庫追い番
				}else{
					IoLog_write(IOLOG_EVNT_OUT_ILLEGAL, (ushort)FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].WPlace, 0, 0);
					CountUp( FUSEI_COUNT );														//不正出庫追い番
					if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
						RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
					}
				}
				FLP_LOG_DAT.Flp_wtp++;															// ﾗｲﾄﾎﾟｲﾝﾀ +1
				if( FLP_LOG_DAT.Flp_wtp >= FLP_LOG_CNT ){
					FLP_LOG_DAT.Flp_wtp = 0;
				}
				ac_flg.cycl_fg = 35;
				goto	_after_abnormal_log_regist;
																	/*								*/
			case 33:												/* Ｔｴﾘｱ集計加算完了			*/
				Log_regist( LOG_ABNORMAL );							/* 不正・強制出庫情報			*/
				ac_flg.cycl_fg += 1;								/* 35							*/
																	/*								*/
			case 35:												/* 不正・強制出庫情報登録完了	*/
_after_abnormal_log_regist:
				memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );/* 不正ﾃﾞｰﾀをﾜｰｸｴﾘｱへ			*/
				ac_flg.cycl_fg += 1;								/* 36							*/
																	/*								*/
			case 36:												/* 不正ﾃﾞｰﾀをﾜｰｸｴﾘｱへ転送完了	*/
				memcpy( &fusei, &wkfus, sizeof( struct FUSEI_SD ) );/*								*/
																	/*								*/
				memcpy( &fusei, &wkfus.fus_d[1],					/* ﾃﾞｰﾀｼﾌﾄ						*/
						sizeof(struct FUSEI_D)*(LOCK_MAX-1) );		/*								*/
				fusei.kensuu -= 1;									/*								*/
				ac_flg.cycl_fg += 1;								/* 37							*/
																	/*								*/
			case 37:												/* ﾃﾞｰﾀｼﾌﾄ・件数-1完了			*/
				memset( &fusei.fus_d[(LOCK_MAX-1)],					/*								*/
						0, sizeof(struct FUSEI_D));					/*								*/
				ac_flg.cycl_fg += 1;								/* 38							*/
				break;												/*								*/
																	/*								*/
																	/*  ﾌﾗｯﾌﾟ上昇・ﾛｯｸ閉ﾀｲﾑ内出庫	*/
			case 42:
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
				syasitu = locktimeout.fus_d[0].t_iti - 1;			/*								*/
				lkts = &loktl.tloktl.loktldat[syasitu];				/*								*/
				memcpy( lkts, &wkloktotal, sizeof( LOKTOTAL_DAT ) );/* ﾜｰｸｴﾘｱをＴ集計へ(停電対策)	*/
																	/*								*/
			case 41:
				lto_syuu();											/*								*/
				break;												/*								*/
			case 44:
			
			case 43:
				PayData_set_LO(locktimeout.fus_d[0].t_iti,0,0,97);	/*								*/
				Log_Write(eLOG_PAYMENT, &PayData, TRUE);			/*								*/
			case 45:
				memcpy( &wklocktimeout, &locktimeout, sizeof( struct FUSEI_SD ) );	/*								*/
			case 46:
				memcpy( &locktimeout, &wklocktimeout.fus_d[1],		/* ﾃﾞｰﾀｼﾌﾄ						*/
						sizeof(struct FUSEI_D)*(LOCK_MAX-1) );		/*								*/
				locktimeout.kensuu -= 1;							/* 件数-1						*/
			case 47:
				memset( &locktimeout.fus_d[(LOCK_MAX-1)],			/*								*/
						0, sizeof(struct FUSEI_D));					/*								*/
				ac_flg.cycl_fg = 48;								/* 48							*/
				break;
																	/* ***** 精算途中の停電 *****	*/
			case 51:												/* Ｔ集計をﾜｰｸｴﾘｱへ転送完了		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
				DailyAggregateDataBKorRES( 1 );						/* ﾊﾞｯｸｱｯﾌﾟﾃﾞｰﾀのﾘｽﾄｱ			*/
																	/*								*/
			case 50:												/* 入金中停電復帰集計開始		*/
				toty_syu();											/* 精算途中停電復帰集計			*/
				break;												/*								*/
																	/*								*/
			case 53:												/* 精算中止ﾃﾞｰﾀﾜｰｸ登録完了		*/
				memcpy( &tki_cyusi, &tki_cyusi_wk,					/*								*/
						sizeof(t_TKI_CYUSI) );						/* 退避中止定期ﾃﾞｰﾀを元に戻す	*/
				if( PrnJnlCheck() == ON ){
					//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
					ReceiptPreqData.prn_kind = J_PRI;				// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
					ReceiptPreqData.prn_data = &Cancel_pri_work;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
					ReceiptPreqData.reprint = OFF;					// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
				}
				IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
				goto	_after_can_log21_regist;
			case 55:												/* 精算中止ﾃﾞｰﾀﾜｰｸ登録完了		*/
				if( PrnJnlCheck() == ON ){
					//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続ありの場合
					ReceiptPreqData.prn_kind = J_PRI;				// 出力ﾌﾟﾘﾝﾀ（ｼﾞｬｰﾅﾙ）
					ReceiptPreqData.prn_data = &Cancel_pri_work;	// ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
					ReceiptPreqData.reprint = OFF;					// 再発行ﾌﾗｸﾞﾘｾｯﾄ（通常）
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// 印字要求ﾒｯｾｰｼﾞ登録
				}
				IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
				goto	_after_can_log22_regist;
																	/*								*/
			case 52:												/* Ｔｴﾘｱ集計加算完了			*/
				if( ryo_buf.ryo_flg >= 2 )							/* 定期使用?					*/
				{													/*								*/
					memcpy( &tki_cyusi_wk,							/*								*/
							&tki_cyusi,								/*								*/
							sizeof(t_TKI_CYUSI) );					/* 中止定期ﾃﾞｰﾀ退避				*/
																	/*								*/
					Log_regist( LOG_PAYSTOP );						/* 精算中止情報登録				*/
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime精算中止データ送信する
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
							Set_Cancel_RTPay_Data();
							// ﾘｱﾙﾀｲﾑ精算ﾃﾞｰﾀのｾﾝﾀｰ追番を更新してlogに書き込む処理
							RTPay_LogRegist_AddOiban();
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						}
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						// QR確定・取消データ登録
						ope_SendCertifCancel();
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
_after_can_log21_regist:
																	/*								*/
					f_SameData = 0;									/*								*/
																	/*								*/
					if( TKI_CYUSI_MAX < tki_cyusi.count )			/* fail safe					*/
						tki_cyusi.count = TKI_CYUSI_MAX;			/*								*/
																	/*								*/
					if( TKI_CYUSI_MAX <= tki_cyusi.wtp )			/* fail safe					*/
						tki_cyusi.wtp = tki_cyusi.count - 1;		/*								*/
																	/*								*/
					for( i=0; i<tki_cyusi.count; i++ )				/*								*/
					{												/*								*/
						if( 0L == tki_cyusi.dt[i].pk ){				/* ﾃｰﾌﾞﾙにﾃﾞｰﾀなし				*/
							break;									/* i=登録件数					*/
						}											/*								*/
						if( tki_cyusi.dt[i].no == PayData.teiki.id &&/* 再中止?						*/
							tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*		*/
						{											/*								*/
							f_SameData = 1;							/*								*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					if( 0 == f_SameData )							/* 同一ﾃﾞｰﾀなし(新規登録必要)	*/
					{												/*								*/
						if( i == TKI_CYUSI_MAX ){					/* 登録件数Full					*/
							NTNET_Snd_Data219(1, &tki_cyusi.dt[0]);	/* 1件削除通知送信				*/
							TKI_Delete(0);							/* 最古ﾃﾞｰﾀ削除					*/
							nmisave( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* 中止定期ﾃﾞｰﾀ退避	*/
						}											/*								*/
						tki_cyusi.dt[tki_cyusi.wtp].syubetu =		/*								*/
													PayData.syu;	/* 料金種別(A～L:1～12)			*/
						tki_cyusi.dt[tki_cyusi.wtp].pk =			/*								*/
							CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];/* 駐車場№						*/
						tki_cyusi.dt[tki_cyusi.wtp].no =			/*								*/
												PayData.teiki.id;	/* 個人ｺｰﾄﾞ(1～12000)			*/
						tki_cyusi.dt[tki_cyusi.wtp].tksy =			/*								*/
										PayData.teiki.syu;			/* 定期種別(1～15)				*/
						tki_cyusi.dt[tki_cyusi.wtp].year =			/* 処理年						*/
											PayData.TInTime.Year;	/*								*/
						memcpy( &tki_cyusi.dt[tki_cyusi.wtp].mon,	/*								*/
								PayData.teiki.t_tim, 4 );			/* 処理月日時分					*/
						tki_cyusi.dt[tki_cyusi.wtp].sec = 0;		/* 処理秒						*/
																	/*								*/
						NTNET_Snd_Data219(0, &tki_cyusi.dt[tki_cyusi.wtp]);	/* 1件更新通知送信		*/
																	/*								*/
						if( tki_cyusi.count < TKI_CYUSI_MAX )		/*								*/
						{											/*								*/
							tki_cyusi.count++;						/* 登録件数を+1					*/
						}											/*								*/
																	/*								*/
						tki_cyusi.wtp++;							/* ﾗｲﾄﾎﾟｲﾝﾀ+1					*/
						if( tki_cyusi.wtp >= TKI_CYUSI_MAX )		/*								*/
						{											/*								*/
							tki_cyusi.wtp = 0;						/*								*/
						}											/*								*/
					}												/*								*/
					WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* 定期券ﾃｰﾌﾞﾙ更新(出庫)	*/
								  (ushort)PayData.teiki.id,			/*								*/
								  1,								/*								*/
								  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*				*/
								  0xffff );							/*								*/
				}													/*								*/
				else{												/*								*/
					Log_regist( LOG_PAYSTOP );						/* 精算中止情報登録				*/
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime精算中止データ送信する
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
							Set_Cancel_RTPay_Data();
							// ﾘｱﾙﾀｲﾑ精算ﾃﾞｰﾀのｾﾝﾀｰ追番を更新してlogに書き込む処理
							RTPay_LogRegist_AddOiban();
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						}
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
						// QR確定・取消データ登録
						ope_SendCertifCancel();
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
				}													/*								*/
_after_can_log22_regist:
																	/*								*/
				CountUp( CANCEL_COUNT );							/* 精算中止追番+1				*/
				if( ryo_buf.fusoku != 0 ){							/* 預り証発行?					*/
					CountUp(DEPOSIT_COUNT);							/* 預り証追番+1					*/
				}
// MH810100(S) K.Onodera  2020/01/22 車番チケットレス(QR確定・取消データ対応)
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
				if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
					DC_PopCenterSeqNo( DC_SEQNO_QR );	// センター追番をPop
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
				}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) K.Onodera  2020/01/22 車番チケットレス(QR確定・取消データ対応)
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
				if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
					PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
					(PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0)) {
					// 決済中に障害発生、または、未了残高照会タイムアウトしたため、
					// 処理未了取引記録を登録する
					ac_flg.cycl_fg = 110;							// 処理未了取引記録登録開始
					EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
																	/*								*/
				ac_flg.cycl_fg = 54;								/* 54:精算中止ﾃﾞｰﾀ登録完了		*/
				break;												/*								*/
																	/*								*/
			case 54:												/* 精算中止ﾃﾞｰﾀ登録完了			*/
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
_after_can_log23_regist:
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
				safecl( 7 );										/* 金庫枚数算出、釣銭管理集計	*/
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//				Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );/* 中止時の金銭管理ログデータ作成	*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* 金銭管理ログ登録				*/
				if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {/* 中止時の金銭管理ログデータ作成	*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* 金銭管理ログ登録				*/
				}
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
				break;												/*								*/
																	/*								*/
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
			case 110:												// 処理未了取引記録登録開始
				// 決済中に障害発生、または、未了残高照会タイムアウトしたため、
				// 処理未了取引記録を登録する
				EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				ac_flg.cycl_fg = 54;								/* 54:精算中止ﾃﾞｰﾀ登録完了		*/
				goto	_after_can_log23_regist;
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
// MH321800(S) T.Nagai ICクレジット対応
			case 57:												/* 57: 決済精算中止時			*/
				Log_Write(eLOG_PAYMENT, &EcRecvDeemedData, TRUE);
				// no break
			case 58:												/* 58: 決済精算中止データ受付	*/
				if( PrnJnlCheck() == ON ){
					ReceiptPreqData.prn_kind = J_PRI;				/* ﾌﾟﾘﾝﾀ種別：ｼﾞｬｰﾅﾙ			*/
					ReceiptPreqData.prn_data = &EcRecvDeemedData;	/* 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ		*/

					// pritaskへ通知
					queset(PRNTCBNO, PREQ_RECV_DEEMED, sizeof(T_FrmReceipt), &ReceiptPreqData);
				}
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
				// 詳細エラーコードを登録する
				EcErrCodeChk( EcRecvDeemedData.EcErrCode, EcRecvDeemedData.Electron_data.Ec.e_pay_kind );
				EcRecvDeemedData.WFlag = 1;							/* 直前取引ﾃﾞｰﾀ復電印字完了ﾌﾗｸﾞON  */
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
				break;
// MH321800(E) T.Nagai ICクレジット対応

			default:												/*								*/
				ac_flg.cycl_fg = 0;									/*								*/
				break;												/*								*/
		}															/*								*/
		ac_flg.cycl_fg = 0;											/*								*/
	}																/*								*/

	if( sy != 0 )													// 集計印字から集計加算中(合計)の停電有り？
	{																/*								*/
		switch( sy )												/*								*/
		{															/* ***** Ｔ合計 *****			*/
			case 11:
				Make_Log_TGOUKEI();									/* 車室毎集計を含めたT合計ﾛｸﾞ作成*/
				if( prm_get(COM_PRM, S_NTN, 26, 1, 3) ){			// Ｔ合計送信する
					NTNET_Snd_TGOUKEI();
				}
				syuukei_clr( 0 );									/*								*/
				break;												/*								*/

			case 13:												/* ﾜｰｸｴﾘｱへ転送済み				*/
																	/* remote.ﾊﾞｯｸｱｯﾌﾟ（停電対策）*/
				memcpy( gs, ws, sizeof( sky.gsyuk ) );				/* ﾜｰｸｴﾘｱをＧＴ集計へ（停電対策）*/
				memcpy( loktl.gloktl.loktldat, &wkloktotal, sizeof( LOKTOTAL_DAT ) ); /* ﾜｰｸｴﾘｱを車室毎GT集計へ(停電対策)	*/
				memcpy( &CarCount, &CarCount_W, sizeof( CAR_COUNT ) );	/* 入出庫ｶｳﾝﾄを元に戻す		*/
				ac_flg.syusyu = 12;									/* 12:Ｔ合計印字完了			*/
																	/*								*/
			case 12:												/* Ｔ合計印字完了				*/
				syuukei_clr( 0 );									/*								*/
				if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// バッチ送信設定有
	 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
						ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
	 				}
					else {
						ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
					}
				}
				break;												/*								*/
																	/*								*/
			case 15:												/* Ｔ合計情報ﾜｰｸ登録完了		*/
// MH810100(S) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// Log_Write()/Log_Write_Pon()内でeLOG_LCKTTLの場合はac_flg.syusyu=19としているが、ここででac_flg.syusyu=19
// としてもcase 19では結局_after_syu_log_registに飛ぶだけなので、単純に車室毎集計ログ登録をスキップする
//-				// case 15のリカバリはLog_Write_Pon()で処理する
//-				Log_Write(eLOG_LCKTTL, &LCKT_LOG_WK, TRUE);			// 車室毎集計ログ(31車室以降)登録
// MH810100(E) K.Onodera  2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
				goto	_after_syu_log_regist;
																	/*								*/
			case 14:												/* Ｔ集計をＧＴ集計へ加算完了	*/
				Log_regist( LOG_TTOTAL );							/* Ｔ合計情報登録				*/

_after_syu_log_regist:
				ac_flg.syusyu = 16;									/* 16:Ｔ合計情報登録完了		*/

				if( prm_get(COM_PRM, S_SCA, 11, 1, 1) == 1 )		/* 共通ﾊﾟﾗﾒｰﾀの設定値が有効範囲内の場合	*/
					Suica_Rec.Data.BIT.FUKUDEN_SET = 1;				/* 精算データ送信要求あり		*/

				ds = &sky.tsyuk;		// dst = T					/* Ｔ集計						*/
				memcpy( ws, ds, sizeof( SYUKEI ) );					/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
				ac_flg.syusyu = 17;									/* 17:今回時刻をﾜｰｸ登録完了		*/
				memset( ds, 0, _SYU_HDR_SIZ );						/* ヘッダー部クリア				*/
				ss = &ds->Uri_Tryo;									/*								*/
				as = &ts->Uri_Tryo;									/* Ｔ－印字Ｔ					*/ 
				for( i = 0; i < _SYU_DAT_CNT; i++, ss++, as++ )		/*								*/
				{													/*								*/
					*ss -= *as;										/*								*/
				}													/*								*/
				memcpy( &ds->OldTime, &skybk.tsyuk.NowTime, sizeof( date_time_rec ) ); /* 前回集計時刻ｾｯﾄ			*/
				ac_flg.syusyu = 18;									/* 18:Ｔ集計ｴﾘｱｸﾘｱ完了			*/
				CountUp( T_TOTAL_COUNT );							/* T合計追番+1					*/
				CountClear( CLEAR_COUNT_T );						/* 追番ｸﾘｱ(T合計完了時)			*/
				break;												/*								*/
																	/*								*/
			case 19:												/* 集計ログ登録完了				*/
			// フラッシュが先に復電を行うため、処理済みならばSyu_countは0である
				goto	_after_syu_log_regist;
				
																	/*								*/
			case 16:												/* 16:Ｔ合計情報登録完了		*/
				if( prm_get(COM_PRM, S_SCA, 11, 1, 1) == 1 )		/* 共通ﾊﾟﾗﾒｰﾀの設定値が有効範囲内の場合	*/
					Suica_Rec.Data.BIT.FUKUDEN_SET = 1;				/* 精算データ送信要求あり		*/
				ds = &sky.tsyuk;		// dst = T					/* Ｔ集計						*/
				memcpy( ws, ds, sizeof( SYUKEI ) );					/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
				ac_flg.syusyu = 17;									/* 17:今回時刻をﾜｰｸ登録完了		*/
			case 17:												/* 今回時刻をﾜｰｸ登録完了		*/
				ds = &sky.tsyuk;		// dst = T					/* Ｔ集計						*/
				memset( ds, 0, _SYU_HDR_SIZ );						/* ヘッダー部クリア				*/
				ss = &ds->Uri_Tryo;									/*								*/
				as = &ts->Uri_Tryo;									/* Ｔ－印字Ｔ					*/ 
				for( i = 0; i < _SYU_DAT_CNT; i++, ss++, as++ )		/*								*/
				{													/*								*/
					*ss -= *as;										/*								*/
				}													/*								*/
				memcpy( &ds->OldTime, &skybk.tsyuk.NowTime, sizeof( date_time_rec ) ); /* 前回集計時刻ｾｯﾄ			*/
				memset( &loktl.tloktl, 0, sizeof( LOKTOTAL ) );		/* 車室毎集計をｸﾘｱ				*/
				ac_flg.syusyu = 18;									/* 18:Ｔ集計ｴﾘｱｸﾘｱ完了			*/
				CountUp( T_TOTAL_COUNT );							/* T合計追番+1					*/
				CountClear( CLEAR_COUNT_T );						/* 追番ｸﾘｱ(T合計完了時)			*/
				if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
					RAU_UpdateCenterSeqNo(RAU_SEQNO_TOTAL);
				}
				break;												/*								*/
																	/*								*/
																	/* ***** ＧＴ合計 *****			*/
			case 25:												/* ＧＴ合計ログ登録受付			*/
				// case 25のリカバリはLog_Write_Pon()で処理する
				goto	_after_gsyu_log_regist;
			case 23:												/* 今回時刻をﾜｰｸ登録完了		*/
				memcpy( ws, ms, sizeof( SYUKEI ) );					/* ＭＴ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
				ac_flg.syusyu = 22;									/* 22:ＧＴ合計印字完了			*/
																	/*								*/
			case 22:												/* ＧＴ合計印字完了				*/
				memcpy( ws, ms, sizeof( SYUKEI ) );					/* ＭＴ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
				ac_flg.syusyu = 23;									/* 23:ＭＴ集計をﾜｰｸｴﾘｱへ転送完了*/
				memcpy( &skybk.gsyuk, gs, sizeof( SYUKEI ) );		/* ＧＴ集計を前回ＧＴ集計へｾｯﾄ	*/
				memcpy( &loktlbk.gloktl, &loktl.gloktl, sizeof( LOKTOTAL ) ); /* 車室毎集計を前回集計へｾｯﾄ	*/
				if ( prm_get(COM_PRM, S_TOT, 1, 1, 1) != 0) {
					as = &gs->Uri_Tryo;								/*								*/
					ss = &ms->Uri_Tryo;								/*								*/
																	/*								*/
					for( i = 0; i < _SYU_DAT_CNT; i++, as++, ss++ )	/*								*/
					{												/*								*/
						*ss += *as;									/*								*/
					}												/*								*/
					CountAdd(ms, gs);								/* 追番加算						*/
					if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
						gs->CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_TOTAL);
					}
				}

				ac_flg.syusyu = 24;									/* 24:ＧＴ集計をＭＴ集計へ加算完了*/
			case 24:												/* ＧＴ集計をＭＴ集計へ加算完了	*/
				Log_regist( LOG_GTTOTAL );							/* ＧＴ合計情報登録				*/

_after_gsyu_log_regist:

				ac_flg.syusyu = 26;									/* 26:ＧＴ合計情報登録完了		*/
			case 26:												/* ＧＴ合計情報登録完了			*/
				memset( gs, 0, sizeof( sky.gsyuk ) );				/* ＧＴ集計ｴﾘｱｸﾘｱ				*/
				memcpy( &gs->OldTime, &skybk.gsyuk.NowTime, sizeof( date_time_rec ) );	/* 前回集計時刻ｾｯﾄ		*/
				memset( &loktl.gloktl, 0, sizeof( LOKTOTAL ) );		/* 車室毎集計をｸﾘｱ				*/
																	/*								*/
				ac_flg.syusyu = 27;									/* 27:ＧＴ集計ｴﾘｱｸﾘｱ完了		*/
																	/*								*/
				if ( prm_get(COM_PRM, S_TOT, 1, 1, 1) != 0) {
					CountUp( GT_TOTAL_COUNT );						/* GT合計追番+1					*/
				}
				CountClear( CLEAR_COUNT_GT );						/* 追番ｸﾘｱ(GT合計完了時)		*/
				if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
					RAU_UpdateCenterSeqNo(RAU_SEQNO_TOTAL);
				}
				break;												/*								*/
																	/*								*/
																	/* ***** ＭＴ合計 *****			*/
			case 102:												/* ＭＴ合計印字完了				*/
				syuukei_clr( 2 );									/*								*/
				break;												/*								*/
																	/*								*/
			case 107:												/* ＭＴ集計完了					*/
				memset( ms, 0, sizeof( SYUKEI ) );					/* ＭＴｴﾘｱｸﾘｱ					*/
				memcpy( &ms->OldTime, &skybk.msyuk.NowTime, sizeof( date_time_rec ) ); /* 前回集計時刻ｾｯﾄ			*/
				CountClear( CLEAR_COUNT_MT );						/* 追番ｸﾘｱ(MT合計完了時)		*/
				break;												/*								*/
																	/* ***** コイン金庫集計 *****	*/
			case 33:												/* Ｔ集計をﾜｰｸｴﾘｱへ転送完了		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
				ac_flg.syusyu = 32;									/* 32:印字完了					*/
																	/*								*/
			case 32:												/* 印字完了						*/
				kinko_clr( 0 );										/* ｺｲﾝ金庫集計印字完了処理		*/
				break;												/*								*/
																	/*								*/
			case 35:												/* ｺｲﾝ金庫集計情報ﾜｰｸ登録完了	*/
			Make_Log_MnyMng( 10 );									// 金銭管理ログデータ作成
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);			// 金銭管理ログ登録
			goto	_after_money_change_log_coin_regist;
																	/*								*/
			case 34:												/* 金庫集計をＴ集計へ転送完了	*/
				Log_regist( LOG_COINBOX );							/* ｺｲﾝ金庫集計情報加算			*/
				// Don't break										/*								*/
																	/*								*/
			case 36:												/* 金庫集計情報登録完了			*/
_after_money_change_log_coin_regist:
				memset( &SFV_DAT.safe_dt[0],						/*								*/
							0, sizeof( SFV_DAT.safe_dt ));			/*								*/
				memcpy( &coin_syu.OldTime,							/*								*/
					&coin_syu.NowTime, sizeof( date_time_rec ) );	/* 前回集計時刻ｾｯﾄ				*/
				memset( &coin_syu.Tryo, 0, sizeof(ulong) * (1+(COIN_SYU_CNT*2)) );	/*				*/
				ac_flg.syusyu = 37;									/* 37:金庫集計ｸﾘｱ完了			*/
				CountUp( COIN_SAFE_COUNT );							/* ｺｲﾝ金庫追番+1				*/
				break;												/*								*/
																	/*								*/
																	/* ***** 紙幣金庫集計 *****		*/
			case 43:												/* Ｔ集計をﾜｰｸｴﾘｱへ転送完了		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
				ac_flg.syusyu = 42;									/* 42:印字完了					*/
																	/*								*/
			case 42:												/* 印字完了						*/
				kinko_clr( 1 );										/* 紙幣金庫集計印字完了処理		*/
				break;												/*								*/
																	/*								*/
			case 45:												/* 紙幣金庫集計情報ﾜｰｸ登録完了	*/
				Make_Log_MnyMng( 11 );								// 金銭管理ログデータ作成
				Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);		// 金銭管理ログ登録
				goto	_after_money_change_log_note_regist;
																	/*								*/
			case 44:												/* 金庫集計をＴ集計へ転送完了	*/
				Log_regist( LOG_NOTEBOX );							/* 紙幣金庫集計情報加算			*/
				// Don't break										/*								*/
																	/*								*/
			case 46:												/* 金庫集計情報登録完了			*/
_after_money_change_log_note_regist:
				SFV_DAT.nt_safe_dt = 0;								/*								*/
				memcpy( &note_syu.OldTime,							/*								*/
					&note_syu.NowTime, sizeof( date_time_rec ) );	/* 前回集計時刻ｾｯﾄ				*/
				memset( &note_syu.Tryo, 0, sizeof(ulong) * (1+(SIHEI_SYU_CNT*2)) );/*		*/
					ac_flg.syusyu = 47;								/* 47:金庫集計ｸﾘｱ完了			*/
				CountUp( NOTE_SAFE_COUNT );							/* 紙幣金庫追番+1				*/
				break;												/*								*/
																	/*								*/
																	/* ***** 釣銭管理集計 *****		*/
			case 53:												/* 金銭管理情報ﾜｰｸ登録完了		*/
				Log_Write(eLOG_MNYMNG_SRAM, &turi_kan, FALSE);								// 金銭管理ログ登録(SRAM)
				goto	_after_money_change_log_regist;
																	/*								*/
			case 52:												/* 印字完了						*/
				turikan_clr();										/* 釣銭管理ｴﾘｱの更新処理		*/
				break;												/*								*/
																	/*								*/
			case 54:												/*								*/
_after_money_change_log_regist:
				memcpy( &turi_kwk,									/*								*/
						&turi_kan, sizeof( TURI_KAN ) );			/*								*/
				ac_flg.syusyu = 55;									/* 55:釣銭管理集計をﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
			case 55:												/* 釣銭管理集計をﾜｰｸｴﾘｱへ転送完了*/
				turikan_clr_sub2();									/*								*/
				ac_flg.syusyu = 56;									/* 56:金銭管理集計ｸﾘｱ完了		*/
				turikan_clr_sub();									/* T集計へ釣銭補充、強制払出を加算 */
																	/*								*/
				ac_flg.syusyu = 58;									/*								*/
				break;												/*								*/
			case 57:												/* 釣銭補充、強制払出をT集計へ加算前 */
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/
				ac_flg.syusyu = 56;									/* 55:釣銭管理集計をﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
			case 56:												/* 釣銭補充、強制払出をT集計へ加算前 */
				turikan_clr_sub();									/* T集計へ釣銭補充、強制払出を加算 */
				break;												/*								*/
																	/*								*/
			case 60:												/* 強制払出枚数をT集計へ加算前	*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/

				turikan_inventry( 0 );								/* T集計へ強制払出を加算 		*/
				break;

			case 61:												/* 強制払出枚数をT集計へ加算前	*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ﾜｰｸｴﾘｱをＴ集計へ（停電対策）	*/

				turikan_inventry( 1 );								/* T集計へ強制払出を加算 		*/
				break;
			default:												/*								*/
				ac_flg.syusyu = 0;									/*								*/
				break;												/*								*/
		}															/*								*/
		ac_flg.syusyu = 0;
	}
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
//// 不具合修正(S) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
//	if( FurikaeDestFlapNo ){
//		queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof( FurikaeDestFlapNo ), &FurikaeDestFlapNo );	// 精算完了ｷｭｰｾｯﾄ(ﾌﾗｯﾊﾟｰ下降)
//	}
//// 不具合修正(E) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
	return;
}
