#ifndef	___FLA_DEFH___
#define	___FLA_DEFH___
/*[]----------------------------------------------------------------------[]
 *|	filename: fla_def.h
 *[]----------------------------------------------------------------------[]
 *| summary	: フラッシュ2共通定義
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

/* 外部FLASH2メモリマップバージョン */
#define	FLASH2_MAP_VERSION_0100		0x00010000		/* ver.1-0 */
#define	FLASH2_MAP_VERSION			0x00010000		/* current version */
// MAPバージョンは、バックアップデータ情報テーブルの先頭データのアドレス部に
// 書き込まれるが、バージョン0のプログラム(MAPバージョン自体が存在しない)では
// ここに有効データ(最初のバックアップデータのバックアップ領域先頭からのオフセットアドレス)
// が書き込まれている。
// そのため、バージョン1を示す実値を、バックアップ情報テーブルサイズ(現状218byte使用)より
// 大幅に大きい0x00010000(＞バックアップデータ情報テーブルサイズ)に設定する。
// 0x00010000より小さい値の場合、バージョン0のプログラムが書いたFLASHとして判断する。

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
#define LOG_STRAGEN	21		// ログ格納項目数
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
#define	FLT_EDYATCMD_SIZE			120		// 20文字*6行

#define	FLT_CREDIT_SIZE				FLT_PRINTHEADER_SIZE
#define	FLT_ACCEPTFOOTER_SIZE		FLT_PRINTHEADER_SIZE

#define	FLT_TCARDFTR_SIZE			360		// 36*T_FOOTER_GYO_MAX(10)
#define	FLT_BR_LOG_SIZE				sizeof(t_FltParam_BRLOGSZ)*eLOG_MAX

#define	FLT_TCARD_DATA_SIZE			1142
#define	FLT_HCARD_DATA_SIZE			1122
#define	FLT_OFS_HCARD				4096			// オフセットは1セクタ
#define	FLT_OFS_VERTION(a)			(ulong)(((a+1)*4096)-3)// セクタの最後から3バイト前をバージョン、2バイトをデータサイズとする
#define	FLT_T_POINT_DATA_SIZE		20

#define	FLT_AZUFTR_SIZE			144		// 36*4

// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
#define	FLT_CREKABARAI_SIZE		144		// 36*4
#define	FLT_EPAYKABARAI_SIZE	144		// 36*4
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
#define	FLT_FUTURE_SIZE			144		// 36*4
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
#define	FLT_PRINTEMGFOOTER_SIZE		FLT_PRINTHEADER_SIZE
// MH810105(E) MH364301 QRコード決済対応

#define	FLT_USERDEF_SIZE			2048
#define	FLT_CHKRESULT_SIZE			sizeof(t_Chk_res_ftp)

/* フラッシュ内オフセット */
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
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
		char	crekbrftr[FLT_HEADER_SIZE];				// クレジット振替過払いフッター
		char		dcrekbrftr[FLT_CREKABARAI_SIZE];	// クレジット振替過払いフッター
		char	epaykbrftr[FLT_HEADER_SIZE];			// 電子マネー振替過払いフッター
		char		depaykbrftr[FLT_EPAYKABARAI_SIZE];	// 電子マネー振替過払いフッター
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
		char	futureftr[FLT_HEADER_SIZE];			// 後日支払額フッター
		char		dfutureftr[FLT_FUTURE_SIZE];	// 後日支払額フッター
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
		char	emgfooter[FLT_HEADER_SIZE];
		char		demgfooter[FLT_PRINTEMGFOOTER_SIZE];
// MH810105(E) MH364301 QRコード決済対応

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

/* LOG検索ハンドル */
typedef struct {
	ushort	kind;
	ushort	sectno;
	ushort	recordno;
}t_FltLogHandle;

/*-------------------------------------[ PRIVATE ]-----*/

/* 要求種別 */
enum {
	FLT_REQTBL_LOG = 0,			// = eLOG_PAYMENTとすること
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
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
	FLT_REQTBL_CREKBRFTR,
	FLT_REQTBL_EPAYKBRFTR,
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
	FLT_REQTBL_FUTUREFTR,
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
	FLT_REQTBL_EMGFOOTER,
// MH810105(E) MH364301 QRコード決済対応
	FLT_REQTBL_BRLOGSZ,

	FLT_REQTBL_MAX
};

/*----------↓ メモリマップ ↓----------*/
#define SPI_FROM1_BLOCK_SIZE	0x10000L // 64K
#define SPI_FROM1_DATA_ENDADRESS	0x6FFFFFL // ログ領域、パラメータ、バックアップエリアまでのアドレス


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
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//#define FLT_LOG17_SECTOR	FROM1_SA17
//#define FLT_LOG17_SECTORMAX	4
#define FLT_LOG17_SECTOR	FROM1_SA40
#define FLT_LOG17_SECTORMAX	12
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
#define FLT_LOG18_SECTOR	FROM1_SA18
#define FLT_LOG18_SECTORMAX	8
#define FLT_LOG19_SECTOR	FROM1_SA19
#define FLT_LOG19_SECTORMAX	5
#define FLT_LOG20_SECTOR	FROM1_SA38
#define FLT_LOG20_SECTORMAX	4

// 共通パラメータ
#define	FLT_PARAM_SECTOR_1		FROM1_SA20
#define	FLT_PARAM_SECTOR_2		FROM1_SA21
#define	FLT_PARAM_SECTORMAX		16
#define	FLT_PARAM_SECT_SIZE		0x1000

// プリンタロゴ、ヘッダ、フッタ
#define	FLT_PRINTDATA_SECTOR	FROM1_SA22
#define	FLT_PRINTDATA_SECTORMAX	16
#define	FLT_PRINTDATA_SECT_SIZE	0x1000

// 車室パラメータ
#define	FLT_LOCKPARAM_SECTOR	FROM1_SA23
#define	FLT_LOCKPARAM_SECTORMAX	4
#define	FLT_LOCKPARAM_SECT_SIZE	0x1000

// Tカード対応・法人カード
#define	FLT_CARDDATA_SECTOR		FROM1_SA24
#define	FLT_CARDDATA_SECTORMAX	2
#define	FLT_CARDDATA_SECT_SIZE	0x1000

#define	FLT_BRLOGSZ_SECTOR		FROM1_SA25
#define	FLT_BRLOGSZ_SECTORMAX	1
#define	FLT_BRLOGSZ_SECT_SIZE	0x1000

// バックアップ
#define	FLT_BACKUP_SECTOR0		FROM1_SA26
#define	FLT_BACKUP_SECTORMAX	200 //バックアップ領域の最終セクタはログバージョンよりエリア対象外とする
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
// MH321800(S) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
//#define	FLT_BACKUP_SECTOR20_COUNT	1
#define	FLT_BACKUP_SECTOR20_COUNT	5
// MH321800(E) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)

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

// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
#define	FLT_BACKUP_SECTOR31			FLT_BACKUP_SECTOR30+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR30_COUNT)
#define	FLT_BACKUP_SECTOR31_COUNT	1
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応

// MH321800(S) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)
#define	FLT_BACKUP_SECTOR32			FLT_BACKUP_SECTOR31+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR31_COUNT)
#define	FLT_BACKUP_SECTOR32_COUNT	1

#define	FLT_BACKUP_SECTOR33			FLT_BACKUP_SECTOR32+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR32_COUNT)
#define	FLT_BACKUP_SECTOR33_COUNT	1
// MH321800(E) hosoda ICクレジット対応 (集計ログ/Edyの上位端末ID/アラーム取引ログ)

// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
#define	FLT_BACKUP_SECTOR34			FLT_BACKUP_SECTOR33+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR33_COUNT)
#define	FLT_BACKUP_SECTOR34_COUNT	1

#define	FLT_BACKUP_SECTOR35			FLT_BACKUP_SECTOR34+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR34_COUNT)
#define	FLT_BACKUP_SECTOR35_COUNT	1
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
#define	FLT_BACKUP_SECTOR36			FLT_BACKUP_SECTOR35+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR35_COUNT)
// MH810103(s) 電子マネー対応 #5402 システムメンテナンスの全データバックアップでベリファイエラーが発生する
//#define	FLT_BACKUP_SECTOR36_COUNT	2
// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6584 精算データがあるとバックアップができない
// #define	FLT_BACKUP_SECTOR36_COUNT	6
#define	FLT_BACKUP_SECTOR36_COUNT	7
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6584 精算データがあるとバックアップができない
// MH810103(s) 電子マネー対応 #5402 システムメンテナンスの全データバックアップでベリファイエラーが発生する
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
#define	FLT_BACKUP_SECTOR37			FLT_BACKUP_SECTOR36+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR36_COUNT)
#define	FLT_BACKUP_SECTOR37_COUNT	1
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする

// MH810103 GG119202(S) 個別パラメータがリストアされない
#define	FLT_BACKUP_SECTOR38			FLT_BACKUP_SECTOR37+(FLT_BACKUP_SECT_SIZE*FLT_BACKUP_SECTOR37_COUNT)
#define	FLT_BACKUP_SECTOR38_COUNT	5
// MH810103 GG119202(E) 個別パラメータがリストアされない

// FROM内のログバージョン
#define	FLT_LOG_VERSION_SECTOR		FROM1_SA37
#define	FLT_LOG_VERSION_SECTORMAX	1
#define	FLT_LOG_VERSION_SECT_SIZE	0x1000
#define	FLT_LOG_VERSION_DATA_SIZE	1

// 音声データ
#define	FLT_SOUND0_SECTOR		FROM1_SA27
#define	FLT_SOUND0_SECTORMAX	640
#define	FLT_SOUND0_SECT_SIZE	0x1000

// 音声データ（ダウンロード用）
#define	FLT_SOUND1_SECTOR		FROM1_SA28
#define	FLT_SOUND1_SECTORMAX	640
#define	FLT_SOUND1_SECT_SIZE	0x1000

// プログラム
#define	FLT_PROGRAM_SECTOR		FROM1_SA29
#define	FLT_PROGRAM_SECTORMAX	512
#define	FLT_PROGRAM_SECT_SIZE	0x1000

// 予備
#define	FLT_RESERVE1_SECTOR		FROM1_SA30
// MH322914 (s) kasiyama 2016/07/13 FlashROMメモリアドレスシンボル間違い修正[共通バグNo.1271]
//#define	FLT_RESERVE1_SECTORMAX	62
#define	FLT_RESERVE1_SECTORMAX	208
// MH322914 (e) kasiyama 2016/07/13 FlashROMメモリアドレスシンボル間違い修正[共通バグNo.1271]
#define	FLT_RESERVE1_SECT_SIZE	0x1000

#define	FLT_RESERVE2_SECTOR		FROM1_SA31
#define	FLT_RESERVE2_SECTORMAX	768
#define	FLT_RESERVE2_SECT_SIZE	0x1000

#define	FLT_SUM_VER_DATA_SIZE	18

// 音声データ運用面情報
#define	FLT_WAVE_SWDATA_SECTOR			FROM1_SA32
#define	FLT_WAVE_SWDATA_SECTORMAX		1
#define	FLT_WAVE_SWDATA_SECT_SIZE		0x1000
#define	FLT_WAVE_SWDATA_DATA_SIZE		2

// 共通パラメータ運用面情報
#define	FLT_PARM_SWDATA_SECTOR			FROM1_SA33
#define	FLT_PARM_SWDATA_SECTORMAX		1
#define	FLT_PARM_SWDATA_SECT_SIZE		0x1000
#define	FLT_PARM_SWDATA_DATA_SIZE		2

// 音声データ運用面1データチェックサム、バージョン
#define	FLT_SOUND0_SUM_VER_SECTOR		FROM1_SA34
#define	FLT_SOUND0_SUM_VER_SECTORMAX	1
#define	FLT_SOUND0_SUM_VER_SECT_SIZE	0x1000

// 音声データ運用面2データチェックサム、バージョン
#define	FLT_SOUND1_SUM_VER_SECTOR		FROM1_SA35
#define	FLT_SOUND1_SUM_VER_SECTORMAX	1
#define	FLT_SOUND1_SUM_VER_SECT_SIZE	0x1000

// プログラムデータチェックサム、バージョン
#define	FLT_PROGRAM_SUM_VER_SECTOR		FROM1_SA36
#define	FLT_PROGRAM_SUM_VER_SECTORMAX	1
#define	FLT_PROGRAM_SUM_VER_SECT_SIZE	0x1000
/*----------↑ メモリマップ ↑----------*/

#define	GET_MAPP		(PRG_HDR*)(FROM1_SA29)
#define	GET_VAPP(sw)	(PRG_HDR*)(sw==0?FROM1_SA27:FROM1_SA28)
#define	GET_MAIN_H		(PRG_HDR*)(FROM1_SA36)
#define	GET_VOICE_H(sw)	(PRG_HDR*)(sw==0?FROM1_SA34:FROM1_SA35)

/* ワークバッファサイズ */
#define	FLT_WKBUF_SIZE			65536	/* 64*1024 */

/* 要求受付状態 */
typedef struct {
	uchar	req;				/* 要求受付状態 */
	ulong	result;				/* 実行結果 */
}t_FltReq;

/* 書き込みに使用するパラメータ(API⇒フラッシュタスクへの引き渡し用) */
/* for FLT_REQTBL_PARAM2 */
typedef struct {
	const char	*param_image;
	size_t		image_size;
}t_FltParam_Param2;
/* for FLT_REQTBL_SALE_LOG and FLT_REQTBL_TOTAL_LOG */
typedef struct {
	const char	*records;
	short		record_num;
	short		sramlog_kind;// SRAM上のLOG(4Kbyte2面のどちらの情報を書きたいかを示すデータ)
	ushort		f_recover;// 復電時の状態を示すフラグ
	short		stat_Fcount;// 件数
	ushort		stat_sectnum;// セクタ数
	ushort		get_sector;// GetWriteLogSectr()で取得する値
	ushort		get_seqno;// GetWriteLogSectr()で取得する値
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
// 仕様変更(S) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
/* for FLT_REQTBL_CREKBRFTR */
typedef struct {
	const char	*image;
}t_FltParam_CreKbrFtr;

/* for FLT_REQTBL_EPAYKBRFTR */
typedef struct {
	const char	*image;
}t_FltParam_EpayKbrFtr;
// 仕様変更(E) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
/* for FLT_REQTBL_FUTUREFTR */
typedef struct {
	const char	*image;
}t_FltParam_FutureFtr;
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
/* for FLT_REQTBL_EMGFOOTER */
typedef struct {
	const char	*image;
}t_FltParam_EmgFooter;
// MH810105(E) MH364301 QRコード決済対応
/* for FLT_REQTBL_BRLOGSZ */
typedef struct {
	ushort	log_size;
}t_FltParam_BRLOGSZ;

typedef	t_FltParam_Header	t_FltParam_UserDef;			// ヘッダと同型

typedef	t_FltParam_Header	t_FltParam_Etc;				// ヘッダと同型
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
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
	t_FltParam_CreKbrFtr	crekbrftr;
	t_FltParam_EpayKbrFtr	epaykbrftr;
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
	t_FltParam_FutureFtr	futureftr;
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
	t_FltParam_BRLOGSZ	brlogsz[eLOG_MAX];
// MH810105(S) MH364301 QRコード決済対応
	t_FltParam_EmgFooter	emgfooter;
// MH810105(E) MH364301 QRコード決済対応
}t_FltParamForWrite;

/* フラッシュ上データ制御情報 */
/* セクタ毎のLOG情報 */
typedef struct {
	ulong		address;		/* 該当セクタ先頭アドレス */
	ushort		seqno;			/* 追い番 */
	ushort		record_num;		/* 格納レコード件数 */
}t_FltLogInfo;
/* フラッシュ上のLOG管理情報 */
typedef struct {
	ushort			secttop;		/* 先頭セクタ */
	ushort			sectnum;		/* 有効データがあるセクタ数 */
	ushort			sectmax;		/* 最大セクタ数 */
	ushort			record_size;	/* レコード1件のサイズ */
	ushort			recordnum_max;	/* 1セクタあたりの最大レコード件数 */
	t_FltLogInfo	sectinfo[FLT_LOG0_SECTORMAX];	/* 処理を共通化のため、全てこのサイズ */
}t_FltLogMgr;

typedef struct {
	t_FltLogMgr		log_mgr[LOG_STRAGEN];	// LOG領域管理情報
}t_FltDataInfo;

/* バックアップされる、フラッシュ制御データ */
// MH322917(S) A.Iiizumi 2018/11/22 注意事項のコメント追加
// 注意事項：t_FLT_Ctrl_Backup構造体のt_FltParamForWrite、t_FltDataInfoはFROM書き込み時の停電リカバリ用のエリア
// のためメモリ配置を変更することを禁止する。(ログ書き込み中にバージョンアップを伴う停電するとデータが壊れるため)
// t_AppServ_ParamInfoTblに該当する内容に追加が必要となる
// 場合は構造体の最後尾に新規メンバで追加し、メモリ構造が変わらないようにすること
// MH322917(E) A.Iiizumi 2018/11/22 注意事項のコメント追加
typedef struct {
	t_FltReq				req_tbl[FLT_REQTBL_MAX];
	uchar dummy[160]; 		//t_FltReqが増えた時配置が換わらないようにカバーするため
	uchar					workbuf[FLT_WKBUF_SIZE];
	t_AppServ_ParamInfoTbl	param_tbl;
	t_FltParamForWrite		params;
	t_FltDataInfo			data_info_bk;
	BOOL					clearing;				/* TRUE=LOG管理情報全クリア中 */
}t_FLT_Ctrl_Backup;
/* バックアップされない、フラッシュ制御データ */
// MH322917(S) A.Iiizumi 2018/11/22 注意事項のコメント追加
// 注意事項：基本的には以下構造体のメンバを変更しないこと
// 変更が必要な場合はメモリ配置が換わるためバージョンアップでログをログバージョン「LOG_VERSION」をアップデートし、
// opetask()の中でLOG_VERSIONにあった全ログクリア処理を追加すること
// t_FltDataInfo:ログのFROM管理領域、t_AppServ_ParamInfoTbl：パラメータFROM書き込み用領域
// t_AppServ_AreaInfoTbl：バックアップFROM書き込み用領域
// MH322917(E) A.Iiizumi 2018/11/22 注意事項のコメント追加
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
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
ulong	FLT_ReadEdyAt(char *buf, uchar flags);
ulong	FLT_WriteEdyAt(const char *image, uchar flags);
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#endif
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
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
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
ulong	FLT_ReadCreKbrFtr(char *buf);
ulong	FLT_WriteCreKbrFtr(const char *image, uchar flags);
ulong	FLT_ReadEpayKbrFtr(char *buf);
ulong	FLT_WriteEpayKbrFtr(const char *image, uchar flags);
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
ulong	FLT_ReadFutureFtr(char *buf);
ulong	FLT_WriteFutureFtr(const char *image, uchar flags);
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
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
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
//void FLT_write_log_version(uchar log_ver);
void FLT_write_log_version(uchar log_ver,uchar chg_info_ver);
uchar FLT_read_chg_info_version(void);
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
void FLT_Force_ResetLogWriteReq(short kind);
// MH321800(S) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
uchar FLT_check_syukei_log_delete(void);
// MH321800(E) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
// MH810105(S) MH364301 QRコード決済対応
ulong	FLT_WriteEmgFooter(const char *image, uchar flags);
ulong	FLT_ReadEmgFooter(char *buf);
// MH810105(E) MH364301 QRコード決済対応

#endif	/* ___FLA_DEFH___ */
