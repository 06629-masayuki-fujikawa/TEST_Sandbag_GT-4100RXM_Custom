// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/*[]----------------------------------------------------------------------[]
 *|	filename: ntcomtimer.c
 *[]----------------------------------------------------------------------[]
 *| summary	: タイマー機能
 *| author	: machida.k
 *| date	: 2005.06.09
 *| update	:
 *|		2005-12-20			machida.k	RAM共有化
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
#define	_GET_UNIT(id)		((ushort)(id) >> 8)
#define	_GET_CH(id)			((unsigned char)(id))
#define	_MAKE_ID(unit, ch)	(((unsigned char)(unit)<<8) | (ch))

static	T_NT_TIM_CTRL	*get_timer(ushort id);
static	BOOL	get_unit_detail(ushort unit, ushort *pchmax, T_NT_TIM_CTRL **ptimer);

static	long	NTComTim10ms;
static	long	NTComTim100ms;

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComTimer_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: タイマー機能初期化
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComTimer_Init(void)
{
//	uchar ists;
	int i;

	NTComTimer_Enable = FALSE;

	for (i = 0; i < NT_TIM_1MS_CH_MAX; i++) {
		NTComTimer_1ms[i].sts = NT_TIM_INVALID;
	}
	for (i = 0; i < NT_TIM_10MS_CH_MAX; i++) {
		NTComTimer_10ms[i].sts = NT_TIM_INVALID;
	}
	for (i = 0; i < NT_TIM_100MS_CH_MAX; i++) {
		NTComTimer_100ms[i].sts = NT_TIM_INVALID;
	}

	NTComTimer_Enable = TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComTimer_Create()
 *[]----------------------------------------------------------------------[]
 *| summary	: タイマークリエイト
 *| param	: unit - タイマー精度(1, 10, 100)
 *|			  time - タイマー間隔([msec])
 *|			  func - タイムアウト時に呼び出す関数
 *|			  cyclic - TRUE as サイクリックタイマ, FALSE as １ショットタイマ
 *| return	: タイマーID as success, 0 as error
 *[]----------------------------------------------------------------------[]*/
ushort	NTComTimer_Create(ushort unit, ushort time, void (*func)(void), BOOL cyclic)
{
	ushort			i;
	ushort			chmax;
	T_NT_TIM_CTRL	*timer;

	if (get_unit_detail(unit, &chmax, &timer) == FALSE) {
		return 0;		/* "unit" is invalid */
	}

	for (i = 0; i < chmax; i++) {
		if (timer[i].sts == NT_TIM_INVALID) {
			timer[i].sts	= NT_TIM_STOP;
			timer[i].func	= func;
			timer[i].cyclic= cyclic;
			timer[i].time	= time / unit;
			if (cyclic == FALSE) {
				/* 1ショットのときは指定された時間を保証するためにカウンタを+1する */
				timer[i].time++;
			}
			break;
		}
	}

	if (i >= chmax) {
		return 0;		/* no free timer */
	}

	return (ushort)_MAKE_ID(unit, i);	/* success */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComTimer_Delete()
 *[]----------------------------------------------------------------------[]
 *| summary	: タイマーデリート
 *| param	: id - タイマーID
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComTimer_Delete(ushort id)
{
	T_NT_TIM_CTRL	*timer;

	timer = get_timer(id);
	if (timer == NULL) {
		return FALSE;		/* "id" is invalid */
	}

	timer->sts = NT_TIM_INVALID;

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComTimer_Start()
 *[]----------------------------------------------------------------------[]
 *| summary	: タイマースタート
 *| param	: id - タイマーID
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComTimer_Start(ushort id)
{
	T_NT_TIM_CTRL	*timer;

	timer = get_timer(id);
	if (timer == NULL) {
		return FALSE;		/* "id" is invalid */
	}

	if (timer->sts == NT_TIM_INVALID) {
		return FALSE;		/* timer is not created */
	}

	timer->sts = NT_TIM_START;
	timer->cnt = timer->time;

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComTimer_Stop()
 *[]----------------------------------------------------------------------[]
 *| summary	: タイマーストップ
 *| param	: id - タイマーID
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
int	NTComTimer_Stop(ushort id)
{
	T_NT_TIM_CTRL	*timer;

	timer = get_timer(id);
	if (timer == NULL) {
		return FALSE;		/* "id" is invalid */
	}

	if (timer->sts == NT_TIM_INVALID) {
		return FALSE;		/* timer is not created */
	}

	timer->sts = NT_TIM_STOP;

	return TRUE;
}

/*======================================================================================[MISC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: get_detail()
 *[]----------------------------------------------------------------------[]
 *| summary	: idで指定されたタイマの詳細情報を取得する
 *| param	: id - タイマーID
 *| return	: idが指すタイマー
 *[]----------------------------------------------------------------------[]*/
T_NT_TIM_CTRL	*get_timer(ushort id)
{
	T_NT_TIM_CTRL *timer;
	ushort ch, chmax;

	/* 測定単位のチェック＆タイマー配列の取得 */
	if (get_unit_detail(_GET_UNIT(id), &chmax, &timer) == FALSE) {
		return FALSE;		/* "id" is invalid */
	}

	/* タイマーチャンネルのチェック */
	ch = _GET_CH(id);
	if (ch >= chmax) {
		return FALSE;		/* "id" is invalid */
	}

	return &timer[ch];
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get_unit_detail()
 *[]----------------------------------------------------------------------[]
 *| summary	: unitで指定されたタイマ精度の情報を取得する
 *| param	: unit - タイマーID
 *|			  pchmax - タイマーチャンネル数			<OUT>
 *|			  ptimer - タイマー配列の先頭			<OUT>
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	get_unit_detail(ushort unit, ushort *pchmax, T_NT_TIM_CTRL **ptimer)
{
	switch (unit) {
	case NT_TIM_UNIT_1MS:
		*ptimer = NTComTimer_1ms;
		*pchmax = _countof(NTComTimer_1ms);
		break;
	case NT_TIM_UNIT_10MS:
		*ptimer = NTComTimer_10ms;
		*pchmax = _countof(NTComTimer_10ms);
		break;
	case NT_TIM_UNIT_100MS:
		*ptimer = NTComTimer_100ms;
		*pchmax = _countof(NTComTimer_100ms);
		break;
	default:
		return FALSE;		/* "id" is invalid */
	}

	return TRUE;
}

/*=======================================================================================[ISR]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_2msInt()
 *[]----------------------------------------------------------------------[]
 *| summary	: 1msタイマ割り込みハンドラ(現在未使用)
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTCom_2msInt( void )
{
	register T_NT_TIM_CTRL *tim;
	int chnum = NT_TIM_1MS_CH_MAX;

	if (NTComTimer_Enable) {
		while (--chnum >= 0) {
			tim = &NTComTimer_1ms[chnum];
			if (tim->sts ==  NT_TIM_START) {
				if (tim->cnt > 0) {
					--tim->cnt;
				}
			}
		}

		// 10ms
		++NTComTim10ms;
		if(NTComTim10ms >= 5) {
			chnum = NT_TIM_10MS_CH_MAX;
			while (--chnum >= 0) {
				tim = &NTComTimer_10ms[chnum];
				if (tim->sts ==  NT_TIM_START) {
					if (tim->cnt > 0) {
						--tim->cnt;
					}
				}
			}
			NTComTim10ms = 0;
		}

		// 100ms
		++NTComTim100ms;
		if(NTComTim100ms >= 50) {
			chnum = NT_TIM_100MS_CH_MAX;
			while (--chnum >= 0) {
				tim = &NTComTimer_100ms[chnum];
				if (tim->sts ==  NT_TIM_START) {
					if (tim->cnt > 0) {
						--tim->cnt;
					}
				}
			}
			NTComTim100ms = 0;
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_TimoutChk()
 *[]----------------------------------------------------------------------[]
 *| summary	: タイムアウトチェック処理
 *| param	:
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTCom_TimoutChk()
{
	T_NT_TIM_CTRL *tim;
	int chnum;

	if (NTComTimer_Enable) {
		// 1msタイマーチェック
		chnum = NT_TIM_1MS_CH_MAX;
		while (--chnum >= 0) {
			tim = &NTComTimer_1ms[chnum];
			if (tim->sts ==  NT_TIM_START) {
				if (tim->cnt == 0) {
					tim->func();
					if (tim->cyclic) {
						tim->cnt = tim->time;
					}
				}
			}
		}

		// 10msタイマーチェック
		chnum = NT_TIM_10MS_CH_MAX;
		while (--chnum >= 0) {
			tim = &NTComTimer_10ms[chnum];
			if (tim->sts ==  NT_TIM_START) {
				if (tim->cnt == 0) {
					tim->func();
					if (tim->cyclic) {
						tim->cnt = tim->time;
					}
				}
			}
		}

		// 100msタイマーチェック
		chnum = NT_TIM_100MS_CH_MAX;
		while (--chnum >= 0) {
			tim = &NTComTimer_100ms[chnum];
			if (tim->sts ==  NT_TIM_START) {
				if (tim->cnt == 0) {
					tim->func();
					if (tim->cyclic) {
						tim->cnt = tim->time;
					}
				}
			}
		}
	}
}

// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
