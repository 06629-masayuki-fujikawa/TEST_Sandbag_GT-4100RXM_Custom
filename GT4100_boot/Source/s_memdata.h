#ifndef _S_MEMDATA_
#define _S_MEMDATA_
//[]----------------------------------------------------------------------[]
///	@file		s_memdata.h
///	@brief		ブート領域メモリ定義 ヘッダファイル
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
typedef struct{
	unsigned short wave;// 音声データ
	unsigned short parm;// 共通パラメータ
	unsigned short reserve1;
	unsigned short reserve2;
}SW;

// 面切り替えセクション情報
typedef struct{
	unsigned char f_prog_update[4];			// プログラム更新フラグ
	unsigned char f_wave_update[4];			// 音声データ更新フラグ
	unsigned char f_parm_update[4];			// 共通パラメータ更新フラグ
	SW sw;									// 運用面情報
	SW sw_bakup;							// 運用面情報バックアップ(更新中のリカバリ用)
	SW sw_flash;							// フラッシュに書かれた運用面情報
	unsigned short err_count;				// ブートプログラムで発生したエラーの回数(トータル10発生したら復旧不可能とする)
}SWITCH_DATA;

/*--- Define --------------------*/

/*--- Extern --------------------*/

extern const unsigned char prog_update_flag[4];
extern SWITCH_DATA	BootInfo;						// 起動時面選択情報

#endif

