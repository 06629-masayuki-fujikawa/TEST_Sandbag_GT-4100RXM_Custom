#ifndef	___CRE_CTRL_H___
#define	___CRE_CTRL_H___
/*[]----------------------------------------------------------------------[]
 *| System      : FT4000
 *| Module      : CRE通信制御共通定義
 *[]----------------------------------------------------------------------[]
 *| Date        : 2013.07.01
 *| Update      : 
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/


// ｸﾚｼﾞｯﾄ機能内部共通エリア
typedef struct st_cre_ctl {

	uchar	*RcvData;							// 受信バッファポインタ

	uchar	Initial_Connect_Done;				// 初回接続完了
	uchar	Credit_Stop;						// クレジット精算停止中

	short	Status;								// 状態番号
	short	Event;								// 起動要因
	short	OpenKind;							// 開局コマンド要因
	short	SalesKind;							// 売上データ送信要因
	
	ushort	SeqNo;								// ｸﾚｼﾞｯﾄ電文処理追い番
	ulong	SlipNo;								// 端末処理通番（伝票№）

	uchar	Timeout_Announce;					// 受信タイムアウト時のアナウンス可否フラグ
	short	MessagePtnNum;						// LCD表示 cre 固有のﾊﾟﾀｰﾝ番号

	ushort	Result_Wait;						// 応答電文用待ち時間

	struct st_open {							// 開局コマンド用リトライ制御
		char	Cnt;							// リトライ回数（３回目まで数える）
		char	Stage;							// 0:１～２回目、1:３回目以降
		char	Min;							// 経過時間（分）
		char	Wait[2];						// [0]:５分、[1]:５０分
	} Open;

	struct st_sales {							// 売上依頼用リトライ制御
		char	Cnt;							// リトライ回数
		char	Stage;							// 0:１～２回目、1:３回目以降
		char	Min;							// 経過時間（分）
		char	Wait[2];						// [0]:５分、[1]:５０分
	} Sales;

	struct st_onlinetest {						// 生存確認用リトライ制御
		char	Min;							// 経過時間（分）
		char	Wait;							// 実行間隔(分)
	} OnlineTest;

	// 退避エリア（応答データチェック用）
	struct st_save {
		ushort			SeqNo;					// 処理追い番
		date_time_rec2	Date;					// 処理年月日時分秒
		ulong			SlipNo;					// 端末処理通番（伝票№）
		ulong			Amount;					// 売上金額
		uchar			AppNo[6];				// 承認番号
		uchar			ShopAccountNo[20];		// 加盟店取引番号
		uchar			TestKind;				// ﾃｽﾄ種別
	} Save;

	date_time_rec2	PayStartTime;				// 精算開始時刻

} td_cre_ctl;

extern td_cre_ctl	cre_ctl;


// 追い番範囲
#define		CRE_SEQ_MIN		1					// 処理追い番
#define		CRE_SEQ_MAX		0xFFFF
#define		CRE_SLIP_MIN	1					// 端末処理通番（伝票№）
#define		CRE_SLIP_MAX	99999

// 状態番号: Status
#define		CRE_STS_IDLE				0		// 待機
#define		CRE_STS_WAIT_OPEN			1		// 02待ち
#define		CRE_STS_WAIT_CONFIRM		2		// 04待ち
#define		CRE_STS_WAIT_SALES			3		// 06待ち
#define		CRE_STS_WAIT_ONLINETEST		4		// 08待ち
#define		CRE_STS_WAIT_RETURN			5		// 10待ち

// 起動要因: Event
#define		CRE_EVT_SEND_OPEN			1		// 開局コマンド(01)
#define		CRE_EVT_SEND_CONFIRM		2		// 与信問合せ(03)
#define		CRE_EVT_SEND_SALES			3		// 売上依頼(05)
#define		CRE_EVT_SEND_ONLINETEST		4		// ｵﾝﾗｲﾝﾃｽﾄ(07)
#define		CRE_EVT_SEND_RETURN			5		// 返品問合せ(09)
#define		CRE_EVT_RECV_ANSWER			11		// 応答電文受信
#define		CRE_EVT_TIME_UP				21		// 応答ﾀｲﾑｱｳﾄ

// 送信コマンド種別
#define		CRE_SNDCMD_OPEN				1		// 開局コマンド(01)
#define		CRE_SNDCMD_CONFIRM			3		// 与信問合せ(03)
#define		CRE_SNDCMD_SALES			5		// 売上依頼(05)
#define		CRE_SNDCMD_ONLINETEST		7		// ｵﾝﾗｲﾝﾃｽﾄ(07)
#define		CRE_SNDCMD_RETURN			9		// 返品問合せ(09)

// 受信コマンド種別
#define		CRE_RCVCMD_OPEN				2		// 開局コマンド応答(02)
#define		CRE_RCVCMD_CONFIRM			4		// 与信問合せ結果(04)
#define		CRE_RCVCMD_SALES			6		// 売上依頼結果(06)
#define		CRE_RCVCMD_ONLINETEST		8		// ｵﾝﾗｲﾝﾃｽﾄ応答(08)
#define		CRE_RCVCMD_RETURN			10		// 返品問合せ結果(10)

// コマンドタイムアウト
#define		CRE_CMD_TIMEOUT				0xff	// コマンドタイムアウト

// コマンド送信要因
#define		CRE_KIND_STARTUP			0		// 起動
#define		CRE_KIND_MANUAL				1		// 手動(メンテナンスから)
#define		CRE_KIND_RETRY				2		// リトライ
#define		CRE_KIND_PAYMENT			3		// 精算動作
#define		CRE_KIND_REOPEN				4		// 再開局(拡張用:現在未使用)
#define		CRE_KIND_AUTO				5		// 自動

// カードＩＤ
#define		CREDIT_CARDID_JIS1			0x7F	// ＪＩＳ１（ＩＳＯカード）7F=内部番号
#define		CREDIT_CARDID_JIS2			0x61	// ＪＩＳ２のリードＩＤ(JIS2,JIS1以外でプリペイド,係員カード、以外のカードも、JIS2扱いとしてｾﾝﾀｰに問い合わせする）

// 端末識別番号サイズ
#define		CREDIT_TERM_ID_NO_SIZE		13		// 遠隔クレジットの端末識別番号サイズ

// 開局コマンド・リトライ
#define		CRE_RETRY_CNT_0				2		// Stage1:繰返し回数
#define		CRE_RETRY_CNT_1				3		// Stage2:移行回数
#define		CRE_RETRY_WAIT_0			5		// Stage1:間隔
#define		CRE_RETRY_WAIT_1			50		// Stage2:間隔

#define		CRE_STATUS_OK				0x00	// クレジット正常
#define		CRE_STATUS_DISABLED			0x01	// クレジット設定なし
#define		CRE_STATUS_NOTINIT			0x02	// 初回接続未完了
#define		CRE_STATUS_STOP				0x04	// クレジット停止中
#define		CRE_STATUS_UNSEND			0x08	// 売上依頼未送信
#define		CRE_STATUS_SENDING			0x10	// 通信中
#define		CRE_STATUS_PPP_DISCONNECT	0x20	// PPP未接続

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信（未使用定義削除））（FT-4000N：MH364304流用）
//// クレジットサーバー接続有無
//#define		CREDIT_ENABLED()		( (prm_get(COM_PRM, S_PAY, 24, 1, 2)) == 2 || (prm_get(COM_PRM, S_PAY, 24, 1, 2)) == 3)	// 1=CCT(未対応)
//																															// 2=CRE(FOMA)
//																															// 3=CRE(LAN)
//
//
//// 精算情報
//typedef struct st_creSeisanInfo {	// opetask => creCtrl
//	long			amount;				// 売上金額
//	unsigned char	jis_1[37];			// JIS1ｶｰﾄﾞﾃﾞｰﾀ
//	unsigned char	jis_2[69];			// JIS2ｶｰﾄﾞﾃﾞｰﾀ
//} td_creSeisanInfo;
//
//extern td_creSeisanInfo		creSeisanInfo;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信（未使用定義削除））（FT-4000N：MH364304流用）


// プロトタイプ

//cre_ctrl.c
extern	short	creCtrl( short event );
extern	void	creCtrlInit( uchar flg );
extern	uchar	creStatusCheck( void );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信（未使用定義削除））（FT-4000N：MH364304流用）
//extern	short	creLimitCheck( ulong Amount );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信（未使用定義削除））（FT-4000N：MH364304流用）
extern	void	creOneMinutesCheckProc( void );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信（未使用定義削除））（FT-4000N：MH364304流用）
//extern	short	creOnlineTestCheck( void );
//extern	short	creErrorCheck( void );
//
//short			creSendData_OPEN( void );		// 開局コマンド(01)
//short			creSendData_CONFIRM( void );	// 与信問合せデータ(03)
//short			creSendData_SALES( void );		// 売上依頼データ(05)
//short			creSendData_ONLINETEST( void );	// ｵﾝﾗｲﾝﾃｽﾄ(07)
//short			creSendData_RETURN( void );		// 返品問合せデータ(09)
//
//short			creRecvData( void );
//
//void			creSaleNG_Add( void );
//void			creUpdatePayData( DATA_KIND_137_04 *RcvData );
//short			creTimeOut( void );
//short			creResultCheck( void );
//void			creMemCpyRight(uchar *dist, uchar *src, ushort dist_len, ushort src_len);
//extern void		creMessageAnaOnOff( short OnOff, short num );
//
//extern void		creSeqNo_Init( void );		// ｸﾚｼﾞｯﾄ電文用追い番初期化
//extern ushort	creSeqNo_Count( void );		// ｸﾚｼﾞｯﾄ電文用追い番カウント
//extern ushort	creSeqNo_Get( void );		// ｸﾚｼﾞｯﾄ電文用追い番：現在値を返す
//extern ulong	creSlipNo_Count( void );	// 端末処理通番（伝票番号）カウント
//extern ulong	creSlipNo_Get( void );		// 端末処理通番（伝票番号）：現在値を返す
//extern void		creSales_Init( void );		// 売上依頼用リトライ制御データ初期化
//extern void		creSales_Reset( void );		// 売上依頼の経過時間カウントをリセット
//extern uchar	creSales_Check( void );		// 売上依頼の経過時間をチェック
//extern void		creSales_Count( void );		// 売上依頼データ再送回数カウント
//extern void		creSales_Send( short kind );	// 売上依頼の再送処理
//extern void		creOpen_Init( void );		// 開局コマンド用リトライ制御データ初期化
//extern void		creOpen_Enable( void );		// 開局コマンドのリトライ送信許可
//extern void		creOpen_Disable( void );	// 開局コマンドのリトライ送信禁止
//extern uchar	creOpen_Check( void );		// 開局コマンドの経過時間をチェック
//extern void		creOpen_Send( void );		// 開局コマンド送信処理
//extern void		creOnlineTest_Init( void );	// ｵﾝﾗｲﾝﾃｽﾄの制御データ初期化
//extern void		creOnlineTest_Reset( void );// ｵﾝﾗｲﾝﾃｽﾄの経過時間カウントをリセット
//extern void		creOnlineTest_Count( void );// ｵﾝﾗｲﾝﾃｽﾄの経過時間をカウント
//extern uchar	creOnlineTest_Check( void );// ｵﾝﾗｲﾝﾃｽﾄの経過時間をチェック
//extern void		creOnlineTest_Send( void );	// ｵﾝﾗｲﾝﾃｽﾄの送信処理
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信（未使用定義削除））（FT-4000N：MH364304流用）
extern void		creInfoInit( void );		// クレジット情報初期化
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信（未使用定義削除））（FT-4000N：MH364304流用）
//extern void		creRegMonitor( ushort code, uchar type, ulong info );	// クレジット関連モニタ登録
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信（未使用定義削除））（FT-4000N：MH364304流用）
extern	void	creCheckRejectSaleData( void );

#endif	/* ___CRE_CTRL_H___ */
