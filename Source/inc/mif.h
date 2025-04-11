/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	MIFARE関連ﾍｯﾀﾞｰﾌｧｲﾙ																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      : A.Iiizumi																					   |*/
/*| Date        : 2011-12-20																				   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
#ifndef _MIF_H_
#define _MIF_H_
																					/*							*/
#define		MIF_SCAN_ID_CMD		0xA1												/*							*/
#define		MIF_SCAN_DT_CMD		0xA2												/*							*/
#define		MIF_CANSEL_CMD		0xA3												/*							*/
#define		MIF_DAT_WRT_CMD		0xA4												/*							*/
#define		MIF_DAT_RED_CMD		0xA5												/*							*/
#define		MIF_WRT_CMD			0xA6												/*							*/
#define		MIF_KEY_WRT_CMD		0xA7												/*							*/
#define		MIF_TEST_CMD		0xAE												/*							*/
#define		MIF_PART_CMD		0xAF												/*							*/
																					/*							*/
//----------------------------------------------------------------------------------------------------------------
// <<Mifareｶｰﾄﾞ読出し情報>>		精算開始前に読出たｶｰﾄﾞの情報						/*							*/
//----------------------------------------------------------------------------------------------------------------
extern	uchar	MIF_CARD_READ[4][3][16];											/* Mifareｶｰﾄﾞ読出情報		*/
																					/* 精算開始前に読出した情報	*/
//----------------------------------------------------------------------------------------------------------------
// <<Mifareｶｰﾄﾞ書込み情報>>		精算完了後に書き換えるきｶｰﾄﾞの情報					/*							*/
//								ｶｰﾄﾞの書換え失敗時のﾘﾄﾗｲ書込みで使用可				/*							*/
//----------------------------------------------------------------------------------------------------------------
extern	uchar	MIF_CARD_WRIT[4][3][16];											/* Mifareｶｰﾄﾞ書込情報		*/
																					/*							*/
//----------------------------------------------------------------------------------------------------------------
// <<Mifareｶｰﾄﾞ固有番号情報>>	読取ったｶｰﾄﾞの固有番号情報（今ｱｸｾｽ中のｶｰﾄﾞ）		/*							*/
//								書込みﾘﾄﾗｲで翳されたｶｰﾄﾞが前回と一致か確認			/*							*/
//----------------------------------------------------------------------------------------------------------------
extern	uchar	MIF_CARD_SID[4];													/* ｶｰﾄﾞのｼﾘｱﾙ番号			*/
																					/*							*/
//----------------------------------------------------------------------------------------------------------------
// <<Mifare受信ﾃﾞｰﾀｽﾃｰﾀｽ情報>>	各受信ﾃﾞｰﾀのｽﾃｰﾀｽをﾒｯｾｰｼﾞ受信時に展開するﾜｰｸﾊﾞｯﾌｧ	/*							*/
//----------------------------------------------------------------------------------------------------------------
typedef	struct {																	/*							*/
	uchar	sts1;																	/* 終了ｽﾃｰﾀｽ1				*/
	uchar	sts2;																	/* 終了ｽﾃｰﾀｽ2				*/
} t_MIF_ENDSTS;																		/*							*/
																					/*							*/
extern	t_MIF_ENDSTS		MIF_ENDSTS;												/*							*/
																					/*							*/
//----------------------------------------------------------------------------------------------------------------
//	Mifareﾃｽﾄ結果格納構造体 														/*							*/
//----------------------------------------------------------------------------------------------------------------
typedef	struct {																	/*							*/
	uchar	req;																	/* 0=要求なし				*/
																					/* 1=要求済み				*/
																					/* 2=ﾚｽﾎﾟﾝｽあり				*/
	uchar	tst;																	/* 0=通信ﾃｽﾄ				*/
																					/* 1=R/Wﾃｽﾄ					*/
																					/* 2=ｶｰﾄﾞ固有番号ﾘｰﾄﾞ		*/
																					/* 3=ﾊﾞｰｼﾞｮﾝﾁｪｯｸ			*/
	uchar	res;																	/* 0=OK/1=NG				*/
	uchar	len;																	/* ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ長			*/
	uchar	ver[20];																/* ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ				*/
} t_MIF_MENTBUF;																	/*							*/
																					/*							*/
extern	t_MIF_MENTBUF		MIF_MENTBUF;											/*							*/
																					/*							*/
//----------------------------------------------------------------------------------------------------------------
//	Mifareｶｰﾄﾞ構造体		 														/*							*/
//----------------------------------------------------------------------------------------------------------------
typedef struct {																	/*							*/
																					/*							*/
	uchar	syu;																	/* 割引種別					*/
	uchar	kub;																	/* 割引区分					*/
	ushort	card;																	/* 割引ｶｰﾄﾞ区分				*/
	uchar	mai;																	/* 割引枚数					*/
	ulong	kin;																	/* 割引金額/時間			*/
	uchar	cd_syu;																	/* 割引対応ｶｰﾄﾞ種別			*/
	uchar	cond;																	/* 割引状況					*/
	uchar	stat;																	/* 割引ｽﾃｰﾀｽ				*/
} t_MIF_WARI;																		/*							*/
																					/*							*/
typedef struct {																	/*							*/
																					/*							*/
	uchar				type;														/* ｶｰﾄﾞﾀｲﾌﾟ					*/
	uchar				user_no[10];												/* ﾕｰｻﾞｰ定義ｶｰﾄﾞ番号		*/
	uchar				pas_knd;													/* ｶｰﾄﾞ定期種別				*/
	uchar				mem_knd;													/* ｶｰﾄﾞ会員種別				*/
	ulong				pk_no;														/* 駐車場No					*/
	uchar				cal_syu;													/* 料金種別					*/
	uchar				io_stat;													/* ｶｰﾄﾞ入出庫ｽﾃｰﾀｽ			*/
	date_time_rec		ent_tm;														/* 入庫年月日時分			*/
	date_time_rec		pay_tm;														/* 精算年月日時分			*/
	date_time_rec		ext_tm;														/* 出庫年月日時分			*/
	uchar				cd_stat;													/* ｶｰﾄﾞｽﾃｰﾀｽ				*/
	date_time_rec		limit_s;													/* ｶｰﾄﾞ有効開始年月日		*/
	date_time_rec		limit_e;													/* ｶｰﾄﾞ有効終了年月日		*/
	date_time_rec		prp_tm;														/* 処理年月日時分			*/
	uchar				prp_kin;													/* ﾌﾟﾘﾍﾟｲﾄﾞ販売金額			*/
	ulong				prp_zan;													/* ﾌﾟﾘﾍﾟｲﾄﾞ残額				*/
	t_MIF_WARI			wari1;														/* 割引1					*/
	t_MIF_WARI			wari2;														/* 割引2					*/
	t_MIF_WARI			wari3;														/* 割引3					*/
	ushort				pas_id;														/* 定期ID					*/
	uchar				kukaku;														/* 区画						*/
	ushort				pk_pos;														/* 駐車位置No.				*/
	date_time_rec		kosin_ymd;													/* 更新年月日時分			*/
	uchar				rack_sts;													/* ﾗｯｸ情報					*/
	ushort				point;														/* ﾎﾟｲﾝﾄ					*/
	uchar				crc[2];														/* crc([0]下位,[1]上位		*/
																					/*							*/
} t_MIF_CARDFORM;																	/*							*/
																					/*							*/
extern	t_MIF_CARDFORM		MifCard;												/* Miffareｶｰﾄﾞ構造体		*/
extern	t_MIF_CARDFORM		MifCard_bak;											/* 券データチェック画面参照用バックアップ	*/
																					/*							*/
typedef struct {																	/*							*/
	uchar	Sid[4];																	/* ｶｰﾄﾞのｼﾘｱﾙ番号			*/
	t_MIF_CARDFORM	Data;															/* ｶｰﾄﾞ内のﾃﾞｰﾀ				*/
} t_MIF_CardData;
extern	t_MIF_CardData	MIF_LastReadCardData;										/* 最後に受け付けたｶｰﾄﾞ内容	*/

#define	MIF_WRITE_NG_BUF_COUNT	3
typedef	struct {
	/* 実ﾃﾞｰﾀ */
	ushort			count;															/* 登録件数					*/
	t_MIF_CardData	data[MIF_WRITE_NG_BUF_COUNT];									/* ﾃﾞｰﾀ						*/
	/* 登録/削除用 作業ｴﾘｱ */
	ushort			f_Phase;														/* 0=作業なし				*/
																					/* 0x55=ﾜｰｸｴﾘｱから実ﾃﾞｰﾀへcopy中 */
	ushort			wk_count;														/* 登録件数					*/
	t_MIF_CardData	wk_data[MIF_WRITE_NG_BUF_COUNT];								/* ﾃﾞｰﾀ						*/
} t_MIF_WriteNgCard_ReadData;
	/* 本ﾃﾞｰﾀは停電保障ｴﾘｱであり、ﾃﾞｰﾀは前詰めに登録する。*/
	/* 登録時点の古い順に並ぶ事となり、Full時に登録する場合は最古ﾃﾞｰﾀを削除して新ﾃﾞｰﾀを末尾に登録する */
extern	t_MIF_WriteNgCard_ReadData	MIF_WriteNgCard_ReadData;						/* 書込みに失敗したｶｰﾄﾞ情報	*/
																					/* 読み出し時の値を保存する	*/
																					/* （停電保障ﾃﾞｰﾀ）			*/
extern	uchar	MIF_mod;															/* ｺﾏﾝﾄﾞ状態				*/

#endif	// _MIF_H_
