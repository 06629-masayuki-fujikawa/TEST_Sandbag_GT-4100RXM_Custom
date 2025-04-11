/*[]----------------------------------------------------------------------------------------------[]*/
/*| �s���E�����W�v����																			   |*/
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
/*| �s���E�����o�ɏW�v����																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fus_kyo( void )																   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: short	ret		0=������, 1=�����o��, 2=�s���o��								   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
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
	if (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0)				// ���Z���O��FlashRom�����ݒ��H
		return 0;													// 
	if(( fusei.kensuu != 0 )&&										/* �s���E�����o�ɗL��A����		*/
	   ( ac_flg.cycl_fg == 0 ))										/* �󎚊���?					*/
	{																/*								*/
		iti = fusei.fus_d[0].t_iti;									/*								*/
		pkiti = ryo_buf.pkiti;										/* ���Ԉʒu						*/
		memcpy( &in_bak, &car_in, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &ot_bak, &car_ot, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &in_f_bak, &car_in_f, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &ot_f_bak, &car_ot_f, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &PayDataBack, &PayData, sizeof( Receipt_data ));	/* 1���Z���,�̎��؈��ް�		*/
		tyu_ryo = ryo_buf.tyu_ryo;									/* ���ԗ���						*/
		tax = ryo_buf.tax;											/* �����						*/
		nyukin = ryo_buf.nyukin;									/* �����z						*/
		turisen = ryo_buf.turisen;									/* �ޑK							*/
		bk_ntnet_nmax_flg = ntnet_nmax_flg;							/* �ő嗿���׸��ޯ�����			*/
		if(( iti >= 1 )&&( iti <= LOCK_MAX ))						/* ���Ԉʒu�ԍ�1�ȏ�324�ȉ�?	*/
		{															/*								*/
			ac_flg.cycl_fg = 30;									/*								*/
			cm27();													/*								*/
			if( fus_tim() == 1 ){									/* ���o�Ɏ������				*/
																	/* ����NG?(Y)					*/
																	/*								*/
				ac_flg.cycl_fg = 35;								/* 35:�s���E�����o�ɏ��o�^����*/
																	/*								*/
				memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );/*								*/
																	/*								*/
				ac_flg.cycl_fg = 36;								/* 36:�s���ް���ܰ��ر�֓]������*/
																	/*								*/
				memcpy( &fusei, &wkfus.fus_d[1],					/* �ް����						*/
						sizeof(struct FUSEI_D)*(LOCK_MAX-1) );		/*								*/
				fusei.kensuu -= 1;									/* ����-1						*/
																	/*								*/
				ac_flg.cycl_fg = 37;								/* 37:�ް���āE����-1����		*/
																	/*								*/
				memset( &fusei.fus_d[(LOCK_MAX-1)],					/*								*/
						0, sizeof(struct FUSEI_D));					/*								*/
																	/*								*/
				ac_flg.cycl_fg = 0;									/* 0:6���ڸر����(�ҋ@)			*/
				return ret;											/*								*/
			}														/*								*/
			ryo_buf.credit.pay_ryo = 0;
			ntnet_nmax_flg = 0;										/* �ő嗿���K�p�׸޸ر			*/
			ryo_cal( 50, iti );										/* �s���o�ɗ����v�Z				*/
			ac_flg.cycl_fg = 31;									/*								*/
			ret = fus_syuu();										/* �W�v���Z						*/
																	/* 								*/
		} else {													/* �Ԏ��ԍ����z��O?(Y)			*/
																	/* 								*/
			ac_flg.cycl_fg = 35;									/* 35:�s���E�����o�ɏ��o�^����*/
																	/*								*/
			memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );	/*								*/
																	/*								*/
			ac_flg.cycl_fg = 36;									/* 36:�s���ް���ܰ��ر�֓]������*/
																	/*								*/
			memcpy( &fusei, &wkfus.fus_d[1],						/* �ް����						*/
					sizeof(struct FUSEI_D)*(LOCK_MAX-1) );			/*								*/
			fusei.kensuu -= 1;										/* ����-1						*/
																	/*								*/
			ac_flg.cycl_fg = 37;									/* 37:�ް���āE����-1����		*/
																	/*								*/
			memset( &fusei.fus_d[(LOCK_MAX-1)],						/*								*/
					0, sizeof(struct FUSEI_D));						/*								*/
																	/*								*/
			ac_flg.cycl_fg = 38;									/* 38:6���ڸر����				*/
																	/*								*/
			ex_errlg( ERRMDL_MAIN, ERR_MAIN_LOCKNUMNG, 2, 0 );		/* �s���E�����o�Ɉʒu1�`324�ȊO	*/
		}															/*								*/
		ac_flg.cycl_fg = 0;											/*								*/
		ryo_buf.pkiti = pkiti;										/* ���Ԉʒu						*/
		memcpy( &car_in, &in_bak, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &car_ot, &ot_bak, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &car_in_f, &in_f_bak, sizeof( struct CAR_TIM ));	/*								*/
		NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
		memcpy( &car_ot_f, &ot_f_bak, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &PayData, &PayDataBack, sizeof( Receipt_data ));	/* 1���Z���,�̎��؈��ް�		*/
		NTNET_Data152_SaveDataUpdate();
		ryo_buf.tyu_ryo = tyu_ryo;									/* ���ԗ���						*/
		ryo_buf.tax = tax;											/* �����						*/
		ryo_buf.nyukin = nyukin;									/* �����z						*/
		ryo_buf.turisen = turisen;									/* �ޑK							*/
		ntnet_nmax_flg = bk_ntnet_nmax_flg;							/* �ő嗿���׸�ؽı				*/
																	/*								*/
	}																/*								*/
	return ret;														/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���o�Ɏ����Z�b�g																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fus_tim( num )																   |*/
/*| PARAMETER	: num	: ���Ԉʒu																   |*/
/*| RETURN VALUE: ret	: ���o�Ɏ�������(1980�`2079�N�ȓ����H)  0:OK  1:NG						   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*|				car_in,car_ot,car_in_f,car_ot_f�ɓ��o�Ɏ������Z�b�g����							   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short	fus_tim( void )										/*								*/
{																	/*								*/
	ushort	in_tim, out_tim;										/*								*/
																	/*								*/
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					/* ���Ԏ����ر					*/
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					/* �o�Ԏ����ر					*/
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				/* ���Ԏ����ر					*/
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				/* �o�Ԏ����ر					*/
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			/* �Čv�Z�p���Ԏ����ر			*/
																	/*								*/
	car_in.year = fusei.fus_d[0].iyear;								/* ����	�N						*/
	car_in.mon  = fusei.fus_d[0].imont;								/*		��						*/
	car_in.day  = fusei.fus_d[0].idate;								/*		��						*/
	car_in.hour = fusei.fus_d[0].ihour;								/*		��						*/
	car_in.min  = fusei.fus_d[0].iminu;								/*		��						*/
	car_in.week = (char)youbiget( car_in.year,						/*		�j��					*/
								(short)car_in.mon,					/*								*/
								(short)car_in.day );				/*								*/
						 											/*								*/
	car_ot.year = fusei.fus_d[0].oyear;								/* �o��	�N						*/
	car_ot.mon  = fusei.fus_d[0].omont;								/*		��						*/
	car_ot.day  = fusei.fus_d[0].odate;								/*		��						*/
	car_ot.hour = fusei.fus_d[0].ohour;								/*		��						*/
	car_ot.min  = fusei.fus_d[0].ominu;								/*		��						*/
	car_ot.week = (char)youbiget( car_ot.year,						/*		�j��					*/
								(short)car_ot.mon,					/*								*/
								(short)car_ot.day );				/*								*/
	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			/* ���Ԏ���Fix					*/
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			/* �o�Ԏ���Fix					*/
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		/* �Čv�Z�p���Ԏ���Fix			*/
																	/*								*/
	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	/* ���Ɏ����K��O?			*/
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	/* �o�Ɏ����K��O?			*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKINOUTTIMENG, 2, 0 );		/* ���o�Ɏ����K��O				*/
		return( 1 );												/*								*/
	}																/*								*/
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					/* (���ɓ�+1�N)normlize			*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	out_tim = dnrmlzm( car_ot.year,									/* �o�ɓ�normlize				*/
					(short)car_ot.mon,								/*								*/
					(short)car_ot.day );							/*								*/
	if( in_tim <= out_tim ){										/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKOVER1YEARCAL, 2, 1 );		/* 1�N�ȏ�̗����v�Z���s�����Ƃ��� */
		return( 1 );												/*								*/
	}																/*								*/
	in_tim = dnrmlzm((short)( car_in.year ),						/* (���ɓ�)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){											/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKINOUTGYAKU, 2, 1 );		/* ���o�ɋt�]					*/
		return( 1 );												/*								*/
	}																/*								*/
	return( 0 );													/*								*/
}																	/*								*/
