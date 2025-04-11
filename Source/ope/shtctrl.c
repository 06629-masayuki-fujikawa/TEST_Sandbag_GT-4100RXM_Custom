/*[]----------------------------------------------------------------------[]*/
/*| shutter control                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"prm_tbl.h"


// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//char	pcard_shtter_ctl;
//void	Pcard_shut_close( void );
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

/*[]----------------------------------------------------------------------[]*/
/*| 『ｼｬｯﾀｰ制御処理』                                                      |*/
/*|                                                                        |*/
/*|□券シャッターとコインシャッターが同時に動作しないように制御する。      |*/
/*| 　理由：電源容量の懸念                                                 |*/
/*|□制御方法                                                              |*/
/*|	シャッター動作させるとき（開でも閉でも）、                             |*/
/*|		他シャッターが動作中なら、動作を保留する。                         |*/
/*|		自シャッターが動作中または動作禁止中なら、動作を保留する。         |*/
/*|	とする。                                                               |*/
/*|□変数                                                                  |*/
/*|　シャッター動作要求　BYTE  SHT_REQ　|d7|d6|d5|d4|d3|d2|d1|d0|          |*/
/*|                                                                        |*/
/*|		d1d0 → 　　券シャッター動作要求　=0:なし、=1:閉要求、=2:開要求    |*/
/*|		d3d2 → コインシャッター動作要求　=0:なし、=1:閉要求、=2:開要求    |*/
/*|                                                                        |*/
/*|　シャッター動作状態  BYTE  SHT_CTRL　|d7|d6|d5|d4|d3|d2|d1|d0|         |*/
/*|                                                                        |*/
/*|		d1d0 → 　　券シャッター動作状態　=0:停止、=1:動作中、=2:動作禁止中|*/
/*|		d3d2 → コインシャッター動作状態　=0:停止、=1:動作中、=2:動作禁止中|*/
/*|                                                                        |*/
/*|		動作中とは、シャッター動作のパルス電圧印可中（約200ms）を示す。    |*/
/*|		動作禁止中とは、パルス出力をｏｆｆした直後に設けた動作禁止状態     |*/
/*|		（約200ms）を示す。                                                |*/
/*|		シャッター開直後はソレノイド温度上昇防止のため3secの動作禁止をおく |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : shtctrl( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 2005-12-20 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	shtctrl( void )
{
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//	//----- 券ｼｬｯﾀｰ制御 -----
//	if( SHT_REQ.BIT.READ ){											// 動作要求ありか?
//		if( !SHT_CTRL.BIT.READ && (SHT_CTRL.BIT.COIN != 1) ){		// ｼｬｯﾀｰ動作していない
//			// 券ｼｬｯﾀｰ制御中でなく、ｺｲﾝｼｬｯﾀｰも動作していない
//
//			if( SHT_REQ.BIT.READ == 1 ){
//				//券ｼｬｯﾀｰ閉ﾘｸｴｽﾄ
//				_di();
//				SHT_REQ.BIT.READ = 0;								// 要求を落とす
//				SHT_CTRL.BIT.READ = 1;								// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ動作中
//				CP_RED_SHUT_CLOSE = 1;								// ｼｬｯﾀｰ閉
//				CP_RED_SHUT_OPEN = 0;
//				_ei();
//				inc_dct( READ_SHUT_CT, 1 );							// 動作ｶｳﾝﾄ+1
//				LagTim20ms( LAG20_RD_SHTCTL, 11, TpuReadShut );		// 200ms～220msﾎﾟｰﾄ出力
//				LedReq( RD_SHUTLED, LED_OFF );						// 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED OFF
//				READ_SHT_flg = 1;									// ｼｬｯﾀｰ閉状態
//			}
//			else if( SHT_REQ.BIT.READ == 2 ){
//				//券ｼｬｯﾀｰ開ﾘｸｴｽﾄ
//				_di();
//				SHT_REQ.BIT.READ = 0;								// 要求を落とす
//				SHT_CTRL.BIT.READ = 1;								// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ動作中
//				CP_RED_SHUT_OPEN = 1;								// ｼｬｯﾀｰ開
//				CP_RED_SHUT_CLOSE = 0;
//				_ei();
//				inc_dct( READ_SHUT_CT, 1 );							// 動作ｶｳﾝﾄ+1
//				LagTim20ms( LAG20_RD_SHTCTL, 11, TpuReadShut );		// 200ms～220msﾎﾟｰﾄ出力
//				LedReq( RD_SHUTLED, LED_ONOFF );					// 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
//				READ_SHT_flg = 2;									// ｼｬｯﾀｰ開状態
//			}
//		}
//	}
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)

}

/*[]----------------------------------------------------------------------[]*/
/*| ﾘｰﾀﾞｰｼｬｯﾀｰ処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_shutter( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ｼｬｯﾀｰ制御状態とﾘｰﾀﾞｰの状態を判断しｼｬｯﾀｰ閉する                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	rd_shutter( void )
{
	if( PPrmSS[S_P01][1] != 0 ){									// ｼｬｯﾀｰ開放?
		if( GT_Settei_flg == 1 && OPECTL.Ope_Mnt_flg == 0){			// 磁気ﾘｰﾀﾞｰﾀｲﾌﾟ異常の場合で、ｼｽﾃﾑﾒﾝﾃ>動作ﾁｪｯｸ中でない場合は
			return;                                                 // ｼｬｯﾀｰｵｰﾌﾟﾝさせない
		}
		read_sht_opn();												// ﾘｰﾀﾞｰｼｬｯﾀｰ開・LED ON
	}else{
		if(( RD_mod != 11 )&&( RD_mod != 10 )){						// ｶｰﾄﾞ取り出し口?
			start_rdsht_cls();										// 500secﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ｼｬｯﾀｰ閉の起動をﾃﾞｨﾚｲ後に行う                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : read_sht_opn( void )                                    |*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2005-12-20                                              |*/
/*| Update       : 2005-12-20 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void start_rdsht_cls( void )
{
	LagTim20ms( LAG20_RD_SHTCLS, 26, read_sht_cls );				// 500msﾀｲﾏ
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾘｰﾀﾞｰｼｬｯﾀｰ開処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : read_sht_opn( void )                                    |*/
/*| PARAMETER    : unsigned char : エラー判定(0:閉/1:開)                   |*/
/*| RETURN VALUE : unsigned char : エラー結果(0:エラーなし/1:エラーあり)   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Ise                                                     |*/
/*| Date         : 2009-06-03                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
CERTIFY_ERR_ARM certify_opn_err[] =		/* シャッター開動作可否判定用テーブル */
{
	{mod_read,  1},						/* 磁気リーダー通信不良 */
	{mod_read,  2},						/* 磁気リーダータイプエラー */
	{mod_read, 10},						/* 磁気リーダー内券詰まり */
	{mod_read, 11},						/* 磁気プリンタ内券詰まり */
};
uchar read_sht_jdg(uchar opn_cls)
{
	uchar ret = 0;						/* エラー判定結果 */
	int i, loop;						/* エラー対象テーブル参照用変数 */

	if(opn_cls)							/* 判定方向が開方向 */
	{
		if(OPECTL.op_faz < 2)			/* 動作フェーズが精算中以外 */
		{
			loop = sizeof(certify_opn_err) / sizeof(certify_opn_err[0]);
			for(i = 0; i < loop; i++)	/* エラー対象の発生状態を取得 */
			{
				/* 対象のエラーが発生中 */
				if(ERR_CHK[certify_opn_err[i].kind][certify_opn_err[i].code])
				{
					ret = 1;			/* エラーあり */
					break;				/* エラー判定処理を抜ける */
				}
			}
		}
	}

	return ret;							/* 処理終了 */
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾘｰﾀﾞｰｼｬｯﾀｰ開処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : read_sht_opn( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 2005-12-20 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	read_sht_opn( void )
{
	//****************************
	LagCan20ms( LAG20_RD_SHTCLS );								// ｼｬｯﾀｰ閉ﾃﾞｨﾚｲを止める
	//****************************
	if( GT_Settei_flg == 1 && OPECTL.Ope_Mnt_flg == 0){			// 磁気ﾘｰﾀﾞｰﾀｲﾌﾟ異常の場合で、ｼｽﾃﾑﾒﾝﾃ>動作ﾁｪｯｸ中でない場合は
		return;													// ｼｬｯﾀｰｵｰﾌﾟﾝさせない
	}
	if(read_sht_jdg(1))												// シャッター開不可能なエラー発生中
	{
		return;														// ｼｬｯﾀｰｵｰﾌﾟﾝさせない
	}
// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
// MH810103 GG119202(S) みなし決済扱い時の動作
//	if (ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0) {			// WAON決済異常が発生している
	if (ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0 ||			// WAON決済異常が発生している
		ERR_CHK[mod_ec][ERR_EC_SETTLE_ABORT] != 0) {				// 決済処理失敗が発生している
// MH810103 GG119202(E) みなし決済扱い時の動作
		return;														// シャッターオープンさせない
	}
// MH321800(E) hosoda ICクレジット対応 (アラーム取引)

	if(( READ_SHT_flg != 2 )||( SHT_REQ.BIT.READ == 1 )){
		// 券ｼｬｯﾀｰ開でない or 閉要求あり
		SHT_REQ.BIT.READ = 2;										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開ﾘｸｴｽﾄ
		shtctrl();
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾘｰﾀﾞｰｼｬｯﾀｰ閉処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : read_sht_cls( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 2005-12-20 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	read_sht_cls( void )
{
	if(( READ_SHT_flg != 1 )||( SHT_REQ.BIT.READ == 2 )){
		// 券ｼｬｯﾀｰ開でない or 開要求あり
		SHT_REQ.BIT.READ = 1;										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉ﾘｸｴｽﾄ
		shtctrl();
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾘｰﾀﾞｰｼｬｯﾀｰﾎﾟｰﾄ制御                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TpuReadShut( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	TpuReadShut( void )
{
	short	T;

	CP_RED_SHUT_OPEN = 0;											// ｿﾚﾉｲﾄﾞOFF(ﾎﾟｰﾄ出力OFF)
	CP_RED_SHUT_CLOSE = 0;

	if( 1 == SHT_CTRL.BIT.READ && OPECTL.Ope_Mnt_flg != 7 ){		// 今まで動作中?
		SHT_CTRL.BIT.READ = 2;										// 動作禁止状態とする
		if( READ_SHT_flg == 2 ){
			// 開状態
			//T = 71;												// 1500ms～1700msｳｴｲﾄする
			T = 151;												// 3000ms～3200msｳｴｲﾄする
		}else{
			// 閉状態
			T = 11;													// 200ms～220msｳｴｲﾄする
		}
		LagTim20ms( LAG20_RD_SHTCTL, T, TpuReadShut );
	}else{
		SHT_CTRL.BIT.READ = 0;										// 停止状態とする
	}
	SHTTER_CTRL = 1;												// shtctrl()の呼び出し
}

/*[]----------------------------------------------------------------------[]*/
/*| ｺｲﾝ投入口ｼｬｯﾀｰﾎﾟｰﾄ制御                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TpuCoinShut( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	TpuCoinShut( void )
{
	if( 1 == SHT_CTRL.BIT.COIN ){									// 今まで動作中?
		SHT_CTRL.BIT.COIN = 2;										// 動作禁止状態とする
		LagTim20ms( LAG20_CN_SHTCTL, 11, TpuCoinShut );				// 200ms～220msｳｴｲﾄする
	}else{
		SHT_CTRL.BIT.COIN = 0;										// 停止状態とする
	}
	SHTTER_CTRL = 1;												// shtctrl()の呼び出し
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾘｰﾀﾞｰｼｬｯﾀｰ処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : shutter_err_chk( void )                                 |*/
/*| PARAMETER    : count : ﾘﾄﾗｲ回数                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ｼｬｯﾀｰ状態が一定時間動作中のままになった場合、異常と判断して、ｼｬｯﾀｰ制御 |*/
/*| ﾌﾗｸﾞをｸﾘｱし、ｼｬｯﾀｰ閉動作を行なう。（待機・休業専用処理）			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2011-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	shutter_err_chk( uchar *count )
{

	if(( SHT_REQ.BIT.READ != 0 || SHT_CTRL.BIT.READ != 0 ) ||	// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰが動作中
	   ( SHT_REQ.BIT.COIN != 0 || SHT_CTRL.BIT.COIN != 0 )){	// ｺｲﾝｼｬｯﾀｰが動作中
	   	(*count)++;
		if( *count == RD_ERR_COUNT_MAX ){						// ﾘﾄﾗｲ回数上限
			_di();
			CP_RED_SHUT_OPEN = 0;											// ｿﾚﾉｲﾄﾞOFF(ﾎﾟｰﾄ出力OFF)
			CP_RED_SHUT_CLOSE = 0;
			_ei();
			Lagtim( OPETCBNO, 18, RD_SOLENOID_WAIT );			// ｿﾚﾉｲﾄﾞ動作のﾀｲﾏｰｽﾀｰﾄ
		}else if( (*count) > RD_ERR_COUNT_MAX ){				// ﾘﾄﾗｲ回数上限を超えている場合（現状4回目）
			_di();
			READ_SHT_flg = 0;									// ﾘｰﾀﾞｰｼｬｯﾀｰ状態を初期化
			COIN_SHT_flg = 0;									// ｺｲﾝｼｬｯﾀｰ状態を初期化
			SHT_REQ.BYTE = 0;									// 要求状態をｸﾘｱ
			SHT_CTRL.BYTE = 0;									// 制御状態をｸﾘｱ
			_ei();
			rd_shutter();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰを閉じる
			*count = 0;											// ｶｳﾝﾄ数をｸﾘｱ
		}else{													// ﾘﾄﾗｲｵｰﾊﾞｰしていない
			Lagtim( OPETCBNO, 18, RD_ERR_INTERVAL );			// ﾁｪｯｸ間隔用のﾀｲﾏｰｽﾀｰﾄ
		}
	}
}
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//void	Pcard_shut_close( void )
//{
//	uchar	ist;
//
//	if( (SHT_CTRL.BIT.READ == 2)&&(SHT_CTRL.BIT.COIN != 1) ){	// 停止状態でない
//		if( READ_SHT_flg == 2 ){
//			LagCan20ms( LAG20_RD_SHTCTL );
//			ist = _di2();										// 割り込み禁止
//			SHT_CTRL.BIT.READ = 0;
//			_ei2( ist );										// 割り込みステータスを元に戻す
//		}
//	}
//	read_sht_cls();
//	pcard_shtter_ctl = 1;
//}
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
