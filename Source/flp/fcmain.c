/*[]----------------------------------------------------------------------[]*/
/*| ﾛｯｸ装置監視処理                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.Hara(NT4500EXをﾍﾞｰｽにIF盤通信ﾀｲﾌﾟとして作り直した)     |*/
/*| Date        : 2005.02.01                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"mem_def.h"
#include	"flp_def.h"
#include	"ope_def.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"
#include	"mdl_def.h"
#include	"IFM.h"
#include	"ntnet_def.h"

static	void	Lk_InCount( uchar uc_prm );
static	void	Lk_OutCount( uchar uc_prm, uchar mode );
static	void	Lk_AllMove( uchar lk_kind, uchar mov_req, uchar req_kind );
static	uchar	Mnt_Lk_Move( uchar mov_req );

static	void	SyuseiFuseiSet( ushort no, uchar type );

char		ope_RegistFuseiData(ushort no, flp_com *flp, char BFrs);
flp_com		flp_com_frs;

char		ope_LockTimeOutData(ushort no, flp_com *flp);

struct	FLPCTL_rec	FLPCTL;											// ﾌﾗｯﾌﾟ(ﾛｯｸ装置)制御ﾜｰｸｴﾘｱ
ushort	lkreccnt;													// 変化ﾃﾞｰﾀ数
ushort	lkrecrpt;													// 変化ﾃﾞｰﾀ読出し数
ushort	lktimoutno;													// ﾀｲﾑｱｳﾄ監視処理するﾛｯｸ装置№

t_flp_DownLockErrInfo	flp_DownLockErrInfo[LOCK_MAX];				// 下降ﾛｯｸﾀｲﾏｰ情報ｴﾘｱ(起動時オール0)
uchar	flp_f_DownLock_RetryDownCtrl;								// 1=リトライ動作による下降指示
																	// FlpSet()への指示中のみ有効
static void	flp_flag_clr( uchar, uchar );

#define	CAR_SENSOR_ON	0x01
#define	LAG_TIME_OVER	0x02
#define	CAR_SENSOR_OFF	0x04
static	uchar	InOutCountFunc( uchar mode,	ushort	evt );

static	uchar	Aida_Flap_Chk( ushort );
static	void	flp_DownLockRetry_RecvSts( ushort );		// 状態変化によるリトライ処理
static	void	flp_DownLock_ForCarSensorOff( ushort no );
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
extern void LongTermParkingRel( ulong LockNo , uchar knd, flp_com *flp);
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)

/*[]----------------------------------------------------------------------[]*/
/*| Flaper Management Main                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : fcmain                                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	fcmain( void )
{
	ushort	evt;
	char	wt_flg = 0;


	if( LKcom_InitFlg == 0 ){
		// lkcomﾀｽｸｲﾆｼｬﾙ未完了
		return;
	}

	if( ( evt = FlpMsg() ) == 0 ){
		return;
	}

	if( ( FLPCTL.Room_no < 1 ) || ( LOCK_MAX < FLPCTL.Room_no ) ) {
		return;
	}
	Lock_Kind = LkKind_Get( FLPCTL.Room_no );					// 装置種別（ﾌﾗｯﾌﾟ／ﾛｯｸ装置）を取得
	if( Lock_Kind == LK_KIND_ERR ){
		return;
	}

	if( _is_ntnet_remote() && (prm_get(COM_PRM,S_NTN,121,1,1) == 0) ){	// 遠隔NT-NET設定 & 既存形式のときのみチェック
		switch( evt / 100 ){
			case 1:		// Car sensor ON
			case 2:		// Car sensor OFF
				if( (prm_get(COM_PRM,S_NTN,120,3,1)!=0) && (NT_pcars_timer == -1) ) {
					NT_pcars_timer = (short)(prm_get(COM_PRM,S_NTN,120,3,1) * 50);
				}
				break;
		}
	}

	// 車室毎ﾌﾗｯﾌﾟﾃﾞｰﾀをﾜｰｸｴﾘｱへﾛｰﾄﾞ
	memcpy( &FLPCTL.flp_work, &FLAPDT.flp_data[ FLPCTL.Room_no - 1 ], sizeof( FLPCTL.flp_work ) );

	switch( FLPCTL.flp_work.mode ){
		case FLAP_CTRL_MODE1:	// 下降済み(車両なし)
			wt_flg = flp_faz1( evt );
			break;
		case FLAP_CTRL_MODE2:	// 下降済み(車両あり､FTｶｳﾝﾄ中)
			wt_flg = flp_faz2( evt );
			break;
		case FLAP_CTRL_MODE3:	// 上昇動作中
			wt_flg = flp_faz3( evt );
			break;
		case FLAP_CTRL_MODE4:	// 上昇済み(駐車中)
			wt_flg = flp_faz4( evt );
			break;
		case FLAP_CTRL_MODE5:	// 下降動作中
			wt_flg = flp_faz5( evt );
			break;
		case FLAP_CTRL_MODE6:	// 下降済み(車両あり､LTｶｳﾝﾄ中)
			wt_flg = flp_faz6( evt );
			break;
		default:
			if( FLPCTL.Room_no > 0 && FLPCTL.Room_no <= INT_CAR_START_INDEX ){
				ex_errlg( ERRMDL_IFFLAP, ERR_FLAP_STATUSNG, 2, 0 );	// ﾌﾗｯﾌﾟ状態規定外
			}else if( FLPCTL.Room_no > INT_CAR_START_INDEX && FLPCTL.Room_no <= BIKE_START_INDEX ){
				ex_errlg( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_STATENG, 2, 0 );	// フラップ／ロック状態規定外
			} else {
				ex_errlg( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_STATENG, 2, 0 );	// フラップ／ロック状態規定外
			}
			break;
	}
	if( wt_flg != 0 ) {
		nmisave( &FLAPDT.flp_data[ FLPCTL.Room_no - 1 ], &FLPCTL.flp_work, sizeof( FLPCTL.flp_work ) );
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
//		if( wt_flg == 1 ){
//			Make_Log_Enter( FLPCTL.Room_no );		// 入庫データ作成(LOGに保存する形式)
//			Log_regist( LOG_ENTER );				// 入庫履歴登録
//		}
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 下降済み(車両なし)のときのｲﾍﾞﾝﾄ処理                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz1( evt, aflp )                                   |*/
/*| PARAMETER    : evt = ｲﾍﾞﾝﾄ№                                           |*/
/*| RETURN VALUE : ret = ﾌﾗｯﾌﾟﾃﾞｰﾀ書き換え 0:なし、-1:あり                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz1( ushort evt )
{
	char	ret;

	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			/* 出庫情報登録 */
			IoLog_write(IOLOG_EVNT_OUT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			break;
	}

	switch( evt % 100 ){
		case 50:	// 状態変化
			if(( FLPCTL.flp_work.nstat.bits.b02 == 1 )&&			// 上昇動作
			   ( FLPCTL.flp_work.nstat.bits.b01 == 1 )){			// 上昇済み(ﾛｯｸ閉済み)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			else if(( FLPCTL.flp_work.nstat.bits.b02 == 0 )&&		// 下降動作
					( FLPCTL.flp_work.nstat.bits.b01 == 0 )){		// 下降済み(ﾛｯｸ開済み)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				if( ( Aida_Flap_Chk((ushort)(FLPCTL.Room_no-1))) &&
					( FLPCTL.flp_work.nstat.bits.b00 == 0 ) &&
					( FLPCTL.flp_work.nstat.bits.b05 == 1 ) ){
					// 英田製ﾌﾗｯﾌﾟで下降ﾛｯｸ中
					
					// （動作説明）
					// 駐車待ち（車なし／下降済）で下降ｾﾝｻがOFFした場合、IF盤が自立的に下降動作を行うが
					// それでも下降ｾﾝｻがONしない場合、下降ﾛｯｸとなる。英田製ﾌﾗｯﾌﾟ時は１回ﾘﾄﾗｲ（ﾒｲﾝから下降要求）を行う。
					FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
				}
			}
			flp_DownLock_ForCarSensorOff( FLPCTL.Room_no );
			break;
		case 21:	// Payment complete
			break;
		case 23:	// ﾌﾗｯﾌﾟ上昇指示(修正精算用)
			if( syusei[FLPCTL.Room_no-1].ot_car == 2 ){			// 状態2：出庫～次の入庫
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 3 ){	// 状態3：次の入庫～ﾌﾗｯﾌﾟ上昇ﾀｲﾑ中
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 4 ){	// 状態4：ﾌﾗｯﾌﾟ上昇～次の精算
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 5 ){	// 状態5：ﾗｸﾞﾀｲﾑｱｯﾌﾟ～再精算
				SyuseiFuseiSet( FLPCTL.Room_no, 1 );
			}
			break;
		case 24:	// ﾌﾗｯﾌﾟ下降指示(修正精算用)
			break;
		case 25:	// Mentenance Flaper Up
			Mnt_Lk_Move( ON );										// ﾒﾝﾃﾅﾝｽ操作「ﾌﾗｯﾌﾟ上昇（ﾛｯｸ閉）」要求
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			Mnt_Lk_Move( OFF );										// ﾒﾝﾃﾅﾝｽ操作「ﾌﾗｯﾌﾟ下降（ﾛｯｸ開）」要求
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ
			break;
		case 42:	// Lag time timer over
			break;
		case 45:	// 下降ﾛｯｸﾀｲﾏｰﾀｲﾑｱｳﾄ
			(void)flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) );
			break;
		default:
			break;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 下降済み(車両あり､FTｶｳﾝﾄ中)のときのｲﾍﾞﾝﾄ処理                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz2( evt )                                         |*/
/*| PARAMETER    : evt = ｲﾍﾞﾝﾄ№                                           |*/
/*| RETURN VALUE : ret = ﾌﾗｯﾌﾟﾃﾞｰﾀ書き換え 0:なし、-1:あり                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz2( ushort evt )
{
	char	ret;
	uchar		snd20_flg;

	snd20_flg = 0;
	ret = 0;

	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt / 100));
			/* 出庫情報登録 */
			IoLog_write(IOLOG_EVNT_OUT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			// TF4800に合わせてフラップ(ロック)上昇タイマー中に出庫した場合、フラップが上がっていれば、下降させる。
			if( FLPCTL.flp_work.nstat.bits.b01 == 1 ){				// フラップ上昇中？
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// 状態変化
			if(( FLPCTL.flp_work.nstat.bits.b02 == 1 )&&			// 上昇動作
			   ( FLPCTL.flp_work.nstat.bits.b01 == 1 )){			// 上昇済み(ﾛｯｸ閉済み)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			else if(( FLPCTL.flp_work.nstat.bits.b02 == 0 )&&		// 下降動作
					( FLPCTL.flp_work.nstat.bits.b01 == 0 )){		// 下降済み(ﾛｯｸ開済み)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				flp_DownLockRetry_RecvSts( FLPCTL.Room_no );		// 状態変化によるリトライ処理
			}
			break;
		case 21:	// Payment complete
			if(Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,1)){	// ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に精算する
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?
					if( FLPCTL.flp_work.nstat.bits.b01 == 0 ){		// 下降済み?
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
					}else{
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE5;
					}
					FlpSet( FLPCTL.Room_no, 2 );					// Flaper DOWN
					FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
				}else{
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
					FlpSet( FLPCTL.Room_no, 2 );					// Flaper DOWN(IF盤とのズレ防止、念のため)
					FLPCTL.flp_work.timer = -1;						// Lag time stop
				}
				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt % 100));
				ret = (char)-1;
			}
			break;
		case 23:	// ﾌﾗｯﾌﾟ上昇指示(修正精算用)
			if( syusei[FLPCTL.Room_no-1].ot_car == 2 ){			// 状態2：出庫～次の入庫
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 3 ){	// 状態3：次の入庫～ﾌﾗｯﾌﾟ上昇ﾀｲﾑ中
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 4 ){	// 状態4：ﾌﾗｯﾌﾟ上昇～次の精算
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 5 ){	// 状態5：ﾗｸﾞﾀｲﾑｱｯﾌﾟ～再精算
				SyuseiFuseiSet( FLPCTL.Room_no, 1 );
			}
			break;
		case 24:	// ﾌﾗｯﾌﾟ下降指示(修正精算用)
			break;
		case 25:	// Mentenance Flaper Up
			Mnt_Lk_Move( ON );										// ﾒﾝﾃﾅﾝｽ操作「ﾌﾗｯﾌﾟ上昇（ﾛｯｸ閉）」要求
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			Mnt_Lk_Move( OFF );										// ﾒﾝﾃﾅﾝｽ操作「ﾌﾗｯﾌﾟ下降（ﾛｯｸ開）」要求
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 42:	// Lag time timer over
			break;
		case 43:	// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){ 				// 車両あり？
				// 車両あり
				if( FLPCTL.flp_work.nstat.bits.b01 == 1 ){			// 上昇済み（ﾛｯｸ閉済み）？
					// 上昇済み（ﾛｯｸ閉済み）
					if( LockInfo[FLPCTL.Room_no-1].lok_syu <=  6 ){			// ﾛｯｸ装置の時
						if( FLPCTL.flp_work.nstat.bits.b02 == 0 ){			// 下降指示（ﾛｯｸ開指示）
							// ﾛｯｸ開指示済みでﾛｯｸ閉状態＝まだﾛｯｸ開の結果を受信していない
							// 従って、ﾛｯｸ閉ﾀｲﾑｱｯﾌでﾛｯｸ閉を送信する
							FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;			// 状態←上昇動作中
							FlpSet( FLPCTL.Room_no, 1 );					// ﾌﾗｯﾌﾟ上昇（ﾛｯｸ閉）要求
						}else{												// 上昇指示（ﾛｯｸ閉指示）
							FLPCTL.flp_work.mode = FLAP_CTRL_MODE4;			// 状態←上昇済み(駐車中)
							snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt % 100));
						}
					}else{													// ﾌﾗｯﾌﾟの時
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE4;				// 状態←上昇済み(駐車中)
						snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt % 100));
					}
				}
				else{
					// 下降済み（ﾛｯｸ開済み）
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;			// 状態←上昇動作中
					FlpSet( FLPCTL.Room_no, 1 );					// ﾌﾗｯﾌﾟ上昇（ﾛｯｸ閉）要求
				}
			}
			else{
				// 車両なし
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;				// 状態←下降済み(車両なし)
			}
			ret = (char)-1;
			break;
		case 44:	// 入庫判定ﾀｲﾏｰ
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){ 				// 車両あり？
				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt % 100));
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;					// 状態←下降済み(車両なし)
				FLPCTL.flp_work.timer = -1;								// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰｽﾄｯﾌﾟ
			}
			ret = (char)-1;
			break;
		case 45:	// 下降ﾛｯｸﾀｲﾏｰﾀｲﾑｱｳﾄ
			(void)flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) );
			break;
		default:
			break;
	}
	if( snd20_flg == 1 ){
		ret = 1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 上昇動作中のときのｲﾍﾞﾝﾄ処理                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz3( evt, aflp )                                   |*/
/*| PARAMETER    : evt = ｲﾍﾞﾝﾄ№                                           |*/
/*| RETURN VALUE : ret = ﾌﾗｯﾌﾟﾃﾞｰﾀ書き換え 0:なし、-1:あり                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz3( ushort evt )
{
	char	ret;
	uchar	snd20_flg;
	uchar	fp_car_ot = 0;

	snd20_flg = 0;

	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			if(( prm_get( COM_PRM,S_TYP,73,1,1 ) == 1 )&&					// 英田ﾊﾞｲｸ不正出庫なし
			   ( LockInfo[FLPCTL.Room_no-1].lok_syu == LK_TYPE_AIDA2 )){	// 車室設定が英田ﾊﾞｲｸ
				// 英田ﾊﾞｲｸ時は不正出庫なしとする
				break;
			}
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
//			if (FLPCTL.flp_work.lag_to_in.BIT.FUKUG){
//				// 振替精算・復元入庫中の出庫は不正出庫前に仮の再入庫を送る
//				Make_Log_Enter_frs(FLPCTL.Room_no, (void*)&FLPCTL.flp_work);	// 仮入庫データ
//				Log_regist( LOG_ENTER );										// 入庫履歴登録
//			}
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
			// 不正出庫ﾃﾞｰﾀ登録
			if( fusei.kensuu >= LOCK_MAX ){
				fusei.kensuu = LOCK_MAX - 1;
			}
			memcpy( &fusei.fus_d[fusei.kensuu].iyear,
			        &FLAPDT.flp_data[FLPCTL.Room_no-1].year, 6 );			// In Time
			memcpy( &fusei.fus_d[fusei.kensuu].oyear, &CLK_REC.year, 6 );	// Out Time
			fusei.fus_d[fusei.kensuu].t_iti = FLPCTL.Room_no;

			FLPCTL.flp_work.mode = FLAP_CTRL_MODE5;
			fusei.fus_d[fusei.kensuu].kyousei = 0;							// 強制ﾌﾗｸﾞOFF
			IoLog_write(IOLOG_EVNT_OUT_ILLEGAL_START, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			FlpSet( FLPCTL.Room_no, 2 );									// Flaper DOWN
			FLPCTL.flp_work.nstat.bits.b06 = 1;								// 不正
			fusei.kensuu ++;												// 不正・強制件数+1
			InOutCountFunc( FLAP_CTRL_MODE3, (ushort)(evt / 100));

			fp_car_ot = 1;									// 車両なしでﾌﾗｯﾌﾟ下降要求あり
			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// 状態変化
			if( FLPCTL.flp_work.nstat.bits.b01 == 1 ){				// 上昇済み(ﾛｯｸ閉済み)
				if( fp_car_ot == 0 ){								// 車両なしでﾌﾗｯﾌﾟ下降要求なし
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE4;
				}
				IFM_LockTable.ucFlapWait = 0;		// 動作完了
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?

					if(	( !prm_get( COM_PRM,S_TYP,62,1,2 ) ) ||			// 修正精算なし
						( FLPCTL.flp_work.lag_to_in.BIT.SYUUS == 0 ) ){	// 修正精算後でない
						snd20_flg = InOutCountFunc( FLAP_CTRL_MODE3, (ushort)(evt % 100));
					}
					FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 0;
				}
				ret = (char)-1;
			}
			break;
		case 21:	// Payment complete
			if(Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,1)){		// ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に精算する
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?
					if( FLPCTL.flp_work.nstat.bits.b01 == 0 ){		// 下降済み?
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
					}else{
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE5;
					}
					FlpSet( FLPCTL.Room_no, 2 );					// Flaper DOWN
					FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
				}else{
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
					FlpSet( FLPCTL.Room_no, 2 );					// Flaper DOWN(IF盤とのズレ防止、念のため)
					FLPCTL.flp_work.timer = -1;						// Lag time stop
				}
				InOutCountFunc( FLAP_CTRL_MODE3, (ushort)(evt % 100));
				ret = (char)-1;
			}
			break;
		case 23:	// ﾌﾗｯﾌﾟ上昇指示(修正精算用)
			if( syusei[FLPCTL.Room_no-1].ot_car == 2 ){			// 状態2：出庫～次の入庫
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 3 ){	// 状態3：次の入庫～ﾌﾗｯﾌﾟ上昇ﾀｲﾑ中
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 4 ){	// 状態4：ﾌﾗｯﾌﾟ上昇～次の精算
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 5 ){	// 状態5：ﾗｸﾞﾀｲﾑｱｯﾌﾟ～再精算
				SyuseiFuseiSet( FLPCTL.Room_no, 1 );
				memcpy( &FLPCTL.flp_work.year, &syusei[FLPCTL.Room_no - 1].iyear, 6 );
				syusei[FLPCTL.Room_no - 1].sei = 2;				// 修正元車室：ｻｰﾋﾞｽﾀｲﾑ考慮
				ret = 1;
				FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 1;
				FLPCTL.flp_work.lag_to_in.BIT.LAGIN = 0;
			}
			break;
		case 24:	// ﾌﾗｯﾌﾟ下降指示(修正精算用)
			break;
		case 25:	// Mentenance Flaper Up
			NTNET_Snd_Data05_Sub();									// NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			NTNET_Snd_Data05_Sub();									// NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			// ｴﾗｰ登録
			FLPCTL.flp_work.mode = FLAP_CTRL_MODE4;
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 42:	// Lag time timer over
			break;
		default:
			break;
	}
	if (FLPCTL.flp_work.mode != FLAP_CTRL_MODE3) {
	// 振替元の復元が完了したら振替情報をクリア
		FLPCTL.flp_work.lag_to_in.BIT.FUKUG = 0;
		FLPCTL.flp_work.lag_to_in.BIT.FURIK = 0;
		FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 0;
	}
	if( snd20_flg == 1 ){
		ret = 1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 上昇済み(駐車中)のときのｲﾍﾞﾝﾄ処理                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz4( evt )                                         |*/
/*| PARAMETER    : evt = ｲﾍﾞﾝﾄ№                                           |*/
/*| RETURN VALUE : ret = ﾌﾗｯﾌﾟﾃﾞｰﾀ書き換え 0:なし、-1:あり                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz4( ushort evt )
{
	char	ret;
	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			break;
		case 2:		// Car sensor OFF
			if(!((( FLPCTL.flp_work.nstat.bits.b01 == 0 )&&			// 下降済み(ﾛｯｸ開済み)
			      ( FLPCTL.flp_work.nstat.bits.b07 == 1 ))||		// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞON
			     ( FLPCTL.flp_work.nstat.bits.b03 == 1 )))			// 強制出庫
			{
				// 不正出庫
				if(( prm_get( COM_PRM,S_TYP,73,1,1 ) == 1 )&&					// 英田ﾊﾞｲｸ不正出庫なし
				   ( LockInfo[FLPCTL.Room_no-1].lok_syu == LK_TYPE_AIDA2 )){	// 車室設定が英田ﾊﾞｲｸ
					// 英田ﾊﾞｲｸ時は不正出庫なしとする
					break;
				}
			}
			ope_RegistFuseiData(FLPCTL.Room_no, &FLPCTL.flp_work, 0);
			InOutCountFunc( FLAP_CTRL_MODE4, (ushort)(evt / 100));
			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// 状態変化
			if(( FLPCTL.flp_work.nstat.bits.b02 == 1 )&&			// 上昇動作
			   ( FLPCTL.flp_work.nstat.bits.b01 == 1 )){			// 上昇済み(ﾛｯｸ閉済み)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			else if(( FLPCTL.flp_work.nstat.bits.b02 == 0 )&&		// 下降動作
					( FLPCTL.flp_work.nstat.bits.b01 == 0 )){		// 下降済み(ﾛｯｸ開済み)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			break;
		case 21:	// Payment complete
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				if( FLPCTL.flp_work.nstat.bits.b01 == 0 ){			// 下降済み?
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
				}else{
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE5;
				}
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
				FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN(IF盤とのズレ防止、念のため)
				FLPCTL.flp_work.timer = -1;							// Lag time stop
			}
			InOutCountFunc( FLAP_CTRL_MODE4, (ushort)(evt % 100));
			ret = (char)-1;
			break;
		case 23:	// ﾌﾗｯﾌﾟ上昇指示(修正精算用)
			if( syusei[FLPCTL.Room_no-1].ot_car == 2 ){			// 状態2：出庫～次の入庫
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 3 ){	// 状態3：次の入庫～ﾌﾗｯﾌﾟ上昇ﾀｲﾑ中
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 4 ){	// 状態4：ﾌﾗｯﾌﾟ上昇～次の精算
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 5 ){	// 状態5：ﾗｸﾞﾀｲﾑｱｯﾌﾟ～再精算
				SyuseiFuseiSet( FLPCTL.Room_no, 1 );
				memcpy( &FLPCTL.flp_work.year, &syusei[FLPCTL.Room_no - 1].iyear, 6 );
				syusei[FLPCTL.Room_no - 1].sei = 2;				// 修正元車室：ｻｰﾋﾞｽﾀｲﾑ考慮
				ret = 1;
				FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 1;
				FLPCTL.flp_work.lag_to_in.BIT.LAGIN = 0;
			}
			break;
		case 24:	// ﾌﾗｯﾌﾟ下降指示(修正精算用)
			break;
		case 25:	// Mentenance Flaper Up
			Mnt_Lk_Move( ON );										// ﾒﾝﾃﾅﾝｽ操作「ﾌﾗｯﾌﾟ上昇（ﾛｯｸ閉）」要求
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			/* 強制出庫開始情報登録 */
			IoLog_write(IOLOG_EVNT_FORCE_STA, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			Mnt_Lk_Move( OFF );										// ﾒﾝﾃﾅﾝｽ操作「ﾌﾗｯﾌﾟ下降（ﾛｯｸ開）」要求
			if( FLPCTL.Room_no >= LOCK_START_NO ){
				ope_anm( AVM_FORCE_ROCK );							// 精算完了時ｱﾅｳﾝｽ(ロック)
			}else{
				ope_anm( AVM_FORCE_FLAP );							// 精算完了時ｱﾅｳﾝｽ(フラップ)
			}
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ
			break;
		case 42:	// Lag time timer over
			break;
			// 駐車中、メンテによる下降でもリトライする
		case 45:	// 下降ﾛｯｸﾀｲﾏｰﾀｲﾑｱｳﾄ
			(void)flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) );
			break;
		default:
			break;
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 下降動作中のときのｲﾍﾞﾝﾄ処理                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz5( evt )                                         |*/
/*| PARAMETER    : evt = ｲﾍﾞﾝﾄ№                                           |*/
/*| RETURN VALUE : ret = ﾌﾗｯﾌﾟﾃﾞｰﾀ書き換え 0:なし、-1:あり                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz5( ushort evt )
{
	char	ret;
	struct	CAR_TIM		wok_tm;
	uchar	snd20_flg;
	/* 修正方法はflp_faz3()のfp_car_otによる修正と同様とした */
	uchar	fp_car_in = 0;

	snd20_flg = 0;

	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			fp_car_in = 1;											// 車両ありの処理済み
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			Flp_LagExtCnt[FLPCTL.Room_no-1] = 0;					// ラグタイム延長回数クリア
			flp_flag_clr( FLAP_CTRL_MODE1, CAR_SENSOR_OFF );
			IoLog_write(IOLOG_EVNT_OUT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
// MH322914(S) K.Onodera 2016/12/07 下降ロック状態で出庫した場合、設定にてフラップ下降をリトライする[共通バグNo.1336](GW872707)
//			if(	(Aida_Flap_Chk((ushort)(FLPCTL.Room_no-1))) &&
//				( FLPCTL.flp_work.nstat.bits.b05 == 1 ) ){
//				// 英田製ﾌﾗｯﾌﾟで下降ﾛｯｸ中の場合
//				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
//			}
			if(	( prm_get( COM_PRM,S_TYP,119,1,1 ) == 1 ) &&		// フラップ下降をリトライする
				( FLPCTL.flp_work.nstat.bits.b05 == 1 )		){		// 下降ロック発生
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
			if( prm_get( COM_PRM,S_TYP,119,1,1 ) == 2 ){ 			// フラップ下降を無条件でリトライする
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
// MH322914(E) K.Onodera 2016/12/07 下降ロック状態で出庫した場合、設定にてフラップ下降をリトライする[共通バグNo.1336](GW872707)
			InOutCountFunc( FLAP_CTRL_MODE5, (ushort)(evt / 100));
			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// 状態変化
			if( FLPCTL.flp_work.nstat.bits.b01 == 0 ){				// 下降済み(ﾛｯｸ開済み)
				IFM_LockTable.ucFlapWait = 0;		// 動作完了
				if( fp_car_in == 0 ){								// 車両ありの処理済みでない
					if( FLPCTL.flp_work.nstat.bits.b06 == 1 ){		// 不正
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
						FLPCTL.flp_work.nstat.bits.b06 = 0;			// 不正ｸﾘｱ
					}else{
						if( FLPCTL.flp_work.nstat.bits.b00 ){
							FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
						}
					}
				}
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				ret = (char)-1;
				flp_DownLockRetry_RecvSts( FLPCTL.Room_no );		// 状態変化によるリトライ処理
			}
			flp_DownLock_ForCarSensorOff( FLPCTL.Room_no );
			break;
		case 21:	// Payment complete
// 不具合修正(S) K.Onodera 2016/10/13 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
			// 上昇済み？
			if( FLPCTL.flp_work.nstat.bits.b01 == 1 ){
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
// 不具合修正(E) K.Onodera 2016/10/13 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
			break;
		case 23:	// ﾌﾗｯﾌﾟ上昇指示(修正精算用)
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
				FlpSet( FLPCTL.Room_no, 1 );						// Flaper UP

				// 01-0039④=0設定の時は（間違え）精算時に出庫ｶｳﾝﾄしているため、復元時に入庫ｶｳﾝﾄを行う

				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE5, (ushort)(evt % 100));
				memcpy( &FLPCTL.flp_work.year, &syusei[FLPCTL.Room_no-1].iyear, 6 );
				memset( &FLPCTL.flp_work.s_year, 0, 6 );
				syusei[FLPCTL.Room_no - 1].sei = 2;					// 修正元車室：ｻｰﾋﾞｽﾀｲﾑ考慮

				FLPCTL.flp_work.timer = -1;							// ﾗｸﾞﾀｲﾑ ﾀｲﾏｰｽﾄｯﾌﾟ
				FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 1;
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 24:	// ﾌﾗｯﾌﾟ下降指示(修正精算用)
#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
			if( FLPCTL.flp_work.nstat.bits.b00 == 0 ){				// Car OFF?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
				ret = -1;
			}
#endif	// SYUSEI_PAYMENT
			break;
		case 25:	// Mentenance Flaper Up
			NTNET_Snd_Data05_Sub();									// NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			NTNET_Snd_Data05_Sub();									// NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			// ｴﾗｰ登録
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ
			break;
		case 42:	// Lag time timer over
			Flp_LagExtCnt[FLPCTL.Room_no-1] = 0;					// ラグタイム延長回数クリア
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE5, (ushort)(evt % 100));
				IoLog_write(IOLOG_EVNT_RENT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);		/* 再入庫情報登録 */

				if (FLPCTL.flp_work.lag_to_in.BIT.FUKUG) {
					// 振替精算
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
					FlpSet( FLPCTL.Room_no, 1 );						// Flaper UP
					FLPCTL.flp_work.nstat.bits.b06 = 0;					// 不正ｸﾘｱ
					FLPCTL.flp_work.nstat.bits.b07 = 0;					// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞOFF
					FLPCTL.flp_work.nstat.bits.b09 = 0;					// NT-NETﾌﾗｸﾞOFF

					FLPCTL.flp_work.nstat.bits.b14 = 0;					// 遠隔からの操作指示種別クリア
					FLPCTL.flp_work.nstat.bits.b15 = 0;					// 遠隔からのフラップ操作要求クリア
					ret = (char)-1;
					break;	// 入庫時刻、uketuke、passwd、bk_syuはそのまま
				}
				
				switch( Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,3) ){	// ﾗｸﾞﾀｲﾑ経過後の入庫時間？
				
					case	0:		// 「現在時間」を入庫時間とする
						memcpy( &FLPCTL.flp_work.year, &CLK_REC, 6 );
						break;

					case	1:		// 「現在時間-1分」を入庫時間とする
						wok_tm.year = CLK_REC.year;					// 入車	年
						wok_tm.mon  = CLK_REC.mont;					//		月
						wok_tm.day  = CLK_REC.date;					//		日
						wok_tm.hour = CLK_REC.hour;					//		時
						wok_tm.min  = CLK_REC.minu;					//		分
						wok_tm.week = CLK_REC.week;					//		曜日
						ec66( &wok_tm,(long)1 );					// 日付更新処理（現在時刻を-1分減算）
						FLPCTL.flp_work.year = wok_tm.year;			// 入庫	年
						FLPCTL.flp_work.mont = wok_tm.mon;			//		月
						FLPCTL.flp_work.date = wok_tm.day;			//		日
						FLPCTL.flp_work.hour = wok_tm.hour;			//		時
						FLPCTL.flp_work.minu = wok_tm.min;			//		分
						break;

					case	2:		// 「精算時間」を入庫時間とする
						memcpy( &FLPCTL.flp_work.year, &FLPCTL.flp_work.s_year, 6 );
						break;

					default:		// その他（現在時間を入庫時間とする）
						memcpy( &FLPCTL.flp_work.year, &CLK_REC, 6 );
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				}
				flp_flag_clr( FLAP_CTRL_MODE3, LAG_TIME_OVER );
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 45:	// 下降ﾛｯｸﾀｲﾏｰﾀｲﾑｱｳﾄ
			(void)flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) );
			break;
		default:
			break;
	}
	if (FLPCTL.flp_work.mode == FLAP_CTRL_MODE1) {
	// 空車に変化
		if (FLPCTL.flp_work.lag_to_in.BIT.FURIK) {
			// 振替中なら振替元車室出庫情報を通知
			queset(OPETCBNO, CAR_FURIKAE_OUT, 0, 0);			// 振替元の車が出庫
			FLPCTL.flp_work.lag_to_in.BIT.FUKUG = 0;
			FLPCTL.flp_work.lag_to_in.BIT.FURIK = 0;
			FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 0;			// <- とりあえずクリアしておく
		}
	}
	if( snd20_flg == 1 ){
		ret = 1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 下降済み(車両あり､LTｶｳﾝﾄ中)のときのｲﾍﾞﾝﾄ処理                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz6( evt )                                         |*/
/*| PARAMETER    : evt = ｲﾍﾞﾝﾄ№                                           |*/
/*| RETURN VALUE : ret = ﾌﾗｯﾌﾟﾃﾞｰﾀ書き換え 0:なし、-1:あり                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz6( ushort evt )
{
	char	ret;
	struct	CAR_TIM		wok_tm;
	uchar	snd20_flg;

	snd20_flg = 0;

	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			Flp_LagExtCnt[FLPCTL.Room_no-1] = 0;					// ラグタイム延長回数クリア
			flp_flag_clr( FLAP_CTRL_MODE1, CAR_SENSOR_OFF );
			IoLog_write(IOLOG_EVNT_OUT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
// MH322914(S) K.Onodera 2016/12/07 下降ロック状態で出庫した場合、設定にてフラップ下降をリトライする[共通バグNo.1336](GW872707)
//			if(	(Aida_Flap_Chk((ushort)(FLPCTL.Room_no-1))) &&
//				( FLPCTL.flp_work.nstat.bits.b05 == 1 ) ){
//				// 英田製ﾌﾗｯﾌﾟで下降ﾛｯｸ中の場合
//				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
//			}
			if(	( prm_get( COM_PRM,S_TYP,119,1,1 ) == 1 ) &&		// フラップ下降をリトライする
				( FLPCTL.flp_work.nstat.bits.b05 == 1 )		){		// 下降ロック発生
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
			if( prm_get( COM_PRM,S_TYP,119,1,1 ) == 2 ){			// フラップ下降を無条件でリトライする
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
// MH322914(E) K.Onodera 2016/12/07 下降ロック状態で出庫した場合、設定にてフラップ下降をリトライする[共通バグNo.1336](GW872707)
			InOutCountFunc( FLAP_CTRL_MODE6, (ushort)(evt / 100));

			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// 状態変化
			if(( FLPCTL.flp_work.nstat.bits.b02 == 1 )&&			// 上昇動作
			   ( FLPCTL.flp_work.nstat.bits.b01 == 1 )){			// 上昇済み(ﾛｯｸ閉済み)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			else if(( FLPCTL.flp_work.nstat.bits.b02 == 0 )&&		// 下降動作
					( FLPCTL.flp_work.nstat.bits.b01 == 0 )){		// 下降済み(ﾛｯｸ開済み)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				flp_DownLockRetry_RecvSts( FLPCTL.Room_no );		// 状態変化によるリトライ処理
			}
			break;
		case 21:	// Payment complete
			break;
		case 23:	// ﾌﾗｯﾌﾟ上昇指示(修正精算用)
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
				FlpSet( FLPCTL.Room_no, 1 );						// Flaper UP

				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE6, (ushort)(evt % 100));
				memcpy( &FLPCTL.flp_work.year, &syusei[FLPCTL.Room_no-1].iyear, 6 );
				memset( &FLPCTL.flp_work.s_year, 0, 6 );
				syusei[FLPCTL.Room_no - 1].sei = 2;					// 修正元車室：ｻｰﾋﾞｽﾀｲﾑ考慮

				FLPCTL.flp_work.timer = -1;							// ﾗｸﾞﾀｲﾑ ﾀｲﾏｰｽﾄｯﾌﾟ
				FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 1;
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 24:	// ﾌﾗｯﾌﾟ下降指示(修正精算用)
#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
			if( FLPCTL.flp_work.nstat.bits.b00 == 0 ){				// Car OFF?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
				ret = -1;
			}
#endif	// SYUSEI_PAYMENT
			break;
		case 25:	// Mentenance Flaper Up
			Mnt_Lk_Move( ON );										// ﾒﾝﾃﾅﾝｽ操作「ﾌﾗｯﾌﾟ上昇（ﾛｯｸ閉）」要求
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			if( Mnt_Lk_Move( OFF ) == OK ){							// ﾒﾝﾃﾅﾝｽ操作「ﾌﾗｯﾌﾟ下降（ﾛｯｸ開）」要求
				// 下降ﾛｯｸ時にﾒﾝﾃﾅﾝｽで下降させた場合、ﾗｸﾞﾀｲﾏを再起動する
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?
					FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
				}
			}
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ
			break;
		case 42:	// Lag time timer over
			Flp_LagExtCnt[FLPCTL.Room_no-1] = 0;					// ラグタイム延長回数クリア
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE6, (ushort)(evt % 100));
				IoLog_write(IOLOG_EVNT_RENT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);		/* 再入庫情報登録 */
				if (FLPCTL.flp_work.lag_to_in.BIT.FUKUG) {
					// 振替精算
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
					FlpSet( FLPCTL.Room_no, 1 );						// Flaper UP
					FLPCTL.flp_work.nstat.bits.b06 = 0;					// 不正ｸﾘｱ
					FLPCTL.flp_work.nstat.bits.b07 = 0;					// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞOFF
					FLPCTL.flp_work.nstat.bits.b09 = 0;					// NT-NETﾌﾗｸﾞOFF
					FLPCTL.flp_work.nstat.bits.b14 = 0;					// 遠隔からの操作指示種別クリア
					FLPCTL.flp_work.nstat.bits.b15 = 0;					// 遠隔からのフラップ操作要求クリア
					ret = (char)-1;
					break;	// 入庫時刻、uketuke、passwd、bk_syuはそのまま
				}

				switch( Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,3) ){	// ﾗｸﾞﾀｲﾑ経過後の入庫時間？
				
					case	0:		// 「現在時間」を入庫時間とする
						memcpy( &FLPCTL.flp_work.year, &CLK_REC, 6 );
						break;

					case	1:		// 「現在時間-1分」を入庫時間とする
						wok_tm.year = CLK_REC.year;					// 入車	年
						wok_tm.mon  = CLK_REC.mont;					//		月
						wok_tm.day  = CLK_REC.date;					//		日
						wok_tm.hour = CLK_REC.hour;					//		時
						wok_tm.min  = CLK_REC.minu;					//		分
						wok_tm.week = CLK_REC.week;					//		曜日
						ec66( &wok_tm,(long)1 );					// 日付更新処理（現在時刻を-1分減算）
						FLPCTL.flp_work.year = wok_tm.year;			// 入庫	年
						FLPCTL.flp_work.mont = wok_tm.mon;			//		月
						FLPCTL.flp_work.date = wok_tm.day;			//		日
						FLPCTL.flp_work.hour = wok_tm.hour;			//		時
						FLPCTL.flp_work.minu = wok_tm.min;			//		分
						break;

					case	2:		// 「精算時間」を入庫時間とする
						memcpy( &FLPCTL.flp_work.year, &FLPCTL.flp_work.s_year, 6 );
						break;

					default:		// その他（現在時間を入庫時間とする）
						memcpy( &FLPCTL.flp_work.year, &CLK_REC, 6 );
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				}
				flp_flag_clr( FLAP_CTRL_MODE3, LAG_TIME_OVER );
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 45:	// 下降ﾛｯｸﾀｲﾏｰﾀｲﾑｱｳﾄ
			if( 1 == flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) ) ){
				// 下降ﾛｯｸ時に下降させた場合、ﾗｸﾞﾀｲﾏを再起動する
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?
					FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
				}
			}
			break;
		default:
			break;
	}
	if (FLPCTL.flp_work.mode == FLAP_CTRL_MODE1) {
	// 空車に変化
		if (FLPCTL.flp_work.lag_to_in.BIT.FURIK) {
			// 振替中なら振替元車室出庫情報を通知
			queset(OPETCBNO, CAR_FURIKAE_OUT, 0, 0);			// 振替元の車が出庫
			FLPCTL.flp_work.lag_to_in.BIT.FUKUG = 0;
			FLPCTL.flp_work.lag_to_in.BIT.FURIK = 0;
			FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 0;
		}
	}
	if( snd20_flg == 1 ){
		ret = 1;
	}
	return( ret );
}


/*[]----------------------------------------------------------------------[]*/
/*| ﾒｯｾｰｼﾞ取込処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FlpMsg( void )                                          |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ｲﾍﾞﾝﾄ№                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005.02.01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	FlpMsg( void )
{
	MsgBuf		*msb;

	union {
		unsigned short comd;
		unsigned char cc[2];
	} sc;

	ushort	ret = 0;
	ushort	dat;

	LkTimChk();

	if( lkreccnt == 0 ){
		if( (msb = GetMsg( FLPTCBNO )) == NULL ){
			return( 0 );
		}
		ret = 0;
		sc.comd = msb->msg.command;
		FLPCTL.Comd_knd = sc.cc[0];									// Command kind
		FLPCTL.Comd_cod = sc.cc[1];									// Command code
		FLPCTL.Ment_flg = 0;										// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞｸﾘｱ

		switch( sc.comd ){
			case LK_DATAREC:										// ﾃﾞｰﾀ受信
				if( lkreccnt == 0 ){
					if( LKcom_RcvDataGet( &lkreccnt ) == 2 ){
						lkrecrpt = 0;
						ret = 99;									// 車両及びﾛｯｸ状態変化有り
					}else{
						lkreccnt = 0;								// 変化ﾃﾞｰﾀ数ｸﾘｱ
						lkrecrpt = 0;
						ret = 0;
					}
				}else{
					ret = 0;
				}
				break;
			case CAR_PAYCOM_SND:									// 精算完了
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				syusei[FLPCTL.Room_no - 1].ot_car = 1;
				ret = 21;
				break;
			case CAR_SVSLUG_SND:									// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ,ﾗｸﾞﾀｲﾑﾀｲﾑｱｯﾌﾟ
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				dat = FLAPDT.flp_data[ FLPCTL.Room_no - 1 ].mode;
				if(( dat == FLAP_CTRL_MODE5 )||( dat == FLAP_CTRL_MODE6 )){
					ret = 42;										// ﾗｸﾞﾀｲﾑﾀｲﾑｱｳﾄ
					syusei[FLPCTL.Room_no - 1].ot_car = 5;
				}else{
					ret = 43;										// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰﾀｲﾑｱｳﾄ
					syusei[FLPCTL.Room_no - 1].ot_car = 4;
				}
				break;
			case CAR_INCHK_SND:										// 入庫判定ﾀｲﾑｱｯﾌﾟ
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				ret = 44;											// 入庫判定ﾀｲﾑｱｳﾄ
				break;
			case CAR_FLPMOV_SND:									// ﾌﾗｯﾌﾟ板動作監視ﾀｲﾏｰﾀｲﾑｱｯﾌﾟ
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				ret = 33;
				break;
			case FLAP_UP_SND:										// ﾌﾗｯﾌﾟ上昇指示(ﾒﾝﾃﾅﾝｽ)
			case FLAP_UP_SND_NTNET:									// ﾌﾗｯﾌﾟ上昇指示(NTNET)
			case FLAP_UP_SND_MAF:									// ﾌﾗｯﾌﾟ上昇指示(MAF)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				if( sc.comd == FLAP_UP_SND ){
					FLPCTL.Ment_flg = 0;							// ﾒﾝﾃﾅﾝｽで上昇指示
				}else if( sc.comd == FLAP_UP_SND_MAF ){				// ﾌﾗｯﾌﾟ上昇指示(MAF)
					FLPCTL.Ment_flg = 2;							// MAFで上昇指示
				}else{
					FLPCTL.Ment_flg = 1;							// NTNETで上昇指示
				}
				ret = 25;
				break;
			case FLAP_DOWN_SND:										// ﾌﾗｯﾌﾟ下降指示(ﾒﾝﾃﾅﾝｽ)
			case FLAP_DOWN_SND_NTNET:								// ﾌﾗｯﾌﾟ下降指示(NTNET)
			case FLAP_DOWN_SND_MAF:									// ﾌﾗｯﾌﾟ上昇指示(MAF)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				if( sc.comd == FLAP_DOWN_SND ){
					FLPCTL.Ment_flg = 0;							// ﾒﾝﾃﾅﾝｽで下降指示
				}else if( sc.comd == FLAP_DOWN_SND_MAF ){			// ﾌﾗｯﾌﾟ下降指示(MAF)
					FLPCTL.Ment_flg = 2;							// MAFで下降指示
				}else{
					FLPCTL.Ment_flg = 1;							// NTNETで下降指示
				}
				ret = 26;
				break;
			case FLAP_UP_SND_SS:									// ﾌﾗｯﾌﾟ上昇指示(修正精算用)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				ret = 23;
				break;
			case FLAP_DOWN_SND_SS:									// ﾌﾗｯﾌﾟ下降指示(修正精算用)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				ret = 24;
				break;
			case LK_SND_P_CHK:
				memset( MntLockTest, 0xff, sizeof( MntLockTest ));
			case LK_SND_CLS:										// IF盤へ制御ﾃﾞｰﾀ送信要求(閉)
			case LK_SND_OPN:										// IF盤へ制御ﾃﾞｰﾀ送信要求(開)
			case LK_SND_STS:										// IF盤へ制御ﾃﾞｰﾀ送信要求(状態要求)
			case LK_SND_TEST:										// IF盤へ制御ﾃﾞｰﾀ送信要求(開閉ﾃｽﾄ)
			case LK_SND_CNT:										// IF盤へ制御ﾃﾞｰﾀ送信要求(動作ｶｳﾝﾄｸﾘｱ)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				if( ( FLPCTL.Room_no >= 1 )&&( LOCK_MAX >= FLPCTL.Room_no ) ){
					LKopeApiLKCtrl( FLPCTL.Room_no, (uchar)( FLPCTL.Comd_cod & 0x0f ) );	// 制御ﾃﾞｰﾀ送信要求
				}
				break;
			case LK_SND_A_STS:										// IF盤へ制御ﾃﾞｰﾀ送信要求(全状態要求)
			case LK_SND_MNT:										// IF盤へ制御ﾃﾞｰﾀ送信要求(ﾒﾝﾃ情報要求)
			case LK_SND_A_TEST:										// IF盤へ制御ﾃﾞｰﾀ送信要求(全開閉ﾃｽﾄ)
			case LK_SND_A_CNT:										// IF盤へ制御ﾃﾞｰﾀ送信要求(全動作ｶｳﾝﾄｸﾘｱ)
			case LK_SND_VER:										// IF盤へ制御ﾃﾞｰﾀ送信要求(ﾊﾞｰｼﾞｮﾝ要求)
			case LK_SND_ERR:										// IF盤へ制御ﾃﾞｰﾀ送信要求(ｴﾗｰ状態要求)
			case LK_SND_A_LOOP_DATA:								// ループデータ要求
				LKopeApiLKCtrl_All( msb->msg.data[0], (uchar)( FLPCTL.Comd_cod & 0x0f ) );	// 制御ﾃﾞｰﾀ送信要求
				break;
			case FLAP_A_UP_SND_MNT:									// 全ﾌﾗｯﾌﾟ上昇要求(ﾒﾝﾃﾅﾝｽ)
				Lk_AllMove( LK_KIND_FLAP, 1, 0 );
				break;
			case FLAP_A_DOWN_SND_MNT:								// 全ﾌﾗｯﾌﾟ下降要求(ﾒﾝﾃﾅﾝｽ)
				Lk_AllMove( LK_KIND_FLAP, 0, 0 );
				break;
			case LOCK_A_CLS_SND_MNT:								// 全ﾛｯｸ装置閉要求(ﾒﾝﾃﾅﾝｽ)
				Lk_AllMove( LK_KIND_LOCK, 1, 0 );
				break;
			case LOCK_A_OPN_SND_MNT:								// 全ﾛｯｸ装置開要求(ﾒﾝﾃﾅﾝｽ)
				Lk_AllMove( LK_KIND_LOCK, 0, 0 );
				break;
			case INT_FLAP_A_UP_SND_MNT:								// 全内蔵ﾌﾗｯﾌﾟ上昇要求(ﾒﾝﾃﾅﾝｽ)
				Lk_AllMove( LK_KIND_INT_FLAP, 1, 0 );
				break;
			case INT_FLAP_A_DOWN_SND_MNT:							// 全内蔵ﾌﾗｯﾌﾟ下降要求(ﾒﾝﾃﾅﾝｽ)
				Lk_AllMove( LK_KIND_INT_FLAP, 0, 0 );
				break;
			case FLAP_A_UP_SND_NTNET:								// 全ﾌﾗｯﾌﾟ上昇要求(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// 種別セット
				Lk_AllMove( LK_KIND_FLAP, 1, (dat==1?2:1) );
				break;
			case FLAP_A_DOWN_SND_NTNET:								// 全ﾌﾗｯﾌﾟ下降要求(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// 種別セット
				Lk_AllMove( LK_KIND_FLAP, 0, (dat==1?2:1) );
				break;
			case LOCK_A_CLS_SND_NTNET:								// 全ﾛｯｸ装置閉要求(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// 種別セット
				Lk_AllMove( LK_KIND_LOCK, 1, (dat==1?2:1) );
				break;
			case LOCK_A_OPN_SND_NTNET:								// 全ﾛｯｸ装置開要求(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// 種別セット
				Lk_AllMove( LK_KIND_LOCK, 0, (dat==1?2:1) );
				break;
			case INT_FLAP_A_UP_SND_NTNET:							// 全内蔵ﾌﾗｯﾌﾟ上昇要求(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// 種別セット
				Lk_AllMove( LK_KIND_INT_FLAP, 1, (dat==1?2:1) );
				break;
			case INT_FLAP_A_DOWN_SND_NTNET:							// 全内蔵ﾌﾗｯﾌﾟ下降要求(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// 種別セット
				Lk_AllMove( LK_KIND_INT_FLAP, 0, (dat==1?2:1) );
				break;
			case DOWNLOCKOVER_SND:									// 下降ﾛｯｸﾀｲﾑｱｯﾌﾟ
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				ret = 45;											// 
				break;
			case LK_SND_FORCE_LOOP_ON:								// 強制ループON
			case LK_SND_FORCE_LOOP_OFF:								// 強制ループOFF
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// 車室№ｾｯﾄ
				if( ( FLPCTL.Room_no >= 1 )&&( LOCK_MAX >= FLPCTL.Room_no ) ){
					LKopeApiLKCtrl( FLPCTL.Room_no, (uchar)( FLPCTL.Comd_cod & 0xff ) );	// 制御ﾃﾞｰﾀ送信要求
				}
				break;
			default:
				ex_errlg( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_EVENTOUTOFRULE, 2, 0 );	// 規定外ｲﾍﾞﾝﾄ発生
				break;
		}
		FreeBuf( msb );
	}
	if(( lkreccnt )||( ret == 99 )){
		/* 受信したﾃﾞｰﾀは複数車室のﾃﾞｰﾀを含んでいるため、順次処理する	*/
		/* 処理は1車室毎にｱｲﾄﾞﾙへ戻り、その他のﾀｽｸへの負担を減らす		*/
		for( ; lkrecrpt < lkreccnt; ){
			ret = 0;
			if( LKcom_RcvDataAnalys( lkrecrpt ) != 2 ){
				lkrecrpt++;
				continue;
			}
			if( LockBuf[lkrecrpt].LockNoBuf == 0xffff ){			// これ以降ﾃﾞｰﾀなし
				lkreccnt = 0;										// 変化ﾃﾞｰﾀ数ｸﾘｱ
				lkrecrpt = 0;
				break;
			}
			if( LockBuf[lkrecrpt].LockNoBuf == 0xfffe ){			// 状態変化なし
				lkrecrpt++;
				continue;
			}
			FLPCTL.Room_no = LockBuf[lkrecrpt].LockNoBuf + 1;		// 駐車位置番号(1～324)
			dat = FLAPDT.flp_data[LockBuf[lkrecrpt].LockNoBuf].nstat.word ^
					FLAPDT.flp_data[LockBuf[lkrecrpt].LockNoBuf].ostat.word;
			if( dat & 0x0001 ){										// 車両有無変化
				if( FLAPDT.flp_data[LockBuf[lkrecrpt].LockNoBuf].nstat.bits.b00 == 1 ){
					/*** 車両なし=>あり ***/
					ret += 100;
					syusei[FLPCTL.Room_no - 1].ot_car = 3;
					//入庫台数ｶｳﾝﾄ位置移動
				}else{
					/*** 車両あり=>なし ***/
					ret += 200;
					syusei[FLPCTL.Room_no - 1].ot_car = 2;
					//出庫台数ｶｳﾝﾄ位置移動
				}
			}
			if( dat & 0x0036 ){										// ﾛｯｸ装置状態変化
				ret += 50;
			}
			else{
				// この条件にはLKcom_RcvDataAnalys()で状態変化あり（ret==2）で返さない限り入ってこない為、初回かどうかの判定は省略
				if( LockBuf[lkrecrpt].lock_cnd == 4 ){				// 状態を下降ロック発生で受信した場合
					ret += 50;
				}
				else if( (dat & 0x0001) == 0 &&						// 車両有無変化なし
						 LockBuf[lkrecrpt].lock_cnd == 1 ){			// 状態を上昇済みで受信した場合
					ret += 50;
				}
			}
			lkrecrpt++;
			break;
		}
		if( lkrecrpt == lkreccnt ){
			lkreccnt = 0;											// 変化ﾃﾞｰﾀ数ｸﾘｱ
			lkrecrpt = 0;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 入庫判定時間起動(500msecﾀｲﾏｰ使用)                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : InChkTimer                                              |*/
/*| PARAMETER    : no  = 車室№1～324                                      |*/
/*| RETURN VALUE : ret = 入庫判定時間                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-09-03                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	InChkTimer( void )
{
	short	tim;

	if( Lock_Kind == LK_KIND_FLAP || Lock_Kind == LK_KIND_INT_FLAP ){// 装置種別？
		tim = (short)Carkind_Param(IN_CHK_TIME,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,3,4);					// 秒の設定を格納する
	}else{
		tim = (short)Carkind_Param(IN_CHK_TIME,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,3,1);					// 秒の設定を格納する
	}

	tim = tim * 2;												// 分を500ms単位に変換

	if( tim == 0 ){					// 設定0,種別異常時は最小値とする
		tim = 1;
	}

	return( tim );
}
/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ起動(500msecﾀｲﾏｰ使用)                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockTimer                                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret = ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
long	LockTimer( void )
{
	long	tim;
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//// MH322914 (s) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
//	long			svstime;			// ｻｰﾋﾞｽﾀｲﾑ設定値
//	uchar			syu;				// 車種
//// MH322914 (e) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)

	if( Lock_Kind == LK_KIND_FLAP || Lock_Kind == LK_KIND_INT_FLAP ){// 装置種別？
		// ﾌﾗｯﾌﾟ
		tim = Carkind_Param(FLAP_UP_TIMER, (char)(LockInfo[FLPCTL.Room_no - 1].ryo_syu), 5,1);
	}
	else{
		// ﾛｯｸ装置
		tim = Carkind_Param(ROCK_CLOSE_TIMER, (char)(LockInfo[FLPCTL.Room_no - 1].ryo_syu), 5,1);
	}
	tim = (long)(((tim/100)*60) + (tim%100));

// MH322914 (s) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//	if( prm_get( COM_PRM, S_TYP, 68, 1, 5 ) == 1 ) {
//		// ロック閉タイマーとサービスタイムが同じ時間の時にロック閉タイムをサービスタイムまで延長する
//		if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){	// 全共通ｻｰﾋﾞｽﾀｲﾑ？
//			svstime = CPrmSS[S_STM][2];				// 共通ｻｰﾋﾞｽﾀｲﾑ設定値取得
//		}else{
//			syu = LockInfo[FLPCTL.Room_no-1].ryo_syu;
//			svstime = CPrmSS[S_STM][5+(3*(syu-1))];	// 種別毎ｻｰﾋﾞｽﾀｲﾑ設定値取得
//		}
//		svstime *= 60;	/* サービスタイム：秒	*/
//		if(svstime == tim) {
//			// サービスタイム終了でロック閉するようにする
//			tim += 60 - CLK_REC.seco;
//		}
//	}
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
// MH322914 (e) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)

	tim *= 2;														// 500ms単位に変換

	if( tim == 0 ){													// 設定0時は最小値とする
		tim = 1;
	}
	return( tim );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾗｸﾞﾀｲﾑ起動(500msecﾀｲﾏｰ使用)                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagTimer                                                |*/
/*| PARAMETER    : no  = 車室№1～324                                      |*/
/*| RETURN VALUE : ret = ﾗｸﾞﾀｲﾑ                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
long	LagTimer( ushort no )
{
	long	tim;
	uchar	syu;

	tim = 0;														// ラグタイム値初期化
	if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){						// 全車種共通?
		/*** ﾗｸﾞﾀｲﾑは全車種共通 ***/
		tim = (long)CPrmSS[S_STM][4];
	}else{
		/*** ﾗｸﾞﾀｲﾑは車種毎で異なる ***/
		syu = SvsTime_Syu[no-1];									// 車室毎の精算時の料金種別get
		if(( 1 <= syu )&&( syu <= 12 )){
			tim = (long)CPrmSS[S_STM][7+((syu-1)*3)];
		}
	}
	tim = tim * 60 * 2;												// 分を500ms単位に変換
	if( tim == 0 ){													// 設定が０,種別範囲外
		tim = 1200L;												// 強制的に10分にする
	}

	return( tim );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ板上昇/下降開始                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FlpSet( no, updw )                                      |*/
/*| PARAMETER    : no   = 車室№1～324                                     |*/
/*|              : updw = 1:上昇(閉), 2:下降(開)                           |*/
/*|              : wait = 0:通常時動作,1:動作待ち状態                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	FlpSet( ushort no, uchar updw )
{
	short	tim;

	if( updw == 2 ){
		FLPCTL.flp_work.nstat.bits.b02 = 0;							// 下降動作
	}else{
		FLPCTL.flp_work.nstat.bits.b02 = 1;							// 上昇動作
	}
	IFM_LockTable.ucFlapWait = 1;									// フラップ動作中
	LKopeApiLKCtrl( no, updw );										// ﾌﾗｯﾌﾟ(ﾛｯｸ装置)動作指示

	if( Aida_Flap_Chk((ushort)(no-1)) ) {							// 装置種別が英田製フラップ
		flp_DownLock_FlpSet( (ushort)(no-1) );						// フラップ制御指示実施後の下降ロック保護処理
	}

	if( Lock_Kind == LK_KIND_FLAP || Lock_Kind == LK_KIND_INT_FLAP ){// 装置種別？
		// ﾌﾗｯﾌﾟ
		tim = 300;													// ﾌﾗｯﾌﾟ動作監視ﾀｲﾏｰ起動(150s)
	}
	else{
		// ﾛｯｸ装置
		tim = 60;													// ﾛｯｸ装置動作監視ﾀｲﾏｰ起動(30s)
	}
	FmvTimer( no, tim );											// 動作監視ﾀｲﾏｰ起動
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ板動作監視ﾀｲﾏｰ起動(500msﾀｲﾏｰ使用)                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FmvTimer( no, tim )                                     |*/
/*| PARAMETER    : no = 車室№1～324                                       |*/
/*|              : tim = ﾀｲﾏｰ値(500ms単位)                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	FmvTimer( ushort no, short tim )
{
	FLPCTL.Flp_mv_tm[no-1] = tim;									 // ﾌﾗｯﾌﾟ板動作監視ﾀｲﾏｰｾｯﾄ

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ、ﾗｸﾞﾀｲﾑﾀｲﾏｰ、                                 |*/
/*| ﾌﾗｯﾌﾟ板動作監視ﾀｲﾏｰ減算(500msﾀｲﾏｰ使用)                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SrvTimMng                                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SrvTimMng( void )
{
	short	i;

	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;																// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ、ﾗｸﾞﾀｲﾑﾀｲﾏｰ監視
		if( FLAPDT.flp_data[i].timer > 0 ){
			FLAPDT.flp_data[i].timer--;
		}
		// ﾌﾗｯﾌﾟ板(ﾛｯｸ装置)動作監視ﾀｲﾏｰ監視
		if( FLPCTL.Flp_mv_tm[i] > 0 ){
			FLPCTL.Flp_mv_tm[i]--;
		}
		// 入庫判定ﾀｲﾏｰ
		if( FLAPDT.flp_data[i].in_chk_cnt > 0){
			FLAPDT.flp_data[i].in_chk_cnt--;
		}
		/* フラップ下降ロック発生時保護処理 */
		if( (flp_DownLockErrInfo[i].f_info.BIT.EXEC == 1) &&				// リトライ動作中
			(flp_DownLockErrInfo[i].TimerCount != 0) )						// タイムアウトしていない
		{
			flp_DownLockErrInfo[i].TimerCount--;							// タイマーカウント
		}
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ上昇下降異常(ﾛｯｸ装置開閉異常)判定ﾀｲﾏｰ監視処理(5sﾀｲﾏｰ使用)         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LkErrTimMng                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-09-27                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LkErrTimMng( void )
{
	short	i;

	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;																// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		// 上昇ﾛｯｸ(閉異常)ｴﾗｰ判定ﾀｲﾏｰ
		if( FLPCTL.Flp_uperr_tm[i] > 0 ){
			FLPCTL.Flp_uperr_tm[i]--;
		}
		// 下降ﾛｯｸ(開異常)ｴﾗｰ判定ﾀｲﾏｰ
		if( FLPCTL.Flp_dwerr_tm[i] > 0 ){
			FLPCTL.Flp_dwerr_tm[i]--;
		}
	}

	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ、ﾗｸﾞﾀｲﾑﾀｲﾏｰ、ﾌﾗｯﾌﾟ板動作監視ﾀｲﾏｰの監視処理    |*/
/*| ﾀｲﾑｱｳﾄした物は順次ﾒｯｾｰｼﾞ登録する。                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LkTimChk                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-09-21                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LkTimChk( void )
{
	ushort	no;
	t_flp_DownLockErrInfo *pInfo = &flp_DownLockErrInfo[lktimoutno];

	//入庫判定時間監視
	if( FLAPDT.flp_data[lktimoutno].in_chk_cnt == 0){	// 入庫判定ﾀｲﾑｱｳﾄ
		
		no = lktimoutno + 1;
		queset( FLPTCBNO, CAR_INCHK_SND, sizeof( no ), &no );
		FLAPDT.flp_data[lktimoutno].in_chk_cnt = -1;				// ﾀｲﾑｱｳﾄ
		/* 入庫情報登録 */
		IoLog_write(IOLOG_EVNT_ENT_GARAGE, (ushort)LockInfo[lktimoutno].posi, 0, 0);
	}
	// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ、ﾗｸﾞﾀｲﾑﾀｲﾏｰ監視
	if( FLAPDT.flp_data[lktimoutno].timer == 0 ){					// ﾀｲﾑｱｳﾄ?

		if( (FLAPDT.flp_data[lktimoutno].lag_to_in.BYTE < _FLP_FURIKAE )||
			(FLAPDT.flp_data[lktimoutno].lag_to_in.BIT.SYUUS) ){

			FLAPDT.flp_data[lktimoutno].lag_to_in.BIT.SYUUS = 0;
		no = lktimoutno + 1;
		queset( FLPTCBNO, CAR_SVSLUG_SND, sizeof( no ), &no );
		FLAPDT.flp_data[lktimoutno].timer = -1;						// ﾀｲﾏｰｽﾄｯﾌﾟ
		} else {
			// 振替精算中につき、タイムアウト通知/タイマー停止は行わない
			FLAPDT.flp_data[lktimoutno].timer = 1;
		}
	}

	/* フラップ下降ロック発生時保護処理 */
	if( (pInfo->f_info.BIT.EXEC == 1) &&							// リトライ動作中
		(pInfo->TimerCount == 0) &&									// タイムアウトしている
		(pInfo->f_info.BIT.TO_MSG == 0) )							// タイムアウトメッセージ未送信
	{
		no = lktimoutno + 1;										// 対象の車室番号をｾｯﾄ
		queset( FLPTCBNO, DOWNLOCKOVER_SND, sizeof( no ), &no );	// 下降ロックタイムオーバー通知
		pInfo->f_info.BIT.TO_MSG = 1;								// タイムアウトメッセージ送信済み
	}

	// ﾌﾗｯﾌﾟ板(ﾛｯｸ装置)動作監視ﾀｲﾏｰ監視
	if( FLPCTL.Flp_mv_tm[lktimoutno] == 0 ){						// ﾀｲﾑｱｳﾄ?
		no = lktimoutno + 1;
		queset( FLPTCBNO, CAR_FLPMOV_SND, sizeof( no ), &no );
		FLPCTL.Flp_mv_tm[lktimoutno] = -1;							// ﾀｲﾏｰｽﾄｯﾌﾟ
	}

	// 上昇ﾛｯｸ(閉異常)ｴﾗｰ判定ﾀｲﾏｰ監視
	if( FLPCTL.Flp_uperr_tm[lktimoutno] == 0 ){						// ﾀｲﾑｱｳﾄ?
		FLPCTL.Flp_uperr_tm[lktimoutno] = -1;						// ﾀｲﾏｰｽﾄｯﾌﾟ
		if( FLAPDT.flp_data[lktimoutno].nstat.bits.b02 == 1 ){		// 上昇動作(閉動作異常)
			if( FLAPDT.flp_data[lktimoutno].nstat.bits.b04 == 1 ){	// 上昇ﾛｯｸ(閉ﾛｯｸ)
				ErrBinDatSet( (ulong)lktimoutno, 1 );
				if (lktimoutno < INT_CAR_START_INDEX ) {
					err_chk( ERRMDL_IFFLAP, ERR_FLAPCLOSEFAIL, 1, 2, 1 );
				}else if( lktimoutno < BIKE_START_INDEX ){
					err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LOCKCLOSEFAIL, 1, 2, 1 );
				} else {
					err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LOCKCLOSEFAIL, 1, 2, 1 );
				}
			}
		}
	}
	// 下降ﾛｯｸ(開異常)ｴﾗｰ判定ﾀｲﾏｰ監視
	if( FLPCTL.Flp_dwerr_tm[lktimoutno] == 0 ){						// ﾀｲﾑｱｳﾄ?
		FLPCTL.Flp_dwerr_tm[lktimoutno] = -1;						// ﾀｲﾏｰｽﾄｯﾌﾟ
		if( FLAPDT.flp_data[lktimoutno].nstat.bits.b02 == 0 ){		// 下降動作(開動作異常)
			if( FLAPDT.flp_data[lktimoutno].nstat.bits.b05 == 1 ){	// 下降ﾛｯｸ(開ﾛｯｸ)
				ErrBinDatSet( (ulong)lktimoutno, 1 );
				if (lktimoutno < INT_CAR_START_INDEX ) {
					err_chk( ERRMDL_IFFLAP, ERR_FLAPOPENFAIL, 1, 2, 1 );
				}else if( lktimoutno < BIKE_START_INDEX ){
					err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LOCKOPENFAIL, 1, 2, 1 );
				} else {
					err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LOCKOPENFAIL, 1, 2, 1 );
				}
			}
		}
	}

	lktimoutno++;
	if( lktimoutno >= LOCK_MAX ){
		lktimoutno = 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成処理(動作させなかった時用)                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data05_Sub                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-09-12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data05_Sub( void )
{
	uchar	CarCnd, LockCnd;
	void	(*_ntnet_snd_data05)(ulong, uchar, uchar);		/* 関数ポインタ */

	_ntnet_snd_data05 = NTNET_Snd_Data05;		// 通常のNT-NETへの送信ポインタをセット

	CarCnd = 0;
	LockCnd = 0;

	if( FLPCTL.Ment_flg == 0 && FLPCTL.flp_work.nstat.bits.b09 == 0 ){	// NT-NET?
		return;
	}

	if( FLPCTL.flp_work.nstat.bits.b08 == 0 ){						// 接続有り?
		if( FLPCTL.flp_work.nstat.bits.b00 ){						// 車両有り?
			CarCnd = 1;												// 車両有り
		}else{
			CarCnd = 2;												// 車両無し
		}
		if( FLPCTL.flp_work.nstat.bits.b01 ){						// 上昇済み?
			if( FLPCTL.flp_work.nstat.bits.b04 ){					// 上昇ﾛｯｸ?
				LockCnd = 3;										// ﾛｯｸ閉異常
			}else{
				LockCnd = 1;										// ﾛｯｸ閉済み
			}
		}else{
			if( FLPCTL.flp_work.nstat.bits.b05 ){					// 下降ﾛｯｸ?
				LockCnd = 4;										// ﾛｯｸ開異常
			}else{
				LockCnd = 2;										// ﾛｯｸ開済み
			}
		}
	}else{
		CarCnd = 0;													// 接続無し
		LockCnd = 6;												// 指定ﾛｯｸ装置接続無し
	}

	_ntnet_snd_data05( (ulong)( LockInfo[FLPCTL.Room_no - 1].area*10000L + LockInfo[FLPCTL.Room_no - 1].posi ),
					  CarCnd, LockCnd );							// NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成
	FLPCTL.flp_work.nstat.bits.b09 = 0;								// NT-NETﾌﾗｸﾞOFF
}

/*[]----------------------------------------------------------------------[]*/
/*| 入庫時の台数ｶｳﾝﾄ処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lk_InCount                                              |*/
/*| PARAMETER    : uc_prm : ｶｳﾝﾄする／しないの設定                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| 入出庫台数ｶｳﾝﾄはCarCount領域で行い、T集計印字時に集計ｴﾘｱへ加算する。   |*/
/*| 印字中も台数ｶｳﾝﾄし、印字完了時にT集計ｴﾘｱをｸﾘｱするﾀｲﾐﾝｸﾞでCarCount領域  |*/
/*| とT集計領域を比較し印字中に増えた台数を求め、増えた台数にCarCount領域  |*/
/*| を更新する。これにより印字中の台数ｶｳﾝﾄは次のT集計に反映される。        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-02-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	void	Lk_InCount( uchar uc_prm )
{
uchar	set1_39_5;
ulong	ul_lkNo_w = 0;												// 接客用ﾛｯｸ装置№

	set1_39_5 = (uchar)prm_get(COM_PRM,S_SYS,39,1,2);

	if( uc_prm == 1 ){												// 現在台数1ｶｳﾝﾄする設定?
		if( set1_39_5 == 2 ){										// 本体にてカウントする
			PPrmSS[S_P02][6]++;										// 現在台数1
		}
		CarCount.In_car_cnt[0]++;									// 入庫台数1
	}else if( uc_prm == 2 ){										// 現在台数2ｶｳﾝﾄする設定?
		if( set1_39_5 == 2 ){										// 本体にてカウントする
			PPrmSS[S_P02][10]++;									// 現在台数2
		}
		CarCount.In_car_cnt[1]++;									// 入庫台数2
	}else if( uc_prm == 3 ){										// 現在台数3ｶｳﾝﾄする設定?
		if( set1_39_5 == 2 ){										// 本体にてカウントする
			PPrmSS[S_P02][14]++;									// 現在台数3
		}
		CarCount.In_car_cnt[2]++;									// 入庫台数3
	}
	if( uc_prm ){													// 種別毎ｶｳﾝﾄする設定?
		if( set1_39_5 == 2 ){										// 本体にてカウントする
			PPrmSS[S_P02][2]++;										// 現在台数
			if( _is_ntnet_remote() && (prm_get(COM_PRM,S_NTN,121,1,1) != 0) ){	// 遠隔NT-NET設定 & 駐車場センター形式のときのみチェック
				if( (prm_get(COM_PRM,S_NTN,120,3,1)!=0) && (NT_pcars_timer == -1) ) {
					NT_pcars_timer = (short)(prm_get(COM_PRM,S_NTN,120,3,1) * 50);
				}
			}
		}
		CarCount.In_car_Tcnt++;										// 総入庫台数
	}
	//モニター登録
	ul_lkNo_w = (ulong)( LockInfo[FLPCTL.Room_no - 1].area*10000L +
					  LockInfo[FLPCTL.Room_no - 1].posi );	// 接客用ﾛｯｸ装置№
					  
	wmonlg(OPMON_CAR_IN,NULL,ul_lkNo_w);								// ﾓﾆﾀ登録（入庫）
}


/*[]----------------------------------------------------------------------[]*/
/*| 出庫時の台数ｶｳﾝﾄ処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lk_OutCount                                             |*/
/*| PARAMETER    : uc_prm : ｶｳﾝﾄする／しないの設定                         |*/
/*| 		     : mode : 0:在車ﾃﾞｰﾀ送信あり 1:在車ﾃﾞｰﾀ送信なし            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-02-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	void	Lk_OutCount( uchar uc_prm, uchar mode )
{

uchar	set1_39_5;
ulong	ul_lkNo_w = 0;												// 接客用ﾛｯｸ装置№

	set1_39_5 = (uchar)prm_get(COM_PRM,S_SYS,39,1,2);

	if( uc_prm == 1 ){												// 現在台数1ｶｳﾝﾄする設定?
		if( set1_39_5 == 2 ){										// 本体にてカウントする
			if( PPrmSS[S_P02][6] > 0 ){
				PPrmSS[S_P02][6]--;										// 現在台数1
			}
		}
		CarCount.Out_car_cnt[0]++;									// 出庫台数1
	}else if( uc_prm == 2 ){										// 現在台数2ｶｳﾝﾄする設定?
		if( set1_39_5 == 2 ){										// 本体にてカウントする
			if( PPrmSS[S_P02][10] > 0 ){
				PPrmSS[S_P02][10]--;									// 現在台数2
			}
		}
		CarCount.Out_car_cnt[1]++;									// 出庫台数2
	}else if( uc_prm == 3 ){										// 現在台数3ｶｳﾝﾄする設定?
		if( set1_39_5 == 2 ){										// 本体にてカウントする
			if( PPrmSS[S_P02][14] > 0 ){
				PPrmSS[S_P02][14]--;									// 現在台数3
			}
		}
		CarCount.Out_car_cnt[2]++;									// 出庫台数3
	}
	if( uc_prm ){													// 種別毎ｶｳﾝﾄする設定?
		if( set1_39_5 == 2 ){										// 本体にてカウントする
			if( PPrmSS[S_P02][2] > 0 ){
				PPrmSS[S_P02][2]--;										// 現在台数-1
				if( _is_ntnet_remote() && (prm_get(COM_PRM,S_NTN,121,1,1) != 0) ){	// 遠隔NT-NET設定 & 駐車場センター形式のときのみチェック
					if( (prm_get(COM_PRM,S_NTN,120,3,1)!=0) && (NT_pcars_timer == -1) ) {
						NT_pcars_timer = (short)(prm_get(COM_PRM,S_NTN,120,3,1) * 50);
					}
				}
			}
		}
		CarCount.Out_car_Tcnt++;									// 総出庫台数
	}
	//モニター登録
	ul_lkNo_w = (ulong)( LockInfo[FLPCTL.Room_no - 1].area*10000L +
					  LockInfo[FLPCTL.Room_no - 1].posi );	// 接客用ﾛｯｸ装置№
					  
	wmonlg(OPMON_CAR_OUT,NULL,ul_lkNo_w);								// ﾓﾆﾀ登録（出庫）

}

/*[]----------------------------------------------------------------------[]*/
/*| ﾛｯｸ装置種別取得処理                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LkKind_Get                                              |*/
/*| PARAMETER    : no : 装置No.                                            |*/
/*| RETURN VALUE : ﾛｯｸ装置種別                                             |*/
/*| 				LK_KIND_FLAP = ﾌﾗｯﾌﾟ	                               |*/
/*| 				LK_KIND_LOCK = ﾛｯｸ装置	                               |*/
/*|					LK_KIND_INT_FLAP = 内蔵ﾌﾗｯﾌﾟ						   |*/
/*| 				LK_KIND_ERR  = 不明	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| ﾊﾟﾗﾒｰﾀで指定されたﾛｯｸ装置がﾌﾗｯﾌﾟなのかﾛｯｸ装置なのか判定し出力する      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-09-11                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	LkKind_Get( ushort no )
{
	uchar	kind = LK_KIND_ERR;

	if( ( no >= FLAP_START_NO ) && ( no <= FLAP_END_NO ) ){
		// ﾌﾗｯﾌﾟ
		kind = LK_KIND_FLAP;
	}
	else if( ( no >= INT_FLAP_START_NO ) && ( no <= INT_FLAP_END_NO ) ){
		// ﾌﾗｯﾌﾟ装置
		kind = LK_KIND_INT_FLAP;
	}
	else if( ( no >= LOCK_START_NO ) && ( no <= LOCK_END_NO ) ){
		// ﾛｯｸ装置
		kind = LK_KIND_LOCK;
	}
	return( kind );
}

/*[]----------------------------------------------------------------------[]*/
/*| 全ﾛｯｸ装置動作処理                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lk_AllMove                                              |*/
/*|																		   |*/
/*| PARAMETER    :	lk_kind : 装置種別                                     |*/
/*| 					LK_KIND_FLAP = ﾌﾗｯﾌﾟ	                           |*/
/*| 					LK_KIND_LOCK = ﾛｯｸ装置	                           |*/
/*| 					LK_KIND_INT_FLAP = 内蔵ﾌﾗｯﾌﾟ装置                   |*/
/*|																		   |*/
/*| 				mov_req : 要求動作	                                   |*/
/*| 					０ = ﾌﾗｯﾌﾟ下降／ﾛｯｸ開	                           |*/
/*| 					１ = ﾌﾗｯﾌﾟ上昇／ﾛｯｸ閉	                           |*/
/*|																		   |*/
/*| 				req_kind : 要求元	                                   |*/
/*| 					０ = ﾒﾝﾃﾅﾝｽ操作			                           |*/
/*| 					１ = NT-NET(遠隔操作)	                           |*/
/*| RETURN VALUE : なし	                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| 「全ﾌﾗｯﾌﾟ上昇／下降」、「全ﾛｯｸ装置開／閉」要求に対する処理を行う       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-09-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	void	Lk_AllMove( uchar lk_kind, uchar mov_req, uchar req_kind )
{
	ushort		s_no;			// 動作対象装置No.(start)
	ushort		e_no;			// 動作対象装置No.(end)
	uchar		mtype;			// ﾌﾗｯﾌﾟ／ﾛｯｸ種別
	ushort		lk_ofs;			// 装置No.ｵﾌｾｯﾄ(装置No.-1)
	ushort		lk_cnt = 0;		// 動作対象装置数
	flp_com		*lk_data;		// 駐車位置情報ﾎﾟｲﾝﾀｰ
	uchar		force_exec=0;	// 強制出庫ﾌﾗｸﾞ

	// 装置種別を判断し動作対象とする装置No.をｾｯﾄ
	if( lk_kind == LK_KIND_FLAP ){
		if( m_mode_chk_all( LK_KIND_FLAP ) == ON ){
			// 何れかのﾀｰﾐﾅﾙが手動ﾓｰﾄﾞの場合動作ＮＧとする
			return;
		}
		s_no = FLAP_START_NO;
		e_no = FLAP_END_NO;
		mtype = _MTYPE_FLAP;
	}
	else if( lk_kind == LK_KIND_INT_FLAP ){
		if( m_mode_chk_all( LK_KIND_INT_FLAP ) == ON ){
			// 何れかのﾀｰﾐﾅﾙが手動ﾓｰﾄﾞの場合動作ＮＧとする
			return;
		}
		s_no = INT_FLAP_START_NO;
		e_no = INT_FLAP_END_NO;
		mtype = _MTYPE_INT_FLAP;		
	}
	else{
		if( m_mode_chk_all( LK_KIND_LOCK ) == ON ){
			// 何れかのﾀｰﾐﾅﾙが手動ﾓｰﾄﾞの場合動作ＮＧとする
			return;
		}
		s_no = LOCK_START_NO;
		e_no = LOCK_END_NO;
		mtype = _MTYPE_LOCK;
	}
	// 対象とする全ての装置に対し処理を行う
	for( lk_ofs = (s_no-1) ; lk_ofs < e_no ; lk_ofs++ ){
		WACDOG;										// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if (LockInfo[lk_ofs].lok_syu != 0 ) {		// 接続あり？

			lk_cnt++;								// 動作対象装置ｶｳﾝﾄ＋1

			lk_data = &FLAPDT.flp_data[lk_ofs];		// 駐車位置情報ﾎﾟｲﾝﾀｰｾｯﾄ

			if( mov_req == 0 ){						// 要求動作？
				if(FLPCTL.flp_work.mode == FLAP_CTRL_MODE4)		// 現在フラップが上昇済み
				{
					/* 強制出庫開始情報登録 */
					IoLog_write(IOLOG_EVNT_FORCE_STA, (ushort)LockInfo[lk_ofs].posi, 0, 0);
					force_exec = 1;					// 強制出庫ﾌﾗｸﾞON
				}
				// ﾌﾗｯﾌﾟ下降／ﾛｯｸ開
				lk_data->nstat.bits.b02 = 0;		// 下降動作ﾌﾗｸﾞON
				Flapdt_sub_clear(lk_ofs);			// 中止詳細ｴﾘｱｸﾘｱ
			}
			else{
				// ﾌﾗｯﾌﾟ上昇／ﾛｯｸ閉
				lk_data->nstat.bits.b02 = 1;		// 上昇動作ﾌﾗｸﾞON
			}

			if( req_kind == 0 ){					// 要求元？
				// ﾒﾝﾃﾅﾝｽ操作
				lk_data->nstat.bits.b07 = 1;		// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞON
				lk_data->nstat.bits.b09 = 0;		// NT-NETﾌﾗｸﾞOFF
			}
			else{
				// NT-NET(遠隔操作)
				lk_data->nstat.bits.b07 = 1;		// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞON
				lk_data->nstat.bits.b09 = 1;		// NT-NETﾌﾗｸﾞON
				lk_data->nstat.bits.b15 = 1;		// 遠隔からのﾌﾗｯﾌﾟ/ﾛｯｸ操作あり
				if( req_kind == 2 ){
					lk_data->nstat.bits.b14 = 1;	// MAFからの要求
				}else{
					lk_data->nstat.bits.b14 = 0;	// NT-NETからの要求
				}
			}
		}
	}
	if( lk_cnt != 0 ){								// 動作対象とする装置あり？
		if( mov_req == 0 ){
			// ﾌﾗｯﾌﾟ下降／ﾛｯｸ開
			LKopeApiLKCtrl_All( mtype, 6 );			// 全装置（下降/開）制御ﾃﾞｰﾀ送信要求
			if( force_exec ){
				if( lk_kind == LK_KIND_LOCK ){
					ope_anm( AVM_FORCE_ROCK );			// 下降ｱﾅｳﾝｽ(ロック)
				}else{
					ope_anm( AVM_FORCE_FLAP );			// 開ｱﾅｳﾝｽ(フラップ)
				}
			}
		}
		else{
			// ﾌﾗｯﾌﾟ上昇／ﾛｯｸ閉
			LKopeApiLKCtrl_All( mtype, 5 );			// 全装置（上昇/閉）制御ﾃﾞｰﾀ送信要求
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 個別ﾛｯｸ装置動作要求処理                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Mnt_Lk_Move                                             |*/
/*|																		   |*/
/*| PARAMETER	：	mov_req : 要求動作	                                   |*/
/*| 					ON  = ﾌﾗｯﾌﾟ上昇／ﾛｯｸ閉	                           |*/
/*| 					OFF = ﾌﾗｯﾌﾟ下降／ﾛｯｸ開	                           |*/
/*|																		   |*/
/*| RETURN VALUE : 動作結果	                                               |*/
/*| 					OK = 要求動作実行		                           |*/
/*| 					NG = 要求動作無効		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| ﾒﾝﾃﾅﾝｽ操作による個別のﾌﾗｯﾌﾟ（上昇／下降）／ﾛｯｸ装置（開／閉）処理を行う |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-10-04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	uchar	Mnt_Lk_Move( uchar mov_req )
{
	uchar	ret = NG;
	uchar	m_mode = OFF;


	m_mode = m_mode_chk_psl( FLPCTL.Room_no );					// 手動ﾓｰﾄﾞ状態取得
	
	if( mov_req == ON ){
		// ﾌﾗｯﾌﾟ上昇（ﾛｯｸ閉）要求
		if( ( FLPCTL.Flp_mv_tm[ FLPCTL.Room_no - 1 ] == -1 )	// 動作中以外?
					&&											//   且つ
			( m_mode == OFF )									// 手動ﾓｰﾄﾞ中でない
		){

			if(FLPCTL.Ment_flg != 0) {
				if(FLPCTL.flp_work.nstat.bits.b01 == 1 &&		// 上昇済み
					FLPCTL.flp_work.nstat.bits.b04 == 0 &&		// 上昇ロックではない
					FLPCTL.flp_work.nstat.bits.b08 == 0 &&		// 接続あり
					FLPCTL.flp_work.nstat.bits.b12 == 0) {		// 上昇異常ではない
					FLPCTL.flp_work.nstat.bits.b09 = 1;
					// 要求動作無効
					NTNET_Snd_Data05_Sub();						// NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成
					return NG;
				}
			}
			FlpSet( FLPCTL.Room_no, 1 );						// ﾌﾗｯﾌﾟ上昇（ﾛｯｸ閉）要求
			ret = OK;
		}
	}
	else{
		// ﾌﾗｯﾌﾟ下降（ﾛｯｸ開）要求
		if( ( FLPCTL.Flp_mv_tm[ FLPCTL.Room_no - 1 ] == -1 )	// 動作中以外?
					&&											//   且つ
			( m_mode == OFF )									// 手動ﾓｰﾄﾞ中でない
		){
			if(FLPCTL.Ment_flg != 0) {
				if(FLPCTL.flp_work.nstat.bits.b01 == 0 &&		// 下降済み
					FLPCTL.flp_work.nstat.bits.b05 == 0 &&		// 下降ロックではない
					FLPCTL.flp_work.nstat.bits.b08 == 0 &&		// 接続あり
					FLPCTL.flp_work.nstat.bits.b13 == 0) {		// 下降異常ではない
					FLPCTL.flp_work.nstat.bits.b09 = 1;			// NTNETフラグON
					FLPCTL.Ment_flg = 0;						// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞｸﾘｱ
					// 要求動作無効
					NTNET_Snd_Data05_Sub();						// NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成
					return NG;
				}
			}
			FlpSet( FLPCTL.Room_no, 2 );						// ﾌﾗｯﾌﾟ下降（ﾛｯｸ開）要求
			ret = OK;
		}
	}
	if( ret == OK ){
		// 要求動作実行
		if( FLPCTL.Ment_flg == 0 ){								// ﾒﾝﾃﾅﾝｽ
			FLPCTL.flp_work.nstat.bits.b07 = 1;					// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞON
			FLPCTL.flp_work.nstat.bits.b09 = 0;					// NT-NETﾌﾗｸﾞOFF
		}else{													// NT-NET
			FLPCTL.flp_work.nstat.bits.b07 = 1;					// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞON
			FLPCTL.flp_work.nstat.bits.b09 = 1;					// NT-NETﾌﾗｸﾞON
			FLPCTL.flp_work.nstat.bits.b15 = 1;					// 遠隔からのﾌﾗｯﾌﾟ/ﾛｯｸ操作あり
			if( FLPCTL.Ment_flg == 2 ){
				FLPCTL.flp_work.nstat.bits.b14 = 1;				// MAFからの要求
			}else{
				FLPCTL.flp_work.nstat.bits.b14 = 0;				// NT-NETからの要求
			}
		}
	}
	else{
		// 要求動作無効
		NTNET_Snd_Data05_Sub();									// NT-NETﾛｯｸ制御結果ﾃﾞｰﾀ作成
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ/ﾛｯｸ装置手動ﾓｰﾄﾞﾁｪｯｸ処理（個別）                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : m_mode_chk_psl                                          |*/
/*|																		   |*/
/*| PARAMETER	：	no : 装置No.							               |*/
/*|																		   |*/
/*| RETURN VALUE : ﾁｪｯｸ結果	                                               |*/
/*| 					OFF = 手動ﾓｰﾄﾞでない		                       |*/
/*| 					ON  = 手動ﾓｰﾄﾞ中		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| 指定されたﾌﾗｯﾌﾟ装置の手動ﾓｰﾄﾞ状態をﾁｪｯｸし戻り値として出力する 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-10-17                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	m_mode_chk_psl( ushort no )
{
	uchar	mode = OFF;
	uchar	tno;
	
	if( (no >= 1) && (no <= LOCK_MAX) ){			// 装置No.ﾁｪｯｸ

		tno = LockInfo[no - 1].if_oya;				// 接続されているﾀｰﾐﾅﾙNo.取得

		if( no <= INT_FLAP_END_NO ){
			// ﾌﾗｯﾌﾟ装置
			mode = flp_m_mode[tno-1];				// 手動ﾓｰﾄﾞ状態取得
		}
		else{
			// ﾛｯｸ装置
			mode = lok_m_mode[tno-1];				// 手動ﾓｰﾄﾞ状態取得
		}
	}
	return( mode );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ/ﾛｯｸ装置手動ﾓｰﾄﾞﾁｪｯｸ処理（全て）                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : m_mode_chk_all                                          |*/
/*|																		   |*/
/*| PARAMETER	：	kind : 装置種別							               |*/
/*|						LK_KIND_FLAP＝ﾌﾗｯﾌﾟ装置							   |*/
/*|						LK_KIND_LOCK＝ﾛｯｸ装置							   |*/
/*| 					LK_KIND_INT_FLAP = 内蔵ﾌﾗｯﾌﾟ装置                   |*/
/*|																		   |*/
/*| RETURN VALUE : ﾁｪｯｸ結果	                                               |*/
/*| 					OFF = 手動ﾓｰﾄﾞでない		                       |*/
/*| 					ON  = 手動ﾓｰﾄﾞ中		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| 指定された装置種別の手動ﾓｰﾄﾞ状態をﾁｪｯｸし戻り値として出力する 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-10-17                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	m_mode_chk_all( uchar kind )
{
	uchar	mode = OFF;
	uchar	tno;
	uchar	type;
	
	if( kind == LK_KIND_FLAP ){
		// ﾌﾗｯﾌﾟ装置のﾁｪｯｸ
		for ( tno = 0 ; tno < FLAP_IF_MAX ; tno++ ){	// 全ﾀｰﾐﾅﾙﾁｪｯｸ
			if( flp_m_mode[tno] == ON ){
				mode = ON;
				break;
			}
		}
	}
	else{
		// ﾛｯｸ装置のﾁｪｯｸ
		for ( tno = 0 ; tno < LOCK_IF_MAX ; tno++ ){	// 全ﾀｰﾐﾅﾙﾁｪｯｸ
			type = LKcom_Search_Ifno( (uchar)(tno+1) );
			if( lok_m_mode[tno] == ON && 
			  (( kind == LK_KIND_LOCK && !type ) ||
			   ( kind == LK_KIND_INT_FLAP && type ))){
				mode = ON;
				break;
			}
		}
	}
	return( mode );
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 詳細中止ｴﾘｱｸﾘｱ処理																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: Flapdt_sub_clear( no )														   |*/
/*| PARAMETER	: no	; 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: H.Sekiguchi																	   |*/
/*| Date		: 2006-10-24																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Flapdt_sub_clear(ushort no){
	struct CAR_TIM wk_time1,wk_time2;
	uchar	i;
	uchar	buf_size;
	ulong	posi;

	posi = LockInfo[no].posi;				//内部番号より駐車位置を取得

	for(i = 0 ; i < 10 ; i++){
		if(FLAPDT_SUB[i].WPlace == posi){		//駐車位置が同じ
			/*入庫時間*/
			wk_time1.year = FLAPDT.flp_data[no].year ;
			wk_time1.mon = FLAPDT.flp_data[no].mont ;
			wk_time1.day = FLAPDT.flp_data[no].date ;
			wk_time1.hour = FLAPDT.flp_data[no].hour ;
			wk_time1.min = FLAPDT.flp_data[no].minu ;
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
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 振替精算開始処理																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_Furikae_start(void)														   |*/
/*| PARAMETER	: ushort; 振替元車室No															   |*/
/*| RETURN VALUE: char	; 1:成功 0:失敗															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-04																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	ope_Furikae_start(ushort no)
{
	if( FLAPDT.flp_data[no - 1].lag_to_in.BIT.SYUUS == 1 ){
		return 0;
	}
	if ((FLAPDT.flp_data[no - 1].mode == FLAP_CTRL_MODE5 ||
		 FLAPDT.flp_data[no - 1].mode == FLAP_CTRL_MODE6) &&
		FLAPDT.flp_data[no - 1].timer != -1) {
		// 振替精算開始OK
		FLAPDT.flp_data[no - 1].lag_to_in.BIT.FURIK = 1;
		memcpy(&flp_com_frs, &FLAPDT.flp_data[no - 1], sizeof(flp_com));

		return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 振替精算終了処理																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_Furikae_stop																   |*/
/*| PARAMETER	: ushort; 振替元車室No / char	; 0:中止 1:完了									   |*/
/*| RETURN VALUE: char	; 1:成功 0:失敗															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-04																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	ope_Furikae_stop(ushort no, char bOK)
{

	if (((FLAPDT.flp_data[no - 1].mode == FLAP_CTRL_MODE5 || (FLAPDT.flp_data[no - 1].mode == FLAP_CTRL_MODE6)) &&
		(FLAPDT.flp_data[no - 1].lag_to_in.BIT.FURIK))) {
		// モードが4または5、かつラグタイマ停止中(振替中)なら振替成功
		FLAPDT.flp_data[no - 1].lag_to_in.BIT.FUKUG = 0;
		FLAPDT.flp_data[no - 1].lag_to_in.BIT.FURIK = 0;
		FLAPDT.flp_data[no - 1].lag_to_in.BIT.SYUUS = 0;

		if (bOK) {
			// 振替完了
			FLAPDT.flp_data[no - 1].lag_to_in.BIT.FUKUG = 1;
			FLAPDT.flp_data[no - 1].timer = 0;				// ラグタイマータイムアウト発生
		}
	} else {
		if (bOK) {
			// 車なし、もしくは別の車がいる場合は振替失敗 -> 再入庫
			flp_com_frs.lag_to_in.BIT.FURIK = 0;
			flp_com_frs.lag_to_in.BIT.SYUUS = 0;
			flp_com_frs.lag_to_in.BIT.FUKUG = 1;
			if(( prm_get( COM_PRM,S_TYP,73,1,1 ) == 1 )&&		// 英田ﾊﾞｲｸ不正出庫なし
			   ( LockInfo[no-1].lok_syu == LK_TYPE_AIDA2 )){	// 車室設定が英田ﾊﾞｲｸ
				// 英田ﾊﾞｲｸ時は不正出庫なしとする
			}
			else {
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
//				Make_Log_Enter_frs(no, (void*)&flp_com_frs);	// 仮入庫データ
//				Log_regist( LOG_ENTER );						// 入庫履歴登録
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
				ope_RegistFuseiData(no, &flp_com_frs, 1);		// 不正出庫
			}
		}
	}
	return 1;
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| 復電時 振替精算処理																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_Furikae_fukuden(void)														   |*/
/*| PARAMETER	: void	;																		   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-04																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	ope_Furikae_fukuden(void)
{
	int	i;

// 振替精算途中のものをクリアする
	for (i = 0; i < LOCK_MAX; i++) {
		WACDOG;																// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if (FLAPDT.flp_data[i].lag_to_in.BIT.FURIK) {
			FLAPDT.flp_data[i].lag_to_in.BIT.FUKUG = 0;
			FLAPDT.flp_data[i].lag_to_in.BIT.FURIK = 0;
			FLAPDT.flp_data[i].lag_to_in.BIT.SYUUS = 0;
		}
	}
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| 不正出庫登録処理																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_RegistFuseiData(void)														   |*/
/*| PARAMETER	: flp_com	登録車室データ														   |*/
/*|          	: ushort	登録車室番号														   |*/
/*|          	: uchar		振替精算による不正出庫登録なら1										   |*/
/*| RETURN VALUE: char	; 0：不正出庫															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-17																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	ope_RegistFuseiData(ushort no, flp_com *flp, char BFrs)
{
	if( fusei.kensuu >= LOCK_MAX ){
		fusei.kensuu = LOCK_MAX - 1;
	}
	fusei.fus_d[fusei.kensuu].iyear = flp->year;	// In Time
	fusei.fus_d[fusei.kensuu].imont = flp->mont;
	fusei.fus_d[fusei.kensuu].idate = flp->date;
	fusei.fus_d[fusei.kensuu].ihour = flp->hour;
	fusei.fus_d[fusei.kensuu].iminu = flp->minu;

	memcpy( &fusei.fus_d[fusei.kensuu].oyear, &CLK_REC.year, 6 );	// Out Time
	fusei.fus_d[fusei.kensuu].t_iti = no;

	if((( flp->nstat.bits.b01 == 0 )&&			// 下降済み(ﾛｯｸ開済み)
	    ( flp->nstat.bits.b07 == 1 ))||			// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞON
	   ( flp->nstat.bits.b03 == 1 )){			// 強制出庫
		// 強制出庫
		fusei.fus_d[fusei.kensuu].kyousei = 1;	// 強制ﾌﾗｸﾞON
		if( flp->nstat.bits.b15 ){				// // 遠隔からのﾌﾗｯﾌﾟ/ﾛｯｸ操作あり？
			fusei.fus_d[fusei.kensuu].kyousei = 11;
			flp->nstat.bits.b15 = 0;
		}
		flp_flag_clr( FLAP_CTRL_MODE1, CAR_SENSOR_OFF );
		flp->nstat.bits.b06 = 0;				// 不正ｸﾘｱ
// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
		// 長期駐車の場合、強制出庫によるアラーム解除は下降指示を受けた時ではなく
		// 強制出庫が完了した時点で行わないと次の時計歩進で誤検知するためここで解除
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//		LongTermParkingRel( no );// 長期駐車解除(強制出庫)
		LongTermParkingRel( no, LONGPARK_LOG_KYOUSEI_FUSEI, flp);// 長期駐車解除(強制出庫)
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
	}
	else{
		// 不正出庫
		flp->mode = FLAP_CTRL_MODE5;
		fusei.fus_d[fusei.kensuu].kyousei = 0;	// 強制ﾌﾗｸﾞOFF
		IoLog_write(IOLOG_EVNT_OUT_ILLEGAL_START, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
		if (!BFrs) {
			FlpSet( no, 2 );						// Flaper DOWN
		}
		flp->nstat.bits.b06 = 1;				// 不正
// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//		LongTermParkingRel( no );// 長期駐車解除(不正出庫)
		LongTermParkingRel( no, LONGPARK_LOG_KYOUSEI_FUSEI, flp);// 長期駐車解除(不正出庫)
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
	}
	fusei.kensuu ++;							// 不正・強制件数+1

	return 0;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| フラップ上昇、ロック閉ﾀｲﾏｰ内出庫登録処理													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_LockTimeOutData(void)														   |*/
/*| PARAMETER	: flp_com	登録車室データ														   |*/
/*|          	: ushort	登録車室番号														   |*/
/*| RETURN VALUE: char	; 0：フラップ上昇、ロック閉出庫											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-17																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	ope_LockTimeOutData(ushort no, flp_com *flp)
{
	if( locktimeout.kensuu >= LOCK_MAX ){
		locktimeout.kensuu = LOCK_MAX - 1;
	}
	locktimeout.fus_d[locktimeout.kensuu].iyear = flp->year;	// In Time
	locktimeout.fus_d[locktimeout.kensuu].imont = flp->mont;
	locktimeout.fus_d[locktimeout.kensuu].idate = flp->date;
	locktimeout.fus_d[locktimeout.kensuu].ihour = flp->hour;
	locktimeout.fus_d[locktimeout.kensuu].iminu = flp->minu;

	memcpy( &locktimeout.fus_d[locktimeout.kensuu].oyear, &CLK_REC.year, 6 );	// Out Time
	locktimeout.fus_d[locktimeout.kensuu].t_iti = no;

	locktimeout.kensuu ++;							// フラップ上昇、ロック閉ﾀｲﾏ内出庫件数+1

	return 0;
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| 新修正精算用関数（修正精算開始）                                                               |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_SyuseiStart(void)	                                                           |*/
/*| PARAMETER	: flp_com	登録車室データ														   |*/
/*|          	: ushort	登録車室番号														   |*/
/*| RETURN VALUE: char	; 0：フラップ上昇、ロック閉出庫											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-17																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	ope_SyuseiStart(ushort no)
{
	FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].lag_to_in.BIT.SYUUS = 1;	
	memcpy(&flp_com_frs, &FLAPDT.flp_data[no - 1], sizeof(flp_com));
}

/*[]----------------------------------------------------------------------[]*/
/*| 新修正精算(不正出庫登録)                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SyuseiFuseiSet(void)                                    |*/
/*| PARAMETER    : no : フラップ№                                         |*/
/*|              : type : 1=修正元車室がﾗｸﾞﾀｲﾑｱｯﾌﾟ後の場合                 |*/
/*|              :          ・FLAPDTの入庫時刻～現在時刻                   |*/
/*|              :          ・駐車料金は強制0円                            |*/
/*|              :        0=その他の修正での不正出庫                       |*/
/*|              :          ・修正ﾊﾞｯﾌｧの入庫時刻～出庫時刻                |*/
/*|              :          ・修正ﾊﾞｯﾌｧでの駐車料金                        |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Akiba                                                 |*/
/*| Date         : 2008-03-07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SyuseiFuseiSet( ushort no, uchar type )
{

	if( fusei.kensuu >= LOCK_MAX ){
		fusei.kensuu = LOCK_MAX - 1;
	}

	if( type == 1 ){
		memcpy( &fusei.fus_d[fusei.kensuu].iyear,&FLAPDT.flp_data[no-1].year,6 );	// In Time
		memcpy( &fusei.fus_d[fusei.kensuu].oyear,&CLK_REC.year,6 );					// Out Time
	}else{ 
		memcpy( &fusei.fus_d[fusei.kensuu].iyear,&syusei[no-1].iyear,6 );			// In Time
		memcpy( &fusei.fus_d[fusei.kensuu].oyear,&syusei[no-1].oyear,6 );			// Out Time
	}

	fusei.fus_d[fusei.kensuu].t_iti = no;
	fusei.fus_d[fusei.kensuu].kyousei = (uchar)(type+2);	// 強制ﾌﾗｸﾞOFF(カウントなし）

	fusei.kensuu ++;							// 不正・強制件数+1

	return;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 下降指示送信可能チェック（ｎ分ｍ回リトライ処理用）                                             |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: 車室情報データ（目的車室の最新データとして flp_work を参照する				   |*/
/*| RETURN VALUE: uchar	; 1=下降ロックリトライ処理実行可能（下降指示送信可能）					   |*/
/*|						  0=下降指示送信不可													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																			   |*/
/*| Date		: 2008/10/16																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
uchar	flp_DownLock_DownSendEnableCheck( void )
{
	flp_com	*p_flp_com = &FLPCTL.flp_work;
	uchar ret = 0;

	if( (p_flp_com->nstat.bits.b02 == 0) &&						  // 下降動作（下降/上昇指示のうち最後に送信したもの）
		(p_flp_com->nstat.bits.b05 == 1) )						  // 下降ロックエラー発生中
	{
		ret = 1;
	}
	return	ret;
}

uchar	flp_DownLock_DownSendEnableCheck2( ushort no )
{
	flp_com	*p_flp_com = &FLAPDT.flp_data[no];
	uchar ret = 0;

	if( (p_flp_com->nstat.bits.b02 == 0) &&						  // 下降動作（下降/上昇指示のうち最後に送信したもの）
		(p_flp_com->nstat.bits.b05 == 1) )						  // 下降ロックエラー発生中
	{
		ret = 1;
	}
	return	ret;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	エラー登録処理																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: DownLockErr()																				   |*/
/*| PARAMETER	: no	: 車室No（0～）																		   |*/
/*| 			: ErrNo	: エラーNo (モジュールコード11の下2桁)												   |*/
/*| 			: knd	: 1=発生，0=解除																	   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_ErrSet( ushort no, char ErrNo, char knd )
{
	uchar	edit=0;														// エラー状態変化：1=あり
	ulong	err_sub;
	t_flp_DownLockErrInfo* pInfo = &flp_DownLockErrInfo[no];
	
	switch( ErrNo ){
	case ERR_FLAPLOCK_DOWNRETRY:										// E1641:リトライ動作開始エラー
		if( pInfo->f_info.BIT.RETRY_START_ERR != knd ){					// 変化あり
		    pInfo->f_info.BIT.RETRY_START_ERR = knd;					// 新状態save
			edit = 1;
		}
		break;

	case ERR_FLAPLOCK_DOWNRETRYOVER:									// E1640:リトライオーバーエラー
		if( pInfo->f_info.BIT.RETRY_OVER_ERR != knd ){					// 変化あり
		    pInfo->f_info.BIT.RETRY_OVER_ERR = knd;						// 新状態save
			edit = 1;
		}
		break;

	default:
		break;
	}

	if( edit == 1 ){													// 変化あり
		err_sub = flp_ErrBinDataEdit( no );								// エラー登録用車室番号作成
		if( no < INT_CAR_START_INDEX )
			err_chk2( ERRMDL_IFFLAP, ErrNo, knd, 2, 1, &err_sub );		// IF盤装置へエラー登録
		else
			err_chk2( ERRMDL_FLAP_CRB, ErrNo, knd, 2, 1, &err_sub );		// CRR基板装置へエラー登録
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	エラー情報の編集																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: 車室No（0～）																		   |*/
/*| RETURN VALUE: エラー登録用の区画情報																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008-10-06																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
ulong	flp_ErrBinDataEdit( ushort no )
{
	ulong	ul;

	ul = (ulong)LockInfo[no].area * 10000;
	ul += (ulong)LockInfo[no].posi;
	return (ul);
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	下降ロックタイマーエリアの初期化																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: void																						   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																					   |*/
/*| Date		: 2008-10-06																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_Initial( void )
{
	memset( flp_DownLockErrInfo, 0, sizeof( flp_DownLockErrInfo ));
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	リトライ用ｎ分タイマー開始																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| 最初でもｍ回リトライ中でもCallされる																	   |*/
/*|	リトライ機能開始判断もここで行う																		   |*/
/*|	flp_DownLock_DownSendEnableCheck() or flp_DownLock_DownSendEnableCheck2()の実行可能チェック実施後にCall	   |*/
/*|	すること																								   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	  : 車室No（0～323）																   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_RetryTimerStart( ushort no )
{
	ushort	prm1 = (ushort)prm_get(COM_PRM, S_TYP, 74, 2, 3);		// n分タイマー値get
	ushort	prm2 = (ushort)prm_get(COM_PRM, S_TYP, 74, 2, 1);		// m回リトライ回数get

	/* n分m回リトライタイマー起動開始可能チェック */
	if( (prm1 && prm2) &&											// n分m回リトライ処理実行設定あり
		( Aida_Flap_Chk( no )) )				// 装置種別が英田製フラップ
		/*(flp_DownLock_DownSendEnableCheck()) )*/					// 下降指示送信可能(ここでは出来ない)
	{
		flp_DownLockErrInfo[no].TimerCount = (prm1 * 60 * 2);		// タイマ値(x500ms)set
		flp_DownLockErrInfo[no].f_info.BIT.TO_MSG = 0;				// タイムアウトメッセージ未送信

		if( flp_DownLockErrInfo[no].f_info.BIT.EXEC == 0 ){			// 最初の起動（今までリトライ動作実行中で無い）
			flp_DownLockErrInfo[no].RetryCount = 0;					// リトライ回数クリア
			flp_DownLock_ErrSet( no, ERR_FLAPLOCK_DOWNRETRY, 1 );	// リトライ開始エラー発生
			flp_DownLockErrInfo[no].f_info.BIT.EXEC = 1;			// タイマースタート
		}
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	リトライ動作停止																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: 車室No（0～323）																	   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_RetryStop( ushort no )
{
	if( flp_DownLockErrInfo[no].f_info.BIT.EXEC == 1 ){				// リトライ動作中
		flp_DownLockErrInfo[no].f_info.BIT.EXEC = 0;				// 動作停止（タイマーもストップ）
		flp_DownLock_ErrSet( no, ERR_FLAPLOCK_DOWNRETRY, 0 );		// リトライ開始エラー解除
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	リトライ中 下降指示送信処理																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	送信可能状況かを確認し、可能であれば下降指示を送信しｎ分タイマーを再起動する							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: 車室No（0～323）																	   |*/
/*| RETURN VALUE: 1 = フラップ下降指示実施、0=未実施														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
uchar	flp_DownLock_DownSend( ushort no )
{
	ushort	prm2 = (ushort)prm_get(COM_PRM, S_TYP, 74, 2, 1);		// m回リトライ回数get
	uchar	ret = 0;

	if( flp_DownLockErrInfo[no].f_info.BIT.EXEC == 1 ){				// リトライ動作中である
		if( flp_DownLock_DownSendEnableCheck() ){					// 下降指示送信可能
			if( flp_DownLockErrInfo[no].RetryCount < prm2 ){		// リトライオーバーでない
				++flp_DownLockErrInfo[no].RetryCount;

				/* リトライ動作による下降指示 */
				flp_f_DownLock_RetryDownCtrl = 1;					// リトライ動作による下降指示(ON)
				FlpSet( (ushort)(no+1), 2 );						// Flaper DOWN
																	// この間に　flp_DownLock_FlpSet() がCallされる
				flp_f_DownLock_RetryDownCtrl = 0;					// リトライ動作による下降指示(OFF)
				ret = 1;
			}
			else{													// リトライオーバー
				flp_DownLock_ErrSet( no, ERR_FLAPLOCK_DOWNRETRYOVER, 1 );	// リトライオーバーエラー発生
				flp_DownLock_RetryStop( no );						// リトライ動作終了
			}
		}
	}
	return	ret;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	上昇/下降フラップ指示送信後にCallされる処理	（ FlpSet()からCallされる ）								   |*/
/*| （フラップ装置の場合のみCallされる）																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: 車室No（0～323）																	   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_FlpSet( ushort no )
{
	/* 下降指示送信後 */
	if( FLPCTL.flp_work.nstat.bits.b02 == 0 ){						// 下降動作指示後
		if( FLPCTL.flp_work.nstat.bits.b05 == 1 ){					// 下降ロックエラー発生中
			// n分m回リトライタイムアウトによる下降指示ではない場合は
			// そこから新たにn分m回リトライを開始する。
			// （リトライ回数=0回、タイマー再起動）
			if( flp_f_DownLock_RetryDownCtrl == 0 ){				// n分m回リトライタイムアウトによる下降指示ではない
				flp_DownLockErrInfo[no].RetryCount = 0;				// リトライ回数クリア
			}
			flp_DownLock_RetryTimerStart( no );						// n分m回リトライタイマー開始（又はリスタート）
		}
	}
	/* 上昇指示送信後 */
	else{
		flp_DownLock_RetryStop( no );								// リトライ動作終了
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	下降ロックエラー発生時にCallされる処理	（ lk_err_chk()からCallされる ）								   |*/
/*| （フラップ装置で且つ下降ロックエラー登録の場合のみCallされる）											   |*/
/*|	※対象車室の情報は FLPCTL.flp_work では無いので注意。													   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: 車室No（0～323）																	   |*/
/*|               ErrNo	: ｴﾗｰ番号（下2桁）																	   |*/
/*|               knd	: 0:解除 1:発生																		   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_lk_err_chk( ushort no, char ErrNo, char kind )
{
	if( ErrNo == ERR_LOCKOPENFAIL ){								// 下降ロック（の時しかCallされないが）
		if( kind == 1 ){											// 発生
			if( flp_DownLock_DownSendEnableCheck2( no ) ){			// n分m回リトライ実行可能チェック
				flp_DownLock_RetryTimerStart( no );					// n分m回リトライタイマー開始
			}
		}
		else{														// 解除
			flp_DownLock_RetryStop( no );							// リトライ動作終了
			flp_DownLock_ErrSet( no, ERR_FLAPLOCK_DOWNRETRYOVER, 0 );	// リトライオーバーエラー解除
		}
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	車両入庫時・ラグタイムアップ時などでクリアされるエリアをまとめて本関数でクリア処理する					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: mode	遷移先（変更先）のmode																   |*/
/*| 			: type	1:CarSenserON	2:LagTimeOver 3:CarSensorOFF										   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: namioka																					   |*/
/*| Date		: 2009/08/26																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2009 AMANO Corp.------[]*/
void	flp_flag_clr( uchar	mode, uchar	type )
{
	
	flp_com *p = &FLPCTL.flp_work;
	
	if(( type & (CAR_SENSOR_ON | LAG_TIME_OVER)) ){
		p->mode = mode;									// 状態管理エリアの更新
		p->nstat.bits.b06 = 0;							// 不正ｸﾘｱ
		p->nstat.bits.b07 = 0;							// ﾒﾝﾃﾅﾝｽﾌﾗｸﾞOFF
		p->nstat.bits.b09 = 0;							// NT-NETﾌﾗｸﾞOFF
		p->uketuke = 0;									// 受付券発行済みｸﾘｱ
		p->passwd = 0;									// ﾊﾟｽﾜｰﾄﾞｸﾘｱ
		p->bk_syu = 0;									// 種別(中止,修正用)ｸﾘｱ
		p->lag_to_in.BYTE = OFF;						// ﾗｸﾞﾀｲﾏｰﾀｲﾑｱｳﾄによる再入庫ﾌﾗｸﾞOFF
		p->issue_cnt = 0;								// 駐車証明書発行回数初期化
		p->nstat.bits.b14 = 0;							// 遠隔からの操作指示種別クリア
		p->nstat.bits.b15 = 0;							// 遠隔からのフラップ操作要求クリア
	}
	switch( type ){
		case	CAR_SENSOR_ON:							// CarSensorONの場合
			memcpy( &p->year, &CLK_REC, 6 );			// 現在時刻をセット
			p->timer = LockTimer();						// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰｽﾀｰﾄ
			// 入庫ｶｳﾝﾄ設定がﾛｯｸ閉、ﾌﾗｯﾌﾟ上昇時の場合は入庫判定ﾀｲﾏｰを起動しない
			if( Carkind_Param(TYUU_DAISUU_MODE,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,4) != 2){
				p->in_chk_cnt = InChkTimer();			// 入庫判定ﾀｲﾏｰﾌﾗｸﾞON
				// 入庫判定時間チェック
				if( p->timer <= p->in_chk_cnt ){		// ロック閉時間が入庫判定時間より小さい時
					p->in_chk_cnt = (short)p->timer;	// ロック装置閉（ﾌﾗｯﾌﾟ上昇）時間に合わせる。
				}
			}else{
				p->in_chk_cnt = -1;						// 停止状態をセット
				/* 入庫情報登録 */
				IoLog_write(IOLOG_EVNT_ENT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			}
			break;
		case	CAR_SENSOR_OFF:
			p->mode = mode;								// 状態：下降済み（車両なし）へ
			p->timer = -1;								// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰｽﾄｯﾌﾟ
			p->in_chk_cnt = -1;							// 入庫判定ﾀｲﾏｰｽﾄｯﾌﾟ
			break;
		case	LAG_TIME_OVER:							// LagTimeUPの場合
			p->lag_to_in.BYTE = ON;
			FlpSet( FLPCTL.Room_no, 1 );				// Flaper UP
			break;
		default:
			break;
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	入出庫ｶｳﾝﾄ処理																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: mode	処理ﾌｪｰｽﾞ																			   |*/
/*| 			: evt	各動作ｲﾍﾞﾝﾄ																			   |*/
/*| RETURN VALUE: ret	0:入庫ﾃﾞｰﾀ送信しない 1:入庫ﾃﾞｰﾀ送信する												   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: namioka																					   |*/
/*| Date		: 2009/09/11																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2009 AMANO Corp.------[]*/
uchar	InOutCountFunc( uchar mode, ushort	evt )
{
	uchar ret = 0;
	uchar inparam;
	uchar outparam;
	uchar uc_prm;
	flp_com *p = &FLPCTL.flp_work;
	
	outparam = (uchar)Carkind_Param(TYUU_DAISUU_MODE,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,3);			// 出庫カウント
	inparam = (uchar)Carkind_Param(TYUU_DAISUU_MODE,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,4);			// 入庫カウント
	uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[FLPCTL.Room_no - 1].ryo_syu-1)*6)),1,1 );	// 種別毎ｶｳﾝﾄする設定get

	switch( mode ){
		case FLAP_CTRL_MODE1:	// 下降済み(車両なし)
			break;
		case FLAP_CTRL_MODE2:	// 下降済み(車両あり､FTｶｳﾝﾄ中)
			switch( evt ){
				case	2:		// Car sensor OFF
					if( inparam != 2 ){
						if(p->in_chk_cnt == -1){						// 入庫判定ﾀｲﾏｰﾀｲﾑｱｯﾌﾟ
							//出庫ｶｳﾝﾄ
							Lk_OutCount( uc_prm, 0 );					// 出庫台数ｶｳﾝﾄ処理
							//フラップ上昇、ロック閉ﾀｲﾏ内出庫情報登録
							if( inparam == 1 ){							// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後に送信する)
								ret = 1;
							}
							if( inparam != 2 ){							// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後にｶｳﾝﾄ以外)
								ope_LockTimeOutData( FLPCTL.Room_no, p );
							}
						}
					}
					flp_flag_clr( FLAP_CTRL_MODE1, CAR_SENSOR_OFF );	// ﾌﾗｸﾞｸﾘｱ
					break;
				case	21:		// Payment complete
					if( outparam == 1 ){								// 出庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(出庫時にｶｳﾝﾄ)
						if( inparam == 2 ){								// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後にｶｳﾝﾄ)
							Lk_InCount( uc_prm );
							ret = 1;
						}else{
							if(p->in_chk_cnt > 0){						// 入庫判定タイマー稼動中
								Lk_InCount( uc_prm );
								ret = 1;
							}else{
								if( inparam == 1 ){						// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後にｶｳﾝﾄする)
									ret = 1;
								}
							}
						}
					}else{												// 出庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(精算時にｶｳﾝﾄ)
						if(p->in_chk_cnt > 0){							// 入庫判定ﾀｲﾏｰ稼動中
							Lk_InCount( uc_prm );						// 入庫台数ｶｳﾝﾄ処理
							ret = 1;
						}
						else{
							if( inparam == 2 ){							// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後にｶｳﾝﾄする)
								Lk_InCount( uc_prm );					// 入庫台数ｶｳﾝﾄ処理
								ret = 1;
							}
							else if( inparam == 1 ){					// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(入庫判定時間経過後にｶｳﾝﾄ)
								ret = 1;
							}
						}
						Lk_OutCount( uc_prm, 0 );						// 出庫台数ｶｳﾝﾄ処理
					}
					break;
				case	43:		// ﾌﾗｯﾌﾟ上昇(ﾛｯｸ装置閉)ﾀｲﾏｰ
					if( inparam == 1 ){									// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(入庫判定時間経過後にｶｳﾝﾄ)
						ret = 1;										// 入庫ﾃﾞｰﾀ送信
					}
					if( inparam == 2 ){									// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後にｶｳﾝﾄする)
						Lk_InCount( uc_prm );							// 入庫台数ｶｳﾝﾄ処理
						ret = 1;										// 入庫ﾃﾞｰﾀ送信
					}
					break;
				case	44:		// 入庫判定ﾀｲﾏｰ
					if( inparam == 0 ){									// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(入庫判定時間経過後にｶｳﾝﾄ)
						ret = 1;
					}
					if( inparam != 2 ){									// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後にｶｳﾝﾄ以外)
						Lk_InCount( uc_prm );							// 入庫台数ｶｳﾝﾄ処理
					}
					break;
			}
			break;
		case FLAP_CTRL_MODE3:				// 上昇動作中
			switch( evt ){
				case	2:		// Car sensor OFF
					Lk_OutCount( uc_prm, 0 );							// 出庫台数ｶｳﾝﾄ処理
					break;
				case	50:		// 状態変化
					if( inparam == 1 ){									// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(入庫判定時間経過後にｶｳﾝﾄ)
						ret = 1;
					}
					if( inparam == 2 ){									// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後にｶｳﾝﾄ以外)
						Lk_InCount( uc_prm );							// 入庫台数ｶｳﾝﾄ処理
						ret = 1;
					}
					break;
				case	21:		// Payment complete
					if( outparam == 0 ){								// 出庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(精算完了時にｶｳﾝﾄ)
						Lk_OutCount( uc_prm, 0 );						// 出庫台数ｶｳﾝﾄ処理
					}
					break;
			}
			break;
		case FLAP_CTRL_MODE4:				// 上昇済み(駐車中)
			switch( evt ){
				case	2:		// Car sensor OFF
					Lk_OutCount( uc_prm, 0 );							// 出庫台数ｶｳﾝﾄ処理
					break;
				case	21:		// Payment complete
					if( outparam == 0 ){								// 出庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(精算完了時にｶｳﾝﾄ)
						Lk_OutCount( uc_prm, 0 );						// 出庫台数ｶｳﾝﾄ処理
					}
					break;
			}
			break;
		case FLAP_CTRL_MODE5:				// 下降動作中
		case FLAP_CTRL_MODE6:				// 下降済み(車両あり､LTｶｳﾝﾄ中)
			switch( evt ){
				case	2:		// Car sensor OFF
					if( outparam == 1 ){								// 出庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(出庫時にｶｳﾝﾄ)
						Lk_OutCount( uc_prm, 0 );						// 出庫台数ｶｳﾝﾄ処理
					}
					break;
				case	23:		// ﾌﾗｯﾌﾟ上昇指示(修正精算用)
					if( outparam == 0 ){								// 出庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(精算完了時にｶｳﾝﾄ)
						Lk_OutCount( uc_prm, 0 );						// 出庫台数ｶｳﾝﾄ処理
					}
					ret = 1;
					break;
				case 42:		// Lag time timer over
					if( outparam == 1 ){								// 出庫時にｶｳﾝﾄ(出庫していないためここでｶｳﾝﾄする)
						Lk_OutCount( uc_prm, 1 );						// 出庫台数ｶｳﾝﾄ処理
					}
					if( inparam != 2 ){									// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(ﾛｯｸ閉・ﾌﾗｯﾌﾟ上昇後にｶｳﾝﾄ以外)
						Lk_InCount( uc_prm );							// 入庫台数ｶｳﾝﾄ処理
						if( inparam == 0 ){								// 入庫ｶｳﾝﾄﾀｲﾐﾝｸﾞ(入庫判定時間経過後にｶｳﾝﾄ)
							ret = 1;
						}
					}
					break;
			}
			break;
		default:			// その他のﾌｪｰｽﾞ（ありえない）
			break;
	}
	return ret;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	英田製フラップ判定処理																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: index	：対象の車室情報インデックス 0～423													   |*/
/*| RETURN VALUE: ret	：0 英田製フラップ以外の装置  1 英田製フラップ										   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: namioka																					   |*/
/*| Date		: 2010/11/22																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2010 AMANO Corp.------[]*/
uchar	Aida_Flap_Chk( ushort index )
{
	uchar ret = 0;
	
	return ret;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	状態要求受信時の下降ロックリトライタイマー処理															   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	：対象の車室情報インデックス 0～423													   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: namioka																					   |*/
/*| Date		: 2010/11/22																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2010 AMANO Corp.------[]*/
void	flp_DownLockRetry_RecvSts( ushort no )
{
	
	if( FLPCTL.flp_work.nstat.bits.b00 == 1 &&								// 車両あり
		FLPCTL.flp_work.nstat.bits.b02 == 0 &&								// 最後に送信したのが、下降指示
		FLPCTL.flp_work.nstat.bits.b05 == 1 &&								// 下降ロック発生中
		DownLockFlag[ no - 1 ] == 1			&&								// 起動時から初回の状態要求
		Aida_Flap_Chk((ushort)(no - 1))){								 	// 装置種別が英田製フラップ
		FlpSet( no, 2 );													// Flaper DOWN
		DownLockFlag[ no - 1 ]++; 
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	エラーが発生しているフラップ板／ロック装置を検索する													   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no		:検索インデックス																   |*/
/*|				: errinfo	:車室番号格納アドレス															   |*/
/*| RETURN VALUE: ret   ：0x00=エラー車室なし	0x00<>エラー車室あり										   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: t.hashimoto																				   |*/
/*| Date		: 2013/02/27																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2013 AMANO Corp.------[]*/
uchar	flp_err_search( ushort sno, ulong* errinfo )
{
	uchar	ret_ecd;
	BITS	f_info;
	t_flp_DownLockErrInfo* pInfo;

	if( LockInfo[sno].lok_syu == 0 ){					// 接続あり?
		return( 0 );
	}

	ret_ecd = 0;
	// --- エラーチェック ---
	f_info = FLAPDT.flp_data[sno].nstat;
	if( f_info.word & 0x0030 ){							// エラー発生中かどうか
		if( f_info.bits.b04 ){							// E1638:上昇ロック発生中
			ret_ecd |= 0x01;
		}
		if( f_info.bits.b05 ){							// E1639:下昇ロック発生中
			ret_ecd |= 0x02;
		}
	}

	pInfo = &flp_DownLockErrInfo[sno];
	if( pInfo->f_info.BYTE & 0x03 ){
		if( pInfo->f_info.BIT.RETRY_OVER_ERR == 1 ){	// E1640:リトライオーバーエラー
			ret_ecd |= 0x04;
		}
		if( pInfo->f_info.BIT.RETRY_START_ERR == 1 ){	// E1641:リトライ動作開始エラー
			ret_ecd |= 0x08;
		}
	}
	if( ret_ecd ){
		*errinfo = flp_ErrBinDataEdit( sno );
	}

	return( ret_ecd );
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	車両なし時のフラップ下降処理																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no		:検索インデックス																   |*/
/*| RETURN VALUE: none																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Nagai																					   |*/
/*| Date		: 2015/01/20																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2015 AMANO Corp.------[]*/
static void flp_DownLock_ForCarSensorOff( ushort no )
{
	if( FLPCTL.flp_work.nstat.bits.b00 == 0 &&								// 車両なし
		FLPCTL.flp_work.nstat.bits.b01 == 1 &&								// 上昇済み
		DownLockFlag[ no - 1 ] == 1){										// 起動時から初回の状態要求
// MH322914 (s) kasiyama 2016/07/07 メンテナンスや遠隔からフラップ上昇を行うとフラップの上昇後に勝手にフラップが下降してしまう(共通改善No.1252)
//		FlpSet( no, 2 );													// Flaper DOWN
		// メンテナンス・遠隔の時は行わない
		if( FLPCTL.flp_work.nstat.bits.b07 != 1){
			FlpSet( no, 2 );													// Flaper DOWN
		}
// MH322914 (e) kasiyama 2016/07/07 メンテナンスや遠隔からフラップ上昇を行うとフラップの上昇後に勝手にフラップが下降してしまう(共通改善No.1252)
		DownLockFlag[ no - 1 ]++;
	}
}
