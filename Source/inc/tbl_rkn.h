/*[]------------------------------------------------------------------------------------------------------------[]*/
/*| 料金計算テーブル																							 |*/
/*[]------------------------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																						 |*/
/*| Date		: 2001-11-05																					 |*/
/*|	UpData		:																								 |*/
/*[]--------------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _TBL_RKN_H_
#define _TBL_RKN_H_
#include "mem_def.h"
																	/**********************************************/
																	/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	CAR_TIM{													/*　　　　　　　　　　　　　　　　　　　　　　*/
			short	year;											/*　西暦　１９８０～　　　　　　　　　　　　　*/
			char	mon;											/*　月　　　　　　　　　　　　　　　　　　　　*/
			char	day;											/*　日　　　　　　　　　　　　　　　　　　　　*/
			char	week;											/*　曜日　　　　　　　　　　　　　　　　　　　*/
			char	hour;											/*　時　　　　　　　　　　　　　　　　　　　　*/
			char	min;											/*　分　　　　　　　　　　　　　　　　　　　　*/
};																	/*　　　　　　　　　　　　　　　　　　　　　　*/
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
struct	CAR_TIM2{													/*　　　　　　　　　　　　　　　　　　　　　　*/
			short	year;											/*　西暦　１９８０～　　　　　　　　　　　　　*/
			char	mon;											/*　月　　　　　　　　　　　　　　　　　　　　*/
			char	day;											/*　日　　　　　　　　　　　　　　　　　　　　*/
			char	week;											/*　曜日　　　　　　　　　　　　　　　　　　　*/
			char	hour;											/*　時　　　　　　　　　　　　　　　　　　　　*/
			char	min;											/*　分　　　　　　　　　　　　　　　　　　　　*/
			char	sec;											/*　秒　　　　　　　　　　　　　　　　　　　　*/
};																	/*　　　　　　　　　　　　　　　　　　　　　　*/
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）

extern	struct	CAR_TIM		car_in;									/*　入車時刻　　　　　　　　　　　　　　　　　*/
extern	struct	CAR_TIM		car_ot;									/*　出車時刻　　　　　　　　　　　　　　　　　*/
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//extern	struct	CAR_TIM		car_in_f;								/*　入車時刻 fix　　　　　　　　　　　　　　　*/
//extern	struct	CAR_TIM		car_ot_f;								/*　出車時刻 fix　　　　　　　　　　　　　　　*/
extern	struct	CAR_TIM2	car_in_f;								/*　入車時刻 fix　　　　　　　　　　　　　　　*/
extern	struct	CAR_TIM2	car_ot_f;								/*　出車時刻 fix　　　　　　　　　　　　　　　*/
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
extern	struct	CAR_TIM		carin_mt;								/*　入車時刻（元）　　　　　　　　　　　　　　*/
extern	struct	CAR_TIM		carot_mt;								/*　出車時刻（元）　　　　　　　　　　　　　　*/
extern	struct	CAR_TIM		cal_tm;									/*　計算対象時刻　　　　　　　　　　　　　　　*/
extern	struct	CAR_TIM		ren_tm;									/*　更新用時刻　　　　　　　　　　　　　　　　*/
extern	struct	CAR_TIM		org_ot;									/*　オリジナルの出庫時刻　　　　　　　　　　　*/
extern	struct	CAR_TIM		org_in;									/*　オリジナルの入庫時刻　　　　　　　　　　　*/
extern	struct	CAR_TIM		tki_fa;									/*　定期有効開始時刻　　　　　　　　　　　　　*/
extern	struct	CAR_TIM		cardmy;									/*　現在時刻（出庫時刻）サブルーチン用　　　　*/
extern	struct	CAR_TIM		recalc_carin;							/* 再計算用入庫時刻							*/
extern	struct	CAR_TIM		tki_tm;
																	// 
extern	struct	CAR_TIM		nmax_in;								// ｎ時間最大基点時刻
extern	struct	CAR_TIM		cal_tm24;								// ｎ時間最大用現在計算時刻
extern	struct	CAR_TIM		nmax_orgin;								// 定期帯入庫基準時刻保持用ﾊﾞｯﾌｧ
																	/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	RYO_BUF{													/*　　　　　　　　　　　　　　　　　　　　　　*/
	unsigned long	tyu_ryo;										/*　駐車料金　　　　　　　　　　　　　　　　　*/
	unsigned long	dsp_ryo;										/*　表示料金　　　　　　　　　　　　　　　　　*/
	unsigned long	nyukin;											/*　現金入金額計　　　　　　　　　　　　　　　*/
	unsigned long	zankin;											/*　表示残り金額　　　　　　　　　　　　　　　*/
	unsigned long	turisen;										/*　釣り銭　　　　　　　　　　　　　　　　　　*/
	unsigned long	modoshi;										/*　中止戻し額　　　　　　　　　　　　　　　　*/
	unsigned long	fusoku;											/*　払い出し不足額　　　　　　　　　　　　　　*/
	unsigned long	waribik;										/*　割引額計　　　　　　　　　　　　　　　　　*/
	unsigned long	kazei;											/*　課税対象額　　　　　　　　　　　　　　　　*/
	unsigned long	tax;											/*　消費税額　　　　　　　　　　　　　　　　　*/
	unsigned long	tei_ryo;										/*　定期券での駐車料金　　　　　　　　　　　　*/
	unsigned long	dis_fee;										/*　前回料金割引額合計　　　　　　　　　　　　*/
	unsigned long	fee_amount;										/*　料金割引の合計金額（ポイントカードは除く）*/
	unsigned long	dis_tim;										/*　前回時間割引額合計　　　　　　　　　　　　*/
	unsigned long	tim_amount;										/*　時間割引の合計時間（ポイントカードは除く）*/
	unsigned long	dis_per;										/*　前回％割引の割引額合計　　　　　　　　　　*/
	unsigned long	per_amount;										/*　％割引の合計％（ポイントカードは除く）　　*/
	unsigned short	in_coin[5];										/*　金種毎入金枚数　　　　　　　　　　　　　　*/
	unsigned short	out_coin[7];									/*　金種毎出金枚数(10,50,100,500,予10,予50,予100)*/
	char	syubet;													/*　料金種別　　　　　　　　　　　　　　　　　*/
	char	ryo_flg;												/*　定期使用無し精算：通常／修正＝０／１　　　*/
	char	tik_syu;												/*　駐車券料金種別（Ａ～Ｌ種）　　　　　　　　*/
	char	pas_syu;												/*　定期券料金種別（Ａ～Ｌ種）　　　　　　　　*/
	unsigned short	pkiti;											/*　精算した駐車位置番号　　　　　　　　　　　*/
	char	ryos_fg;												/*　領収証発行フラグ（０＝未発行、１＝発行）　*/
	unsigned char	pri_mai[WTIK_USEMAX];							/*　Ｐカード使用枚数（回数券設定の場合回数券）*/
	unsigned long	mis_tyu;										/*　間違い駐車料金　　　　　　　　　　　　　　*/
	unsigned long	mis_ryo;										/*　間違い現金領収額　　　　　　　　　　　　　*/
	unsigned long	mis_wari;										/*　間違い割引額計　　　　　　　　　　　　　　*/
	unsigned char	mis_zenwari;									/*　全額割引フラグ　　　　　　　　　　　　　　*/
	unsigned char	svs_tim;										/*　サービスタイム内精算フラグ：内／外＝１／０*/
	unsigned char	pass_zero;										/*　定期券強制０円精算要求フラグ　　　　　　　*/
	unsigned char	apass_off;										/* 振替精算時のアンチパス強制OFF */

	unsigned long	emonyin;										/*　電子マネー入金額計　　　　　　　　　　　　*/
	credit_use		credit;
	unsigned char	lag_tim_over;										/*　ラグタイムオーバー判定：オーバー＝１／それ以外＝０*/
// 仕様変更(S) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
	ulong			kabarai;										// 振替過払い金(払戻し)
// 仕様変更(E) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
	char			fumei_fg;										/*　出庫・入庫時刻不明フラグ（１＝出庫時刻不明、２＝入庫時刻不明）			　*/
	char			fumei_reason;									/*　不明フラグ詳細理由						  */
																	/* （0:紛失精算、1:初回定期、2:アンチパスOFF、3:強制出庫）*/
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)料金計算
	ulong			zenkai;											// 前回支払額(割引を除く)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)料金計算
// MH810100(S) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
	ulong			zenkaiWari;										// 前回割引額合計(支払額を除く)
// MH810100(E) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
// GG129000(S) R.Endo 2024/01/12 #7217 お買物合計表示対応
	unsigned long	shopping_total;									/*　買物割引金額合計　　　　　　　　　　　　　*/
// GG129000(E) R.Endo 2024/01/12 #7217 お買物合計表示対応
// GG129004(S) M.Fujikawa 2024/10/22 買物金額割引情報対応
	unsigned long	shopping_info;									/*　買物割引金額合計　　　　　　　　　　　　　*/
// GG129004(E) M.Fujikawa 2024/10/22 買物金額割引情報対応
};																	/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	RYO_BUF		ryo_buf;								/*　料金データバッファ　　　　　　　　　　　　*/
																	/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	unsigned short	in_nday;									/*  入車年月日ノーマライズ　　　　　　　　　　*/
extern	unsigned short	in_ntime;									/*　入車時分ノーマライズ　　　　　　　　　　　*/
extern	unsigned short	ot_nday;									/*  出車年月日ノーマライズ　　　　　　　　　　*/
extern	unsigned short	ot_ntime;									/*　出車時分ノーマライズ　　　　　　　　　　　*/
extern	unsigned short	cal_nday;									/*  演算用年月日ノーマライズデータ			  */
extern	unsigned short	cal_ntime;									/*　演算用時分ノーマライズデータ　　　　　　　*/
																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																	/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	OI_BAN{														/*　　　　　　　　　　　　　　　　　　　　　　*/
 	unsigned long	cco_no;											/*  領収書追番　　　　　　　　　　　　　　　　*/
 	unsigned long	tyu_no;											/*  精算中止追番　　　　　　　　　　　　　　　*/
 	unsigned long	fse_no;											/*  不正個別追番　　　　　　　　　　　　　　　*/
 	unsigned long	kyo_no;											/*  強制個別追番　　　　　　　　　　　　　　　*/
	unsigned long	tsyno;											/*　Ｔ小計追番　　　　　　　　　　　　　　　　*/
	unsigned long	tgyno;											/*　Ｔ合計追番　　　　　　　　　　　　　　　　*/
	unsigned long	gtsyno;											/*　ＧＴ小計追番　　　　　　　　　　　　　　　*/
	unsigned long	csyno;											/*　ｺｲﾝ金庫開集計個別追番	　　　　　　　　　*/
	unsigned long	nsyno;											/*　紙幣金庫開集計個別追番　　　　　　　　　　*/
};																	/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	OI_BAN		oi_ban;									/*　　　　　　　　　　　　　　　　　　　　　　*/
																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																	/**********************************************/
typedef struct	{								/* 登録ﾎﾟｲﾝﾀ管理ｴﾘｱ　　　　　　 */
	int		mod;								/* ﾊﾞｯｸｱｯﾌﾟﾌﾗｸﾞ					*/
	long	ryoukin;							/* 								*/
	long	total;								/* 								*/
	long	base_ryo;							/* 								*/
	long	discount;							/* 								*/
	int		syubt;								/* ﾊﾞｯｸｱｯﾌﾟﾌﾗｸﾞ					*/
	struct	CAR_TIM		car_in;					/*　入車時刻　　　　　　　　　　*/
	struct	CAR_TIM		car_ot;					/*　出車時刻　　　　　　　　　　*/
	struct	CAR_TIM		org_ot;					/*　オリジナルの出庫時刻　　　　*/
	struct	CAR_TIM		org_in;					/*　オリジナルの入庫時刻　　　　*/
	char	ntnet_nmax_bak;						/* 最大料金フラグバックアップ　 */
	short	stim;								/*　サービスタイム(料金計算上は最大59400分までOK)*/
	short	gtim;								/*　グレースタイム(料金計算上は最大600分までOK)*/
	short	rtim;								/*　ラグタイム　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM		carin_mt;				/*　入車時刻(元)　　　　　　　　*/
	struct	CAR_TIM		carot_mt;				/*　出車時刻(元)　　　　　　　　*/
	struct	CAR_TIM		nmax_in;				/*　ｎ時間最大基点時刻　　　　　*/
	char	iti_flg;							/*指定時刻最大check時刻に加算する時間　 */
	short 	wk_strt;                            /*　一定料金帯フラグ　　　　　　　　　　*/
	short	amartim;                            /*▲一定料金帯余り時間　　　　　　　　　*/
} t_cal_bak_rec;								/*------------------------------*/
extern t_cal_bak_rec	CAL_WORK;							/* 遠隔用ﾃﾞｰﾀ格納ｴﾘｱ	  byte	*/
#endif	// _TBL_RKN_H_
