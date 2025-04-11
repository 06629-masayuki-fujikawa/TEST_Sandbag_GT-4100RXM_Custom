/*[]----------------------------------------------------------------------[]*/
/*| headder file for system                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005.01.31                                               |*/
/*| PORTING     : 2008-10-17 MATSUSHITA for CRW                            |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include	"iodefine.h"
#define	SYSTEM_TYPE_IS_NTNET	1

/*[]----------------------------------------------------------------------[]*/
/*| shorten style symbol                                                   |*/
/*[]----------------------------------------------------------------------[]*/
typedef		unsigned char	uchar;							/*								*/
typedef		unsigned short	ushort;							/*								*/
typedef		unsigned long	ulong;							/*								*/
typedef		unsigned long long	ulonglong;					/*								*/

#define	UPDATE_A_PASS		0

#define	SYUSEI_PAYMENT		0

// 自動連続精算
/*[]----------------------------------------------------------------------[]*/
/*| 自動連続精算ﾌﾟﾛｸﾞﾗﾑ                                                    |*/
/*[]----------------------------------------------------------------------[]*/
#define	AUTO_PAYMENT_PROGRAM		0		// なし（出荷時設定）

												// ﾕｰｻﾞｰﾒﾝﾃ → 拡張機能 → Edy処理内にﾒﾆｭｰを追加し、ﾒﾝﾃ画面から送信ﾃﾞｰﾀを選択。
												// Debug精算モードを精算ｷｰ＆取消ｷｰ ONしなくても待機に戻るたびに有効にする。（24件）
												// 通常は、受信データを分割した後にログ登録を行うため、データが分割して受信しているのか
												// 纏まって受信しているのかを判断することが出来なかった。
												// 本デバッグモードを有効にすることで、受信データ（生データ）をそのままログに記録することで、
												// どの単位でデータを受信したのかを把握することが出来る。

/*================  デバッグモード  ================*/
#define	DEBUG_MODE			1
#define	NORMAL_MODE			0

// MH810100(S)
#define	GT4100_NO_LCD		0
// MH810100(E)
/*------- DEBUG_MODE_01：エラー・アラームマスク	--------*/
/*														*/
/* 古いCRM基板では、通常状態でも「紙幣リーダー脱落」等の*/
/* エラーが発生するのでマスクする。	　　　　　　　　　　*/

  #define	DEBUG_MODE_01		NORMAL_MODE		// default
// MH321800(S) T.Nagai ICクレジット対応
// ※従来のEdy機能を未使用とする
//#define	FUNCTION_MASK_EDY	DEBUG_MODE
// MH321800(E) T.Nagai ICクレジット対応

/*														*/
/*------------------------------------------------------*/

// GG124100(S) R.Endo 2022/08/29 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
#define DEBUG_MESSAGEFULL 0		// 1の場合にメッセージバッファフル調査用のデバッグ機能が動作する
// 詳細：メッセージバッファが残り100未満になるとA9999を発行し、150以上になると解除する。
//       A9999の4桁アラーム情報には、上位1桁に使用量の多いタスクID、下位3桁に取得可能なバッファの変化量が入る。
//       ※未使用のアラームを使用すること。
#if DEBUG_MESSAGEFULL
#define DEBUG_ALARM_ACT 0x0045	// デバッグ用アラーム：Alarm Action
#define DEBUG_ALARM_MD 99		// デバッグ用アラーム：Alarm Module Code
#define DEBUG_ALARM_NO 99		// デバッグ用アラーム：Alarm No.
#endif
// GG124100(E) R.Endo 2022/08/29 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]

// MH810105(S) R.Endo 2021/12/21 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
#define DEBUG_JOURNAL_PRINT	0	// 1の場合にデバッグ情報をジャーナルに印字する
// MH810105(E) R.Endo 2021/12/21 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発

#define ARCSENDCLASSIC		//従来方式とする

// MH810103 GG119202(S) DT10のSPIドライバ対応
/*--------------------------------------------------------------------------------------*/
/*	DT10のSPIドライバ																	*/
/*--------------------------------------------------------------------------------------*/
// DT10を使用する場合は下記defineを有効にしてdt_spi_drv.cをCubeSuite+のプロジェクトに追加してください
//#define	DT10_SPI					1		// SPIドライバあり

#ifdef	DT10_SPI
extern void Rspi_DT10_Init(void);
#endif	// DT10_SPI
// MH810103 GG119202(E) DT10のSPIドライバ対応

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾗｯﾊﾟｼｽﾃﾑ設定値                                                        |*/
/*[]----------------------------------------------------------------------[]*/
#define		LOCK_IF_MAX		31								/* I/F盤最大接続台数			*/
#define		LOCK_IFS_MAX	12								/* I/F盤子機最大接続台数(1親機に対して)	*/
#define		LOCK_IF_REN_MAX	54								/* 親I/F盤に対するﾛｯｸ装置連番の最大数	*/

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用SRAM削減）
//#define		LOCK_MAX		324								/* 新ﾛｯｸ装置最大接続数			*/
//#define		OLD_LOCK_MAX	324								/* 旧ﾛｯｸ装置最大接続数			*/
#define		LOCK_MAX		150								/* 新ﾛｯｸ装置最大接続数			*/
#define		OLD_LOCK_MAX	150								/* 旧ﾛｯｸ装置最大接続数			*/
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用SRAM削減）
#define		SYU_LOCK_MAX	30								/* 集計エリア格納車室最大数		*/
#define		FLAP_IF_MAX		20								/* ﾌﾗｯﾌﾟI/F盤最大接続台数（ﾀｰﾐﾅﾙ数）	*/
#define		FLAP_IFS_MAX	10								/* ﾌﾗｯﾌﾟ接続台数（１ﾀｰﾐﾅﾙに対して）		*/
#define		INT_FLAP_MAX	20								/* ﾌﾗｯﾌﾟ最大接続台数					*/

#define		CAR_START_INDEX		0
#define		INT_CAR_START_INDEX		50
#ifndef	BIKE_START_INDEX
#define		BIKE_START_INDEX	100
#endif
#ifndef	CAR_LOCK_MAX
#define 	CAR_LOCK_MAX		50
#endif
#ifndef	BIKE_LOCK_MAX
#define 	BIKE_LOCK_MAX		50
#endif
#ifndef	INT_CAR_LOCK_MAX
#define 	INT_CAR_LOCK_MAX	20
#endif
#ifndef	TOTAL_CAR_LOCK_MAX
#define 	TOTAL_CAR_LOCK_MAX	(CAR_LOCK_MAX+INT_CAR_LOCK_MAX)
#endif

/*[]----------------------------------------------------------------------[]*/
/*| ｳｫｯﾁﾄﾞｯｸﾞﾀｲﾏｰﾘｾｯﾄ                                                      |*/
/*[]----------------------------------------------------------------------[]*/

/*[]----------------------------------------------------------------------[]*/
/*| 年月日時分初期値                                                       |*/
/*[]----------------------------------------------------------------------[]*/
#define		Clock_YEAR		2005							/* 年初期値						*/
#define		Clock_MONT		1								/* 月初期値						*/
#define		Clock_DAY		1								/* 日初期値						*/
#define		Clock_HOUR		0								/* 時初期値						*/
#define		Clock_MINU		0								/* 分初期値						*/

/*[]----------------------------------------------------------------------[]*/
/*| ﾒｯｾｰｼﾞ送信型LAGﾀｲﾏｰ（各機能毎のﾀｲﾏｰ数）                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| ﾀｲﾏｰのMAX値や他のﾀｽｸに増やしてもOK。10個位なら問題ありません。         |*/
/*| 20msの割込みを使用しているのでその範囲内で増やしてください。           |*/
/*[]----------------------------------------------------------------------[]*/
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
//#define		PRN_TIM_MAX		8								// ﾌﾟﾘﾝﾀﾀｽｸ ﾀｲﾏｰ数
#define		PRN_TIM_MAX		10								// ﾌﾟﾘﾝﾀﾀｽｸ ﾀｲﾏｰ数
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
															// 1:ﾌﾟﾘﾝﾀBUSY監視ﾀｲﾏｰ（ﾚｼｰﾄ）
															// 2:ﾌﾟﾘﾝﾀBUSY監視ﾀｲﾏｰ（ｼﾞｬｰﾅﾙ）
															// 3:ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞ監視ﾀｲﾏｰ（ﾚｼｰﾄ）
															// 4:ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞ監視ﾀｲﾏｰ（ｼﾞｬｰﾅﾙ）
															// 5:印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ（ﾚｼｰﾄ）
															// 6:印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ（ｼﾞｬｰﾅﾙ）
															// 7:ﾚｼｰﾄ印字完了後ﾀｲﾏｰ
															// 8:ｼﾞｬｰﾅﾙ印字完了後ﾀｲﾏｰ
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
															// 9:時刻／初期設定待ちタイムアウト
															// 10:データ書込み完了待ちタイマ
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）

#define		NTNET_TIM_MAX	2								/* NTNETﾀｽｸ ﾀｲﾏｰ数				*/
#define		NTNETDOPA_TIM_MAX	2							/* NTNET-DOPAﾀｽｸ ﾀｲﾏｰ数			*/
#define		REMOTEDL_TIM_MAX	5							/* 遠隔ﾀﾞｳﾝﾛｰﾄﾞﾀｽｸ ﾀｲﾏｰ数		*/
#define		CAN_TIM_MAX	2									/* CANﾀｽｸ ﾀｲﾏｰ数				*/
// MH810100(S) K.Onodera 2019/10/31 車番チケットレス（LCD通信）
// PKTﾀｽｸ ﾀｲﾏｰ
enum{
	PKT_TIM_CONNECT = 1,									// 接続コマンド開始タイマ
	PKT_TIM_CON_RES_WT,										// コネクション応答待ち時間
	PKT_TIM_CON_INTVL,										// コネクション送信間隔
	PKT_TIM_CON_DIS_WT,										// コネクション切断待ち時間
	PKT_TIM_RES_WT,											// パケット応答待ち時間
	PKT_TIM_KEEPALIVE,										// キープアライブ送信間隔
	PKT_TIM_RTPAY_RES,										// 精算応答データ待ちタイマ
	PKT_TIM_DC_QR_RES,										// QR確定・取消応答データ待ちタイマ
	PKT_TIM_RTPAY_RESND,									// 再送ウェイトタイマ
	PKT_TIM_DC_QR_RESND,									// 再送ウェイトタイマ
	PKT_TIM_DISCON_WAIT,									// コネクション切断検出待ちタイマ
// MH810100(S) K.Onodera 2020/03/12 車番チケットレス(接続エラーガードタイマ追加)
	PKT_TIM_ERR_GUARD,										// 接続エラーガードタイマ
// MH810100(E) K.Onodera 2020/03/12 車番チケットレス(接続エラーガードタイマ追加)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	PKT_TIM_DC_LANE_RES,									// レーンモニタ応答データ待ちタイマ
	PKT_TIM_DC_LANE_RESND,									// 再送ウェイトタイマ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	PKT_TIM_RTRECEIPT_RES,									// 領収証データ応答待ちタイマ
	PKT_TIM_RTRECEIPT_RESND,								// 再送ウェイトタイマ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	// ↑追加はこの上に
	PKT_TIM_END
};
#define PKT_TIM_MAX	 (PKT_TIM_END - 1)	// PKTﾀｽｸ ﾀｲﾏｰ数
// MH810100(E) K.Onodera 2019/10/31 車番チケットレス（LCD通信）

/*[]----------------------------------------------------------------------[]*/
/*| 関数Call型LAGﾀｲﾏｰ定義(各ｲﾝﾀｰﾊﾞﾙﾀｲﾏｰで使用可能なﾀｲﾏｰ数他)               |*/
/*[]----------------------------------------------------------------------[]*/
/*** x10ms Lag timer ***/
#define		LAG10_MAX		1								/* 10msｲﾝﾀｰﾊﾞﾙﾀｲﾏｰ数(base 2ms)	*/

/*** x20ms Lag timer ***/

enum{
	LAG20_RD_SHTCTL = 0,									/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ制御			*/
	LAG20_CN_SHTCTL,										/* ｺｲﾝ投入口ｼｬｯﾀｰ制御			*/
	LAG20_OUTCNT1,											/* 精算完了信号1(出庫信号1)制御	*/
	LAG20_OUTCNT2,                                          /* 精算完了信号2(出庫信号2)制御	*/
	LAG20_GATEOPN,                                          /* ｹﾞｰﾄ開信号制御				*/
	LAG20_GATECLS,                                          /* ｹﾞｰﾄ閉信号制御				*/
	LAG20_RD_SHTCLS,										/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉待ちﾀｲﾏｰ		*/
	LAG20_KEY_REPEAT,
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	LAG20_JNL_PRI_WAIT_TIMER,								// ジャーナル印字完了待ち
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	
	// タイマー追加する場合には上記より上に記載すること
	LAG20_MAX
};


/*** x500ms Lag timer ***/
#define		LAG500_PRINTER_ERR_RECIEVE	0					/* ﾌﾟﾘﾝﾀｴﾗｰ受信割り込みの連続発生防止の為				*/
#define		LAG500_MIF_WRITING_FAIL_BUZ	1					/* MIFARE書込みNG用ﾌﾞｻﾞｰ */
#define		LAG500_MIF_LED_ONOFF		2					/* ICC LED 点滅用		 */
#define		LAG500_SUICA_STATUS_TIMER			3			/* SX-10状態監視ﾀｲﾏ */
#define		LAG500_SUICA_NONE_TIMER				4			/* 制御データの送信時の無応答(受付不可) */
#define		LAG500_SUICA_MIRYO_RESET_TIMER		5			/* 未了状態解除ﾀｲﾏｰ */
#define		LAG500_SUICA_ZANDAKA_RESET_TIMER	6			/* Suika残高不足表示解除ﾀｲﾏｰ */
#define		LAG500_RECEIPT_LEDOFF_DELAY			7			/* 精算完了後 領収書発行時の取出し口LED点灯時間 */
#define		LAG500_EDY_LED_RESET_TIMER			8			/* EDYLED解除ﾀｲﾏｰ */
#define		LAG500_VIB_SNS_TIMER				9			/* 振動検知ﾀｲﾏｰ */

enum {
	LAG500_iDC_TIMER = 10,
	LAG500_ERROR_DISP_DELAY,
	LAG500_CREDIT_ANA_TIMER,
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	LAG500_EDY_STOP_RECV_TIMER,								// Edy停止応答受信待ちタイマー（設定＋10秒）
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	LAG500_REMOTEDL_RETRY_CONNECT_TIMER,					// 遠隔ダウンロード用接続リトライタイマ
	LAG500_DISCONNECT_WAIT_TIMER,							// Rism切断待ちタイマー
	LAG500_CAN_HEARTBEAT_TIMER,								// ハートビート送信周期タイマー
	LAG500_CAPPI_CONNECT_TIMER,								// Cappi TCPコネクト待ちタイマ
	LAG500_CAPPI_CLOSE_TIMER,								// Cappi TCP切断待ちタイマ
	LAG500_SUICA_NO_RESPONSE_TIMER,							/* 制御データの送信時の無応答用(受付可) */
// MH321800(S) G.So ICクレジット対応
	LAG500_EC_NOT_READY_TIMER,								// 制御データの送信時の無応答用(受付可)
	LAG500_EC_AUTO_CANCEL_TIMER,							// アラーム取引後の精算自動キャンセルタイマ
	LAG500_EC_WAIT_BOOT_TIMER,								// 決済リーダ起動完了待ちタイマー
	LAG500_EC_START_REMOVAL_TIMER,							// 決済リーダカード抜き取り待ち起動タイマー
// MH321800(E) G.So ICクレジット対応
// MH810104 GG119201(S) 電子ジャーナル対応
	LAG500_EJA_PINFO_RES_WAIT_TIMER,						// 電子ジャーナルプリンタ情報応答待ちタイマ
	LAG500_EJA_RESET_WAIT_TIMER,							// 電子ジャーナルリセット完了待ちタイマ
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810105(S) MH364301 インボイス対応
	LAG500_JNL_PRI_WAIT_TIMER,								// ジャーナル印字完了待ち
	LAG500_RECEIPT_MISS_DISP_TIMER,							// 領収証失敗表示タイマ
// MH810105(E) MH364301 インボイス対応
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	LAG500_RCT_PINFO_RES_WAIT_TIMER,						// レシートプリンタ情報応答待ちタイマ
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	// 今後追加タイマーはここに追加
	LAG500_MAX
};


/*[]----------------------------------------------------------------------[]*/
/*| TCB,MCB制御                                                            |*/
/*[]----------------------------------------------------------------------[]*/
#define		MSGBUF_CNT		512								/* number of MCB max			*/
#define		MSGBUFSZ		64								/* message buffer size			*/

enum{
	OPETCBNO = 0,
	PRNTCBNO,
	FLPTCBNO,
	NTNETTCBNO,
	NTNETDOPATCBNO,
	REMOTEDLTCBNO,												/* TCB For Remote Download task	*/
	CANTCBNO,
	CAPPITCBNO,
// MH810100(S) K.Onodera 2019/10/31 車番チケットレス（LCD通信）
	PKTTCBNO,
// MH810100(E) K.Onodera 2019/10/31 車番チケットレス（LCD通信）
	// ↑今後はこの上に追加していくこと↑
	TCB_CNT
};

#define		MSG_SET			0x01							/*								*/
#define		MSG_EMPTY		0x00							/*								*/

/*[]----------------------------------------------------------------------[]*/
/*| ﾀｽｸ番号定義                                                            |*/
/*[]----------------------------------------------------------------------[]*/
enum {
			IDLETSKNO = 0,									/* idle task					*/
			OPETSKNO,										/* operation task				*/
			CNMTSKNO,										/* coinmech and notereader task */
			PRNTSKNO,										/* printer task					*/
// MH810100(S) K.Onodera 2019/10/31 車番チケットレス（LCD通信）
//			LKCOMTSKNO,										/* lkcom task					*/
			PKTTSKNO,										// PKT task
// MH810100(E) K.Onodera 2019/10/31 車番チケットレス（LCD通信）
			FLATSKNO,										/* flashrom task				*/
			NTNETTSKNO,										/* nt-net task					*/
			REMOTEDLTSKNO,									/* Remote Download task			*/
			CANTSKNO,										/* can task						*/
			MRDTSKNO,										/* Mag Reader task				*/
			KSGTSKNO,
			IDLESUBTSKNO,									/* idle sub task				*/
			PIPTSKNO,										/* ParkiPro task				*/
			RAUTSKNO,										/* rau task						*/
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
			NTCOMTSKNO,										/* nt task(NT-NET通信処理部)	*/
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
			TSKMAX											/* count of task */
};
/*[]----------------------------------------------------------------------[]*/
/*| ﾎﾟｰﾄ割り付け定義                                                       |*/
/*[]----------------------------------------------------------------------[]*/

#define		FNT_DIPSW_1		key_dat[0].BIT.B7				/* ディップスイッチ1			*/
#define		FNT_DIPSW_2		key_dat[0].BIT.B6				/* ディップスイッチ2			*/
#define		FNT_DIPSW_3		key_dat[0].BIT.B5				/* ディップスイッチ3			*/
#define		FNT_DIPSW_4		key_dat[0].BIT.B4				/* ディップスイッチ4			*/

#define		TENKEY_0		key_dat[1].BIT.B7				/* テンキー0					*/
#define		TENKEY_1		key_dat[1].BIT.B6				/* テンキー1					*/
#define		TENKEY_2		key_dat[1].BIT.B5				/* テンキー2					*/
#define		TENKEY_3		key_dat[1].BIT.B4				/* テンキー3					*/
#define		TENKEY_4		key_dat[1].BIT.B3				/* テンキー4					*/
#define		TENKEY_5		key_dat[1].BIT.B2				/* テンキー5					*/
#define		TENKEY_6		key_dat[1].BIT.B1				/* テンキー6					*/
#define		TENKEY_7		key_dat[1].BIT.B0				/* テンキー7					*/

#define		TENKEY_8		key_dat[2].BIT.B7				/* テンキー8					*/
#define		TENKEY_9		key_dat[2].BIT.B6				/* テンキー9					*/
#define		TENKEY_F1		key_dat[2].BIT.B5				/* テンキーF1(精算)				*/
#define		TENKEY_F2		key_dat[2].BIT.B4				/* テンキーF2(領収書)			*/
#define		TENKEY_F3		key_dat[2].BIT.B3				/* テンキーF3(駐車証明書)		*/
#define		TENKEY_F4		key_dat[2].BIT.B2				/* テンキーF4(取消)				*/
#define		TENKEY_F5		key_dat[2].BIT.B1				/* テンキーF5(登録)				*/
#define		TENKEY_C		key_dat[2].BIT.B0				/* テンキーC					*/

#define		FNT_CN_DRSW		key_dat[3].BIT.B7				/* コインメック鍵				*/
#define		FNT_NT_BOX_SW	key_dat[3].BIT.B6				/* 紙幣リーダー鍵				*/
#define		FNT_CN_BOX_SW	key_dat[3].BIT.B5				/* コイン金庫脱着				*/
#define		FNT_NT_FALL_SW	key_dat[3].BIT.B4				/* 紙幣リーダー脱落				*/
#define		FNT_VIB_SNS		key_dat[3].BIT.B3				/* 振動センサー					*/
#define		FNT_MAN_DET		key_dat[3].BIT.B2				/* 人体検知センサー				*/

#define		RT_SW0			port_in_dat.BIT.B0				/* (RTSW0)					*/
#define		RT_SW1			port_in_dat.BIT.B1				/* (RTSW1)					*/
#define		RT_SW2			port_in_dat.BIT.B2				/* (RTSW2)					*/
#define		RT_SW3			port_in_dat.BIT.B3				/* (RTSW3)					*/
#define		DIP_SW0			port_in_dat.BIT.B4				/* (DIPSW0)					*/
#define		DIP_SW1			port_in_dat.BIT.B5				/* (DIPSW1)					*/
#define		DIP_SW2			port_in_dat.BIT.B6				/* (DIPSW2)					*/
#define		DIP_SW3			port_in_dat.BIT.B7				/* (DIPSW3)					*/

#define		FNTH_OFF		port_in_dat.BIT.B10				/* (TH_OFF)					*/
#define		FNTH_ON			port_in_dat.BIT.B11				/* (TH_ON)					*/
#define		SC_KEY			port_in_dat.BIT.B12				/* (C_KEY)					*/
#define		SM_KEY			port_in_dat.BIT.B13				/* (M_KEY)					*/
#define		RXM1_IN			port_in_dat.BIT.B14				/* (M_IN)					*/
#define		RXI1_IN			port_in_dat.BIT.B15				/* (RXI_IN)					*/

extern	unsigned char	SD_LCD;								/* ﾊﾞｯｸﾗｲﾄ点灯状態 0=消灯 1=点灯*/
extern	unsigned short	SD_EXIOPORT;						/* 拡張I/Oポートイメージバッファ*/

/*[]----------------------------------------------------------------------[]*/
/*| ﾎﾟｰﾄ割り付け定義                                                       |*/
/*[]----------------------------------------------------------------------[]*/
#define		CP_CN_SYN		PORT8.PODR.BIT.B6				/* COINMECH SYN					*/

#define		CP_FB_RES		PORT3.PODR.BIT.B2				/* FB-7000のリセット出力		*/

#define		CP_RES_DET1		PORT1.PODR.BIT.B1				/* 予備ﾎﾞﾀﾝ1					*/

#define		CP_IF_RTS		PORT7.PODR.BIT.B0				/* APS通信 IF盤通信 RTS制御信号		*/
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#define		CP_NT_RTS		PORT6.PODR.BIT.B0				/* NT-NET通信 RTS制御信号		*/
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#define		CP_CAN_RES		PORT6.PODR.BIT.B6				/* CANのﾘｾｯﾄ出力				*/

#define		CP_RED_SHUT_OPEN	PORT0.PODR.BIT.B3			// 磁気リーダーシャッタオープン
#define		CP_RED_SHUT_CLOSE	PORT0.PODR.BIT.B5			// 磁気リーダーシャッタクローズ
#define		CP_FAN_CTRL_SW		PORT6.PODR.BIT.B1			// FAN制御SW
#define		CP_DOOR_OPEN_SEN	PORT6.PIDR.BIT.B2			// ドア開センサ
#define		CP_RD_SHUTLED		PORT8.PODR.BIT.B7			// 磁気リーダーシャッタLED

#define		CP_MODECHG		(0 == OPE_SIG_DOORNOBU_Is_OPEN)	// ドアノブ状態を設定ﾓｰﾄﾞｽｲｯﾁに代用 0:ON 1:OFF

#define		MISC0_CP_SUB_CPU	0x80						/* サブCPUリセット制御 H=リセット         書き込みはOutMISC0wtを使用，読み出しはSD_MISC0を参照すること*/

#define		MISC2_CP_CPU_HWRST2	0x08						/* CPUハードウェアリセット制御 H=リセット 書き込みはOutMISC2wtを使用，読み出しはSD_MISC2を参照すること*/
#define		MISC2_CP_PRN2_RES	0x10						/* レシートプリンタ2リセット H=リセット   書き込みはOutMISC2wtを使用，読み出しはSD_MISC2を参照すること*/
#define		MISC2_CP_RAM_WPT	0x20						/* RAM WRITE PROTECT                      書き込みはOutMISC2wtを使用，読み出しはSD_MISC1を参照すること*/
#define		MISC2_CP_LD1		0x40						/* LD1点灯制御 H=点灯                     書き込みはOutMISC2wtを使用，読み出しはSD_MISC2を参照すること*/
#define		MISC2_CP_LD2		0x80						/* LD2点灯制御 H=点灯                     書き込みはOutMISC2wtを使用，読み出しはSD_MISC2を参照すること*/
/*[]----------------------------------------------------------------------[]*/
/*| Sodiac定義   		                                                   |*/
/*[]----------------------------------------------------------------------[]*/
#define		SODIAC_MUTE_SW		PORT0.PODR.BIT.B2			/*	SodiacMute switch	*/

/*----------------------------------*/
/* CRC-CCITT :  x^{16}+x^{12}+x^5+1 */
/*----------------------------------*/
#define		CRCPOLY1			0x1021						/* 左ｼﾌﾄ						*/
#define		CRCPOLY2			0x8408  					/* 右ｼﾌﾄ						*/
#define		CHAR_BIT			8							/* number of bits in a char		*/
#define		L_SHIFT				0							/* 左ｼﾌﾄ						*/
#define		R_SHIFT				1							/* 右ｼﾌﾄ						*/

/*** structure ***/
/*[]----------------------------------------------------------------------[]*/
/*| ﾀｲﾏｰ変数                                                               |*/
/*[]----------------------------------------------------------------------[]*/
extern	unsigned short OPE_Timer[];							/* ｵﾍﾟﾚｰｼｮﾝﾀｽｸ用ﾀｲﾏｰ			*/
extern	unsigned short PRN_Timer[PRN_TIM_MAX];				/* ﾌﾟﾘﾝﾀﾀｽｸ用ﾀｲﾏｰ				*/
extern	unsigned short NTNET_Timer[NTNET_TIM_MAX];			/* NTNETﾀｽｸ用ﾀｲﾏｰ				*/
extern	unsigned short NTNETDOPA_Timer[NTNETDOPA_TIM_MAX];	/* NTNET-DOPAﾀｽｸ用ﾀｲﾏｰ			*/
extern	unsigned short REMOTEDL_Timer[REMOTEDL_TIM_MAX];	/* NTNET-DOPAﾀｽｸ用ﾀｲﾏｰ			*/
extern	unsigned short CAN_Timer[CAN_TIM_MAX];				/* CANﾀｽｸ用ﾀｲﾏｰ					*/
// MH810100(S) K.Onodera 2019/10/31 車番チケットレス（LCD通信）
extern	unsigned short PKT_Timer[PKT_TIM_MAX];				// PKTﾀｽｸ用ﾀｲﾏｰ
// MH810100(E) K.Onodera 2019/10/31 車番チケットレス（LCD通信）

/*[]----------------------------------------------------------------------[]*/
/*| ｷｰｽｷｬﾝ関連定義                                                         |*/
/*[]----------------------------------------------------------------------[]*/
#define		KeyDatMax		4								/* ｽｷｬﾝﾗｲﾝ数					*/
// MH810100(S) K.Onodera 2019/11/06 車番チケットレス(精算機－LCD間通信機能追加)
#define		KeyDatMaxLCD	2								/* LCDからのボタン状態Byte数	*/
// MH810100(E) K.Onodera 2019/11/06 車番チケットレス(精算機－LCD間通信機能追加)
#define		PortInTblMax	16								/* 入力ﾎﾟｰﾄ数					*/

union	key_rec {
	unsigned char BYTE;
	struct {
		unsigned char B7:1;									/* Bit 7						*/
		unsigned char B6:1;									/* Bit 6						*/
		unsigned char B5:1;									/* Bit 5						*/
		unsigned char B4:1;									/* Bit 4						*/
		unsigned char B3:1;									/* Bit 3						*/
		unsigned char B2:1;									/* Bit 2						*/
		unsigned char B1:1;									/* Bit 1						*/
		unsigned char B0:1;									/* Bit 0						*/
	} BIT;
};

extern	union	key_rec	key_dat[KeyDatMax];					/* ｷｰｽｷｬﾝﾃﾞｰﾀｲﾒｰｼﾞ				*/
// MH810100(S) K.Onodera 2019/11/06 車番チケットレス(精算機－LCD間通信機能追加)
extern	union	key_rec	key_dat_LCD[KeyDatMaxLCD];			/* LCDから受信した前ボタン状態	*/
// MH810100(E) K.Onodera 2019/11/06 車番チケットレス(精算機－LCD間通信機能追加)
union	port_in_rec {
	unsigned short WORD;
	struct {
		unsigned char B15:1;								/* Bit 15						*/
		unsigned char B14:1;								/* Bit 14						*/
		unsigned char B13:1;								/* Bit 13						*/
		unsigned char B12:1;								/* Bit 12						*/
		unsigned char B11:1;								/* Bit 11						*/
		unsigned char B10:1;								/* Bit 10						*/
		unsigned char B9:1;									/* Bit 9						*/
		unsigned char B8:1;									/* Bit 8						*/
		unsigned char B7:1;									/* Bit 7						*/
		unsigned char B6:1;									/* Bit 6						*/
		unsigned char B5:1;									/* Bit 5						*/
		unsigned char B4:1;									/* Bit 4						*/
		unsigned char B3:1;									/* Bit 3						*/
		unsigned char B2:1;									/* Bit 2						*/
		unsigned char B1:1;									/* Bit 1						*/
		unsigned char B0:1;									/* Bit 0						*/
	} BIT;
};
extern	union	port_in_rec	port_in_dat;					/* 入力ﾎﾟｰﾄﾃﾞｰﾀｲﾒｰｼﾞ			*/
extern	const	unsigned short scn_evt_tbl[16];				/* ｲﾍﾞﾝﾄ№登録					*/
extern	unsigned short	port_data[3];						/* 拡張I/Oﾎﾟｰﾄﾁｬﾀ処理用ﾊﾞｯﾌｧ	*/
extern	unsigned char	door_data[3];						/* ドア開センサ情報ﾁｬﾀ処理用	*/
extern	unsigned char	doorDat;							/* 前回ドア開センサ情報			*/
extern	unsigned char	doornobu_data[3];					/* ドアノブセンサ情報ﾁｬﾀ処理用	*/
extern	unsigned char	doornobuDat;						/* 前回ドアノブセンサ情報		*/
extern	char		port_indat_onff[PortInTblMax];			// 拡張I/Oポートのon/offフラグ
extern	char		door_indat_onff;						// ドア開センサのon/offフラグ
extern	uchar		f_port_scan_OK;							// 割り込みスキャン許可フラグ 0:禁止 1:許可

extern	volatile unsigned char	IN_TIM_Cnt;					/* 現在のｷｰｽｷｬﾝの種類			*/

/*[]----------------------------------------------------------------------[]*/
/*| ﾌﾞｻﾞｰ関連定義                                                          |*/
/*[]----------------------------------------------------------------------[]*/

/*[]----------------------------------------------------------------------[]*/
/*| 時計定義                                                               |*/
/*[]----------------------------------------------------------------------[]*/
															/*------------------------------*/
struct	clk_rec	{											/* Clock Data					*/
															/* ---------------------------- */
	unsigned short	year;									/* Year							*/
	unsigned char	mont;									/* Month (1-12)					*/
	unsigned char	date;									/* Date  (1-31)					*/
	unsigned char	hour;									/* Hour  (0-23)					*/
	unsigned char	minu;									/* Minute(0-59)					*/
	unsigned char	seco;									/* Second(0-59)					*/
	unsigned char	week;									/* Day   (0:SUN-6:SAT)			*/
	unsigned short	ndat;									/* Normalize Date				*/
	unsigned short	nmin;									/* Normalize Minute				*/
															/*------------------------------*/
};
															/*------------------------------*/
extern	struct	clk_rec	CLK_REC;							/* Clock Data Recive Buffer		*/
															/*------------------------------*/
extern	struct	clk_rec	clk_save;							// 停電時刻

#define	_1DAY_TIME_MILLI_SEC		86400000L				// 24時間をms単位で表現した値
/*[]----------------------------------------------------------------------[]*/
/*| 年月日時分ノーマライズ構造体                                           |*/
/*[]----------------------------------------------------------------------[]*/
typedef	union {
															/*------------------------------*/
	ulong	ul;												/* b31-16=年月日, b15-0=時分	*/
	ushort	us[2];											/* [0]=年月日, [1]=時分			*/
															/*------------------------------*/
} t_NrmYMDHM;
#pragma pack
/*[]----------------------------------------------------------------------[]*/
/*| 日付構造体                                                             |*/
/*[]----------------------------------------------------------------------[]*/
// MH810100(S)
typedef	struct {
															/*------------------------------*/
	unsigned short	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
	unsigned char	Hour;									/* Hour							*/
	unsigned char	Min;									/* Minute						*/
	unsigned char	Sec;									/* Second						*/
															/*------------------------------*/
} DATE_YMDHMS;
// MH810100(E)

// MH810100(S) K.Onodera 2020/01/12 車番チケットレス（QR確定・取消データ）
typedef	struct {
															/*------------------------------*/
	unsigned short	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
															/*------------------------------*/
} DATE_YMD;
// MH810100(E) K.Onodera 2020/01/12 車番チケットレス（QR確定・取消データ）

typedef	struct {
															/*------------------------------*/
	unsigned short	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
	unsigned char	Hour;									/* Hour							*/
	unsigned char	Min;									/* Minute						*/
															/*------------------------------*/
}	date_time_rec;

/*[]----------------------------------------------------------------------[]*/
/*| 日付構造体2                                                            |*/
/*[]----------------------------------------------------------------------[]*/
typedef	struct {
															/*------------------------------*/
	unsigned short	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
	unsigned char	Hour;									/* Hour							*/
	unsigned char	Min;									/* Minute						*/
	unsigned short	Sec;									/* Second						*/
															/*------------------------------*/
}	date_time_rec2;

/*[]----------------------------------------------------------------------[]*/
/*| 日付構造体3                                                            |*/
/*[]----------------------------------------------------------------------[]*/
typedef	struct {
															/*------------------------------*/
	unsigned char	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
	unsigned char	Hour;									/* Hour							*/
	unsigned char	Min;									/* Minute						*/
	unsigned char	Sec;									/* Second						*/
															/*------------------------------*/
}	date_time_rec3;
#pragma unpack

/*[]----------------------------------------------------------------------[]*/
/*| ﾒｯｾｰｼﾞ定義                                                             |*/
/*[]----------------------------------------------------------------------[]*/
typedef	struct {
															/*------------------------------*/
	unsigned short	command;								/* command						*/
															/*------------------------------*/
	unsigned char	data[MSGBUFSZ];							/* infomation					*/
															/*------------------------------*/
}	MSG;

typedef	struct {
															/*------------------------------*/
	char	*msg_next;										/* 次ﾊﾞｯﾌｧｱﾄﾞﾚｽ					*/
															/*------------------------------*/
	MSG		msg;											/* 								*/
															/*------------------------------*/
} MsgBuf;

extern	MsgBuf	msgbuf[MSGBUF_CNT];

typedef struct {
															/*------------------------------*/
	MsgBuf		*msg_top;									/* ﾒｯｾｰｼﾞ先頭ｱﾄﾞﾚｽ				*/
															/*------------------------------*/
	MsgBuf		*msg_end;									/* ﾒｯｾｰｼﾞ最終ｱﾄﾞﾚｽ				*/
															/*------------------------------*/
} MCB;

extern	MCB		mcb;

typedef	struct {
															/*------------------------------*/
	unsigned char 	event;									/* 起動 1(ON)/0(OFF)            */
															/*------------------------------*/
	unsigned char	level;									/* 優先順位7～0                 */
															/*------------------------------*/
	unsigned short	status;									/* ﾀｽｸｽﾃｰﾀｽ						*/
															/*------------------------------*/
	MsgBuf			*msg_top;								/* 先頭ｲﾍﾞﾝﾄﾊﾞｯﾌｧｱﾄﾞﾚｽ			*/
															/*------------------------------*/
	MsgBuf			*msg_end;								/* 最終ｲﾍﾞﾝﾄﾊﾞｯﾌｧｱﾄﾞﾚｽ			*/
															/*------------------------------*/
} TCB;
#define	TARGET_MSGGET_PRM_MAX	8
typedef struct {
	unsigned short	Count;									/* 登録件数						*/
	unsigned short	Command[TARGET_MSGGET_PRM_MAX];			/* ﾒｯｾｰｼﾞID						*/
} t_TARGET_MSGGET_PRM;

extern	TCB		tcb[];

extern	char	CNMTSK_START;								/* ｺｲﾝﾒｯｸﾀｽｸ起動ﾌﾗｸﾞ			*/
extern	char	RMTCOM_START;								/* 遠隔通信起動ﾌﾗｸﾞ				*/
extern	char	PCCOM_START;								/* PC通信&遠隔通信起動ﾌﾗｸﾞ		*/
extern	unsigned short	SUBCPU_MONIT;						/* ｻﾌﾞCPU監視ﾀｲﾏｰ				*/
extern	unsigned char	WPTCOUNT;							/* write protect off count		*/
extern	unsigned long	LifeTimer10ms;
extern	unsigned char	Tim10msCount;						/* 10ms積算カウンタ(every 2ms + 1) */
extern	unsigned char	xPauseStarting;						/* xPause()でｵﾍﾟﾀｽｸを起動させる */
extern	unsigned char	xPauseStarting_PRNTSK;				/* xPause()でﾌﾟﾘﾝﾀﾀｽｸを起動させる */
extern	unsigned char	Tim500msCount;						/* 500ms積算カウンタ(every 20ms + 1) */
extern	unsigned char	TIM500_START;						/* 500ms interval経過フラグ (0=経過してない、以外はN回経過) */
extern	unsigned char	SIGCHK_TRG;							/* 出力信号ﾁｪｯｸ、他ﾁｪｯｸのﾄﾘｶﾞ用	*/
typedef union{
	unsigned short	USHORT;
	struct{
		unsigned char	RESERVE_BIT15		:1;						// Bit 15 = 予備
		unsigned char	RESERVE_BIT14		:1;						// Bit 14 = 予備
		unsigned char	RESERVE_BIT13		:1;						// Bit 13 = 予備
		unsigned char	RESERVE_BIT12		:1;						// Bit 12 = 予備
		unsigned char	RESERVE_BIT11		:1;						// Bit 11 = 予備
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
//		unsigned char	RESERVE_BIT10		:1;						// Bit 10 = 予備
		unsigned char	R_PRINTER_RCV_INFO	:1;						// Bit 10 = レシートプリンタ情報応答受信
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// MH810104 GG119201(S) 電子ジャーナル対応
//		unsigned char	RESERVE_BIT9		:1;						// Bit 9 = 予備
//		unsigned char	RESERVE_BIT8		:1;						// Bit 8 = 予備
		unsigned char	EJA_RCV_STS			:1;						// Bit 9 = 電子ジャーナルプリンタステータス受信
		unsigned char	J_PRINTER_RCV_INFO	:1;						// Bit 8 = ジャーナルプリンタ情報応答受信
// MH810104 GG119201(E) 電子ジャーナル対応
// Sc16RecvInSending_IRQ5_Level_L
//		unsigned char	RESERVE_BIT7		:1;						// Bit 7 = 予備
		unsigned char	Print1ByteSendReq	:1;						// Bit 7 = 予備
		unsigned char	J_PRINTER_NEAR_END	:1;						// Bit 6 = プリンタのニアエンドステータス情報
		unsigned char	J_PRINTER_RCV_CMP	:1;						// Bit 5 = ジャーナルプリンタ受信完了
		unsigned char	J_PRINTER_SND_CMP	:1;						// Bit 4 = ジャーナルプリンタ送信完了
		unsigned char	R_PRINTER_NEAR_END	:1;						// Bit 3 = プリンタのニアエンドステータス情報
		unsigned char	R_PRINTER_RCV_CMP	:1;						// Bit 2 = レシートプリンタ受信完了
		unsigned char	R_PRINTER_SND_CMP	:1;						// Bit 1 = レシートプリンタ送信完了
		unsigned char	RTC_1_MIN_IRQ		:1;						// Bit 0 = RTC分歩進割り込み発生
	} BIT;
} t_I2C_EVENT;

extern	t_I2C_EVENT	I2C_Event_Info;									// I2Cイベント発生
extern	unsigned char	RP_OV_PA_FR;								/* ﾚｼｰﾄのｵｰﾊﾞｰﾗﾝ、ﾊﾟﾘﾃｨ、ﾌﾚｰﾐﾝｸﾞｴﾗｰ発生	*/
extern	unsigned char	JP_OV_PA_FR;								/* ｼﾞｬｰﾅﾙのｵｰﾊﾞｰﾗﾝ、ﾊﾟﾘﾃｨ、ﾌﾚｰﾐﾝｸﾞｴﾗｰ発生	*/
typedef union{
	unsigned short	USHORT;
	struct{
		unsigned char	I2C_BFULL_RECV_PIP		:1;		// 予約(PIP受信バッファフル)	発生/解除別
		unsigned char	I2C_BFULL_SEND_PIP		:1;		// 予約(PIP送信バッファフル)	発生/解除別
		unsigned char	I2C_BFULL_RECV_CAPPI	:1;		// 予約(CAPPI受信バッファフル)	発生/解除別
		unsigned char	I2C_BFULL_SEND_CAPPI	:1;		// 予約(CAPPI送信バッファフル)	発生/解除別
		unsigned char	I2C_BFULL_RECV_JP		:1;		// BIT11 = ジャーナルプリンタ受信バッファフル(『I2Cｼﾞｬｰﾅﾙ受信ﾊﾞｯﾌｧﾌﾙ』)	発生/解除別
		unsigned char	I2C_BFULL_SEND_JP		:1;		// BIT10 = ジャーナルプリンタ送信バッファフル(『I2Cｼﾞｬｰﾅﾙ送信ﾊﾞｯﾌｧﾌﾙ』)	発生/解除別
		unsigned char	I2C_BFULL_RECV_RP		:1;		// BIT 9 = レシートプリンタ受信バッファフル(『I2Cﾚｼｰﾄ受信ﾊﾞｯﾌｧﾌﾙ』)		発生/解除別
		unsigned char	I2C_BFULL_SEND_RP		:1;		// BIT 8 = レシートプリンタ送信バッファフル(『I2Cﾚｼｰﾄ送信ﾊﾞｯﾌｧﾌﾙ』)		発生/解除別
		unsigned char	BIT7					:1;		// BIT 7 = Reserved
		unsigned char	BIT6					:1;		// Bit 6 = Reserved
		unsigned char	BIT5					:1;		// Bit 5 = Reserved
		unsigned char	I2C_PREV_TOUT			:1;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
		unsigned char	I2C_BUS_BUSY			:1;		// Bit 3 = バスBUSY						発生解除同時
		unsigned char	I2C_SEND_DATA_LEVEL		:1;		// Bit 2 = 送信データレベルエラー		発生解除同時
		unsigned char	I2C_CLOCK_LEVEL			:1;		// Bit 1 = クロックレベルエラー			発生解除同時 
		unsigned char	I2C_R_W_MODE			:1;		// Bit 0 = Read/Writeモードエラー		発生解除同時
	} BIT;
} t_I2C_BUSERROR;

typedef struct {
	unsigned char	SendRP;
	unsigned char	RecvRP;
	unsigned char	SendJP;
	unsigned char	RecvJP;
} t_I2C_ERR_STS_EVENT;

extern	t_I2C_BUSERROR			I2C_BusError_Info;
extern	t_I2C_ERR_STS_EVENT		I2cErrEvent;
extern	t_I2C_ERR_STS_EVENT		I2cErrStatus;


// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
extern	unsigned long	LifeTimer1ms;						/* 1ms積算ｶｳﾝﾀ					*/
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
extern	unsigned long	LifeTimer2ms;						/* 2ms積算ｶｳﾝﾀ(Use IFcom)		*/
extern	uchar		Tim1sCount;									// 1s積算カウンタ(every 20ms + 1) 
extern	unsigned short	FBcom_2msT1;
extern	unsigned short	FBcom_2msT2;
extern	unsigned short	FBcom_20msT1;
extern	unsigned short	FBcom_20msT2;
extern	unsigned short	Capcom_2msT1;
extern	unsigned short	Capcom_2msT2;
extern	unsigned short	Capcom_20msT1;
extern	unsigned short	Capcom_20msT2;
extern	unsigned short	Capcom_20msT3;
extern	ushort			Btcom_1msT1;								// 文字間監視タイマー
extern	ushort			Btcom_20msT1;								// 受信待ちタイムアウト用
extern	uchar			f_bluetooth_init;							// bluetoothイニシャルシーケンス制御フラグ
#define FAN_EXE_BUF 10
typedef struct {
	ushort	year;									// Year
	uchar	mont;									// Month (1-12)
	uchar	date;									// Date  (1-31)
	uchar	hour;									// Hour  (0-23)
	uchar	minu;									// Minute(0-59)
	uchar	f_exe;									// 0:データなし 1:FAN ON 2:FAN OFF
} t_fan_time;
typedef struct {
	t_fan_time	fan_exe[FAN_EXE_BUF];
	uchar		index;
} t_fan_time_ctrl;
extern	t_fan_time_ctrl	fan_exe_time;								// FANを駆動開始した時間の格納バッファ
#define	FLT_SWDATA_WRITE_SIZE			256 // FROMの書き込み最少サイズが256BYTEのため
extern unsigned char swdata_write_buf[FLT_SWDATA_WRITE_SIZE];
extern	unsigned short	FLP_TIM_CTRL;
#define		rangechk(c,d,e)	((c)<=(e)&&(e)<=(d))			/* 範囲ﾁｪｯｸ						*/
extern	volatile unsigned short	SHTTER_CTRL;						/* ｼｬｯﾀｰ制御呼び出し			*/

extern	ulong	system_ticks;
extern	uchar	MAC_address[6];

// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
// 入庫用構造体
typedef struct {
	unsigned short count;	// 5secｶｳﾝﾀ
	unsigned short index;	// 車室ｲﾝﾃﾞｯｸｽ 1～100
}t_AutoPayment_In;

// 精算用構造体
typedef struct {
	unsigned char  mode;	// 処理ﾓｰﾄﾞ（0：精算開始／1：車室入力中／2：入金中）
	unsigned short proc_no;	// 精算中の車室ｲﾝﾃﾞｯｸｽ
	unsigned short next_no;	// 次回検索開始ｲﾝﾃﾞｯｸｽ
}t_AutoPayment_Out;

// 自動精算用構造体
typedef struct {
	t_AutoPayment_In	In;
	t_AutoPayment_Out	Out;
	unsigned short AUTOPAY_TRG;
}t_AutoPayment;

extern	t_AutoPayment  AutoPayment;

#endif

#define	SKIP_APASS_CHK	(ryo_buf.apass_off == 1) || (PPrmSS[S_P01][3] == 1)		// どっちかOFF
#define	DO_APASS_CHK	(ryo_buf.apass_off == 0) && (PPrmSS[S_P01][3] == 0)		// どっちもOFF

extern	volatile	uchar	dummy_Read;
extern uchar	x10ms_count;								/* 内部 20msｶｳﾝﾀ			*/
extern uchar	x100ms_count;								/* 内部 100msｶｳﾝﾀ			*/
extern ulong	Now_t;										/* 現在時刻(ms単位)			*/
extern uchar fan_exe_state;	// サーミスタの温度によるFANのポート制御の値,時間帯で強制停止するため復帰させるための退避エリア
							// 0:FAN制御OFF 1:FAN制御ON
extern uchar f_fan_timectrl;// 時刻による強制停止中を示すフラグ 0:通常 1:強制停止中
extern uchar f_SoundIns_OK;	// 音声インストールモードRSW=5の結果フラグ 1:成功 0:失敗

extern const unsigned char prog_update_flag[4];
extern const unsigned char wave_update_flag[4];
extern const unsigned char parm_update_flag[4];
typedef struct{
	unsigned short wave;// 音声データ
	unsigned short parm;// 共通パラメータ
	unsigned short reserve1;
	unsigned short reserve2;
}SW;

// 面切り替えセクション情報
typedef struct{
	unsigned char f_prog_update[4];			// プログラム更新フラグ
	unsigned char f_wave_update[4];			// 音声データ更新フラグ
	unsigned char f_parm_update[4];			// 共通パラメータ更新フラグ
	SW sw;									// 運用面情報
	SW sw_bakup;							// 運用面情報バックアップ(更新中のリカバリ用)
	SW sw_flash;							// フラッシュに書かれた運用面情報
	unsigned short err_count;				// ブートプログラムで発生したエラーの回数(トータル10発生したら復旧不可能とする)
}SWITCH_DATA;

extern SWITCH_DATA	BootInfo;						// 起動時面選択情報
extern unsigned char f_wave_datchk;					// 音声データがあるかないかを示すフラグwave_data_swchk()の戻り値を格納する
extern const unsigned char LOG_VERSION;				// ログバージョン1 それ以前はバージョン無し
extern const unsigned char LOG_PASS_ROM[8];			// ログエリアのパスワードROM値
extern unsigned char LOG_PASS_RAM[8];				// ログエリアのパスワードRAM値
extern uchar bk_log_ver;							// バックアップ時のログバージョン
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
extern uchar LONG_PARK_COUNT;						// 長期駐車検出用タイマカウンタ(アラーム用)
extern uchar LONG_PARK_COUNT2;						// 長期駐車検出用タイマカウンタ(ドア閉アラーム用)
extern uchar LONG_PARK_COUNT3;						// 長期駐車検出用タイマカウンタ(長期駐車ログ用)
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH810100(S) K.Onodera 2020/01/17 車番チケットレス(LCDリセット通知対応)
extern uchar SystemResetFlg;						// システムリセット実行フラグ
// MH810100(E) K.Onodera 2020/01/17 車番チケットレス(LCDリセット通知対応)

// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
// ParkingWeb/RealTime精算中止データ送信制御
#define	PAY_CAN_DATA_SEND	0	// 0:精算中止データ送信しない/1:する
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:精算中止時、PW/RT精算データを送信しない)
// MH810102(S) 電子マネー対応 #5329 精算機の電源をOFF/ONした時、E2615-265が発生する
extern uchar		WAKEUP_COUNT;								// 起動タイマカウンタ
extern uchar		WAKEUP_OVER;								// 起動済みフラグ
// MH810102(E) 電子マネー対応 #5329 精算機の電源をOFF/ONした時、E2615-265が発生する

/*[]----------------------------------------------------------------------[]*/
/*| function prototype                                                     |*/
/*[]----------------------------------------------------------------------[]*/

/* idletask.c */
extern	void	idletask( void );
extern	void	tim500_mon( void );
extern	void	Tcb_Init( void );
extern	void	Mcb_Init( void );
extern	MsgBuf	*GetBuf( void );
extern	void	PutMsg( unsigned char, MsgBuf* );
extern	MsgBuf	*GetMsg( unsigned char );
extern	void	FreeBuf( MsgBuf* );
extern	void	GetMsgWait( unsigned char , MsgBuf * );
MsgBuf	*Target_MsgGet( uchar id, t_TARGET_MSGGET_PRM *pReq );
// MH321800(S) Y.Tanizaki ICクレジット対応(共通改善No.1362/精算中止と同時に電子マネータッチで、カード引去りするが精算中止してしまう不具合対策)
extern const MsgBuf *Target_MsgGet_delete1( const unsigned char task_id, const unsigned short message_id );
// MH321800(E) Y.Tanizaki ICクレジット対応(共通改善No.1362/精算中止と同時に電子マネータッチで、カード引去りするが精算中止してしまう不具合対策)
extern	void	xPause( unsigned long );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
extern	void	xPause1ms( unsigned long );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
extern	void	xPause2ms( unsigned long );
extern	void	xPause_PRNTSK( unsigned long );
extern	MsgBuf	*Target_MsgGet_Range( uchar, t_TARGET_MSGGET_PRM * );

/* taskchg.src */
extern	void	taskchg( unsigned short );

/* comctl.src */
extern	void	_di( void );
extern	void	_ei( void );
extern	unsigned char _di2( void );
extern	void	_ei2( unsigned long );
extern	void	wait2us( unsigned long );
extern	void	wait1us( void );

/* comfnc.c */
extern	short	queset( unsigned char, unsigned short, char, void * );
extern	void	inc_dct( short, short );
extern	unsigned char	bcdbin( unsigned char );
extern	short	bcdbin2( unsigned char * );
extern	short	bcdbin3( unsigned char * );
extern	unsigned char	binbcd( unsigned char );
extern	unsigned char	bcccal	( char *, short );
extern	unsigned char	sumcal( char *, short );
extern	void	cnvdec2	( char *, short );
extern	void	cnvdec4	( char *, long );
extern	unsigned short	astoin	( unsigned char *, short );
extern	unsigned short	astotm	( unsigned char * );
extern	unsigned long	astoinl ( unsigned char *, short );
extern	unsigned char	astohx	( unsigned char * );
extern	unsigned short	astohx2 ( unsigned char * );
extern	void	intoas	( unsigned char *, unsigned short, unsigned short );
extern	void	intoasl ( unsigned char *, unsigned long, unsigned short );
extern	unsigned char	intoasl_0sup( unsigned char *buf, unsigned long data, unsigned short size );
extern	char	hexcnv1 ( char );
extern	void	hxtoasc ( unsigned char *, unsigned char );
extern	void	hxtoas	( unsigned char *, unsigned char );
extern	void	hxtoas2	( unsigned char *, unsigned short );
extern	ushort	binlen( ulong );
extern	void	tmtoas	( unsigned char *, short );
extern	void	nmisave	( void *, void *, short );
extern	void	nmicler	( void *, short );
extern	void	nmimset	( void *, unsigned char, short );
extern	void	nmitrap	( void );
extern	unsigned short	dnrmlzm ( short, short, short );
extern	void	idnrmlzm( unsigned short, short *, short *, short * );
extern	unsigned long	enc_nmlz_mdhm (unsigned short, unsigned char, unsigned char, unsigned char, unsigned char);
extern	void	dec_nmlz_mdhm (unsigned long, unsigned short*, unsigned char*, unsigned char*, unsigned char*, unsigned char*);
extern	short	medget ( short, short );
extern	short	tnrmlz ( short, short, short, short );
extern	void	itnrmlz( short, short, short, short *, short * );
extern	unsigned char	majorty( unsigned char *, unsigned char );
extern	unsigned char	majomajo( unsigned char *, unsigned char, unsigned char );
extern	void	ClkrecUpdate( unsigned char * );
extern	short	chkdate( short, short, short );
extern	short	youbiget( short, short, short );
extern	void	crc_ccitt( ushort, uchar *, uchar *, uchar );				/* CRC16算出:Lock,NT-NET,Mifare	*/
// GG120600(S) // Phase9 CRCチェック
extern	void	crc_ccitt_update( ushort, uchar *, uchar *, uchar );				/* CRC16算出:Lock,NT-NET,Mifare	*/
// GG120600(E) // Phase9 CRCチェック
extern	uchar	HexAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin );
extern	uchar	DeciAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin );
extern	unsigned short	sumcal_ushort( unsigned short *stad, unsigned short nu );
extern	unsigned char	memcmp_w( const unsigned short *pSrc1, const unsigned short *pSrc2, unsigned long Length );
extern	uchar	DecAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin );
extern	ulong	get_bmplen(uchar *bmp);
extern	uchar	DecAsc_to_LongBin_Minus( uchar *pAsc, uchar Length, long *pBin );
extern	unsigned long	c_arraytoint32(unsigned char array[], short size);
extern	void	c_int32toarray(unsigned char array[], unsigned long data, short size);
extern	uchar	CheckCRC(uchar *pFrom, ushort usSize, uchar *pHere);
extern	unsigned long	astohex ( char *, unsigned int, unsigned char );
extern	void	hextoas ( unsigned char *, unsigned long, unsigned int, unsigned long );
extern	uchar	SJIS_Size( uchar *data );																		// 漢字判定処理
extern	void	c_Now_CLK_REC_ms_Read( struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec );
extern	uchar	c_ClkCheck( struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec );
extern	void	c_Normalize_ms( struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec, ushort *pNormDay, ulong *pNormTime );
extern	void	c_UnNormalize_ms( ushort *pNormDay, ulong *pNormTime, struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec );
extern	ulong	c_Normalize_sec( struct clk_rec *pCLK_REC );
extern	void	c_UnNormalize_sec( ulong NormSec, struct clk_rec *pCLK_REC );
extern	void	memcpyFlushLeft(uchar *dist, uchar *src, ushort dist_len, ushort src_len);
extern	void	memcpyFlushRight(uchar *dest, uchar *src, ushort dest_len, ushort src_len);
// MH322915(S) K.Onodera 2017/05/18 遠隔ダウンロード修正
extern	uchar	Check_date_time_rec( date_time_rec *pTime );
// MH322915(E) K.Onodera 2017/05/18 遠隔ダウンロード修正
// MH321800(S) D.Inaba ICクレジット対応
extern	void	memcpyFlushLeft2(uchar *dist, uchar *src, ushort dist_len, ushort src_len);
extern	void	change_CharInArray(uchar *src, ushort src_len, ushort start, ushort end, uchar before, uchar after);
// MH321800(E) D.Inaba ICクレジット対応
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
extern	int		datetimecheck(struct clk_rec *clk_data);
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

/* cpu_init.c */
extern	void	cpu_init( void );
extern	void	port_init( void );
extern	void	port_nmi( void );
extern	void	dog_init( void );
extern	void	dog_reset( void );
extern	void	dog_stop( void );
extern	void	scpu_reset( void );
extern	void	System_reset( void );
extern	void	TPU0_init( void );
extern	void	TPU1_init( void );
extern	void	TPU2_init( void );
extern	void	TPU3_init( void );
extern	void CAN1_init( void );
extern	void I2C1_init( void );
extern	void Sci2Init( void );
extern	void Sci3Init( void );
extern	void Sci4Init( void );
extern	void Sci6Init( void );
extern	void Sci7Init( void );
extern	void Sci9Init( void );
extern	void Sci10Init( void );
extern	void Sci11Init( void );
extern	void CMTU0_CMT0_init( void );
#define		WACDOG			dog_reset()	/* wacdog counter reset			*/

extern	int		ExSRam_clear( void );
/* hard_init.c */


/* Int_RAM.c */


/* Lagtim.c */
extern	void	Lagtim( unsigned char ucId , unsigned char ucNo , unsigned short usTime );
extern	void	Lagcan( unsigned char ucId , unsigned char ucNo );
extern	void	LagChk( void );
extern	unsigned char	LagChkExe( unsigned char ucId , unsigned char ucNo );
extern	void	Lag10msInit( void );
extern	void	Lag20msInit( void );
extern	void	Lag500msInit( void );
extern	void	LagTim10ms( char no, short time, void ( *func )( void ) );
extern	void	LagTim20ms( char no, short time, void ( *func )( void ) );
extern	void	LagTim500ms( char no, short time, void ( *func )( void ) );
extern	void	LagCan10ms( char no );
extern	void	LagCan20ms( char no );
extern	void	LagCan500ms( char no );
extern	void	LagChk10ms( void );
extern	void	LagChk20ms( void );
extern	void	LagChk500ms( void );
// MH810104 GG119201(S) 電子ジャーナル対応
extern	uchar	LagTim20ms_Is_Counting( char no );
extern	uchar	LagTim500ms_Is_Counting( char no );
// MH810104 GG119201(E) 電子ジャーナル対応
extern	unsigned long	LifeTimGet( void );
extern	unsigned long	LifePastTimGet( unsigned long StartTime );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
extern	unsigned long	LifeTim1msGet( void );
extern	unsigned long	LifePastTim1msGet( unsigned long StartTime );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
extern	unsigned long	LifeTim2msGet( void );
extern	unsigned char	LifePastTim2msGet( unsigned long, unsigned long );
extern	unsigned long c_2msPastTimeGet( unsigned long StartTime );
extern	unsigned long c_PastTime_Subtraction( unsigned long Time, unsigned long SubValue );

/* keyctrl.c */
extern	void	(* const KEY_SCAN_Exec[])( void );
extern	void	key_read( void );
extern	void	door_int_read( void );
extern	unsigned short	ex_port_red( void );
extern	void	SCAN_INP1_Exec( void );
extern	void	SCAN_INP2_Exec( void );
extern	void	BUZINT( void );
extern	void	BUZERR( void );
extern	void	BUZPI( void );
extern	void	BUZPIPI( void );
extern	void	BUZPIPIPI( void );
extern	void	PION( void );
extern	void	Ptout_Init( void );
extern	void	PortAlm_Init( void );

extern	void	BCDtoASCII( uchar *bcd, uchar *ascii, ushort len );

extern	uchar	RXF_VERSION[10];
extern	ulong	SodiacSoundAddress;
extern	uchar	SOUND_VERSION[10];
#define		OPE_TIM1MS_MAX		4	/* ｵﾍﾟﾚｰｼｮﾝﾀｽｸ用ﾀｲﾏｰ(Base = 1ms)		*/
#define		REMOTEDL_TIM1MS_MAX	4	/* 遠隔ﾀﾞｳﾝﾛｰﾄﾞﾀｽｸ用ﾀｲﾏｰ(Base = 1ms)	*/
extern	void	Lagtim_1ms( unsigned char ucId , unsigned char ucNo , unsigned short usTime );
extern	void	Lagcan_1ms( unsigned char ucId , unsigned char ucNo );
extern	void	LagChk_1ms( void );
extern	unsigned short OPE_Tim_Base1ms[OPE_TIM1MS_MAX];				/* ｵﾍﾟﾚｰｼｮﾝﾀｽｸ用ﾀｲﾏｰ(Base = 1ms)		*/
extern	unsigned short REMOTEDL_Tim_Base1ms[REMOTEDL_TIM1MS_MAX];	/* 遠隔ﾀﾞｳﾝﾛｰﾄﾞﾀｽｸ用ﾀｲﾏｰ(Base = 1ms)	*/

/* nonslib.c */
extern	int stricmp(const char *d, const char *s);
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
/* longpark.c */
extern	void LongTermParkingCheck( void );
extern	void LongTermParkingCheck_r10( void );
extern	void LongTermParkingCheck_Resend( void );
extern	void LongTermParkingCheck_Resend_flagset( void );
extern	void LongTermParkingCheck_r10_prmcng( void );
extern	void LongTermParkingCheck_r10_defset( void );
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)


#endif	// _SYSTEM_H_
