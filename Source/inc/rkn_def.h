/*[]----------------------------------------------------------------------[]*/
/*| Rkn_def.h                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.HARA                                                   |*/
/*| Date        : 2001-11-22                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#ifndef _RKN_DEF_H_
#define _RKN_DEF_H_
												/*							*/
#define		TKI_MAX			15					/* 定期種別					*/
#define		SKI_MAX			3					/* サービス券期限変更		*/
#define		SVS_MAX			15					/* サービス券種最大数		*/
												/*							*/
#define		V_CHM			24					/* 駐車券　精算前			*/
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
#define		V_FUN			25					/* 紛失券　　　　　　　　　 */
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
#define		V_CHG			27					/* 駐車券　精算後			*/
#define		V_CHS			28					/* 中止券					*/
#define		V_CHU			29					/* 掛売券					*/
#define		V_KAM			30					/* 回数券　無料				*/
#define		V_KAG			31					/* 回数券　減額				*/
#define		V_SAK			32					/* サービス券				*/
#define		V_SYU			33					/* 種別切換　定期券　サービス券	*/
#define		V_PRI			34					/* プリペイドカード			*/
#define		V_TSC			35					/* 定期券（駐車券無し）		*/
#define		V_TST			42					/* 定期券（駐車券併用）		*/
#define		V_CRE			43					/* クレジットカード */
#define		V_EMY			50					/* 電子マネー				*/
#define		V_GNG			51					/* 減額精算 				*/
#define		V_FRK			52					/* 振替精算 				*/
#define		V_SSS			53					/* 修正精算					*/
// MH810105(S) MH364301 QRコード決済対応
#define		V_EQR			54					/* QRコード 				*/
// MH810105(E) MH364301 QRコード決済対応
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
#define		V_DIS_FEE		70					// 遠隔精算中割引(金額指定)
#define		V_DIS_TIM		71					// 遠隔精算中割引(入庫時刻指定)
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
												/*							*/
/* カードリーダーコマンド	*/
#define		RID_JIS			0x41				/* ＪＩＳ定期券（ＡＴＭ標準定期）	*/
#define		RID_CKM			0x24				/* 駐車券（精算前）			*/
#define		RID_CKG			0x25				/* 駐車券（精算後）			*/
#define		RID_CKS			0x26				/* 駐車券（精算機における精算中止）	*/
#define		RID_CKU			0x27				/* 駐車券（掛売りライタにて掛売りされた券）	*/
#define		RID_CKT			0x28				/* 駐車券（再精算の精算中止券）	*/
#define		RID_CKZ			0x29				/* 駐車券（使用済券　設定4=2で精算済みの券）	*/
#define		RID_SAI			0x2a				/* 再発行券					*/
#define		RID_FUN			0x2b				/* 紛失券（テスト券）		*/
#define		RID_KAI			0x2c				/* 回数券					*/
#define		RID_SAV			0x2d				/* サービス券				*/
#define		RID_APS			0x1a				/* ＡＰＳ定期券				*/
#define		RID_RCM			0x34				/* 立駐駐車券（精算前）		*/
#define		RID_RCG			0x35				/* 立駐駐車券（精算後）		*/
#define		RID_RCS			0x36				/* 立駐駐車券（精算機における精算中止）	*/
#define		RID_RCU			0x37				/* 立駐駐車券（掛売りライタにて掛売り券）	*/
#define		RID_RCT			0x38				/* 立駐駐車券（再精算の精算中止券）	*/
												/*							*/
/* 料金計算デファイン		*/
#define		RY_TSA			0					/* 入車時刻からの料金計算	*/
#define		RY_OTR			1					/* おつり計算				*/
#define		RY_TWR			2					/* 時間割引処理				*/
#define		RY_RWR			3					/* 料金割引処理				*/
#define		RY_FRE			4					/* 回数券処理				*/
#define		RY_SKC			5					/* サービス券処理			*/
#define		RY_PCO			6					/* プリペイドカード処理		*/
#define		RY_PKC			7					/* 定期券処理				*/
#define		RY_FKN			8					/* 紛失券処理				*/
#define		RY_BNC			9					/* 部門加算処理				*/
#define		RY_TAT			10					/* 税キー処理				*/
#define		RY_SNC			11					/* 店Ｎｏ割引処理			*/
#define		RY_XXX			12					/* 積算処理					*/
#define		RY_PWR			13					/* ％割引処理				*/
#define		RY_KRY			14					/* 固定料金処理				*/
#define		RY_QUA			15					/* 保証金処理				*/
#define		RY_RAY			16					/* 払い戻しキー				*/
#define		RY_KCH			17					/* 車種切り替え				*/
#define		RY_RAG			18					/* ラグタイム処理			*/
#define		RY_KGK			19					/* 料金差引割引処理			*/
#define		RY_CSK			20					/* 精算中止サービス券処理	*/
#define		RY_CPP			21					/* 精算中止プリペイドカード処理	*/
#define		RY_CMI			22					/* 精算中止店Ｎｏ割引処理		*/
#define		RY_KAK			23					/* 掛売券処理				*/
#define		RY_FRE_K		24					/* 回数券処理(拡張駐車NO.)	*/
#define		RY_SKC_K		25					/* サービス券処理(拡張駐車NO.)	*/
#define		RY_PCO_K		26					/* プリペイドカード処理(拡張駐車NO.)	*/
#define		RY_PKC_K		27					/* 定期券処理(拡張駐車NO.)	*/
#define		RY_SKC_K2		28					/* サービス券処理(拡張2駐車NO.)	*/
#define		RY_SKC_K3		29					/* サービス券処理(拡張3駐車NO.)	*/
#define		RY_KAK_K		30					/* 掛売券処理(拡張駐車NO.)	*/
#define		RY_KAK_K2		31					/* 掛売券処理(拡張2駐車NO.)	*/
#define		RY_KAK_K3		32					/* 掛売券処理(拡張3駐車NO.)	*/
#define		RY_WBC			33					/* 割引券処理				*/
#define		RY_WBC_K		34					/* 割引券処理(拡張駐車NO.)	*/
#define		RY_WBC_K2		35					/* 割引券処理(拡張2駐車NO.)	*/
#define		RY_WBC_K3		36					/* 割引券処理(拡張3駐車NO.)	*/
#define		RY_PCO_K2		37					/* プリペイドカード処理(拡張2駐車NO.)	*/
#define		RY_PCO_K3		38					/* プリペイドカード処理(拡張3駐車NO.)	*/
#define		RY_FRE_K2		39					/* 回数券処理(拡張2駐車NO.)	*/
#define		RY_FRE_K3		40					/* 回数券処理(拡張3駐車NO.)	*/
#define		RY_PKC_K2		41					/* 定期券処理(拡張2駐車NO.)	*/
#define		RY_PKC_K3		42					/* 定期券処理(拡張3駐車NO.)	*/
#define		RY_CRE			43					/* クレジットカード */
#define		RY_EMY			44					/* 電子マネー処理				*/
#define		RY_GNG			45					/* 減額精算 				*/
#define		RY_FRK			46					/* 振替精算 				*/
#define		RY_SSS			47					/* 修正精算					*/
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
#define		RY_GNG_FEE		48					// 遠隔精算(金額指定)減額処理
#define		RY_GNG_TIM		49					// 遠隔精算(入庫時刻指定)減額処理
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
// MH810105(S) MH364301 QRコード決済対応
#define		RY_EQR			51					/* QRコード処理				*/
// MH810105(E) MH364301 QRコード決済対応
												/*							*/
/* シスデファイン			*/
#define		LOOP			1					/* 永久ループ有り			*/
#define		NG				0xff				/* ＮＧ						*/
#define		OK				0x00				/* ＯＫ						*/
#define		OFF				0					/* ＯＦＦ					*/
#define		ON				1					/* ＯＮ						*/
#define		NOF				2					/* ＯＮ／ＯＦＦ以外			*/
#define		T_TX_TIM		0x77				/* 定期後加算時間有り		*/
												/*							*/
/* 時間デファイン			*/
#define		T_DAY			1440				/* 一日分の分				*/

/* FCMAIN					*/
#define		wbcd2i(p)		(((char *)(p))[0]*10+((char *)(p))[1])	/* ﾜｰﾄﾞBCD値→整数値変換*/
#define		cardLOST		0					/* 紛失精算券				*/
#define		cardREPUB		1					/* 再発行券					*/
#define		cardTICKET		2					/* 駐車券					*/
#define		cardPASS		3					/* 定期券					*/
#define		cardPREPAY		4					/* ＰＰＣ					*/
#define		cardLIMITNUM	5					/* 回数券 					*/
#define		cardSERVICE		6					/* ｻｰﾋﾞｽ券					*/
#define 	cardANOTHER		10					/* 他使用不可券				*/
												/*							*/
#define		Cmarching		0					/* 判:挿入方向				*/
#define		Creadable		1					/* 判:読取不可				*/
#define		Cparknum		2					/* 判:駐車№				*/
#define		Caskhost		3					/* 判:CM問合せ有			*/
#define		Cillegal		4					/* 判:不正券				*/
#define		Coverend		5					/* 判:期限切れ				*/
#define		Cnearend		6					/* 判:期限切れ間近			*/
												/*							*/
#define		SP_DAY_MAX		31					/* 特別日					*/
#define		SP_RANGE_MAX	3					/* 特別期間					*/
#define		SP_HAPPY_MAX	12					/* ﾊｯﾋﾟｰﾏﾝﾃﾞｰ				*/
#define		SP_YAER_MAX		6					/* 特別年月日					*/
#define		SP_WEEK_MAX		12					/* 特別曜日					*/


#define		MACRO_WARIBIKI_GO_RYOUKIN	((wk_ryo < (c_pay+e_pay)) ? 0L:(wk_ryo - c_pay - e_pay ))	/* 割引後 駐車料金にプリペ分を加算する */

#define		NMAX_OVER		999990					// Ｎ時間最大Ｍ回経過後の最大料金
													// 
#define		PASS_WEEK_TAIKEI	0					// 体系切換(定期帯途中で曜日切換なし)
#define		PASS_WEEK_DAY_SP	1					// 暦(0:00)切換(定期帯途中で曜日切換あり)
#define		PASS_WEEK_DAY_NO	2					// 暦(0:00)切換(定期帯途中で曜日切換なし)
													// 
#define		NMAX_OUT_PIT		0					// 出庫基準
#define		NMAX_IN_PIT			1					// 入庫基準
#define		NMAX_TAI_PIT		2					// 体系基準

#define		PAS_TIME			1					// 時間帯定期
#define		PAS_DISC			2					// 割引時間定期
#define		PAS_CANG			3					// 車種切換定期
#define		FEE_TAIKEI_MAX		9					// 料金体系数
#define		RYO_CAL_EXTEND		(((char)CPrmSS[S_CLX][1]) == 1)
#define		SYUBET_MAX		12			// 種別の数(Ａ～Ｌ)
#define 	RYO_TAIKEI_SETCNT	300		// １料金体系の設定数
#define 	RYO_SYUBET_SETCNT	20		// １種別の設定数

/* 最大料金タイプの設定値をdefineで定義する。 */
/* 以前の直値をdefineに置き換えることはしないので、検索時など注意すること */
#define		SP_MX_NON			0		// 日毎最大料金なし
#define		SP_MX_INTIME		1		// 入庫時刻最大(6/12/24)
#define		SP_MX_TIME			2		// 時刻指定日毎最大
#define		SP_MX_WTIME			3		// ２種類の時刻指定日毎最大
#define		SP_MX_N_HOUR		4		// ｎ時間ｍ回最大
#define		SP_MX_BAND			5		// 時間帯上限（料金帯上限）
#define		SP_MX_N_MH_NEW		10		// ｎ時間ｍ回最大
#define		SP_MX_N_MHOUR_W		20		// ２種類のｎ時間ｍ回最大
#define		SP_MX_612HOUR		9		// 6h,12,Max 
#endif	// _RKN_DEF_H_
