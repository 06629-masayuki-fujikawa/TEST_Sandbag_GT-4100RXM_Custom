//[]----------------------------------------------------------------------[]
///	@mainpage		
///	external flashrom1(S29JL032H70TFI020) driver<br><br>
///	<b>Copyright(C) 2007 AMANO Corp.</b>
///	CREATE			2007/08/22 Y.FUJIWARA<br>
///	UPDATE			
///	@file			l_flashrom1.c
///	@date			2007/08/22
/// @note			l_flashrom2.cから流用(同じflashのため)
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/

#include	"iodefine.h"
#include	"system.h"
#include	"L_FLASHROM.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"FlashSerial.h"


/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

/* sector number of flash1 */
#define	FROM1_SECTOR_MAX	71

/* for accessing flash */
#define	_fmask(a)	((a) & MEMORY_ADR_MASK)
#define	_faddr(a)	((ulong)(FROM1_ADDRESS+(a)))
#define	_fout(a,d)	*(volatile ushort*)_faddr(a) = (ushort)(d)
#define	_fin(a)		*(volatile ushort*)_faddr(a)
#define	_waddr(a)	((a)<<1)

/* timers */
#define	_2usTo2ms(t)				((t)/1000+2)		/* [2us]⇒[2ms] */
#define	FROM1_WRITE_TIMEOUT		FLASH_WORD_WRITE_MAX
#define	FROM1_ERASE_TIMEOUT		FLASH_SECTOR_ERASE_MAX

#define	FROM1_TIMER_STOP		0x8000

#define	FROM1_SLEEP_INTERVAL	0x00000FFF

#define	STBY_AREA_TOP			FROM1_SA29
#define	FROM1_NORMAL			FROM2_NORMAL

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T													*/
/*--------------------------------------------------------------------------*/

/* address of sector (offset0=sector0, offset1=sector1, offset2=sector2, ....) */
extern const ulong const_From2SectAddr[];

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/

/* timer counter to check write/erase timeout */
extern	ushort	FROM2_timer;
#define	FROM1_timer		FROM2_timer


/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

static ulong	from1_write_word(ulong addr, ushort  data);
static ulong	from1_wait(ulong addr, ushort data, ulong timeout, BOOL write);
static void		from1_sleep(long count);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*==================================================================================[FROM1 LIBRARY FUNCTIONS]====*/

//[]----------------------------------------------------------------------[]
///	@brief			Flash1Init(initialize flashrom1 driver)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
void	Flash1Init(void)
{
//	FROM1_timer		= FROM1_TIMER_STOP;
}

//[]----------------------------------------------------------------------[]
///	@brief			Flash1EraseBlock(erase sector)
//[]----------------------------------------------------------------------[]
///	@param[in]		addr	: top address of sector erased
///	@return					: FROM1_NORMAL = normal
///							: FROM1_NORMAL != abnormal(address)
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
#define SMALL_SECTOR_NUM		8

ulong Flash1EraseBlock(ulong addr)
{
// 対象は待機面のみ
	addr = _fmask(addr);
	if (addr < STBY_AREA_TOP)
		return	addr;
// １ブロック＝64Kとする
// 面情報によって、8K×8セクタの場合と、64K×1セクタの場合を判別
	FlashErase_direct( addr, 2 );
	return FROM1_NORMAL;
}

//[]----------------------------------------------------------------------[]
///	@brief			Flash1Write(Flash Byte N Write)
//[]----------------------------------------------------------------------[]
///	@param[in]		addr	: address, writing data to
///	@param[in]		data	: data to write
///	@param[in]		cnt		: data count to write
///	@return					: FROM1_NORMAL = normal
///							: FROM1_NORMAL != abnormal(address)
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
ulong Flash1Write(ulong addr, uchar *data, ulong cnt)
{
	union {
		uchar	c[2];
		ushort	w;
	}u;
	ulong	ret;
	long	i;
	ulong	addrw;
	
	addrw = _fmask(addr);
	if (cnt <= 0) {
		return FROM1_NORMAL;
	}
	if (addrw < STBY_AREA_TOP)
		return	addr;
	addr = addrw;

	if (addr & 1) {
	/* 奇数アドレスの先頭1バイト */
		u.w		= _fin(addr-1);
		u.c[1]	= *data++;
		ret = from1_write_word(addr-1, u.w);
		if (ret != FROM1_NORMAL) {
			return ret;		/* error occurred! */
		}
		addr++;
		cnt--;
	}
	
	for (i = 0; i < (long)(cnt - 1); i += 2) {
		u.c[0] = *data++;
		u.c[1] = *data++;
		ret = from1_write_word(addr, u.w);
		if (ret != FROM1_NORMAL) {
			return ret;		/* error occurred! */
		}
		addr += 2;
		
		from1_sleep(i);
	}
	
	if ((long)cnt - i > 0) {
	/* 最後の偶数アドレス1バイト */
		u.w		= _fin(addr);
		u.c[0]	= *data;
		ret = from1_write_word(addr, u.w);
		if (ret != FROM1_NORMAL) {
			return ret;		/* error occurred! */
		}
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			Flash1Cmp(compare memory with flash)
//[]----------------------------------------------------------------------[]
///	@param[in]		faddr	: flash address
///	@param[in]		maddr	: memory address to compare with
///	@param[in]		size	: size to compare
///	@return			1/0		: 1=
///							  0=equal
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
uchar	Flash1Cmp(ulong faddr, uchar *maddr, ulong size)
{
	union {
		uchar	c[2];
		ushort	w;
	}u;
	long i;
	
	faddr = _fmask(faddr);
	
	if (size <= 0) {
		return 0;
	}
	
	if (faddr & 1) {
	/* 先頭の奇数アドレス1バイト */
		u.w = _fin(faddr-1);
		if (u.c[1] != *maddr) {
			return 1;
		}
		maddr++;
		faddr++;
		size--;
	}
	
	for (i = 0; i < (long)(size - 1); i += 2) {
		u.w = _fin(faddr + i);
		u.w = _fin(faddr + i);
		if (u.c[0] != *maddr++) {
			return 1;
		}
		if (u.c[1] != *maddr++) {
			return 1;
		}
		
		from1_sleep(i);
	}
	
	if ((long)size - i > 0) {
	/* 最後の偶数アドレス1バイト */
		u.w = _fin(faddr + i);
		if (u.c[0] != *maddr) {
			return 1;
		}
	}
	
	return 0;
}

/*==============================================================================================[PRIVATE]====*/
//[]----------------------------------------------------------------------[]
///	@brief			from1_write_word(write 1word)
//[]----------------------------------------------------------------------[]
///	@param[in]		addr	: address
///	@param[in]		data	: write data
///	@return					: FROM1_NORMAL = normal
///							: FROM1_NORMAL != abnormal(address)
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
ulong from1_write_word(ulong addr, ushort  data)
{
	ulong	ist;													// 現在の割込受付状態
	ulong	ret;

	ist = _di2();
	_fout(_waddr(0x0555L), 0xAAAA);
	_fout(_waddr(0x02AAL), 0x5555);
	_fout(_waddr(0x0555L), 0xA0A0);
	_fout(addr, data);
	_ei2( ist );
	
	ret = from1_wait(addr, data, FROM1_WRITE_TIMEOUT, TRUE);
	if (ret != FROM1_NORMAL) {
		FlashRegistErr(ERR_MAIN_FLASH_WRITE_ERR, ret);
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			from1_wait(Wait to complete to write/erase)
//[]----------------------------------------------------------------------[]
///	@param[in]		addr	: address to be written
///	@param[in]		data	: data to be written(when erase, should be set 0xFFFF)
///	@param[in]		timeout	: timeout	[2us]
///	@param[in]		write	: TRUE as waiting to written
///	@return					: FROM1_NORMAL = normal
///							: FROM1_NORMAL != abnormal(address)
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
ulong from1_wait(ulong addr, ushort data, ulong timeout, BOOL write)
{
	data &= 0x80;
	
	FROM1_timer = (ushort)_2usTo2ms(timeout);	/* [2us] ⇒ [2ms] */
	
	while(1) {
		if ((_fin(addr) & 0x80) == data) {
			goto _success;
		}
		if (_fin(addr) & 0x20) {
			if ((_fin(addr) & 0x80) == data) {
				goto _success;
			}
			break;
		}
		if (!write) {		/* 書き込み完了待ち時は上の層でtaskchg()を呼ぶ。書き込み時間の短縮化のため */
			/* execute other task */
			taskchg(IDLETSKNO);
		}
		else {
			WACDOG;
			wait2us(1);
		}
		/* check timeout */
		if (FROM1_timer <= 0) {
			break;
		}
	}
	FROM1_timer = FROM1_TIMER_STOP;
	
/* error operation */
	_fout(addr, 0xf0);		/* read/reset */
	return _faddr(addr);	/* detect timeout! */

_success:
	FROM1_timer = FROM1_TIMER_STOP;
	return FROM1_NORMAL;
}

//[]----------------------------------------------------------------------[]
///	@brief			from1_sleep(task sleep)
//[]----------------------------------------------------------------------[]
///	@param[in]		count	: current value of counter
///	@return			void
///	@attention		None
/// @note			処理時間を短くするため、taskchg()のコールはは2バイト毎ではなく下記単位で行う
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/22<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
void	from1_sleep(long count)
{
	if ((count & FROM1_SLEEP_INTERVAL) == 0) {
		/* execute other task */
		taskchg(IDLETSKNO);
	}
}