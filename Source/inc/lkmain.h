/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾌﾟ(ﾛｯｸ装置)制御部																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      :																							   |*/
/*| Date        : 2005-02-01																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _LK_MAIN_H_
#define _LK_MAIN_H_
																				/*								*/
/*------------------*/															/*								*/
/*	ﾛｯｸ装置毎設定	*/															/*								*/
/*------------------*/															/*								*/
typedef struct {																/*								*/
	uchar		lok_syu;														/* ﾛｯｸ装置種別					*/
	uchar		ryo_syu;														/* 料金種別						*/
	uchar		area;															/* 区画(1～26)					*/
	ulong		posi;															/* 駐車位置№(1～9999)			*/
	uchar		if_oya;															// 接続ターミナルNo
	uchar		lok_no;															// 接続ターミナルNoに対するロック装置連番
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
}t_LockInfo;																	/*								*/
																				/*								*/
extern t_LockInfo	LockInfo[LOCK_MAX];											/* ﾛｯｸ装置情報ﾃｰﾌﾞﾙ				*/
extern t_LockInfo	bk_LockInfo[LOCK_MAX];										/* 故障車室用ﾛｯｸ装置情報ﾃｰﾌﾞﾙ	*/
																				/*								*/
																				/*								*/
/*----------------------*/														/*								*/
/*	ﾛｯｸ装置種別毎設定	*/														/*								*/
/*----------------------*/														/*								*/
typedef struct {																/*								*/
	uchar		in_tm;															/* 入庫車両検知ﾀｲﾏｰ				*/
	uchar		ot_tm;															/* 出庫車両検知ﾀｲﾏｰ				*/
	uchar		r_cnt;															/* ﾛｯｸ装置ﾘﾄﾗｲ回数				*/
	ushort		r_tim;															/* ﾛｯｸ装置ﾘﾄﾗｲ間隔				*/
	uchar		open_tm;														/* 開動作信号出力時間			*/
	uchar		clse_tm;														/* 閉動作信号出力時間			*/
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
}t_LockMaker;																	/*								*/
																				/*								*/
extern t_LockMaker	LockMaker[6];												/* ﾛｯｸ装置種別					*/
																				/* [0]：英田自転車				*/
																				/* [1]：英田バイク				*/
																				/* [2]：吉益自転車・バイク		*/
																				/* [3]：コムズ自転車・バイク	*/
																				/* [4]：HID自転車・バイク		*/
																				/* [5]：予備					*/
																				/* 								*/
/*----------------------*/														/*								*/
/*	車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ	*/														/*								*/
/*----------------------*/														/*								*/
struct	LOCKINFO_REC{															/* 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ値用構造体	*/
	short		adr;															/*								*/
	t_LockInfo	dat;															/*								*/
};																				/*								*/
																				/*								*/
extern	const struct	LOCKINFO_REC	lockinfo_rec1[];						/* 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ（駐車）		*/
extern	const struct	LOCKINFO_REC	lockinfo_rec2[];						/* 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ（駐輪）		*/
extern	const struct	LOCKINFO_REC	lockinfo_rec3[];						/* 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ（駐輪（内蔵））		*/

// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
extern	const struct	LOCKINFO_REC	lockinfo_recAP1[];						/* 自動連続精算用車室ﾊﾟﾗﾒｰﾀ（駐車）	*/
extern	const struct	LOCKINFO_REC	lockinfo_recAP2[];						/* 自動連続精算用車室ﾊﾟﾗﾒｰﾀ（駐輪）	*/
#endif
																				/* 								*/
																				/* 								*/
/*----------------------*/														/*								*/
/*	ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ	*/														/*								*/
/*----------------------*/														/*								*/
struct	LOCKMAKER_REC{															/* ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ値用構造体*/
	short		adr;															/*								*/
	t_LockMaker	dat;															/*								*/
};																				/*								*/
																				/*								*/
extern	const struct	LOCKMAKER_REC	lockmaker_rec[];						/* ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ			*/
																				/* 								*/
																				/* 								*/
#define	LOCKINFO_REC_MAX1	1													/* 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ数（駐車）	*/
#define	LOCKINFO_REC_MAX2	1													/* 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ数（駐輪）	*/
#define	LOCKMAKER_REC_MAX	6													/* ﾛｯｸ種別ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ数		*/
#define	LOCKINFO_REC_MAX3	20													/* 車室ﾊﾟﾗﾒｰﾀﾃﾞﾌｫﾙﾄ数（内蔵駐輪）	*/
																				/*------------------------------*/
#endif	// _LK_MAIN_H_
