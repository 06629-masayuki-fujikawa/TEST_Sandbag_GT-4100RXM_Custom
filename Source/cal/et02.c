/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：料金計算処理　　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et02()　　　　　　　　　　：　料金計算を計算要求パラメータにより行う、但し割引処理において：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：は、０円より大きい場合のみ行う。　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　又、定期（時間帯定期）処理後は、要求内容を定期処理後料金計算：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：テーブルにセットする。　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：無し　　　　　　　　　　　　無し　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
#define		CARD		0x01											/*　カードリーダーシャッタービット　　　　　　*/
#define		BILL		0x02											/*　紙幣リーダーシャッタービット　　　　　　　*/
#define		EXPP		0x04											/*　拡張プリペイドカードシャッタービット　　　*/
#define		B_OUT		0x08											/*　紙幣払い出しシャッタービット　　　　　　　*/
#define		COIN		0x10											/*　コイン受付シャッタービット　　　　　　　　*/
#define		ALL			0x1f											/*　シャッター全部ビット　　　　　　　　　　　*/
#define		OPEN		0												/*　開要求　　　　　　　　　　　　　　　　　　*/
#define		CLOSE		1												/*　閉要求　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et02()															/*　料金計算要求テーブル解析処理　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short 	parameter;													/*　パラメータ　　　　　　　　　　　　　　　　*/
	char	flg = 0;
	long	taxable	;													/* 被消費税額								  */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	parameter	= (short)req_rkn.param;									/*　計算要求テーブルよりパラメータを参照　　　*/
	switch( parameter )													/*　パラメータにより分類　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_TSA:														/*　料金計算要求の場合　　　　　　　　　　　　*/
		memcpy( &carin_mt,	&car_in, 7 );								/*　入庫時刻を基本入庫時刻にセット　　　　　　*/
		memcpy( &carot_mt,	&car_ot, 7 );								/*　出庫時刻を基本出庫時刻にセット　　　　　　*/
		memcpy( &org_in,	&car_in, 7 );								/*　入庫時刻を基本入庫時刻（大元）にセット　　*/
		memcpy( &org_ot,	&car_ot, 7 );								/*　出庫時刻を基本出庫時刻（大元）にセット　　*/
		memcpy( &nmax_in,	&org_in, 7 );								// 実際の入庫時刻をＮ時間基点時刻バッファへ格納
		et40();															/*　料金計算処理　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_PKC:														/*　定期券処理要求の場合　　　　　　　　　　　*/
	case RY_PKC_K:														/*　拡張定期券処理要求　　　　　　　		  */
	case RY_PKC_K2:														/*　拡張2定期券処理要求　　　　　　　		  */
	case RY_PKC_K3:														/*　拡張3定期券処理要求　　　　　　　		  */
		tik_syubet = (char)req_rkn.data[0];								// 定期種別ｾｯﾄ
		memcpy( &nmax_in,	&org_in, 7 );								// 実際の入庫時刻をＮ時間基点時刻バッファへ格納
		memcpy( &nmax_orgin,&org_in,7 );								// 
		et47();												 			/*　定期券処理　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_KCH:														/*　車種切り換え要求の場合　　　　　　　　　　*/
		memcpy( &car_in,   &org_in, 7 );								/*　入庫時刻を基本入庫時刻（大元）にセット　　*/
		memcpy( &car_ot,   &org_ot, 7 );								/*　出庫時刻を基本出庫時刻（大元）にセット　　*/
		memcpy( &carin_mt, &car_in, 7 );								/*　入庫時刻を基本入庫時刻にセット　　　　　　*/
		memcpy( &carot_mt, &car_ot, 7 );								/*　出庫時刻を基本出庫時刻にセット　　　　　　*/
		memcpy( &nmax_in,	&org_in, 7 );								// 実際の入庫時刻をＮ時間基点時刻バッファへ格納
		memcpy( &nmax_orgin,&org_in,7 );								// 
		et57();															/*　車種切り換え処理　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_KGK:														/*　料金差引割引要求の場合　　　　　　　　　　*/
		et59();															/*　料金差引割引　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_PCO:														/*　プリペイドカード処理要求の場合　　　　　　*/
	case RY_CPP:														/*　精算中止プリペイドカード処理　　　　　　　*/
	case RY_PCO_K:														/*　拡張プリペイドカード処理要求　　		  */
	case RY_PCO_K2:														/*　拡張2プリペイドカード処理要求　　		  */
	case RY_PCO_K3:														/*　拡張3プリペイドカード処理要求　　		  */
		if(( ! ryoukin )&&((tki_ken == 0)||(tki_flg == OFF)))	flg = 1;	/*　料金０円の場合　　　　　　　　　　　　*/
		else			et46();											/*　料金０円でない場合　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_SKC:														/*　サービス券処理要求の場合　サービス券処理　*/
	case RY_SKC_K:														/*　拡張サービス券処理要求　　　　　		  */
	case RY_SKC_K2:														/*　拡張2サービス券処理要求　　　　　		  */
	case RY_SKC_K3:														/*　拡張3サービス券処理要求　　　　　		  */
		et45();															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_CSK:														/*　精算中止サービス券処理要求の場合　中止Ｓ券*/
		et45();															/*　　　　　　　　　　　　　　　　　　　　　　*/
		flg = 2;
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
	case RY_FKN:														/*　紛失券処理要求の場合　　　　　　　　　　　*/
		et48();															/*　紛失券処理　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
	case RY_KAK:														/*　掛け売り券処理　　　　　　　　  		  */
	case RY_KAK_K:														/*　拡張1掛売券(店№割引)		　  		  */
	case RY_KAK_K2:														/*　拡張2掛売券(店№割引)	　  		  	  */
	case RY_KAK_K3:														/*　拡張3掛売券(店№割引)					  */
	case RY_SNC:														/*　店№割引要求の場合　店№割引処理　　　　　*/
		et51();															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_EMY:														/*　電子マネー処理要求			　　		  */
// MH810105(S) MH364301 QRコード決済対応
	case RY_EQR:														/*　QRコード処理要求    			　　	  */
// MH810105(E) MH364301 QRコード決済対応
		et94();															/*　					　　　　　　　　　　　*/
// MH810103(s) 電子マネー対応 残金＝０対応
		goto L_et02_CreditEnd;
// MH810103(e) 電子マネー対応 残金＝０対応
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case RY_CRE:														/* クレジットカード							  */
		et90();
// MH321800(S) 割引種別8000のセット方法を修正
//// MH321800(S) D.Inaba ICクレジット対応
//		// クレジット決済も割引情報にデータ(8000)をセットする為、テーブル番号を加算する
//		tb_number++;													/*　計算結果格納テーブル番号を１つ加算　　　　*/
//		rhspnt = (char)tb_number;										/*　現在格納数のセット　　　　　　　　　　　　*/
//// MH321800(E) D.Inaba ICクレジット対応
// MH321800(E) 割引種別8000のセット方法を修正
		goto L_et02_CreditEnd;
		break;
	case RY_CMI:														/*　精算中止店№割引要求の場合　中止店№割処理*/
		et51();															/*　　　　　　　　　　　　　　　　　　　　　　*/
		flg = 2;
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case	RY_GNG:														/* 減額精算									  */
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
	case	RY_GNG_FEE:
	case	RY_GNG_TIM:	
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
		et95();															/*　					　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case	RY_FRK:														/* 振替精算									  */
		et96();															/*　					　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case	RY_SSS:														// 修正精算
		et97();
		break;
	default:															/*　その他の要求（割引）の場合　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( ! ryoukin )&&((tki_ken == 0)||(tki_flg == OFF)))	flg = 1;	/*　料金０円の場合　　　　　　　　　　　　*/
		else															/*　料金０円でない場合　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			switch ( parameter )										/*　パラメータにより分類　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
			case RY_TWR:		et42();		break;						/*　中止券の時間割引処理要求の場合　　　　　　*/
			case RY_RWR:		et43();		break;						/*　中止券の料金差引割引要求の場合　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			case RY_FRE_K:												/*　拡張回数券割引要求　					  */
			case RY_FRE_K2:												/*　拡張2回数券割引要求 　　　　　　　　　　　*/
			case RY_FRE_K3:												/*　拡張3回数券割引要求 　　　　　　　　　　　*/
			case RY_FRE:		et44();		break;						/*　回数券割引要求の場合　回数券割引処理　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			default:			flg = 1;	break;						/*　その他の要求の場合　フラグＮＧ　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( parameter	!=	RY_PKC		&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		parameter	!=	RY_TSA		&&									/*　要求が定期・時間割引　　　　　　　　　　　*/
		parameter	!=  RY_RAG		&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		parameter	!=  RY_OTR		&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		parameter 	!=  RY_FRE_K	&&									/*　　拡張回数券　　　以外で　				  */
		parameter	!=	RY_FRE		&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		parameter	!=	RY_FRE_K2	&&									/*　  拡張2回数券 　　　　		　　　　　　　*/
		parameter	!=	RY_FRE_K3	&&									/*　  拡張3回数券 　　　　		　　　　　　　*/
		parameter	!=	RY_KCH		&&									/*　　種別切替　		 		  　　　　　　*/
		parameter   !=  RY_PKC_K	&&									/*	  拡張定期								  */
		parameter   !=  RY_PKC_K2	&&									/*	  拡張2定期								  */
		parameter   !=  RY_PKC_K3	&&									/*	  拡張3定期								  */
		tki_flg		==	OFF			&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		flg			!=	1			&&
		tki_ken		<	20 			)									/*　　定期処理後ｆｌｇがＯＮで　　　　　　　　*/
	{																	/*　　定期処理後料金計算要求件数が２０件以下　*/
		req_tki[tki_ken].syubt	 = req_rhs[tb_number].syubt;			/*　定期処理後料金計算要求テーブルのセット　　*/
		req_tki[tki_ken].param	 = req_rhs[tb_number].param;			/*　　　　　　　　　　　　　　　　　　　　　　*/
		req_tki[tki_ken].data[0] = req_rhs[tb_number].data[0];			/*　　　　　　　　　　　　　　　　　　　　　　*/
		req_tki[tki_ken].data[1] = req_rhs[tb_number].data[1];			/*　　　　　　　　　　　　　　　　　　　　　　*/
		tki_ken ++;														/*　定期処理後料金計算要求件数インクリメント　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( flg == 0 )														/*　パラメータ正常の場合　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( ( parameter		!= RY_OTR )	||								/*　要求がおつり計算でないか        　　　　　*/
			( parameter		== RY_OTR )	&&								/*　お釣り計算　　　　　　　　　　　　　　　　*/
			( husoku_flg	== OFF	  )	)								/*　不足無し　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			tb_number++;												/*　計算結果格納テーブル番号を１つ加算　　　　*/
			rhspnt = (char)tb_number;									/*　現在格納数のセット　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#if 1	// ここから：クラウド料金計算のryo_buf_n_calc(cal_cloud/cal_cloud.c)に相当。
		// ※修正する際はryo_buf_n_calc側も確認すること。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
		ryo_buf_n.require = ryo_buf_n.ryo - ryo_buf_n.dis ;				/* 割引後額=料金-割引額						  */
																		/*											  */
		taxable = 0L ;													/* 											  */
		if( cons_tax_ratio != 0 )										/*											  */
		{																/*											  */
			switch ( ctyp_flg )											/* 被消費税額 取得							  */
			{															/*											  */
			case  2: taxable = ryo_buf_n.require - c_pay ; break ;		/* 割引後額-PP引落額						  */
			case  3: taxable = ryo_buf_n.require		 ; break ;		/* 割引後額									  */
			case  4: taxable = ryo_buf_n.ryo			 ; break ;		/* 基本料金									  */
			default: taxable = 0L						 ; break ;		/* (内税)									  */
			}															/*											  */
		}																/*											  */
		if ( taxable )													/* 税額有り									  */
		{																/*											  */
			ryo_buf_n.require +=										/*											  */
				ec68( taxable, cons_tax_ratio );						/* 割引後額 += 消費税額						  */
		}																/*											  */
																		/*　　　　　　　　　　　					  */
/**********************/												/*											  */
																		/*											  */
		ryo_buf_n.require -=	c_pay ;									/*	ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ支払額を差し引く			  */
		if( ryo_buf_n.require  <= 0 )	ryo_buf_n.require = 0;			/*　料金が０円未満      　  　　			  */
		if( svcd_dat  != 0 )	ryo_buf_n.nyu	   += svcd_dat;			/*　ｻｰﾋﾞｽ券釣銭  入金額に加算   			  */
																		/*											  */
/*********************/													/*											  */
																		/*											  */
		if(ryo_buf_n.require < ryo_buf_n.nyu)							/*	料金＜入金 taka stat					  */
		{																/*											  */
			ryo_buf_n.hus = ryo_buf_n.nyu - ryo_buf_n.require ;			/* 釣り額=入金額－料金						  */
			ryo_buf_n.require = 0;										/* 請求額=０　　　　　						  */
		}																/*											  */
		else															/*	料金＞＝入金							  */
		{																/*											  */
			ryo_buf_n.require -= ryo_buf_n.nyu ;						/* 請求額=料金-入金額						  */
			ryo_buf_n.hus = ryo_buf_n.require ;							/* 請求額=料金-入金額						  */
		}																/*											  */
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#endif	// ここまで：クラウド料金計算のryo_buf_n_calc(cal_cloud/cal_cloud.c)に相当。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
	}																	/*　　　　　　　　　　　					  */
L_et02_CreditEnd:
	return ;															/*　　　　　　　　　　　					  */
																		/*											  */
}																		/**********************************************/
