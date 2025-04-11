/*[]----------------------------------------------------------------------------------------------[]*/
/*| 券ﾃﾞｰﾀ処理																					   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"tbl_rkn.h"
#include	"mem_def.h"
#include	"flp_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mif.h"
#include	"LKmain.h"
#include	"prm_tbl.h"
#include	"pri_def.h"


static char	datenearend( char *edate );

// MH322914 (s) kasiyama 2016/07/15 AI-V対応
//[]----------------------------------------------------------------------[]
///	@brief			紛失券／紛失ﾎﾞﾀﾝﾃﾞｰﾀ処理(料金計算用に変換する)
//[]----------------------------------------------------------------------[]
///	@param[in]		f_Button 1=紛失ﾎﾞﾀﾝ、0=紛失券
///	@param[in]		mag		磁気ｶｰﾄﾞﾃﾞｰﾀ
///	@return			ret		0 = OK
///	@author			R.Hara
///	@note			券種 2Bhに対応	
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/10/01<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
int		vl_funchg( char f_Button, m_gtticstp *mag )
{
	cr_dat_n = RID_FUN/*0x2B*/;										// 券種ｾｯﾄ
	vl_now = V_FUN;
	memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );

	car_in.mon   = vl_tik.tki;										//   月
	car_in.day   = vl_tik.hii;										//   日
	car_in.hour  = vl_tik.jii;										//   時
	car_in.min   = vl_tik.fun;										//   分
	car_in.year  = CLK_REC.year;
	if( (car_in.mon > CLK_REC.mont) ||								/* 入車月日＞現在月日？ */
		((car_in.mon == CLK_REC.mont) && (car_in.day > CLK_REC.date)) ){
		car_in.year--;
	}
	car_in.week  = (char)youbiget( car_in.year ,					//     曜日
							(short)car_in.mon  ,
							(short)car_in.day  );
	return	0;
}
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 駐車券ﾃﾞｰﾀ処理(料金計算用に変換する)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_carchg( no, chg_flg )														   |*/
/*| PARAMETER	: no	; 内部処理用駐車位置番号(1～324)										   |*/
/*|				: chg_flg	; 変換種別															   |*/
/*|							;   0:通常															   |*/
/*|							;   1,2:料金計算シミュレータ										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	vl_carchg( ushort no , uchar chg_flg )
{
// MH810100(S) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	flp_com	*lk;
//	ulong	posi;
//	uchar	i;
//	ushort	wari_main = 0;
// MH810100(E) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

	if(( no >= 1 && no <= LOCK_MAX )||( no == 0xffff )){		// 駐車位置番号ﾁｪｯｸ(1～LOCK_MAX or ﾏﾙﾁ精算)

		memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );		// 駐車券ﾃﾞｰﾀ(vl_tik)初期化(0ｸﾘｱ)
// MH810100(S) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		if( no == 0xffff ){										// ﾏﾙﾁ精算問合せ中?
//			lk = &LOCKMULTI.lock_mlt;
//			vl_tik.syu = (char)lk->ryo_syu;						// 種別ｾｯﾄ(ﾏﾙﾁ精算用)
//			Flap_Sub_Num = 10;									// ﾏﾙﾁ精算
//		}else{
//			lk = &FLAPDT.flp_data[no-1];
//			vl_tik.syu = LockInfo[no-1].ryo_syu;				// 種別ｾｯﾄ(駐車位置設定毎)
//		}
//
//		if (chg_flg != 0) {										// 料金計算シミュレータ
//		// 料金計算シミュレータ中は中止データを参照しない
//			cr_dat_n = 0x24;									// 券IDｺｰﾄﾞｾｯﾄ(精算前券)
//			vl_now = V_CHM;										// 読込み券種別ｾｯﾄ(精算前券)
//		}
//		else 
//		if( lk->bk_syu ){										// 種別(中止,修正用)
//			cr_dat_n = 0x26;									// 券IDｺｰﾄﾞｾｯﾄ(精算中止券)
//			vl_now = V_CHS;										// 読込み券種別ｾｯﾄ(精算中止券)
//
//			vl_tik.syu = (uchar)lk->bk_syu;						// 種別ｾｯﾄ(精算中止時保存ﾃﾞｰﾀ)
//
//			card_use[USE_SVC] = (uchar)lk->bk_wmai;				// 使用枚数(中止,修正用)
//			vl_tik.wari = lk->bk_wari;							// 割引金額(中止,修正用)
//			vl_tik.time = lk->bk_time;							// 割引時間数(中止,修正用)
//			vl_tik.pst = (uchar)lk->bk_pst;						// ％割引率(中止,修正用)
//		}else{
//			// 精算状態が「精算中止」以外の場合
//			cr_dat_n = 0x24;									// 券IDｺｰﾄﾞｾｯﾄ(精算前券)
//			vl_now = V_CHM;										// 読込み券種別ｾｯﾄ(精算前券)
//		}
		cr_dat_n = 0x24;									// 券IDｺｰﾄﾞｾｯﾄ(精算前券)
		vl_now = V_CHM;										// 読込み券種別ｾｯﾄ(精算前券)

		vl_tik.syu = syashu;
// MH810100(E) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

		vl_tik.chk = 0;											// 精算前券ｾｯﾄ

		vl_tik.cno = KIHON_PKNO;								// 駐車場№ｾｯﾄ(基本駐車場№)
		vl_tik.ckk = (short)CPrmSS[S_PAY][2];					// 発券機No.
		vl_tik.hno = 0;											// 発券No.
		vl_tik.mno = 0;											// 店№ｾｯﾄ

		vl_tik.tki = car_in_f.mon;								// 入車月ｾｯﾄ
		vl_tik.hii = car_in_f.day;								// 入車日ｾｯﾄ
		vl_tik.jii = car_in_f.hour;								// 入車時ｾｯﾄ
		vl_tik.fun = car_in_f.min;					 			// 入車分ｾｯﾄ
// MH810100(S) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		syashu       = vl_tik.syu	;							// 車種
// MH810100(E) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		hzuk.y = car_ot_f.year;
		hzuk.m = car_ot_f.mon;
		hzuk.d = car_ot_f.day;

		hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

		jikn.t = car_ot_f.hour;
		jikn.m = car_ot_f.min;

		jikn.s = 0;

		car_in.year  = car_in_f.year;							// 入車年
		car_in.mon   = vl_tik.tki;								//     月
		car_in.day   = vl_tik.hii;								//     日
		car_in.hour  = vl_tik.jii;								//     時
		car_in.min   = vl_tik.fun;								//     分
		car_in.week  = (char)youbiget( car_in.year ,			//     曜日
								(short)car_in.mon  ,
								(short)car_in.day  );
		car_ot.year = hzuk.y;									// 出庫年
		car_ot.mon  = hzuk.m;									//     月
		car_ot.day  = hzuk.d;									//     日
		car_ot.week = hzuk.w;									//     曜日
		car_ot.hour = jikn.t;									//     時
		car_ot.min  = jikn.m;									//     分
// MH810100(S) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		if (chg_flg != 0) {										// 料金計算シミュレータ
//		// 料金計算シミュレータ中は中止データを参照しない
//			Flap_Sub_Flg = 0;
//		}
//		else 
//		if( lk->bk_syu ){										// 種別(中止,修正用)
//			if( no == 0xffff ){									// ﾏﾙﾁ精算
//				//lock_multiを詳細中止バッファに格納する。
//				wari_main = 0;
//				Flap_Sub_Flg = 0;								//ﾌﾗｸﾞｸﾘｱ
//				memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));	//ｸﾘｱ
//				memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));
//				FLAPDT_SUB[10].TInTime.Year=LOCKMULTI.lock_mlt.year;
//				FLAPDT_SUB[10].TInTime.Mon=LOCKMULTI.lock_mlt.mont;
//				FLAPDT_SUB[10].TInTime.Day=LOCKMULTI.lock_mlt.date;
//				FLAPDT_SUB[10].TInTime.Hour=LOCKMULTI.lock_mlt.hour;
//				FLAPDT_SUB[10].TInTime.Min=LOCKMULTI.lock_mlt.minu;
//				memcpy(&FLAPDT_SUB[10].sev_tik,&LOCKMULTI.sev_tik_mlt,sizeof(uchar)*15);			//ｻｰﾋﾞｽ券枚数格納
//				memcpy(&FLAPDT_SUB[10].kake_data,&LOCKMULTI.kake_data_mlt,sizeof(kake_tiket)*5);	//掛売券枚数格納
//				FLAPDT_SUB[10].ppc_chusi_ryo = LOCKMULTI.ppc_chusi_ryo_mlt;							//プリペイド金額格納
//				FLAPDT_SUB[10].syu = (uchar)LOCKMULTI.lock_mlt.bk_syu;								//料金種別格納
//				Flap_Sub_Num = 10;																	//ﾏﾙﾁ精算用格納位置
//				for(i=0;i<15;i++){
//					if(FLAPDT_SUB[10].sev_tik[i] != 0){					//ｻｰﾋﾞｽ券あり
//						Flap_Sub_Flg = 1;								//Bｴﾘｱ使用フラグON
//						wari_main += FLAPDT_SUB[10].sev_tik[i];			//使用枚数加算
//					}
//				}
//				for(i=0;i<5;i++){
//					if(FLAPDT_SUB[10].kake_data[i].maisuu != 0){			//掛売券あり
//						Flap_Sub_Flg = 1;									//Bｴﾘｱ使用フラグON
//						wari_main += FLAPDT_SUB[10].kake_data[i].maisuu;	//使用枚数加算
//					}
//				}
//				if(wari_main != LOCKMULTI.lock_mlt.bk_wmai){		//使用枚数が正常で無い時
//					Flap_Sub_Flg = 0;								//ﾌﾗｸﾞｸﾘｱ
//				}
//				if((FLAPDT_SUB[10].ppc_chusi_ryo)&&(LOCKMULTI.lock_mlt.bk_wari)){				//プリペイドあり
//					if(LOCKMULTI.lock_mlt.bk_wari >= FLAPDT_SUB[10].ppc_chusi_ryo){
//						Flap_Sub_Flg = 1;														//Bｴﾘｱ使用フラグON
//					}else{
//						Flap_Sub_Flg = 0;								//ﾌﾗｸﾞｸﾘｱ
//					}
//				}
//			}else{
//				posi = LockInfo[no-1].posi;						//内部番号より駐車位置を取得
//				vl_cyushiset( posi );
//			}
//		}else{
//			Flap_Sub_Flg = 0;
//		}
// MH810100(E) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	}
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| 定期券処理(料金計算用に変換する)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_paschg( mag )																   |*/
/*| PARAMETER	: mag	; 磁気ﾘｰﾀﾞｰ受信ﾃﾞｰﾀ														   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short 	pno_dt[8] = {
	0x0000, 0x0200, 0x0100, 0x0300, 0x0080,0x0280, 0x0180, 0x0380
};

short	vl_paschg( void )
{
	short	ret = 0;
	char	c_prm;
	short	cnt;

	cr_dat_n = RID_APS;										// ID Code(1AH)
	memset( &tsn_tki, 0x00, sizeof( struct TSN_TKI) );

	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(CRD_DAT.PAS.pno == CPrmSS[S_SYS][cnt]){
		tsn_tki.pkno = cnt-1;								// 駐車場№種別ｾｯﾄ
		break;
		}
	}
	tsn_tki.kind = (char)(CRD_DAT.PAS.knd);						// 種別ｾｯﾄ
	tsn_tki.code = CRD_DAT.PAS.cod;								// 個人ｺｰﾄﾞｾｯﾄ

	c_prm = (char)prm_get( COM_PRM,S_PAS,						// 無期限設定Get
			(short)(5+10*(CRD_DAT.PAS.knd-1)),1,2 );
	if( c_prm == 1 || c_prm == 2 ){								// 無期限or開始日無期限
		tsn_tki.data[0] = 90;									// 1990/1/1ｾｯﾄ
		tsn_tki.data[1] = 1;
		tsn_tki.data[2] = 1;
	}else{
		memcpy( &tsn_tki.data[0], &CRD_DAT.PAS.std_end[0], 3 );
	}

	if( c_prm == 1 || c_prm == 3 ){								// 無期限or終了日無期限
		tsn_tki.data[3] = 79;									// 2079/12/31ｾｯﾄ
		tsn_tki.data[4] = 12;
		tsn_tki.data[5] = 31;
	}else{
		memcpy( &tsn_tki.data[3], &CRD_DAT.PAS.std_end[3], 3 );
	}

	tsn_tki.status = (char)CRD_DAT.PAS.sts;
	memcpy( tsn_tki.sttim, CRD_DAT.PAS.trz, 4 );

	hzuk.y = CLK_REC.year;
	hzuk.m = CLK_REC.mont;
	hzuk.d = CLK_REC.date;
	hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

	jikn.t = CLK_REC.hour;
	jikn.m = CLK_REC.minu;
	jikn.s = 0;

	if( c_prm == 1 || c_prm == 3 ){								// 無期限or終了日無期限
		;
	}else{
		if( !datenearend( &tsn_tki.data[3] ) ){					// 期限切れ間近ﾁｪｯｸ
			ret = 1;											// 期限切れ間近
		}
	}

	return( ret );
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| Mifare定期券処理(料金計算用に変換する)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_mifpaschg( void )															   |*/
/*| PARAMETER	: void	;																		   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	vl_mifpaschg( void )
{
	short	ret = 0;
	char	c_prm;
	short	cnt;

	cr_dat_n = RID_APS;											// APS定期とする(1AH)
	memset( &tsn_tki, 0x00, sizeof( struct TSN_TKI ) );

	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(MifCard.pk_no == CPrmSS[S_SYS][cnt]){
		tsn_tki.pkno = cnt-1;									// 駐車場№ｾｯﾄ
		break;
		}
	}
	tsn_tki.kind = (char)(MifCard.pas_knd);						// 種別ｾｯﾄ
	tsn_tki.code = MifCard.pas_id;								// 個人ｺｰﾄﾞｾｯﾄ

	c_prm = (char)prm_get( COM_PRM,S_PAS,						// 無期限設定Get
			(short)(5+10*(MifCard.pas_knd-1)),1,2 );
	if( c_prm == 1 || c_prm == 2 ){								// 無期限or開始日無期限
		tsn_tki.data[0] = 90;									// 1990/1/1ｾｯﾄ
		tsn_tki.data[1] = 1;
		tsn_tki.data[2] = 1;
	}else{
		tsn_tki.data[0] =										// 開始年
				(char)( MifCard.limit_s.Year % 100 );
		tsn_tki.data[1] = MifCard.limit_s.Mon;					// 開始月
		tsn_tki.data[2] = MifCard.limit_s.Day;					// 開始日
	}

	if( c_prm == 1 || c_prm == 3 ){								// 無期限or終了日無期限
		tsn_tki.data[3] = 79;									// 2079/12/31ｾｯﾄ
		tsn_tki.data[4] = 12;
		tsn_tki.data[5] = 31;
	}else{
		tsn_tki.data[3] =										// 終了年
				(char)( MifCard.limit_e.Year % 100 );
		tsn_tki.data[4] = MifCard.limit_e.Mon;					// 終了月
		tsn_tki.data[5] = MifCard.limit_e.Day;					// 終了日
	}
	
	tsn_tki.status = (uchar)(MifCard.io_stat & 0x0F);

	memcpy( tsn_tki.sttim, &MifCard.ext_tm.Mon, 4 );			// 処理月日時分

	hzuk.y = CLK_REC.year;
	hzuk.m = CLK_REC.mont;
	hzuk.d = CLK_REC.date;
	hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

	jikn.t = CLK_REC.hour;
	jikn.m = CLK_REC.minu;
	jikn.s = 0;

	if( c_prm == 1 || c_prm == 3 ){								// 無期限or終了日無期限
		;
	}else{
		if( !datenearend( &tsn_tki.data[3] ) ){					// 期限切れ間近ﾁｪｯｸ
			ret = 1;											// 期限切れ間近
		}
	}

	return( ret );
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| ｻｰﾋﾞｽ券処理(料金計算用に変換する)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_svschk( mag )																   |*/
/*| PARAMETER	: mag	; 磁気ﾘｰﾀﾞｰ受信ﾃﾞｰﾀ														   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

short	vl_svschk( m_gtservic *mag )
{

	long	temp = 0;
	short	cnt;

	cr_dat_n = 0x2d;

	memset( &vl_svs, 0x00, sizeof( struct VL_SVS ) );

	temp = (long)mag->servic.svc_pno[0] + 						// 駐車場No.
						(long)pno_dt[mag->servic.svc_pno[1]>>4];
	if(mag->magformat.type == 1){//GTフォーマット
		temp |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17格納
	}
	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(CPrmSS[S_SYS][cnt] == temp){
			vl_svs.pkno = cnt-1;									// 駐車場No.種別
			break;
		}
	}

	vl_svs.sno  = (char)(mag->servic.svc_pno[1] & 0x0f);				// 種別ｾｯﾄ

	vl_svs.mno  = (short)mag->servic.svc_sno[1] + 						// 店No.
						(((short)mag->servic.svc_sno[0])<<7);

	vl_svs.ymd[0] = mag->servic.svc_sta[0];							// 有効期限(開始)
	vl_svs.ymd[1] = mag->servic.svc_sta[1];
	vl_svs.ymd[2] = mag->servic.svc_sta[2];

	vl_svs.ymd[3] = mag->servic.svc_end[0];							// 有効期限(終了)
	vl_svs.ymd[4] = mag->servic.svc_end[1];
	vl_svs.ymd[5] = mag->servic.svc_end[2];

	vl_svs.sts = mag->servic.svc_sts;									// 割引種別set

	return( 0 );
}

// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) G.So ICクレジット対応
////[]----------------------------------------------------------------------[]
/////	@brief			相殺割引処理(料金計算用にサービス(掛売)券形式変換する)
////[]----------------------------------------------------------------------[]
/////	@param[in]		mno		店No.／サービス券種
/////	@return			ret		0/-1
/////	@author			G.So
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/01/25<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//short	vl_sousai( ushort mno )
//{
//	cr_dat_n = 0x2d;												// サービス(掛売)券
//	vl_now = V_SAK;													// サービス券
//
//	memset( &vl_svs, 0x00, sizeof( struct VL_SVS ) );
//	vl_svs.pkno = KIHON_PKNO;										// 駐車場種別=基本駐車場No.
//	if (mno >= 1 && mno <= 100) {
//	// 掛売券
//		vl_svs.mno  = mno;											// 店No.
//	}
//	else if (mno >= 9001 && mno <= 9015) {
//	// サービス券
//		vl_svs.sno  = (char)(mno - 9000);							// サービス券種
//	}
//	else {
//		vl_svs.sno  = 1;											// 範囲外はサービス券Ａとみなす
//	}
//
//	return( 0 );
//}
//// MH321800(E) G.So ICクレジット対応
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正


/*[]----------------------------------------------------------------------------------------------[]*/
/*| ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ処理(料金計算用に変換する)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_prechg( mag )																   |*/
/*| PARAMETER	: mag	; 磁気ﾘｰﾀﾞｰ受信ﾃﾞｰﾀ														   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

short	vl_prechg( m_gtprepid *mag )
{
	short	wk, i ,cnt;
	long temp;

	cr_dat_n = mag->prepid.pre_idc;

	memset( &tsn_prp, 0x00, sizeof( struct TSN_PRP ) );

	if(mag->magformat.type == 1){
		temp  =  (long)mag->prepid.pre_pno[0] & 0x0000003F;
		temp |= ((long)mag->prepid.pre_pno[1] & 0x0000003F) << 6;
		temp |= ((long)mag->prepid.pre_pno[2] & 0x0000003F) << 12;
	}else{
		temp = astoin( mag->prepid.pre_pno, 2 );
		temp += ( mag->prepid.pre_pno[2]&0x0f ) * 100;
	}
	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(CPrmSS[S_SYS][cnt] == temp){
			tsn_prp.pakno = cnt-1;									//駐車場No種別
			break;
		}
	}

	tsn_prp.hanno = (char)(astoin( mag->prepid.pre_mno, 2 ));			// 販売機№ｾｯﾄ

	wk = (short)mag->prepid.pre_amo;
	if( wk < 0x40 ){											// "0"-"9"?
		wk -= 0x31;
		wk += 1;
	}else if( wk < 0x5b ){										// "A"-"Z"?
		wk -= 0x41;
		wk += 10;
	}else{														// "a"-"z"
		wk -= 0x61;
		wk += 36;
	}
	tsn_prp.hangk = wk * 1000l;									// 販売金額ｾｯﾄ
	tsn_prp.zan = (long)astoinl( mag->prepid.pre_ram, 5 );				// 残額ｾｯﾄ
	tsn_prp.rno= (long)astoinl( mag->prepid.pre_cno, 5 );				// ｶｰﾄﾞ№

	tsn_prp.kid = (char)( mag->prepid.pre_pno[3] - 0x30 );

	for( i = 0; i < 3; i++ )
	{
		tsn_prp.ymd[i] = (char)(astoin( &mag->prepid.pre_sta[i*2],2 ));
	}

	tsn_prp.han = mag->prepid.pre_amo;
	tsn_prp.sei = PRC_GENDOGAKU_MIN;				 					// 限度額ｾｯﾄ
	tsn_prp.hansam = mag->prepid.pre_sum;								// Check Sum

	hzuk.y = CLK_REC.year;
	hzuk.m = CLK_REC.mont;
	hzuk.d = CLK_REC.date;
	hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

	jikn.t = CLK_REC.hour;
	jikn.m = CLK_REC.minu;
	jikn.s = 0;

	return( 0 );
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 回数券処理(料金計算用に変換する)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_kaschg( mag )																   |*/
/*| PARAMETER	: mag	; 磁気ﾘｰﾀﾞｰ受信ﾃﾞｰﾀ														   |*/
/*| RETURN VALUE: 100=期限切れ間近、0=OK（間近でない）											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

short	vl_kaschg( m_gtservic *mag )
{
	short	ret = 0;
	long	temp = 0;
	short	cnt;

	cr_dat_n = 0x2c;

	memset( &vl_kas, 0x00, sizeof( struct VL_KAS ) );

	temp = (long)mag->servic.svc_pno[0] + 						// 駐車場No.
						(long)pno_dt[mag->servic.svc_pno[1]>>4];
	if(mag->magformat.type == 1){//GTフォーマット
		temp |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17格納
	}
	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(CPrmSS[S_SYS][cnt] == temp){
			vl_kas.pkno = cnt-1;									//駐車場No種別
			break;
		}
	}

	vl_kas.rim  = (char)(mag->servic.svc_pno[1] & 0x0f);				// 制限度数ｾｯﾄ

	vl_kas.tnk  = (short)mag->servic.svc_sno[1] + 						// 単位金額
						(((short)mag->servic.svc_sno[0])<<7);
	vl_kas.nno = mag->servic.svc_sts;									// 残回数


	vl_kas.data[0] = mag->servic.svc_sta[0];							// 有効期限(開始)
	vl_kas.data[1] = mag->servic.svc_sta[1];
	vl_kas.data[2] = mag->servic.svc_sta[2];

	vl_kas.data[3] = mag->servic.svc_end[0];							// 有効期限(終了)
	vl_kas.data[4] = mag->servic.svc_end[1];
	vl_kas.data[5] = mag->servic.svc_end[2];

	vl_kas.sttim[0] = mag->servic.svc_tim[0];							// 処理日時
	vl_kas.sttim[1] = mag->servic.svc_tim[1];
	vl_kas.sttim[2] = mag->servic.svc_tim[2];
	vl_kas.sttim[3] = mag->servic.svc_tim[3];

	if( (99 == vl_kas.data[3]) && (99 == vl_kas.data[4]) && (99 == vl_kas.data[5]) ){ 	// 無期限券
		;
	}else{														// 期限あり券
		if( !datenearend( &vl_kas.data[3] ) ){					// 期限切れ間近ﾁｪｯｸ
			ret = 100;											// 間近
		}
	}

	return( ret );
}


#if SYUSEI_PAYMENT
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 修正精算ﾃﾞｰﾀ処理(料金計算用に変換する)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_scarchg( no )																   |*/
/*| PARAMETER	: no	; 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EXﾍﾞｰｽ)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	vl_scarchg( ushort no )
{

	if( ( no >= 1 )&&( no <= LOCK_MAX ) ){						// 駐車位置番号ﾁｪｯｸ(1～15?)

		no -= 1;												// 車室1(no=1)はsei_stp_dat[0]

		memset( &vl_tik, 0x00, sizeof( struct VL_TIK ) );		// 駐車券ﾃﾞｰﾀ(vl_tik)初期化(0ｸﾘｱ)

		cr_dat_n = 0x26;										// 券IDｺｰﾄﾞｾｯﾄ(精算中止券)
		vl_now = V_CHS;											// 読込み券種別ｾｯﾄ(精算中止券)

		vl_tik.syu = LockInfo[no].ryo_syu;						// 種別ｾｯﾄ(駐車位置設定毎)

		card_use[USE_SVC] = (uchar)( syusei[no].sy_wmai +		// 使用枚数(中止,修正用)
						FLAPDT.flp_data[no].bk_wmai );
		vl_tik.wari = syusei[no].sy_wari +						// 割引金額(中止,修正用)
						FLAPDT.flp_data[no].bk_wari;
		vl_tik.time = syusei[no].sy_time +						// 割引時間数(中止,修正用)
						FLAPDT.flp_data[no].bk_time;
		vl_tik.pst = (uchar)FLAPDT.flp_data[no].bk_pst;			// ％割引率(中止,修正用)

		vl_tik.chk = 0;											// 精算前券ｾｯﾄ

		vl_tik.cno = KIHON_PKNO;								// 駐車場№ｾｯﾄ(基本駐車場№)
		vl_tik.ckk = (short)CPrmSS[S_PAY][2];					// 発券機No.
		vl_tik.hno = 0;											// 発券No.
		vl_tik.mno = 0;											// 店№ｾｯﾄ

		vl_tik.tki = car_in_f.mon;								// 入車月ｾｯﾄ
		vl_tik.hii = car_in_f.day;								// 入車日ｾｯﾄ
		vl_tik.jii = car_in_f.hour;								// 入車時ｾｯﾄ
		vl_tik.fun = car_in_f.min;								// 入車分ｾｯﾄ

		syashu = vl_tik.syu;									// 車種

		hzuk.y = car_ot_f.year;
		hzuk.m = car_ot_f.mon;
		hzuk.d = car_ot_f.day;

		hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

		jikn.t = car_ot_f.hour;
		jikn.m = car_ot_f.min;

		jikn.s = 0;

		car_in.year  = car_in_f.year;							// 入車年
		car_in.mon   = vl_tik.tki;								//     月
		car_in.day   = vl_tik.hii;								//     日
		car_in.hour  = vl_tik.jii;								//     時
		car_in.min   = vl_tik.fun;								//     分
		car_in.week  = (char)youbiget( car_in.year ,			//     曜日
								(short)car_in.mon  ,
								(short)car_in.day  );
		car_ot.year = hzuk.y;									// 出庫年
		car_ot.mon  = hzuk.m;									//     月
		car_ot.day  = hzuk.d;									//     日
		car_ot.week = hzuk.w;									//     曜日
		car_ot.hour = jikn.t;									//     時
		car_ot.min  = jikn.m;									//     分
	}
}
#endif		// SYUSEI_PAYMENT

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 修正用定期券処理(料金計算用に変換する)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_passet( no )																   |*/
/*| PARAMETER	: no	; 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	vl_passet( ushort no )
{
	short	yyy, mmm, ddd;
	char	c_prm;

	cr_dat_n = RID_APS;											// ID Code(1AH)
	memset( &tsn_tki, 0x00, sizeof( struct TSN_TKI) );

	tsn_tki.pkno = (short)((syusei[no-1].tei_syu&0xc0)>>6);		// 駐車場№ｾｯﾄ
	tsn_tki.kind = (char)(syusei[no-1].tei_syu&0x3f);					// 種別ｾｯﾄ
	tsn_tki.code = (short)syusei[no-1].tei_id;					// 個人ｺｰﾄﾞｾｯﾄ

	idnrmlzm( syusei[no-1].tei_sd, &yyy, &mmm, &ddd );			// 有効期限開始日
	tsn_tki.data[0] = (char)( yyy % 100 );
	tsn_tki.data[1] = (char)mmm;
	tsn_tki.data[2] = (char)ddd;

	idnrmlzm( syusei[no-1].tei_ed, &yyy, &mmm, &ddd );			// 有効期限終了日
	tsn_tki.data[3] = (char)( yyy % 100 );
	tsn_tki.data[4] = (char)mmm;
	tsn_tki.data[5] = (char)ddd;

	c_prm = (char)prm_get( COM_PRM,S_PAS,						// 無期限設定Get
			(short)(5+10*(tsn_tki.kind-1)),1,2 );
	if( c_prm == 1 || c_prm == 2 ){								// 無期限or開始日無期限
		tsn_tki.data[0] = 90;									// 1990/1/1ｾｯﾄ
		tsn_tki.data[1] = 1;
		tsn_tki.data[2] = 1;
	}
	if( c_prm == 1 || c_prm == 3 ){								// 無期限or終了日無期限
		tsn_tki.data[3] = 79;									// 2079/12/31ｾｯﾄ
		tsn_tki.data[4] = 12;
		tsn_tki.data[5] = 31;
	}

	tsn_tki.status = 0;

	hzuk.y = CLK_REC.year;
	hzuk.m = CLK_REC.mont;
	hzuk.d = CLK_REC.date;
	hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

	jikn.t = CLK_REC.hour;
	jikn.m = CLK_REC.minu;
	jikn.s = 0;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 期限切れ間近ﾁｪｯｸ																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: datenearend()																	   |*/
/*| PARAMETER	: char *edate	; 年月日														   |*/
/*| RETURN VALUE: ret	; 0=間近（3日前）、0以外=間近でない										   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: TF7700より流用																   |*/
/*| Date		: 2005-11-28																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	datenearend( char *edate )
{
	struct CAR_TIM	wk_e3;										// 期限終了3日前
	char	date_e3[3];
	char	ret;
// MH322914 (s) kasiyama 2016/07/12 GT7000に合わせて期限切れ間近案内を可変日数とする(共通改善No.1212)(MH341106)
	short	i;
	uchar	uc;
// MH322914 (e) kasiyama 2016/07/12 GT7000に合わせて期限切れ間近案内を可変日数とする(共通改善No.1212)(MH341106)

	memset(&wk_e3,0,sizeof(wk_e3));
	if( edate[0] >= 80 )
		wk_e3.year = ((int)edate[0] + 1900);					// 期限終了年
	else
		wk_e3.year = ((int)edate[0] + 2000);					// 期限終了年

	wk_e3.mon  = edate[1];										// 期限終了月ｾｯﾄ
	wk_e3.day  = edate[2];										// 期限終了日ｾｯﾄ
	wk_e3.hour = 0;
	wk_e3.min  = 0;

// MH322914 (s) kasiyama 2016/07/12 GT7000に合わせて期限切れ間近案内を可変日数とする(共通改善No.1212)(MH341106)
//	ec62( &wk_e3 );												// 1日戻す
//	ec62( &wk_e3 );												// 2日戻す
//	ec62( &wk_e3 );												// 3日戻す
	ret = OK;
	switch (cr_dat_n) {
	case RID_APS:												// APS定期券
		uc = (uchar)prm_get(COM_PRM, S_TIK, 13, 2, 1);			// 05-0013⑤⑥
		if (uc <= 15) {
			if (uc == 0) {
				uc = 3;											// ﾃﾞﾌｫﾙﾄは3日前
			}
			for (i = 0; i < uc; i++) {
				ec62( &wk_e3 );									// 1日戻す
			}
		} else {
			ret = NG;											// 期限切れ予告なし
		}
		break;
// MH322914 (s) kasiyama 2016/07/12 GT7000に合わせて期限切れ間近案内を可変日数とする(共通改善No.1212)(MH341106)回数券期限切れ予告
	case RID_KAI:												/* 回数券			*/
		uc = (uchar)prm_get(COM_PRM, S_PRP, 24, 2, 1);			/* 13-0024⑤⑥		*/
		if (uc <= 15) {											/*					*/
			if (uc == 0) {										/*					*/
				uc = 3;											/* ﾃﾞﾌｫﾙﾄは3日前	*/
			}													/*					*/
			for (i = 0; i < uc; i++) {							/*					*/
				ec62( &wk_e3 );									/* １日戻す			*/
			}													/*					*/
		} else {												/*					*/
			ret = NG;											/* 期限切れ予告なし	*/
		}														/*					*/
		break;													/*					*/
// MH322914 (e) kasiyama 2016/07/12 GT7000に合わせて期限切れ間近案内を可変日数とする(共通改善No.1212)(MH341106)回数券期限切れ予告
	default:
		ec62( &wk_e3 );											//　1日戻す
		ec62( &wk_e3 );											//　2日戻す
		ec62( &wk_e3 );											//　3日戻す
		break;
	}
// MH322914 (e) kasiyama 2016/07/12 GT7000に合わせて期限切れ間近案内を可変日数とする(共通改善No.1212)(MH341106)

	date_e3[0] = (char)(wk_e3.year % 100);
	date_e3[1] = wk_e3.mon;
	date_e3[2] = wk_e3.day;

// MH810101(S) R.Endo 2021/02/09 #5256 【設計内の評価NG】定期の有効期限切れチェックをしない設定の場合にも、まもなく有効期限切れを表示してしまう。
//	ret = ec081( date_e3 , edate );
	if ( ret == OK ) {
		ret = ec081( date_e3 , edate );
	}
// MH810101(E) R.Endo 2021/02/09 #5256 【設計内の評価NG】定期の有効期限切れチェックをしない設定の場合にも、まもなく有効期限切れを表示してしまう。

	return( ret );
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 詳細中止ｴﾘｱ検索処理																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_cyushiset( no )															   |*/
/*| PARAMETER	: no	; 駐車位置番号(1～9999)													   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: H.Sekiguchi																	   |*/
/*| Date		: 2006-10-15																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void vl_cyushiset( ulong no )
{
	struct CAR_TIM wk_time1,wk_time2;
	uchar	i;

	memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));
	wrcnt_sub = 0;	//印字用カウンターｸﾘｱ
	
	for(i = 0 ; i < 10 ; i++){
		if(FLAPDT_SUB[i].WPlace == no){
			/*入庫時間*/
			wk_time1.year = car_in.year ;
			wk_time1.mon = car_in.mon ;
			wk_time1.day = car_in.day ;
			wk_time1.hour = car_in.hour ;
			wk_time1.min = car_in.min ;
			/*詳細中止入庫ﾃﾞｰﾀ*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;
			
			if(0 == ec64(&wk_time1,&wk_time2)){
					Flap_Sub_Num = i;
					break;
			}
		}
	}
	if(i >= 10){
		Flap_Sub_Flg = 2;									//再精算&&詳細中止ｴﾘｱ無
	}else{
		Flap_Sub_Flg = 1;									//再精算&&詳細中止ｴﾘｱ有
	}
}

// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)
//[]----------------------------------------------------------------------[]
///	@brief		遠隔精算データをセット(料金計算用にセット)
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
void vl_tikchg( void )
{
	cr_dat_n = RID_CKM;

	memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );

	vl_tik.tki = g_PipCtrl.stRemoteTime.InTime.Mon;			// 入車月ｾｯﾄ
	vl_tik.hii = g_PipCtrl.stRemoteTime.InTime.Day;			// 入車日ｾｯﾄ
	vl_tik.jii = g_PipCtrl.stRemoteTime.InTime.Hour;		// 入車時ｾｯﾄ
	vl_tik.fun = g_PipCtrl.stRemoteTime.InTime.Min;			// 入車分ｾｯﾄ
// MH810100(S) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	vl_tik.syu = LockInfo[OPECTL.Pr_LokNo-1].ryo_syu;
// MH810100(E) K.Onodera 2020/02/07 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	vl_tik.syu = g_PipCtrl.stRemoteTime.RyoSyu;				// 料金種別

	syashu = vl_tik.syu;									// 車種

	car_in.mon   = vl_tik.tki;								//     月
	car_in.day   = vl_tik.hii;								//     日
	car_in.hour  = vl_tik.jii;								//     時
	car_in.min   = vl_tik.fun;								//     分
	car_in.year  = CLK_REC.year;
	if( (car_in.mon > CLK_REC.mont) ||						/* 入車月日＞現在月日？ */
		((car_in.mon == CLK_REC.mont) && (car_in.day > CLK_REC.date)) ){
		car_in.year--;
	}

	car_in.week  = (char)youbiget( car_in.year,				//     曜日
							(short)car_in.mon,
							(short)car_in.day );
}
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
//[]----------------------------------------------------------------------[]
///	@brief		カード情報から、料金計算用データセット
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2019/11/15<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void vl_lcd_tikchg( void )
{
// GG129000(S) T.Nagai 2023/01/22 ゲート式車番チケットレスシステム対応（車番データID取得処理修正）
	uchar	i;
// GG129000(E) T.Nagai 2023/01/22 ゲート式車番チケットレスシステム対応（車番データID取得処理修正）

	cr_dat_n = RID_CKM;		// 駐車券（精算前）

	memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );

	// 入庫年月日時分
	             car_in.year = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.shYear;
	vl_tik.tki = car_in.mon  = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.byMonth;
	vl_tik.hii = car_in.day  = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.byDay;
	vl_tik.jii = car_in.hour = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.byHours;
	vl_tik.fun = car_in.min  = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.byMinute;

	// 曜日
	car_in.week  = (char)youbiget( car_in.year, (short)car_in.mon, (short)car_in.day );

	// 料金種別
	vl_tik.syu = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType;
	syashu = vl_tik.syu;

	// 車番情報のセット
	memset( &vl_car_no, 0, sizeof(vl_car_no) );

// GG129000(S) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応/コメント）
//	// 種別(0=車番検索/1=日時検索)
	// 種別(0=車番検索/1=日時検索/2=QR検索)
// GG129000(E) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応/コメント）
	vl_car_no.CarSearchFlg = lcdbm_rsp_in_car_info_main.kind;

	// 車番検索の場合のshaban[4];
// MH810100(S) S.Nishimoto 2020/04/07 静的解析(20200407:60)対応
//	memcpy( vl_car_no.CarSearchData, &lcdbm_rsp_in_car_info_main.data.shabanSearch.shaban, sizeof(vl_car_no.CarSearchData) );
	memcpy( vl_car_no.CarSearchData, &lcdbm_rsp_in_car_info_main.data.shabanSearch.shaban, sizeof(lcdbm_rsp_in_car_info_main.data.shabanSearch.shaban) );
// MH810100(E) S.Nishimoto 2020/04/07 静的解析(20200407:60)対応

	// ｾﾝﾀｰ問合せ時の車の車番
	memcpy( vl_car_no.CarNumber, lcdbm_rsp_in_car_info_main.shaban, sizeof(vl_car_no.CarNumber) );

	// 問合せ媒体番号
// GG129000(S) T.Nagai 2023/01/22 ゲート式車番チケットレスシステム対応（車番データID取得処理修正）
//	memcpy( vl_car_no.CarDataID, lcdbm_rsp_in_car_info_main.crd_info.byAskMediaNo, sizeof(vl_car_no.CarDataID) );
	for (i = 0; i < ONL_MAX_CARDNUM; i++) {
		// マスター情報から車番データID（入出庫ID）を検索する
		if (lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i].CardType == CARD_TYPE_INOUT_ID) {
			memcpy( vl_car_no.CarDataID,
					lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i].byCardNo,
					sizeof(vl_car_no.CarDataID) );
			break;
		}
	}
// GG129000(E) T.Nagai 2023/01/22 ゲート式車番チケットレスシステム対応（車番データID取得処理修正）
}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

