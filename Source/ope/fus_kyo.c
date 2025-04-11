/*[]----------------------------------------------------------------------------------------------[]*/
/*| 不正・強制集計処理																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"mem_def.h"
#include	"prm_tbl.h"
#include	"flp_def.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"AppServ.h"
#include	"ntnet_def.h"

static short fus_tim( void );

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 不正・強制出庫集計処理																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fus_kyo( void )																   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: short	ret		0=未処理, 1=強制出庫, 2=不正出庫								   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	fus_kyo( void )												/*								*/
{																	/*								*/
	ushort	iti = 0;												/*								*/
	struct	CAR_TIM		in_bak;										/*								*/
	struct	CAR_TIM		ot_bak;										/*								*/
	struct	CAR_TIM		in_f_bak;									/*								*/
	struct	CAR_TIM		ot_f_bak;									/*								*/
	ulong	tyu_ryo, tax, nyukin, turisen;							/*								*/
	ushort	pkiti;													/*								*/
	uchar	bk_ntnet_nmax_flg;
	short	ret = 1;
																	/*								*/
	if (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0)				// 精算ログのFlashRom書込み中？
		return 0;													// 
	if(( fusei.kensuu != 0 )&&										/* 不正・強制出庫有り、かつ		*/
	   ( ac_flg.cycl_fg == 0 ))										/* 印字完了?					*/
	{																/*								*/
		iti = fusei.fus_d[0].t_iti;									/*								*/
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
		bk_ntnet_nmax_flg = ntnet_nmax_flg;							/* 最大料金ﾌﾗｸﾞﾊﾞｯｸｱｯﾌﾟ			*/
		if(( iti >= 1 )&&( iti <= LOCK_MAX ))						/* 駐車位置番号1以上324以下?	*/
		{															/*								*/
			ac_flg.cycl_fg = 30;									/*								*/
			cm27();													/*								*/
			if( fus_tim() == 1 ){									/* 入出庫時刻ｾｯﾄ				*/
																	/* 判定NG?(Y)					*/
																	/*								*/
				ac_flg.cycl_fg = 35;								/* 35:不正・強制出庫情報登録完了*/
																	/*								*/
				memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );/*								*/
																	/*								*/
				ac_flg.cycl_fg = 36;								/* 36:不正ﾃﾞｰﾀをﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
				memcpy( &fusei, &wkfus.fus_d[1],					/* ﾃﾞｰﾀｼﾌﾄ						*/
						sizeof(struct FUSEI_D)*(LOCK_MAX-1) );		/*								*/
				fusei.kensuu -= 1;									/* 件数-1						*/
																	/*								*/
				ac_flg.cycl_fg = 37;								/* 37:ﾃﾞｰﾀｼﾌﾄ・件数-1完了		*/
																	/*								*/
				memset( &fusei.fus_d[(LOCK_MAX-1)],					/*								*/
						0, sizeof(struct FUSEI_D));					/*								*/
																	/*								*/
				ac_flg.cycl_fg = 0;									/* 0:6件目ｸﾘｱ完了(待機)			*/
				return ret;											/*								*/
			}														/*								*/
			ryo_buf.credit.pay_ryo = 0;
			ntnet_nmax_flg = 0;										/* 最大料金適用ﾌﾗｸﾞｸﾘｱ			*/
			ryo_cal( 50, iti );										/* 不正出庫料金計算				*/
			ac_flg.cycl_fg = 31;									/*								*/
			ret = fus_syuu();										/* 集計加算						*/
																	/* 								*/
		} else {													/* 車室番号が想定外?(Y)			*/
																	/* 								*/
			ac_flg.cycl_fg = 35;									/* 35:不正・強制出庫情報登録完了*/
																	/*								*/
			memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );	/*								*/
																	/*								*/
			ac_flg.cycl_fg = 36;									/* 36:不正ﾃﾞｰﾀをﾜｰｸｴﾘｱへ転送完了*/
																	/*								*/
			memcpy( &fusei, &wkfus.fus_d[1],						/* ﾃﾞｰﾀｼﾌﾄ						*/
					sizeof(struct FUSEI_D)*(LOCK_MAX-1) );			/*								*/
			fusei.kensuu -= 1;										/* 件数-1						*/
																	/*								*/
			ac_flg.cycl_fg = 37;									/* 37:ﾃﾞｰﾀｼﾌﾄ・件数-1完了		*/
																	/*								*/
			memset( &fusei.fus_d[(LOCK_MAX-1)],						/*								*/
					0, sizeof(struct FUSEI_D));						/*								*/
																	/*								*/
			ac_flg.cycl_fg = 38;									/* 38:6件目ｸﾘｱ完了				*/
																	/*								*/
			ex_errlg( ERRMDL_MAIN, ERR_MAIN_LOCKNUMNG, 2, 0 );		/* 不正・強制出庫位置1～324以外	*/
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
		ntnet_nmax_flg = bk_ntnet_nmax_flg;							/* 最大料金ﾌﾗｸﾞﾘｽﾄｱ				*/
																	/*								*/
	}																/*								*/
	return ret;														/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 入出庫時刻セット																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fus_tim( num )																   |*/
/*| PARAMETER	: num	: 駐車位置																   |*/
/*| RETURN VALUE: ret	: 入出庫時刻判定(1980～2079年以内か？)  0:OK  1:NG						   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*|				car_in,car_ot,car_in_f,car_ot_fに入出庫時刻をセットする							   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short	fus_tim( void )										/*								*/
{																	/*								*/
	ushort	in_tim, out_tim;										/*								*/
																	/*								*/
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					/* 入車時刻ｸﾘｱ					*/
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					/* 出車時刻ｸﾘｱ					*/
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				/* 入車時刻ｸﾘｱ					*/
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				/* 出車時刻ｸﾘｱ					*/
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			/* 再計算用入車時刻ｸﾘｱ			*/
																	/*								*/
	car_in.year = fusei.fus_d[0].iyear;								/* 入車	年						*/
	car_in.mon  = fusei.fus_d[0].imont;								/*		月						*/
	car_in.day  = fusei.fus_d[0].idate;								/*		日						*/
	car_in.hour = fusei.fus_d[0].ihour;								/*		時						*/
	car_in.min  = fusei.fus_d[0].iminu;								/*		分						*/
	car_in.week = (char)youbiget( car_in.year,						/*		曜日					*/
								(short)car_in.mon,					/*								*/
								(short)car_in.day );				/*								*/
						 											/*								*/
	car_ot.year = fusei.fus_d[0].oyear;								/* 出庫	年						*/
	car_ot.mon  = fusei.fus_d[0].omont;								/*		月						*/
	car_ot.day  = fusei.fus_d[0].odate;								/*		日						*/
	car_ot.hour = fusei.fus_d[0].ohour;								/*		時						*/
	car_ot.min  = fusei.fus_d[0].ominu;								/*		分						*/
	car_ot.week = (char)youbiget( car_ot.year,						/*		曜日					*/
								(short)car_ot.mon,					/*								*/
								(short)car_ot.day );				/*								*/
	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			/* 入車時刻Fix					*/
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			/* 出車時刻Fix					*/
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		/* 再計算用入車時刻Fix			*/
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
