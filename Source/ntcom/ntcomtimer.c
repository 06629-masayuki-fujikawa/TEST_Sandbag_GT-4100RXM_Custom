// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/*[]----------------------------------------------------------------------[]
 *|	filename: ntcomtimer.c
 *[]----------------------------------------------------------------------[]
 *| summary	: �^�C�}�[�@�\
 *| author	: machida.k
 *| date	: 2005.06.09
 *| update	:
 *|		2005-12-20			machida.k	RAM���L��
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
 *| summary	: �^�C�}�[�@�\������
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
 *| summary	: �^�C�}�[�N���G�C�g
 *| param	: unit - �^�C�}�[���x(1, 10, 100)
 *|			  time - �^�C�}�[�Ԋu([msec])
 *|			  func - �^�C���A�E�g���ɌĂяo���֐�
 *|			  cyclic - TRUE as �T�C�N���b�N�^�C�}, FALSE as �P�V���b�g�^�C�}
 *| return	: �^�C�}�[ID as success, 0 as error
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
				/* 1�V���b�g�̂Ƃ��͎w�肳�ꂽ���Ԃ�ۏ؂��邽�߂ɃJ�E���^��+1���� */
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
 *| summary	: �^�C�}�[�f���[�g
 *| param	: id - �^�C�}�[ID
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
 *| summary	: �^�C�}�[�X�^�[�g
 *| param	: id - �^�C�}�[ID
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
 *| summary	: �^�C�}�[�X�g�b�v
 *| param	: id - �^�C�}�[ID
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
 *| summary	: id�Ŏw�肳�ꂽ�^�C�}�̏ڍ׏����擾����
 *| param	: id - �^�C�}�[ID
 *| return	: id���w���^�C�}�[
 *[]----------------------------------------------------------------------[]*/
T_NT_TIM_CTRL	*get_timer(ushort id)
{
	T_NT_TIM_CTRL *timer;
	ushort ch, chmax;

	/* ����P�ʂ̃`�F�b�N���^�C�}�[�z��̎擾 */
	if (get_unit_detail(_GET_UNIT(id), &chmax, &timer) == FALSE) {
		return FALSE;		/* "id" is invalid */
	}

	/* �^�C�}�[�`�����l���̃`�F�b�N */
	ch = _GET_CH(id);
	if (ch >= chmax) {
		return FALSE;		/* "id" is invalid */
	}

	return &timer[ch];
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get_unit_detail()
 *[]----------------------------------------------------------------------[]
 *| summary	: unit�Ŏw�肳�ꂽ�^�C�}���x�̏����擾����
 *| param	: unit - �^�C�}�[ID
 *|			  pchmax - �^�C�}�[�`�����l����			<OUT>
 *|			  ptimer - �^�C�}�[�z��̐擪			<OUT>
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
 *| summary	: 1ms�^�C�}���荞�݃n���h��(���ݖ��g�p)
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
 *| summary	: �^�C���A�E�g�`�F�b�N����
 *| param	:
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTCom_TimoutChk()
{
	T_NT_TIM_CTRL *tim;
	int chnum;

	if (NTComTimer_Enable) {
		// 1ms�^�C�}�[�`�F�b�N
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

		// 10ms�^�C�}�[�`�F�b�N
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

		// 100ms�^�C�}�[�`�F�b�N
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

// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
