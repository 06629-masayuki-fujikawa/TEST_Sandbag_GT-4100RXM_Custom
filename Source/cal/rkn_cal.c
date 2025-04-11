/*[]----------------------------------------------------------------------[]*/
/*| Rkn_cal.h                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.HARA                                                   |*/
/*| Date        : 2001-11-22                                               |*/
/*| UpDate      : 2004-05-19 T.Nakayama                                    |*/
/*|             : NT4500MK2 拡張ｻｰﾋﾞｽ券追加                                |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
																		/*　平年及び閏年の１月～１２月の日ごとの日数　*/
																		/*　平年及び閏年の１月～１２月の日ごとの日数　*/
																		/*　平年及び閏年の１月～１２月の日ごとの日数　*/
char	monday[2][12] = {{ 31,28,31,30 ,								/*　１月，２月，３月，４月　　　　　　　　　　*/
						   31,30,31,31 ,								/*　５月，６月，７月，８月　　　　　　　　　　*/
						   30,31,30,31 },								/*　９月　１０月，１１月，１２月　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						 { 31,29,31,30 ,								/*　１月，２月，３月，４月　　　　　　　　　　*/
						   31,30,31,31 ,								/*　５月，６月，７月，８月　　　　　　　　　　*/
						   30,31,30,31 }};								/*　９月　１０月，１１月，１２月　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　平年及び閏年の１月１日からの日数　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	cal_mon[2][12] = {{   0,  31,  59,  90, 120, 151,				/*　１月，２月，３月，４月、５月、６月　　　　*/
							181, 212, 243, 273, 304, 334 },				/*　７月，８月，９月，１０月、１１月、１２月　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						  {   0,  31,  60,  91, 121, 152,				/*　１月，２月，３月，４月、５月、６月　　　　*/
							182, 213, 244, 274, 305, 335 }};			/*　７月，８月，９月，１０月、１１月、１２月　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	TSN_PRP		tsn_prp;											/*　　　　　　　　　　　　　　　　　　　　　　*/
unsigned long	prp_rno_buf[7];											/*　プリペイド追番（カードＮｏ．）バッファ　　*/
char	prp_rno_cnt;													/*　プリペイド追番（カードＮｏ．）カウント　　*/
struct	VL_SVS		vl_svs;												/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	VL_GGS		vl_ggs;
struct	VL_FRS		vl_frs;
// MH810100(S)
struct	VL_CAR_NO	vl_car_no;
// MH810100(E)
char	vl_now;															/*　現在ＶＬカードデータ　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	cr_dat_n;														/*　カードリーダデータ(ＩＤ)　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	syashu;															/*車種　　　　　　　　　　　　　　　　　　　　*/
char	sitei_flg;														/*指定時刻日毎最大　フラグ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	rysyasu;														/*料金計算キー車種　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	CHK_TIM		chk_tim;											/*　ＩＮ＋２４時チェック用　　　　　　　　　　*/
short	chk_time;														/*　　　　　”　　　　　　　　　　　　　　　　*/
short	rmax_cnt;														/*　最大料金到達回数　　　　　　　　　　　　　*/
short	rmax_ptn;														/*　最大料金到達時パタン　　　　　　　　　　　*/
short	rmax_pcn;														/*　最大料金到達時単位係数　　　　　　　　　　*/
short	rmax_cyc;														/*　最大料金到達時パタン繰り返し回数　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	sp_mx_1;														/*　IN+24最大時cal_time調整フラグ 　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short 	wk_strt;														/*指定時刻最大check時刻に加算する時間　　　　 */
short	last_bnd;														/*　次体系突入直前の料金帯　　　　　　　　　　*/
char	et_dbl;															/*　重複帯ﾌﾗｸﾞ OFF:なし ON:重復0で内出庫　　　*/
																		/*　　　　　　 NOF:重複1（重複0で帯内入出庫） */
char	et_jiot;														/*　重複帯内入出庫ﾌﾗｸﾞ　NOF 　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	REQ_RKN		req_rkn;											/*　料金要求テーブル　　　　　　　　　　　　　*/
  																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	REQ_CRD		req_crd[WAR_CRD_MAX];								/*　料金要求テーブル　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	req_crd_cnt;													/*　駐車券読み取り時券データ要求件数　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	ryoukin;														/*　現在料金　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	discount;														/*　割引額　　　　　　　　　　　　　　　　　　*/
long	tol_dis;														/*　割引額合計　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	discount_tyushi;												/*　割引額　　　　　　　　　　　　　　　　　　*/
																		/*                                            */
struct	REQ_TKI		req_tki[20];										/*　定期処理後格納エリア(ＭＡＸ２０件)　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	tki_ken;														/*　定期処理後料金計算要求件数　　　　　　　　*/
char	tki_flg					= OFF	;								/*　定期処理後ｆｌｇ　　　　　　　　　　　　　*/
short	ps_type[TKI_MAX];												/*　定期使用目的　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		// 
short	ta_st[9];														/*　体系開始時刻　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	ps_st[9][TKI_MAX];												/*　定期開始時刻　　　　　　　　　　　　　　　*/
short	ps_ed[9][TKI_MAX];												/*　定期終了時刻　　　　　　　　　　　　　　　*/
																		// 
char	ps_sepa[9][TKI_MAX];											// 定期帯分割フラグ 0＝なし/1＝あり
short	ps_st1[9][TKI_MAX];												// 定期開始時刻(分割1)
short	ps_ed1[9][TKI_MAX];												// 定期終了時刻(分割1)
short	ps_tm1[9][TKI_MAX];												// 定期時間(分割1)
short	ps_st2[9][TKI_MAX];												// 定期開始時刻(分割2)
short	ps_ed2[9][TKI_MAX];												// 定期終了時刻(分割2)
short	ps_tm2[9][TKI_MAX];												// 定期時間(分割2)
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	ji_st[9];														/*　時間割引の有効開始時刻　　　　　　　　　　*/
short	ji_ed[9];														/*　時間割引の有効終了時刻　　　　　　　　　　*/
long	jik_dtm;														/*　時間割引処理用割引時間　　　　　　　　　　*/
char	jik_flg					= NG	;								/*　時間割引処理ｆｌｇ　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	base_ryo;														/*　駐車料金(基本型)　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	prc_no;															/*　金額切上げ切捨て処理方法　　　　　　　　　*/
short	prc_dat;														/*　金額切上げ切捨て処理内容　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	rag_flg					= OFF	;								/*　ラグタイムｆｌｇ　　　　　　　　　　　　　*/
char	re_req_flg				= OFF	;								/*　再要求フラグ　　　　　　　　　　　　　　　*/
char	iti_flg					= OFF	;								/*　一定料金帯フラグ　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	p_taxflg[4];													/*プリペイド税金処理ｆｌｇ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short 	ptn_ccnt;														/*　現在単位係数カウンタ　　　　　　　　　　　*/
short 	cycl_ccnt;														/*　現在繰り返し回数カウンタ　　　　　　　　　*/
char	befr_style;														/*　前日の体系　　　　　　　　　　　　　　　　*/
char	befr_flag;														/*　前日の体系フラグ　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	su_jik_dtm;														/*　種別割引時間　　　　　　　　　　　　　　　*/
long	mae_dis;														/*　旧時間割引額　　　　　　　　　　　　　　　*/
long	sn_stp_pzan;													/*　精算中止券Ｐ残額　　　　　　　　　　　　　*/
long	tyushi_mae_dis;													/*                                            */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	ctyp_flg;														/*　計算方法フラグ　　　　　　　　　　　　　　*/
long	svcd_dat;														/*　サービス券フラグ　　　　　　　　　　　　　*/
char	base_dis_flg;													/*　ベース料金全額割引フラグ　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	sei_type;														/*　精算タイプ　０：通常　１：再精算　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	c_pay;															/*　プリペイドカード使用額　　　　　　　　　　*/
char	tkv_flg					= OK	;								/*　定期有効無効フラグ　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	fee_std[9][12][6];												/*　基本料金[体系][車種][帯]　　　　　　　　　*/
short	fee_exp[9][12][6];												/*　追加料金[体系][車種][帯]　　　　　　　　　*/
short	bnd_strt[9][13];												/*　料金帯開始時刻[体系][帯]　　　　　　　　　*/
char	ovl_type[9][6];													/*　重複方式[体系][帯]　　　　　　　　　　　　*/
short	ovl_time[9][6];													/*　重複時間[体系][帯]　　　　　　　　　　　　*/
short	leng_std[9][6];													/*　基本時間[体系][帯]　　　　　　　　　　　　*/
short	leng_exp[9][6];													/*　追加時間[体系][帯]　　　　　　　　　　　　*/
short	max_bnd[9];														/*　体系で使用する料金帯数[体系]　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	fee_midnig[3][12];												/*　深夜料金[体系][車種]　　　　　　　　　　　*/
short	fee[3][12][12];													/*　パターン単位料金[体系][車種][パターン]　　*/
short	teigen_strt[3];													/*　逓減帯開始時刻[体系]　　　　　　　　　　　*/
short	teigen_ovl[3];													/*　逓減帯重複時刻[体系]　　　　　　　　　　　*/
short	midnig_strt[3];													/*　深夜帯開始時刻[体系]　　　　　　　　　　　*/
short	midnig_ovl[3];													/*　深夜帯重複時刻[体系]　　　　　　　　　　　*/
short	style_type[3];													/*　料金体系使用の有無[体系]　　　　　　　　　*/
char	cycl_frst[3];													/*　繰り返し開始パターン[体系]　　　　　　　　*/
char	cycl_last[3];													/*　繰り返し終了パターン[体系]　　　　　　　　*/
short 	cycl_cnt[3];													/*　繰り返し回数[体系]　　　　　　　　　　　　*/
short	ptn_leng[3][12];												/*　パターン単位時間[体系][パターン]　　　　　*/
short	ptn_cycl[3][12];												/*　パターン単位係数[体系][パターン]　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	abs_max[9][12];													/*　絶対最大料金[体系][車種]　　　　　　　　　*/
long	day_max1[9][12];												/*　日毎最大料金１[体系][車種]　　　　　　　　*/
long	day_max2[9][12];												/*　日毎最大料金２[体系][車種]　　　　　　　　*/
long	band_max[6][9][12];												/*　時間帯別最大料金（料金体別）　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	total;															/*　駐車料金集計用変数　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	tb_number;														/*　計算結果格納テーブル通し番号　　　　　　　*/
short	cal_type;														/*　料金計算方式　　　　　　　　　　　　　　　*/
short	cal_time;														/*　計算対象時刻　　　　　　　　　　　　　　　*/
short	in_time;														/*　入庫時刻　　　　　　　　　　　　　　　　　*/
short	ot_time;														/*　出庫時刻　　　　　　　　　　　　　　　　　*/
char	style;															/*　料金体系　　　　　　　　　　　　　　　　　*/
char	band;															/*　料金帯　　　　　　　　　　　　　　　　　　*/
char	car_type;														/*　車種　　　　　　　　　　　　　　　　　　　*/
short	cons_tax_ratio;													/*　消費税率　　　　　　　　　　　　　　　　　*/
short	husoku_flg;														/*　不足額に対するお釣り計算を表すフラグ　　　*/
char	next_style;														/*　次の料金体系　　　　　　　　　　　　　　　*/
short	next_style_strt;												/*　次の料金体系開始時刻　　　　　　　　　　　*/
short	strt_ptn;														/*　定期帯入車後の開始逓減パターン　　　　　　*/
short	dis_tax;														/*　割引額の消費税計算　　　　　　　　　　　　*/
int		bnd_end_tm;														/*　料金帯終了時刻　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	maxmonth[13]		= {0,31,28,31,30,31,30,31,31,30,31,30,31};	/*各月の最大日設定　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	sp_op;															/*▲余り処理方式　　　　　　　　　　　　　　　*/
char	sp_it;															/*▲一定料金帯余り方式　　　　　　　　　　　　*/
char	sp_mx;															/*▲日毎最大料金方式　　　　　　　　　　　　　*/
short	h24time;														/*▲入庫からトータル時分　　　　　　　　　　　*/
short	wk_h24t;														/*▲計算時分　　　　　　　　　　　　　　　　　*/
short	amartim;														/*▲一定料金帯余り時間　　　　　　　　　　　　*/
short	amr_flg;														/*▲一定料金帯余りフラグ　　　　　　　　　　　*/
short	NTIME;															/*　日毎最大適用時間　　　　　　　　　　　　　*/
short	koetim;															/*　Ｎ時間超過分の時間			　　　　　　　*/
long	chk_max1;														/*▲日毎最大料金　　　　　　　　　　　　　　　*/
long	chk_max2;														// 2種類目のｎ時間最大料金
long	nyukin;															/*入金金額　　　　　　　　　　　　　　　　　　*/
long	turi;															/*釣り銭金額　　　　　　　　　　　　　　　　　*/
char 	n_hour_style;													/*　ｎ時間最大の最大料金を取る料金体系　　　　*/
int  	n_hour_cnt;														/*　ｎ時間最大ｍ回の適用回数　　　　　　　　　*/
short	T_N_HOUR;														/*　ｎ時間最大の最大時間　　　　　　　　　　　*/
char 	nmax_style;														// ｎ時間最大の最大料金を取る料金体系
short	NMAX_HOUR1;														// ｎ時間最大1（全種別共通）
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	tt_tim;															/*　定期帯時間　　　　　　　　　　　　　　　　*/
long	tt_ryo;															/*　最大料金用料金　　　　　　　　　　　　　　*/
long	tt_x;															/*　時刻指定用ｔｏｔａｌ　ｗｏｒｋ　　　　　　*/
long	tt_y;															/*　時刻指定用ｒｙｏｕｋｉｎ　ｗｏｒｋ　　　　*/
long	c_pay_afr;														/*　プリペイドカード使用後料金　　　　　　　　*/
char	percent;														/*　％割引率　　　　　　　　　　　　　　　　　*/
long	jmax_ryo;														// ２種日毎最大用Total Work
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	tki_syu;														/*　定期帯料金計算の為　　　　　　　　　　　　*/
long	azukari;														/*	駐車券ﾃﾞｰﾀ預り金額       				  */
char 	su_jik_plus;													/*　定期種別割引時間プラスｆｌｇ　　　　　　　*/
struct	REQ_TKC		req_tkc[5];											/*　定期券中止精算要求格納バッファ(MAX 5件)	  */
char	req_tkc_cnt;													/*　定期券中止精算要求納件数		　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	HZUK	hzuk;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	JIKN	jikn;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	VER		version;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	SVT		se_svt;													/*　サービスタイムテーブル　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	crd_inf	CRD_DAT;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
uchar	wrcnt_sub = 0;

uchar	e_inflg;														/*　今回の入金は「電子マネー」1:である0:ない  */
ushort	e_incnt;														/*　精算内の「電子マネー」使用回数（含む中止）*/
long	e_pay;															/*　電子マネー使用額						  */
long	e_zandaka;														/*　電子マネー残高							  */
// MH810105(S) MH364301 QRコード決済対応
uchar	q_inflg;														/*　今回の入金は「QRコード」1:である0:ない 	  */
// MH810105(E) MH364301 QRコード決済対応

/*--------------------------------------------------------------------------------------------------------------------*/
unsigned short	Prm_RateDiscTime[12];									/*											  */
unsigned char	SP_DAY[SP_DAY_MAX][3];									/*											  */
unsigned char	SP_RANGE[SP_RANGE_MAX][4];								/*											  */
unsigned char	SP_HAPPY[SP_HAPPY_MAX][2];								/*											  */
unsigned char 	SP_YEAR[SP_YAER_MAX][3];								/*　特別年月日								  */
unsigned char 	SP_WEEK[SP_WEEK_MAX][3];								/*　特別曜日								  */
char			jitu_wari;												/* 実割引									  */
char			turi_wari;												/* 割引の釣銭対象							  */
unsigned long	KaisuuWaribikiGoukei;									/* 回数券割引額（合計）							*/
unsigned char	KaisuuAfterDosuu;										/* 回数券 利用度数								*/
unsigned char	KaisuuUseDosuu;											/* 回数券 利用度数 								*/
unsigned char	fus_subcal_flg;
																		// 
uchar	nmax_countup;													// Ｎ時間最大のカウント方法
																		// 
uchar	pass_week_chg;													// 定期帯の曜日切換基準
uchar	nmax_point;														// Ｎ時間最大の基準時刻
uchar	nmax_itei;														// Ｎ時間最大の一定料金帯の扱い
uchar	nmax_pass_reset;												// Ｎ時間最大の定期帯後の基準時刻リセット
uchar	nmax_pass_calc;													// Ｎ時間最大の定期帯後の体系
uchar	nmax_tim_disc;													// Ｎ時間最大の時間割引後の入庫時刻
short	in_calcnt;														// Ｎ時間最大用カウンター
short	nmax_cnt;														// Ｎ時間最大料金徴収カウント
short	nmax_amari;														// Ｎ時間最大経過時の単位時間の余り時間
																		// 
uchar	nmax_tani_type;													// Ｎ時間経過時の単位時間処理
char	n_itei_adj;														// 一定料金後基準時刻調整
short	nmax_set1;														// ｎ時間最大1徴収回数（全種別共通）
																		// 
char	tik_syubet;														// 定期種別(時間帯定期後に車種切換すると
																		// 
char	sp_mx_exp[12];													// 料金種別毎の最大料金タイプ設定
short	EXP_NMAX_HOUR1[12];												// ｎ時間最大1（種別毎）
short	EXP_NMAX_HOUR2[12];												// ｎ時間最大2（種別毎）
short	exp_nmax_set1[12];												// ｎ時間最大1の徴収回数（種別毎）
uchar	EXP_FeeCal;														// 拡張料金計算の有無
short	NMAX_HOUR2;														// ｎ時間最大2（全種別共通）
short	nmax_amari2;													// ｎ時間最大2の余り時間
																		// 
uchar	pass_tm_flg;													// 

// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
//unsigned char	f_sousai;												// 相殺割引
//// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
struct	VL_FRS		vl_frs;
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)