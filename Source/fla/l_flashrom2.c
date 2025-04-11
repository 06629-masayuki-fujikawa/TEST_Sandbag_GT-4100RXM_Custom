//[]----------------------------------------------------------------[]
// @brief	SPI connectecd FlashROM driver
//[]----------------------------------------------------------------[]
// @author	Y.Iiduka
// @date	Create	: 2011/12/14<br>
//[]--------------------------------- Copyright(C) 2011 AMANO Corp.-[]

//--------- I N C L U D E S ----------------------------------------//
#include	<string.h>
#include	"system.h"
#include	"sysmnt_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"FlashSerial.h"

//--------- D E F I N I T I O N S ----------------------------------//
#define MIN(x, y)	(((x)<=(y))? (x): (y))
#define MAX(x, y)	(((x)>=(y))? (x): (y))
// flash
#define SR_BSY		0x01
#define SR_BPL		0x08
// timers
#define	_2usTo2ms(t)	((t)/1000+2)	/* [2us]��[2ms] */
#define	ERASE_TIMEOUT	FLASH_SECTOR_ERASE_MAX	// (L_FLASHROM.h)
#define	WRITE_TIMEOUT	FLASH_WORD_WRITE_MAX	// (L_FLASHROM.h)
// error
#define ERR_MAIN_FLASH_ERASE_ERR	13	// FlashROM�������s(ope_def.h)
#define ERR_MAIN_FLASH_WRITE_ERR	14	// FlashROM�������ݎ��s(ope_def.h)
#define ERRMDL_MAIN			0	// Ҳݐ���Ӽޭ�ٺ���(ope_def.h)

//--------- V A L I A B L E S --------------------------------------//
ushort			FROM2_timer;	// timer value (decriment external)
static uchar	FROM2_waitmode = FROM2_WITHOUT_TSKCHG;
static	uchar	buf[256];

//--------- P R O T O T Y P E S ------------------------------------//
extern void	err_chk2(char md,char no, char knd, char f_data, char err_ctl, void *pData);	// (ope_def.h)

//--------- F U N C T I O N S --------------------------------------//
//============================================[LIBRARY FUNCTIONS]===//
//[]----------------------------------------------------------------[]
// @brief		initialize flashrom2 driver
//[]----------------------------------------------------------------[]
// @return		void
//[]----------------------------------------------------------------[]
// @author		machida.k
// @date		Create	: 2005/07/27<br>
//[]------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void Flash2Init(void)
{
	FROM2_timer = SYSMNT_TIMER_STOP;
	Flash2SetWaitMode(FROM2_WITHOUT_TSKCHG);
	FlashWriteStatus_direct(SR_BP_SET(0x0));	// all BP disable
}
	
//[]----------------------------------------------------------------[]
// @brief		set execution mode
//[]----------------------------------------------------------------[]
// @param[in]	mode	: execution mode
// @param[in]	WITH_TSKCHG	: wait with using tskchg
// @param[in]	WITHOUT_TSKCHG	: wait without using tskchg
// @return		void
//[]----------------------------------------------------------------[]
// @author		machida.k
// @date		Create	: 2005/07/27<br>
//[]------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void Flash2SetWaitMode(uchar mode)
{
	FROM2_waitmode = mode;
}
//[]----------------------------------------------------------------[]
// @brief		get flash wait mode
//[]----------------------------------------------------------------[]
// @param[in]	void
// @return		uchar mode
//[]----------------------------------------------------------------[]
// @author		K.Tanaka
// @date		Create	: 2012/03/23<br>
//[]------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar Flash2GetWaitMode( void )
{
	return FROM2_waitmode;
}
//[]----------------------------------------------------------------------[]
///	@brief		�Z�N�^�C���[�X
//[]----------------------------------------------------------------------[]
///	@param[in]	sect_addr:	�Z�N�^�ԍ�
///	@return		ret:		���s����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/27<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong Flash2EraseSector(ulong sect_addr)
{
	return Flash2EraseSector2(sect_addr);
}

//[]----------------------------------------------------------------------[]
///	@brief		�Z�N�^�C���[�X2
//[]----------------------------------------------------------------------[]
///	@param[in]	addr:	�Z�N�^�A�h���X
///	@return		ret:	���s����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/27<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong Flash2EraseSector2(ulong addr)
{
	From_Access_SemGet_Common( 1 );
	if( FROM2_waitmode==FROM2_WITH_TSKCHG ){
		FlashErase(addr, 1);	// 1: �Z�N�^
	}
	else{
		FlashErase_direct(addr, 1);
	}
	From_Access_SemFree(1);				// �A�N�Z�X���J��
	return FROM2_NORMAL;
}

//[]----------------------------------------------------------------------[]
///	@brief		FLASH���[�h
//[]----------------------------------------------------------------------[]
///	@param[out]	dst:	���[�h�f�[�^�i�[�A�h���X
///	@param[in]	addr:	���[�h�A�h���X
///	@param[in]	size:	�T�C�Y
///	@return		ret:	void
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/27<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Flash2Read(uchar *dst, ulong addr, ulong size)
{
	ulong	wn;
	From_Access_SemGet_Common( 1 );
	while( size>0 ){
		if( size>FLT_LOGSECT_SIZE ){
			wn = FLT_LOGSECT_SIZE;
		}
		else{
			wn = size;
		}
		if( FROM2_waitmode==FROM2_WITH_TSKCHG ){
			FlashReadData(addr, dst, wn);
		}
		else{
			FlashReadData_direct(addr, dst, wn);
		}
		addr += wn;
		dst += wn;
		size -= wn;
	}
	From_Access_SemFree(1);				// �A�N�Z�X���J��
}

//[]----------------------------------------------------------------------[]
///	@brief		FLASH���C�g
//[]----------------------------------------------------------------------[]
///	@param[in]	addr:	���C�g�A�h���X
///	@param[in]	dst:	���C�g�f�[�^�i�[�A�h���X
///	@param[in]	size:	�T�C�Y
///	@return		ret:	���s����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/27<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong Flash2Write(ulong addr, uchar *data, ulong cnt)
{
	From_Access_SemGet_Common( 1 );
	while( cnt>0 ){
		if( FROM2_waitmode==FROM2_WITH_TSKCHG ){
			FlashWriteData(addr, data);	// 256bytes
		}
		else{
			FlashWriteData_direct(addr, data);	// 256bytes
		}
		if( cnt<255 ){
			break;
		}
		cnt -= 256;
		addr += 256;
		data += 256;
	}
	From_Access_SemFree(1);				// �A�N�Z�X���J��
	return FROM2_NORMAL;
}

//[]----------------------------------------------------------------------[]
///	@brief		FLASH���C�g
//[]----------------------------------------------------------------------[]
///	@param[in]	addr:	���C�g�A�h���X
///	@param[in]	dst:	���C�g�f�[�^�i�[�A�h���X
///	@param[in]	size:	�T�C�Y
///	@return		ret:	���s����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/27<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Flash2Read_WpOff(uchar *dst, ulong addr, ulong size)
{
	Flash2Read(dst, addr, size);
}

//[]----------------------------------------------------------------------[]
///	@brief		FLASH�̃T�����Z�o����
//[]----------------------------------------------------------------------[]
///	@param[in]	addr:	�擪�A�h���X
///	@param[in]	size:	�T�C�Y
///	@return		ret:	���s����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/27<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar Flash2Sum(ulong addr, ulong size)
{
	int		i;
	ulong	n;
	uchar	sum;

	sum = 0;
	while( size>0 ){
		n = MIN(size, sizeof(buf));
		Flash2Read(buf, addr, n);
		for(i=0; i<n; i++){
			sum += buf[i];
		}
		addr += n;
		size -= n;
	}
	return sum;
}

//[]----------------------------------------------------------------------[]
///	@brief		FLASH�̃u�����L���O�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]	addr:	�擪�A�h���X
///	@param[in]	size:	�T�C�Y
///	@return		ret:	���s����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/27<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
BOOL Flash2ChkBlank(ulong addr, ulong size)
{
	int		i;
	ulong	n;

	while( size>0 ){
		n = MIN(size, sizeof(buf));
		Flash2Read(buf, addr, n);
		for(i=0; i<n; i++){
			if( buf[i]!=0xff ){
				return FALSE;
			}
		}
		addr += n;
		size -= n;
	}
	return TRUE;
}

//[]----------------------------------------------------------------------[]
///	@brief		FLASH�̃f�[�^��r
//[]----------------------------------------------------------------------[]
///	@param[in]	faddr:	FLASH�A�h���X
///	@param[in]	maddr:	SRAM�A�h���X
///	@param[in]	size:	�T�C�Y
///	@return		ret:	���s����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/27<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar Flash2Cmp(ulong faddr, uchar *maddr, ulong size)
{
	ulong	n;

	while( size>0 ){
		n = MIN(size, sizeof(buf));
		Flash2Read(buf, faddr, n);
		if( memcmp(buf, maddr, n) ){
			return 1;
		}
		faddr += n;
		maddr += n;
		size -= n;
	}
	return 0;
}

//===================================================[PRIVATE]======//

//[]----------------------------------------------------------------[]
// @brief		regist flash error
//[]----------------------------------------------------------------[]
// @param[in]	c	: flash error code
// @param[in]	r	: reason
// @return		void
//[]----------------------------------------------------------------[]
// @author		MATSUSHITA
// @date		Create	: 2008/06/23<br>
//[]------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void FlashRegistErr(char c, ulong r)
{
	err_chk2(ERRMDL_MAIN, c, 1, 0, 0, NULL);
}
//[]----------------------------------------------------------------------[]
///	@brief		�`�b�v�C���[�X
//[]----------------------------------------------------------------------[]
///	@param[in]	
///	@return		ret:	���s����
//[]----------------------------------------------------------------------[]
///	@author		a.iiizumi
///	@date		Create	: 2012/03/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong Flash2ChipErase(void)
{
	unsigned long i;

	From_Access_SemGet_Common( 1 );
	// ���S�ȃ`�b�v�C���[�X�͍s�Ȃ킸�Ƀu���b�N�C���[�X�ŉ����ƃv���O�����i�[�̈�ȊO���C���[�X����
	for(i = FROM1_SA0;i < SPI_FROM1_DATA_ENDADRESS; i += SPI_FROM1_BLOCK_SIZE) {
		if( FROM2_waitmode == FROM2_WITH_TSKCHG ){
			FlashErase(i, 2);
		}
		else{
			FlashErase_direct(i, 2);
		}
	}
	From_Access_SemFree(1);				// �A�N�Z�X���J��
	return FROM2_NORMAL;
}

//[]----------------------------------------------------------------------[]
///	@brief		�`�b�v�C���[�X(�S�A�h���X)
//[]----------------------------------------------------------------------[]
///	@param[in]	
///	@return		ret:	���s����
//[]----------------------------------------------------------------------[]
///	@author		t.Namioka
///	@date		Create	: 2012/05/17<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong Flash2ChipEraseComp(void)
{

	From_Access_SemGet_Common( 1 );
	if( FROM2_waitmode==FROM2_WITH_TSKCHG ){
		FlashErase(0, 0);	// 0: �`�b�v�C���[�X
	}
	else{
		FlashErase_direct(0, 0);
	}
	From_Access_SemFree(1);				// �A�N�Z�X���J��
	return FROM2_NORMAL;
}

uchar From_Access_SemGet_Common( uchar type )
{
	unsigned long			loopcnt;
	unsigned char			ret = 0;
	
	loopcnt = LifeTimGet();
	while( From_Access_SemGet(type) == 0){	// �A�N�Z�X���擾�s��
		taskchg( IDLETSKNO );			// task switch (a little bit wait)
		if( 9000 <= LifePastTimGet( loopcnt ) ){			// 1.3M �o���Ă������Ȃ��́A���M�ُ�Ɣ��f
			ret = 1;
//	DebugNo39	�����Ŗ���loop�����炨�����������m�F�����break��commentout����
			break;
		}
	}
	return ret;
}
