/*[]----------------------------------------------------------------------[]*/
/*| Refund Procces                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 95-07-28                                                 |*/
/*| Update      : 2001-11-01                                               |*/
/*|  V25->H8S convert                                                      |*/
/*|  Coinmech and notereader are changed the one manufactured by CONLAX.   |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/

#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"

/*[]----------------------------------------------------------------------[]*/
/*| Refund Coin and Note                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ret = refund( val )                                     |*/
/*| PARAMETER	 : long  val : Refund Value                                |*/
/*| RETURN VALUE : short ret : 0:OK -1:NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short
refund( long val )
{
	memset( ryo_buf.out_coin, 0, sizeof(ryo_buf.out_coin) );
	SFV_DAT.refval = 0;
	SFV_DAT.reffal = val;
	if( val == 0 ){
		queset( OPETCBNO, (unsigned short)COIN_OT_EVT, 0, NULL ); /* 払出し完了とする */
		if( NT_RDAT.r_dat18[0] != 0 ){	/* In Escrow ? */
			nt_com = 0x80 + 4;	/* ｴｽｸﾛ紙幣取り込み */
			return( 0 );
		}
	}
	if( CN_RDAT.r_dat0b & 0x02 ){	/* Inventory ? NJ */
		return( -1 );
	}

	CN_refund = 0;
	if( NT_RDAT.r_dat18[0] != 0 ){	/* In Escrow ? */
		if( val >= 1000L ){
			SFV_DAT.reffal = val - 1000L;
			nt_com = 0x80 + 10;	/* Refund Req. Set */
			CN_refund |= 2;
		}else{
			nt_com = 0x80 + 4;	/* ｴｽｸﾛ紙幣取り込み */
		}
	}
	if( cn_errst[0] ){										/* コインメック故障中?(Y)			*/
		if((( CN_refund & 2) == 2)&&( cn_errst[1] == 0 )){	/* 紙幣払い出しありかつ紙幣正常?(Y)	*/
			if( ac_flg.cycl_fg >= 10 ){						/*									*/
				ac_flg.cycl_fg += 1;						/* 精算完了(12)又は精算中止(22) 	*/
			}												/*									*/
			return( 0 );									/* 紙幣払い出しありの為正常とする	*/
		}else{												/*									*/
			return( -1 );									/* コインメック故障の為払い出し不可	*/
		}													/*									*/
	}														/*									*/
	if( SFV_DAT.reffal ){
		cn_com = 0x80 + 9;	/* Refund Req. Set */
		CN_refund |= 1;
	}
	if( ac_flg.cycl_fg >= 10 ){
		ac_flg.cycl_fg += 1;	/* 精算完了(12)又は精算中止(22) */
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*|	Calculate Shortage Amount of Refund Fault                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : void refund( void )                                     |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
refalt( void )
{
	short	i;
	short	val;
	unsigned char	*r_dat;

	if( SFV_DAT.refval ){
		r_dat = SFV_DAT.r_dat0a;
		val = 0;
		for( i=0; i<4; i++ ,r_dat++ ){
			val += (short)( bcdbin( *r_dat ) * coin_vl[i] );
		}

		if( SFV_DAT.refval > (long)val ){ /* 不足額がﾏｲﾅｽ印字になった為、ｶﾞｰﾄﾞしておく 02/03/07 */
			SFV_DAT.reffal += ( SFV_DAT.refval - (long)val );
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|	Change Status Coin and Note                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ret = cn_stat( mod, no )                                |*/
/*| PARAMETER    : short mod : Order Kind                                  |*/
/*|                short no  : 0:Coinmech 1:Note-reader 2:Both             |*/
/*| RETURN VALUE : short ret : 0:success -1:NG <>0:NG                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short
cn_stat( short mod, short no )
{
	switch( no ){
		case 0: /* Coin Mech */
			if(( mod == 2 )||( mod == 4)){				/* 	入金不可?(Y)紙幣取込?(Y)*/	//コメント復活
														/*							*/
			}else{										/*							*/
				if( cn_errst[0] ){						/* Coin mech Trouble ?(Y)	*/
					return( -1 );						/*							*/
				}										/*							*/
			}											/*							*/
			cn_com = (unsigned char)(0x80 + mod);
			CNMTSK_START = 1;
			break;
		case 1: /* Note Reader */
			if(( mod == 2 )||( mod == 4)){				/* 	入金不可?(Y)紙幣取込?(Y)*/
														/*							*/
			}else{										/*							*/
				if( cn_errst[1] ){						/* Reader Trouble ?(Y)		*/
					return( -1 );						/*							*/
				}										/*							*/
			}											/*							*/
			if(( nt_com & 0x80 ) == 0 ){
				nt_com = (unsigned char)(0x80 + mod);
				CNMTSK_START = 1;
			}
			break;
		case 2: /* Both */
			if(( cn_errst[0] == 0 )||( mod == 2 )||( mod == 4)){/* Coin mech OK ?(Y) */
														/*	 						*/
														/*	 						*/
				cn_com = (unsigned char)(0x80 + mod);
				CNMTSK_START = 1;
			}
			if(( cn_errst[1] == 0 )||( mod == 2 )||( mod == 4)){/* Reader OK?(Y)	*/	//コメント復活
														/*	 						*/
														/*	 						*/
				nt_com = (unsigned char)(0x80 + mod);
				CNMTSK_START = 1;
			}
			break;
		default:
			break;
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Calculate Safe Coin Credit                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : void = safecl( prm )                                    |*/
/*| PARAMETER	 : prm                                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void safecl( short prm )
{
	short	i, c;

	switch( prm ){
		case 0: /* 入金前の初期化 */
			memset( turi_dat.pay_safe, 0, sizeof( turi_dat.pay_safe ) );
			memset( SFV_DAT.in_dat07, 0, sizeof( SFV_DAT.in_dat07 ) );
			memset( SFV_DAT.out_dat07, 0, sizeof( SFV_DAT.out_dat07 ) );
			memset( SFV_DAT.r_add08, 0, sizeof( SFV_DAT.r_add08 ) );
			memset( SFV_DAT.r_add0a, 0, sizeof( SFV_DAT.r_add0a ) );
			SFV_DAT.safe_inout = 0;
			SFV_DAT.cn_escrow = 0; 		/* ｴｽｸﾛ枚数ｸﾘｱ */
			break;
		case 1: /* 入金完了後 */
			for( i = 0; i < 4; i++ ){
				SFV_DAT.in_dat07[i] = (short)bcdbin( SFV_DAT.r_dat07[i] );
			}
			break;
		case 7: /* 金庫入金枚数算出 */
			if( SFV_DAT.safe_cal_do == 0 ){ /* 金庫枚数算出済み? */
											/* 入出金がないので以降の処理は不要 */
				ac_flg.turi_syu = 23;		/* 23:金庫枚数算出、釣銭管理集計完了 */
				break;
			}

			memcpy( SFV_DAT.safe_dt_wk, SFV_DAT.safe_dt, sizeof( SFV_DAT.safe_dt_wk ) );
			ac_flg.turi_syu = 20;

			if( SFV_DAT.safe_inout == 0 ){ 
				/* 払出しを行っていない */
				for( i = 0; i < 4; i++ ){
					SFV_DAT.out_dat07[i] = SFV_DAT.in_dat07[i] - (short)bcdbin( SFV_DAT.r_dat07[i] );
				}
			}else{
				/* 払出しを行っている */
				for( i = 0; i < 4; i++ ){
					SFV_DAT.out_dat07[i] = (short)bcdbin( sv_coin_max[i] ) - (short)bcdbin( SFV_DAT.r_dat07[i] );
				}
			}
			for( i = 0; i < 4; i++ ){
				if( SFV_DAT.r_add08[i] != 0 ){	/* Coin Credit ? NJ */
					c = (short)( SFV_DAT.r_add08[i] ); 		/* 総入金枚数 */
					c -= ((short)( SFV_DAT.in_dat07[i] ));	/* 総入金枚数-筒増加分 */
					if( c > 0 ){
						SFV_DAT.safe_dt[i] += c;				/* 今回金庫分を足す */
						turi_dat.pay_safe[i] = (short)c;
					}
				}
			}

			ac_flg.turi_syu = 21;

			turikan_pay(); /* 釣銭管理へ精算時入出金枚数を加算 */
			SFV_DAT.safe_cal_do = 0;	/* 金庫枚数計算指示ﾘｾｯﾄ */
			break;
		default:
			break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Calculate Coin Credit                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ret = cn_crdt( void )                                   |*/
/*| PARAMETER    : short not :Note Input Count                             |*/
/*| RETURN VALUE : long ret :Credit Value                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
long
cn_crdt( void )
{
	short	i;
	long	crdt, wk;

	crdt = 0l;
	for( i = 0; i < 4; i++ ){
		wk = ( (long)ryo_buf.in_coin[i] );
		crdt += ( (long)( wk * coin_vl[i] ) );
	}
	crdt += ( (long)( ryo_buf.in_coin[4] * 1000l ) );
	ryo_buf.nyukin = crdt;
	SFV_DAT.powoff = 0;
	return( crdt );
}

/*[]----------------------------------------------------------------------[]*/
/*| Initial Memory                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : void cnm_mem( mod )                                     |*/
/*| PARAMETER    : short mod | 0:Password Broken 1:Not Broken              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
cnm_mem( short mod )
{

	if( mod == 0 ){	/* Password Broken */
		memset( &SFV_DAT, 0, sizeof( sfv_rec ));
		SFV_SNDSTS.BYTE = 0;		// 払出し要求ﾌﾗｸﾞクリア
	}else{
		if( SFV_DAT.nt_escrow ){ /* 紙幣ｴｽｸﾛ中? */
			SFV_DAT.nt_escrow = 0;
		}
		if( SFV_DAT.powoff ){
			ryo_buf.nyukin = cn_crdt();
		}
	}
	CN_SDAT.s_dat00 = 0x40;	/* 自動釣り合わせﾓｰﾄﾞとする */

}

/*[]----------------------------------------------------------------------[]*/
/*| ｴｽｸﾛｺｲﾝの落とし待ちﾀｲﾏをｾｯﾄ。ﾀｲﾑｱｳﾄで金庫枚数算出                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : void set_escrowtimer( void )                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2005-12-10                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
set_escrowtimer( void )
{
	_di();
	CN_escr_timer = ( 50 * CN_ESCROW );					// ｴｽｸﾛﾀｲﾏｰｾｯﾄ
	_ei();

}
/*[]----------------------------------------------------------------------[]*/
/*| 現在の入金額の取得のみ行う（ryo_buf等は更新しない）                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ret = cn_crdt( void )                                   |*/
/*| PARAMETER    : short not :Note Input Count                             |*/
/*| RETURN VALUE : long ret :Credit Value                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Takahashi                                             |*/
/*| Date         : 2012-05-28                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
long cn_GetTempCredit( void )
{
	short	i;
	long	crdt, wk;

	crdt = 0l;
	for( i = 0; i < 4; i++ ){
		wk = ( (long)ryo_buf.in_coin[i] );
		crdt += ( (long)( wk * coin_vl[i] ) );
	}
	crdt += ( (long)( ryo_buf.in_coin[4] * 1000l ) );
	return crdt;
}
