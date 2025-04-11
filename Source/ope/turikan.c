/*[]----------------------------------------------------------------------[]*/
/*| 釣銭管理関連                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 2002.02.01                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"pri_def.h"
#include	"mnt_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"raudef.h"
#include	"ntnet_def.h"
#include	"oiban.h"

/*[]----------------------------------------------------------------------[]*/
/*| 釣銭管理の現在保有枚数を求める                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turikan_gen                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*| Update		 :                                                         |*/
/*| Update		 :                                                         |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_gen( void )
{
	int	i;
	long cl;

	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );

	ac_flg.turi_syu = 2;

	for( i = 0; i < 4; i++ ){
		cl = (long)turi_kan.turi_dat[i].zen_mai;
		cl += turi_kan.turi_dat[i].sei_nyu;
		cl += turi_kan.turi_dat[i].hojyu;
		cl -= turi_kan.turi_dat[i].sei_syu;
		if( cl < 0L ){
			cl = 0L;
		}
		if( turi_kan.turi_dat[i].kyosei != 0 ){ /* ｲﾝﾍﾞﾝﾄﾘあり? */
			if( cl >= (long)turi_kan.turi_dat[i].kyosei ){
				cl -= (long)turi_kan.turi_dat[i].kyosei;
			}else{
				/* ｲﾝﾍﾞﾝﾄﾘ枚数は全数払い出すと枚数を正確に把握できない為、調整する */
				turi_kan.turi_dat[i].kyosei = cl;
				cl = 0L;
			}
		}
		turi_kan.turi_dat[i].gen_mai = (unsigned short)cl;

		cl = (long)turi_kan.turi_dat[i].yzen_mai;
		cl -= turi_kan.turi_dat[i].ysei_syu;
		if( cl < 0L ){
			cl = 0L;
		}
		if( turi_kan.turi_dat[i].ykyosei != 0 ){ /* ｲﾝﾍﾞﾝﾄﾘあり? */
			if( cl >= (long)turi_kan.turi_dat[i].ykyosei ){
				cl -= (long)turi_kan.turi_dat[i].ykyosei;
			}else{
				/* ｲﾝﾍﾞﾝﾄﾘ枚数は全数払い出すと枚数を正確に把握できない為、調整する */
				turi_kan.turi_dat[i].ykyosei = cl;
				cl = 0L;
			}
		}
		turi_kan.turi_dat[i].ygen_mai = (unsigned short)cl;
	}

	ac_flg.turi_syu = 0;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算完了時に入出金枚数（循環のみ）をｶｳﾝﾄする                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turikan_pay                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_pay( void )
{
	int	i, k;

	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );

	ac_flg.turi_syu = 22;

	for( i = 0; i < 4; i++ ){
		k = ryo_buf.in_coin[i] - turi_dat.pay_safe[i];
		if( k > 0 ){
			turi_kan.turi_dat[i].sei_nyu += (long)k; 	/* 精算時入金枚数 */
			turi_kan.turi_dat[i].gen_mai += k; 			/*現保有枚数 */
		}
	}
	for( i = 0; i < 4; i++ ){
		k = ryo_buf.out_coin[i];
		if( k > 0 ){
			turi_kan.turi_dat[i].sei_syu += (long)k;
			if( turi_kan.turi_dat[i].gen_mai >= k ){
				turi_kan.turi_dat[i].gen_mai -= k; 		/*現保有枚数 */
			}
		}
	}
	k = ryo_buf.out_coin[4]; /* 10yen予蓄 */
	if( k > 0 ){
		turi_kan.turi_dat[0].ysei_syu += (long)k;
		turi_kan.turi_dat[0].ygen_mai  -= (ushort)k;			// 予備ﾁｭｰﾌﾞから払いだした10円の枚数分を減算する
	}
	k = ryo_buf.out_coin[5]; /* 50yen予蓄 */
	if( k > 0 ){
		turi_kan.turi_dat[1].ysei_syu += (long)k;
		turi_kan.turi_dat[1].ygen_mai  -= (ushort)k;			// 予備ﾁｭｰﾌﾞから払いだした50円の枚数分を減算する
	}
	k = ryo_buf.out_coin[6]; /* 100yen予蓄 */
	if( k > 0 ){
		turi_kan.turi_dat[2].ysei_syu += (long)k;
		turi_kan.turi_dat[2].ygen_mai  -= (ushort)k;			// 予備ﾁｭｰﾌﾞから払いだした100円の枚数分を減算する
	}

// MH810104(S) R.Endo 2021/10/25 車番チケットレス フェーズ2.3 #6127 【現場指摘】精算完了後すぐにトップへ戻ると、現在枚数が更新されず、金銭管理データが送信されない。
	if ( Make_Log_MnyMng(OpeNtnetAddedInfo.PayClass) ) {		// 金銭管理ログデータ作成
		Log_regist(LOG_MONEYMANAGE_NT);							// 金銭管理ログ登録
	}
// MH810104(E) R.Endo 2021/10/25 車番チケットレス フェーズ2.3 #6127 【現場指摘】精算完了後すぐにトップへ戻ると、現在枚数が更新されず、金銭管理データが送信されない。

	ac_flg.turi_syu = 23;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ｶｾｯﾄ着時の初期化                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turian_ini                                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_ini( void )
{
	int	i;
	short nu;

	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );

	ac_flg.turi_syu = 1;

	if( Tubu_cnt_set != 0 ){ /* 枚数設定あり? */
		nu = (unsigned short)CPrmSS[S_KAN][3];		// 10円デフォルト枚数
		if( nu > TUB_MAX10 ) nu = TUB_MAX10;
		turi_kan.turi_dat[0].sin_mai = nu;

		nu = (unsigned short)CPrmSS[S_KAN][9];		// 50円デフォルト枚数
		if( nu > TUB_MAX50 ) nu = TUB_MAX50;
		turi_kan.turi_dat[1].sin_mai = nu;

		nu = (unsigned short)CPrmSS[S_KAN][15];		// 100円デフォルト枚数
		if( nu > TUB_MAX100 ) nu = TUB_MAX100;
		turi_kan.turi_dat[2].sin_mai = nu;

		nu = (unsigned short)CPrmSS[S_KAN][21];		// 500円デフォルト枚数
		if( nu > TUB_MAX500 ) nu = TUB_MAX500;
		turi_kan.turi_dat[3].sin_mai = nu;

		turi_kan.turi_dat[0].ysin_mai = (unsigned short)prm_get( COM_PRM,S_KAN,27,3,1 );	// 10円予蓄デフォルト枚数
		turi_kan.turi_dat[1].ysin_mai = (unsigned short)prm_get( COM_PRM,S_KAN,30,3,1 );	// 50円予蓄デフォルト枚数
		turi_kan.turi_dat[2].ysin_mai = (unsigned short)prm_get( COM_PRM,S_KAN,33,3,1 );	// 100円予蓄デフォルト枚数

		for( i = 0; i < 4; i++ ){
			turi_kan.turi_dat[i].gen_mai = turi_kan.turi_dat[i].sin_mai;
			turi_kan.turi_dat[i].ygen_mai = turi_kan.turi_dat[i].ysin_mai;
		}
	}

	ac_flg.turi_syu = 0;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 釣銭管理操作の各処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Cash_proc( ope_kind )                                   |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : hashimoto(AMANO)                                        |*/
/*| Date         : 2001-11-02                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	turikan_proc( short ope_kind )
{
	int	i;
	short kin = 0;
	short ckn[4];


	switch( ope_kind )
	{
		case MNY_CHG_BEFORE: // 釣銭枚数変更の前処理
			turi_dat.turi_in = 0x01ff;
			turikan_gen();
			turi_dat.coin_dsp[0] = turi_kan.turi_dat[0].gen_mai;
			turi_dat.coin_dsp[1] = turi_kan.turi_dat[1].gen_mai;
			turi_dat.coin_dsp[2] = turi_kan.turi_dat[2].gen_mai;
			turi_dat.coin_dsp[3] = turi_kan.turi_dat[3].gen_mai;
			turi_dat.coin_dsp[4] = turi_kan.turi_dat[0].ygen_mai; // 10円予蓄
			turi_dat.coin_dsp[5] = turi_kan.turi_dat[1].ygen_mai; // 50円予蓄
			turi_dat.coin_dsp[6] = turi_kan.turi_dat[2].ygen_mai; // 100円予蓄
			cn_stat( 24, 0 ); // CREM OFF, ｲﾝﾍﾞﾝﾄﾘ禁止
			// 釣銭管理合計ﾌﾟﾘﾝﾄ時に参照する為ｸﾘｱしておく
			memset( turi_dat.outcount, 0, sizeof( turi_dat.outcount ) ); 	// 出金ｺｲﾝｶｳﾝﾄ(保有枚数変化分) 
			break;
		case MNY_CHG_10YEN: // 10円枚数変更
			turi_dat.turi_in &= (~0x0010);
			break;
		case MNY_CHG_50YEN: // 50円枚数変更
			turi_dat.turi_in &= (~0x0020);
			break;
		case MNY_CHG_100YEN: // 100円枚数変更
			turi_dat.turi_in &= (~0x0040);
			break;
		case MNY_CHG_500YEN: // 500円枚数変更
			turi_dat.turi_in &= (~0x0080);
			break;
		case MNY_CHG_10SUBYEN: // 10円SUB枚数変更
			turi_dat.turi_in &= (~0x0004);
			break;
		case MNY_CHG_50SUBYEN: // 50円SUB枚数変更
			turi_dat.turi_in &= (~0x0100);
			break;
		case MNY_CHG_100SUBYEN: // 100円SUB枚数変更
			turi_dat.turi_in &= (~0x0008);
			break;
		case MNY_CHG_AFTER: // 釣銭枚数変更の後処理
			if( (turi_dat.turi_in & 0x01fc) != 0x01fc )
			{	// 枚数を変更した
				turi_kan.turi_dat[0].sin_mai = turi_dat.coin_dsp[0];
				turi_kan.turi_dat[1].sin_mai = turi_dat.coin_dsp[1];
				turi_kan.turi_dat[2].sin_mai = turi_dat.coin_dsp[2];
				turi_kan.turi_dat[3].sin_mai = turi_dat.coin_dsp[3];
				turi_kan.turi_dat[0].ysin_mai = turi_dat.coin_dsp[4];
				turi_kan.turi_dat[1].ysin_mai = turi_dat.coin_dsp[5];
				turi_kan.turi_dat[2].ysin_mai = turi_dat.coin_dsp[6];
				memcpy( turi_dat.coin_sin, turi_dat.coin_dsp, sizeof(turi_dat.coin_sin) );
				turikan_prn(MNY_CHG_AFTER);
				cn_stat( 32, 0 ); // 保有枚数の送信、ｲﾝﾍﾞﾝﾄﾘ可
			}else{
				cn_stat( 2, 0 ); // ｲﾝﾍﾞﾝﾄﾘ可
			}
			turi_dat.turi_in = 0;
			break;
		case MNY_CTL_BEFORE: // 釣銭補充の前処理
			memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) ); //復電用ﾊﾞｯｸｱｯﾌﾟ 
			turi_dat.turi_in = 0x01ff;	// 補充操作ﾌﾗｸﾞｾｯﾄ */
			memset( turi_dat.incount, 0, sizeof( turi_dat.incount ) ); // 入金ｺｲﾝｶｳﾝﾄ(保有枚数変化分) 
			memset( turi_dat.outcount, 0, sizeof( turi_dat.outcount ) ); // 出金ｺｲﾝｶｳﾝﾄ(保有枚数変化分) 
			memset( SFV_DAT.r_add08, 0, sizeof( SFV_DAT.r_add08 ) ); // 金庫枚数計算用 
			memset( ryo_buf.out_coin, 0, sizeof(ryo_buf.out_coin) );
			memset( ryo_buf.in_coin, 0, sizeof(ryo_buf.in_coin) );
			memset( SFV_DAT.r_add0a, 0, sizeof( SFV_DAT.r_add0a ) ); // 金庫枚数計算用 ( 補充時金庫搬送枚数算出に使用 )
			for( i = 0; i < 4; i++ ){
				turi_kan.turi_dat[i].hojyu_safe = 0;					 // 釣銭補充時金庫搬送枚数のクリア
			}

			if( CPrmSS[S_KAN][1] ){
				turikan_gen();
				turi_dat.coin_dsp[0] = turi_dat.dsp_ini[0] = turi_kan.turi_dat[0].gen_mai;
				turi_dat.coin_dsp[1] = turi_dat.dsp_ini[1] = turi_kan.turi_dat[1].gen_mai;
				turi_dat.coin_dsp[2] = turi_dat.dsp_ini[2] = turi_kan.turi_dat[2].gen_mai;
				turi_dat.coin_dsp[3] = turi_dat.dsp_ini[3] = turi_kan.turi_dat[3].gen_mai;
				turi_dat.coin_dsp[4] = turi_dat.dsp_ini[4] = turi_kan.turi_dat[0].ygen_mai;
				turi_dat.coin_dsp[5] = turi_dat.dsp_ini[5] = turi_kan.turi_dat[1].ygen_mai;
				turi_dat.coin_dsp[6] = turi_dat.dsp_ini[6] = turi_kan.turi_dat[2].ygen_mai;
				cn_stat( 21, 0 );	/* Coin Enable */
			}else{
				turi_dat.dsp_ini[0] = 0;
				turi_dat.dsp_ini[1] = 0;
				turi_dat.dsp_ini[2] = 0;
				turi_dat.dsp_ini[3] = 0;
				turi_dat.dsp_ini[4] = 0;
				turi_dat.dsp_ini[5] = 0;
				turi_dat.dsp_ini[6] = 0;
				cn_stat( 1, 0 );	// CREM ON, ｲﾝﾍﾞﾝﾄﾘ禁止
			}
			break;
		case MNY_INCOIN: // 釣銭補充中の入金
			if( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_IH_EVT) )){
												/* 入金あり? */
				turi_dat.turi_in &= (~0x0001);	/* 補充操作ありｾｯﾄ */
				for( i = 0; i < 4; i++ ){
									/* 保有枚数の増加分を表示ｴﾘｱに加算する */
					turi_dat.coin_dsp[i] = (ushort)bcdbin( CN_RDAT.r_dat07[i] );
				}
			}
			if( OPECTL.CN_QSIG == 5 ){	/* 入金あり? */
				cn_stat( 5, 0 ); /* 入金継続 */
			}
			if( OPECTL.CN_QSIG == 1 ){	/* 入金あり? */
				turi_dat.turi_in &= (~0x0001);	/* 補充操作ありｾｯﾄ */
			}
			break;
		case MNY_CTL_AFTER: // 釣銭補充の後処理
							// CREM OFFしてから呼ばれる
			if(( turi_dat.turi_in & 0x0003 ) != 0x0003 ){
				if( CPrmSS[S_KAN][1] ){
					if( ( turi_dat.turi_in & 0x0001 ) == 0 )
					{	// 入金あり
						for( i = 0; i < 4; i++ )
						{
							turi_kan.turi_dat[i].hojyu = turi_dat.incount[i];
						}
						turiadd_hojyu_safe();					// 釣銭補充時金庫搬送枚数の加算
					}
					if( ( turi_dat.turi_in & 0x0002 ) == 0 )
					{	// 釣り合わせあり
						for( i = 0; i < 4; i++ )
						{
							turi_kan.turi_dat[i].kyosei += turi_dat.outcount[i];
						}
					}
					turi_kan.turi_dat[0].sin_mai = turi_dat.coin_dsp[0];
					turi_kan.turi_dat[1].sin_mai = turi_dat.coin_dsp[1];
					turi_kan.turi_dat[2].sin_mai = turi_dat.coin_dsp[2];
					turi_kan.turi_dat[3].sin_mai = turi_dat.coin_dsp[3];
					turi_kan.turi_dat[0].ysin_mai = turi_dat.coin_dsp[4];
					turi_kan.turi_dat[1].ysin_mai = turi_dat.coin_dsp[5];
					turi_kan.turi_dat[2].ysin_mai = turi_dat.coin_dsp[6];

					turikan_prn( MNY_CTL_AFTER );	// 釣銭管理印字 
					kin = 1;
				}
				else{
					wopelg( OPLOG_TURIHOJU, 0, 0 );
				}
				cn_stat( 22, 0 );					// CREM OFF,CLR 
			}

			turi_dat.turi_in = 0;
			memcpy( &ckn[0], &SFV_DAT.safe_dt[0], 8 );
			turikan_sfv();
			if( ( kin == 1 )||
				( memcmp( &ckn[0], &SFV_DAT.safe_dt[0], 8 ) != 0 ) ){
				if( 4 == OPECTL.Mnt_mod ){			// 補充ｶｰﾄﾞ使用
					Make_Log_MnyMng( 20 );			// 金銭管理ログデータ作成
				}else{								// 設定機から
					Make_Log_MnyMng( 30 );			// 金銭管理ログデータ作成
				}
			}
			break;
		case MNY_CTL_AUTO: // 釣り合わせキー押下
			turi_dat.turi_in &= (~0x0002);	// AUTO操作ありｾｯﾄ */
			wopelg( OPLOG_TURIAUTO, 0, 0 );	// AUTO操作記録 */
			turi_kan_f_exe_autostart = TURIKAN_AUTOSTART_NOEXE;// 釣り合せ開始で未実行状態
			cn_stat( 2, 0 );
			break;
		case MNY_CTL_AUTOSTART: // 釣り合わせキー押下後CREM OFF
			if( ( turi_dat.turi_in & 0x0001 ) == 0 )
			{	// 入金あり
				turikan_sfv();
				turiadd_hojyu_safe();					// 釣銭補充時金庫搬送枚数の加算
				memset( SFV_DAT.r_add08, 0, sizeof( SFV_DAT.r_add08 ) ); // 金庫枚数計算用 
			}
			turi_kan_f_exe_autostart = TURIKAN_AUTOSTART_EXE;// 釣り合せ開始で実行状態
			cn_stat( 34, 0 );		// 釣り合わせ開始
			break;
		case MNY_CTL_AUTOCOMPLETE: // 釣り合わせ終了
			for( i=0; i<4; i++ ){
				if(turi_kan_f_exe_autostart == TURIKAN_AUTOSTART_EXE){	// 釣り合せ実行済み
					turi_dat.coin_dsp[i] = turi_dat.dsp_ini[i] = (ushort)bcdbin( CN_RDAT.r_dat07[i] );
				}
			}
			if(turi_kan_f_exe_autostart == TURIKAN_AUTOSTART_NOEXE){	// 釣り合せ未実行
				turi_dat.turi_in |= 0x0002;							// 金銭管理合計を印字しない
			}
			cn_stat( 33, 0 );	// CREM ON, ｲﾝﾍﾞﾝﾄﾘ可
			break;
		case MNY_COIN_INVSTART: // インベントリ操作開始
			if( CPrmSS[S_KAN][1] != 0 ){
				cn_stat( 8, 0 );	// CREM OFF+保有枚数送信
			}
			else{
				cn_stat( 2, 0 );	// CREM OFF
			}
			turi_dat.turi_in = 0x01ff;	// 補充操作ﾌﾗｸﾞｾｯﾄ */
			memset( turi_dat.outcount, 0, sizeof( turi_dat.outcount ) ); // 出金ｺｲﾝｶｳﾝﾄ(保有枚数変化分) 
			break;
		case MNY_COIN_INVCOMPLETE: // インベントリ操作終了
			wopelg( OPLOG_INVENTRY, 0, 0 );	// ｲﾝﾍﾞﾝﾄﾘ操作記録
			turi_dat.turi_in = 0;
			turikan_inventry( 0 );				// T集計に加算
			break;
		default:
			break;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			補充時金庫搬送枚数計算
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2009/02/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	turiadd_hojyu_safe(void)
{
	int		c, c2, i;
	
	for (i = 0; i < 4; i++) {
		if( SFV_DAT.r_add08[i] != 0 ){	/* 入金あり? */
			c = (int)turi_kan.turi_dat[i].gen_mai;			// 釣銭補充開始時の初期値(turi_dat.dsp_iniは釣り合わせ後に更新される)
			c2 = (int)( bcdbin( SFV_DAT.r_dat07[i] ));
			c += SFV_DAT.r_add08[i];
			c -= SFV_DAT.r_add0a[i];
			c -= c2;
//NOTE：（補充開始時の枚数＋入金枚数－出金枚数＞メックの保有枚数）の時金庫に搬送されたことになる
//       メックの保有枚数が筒の上限の時に入金されても加算されないことと、出金枚数は釣り合せ時に発生するので上記の式となります
			if( c > 0 ){
				turi_kan.turi_dat[i].hojyu_safe += c;
			}
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 釣銭補充完了処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turian_prn( prm )                                       |*/
/*| PARAMETER    : prm = 0:釣銭補充操作で入金なし                          |*/
/*|                    = 1:釣銭補充操作で入金あり                          |*/
/*|                    = 2:ｲﾝﾍﾞﾝﾄﾘ操作                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2001-12-25                                              |*/
/*| Update		 : ｲﾝﾍﾞﾝﾄﾘ操作ではﾌﾟﾘﾝﾄしない 2005-09-03 T.Hashimoto       |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/

void turikan_prn( short ope_kind )
{
	T_FrmTuriKan	FrmTuriKan;

	if( CPrmSS[S_KAN][1] == 0 ) return;

	turi_kan.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// 機械№
	turi_kan.Kakari_no = OPECTL.Kakari_Num;								// 係員番号set
	memcpy( &turi_kan.NowTime, &CLK_REC, sizeof( date_time_rec ) );		// 現在時刻
	turikan_subtube_set();
	switch( ope_kind ){
		case MNY_CHG_AFTER: // 現在枚数の変更
			wopelg( OPLOG_TURIHENKO, 0, 0 );
			Make_Log_MnyMng( 32 );				// 金銭管理ログデータ作成
			break;
		case MNY_CTL_AFTER: // 釣銭補充の後処理
			wopelg( OPLOG_TURIHOJU, 0, 0 );
			Make_Log_MnyMng( 30 );				// 金銭管理ログデータ作成
			break;
		case MNY_COIN_CASETTE: // コインカセット着
			//wopelg( OPLOG_TURICASETTE );
			Make_Log_MnyMng( 32 );				// 金銭管理ログデータ作成
			break;
		default:
			break;
	}
	CountGet( TURIKAN_COUNT, &turi_kan.Oiban );				// 追番

	FrmTuriKan.prn_kind = R_PRI;
	FrmTuriKan.prn_data = &turi_kan;

	CountUp(TURIKAN_COUNT);

	queset( PRNTCBNO, PREQ_TURIKAN, sizeof(T_FrmTuriKan), &FrmTuriKan  ); /* 釣銭管理印字	*/
	turi_kan_f_defset_wait = TURIKAN_DEFAULT_WAIT;						//印字待ちフラグセット

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 釣銭補充操作終了で金庫枚数をｶｳﾝﾄする                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turikan_sfv                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_sfv( void )
{
	int	i, c;
	int c2;

	memcpy( SFV_DAT.safe_dt_wk, SFV_DAT.safe_dt, sizeof( SFV_DAT.safe_dt_wk ) );

	ac_flg.turi_syu = 10;

	for( i=0; i<4; i++ ){
		if( SFV_DAT.r_add08[i] != 0 ){	/* 入金あり? */
			c = (int)turi_dat.dsp_ini[i];
			c2 = (int)( bcdbin( SFV_DAT.r_dat07[i] ));
			if( c > c2 ){
				// 初期枚数>今回枚数 の場合(枚数減少)
				c2 += 100;
				//釣銭管理枚数不良ｱﾗｰﾑｾｯﾄ(発生/解除)
				alm_chk( ALMMDL_SUB, ALARM_TURIKAN_REV, 2 );			// ｱﾗｰﾑ登録
			}
			c += SFV_DAT.r_add08[i];
			c -= c2;
			if( c > 0 ){
				SFV_DAT.safe_dt[i] += c;
			}
		}
	}
	SFV_DAT.safe_cal_do = 0; /* 金庫枚数算出済み */
	ac_flg.turi_syu = 0;

	return;
}

// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
extern	const	date_time_rec	nulldate;
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 釣銭管理ｴﾘｱクリア判定処理																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: Check_turikan_clr( void )														   |*/
/*| PARAMETER	: 																				   |*/
/*| RETURN VALUE: char	: 0 = クリア不可														   |*/
/*|						  1 = クリア可															   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
char	Check_turikan_clr( void )
{
	TURI_KAN	*dsy;

	dsy = &turi_kan;													/* 現在集計						*/

	//④現在集計の今回集計日時が0クリアされているか確認
	if (memcmp(&dsy->NowTime, &nulldate, sizeof(dsy->NowTime)) == 0) {
		// 現在集計日時がない＝クリアされている
		wmonlg(OPMON_TURIKAN_IRGCLR1, 0, 0);						// モニタ登録
		return 0;
	}

	//⑤現在の追番、係員№、機械№が0クリア去れている場合はクリアキャンセル
	if (( CountSel( &dsy->Oiban ) == 0) && 
		( dsy->Kakari_no == 0 ) && 
		( dsy->Kikai_no == 0 )){
		wmonlg(OPMON_TURIKAN_IRGCLR2, 0, 0);						// モニタ登録
		return 0;
	}

	return 1;
}
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 釣銭管理ｴﾘｱの更新																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: turikan_clr( void )															   |*/
/*| PARAMETER	: void	:																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimoto(AMANO)															   |*/
/*| Date		: 2001-12-25																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void turikan_clr( void )
{
																	/*								*/
	ac_flg.syusyu = 52;												/* 52:印字完了					*/
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		turi_kan.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_CHANGE);
	}
																	/*								*/
	Log_regist( LOG_MONEYMANAGE );									/* 金銭管理情報登録				*/
																	/*								*/
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		RAU_UpdateCenterSeqNo(RAU_SEQNO_CHANGE);
	}
																	/*								*/
	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );				/*								*/
																	/*								*/
	ac_flg.syusyu = 55;												/* 55:釣銭管理集計をﾜｰｸｴﾘｱへ転送完了*/
	turikan_clr_sub2();												/*								*/
																	/*								*/
	ac_flg.syusyu = 56;												/* 56:金銭管理集計ｸﾘｱ完了		*/
	turikan_clr_sub();												/* T集計へ釣銭補充、強制払出を加算 */
																	/*								*/
	ac_flg.syusyu = 58;												/* 58:補充ﾃﾞｰﾀをT集計へ加算終了	*/

	turi_kan_f_defset_wait = TURIKAN_DEFAULT_NOWAIT;//デフォルトセット待ちフラグクリア

	return;
}

void turikan_clr_sub2( void )
{
	int	i;															/*								*/
																	/*								*/
	memset( &turi_kan, 0, sizeof( TURI_KAN ) );						/* 釣銭管理ｴﾘｱｸﾘｱ				*/
																	/*								*/
	for( i = 0; i < 4; i++ ){										/*								*/
		turi_kan.turi_dat[i].gen_mai = turi_kwk.turi_dat[i].sin_mai;/* 現在保有枚数<-新規設定枚数	*/
		turi_kan.turi_dat[i].zen_mai = turi_kwk.turi_dat[i].sin_mai;/* 前回保有枚数<-新規設定枚数	*/
		turi_kan.turi_dat[i].ygen_mai = turi_kwk.turi_dat[i].ysin_mai;/* 前回保有枚数<-新規設定枚数	*/
		turi_kan.turi_dat[i].yzen_mai = turi_kwk.turi_dat[i].ysin_mai;/* 前回保有枚数<-新規設定枚数	*/
	}																/*								*/
	memcpy( &turi_kan.OldTime, &turi_kwk.NowTime, sizeof( date_time_rec ) );	/*					*/

	return;
}																	/* 前回集計日時<-今回集計日時	*/

void turikan_clr_sub( void )
{
	int		i;														/*								*/
	SYUKEI	*ts, *ws;												/*								*/
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
	memcpy( ws, ts, sizeof( SYUKEI ) );								/* Ｔ集計をﾜｰｸｴﾘｱへ（停電対策）	*/
																	/*								*/
	ac_flg.syusyu = 57;												/* 57:補充ﾃﾞｰﾀをT集計へ加算前	*/
																	/*								*/
	ts->hoj[0] += ( (unsigned long)turi_kwk.turi_dat[0].hojyu );	/* 釣銭補充金額  10円使用額		*/
	ts->hoj[1] += ( (unsigned long)turi_kwk.turi_dat[1].hojyu );	/* 釣銭補充金額  50円使用額		*/
	ts->hoj[2] += ( (unsigned long)turi_kwk.turi_dat[2].hojyu );	/* 釣銭補充金額 100円使用額		*/
	ts->hoj[3] += ( (unsigned long)turi_kwk.turi_dat[3].hojyu );	/* 釣銭補充金額 500円使用額		*/
	ts->hojyu = 0L;													/*								*/
	for( i = 0; i < 4; i++ ){										/*								*/
		ts->hojyu += (ts->hoj[i] * coin_vl[i]);						/* 釣銭補充金額	総額			*/
	}																/*								*/
																	/*								*/
	// 以下は釣り合わせで払い出した枚数用
	ts->kyo[0] += ( (unsigned long)turi_dat.outcount[0] );				/* 強制払出金額  10円使用額		*/
	ts->kyo[1] += ( (unsigned long)turi_dat.outcount[1] );				/* 強制払出金額  50円使用額		*/
	ts->kyo[2] += ( (unsigned long)turi_dat.outcount[2] );				/* 強制払出金額 100円使用額		*/
	ts->kyo[3] += ( (unsigned long)turi_dat.outcount[3] );				/* 強制払出金額 500円使用額		*/
	memset( turi_dat.outcount, 0, sizeof( turi_dat.outcount ) ); 	// 出金ｺｲﾝｶｳﾝﾄ(保有枚数変化分) 
	ts->kyosei = 0L;												/*								*/
	for( i = 0; i < 4; i++ ){										/*								*/
		ts->kyosei += (ts->kyo[i] * coin_vl[i]);					/* 強制払出金額	総額			*/
	}																/*								*/

	return;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ｲﾝﾍﾞﾝﾄﾘ枚数をＴ集計に加算																	   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: turikan_inventry( prm )														   |*/
/*| PARAMETER	: short	prm=0:ﾕｰｻﾞｰﾊﾟﾗﾒｰﾀのｲﾝﾍﾞﾝﾄﾘ、=1:ﾒｯｸﾎﾞﾀﾝのｲﾝﾍﾞﾝﾄﾘ							   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimoto(AMANO)															   |*/
/*| Date		: 2005-12-25																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void turikan_inventry( short prm )
{
	int	i;
	SYUKEI	*ts, *ws;

	ts = &sky.tsyuk;
	ws = &wksky;
	memcpy( ws, ts, sizeof( SYUKEI ) );								/* Ｔ集計をﾜｰｸｴﾘｱへ（停電対策）	*/
	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );

	if( prm == 0 ){

		// ﾕｰｻﾞｰﾊﾟﾗﾒｰﾀ操作によるｲﾝﾍﾞﾝﾄﾘ
		ac_flg.syusyu = 60;											/* 60:強制払出枚数をT集計へ加算前	*/

		for( i = 0; i < 4; i++ ){
			turi_kan.turi_dat[i].kyosei += turi_dat.outcount[i];	/* 強制払出金額  10～500円使用額	*/
		}
		turi_kan.turi_dat[0].ykyosei += turi_dat.outcount[4];		/* 強制払出金額  10円使用額		*/
		turi_kan.turi_dat[1].ykyosei += turi_dat.outcount[5];		/* 強制払出金額 50円使用額		*/
		turi_kan.turi_dat[2].ykyosei += turi_dat.outcount[6];		/* 強制払出金額 100円使用額		*/

		for( i = 0; i < 4; i++ ){
			ts->kyo[i] += ( (unsigned long)turi_dat.outcount[i] );	/* 強制払出金額  10円使用額		*/
		}
		ts->kyo[0] += ( (unsigned long)turi_dat.outcount[4] );		/* 強制払出金額  10円予蓄使用額 */
		ts->kyo[1] += ( (unsigned long)turi_dat.outcount[5] );		/* 強制払出金額 50円予蓄使用額 */
		ts->kyo[2] += ( (unsigned long)turi_dat.outcount[6] );		/* 強制払出金額 100円予蓄使用額 */
		ts->kyosei = 0L;											/*								*/
		for( i = 0; i < 4; i++ ){									/*								*/
			ts->kyosei += (ts->kyo[i] * coin_vl[i]);				/* 強制払出金額	総額			*/
		}															/*								*/

		ac_flg.syusyu = 62;											/* 58:強制払出枚数をT集計へ加算後	*/

	}else{

		// ﾒｯｸ押しﾎﾞﾀﾝによるｲﾝﾍﾞﾝﾄﾘ
		ac_flg.syusyu = 61;											/* 58:強制払出枚数をT集計へ加算前	*/

		for( i = 0; i < 4; i++ ){
			turi_kan.turi_dat[i].kyosei += turi_dat.forceout[i];	/* 強制払出金額  10～500円使用額	*/
		}
		turi_kan.turi_dat[0].ykyosei += turi_dat.forceout[4];		/* 強制払出金額 10円予蓄使用額 */
		turi_kan.turi_dat[1].ykyosei += turi_dat.outcount[5];		/* 強制払出金額 50円使用額		*/
		turi_kan.turi_dat[2].ykyosei += turi_dat.outcount[6];		/* 強制払出金額 100円使用額		*/

		for( i = 0; i < 4; i++ ){
			ts->kyo[i] += turi_dat.forceout[i];						/* 強制払出金額  10～500円使用額 */
		}
		ts->kyo[0] += turi_dat.forceout[4];							/* 強制払出金額  10円予蓄使用額	*/
		ts->kyo[1] += turi_dat.forceout[5];							/* 強制払出金額 50円予蓄使用額 */
		ts->kyo[2] += turi_dat.forceout[6];							/* 強制払出金額 100円予蓄使用額 */
		ts->kyosei = 0L;											/*								*/
		for( i = 0; i < 4; i++ ){									/*								*/
			ts->kyosei += (ts->kyo[i] * coin_vl[i]);				/* 強制払出金額	総額			*/
		}															/*								*/

		ac_flg.syusyu = 62;											/* 58:強制払出枚数をT集計へ加算後	*/
	}
	turikan_gen();

	Make_Log_MnyMng( 31 );											/* 金銭管理ログデータ作成		*/
	Log_regist( LOG_MONEYMANAGE_NT );								/* 金銭管理ログ登録				*/

}

/*[]----------------------------------------------------------------------[]*/
/*| 釣銭管理の復電処理                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turikan_fuk                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_fuk( void )
{
	int	i;

	if( turi_dat.turi_in != 0 ){ /* 釣補充操作中? */

		memcpy( &turi_kan, &turi_kwk, sizeof( TURI_KAN ) );

		if( ( turi_dat.turi_in & 0x0003 ) != 0x0003 ){ /* 入出金あり */
			if( ( turi_dat.turi_in & 0x0001 ) == 0 ){ /* 入出金あり */
				turikan_sfv(); /* 金庫枚数算出 */
			}
			if( CPrmSS[S_KAN][1] ){
				// hojyu, kyosei, kyosei_safeの更新
				if( ( turi_dat.turi_in & 0x0001 ) == 0 )
				{	// 入金あり
					for( i = 0; i < 4; i++ )
					{
						turi_kan.turi_dat[i].hojyu += turi_dat.incount[i];
					}
				}
				if( ( turi_dat.turi_in & 0x0002 ) == 0 )
				{	// 釣り合わせあり
					for( i = 0; i < 4; i++ )
					{
						turi_kan.turi_dat[i].kyosei += turi_dat.outcount[i];
					}
				}
				// sin_mai更新
				for( i = 0; i < 4; i++){
					turi_kan.turi_dat[i].sin_mai = (ushort)bcdbin( SFV_DAT.r_dat07[i] );
				}
				turiadd_hojyu_safe();					// 釣銭補充時金庫搬送枚数の加算
			}
		}
	}
	turi_dat.turi_in = 0;

	switch( ac_flg.turi_syu ){ /* 釣銭管理更新中 */
		case 1: /* ｶｾｯﾄ着時の更新 */
		case 2: /* 現保有枚数算出時 */
			memcpy( &turi_kan, &turi_kwk, sizeof( TURI_KAN ) );
			ac_flg.turi_syu = 0;
			break;
		case 10: /* 金庫枚数算出中 */
			memcpy( SFV_DAT.safe_dt, SFV_DAT.safe_dt_wk, sizeof( SFV_DAT.safe_dt ) );
			turikan_sfv();
			break;
		case 20: /* 金庫枚数をﾜｰｸｴﾘｱへ転送完了 */
			memcpy( SFV_DAT.safe_dt, SFV_DAT.safe_dt_wk, sizeof( SFV_DAT.safe_dt ) );	/* 退避ﾃﾞｰﾀを元に戻す */
			safecl( 7 );	/* 金庫枚数算出 */
			ac_flg.turi_syu = 0;
			break;
		case 22: /* 釣銭管理ｴﾘｱをﾜｰｸへ転送完了 */
			memcpy( &turi_kan, &turi_kwk, sizeof( TURI_KAN ) );	/* 退避ﾃﾞｰﾀを元に戻す */
			ac_flg.turi_syu = 21;
			// no break
		case 21: /* 金庫集計完了 */
			turikan_pay();				/* 精算完了時に入出金枚数（循環のみ）をｶｳﾝﾄする */
			SFV_DAT.safe_cal_do = 0;	/* 金庫枚数計算指示ﾘｾｯﾄ */
			ac_flg.turi_syu = 0;
			break;
		default:
			ac_flg.turi_syu = 0;
			break;
	}

	return;
}
/*[]----------------------------------------------------------------------[]*/
/*| 釣銭管理予蓄情報セット                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turian_subtube_set( void )                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2014-10-28                                              |*/
/*| Update		 :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
void turikan_subtube_set( void )
{
	int	i, ret;

	// NOTE: turi_kan.sub_tube には金額の小さい順に、下位バイトから
	//       予蓄１、予蓄２、という扱いで格納される（存在する分だけ格納）。
	//       その為、実際の装着位置（サブ、補助）とは必ずしも一致しない。

	turi_kan.sub_tube = 0;

	for( i=0; i<3; i++ ){
		ret = subtube_use_check( i );	// 各金種に対する予蓄の割当状況を取得する
		switch( ret ){
		case 1:	// 予蓄１
			turi_kan.sub_tube |= (0x0001<<i);
			break;
		case 2:	// 予蓄２
			turi_kan.sub_tube |= (0x0001<<(8+i));
			break;
		default:
			break;
		}
	}
}
