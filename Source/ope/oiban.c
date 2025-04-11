//[]----------------------------------------------------------------------[]
///	@file		oiban.c
///	@brief		'oiban' management
/// @date		2008/09/22
///	@author		MATSUSHITA
///	@version	GT-7000_36
/// @note		通し追い番、追い番範囲対応
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"mem_def.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"prm_tbl.h"

#ifdef	SYSTEM_TYPE_IS_NTNET
#define	_OIBAN_MAX	999999L
#else
#define	_OIBAN_MAX	99999L
#endif

#pragma	section	_UNINIT1
// 通し追い番と個別追い番をペアで管理する
// 通し追い番は”貸し出し”扱いとし、更新するか、”返却”が必要
// 同時に複数の追い番を貸し出すことはできない（預り証追い番のみ例外）
// 追い番更新はここに保存したものを＋１して個別パラメータに書き戻す
static	ST_OIBAN	oiban_work[_OBN_AREA_MAX];
#pragma	section

static	const	int		kind_no[][2] = {
		{PAYMENT_COUNT,		_OBN_PAYMENT}	// 精算追番
// GM849100(S) 名鉄協商コールセンター対応（精算／精算中止の追番を１つにする）（GT-7700：GM747902参考）
//	,	{CANCEL_COUNT,		_OBN_CANCEL}	// 精算中止追番
// GM849100(E) 名鉄協商コールセンター対応（精算／精算中止の追番を１つにする）（GT-7700：GM747902参考）
	,	{DEPOSIT_COUNT,		_OBN_DEPOSIT}	// 預り証追番
	,	{TURIKAN_COUNT,		_OBN_TURIKAN}	// 釣銭管理合計追い番
	,	{COIN_SAFE_COUNT,	_OBN_COIN_SAFE}	// ｺｲﾝ金庫追番
	,	{NOTE_SAFE_COUNT,	_OBN_NOTE_SAFE}	// 紙幣金庫追番
	,	{KIYOUSEI_COUNT,	_OBN_KIYOUSEI}	// 強制出庫追番
	,	{FUSEI_COUNT,		_OBN_FUSEI}		// 不正出庫追番
	,	{T_TOTAL_COUNT,		_OBN_T_TOTAL}	// T合計追番
	,	{GT_TOTAL_COUNT,	_OBN_GT_TOTAL}	// GT合計追番
	,	{-1, -1}
};

// prototype
static	int		search_table(int data, int type);
#define	no_search(kind)	search_table((int)kind,0)
#define	kind_search(no)	search_table(no,1)

//[]----------------------------------------------------------------------[]
///	@brief			追い番種別相互変換
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: 元データ
///	@param[in]		type	: 0:kind->no/1:no->kind
///	@return			変換結果<br>
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static	int		search_table(int data, int type)
{
	int		i;
	i = 0;
	while(kind_no[i][type] >= 0) {
		if (kind_no[i][type] == data) {
			return (int)kind_no[i][type^1];
		}
		i++;
	}
	return -1;
}

//[]----------------------------------------------------------------------[]
///	@brief			追番初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		clr		: RAM初期化
///	@return			void<br>
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	CountInit(uchar clr)
{
	if (clr) {
		memset(oiban_work, 0, sizeof(oiban_work));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			追番発行状況
//[]----------------------------------------------------------------------[]
///	@return			1 = OK, 0 = NG
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/22<br>
///					Update	:	
///[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		CountCheck(void)
{
	int		no;
	for (no = _OBN_PAYMENT; no < _OBN_AREA_MAX; no++) {
		if (oiban_work[no].w != 0){
			return 0;		// 貸し出し中
		}
	}
	return 1;		// 貸し出し可
}

//[]----------------------------------------------------------------------[]
///	@brief			追番発行
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 追番の種類<br>
///	@param[out]		oiban	: 追番保存先<br>
///	@return			1 = OK, 0 = NG
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/22<br>
///					Update	:	
///[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		CountGet( uchar kind, ST_OIBAN *oiban )
{
	int		no, i;

	oiban->w = 0;
	oiban->i = 0;
	if (kind == GT_TOTAL_COUNT) {
		if (prm_get(COM_PRM, S_TOT, 1, 1, 1) == 0) {
		// GT追い番なし
			return 1;
		}
	}
	no = no_search(kind);
	if (no < 0) {
		return 0;
	}
	if (oiban_work[no].w == 0) {
	// 通し追い番未発行
		for (i = _OBN_PAYMENT; i < _OBN_AREA_MAX; i++) {
			if (oiban_work[i].w != 0){
				break;
			}
		}
		if (i < _OBN_AREA_MAX) {
		// 既に貸し出し中
			if (no != _OBN_DEPOSIT ||
				(i != _OBN_PAYMENT && i != _OBN_CANCEL)) {
				return 0;	// 通し追い番使用中
			}
			oiban_work[no].i = PPrmSS[S_P03][kind];
			oiban_work[no].w = (PPrmSS[S_P03][ALL_COUNT] >= _OIBAN_MAX)?
									1 : (PPrmSS[S_P03][ALL_COUNT]+1);
		}
		else {
		// 貸し出しなし
			oiban_work[no].i = PPrmSS[S_P03][kind];
			oiban_work[no].w = PPrmSS[S_P03][ALL_COUNT];
		}
	}
	*oiban = oiban_work[no];
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			追番ｶｳﾝﾄ+1
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 追番の種類<br>
///	@return			void<br>
///	@author			R.Hara
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/02/01<br>
///					Update	:	2008/09/22(MATSUSHITA)
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	CountUp( uchar kind )
{
	int		no;
	SYUKEI	*wsy;
	no = no_search(kind);
	if (no >= 0) {
		if (oiban_work[no].w != 0) {
		// 通し追い番貸し出し中
			if (kind == T_TOTAL_COUNT){
				wsy = &sky.gsyuk;
			}
			else if (kind == GT_TOTAL_COUNT){
				wsy = &sky.msyuk;
			}
			else{
				wsy = &sky.tsyuk;
			}
//			wptoff();			// 追い番範囲保存開始
			if (wsy->oiban_range[_OBN_WHOLE][0] == 0) {
				wsy->oiban_range[_OBN_WHOLE][0] = oiban_work[no].w;
			}
			wsy->oiban_range[_OBN_WHOLE][1] = oiban_work[no].w;
			if (wsy->oiban_range[no][0] == 0) {
				wsy->oiban_range[no][0] = oiban_work[no].i;
			}
			wsy->oiban_range[no][1] = oiban_work[no].i;
//			wpton();			// 追い番範囲保存終了
			if (oiban_work[no].w >= _OIBAN_MAX){
				PPrmSS[S_P03][ALL_COUNT] = 1;
			}
			else{
				PPrmSS[S_P03][ALL_COUNT] = oiban_work[no].w + 1;
			}
			if (oiban_work[no].i >= _OIBAN_MAX){
				PPrmSS[S_P03][kind] = 1;
			}
			else{
				PPrmSS[S_P03][kind] = oiban_work[no].i + 1;
			}
			oiban_work[no].w = 0;					// 通し追い番破棄
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			追番返却
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 追番の種類<br>
///	@return			void<br>
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/22<br>
///					Update	:	
///[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	CountFree( uchar kind )
{
	int		no;
	if (kind == ALL_COUNT) {
		for (no = _OBN_PAYMENT; no < _OBN_AREA_MAX; no++) {
			oiban_work[no].w = 0;					// 通し追い番破棄
		}
	}
	else {
		no = no_search(kind);
		if (no >= 0) {
			oiban_work[no].w = 0;					// 通し追い番破棄
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			追番ｶｳﾝﾄｸﾘｱ
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	追番の種類<br>
///	@return			void<br>
///	@author			R.Hara
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/02/01<br>
///					Update	:	2008/09/22(MATSUSHITA)
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	CountClear( uchar kind )
{
	int		i;

	switch( kind ){
	case CLEAR_COUNT_ALL:										// 追番ｸﾘｱ(全て)
// GM849100(S) 名鉄協商コールセンター対応（T合計時に精算追番をクリアしない）（GT-7700：GM747902参考）
// CLEAR_COUNT_ALLは使用されていない
// GM849100(E) 名鉄協商コールセンター対応（T合計時に精算追番をクリアしない）（GT-7700：GM747902参考）
		for (i = 1; i <= COUNT_MAX; i++) {
			PPrmSS[S_P03][i] = 1L;
		}
		break;

	case CLEAR_COUNT_T:											// 追番ｸﾘｱ(T合計)
// GM849100(S) 名鉄協商コールセンター対応（T合計時に精算追番をクリアしない）（GT-7700：GM747902参考）
//		PPrmSS[S_P03][PAYMENT_COUNT] = 1L;						// 精算追番
// GM849100(E) 名鉄協商コールセンター対応（T合計時に精算追番をクリアしない）（GT-7700：GM747902参考）
// GM849100(S) 名鉄協商コールセンター対応（精算／精算中止の追番を１つにする）（GT-7700：GM747902参考）
//		PPrmSS[S_P03][CANCEL_COUNT] = 1L;						// 精算中止追番
// GM849100(E) 名鉄協商コールセンター対応（精算／精算中止の追番を１つにする）（GT-7700：GM747902参考）
		PPrmSS[S_P03][DEPOSIT_COUNT] = 1L;						// 預り証追番
		PPrmSS[S_P03][TURIKAN_COUNT] = 1L;						// 釣銭管理合計追番
		PPrmSS[S_P03][COIN_SAFE_COUNT] = 1L;					// ｺｲﾝ金庫追番
		PPrmSS[S_P03][NOTE_SAFE_COUNT] = 1L;					// 紙幣金庫追番
		PPrmSS[S_P03][FUSEI_COUNT] = 1L;						// 不正出庫追番
		PPrmSS[S_P03][KIYOUSEI_COUNT] = 1L;						// 強制出庫追番
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		Edy_OpeDealNo = 0;										// Main管理用Edy取引通番
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		break;

	case CLEAR_COUNT_GT:										// 追番ｸﾘｱ(GT合計)
		PPrmSS[S_P03][T_TOTAL_COUNT] = 1L;						// T合計追番
		if (prm_get(COM_PRM, S_TOT, 1, 1, 1) == 0) {
			PPrmSS[S_P03][ALL_COUNT] = 1L;						// 通し追番
		}
		break;

	case CLEAR_COUNT_MT:										// 追番ｸﾘｱ(MT合計)
		PPrmSS[S_P03][GT_TOTAL_COUNT] = 1L;						// GT合計追番
		PPrmSS[S_P03][ALL_COUNT] = 1L;							// 通し追番
		break;

	default:													// 各種追番ｸﾘｱ
		if (kind > 0 && kind <= COUNT_MAX){
			PPrmSS[S_P03][kind] = 1L;
		}
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			追番選択
//[]----------------------------------------------------------------------[]
///	@param[in]		oiban	追番保存先<br>
///	@return			通し追い番／個別追い番／０
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/22<br>
///					Update	:	
///[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ulong	CountSel( ST_OIBAN *oiban )
{
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	default:
//	case	0:		// 個別追い番
		return (ulong)oiban->i;
	case	1:		// 通し追い番
		return (ulong)oiban->w;
	case	2:		// 追い番なし
		return 0;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			追番加算
//[]----------------------------------------------------------------------[]
///	@param[in]		*dst	加算先<br>
///	@param[in]		*src	加算元<br>
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/22<br>
///					Update	:	
///[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	CountAdd(SYUKEI *dst, SYUKEI *src)
{
	int		no;
	for (no = _OBN_WHOLE; no < _OBN_AREA_MAX; no++) {
		if (src->oiban_range[no][0] == 0) {
			continue;	// 追い番範囲なし
		}
		if (dst->oiban_range[no][0] == 0) {
		// 追い番範囲なし
			dst->oiban_range[no][0] = src->oiban_range[no][0];		// 開始を保存
		}
		dst->oiban_range[no][1] = src->oiban_range[no][1];			// 終了を保存
	}
}
