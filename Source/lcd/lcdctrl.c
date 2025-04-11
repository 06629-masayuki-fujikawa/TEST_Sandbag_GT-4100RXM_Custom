/*[]----------------------------------------------------------------------[]*/
/*| LCD control                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2005-02-01                                               |*/
/*| Update      : font_change 2005-09-01 T.Hashimoto                       |*/
/*| Update      : 新精算機対応 2012-12-15 k.totsuka                        |*/
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
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"can_def.h"
#include	"ntnet_def.h"
#include	"cre_ctrl.h"

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern void lcdbm_notice_pay(ulong pay_status);
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

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
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//int		chk_edy(void);
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
int		chk_Tcard(void);
// MH321800(S) hosoda ICクレジット対応
int		chk_ec_credit(void);
int		chk_ec_emoney(void);
int		chk_coin(void);
// MH321800(E) hosoda ICクレジット対応
// MH810105(S) MH364301 QRコード決済対応
int		chk_ec_qr(void);
// MH810105(E) MH364301 QRコード決済対応

enum {
	CYCLIC_DSP01 = 1,
	CYCLIC_DSP09 = 9,
	EXTPAND_MSG1 = 41,
	EXTPAND_MSG2,
	EXTPAND_MSG3,
};

// 関数型定義
typedef int (*funcDsp)(void);

// 表示メッセージテーブル
typedef struct {
	funcDsp func;		// チェック関数
	int alternate;			// 次候補メッセージNo
}t_DspMsg;

// MH810100(S) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
static ulong old_pay_status = 0;
static BOOL diff_pay_status( ulong sta );
// MH810100(E) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)

//----------------------------------------------------------
// ※opestr.cのOPE_CHR_G_SALE_MSG[][]と関連して修正のこと
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
// MH321800(S) T.Nagai ICクレジット対応
//	{{chk_note, 11}, {chk_OK, 0}},						// 10
//	{{chk_OK, 0}, {chk_OK, 0}},							// 11
	{{chk_coin, 0}, {chk_note, 11}},					// 10
	{{chk_coin, 0}, {chk_OK, 0}},						// 11
// MH321800(E) T.Nagai ICクレジット対応
	{{chk_service, 0}, {chk_OK, 0}},					// 12
	{{chk_service, 3}, {chk_prepaid, EXTPAND_MSG3}},	// 13
	{{chk_OK, 0}, {chk_OK, 0}},							// 14
	{{chk_OK, 0}, {chk_OK, 0}},							// 15
	{{chk_OK, 0}, {chk_OK, 0}},							// 16
	{{chk_OK, 0}, {chk_OK, 0}},							// 17
	{{chk_note, 0}, {chk_OK, 0}},						// 18
	{{chk_OK, 0}, {chk_OK, 0}},							// 19
	{{chk_OK, 0}, {chk_OK, 0}},							// 20
// MH321800(S) hosoda ICクレジット対応 (表示不良修正)
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
// MH321800(E) hosoda ICクレジット対応 (表示不良修正)
	{{chk_OK, 0}, {chk_OK, 0}},							// 26
	{{chk_OK, 0}, {chk_OK, 0}},							// 27
	{{chk_OK, 0}, {chk_OK, 0}},							// 28
	{{chk_flp, 32}, {chk_OK, 0}},						// 29 フラップ板が下がったことを
	{{chk_flp, 32}, {chk_OK, 0}},						// 30 ロック板が下がったことを
	{{chk_OK, 0}, {chk_OK, 0}},							// 31 確認して
	{{chk_OK, 0}, {chk_OK, 0}},							// 32 ロック解除を
	{{chk_OK, 0}, {chk_OK, 0}},							// 33
	{{chk_OK, 0}, {chk_OK, 0}},							// 34
	{{chk_OK, 0}, {chk_OK, 0}},							// 35
	{{chk_OK, 0}, {chk_OK, 0}},							// 36
	{{chk_OK, 0}, {chk_OK, 0}},							// 37
	{{chk_credit, 0}, {chk_OK, 0}},						// 38
	{{chk_credit, 0}, {chk_OK, 0}},						// 39
	{{chk_Tcard, 0}, {chk_OK, 0}},						// 40
	//----------------------------------------------//
	// 以下、拡張メッセージ
	//----------------------------------------------//
	{{chk_coupon, 0}, {chk_OK, 0}},						// EXPAND1
	{{chk_coupon, 5}, {chk_credit, EXTPAND_MSG1}},		// EXPAND2
	{{chk_service, EXTPAND_MSG1}, {chk_coupon, 2}},		// EXPAND3
};

extern char	PrcKigenStr[32];

/*[]----------------------------------------------------------------------[]*/
/*| ﾊﾞｲﾅﾘﾃﾞｰﾀ全角表示                                                      |*/
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
/*| ﾊﾞｲﾅﾘﾃﾞｰﾀ半角表示                                                      |*/
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
/*| ﾊﾞｲﾅﾘﾃﾞｰﾀ表示                                                          |*/
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
/*| long型ﾊﾞｲﾅﾘﾃﾞｰﾀ全角表示                                                |*/
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
/*| long型ﾊﾞｲﾅﾘﾃﾞｰﾀ半角表示                                                |*/
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
/*| long型ﾊﾞｲﾅﾘﾃﾞｰﾀ表示                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nmdspll                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ulong  dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not / 2:'･' supress        |*/
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
// MH810100(S) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)
//	unsigned long	bb;
	unsigned long long	bb;		// 10の10乗が入りきらないので拡張
// MH810100(E) Y.Yoshida 2020/06/11 割引券に項目追加(#4206 QRフォーマット変更対応)

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
/*| long型HEXﾃﾞｰﾀ半角表示                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : opedpl2                                                 |*/
/*| PARAMETER    : ushort low    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort col    colmn = 0-29                              |*/
/*|              : ulong  dat    data                                      |*/
/*|              : ushort siz    data size                                 |*/
/*|              : ushort zer    0:0supress / 1:not / 2:'･' supress        |*/
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
/*| HEXﾃﾞｰﾀ全角表示                                                        |*/
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
/*| long型ﾊﾞｲﾅﾘﾃﾞｰﾀ全角表示(左寄せ用)                                      |*/
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
	if(dat / 10000)		/* 値が5桁ある */
	{
		nmdspll( low, col, (ulong)dat, 5, zer, mod, color, blink, 1 );
	}
	else if(dat / 1000)	/* 値が4桁ある */
	{
		nmdspll( low, col, (ulong)dat, 4, zer, mod, color, blink, 1 );
	}
	else if(dat / 100)	/* 値が3桁ある */
	{
		nmdspll( low, col, (ulong)dat, 3, zer, mod, color, blink, 1 );
	}
	else if(dat / 10)	/* 値が2桁ある */
	{
		nmdspll( low, col, (ulong)dat, 2, zer, mod, color, blink, 1 );
	}
	else				/* 上記以外(1桁) */
	{
		nmdspll( low, col, (ulong)dat, 1, zer, mod, color, blink, 1 );
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pntchr                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Ascii data ･ supress                                    |*/
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
		dat[ l_cnt ] = '･';
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

			buf.d = (ushort)(0x824f + ( cc - '0' )) ; // '０'

		} else if(( 'A' <= cc ) && ( cc <= 'Z' )){

			buf.d = (ushort)(0x8260 + ( cc - 'A' )); // 'Ａ'

		} else if(( 'a' <= cc ) && ( cc <= 'z' )){

			buf.d = (ushort)(0x8281 + ( cc - 'a' )); // 'ａ'

		} else if( cc == ' ' ) {

			buf.d = 0x8140; // '　';

		} else {

			buf.d = 0x8145; // '・';

		}
		*(dat2+j) = buf.c[0];
		j++;
		*(dat2+j) = buf.c[1];
		j++;
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 曜日を曜日用ｸﾞﾗﾌｨｯｸｱﾄﾞﾚｽへ変更                                         |*/
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
/*| 時計表示                                                               |*/
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
	// フロント基板へ時計表示指定

	uchar	d_buf[2];

	grachr(lin, 0, 30, 0, color, LCD_BLINK_OFF, OPE_CHR[1]);					// "    月    日（  ）     ：     "
	opedsp(lin, 0, (ushort)CLK_REC.mont, 2, 0, 0, color, LCD_BLINK_OFF);		// 月表示
	opedsp(lin, 6, (ushort)CLK_REC.date, 2, 0, 0, color, LCD_BLINK_OFF);		// 日表示

	wekchg( CLK_REC.week, d_buf );
	grachr(lin, 14, 2, 0, color, LCD_BLINK_OFF, d_buf );						// 曜日表示
	
	opedsp(lin, 19, (ushort)CLK_REC.hour, 2, 0, 0, color, LCD_BLINK_OFF);		// 時表示
	opedsp(lin, 25, (ushort)CLK_REC.minu, 2, 1, 0, color, LCD_BLINK_OFF);		// 分表示
}
/*[]----------------------------------------------------------------------[]*/
/*| 背景色設定                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dsp_background_color                                    |*/
/*| PARAMETER    : ushort rgb      rgb    		(R=5bit，G=6bit，B=5bit)　 |*/
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
	bkclr.Rgb	=	rgb;												//RGB設定
	can_snd_data4((void *)&bkclr, sizeof(t_BackColor));
	return;
}
/*[]----------------------------------------------------------------------[]*/
/*| 時計表示                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dsp_intime                                              |*/
/*| PARAMETER    : ushort lin    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ushort num    車室番号                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	dsp_intime( ushort lin, ushort num )
{
// MH322914(S) K.Onodera 2016/09/16 AI-V対応：遠隔精算(精算金額指定)
	// 金額指定遠隔精算 且つ、車両なしフラップダウン？
	if( (OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE && FLAPDT.flp_data[num-1].mode <= FLAP_CTRL_MODE3) || num == 0 ){
		return;
	}
// MH322914(E) K.Onodera 2016/09/16 AI-V対応：遠隔精算(精算金額指定)

	grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[19]);	// "入庫      月    日     ：     "

// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)
	// 入庫時刻指定遠隔精算
	if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_TIME ){
		opedsp(lin, 6, (ushort)g_PipCtrl.stRemoteTime.InTime.Mon,  2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 月表示
		opedsp(lin,12, (ushort)g_PipCtrl.stRemoteTime.InTime.Day, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// 日表示
		opedsp(lin,19, (ushort)g_PipCtrl.stRemoteTime.InTime.Hour,2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// 時表示
		opedsp(lin,25, (ushort)g_PipCtrl.stRemoteTime.InTime.Min, 2, 1, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// 分表示
		return;
	}
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)
	if( OPECTL.multi_lk ){											// ﾏﾙﾁ精算問合せ中?
		opedsp(lin, 6, (ushort)LOCKMULTI.lock_mlt.mont, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// 月表示
		opedsp(lin,12, (ushort)LOCKMULTI.lock_mlt.date, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// 日表示
		opedsp(lin,19, (ushort)LOCKMULTI.lock_mlt.hour, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// 時表示
		opedsp(lin,25, (ushort)LOCKMULTI.lock_mlt.minu, 2, 1, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);		// 分表示
	}else{
		opedsp(lin, 6, (ushort)FLAPDT.flp_data[num-1].mont, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 月表示
		opedsp(lin,12, (ushort)FLAPDT.flp_data[num-1].date, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 日表示
		opedsp(lin,19, (ushort)FLAPDT.flp_data[num-1].hour, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 時表示
		opedsp(lin,25, (ushort)FLAPDT.flp_data[num-1].minu, 2, 1, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 分表示
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
/*| 数値入力表示(BIGﾌｫﾝﾄ)                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : teninb                                                  |*/
/*| PARAMETER    : ushort nu   : 入力ﾊﾞｯﾌｧ                                 |*/
/*|              : ushort in_k : 表示桁数(MAX4桁)                          |*/
/*|              : ushort lin  : 表示行                                    |*/
/*|              : ushort col  : 表示桁(左端の文字)                        |*/
/*|              : ushort color: 表示色                                    |*/
/*| RETURN VALUE : ret : 表示値                                            |*/
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

// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け
//	for( i=0, wcol=0; i<in_k; i++, wcol+=3 ){
	for( i=0, wcol=0; i<in_k; i++, wcol+=2 ){
// MH810100(E) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け
		numchr( lin, (ushort)(col+wcol), color, LCD_BLINK_OFF, c_bf[i] );
	}
	return( nu );
}

/*[]----------------------------------------------------------------------[]*/
/*| 数値入力表示(BIGﾌｫﾝﾄ)longデータ                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : teninb                                                  |*/
/*| PARAMETER    : ulong nu   : 入力ﾊﾞｯﾌｧ                                 |*/
/*|              : ushort in_k : 表示桁数(MAX6桁)                          |*/
/*|              : ushort lin  : 表示行                                    |*/
/*|              : ushort col  : 表示桁(左端の文字)                        |*/
/*|              : ushort color: 表示色                                    |*/
/*| RETURN VALUE : ret : 表示値                                            |*/
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
/*| 数値入力表示(BIGﾌｫﾝﾄ)＜暗証番号＞                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : teninb                                                  |*/
/*| PARAMETER    : ushort nu   : 入力ﾊﾞｯﾌｧ                                 |*/
/*|              : ushort in_k : 表示桁数(MAX4桁)                          |*/
/*|              : ushort lin  : 表示行                                    |*/
/*|              : ushort col  : 表示桁(左端の文字)                        |*/
/*|              : uchar pas_cnt : カウンター                              |*/
/*|              : ushort color: 表示色                                    |*/
/*| RETURN VALUE : ret : 表示値                                            |*/
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
			//"-"を表示
			bigcr(lin, (ushort)(col+wcol), 2, color, LCD_BLINK_OFF, (uchar *)"－");	// "－"
		}else{
			p_cnt = (uchar)(4 - pas_cnt);
			numchr( lin, (ushort)(col+wcol), color, LCD_BLINK_OFF, c_bf[p_cnt] );
			pas_cnt--;
		}
	}
	return( nu );
}


/*[]----------------------------------------------------------------------[]*/
/*| 隠し暗証番号入力表示(BIGﾌｫﾝﾄ)                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : blindpasword                                            |*/
/*| PARAMETER    : ushort nu   : 入力ﾊﾞｯﾌｧ                                 |*/
/*|              : ushort in_k : 表示桁数(MAX4桁)                          |*/
/*|              : ushort lin  : 表示行                                    |*/
/*|              : ushort col  : 表示桁(左端の文字)                        |*/
/*| RETURN VALUE : ret : 表示値                                            |*/
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
			//"-"を表示
			bigcr(lin, (ushort)(col+wcol), 2, COLOR_FIREBRICK, LCD_BLINK_OFF, (uchar *)"－");	// "－"
		}else{
			bigcr(lin, (ushort)(col+wcol), 2, COLOR_FIREBRICK, LCD_BLINK_OFF, (uchar *)"Ｘ");	// "Ｘ"
			pas_cnt--;
		}
	}
	return( nu );
}

/*[]----------------------------------------------------------------------[]*/
/*| 駐車料金表示(BIGﾌｫﾝﾄ)                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : feedsp                                                  |*/
/*| PARAMETER    : ushort lin    line = 0-7(8line mode) / 0-6(7)           |*/
/*|              : ulong  fee    表示料金                                  |*/
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
/*| 接客LCDﾊﾞｯｸﾗｲﾄ制御                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LcdBackLightCtrl                                        |*/
/*| PARAMETER    : char onoff  0=OFF(待機時の輝度), 1=ON(精算時の輝度)     |*/
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
		case 0:														// 常時点灯
			onoff = 1;												// 常時ONとする
			backlight = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027⑤⑥ 点灯時の輝度
			break;
		case 1:														// ﾃﾝｷｰ入力にて点灯
		case 2:														// 外部信号にて点灯
		default:
			if(onoff == 0){											// OFFの場合
				backlight = (ushort)prm_get(COM_PRM, S_PAY, 28, 2, 1);	// 02-0028⑤⑥ 消灯時の輝度
			}else{													// ONの場合
				backlight = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027⑤⑥ 点灯時の輝度
			}
			// 輝度でON/OFFが決まる 輝度(LCD_contrast)0:バックライトOFF/0以外はバックライトON+輝度の設定値
			onoff = 1;
			break;
		case 3:														// 夜間時間帯のみ点灯
			set = (ushort)CPrmSS[S_PAY][30];
			tim1 = ( set / 100 ) * 60 + ( set % 100 );
			set = (ushort)CPrmSS[S_PAY][31];
			tim2 = ( set / 100 ) * 60 + ( set % 100 );
			if( tim1 == tim2 ){
				;													// 夜間時間帯無し
			}else if( tim1 < tim2 ){
				if(( tim1 <= CLK_REC.nmin )&&( CLK_REC.nmin <= tim2 )){
					nightMode = 1;
				}
			}else{
				if(( CLK_REC.nmin >= tim1 )||( tim2 >= CLK_REC.nmin )){
					nightMode = 1;
				}
			}
			if(nightMode == 0){										// 昼間時間帯の場合
				backlight = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027⑤⑥ 点灯時の輝度
			}else{													// ONの場合
				backlight = (ushort)prm_get(COM_PRM, S_PAY, 28, 2, 1);	// 02-0028⑤⑥ 消灯時の輝度
			}
			// 輝度でON/OFFが決まる 輝度(LCD_contrast)0:バックライトOFF/0以外はバックライトON+輝度の設定値
			onoff = 1;
			break;
	}
	lcd_backlight( onoff );
// MH810103(s) 電子マネー対応 待機画面での残高照会可否
	if(OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 1)){
// MH810103(s) 電子マネー対応 #5413 精算不可画面または警告画面表示時に、VPが残高照会ボタンを表示してしまっている
		// リアルタイム通信/LCD状態を取得 or 警告通知中
// MH810104 GG119201(S) 電子ジャーナル対応 #5973 電子ジャーナル利用不可の発生中、電子マネーの残高照会ができなくなる
//		if( ERR_CHK[mod_realtime][1] || ERR_CHK[mod_tklslcd][1] || lcdbm_alarm_check()){
		if( ERR_CHK[mod_realtime][1] || ERR_CHK[mod_tklslcd][1] ){
// MH810104 GG119201(E) 電子ジャーナル対応 #5973 電子ジャーナル利用不可の発生中、電子マネーの残高照会ができなくなる
			if( isEC_USE() ){
				if(OPECTL.lcd_query_onoff != 0){	// 変化したら
					Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );
				}
			}
			OPECTL.lcd_query_onoff = 0;								// 待機画面での残高照会可否(0:NG/1:OK/2:その他)
			return;
		}
// MH810103(e) 電子マネー対応 #5413 精算不可画面または警告画面表示時に、VPが残高照会ボタンを表示してしまっている
		// 待機の場合送信する
		// 残高可否をチェック
		if(isEcEmoneyEnabledNotCtrl(1,1)){
			if( isEC_USE() ){
				if(OPECTL.lcd_query_onoff != 1){	// 変化したら
					Ec_Pri_Data_Snd( S_CNTL_DATA, 0x1 );
				}
			}
			OPECTL.lcd_query_onoff = 1;								// 待機画面での残高照会可否(0:NG/1:OK/2:その他)
		}else{
			OPECTL.lcd_query_onoff = 0;								// 待機画面での残高照会可否(0:NG/1:OK/2:その他)
		}
	}else{
		OPECTL.lcd_query_onoff = 2;								// 待機画面での残高照会可否(0:NG/1:OK/2:その他)
	}
// MH810103(e) 電子マネー対応 待機画面での残高照会可否
}

/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝ画面表示                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpeLcd( no )                                            |*/
/*| PARAMETER    : no = 画面№                                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpeLcd( ushort no )
{
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(不要処理削除)
//	ushort	loklen, Mloklen, lin;
//	ulong	w_tax;
//	ulong	wkl;
//
//// 車種表示用変数
//	uchar	r_syu = 0;		// パラメータ番号格納
//	ushort syasyu = 0;		// 車種番号格納
//	uchar	ryosyu = 0;		//料金種別格納
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
//		// NOTE:全画面表示切替をした時はCANへの表示データが多く発生するのでWAITをおく。未了発生時は表示しきれなかったため
//		// 本来はCANの送信バッファを拡大すべきであるが構造上変更のリスクが多きいためこの体策にとどめておく
//		xPause(4);// 40ms wait
//		switch (no) {
//		case 17:		// 休業中
//		case 18:		// 警告
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
//		case 1:														// 待機状態画面表示
//			dspclk(0, COLOR_DARKSLATEBLUE);							// 時計表示
//
//			if( OPECTL.Pay_mod == 1 ){								// ﾊﾟｽﾜｰﾄﾞ忘れ処理?
//				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[35]);		// "  暗証番号忘れ精算を行います  "
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[5]);		// "精算される方は                "
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[6]);		// "    車室番号を入力して下さい  "
//			}else{
//				if( OPECTL.Seisan_Chk_mod == ON ){
//					grachr(1, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[65]);	// "          検査モード          " 反転表示
//				}
//
//#if	UPDATE_A_PASS
//				// 定期券更新機能あり／なしによりlinで表示する行数を補正する
//				if( CPrmSS[S_KOU][1] && LprnErrChk( 1 ) && prm_get( COM_PRM,S_KOU,26,1,1 ) ){
//					// 更新ありでﾗﾍﾞﾙ関連ｴﾗｰありで更新不可
//					grachr( 2, 0, 30, 1, COLOR_BLACK,
//							LCD_BLINK_OFF, OPE_CHR2[7] );			// " 只今、定期券更新は出来ません "反転表示
//					grachr( 5, 0, 30, 0, COLOR_BLACK,
//							LCD_BLINK_OFF, OPE_CHR[6] );			// "              入力して下さい  "
//					lin = 1;
//				}else{
//					lin = 0;
//				}
//#else
//				lin = 0;
//#endif	// UPDATE_A_PASS
//
//
//				set62 = (uchar)prm_get( COM_PRM,S_TYP,62,1,3 );		// 駐車証明書発行可能枚数取得
//				set63 = (uchar)prm_get( COM_PRM,S_TYP,63,1,1 );
//				if( set62 && set63 ){				 				// 受付券&暗証番号登録あり?
//					grachr((ushort)(2+lin), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[3]);		// "暗証番号登録、受付券発行、    "
//				}
//				else if( set62 ){									// 受付券あり?
//					grachr((ushort)(2+lin), 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, &OPE_CHR[3][14]);	// "駐車証明書発行、              "
//				}
//				else if( set63 ){									// 暗証番号登録あり?
//					grachr((ushort)(2+lin), 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[3]);		// "暗証番号登録、                "
//				}
//
//				grachr((ushort)(3+lin), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[5]);			// "精算の方は                    "
//
//				if( lin == 0 ){
//#if	UPDATE_A_PASS
//					if( CPrmSS[S_KOU][1] ){
//						grachr( 4, 0, 30, 0, COLOR_BLACK,
//								LCD_BLINK_OFF, OPE_CHR2[0] );		// "定期券更新の方は更新キーを    "
//						grachr( 5, 0, 30, 0, COLOR_BLACK,
//								LCD_BLINK_OFF, OPE_CHR[6] );		// "              入力して下さい  "
//					}else{
//						grachr( 4, 0, 30, 0, COLOR_BLACK,
//								LCD_BLINK_OFF, OPE_CHR[6] );		// "              入力して下さい  "	
//					}
//#else
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[6]);					// "    車室番号を入力して下さい  "
//#endif	// UPDATE_A_PASS
//				}
//
//			}
//
//			dspCyclicErrMsgRewrite();
//
//// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////			if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 && EDY_TEST_MODE ){	// Edy設定がありかつﾃｽﾄﾓｰﾄﾞ設定
////				grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);				// "                   "
////				grachr(1, 6, 18, 1, COLOR_BLACK, LCD_BLINK_OFF, EdyTestModeStr[0]);			// " Eｄｙテストモード "
////			}
//// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			op_mod01_dsp();
//			break;
//
//		case 2:														// 駐車位置番号入力画面表示
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[8]);		// "車室番号を入力して下さい      "
//			grawaku( 1, 4 );
//			bigcr(2, 2, 8, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "  車室番号                    "
//			lin = 7;
//			if ((prm_get(COM_PRM, S_TYP, 63, 1, 1)) && (OPECTL.Pay_mod == 0)) {			// 暗証番号登録あり＆通常精算
//				grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[10]);	// "   暗証番号の登録  →  登録   "
//				lin--;
//			}
//			if (prm_get(COM_PRM, S_TYP, 62, 1, 3)) {									// 駐車証明書発行設定あり
//				grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[11]);	// "  駐車証明書の発行 →  駐証   "
//				lin--;
//			}
//			if (lin == 7) {
//				lin--;
//			}
//			grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[12]);		// "   駐車料金の精算  →  精算   "
//			dsp_background_color(COLOR_WHITE);
//			teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示
//			bigcr(2, 14, 2, COLOR_FIREBRICK, LCD_BLINK_OFF, (uchar *)&OPE_ENG[key_sec*2]);	// 区画表示
//// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////			if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 && EDY_TEST_MODE ){	// Edy設定がありかつﾃｽﾄﾓｰﾄﾞ設定
////				grachr(1, 6, 18, 1, COLOR_BLACK, LCD_BLINK_OFF, EdyTestModeStr[0]);			// " Eｄｙテストモード "
////			}
//// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			break;
//
//		case 3:														// 料金表示(初回)
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			bigcr(0, 0, 20, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[0]);	// "料金　　　　　　　円"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "━━━━━━━━━━━━━━━"
//			feedsp(0, ryo_buf.dsp_ryo);
//			if( Ex_portFlag[EXPORT_CHGNEND] == 0 ){					// 0:釣銭切れ予告無し/1:釣銭切れ予告有り
//				/*** 釣銭有り時表示 ***/
//				// 接客画面 メッセージ表示処理
//				dspCyclicMsgInit();
//				for (i = 0; i < 3; i++) {
//					// 共通パラメータを引数に、１行表示関数コール → ３回コール
//					msgno = (uchar)prm_get(COM_PRM, S_DSP, 4, 2, (char)((4-(i*2))+1));
//					dsp_msg(msgno, (uchar)(i+3), 6, COLOR_BLACK);
//				}
//			}else{
//				/*** 釣銭無し時表示 ***/
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[56]);	// "只今釣銭が不足しております    "
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[60]);	// "釣銭の無い様に精算して下さい  "
//				msgno = 40;											// 釣り切れ時はTカード使用設定専用
//				dsp_msg(msgno, 5, 6, COLOR_BLACK);					// "Ｔカードでポイントが貯まります"
//			}
//			grachr(6, 0, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "車室番号                      "
//			ryosyu = (uchar)(ryo_buf.syubet + 1);
//
//			if( (ryosyu >= 1) && (ryosyu <= 6 ) ){		//Ａ～Ｆ
//				r_syu = (uchar)(7 - ryosyu);			//
//				syasyu = (ushort)prm_get( 0,S_TYP,(short)(121),1,(char)r_syu );
//			}else if( (ryosyu >= 7) && (ryosyu <= 12 ) ){	//Ｇ～Ｌ
//				r_syu = (uchar)(13 - ryosyu);				//
//				syasyu = (ushort)prm_get( 0,S_TYP,(short)(122),1,(char)r_syu );
//			}
//			
//			opedsp(6, 10, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			grachr(6, (ushort)(10+loklen*2), 10, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar*)Sya_Prn[syasyu]);	// "            （ＸＸＸ）        "
//
//			dsp_intime(7, OPECTL.Pr_LokNo);				// 入庫時刻表示
//			dsp_background_color(COLOR_WHITE);
//			break;
//
//		case 4:														// 料金表示(入金又は割引後)
//			bigcr(0, 0, 20, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[1]);	// "あと　　　　　　　円"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "━━━━━━━━━━━━━━━"
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[21] );				// "    料金                  円  "
//			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[22] );				// "    入金                  円  "
//			// 5行目消去(空白を表示)
//			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[0]);
//			feedsp( 0, ryo_buf.zankin );
//			w_tax = 0L;
//			if(( CPrmSS[S_CAL][19] )&&( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2L )){	// 税率設定あり & 外税?
//				w_tax = ryo_buf.tax;
//			}
//			if( ryo_buf.ryo_flg >= 2 ){								// 定期使用?
//				opedpl(3, 14, ryo_buf.tei_ryo + w_tax, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//			}else{
//				opedpl(3, 14, ryo_buf.tyu_ryo + w_tax, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//			}
//			opedpl(4, 14, ryo_buf.nyukin, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//
//			wkl = ryo_buf.waribik;									// 精算中止券の預かり額は waribikに入る(TF7700)
//
//			if( 2 == CPrmSS[S_PRP][1] ){							// 回数券使用可能
//				if(Flap_Sub_Flg == 1){
//					if( wkl >= (KaisuuWaribikiGoukei + PayData_Sub.pay_ryo) )
//						wkl -= (KaisuuWaribikiGoukei + PayData_Sub.pay_ryo);	// ryo_buf.waribik から 回数券分を引く
//				}else{
//					if( KaisuuWaribikiGoukei ){						// 回数券利用あり（今回）
//						if( wkl >= KaisuuWaribikiGoukei )
//							wkl -= KaisuuWaribikiGoukei;			// ryo_buf.waribik から 回数券分を引く
//					}
//				}
//			}else if( 1 == CPrmSS[S_PRP][1] ){
//				if(Flap_Sub_Flg == 1){
//					if( PayData_Sub.pay_ryo ){							// プリペイド利用あり（中止）
//						if( wkl >= PayData_Sub.pay_ryo )
//							wkl -= (PayData_Sub.pay_ryo);	// ryo_buf.waribik から プリペイド分を引く
//					}
//				}
//			}
//
//			if( 0L != wkl ){										// 割引あり？
//				if(OPECTL.Pay_mod != 2){							// 修正精算以外
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[23]);	// "    割引                  円  "
//				}else{												// 修正精算の時
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[52]);	// "  割引(現金含)            円  "
//				}
//				opedpl(5, 14, wkl, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//			}
//
//			if(Flap_Sub_Flg == 1){								//再精算時
//				if( (PayData_Sub.pay_ryo != 0) && (Ope_Last_Use_Card == 0)){ // プリペイド、回数券にて精算中止後
//					if( 1 == CPrmSS[S_PRP][1]){
//						wk_pri = PayData_Sub.pay_ryo + c_pay;
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,OPE_CHR[67]);	// "    プリペイド            円  "
//						opedpl(6, 14, (ulong)wk_pri, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//					}
//					else if( 2 == CPrmSS[S_PRP][1] ){						// 回数券利用あり？
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[72]);	// "    回数券                円  "
//						opedpl(6, 14, (KaisuuWaribikiGoukei+PayData_Sub.pay_ryo), 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//					}
//				}else{
//					if( e_pay || c_pay || KaisuuWaribikiGoukei || PayData_Sub.pay_ryo ){
//
//// MH321800(S) T.Nagai ICクレジット対応
//						i = 16;
//// MH321800(E) T.Nagai ICクレジット対応
//						WkCal = (ulong)(e_pay + c_pay + PayData.ppc_chusi_ryo + KaisuuWaribikiGoukei + PayData_Sub.pay_ryo + ryo_buf.credit.pay_ryo);
//
//						switch( Ope_Last_Use_Card ){						// 最後に使ったｶｰﾄﾞ
//						case	1:		// "プリペイド"
//							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[67]);		// "  プリペイド            円    "
//							break;
//						case	2:		// "　回数券　"
//							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[72] );		// "    回数券              円    "
//							break;
//						case	3:		// 電子決済カード
//							wk_MsgNo = Ope_Disp_Media_GetMsgNo( 1, 0, 0 );
//							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "電子媒体                円    "
//// MH321800(S) T.Nagai ICクレジット対応
//							if (isEC_USE()) {
//							// マルチ電子マネー：電子マネー支払額のみ表示
//								WkCal = e_pay;
//								i = 14;
//							}
//// MH321800(E) T.Nagai ICクレジット対応
//							break;
//						case	4:		// "前回支払い"
//							break;
//						}
//						if (Ope_Last_Use_Card != 6) {
//// MH321800(S) T.Nagai ICクレジット対応
////							opedpl(6, 16, WkCal, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//							opedpl(6, (ushort)i, WkCal, (i == 16)? 5 : 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//// MH321800(E) T.Nagai ICクレジット対応
//						}
//						grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);			// "                              "
//					}
//				}
//			}else{
//				/** プリペ／回数券／電子決済カード／前回支払い 表示 **/
//				if( e_pay || c_pay || KaisuuWaribikiGoukei ){
//// MH321800(S) T.Nagai ICクレジット対応
//					i = 16;
//// MH321800(E) T.Nagai ICクレジット対応
//					WkCal = (ulong)(e_pay + c_pay + PayData.ppc_chusi_ryo + KaisuuWaribikiGoukei);
//					switch (Ope_Last_Use_Card) {						// 最後に使ったｶｰﾄﾞ
//					case 1:		// "プリペイド"
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[67]);				// "  プリペイド            円    "
//						break;
//					case 2:		// "　回数券　"
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[72]);				// "    回数券              円    "
//						break;
//					case 3:		// 電子決済カード
//						wk_MsgNo = Ope_Disp_Media_GetMsgNo( 1, 0, 0 );
//						grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);			// "電子媒体                円    "
//// MH321800(S) T.Nagai ICクレジット対応
//						if (isEC_USE()) {
//						// マルチ電子マネー：電子マネー支払額のみ表示
//							WkCal = e_pay;
//							i = 14;
//						}
//// MH321800(E) T.Nagai ICクレジット対応
//						break;
//					case 4:		// "前回支払い"
//						break;
//					}
//					if (Ope_Last_Use_Card != 6) {
//// MH321800(S) T.Nagai ICクレジット対応
////						opedpl(6, 16, WkCal, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//						opedpl(6, (ushort)i, WkCal, (i == 16)? 5 : 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//// MH321800(E) T.Nagai ICクレジット対応
//					}
//					grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);					// "                              "
//				}
//			}
//
//			if( OPECTL.PassNearEnd == 1 ){							// 期限切れ間近
//				grachr(7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[10] );						// "このｶｰﾄﾞはもうすぐ期限切れです"
//			}else{
//				Lcd_Receipt_disp();
//			}
//			if( (OPECTL.Pay_mod == 2)&&(prm_get(COM_PRM, S_TYP, 98, 1, 3)) ){	// 修正精算でかつ強制精算完了ｷｰあり
//				grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[48] );				// "    強制精算完了   →  駐証   "
//			}
//			break;
//
//		case 5:														// 精算完了表示
//			bigcr(0, 0, 20, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[3]);				// "おつり　　　　　　円"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2] );							// "━━━━━━━━━━━━━━━"
//			if( (OPECTL.PriUsed==1) && (CPrmSS[S_PRP][1] == 1) ){	// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用？（回数券使用時もPriUsed=1になっているので設定ﾁｪｯｸ）
//				if ( 99 == prm_get( COM_PRM,S_PRP,11,2,1 ) ){
//					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[69]);					// "プリペイド残額            円  "
//					opedpl(6, 16, OPECTL.LastUsePCardBal, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//				}else{
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[69]);					// "プリペイド残額            円  "
//					opedpl(5, 16, OPECTL.LastUsePCardBal, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)PrcKigenStr );			// "有効期限２０１１年　７月２０日"
//				}
//			}
//			// 接客画面 メッセージ表示処理 (4,5,6行目を設定値で表示)
//			dspCyclicMsgInit();
//// GG119202(S) iD決済時の案内表示対応
//			if (e_inflg != 0 &&
//				Ec_Settlement_Res.brand_no == BRANDNO_ID &&
//				PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin != 0 &&
//				PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0) {
//				// iD決済でみなし決済＋決済OK受信の場合
//				grachr( 4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[164] );		// " エラー発生でお時間を頂くため "
//				grachr( 5, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[165] );		// "       支払完了とします       "
//				grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[156] );		// "電子マネー支払             円 "
//				opedpl( 6, 15, (ulong)e_pay, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//			}
//			else {
//// GG119202(E) iD決済時の案内表示対応
//			for (i = 0; i < 3; i++) {
//				if( (OPECTL.PriUsed==1) && (CPrmSS[S_PRP][1] == 1) ){
//					if ( (i == 2) && (99 != prm_get( COM_PRM,S_PRP,11,2,1 )) ){
//						break;	// 6行目は 有効期限で使用 //
//					}
//				}
//				// 共通パラメータを引数に、１行表示関数コール → ３回コール
//				msgno = (uchar)prm_get(COM_PRM, S_DSP,  1, 2, (char)((4-(i*2))+1));
//				dsp_msg(msgno, (uchar)(i+3), 15, COLOR_BLACK);
//			}
//			if( OPECTL.Pay_mod == 2 ){								// 修正精算の時
//				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[53] );						// "    修正処理が完了しました    "
//			}
//			if( e_inflg != 0 ){										// Suica使用
//				wk_MsgNo = Ope_Disp_Media_GetMsgNo( 1, 1, 0 );
//// MH321800(S) hosoda ICクレジット対応
//				if (wk_MsgNo == 156) {								// 決済リーダでの精算の場合
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);				// "ＸＸＸＸＸ支払             円 "
//					opedpl(5, 15, (ulong)e_pay, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[157]);					// "          残高             円 "
//					opedpl(6, 15, (ulong)e_zandaka, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//				}
//				else {
//// MH321800(E) hosoda ICクレジット対応
//				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);					// "ＸＸＸＸＸ利用額           円 "
//				opedpl(5, 17, (ulong)e_pay, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
////				if( wk_MsgNo == 109 ){								// Edyでの精算の場合
////					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[114]);					// "          残高             円 "
////				} else {
//				{
//// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
//					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[95]);					// "            残額           円 "
//				}
//				opedpl(6, 17, (ulong)e_zandaka, 5, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//// MH321800(S) hosoda ICクレジット対応
//				}
//// MH321800(E) hosoda ICクレジット対応
//// GG119202(S) iD決済時の案内表示対応
//			}
//// GG119202(E) iD決済時の案内表示対応
//			}
//
//			if( OPECTL.PassNearEnd == 1 ){							// 期限切れ間近
//				grachr(7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[10]);							// "このｶｰﾄﾞはもうすぐ期限切れです"
//			}else{
//				Lcd_Receipt_disp();
//			}
//			WkCal = ryo_buf.turisen;
//			if( OPECTL.Pay_mod == 2 ){								// 修正精算中
//				if(	(prm_get(COM_PRM, S_TYP, 98, 1, 1)==1)||		// 修正元現金入金額の払戻あり設定
//					(Syusei_Select_Flg == 2) ){						// 修正元現金入金額の払戻あり選択
//						if( ryo_buf.mis_tyu ){						// 修正元入金額の払戻額あり
//							WkCal = ryo_buf.mis_tyu;
//						}
//				}
//			}
//// 仕様変更(S) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
//			// 振替過払い金あり？
//			if( ryo_buf.kabarai ){
//				WkCal = ryo_buf.kabarai;
//			}
//// 仕様変更(E) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
//			feedsp( 0, WkCal );
//			break;
//
//		case 6:														// 受付券発行OK表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			bigcr(1, 0, 20, COLOR_BLACK, LCD_BLINK_OFF, (uchar *)BIG_CHR[4]);	// "駐車証明書を　　　　"
//			bigcr(3, 0, 20, COLOR_BLACK, LCD_BLINK_OFF, (uchar *)BIG_CHR[5]);	// "　　　　お取り下さい"
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      車室番号                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//
//		case 7:														// 受付券発行済みNG表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[54]);			// "  この車室番号の駐車証明書は  "
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[55]);			// "  発行限度枚数を超えています  "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "      車室番号                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//
//		case 8:														// ﾊﾟｽﾜｰﾄﾞ登録画面表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,OPE_CHR[37]);			// "暗証番号を４桁で入力して下さい"
//			grachr(1, 2, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "  車室番号                    "
//			grawaku( 2, 5 );										// 飾り枠表示
//			bigcr(3, 2, 8, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[37]);		// "  暗証番号                    "
//			set62 = (uchar)prm_get( COM_PRM,S_TYP,62,1,3 );			// 駐車証明書発行可能枚数取得
//			lin = 6;
//			if( set62 ){											// 受付券発行機能あり?
//				pri_time_flg = pri_time_chk(OPECTL.Pr_LokNo,0);
//				if( pri_time_flg != 0 ){
//					grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[11]);	// "  駐車証明書の発行 →  駐証   "
//					lin++;
//				}
//			}
//			grachr(lin, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[10]);			// "   暗証番号の登録  →  登録   "
//			teninb_pass(0, 4, 3, 16, 0, COLOR_FIREBRICK);
//			opedsp(1, 12, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//
//		case 9:														// 暗証番号入力画面表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr( 1, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[41] );					// "  登録された暗証番号を        "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[145] );					// "              入力して下さい  "
//			grachr(4, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "      車室番号                "
//			opedsp(4, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[4]);			// "      暗証番号  ○○○○      "
//			break;
//
//		case 10:													// 修正精算駐車位置番号入力画面
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "＜修正精算＞                  "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[43] );					// "間違えた車室番号              "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[44] );					// "正しい車室番号                "
//			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[45]);	// " 番号入力後精算を押して下さい "
//			if( CPrmSS[S_TYP][81] ){								// 区画使用設定有り?
//				grachr( 2, 20, 2, 1, COLOR_BLACK,
//						LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );	// 区画表示
//				opedsp( 2, 22, 0, knum_len, 1, 1,
//						COLOR_BLACK, LCD_BLINK_OFF );			// 駐車位置番号0表示
//				grachr( 3, 20, 2, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, &OPE_ENG[ 1 * 2 ] );			// 区画表示
//				opedsp( 3, 22, 0, knum_len, 1, 0,
//						COLOR_BLACK, LCD_BLINK_OFF );			// 駐車位置番号0表示
//			}else{
//				opedsp( 2, 22, 0, knum_len, 1, 1,
//						COLOR_BLACK, LCD_BLINK_OFF );			// 駐車位置番号0表示
//				opedsp( 3, 22, 0, knum_len, 1, 0,
//						COLOR_BLACK, LCD_BLINK_OFF );			// 駐車位置番号0表示
//			}
//			dspclk(7, COLOR_DARKSLATEBLUE);							// 時計表示
//			break;
//
//		case 11:													// 修正精算料金表示(違＞正)
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			Mloklen = binlen( OPECTL.MOp_LokNo % 10000L );
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );										// "＜修正精算＞                  "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[44] );										// "正しい車室番号                "
//			if( Syusei_Select_Flg == 1 ){
//				grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[49]);	// "     硬貨で戻す    →  精算   "
//				grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[50]);	// "      戻さない     →  領収   "
//			}else{
//				grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[132]);	// "   確認後精算を押して下さい   "
//			}
//			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[43]);				// "間違えた車室番号              "
//			grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[131]);		// "精算      月    日     ：     "
//			grachr(7, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[133]);		// "精算追番      No.００００００ "
//
//			opedsp(2, (ushort)(28-loklen*2), (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			opedsp(5, (ushort)(28-Mloklen*2), (ushort)(OPECTL.MOp_LokNo % 10000L), Mloklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//
//			opedsp(6, 6, (ushort)syusei[OPECTL.MPr_LokNo-1].omont, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 月表示
//			opedsp(6, 12, (ushort)syusei[OPECTL.MPr_LokNo-1].odate, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 日表示
//			opedsp(6, 19, (ushort)syusei[OPECTL.MPr_LokNo-1].ohour, 2, 0, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 時表示
//			opedsp(6, 25, (ushort)syusei[OPECTL.MPr_LokNo-1].ominu, 2, 1, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF);	// 分表示
//
//			opedpl(7,17, (ulong)CountSel(&syusei[OPECTL.MPr_LokNo-1].oiban), 6, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);				// 追番表示
//			break;
//
//#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
//		case 12:													// 修正精算料金表示(違＜正)
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			Mloklen = binlen( OPECTL.MOp_LokNo % 10000L );
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "＜修正精算＞                  "
//			grachr( 1, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[43] );					// "間違えた車室番号              "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[21] );					// "    料金                  円  "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[44] );					// "正しい車室番号                "
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[21] );					// "    料金                  円  "
//			grachr( 5, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[22] );					// "    入金                  円  "
//			grachr( 6, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[51] );					// "    あと                  円  "
//			grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);	// " 必要な方は領収を押して下さい "
//
//			grachr( 1, 20, 2, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, &OPE_ENG[ ( OPECTL.MOp_LokNo / 10000L )* 2 ] );
//			opedsp( 1, 22, (ushort)( OPECTL.MOp_LokNo % 10000L ), Mloklen, 1, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 2, 14, ryo_buf.mis_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// 間違えた料金表示
//			grachr( 3, 20, 2, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, &OPE_ENG[ ( OPECTL.Op_LokNo / 10000L )* 2 ] );
//			opedsp( 3, 22, (ushort)( OPECTL.Op_LokNo % 10000L ), loklen, 1, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 4, 14, ryo_buf.dsp_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// 正しい料金表示
//			opedpl( 5, 14, ryo_buf.nyukin, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// 入金額表示
//			opedpl( 6, 14, ryo_buf.dsp_ryo - ryo_buf.mis_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// 残額表示
//			break;
//
//		case 13:													// 修正精算完了表示(同額)
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			Mloklen = binlen( OPECTL.MOp_LokNo % 10000L );
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "＜修正精算＞                  "
//			grachr( 1, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[43] );					// "間違えた車室番号              "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[21] );					// "    料金                  円  "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[44] );					// "正しい車室番号                "
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[21] );					// "    料金                  円  "
//			grachr( 5, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[52] );					// "間違えた番号の料金と同額です  "
//			grachr( 7, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[53] );					// "    修正処理は完了しました    "
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
//		case 14:													// 修正精算完了表示(釣無し)
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "＜修正精算＞                  "
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[53] );					// "    修正処理は完了しました    "
//			break;
//
//		case 15:													// 修正精算完了表示(釣有り)
//			grachr( 0, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[9] );					// "＜修正精算＞                  "
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[16] );					// "おつり                      円"
//			feedsp( 1, ryo_buf.turisen );
//			grachr( 6, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[53] );					// "    修正処理は完了しました    "
//			break;
//#endif		// SYUSEI_PAYMENT
//
//		case 16:													// 修正精算料金表示(入金又は割引後)
//			opedpl( 4, 16, ryo_buf.tyu_ryo, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 5, 16, ryo_buf.nyukin, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			opedpl( 6, 16, ryo_buf.zankin, 6, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );
//			if( OPECTL.RECI_SW == 1 ){								// 領収書ﾎﾞﾀﾝ使用?
//				wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
//				grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      領収証を発行します      "
//			}else{
//				grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);		// " 必要な方は領収を押して下さい "
//			}
//			break;
//
//		case 17:													// 休業中画面表示
//			/****************************************************/
//			/* CLOSE_stat = 0:営業中							*/
//			/*              1:強制営休業(I)						*/
//			/*              2:営業時間外(只今、営業時間外です)	*/
//			/*              3:紙幣金庫満杯(N)					*/
//			/*              4:ｺｲﾝ金庫満杯(C)					*/
//			/*              5:紙幣ﾘｰﾀﾞｰ脱落(F)					*/
//			/*              6:釣銭切れ(E)						*/
//			/*              7:ｺｲﾝﾒｯｸ通信不良(T)					*/
//			/*              8:NT-NET 送信ﾊﾞｯﾌｧFULL(NT)			*/
//			/*              9:ﾌﾗｯﾌﾟ/ﾛｯｸ装置手動ﾓｰﾄﾞ(手動モード中です)	*/
//			/*             10:信号で休業指示					*/
//			/*             11:通信で休業指示					*/
//			/****************************************************/
//			/* 休業理由表示する／しない */
//			if( (0 == (uchar)prm_get(COM_PRM, S_DSP, 35, 1, 1)) ||	// 休業表示する or 
//				(2 == CLOSE_stat) ){								// 営業時間外(これは常に表示)
//				grachr(0, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, CLSMSG[CLOSE_stat]);	// 休業理由表示
//			}
//			bigcr(2, 0, 26, COLOR_WHITE, LCD_BLINK_OFF, (uchar *)&BIG_OPE[0]);		// "      休     業     中    "
//			dspclk(7, COLOR_WHITE);							// 時計表示
//			// 接客画面 メッセージ表示処理
//			dspCyclicMsgInit();
//			for (i = 0; i < 3; i++) {
//				// 共通パラメータを引数に、１行表示関数コール → ３回コール
//				msgno = (uchar)prm_get(COM_PRM, S_DSP, 18, 2, (char)((4-(i*2))+1));
//				dsp_msg(msgno, (uchar)(i+4), 18, COLOR_BLACK);
//			}
//			break;
//
//		case 18:													// ﾄﾞｱ閉時のｱﾗｰﾑ表示
//			/*** ｱﾗｰﾑ表示は最大4つまで表示 ***/
//			lin = 2;
//			bigcr(0, 0, 20,COLOR_RED, LCD_BLINK_OFF, (uchar *)BIG_CHR[6]);				// "　　　警　　告　　　"
//			if( OPE_SIG_DOOR_Is_OPEN == 0 ){						// ﾄﾞｱ閉?
//				if (RAMCLR) {										// メモリイニシャル実施？
//					grachr(lin++, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, SMSTR2[21]);		// "　 精算機の再起動が必要です 　"
//					grachr(lin++, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, SMSTR2[22]);		// "　　 電源のOFF/ONを行い、 　　"
//					grachr(lin++, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, SMSTR2[23]);		// "　精算機を再起動してください　"
//					break;
//				}
//				if(prm_get(COM_PRM, S_PAY, 33, 1, 1) == 1) {		// ドア閉時のコイン金庫警告あり
//					if( FNT_CN_BOX_SW ){								// ｺｲﾝ金庫ｾｯﾄ?
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[0]);	// "ｺｲﾝ金庫をｾｯﾄして下さい        "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 33, 1, 2) == 1) {		// ドア閉時のコインカセット警告あり
//					if( CN_RDAT.r_dat0c[1] & 0x40 ){					// ｺｲﾝｶｾｯﾄ開
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[1]);	// "ｺｲﾝｶｾｯﾄをｾｯﾄして下さい        "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 33, 1, 3) == 1) {		// ドア閉時のコインメック鍵警告あり
//					if( FNT_CN_DRSW ){									// ｺｲﾝﾒｯｸﾄﾞｱｽｲｯﾁ開(ｺｲﾝﾒｯｸﾄﾞｱｷｰ開)?
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[2]);	// "ｺｲﾝﾒｯｸの鍵を閉めて下さい      "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 33, 1, 5) == 1) {		// ドア閉時の紙幣リーダー警告あり
//					if( NT_RDAT.r_dat1b[0] & 0x02 ){					// ｽﾀｯｶｰ開?
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[3]);	// "紙幣ﾘｰﾀﾞｰのﾌﾀを閉めて下さい   "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 33, 1, 6) == 1) {		// ドア閉時の紙幣金庫鍵警告あり
//					if( FNT_NT_BOX_SW ){								// 紙幣金庫脱着ｽｲｯﾁ開?
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[4]);	// "紙幣ﾘｰﾀﾞｰの鍵を閉めて下さい   "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 34, 1, 1) == 1) {		// ドア閉時のレシートプリンタ警告あり
//					if( 0 != ALM_CHK[ALMMDL_SUB][ALARM_RPAPEROPEN] ){	// ﾚｼｰﾄﾌﾟﾘﾝﾀ蓋開
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[5]);	// "ﾚｼｰﾄﾌﾟﾘﾝﾀのﾌﾀを閉めて下さい   "
//						if (lin >= 6) {
//							break;
//						} else {
//							lin++;
//						}
//					}
//				}
//				if (prm_get(COM_PRM, S_PAY, 34, 1, 3) == 1 &&		// ドア閉時のジャーナルプリンタ警告あり
//					prm_get(COM_PRM, S_PAY, 21, 1, 1) == 1) {		// ジャーナルプリンタあり
//					if( 0 != ALM_CHK[ALMMDL_SUB][ALARM_JPAPEROPEN] ){	// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ蓋開
//						grachr(lin, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, DRCLS_ALM[6]);	// "ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀのﾌﾀを閉めて下さい "
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
//		case 19:													// ﾊﾟｽﾜｰﾄﾞ登録済み画面表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[36] );					// "この車室番号の暗証番号は      "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[24] );								// "                  登録済みです"
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      車室番号                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//
//#if	UPDATE_A_PASS
//		case 20:													// 定期券更新 券挿入待ち画面表示
//			dspclk(0 COLOR_DARKSLATEBLUE);							// 時計表示
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[1] );					// "更新する定期券を挿入して下さい"
//			grachr( 5, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[2] );					// "表示された料金を精算すると    "
//			grachr( 6, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[3] );					// "          定期券が更新されます"
//			break;
//
//		case 21:													// 定期券更新 料金表示(初回)
//			bigcr(0, 0, 30, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[0]);	// "料金                        円"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "━━━━━━━━━━━━━━━"
//			if( Ex_portFlag[EXPORT_CHGNEND] == 0 ){					// 0:釣銭切れ予告無し/1:釣銭切れ予告有り
//				/*** 釣銭有り時表示 ***/
//				grachr( 4, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR[18] );				// "硬貨、又は紙幣で精算して下さい"
//			}else{
//				/*** 釣銭無し時表示 ***/
//				grachr( 3, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR[56] );				// "只今釣銭が不足しております    "
//				grachr( 4, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR[60] );				// "釣銭の無い様に精算して下さい  "
//			}
//			feedsp( 0, ryo_buf.dsp_ryo );
//			grachr( 6, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[4] );					// "    ヶ月更新後の終了日は      "
//			opedsp( 6, 0, RenewalMonth, 2, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// 月数表示
//			grachr( 7, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR2[5] );					// "            年    月    日です"
//			opedsp( 7, 4, (ushort)PayData.teiki.e_year, 4, 1, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// 年表示
//			opedsp( 7, 14, (ushort)PayData.teiki.e_mon, 2, 0, 0,
//					COLOR_BLACK, LCD_BLINK_OFF );				// 月表示
//			opedsp( 7, 20, (ushort)PayData.teiki.e_day, 2, 0, 0 );	// 日表示
//			break;
//
//		case 23:													// 定期券更新 精算完了表示
//			bigcr(0, 0, 30, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)BIG_CHR[3] );	// "おつり                      円"
//			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "━━━━━━━━━━━━━━━"
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[25] );					// " ご利用ありがとうございました "
//			if( prm_get( COM_PRM,S_PAY,25,1,2 ) ){					// ﾗﾍﾞﾙ設定あり?
//				grachr( 5, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR2[9] );				// " 定期券、ラベルをお取り下さい "
//			}else{
//				grachr( 5, 0, 30, 0, COLOR_BLACK,
//						LCD_BLINK_OFF, OPE_CHR2[6] );				// "更新された定期券をお取り下さい"
//			}
//
//			if( OPECTL.RECI_SW == 1 ){								// 領収書ﾎﾞﾀﾝ使用?
//				wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
//				grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "     領収証をお取り下さい     "
//			}else{
//				grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);		// " 必要な方は領収を押して下さい "
//			}
//			feedsp( 0, ryo_buf.turisen );
//			break;
//#endif	// UPDATE_A_PASS
//
//		case 24:															// 暗証番号出庫操作 車室入力画面
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[8]);	// "車室番号を入力して下さい      "
//			grawaku( 1, 4 );										// 飾り枠表示
//			bigcr(2, 2, 8, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);			// "  車室番号                    "
//			grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR3[0]);	// "      車室番号     →  精算   "
//			dsp_background_color(COLOR_WHITE);
//			teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示
//			break;
//
//		case 25:																	// 暗証番号出庫操作 暗証番号入力画面
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[81]);			// "出庫番号を４桁で入力して下さい"
//			grachr(1, 0, 10, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, ERR_CHR[6]);	// "  車室番号                    "
//			grawaku( 2, 5 );										// 飾り枠表示
//			bigcr(3, 2, 8, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[81]);		// "  出庫番号                    "
//			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[11]);			// "お客様コード                      "
//			grachr(7, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR3[12]);	// "    解除番号出庫   →  精算   "
//			teninb_pass(0, 4, 3, 16, 0, COLOR_FIREBRICK);
//			break;
//		case 26:													// 受付券発行済みNG表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[79] );								// "暗証番号登録時間が過ぎています"
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      車室番号                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		case 27:													// 受付券発行済みNG表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[80]);			// " 駐車証発行時間が過ぎています "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);		// "      車室番号                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		case 28:													// 
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[138] );					// "      只今、駐車証明書の      "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[139] );					// "      発行ができません        "
//			grachr( 4, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, shomei_errstr[shomei_errnum] );	// "          (紙切れ)            "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      車室番号                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		case 29:													// 受付券発行不可表示
//			grachr( 2, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[138] );					// "      只今、駐車証明書の      "
//			grachr( 3, 0, 30, 0, COLOR_BLACK,
//					LCD_BLINK_OFF, OPE_CHR[139] );					// "      発行ができません        "
//			break;
//		case 30:													// ラグタイム延長案内画面表示（フラップ）
//			dspclk(0, COLOR_DARKSLATEBLUE);							// 時計表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[151]);		// "   出庫できます               "
//			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[153]);		// "     ありがとうございました   "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      車室番号                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		case 31:													// ラグタイム延長案内画面表示（フラップ）
//			dspclk(0, COLOR_DARKSLATEBLUE);							// 時計表示
//			loklen = binlen( OPECTL.Op_LokNo % 10000L );
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[152]);		// "   精算済みです               "
//			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[153]);		// "     ありがとうございました   "
//			grachr(6, 6, 8, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[8]);	// "      車室番号                "
//			opedsp(6, 16, (ushort)(OPECTL.Op_LokNo % 10000L), loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);
//			break;
//		default:
//			break;
//	}
//	return;
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(不要処理削除)
}

/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝ画面表示 クレジット対応 4-6行目メッセージ表示初期化            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspCyclicMsgInit                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	dspCyclicMsgInit(void)
{
	memset( OPECTL.op_mod02_dispCnt,0,sizeof( OPECTL.op_mod02_dispCnt ));// 利用可能媒体：先頭を指定
}

/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝ画面表示 クレジット対応 サイクリック表示有無取得関数(外部用)   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspIsCyclicMsg                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	dspIsCyclicMsg(void)
{
	uchar msgno, i;

	// 現在釣り銭切れ表示中ならサイクリック表示なし
	if (Ex_portFlag[EXPORT_CHGNEND] != 0) {
		return 0;
	}

	for (i = 0; i < 3; i++) {
		msgno = (uchar)prm_get(COM_PRM, S_DSP, 4, 2, (char)((4-(i*2))+1));
		if ((msgno == CYCLIC_DSP01) || (msgno == CYCLIC_DSP09)) {
			return 1;	// サイクリック表示あり
		}
	}
	return 0;	// サイクリック表示なし
}


/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝ画面表示 クレジット対応 4-6行目メッセージ表示切替えチェック    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspCyclicMsgRewrite                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	dspCyclicMsgRewrite(uchar tik_add)
{
	uchar msgno, i,line = 0;
// MH321800(S) hosoda ICクレジット対応 (表示不良修正)
	short	line_max;

	if (LCDNO == 5) {
		// 精算完了画面で電子マネーやプリペイドカードがある場合、
		// それらの情報は書き換えできないのでサイクリック表示行数を減少させる
		line_max = 3;
		if (e_pay || c_pay) {		// 電子マネー使用、プリペイドカード使用
			line_max = 2;			// 電子マネーを使用した場合は6行目の設定は無視する(設定表参照)
		}
		for (i = 0; i < line_max; i++) {
			msgno = (uchar)prm_get(COM_PRM, S_DSP,(short)tik_add, 2, (char)((4-(i*2))+1));
			if (msgno >= 21 && msgno <= 25) {					// 払出不足時のメッセージ表示あり
				dsp_msg(msgno, (uchar)(i+3), tik_add, COLOR_BLACK);
			}
		}
		return;
	}
// MH321800(E) hosoda ICクレジット対応 (表示不良修正)

	// 現在釣り銭切れ表示中なら何もしない
// MH321800(S) hosoda ICクレジット対応 (表示不良修正)
//	料金案内画面でつり切れ表示を行うため、同画面ではつり切れがあれば何もしない
//-	if (Ex_portFlag[EXPORT_CHGNEND] != 0) {
	if (LCDNO == 3 && Ex_portFlag[EXPORT_CHGNEND] != 0) {
// MH321800(E) hosoda ICクレジット対応 (表示不良修正)
		return;
	}
	if(tik_add == 18){			//休業中表示は５～７行目を選択
		line = 1;
	}

	// サイクリック表示の有無チェック
	for (i = 0; i < 3; i++) {
		// 共通パラメータを引数に、１行表示関数コール → ３回コール
		msgno = (uchar)prm_get(COM_PRM, S_DSP,(short)tik_add, 2, (char)((4-(i*2))+1));
		if ((msgno == CYCLIC_DSP01) || (msgno == CYCLIC_DSP09)) {
			// サイクリック表示更新
			dsp_cyclic_msg((uchar)(line+(i+3)),msgno,tik_add);
		}
// MH321800(S) hosoda ICクレジット対応 (表示不良修正)
//-		else
//-		if ((OPECTL.Ope_mod == 3)
//-		&&	(mode_Lagtim10 == 0)) {
//-		// 精算完了表示設定（24-0001）の表示を再表示
//-			dsp_msg(msgno, (uchar)(i+3), tik_add, COLOR_BLACK);
//-		}
// MH321800(E) hosoda ICクレジット対応 (表示不良修正)
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝ画面表示 待機中8行目釣銭、用紙不足メッセージ表示切替えチェック |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dspCyclicErrMsgRewrite                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-05-14                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	dspCyclicErrMsgRewrite(void)
{
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//	ushort	lin[3];
//	ushort	i = 0;
//
//	memset(lin, 0, sizeof(lin));
//
// 精算状態通知の状態ﾁｪｯｸと精算状態の通知処理
//	if(( Ex_portFlag[EXPORT_CHGNEND] != 0 )||				// 0:釣銭切れ予告無し/1:釣銭切れ予告有り
//	   ( Ex_portFlag[EXPORT_CHGEND] != 0 )){				// 0:釣銭切れ無し/1:釣銭切れ有り
//		/*** 釣銭無し時表示 ***/
//		lin[i] = 62;										// "ⅰ釣銭不足                    "
//		i++;
//	}
//	if(( Ex_portFlag[EXPORT_R_PNEND] != 0 )||				// 0:ﾚｼｰﾄ紙切れ予告無し/1:ﾚｼｰﾄ紙切れ予告有り
//	   ( Ex_portFlag[EXPORT_R_PEND] != 0 )){				// 0:ﾚｼｰﾄ紙切れ無し/1:ﾚｼｰﾄ紙切れ有り
//		/*** ﾚｼｰﾄ紙切れ時表示 ***/
//		lin[i] = 63;										// "ⅰレシート用紙不足            "
//		i++;
//	}
//	if(( Ex_portFlag[EXPORT_J_PNEND] != 0 )||				// 0:ｼﾞｬｰﾅﾙ紙切れ予告無し/1:ｼﾞｬｰﾅﾙ紙切れ予告有り
//	   ( Ex_portFlag[EXPORT_J_PEND] != 0 )){				// 0:ｼﾞｬｰﾅﾙ紙切れ無し/1:ｼﾞｬｰﾅﾙ紙切れ有り
//		/*** ｼﾞｬｰﾅﾙ紙切れ時表示 ***/
//		lin[i] = 64;										// "ⅰジャーナル用紙不足          "
//		i++;
//	}
//	if(( dspErrMsg != lin[0] ) && ( dspErrMsg != lin[1] ) && ( dspErrMsg != lin[2] )){	// エラー解除の検索
//		dspErrMsg = 0;
//	}
//	if(i != 0){												// エラーメッセージが存在する場合
//		if(dspErrMsg == lin[i-1]){
//			dspErrMsg = 0;
//		}
//		for( i = 0; i < 3; i++ ){								// 現在表示中のメッセージより上の物が存在するか
//			if(dspErrMsg < lin[i]){
//				/*** エラーメッセージ表示 ***/
//				grachr(7, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[lin[i]]);	// "エラーメッセージ"
//				dspErrMsg = lin[i];
//				break;
//			}
//		}
//	}
// MH810100(S) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
//	static ulong old_pay_status = 0;
// MH810100(E) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
	ulong status = 0;

	if(( Ex_portFlag[EXPORT_CHGNEND] != 0 )||				// 0:釣銭切れ予告無し/1:釣銭切れ予告有り
	   ( Ex_portFlag[EXPORT_CHGEND] != 0 )){				// 0:釣銭切れ無し/1:釣銭切れ有り
		// 釣銭無し時表示
		status |= 0x01;										// 精算状態通知		b0:		釣銭不足
	}
// MH810100(S) S.Nishimoto 2020/08/17 #4592 車番チケットレス(レシートプリンタ通信不良チェック追加)
	if (IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_R_PRINTCOM)) {
		status |= 0x40000;										// 精算状態通知	b18:	ﾚｼｰﾄﾌﾟﾘﾝﾀ通信不良
	}
// MH810100(E) S.Nishimoto 2020/08/17 #4592 車番チケットレス(レシートプリンタ通信不良チェック追加)
// MH810100(S) K.Onodera 2020/04/09 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
//	if(( Ex_portFlag[EXPORT_R_PNEND] != 0 )||				// 0:ﾚｼｰﾄ紙切れ予告無し/1:ﾚｼｰﾄ紙切れ予告有り
//	   ( Ex_portFlag[EXPORT_R_PEND] != 0 )){				// 0:ﾚｼｰﾄ紙切れ無し/1:ﾚｼｰﾄ紙切れ有り
	if( Ex_portFlag[EXPORT_R_PNEND] != 0 ){					// 0:ﾚｼｰﾄ紙切れ予告無し/1:ﾚｼｰﾄ紙切れ予告有り
		status |= 0x10000;									// 精算状態通知		b16:		ﾚｼｰﾄ用紙切れ予告
	}
	if( Ex_portFlag[EXPORT_R_PEND] != 0 ){					// 0:ﾚｼｰﾄ紙切れ無し/1:ﾚｼｰﾄ紙切れ有り
// MH810100(E) K.Onodera 2020/04/09 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
		// ﾚｼｰﾄ紙切れ時表示
		status |= 0x02;										// 精算状態通知		b1:		ﾚｼｰﾄ用紙不足
	}
// GG129000(S) M.Fujikawa 2023/09/22 #6953 設定01-0016=0でジャーナル通信不良発生中に領収証発行に失敗しても、領収証発行失敗のポップアップが発生しない
	if ( !Ope_isJPrinterReady() ) {
		status |= 0x100000;									// 精算状態通知		b20:	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ通信不良
	}
// GG129000(E) M.Fujikawa 2023/09/22 #6953 設定01-0016=0でジャーナル通信不良発生中に領収証発行に失敗しても、領収証発行失敗のポップアップが発生しない
// MH810100(S) K.Onodera 2020/04/09 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
//	if(( Ex_portFlag[EXPORT_J_PNEND] != 0 )||				// 0:ｼﾞｬｰﾅﾙ紙切れ予告無し/1:ｼﾞｬｰﾅﾙ紙切れ予告有り
//	   ( Ex_portFlag[EXPORT_J_PEND] != 0 )){				// 0:ｼﾞｬｰﾅﾙ紙切れ無し/1:ｼﾞｬｰﾅﾙ紙切れ有り
	if( Ex_portFlag[EXPORT_J_PNEND] != 0 ){
		status |= 0x20000;									// 精算状態通知		b17:	ｼﾞｬｰﾅﾙ用紙切れ予告
	}
	if( Ex_portFlag[EXPORT_J_PEND] != 0 ){					// 0:ｼﾞｬｰﾅﾙ紙切れ無し/1:ｼﾞｬｰﾅﾙ紙切れ有り
// MH810100(E) K.Onodera 2020/04/09 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
		// ｼﾞｬｰﾅﾙ紙切れ時表示
		status |= 0x04;										// 精算状態通知		b2:		ｼﾞｬｰﾅﾙ用紙不足
	}
	// ICクレジット使用あり？
	if( isEC_USE() != 0 ){
// MH810103(s) 電子マネー対応 精算状態通知をOpe_modにより切り替える
//// MH810103(s) 電子マネー対応 状態をみないようにする
////// MH810100(S) S.Fujii 2020/07/06 車番チケットレス(電子決済端末対応)
////		if( !chk_ec_credit() ){								// クレジット利用不可
////			status |= 0x10;									// 精算状態通知		b4:		クレジット利用不可
////		}
////		if( !chk_ec_emoney() ){								// クレジット利用不可
////// MH810103(s) 電子マネー対応
//////			status |= 0x08;									// 精算状態通知		b3:		Suica利用不可
////// MH810103(e) 電子マネー対応
////			status |= 0x20;									// 精算状態通知		b5:		電子マネー利用不可
////		}
////// MH810100(E) S.Fujii 2020/07/06 車番チケットレス(電子決済端末対応)
//// MH810103(e) 電子マネー対応 状態をみないようにする
//		// 通信不良発生中？
//		if( ERR_CHK[mod_ec][ERR_EC_COMFAIL] ){
//// MH810103(s) 電子マネー対応
////			status |= 0x08;									// 精算状態通知		b3:		Suica利用不可
//// MH810103(e) 電子マネー対応
//			status |= 0x10;									// 精算状態通知		b4:		クレジット利用不可
//			status |= 0x20;									// 精算状態通知		b5:		電子マネー利用不可
//		}else{
//// MH810103(s) 電子マネー対応
////			if( !isEcEnabled(EC_CHECK_CREDIT) ){
//			if ( !isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT) ) {				// クレジットで精算可能?
//// MH810103(e) 電子マネー対応
//				status |= 0x10;								// 精算状態通知		b4:		クレジット利用不可
//			}
//// MH810103(s) 電子マネー対応
////			if( !isEcEnabled(EC_CHECK_EMONEY) ){
////				status |= 0x08;								// 精算状態通知		b3:		Suica利用不可
////				status |= 0x20;								// 精算状態通知		b5:		Edy利用不可
////			}
//			if( !isEcEmoneyEnabled(0, 0) ){					// 電子マネー決済が可能
//				status |= 0x20;								// 精算状態通知		b5:		電子マネー利用不可
//			}
//		}
//// MH810103(e) 電子マネー対応
		// 通信不良発生中？
		if( ERR_CHK[mod_ec][ERR_EC_COMFAIL] ){
			status |= 0x10;									// 精算状態通知		b4:		クレジット利用不可
			status |= 0x20;									// 精算状態通知		b5:		電子マネー利用不可
// MH810105(S) QRコード決済対応 #6578 QRコード決済が使用できない状態なのに、出来るような精算案内を行う
			status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
// MH810105(E) QRコード決済対応 #6578 QRコード決済が使用できない状態なのに、出来るような精算案内を行う
		}else{
			// 待機画面
			if(OPECTL.Ope_mod != 2 && OPECTL.Ope_mod != 3){
				// 共通チェック
				if ( isEC_USE()											// 決済リーダ使用設定
					&& !Suica_Rec.suica_err_event.BYTE					// リーダー使用可
					&& Suica_Rec.Data.BIT.INITIALIZE					// 初期化完了
//					&& Suica_Rec.Data.BIT.CTRL							// 受付可状態
				) {
					// 設定チェック
					if (!isEcBrandNoEnabledForSetting(BRANDNO_CREDIT)) {
						status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					}
					// 受信ブランドチェック
					if (!isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT)) {
						status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					}
					if( !isEcEmoneyEnabled(0, 0) ){					// 電子マネー決済が可能
						status |= 0x20;								// 精算状態通知		b5:		電子マネー利用不可
					}
// MH810105(S) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
					// 待機画面ではみない
// MH810105(E) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
					// 開局可否
					if( !isEcEmoneyEnabledNotCtrl(1,0) ){					// 電子マネー決済が可能
						status |= 0x20;								// 精算状態通知		b5:		電子マネー利用不可
					}

				}else{

					status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					status |= 0x20;										// 精算状態通知		b5:		電子マネー利用不可
// MH810105(S) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
					status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
// MH810105(E) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
				}
			}else if(OPECTL.Ope_mod == 3){
				// 共通チェック
				if ( isEC_USE()											// 決済リーダ使用設定
					&& !Suica_Rec.suica_err_event.BYTE					// リーダー使用可
					&& Suica_Rec.Data.BIT.INITIALIZE					// 初期化完了
//					&& Suica_Rec.Data.BIT.CTRL							// 受付可状態
				) {
					// 設定チェック
					if (!isEcBrandNoEnabledForSetting(BRANDNO_CREDIT)) {
						status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					}
					// 受信ブランドチェック
					if (!isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT)) {
						status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					}
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
//					if( !isEcEmoneyEnabled(0, 0) ){					// 電子マネー決済が可能
//						status |= 0x20;								// 精算状態通知		b5:		電子マネー利用不可
//					}
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
					// 開局可否
					if( !isEcEmoneyEnabledNotCtrl(1,0) ){					// 電子マネー決済が可能
						status |= 0x20;								// 精算状態通知		b5:		電子マネー利用不可
					}
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
//// MH810105(S) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
//					if( !chk_ec_qr() ){
//						status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
//					}
//// MH810105(E) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
					if (!isEcBrandNoEnabledNoCtrl(BRANDNO_QR)) {					// QRコード有効? 
						status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
					}
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
					if( Ope_isJPrinterReady() != 1 || IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)){		// ジャーナル使用不可時
						status |= 0x20;								// 精算状態通知		b5:		電子マネー利用不可
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
						status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
					}

				}else{

					status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					status |= 0x20;										// 精算状態通知		b5:		電子マネー利用不可
// MH810105(S) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
					status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
// MH810105(E) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
				}

			}else{
			// 精算中
				// 共通チェック
				if ( isEC_USE()											// 決済リーダ使用設定
					&& !Suica_Rec.suica_err_event.BYTE					// リーダー使用可
					&& Suica_Rec.Data.BIT.INITIALIZE					// 初期化完了
//					&& Suica_Rec.Data.BIT.CTRL							// 受付可状態
				) {
					if (!isEcBrandNoEnabledForSetting(BRANDNO_CREDIT)) {
						status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					}
					// 受信ブランドチェック
					if (!isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT)) {
						status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					}
					// 限度額チェック
					if(!Ec_check_cre_pay_limit()){
						status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					}
					if( !isEcEmoneyEnabled(1, 0) ){					// 電子マネー決済が可能
						status |= 0x20;								// 精算状態通知		b5:		電子マネー利用不可
					}
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
//// MH810105(S) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
//					if( !chk_ec_qr() ){
//						status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
//					}
//// MH810105(E) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
					if (!isEcBrandNoEnabled(BRANDNO_QR)) {					// QRコード有効?
						status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
					}
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6482 VPで精算時にクレジットか電子マネーを選択すると、ブランドを選択したタイミングで精算画面の精算方法から「コード決済」が消える
					if( Ope_isJPrinterReady() != 1 || IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)){		// ジャーナル使用不可時
						status |= 0x20;								// 精算状態通知		b5:		電子マネー利用不可
// MH810105(S) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
						status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
// MH810105(E) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
					}
				}else{

					status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
					status |= 0x20;										// 精算状態通知		b5:		電子マネー利用不可
// MH810105(S) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
					status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
// MH810105(E) MH364301 QRコード決済対応 #6400 QRコードで精算完了時の精算終了画面で「電子マネー \10-」と表示されてしまう
				}

			}
		}
// MH810103(e) 電子マネー対応 精算状態通知をOpe_modにより切り替える
	}
	// ICクレジット使用なし？
	else{
		// 使用なしは利用不可とする
// MH810103(s) 電子マネー対応
//		status |= 0x08;										// 精算状態通知		b3:		Suica利用不可
// MH810103(e) 電子マネー対応
		status |= 0x10;										// 精算状態通知		b4:		クレジット利用不可
		status |= 0x20;										// 精算状態通知		b5:		電子マネー利用不可
// MH810105(S) QRコード決済対応 #6578 QRコード決済が使用できない状態なのに、出来るような精算案内を行う
		status |= 0x80000;							// 精算状態通知	b19:	コード決済利用不可
// MH810105(E) QRコード決済対応 #6578 QRコード決済が使用できない状態なのに、出来るような精算案内を行う

	}
	
// MH810100(S) S.Takahashi 2020/05/08 #4151 車番チケットレス（紙幣利用不可状態でも支払い可能の音声案内、メッセージ表示を行う）
	if(cn_errst[1]) {
		// 紙幣リーダーエラーあり
		status |= 0x40;										// 精算状態通知		b6:		紙幣リーダ通信不良
	}
// MH810100(E) S.Takahashi 2020/05/08 #4151 車番チケットレス（紙幣利用不可状態でも支払い可能の音声案内、メッセージ表示を行う）
	
#if GT4100_NO_LCD
	return;
#endif

// MH810100(S) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
//	if( old_pay_status != status ){
//		// データ更新
//		old_pay_status = status;
	// 前回との差分あり？
	if( diff_pay_status(status) ){
// MH810100(E) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
		// 精算状態通知
		lcdbm_notice_pay( status );
	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
}

//--以下、内部関数
/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝ画面表示 クレジット対応 4-6行目メッセージ表示(1コールで1行表示)|*/
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
		// サイクリック表示
		dsp_cyclic_msg(line,msgno,tik_add);
	} else {
		// 非サイクリック表示
		if (msgno >= EXTPAND_MSG1) {
			no = 0;
		} else {
			no = msgno;
		}
		while (1) {
			if (infomsglist[no][0].func()) {
				// 第1表示OK
				if (infomsglist[no][1].func()) {
					// 第2表示OK → メッセージを表示
					grachr( (ushort)line, 0, 30, 0, color, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[no]);
					return;
				} else {
					// 第2表示NG → 次メッセージ候補へ
					no = infomsglist[no][1].alternate;
				}
			} else {
				// 第1表示NG → 次メッセージ候補へ
				no = infomsglist[no][0].alternate;
// MH321800(S) hosoda ICクレジット対応 (表示不良修正)
				if (no == -1) {
					return;
				}
// MH321800(E) hosoda ICクレジット対応 (表示不良修正)
			} 
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝ画面表示 クレジット対応 4-6行目メッセージ サイクリック表示     |*/
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

	wk_DspNo = 0;											// 表示対象：なし

	for(i=0; i<12; i++) {
		wk_para_num = OPECTL.op_mod02_dispCnt[wk_DspLineNo] / 3;
		wk_Prm = wk_base + wk_para_num;
		wk_Cnt = OPECTL.op_mod02_dispCnt[wk_DspLineNo] - 3*wk_para_num;

		if(wk_Cnt == 0) {
			wk_Cnt = 5;										// prm_getの第５引数（位置：654321の順）を算出
		} else {
			wk_Cnt = (ushort)(6 - wk_Cnt*2 -1);				// prm_getの第５引数（位置：654321の順）を算出
		}
		wk_DspNo = (uchar)prm_get(COM_PRM, S_DSP, (short)wk_Prm, 2, (char)wk_Cnt);	// 表示対象get
		OPECTL.op_mod02_dispCnt[wk_DspLineNo]++;			// 表示切替用カウンタUP
		if(OPECTL.op_mod02_dispCnt[wk_DspLineNo] > 11) {
			OPECTL.op_mod02_dispCnt[wk_DspLineNo] = 0;		// 先頭に戻る
		}

		if(wk_DspNo != 0) {									// 表示対象である
			switch( wk_DspNo ){
				case	2:									// サービス券の時
					if (chk_service()) {
						break;
					}
					continue;
				case	3:									// プリペイドカードの時
					if (chk_prepaid()) {
						break;
					}
					continue;
				case	4:									// 回数券の時
					if (chk_coupon()) {
						break;
					}
					continue;
				case	5:									// 定期券の時
					if (chk_teiki()) {
						break;
					}
					continue;
				case	6:									// クレジットの時
// MH321800(S) D.Inaba ICクレジット対応
//						if (chk_credit()) {
//							break;
//						}
//						continue;
					if ( isEC_USE() ) {
						// 決済リーダ
						if( chk_ec_credit() ){				// クレジット精算可能
							break;
						}
// MH810103 GG119202(S) 接客画面の電子マネー対応
						// 利用不可メッセージは表示しない
//						else{								// 決済リーダ利用中のクレジット使用不可のみ別メッセージを表示させる
//// MH321800(S) 文字列の追加箇所を変更
////							grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[44]);			// "                  は使えません"
//							grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[161]);					// "                  は使えません"
//// MH321800(E) 文字列の追加箇所を変更
//							grachr((ushort)line, 0, 16, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR_CYCLIC_MSG[wk_DspNo]);	// クレジット
//							return;
//						}
// MH810103 GG119202(E) 接客画面の電子マネー対応
					}
// MH810103 GG119202(S) 接客画面の電子マネー対応
//					else{
//						if (chk_credit()) {
//							break;
//						}
//					}
// MH810103 GG119202(E) 接客画面の電子マネー対応
					continue;
// MH321800(E) D.Inaba ICクレジット対応
				case	7:									// Ｓｕｉｃａの時
				case	9:									// ＰＡＳＭＯの時
				case   19:									// ＩＣＯＣＡの時
// MH321800(S) hosoda ICクレジット対応
//				case   20:									// ＩＣカードの時
// MH321800(E) hosoda ICクレジット対応
					if (chk_suica()) {
						break;
					}
					continue;
// MH321800(S) hosoda ICクレジット対応
				case   20:									// 電子マネーの時
					if (isEC_USE()) {
					// EC決済リーダ－
						if (chk_ec_emoney()) {
							break;
						}
// MH810103 GG119202(S) 接客画面の電子マネー対応
						// 利用不可メッセージは表示しない
//// GG119200(S) 電子マネー利用不可表示対応
//						else{								// 決済リーダ利用中の電子マネー使用不可のみ別メッセージを表示させる
//							grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[161]);					// "                  は使えません"
//							grachr((ushort)line, 0, 16, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR_CYCLIC_MSG[wk_DspNo]);	// 電子マネー
//							return;
//						}
//// GG119200(E) 電子マネー利用不可表示対応
// MH810103 GG119202(E) 接客画面の電子マネー対応
					} else {
						if (chk_suica()) {
							break;
						}
					}
					continue;
// MH321800(E) hosoda ICクレジット対応
				case	8:									// Ｅｄｙの時
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					if (chk_edy()) {
//						break;
//					}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					continue;	
// MH321800(S) T.Nagai ICクレジット対応
				case	1:									// 現金
					if (chk_coin() || chk_note()) {
						break;
					}
					continue;
				case	15:
					if (chk_coin()) {						// 硬貨
						break;
					}
					continue;
// MH321800(E) T.Nagai ICクレジット対応
				case   16:									// 紙幣の時
				case   17:									// 千円札の時
					if (chk_note()) {
						break;
					}
					continue;
				case   26:									// Tカードの時
					if( chk_Tcard() ){						
						break;
					}
					continue;
// MH810105(S) MH364301 QRコード決済対応
				case	28:									// QRコード決済
					if (chk_ec_qr()) {
						break;
					}
					continue;
// MH810105(E) MH364301 QRコード決済対応
				default:
					if((11 <= wk_DspNo)&&(wk_DspNo <= 14) ||
				    		(18 <= wk_DspNo)&&(wk_DspNo <= 99)){
						// 未設定エリア
						continue;
				    }else{
						break;
					}
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			}
			break;
		}
	}
// MH322914 (s) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
//	if( (wk_DspNo == 0) || (wk_DspNo > 26) ) {
// MH810105(S) MH364301 QRコード決済対応
//	if( (wk_DspNo == 0) || (wk_DspNo > 26) || (i == 12)) {
	if( (wk_DspNo == 0) || (wk_DspNo > 28) || (i == 12)) {
// MH810105(E) MH364301 QRコード決済対応
		displclr(line);
// MH322914 (e) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
		return;
	}

	if(msgno == 1){
		grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[msgno]);		// "                が利用できます"
		grachr((ushort)line, 0, 16, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR_CYCLIC_MSG[wk_DspNo]);	// 媒体名表示
	}else if(msgno == 9){
		grachr((ushort)line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_G_SALE_MSG[msgno]);		// ""お支払いは"
		grachr((ushort)line, 12, 16, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR_CYCLIC_MSG[wk_DspNo]);// 媒体名表示
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝ画面表示 クレジット対応 4-6行目メッセージ表示 判定関数群       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-12-21                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// 表示 無条件OK
int chk_OK(void)
{
	return 1;
}

// サービス券判定
int chk_service(void)
{
	uchar i;				// カウンタ
	short	data_adr;		// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ
	char	data_pos;		// 使用可能料金種別のﾃﾞｰﾀ位置

	if( 0 == prm_get( COM_PRM, S_PAY, 21, 1, 3 ) ) {	// 磁気リーダなし?
		return 0;										// サービス券使用不可
	}

	for(i = 0 ; i < 15 ; i++){			//サービス券種類回ﾙｰﾌﾟ
		if( ryo_buf.syubet < 6 ){
			// 精算対象の料金種別A～F(0～5)
			data_adr = (2*(i))+76;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
			data_pos = (char)(6-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
		}
		else{
			// 精算対象の料金種別G～L(6～11)
			data_adr = (2*(i))+77;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
			data_pos = (char)(12-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
		}
		if( prm_get( COM_PRM, S_SER, data_adr, 1, data_pos ) == 0 ){		// 使用可設定？
			return 1;														// 1つでも使用可であれば表示OKとする。
		}
	}
	return 0;
}

// プリペイドカード判定
int chk_prepaid(void)
{
	if (prm_get( COM_PRM, S_PRP, 1, 1, 1 ) == 1) {
		return 1;
	} else {
		return 0;
	}
}

// クレジットカード判定
int chk_credit(void)
{
	int		ret = 1;
	uchar	sts;

// MH321800(S) hosoda ICクレジット対応
	if ( isEC_USE() ) {
	// 決済リーダ
		return chk_ec_credit();
	}
// MH321800(E) hosoda ICクレジット対応

	sts = creStatusCheck();

	if( sts & CRE_STATUS_DISABLED ){	// クレジット設定なし
		ret = 0;	// クレジット利用不可
	}
	if( sts & CRE_STATUS_STOP ){		// クレジット停止中
		ret = 0;	// クレジット利用不可
	}
	if( sts & CRE_STATUS_PPP_DISCONNECT ){	// PPP未接続
		ret = 0;	// クレジット利用不可
	}

	return ret;
}

// 回数券判定
int chk_coupon(void)
{
	if ((uchar)prm_get( COM_PRM, S_PRP, 1, 1, 1 ) == 2) {
		return 1;
	} else {
		return 0;
	}
}

// 定期券判定
int chk_teiki(void)
{
	return 1;
}

// 紙幣判定
int chk_note(void)
{
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
	if (isEC_USE() &&
// MH810104 GG119201(S) 硬貨・紙幣利用判定修正
		check_enable_multisettle() > 1 &&		// マルチブランドで電子マネー選択時は利用不可
// MH810104 GG119201(E) 硬貨・紙幣利用判定修正
		RecvBrandResTbl.res == EC_BRAND_SELECTED &&
// MH810105(S) MH364301 QRコード決済対応
//		isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
//		// 電子マネー選択時は利用不可とする
		EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
		// 電子マネー、QRコード選択時は利用不可とする
// MH810105(E) MH364301 QRコード決済対応
		return 0;
	}
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする

	if((cn_errst[1] != 0 )||		//紙幣正常?(Y)
// MH321800(S) T.Nagai ICクレジット対応
		( note_err_flg != 0 )||		//紙幣利用不可？
// MH321800(E) T.Nagai ICクレジット対応
		( notful() == (char)-1 )){	//紙幣金庫満杯
		return 0;
	}else{
		return 1;
	}
}
// 預り証判定
int chk_azukari(void)
{
	if(ryo_buf.fusoku){	//不足金額有
		return 1;
	}else{
		return 0;
	}
}
// フラップ判定
int chk_flp(void)
{
	if(LkKind_Get(OPECTL.Pr_LokNo) == LK_KIND_FLAP || 
	   LkKind_Get(OPECTL.Pr_LokNo) == LK_KIND_INT_FLAP ){
		return 1;
	}else{
		return 0;
	}
}
// ロック判定
int chk_lock(void)
{
	if(LkKind_Get(OPECTL.Pr_LokNo) == LK_KIND_LOCK){
		return 1;
	}else{
		return 0;
	}
}
// Ｓｕｉｃａ判定
int chk_suica(void)
{
	if(Suica_Rec.suica_err_event.BYTE == 0 &&
		!Suica_Rec.Data.BIT.MIRYO_TIMEOUT &&		// 未了タイムアウト未発生
		!Suica_Rec.Data.BIT.PRI_NG &&				// ジャーナル異常未発生
		prm_get(COM_PRM, S_PAY, 24, 1, 3) == 1 ) {
		return 1;
	}else{
		return 0;
	}
}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//// Ｅｄｙ判定
//int chk_edy(void)
//{
//	if( !EDY_USE_ERR && 														// 利用可能状態である事を確認し表示する
//		!Edy_Rec.edy_status.BIT.PRI_NG &&										// ジャーナル異常未発生
//		(Edy_Arc_Connection_Chk() == 0 ) &&
//		!Edy_Rec.edy_status.BIT.MIRYO_LOSS ) {									// 07-01-23変更 Edy減算失敗ﾀｲﾑｱｳﾄ後はＥｄｙ表示しない
//		return 1;
//	}else{
//		return 0;
//	}
//}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

int chk_Tcard(void)
{
	return 0;
}

// MH321800(S) hosoda ICクレジット対応
// クレジットカード判定 (決済リーダ)
int chk_ec_credit(void)
{
	int	ret = 0;

// MH810103 GG119202(S) 開局・有効条件変更
//	if ( isEcReady(EC_CHECK_CREDIT) ) {						// クレジットで精算可能?
	if ( isEcBrandNoEnabled(BRANDNO_CREDIT) ) {				// クレジットで精算可能?
// MH810103 GG119202(E) 開局・有効条件変更
		if(RecvBrandResTbl.res == EC_BRAND_UNSELECTED) {	// ブランド未選択
			ret = 1;
		}
		else {												// ブランド選択
			if(RecvBrandResTbl.no == BRANDNO_CREDIT) {		// 選択したブランドがクレジット
				ret = 1;
			}
		}
	}
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
	if( ret && !Ec_check_cre_pay_limit() ){	// クレジットは精算限度額を超えている？
		ret = 0;	// クレジット使用不可
	}
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
	return ret;
}

// 電子マネー判定 (決済リーダ)
int chk_ec_emoney(void)
{
	int	ret = 0;

// MH810103 GG119202(S) 電子マネー決済時はジャーナルプリンタ接続設定を参照する
	if (Suica_Rec.Data.BIT.PRI_NG != 0) {
		// ジャーナルプリンタ使用不可時は電子マネー決済不可
		return 0;
	}
// MH810103 GG119202(E) 電子マネー決済時はジャーナルプリンタ接続設定を参照する

// MH810103 GG119202(S) 開局・有効条件変更
//	if ( isEcReady(EC_CHECK_EMONEY) && WAR_MONEY_CHECK ) {	// 電子マネーで精算可能?
	if (isEcEmoneyEnabled(1, 0)) {							// 電子マネー有効
// MH810103 GG119202(E) 開局・有効条件変更
		if(RecvBrandResTbl.res == EC_BRAND_UNSELECTED) {	// ブランド未選択
			ret = 1;
		}
		else {												// ブランド選択
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//			if(isEcBrandNoEMoney(RecvBrandResTbl.no)) {		// 選択したブランドが電子マネー
			if(isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {	// 選択したブランドが電子マネー
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
				ret = 1;
			}
		}
	}
	return ret;
}

// MH810105(S) MH364301 QRコード決済対応
// QRコード決済判定 (決済リーダ)
int chk_ec_qr(void)
{
	int	ret = 0;

	if (Suica_Rec.Data.BIT.PRI_NG != 0) {
		// ジャーナルプリンタ使用不可時はQR決済不可
		return 0;
	}

	if (isEcBrandNoEnabled(BRANDNO_QR)) {					// QRコード有効?
		if (RecvBrandResTbl.res == EC_BRAND_UNSELECTED) {	// ブランド未選択
			ret = 1;
		}
		else {												// ブランド選択
			if (RecvBrandResTbl.no == BRANDNO_QR) {			// 選択したブランドがQRコード
				ret = 1;
			}
		}
	}
	return ret;
}
// MH810105(E) MH364301 QRコード決済対応

// 硬貨判定
int chk_coin(void)
{
	int	ret = 0;

// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
	if (isEC_USE() &&
// MH810104 GG119201(S) 硬貨・紙幣利用判定修正
		check_enable_multisettle() > 1 &&		// マルチブランドで電子マネー選択時は利用不可
// MH810104 GG119201(E) 硬貨・紙幣利用判定修正
		RecvBrandResTbl.res == EC_BRAND_SELECTED &&
// MH810105(S) MH364301 QRコード決済対応
//		isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
//		// 電子マネー選択時は利用不可とする
		EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
		// 電子マネー、QRコード選択時は利用不可とする
// MH810105(E) MH364301 QRコード決済対応
		return 0;
	}
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする

	if (coin_err_flg == 0) {
		ret = 1;
	}
	return ret;
}
// MH321800(E) hosoda ICクレジット対応

/*-------------------------------*/
/* 画面表示用　精算媒体種別 取得 */
/*-------------------------------*/
ushort	Ope_Disp_Media_Getsub( uchar mode )
{
	ushort	ret = 0;
	long	suica_mode=0;
																// 設定による切り分け
// MH321800(S) hosoda ICクレジット対応
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//// GG119200(S) 電子マネー対応
////	if (isEC_USE()) {											// 決済リーダ使用設定
//	if (isEcEnabled(EC_CHECK_EMONEY)) {							// 決済リーダ使用設定
//// GG119200(E) 電子マネー対応
	if (isEcEmoneyEnabled(0, 0)) {								// 電子マネー有効
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
		ret = OPE_DISP_MEDIA_TYPE_EC;
	}
	else
// MH321800(E) hosoda ICクレジット対応
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if(prm_get(COM_PRM, S_PAY, 24, 2, 3) == 1 ) {				// Suicaﾘｰﾀﾞｰのみ使用可能
	if(prm_get(COM_PRM, S_PAY, 24, 1, 3) == 1 ) {				// Suicaﾘｰﾀﾞｰのみ使用可能
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		if( Is_SUICA_STYLE_OLD ){								// 旧版動作設定
			suica_mode = prm_get(COM_PRM, S_SCA, 2, 3, 1);		// ICOCAモード追加の為修正
			switch( suica_mode )
			{
				case  1:
				default:
					ret = OPE_DISP_MEDIA_TYPE_SUICA;					// Ｓｕｉｃａ
					break;
				case 10:
					ret = OPE_DISP_MEDIA_TYPE_PASMO;					// ＰＡＳＭＯ
					break;
			}
		}
		else{													// 新版動作設定
			suica_mode = prm_get(COM_PRM, S_SCA, 1, 1, 1);		// リーダー種別get
			switch( (ushort)suica_mode ){
			case	0:											// 0=電子マネー
			default:
				ret = OPE_DISP_MEDIA_TYPE_eMONEY;
				break;
			case	1:											// 1=Ｓｕｉｃａ
				ret = OPE_DISP_MEDIA_TYPE_SUICA;
				break;
			case	2:											// 2=ＰＡＳＭＯ
				ret = OPE_DISP_MEDIA_TYPE_PASMO;
				break;
			case	4:											// 4＝ＩＣＯＣＡ
				ret = OPE_DISP_MEDIA_TYPE_ICOCA;
				break;
			}
		}
	}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	else if(prm_get(COM_PRM, S_PAY, 24, 2, 3) == 10 ) {			// Edyﾘｰﾀﾞｰのみ使用可能
//		ret = OPE_DISP_MEDIA_TYPE_EDY;							// Ｅｄｙ
//	}
//	else if(prm_get(COM_PRM, S_PAY, 24, 2, 3) == 11 ) {			// 複数ﾘｰﾀﾞｰ使用可能(Suica & Edyﾘｰﾀﾞｰ)
//		ret = OPE_DISP_MEDIA_TYPE_ICCARD;						// ＩＣカード
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	
	if(mode == 0) {												// 「設定のみでの判断」指定時
	}
	else {														// 「使用されたカードを判別」指定時
// MH321800(S) hosoda ICクレジット対応
// MH810105(S) MH364301 QRコード決済対応
//		if ( EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {
		if ( EcUseEMoneyKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {
// MH810105(E) MH364301 QRコード決済対応
			ret = OPE_DISP_MEDIA_TYPE_EC;
		}
		else
// MH321800(E) hosoda ICクレジット対応
		if(Settlement_Res.settlement_data ) {					// Ｓｕｉｃａﾘｰﾀﾞｰｶｰﾄﾞ
			ret = OPE_DISP_MEDIA_TYPE_SUICA;					// カード利用後はSuicaとして扱う
			if( Is_SUICA_STYLE_OLD ){							// 旧版動作設定
				if ( memcmp(Settlement_Res.Suica_ID , "PB" , 2) == 0){	// カード番号の先頭2文字が"PB"であれば
					ret = OPE_DISP_MEDIA_TYPE_PASMO;					// PASMOカードと判断する
				}
			}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		} else if( PayData.Electron_data.Edy.e_pay_kind == EDY_USED ){	// Ｅｄｙﾘｰﾀﾞｰｶｰﾄﾞ
//				ret = OPE_DISP_MEDIA_TYPE_EDY;					// Ｅｄｙ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		}
	}
	return( ret );
}


/*---------------------------------------------------*/
/* 画面表示用　精算媒体種別 ﾒｯｾｰｼﾞNo(OPE_CHR_G)取得 */
/*---------------------------------------------------*/
ushort	Ope_Disp_Media_GetMsgNo( uchar mode, uchar type1, uchar type2 )
{
	ushort	msgno = 0;

	wk_media_Type = Ope_Disp_Media_Getsub(mode);					// 画面表示用　精算媒体種別 取得

	switch(wk_media_Type) {		
		case OPE_DISP_MEDIA_TYPE_SUICA:								// Ｓｕｉｃａ
			if( (mode == 0) || (Is_SUICA_STYLE_OLD) ){				// mode=0：名称は設定に従う（カードタッチ前）
																	// 又は旧版動作設定
				if(type1 == 0) {									// 使用媒体　　　　　円　指定
					if(type2 == 0) {								// 単独使用　指定
						msgno = 86;									// "  Ｓｕｉｃａ            円    ",	//71
					} 
				} else if(type1 == 1) {								// 使用媒体　利用額　円　指定
					msgno = 87;										//"Ｓｕｉｃａ利用額           円 ",		//72
				} else if(type1 == 3) {								// 使用媒体　利用不可エラー２　指定
					msgno = 99;										//"   現金使用後はＳｕｉｃａの   ",		//86
				}
			}
			else{													// 新版動作指定
				// ---【パラメータ説明】------------------------------------
				// type1 : 0 = OpeLcd(4)　「あと　」画面表示用
				//		   1 = OpeLcd(5)　「おつり」画面表示用
				// type2 : 0 固定
				// ---------------------------------------------------------

				if(type1 == 0){
					msgno = 125;									// " 　利用額　 　　　　　　　　円"
				} 
				else if(type1 == 1){
					msgno = 126;									// "　　　　　利用額　　　　　 円 "
				}
			}
			break;

		case OPE_DISP_MEDIA_TYPE_PASMO:								// ＰＡＳＭＯ
			if(type1 == 0) {										// 使用媒体　　　　　円　指定
				if(type2 == 0) {									// 単独使用　指定
					msgno = 106;									// "  ＰＡＳＭＯ            円    ",	//95
				}			
			} else if(type1 == 1) {									// 使用媒体　利用額　円　指定
				msgno = 107;										// "ＰＡＳＭＯ利用額           円 ",	//96
			} else if(type1 == 3) {									// 使用媒体　利用不可エラー２　指定
				msgno = 112;										// "   現金使用後はＰＡＳＭＯの   ",	//110
			}
			break;
	
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case OPE_DISP_MEDIA_TYPE_EDY:								// Ｅｄｙ
//			if(type1 == 0) {										// 使用媒体　　　　　円　指定
//				if(type2 == 0) {									// 単独使用　指定
//					msgno = 108;									// "  Ｅｄｙ                円    ",	//100
//				}			
//			} else if(type1 == 1) {									// 使用媒体　利用額　円　指定
//				msgno = 109;										// "   Ｅｄｙ利用額            円 ",	//101
//			}
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		case OPE_DISP_MEDIA_TYPE_ICOCA:								// ＩＣＯＣＡ
			if(type1 == 0) {										// 使用媒体　　　　　円　指定
				if(type2 == 0) {									// 単独使用　指定
					msgno = 120;									// "  ＩＣＯＣＡ            円    ",	//81
				}
			} else if(type1 == 1) {									// 使用媒体　利用額　円　指定
				msgno = 121;										// "ＩＣＯＣＡ利用額           円 ",	//82
			} else if(type1 == 3) {									// 使用媒体　利用不可エラー２　指定
				msgno = 122;										// "   現金使用後はＩＣＯＣＡの   ",	//84
			}
			break;

		case OPE_DISP_MEDIA_TYPE_ICCARD:							// ＩＣカード（複数カード使用可能）
			if(type1 == 0) {										// 使用媒体　　　　　円　指定
				if(type2 == 0) {									// 単独使用　指定
					msgno = 110;									// "  ＩＣカード            円    ",	//105
				}			
			} else if(type1 == 1) {									// 使用媒体　利用額　円　指定
				msgno = 111;										// "ＩＣカード利用額           円 ",	//106
			} else if(type1 == 3) {									// 使用媒体　利用不可エラー２　指定
				msgno = 113;										// "   現金使用後はＩＣカードの   ",	//111
			}
			break;
		case OPE_DISP_MEDIA_TYPE_eMONEY:							// 電子マネー
			if(type1 == 3) {										// 使用媒体　利用不可エラー２　指定
				msgno = 128;										// "   現金使用後は電子マネーの   "
			}														// 新仕様で mode==0はエラーメッセージのみ
			break;
// MH321800(S) hosoda ICクレジット対応
		case OPE_DISP_MEDIA_TYPE_EC:								// 決済リーダ
			if (type1 == 0) {										// 精算中 あと画面
				msgno = 155;										// "  電子マネー            円    "
			} else if (type1 == 1) {								// 精算完了後おつり画面
				msgno = 156;										// "電子マネー支払             円 "
			} else if (type1 == 3) {								// 使用媒体　利用不可エラー２　指定
				msgno = 128;										// "   現金使用後は電子マネーの   "
			}
			break;
// MH321800(E) hosoda ICクレジット対応
		default:
			break;
	}

	return(msgno);

}

/*[]----------------------------------------------------------------------[]*/
/*| 磁気データ読取りエラー詳細表示                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MagReadErrCodeDisp( ercd )                              |*/
/*| PARAMETER    : ercd = エラーコード                                     |*/
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
			OPECTL.CR_ERR_DSP = 40;						// 読取りできません (ST)
			break;
		case 0x12:
			OPECTL.CR_ERR_DSP = 41;						// 読取りできません (LT)
			break;
		case 0x21:
			OPECTL.CR_ERR_DSP = 42;						// 読取りできません (S)
			break;
		case 0x22:
			OPECTL.CR_ERR_DSP = 43;						// 読取りできません(D)
			break;
		case 0x23:
			OPECTL.CR_ERR_DSP = 44;						// 読取りできません(P)
			break;
		case 0x25:
			OPECTL.CR_ERR_DSP = 45;						// 読取りできません (C)
			break;
		default:
			OPECTL.CR_ERR_DSP = 46;						// 読取りできません(EO)
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			領収証表題の取得
//[]----------------------------------------------------------------------[]
///	@return			26	: "      領収書を発行します      "<br>
///					27	: "     領収書をお取り下さい     "<br>
///					147	: "      領収証を発行します      "<br>
///					148	: "     領収証をお取り下さい     "<br>
///					149	: "    利用明細書を発行します    "<br>
///					150	: "   利用明細書をお取り下さい   "
///	@author			m.onouchi
///	@attention		領収証(F2)キーが押下されている状態でコールすること。
///	@note			領収証に印字する表題を取得し<br>
///					文字列テーブルOPE_CHR[][31]のインデックスを返す。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/08/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Disp_Receipt_GetMsgNo(void)
{
	ushort	msgno;

	if (prm_get(COM_PRM, S_RTP, 2, 1, 3) == 1) {			// 領収証表題印字(2)(17-0002④)
		msgno = 26;											// "      領収書を発行します      " OPE_CHR[26][31]
	} else {
		msgno = 147;										// "      領収証を発行します      " OPE_CHR[147][31]
	}

	if (ryo_buf.credit.pay_ryo != 0) {						// クレジット精算
		switch (prm_get(COM_PRM, S_RTP, 2, 1, 1)) {
		case 2:												// 現金ありクレジット精算時は"領収証"で発行
			if (ryo_buf.nyukin != 0) {						// 現金あり
				break;
			}
			// not break
		case 1:												// クレジット精算時は"利用明細書"で発行
			msgno = 149;									// "    利用明細書を発行します    " OPE_CHR[149][31]
			break;
		default:											// すべて"領収証"で発行
			break;
		}
	}

	if ((OPECTL.Ope_mod == 3) || (OPECTL.Ope_mod == 23)){	// 精算完了
		msgno++;											// ～をお取り下さい
	}

	return(msgno);
}


/*[]----------------------------------------------------------------------[]*/
/*| 領収書案内表示				                                           |*/
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

// MH810105(S) MH364301 インボイス対応
//	if (Ope_isPrinterReady() == 0) {				// プリンタ異常あり
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 	if (Ope_isPrinterReady() == 0 ||				// プリンタ異常あり
	if ((!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) ||	// プリンタ異常あり
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
		(IS_INVOICE &&								// インボイス設定時はレシートとジャーナルが片方でも動かなかった場合
		  Ope_isJPrinterReady() == 0)) {			// 領収証を印字しない
// MH810105(E) MH364301 インボイス対応
		if (prm_get(COM_PRM, S_RTP, 2, 1, 3) == 1) {			// 領収証表題印字(2)(17-0002④)
			grachr(7, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[141]);		// " 只今領収書の発行はできません "
		}
		else {
			grachr(7, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[154]);		// " 只今領収証の発行はできません "
		}
	}else{
		if (ReceiptChk() == 0) {							// レシート発行可能設定時
				if( OPECTL.RECI_SW == 1 ){
					wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();									// "      領収証を発行します      "
					grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "     領収証をお取り下さい     "
				}
				else{
					grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);		// " 必要な方は領収を押して下さい "
				}
			}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			駐車位置番号の描画開始列取得
//[]----------------------------------------------------------------------[]
///	@return			駐車位置番号の描画開始列(16-21)
///	@author			m.onouchi
///	@attention		グローバル変数knum_lenを使用しているため<br>
///					値が確定している状態でコールすること。
///	@note			車室番号入力画面などで表示する駐車位置番号の<br>
///					描画開始列を返す。
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
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
	}

	return col;
}

// MH810100(S) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
//[]----------------------------------------------------------------------[]
///	@brief			前回の精算状態と比較(比較後、前回状態を更新)
//[]----------------------------------------------------------------------[]
/// @param[in]		sta		比較対象の状態
///	@return			bRet	TRUE(違いあり)/FALSE(違いなし)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/04/10<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static BOOL diff_pay_status( ulong sta )
{
	BOOL bRet = FALSE;	// 違いなし

	// 前回と今回の精算状態比較
	if( old_pay_status != sta ){
		bRet = TRUE;
		// 今回の状態を前回状態へ
		old_pay_status = sta;
	}

	return bRet;
}

//[]----------------------------------------------------------------------[]
///	@brief			前回精算状態をクリア
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
// MH810100(E) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)