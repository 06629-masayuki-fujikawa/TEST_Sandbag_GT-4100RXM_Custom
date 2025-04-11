//[]----------------------------------------------------------------------[]
///	@file		oiban.c
///	@brief		'oiban' management
/// @date		2008/09/22
///	@author		MATSUSHITA
///	@version	GT-7000_36
/// @note		�ʂ��ǂ��ԁA�ǂ��Ԕ͈͑Ή�
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
// �ʂ��ǂ��Ԃƌʒǂ��Ԃ��y�A�ŊǗ�����
// �ʂ��ǂ��Ԃ́h�݂��o���h�����Ƃ��A�X�V���邩�A�h�ԋp�h���K�v
// �����ɕ����̒ǂ��Ԃ�݂��o�����Ƃ͂ł��Ȃ��i�a��ؒǂ��Ԃ̂ݗ�O�j
// �ǂ��ԍX�V�͂����ɕۑ��������̂��{�P���Čʃp�����[�^�ɏ����߂�
static	ST_OIBAN	oiban_work[_OBN_AREA_MAX];
#pragma	section

static	const	int		kind_no[][2] = {
		{PAYMENT_COUNT,		_OBN_PAYMENT}	// ���Z�ǔ�
// GM849100(S) ���S�����R�[���Z���^�[�Ή��i���Z�^���Z���~�̒ǔԂ��P�ɂ���j�iGT-7700�FGM747902�Q�l�j
//	,	{CANCEL_COUNT,		_OBN_CANCEL}	// ���Z���~�ǔ�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��i���Z�^���Z���~�̒ǔԂ��P�ɂ���j�iGT-7700�FGM747902�Q�l�j
	,	{DEPOSIT_COUNT,		_OBN_DEPOSIT}	// �a��ؒǔ�
	,	{TURIKAN_COUNT,		_OBN_TURIKAN}	// �ޑK�Ǘ����v�ǂ���
	,	{COIN_SAFE_COUNT,	_OBN_COIN_SAFE}	// ��݋��ɒǔ�
	,	{NOTE_SAFE_COUNT,	_OBN_NOTE_SAFE}	// �������ɒǔ�
	,	{KIYOUSEI_COUNT,	_OBN_KIYOUSEI}	// �����o�ɒǔ�
	,	{FUSEI_COUNT,		_OBN_FUSEI}		// �s���o�ɒǔ�
	,	{T_TOTAL_COUNT,		_OBN_T_TOTAL}	// T���v�ǔ�
	,	{GT_TOTAL_COUNT,	_OBN_GT_TOTAL}	// GT���v�ǔ�
	,	{-1, -1}
};

// prototype
static	int		search_table(int data, int type);
#define	no_search(kind)	search_table((int)kind,0)
#define	kind_search(no)	search_table(no,1)

//[]----------------------------------------------------------------------[]
///	@brief			�ǂ��Ԏ�ʑ��ݕϊ�
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: ���f�[�^
///	@param[in]		type	: 0:kind->no/1:no->kind
///	@return			�ϊ�����<br>
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
///	@brief			�ǔԏ�����
//[]----------------------------------------------------------------------[]
///	@param[in]		clr		: RAM������
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
///	@brief			�ǔԔ��s��
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
			return 0;		// �݂��o����
		}
	}
	return 1;		// �݂��o����
}

//[]----------------------------------------------------------------------[]
///	@brief			�ǔԔ��s
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �ǔԂ̎��<br>
///	@param[out]		oiban	: �ǔԕۑ���<br>
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
		// GT�ǂ��ԂȂ�
			return 1;
		}
	}
	no = no_search(kind);
	if (no < 0) {
		return 0;
	}
	if (oiban_work[no].w == 0) {
	// �ʂ��ǂ��Ԗ����s
		for (i = _OBN_PAYMENT; i < _OBN_AREA_MAX; i++) {
			if (oiban_work[i].w != 0){
				break;
			}
		}
		if (i < _OBN_AREA_MAX) {
		// ���ɑ݂��o����
			if (no != _OBN_DEPOSIT ||
				(i != _OBN_PAYMENT && i != _OBN_CANCEL)) {
				return 0;	// �ʂ��ǂ��Ԏg�p��
			}
			oiban_work[no].i = PPrmSS[S_P03][kind];
			oiban_work[no].w = (PPrmSS[S_P03][ALL_COUNT] >= _OIBAN_MAX)?
									1 : (PPrmSS[S_P03][ALL_COUNT]+1);
		}
		else {
		// �݂��o���Ȃ�
			oiban_work[no].i = PPrmSS[S_P03][kind];
			oiban_work[no].w = PPrmSS[S_P03][ALL_COUNT];
		}
	}
	*oiban = oiban_work[no];
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ǔԶ���+1
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �ǔԂ̎��<br>
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
		// �ʂ��ǂ��ԑ݂��o����
			if (kind == T_TOTAL_COUNT){
				wsy = &sky.gsyuk;
			}
			else if (kind == GT_TOTAL_COUNT){
				wsy = &sky.msyuk;
			}
			else{
				wsy = &sky.tsyuk;
			}
//			wptoff();			// �ǂ��Ԕ͈͕ۑ��J�n
			if (wsy->oiban_range[_OBN_WHOLE][0] == 0) {
				wsy->oiban_range[_OBN_WHOLE][0] = oiban_work[no].w;
			}
			wsy->oiban_range[_OBN_WHOLE][1] = oiban_work[no].w;
			if (wsy->oiban_range[no][0] == 0) {
				wsy->oiban_range[no][0] = oiban_work[no].i;
			}
			wsy->oiban_range[no][1] = oiban_work[no].i;
//			wpton();			// �ǂ��Ԕ͈͕ۑ��I��
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
			oiban_work[no].w = 0;					// �ʂ��ǂ��Ԕj��
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�ǔԕԋp
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �ǔԂ̎��<br>
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
			oiban_work[no].w = 0;					// �ʂ��ǂ��Ԕj��
		}
	}
	else {
		no = no_search(kind);
		if (no >= 0) {
			oiban_work[no].w = 0;					// �ʂ��ǂ��Ԕj��
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�ǔԶ��ĸر
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	�ǔԂ̎��<br>
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
	case CLEAR_COUNT_ALL:										// �ǔԸر(�S��)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iT���v���ɐ��Z�ǔԂ��N���A���Ȃ��j�iGT-7700�FGM747902�Q�l�j
// CLEAR_COUNT_ALL�͎g�p����Ă��Ȃ�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iT���v���ɐ��Z�ǔԂ��N���A���Ȃ��j�iGT-7700�FGM747902�Q�l�j
		for (i = 1; i <= COUNT_MAX; i++) {
			PPrmSS[S_P03][i] = 1L;
		}
		break;

	case CLEAR_COUNT_T:											// �ǔԸر(T���v)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iT���v���ɐ��Z�ǔԂ��N���A���Ȃ��j�iGT-7700�FGM747902�Q�l�j
//		PPrmSS[S_P03][PAYMENT_COUNT] = 1L;						// ���Z�ǔ�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iT���v���ɐ��Z�ǔԂ��N���A���Ȃ��j�iGT-7700�FGM747902�Q�l�j
// GM849100(S) ���S�����R�[���Z���^�[�Ή��i���Z�^���Z���~�̒ǔԂ��P�ɂ���j�iGT-7700�FGM747902�Q�l�j
//		PPrmSS[S_P03][CANCEL_COUNT] = 1L;						// ���Z���~�ǔ�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��i���Z�^���Z���~�̒ǔԂ��P�ɂ���j�iGT-7700�FGM747902�Q�l�j
		PPrmSS[S_P03][DEPOSIT_COUNT] = 1L;						// �a��ؒǔ�
		PPrmSS[S_P03][TURIKAN_COUNT] = 1L;						// �ޑK�Ǘ����v�ǔ�
		PPrmSS[S_P03][COIN_SAFE_COUNT] = 1L;					// ��݋��ɒǔ�
		PPrmSS[S_P03][NOTE_SAFE_COUNT] = 1L;					// �������ɒǔ�
		PPrmSS[S_P03][FUSEI_COUNT] = 1L;						// �s���o�ɒǔ�
		PPrmSS[S_P03][KIYOUSEI_COUNT] = 1L;						// �����o�ɒǔ�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		Edy_OpeDealNo = 0;										// Main�Ǘ��pEdy����ʔ�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		break;

	case CLEAR_COUNT_GT:										// �ǔԸر(GT���v)
		PPrmSS[S_P03][T_TOTAL_COUNT] = 1L;						// T���v�ǔ�
		if (prm_get(COM_PRM, S_TOT, 1, 1, 1) == 0) {
			PPrmSS[S_P03][ALL_COUNT] = 1L;						// �ʂ��ǔ�
		}
		break;

	case CLEAR_COUNT_MT:										// �ǔԸر(MT���v)
		PPrmSS[S_P03][GT_TOTAL_COUNT] = 1L;						// GT���v�ǔ�
		PPrmSS[S_P03][ALL_COUNT] = 1L;							// �ʂ��ǔ�
		break;

	default:													// �e��ǔԸر
		if (kind > 0 && kind <= COUNT_MAX){
			PPrmSS[S_P03][kind] = 1L;
		}
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�ǔԑI��
//[]----------------------------------------------------------------------[]
///	@param[in]		oiban	�ǔԕۑ���<br>
///	@return			�ʂ��ǂ��ԁ^�ʒǂ��ԁ^�O
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/22<br>
///					Update	:	
///[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ulong	CountSel( ST_OIBAN *oiban )
{
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	default:
//	case	0:		// �ʒǂ���
		return (ulong)oiban->i;
	case	1:		// �ʂ��ǂ���
		return (ulong)oiban->w;
	case	2:		// �ǂ��ԂȂ�
		return 0;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�ǔԉ��Z
//[]----------------------------------------------------------------------[]
///	@param[in]		*dst	���Z��<br>
///	@param[in]		*src	���Z��<br>
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
			continue;	// �ǂ��Ԕ͈͂Ȃ�
		}
		if (dst->oiban_range[no][0] == 0) {
		// �ǂ��Ԕ͈͂Ȃ�
			dst->oiban_range[no][0] = src->oiban_range[no][0];		// �J�n��ۑ�
		}
		dst->oiban_range[no][1] = src->oiban_range[no][1];			// �I����ۑ�
	}
}
