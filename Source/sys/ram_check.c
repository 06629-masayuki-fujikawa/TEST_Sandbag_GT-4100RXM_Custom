//[]----------------------------------------------------------------------[]
///	@mainpage		
///	RAM CHECK for CRM2<br><br>
///	<b>Copyright(C) 2007 AMANO Corp.</b>
///	CREATE			2007/08/15 MATSUSHITA<br>
///	UPDATE			2007/08/24
///	@file			ram_check.c
///	@date			2007/08/15
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
#include	"iodefine.h"
#include	"system.h"

/*
	CPU			: H8SX/1663
	SRAM		: バイト制御SRAM/16ビット
			　	  1MB×2
	内蔵SRAM	: 40KB

	アドレスバステスト
		アドレスA0-19を１ビットづつONしたアドレスにテストデータを書き込み
		そのアドレスのみ書き込まれていることを確認する。
	データバステスト
		データバスに0-0xFFFFを出力し、読み出して一致することを確認する。
		（SRAMごと先頭64KBを使用する）
		1663は偶数／奇数アドレスでD15-8/D7-0が固定で使用されるので、アクセ
		スサイズを変えてテストすることは省略する。
	データパターンテスト
		各メモリアドレスにテストデータを書き込み、読み出して一致することを
		確認する。テストデータはバイト単位に各ビット(0-7)を１ビットづつON
		したものとする。（処理時間短縮のため）
*/

#define	_RAM_TOP	0x200000L
#define	_ADR_BIT	20
#define	_RAM_SIZ	(1L<<_ADR_BIT)
#define	_RAM_CNT	2
#define	_RAM_END	(_RAM_TOP+_RAM_SIZ*_RAM_CNT)

#define	_CHK_SIZ	(65536*4)

#define	_INRAM_TOP	0xFF2000
#define	_INRAM_END	0xFFC000

#define	shortofs(n)	((n)>>1)
#define	longofs(n)	((n)>>2)

__global_register(R4L)	unsigned char _misc2;
#define	OUTMISC2()	_misc2 |= _misc2
#define	LED()		{_misc2 ^= (MISC2_CP_LD1|MISC2_CP_LD2);	OUTMISC2();}


//[]----------------------------------------------------------------------[]
///	@brief			check_end(result check result)
//[]----------------------------------------------------------------------[]
///	@param[in]		result	: 表示金額
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static
void	check_end(int result)
{
	if (result) {
		_misc2 &= (uchar)~(MISC2_CP_LD1|MISC2_CP_LD2);
		_misc2 |= MISC2_CP_LD2;
		while(1) {
			OUTMISC2();
			wait2us( 50000L );
			_misc2 ^= MISC2_CP_LD2;
		}
	}
	else {
		_misc2 |= (MISC2_CP_LD1|MISC2_CP_LD2);
		OUTMISC2();
		for ( ; ; ) {}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			check_address(check address bus brige)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static
void	check_address(void)
{
	short	*base, *p, *q;		// 16 bit data bus
	int		i, bno, flag;

// clear RAM
	for (p = (short*)_RAM_TOP; p < (short*)_RAM_END; )
		*p++ = 0;
// check each RAM
	base = (short*)_RAM_TOP;
	for (i = 0; i < _RAM_CNT; i++) {
	// check each bus
		for (bno = 0; bno < _ADR_BIT; bno++) {
			LED();
			p = base + shortofs(1L<<bno);
			*p = -1;					// write test data
			flag = 0;
		// search test data
			for (q = (short*)_RAM_TOP; q < (short*)_RAM_END; q++) {
				if (*q) {
				// if detect non-zero data
					if (q != p) {
						flag = -1;		// detect invalid data
						break;
					}
					flag = 1;
					*q++ = 0;			// clear test data
				// check remain data is all zero?
					for ( ; q < (short*)_RAM_END; q++) {
						if (*q) {
							flag = -1;	// detect invalid data
							break;
						}
					}
					break;
				}
			}
			if (flag != 1)
				check_end(1);		// address error
		}
	// check next RAM
		base += shortofs(_RAM_SIZ);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			check_data(check data bus brige(and swap))
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static
void	check_data(void)
{
	short	*base, *p, data;		// 16 bit data bus
	int		i;

// check each RAM
	base = (short*)_RAM_TOP;
	for (i = 0; i < _RAM_CNT; i++) {
		LED();
		p = base;
		data = 0;
		do {
			*p++ = data++;
		} while(data);
		p = base;
		data = 0;
		do {
			if (*p++ != data++) {
				check_end(2);		// data error
				break;
			}
		} while(data);
	// check next RAM
		base += shortofs(_RAM_SIZ);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			check_pattern(check data pattern each memory cell)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static
void	check_pattern(void)
{
	long	*base, *p, *q, data;
	int		i;

// check each block
	base = (long*)_RAM_TOP;
	for (i = 0; i < ((_RAM_SIZ*_RAM_CNT)/_CHK_SIZ); i++) {
	// check pattern
		p = base;
		q = p + longofs(_CHK_SIZ);
		data = 0x01010101L;				// check 4 bytes at once
		do {
			LED();
			p = base;
			do {
				*p++ = data;
			} while(p < q);
			p = base;
			do {
				if (*p++ != data) {
					check_end(2);		// data error
					break;
				}
			} while(p < q);
			data <<= 1;
		} while((data & 0xff) != 0);
	// check next block
		base = q;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			check_internal(check internal RAM)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static
void	check_internal(void)
{
	long	*p;

	LED();
	for (p = (long*)_INRAM_TOP; p < (long*)_INRAM_END; p++) {
		*p = (long)p;
	}
	for (p = (long*)_INRAM_TOP; p < (long*)_INRAM_END; p++) {
		if (*p != (long)p) {
			check_end(3);		// internal RAM error
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ram_check(RAM check)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
void	ram_check(void)
{
	uchar	sw;

// check DIPSW 3-2
	_misc2 = 0x10;
	OUTMISC2();
	_misc2 |= 0x04;
	OUTMISC2();
	wait2us( 50L );
	_misc2 &= ~(0x04);
	OUTMISC2();

	if ((sw & 0x20) == 0) {
		_misc2 |= (MISC2_CP_RAM_WPT|MISC2_CP_LD1);		// protect off & LD1 on
		OUTMISC2();
		check_address();			// address check
		check_data();				// data check
		check_pattern();			// pattern check
		_misc2 &= (uchar)(~MISC2_CP_RAM_WPT);			// protect on
		OUTMISC2();

		check_internal();			// internal RAM check
		check_end(0);				// address error
	}
}