/*[]----------------------------------------------------------------------[]*/
/*| LCD control                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2005-02-01                                               |*/
/*| Update      : font_change 2005-09-01 T.Hashimoto                       |*/
/*| Update      : �V���Z�@�Ή� 2012-12-15 k.totsuka                        |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"flp_def.h"
#include	"tbl_rkn.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"strdef.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"can_def.h"
#include	"ntnet_def.h"
#include	"cre_ctrl.h"

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
extern void lcdbm_notice_pay(ulong pay_status);
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))

static	void	nmdsply( ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort );
static	void	nmdspll( ushort, ushort, ulong, ushort, ushort, ushort, ushort, ushort, ushort );
static	void	dsp_msg(uchar, uchar, uchar, ushort);
static	void	dsp_cyclic_msg(uchar,uchar,uchar);

int		chk_OK(void);
int		chk_service(void);
int		chk_prepaid(void);
int		chk_credit(void);
int		chk_coupon(void);
int		chk_teiki(void);
int		chk_note(void);
int		chk_azukari(void);
int		chk_flp(void);
int		chk_lock(void);
int		chk_suica(void);
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//int		chk_edy(void);
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
int		chk_Tcard(void);
// MH321800(S) hosoda IC�N���W�b�g�Ή�
int		chk_ec_credit(void);
int		chk_ec_emoney(void);
int		chk_coin(void);
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
int		chk_ec_qr(void);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

enum {
	CYCLIC_DSP01 = 1,
	CYCLIC_DSP09 = 9,
	EXTPAND_MSG1 = 41,
	EXTPAND_MSG2,
	EXTPAND_MSG3,
};

// �֐��^��`
typedef int (*funcDsp)(void);

// �\�����b�Z�[�W�e�[�u��
typedef struct {
	funcDsp func;		// �`�F�b�N�֐�
	int alternate;			// ����⃁�b�Z�[�WNo
}t_DspMsg;

// MH810100(S) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
static ulong old_pay_status = 0;
static BOOL diff_pay_status( ulong sta );
// MH810100(E) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)

//----------------------------------------------------------
// ��opestr.c��OPE_CHR_G_SALE_MSG[][]�Ɗ֘A���ďC���̂���
//----------------------------------------------------------
const t_DspMsg infomsglist[][2] = {
	{{chk_OK, 0}, {chk_OK, 0}},							// 00
	{{chk_OK, 0}, {chk_OK, 0}},							// 01
	{{chk_service, 0}, {chk_OK, 0}},					// 02
	{{chk_prepaid, EXTPAND_MSG1}, {chk_OK, 0}},			// 03
	{{chk_service, 5}, {chk_credit, 2}},				// 04
	{{chk_credit, 0}, {chk_OK, 0}},						// 05
	{{chk_prepaid, EXTPAND_MSG2}, {chk_credit, 3}},		// 06
	{{chk_service, 0}, {chk_OK, 0}},					// 07
	{{chk_service, 0}, {chk_OK, 0}},					// 08
	{{chk_OK, 0}, {chk_OK, 0}},							// 09
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//	{{chk_note, 11}, {chk_OK, 0}},						// 10
//	{{chk_OK, 0}, {chk_OK, 0}},							// 11
	{{chk_coin, 0}, {chk_note, 11}},					// 10
	{{chk_coin, 0}, {chk_OK, 0}},						// 11
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
	{{chk_service, 0}, {chk_OK, 0}},					// 12
	{{chk_service, 3}, {chk_prepaid, EXTPAND_MSG3}},	// 13
	{{chk_OK, 0}, {chk_OK, 0}},							// 14
	{{chk_OK, 0}, {chk_OK, 0}},							// 15
	{{chk_OK, 0}, {chk_OK, 0}},							// 16
	{{chk_OK, 0}, {chk_OK, 0}},							// 17
	{{chk_note, 0}, {chk_OK, 0}},						// 18
	{{chk_OK, 0}, {chk_OK, 0}},							// 19
	{{chk_OK, 0}, {chk_OK, 0}},							// 20
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
//-	{{chk_azukari, 0}, {chk_OK, 0}},					// 21
//-	{{chk_azukari, 0}, {chk_OK, 0}},					// 22
//-	{{chk_azukari, 0}, {chk_OK, 0}},					// 23
//-	{{chk_azukari, 0}, {chk_OK, 0}},					// 24
//-	{{chk_azukari, 0}, {chk_OK, 0}},					// 25
	{{chk_azukari, -1}, {chk_OK, 0}},					// 21
	{{chk_azukari, -1}, {chk_OK, 0}},					// 22
	{{chk_azukari, -1}, {chk_OK, 0}},					// 23
	{{chk_azukari, -1}, {chk_OK, 0}},					// 24
	{{chk_azukari, -1}, {chk_OK, 0}},					// 25
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
	{{chk_OK, 0}, {chk_OK, 0}},							// 26
	{{chk_OK, 0}, {chk_OK, 0}},							// 27
	{{chk_OK, 0}, {chk_OK, 0}},							// 28
	{{chk_flp, 32}, {chk_OK, 0}},						// 29 �t���b�v�������������Ƃ�
	{{chk_flp, 32}, {chk_OK, 0}},						// 30 ���b�N�������������Ƃ�
	{{chk_OK, 0}, {chk_OK, 0}},							// 31 �m�F����
	{{chk_OK, 0}, {chk_OK, 0}},							// 32 ���b�N������
	{{chk_OK, 0}, {chk_OK, 0}},							// 33
	{{chk_OK, 0}, {chk_OK, 0}},							// 34
	{{chk_OK, 0}, {chk_OK, 0}},							// 35
	{{chk_OK, 0}, {chk_OK, 0}},							// 36
	{{chk_OK, 0}, {chk_OK, 0}},							// 37
	{{chk_credit, 0}, {chk_OK, 0}},						// 38
	{{chk_credit, 0}, {chk_OK, 0}},						// 39
	{{chk_Tcard, 0}, {chk_OK, 0}},						// 40
	//----------------------------------------------//
	// �ȉ��A�g�����b�Z�[�W
	//----------------------------------------------//
	{{chk_coupon, 0}, {chk_OK, 0}},						// EXPAND1
	{{chk_coupon, 5}, {chk_credit, EXTPAND_MSG1}},		// EXPAND2
	{{chk_service, EXTPAND_MSG1}, {chk_coupon, 2}},		// EXPAND3
};

extern char	PrcKigenStr[32];

/*[]----------------------------------------------------------------------[]*/
/*| �޲���ް��S�p�\��                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opedsp                                                  |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ushort dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not                        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opedsp( ushort	low,
				ushort	col,
				ushort	dat,
				ushort	siz,
				ushort	zer,
				ushort	mod,
				ushort	color,
				ushort	blink )
{
	nmdsply( low, col, dat, siz, zer, mod, color, blink, 1 );
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �޲���ް����p�\��                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opedsp3                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ushort dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not                        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opedsp3( ushort	low,
				 ushort	col,
				 ushort	dat,
				 ushort	siz,
				 ushort	zer,
				 ushort	mod,
				 ushort	color,
				 ushort	blink )
{
	nmdsply( low, col, dat, siz, zer, mod, color, blink, 0 );
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �޲���ް��\��                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nmdsply                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ushort dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not                        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*|              : ushort big    0:HALF / 1:ALL                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	void	nmdsply( ushort	low,
						 ushort	col,
						 ushort	dat,
						 ushort	siz,
						 ushort	zer,
						 ushort	mod,
						 ushort	color,
						 ushort	blink,
						 ushort	big )
{
	unsigned char	d_buf[8];
	unsigned char	w_buf[4];
	short	bb, i;

	bb = 0;
	if( siz > 4 ) siz = 4;
	if( siz != 0 ) {
		for( bb = 1,i = 0; i < siz; i++ ) {
			bb = bb * 10;
		}
	}
	dat = ( dat % bb );
	bb = bb / 10;
	for( i = 0;i < siz; i++ ) {
		w_buf[i] = (unsigned char)(( dat / bb ) | 0x30 );
		dat = ( dat % bb );
		bb = bb / 10;
	}
	if( zer == 0 ) {
		zrschr( w_buf, (uchar)siz );
	}
	if( big == 1 ) {
		as1chg( w_buf, d_buf, (uchar)siz );
		grachr( low, col, (ushort)(siz * 2), mod, color, blink, d_buf );
	} else {
		grachr( low, col, siz, mod, color, blink, w_buf );
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| long�^�޲���ް��S�p�\��                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opedpl                                                  |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ulong  dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not                        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opedpl( ushort	low,
				ushort	col,
				ulong	dat,
				ushort	siz,
				ushort	zer,
				ushort	mod,
				ushort	color,
				ushort	blink )
{
	nmdspll( low, col, dat, siz, zer, mod, color, blink, 1 );
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| long�^�޲���ް����p�\��                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opedpl3                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ulong  dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not                        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opedpl3( ushort	low,
				 ushort	col,
				 ulong	dat,
				 ushort	siz,
				 ushort	zer,
				 ushort	mod,
				 ushort	color,
				 ushort	blink )
{
	nmdspll( low, col, dat, siz, zer, mod, color, blink, 0 );
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| long�^�޲���ް��\��                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nmdspll                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ulong  dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not / 2:'�' supress        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*|              : ushort big    0:HALF / 1:ALL                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	void	nmdspll( ushort	low,
						 ushort	col,
						 ulong	dat,
						 ushort	siz,
						 ushort	zer,
						 ushort	mod,
						 ushort	color,
						 ushort	blink,
						 ushort	big )
{
	unsigned char	d_buf[20];
	unsigned char	w_buf[10];
	short		i;
// MH810100(S) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
//	unsigned long	bb;
	unsigned long long	bb;		// 10��10�悪���肫��Ȃ��̂Ŋg��
// MH810100(E) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)

	bb = 0;
	if( siz > 10 ) siz =10;
	if( siz != 0 ) {
		for( bb = 1l,i = 0; i < siz; i++ ) {
			bb = bb * 10l;
		}
	}
	dat = ( dat % bb );
	bb = bb / 10l;
	for( i = 0;i < siz; i++ ) {
		w_buf[i] = (unsigned char)(( dat / bb ) | 0x30);
		dat = ( dat % bb );
		bb = bb / 10l;
	}
	if( zer == 0 ) {
		zrschr( w_buf, (uchar)siz );
	}
	if( zer == 2 ) {
		pntchr( w_buf, (uchar)siz );
	}

	if( big == 1 ) {
		as1chg( w_buf, d_buf, (uchar)siz );
		grachr( low, col, (ushort)(siz * 2), mod, color, blink, d_buf );
	}else{
		grachr( low, col, siz, mod, color, blink, w_buf );
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| long�^HEX�ް����p�\��                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opedpl2                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ulong  dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not / 2:'�' supress        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opedpl2( ushort	low,
				 ushort	col,
				 ulong	dat,
				 ushort	siz,
				 ushort	zer,
				 ushort	mod,
				 ushort	color,
				 ushort	blink )
{
	unsigned char	w_buf[10];
	short		i;

	for( i = siz - 1; i >= 0; i-- ) {
		w_buf[i] = ( unsigned char )( dat & 0x000fL );
		if( w_buf[i] <= 9 ) {
			w_buf[i] += '0';
		} else {
			w_buf[i] += 0x37;
		}
		dat = dat >> 4L;
	}
	if( zer == 0 ) {
		zrschr( w_buf, (uchar)siz );
	}
	if( zer == 2 ) {
		pntchr( w_buf, (uchar)siz );
	}
	grachr( low, col, siz, mod, color, blink, w_buf );
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| HEX�ް��S�p�\��                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opedsp5                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ushort dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not                        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : S.Ogura                                                 |*/
/*| Date         : 2005-09-08                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opedsp5( ushort	low,
				 ushort	col,
				 ushort	dat,
				 ushort	siz,
				 ushort	zer,
				 ushort	mod,
				 ushort	color,
				 ushort	blink )
{
	unsigned char	d_buf[8];
	unsigned char	w_buf[4];
	short	i;

	for( i = siz - 1; i >= 0; i-- ) {
		w_buf[i] = ( unsigned char )( dat & 0x000f );
		if( w_buf[i] <= 9 ) {
			w_buf[i] += '0';
		} else {
			w_buf[i] += 0x37;
		}
		dat = dat >> 4;
	}
	if( zer == 0 ) {
		zrschr( w_buf, (uchar)siz );
	}
	as1chg( w_buf, d_buf, (uchar)siz );
	grachr( low, col, (ushort)(siz*2), mod, color, blink, (uchar*)d_buf );
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| long�^�޲���ް��S�p�\��(���񂹗p)                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opedsp6                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ulong  dat    data                                      |*/
/*|              : ushort zer    0:0supress / 1:not                        |*/
/*|              : ushort mod    mode = 0(normal) / 1(reverse)             |*/
/*|              : ushort color  charactor color                           |*/
/*|              : ushort blink  blink = 0(none) - blink time(10ms)        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2009-06-19                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	opedsp6( ushort	low,
				 ushort	col,
				 ushort	dat,
				 ushort	zer,
				 ushort	mod,
				 ushort	color,
				 ushort	blink )
{
	if(dat / 10000)		/* �l��5������ */
	{
		nmdspll( low, col, (ulong)dat, 5, zer, mod, color, blink, 1 );
	}
	else if(dat / 1000)	/* �l��4������ */
	{
		nmdspll( low, col, (ulong)dat, 4, zer, mod, color, blink, 1 );
	}
	else if(dat / 100)	/* �l��3������ */
	{
		nmdspll( low, col, (ulong)dat, 3, zer, mod, color, blink, 1 );
	}
	else if(dat / 10)	/* �l��2������ */
	{
		nmdspll( low, col, (ulong)dat, 2, zer, mod, color, blink, 1 );
	}
	else				/* ��L�ȊO(1��) */
	{
		nmdspll( low, col, (ulong)dat, 1, zer, mod, color, blink, 1 );
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pntchr                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Ascii data � supress                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char *dat   change data                        |*/
/*|              : unsigned char cnt    character number                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/

void	pntchr( unsigned char	*dat,
				unsigned char	cnt )
{
	unsigned char	l_cnt;

	for( l_cnt = 0 ; l_cnt < ( cnt - (char)1 ) ; l_cnt++ ){
		if( dat[ l_cnt ] != '0' ){
			break;
		}
		dat[ l_cnt ] = '�';
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : as1chg                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ASCII DATA -> GDC DATA ( ALL )                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char *dat1		pre-change data                |*/
/*|              : unsigned char *dat2		changed data                   |*/
/*|              : unsigned char cnt		character number               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       : font_change 2005-09-01 T.Hashimoto                      |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void	as1chg(const uchar *dat1, uchar *dat2, uchar cnt)
{
	int		i,j;
	union {
		unsigned short	d;
		unsigned char	c[2];
	} buf;
	uchar	cc;

	for( i = j = 0 ; i < cnt ; i++ ){

		cc = dat1[i];
		if(( '0' <= cc ) && ( cc <= '9' )){

			buf.d = (ushort)(0x824f + ( cc - '0' )) ; // '�O'

		} else if(( 'A' <= cc ) && ( cc <= 'Z' )){

			buf.d = (ushort)(0x8260 + ( cc - 'A' )); // '�`'

		} else if(( 'a' <= cc ) && ( cc <= 'z' )){

			buf.d = (ushort)(0x8281 + ( cc - 'a' )); // '��'

		} else if( cc == ' ' ) {

			buf.d = 0x8140; // '�@';

		} else {

			buf.d = 0x8145; // '�E';

		}
		*(dat2+j) = buf.c[0];
		j++;
		*(dat2+j) = buf.c[1];
		j++;
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �j����j���p���̨�����ڽ�֕ύX                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspclk                                                  |*/
/*| PARAMETER    : uchar  wek   week = 0-6(Sunday-Saturday)                |*/
/*|              : ushort *buf  changed data buffer                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	wekchg( uchar wek, uchar *buf )
{
	if( wek >= 7 ){
		wek = 0;
	}
	buf[0] = WEKFFNT[wek][0];
	buf[1] = WEKFFNT[wek][1];
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���v�\��                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspclk                                                  |*/
/*| PARAMETER    : ushort lin    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort color  charactor color                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	dspclk(ushort lin, ushort color)
{
	// �t�����g��֎��v�\���w��

	uchar	d_buf[2];

	grachr(lin, 0, 30, 0, color, LCD_BLINK_OFF, OPE_CHR[1]);					// "    ��    ���i  �j     �F     "
	opedsp(lin, 0, (ushort)CLK_REC.mont, 2, 0, 0, color, LCD_BLINK_OFF);		// ���\��
	opedsp(lin, 6, (ushort)CLK_REC.date, 2, 0, 0, color, LCD_BLINK_OFF);		// ���\��

	wekchg( CLK_REC.week, d_buf );
	grachr(lin, 14, 2, 0, color, LCD_BLINK_OFF, d_buf );						// �j���\��
	
	opedsp(lin, 19, (ushort)CLK_REC.hour, 2, 0, 0, color, LCD_BLINK_OFF);		// ���\��
	opedsp(lin, 25, (ushort)CLK_REC.minu, 2, 1, 0, color, LCD_BLINK_OFF);		// ���\��
}
/*[]----------------------------------------------------------------------[]*/
/*| �w�i�F�ݒ�                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dsp_background_color                                    |*/
/*| PARAMETER    : ushort rgb      rgb    		(R=5bit�CG=6bit�CB=5bit)�@ |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-16                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	dsp_background_color( ushort rgb )
{
	t_BackColor bkclr;
	bkclr.Esc	=	LCD_ESC_BKCOLOR;
	bkclr.Rgb	=	rgb;												//RGB�ݒ�
	can_snd_data4((void *)&bkclr, sizeof(t_BackColor));
	return;
}
/*[]----------------------------------------------------------------------[]*/
/*| ���v�\��                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dsp_intime                                              |*/
/*| PARAMETER    : ushort lin    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort num    �Ԏ��ԍ�                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	dsp_intime( ushort lin, ushort num )
{
// MH322914(S) K.Onodera 2016/09/16 AI-V�Ή��F���u���Z(���Z���z�w��)
	// ���z�w�艓�u���Z ���A�ԗ��Ȃ��t���b�v�_�E���H
	if( (OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE && FLAPDT.flp_data[num-1].mode <= FLAP_CTRL_MODE3) || num == 0 ){
		return;
	}
// MH322914(E) K.Onodera 2016/09/16 AI-V�Ή��F���u���Z(���Z���z�w��)

	grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[19]);	// "����      ��    ��     �F     "

// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)
	// ���Ɏ����w�艓�u���Z
	if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_TIME ){
		opedsp(lin, 6, (ushort)g_PipCtrl.stRemoteTime.InTime.Mon,  2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
		opedsp(lin,12, (ushort)g_PipCtrl.stRemoteTime.InTime.Day, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// ���\��
		opedsp(lin,19, (ushort)g_PipCtrl.stRemoteTime.InTime.Hour,2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// ���\��
		opedsp(lin,25, (ushort)g_PipCtrl.stRemoteTime.InTime.Min, 2, 1, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// ���\��
		return;
	}
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)
	if( OPECTL.multi_lk ){											// ������Z�⍇����?
		opedsp(lin, 6, (ushort)LOCKMULTI.lock_mlt.mont, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// ���\��
		opedsp(lin,12, (ushort)LOCKMULTI.lock_mlt.date, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// ���\��
		opedsp(lin,19, (ushort)LOCKMULTI.lock_mlt.hour, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// ���\��
		opedsp(lin,25, (ushort)LOCKMULTI.lock_mlt.minu, 2, 1, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// ���\��
	}else{
		opedsp(lin, 6, (ushort)FLAPDT.flp_data[num-1].mont, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
		opedsp(lin,12, (ushort)FLAPDT.flp_data[num-1].date, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
		opedsp(lin,19, (ushort)FLAPDT.flp_data[num-1].hour, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
		opedsp(lin,25, (ushort)FLAPDT.flp_data[num-1].minu, 2, 1, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : zrschr                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Ascii data 0 supress                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char *dat	change data                        |*/
/*|              : unsigned char cnt	character number                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/

void	zrschr( unsigned char *dat, unsigned char cnt )
{
	unsigned char	l_cnt;

	for( l_cnt = 0 ; l_cnt < ( cnt - (char)1 ) ; l_cnt++ ){
		if( dat[ l_cnt ] != '0' ){
			break;
		}
		dat[ l_cnt ] = ' ';
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���l���͕\��(BIG̫��)                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : teninb                                                  |*/
/*| PARAMETER    : ushort nu   : �����ޯ̧                                 |*/
/*|              : ushort in_k : �\������(MAX4��)                          |*/
/*|              : ushort lin  : �\���s                                    |*/
/*|              : ushort col  : �\����(���[�̕���)                        |*/
/*|              : ushort color: �\���F                                    |*/
/*| RETURN VALUE : ret : �\���l                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	teninb( ushort nu, ushort in_k, ushort lin, ushort col, ushort color)
{
	uchar	c_bf[4];
	ushort	wcol;
	short	i;

	nu %= 10000;
	if( in_k > 4 )	in_k = 4;

	intoas( c_bf, nu, in_k );

// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ�������
//	for( i=0, wcol=0; i<in_k; i++, wcol+=3 ){
	for( i=0, wcol=0; i<in_k; i++, wcol+=2 ){
// MH810100(E) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ�������
		numchr( lin, (ushort)(col+wcol), color, LCD_BLINK_OFF, c_bf[i] );
	}
	return( nu );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���l���͕\��(BIG̫��)long�f�[�^                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : teninb                                                  |*/
/*| PARAMETER    : ulong nu   : �����ޯ̧                                 |*/
/*|              : ushort in_k : �\������(MAX6��)                          |*/
/*|              : ushort lin  : �\���s                                    |*/
/*|              : ushort col  : �\����(���[�̕���)                        |*/
/*|              : ushort color: �\���F                                    |*/
/*| RETURN VALUE : ret : �\���l                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	teninb2( ulong nu, ushort in_k, ushort lin, ushort col, ushort color)
{
	uchar	c_bf[6];
	ushort	wcol;
	short	i;

	nu %= 1000000L;
	if( in_k > 6 )	in_k = 6;

	intoasl( c_bf, nu, in_k );

	for( i=0, wcol=0; i<in_k; i++, wcol+=3 ){
		numchr( lin, (ushort)(col+wcol), color, LCD_BLINK_OFF, c_bf[i] );
	}
	return( nu );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���l���͕\��(BIG̫��)���Ïؔԍ���                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : teninb                                                  |*/
/*| PARAMETER    : ushort nu   : �����ޯ̧                                 |*/
/*|              : ushort in_k : �\������(MAX4��)                          |*/
/*|              : ushort lin  : �\���s                                    |*/
/*|              : ushort col  : �\����(���[�̕���)                        |*/
/*|              : uchar pas_cnt : �J�E���^�[                              |*/
/*|              : ushort color: �\���F                                    |*/
/*| RETURN VALUE : ret : �\���l                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2006-09-07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	teninb_pass( ushort nu, ushort in_k, ushort lin, ushort col, uchar pas_cnt, ushort color)
{
	uchar	c_bf[4];
	ushort	wcol;
	short	i;
	uchar	p_cnt;

	nu %= 10000;
	if( in_k > 4 )	in_k = 4;

	intoas( c_bf, nu, in_k );

	for( i=0, wcol=0; i<in_k; i++, wcol+=3 ){
		if(pas_cnt == 0){
			//"-"��\��
			bigcr(lin, (ushort)(col+wcol), 2, color, LCD_BLINK_OFF, (uchar *)"�|");	// "�|"
		}else{
			p_cnt = (uchar)(4 - pas_cnt);
			numchr( lin, (ushort)(col+wcol), color, LCD_BLINK_OFF, c_bf[p_cnt] );
			pas_cnt--;
		}
	}
	return( nu );
}


/*[]----------------------------------------------------------------------[]*/
/*| �B���Ïؔԍ����͕\��(BIG̫��)                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : blindpasword                                            |*/
/*| PARAMETER    : ushort nu   : �����ޯ̧                                 |*/
/*|              : ushort in_k : �\������(MAX4��)                          |*/
/*|              : ushort lin  : �\���s                                    |*/
/*|              : ushort col  : �\����(���[�̕���)                        |*/
/*| RETURN VALUE : ret : �\���l                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2006-05-16                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	blindpasword( ushort nu, ushort in_k, ushort lin, ushort col,uchar pas_cnt )
{
	uchar	c_bf[4];
	ushort	wcol;
	short	i;
	ushort	cnt;

	nu %= 10000;
	if( in_k > 4 )	in_k = 4;

	intoas( c_bf, nu, in_k );

	cnt = 0;
	if( nu != 0 ){
		cnt = binlen( (ulong)nu );
		for( i=0; i<cnt; i++ ){
			c_bf[3-i] = 'X';
		}
	}

	for( i=0, wcol=0; i<in_k; i++, wcol+=3 ){
		if(pas_cnt == 0){
			//"-"��\��
			bigcr(lin, (ushort)(col+wcol), 2, COLOR_FIREBRICK, LCD_BLINK_OFF, (uchar *)"�|");	// "�|"
		}else{
			bigcr(lin, (ushort)(col+wcol), 2, COLOR_FIREBRICK, LCD_BLINK_OFF, (uchar *)"�w");	// "�w"
			pas_cnt--;
		}
	}
	return( nu );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ԗ����\��(BIG̫��)                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : feedsp                                                  |*/
/*| PARAMETER    : ushort lin    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ulong  fee    �\������                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	feedsp( ushort lin, ulong fee )
{
	char	i;
	ulong	f_wk;
	uchar	w_buf[6];

	f_wk = fee;
	intoasl( w_buf, f_wk, 6 );
	if( fee < 1000000L ){
		zrschr( w_buf, 6 );
	}
	for( i=0; i<6; i++ ){
		numchr(lin, (ushort)(9+(3*i)), COLOR_FIREBRICK, LCD_BLINK_OFF, w_buf[i]);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ڋqLCD�ޯ�ײĐ���                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LcdBackLightCtrl                                        |*/
/*| PARAMETER    : char onoff  0=OFF(�ҋ@���̋P�x), 1=ON(���Z���̋P�x)     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LcdBackLightCtrl( char onoff )
{
	ushort	tim1, tim2;
	ushort	set;
	ushort	nightMode = 0;

	switch( prm_get( COM_PRM,S_PAY,29,1,1 ) ){
		case 0:														// �펞�_��
			onoff = 1;												// �펞ON�Ƃ���
			backlight = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027�D�E �_�����̋P�x
			break;
		case 1:														// �ݷ����͂ɂē_��
		case 2:														// �O���M���ɂē_��
		default:
			if(onoff == 0){											// OFF�̏ꍇ
				backlight = (ushort)prm_get(COM_PRM, S_PAY, 28, 2, 1);	// 02-0028�D�E �������̋P�x
			}else{													// ON�̏ꍇ
				backlight = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027�D�E �_�����̋P�x
			}
			// �P�x��ON/OFF�����܂� �P�x(LCD_contrast)0:�o�b�N���C�gOFF/0�ȊO�̓o�b�N���C�gON+�P�x�̐ݒ�l
			onoff = 1;
			break;
		case 3:														// ��Ԏ��ԑт̂ݓ_��
			set = (ushort)CPrmSS[S_PAY][30];
			tim1 = ( set / 100 ) * 60 + ( set % 100 );
			set = (ushort)CPrmSS[S_PAY][31];
			tim2 = ( set / 100 ) * 60 + ( set % 100 );
			if( tim1 == tim2 ){
				;													// ��Ԏ��ԑі���
			}else if( tim1 < tim2 ){
				if(( tim1 <= CLK_REC.nmin )&&( CLK_REC.nmin <= tim2 )){
					nightMode = 1;
				}
			}else{
				if(( CLK_REC.nmin >= tim1 )||( tim2 >= CLK_REC.nmin )){
					nightMode = 1;
				}
			}
			if(nightMode == 0){										// ���Ԏ��ԑт̏ꍇ
				backlight = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027�D�E �_�����̋P�x
			}else{													// ON�̏ꍇ
				backlight = (ushort)prm_get(COM_PRM, S_PAY, 28, 2, 1);	// 02-0028�D�E �������̋P�x
			}
			// �P�x��ON/OFF�����܂� �P�x(LCD_contrast)0:�o�b�N���C�gOFF/0�ȊO�̓o�b�N���C�gON+�P�x�̐ݒ�l
			onoff = 1;
			break;
	}
	lcd_backlight( onoff );
// MH810103(s) �d�q�}�l�[�Ή� �ҋ@��ʂł̎c���Ɖ��
	if(OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 1)){
// MH810103(s) �d�q�}�l�[�Ή� #5413 ���Z�s��ʂ܂��͌x����ʕ\�����ɁAVP���c���Ɖ�{�^����\�����Ă��܂��Ă���
		// ���A���^�C���ʐM/LCD��Ԃ��擾 or �x���ʒm��
// MH810104 GG119201(S) �d�q�W���[�i���Ή� #5973 �d�q�W���[�i�����p�s�̔������A�d�q�}�l�[�̎c���Ɖ�ł��Ȃ��Ȃ�
//		if( ERR_CHK[mod_realtime][1] || ERR_CHK[mod_tklslcd][1] || lcdbm_alarm_check()){
		if( ERR_CHK[mod_realtime][1] || ERR_CHK[mod_tklslcd][1] ){
// MH810104 GG119201(E) �d�q�W���[�i���Ή� #5973 �d�q�W���[�i�����p�s�̔������A�d�q�}�l�[�̎c���Ɖ�ł��Ȃ��Ȃ�
			if( isEC_USE() ){
				if(OPECTL.lcd_query_onoff != 0){	// �ω�������
					Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );
				}
			}
			OPECTL.lcd_query_onoff = 0;								// �ҋ@��ʂł̎c���Ɖ��(0:NG/1:OK/2:���̑�)
			return;
		}
// MH810103(e) �d�q�}�l�[�Ή� #5413 ���Z�s��ʂ܂��͌x����ʕ\�����ɁAVP���c���Ɖ�{�^����\�����Ă��܂��Ă���
		// �ҋ@�̏ꍇ���M����
		// �c���ۂ��`�F�b�N
		if(isEcEmoneyEnabledNotCtrl(1,1)){
			if( isEC_USE() ){
				if(OPECTL.lcd_query_onoff != 1){	// �ω�������
					Ec_Pri_Data_Snd( S_CNTL_DATA, 0x1 );
				}
			}
			OPECTL.lcd_query_onoff = 1;								// �ҋ@��ʂł̎c���Ɖ��(0:NG/1:OK/2:���̑�)
		}else{
			OPECTL.lcd_query_onoff = 0;								// �ҋ@��ʂł̎c���Ɖ��(0:NG/1:OK/2:���̑�)
		}
	}else{
		OPECTL.lcd_query_onoff = 2;								// �ҋ@��ʂł̎c���Ɖ��(0:NG/1:OK/2:���̑�)
	}
// MH810103(e) �d�q�}�l�[�Ή� �ҋ@��ʂł̎c���Ɖ��
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ��݉�ʕ\��                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpeLcd( no )                                            |*/
/*| PARAMETER    : no = ��ʇ�                                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpeLcd( ushort no )
{
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	ushort	loklen, Mloklen, lin;
//	ulong	w_tax;
//	ulong	wkl;
//
//// �Ԏ�\���p�ϐ�
//	uchar	r_syu = 0;		// �p�����[�^�ԍ��i�[
//	ushort syasyu = 0;		// �Ԏ�ԍ��i�[
//	uchar	ryosyu = 0;		//������ʊi�[
//
//	ulong	wk_pri;
//	uchar	set62,set63;
//
//	uchar msgno, i;
//	char	pri_time_flg;
//	ulong	WkCal;
//	ushort	wk_MsgNo;
//
//	if(( LCDNO != no )||( no == (ushort)-1 )){
//		// NOTE:�S��ʕ\���ؑւ���������CAN�ւ̕\���f�[�^��������������̂�WAIT�������B�����������͕\��������Ȃ���������
//		// �{����CAN�̑��M�o�b�t�@���g�傷�ׂ��ł��邪�\����ύX�̃��X�N�����������߂��̑̍�ɂƂǂ߂Ă���
//		xPause(4);// 40ms wait
//		switch (no) {
//		case 17:		// �x�ƒ�
//		case 18:		// �x��
//			dsp_background_color(COLOR_BLACK);
//			break;
//		default:
//			dsp_background_color(COLOR_WHITE);
//			break;
//		}
//		dispclr();													// Display All Clear
//	}
//	LCDNO = no;
//	switch( no ){
//		case 1:														// �ҋ@��ԉ�ʕ\��
//			dspclk(0, COLOR_DARKSLATEBLUE);							// ���v�\��
//
//			if( OPECTL.Pay_mod == 1 ){								// �߽ܰ�ޖY�ꏈ��?
//				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[35]);		// "  �Ïؔԍ��Y�ꐸ�Z���s���܂�  "
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[5]);		// "���Z��������                "
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[6]);		// "    �Ԏ��ԍ�����͂��ĉ�����  "
//			}else{
//				if( OPECTL.Seisan_Chk_mod == ON ){
//					grachr(1, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[65]);	// "          �������[�h          " ���]�\��
//				}
//
//#if	UPDATE_A_PASS
//				// ������X�V�@�\����^�Ȃ��ɂ��lin�ŕ\������s����␳����
//				if( CPrmSS[S_KOU][1] && LprnErrChk( 1 ) && prm_get( COM_PRM,S_KOU,26,1,1 ) ){
//					// �X�V��������ي֘A�װ����ōX�V�s��
//					grachr( 2, 0, 30, 1, COLOR_BLACK,
//							LCD_BLINK_OFF, OPE_CHR2[7] );			// " �����A������X�V�͏o���܂��� "���]�\��
//					grachr( 5, 0, 30, 0, COLOR_BLACK,
//							LCD_BLINK_OFF, OPE_CHR[6] );			// "              ���͂��ĉ�����  "
//					lin = 1;
//				}else{
//					lin = 0;
//				}
//#else
//				lin = 0;
//#endif	// UPDATE_A_PASS
//
//
//				set62 = (uchar)prm_get( COM_PRM,S_TYP,62,1,3 );		// ���ԏؖ������s�\�����擾
//				set63 = (uchar)prm_get( COM_PRM,S_TYP,63,1,1 );
//				if( set62 && set63 ){				 				// ��t��&�Ïؔԍ��o�^����?
//					grachr((ushort)(2+lin), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[3]);		// "�Ïؔԍ��o�^�A��t�����s�A    "
//				}
//				else if( set62 ){									// ��t������?
//					grachr((ushort)(2+lin), 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, &OPE_CHR[3][14]);	// "���ԏؖ������s�A              "
//				}
//				else if( set63 ){									// �Ïؔԍ��o�^����?
//					grachr((ushort)(2+lin), 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[3]);		// "�Ïؔԍ��o�^�A                "
//				}
//
//				grachr((ushort)(3+lin), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[5]);			// "���Z�̕���                    "
//
//				if( lin == 0 ){
//#if	UPDATE_A_PASS
//					if( CPrmSS[S_KOU][1] ){
//						grachr( 4, 0, 30, 0, COLOR_BLACK,
//								LCD_BLINK_OFF, OPE_CHR2[0] );		// "������X�V�̕��͍X�V�L�[��    "
//						grachr( 5, 0, 30, 0, COLOR_BLACK,
//								LCD_BLINK_OFF, OPE_CHR[6] );		// "              ���͂��ĉ�����  "
//					}else{
//						grachr( 4, 0, 30, 0, COLOR_BLACK,
//								LCD_BLINK_OFF, OPE_CHR[6] );		// "              ���͂��ĉ�����  "	
//					}
//#else
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[6]);					// "    �Ԏ��ԍ�����͂��ĉ�����  "
//#endif	// UPDATE_A_PASS
//				}
//
//			}
//
//			dspCyclicErrMsgRewrite();
//
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////			if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 && EDY_TEST_MODE ){	// Edy�ݒ肪���肩��ý�Ӱ�ސݒ�
////				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);				// "                   "
////				grachr(1, 6, 18, 1, COLOR_BLACK, LCD_BLINK_OFF, EdyTestModeStr[0]);			// " E�����e�X�g���[�h "
////			}
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			op_mod01_dsp();
//			break;
//
//		case 2:														// ���Ԉʒu�ԍ����͉�ʕ\��
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[8]);		// "�Ԏ��ԍ�����͂��ĉ�����      "
//			grawaku( 1, 4 );
//			bigcr(2, 2, 8, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "  �Ԏ��ԍ�                    "
//			lin = 7;
//			if ((prm_get(COM_PRM, S_TYP, 63, 1, 1)) && (OPECTL.Pay_mod == 0)) {			// �Ïؔԍ��o�^���聕�ʏ퐸�Z
//				grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[10]);	// "   �Ïؔԍ��̓o�^  ��  �o�^   "
//				lin--;
//			}
//			if (prm_get(COM_PRM, S_TYP, 62, 1, 3)) {									// ���ԏؖ������s�ݒ肠��
//				grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[11]);	// "  ���ԏؖ����̔��s ��  ����   "
//				lin--;
//			}
//			if (lin == 7) {
//				lin--;
//			}
//			grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[12]);		// "   ���ԗ����̐��Z  ��  ���Z   "
//			dsp_background_color(COLOR_WHITE);
//			teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��
//			bigcr(2, 14, 2, COLOR_FIREBRICK, LCD_BLINK_OFF, (uchar *)&OPE_ENG[key_sec*2]);	// ���\��
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////			if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 && EDY_TEST_MODE ){	// Edy�ݒ肪���肩��ý�Ӱ�ސݒ�
////				grachr(1, 6, 18, 1, COLOR_BLACK, LCD_BLINK_OFF, EdyTestModeStr[0]);			// " E�����e�X�g���[�h "
////			}
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			break;
//
//		case 3:														// �����\��(����)
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			bigcr(0, 0, 20, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[0]);	// "�����@�@�@�@�@�@�@�~"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "������������������������������"
//			feedsp(0, ryo_buf.dsp_ryo);
//			if( Ex_portFlag[EXPORT_CHGNEND] == 0 ){					// 0:�ޑK�؂�\������/1:�ޑK�؂�\���L��
//				/*** �ޑK�L�莞�\�� ***/
//				// �ڋq��� ���b�Z�[�W�\������
//				dspCyclicMsgInit();
//				for (i = 0; i < 3; i++) {
//					// ���ʃp�����[�^�������ɁA�P�s�\���֐��R�[�� �� �R��R�[��
//					msgno = (uchar)prm_get(COM_PRM, S_DSP, 4, 2, (char)((4-(i*2))+1));
//					dsp_msg(msgno, (uchar)(i+3), 6, COLOR_BLACK);
//				}
//			}else{
//				/*** �ޑK�������\�� ***/
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[56]);	// "�����ޑK���s�����Ă���܂�    "
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[60]);	// "�ޑK�̖����l�ɐ��Z���ĉ�����  "
//				msgno = 40;											// �ނ�؂ꎞ��T�J�[�h�g�p�ݒ��p
//				dsp_msg(msgno, 5, 6, COLOR_BLACK);					// "�s�J�[�h�Ń|�C���g�����܂�܂�"
//			}
//			grachr(6, 0, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "�Ԏ��ԍ�                      "
//			ryosyu = (uchar)(ryo_buf.syubet + 1);
//
//			if( (ryosyu >= 1) && (ryosyu <= 6 ) ){		//�`�`�e
//				r_syu = (uchar)(7 - ryosyu);			//
//				syasyu = (ushort)prm_get( 0,S_TYP,(short)(121),1,(char)r_syu );
//			}else if( (ryosyu >= 7) && (ryosyu <= 12 ) ){	//�f�`�k
//				r_syu = (uchar)(13 - ryosyu);				//
//				syasyu = (ushort)prm_get( 0,S_TYP,(short)(122),1,(char)r_syu );
//			}
//			
//			opedsp(6, 10, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			grachr(6, (ushort)(10+loklen*2), 10, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar*)Sya_Prn[syasyu]);	// "            �i�w�w�w�j        "
//
//			dsp_intime(7, OPECTL.Pr_LokNo);				// ���Ɏ����\��
//			dsp_background_color(COLOR_WHITE);
//			break;
//
//		case 4:														// �����\��(�������͊�����)
//			bigcr(0, 0, 20, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[1]);	// "���Ɓ@�@�@�@�@�@�@�~"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "������������������������������"
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[21] );				// "    ����                  �~  "
//			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[22] );				// "    ����                  �~  "
//			// 5�s�ڏ���(�󔒂�\��)
//			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[0]);
//			feedsp( 0, ryo_buf.zankin );
//			w_tax = 0L;
//			if(( CPrmSS[S_CAL][19] )&&( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2L )){	// �ŗ��ݒ肠�� & �O��?
//				w_tax = ryo_buf.tax;
//			}
//			if( ryo_buf.ryo_flg >= 2 ){								// ����g�p?
//				opedpl(3, 14, ryo_buf.tei_ryo + w_tax, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//			}else{
//				opedpl(3, 14, ryo_buf.tyu_ryo + w_tax, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//			}
//			opedpl(4, 14, ryo_buf.nyukin, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//
//			wkl = ryo_buf.waribik;									// ���Z���~���̗a����z�� waribik�ɓ���(TF7700)
//
//			if( 2 == CPrmSS[S_PRP][1] ){							// �񐔌��g�p�\
//				if(Flap_Sub_Flg == 1){
//					if( wkl >= (KaisuuWaribikiGoukei + PayData_Sub.pay_ryo) )
//						wkl -= (KaisuuWaribikiGoukei + PayData_Sub.pay_ryo);	// ryo_buf.waribik ���� �񐔌���������
//				}else{
//					if( KaisuuWaribikiGoukei ){						// �񐔌����p����i����j
//						if( wkl >= KaisuuWaribikiGoukei )
//							wkl -= KaisuuWaribikiGoukei;			// ryo_buf.waribik ���� �񐔌���������
//					}
//				}
//			}else if( 1 == CPrmSS[S_PRP][1] ){
//				if(Flap_Sub_Flg == 1){
//					if( PayData_Sub.pay_ryo ){							// �v���y�C�h���p����i���~�j
//						if( wkl >= PayData_Sub.pay_ryo )
//							wkl -= (PayData_Sub.pay_ryo);	// ryo_buf.waribik ���� �v���y�C�h��������
//					}
//				}
//			}
//
//			if( 0L != wkl ){										// ��������H
//				if(OPECTL.Pay_mod != 2){							// �C�����Z�ȊO
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[23]);	// "    ����                  �~  "
//				}else{												// �C�����Z�̎�
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[52]);	// "  ����(������)            �~  "
//				}
//				opedpl(5, 14, wkl, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//			}
//
//			if(Flap_Sub_Flg == 1){								//�Đ��Z��
//				if( (PayData_Sub.pay_ryo != 0) && (Ope_Last_Use_Card == 0)){ // �v���y�C�h�A�񐔌��ɂĐ��Z���~��
//					if( 1 == CPrmSS[S_PRP][1]){
//						wk_pri = PayData_Sub.pay_ryo + c_pay;
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,OPE_CHR[67]);	// "    �v���y�C�h            �~  "
//						opedpl(6, 14, (ulong)wk_pri, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//					}
//					else if( 2 == CPrmSS[S_PRP][1] ){						// �񐔌����p����H
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[72]);	// "    �񐔌�                �~  "
//						opedpl(6, 14, (KaisuuWaribikiGoukei+PayData_Sub.pay_ryo), 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//					}
//				}else{
//					if( e_pay || c_pay || KaisuuWaribikiGoukei || PayData_Sub.pay_ryo ){
//
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//						i = 16;
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
//						WkCal = (ulong)(e_pay + c_pay + PayData.ppc_chusi_ryo + KaisuuWaribikiGoukei + PayData_Sub.pay_ryo + ryo_buf.credit.pay_ryo);
//
//						switch( Ope_Last_Use_Card ){						// �Ō�Ɏg��������
//						case	1:		// "�v���y�C�h"
//							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[67]);		// "  �v���y�C�h            �~    "
//							break;
//						case	2:		// "�@�񐔌��@"
//							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[72] );		// "    �񐔌�              �~    "
//							break;
//						case	3:		// �d�q���σJ�[�h
//							wk_MsgNo = Ope_Disp_Media_GetMsgNo( 1, 0, 0 );
//							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "�d�q�}��                �~    "
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//							if (isEC_USE()) {
//							// �}���`�d�q�}�l�[�F�d�q�}�l�[�x���z�̂ݕ\��
//								WkCal = e_pay;
//								i = 14;
//							}
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
//							break;
//						case	4:		// "�O��x����"
//							break;
//						}
//						if (Ope_Last_Use_Card != 6) {
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
////							opedpl(6, 16, WkCal, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//							opedpl(6, (ushort)i, WkCal, (i == 16)? 5 : 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
//						}
//						grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);			// "                              "
//					}
//				}
//			}else{
//				/** �v���y�^�񐔌��^�d�q���σJ�[�h�^�O��x���� �\�� **/
//				if( e_pay || c_pay || KaisuuWaribikiGoukei ){
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//					i = 16;
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
//					WkCal = (ulong)(e_pay + c_pay + PayData.ppc_chusi_ryo + KaisuuWaribikiGoukei);
//					switch (Ope_Last_Use_Card) {						// �Ō�Ɏg��������
//					case 1:		// "�v���y�C�h"
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[67]);				// "  �v���y�C�h            �~    "
//						break;
//					case 2:		// "�@�񐔌��@"
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[72]);				// "    �񐔌�              �~    "
//						break;
//					case 3:		// �d�q���σJ�[�h
//						wk_MsgNo = Ope_Disp_Media_GetMsgNo( 1, 0, 0 );
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);			// "�d�q�}��                �~    "
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//						if (isEC_USE()) {
//						// �}���`�d�q�}�l�[�F�d�q�}�l�[�x���z�̂ݕ\��
//							WkCal = e_pay;
//							i = 14;
//						}
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
//						break;
//					case 4:		// "�O��x����"
//						break;
//					}
//					if (Ope_Last_Use_Card != 6) {
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
////						opedpl(6, 16, WkCal, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//						opedpl(6, (ushort)i, WkCal, (i == 16)? 5 : 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
//					}
//					grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);					// "                              "
//				}
//			}
//
//			if( OPECTL.PassNearEnd == 1 ){							// �����؂�ԋ�
//				grachr(7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[10] );						// "���̶��ނ͂������������؂�ł�"
//			}else{
//				Lcd_Receipt_disp();
//			}
//			if( (OPECTL.Pay_mod == 2)&&(prm_get(COM_PRM, S_TYP, 98, 1, 3)) ){	// �C�����Z�ł��������Z����������
//				grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[48] );				// "    �������Z����   ��  ����   "
//			}
//			break;
//
//		case 5:														// ���Z�����\��
//			bigcr(0, 0, 20, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[3]);				// "����@�@�@�@�@�@�~"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2] );							// "������������������������������"
//			if( (OPECTL.PriUsed==1) && (CPrmSS[S_PRP][1] == 1) ){	// ����߲�޶��ގg�p�H�i�񐔌��g�p����PriUsed=1�ɂȂ��Ă���̂Őݒ������j
//				if ( 99 == prm_get( COM_PRM,S_PRP,11,2,1 ) ){
//					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[69]);					// "�v���y�C�h�c�z            �~  "
//					opedpl(6, 16, OPECTL.LastUsePCardBal, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//				}else{
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[69]);					// "�v���y�C�h�c�z            �~  "
//					opedpl(5, 16, OPECTL.LastUsePCardBal, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)PrcKigenStr );			// "�L�������Q�O�P�P�N�@�V���Q�O��"
//				}
//			}
//			// �ڋq��� ���b�Z�[�W�\������ (4,5,6�s�ڂ�ݒ�l�ŕ\��)
//			dspCyclicMsgInit();
//// GG119202(S) iD���ώ��̈ē��\���Ή�
//			if (e_inflg != 0 &&
//				Ec_Settlement_Res.brand_no == BRANDNO_ID &&
//				PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin != 0 &&
//				PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0) {
//				// iD���ςł݂Ȃ����ρ{����OK��M�̏ꍇ
//				grachr( 4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[164] );		// " �G���[�����ł����Ԃ𒸂����� "
//				grachr( 5, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[165] );		// "       �x�������Ƃ��܂�       "
//				grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[156] );		// "�d�q�}�l�[�x��             �~ "
//				opedpl( 6, 15, (ulong)e_pay, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//			}
//			else {
//// GG119202(E) iD���ώ��̈ē��\���Ή�
//			for (i = 0; i < 3; i++) {
//				if( (OPECTL.PriUsed==1) && (CPrmSS[S_PRP][1] == 1) ){
//					if ( (i == 2) && (99 != prm_get( COM_PRM,S_PRP,11,2,1 )) ){
//						break;	// 6�s�ڂ� �L�������Ŏg�p //
//					}
//				}
//				// ���ʃp�����[�^�������ɁA�P�s�\���֐��R�[�� �� �R��R�[��
//				msgno = (uchar)prm_get(COM_PRM, S_DSP,  1, 2, (char)((4-(i*2))+1));
//				dsp_msg(msgno, (uchar)(i+3), 15, COLOR_BLACK);
//			}
//			if( OPECTL.Pay_mod == 2 ){								// �C�����Z�̎�
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[53] );						// "    �C���������������܂���    "
//			}
//			if( e_inflg != 0 ){										// Suica�g�p
//				wk_MsgNo = Ope_Disp_Media_GetMsgNo( 1, 1, 0 );
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
//				if (wk_MsgNo == 156) {								// ���σ��[�_�ł̐��Z�̏ꍇ
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);				// "�w�w�w�w�w�x��             �~ "
//					opedpl(5, 15, (ulong)e_pay, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[157]);					// "          �c��             �~ "
//					opedpl(6, 15, (ulong)e_zandaka, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//				}
//				else {
//// MH321800(E) hosoda IC�N���W�b�g�Ή�
//				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);					// "�w�w�w�w�w���p�z           �~ "
//				opedpl(5, 17, (ulong)e_pay, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////				if( wk_MsgNo == 109 ){								// Edy�ł̐��Z�̏ꍇ
////					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[114]);					// "          �c��             �~ "
////				} else {
//				{
//// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[95]);					// "            �c�z           �~ "
//				}
//				opedpl(6, 17, (ulong)e_zandaka, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
//				}
//// MH321800(E) hosoda IC�N���W�b�g�Ή�
//// GG119202(S) iD���ώ��̈ē��\���Ή�
//			}
//// GG119202(E) iD���ώ��̈ē��\���Ή�
//			}
//
//			if( OPECTL.PassNearEnd == 1 ){							// �����؂�ԋ�
//				grachr(7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[10]);							// "���̶��ނ͂������������؂�ł�"
//			}else{
//				Lcd_Receipt_disp();
//			}
//			WkCal = ryo_buf.turisen;
//			if( OPECTL.Pay_mod == 2 ){								// �C�����Z��
//				if(	(prm_get(COM_PRM, S_TYP, 98, 1, 1)==1)||		// �C�������������z�̕��߂���ݒ�
//					(Syusei_Select_Flg == 2) ){						// �C�������������z�̕��߂���I��
//						if( ryo_buf.mis_tyu ){						// �C���������z�̕��ߊz����
//							WkCal = ryo_buf.mis_tyu;
//						}
//				}
//			}
//// �d�l�ύX(S) K.Onodera 2016/10/28 �U�։ߕ������ƒޑK�؂蕪��
//			// �U�։ߕ���������H
//			if( ryo_buf.kabarai ){
//				WkCal = ryo_buf.kabarai;
//			}
//// �d�l�ύX(E) K.Onodera 2016/10/28 �U�։ߕ������ƒޑK�؂蕪��
//			feedsp( 0, WkCal );
//			break;
//
//		case 6:														// ��t�����sOK�\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			bigcr(1, 0, 20, COLOR_BLACK, LCD_BLINK_OFF, (uchar *)BIG_CHR[4]);	// "���ԏؖ������@�@�@�@"
//			bigcr(3, 0, 20, COLOR_BLACK, LCD_BLINK_OFF, (uchar *)BIG_CHR[5]);	// "�@�@�@�@����艺����"
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      �Ԏ��ԍ�                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//
//		case 7:														// ��t�����s�ς�NG�\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[54]);			// "  ���̎Ԏ��ԍ��̒��ԏؖ�����  "
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[55]);			// "  ���s���x�����𒴂��Ă��܂�  "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "      �Ԏ��ԍ�                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//
//		case 8:														// �߽ܰ�ޓo�^��ʕ\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,OPE_CHR[37]);			// "�Ïؔԍ����S���œ��͂��ĉ�����"
//			grachr(1, 2, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "  �Ԏ��ԍ�                    "
//			grawaku( 2, 5 );										// ����g�\��
//			bigcr(3, 2, 8, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[37]);		// "  �Ïؔԍ�                    "
//			set62 = (uchar)prm_get( COM_PRM,S_TYP,62,1,3 );			// ���ԏؖ������s�\�����擾
//			lin = 6;
//			if( set62 ){											// ��t�����s�@�\����?
//				pri_time_flg = pri_time_chk(OPECTL.Pr_LokNo,0);
//				if( pri_time_flg != 0 ){
//					grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[11]);	// "  ���ԏؖ����̔��s ��  ����   "
//					lin++;
//				}
//			}
//			grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[10]);			// "   �Ïؔԍ��̓o�^  ��  �o�^   "
//			teninb_pass(0, 4, 3, 16, 0, COLOR_FIREBRICK);
//			opedsp(1, 12, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//
//		case 9:														// �Ïؔԍ����͉�ʕ\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr( 1, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[41] );					// "  �o�^���ꂽ�Ïؔԍ���        "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[145] );					// "              ���͂��ĉ�����  "
//			grachr(4, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "      �Ԏ��ԍ�                "
//			opedsp(4, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[4]);			// "      �Ïؔԍ�  ��������      "
//			break;
//
//		case 10:													// �C�����Z���Ԉʒu�ԍ����͉��
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "���C�����Z��                  "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[43] );					// "�ԈႦ���Ԏ��ԍ�              "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[44] );					// "�������Ԏ��ԍ�                "
//			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[45]);	// " �ԍ����͌㐸�Z�������ĉ����� "
//			if( CPrmSS[S_TYP][81] ){								// ���g�p�ݒ�L��?
//				grachr( 2, 20, 2, 1, COLOR_BLACK,
//						LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );	// ���\��
//				opedsp( 2, 22, 0, knum_len, 1, 1,
//						COLOR_BLACK, LCD_BLINK_OFF );			// ���Ԉʒu�ԍ�0�\��
//				grachr( 3, 20, 2, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, &OPE_ENG[ 1 * 2 ] );			// ���\��
//				opedsp( 3, 22, 0, knum_len, 1, 0,
//						COLOR_BLACK, LCD_BLINK_OFF );			// ���Ԉʒu�ԍ�0�\��
//			}else{
//				opedsp( 2, 22, 0, knum_len, 1, 1,
//						COLOR_BLACK, LCD_BLINK_OFF );			// ���Ԉʒu�ԍ�0�\��
//				opedsp( 3, 22, 0, knum_len, 1, 0,
//						COLOR_BLACK, LCD_BLINK_OFF );			// ���Ԉʒu�ԍ�0�\��
//			}
//			dspclk(7, COLOR_DARKSLATEBLUE);							// ���v�\��
//			break;
//
//		case 11:													// �C�����Z�����\��(�၄��)
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			Mloklen = binlen( OPECTL.MOp_LokNo % 10000L );
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );										// "���C�����Z��                  "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[44] );										// "�������Ԏ��ԍ�                "
//			if( Syusei_Select_Flg == 1 ){
//				grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[49]);	// "     �d�݂Ŗ߂�    ��  ���Z   "
//				grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[50]);	// "      �߂��Ȃ�     ��  �̎�   "
//			}else{
//				grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[132]);	// "   �m�F�㐸�Z�������ĉ�����   "
//			}
//			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[43]);				// "�ԈႦ���Ԏ��ԍ�              "
//			grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[131]);		// "���Z      ��    ��     �F     "
//			grachr(7, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[133]);		// "���Z�ǔ�      No.�O�O�O�O�O�O "
//
//			opedsp(2, (ushort)(28-loklen*2), (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			opedsp(5, (ushort)(28-Mloklen*2), (ushort)(OPECTL.MOp_LokNo % 10000L), Mloklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//
//			opedsp(6, 6, (ushort)syusei[OPECTL.MPr_LokNo-1].omont, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
//			opedsp(6, 12, (ushort)syusei[OPECTL.MPr_LokNo-1].odate, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
//			opedsp(6, 19, (ushort)syusei[OPECTL.MPr_LokNo-1].ohour, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
//			opedsp(6, 25, (ushort)syusei[OPECTL.MPr_LokNo-1].ominu, 2, 1, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// ���\��
//
//			opedpl(7,17, (ulong)CountSel(&syusei[OPECTL.MPr_LokNo-1].oiban), 6, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				// �ǔԕ\��
//			break;
//
//#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
//		case 12:													// �C�����Z�����\��(�၃��)
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			Mloklen = binlen( OPECTL.MOp_LokNo % 10000L );
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "���C�����Z��                  "
//			grachr( 1, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[43] );					// "�ԈႦ���Ԏ��ԍ�              "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[21] );					// "    ����                  �~  "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[44] );					// "�������Ԏ��ԍ�                "
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[21] );					// "    ����                  �~  "
//			grachr( 5, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[22] );					// "    ����                  �~  "
//			grachr( 6, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[51] );					// "    ����                  �~  "
//			grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);	// " �K�v�ȕ��͗̎��������ĉ����� "
//
//			grachr( 1, 20, 2, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, &OPE_ENG[ ( OPECTL.MOp_LokNo / 10000L )* 2 ] );
//			opedsp( 1, 22, (ushort)( OPECTL.MOp_LokNo % 10000L ), Mloklen, 1, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 2, 14, ryo_buf.mis_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// �ԈႦ�������\��
//			grachr( 3, 20, 2, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, &OPE_ENG[ ( OPECTL.Op_LokNo / 10000L )* 2 ] );
//			opedsp( 3, 22, (ushort)( OPECTL.Op_LokNo % 10000L ), loklen, 1, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 4, 14, ryo_buf.dsp_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// �����������\��
//			opedpl( 5, 14, ryo_buf.nyukin, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// �����z�\��
//			opedpl( 6, 14, ryo_buf.dsp_ryo - ryo_buf.mis_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// �c�z�\��
//			break;
//
//		case 13:													// �C�����Z�����\��(���z)
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			Mloklen = binlen( OPECTL.MOp_LokNo % 10000L );
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "���C�����Z��                  "
//			grachr( 1, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[43] );					// "�ԈႦ���Ԏ��ԍ�              "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[21] );					// "    ����                  �~  "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[44] );					// "�������Ԏ��ԍ�                "
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[21] );					// "    ����                  �~  "
//			grachr( 5, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[52] );					// "�ԈႦ���ԍ��̗����Ɠ��z�ł�  "
//			grachr( 7, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[53] );					// "    �C�������͊������܂���    "
//
//			grachr( 1, 20, 2, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, &OPE_ENG[ ( OPECTL.MOp_LokNo / 10000L )* 2 ] );
//			opedsp( 1, 22, (ushort)( OPECTL.MOp_LokNo % 10000L ), Mloklen, 1, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 2, 14, ryo_buf.mis_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			grachr( 3, 20, 2, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, &OPE_ENG[ ( OPECTL.Op_LokNo / 10000L )* 2 ] );
//			opedsp( 3, 22, (ushort)( OPECTL.Op_LokNo % 10000L ), loklen, 1, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 4, 14, ryo_buf.dsp_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			break;
//
//		case 14:													// �C�����Z�����\��(�ޖ���)
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "���C�����Z��                  "
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[53] );					// "    �C�������͊������܂���    "
//			break;
//
//		case 15:													// �C�����Z�����\��(�ޗL��)
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "���C�����Z��                  "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[16] );					// "����                      �~"
//			feedsp( 1, ryo_buf.turisen );
//			grachr( 6, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[53] );					// "    �C�������͊������܂���    "
//			break;
//#endif		// SYUSEI_PAYMENT
//
//		case 16:													// �C�����Z�����\��(�������͊�����)
//			opedpl( 4, 16, ryo_buf.tyu_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 5, 16, ryo_buf.nyukin, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 6, 16, ryo_buf.zankin, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			if( OPECTL.RECI_SW == 1 ){								// �̎������ݎg�p?
//				wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
//				grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      �̎��؂𔭍s���܂�      "
//			}else{
//				grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);		// " �K�v�ȕ��͗̎��������ĉ����� "
//			}
//			break;
//
//		case 17:													// �x�ƒ���ʕ\��
//			/****************************************************/
//			/* CLOSE_stat = 0:�c�ƒ�							*/
//			/*              1:�����c�x��(I)						*/
//			/*              2:�c�Ǝ��ԊO(�����A�c�Ǝ��ԊO�ł�)	*/
//			/*              3:�������ɖ��t(N)					*/
//			/*              4:��݋��ɖ��t(C)					*/
//			/*              5:����ذ�ް�E��(F)					*/
//			/*              6:�ޑK�؂�(E)						*/
//			/*              7:���ү��ʐM�s��(T)					*/
//			/*              8:NT-NET ���M�ޯ̧FULL(NT)			*/
//			/*              9:�ׯ��/ۯ����u�蓮Ӱ��(�蓮���[�h���ł�)	*/
//			/*             10:�M���ŋx�Ǝw��					*/
//			/*             11:�ʐM�ŋx�Ǝw��					*/
//			/****************************************************/
//			/* �x�Ɨ��R�\������^���Ȃ� */
//			if( (0 == (uchar)prm_get(COM_PRM, S_DSP, 35, 1, 1)) ||	// �x�ƕ\������ or 
//				(2 == CLOSE_stat) ){								// �c�Ǝ��ԊO(����͏�ɕ\��)
//				grachr(0, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, CLSMSG[CLOSE_stat]);	// �x�Ɨ��R�\��
//			}
//			bigcr(2, 0, 26, COLOR_WHITE, LCD_BLINK_OFF, (uchar *)&BIG_OPE[0]);		// "      �x     ��     ��    "
//			dspclk(7, COLOR_WHITE);							// ���v�\��
//			// �ڋq��� ���b�Z�[�W�\������
//			dspCyclicMsgInit();
//			for (i = 0; i < 3; i++) {
//				// ���ʃp�����[�^�������ɁA�P�s�\���֐��R�[�� �� �R��R�[��
//				msgno = (uchar)prm_get(COM_PRM, S_DSP, 18, 2, (char)((4-(i*2))+1));
//				dsp_msg(msgno, (uchar)(i+4), 18, COLOR_BLACK);
//			}
//			break;
//
//		case 18:													// �ޱ���̱װѕ\��
//			/*** �װѕ\���͍ő�4�܂ŕ\�� ***/
//			lin = 2;
//			bigcr(0, 0, 20,COLOR_RED, LCD_BLINK_OFF, (uchar *)BIG_CHR[6]);				// "�@�@�@�x�@�@���@�@�@"
//			if( OPE_SIG_DOOR_Is_OPEN == 0 ){						// �ޱ��?
//				if (RAMCLR) {										// �������C�j�V�������{�H
//					grachr(lin++, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, SMSTR2[21]);		// "�@ ���Z�@�̍ċN�����K�v�ł� �@"
//					grachr(lin++, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, SMSTR2[22]);		// "�@�@ �d����OFF/ON���s���A �@�@"
//					grachr(lin++, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, SMSTR2[23]);		// "�@���Z�@���ċN�����Ă��������@"
//					break;
//				}
//				if(prm_get(COM_PRM, S_PAY, 33, 1, 1) == 1) {		// �h�A���̃R�C�����Ɍx������
//					if( FNT_CN_BOX_SW ){								// ��݋��ɾ��?
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[0]);	// "��݋��ɂ�Ă��ĉ�����        "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 33, 1, 2) == 1) {		// �h�A���̃R�C���J�Z�b�g�x������
//					if( CN_RDAT.r_dat0c[1] & 0x40 ){					// ��ݶ��ĊJ
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[1]);	// "��ݶ��Ă�Ă��ĉ�����        "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 33, 1, 3) == 1) {		// �h�A���̃R�C�����b�N���x������
//					if( FNT_CN_DRSW ){									// ���ү��ޱ�����J(���ү��ޱ���J)?
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[2]);	// "���ү��̌���߂ĉ�����      "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 33, 1, 5) == 1) {		// �h�A���̎������[�_�[�x������
//					if( NT_RDAT.r_dat1b[0] & 0x02 ){					// ������J?
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[3]);	// "����ذ�ް������߂ĉ�����   "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 33, 1, 6) == 1) {		// �h�A���̎������Ɍ��x������
//					if( FNT_NT_BOX_SW ){								// �������ɒE�������J?
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[4]);	// "����ذ�ް�̌���߂ĉ�����   "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 34, 1, 1) == 1) {		// �h�A���̃��V�[�g�v�����^�x������
//					if( 0 != ALM_CHK[ALMMDL_SUB][ALARM_RPAPEROPEN] ){	// ڼ��������W�J
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[5]);	// "ڼ�������������߂ĉ�����   "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 34, 1, 3) == 1 &&		// �h�A���̃W���[�i���v�����^�x������
//					prm_get(COM_PRM, S_PAY, 21, 1, 1) == 1) {		// �W���[�i���v�����^����
//					if( 0 != ALM_CHK[ALMMDL_SUB][ALARM_JPAPEROPEN] ){	// �ެ���������W�J
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[6]);	// "�ެ��������������߂ĉ����� "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				break;
//			}
//			break;
//
//		case 19:													// �߽ܰ�ޓo�^�ς݉�ʕ\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[36] );					// "���̎Ԏ��ԍ��̈Ïؔԍ���      "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[24] );								// "                  �o�^�ς݂ł�"
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      �Ԏ��ԍ�                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//
//#if	UPDATE_A_PASS
//		case 20:													// ������X�V ���}���҂���ʕ\��
//			dspclk(0 COLOR_DARKSLATEBLUE);							// ���v�\��
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[1] );					// "�X�V����������}�����ĉ�����"
//			grachr( 5, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[2] );					// "�\�����ꂽ�����𐸎Z�����    "
//			grachr( 6, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[3] );					// "          ��������X�V����܂�"
//			break;
//
//		case 21:													// ������X�V �����\��(����)
//			bigcr(0, 0, 30, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[0]);	// "����                        �~"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "������������������������������"
//			if( Ex_portFlag[EXPORT_CHGNEND] == 0 ){					// 0:�ޑK�؂�\������/1:�ޑK�؂�\���L��
//				/*** �ޑK�L�莞�\�� ***/
//				grachr( 4, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR[18] );				// "�d�݁A���͎����Ő��Z���ĉ�����"
//			}else{
//				/*** �ޑK�������\�� ***/
//				grachr( 3, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR[56] );				// "�����ޑK���s�����Ă���܂�    "
//				grachr( 4, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR[60] );				// "�ޑK�̖����l�ɐ��Z���ĉ�����  "
//			}
//			feedsp( 0, ryo_buf.dsp_ryo );
//			grachr( 6, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[4] );					// "    �����X�V��̏I������      "
//			opedsp( 6, 0, RenewalMonth, 2, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// �����\��
//			grachr( 7, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[5] );					// "            �N    ��    ���ł�"
//			opedsp( 7, 4, (ushort)PayData.teiki.e_year, 4, 1, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// �N�\��
//			opedsp( 7, 14, (ushort)PayData.teiki.e_mon, 2, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// ���\��
//			opedsp( 7, 20, (ushort)PayData.teiki.e_day, 2, 0, 0 );	// ���\��
//			break;
//
//		case 23:													// ������X�V ���Z�����\��
//			bigcr(0, 0, 30, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[3] );	// "����                      �~"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "������������������������������"
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[25] );					// " �����p���肪�Ƃ��������܂��� "
//			if( prm_get( COM_PRM,S_PAY,25,1,2 ) ){					// ���ِݒ肠��?
//				grachr( 5, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR2[9] );				// " ������A���x��������艺���� "
//			}else{
//				grachr( 5, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR2[6] );				// "�X�V���ꂽ�����������艺����"
//			}
//
//			if( OPECTL.RECI_SW == 1 ){								// �̎������ݎg�p?
//				wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
//				grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "     �̎��؂�����艺����     "
//			}else{
//				grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);		// " �K�v�ȕ��͗̎��������ĉ����� "
//			}
//			feedsp( 0, ryo_buf.turisen );
//			break;
//#endif	// UPDATE_A_PASS
//
//		case 24:															// �Ïؔԍ��o�ɑ��� �Ԏ����͉��
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[8]);	// "�Ԏ��ԍ�����͂��ĉ�����      "
//			grawaku( 1, 4 );										// ����g�\��
//			bigcr(2, 2, 8, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);			// "  �Ԏ��ԍ�                    "
//			grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR3[0]);	// "      �Ԏ��ԍ�     ��  ���Z   "
//			dsp_background_color(COLOR_WHITE);
//			teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��
//			break;
//
//		case 25:																	// �Ïؔԍ��o�ɑ��� �Ïؔԍ����͉��
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[81]);			// "�o�ɔԍ����S���œ��͂��ĉ�����"
//			grachr(1, 0, 10, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, ERR_CHR[6]);	// "  �Ԏ��ԍ�                    "
//			grawaku( 2, 5 );										// ����g�\��
//			bigcr(3, 2, 8, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[81]);		// "  �o�ɔԍ�                    "
//			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[11]);			// "���q�l�R�[�h                      "
//			grachr(7, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR3[12]);	// "    �����ԍ��o��   ��  ���Z   "
//			teninb_pass(0, 4, 3, 16, 0, COLOR_FIREBRICK);
//			break;
//		case 26:													// ��t�����s�ς�NG�\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[79] );								// "�Ïؔԍ��o�^���Ԃ��߂��Ă��܂�"
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      �Ԏ��ԍ�                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		case 27:													// ��t�����s�ς�NG�\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[80]);			// " ���ԏؔ��s���Ԃ��߂��Ă��܂� "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "      �Ԏ��ԍ�                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		case 28:													// 
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[138] );					// "      �����A���ԏؖ�����      "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[139] );					// "      ���s���ł��܂���        "
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, shomei_errstr[shomei_errnum] );	// "          (���؂�)            "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      �Ԏ��ԍ�                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		case 29:													// ��t�����s�s�\��
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[138] );					// "      �����A���ԏؖ�����      "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[139] );					// "      ���s���ł��܂���        "
//			break;
//		case 30:													// ���O�^�C�������ē���ʕ\���i�t���b�v�j
//			dspclk(0, COLOR_DARKSLATEBLUE);							// ���v�\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[151]);		// "   �o�ɂł��܂�               "
//			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[153]);		// "     ���肪�Ƃ��������܂���   "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      �Ԏ��ԍ�                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		case 31:													// ���O�^�C�������ē���ʕ\���i�t���b�v�j
//			dspclk(0, COLOR_DARKSLATEBLUE);							// ���v�\��
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[152]);		// "   ���Z�ς݂ł�               "
//			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[153]);		// "     ���肪�Ƃ��������܂���   "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      �Ԏ��ԍ�                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		default:
//			break;
//	}
//	return;
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ��݉�ʕ\�� �N���W�b�g�Ή� 4-6�s�ڃ��b�Z�[�W�\��������            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspCyclicMsgInit                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	dspCyclicMsgInit(void)
{
	memset( OPECTL.op_mod02_dispCnt,0,sizeof( OPECTL.op_mod02_dispCnt ));// ���p�\�}�́F�擪���w��
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ��݉�ʕ\�� �N���W�b�g�Ή� �T�C�N���b�N�\���L���擾�֐�(�O���p)   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspIsCyclicMsg                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	dspIsCyclicMsg(void)
{
	uchar msgno, i;

	// ���ݒނ�K�؂�\�����Ȃ�T�C�N���b�N�\���Ȃ�
	if (Ex_portFlag[EXPORT_CHGNEND] != 0) {
		return 0;
	}

	for (i = 0; i < 3; i++) {
		msgno = (uchar)prm_get(COM_PRM, S_DSP, 4, 2, (char)((4-(i*2))+1));
		if ((msgno == CYCLIC_DSP01) || (msgno == CYCLIC_DSP09)) {
			return 1;	// �T�C�N���b�N�\������
		}
	}
	return 0;	// �T�C�N���b�N�\���Ȃ�
}


/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ��݉�ʕ\�� �N���W�b�g�Ή� 4-6�s�ڃ��b�Z�[�W�\���ؑւ��`�F�b�N    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspCyclicMsgRewrite                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	dspCyclicMsgRewrite(uchar tik_add)
{
	uchar msgno, i,line = 0;
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
	short	line_max;

	if (LCDNO == 5) {
		// ���Z������ʂœd�q�}�l�[��v���y�C�h�J�[�h������ꍇ�A
		// �����̏��͏��������ł��Ȃ��̂ŃT�C�N���b�N�\���s��������������
		line_max = 3;
		if (e_pay || c_pay) {		// �d�q�}�l�[�g�p�A�v���y�C�h�J�[�h�g�p
			line_max = 2;			// �d�q�}�l�[���g�p�����ꍇ��6�s�ڂ̐ݒ�͖�������(�ݒ�\�Q��)
		}
		for (i = 0; i < line_max; i++) {
			msgno = (uchar)prm_get(COM_PRM, S_DSP,(short)tik_add, 2, (char)((4-(i*2))+1));
			if (msgno >= 21 && msgno <= 25) {					// ���o�s�����̃��b�Z�[�W�\������
				dsp_msg(msgno, (uchar)(i+3), tik_add, COLOR_BLACK);
			}
		}
		return;
	}
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)

	// ���ݒނ�K�؂�\�����Ȃ牽�����Ȃ�
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
//	�����ē���ʂł�؂�\�����s�����߁A����ʂł͂�؂ꂪ����Ή������Ȃ�
//-	if (Ex_portFlag[EXPORT_CHGNEND] != 0) {
	if (LCDNO == 3 && Ex_portFlag[EXPORT_CHGNEND] != 0) {
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
		return;
	}
	if(tik_add == 18){			//�x�ƒ��\���͂T�`�V�s�ڂ�I��
		line = 1;
	}

	// �T�C�N���b�N�\���̗L���`�F�b�N
	for (i = 0; i < 3; i++) {
		// ���ʃp�����[�^�������ɁA�P�s�\���֐��R�[�� �� �R��R�[��
		msgno = (uchar)prm_get(COM_PRM, S_DSP,(short)tik_add, 2, (char)((4-(i*2))+1));
		if ((msgno == CYCLIC_DSP01) || (msgno == CYCLIC_DSP09)) {
			// �T�C�N���b�N�\���X�V
			dsp_cyclic_msg((uchar)(line+(i+3)),msgno,tik_add);
		}
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
//-		else
//-		if ((OPECTL.Ope_mod == 3)
//-		&&	(mode_Lagtim10 == 0)) {
//-		// ���Z�����\���ݒ�i24-0001�j�̕\�����ĕ\��
//-			dsp_msg(msgno, (uchar)(i+3), tik_add, COLOR_BLACK);
//-		}
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ��݉�ʕ\�� �ҋ@��8�s�ڒޑK�A�p���s�����b�Z�[�W�\���ؑւ��`�F�b�N |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspCyclicErrMsgRewrite                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-05-14                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	dspCyclicErrMsgRewrite(void)
{
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
//	ushort	lin[3];
//	ushort	i = 0;
//
//	memset(lin, 0, sizeof(lin));
//
// ���Z��Ԓʒm�̏�������Ɛ��Z��Ԃ̒ʒm����
//	if(( Ex_portFlag[EXPORT_CHGNEND] != 0 )||				// 0:�ޑK�؂�\������/1:�ޑK�؂�\���L��
//	   ( Ex_portFlag[EXPORT_CHGEND] != 0 )){				// 0:�ޑK�؂ꖳ��/1:�ޑK�؂�L��
//		/*** �ޑK�������\�� ***/
//		lin[i] = 62;										// "�@�ޑK�s��                    "
//		i++;
//	}
//	if(( Ex_portFlag[EXPORT_R_PNEND] != 0 )||				// 0:ڼ�Ď��؂�\������/1:ڼ�Ď��؂�\���L��
//	   ( Ex_portFlag[EXPORT_R_PEND] != 0 )){				// 0:ڼ�Ď��؂ꖳ��/1:ڼ�Ď��؂�L��
//		/*** ڼ�Ď��؂ꎞ�\�� ***/
//		lin[i] = 63;										// "�@���V�[�g�p���s��            "
//		i++;
//	}
//	if(( Ex_portFlag[EXPORT_J_PNEND] != 0 )||				// 0:�ެ��َ��؂�\������/1:�ެ��َ��؂�\���L��
//	   ( Ex_portFlag[EXPORT_J_PEND] != 0 )){				// 0:�ެ��َ��؂ꖳ��/1:�ެ��َ��؂�L��
//		/*** �ެ��َ��؂ꎞ�\�� ***/
//		lin[i] = 64;										// "�@�W���[�i���p���s��          "
//		i++;
//	}
//	if(( dspErrMsg != lin[0] ) && ( dspErrMsg != lin[1] ) && ( dspErrMsg != lin[2] )){	// �G���[�����̌���
//		dspErrMsg = 0;
//	}
//	if(i != 0){												// �G���[���b�Z�[�W�����݂���ꍇ
//		if(dspErrMsg == lin[i-1]){
//			dspErrMsg = 0;
//		}
//		for( i = 0; i < 3; i++ ){								// ���ݕ\�����̃��b�Z�[�W����̕������݂��邩
//			if(dspErrMsg < lin[i]){
//				/*** �G���[���b�Z�[�W�\�� ***/
//				grachr(7, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[lin[i]]);	// "�G���[���b�Z�[�W"
//				dspErrMsg = lin[i];
//				break;
//			}
//		}
//	}
// MH810100(S) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
//	static ulong old_pay_status = 0;
// MH810100(E) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
	ulong status = 0;

	if(( Ex_portFlag[EXPORT_CHGNEND] != 0 )||				// 0:�ޑK�؂�\������/1:�ޑK�؂�\���L��
	   ( Ex_portFlag[EXPORT_CHGEND] != 0 )){				// 0:�ޑK�؂ꖳ��/1:�ޑK�؂�L��
		// �ޑK�������\��
		status |= 0x01;										// ���Z��Ԓʒm		b0:		�ޑK�s��
	}
// MH810100(S) S.Nishimoto 2020/08/17 #4592 �Ԕԃ`�P�b�g���X(���V�[�g�v�����^�ʐM�s�ǃ`�F�b�N�ǉ�)
	if (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM)) {
		status |= 0x40000;										// ���Z��Ԓʒm	b18:	ڼ��������ʐM�s��
	}
// MH810100(E) S.Nishimoto 2020/08/17 #4592 �Ԕԃ`�P�b�g���X(���V�[�g�v�����^�ʐM�s�ǃ`�F�b�N�ǉ�)
// MH810100(S) K.Onodera 2020/04/09 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
//	if(( Ex_portFlag[EXPORT_R_PNEND] != 0 )||				// 0:ڼ�Ď��؂�\������/1:ڼ�Ď��؂�\���L��
//	   ( Ex_portFlag[EXPORT_R_PEND] != 0 )){				// 0:ڼ�Ď��؂ꖳ��/1:ڼ�Ď��؂�L��
	if( Ex_portFlag[EXPORT_R_PNEND] != 0 ){					// 0:ڼ�Ď��؂�\������/1:ڼ�Ď��؂�\���L��
		status |= 0x10000;									// ���Z��Ԓʒm		b16:		ڼ�ėp���؂�\��
	}
	if( Ex_portFlag[EXPORT_R_PEND] != 0 ){					// 0:ڼ�Ď��؂ꖳ��/1:ڼ�Ď��؂�L��
// MH810100(E) K.Onodera 2020/04/09 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
		// ڼ�Ď��؂ꎞ�\��
		status |= 0x02;										// ���Z��Ԓʒm		b1:		ڼ�ėp���s��
	}
// GG129000(S) M.Fujikawa 2023/09/22 #6953 �ݒ�01-0016=0�ŃW���[�i���ʐM�s�ǔ������ɗ̎��ؔ��s�Ɏ��s���Ă��A�̎��ؔ��s���s�̃|�b�v�A�b�v���������Ȃ�
	if ( !Ope_isJPrinterReady() ) {
		status |= 0x100000;									// ���Z��Ԓʒm		b20:	�ެ���������ʐM�s��
	}
// GG129000(E) M.Fujikawa 2023/09/22 #6953 �ݒ�01-0016=0�ŃW���[�i���ʐM�s�ǔ������ɗ̎��ؔ��s�Ɏ��s���Ă��A�̎��ؔ��s���s�̃|�b�v�A�b�v���������Ȃ�
// MH810100(S) K.Onodera 2020/04/09 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
//	if(( Ex_portFlag[EXPORT_J_PNEND] != 0 )||				// 0:�ެ��َ��؂�\������/1:�ެ��َ��؂�\���L��
//	   ( Ex_portFlag[EXPORT_J_PEND] != 0 )){				// 0:�ެ��َ��؂ꖳ��/1:�ެ��َ��؂�L��
	if( Ex_portFlag[EXPORT_J_PNEND] != 0 ){
		status |= 0x20000;									// ���Z��Ԓʒm		b17:	�ެ��ٗp���؂�\��
	}
	if( Ex_portFlag[EXPORT_J_PEND] != 0 ){					// 0:�ެ��َ��؂ꖳ��/1:�ެ��َ��؂�L��
// MH810100(E) K.Onodera 2020/04/09 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
		// �ެ��َ��؂ꎞ�\��
		status |= 0x04;										// ���Z��Ԓʒm		b2:		�ެ��ٗp���s��
	}
	// IC�N���W�b�g�g�p����H
	if( isEC_USE() != 0 ){
// MH810103(s) �d�q�}�l�[�Ή� ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���
//// MH810103(s) �d�q�}�l�[�Ή� ��Ԃ��݂Ȃ��悤�ɂ���
////// MH810100(S) S.Fujii 2020/07/06 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
////		if( !chk_ec_credit() ){								// �N���W�b�g���p�s��
////			status |= 0x10;									// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
////		}
////		if( !chk_ec_emoney() ){								// �N���W�b�g���p�s��
////// MH810103(s) �d�q�}�l�[�Ή�
//////			status |= 0x08;									// ���Z��Ԓʒm		b3:		Suica���p�s��
////// MH810103(e) �d�q�}�l�[�Ή�
////			status |= 0x20;									// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
////		}
////// MH810100(E) S.Fujii 2020/07/06 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//// MH810103(e) �d�q�}�l�[�Ή� ��Ԃ��݂Ȃ��悤�ɂ���
//		// �ʐM�s�ǔ������H
//		if( ERR_CHK[mod_ec][ERR_EC_COMFAIL] ){
//// MH810103(s) �d�q�}�l�[�Ή�
////			status |= 0x08;									// ���Z��Ԓʒm		b3:		Suica���p�s��
//// MH810103(e) �d�q�}�l�[�Ή�
//			status |= 0x10;									// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
//			status |= 0x20;									// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
//		}else{
//// MH810103(s) �d�q�}�l�[�Ή�
////			if( !isEcEnabled(EC_CHECK_CREDIT) ){
//			if ( !isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT) ) {				// �N���W�b�g�Ő��Z�\?
//// MH810103(e) �d�q�}�l�[�Ή�
//				status |= 0x10;								// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
//			}
//// MH810103(s) �d�q�}�l�[�Ή�
////			if( !isEcEnabled(EC_CHECK_EMONEY) ){
////				status |= 0x08;								// ���Z��Ԓʒm		b3:		Suica���p�s��
////				status |= 0x20;								// ���Z��Ԓʒm		b5:		Edy���p�s��
////			}
//			if( !isEcEmoneyEnabled(0, 0) ){					// �d�q�}�l�[���ς��\
//				status |= 0x20;								// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
//			}
//		}
//// MH810103(e) �d�q�}�l�[�Ή�
		// �ʐM�s�ǔ������H
		if( ERR_CHK[mod_ec][ERR_EC_COMFAIL] ){
			status |= 0x10;									// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
			status |= 0x20;									// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
// MH810105(S) QR�R�[�h���ϑΉ� #6578 QR�R�[�h���ς��g�p�ł��Ȃ���ԂȂ̂ɁA�o����悤�Ȑ��Z�ē����s��
			status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
// MH810105(E) QR�R�[�h���ϑΉ� #6578 QR�R�[�h���ς��g�p�ł��Ȃ���ԂȂ̂ɁA�o����悤�Ȑ��Z�ē����s��
		}else{
			// �ҋ@���
			if(OPECTL.Ope_mod != 2 && OPECTL.Ope_mod != 3){
				// ���ʃ`�F�b�N
				if ( isEC_USE()											// ���σ��[�_�g�p�ݒ�
					&& !Suica_Rec.suica_err_event.BYTE					// ���[�_�[�g�p��
					&& Suica_Rec.Data.BIT.INITIALIZE					// ����������
//					&& Suica_Rec.Data.BIT.CTRL							// ��t���
				) {
					// �ݒ�`�F�b�N
					if (!isEcBrandNoEnabledForSetting(BRANDNO_CREDIT)) {
						status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					}
					// ��M�u�����h�`�F�b�N
					if (!isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT)) {
						status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					}
					if( !isEcEmoneyEnabled(0, 0) ){					// �d�q�}�l�[���ς��\
						status |= 0x20;								// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
					}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
					// �ҋ@��ʂł݂͂Ȃ�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
					// �J�ǉ�
					if( !isEcEmoneyEnabledNotCtrl(1,0) ){					// �d�q�}�l�[���ς��\
						status |= 0x20;								// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
					}

				}else{

					status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					status |= 0x20;										// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
					status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
				}
			}else if(OPECTL.Ope_mod == 3){
				// ���ʃ`�F�b�N
				if ( isEC_USE()											// ���σ��[�_�g�p�ݒ�
					&& !Suica_Rec.suica_err_event.BYTE					// ���[�_�[�g�p��
					&& Suica_Rec.Data.BIT.INITIALIZE					// ����������
//					&& Suica_Rec.Data.BIT.CTRL							// ��t���
				) {
					// �ݒ�`�F�b�N
					if (!isEcBrandNoEnabledForSetting(BRANDNO_CREDIT)) {
						status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					}
					// ��M�u�����h�`�F�b�N
					if (!isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT)) {
						status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
//					if( !isEcEmoneyEnabled(0, 0) ){					// �d�q�}�l�[���ς��\
//						status |= 0x20;								// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
//					}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
					// �J�ǉ�
					if( !isEcEmoneyEnabledNotCtrl(1,0) ){					// �d�q�}�l�[���ς��\
						status |= 0x20;								// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
					}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
//// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
//					if( !chk_ec_qr() ){
//						status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
//					}
//// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
					if (!isEcBrandNoEnabledNoCtrl(BRANDNO_QR)) {					// QR�R�[�h�L��? 
						status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
					}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
					if( Ope_isJPrinterReady() != 1 || IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)){		// �W���[�i���g�p�s��
						status |= 0x20;								// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
						status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
					}

				}else{

					status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					status |= 0x20;										// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
					status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
				}

			}else{
			// ���Z��
				// ���ʃ`�F�b�N
				if ( isEC_USE()											// ���σ��[�_�g�p�ݒ�
					&& !Suica_Rec.suica_err_event.BYTE					// ���[�_�[�g�p��
					&& Suica_Rec.Data.BIT.INITIALIZE					// ����������
//					&& Suica_Rec.Data.BIT.CTRL							// ��t���
				) {
					if (!isEcBrandNoEnabledForSetting(BRANDNO_CREDIT)) {
						status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					}
					// ��M�u�����h�`�F�b�N
					if (!isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT)) {
						status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					}
					// ���x�z�`�F�b�N
					if(!Ec_check_cre_pay_limit()){
						status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					}
					if( !isEcEmoneyEnabled(1, 0) ){					// �d�q�}�l�[���ς��\
						status |= 0x20;								// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
					}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
//// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
//					if( !chk_ec_qr() ){
//						status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
//					}
//// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
					if (!isEcBrandNoEnabled(BRANDNO_QR)) {					// QR�R�[�h�L��?
						status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
					}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
					if( Ope_isJPrinterReady() != 1 || IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)){		// �W���[�i���g�p�s��
						status |= 0x20;								// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
						status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
					}
				}else{

					status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
					status |= 0x20;										// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
					status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6400 QR�R�[�h�Ő��Z�������̐��Z�I����ʂŁu�d�q�}�l�[ \10-�v�ƕ\������Ă��܂�
				}

			}
		}
// MH810103(e) �d�q�}�l�[�Ή� ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���
	}
	// IC�N���W�b�g�g�p�Ȃ��H
	else{
		// �g�p�Ȃ��͗��p�s�Ƃ���
// MH810103(s) �d�q�}�l�[�Ή�
//		status |= 0x08;										// ���Z��Ԓʒm		b3:		Suica���p�s��
// MH810103(e) �d�q�}�l�[�Ή�
		status |= 0x10;										// ���Z��Ԓʒm		b4:		�N���W�b�g���p�s��
		status |= 0x20;										// ���Z��Ԓʒm		b5:		�d�q�}�l�[���p�s��
// MH810105(S) QR�R�[�h���ϑΉ� #6578 QR�R�[�h���ς��g�p�ł��Ȃ���ԂȂ̂ɁA�o����悤�Ȑ��Z�ē����s��
		status |= 0x80000;							// ���Z��Ԓʒm	b19:	�R�[�h���ϗ��p�s��
// MH810105(E) QR�R�[�h���ϑΉ� #6578 QR�R�[�h���ς��g�p�ł��Ȃ���ԂȂ̂ɁA�o����悤�Ȑ��Z�ē����s��

	}
	
// MH810100(S) S.Takahashi 2020/05/08 #4151 �Ԕԃ`�P�b�g���X�i�������p�s��Ԃł��x�����\�̉����ē��A���b�Z�[�W�\�����s���j
	if(cn_errst[1]) {
		// �������[�_�[�G���[����
		status |= 0x40;										// ���Z��Ԓʒm		b6:		�������[�_�ʐM�s��
	}
// MH810100(E) S.Takahashi 2020/05/08 #4151 �Ԕԃ`�P�b�g���X�i�������p�s��Ԃł��x�����\�̉����ē��A���b�Z�[�W�\�����s���j
	
#if GT4100_NO_LCD
	return;
#endif

// MH810100(S) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
//	if( old_pay_status != status ){
//		// �f�[�^�X�V
//		old_pay_status = status;
	// �O��Ƃ̍�������H
	if( diff_pay_status(status) ){
// MH810100(E) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
		// ���Z��Ԓʒm
		lcdbm_notice_pay( status );
	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
}

//--�ȉ��A�����֐�
/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ��݉�ʕ\�� �N���W�b�g�Ή� 4-6�s�ڃ��b�Z�[�W�\��(1�R�[����1�s�\��)|*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dsp_msg                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static void dsp_msg(uchar msgno, uchar line, uchar tik_add, ushort color)
{
	int no;

	if ((msgno == CYCLIC_DSP01) || (msgno == CYCLIC_DSP09)) {
		// �T�C�N���b�N�\��
		dsp_cyclic_msg(line,msgno,tik_add);
	} else {
		// ��T�C�N���b�N�\��
		if (msgno >= EXTPAND_MSG1) {
			no = 0;
		} else {
			no = msgno;
		}
		while (1) {
			if (infomsglist[no][0].func()) {
				// ��1�\��OK
				if (infomsglist[no][1].func()) {
					// ��2�\��OK �� ���b�Z�[�W��\��
					grachr( (ushort)line, 0, 30, 0, color, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[no]);
					return;
				} else {
					// ��2�\��NG �� �����b�Z�[�W����
					no = infomsglist[no][1].alternate;
				}
			} else {
				// ��1�\��NG �� �����b�Z�[�W����
				no = infomsglist[no][0].alternate;
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
				if (no == -1) {
					return;
				}
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�\���s�ǏC��)
			} 
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ��݉�ʕ\�� �N���W�b�g�Ή� 4-6�s�ڃ��b�Z�[�W �T�C�N���b�N�\��     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dsp_cyclic_msg                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	void	dsp_cyclic_msg(uchar line,uchar msgno,uchar tik_add)
{
	ushort	wk_Prm, wk_Cnt,wk_base = 31;
	uchar	wk_DspNo;
	int		i;
	uchar	wk_DspLineNo;
	ushort	wk_para_num;

	switch( line ){
		case	3:
			wk_DspLineNo = 0;
			break;
		case	4:
			wk_DspLineNo = 1;
			break;
		case	5:
			wk_DspLineNo = 2;
			break;
		default:
			wk_DspLineNo = 0;
			break;
	}

	wk_DspNo = 0;											// �\���ΏہF�Ȃ�

	for(i=0; i<12; i++) {
		wk_para_num = OPECTL.op_mod02_dispCnt[wk_DspLineNo] / 3;
		wk_Prm = wk_base + wk_para_num;
		wk_Cnt = OPECTL.op_mod02_dispCnt[wk_DspLineNo] - 3*wk_para_num;

		if(wk_Cnt == 0) {
			wk_Cnt = 5;										// prm_get�̑�T�����i�ʒu�F654321�̏��j���Z�o
		} else {
			wk_Cnt = (ushort)(6 - wk_Cnt*2 -1);				// prm_get�̑�T�����i�ʒu�F654321�̏��j���Z�o
		}
		wk_DspNo = (uchar)prm_get(COM_PRM, S_DSP, (short)wk_Prm, 2, (char)wk_Cnt);	// �\���Ώ�get
		OPECTL.op_mod02_dispCnt[wk_DspLineNo]++;			// �\���ؑ֗p�J�E���^UP
		if(OPECTL.op_mod02_dispCnt[wk_DspLineNo] > 11) {
			OPECTL.op_mod02_dispCnt[wk_DspLineNo] = 0;		// �擪�ɖ߂�
		}

		if(wk_DspNo != 0) {									// �\���Ώۂł���
			switch( wk_DspNo ){
				case	2:									// �T�[�r�X���̎�
					if (chk_service()) {
						break;
					}
					continue;
				case	3:									// �v���y�C�h�J�[�h�̎�
					if (chk_prepaid()) {
						break;
					}
					continue;
				case	4:									// �񐔌��̎�
					if (chk_coupon()) {
						break;
					}
					continue;
				case	5:									// ������̎�
					if (chk_teiki()) {
						break;
					}
					continue;
				case	6:									// �N���W�b�g�̎�
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//						if (chk_credit()) {
//							break;
//						}
//						continue;
					if ( isEC_USE() ) {
						// ���σ��[�_
						if( chk_ec_credit() ){				// �N���W�b�g���Z�\
							break;
						}
// MH810103 GG119202(S) �ڋq��ʂ̓d�q�}�l�[�Ή�
						// ���p�s���b�Z�[�W�͕\�����Ȃ�
//						else{								// ���σ��[�_���p���̃N���W�b�g�g�p�s�̂ݕʃ��b�Z�[�W��\��������
//// MH321800(S) ������̒ǉ��ӏ���ύX
////							grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[44]);			// "                  �͎g���܂���"
//							grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[161]);					// "                  �͎g���܂���"
//// MH321800(E) ������̒ǉ��ӏ���ύX
//							grachr((ushort)line, 0, 16, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR_CYCLIC_MSG[wk_DspNo]);	// �N���W�b�g
//							return;
//						}
// MH810103 GG119202(E) �ڋq��ʂ̓d�q�}�l�[�Ή�
					}
// MH810103 GG119202(S) �ڋq��ʂ̓d�q�}�l�[�Ή�
//					else{
//						if (chk_credit()) {
//							break;
//						}
//					}
// MH810103 GG119202(E) �ڋq��ʂ̓d�q�}�l�[�Ή�
					continue;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
				case	7:									// �r���������̎�
				case	9:									// �o�`�r�l�n�̎�
				case   19:									// �h�b�n�b�`�̎�
// MH321800(S) hosoda IC�N���W�b�g�Ή�
//				case   20:									// �h�b�J�[�h�̎�
// MH321800(E) hosoda IC�N���W�b�g�Ή�
					if (chk_suica()) {
						break;
					}
					continue;
// MH321800(S) hosoda IC�N���W�b�g�Ή�
				case   20:									// �d�q�}�l�[�̎�
					if (isEC_USE()) {
					// EC���σ��[�_�|
						if (chk_ec_emoney()) {
							break;
						}
// MH810103 GG119202(S) �ڋq��ʂ̓d�q�}�l�[�Ή�
						// ���p�s���b�Z�[�W�͕\�����Ȃ�
//// GG119200(S) �d�q�}�l�[���p�s�\���Ή�
//						else{								// ���σ��[�_���p���̓d�q�}�l�[�g�p�s�̂ݕʃ��b�Z�[�W��\��������
//							grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[161]);					// "                  �͎g���܂���"
//							grachr((ushort)line, 0, 16, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR_CYCLIC_MSG[wk_DspNo]);	// �d�q�}�l�[
//							return;
//						}
//// GG119200(E) �d�q�}�l�[���p�s�\���Ή�
// MH810103 GG119202(E) �ڋq��ʂ̓d�q�}�l�[�Ή�
					} else {
						if (chk_suica()) {
							break;
						}
					}
					continue;
// MH321800(E) hosoda IC�N���W�b�g�Ή�
				case	8:									// �d�����̎�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					if (chk_edy()) {
//						break;
//					}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					continue;	
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
				case	1:									// ����
					if (chk_coin() || chk_note()) {
						break;
					}
					continue;
				case	15:
					if (chk_coin()) {						// �d��
						break;
					}
					continue;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
				case   16:									// �����̎�
				case   17:									// ��~�D�̎�
					if (chk_note()) {
						break;
					}
					continue;
				case   26:									// T�J�[�h�̎�
					if( chk_Tcard() ){						
						break;
					}
					continue;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				case	28:									// QR�R�[�h����
					if (chk_ec_qr()) {
						break;
					}
					continue;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				default:
					if((11 <= wk_DspNo)&&(wk_DspNo <= 14) ||
				    		(18 <= wk_DspNo)&&(wk_DspNo <= 99)){
						// ���ݒ�G���A
						continue;
				    }else{
						break;
					}
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			}
			break;
		}
	}
// MH322914 (s) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
//	if( (wk_DspNo == 0) || (wk_DspNo > 26) ) {
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	if( (wk_DspNo == 0) || (wk_DspNo > 26) || (i == 12)) {
	if( (wk_DspNo == 0) || (wk_DspNo > 28) || (i == 12)) {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		displclr(line);
// MH322914 (e) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
		return;
	}

	if(msgno == 1){
		grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[msgno]);		// "                �����p�ł��܂�"
		grachr((ushort)line, 0, 16, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR_CYCLIC_MSG[wk_DspNo]);	// �}�̖��\��
	}else if(msgno == 9){
		grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[msgno]);		// ""���x������"
		grachr((ushort)line, 12, 16, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR_CYCLIC_MSG[wk_DspNo]);// �}�̖��\��
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ���ڰ��݉�ʕ\�� �N���W�b�g�Ή� 4-6�s�ڃ��b�Z�[�W�\�� ����֐��Q       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// �\�� ������OK
int chk_OK(void)
{
	return 1;
}

// �T�[�r�X������
int chk_service(void)
{
	uchar i;				// �J�E���^
	short	data_adr;		// �g�p�\������ʂ��ް����ڽ
	char	data_pos;		// �g�p�\������ʂ��ް��ʒu

	if( 0 == prm_get( COM_PRM, S_PAY, 21, 1, 3 ) ) {	// ���C���[�_�Ȃ�?
		return 0;										// �T�[�r�X���g�p�s��
	}

	for(i = 0 ; i < 15 ; i++){			//�T�[�r�X����މ�ٰ��
		if( ryo_buf.syubet < 6 ){
			// ���Z�Ώۂ̗������A�`F(0�`5)
			data_adr = (2*(i))+76;					// �g�p�\������ʂ��ް����ڽ�擾
			data_pos = (char)(6-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
		}
		else{
			// ���Z�Ώۂ̗������G�`L(6�`11)
			data_adr = (2*(i))+77;					// �g�p�\������ʂ��ް����ڽ�擾
			data_pos = (char)(12-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
		}
		if( prm_get( COM_PRM, S_SER, data_adr, 1, data_pos ) == 0 ){		// �g�p�ݒ�H
			return 1;														// 1�ł��g�p�ł���Ε\��OK�Ƃ���B
		}
	}
	return 0;
}

// �v���y�C�h�J�[�h����
int chk_prepaid(void)
{
	if (prm_get( COM_PRM, S_PRP, 1, 1, 1 ) == 1) {
		return 1;
	} else {
		return 0;
	}
}

// �N���W�b�g�J�[�h����
int chk_credit(void)
{
	int		ret = 1;
	uchar	sts;

// MH321800(S) hosoda IC�N���W�b�g�Ή�
	if ( isEC_USE() ) {
	// ���σ��[�_
		return chk_ec_credit();
	}
// MH321800(E) hosoda IC�N���W�b�g�Ή�

	sts = creStatusCheck();

	if( sts & CRE_STATUS_DISABLED ){	// �N���W�b�g�ݒ�Ȃ�
		ret = 0;	// �N���W�b�g���p�s��
	}
	if( sts & CRE_STATUS_STOP ){		// �N���W�b�g��~��
		ret = 0;	// �N���W�b�g���p�s��
	}
	if( sts & CRE_STATUS_PPP_DISCONNECT ){	// PPP���ڑ�
		ret = 0;	// �N���W�b�g���p�s��
	}

	return ret;
}

// �񐔌�����
int chk_coupon(void)
{
	if ((uchar)prm_get( COM_PRM, S_PRP, 1, 1, 1 ) == 2) {
		return 1;
	} else {
		return 0;
	}
}

// ���������
int chk_teiki(void)
{
	return 1;
}

// ��������
int chk_note(void)
{
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
	if (isEC_USE() &&
// MH810104 GG119201(S) �d�݁E�������p����C��
		check_enable_multisettle() > 1 &&		// �}���`�u�����h�œd�q�}�l�[�I�����͗��p�s��
// MH810104 GG119201(E) �d�݁E�������p����C��
		RecvBrandResTbl.res == EC_BRAND_SELECTED &&
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
//		// �d�q�}�l�[�I�����͗��p�s�Ƃ���
		EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
		// �d�q�}�l�[�AQR�R�[�h�I�����͗��p�s�Ƃ���
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		return 0;
	}
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���

	if((cn_errst[1] != 0 )||		//��������?(Y)
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
		( note_err_flg != 0 )||		//�������p�s�H
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
		( notful() == (char)-1 )){	//�������ɖ��t
		return 0;
	}else{
		return 1;
	}
}
// �a��ؔ���
int chk_azukari(void)
{
	if(ryo_buf.fusoku){	//�s�����z�L
		return 1;
	}else{
		return 0;
	}
}
// �t���b�v����
int chk_flp(void)
{
	if(LkKind_Get(OPECTL.Pr_LokNo) == LK_KIND_FLAP || 
	   LkKind_Get(OPECTL.Pr_LokNo) == LK_KIND_INT_FLAP ){
		return 1;
	}else{
		return 0;
	}
}
// ���b�N����
int chk_lock(void)
{
	if(LkKind_Get(OPECTL.Pr_LokNo) == LK_KIND_LOCK){
		return 1;
	}else{
		return 0;
	}
}
// �r������������
int chk_suica(void)
{
	if(Suica_Rec.suica_err_event.BYTE == 0 &&
		!Suica_Rec.Data.BIT.MIRYO_TIMEOUT &&		// �����^�C���A�E�g������
		!Suica_Rec.Data.BIT.PRI_NG &&				// �W���[�i���ُ햢����
		prm_get(COM_PRM, S_PAY, 24, 1, 3) == 1 ) {
		return 1;
	}else{
		return 0;
	}
}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//// �d��������
//int chk_edy(void)
//{
//	if( !EDY_USE_ERR && 														// ���p�\��Ԃł��鎖���m�F���\������
//		!Edy_Rec.edy_status.BIT.PRI_NG &&										// �W���[�i���ُ햢����
//		(Edy_Arc_Connection_Chk() == 0 ) &&
//		!Edy_Rec.edy_status.BIT.MIRYO_LOSS ) {									// 07-01-23�ύX Edy���Z���s��ѱ�Č�͂d�����\�����Ȃ�
//		return 1;
//	}else{
//		return 0;
//	}
//}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

int chk_Tcard(void)
{
	return 0;
}

// MH321800(S) hosoda IC�N���W�b�g�Ή�
// �N���W�b�g�J�[�h���� (���σ��[�_)
int chk_ec_credit(void)
{
	int	ret = 0;

// MH810103 GG119202(S) �J�ǁE�L�������ύX
//	if ( isEcReady(EC_CHECK_CREDIT) ) {						// �N���W�b�g�Ő��Z�\?
	if ( isEcBrandNoEnabled(BRANDNO_CREDIT) ) {				// �N���W�b�g�Ő��Z�\?
// MH810103 GG119202(E) �J�ǁE�L�������ύX
		if(RecvBrandResTbl.res == EC_BRAND_UNSELECTED) {	// �u�����h���I��
			ret = 1;
		}
		else {												// �u�����h�I��
			if(RecvBrandResTbl.no == BRANDNO_CREDIT) {		// �I�������u�����h���N���W�b�g
				ret = 1;
			}
		}
	}
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
	if( ret && !Ec_check_cre_pay_limit() ){	// �N���W�b�g�͐��Z���x�z�𒴂��Ă���H
		ret = 0;	// �N���W�b�g�g�p�s��
	}
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
	return ret;
}

// �d�q�}�l�[���� (���σ��[�_)
int chk_ec_emoney(void)
{
	int	ret = 0;

// MH810103 GG119202(S) �d�q�}�l�[���ώ��̓W���[�i���v�����^�ڑ��ݒ���Q�Ƃ���
	if (Suica_Rec.Data.BIT.PRI_NG != 0) {
		// �W���[�i���v�����^�g�p�s���͓d�q�}�l�[���ϕs��
		return 0;
	}
// MH810103 GG119202(E) �d�q�}�l�[���ώ��̓W���[�i���v�����^�ڑ��ݒ���Q�Ƃ���

// MH810103 GG119202(S) �J�ǁE�L�������ύX
//	if ( isEcReady(EC_CHECK_EMONEY) && WAR_MONEY_CHECK ) {	// �d�q�}�l�[�Ő��Z�\?
	if (isEcEmoneyEnabled(1, 0)) {							// �d�q�}�l�[�L��
// MH810103 GG119202(E) �J�ǁE�L�������ύX
		if(RecvBrandResTbl.res == EC_BRAND_UNSELECTED) {	// �u�����h���I��
			ret = 1;
		}
		else {												// �u�����h�I��
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//			if(isEcBrandNoEMoney(RecvBrandResTbl.no)) {		// �I�������u�����h���d�q�}�l�[
			if(isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {	// �I�������u�����h���d�q�}�l�[
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
				ret = 1;
			}
		}
	}
	return ret;
}

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
// QR�R�[�h���ϔ��� (���σ��[�_)
int chk_ec_qr(void)
{
	int	ret = 0;

	if (Suica_Rec.Data.BIT.PRI_NG != 0) {
		// �W���[�i���v�����^�g�p�s����QR���ϕs��
		return 0;
	}

	if (isEcBrandNoEnabled(BRANDNO_QR)) {					// QR�R�[�h�L��?
		if (RecvBrandResTbl.res == EC_BRAND_UNSELECTED) {	// �u�����h���I��
			ret = 1;
		}
		else {												// �u�����h�I��
			if (RecvBrandResTbl.no == BRANDNO_QR) {			// �I�������u�����h��QR�R�[�h
				ret = 1;
			}
		}
	}
	return ret;
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// �d�ݔ���
int chk_coin(void)
{
	int	ret = 0;

// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
	if (isEC_USE() &&
// MH810104 GG119201(S) �d�݁E�������p����C��
		check_enable_multisettle() > 1 &&		// �}���`�u�����h�œd�q�}�l�[�I�����͗��p�s��
// MH810104 GG119201(E) �d�݁E�������p����C��
		RecvBrandResTbl.res == EC_BRAND_SELECTED &&
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
//		// �d�q�}�l�[�I�����͗��p�s�Ƃ���
		EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
		// �d�q�}�l�[�AQR�R�[�h�I�����͗��p�s�Ƃ���
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		return 0;
	}
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���

	if (coin_err_flg == 0) {
		ret = 1;
	}
	return ret;
}
// MH321800(E) hosoda IC�N���W�b�g�Ή�

/*-------------------------------*/
/* ��ʕ\���p�@���Z�}�̎�� �擾 */
/*-------------------------------*/
ushort	Ope_Disp_Media_Getsub( uchar mode )
{
	ushort	ret = 0;
	long	suica_mode=0;
																// �ݒ�ɂ��؂蕪��
// MH321800(S) hosoda IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//// GG119200(S) �d�q�}�l�[�Ή�
////	if (isEC_USE()) {											// ���σ��[�_�g�p�ݒ�
//	if (isEcEnabled(EC_CHECK_EMONEY)) {							// ���σ��[�_�g�p�ݒ�
//// GG119200(E) �d�q�}�l�[�Ή�
	if (isEcEmoneyEnabled(0, 0)) {								// �d�q�}�l�[�L��
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
		ret = OPE_DISP_MEDIA_TYPE_EC;
	}
	else
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if(prm_get(COM_PRM, S_PAY, 24, 2, 3) == 1 ) {				// Suicaذ�ް�̂ݎg�p�\
	if(prm_get(COM_PRM, S_PAY, 24, 1, 3) == 1 ) {				// Suicaذ�ް�̂ݎg�p�\
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		if( Is_SUICA_STYLE_OLD ){								// ���œ���ݒ�
			suica_mode = prm_get(COM_PRM, S_SCA, 2, 3, 1);		// ICOCA���[�h�ǉ��̈׏C��
			switch( suica_mode )
			{
				case  1:
				default:
					ret = OPE_DISP_MEDIA_TYPE_SUICA;					// �r��������
					break;
				case 10:
					ret = OPE_DISP_MEDIA_TYPE_PASMO;					// �o�`�r�l�n
					break;
			}
		}
		else{													// �V�œ���ݒ�
			suica_mode = prm_get(COM_PRM, S_SCA, 1, 1, 1);		// ���[�_�[���get
			switch( (ushort)suica_mode ){
			case	0:											// 0=�d�q�}�l�[
			default:
				ret = OPE_DISP_MEDIA_TYPE_eMONEY;
				break;
			case	1:											// 1=�r��������
				ret = OPE_DISP_MEDIA_TYPE_SUICA;
				break;
			case	2:											// 2=�o�`�r�l�n
				ret = OPE_DISP_MEDIA_TYPE_PASMO;
				break;
			case	4:											// 4���h�b�n�b�`
				ret = OPE_DISP_MEDIA_TYPE_ICOCA;
				break;
			}
		}
	}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	else if(prm_get(COM_PRM, S_PAY, 24, 2, 3) == 10 ) {			// Edyذ�ް�̂ݎg�p�\
//		ret = OPE_DISP_MEDIA_TYPE_EDY;							// �d����
//	}
//	else if(prm_get(COM_PRM, S_PAY, 24, 2, 3) == 11 ) {			// ����ذ�ް�g�p�\(Suica & Edyذ�ް)
//		ret = OPE_DISP_MEDIA_TYPE_ICCARD;						// �h�b�J�[�h
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	
	if(mode == 0) {												// �u�ݒ�݂̂ł̔��f�v�w�莞
	}
	else {														// �u�g�p���ꂽ�J�[�h�𔻕ʁv�w�莞
// MH321800(S) hosoda IC�N���W�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		if ( EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {
		if ( EcUseEMoneyKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			ret = OPE_DISP_MEDIA_TYPE_EC;
		}
		else
// MH321800(E) hosoda IC�N���W�b�g�Ή�
		if(Settlement_Res.settlement_data ) {					// �r��������ذ�ް����
			ret = OPE_DISP_MEDIA_TYPE_SUICA;					// �J�[�h���p���Suica�Ƃ��Ĉ���
			if( Is_SUICA_STYLE_OLD ){							// ���œ���ݒ�
				if ( memcmp(Settlement_Res.Suica_ID , "PB" , 2) == 0){	// �J�[�h�ԍ��̐擪2������"PB"�ł����
					ret = OPE_DISP_MEDIA_TYPE_PASMO;					// PASMO�J�[�h�Ɣ��f����
				}
			}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		} else if( PayData.Electron_data.Edy.e_pay_kind == EDY_USED ){	// �d����ذ�ް����
//				ret = OPE_DISP_MEDIA_TYPE_EDY;					// �d����
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		}
	}
	return( ret );
}


/*---------------------------------------------------*/
/* ��ʕ\���p�@���Z�}�̎�� ү����No(OPE_CHR_G)�擾 */
/*---------------------------------------------------*/
ushort	Ope_Disp_Media_GetMsgNo( uchar mode, uchar type1, uchar type2 )
{
	ushort	msgno = 0;

	wk_media_Type = Ope_Disp_Media_Getsub(mode);					// ��ʕ\���p�@���Z�}�̎�� �擾

	switch(wk_media_Type) {		
		case OPE_DISP_MEDIA_TYPE_SUICA:								// �r��������
			if( (mode == 0) || (Is_SUICA_STYLE_OLD) ){				// mode=0�F���̂͐ݒ�ɏ]���i�J�[�h�^�b�`�O�j
																	// ���͋��œ���ݒ�
				if(type1 == 0) {									// �g�p�}�́@�@�@�@�@�~�@�w��
					if(type2 == 0) {								// �P�Ǝg�p�@�w��
						msgno = 86;									// "  �r��������            �~    ",	//71
					} 
				} else if(type1 == 1) {								// �g�p�}�́@���p�z�@�~�@�w��
					msgno = 87;										//"�r�����������p�z           �~ ",		//72
				} else if(type1 == 3) {								// �g�p�}�́@���p�s�G���[�Q�@�w��
					msgno = 99;										//"   �����g�p��͂r����������   ",		//86
				}
			}
			else{													// �V�œ���w��
				// ---�y�p�����[�^�����z------------------------------------
				// type1 : 0 = OpeLcd(4)�@�u���Ɓ@�v��ʕ\���p
				//		   1 = OpeLcd(5)�@�u����v��ʕ\���p
				// type2 : 0 �Œ�
				// ---------------------------------------------------------

				if(type1 == 0){
					msgno = 125;									// " �@���p�z�@ �@�@�@�@�@�@�@�@�~"
				} 
				else if(type1 == 1){
					msgno = 126;									// "�@�@�@�@�@���p�z�@�@�@�@�@ �~ "
				}
			}
			break;

		case OPE_DISP_MEDIA_TYPE_PASMO:								// �o�`�r�l�n
			if(type1 == 0) {										// �g�p�}�́@�@�@�@�@�~�@�w��
				if(type2 == 0) {									// �P�Ǝg�p�@�w��
					msgno = 106;									// "  �o�`�r�l�n            �~    ",	//95
				}			
			} else if(type1 == 1) {									// �g�p�}�́@���p�z�@�~�@�w��
				msgno = 107;										// "�o�`�r�l�n���p�z           �~ ",	//96
			} else if(type1 == 3) {									// �g�p�}�́@���p�s�G���[�Q�@�w��
				msgno = 112;										// "   �����g�p��͂o�`�r�l�n��   ",	//110
			}
			break;
	
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case OPE_DISP_MEDIA_TYPE_EDY:								// �d����
//			if(type1 == 0) {										// �g�p�}�́@�@�@�@�@�~�@�w��
//				if(type2 == 0) {									// �P�Ǝg�p�@�w��
//					msgno = 108;									// "  �d����                �~    ",	//100
//				}			
//			} else if(type1 == 1) {									// �g�p�}�́@���p�z�@�~�@�w��
//				msgno = 109;										// "   �d�������p�z            �~ ",	//101
//			}
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		case OPE_DISP_MEDIA_TYPE_ICOCA:								// �h�b�n�b�`
			if(type1 == 0) {										// �g�p�}�́@�@�@�@�@�~�@�w��
				if(type2 == 0) {									// �P�Ǝg�p�@�w��
					msgno = 120;									// "  �h�b�n�b�`            �~    ",	//81
				}
			} else if(type1 == 1) {									// �g�p�}�́@���p�z�@�~�@�w��
				msgno = 121;										// "�h�b�n�b�`���p�z           �~ ",	//82
			} else if(type1 == 3) {									// �g�p�}�́@���p�s�G���[�Q�@�w��
				msgno = 122;										// "   �����g�p��͂h�b�n�b�`��   ",	//84
			}
			break;

		case OPE_DISP_MEDIA_TYPE_ICCARD:							// �h�b�J�[�h�i�����J�[�h�g�p�\�j
			if(type1 == 0) {										// �g�p�}�́@�@�@�@�@�~�@�w��
				if(type2 == 0) {									// �P�Ǝg�p�@�w��
					msgno = 110;									// "  �h�b�J�[�h            �~    ",	//105
				}			
			} else if(type1 == 1) {									// �g�p�}�́@���p�z�@�~�@�w��
				msgno = 111;										// "�h�b�J�[�h���p�z           �~ ",	//106
			} else if(type1 == 3) {									// �g�p�}�́@���p�s�G���[�Q�@�w��
				msgno = 113;										// "   �����g�p��͂h�b�J�[�h��   ",	//111
			}
			break;
		case OPE_DISP_MEDIA_TYPE_eMONEY:							// �d�q�}�l�[
			if(type1 == 3) {										// �g�p�}�́@���p�s�G���[�Q�@�w��
				msgno = 128;										// "   �����g�p��͓d�q�}�l�[��   "
			}														// �V�d�l�� mode==0�̓G���[���b�Z�[�W�̂�
			break;
// MH321800(S) hosoda IC�N���W�b�g�Ή�
		case OPE_DISP_MEDIA_TYPE_EC:								// ���σ��[�_
			if (type1 == 0) {										// ���Z�� ���Ɖ��
				msgno = 155;										// "  �d�q�}�l�[            �~    "
			} else if (type1 == 1) {								// ���Z�����エ����
				msgno = 156;										// "�d�q�}�l�[�x��             �~ "
			} else if (type1 == 3) {								// �g�p�}�́@���p�s�G���[�Q�@�w��
				msgno = 128;										// "   �����g�p��͓d�q�}�l�[��   "
			}
			break;
// MH321800(E) hosoda IC�N���W�b�g�Ή�
		default:
			break;
	}

	return(msgno);

}

/*[]----------------------------------------------------------------------[]*/
/*| ���C�f�[�^�ǎ��G���[�ڍו\��                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MagReadErrCodeDisp( ercd )                              |*/
/*| PARAMETER    : ercd = �G���[�R�[�h                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         :                                                         |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void	MagReadErrCodeDisp(uchar ercd)
{
	switch(ercd){
		case 0x11:
			OPECTL.CR_ERR_DSP = 40;						// �ǎ��ł��܂��� (ST)
			break;
		case 0x12:
			OPECTL.CR_ERR_DSP = 41;						// �ǎ��ł��܂��� (LT)
			break;
		case 0x21:
			OPECTL.CR_ERR_DSP = 42;						// �ǎ��ł��܂��� (S)
			break;
		case 0x22:
			OPECTL.CR_ERR_DSP = 43;						// �ǎ��ł��܂���(D)
			break;
		case 0x23:
			OPECTL.CR_ERR_DSP = 44;						// �ǎ��ł��܂���(P)
			break;
		case 0x25:
			OPECTL.CR_ERR_DSP = 45;						// �ǎ��ł��܂��� (C)
			break;
		default:
			OPECTL.CR_ERR_DSP = 46;						// �ǎ��ł��܂���(EO)
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�̎��ؕ\��̎擾
//[]----------------------------------------------------------------------[]
///	@return			26	: "      �̎����𔭍s���܂�      "<br>
///					27	: "     �̎���������艺����     "<br>
///					147	: "      �̎��؂𔭍s���܂�      "<br>
///					148	: "     �̎��؂�����艺����     "<br>
///					149	: "    ���p���׏��𔭍s���܂�    "<br>
///					150	: "   ���p���׏�������艺����   "
///	@author			m.onouchi
///	@attention		�̎���(F2)�L�[����������Ă����ԂŃR�[�����邱�ƁB
///	@note			�̎��؂Ɉ󎚂���\����擾��<br>
///					������e�[�u��OPE_CHR[][31]�̃C���f�b�N�X��Ԃ��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/08/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Disp_Receipt_GetMsgNo(void)
{
	ushort	msgno;

	if (prm_get(COM_PRM, S_RTP, 2, 1, 3) == 1) {			// �̎��ؕ\���(2)(17-0002�C)
		msgno = 26;											// "      �̎����𔭍s���܂�      " OPE_CHR[26][31]
	} else {
		msgno = 147;										// "      �̎��؂𔭍s���܂�      " OPE_CHR[147][31]
	}

	if (ryo_buf.credit.pay_ryo != 0) {						// �N���W�b�g���Z
		switch (prm_get(COM_PRM, S_RTP, 2, 1, 1)) {
		case 2:												// ��������N���W�b�g���Z����"�̎���"�Ŕ��s
			if (ryo_buf.nyukin != 0) {						// ��������
				break;
			}
			// not break
		case 1:												// �N���W�b�g���Z����"���p���׏�"�Ŕ��s
			msgno = 149;									// "    ���p���׏��𔭍s���܂�    " OPE_CHR[149][31]
			break;
		default:											// ���ׂ�"�̎���"�Ŕ��s
			break;
		}
	}

	if ((OPECTL.Ope_mod == 3) || (OPECTL.Ope_mod == 23)){	// ���Z����
		msgno++;											// �`������艺����
	}

	return(msgno);
}


/*[]----------------------------------------------------------------------[]*/
/*| �̎����ē��\��				                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_Receipt_disp( void )                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void	Lcd_Receipt_disp(void)
{
	ushort	wk_MsgNo;

// MH810105(S) MH364301 �C���{�C�X�Ή�
//	if (Ope_isPrinterReady() == 0) {				// �v�����^�ُ킠��
// GG129004(S) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
// 	if (Ope_isPrinterReady() == 0 ||				// �v�����^�ُ킠��
	if ((!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) ||	// �v�����^�ُ킠��
// GG129004(E) R.Endo 2024/12/13 #7561 �d�q�̎��؂𔭍s����ݒ�Ń��V�[�g�������؂�ɂ���ƁA�d�q�̎��؂����s�ł��Ȃ�
		(IS_INVOICE &&								// �C���{�C�X�ݒ莞�̓��V�[�g�ƃW���[�i�����Е��ł������Ȃ������ꍇ
		  Ope_isJPrinterReady() == 0)) {			// �̎��؂��󎚂��Ȃ�
// MH810105(E) MH364301 �C���{�C�X�Ή�
		if (prm_get(COM_PRM, S_RTP, 2, 1, 3) == 1) {			// �̎��ؕ\���(2)(17-0002�C)
			grachr(7, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[141]);		// " �����̎����̔��s�͂ł��܂��� "
		}
		else {
			grachr(7, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[154]);		// " �����̎��؂̔��s�͂ł��܂��� "
		}
	}else{
		if (ReceiptChk() == 0) {							// ���V�[�g���s�\�ݒ莞
				if( OPECTL.RECI_SW == 1 ){
					wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();									// "      �̎��؂𔭍s���܂�      "
					grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "     �̎��؂�����艺����     "
				}
				else{
					grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);		// " �K�v�ȕ��͗̎��������ĉ����� "
				}
			}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���Ԉʒu�ԍ��̕`��J�n��擾
//[]----------------------------------------------------------------------[]
///	@return			���Ԉʒu�ԍ��̕`��J�n��(16-21)
///	@author			m.onouchi
///	@attention		�O���[�o���ϐ�knum_len���g�p���Ă��邽��<br>
///					�l���m�肵�Ă����ԂŃR�[�����邱�ƁB
///	@note			�Ԏ��ԍ����͉�ʂȂǂŕ\�����钓�Ԉʒu�ԍ���<br>
///					�`��J�n���Ԃ��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/08/17<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Disp_LokNo_GetFirstCol(void)
{
	ushort	col;

	switch (knum_len) {
	case 1:
		col = 21;
		break;
	case 2:
		col = 19;
		break;
	case 3:
		col = 17;
		break;
	case 4:
		col = 16;
		break;
	default:
		col = 17;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}

	return col;
}

// MH810100(S) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)
//[]----------------------------------------------------------------------[]
///	@brief			�O��̐��Z��ԂƔ�r(��r��A�O���Ԃ��X�V)
//[]----------------------------------------------------------------------[]
/// @param[in]		sta		��r�Ώۂ̏��
///	@return			bRet	TRUE(�Ⴂ����)/FALSE(�Ⴂ�Ȃ�)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/04/10<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static BOOL diff_pay_status( ulong sta )
{
	BOOL bRet = FALSE;	// �Ⴂ�Ȃ�

	// �O��ƍ���̐��Z��Ԕ�r
	if( old_pay_status != sta ){
		bRet = TRUE;
		// ����̏�Ԃ�O���Ԃ�
		old_pay_status = sta;
	}

	return bRet;
}

//[]----------------------------------------------------------------------[]
///	@brief			�O�񐸎Z��Ԃ��N���A
//[]----------------------------------------------------------------------[]
/// @param			none
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/04/10<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void clear_pay_status( void )
{
	old_pay_status = 0;
}
// MH810100(E) K.Onodera 2020/04/10 #4008 �Ԕԃ`�P�b�g���X(�̎��؈󎚒��̗p���؂ꓮ��)