/*[]----------------------------------------------------------------------[]*/
/*| ﾒｯｾｰｼﾞ定義部                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-01-28                                               |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

/*--------------------------------------------------------------------------*/
/*	ｲﾍﾞﾝﾄﾏｽｸ定義															*/
/*--------------------------------------------------------------------------*/
#define		MSGGETLOW(msg)			msg&0xff/* ﾒｯｾｰｼﾞの下位を取得			*/
#define		MSGGETHIGH(msg)			msg>>8	/* ﾒｯｾｰｼﾞの上位を取得			*/

/*--------------------------------------------------------------------------*/
/*	ｲﾍﾞﾝﾄ番号定義															*/
/*--------------------------------------------------------------------------*/

	/*----------------------------------------------------------------------*/
	/*	Tenkey ﾃﾝｷｰ															*/
	/*----------------------------------------------------------------------*/
#define		KEY_TEN					0x0130	/* Ten Key event				*/
#define		KEY_TEN0				0x0130	/* Ten Key[0] event				*/
#define		KEY_TEN1				0x0131	/* Ten Key[1] event				*/
#define		KEY_TEN2				0x0132	/* Ten Key[2] event				*/
#define		KEY_TEN3				0x0133	/* Ten Key[3] event				*/
#define		KEY_TEN4				0x0134	/* Ten Key[4] event				*/
#define		KEY_TEN5				0x0135	/* Ten Key[5] event				*/
#define		KEY_TEN6				0x0136	/* Ten Key[6] event				*/
#define		KEY_TEN7				0x0137	/* Ten Key[7] event				*/
#define		KEY_TEN8				0x0138	/* Ten Key[8] event				*/
#define		KEY_TEN9				0x0139	/* Ten Key[9] event				*/
#define		KEY_TEN_F1				0x013a	/* Ten Key[F1](精算) event		*/
#define		KEY_TEN_F2				0x013b	/* Ten Key[F2](領収書) event	*/
#define		KEY_TEN_F3				0x013c	/* Ten Key[F3](駐車証明書) event*/
#define		KEY_TEN_F4				0x013d	/* Ten Key[F4](取消) event		*/
#define		KEY_TEN_F5				0x013e	/* Ten Key[F5](登録) event		*/
#define		KEY_TEN_CL				0x013f	/* Ten Key[CL](C) event			*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Switch																*/
	/*----------------------------------------------------------------------*/
#define		KEY_RECIEPT				0x0111	/* 領収証ﾎﾞﾀﾝ event				*/
#define		KEY_CANCEL				0x0112	/* 精算中止ﾎﾞﾀﾝ event			*/
#define		KEY_MODECHG				0x0113	/* Mode change key(設定ﾓｰﾄﾞｽｲｯﾁ)*/
#define		KEY_DOOR1				0x0114	/* Door1 key event				*/
#define		KEY_DOOR2				0x0115	/* Door2 key event				*/
#define		KEY_COINSF				0x0116	/* Coin safe event				*/
#define		KEY_NOTESF				0x0117	/* Note safe event				*/
#define		KEY_MENTSW				0x0118	/* Mentenance SW event			*/
#define		KEY_DOORARM				0x0119	/* Door alarm event				*/
#define		KEY_YOBI1				0x011A	/* 予備(空き)					*/
#define		KEY_YOBI2				0x011B	/* 予備(空き)					*/
#define		KEY_YOBI3				0x011C	/* 予備(空き)					*/
#define		KEY_YOBI4				0x011D	/* 予備(空き)					*/
#define		KEY_VIBSNS				0x011E	/* 振動ｾﾝｻｰ event				*/
#define		KEY_PAYSTR				0x011F	/* 精算開始ﾎﾞﾀﾝ event			*/
#define		KEY_DIPSW1				0x0120	/* DIPSW1 event					*/
#define		KEY_DIPSW2				0x0121	/* DIPSW2 event					*/
#define		KEY_DIPSW3				0x0122	/* DIPSW3 event					*/
#define		KEY_DIPSW4				0x0123	/* DIPSW4 event					*/
#define		KEY_MANDET				0x0124	/* 人体検知ｾﾝｻｰ入力				*/
#define		KEY_CNDRSW				0x0125	/* ｺｲﾝﾒｯｸﾄﾞｱｽｲｯﾁ				*/
#define		KEY_RESSW1				0x0126	/* 予備ﾎﾞﾀﾝ1					*/
#define		KEY_RESSW2				0x0127	/* 予備ﾎﾞﾀﾝ2					*/
#define		KEY_NTFALL				0x0128	/* 紙幣ﾘｰﾀﾞｰ脱落検知ｽｲｯﾁ		*/
#define		KEY_DOORNOBU			0x0129	/* ﾄﾞｱﾉﾌﾞｲﾍﾞﾝﾄ					*/
#define		KEY_RTSW1				0x0140	/* RTSW1 event					*/
#define		KEY_RTSW2				0x0141	/* RTSW2 event					*/
#define		KEY_RTSW3				0x0142	/* RTSW3 event					*/
#define		KEY_RTSW4				0x0143	/* RTSW4 event					*/
#define		KEY_FANET				0x0144	/* FAN停止温度検出 event		*/
#define		KEY_FANST				0x0145	/* FAN駆動開始温度検出 event	*/
#define		KEY_MIN					0x0146	/* RXM-1基板予備入力 event		*/
#define		KEY_RXIIN				0x0147	/* RXI-1基板汎用入力 event		*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	RXF DIPSW		 													*/
	/*----------------------------------------------------------------------*/
#define		FNT_DIPSW1				0x0150	/* DIPSW1 event					*/
#define		FNT_DIPSW2				0x0151	/* DIPSW2 event					*/
#define		FNT_DIPSW3				0x0152	/* DIPSW3 event					*/
#define		FNT_DIPSW4				0x0153	/* DIPSW4 event					*/
#define		FNT_YOBI1				0x0154	/* 予備(空き)					*/
#define		FNT_YOBI2				0x0155	/* 予備(空き)					*/
#define		FNT_YOBI3				0x0156	/* 予備(空き)					*/
#define		FNT_YOBI4				0x0157	/* 予備(空き)					*/

	/*----------------------------------------------------------------------*/
	/*	ｼｮｰﾄｶｯﾄｷｰ															*/
	/*----------------------------------------------------------------------*/
// MH810100(S) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
//#define		CANCEL_EVENT			0x01f1	// 振替元の車が出庫
// MH810100(E) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）

	/*----------------------------------------------------------------------*/
	/*	Clock																*/
	/*----------------------------------------------------------------------*/
#define		CLOCK_CHG				0x0201	/* Clock change evet			*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Time Out															*/
	/*----------------------------------------------------------------------*/
#define		TIMEOUT					0x0300	/* Time out 					*/
#define		TIMEOUT1				0x0301	/* Time out No.1				*/
#define		TIMEOUT2				0x0302	/* Time out No.2				*/
#define		TIMEOUT3				0x0303	/* Time out No.3				*/
#define		TIMEOUT4				0x0304	/* Time out No.4				*/
#define		TIMEOUT5				0x0305	/* Time out No.5				*/
#define		TIMEOUT6				0x0306	/* Time out No.6				*/
#define		TIMEOUT7				0x0307	/* Time out No.7				*/
#define		TIMEOUT8				0x0308	/* Time out No.8				*/
#define		TIMEOUT9				0x0309	/* Time out No.9				*/
#define		TIMEOUT10				0x030a	/* Time out No.10				*/
#define		TIMEOUT11				0x030b	/* Time out No.11				*/
#define		TIMEOUT12				0x030c	/* Time out No.12				*/

/* 1:ｵﾍﾟﾚｰｼｮﾝ関連				*/
/* 2:ﾒｯｸ,領収証受付,ｴﾗｰ表示監視	*/
/* 3:磁気ﾘｰﾀﾞｰｲﾆｼｬﾙ監視			*/
/* 4:不正･強制監視,ｴﾗｰ表示監視	*/
/* 5:ドアノブ戻し忘れ防止タイマー	*/
/* 6:ﾒﾝﾃﾅﾝｽ,同時押,定期券問合せ	*/
/* 7:ﾒﾝﾃﾅﾝｽし,ｼｬｯﾀｰ開状態継続監視,読取ｴﾗｰ表示用	*/
/* 8:入金後戻り判定ﾀｲﾏｰ			*/
/* 9:クレジット(CCT)関連タイマー (応答TimeOut / 処理結果TimeOut / 'r'TimeOut) */
/* 10:サイクリック表示タイマー & Suicaｺﾏﾝﾄﾞ要求後のwaitﾀｲﾏｰ */
/* 11:Suica残高表示ﾀｲﾏｰ			*/
/* 12:Mifare書き込み用			*/
/* 13:ParkiPRO再送タイマー		*/
enum {
		TIMEOUT13 =	0x030d,			/* 13:ParkiPRO再送タイマー				*/
		TIMEOUT14,					/* Time out No.14				*/
		TIMEOUT15,					/* Time out No.15(未使用)		*/
		TIMEOUT16,
		TIMEOUT17,
		TIMEOUT18,					// ｼｬｯﾀｰ制御ﾌﾗｸﾞ監視用ﾀｲﾏｰ（他の機能との併用不可）
		TIMEOUT19,				/* 						*/
		TIMEOUT20,				/*	ｺﾝﾒｯｸ払い出し完了待ちﾀｲﾏｰ	*/
		TIMEOUT21,				/*	入出力信号ﾁｪｯｸﾀｲﾏｰ			*/
		TIMEOUT22,					/* SODIAC再生要求インターバルタイマー(CH1)	*/
		TIMEOUT23,					/* SODIAC再生要求インターバルタイマー(CH2)	*/
		TIMEOUT24,					/* SODIAC FROMアクセス権開放ﾀｲﾏｰ			*/
		TIMEOUT25,					/* FTPプログラムダウンロード用	*/
		TIMEOUT26,					/* 釣銭不足、用紙不足のサイクリック表示用タイマー	*/
		TIMEOUT27,					/* 人体検知センサーマスク時間タイマ		*/
		TIMEOUT28,					/* LCD焼き付き防止タイマ		*/
		TIMEOUT29,					/* ドアノブ戻し忘れ防止アラーム警告時間タイマ		*/
		TIMEOUT30,					/* ドアノブ戻し後エラー・アラーム送信タイマー		*/
		TIMEOUT31,					// プログラム切換前の未送信データ送信監視タイマ
		TIMEOUT32,					// SNTP応答監視
		TIMEOUT33,					/*	警報ブザー発報ディレイタイマ(ドア警報)	*/
		TIMEOUT34,					/*	警報ブザー発報ディレイタイマ(振動センサー)	*/
		TIMEOUT35,					/*	警報ブザー発報ディレイタイマ(紙幣脱落)	*/
// MH321800(S) G.So ICクレジット対応
		EC_CMDWAIT_TIMEOUT,			/* リーダ操作コマンド送信時の結果受信待ちタイマー	*/
		EC_CYCLIC_DISP_TIMEOUT,		/* サイクリック表示タイマー & ecｺﾏﾝﾄﾞ要求後のwaitﾀｲﾏｰ	*/
		EC_RECEPT_SEND_TIMEOUT,		/* ec受入可送信タイマー	*/
		EC_INQUIRY_WAIT_TIMEOUT,	/* 問合せ(処理中)結果待ちタイマー*/
		EC_REMOVAL_WAIT_TIMEOUT,	/* カード抜き取り待ちタイマー*/
// MH321800(E) G.So ICクレジット対応
// MH321800(S) Y.Tanizaki ICクレジット対応
// MH810103 GG119202(S) 起動シーケンス不具合修正
		EC_BRAND_NEGO_WAIT_TIMEOUT,	// ブランドネゴシエーション中の応答待ちタイマ
// MH810103 GG119202(E) 起動シーケンス不具合修正
#if (6 == AUTO_PAYMENT_PROGRAM)
		AUTO_PAYMENT_TIMEOUT,	
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
		TIMEOUT_ICCREDIT_STOP,		// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ
		TIMEOUT_MNT_RESTART,		// ﾒﾝﾃﾅﾝｽ開始再送ﾀｲﾏ
		TIMEOUT_MNT_RESTOP,			// ﾒﾝﾃﾅﾝｽ終了再送ﾀｲﾏ
		TIMEOUT_REMOTE_PAY_RESP,	// 遠隔精算開始応答OK待ちﾀｲﾏ
		TIMEOUT_QR_START_RESP,		// QRﾘｰﾀﾞ開始応答OK待ちﾀｲﾏ
		TIMEOUT_QR_STOP_RESP,		// QRﾘｰﾀﾞ停止応答OK待ちﾀｲﾏ
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera 2020/02/21 車番チケットレス( #3858 起動待機状態無限ループ防止)
		TIMEOUT_LCD_STA_WAIT,		// LCD起動通知/起動完了通知待ちﾀｲﾏ
// MH810100(E) K.Onodera 2020/02/21 車番チケットレス( #3858 起動待機状態無限ループ防止)
// MH810104 GG119201(S) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
		EJA_INIT_WAIT_TIMEOUT,		// 電子ジャーナル初期化設定待ちタイマ
// MH810104 GG119201(E) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
		RPT_PRINT_WAIT_TIMEOUT,		// レシート印字完了待ちタイマ
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
		RPT_INFORES_WAIT_TIMEOUT,	// レシート情報要求タイマ
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
		TIMEOUT_QR_RECEIPT_DISP,	// QRコード発行案内表示タイマー
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）

//　<--以後タイマー追加はここへ
		TIMEOUT_MAX
};
#define	OPE_TIM_MAX		(TIMEOUT_MAX-TIMEOUT1)		
#define	TIMEOUT_SNTP	TIMEOUT32
#define	_TIMERNO_SNTP	(TIMEOUT_SNTP-TIMEOUT)
// MH321800(S) G.So ICクレジット対応
#define TIMERNO_EC_CMDWAIT		(EC_CMDWAIT_TIMEOUT-TIMEOUT)
#define	TIMERNO_EC_CYCLIC_DISP	(EC_CYCLIC_DISP_TIMEOUT-TIMEOUT)
#define	TIMERNO_EC_RECEPT_SEND	(EC_RECEPT_SEND_TIMEOUT-TIMEOUT)
#define	TIMERNO_EC_INQUIRY_WAIT (EC_INQUIRY_WAIT_TIMEOUT-TIMEOUT)
#define TIMERNO_EC_REMOVAL_WAIT (EC_REMOVAL_WAIT_TIMEOUT-TIMEOUT)
// MH321800(E) G.So ICクレジット対応
// MH810103 GG119202(S) 起動シーケンス不具合修正
#define	TIMERNO_EC_BRAND_NEGO_WAIT	(EC_BRAND_NEGO_WAIT_TIMEOUT-TIMEOUT)
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH321800(S) Y.Tanizaki ICクレジット対応
#if (6 == AUTO_PAYMENT_PROGRAM)
#define TIMERNO_AUTO_PAYMENT_TIMEOUT	(AUTO_PAYMENT_TIMEOUT-TIMEOUT)
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
#define TIMERNO_ICCREDIT_STOP		(TIMEOUT_ICCREDIT_STOP 		- TIMEOUT)	// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ
#define TIMERNO_MNT_RESTART			(TIMEOUT_MNT_RESTART 		- TIMEOUT)	// ﾒﾝﾃﾅﾝｽ開始再送ﾀｲﾏ
#define TIMERNO_MNT_RESTOP			(TIMEOUT_MNT_RESTOP 		- TIMEOUT)	// ﾒﾝﾃﾅﾝｽ停止再送ﾀｲﾏ
#define TIMERNO_REMOTE_PAY_RESP		(TIMEOUT_REMOTE_PAY_RESP 	- TIMEOUT)	// 操作通知(遠隔精算開始応答OK)待ちﾀｲﾏ
#define TIMERNO_QR_START_RESP		(TIMEOUT_QR_START_RESP 		- TIMEOUT)	// QRﾘｰﾀﾞ開始応答OK待ちﾀｲﾏ
#define TIMERNO_QR_STOP_RESP		(TIMEOUT_QR_STOP_RESP 		- TIMEOUT)	// QRﾘｰﾀﾞ停止応答OK待ちﾀｲﾏ
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) K.Onodera 2020/02/21 車番チケットレス(#3858起動待機状態無限ループ防止)
#define TIMERNO_LCD_STA_WAIT		(TIMEOUT_LCD_STA_WAIT 		- TIMEOUT)	// LCD起動通知/起動完了通知待ちﾀｲﾏ
// MH810100(E) K.Onodera 2020/02/21 車番チケットレス(#3858起動待機状態無限ループ防止)
// MH810104 GG119201(S) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
#define	TIMERNO_EJA_INIT_WAIT	(EJA_INIT_WAIT_TIMEOUT-TIMEOUT)
// MH810104 GG119201(E) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
#define	TIMERNO_RPT_PRINT_WAIT	(RPT_PRINT_WAIT_TIMEOUT-TIMEOUT)			// ﾚｼｰﾄ印字完了待ちﾀｲﾏ
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
#define	TIMERNO_RPT_INFORES_WAIT	(RPT_INFORES_WAIT_TIMEOUT-TIMEOUT)		// ﾚｼｰﾄ情報要求ﾀｲﾏ
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
#define	TIMERNO_QR_RECEIPT_DISP		(TIMEOUT_QR_RECEIPT_DISP-TIMEOUT)		// QRコード発行案内表示タイマー
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Flapper Define														*/
	/*----------------------------------------------------------------------*/
#define		LK_DATAREC				0x0400	/* IF盤よりﾃﾞｰﾀ受信				*/
#define		LK_SND_CLS				0x0411	/* IF盤へ制御ﾃﾞｰﾀ送信要求(閉)	*/
#define		LK_SND_OPN				0x0412	/* IF盤へ制御ﾃﾞｰﾀ送信要求(開)	*/
#define		LK_SND_A_STS			0x0413	/* IF盤へ制御ﾃﾞｰﾀ送信要求(全状態要求)	*/
#define		LK_SND_STS				0x0414	/* IF盤へ制御ﾃﾞｰﾀ送信要求(状態要求)		*/
#define		LK_SND_A_CLS			0x0415	/* IF盤へ制御ﾃﾞｰﾀ送信要求(全て閉)		*/
#define		LK_SND_A_OPN			0x0416	/* IF盤へ制御ﾃﾞｰﾀ送信要求(全て開)		*/
#define		LK_SND_MNT				0x0417	/* IF盤へ制御ﾃﾞｰﾀ送信要求(ﾒﾝﾃ情報要求)	*/
#define		LK_SND_A_TEST			0x0418	/* IF盤へ制御ﾃﾞｰﾀ送信要求(全開閉ﾃｽﾄ)	*/
#define		LK_SND_TEST				0x0419	/* IF盤へ制御ﾃﾞｰﾀ送信要求(開閉ﾃｽﾄ)		*/
#define		LK_SND_A_CNT			0x041a	/* IF盤へ制御ﾃﾞｰﾀ送信要求(全動作ｶｳﾝﾄｸﾘｱ)*/
#define		LK_SND_CNT				0x041b	/* IF盤へ制御ﾃﾞｰﾀ送信要求(動作ｶｳﾝﾄｸﾘｱ)	*/
#define		LK_SND_VER				0x041c	/* IF盤へ制御ﾃﾞｰﾀ送信要求(ﾊﾞｰｼﾞｮﾝ要求)	*/
#define		LK_SND_ERR				0x041d	/* IF盤へ制御ﾃﾞｰﾀ送信要求(ｴﾗｰ状態要求)	*/
#define		LK_SND_P_CHK			0x041e	/* IF盤へ制御ﾃﾞｰﾀ送信要求(制御ポート検査)	*/
#define		LK_SND_A_LOOP_DATA		0x041f	/* ループデータ要求						*/
#define		FLAP_A_UP_SND_MNT		0x0420	/* 全ﾌﾗｯﾌﾟ上昇要求(ﾒﾝﾃﾅﾝｽ)		*/
#define		FLAP_A_DOWN_SND_MNT		0x0421	/* 全ﾌﾗｯﾌﾟ下降要求(ﾒﾝﾃﾅﾝｽ)		*/
#define		LOCK_A_CLS_SND_MNT		0x0422	/* 全ﾛｯｸ装置閉要求(ﾒﾝﾃﾅﾝｽ)		*/
#define		LOCK_A_OPN_SND_MNT		0x0423	/* 全ﾛｯｸ装置開要求(ﾒﾝﾃﾅﾝｽ)		*/
#define		FLAP_A_UP_SND_NTNET		0x0424	/* 全ﾌﾗｯﾌﾟ上昇要求(NTNET)		*/
#define		FLAP_A_DOWN_SND_NTNET	0x0425	/* 全ﾌﾗｯﾌﾟ下降要求(NTNET)		*/
#define		LOCK_A_CLS_SND_NTNET	0x0426	/* 全ﾛｯｸ装置閉要求(NTNET)		*/
#define		LOCK_A_OPN_SND_NTNET	0x0427	/* 全ﾛｯｸ装置開要求(NTNET)		*/
#define		INT_FLAP_A_UP_SND_MNT		0x042a	/* 全内蔵ﾌﾗｯﾌﾟ上昇要求(ﾒﾝﾃﾅﾝｽ)		*/
#define		INT_FLAP_A_DOWN_SND_MNT		0x042b	/* 全内蔵ﾌﾗｯﾌﾟ下降要求(ﾒﾝﾃﾅﾝｽ)		*/
#define		INT_FLAP_A_UP_SND_NTNET		0x042c	/* 全内蔵ﾌﾗｯﾌﾟ上昇要求(NTNET)		*/
#define		INT_FLAP_A_DOWN_SND_NTNET	0x042d	/* 全内蔵ﾌﾗｯﾌﾟ下降要求(NTNET)		*/
#define		FLAP_UP_SND				0x0430	/* ﾌﾗｯﾌﾟ上昇指示(ﾒﾝﾃﾅﾝｽ)		*/
#define		FLAP_DOWN_SND			0x0440	/* ﾌﾗｯﾌﾟ下降指示(ﾒﾝﾃﾅﾝｽ)		*/
#define		FLAP_UP_SND_SS			0x0431	/* ﾌﾗｯﾌﾟ上昇指示(修正精算時)	*/
#define		FLAP_DOWN_SND_SS		0x0441	/* ﾌﾗｯﾌﾟ下降指示(修正精算時)	*/
#define		FLAP_UP_SND_NTNET		0x0432	/* NT-NET ﾌﾗｯﾌﾟ上昇指示			*/
#define		FLAP_DOWN_SND_NTNET		0x0442	/* NT-NET ﾌﾗｯﾌﾟ下降指示			*/
#define		FLAP_UP_SND_RAU			0x0433	/* RAU ﾌﾗｯﾌﾟ上昇指示			*/
#define		FLAP_DOWN_SND_RAU		0x0443	/* RAU ﾌﾗｯﾌﾟ下降指示			*/
#define		CAR_FLPMOV_SND			0x0450	/* ﾌﾗｯﾌﾟ板動作監視ﾀｲﾏｰﾀｲﾑｱｯﾌﾟ	*/
#define		CAR_SVSLUG_SND			0x0460	/* ｻｰﾋﾞｽﾀｲﾑﾗｸﾞﾀｲﾑｱｯﾌﾟ			*/
#define		CAR_INCHK_SND			0x0461	/* 入庫判定ﾀｲﾑｱｯﾌﾟ				 */
#define		CAR_PAYCOM_SND			0x0491	/* 精算完了						*/
											/*								*/
#define		FLAP_UP_SND_MAF			0x0492	/* MAF ﾌﾗｯﾌﾟ上昇指示			*/
#define		FLAP_DOWN_SND_MAF		0x0493	/* MAF ﾌﾗｯﾌﾟ下降指示			*/

#define		CAR_FURIKAE_OUT			0x0495	// 振替元の車が出庫

#define		DOWNLOCKOVER_SND		0x0496	/* 下降ﾛｯｸﾀｲﾑｱｯﾌﾟ				*/
#define		CTRL_PORT_CHK_RECV		0x0497	/* 制御ポート検査応答受信		*/
#define		LK_SND_FORCE_LOOP_ON	0x5010	/* フラップ強制ループON			*/
#define		LK_SND_FORCE_LOOP_OFF	0x5011	/* フラップ強制ループOFF		*/

enum {
	_MTYPE_ALL = 0,  // 全指定
	_MTYPE_LOCK,     // 全ﾛｯｸ装置   = 1
	_MTYPE_FLAP,     // 全ﾌﾗｯﾌﾟ装置 = 2
	_MTYPE_INT_FLAP, // 全内蔵ﾌﾗｯﾌﾟ装置 = 3
};
	/*----------------------------------------------------------------------*/
	/*	Coin																*/
	/*----------------------------------------------------------------------*/
#define		COIN_EVT				0x0500	/* ｺｲﾝﾒｯｸｲﾍﾞﾝﾄ					*/
#define		COIN_IN_EVT				0x0501	/* 入金あり						*/
#define		COIN_OT_EVT				0x0502	/* 払出し完了					*/
#define		COIN_IM_EVT				0x0505	/* 入金あり（max達成）			*/
#define		COIN_EN_EVT				0x0507	/* 払出し可状態					*/
#define		COIN_ER_EVT				0x0509	/* 払出しｴﾗｰ発生				*/
#define		COIN_CASSET				0x0510	/* ｺｲﾝｶｾｯﾄ着					*/
#define		COIN_INVENTRY			0x0511	/* ﾒｯｸﾎﾞﾀﾝによるｲﾝﾍﾞﾝﾄﾘ終了		*/
#define		COIN_IH_EVT				0x0521	/* 入金あり(保有枚数増加)		*/
#define		COIN_ES_TIM				0x0530	/* ｴｽｸﾛﾀｲﾏｰﾀｲﾑｱｳﾄ				*/
#define		COIN_CNT_SND			0x0535	/* 保有枚数送信完了				*/
#define		COIN_RJ_EVT				0x0540	/* 精算中止						*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Printer																*/
	/*----------------------------------------------------------------------*/
#define		PRINTER_EVT				0x0600	/* ﾌﾟﾘﾝﾀ関連ﾒｯｾｰｼﾞ				*/
#define		PRNFINISH_EVT			0x1600	/* ﾌﾟﾘﾝﾀ関連ﾒｯｾｰｼﾞ				*/
	/* その他のﾒｯｾｰｼﾞ0x0600～0x06ffまでpri_def.hで定義						*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	NT45EXで0x0701～0x0751まで使用のため0x0701～0x07ffは未使用とする	*/
	/*----------------------------------------------------------------------*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Note																*/
	/*----------------------------------------------------------------------*/
#define		NOTE_EVT				0x0800	/* 紙幣ﾘｰﾀﾞｰｲﾍﾞﾝﾄ				*/
#define		NOTE_IN_EVT				0x0801	/* 入金あり						*/
#define		NOTE_OT_EVT				0x0802	/* 払出し完了					*/
#define		NOTE_IM_EVT				0x0803	/* 収金完了						*/
#define		NOTE_I2_EVT				0x0805	/* 後続入金あり					*/
#define		NOTE_EN_EVT				0x0807	/* 払出し可状態					*/
#define		NOTE_EM_EVT				0x0809	/* 戻しｴﾗｰ発生					*/
#define		NOTE_ER_EVT				0x080A	/* 収納ｴﾗｰ発生					*/
#define		NOTE_SO_EVT				0x0810	/* ｽﾀｯｶｰ開						*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	NT45EXで0x0901～0x0925まで使用のため0x0901～0x09ffは未使用とする	*/
	/*----------------------------------------------------------------------*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Operation Event Message												*/
	/*----------------------------------------------------------------------*/
#define		OPE_EVT					0x0a00	/* オペレーションイベント		*/
#define		OPE_OPNCLS_EVT			0x0a01	/* 強制営休業 event				*/
#define		OPE_REQ_CALC_FEE		0x0a02	/* 料金計算要求					*/
											/*								*/
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
#define		OPE_REQ_REMOTE_CALC_TIME_PRE		0x0a11	// 遠隔精算料金計算要求
#define		OPE_REQ_REMOTE_CALC_TIME			0x0a12	// 遠隔精算時刻指定精算要求
#define		OPE_REQ_REMOTE_CALC_FEE				0x0a14	// 遠隔精算金額指定精算要求
#define		OPE_REQ_FURIKAE_TARGET_INFO			0x0a16	// 振替対象情報取得要求
#define		OPE_REQ_FURIKAE_GO					0x0a17	// 振替実行要求
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加)_for_LCD_IN_CAR_INFO_op_mod00))
#define		OPE_REQ_LCD_CALC_IN_TIME			0x0a22	// 車番チケットレス精算(入庫時刻指定)要求
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加)_for_LCD_IN_CAR_INFO_op_mod00))

// MH321800(S) G.So ICクレジット対応
#define		AUTO_CANCEL				0x0ae2
#define		OPE_EC_ALARM_LOG_PRINT_1_END	0x0ae3
// MH321800(E) G.So ICクレジット対応
// MH810105(S) MH364301 インボイス対応
#define		OPE_DELAY_RYO_PRINT		0x0ae4
// MH810105(E) MH364301 インボイス対応
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
#define		OPE_DUMMY_NOTICE		0x0ae5
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）

// MH322914 (e) kasiyama 2016/07/15 AI-V対応
/* 0x0b00～0x0bff = 予備（特注用）											*/
	/*----------------------------------------------------------------------*/
	/*	Arcnet Event Message												*/
	/*----------------------------------------------------------------------*/
											/* Reader						*/
#define		ARC_CR_EVT				0x0c00	/* Reader event					*/
#define		ARC_CR_R_EVT			0x0c14	/* 20:Read Data Receive event	*/
#define		ARC_CR_E_EVT			0x0c15	/* 21:END  Data Receive event	*/
#define		ARC_CR_VER_EVT			0x0c16	/* 22:Vertion   Receive event	*/
#define		ARC_CR_EOT_EVT			0x0c17	/* 23:EOT Status Receive event	*/
#define		ARC_CR_EOT_RCMD			0x0c18	/* 24:EOT Status RCMD Receive event*/
#define		ARC_CR_EOT_RCMD_WT		0x0c19	/* 25:EOT Status RCMD Wait event*/
// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
#define		ARC_CR_EOT_MOVE_TKT		0x0c1e	/* 30:EOT Status Tkt move mouth */
// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
#define		ARC_CR_SND_EVT			0x0c32	/* 50:Command Send Complete event*/
#define		ARC_CR_SER_EVT			0x0c50	/* 80:Command Send Error event	*/
											/* ARC_CR_SER_EVTは未使用		*/
											/*								*/
											/* Announce Machine				*/
#define		ARC_AVM_EVT				0x0d00  /* Announce event				*/
#define		ARC_AVM_EXEC_EVT		0x0d01  /* 01:放送状態ｲﾍﾞﾝﾄ				*/
#define		ARC_AVM_VER_EVT			0x0d20	/* 32:ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀｲﾍﾞﾝﾄ			*/
#define		ARC_AVM_SND_EVT			0x0d3c	/* 60:送信完了ｲﾍﾞﾝﾄ				*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	NTNET Event Message													*/
	/*----------------------------------------------------------------------*/
#define		NTLWR_RCV_EVT			0x0e00	/* recv event					*/
#define		NTLWR_SNDFIN_EVT		0x0e01	/* send finish event			*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	0x0f00～0x0fff = 予備(特注用)										*/
	/*	0x1000～0x12ff = NT45EXﾌﾗｯﾌﾟ拡張で使用								*/
	/*	0x1300～0x15ff = 予備(特注用)										*/
	/*	0x1600～0x16ff = ﾌﾟﾘﾝﾀで使用										*/
	/*	0x1700～0x1fff = 予備(特注用)										*/
	/*----------------------------------------------------------------------*/
#define		NTDOPALWR_RCV_EVT		0x0f00	/* recv event					*/
#define		NTDOPALWR_SNDFIN_EVT	0x0f01	/* send finish event			*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	IBK関連(0x2000～0x2fff)												*/
	/*----------------------------------------------------------------------*/
#define		IBK_EVT					0x2000  /* IBK event					*/
#define		IBK_MIF_EVT_ERR			0x2001	/* MIF:送信結果ｴﾗｰ				*/
											/*								*/
// A1ｺﾏﾝﾄﾞ（ｶｰﾄﾞｾﾝｼﾝｸﾞ)						/*								*/
#define		IBK_MIF_A1_OK_EVT		0x2002	/* MIF:ｶｰﾄﾞｼﾘｱﾙID取得完了		*/
#define		IBK_MIF_A1_NG_EVT		0x2003	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
// A2ｺﾏﾝﾄﾞ (ｶｰﾄﾞｾﾝｼﾝｸﾞ+読出し)				/*								*/
#define		IBK_MIF_A2_OK_EVT		0x2004	/* MIF:ｶｰﾄﾞﾃﾞｰﾀ読出し完了		*/
#define		IBK_MIF_A2_NG_EVT		0x2005	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
// A3ｺﾏﾝﾄﾞ (ｶｰﾄﾞｾﾝｼﾝｸﾞ+読出し)				/*								*/
#define		IBK_MIF_A3_OK_EVT		0x2006	/* MIF:ｶｰﾄﾞﾃﾞｰﾀ読出し完了		*/
#define		IBK_MIF_A3_NG_EVT		0x2007	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
// A4ｺﾏﾝﾄﾞ (ｶｰﾄﾞｾﾝｼﾝｸﾞ+読出し)				/*								*/
#define		IBK_MIF_A4_OK_EVT		0x2008	/* MIF:ｶｰﾄﾞﾃﾞｰﾀ読出し完了		*/
#define		IBK_MIF_A4_NG_EVT		0x2009	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
// A5ｺﾏﾝﾄﾞ (ｶｰﾄﾞｾﾝｼﾝｸﾞ+読出し)				/*								*/
#define		IBK_MIF_A5_OK_EVT		0x200a	/* MIF:ｶｰﾄﾞﾃﾞｰﾀ読出し完了		*/
#define		IBK_MIF_A5_NG_EVT		0x200b	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
// A6ｺﾏﾝﾄﾞ (ｶｰﾄﾞｾﾝｼﾝｸﾞ+読出し)				/*								*/
#define		IBK_MIF_A6_OK_EVT		0x200c	/* MIF:ｶｰﾄﾞﾃﾞｰﾀ読出し完了		*/
#define		IBK_MIF_A6_NG_EVT		0x200d	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
// A7ｺﾏﾝﾄﾞ (ｶｰﾄﾞｾﾝｼﾝｸﾞ+読出し)				/*								*/
#define		IBK_MIF_A7_OK_EVT		0x200e	/* MIF:ｶｰﾄﾞﾃﾞｰﾀ読出し完了		*/
#define		IBK_MIF_A7_NG_EVT		0x200f	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
// AEｺﾏﾝﾄﾞ (ｶｰﾄﾞｾﾝｼﾝｸﾞ+読出し)				/*								*/
#define		IBK_MIF_AE_OK_EVT		0x2010	/* MIF:ｶｰﾄﾞﾃﾞｰﾀ読出し完了		*/
#define		IBK_MIF_AE_NG_EVT		0x2011	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
// AFｺﾏﾝﾄﾞ (ｶｰﾄﾞｾﾝｼﾝｸﾞ+読出し)				/*								*/
#define		IBK_MIF_AF_OK_EVT		0x2012	/* MIF:ｶｰﾄﾞﾃﾞｰﾀ読出し完了		*/
#define		IBK_MIF_AF_NG_EVT		0x2013	/* MIF:ｶｰﾄﾞｼﾘｱﾙ読出しNG			*/
											/*								*/
#define		IBK_NTNET_LOCKMULTI		0x2020	/* NTNET ﾏﾙﾁ精算用ﾃﾞｰﾀ受信		*/
#define		IBK_NTNET_CHKPASS		0x2021	/* NTNET 定期問合せ結果ﾃﾞｰﾀ受信	*/

											/*								*/
#define		IBK_NTNET_ERR_REC		0x20a1	/* NTNET ｴﾗｰﾃﾞｰﾀ受信			*/
#define		IBK_NTNET_FREE_REC		0x20a2	/* NTNET FREEﾊﾟｹｯﾄﾃﾞｰﾀ受信		*/
#define		IBK_NTNET_DAT_REC		0x20a5	/* NTNETﾃﾞｰﾀ受信				*/
#define		IBK_NTNET_BUFSTATE_CHG	0x20aa	/* NTNETﾊﾞｯﾌｧ状態変化			*/
											/*								*/
#define		IBK_LPR_SRLT_REC		0x20b0	/* ﾗﾍﾞﾙﾌﾟﾘﾝﾀ:送信結果ﾃﾞｰﾀ受信	*/
#define		IBK_LPR_ERR_REC			0x20b1	/* ﾗﾍﾞﾙﾌﾟﾘﾝﾀ:ｴﾗｰﾃﾞｰﾀ受信		*/
#define		IBK_LPR_B1_REC			0x20b2	/* ﾗﾍﾞﾙﾌﾟﾘﾝﾀ:印字終了通知受信	*/
#define		IBK_LPR_B1_REC_OK		0x20b3	/* ﾗﾍﾞﾙﾌﾟﾘﾝﾀ:印字正常終了		*/
#define		IBK_LPR_B1_REC_NG		0x20b4	/* ﾗﾍﾞﾙﾌﾟﾘﾝﾀ:印字異常終了		*/

#define		IBK_COUNT_UPDATE	0x20c0		/* IBK テーブル件数結果受信	*/
#define		IBK_CTRL_OK			0x20c1		/* IBK 指示送信OK			*/
#define		IBK_CTRL_NG			0x20c2		/* IBK 指示送信NG			*/
#define		IBK_COMCHK_END		0x20c3		/* IBK 通信チェック終了		*/

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		IBK_EDY_RCV				0x20d0	/* EDY 受信ｲﾍﾞﾝﾄ			*/
//#define		IBK_EDY_ERR_REC			0x20d1	/* EDY 送信ｴﾗｰｲﾍﾞﾝﾄ			*/
//#define		IBK_EDY_RECV_TIMEOUT	0x20d2	/* EDY 応答受信タイムアウト	*/
//#define		IBK_EDY_SEND_ERR		0x20d3	/* EDY 送信異常通知	*/
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

#define		IFM_SEND_OK				0x20e0
#define		IFM_SEND_NG				0x20e1
#define		IFM_SEND_FAIL			0x20e2
#define		IFM_RECV				0x20e8
#define		IFM_RECV_VER			0x20e9

	/*----------------------------------------------------------------------*/
	/*	Other	(0x2100～0x21ff)											*/
	/*----------------------------------------------------------------------*/
#define		MID_OTHER_EVT			0x2100	/*								*/
#define		MID_STSCHG_CHGEND		0x2100	/* 釣銭有無変化 (data[0] 0=釣無し復旧,1=発生		*/
#define		MID_STSCHG_R_PNEND		0x2101	/* ﾚｼｰﾄﾌﾟﾘﾝﾀ紙切れ変化 (data[0] 0=切れ復旧,1=発生	*/
#define		MID_STSCHG_J_PNEND		0x2102	/* ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ紙切れ変化 (data[0] 0=切れ復旧,1=発生	*/
#define		MID_STSCHG_ANY_ERROR	0x2103	/* ｴﾗｰ 有無に変化あり			*/
#define		MID_STSCHG_ANY_ALARM	0x2104	/* ｱﾗｰﾑ有無に変化あり			*/
#define		MID_RECI_SW_TIMOUT		0x2105	/* 領収証発行可能ﾀｲﾏｰﾀｲﾑｱｳﾄ通知	*/
#define		SYU_SEI_SEISAN			0x2106	// 修正精算要求
#define		DUMMY_KEY_EVENT			0x2107	// 精算完了画面からのテンキー入力イベント
#define		KEY_DOOR1_DUMMY			0x2108	// Edyセンター通信中のドア閉イベント（ダミー用）
											/*								*/
	/*----------------------------------------------------------------------*/
	/* センタークレジット関連 (0x2200～0x22ff)								*/
	/*----------------------------------------------------------------------*/
#define		CRE_EVT					0x2200		/* Bit15 - - Bit8 */
#define		CRE_EVT_RECV			0x2220		/* CREデータ受信通知 */
#define		CRE_EVT_02_OK			0x2221		/* CREデータ種別 02 = 開局コマンド応答 */
#define		CRE_EVT_04_OK			0x2222		/* CREデータ種別 04 = 与信問合せ結果データ */
#define		CRE_EVT_06_OK			0x2223		/* CREデータ種別 06 = 売上依頼結果データ */
#define		CRE_EVT_08_OK			0x2224		/* CREデータ種別 08 = ｵﾝﾗｲﾝｴﾂﾄ応答 */
#define		CRE_EVT_0A_OK			0x2225		/* CREデータ種別 0A = 返品問合せ結果データ */
#define		CRE_EVT_02_NG			0x2226		/* CREデータ種別 02 = 開局コマンド応答 */
#define		CRE_EVT_04_NG1			0x2227		/* CREデータ種別 04 = 与信問合せ結果データ */
#define		CRE_EVT_04_NG2			0x2228		/* CREデータ種別 04 = 与信問合せ結果データ */
#define		CRE_EVT_06_NG			0x2229		/* CREデータ種別 06 = 売上依頼結果データ */
#define		CRE_EVT_08_NG			0x222A		/* CREデータ種別 08 = ｵﾝﾗｲﾝｴﾂﾄ応答 */
#define		CRE_EVT_0A_NG			0x222B		/* CREデータ種別 0A = 返品問合せ結果データ */
#define		CRE_EVT_SNDERR			0x222C		/* クレジットサーバへのデータ送信失敗通知 */
#define		CRE_EVT_CONN_PPP		0x222D		/* PPP接続完了 */
#define		CRE_EVT_06_TOUT			0x222E		/* CREデータ種別 06 = 売上依頼結果データ(ﾀｲﾑｱｳﾄ) */
#define		MAF_CAPPI_EVT_COMTEST_RCV	0x2251	/* 折り返しテスト バージョン通知 */
#define		MAF_CAPPI_EVT_SEVCOM_RCV	0x2252	/* Cappiサーバー疎通確認応答受信(B1受信) */
#define		I2CDEV2_1_EVT_I2CSNDCMP_RCV	0x2253		/* I2cDev2_1 I2C送信完了通知受信 */

	/*----------------------------------------------------------------------*/
	/*	0x2300～0xffff = 予備(特注用)										*/
	/*----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*/
	/* Suica関連 (0x2300～0x2310)											*/
	/*----------------------------------------------------------------------*/
#define		SUICA_EVT				0x2300	/* SUICA用ｲﾍﾞﾝﾄ	*/
#define		ELE_EVENT_CANSEL		0x2301	/* 電子決済ｷｬﾝｾﾙ完了ｲﾍﾞﾝﾄ（精算中止用）	*/
#define		ELE_EVT_STOP			0x2302	/* 電子決済停止用ｲﾍﾞﾝﾄ	*/
#define		SUICA_INITIAL_END		0x2303	/* Suica初期化完了	*/
#define		SUICA_ENABLE_EVT		0x2304	/* SUICAステータス変化用（受付可）ｲﾍﾞﾝﾄ	*/
#define		SUICA_DISABLE_EVT		0x2305	/* SUICAステータス変化用（受付不可）ｲﾍﾞﾝﾄ	*/

#define		SUICA_PAY_DATA_ERR		0x2307	/* SUICA決済データ異常通知	*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/* ParkiPROオペレーション関連 (0x24xx)									*/
	/*----------------------------------------------------------------------*/
/* for communication test */
#define		IFMCHK_OK_EVT			0x2400
#define		IFMCHK_NG_EVT			0x2401
/* for payment */
#define		IFMPAY_GENGAKU			0x2410		/* 減額精算受付け */
#define		IFMPAY_FURIKAE			0x2411		/* 振替精算受付け */


	/*----------------------------------------------------------------------*/
	/*	MAF関連(0x2500～0x25ff)												*/
	/*----------------------------------------------------------------------*/

	/*----------------------------------------------------------------------*/
	/*	遠隔ダウンロード関連(0x2600～)												*/
	/*----------------------------------------------------------------------*/
#define		REMOTE_DL_EVT			0x2600		/* 遠隔ダウンロードイベント	*/
#define		REMOTE_DL_REQ			0x2601		/* 遠隔ダウンロード要求		*/
#define		REMOTE_CONNECT_EVT		0x2602		/* Rism接続切断/復旧完了通知*/
#define		REMOTE_DL_END			0x2603		/* 遠隔ダウンロード完了		*/
#define		REMOTE_RETRY_SND		0x2604		/* リトライ送信要求			*/
#define		REMOTE_CONNECTCHK_END	0x2605		/* 接続確認終了 */
	/*----------------------------------------------------------------------*/
	/*	CAN通信関連(0x2700～)												*/
	/*----------------------------------------------------------------------*/
enum {
		CAN_EMG_EVT		= 0x2700,	// エマージェンシー受信
};

	/*----------------------------------------------------------------------*/
	/*	I2C通信関連(0x2800～)												*/
	/*----------------------------------------------------------------------*/
enum {
		I2C_EVT		= 0x2800,					// I2Cイベント
		I2C_RP_ERR_EVT,							// I2C通信エラー通知（RP）
		I2C_JP_ERR_EVT,							// I2C通信エラー通知（JP）
};

enum {
		SODIAC_EVT		= 0x2900,				// SODIAC(音声)イベント
		SODIAC_NEXT_REQ,						// 次の音声データ再生要求
		SODIAC_PLAY_CMP,						// 音声データ再生完了
};
enum {
		FTP_EVENT				=	0x2A00,		/* FTP関連イベント */
		FTP_LOGIN,								/* FTPログイン結果通知（FTPD専用） */
		FTP_LOGOUT,								/* FTPログアウト結果通知（FTPD専用） */
		FTP_RECVING,							/* FTP	受信処理中 */
		FTP_SENDING,							/* FTP	送信処理中 */
		FTP_NOTFILE_R,							/* FTP	対象データがない(Read時) */
		FTP_NOTFILE_W,							/* FTP	対象データがない(Write時) */
		FTP_SERVER_TIMEOUT,						/* FTPサーバータイムアウト通知（FTPD専用） */
		FTP_FORCE_LOGOUT,						/* FTPサーバー強制ログアウト（FTPD専用） */
		
};

	/*----------------------------------------------------------------------*/
	/*	KASAGO通信関連(0x2B00～)												*/
	/*----------------------------------------------------------------------*/
enum {
		KSG_ANT_EVT		= 0x2B00,				// アンテナレベル受信イベント
};

// MH321800(S) G.So ICクレジット対応
	/*----------------------------------------------------------------------*/
	/* 決済リーダ関連 (0x2C00～0x2C10)										*/
	/*----------------------------------------------------------------------*/
enum {
		EC_EVT = 0x2C00,					/* 決済リーダ用ｲﾍﾞﾝﾄ			*/
		EC_BRAND_UPDATE,					/* ブランド状態更新ｲﾍﾞﾝﾄ		*/
		EC_VOLUME_UPDATE,					/* 音量変更更新ｲﾍﾞﾝﾄ			*/
		EC_MNT_UPDATE,						/* リーダメンテナンス更新ｲﾍﾞﾝﾄ	*/
		EC_EVT_CRE_PAID,					/* クレジット決済完了			*/
		EC_EVT_CRE_PAID_NG,					/* クレジット決済失敗			*/
		EC_EVT_CARD_STS_UPDATE,				/* カード差込状態更新通知 		*/
		EC_EVT_DEEMED_SETTLEMENT,			/* みなし決済トリガ発生通知 	*/
		EC_EVT_CANCEL_PAY_OFF,				/* 精算キャンセル発生通知 		*/
		EC_EVT_COMFAIL,						/* 通信不良発生通知				*/
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		EC_BRAND_RESULT_CANCELED,			/* ブランド選択結果 キャンセル  */
		EC_BRAND_RESULT_UNSELECTED,			/* ブランド選択結果 失敗		*/
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) JVMAリセット処理変更
		EC_EVT_NOT_AVAILABLE_ERR,			// 決済リーダ切り離しエラー発生
// MH810103 GG119202(E) JVMAリセット処理変更
// MH810103 GG119202(S) みなし決済扱い時の動作
		EC_EVT_DEEMED_SETTLE_FUKUDEN,		// みなし決済扱い（復電）
// MH810103 GG119202(E) みなし決済扱い時の動作
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
		EC_EVT_ENABLE_NYUKIN,				// 再入金可イベント
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
// MH810105(S) MH364301 QRコード決済対応
		EC_EVT_QR_PAID,						/* QRコード決済完了				*/
// MH810105(E) MH364301 QRコード決済対応
};
// MH321800(E) G.So ICクレジット対応
// MH810100(S) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)	
	/*----------------------------------------------------------------------*/
	/*	LCD通信関連(0x3000～)												*/
	/*----------------------------------------------------------------------*/
enum{
		LCD_BASE			= 0x3000,		// LCD関連
		LCD_QR_REQ,							// 0x3001	QRリーダ制御要求
		LCD_ERROR_NOTICE,					// 0x3002	ｴﾗｰ通知
		LCD_OPERATION_NOTICE,				// 0x3003	操作通知
		LCD_IN_CAR_INFO,					// 0x3004	入庫情報
		LCD_QR_CONF_CAN_RES,				// 0x3005	QR確定・取消データ応答
		LCD_QR_DATA,						// 0x3006	QRデータ
		LCD_ICC_SETTLEMENT_STS,				// 0x3007	決済状態ﾃﾞｰﾀ
		LCD_ICC_SETTLEMENT_RSLT,			// 0x3008	決済結果情報
		LCD_MNT_QR_CTRL_RESP,				// 0x3009	QRﾘｰﾀﾞ制御応答(ﾒﾝﾃﾅﾝｽ)
		LCD_MNT_QR_READ_RESULT,				// 0x300A	QR読取結果(ﾒﾝﾃﾅﾝｽ)
		LCD_MNT_REALTIME_RESULT,			// 0x300B	ﾘｱﾙﾀｲﾑ通信疎通結果(ﾒﾝﾃﾅﾝｽ)
		LCD_MNT_DCNET_RESULT,				// 0x300C	DC-NET通信疎通結果(ﾒﾝﾃﾅﾝｽ)
		LCD_COMMUNICATION_CUTTING,			// 0x300D	切断検知(PKTtaskでLCDとの通信が不通時に発行される)
		LCD_DISCONNECT,						// 0x300E	LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTINGの共通ﾒｯｾｰｼﾞ
		LCD_LCDBM_OPCD_PAY_STP,				// 0x300F	精算中止処理ﾒｯｾｰｼﾞ	操作通知(精算中止要求=LCDBM_OPCD_PAY_STP)受信時にICｸﾚｼﾞｯﾄ非問合せ中の時に発行
		LCD_LCDBM_OPCD_STA_NOT,				// 0x3010	精算中止処理ﾒｯｾｰｼﾞ	操作通知(起動通知=LCDBM_OPCD_STA_NOT)受信時にICｸﾚｼﾞｯﾄ非問合せ中の時に発行
		LCD_LCD_DISCONNECT,					// 0x3011	精算中止処理ﾒｯｾｰｼﾞ	LCD_DISCONNECT(以下の2つのmessage受信時に発行)受信時にICｸﾚｼﾞｯﾄ非問合せ中の時に発行
											// 								1=LCDBM_OPCD_RESET_NOT(ﾘｾｯﾄ通知) / 2=LCD_COMMUNICATION_CUTTING(切断検知)
// MH810100(S) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
		LCD_CHG_IDLE_DISP,					// 0x3012	待機画面通知(op_mod02()で操作通知 - 待機画面通知受信済みであることの通知用)
// MH810100(E) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		LCD_LANE_DATA_RES,					// 0x3013	レーンモニタデータ応答
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//		↑新規LCD_系追加
		LCD_MESSAGE_MAX						// 
};
// MH810100(E) Y.Yamauchi 2019/10/10 車番チケットレス(メンテナンス)	

// MH810100(S) K.Onodera 2019/11/01 車番チケットレス（LCD通信）
	/*----------------------------------------------------------------------*/
	/*	PKTタスク用(0x3100～)												*/
	/*----------------------------------------------------------------------*/
enum{
		PKT_BASE			= 0x3100,		// PKT関連
		PKT_SOC_CON,
		PKT_SOC_CLS,
		PKT_SOC_RST,
		PKT_SOC_SND,
		PKT_SOC_RCV,
		PKT_SND_PRT,
		PKT_RST_PRT,
};
// MH810100(E) K.Onodera 2019/11/01 車番チケットレス（LCD通信）


	/*----------------------------------------------------------------------*/
	/* SNTP		(0xE00x)													*/
	/*----------------------------------------------------------------------*/
enum {
	SNTPMSG_EVT = 0xE000,
	SNTPMSG_RECV_TIME,						/* SNTP response manual			*/
	SNTPMSG_INVALID_RECV_TIME,				/* 不正時刻受信(手動)			*/
	SNTPMSG_RECV_AUTORES,					/* SNTP response auto			*/
	SNTPMSG_TIMEOUT,						/* SNTP request timeout			*/
	SNTPMSG_AUTO_SET,						/* clock set					*/
	SNTPMSG_INVALID_RECV_AUTO,				/* 不正時刻受信(自動)			*/
};
	/*----------------------------------------------------------------------*/
	/* TELNET	(0xE01x)													*/
	/*----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*/
	/* FTP		(0xE02x)													*/
	/*----------------------------------------------------------------------*/
enum {
	FTPMSG_CTRL_RECV = 0xE020,
	FTPMSG_CTRL_SEND,
	FTPMSG_FILE_SEND,
	FTPMSG_FILE_RECV,
	FTPMSG_FILE_RECVEND
};
	/*----------------------------------------------------------------------*/
	/* BASE		(0xEF0x)													*/
	/*----------------------------------------------------------------------*/
enum {
	HIF_RECV_VER = 0xEF00
};
enum {
			TIMEOUT1_1MS =	0x3301,
			TIMEOUT2_1MS,
			TIMEOUT3_1MS,
			TIMEOUT4_1MS,
			TIMEOUT_1MS_MAX
};
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
/**
	*	LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING受信時の共通ﾒｯｾｰｼﾞ(LCD_DISCONNECT)
 */
typedef	struct {
	unsigned short		MsgId;	// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
} lcdbm_lcd_disconnect_t;		// LCD_DISCONNECT
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

#endif	// _MESSAGE_H_
