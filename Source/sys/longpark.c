// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
/*[]----------------------------------------------------------------------[]*/
/*| 長期駐車チェック機能                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2018-09-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"flp_def.h"
#include	"ntnet_def.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"AppServ.h"

void LongTermParkingCheck( void );
void LongTermParkingCheck_r10( void );
void LongTermParkingCheck_Resend( void );
void LongTermParkingCheck_Resend_flagset( void );
void LongTermParkingRel( ulong LockNo , uchar knd, flp_com *flp);

extern	void LongTermParkingCheck_r10_prmcng( void );
extern	void LongTermParkingCheck_r10_defset( void );
static short LongTermParking_stay_hour_check(flp_com *p, short hours);

#define ARMLOG_CHK_COUNT 50 // アラームログの連続登録のマスク件数
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車チェック(アラームデータによる検出)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck( void )
{
	ushort	wHour;
	ushort	i,j;
	ulong	ulwork;
	uchar	prm_wk;

	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// 長期駐車検出する
	if((prm_wk == 0) || (prm_wk == 2)){// 長期駐車検出(アラームデータによる通知)
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// 長期駐車
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;	// 設定が0の場合は強制的に48時間とする
		}
		// 検索前にアラームログがFROM書き込み中の場合一旦抜ける(一斉に発生した場合にログ登録のパンクを防ぐため)
		if (AppServ_IsLogFlashWriting(eLOG_ALARM) != 0){	// アラームログがFROM書き込み中？
			return;
		}
		if(prm_get( COM_PRM, S_PRN, 33, 1, 1 ) == 0 ){	// 18-0033⑥長期駐車アラームジャーナル印字 する
			if(NG == PriJrnExeCheck()){					// ジャーナルプリンタ印字中
				// 集計印字が実行されている場合は長時間印字できないケースがあるのでそのためのガード処理
				return;
			}
		}
		for( i = 0,j = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if ((LockInfo[i].lok_syu != 0) && (LockInfo[i].ryo_syu != 0) ) {// 車室有効
				if((FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4) ) {//「出庫処理中」または「空車」以外で有効
					ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// 区画情報取得
					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour ) != 0 ) {// 指定時間以上停車
						if(LongParkingFlag[i] == 0){					// 長期駐車状態：長期駐車状態なし
							LongParkingFlag[i] = 1;						// 長期駐車状態あり
							alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 1, 2, 1, &ulwork);	// A0031登録 長期駐車検出
							j++;// ループ内のアラーム登録件数カウントアップ
						}
					}else{// 指定時間未満
						if(LongParkingFlag[i] != 0){					// 長期駐車状態：長期駐車状態あり
							// 時刻変更により長期駐車状態「あり」→「なし」に変化する場合は解除する
							LongParkingFlag[i] = 0;						// 長期駐車状態なし
							alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);	// A0031解除 長期駐車検出
							j++;// ループ内のアラーム登録件数カウントアップ
						}
					}
				}
			}
			// 本関数は10秒ごとにコールされる。アラーム50件印字するのに約4.5秒のためその2倍の時間をしています
			if(prm_get( COM_PRM, S_PRN, 33, 1, 1 ) == 0 ){// 18-0033⑥長期駐車アラームジャーナル印字 する
				if(j >= ARMLOG_CHK_COUNT){// ループ内でのアラーム登録件数到達
					break;// ジャーナル印字バッファ登録あふれ抑制のため抜ける
				}
			}
			// アラームログがFROM書き込み中になったら一旦抜ける(一斉に発生した場合にログ登録のパンクを防ぐため)
			if (AppServ_IsLogFlashWriting(eLOG_ALARM) != 0){	// アラームログがFROM書き込み中？
				break;
			}
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車チェック(長期駐車データによる検出)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_r10( void )
{
	ushort	wHour,wHour2;
	ushort	i;
	uchar	prm_wk;

	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// 長期駐車検出する
	if(prm_wk == 2){	// 長期駐車検出(長期駐車データによる通知)
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// 長期駐車検出時間1
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;	// 設定が0の場合は強制的に48時間(2日)とする
		}
		wHour2 = (ushort)prm_get(COM_PRM,  S_TYP, 136, 4, 1);		// 長期駐車検出時間2

		// 検索前に長期駐車ログがFROM書き込み中の場合一旦抜ける(一斉に発生した場合にログ登録のパンクを防ぐため)
		if (AppServ_IsLogFlashWriting(eLOG_LONGPARK_PWEB) != 0){	// 長期駐車ログがFROM書き込み中？
			return;
		}
		for( i = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if ((LockInfo[i].lok_syu != 0) && (LockInfo[i].ryo_syu != 0) ) {// 車室有効
				if((FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4) ) {//「出庫処理中」または「空車」以外で有効

					// 長期駐車検出時間1のチェック
					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour ) != 0 ) {	// 指定時間以上停車
						if(FLAPDT.flp_data[i].flp_state.BIT.b00 == 0){									// 長期駐車状態：長期駐車状態なし
							Make_Log_LongParking_Pweb( i+1, wHour, LONGPARK_LOG_SET, LONGPARK_LOG_NON);	// 長期駐車1：発生 長期駐車データログ生成
							Log_regist( LOG_LONGPARK );	// 長期駐車データログ登録
							FLAPDT.flp_data[i].flp_state.BIT.b00 = 1;	// 長期駐車状態あり
						}
					}
					// 長期駐車データの場合、時刻変更による長期駐車状態「あり」→「なし」では解除しない

					if(wHour2 != 0){// 検出時間2の設定が有効な場合
						// 長期駐車検出時間2のチェック
						if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour2 ) != 0 ) {	// 指定時間以上停車
							if(FLAPDT.flp_data[i].flp_state.BIT.b01 == 0){									// 長期駐車状態：長期駐車状態なし
								Make_Log_LongParking_Pweb( i+1, wHour2, LONGPARK_LOG_SET, LONGPARK_LOG_NON);	// 長期駐車2：発生 長期駐車データログ生成
								Log_regist( LOG_LONGPARK );	// 長期駐車データログ登録
								FLAPDT.flp_data[i].flp_state.BIT.b01 = 1;	// 長期駐車状態あり
							}
						}
						// 長期駐車データの場合、時刻変更による長期駐車状態「あり」→「なし」では解除しない
					}
				}
			}
			
			// 長期駐車ログがFROM書き込み中になったら一旦抜ける(一斉に発生した場合にログ登録のパンクを防ぐため)
			if (AppServ_IsLogFlashWriting(eLOG_LONGPARK_PWEB) != 0){	// 長期駐車ログがFROM書き込み中？
				break;
			}
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車の設定が変更されたかの確認(長期駐車データによる検出)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
///	@note		長期駐車の検出時間が変更された場合、現状検出している長期駐車状態を一旦解除する
///				ParkingWebの履歴から削除するため。
///				解除後、LongTermParkingCheck_r10()関数にで再検出する。
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_r10_prmcng( void )
{
	ushort	wHour,wHour2;
	uchar	prm_wk;

	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// 長期駐車検出する
	if(prm_wk == 2){	// 長期駐車検出(長期駐車データによる通知)
		// 検索前に長期駐車ログがFROM書き込み中の場合一旦抜ける(一斉に発生した場合にログ登録のパンクを防ぐため)
		if (AppServ_IsLogFlashWriting(eLOG_LONGPARK_PWEB) != 0){	// 長期駐車ログがFROM書き込み中？
			return;
		}
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// 長期駐車検出時間1
		wHour2 = (ushort)prm_get(COM_PRM,  S_TYP, 136, 4, 1);		// 長期駐車検出時間2
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;	// 設定が0の場合は強制的に48時間(2日)とする
		}

		if( LongPark_Prmcng.time1 != wHour ){		// 検出時間1 設定変化あり
			LongPark_Prmcng.f_prm_cng = 1;			// 設定変更検出フラグセット
		}
		
		if( LongPark_Prmcng.time2 != wHour2 ){		// 検出時間2 設定変化あり
			if(LongPark_Prmcng.time2 == 0){
				// 検出時間2が未使用→使用となった場合は設定の変更を記憶するだけで長期駐車状態は解除しない。
				// この場合、設定追加による検出を行うだけで、現状をあえて解除する必要はないため
				LongPark_Prmcng.time2 = wHour2; // 検出時間2の設定状態更新
			}else{
				LongPark_Prmcng.f_prm_cng = 1;// 設定変更検出フラグセット
			}
		}
		
		if(LongPark_Prmcng.f_prm_cng == 0){
			LongPark_Prmcng.cng_count = 0;// 車室検索用のカウンタのクリア
			return;// 設定変更検出なし時は処理しない
		}
		// LongPark_Prmcng.cng_countがLOCK_MAXの時は更新が終わっているはずなので状態を更新する
		// 以下「停電ポイント①」で電源が落ちたときの対策
		if(LongPark_Prmcng.cng_count >= LOCK_MAX ){
			LongPark_Prmcng.time1 = wHour; // 検出時間1の設定状態更新
			LongPark_Prmcng.time2 = wHour2; // 検出時間2の設定状態更新
			LongPark_Prmcng.f_prm_cng = 0;//検索終了よりフラグクリア
			LongPark_Prmcng.cng_count = 0;// 車室検索用のカウンタのクリア
			return;
		}
		
		while( LongPark_Prmcng.cng_count < LOCK_MAX){
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if ((LockInfo[LongPark_Prmcng.cng_count].lok_syu != 0) && (LockInfo[LongPark_Prmcng.cng_count].ryo_syu != 0)) {// 車室有効
				// ParkingWebでは長期駐車データによる検出解除は入庫時間と車室情報のみで行っているため強制解除では
				// 必ず使用している検出時間1のデータで解除を登録する
				if((FLAPDT.flp_data[LongPark_Prmcng.cng_count].flp_state.BIT.b00 != 0)||	// 長期駐車1検出あり または
				   (FLAPDT.flp_data[LongPark_Prmcng.cng_count].flp_state.BIT.b01 != 0)){	// 長期駐車2検出あり
				   // 長期駐車1：解除 長期駐車データログ生成
					Make_Log_LongParking_Pweb( LongPark_Prmcng.cng_count+1, wHour, LONGPARK_LOG_RESET, LONGPARK_LOG_NON);
					Log_regist( LOG_LONGPARK );	// 長期駐車データログ登録
					FLAPDT.flp_data[LongPark_Prmcng.cng_count].flp_state.BIT.b00 = 0;	// 長期駐車1解除
					FLAPDT.flp_data[LongPark_Prmcng.cng_count].flp_state.BIT.b01 = 0;	// 長期駐車2解除
				}
			}
			LongPark_Prmcng.cng_count++;			// 車室検索用のカウンタの加算
			// 停電ポイント①
			if(LongPark_Prmcng.cng_count >= LOCK_MAX ){
				LongPark_Prmcng.time1 = wHour;		// 検出時間1の設定状態更新
				LongPark_Prmcng.time2 = wHour2;		// 検出時間2の設定状態更新
				LongPark_Prmcng.f_prm_cng = 0;		// 検索終了よりフラグクリア
				LongPark_Prmcng.cng_count = 0;		// 車室検索用のカウンタのクリア
				break;
			}
			
			// 長期駐車ログがFROM書き込み中になったら一旦抜ける(一斉に発生した場合にログ登録のパンクを防ぐため)
			if (AppServ_IsLogFlashWriting(eLOG_LONGPARK_PWEB) != 0){	// 長期駐車ログがFROM書き込み中？
				break;
			}
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車チェックの設定状態の更新処理(長期駐車データによる検出)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_r10_defset( void )
{
	ushort	wHour,wHour2;

	wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// 長期駐車検出時間1
	wHour2 = (ushort)prm_get(COM_PRM,  S_TYP, 136, 4, 1);		// 長期駐車検出時間2
	if(wHour == 0){
		wHour = LONG_PARK_TIME1_DEF;	// 設定が0の場合は強制的に48時間(2日)とする
	}
	LongPark_Prmcng.time1 = wHour;		// 検出時間1の設定状態更新
	LongPark_Prmcng.time2 = wHour2;		// 検出時間2の設定状態更新
}
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車時間チェック
//[]----------------------------------------------------------------------[]
///	@param[in]  flp_com *p   : 先頭番号
///	@param[in]  short   hours: 駐車時間 
///	@return     ret          : 0:指定時間未満 1:指定時間到達
///	@author     A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/05/16<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
short LongTermParking_stay_hour_check(flp_com *p, short hours)
{
	short	year, mon, day, hour, min;
	short	add_day, add_hour;

	year = p->year;		/* 入庫年 */
	mon = p->mont;		/* 入庫月 */
	day = p->date;		/* 入庫日 */
	hour = p->hour;		/* 入庫時 */
	min = p->minu;		/* 入庫分 */

	// 駐車時間(時)を日、時に換算する
	add_day = hours / 24;
	add_hour = hours % 24;

	// 入庫日時からadd_hour時間後の日時を取得する
	hour += add_hour;
	if(hour >= 24){
		day++;//1日加算
		hour = hour % 24;
		for ( ; ; ) {
			if (day <= medget(year, mon)) {// 月末以下
				break;
			}
			// 該当月の日数を引いて月を加算
			day -= medget(year, mon);
			if (++mon > 12) {
				mon = 1;
				year++;
			}
		}
	}
	// 入庫日時からadd_day日後の日時を取得する
	day += add_day;
	for ( ; ; ) {
		if (day <= medget(year, mon)) {// 月末以下
			break;
		}
		// 該当月の日数を引いて月を加算
		day -= medget(year, mon);
		if (++mon > 12) {
			mon = 1;
			year++;
		}
	}

	if(CLK_REC.year > year){
		return 1;	//現在年が長期駐車年を過ぎた(指定時間到達)
	}
	if(CLK_REC.year < year){
		return 0;	//現在年が長期駐車年より過去(指定時間未満)
	}
	// 以下 年は同じ
	if(CLK_REC.mont > mon){
		return 1;	//現在月が長期駐車月を過ぎた(指定時間到達)
	}
	if(CLK_REC.mont < mon){
		return 0;	//現在月が長期駐車月より過去(指定時間未満)
	}
	// 以下 月は同じ
	if(CLK_REC.date > day){
		return 1;	//現在日が長期駐車日を過ぎた(指定時間到達)
	}
	if(CLK_REC.date < day){
		return 0;	//現在日が長期駐車日より過去(指定時間未満)
	}
	// 以下 日は同じ
	if(CLK_REC.hour > hour){
		return 1;	//現在時が長期駐車時を過ぎた(指定時間到達)
	}
	if(CLK_REC.hour < hour){
		return 0;	//現在時が長期駐車時より過去(指定時間未満)
	}
	// 以下 時は同じ
	if(CLK_REC.minu > min){
		return 1;	//現在分が長期駐車分を過ぎた(指定時間到達)
	}
	if(CLK_REC.minu < min){
		return 0;	//現在分が長期駐車分より過去(指定時間未満)
	}
	return 1;// 年月日時分一致(指定時間到達)
}
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車チェック(ドア閉時にセンターに対して送信する処理)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_Resend( void )
{
	ushort	wHour;
	ulong	ulwork;
	uchar	prm_wk;

	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// 長期駐車検出する
	if((prm_wk == 0) || (prm_wk == 2)){
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// 長期駐車
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;// 設定が0の場合は強制的に48時間とする
		}

		if( isDefToErrAlmTbl(1, ALMMDL_MAIN, ALARM_LONG_PARKING) == FALSE ){
			// アラームデータの送信レベルの設定（34-0037③）で送信するレベルでない場合はチェックしない
			f_LongParkCheck_resend = 0;//フラグクリア
			LongParkCheck_resend_count = 0;
			return;
		}
		// ドア閉時に長期駐車チェックを行い検出中であればセンターに対して再送するためにログに登録する

		if(f_LongParkCheck_resend == 0){// 要求なし
			return;
		}
		// アラームログがFROM書き込み中になったら一旦抜ける
		if (AppServ_IsLogFlashWriting(eLOG_ALARM) != 0){	// アラームログがFROM書き込み中？
			return;
		}
		while( LongParkCheck_resend_count < LOCK_MAX ){
			WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
			if ((LockInfo[LongParkCheck_resend_count].lok_syu != 0) && (LockInfo[LongParkCheck_resend_count].ryo_syu != 0) ) {// 車室有効
				//「出庫処理中」または「空車」以外で有効
				if((FLAPDT.flp_data[LongParkCheck_resend_count].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[LongParkCheck_resend_count].mode <= FLAP_CTRL_MODE4) ) {
					ulwork = (ulong)(( LockInfo[LongParkCheck_resend_count].area * 10000L ) + LockInfo[LongParkCheck_resend_count].posi );	// 区画情報取得
					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[LongParkCheck_resend_count], (short)wHour ) != 0 ) {// 指定時間以上停車
						// A0031登録 長期駐車検出
						memcpy( &Arm_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// 発生日時
						Arm_work.Armsyu = ALMMDL_MAIN;										// ｱﾗｰﾑ種別
						Arm_work.Armcod = ALARM_LONG_PARKING;								// ｱﾗｰﾑｺｰﾄﾞ
						Arm_work.Armdtc = 1;												// ｱﾗｰﾑ発生/解除
											
						Arm_work.Armlev = (uchar)getAlmLevel( ALMMDL_MAIN, ALARM_LONG_PARKING );// ｱﾗｰﾑﾚﾍﾞﾙ
						Arm_work.ArmDoor = ERR_LOG_RESEND_F;								// 再送状態とする
						Arm_work.Arminf = 2;												// 付属ﾃﾞｰﾀ(bin)あり
						Arm_work.ArmBinDat = ulwork;										// bin アラーム情報をｾｯﾄ

						Log_regist( LOG_ALARM );											// ｱﾗｰﾑﾛｸﾞ登録
					}
				}
			}
			LongParkCheck_resend_count++;
			if(LongParkCheck_resend_count >= LOCK_MAX ){
				LongParkCheck_resend_count = 0;
				f_LongParkCheck_resend = 0;//検索終了よりフラグクリア
				break;
			}
			// アラームログがFROM書き込み中になったら一旦抜ける(一斉に発生した場合にログ登録のパンクを防ぐため)
			if (AppServ_IsLogFlashWriting(eLOG_ALARM) != 0){	// アラームログがFROM書き込み中？
				break;
			}
		}
	}

}
//[]----------------------------------------------------------------------[]
///	@brief		長期駐車チェックフラグセット処理(ドア閉時にセンターに対して送信する処理)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
///	@note		ドア閉によりセンターへ送信するアラームをセットさせるフラグの判定処理
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_Resend_flagset( void )
{
	uchar	prm_wk;
	
	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// 長期駐車検出する
	if((prm_wk == 0) || (prm_wk == 2)){
		if(f_LongParkCheck_resend == 0){
			// フラグを一回受け付けた後は、長期駐車アラームデータの送信処理を終えるまで受け付けないようにする
			f_LongParkCheck_resend = 1;
			LongParkCheck_resend_count = 0;
		}
	}else{
		f_LongParkCheck_resend = 0;
		LongParkCheck_resend_count = 0;
	}
}
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
