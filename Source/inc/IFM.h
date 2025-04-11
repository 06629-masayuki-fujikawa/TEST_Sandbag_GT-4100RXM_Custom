/*[]----------------------------------------------------------------------[]*/
/*|		New I/F(Master) common include									   |*/
/*|			・ここには、新I/F盤（親機）内の全３タスクが参照する領域を	   |*/
/*|			  宣言します。												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _IFM_H_
#define _IFM_H_

#include	"IF.h"
#include	"IFmail.h"

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/
/** 受信バッファ管理テーブル **/
typedef struct {
    ushort  RcvCnt;						/* 受信バイト数 */
	ushort	OvfCount;					/* 受信バッファオーバーフロー発生回数 */
	ushort	ComerrStatus;				/* 通信エラー状態（ビット立て？）*/

	ushort	usBetweenChar;				/* 文字間受信監視タイマ */

	BOOL			RcvCmpFlg;			/* 受信完了したフラグ */
} t_SciRcvCtrl;

/** 送信バッファ管理テーブル **/
typedef struct {
    ushort  SndReqCnt;					/* 送信要求バイト数 */
    ushort  SndCmpCnt;					/* 送信要求バイト数 */
    ushort  ReadIndex;					/* next send data (read) index */
    ushort  SndCmpFlg;					/* 送信完了したフラグ */
										/*		1=完了 */
										/*		0=未完了 */
} t_SciSndCtrl;

/* 受信エラー情報 */
typedef struct {
	ushort	usOVRcnt;					/* オーバーランエラー発生回数 */
	ushort	usFRMcnt;					/* フレーミングエラー発生回数 */
	ushort	usPRYcnt;					/*     パリティエラー発生回数 */
} t_SciRcvError;

/* 通信エラー情報 */
typedef struct {
	ulong	ulRcv_HeaderString;			/* 受：ヘッダ文字エラー発生回数 */
	ulong	ulRcv_Size;					/* 受：データ長エラー発生回数 */
	ulong	ulRcv_CRC;					/* 受：CRC異常発生回数 */
	ulong	ulRcv_Kind;					/* 受：データ種別異常発生回数 */
	ulong	ulSnd_noEmpty;				/* 送：送信時「TDR空」では無い発生回数 */
} t_SciComError;

/* エラー状態 */
typedef struct {
	uchar	ucState;					/* エラー状態 */
										/*		0=解除 */
										/*		1=発生 */
										/*		2=発生・解除同時 */
	ulong	ulCount;					/* エラー発生回数 */
} t_IF_Error;

/* ロック装置メーカー別の信号出力時間テーブル */
typedef struct {
	ushort	usLockOnTime;		/* 閉(施錠)動作、信号出力時間（×150msec計算前で150msec単位）*/
	ushort	usLockOffTime;		/* 開(開錠)動作、信号出力時間（×150msec計算前で150msec単位）*/
} t_IF_Signal;



/****************************/
/* 親機タスク間キュー		*/
/****************************/
/*
*	本データは親機でのキューの実体を示します。
*	・メールデータフォーマット形式と共通化します。
*
*	1)ポーリング応答データ
*	・精算機からのポーリングに「データあり」で応答するためのキューです。
*	・ロック管理タスク→<キュー>→対精算機通信タスク
*			■(22H)I/F盤要求データ
*			■(61H)通常状態データ
*			■(63H)ロック装置開閉テスト応答（全ロック/指定ロック共通）
*			■(64H)メンテナンス情報応答（＝稼動情報）
*
*	2)子機送信待ちデータ
*	・子機へのロック制御は、一度に１ロック装置にしかできないので、
*	  制御要求を溜め込むためのキュー（待ちリスト）です。
*	・ロック管理タスク→<キュー>→(制御完結後)ロック管理タスク→[メール]→子機通信タスク
*			□(49H：'I')初期設定データ
*			□(57H：'W')状態書込みデータ
*			□(52H：'R')状態要求データ（バージョン要求）
*/

/****************************/
/* 親機タスク間メール全体像	*/
/****************************/
// メイン側へデータを受け渡す形
typedef union {
t_IF_Mail_Header2	sCommon;		// 共通扱い。データ種別確定前に本共用体を扱うとき使用する
									// 対精算機通信タスク→対子機通信タスク
	t_PAY_22_mail	s22;			// (22H)I/F盤要求データ
	t_PAY_61_mail	s61;			// (61H)通常送信データ
	t_PAY_62_mail	s62;			// (62H)ロック装置開閉テスト応答（全ロック/指定ロック共通）
	t_PAY_63_mail	s63;			// (63H)メンテナンス情報応答（＝稼動情報）
	t_PAY_64_mail	s64;			// (64H)バージョン要求応答
	t_PAY_66_mail	s66;			// (66H)エラーデータ通知
} t_IFM_mail;

/* キューの実体 */
typedef union {
t_IF_Mail_Header	sCommon;		/* 共通扱い。データ種別確定前に本共用体を扱うとき使用する */


//（子機へ）セレクティング待ちリスト
	t_IF_I49_mail	sI49;			/*	(49H：'I')初期設定データ */
	t_IF_W57_mail	sW57;			/*	(57H：'W')状態書込みデータ */
	t_IF_R52_mail	sR52;			/*	(52H：'R')状態要求データ（バージョン要求）*/
	t_Flap_W57_mail	sFlapW57;		// (57H：'W')状態書込みデータ
	t_Flap_I49_mail	sFlapI49;		// (49H：'I')CRR初期設定データ

//（ロック管理タスクにて）動作完結待ちリスト
									/*	(57H：'W')状態書込みデータ（同上）*/

//（ロック管理タスクへ）子機状態リスト
	t_IF_A41_mail	sA41;			/* 	(41H：'A')子機状態データ */
	t_IF_V56_mail	sV56;			/* 	(56H：'V')子機バージョンデータ */
	t_Flap_A41_mail	sFlapA41;		// (41H：'A')子機状態データ
	t_Flap_S53_mail	sFlapS53;		// (53H：'S')子機状態データ（ループデータ応答）
	t_IF_V56_mail	sFlapv76;		// (76H：'v')CRRバージョンデータ
	t_IF_t74_mail	sFlapt74;		// (74H：'t')CRR折り返しテスト結果
} t_IFM_Queue;


/* キューの本体 */
struct t_IF_Queue {
	uchar	bQueue;					/* キュー使用中フラグ */
	uchar	used;					/* 送信中フラグ 0:未送信 1:送信・受信待ち中 */
t_IFM_Queue	*pData;					/* 実データへのポインタ */
struct t_IF_Queue	*pPrev;			/* 前へのチェイン（自分型）*/
struct t_IF_Queue	*pNext;			/* 次へのチェイン（自分型）*/
};

/* キュー管理情報 */
typedef struct {
	ushort	usCount;				/* キューの個数 */
struct t_IF_Queue	*pTop;			/* キューの先頭 */
struct t_IF_Queue	*pEnd;			/* キューの末尾 */
} t_IF_Queue_ctl;

typedef struct {
	t_IF_Queue_ctl	New;		// 新規
	t_IF_Queue_ctl	Retry;		// リトライ
} t_LCMque_Ctrl_Action;			// キュー管理情報テーブル

/************************************
*	ロック装置管理テーブル（親機保持用）
*	・初期設定データを含む
*	・親機の全３タスクが使用する。
*	・対精算機通信タスクが初期設定し、
*	・ロック管理タスクが書き込み、
*	・対精算機通信タスク/対子機通信タスクが参照する。
*************************************/
typedef struct {
	uchar	ucBitRateForSlave;		/* DIP-SWより	通信BPS 0=2400/1=9600/2=19200/3=38400 (調歩同期方式)		*/

	uchar	ucOperation;			/* 処理区分（精算機からのどんな要求に起因しているか）・・・連続動作/通信用 */
	BOOL	bTest;					/* 開閉テスト動作中フラグ */

	struct {
		ushort	usLineWatch;		/*@通信受信(回線)監視タイマ			（0～9999、10msec単位	60msec）*/
                                    /*		相手からの応答受信を監視するタイマー */
		ushort	usPolling;			/*@POL/SEL間隔						（0～9999、10msec単位	0msec）	*/
                                    /*		各子機に対し、次のポーリングを送信するまでの間隔 */
		uchar	ucRetry;			/*@データ再送回数					（0～98回、99は無限回	3回）*/
                                    /*		NAK、無応答などによりテキストデータの再送が発生した場合の再送回数 */
		uchar	ucSkipNoAnswer;		/*@無応答エラー判定回数				（0～98回、99は無限回	10回）*/
                                    /* ｎ回連続無応答なら、エラー。エラー発生後はｎ回ポーリングをスキップ */
	} toSlave;

//子機MAX１５台の情報
	struct {						/* 子機はMAX１５台接続可能						子機インデックスは、０から始まる。		*/
		struct {					/* ロック装置はMAX６台接続可能（領域は８台分）	ロック装置インデックスは、０から始まる。*/
									/*		[0]ロック装置No.1													*/
									/*		[6]ロック装置No.7（予備）											*/
									/*		[7]ロック装置No.8（予備）											*/
			uchar	ucConnect;		/*@ロック装置の接続（状態）													*/
									/*		0：接続無し															*/
									/*		1：接続有り（株式会社英田エンジニアリング製ロック装置（自転車））	*/
									/*		2：接続有り（株式会社英田エンジニアリング製ロック装置（バイク））	*/
									/*		3：接続有り（株式会社吉増製作所製ロック装置（自転車、バイク兼用））	*/
									/*		4：接続有り（コムズジャパン製ロック装置（自転車））					*/
									/*		5：接続有り（HID製ロック装置（自転車、バイク兼用））				*/
//子機から来る状態
	t_IF_LockSensor	unSensor;		/* ロック装置センサー状態（バイナリ）										*/
									/*		bit0：ロック開センサー状態	0=OFF/1=ON								*/
									/*		bit1：ロック閉センサー状態	0=OFF/1=ON								*/
									/*		bit2：車両検知センサー状態	0=OFF/1=ON								*/
									/*		bit3：ロック装置動作状態	0=動作完結/1=動作中			2005.03.04 早瀬・変更 */
									/*		bit4：ロック装置動作方向	0=閉動作方向/1=開動作方向				*/
									/*		bit5：予備					0=固定									*/
									/*		bit6：予備					0=固定									*/
									/*		bit7：予備					0=固定									*/
//精算機へ返す状態
			uchar	ucCarState;		/* 車両検知状態																*/
									/*		0：接続無し															*/
									/*		1：車両有り															*/
									/*		2：車両なし															*/
									/*		3：手動モードで強制出庫												*/
			uchar	ucLockState;	/* ロック装置状態															*/
									/*		0：初期値															*/
									/*	 0xff：ロック装置動作中													*/
									/*			→これらの状態は、精算機へは通知しないので注意。				*/
									/*		1：ロック装置閉済み（正常）											*/
									/*		2：ロック装置開済み（正常）											*/
									/*		3：閉動作異常（故障）												*/
									/*		4：開動作異常（故障）												*/
									/*		5：故障解除															*/
									/*		6：指定ロック装置接続無し											*/
									/*		7：メンテナンスモード中（手動モード中）								*/
									/*			→この状態は、内部では保持しない。精算機へは通知するので注意。	*/
									/*		8：メンテナンスモード解除（自動モード中）							*/
									/*			→この状態は、内部では保持しない。精算機へは通知するので注意。	*/
//現在の制御情報
			uchar	ucOperation;	/* 処理区分（精算機からのどんな要求に起因しているか）						*/
			uchar	ucAction;		/* ロック装置動作履歴（連続動作時に使用する。単発動作では無意味？）			*/
									/*		0：何の動作要求もしていない											*/
									/*		1：施錠（閉）動作要求した											*/
									/*		2：開錠（開）動作要求した											*/
			uchar	ucActionBack;	/* ロック装置戻し方向（メンテナンス退避用）			2005.06.07	早瀬・追加	*/
									/*		注）メンテナンスモード終了で、元に戻すロック動作をするために必要	*/
									/*		0：戻さない（戻せない）												*/
									/*		1：施錠（閉）方向へ戻す												*/
									/*		2：開錠（開）方向へ戻す												*/
			uchar	ucActionMainte;	/* ロック装置メンテナンス動作履歴					2005.06.07	早瀬・追加	*/
									/*		注）メンテナンスモード終了で、元に戻すロック動作をするために必要	*/
									/*		0：メンテ中に、何の動作もしていない									*/
									/*		   または、メンテ終了時、元に戻すロック動作済み						*/
									/*		1：メンテ中に、(開/閉何れかの)動作した								*/
									/*		3：メンテ中に「強制出庫」した										*/
			ushort	usRetry;		/* ロック装置故障のための再送カウンタ										*/
//稼動情報


			BOOL	bChangeState;	/* 状態変化通知済みフラグ（初期設定データ受信でリセットされる）*/

			uchar	ucResult;		/* 開閉テスト結果 */

			ushort	usLockOnTime;	/* ロック装置閉時の信号出力時間。過去の動作時間	（1～99 150ミリ秒単位）		*/
			ushort	usLockOffTime;	/* ロック装置開時の信号出力時間。過去の動作時間	（1～99 150ミリ秒単位）		*/

		} sLock[IFS_ROOM_MAX];

		BOOL	bComm;				/* 子機へ通信する必要あるか？フラグ（全てのロック装置が「接続なし」はFALSE）*/
		BOOL	bInitialACK;		/* 子機へ初期設定データを送ってACK返信されたか？フラグ						*/
// このフラグは精算機とCRBが別電源となり精算機のみが電源再起動した時に初期設定データの変更があった場合、送信の判定をするフラグ
		BOOL	bInitialsndACK;		/* 子機へ初期設定データを送ったかフラグ										*/

		BOOL	bSomeone;			/* 子機所有のロック装置が何れかひとつでも「接続状態」にあるか否か？フラグ	*/
		BOOL	bChange;			/* 子機所有のロック装置が何れかひとつでも「状態に変化」があったか？フラグ	*/

		uchar	ucNakRetry;			/* (NAK応答されたときの)再送カウンタ										*/
		uchar	ucSkipCount;		/* (無応答のための)スキップカウンタ											*/
		BOOL	bSkip;				/* スキップ中フラグ															*/
									/*		1：スキップ中（無応答のため）										*/
									/*		0：スキップ解除														*/
		BOOL	bSkipSave;			/* 上記フラグの退避用								2005.07.26 早瀬・追加	*/
		uchar	ucWhatLastCmd;		/* 直前送信コマンド															*/
		uchar	ucWhereLastCmd;		/* 直前送信コマンドが「いま何処にあるか？」区分								*/
									/*		0：何処にもない（ポーリングのための「状態要求データ」のケース）		*/
									/*		1：優先キューにある（セレクティング最初のケース）					*/
									/*		2：順番ボードにある（セレクティング再送のケース。無応答のため）		*/
	t_IFM_Queue	*psNext;			/* 順番ボード上、ポーリング/セレクティング・コマンドへのポインタ			*/

		ushort	usEntryTimer;		/*@入庫車両検知タイマー		 (車両検知確定時間  ＝1～99 1秒単位)			*/
		ushort	usExitTimer;		/*@出庫車両検知タイマー		 (車両非検知確定時間＝1～99 1秒単位)			*/
		ushort	usRetryCount;		/*@ロック装置動作にてリトライする回数( "00"～"98"  回。"99"＝無制限。次の動作要求まで) */
		ushort	usRetryInterval;	/*@ロック装置動作にてリトライする間隔("001"～"999" 秒)						*/

		uchar	ucContinueCnt;		/* 継続カウンタ（バージョン要求/応答で使用）*/
		char	cVersion[8];		/* 子機バージョン（部番）*/

	t_IF_Error	sIFSerror[E_IFS_END];	/* 子機エラー状態 */
		t_IF_FlatLoopCounter	tLoopCount;		/* ループカウント													*/
		ushort	usFlapUpTime;			/* フラップ装置上昇時間														*/
		ushort	usFlapDownTime;			/* フラップ装置下降時間														*/
	} sSlave[IF_SLAVE_MAX];
	struct{
		BOOL	bComm;				/* 子機へ通信する必要あるか？フラグ（全てのロック装置が「接続なし」はFALSE）*/
		uchar	ucWhatLastCmd;		/* 直前送信コマンド															*/
		uchar	ucWhereLastCmd;		/* 直前送信コマンドが「いま何処にあるか？」区分								*/
		char	cVersion[8];		/* 子機バージョン（部番）*/
		t_IF_Error	sIFSerror[E_IFS_END];	/* 子機エラー状態 */
	} sSlave_CRR[IFS_CRR_MAX];		/* CRR管理用																*/
	uchar	ucConnect_Tbl[IFS_CRR_MAX][IFS_CRRFLAP_MAX];	// CRR基板毎、フラップ板No.毎、ターミナル番号指定

	ushort	usWaitCount;			/* ロック動作(完結)待ち数・・・「一つの親についてｎ個まで」並行動作可能とする */
									/*	当初は『ｎ＝１』とする。つまり１個でも動作完結していなければ、待たされる */
	uchar	ucFlapWait;				/* フラップ動作完了待ち 1:動作完了待ち										*/
	uchar	ucCommLockCommand;		/* ロック装置コマンド応答待ち中（子機送信レベル)							*/
	uchar	ucCommFlapCommand;		/* フラップコマンド応答待ち中（子機送信レベル)								*/
t_IF_Error	sIFMerror[E_IFM_END];	/* 親機エラー状態 */
} t_IFM_InitData;

/*----------------------------------*/
/*		RAM area define (external)	*/
/*----------------------------------*/
extern	t_IFM_InitData	IFM_LockTable;				/* ロック装置管理テーブル（初期設定データを含む）*/
extern	t_IF_Signal	IF_SignalOut[LOCK_MAKER_END];	/* ロック装置メーカー別の信号出力時間テーブル */
extern	t_IF_FlapSensor	IFM_FlapSensor[LOCK_IF_MAX];	/* フラップセンサ状態（ループセンサ,電磁センサ,ロック板状態,フラップセンサ）*/
extern  t_IF_LockAction IFM_LockAction;

/*----------------------------------*/
/*		function external			*/
/*----------------------------------*/

/*----------------------------------*/
/*			toScomdr.c				*/
/*----------------------------------*/
extern	void	toScomdr_RcvInit(void);		/* toS_1msInt(toStime.c) が使う */

/*----------------------------------*/
/*			toSsci.c				*/
/*----------------------------------*/
extern	t_SciRcvCtrl	toS_RcvCtrl;
extern	t_SciSndCtrl	toS_SndCtrl;
extern	unsigned char	toS_RcvBuf[TOS_SCI_RCVBUF_SIZE];
extern	unsigned char	toS_SndBuf[TOS_SCI_SNDBUF_SIZE];

/*----------------------------------*/
/*			IFM.c					*/
/*----------------------------------*/
extern	t_IFM_Queue			toSque_Body_Select[TOS_Q_WAIT_SELECT * 2];	/* キュー実体 */
extern	struct t_IF_Queue	toSque_Buff_Select[TOS_Q_WAIT_SELECT];		/* キュー本体 */
extern	t_IF_Queue_ctl		toSque_Ctrl_Select;							/* キュー管理情報 */

extern	t_IFM_Queue			toSque_Body_Status[LCM_Q_SLAVE_STATUS];	/* キュー実体 */
extern	struct t_IF_Queue	toSque_Buff_Status[LCM_Q_SLAVE_STATUS];	/* キュー本体 */
extern	t_IF_Queue_ctl		toSque_Ctrl_Status;						/* キュー管理情報 */


extern	t_IFM_Queue			LCMque_Body_Action[LCM_Q_WAIT_ACTION];	/* キュー実体 */
extern	struct t_IF_Queue	LCMque_Buff_Action[LCM_Q_WAIT_ACTION];	/* キュー本体 */
extern	t_LCMque_Ctrl_Action  LCMque_Ctrl_Action;	// キュー管理情報

extern	uchar	IFM_GetBPSforSalve(void);

//キュー関連
extern	void				EnQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pAdd, int iWhere);
extern	struct t_IF_Queue	*GetQueue(t_IF_Queue_ctl *pCtl);
extern	int					CheckQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pQue, int iWhere);
extern	struct t_IF_Queue	*DeQueue(t_IF_Queue_ctl *pCtl, int iWhere);
extern	struct t_IF_Queue	*GetQueueBuff(struct t_IF_Queue *pQ, ushort byteSize);
extern	t_IFM_Queue			*GetQueueBody(t_IFM_Queue *pQ, ushort byteSize);


//エラー発生/解除履歴
extern	void	IFM_MasterError(int iError, uchar ucNow);
extern	void	IFM_SlaveError(int iError, uchar ucNow, int iSlave);

extern	void	EnQueue2(t_LCMque_Ctrl_Action *pCtl, struct t_IF_Queue *pAdd, int iWhere, BOOL bRetry);
extern	struct t_IF_Queue	*DeQueue2(t_LCMque_Ctrl_Action *pCtl);
extern	struct t_IF_Queue	*RemoveQueue2(t_LCMque_Ctrl_Action *pCtl, int iWhere);
extern	int	MatchRetryQueue_Lock(t_IF_Queue_ctl *pCtl, uchar ucSlave, uchar ucLock);

extern	int Seach_Close_command_sameno(t_IF_Queue_ctl *pCtl,uchar tno);
extern	int Seach_Close_command(t_IF_Queue_ctl *pCtl,ushort scount);
extern	int Get_Close_command_cnt(t_IF_Queue_ctl *pCtl);

/*----------------------------------*/
/*			LCMmain.c				*/
/*----------------------------------*/
extern	void	LCM_SetStartAction(t_IFM_Queue *pMail);	// toScom_Send_or_NextRetry(toScom.c) が使う

extern	void	LCMtim_1secTimerStop(int iTerm, int iLock);
BOOL	LCM_HasFlapForceControl();

#endif	// _IFM_H_


