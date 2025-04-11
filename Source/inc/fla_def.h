#ifndef	___FLA_DEFH___
#define	___FLA_DEFH___
/*[]----------------------------------------------------------------------[]
 *|	filename: fla_def.h
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�V��2���ʒ�`
 *| author	: machida.k
 *| date	: 2005-07-26
 *| update	: 2008.11.17(MATSUSHITA) for CRW
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include <stdio.h>
#include "common.h"
#include "mem_def.h"
#include "rkn_def.h"
#include "rkn_cal.h"
#include "ope_def.h"
#include "L_FLASHROM.h"
#include "AppServ.h"

/*=====================================================================================< l_flashrom2.c >====*/

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

/* �O��FLASH2�������}�b�v�o�[�W���� */
#define	FLASH2_MAP_VERSION_0100		0x00010000		/* ver.1-0 */
#define	FLASH2_MAP_VERSION			0x00010000		/* current version */
// MAP�o�[�W�����́A�o�b�N�A�b�v�f�[�^���e�[�u���̐擪�f�[�^�̃A�h���X����
// �������܂�邪�A�o�[�W����0�̃v���O����(MAP�o�[�W�������̂����݂��Ȃ�)�ł�
// �����ɗL���f�[�^(�ŏ��̃o�b�N�A�b�v�f�[�^�̃o�b�N�A�b�v�̈�擪����̃I�t�Z�b�g�A�h���X)
// ���������܂�Ă���B
// ���̂��߁A�o�[�W����1���������l���A�o�b�N�A�b�v���e�[�u���T�C�Y(����218byte�g�p)���
// �啝�ɑ傫��0x00010000(���o�b�N�A�b�v�f�[�^���e�[�u���T�C�Y)�ɐݒ肷��B
// 0x00010000��菬�����l�̏ꍇ�A�o�[�W����0�̃v���O������������FLASH�Ƃ��Ĕ��f����B

/* wait mode */
#define	FROM2_WITH_TSKCHG		1
#define	FROM2_WITHOUT_TSKCHG	0

/* return value of write/erase */
#define	FROM2_NORMAL			0xFFFFFFFF

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
#define	_flt_DisableTaskChg()	Flash2SetWaitMode(FROM2_WITHOUT_TSKCHG)
#define	_flt_EnableTaskChg()	Flash2SetWaitMode(FROM2_WITH_TSKCHG)

#define	_flt_DestroyParamRamSum()		(DataSum[OPE_DTNUM_COMPARA].Sum = 0)
#define	_flt_DestroyLockParamRamSum()	(DataSum[OPE_DTNUM_LOCKINFO].Sum = 0)

extern	void	Flash2Init(void);
extern	void	Flash2SetWaitMode(uchar mode);
extern 	uchar	Flash2GetWaitMode( void );
extern	ulong	Flash2EraseSector(ulong sect_addr);
extern	ulong	Flash2Write(ulong  addr, uchar *data, ulong cnt);
extern	void	Flash2Read(uchar *dst, ulong src_addr, ulong size);
extern	uchar	Flash2Sum(ulong addr, ulong size);
extern	BOOL	Flash2ChkBlank(ulong addr, ulong size);
extern	uchar	Flash2Cmp(ulong faddr, uchar *maddr, ulong size);
extern	ulong	Flash2EraseSector2(ulong addr);
extern	void	Flash1Init(void);
extern	ulong	Flash1EraseBlock(ulong addr);
extern	ulong	Flash1Write(ulong  addr, uchar *data, ulong cnt);
extern	uchar	Flash1Cmp(ulong faddr, uchar *maddr, ulong size);

extern	ulong	flt_ReadEdyAt(char *buf);
extern	void	FlashRegistErr(char c, ulong r);
extern	ulong Flash2ChipErase(void);
extern	ulong Flash2ChipEraseComp(void);

extern	uchar From_Access_SemGet_Common( uchar );
/*--------------------------------------------------------------------------*/
/*			E X T E R N A L   V A L I A B L E S								*/
/*--------------------------------------------------------------------------*/

extern	ushort	FROM2_timer;


/*=====================================================================================< flatask.c >====*/

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/
#define LOG_STRAGEN	21		// ���O�i�[���ڐ�
#define	FLT_LOGSECT_SIZE	0x1000
#define	FLT_LOGSECT_SIZE2	0x2000

/* flags */
#define	FLT_EXCLUSIVE		0x01
#define	FLT_NOT_EXCLUSIVE	0x00

/* to access return value */
#define	_FLT_RtnKind(rtn)	(((rtn) & 0xFF000000) >> 8*3)
#define	_FLT_RtnDetail(rtn)	((rtn) & 0x00FFFFFF)

/* kind of return value */
#define	FLT_NORMAL			0
#define	FLT_ERASE_ERR		1
#define	FLT_WRITE_ERR		2
#define	FLT_VERIFY_ERR		3
#define	FLT_PARAM_ERR		4
#define	FLT_BUSY			5
#define	FLT_NODATA			6
#define	FLT_NOT_LOCKED		7
#define	FLT_INVALID_SIZE	8

#define	FLT_PARAM_WRITTEN	1

/* sizes */
#define	FLT_PRINTLOGO_SIZE			8190	/* 8192-(size of sum) */
#define	FLT_PRINTHEADER_SIZE		144
#define	FLT_PRINTFOOTER_SIZE		FLT_PRINTHEADER_SIZE
#define	FLT_SALELOG_RECORD_SIZE		sizeof(Receipt_data)
#define	FLT_TOTALLOG_RECORD_SIZE	sizeof(Syu_log)
#define	FLT_PRINTSYOMEI_SIZE		144
#define	FLT_PRINTKAMEI_SIZE			FLT_PRINTSYOMEI_SIZE
#define	FLT_EDYATCMD_SIZE			120		// 20����*6�s

#define	FLT_CREDIT_SIZE				FLT_PRINTHEADER_SIZE
#define	FLT_ACCEPTFOOTER_SIZE		FLT_PRINTHEADER_SIZE

#define	FLT_TCARDFTR_SIZE			360		// 36*T_FOOTER_GYO_MAX(10)
#define	FLT_BR_LOG_SIZE				sizeof(t_FltParam_BRLOGSZ)*eLOG_MAX

#define	FLT_TCARD_DATA_SIZE			1142
#define	FLT_HCARD_DATA_SIZE			1122
#define	FLT_OFS_HCARD				4096			// �I�t�Z�b�g��1�Z�N�^
#define	FLT_OFS_VERTION(a)			(ulong)(((a+1)*4096)-3)// �Z�N�^�̍Ōォ��3�o�C�g�O���o�[�W�����A2�o�C�g���f�[�^�T�C�Y�Ƃ���
#define	FLT_T_POINT_DATA_SIZE		20

#define	FLT_AZUFTR_SIZE			144		// 36*4

// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
#define	FLT_CREKABARAI_SIZE		144		// 36*4
#define	FLT_EPAYKABARAI_SIZE	144		// 36*4
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
#define	FLT_FUTURE_SIZE			144		// 36*4
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define	FLT_PRINTEMGFOOTER_SIZE		FLT_PRINTHEADER_SIZE
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

#define	FLT_USERDEF_SIZE			2048
#define	FLT_CHKRESULT_SIZE			sizeof(t_Chk_res_ftp)

/* �t���b�V�����I�t�Z�b�g */
#define	FLT_OFS_SUM					0
#define	FLT_OFS_SUM_MIRROR			1
#define	FLT_HEADER_SIZE				2

extern ushort	BR_LOG_SIZE[eLOG_MAX];
extern uchar	FLASH_WRITE_BUFFER[FLT_LOGSECT_SIZE2];

// FT4800 layout
typedef	struct {
		char	logo[FLT_HEADER_SIZE];
		char		dlogo[FLT_PRINTLOGO_SIZE];
		char	header[FLT_HEADER_SIZE];
		char		dheader[FLT_PRINTHEADER_SIZE];
		char	footer[FLT_HEADER_SIZE];
		char		dfooter[FLT_PRINTFOOTER_SIZE];
		char	credit1[FLT_HEADER_SIZE];
		char		dcredit1[FLT_CREDIT_SIZE];
		char	credit2[FLT_HEADER_SIZE];
		char		dcredit2[FLT_CREDIT_SIZE];
		char	acceptfooter[FLT_HEADER_SIZE];
		char		dacceptfooter[FLT_ACCEPTFOOTER_SIZE];
		char	syomei[FLT_HEADER_SIZE];
		char		dsyomei[FLT_PRINTSYOMEI_SIZE];
		char	kamei[FLT_HEADER_SIZE];
		char		dkamei[FLT_PRINTKAMEI_SIZE];
		char	atcmd[FLT_HEADER_SIZE];
		char		datcmd[FLT_EDYATCMD_SIZE];
		char	userdef[FLT_HEADER_SIZE];
		char		duserdef[FLT_USERDEF_SIZE];
		char	tcardftr[FLT_HEADER_SIZE];
		char		dtcardftr[FLT_TCARDFTR_SIZE];
		char	azuftr[FLT_HEADER_SIZE];
		char		dazuftr[FLT_AZUFTR_SIZE];
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
		char	crekbrftr[FLT_HEADER_SIZE];				// �N���W�b�g�U�։ߕ����t�b�^�[
		char		dcrekbrftr[FLT_CREKABARAI_SIZE];	// �N���W�b�g�U�։ߕ����t�b�^�[
		char	epaykbrftr[FLT_HEADER_SIZE];			// �d�q�}�l�[�U�։ߕ����t�b�^�[
		char		depaykbrftr[FLT_EPAYKABARAI_SIZE];	// �d�q�}�l�[�U�։ߕ����t�b�^�[
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
		char	futureftr[FLT_HEADER_SIZE];			// ����x���z�t�b�^�[
		char		dfutureftr[FLT_FUTURE_SIZE];	// ����x���z�t�b�^�[
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		char	emgfooter[FLT_HEADER_SIZE];
		char		demgfooter[FLT_PRINTEMGFOOTER_SIZE];
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

		char	end;
} _PRNSECT_IMG;

/* t_FltLogHandle::kind */
enum {
	FLT_SEARCH_LOG,
	FLT_SEARCH_NONE = FLT_SEARCH_LOG+LOG_STRAGEN,
};

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/*-------------------------------------[ PUBLIC ]-----*/

/* LOG�����n���h�� */
typedef struct {
	ushort	kind;
	ushort	sectno;
	ushort	recordno;
}t_FltLogHandle;

/*-------------------------------------[ PRIVATE ]-----*/

/* �v����� */
enum {
	FLT_REQTBL_LOG = 0,			// = eLOG_PAYMENT�Ƃ��邱��
	FLT_REQTBL_PARAM1 = FLT_REQTBL_LOG+LOG_STRAGEN,
	FLT_REQTBL_PARAM2,
	FLT_REQTBL_LOGO,
	FLT_REQTBL_HEADER,
	FLT_REQTBL_FOOTER,
	FLT_REQTBL_BACKUP,
	FLT_REQTBL_RESTORE,
	FLT_REQTBL_ERASEALL,
	FLT_REQTBL_LOCKPARAM1,
	FLT_REQTBL_LOCKPARAM2,
	FLT_REQTBL_ACCEPTFOOTER,
	FLT_REQTBL_SYOMEI,
	FLT_REQTBL_KAMEI,
	FLT_REQTBL_EDYAT,
	FLT_REQTBL_EPAY_LOG,
	FLT_REQTBL_USERDEF,
	FLT_REQTBL_ETC,
	FLT_REQTBL_IMAGE,
	FLT_REQTBL_PARAM3,
	FLT_REQTBL_TCARDFTR,
	FLT_REQTBL_CARDDATA,
	FLT_REQTBL_AZUFTR,
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
	FLT_REQTBL_CREKBRFTR,
	FLT_REQTBL_EPAYKBRFTR,
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
	FLT_REQTBL_FUTUREFTR,
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	FLT_REQTBL_EMGFOOTER,
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	FLT_REQTBL_BRLOGSZ,

	FLT_REQTBL_MAX
};

/*----------�� �������}�b�v ��----------*/
#define SPI_FROM1_BLOCK_SIZE	0x10000L // 64K
#define SPI_FROM1_DATA_ENDADRESS	0x6FFFFFL // ���O�̈�A�p�����[�^�A�o�b�N�A�b�v�G���A�܂ł̃A�h���X


#define FLT_LOG0_SECTOR		FROM1_SA0
#define FLT_LOG0_SECTORMAX	838
#define FLT_LOG1_SECTOR		FROM1_SA1
#define FLT_LOG1_SECTORMAX	192
#define FLT_LOG2_SECTOR		FROM1_SA2
#define FLT_LOG2_SECTORMAX	31
#define FLT_LOG3_SECTOR		FROM1_SA3
#define FLT_LOG3_SECTORMAX	31
#define FLT_LOG4_SECTOR		FROM1_SA4
#define FLT_LOG4_SECTORMAX	8
#define FLT_LOG5_SECTOR		FROM1_SA5
#define FLT_LOG5_SECTORMAX	8
#define FLT_LOG6_SECTOR		FROM1_SA6
#define FLT_LOG6_SECTORMAX	4
#define FLT_LOG7_SECTOR		FROM1_SA7
#define FLT_LOG7_SECTORMAX	4
#define FLT_LOG8_SECTOR		FROM1_SA8
#define FLT_LOG8_SECTORMAX	4
#define FLT_LOG9_SECTOR		FROM1_SA9
#define FLT_LOG9_SECTORMAX	4
#define FLT_LOG10_SECTOR	FROM1_SA10
#define FLT_LOG10_SECTORMAX	4
#define FLT_LOG11_SECTOR	FROM1_SA11
#define FLT_LOG11_SECTORMAX	8
#define FLT_LOG12_SECTOR	FROM1_SA12
#define FLT_LOG12_SECTORMAX	80
#define FLT_LOG13_SECTOR	FROM1_SA13
#define FLT_LOG13_SECTORMAX	4
#define FLT_LOG14_SECTOR	FROM1_SA14
#define FLT_LOG14_SECTORMAX	4
#define FLT_LOG15_SECTOR	FROM1_SA15
#define FLT_LOG15_SECTORMAX	4
#define FLT_LOG16_SECTOR	FROM1_SA16
#define FLT_LOG16_SECTORMAX	6
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//#define FLT_LOG17_SECTOR	FROM1_SA17
//#define FLT_LOG17_SECTORMAX	4
#define FLT_LOG17_SECTOR	FROM1_SA40
#define FLT_LOG17_SECTORMAX	12
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
#define FLT_LOG18_SECTOR	FROM1_SA18
#define FLT_LOG18_SECTORMAX	8
#define FLT_LOG19_SECTOR	FROM1_SA19
#define FLT_LOG19_SECTORMAX	5
#define FLT_LOG20_SECTOR	FROM1_SA38
#define FLT_LOG20_SECTORMAX	4

// ���ʃp�����[�^
#define	FLT_PARAM_SECTOR_1		FROM1_SA20
#define	FLT_PARAM_SECTOR_2		FROM1_SA21
#define	FLT_PARAM_SECTORMAX		16
#define	FLT_PARAM_SECT_SIZE		0x1000

// �v�����^���S�A�w�b�_�A�t�b�^
#define	FLT_PRINTDATA_SECTOR	FROM1_SA22
#define	FLT_PRINTDATA_SECTORMAX	16
#define	FLT_PRINTDATA_SECT_SIZE	0x1000

// �Ԏ��p�����[�^
#define	FLT_LOCKPARAM_SECTOR	FROM1_SA23
#define	FLT_LOCKPARAM_SECTORMAX	4
#define	FLT_LOCKPARAM_SECT_SIZE	0x1000

// T�J�[�h�Ή��E�@�l�J�[�h
#define	FLT_CARDDATA_SECTOR		FROM1_SA24
#define	FLT_CARDDATA_SECTORMAX	2
#define	FLT_CARDDATA_SECT_SIZE	0x1000

#define	FLT_BRLOGSZ_SECTOR		FROM1_SA25
#define	FLT_BRLOGSZ_SECTORMAX	1
#define	FLT_BRLOGSZ_SECT_SIZE	0x1000

// �o�b�N�A�b�v
#define	FLT_BACKUP_SECTOR0		FROM1_SA26
#define	FLT_BACKUP_SECTORMAX	200 //�o�b�N�A�b�v�̈�̍ŏI�Z�N�^�̓��O�o�[�W�������G���A�ΏۊO�Ƃ���
#define	FLT_BACKUP_SECT_SIZE	0x1000

#define	FLT_BACKUP_SECTOR1			FROM1_SA26+FLT_BACKUP_SECT_SIZE
#define	FLT_BACKUP_SECTOR1_COUNT	1

#define	FLT_BACKUP_SECTOR2			FLT_BACKUP_SECTOR1+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR1_COUNT)
#define	FLT_BACKUP_SECTOR2_COUNT	1

#define	FLT_BACKUP_SECTOR3			FLT_BACKUP_SECTOR2+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR2_COUNT)
#define	FLT_BACKUP_SECTOR3_COUNT	3

#define	FLT_BACKUP_SECTOR4			FLT_BACKUP_SECTOR3+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR3_COUNT)
#define	FLT_BACKUP_SECTOR4_COUNT	8

#define	FLT_BACKUP_SECTOR5			FLT_BACKUP_SECTOR4+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR4_COUNT)
#define	FLT_BACKUP_SECTOR5_COUNT	1

#define	FLT_BACKUP_SECTOR6			FLT_BACKUP_SECTOR5+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR5_COUNT)
#define	FLT_BACKUP_SECTOR6_COUNT	1

#define	FLT_BACKUP_SECTOR7			FLT_BACKUP_SECTOR6+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR6_COUNT)
#define	FLT_BACKUP_SECTOR7_COUNT	1

#define	FLT_BACKUP_SECTOR8			FLT_BACKUP_SECTOR7+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR7_COUNT)
#define	FLT_BACKUP_SECTOR8_COUNT	1

#define	FLT_BACKUP_SECTOR9			FLT_BACKUP_SECTOR8+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR8_COUNT)
#define	FLT_BACKUP_SECTOR9_COUNT	1

#define	FLT_BACKUP_SECTOR10			FLT_BACKUP_SECTOR9+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR9_COUNT)
#define	FLT_BACKUP_SECTOR10_COUNT	9

#define	FLT_BACKUP_SECTOR11			FLT_BACKUP_SECTOR10+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR10_COUNT)
#define	FLT_BACKUP_SECTOR11_COUNT	9

#define	FLT_BACKUP_SECTOR12			FLT_BACKUP_SECTOR11+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR11_COUNT)
#define	FLT_BACKUP_SECTOR12_COUNT	5

#define	FLT_BACKUP_SECTOR13			FLT_BACKUP_SECTOR12+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR12_COUNT)
#define	FLT_BACKUP_SECTOR13_COUNT	5

#define	FLT_BACKUP_SECTOR14			FLT_BACKUP_SECTOR13+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR13_COUNT)
#define	FLT_BACKUP_SECTOR14_COUNT	1

#define	FLT_BACKUP_SECTOR15			FLT_BACKUP_SECTOR14+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR14_COUNT)
#define	FLT_BACKUP_SECTOR15_COUNT	1

#define	FLT_BACKUP_SECTOR16			FLT_BACKUP_SECTOR15+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR15_COUNT)
#define	FLT_BACKUP_SECTOR16_COUNT	1

#define	FLT_BACKUP_SECTOR17			FLT_BACKUP_SECTOR16+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR16_COUNT)
#define	FLT_BACKUP_SECTOR17_COUNT	1

#define	FLT_BACKUP_SECTOR18			FLT_BACKUP_SECTOR17+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR17_COUNT)
#define	FLT_BACKUP_SECTOR18_COUNT	1

#define	FLT_BACKUP_SECTOR19			FLT_BACKUP_SECTOR18+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR18_COUNT)
#define	FLT_BACKUP_SECTOR19_COUNT	1

#define	FLT_BACKUP_SECTOR20			FLT_BACKUP_SECTOR19+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR19_COUNT)
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
//#define	FLT_BACKUP_SECTOR20_COUNT	1
#define	FLT_BACKUP_SECTOR20_COUNT	5
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)

#define	FLT_BACKUP_SECTOR21			FLT_BACKUP_SECTOR20+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR20_COUNT)
#define	FLT_BACKUP_SECTOR21_COUNT	1

#define	FLT_BACKUP_SECTOR22			FLT_BACKUP_SECTOR21+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR21_COUNT)
#define	FLT_BACKUP_SECTOR22_COUNT	1

#define	FLT_BACKUP_SECTOR23			FLT_BACKUP_SECTOR22+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR22_COUNT)
#define	FLT_BACKUP_SECTOR23_COUNT	1

#define	FLT_BACKUP_SECTOR24			FLT_BACKUP_SECTOR23+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR23_COUNT)
#define	FLT_BACKUP_SECTOR24_COUNT	3

#define	FLT_BACKUP_SECTOR25			FLT_BACKUP_SECTOR24+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR24_COUNT)
#define	FLT_BACKUP_SECTOR25_COUNT	1

#define	FLT_BACKUP_SECTOR26			FLT_BACKUP_SECTOR25+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR25_COUNT)
#define	FLT_BACKUP_SECTOR26_COUNT	1

#define	FLT_BACKUP_SECTOR27			FLT_BACKUP_SECTOR26+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR26_COUNT)
#define	FLT_BACKUP_SECTOR27_COUNT	1

#define	FLT_BACKUP_SECTOR28			FLT_BACKUP_SECTOR27+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR27_COUNT)
#define	FLT_BACKUP_SECTOR28_COUNT	12

#define	FLT_BACKUP_SECTOR29			FLT_BACKUP_SECTOR28+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR28_COUNT)
#define	FLT_BACKUP_SECTOR29_COUNT	1

#define	FLT_BACKUP_SECTOR30			FLT_BACKUP_SECTOR29+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR29_COUNT)
#define	FLT_BACKUP_SECTOR30_COUNT	1

// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
#define	FLT_BACKUP_SECTOR31			FLT_BACKUP_SECTOR30+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR30_COUNT)
#define	FLT_BACKUP_SECTOR31_COUNT	1
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�

// MH321800(S) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
#define	FLT_BACKUP_SECTOR32			FLT_BACKUP_SECTOR31+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR31_COUNT)
#define	FLT_BACKUP_SECTOR32_COUNT	1

#define	FLT_BACKUP_SECTOR33			FLT_BACKUP_SECTOR32+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR32_COUNT)
#define	FLT_BACKUP_SECTOR33_COUNT	1
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)

// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
#define	FLT_BACKUP_SECTOR34			FLT_BACKUP_SECTOR33+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR33_COUNT)
#define	FLT_BACKUP_SECTOR34_COUNT	1

#define	FLT_BACKUP_SECTOR35			FLT_BACKUP_SECTOR34+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR34_COUNT)
#define	FLT_BACKUP_SECTOR35_COUNT	1
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
#define	FLT_BACKUP_SECTOR36			FLT_BACKUP_SECTOR35+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR35_COUNT)
// MH810103(s) �d�q�}�l�[�Ή� #5402 �V�X�e�������e�i���X�̑S�f�[�^�o�b�N�A�b�v�Ńx���t�@�C�G���[����������
//#define	FLT_BACKUP_SECTOR36_COUNT	2
// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6584 ���Z�f�[�^������ƃo�b�N�A�b�v���ł��Ȃ�
// #define	FLT_BACKUP_SECTOR36_COUNT	6
#define	FLT_BACKUP_SECTOR36_COUNT	7
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6584 ���Z�f�[�^������ƃo�b�N�A�b�v���ł��Ȃ�
// MH810103(s) �d�q�}�l�[�Ή� #5402 �V�X�e�������e�i���X�̑S�f�[�^�o�b�N�A�b�v�Ńx���t�@�C�G���[����������
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
#define	FLT_BACKUP_SECTOR37			FLT_BACKUP_SECTOR36+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR36_COUNT)
#define	FLT_BACKUP_SECTOR37_COUNT	1
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���

// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
#define	FLT_BACKUP_SECTOR38			FLT_BACKUP_SECTOR37+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR37_COUNT)
#define	FLT_BACKUP_SECTOR38_COUNT	5
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�

// FROM���̃��O�o�[�W����
#define	FLT_LOG_VERSION_SECTOR		FROM1_SA37
#define	FLT_LOG_VERSION_SECTORMAX	1
#define	FLT_LOG_VERSION_SECT_SIZE	0x1000
#define	FLT_LOG_VERSION_DATA_SIZE	1

// �����f�[�^
#define	FLT_SOUND0_SECTOR		FROM1_SA27
#define	FLT_SOUND0_SECTORMAX	640
#define	FLT_SOUND0_SECT_SIZE	0x1000

// �����f�[�^�i�_�E�����[�h�p�j
#define	FLT_SOUND1_SECTOR		FROM1_SA28
#define	FLT_SOUND1_SECTORMAX	640
#define	FLT_SOUND1_SECT_SIZE	0x1000

// �v���O����
#define	FLT_PROGRAM_SECTOR		FROM1_SA29
#define	FLT_PROGRAM_SECTORMAX	512
#define	FLT_PROGRAM_SECT_SIZE	0x1000

// �\��
#define	FLT_RESERVE1_SECTOR		FROM1_SA30
// MH322914 (s) kasiyama 2016/07/13 FlashROM�������A�h���X�V���{���ԈႢ�C��[���ʃo�ONo.1271]
//#define	FLT_RESERVE1_SECTORMAX	62
#define	FLT_RESERVE1_SECTORMAX	208
// MH322914 (e) kasiyama 2016/07/13 FlashROM�������A�h���X�V���{���ԈႢ�C��[���ʃo�ONo.1271]
#define	FLT_RESERVE1_SECT_SIZE	0x1000

#define	FLT_RESERVE2_SECTOR		FROM1_SA31
#define	FLT_RESERVE2_SECTORMAX	768
#define	FLT_RESERVE2_SECT_SIZE	0x1000

#define	FLT_SUM_VER_DATA_SIZE	18

// �����f�[�^�^�p�ʏ��
#define	FLT_WAVE_SWDATA_SECTOR			FROM1_SA32
#define	FLT_WAVE_SWDATA_SECTORMAX		1
#define	FLT_WAVE_SWDATA_SECT_SIZE		0x1000
#define	FLT_WAVE_SWDATA_DATA_SIZE		2

// ���ʃp�����[�^�^�p�ʏ��
#define	FLT_PARM_SWDATA_SECTOR			FROM1_SA33
#define	FLT_PARM_SWDATA_SECTORMAX		1
#define	FLT_PARM_SWDATA_SECT_SIZE		0x1000
#define	FLT_PARM_SWDATA_DATA_SIZE		2

// �����f�[�^�^�p��1�f�[�^�`�F�b�N�T���A�o�[�W����
#define	FLT_SOUND0_SUM_VER_SECTOR		FROM1_SA34
#define	FLT_SOUND0_SUM_VER_SECTORMAX	1
#define	FLT_SOUND0_SUM_VER_SECT_SIZE	0x1000

// �����f�[�^�^�p��2�f�[�^�`�F�b�N�T���A�o�[�W����
#define	FLT_SOUND1_SUM_VER_SECTOR		FROM1_SA35
#define	FLT_SOUND1_SUM_VER_SECTORMAX	1
#define	FLT_SOUND1_SUM_VER_SECT_SIZE	0x1000

// �v���O�����f�[�^�`�F�b�N�T���A�o�[�W����
#define	FLT_PROGRAM_SUM_VER_SECTOR		FROM1_SA36
#define	FLT_PROGRAM_SUM_VER_SECTORMAX	1
#define	FLT_PROGRAM_SUM_VER_SECT_SIZE	0x1000
/*----------�� �������}�b�v ��----------*/

#define	GET_MAPP		(PRG_HDR*)(FROM1_SA29)
#define	GET_VAPP(sw)	(PRG_HDR*)(sw==0?FROM1_SA27:FROM1_SA28)
#define	GET_MAIN_H		(PRG_HDR*)(FROM1_SA36)
#define	GET_VOICE_H(sw)	(PRG_HDR*)(sw==0?FROM1_SA34:FROM1_SA35)

/* ���[�N�o�b�t�@�T�C�Y */
#define	FLT_WKBUF_SIZE			65536	/* 64*1024 */

/* �v����t��� */
typedef struct {
	uchar	req;				/* �v����t��� */
	ulong	result;				/* ���s���� */
}t_FltReq;

/* �������݂Ɏg�p����p�����[�^(API�˃t���b�V���^�X�N�ւ̈����n���p) */
/* for FLT_REQTBL_PARAM2 */
typedef struct {
	const char	*param_image;
	size_t		image_size;
}t_FltParam_Param2;
/* for FLT_REQTBL_SALE_LOG and FLT_REQTBL_TOTAL_LOG */
typedef struct {
	const char	*records;
	short		record_num;
	short		sramlog_kind;// SRAM���LOG(4Kbyte2�ʂ̂ǂ���̏��������������������f�[�^)
	ushort		f_recover;// ���d���̏�Ԃ������t���O
	short		stat_Fcount;// ����
	ushort		stat_sectnum;// �Z�N�^��
	ushort		get_sector;// GetWriteLogSectr()�Ŏ擾����l
	ushort		get_seqno;// GetWriteLogSectr()�Ŏ擾����l
}t_FltParam_Log;
/* for FLT_REQTBL_LOGO */
typedef struct {
	const char	*image;
}t_FltParam_Logo;
/* for FLT_REQTBL_HEADER */
typedef struct {
	const char	*image;
}t_FltParam_Header;
/* for FLT_REQTBL_FOOTER */
typedef struct {
	const char	*image;
}t_FltParam_Footer;

/* for FLT_REQTBL_LOCKPARAM */
typedef struct {
	const char	*param_image;
}t_FltParam_LockParam2;

/* for FLT_REQTBL_BACKUP */
typedef struct {
	ulong		ver_flg;
}t_FltParam_Bkup;

/* for FLT_REQTBL_ACCEPTFOOTER */
typedef struct {
	const char	*image;
}t_FltParam_AcceptFooter;

/* for FLT_REQTBL_SYOMEI */
typedef struct {
	const char	*image;
}t_FltParam_Syomei;
/* for FLT_REQTBL_KAMEI */
typedef struct {
	const char	*image;
}t_FltParam_Kamei;

/* for FLT_REQTBL_EDYAT */
typedef struct {
	const char	*image;
}t_FltParam_EdyAt;

/* for FLT_REQTBL_TCARDFTR */
typedef struct {
	const char	*image;
}t_FltParam_TCardFtr;
typedef struct {
	char	card_syu;
}t_FltParam_CardSyu;

/* for FLT_REQTBL_AZUFTR */
typedef struct {
	const char	*image;
}t_FltParam_AzuFtr;
// �d�l�ύX(S) K.Onodera 2016/10/31 #1531 �̎��؃t�b�^�[���󎚂���
/* for FLT_REQTBL_CREKBRFTR */
typedef struct {
	const char	*image;
}t_FltParam_CreKbrFtr;

/* for FLT_REQTBL_EPAYKBRFTR */
typedef struct {
	const char	*image;
}t_FltParam_EpayKbrFtr;
// �d�l�ύX(E) K.Onodera 2016/10/31 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
/* for FLT_REQTBL_FUTUREFTR */
typedef struct {
	const char	*image;
}t_FltParam_FutureFtr;
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
/* for FLT_REQTBL_EMGFOOTER */
typedef struct {
	const char	*image;
}t_FltParam_EmgFooter;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
/* for FLT_REQTBL_BRLOGSZ */
typedef struct {
	ushort	log_size;
}t_FltParam_BRLOGSZ;

typedef	t_FltParam_Header	t_FltParam_UserDef;			// �w�b�_�Ɠ��^

typedef	t_FltParam_Header	t_FltParam_Etc;				// �w�b�_�Ɠ��^
typedef	struct {
	const	char	*image;
	ulong			len;
	ulong			flash;
	int				erase;
} t_FltParam_Image;

typedef struct {
	t_FltParam_Log		log[LOG_STRAGEN];
	t_FltParam_Param2	param2;
	t_FltParam_Logo		logo;
	t_FltParam_Header	header;
	t_FltParam_Footer	footer;
	t_FltParam_LockParam2	lockparam2;
	t_FltParam_Bkup		bkup;
	t_FltParam_AcceptFooter	acceptfooter;
	t_FltParam_Syomei	syomei;
	t_FltParam_Kamei	kamei;
	t_FltParam_EdyAt	edyat;

	t_FltParam_UserDef	userdef;
	t_FltParam_Etc		etc;
	t_FltParam_Image	image;
	t_FltParam_Param2	param3;
	t_FltParam_TCardFtr	tcardftr;
	t_FltParam_CardSyu	syubet;
	t_FltParam_AzuFtr	azuftr;
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
	t_FltParam_CreKbrFtr	crekbrftr;
	t_FltParam_EpayKbrFtr	epaykbrftr;
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
	t_FltParam_FutureFtr	futureftr;
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
	t_FltParam_BRLOGSZ	brlogsz[eLOG_MAX];
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	t_FltParam_EmgFooter	emgfooter;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
}t_FltParamForWrite;

/* �t���b�V����f�[�^������ */
/* �Z�N�^����LOG��� */
typedef struct {
	ulong		address;		/* �Y���Z�N�^�擪�A�h���X */
	ushort		seqno;			/* �ǂ��� */
	ushort		record_num;		/* �i�[���R�[�h���� */
}t_FltLogInfo;
/* �t���b�V�����LOG�Ǘ���� */
typedef struct {
	ushort			secttop;		/* �擪�Z�N�^ */
	ushort			sectnum;		/* �L���f�[�^������Z�N�^�� */
	ushort			sectmax;		/* �ő�Z�N�^�� */
	ushort			record_size;	/* ���R�[�h1���̃T�C�Y */
	ushort			recordnum_max;	/* 1�Z�N�^������̍ő僌�R�[�h���� */
	t_FltLogInfo	sectinfo[FLT_LOG0_SECTORMAX];	/* ���������ʉ��̂��߁A�S�Ă��̃T�C�Y */
}t_FltLogMgr;

typedef struct {
	t_FltLogMgr		log_mgr[LOG_STRAGEN];	// LOG�̈�Ǘ����
}t_FltDataInfo;

/* �o�b�N�A�b�v�����A�t���b�V������f�[�^ */
// MH322917(S) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
// ���ӎ����Ft_FLT_Ctrl_Backup�\���̂�t_FltParamForWrite�At_FltDataInfo��FROM�������ݎ��̒�d���J�o���p�̃G���A
// �̂��߃������z�u��ύX���邱�Ƃ��֎~����B(���O�������ݒ��Ƀo�[�W�����A�b�v�𔺂���d����ƃf�[�^�����邽��)
// t_AppServ_ParamInfoTbl�ɊY��������e�ɒǉ����K�v�ƂȂ�
// �ꍇ�͍\���̂̍Ō���ɐV�K�����o�Œǉ����A�������\�����ς��Ȃ��悤�ɂ��邱��
// MH322917(E) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
typedef struct {
	t_FltReq				req_tbl[FLT_REQTBL_MAX];
	uchar dummy[160]; 		//t_FltReq�����������z�u�������Ȃ��悤�ɃJ�o�[���邽��
	uchar					workbuf[FLT_WKBUF_SIZE];
	t_AppServ_ParamInfoTbl	param_tbl;
	t_FltParamForWrite		params;
	t_FltDataInfo			data_info_bk;
	BOOL					clearing;				/* TRUE=LOG�Ǘ����S�N���A�� */
}t_FLT_Ctrl_Backup;
/* �o�b�N�A�b�v����Ȃ��A�t���b�V������f�[�^ */
// MH322917(S) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
// ���ӎ����F��{�I�ɂ͈ȉ��\���̂̃����o��ύX���Ȃ�����
// �ύX���K�v�ȏꍇ�̓������z�u������邽�߃o�[�W�����A�b�v�Ń��O�����O�o�[�W�����uLOG_VERSION�v���A�b�v�f�[�g���A
// opetask()�̒���LOG_VERSION�ɂ������S���O�N���A������ǉ����邱��
// t_FltDataInfo:���O��FROM�Ǘ��̈�At_AppServ_ParamInfoTbl�F�p�����[�^FROM�������ݗp�̈�
// t_AppServ_AreaInfoTbl�F�o�b�N�A�b�vFROM�������ݗp�̈�
// MH322917(E) A.Iiizumi 2018/11/22 ���ӎ����̃R�����g�ǉ�
typedef struct {
	t_FltDataInfo			data_info;
	t_AppServ_ParamInfoTbl	param_tbl_wk;
	t_AppServ_AreaInfoTbl	area_tbl;
	t_AppServ_AreaInfoTbl	area_tbl_wk;
}t_FLT_Ctrl_NotBackup;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

ulong	FLT_init(uchar *param_ok, uchar *lockparam_ok, ulong *ver_flg, char clr);
ulong	FLT_WriteParam1(uchar flags);
ulong	FLT_WriteParam2(const char *param_image, size_t image_size, uchar flags);
ulong	FLT_WriteSaleLog(const char *records, short record_num, uchar flags);
ulong	FLT_WriteTotalLog(const char *records, short record_num, uchar flags);
ulong	FLT_WriteLogo(const char *image, uchar flags);
ulong	FLT_WriteHeader(const char *image, uchar flags);
ulong	FLT_WriteFooter(const char *image, uchar flags);
ulong	FLT_WriteAcceptFooter(const char *image, uchar flags);
ulong	FLT_WriteLockParam1(uchar flags);
ulong	FLT_WriteLockParam2(const char *param_image, uchar flags);
ulong	FLT_Backup(ulong ver_flg,uchar flags);
ulong	FLT_RestoreParam(void);
ulong	FLT_RestoreLockParam(void);
ulong	FLT_GetLogRecordNum(short id, long *record_num);
ulong	FLT_WriteLog(short id, const char *records, short record_num, uchar flags, short kind);
ulong	FLT_TargetLogRead(short id, ushort num, char *buf);
ulong	FLT_GetSaleLogRecordNum(long *record_num);
ulong	FLT_FindFirstSaleLog(t_FltLogHandle *handle, char *buf);
ulong	FLT_FindNextSaleLog(t_FltLogHandle *handle, char *buf);
ulong	FLT_FindNextSaleLog_OnlyDate(t_FltLogHandle *handle, char *buf);
ulong	FLT_TargetSaleLogRead( ushort num, char *buf );
ulong	FLT_GetTotalLogRecordNum(long *record_num);
ulong	FLT_FindFirstTotalLog(t_FltLogHandle *handle, char *buf);
ulong	FLT_FindNextTotalLog(t_FltLogHandle *handle, char *buf);
ulong	FLT_FindNextTotalLog_OnlyDate(t_FltLogHandle *handle, char *buf);
ulong	FLT_TargetTotalLogRead( ushort num, char *buf );
ulong	FLT_ReadLogo(char *buf);
ulong	FLT_ReadLogoPart(char *buf, ushort TopOffset, ushort ReadSize);
ulong	FLT_ReadHeader(char *buf);
ulong	FLT_ReadFooter(char *buf);
ulong	FLT_ReadAcceptFooter(char *buf);
ulong	FLT_Restore(uchar flags);
uchar	FLT_ChkWriteCmp(ulong req_kind, ulong *endsts);
void	FLT_CloseLogHandle(t_FltLogHandle *handle);
const t_AppServ_ParamInfoTbl	*FLT_GetParamTable(void);
void	FLT_LogErase2( unsigned char Kind );
ulong	FLT_Restore_FLAPDT( void );
uchar	FLT_GetVerupFlag(ulong *ver_flg);
uchar	FLT_Comp_ComParaSize_FRom_to_SRam( void );
ulong	FLT_ReadSyomei(char *buf);
ulong	FLT_ReadKamei(char *buf);
ulong	FLT_WriteSyomei(const char *image, uchar flags);
ulong	FLT_WriteKamei(const char *image, uchar flags);
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
ulong	FLT_ReadEdyAt(char *buf, uchar flags);
ulong	FLT_WriteEdyAt(const char *image, uchar flags);
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#endif
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
ulong	FLT_FindFirstEpayLog(t_FltLogHandle *handle, char *buf);
ulong	FLT_FindNextEpayLog(t_FltLogHandle *handle, char *buf);
ulong	FLT_FindFirstEpayKnd(t_FltLogHandle *handle, char *buf, ushort seek_cnt, ushort offset_num);
ulong	FLT_FindNextEpayKnd(t_FltLogHandle *handle, char *buf, ushort seek_cnt);
ulong	FLT_SeekSaleLog(t_FltLogHandle *handle, ushort num);
ulong	FLT_ReadUserDef(char *buf);
ulong	FLT_WriteUserDef(const char *image, uchar flags);
//-- for FTP
void	FLT_SetSum(uchar *buf, ulong size);
ulong	FLT_ReadEtc(char *buf);
ulong	FLT_WriteEtc(char *buf);
ulong	FLT_WriteImage(char *buf, ulong len, ulong flash, int erase);
ulong	FLT_DirectWrite(char *buf, ulong len, ulong flash);
ulong	FLT_WriteParam2(const char *param_image, size_t image_size, uchar flags);
ulong	FLT_WriteParam3(const char *param_image, size_t image_size, uchar flags);
void	FLT_ReadStbyParamTbl(t_AppServ_ParamInfoTbl *param_tbl_wk);
ulong	FLT_WriteCardData(char card_syu,  uchar flags);
ulong	FLT_ReadAzuFtr(char *buf);
ulong	FLT_WriteAzuFtr(const char *image, uchar flags);
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
ulong	FLT_ReadCreKbrFtr(char *buf);
ulong	FLT_WriteCreKbrFtr(const char *image, uchar flags);
ulong	FLT_ReadEpayKbrFtr(char *buf);
ulong	FLT_WriteEpayKbrFtr(const char *image, uchar flags);
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
ulong	FLT_ReadFutureFtr(char *buf);
ulong	FLT_WriteFutureFtr(const char *image, uchar flags);
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
void	FLT_CardDataSizeGet(char *, uchar);
ulong	FLT_ReadPointData(char *buf);
uchar 	FLT_Carddata_write_exec(char card_syu);
int		FLT_sw_erase_sector_exclusive(char *buf, ulong len, ulong flash, int erase);
ulong	FLT_ReadBRLOGSZ( ushort *buf );
ulong	FLT_WriteBRLOGSZ( char *buf, uchar flags );
void FLT_write_wave_swdata(unsigned short sw);
void FLT_write_parm_swdata(unsigned short sw);
unsigned short FLT_get_wave_swdata(void);
unsigned short FLT_get_parm_swdata(void);
void FLT_read_wave_sum_version(unsigned char sw,unsigned char *data);
void FLT_read_program_version(unsigned char *data);
ulong FLT_write_wave_sum_version(unsigned char sw,unsigned char *data);
ulong FLT_write_program_version(unsigned char *data);
void FLT_WriteLog_Pon(void);
BOOL FLT_Check_LogWriteReq(short kind);
ulong FLT_DirectEraseSector(ulong addr);
void FLT_req_tbl_clear(void);
void	FLT_ChkLog_all( void );
uchar FLT_read_log_version(void);
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
//void FLT_write_log_version(uchar log_ver);
void FLT_write_log_version(uchar log_ver,uchar chg_info_ver);
uchar FLT_read_chg_info_version(void);
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
void FLT_Force_ResetLogWriteReq(short kind);
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
uchar FLT_check_syukei_log_delete(void);
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
ulong	FLT_WriteEmgFooter(const char *image, uchar flags);
ulong	FLT_ReadEmgFooter(char *buf);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

#endif	/* ___FLA_DEFH___ */
