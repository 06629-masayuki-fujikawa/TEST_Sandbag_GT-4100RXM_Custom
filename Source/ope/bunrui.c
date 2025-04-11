/*[]----------------------------------------------------------------------------------------------[]*/
/*| 駐車分類集計																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"tbl_rkn.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
// MH322914(S) K.Onodera 2017/01/05 AI-V対応
#include	"flp_def.h"
// MH322914(E) K.Onodera 2017/01/05 AI-V対応

static	char	getnumber( ulong );									/* 該当駐車分類№取得			*/
static	char	getnumber2( ushort );								/* 該当駐車分類№取得			*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 駐車分類集計加算処理																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kan_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N流用)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	bunrui( ushort pos, ulong ryo ,char Syubetu)				/*								*/
{																	/*								*/
	char	bno;													/* 該当分類№					*/
	char	nno;													/* 分類方法						*/
	char	bn_max;													/* 駐車分類使用MAX数ｴﾘｱ			*/
	ulong	totim;													/* 総駐車時間					*/
	ushort	id, it, od, ot;											/* ﾉｰﾏﾗｲｽﾞ用					*/
	SYUKEI	*ts;													/*								*/
	short	Num;													/* 共通設定番号	(52 or 53)		*/
	char	Ichi;													/* 共通設定内 データ位置(1-6)	*/
	uchar	bn_syasyu;												/* 分類車種格納					*/
	uchar	b_typ = 0;
																	/*								*/
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
																	/*								*/
	if( 6 > Syubetu ) Num = 52;										/* パラメータ設定				*/
	else Num = 53;													/*								*/
																	/*								*/
	Ichi = (char)((Syubetu % 6) + 1);								/* パラメータ桁設定				*/
	bn_syasyu = (uchar)prm_get(COM_PRM, S_BUN, Num, 1, Ichi);		/* 分類車種セット				*/
	if( (!bn_syasyu) && ( bn_syasyu >= 4 ) ) return;				/* 車種設定なし且１～３ではない	*/

	if( PayData.teiki.update_mon )	return;							/* 更新精算は分類集計しない		*/
																	/*								*/
	bn_max = (char)CPrmSS[S_BUN][2];								/* 分類数get					*/
	if( !bn_max )	return;											/* 分類数設定なし				*/
																	/*								*/
	nno = (char)CPrmSS[S_BUN][1];									/*								*/
	switch( nno )													/* 駐車分類集計方法により分岐	*/
	{																/*								*/
	case 1:															/* 駐車時間別台数集計			*/
	case 2:															/* 駐車時間別台数・料金集計		*/
// 不具合修正(S) K.Onodera 2016/11/18 #1570 分類集計が駐車時間不明に分類されない
		// 入庫時刻なし？
		if( (!car_in_f.year && !car_in_f.mon && !car_in_f.day &&
			!car_in_f.hour && !car_in_f.min) ||
			( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE && FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mode <= FLAP_CTRL_MODE3 ) ){
			bno = 0;
			b_typ = 3;
			ts->Bunrui1_cnt2[bn_syasyu-1]++;
			if( nno == 2 ){
				ts->Bunrui1_ryo2[bn_syasyu-1] += ryo;
			}
		}
		else{
// 不具合修正(E) K.Onodera 2016/11/18 #1570 分類集計が駐車時間不明に分類されない
		id = dnrmlzm( car_in_f.year,								/* 入車日ﾉｰﾏﾗｲｽﾞ				*/
				(short)car_in_f.mon, (short)car_in_f.day );			/*								*/
		it = (ushort)tnrmlz( 0, 0, (short)car_in_f.hour,			/* 入車時分ﾉｰﾏﾗｲｽﾞ				*/
							(short)car_in_f.min );					/*								*/
		od = dnrmlzm( car_ot_f.year,								/* 出庫日ﾉｰﾏﾗｲｽﾞ				*/
				(short)car_ot_f.mon, (short)car_ot_f.day );			/*								*/
		ot = (ushort)tnrmlz( 0, 0, (short)car_ot_f.hour,			/* 出庫時分ﾉｰﾏﾗｲｽﾞ				*/
							(short)car_ot_f.min );					/*								*/
																	/*								*/
		if( id == od ){												/* 入車日＝出車日?				*/
			totim = (ulong)(ot - it);								/* 総駐車時間算出				*/
		}else{														/*								*/
			if(( od - id ) > 70 ){									/* 70日以上(99990分超)駐車?		*/
				totim = 100000L;									/* 総駐車時間100000時間			*/
			}else{													/*								*/
				totim = ((ulong)(od - id - 1) * 1440L)				/* 総駐車時間算出				*/
						+ (ulong)ot									/*								*/
						+ (ulong)(1440 - it);						/*								*/
			}														/*								*/
		}															/*								*/
																	/*								*/
		bno = getnumber( totim );									/* 該当駐車分類№取得			*/
		if(( bno != 0 )&&( bno <= bn_max )){						/* 分類№が有で、使用MAX以下なら*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			if( nno == 2 ){											/* 駐車時間別台数・料金集計?	*/
				ts->Bunrui1_ryo[bn_syasyu-1][bno-1] += ryo;			/* 料金加算						*/
			}														/*								*/
			b_typ = 1;
		}else{														/* 分類№無し					*/
			ts->Bunrui1_cnt1[bn_syasyu-1]++;						/* 区分以上駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			if( nno == 2 ){											/* 駐車時間別台数・料金集計?	*/
				ts->Bunrui1_ryo1[bn_syasyu-1] += ryo;				/* 区分以上駐車分類料金加算		*/
			}														/*								*/
			b_typ = 2;
		}															/*								*/
// 不具合修正(S) K.Onodera 2016/11/18 #1570 分類集計が駐車時間不明に分類されない
		}
// 不具合修正(E) K.Onodera 2016/11/18 #1570 分類集計が駐車時間不明に分類されない
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
		break;														/*								*/
	case 3:															/* 駐車料金別台数集計			*/
	case 4:															/* 駐車料金別台数・料金集計		*/
		bno = getnumber( ryo );										/* 該当駐車分類№取得			*/
		if(( bno != 0 )&&( bno <= bn_max )){						/* 分類№が有で、使用MAX以下なら*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			if( nno == 4 ){											/* 駐車料金別台数・料金集計?	*/
				ts->Bunrui1_ryo[bn_syasyu-1][bno-1] += ryo;				/* 料金加算						*/
			}														/*								*/
			b_typ = 1;
		}else{														/* 分類№無し					*/
			ts->Bunrui1_cnt1[bn_syasyu-1]++;						/* 区分以上駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			if( nno == 4 ){											/* 駐車料金別台数・料金集計?	*/
				ts->Bunrui1_ryo1[bn_syasyu-1] += ryo;				/* 区分以上駐車分類料金加算		*/
			}														/*								*/
			b_typ = 2;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
		break;														/*								*/
	case 5:															/* 入車時刻別精算台数集計		*/
		it = (ushort)tnrmlz( 0, 0, (short)car_in_f.hour,			/* 入車時分ﾉｰﾏﾗｲｽﾞ				*/
							(short)car_in_f.min );					/*								*/
// 不具合修正(S) K.Onodera 2016/11/18 #1570 分類集計が駐車時間不明に分類されない
//		bno = getnumber2( it );										/* 該当駐車分類№取得			*/
		if( (!car_in_f.year && !car_in_f.mon && !car_in_f.day &&
			!car_in_f.hour && !car_in_f.min) ||
			( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE && FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mode <= FLAP_CTRL_MODE3 ) ){
			bno = (char)NG;
		}else{
			bno = getnumber2( it );									// 該当駐車分類№取得
		}
// 不具合修正(E) K.Onodera 2016/11/18 #1570 分類集計が駐車時間不明に分類されない
		if( bno == (char)NG ){										/* 該当駐車分類№未取得？		*/
			ts->Bunrui1_cnt2[bn_syasyu-1]++;						/* 区分不明駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			b_typ = 3;
		}else{														/*								*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			b_typ = 1;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
		break;														/*								*/
	case 6:															/* 精算時刻別精算台数集計		*/
		ot = (ushort)tnrmlz( 0, 0, (short)car_ot_f.hour,			/* 出庫時分ﾉｰﾏﾗｲｽﾞ				*/
							(short)car_ot_f.min );					/*								*/
		bno = getnumber2( ot );										/* 該当駐車分類№取得			*/
		if( bno == (char)NG ){										/* 該当駐車分類№未取得？		*/
			ts->Bunrui1_cnt2[bn_syasyu-1]++;						/* 区分不明駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			b_typ = 3;
		}else{														/*								*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			b_typ = 1;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
		break;														/*								*/
	case 7:															/* 入車・精算時刻別精算台数集計	*/
		it = (ushort)tnrmlz( 0, 0, (short)car_in_f.hour,			/* 入車時分ﾉｰﾏﾗｲｽﾞ				*/
							(short)car_in_f.min );					/*								*/
// 不具合修正(S) K.Onodera 2016/11/18 #1570 分類集計が駐車時間不明に分類されない
//		bno = getnumber2( it );										/* 該当駐車分類№取得			*/
		if( (!car_in_f.year && !car_in_f.mon && !car_in_f.day &&
			!car_in_f.hour && !car_in_f.min) ||
			( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE && FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mode <= FLAP_CTRL_MODE3 ) ){
			bno = (char)NG;
		}else{
			bno = getnumber2( it );									// 該当駐車分類№取得
		}
// 不具合修正(E) K.Onodera 2016/11/18 #1570 分類集計が駐車時間不明に分類されない
		if( bno == (char)NG ){										/* 該当駐車分類№未取得？		*/
			ts->Bunrui1_cnt2[bn_syasyu-1]++;						/* 区分不明駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			b_typ = 3;
		}else{														/*								*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			b_typ = 1;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
																	/*								*/
		ot = (ushort)tnrmlz( 0, 0, (short)car_ot_f.hour,			/* 出庫時分ﾉｰﾏﾗｲｽﾞ				*/
							(short)car_ot_f.min );					/*								*/
		bno = getnumber2( ot );										/* 該当駐車分類№取得			*/
		if( bno == (char)NG ){										/* 該当駐車分類№未取得？		*/
			ts->Bunrui1_ryo2[bn_syasyu-1]++;						/* 区分不明駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			b_typ = 3;
		}else{														/*								*/
			ts->Bunrui1_ryo[bn_syasyu-1][bno-1]++;					/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			b_typ = 1;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu2 = bno*100;
			syusei[pos-1].bun_syu2 += bn_syasyu*10;
			syusei[pos-1].bun_syu2 += b_typ;
		}
		break;														/*								*/
	}																/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 該当駐車分類№取得処理																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: getnumber( data )																   |*/
/*| PARAMETER	: data	: 分類する料金(円)又は時間(分)											   |*/
/*| RETURN VALUE: ans	: 0:該当№なし, 1-48:該当№												   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N流用)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	char	getnumber( ulong data )								/*								*/
{																	/*								*/
	short	cnt, num;												/*								*/
	char	ans;													/*								*/
																	/*								*/
	num = (short)CPrmSS[S_BUN][2];									/* 駐車分類数					*/
																	/*								*/
	ans = 0;														/*								*/
	for( cnt = 0; cnt < num; cnt++ ){								/* 該当駐車分類№の取得			*/
		if( data <= (ulong)(CPrmSS[S_BUN][3+cnt]*10L) ){			/*								*/
			ans = (char)(cnt+1);									/*								*/
			break;													/*								*/
		}															/*								*/
	}																/*								*/
	return( ans );													/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 該当駐車分類№取得処理2																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: getnumber2( data )															   |*/
/*| PARAMETER	: data	: 分類する時間(分)														   |*/
/*| RETURN VALUE: ans	: 0:該当№なし, 1-48:該当№												   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N流用)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	char	getnumber2( ushort indat )							/*								*/
{																	/*								*/
	short   bundat[48+1];											/* 駐車分類集計設定値　　　　　 */
	char	ret = NG;												/* リタンコード　　　　　　　　 */
	char	bn_max;													/* 駐車分類使用ＭＡＸ数エリア　 */
	char	i;														/*　　　　　　　　　　　　　　　*/
																	/*								*/
	bn_max = (char)CPrmSS[S_BUN][2];								/* 駐車分類数					*/
																	/*								*/
	bundat[0] = (ushort)prm_tim( 0, S_BUN, (short)(3+bn_max-1) );	/* 設定データ					*/
	for( i = 0; i < bn_max; i ++ )									/*　　　　　　　　　　　　　　　*/
	{																/*								*/
		bundat[i+1] = (ushort)prm_tim( 0, S_BUN, (short)(3+i) );	/* 設定データ					*/
	}																/*　　　　　　　　　　　　　　　*/
																	/*								*/
	for( i = 0; i < bn_max; i ++ )									/* 分類を選別する。　　　　　　 */
	{																/*								*/
		if( bundat[i] < bundat[i+1] ){								/* 次の分類設定配列ﾃﾞｰﾀのほうが大きい */
			if(( indat > bundat[i] )&&( indat <= bundat[i+1] )){	/* 分類設定値の範囲に入った　　 */
				ret = (char)(i+1);									/* 該当Ｎｏ．をセット　　　　　 */
				break;												/*　　　　　　　　　　　　　　　*/
			}														/*　　　　　　　　　　　　　　　*/
		}else if( bundat[i] > bundat[i+1] ){						/* 0:00をまたいだ設定の時　　　 */
			if( indat > bundat[i] ){								/* 分類設定値よりデータが大きい場合 */
				ret = (char)(i+1);									/* 該当Ｎｏ．をセット　　　　　 */
				break;												/*　　　　　　　　　　　　　　　*/
			}else if( indat <= bundat[i+1] ){						/* 次の分類設定値よりデータが小さい場合 */
				ret = (char)(i+1);									/* 該当Ｎｏ．をセット　　　　　 */
				break;												/*　　　　　　　　　　　　　　　*/
			}														/*　　　　　　　　　　　　　　　*/
		}															/*　　　　　　　　　　　　　　　*/
	}																/*　　　　　　　　　　　　　　　*/
	return( ret );													/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 駐車分類集計（修正精算）処理																   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: bunrui_syusei( ushort pos, ulong ryo )										   |*/
/*| PARAMETER	: P1	: 車室位置																   |*/
/*|				: P2	: 料金																	   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N流用)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	bunrui_syusei( ushort pos, ulong ryo )						/*								*/
{																	/*								*/
	uchar	bno;													/* 該当分類№					*/
	uchar	nno;													/* 分類方法						*/
	SYUKEI	*ts;													/*								*/
	uchar	bn_syasyu;												/* 分類車種格納					*/
	uchar	b_typ;
																	/*								*/

	if( !syusei[pos-1].bun_syu1 )	return;

	bn_syasyu = (uchar)((syusei[pos-1].bun_syu1/10)%10);			/* 料金加算						*/
	bno = (uchar)(syusei[pos-1].bun_syu1/100);
	b_typ = (uchar)(syusei[pos-1].bun_syu1%10);
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
																	/*								*/
	nno = (char)CPrmSS[S_BUN][1];									/*								*/
	switch( nno )													/* 駐車分類集計方法により分岐	*/
	{																/*								*/
	case 1:															/* 駐車時間別台数集計			*/
	case 2:															/* 駐車時間別台数・料金集計		*/
		switch(b_typ)
		{
		case 1:
			if( (ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0)&&
				(ts->Bunrui1_ryo[bn_syasyu-1][bno-1] >= ryo) ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
				if( nno == 2 ){										/* 駐車時間別台数・料金集計?	*/
					ts->Bunrui1_ryo[bn_syasyu-1][bno-1] -= ryo;		/* 料金加算						*/
				}													/*								*/
			}
			break;
		case 2:
			if( (ts->Bunrui1_cnt1[bn_syasyu-1] > 0)&&
				(ts->Bunrui1_ryo1[bn_syasyu-1] >= ryo) ){
				ts->Bunrui1_cnt1[bn_syasyu-1]--;					/* 区分以上駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
				if( nno == 2 ){										/* 駐車時間別台数・料金集計?	*/
					ts->Bunrui1_ryo1[bn_syasyu-1] -= ryo;			/* 区分以上駐車分類料金加算		*/
				}													/*								*/
			}
			break;
		}
		break;
	case 3:															/* 駐車料金別台数集計			*/
	case 4:															/* 駐車料金別台数・料金集計		*/
		switch(b_typ)
		{
		case 1:
			if( (ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0)&&
				(ts->Bunrui1_ryo[bn_syasyu-1][bno-1] >= ryo) ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
				if( nno == 4 ){										/* 駐車料金別台数・料金集計?	*/
					ts->Bunrui1_ryo[bn_syasyu-1][bno-1] -= ryo;		/* 料金加算						*/
				}													/*								*/
			}
			break;
		case 2:
			if( (ts->Bunrui1_cnt1[bn_syasyu-1] > 0)&&
				(ts->Bunrui1_ryo1[bn_syasyu-1] >=ryo) ){
				ts->Bunrui1_cnt1[bn_syasyu-1]++;					/* 区分以上駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
				if( nno == 4 ){										/* 駐車料金別台数・料金集計?	*/
					ts->Bunrui1_ryo1[bn_syasyu-1] -= ryo;			/* 区分以上駐車分類料金加算		*/
				}													/*								*/
			}
			break;
		}
		break;
	case 5:															/* 入車時刻別精算台数集計		*/
		if( b_typ == 3 ){											/* 該当駐車分類№未取得？		*/
			if( ts->Bunrui1_cnt2[bn_syasyu-1] > 0 ){
				ts->Bunrui1_cnt2[bn_syasyu-1]--;					/* 区分不明駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			}
		}else if( b_typ == 1 ){										/*								*/
			if( ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0 ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			}
		}															/*								*/
		break;														/*								*/
	case 6:															/* 精算時刻別精算台数集計		*/
		if( b_typ == 3 ){											/* 該当駐車分類№未取得？		*/
			if( ts->Bunrui1_cnt2[bn_syasyu-1] > 0 ){
				ts->Bunrui1_cnt2[bn_syasyu-1]--;					/* 区分不明駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			}
		}else if( b_typ == 1 ){										/*								*/
			if( ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0 ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			}
		}															/*								*/
		break;														/*								*/
	case 7:															/* 入車・精算時刻別精算台数集計	*/
		if( b_typ == 3 ){											/* 該当駐車分類№未取得？		*/
			if( ts->Bunrui1_cnt2[bn_syasyu-1] > 0 ){
				ts->Bunrui1_cnt2[bn_syasyu-1]--;					/* 区分不明駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			}
		}else if( b_typ == 1 ){										/*								*/
			if( ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0 ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			}
		}															/*								*/

		bn_syasyu = (uchar)((syusei[pos-1].bun_syu2/10)%10);		/* 料金加算						*/
		bno = (uchar)(syusei[pos-1].bun_syu2/100);
		b_typ = (uchar)(syusei[pos-1].bun_syu2%10);

		if( b_typ == 3 ){											/* 該当駐車分類№未取得？		*/
			if( ts->Bunrui1_ryo2[bn_syasyu-1] > 0 ){
				ts->Bunrui1_ryo2[bn_syasyu-1]--;					/* 区分不明駐車分類台数ｶｳﾝﾄｱｯﾌﾟ	*/
			}
		}else{														/*								*/
			if( ts->Bunrui1_ryo[bn_syasyu-1][bno-1] > 0 ){
				ts->Bunrui1_ryo[bn_syasyu-1][bno-1]--;				/* 該当分類のｶｳﾝﾄｱｯﾌﾟ			*/
			}
		}															/*								*/
		break;														/*								*/
	}																/*								*/
}																	/*								*/

// 仕様変更(S) K.Onodera 2016/12/13 #1674 集計の精算料金に対する振替分の加算方法を変更する
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 駐車分類集計加算処理																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kan_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N流用)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	bunrui_Erace( Receipt_data* pay, ulong back, char syu )
{
	char	bno;					// 該当分類№
	char	bn_max;					// 駐車分類使用MAX数ｴﾘｱ
	ulong	totim;					// 総駐車時間
	ushort	id, it, od, ot;			// ﾉｰﾏﾗｲｽﾞ用
	ushort	i;
	SYUKEI	*ts;
	uchar	bn_syasyu;				// 分類車種格納
	ulong	ryo = 0;

	ts = &sky.tsyuk;

	// 分類車種セット
	if( 6 > pay->syu ){
		bn_syasyu = (uchar)prm_get( COM_PRM, S_BUN, 52, 1, (char)( (pay->syu % 6) + 1 ) );
	}else{
		bn_syasyu = (uchar)prm_get( COM_PRM, S_BUN, 53, 1, (char)( (pay->syu % 6) + 1 ) );
	}
	// 車種設定なし且１～３ではない
	if( (!bn_syasyu) && ( bn_syasyu >= 4 ) ){
		return;
	}

	bn_max = (char)CPrmSS[S_BUN][2];
	// 分類数設定なし？
	if( !bn_max ){
		return;
	}

	//駐車分類集計方法により分岐
	switch( CPrmSS[S_BUN][1] )
	{
		case 2:			// 駐車時間別台数・料金集計
			// 入庫時刻なし？
			if( !pay->TInTime.Year && !pay->TInTime.Mon && !pay->TInTime.Day &&
				!pay->TInTime.Hour && !pay->TInTime.Min ){
				ts->Bunrui1_ryo2[bn_syasyu-1] -= back;
			}
			else{
				id = dnrmlzm( pay->TInTime.Year, (short)pay->TInTime.Mon, (short)pay->TInTime.Day );		// 入庫日ノーマライズ
				it = (ushort)tnrmlz( 0, 0, (short)pay->TInTime.Hour, (short)pay->TInTime.Min );				// 入庫時分ノーマライズ
				od = dnrmlzm( pay->TOutTime.Year, (short)pay->TOutTime.Mon, (short)pay->TOutTime.Day );		// 出庫日ノーマライズ
				ot = (ushort)tnrmlz( 0, 0, (short)pay->TOutTime.Hour, (short)pay->TOutTime.Min );			// 出庫時分ノーマライズ
				// 入庫日＝出庫日？
				if( id == od ){
					totim = (ulong)(ot - it);		// 総駐車時間算出
				}else{
					// 70日以上(99990分超)駐車?
					if(( od - id ) > 70 ){
						totim = 100000L;			// 総駐車時間100000時間
					}else{
						totim = ((ulong)(od - id - 1) * 1440L) + (ulong)ot + (ulong)(1440 - it);
					}
				}

				bno = getnumber( totim );							// 該当駐車分類№取得
				// 分類№が有で、使用MAX以下？
				if(( bno != 0 )&&( bno <= bn_max )){
					ts->Bunrui1_ryo[bn_syasyu-1][bno-1] -= back;
				}
				// 分類№無し？
				else{
					ts->Bunrui1_ryo1[bn_syasyu-1] -= back;
				}
			}
			break;

		case 4:			// 駐車料金別台数・料金集計
		{
			ulong	wari_ryo = 0;
			ulong	ppc_ryo = 0;
			wari_tiket	wari_dt;
			for( i=0; i<WTIK_USEMAX; i++ ){
				disc_wari_conv( &pay->DiscountData[i], &wari_dt );
				switch( wari_dt.tik_syu ){
					case SERVICE:	/* ｻｰﾋﾞｽ券						*/
					case KAKEURI:	/* 掛売券						*/
						// 振替元種別 = 振替先種別又は、強制的に適用
						if( pay->syu == syu || prm_get(COM_PRM, S_CEN, 32, 1, 1) == 1 ){
							wari_ryo += wari_dt.ryokin;
						}
						break;
					case KAISUU:	/* 回数券						*/
					case MISHUU:	/* 未収金						*/
					case FURIKAE:	/* 振替額						*/
					case SYUUSEI:	/* 修正額						*/
						wari_ryo += wari_dt.ryokin;
						break;
					case	PREPAID:										/* プリペイドカード				*/
						ppc_ryo += wari_dt.ryokin;							/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用料金加算		*/
						break;												/*								*/
				}															/*								*/
			}
			// 駐車料金を求める（割引を含む／含まない設定に従う）
			switch( prm_get(COM_PRM, S_TOT, 12, 1, 2) ){					// 分類集計の駐車料金扱い（設定）により分岐
				// 割引後の料金を駐車料金とする（プリペは現金領収額とする）
				case 1:
					if( pay->WPrice >= (wari_ryo + pay->credit.pay_ryo) )
						ryo = (pay->WPrice) - wari_ryo - pay->credit.pay_ryo;	// 駐車料金－割引料金
					break;
				// 割引後の料金を駐車料金とする（プリペは割引額とする）
				case 2:
					if( pay->WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo + pay->Electron_data.Suica.pay_ryo) )
						ryo = (pay->WPrice) - wari_ryo - ppc_ryo - pay->credit.pay_ryo - pay->Electron_data.Suica.pay_ryo;	// 駐車料金－割引料金－PPC料金－電子ﾏﾈｰ料金
					break;
			}
			bno = getnumber( ryo );
			// 分類№が有で、使用MAX以下？
			if(( bno != 0 )&&( bno <= bn_max )){
				ts->Bunrui1_ryo[bn_syasyu-1][bno-1] -= back;
			}
			// 区分以上駐車分類料金？
			else{
				ts->Bunrui1_ryo1[bn_syasyu-1] -= back;
			}
		}
		break;

		default:
			break;
	}
}
// 仕様変更(E) K.Onodera 2016/12/13 #1674 集計の精算料金に対する振替分の加算方法を変更する