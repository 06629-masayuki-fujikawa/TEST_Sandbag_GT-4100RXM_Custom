/*[]----------------------------------------------------------------------------------------------[]*/
/*| 集計																						   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"prm_tbl.h"
#include	"mem_def.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"flp_def.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"pri_def.h"
#include	"mnt_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"LKmain.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"AppServ.h"
#include	"raudef.h"
#include	"oiban.h"
// MH322914(S) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
#include	"ifm_ctrl.h"
// MH322914(E) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
extern void lcdbm_notice_dsp3( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info, uchar *str, ulong str_size );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
static short lto_tim( void );
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
static uchar date_uriage_cmp_paymentdate( void );
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
static void disc_wari_conv_main( DISCOUNT_DATA *p_disc, wari_tiket *p_wari, uchar opeFlag );
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 精算完了集計																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kan_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT45EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	kan_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	LOKTOTAL_DAT	*lkts;											/*								*/
	ulong	dat1, dat2;												/*								*/
	ushort	iti;													/*								*/
	short	i;														/*								*/
	ulong	wari_ryo = 0;											/* 割引料金						*/
	ulong	ppc_ryo	 = 0;											/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用料金			*/
	uchar	uc_update = PayData.teiki.update_mon;					/* 更新月格納					*/
	ulong	wk_Electron_ryo;										/* 電子ﾏﾈｰ精算金額セーブ		*/
	char	f_BunruiSyu;											/* 1=分類集計する				*/
																	/*								*/
	uchar	wk_ot_car;
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	uchar	wk_sss_flg = 0;
//	uchar	wk_sss_syu = 0;											// 修正元料金種別
//	ushort	wdt1,wdt2,wdt3;
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	uchar	wk_rissu = 0;
	wari_tiket	wari_dt;
	short	j;
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	DATE_SYOUKEI	*date_ts;
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// MH322914(S) K.Onodera 2016/10/11 AI-V対応：遠隔精算
	char	tmp_syu = 0;
	ushort	val = 0;
// MH322914(E) K.Onodera 2016/10/11 AI-V対応：遠隔精算
// 不具合修正(S) K.Onodera 2016/11/28 #1578 集計が実際の金額と一致しない
	ulong	ulFurikae = 0, ulFuriWari = 0, pos = 0;
// 不具合修正(E) K.Onodera 2016/11/28 #1578 集計が実際の金額と一致しない
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
	uchar	taxableAdd_set;											/*	課税対象売上加算設定		*/
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
// MH321800(S) hosoda ICクレジット対応
	struct clk_rec wrk_clk_rec;		/* 集計日時ノーマライズ用 */
	date_time_rec wrk_date;			/* 集計日時ノーマライズ用 */
	int		ec_kind;
	int		syu_idx;				/* 日毎集計のインデックス */
	ulong	sei_date;				/* 精算日時 */
	ulong	sy1_date;				/* 集計日時 */
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
// // GG129004(S) R.Endo 2024/11/19 電子領収証対応
// 	char	qrcode[QRCODE_RECEIPT_SIZE + 4];
// // GG129004(E) R.Endo 2024/11/19 電子領収証対応
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
	memset(&wrk_clk_rec, 0x0, sizeof(wrk_clk_rec));
// MH321800(E) hosoda ICクレジット対応

	if( OPECTL.f_KanSyuu_Cmp )										/* 完了集計実施済み				*/
		return;														/*								*/
	else															/*								*/
		OPECTL.f_KanSyuu_Cmp = 1;									/* 完了集計実施済み				*/
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	date_ts = &Date_Syoukei;
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	iti = ryo_buf.pkiti - 1;										/* 駐車位置						*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
	if(( ryo_buf.pkiti != 0xffff )&&								/* ﾏﾙﾁ精算以外					*/
	   ( uc_update == 0 )){											/* 定期券更新精算以外			*/
		lkts = &loktl.tloktl.loktldat[iti];							/*								*/
		memcpy( &wkloktotal, lkts, sizeof( LOKTOTAL_DAT ) );		/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
	}																/*								*/
	DailyAggregateDataBKorRES( 0 );									/* 日毎集計エリアのバックアップ	*/
																	/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	date_uriage_syoukei_judge();									// 日付切替基準 総売上、現金総売上小計の時刻更新判定（リアルタイム情報用）
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	ac_flg.cycl_fg = 14;											/* 14:Ｔ集計をﾜｰｸｴﾘｱへ転送完了	*/
																	/*								*/
// 仕様変更(S) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
//	dat1 = ryo_buf.nyukin - ryo_buf.turisen;						/* 入金額－釣り銭額				*/
	dat1 = ryo_buf.nyukin - ryo_buf.turisen - ryo_buf.kabarai;		/* 入金額－釣り銭額－振替過払い */
// 仕様変更(E) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
																	/*								*/
																	/*								*/
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	wdt1 = OPECTL.MPr_LokNo - 1;
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	ts->Uri_Tryo += dat1;											/* 総売り上げ					*/
	ts->Genuri_Tryo += dat1;										/* 総現金売り上げ				*/
	ts->Tax_Tryo += ryo_buf.tax;									/* 消費税(内税・外税)			*/
																	/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
		if(date_uriage_cmp_paymentdate() == 0){// 加算可能
			date_ts->Uri_Tryo += dat1;
			date_ts->Genuri_Tryo += dat1;
		}
	}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	if(	(ryo_buf.pkiti != 0xffff)&&(uc_update == 0) ){				/* ﾏﾙﾁ精算、定期更新精算以外	*/

		wk_ot_car = syusei[iti].ot_car;								/*								*/
		wk_rissu = syusei[iti].infofg;
		memset( &syusei[iti], 0, sizeof(struct SYUSEI));			/* 修正先修正精算用ﾃﾞｰﾀｸﾘｱ		*/
		syusei[iti].ot_car = wk_ot_car;								/*								*/
		if( wk_rissu & SSS_RYOUSYU ){								// 修正元で領収証発行あり
			syusei[iti].infofg |= SSS_RYOUSYU;						// 
		}
																	/*								*/
		if( (!FLAPDT.flp_data[iti].passwd) &&						/* 暗証番号入力なしで精算		*/
			(OPECTL.Pay_mod != 2) ){								/* 修正精算でない				*/
																	/*								*/
			syusei[iti].sei = 1;									/* 過去精算有り(修正用)			*/
																	/*								*/
			syusei[iti].tryo = ryo_buf.tyu_ryo;						/* 駐車料金(修正用)				*/
			syusei[iti].gen = dat1;									/* 現金領収額(修正用)			*/
			syusei[iti].tax = ryo_buf.tax;							/* 修正用消費税額				*/
																	/*								*/
			syusei[iti].iyear = car_in_f.year;						/* 入庫 年(修正用)				*/
			syusei[iti].imont = car_in_f.mon;						/*      月						*/
			syusei[iti].idate = car_in_f.day;						/*      日						*/
			syusei[iti].ihour = car_in_f.hour;						/*      時						*/
			syusei[iti].iminu = car_in_f.min;						/*      分						*/
			syusei[iti].oyear = car_ot_f.year;						/* 出庫 年(修正用)				*/
			syusei[iti].omont = car_ot_f.mon;						/*      月						*/
			syusei[iti].odate = car_ot_f.day;						/*      日						*/
			syusei[iti].ohour = car_ot_f.hour;						/*      時						*/
			syusei[iti].ominu = car_ot_f.min;						/*      分						*/
																	/*								*/
			syusei[iti].sy_wmai = card_use[USE_SVC];				/* 使用枚数(修正用)				*/
			syusei[iti].syubetu = (uchar)(ryo_buf.syubet+1);		/* 料金種別						*/
			CountGet( PAYMENT_COUNT, &syusei[iti].oiban );			/* 精算追番						*/
		}															/*								*/
		FLAPDT.flp_data[iti].bk_syu = 0;							/* 種別(中止)ｸﾘｱ				*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ ){								/* 割引料金合計計算				*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );
		disc_wari_conv_all( &PayData.DiscountData[i], &wari_dt );
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
		switch( wari_dt.tik_syu ){									/*								*/
			case	SERVICE:										/* ｻｰﾋﾞｽ券						*/
			case	KAKEURI:										/* 掛売券						*/
// MH810100(S) K.Onodera  2020/01/29 車番チケットレス(割引済み対応/再精算_領収証印字)
			case	C_SERVICE:										/* 精算中止ｻｰﾋﾞｽ券				*/
			case	C_KAKEURI:										/* 精算中止店割引				*/
// MH810100(E) K.Onodera  2020/01/29 車番チケットレス(割引済み対応/再精算_領収証印字)
			case	KAISUU:											/* 回数券						*/
			case	MISHUU:											/* 未収金						*/
			case	FURIKAE:										/* 振替額						*/
			case	SYUUSEI:										/* 修正額						*/
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
			case SHOPPING:											/* 買物割引						*/
			case C_SHOPPING:
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
				wari_ryo += wari_dt.ryokin;							/* 割引料金加算					*/
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//				// 修正精算の場合には修正元の修正用データをクリアする
//				if( wari_dt.tik_syu == SYUUSEI ){
//					wk_sss_syu = syusei[wdt1].syubetu;
//					if( syusei[wdt1].sei == 1 ){
//						syusei[wdt1].sei = 0;						// 修正元 修正ﾊﾞｯﾌｧｸﾘｱ
//					}
//					wk_ot_car = syusei[iti].ot_car;
//					memset( &syusei[iti], 0, sizeof(struct SYUSEI));/* 修正先ﾃﾞｰﾀｸﾘｱ				*/
//					syusei[iti].ot_car = wk_ot_car;
//					wk_sss_flg = 1;
//				}
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
				break;												/*								*/
			case	PREPAID:										/* プリペイドカード				*/
				ppc_ryo += wari_dt.ryokin;							/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用料金加算		*/
				break;												/*								*/
		}															/*								*/
	}																/*								*/

	if( PayData.teiki.Apass_off_seisan ){							/* 強制ｱﾝﾁﾊﾟｽOFF精算あり？		*/
		ts->Apass_off_seisan += 1;									/* 強制ｱﾝﾁﾊﾟｽOFF精算回数+1		*/
		syusei[iti].infofg |= SSS_ANTIOFF;							// 修正元強制ｱﾝﾁﾊﾟｽOFF情報登録
	}

	if( PayData.BeforeTwari )										/* 前回時間割引金額あり?		*/
		wari_ryo += PayData.BeforeTwari;							/* 前回時間割引金額加算			*/
	if( PayData.BeforeRwari )										/* 前回料金割引金額あり?		*/
		wari_ryo += PayData.BeforeRwari;							/* 前回料金割引金額加算			*/
// MH810100(S) K.Onodera  2020/01/29 車番チケットレス(割引済み対応)
	if( PayData.zenkai ){											/* 前回領収額あり?				*/
		wari_ryo += PayData.zenkai;									/* 前回領収額加算				*/
	}
// MH810100(E) K.Onodera  2020/01/29 車番チケットレス(割引済み対応)
																	/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用料金計算		*/
	if( CPrmSS[S_PRP][1] != 2 ){									/* 回数券使用する設定じゃなかったら	*/
		ppc_ryo += PayData.ppc_chusi_ryo;							/* 前回ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用料金加算	*/
	}																/*								*/
	dat2 = 0L;														/*								*/
																	/* 種別毎集計の精算料金計算		*/
	switch( prm_get(COM_PRM, S_TOT, 12, 1, 1) ){					/* 種別毎集計方法？				*/
																	/*								*/
		case	0:													/* 駐車料金						*/
			dat2 = PayData.WPrice;									/*								*/
			break;													/*								*/
																	/*								*/
		case	1:													/* 割引後料金（PPC精算含む）	*/
			if( PayData.WPrice >= (wari_ryo + ryo_buf.credit.pay_ryo) )
																	/* 駐車料金－割引料金			*/
				dat2 = (PayData.WPrice) - wari_ryo - ryo_buf.credit.pay_ryo;
			break;													/*								*/
																	/*								*/
		case	2:													/* 割引後料金（PPC、電子ﾏﾈｰ精算含まない）*/
// MH321800(S) hosoda ICクレジット対応
//			if( PayData.WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo + PayData.Electron_data.Suica.pay_ryo) )
//				dat2 = (PayData.WPrice) - wari_ryo - ppc_ryo - ryo_buf.credit.pay_ryo - PayData.Electron_data.Suica.pay_ryo;
			if(PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) {		// 決済リーダのクレジット決済？
				if( PayData.WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo) )
					dat2 = (PayData.WPrice) - wari_ryo - ppc_ryo - ryo_buf.credit.pay_ryo;
			} else {
				if( PayData.WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo + PayData.Electron_data.Suica.pay_ryo) )
					dat2 = (PayData.WPrice) - wari_ryo - ppc_ryo - ryo_buf.credit.pay_ryo - PayData.Electron_data.Suica.pay_ryo;
			}
// MH321800(E) hosoda ICクレジット対応
			break;													/*								*/
																	/*								*/
		default:													/* その他（設定ｴﾗｰ）			*/
			dat2 = PayData.WPrice;									/*								*/
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}

	if( uc_update == 0 )											/* 定期券更新精算以外			*/
	{																/*								*/
// MH322914(S) K.Onodera 2016/10/11 AI-V対応：遠隔精算
		// 遠隔精算？(ParkingWeb版)
		if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_TIME ||
			OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE ){
			val = prm_get( COM_PRM, S_CEN, 40, 2, 1 );
			if( val >= 1 && val <= 12 ){
				tmp_syu = ryo_buf.syubet;	// 退避
				ryo_buf.syubet = (val - 1);
			}
			ts->Remote_seisan_cnt += 1L;
			ts->Remote_seisan_ryo += ryo_buf.tyu_ryo;
		}
// MH322914(E) K.Onodera 2016/10/11 AI-V対応：遠隔精算
		if(( CPrmSS[S_CAL][19] != 0 )&&								/* 消費税有り?					*/
		   ( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2 ))				/* 外税?						*/
		{															/*								*/
			if( dat2 >= ryo_buf.tax )								/* 税金対象額＞＝税金?			*/
				ts->Rsei_ryo[ryo_buf.syubet] += (dat2 - ryo_buf.tax);/* 種別毎精算料金				*/
			else													/*								*/
				ts->Rsei_ryo[ryo_buf.syubet] += dat2;				/* 種別毎精算料金				*/
																	/*								*/
			if( ryo_buf.pkiti != 0xffff ){							/* ﾏﾙﾁ精算以外					*/
				if( dat1 >= ryo_buf.tax )							/* 税金対象額＞＝税金?			*/
					lkts->Genuri_ryo += (dat1 - ryo_buf.tax);		/* 駐車位置番号別集計(現金売り上げ)	*/
				else												/*								*/
					lkts->Genuri_ryo += dat1;						/* 駐車位置番号別集計(現金売り上げ)	*/
			}
		} else {													/*								*/
			ts->Rsei_ryo[ryo_buf.syubet] += dat2;					/* 種別毎精算料金				*/
			if( ryo_buf.pkiti != 0xffff )							/* ﾏﾙﾁ精算以外					*/
				lkts->Genuri_ryo += dat1;							/* 駐車位置番号別集計(現金売り上げ)	*/
		}															/*								*/
// MH322914(S) K.Onodera 2017/01/05 AI-V対応
		// 振替元から振替額を引く + 振替先に加算
		for( i = 0; i < DETAIL_SYU_MAX; i++ )
		{
			switch( PayData.DetailData[i].DiscSyu ){
				case NTNET_FURIKAE_2:													// 振替精算（ParkingWeb版）
					pos = PayData.DetailData[i].uDetail.Furikae.Pos;
					break;
				case NTNET_FURIKAE_DETAIL:
					ulFurikae = PayData.DetailData[i].uDetail.FurikaeDetail.FrkMoney;	// 割引料金加算
					loktl.tloktl.loktldat[pos-1].Genuri_ryo -= ulFurikae;
					lkts->Genuri_ryo += ulFurikae;
					break;
			}
		}
// MH322914(E) K.Onodera 2017/01/05 AI-V対応
																	/*								*/
		if( ryo_buf.pkiti != 0xffff )								/* ﾏﾙﾁ精算以外					*/
			lkts->Seisan_cnt += 1;									/* 駐車位置番号別集計(精算台数)	*/
																	/*								*/
		ts->Rsei_cnt[ryo_buf.syubet] += 1L;							/* 種別毎精算回数				*/
// MH322914(S) K.Onodera 2016/10/11 AI-V対応：遠隔精算
		if( tmp_syu ){
			// 退避したデータを元に戻す
			ryo_buf.syubet = tmp_syu;
		}
// MH322914(E) K.Onodera 2016/10/11 AI-V対応：遠隔精算
	}
// 不具合修正(S) K.Onodera 2016/11/28 #1578 集計が実際の金額と一致しない
	// 振替額考慮(振替額を振替元から引く)
	val = (ushort)prm_get(COM_PRM, S_TOT, 12, 1, 1);
	if( val == 1 || val == 2 ){
		for( i = 0; i < DETAIL_SYU_MAX; i++ )
		{
			switch( PayData.DetailData[i].DiscSyu ){
				case NTNET_FURIKAE_2:											// 振替精算（ParkingWeb版）
					tmp_syu = (PayData.DetailData[i].uDetail.Furikae.Oiban / 100000L);
					break;
				case NTNET_FURIKAE_DETAIL:
					// 現金は振替元から引く ------------------
					ulFurikae = PayData.DetailData[i].uDetail.FurikaeDetail.FrkMoney;	// 割引料金加算
					ts->Rsei_ryo[tmp_syu-1] -= ulFurikae;		// 振替元から振替額を引く

					// その他は振替先から引く ------------------
					//  ※振替元には元々加算されない
					// 割引
					ulFuriWari = PayData.DetailData[i].uDetail.FurikaeDetail.FrkDiscount;
					// クレジット
					if( PayData.DetailData[i].uDetail.FurikaeDetail.Mod == 1 ){
						ulFuriWari += PayData.DetailData[i].uDetail.FurikaeDetail.FrkCard;
					}
					// 電子マネー
// MH321800(S) Y.Tanizaki ICクレジット対応
//					else if( PayData.DetailData[i].uDetail.FurikaeDetail.Mod == 2 ){
					else if( PayData.DetailData[i].uDetail.FurikaeDetail.Mod != 0 ){
// MH321800(E) Y.Tanizaki ICクレジット対応
						if( val == 2 ){
							ulFuriWari += PayData.DetailData[i].uDetail.FurikaeDetail.FrkCard;
						}
					}
					if( PayData.WPrice < ulFuriWari ){
						ulFuriWari = PayData.WPrice;
					}
					ts->Rsei_ryo[ryo_buf.syubet] -= ulFuriWari;
					break;
			}
		}
	}
// 不具合修正(E) K.Onodera 2016/11/28 #1578 集計が実際の金額と一致しない

	ts->Seisan_Tcnt += 1L;											/* 総精算回数					*/

// GG129000(S) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]
// 	/* 種別割引料金 */
// 	if( PayData.SyuWariRyo ){										/* 種別割引あり					*/
// 		ts->Rtwari_ryo[ryo_buf.syubet] += PayData.SyuWariRyo;		/* 種別割引合計額加算			*/
// 		ts->Rtwari_cnt[ryo_buf.syubet] += 1;						/* 種別割引回数加算				*/
// 	}
// GG129000(E) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]
																	/*								*/
// 仕様変更(S) K.Onodera 2016/11/07 振替精算
//	ts->Turi_modosi_ryo += ryo_buf.turisen;							/* 釣銭払戻額					*/
	ts->Turi_modosi_ryo += ryo_buf.turisen + ryo_buf.kabarai;		/* 釣銭払戻額					*/
// 仕様変更(E) K.Onodera 2016/11/07 振替精算
																	/*								*/
	if( ryo_buf.fusoku != 0 )										/* 支払い不足額有り？			*/
	{																/*								*/
		ts->Harai_husoku_cnt += 1L;									/* 支払い不足回数				*/
		ts->Harai_husoku_ryo += ryo_buf.fusoku;						/* 支払い不足額					*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.svs_tim )											/* ｻｰﾋﾞｽﾀｲﾑ内出庫有り			*/
	{																/*								*/
		ts->In_svst_seisan += 1L;									/* ｻｰﾋﾞｽﾀｲﾑ内精算回数+1			*/
		syusei[iti].infofg |= SSS_SVTIME;							/* 修正元ｻｰﾋﾞｽﾀｲﾑ内出庫情報登録	*/
	}																/*								*/

																	/*								*/
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//	if( PayData.credit.pay_ryo )									/* ｸﾚｼﾞｯﾄｶｰﾄﾞ使用				*/
	if( PayData.credit.pay_ryo &&									/* ｸﾚｼﾞｯﾄｶｰﾄﾞ使用				*/
		PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 0)	/* みなし決済ではない			*/
// MH810105 GG119202(E) 処理未了取引集計仕様改善
	{																/*								*/
		if( PayData.credit.cre_type == CREDIT_CARD ){
			ts->Ccrd_sei_cnt += 1;									/* ｸﾚｼﾞｯﾄｶｰﾄﾞ精算＜回数＞		*/
			ts->Ccrd_sei_ryo += PayData.credit.pay_ryo;				/* ｸﾚｼﾞｯﾄｶｰﾄﾞ精算＜金額＞		*/
																	/*								*/
			dat1 = prm_get( COM_PRM,S_TOT,6,1,1 );					/* ｸﾚｼﾞｯﾄｶｰﾄﾞ総売上額/総掛売額加算設定	*/
		}
		if( dat1 == 1L )											/* 総売上額に加算する			*/
		{															/*								*/
			ts->Uri_Tryo += PayData.credit.pay_ryo;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
			if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
				if(date_uriage_cmp_paymentdate() == 0){// 加算可能
					date_ts->Uri_Tryo += PayData.credit.pay_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
		}															/*								*/
		else if( dat1 == 2L )										/* 総掛売額に加算する			*/
		{															/*								*/
			ts->Kakeuri_Tryo += PayData.credit.pay_ryo;				/* 総掛売額						*/
		}															/*								*/
		else if( dat1 == 3L )										/* 総売上額/総掛売額に加算する	*/
		{															/*								*/
			ts->Uri_Tryo += PayData.credit.pay_ryo;					/* 総売上額						*/
			ts->Kakeuri_Tryo += PayData.credit.pay_ryo;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
			if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
				if(date_uriage_cmp_paymentdate() == 0){// 加算可能
					date_ts->Uri_Tryo += PayData.credit.pay_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
		}															/*								*/
	}																/*								*/
	wk_Electron_ryo = 0L;

	wk_media_Type = Ope_Disp_Media_Getsub(1);						/* 電子ﾏﾈｰ媒体種別 取得			*/
// MH321800(S) hosoda ICクレジット対応
	if (EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) != 0 ||
		PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED ) {
		// 決済リーダが使用されていたら
		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );							/* 電子マネー総売上額/総掛売額加算設定	*/
		wk_Electron_ryo = PayData.Electron_data.Ec.pay_ryo;					/* 電子マネー精算金額セーブ				*/
		ec_kind = PayData.Electron_data.Ec.e_pay_kind;
// MH810105 GG119202(S) 処理未了取引集計仕様改善
		if (ec_kind == EC_CREDIT_USED) {
			wk_Electron_ryo = PayData.credit.pay_ryo;
// MH810105(S) MH364301 QRコード決済対応
			dat1 = 0;
// MH810105(E) MH364301 QRコード決済対応
		}
		if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			// みなし決済であればみなし決済の精算回数、金額に加算する
			ts->Ec_minashi_cnt += 1L;										// みなし決済回数
			ts->Ec_minashi_ryo += wk_Electron_ryo;							// みなし決済金額
		}
		else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			// 未了残高照会完了であれば処理未了取引の支払済み回数、金額に加算する
			ts->miryo_pay_ok_cnt += 1L;										// 支払済み回数
			ts->miryo_pay_ok_ryo += wk_Electron_ryo;						// 支払済み金額
		}
		else {
// MH810105 GG119202(E) 処理未了取引集計仕様改善
		// 集計は既存エリアがあればそれを使用する。
		switch(ec_kind) {
		case EC_EDY_USED:
			ts->Electron_edy_cnt += 1L;										// 精算＜件数＞
			ts->Electron_edy_ryo += wk_Electron_ryo;						//     ＜金額＞
			break;
		case EC_NANACO_USED:
			ts->nanaco_sei_cnt += 1L;										// 精算＜件数＞
			ts->nanaco_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
			break;
		case EC_WAON_USED:
			ts->waon_sei_cnt += 1L;											// 精算＜件数＞
			ts->waon_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
			break;
		case EC_SAPICA_USED:
			ts->sapica_sei_cnt += 1L;										// 精算＜件数＞
			ts->sapica_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
			break;
		case EC_KOUTSUU_USED:
			ts->koutsuu_sei_cnt += 1L;										// 精算＜件数＞
			ts->koutsuu_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
			break;
		case EC_ID_USED:
			ts->id_sei_cnt += 1L;											// 精算＜件数＞
			ts->id_sei_ryo += wk_Electron_ryo;								//     ＜金額＞
			break;
		case EC_QUIC_PAY_USED:
			ts->quicpay_sei_cnt += 1L;										// 精算＜件数＞
			ts->quicpay_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
			break;
// MH810105(S) MH364301 PiTaPa対応
		case EC_PITAPA_USED:
			ts->pitapa_sei_cnt += 1L;										// 精算＜件数＞
			ts->pitapa_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
			break;
// MH810105(E) MH364301 PiTaPa対応
		case EC_CREDIT_USED:
			// クレジットのT集計は既存処理にて処理
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//			wk_Electron_ryo = PayData.credit.pay_ryo;
// MH810105 GG119202(E) 処理未了取引集計仕様改善
			break;
// MH810105(S) MH364301 QRコード決済対応
		case EC_QR_USED:
			ts->qr_sei_cnt += 1L;											// 精算＜件数＞
			ts->qr_sei_ryo += wk_Electron_ryo;								//     ＜金額＞
			break;
// MH810105(E) MH364301 QRコード決済対応
		default:
			break;		// ありえない
		}
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//		if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//			// みなし決済であればみなし決済の精算回数、金額に加算する
//			ts->Ec_minashi_cnt += 1L;										// みなし決済精算＜回数＞
//			ts->Ec_minashi_ryo += wk_Electron_ryo;							// 　　　　 ＜金額＞
//		}
//// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
		}
// MH810105 GG119202(E) 処理未了取引集計仕様改善
		// 最新の通知日時から集計日時を算出
		memcpy(&wrk_date, &Syuukei_sp.ec_inf.NowTime, sizeof(date_time_rec));
		wrk_date.Hour = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 3);			/* 時は共通パラメータから取得 */
		wrk_date.Min = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 1);				/* 分は共通パラメータから取得 */
		sy1_date = Nrm_YMDHM(&wrk_date);									/* 集計日時をノーマライズ */

		// 精算データから（決済結果データで受信した）精算日時を算出
		c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &wrk_clk_rec);
		memcpy(&wrk_date, &wrk_clk_rec, sizeof(date_time_rec));
		sei_date = Nrm_YMDHM(&wrk_date);									/* 精算日時をノーマライズ */

		ec_kind -= EC_EDY_USED;
// MH810105(S) MH364301 QRコード決済対応
		if (ec_kind >= EC_BRAND_TOTAL_MAX) {
			// 範囲外は日毎集計を行わない
		}
		else
// MH810105(E) MH364301 QRコード決済対応
		if(sei_date >= sy1_date && sei_date < (sy1_date + 0x10000)) {
			// 精算日時が当日の集計日時範囲
// MH810105 GG119202(S) 処理未了取引集計仕様改善
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// みなし決済であればみなし決済の精算回数、金額に加算する
				Syuukei_sp.ec_inf.now.sp_minashi_cnt += 1L;						// みなし決済回数
				Syuukei_sp.ec_inf.now.sp_minashi_ryo += wk_Electron_ryo;		// みなし決済金額
			}
			else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
				// 未了残高照会完了であれば処理未了取引の支払済み回数、金額に加算する
				Syuukei_sp.ec_inf.now.sp_miryo_pay_ok_cnt += 1L;				// 支払済み回数
				Syuukei_sp.ec_inf.now.sp_miryo_pay_ok_ryo += wk_Electron_ryo;	// 支払済み金額
			}
			else {
// MH810105 GG119202(E) 処理未了取引集計仕様改善
			Syuukei_sp.ec_inf.now.cnt[ec_kind] += 1L;							/* 使用回数				*/
			Syuukei_sp.ec_inf.now.ryo[ec_kind] += wk_Electron_ryo;				/* 使用金額				*/
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//			// みなし決済であれば日毎集計格納データの一番後ろに加算する
//			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//				Syuukei_sp.ec_inf.now.cnt[EC_BRAND_TOTAL_MAX - 1] += 1L;		/* 使用回数				*/
//				Syuukei_sp.ec_inf.now.ryo[EC_BRAND_TOTAL_MAX - 1] += wk_Electron_ryo; /* 使用金額				*/
//			}
//// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
			}
// MH810105 GG119202(E) 処理未了取引集計仕様改善
		}
		else if(sei_date >= (sy1_date + 0x10000) && sei_date < (sy1_date + 0x20000)) {
			// 精算日時が翌日の集計日時範囲
// MH810105 GG119202(S) 処理未了取引集計仕様改善
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// みなし決済であればみなし決済の精算回数、金額に加算する
				Syuukei_sp.ec_inf.next.sp_minashi_cnt += 1L;					// みなし決済回数
				Syuukei_sp.ec_inf.next.sp_minashi_ryo += wk_Electron_ryo;		// みなし決済金額
			}
			else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
				// 未了残高照会完了であれば処理未了取引の支払済み回数、金額に加算する
				Syuukei_sp.ec_inf.next.sp_miryo_pay_ok_cnt += 1L;				// 支払済み回数
				Syuukei_sp.ec_inf.next.sp_miryo_pay_ok_ryo += wk_Electron_ryo;	// 支払済み金額
			}
			else {
// MH810105 GG119202(E) 処理未了取引集計仕様改善
			Syuukei_sp.ec_inf.next.cnt[ec_kind] += 1L;							/* 使用回数				*/
			Syuukei_sp.ec_inf.next.ryo[ec_kind] += wk_Electron_ryo;				/* 使用金額				*/
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//			// みなし決済であれば日毎集計格納データの一番後ろに加算する
//			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//				Syuukei_sp.ec_inf.next.cnt[EC_BRAND_TOTAL_MAX - 1] += 1L;		/* 使用回数				*/
//				Syuukei_sp.ec_inf.next.ryo[EC_BRAND_TOTAL_MAX - 1] += wk_Electron_ryo; /* 使用金額				*/
//			}
//// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
			}
// MH810105 GG119202(E) 処理未了取引集計仕様改善
		}
		else if(sei_date >= (sy1_date - 0x10000) && sei_date < sy1_date) {
			// 精算日時が前日の集計日時範囲
			if(Syuukei_sp.ec_inf.cnt == 1) {
				// 前日の集計がないため作成する
				UnNrm_YMDHM(&wrk_date, (sy1_date - 0x10000));
				memcpy(&Syuukei_sp.ec_inf.bun[Syuukei_sp.ec_inf.ptr].SyuTime, &wrk_date, sizeof(date_time_rec));
				Syuukei_sp.ec_inf.ptr++;
				Syuukei_sp.ec_inf.cnt++;
			}
			syu_idx = (Syuukei_sp.ec_inf.ptr == 0) ? (SYUUKEI_DAY_EC-1) : (Syuukei_sp.ec_inf.ptr-1);
// MH810105 GG119202(S) 処理未了取引集計仕様改善
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// みなし決済であればみなし決済の精算回数、金額に加算する
				Syuukei_sp.ec_inf.bun[syu_idx].sp_minashi_cnt += 1L;					// みなし決済回数
				Syuukei_sp.ec_inf.bun[syu_idx].sp_minashi_ryo += wk_Electron_ryo;		// みなし決済金額
			}
			else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
				// 未了残高照会完了であれば処理未了取引の支払済み回数、金額に加算する
				Syuukei_sp.ec_inf.bun[syu_idx].sp_miryo_pay_ok_cnt += 1L;				// 支払済み回数
				Syuukei_sp.ec_inf.bun[syu_idx].sp_miryo_pay_ok_ryo += wk_Electron_ryo;	// 支払済み金額
			}
			else {
// MH810105 GG119202(E) 処理未了取引集計仕様改善
			Syuukei_sp.ec_inf.bun[syu_idx].cnt[ec_kind] += 1L;					/* 使用回数				*/
			Syuukei_sp.ec_inf.bun[syu_idx].ryo[ec_kind] += wk_Electron_ryo;		/* 使用金額				*/
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//			// みなし決済であれば日毎集計格納データの一番後ろに加算する
//			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//				Syuukei_sp.ec_inf.bun[syu_idx].cnt[EC_BRAND_TOTAL_MAX - 1] += 1L;	/* 使用回数				*/
//				Syuukei_sp.ec_inf.bun[syu_idx].ryo[EC_BRAND_TOTAL_MAX - 1] += wk_Electron_ryo; /* 使用金額				*/
//			}
//// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
			}
// MH810105 GG119202(E) 処理未了取引集計仕様改善
		} else {
			// 精算日時が翌日から前日の範囲外の場合はアラームを登録して当日に加算
// MH810105 GG119202(S) 処理未了取引集計仕様改善
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// みなし決済であればみなし決済の精算回数、金額に加算する
				Syuukei_sp.ec_inf.now.sp_minashi_cnt += 1L;						// みなし決済回数
				Syuukei_sp.ec_inf.now.sp_minashi_ryo += wk_Electron_ryo;		// みなし決済金額
			}
			else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
				// 未了残高照会完了であれば処理未了取引の支払済み回数、金額に加算する
				Syuukei_sp.ec_inf.now.sp_miryo_pay_ok_cnt += 1L;				// 支払済み回数
				Syuukei_sp.ec_inf.now.sp_miryo_pay_ok_ryo += wk_Electron_ryo;	// 支払済み金額
			}
			else {
// MH810105 GG119202(E) 処理未了取引集計仕様改善
			Syuukei_sp.ec_inf.now.cnt[ec_kind] += 1L;							/* 使用回数				*/
			Syuukei_sp.ec_inf.now.ryo[ec_kind] += wk_Electron_ryo;				/* 使用金額				*/
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//			// みなし決済であれば日毎集計格納データの一番後ろに加算する
//			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//				Syuukei_sp.ec_inf.now.cnt[EC_BRAND_TOTAL_MAX - 1] += 1L;		/* 使用回数				*/
//				Syuukei_sp.ec_inf.now.ryo[EC_BRAND_TOTAL_MAX - 1] += wk_Electron_ryo; /* 使用金額				*/
//			}
//// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
			}
// MH810105 GG119202(E) 処理未了取引集計仕様改善
			Syuukei_sp.ec_inf.now.unknownTimeCnt++;

			// アラーム登録(決済リーダの決済時刻と精算機の時刻に1日以上のずれがある)
			alm_chk( ALMMDL_SUB, ALARM_EC_SETTLEMENT_TIME_GAP, 2 );
		}
	}
	else
// MH321800(E) hosoda ICクレジット対応
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ){

		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );					/* Suica総売上額/総掛売額加算設定	*/
		wk_Electron_ryo = PayData.Electron_data.Suica.pay_ryo;		/* 電子ﾏﾈｰ精算金額セーブ		*/
		Syuukei_sp.sca_inf.now.cnt += 1L;							/* Suica使用回数				*/
		Syuukei_sp.sca_inf.now.ryo += wk_Electron_ryo;				/* Suica使用金額				*/
		if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {			/* Ｓｕｉｃａ					*/
			ts->Electron_sei_cnt += 1L;								/* Suica使用＜回数＞			*/
			ts->Electron_sei_ryo += wk_Electron_ryo;				/* Suica使用＜金額＞			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		/* ＰＡＳＭＯ					*/
			ts->Electron_psm_cnt += 1L;								/* PASMO使用＜回数＞			*/
			ts->Electron_psm_ryo += wk_Electron_ryo;				/* PASMO使用＜金額＞			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICOCA) {		/* ＩＣＯＣＡ					*/
			ts->Electron_ico_cnt += 1L;								/* ICOCA使用＜回数＞			*/
			ts->Electron_ico_ryo += wk_Electron_ryo;				/* ICOCA使用＜金額＞			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICCARD) {	/* ＩＣ-ＣＡＲＤ				*/
			ts->Electron_icd_cnt += 1L;								/* IC-CARD使用＜回数＞			*/
			ts->Electron_icd_ryo += wk_Electron_ryo;				/* IC-CARD使用＜金額＞			*/
		}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//	} else if( PayData.Electron_data.Suica.e_pay_kind == EDY_USED ){/* Edyﾘﾀﾞｰ領収ﾃﾞｰﾀあり			*/
//		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 3 );					/* Edy総売上額/総掛売額加算設定	*/
//		if( EDY_TEST_MODE ){										// ﾃｽﾄﾓｰﾄﾞの場合
//			dat1 = 0L;												// 総売上額/総掛売額に加算しない
//		}
//		else{
//			wk_Electron_ryo = PayData.Electron_data.Edy.pay_ryo;	/* 電子ﾏﾈｰ精算金額セーブ		*/
//			if(wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY) {			/* Ｅｄｙ						*/
//				ts->Electron_edy_cnt += 1L;							/* Ｅｄｙ使用＜回数＞			*/
//				ts->Electron_edy_ryo += wk_Electron_ryo;			/* Ｅｄｙ使用＜金額＞			*/
//				Syuukei_sp.edy_inf.now.cnt += 1L;					/* PASMO使用回数				*/
//				Syuukei_sp.edy_inf.now.ryo += wk_Electron_ryo;		/* PASMO使用金額				*/
//			}
//		}
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	}
// MH321800(S) Y.Tanizaki ICクレジット対応
//	if( PayData.Electron_data.Suica.e_pay_kind != 0 ) {				/* 電子ﾏﾈｰ(Suica/Edy)領収ﾃﾞｰﾀあり*/
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ||	// SX-20による電子マネー決済
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//		EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {			// 決済リーダによる電子マネー決済
		(EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) &&			// 決済リーダによる電子マネー決済
		 PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 0)) {	// みなし決済ではない
// MH810105 GG119202(E) 処理未了取引集計仕様改善
// MH321800(E) Y.Tanizaki ICクレジット対応
		if( dat1 == 1L ) {											/* 総売上額に加算する			*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
			if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
				if(date_uriage_cmp_paymentdate() == 0){// 加算可能
					date_ts->Uri_Tryo += wk_Electron_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
		} else if( dat1 == 2L ) {									/* 総掛売額に加算する			*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* 総掛売額						*/
		} else if( dat1 == 3L ) {									/* 総売上額/総掛売額に加算する	*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* 総売上額						*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
			if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
				if(date_uriage_cmp_paymentdate() == 0){// 加算可能
					date_ts->Uri_Tryo += wk_Electron_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
		}															/*								*/
	}
// MH810105(S) MH364301 QRコード決済対応（みなし決済額を総売上／総掛売へ加算する設定）
	else{
		if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
			dat1 = prm_get( COM_PRM, S_TOT, 7, 1, 1 );				// みなし決済総売上額/総掛売額加算設定
			if( dat1 == 1L )										/* 加算する場合各ブランドの加算設定を参照 */
			{														/*								*/
				if( PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED ){
					dat2 = prm_get( COM_PRM,S_TOT,6,1,1 );			/* ｸﾚｼﾞｯﾄｶｰﾄﾞ総売上額/総掛売額加算設定	*/
					if( dat2 == 1L )								/* 総売上額に加算する			*/
					{												/*								*/
						ts->Uri_Tryo += PayData.credit.pay_ryo;		/* 総売上額						*/
						if(date_uriage_use_chk() == 0){				// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
							if(date_uriage_cmp_paymentdate() == 0){	// 加算可能
								date_ts->Uri_Tryo += PayData.credit.pay_ryo;
							}
						}
					}												/*								*/
					else if( dat2 == 2L )							/* 総掛売額に加算する			*/
					{												/*								*/
						ts->Kakeuri_Tryo += PayData.credit.pay_ryo;	/* 総掛売額						*/
					}												/*								*/
					else if( dat2 == 3L )							/* 総売上額/総掛売額に加算する	*/
					{												/*								*/
						ts->Uri_Tryo += PayData.credit.pay_ryo;		/* 総売上額						*/
						ts->Kakeuri_Tryo += PayData.credit.pay_ryo;	/* 総掛売額						*/
						if(date_uriage_use_chk() == 0){				// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
							if(date_uriage_cmp_paymentdate() == 0){	// 加算可能
								date_ts->Uri_Tryo += PayData.credit.pay_ryo;
							}
						}
					}												/*								*/
				}
				else if( EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ){
					dat2 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );		/* 電子マネー総売上額/総掛売額加算設定	*/
					if( dat2 == 1L )								/* 総売上額に加算する			*/
					{												/*								*/
						ts->Uri_Tryo += wk_Electron_ryo;			/* 総売上額						*/
						if(date_uriage_use_chk() == 0){				// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
							if(date_uriage_cmp_paymentdate() == 0){	// 加算可能
								date_ts->Uri_Tryo += wk_Electron_ryo;
							}
						}
					}
					else if( dat2 == 2L )							/* 総掛売額に加算する			*/
					{												/*								*/
						ts->Kakeuri_Tryo += wk_Electron_ryo;		/* 総掛売額						*/
					}
					else if( dat2 == 3L )							/* 総売上額/総掛売額に加算する	*/
					{												/*								*/
						ts->Uri_Tryo += wk_Electron_ryo;			/* 総売上額						*/
						ts->Kakeuri_Tryo += wk_Electron_ryo;		/* 総掛売額						*/
						if(date_uriage_use_chk() == 0){				// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
							if(date_uriage_cmp_paymentdate() == 0){	// 加算可能
								date_ts->Uri_Tryo += wk_Electron_ryo;
							}
						}
					}
				}
			}
		}															/*								*/
	}																/*								*/														/*								*/
// MH810105(E) MH364301 QRコード決済対応（みなし決済額を総売上／総掛売へ加算する設定）
																	/*								*/
	if( CPrmSS[S_PRP][1] == 1 )										/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞを使用する設定	*/
	{																/*								*/
		dat1 = prm_get( COM_PRM,S_TOT,5,1,3 );						/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ総売上額/総掛売額加算設定	*/
		j = 0;														/*								*/
		for( i = 0; i < WTIK_USEMAX; i++ ){							/*								*/
			disc_wari_conv( &PayData.DiscountData[i], &wari_dt );	/*								*/
			if( wari_dt.tik_syu == PREPAID ){						/* ﾃﾞｰﾀあり						*/
				dat2 = wari_dt.pkno_syu;							/*								*/
				ts->Pcrd_use_cnt[dat2] += ryo_buf.pri_mai[j];		/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜回数＞(基本,拡張1,2,3)	*/
																	/* 7枚以上使用時は最後に使われた種別に合計加算する	*/
				ts->Pcrd_use_ryo[dat2] += wari_dt.ryokin;			/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜金額＞(基本,拡張1,2,3)	*/
																	/*								*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				j++;
			}														/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ )								/*								*/
	{																/*								*/
		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );		/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
		taxableAdd_set = 0xff;
		if( cancelReceipt_Waridata_chk( &wari_dt ) ){				/*			課税対象			*/
			taxableAdd_set = (uchar)prm_get( COM_PRM, S_TOT, 7, 1, 2 );	/*課税対象利用額の売上加算設定*/
		}															/*								*/
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
		switch( wari_dt.tik_syu )									/*								*/
		{															/*								*/
			case SERVICE:											/* ｻｰﾋﾞｽ券						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* ｻｰﾋﾞｽ券A,B,C					*/
																	/*								*/
				ts->Stik_use_cnt[dat1][dat2] += wari_dt.maisuu;		/* ｻｰﾋﾞｽ券種別毎使用＜枚数＞(基本,拡張1,2,3)(ｻｰﾋﾞｽ券A,B,C)	*/
				ts->Stik_use_ryo[dat1][dat2] += wari_dt.ryokin;		/* ｻｰﾋﾞｽ券種別毎使用＜金額＞(基本,拡張1,2,3)(ｻｰﾋﾞｽ券A,B,C)	*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//				if( CPrmSS[S_SER][1+3*dat2] == 4 &&					/* ｻｰﾋﾞｽ券全額割引設定?			*/
				if( prm_get( COM_PRM, S_SER, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* ｻｰﾋﾞｽ券全額割引設定? */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					ryo_buf.pkiti != 0xffff &&						/* ﾏﾙﾁ精算以外?					*/
					uc_update == 0 )								/* 定期券更新精算以外			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* 修正用全額割引ﾌﾗｸﾞｾｯﾄ		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,1 );				/* ｻｰﾋﾞｽ券総売上額/総掛売額加算設定	*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				setting_taxableAdd( &dat1, taxableAdd_set );		/* 課税対象利用額の売上加算設定 */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				break;												/*								*/
																	/*								*/
			case KAKEURI:											/* 掛売券						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* 店№1～999(0～998)			*/
																	/*								*/

				ts->Mno_use_Tcnt += wari_dt.maisuu;					/* 店割引総ﾄｰﾀﾙ＜回数＞			*/
				ts->Mno_use_Tryo += wari_dt.ryokin;					/* 店割引総ﾄｰﾀﾙ＜金額＞			*/

// MH322914(S) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
//				ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;			/* 店№割引ﾄｰﾀﾙ＜回数＞(基本,拡張1,2,3)	*/
//				ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;			/* 店№割引ﾄｰﾀﾙ＜金額＞(基本,拡張1,2,3)	*/
				// ParkingWeb経由のParkiPro対応あり？ 且つ、ParkingWebからの店割引№設定と一致？
				if( _is_ParkingWeb_pip() && ((prm_get( COM_PRM, S_CEN, 45, 4, 1 )) == (dat2+1)) ){
					ts->Mno_use_cnt5 += wari_dt.maisuu;
					ts->Mno_use_ryo5 += wari_dt.ryokin;
				}
				else{
					ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;		/* 店№割引ﾄｰﾀﾙ＜回数＞(基本,拡張1,2,3)	*/
					ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;		/* 店№割引ﾄｰﾀﾙ＜金額＞(基本,拡張1,2,3)	*/
// MH322914(E) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引

				if( dat1 == KIHON_PKNO || dat1 == KAKUCHOU_1 )		/* 基本 or 拡張1				*/
				{													/*								*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
//					if( dat2 < 100 )								/* 店№1～100					*/
					if( dat2 < MISE_NO_CNT )						/* 店№1～100					*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* 店№1～100ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* 店№1～100ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
						ts->Mno_use_cnt4[dat1][dat2] +=				/* 店№毎割引＜回数＞(基本,拡張1)(店№1～100)	*/
												wari_dt.maisuu;		/*								*/
						ts->Mno_use_ryo4[dat1][dat2] +=				/* 店№毎割引＜金額＞(基本,拡張1)(店№1～100)	*/
												wari_dt.ryokin;		/*								*/
					}												/*								*/
					else											/* 店№101～999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* 店№101～999ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* 店№101～999ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
					}												/*								*/
				}													/*								*/
				else												/*								*/
				{													/*								*/
					if(( dat1 == KAKUCHOU_2 )&&						/* 拡張2						*/
					   ( prm_get( COM_PRM,S_TOT,16,1,3 ) == 1 ))	/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
					else if(( dat1 == KAKUCHOU_3 )&&				/* 拡張3						*/
							( prm_get( COM_PRM,S_TOT,16,1,4 ) == 1 ))/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
//					if( dat2 < 100 )								/* 店№1～100					*/
					if( dat2 < MISE_NO_CNT )						/* 店№1～100					*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* 店№1～100ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* 店№1～100ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
						if( dat1 == KAKUCHOU_1 )					/*								*/
						{											/*								*/
							ts->Mno_use_cnt4[dat1][dat2] +=			/* 店№毎割引＜回数＞(基本,拡張1)(店№1～100)	*/
												wari_dt.maisuu;		/*								*/
							ts->Mno_use_ryo4[dat1][dat2] +=			/* 店№毎割引＜金額＞(基本,拡張1)(店№1～100)	*/
												wari_dt.ryokin;		/*								*/
						}											/*								*/
					}												/*								*/
					else											/* 店№101～999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* 店№101～999ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* 店№101～999ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
					}												/*								*/
				}													/*								*/
// MH322914(S) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
				}
// MH322914(E) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//				if( CPrmSS[S_STO][1+3*dat2] == 4 &&					/* 店割引全額割引設定?			*/
				if( prm_get( COM_PRM, S_STO, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* 店割引全額割引設定? */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					ryo_buf.pkiti != 0xffff &&						/* ﾏﾙﾁ精算以外?					*/
					uc_update == 0 )								/* 定期券更新精算以外			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* 修正用全額割引ﾌﾗｸﾞｾｯﾄ		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* 店割引総売上額/総掛売額加算設定	*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				setting_taxableAdd( &dat1, taxableAdd_set );		/* 課税対象利用額の売上加算設定 */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				break;												/*								*/
			case KAISUU:											/* 回数券						*/
				if( CPrmSS[S_PRP][1] != 2 )							/* 回数券を使用する設定ではない	*/
					break;

				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
																	/*								*/
				ts->Ktik_use_cnt[dat1] += wari_dt.maisuu;			/* 使用＜枚数＞(基本,拡張1,2,3) */
				ts->Ktik_use_ryo[dat1] += wari_dt.ryokin;			/* 使用＜金額＞(基本,拡張1,2,3)	*/

				dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );				/* 回数券総売上額/総掛売額加算設定	*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				break;												/*								*/
			case MISHUU:											/* 減額精算						*/
				ts->Gengaku_seisan_cnt += 1L;						// T集計 減額回数+1
				ts->Gengaku_seisan_ryo += wari_dt.ryokin;			// T集計 減額料金
				break;
			case FURIKAE:											/* 振替精算						*/
				ts->Furikae_seisan_cnt += 1L;						// T集計 振替回数+1
				ts->Furikae_seisan_ryo += wari_dt.ryokin;			// T集計 振替料金
				break;
			case SYUUSEI:											/* 修正精算						*/
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//				ts->Syuusei_seisan_Tcnt += 1L;						// T集計 修正精算回数+1
//				lkts->Syuusei_seisan_cnt += 1L;						// 駐車位置番号別集計(修正精算回数)
//				if( prm_get(COM_PRM, S_TOT, 20, 1, 1) == 1 ){		// 振替金額で集計する
//					ts->Syuusei_seisan_Tryo += wari_dt.ryokin;		// T集計 修正精算(振替額)
//					lkts->Syuusei_seisan_ryo += wari_dt.ryokin;		// 駐車位置番号別集計(修正精算額)
//				}else{												// 払戻額で修正する(TF4800互換)
//					if( !(PayData.Zengakufg&0x02) ){				// 未入金なし
//						ts->Syuusei_seisan_Tryo += PayData.MMTwari;	// T集計 修正精算(払戻額)
//						lkts->Syuusei_seisan_ryo += PayData.MMTwari;// 駐車位置番号別集計(修正精算額)
//					}
//				}
//				if( (PayData.Zengakufg&0x02) ){						// 未入金あり
//					ts->Syuusei_seisan_Mcnt += 1L;					// T集計 修正精算(未入金回数)
//					ts->Syuusei_seisan_Mryo += PayData.MMTwari;		// T集計 修正精算(未入金金額)
//				}
//
//#if 1 // AKIBA DEBUG TEST 08.06.27(S)
//				if( prm_get(COM_PRM, S_TOT, 20, 1, 6) == 1 ){				// 集計加算のやり直しする
//
//					// 総精算回数の減算
//					if( ts->Seisan_Tcnt > 0 ){
//						ts->Seisan_Tcnt -= 1L;							// 修正元 総精算回数減算
//					}
//					// ｻｰﾋﾞｽﾀｲﾑ内精算回数の減算
//					if( syusei[wdt1].infofg & SSS_SVTIME ){				// 修正元がｻｰﾋﾞｽﾀｲﾑ内精算の時
//						if( ts->In_svst_seisan > 0 ){
//							ts->In_svst_seisan -= 1L;					// 修正元 ｻｰﾋﾞｽﾀｲﾑ内精算回数-1
//						}
//					}
//					// ｱﾝﾁパスOFF精算回数の減算
//					if( syusei[wdt1].infofg & SSS_ANTIOFF ){			// 修正元がｱﾝﾁﾊﾟｽOFF精算の時
//						if( ts->Apass_off_seisan > 0 ){
//							ts->Apass_off_seisan -= 1L;					// 強制ｱﾝﾁﾊﾟｽOFF精算回数-1
//						}
//					}
//					// 領収証発行回数の減算
//					if( syusei[wdt1].infofg & SSS_RYOUSYU ){			// 修正元 領収証発行あり
//						if( ts->Ryosyuu_pri_cnt > 0 ){
//							ts->Ryosyuu_pri_cnt -= 1L;					// 領収証発行回数-1
//						}
//					}
//					// 定期種別毎精算回数の減算
//					if( syusei[wdt1].tei_syu ){							// 修正元で定期券使用の時
//						wdt2 = (syusei[wdt1].tei_syu & 0xc0) >> 6;		// 駐車場№
//						wdt3 = syusei[wdt1].tei_syu & 0x3f;				// 定期種別
//						if( ts->Teiki_use_cnt[wdt2][wdt3-1] > 0 ){
//							ts->Teiki_use_cnt[wdt2][wdt3-1] -= 1L;		// 修正元 定期使用回数-1
//						}
//					}
//
//					// 総売上額の減算(総売上額-修正元の払戻額)
//					if( !(PayData.Zengakufg&0x02) ){							// 未入金なし
//						if( ts->Uri_Tryo >= PayData.MMTwari ){
//							ts->Uri_Tryo	-= PayData.MMTwari;					/* 総売上額						*/
//// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
//							if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
//								if(date_uriage_cmp_paymentdate() == 0){// 加算可能
//									date_ts->Uri_Tryo -= PayData.MMTwari;
//								}
//							}
//// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
//						}
//						// 総現金売上額の減算(総現金売上額-修正元の払戻額)
//						if( ts->Genuri_Tryo >= PayData.MMTwari ){
//							ts->Genuri_Tryo	-= PayData.MMTwari;					/* 総現金売り上げ				*/
//// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
//							if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
//								if(date_uriage_cmp_paymentdate() == 0){// 加算可能
//									date_ts->Genuri_Tryo -= PayData.MMTwari;
//								}
//							}
//// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
//						}
//					}
//					// 駐車位置別精算回数の減算
//					if( loktl.tloktl.loktldat[wdt1].Seisan_cnt > 0 ){
//						loktl.tloktl.loktldat[wdt1].Seisan_cnt -= 1;	// 修正元 駐車位置別精算回数-1
//					}
//					// 駐車位置別現金領収額のやり直し
//					// ①修正元の車室から修正元での現金分を引く
//					// ②修正元の現金分を修正先へ加算する
//					if( loktl.tloktl.loktldat[wdt1].Genuri_ryo >= syusei[wdt1].gen ){
//						loktl.tloktl.loktldat[wdt1].Genuri_ryo -= syusei[wdt1].gen;
//					}														// 修正元 駐車位置別現金売上減算
//					if( !(PayData.Zengakufg&0x02) ){							// 未入金なし
//							lkts->Genuri_ryo += (syusei[wdt1].gen - PayData.MMTwari);
//					}else{
//							lkts->Genuri_ryo += (syusei[wdt1].gen);
//					}
//
//					// 料金種別毎精算回数の減算
//					if( ts->Rsei_cnt[wk_sss_syu-1] > 0 ){
//						ts->Rsei_cnt[wk_sss_syu-1] -= 1L;				// 修正元 種別毎精算回数減算
//					}
//					// 料金種別毎料金の減算
//					if( !(PayData.Zengakufg&0x02) ){							// 未入金なし
//						if( prm_get(COM_PRM, S_TOT, 12, 1, 1) != 0 ){			/* 種別毎集計方法？：割引後料金	*/
//							ts->Rsei_ryo[ryo_buf.syubet] += 
//									(syusei[wdt1].gen - PayData.MMTwari);		// 修正先に修正元の現金売上分を加算
//							if( ts->Rsei_ryo[wk_sss_syu-1] >= syusei[wdt1].gen ){
//								ts->Rsei_ryo[wk_sss_syu-1] -= syusei[wdt1].gen;	/* 修正元種別毎精算料金から引く	*/
//							}
//						}else{													/* 種別毎集計方法？：駐車料金	*/
//							if( ts->Rsei_ryo[wk_sss_syu-1] >= syusei[wdt1].ryo ){
//								ts->Rsei_ryo[wk_sss_syu-1] -= syusei[wdt1].ryo;
//							}
//						}
//					}else{
//						if( prm_get(COM_PRM, S_TOT, 12, 1, 1) != 0 ){			/* 種別毎集計方法？：割引後料金	*/
//							ts->Rsei_ryo[ryo_buf.syubet] += syusei[wdt1].gen;	// 修正先に修正元の現金売上分を加算
//							if( ts->Rsei_ryo[ryo_buf.syubet] >= PayData.MMTwari ){
//								ts->Rsei_ryo[ryo_buf.syubet] -= PayData.MMTwari;
//							}
//							if( ts->Rsei_ryo[wk_sss_syu-1] >= syusei[wdt1].gen ){
//								ts->Rsei_ryo[wk_sss_syu-1] -= syusei[wdt1].gen;	/* 修正元種別毎精算料金から引く	*/
//							}
//						}else{													/* 種別毎集計方法？：駐車料金	*/
//							if( ts->Rsei_ryo[wk_sss_syu-1] >= syusei[wdt1].ryo ){
//								ts->Rsei_ryo[wk_sss_syu-1] -= syusei[wdt1].ryo;
//							}
//						}
//					}
//					// 分類集計の減算
//					if( prm_get(COM_PRM, S_TOT, 12, 1, 2) != 0 ){			/* 種別毎集計方法？：割引後料金	*/
//						bunrui_syusei( (ushort)(wdt1+1), syusei[wdt1].gen );
//					}else{
//						bunrui_syusei( (ushort)(wdt1+1), syusei[wdt1].ryo );
//					}
//				}
//#endif	// AKIBA DEBUG TEST 08.06.27(S)
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
				break;
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
			case SHOPPING:											/* 買物割引						*/
// GG124100(S) R.Endo 2022/10/03 車番チケットレス3.0 #6635 割引余力のある使用済みの買い物割引を再精算で適用されたとき、集計に1枚使用したとして印字される [共通改善項目 No1534]
// 				ts->Kaimono_use_cnt[wari_dt.minute] += 1;			/* 買物割引①～⑩	＜回数＞	*/
				ts->Kaimono_use_cnt[wari_dt.minute] += wari_dt.maisuu;	/* 買物割引①～⑩	＜回数＞	*/
// GG124100(E) R.Endo 2022/10/03 車番チケットレス3.0 #6635 割引余力のある使用済みの買い物割引を再精算で適用されたとき、集計に1枚使用したとして印字される [共通改善項目 No1534]
				ts->Kaimono_use_ryo[wari_dt.minute] += wari_dt.ryokin;	/* 				 	＜金額＞	*/
				
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* 店割引総売上額/総掛売額加算設定	*/
//				dat1 = prm_get(COM_PRM, S_ONL, ONLINE_MEMBER_TOTAL_COUNT, 1, 3);
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				break;
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
// GG129000(S) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]
			case SYUBETU:											// 種別割引
				dat1 = (ulong)(wari_dt.syubetu - 1);
				ts->Rtwari_ryo[dat1] += wari_dt.ryokin;		// 種別割引合計額加算
				ts->Rtwari_cnt[dat1] += wari_dt.maisuu;		// 種別割引回数加算
				break;
// GG129000(E) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]
		}															/*								*/
	}																/*								*/
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
	for( i = 0; i < DETAIL_SYU_MAX; i++ )
	{
		disc_Detail_conv( &PayData.DetailData[i], &wari_dt );
		switch( wari_dt.tik_syu )
		{
			case FURIKAE2:											// 振替精算（ParkingWeb版）
				ts->Furikae_seisan_cnt2 += 1L;						// T集計 振替回数+1
				ts->Furikae_seisan_ryo2 += wari_dt.ryokin;			// T集計 振替料金
				// 現金以外の過払いあり？
				if( PayData.FRK_RetMod && PayData.FRK_Return ){
// MH321800(S) Y.Tanizaki ICクレジット対応
//					ts->Furikae_Card_cnt[PayData.FRK_RetMod]++;
//					ts->Furikae_CardKabarai[PayData.FRK_RetMod] += PayData.FRK_Return;
					if (PayData.FRK_RetMod == 1) {
						ts->Furikae_Card_cnt[1]++;
						ts->Furikae_CardKabarai[1] += PayData.FRK_Return;
					} else {
						ts->Furikae_Card_cnt[2]++;
						ts->Furikae_CardKabarai[2] += PayData.FRK_Return;
					}
// MH321800(E) Y.Tanizaki ICクレジット対応
				}
				break;
		}
	}
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応

	if(( ryo_buf.pkiti != 0xffff )&&											/* ﾏﾙﾁ精算以外					*/
	   ( uc_update == 0 )){														/* 定期券更新精算以外			*/

	if( syusei[iti].infofg & SSS_ZENWARI ){										// 全額割引あり
		syusei[iti].sy_wari = ryo_buf.waribik + c_pay;
	}else{																		// 全額割引なし
		if( ryo_buf.tim_amount != 0 ){											// 時間割引あり
			syusei[iti].sy_wari = ryo_buf.fee_amount 
								+ PayData.credit.pay_ryo + wk_Electron_ryo;		// ｸﾚｼﾞｯﾄ+Eﾏﾈｰ
			dat2 = 0L;
			if( ryo_buf.fee_amount >= c_pay ){
				dat2 = ryo_buf.fee_amount - c_pay;
			}
			if( ryo_buf.waribik >= dat2 ){
				syusei[iti].sy_time = ryo_buf.waribik - dat2;					//
			}
		}else{																	// 時間割引なし
			syusei[iti].sy_wari = ryo_buf.waribik + c_pay						// time_amountなしであればwaribikは全て料金割引
								+ PayData.credit.pay_ryo + wk_Electron_ryo;		// ｸﾚｼﾞｯﾄ+Eﾏﾈｰ
		}
	}
	}
																	/*								*/
	if( uc_update ){												/* 定期券更新精算				*/
		dat1 = (ulong)PayData.teiki.pkno_syu;						/* 駐車場№の種類				*/
		dat2 = (ulong)(PayData.teiki.syu - 1);						/* 定期券種別-1					*/
		ts->Teiki_kou_cnt[dat1][dat2] += 1L;						/* 定期券更新回数+1				*/
		ts->Teiki_kou_ryo[dat1][dat2] +=							/*								*/
// 仕様変更(S) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
//					(ulong)(ryo_buf.nyukin - ryo_buf.turisen);		/* 定期券更新金額加算			*/
					(ulong)(ryo_buf.nyukin - ryo_buf.turisen - ryo_buf.kabarai);		// 定期券更新金額加算
// 仕様変更(E) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
	}																/*								*/

	f_BunruiSyu = 0;												/* 分類集計するフラグクリア		*/

	if( ryo_buf.ryo_flg >= 2 )										/* 定期使用?					*/
	{																/*								*/
		dat1 = (ulong)PayData.teiki.pkno_syu;						/* 駐車場№の種類				*/
		ts->Teiki_use_cnt[dat1][PayData.teiki.syu-1] += 1L;			/* 定期使用回数+1				*/
		if(( ryo_buf.pkiti != 0xffff )&&							/* ﾏﾙﾁ精算以外					*/
		   ( uc_update == 0 )){										/* 定期券更新精算以外			*/

			syusei[iti].ryo = ryo_buf.tei_ryo;						/* 修正用駐車料金ｾｯﾄ			*/

			syusei[iti].tei_syu = PayData.teiki.syu;				/* 定期種別						*/
			syusei[iti].tei_syu |= (uchar)(PayData.teiki.pkno_syu<<6);
			syusei[iti].tei_id = (ushort)PayData.teiki.id;			/* 定期券id(1～12000)			*/
			syusei[iti].tei_sd = dnrmlzm( (short)PayData.teiki.s_year,/* 有効開始年月日ﾉｰﾏﾗｲｽﾞ		*/
									  (short)PayData.teiki.s_mon,	/*								*/
									  (short)PayData.teiki.s_day );	/*								*/
			syusei[iti].tei_ed = dnrmlzm( (short)PayData.teiki.e_year,/* 有効終了年月日ﾉｰﾏﾗｲｽﾞ		*/
									  (short)PayData.teiki.e_mon,	/*								*/
									  (short)PayData.teiki.e_day );	/*								*/
		}															/*								*/
		if(	( 1 == (uchar)prm_get(COM_PRM, S_BUN, 54, 1, 3) )		/* 定期券精算時に分類集計への加算する */
				&&													/*   且つ						*/
			( DoBunruiSyu(ryo_buf.syubet) ) ){						/* 分類集計する（種別毎ﾁｪｯｸ）	*/
				f_BunruiSyu = 1;									/* 分類集計する情報セット		*/
		}
	}																/*								*/
	else															/*								*/
	{																/*								*/
		if(( ryo_buf.pkiti != 0xffff )&&							/* ﾏﾙﾁ精算以外					*/
		   ( uc_update == 0 ))										/* 定期券更新精算以外			*/
			syusei[iti].ryo = ryo_buf.tyu_ryo;						/* 修正用駐車料金ｾｯﾄ			*/

		if( DoBunruiSyu(ryo_buf.syubet) ){							/* 分類集計する（種別毎ﾁｪｯｸ）	*/
			f_BunruiSyu = 1;										/* 分類集計する情報セット		*/
		}
	}																/*								*/
																	/*								*/
	if( f_BunruiSyu ){												/* 分類集計する					*/

		dat2 = 0L;													/*								*/
		// 駐車料金を求める（割引を含む／含まない設定に従う）
		switch( prm_get(COM_PRM, S_TOT, 12, 1, 2) ){				// 分類集計の駐車料金扱い（設定）により分岐
		case	0:													// 割引前の料金を駐車料金とする
		default:
			dat2 = PayData.WPrice;
			break;
		case	1:													// 割引後の料金を駐車料金とする（プリペは現金領収額とする）
			if( PayData.WPrice >= (wari_ryo + ryo_buf.credit.pay_ryo) )
				dat2 = (PayData.WPrice) - wari_ryo - ryo_buf.credit.pay_ryo;
																	// 駐車料金－割引料金
			break;
		case	2:													// 割引後の料金を駐車料金とする（プリペは割引額とする）
			if( PayData.WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo + PayData.Electron_data.Suica.pay_ryo) )
				dat2 = (PayData.WPrice) - wari_ryo - ppc_ryo - ryo_buf.credit.pay_ryo - PayData.Electron_data.Suica.pay_ryo;
																	// 駐車料金－割引料金－PPC料金－電子ﾏﾈｰ料金
			break;
		}

#if 1 // AKIBA DEBUG TEST 08.06.27(S)
//		bunrui( ryo_buf.pkiti, dat2, ryo_buf.syubet );				/* 分類集計加算		*/
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//		if( wk_sss_flg ){											// 修正精算時
//			if( prm_get(COM_PRM, S_TOT, 20, 1, 6) == 1 ){			// 集計加算のやり直しする
//				if( prm_get(COM_PRM, S_TOT, 12, 1, 2) != 0 ){		// 割引後の料金
//					if( !(PayData.Zengakufg&0x02) ){				// 未入金なし
//						dat2 += syusei[wdt1].gen;
//						dat2 -= PayData.MMTwari;
//					}else{											// 未入金あり
//						dat2 += syusei[wdt1].gen;
//						dat2 -= PayData.MMTwari;
//					}
//				}
//			}
//		}
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
#endif // AKIBA DEBUG TEST 08.06.27(S)
// 仕様変更(S) K.Onodera 2016/12/13 #1674 集計の精算料金に対する振替分の加算方法を変更する
		if( ulFuriWari ){
			dat2 -= ulFuriWari;
		}
// 仕様変更(E) K.Onodera 2016/12/13 #1674 集計の精算料金に対する振替分の加算方法を変更する
		if( ryo_buf.pkiti != 0xffff ){
			bunrui( ryo_buf.pkiti, dat2, ryo_buf.syubet );			/* 分類集計加算		*/
// 仕様変更(S) K.Onodera 2016/12/13 #1674 集計の精算料金に対する振替分の加算方法を変更する
			// 振替精算？
			if( ulFurikae ){
				bunrui_Erace( GetFurikaeSrcReciptData(), ulFurikae, ryo_buf.syubet );
			}
// 仕様変更(E) K.Onodera 2016/12/13 #1674 集計の精算料金に対する振替分の加算方法を変更する
		}else{
			bunrui( 0, dat2, ryo_buf.syubet );						/* 分類集計加算		*/
		}
	}

	ts->tou[0] += (ulong)ryo_buf.in_coin[0];						/* 精算投入金額  10円使用枚数	*/
	ts->tou[1] += (ulong)ryo_buf.in_coin[1];						/* 精算投入金額  50円使用枚数	*/
	ts->tou[2] += (ulong)ryo_buf.in_coin[2];						/* 精算投入金額 100円使用枚数	*/
	ts->tou[3] += (ulong)ryo_buf.in_coin[3];						/* 精算投入金額 500円使用枚数	*/
	ts->tou[4] += (ulong)ryo_buf.in_coin[4];						/* 精算投入金額1000円使用枚数	*/
	ts->tounyu = ts->tou[0]*10 + ts->tou[1]*50 + ts->tou[2]*100 +	/*								*/
				 ts->tou[3]*500 + ts->tou[4]*1000;					/* 精算投入金額	総額			*/
																	/*								*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[0];						/* 精算払出金額  10円使用枚数	*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[1];						/* 精算払出金額  50円使用枚数	*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[2];						/* 精算払出金額 100円使用枚数	*/
	ts->sei[3] += (ulong)ryo_buf.out_coin[3];						/* 精算払出金額 500円使用枚数	*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[4];						/* 精算払出金額  10円使用枚数(予蓄)*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[5];						/* 精算払出金額 50円使用枚数(予蓄)*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[6];						/* 精算払出金額 100円使用枚数(予蓄)*/
	ts->seisan = ts->sei[0]*10 + ts->sei[1]*50 + ts->sei[2]*100 +	/*								*/
				 ts->sei[3]*500;									/* 精算払出金額	総額			*/
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 15;											/* 15:Ｔｴﾘｱ集計加算完了			*/
	Log_regist( LOG_PAYMENT );										/* 個別精算情報登録				*/
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
		if ( OPECTL.f_eReceiptReserve ) {	// 電子領収証予約
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
// 			// RT領収証データ登録
// 			RTReceipt_LogRegist_AddOiban();
// 
// 			// QR領収証データ作成
// 			MakeQRCodeReceipt(qrcode, sizeof(qrcode));
// 
// 			// QR領収証ポップアップ表示
// 			lcdbm_notice_dsp3(POP_QR_RECIPT, 0, 0, (uchar *)qrcode, sizeof(qrcode));	// 12:QR領収証
			// QR領収証表示
			QrReciptDisp();
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
		}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
		// QR確定・取消データ登録
		ope_SendCertifCommit();
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
	}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
	Suica_Rec.Data.BIT.LOG_DATA_SET = 0;							/* Suica精算ﾌﾗｸﾞｸﾘｱ				*/
	ac_flg.cycl_fg = 19;		// ログ登録完了
																	/*								*/
	if( ryo_buf.ryo_flg >= 2 )										/* 定期使用?					*/
	{																/*								*/
		memcpy( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* 中止定期ﾃﾞｰﾀ退避				*/
																	/*								*/
		ac_flg.cycl_fg = 20;		// 定期券中止テーブル退避完了～同更新中
																	/*								*/
		for( i=0; i<TKI_CYUSI_MAX; i++ )							/*								*/
		{															/*								*/
			if( tki_cyusi.dt[i].no == PayData.teiki.id &&			/* 中止ﾊﾞｯﾌｧに使用定期ﾃﾞｰﾀ有り?	*/
				tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*					*/
			{														/*								*/
				NTNET_Snd_Data219(1, &tki_cyusi.dt[i]);				/* 1件削除通知送信				*/
				TKI_Delete(i);										/* 中止定期1件ｸﾘｱ				*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* 定期券ﾃｰﾌﾞﾙ更新(出庫)	*/
					  (ushort)PayData.teiki.id,						/*								*/
					  1,											/*								*/
					  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*							*/
					  0xffff );										/*								*/
	}																/*								*/
																	/*								*/
	CountUp( PAYMENT_COUNT );										/* 精算追番+1					*/
	if( ryo_buf.fusoku != 0 ){										/* 預り証発行?					*/
		CountUp(DEPOSIT_COUNT);										/* 預り証追番+1					*/
	}
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
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
																	/*								*/
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	if( PayData.credit.pay_ryo ){									/* ｸﾚｼﾞｯﾄｶｰﾄﾞ使用				*/
//		ac_flg.cycl_fg = 80;										// クレジット利用明細登録開始
//		Log_regist( LOG_CREUSE );									/* クレジット利用明細登録		*/
//	}																/*								*/
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
	if (EcUseEMoneyKindCheck(PayData.Electron_data.Ec.e_pay_kind) &&
		PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
		// 引き去り済みを確認した場合も処理未了取引記録を登録する
		ac_flg.cycl_fg = 90;										// 処理未了取引記録登録開始
		EcAlarmLog_Regist(&EcAlarm.Ec_Res);
		ac_flg.cycl_fg = 91;										// 処理未了取引記録登録完了
		// 処理未了取引記録の再精算情報を登録する
		EcAlarmLog_RepayLogRegist(&PayData);
	}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
	ac_flg.cycl_fg = 17;											/* 17:個別精算ﾃﾞｰﾀ登録完了		*/
																	/*								*/
	return;															/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 種別毎 分類集計する／しないチェック															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: chu_syuu( void )																   |*/
/*| PARAMETER	: 種別  : 0=A種、...11=L種														   |*/
/*| RETURN VALUE: 1=分類集計する、0=しない														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Date		: 2005-10-25																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	DoBunruiSyu( char Syubetu )									/*								*/
{																	/*								*/
	short	Num;													/* 共通設定番号	(52 or 53)		*/
	char	Ichi;													/* 共通設定内 データ位置(1-6)	*/

	if( 6 > Syubetu )
		Num = 52;
	else
		Num = 53;

	Ichi = (char)((Syubetu % 6) + 1);

	if( 0 != (uchar)prm_get(COM_PRM, S_BUN, Num, 1, Ichi) )			/* 種別毎 分類集計への加算する 	*/
		return	(char)1;											/* 分類集計する					*/

	return	(char)0;												/* 分類集計しない				*/
}
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 精算中止集計																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: chu_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT45EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	cyu_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	ulong	dat1, dat2;												/*								*/
	ushort	iti;													/*								*/
	short	i;														/*								*/
	uchar	f_SameData;												/* 1=定期中止ﾃｰﾌﾞﾙ内に目的ﾃﾞｰﾀあり */
	uchar	uc_update = PayData.teiki.update_mon;					/* 更新月格納					*/
	ushort	Flp_sub_no;
	uchar	Flp_sub_ok;
	uchar	wk_cnt=0;
	uchar	uc_buf_size;
	wari_tiket	wari_dt;
	short	j;
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	DATE_SYOUKEI	*date_ts;
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
	uchar	MiryoaddFlag = 0;
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
	uchar	taxableAdd_set;											/*	課税対象売上加算設定		*/
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	date_ts = &Date_Syoukei;
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	iti = ryo_buf.pkiti - 1;										/* 駐車位置						*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* Ｔ集計をﾜｰｸｴﾘｱへ（停電対策）	*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	date_uriage_syoukei_judge();									// 日付切替基準 総売上、現金総売上小計の時刻更新判定（リアルタイム情報用）
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
	DailyAggregateDataBKorRES( 0 );									/* 日毎集計エリアのバックアップ	*/
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
																	/*								*/
	ac_flg.cycl_fg = 24;											/* 24:Ｔ集計をﾜｰｸｴﾘｱへ転送完了	*/
																	/*								*/
																	/*								*/
	ts->Seisan_chusi_cnt += 1L;										/* 精算中止回数					*/
	ts->Seisan_chusi_ryo += ryo_buf.nyukin;							/* 中止時現金投入額				*/
																	/*								*/
	if( ryo_buf.fusoku != 0 )										/* 支払い不足額有り？			*/
	{																/*								*/
		ts->Harai_husoku_cnt += 1L;									/* 支払い不足回数				*/
		ts->Harai_husoku_ryo += ryo_buf.fusoku;						/* 支払い不足額					*/
	}																/*								*/

// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
	if (EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
		PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) {

		if (PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0) {
			// 決済中に障害発生、または、未了残高照会タイムアウトした
			// 処理未了取引（支払不明）集計
			ts->miryo_unknown_cnt += 1L;
			ts->miryo_unknown_ryo += EcAlarm.Ec_Res.settlement_data;

			// 日毎集計は当日としてカウントする
			Syuukei_sp.ec_inf.now.sp_miryo_unknown_cnt += 1L;
			Syuukei_sp.ec_inf.now.sp_miryo_unknown_ryo += EcAlarm.Ec_Res.settlement_data;
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
			MiryoaddFlag = 1;
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
		}
	}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
																	/*								*/
	if(Flap_Sub_Flg == 1){									// 最精算＋Bエリア有
		Flp_sub_ok = 1;
		Flp_sub_no = Flap_Sub_Num;							// 作業中詳細ｴﾘｱ格納
	}else if((Flap_Sub_Flg == 2) || (Flap_Sub_Num == 10)){	// 再精算＋Bエリア無、マルチ精算(Bｴﾘｱ無)の時
		Flp_sub_ok = 0;
	}else{														// 再精算無、初精算中止
		for(i = 0 ; i < 10 ; i++){
			if(FLAPDT_SUB[i].WPlace == 0){
				break;
			}
		}
		if(i < 10){
			Flp_sub_ok = 1;
			Flp_sub_no = i;
		}else{
			memmove(&FLAPDT_SUB[0],&FLAPDT_SUB[1],sizeof(flp_com_sub)*9);			//前に１個シフト
			memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));	//ｸﾘｱ
			Flp_sub_ok = 1;
			Flp_sub_no = 9;
		}
	}
	if( CPrmSS[S_PRP][1] == 1 )										/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞを使用する設定	*/
	{																/*								*/
		dat1 = prm_get( COM_PRM,S_TOT,5,1,3 );						/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ総売上額/総掛売額加算設定	*/
		j = 0;														/*								*/
		for( i = 0; i < WTIK_USEMAX; i++ ){							/*								*/
			disc_wari_conv( &PayData.DiscountData[i], &wari_dt );	/*								*/
			if( wari_dt.tik_syu == PREPAID ){						/* ﾃﾞｰﾀあり						*/
				dat2 = wari_dt.pkno_syu;							/*								*/
				ts->Pcrd_use_cnt[dat2] += ryo_buf.pri_mai[j];		/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜回数＞(基本,拡張1,2,3)	*/
																	/* 7枚以上使用時は最後に使われた種別に合計加算する	*/
				ts->Pcrd_use_ryo[dat2] += wari_dt.ryokin;			/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜金額＞(基本,拡張1,2,3)	*/
																	/*								*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				if(Flp_sub_ok){										/* 詳細中止エリア使用			*/
					FLAPDT_SUB[Flp_sub_no].ppc_chusi_ryo += wari_dt.ryokin;	/* 詳細中止エリアに加算する */
				}													/*								*/
				j++;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ )								/*								*/
	{																/*								*/
		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );		/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
		taxableAdd_set = 0xff;
		if( cancelReceipt_Waridata_chk( &wari_dt ) ){				/*			課税対象			*/
			taxableAdd_set = (uchar)prm_get( COM_PRM, S_TOT, 7, 1, 2 );	/*課税対象利用額の売上加算設定*/
		}															/*								*/
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
		switch( wari_dt.tik_syu )									/*								*/
		{															/*								*/
			case SERVICE:											/* ｻｰﾋﾞｽ券						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* ｻｰﾋﾞｽ券A,B,C					*/
																	/*								*/
				ts->Stik_use_cnt[dat1][dat2] += wari_dt.maisuu;		/* ｻｰﾋﾞｽ券種別毎使用＜枚数＞(基本,拡張1,2,3)(ｻｰﾋﾞｽ券A,B,C)	*/
				ts->Stik_use_ryo[dat1][dat2] += wari_dt.ryokin;		/* ｻｰﾋﾞｽ券種別毎使用＜金額＞(基本,拡張1,2,3)(ｻｰﾋﾞｽ券A,B,C)	*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//				if( CPrmSS[S_SER][1+3*dat2] == 4 &&					/* ｻｰﾋﾞｽ券全額割引設定?			*/
				if( prm_get( COM_PRM, S_SER, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* ｻｰﾋﾞｽ券全額割引設定? */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					ryo_buf.pkiti != 0xffff &&						/* ﾏﾙﾁ精算以外?					*/
					uc_update == 0 )								/* 定期券更新精算以外			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* 修正用全額割引ﾌﾗｸﾞｾｯﾄ		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,1 );				/* ｻｰﾋﾞｽ券総売上額/総掛売額加算設定	*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				setting_taxableAdd( &dat1, taxableAdd_set );		/* 課税対象利用額の売上加算設定 */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				if(Flp_sub_ok){
					FLAPDT_SUB[Flp_sub_no].sev_tik[dat2] += wari_dt.maisuu;	// 詳細中止エリアに加算する
				}
				break;												/*								*/
																	/*								*/
			case KAKEURI:											/* 掛売券						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* 店№1～999(0～998)			*/
																	/*								*/
				ts->Mno_use_Tcnt += wari_dt.maisuu;					/* 店割引総ﾄｰﾀﾙ＜回数＞			*/
				ts->Mno_use_Tryo += wari_dt.ryokin;					/* 店割引総ﾄｰﾀﾙ＜金額＞			*/
																	/*								*/
				ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;			/* 店№割引ﾄｰﾀﾙ＜回数＞(基本,拡張1,2,3)	*/
				ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;			/* 店№割引ﾄｰﾀﾙ＜金額＞(基本,拡張1,2,3)	*/
																	/*								*/
				if( dat1 == KIHON_PKNO || dat1 == KAKUCHOU_1 )		/* 基本 or 拡張1				*/
				{													/*								*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
//					if( dat2 < 100 )								/* 店№1～100					*/
					if( dat2 < MISE_NO_CNT )						/* 店№1～100					*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* 店№1～100ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* 店№1～100ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
						ts->Mno_use_cnt4[dat1][dat2] +=				/* 店№毎割引＜回数＞(基本,拡張1)(店№1～100)	*/
												wari_dt.maisuu;		/*								*/
						ts->Mno_use_ryo4[dat1][dat2] +=				/* 店№毎割引＜金額＞(基本,拡張1)(店№1～100)	*/
												wari_dt.ryokin;		/*								*/
					}												/*								*/
					else											/* 店№101～999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* 店№101～999ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* 店№101～999ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
					}												/*								*/
				}													/*								*/
				else												/*								*/
				{													/*								*/
					if(( dat1 == KAKUCHOU_2 )&&						/* 拡張2						*/
					   ( prm_get( COM_PRM,S_TOT,16,1,3 ) == 1 ))	/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
					else if(( dat1 == KAKUCHOU_3 )&&				/* 拡張3						*/
							( prm_get( COM_PRM,S_TOT,16,1,4 ) == 1 ))/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
//					if( dat2 < 100 )								/* 店№1～100					*/
					if( dat2 < MISE_NO_CNT )						/* 店№1～100					*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* 店№1～100ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* 店№1～100ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
						if( dat1 == KAKUCHOU_1 )					/*								*/
						{											/*								*/
							ts->Mno_use_cnt4[dat1][dat2] +=			/* 店№毎割引＜回数＞(基本,拡張1)(店№1～100)	*/
												wari_dt.maisuu;		/*								*/
							ts->Mno_use_ryo4[dat1][dat2] +=			/* 店№毎割引＜金額＞(基本,拡張1)(店№1～100)	*/
												wari_dt.ryokin;		/*								*/
						}											/*								*/
					}												/*								*/
					else											/* 店№101～999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* 店№101～999ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* 店№101～999ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
					}												/*								*/
				}													/*								*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//				if( CPrmSS[S_STO][1+3*dat2] == 4 &&					/* 店割引全額割引設定?			*/
				if( prm_get( COM_PRM, S_STO, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* 店割引全額割引設定? */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					ryo_buf.pkiti != 0xffff &&						/* ﾏﾙﾁ精算以外?					*/
					uc_update == 0 )								/* 定期券更新精算以外			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* 修正用全額割引ﾌﾗｸﾞｾｯﾄ		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* 店割引総売上額/総掛売額加算設定	*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				setting_taxableAdd( &dat1, taxableAdd_set );		/* 課税対象利用額の売上加算設定 */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				if(Flp_sub_ok){
					for(wk_cnt=0;wk_cnt<5;wk_cnt++){				//同じ店No検索
						if(FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no == wari_dt.syubetu){
							FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].maisuu += wari_dt.maisuu;//同じ店Noの時、枚数を格納する
							break;
						}
					}
					if(wk_cnt == 5){								//同じ店No無
						for(wk_cnt=0;wk_cnt<5;wk_cnt++){
							if(FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no == 0){
								FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no = wari_dt.syubetu;//店舗No
								FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].maisuu += wari_dt.maisuu;//枚数
								break;
							}
						}
					}
					if(wk_cnt == 5){								//同じ店No無＋詳細中止空きｴﾘｱ無

						if(Flp_sub_no < 9){
							uc_buf_size = (uchar)(9 - Flp_sub_no);
							memset(&FLAPDT_SUB[Flp_sub_no],0,sizeof(flp_com_sub));			// 詳細中止ｴﾘｱ(精算完了)ｸﾘｱ
							memmove(&FLAPDT_SUB[Flp_sub_no],&FLAPDT_SUB[Flp_sub_no+1],sizeof(flp_com_sub)*uc_buf_size);
						}
						if(Flap_Sub_Num == 10){
							memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));	// 終端詳細中止ｴﾘｱｸﾘｱ
						}else{
							memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));	// 終端詳細中止ｴﾘｱｸﾘｱ
						}
						Flp_sub_ok = 0;								//格納ｸﾞﾗｸﾞｸﾘｱ
					}
				}
				break;												/*								*/

			case KAISUU:											/* 回数券						*/
				if( CPrmSS[S_PRP][1] != 2 )							/* 回数券を使用する設定ではない	*/
					break;

				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
																	/*								*/
				ts->Ktik_use_cnt[dat1] += wari_dt.maisuu;			/* 使用＜枚数＞(基本,拡張1,2,3) */
				ts->Ktik_use_ryo[dat1] += wari_dt.ryokin;			/* 使用＜金額＞(基本,拡張1,2,3)	*/

				dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );				/* 回数券総売上額/総掛売額加算設定	*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				if(Flp_sub_ok){										//詳細中止エリア使用
					FLAPDT_SUB[Flp_sub_no].ppc_chusi_ryo += wari_dt.ryokin;	// 詳細中止エリアに加算する。
				}
				break;												/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
																	/*								*/
	ts->tou[0] += (ulong)ryo_buf.in_coin[0];						/* 精算投入金額  10円使用枚数	*/
	ts->tou[1] += (ulong)ryo_buf.in_coin[1];						/* 精算投入金額  50円使用枚数	*/
	ts->tou[2] += (ulong)ryo_buf.in_coin[2];						/* 精算投入金額 100円使用枚数	*/
	ts->tou[3] += (ulong)ryo_buf.in_coin[3];						/* 精算投入金額 500円使用枚数	*/
	ts->tou[4] += (ulong)ryo_buf.in_coin[4];						/* 精算投入金額1000円使用枚数	*/
	ts->tounyu = ts->tou[0]*10 + ts->tou[1]*50 + ts->tou[2]*100 +	/*								*/
				 ts->tou[3]*500 + ts->tou[4]*1000;					/* 精算投入金額	総額			*/
																	/*								*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[0];						/* 精算払出金額  10円使用枚数	*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[1];						/* 精算払出金額  50円使用枚数	*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[2];						/* 精算払出金額 100円使用枚数	*/
	ts->sei[3] += (ulong)ryo_buf.out_coin[3];						/* 精算払出金額 500円使用枚数	*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[4];						/* 精算払出金額  10円使用枚数(予蓄)*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[5];						/* 精算払出金額 50円使用枚数(予蓄)*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[6];						/* 精算払出金額 100円使用枚数(予蓄)*/
	ts->seisan = ts->sei[0]*10 + ts->sei[1]*50 + ts->sei[2]*100 +	/*								*/
				 ts->sei[3]*500;									/* 精算払出金額	総額			*/
																	/*								*/
	if(( card_use[USE_SVC] || card_use[USE_PPC] || card_use[USE_NUM] )&&	/* 割引有り?					*/
	   ryo_buf.pkiti != 0xffff &&									/* ﾏﾙﾁ精算以外?					*/
	   uc_update == 0 )												/* 定期券更新精算以外			*/
	{																/*								*/
		FLAPDT.flp_data[iti].bk_syu = (ushort)(ryo_buf.syubet + 1);	/* 種別(中止,修正用)			*/
		FLAPDT.flp_data[iti].bk_wmai = card_use[USE_SVC];			/* 使用枚数(中止,修正用)		*/
		FLAPDT.flp_data[iti].bk_wari = ryo_buf.fee_amount;			/* 割引金額(中止,修正用)		*/
		FLAPDT.flp_data[iti].bk_time = ryo_buf.tim_amount;			/* 割引時間数(中止,修正用)		*/
		if(Flp_sub_ok){
			FLAPDT_SUB[Flp_sub_no].syu = (uchar)(ryo_buf.syubet + 1);		// 種別
			FLAPDT_SUB[Flp_sub_no].TInTime = PayData.TInTime;				// 入庫時間
			FLAPDT_SUB[Flp_sub_no].WPlace = PayData.WPlace;			// 車室番号を格納する
		}
	}																/*								*/
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 25;											/* 25:Ｔｴﾘｱ集計加算完了			*/
																	/*								*/
	if( ryo_buf.ryo_flg >= 2 )										/* 定期使用?					*/
	{																/*								*/
		memcpy( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* 中止定期ﾃﾞｰﾀ退避				*/
																	/*								*/
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
//		Log_regist( LOG_PAYSTOP );									/* 精算中止情報登録				*/
		if(	MiryoaddFlag == 1){
			Log_regist( LOG_PAYSTOP_FU );									/* 精算中止情報登録				*/
		}
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
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
		Suica_Rec.Data.BIT.LOG_DATA_SET = 0;						/* Suica精算ﾌﾗｸﾞｸﾘｱ				*/
																	/*								*/
		f_SameData = 0;												/*								*/
																	/*								*/
		if( TKI_CYUSI_MAX < tki_cyusi.count )						/* fail safe					*/
			tki_cyusi.count = TKI_CYUSI_MAX;						/*								*/
																	/*								*/
		if( TKI_CYUSI_MAX <= tki_cyusi.wtp )						/* fail safe					*/
			tki_cyusi.wtp = tki_cyusi.count - 1;					/*								*/
																	/*								*/
		for( i=0; i<tki_cyusi.count; i++ )							/*								*/
		{															/*								*/
			if( 0L == tki_cyusi.dt[i].pk ){							/* ﾃｰﾌﾞﾙにﾃﾞｰﾀなし				*/
				break;												/* i=登録件数					*/
			}														/*								*/
			if( tki_cyusi.dt[i].no == PayData.teiki.id &&			/* 再中止?						*/
				tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*					*/
			{														/*								*/
				f_SameData = 1;										/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		if( 0 == f_SameData )										/* 同一ﾃﾞｰﾀなし(新規登録必要)	*/
		{															/*								*/
			if( i == TKI_CYUSI_MAX ){								/* 登録件数Full					*/
				NTNET_Snd_Data219(1, &tki_cyusi.dt[0]);				/* 1件削除通知送信				*/
				TKI_Delete(0);										/* 最古ﾃﾞｰﾀ削除					*/
				nmisave( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* 中止定期ﾃﾞｰﾀ退避		*/
			}														/*								*/
			tki_cyusi.dt[tki_cyusi.wtp].syubetu = PayData.syu;		/* 料金種別(A～L:1～12)			*/
			tki_cyusi.dt[tki_cyusi.wtp].pk =						/*								*/
						CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];	/* 駐車場№						*/
			tki_cyusi.dt[tki_cyusi.wtp].no = PayData.teiki.id;		/* 個人ｺｰﾄﾞ(1～12000)			*/
			tki_cyusi.dt[tki_cyusi.wtp].tksy = PayData.teiki.syu;	/* 定期種別(1～15)				*/
			tki_cyusi.dt[tki_cyusi.wtp].year = PayData.TInTime.Year;/* 処理年						*/
			memcpy( &tki_cyusi.dt[tki_cyusi.wtp].mon, PayData.teiki.t_tim, 4 );	/* 処理月日時分		*/
			tki_cyusi.dt[tki_cyusi.wtp].sec = 0;					/* 処理秒						*/
																	/*								*/
			NTNET_Snd_Data219(0, &tki_cyusi.dt[tki_cyusi.wtp]);		/* 1件更新通知送信				*/
																	/*								*/
			if( tki_cyusi.count < TKI_CYUSI_MAX )					/*								*/
			{														/*								*/
				tki_cyusi.count++;									/* 登録件数を+1					*/
			}														/*								*/
																	/*								*/
			tki_cyusi.wtp++;										/* ﾗｲﾄﾎﾟｲﾝﾀ+1					*/
			if( tki_cyusi.wtp >= TKI_CYUSI_MAX )					/*								*/
			{														/*								*/
				tki_cyusi.wtp = 0;									/*								*/
			}														/*								*/
		}															/*								*/
		WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* 定期券ﾃｰﾌﾞﾙ更新(出庫)	*/
					  (ushort)PayData.teiki.id,						/*								*/
					  1,											/*								*/
					  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*							*/
					  0xffff );										/*								*/
	}																/*								*/
	else{															/*								*/
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
//		Log_regist( LOG_PAYSTOP );									/* 精算中止情報登録				*/
		if(	MiryoaddFlag == 1){
			Log_regist( LOG_PAYSTOP_FU );									/* 精算中止情報登録				*/
		}
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6386 50-0014で精算中止または、休業設定で決済失敗時で精算中止した際に精算データが送信されない
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
		Suica_Rec.Data.BIT.LOG_DATA_SET = 0;						/* Suica精算ﾌﾗｸﾞｸﾘｱ				*/
	}																/*								*/
																	/*								*/
	CountUp( CANCEL_COUNT );										/* 精算中止追番+1				*/
	if( ryo_buf.fusoku != 0 ){										/* 預り証発行?					*/
		CountUp(DEPOSIT_COUNT);										/* 預り証追番+1					*/
	}
// MH810100(S) 2020/06/22  #4507 ID56のｾﾝﾀ追番が正しくｲﾝｸﾘﾒﾝﾄされていない場合がある
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
//	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//		RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
//	}
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5379：精算中止の精算データ送信後に精算データのセンター追番が重複する不具合の対策）
//	if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime精算中止データ送信する
	if( PAY_CAN_DATA_SEND || MiryoaddFlag == 1 ){
		// ParkingWeb/RealTime精算中止データ送信するまたは決済リーダによる精算中止情報登録時
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5379：精算中止の精算データ送信後に精算データのセンター追番が重複する不具合の対策）
		if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
		}
	}
// MH810100(E) 2020/06/22  #4507 ID56のｾﾝﾀ追番が正しくｲﾝｸﾘﾒﾝﾄされていない場合がある
// MH810100(S) K.Onodera  2020/01/22 車番チケットレス(QR確定・取消データ対応)
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
	if( PayData.PayMode !=4 ){	// 遠隔精算でない？
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
		DC_PopCenterSeqNo( DC_SEQNO_QR );	// センター追番をPop
// MH810100(S) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
	}
// MH810100(E) K.Onodera  2020/03/03 #3944 車番チケットレス(遠隔精算で精算終了時にリアルタイムサーバーへ精算データを送信する不具合修正)
// MH810100(E) K.Onodera  2020/01/22 車番チケットレス(QR確定・取消データ対応)
																	/*								*/
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
	if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
		 PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
		(PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0)) {
		// 決済中に障害発生、または、未了残高照会タイムアウトしたため、
		// 処理未了取引記録を登録する
		ac_flg.cycl_fg = 100;										// 処理未了取引記録登録開始
		EcAlarmLog_Regist(&EcAlarm.Ec_Res);
	}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
	ac_flg.cycl_fg = 27;											/* 27:精算中止ﾃﾞｰﾀ登録完了		*/
																	/*								*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| ﾏﾙﾁ精算完了集計(精算ﾃﾞｰﾀ受信集計)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: mulkan_syu( pr_lokno )														   |*/
/*| PARAMETER	: pr_lokno : 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-10-31																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	mulkan_syu( ushort pr_lokno )								/*								*/
{																	/*								*/
	LOKTOTAL_DAT	*lkts;											/*								*/
	ulong	dat1;													/*								*/
	ushort	iti;													/*								*/
	struct CAR_TIM wk_time1,wk_time2;								/*								*/
	uchar	i;														/*								*/
	uchar	buf_size;												/*								*/
																	/*								*/
	iti = pr_lokno - 1;												/* 駐車位置						*/
	lkts = &loktl.tloktl.loktldat[iti];								/*								*/
	memcpy( &wkloktotal, lkts, sizeof( LOKTOTAL_DAT ) );			/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
																	/*								*/
																	/*								*/
																	/*								*/
//	dat1 = RecvNtnetDt.RData22.InPrice - RecvNtnetDt.RData22.ChgPrice;/* 入金額－釣り銭額			*/
	dat1 = RecvNtnetDt.RData22.CashPrice;							/* 現金売上						*/
																	/*								*/
																	/*								*/
	memset( &syusei[iti], 0, sizeof(struct SYUSEI));				/* 修正精算用ﾃﾞｰﾀｸﾘｱ			*/
																	/*								*/
																	/*								*/
	syusei[iti].gen = dat1;											/* 現金領収額(修正用)			*/
	syusei[iti].tax = RecvNtnetDt.RData22.Tax;						/* 修正用消費税額				*/
																	/*								*/
	syusei[iti].iyear = RecvNtnetDt.RData22.InTime.Year;			/* 入庫 年(修正用)				*/
	syusei[iti].imont = RecvNtnetDt.RData22.InTime.Mon;				/*      月						*/
	syusei[iti].idate = RecvNtnetDt.RData22.InTime.Day;				/*      日						*/
	syusei[iti].ihour = RecvNtnetDt.RData22.InTime.Hour;			/*      時						*/
	syusei[iti].iminu = RecvNtnetDt.RData22.InTime.Min;				/*      分						*/
	syusei[iti].oyear = RecvNtnetDt.RData22.OutTime.Year;			/* 出庫 年(修正用)				*/
	syusei[iti].omont = RecvNtnetDt.RData22.OutTime.Mon;			/*      月						*/
	syusei[iti].odate = RecvNtnetDt.RData22.OutTime.Day;			/*      日						*/
	syusei[iti].ohour = RecvNtnetDt.RData22.OutTime.Hour;			/*      時						*/
	syusei[iti].ominu = RecvNtnetDt.RData22.OutTime.Min;			/*      分						*/
//	syusei[iti].ot_car = 0;											/* 未出庫						*/
//	syusei[iti].warigk = ryo_buf.waribik + c_pay;					/* 修正用割引額ｾｯﾄ				*/
																	/*								*/
//	syusei[iti].sy_wmai = card_use[USE_SVC];						/* 使用枚数(修正用)				*/
//	syusei[iti].sy_wari = ryo_buf.fee_amount;						/* 割引金額(修正用)				*/
//	syusei[iti].sy_time = ryo_buf.tim_amount;						/* 割引時間数(修正用)			*/
																	/*								*/
	if(( CPrmSS[S_CAL][19] != 0 )&&									/* 消費税有り?					*/
	   ( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2 )&&					/* 外税?						*/
	   ( dat1 >= RecvNtnetDt.RData22.Tax ))							/* 税金対象額＞＝税金?			*/
	{																/*								*/
		lkts->Genuri_ryo += (dat1 - RecvNtnetDt.RData22.Tax);		/* 駐車位置番号別集計(現金売り上げ)	*/
	} else {														/*								*/
		lkts->Genuri_ryo += dat1;									/* 駐車位置番号別集計(現金売り上げ)	*/
	}																/*								*/
																	/*								*/
	lkts->Seisan_cnt += 1;											/* 駐車位置番号別集計(精算台数)	*/
																	/*								*/
	if( RecvNtnetDt.RData22.PassData.PassID )						/* 定期使用?					*/
	{																/*								*/
		syusei[iti].tei_syu =										/*								*/
				(uchar)RecvNtnetDt.RData22.PassData.Syubet;			/* 定期種別						*/
		syusei[iti].tei_id =										/*								*/
				(ushort)RecvNtnetDt.RData22.PassData.PassID;		/* 定期券id(1～12000)			*/
		syusei[iti].tei_sd =										/*								*/
			dnrmlzm( (short)RecvNtnetDt.RData22.PassData.SYear,		/* 有効開始年月日ﾉｰﾏﾗｲｽﾞ		*/
				(short)RecvNtnetDt.RData22.PassData.SMon,			/*								*/
				(short)RecvNtnetDt.RData22.PassData.SDate );		/*								*/
		syusei[iti].tei_ed =										/*								*/
			dnrmlzm( (short)RecvNtnetDt.RData22.PassData.EYear,		/* 有効終了年月日ﾉｰﾏﾗｲｽﾞ		*/
				(short)RecvNtnetDt.RData22.PassData.EMon,			/*								*/
				(short)RecvNtnetDt.RData22.PassData.EDate );		/*								*/
	}																/*								*/
	syusei[iti].ryo = RecvNtnetDt.RData22.Price;					/* 修正用駐車料金ｾｯﾄ			*/
																	/*								*/
	for(i = 0 ; i < 10 ; i++){										/*	精算中止ﾃﾞｰﾀ検索			*/
		if(FLAPDT_SUB[i].WPlace ==  LockInfo[iti].posi){			/*	駐車場Noあり				*/
			wk_time1.year = RecvNtnetDt.RData22.InTime.Year;		/*								*/
			wk_time1.mon = RecvNtnetDt.RData22.InTime.Mon;			/*								*/
			wk_time1.day = RecvNtnetDt.RData22.InTime.Day;			/*								*/
			wk_time1.hour = RecvNtnetDt.RData22.InTime.Hour;		/*								*/
			wk_time1.min = RecvNtnetDt.RData22.InTime.Min;			/*								*/
			/*詳細中止入庫ﾃﾞｰﾀ*/									/*								*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;				/*								*/
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;				/*								*/
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;				/*								*/
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;				/*								*/
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;				/*								*/
																	/*								*/
			if(0 == ec64(&wk_time1,&wk_time2)){						/*	入庫時間同じ				*/
				buf_size = (uchar)(9 - i);							/*								*/
				memset(&FLAPDT_SUB[i],0,sizeof(flp_com_sub));		/*	詳細中止ｴﾘｱ(精算完了)ｸﾘｱ	*/
				memmove(&FLAPDT_SUB[i],&FLAPDT_SUB[i+1],sizeof(flp_com_sub)*buf_size);
				memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));		/*	終端詳細中止ｴﾘｱｸﾘｱ			*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| ﾏﾙﾁ精算中止集計(精算ﾃﾞｰﾀ受信集計)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: mulchu_syu( pr_lokno )														   |*/
/*| PARAMETER	: pr_lokno : 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-10-31																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	mulchu_syu( ushort pr_lokno )								/*								*/
{																	/*								*/
	ushort	iti, us_wsyu;											/*								*/
	short	i;														/*								*/
	struct CAR_TIM wk_time1,wk_time2;								/*	ﾜｰｸ時間ｴﾘｱ					*/
	uchar	cyuushi_flg1=0;											/*	中止ﾃﾞｰﾀｴﾘｱﾌﾗｸﾞ				*/
	uchar	cyuushi_flg2=0;											/*	詳細中止ﾃﾞｰﾀｴﾘｱﾌﾗｸﾞ			*/
	ushort	flap_no;												/*	駐車位置					*/
	uchar	wk_cnt;													/*	ﾜｰｸｶｳﾝﾀ						*/
	uchar	uc_buf_size;
																	/*								*/
	iti = pr_lokno - 1;												/* 駐車位置						*/
																	/*								*/
	if(FLAPDT.flp_data[iti].bk_syu) {								/*								*/
		cyuushi_flg1 = 1;											/*	中止ﾃﾞｰﾀｴﾘｱﾌﾗｸﾞON			*/
	}else{															/*								*/
		FLAPDT.flp_data[iti].bk_wari = 0;							/*								*/
		FLAPDT.flp_data[iti].bk_wmai = 0;							/*								*/
		FLAPDT.flp_data[iti].bk_time = 0;							/*								*/
	}																/*								*/
	us_wsyu = RecvNtnetDt.RData22.Syubet;							/* 中止種別get					*/
	if( prm_get( COM_PRM,S_SHA,(short)(1+6*(us_wsyu-1)),2,5 ) ){	/* 中止種別は設定済み?			*/
		FLAPDT.flp_data[iti].bk_syu = us_wsyu;						/* 種別(中止,修正用)			*/
	}																/*								*/
	for(i = 0 ; i < 10 ; i++){										/*	精算中止ﾃﾞｰﾀ検索			*/
		if(FLAPDT_SUB[i].WPlace ==  LockInfo[iti].posi){			/*	駐車場Noあり				*/
			/*入庫ﾃﾞｰﾀ*/											/*								*/
			wk_time1.year = RecvNtnetDt.RData22.InTime.Year;		/*								*/
			wk_time1.mon = RecvNtnetDt.RData22.InTime.Mon;			/*								*/
			wk_time1.day = RecvNtnetDt.RData22.InTime.Day;			/*								*/
			wk_time1.hour = RecvNtnetDt.RData22.InTime.Hour;		/*								*/
			wk_time1.min = RecvNtnetDt.RData22.InTime.Min;			/*								*/
			/*詳細中止入庫ﾃﾞｰﾀ*/									/*								*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;				/*								*/
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;				/*								*/
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;				/*								*/
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;				/*								*/
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;				/*								*/
																	/*								*/
			if(0 == ec64(&wk_time1,&wk_time2)){						/*	入庫時間同じ				*/
				if(cyuushi_flg1 == 0){								/*	中止ｴﾘｱ無					*/
					memset(&FLAPDT_SUB[i],0,sizeof(flp_com_sub));	/*	詳細中止ﾃﾞｰﾀｴﾘｱｸﾘｱ			*/
				}													/*								*/
				flap_no = i;										/*	駐車位置ｾｯﾄ					*/
				cyuushi_flg2 = 1;									/*	詳細中止ﾃﾞｰﾀｴﾘｱﾌﾗｸﾞON		*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
	if((i == 10) && (cyuushi_flg1 == 0) && (cyuushi_flg2 == 0)){	/*	中止、詳細中止ﾌﾗｸﾞOFF		*/
		for(i = 0 ; i < 10 ; i++){									/*	詳細中止ｴﾘｱ空き検索			*/
			if(FLAPDT_SUB[i].WPlace == 0){							/*	空き有						*/
				flap_no = i;										/*								*/
				cyuushi_flg2 = 1;									/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		if(i > 10){													/*	空き無						*/
			memmove(&FLAPDT_SUB[0],&FLAPDT_SUB[1],sizeof(flp_com_sub)*9);/*	一番古いﾃﾞｰﾀ削除		*/
			memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));			/*	最終ｴﾘｱｸﾘｱ					*/
			cyuushi_flg2 = 1;										/*								*/
			flap_no = 9;											/*								*/
		}															/*								*/
		if(cyuushi_flg2){											/*								*/
			FLAPDT_SUB[flap_no].WPlace = LockInfo[iti].posi;		/*								*/
			FLAPDT_SUB[flap_no].TInTime.Year = RecvNtnetDt.RData22.InTime.Year;	/*					*/
			FLAPDT_SUB[flap_no].TInTime.Mon = RecvNtnetDt.RData22.InTime.Mon;	/*					*/
			FLAPDT_SUB[flap_no].TInTime.Day = RecvNtnetDt.RData22.InTime.Day;	/*					*/
			FLAPDT_SUB[flap_no].TInTime.Hour = RecvNtnetDt.RData22.InTime.Hour;	/*					*/
			FLAPDT_SUB[flap_no].TInTime.Min = RecvNtnetDt.RData22.InTime.Min;	/*					*/
			FLAPDT_SUB[flap_no].syu = (uchar)RecvNtnetDt.RData22.Syubet;	/*						*/
		}															/*								*/
	}																/*								*/
	for( i=0; i<NTNET_DIC_MAX; i++ ){								/*								*/
		switch( RecvNtnetDt.RData22.DiscountData[i].DiscSyu ){		/* 割引種別						*/
		case NTNET_SVS_M:											/* 割引種別 ｻｰﾋﾞｽ券(料金)		*/
		case NTNET_KAK_M:											/* 割引種別 店割引(料金)		*/
		case NTNET_TKAK_M:											/* 割引種別 多店舗(料金)		*/
		case NTNET_WRI_M:											/* 割引種別 割引券(料金)		*/
		case NTNET_FRE:												/* 割引種別 回数券				*/
		case NTNET_PRI_W:											/* 割引種別 割引ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ	*/
			if( RecvNtnetDt.RData22.DiscountData[i].uDiscData.common.DiscFlg == 0 ){/* 新規精算(割引済みではない) */
				FLAPDT.flp_data[iti].bk_wari +=						/*								*/
					RecvNtnetDt.RData22.DiscountData[i].Discount;	/* 割引金額(中止,修正用)		*/
				FLAPDT.flp_data[iti].bk_wmai +=						/*								*/
					RecvNtnetDt.RData22.DiscountData[i].DiscCount;	/* 使用枚数(中止,修正用)		*/
			}														/*								*/
			break;													/*								*/
		case NTNET_SVS_T:											/* 割引種別 ｻｰﾋﾞｽ券(時間)		*/
		case NTNET_KAK_T:											/* 割引種別 店割引(時間)		*/
		case NTNET_TKAK_T:											/* 割引種別 多店舗(時間)		*/
		case NTNET_WRI_T:											/* 割引種別 割引券(時間)		*/
			if( RecvNtnetDt.RData22.DiscountData[i].uDiscData.common.DiscFlg == 0 ){/* 新規精算(割引済みではない) */
				FLAPDT.flp_data[iti].bk_time +=						/*								*/
					RecvNtnetDt.RData22.DiscountData[i].uDiscData.common.DiscInfo2;/* 割引時間数(中止,修正用) */
				FLAPDT.flp_data[iti].bk_wmai +=						/*								*/
					RecvNtnetDt.RData22.DiscountData[i].DiscCount;	/* 使用枚数(中止,修正用)		*/
			}														/*								*/
			break;													/*								*/
		}															/*								*/
		if(cyuushi_flg2){											/* 中止割引ﾃﾞｰﾀ格納				*/
			switch( RecvNtnetDt.RData22.DiscountData[i].DiscSyu ){	/* 割引種別						*/
			case NTNET_FRE:											/* 割引種別 回数券				*/
			case NTNET_PRI_W:										/* 割引種別 割引ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ	*/
				FLAPDT_SUB[flap_no].ppc_chusi_ryo +=
					RecvNtnetDt.RData22.DiscountData[i].Discount;	/* 割引金額(中止,修正用)		*/
				break;												/*								*/
			case NTNET_SVS_M:										/* 割引種別 ｻｰﾋﾞｽ券(料金)		*/
			case NTNET_SVS_T:										/* 割引種別 ｻｰﾋﾞｽ券(時間)		*/
				FLAPDT_SUB[flap_no].sev_tik[RecvNtnetDt.RData22.DiscountData[i].DiscNo-1] +=
						(uchar)RecvNtnetDt.RData22.DiscountData[i].DiscCount;	/* 使用枚数(中止,修正用)	*/
				break;												/*								*/
			case NTNET_KAK_M:										/* 割引種別 店割引(料金)		*/
			case NTNET_TKAK_M:										/* 割引種別 多店舗(料金)		*/
			case NTNET_WRI_M:										/* 割引種別 割引券(料金)		*/
			case NTNET_KAK_T:										/* 割引種別 店割引(時間)		*/
			case NTNET_TKAK_T:										/* 割引種別 多店舗(時間)		*/
			case NTNET_WRI_T:										/* 割引種別 割引券(時間)		*/
				for(wk_cnt=0;wk_cnt<5;wk_cnt++){					/* 同じ店No検索					*/
					if(FLAPDT_SUB[flap_no].kake_data[wk_cnt].mise_no == RecvNtnetDt.RData22.DiscountData[i].DiscNo){
						FLAPDT_SUB[flap_no].kake_data[wk_cnt].maisuu += (uchar)RecvNtnetDt.RData22.DiscountData[i].DiscCount;/*同じ店Noの時、枚数を格納する*/
						break;										/*								*/
					}												/*								*/
				}													/*								*/
				if(wk_cnt == 5){									/* 同じ店No無					*/
					for(wk_cnt=0;wk_cnt<5;wk_cnt++){				/*								*/
						if(FLAPDT_SUB[flap_no].kake_data[wk_cnt].mise_no == 0){
							FLAPDT_SUB[flap_no].kake_data[wk_cnt].mise_no = RecvNtnetDt.RData22.DiscountData[i].DiscNo;/*店舗No*/
							FLAPDT_SUB[flap_no].kake_data[wk_cnt].maisuu += (uchar)RecvNtnetDt.RData22.DiscountData[i].DiscCount;/*枚数*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
				}													/*								*/
				if(wk_cnt == 5){									/* 同じ店No無＋詳細中止空きｴﾘｱ無*/
					if(flap_no < 9){
						uc_buf_size = (uchar)(9 - flap_no);
						memset(&FLAPDT_SUB[flap_no],0,sizeof(flp_com_sub));			// 詳細中止ｴﾘｱ(精算完了)ｸﾘｱ
						memmove(&FLAPDT_SUB[flap_no],&FLAPDT_SUB[flap_no+1],sizeof(flp_com_sub)*uc_buf_size);
					}
					if(flap_no == 10){
						memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));							// 終端詳細中止ｴﾘｱｸﾘｱ
					}else{
						memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// 終端詳細中止ｴﾘｱｸﾘｱ
					}
					cyuushi_flg2 = 0;								/*	格納ｸﾞﾗｸﾞｸﾘｱ				*/
				}													/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 不正出庫・強制出庫時集計																	   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fus_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: short	: ret	1=強制出庫,	2=不正出庫											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	fus_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	ushort	iti;													/*								*/
	struct CAR_TIM wk_time1,wk_time2;
	uchar	i;
	uchar	buf_size;
	short	ret = 1;
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
																	/*								*/
	iti = fusei.fus_d[0].t_iti - 1;									/* 駐車位置(1～324)-1			*/
																	/*								*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
	memcpy( &wkloktotal,											/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
			&loktl.tloktl.loktldat[iti],							/*								*/
			sizeof( LOKTOTAL_DAT ) );								/*								*/
																	/*								*/
	ac_flg.cycl_fg = 32;											/* 32:Ｔ集計をﾜｰｸｴﾘｱへ転送完了	*/
																	/*								*/
																	/*								*/
	if( fusei.fus_d[0].kyousei == 1 || fusei.fus_d[0].kyousei == 11 )/* 強制操作で出庫？				*/
	{																/*								*/
		ts->Kyousei_out_Tcnt += 1;									/* 強制出庫回数					*/
		loktl.tloktl.loktldat[iti].Kyousei_out_cnt += 1;			/* 強制出庫回数					*/
																	/*								*/
		/*** 中止時に割引がある場合は駐車料金－割引料金を集計する ***/
		ts->Kyousei_out_Tryo += ryo_buf.dsp_ryo;					/* 強制出庫回数					*/
		loktl.tloktl.loktldat[iti].Kyousei_out_ryo +=				/* 強制出庫金額					*/
												ryo_buf.dsp_ryo;	/*								*/
	}else{															/* 不正(通常：0、修正：2、3)	*/
		ts->Husei_out_Tcnt += 1;									/* 不正出庫回数					*/
		loktl.tloktl.loktldat[iti].Husei_out_cnt += 1;				/* 不正出庫回数					*/
																	/*								*/
		/*** 中止時に割引がある場合は駐車料金－割引料金を集計する ***/
																	/*								*/
		// kyousei = 3は駐車料金0円
		if( fusei.fus_d[0].kyousei != 3 ){							/* 通常、修正で料金あり			*/
			ts->Husei_out_Tryo += ryo_buf.dsp_ryo;					/* 不正出庫金額					*/
			loktl.tloktl.loktldat[iti].Husei_out_ryo +=				/* 不正出庫金額					*/
												ryo_buf.dsp_ryo;	/*								*/
		}
		ret = 2;
	}																/*								*/
																	/*								*/
	if( DoBunruiSyu(ryo_buf.syubet) ){								/* 分類集計する（種別毎ﾁｪｯｸ）	*/

		// kyousei = 3は駐車料金0円
		if( fusei.fus_d[0].kyousei == 3 ){							/* 修正精算での不正(料金0円)	*/
			bunrui( 0, 0, ryo_buf.syubet );							/* 分類集計加算					*/
		}else{														/* 通常不正、修正で料金あり		*/
			if( fusei.fus_d[0].kyousei == 1 ){						// 強制出庫
				if( prm_get(COM_PRM, S_BUN, 59, 1, 1) == 0 ){		// 分類集計加算する
					bunrui( 0, ryo_buf.dsp_ryo , ryo_buf.syubet );	// 分類集計加算
				}													// 
			}else{													// 不正出庫
				if( prm_get(COM_PRM, S_BUN, 59, 1, 2) == 0 ){		// 分類集計加算する
					bunrui( 0, ryo_buf.dsp_ryo , ryo_buf.syubet );	// 分類集計加算
				}													// 
			}														// 
		}
	}
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 33;											/* 33:Ｔｴﾘｱ集計加算完了			*/
																	/*								*/
	Log_regist( LOG_ABNORMAL );										/* 不正・強制出庫情報登録		*/
																	/*								*/
	ac_flg.cycl_fg = 35;											/* 35:不正・強制出庫情報登録完了*/
																	/*								*/
	memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );			/* 不正ﾃﾞｰﾀをﾜｰｸｴﾘｱへ			*/
																	/*								*/
	ac_flg.cycl_fg = 36;											/* 36:不正ﾃﾞｰﾀをﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
	memcpy( &fusei, &wkfus.fus_d[1],								/* ﾃﾞｰﾀｼﾌﾄ						*/
			sizeof( struct FUSEI_D )*(LOCK_MAX-1) );				/*								*/
	fusei.kensuu -= 1;												/* 件数－１						*/
																	/*								*/
	ac_flg.cycl_fg = 37;											/* 37:ﾃﾞｰﾀｼﾌﾄ・件数-1完了		*/
																	/*								*/
	memset( &fusei.fus_d[LOCK_MAX-1], 0, sizeof( struct FUSEI_D ) );/*								*/
																	/*								*/
	ac_flg.cycl_fg = 38;											/* 38:6件目ｸﾘｱ完了				*/
																	/*								*/
	memset( &syusei[iti], 0, sizeof(struct SYUSEI));				/* 修正精算用ﾃﾞｰﾀｸﾘｱ			*/
	for(i = 0 ; i < 10 ; i++){
		if(FLAPDT_SUB[i].WPlace == (ulong)( LockInfo[iti].area*10000L +
					  LockInfo[iti].posi )){		//駐車位置が同じ
			/*入庫時間*/
			wk_time1.year = wkfus.fus_d[0].iyear ;
			wk_time1.mon = wkfus.fus_d[0].imont ;
			wk_time1.day = wkfus.fus_d[0].idate ;
			wk_time1.hour = wkfus.fus_d[0].ihour ;
			wk_time1.min = wkfus.fus_d[0].iminu ;
			/*詳細中止入庫ﾃﾞｰﾀ*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;
			
			if(0 == ec64(&wk_time1,&wk_time2)){		//入庫時刻が同じ
				if(i < 9){
					buf_size = (uchar)(9 - i);
					memset(&FLAPDT_SUB[i],0,sizeof(flp_com_sub));			// 詳細中止ｴﾘｱ(精算完了)ｸﾘｱ
					memmove(&FLAPDT_SUB[i],&FLAPDT_SUB[i+1],sizeof(flp_com_sub)*buf_size);
				}
				memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// 終端詳細中止ｴﾘｱｸﾘｱ
				break;
			}
		}
	}
	return ret;														/*								*/
}																	/*								*/
																	/*								*/
#if SYUSEI_PAYMENT
//---------------------------------------------------------------------------------------------------
// 新修正精算機能追加(新修正精算ではこの関数は使わない)
//---------------------------------------------------------------------------------------------------
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 修正精算時集計																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: syu_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	syu_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	LOKTOTAL_DAT	*lkts;											/*								*/
	ushort	itis, itiw;												/*								*/
	short	i;														/*								*/
	ulong	dat1, dat2;												/*								*/
	ulong	wk_Electron_ryo;										/* 電子ﾏﾈｰ精算金額セーブ		*/
	wari_tiket	wari_dt;
																	/*								*/
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
																	/*								*/
	itis = OPECTL.Pr_LokNo - 1;										/* 正しい駐車位置番号			*/
	itiw = OPECTL.MPr_LokNo - 1;									/* 間違い駐車位置番号			*/
	lkts = &loktl.tloktl.loktldat[itis];							/*								*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* Ｔ集計をﾜｰｸｴﾘｱへ（停電対策）	*/
	memcpy( &wkloktotal, lkts, sizeof( LOKTOTAL_DAT ) );			/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
	dat1 = 0L;														/*								*/
																	/*								*/
	ac_flg.cycl_fg = 74;											/* 74							*/
																	/*								*/
																	/*								*/
	if( ryo_buf.nyukin > ryo_buf.turisen )							/*								*/
	{																/*								*/
		dat1 = ryo_buf.nyukin - ryo_buf.turisen;					/* 入金額－釣り銭額				*/
		ts->Genuri_Tryo += dat1;									/* 総現金売り上げ				*/
		ts->Uri_Tryo += dat1;										/* 総売り上げ					*/
	} else {														/*								*/
		if( ryo_buf.nyukin == 0 )									/*								*/
		{															/*								*/
			ts->Syuusei_seisan_Tryo += ryo_buf.turisen;				/* 修正精算払戻金額				*/
		}															/*								*/
	}																/*								*/
	if( ryo_buf.ryo_flg < 2 )										/* 定期使用無し精算?			*/
	{																/*								*/
		syusei[itis].ryo = ryo_buf.tyu_ryo;							/* 修正用駐車料金ｾｯﾄ			*/
		if( DoBunruiSyu(ryo_buf.syubet) ){							/* 分類集計する（種別毎ﾁｪｯｸ）	*/
		if( ryo_buf.tyu_ryo > ryo_buf.mis_tyu )						/* 駐車料金(正)＞駐車料金(違)?	*/
		{															/*								*/
			bunrui( 0, ryo_buf.tyu_ryo - ryo_buf.mis_tyu , ryo_buf.syubet );/* 分類集計加算			*/
		} else {													/*								*/
			bunrui( 0, 0 ,ryo_buf.syubet );							/*								*/
		}															/*								*/
		}
	} else {														/*								*/
		syusei[itis].ryo = ryo_buf.tei_ryo;							/* 修正用駐車料金ｾｯﾄ			*/
		if(	( 1 == (uchar)prm_get(COM_PRM, S_BUN, 54, 1, 3) )		/* 定期券精算時に分類集計への加算する */
				&&													/*   且つ						*/
			( DoBunruiSyu(ryo_buf.syubet) ) ){						/* 分類集計する（種別毎ﾁｪｯｸ）	*/
		if( ryo_buf.tei_ryo > ryo_buf.mis_tyu )						/* 駐車料金(正)＞駐車料金(違)?	*/
		{															/*								*/
			bunrui( 0, ryo_buf.tei_ryo - ryo_buf.mis_tyu ,ryo_buf.syubet );	/* 分類集計加算			*/
		} else {													/*								*/
			bunrui( 0, 0 , ryo_buf.syubet );						/*								*/
		}															/*								*/
		}
	}																/*								*/
	lkts->Seisan_cnt += 1;											/* 駐車位置番号別集計(精算台数)	*/
	syusei[itis].sei = 0;											/* 過去精算有りを無しにする		*/
	syusei[itis].gen = dat1;										/* 現金領収額					*/
	ts->Rsei_cnt[ryo_buf.syubet] += 1L;								/* 種別毎精算回数				*/
	ts->Seisan_Tcnt += 1L;											/* 総精算回数					*/
	ts->Syuusei_seisan_Tcnt += 1L;									/* 修正精算回数					*/
																	/*								*/
	ts->Turi_modosi_ryo += ryo_buf.turisen;							/* 釣銭払戻額					*/
																	/*								*/
	if( ryo_buf.fusoku != 0 )										/* 支払い不足額有り?			*/
	{																/*								*/
		ts->Harai_husoku_cnt += 1L;									/* 支払い不足回数				*/
		ts->Harai_husoku_ryo += ryo_buf.fusoku;						/* 支払い不足額					*/
	}																/*								*/
																	/*								*/
	if(( CPrmSS[S_CAL][19] != 0 )&&									/* 消費税有り?					*/
	   ( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2 )&&					/* 外税?						*/
	   ( dat1 >= ryo_buf.tax ))										/* 税金対象額＞＝税金?			*/
	{																/*								*/
		dat2 = 0;													/*								*/
		if( ryo_buf.tax > syusei[itiw].tax )						/* 正しい精算税額＞間違い精算税額*/
		{															/*								*/
			dat2 = ( ryo_buf.tax - syusei[itiw].tax );				/* 消費税の差額を加算			*/
			ts->Tax_Tryo += dat2;									/* 消費税(内税・外税)			*/
		}															/*								*/
		ts->Rsei_ryo[ryo_buf.syubet] += (dat1 - dat2);				/* 種別毎現金売り上げ			*/
		lkts->Genuri_ryo += (dat1 - dat2);							/* 駐車位置番号別集計(現金売り上げ)	*/
	} else {														/*								*/
		ts->Rsei_ryo[ryo_buf.syubet] += dat1;						/* 種別毎現金売り上げ			*/
		lkts->Genuri_ryo += dat1;									/* 駐車位置番号別集計(現金売り上げ)	*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.svs_tim )											/* ｻｰﾋﾞｽﾀｲﾑ内精算有り			*/
	{																/*								*/
		ts->In_svst_seisan += 1L;									/* ｻｰﾋﾞｽﾀｲﾑ内精算回数+1			*/
	}																/*								*/
																	/*								*/
	if( PayData.credit.pay_ryo )									/* ｸﾚｼﾞｯﾄｶｰﾄﾞ使用				*/
	{																/*								*/
		ts->Ccrd_sei_cnt += 1;										/* ｸﾚｼﾞｯﾄｶｰﾄﾞ精算＜回数＞		*/
		ts->Ccrd_sei_ryo += PayData.credit.pay_ryo;					/* ｸﾚｼﾞｯﾄｶｰﾄﾞ精算＜金額＞		*/
																	/*								*/
		dat1 = prm_get( COM_PRM,S_TOT,6,1,6 );						/* ｸﾚｼﾞｯﾄｶｰﾄﾞ総売上額/総掛売額加算設定	*/
		if( dat1 == 1L )											/* 総売上額に加算する			*/
		{															/*								*/
			ts->Uri_Tryo += PayData.credit.pay_ryo;					/* 総売上額						*/
		}															/*								*/
		else if( dat1 == 2L )										/* 総掛売額に加算する			*/
		{															/*								*/
			ts->Kakeuri_Tryo += PayData.credit.pay_ryo;				/* 総掛売額						*/
		}															/*								*/
		else if( dat1 == 3L )										/* 総売上額/総掛売額に加算する	*/
		{															/*								*/
			ts->Uri_Tryo += PayData.credit.pay_ryo;					/* 総売上額						*/
			ts->Kakeuri_Tryo += PayData.credit.pay_ryo;				/* 総掛売額						*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	wk_media_Type = Ope_Disp_Media_Getsub(1);						/* 電子ﾏﾈｰ媒体種別 取得			*/
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind )){
		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );					/* Suica総売上額/総掛売額加算設定	*/
		wk_Electron_ryo = PayData.Electron_data.Suica.pay_ryo;		/* 電子ﾏﾈｰ精算金額セーブ		*/
		if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {			/* Ｓｕｉｃａ					*/
			ts->Electron_sei_cnt += 1L;								/* Suica使用＜回数＞			*/
			ts->Electron_sei_ryo += wk_Electron_ryo;				/* Suica使用＜金額＞			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		/* ＰＡＳＭＯ					*/
			ts->Electron_psm_cnt += 1L;								/* PASMO使用＜回数＞			*/
			ts->Electron_psm_ryo += wk_Electron_ryo;				/* PASMO使用＜金額＞			*/
		}
	} else if( PayData.Electron_data.Suica.e_pay_kind == EDY_USED ){/* Edyﾘﾀﾞｰ領収ﾃﾞｰﾀあり			*/
		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 3 );					/* Edy総売上額/総掛売額加算設定	*/
		if( EDY_TEST_MODE ){										// ﾃｽﾄﾓｰﾄﾞの場合
			dat1 = 0L;												// 総売上額/総掛売額に加算しない
		}
		else{
			wk_Electron_ryo = PayData.Electron_data.Edy.pay_ryo;	/* 電子ﾏﾈｰ精算金額セーブ		*/
			if(wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY) {			/* Ｅｄｙ						*/
				ts->Electron_edy_cnt += 1L;							/* Ｅｄｙ使用＜回数＞			*/
				ts->Electron_edy_ryo += wk_Electron_ryo;			/* Ｅｄｙ使用＜金額＞			*/
			}
		}
	}
	if( PayData.Electron_data.Suica.e_pay_kind != 0 ) {				/* 電子ﾏﾈｰ(Suica/Edy)領収ﾃﾞｰﾀあり*/
		if( dat1 == 1L ) {											/* 総売上額に加算する			*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* 総売上額						*/
		} else if( dat1 == 2L ) {									/* 総掛売額に加算する			*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* 総掛売額						*/
		} else if( dat1 == 3L ) {									/* 総売上額/総掛売額に加算する	*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* 総売上額						*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* 総掛売額						*/
		}															/*								*/
	}

	if( CPrmSS[S_PRP][1] )											/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ,回数券使用する設定	*/
	{																/*								*/
		if( CPrmSS[S_PRP][1] == 1 )									/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用する設定		*/
		{															/*								*/
			dat1 = prm_get( COM_PRM,S_TOT,5,1,3 );					/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ総売上額/総掛売額加算設定	*/
		}															/*								*/
		else														/* 回数券使用する設定			*/
		{															/*								*/
			dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );					/* 回数券総売上額/総掛売額加算設定	*/
		}															/*								*/
		j = 0;														/*								*/
		for( i = 0; i < WTIK_USEMAX; i++ ){							/*								*/
			disc_wari_conv( &PayData.DiscountData[i], &wari_dt );	/*								*/
			if( wari_dt.tik_syu == PREPAID ){						/* ﾃﾞｰﾀあり						*/
				dat2 = wari_dt.pkno_syu;							/*								*/
				if( CPrmSS[S_PRP][1] == 1 ){						/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用する設定		*/
					ts->Pcrd_use_cnt[dat2] += ryo_buf.pri_mai[j];	/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜回数＞(基本,拡張1,2,3)	*/
																	/* 2枚以上使用時は最後に使われた種別に合計加算する	*/
					ts->Pcrd_use_ryo[dat2] += wari_dt.ryokin;		/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜金額＞(基本,拡張1,2,3)	*/
				}													/*								*/
				else												/* 回数券使用する設定			*/
				{													/*								*/
					ts->Ktik_use_cnt[dat2] += 1L;					/* 回数券使用＜回数＞(基本,拡張1,2,3)	*/
					ts->Ktik_use_ryo[dat2] += wari_dt.ryokin;		/* 回数券使用＜金額＞(基本,拡張1,2,3)	*/
				}													/*								*/
																	/*								*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				j++;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ )								/*								*/
	{																/*								*/
		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );		/*								*/
		switch( wari_dt.tik_syu )									/*								*/
		{															/*								*/
			case SERVICE:											/* ｻｰﾋﾞｽ券						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* ｻｰﾋﾞｽ券A,B,C					*/
																	/*								*/
				ts->Stik_use_cnt[dat1][dat2] += wari_dt.maisuu;		/* ｻｰﾋﾞｽ券種別毎使用＜枚数＞(基本,拡張1,2,3)(ｻｰﾋﾞｽ券A,B,C)	*/
				ts->Stik_use_ryo[dat1][dat2] += wari_dt.ryokin;		/* ｻｰﾋﾞｽ券種別毎使用＜金額＞(基本,拡張1,2,3)(ｻｰﾋﾞｽ券A,B,C)	*/
																	/*								*/
				if( CPrmSS[S_SER][1+3*dat2] == 4 )					/* ｻｰﾋﾞｽ券全額割引設定?			*/
				{													/*								*/
					syusei[itis].infofg |= SSS_ZENWARI;				/* 修正用全額割引ﾌﾗｸﾞｾｯﾄ		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,1 );				/* ｻｰﾋﾞｽ券総売上額/総掛売額加算設定	*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			case KAKEURI:											/* 掛売券						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* 店№1～999(0～998)			*/
																	/*								*/
				ts->Mno_use_Tcnt += wari_dt.maisuu;					/* 店割引総ﾄｰﾀﾙ＜回数＞			*/
				ts->Mno_use_Tryo += wari_dt.ryokin;					/* 店割引総ﾄｰﾀﾙ＜金額＞			*/
																	/*								*/
				ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;			/* 店№割引ﾄｰﾀﾙ＜回数＞(基本,拡張1,2,3)	*/
				ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;			/* 店№割引ﾄｰﾀﾙ＜金額＞(基本,拡張1,2,3)	*/
																	/*								*/
				if( dat1 == KIHON_PKNO || dat1 == KAKUCHOU_1 )		/* 基本 or 拡張1				*/
				{													/*								*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
//					if( dat2 < 100 )								/* 店№1～100					*/
					if( dat2 < MISE_NO_CNT )						/* 店№1～100					*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* 店№1～100ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* 店№1～100ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
						ts->Mno_use_cnt4[dat1][dat2] +=				/* 店№毎割引＜回数＞(基本,拡張1)(店№1～100)	*/
												wari_dt.maisuu;		/*								*/
						ts->Mno_use_ryo4[dat1][dat2] +=				/* 店№毎割引＜金額＞(基本,拡張1)(店№1～100)	*/
												wari_dt.ryokin;		/*								*/
					}												/*								*/
					else											/* 店№101～999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* 店№101～999ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* 店№101～999ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
					}												/*								*/
				}													/*								*/
				else												/*								*/
				{													/*								*/
					if(( dat1 == KAKUCHOU_2 )&&						/* 拡張2						*/
					   ( prm_get( COM_PRM,S_TOT,16,1,3 ) == 1 ))	/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
					else if(( dat1 == KAKUCHOU_3 )&&				/* 拡張3						*/
							( prm_get( COM_PRM,S_TOT,16,1,4 ) == 1 ))/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
//					if( dat2 < 100 )								/* 店№1～100					*/
					if( dat2 < MISE_NO_CNT )						/* 店№1～100					*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* 店№1～100ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* 店№1～100ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
						if( dat1 == KAKUCHOU_1 )					/*								*/
						{											/*								*/
							ts->Mno_use_cnt4[dat1][dat2] +=			/* 店№毎割引＜回数＞(基本,拡張1)(店№1～100)	*/
												wari_dt.maisuu;		/*								*/
							ts->Mno_use_ryo4[dat1][dat2] +=			/* 店№毎割引＜金額＞(基本,拡張1)(店№1～100)	*/
												wari_dt.ryokin;		/*								*/
						}											/*								*/
					}												/*								*/
					else											/* 店№101～999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* 店№101～999ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* 店№101～999ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
					}												/*								*/
				}													/*								*/
																	/*								*/
				if( CPrmSS[S_STO][1+3*dat2] == 4 &&					/* 店割引全額割引設定?			*/
					ryo_buf.pkiti != 0xffff )						/* ﾏﾙﾁ精算以外?					*/
				{													/*								*/
					syusei[itis].infofg |= SSS_ZENWARI;				/* 修正用全額割引ﾌﾗｸﾞｾｯﾄ		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* 店割引総売上額/総掛売額加算設定	*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				break;												/*								*/
			case KAISUU:											/* 回数券						*/
				if( CPrmSS[S_PRP][1] != 2 )							/* 回数券を使用する設定ではない	*/
					break;

				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
																	/*								*/
				ts->Ktik_use_cnt[dat1] += wari_dt.maisuu;			/* 使用＜枚数＞(基本,拡張1,2,3) */
				ts->Ktik_use_ryo[dat1] += wari_dt.ryokin;			/* 使用＜金額＞(基本,拡張1,2,3)	*/

				dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );				/* 回数券総売上額/総掛売額加算設定	*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				break;												/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
																	/*								*/
	if( ryo_buf.ryo_flg >= 2 )										/* 定期使用?					*/
	{																/*								*/
		dat1 = (ulong)PayData.teiki.pkno_syu;						/* 駐車場№の種類				*/
		ts->Teiki_use_cnt[dat1][PayData.teiki.syu-1] += 1L;			/* 定期使用回数+1				*/
		syusei[itis].tei_syu = PayData.teiki.syu;					/* 定期種別						*/
		syusei[itis].tei_id = PayData.teiki.id;						/* 定期券id(1～12000)			*/
		syusei[itis].tei_sd = dnrmlzm( (short)PayData.teiki.s_year,	/* 有効開始年月日ﾉｰﾏﾗｲｽﾞ		*/
									   (short)PayData.teiki.s_mon,	/*								*/
									   (short)PayData.teiki.s_day );/*								*/
		syusei[itis].tei_ed = dnrmlzm( (short)PayData.teiki.e_year,	/* 有効終了年月日ﾉｰﾏﾗｲｽﾞ		*/
									   (short)PayData.teiki.e_mon,	/*								*/
									   (short)PayData.teiki.e_day );/*								*/

		if(	( 1 == (uchar)prm_get(COM_PRM, S_BUN, 54, 1, 3) )		/* 定期券精算時に分類集計への加算する */
				&&													/*   且つ						*/
			( DoBunruiSyu(ryo_buf.syubet) ) ){						/* 分類集計する（種別毎ﾁｪｯｸ）	*/

			bunrui( 0, ryo_buf.tei_ryo , ryo_buf.syubet );			/* 分類集計加算					*/
		}
	}																/*								*/
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 75;											/* 75							*/
																	/*								*/
}																	/*								*/
#endif
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 精算途中(精算完了前)停電復帰集計															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: toty_syu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-11-15																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	toty_syu( void )											/* 精算途中(精算完了前)停電復帰	*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	ulong	dat1, dat2;												/*								*/
	ushort	iti;													/*								*/
	short	i;														/*								*/
	uchar	f_SameData;												/* 1=定期中止ﾃｰﾌﾞﾙ内に目的ﾃﾞｰﾀあり */
	uchar	uc_update = PayData.teiki.update_mon;					/* 更新月格納					*/
	ushort	Flp_sub_no;
	uchar	Flp_sub_ok;
	uchar	wk_cnt=0;
	uchar	uc_buf_size;
	ulong	wk_Electron_ryo=0;										/* 電子ﾏﾈｰ精算金額セーブ		*/
	wari_tiket	wari_dt;
	short	j;
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	DATE_SYOUKEI	*date_ts;
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
	uchar	taxableAdd_set;											/*	課税対象売上加算設定		*/
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH321800(S) hosoda ICクレジット対応
//	struct clk_rec wrk_clk_rec;		/* 集計日時ノーマライズ用 */
//	date_time_rec wrk_date;			/* 集計日時ノーマライズ用 */
//	int		ec_kind;
//	int		syu_idx;				/* 日毎集計のインデックス */
//	ulong	sei_date;				/* 精算日時 */
//	ulong	sy1_date;				/* 集計日時 */
//	memset(&wrk_clk_rec, 0x0, sizeof(wrk_clk_rec));
//// MH321800(E) hosoda ICクレジット対応
// MH810105 GG119202(E) 処理未了取引集計仕様改善

	dat1 = 0;
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	date_ts = &Date_Syoukei;
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	iti = ryo_buf.pkiti - 1;										/* 駐車位置						*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* Ｔ集計をﾜｰｸｴﾘｱへ（停電対策）	*/
	DailyAggregateDataBKorRES( 0 );									/* 日毎集計エリアのバックアップ	*/
// MH322914 (s) kasiyama 2016/07/13 復電領収証の払戻額修正[共通バグNo.1232](MH341106)
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	date_uriage_syoukei_judge();									// 日付切替基準 総売上、現金総売上小計の時刻更新判定（リアルタイム情報用）
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	if( ryo_buf.nyukin != 0L )										/* 入金有り？					*/
	{																/*								*/
		ryo_buf.fusoku = ryo_buf.nyukin;							/* 入金額を不足額				*/
		ryo_buf.turisen = ryo_buf.nyukin;							/* 入金額を釣り銭				*/
	}																/*								*/
// MH322914 (e) kasiyama 2016/07/13 復電領収証の払戻額修正[共通バグNo.1232](MH341106)
																	/*								*/
	PayData_set( 1, 1 );											/* 1精算情報ｾｯﾄ					*/
																	/*								*/
	ac_flg.cycl_fg = 51;											/* 51:Ｔ集計をﾜｰｸｴﾘｱへ転送完了	*/
																	/*								*/
																	/*								*/
	if( ryo_buf.nyukin != 0L )										/* 入金有り？					*/
	{																/*								*/
		ts->Harai_husoku_cnt += 1L;									/* 支払い不足回数				*/
		ts->Harai_husoku_ryo += ryo_buf.nyukin;						/* 支払い不足額に入金額加算		*/
																	/*								*/
// MH322914 (s) kasiyama 2016/07/13 復電領収証の払戻額修正[共通バグNo.1232](MH341106)
//		ryo_buf.fusoku = ryo_buf.nyukin;							/* 入金額を不足額				*/
//		ryo_buf.turisen = ryo_buf.nyukin;							/* 入金額を釣り銭				*/
// MH322914 (e) kasiyama 2016/07/13 復電領収証の払戻額修正[共通バグNo.1232](MH341106)
	}																/*								*/
																	/*								*/
	ts->Seisan_chusi_cnt += 1L;										/* 精算中止回数					*/
	ts->Seisan_chusi_ryo += ryo_buf.nyukin;							/* 中止時現金投入額				*/
																	/*								*/
	if(Flap_Sub_Flg == 1){									// 最精算＋Bエリア有
		Flp_sub_ok = 1;
		Flp_sub_no = Flap_Sub_Num;							// 作業中詳細ｴﾘｱ格納
	}else if((Flap_Sub_Flg == 2) || (Flap_Sub_Num == 10)){	// 再精算＋Bエリア無、マルチ精算(Bｴﾘｱ無)の時
		Flp_sub_ok = 0;
	}else{														// 再精算無、初精算中止
		for(i = 0 ; i < 10 ; i++){
			if(FLAPDT_SUB[i].WPlace == 0){
				break;
			}
		}
		if(i < 10){
			Flp_sub_ok = 1;
			Flp_sub_no = i;
		}else{
			memmove(&FLAPDT_SUB[0],&FLAPDT_SUB[1],sizeof(flp_com_sub)*9);			//前に１個シフト
			memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));	//ｸﾘｱ
			Flp_sub_ok = 1;
			Flp_sub_no = 9;
		}
	}

	wk_media_Type = Ope_Disp_Media_Getsub(1);						/* 電子ﾏﾈｰ媒体種別 取得			*/
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH321800(S) hosoda ICクレジット対応
//// ※電子マネー使用時は中止がないので、この処理が動くことはないはずです。（復電でも完了になるはず。）
//// MH810103 GG119202(S) 決済精算中止印字処理修正
////	if (EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) != 0 ||
////		PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED ) {
//	if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) != 0 ||
//		 PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
//		(PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0)) {	// みなし決済の復電時は集計しない(精算中止扱いのため)
//// MH810103 GG119202(E) 決済精算中止印字処理修正
//		// 決済リーダが使用されていたら
//		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );							/* 電子マネー総売上額/総掛売額加算設定	*/
//		wk_Electron_ryo = PayData.Electron_data.Ec.pay_ryo;					/* 電子マネー精算金額セーブ				*/
//		ec_kind = PayData.Electron_data.Ec.e_pay_kind;
//		// 集計は既存エリアがあればそれを使用する。
//		switch(ec_kind) {
//		case EC_EDY_USED:
//			ts->Electron_edy_cnt += 1L;										// 精算＜件数＞
//			ts->Electron_edy_ryo += wk_Electron_ryo;						//     ＜金額＞
//			break;
//		case EC_NANACO_USED:
//			ts->nanaco_sei_cnt += 1L;										// 精算＜件数＞
//			ts->nanaco_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
//			break;
//		case EC_WAON_USED:
//			ts->waon_sei_cnt += 1L;											// 精算＜件数＞
//			ts->waon_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
//			break;
//		case EC_SAPICA_USED:
//			ts->sapica_sei_cnt += 1L;										// 精算＜件数＞
//			ts->sapica_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
//			break;
//		case EC_KOUTSUU_USED:
//			ts->koutsuu_sei_cnt += 1L;										// 精算＜件数＞
//			ts->koutsuu_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
//			break;
//		case EC_ID_USED:
//			ts->id_sei_cnt += 1L;											// 精算＜件数＞
//			ts->id_sei_ryo += wk_Electron_ryo;								//     ＜金額＞
//			break;
//		case EC_QUIC_PAY_USED:
//			ts->quicpay_sei_cnt += 1L;										// 精算＜件数＞
//			ts->quicpay_sei_ryo += wk_Electron_ryo;							//     ＜金額＞
//			break;
//		case EC_CREDIT_USED:
//			// クレジットのT集計は既存処理にて処理
//			wk_Electron_ryo = PayData.credit.pay_ryo;
//			break;
//		default:
//			break;		// ありえない
//		}
//		// 最新の通知日時から集計日時を算出
//		memcpy(&wrk_date, &Syuukei_sp.ec_inf.NowTime, sizeof(date_time_rec));
//		wrk_date.Hour = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 3);			/* 時は共通パラメータから取得 */
//		wrk_date.Min = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 1);				/* 分は共通パラメータから取得 */
//		sy1_date = Nrm_YMDHM(&wrk_date);									/* 集計日時をノーマライズ */
//
//		// 精算データから（決済結果データで受信した）精算日時を算出
//		c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &wrk_clk_rec);
//		memcpy(&wrk_date, &wrk_clk_rec, sizeof(date_time_rec));
//		sei_date = Nrm_YMDHM(&wrk_date);									/* 精算日時をノーマライズ */
//
//		ec_kind -= EC_EDY_USED;
//		if(sei_date >= sy1_date && sei_date < (sy1_date + 0x10000)) {
//			// 精算日時が当日の集計日時範囲
//			Syuukei_sp.ec_inf.now.cnt[ec_kind] += 1L;							/* 使用回数				*/
//			Syuukei_sp.ec_inf.now.ryo[ec_kind] += wk_Electron_ryo;				/* 使用金額				*/
//		}
//		else if(sei_date >= (sy1_date + 0x10000) && sei_date < (sy1_date + 0x20000)) {
//			// 精算日時が翌日の集計日時範囲
//			Syuukei_sp.ec_inf.next.cnt[ec_kind] += 1L;							/* 使用回数				*/
//			Syuukei_sp.ec_inf.next.ryo[ec_kind] += wk_Electron_ryo;				/* 使用金額				*/
//		}
//		else if(sei_date >= (sy1_date - 0x10000) && sei_date < sy1_date) {
//			// 精算日時が前日の集計日時範囲
//			if(Syuukei_sp.ec_inf.cnt == 1) {
//				// 前日の集計がないため作成する
//				UnNrm_YMDHM(&wrk_date, (sy1_date - 0x10000));
//				memcpy(&Syuukei_sp.ec_inf.bun[Syuukei_sp.ec_inf.ptr].SyuTime, &wrk_date, sizeof(date_time_rec));
//				Syuukei_sp.ec_inf.ptr++;
//				Syuukei_sp.ec_inf.cnt++;
//			}
//			syu_idx = (Syuukei_sp.ec_inf.ptr == 0) ? (SYUUKEI_DAY_EC-1) : (Syuukei_sp.ec_inf.ptr-1);
//			Syuukei_sp.ec_inf.bun[syu_idx].cnt[ec_kind] += 1L;					/* 使用回数				*/
//			Syuukei_sp.ec_inf.bun[syu_idx].ryo[ec_kind] += wk_Electron_ryo;		/* 使用金額				*/
//		} else {
//			// 精算日時が翌日から前日の範囲外の場合はアラームを登録して当日に加算
//			Syuukei_sp.ec_inf.now.cnt[ec_kind] += 1L;							/* 使用回数				*/
//			Syuukei_sp.ec_inf.now.ryo[ec_kind] += wk_Electron_ryo;				/* 使用金額				*/
//			Syuukei_sp.ec_inf.now.unknownTimeCnt++;
//
//			// アラーム登録(決済リーダの決済時刻と精算機の時刻に1日以上のずれがある)
//			alm_chk( ALMMDL_SUB, ALARM_EC_SETTLEMENT_TIME_GAP, 2 );
//		}
//	}
//	else
//// MH321800(E) hosoda ICクレジット対応
// MH810105 GG119202(E) 処理未了取引集計仕様改善
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ){
		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );					/* Suica総売上額/総掛売額加算設定	*/
		wk_Electron_ryo = PayData.Electron_data.Suica.pay_ryo;		/* 電子ﾏﾈｰ精算金額セーブ		*/
		Syuukei_sp.sca_inf.now.cnt += 1L;							/* Suica使用回数				*/
		Syuukei_sp.sca_inf.now.ryo += wk_Electron_ryo;				/* Suica使用金額				*/
		if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {			/* Ｓｕｉｃａ					*/
			ts->Electron_sei_cnt += 1L;								/* Suica使用＜回数＞			*/
			ts->Electron_sei_ryo += wk_Electron_ryo;				/* Suica使用＜金額＞			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		/* ＰＡＳＭＯ					*/
			ts->Electron_psm_cnt += 1L;								/* PASMO使用＜回数＞			*/
			ts->Electron_psm_ryo += wk_Electron_ryo;				/* PASMO使用＜金額＞			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICOCA) {		/* ＩＣＯＣＡ					*/
			ts->Electron_ico_cnt += 1L;								/* ICOCA使用＜回数＞			*/
			ts->Electron_ico_ryo += wk_Electron_ryo;				/* ICOCA使用＜金額＞			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICCARD) {	/* ＩＣ-ＣＡＲＤ				*/
			ts->Electron_icd_cnt += 1L;								/* IC-CARD使用＜回数＞			*/
			ts->Electron_icd_ryo += wk_Electron_ryo;				/* IC-CARD使用＜金額＞			*/
		}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//	} else if( PayData.Electron_data.Suica.e_pay_kind == EDY_USED ){/* Edyﾘﾀﾞｰ領収ﾃﾞｰﾀあり			*/
//		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 3 );					/* Edy総売上額/総掛売額加算設定	*/
//		if( EDY_TEST_MODE ){										// ﾃｽﾄﾓｰﾄﾞの場合
//			dat1 = 0L;												// 総売上額/総掛売額に加算しない
//		}
//		else{
//			wk_Electron_ryo = PayData.Electron_data.Edy.pay_ryo;	/* 電子ﾏﾈｰ精算金額セーブ		*/
//			if(wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY) {			/* Ｅｄｙ						*/
//				ts->Electron_edy_cnt += 1L;							/* Ｅｄｙ使用＜回数＞			*/
//				ts->Electron_edy_ryo += wk_Electron_ryo;			/* Ｅｄｙ使用＜金額＞			*/
//				Syuukei_sp.edy_inf.now.cnt += 1L;					/* PASMO使用回数				*/
//				Syuukei_sp.edy_inf.now.ryo += wk_Electron_ryo;		/* PASMO使用金額				*/
//			}
//		}
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	}
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
	else if (EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
			PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) {
		// 売上額に加算しない
		wk_Electron_ryo = 0;

		if (PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0) {
			// 決済中に障害発生、または、未了残高照会タイムアウトした
			// 処理未了取引（支払不明）集計
			ts->miryo_unknown_cnt += 1L;
			ts->miryo_unknown_ryo += EcAlarm.Ec_Res.settlement_data;

			// 日毎集計は当日としてカウントする
			Syuukei_sp.ec_inf.now.sp_miryo_unknown_cnt += 1L;
			Syuukei_sp.ec_inf.now.sp_miryo_unknown_ryo += EcAlarm.Ec_Res.settlement_data;
		}
	}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH321800(S) Y.Tanizaki ICクレジット対応
////	if( PayData.Electron_data.Suica.e_pay_kind != 0 ) {				/* 電子ﾏﾈｰ(Suica/Edy)領収ﾃﾞｰﾀあり*/
//	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ||	// SX-20による電子マネー決済
//// MH810103 GG119202(S) 決済精算中止印字処理修正
////		EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {			// 決済リーダによる電子マネー決済
//		EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) &&			// 決済リーダによる電子マネー決済
//		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){	// みなし決済の復電時は集計しない(精算中止扱いのため)
//// MH810103 GG119202(E) 決済精算中止印字処理修正
//// MH321800(E) Y.Tanizaki ICクレジット対応
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ){	// SX-20による電子マネー決済
// MH810105 GG119202(E) 処理未了取引集計仕様改善
		if( dat1 == 1L ) {											/* 総売上額に加算する			*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
			if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
				if(date_uriage_cmp_paymentdate() == 0){// 加算可能
					date_ts->Uri_Tryo += wk_Electron_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
		} else if( dat1 == 2L ) {									/* 総掛売額に加算する			*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* 総掛売額						*/
		} else if( dat1 == 3L ) {									/* 総売上額/総掛売額に加算する	*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* 総売上額						*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
			if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
				if(date_uriage_cmp_paymentdate() == 0){// 加算可能
					date_ts->Uri_Tryo += wk_Electron_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
		}															/*								*/
	}

	if( CPrmSS[S_PRP][1] == 1 )										/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞを使用する設定	*/
	{																/*								*/
		dat1 = prm_get( COM_PRM,S_TOT,5,1,3 );						/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ総売上額/総掛売額加算設定	*/
		j = 0;														/*								*/
		for( i = 0; i < WTIK_USEMAX; i++ ){							/*								*/
			disc_wari_conv( &PayData.DiscountData[i], &wari_dt );	/*								*/
			if( wari_dt.tik_syu == PREPAID ){						/* ﾃﾞｰﾀあり						*/
				dat2 = wari_dt.pkno_syu;							/*								*/
				ts->Pcrd_use_cnt[dat2] += ryo_buf.pri_mai[j];		/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜回数＞(基本,拡張1,2,3)	*/
																	/* 7枚以上使用時は最後に使われた種別に合計加算する	*/
				ts->Pcrd_use_ryo[dat2] += wari_dt.ryokin;			/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜金額＞(基本,拡張1,2,3)	*/
																	/*								*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				if(Flp_sub_ok){										/* 詳細中止エリア使用			*/
					FLAPDT_SUB[Flp_sub_no].ppc_chusi_ryo += wari_dt.ryokin;/* 詳細中止エリアに加算する */
				}													/*								*/
				j++;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ )								/*								*/
	{																/*								*/
		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );		/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
		taxableAdd_set = 0xff;
		if( cancelReceipt_Waridata_chk( &wari_dt ) ){				/*			課税対象			*/
			taxableAdd_set = (uchar)prm_get( COM_PRM, S_TOT, 7, 1, 2 );	/*課税対象利用額の売上加算設定*/
		}	
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
		switch( wari_dt.tik_syu )									/*								*/
		{															/*								*/
			case SERVICE:											/* ｻｰﾋﾞｽ券						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* ｻｰﾋﾞｽ券A,B,C					*/
																	/*								*/
				ts->Stik_use_cnt[dat1][dat2] += wari_dt.maisuu;		/* ｻｰﾋﾞｽ券種別毎使用＜枚数＞(基本,拡張1,2,3)(ｻｰﾋﾞｽ券A,B,C)	*/
				ts->Stik_use_ryo[dat1][dat2] += wari_dt.ryokin;		/* ｻｰﾋﾞｽ券種別毎使用＜金額＞(基本,拡張1,2,3)(ｻｰﾋﾞｽ券A,B,C)	*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//				if( CPrmSS[S_SER][1+3*dat2] == 4 &&					/* ｻｰﾋﾞｽ券全額割引設定?			*/
				if( prm_get( COM_PRM, S_SER, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* ｻｰﾋﾞｽ券全額割引設定? */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					ryo_buf.pkiti != 0xffff &&						/* ﾏﾙﾁ精算以外?					*/
					uc_update == 0 )								/* 定期券更新精算以外			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* 修正用全額割引ﾌﾗｸﾞｾｯﾄ		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,1 );				/* ｻｰﾋﾞｽ券総売上額/総掛売額加算設定	*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				setting_taxableAdd( &dat1, taxableAdd_set );		/* 課税対象利用額の売上加算設定 */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				if(Flp_sub_ok){
					FLAPDT_SUB[Flp_sub_no].sev_tik[dat2] += wari_dt.maisuu;	// 詳細中止エリアに加算する
				}
				break;												/*								*/
																	/*								*/
			case KAKEURI:											/* 掛売券						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* 店№1～999(0～998)			*/
																	/*								*/
				ts->Mno_use_Tcnt += wari_dt.maisuu;					/* 店割引総ﾄｰﾀﾙ＜回数＞			*/
				ts->Mno_use_Tryo += wari_dt.ryokin;					/* 店割引総ﾄｰﾀﾙ＜金額＞			*/
																	/*								*/
				ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;			/* 店№割引ﾄｰﾀﾙ＜回数＞(基本,拡張1,2,3)	*/
				ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;			/* 店№割引ﾄｰﾀﾙ＜金額＞(基本,拡張1,2,3)	*/
																	/*								*/
				if( dat1 == KIHON_PKNO || dat1 == KAKUCHOU_1 )		/* 基本 or 拡張1				*/
				{													/*								*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
//					if( dat2 < 100 )								/* 店№1～100					*/
					if( dat2 < MISE_NO_CNT )						/* 店№1～100					*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* 店№1～100ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* 店№1～100ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
						ts->Mno_use_cnt4[dat1][dat2] +=				/* 店№毎割引＜回数＞(基本,拡張1)(店№1～100)	*/
												wari_dt.maisuu;		/*								*/
						ts->Mno_use_ryo4[dat1][dat2] +=				/* 店№毎割引＜金額＞(基本,拡張1)(店№1～100)	*/
												wari_dt.ryokin;		/*								*/
					}												/*								*/
					else											/* 店№101～999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* 店№101～999ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* 店№101～999ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
					}												/*								*/
				}													/*								*/
				else												/*								*/
				{													/*								*/
					if(( dat1 == KAKUCHOU_2 )&&						/* 拡張2						*/
					   ( prm_get( COM_PRM,S_TOT,16,1,3 ) == 1 ))	/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
					else if(( dat1 == KAKUCHOU_3 )&&				/* 拡張3						*/
							( prm_get( COM_PRM,S_TOT,16,1,4 ) == 1 ))/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
//					if( dat2 < 100 )								/* 店№1～100					*/
					if( dat2 < MISE_NO_CNT )						/* 店№1～100					*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* 店№1～100ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* 店№1～100ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
						if( dat1 == KAKUCHOU_1 )					/*								*/
						{											/*								*/
							ts->Mno_use_cnt4[dat1][dat2] +=			/* 店№毎割引＜回数＞(基本,拡張1)(店№1～100)	*/
												wari_dt.maisuu;		/*								*/
							ts->Mno_use_ryo4[dat1][dat2] +=			/* 店№毎割引＜金額＞(基本,拡張1)(店№1～100)	*/
												wari_dt.ryokin;		/*								*/
						}											/*								*/
					}												/*								*/
					else											/* 店№101～999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* 店№101～999ﾄｰﾀﾙ割引＜回数＞(基本,拡張1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* 店№101～999ﾄｰﾀﾙ割引＜金額＞(基本,拡張1,2,3)	*/
					}												/*								*/
				}													/*								*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//				if( CPrmSS[S_STO][1+3*dat2] == 4 &&					/* 店割引全額割引設定?			*/
				if( prm_get( COM_PRM, S_STO, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* 店割引全額割引設定? */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
					ryo_buf.pkiti != 0xffff &&						/* ﾏﾙﾁ精算以外?					*/
					uc_update == 0 )								/* 定期券更新精算以外			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* 修正用全額割引ﾌﾗｸﾞｾｯﾄ		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* 店割引総売上額/総掛売額加算設定	*/
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				setting_taxableAdd( &dat1, taxableAdd_set );		/* 課税対象利用額の売上加算設定 */
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				if(Flp_sub_ok){
					for(wk_cnt=0;wk_cnt<5;wk_cnt++){				//同じ店No検索
						if(FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no == wari_dt.syubetu){
							FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].maisuu += wari_dt.maisuu;//同じ店Noの時、枚数を格納する
							break;
						}
					}
					if(wk_cnt == 5){								//同じ店No無
						for(wk_cnt=0;wk_cnt<5;wk_cnt++){
							if(FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no == 0){
								FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no = wari_dt.syubetu;//店舗No
								FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].maisuu += wari_dt.maisuu;//枚数
								break;
							}
						}
					}
					if(wk_cnt == 5){								//同じ店No無＋詳細中止空きｴﾘｱ無

						if(Flp_sub_no < 9){
							uc_buf_size = (uchar)(9 - Flp_sub_no);
							memset(&FLAPDT_SUB[Flp_sub_no],0,sizeof(flp_com_sub));	// 詳細中止ｴﾘｱ(精算完了)ｸﾘｱ
							memmove(&FLAPDT_SUB[Flp_sub_no],&FLAPDT_SUB[Flp_sub_no+1],sizeof(flp_com_sub)*uc_buf_size);
						}
						if(Flp_sub_no == 10){
							memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));	// 終端詳細中止ｴﾘｱｸﾘｱ
						}else{
							memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));// 終端詳細中止ｴﾘｱｸﾘｱ
						}
						Flp_sub_ok = 0;								//格納ｸﾞﾗｸﾞｸﾘｱ
					}
				}
				break;												/*								*/
			case KAISUU:											/* 回数券						*/
				if( CPrmSS[S_PRP][1] != 2 )							/* 回数券を使用する設定ではない	*/
					break;

				dat1 = (ulong)wari_dt.pkno_syu;						/* 駐車場№の種類				*/
																	/*								*/
				ts->Ktik_use_cnt[dat1] += wari_dt.maisuu;			/* 使用＜枚数＞(基本,拡張1,2,3) */
				ts->Ktik_use_ryo[dat1] += wari_dt.ryokin;			/* 使用＜金額＞(基本,拡張1,2,3)	*/

				dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );				/* 回数券総売上額/総掛売額加算設定	*/
				if( dat1 == 1L )									/* 総売上額に加算する			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				else if( dat1 == 2L )								/* 総掛売額に加算する			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
				}													/*								*/
				else if( dat1 == 3L )								/* 総売上額/総掛売額に加算する	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* 総売上額						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* 総掛売額						*/
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
					if(date_uriage_use_chk() == 0){// 日付切替基準の総売上の小計取得する（リアルタイム情報用）
						if(date_uriage_cmp_paymentdate() == 0){// 加算可能
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
				}													/*								*/
				if(Flp_sub_ok){										//詳細中止エリア使用
					FLAPDT_SUB[Flp_sub_no].ppc_chusi_ryo += wari_dt.ryokin;	// 詳細中止エリアに加算する。
				}
				break;												/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
																	/*								*/
	ts->tou[0] += (ulong)ryo_buf.in_coin[0];						/* 精算投入金額  10円使用枚数	*/
	ts->tou[1] += (ulong)ryo_buf.in_coin[1];						/* 精算投入金額  50円使用枚数	*/
	ts->tou[2] += (ulong)ryo_buf.in_coin[2];						/* 精算投入金額 100円使用枚数	*/
	ts->tou[3] += (ulong)ryo_buf.in_coin[3];						/* 精算投入金額 500円使用枚数	*/
	ts->tou[4] += (ulong)ryo_buf.in_coin[4];						/* 精算投入金額1000円使用枚数	*/
	ts->tounyu = ts->tou[0]*10 + ts->tou[1]*50 + ts->tou[2]*100 +	/*								*/
				 ts->tou[3]*500 + ts->tou[4]*1000;					/* 精算投入金額	総額			*/
																	/*								*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[0];						/* 精算払出金額  10円使用枚数	*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[1];						/* 精算払出金額  50円使用枚数	*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[2];						/* 精算払出金額 100円使用枚数	*/
	ts->sei[3] += (ulong)ryo_buf.out_coin[3];						/* 精算払出金額 500円使用枚数	*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[4];						/* 精算払出金額  10円使用枚数(予蓄)*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[5];						/* 精算払出金額 50円使用枚数(予蓄)*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[6];						/* 精算払出金額 100円使用枚数(予蓄)*/
	ts->seisan = ts->sei[0]*10 + ts->sei[1]*50 + ts->sei[2]*100 +	/*								*/
				 ts->sei[3]*500;									/* 精算払出金額	総額			*/
																	/*								*/
	if(( card_use[USE_SVC] || card_use[USE_PPC] || card_use[USE_NUM] )&&	/* 割引有り?					*/
	   ryo_buf.pkiti != 0xffff &&									/* ﾏﾙﾁ精算以外?					*/
	   uc_update == 0 )												/* 定期券更新精算以外			*/
	{																/*								*/
		FLAPDT.flp_data[iti].bk_syu = (ushort)(ryo_buf.syubet + 1);	/* 種別(中止,修正用)			*/
		FLAPDT.flp_data[iti].bk_wmai = card_use[USE_SVC];			/* 使用枚数(中止,修正用)		*/
		FLAPDT.flp_data[iti].bk_wari = ryo_buf.fee_amount;			/* 割引金額(中止,修正用)		*/
		FLAPDT.flp_data[iti].bk_time = ryo_buf.tim_amount;			/* 割引時間数(中止,修正用)		*/
		if(Flp_sub_ok){
			FLAPDT_SUB[Flp_sub_no].syu = (uchar)(ryo_buf.syubet + 1);		// 種別
			FLAPDT_SUB[Flp_sub_no].TInTime = PayData.TInTime;				// 入庫時間
			FLAPDT_SUB[Flp_sub_no].WPlace = PayData.WPlace;			// 車室番号を格納する
		}
	}																/*								*/
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 52;											/* 52:精算途中停電復帰集計完了	*/
																	/*								*/
	if( ryo_buf.ryo_flg >= 2 )										/* 定期使用?					*/
	{																/*								*/
		memcpy( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* 中止定期ﾃﾞｰﾀ退避				*/
																	/*								*/
// MH810105(S) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
//		Log_regist( LOG_PAYSTOP );									/* 精算中止情報登録				*/
		Log_regist( LOG_PAYSTOP_FU );								/* 精算中止情報登録				*/
// MH810105(E) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
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
		Suica_Rec.Data.BIT.LOG_DATA_SET = 0;						/* Suica精算ﾌﾗｸﾞｸﾘｱ				*/
																	/*								*/
		f_SameData = 0;												/*								*/
																	/*								*/
		if( TKI_CYUSI_MAX < tki_cyusi.count )						/* fail safe					*/
			tki_cyusi.count = TKI_CYUSI_MAX;						/*								*/
																	/*								*/
		if( TKI_CYUSI_MAX <= tki_cyusi.wtp )						/* fail safe					*/
			tki_cyusi.wtp = tki_cyusi.count - 1;					/*								*/
																	/*								*/
		for( i=0; i<tki_cyusi.count; i++ )							/*								*/
		{															/*								*/
			if( 0L == tki_cyusi.dt[i].pk ){							/* ﾃｰﾌﾞﾙにﾃﾞｰﾀなし				*/
				break;												/* i=登録件数					*/
			}														/*								*/
			if( tki_cyusi.dt[i].no == PayData.teiki.id &&			/* 再中止?						*/
				tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*					*/
			{														/*								*/
				f_SameData = 1;										/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		if( 0 == f_SameData )										/* 同一ﾃﾞｰﾀなし(新規登録必要)	*/
		{															/*								*/
			if( i == TKI_CYUSI_MAX ){								/* 登録件数Full					*/
				NTNET_Snd_Data219(1, &tki_cyusi.dt[0]);				/* 1件削除通知送信				*/
				TKI_Delete(0);										/* 最古ﾃﾞｰﾀ削除					*/
				nmisave( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* 中止定期ﾃﾞｰﾀ退避		*/
			}														/*								*/
			tki_cyusi.dt[tki_cyusi.wtp].syubetu = PayData.syu;		/* 料金種別(A～L:1～12)			*/
			tki_cyusi.dt[tki_cyusi.wtp].pk =						/*								*/
						CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];	/* 駐車場№						*/
			tki_cyusi.dt[tki_cyusi.wtp].no = PayData.teiki.id;		/* 個人ｺｰﾄﾞ(1～12000)			*/
			tki_cyusi.dt[tki_cyusi.wtp].tksy = PayData.teiki.syu;	/* 定期種別(1～15)				*/
			tki_cyusi.dt[tki_cyusi.wtp].year = PayData.TInTime.Year;/* 処理年						*/
			memcpy( &tki_cyusi.dt[tki_cyusi.wtp].mon, PayData.teiki.t_tim, 4 );	/* 処理月日時分		*/
			tki_cyusi.dt[tki_cyusi.wtp].sec = 0;					/* 処理秒						*/
																	/*								*/
			NTNET_Snd_Data219(0, &tki_cyusi.dt[tki_cyusi.wtp]);		/* 1件更新通知送信				*/
																	/*								*/
			if( tki_cyusi.count < TKI_CYUSI_MAX )					/*								*/
			{														/*								*/
				tki_cyusi.count++;									/* 登録件数を+1					*/
			}														/*								*/
																	/*								*/
			tki_cyusi.wtp++;										/* ﾗｲﾄﾎﾟｲﾝﾀ+1					*/
			if( tki_cyusi.wtp >= TKI_CYUSI_MAX )					/*								*/
			{														/*								*/
				tki_cyusi.wtp = 0;									/*								*/
			}														/*								*/
		}															/*								*/
		WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* 定期券ﾃｰﾌﾞﾙ更新(出庫)	*/
					  (ushort)PayData.teiki.id,						/*								*/
					  1,											/*								*/
					  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*							*/
					  0xffff );										/*								*/
	}																/*								*/
	else{															/*								*/
// MH810105(S) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
//		Log_regist( LOG_PAYSTOP );									/* 精算中止情報登録				*/
		Log_regist( LOG_PAYSTOP_FU );								/* 精算中止情報登録				*/
// MH810105(E) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
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
		Suica_Rec.Data.BIT.LOG_DATA_SET = 0;					/* Suica精算ﾌﾗｸﾞｸﾘｱ				*/
	}																/*								*/
																	/*								*/
	CountUp( CANCEL_COUNT );										/* 精算中止追番+1				*/
	if( ryo_buf.fusoku != 0 ){										/* 預り証発行?					*/
		CountUp(DEPOSIT_COUNT);										/* 預り証追番+1					*/
	}
// MH810105(S) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
	}
// MH810105(E) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
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
		ac_flg.cycl_fg = 110;										// 処理未了取引記録登録開始
		EcAlarmLog_Regist(&EcAlarm.Ec_Res);
	}
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
																	/*								*/
	ac_flg.cycl_fg = 54;											/* 54:精算中止ﾃﾞｰﾀ登録完了		*/
																	/*								*/
	safecl( 7 );													/* 金庫枚数算出、釣銭管理集計	*/
																	/*								*/
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//	Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );		/* 中止時の金銭管理ログデータ作成*/
//	Log_regist( LOG_MONEYMANAGE_NT );								/* 金銭管理ログ登録				*/
	if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {	/* 中止時の金銭管理ログデータ作成*/
		Log_regist( LOG_MONEYMANAGE_NT );							/* 金銭管理ログ登録				*/
	}
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
																	/* 必ず金庫枚数算出後に行う 	*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| フラップ上昇・ロック閉タイム内出庫集計														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: lto_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lto_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	ushort	iti;													/*								*/

																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
																	/*								*/
	iti = locktimeout.fus_d[0].t_iti - 1;							/* 駐車位置(1～324)-1			*/
																	/*								*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
	memcpy( &wkloktotal,											/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
			&loktl.tloktl.loktldat[iti],							/*								*/
			sizeof( LOKTOTAL_DAT ) );								/*								*/
																	/*								*/
	ac_flg.cycl_fg = 42;											/* 42:Ｔ集計をﾜｰｸｴﾘｱへ転送完了	*/
																	/*								*/
																	/*								*/
	if( ryo_buf.svs_tim )											/* ｻｰﾋﾞｽﾀｲﾑ内出庫有り			*/
	{																/*								*/
		ts->In_svst_seisan += 1L;									/* ｻｰﾋﾞｽﾀｲﾑ内精算回数+1			*/
	}																/*								*/
																	/*								*/
	if( DoBunruiSyu(ryo_buf.syubet) ){								/* 分類集計する（種別毎ﾁｪｯｸ）	*/
		bunrui( 0, ryo_buf.dsp_ryo , ryo_buf.syubet );				/* 分類集計加算					*/
	}
																	/*								*/

	ac_flg.cycl_fg = 43;											/* 45:ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫情報登録完了*/
	PayData_set_LO( locktimeout.fus_d[0].t_iti , 0 , 0 , 97);		/*								*/
	Log_Write(eLOG_PAYMENT, &PayData, TRUE);						/*								*/
																	/*								*/
	ac_flg.cycl_fg = 45;											/* 45:ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫情報登録完了*/
																	/*								*/
	memcpy( &wklocktimeout, &locktimeout, sizeof( struct FUSEI_SD ) );	/* ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫ﾃﾞｰﾀをﾜｰｸｴﾘｱへ	*/
																	/*								*/
	ac_flg.cycl_fg = 46;											/* 46:ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫ﾃﾞｰﾀをﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
	memcpy( &locktimeout, &wklocktimeout.fus_d[1],					/* ﾃﾞｰﾀｼﾌﾄ						*/
			sizeof( struct FUSEI_D )*(LOCK_MAX-1) );				/*								*/
	locktimeout.kensuu -= 1;										/* 件数－１						*/
																	/*								*/
	ac_flg.cycl_fg = 47;											/* 47:ﾃﾞｰﾀｼﾌﾄ・件数-1完了		*/
																	/*								*/
	memset( &locktimeout.fus_d[LOCK_MAX-1], 0, sizeof( struct FUSEI_D ) );/*						*/
																	/*								*/
	ac_flg.cycl_fg = 48;											/* 48:6件目ｸﾘｱ完了				*/
																	/*								*/
	CountUp( PAYMENT_COUNT );										/* 精算追番+1					*/
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
	}
	return;															/*								*/
}																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| フラップ上昇・ロック閉タイム内出庫集計処理													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fus_kyo( void )																   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: void																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lto_syuko( void )											/*								*/
{																	/*								*/
	ushort	iti = 0;												/*								*/
	struct	CAR_TIM		in_bak;										/*								*/
	struct	CAR_TIM		ot_bak;										/*								*/
	struct	CAR_TIM		in_f_bak;									/*								*/
	struct	CAR_TIM		ot_f_bak;									/*								*/
	ulong	tyu_ryo, tax, nyukin, turisen;							/*								*/
	ushort	pkiti;													/*								*/
																	/*								*/
	if (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0)				// 精算ログのFlashRom書込み中？
		return;														// 
																	// 
	if(( locktimeout.kensuu != 0 )&&								/* ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫有り、かつ*/
	   ( ac_flg.cycl_fg == 0 ))										/* 印字完了?					*/
	{																/*								*/
		iti = locktimeout.fus_d[0].t_iti;							/*								*/
		pkiti = ryo_buf.pkiti;										/* 駐車位置						*/
		memcpy( &in_bak, &car_in, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &ot_bak, &car_ot, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &in_f_bak, &car_in_f, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &ot_f_bak, &car_ot_f, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &PayDataBack, &PayData, sizeof( Receipt_data ));	/* 1精算情報,領収証印字ﾃﾞｰﾀ		*/
		tyu_ryo = ryo_buf.tyu_ryo;									/* 駐車料金						*/
		tax = ryo_buf.tax;											/* 消費税						*/
		nyukin = ryo_buf.nyukin;									/* 入金額						*/
		turisen = ryo_buf.turisen;									/* 釣銭							*/
		if(( iti >= 1 )&&( iti <= LOCK_MAX ))						/* 駐車位置番号1以上324以下?	*/
		{															/*								*/
			ac_flg.cycl_fg = 40;									/*								*/
			cm27();													/*								*/
			if( lto_tim() == 1 ){									/* 入出庫時刻ｾｯﾄ				*/
																	/* 判定NG?(Y)					*/
																	/*								*/
				ac_flg.cycl_fg = 45;								/* 45:ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫情報登録完了*/
																	/*								*/
				memcpy( &wklocktimeout, &locktimeout, sizeof( struct FUSEI_SD ) );/*				*/
																	/*								*/
				ac_flg.cycl_fg = 46;								/* 46:ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫ﾃﾞｰﾀをﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
				memcpy( &locktimeout, &wklocktimeout.fus_d[1],		/* ﾃﾞｰﾀｼﾌﾄ						*/
						sizeof(struct FUSEI_D)*(LOCK_MAX-1) );		/*								*/
				locktimeout.kensuu -= 1;							/* 件数-1						*/
																	/*								*/
				ac_flg.cycl_fg = 47;								/* 47:ﾃﾞｰﾀｼﾌﾄ・件数-1完了		*/
																	/*								*/
				memset( &locktimeout.fus_d[(LOCK_MAX-1)],			/*								*/
						0, sizeof(struct FUSEI_D));					/*								*/
																	/*								*/
				ac_flg.cycl_fg = 0;									/* 0:6件目ｸﾘｱ完了(待機)			*/
				return;												/*								*/
			}
			ryo_buf.credit.pay_ryo = 0;
			ryo_cal( 0, iti );										/* 料金計算						*/
//			PayData_set(0,0);
			ryo_buf.svs_tim = 1;
			ryo_buf.dsp_ryo = 0;
			ac_flg.cycl_fg = 41;									/*								*/
			lto_syuu();												/* 集計加算						*/
																	/* 								*/
		} else {													/* 車室番号が想定外?(Y)			*/
																	/* 								*/
			ac_flg.cycl_fg = 45;									/* 45:ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫情報登録完了*/
																	/*								*/
			memcpy( &wklocktimeout, &locktimeout, sizeof( struct FUSEI_SD ) );	/*					*/
																	/*								*/
			ac_flg.cycl_fg = 46;									/* 46:ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫ﾃﾞｰﾀをﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
			memcpy( &locktimeout, &wklocktimeout.fus_d[1],			/* ﾃﾞｰﾀｼﾌﾄ						*/
					sizeof(struct FUSEI_D)*(LOCK_MAX-1) );			/*								*/
			locktimeout.kensuu -= 1;								/* 件数-1						*/
																	/*								*/
			ac_flg.cycl_fg = 47;									/* 47:ﾃﾞｰﾀｼﾌﾄ・件数-1完了		*/
																	/*								*/
			memset( &locktimeout.fus_d[(LOCK_MAX-1)],				/*								*/
					0, sizeof(struct FUSEI_D));						/*								*/
																	/*								*/
			ac_flg.cycl_fg = 48;									/* 48:6件目ｸﾘｱ完了				*/
																	/*								*/
			ex_errlg( ERRMDL_MAIN, ERR_MAIN_LOCKNUMNG, 2, 0 );		/* ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇出庫位置1～324以外	*/
		}															/*								*/
		ac_flg.cycl_fg = 0;											/*								*/
		ryo_buf.pkiti = pkiti;										/* 駐車位置						*/
		memcpy( &car_in, &in_bak, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &car_ot, &ot_bak, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &car_in_f, &in_f_bak, sizeof( struct CAR_TIM ));	/*								*/
		NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
		memcpy( &car_ot_f, &ot_f_bak, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &PayData, &PayDataBack, sizeof( Receipt_data ));	/* 1精算情報,領収証印字ﾃﾞｰﾀ		*/
		NTNET_Data152_SaveDataUpdate();
		ryo_buf.tyu_ryo = tyu_ryo;									/* 駐車料金						*/
		ryo_buf.tax = tax;											/* 消費税						*/
		ryo_buf.nyukin = nyukin;									/* 入金額						*/
		ryo_buf.turisen = turisen;									/* 釣銭							*/
																	/*								*/
	}																/*								*/
	return;															/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 入出庫時刻セット																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: lto_tim( void )																   |*/
/*| RETURN VALUE: ret	: 入出庫時刻判定(1980～2079年以内か？)  0:OK  1:NG						   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*|				car_in,car_ot,car_in_f,car_ot_fに入出庫時刻をセットする							   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short	lto_tim( void )										/*								*/
{																	/*								*/
	ushort	in_tim, out_tim;										/*								*/
																	/*								*/
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					/* 入車時刻ｸﾘｱ					*/
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					/* 出車時刻ｸﾘｱ					*/
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				/* 入車時刻ｸﾘｱ					*/
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				/* 出車時刻ｸﾘｱ					*/
																	/*								*/
	car_in.year = locktimeout.fus_d[0].iyear;						/* 入車	年						*/
	car_in.mon  = locktimeout.fus_d[0].imont;						/*		月						*/
	car_in.day  = locktimeout.fus_d[0].idate;						/*		日						*/
	car_in.hour = locktimeout.fus_d[0].ihour;						/*		時						*/
	car_in.min  = locktimeout.fus_d[0].iminu;						/*		分						*/
	car_in.week = (char)youbiget( car_in.year,						/*		曜日					*/
								(short)car_in.mon,					/*								*/
								(short)car_in.day );				/*								*/
						 											/*								*/
	car_ot.year = locktimeout.fus_d[0].oyear;						/* 出庫	年						*/
	car_ot.mon  = locktimeout.fus_d[0].omont;						/*		月						*/
	car_ot.day  = locktimeout.fus_d[0].odate;						/*		日						*/
	car_ot.hour = locktimeout.fus_d[0].ohour;						/*		時						*/
	car_ot.min  = locktimeout.fus_d[0].ominu;						/*		分						*/
	car_ot.week = (char)youbiget( car_ot.year,						/*		曜日					*/
								(short)car_ot.mon,					/*								*/
								(short)car_ot.day );				/*								*/
	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			/* 入車時刻Fix					*/
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			/* 出車時刻Fix					*/
																	/*								*/
	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	/* 入庫時刻規定外?			*/
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	/* 出庫時刻規定外?			*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKINOUTTIMENG, 2, 0 );		/* 入出庫時刻規定外				*/
		return( 1 );												/*								*/
	}																/*								*/
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					/* (入庫日+1年)normlize			*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	out_tim = dnrmlzm( car_ot.year,									/* 出庫日normlize				*/
					(short)car_ot.mon,								/*								*/
					(short)car_ot.day );							/*								*/
	if( in_tim <= out_tim ){										/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKOVER1YEARCAL, 2, 1 );		/* 1年以上の料金計算を行おうとした */
		return( 1 );												/*								*/
	}																/*								*/
	in_tim = dnrmlzm((short)( car_in.year ),						/* (入庫日)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){											/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKINOUTGYAKU, 2, 1 );		/* 入出庫逆転					*/
		return( 1 );												/*								*/
	}																/*								*/
	return( 0 );													/*								*/
}																	/*								*/

//[]----------------------------------------------------------------------[]
///	@brief			Ｔ・ＧＴ・ＭＴプリント
//[]----------------------------------------------------------------------[]
///	@param[in]		preq	: 印字タイプ
///	@param[in]		*src	: 集計データ
///	@return			*印字データ
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
SYUKEI	*syuukei_prn( int preq, SYUKEI *src )
{
	uchar	set1_39_6;

	src->Kikai_no = (uchar)CPrmSS[S_PAY][2];					// 機械№
	if (preq == PREQ_AT_SYUUKEI){
		src->Kakari_no = 0;										// 自動集計時は係員番号を印字しない
	}
	else{
		src->Kakari_no = OPECTL.Kakari_Num;						// 係り員No.
		loktl.tloktl.Kakari_no = OPECTL.Kakari_Num;
	}
	memcpy( &src->NowTime, &CLK_REC, sizeof( date_time_rec ) );	// 現在時刻
	if (preq == PREQ_AT_SYUUKEI ||
		preq == PREQ_TGOUKEI) {
		memcpy( &src->In_car_Tcnt, &CarCount, sizeof( CAR_COUNT ) );	// 入出庫台数
		CountGet( T_TOTAL_COUNT, &src->Oiban );					// T合計追番
		CountGet( T_TOTAL_COUNT, &loktl.tloktl.Oiban );			// T合計追番(車室集計)
	}
	else if (preq == PREQ_GTGOUKEI) {
		CountGet( GT_TOTAL_COUNT, &src->Oiban );				// GT合計追番
		CountGet( GT_TOTAL_COUNT, &loktl.tloktl.Oiban );		// GT合計追番(車室集計)
	}
	set1_39_6 = (uchar)prm_get(COM_PRM,S_SYS,39,1,1);
	if( set1_39_6 == 3 ){
		src->Now_car_Tcnt = PPrmSS[S_P02][6]+PPrmSS[S_P02][10];	// 総現在駐車台数
	}else if( set1_39_6 == 4 ){
		src->Now_car_Tcnt = PPrmSS[S_P02][6]+PPrmSS[S_P02][10]	// 総現在駐車台数
							   + PPrmSS[S_P02][14];
	}else{
		src->Now_car_Tcnt = PPrmSS[S_P02][2];					// 総現在駐車台数
	}
	src->Now_car_cnt[0] = PPrmSS[S_P02][6];						// 現在駐車台数1
	src->Now_car_cnt[1] = PPrmSS[S_P02][10];					// 現在駐車台数2
	src->Now_car_cnt[2] = PPrmSS[S_P02][14];					// 現在駐車台数3
	memcpy(&skyprn, src, sizeof(SYUKEI));			// 印字データ確定
	return &skyprn;
}

// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
const	date_time_rec	nulldate = { 0 };
SYUKEI	*tsyTbl[] = { &skyprn, &sky.gsyuk, &sky.msyuk};				/* 今回Ｔ集計（印字）			*/
SYUKEI	*dsyTbl[] = { &sky.tsyuk, &sky.gsyuk, &sky.msyuk};			/* 現在Ｔ集計					*/
SYUKEI	*zsyTbl[] = { &skybk.tsyuk, &skybk.gsyuk, &skybk.msyuk};	/* 前回Ｔ集計					*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 集計ｴﾘｱクリア判定処理																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: Check_syuukei_clr( char no )														   |*/
/*| PARAMETER	: no	: 0 = Ｔ合計																		   |*/
/*| 					: 1 = ＧＴ合計															   |*/
/*| 					: 2 = ＭＴ合計															   |*/
/*| RETURN VALUE: char	: 0 = クリア不可														   |*/
/*|						  1 = クリア可															   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
char	Check_syuukei_clr( char no )
{
	unsigned long	*ts, *gs;										/*								*/
	SYUKEI	*tsy, *dsy, *zsy;
	int		i;														/*								*/

	tsy = tsyTbl[no];												/* 今回集計（印字）				*/
	dsy = dsyTbl[no];												/* 現在集計						*/
	zsy = zsyTbl[no];												/* 前回集計						*/
	

	//①印字集計のヘッダーと前回集計のヘッダーの比較
	// syuukei_clr()された場合、印字したデータと前回集計が一致する
	if (memcmp(zsy, tsy, _SYU_HDR_SIZ) == 0) {
	// 印字内容が登録済みの前回合計と一致
		wmonlg(OPMON_SYUUKEI_IRGCLR1, NULL, (ulong)(no+1));					// モニタ登録
		return 0;
	}

	//②現在集計の追番、今回集計日時、前回集計日時と 印字集計の追番、今回集計日時、前回集計日時の比較
	if( ( 0 != memcmp( &dsy->NowTime, &tsy->NowTime, sizeof( date_time_rec ) ))	||	// 今回集計日時が不一致
		( 0 != memcmp( &dsy->OldTime, &tsy->OldTime, sizeof( date_time_rec ) )) ||		// 前回集計日時が不一致
		( CountSel( &dsy->Oiban ) != CountSel( &tsy->Oiban ) )){						// 印字開始時にセットした追番と不一致
		// 不一致の場合、クリア不可
		wmonlg(OPMON_SYUUKEI_IRGCLR2, NULL, (ulong)(no+1));					// モニタ登録
		return 0;
	}

	//④現在集計の今回集計日時が0クリアされているか確認
	if (memcmp(&dsy->NowTime, &nulldate, sizeof(dsy->NowTime)) == 0) {
		// 現在集計日時がない＝クリアされている
		wmonlg(OPMON_SYUUKEI_IRGCLR4, NULL, (ulong)(no+1));					// モニタ登録
		return 0;
	}

	// T合計以外はここまででOKとする
	if( no != 0 ){
		return 1;
	}
	
	// ここまでで1回のT合計で2回クリア処理が実行されてもはじかれると思われるが、値の比較を行う
	// 必ず「T合計印字エリア≦T合計本体エリア」のはずなので左記条件を満たさない項目があればクリア処理を実行しない
	gs = &dsy->Uri_Tryo;										/*								*/
	ts = &tsy->Uri_Tryo;										/* Ｔ－印字Ｔ					*/ 
	WACDOG;														/* ｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行				*/
	//③現在T合計項目の値と印字T合計項目の値の比較
	for( i = 0; i < _SYU_DAT_CNT; i++, ts++, gs++ )				/*								*/
	{															/*								*/
		if( *gs < *ts ){										// T集計印字の方が大きい値はあり得ない
			wmonlg(OPMON_SYUUKEI_IRGCLR3, NULL, (ulong)(no+1));			// モニタ登録
			return 0;											/*								*/
		}														/*								*/
	}															/*								*/
	WACDOG;														/* ｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行				*/
	
	return 1;
}
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）

/*[]----------------------------------------------------------------------------------------------[]*/
/*| Ｔ・ＧＴ集計ｴﾘｱ更新																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: syuukei_clr( no )																   |*/
/*| PARAMETER	: no	: 0 = Ｔ合計															   |*/
/*| 					: 1 = ＧＴ合計															   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-12-28																	   |*/
/*| Update		: 2002-03-26 Y.Takahashi                                                           |*/
/*| Update		: 2004-07-13 T.Nakayama 4500EX:FLP_MAXに置き換え                                   |*/
/*|             : サービス券、定期券情報加算                                                       |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	syuukei_clr( char no )										/*								*/
{																	/*								*/
	unsigned long	*ts, *gs;										/*								*/
	SYUKEI	*tsy, *gsy, *msy, *wsy, *tsybk, *dsy;					/* msy(MT合計),dsy(T合計)を追加	*/
	int		i;														/*								*/
																	/*								*/
	tsy = &skyprn;													/*								*/
	gsy = &sky.gsyuk;												/*								*/
	msy = &sky.msyuk;												/* msy(MT合計)を追加			*/
	tsybk = &skybk.tsyuk;											/*								*/
	wsy = &wksky;													/*								*/
																	/*								*/
	if( no == 0 ){													/*								*/
		ac_flg.syusyu = 12;											/* 12:Ｔ合計印字完了			*/
																	/*								*/
		memcpy( wsy, gsy, sizeof( SYUKEI ) );						/* ＧＴ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
		memcpy( &wkloktotal, loktl.gloktl.loktldat, sizeof( LOKTOTAL_DAT ) ); /* 車室毎GT集計をﾜｰｸｴﾘｱへ(停電対策)	*/
		memcpy( &CarCount_W, &CarCount, sizeof( CAR_COUNT ) );		/* 入出庫ｶｳﾝﾄﾊﾞｯｸｱｯﾌﾟ			*/
		ac_flg.syusyu = 13;											/* 13:ＧＴ集計をﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
		if( tsy->In_car_Tcnt <= CarCount.In_car_Tcnt ){				/*								*/
			CarCount.In_car_Tcnt -= tsy->In_car_Tcnt;				/* 総入庫台数更新				*/
		}else{														/*								*/
			CarCount.In_car_Tcnt = 0L;								/* 総入庫台数更新				*/
		}															/*								*/
		if( tsy->In_car_cnt[0] <= CarCount.In_car_cnt[0] ){			/*								*/
			CarCount.In_car_cnt[0] -= tsy->In_car_cnt[0];			/* 入庫台数1更新				*/
		}else{														/*								*/
			CarCount.In_car_cnt[0] = 0L;							/* 入庫台数1更新				*/
		}															/*								*/
		if( tsy->In_car_cnt[1] <= CarCount.In_car_cnt[1] ){			/*								*/
			CarCount.In_car_cnt[1] -= tsy->In_car_cnt[1];			/* 入庫台数2更新				*/
		}else{														/*								*/
			CarCount.In_car_cnt[1] = 0L;							/* 入庫台数2更新				*/
		}															/*								*/
		if( tsy->In_car_cnt[2] <= CarCount.In_car_cnt[2] ){			/*								*/
			CarCount.In_car_cnt[2] -= tsy->In_car_cnt[2];			/* 入庫台数3更新				*/
		}else{														/*								*/
			CarCount.In_car_cnt[2] = 0L;							/* 入庫台数3更新				*/
		}															/*								*/
		if( tsy->Out_car_Tcnt <= CarCount.Out_car_Tcnt ){			/*								*/
			CarCount.Out_car_Tcnt -= tsy->Out_car_Tcnt;				/* 総出庫台数更新				*/
		}else{														/*								*/
			CarCount.Out_car_Tcnt = 0L;								/* 総出庫台数更新				*/
		}															/*								*/
		if( tsy->Out_car_cnt[0] <= CarCount.Out_car_cnt[0] ){		/*								*/
			CarCount.Out_car_cnt[0] -= tsy->Out_car_cnt[0];			/* 出庫台数1更新				*/
		}else{														/*								*/
			CarCount.Out_car_cnt[0] = 0L;							/* 出庫台数1更新				*/
		}															/*								*/
		if( tsy->Out_car_cnt[1] <= CarCount.Out_car_cnt[1] ){		/*								*/
			CarCount.Out_car_cnt[1] -= tsy->Out_car_cnt[1];			/* 出庫台数2更新				*/
		}else{														/*								*/
			CarCount.Out_car_cnt[1] = 0L;							/* 出庫台数2更新				*/
		}															/*								*/
		if( tsy->Out_car_cnt[2] <= CarCount.Out_car_cnt[2] ){		/*								*/
			CarCount.Out_car_cnt[2] -= tsy->Out_car_cnt[2];			/* 出庫台数3更新				*/
		}else{														/*								*/
			CarCount.Out_car_cnt[2] = 0L;							/* 出庫台数3更新				*/
		}															/*								*/
																	/*								*/
		ts = &tsy->Uri_Tryo;										/*								*/
		gs = &sky.gsyuk.Uri_Tryo;									/*								*/
																	/*								*/
		for( i = 0; i < _SYU_DAT_CNT; i++, ts++, gs++ )				/*								*/
		{															/*								*/
			*gs += *ts;												/*								*/
		}															/*								*/
		CountAdd(gsy, tsy);											/* 追番加算						*/
																	/*								*/
		for( i = 0; i < LOCK_MAX; i++ )
		{
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			loktl.gloktl.loktldat[i].Genuri_ryo += loktl.tloktl.loktldat[i].Genuri_ryo;
			loktl.gloktl.loktldat[i].Seisan_cnt += loktl.tloktl.loktldat[i].Seisan_cnt;
			loktl.gloktl.loktldat[i].Husei_out_cnt += loktl.tloktl.loktldat[i].Husei_out_cnt;
			loktl.gloktl.loktldat[i].Husei_out_ryo += loktl.tloktl.loktldat[i].Husei_out_ryo;
			loktl.gloktl.loktldat[i].Kyousei_out_cnt += loktl.tloktl.loktldat[i].Kyousei_out_cnt;
			loktl.gloktl.loktldat[i].Kyousei_out_ryo += loktl.tloktl.loktldat[i].Kyousei_out_ryo;
			loktl.gloktl.loktldat[i].Uketuke_pri_cnt += loktl.tloktl.loktldat[i].Uketuke_pri_cnt;
			loktl.gloktl.loktldat[i].Syuusei_seisan_cnt += loktl.tloktl.loktldat[i].Syuusei_seisan_cnt;
			loktl.gloktl.loktldat[i].Syuusei_seisan_ryo += loktl.tloktl.loktldat[i].Syuusei_seisan_ryo;
		}

		memcpy( tsybk, tsy, sizeof( SYUKEI ) );						/* Ｔ集計を前回Ｔ集計へｾｯﾄ		*/
		memcpy( &loktlbk.tloktl, &loktl.tloktl, sizeof( LOKTOTAL ) ); /* 車室毎集計を前回集計へｾｯﾄ	*/
																	/*								*/
		ac_flg.syusyu = 14;											/* 14:Ｔ集計をＧＴ集計へ加算完了*/
																	/*								*/
		Log_regist( LOG_TTOTAL );									/* Ｔ合計情報登録				*/
																	/*								*/
		ac_flg.syusyu = 16;											/* 16:Ｔ合計情報登録完了		*/
																	/*								*/
																	/*								*/
		if( prm_get(COM_PRM, S_SCA, 11, 1, 1) == 1 )				/*								*/
			Suica_Data_Snd( S_PAY_DATA,&CLK_REC);					/* SX-10締め処理実行			*/
																	/*								*/
// note:印字中に精算なし出庫などでT集計がカウントアップしてしまった場合の対策
//		T集計実行時、「sky.tsyuk」から「skyprn」を作成し、今回のT集計のデータとして確定させる。
//		「skyprn」のデータで印刷、Log登録などの処理を実施。「sky.tsyuk」は集計領域として、通常通りカウントアップをする。
//		「sky.tsyuk」 - 「skyprn」 をすると印字中にカウントアップしたもののみ残り、これを以降のT集計「sky.tsyuk」とする。
		dsy = &sky.tsyuk;		// dst = T							/* Ｔ集計						*/
		memcpy( wsy, dsy, sizeof( SYUKEI ) );						/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
		ac_flg.syusyu = 17;											/* 17:Ｔ集計をﾜｰｸｴﾘｱへ転送完了	*/
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）(コメントのみ追加)
// ここでT集計のヘッダー部をクリアしているので追番エリアは0になる
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）(コメントのみ追加)
		memset( dsy, 0, _SYU_HDR_SIZ );								/* ヘッダー部クリア				*/
		gs = &dsy->Uri_Tryo;										/*								*/
		ts = &tsy->Uri_Tryo;										/* Ｔ－印字Ｔ					*/ 
		WACDOG;														/* ｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行				*/
		for( i = 0; i < _SYU_DAT_CNT; i++, ts++, gs++ )				/*								*/
		{															/*								*/
			*gs -= *ts;												/*								*/
		}															/*								*/
		memcpy( &dsy->OldTime, &tsybk->NowTime, sizeof( date_time_rec ) ); /* 前回集計時刻ｾｯﾄ			*/
		memset( &loktl.tloktl, 0, sizeof( LOKTOTAL ) );				/* 車室毎集計をｸﾘｱ				*/
																	/*								*/
		ac_flg.syusyu = 18;											/* 18:Ｔ集計ｴﾘｱｸﾘｱ完了			*/
																	/*								*/
		CountUp( T_TOTAL_COUNT );									/* T合計追番+1					*/
		CountClear( CLEAR_COUNT_T );								/* 追番ｸﾘｱ(T合計完了時)			*/
		if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			RAU_UpdateCenterSeqNo(RAU_SEQNO_TOTAL);
		}
																	/*								*/
	}																/*								*/
	else if( no == 1 ) {											/*								*/
		int		mtflg;
		ac_flg.syusyu = 22;											/* 22:ＧＴ合計印字完了			*/
		mtflg = (prm_get(COM_PRM, S_TOT, 1, 1, 1) != 0)? 1 : 0;		/* MT集計する／しない			*/
																	/*								*/
		memcpy( wsy, msy, sizeof( SYUKEI ) );						/* ＭＴ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
																	/*								*/
		ac_flg.syusyu = 23;											/* 23:ＭＴ集計をﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
		memcpy( &skybk.gsyuk, gsy, sizeof( SYUKEI ) );				/* ＧＴ集計を前回ＧＴ集計へｾｯﾄ	*/
		memcpy( &loktlbk.gloktl, &loktl.gloktl, sizeof( LOKTOTAL ) ); /* 車室毎集計を前回集計へｾｯﾄ	*/
		if (mtflg) {
			ts = &gsy->Uri_Tryo;									/*								*/
			gs = &msy->Uri_Tryo;									/*								*/
																	/*								*/
			WACDOG;													/* ｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行				*/
			for( i = 0; i < _SYU_DAT_CNT; i++, ts++, gs++ )			/*								*/
			{														/*								*/
				*gs += *ts;											/*								*/
			}														/*								*/
			CountAdd(msy, gsy);										/* 追番加算						*/
		}
		if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			tsy->CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_TOTAL);
		}

		ac_flg.syusyu = 24;											/* 24:ＧＴ集計をＭＴ集計へ加算完了*/
		Log_regist( LOG_GTTOTAL );									/* ＧＴ合計情報登録				*/

		ac_flg.syusyu = 26;											/* 26:ＧＴ合計情報登録完了		*/

		memset( gsy, 0, sizeof( sky.gsyuk ) );						/* ＧＴ集計ｴﾘｱｸﾘｱ				*/
		memcpy( &gsy->OldTime, &skybk.gsyuk.NowTime, sizeof( date_time_rec ) );	/* 前回集計時刻ｾｯﾄ		*/
		memset( &loktl.gloktl, 0, sizeof( LOKTOTAL ) );				/* 車室毎集計をｸﾘｱ				*/
																	/*								*/
		ac_flg.syusyu = 27;											/* 27:ＧＴ集計ｴﾘｱｸﾘｱ完了		*/
																	/*								*/
		if (mtflg) {
			CountUp( GT_TOTAL_COUNT );								/* GT合計追番+1					*/
		}
		CountClear( CLEAR_COUNT_GT );								/* 追番ｸﾘｱ(GT合計完了時)		*/
		if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			RAU_UpdateCenterSeqNo(RAU_SEQNO_TOTAL);
		}
																	/*								*/
	}																/*								*/
	else{
	// ＭＴ合計印字完了後
		ac_flg.syusyu = 102;										/* 102:ＭＴ合計印字完了			*/
		memcpy( &skybk.msyuk, msy, sizeof( SYUKEI ) );				/* ①ＭＴ集計を前回ＭＴ集計へ	*/
																	/*								*/
		ac_flg.syusyu = 107;										/*								*/
		memset( msy, 0, sizeof( SYUKEI ) );							/* ＭＴ集計ｴﾘｱｸﾘｱ				*/
		memcpy( &msy->OldTime, &skybk.msyuk.NowTime, sizeof( date_time_rec ) ); /* 前回集計時刻ｾｯﾄ			*/
		CountClear( CLEAR_COUNT_MT );								/* 追番ｸﾘｱ(MT合計完了時)		*/ 
																	/*								*/
		ac_flg.syusyu = 0;											/*								*/
	}																/*								*/
																	/*								*/
	return;															/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ｺｲﾝ・紙幣金庫集計＆印字																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kinko_syu( no, pri )															   |*/
/*| PARAMETER	: no	: 0 = ｺｲﾝ金庫合計														   |*/
/*| 					: 1 = 紙幣金庫合計														   |*/
/*| 					: 2 = ｺｲﾝ金庫小計														   |*/
/*| 					: 3 = 紙幣金庫小計														   |*/
/*| 																							   |*/
/*| 			  pri	: 0 = 印字なし															   |*/
/*| 			  		: 1 = 印字あり															   |*/
/*| 																							   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*| 																							   |*/
/*| 備考：第２ﾊﾟﾗﾒｰﾀ（印字なし／あり）は、金庫合計時のみ有効とする								   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Motohashi																	   |*/
/*| Date		: 2005-09-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	kinko_syu( char no, char pri )
{
	int		i;
	T_FrmCoSyuukei FrmCoSyuukei;
	T_FrmSiSyuukei FrmSiSyuukei;

	switch( no ){

		case	0:	// ｺｲﾝ金庫合計

			if( pri == 1 ){
				ac_flg.syusyu = 30;														// 30:印字ﾃﾞｰﾀ作成開始
			}
			memset( &coin_syu.Tryo, 0, sizeof(ulong) * (1+(COIN_SYU_CNT*2)) );
			coin_syu.cnt[0] = SFV_DAT.safe_dt[0];
			coin_syu.cnt[1] = SFV_DAT.safe_dt[1];
			coin_syu.cnt[2] = SFV_DAT.safe_dt[2];
			coin_syu.cnt[3] = SFV_DAT.safe_dt[3];
			coin_syu.ryo[0] = ( (unsigned long)coin_syu.cnt[0] * 10L );
			coin_syu.ryo[1] = ( (unsigned long)coin_syu.cnt[1] * 50L );
			coin_syu.ryo[2] = ( (unsigned long)coin_syu.cnt[2] * 100L );
			coin_syu.ryo[3] = ( (unsigned long)coin_syu.cnt[3] * 500L );
			for( i = 0; i < 4; i++ ){
				coin_syu.Tryo += coin_syu.ryo[i];										// 金庫入金総額加算
			}
			if( pri == 1 ){
				ac_flg.syusyu = 31;														// 31:印字ﾃﾞｰﾀ作成完了
			}
			coin_syu.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// 機械№
			CountGet( COIN_SAFE_COUNT, &coin_syu.Oiban );								// 追番
			coin_syu.Kakari_no = OPECTL.Kakari_Num;										// 係員番号set
			memcpy( &coin_syu.NowTime, &CLK_REC, sizeof( date_time_rec ) );				// 現在時刻
			if( pri == 1 ){
				// 印字する場合
				FrmCoSyuukei.prn_kind = R_PRI;
				FrmCoSyuukei.prn_data = &coin_syu;
				queset( PRNTCBNO, PREQ_COKINKO_G, sizeof(T_FrmCoSyuukei), &FrmCoSyuukei );	// ｺｲﾝ金庫合計印字
			}
			else{
				// 印字しない場合
				kinko_clr( 0 );															// ｺｲﾝ金庫更新処理
			}

			break;

		case	1:	// 紙幣金庫合計

			if( pri == 1 ){
				ac_flg.syusyu = 40;														// 40:印字ﾃﾞｰﾀ作成開始
			}
			memset( &note_syu.Tryo, 0, sizeof(ulong) * (1+(SIHEI_SYU_CNT*2)) );
			note_syu.cnt[0] = ( (unsigned long)SFV_DAT.nt_safe_dt );					// 紙幣枚数
			note_syu.ryo[0] = ( (unsigned long)SFV_DAT.nt_safe_dt*1000L );				// 金庫入金額
			for( i = 0; i < 4; i++ ){
				note_syu.Tryo += note_syu.ryo[i];										// 金庫入金総額加算
			}
			if( pri == 1 ){
				ac_flg.syusyu = 41;														// 41:印字ﾃﾞｰﾀ作成完了
			}
			note_syu.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// 機械№
			CountGet( NOTE_SAFE_COUNT, &note_syu.Oiban );								// 追番
			note_syu.Kakari_no = OPECTL.Kakari_Num;										// 係員番号set
			memcpy( &note_syu.NowTime, &CLK_REC, sizeof( date_time_rec ) );				// 現在時刻
			if( pri == 1 ){
				// 印字する場合
				FrmSiSyuukei.prn_kind = R_PRI;
				FrmSiSyuukei.prn_data = &note_syu;
				queset( PRNTCBNO, PREQ_SIKINKO_G, sizeof(T_FrmSiSyuukei), &FrmSiSyuukei );	// 紙幣金庫合計印字
			}
			else{
				// 印字しない場合
				kinko_clr( 1 );															// 紙幣金庫更新処理
			}

			break;

		case	2:	// ｺｲﾝ金庫小計

			memset( &coin_syu.Tryo, 0, sizeof(ulong) * (1+(COIN_SYU_CNT*2)) );
			coin_syu.cnt[0] = SFV_DAT.safe_dt[0];
			coin_syu.cnt[1] = SFV_DAT.safe_dt[1];
			coin_syu.cnt[2] = SFV_DAT.safe_dt[2];
			coin_syu.cnt[3] = SFV_DAT.safe_dt[3];
			coin_syu.ryo[0] = ( (unsigned long)coin_syu.cnt[0] * 10L );
			coin_syu.ryo[1] = ( (unsigned long)coin_syu.cnt[1] * 50L );
			coin_syu.ryo[2] = ( (unsigned long)coin_syu.cnt[2] * 100L );
			coin_syu.ryo[3] = ( (unsigned long)coin_syu.cnt[3] * 500L );
			for( i = 0; i < 4; i++ ){
				coin_syu.Tryo += coin_syu.ryo[i];										// 金庫入金総額加算
			}

			coin_syu.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// 機械№
			coin_syu.Kakari_no = OPECTL.Kakari_Num;										// 係員番号set
			memcpy( &coin_syu.NowTime, &CLK_REC, sizeof( date_time_rec ) );				// 現在時刻
			FrmCoSyuukei.prn_kind = R_PRI;
			FrmCoSyuukei.prn_data = &coin_syu;
			queset( PRNTCBNO, PREQ_COKINKO_S, sizeof(T_FrmCoSyuukei), &FrmCoSyuukei );	// ｺｲﾝ金庫小計印字

			break;

		case	3:	// 紙幣金庫小計

			memset( &note_syu.Tryo, 0, sizeof(ulong) * (1+(SIHEI_SYU_CNT*2)) );
			note_syu.cnt[0] = ( (unsigned long)SFV_DAT.nt_safe_dt );					// 紙幣枚数
			note_syu.ryo[0] = ( (unsigned long)SFV_DAT.nt_safe_dt*1000L );				// 金庫入金額
			for( i = 0; i < 4; i++ ){
				note_syu.Tryo += note_syu.ryo[i];										// 金庫入金総額加算
			}

			note_syu.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// 機械№
			note_syu.Kakari_no = OPECTL.Kakari_Num;										// 係員番号set
			memcpy( &note_syu.NowTime, &CLK_REC, sizeof( date_time_rec ) );				// 現在時刻
			FrmSiSyuukei.prn_kind = R_PRI;
			FrmSiSyuukei.prn_data = &note_syu;
			queset( PRNTCBNO, PREQ_SIKINKO_S, sizeof(T_FrmSiSyuukei), &FrmSiSyuukei );	// 紙幣金庫集計印字

			break;

	}

	return;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ｺｲﾝ・紙幣金庫集計更新																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kinko_clr( no )																   |*/
/*| PARAMETER	: no	: 0 = ｺｲﾝ金庫															   |*/
/*| 					: 1 = 紙幣金庫															   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-12-12																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	kinko_clr( char no )										/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	int		i;														/*								*/
	COIN_SYU	*cbk;
	NOTE_SYU	*nbk;

	cbk = &cobk_syu;
	nbk = &nobk_syu;
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
																	/*								*/
																	/*								*/
	if( no == 0 ){													/*								*/
		ac_flg.syusyu = 32;											/* 32:印字完了					*/
		memcpy( ws, ts, sizeof( sky.tsyuk ) );						/* Ｔ集計をﾜｰｸｴﾘｱへ（停電対策）	*/
		ac_flg.syusyu = 33;											/* 33:Ｔ集計をﾜｰｸｴﾘｱへ転送完了	*/
																	/*								*/
		ts->Kinko_Tryo += coin_syu.Tryo;							/* Ｔ集計へ加算					*/
		ts->Coin_Tryo += coin_syu.Tryo;								/* Ｔ集計へ加算					*/
		for( i = 0; i < 4; i++ ){									/*								*/
			ts->Coin_cnt[i] += coin_syu.cnt[i];						/*								*/
			ts->Coin_ryo[i] += coin_syu.ryo[i];						/*								*/
		}															/*								*/
		ts->Ckinko_goukei_cnt += 1;									/* ｺｲﾝ金庫合計回数				*/
																	/*								*/
		//ts->kinko += coin_syu.Tryo;								/* Ｔ集計へ加算					*/
		//for( i = 0; i < 4; i++ ){									/*								*/
		//	ts->kin[i] += coin_syu.ryo[i];							/*								*/
		//}															/*								*/
																	/*								*/
		memcpy( cbk, &coin_syu, sizeof( coin_syu ) );				/* コイン金庫合計を前回コイン金庫合計へｾｯﾄ*/
		ac_flg.syusyu = 34;											/* 34:金庫集計をＴ集計へ転送完了*/
																	/*								*/
		Log_regist( LOG_COINBOX );									/* コイン金庫集計情報加算		*/
																	/*								*/
																	/*								*/
		memset( &SFV_DAT.safe_dt[0], 0, sizeof( SFV_DAT.safe_dt ));	/*								*/
		memcpy( &coin_syu.OldTime,									/*								*/
				&coin_syu.NowTime, sizeof( date_time_rec ) );		/* 前回集計時刻ｾｯﾄ				*/
		memset( &coin_syu.Tryo, 0, sizeof(ulong) * (1+(COIN_SYU_CNT*2)) );	/*				*/
																	/*								*/
		ac_flg.syusyu = 37;											/* 37:金庫集計ｸﾘｱ完了			*/
																	/*								*/
		CountUp( COIN_SAFE_COUNT );									/* ｺｲﾝ金庫追番+1				*/
																	/*								*/
	}																/*								*/
	else{															/*								*/
		ac_flg.syusyu = 42;											/* 42:印字完了					*/
		memcpy( ws, ts, sizeof( sky.tsyuk ) );						/* Ｔ集計をﾜｰｸｴﾘｱへ（停電対策）	*/
		ac_flg.syusyu = 43;											/* 43:Ｔ集計をﾜｰｸｴﾘｱへ転送完了	*/
																	/*								*/
		ts->Kinko_Tryo += note_syu.Tryo;							/* Ｔ集計へ加算					*/
		ts->Note_Tryo += note_syu.Tryo;								/* Ｔ集計へ加算					*/
		for( i = 0; i < 4; i++ ){									/*								*/
			ts->Note_cnt[i] += note_syu.cnt[i];						/*								*/
			ts->Note_ryo[i] += note_syu.ryo[i];						/*								*/
		}															/*								*/
		ts->Skinko_goukei_cnt += 1;									/* 紙幣金庫合計回数				*/
																	/*								*/
		memcpy( nbk, &note_syu, sizeof( note_syu ) );				/* 紙幣金庫合計を前回紙幣金庫合計へｾｯﾄ*/
		ac_flg.syusyu = 44;											/* 44:金庫集計をＴ集計へ転送完了*/
																	/*								*/
		Log_regist( LOG_NOTEBOX );									/* 紙幣金庫集計情報加算			*/
																	/*								*/
																	/*								*/
		SFV_DAT.nt_safe_dt = 0;										/*								*/
		memcpy( &note_syu.OldTime,									/*								*/
				&note_syu.NowTime, sizeof( date_time_rec ) );		/* 前回集計時刻ｾｯﾄ				*/
		memset( &note_syu.Tryo, 0, sizeof(ulong) * (1+(SIHEI_SYU_CNT*2)) );	/*			*/
																	/*								*/
		ac_flg.syusyu = 47;											/* 47:金庫集計ｸﾘｱ完了			*/
																	/*								*/
		CountUp( NOTE_SAFE_COUNT );									/* 紙幣金庫追番+1				*/
																	/*								*/
	}																/*								*/
																	/*								*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 集計ｴﾘｱ初期化																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: syu_init( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-12-12																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	syu_init( void )											/*								*/
{																	/*								*/
																	/*								*/
	memset( &sky, 0, sizeof( struct SKY ) );						/*								*/
	memset( &skybk, 0, sizeof( struct SKY ) );						/*								*/
	memset( &turi_kan, 0, sizeof( TURI_KAN ) );						/*								*/
	memset( &turi_dat, 0, sizeof( struct turi_dat_rec ) );			/*								*/
	memset( &fusei, 0, sizeof( struct FUSEI_SD ) );					/*								*/
	memset( &coin_syu, 0, sizeof( COIN_SYU ) );						/*								*/
	memset( &note_syu, 0, sizeof( NOTE_SYU ) );						/*								*/
	memset( &loktl, 0, sizeof( struct LOKTL ) );					/*								*/
	memset( &loktlbk, 0, sizeof( struct LOKTL ) );					/*								*/
	memset( &cobk_syu, 0, sizeof( cobk_syu ) );						/*								*/
	memset( &nobk_syu, 0, sizeof( nobk_syu ) );						/*								*/
	memset( &locktimeout, 0, sizeof( struct FUSEI_SD ) );			/*								*/
																	/*								*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| DISCOUNT_DATA → wari_tiket 割引データ変換													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: disc_wari_conv( void )														   |*/
/*| PARAMETER	: index	: 0 = PayData.DiscountDataインデックス値								   |*/
/*| 			  pdata	: 1 = wari_tiket型データポインタ										   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: m.nagashima																	   |*/
/*| Date		: 2012-02-09																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	disc_wari_conv( DISCOUNT_DATA *p_disc, wari_tiket *p_wari )
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
{
	disc_wari_conv_main(p_disc, p_wari, 0);	// 今回使用した割引を返す
}

void	disc_wari_conv_prev( DISCOUNT_DATA *p_disc, wari_tiket *p_wari )
{
	disc_wari_conv_main(p_disc, p_wari, 1);	// 前回精算までの使用済み割引を返す
}

void	disc_wari_conv_all( DISCOUNT_DATA *p_disc, wari_tiket *p_wari )
{
	disc_wari_conv_main(p_disc, p_wari, 2);	// 全ての割引を返す
}

// GG124100(S) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
void	disc_wari_conv_new( DISCOUNT_DATA *p_disc, wari_tiket *p_wari )
{
	disc_wari_conv_main(p_disc, p_wari, 3);	// 未使用割引を返す
}
// GG124100(E) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない

//[]----------------------------------------------------------------------[]
///	@brief			DISCOUNT_DATA → wari_tiket 割引データ変換(動作指定)
//[]----------------------------------------------------------------------[]
///	@param[in]		DISCOUNT_DATA	: PayData.DiscountDataポインタ
///	@param[in]		wari_tiket		: wari_tiket型データポインタ
///	@param[in]		uchar			: 動作フラグ<br>
///									  0 = 今回使用した割引を返す<br>
///									  1 = 前回精算までの使用済み割引を返す<br>
///									  2 = 全ての割引を返す<br>
// GG124100(S) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
///									  3 = 未使用割引を返す<br>
// GG124100(E) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
///	@return			void
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void disc_wari_conv_main( DISCOUNT_DATA *p_disc, wari_tiket *p_wari, uchar opeFlag )
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
{
	uchar	i;
	uchar	tik_syu;
	ulong	ParkingNo;
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	ulong	tmpDiscount = 0;
	ushort	tmpDiscCount = 0;
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]

	memset( p_wari, 0, sizeof( wari_tiket ) );

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	switch ( opeFlag ) {
	case 1:	// 前回精算までの使用済み割引を返す
		tmpDiscCount = p_disc->uDiscData.common.PrevUsageDiscCount;
		tmpDiscount  = p_disc->uDiscData.common.PrevUsageDiscount;
		tmpDiscount += p_disc->uDiscData.common.PrevDiscount;
		break;
	case 2:	// 全ての割引を返す
		tmpDiscCount  = p_disc->DiscCount;
		tmpDiscCount += p_disc->uDiscData.common.PrevUsageDiscCount;
		tmpDiscount  = p_disc->Discount;
		tmpDiscount += p_disc->uDiscData.common.PrevUsageDiscount;
		break;
// GG124100(S) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
// 	case 0:	// 今回使用した割引を返す
// 	default:
	case 3:	// 新規の割引を返す
		// 前回精算までの割引の未使用分は含めない
// GG124100(E) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
		tmpDiscCount = p_disc->DiscCount;
		tmpDiscount  = p_disc->Discount;
		tmpDiscount -= p_disc->uDiscData.common.PrevDiscount;
		break;
// GG124100(S) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
	case 0:	// 今回使用した割引を返す
	default:
		tmpDiscCount = p_disc->DiscCount;
		tmpDiscount  = p_disc->Discount;
		break;
// GG124100(E) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
	}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]

	switch( p_disc->DiscSyu ){
		case NTNET_SVS_T:			// ｻｰﾋﾞｽ券(料金)
		case NTNET_SVS_M:			// ｻｰﾋﾞｽ券(時間)
// MH810100(S) K.Onodera  2020/01/29 車番チケットレス(割引済み対応/再精算_領収証印字)
//			tik_syu = SERVICE;
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 			if( p_disc->uDiscData.common.DiscFlg == 0 ){
// 				tik_syu = SERVICE;
// 			}else{
// 				tik_syu = C_SERVICE;
// 			}
			if ( tmpDiscount || tmpDiscCount ) {
				if ( opeFlag == 1 ) {
					tik_syu = C_SERVICE;
				} else {
					tik_syu = SERVICE;
				}
			} else {
				return;	// データなし
			}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) K.Onodera  2020/01/29 車番チケットレス(割引済み対応/再精算_領収証印字)
			break;
		case NTNET_KAK_T:			// 店割引(料金)
		case NTNET_KAK_M:			// 店割引(時間)
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引データ変換の変更)
		case NTNET_TKAK_M:			// 多店舗(料金)
		case NTNET_TKAK_T:			// 多店舗(時間)
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引データ変換の変更)
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
//// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
//		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
//		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
//// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
// MH810100(S) K.Onodera  2020/01/29 車番チケットレス(割引済み対応/再精算_領収証印字)
//			tik_syu = KAKEURI;
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 			if( p_disc->uDiscData.common.DiscFlg == 0 ){
// 				tik_syu = KAKEURI;
// 			}else{
// 				tik_syu = C_KAKEURI;
// 			}
			if ( tmpDiscount || tmpDiscCount ) {
				if ( opeFlag == 1 ) {
					tik_syu = C_KAKEURI;
				} else {
					tik_syu = KAKEURI;
				}
			} else {
				return;	// データなし
			}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) K.Onodera  2020/01/29 車番チケットレス(割引済み対応/再精算_領収証印字)
			break;
		case NTNET_FRE:				// 回数券
			tik_syu = KAISUU;
			break;
		case NTNET_WRI_M:			// 割引券(料金)
		case NTNET_WRI_T:			// 割引券(時間)
			tik_syu = WARIBIKI;
			break;
		case NTNET_PRI_W:			// プリペイドカード(種別／枚数は参照不可)
			tik_syu = PREPAID;
			break;
		case NTNET_GENGAKU:			// 減額精算
			tik_syu = MISHUU;
			break;
		case NTNET_FURIKAE:			// 振替精算
			tik_syu = FURIKAE;
			break;
		case NTNET_SYUSEI_1:		// 修正精算
			tik_syu = SYUUSEI;
			break;
		case NTNET_CSVS_M:			// 精算中止ｻｰﾋﾞｽ券(料金)
		case NTNET_CSVS_T:			// 精算中止ｻｰﾋﾞｽ券(時間)
			tik_syu = C_SERVICE;
			break;
		case NTNET_CKAK_M:			// 精算中止店割引(料金)
		case NTNET_CKAK_T:			// 精算中止店割引(時間)
			tik_syu = C_KAKEURI;
			break;
		case NTNET_CWRI_M:			// 精算中止割引券(料金)
		case NTNET_CWRI_T:			// 精算中止割引券(時間)
			tik_syu = C_WARIBIKI;
			break;
		case NTNET_CFRE:			// 精算中止回数券
			tik_syu = C_KAISUU;
			break;
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 			if( p_disc->uDiscData.common.DiscFlg == 0 ){
// 				tik_syu = SHOPPING;
// 			}else{
// 				tik_syu = C_SHOPPING;
// 			}
			if ( tmpDiscount || tmpDiscCount ) {
				if ( opeFlag == 1 ) {
					tik_syu = C_SHOPPING;
				} else {
					tik_syu = SHOPPING;
				}
			} else {
				return;	// データなし
			}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			break;
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
// MH321800(S) Y.Tanizaki ICクレジット対応
//		default :
//			return;
// GG129000(S) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]
		case NTNET_SYUBET:		// 種別割引(金額)
		case NTNET_SYUBET_TIME:	// 種別割引(時間)
			if ( tmpDiscount || tmpDiscCount ) {
				if ( opeFlag == 1 ) {
					tik_syu = C_SYUBETU;
				} else {
					tik_syu = SYUBETU;
				}
			} else {
				return;	// データなし
			}
			break;
// GG129000(E) R.Endo 2022/10/25 車番チケットレス4.0 #6651 車種割引で割引余力を残し、再精算した場合の集計印字が不正 [共通改善項目 No1541]
		case	0:					// データなし
			return;
		default :					// 不明
			tik_syu = INVALID;
			break;
// MH321800(E) Y.Tanizaki ICクレジット対応
	}

	ParkingNo = p_disc->ParkingNo;
	for( i = 0; i < 4; i++ ){
		if( ParkingNo == CPrmSS[S_SYS][i+1] ){
			break;
		}
	}
	if( i > 3 ){
		i = 0;
	}
	p_wari->pkno_syu = i;						// 駐車場Ｎｏ．種別
	p_wari->tik_syu = tik_syu;					// 割引券種別
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引データ変換の変更)
// // MH322914(S) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
// 	p_wari->syubetu = (uchar)p_disc->DiscNo;	// 種別
// 	p_wari->syubetu = p_disc->DiscNo;			// 種別
// // MH322914(E) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
	p_wari->syubetu = p_disc->DiscNo;			// 種別(店№を格納)
	if ( (p_disc->DiscSyu == NTNET_TKAK_M) || (p_disc->DiscSyu == NTNET_TKAK_T) ) {
		p_wari->mise_no = p_disc->DiscInfo1;	// 店Ｎｏ．(多店舗割引種別を格納)
	}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(割引データ変換の変更)
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	p_wari->maisuu = (uchar)p_disc->DiscCount;	// 枚数
// 	p_wari->ryokin = p_disc->Discount;			// 割引料金
	p_wari->maisuu = (uchar)tmpDiscCount;				// 枚数
	p_wari->ryokin = tmpDiscount;				// 割引料金
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	if( tik_syu == SYUUSEI ){
		// 修正精算(振替額)は割引エリア2のDiscount
		p_wari->ryokin = (p_disc+1)->Discount;
	}
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
	if(tik_syu == SHOPPING || tik_syu == C_SHOPPING) {
		// 買物割引の割引適用No.を設定
		p_wari->minute = p_disc->DiscInfo1;
	}
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)

	return;
}
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
void	disc_Detail_conv( DETAIL_DATA *p_disc, wari_tiket *p_wari )
{
	uchar	i;
	uchar	tik_syu;
	ulong	ParkingNo;

	memset( p_wari, 0, sizeof( wari_tiket ) );
	switch( p_disc->DiscSyu ){
		case NTNET_FURIKAE_2:		// 振替精算
			tik_syu = FURIKAE2;
			break;
		case NTNET_FURIKAE_DETAIL:
			tik_syu = FURIKAE_DETAIL;
			break;
		default:
			return;
	}

	ParkingNo = p_disc->ParkingNo;
	for( i = 0; i < 4; i++ ){
		if( ParkingNo == CPrmSS[S_SYS][i+1] ){
			break;
		}
	}
	if( i > 3 ){
		i = 0;
	}
	p_wari->pkno_syu = i;										// 駐車場Ｎｏ．種別
	p_wari->tik_syu = tik_syu;									// 割引券種別
	p_wari->syubetu = (uchar)p_disc->uDetail.Common.DiscNo;		// 種別
	p_wari->maisuu = (uchar)p_disc->uDetail.Common.DiscCount;	// 枚数
	p_wari->ryokin = p_disc->uDetail.Common.DiscInfo1;			// 割引料金

	return;
}
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
//[]----------------------------------------------------------------------[]
///	@brief		リアルタイム情報での日付切替基準の総売上の小計取得機能の使用可否
//[]----------------------------------------------------------------------[]
///	@return		ret : 0:使用する  1:使用しない
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/08/30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar	date_uriage_use_chk( void )
{
	uchar	hour;
	uchar	min;
	uchar	ret;

	ret = 0;// 日付切替基準の総売上の小計取得を最初設定で設けていたが事業部指摘で処理固定(する)に変更
	hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// 基準時刻 時
	min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// 基準時刻 分

	if((23 < hour) || (59 < min)){
		ret = 1;//設定値が不正より使用しない
	}

	return( ret );
}

//[]----------------------------------------------------------------------[]
///	@brief		リアルタイム情報での日付切替基準の小計の時刻の更新判定
//[]----------------------------------------------------------------------[]
///	@return      ret : DATE_URIAGE_NO_CHANGE     範囲内（更新不要） 
///	                   DATE_URIAGE_CLR_TODAY     範囲外（更新必要：進んだ場合）→当日の基準日をセット+集計情報クリア
///	                   DATE_URIAGE_CLR_BEFORE    範囲外（更新必要：戻った場合）→前日の基準日をセット+集計情報クリア
///	                   DATE_URIAGE_PRMCNG_TODAY  設定変更より基準日更新（当日更新）→ 当日の基準日をセット(集計クリアしない)
///	                   DATE_URIAGE_PRMCNG_BEFORE 設定変更より基準日更新（前日更新）→ 前日の基準日をセット(集計クリアしない)
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/08/30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar	date_uriage_update_chk( void )
{
	uchar	update = 0;
	short	ck_year,ck_mon,ck_day,ck_hour,ck_min;
	uchar	settei_Hour,settei_Min,ck_flg;

// 本関数をコールする前に設定値の判定を行っているのでここでは行わない
	settei_Hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// 基準時刻 時
	settei_Min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// 基準時刻 分
	ck_flg = DATE_URIAGE_PRMCNG_NOCLR;	// 基準時刻変更時の集計クリア(しない固定) 最初設定で設けていたが事業部指摘で処理固定に変更
	
// 運用中に設定変更された時は基準日のみを更新する
	if( (Date_Syoukei.Date_Time.Hour != settei_Hour) || (Date_Syoukei.Date_Time.Min != settei_Min) ){
		if(CLK_REC.hour <  settei_Hour){
			if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//基準時刻変更時集計クリアしない
				update = DATE_URIAGE_PRMCNG_BEFORE;		// 集計クリアしないで、基準時刻更新（前日更新）
			}else{
				update = DATE_URIAGE_CLR_BEFORE;		// 集計クリアし、基準時刻更新（前日更新）
			}
		}else if(CLK_REC.hour ==  settei_Hour){
			// 時一致
			if(CLK_REC.minu < settei_Min){
				if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//基準時刻変更時集計クリアしない
					update = DATE_URIAGE_PRMCNG_BEFORE;	// 集計クリアしないで、基準時刻更新（前日更新）
				}else{
					update = DATE_URIAGE_CLR_BEFORE;	// 集計クリアし、基準時刻更新（前日更新）
				}
			}else{
				if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//基準時刻変更時集計クリアしない
					update = DATE_URIAGE_PRMCNG_TODAY;	// 集計クリアしないで、基準時刻更新（当日更新）
				}else{
					update = DATE_URIAGE_CLR_TODAY;		// 集計クリアし、基準時刻更新（当日更新）
				}
			}
		}else{
			if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//基準時刻変更時集計クリアしない
				update = DATE_URIAGE_PRMCNG_TODAY;		// 集計クリアしないで、基準時刻更新（当日更新）
			}else{
				update = DATE_URIAGE_CLR_TODAY;			// 集計クリアし、基準時刻更新（当日更新）
			}
		}
		return( update );
	}

// 現在時刻が更新基準日より前の判定（時刻変更で更新基準日より前に戻された時）
	if(CLK_REC.year < Date_Syoukei.Date_Time.Year){
		update = DATE_URIAGE_CLR_TODAY;// 過去のため更新
	}else if(CLK_REC.year == Date_Syoukei.Date_Time.Year){
		// 年一致
		if(CLK_REC.mont < Date_Syoukei.Date_Time.Mon){
			update = DATE_URIAGE_CLR_TODAY;// 過去のため更新
		}else if(CLK_REC.mont == Date_Syoukei.Date_Time.Mon){
			// 月一致
			if(CLK_REC.date < Date_Syoukei.Date_Time.Day){
				update = DATE_URIAGE_CLR_TODAY;// 過去のため更新
			}else if(CLK_REC.date == Date_Syoukei.Date_Time.Day){
				// 日一致
				if(CLK_REC.hour < Date_Syoukei.Date_Time.Hour){
					update = DATE_URIAGE_CLR_BEFORE;// 過去のため更新（前日更新）
				}else if(CLK_REC.hour == Date_Syoukei.Date_Time.Hour){
					// 時一致
					if(CLK_REC.minu < Date_Syoukei.Date_Time.Min){
						update = DATE_URIAGE_CLR_BEFORE;// 過去のため更新（前日更新）
					}
				}
			}
		}
	}
	
	if(update == DATE_URIAGE_CLR_TODAY){
		// 年／月／日のいずれかが過去の場合でも、時／分の値によって当日か前日のどちらに更新するか判定必要
		// 時／分も過去の場合は前日更新でそれ以外は当日更新
		if(CLK_REC.hour < Date_Syoukei.Date_Time.Hour){
				update = DATE_URIAGE_CLR_BEFORE;// 過去のため更新（前日更新）
		}else if(CLK_REC.hour == Date_Syoukei.Date_Time.Hour){
			// 時一致
			if(CLK_REC.minu < Date_Syoukei.Date_Time.Min){
					update = DATE_URIAGE_CLR_BEFORE;// 過去のため更新（前日更新）
			}
		}
	}
	if(update != DATE_URIAGE_NO_CHANGE){
		return( update );// 更新基準日より前に戻されていることによる範囲外
	}

// 現在時刻が更新基準日に対して24時間以上経過したかチェック
	ck_year = Date_Syoukei.Date_Time.Year;
	ck_mon =  Date_Syoukei.Date_Time.Mon;
	ck_day =  Date_Syoukei.Date_Time.Day;
	ck_hour = Date_Syoukei.Date_Time.Hour;
	ck_min =  Date_Syoukei.Date_Time.Min;

	ck_hour += 24;//更新基準日に24時間加算
	// 24h加算したことによる補正処理
	if(ck_hour >= 24){
		ck_day++;//1日加算
		ck_hour = ck_hour % 24;
		for ( ; ; ) {
			if (ck_day <= medget(ck_year, ck_mon)) {// 月末以下
				break;
			}
			// 該当月の日数を引いて月を加算
			ck_day -= medget(ck_year, ck_mon);
			if (++ck_mon > 12) {
				ck_mon = 1;
				ck_year++;
			}
		}
	}

	if(CLK_REC.year > ck_year){
		update = DATE_URIAGE_CLR_BEFORE;// 更新基準日到達（当日／前日判定有り）
	}else if(CLK_REC.year == ck_year){
		// 年一致
		if(CLK_REC.mont > ck_mon){
			update = DATE_URIAGE_CLR_BEFORE;// 更新基準日到達（当日／前日判定有り）
		}else if(CLK_REC.mont == ck_mon){
			// 月一致
			if(CLK_REC.date > ck_day){
				update = DATE_URIAGE_CLR_BEFORE;// 更新基準日到達（当日／前日判定有り）
			}else if(CLK_REC.date == ck_day){
				// 日一致
				if(CLK_REC.hour > ck_hour){
					update = DATE_URIAGE_CLR_TODAY;// 更新基準日到達（当日更新確定）
				}else if(CLK_REC.hour == ck_hour){
					// 時一致
					if(CLK_REC.minu >= ck_min){
						update = DATE_URIAGE_CLR_TODAY;// 更新基準日到達（当日更新確定）
					}
				}
			}
		}
	}
	
	if(update == DATE_URIAGE_CLR_BEFORE){
		// 年／月／日のいずれかが到達しても、時／分の値によって当日か前日のどちらに更新するか判定必要
		// 時／分も過去の場合は前日更新でそれ以外は当日更新
		if(CLK_REC.hour <  ck_hour){
				update = DATE_URIAGE_CLR_BEFORE;// 過去のため更新（前日更新）
		}else if(CLK_REC.hour ==  ck_hour){
			// 時一致
			if(CLK_REC.minu < ck_min){
				update = DATE_URIAGE_CLR_BEFORE;// 過去のため更新（前日更新）
			}else{
				update = DATE_URIAGE_CLR_TODAY;// 当日更新
			}
		}else{
			update = DATE_URIAGE_CLR_TODAY;// 当日更新
		}
	}
	return( update );
}
//[]----------------------------------------------------------------------[]
///	@brief		リアルタイム情報での日付切替基準小計の時刻の更新判定
//[]----------------------------------------------------------------------[]
///	@return	
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/09/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	date_uriage_syoukei_judge( void )
{
	uchar	flg;
	if(date_uriage_use_chk() != 0){// 日付切替基準の総売上の小計取得しない
		return;
	}
	
	flg = date_uriage_update_chk();
	
	date_uriage_syoukei_clr(flg);// 集計情報のクリア、設定の変更
}

//[]----------------------------------------------------------------------[]
///	@brief		リアルタイム情報での日付切替基準の時刻の更新処理
//[]----------------------------------------------------------------------[]
///	@param[in]  uchar flg: DATE_URIAGE_NO_CHANGE 更新不要
///	                       DATE_URIAGE_CLR_TODAY:当日の基準日をセット+集計情報クリア
///	                       DATE_URIAGE_CLR_BEFORE:前日の基準日をセット+集計情報クリア
///	                       DATE_URIAGE_PRMCNG_TODAY:当日の基準日をセット(集計クリアしない)
///	                       DATE_URIAGE_PRMCNG_BEFORE:前日の基準日をセット(集計クリアしない)
///	@return	
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/08/30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	date_uriage_syoukei_clr( uchar flg)
{
	uchar	hour;
	uchar	min;
	ushort	us_wk;
	short	s_wky, s_wkm, s_wkd;

	// 本関数をコールする前に設定値の判定を行っているのでここでは行わない
	hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// 基準時刻 時
	min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// 基準時刻 分

	if((flg == DATE_URIAGE_CLR_TODAY) || (flg == DATE_URIAGE_PRMCNG_TODAY)){// 当日の基準日をセット
		if(flg == DATE_URIAGE_CLR_TODAY){
			memset( &Date_Syoukei, 0, sizeof( Date_Syoukei ) );//集計情報をクリア
		}
		Date_Syoukei.Date_Time.Year = CLK_REC.year;
		Date_Syoukei.Date_Time.Mon = CLK_REC.mont;
		Date_Syoukei.Date_Time.Day = CLK_REC.date;
		Date_Syoukei.Date_Time.Hour = hour;
		Date_Syoukei.Date_Time.Min = min;
	}else if((flg == DATE_URIAGE_CLR_BEFORE) || (flg == DATE_URIAGE_PRMCNG_BEFORE)){// 前日の基準日をセット
		if(flg == DATE_URIAGE_CLR_BEFORE){
			memset( &Date_Syoukei, 0, sizeof( Date_Syoukei ) );//集計情報をクリア
		}
		us_wk = dnrmlzm((short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date );// 年月日ノーマライズ
		us_wk--;// -1日更新
		idnrmlzm( us_wk, &s_wky, &s_wkm, &s_wkd );// 逆ノーマライズし1日更新した日付を求める
		Date_Syoukei.Date_Time.Year = (ushort)s_wky;
		Date_Syoukei.Date_Time.Mon  = (uchar)s_wkm;
		Date_Syoukei.Date_Time.Day  = (uchar)s_wkd;
		Date_Syoukei.Date_Time.Hour = hour;
		Date_Syoukei.Date_Time.Min = min;
	}else{
		//DATE_URIAGE_NO_CHANGEの時は何もしない
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		リアルタイム情報での日付切替基準の小計の時刻と精算時刻の比較
//[]----------------------------------------------------------------------[]
///	@return		ret : 0:範囲内（加算可）  1:範囲外（加算不可）
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/09/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar	date_uriage_cmp_paymentdate( void )
{
	uchar	update = 0;
// 日付切替基準時刻前に精算を開始したが、精算完了時点で日付切替基準時刻を過ぎていた場合
// 日付切替基準の小計に加算させないための判定

// 精算時刻と日付切替基準時刻を比較する
	if(car_ot.year < Date_Syoukei.Date_Time.Year){
		update = 1;// 過去のため加算不可
	}else if(CLK_REC.year == Date_Syoukei.Date_Time.Year){
		// 年一致
		if(car_ot.mon < Date_Syoukei.Date_Time.Mon){
			update = 1;// 過去のため加算不可
		}else if(car_ot.mon == Date_Syoukei.Date_Time.Mon){
			// 月一致
			if(car_ot.day < Date_Syoukei.Date_Time.Day){
				update = 1;// 過去のため加算不可
			}else if(car_ot.day == Date_Syoukei.Date_Time.Day){
				// 日一致
				if(car_ot.hour < Date_Syoukei.Date_Time.Hour){
					update = 1;// 過去のため加算不可
				}else if(car_ot.hour == Date_Syoukei.Date_Time.Hour){
					// 時一致
					if(car_ot.min < Date_Syoukei.Date_Time.Min){
						update = 1;// 過去のため加算不可
					}
				}
			}
		}
	}
	return( update );
}
//[]----------------------------------------------------------------------[]
///	@brief		リアルタイム情報での日付切替基準時刻の設定変更が行われたときの設定更新処理
//[]----------------------------------------------------------------------[]
///	@return	
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/12/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	date_uriage_prmcng_judge( void )
{
	uchar	update = DATE_URIAGE_NO_CHANGE;
	uchar	settei_Hour,settei_Min,ck_flg;
	if(date_uriage_use_chk() != 0){// 日付切替基準の総売上の小計取得しない
		return;
	}

	settei_Hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// 基準時刻 時
	settei_Min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// 基準時刻 分
	ck_flg = DATE_URIAGE_PRMCNG_NOCLR;	// 基準時刻変更時の集計クリア(しない固定) 最初設定で設けていたが事業部指摘で処理固定に変更

	// 運用中に設定変更された時の基準日更新判定
	// 「基準時刻変更時集計クリアしない」設定の場合、設定変更した時点で基準時刻を更新しないと
	// 設定変更後、長時間精算が発生しない状態で設定変更後の基準時刻に到達した場合、集計クリアがされないための対応
	if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//基準時刻変更時集計クリアしない
		// 設定が変更されていた場合はその時点で基準時刻を更新する
		if( (Date_Syoukei.Date_Time.Hour != settei_Hour) || (Date_Syoukei.Date_Time.Min != settei_Min) ){
			if(CLK_REC.hour <  settei_Hour){
				update = DATE_URIAGE_PRMCNG_BEFORE;		// 集計クリアしないで、基準時刻更新（前日更新）
			}else if(CLK_REC.hour ==  settei_Hour){
				// 時一致
				if(CLK_REC.minu < settei_Min){
					update = DATE_URIAGE_PRMCNG_BEFORE;	// 集計クリアしないで、基準時刻更新（前日更新）
				}else{
					update = DATE_URIAGE_PRMCNG_TODAY;	// 集計クリアしないで、基準時刻更新（当日更新）
				}
			}else{
				update = DATE_URIAGE_PRMCNG_TODAY;		// 集計クリアしないで、基準時刻更新（当日更新）
			}
			date_uriage_syoukei_clr(update);// 基準時刻の更新
		}
	}

}
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
//// MH321800(S) G.So ICクレジット対応
///*[]----------------------------------------------------------------------------------------------[]*/
///*| アラーム取引登録																			   |*/
///*[]----------------------------------------------------------------------------------------------[]*/
///*| MODULE NAME	: ec_alarm_syuu( uchar brand, ulong ryo )										   |*/
///*| PARAMETER	: brand	: 使用ブランド															   |*/
///*| 			  ryo	: アラーム取引額														   |*/
///*| RETURN VALUE: void	: 																		   |*/
///*[]----------------------------------------------------------------------------------------------[]*/
///*| Author		: G.So																			   |*/
///*| Date		: 2019-01-29																	   |*/
///*| Update		:																				   |*/
///*[]------------------------------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
//void	ec_alarm_syuu( uchar brand, ulong ryo )
//{
//	SYUKEI	*ts, *ws;
//// MH810103 GG119202(S) アラーム取引回数は当日にカウントする
////	struct clk_rec wrk_clk_rec;		/* 集計日時ノーマライズ用 */
////	date_time_rec wrk_date;			/* 集計日時ノーマライズ用 */
//// MH810103 GG119202(E) アラーム取引回数は当日にカウントする
//// MH810105 GG119202(S) 処理未了取引集計仕様改善
////	int		ec_kind;
//// MH810105 GG119202(E) 処理未了取引集計仕様改善
//// MH810103 GG119202(S) アラーム取引回数は当日にカウントする
////	int		syu_idx;				/* 日毎集計のインデックス */
////	ulong	sei_date;				/* 精算日時 */
////	ulong	sy1_date;				/* 集計日時 */
////	memset(&wrk_clk_rec, 0x0, sizeof(wrk_clk_rec));
//// MH810103 GG119202(E) アラーム取引回数は当日にカウントする
//
//	EcAlarm.Ryo = ryo;											// 復電用アラーム取引額
//	EcAlarm.Brand = brand;										// 復電用アラーム取引ブランド
//
//	ac_flg.ec_alarm = 1;										/* 1:アラーム取引情報を保存完了 */
//
//	ts = &sky.tsyuk;												/*								*/
//	ws = &wksky;													/*								*/
//	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* Ｔ集計をﾜｰｸｴﾘｱへ(停電対策)	*/
//
//	DailyAggregateDataBKorRES( 0 );									/* 日毎集計エリアのバックアップ	*/
//
//	ac_flg.ec_alarm = 2;											/* 2:集計をﾜｰｸｴﾘｱへ転送完了	*/
//
//// MH810105 GG119202(S) 処理未了取引集計仕様改善
////// 集計は既存エリアがあればそれを使用するので、それぞれに加算する。
////	switch(brand) {
////	case EC_EDY_USED:
////		ts->Electron_Arm_cnt += 1L;									// 精算＜件数＞
////		ts->Electron_Arm_ryo += ryo;								//     ＜金額＞
////		break;
////	case EC_NANACO_USED:
////		ts->nanaco_alm_cnt += 1L;									// 精算＜件数＞
////		ts->nanaco_alm_ryo += ryo;									//     ＜金額＞
////		break;
////	case EC_WAON_USED:
////		ts->waon_alm_cnt += 1L;										// 精算＜件数＞
////		ts->waon_alm_ryo += ryo;									//     ＜金額＞
////		break;
////	case EC_SAPICA_USED:
////		ts->sapica_alm_cnt += 1L;									// 精算＜件数＞
////		ts->sapica_alm_ryo += ryo;									//     ＜金額＞
////		break;
////	case EC_KOUTSUU_USED:
////		ts->koutsuu_alm_cnt += 1L;									// 精算＜件数＞
////		ts->koutsuu_alm_ryo += ryo;									//     ＜金額＞
////		break;
////	case EC_ID_USED:
////		ts->id_alm_cnt += 1L;										// 精算＜件数＞
////		ts->id_alm_ryo += ryo;										//     ＜金額＞
////		break;
////	case EC_QUIC_PAY_USED:
////		ts->quicpay_alm_cnt += 1L;									// 精算＜件数＞
////		ts->quicpay_alm_ryo += ryo;									//     ＜金額＞
////		break;
////	case EC_CREDIT_USED:
////		// クレジットでアラーム取引は無い
////		break;
////	default:
////		// ありえない
////		ac_flg.ec_alarm = 0;
////		return;
////	}
//	// 処理未了取引（支払不明）集計
//	ts->miryo_unknown_cnt += 1L;
//	ts->miryo_unknown_ryo += ryo;
//
//	// 決済結果データ（決済未了確定）には決済日時がセットされないため、日毎集計は当日としてカウントする
//	Syuukei_sp.ec_inf.now.sp_miryo_unknown_cnt += 1L;
//	Syuukei_sp.ec_inf.now.sp_miryo_unknown_ryo += ryo;
//// MH810105 GG119202(E) 処理未了取引集計仕様改善
//// MH810103 GG119202(S) アラーム取引回数は当日にカウントする
////	// 最新の通知日時から集計日時を算出
////	memcpy(&wrk_date, &Syuukei_sp.ec_inf.NowTime, sizeof(date_time_rec));
////	wrk_date.Hour = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 3);			/* 時は共通パラメータから取得 */
////	wrk_date.Min = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 1);				/* 分は共通パラメータから取得 */
////	sy1_date = Nrm_YMDHM(&wrk_date);									/* 集計日時をノーマライズ */
////
////	// 精算データから（決済結果データで受信した）精算日時を算出
////	c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &wrk_clk_rec);
////	memcpy(&wrk_date, &wrk_clk_rec, sizeof(date_time_rec));
////	sei_date = Nrm_YMDHM(&wrk_date);									/* 精算日時をノーマライズ */
////
////	ec_kind = brand - EC_EDY_USED;
////	if(sei_date >= sy1_date && sei_date < (sy1_date + 0x10000)) {
////		// 精算日時が当日の集計日時範囲
////		Syuukei_sp.ec_inf.now.cnt2[ec_kind] += 1L;						/* 使用回数				*/
////		Syuukei_sp.ec_inf.now.ryo2[ec_kind] += ryo;						/* 使用金額				*/
////	}
////	else if(sei_date >= (sy1_date + 0x10000) && sei_date < (sy1_date + 0x20000)) {
////		// 精算日時が翌日の集計日時範囲
////		Syuukei_sp.ec_inf.next.cnt2[ec_kind] += 1L;						/* 使用回数				*/
////		Syuukei_sp.ec_inf.next.ryo2[ec_kind] += ryo;					/* 使用金額				*/
////	}
////	else if(sei_date >= (sy1_date - 0x10000) && sei_date < sy1_date) {
////		// 精算日時が前日の集計日時範囲
////		if(Syuukei_sp.ec_inf.cnt == 1) {
////			// 前日の集計がないため作成する
////			UnNrm_YMDHM(&wrk_date, (sy1_date - 0x10000));
////			memcpy(&Syuukei_sp.ec_inf.bun[Syuukei_sp.ec_inf.ptr].SyuTime, &wrk_date, sizeof(date_time_rec));
////			Syuukei_sp.ec_inf.ptr++;
////			Syuukei_sp.ec_inf.cnt++;
////		}
////		syu_idx = (Syuukei_sp.ec_inf.ptr == 0) ? (SYUUKEI_DAY_EC-1) : (Syuukei_sp.ec_inf.ptr-1);
////		Syuukei_sp.ec_inf.bun[syu_idx].cnt2[ec_kind] += 1L;				/* 使用回数				*/
////		Syuukei_sp.ec_inf.bun[syu_idx].ryo2[ec_kind] += ryo;			/* 使用金額				*/
////	} else {
////		// 精算日時が翌日から前日の範囲外の場合はアラームを登録して当日に加算
////		Syuukei_sp.ec_inf.now.cnt2[ec_kind] += 1L;						/* 使用回数				*/
////		Syuukei_sp.ec_inf.now.ryo2[ec_kind] += ryo;						/* 使用金額				*/
////		Syuukei_sp.ec_inf.now.unknownTimeCnt++;
////
////		// アラーム登録(決済リーダの決済時刻と精算機の時刻に1日以上のずれがある)
////		alm_chk( ALMMDL_SUB, ALARM_EC_SETTLEMENT_TIME_GAP, 2 );
////	}
//// MH810105 GG119202(S) 処理未了取引集計仕様改善
////	// 決済結果データ（決済未了確定）には決済日時がセットされないため、日毎集計は当日としてカウントする
////	ec_kind = brand - EC_EDY_USED;
////	Syuukei_sp.ec_inf.now.cnt2[ec_kind] += 1L;							/* 使用回数				*/
////	Syuukei_sp.ec_inf.now.ryo2[ec_kind] += ryo;							/* 使用金額				*/
//// MH810105 GG119202(E) 処理未了取引集計仕様改善
//// MH810103 GG119202(E) アラーム取引回数は当日にカウントする
//
//	ac_flg.ec_alarm = 0;												/* 集計加算完了			*/
//
//	return;
//}
//// MH321800(E) G.So ICクレジット対応
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更

// MH810105 GG119202(S) T合計連動印字対応
//[]----------------------------------------------------------------------[]
///	@brief			T合計連動印字
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_req		: 印字種別（T合計／前回T合計／自動集計）
///	@param[in]		pFrmSyuukei	: Ｔ合計印字データフォーマットポインタ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/08/30
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void ec_linked_total_print(ushort pri_req, T_FrmSyuukei *pFrmSyuukei)
{
	T_FrmSyuukeiEc	FrmSyuk2;
	SYUKEI			*syu = pFrmSyuukei->prn_data;
	date_time_rec	oldtime, nowtime, wk_time;
	ushort			LogCount;
	ushort			FstIdx, LstIdx;

	// T_FrmSyuukeiEcはT_FrmSyuukei+T_FrmTgoukeiEcの構造体
	memset(&FrmSyuk2, 0, sizeof(FrmSyuk2));
	memcpy(&FrmSyuk2, pFrmSyuukei, sizeof(*pFrmSyuukei));

	if (prm_get(COM_PRM, S_ECR, 5, 1, 3) == 1) {
		// みなし決済プリント
		FrmSyuk2.syuk.print_flag |= TGOUKEI_EC_MINASHI;
		FrmSyuk2.Ec.Minashi.prn_kind = R_PRI;						// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
		LogCount = Ope2_Log_NewestOldestDateGet_AttachOffset(LOG_ECMINASHI,
								&nowtime, &oldtime, &FstIdx, &LstIdx);
		if (syu->OldTime.Year != 0 ||
			syu->OldTime.Mon != 0 ||
			syu->OldTime.Day != 0 ||
			syu->OldTime.Hour != 0 ||
			syu->OldTime.Min != 0) {
			// 前回集計日時～今回集計日時を検索範囲とする
			FrmSyuk2.Ec.Minashi.BType = 2;							// 検索方法：期間
			memcpy(&FrmSyuk2.Ec.Minashi.TSttTime, &syu->OldTime,
					sizeof(FrmSyuk2.Ec.Minashi.TSttTime));			// 開始日時
			memcpy(&FrmSyuk2.Ec.Minashi.TEndTime, &syu->NowTime,
					sizeof(FrmSyuk2.Ec.Minashi.TEndTime));			// 終了日時
		}
		else {
			if (pri_req == PREQ_TGOUKEI_Z) {
				// 前回集計日時がオール0の場合は
				// ノーマライズ基点+1日～今回集計日時を検索範囲とする（前回T合計）
				FrmSyuk2.Ec.Minashi.BType = 2;						// 検索方法：期間
				FrmSyuk2.Ec.Minashi.TSttTime.Year = 1980;			// 開始日時（1980/3/2 00:00）
				FrmSyuk2.Ec.Minashi.TSttTime.Mon = 3;
				FrmSyuk2.Ec.Minashi.TSttTime.Day = 2;
				FrmSyuk2.Ec.Minashi.TSttTime.Hour = 0;
				FrmSyuk2.Ec.Minashi.TSttTime.Min = 0;
				memcpy(&FrmSyuk2.Ec.Minashi.TEndTime, &syu->NowTime,
						sizeof(FrmSyuk2.Ec.Minashi.TEndTime));		// 終了日時
			}
			else {
				// 前回集計日時がオール0の場合は全検索とする（T合計）
				FrmSyuk2.Ec.Minashi.BType = 0;						// 検索方法：全て
				memcpy(&FrmSyuk2.Ec.Minashi.TSttTime, &oldtime,
						sizeof(FrmSyuk2.Ec.Minashi.TSttTime));		// 開始日時
				memcpy(&FrmSyuk2.Ec.Minashi.TEndTime, &nowtime,
						sizeof(FrmSyuk2.Ec.Minashi.TEndTime));		// 終了日時
			}
		}
		FrmSyuk2.Ec.Minashi.LogCount = LogCount;					// みなし決済ログ件数
		FrmSyuk2.Ec.Minashi.Ffst_no = FstIdx;						// 検索条件が最初に一致したFlashROMの情報番号
		FrmSyuk2.Ec.Minashi.Flst_no = LstIdx;						// 検索条件が最後に一致したFlashROMの情報番号
	}
	if (prm_get(COM_PRM, S_ECR, 5, 1, 2) == 1) {
		// 処理未了取引記録
		FrmSyuk2.syuk.print_flag |= TGOUKEI_EC_ALARM;
		FrmSyuk2.Ec.Alarm.prn_kind = R_PRI;							// 対象ﾌﾟﾘﾝﾀ：ﾚｼｰﾄ
		if (syu->OldTime.Year != 0 ||
			syu->OldTime.Mon != 0 ||
			syu->OldTime.Day != 0 ||
			syu->OldTime.Hour != 0 ||
			syu->OldTime.Min != 0) {
			// 前回集計日時～今回集計日時を検索範囲とする
			FrmSyuk2.Ec.Alarm.StartTime = Nrm_YMDHM((date_time_rec*)&syu->OldTime);	// 開始日時
			FrmSyuk2.Ec.Alarm.EndTime = Nrm_YMDHM((date_time_rec*)&syu->NowTime);	// 終了日時
		}
		else {
			if(pri_req == PREQ_TGOUKEI_Z){
				// 前回集計日時がオール0の場合は
				// ノーマライズ基点+1日～今回集計日時を検索範囲とする（前回T合計）
				wk_time.Year = 1980;								// 開始日時（1980/3/2 00:00）
				wk_time.Mon = 3;
				wk_time.Day = 2;
				wk_time.Hour = 0;
				wk_time.Min = 0;
				FrmSyuk2.Ec.Alarm.StartTime = Nrm_YMDHM((date_time_rec*)&wk_time);
				FrmSyuk2.Ec.Alarm.EndTime = Nrm_YMDHM((date_time_rec*)&syu->NowTime);	// 終了日時
			}
			else{
				// 前回集計日時がオール0の場合は全検索とする（T合計）
			}
		}
	}
	queset(PRNTCBNO, pri_req, sizeof(FrmSyuk2), &FrmSyuk2);
}
// MH810105 GG119202(E) T合計連動印字対応
// GM849100(S) M.Fujikawa 2025/03/11 名鉄協商コールセンター対応 コードチェック#257073
ulong set_parking_time( date_time_rec *InTime, date_time_rec *OutTime )
{

	ulong			indate;					// 入庫日
	ulong			outdate;				// 出庫日
	ulong			m_intime;				// 入庫日時（分換算）
	ulong			m_outtime;				// 出庫日時（分換算）
	ulong			parktime = 0L;			// 駐車時間（分換算）


	if( InTime->Year != 0 ) {
		// 入庫日計算
		indate	= dnrmlzm( (short)InTime->Year, (short)InTime->Mon, (short)InTime->Day );
		// 出庫日計算
		outdate	= dnrmlzm( (short)OutTime->Year, (short)OutTime->Mon, (short)OutTime->Day	);

		// 入庫日時（分換算）計算
		m_intime = (indate*24*60) + (InTime->Hour*60) + (InTime->Min);
		// 出庫日時（分換算）計算
		m_outtime = (outdate*24*60) + (OutTime->Hour*60) + (OutTime->Min);

		// 出庫日時≧入庫日時
		if( m_outtime >= m_intime ) {
			parktime = m_outtime - m_intime;		// 駐車時間取得（分換算）

			// 算出した駐車時間は９９９９９９分以上
			if( parktime > 999999L ) {
				parktime = 999999L;				// ９９９９９９分でセット
			} 
		// 出庫日時＜入庫日時
		} else {
			parktime = 0L;					// ０分でセット
		}
	// 入庫日時なし
	} else {
		parktime = 0L;			// 駐車時刻を０分でセット
	}

	return( parktime );

}
// GM849100(E) M.Fujikawa 2025/03/11 名鉄協商コールセンター対応 コードチェック#257073
