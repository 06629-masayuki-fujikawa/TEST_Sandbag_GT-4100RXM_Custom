//[]----------------------------------------------------------------------[]
///	@file		update_ope.c
///	@brief		運用面管理処理
/// @date		2012/04/24
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

/*--- Include -------------------*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"updateope.h"

/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void update_sw_area_init(void);
void get_from_swdata(void);
unsigned char chk_wave_data_onfrom(unsigned short, unsigned char *);
void wave_data_swupdate(void);
void parm_data_swupdate(void);
unsigned char wave_data_swchk(void);
unsigned char parm_data_swchk(void);
void update_flag_set(unsigned char);
void update_flag_clear(unsigned char);
unsigned char chk_prog_data_onfrom(unsigned char *dat);

/*--- Define --------------------*/

/*--- Gloval Value --------------*/

/*--- Extern --------------------*/


//[]----------------------------------------------------------------------[]
///	@brief			運用面情報初期化処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		RSW=4(スーパーイニシャライズの時のみコールする)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void update_sw_area_init(void)
{
	// 更新フラグの初期化(プログラム更新フラグ,音声データ更新フラグ,共通パラメータ更新フラグ)
	update_flag_clear(PROGRAM_UPDATE_FLAG | WAVEDATA_UPDATE_FLAG | PARAM_UPDATE_FLAG);
	// 運用面情報バックアップ(更新中のリカバリ用)
	BootInfo.sw_bakup.wave = 0;
	BootInfo.sw_bakup.parm = 0;
	// フラッシュに書かれた運用面情報
	BootInfo.sw_flash.wave = 0;
	BootInfo.sw_flash.parm = 0;
	// ブートプログラム側でのみ使用するエラーリトライカウンタであるが、念のため初期化する
	BootInfo.err_count = 0;// ブートプログラムで発生したエラーの回数

	// 運用面情報
	// 共通パラメータの運用面を強制的に面1にする
	BootInfo.sw.parm = OPERATE_AREA1;
	FLT_write_parm_swdata(OPERATE_AREA1);								// FROMの運用面も面1とする
	// NOTE:音声データの場合、インストールした時点で書き込むのでここでは何もしない
	// プログラムは内部FROMで動作するため、外部FROMにダウンロードプログラムを内部FROMに対して
	// 更新をかける機能のみ搭載する仕様
}

//[]----------------------------------------------------------------------[]
///	@brief			FROMの運用面情報取得処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void get_from_swdata(void)
{
	BootInfo.sw_flash.wave = FLT_get_wave_swdata();
	BootInfo.sw_flash.parm = FLT_get_parm_swdata();
}

//[]----------------------------------------------------------------------[]
///	@brief			音声データが指定した運用面に格納されているかチェックする
//[]----------------------------------------------------------------------[]
///	@param[in]		sw :運用面情報 OPERATE_AREA1(面1)またはOPERATE_AREA2(面2)
///	@param[in]		*dat :バージョン情報を格納して欲しいポインタ(バージョンデータは8Byte)
///					      NULLを指定した場合とチェック結果がデータ無しの場合は格納しない
///	@return			0:データ有 1:データ無し
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char chk_wave_data_onfrom(unsigned short sw, unsigned char *dat)
{
	int i,j;
	int f_ok = 1;
	
	if(OPERATE_AREA1 == sw){						// 運用面1
		FLT_read_wave_sum_version(0,swdata_write_buf);
	} else {										// 運用面2
		FLT_read_wave_sum_version(1,swdata_write_buf);
	}
	// レングス領域の確認
	for(i = LENGTH_OFFSET; i<(LENGTH_OFFSET+LENGTH_SIZE);i++){
		if(swdata_write_buf[i] != 0xFF){
			f_ok = 0;//データ有
			break;
		}
	}
	if(f_ok == 1){
		return 1;									// ブランクなのでデータなし
	}
	// チェックサム領域の確認
	if((0xFF == swdata_write_buf[SUM_OFFSET])&&(0xFF == swdata_write_buf[SUM_OFFSET+1])) {
		return 1;									// ブランクなのでデータなし
	}
	// バージョンがASCIIの文字コード範囲外なら有効データなし
	for(i = VER_OFFSET; i<(VER_OFFSET+VER_SIZE);i++){
		if((swdata_write_buf[i] < 0x20 )||(swdata_write_buf[i] > 0x7A)) {
			return 1;// ASCIIのスペースから'z'の範囲外(文字コードが対象外)
		}
	}
	// ここに来た時点でデータ正常
	if(dat != NULL){
		for(i = VER_OFFSET,j = 0; i<(VER_OFFSET+VER_SIZE); i++, j++){
			dat[j] = swdata_write_buf[i];
		}
	}
	return 0;// データ有
}
//[]----------------------------------------------------------------------[]
///	@brief			音声データ運用面更新処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void wave_data_swupdate(void)
{
// MH810100(S) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
//	if((BootInfo.sw.wave != OPERATE_AREA1)&&(BootInfo.sw.wave != OPERATE_AREA2)) {
//		memset(BootInfo.f_wave_update,0x00,sizeof(BootInfo.f_wave_update));// 念のため更新フラグクリア
//		return;// 更新処理は内部RAMの面情報が正常であることが前提、異常時は更新しない
//	}
//	
//	if(0 == memcmp(BootInfo.f_wave_update,wave_update_flag,sizeof(BootInfo.f_wave_update))) {// 更新フラグ有
//		if((BootInfo.sw_bakup.wave != OPERATE_AREA1)&&(BootInfo.sw_bakup.wave != OPERATE_AREA2)) {
//			// 運用面情報バックアップがない場合
//			BootInfo.sw_bakup.wave = BootInfo.sw.wave;// FROM更新途中で電源断した時を考慮して運用面情報をバックアップ
//		}
//		if((BootInfo.sw_flash.wave != OPERATE_AREA1)&&(BootInfo.sw_flash.wave != OPERATE_AREA2)) {
//			// 外部FROMに運用面情報が無い(FROM更新中の電源断のケース)
//			if(BootInfo.sw.wave == OPERATE_AREA1) {		// 現在の運用面が1の場合
//				FLT_write_wave_swdata(OPERATE_AREA2);
//				BootInfo.sw_flash.wave = OPERATE_AREA2;
//			} else {									// 現在の運用面が2の場合
//				FLT_write_wave_swdata(OPERATE_AREA1);
//				BootInfo.sw_flash.wave = OPERATE_AREA1;
//			}
//		} else {
//			// 外部FROMにデータ有
//			if(BootInfo.sw_bakup.wave == BootInfo.sw_flash.wave) {
//				// FROMと運用面情報バックアップが一致した時は先にFROMの更新を行なう
//				if(BootInfo.sw.wave == OPERATE_AREA1) {		// 現在の運用面が1の場合
//					FLT_write_wave_swdata(OPERATE_AREA2);
//					BootInfo.sw_flash.wave = OPERATE_AREA2;
//				} else {									// 現在の運用面が2の場合
//					FLT_write_wave_swdata(OPERATE_AREA1);
//					BootInfo.sw_flash.wave = OPERATE_AREA1;
//				}
//			}
//		}
//
//		if(BootInfo.sw.wave == BootInfo.sw_bakup.wave) {// 現在の運用面バックアップの運用面一致
//			if(BootInfo.sw.wave == OPERATE_AREA1) {		// 現在の運用面が1の場合
//				BootInfo.sw.wave = OPERATE_AREA2;
//			} else {									// 現在の運用面が2の場合
//				BootInfo.sw.wave = OPERATE_AREA1;
//			}
//		}
//	}
// MH810100(E) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
	memset(BootInfo.f_wave_update,0x00,sizeof(BootInfo.f_wave_update));// 更新フラグクリア
}


//[]----------------------------------------------------------------------[]
///	@brief			共通パラメータ運用面更新処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void parm_data_swupdate(void)
{
	if((BootInfo.sw.parm != OPERATE_AREA1)&&(BootInfo.sw.parm != OPERATE_AREA2)) {
		memset(BootInfo.f_parm_update,0x00,sizeof(BootInfo.f_parm_update));// 念のため更新フラグクリア
		return;// 更新処理は内部RAMの面情報が正常であることが前提、異常時は更新しない
	}
	
	if(0 == memcmp(BootInfo.f_parm_update,parm_update_flag,sizeof(BootInfo.f_parm_update))) {// 更新フラグ有
		if((BootInfo.sw_bakup.parm != OPERATE_AREA1)&&(BootInfo.sw_bakup.parm != OPERATE_AREA2)) {
			// 運用面情報バックアップがない場合
			BootInfo.sw_bakup.parm = BootInfo.sw.parm;// FROM更新途中で電源断した時を考慮して運用面情報をバックアップ
		}
		if((BootInfo.sw_flash.parm != OPERATE_AREA1)&&(BootInfo.sw_flash.parm != OPERATE_AREA2)) {
			// 外部FROMに運用面情報が無い(FROM更新中の電源断のケース)
			if(BootInfo.sw.parm == OPERATE_AREA1) {		// 現在の運用面が1の場合
				FLT_write_parm_swdata(OPERATE_AREA2);
			} else {									// 現在の運用面が2の場合
				FLT_write_parm_swdata(OPERATE_AREA1);
			}
		} else {
			// 外部FROMにデータ有
			if(BootInfo.sw_bakup.parm == BootInfo.sw_flash.parm) {
				// FROMと運用面情報バックアップが一致した時は先にFROMの更新を行なう
				if(BootInfo.sw.parm == OPERATE_AREA1) {		// 現在の運用面が1の場合
					FLT_write_parm_swdata(OPERATE_AREA2);
				} else {									// 現在の運用面が2の場合
					FLT_write_parm_swdata(OPERATE_AREA1);
				}
			}
		}

		if(BootInfo.sw.parm == BootInfo.sw_bakup.parm) {// 現在の運用面バックアップの運用面一致
			if(BootInfo.sw.parm == OPERATE_AREA1) {		// 現在の運用面が1の場合
				BootInfo.sw.parm = OPERATE_AREA2;
			} else {									// 現在の運用面が2の場合
				BootInfo.sw.parm = OPERATE_AREA1;
			}
		}
		_flt_DestroyParamRamSum();						// RAM上のパラメータサムをクリア
// GG120600(S) // Phase9 設定変更通知対応
		if(!remotedl_work_update_get()){
			// 手動で面切替の場合のみ
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
		}
// GG120600(E) // Phase9 設定変更通知対応				
	}
	memset(BootInfo.f_parm_update,0x00,sizeof(BootInfo.f_parm_update));// 更新フラグクリア
}


//[]----------------------------------------------------------------------[]
///	@brief			音声データ運用面チェック処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return         0:正常 
///                 1:FROMが正常なのでFROMに更新して復旧
///                 2:運用面情報異常(面1にデータ有より運用面1に更新して復旧)
///                 3:運用面情報異常(面2にデータ有より運用面2に更新して復旧)
///                 4:データの実体が存在しない(インストールされていない)
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char wave_data_swchk(void)
{
// MH810100(S) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
//	unsigned char ret;
//
//	if(( 0 != chk_wave_data_onfrom(OPERATE_AREA1,NULL)) && ( 0 != chk_wave_data_onfrom(OPERATE_AREA2,NULL)) ) {
//		// データの実体が存在しない(初回インストールもしていない)
//		ret = 4;
//	} else if((BootInfo.sw.wave == OPERATE_AREA1)||(BootInfo.sw.wave == OPERATE_AREA2)) {// SRAM運用面正常
//		ret = 0;
//	} else if((BootInfo.sw_flash.wave == OPERATE_AREA1)||(BootInfo.sw_flash.wave == OPERATE_AREA2)) {
//		// FROM運用面が正常なため更新
//		BootInfo.sw.wave = BootInfo.sw_flash.wave;
//		ret = 1;
//	} else if( 0 == chk_wave_data_onfrom(OPERATE_AREA1,NULL)) {
//		// 運用面1に実際にデータ有
//		FLT_write_wave_swdata(OPERATE_AREA1);
//		BootInfo.sw_flash.wave = OPERATE_AREA1;
//		BootInfo.sw.wave = OPERATE_AREA1;
//		ret = 2;
//	} else if( 0 == chk_wave_data_onfrom(OPERATE_AREA2,NULL)) {
//		// 運用面2に実際にデータ有
//		FLT_write_wave_swdata(OPERATE_AREA2);
//		BootInfo.sw_flash.wave = OPERATE_AREA2;
//		BootInfo.sw.wave = OPERATE_AREA2;
//		ret = 3;
//	} else {// データの実体が存在しない
//		ret = 4;
//	}
//	if( BootInfo.sw.wave == OPERATE_AREA1 ){
//		SodiacSoundAddress = FROM1_SA27;
//	}else{
//		SodiacSoundAddress = FROM1_SA28;
//	}
//	memset( SOUND_VERSION, 0, sizeof( SOUND_VERSION ));
//	chk_wave_data_onfrom( BootInfo.sw.wave, SOUND_VERSION );
//	return ret;
	return 4;
// MH810100(E) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
}

//[]----------------------------------------------------------------------[]
///	@brief			共通パラメータ運用面チェック処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@return         0:正常 
///                 1:FROMが正常なのでFROMに更新して復旧
///                 2:運用面情報異常より運用面1に更新して復旧
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char parm_data_swchk(void)
{
	unsigned char ret;

	if((BootInfo.sw.parm == OPERATE_AREA1)||(BootInfo.sw.parm == OPERATE_AREA2)) {
		// SRAM運用面正常
		ret = 0;
	} else if((BootInfo.sw_flash.parm == OPERATE_AREA1)||(BootInfo.sw_flash.parm == OPERATE_AREA2)) {
		// SRAM異常であるがFROM運用面が正常なため更新
		BootInfo.sw.parm = BootInfo.sw_flash.parm;
		ret = 1;
	} else {
		// 運用面が異常な場合は必ずデータが存在する運用面1に強制セット
		FLT_write_parm_swdata(OPERATE_AREA1);
		BootInfo.sw_flash.parm = OPERATE_AREA1;
		BootInfo.sw.parm = OPERATE_AREA1;
		ret = 2;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			データ更新フラグ書き込み処理
//[]----------------------------------------------------------------------[]
///	@param[in]		update	: 0ビット目ON更新 プログラム 1ビット目ON 音声データ更新 2ビット目ON 共通パラメータ更新
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void update_flag_set(unsigned char update)
{
	if(PROGRAM_UPDATE_FLAG & update) {							// プログラム更新フラグ要求有
		memcpy(BootInfo.f_prog_update,prog_update_flag,sizeof(BootInfo.f_prog_update));
	}

	if(WAVEDATA_UPDATE_FLAG & update) {							// 音声データ更新フラグ要求有
		BootInfo.sw_bakup.wave = 0;	// 運用面情報バックアップ(更新中のリカバリ用)
		memcpy(BootInfo.f_wave_update,wave_update_flag,sizeof(BootInfo.f_wave_update));
	}

	if(PARAM_UPDATE_FLAG & update) {							// 共通パラメータ更新フラグ要求有
		BootInfo.sw_bakup.parm = 0;	// 運用面情報バックアップ(更新中のリカバリ用)
		memcpy(BootInfo.f_parm_update,parm_update_flag,sizeof(BootInfo.f_parm_update));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			データ更新フラグクリア処理
//[]----------------------------------------------------------------------[]
///	@param[in]		update	: 0ビット目ON更新 プログラム 1ビット目ON 音声データ更新 2ビット目ON 共通パラメータ更新
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void update_flag_clear(unsigned char update)
{
	if(PROGRAM_UPDATE_FLAG & update) {							// プログラム更新フラグ要求有
		memset(BootInfo.f_prog_update,0x00,sizeof(BootInfo.f_prog_update));
	}

	if(WAVEDATA_UPDATE_FLAG & update) {							// 音声データ更新フラグ要求有
		memset(BootInfo.f_wave_update,0x00,sizeof(BootInfo.f_wave_update));
	}

	if(PARAM_UPDATE_FLAG & update) {							// 共通パラメータ更新フラグ要求有
		memset(BootInfo.f_parm_update,0x00,sizeof(BootInfo.f_parm_update));
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			ダウンロードしたプログラムデータが外部FROMに格納されているかチェックする
//[]----------------------------------------------------------------------[]
///	@param[in]		*dat バージョン情報を格納して欲しいポインタ(バージョンデータは8Byte)
///					     NULLを指定した場合とチェック結果がデータ無しの場合は格納しない
///	@return			0:データ有 1:データ無し
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char chk_prog_data_onfrom(unsigned char *dat)
{
	int i,j;
	int f_ok = 1;
	FLT_read_program_version(swdata_write_buf);
	// レングス領域の確認
	for(i = LENGTH_OFFSET; i<(LENGTH_OFFSET+LENGTH_SIZE);i++){
		if(swdata_write_buf[i] != 0xFF){
			f_ok = 0;//データ有
			break;
		}
	}
	if(f_ok == 1){
		return 1;									// ブランクなのでデータなし
	}
	// チェックサム領域の確認
	if((0xFF == swdata_write_buf[SUM_OFFSET])&&(0xFF == swdata_write_buf[SUM_OFFSET+1])) {
		return 1;									// ブランクなのでデータなし
	}
	// バージョンがASCIIの文字コード範囲外なら有効データなし
	for(i = VER_OFFSET; i<(VER_OFFSET+VER_SIZE);i++){
		if((swdata_write_buf[i] < 0x20 )||(swdata_write_buf[i] > 0x7A)) {
			return 1;// ASCIIのスペースから'z'の範囲外(文字コードが対象外)
		}
	}
	// ここに来た時点でデータ正常
	if(dat != NULL){
		for(i = VER_OFFSET,j = 0; i<(VER_OFFSET+VER_SIZE); i++, j++){
			dat[j] = swdata_write_buf[i];
		}
	}
	return 0;// データ有
}
