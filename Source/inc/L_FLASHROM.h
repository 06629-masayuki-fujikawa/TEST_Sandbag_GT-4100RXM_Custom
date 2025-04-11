#ifndef	_L_FLASHROM_H_
#define	_L_FLASHROM_H_
/*[]----------------------------------------------------------------------[]*/
/*| フラッシュ制御部                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| FROM1 4MBIT,FROM2 4MBITを想定している                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : H.Sada                                                   |*/
/*| Date        : 2001-11-02                                               |*/
/*| Update      : 2002-04-18 I.Naimen Define ALL ADDRESS                   |*/
/*|             : 2004-07-10 T.Nakayama                                    |*/
/*|             : 2005-02-10 R.Hara ﾌﾗｯｼｭの数,ｱﾄﾞﾚｽ変更                    |*/
/*|             : 2007-07-13 A.Iiizumi ﾌﾗｯｼｭの数,ｱﾄﾞﾚｽ変更                 |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/


#define FROM0_ADDRESS			0x00000000L	/*ｴﾘｱ0 							*/
#define FROM1_ADDRESS			0x00E00000L	/*ｴﾘｱ2 							*/
#define FROM2_ADDRESS			0x00400000L	/*ｴﾘｱ2 							*/

#define SRAM1_ADDRESS			0x00200000L	/*ｴﾘｱ0 							*/

#define MEMORY_SIZE_32MBIT		0x200000L   /* 32MBit FLASHのサイズ			*/
#define MEMORY_SIZE_8MBIT		0x050000L  	/* 8MBit FLASHのサイズ			*/
#define MEMORY_SIZE_4MBIT		0x025000L   /* 4MBit FLASHのサイズ			*/
#define MEMORY_SIZE_2MBIT		0x012500L   /* 2MBit FLASHのサイズ			*/

#define MEMORY_ADR_MASK			(MEMORY_SIZE_32MBIT-1L)	/*アドレス演算用マスク*/

// MBM29F400TC(4M)-90 / AMIC A29400TV-70

#define FLASH4M_SA0		0x00000L	// SA0  64K
#define FLASH4M_SA1		0x10000L	// SA1  64K
#define FLASH4M_SA2		0x20000L	// SA2  64K
#define FLASH4M_SA3		0x30000L	// SA3  64K
#define FLASH4M_SA4		0x40000L	// SA4  64K
#define FLASH4M_SA5		0x50000L	// SA5  64K
#define FLASH4M_SA6		0x60000L	// SA6  64K
#define FLASH4M_SA7		0x70000L	// SA7  32K
#define FLASH4M_SA8		0x78000L	// SA8   8K
#define FLASH4M_SA9		0x7A000L	// SA9   8K
#define FLASH4M_SA10	0x7C000L	// SA10 16K

// MBM29F800TC(8M)-90

#define FLASH8M_SA0		0x00000L	// SA0  64K
#define FLASH8M_SA1		0x10000L	// SA1  64K
#define FLASH8M_SA2		0x20000L	// SA2  64K
#define FLASH8M_SA3		0x30000L	// SA3  64K
#define FLASH8M_SA4		0x40000L	// SA4  64K
#define FLASH8M_SA5		0x50000L	// SA5  64K
#define FLASH8M_SA6		0x60000L	// SA6  64K
#define FLASH8M_SA7		0x70000L	// SA7  64K
#define FLASH8M_SA8		0x80000L	// SA8  64K
#define FLASH8M_SA9		0x90000L	// SA9  64K
#define FLASH8M_SA10	0xA0000L	// SA10 64K
#define FLASH8M_SA11	0xB0000L	// SA11 64K
#define FLASH8M_SA12	0xC0000L	// SA12 64K
#define FLASH8M_SA13	0xD0000L	// SA13 64K
#define FLASH8M_SA14	0xE0000L	// SA14 64K
#define FLASH8M_SA15	0xF0000L	// SA15 32K
#define FLASH8M_SA16	0xF8000L	// SA16  8K
#define FLASH8M_SA17	0xFA000L	// SA17  8K
#define FLASH8M_SA18	0xFC000L	// SA18 16K

// S29JL032H70TFI020(32M)
#define FLASH32M_SA0	0x000000L	// SA0   8K
#define FLASH32M_SA1	0x002000L	// SA1   8K
#define FLASH32M_SA2	0x004000L	// SA2   8K
#define FLASH32M_SA3	0x006000L	// SA3   8K
#define FLASH32M_SA4	0x008000L	// SA4   8K
#define FLASH32M_SA5	0x00A000L	// SA5   8K
#define FLASH32M_SA6	0x00C000L	// SA6   8K
#define FLASH32M_SA7	0x00E000L	// SA7   8K
#define FLASH32M_SA8	0x010000L	// SA8  64K
#define FLASH32M_SA9	0x020000L	// SA9  64K
#define FLASH32M_SA10	0x030000L	// SA10 64K
#define FLASH32M_SA11	0x040000L	// SA11 64K
#define FLASH32M_SA12	0x050000L	// SA12 64K
#define FLASH32M_SA13	0x060000L	// SA13 64K
#define FLASH32M_SA14	0x070000L	// SA14 64K
#define FLASH32M_SA15	0x080000L	// SA15 64K
#define FLASH32M_SA16	0x090000L	// SA16 64K
#define FLASH32M_SA17	0x0A0000L	// SA17 64K
#define FLASH32M_SA18	0x0B0000L	// SA18 64K
#define FLASH32M_SA19	0x0C0000L	// SA19 64K
#define FLASH32M_SA20	0x0D0000L	// SA20 64K
#define FLASH32M_SA21	0x0E0000L	// SA21 64K
#define FLASH32M_SA22	0x0F0000L	// SA22 64K
#define FLASH32M_SA23	0x100000L	// SA23 64K
#define FLASH32M_SA24	0x110000L	// SA24 64K
#define FLASH32M_SA25	0x120000L	// SA25 64K
#define FLASH32M_SA26	0x130000L	// SA26 64K
#define FLASH32M_SA27	0x140000L	// SA27 64K
#define FLASH32M_SA28	0x150000L	// SA28 64K
#define FLASH32M_SA29	0x160000L	// SA29 64K
#define FLASH32M_SA30	0x170000L	// SA30 64K
#define FLASH32M_SA31	0x180000L	// SA31 64K
#define FLASH32M_SA32	0x190000L	// SA32 64K
#define FLASH32M_SA33	0x1A0000L	// SA33 64K
#define FLASH32M_SA34	0x1B0000L	// SA34 64K
#define FLASH32M_SA35	0x1C0000L	// SA35 64K
#define FLASH32M_SA36	0x1D0000L	// SA36 64K
#define FLASH32M_SA37	0x1E0000L	// SA37 64K
#define FLASH32M_SA38	0x1F0000L	// SA38 64K
#define FLASH32M_SA39	0x200000L	// SA39 64K
#define FLASH32M_SA40	0x210000L	// SA40 64K
#define FLASH32M_SA41	0x220000L	// SA41 64K
#define FLASH32M_SA42	0x230000L	// SA42 64K
#define FLASH32M_SA43	0x240000L	// SA43 64K
#define FLASH32M_SA44	0x250000L	// SA44 64K
#define FLASH32M_SA45	0x260000L	// SA45 64K
#define FLASH32M_SA46	0x270000L	// SA46 64K
#define FLASH32M_SA47	0x280000L	// SA47 64K
#define FLASH32M_SA48	0x290000L	// SA48 64K
#define FLASH32M_SA49	0x2A0000L	// SA49 64K
#define FLASH32M_SA50	0x2B0000L	// SA50 64K
#define FLASH32M_SA51	0x2C0000L	// SA51 64K
#define FLASH32M_SA52	0x2D0000L	// SA52 64K
#define FLASH32M_SA53	0x2E0000L	// SA53 64K
#define FLASH32M_SA54	0x2F0000L	// SA54 64K
#define FLASH32M_SA55	0x300000L	// SA55 64K
#define FLASH32M_SA56	0x310000L	// SA56 64K
#define FLASH32M_SA57	0x320000L	// SA57 64K
#define FLASH32M_SA58	0x330000L	// SA58 64K
#define FLASH32M_SA59	0x340000L	// SA59 64K
#define FLASH32M_SA60	0x350000L	// SA60 64K
#define FLASH32M_SA61	0x360000L	// SA61 64K
#define FLASH32M_SA62	0x370000L	// SA62 64K
#define FLASH32M_SA63	0x380000L	// SA63 64K
#define FLASH32M_SA64	0x390000L	// SA64 64K
#define FLASH32M_SA65	0x3A0000L	// SA65 64K
#define FLASH32M_SA66	0x3B0000L	// SA66 64K
#define FLASH32M_SA67	0x3C0000L	// SA67 64K
#define FLASH32M_SA68	0x3D0000L	// SA68 64K
#define FLASH32M_SA69	0x3E0000L	// SA69 64K
#define FLASH32M_SA70	0x3F0000L	// SA70 64K

// EN25Q128(16Mbytes)
#define FROM1_SA0	0x000000L	// 3280K
#define FROM1_SA1	0x346000L	// 768K
#define FROM1_SA2	0x406000L	// 248K
#define FROM1_SA3	0x444000L	// 248K
#define FROM1_SA4	0x482000L	// 32K
#define FROM1_SA5	0x48A000L	// 32K
#define FROM1_SA6	0x492000L	// 16K
#define FROM1_SA7	0x496000L	// 16K
#define FROM1_SA8	0x49A000L	// 16K
#define FROM1_SA9	0x49E000L	// 16K
#define FROM1_SA10	0x4A2000L	// 16K
#define FROM1_SA11	0x4A6000L	// 32K
#define FROM1_SA12	0x4AE000L	// 240K
#define FROM1_SA13	0x4FE000L	// 16K
#define FROM1_SA14	0x502000L	// 16K
#define FROM1_SA15	0x506000L	// 16K
#define FROM1_SA16	0x50A000L	// 24K
#define FROM1_SA17	0x510000L	// 16K
#define FROM1_SA18	0x514000L	// 32K
#define FROM1_SA19	0x51C000L	// 40K
#define FROM1_SA20	0x600000L	// 64K
#define FROM1_SA21	0x610000L	// 64K
#define FROM1_SA22	0x620000L	// 64K
#define FROM1_SA23	0x630000L	// 16K
#define FROM1_SA24	0x634000L	// 8K
#define FROM1_SA25	0x636000L	// 4K
#define FROM1_SA26	0x637000L	// 804K
#define FROM1_SA27	0x900000L	// 2M
#define FROM1_SA28	0xb80000L	// 2M
#define FROM1_SA29	0xe00000L	// 2M
// MH322914 (s) kasiyama 2016/07/13 FlashROMメモリアドレスシンボル間違い修正[共通バグNo.1271]
//#define FROM1_SA30	0x4c2000L	// 248K
#define FROM1_SA30	0x530000L	// 832K
// MH322914 (s) kasiyama 2016/07/13 FlashROMメモリアドレスシンボル間違い修正[共通バグNo.1271]
#define FROM1_SA31	0x700000L	// 3M
#define FROM1_SA32	0x8fb000L	// 4K	// 音声データ運用面切替
#define FROM1_SA33	0x8fc000L	// 4K	// 共通パラメータ運用面切替
#define FROM1_SA34	0x8fd000L	// 4K	// 音声データ面１ヘッダ情報
#define FROM1_SA35	0x8fe000L	// 4K	// 音声データ面２ヘッダ情報
#define FROM1_SA36	0x8ff000L	// 4K	// プログラムヘッダ情報
#define FROM1_SA37	0x6ff000L	// 4K	// FROM内のログバージョン
#define FROM1_SA38	0x526000L	// 16K
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
#define FROM1_SA40	0x534000L	// 48K	// 長期駐車ログ(ParkingWeb用)
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)

#define	FLASH_SECTOR_ERASE_MAX	1500000L	// 3s    = ( S29JL032H70TFI020(32M) 2s    ) * 1.5(Margine)
#define	FLASH_WORD_WRITE_MAX	75L			// 150us = ( S29JL032H70TFI020(32M) 100us ) * 1.5(Margine)


#define FROM0_STADRESS	(unsigned char *)FROM0_ADDRESS

#define RAM_STADRESS	(unsigned char *)SRAM1_ADDRESS
#define RAM_EDADDRESS	(unsigned char *)(SRAM1_ADDRESS + MEMORY_SIZE_4MBIT -1)
extern const unsigned long RAM_ADDRESS[8];

#define FROM1_STADRESS	(unsigned char *)FROM1_ADDRESS


#define FROM2_STADRESS	(unsigned char *)(FROM2_ADDRESS)
#define FROM2_EDADDRESS	(unsigned char *)(FROM2_ADDRESS+MEMORY_SIZE_32MBIT-1)

#define		FROM1		0		/* 0:FROM1									*/
#define		FROM2		1		/* 1:FROM2									*/
extern const unsigned long FROM_BASE_ADDRESS[2];


#endif		//_L_FLASHROM_H_
