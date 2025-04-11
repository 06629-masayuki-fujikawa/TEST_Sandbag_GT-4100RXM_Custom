/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		IF board communications heder file																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	lkcom.h																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      :																							   |*/
/*| Date        :	2005-03-26																				   |*/
/*| Update      :																							   |*/
/*|------------------------------------------------------------------------------------------------------------|*/
/*| ・IF盤に対する通信関連の変数、ﾃｰﾌﾞﾙ定義、ﾌﾟﾛﾄﾀｲﾌﾟ宣言													   |*/
/*|	・system.hの後に宣言すること																			   |*/
/*[]--------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.-------[]*/
#ifndef _LK_COM_H_
#define _LK_COM_H_

/*==============================================================================================================*/
/*				ﾃﾞﾌｧｲﾝ定義																						*/
/*==============================================================================================================*/
#define		LK_PKT_MAX				867	



/*--------------*/
/*	送信ﾃﾞｰﾀID	*/
/*--------------*/
#define		LK_SND_INIT_ID			0x23										/* 初期設定ﾃﾞｰﾀ					*/
#define		LK_SND_CTRL_ID			0x21										/* ﾛｯｸ装置制御ﾃﾞｰﾀ				*/
// ARCﾊﾟｹｯﾄ送信要求
#define		ARC_SND_RRES_ID			0x52										/* 受信ﾃﾞｰﾀ応答					*/
#define		ARC_SND_TEST_ID			0x54										/* ﾃｽﾄｺﾏﾝﾄﾞ						*/

/*--------------*/
/*	受信ﾃﾞｰﾀID	*/
/*--------------*/
#define		LK_RCV_REQU_ID			0x22										/* 要求ﾃﾞｰﾀID					*/
#define		LK_RCV_COND_ID			0x61										/* 状態ﾃﾞｰﾀID					*/
#define		LK_RCV_TEST_ID			0x62										/* ﾃｽﾄﾃﾞｰﾀID					*/
#define		LK_RCV_MENT_ID			0x63										/* ﾒﾝﾃﾅﾝｽﾃﾞｰﾀID					*/
#define		LK_RCV_VERS_ID			0x64										/* ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀID				*/
#define		LK_RCV_ERR_ID			0x65										/* ｴﾗｰ状態ﾃﾞｰﾀID				*/
#define		LK_RCV_ECOD_ID			0x66										/* ｴﾗｰﾃﾞｰﾀID					*/
#define		IBC_RCV_COND_ID			0x67
#define		IBC_RCV_REQU_ID			0x68
#define		CRR_RCV_TEST_ID			0x69										/* CRR折り返しテスト完了		*/
#define		IBC_RCV_MENT_ID			0x70										/* ﾒﾝﾃﾅﾝｽﾃﾞｰﾀID					*/
// IBC固有のもの
// 以下、ARCﾊﾟｹｯﾄ特有のもの
#define		IBC_RCV_VERS_ID			0x84										/* ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ					*/
#define		IBC_RCV_RSLT_ID			0x80										/* 送信結果ID					*/
#define		IBC_RCV_STAT_ID			0x81										/* 回線状態ID					*/
#define		IBC_RCV_FINU_ID			0x82										/* 初期設定完了ID				*/
#define		IBC_RCV_TEND_ID			0x83										/* ﾃｽﾄ結果ID					*/

#define		LK_RCV_ZAN				960											/* 受信ﾊﾞｯﾌｧ空きｻｲｽﾞ			*/
#define		LK_LOCK_MAX				50											/* 1親機ﾛｯｸ装置MAX接続台数		*/
																				/*								*/
#define		LK_CTRL_DT_SIZE			7											/* ﾛｯｸ装置制御ﾃﾞｰﾀｻｲｽﾞ			*/
#define		LK_INIT_DT_SIZE			206											/* 初期設定ﾃﾞｰﾀｻｲｽﾞ				*/

#define	TERM_NO_MAX				FLAP_IF_MAX	// 最大ﾀｰﾐﾅﾙNo

// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
#define MAX_AUTO_PAYMENT_NO		INT_CAR_LOCK_MAX	//(自動登録する駐輪は20車室なのでとりあえず20とする) 
#define MAX_AUTO_PAYMENT_CAR	INT_CAR_LOCK_MAX-1	// ﾌﾗｯﾌﾟ 0～19
#define MAX_AUTO_PAYMENT_BIKE	BIKE_LOCK_MAX-1		// ﾛｯｸ   20～69
enum {
	AUTOPAY_STS_IN = 1,
	AUTOPAY_STS_OUT,
	AUTOPAY_STS_UP,
	AUTOPAY_STS_DOWN,
};

#endif

#define LOCK_CTRL_BUFSIZE_MAX 74												// 9*6 + 20 CRB9台+フラップ20台
#define	FLAP_NUM_MAX			BIKE_START_INDEX	// 最大ﾌﾗｯﾌﾟNo(100) indexは-1
#define	CRR_CTRL_START			INT_CAR_START_INDEX	// 最大IFﾌﾗｯﾌﾟNo(50) indexは-1
#define	LOCK_REQ_ALL_TNO	0xFF					// ロック装置全てに対する要求の結果を表すターミナルNo(CRB I/F→CRA I/F変換用)
#define	FLAP_REQ_ALL_TNO	0xFE					// フラップ装置全てに対する要求の結果を表すターミナルNo(CRB I/F→CRA I/F変換用)
#define	FLAP_LOCK_TNO_MAX	31						// フラップロック装置合わせたターミナルNoの最大値

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

#define		LK_SCI_SNDBUF_SIZE		(1024)										/*								*/
#define		LK_SCI_RCVBUF_SIZE		(1024)										/*								*/


/*----------------------------------*/
/* CRC-CCITT :  x^{16}+x^{12}+x^5+1 */
/*----------------------------------*/
#define		LK_CRCPOLY1				0x1021										/* 左シフト						*/
#define		LK_CRCPOLY2				0x8408  									/* 右シフト						*/
#define		LK_CHAR_BIT				8											/* number of bits in a char		*/
#define		LK_L_SHIFT				0											/* 左シフト						*/
#define		LK_R_SHIFT				1											/* 右シフト						*/

#define		LKCOM_TYPE_NONE			(uchar)0				// 装置未定義
#define		LKCOM_TYPE_LOCK			(uchar)1				// ロック装置（駐輪）
#define		LKCOM_TYPE_FLAP			(uchar)2				// フラップ装置（駐車）


/*==========================================================*/
/*						構造体定義							*/
/*==========================================================*/

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/

/*---------------------*/
/***  受信ﾃﾞｰﾀﾃｰﾌﾞﾙ  ***/
/*---------------------*/

/***** ﾛｯｸ装置通常状態ﾃﾞｰﾀ *****/
/*------------------*/
/* このﾃﾞｰﾀは可変長 */
/*------------------*/
typedef struct {																/*								*/
	uchar			lock_no;													/* ﾛｯｸ装置No.					*/
	uchar			car_cnd;													/* 車両検知状態					*/
	uchar			lock_cnd;													/* ﾛｯｸ装置状態					*/
} t_LKcomSubLock;																/*								*/
																				/*								*/
typedef struct {																/*								*/
	uchar			did[4];														/* ﾃﾞｰﾀID						*/
	uchar			kflg;														/* 保持ﾌﾗｸﾞ						*/
	uchar			sno;														/* ﾃﾞｰﾀ追番						*/
	uchar			dcnt;														/* ﾃﾞｰﾀ数						*/
} t_LKcomLock;																	/*								*/
																				/*								*/
																				/*								*/
/*--- ﾛｯｸ装置開閉ﾃｽﾄ要求応答 ---*/
typedef struct {																/*								*/
	uchar			did[4];														/* ﾃﾞｰﾀID						*/
	uchar			kflg;														/* 保持ﾌﾗｸﾞ						*/
	uchar			sno;														/* ﾃﾞｰﾀ追番						*/
	uchar			lock_tst[90];												/* ﾛｯｸ装置毎ﾃｽﾄ結果				*/
} t_LKcomTest;																	/*								*/
																				/*								*/
																				/*								*/
/***** ﾒﾝﾃﾅﾝｽ情報要求応答 *****/
typedef struct {																/*								*/
	uchar			auto_cnt[4];												/* ﾛｯｸ装置動作ｶｳﾝﾄ				*/
	uchar			manu_cnt[4];												/* ﾛｯｸ装置手動動作ｶｳﾝﾄ			*/
	uchar			trbl_cnt[4];												/* ﾛｯｸ装置故障回数				*/
} t_LKcomSubMnt;																/*								*/
																				/*								*/
typedef struct {																/*								*/
	t_LKcomSubMnt	lock_mnt[72];												/* ﾛｯｸ装置毎動作ｶｳﾝﾄ			*/
} t_LKcomMnt;																	/*								*/

/***** ﾒﾝﾃﾅﾝｽ情報要求応答 *****/
typedef struct {																/*								*/
	uchar			auto_cnt[6];												/* ﾌﾗｯﾌﾟ装置動作ｶｳﾝﾄ			*/
	uchar			manu_cnt[4];												/* ﾌﾗｯﾌﾟ装置手動動作ｶｳﾝﾄ		*/
	uchar			trbl_cnt[4];												/* ﾌﾗｯﾌﾟ装置故障回数			*/
} t_FLcomSubMnt;																/*								*/
																				/*								*/
typedef struct {																/*								*/
	uchar			did[4];														/* ﾃﾞｰﾀID						*/
	uchar			kflg;														/* 保持ﾌﾗｸﾞ						*/
	uchar			sno;														/* ﾃﾞｰﾀ追番						*/
	t_FLcomSubMnt	flap_mnt[72];												/* ﾛｯｸ装置毎動作ｶｳﾝﾄ			*/
} t_FLcomMnt;																	/*								*/
																				/*								*/
/***** ﾊﾞｰｼﾞｮﾝ要求応答 *****/
typedef struct {																/*								*/
	uchar			s_ver[15][8];												/* IF盤子機ﾊﾞｰｼﾞｮﾝ				*/
} t_LKcomVer;																	/*								*/
																				/*								*/
/***** ｴﾗｰ要求応答 *****/
typedef struct {																/*								*/
	uchar			did[4];														/* ﾃﾞｰﾀID						*/
	uchar			kflg;														/* 保持ﾌﾗｸﾞ						*/
	uchar			sno;														/*								*/
	ushort			m_err;														/* IF盤親機ｴﾗｰ					*/
	uchar			s_err[15];													/* IF盤子機ｴﾗｰ					*/
} t_LKcomErr;																	/*								*/

/***** IF盤要求ﾃﾞｰﾀ	*****/
typedef struct {																/*								*/
	uchar			did[4];														/* ﾃﾞｰﾀID						*/
	uchar			kflg;														/* 保持ﾌﾗｸﾞ						*/
	uchar			sno;														/* ﾃﾞｰﾀ追番						*/
	uchar			req;														/* 要求							*/
} t_LKcomReq;																	/*								*/
																				/*								*/
/***** ｴﾗｰﾃﾞｰﾀ	*****/
typedef struct {																/*								*/
	uchar			did;														/* ﾃﾞｰﾀID						*/
	uchar			kflg;														/* 保持ﾌﾗｸﾞ						*/
	uchar			sno;														/* ﾃﾞｰﾀ追番						*/
	uchar			tno;														/* 端末情報(0=親機/1～15:子機)	*/
	uchar			err;														/* ｴﾗｰNo.						*/
	uchar			occr;														/* 解除/発生/発生解除同時		*/
	uchar			dmy[2];														/* 予備							*/
} t_LKcomEcod;																	/*								*/
																				/*								*/
/***** 受信ﾃﾞｰﾀﾍｯﾀﾞ	*****/
typedef struct {
	uchar			did[4];														/* ﾃﾞｰﾀID						*/
	uchar			kflg;														/* 保持ﾌﾗｸﾞ						*/
	uchar			sno;														/* ﾃﾞｰﾀ追番						*/
} T_LKcomRcvHdrIbc;

/***** 送信結果ﾃﾞｰﾀ	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ﾍｯﾀﾞ							*/
	uchar				ssno;													/* 送信ﾃﾞｰﾀ追番					*/
	uchar				tno;													/* 端末情報(0=親機/1～15:子機)	*/
	uchar				rslt;													/* 結果							*/
	uchar				dmy;													/* 予備							*/
} t_LKcomRsltIbc;																/*								*/

/***** 回線状態ﾃﾞｰﾀ	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ﾍｯﾀﾞ							*/
	uchar				tsts[20];												/* 端末状態(1～20)				*/
} t_LKcomStatIbc;																/*								*/

/***** ﾃｽﾄ結果ﾃﾞｰﾀ	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ﾍｯﾀﾞ							*/
	uchar				rslt;													/* 結果							*/
	uchar				dmy;													/* 予備							*/
} t_LKcomTEndIbc;																/*								*/

/***** 初期設定完了ﾃﾞｰﾀ	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ﾍｯﾀﾞ							*/
	uchar				dmy[2];													/* 予備							*/
} t_LKcomFinuIbc;																/*								*/

/***** ﾊﾞｰｼﾞｮﾝﾃﾞｰﾀ	*****/
typedef struct {																/*								*/
	T_LKcomRcvHdrIbc	hdr;													/* ﾍｯﾀﾞ							*/
	uchar				ver[10];												/* ﾌﾟﾛｸﾞﾗﾑﾊﾞｰｼﾞｮﾝ				*/
	uchar				psum[2];												/* ﾌﾟﾛｸﾞﾗﾑﾁｪｯｸｻﾑ				*/
	uchar				mchk[4];												/* ﾒﾓﾘR/Wﾁｪｯｸ結果				*/
	uchar				dmy[2];													/* 予備							*/
} t_LKcomVerIbc;																/*								*/

/*------------------------*/
/* 端末統一で管理する変数 */
/*------------------------*/
typedef	struct {																/*								*/
																				/*								*/
	uchar	PktSeqNo;															/* ﾊﾟｹｯﾄｼｰｹﾝｼｬﾙNo.				*/
	uchar	TnoNow;																/* 現在のｱｸｾｽ端末番号			*/
	uchar	TnoMax;																/* 端末接続台数					*/
	uchar	MentFlg;															/* ﾒﾝﾃﾅﾝｽ中ﾌﾗｸﾞ					*/
} t_LKcomLineCtrl;																/*								*/


/*----------------------*/
/* 端末毎に管理する変数 */
/*----------------------*/

/***** 送信ﾃﾞｰﾀｷｭｰ管理ﾃｰﾌﾞﾙ *****/
typedef struct {																/*								*/
	ushort	R_Index;															/* ﾘｰﾄﾞﾎﾟｲﾝﾀ					*/
	ushort	W_Index;															/* ﾗｲﾄﾎﾟｲﾝﾀ						*/
	ushort	Count;																/* 件数							*/
} t_LKcomQueCtrl;																/*								*/


/***** 初期設定ﾃﾞｰﾀ *****/
typedef struct {																/*								*/
	uchar	did4;																/* ﾃﾞｰﾀID4						*/
	uchar	kflg;																/* 保持ﾌﾗｸﾞ						*/
	uchar	sno;																/* ｼｰｹﾝｼｬﾙNo.					*/
	uchar	lock[90];															/* ﾛｯｸ装置接続					*/
	uchar	i_tm[15];															/* 入庫車両検知ﾀｲﾏｰ				*/
	uchar	o_tm[15];															/* 出庫車両検知ﾀｲﾏｰ				*/
	uchar	r_cn[15];															/* ﾘﾄﾗｲ回数						*/
	ushort	r_tm[15];															/* ﾘﾄﾗｲ間隔						*/
	ushort	mst_tm1;															/* 受信ﾃﾞｰﾀ最大時間				*/
	ushort	mst_tm2;															/* ﾃｷｽﾄ送信後の応答待ち時間		*/
	ushort	mst_tm3;															/* 送信ﾃﾞｰﾀWaitﾀｲﾏｰ				*/
	uchar	mst_ret1;															/* ﾃﾞｰﾀ送信ﾘﾄﾗｲ回数				*/
	uchar	mst_ret2;															/* NAK送信ﾘﾄﾗｲ回数				*/
	ushort	dummy1;																/* 予備							*/
	ushort	slv_tm1;															/* 受信監視ﾀｲﾏｰ					*/
	ushort	slv_tm2;															/* POL間隔						*/
	uchar	slv_ret1;															/* ﾘﾄﾗｲ回数						*/
	uchar	slv_ret2;															/* 無応答ｴﾗｰ判定回数			*/
	ushort	dummy2;																/* 								*/
	uchar	oc_tm[5][2];														/* open/close時間				*/
	uchar	dummy3[10];															/*								*/
} t_LKcomInit;																	/*								*/

/***** ﾛｯｸ装置制御ﾃﾞｰﾀ *****/
typedef struct {																/*								*/
	uchar	did4;																/* ﾃﾞｰﾀID4						*/
	uchar	tno;																// CRBインターフェースのターミナルNo
	uchar	kind;																/* 処理区分						*/
	uchar	lock;																/* ﾛｯｸ装置No.					*/
	uchar	clos_tm;															/* 閉動作信号出力時間			*/
	uchar	open_tm;															/* 開動作信号出力時間			*/
} t_LKcomCtrl;																	/*								*/


/***** 回線管理 *****/
typedef struct {																/*								*/
	/*** 回線管理 ***/
																				/*								*/
	/*** ﾛｯｸ装置制御 ***/
	t_LKcomQueCtrl	CtrlInfo;													/* ｷｭｰﾊﾞｯﾌｧｺﾝﾄﾛｰﾙ				*/
	t_LKcomCtrl		CtrlData[LOCK_CTRL_BUFSIZE_MAX];							// ﾛｯｸ装置制御ﾃﾞｰﾀ格納ﾊﾞｯﾌｧ:74件 9*6+20= CRB9台+フラップ20台
																				/*								*/
	// 最終的には1件にする
																				/*								*/

} t_LKcomTerm;																	/* 								*/
																				/*								*/
extern	t_LKcomTerm		LKcomTerm;
/***** 装置種別 *****/
extern	uchar			LKcom_Type[LOCK_IF_MAX];								// 端末毎の装置種別（駐車・駐輪）
																			/*								*/
/*-----------------------------------*/
/* 受信したﾃﾞｰﾀをMAINへ渡す為のﾊﾞｯﾌｧ */
/*-----------------------------------*/
#define		LK_RCV_BUF_SIZE		20000											/* 受信ﾃﾞｰﾀ格納ﾊﾞｯﾌｧｻｲｽﾞ		*/
																				/*								*/
typedef	struct {																/*								*/
	uchar		dcnt;															/* ﾃﾞｰﾀ格納件数					*/
	ushort		all_siz;														/* 総ﾃﾞｰﾀ格納ﾊﾞｲﾄ数				*/
	ushort		zan_siz;														/* 空きｴﾘｱﾊﾞｲﾄ数				*/
	uchar		data[LK_RCV_BUF_SIZE];											/* ﾃﾞｰﾀ格納ﾊﾞｯﾌｧ				*/
																				/*								*/
} t_LK_RCV_INFO;																/*								*/
																				/*								*/
extern	t_LK_RCV_INFO		LKcom_RcvData;										/*								*/
																				/*								*/
extern	uchar		LKcom_InitFlg;												/* ﾛｯｸ装置通信初期化ﾌﾗｸﾞ		*/
extern	uchar		LKcom_f_TaskStop;											/* ﾛｯｸ装置通信ﾀｽｸ停止ﾌﾗｸﾞ		*/
																				/*								*/
extern	uchar		LKcom_RcvDtWork[LK_SCI_RCVBUF_SIZE];						/* 受信ﾃﾞｰﾀ解析用ﾜｰｸ			*/
																				/*								*/
																				/*								*/
extern	uchar		LK_Init;													/*								*/
extern	uchar		LKcom_f_SndReq;												/* POL/SEL送信ﾘｸｴｽﾄ				*/
extern				t_LKcomLineCtrl		LKcomLineCtrl;							/* 回線ｺﾝﾄﾛｰﾙ					*/
																				/*								*/
extern	uchar		LKcomdr_RcvData[LK_SCI_RCVBUF_SIZE];						/* 受信ﾃﾞｰﾀﾊﾞｯﾌｧ(ﾜｰｸ)			*/
extern	uchar		LK_SndBuf[LK_SCI_SNDBUF_SIZE];								/* 送信ﾊﾞｯﾌｧ					*/
																				/*								*/
extern	uchar		MntLockTest[LK_LOCK_MAX];									/*								*/
extern	ulong		MntLockDoCount[BIKE_LOCK_MAX][3];							/* LOCK装置動作ｶｳﾝﾄ				*/
extern	ulong		MntFlapDoCount[TOTAL_CAR_LOCK_MAX][3];						/* ﾌﾗｯﾌﾟ(内蔵＋外付け)装置動作ｶｳﾝﾄ	*/
extern	ushort		MntLockWkTbl[LOCK_IF_REN_MAX];								/* ﾛｯｸ装置動作ｶｳﾝﾄ受信用変換ﾃｰﾌﾞﾙ	*/
extern	uchar		MntFlapTest[CAR_LOCK_MAX];
																				/*								*/
typedef struct {																/*								*/
	ushort			LockNoBuf;													/* 車室№(1～324)-1				*/
	uchar			car_cnd;													/* 車両検知状態					*/
	uchar			lock_cnd;													/* ﾛｯｸ装置状態					*/
	uchar			dummy[2];													/* 予備1						*/
} t_LKBUF;																		/*								*/
extern	t_LKBUF		LockBuf[LK_LOCK_MAX];										/*								*/
extern	uchar		child_mk[LOCK_IF_MAX];												// 子機別ﾛｯｸ装置ﾒｰｶｰ編集
																				/*								*/
extern	uchar	flp_m_mode[FLAP_IF_MAX];										/* ﾌﾗｯﾌﾟ装置手動ﾓｰﾄﾞﾌﾗｸﾞ		*/
																				/*  ON = 手動ﾓｰﾄﾞ中				*/
extern	uchar	lok_m_mode[LOCK_IF_MAX];										/* ﾛｯｸ装置手動ﾓｰﾄﾞﾌﾗｸﾞ			*/
																				/*  ON = 手動ﾓｰﾄﾞ中				*/
																				/*								*/
																				/*								*/
/*==============================================================================================================*/
/*				ﾌﾟﾛﾄﾀｲﾌﾟ宣言																					*/
/*==============================================================================================================*/
																				/*								*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	LKcom.c																										*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	void		LKcom_Init( void );											/*								*/
extern	ushort		LKcom_SndDtDec( void );										/*								*/
extern	uchar		LKcom_RcvDataSave(uchar *data, ushort len );
extern	uchar		LKcom_RcvDataDel( void );									/*								*/
extern	void		LKcom_PassBreak( void );									/*								*/
extern	void		LKcom_RcvBuffReset( void );									/* 受信ﾃﾞｰﾀ格納ﾊﾞｯﾌｧﾘｾｯﾄ		*/
																				/*								*/
																				/*								*/
																				/*								*/
/*--------------------------------------------------------------------------------------------------------------*/
/*	LKcomApi.c																									*/
/*--------------------------------------------------------------------------------------------------------------*/
extern	char		LKcom_SetDtPkt( uchar, uchar, ushort, uchar );				/*								*/
extern	uchar		LKcom_RcvDataGet( ushort * );								/*								*/
extern	uchar		LKcom_RcvDataAnalys( ushort );								/*								*/
extern	void		LKcom_SetLockMaker( void );									/*								*/
extern	short		LKopeGetLockNum( uchar, ushort, ushort * );					/*								*/
extern	void		LKopeApiLKCtrl( ushort, uchar );							/* ﾛｯｸ装置指示					*/
extern	void		LKopeApiLKCtrl_All(uchar, uchar);							/*								*/

extern	void		LKopeErrRegist( uchar, t_LKcomEcod* );						/*								*/
extern	short		LKopeLockErrCheck( short, uchar );							/*								*/
extern	void		ErrBinDatSet( ulong, short );								/*								*/
extern	void		lk_err_chk( ushort, char, char );							/*								*/
extern	void	LKope_ClearAllTermVersion(void);
extern	char	LKcom_Search_Ifno( uchar );
extern	void	LKcom_AllInfoReq( uchar );
extern	uchar	get_lktype(uchar lok_syu);
extern	void	LKcom_InitAccessTarminalType(void);

// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
extern	void LK_AutoPayment_Rcv(ushort index, uchar tno, uchar lkno, uchar kind);
#endif
extern 	ushort	LKcom_RoomNoToType( ulong roomNo );
extern	ushort	LKcom_GetAccessTarminalCount(void);
extern 	uchar	LKcom_GetAccessTarminalType(ushort tno);
// MH322914(S) K.Onodera 2016/09/07 AI-V対応：エラーアラーム
extern	int		LKcom_RoomNoToIndex( ulong roomNo );
// MH322914(E) K.Onodera 2016/09/07 AI-V対応：エラーアラーム

///*--------------------------------------------------------------------------------------------------------------*/
///*	LKsci.c																										*/
///*--------------------------------------------------------------------------------------------------------------*/

#endif	// _LK_COM_H_
