#ifndef	___RKN_CALH___
#define	___RKN_CALH___
/*[]----------------------------------------------------------------------[]*/
/*| Rkn_cal.h                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.HARA                                                   |*/
/*| Date        : 2001-11-22                                               |*/
/*| UpDate      : 2004-05-19 T.Nakayama                                    |*/
/*|             : NT4500EX					                               |*/
/*|             : 1. 拡張ｻｰﾋﾞｽ券追加                 		               |*/
/*|             : 2. 定期精算中止ﾊﾞｯﾌｧを配列5件とする  		               |*/
/*|             : 3. 精算中止券ﾃﾞｰﾀ追加(FCR-P30033 TF4800N DH917004)       |*/
/*|             :   										               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	"rkn_def.h"
#include	"mem_def.h"

/* カレンダーテーブル		*/
extern		char	monday[2][12];						/*　平年及び閏年の１月～１２月の日ごとの日数　*/
extern		short	cal_mon[2][12];						/*　平年及び閏年の１月１日からの日数　　　　　*/

/* カードデータテーブル		*/
struct	TSN_TKI{					 					/*　定期データ　　　　　　　　　　　　　　　　*/
			short	pkno;								/*　　駐車場Ｎｏ．　　　　　　　　　　　　　　*/
			char 	pkco[3];							/*　　駐車場コード　　　　　　　　　　　　　　*/
			char	kind;								/*　　定期種別　　　　　　　　　　　　　　　　*/
			short	code;								/*　　個人コード　　　　　　　　　　　　　　　*/
			char	data[6];							/*　　有効開始年月日／有効終了年月日　　　　　*/
			char	status;								/*　　ステータス　　　　　　　　　　　　　　　*/
			char	sttim[4];							/*　　ステータス時刻　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	TSN_TKI		tsn_tki;					/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	TSN_TKI		tsn_tki_b;					/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	TSN_PRP{										/*　プリペイドカードデータ　　　　　　　　　　*/
			unsigned long	zan;						/*　　残額　　　　　　　　　　　　　　　　　　*/
			unsigned long	rno;						/*　　追番　　　　　　　　　　　　　　　　　　*/
			short	idno;								/*　　ＩＤ番号　　　　　　　　　　　　　　　　*/
			short	pakno;								/*　　駐車場Ｎｏ．（特注１）　　　　　　　　　*/
			char	kid;								/*　　拡張ＩＤ　　（特注１）　ダミー　　　　　*/
			char	hanno;								/*　　販売機Ｎｏ．（特注１）　　　　　　　　　*/
			char	ymd[3];								/*　　販売年月日　（特注１）　　　　　　　　　*/
			unsigned long	hangk;						/*　　販売金額　　（特注１）　　　　　　　　　*/
			char	han;								/*　　販売金額データ　（特注１）　　　　　　　*/
			short	sei;								/*　　精算単位　　（特注１）　　　　　　　　　*/
			char	hansam;								/*　　販売金額チェックサム　（特注１）　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	TSN_PRP		tsn_prp;					/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	unsigned long	prp_rno_buf[7];					/*　プリペイド追番（カードＮｏ．）バッファ　　*/
extern	char	prp_rno_cnt;							/*　プリペイド追番（カードＮｏ．）カウント　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	VL_TIK{									/*　駐車券データ　　　　　　　　　　　　　　　*/
			char	chk;								/*　　精算前／後　（０／１）　　　　　　　　　*/
			short	cno;								/*　　駐車場Ｎｏ　　　　　　　　　　　　　　　*/
			short	ckk;								/*　　発券機械Ｎｏ　　　　　　　　　　　　　　*/
			char	syu;								/*　　種別　　　　　　　　　　　　　　　　　　*/
			long	hno;								/*　　発券追番　　　　　　　　　　　　　　　　*/
			char	tki;								/*　　発券／精算月　　　　　　　　　　　　　　*/
			char	hii;								/*　　　〃　　　日  　　　　　　　　　　　　　*/
			char	jii;								/*　　　〃　　　時　　　　　　　　　　　　　　*/
			char	fun;								/*　　　〃　　　分　　　　　　　　　　　　　　*/
			char	mno;								/*　　店Ｎｏ　　　　　　　　　　　　　　　　　*/
			char	kkr;								/*　　操作係員Ｎｏ　　　　　　　　　　　　　　*/
			long	kry;								/*　　回数券領収額　　　　　　　　　　　　　　*/
			char	sa[3];								/*　　サービス券種別別枚数　　　　　　　　　　*/
			short	ichi;								/*　　駐車位置　（立駐）　　　　　　　　　　　*/
			short	sban;								/*　　車番　　　（立駐）　　　　　　　　　　　*/
			char	renum ;								/*    再発行券NO.							  */
			char	retype;								/*	  再発行券ﾀｲﾌﾟ							  */
			ulong	wari;								/* 割引金額(中止,修正用)　　　　　　　　　　　*/
			ulong	time;								/* 割引時間数(中止,修正用)　　　　　　　　　　*/
			uchar	pst;								/* ％割引率(中止,修正用)　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	VL_TIK		vl_tik;						/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	VL_KAS{									/*　回数券データ　　　　　　　　　　　　　　　*/
			short	pkno;								/*　　駐車場Ｎｏ．　　　　　　　　　　　　　　*/
			short	rim;								/*　　制限度数　　　　　　　　　　　　　　　　*/
			short	tnk;								/*　　単位金額（１０～９９９０）　　　　　　　*/
			char	nno;								/*　　残り回数（１～１１）　　　　　　　　　　*/
			char	data[6];							/*　　有効開始年月日／有効終了年月日　　　　　*/
			char	sttim[4];							/*　　ステータス時刻　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	VL_KAS		vl_kas;						/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	cr_kai_nno;								/*　　残り回数（１～１１）読込データ　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	VL_SVS{											/*　サービス券データ　　　　　　　　　　　　　*/
			short	pkno;								/*　　駐車場Ｎｏ．　　　　　　　　　　　　　　*/
			char	sno;								/*　　サービス券種（１～３）　　　　　　　　　*/
			char	sts;								/*    割引券割引種別（１～１００．０＝割引券以外）*/
			short	mno;								/*　　店ＮＯ．（　０～１００）　　　　　　　　*/
			char	ymd[6];								/*　　有効開始年月日／終了年月日　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	VL_SVS		vl_svs;						/*　　　　　　　　　　　　　　　　　　　　　　*/

struct	VL_GGS{									// 減額精算券データ
			ulong	ryokin;								// 駐車料金
			ulong	zangaku;							// 支払い残額
			ulong	tonyugaku;							// 投入金額
			ulong	waribikigaku;						// 駐車料金割引額
};
extern	struct	VL_GGS		vl_ggs;

struct	VL_FRS{									// 振替精算券データ
			ushort	lockno;								// 振替元・フラップ／ロック装置No.(1-324)
			ulong	price;								// 振替元・支払額（料金＋外税）
			ulong	in_price;							// 振替元・現金,プリペイド,回数券,クレジット,Eマネー入金額
			uchar	syubetu;							// 振替元料金種別
			ST_OIBAN	seisan_oiban;					// 振替元精算追い番
			date_time_rec	seisan_time;				// 振替元精算時刻
			ulong	furikaegaku;						// 振替可能額
			ulong	zangaku;							// 振替先・振替前残額
			uchar	antipassoff_req;					// アンチパス強制OFF要求
// MH322914(S) K.Onodera 2016/09/14 AI-V対応：振替精算
			ulong	genkin_furikaegaku;					// 振替額（現金のみ）
			ushort	card_type;							// 振替媒体
			ulong	card_furikaegaku;					// 振替額（クレジット、電子マネー）
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
			ulong	wari_furikaegaku;					// 振替額(割引)
			ushort	kabarai;							// 振替過払い金
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
// MH322914(E) K.Onodera 2016/09/14 AI-V対応：振替精算
};
extern	struct	VL_FRS		vl_frs;

// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
// 車番データ
struct VL_CAR_NO{
	uchar			CarSearchFlg;									// 車番/日時検索種別
	uchar			CarSearchData[6];								// 車番/日時検索データ
																	// 車番検索の場合：0～3に車番、4,5は0固定
																	// 日時検索の場合：年２バイト、それ以外は１バイト
	uchar			CarNumber[4];									// センター問い合わせ時の車番
	uchar			CarDataID[32];									// 車番データID(問合せに使用するID)
};
extern	struct	VL_CAR_NO		vl_car_no;
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)

														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern		char	prk_kind;							/*　駐車Ｎｏ．／拡張Ｎｏ．（１／２）　　　　　*/
extern		char	vl_now;								/*　現在ＶＬカードデータ　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	cr_dat_n;								/*　カードリーダデータ　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	cr_tkpk_flg;							/*　カードデータ定期／駐車券フラグ(1:定　2:駐)*/
extern	char	tki_pk_use;								/*　定期／駐車券使用データ(1:定　2:駐　3:両方)*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	TKI_CYUSI {										/*　定期券中止データ格納領域　　　　　　　　　*/
			ulong	pk	;								/*　１～３００００　　　　　　　　　　　　　　*/
			ulong	no 	;								/*　定期ID（１～１２０００）　　　　　　　　　*/
			uchar	tksy;								/*　定期種別（１～１５）　　　　　　　　　　　*/
			uchar	syubetu;							/*　券種（１～１２：Ａ～Ｌ）　　　　　　　　　*/
			ushort	year;								/*　処理　年（４桁）　　　　　　　　　　　　　*/
			uchar	mon	;								/*　　　　月（１～１２）　　　　　　　　　　　*/
			uchar	day	;								/*　　　　日（１～３１）　　　　　　　　　　　*/
			uchar	hour;								/*　　　　時（０～２３）　　　　　　　　　　　*/
			uchar	min	;								/*　　　　分（０～５９）　　　　　　　　　　　*/
			ushort	sec	;								/*　　　　秒（０～５９）　　　　　　　　　　　*/
			ulong	kry	;								/*　回数券領収額　　　　　　　　　　　　（中止券ﾀｲﾌﾟ=APS時有効）*/
			ushort	mno	;								/*　店№　　　　　　　　　　　　　　　　（中止券ﾀｲﾌﾟ=APS時有効）*/
			uchar	sa[3];								/*　サービス券Ａ～Ｃ　　　　　　　　　　（中止券ﾀｲﾌﾟ=APS時有効）*/
			uchar	use_count;							/*　使用枚数（０～９９枚）　　　　　　　（中止券ﾀｲﾌﾟ=掛売券時有効）*/
			ulong	wryo;								/*　料金割引合計額（０～９９９９０円）　（中止券ﾀｲﾌﾟ=掛売券時有効）*/
														/*　（回数券／プリペイド領収額も含む）　（中止券ﾀｲﾌﾟ=掛売券時有効）*/
			ulong	wminute;							/*　時間割引合計時間（０～９９９９９分）（中止券ﾀｲﾌﾟ=掛売券時有効）*/
			ushort	wpercent;							/*　％割引率（０～１００％）現在未使用　（中止券ﾀｲﾌﾟ=掛売券時有効）*/
};

#define	TKI_CYUSI_MAX	20
typedef struct {
	short		count;									/*　登録件数　　　　　　　　　　　　　　　　　*/
	short		wtp;									/*　次書込み位置　　　　　　　　　　　　　　　*/
	struct	TKI_CYUSI	dt[TKI_CYUSI_MAX];				/*　無効駐車券データテーブル　　　　　　　　　*/
} t_TKI_CYUSI;

extern	t_TKI_CYUSI		tki_cyusi;						/*　定期中止バッファ　　　　　　　　　　　　　*/
extern	t_TKI_CYUSI		tki_cyusi_wk;					/*　定期中止バッファ（退避バッファ）　　　　　*/
extern	char	tkcyu_ichi	 ;							/*　定期中止ﾃﾞｰﾀ読取位置            　　　　　*/

/* 料金計算データ編集共通テーブル	*/
extern	char	syashu;									/*　車種　　　　　　　　　　　　　　　　　　　*/
extern	char	sitei_flg;								/*　指定時刻日毎最大　フラグ　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	rysyasu;								/*　料金計算キー車種　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	CHK_TIM{								/*　　　　　　　　　　　　　　　　　　　　　　*/
		char	hour;									/*　時　　　　　　　　　　　　　　　　　　　　*/
		char	min;									/*　分　　　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	CHK_TIM		chk_tim;					/*　ＩＮ＋２４時チェック用　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	chk_time;								/*　　　　　”　　　　　　　　　　　　　　　　*/
extern	short	rmax_cnt;								/*　最大料金到達回数　　　　　　　　　　　　　*/
extern	short	rmax_ptn;								/*　最大料金到達時パタン　　　　　　　　　　　*/
extern	short	rmax_pcn;								/*　最大料金到達時単位係数　　　　　　　　　　*/
extern	short	rmax_cyc;								/*　最大料金到達時パタン繰り返し回数　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	sp_mx_1;								/*　IN+24最大時cal_time調整フラグ 　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern 	short	wk_strt;								/*指定時刻最大check時刻に加算する時間　　　　 */
extern	short	last_bnd;								/*　次体系突入直前の料金帯　　　　　　　　　　*/
extern	char	et_dbl;									/*　重複帯ﾌﾗｸﾞ OFF:なし ON:重復0で内出庫　　　*/
														/*　　　　　　 NOF:重複1（重複0で帯内入出庫） */
extern	char	et_jiot;								/*　重複帯内入出庫ﾌﾗｸﾞ　NOF 　　　　　　　　　*/
														/*											  */
														/*											  */
/* 料金計算要求テーブル		*/							/*											  */
struct	REQ_RKN{										/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	syubt;										/*　車種　　　　　　　　　　　　　　　　　　　*/
	char	param;										/*　パラメータ　　　　　　　　　　　　　　　　*/
	long	data[5];									/*　データ　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	REQ_RKN		req_rkn;					/*　料金要求テーブル　　　　　　　　　　　　　*/
extern	struct	REQ_RKN		req_rhs[150];				/*　現在計算後格納エリア（ＭＡＸ２５件）　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	REQ_CRD{										/*　駐車券読み取り時券データ要求格納バッファ　*/
		char	syubt;									/*　車種　　　　　　　　　　　　　　　　　　　*/
		char	param;									/*　パラメータ　　　　　　　　　　　　　　　　*/
		long	data[2];								/*　データ　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/

#define WAR_CRD_MAX	25

extern	struct	REQ_CRD		req_crd[WAR_CRD_MAX];		/*　料金要求テーブル　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	req_crd_cnt;							/*　駐車券読み取り時券データ要求件数　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	rhspnt;									/*　現在格納数　　　　　　　　　　　　　　　　*/
extern	long	ryoukin;								/*　現在料金　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	discount;								/*　割引額　　　　　　　　　　　　　　　　　　*/
extern	long	tol_dis;								/*　割引額合計　　　　　　　　　　　　　　　　*/
extern	long	discount_tyushi;						/*　割引額　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	REQ_TKI{										/*　定期処理後料金計算要求テーブル　　　　　　*/
			char	syubt;								/*　車種　　　　　　　　　　　　　　　　　　　*/
			char	param;								/*　パラメータ　　　　　　　　　　　　　　　　*/
			long	data[2];							/*　データ　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	REQ_TKI		req_tki[20];				/*　定期処理後格納エリア（ＭＡＸ２０件）　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	tki_ken;								/*　定期処理後料金計算要求件数　　　　　　　　*/
extern	char	tki_flg;								/*　定期処理後ｆｌｇ　　　　　　　　　　　　　*/
extern	short	ps_type[TKI_MAX];						/*　定期使用目的の算出　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
														// 
														// 
extern	short	ta_st[9];								/*　体系開始時刻　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	ps_st[9][TKI_MAX];						/*　定期開始時刻　　　　　　　　　　　　　　　*/
extern	short	ps_ed[9][TKI_MAX];						/*　定期終了時刻　　　　　　　　　　　　　　　*/
extern	short	ps_tm[9][TKI_MAX];						/*　定期時間　　　　　　　　　　　　　　　　　*/
														// 
extern	char	ps_sepa[9][TKI_MAX];					// 定期帯分割フラグ 0＝なし/1＝あり
extern	short	ps_st1[9][TKI_MAX];						// 定期開始時刻(分割1)
extern	short	ps_ed1[9][TKI_MAX];						// 定期終了時刻(分割1)
extern	short	ps_tm1[9][TKI_MAX];						// 定期時間(分割1)
extern	short	ps_st2[9][TKI_MAX];						// 定期開始時刻(分割2)
extern	short	ps_ed2[9][TKI_MAX];						// 定期終了時刻(分割2)
extern	short	ps_tm2[9][TKI_MAX];						// 定期時間(分割2)
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	syu_tax;								/*　集計用消費税金額　　　　　　　　　　　　　*/
extern	short	ji_st[9];								/*　時間割引の有効開始時刻　　　　　　　　　　*/
extern	short	ji_ed[9];								/*　時間割引の有効終了時刻　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	jik_dtm;								/*　時間割引処理用割引時間　　　　　　　　　　*/
extern	char	jik_flg;								/*　時間割引処理ｆｌｇ　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	base_ryo;								/*　駐車料金（基本型）　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	prc_no;									/*　金額切上げ切捨て処理方法　　　　　　　　　*/
extern	short	prc_dat;								/*　金額切上げ切捨て処理内容　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	rag_flg;								/*　ラグタイムｆｌｇ　　　　　　　　　　　　　*/
extern	char	re_req_flg;								/*　再要求フラグ　　　　　　　　　　　　　　　*/
extern	char	iti_flg;								/*　一定料金帯フラグセット　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	p_taxflg[4];							/*プリペイド税金処理ｆｌｇ　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	ptn_ccnt ;								/*　現在単位係数カウンタクリア　　　　　　　　*/
extern	short	cycl_ccnt;								/*　現在繰り返し回数カウンタクリア　　　　　　*/
extern	char	befr_style;								/*　前日の体系　　　　　　　　　　　　　　　　*/
extern	char	befr_flag;								/*　前日の体系フラグ　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	su_jik_dtm;								/*　種別割引時間　　　　　　　　　　　　　　　*/
extern	long	mae_dis;								/*　旧時間割引額　　　　　　　　　　　　　　　*/
extern	long	sn_stp_pzan;							/*　精算中止券Ｐ残額　　　　　　　　　　　　　*/
extern	long	tyushi_mae_dis;
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	ctyp_flg;								/*　計算方法フラグ　　　　　　　　　　　　　　*/
extern	long	svcd_dat;								/*　サービス券フラグ　　　　　　　　　　　　　*/
extern	char	base_dis_flg;							/*　ベース料金全額割引フラグ　　　　　　　　　*/
extern	char	fun_kti;								/*　紛失料金は固定料金ｆｌｇ　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	sei_type;								/*　精算タイプ　０：通常　１：再精算　　　　　*/
extern	long	sky_ryo;								/*　集計用駐車料金（売上額）　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	RYO_BUF_N {										/*　　　　　　　　　　　　　　　　　　　　　　*/
	long	ryo ;										/*　料金額　　　　　　　　　　　　　　　　　　*/
	long	tax ;										/*　消費税額　　　　　　　　　　　　　　　　　*/
	long	dis ;										/*　割引額　　　　　　　　　　　　　　　　　　*/
	long	nyu ;										/*　入金額　　　　　                          */
	long	hus ;										/*　不足額　　　　　                          */
	long	require ;									/*　不足額　　　　　                          */
};														/*　　　　　　　　　　　　　　　　            */
extern	struct	RYO_BUF_N		ryo_buf_n;				/*　料金データバッファ　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	c_pay;									/*　プリペイドカード使用額　　　　　　　　　　*/
extern	char	tkv_flg;								/*　定期有効無効フラグ　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/

extern	short	fee_std[9][12][6];						/*　基本料金［体系］［車種］［帯］　　　　　　*/
extern	short	fee_exp[9][12][6];						/*　追加料金［体系］［車種］［帯］　　　　　　*/
extern	short	bnd_strt[9][13];						/*　料金帯開始時刻［体系］［帯］　　　　　　　*/
extern	char	ovl_type[9][6];							/*　重複方式［体系］［帯］　　　　　　　　　　*/
extern	short	ovl_time[9][6];							/*　重複時間［体系］［帯］　　　　　　　　　　*/
extern	short	leng_std[9][6];							/*　基本時間［体系］［帯］　　　　　　　　　　*/
extern	short	leng_exp[9][6];							/*　追加時間［体系］［帯］　　　　　　　　　　*/
extern	short	max_bnd[9];								/*　体系で使用する料金帯数［体系］　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	fee_midnig[3][12];						/*　深夜料金［体系］［車種］　　　　　　　　　*/
extern	short	fee[3][12][12];							/*パターン単位料金［体系］［車種］［パターン］*/
extern	short	teigen_strt[3];							/*　逓減帯開始時刻［体系］　　　　　　　　　　*/
extern	short	teigen_ovl[3];							/*　逓減帯重複時刻［体系］　　　　　　　　　　*/
extern	short	midnig_strt[3];							/*　深夜帯開始時刻［体系］　　　　　　　　　　*/
extern	short	midnig_ovl[3];							/*　深夜帯重複時刻［体系］　　　　　　　　　　*/
extern	short	style_type[3];							/*　料金体系使用の有無［体系］　　　　　　　　*/
extern	char	cycl_frst[3];							/*　繰り返し開始パターン［体系］　　　　　　　*/
extern	char	cycl_last[3];							/*　繰り返し終了パターン［体系］　　　　　　　*/
extern	short 	cycl_cnt[3];							/*　繰り返し回数［体系］　　　　　　　　　　　*/
extern	short	ptn_leng[3][12];						/*　パターン単位時間［体系］［パターン］　　　*/
extern	short	ptn_cycl[3][12];						/*　パターン単位係数［体系］［パターン］　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/

extern	long	abs_max[9][12];							/*　絶対最大料金［体系］［車種］　　　　　　　*/
extern	long	day_max1[9][12];						/*　日毎最大料金１［体系］［車種］　　　　　　*/
extern	long	day_max2[9][12];						/*　日毎最大料金２［体系］［車種］　　　　　　*/
extern	long	band_max[6][9][12];						/*　時間帯別最大料金　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	cons_tax;								/*　消費税額　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	total;									/*　駐車料金集計用変数　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	tb_number;								/*　計算結果格納テーブル通し番号　　　　　　　*/
extern	short	cal_type;								/*　料金計算方式　　　　　　　　　　　　　　　*/
extern	short	cal_time;								/*　計算対象時刻　　　　　　　　　　　　　　　*/
extern	short	in_time;								/*　入庫時刻　　　　　　　　　　　　　　　　　*/
extern	short	ot_time;								/*　出庫時刻　　　　　　　　　　　　　　　　　*/
extern	char	style;									/*　料金体系　　　　　　　　　　　　　　　　　*/
extern	char	band;									/*　料金帯　　　　　　　　　　　　　　　　　　*/
extern	char	car_type;								/*　車種　　　　　　　　　　　　　　　　　　　*/
extern	short	cons_tax_ratio;							/*　消費税率　　　　　　　　　　　　　　　　　*/
extern	short	husoku_flg;								/*　不足額に対するお釣り計算を表すフラグ　　　*/
extern	char	next_style;								/*　次の料金体系　　　　　　　　　　　　　　　*/
extern	short	next_style_strt;						/*　次の料金体系開始時刻　　　　　　　　　　　*/
extern	short	strt_ptn;								/*　定期帯入車後の開始逓減パターン　　　　　　*/
extern	short	dis_tax;								/*　割引額の消費税計算　　　　　　　　　　　　*/
extern	int		bnd_end_tm;								/*　料金帯終了時刻　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	maxmonth[13];							/*　各月の最大日設定　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	sp_op;									/*▲余り処理方式　　　　　　　　　　　　　　　*/
extern	char	sp_it;									/*▲一定料金帯余り方式　　　　　　　　　　　　*/
extern	char	sp_mx;									/*▲日毎最大料金方式　　　　　　　　　　　　　*/
extern	short	h24time;								/*▲入庫からトータル時分　　　　　　　　　　　*/
extern	short	wk_h24t;								/*▲計算時分　　　　　　　　　　　　　　　　　*/
extern	short	amartim;								/*▲一定料金帯余り時間　　　　　　　　　　　　*/
extern	short	amr_flg;								/*▲一定料金帯余りフラグ　　　　　　　　　　　*/
extern	short	NTIME;									/*　日毎最大適用時間　　　　　　　　　　　　　*/
extern	short	koetim;									/*　Ｎ時間超過分の時間			　　　　　　　*/
extern	long	chk_max1;								/*▲日毎最大料金　　　　　　　　　　　　　　　*/
extern	long	chk_max2;								// 2種類目のｎ時間最大料金
extern	long	nyukin;									/*入金金額　　　　　　　　　　　　　　　　　　*/
extern	long	turi;									/*釣り銭金額　　　　　　　　　　　　　　　　　*/
extern	char 	n_hour_style;							/*　ｎ時間最大の最大料金を取る料金体系　　　　*/
extern	int  	n_hour_cnt;								/*　ｎ時間最大ｍ回の適用回数　　　　　　　　　*/
extern	short	T_N_HOUR;								/*　ｎ時間最大の最大時間　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
#define			N_MAX_OVER		999990					/*　ｎ時間最大ｍ回経過後の最大料金　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	tt_tim;									/*　定期帯時間　　　　　　　　　　　　　　　　*/
extern	long	tt_ryo;									/*　最大料金用料金　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	long	tt_x;									/*　時刻指定用ｔｏｔａｌ　ｗｏｒｋ　　　　　　*/
extern	long	tt_y;									/*　時刻指定用ｒｙｏｕｋｉｎ　ｗｏｒｋ　　　　*/
extern	long	c_pay_afr;								/*　プリペイドカード使用後料金　　　　　　　　*/
extern	char	percent;								/*　％割引率　　　　　　　　　　　　　　　　　*/
extern	char	ryo_cnt;     							/*　領収証用計算結果格納件数カウンタ　　　　　*/
extern	char 	nmax_style;								// ｎ時間最大の最大料金を取る料金体系
extern	short	NMAX_HOUR1;								// ｎ時間最大1の最大時間（全種別共通）
extern	long	jmax_ryo;								// ２種日毎最大用Total Work
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	TNSA_DAT{										/*　領収証用　店ＮＯ．、サービス券データ　　　*/
			char	  maisu;							/*　枚数　　　　　　　　　　　　　　　　　　　*/
			long	  king;								/*　金額　　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	TNSA_DAT	sav_dat[3];					/*　サービス券用（Ａ～Ｃ）データ格納エリア　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	tki_syu;								/*											  */
extern	long	azukari   ;								/*	駐車券ﾃﾞｰﾀ預り金額       				  */
extern	char 	su_jik_plus;							/*　定期種別割引時間プラスｆｌｇ　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	REQ_TKC{										/*　定期処理後料金計算要求テーブル　　　　　　*/
			char	syubt;								/*　車種　　　　　　　　　　　　　　　　　　　*/
			char	param;								/*　パラメータ　　　　　　　　　　　　　　　　*/
			long	data[2];							/*　データ　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	REQ_TKC		req_tkc[5];					/*　定期中止ﾃﾞｰﾀ格納領域（ＭＡＸ５件）    　　*/
extern	char	req_tkc_cnt ;							/*　定期券中止精算要求納件数		　　　　　*/

/* 設定テーブル		*/
struct	SVT{									/*　サービスタイムテーブル　　　　　　　　　　*/
	short	stim;										/*　サービスタイム(料金計算上は最大59400分までOK)*/
	short	gtim;										/*　グレースタイム(料金計算上は最大600分までOK)*/
	short	rtim;										/*　ラグタイム　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	SVT	se_svt;								/*　サービスタイムテーブル　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	STD{									/*　サービス券期限用日付データ　　　　　　　　*/
	char	y;											/*　年　　　　　　　　　　　　　　　　　　　　*/
	char	m;											/*　月　　　　　　　　　　　　　　　　　　　　*/
	char	d;											/*　日　　　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/

/* 時間テーブル設定		*/
struct	HZUK{									/*　　　　　　　　　　　　　　　　　　　　　　*/
		short	y;										/*　西暦　１９９０～　　　　　　　　　　　　　*/
		char	m;										/*　月　　　　　　　　　　　　　　　　　　　　*/
		char	d;										/*　日　　　　　　　　　　　　　　　　　　　　*/
		char	w;										/*　曜日　　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	HZUK	hzuk;							/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	JIKN{									/*　　　　　　　　　　　　　　　　　　　　　　*/
		char	t;										/*　時　　　　　　　　　　　　　　　　　　　　*/
		char	m;										/*　分　　　　　　　　　　　　　　　　　　　　*/
		char	s;										/*　秒　　　　　　　　　　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	JIKN	jikn;							/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/

/* バージョンテーブル		*/
struct	VER{									/*　　　　　　　　　　　　　　　　　　　　　　*/
		char	kind;									/*　ＴＭバージョン１ー４　　　　　　　　　　　*/
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	VER		version;						/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	uchar	wrcnt_sub;
/*[]----------------[]*/
/*| APS Pass Card    |*/
/*[]----------------[]*/
typedef	struct	m_apspas_rec {							/*　　　　　　　　　　　　　　　　　　　　　　*/
	unsigned char	aps_idc;							/*　ID Code(1AH)　　　　　　　　　　　　　　　*/
	unsigned char	aps_pno[2];							/*　[0]Parking No.(P0-6)　　　　　　　　　　　*/
														/*　[1]	(P7-9)&Pas Kind 　　　　　　　　　　　*/
	unsigned char	aps_pcd[2];							/*　Personal Code 　　　　　　　　　　　　　　*/
	unsigned char	aps_sta[3];							/*　Start Y-M-D 　　　　　　　　　　　　　　　*/
	unsigned char	aps_end[3];							/*　End Y-M-D 　　　　　　　　　　　　　　　　*/
	unsigned char	aps_sts;							/*　Status　　　　　　　　　　　　　　　　　　*/
	unsigned char	aps_wrt[4];							/* Write Y-D-H-M　　　　　　　　　　　　　　　*/
} m_apspas;												/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
typedef struct pas_rec_rec {							/*　　　　　　　　　　　　　　　　　　　　　　*/
	long	pno;										/*　Parking No. 　　　　　　　　　　　　　　　*/
	unsigned char GT_flg;
	unsigned short	cod;								/*　Personal Code 　　　　　　　　　　　　　　*/
	short	knd;										/*　Pass Kind 　　　　　　　　　　　　　　　　*/
	unsigned short	std;								/*　Pass Start Day (BIN)　　　　　　　　　　　*/
	unsigned short	end;								/*　Pass End Day (BIN)　　　　　　　　　　　　*/
	short 	sts;										/*　Status　　　　　　　　　　　　　　　　　　*/
	unsigned char	trz[4];								/*　Tranzaction Day-Time　　　　　　　　　　　*/
	char	typ;										/*　Pass Type (0:Basic/1:Extra1/2:Extra2/3:Extra3)*/
	char	std_end[6];									/*  [0]-[2]=Start Day,[3]-[5]=End Day		  */
} pas_rcc;												/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
/*[]----------------[]*/
/*| Service Ticket   |*/
/*[]----------------[]*/
typedef	struct	m_servic_rec {							/*　　　　　　　　　　　　　　　　　　　　　　*/
	unsigned char	svc_idc;							/*　ID Code(2DH)　　　　　　　　　　　　　　　*/
	unsigned char	svc_pno[2];							/*　[0]Parking No.(P0-6)　　　　　　　　　　　*/
														/*　[1] (P7-9)&Kind 　　　　　　　　　　　　　*/
	unsigned char	svc_sno[2];							/*　Shop No.　　　　　　　　　　　　　　　　　*/
	unsigned char	svc_sta[3];							/*　Start Y-M-D 　　　　　　　　　　　　　　　*/
	unsigned char	svc_end[3];							/*　End   Y-M-D 　　　　　　　　　　　　　　　*/
	unsigned char	svc_sts;							/*　Status　　　　　　　　　　　　　　　　　　*/
	unsigned char	svc_tim[4];							/*　Write Time M-D-H-M　　　　　　　　　　　　*/
} m_servic;												/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
typedef struct	svs_rec_rec {							/*　　　　　　　　　　　　　　　　　　　　　　*/
	long	pno;										/* Parking No.　　　　　　　　　　　　　　　　*/
	unsigned char GT_flg;
	short	knd;										/* Service Kind 　　　　　　　　　　　　　　　*/
	short	cod;										/* Shop No. 　　　　　　　　　　　　　　　　　*/
	unsigned short	std;								/* Pass Start Day (BIN) 　　　　　　　　　　　*/
	unsigned short	end;								/* Pass End Day (BIN) 　　　　　　　　　　　　*/
	short	sts;										/* Status 　　　　　　　　　　　　　　　　　　*/
} svs_rec;												/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
/*[]----------------[]*/
/*| Prepaid Card     |*/
/*[]----------------[]*/
typedef	struct	m_prepid_rec {							/*　　　　　　　　　　　　　　　　　　　　　　*/
	unsigned char	pre_idc;							/*　ID Code(0EH)　　　　　　　　　　　　　　　*/
	unsigned char	pre_pno[4];							/*　[0-4]Parking No.　　　　　　　　　　　　　*/
	unsigned char	pre_mno[2];							/*　Macine No.　　　　　　　　　　　　　　　　*/
	unsigned char	pre_sta[6];							/*　Start Y-M-D 　　　　　　　　　　　　　　　*/
	unsigned char	pre_amo;							/*　Ammount 　　　　　　　　　　　　　　　　　*/
	unsigned char	pre_ram[5];							/*　Remain Ammount　　　　　　　　　　　　　　*/
	unsigned char	pre_plm[3];							/*　Pay Limmite 　　　　　　　　　　　　　　　*/
	unsigned char	pre_cno[5];							/*　Card No.　　　　　　　　　　　　　　　　　*/
	unsigned char	pre_sum;							/*　Check Sum 　　　　　　　　　　　　　　　　*/
} m_prepid;												/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
typedef struct	pre_rec_rec {							/*　　　　　　　　　　　　　　　　　　　　　　*/
	long	pno;										/*　Parking No. 　　　　　　　　　　　　　　　*/
	unsigned char GT_flg;
	short	mno;										/*　Macine No.　　　　　　　　　　　　　　　　*/
	long	amo;										/*　Ammount 　　　　　　　　　　　　　　　　　*/
	long	ram;										/*　Remain Ammount　　　　　　　　　　　　　　*/
	short	plm;										/*　Pay Limmite 　　　　　　　　　　　　　　　*/
	long	cno;										/*　 Card No. 　　　　　　　　　　　　　　　　*/
	ushort	kigen_year;									/* 有効期限年 */
	uchar	kigen_mon;									/* 有効期限月 */
	uchar	kigen_day;									/* 有効期限日 */
} pre_rec;												/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/

// MH322914 (s) kasiyama 2016/07/15 AI-V対応
/*[]----------------[]*/
/*| Ticket (Stop) 	 |*/
/*[]----------------[]*/
typedef	struct	m_ticstp_rec {
	unsigned char	tcb_idc;							/* ID Code (24/25H/26H/27H)					*/
	unsigned char	tcb_pno[2];							/* [0]Parking No.(P0-6)						*/
														/* [1]	(P7-8)&Machine						*/
	unsigned char	tcb_tno[3];							/* Kind & Ticket No.						*/
	unsigned char	tcb_tim[3];							/* Dispance Time							*/
	unsigned char	tcb_mno;							/* Shop No.									*/
	unsigned char	tcb_amo[3];							/* Oprater no.- Amount						*/
	unsigned char	tcb_svs[3];							/* Service 									*/
} m_ticstp;
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

// MH810100(S) Y.Yamauchi 20191212 車番チケットレス(メンテナンス)			
typedef struct	tik_rec_rec {
	ushort	tkd;										/* Ticket Kind								*/
	ushort	pos;										/* Head Position							*/
	long	pno;										/* Parking No.								*/
	unsigned char GT_flg;
	uchar	tdno;										/* 発券機番号		(0, 1-20)				*/
	ushort	knd;										/* Kind										*/
	ulong	tno;										/* Ticket No.								*/
	uchar	tim[4];										/* Dispance Time							*/
	ushort  tate_s[3];									/* 多店舗 店№								*/
	ushort	tate_w[3];									/* 多店舗 割引種別							*/

	ushort	mno;										/* Shop No.			(中止券ﾀｲﾌﾟ=APS時有効)	*/
	uchar	svs[3];										/* Service 			(中止券ﾀｲﾌﾟ=APS時有効)	*/
	ulong	amo;										/* Ammount			(中止券ﾀｲﾌﾟ=APS時有効)	*/
	uchar	svs_t[3];									/* Service 多店舗	(中止券ﾀｲﾌﾟ=APS時有効)	*/
	ulong	amo_t;										/* Amount  多店舗	(中止券ﾀｲﾌﾟ=APS時有効)	*/

	ushort	percent;									/* %割引率			(中止券ﾀｲﾌﾟ=掛売券時有効)*/
	ushort	use_count;									/* 使用枚数			(中止券ﾀｲﾌﾟ=掛売券時有効)*/
	ulong	w_ryo;										/* 料金割引 割引額	(中止券ﾀｲﾌﾟ=掛売券時有効)*/
	ulong	w_minute;									/* 時間割引 時間数	(中止券ﾀｲﾌﾟ=掛売券時有効)*/
} tik_rec;
// MH810100(E) Y.Yamauchi 20191212 車番チケットレス(メンテナンス)			
/*[]----------------[]*/
/*| 回数券           |*/
/*[]----------------[]*/
typedef	struct	m_kaisuutik_rec {						/*　　　　　　　　　　　　　　　　　　　　　　*/
	unsigned char	kaitik_idc;							/*　ID Code(1AH)　　　　　　　　　　　　　　　*/
	unsigned char	kaitik_pno[2];						/*　[0]駐車場 No.	(P0-6)　　　　　　　　　　*/
														/*　[1]駐車場 No.	(P7-9)　　　　　　　　　　*/
														/*　[1]制限度数		(P0-3)　　　　　　　　　　*/
	unsigned char	kaitik_tan[2];						/*　単位金額 　　 　　　　　　　　　　　　　　*/
	unsigned char	kaitik_sta[3];						/*　有効開始日（年／月／日）　　　　　　　　　*/
	unsigned char	kaitik_end[3];						/*　有効終了日（年／月／日）　　　　　　　　　*/
	unsigned char	kaitik_kai;							/*　回数（残り）　　　　　　　　　　　　　　　*/
	unsigned char	kaitik_wrt[4];						/*　処理日時（月／日／時／分）　　　　　　　　*/
} m_kaisuutik;											/*　　　　　　　　　　　　　　　　　　　　　　*/

struct	crd_inf {										/*　　　　　　　　　　　　　　　　　　　　　　*/
	pas_rcc		PAS;									/*　Pass Card Inf.　　　　　　　　　　　　　　*/
	svs_rec		SVS;									/*　Service Card Inf. 　　　　　　　　　　　　*/
	pre_rec		PRE;									/*　Prepaid Card Inf. 　　　　　　　　　　　　*/
// MH810100(S) Y.Yamauchi 20191212 車番チケットレス(メンテナンス)			
	tik_rec		TIK;									/*  Ticket Card Inf.						  */
// MH810100(E) Y.Yamauchi 20191212 車番チケットレス(メンテナンス)			
};														/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	struct	crd_inf	CRD_DAT;						/*　　　　　　　　　　　　　　　　　　　　　　*/
														/*　　　　　　　　　　　　　　　　　　　　　　*/
/*[]----------------[]*/
/*| 係員カード       |*/
/*[]----------------[]*/
typedef	struct	m_kakari_rec {							/*　　　　　　　　　　　　　　　　　　　　　　*/
	unsigned char	kkr_idc;							/*　ID Code (41H)　 　　　　　　　　　　　　　*/
	unsigned char	kkr_year[2];						/*　Year  (Ascii code)        　　　　　　  　*/
	unsigned char	kkr_mon[2];							/*　Month (Ascii code)　　　　　　　　　　　　*/
	unsigned char	kkr_day[2];							/*　Day 　(Ascii code)　　　　　　　　　　　　*/
	unsigned char	kkr_rsv1[3];						/*　Reserve (20H-all) 　　    　　　　　　　　*/
	unsigned char	kkr_did;							/*　Data ID (53H)  　　　　　　　　　　　　　 */
	unsigned char	kkr_park[3];						/*　Parking code (50H,41H,4BH)　　　　　　　　*/
	unsigned char	kkr_role;							/*　Role code                   　　　　　　　*/
	unsigned char	kkr_lev;							/*　Level                     　　　　　　　　*/
	unsigned char	kkr_type;							/*　Card Type                 　　　　　　　　*/
	unsigned char	kkr_kno[4];							/*　Kakari-inn Number (Ascii code)  　　　　　*/
	unsigned char	kkr_rsv2[6];						/*　Reserve (20H-all) 　　    　　　　　　　　*/
	unsigned char	kkr_pno[4];							/*　Parking Number (Ascii code)　　　 　　　　*/
	struct {
		unsigned char	park;							/*　Parking ticket            　　　　　　　　*/
		unsigned char	pass;							/*　Pass Card                 　　　　　　　　*/
		unsigned char	serv;							/*　Service Ticket            　　　　　　　　*/
		unsigned char	kake;							/*　Kakeuri Ticket            　　　　　　　　*/
		unsigned char	wari;							/*　Waribiki Ticket           　　　　　　　　*/
		unsigned char	kaisu;							/*　Kaisu Ticket              　　　　　　　　*/
		unsigned char	prep;							/*　Prepaid Card              　　　　　　　　*/
		unsigned char	point;							/*　Point Card                　　　　　　　　*/
		unsigned char	rsv1;							/*　Reserve                   　　　　　　　　*/
		unsigned char	rsv2;							/*　Reserve                   　　　　　　　　*/
	} kkr_jdg;
	unsigned char	kkr_rsv3[28];						/*　Reserve (20H-all) 　　    　　　　　　　　*/
} m_kakari;												/*　　　　　　　　　　　　　　　　　　　　　　*/
/*[]----------------[]*/
/*| Repark専用カード |*/
/*[]----------------[]*/
typedef	struct	m_repark_kakari_rec {					/*　　　　　　　　　　　　　　　　　　　　　　*/
	unsigned char	repark_code[6];						/*　Parking code (50H,41H,4BH)　　　　　　　　*/
	unsigned char	repark_rsv1[4];						/*　Reserve (20H-all) 　　    　　　　　　　　*/
	unsigned char	repark_kno[6];						/*　Kakari-inn Number (Ascii code)  　　　　　*/
	unsigned char	repark_rsv2[4];						/*　Reserve (20H-all) 　　    　　　　　　　　*/
	unsigned char	repark_year[2];						/*　Year  (Ascii code)        　　　　　　  　*/
	unsigned char	repark_mon[2];						/*　Month (Ascii code)　　　　　　　　　　　　*/
	unsigned char	repark_day[2];						/*　Day 　(Ascii code)　　　　　　　　　　　　*/
	unsigned char	repark_rsv3[43];					/*　Reserve (20H-all) 　　    　　　　　　　　*/
} m_repark;												/*　　　　　　　　　　　　　　　　　　　　　　*/
/*	クレジット機能付きTカードフォーマット							*/
typedef struct m_Tcard_rec {
	unsigned char	mark[2];							/*　IDマーク & 業態マーク 　　　*/
	unsigned char	ansyou[4];							/*　暗証番号　　　　　　　　　　*/
	unsigned char	company[4];							/*　会社番号　　　　　　　　　　*/
	unsigned char	cre_mem_no[16];						/*　クレジット会員番号　　　　　*/
	unsigned char	data1;								/*　未使用　　　　　　　　　　　*/
	unsigned char	field;								/*　任意フィールド(法人カード用)*/
	unsigned char	data2;								/*　未使用　　　　　　　　　　　*/
	unsigned char	credit_data[10];					/*　クレジット情報　　　　　　　*/
	unsigned char	credit_limit[4];					/*　クレジットカード有効期限　　*/
	unsigned char	Tcard_no[17];						/*　Tカード会員番号 　　　　　　*/
	unsigned char	Ptr2_Chk;							/*　提携先コードチェック有無　　*/
} m_Tcard;

typedef	struct	m_magformat_rec {						/*　　　　　　　　　　　　　　　　　　　　　　*/
	unsigned char	type;								/*　０：APSフォーマット　１：GTフォーマット　　*/
	unsigned char	ex_pkno;							/*　GTフォーマット拡張した駐車場No格納P10-P17　*/
}m_magformat;

// MH322914 (s) kasiyama 2016/07/15 AI-V対応
typedef	struct	m_gtticstp_rec {						/*　駐車券　　　　　　　　　　　　　　　　　　*/
	m_magformat	magformat;								/*　GTフォーマット情報　　　　　　　　　　　　*/
	m_ticstp	ticstp;
}m_gtticstp;
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

typedef	struct	m_gtapspas_rec {						/*　定期券　　　　　　　　　　　　　　　　　　*/
	m_magformat	magformat;								/*　GTフォーマット情報　　　　　　　　　　　　*/
	m_apspas	apspas;									/*　APSフォーマット情報 　　　　　　　　　　　*/
}m_gtapspas;

typedef	struct	m_gtservic_rec {						/*　サービス券　　　　　　　　　　　　　　　　*/
	m_magformat	magformat;								/*　GTフォーマット情報　　　　　　　　　　　　*/
	m_servic	servic;									/*　APSフォーマット情報 　　　　　　　　　　　*/
}m_gtservic;

typedef	struct	m_gtprepid_rec {						/*　プリペイド　　　　　　　　　　　　　　　　*/
	m_magformat	magformat;								/*　GTフォーマット情報　　　　　　　　　　　　*/
	m_prepid	prepid;									/*　APSフォーマット情報 　　　　　　　　　　　*/
}m_gtprepid;

typedef	struct	m_gtkaisuutik_rec {						/*　回数券　　　　　　　　　　　　　　　　　　*/
	m_magformat	magformat;								/*　GTフォーマット情報　　　　　　　　　　　　*/
	m_kaisuutik	kaisuutik;								/*　APSフォーマット情報 　　　　　　　　　　　*/
}m_gtkaisuutik;

extern	unsigned short	Prm_RateDiscTime[12];			/*											*/
extern	unsigned char	SP_DAY[][3];					/*											*/
extern	unsigned char	SP_RANGE[][4];					/*											*/
extern	unsigned char	SP_HAPPY[][2];					/*											*/
extern	unsigned char	SP_YEAR[][3];					/*　特別年月日		    					*/
extern	unsigned char	SP_WEEK[][3];					/*　特別曜日		    					*/
extern	char			jitu_wari;						/* 実割引									*/
extern	char			turi_wari;						/* 割引の釣銭対象							*/

extern	uchar	e_inflg;								/*　今回の入金は「電子マネー」1:である0:ない  */
extern	ushort	e_incnt;								/*　精算内の「電子マネー」使用回数（含む中止）*/
extern	long	e_pay;									/*　電子マネー使用額	　　　　　　　　　　*/
extern	long	e_zandaka;								/*　電子マネー残高		　　　　　　　　　　*/

extern	unsigned long	KaisuuWaribikiGoukei;			/* 回数券割引額（合計）						*/
extern	unsigned char	KaisuuAfterDosuu;				/* 回数券 利用度数							*/
extern	unsigned char	KaisuuUseDosuu;					/* 回数券 利用度数 							*/

extern	unsigned char	fus_subcal_flg;

extern	char	h24t_init_flg;							// h24time初期化フラグ

extern	uchar	nmax_countup;							// Ｎ時間最大のカウント方法
extern	uchar	pass_week_chg;							// 定期帯の曜日切換基準
extern	uchar	nmax_point;								// Ｎ時間最大の基準時刻
extern	uchar	nmax_itei;								// Ｎ時間最大の一定料金帯の扱い
extern	uchar	nmax_pass_reset;						// Ｎ時間最大の定期帯後の基準時刻リセット(入庫基準のみ有効)
extern	uchar	nmax_pass_calc;							// Ｎ時間最大の定期帯後の体系
extern	uchar	nmax_tim_disc;							// Ｎ時間最大の時間割引後の入庫時刻
extern	short	in_calcnt;								// Ｎ時間最大用カウンター
extern	short	nmax_cnt;								// Ｎ時間最大料金徴収カウント
extern	short	nmax_amari;								// Ｎ時間最大経過時の単位時間の余り時間
extern	char	tik_syubet;								// 定期種別(時間帯定期後に車種切換すると定期帯が消える不具合対策)
extern	uchar	nmax_tani_type;							// Ｎ時間経過時の単位時間処理
extern	char	n_itei_adj;								// 一定料金後基準時刻調整
extern	short	nmax_set1;								// 
extern	char	ntnet_nmax_flg;							// 

extern	char	sp_mx_exp[12];							// 料金種別毎の最大料金タイプ設定
extern	short	EXP_NMAX_HOUR1[12];						// ｎ時間最大１の最大時間（外）
extern	short	EXP_NMAX_HOUR2[12];						// ｎ時間最大２の最大時間（内）
extern	short	exp_nmax_set1[12];						// ｎ時間最大１の徴収回数
extern	uchar	EXP_FeeCal;								// 料金種別毎最大料金の有無
extern	short	NMAX_HOUR2;								// ｎ時間最大２の最大時間（内）
extern	short	nmax_amari2;							// ｎ２の余り時間
extern	uchar	pass_tm_flg;							// 時間帯定期関数からのコールフラグ
// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
//extern	uchar	f_sousai;								// 相殺割引
//// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
// MH810105(S) MH364301 QRコード決済対応
extern	uchar	q_inflg;								/*　今回の入金は「QRコード」1:である 0:ない  */
// MH810105(E) MH364301 QRコード決済対応

#endif	/* ___RKN_CALH___ */
