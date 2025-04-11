/*[]----------------------------------------------------------------------[]*/
/*| operation main control                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-02-01                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
#include	<stddef.h>
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
#include	<stdio.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"pri_def.h"
#include	"tbl_rkn.h"
#include	"cnm_def.h"
#include	"flp_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"L_FLASHROM.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"mdl_def.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"prm_tbl.h"
#include	"mif.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"ope_ifm.h"
#include	"ifm_ctrl.h"

#include	"AppServ.h"
#include	"remote_dl.h"
#include	"I2c_driver.h"
#include	"rtc_readwrite.h"

#include	"lcd_def.h"
#include	"ntnetauto.h"
#include	"cre_ctrl.h"

// GG124100(S) R.Endo 2022/06/13 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/06/13 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
#include	"pkt_def.h"
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
#include	"aes_sub.h"
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
#include	"pktctrl.h"

extern uchar Is_in_lagtim( void );
extern short OnlineDiscount(void);
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
extern short GetTypeSwitch(ushort syu, ushort card_no);
extern short CheckDiscount(ushort syu, ushort card_no, ushort info);
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
extern BOOL IsDupSyubetuwari(void);
// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// extern void CopyDiscountCalcInfoToZaisha(void);
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

extern void Test_Ope_EnterLog(void);

uchar ryodsp;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//t_Edy_Dsp_erea	edy_dsp;					// 画面表示制御用
//ulong	Edy_Pay_Work;						// 減算料金格納用
//uchar	Dsp_Prm_Setting;					// 電子媒体使用設定状態
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
 
t_End_Status	cansel_status;				// 精算中止用Edy・Suica・cmn終了待ち
t_End_Status	CCT_Cansel_Status;			// ｸﾚｼﾞｯﾄｶｰﾄﾞ使用時のEdy・Suica停止待ち用

char	TekKoshinCRE_flg;		//定期更新でのクレジット精算処理中フラグ

char	TekKoshinCRE_faz;		// 更新定期ﾓｰﾄﾞでのｸﾚｼﾞｯﾄ精算ﾎﾞﾀﾝﾌﾗｸﾞ
								// 0= 通常(F3が押されない or Suicaなし設定)
								// 1= F3押下されSuica停止待ち状態
								// 2= Suica停止後にｸﾚｼﾞｯﾄ精算へ移行する状態
uchar	Ope_Last_Use_Card;					// 次のカードのうち最後に使用したもの
											// 1=プリペ，2=回数券，3=電子決済カード
											// 0=上記まだ未使用

short	tim1_mov;
static void	op_Cycle_BUZPIPI( void );
static void	op_IccLedOff( void );
static void	op_IccLedOff2( void );
static void	op_IccLedOnOff( void );
static void op_RestoreErrDsp( uchar dsp );

static void		SyuseiDataSet( ushort mot_lkno );
static short	SyuseiModosiPay( short rt );

char	First_Pay_Dsp( void );
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//uchar	FurikaeCancelFlg;		// 振替精算失敗フラグ
//uchar	FurikaeMotoSts;			// 振替元状態フラグ 0:チェック不要 / 1:車あり / 2:車なし
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
uchar	MifStat;			// Mifare処理状態

uchar	Syusei_Select_Flg;
uchar	CreditReqResult;				// クレジットカード精算問い合わせ結果情報
uchar CardStackStatus;			// カード詰まり処理状態
static ulong EjectSafetyTime;			// カード連続排出抑止用
static uchar EjectActionCnt;			// 排出動作回数
static uchar OutputRelease;				// カード詰まり解除出力(0:出力あり/!0:出力なし)
void StackCardEject(uchar req);			// カード詰まり時の処理関数
#define CARD_EJECT_AUTOLAG		(ushort)((prm_get(COM_PRM, S_SYS, 14, 2, 3) * 50) + 1)
#define CARD_EJECT_MANULAG		(ulong)((prm_get(COM_PRM, S_SYS, 14, 2, 5) * 100) + 1)
#define IS_ERR_RED_STACK		((ERR_CHK[ERRMDL_READER][ERR_RED_STACK_R]) || (ERR_CHK[ERRMDL_READER][ERR_RED_STACK_P]))
uchar	CardStackRetry;					// 排出動作中ﾌﾗｸﾞ(磁気リーダ制御用)
										// 排出動作中はリードコマンドを投げない
static  long	decode_number=0;
static  long	op_Calc_Notice( ushort stimer, struct clk_rec *pDate, long sParkNo );
static  long	op_Calc_Ansyou( long lNoticeNo );
static	long	op_Calc_BekiJyo( long Val1, long Val2 );
static	void	teninb_pass2( ushort nu, ushort in_k, ushort lin, ushort col, uchar pas_cnt);
#define ONE_CHARACTER_CHECK(x,y) (((x/y)>10) ? ((x/y)%10):(x/y))	/* 比較用に一桁に分解するマクロ */
																	/* 第１引数に暗証・出庫番号を指定し、第２引数に分解値を指定 */


#define SERCRET_MODE_OFF	0000
#define	DSP_DEFPOS	9
#define	GETA_10_6	1000000L
#define	GETA_10_5	100000L
#define	GETA_10_4	10000L
#define	GETA_10_3	1000L
#define	GETA_10_2	100L
#define	GETA_10_1	10L
#define	MAXVAL		9999L
#define	ZeroToMitsu	3L
char	shomei_errnum;
#define KEEP_EVT_MAX	10
ushort	Keep_Evt_Buf[KEEP_EVT_MAX];

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//void	Key_Event_Set( ushort msg );
//ushort	Key_Event_Get( void );
//ushort	key_event_keep[4];
//
//ushort	dspErrMsg;
//
//static	uchar	f_al_card_wait;			// 1=磁気カードが入れられ、al_card() 実施前に電子マネーリーダなどの停止待ちをしている
// MH810100(S) 2020/09/09 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
//static struct clk_rec	PayStartTime;			// 精算開始入時刻(精算が開始された時の現在時刻)
// MH810100(E) 2020/09/09 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
static DATE_YMDHMS		PayTargetTime;			// 精算時刻(精算開始時刻。未払精算では出庫時刻となる)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))

static	uchar	f_MandetMask;			// 人体検知マスク：0=なし / 1=あり
#define	TIMER_MANDETMASK	27			// 人体検知マスクタイマーNo.
// MH321800(S) T.Nagai ICクレジット対応
uchar	coin_err_flg = 0;				// 硬貨使用不可ﾌﾗｸﾞ
uchar	note_err_flg = 0;				// 紙幣使用不可ﾌﾗｸﾞ
// MH321800(E) T.Nagai ICクレジット対応
// MH810105(S) MH364301 インボイス対応
uchar	f_reci_ana;						// 延長後の領収証ボタン押下
// MH810105(E) MH364301 インボイス対応
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
short	jnl_pri_wait_Timer;				// ジャーナル印字完了待ちタイマー
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
ushort 	rpt_pri_wait_Timer;				// レシート印字完了待ちタイマー
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）

extern	enum I2cResultFnc_e 	I2C_Request(I2C_REQUEST *pRequest, unsigned char ExeMode);

static	void	ryo_cal_sim(void);
static	void	init_ryocalsim(void);
static	void	set_tim_ryocalsim(ushort num, struct clk_rec *indate, struct clk_rec *outdate);
// MH322914(S) K.Onodera 2016/08/08 AI-V対応
static void		op_mod01_Init( void );
static	int		Ope_PipRemoteCalcTimePre( void );
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//static	int		Ope_PipFurikaeCalc( void );
static	void	lcdbm_notice_opn( uchar	opn_cls,  uchar	opn_cls_reason );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH322914(E) K.Onodera 2016/08/08 AI-V対応
extern	uchar	SysMnt_Work[];			// ｼｽﾃﾑﾒﾝﾃ workarea (64KB)
// MH322914(S) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
short	IsMagReaderRunning();
// MH322914(E) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
// MH810100(S) K.Onodera 2019/12/27 車番チケットレス（精算ID対応）
static void op_SetSequenceID( ulong id );
static ulong op_GetSequenceID( void );
static void op_ClearSequenceID( void );
// GG129000(S) ゲート式車番チケットレスシステム対応（改善連絡No.79)
//static BOOL isSeasonCardType( stMasterInfo_t* pstMasterInfo );
static BOOL isSeasonCardType( stMasterInfo_t* pstMasterInfo, stZaishaInfo_t* pstZaishaInfo );
// GG129000(E) ゲート式車番チケットレスシステム対応（改善連絡No.79)

static eSEASON_CHK_RESULT CheckSeasonCardData( lcdbm_rsp_in_car_info_t* pstCardData );
static eSEASON_CHK_RESULT CheckSeasonCardData_main( lcdbm_rsp_in_car_info_t* pstCardData, ushort no );
static void SetPayTargetTime( DATE_YMDHMS *pTime );
static DATE_YMDHMS* GetPayTargetTime( void );
static short set_tim_only_out_card( short err_mode );
// MH810100(E) K.Onodera 2019/12/27 車番チケットレス（精算ID対応）
// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
static void SetPayStartTimeInCarInfo();
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// static short in_time_set();
static short in_time_set(uchar firstFlg);
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
// MH810100(S) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）
void SetQRMediabyQueryResult();
// MH810100(E) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）

// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
extern void LongTermParkingRel( ulong LockNo , uchar knd, flp_com *flp);
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//extern	char	pcard_shtter_ctl;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
// MH810100(S) Y.Yamauchi 2020/02/28 車番チケットレス(メンテナンス)
extern	uchar	Anounce_volume	( uchar volume );
extern	uchar	Key_volume ( uchar volume, uchar kind);
// MH810100(E) Y.Yamauchi 2020/02/28 車番チケットレス(メンテナンス)
// MH321800(S) Y.Tanizaki ICクレジット対応
#if (6 == AUTO_PAYMENT_PROGRAM)
static	ushort	auto_payment_locksts_set( void );
static	short	car_index = INT_CAR_START_INDEX;
static	uchar	is_auto_pay = 0;
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810105(S) MH364301 QRコード決済対応
static	void	lcd_Recv_FailureContactData_edit(void);
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 インボイス対応
static	void	ryo_reisu(void);
static 	void	receipt_output_error(void);
static	void	statusChange_DispUpdate(void);
// MH810105(E) MH364301 インボイス対応

// MH810100(S) テスト用入庫ログ作成
#ifdef TEST_ENTER_LOG
void Test_Ope_EnterLog(void);
#endif
// MH810100(E) テスト用入庫ログ作成
// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
uchar autopay_coin_cnt = 0;

#define AP_CHARGE_INTERVAL_SEC		10		// コイン投入間隔秒
#define AP_PARKING_FEE			   	200		// 駐車料金
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算

// MH810101(S) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
static uchar season_chk_result;	// 定期チェック結果
// MH810101(E) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
static uchar cmp_send;
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
struct	stLaneDataInfo	m_stLaneWork;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102流用）（GM803003流用）
uchar r_zero_call;							// 0円精算による表示短縮フラグ
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102流用）（GM803003流用）

/*[]----------------------------------------------------------------------[]*/
/*| 通常精算                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpeMain( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpeMain( void )
{
	short	ret;

	ac_flg.cycl_fg = 0;												// 0:待機状態
	if( 0 != OPECTL.Kakari_Num ){									// ﾒﾝﾃﾅﾝｽ画面からの戻り
		if( OPECTL.Pay_mod != 2 ){									// 修正精算以外
		wopelg( OPLOG_MNT_END, 0L, (ulong)OPECTL.Kakari_Num );		// 係員操作終了操作ﾛｸﾞ登録
		OPECTL.Kakari_Num = 0;										// 係員№ｸﾘｱ
		}
	}

	rd_shutter();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//	if(( CP_MODECHG == 0 )&&										// Mode change key ON?
//	   ( OPECTL.Pay_mod == 0 )){									// 通常精算
//		OPECTL.Mnt_mod = 1;											// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
//		Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//		return;
//	}
//
//	if( CP_MODECHG == 1 ){											// Mode change key OFF?
//		OPECTL.Pay_mod = 0;											// 通常精算
//	}
//
//	if( DoorCloseChk() >= 100 ){									// ﾄﾞｱ閉時の内部ｽｲｯﾁﾁｪｯｸ
//		if( OPECTL.Pay_mod == 2 ){									// 修正精算処理?
//		}else{
//			OPECTL.Ope_mod = 0;										// 待機に戻る
//			if( opncls() == 2 ){									// 休業?
//				OPECTL.Ope_mod = 100;								// 休業処理へ
//			}
//		}
//	}else{
//		OPECTL.Ope_mod = 110;										// ﾄﾞｱ閉時ｱﾗｰﾑ表示処理へ
//	}
	// PowerOn起動時はﾄﾞｱ開閉状態に拘わらず op_init00()へ移行する
	if (OPECTL.Ope_mod != 255) {
		// メンテキーON？
		if(( CP_MODECHG == 0 )&&										// Mode change key ON?
		   ( OPECTL.Pay_mod == 0 )){									// 通常精算
			OPECTL.Mnt_mod = 1;											// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
		   	ret = -1;
		}
		// メンテキーOFF？
		if( CP_MODECHG == 1 ){											// Mode change key OFF?
			OPECTL.Pay_mod = 0;											// 通常精算
		}
		if( DoorCloseChk() >= 100 ){									// ﾄﾞｱ閉時の内部ｽｲｯﾁﾁｪｯｸ
			if( OPECTL.Pay_mod == 2 ){									// 修正精算処理?
			}else{
				if( opncls() == 2 ){									// 休業?
					OPECTL.Ope_mod = 100;								// 休業処理へ
					// 前回営業で送信済み？
					if( OPECTL.sended_opcls != 1 ){
						lcdbm_notice_opn( 1, (uchar)CLOSE_stat );		// 休業通知
						OPECTL.sended_opcls = 1;
					}
				}else{
					OPECTL.Ope_mod = 0;									// 待機に戻る
					// 前回休業で送信済み？
					if( OPECTL.sended_opcls != 0 ){
						lcdbm_notice_opn( 0, (uchar)CLOSE_stat );		// 営業通知
						OPECTL.sended_opcls = 0;
					}
				}

			}
		}else{
			OPECTL.Ope_mod = 110;										// ﾄﾞｱ閉時ｱﾗｰﾑ表示処理へ
		}
	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))

	knum_len = binlen( (ulong)CPrmSS[S_TYP][82] );					// ｼｽﾃﾑで使用する駐車位置の桁数
	if( knum_len == 1 ){											// 1桁の場合
		knum_ket = 1;
	}else if( knum_len == 2 ){										// 2桁の場合
		knum_ket = 10;
	}else if( knum_len == 3 ){										// 3桁の場合
		knum_ket = 100;
	}else{															// 4桁以上の場合
		knum_ket = 1000;
		knum_len = 4;
	}
	LCDNO = (ushort)-1;
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
	if( isEJA_USE() ){												// 電子ジャーナル接続
		jnl_pri_wait_Timer = EJNL_PRI_WAIT_TIME;
	}
	else{
		jnl_pri_wait_Timer = JNL_PRI_WAIT_TIME;
	}
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
// MH810103 GG119202(S) JVMAリセット処理変更
	Ec_check_PendingJvmaReset();
// MH810103 GG119202(E) JVMAリセット処理変更
// MH810103 GG119202(S) 起動シーケンス不具合修正
	// 保留していたブランドネゴシエーションを再開する
	Ec_check_PendingBrandNego();
// MH810103 GG119202(E) 起動シーケンス不具合修正
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス（OPE対応）
//	for( ret = 0; ret != -1; ){
	for( ret = 0; ; ){
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス（OPE対応）
		if( ret == 10 ){ 											// 中止?
// MH810100(S) K.Onodera  2020/01/22 車番チケットレス(精算中止処理見直し)
			if( 255 != OPECTL.Ope_mod ){	// 精算中止からの起動待機遷移ではない
// MH810100(E) K.Onodera  2020/01/22 車番チケットレス(精算中止処理見直し)
				OPECTL.Ope_mod = 0;									// 待機に戻る
				ac_flg.cycl_fg = 0;									// 0:待機状態
				OPECTL.nyukin_flg = 0;								// 入金有りﾌﾗｸﾞOFF
// MH810103 GG119202(S) JVMAリセット処理変更
				Ec_check_PendingJvmaReset();
// MH810103 GG119202(E) JVMAリセット処理変更
// MH810103 GG119202(S) 起動シーケンス不具合修正
				// 保留していたブランドネゴシエーションを再開する
				Ec_check_PendingBrandNego();
// MH810103 GG119202(E) 起動シーケンス不具合修正
				if( CP_MODECHG == 0 ){								// Mode change key ON?
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス（OPE対応）
//					if( OPECTL.Pay_mod == 0 ){						// 通常精算?
//						OPECTL.Mnt_mod = 1;							// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
//						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//						return;
//					}else if( OPECTL.Pay_mod == 2 ){				// 修正精算?
//					}
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス（OPE対応）
				}else{
					OPECTL.Pay_mod = 0;								// 通常精算
					OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}
				StackCardEject(1);										/* 精算中止時の排出動作 */
// MH810100(S) K.Onodera  2020/01/22 車番チケットレス(精算中止処理見直し)
			}
// MH810100(E) K.Onodera  2020/01/22 車番チケットレス(精算中止処理見直し)
		}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//		if( 0 == OPECTL.Ope_mod ){									// 次は待機
//			if( opncls() == 2 ){									// 休業?
//				OPECTL.Ope_mod = 100;								// 休業処理へ
//			}
//		}
		if( 0 == OPECTL.Ope_mod ) {
			if( opncls() == 2 ){									// 休業?
				OPECTL.Ope_mod = 100;								// 休業処理へ
				// 営休業通知_営休業_休業理由の送信処理
				lcdbm_notice_opn( 1, (uchar)CLOSE_stat );			// 休業通知
				OPECTL.sended_opcls = 1;
			}
		}
		else if( 100 == OPECTL.Ope_mod ) {
			if( opncls() == 1 ){									// 営業?
				OPECTL.Ope_mod = 0;									// 休業処理へ
				// 営休業通知_営休業_休業理由の送信処理
				lcdbm_notice_opn( 0, (uchar)CLOSE_stat );			// 営業通知
				OPECTL.sended_opcls = 0;
			}
		}
		else if(( 255 == OPECTL.Ope_mod ) && ( ret != 10 ) && ( OPECTL.init_sts == 1 )){
			// 精算中止からの起動待機遷移ではなくかつ初期化完了済み
			if( opncls() == 1 ){									// 営業?
				OPECTL.Ope_mod = 0;									// 休業処理へ
				// 営休業通知_営休業_休業理由の送信処理
				lcdbm_notice_opn( 0, (uchar)CLOSE_stat );			// 営業通知
				OPECTL.sended_opcls = 0;
			}
			else {													// 休業?(2)
				OPECTL.Ope_mod = 100;								// 休業処理へ
				// 営休業通知_営休業_休業理由の送信処理
				lcdbm_notice_opn( 1, (uchar)CLOSE_stat );			// 休業通知
				OPECTL.sended_opcls = 1;
			}
		}

		// メンテ移行したい？
		if( OPECTL.Mnt_mod == 1 ){
			// メンテ開始応答待ち => OK受信/精算開始要求でメンテへ
			if( MOD_CHG == op_wait_mnt_start() ){
				// メンテナンスへ
				Ope_KeyRepeatEnable( OPECTL.Mnt_mod );
				break;
			}else{
				// OPEへ
				OPECTL.Pay_mod = 0;									// 通常精算
				OPECTL.Mnt_mod = 0;									// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
				Ope_KeyRepeatEnable( OPECTL.Mnt_mod );
			}
		}
		// メンテには移行しないが抜ける場合
		else{
			// op_init00()の時はﾄﾞｱ開閉のﾁｪｯｸは行わない
			if( ret == -1 && OPECTL.init_sts == 1 ){
				if( CP_MODECHG == 0 ){								// Mode change key ON?
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
					continue;
				}else{
					// OPEへ
					OPECTL.Pay_mod = 0;								// 通常精算
					OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
					Ope_KeyRepeatEnable( OPECTL.Mnt_mod );
				}
				return;
			}
		}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
		if( Suica_Rec.Data.BIT.LOG_DATA_SET == 1 &&					// 決済結果ﾃﾞｰﾀ受信済み後に
		  ( OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100 || 		// 待機か休業か
		  	OPECTL.Ope_mod == 11 || OPECTL.Ope_mod == 21 )){		// 修正精算駐車位置番号入力か定期更新選択画面表示時は
// MH321800(S) hosoda ICクレジット対応
//			Settlement_rcv_faze_err((uchar*)&Settlement_Res, 4 );	// エラー登録処理
			if (isEC_USE()) {
				EcSettlementPhaseError((uchar*)&Ec_Settlement_Res, 4 ); // エラー登録処理
			} else {
				Settlement_rcv_faze_err((uchar*)&Settlement_Res, 4 );	// エラー登録処理
			}
// MH321800(E) hosoda ICクレジット対応
		}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//		if( OPECTL.Ope_mod == 100 || OPECTL.Ope_mod == 110 ){		// 次の画面遷移が休業・警報アラームの場合は、保持している車室番号を開放
//			Key_Event_Get();										// その他は待機を経由するので待機で処理する
//		}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))

		if (OPECTL.Ope_mod == 0 ||
			OPECTL.Ope_mod == 100) {
			TimeAdjustCtrl(2);			// 自動時刻補正停止解除
		}
		else {
			TimeAdjustCtrl(1);			// 自動時刻補正停止
		}
		switch( OPECTL.Ope_mod ){
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			case 255:
				// ﾁｹｯﾄﾚｽ起動通知/起動完了通知受信
				ret = op_init00();
				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			case 0:
				ret = op_mod00();									// 待機
				break;
			case 1:
				ret = op_mod01();									// 駐車位置番号入力処理
				break;
			case 2:
				NTNET_ClrSetup();
				ret = op_mod02();									// 料金表示,入金処理
				break;
			case 3:
				ret = op_mod03();									// 精算完了処理
				break;
			case 11:
				ret = op_mod11();									// 修正精算駐車位置番号入力処理
				break;
			case 12:
				ret = op_mod12();									// 修正精算確認処理（旧：修正精算料金表示）
				break;
#if SYUSEI_PAYMENT
			case 13:
				ret = op_mod13();									// 修正精算入金処理
				break;
			case 14:
				ret = op_mod14();									// 修正精算完了処理
				break;
#endif		// SYUSEI_PAYMENT

#if	UPDATE_A_PASS
			case 21:
				ret = op_mod21();									// 定期券更新 券挿入待ち処理
				break;
			case 22:
				ret = op_mod22();									// 定期券更新 料金表示,入金処理
				break;
			case 23:
				ret = op_mod23();									// 定期券更新 精算完了処理
				break;
#endif	// UPDATE_A_PASS
			case 70:
				ret = op_mod70();									// ﾊﾟｽﾜｰﾄﾞ入力処理(精算時)
				break;
			case 80:
				ret = op_mod80();									// ﾊﾟｽﾜｰﾄﾞ登録処理(入庫時)
				break;
			case 90:
				ret = op_mod90();									// 受付券発行処理
				break;
			case 100:
				ret = op_mod100();									// 休業処理
				break;
			case 110:
				ret = op_mod110();									// ﾄﾞｱ閉時ｱﾗｰﾑ表示処理
				break;
			case 200:
				ret = op_mod200();									// 暗証番号(Btype)出庫操作
				break;
			case 210:
				ret = op_mod210();									// 暗証番号(Btype)入力操作
				break;
			case 220:
				ret = op_mod220();									// ラグタイム延長処理
				break;
			case 230:
				ret = op_mod230();									// 精算済み案内処理
				break;
			case 81:
			case 91:
			case 92:
				ret = op_mod81(OPECTL.Ope_mod);						// 受付、ﾊﾟｽﾜｰﾄﾞエラー表示
				break;
			default:
				ret = 10;
				break;
		}
	}
}

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//[]----------------------------------------------------------------------[]
///	@brief			LCDﾓｼﾞｭｰﾙに対するﾊﾟﾗﾒｰﾀｱｯﾌﾟﾛｰﾄﾞを行う
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			result : 実行結果
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
int lcdbm_setting_upload_FTP( void )
{
	int ret;
	int result = 1;	// 1=NG
	int retry = 0;

#if GT4100_NO_LCD
	return(OK);
#endif

// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
// GM760201(S) センターアップロードとの排他条件の改善
//	if( TASK_START_FOR_DL() ){
	if( DOWNLOADING() ){
// GM760201(E) センターアップロードとの排他条件の改善
		return 2;
	}
	OPECTL.lcd_prm_update = 1;	// LCDへのパラメータアップロード中
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)

	// original含めてMax 4回行う
	for( retry = 0; retry < 4; retry++ ) {
		/****************************************************************/
		/* LCDﾓｼﾞｭｰﾙに対するﾊﾟﾗﾒｰﾀｱｯﾌﾟﾛｰﾄﾞ(ftpctrl.resultの値を返す)	*/
		/* 		while( ftpctrl.start )でloop	( FTP_IDLE == 0 )		*/
		/* 		TIMEOUT25 = FTP_Execute()処理実行						*/
		/* 		TIMEOUT19 = ftp_send_quit(0)処理実行					*/
		/****************************************************************/
		ret = ftp_auto_update_for_LCD();
				//	AU_DWL_SCR 			0
				//	AU_CONFIRM			1
				//	AU_EXEC				2
				//	AU_NORMAL_END		10
				//	AU_ABORT_END		11
				//	AU_NO_UPDATE		12
				//	AU_CANCEL			13
				//	AU_DISCONNECT		14
				//	AU_LOGIN_ERR		15
				//	AU_LOGOUT_ERR		16

		ret &= ~0x8000;
		// ｱｯﾌﾟﾃﾞｰﾄは正常終了？
		if( ret == 10 ){	// AU_NORMAL_END
			result = 0;		// 0=OK
			break;
		}
	}
// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
	OPECTL.lcd_prm_update = 0;	// LCDへのパラメータアップロード中解除
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)

	// NG？
	if( result ){
		// エラー登録
		err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_PRM_UPLD_FL, 1, 2, 0, &ret );
	}

	return(result);
}

// MH810100(S) K.Onodera 2020/03/13 車番チケットレス(待機状態遷移処理共通化)
//[]----------------------------------------------------------------------[]
///	@brief			待機状態遷移共通処理
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@note			起動完了通知を受信して待機状態に戻る際の共通処理
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void	ope_idle_transit_common( void )
{
	uchar	result_PKTcmd;
// MH810100(S) Y.Yamauchi #3965 ブザー音量が正しくならない
	uchar	volume;	// 音量
// MH810100(E) Y.Yamauchi #3965 ブザー音量が正しくならない

	// LCD輝度設定	輝度値(0～15) 0=ﾊﾞｯｸﾗｲﾄOFF （02-0028⑤⑥ = 消灯時の輝度(待機時)）
// MH810100(S) Y.Yamauchi #3901 バックライトの点灯/消灯が設定通りに動作しない
	LcdBackLightCtrl(ON);
// MH810100(E) Y.Yamauchi #3901 バックライトの点灯/消灯が設定通りに動作しない

	// ﾌﾞｻﾞｰ音量設定	音量 = 0～3(0=無音,3=最大音量)
	LcdSend.lcdbm_BUZ_volume = (uchar)prm_get(COM_PRM, S_PAY, 29, 1, 2);	// 02-0029⑤ = 0=音なし／1=大／2=中／3=小
	volume = Key_volume( LcdSend.lcdbm_audio_volume, 1);					// キー音量
	result_PKTcmd = PKTcmd_beep_volume( volume );							// ブザー要求（設定）
	if (result_PKTcmd == FALSE) {
		// error
	}

	// ｱﾅｳﾝｽ音量		0～100(0=無音)
	LcdSend.lcdbm_audio_volume = (ushort)prm_get(COM_PRM, S_SYS, 51, 2, 5);	// 01-0051	音量設定(ch0)	①②:音量ﾊﾟﾀｰﾝ1  01～15/00=無音
																			//							③④:音量ﾊﾟﾀｰﾝ2  01～15/00=無音
																			//							⑤⑥:音量ﾊﾟﾀｰﾝ3  01～15/00=無音	080000
// MH810100(S) Y.Yamauchi #3965 ブザー音量が正しくならない
	volume = Anounce_volume( LcdSend.lcdbm_audio_volume );					// アナウンス音量取得
	result_PKTcmd = PKTcmd_audio_volume( volume );							// アナウンス音量をLCDに送信
// MH810100(E) Y.Yamauchi #3965 ブザー音量が正しくならない

	// 時計
	PKTcmd_clock( 0 );														// RXMの時計の値を送信
	if (result_PKTcmd == FALSE) {
		// error
	}

	// 起動完了通知受信時の起動完了状態更新
	PktStartUpStsChg( 2 );
}

// MH810100(E) K.Onodera 2020/03/13 車番チケットレス(待機状態遷移処理共通化)
/*[]----------------------------------------------------------------------[]*/
/*| ﾁｹｯﾄﾚｽ用起動待機                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_init00( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = 他処理へ切換                              |*/
/*|                       : -1 = ﾓｰﾄﾞ切替                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2019-11-15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short	op_init00( void )
{
	short	ret;
	ushort	msg = 0;
	int		result;
// MH810100(S) K.Onodera  2020/02/21 #3858 車番チケットレス(起動待機状態無限ループ防止)
	ushort	sta_tim;
// MH810100(E) K.Onodera  2020/02/21 #3858 車番チケットレス(起動待機状態無限ループ防止)
// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
	uchar	req_ftp = OFF;
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)

// MH810100(S) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
	// LCDとの接続が切れたので、保持している精算状態をクリアしておく
	clear_pay_status();
// MH810100(E) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)

// MH810100(S) K.Onodera  2020/03/23 #3858 車番チケットレス(起動待機状態無限ループ防止)
	if(( OPECTL.init_sts == 0xff ) || ( OPECTL.init_sts == 0xfe )){	// 起動通知(0/1)受信後？
		if( OPECTL.init_sts == 0xff ){	// 起動通知(0)受信後？
			// 設定アップロード処理(FTP)
			result = lcdbm_setting_upload_FTP();
// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
			if( result == 2 ){	// 遠隔ダウンロード中？
				Lagtim( OPETCBNO, 1, 10*50 );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
			}else{
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
				// 設定アップロード通知
				lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)result );
// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
			}
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
		}
// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
//		OPECTL.init_sts = 1;
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:221)対応
		else {	// 起動通知(1)受信後？
			result = 0;
		}
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:221)対応
		if( result != 2 ){
			OPECTL.init_sts = 1;
		}
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
	}
// GG120600(S) // Phase9 #6185 設定アップロード実行延期中にシステム切替で共通パラメータ変更すると、「 [設定アップロード機能]-[処理開始判定結果]-[正常開始](7000100)」が発生する
	// GetMessage前に実施（歩進が動作することがあるので）
	if(mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE){
		// parkingWeb接続あり
		if(_is_ntnet_remote()) {
			// 端末で設定パラメータ変更
			rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
			// 端末側で設定が更新されたため、予約が入っていたらキャンセルを行う
			remotedl_cancel_setting();
		}
		mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
	}
// GG120600(E) // Phase9 #6185 設定アップロード実行延期中にシステム切替で共通パラメータ変更すると、「 [設定アップロード機能]-[処理開始判定結果]-[正常開始](7000100)」が発生する

	sta_tim = (ushort)prm_get( COM_PRM, S_PKT, 26, 3, 1 );
	if(( sta_tim < 1 ) || ( 999 < sta_tim )){
		sta_tim = 180;
	}
	// LCD起動通知/起動完了通知待ちタイマ開始
	Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
// MH810100(E) K.Onodera  2020/03/23 #3858 車番チケットレス(起動待機状態無限ループ防止)

	// ret != 0 まで無限loop
	for( ret = 0; ret == 0; ){
		msg = GetMessage();

		switch( msg ){

			// 操作通知
			case LCD_OPERATION_NOTICE:
				// 操作コード
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {

					// 起動通知
					case LCDBM_OPCD_STA_NOT:
						// 起動ﾓｰﾄﾞ = 設定ｱｯﾌﾟﾛｰﾄﾞ起動
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
//							// 設定アップロード処理(FTP)
//							result = lcdbm_setting_upload_FTP();
//
//							// 設定アップロード通知
//							lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)result );
//
//							OPECTL.init_sts = 1;
							req_ftp = ON;
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
						}
						// 起動ﾓｰﾄﾞ = 通常起動
						else {
							OPECTL.init_sts = 1;	// パラメータアップロード不要なので初期化完了とする
// MH810100(S) K.Onodera  2020/02/21 #3858 車番チケットレス(起動待機状態無限ループ防止)
							// LCD起動通知/起動完了通知待ちタイマ再開
							Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
// MH810100(E) K.Onodera  2020/02/21 #3858 車番チケットレス(起動待機状態無限ループ防止)
						}
						break;

					// 起動完了通知
					case LCDBM_OPCD_STA_CMP_NOT:
// MH810100(S) K.Onodera  2020/03/13 #3858 車番チケットレス(起動待機状態無限ループ防止)
						// LCD起動通知/起動完了通知待ちタイマ停止
						Lagcan( OPETCBNO, TIMERNO_LCD_STA_WAIT );
						ope_idle_transit_common();	// 待機状態遷移共通処理
						OPECTL.init_sts = 1;		// 初期化完了
						ret = -1;
// MH810100(E) K.Onodera  2020/03/13 #3858 車番チケットレス(起動待機状態無限ループ防止)
						break;
				}
				break;

// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
			// 設定アップロード待ち合わせタイムアウト
			case TIMEOUT1:
				req_ftp = ON;
				break;
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
// MH810100(S) K.Onodera  2020/02/21 #3858 車番チケットレス(起動待機状態無限ループ防止)
				// LCD起動通知/起動完了通知待ちタイマ再開
				Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
// MH810100(E) K.Onodera  2020/02/21 #3858 車番チケットレス(起動待機状態無限ループ防止)
				OPECTL.init_sts = 0;	// 起動通知から待つ
				break;

// MH810100(S) K.Onodera  2020/02/21 #3858 車番チケットレス(起動待機状態無限ループ防止)
			case TIMEOUT_LCD_STA_WAIT:	// LCD起動通知/起動完了通知待ちタイマタイムアウト
				PktResetReqFlgSet( 1 );	// LCDリセット通知要求要否セット
				OPECTL.init_sts = 0;	// 起動通知から待つ
				break;

// MH810100(E) K.Onodera  2020/02/21 #3858 車番チケットレス(起動待機状態無限ループ防止)
			default:
				break;
		}	// switch

// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
		if( req_ftp == ON ){
			req_ftp = OFF;

			Lagcan( OPETCBNO, TIMERNO_LCD_STA_WAIT );

			// 設定アップロード処理(FTP)
			result = lcdbm_setting_upload_FTP();

			// 遠隔ダウンロード中？
			if( result == 2 ){
				Lagtim( OPETCBNO, 1, 10*50 );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				// LCD起動通知/起動完了通知待ちタイマリセット
				Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
			}
			// FTP実施
			else{
				// 設定アップロード通知
				lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)result );

				OPECTL.init_sts = 1;
			}
			// LCD起動通知/起動完了通知待ちタイマ再開
			Lagtim( OPETCBNO, TIMERNO_LCD_STA_WAIT, sta_tim*50 );
		}
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)

	}	// for( ret = 0; ret == 0; ){

// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
	Lagcan( OPETCBNO, 1 );						// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
	Lagcan( OPETCBNO, TIMERNO_LCD_STA_WAIT );	// LCD起動通知/起動完了通知待ちタイマリセット
// GG120600(S) // Phase9 #6183 設定アップロードのFTP接続中に電断すると、再起動後にメンテナンスが使えなくなる
	if( DOWNLOADING() ){	
		// task_statusにも反映
		remotedl_status_set(remotedl_status_get());
	}
// GG120600(E) // Phase9 #6183 設定アップロードのFTP接続中に電断すると、再起動後にメンテナンスが使えなくなる

	return( ret );
}

//[]----------------------------------------------------------------------[]
///	@brief			操作通知送信
//[]----------------------------------------------------------------------[]
///	@param			ope_code：操作コード
///					status	：状態
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_notice_ope( eOPE_NOTIF_CODE ope_code, ushort status )
{
	uchar	result_PKTcmd;

	LcdSend.lcdbm_cmd_notice_ope.ope_code 	= (uchar)ope_code;	// 操作コード
	LcdSend.lcdbm_cmd_notice_ope.status 	= status;			// 状態

	// 操作通知送信
	result_PKTcmd = PKTcmd_notice_ope( LcdSend.lcdbm_cmd_notice_ope.ope_code, LcdSend.lcdbm_cmd_notice_ope.status );
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			QRﾃﾞｰﾀ応答(OK))送信
//[]----------------------------------------------------------------------[]
///	@param			result：結果(0:OK,1:NG(排他),2:NG(枚数上限))
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//	typedef	struct {
//		lcdbm_cmd_base_t		command;		///< ｺﾏﾝﾄﾞ共通情報
//		unsigned long			id;				///< ID(入庫から精算完了までを管理するID)
//		unsigned char			result;			///< 結果(0:OK,1:NG(排他),2:NG(枚数上限))
//		unsigned char			reserve;		///< 予備
//	} lcdbm_cmd_QR_data_res_t;
void	lcdbm_QR_data_res( uchar result )
{
	uchar	result_PKTcmd;

	// lcdbm_cmd_QR_data_res_t
	result_PKTcmd = PKTcmd_QR_data_res(
								op_GetSequenceID(),		// 入庫から精算完了までを管理するID
								result );				// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ブザー鳴動要求処理
//[]----------------------------------------------------------------------[]
///	@param			beep	: 0=ﾋﾟｯ♪､1=ﾋﾟﾋﾟｨ♪､2=ﾋﾟﾋﾟﾋﾟｨ♪
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void lcdbm_beep_start( uchar beep )
{
	uchar	result_PKTcmd;

	// 通常モード？
	if( OPECTL.Mnt_mod == 0 ){
		return;		// メンテナンス以外は、LCD自身で鳴らす
	}

	// 種類 = 0=ﾋﾟｯ♪､1=ﾋﾟﾋﾟｨ♪､2=ﾋﾟﾋﾟﾋﾟｨ♪
	LcdSend.lcdbm_BUZ_beep = beep;
	result_PKTcmd = PKTcmd_beep_start( LcdSend.lcdbm_BUZ_beep);
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			 エラー通知受信時のエラーlog登録処理
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810100(S) S.Fujii 2020/08/24 #4609 SDカードエラー対応
//const static uchar LCD_SEC_TBL[] = { 28, 30, 37, 0 };
const static uchar LCD_SEC_TBL[] = { 28, 30, 37, 31, 0 };
// MH810100(E) S.Fujii 2020/08/24 #4609 SDカードエラー対応

void lcdbm_ErrorReceiveProc( void )
{
	date_time_rec date;
	char kind = 0, sts = 0;
	ushort sec, cnt;
	lcdbm_rsp_error_t* rcv = &LcdRecv.lcdbm_rsp_error;

	// 発生解除日時
	date.Year = rcv->year;
	date.Mon  = rcv->month;
	date.Day  = rcv->day;
	date.Hour = rcv->hour;
	date.Min  = rcv->minute;

	// 現在のリアルタイム通信状態を取得
	sts = ERR_CHK[mod_realtime][1];

	// 全解除？
	if( rcv->kind == 255 ){
		for( sec=0; LCD_SEC_TBL[sec]; sec++ ){
			for( cnt=1; cnt<ERR_NUM_MAX; cnt++ ){
				WACDOG;
				// エラー解除
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:222)対応
//				err_chk3( (char)LCD_SEC_TBL[sec], (char)cnt, 0, 0, 0, 0, &date );
				err_chk3( (char)LCD_SEC_TBL[sec], (char)cnt, 0, 0, 0, 0, 0, &date );
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:222)対応
			}
		}
	}
	// 通常
	else{
		switch( rcv->occur ){
			case 1:	// 発生
				kind = 1;
				break;
			case 2:	// 解除
				kind = 0;
				break;
			case 3:	// 同時
				kind = 2;
				break;
			default:
				return;	// 異常値
				break;
		}

		// エラー登録
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:222)対応
//		err_chk3( rcv->kind, rcv->code, kind, 1, 0, rcv->info, &date );
		err_chk3( rcv->kind, rcv->code, kind, 1, 0, rcv->info, sizeof(rcv->info), &date );
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:222)対応
	}

// MH810103(s) 電子マネー対応 #5583 【検証課指摘事項】「ただいま精算できません」表示中に残高照会ができる
	// リアルタイム通信異常が発生？
	if( sts == 0 && 1 == ERR_CHK[mod_realtime][1] ){
		LcdBackLightCtrl(ON);
	}
// MH810103(e) 電子マネー対応 #5583 【検証課指摘事項】「ただいま精算できません」表示中に残高照会ができる
	// リアルタイム通信異常が解除された？
	if( sts && 0 == ERR_CHK[mod_realtime][1] ){
// MH810103(s) 電子マネー対応 #5583 【検証課指摘事項】「ただいま精算できません」表示中に残高照会ができる
		LcdBackLightCtrl(ON);
// MH810103(e) 電子マネー対応 #5583 【検証課指摘事項】「ただいま精算できません」表示中に残高照会ができる
		// リアルタイム精算データなし？
		if( 0 == Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) ){
			// 復旧データ通知(全データ送信完了)
			PKTcmd_notice_ope( (uchar)LCDBM_OPCD_RESTORE_NOTIFY, (ushort)0 );
		}
		// 精算データあり？
		else{
			// 復旧データ未送信ON
			pkt_set_restore_unsent_flg(2);
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			精算状態通知送信処理
//[]----------------------------------------------------------------------[]
///	@param			pay_status : 精算状態
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//	b0:		釣銭不足
//	b1:		ﾚｼｰﾄ用紙不足
//	b2:		ｼﾞｬｰﾅﾙ用紙不足
//	b3:		Suica利用不可
//	b4:		ｸﾚｼﾞｯﾄ利用不可
//	b5:		Edy利用不可
//	b6 ～15:予備
void	lcdbm_notice_pay( ulong pay_status )
{
	uchar	result_PKTcmd;

	// 精算状態通知
	result_PKTcmd = PKTcmd_notice_pay( pay_status );
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			営休業通知
//[]----------------------------------------------------------------------[]
///	@param			opn_cls : 営休業通知	0=営業 / 1=休業<br>
/// 				opn_cls_reason : 休業理由ｺｰﾄﾞ
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// ＜休業理由ｺｰﾄﾞ＞
//	LCDBM_OPNCLS_RSN_FORCED				// 強制営休業
//	LCDBM_OPNCLS_RSN_OUT_OF_HOURS		// 営業時間外
//	LCDBM_OPNCLS_RSN_BANKNOTE_FULL		// 紙幣金庫満杯
//	LCDBM_OPNCLS_RSN_COIN_SAFE_FULL		// ｺｲﾝ金庫満杯
//	LCDBM_OPNCLS_RSN_BANKNOTE_DROP		// 紙幣ﾘｰﾀﾞｰ脱落
//	LCDBM_OPNCLS_RSN_NO_CHANGE			// 釣銭切れ
//	LCDBM_OPNCLS_RSN_COINMECH_COMM_FAIL	// ｺｲﾝﾒｯｸ通信不良
//	LCDBM_OPNCLS_RSN_SEND_BUFF_FULL		// 送信ﾊﾞｯﾌｧﾌﾙ
//	LCDBM_OPNCLS_RSN_SIGNAL				// 信号による休業
//	LCDBM_OPNCLS_RSN_COMM				// 通信による休業
static void	lcdbm_notice_opn( uchar	opn_cls, uchar	opn_cls_reason )
{
	uchar	result_PKTcmd;

	// 営休業通知
// MH810103(s) 電子マネー対応 営休業通知修正
//	LcdSend.lcdbm_opn_cls 			= opn_cls;
//	LcdSend.lcdbm_opn_cls_reason 	= opn_cls_reason;
//	result_PKTcmd = PKTcmd_notice_opn( LcdSend.lcdbm_opn_cls, LcdSend.lcdbm_opn_cls_reason );
	LcdSend.lcdbm_opn_cls.opn_cls = opn_cls;
	LcdSend.lcdbm_opn_cls.reason = opn_cls_reason;
	result_PKTcmd = PKTcmd_notice_opn( LcdSend.lcdbm_opn_cls.opn_cls, LcdSend.lcdbm_opn_cls.reason );
// MH810103(e) 電子マネー対応 営休業通知修正
	if (result_PKTcmd == FALSE) {
		// error
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			警告通知状態取得
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			ret_warning: 警告通知状態
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//		b0:		ｺｲﾝ金庫未ｾｯﾄ
//		b1:		ｺｲﾝｶｾｯﾄ未ｾｯﾄ
//		b2:		ｺｲﾝﾒｯｸ未施錠
//		b3:		紙幣ﾘｰﾀﾞｰ蓋開
//		b4:		紙幣ﾘｰﾀﾞｰ未施錠
//		b5:		ﾚｼｰﾄﾌﾟﾘﾝﾀ蓋開
//		b6:		ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ蓋開
//		b7:		RSWが0以外 = read_rotsw()
//		b8:		CAN通信不良
//		b9:		イニシャル起動
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
////		b10～15:	予備
//		b10:	SDカード利用不可
//		b11:	レシートプリンタ設定不一致
//		b12～15:	予備
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
ushort	lcdbm_alarm_check( void )
{
	ushort	ret_warning = 0x0000;

	// b0
	if( FNT_CN_BOX_SW ){									// ｺｲﾝ金庫ｾｯﾄ?
		ret_warning |= 0x0001;
	}
	// b1
	if( CN_RDAT.r_dat0c[1] & 0x40 ){						// ｺｲﾝｶｾｯﾄ開
		ret_warning |= 0x0002;
	}
	// b2
	if( FNT_CN_DRSW ){										// ｺｲﾝﾒｯｸﾄﾞｱｽｲｯﾁ開(ｺｲﾝﾒｯｸﾄﾞｱｷｰ開)?
		ret_warning |= 0x0004;
	}
	// b3
	if( NT_RDAT.r_dat1b[0] & 0x02 ){						// ｽﾀｯｶｰ開?
		ret_warning |= 0x0008;
	}
	// b4
	if( FNT_NT_BOX_SW ){									// 紙幣金庫脱着ｽｲｯﾁ開?
		ret_warning |= 0x0010;
	}
	// b5
	if( 0 != ALM_CHK[ALMMDL_SUB][ALARM_RPAPEROPEN] ){		// ﾚｼｰﾄﾌﾟﾘﾝﾀ蓋開
		ret_warning |= 0x0020;
	}
	// b6
	if ( prm_get(COM_PRM, S_PAY, 21, 1, 1) == 1 ){			// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀあり
		if( 0 != ALM_CHK[ALMMDL_SUB][ALARM_JPAPEROPEN] ){		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ蓋開
			ret_warning |= 0x0040;
		}
// MH810104 GG119201(S) 電子ジャーナル対応
		else if( 0 != IsErrorOccuerd(ERRMDL_EJA, ERR_EJA_SD_UNAVAILABLE) ){	// SDカード使用不可
// MH810104 GG119201(S) 電子ジャーナル対応 #5944 SDカード利用不可時の精算機の警告画面が違う
//			ret_warning |= 0x0040;
			ret_warning |= 0x0400;
// MH810104 GG119201(E) 電子ジャーナル対応 #5944 SDカード利用不可時の精算機の警告画面が違う
		}
// MH810104 GG119201(E) 電子ジャーナル対応
	}
	// b7
	if (read_rotsw() != 0) {
		ret_warning |= 0x0080;
	}
	// b8 = CAN 通信不良 || CAN スタッフエラー || CAN フォームエラー
// MH810100(S) Y.Watanabe 2020/01/26 #3884 警告通知_E8601の未表示不具合修正
//	if( ERR_CHK[mod_can][ERR_CAN_COMFAIL] ){
	if(( ERR_CHK[mod_can][ERR_CAN_COMFAIL] )||( ERR_CHK[mod_can][ERR_CAN_STUFF_ERR] )||( ERR_CHK[mod_can][ERR_CAN_FORM_ERR] )){
// MH810100(E) Y.Watanabe 2020/01/26 #3884 警告通知_E8601の未表示不具合修正
		ret_warning |= 0x0100;
	}
	// b9
	if( RAMCLR ){
		ret_warning |= 0x0200;
	}
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	if( isModelMiss_R() ){
		ret_warning |= 0x0800;
	}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

	return( ret_warning );
}

//[]----------------------------------------------------------------------[]
///	@brief			警告通知送信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		lcdbm_alarm_check()の戻り値 = ﾁｪｯｸ後の警告通知状態
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_notice_alm( ushort warning )
{
	static ushort old_warning = 0;
	uchar	result_PKTcmd;

	// 現在の警告通知状態とﾁｪｯｸ後の警告通知状態に変化があった時のみ送信する
	if (old_warning != warning) {
		// 警告通知
		old_warning = warning;

		result_PKTcmd = PKTcmd_notice_alm( old_warning );

// MH810103(s) 電子マネー対応 #5413 精算不可画面または警告画面表示時に、VPが残高照会ボタンを表示してしまっている
		// 変化があったため通知
		LcdBackLightCtrl(ON);
// MH810103(e) 電子マネー対応 #5413 精算不可画面または警告画面表示時に、VPが残高照会ボタンを表示してしまっている
		if (result_PKTcmd == FALSE) {
			// error
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ポップアップ表示要求
//[]----------------------------------------------------------------------[]
///	@param[in]		DispCode：表示コード<br>
///					DispStatus：表示状態
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//	code;						status															音声				ﾒｯｾｰｼﾞ
//	1:預り証発行(精算完了時)	0:釣銭なし, 1:釣銭あり, 2:釣銭なし(領収証), 3:釣銭あり(領収証)	0,2=35／1,3＝33		0,1=22/2,3=24
//	2:預り証発行(払い戻し時)	0:釣銭なし, 1:釣銭あり, 2:釣銭なし(領収証), 3:釣銭あり(領収証)	0,2=35／1,3＝33		26(OPE_CHR_G_SALE_MSG)
//	3:減額失敗(電子mony)		---																---					---
//	4:限度超え(ICC)				---																---					29
//	5:照会NG(ICC)				---																---					30
//	6:ｸﾚｼﾞｯﾄｻｰﾊﾞ異常			---																---					31
//	7:残高不足(電子mony)		---																---					---
//	8:精算不可					---																15					13
//	9:QR利用不可				0:重複，1:データ不正，2:その他異常								0=37，1/2=26		0=14，2/3=15
// 10:割引き不可				0=精算処理区分NG, 1=割引種別NG, 2=枚数NG, 3=カード区分NG
// MH810105(S) MH364301 インボイス対応
// 11:領収証発行失敗			---	
// MH810105(E) MH364301 インボイス対応
void	lcdbm_notice_dsp( ePOP_DISP_KIND kind, uchar DispStatus )
{
	uchar	result_PKTcmd;
// MH810103(s) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更
	if(DispStatus == 198){
		DispStatus = 98;
	}
// MH810103(e) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更

// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
//	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus );
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// 	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus, 0 );
	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus, 0, NULL, 0 );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)

	if (result_PKTcmd == FALSE) {
		// error
	}
}

// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
//[]----------------------------------------------------------------------[]
///	@brief			ポップアップ表示要求(追加情報あり版)
//[]----------------------------------------------------------------------[]
///	@param[in]		DispCode：表示コード<br>
///					DispStatus：表示状態<br>
///					add_info:追加情報
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/07/20<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
//	code;						status																	ﾒｯｾｰｼﾞ							追加情報
//	1:預り証発行(精算完了時)	0:釣銭なし, 1:釣銭あり, 2:釣銭なし(領収証), 3:釣銭あり(領収証)			0,1=2/2,3=2
//	2:預り証発行(払い戻し時)	0:釣銭なし, 1:釣銭あり, 2:釣銭なし(領収証), 3:釣銭あり(領収証)			6
//	3:減額失敗(電子mony)		---																		---
//	5:照会NG(ICC)				1=取扱い不可/2=無効カード/3=使用不可/4=期限切れ/5=読み取りNG/6=残高不足	1=11/2=19/3=10/4=20/5=21/6=22	6=カード残高
//	8:精算不可					---																		13
//	9:QR利用不可				0:重複，1:データ不正，2:その他異常										0=14/1,2=15(16,17,18)
// 10:割引き不可				0=精算処理区分NG, 1=割引種別NG, 2=枚数NG, 3=カード区分NG				0=16/1,3=17/2=18
void	lcdbm_notice_dsp2( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info )
{
	uchar	result_PKTcmd;

// MH810103(s) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更
	if(DispStatus == 198 ){
		DispStatus = 98;
	}
// MH810103(e) 電子マネー対応 WAON決済時の残高不足メッセージ内容を変更

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// 	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus, add_info );
	result_PKTcmd = PKTcmd_notice_dsp( (uchar)kind, DispStatus, add_info, NULL, 0 );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	if (result_PKTcmd == FALSE) {
		// error
	}
}
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//[]----------------------------------------------------------------------[]
///	@brief			ポップアップ表示要求(追加情報あり版)
//[]----------------------------------------------------------------------[]
///	@param[in]		DispCode：表示コード<br>
///					DispStatus：表示状態<br>
///					add_info:追加情報<br>
///					str:追加文字列<br>
///					str_size:追加情報
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	lcdbm_notice_dsp3( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info, uchar *str, ulong str_size )
{
	if ( DispStatus == 198 ) {
		DispStatus = 98;
	}

	PKTcmd_notice_dsp((uchar)kind, DispStatus, add_info, str, str_size);
}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
//[]----------------------------------------------------------------------[]
///	@brief			ポップアップ削除要求
//[]----------------------------------------------------------------------[]
///	@param[in]		kind：種類<br>
///					status：状態<br>
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2023/05/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	lcdbm_notice_del( uchar kind, uchar status )
{
	uchar	result_PKTcmd;

	if (OPECTL.Ope_mod == 3) {
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 		if ( ( IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1)) &&
// 		                     (Ope_isJPrinterReady() && (paperchk2() != -1))) ||	
// 		     (!IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1))) ) {	// インボイスではない場合はレシートのみ判定
		// インボイスではない場合はレシートのみ判定、電子領収証の場合は常にレシート出力可
		if ( ( IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1))) &&
		                                     (Ope_isJPrinterReady() && (paperchk2() != -1)) ) ||
		     (!IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1)))) ) {
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
			// レシート・ジャーナル印字可能であれば、印字成功と判断する
			result_PKTcmd = PKTcmd_notice_del( kind, status );
		}
		else {
			// レシート・ジャーナルのどちらか印字不可であれば、印字失敗と判断する
			// 印字失敗POPが出るので削除要求は送信しない
		}
	}

	if (result_PKTcmd == FALSE) {
		// error
	}
}
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）

//[]----------------------------------------------------------------------[]
///	@brief			精算残高変化通知
//[]----------------------------------------------------------------------[]
// GG124100(S) R.Endo 2022/09/29 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
// ///	@param[in]		pay_sts：精算ステータス(0:精算中,1:精算完了,2:精算済,3:精算不可)
///	@param[in]		pay_sts：精算ステータス(0:精算中,1:精算完了,2:精算済,3:精算不可,4:精算済(中止))
// GG124100(E) R.Endo 2022/09/29 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_pay_rem_chg( uchar pay_sts )
{
	lcdbm_cmd_pay_rem_chg_t lcdbm_cmd_pay_rem_chg;
	DATE_YMDHMS *paytime;

	memset( &lcdbm_cmd_pay_rem_chg, 0, sizeof(lcdbm_cmd_pay_rem_chg) );

	// ｺﾏﾝﾄﾞ共通情報_ｺﾏﾝﾄﾞ長
	lcdbm_cmd_pay_rem_chg.command.length = sizeof(lcdbm_cmd_pay_rem_chg_t) - sizeof(unsigned short);

	// ｺﾏﾝﾄﾞ共通情報_ｺﾏﾝﾄﾞID
	lcdbm_cmd_pay_rem_chg.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;

	// ｺﾏﾝﾄﾞ共通情報_ｻﾌﾞｺﾏﾝﾄﾞID
	lcdbm_cmd_pay_rem_chg.command.subid = LCDBM_CMD_SUBID_PAY_REM_CHG;

	// ID(入庫から精算完了までを管理するID)
	lcdbm_cmd_pay_rem_chg.id = op_GetSequenceID();

	// 精算ステータス
	//		0:精算中
	//		1:精算完了
	//		2:精算済
	//		3:精算不可
// GG124100(S) R.Endo 2022/09/29 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
// 	//      4:精算完了(みなし決済)
	//      4:精算済(中止)
// GG124100(E) R.Endo 2022/09/29 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
	lcdbm_cmd_pay_rem_chg.pay_sts = pay_sts;
// MH810100(S) K.Onodera 2020/03/03 #3989 入庫時刻指定の遠隔精算で入場日時が表示されない
	// 遠隔精算(入庫時刻指定)
	if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_TIME ){
		lcdbm_cmd_pay_rem_chg.pay_sts += 10;
	}
	// 遠隔精算(金額指定)
	else if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE ){
		lcdbm_cmd_pay_rem_chg.pay_sts += 20;
	}
// MH810100(E) K.Onodera 2020/03/03 #3989 入庫時刻指定の遠隔精算で入場日時が表示されない

	// 精算(入庫)年月日時分秒 ※遠隔精算(入庫時刻指定)では入庫時刻がセットされている
	paytime = GetPayTargetTime();
	memcpy( &lcdbm_cmd_pay_rem_chg.pay_time, paytime, sizeof(lcdbm_cmd_pay_rem_chg.pay_time) );

// MH810101(S) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
	// 定期チェック結果
	lcdbm_cmd_pay_rem_chg.season_chk_result = season_chk_result;
// MH810101(E) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129000(S) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
//	if ( OPECTL.op_faz == 1 ){
//	if ( OPECTL.op_faz == 1 || OPECTL.op_faz == 2){	// 入金中または精算完了
	if ( ope_MakeLaneLog_Check(pay_sts) ){	// 入金中または精算完了	
// GG129000(E) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
		// 入金中
		// レーンモニタデータ登録
		SetLaneFeeKind(ryo_buf.syubet+1);
		SetLaneFreeNum(ryo_buf.zankin);
		ope_MakeLaneLog(LM_PAY_MNY_IN);
	}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

	if( pay_sts < 2 ){
		// 駐車料金
// MH810100(S) K.Onodera 2020/02/25 #3911 定期車の場合に料金画面が表示されてしまう
//		lcdbm_cmd_pay_rem_chg.prk_fee = ryo_buf.tyu_ryo;
		if( ryo_buf.ryo_flg < 2 ){
			lcdbm_cmd_pay_rem_chg.prk_fee = ryo_buf.tyu_ryo;
		}else{
			lcdbm_cmd_pay_rem_chg.prk_fee = ryo_buf.tei_ryo;
		}
// MH810100(E) K.Onodera 2020/02/25 #3911 定期車の場合に料金画面が表示されてしまう

		// 精算残額
		lcdbm_cmd_pay_rem_chg.pay_rem = ryo_buf.zankin;

		// 現金投入額(前回精算による支払がある場合は加算)
// MH810103(s) 電子マネー対応
//		lcdbm_cmd_pay_rem_chg.cash_in = (ryo_buf.nyukin + ryo_buf.zenkai);
		// 現金投入合計額、ICカード／クレジットによる決済金額
		if(PayData.Electron_data.Suica.e_pay_kind == EC_CREDIT_USED){
			// クレジット時は、現金のみに。
			lcdbm_cmd_pay_rem_chg.cash_in = (ryo_buf.nyukin +  ryo_buf.zenkai);
		}else if(PayData.Electron_data.Suica.e_pay_kind != 0){
// MH810103(s) 電子マネー対応 #5465 再精算で電子マネーを使用した精算完了時、精算機画面上の引き去り金額に初回精算分も含んでしまう
//			lcdbm_cmd_pay_rem_chg.cash_in = (PayData.Electron_data.Suica.pay_ryo + ryo_buf.zenkai);
			lcdbm_cmd_pay_rem_chg.cash_in = ryo_buf.zenkai;		// 前回のみ
			lcdbm_cmd_pay_rem_chg.emoney_pay = PayData.Electron_data.Suica.pay_ryo;
// MH810103(e) 電子マネー対応 #5465 再精算で電子マネーを使用した精算完了時、精算機画面上の引き去り金額に初回精算分も含んでしまう
		}else{
			lcdbm_cmd_pay_rem_chg.cash_in = (ryo_buf.nyukin + ryo_buf.zenkai);
		}
// MH810103(e) 電子マネー対応

		// 釣銭額
		lcdbm_cmd_pay_rem_chg.chg_amt = ryo_buf.turisen;

		// 割引額
		// lcdbm_cmd_pay_rem_chg.disc_amt = ryo_buf_n.dis;
// MH810100(S) 2020/06/17 #4497【連動評価指摘事項】Ai-Ⅴより入庫時刻あり割引ありの遠隔精算を行うと、不正な（前回精算時に適応した）割引を再適応して精算してしまう
//// MH810100(S) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
////		lcdbm_cmd_pay_rem_chg.disc_amt = ryo_buf.waribik;
//		lcdbm_cmd_pay_rem_chg.disc_amt = ryo_buf.waribik - ryo_buf.zenkai;
//// MH810100(E) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
		if( ryo_buf.waribik > ryo_buf.zenkai){
			lcdbm_cmd_pay_rem_chg.disc_amt = ryo_buf.waribik - ryo_buf.zenkai;
		}else{
			lcdbm_cmd_pay_rem_chg.disc_amt = 0;
		}
// MH810100(E) 2020/06/17 #4497【連動評価指摘事項】Ai-Ⅴより入庫時刻あり割引ありの遠隔精算を行うと、不正な（前回精算時に適応した）割引を再適応して精算してしまう
// MH810103(s) 電子マネー対応
		lcdbm_cmd_pay_rem_chg.method = 0;	// 現金
		
		if(PayData.Electron_data.Suica.e_pay_kind == EC_CREDIT_USED){
			lcdbm_cmd_pay_rem_chg.method = 1; 												// クレジット精算
		}else if( PayData.Electron_data.Suica.e_pay_kind != 0 ){							// 電子決済種別
			if (e_zandaka >= 0) {
				lcdbm_cmd_pay_rem_chg.method = 2; 												// 電子マネー
			}else{
// MH810105(S) 2022/01/13 iD対応 ポストペイ型でみなし決済の表示になってしまう不具合修正
//				lcdbm_cmd_pay_rem_chg.method = 3; 												// 電子マネー(みなし決済)
				// iD or QUICPayで、みなしの場合は、フラグをみる。それ以外は、e_zandakaでチェック
				if( PayData.Electron_data.Suica.e_pay_kind == EC_ID_USED || 
				PayData.Electron_data.Suica.e_pay_kind == EC_QUIC_PAY_USED ){
				    if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin != 0 &&
					PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0) {
						lcdbm_cmd_pay_rem_chg.method = 3; 												// 電子マネー(みなし決済)
					}else{
						lcdbm_cmd_pay_rem_chg.method = 2; 												// 電子マネー
					}
				}else{
					lcdbm_cmd_pay_rem_chg.method = 3; 												// 電子マネー(みなし決済)
				}
// MH810105(E) 2022/01/13 iD対応 ポストペイ型でみなし決済の表示になってしまう不具合修正
			}
			lcdbm_cmd_pay_rem_chg.brand = PayData.Electron_data.Suica.e_pay_kind;			// ブランド
			lcdbm_cmd_pay_rem_chg.emoney_balance = PayData.Electron_data.Suica.pay_after; 	// 電子マネー残高
		}
// MH810103(e) 電子マネー対応

// GG129000(S) R.Endo 2024/01/12 #7217 お買物合計表示対応
		// 買物合計
		lcdbm_cmd_pay_rem_chg.Kaimono = ryo_buf.shopping_total;
// GG129000(E) R.Endo 2024/01/12 #7217 お買物合計表示対応
// GG129004(S) M.Fujikawa 2024/10/22 買物金額割引情報対応
		lcdbm_cmd_pay_rem_chg.Kaimono_info = ryo_buf.shopping_info;
// GG129004(E) M.Fujikawa 2024/10/22 買物金額割引情報対応
	}

	// lcdbm_cmd_pay_rem_chg_t型のI/F
	PKTcmd_pay_rem_chg( &lcdbm_cmd_pay_rem_chg );
}

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
//[]----------------------------------------------------------------------[]
///	@brief			入庫時刻指定遠隔精算開始
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void	lcdbm_remote_time_start( void )
{
	lcdbm_cmd_remote_time_start_t lcdbm_cmd_remote_time_start;

	// 初期化
	memset(&lcdbm_cmd_remote_time_start, 0, sizeof(lcdbm_cmd_remote_time_start_t));

	// 駐車場№
	lcdbm_cmd_remote_time_start.ulPno = g_PipCtrl.stRemoteTime.ulPno;

	// 料金種別
	lcdbm_cmd_remote_time_start.RyoSyu = g_PipCtrl.stRemoteTime.RyoSyu;

	// 入庫年月日時分秒
	lcdbm_cmd_remote_time_start.InTime.Year	 = g_PipCtrl.stRemoteTime.InTime.Year;
	lcdbm_cmd_remote_time_start.InTime.Mon	 = g_PipCtrl.stRemoteTime.InTime.Mon;
	lcdbm_cmd_remote_time_start.InTime.Day	 = g_PipCtrl.stRemoteTime.InTime.Day;
	lcdbm_cmd_remote_time_start.InTime.Hour	 = g_PipCtrl.stRemoteTime.InTime.Hour;
	lcdbm_cmd_remote_time_start.InTime.Min	 = g_PipCtrl.stRemoteTime.InTime.Min;
	lcdbm_cmd_remote_time_start.InTime.Sec	 = (uchar)g_PipCtrl.stRemoteTime.InTime.Sec;

	PKTcmd_remote_time_start(&lcdbm_cmd_remote_time_start);
}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

//[]----------------------------------------------------------------------[]
///	@brief			 RT精算データの基本情報作成
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Set_All_RTPay_Data( void )
{
	uchar 	loop_cnt;
// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//	stDiscount2_t* pDisc2;
//	stDiscount_t*  pDisc3;
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応

	// リアルタイム精算データ用バッファをクリアしておく
	memset( &RTPay_Data, 0, sizeof(RTPay_Data) );

// MH810100(S) K.Onodera 2020/03/05 #3912 未払精算の精算データが不正
	// 事前/未払種別
	RTPay_Data.shubetsu = lcdbm_rsp_in_car_info_main.shubetsu;
// MH810100(E) K.Onodera 2020/03/05 #3912 未払精算の精算データが不正

// MH810100(S) 2020/09/08 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
//	// フォーマットRev.No.
//	RTPay_Data.crd_info.FormatNo = 0;
	// ID
	RTPay_Data.ID = lcdbm_rsp_in_car_info_main.id;

	// カード情報Rev.No.
	RTPay_Data.crd_info_rev_no = lcdbm_rsp_in_car_info_main.crd_info_rev_no;

	// ﾌｫｰﾏｯﾄRev.No.
	RTPay_Data.crd_info.FormatNo = lcdbm_rsp_in_car_info_main.crd_info.FormatNo;
// MH810100(E) 2020/09/08 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）

// MH810100(S) 2020/07/27 #4557【検証課指摘事項】 リアルタイム精算データの処理日時が不正(Ａ×：39)
// // 年月日時分秒
// memcpy( &RTPay_Data.crd_info.dtTimeYtoSec,
// 		&lcdbm_rsp_in_car_info_main.crd_info.dtTimeYtoSec,
// 		sizeof(stDatetTimeYtoSec_t));
	// RTPay_LogRegist_AddOibanで処理年月日を入れる
// MH810100(E) 2020/07/27 #4557【検証課指摘事項】 リアルタイム精算データの処理日時が不正(Ａ×：39)

	// カード情報
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// 	RTPay_Data.crd_info.dtCardInfo.RevNoInfo	= 10;		// Rev.No.					10～65535
// GG129004(S) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
//	RTPay_Data.crd_info.dtCardInfo.RevNoInfo	= 11;		// Rev.No.					11～65535
	if((char)prm_get(COM_PRM,S_LCD, 49, 2, 1) != 11){
		RTPay_Data.crd_info.dtCardInfo.RevNoInfo	= 12;		// Rev.No.					11～65535
	}else{
		RTPay_Data.crd_info.dtCardInfo.RevNoInfo	= 11;		// Rev.No.					11～65535
	}
// GG129004(E) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)

	RTPay_Data.crd_info.dtCardInfo.MasterOnOff	= 1;		// ﾏｽﾀｰ情報  有無			0:無､1:有
// MH810100(S) K.Onodera 2020/02/12 #3852 リアルタイム精算データの「カード情報　予約情報」に「有」をセットしてしまっている
//	RTPay_Data.crd_info.dtCardInfo.ReserveOnOff	= 1;		// 予約情報  有無			0:無､1:有
	RTPay_Data.crd_info.dtCardInfo.ReserveOnOff	= 0;		// 予約情報  有無			0:無､1:有
// MH810100(E) K.Onodera 2020/02/12 #3852 リアルタイム精算データの「カード情報　予約情報」に「有」をセットしてしまっている
	RTPay_Data.crd_info.dtCardInfo.PassOnOff	= 1;		// 定期･会員情報  有無		0:無､1:有
	RTPay_Data.crd_info.dtCardInfo.ZaishaOnOff	= 1;		// 在車情報  有無			0:無､1:有

	RTPay_Data.crd_info.dtCardInfo.MasterSize	= sizeof(stMasterInfo_t);	// マスター情報サイズ
	RTPay_Data.crd_info.dtCardInfo.ReserveSize	= sizeof(stReserveInfo_t);	// 予約情報サイズ
	RTPay_Data.crd_info.dtCardInfo.PassSize		= sizeof(stPassInfo_t);		// 定期･会員情報サイズ
	RTPay_Data.crd_info.dtCardInfo.ZaishaSize	= sizeof(stZaishaInfo_t);	// 在車情報サイズ

	// ============================================== //
	//		マスター情報
	// ============================================== //

	// カード1～6(駐車場№, 種別, 番号)
	for( loop_cnt = 0; loop_cnt < ONL_MAX_CARDNUM; loop_cnt++ ){
		memcpy( &RTPay_Data.crd_info.dtMasterInfo.stCardDataInfo[loop_cnt],
				&lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[loop_cnt],
				sizeof(stParkKindNum_t) );
	}

	// 表示用カード1～6
	for( loop_cnt = 0; loop_cnt < ONL_MAX_CARDNUM; loop_cnt++) {
		memcpy( &RTPay_Data.crd_info.dtMasterInfo.stDispCardDataInfo[loop_cnt],
				&lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[loop_cnt],
				sizeof(stDispCardData_t) );
	}

	// 有効開始年月日
	memcpy( &RTPay_Data.crd_info.dtMasterInfo.StartDate,
			&lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.StartDate,
			sizeof(stDate_YYMD_t));

	// 有効終了年月日
	memcpy( &RTPay_Data.crd_info.dtMasterInfo.EndDate,
			&lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.EndDate,
			sizeof(stDate_YYMD_t));

	// 定期種別(0～15)
	RTPay_Data.crd_info.dtMasterInfo.SeasonKind = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.SeasonKind;

	// 会員種別(0～255)
	RTPay_Data.crd_info.dtMasterInfo.MemberKind = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.MemberKind;

	// 入出庫ステータス 0～99(0=初期状態(入出庫可),1=出庫中(入庫可),2=入庫中(出庫可))
	RTPay_Data.crd_info.dtMasterInfo.InOutStatus = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.InOutStatus;

	// 支払い手段
	RTPay_Data.crd_info.dtMasterInfo.PayMethod = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.PayMethod;

	// 入場確認フラグ
	RTPay_Data.crd_info.dtMasterInfo.EntryConf = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.EntryConf;

	// 予備(0固定)
	RTPay_Data.crd_info.dtMasterInfo.Reserve1 = lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.Reserve1;


	// ============================================== //
	//		定期情報
	// ============================================== //

	// 契約管理番号		1～16,000,000
	RTPay_Data.crd_info.dtPassInfo.ContractNo = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.ContractNo;

	// 販売ｽﾃｰﾀｽ		1=販売済み/9=販売前
	RTPay_Data.crd_info.dtPassInfo.SaleStatus = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.SaleStatus;

	// ｶｰﾄﾞｽﾃｰﾀｽ		0～99 (1=有効､2=無効)
	RTPay_Data.crd_info.dtPassInfo.CardStatus = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.CardStatus;

	// 定期区分			0～99
	RTPay_Data.crd_info.dtPassInfo.Classification = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.Classification;

	// 契約区画番号		0～26
	RTPay_Data.crd_info.dtPassInfo.DivisionNo = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.DivisionNo;

	// 契約車室番号		1～9999
	RTPay_Data.crd_info.dtPassInfo.CasingNo = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.CasingNo;

	// 契約ﾗｯｸ情報
	RTPay_Data.crd_info.dtPassInfo.RackInfo = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.RackInfo;

	// 4月跨ぎ更新許可	0～1(0=許可しない/1=許可する)
	RTPay_Data.crd_info.dtPassInfo.UpdatePermission = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.UpdatePermission;

	// ｵﾌﾟｼｮﾝ代金ﾌﾗｸﾞ①	各ﾋﾞｯﾄ対応
	RTPay_Data.crd_info.dtPassInfo.Option1 = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.Option1;

	// ｵﾌﾟｼｮﾝ代金ﾌﾗｸﾞ②	各ﾋﾞｯﾄ対応
	RTPay_Data.crd_info.dtPassInfo.Option2 = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.Option2;

	// 再発行回数		0～255
	RTPay_Data.crd_info.dtPassInfo.ReWriteCount = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.ReWriteCount;

	// 予備				0固定
	RTPay_Data.crd_info.dtPassInfo.Reserve1 = lcdbm_rsp_in_car_info_main.crd_info.dtPassInfo.Reserve1;


	// ============================================== //
	//		在車情報
	// ============================================== //

	// 在車中の駐車場№(0～999999)在車なしの場合は｢0｣
	RTPay_Data.crd_info.dtZaishaInfo.ResParkingLotNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.ResParkingLotNo;

	// 精算処理区分	0=未精算/1=精算/2=再精算/3=精算中止/4=再精算中止
	RTPay_Data.crd_info.dtZaishaInfo.PaymentType = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType;

	// 精算出庫		0=通常精算/1=強制出庫/(2=精算なし出庫)/3=不正出庫/
	// 				9=突破出庫10=ｹﾞｰﾄ開放/97=ﾛｯｸ開･ﾌﾗｯﾌﾟ上昇前未精算出庫/
	// 				98=ﾗｸﾞﾀｲﾑ内出庫/99=ｻｰﾋﾞｽﾀｲﾑ内出庫
	RTPay_Data.crd_info.dtZaishaInfo.ExitPayment = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.ExitPayment;

	// 現金売上額(0～999999)
	RTPay_Data.crd_info.dtZaishaInfo.GenkinFee = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.GenkinFee;

	// 在車情報_料金種別(0～99)
	RTPay_Data.crd_info.dtZaishaInfo.shFeeType = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType;

	// 入庫情報
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtEntryDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime,
			sizeof(stDateParkTime_t));

	// 精算情報
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime,
			sizeof(stDateParkTime_t));

	// 出庫情報
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtExitDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime,
			sizeof(stDateParkTime_t));

	// 入口通過情報
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtEntranceDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntranceDateTime,
			sizeof(stDateParkTime_t));

	// 出口通過情報
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtOutleteDateTime,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtOutleteDateTime,
			sizeof(stDateParkTime_t));

	// 駐車料金(0～999990(10円単位))
	RTPay_Data.crd_info.dtZaishaInfo.ParkingFee = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.ParkingFee;

	// 駐車時間(0～999999(分単位))
	RTPay_Data.crd_info.dtZaishaInfo.ParkingTime = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.ParkingTime;

	// 一時利用フラグ(0=定期利用,1=一時利用)
	RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.NormalFlag;

// GG129004(S) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
//	// 予備
//	RTPay_Data.crd_info.dtZaishaInfo.Reserve3 = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.Reserve3;
	// 入庫モード
	RTPay_Data.crd_info.dtZaishaInfo.InCarMode = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.InCarMode;
// GG129004(E) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応

	// 決算情報01～10
	for( loop_cnt = 0; loop_cnt < ONL_MAX_SETTLEMENT; loop_cnt++) {
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt],
				&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[loop_cnt],
				sizeof(stSettlement_t) );
	}

	// 暗証番号(発券機番号, 番号)
	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.stPassword,
			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stPassword,
			sizeof(stPasswordNo_t) );

// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//	// 割引01～25
//	/************************************************************************************/
//	/* 在車情報_割引01～25は割引の精算処理終了時(OnlineDiscount = ｵﾝﾗｲﾝ割引処理)で		*/
//	/* m_stDiscからｺﾋﾟｰするのでその割引精算後のﾃﾞｰﾀを使用する							*/
//	/************************************************************************************/
//	taskchg(IDLETSKNO);
//	for( loop_cnt = 0; loop_cnt < ONL_MAX_DISC_NUM; loop_cnt++) {
//		pDisc3 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt];
//		pDisc2 = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[loop_cnt];
//
//		pDisc3->DiscParkNo		= pDisc2->DiscParkNo;			// 割引 駐車場№(0～999999)
//		pDisc3->DiscSyu			= pDisc2->DiscSyu;				// 割引 種別(0～9999)
//		pDisc3->DiscCardNo		= pDisc2->DiscCardNo;			// 割引 ｶｰﾄﾞ区分(0～65000)
//		pDisc3->DiscNo			= pDisc2->DiscNo;				// 割引 区分(0～9)
//		pDisc3->DiscCount		= pDisc2->DiscCount;			// 割引 枚数(0～99)
//		pDisc3->DiscInfo		= pDisc2->DiscInfo;				// 割引 割引情報(0～65000)
//		pDisc3->DiscCorrType	= pDisc2->DiscCorrType;			// 割引 対応ｶｰﾄﾞ種別(0～65000)
//		pDisc3->DiscStatus		= pDisc2->DiscStatus;			// 割引 ｽﾃｰﾀｽ(0～9)
//		pDisc3->DiscFlg			= pDisc2->DiscFlg;				// 割引 割引状況(0～9)
//		// 時間割引？
//// MH810100(S) 2020/05/28 車番チケットレス(#4196)
////		if( pDisc2->DiscSyu >= NTNET_SVS_T ){
////			pDisc3->Discount		= pDisc2->DiscountT;		// 割引 時間(0～999999)
////		}else{
////			pDisc3->Discount		= pDisc2->DiscountM;		// 割引 金額(0～999999)
////		}
//// MH810100(S) 2020/07/29 車番チケットレス(#4561 センターから受信した割引情報を変更する)
//// pDisc3->Discount		= pDisc2->Discount;				// 金額/時間(0～999999)
//		pDisc3->Discount		= pDisc2->UsedDisc;				// 金額/時間(0～999999)
//// MH810100(E) 2020/07/29 車番チケットレス(#4561 センターから受信した割引情報を変更する)
////		pDisc3->UsedDisc		= pDisc2->UsedDisc;				// 買物割引利用時の使用済み割引（金額/時間）(0～999999)
//// MH810100(E) 2020/05/28 車番チケットレス(#4196)
//	}
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応

}

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//[]----------------------------------------------------------------------[]
///	@brief			RT領収証データの基本情報作成
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	Set_All_RTReceipt_Data( void )
{
	// RT領収証データ用バッファをクリアしておく
	memset(&RTReceipt_Data, 0, sizeof(RTReceipt_log));

	// 入庫から精算完了までを管理するID
	RTReceipt_Data.ID = lcdbm_rsp_in_car_info_main.id;

	// RTReceipt_LogRegist_AddOiban で センター追番 を入れる

	// RTReceipt_LogRegist_AddOiban で センター追番不正フラグ を入れる

	// フォーマットRev.No.
	RTReceipt_Data.receipt_info.FormatNo = 1;

	// 精算機 機種コード
	RTReceipt_Data.receipt_info.ModelCode = NTNET_MODEL_CODE;

	// RTReceipt_LogRegist_AddOiban で 処理 年月日時分秒 を入れる

	// Set_Pay_RTReceipt_Data で残りを入れる
}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

//[]----------------------------------------------------------------------[]
///	@brief			入庫情報のチェックと設定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void lcdbm_rsp_in_car_info_proc( void )
{
// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//	uchar 	loop_cnt, i, j;
	uchar 	i, j;
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
	stParkKindNum_t *pCard1 = NULL, *pCard2 = NULL;
	stParkKindNum_t stTmp;
// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//	stDiscount_t* pDisc1;
//	stDiscount2_t *pDisc2;
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
// GG129000(S) T.Nagai 2023/02/13 ゲート式車番チケットレスシステム対応（遠隔精算対応）
	uchar	carno[36];
// GG129000(E) T.Nagai 2023/02/13 ゲート式車番チケットレスシステム対応（遠隔精算対応）

	switch( OPECTL.Ope_mod ){
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
		// op_mod00
		case 0:
// GG129003(S) M.Fujikawa 2024/11/22 標準不具合修正　GM858400(S)[マージ] クラウド料金計算対応(改善連絡7、8:遠隔精算時の情報セット漏れ)
			op_mod01_Init();												// op_mod01() ｽｷｯﾌﾟのためｴﾘｱのみｸﾘｱ
			memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ

			memcpy( &lcdbm_rsp_in_car_info_main, &lcdbm_rsp_in_car_info_recv, sizeof(lcdbm_rsp_in_car_info_main) );

			// 精算開始時刻保持
			SetPayStartTimeInCarInfo();

			Ope_Set_tyudata_Card();
			Set_All_RTPay_Data();

			set_tim_only_out_card(0);

			// カード情報ソート
			taskchg(IDLETSKNO);
			for( i=0; i<ONL_MAX_CARDNUM; i++ ){
				pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
				for( j=i+1; j<ONL_MAX_CARDNUM; ++j ){
					pCard2 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[j];
					if( pCard2->CardType && (pCard1->CardType > pCard2->CardType) ){
						memcpy( &stTmp, pCard1, sizeof(stTmp) );
						memcpy( pCard1, pCard2, sizeof(stTmp) );
						memcpy( pCard2, &stTmp, sizeof(stTmp) );
					}
				}
			}
			
			// 表示カード番号セット
			taskchg(IDLETSKNO);
			for( i=0; i<ONL_MAX_CARDNUM; i++ ){
				pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
				if( pCard1->CardType ){
					memcpy( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[i].byDispCardNo, pCard1->byCardNo, sizeof(stDispCardData_t) );
					if (pCard1->CardType == CARD_TYPE_PARKING_TKT) {
						SetLaneMedia(pCard1->ParkingLotNo,
										pCard1->CardType,
										pCard1->byCardNo);
					}
				}
			}
			// 車番情報（UTF-8）（予備を抜いてコピーする）
			// 陸運支局名、分類番号
			memcpy(&carno[0], lcdbm_rsp_in_car_info_main.ShabanInfo.LandTransOfficeName, (size_t)(12+9));
			// 用途文字
			memcpy(&carno[(12+9)], lcdbm_rsp_in_car_info_main.ShabanInfo.UsageCharacter, (size_t)3);
			// 一連番号
			memcpy(&carno[(12+9+3)], lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber, (size_t)12);
			SetLaneFreeStr(carno, sizeof(carno));
			SetLaneFeeKind((ushort)lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.FeeType);
			SetLaneFreeNum(lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.ParkingFee);
			// 遠隔精算開始時
			ope_MakeLaneLog(LM_PAY_CHANGE);

			// 精算中変更データ受信時は割引媒体重複チェック用エリアをクリア
			memset( &g_UseMediaData, 0, sizeof( t_UseMedia ) );

			// ID(入庫から精算完了までを管理するID)のセット
			op_SetSequenceID( lcdbm_rsp_in_car_info_main.id );
			
			break;
// GG129003(E) M.Fujikawa 2024/11/22 標準不具合修正　GM858400(S)[マージ] クラウド料金計算対応(改善連絡7、8:遠隔精算時の情報セット漏れ)
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
		// op_mod01
		case 1:
			// 入庫情報の全コピー
			memcpy( &lcdbm_rsp_in_car_info_main, &lcdbm_rsp_in_car_info_recv, sizeof(lcdbm_rsp_in_car_info_main) );

			// カード情報ソート
			taskchg(IDLETSKNO);
			for( i=0; i<ONL_MAX_CARDNUM; i++ ){
				pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
				for( j=i+1; j<ONL_MAX_CARDNUM; ++j ){
					pCard2 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[j];
					if( pCard2->CardType && (pCard1->CardType > pCard2->CardType) ){
						memcpy( &stTmp, pCard1, sizeof(stTmp) );
						memcpy( pCard1, pCard2, sizeof(stTmp) );
						memcpy( pCard2, &stTmp, sizeof(stTmp) );
					}
				}
			}

			// 表示カード番号セット
			taskchg(IDLETSKNO);
			for( i=0; i<ONL_MAX_CARDNUM; i++ ){
				pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
				if( pCard1->CardType ){
					memcpy( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[i].byDispCardNo, pCard1->byCardNo, sizeof(stDispCardData_t) );
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					if (pCard1->CardType == CARD_TYPE_PARKING_TKT) {
						SetLaneMedia(pCard1->ParkingLotNo,
										pCard1->CardType,
										pCard1->byCardNo);
					}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				}else{
					memset( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[i], 0, sizeof(stDispCardData_t) );
				}
			}

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
			if ( OPECTL.remote_wait_flg == 0 ) {	// 車番チケットレス精算(入庫時刻指定)
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

			// 精算データの基本情報作成 = RTPay_Data の作成
			Set_All_RTPay_Data();

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
			// 領収証データの基本情報作成 = RTReceipt_Data の作成
			Set_All_RTReceipt_Data();
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

			// ID(入庫から精算完了までを管理するID)のセット
			op_SetSequenceID( lcdbm_rsp_in_car_info_main.id );

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// 			// QR利用時の割引データ比較用にバックアップ
// 			memcpy( DiscountBackUpArea, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo, sizeof(DiscountBackUpArea) );
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
			}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

			break;

		// op_mod02
		case 2:
			// IDのチェック(入庫～精算完了までの管理ID) 
			if( lcdbm_rsp_in_car_info_main.id == lcdbm_rsp_in_car_info_recv.id ){

				// 割引重複チェック結果
				lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup = lcdbm_rsp_in_car_info_recv.crd_info.ValidCHK_Dup;

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// 				// 料金計算結果コピー
// 				memcpy( &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo, &lcdbm_rsp_in_car_info_recv.crd_info.dtCalcInfo, sizeof( stPayResultInfo_t ) );
				if ( lcdbm_rsp_in_car_info_recv.crd_info.dtReqRslt.FeeCalcOnOff ) {			// 料金計算結果あり
					// 料金計算結果コピー
					memcpy(&lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo, &lcdbm_rsp_in_car_info_recv.crd_info.dtFeeCalcInfo, sizeof(stFeeCalcInfo_t));
				} else if ( lcdbm_rsp_in_car_info_recv.crd_info.dtReqRslt.PayResultInfo ) {	// 割引情報あり
					// 割引情報コピー
					memcpy(&lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo, &lcdbm_rsp_in_car_info_recv.crd_info.dtCalcInfo, sizeof(stPayResultInfo_t));
				}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// GG129000(S) H.Fujinaga 2023/01/26 ゲート式車番チケットレスシステム対応（遠隔精算対応）
				if (lcdbm_rsp_in_car_info_recv.shubetsu == 2) {
					// 精算中変更データ適用時

					lcdbm_rsp_in_car_info_main.shubetsu = lcdbm_rsp_in_car_info_recv.shubetsu;
					memcpy(&lcdbm_rsp_in_car_info_main.data.PayInfoChange,
							&lcdbm_rsp_in_car_info_recv.data.PayInfoChange,
							sizeof(lcdbm_rsp_in_car_info_main.data.PayInfoChange));
					if ( PayInfoChange_StateCheck() == 1 ){
						// 遠隔精算初回
						memcpy( &lcdbm_rsp_in_car_info_main, &lcdbm_rsp_in_car_info_recv, sizeof(lcdbm_rsp_in_car_info_main) );

						// 精算開始時刻保持
						SetPayStartTimeInCarInfo();

						Ope_Set_tyudata_Card();
						Set_All_RTPay_Data();
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
						Set_All_RTReceipt_Data();
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

						set_tim_only_out_card(0);

						taskchg(IDLETSKNO);
						for( i=0; i<ONL_MAX_CARDNUM; i++ ){
							pCard1 = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i];
							if( pCard1->CardType ){
								memcpy( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stDispCardDataInfo[i].byDispCardNo, pCard1->byCardNo, sizeof(stDispCardData_t) );
								if (pCard1->CardType == CARD_TYPE_PARKING_TKT) {
									SetLaneMedia(pCard1->ParkingLotNo,
													pCard1->CardType,
													pCard1->byCardNo);
								}
							}
						}
						// 車番情報（UTF-8）（予備を抜いてコピーする）
						// 陸運支局名、分類番号
						memcpy(&carno[0], lcdbm_rsp_in_car_info_main.ShabanInfo.LandTransOfficeName, (size_t)(12+9));
						// 用途文字
						memcpy(&carno[(12+9)], lcdbm_rsp_in_car_info_main.ShabanInfo.UsageCharacter, (size_t)3);
						// 一連番号
						memcpy(&carno[(12+9+3)], lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber, (size_t)12);
						SetLaneFreeStr(carno, sizeof(carno));
// GG129000(S) 改善連絡No.69,No.73 精算済みの在車を在車送信した時に精算中（事前精算：入金済み（割引含む））が（GM803002流用）
						SetLaneFeeKind((ushort)lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.FeeType);
						SetLaneFreeNum(lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.ParkingFee);
						// 遠隔精算開始時
						ope_MakeLaneLog(LM_PAY_CHANGE);
// GG129000(E) 改善連絡No.69,No.73 精算済みの在車を在車送信した時に精算中（事前精算：入金済み（割引含む））が（GM803002流用）

						// 精算中変更データ受信時は割引媒体重複チェック用エリアをクリア
						memset( &g_UseMediaData, 0, sizeof( t_UseMedia ) );
					}
				}
// GG129000(E) H.Fujinaga 2023/01/26 ゲート式車番チケットレスシステム対応（遠隔精算対応）

// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//				// 精算ﾃﾞｰﾀ(ｶｰﾄﾞ情報)の在車情報_割引の作成
//				taskchg(IDLETSKNO);
//				for( loop_cnt = 0; loop_cnt < ONL_MAX_DISC_NUM; loop_cnt++) {
//					pDisc1 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt];
//					pDisc2 = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[loop_cnt];
//					// 未割引の割引情報のみコピーする
//					if( pDisc1->DiscStatus ){	// 未割引でない
//						continue;
//					}
//
//					pDisc1->DiscParkNo		= pDisc2->DiscParkNo;			// 割引 駐車場№(0～999999)
//					pDisc1->DiscSyu			= pDisc2->DiscSyu;				// 割引 種別(0～9999)
//					pDisc1->DiscCardNo		= pDisc2->DiscCardNo;			// 割引 ｶｰﾄﾞ区分(0～65000)
//					pDisc1->DiscNo			= pDisc2->DiscNo;				// 割引 区分(0～9)
//					pDisc1->DiscCount		= pDisc2->DiscCount;			// 割引 枚数(0～99)
//					pDisc1->DiscInfo		= pDisc2->DiscInfo;				// 割引 割引情報(0～65000)
//					pDisc1->DiscCorrType	= pDisc2->DiscCorrType;			// 割引 対応ｶｰﾄﾞ種別(0～65000)
//					pDisc1->DiscStatus		= pDisc2->DiscStatus;			// 割引 ｽﾃｰﾀｽ(0～9)
//					pDisc1->DiscFlg			= pDisc2->DiscFlg;				// 割引 割引状況(0～9)
//// MH810100(S) 2020/05/28 車番チケットレス(#4196)
////					// 時間割引？
////					if( pDisc2->DiscSyu >= NTNET_SVS_T ){
////						pDisc1->Discount		= pDisc2->DiscountT;		// 割引 時間(0～999999)
////					}else{
////						pDisc1->Discount		= pDisc2->DiscountM;		// 割引 金額(0～999999)
////					}
//// MH810100(S) 2020/07/29 車番チケットレス(#4561 センターから受信した割引情報を変更する)
////					pDisc1->Discount		= pDisc2->Discount;				// 金額/時間(0～999999)
//					pDisc1->Discount		= pDisc2->UsedDisc;				// 金額/時間(0～999999)
//// MH810100(E) 2020/07/29 車番チケットレス(#4561 センターから受信した割引情報を変更する)
////					pDisc1->UsedDisc		= pDisc2->UsedDisc;				// 買物割引利用時の使用済み割引（金額/時間）(0～999999)
//// MH810100(E) 2020/05/28 車番チケットレス(#4196)
//				}
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
			}
			// ID不一致
			else {
				// 新しい入庫情報(ｶｰﾄﾞ情報)として扱わず現在の精算を優先するので以下はｺﾒﾝﾄｱｳﾄ
				// goto rsp_in_car_info_proc_NewData;
				// 精算中止要求の送信
			}
			break;

		default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			バーコード解析
//[]----------------------------------------------------------------------[]
///	@param			pPrefix   : prefix
///					pMedia	  : 割引媒体情報のポインタ
//[]----------------------------------------------------------------------[]
///	@return			TRUE : 解析成功
///					FALSE : 解析失敗
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
BOOL ope_ParseBarcodeData( tMediaDetail *pMedia )
{
	ushort	row_size = 0;
	lcdbm_rsp_QR_data_t *pQR = &lcdbm_rsp_QR_data_recv;

	// LCDで解析済みのため, 解析データをコピー
	pMedia->Barcode.id = pQR->id;					// QRコードID
	pMedia->Barcode.rev = pQR->rev;					// QRコードフォーマットRev.
	pMedia->Barcode.enc_type = pQR->enc_type;		// QRコードエンコードタイプ
	pMedia->Barcode.info_size = pQR->info_size;		// QRデータ情報(パースデータ)サイズ
	memcpy( &pMedia->Barcode.QR_data, &pQR->QR_data, sizeof(pMedia->Barcode.QR_data) );	// QRパースデータ
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	pMedia->Barcode.qr_type = pQR->qr_type;		// QRコードフォーマットタイプ
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）

	// 生データコピー
	row_size = pQR->data_size;
	if( row_size > BAR_DATASIZE ){
		row_size = BAR_DATASIZE;
	}
	memcpy( pMedia->RowData, pQR->data, (size_t)row_size );
	pMedia->RowSize = row_size;

	return TRUE;
}

//[]----------------------------------------------------------------------[]
///	@brief			レシート期限チェック
//[]----------------------------------------------------------------------[]
///	@param			pData : バーコード情報
//[]----------------------------------------------------------------------[]
///	@return			0=期限開始日より前，1=有効期限内，2=期限終了日より後
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
//static uchar ope_CheckReceiptLimit( tBarcodeDetail* pData )
static uchar ope_CheckReceiptLimit( ushort id, QR_YMDData* pYmdData )
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
{
	ushort	startDay, endDay;			// 開始日/終了日(normalize)
	ushort	validDay;
	ushort	BoundaryTime = 0;			// 日付切換時間(normalize)
	ushort	BarMin = 0;
	uchar	ret = 1;
// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
//	QR_AmountInfo* pAmount = NULL;
//	QR_DiscountInfo* pDisc = NULL;
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
	struct clk_rec *pTime;

	// 現在時刻の代わりに精算開始時刻を使用する
	pTime = GetPayStartTime();

	// QR買上券
// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
//	if( pData->id == BAR_ID_AMOUNT ){
//		pAmount = &pData->QR_data.AmountType;
//
//		startDay = dnrmlzm( pAmount->IssueDate.Year, pAmount->IssueDate.Mon, pAmount->IssueDate.Day );	// 有効開始日
	if( id == BAR_ID_AMOUNT ){
		startDay = dnrmlzm( pYmdData->IssueDate.Year, pYmdData->IssueDate.Mon, pYmdData->IssueDate.Day );	// 有効開始日
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
		validDay = (ushort)prm_get(COM_PRM, S_SYS, 77, 2, 1);		// 有効期間
		if( validDay == 0 ){
			validDay = 99;
		}
		endDay = startDay + validDay - 1;																// 有効終了日

		// 日付切換時刻の算出
		BoundaryTime = (ushort)prm_tim( COM_PRM, S_SYS, 76 );

// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
//		BarMin = (pAmount->IssueDate.Hour * 60) + pAmount->IssueDate.Min;
		BarMin = (pYmdData->IssueDate.Hour * 60) + pYmdData->IssueDate.Min;
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
		// バーコード発行が日付切換え前？
		if( BarMin < BoundaryTime ){
			// 券挿入が日付切換え前？
			if( pTime->nmin < BoundaryTime ){
				// 日付切換え前のレシートは、日付切換え前は使える
				;
			}
			// 日付切換え後？
			else{
				// 日付切換え前のレシートは、日付切換え後は使えない(有効期限１日の場合)
				--startDay;
				--endDay;
			}
		}
		// バーコード発行が日付切換え後？
		else{
			// 券挿入が日付切換え前？
			if( pTime->nmin < BoundaryTime ){
				++startDay;						// 開始日と終了日を＋１日して範囲チェックを行う
				++endDay;
			}
		}
	}
	// QR割引券
	else{
// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
//		pDisc = &pData->QR_data.DiscountType;
//
//		startDay = dnrmlzm( pDisc->StartDate.Year, pDisc->StartDate.Mon, pDisc->StartDate.Day );		// 有効開始日
//		endDay   = dnrmlzm( pDisc->EndDate.Year, pDisc->EndDate.Mon, pDisc->EndDate.Day );				// 有効終了日
		if( pYmdData->StartDate.Year == 0 && pYmdData->StartDate.Mon == 0 && pYmdData->StartDate.Day == 0 &&
			pYmdData->EndDate.Year == 99 && pYmdData->EndDate.Mon == 99 && pYmdData->EndDate.Day == 99 ){	// 無期限？

			// 有効期限なし（無期限）
			startDay = 0;
			endDay	 = 0xffff;
		}
		else{
			startDay = dnrmlzm( pYmdData->StartDate.Year, pYmdData->StartDate.Mon, pYmdData->StartDate.Day );	// 有効開始日
			endDay   = dnrmlzm( pYmdData->EndDate.Year, pYmdData->EndDate.Mon, pYmdData->EndDate.Day );			// 有効終了日
		}
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
	}

	// 範囲ﾁｪｯｸ
	if( pTime->ndat < startDay ) {		// 期限前（対象日＜開始日）
		ret = 0;
	}
	else if( endDay < pTime->ndat ) {	// 期限後（終了日＜対象日）
		ret = 2;
	}

	return ret;
}
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）

//[]----------------------------------------------------------------------[]
///	@brief			レシート発行日チェック
//[]----------------------------------------------------------------------[]
///	@param			pMedia : 割引媒体情報
//[]----------------------------------------------------------------------[]
///	@return			TRUE : 入庫時刻より後
///					FALSE : 入庫時刻より前
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
static BOOL ope_CheckReceiptIssueDate( tBarcodeDetail* pData )
{
	ulong	BarTime;					// レシート発行日(normalize)
	ulong	InTime;						// 入庫時刻(normalize)
	BOOL	bRet = FALSE;
	QR_AmountInfo* pAmount = &pData->QR_data.AmountType;
	QR_DiscountInfo* pDisc = &pData->QR_data.DiscountType;

	// 月日時分チェック
// MH810100(S) K.Onodera 2020/03/18 車番チケットレス(#4062 01-0078=⑥0(入庫時刻チェックする(月日時分))が効かない)
//	if( prm_get(COM_PRM, S_SYS, 78, 1, 1) ){
	if( prm_get(COM_PRM, S_SYS, 78, 1, 1) == 0 ){
// MH810100(E) K.Onodera 2020/03/18 車番チケットレス(#4062 01-0078=⑥0(入庫時刻チェックする(月日時分))が効かない)
		// QR買上券
		if( pData->id == BAR_ID_AMOUNT ){
			// レシート発行日
			BarTime = dnrmlzm( pAmount->IssueDate.Year, pAmount->IssueDate.Mon, pAmount->IssueDate.Day );
			BarTime = (BarTime * T_DAY) + tnrmlz( 0, 0, pAmount->IssueDate.Hour, pAmount->IssueDate.Min );
		}
		// QR割引券
		else{
			// レシート発行日
			BarTime = dnrmlzm( pDisc->IssueDate.Year, pDisc->IssueDate.Mon, pDisc->IssueDate.Day );
			BarTime = (BarTime * T_DAY) + tnrmlz( 0, 0, pDisc->IssueDate.Hour, pDisc->IssueDate.Min );
		}

		// 入庫時刻
		InTime = dnrmlzm( car_in_f.year, car_in_f.mon, car_in_f.day );
		InTime = (InTime * T_DAY) + tnrmlz(0, 0, car_in_f.hour, car_in_f.min);
	}
	// 日付のみチェック
	else{
		// QR買上券
		if( pData->id == BAR_ID_AMOUNT ){
			BarTime = dnrmlzm( pAmount->IssueDate.Year, pAmount->IssueDate.Mon, pAmount->IssueDate.Day );
		}
		// QR割引券
		else{
			BarTime = dnrmlzm( pDisc->IssueDate.Year, pDisc->IssueDate.Mon, pDisc->IssueDate.Day );
		}

		// 入庫年月日
		InTime = dnrmlzm( car_in_f.year, car_in_f.mon, car_in_f.day );
	}

	// 入庫後に発行したレシート？
	if( BarTime >= InTime ){
		bRet = TRUE;
	}

	return bRet;
}

// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
/*[]----------------------------------------------------------------------[]*/
/*| 各種割引券としてのチェック                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ope_CanUseBarcode_sub( tBarcodeDetail* Barcode )        |*/
/*| PARAMETER    : tBarcodeDetail* Barcode : QRﾃﾞｰﾀ格納先頭ｱﾄﾞﾚｽ           |*/
/*|              : QR_YMDData* pYmdData : QR日付構造体先頭ｱﾄﾞﾚｽ            |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = 駐車場№ｴﾗｰ                                  |*/
/*|                       2 = ﾃﾞｰﾀ異常                                     |*/
/*|                       3 = 期限切れ                                     |*/
/*|                       6 = 期限前                                       |*/
/*|                      13 = 種別規定外                                   |*/
/*|                      14 = 限度枚数ｵｰﾊﾞｰ                                |*/
/*|                      25 = 車種ｴﾗｰ                                      |*/
/*|                      26 = 設定ｴﾗｰ                                      |*/
/*|                      27 = 料金種別ｴﾗｰ                                  |*/
/*|                      28 = 限度枚数0ｴﾗｰ                                 |*/
/*|                      30 = 精算順序ｴﾗｰ                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2020-05-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]*/
static short ope_CanUseBarcode_sub( tBarcodeDetail* Barcode, QR_YMDData* pYmdData )
{
	short	ret = 0;
	short	cardknd;
//	ushort	pkno_syu;
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	ushort	pkno_syu;
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	uchar	chk_end1;		// 券ﾃﾞｰﾀ一致ﾌﾗｸﾞ	（OFF=不一致／ON=一致）
	uchar	chk_end2;		// 券種別不一致ﾌﾗｸﾞ	（OFF=一致／ON=不一致）
	uchar	tbl_no;			// 期限変更ﾃｰﾌﾞﾙNo.
	short	tbl_syu;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：種別
	short	tbl_data;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：内容
	short	tbl_syear;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：開始年ﾃﾞｰﾀ
	short	tbl_smon;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：開始月ﾃﾞｰﾀ
	short	tbl_sday;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：開始日ﾃﾞｰﾀ
	short	tbl_eyear;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：終了年ﾃﾞｰﾀ
	short	tbl_emon;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：終了月ﾃﾞｰﾀ
	short	tbl_eday;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：終了日ﾃﾞｰﾀ
	ushort	tbl_sdate;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：開始年月日（dnrmlzm変換ﾃﾞｰﾀ）
	ushort	tbl_edate;		// 期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ：終了年月日（dnrmlzm変換ﾃﾞｰﾀ）
	short	data_adr;		// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ
	char	data_pos;		// 使用可能料金種別のﾃﾞｰﾀ位置
	ushort	std;			// Pass Start Day (BIN) 
	ushort	end;			// Pass End Day (BIN) 
	struct clk_rec		*pTime;	// 精算開始時刻
	QR_DiscountInfo*	pDisc;	// QRデータ情報 QR割引券
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
	short	role;			// 役割
	short	typeswitch;		// 種別切換
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)

	// 現在時刻の代わりに精算開始時刻を使用する
	pTime = GetPayStartTime();

	// QR買上券
	if( Barcode->id == BAR_ID_AMOUNT ){
		ret = 0;
	}
	// QR割引券
	else{
		pDisc = &Barcode->QR_data.DiscountType;		// QRデータ情報 QR割引券の先頭ポインタ

		if( chkdate( pDisc->StartDate.Year,
					(short)pDisc->StartDate.Mon,
					(short)pDisc->StartDate.Day ) ){	// 有効開始日ﾁｪｯｸNG?
			return( 2 );					// ﾃﾞｰﾀ異常
		}
		std = dnrmlzm( pDisc->StartDate.Year,
					  (short)pDisc->StartDate.Mon,
					  (short)pDisc->StartDate.Day );	// 有効開始日ｾｯﾄ

		if( chkdate( pDisc->EndDate.Year,
					(short)pDisc->EndDate.Mon,
					(short)pDisc->EndDate.Day ) ){		// 有効開始日ﾁｪｯｸNG?
			return( 2 );					// ﾃﾞｰﾀ異常
		}
		end = dnrmlzm( pDisc->EndDate.Year,
					  (short)pDisc->EndDate.Mon,
					  (short)pDisc->EndDate.Day );		// 有効開始日ｾｯﾄ

		ret = 0;
		for( ; ; ){
// GG124100(S) R.Endo 2022/08/03 車番チケットレス3.0 #6343 クラウド料金計算対応
			if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
				switch ( pDisc->DiscKind ) {	// 割引種別
				case 1:		// サービス券（金額）
				case 101:	// サービス券（時間）
					cardknd = 11;	// ｻｰﾋﾞｽ券
					break;
				case 2:		// 店割引（金額）
				case 102:	// 店割引（時間）
				case 3:		// 多店舗割引（金額）
				case 103:	// 多店舗割引（時間）
					cardknd = 12;	// 掛売券
					break;
				default:
					cardknd = 0;
					break;
				}
			} else {					// 通常料金計算モード
// GG124100(E) R.Endo 2022/08/03 車番チケットレス3.0 #6343 クラウド料金計算対応

			switch( pDisc->DiscKind ){			// 割引種別
			case 1:		// サービス券（金額）
			case 101:	// サービス券（時間）
				cardknd = 11;											// ｻｰﾋﾞｽ券
				break;
			case 2:		// 店割引（金額）
			case 102:	// 店割引（時間）
				cardknd = 12;											// 掛売券
				break;
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
// 			case 4:		// 割引券（金額）
// 			case 104:	// 割引券（時間）
// 				cardknd = 14;											// 割引券
// 				break;
			case 3:		// 多店舗割引（金額）
			case 103:	// 多店舗割引（時間）
				if ( prm_get(COM_PRM, S_TAT, 1, 1, 1) == 1 ) {			// 多店舗割引あり
					cardknd = 12;										// 掛売券
				} else {												// 多店舗割引なし
					ret = 13;											// 種別規定外
				}
				break;
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
			default:
				ret = 13;												// 種別規定外
				break;
			}
			if( ret ){
				break;													// ｴﾗｰあり
			}

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応
			}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応

			if( 0L == pDisc->ParkingNo ){
				ret = 1;												// 駐車場№ｴﾗｰ
				break;
			}

			if(( prm_get( COM_PRM, S_SYS, 71, 1, 6 ) == 1 )&&			// 基本サービス券使用可
			   ( CPrmSS[S_SYS][1] == pDisc->ParkingNo )){				// 基本駐車場№?
//				pkno_syu = KIHON_PKNO;									// 基本
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
				pkno_syu = KIHON_PKNO;									// 基本
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			}
			else if(( prm_get( COM_PRM, S_SYS, 71, 1, 1 ) == 1 )&&		// 拡張1サービス券使用可
					( CPrmSS[S_SYS][2] == pDisc->ParkingNo )){			// 拡張1駐車場№?
//				pkno_syu = KAKUCHOU_1;									// 拡張1
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
				pkno_syu = KAKUCHOU_1;									// 拡張1
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			}
			else if(( prm_get( COM_PRM, S_SYS, 71, 1, 2 ) == 1 )&&		// 拡張2サービス券使用可
					( CPrmSS[S_SYS][3] == pDisc->ParkingNo )){			// 拡張2駐車場№?
//				pkno_syu = KAKUCHOU_2;									// 拡張2
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
				pkno_syu = KAKUCHOU_2;									// 拡張2
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			}
			else if(( prm_get( COM_PRM, S_SYS, 71, 1, 3 ) == 1 )&&		// 拡張3サービス券使用可
					( CPrmSS[S_SYS][4] == pDisc->ParkingNo )){			// 拡張3駐車場№?
//				pkno_syu = KAKUCHOU_3;									// 拡張3
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
				pkno_syu = KAKUCHOU_3;									// 拡張3
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			}
			else{
				ret = 1;												// 駐車場№ｴﾗｰ
				break;
			}

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
			if ( !CLOUD_CALC_MODE ) {	// 通常料金計算モード
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

//			if( prm_get( COM_PRM, S_DIS, 5, 1, 1 ) ){
//				if( CardSyuCntChk( pkno_syu, cardknd,
//					(short)pDisc->DiscClass, (short)pDisc->ShopNp, 0 ) ){	// 1精算の割引種類の件数ｵｰﾊﾞｰ?
//					ret = 14;											// 限度枚数ｵｰﾊﾞｰ
//					break;
//				}
//			}
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			if ( prm_get(COM_PRM, S_DIS, 5, 1, 1) ) {	// 同一割引券種数の制限(08-0005)が精算ログの制限を越えた割引を受け付けない
				if ( CardSyuCntChk(pkno_syu, cardknd, (short)pDisc->DiscClass, (short)pDisc->ShopNp, 0) ) {	// 同一割引券種数オーバー
					ret = 14;	// 限度枚数ｵｰﾊﾞｰ
					break;
				}
			}
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
			}
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

			// 券期限変更処理
			chk_end1 = OFF;												// 券ﾃﾞｰﾀ一致ﾌﾗｸﾞ：OFF

			for( tbl_no = 1 ; tbl_no <= 3 ; tbl_no++ ){					// 期限変更ﾃｰﾌﾞﾙ（１～３）検索

				chk_end2 = OFF;											// 種別不一致ﾌﾗｸﾞ：OFF

				tbl_syu = (short)CPrmSS[S_DIS][8+((tbl_no-1)*6)];		// 期限変更ﾃｰﾌﾞﾙに設定されている種別を取得
				tbl_data= (short)CPrmSS[S_DIS][9+((tbl_no-1)*6)];		// 期限変更ﾃｰﾌﾞﾙに設定されている内容を取得

				// 券種別ﾁｪｯｸ
				switch( tbl_syu ){										// 設定されている種別と券ﾃﾞｰﾀの種別を比較
				case	1:												// 設定種別＝サービス券
					if( cardknd != 11 ){								// 券ﾃﾞｰﾀ種別＝サービス券？
						chk_end2 = ON;									// NO → 種別不一致
					}
					else{
						if( tbl_data != 0 ){							// 設定内容＝０（サービス券全て）以外？
							if( tbl_data != (short)pDisc->DiscClass ){	// サービス券種別(A～C)一致？
								chk_end2 = ON;							// NO → 種別不一致
							}
						}
					}
					break;
				case	2:												// 設定種別＝掛売券
					if( cardknd != 12 ){								// 券ﾃﾞｰﾀ種別＝掛売券？
						chk_end2 = ON;									// NO → 種別不一致
					}
					else{
						if( tbl_data != 0 ){							// 設定内容＝０（掛売券全て）以外？
							if( tbl_data != (short)pDisc->ShopNp ){		// 店No.一致？
								chk_end2 = ON;							// NO → 種別不一致
							}
						}
					}
					break;
				case	3:												// 設定種別＝割引券
					if( cardknd != 14 ){								// 券ﾃﾞｰﾀ種別＝割引券？
						chk_end2 = ON;									// NO → 種別不一致
					}
					else{
						if( tbl_data != 0 ){							// 設定内容＝０（割引券全て）以外？
							if( tbl_data != (short)pDisc->ShopNp ){		// 店No.一致？
								chk_end2 = ON;							// NO → 種別不一致
							}
						}
					}
					break;
				case	4:												// 設定種別＝全て
					break;												// サービス券、掛売券、割引券全て一致と判断
				case	0:												// 設定種別＝なし
				default:												// 設定種別＝その他
					chk_end2 = ON;										// → 種別不一致
					break;
				}
				if( chk_end2 == ON ){									// 設定されている種別と券ﾃﾞｰﾀの種別不一致？
					continue;											// YES → 判定中の期限変更ﾃｰﾌﾞﾙ検索終了
				}

				// 有効期限ﾁｪｯｸ
				tbl_syear = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 5 );	// 変更前の開始（年）取得
				tbl_smon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 3 );	// 変更前の開始（月）取得
				tbl_sday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(10+((tbl_no-1)*6)), 2, 1 );	// 変更前の開始（日）取得

				tbl_eyear = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 5 );	// 変更前の終了（年）取得
				tbl_emon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 3 );	// 変更前の終了（月）取得
				tbl_eday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(11+((tbl_no-1)*6)), 2, 1 );	// 変更前の終了（日）取得

				if( tbl_syear >= 80 ){		// 開始年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
					tbl_syear += 1900;
				}else{
					tbl_syear += 2000;
				}
				if( tbl_eyear >= 80 ){		// 終了年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
					tbl_eyear += 1900;
				}else{
					tbl_eyear += 2000;
				}
				tbl_sdate = dnrmlzm( tbl_syear, tbl_smon, tbl_sday );	// 有効開始日変換
				tbl_edate = dnrmlzm( tbl_eyear, tbl_emon, tbl_eday );	// 有効終了日変換

				if( (std == tbl_sdate) && (end == tbl_edate) ){			// 開始日＆終了日一致？
					chk_end1 = ON;										// YES → 券ﾃﾞｰﾀ一致
				}

				if( chk_end1 == ON ){									// 券ﾃﾞｰﾀ一致？

					// 券ﾃﾞｰﾀ（種別＆有効期限）と期限変更ﾃｰﾌﾞﾙﾃﾞｰﾀ（種別＆変更前有効期限）が一致した場合
					tbl_syear = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 5 );	// 変更後の開始（年）取得
					tbl_smon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 3 );	// 変更後の開始（月）取得
					tbl_sday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(12+((tbl_no-1)*6)), 2, 1 );	// 変更後の開始（日）取得

					tbl_eyear = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 5 );	// 変更後の終了（年）取得
					tbl_emon  = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 3 );	// 変更後の終了（月）取得
					tbl_eday  = (uchar)prm_get( COM_PRM, S_DIS, (short)(13+((tbl_no-1)*6)), 2, 1 );	// 変更後の終了（日）取得

					if(	(tbl_syear == 0) && (tbl_smon == 0) && (tbl_sday == 0) &&
						(tbl_eyear == 99) && (tbl_emon == 99) && (tbl_eday == 99) ){	// 変更後有効期限？

						// 有効期限なし（無期限）
						tbl_sdate = 0;
						tbl_edate = 0xffff;
					}
					else{
						if( tbl_eyear >= 80 ){
							// 年が80以上入力されていたら調整する。
							tbl_eyear = 79; tbl_emon = 12; tbl_eday = 31;
		 				}
						// 有効期限あり
						if( tbl_syear >= 80 ){		// 開始年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
							tbl_syear += 1900;
						}else{
							tbl_syear += 2000;
						}
						if( tbl_eyear >= 80 ){		// 終了年ﾃﾞｰﾀ変換(西暦下２桁→西暦４桁)
							tbl_eyear += 1900;
						}else{
							tbl_eyear += 2000;
						}

						tbl_sdate = dnrmlzm( tbl_syear, tbl_smon, tbl_sday );	// 有効開始日変換
						tbl_edate = dnrmlzm( tbl_eyear, tbl_emon, tbl_eday );	// 有効終了日変換
					}
					std = tbl_sdate;											// 有効開始日を変更後の開始日とする
					end = tbl_edate;											// 有効終了日を変更後の終了日とする

					pYmdData->StartDate.Year = tbl_syear;						// QR有効開始年
					pYmdData->StartDate.Mon  = tbl_smon;						// QR有効開始月
					pYmdData->StartDate.Day  = tbl_sday;						// QR有効開始日

					pYmdData->EndDate.Year = tbl_eyear;							// QR有効終了年
					pYmdData->EndDate.Mon  = tbl_emon;							// QR有効終了月
					pYmdData->EndDate.Day  = tbl_eday;							// QR有効終了日

					break;														// → 期限変更ﾃｰﾌﾞﾙ（１～３）検索終了
				}
			}

			if( std > pTime->ndat ){											// 期限前
				ret = 6;														// 期限前ｴﾗｰ
				break;
			}
			if( end < pTime->ndat ){											// 期限切れ
				ret = 3;														// 期限切れｴﾗｰ
				break;
			}

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応
			if ( !CLOUD_CALC_MODE ) {	// 通常料金計算モード
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応

			if( cardknd == 12 ){
				/*** 掛売券 ***/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
// 				if( !rangechk( 1, 100, (short)pDisc->ShopNp ) ) {				// 店№範囲外?
// 					ret = 13;													// 種別規定外
// 					break;
// 				}
// 				if (( CPrmSS[S_STO][1+3*((short)pDisc->ShopNp-1)] == 0L )&&
// 					( CPrmSS[S_STO][3+3*((short)pDisc->ShopNp-1)] == 0L ))
// 				{
				// 種別規定外(13)と料金種別ｴﾗｰ(27)のチェック
				ret = CheckDiscount(pDisc->DiscKind, pDisc->ShopNp, pDisc->DiscClass);
				if ( ret ) {
					break;
				}

				// 役割未設定(26)のチェック
				if ( (pDisc->DiscKind == NTNET_TKAK_M) || 						// 多店舗割引（金額）
					 (pDisc->DiscKind == NTNET_TKAK_T) ) {						// 多店舗割引（時間）
					role = (short)prm_get(COM_PRM, S_TAT, (pDisc->DiscClass * 2), 1, 1);
				} else {														// 店割引
					role = (short)prm_get(COM_PRM, S_STO, ((pDisc->ShopNp * 3) - 2), 1, 1);
				}
				typeswitch = GetTypeSwitch(pDisc->DiscKind, pDisc->ShopNp);
				if ( !role && !typeswitch ) {									// 役割/種別切替なし
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
					ret = 26;													// 役割未設定
					break;
				}
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
// 				if( CPrmSS[S_STO][3+3*((short)pDisc->ShopNp-1)] ){				// 種別切替有り?
// 					if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_STO][3+3*((short)pDisc->ShopNp-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
// 						ret = 27;												// 料金種別ｴﾗｰ
// 						break;
// 					}
// // MH810100(S) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
// // // MH810100(S) 2020/06/19 #4503【連動評価指摘事項】サービス券QR適用後の種別切替の認証取消データが送信されてこない
// // 					if(( ryo_buf.nyukin )||( ryo_buf.waribik )||			// 入金済み? or 割引済み?
// // 					   ( e_incnt > 0 )||									// or 電子マネー使用あり?（現時点では不要）
// // 					   ( c_pay )) {											// or ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用あり?
// // 						ret = 30;											// 精算順序ｴﾗｰ
// // 						break;
// // 					}
// // // MH810100(E) 2020/06/19 #4503【連動評価指摘事項】サービス券QR適用後の種別切替の認証取消データが送信されてこない
// // MH810100(E) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
// 				}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
//					if( CPrmSS[S_STO][1+3*((short)pDisc->ShopNp-1)] == 1L ){	// 時間割引?
//						if( card_use[USE_PPC] || card_use[USE_NUM] ||			// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ or 回数券使用済み?
//							(e_incnt > 0))										// or 電子マネー使用あり?（現時点では不要）
//						{
//							ret = 30;											// 精算順序ｴﾗｰ
//							break;
//						}
//					}
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
// 			}else if( cardknd == 14 ){
// 				/*** 割引券 ***/
// 				if(( CPrmSS[S_WAR][1] == 0 )||									// 割引券使用しない設定?
// 				   ( !rangechk( 1, 100, (short)pDisc->DiscClass ) )||			// 割引種別範囲外?
// 				   (( CPrmSS[S_WAR][2+3*((short)pDisc->DiscClass-1)] == 0L )&&	// 未設定
// 				    ( CPrmSS[S_WAR][4+3*((short)pDisc->DiscClass-1)] == 0L )))
// 				{
// 					ret = 13;													// 種別規定外
// 					break;
// 				}
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
			}else{
				/*** ｻｰﾋﾞｽ券 ***/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
// 				if( !rangechk( 1, SVS_MAX, (short)pDisc->DiscClass ) ){			// ｻｰﾋﾞｽ券範囲外?
// 					ret = 13;													// 種別規定外
// 					break;
// 				}
// 				if (( CPrmSS[S_SER][1+3*((short)pDisc->DiscClass-1)] == 0L )&&
// 					( CPrmSS[S_SER][3+3*((short)pDisc->DiscClass-1)] == 0L ))
// 				{
				// 種別規定外(13)と料金種別ｴﾗｰ(27)のチェック
// MH810104(S) R.Endo 2021/09/22 車番チケットレス フェーズ2.3 #6005 【連動試験】QR割引券のサービス券で店番号が「0」の場合に利用できない
// 				ret = CheckDiscount(pDisc->DiscKind, pDisc->ShopNp, pDisc->DiscClass);
				// サービス券は割引区分がカード区分、店番号が割引情報に入る。
				ret = CheckDiscount(pDisc->DiscKind, pDisc->DiscClass, pDisc->ShopNp);
// MH810104(E) R.Endo 2021/09/22 車番チケットレス フェーズ2.3 #6005 【連動試験】QR割引券のサービス券で店番号が「0」の場合に利用できない
				if ( ret ) {
					break;
				}

				// 役割未設定(26)のチェック
				role = (short)prm_get(COM_PRM, S_SER, ((pDisc->DiscClass * 3) - 2), 1, 1);
// MH810104(S) R.Endo 2021/10/06 車番チケットレス フェーズ2.3 #6067 【連動試験】サービス券割引（種別切換）のQR割引券が利用できない
// 				typeswitch = GetTypeSwitch(pDisc->DiscKind, pDisc->ShopNp);
				typeswitch = GetTypeSwitch(pDisc->DiscKind, pDisc->DiscClass);
// MH810104(E) R.Endo 2021/10/06 車番チケットレス フェーズ2.3 #6067 【連動試験】サービス券割引（種別切換）のQR割引券が利用できない
				if ( !role && !typeswitch ) {									// 役割/種別切替なし
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
					ret = 26;													// 役割未設定
					break;
				}
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
// 				if( CPrmSS[S_SER][3+3*((short)pDisc->DiscClass-1)] ){			// 種別切替有り?
// 					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(CPrmSS[S_SER][3+3*((short)pDisc->DiscClass-1)]-1)),2,5 ) == 0L ) {	// 車種設定なし?
// 						ret = 27;												// 種別切替なし
// 						break;
// 					}
// // MH810100(S) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
// // // MH810100(S) 2020/06/19 #4503【連動評価指摘事項】サービス券QR適用後の種別切替の認証取消データが送信されてこない
// // 					if(( ryo_buf.nyukin )||( ryo_buf.waribik )||				// 入金済み? or 割引済み?
// // 					   ( e_incnt > 0 )||										// or 電子マネー使用あり?（現時点では不要）
// // 					   ( c_pay )) {												// or ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用あり?
// // 						ret = 30;												// 精算順序ｴﾗｰ
// // 						break;
// // 					}
// // // MH810100(E) 2020/06/19 #4503【連動評価指摘事項】サービス券QR適用後の種別切替の認証取消データが送信されてこない
// // MH810100(E) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
// 				}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(各種割引券チェックの変更)
//				if( CPrmSS[S_SER][1+3*((short)pDisc->DiscClass-1)] == 1L ){		// 時間割引?
//					if( card_use[USE_PPC] || card_use[USE_NUM] ||				// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ or 回数券使用済み?
//						(e_incnt > 0))											// or 電子マネー使用あり?（現時点では不要）
//					{
//						ret = 30;												// 精算順序ｴﾗｰ
//						break;
//					}
//				}
				if( ryo_buf.syubet < 6 ){
					// 精算対象の料金種別A～F(0～5)
					data_adr = 2*((short)pDisc->DiscClass-1)+76;				// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
					data_pos = (char)(6-ryo_buf.syubet);						// 使用可能料金種別のﾃﾞｰﾀ位置取得
				}
				else{
					// 精算対象の料金種別G～L(6～11)
					data_adr = 2*((short)pDisc->DiscClass-1)+77;				// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
					data_pos = (char)(12-ryo_buf.syubet);						// 使用可能料金種別のﾃﾞｰﾀ位置取得
				}
				if( prm_get( COM_PRM, S_SER, data_adr, 1, data_pos ) ){			// 使用不可設定？
					ret = 25;													// 他の車種の券
					break;
				}
			}

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応
			}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応

			break;
		}
	}

	return( ret );
}
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）

//[]----------------------------------------------------------------------[]
///	@brief			バーコード可能チェック
//[]----------------------------------------------------------------------[]
///	@return			RESULT_NO_ERROR : 使用可能なバーコード
///					RESULT_QR_INQUIRYING：問合せ中
///					RESULT_DISCOUNT_TIME_MAX : 割引上限
///					RESULT_BAR_USED : 使用済のバーコード
//					RESULT_BAR_EXPIRED: 期限切れバーコード
///					RESULT_BAR_READ_MAX : バーコード枚数上限
///					RESULT_BAR_FORMAT_ERR：フォーマットエラー
///					RESULT_BAR_ID_ERR：対象外
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
static OPE_RESULT ope_CanUseBarcode( tMediaDetail *pMedia )
{
	ushort	index = 0;
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// 	ulong	alm_info = 0;
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
	QR_AmountInfo* pAmount = NULL;
	QR_DiscountInfo* pDisc = NULL;
	QR_YMDData	YmdData;
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）

	// ICクレジット設定あり
// MH810103(s) 電子マネー対応
//	if (prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1 ) {
	if(isEC_USE()) {
// MH810103(e) 電子マネー対応
		// ICクレジット問合せ中
		if( OPECTL.InquiryFlg == 1 ){
			return RESULT_QR_INQUIRYING;		// 問合せ中
		}
	}

	// 1認証操作中の使用済バーコードチェック
	for( index = 0; index < g_UseMediaData.bar_count; ++index ){
		WACDOG;
		if( 0 == memcmp(pMedia->RowData, &g_UseMediaData.data[index].RowData, sizeof(pMedia->RowData)) ){
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// 			alm_info = 1;	// 1認証中に2回同じバーコードを読取りする
// 			alm_chk2( ALMMDL_AUTH, ALARM_USED_RECEIPT, 2, 2, 1, (void *)&alm_info );	// アラーム登録(A0530)
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
			return RESULT_BAR_USED;				// 使用済のバーコード
		}
	}

	// レシート枚数チェック
// MH810103(S) R.Endo 2021/05/31 車番チケットレス フェーズ2.2 #5669 QR買上券レシート読込み枚数上限の判定不具合
//	if( 0 != prm_get(COM_PRM, S_SYS, 75, 2, 1) && g_UseMediaData.bar_count >= prm_get(COM_PRM, S_SYS, 75, 2, 1) || g_UseMediaData.bar_count >= BARCODE_USE_MAX ){
	if ( (g_UseMediaData.bar_count >= BARCODE_USE_MAX) || (
		 (pMedia->Barcode.id == BAR_ID_AMOUNT) &&
		 (0 != prm_get(COM_PRM, S_SYS, 75, 2, 1)) &&
		 (g_UseMediaData.amount_count >= prm_get(COM_PRM, S_SYS, 75, 2, 1))) ) {
// MH810103(E) R.Endo 2021/05/31 車番チケットレス フェーズ2.2 #5669 QR買上券レシート読込み枚数上限の判定不具合
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// 		alm_info = 6;		// 固定
// 		alm_chk2( ALMMDL_AUTH, ALARM_USE_COUNT_OVER, 2, 2, 1, (void *)&alm_info );		// アラーム登録(A0533)
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
		return RESULT_BAR_READ_MAX;				// 既に上限
	}

	// 非対応バーコード
	if( pMedia->Barcode.id != BAR_ID_AMOUNT && pMedia->Barcode.id != BAR_ID_DISCOUNT ){
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// 		alm_info = 1;		// フォーマットパターンに一致しないバーコードを読取りする（
// 		alm_chk2( ALMMDL_AUTH, ALARM_NOT_USE_RECEIPT, 2, 2, 1, (void *)&alm_info );		// アラーム登録(A0531)
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
		return RESULT_BAR_ID_ERR;
	}

// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
	// 各種割引券としてのチェック
	if( pMedia->Barcode.id == BAR_ID_AMOUNT ){
		// QR買上券
		pAmount = &pMedia->Barcode.QR_data.AmountType;
		memcpy( &YmdData.IssueDate, &pAmount->IssueDate, sizeof(DATE_YMDHMS) );	// QR発行年月日時分秒
		memset( &YmdData.StartDate, 0, sizeof(DATE_YMD) );						// QR有効開始年月日
		memset( &YmdData.EndDate  , 0, sizeof(DATE_YMD) );						// QR有効終了年月日
	}else{
		// QR割引券
		pDisc = &pMedia->Barcode.QR_data.DiscountType;
		memset( &YmdData.IssueDate, 0, sizeof(DATE_YMDHMS) );					// QR発行年月日時分秒
		memcpy( &YmdData.StartDate, &pDisc->StartDate, sizeof(DATE_YMD) );		// QR有効開始年月日
		memcpy( &YmdData.EndDate  , &pDisc->EndDate  , sizeof(DATE_YMD) );		// QR有効終了年月日
	}
	switch( ope_CanUseBarcode_sub( &pMedia->Barcode, &YmdData ) ){
	case  0:	// OK
	default:
		break;
	case  1:	// 駐車場№ｴﾗｰ
	case  2:	// ﾃﾞｰﾀ異常
	case 13:	// 種別規定外
	case 25:	// 車種ｴﾗｰ
	case 26:	// 設定ｴﾗｰ
	case 27:	// 料金種別ｴﾗｰ
		return RESULT_BAR_ID_ERR;				// 対象外
		break;
	case  3:	// 期限切れ
	case  6:	// 期限前
//		return RESULT_BAR_EXPIRED;				// 有効期限外
		break;
//	case 14:	// 限度枚数ｵｰﾊﾞｰ
//	case 28:	// 限度枚数0ｴﾗｰ
//		return RESULT_DISCOUNT_TIME_MAX;		// 割引上限
//		break;
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	case 14:	// 限度枚数ｵｰﾊﾞｰ
		return RESULT_DISCOUNT_TIME_MAX;		// 割引上限
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(S) 2020/06/19 #4503【連動評価指摘事項】サービス券QR適用後の種別切替の認証取消データが送信されてこない
	case 30:	// 精算順序ｴﾗｰ
		return RESULT_BAR_NOT_USE_CHANGEKIND;
		break;
// MH810100(E) 2020/06/19 #4503【連動評価指摘事項】サービス券QR適用後の種別切替の認証取消データが送信されてこない
	}
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
	if ( !CLOUD_CALC_MODE ) {	// 通常料金計算モード
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

	// 期限切れチェック
// MH810100(S) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
//	if( 1 != ope_CheckReceiptLimit(&pMedia->Barcode) ){
	if( 1 != ope_CheckReceiptLimit( pMedia->Barcode.id, &YmdData ) ){
// MH810100(E) m.saito 2020/05/21 車番チケットレス（#4178 割引券の期限変更テーブル設定が反映されない）
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// 		alm_info = 1;		// 有効期限切れのバーコードを読取りする
// 		alm_chk2( ALMMDL_AUTH, ALARM_EXPIRED_RECEIPT, 2, 2, 1, (void *)&alm_info );		// アラーム登録(A0532)
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
		return RESULT_BAR_EXPIRED;				// 有効期限外
	}

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
	}
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
	// 発行日チェック
	if( 0 == prm_get(COM_PRM, S_SYS, 78, 1, 1) || 2 == prm_get(COM_PRM, S_SYS, 78, 1, 1) ){ 	// 入庫時刻チェックする
		if( FALSE == ope_CheckReceiptIssueDate(&pMedia->Barcode) ){
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// 			alm_info = 2;	// 入庫時刻よりも前に発行されたバーコードを読取りする
// 			alm_chk2( ALMMDL_AUTH, ALARM_EXPIRED_RECEIPT, 2, 2, 1, (void *)&alm_info );	// アラーム登録(A0532)
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
			return RESULT_BAR_EXPIRED;			// 有効期限外
		}
	}

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
	if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
		// QRデータチェック
		return cal_cloud_qrdata_check(pMedia, &YmdData);
	}
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

	return RESULT_NO_ERROR;
}

// GG129000(S) H.Fujinaga 2023/02/15 ゲート式車番チケットレスシステム対応（QR読取画面対応）
//[]----------------------------------------------------------------------[]
///	@brief			QR駐車券可能チェック
//[]----------------------------------------------------------------------[]
///	@return			0 : 使用可能
///					1 : 使用不可
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
static ushort ope_CanUseQRTicket( tMediaDetail *pMedia )
{
	long prm;
	ulong QrTime_all;
	ulong NowTime_all;
	struct clk_rec	QrTime;					// 発行時刻(normalize)
	struct clk_rec	NowTime;				// 現在時刻(normalize)
	QR_TicketInfo* pTicket = &pMedia->Barcode.QR_data.TicketType;

	// 駐車場No = 基本駐車場No？
	prm = CPrmSS[S_SYS][1];
	if(pTicket->ParkingNo != prm){
		return 1;
	}

	// 入庫時刻 <= 現在日時？
	// 発券時刻(入庫時刻)
	QrTime.year = pTicket->IssueDate.Year;
	QrTime.mont = (short)pTicket->IssueDate.Mon;
	QrTime.date = (short)pTicket->IssueDate.Day;
	QrTime.hour = (short)pTicket->IssueDate.Hour;
	QrTime.minu = (short)pTicket->IssueDate.Min;
	QrTime.seco = (short)pTicket->IssueDate.Sec;
	QrTime.week	= 0;		// Day   (0:SUN-6:SAT)
	QrTime.ndat	= 0;		// Normalize Date
	QrTime.nmin	= 0;		// Normalize Minute
	QrTime_all = c_Normalize_sec(&QrTime);

	// 現在時刻
	NowTime.year = CLK_REC.year;
	NowTime.mont = (short)CLK_REC.mont;
	NowTime.date = (short)CLK_REC.date;
	NowTime.hour = (short)CLK_REC.hour;
	NowTime.minu = (short)CLK_REC.minu;
	NowTime.seco = (short)CLK_REC.seco;
	NowTime.week = 0;		// Day   (0:SUN-6:SAT)
	NowTime.ndat = 0;		// Normalize Date
	NowTime.nmin = 0;		// Normalize Minute
	NowTime_all = c_Normalize_sec(&NowTime);

	if(QrTime_all > NowTime_all){
		return 1;
	}

	// 入庫時刻が現在日時の1年前？
	// 発券時刻(入庫時刻+1年)
	QrTime.year = (short)(pTicket->IssueDate.Year + 1);
	QrTime.mont = (short)pTicket->IssueDate.Mon;
	QrTime.date = (short)pTicket->IssueDate.Day;
	QrTime.hour = (short)pTicket->IssueDate.Hour;
	QrTime.minu = (short)pTicket->IssueDate.Min;
	QrTime.seco = (short)pTicket->IssueDate.Sec;
	QrTime.week	= 0;		// Day   (0:SUN-6:SAT)
	QrTime.ndat	= 0;		// Normalize Date
	QrTime.nmin	= 0;		// Normalize Minute
	QrTime_all = c_Normalize_sec(&QrTime);

	if(QrTime_all <= NowTime_all){
		return 1;
	}

	return 0;
}
// GG129000(E) H.Fujinaga 2023/02/15 ゲート式車番チケットレスシステム対応（QR読取画面対応）

//[]----------------------------------------------------------------------[]
///	@brief			使用済割引媒体登録
//[]----------------------------------------------------------------------[]
///	@return			TRUE : 登録成功
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
BOOL ope_AddUseMedia( tMediaDetail* pMedia )
{
	// 使用済割引媒体登録
	memcpy( &g_UseMediaData.data[g_UseMediaData.bar_count], pMedia, sizeof(g_UseMediaData.data[g_UseMediaData.bar_count]) );
	++g_UseMediaData.bar_count;

// MH810103(S) R.Endo 2021/05/31 車番チケットレス フェーズ2.2 #5669 QR買上券レシート読込み枚数上限の判定不具合
	if( pMedia->Barcode.id == BAR_ID_AMOUNT ) {
		++g_UseMediaData.amount_count;
	}
// MH810103(E) R.Endo 2021/05/31 車番チケットレス フェーズ2.2 #5669 QR買上券レシート読込み枚数上限の判定不具合

	return TRUE;
}

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//[]----------------------------------------------------------------------[]
///	@brief			レーンモニタデータ登録
//[]----------------------------------------------------------------------[]
///	@param			usStsNo	：状態種別・コード
//[]----------------------------------------------------------------------[]
///	@return			TRUE : 登録成功
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
BOOL ope_MakeLaneLog( ushort usStsNo )
{
	memset( &DC_LANE_work, 0, sizeof(DC_LANE_work) );

	// 入庫から精算完了までを管理するID
	DC_LANE_work.ID = op_GetSequenceID();

	// センター追番
	DC_LANE_work.CenterOiban = DC_GetCenterSeqNo( DC_SEQNO_LANE );

	// センター追番不正フラグ
	if( DC_LANE_work.CenterOiban == 0 ){
		DC_UpdateCenterSeqNo( DC_SEQNO_LANE );
		DC_LANE_work.CenterOiban = 1;
		DC_LANE_work.CenterOibanFusei = 1;
	}

	// レーンモニタ情報セット ---------------------------------------------------------------
	// フォーマットRev.№
	DC_LANE_work.LANE_Info.FormRev = 1;

	// 発生時刻
// GG129000(S) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(不要なキャストを修正)（GM803000流用）
//	memcpy( &DC_LANE_work.LANE_Info.ProcessDate, (date_time_rec2*)&CLK_REC, sizeof(DC_LANE_work.LANE_Info.ProcessDate) );
	memcpy( &DC_LANE_work.LANE_Info.ProcessDate, &CLK_REC, sizeof(DC_LANE_work.LANE_Info.ProcessDate) );
// GG129000(E) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(不要なキャストを修正)（GM803000流用）

	// 状態一連番号
	DC_LANE_work.LANE_Info.StsSeqNo = LaneStsSeqNo;

	// 媒体 駐車場№、種別、番号、料金種別
	memcpy(&DC_LANE_work.LANE_Info.MediaParkNo, &m_stLaneWork, (size_t)offsetof(struct stLaneDataInfo, FreeNum));

	// 状態種別
	DC_LANE_work.LANE_Info.StsSyu = (uchar)(usStsNo / 100);

	// 状態コード
	DC_LANE_work.LANE_Info.StsCode = (uchar)(usStsNo % 100);

	// フリー数値
	DC_LANE_work.LANE_Info.FreeNum = m_stLaneWork.FreeNum;

	// フリー文字
	memcpy(DC_LANE_work.LANE_Info.FreeStr, m_stLaneWork.FreeStr, sizeof(m_stLaneWork.FreeStr));

	// 状態名、状態メッセージは精算機ではセットしない

	Log_regist( LOG_DC_LANE );				// ログ登録
	DC_UpdateCenterSeqNo( DC_SEQNO_LANE );	// センター追番をカウントアップ
// GG129000(S) M.Fujikawa 2023/11/21 ゲート式車番チケットレスシステム対応　不具合#7211対応
	DC_PopCenterSeqNo( DC_SEQNO_LANE );	// センター追番をPop
// GG129000(S) M.Fujikawa 2023/11/21 ゲート式車番チケットレスシステム対応　不具合#7211対応

	return TRUE;
}
//[]----------------------------------------------------------------------[]
///	@brief			レーンモニタデータの媒体セット
//[]----------------------------------------------------------------------[]
/// @param		ulParkNo	：駐車場No.
/// 			usMediaKind	：媒体種別
/// 			pMediaNo	：媒体番号（32桁）
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void SetLaneMedia(ulong ulParkNo, ushort usMediaKind, uchar *pMediaNo)
{
	// 駐車場No.
	m_stLaneWork.MediaParkNo = ulParkNo;

	// 媒体種別
	m_stLaneWork.Mediasyu = usMediaKind;

	// 媒体番号
	memset(m_stLaneWork.MediaNo, 0, sizeof(m_stLaneWork.MediaNo));
	if (pMediaNo != NULL) {
		memcpy(m_stLaneWork.MediaNo, pMediaNo, sizeof(m_stLaneWork.MediaNo));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			レーンモニタデータの料金種別セット
//[]----------------------------------------------------------------------[]
/// @param		usFeeKind	:料金種別
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void SetLaneFeeKind(ushort usFeeKind)
{
	// 料金種別
	m_stLaneWork.PaySyu = usFeeKind;
}

//[]----------------------------------------------------------------------[]
///	@brief			レーンモニタデータのフリー数値セット
//[]----------------------------------------------------------------------[]
/// @param		ulNum	:フリー数値
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void SetLaneFreeNum(ulong ulNum)
{
	// フリー数値
	m_stLaneWork.FreeNum = ulNum;
}

//[]----------------------------------------------------------------------[]
///	@brief			レーンモニタデータのフリー文字セット
//[]----------------------------------------------------------------------[]
/// @param		pStr		:フリー文字（60byte）
/// 			usStrSize	:フリー文字サイズ
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void SetLaneFreeStr(uchar *pStr, uchar usStrSize)
{
	// フリー文字
	memset(m_stLaneWork.FreeStr, 0, sizeof(m_stLaneWork.FreeStr));
	if (pStr != NULL) {
		memcpy(m_stLaneWork.FreeStr, pStr, (size_t)usStrSize);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			状態一連番号更新
//[]----------------------------------------------------------------------[]
/// @param		none
//[]----------------------------------------------------------------------[]
///	@return		none
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void LaneStsSeqNoUpdate(void)
{
// GG129000(S) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
	if(OPECTL.f_req_paystop == 1){
//		OPECTL.f_req_paystop = 0;
		;	// 何もしない
	}else{
// GG129000(E) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
	// 状態一連番号インクリメント
	LaneStsSeqNo++;
	if( LaneStsSeqNo > 999999999L ){
		LaneStsSeqNo = 0;
	}
	memset( &m_stLaneWork, 0, sizeof(m_stLaneWork) );
// GG129000(S) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
	}
// GG129000(E) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
// GG129000(S) M.Fujikawa 2023/11/21 ゲート式車番チケットレスシステム対応　不具合#7211対応
	DC_PushCenterSeqNo( DC_SEQNO_LANE );	// センター追番をPush
// GG129000(S) M.Fujikawa 2023/11/21 ゲート式車番チケットレスシステム対応　不具合#7211対応
}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129000(S) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
//[]----------------------------------------------------------------------[]
///	@brief			レーンモニタ登録判定(精算中(入金済み))
//[]----------------------------------------------------------------------[]
/// @param		status		:精算ステータス
//[]----------------------------------------------------------------------[]
///	@return		TRUE		:登録する
///				FALSE		:登録しない
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
BOOL ope_MakeLaneLog_Check( uchar pay_sts )
{
	if( pay_sts == 0 ){
		// 精算中
		if( OPECTL.op_faz == 1 && !cal_cloud_discount_check_only() ){
			// 1:入金中(入金有り時にｾｯﾄ) かつ 入庫情報正常
			return TRUE;
		}
	}else if( pay_sts == 1 ){
		// 精算完了
		if( OPECTL.op_faz == 2 && OPECTL.Ope_mod == 3 ){
			// 2:精算完了(入金不可送信後ｾｯﾄ) かつ mod03
			return TRUE;
		}
	}
	return FALSE;
}
// GG129000(E) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）

//[]----------------------------------------------------------------------[]
///	@brief			認証ログ作成
//[]----------------------------------------------------------------------[]
///	@param			pTicketData : 駐車券情報
///					pMedia : 割引媒体情報
///					flg : 最終フラグ
///					kind : 処理区分(0=確定、1=取消)
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void ope_MakeCertifLog( tMediaDetail *pMedia, uchar flg, eCertifKind kind )
{
	ushort cnt = 0, disc_cnt = 0;
	stDiscount_t* pDisc = NULL;

	memset( &DC_QR_work, 0, sizeof(DC_QR_work) );

	DC_QR_work.ID = op_GetSequenceID();							// 入庫から精算完了までを管理するID
	DC_QR_work.CenterOiban = DC_GetCenterSeqNo( DC_SEQNO_QR );	// センター追番
	if( DC_QR_work.CenterOiban == 0 ){
		DC_UpdateCenterSeqNo( DC_SEQNO_QR );
		DC_QR_work.CenterOiban = 1;
		DC_QR_work.CenterOibanFusei = 1;						// センター追番不正フラグ
	}
	DC_QR_work.QR_Info_Rev = 1;									// QR情報Rev.№

	// QR情報セット ---------------------------------------------------------------
	DC_QR_work.QR_Info.FormRev = 1;								// フォーマットRev.№
	memcpy( &DC_QR_work.QR_Info.CertDate, &g_UseMediaData.Certif_tim, sizeof(DC_QR_work.QR_Info.CertDate) );	// 認証確定時刻
	DC_QR_work.QR_Info.ProcessKind = kind;						// 処理区分(0=確定、1=取消)
	// 入庫媒体
// MH810100(S) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）
//// MH810100(S) 2020/06/12 #4227 【連動評価指摘事項】DC-NET（DataID：1101　割引認証登録・取消要求データ）のカード情報はリアルタイム通信の精算媒体と同じにしてほしい
////	DC_QR_work.QR_Info.in_media.ParkingNo = lcdbm_rsp_in_car_info_main.crd_info.AskMediaParkNo;		// 問合せ媒体 駐車場№		0～999999
////	DC_QR_work.QR_Info.in_media.Kind = lcdbm_rsp_in_car_info_main.crd_info.AskMediaKind;			// 問合せ媒体 種別			0～65000
////	memcpy( DC_QR_work.QR_Info.in_media.CardNo, lcdbm_rsp_in_car_info_main.crd_info.byAskMediaNo, sizeof(DC_QR_work.QR_Info.in_media.CardNo) );
////																									// 問合せ媒体 番号			【ASCII】32桁（左詰、残りはNull固定）※本来種別101なら機械№+券№だが、それが入る？
//
//	// 問合せ応答の割引情報取得結果の「精算媒体」をセットする
//	DC_QR_work.QR_Info.in_media.ParkingNo = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;		// 問合せ媒体 駐車場№		0～999999
//	DC_QR_work.QR_Info.in_media.Kind = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;			// 問合せ媒体 種別			0～65000
//	memcpy( DC_QR_work.QR_Info.in_media.CardNo, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo, sizeof(DC_QR_work.QR_Info.in_media.CardNo) );
//																									// 問合せ媒体 番号			【ASCII】32桁（左詰、残りはNull固定）※本来種別101なら機械№+券№だが、それが入る？
//// MH810100(E) 2020/06/12 #4227 【連動評価指摘事項】DC-NET（DataID：1101　割引認証登録・取消要求データ）のカード情報はリアルタイム通信の精算媒体と同じにしてほしい
//
//// MH810100(S) K.Onodera  2020/02/27 車番チケットレス(#3946 QRコードで割引き後、精算完了やとりけしで初期画面に戻るタイミングで再起動してしまう)
//	DC_QR_work.QR_Info.Passkind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.SeasonKind;		// 定期種別
//	DC_QR_work.QR_Info.MemberKind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.MemberKind;	// 会員種別
//
//	// 入庫駐車場№
//	DC_QR_work.QR_Info.InParkNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.ParkingNo;
//// MH810100(E) K.Onodera  2020/02/27 車番チケットレス(#3946 QRコードで割引き後、精算完了やとりけしで初期画面に戻るタイミングで再起動してしまう)
//	// 入庫時間(年月日時分秒)
//	memcpy( &DC_QR_work.QR_Info.InDate, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec, sizeof(DC_QR_work.QR_Info.InDate) );

	// 問合せ応答の割引情報取得結果の「精算媒体」をセットする
	DC_QR_work.QR_Info.in_media.ParkingNo = g_UseMediaData.QueryResult.Query_ParkingNo;		// 問合せ媒体 駐車場№		0～999999
	DC_QR_work.QR_Info.in_media.Kind = g_UseMediaData.QueryResult.Query_Kind;			// 問合せ媒体 種別			0～65000
	memcpy( DC_QR_work.QR_Info.in_media.CardNo, g_UseMediaData.QueryResult.Query_CardNo, sizeof(DC_QR_work.QR_Info.in_media.CardNo) );
																									// 問合せ媒体 番号			【ASCII】32桁（左詰、残りはNull固定）※本来種別101なら機械№+券№だが、それが入る？

	DC_QR_work.QR_Info.Passkind	= g_UseMediaData.QueryResult.Passkind;		// 定期種別
	DC_QR_work.QR_Info.MemberKind	= g_UseMediaData.QueryResult.MemberKind;	// 会員種別
	// 入庫駐車場№
	DC_QR_work.QR_Info.InParkNo = g_UseMediaData.QueryResult.InParkNo;
	// 入庫時間(年月日時分秒)
	memcpy( &DC_QR_work.QR_Info.InDate, &g_UseMediaData.QueryResult.InDate, sizeof(DC_QR_work.QR_Info.InDate) );
//// MH810100(E) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）

	DC_QR_work.QR_Info.ReciptCnt = g_UseMediaData.bar_count;							// レシート枚数
	DC_QR_work.QR_Info.Method = 0;														// 割引方法(0=オンラインデータ、1=オフラインデータ、2=AR-100／150 )
	DC_QR_work.QR_Info.Lastflg = flg;													// 最終フラグ(0=認証開始データ、1=認証途中データ、2=認証最終データ)

// MH810100(S) K.Onodera  2020/03/10 車番チケットレス(#3971 割引認証登録・取消要求データにQRコードIDを設定していない)
//	DC_QR_work.QR_Info.DiscountInfoSize = (sizeof(QR_Discount) * ONL_MAX_DISC_NUM) ;	// 割引認証情報データサイズ
// MH810100(E) K.Onodera  2020/03/10 車番チケットレス(#3971 割引認証登録・取消要求データにQRコードIDを設定していない)

	// 割引認証情報
// MH810100(S) m.saito 2020/05/14 車番チケットレス(取消時に割引認証情報をセットしてしまう)
	if( kind == 0 ){	// 処理区分(0=確定、1=取消)
// MH810100(E) m.saito 2020/05/14 車番チケットレス(取消時に割引認証情報をセットしてしまう)
		for( cnt=0; cnt<ONL_MAX_DISC_NUM; cnt++ ){
			pDisc = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt];
			// 割引情報あり？
			if( pDisc->DiscSyu || pDisc->DiscParkNo ){
				disc_cnt++;
				// 情報セット
				DC_QR_work.QR_Info.DiscountInfo[cnt].ParkingNo = pDisc->DiscParkNo;		// 駐車場№
				DC_QR_work.QR_Info.DiscountInfo[cnt].Kind = pDisc->DiscSyu;				// 種別
				DC_QR_work.QR_Info.DiscountInfo[cnt].CardType = pDisc->DiscCardNo;		// カード区分
				DC_QR_work.QR_Info.DiscountInfo[cnt].Type = pDisc->DiscNo;				// 区分
				DC_QR_work.QR_Info.DiscountInfo[cnt].DiscInfo = pDisc->DiscInfo;		// 割引情報
			}
		}
// MH810100(S) m.saito 2020/05/14 車番チケットレス(取消時に割引認証情報をセットしてしまう)
	}
// MH810100(E) m.saito 2020/05/14 車番チケットレス(取消時に割引認証情報をセットしてしまう)
	DC_QR_work.QR_Info.CertCnt = disc_cnt;											// 割引認証数(0～25)
// MH810100(S) K.Onodera  2020/03/10 車番チケットレス(#3971 割引認証登録・取消要求データにQRコードIDを設定していない)
	DC_QR_work.QR_Info.DiscountInfoSize = (sizeof(QR_Discount) * disc_cnt) ;		// 割引認証情報データサイズ
// MH810100(E) K.Onodera  2020/03/10 車番チケットレス(#3971 割引認証登録・取消要求データにQRコードIDを設定していない)

	DC_QR_work.QR_Info.QR_ID = pMedia->Barcode.id;						// QRコードID(ユニークID)
	DC_QR_work.QR_Info.QR_FormRev = pMedia->Barcode.rev;				// QRコードフォーマットRev.№
	DC_QR_work.QR_Info.QR_type = (pMedia->Barcode.id - BAR_ID_AMOUNT);	// QR種別(0=QR買上げ/1=QR割引き)
// MH810100(S) K.Onodera  2020/03/10 車番チケットレス(#3971 割引認証登録・取消要求データにQRコードIDを設定していない)
//	DC_QR_work.QR_Info.QR_data_size = BAR_INFO_SIZE;					// QR情報データサイズ(パースデータ)
//	DC_QR_work.QR_Info.QR_row_size = BAR_DATASIZE;						// QRコードデータサイズ(生データ)
	DC_QR_work.QR_Info.QR_data_size = pMedia->Barcode.info_size;		// QR情報データサイズ(パースデータ)
	DC_QR_work.QR_Info.QR_row_size = pMedia->RowSize;					// QRコードデータサイズ(生データ)
// MH810100(E) K.Onodera  2020/03/10 車番チケットレス(#3971 割引認証登録・取消要求データにQRコードIDを設定していない)
	memcpy( DC_QR_work.QR_Info.QR_data, &pMedia->Barcode.QR_data, BAR_INFO_SIZE );
	memcpy( DC_QR_work.QR_Info.QR_row_data, pMedia->RowData, BAR_DATASIZE );

	Log_regist( LOG_DC_QR );				// ログ登録
	DC_UpdateCenterSeqNo( DC_SEQNO_QR );	// センター追番をカウントアップ
}

// MH810100(S) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
void ope_MakeCertifLog_quick( tMediaDetail *pMedia, uchar flg, eCertifKind kind )
{

	ushort cnt = 0, disc_cnt = 0;
	stDiscount_t* pDisc = NULL;
	DATE_YMDHMS		Certif_tim;
	// 認証取消時刻
	memcpy( &Certif_tim, &CLK_REC, sizeof(DATE_YMDHMS) );

	memset( &DC_QR_work, 0, sizeof(DC_QR_work) );

	DC_QR_work.ID = op_GetSequenceID();							// 入庫から精算完了までを管理するID
	DC_QR_work.CenterOiban = DC_GetCenterSeqNo( DC_SEQNO_QR );	// センター追番
	if( DC_QR_work.CenterOiban == 0 ){
		DC_UpdateCenterSeqNo( DC_SEQNO_QR );
		DC_QR_work.CenterOiban = 1;
		DC_QR_work.CenterOibanFusei = 1;						// センター追番不正フラグ
	}
	DC_QR_work.QR_Info_Rev = 1;									// QR情報Rev.№

	// QR情報セット ---------------------------------------------------------------
	DC_QR_work.QR_Info.FormRev = 1;								// フォーマットRev.№
	memcpy( &DC_QR_work.QR_Info.CertDate, &Certif_tim, sizeof(DC_QR_work.QR_Info.CertDate) );	// 認証確定時刻
	DC_QR_work.QR_Info.ProcessKind = kind;						// 処理区分(0=確定、1=取消)
	// 入庫媒体

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// 	// 問合せ応答の割引情報取得結果の「精算媒体」をセットする
// 	DC_QR_work.QR_Info.in_media.ParkingNo = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;		// 問合せ媒体 駐車場№		0～999999
// 	DC_QR_work.QR_Info.in_media.Kind = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;			// 問合せ媒体 種別			0～65000
// 	memcpy( DC_QR_work.QR_Info.in_media.CardNo, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo, sizeof(DC_QR_work.QR_Info.in_media.CardNo) );
// 																									// 問合せ媒体 番号			【ASCII】32桁（左詰、残りはNull固定）※本来種別101なら機械№+券№だが、それが入る？
	if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.FeeCalcOnOff ) {			// 料金計算結果あり
		// 料金計算結果
		DC_QR_work.QR_Info.in_media.ParkingNo =
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.ParkingLotNo;	// 精算媒体 駐車場№
		DC_QR_work.QR_Info.in_media.Kind =
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.CardType;		// 精算媒体 種別
		memcpy(DC_QR_work.QR_Info.in_media.CardNo,
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.byCardNo,		// 精算媒体 番号
			sizeof(DC_QR_work.QR_Info.in_media.CardNo));
	} else if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.PayResultInfo ) {	// 割引情報あり
		// 割引情報
		DC_QR_work.QR_Info.in_media.ParkingNo =
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;	// 精算媒体 駐車場№
		DC_QR_work.QR_Info.in_media.Kind =
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;		// 精算媒体 種別
		memcpy(DC_QR_work.QR_Info.in_media.CardNo,
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo,		// 精算媒体 番号
			sizeof(DC_QR_work.QR_Info.in_media.CardNo));
	}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

	DC_QR_work.QR_Info.Passkind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.SeasonKind;		// 定期種別
	DC_QR_work.QR_Info.MemberKind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.MemberKind;	// 会員種別

	// 入庫駐車場№
	DC_QR_work.QR_Info.InParkNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.ParkingNo;
	// 入庫時間(年月日時分秒)
	memcpy( &DC_QR_work.QR_Info.InDate, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec, sizeof(DC_QR_work.QR_Info.InDate) );

	DC_QR_work.QR_Info.ReciptCnt = 1;			// レシート枚数1枚固定（即時なので）
	DC_QR_work.QR_Info.Method = 0;														// 割引方法(0=オンラインデータ、1=オフラインデータ、2=AR-100／150 )
	DC_QR_work.QR_Info.Lastflg = flg;													// 最終フラグ(0=認証開始データ、1=認証途中データ、2=認証最終データ)

	// 割引認証情報
	if( kind == 0 ){	// 処理区分(0=確定、1=取消)
		for( cnt=0; cnt<ONL_MAX_DISC_NUM; cnt++ ){
			pDisc = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[cnt];
			// 割引情報あり？
			if( pDisc->DiscSyu || pDisc->DiscParkNo ){
				disc_cnt++;
				// 情報セット
				DC_QR_work.QR_Info.DiscountInfo[cnt].ParkingNo = pDisc->DiscParkNo;		// 駐車場№
				DC_QR_work.QR_Info.DiscountInfo[cnt].Kind = pDisc->DiscSyu;				// 種別
				DC_QR_work.QR_Info.DiscountInfo[cnt].CardType = pDisc->DiscCardNo;		// カード区分
				DC_QR_work.QR_Info.DiscountInfo[cnt].Type = pDisc->DiscNo;				// 区分
				DC_QR_work.QR_Info.DiscountInfo[cnt].DiscInfo = pDisc->DiscInfo;		// 割引情報
			}
		}
	}
	DC_QR_work.QR_Info.CertCnt = disc_cnt;											// 割引認証数(0～25)
	DC_QR_work.QR_Info.DiscountInfoSize = (sizeof(QR_Discount) * disc_cnt) ;		// 割引認証情報データサイズ

	DC_QR_work.QR_Info.QR_ID = pMedia->Barcode.id;						// QRコードID(ユニークID)
	DC_QR_work.QR_Info.QR_FormRev = pMedia->Barcode.rev;				// QRコードフォーマットRev.№
	DC_QR_work.QR_Info.QR_type = (pMedia->Barcode.id - BAR_ID_AMOUNT);	// QR種別(0=QR買上げ/1=QR割引き)
	DC_QR_work.QR_Info.QR_data_size = pMedia->Barcode.info_size;		// QR情報データサイズ(パースデータ)
	DC_QR_work.QR_Info.QR_row_size = pMedia->RowSize;					// QRコードデータサイズ(生データ)
	memcpy( DC_QR_work.QR_Info.QR_data, &pMedia->Barcode.QR_data, BAR_INFO_SIZE );
	memcpy( DC_QR_work.QR_Info.QR_row_data, pMedia->RowData, BAR_DATASIZE );

	Log_regist( LOG_DC_QR );				// ログ登録
	DC_UpdateCenterSeqNo( DC_SEQNO_QR );	// センター追番をカウントアップ
}
// MH810100(E) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)

//[]----------------------------------------------------------------------[]
///	@brief			オフライン時の認証データ登録
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
void ope_RegisterCertificateData( eCertifKind kind )
{
	ushort	index = 0;
	ushort	start = 0;
	uchar 	lastflg = 0;

	DC_PushCenterSeqNo( DC_SEQNO_QR );	// センター追番をPush

	for( index = 0; index < g_UseMediaData.bar_count; ++index ){
		WACDOG;

		// 最終フラグ
		if( index == g_UseMediaData.bar_count-1 ){
			lastflg = 2;		// 最終
		}
		else if( start == 0 ){
			start = 1;			// 開始
			lastflg = 0;
		}
		else{
			lastflg = 1;		// 途中
		}
		ope_MakeCertifLog( &g_UseMediaData.data[index], lastflg, kind );
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			認証確定要求送信
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
BOOL ope_SendCertifCommit( void )
{
	// 認証確定時刻
	memcpy( &g_UseMediaData.Certif_tim, &CLK_REC, sizeof(DATE_YMDHMS) );

	ope_RegisterCertificateData( CERTIF_COMMIT );

	return TRUE;
}

//[]----------------------------------------------------------------------[]
///	@brief			認証取消要求送信
//[]----------------------------------------------------------------------[]
///	@param			bWaitRes : 応答待ちフラグ
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
//static BOOL ope_SendCertifCancel( void )
BOOL ope_SendCertifCancel( void )
{
	// 認証取消時刻
	memcpy( &g_UseMediaData.Certif_tim, &CLK_REC, sizeof(DATE_YMDHMS) );

	ope_RegisterCertificateData( CERTIF_ROLLBACK );

	return TRUE;
}

// MH810100(S) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
// キャンセルデータを即時登録
void ope_CancelRegist( tMediaDetail* pMedia )
{
	ope_MakeCertifLog_quick( pMedia, 2, CERTIF_ROLLBACK );
}
// MH810100(E) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
// MH810100(S) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）
// センター問い合わせっ結果をDC用の情報にセット
void SetQRMediabyQueryResult()
{
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// 	// 問合せ応答の割引情報取得結果の「精算媒体」をセットする
// 	g_UseMediaData.QueryResult.Query_ParkingNo = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;		// 問合せ媒体 駐車場№		0～999999
// 	g_UseMediaData.QueryResult.Query_Kind = lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;			// 問合せ媒体 種別			0～65000
// 	memcpy( g_UseMediaData.QueryResult.Query_CardNo, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo, sizeof(g_UseMediaData.QueryResult.Query_CardNo) );
// 																									// 問合せ媒体 番号			【ASCII】32桁（左詰、残りはNull固定）※本来種別101なら機械№+券№だが、それが入る？
	if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.FeeCalcOnOff ) {			// 料金計算結果あり
		// 料金計算結果
		g_UseMediaData.QueryResult.Query_ParkingNo =
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.ParkingLotNo;	// 精算媒体 駐車場№
		g_UseMediaData.QueryResult.Query_Kind =
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.CardType;		// 精算媒体 種別
// GG124100(S) R.Endo 2022/08/24 車番チケットレス3.0 #6558 精算中止時、割引認証登録・取消要求データのカード番号の値が「0」となっている
// 		memcpy(DC_QR_work.QR_Info.in_media.CardNo,
// 			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.byCardNo,		// 精算媒体 番号
// 			sizeof(DC_QR_work.QR_Info.in_media.CardNo));
		memcpy(g_UseMediaData.QueryResult.Query_CardNo,
			lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.dtPayMedia.byCardNo,		// 精算媒体 番号
			sizeof(g_UseMediaData.QueryResult.Query_CardNo));
// GG124100(E) R.Endo 2022/08/24 車番チケットレス3.0 #6558 精算中止時、割引認証登録・取消要求データのカード番号の値が「0」となっている
	} else if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.PayResultInfo ) {	// 割引情報あり
		// 割引情報
		g_UseMediaData.QueryResult.Query_ParkingNo =
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.ParkingLotNo;	// 精算媒体 駐車場№
		g_UseMediaData.QueryResult.Query_Kind =
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.CardType;		// 精算媒体 種別
		memcpy(g_UseMediaData.QueryResult.Query_CardNo,
			lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.dtPayMedia.byCardNo,		// 精算媒体 番号
			sizeof(g_UseMediaData.QueryResult.Query_CardNo));
	}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
	g_UseMediaData.QueryResult.Passkind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.SeasonKind;		// 定期種別
	g_UseMediaData.QueryResult.MemberKind	= lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.MemberKind;	// 会員種別
	// 入庫駐車場№
	g_UseMediaData.QueryResult.InParkNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.ParkingNo;
	// 入庫時間(年月日時分秒)
	memcpy( &g_UseMediaData.QueryResult.InDate, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec, sizeof(g_UseMediaData.QueryResult.InDate) );
}
// MH810100(E) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）

//[]----------------------------------------------------------------------[]
///	@brief			リアルタイム精算中止ログデータの作成
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void Set_Cancel_RTPay_Data(void)
{
	ushort	i,j;
// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// 	stDiscount2_t* pDisc2;
	stDiscount4_t* pDisc = NULL;
	stDiscount2_t* pDisc2 = NULL;
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
	stDiscount_t*  pDisc3;

	// ID
	RTPay_Data.ID = lcdbm_rsp_in_car_info_main.id;

	// カード情報Rev.No.
	RTPay_Data.crd_info_rev_no = lcdbm_rsp_in_car_info_main.crd_info_rev_no;

	// ﾌｫｰﾏｯﾄRev.No.
	RTPay_Data.crd_info.FormatNo = lcdbm_rsp_in_car_info_main.crd_info.FormatNo;

	// 在車情報_精算処理区分：中止にせず受信値をそのまま返す

	// 在車情報_精算_年月日時分秒(処理日時)：受信値をそのまま返す

	// 在車情報_駐車料金(0～999990(10円単位))：受信値をそのまま返す

	// 在車情報_駐車時間(0～999999(分単位))：受信値をそのまま返す

// MH810101(S) R.Endo 2021/02/09 #5255 【連動評価NG】センター問合せ結果の一時利用フラグを、精算完了後に精算機で書換ないでほしい
//	// 在車情報_一時利用フラグ(0=定期利用、1=一時利用)
//	if( ryo_buf.ryo_flg < 2 ){	// 駐車券精算処理
//		RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = 1;	// 一時利用
//	}else{
//		RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = 0;	// 定期利用
//	}
	// 在車情報_一時利用フラグ(0=定期利用、1=一時利用)：受信値をそのまま返す
// MH810101(E) R.Endo 2021/02/09 #5255 【連動評価NG】センター問合せ結果の一時利用フラグを、精算完了後に精算機で書換ないでほしい

	// 在車情報_割引01～25：受信値から割引済のみをコピーし直す※
	// ※割引済であっても再精算時に残りの割引を使用して割引額が更新されている可能性がある為
	memset( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0, sizeof(stDiscount_t)*ONL_MAX_DISC_NUM );

// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
	if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.FeeCalcOnOff ) {			// 料金計算結果あり
		for ( i = 0, j = 0; i < ONL_MAX_DISC_NUM; i++ ) {
			pDisc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.stDiscountInfo[i];
			if ( pDisc->DiscStatus == 2 ) {	// 割引済
				pDisc3 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j];

				pDisc3->DiscParkNo		= pDisc->DiscParkNo;			// 駐車場№(0～999999)
				pDisc3->DiscSyu			= pDisc->DiscSyu;				// 種別(0～9999)
				pDisc3->DiscCardNo		= pDisc->DiscCardNo;			// ｶｰﾄﾞ区分(0～65000)
				pDisc3->DiscNo			= pDisc->DiscNo;				// 区分(0～9)
				pDisc3->DiscCount		= pDisc->DiscCount;				// 枚数(0～99)
				pDisc3->Discount		= pDisc->PrevUsageDiscAmount;	// 金額(0～999999)
				pDisc3->DiscTime		= pDisc->PrevUsageDiscTime;		// 時間(0～999999)
				pDisc3->DiscInfo		= pDisc->DiscInfo;				// 割引情報(0～65000)
				pDisc3->DiscCorrType	= pDisc->DiscCorrType;			// 対応ｶｰﾄﾞ種別(0～65000)
				pDisc3->DiscStatus		= pDisc->DiscStatus;			// ｽﾃｰﾀｽ(0～9)
				pDisc3->DiscFlg			= pDisc->DiscFlg;				// 割引状況(0～9)

				j++;
			}
		}
	} else if ( lcdbm_rsp_in_car_info_main.crd_info.dtReqRslt.PayResultInfo ) {	// 割引情報あり
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

	for( i = 0, j = 0; i < ONL_MAX_DISC_NUM; i++ ){
		pDisc2 = &lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[i];
		if( pDisc2->DiscStatus == 2 ){	// 割引済
			pDisc3 = &RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[j];

			pDisc3->DiscParkNo		= pDisc2->DiscParkNo;			// 割引 駐車場№(0～999999)
			pDisc3->DiscSyu			= pDisc2->DiscSyu;				// 割引 種別(0～9999)
			pDisc3->DiscCardNo		= pDisc2->DiscCardNo;			// 割引 ｶｰﾄﾞ区分(0～65000)
			pDisc3->DiscNo			= pDisc2->DiscNo;				// 割引 区分(0～9)
			pDisc3->DiscCount		= pDisc2->DiscCount;			// 割引 枚数(0～99)
			pDisc3->DiscInfo		= pDisc2->DiscInfo;				// 割引 割引情報(0～65000)
			pDisc3->DiscCorrType	= pDisc2->DiscCorrType;			// 割引 対応ｶｰﾄﾞ種別(0～65000)
			pDisc3->DiscStatus		= pDisc2->DiscStatus;			// 割引 ｽﾃｰﾀｽ(0～9)
			pDisc3->DiscFlg			= pDisc2->DiscFlg;				// 割引 割引状況(0～9)
// MH810100(S) 2020/05/28 車番チケットレス(#4196)
//			if( pDisc2->DiscSyu >= NTNET_SVS_T ){	// 時間割引？
//				pDisc3->Discount		= pDisc2->DiscountT;		// 割引 金額／時間(時間エリアからセット)
//			}else{
//				pDisc3->Discount		= pDisc2->DiscountM;		// 割引 金額／時間(金額エリアからセット)
//			}
// MH810100(S) 2020/07/29 車番チケットレス(#4561 センターから受信した割引情報を変更する)
//			pDisc3->Discount		= pDisc2->Discount;				// 金額/時間(0～999999)
			pDisc3->Discount		= pDisc2->UsedDisc;				// 金額/時間(0～999999)
// MH810100(E) 2020/07/29 車番チケットレス(#4561 センターから受信した割引情報を変更する)
//			pDisc3->UsedDisc		= pDisc2->UsedDisc;				// 買物割引利用時の使用済み割引（金額/時間）(0～999999)
// MH810100(E) 2020/05/28 車番チケットレス(#4196)
// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
			pDisc3->DiscTime		= 0;							// 時間(0～999999)
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)

			j++;
		}
	}

// GG124100(S) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
	}
// GG124100(E) R.Endo 2022/08/08 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

}

//[]----------------------------------------------------------------------[]
///	@brief			リアルタイム精算完了ログデータの作成
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
#define	EC_KOUTSUU_USED_TEMP	99
void Set_Pay_RTPay_Data(void)
{
	ushort	CardKind = 0;	// 決済区分
	ulong	amount = 0L;	// 決算金額
	ulong	indate;			// 入庫日
	ulong	outdate;		// 出庫日
	ulong	intime;			// 入庫日時（分換算）
	ulong	outtime;		// 出庫日時（分換算）
	ulong	parktime = 0L;	// 駐車時間（分換算）
	ushort	loop_cnt;
// MH810100(S) 2020/08/31 #4787 リアルタイム通信　未精算出庫の精算データ（ID：57）の精算年月日時分秒が出庫年月日時分秒と同じになっている
	struct clk_rec *pTime;
// MH810100(E) 2020/08/31 #4787 リアルタイム通信　未精算出庫の精算データ（ID：57）の精算年月日時分秒が出庫年月日時分秒と同じになっている

// MH810100(S) 2020/09/08 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
// Set_All_RTPay_Dataでセット済み
//	// ID
//	RTPay_Data.ID = lcdbm_rsp_in_car_info_main.id;
//
//	// カード情報Rev.No.
//	RTPay_Data.crd_info_rev_no = lcdbm_rsp_in_car_info_main.crd_info_rev_no;
//
//	// ﾌｫｰﾏｯﾄRev.No.
//	RTPay_Data.crd_info.FormatNo = lcdbm_rsp_in_car_info_main.crd_info.FormatNo;
// MH810100(E) 2020/09/08 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）

	// 在車情報_駐車場№(0～999999)
	RTPay_Data.crd_info.dtZaishaInfo.ResParkingLotNo = prm_get( COM_PRM, S_SYS, 1, 6, 1);	// 駐車場№

	// 在車情報_精算処理区分
	// 通常精算？
	if( PayData.PayClass <= 3 ){ // PayData.PayClass = 0(精算)/1(再精算)(2(精算中止)/3(再精算中止)は通らない)
		RTPay_Data.crd_info.dtZaishaInfo.PaymentType = PayData.PayClass + 1;	// 1(精算)/2(再精算)
	}
	// クレジット？
	else if( PayData.PayClass <= 5 ){
		RTPay_Data.crd_info.dtZaishaInfo.PaymentType = (PayData.PayClass - 4) + 1;
	}
	// その他
	else{
		RTPay_Data.crd_info.dtZaishaInfo.PaymentType = 1;	// 範囲外の場合は「1(精算)」固定
	}

	// 在車情報_精算出庫(0＝通常精算／1＝強制出庫／（2＝精算なし出庫）／3＝不正出庫／9＝突破出庫／10＝ゲート開放／
	//					 97＝ロック開・フラップ上昇前未精算出庫／98＝ラグタイム内出庫／99＝サービスタイム内出庫)
	RTPay_Data.crd_info.dtZaishaInfo.ExitPayment = PayData.OutKind;

	// 在車情報_現金売上額(0～999999)
	RTPay_Data.crd_info.dtZaishaInfo.GenkinFee = (ulong)(PayData.WTotalPrice + PayData.zenkai);	// 今回領収額＋前回領収額
// MH810100(S) S.Fujii 2020/08/26 #4747 クレジット精算で再精算時に現金売上額が入っている
	for( loop_cnt = 0; loop_cnt < ONL_MAX_SETTLEMENT; loop_cnt++ ){
		if( RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeType != 0L ) {
			// 前回決算額を引く
			// CargeFeeにはこの時点で今回分の決済額はまだ足されていないので前回決済額のみが入っている
			RTPay_Data.crd_info.dtZaishaInfo.GenkinFee -= RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeFee;	// 決済額
		} else {
			break;
		}
	}
// MH810100(E) S.Fujii 2020/08/26 #4747 クレジット精算で再精算時に現金売上額が入っている

	// 在車情報_料金種別(0～99)
// MH810100(S) 2020/08/19 車番チケットレス(#4742 【検証課指摘事項】　種別切替後のリアルタイム精算データの料金種別が変更されていない)
	//RTPay_Data.crd_info.dtZaishaInfo.shFeeType = (ushort)PayData.syu;		// 「料金種別」は参照のみのため、種別切替が発生しても変更しない
	// 仕様変更により復活
	RTPay_Data.crd_info.dtZaishaInfo.shFeeType = (ushort)PayData.syu;		
// MH810100(S) 2020/08/19 車番チケットレス(#4742 【検証課指摘事項】　種別切替後のリアルタイム精算データの料金種別が変更されていない)

	// 在車情報_精算_年月日時分秒(処理日時)
	RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.ParkingNo = prm_get( COM_PRM, S_SYS, 1, 6, 1);	// 駐車場№(0～999999)
	RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.MachineKind = NTNET_MODEL_CODE;					// 機種ｺｰﾄﾞ(0～999)
	RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.MachineNo = prm_get( COM_PRM, S_PAY, 2, 2, 1);	// 機械№(0～999)
// MH810100(S) 2020/08/31 #4787 リアルタイム通信　未精算出庫の精算データ（ID：57）の精算年月日時分秒が出庫年月日時分秒と同じになっている
//// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:223,224)対応
////	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear,					// 年月日時分秒
////			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear,
////			sizeof( stDatetTimeYtoSec_t ));
//	memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,							// 年月日時分秒
//			&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,
//			sizeof( stDatetTimeYtoSec_t ));
	if( lcdbm_rsp_in_car_info_main.shubetsu == 1 ){
		pTime = GetPayStartTime();

		// 精算時間に今の時間、出庫時間に、精算時間（set_tim_only_out_card関数で、出庫時間を精算時間にいれているので）、精算時間を入れる

		// 在車情報_精算 年月日時分秒
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear		= pTime->year;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth		= pTime->mont;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay		= pTime->date;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours		= pTime->hour;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute	= pTime->minu;
		RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond	= pTime->seco;

		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec,							// 年月日時分秒
				&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,
				sizeof( stDatetTimeYtoSec_t ));

	}else{
		memcpy( &RTPay_Data.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,							// 年月日時分秒
				&lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec,
				sizeof( stDatetTimeYtoSec_t ));
	}
// MH810100(E) 2020/08/31 #4787 リアルタイム通信　未精算出庫の精算データ（ID：57）の精算年月日時分秒が出庫年月日時分秒と同じになっている

	// 在車情報_駐車料金(0～999990(10円単位))
	RTPay_Data.crd_info.dtZaishaInfo.ParkingFee = PayData.WPrice;

	// 在車情報_駐車時間(0～999999(分単位)):駐車時間印字ﾃﾞｰﾀ編集処理(RYOUSYUU_edit_parktime())参考
	if( PayData.TInTime.Year != 0 ){		// 入庫時刻あり？
		indate	= dnrmlzm(					// 入庫日計算
							(short)PayData.TInTime.Year,
							(short)PayData.TInTime.Mon,
							(short)PayData.TInTime.Day
						);
		outdate	= dnrmlzm(					// 出庫日計算
							(short)PayData.TOutTime.Year,
							(short)PayData.TOutTime.Mon,
							(short)PayData.TOutTime.Day
						);
		intime	= (indate*24*60) + (PayData.TInTime.Hour*60) + (PayData.TInTime.Min);		// 入庫日時（分換算）計算
		outtime	= (outdate*24*60) + (PayData.TOutTime.Hour*60) + (PayData.TOutTime.Min);	// 出庫日時（分換算）計算
		if( outtime >= intime ){			// 出庫日時≧入庫日時
			parktime  = outtime - intime;	// 駐車時間取得(分換算)
		}
	}
	RTPay_Data.crd_info.dtZaishaInfo.ParkingTime = parktime;

// MH810101(S) R.Endo 2021/02/09 #5255 【連動評価NG】センター問合せ結果の一時利用フラグを、精算完了後に精算機で書換ないでほしい
//	// 在車情報_一時利用フラグ(0=定期利用、1=一時利用)
//	if( ryo_buf.ryo_flg < 2 ){	// 駐車券精算処理
//		RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = 1;	// 一時利用
//	}else{
//		RTPay_Data.crd_info.dtZaishaInfo.NormalFlag = 0;	// 定期利用
//	}
	// 在車情報_一時利用フラグ(0=定期利用、1=一時利用)：受信値をそのまま返す
// MH810101(E) R.Endo 2021/02/09 #5255 【連動評価NG】センター問合せ結果の一時利用フラグを、精算完了後に精算機で書換ないでほしい

	// 在車情報_決算区分/決算金額
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// 	if( PayData.Electron_data.Suica.e_pay_kind != 0 ){	// 電子決済種別
// 		switch( PayData.Electron_data.Suica.e_pay_kind ){
// 		case EC_CREDIT_USED:	// クレジット
// 		case EC_HOUJIN_USED:	// 法人カード
// 			CardKind = (ushort)1;
// 			break;
// 		case EC_EDY_USED:		// Edy
// 			CardKind = (ushort)3;
// 			break;
// // 「EC_PITAPA_USED」は「ICクレジット対応」移植後も定義がない為(PiTaPa精算未対応？)、コメントアウト
// //		case EC_PITAPA_USED:	// PiTaPa
// //			CardKind = (ushort)4;
// //			break;
// 		case EC_WAON_USED:		// WAON
// 			CardKind = (ushort)5;
// 			break;
// 		case EC_NANACO_USED:	// nanaco
// 			CardKind = (ushort)6;
// 			break;
// 		case EC_SAPICA_USED:	// SAPICA
// 			CardKind = (ushort)7;
// 			break;
// 		case EC_KOUTSUU_USED:	// 交通系電子マネー
// 			CardKind = (ushort)EC_KOUTSUU_USED_TEMP;	// 既存と区別するための値を一時的にセット
// 			break;
// // 「EC_ETC_USED/EC_ETC_EC_USED」は「ICクレジット対応」移植後も定義がない為(ETC/ETC-EC精算未対応？)、コメントアウト
// //		case EC_ETC_USED:
// //			CardKind = (ushort)10;
// //			break;
// //		case EC_ETC_EC_USED:
// //			CardKind = (ushort)11;
// //			break;
// // 「EC_ID_USED/EC_QUIC_PAY_USED」は「遠隔通信データ カード情報仕様書」に記載がない為、コメントアウト
// //		case EC_ID_USED:
// //			CardKind = (ushort)12;
// //			break;
// //		case EC_QUIC_PAY_USED:
// //			CardKind = (ushort)13;
// //			break;
// // MH810104(S) nanaco・iD・QUICPay対応2 #5849
// 		case EC_ID_USED:
// 			CardKind = (ushort)12;
// 			break;
// 		case EC_QUIC_PAY_USED:
// 			CardKind = (ushort)13;
// 			break;
// // MH810104(E) nanaco・iD・QUICPay対応2 #5849
// // MH810105(S) MH364301 QRコード決済対応 #6508 リアルタイム通信の在車情報の決算区分にQR決済がない
// 		case EC_QR_USED:					// [24]QRコード決済
// 			CardKind = (ushort)16;
// 			break;
// 		case EC_PITAPA_USED:				// [25]PiTaPa
// 			CardKind = (ushort)4;
// 			break;
// // MH810105(E) MH364301 QRコード決済対応 #6508 リアルタイム通信の在車情報の決算区分にQR決済がない
// 		default:		// 交通系電子マネー(Suica,PASMO,ICOCA等)
// 			CardKind = (ushort)2;
// 			break;
// 		}
// 	}
	CardKind = GetCardKind();
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	switch( CardKind ){	// 決済区分
	case 1:		// クレジット
		amount = PayData.credit.pay_ryo;				// 決済額
		break;
	case 2:		// 交通系電子マネー(SX-20:Suica,PASMO,ICOCA等)
		amount = PayData.Electron_data.Suica.pay_ryo;	// 電子決済精算情報　決済額
		break;
	case EC_KOUTSUU_USED_TEMP:	// 交通系 (既存と区別するため一時的にセットした値)
		CardKind = 2;
		// no break
	case 3:		// Edy
//	case 4:		// PiTaPa
// MH810105(S) MH364301 QRコード決済対応 #6508 リアルタイム通信の在車情報の決算区分にQR決済がない
	case 4:		// PiTaPa
// MH810105(E) MH364301 QRコード決済対応 #6508 リアルタイム通信の在車情報の決算区分にQR決済がない
	case 5:		// WAON
	case 6:		// nanaco
	case 7:		// SAPICA
//	case 12:	// iD
//	case 13:	// QUICPay
// MH810104(S) nanaco・iD・QUICPay対応2
	case 12:	// iD
	case 13:	// QUICPay
// MH810104(E) nanaco・iD・QUICPay対応2
// MH810105(S) MH364301 QRコード決済対応 #6508 リアルタイム通信の在車情報の決算区分にQR決済がない
	case 16:	// QR
// MH810105(E) MH364301 QRコード決済対応 #6508 リアルタイム通信の在車情報の決算区分にQR決済がない
		amount = PayData.Electron_data.Ec.pay_ryo;		// 電子決済精算情報　決済額
		break;
	default:
		break;
	}
// MH810100(S) S.Fujii 2020/07/06 車番チケットレス(電子決済端末対応)
//	// クレジット/電子マネー共に「使用＝精算完了」なので複数使用はない([0]にのみセット)
//	RTPay_Data.crd_info.dtZaishaInfo.stSettlement[0].CargeType = CardKind;	// 決済区分
//	RTPay_Data.crd_info.dtZaishaInfo.stSettlement[0].CargeFee = amount;		// 決算金額
	for( loop_cnt = 0; loop_cnt < ONL_MAX_SETTLEMENT; loop_cnt++ ){
		if(( RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeType == 0L ) &&
		   ( RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeFee == 0 )){
			// 今回の決済データを追加
			RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeType = CardKind;	// 決済区分
			RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeFee = amount;		// 決算金額
			break;
		} else if ( RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeType == CardKind ) {
			// 既に同じ決済区分が存在する場合は決済金額を加算する
			RTPay_Data.crd_info.dtZaishaInfo.stSettlement[loop_cnt].CargeFee += amount;		// 決算金額
			break;
		}
	}
// MH810100(E) S.Fujii 2020/07/06 車番チケットレス(電子決済端末対応)

	// 最大割引データ数分ループ(今回割引を割引済へ)
	for( loop_cnt = 0; loop_cnt < ONL_DISC_MAX; loop_cnt++ ){
		if(( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt].DiscParkNo == 0L ) &&
		   ( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt].DiscSyu == 0 )){
			// 割引データがなければ抜ける
			break;
		}
		if( RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt].DiscStatus == 1 ){	// 今回割引？
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[loop_cnt].DiscStatus = 2;	// 割引済へ
		}
	}

}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

// GG129004(S) R.Endo 2024/12/10 電子領収証対応
//[]----------------------------------------------------------------------[]
///	@brief			決済区分取得
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			決済区分
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
ushort	GetCardKind( void )
{
	ushort	CardKind = 0;	// 決済区分

	if ( PayData.Electron_data.Suica.e_pay_kind != 0 ) {	// 電子決済種別
		switch ( PayData.Electron_data.Suica.e_pay_kind ) {
		case EC_CREDIT_USED:		// [20]クレジット
		case EC_HOUJIN_USED:		// [22]法人
			CardKind = (ushort)1;
			break;
		case EC_EDY_USED:			// [13]Edy
			CardKind = (ushort)3;
			break;
		case EC_WAON_USED:			// [15]WAON
			CardKind = (ushort)5;
			break;
		case EC_NANACO_USED:		// [14]nanaco
			CardKind = (ushort)6;
			break;
		case EC_SAPICA_USED:		// [16]SAPICA
			CardKind = (ushort)7;
			break;
		case EC_KOUTSUU_USED:		// [17]交通系ICカード
			CardKind = (ushort)EC_KOUTSUU_USED_TEMP;	// 既存と区別するための値を一時的にセット
			break;
		case EC_ID_USED:			// [18]iD
			CardKind = (ushort)12;
			break;
		case EC_QUIC_PAY_USED:		// [19]QuicPay
			CardKind = (ushort)13;
			break;
		case EC_QR_USED:			// [24]QRコード決済
			CardKind = (ushort)16;
			break;
		case EC_PITAPA_USED:		// [25]PiTaPa
			CardKind = (ushort)4;
			break;
//		case EC_TCARD_USED:			// [21]Tカード
//		case EC_HOUSE_USED:			// [23]ハウスカード
		default:					// 交通系電子マネー(Suica,PASMO,ICOCA等)
			CardKind = (ushort)2;
			break;
		}
	}

	return CardKind;
}

//[]----------------------------------------------------------------------[]
///	@brief			RT領収証データの作成
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	Set_Pay_RTReceipt_Data( void )
{
	struct clk_rec wkClk;
	stReceiptInfo *pInfo = &RTReceipt_Data.receipt_info;	// 領収証情報
	stMasterInfo_t *pMaster = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo;	// マスター情報
	stParkKindNum_t *pCard = NULL;
	ulonglong carDataId;
	ulong indate;	// 入庫日
	ulong outdate;	// 出庫日
	ulong intime;	// 入庫日時（分換算）
	ulong outtime;	// 出庫日時（分換算）
	ulong parktime;	// 駐車時間（分換算）
	int i;
	int j;
// GG129004(S) R.Endo 2024/12/13 #7565 現金で精算完了しても領収証データのカード決済日時に「1980/3/3 00:00:00」がセットされる
	int isCard = 1;
// GG129004(E) R.Endo 2024/12/13 #7565 現金で精算完了しても領収証データのカード決済日時に「1980/3/3 00:00:00」がセットされる
	ushort CardKind;	// 決済区分
	uchar buffer[30];

	// Set_All_RTReceipt_Data で一部設定済み

	// 精算情報 RYOUSYUU_edit_seisanj
	pInfo->MachineNo          = PayData.Kikai_no;		// 機械№
	pInfo->SettlementModeMode = PayData.Seisan_kind;	// 精算モード
	pInfo->IndividualNo       = PayData.Oiban.i;		// 精算№ 個別追番
	pInfo->ConsecutiveNo      = PayData.Oiban.w;		// 精算№ 通し追番

	// 入庫時刻 RYOUSYUU_edit_intime
	if ( (PayData.TInTime.Year != 0) ||
	     (PayData.TInTime.Mon  != 0) ||
	     (PayData.TInTime.Day  != 0) ||
	     (PayData.TInTime.Hour != 0) ||
	     (PayData.TInTime.Min  != 0) ) {
		pInfo->dtEntry.shYear   = PayData.TInTime.Year;	// 入庫年
		pInfo->dtEntry.byMonth  = PayData.TInTime.Mon ;	// 入庫月
		pInfo->dtEntry.byDay    = PayData.TInTime.Day ;	// 入庫日
		pInfo->dtEntry.byHours  = PayData.TInTime.Hour;	// 入庫時
		pInfo->dtEntry.byMinute = PayData.TInTime.Min ;	// 入庫分
	}
	if ( (PayData.BeforeTPayTime.Year != 0) ||
	     (PayData.BeforeTPayTime.Mon  != 0) ||
	     (PayData.BeforeTPayTime.Day  != 0) ||
	     (PayData.BeforeTPayTime.Hour != 0) ||
	     (PayData.BeforeTPayTime.Min  != 0) ) {
			pInfo->dtOldPayment.shYear   = PayData.BeforeTPayTime.Year;	// 前回精算年
			pInfo->dtOldPayment.byMonth  = PayData.BeforeTPayTime.Mon ;	// 前回精算月
			pInfo->dtOldPayment.byDay    = PayData.BeforeTPayTime.Day ;	// 前回精算日
			pInfo->dtOldPayment.byHours  = PayData.BeforeTPayTime.Hour;	// 前回精算時
			pInfo->dtOldPayment.byMinute = PayData.BeforeTPayTime.Min ;	// 前回精算分
	}

	// 出庫(精算)時刻 RYOUSYUU_edit_outtime
	if ( PayData.shubetsu == 1 ) {	// 未精算出庫精算
		if ( (PayData.TUnpaidPayTime.Year != 0) ||
		     (PayData.TUnpaidPayTime.Mon  != 0) ||
		     (PayData.TUnpaidPayTime.Day  != 0) ||
		     (PayData.TUnpaidPayTime.Hour != 0) ||
		     (PayData.TUnpaidPayTime.Min  != 0) ) {
			pInfo->dtPayment.shYear   = PayData.TUnpaidPayTime.Year;	// 精算年
			pInfo->dtPayment.byMonth  = PayData.TUnpaidPayTime.Mon ;	// 精算月
			pInfo->dtPayment.byDay    = PayData.TUnpaidPayTime.Day ;	// 精算日
			pInfo->dtPayment.byHours  = PayData.TUnpaidPayTime.Hour;	// 精算時
			pInfo->dtPayment.byMinute = PayData.TUnpaidPayTime.Min ;	// 精算分
		}
		if ( (PayData.TOutTime.Year != 0) ||
		     (PayData.TOutTime.Mon  != 0) ||
		     (PayData.TOutTime.Day  != 0) ||
		     (PayData.TOutTime.Hour != 0) ||
		     (PayData.TOutTime.Min  != 0) ) {
			pInfo->dtExitDateTime.shYear   = PayData.TOutTime.Year;	// 出庫年
			pInfo->dtExitDateTime.byMonth  = PayData.TOutTime.Mon ;	// 出庫月
			pInfo->dtExitDateTime.byDay    = PayData.TOutTime.Day ;	// 出庫日
			pInfo->dtExitDateTime.byHours  = PayData.TOutTime.Hour;	// 出庫時
			pInfo->dtExitDateTime.byMinute = PayData.TOutTime.Min ;	// 出庫分
		}
	} else {
		if ( (PayData.TOutTime.Year != 0) ||
		     (PayData.TOutTime.Mon  != 0) ||
		     (PayData.TOutTime.Day  != 0) ||
		     (PayData.TOutTime.Hour != 0) ||
		     (PayData.TOutTime.Min  != 0) ) {
			pInfo->dtPayment.shYear  	= PayData.TOutTime.Year;	// 精算年
			pInfo->dtPayment.byMonth 	= PayData.TOutTime.Mon ;	// 精算月
			pInfo->dtPayment.byDay   	= PayData.TOutTime.Day ;	// 精算日
			pInfo->dtPayment.byHours 	= PayData.TOutTime.Hour;	// 精算時
			pInfo->dtPayment.byMinute	= PayData.TOutTime.Min ;	// 精算分
		}
	}

	// 駐車時間 RYOUSYUU_edit_parktime
	if ( PayData.TInTime.Year != 0 ) {	// 入庫時刻あり
		// 入庫日計算
		indate	= dnrmlzm(
			(short)PayData.TInTime.Year,
			(short)PayData.TInTime.Mon,
			(short)PayData.TInTime.Day);

		// 出庫日計算
		outdate	= dnrmlzm(
			(short)PayData.TOutTime.Year,
			(short)PayData.TOutTime.Mon,
			(short)PayData.TOutTime.Day);

		// 入庫日時（分換算）計算
		intime	= (indate *24*60) + (PayData.TInTime.Hour *60) + (PayData.TInTime.Min);

		// 出庫日時（分換算）計算
		outtime	= (outdate*24*60) + (PayData.TOutTime.Hour*60) + (PayData.TOutTime.Min);

		// 駐車時間取得（分換算）
		if ( outtime >= intime ) {	// 出庫日時≧入庫日時
			parktime = outtime - intime;
		} else {	// 出庫日時＜入庫日時（入出庫日時ﾃﾞｰﾀ異常）
			parktime = 0;
		}
		pInfo->ParkingTime = parktime;	// 駐車時間
	}

	// 駐車料金 RYOUSYUU_edit_churyo
	pInfo->FeeKind    = PayData.syu;	// 料金種別
	pInfo->ParkingFee = PayData.WPrice;	// 駐車料金

	// 定期券種別, 契約№ RYOUSYUU_edit_teiki
	if ( PayData.teiki.id != 0 ) {	// 定期券使用
		for ( i = 0; i < ONL_MAX_CARDNUM; i++ ) {
			if( pMaster->stCardDataInfo[i].CardType == CARD_TYPE_PASS ) {	// 定期
				pCard = &pMaster->stCardDataInfo[i];
				break;
			}
		}
		if ( pCard != NULL ) {
			pInfo->SeasonParkingLotNo = pCard->ParkingLotNo;	// 定期券 駐車場№
			pInfo->SeasonKind         = PayData.teiki.syu;	// 定期券 種別
			pInfo->SeasonId           = PayData.teiki.id;	// 定期券 ID
			if ( ((PayData.teiki.s_year !=  0) || (PayData.teiki.s_mon !=  0) || (PayData.teiki.s_day !=  0) ||
			      (PayData.teiki.e_year != 99) || (PayData.teiki.e_mon != 99) || (PayData.teiki.e_day != 99)) ) {
				// 電子領収証は設定による表示切替は全てデフォルトとするため17-0002（定期券有効期限印字）は「する」固定
				pInfo->stSeasonValidStart.shYear  = PayData.teiki.s_year;	// 定期券 有効開始年月日
				pInfo->stSeasonValidStart.byMonth = PayData.teiki.s_mon;	// 定期券 有効開始年月日
				pInfo->stSeasonValidStart.byDay   = PayData.teiki.s_day;	// 定期券 有効開始年月日
				pInfo->stSeasonValidEnd.shYear    = PayData.teiki.e_year;	// 定期券 有効終了年月日
				pInfo->stSeasonValidEnd.byMonth   = PayData.teiki.e_mon;	// 定期券 有効終了年月日
				pInfo->stSeasonValidEnd.byDay     = PayData.teiki.e_day;	// 定期券 有効終了年月日
			}
		}
	}

	// 車番(精算)
	memcpy(pInfo->VehicleNumberSerialNo,
		lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber,
		sizeof(pInfo->VehicleNumberSerialNo));	// 車番 一連指定番号

	// 車番データID
	carDataId = atoll((const char *)PayData.CarDataID);
	if ( carDataId <= 0xFFFFFFFFFFFF ) {
        for ( i = 5; i >= 0; i-- ) {
            pInfo->UniqueID[i] = carDataId & 0xFF;
            carDataId >>= 8;
        }
	}

	// 非課税媒体利用額 RYOUSYUU_edit_taxableDiscount
	pInfo->PaidFeeAmount = PayData.zenkai;	// 支払済み料金

	// 合計金額(適用税率付) RYOUSYUU_edit_totalAmount_taxRate
	pInfo->Parking_Bil = PayData.WBillAmount;	// 合計金額（請求額）

	// 左詰めとスペースの追加とマスクの設定
	memset((char *)buffer, ' ', sizeof(buffer));
	CardKind = GetCardKind();
	switch ( CardKind ) {
	case 1:		// クレジット
		// 0123456789012345 -> 012345******2345
		for ( i = 0, j = 0; i < sizeof(PayData.credit.card_no); i++ ) {
			if ( PayData.credit.card_no[i] == '\0' || PayData.credit.card_no[i] == ' ' ) {
				continue;
			}
			if ( (i <= 5) || (12 <= i) ) {
				buffer[j] = PayData.credit.card_no[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case 2:		// 交通系電子マネー(SX-20)
		// JE234567890123456 -> JE*** **** **** 3456
		for ( i = 0, j = 0; i < sizeof(PayData.Electron_data.Suica.Card_ID); i++ ) {
			if ( PayData.Electron_data.Suica.Card_ID[i] == '\0' || PayData.Electron_data.Suica.Card_ID[i] == ' ' ) {
				continue;
			}
			switch ( i ) {
			case 5:
			case 9:
			case 13:
				buffer[j] = ' ';
				j++;
			}
			if ( (i <= 2) || (13 <= i) ) {
				buffer[j] = PayData.Electron_data.Suica.Card_ID[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case EC_KOUTSUU_USED_TEMP:	// 交通系 (既存と区別するため一時的にセットした値)
	case 4:		// PiTaPa
		// JE234567890123456 -> JE*** **** **** 3456
		for ( i = 0, j = 0; i < sizeof(PayData.Electron_data.Ec.Card_ID); i++ ) {
			if ( PayData.Electron_data.Ec.Card_ID[i] == '\0' || PayData.Electron_data.Ec.Card_ID[i] == ' ' ) {
				continue;
			}
			switch ( i ) {
			case 5:
			case 9:
			case 13:
				buffer[j] = ' ';
				j++;
			}
			if ( (i <= 2) || (13 <= i) ) {
				buffer[j] = PayData.Electron_data.Ec.Card_ID[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case 3:		// Edy
	case 5:		// WAON
	case 6:		// nanaco
	case 7:		// SAPICA
	case 12:	// iD
		// 0123456789012345 -> **** **** **** 2345
		for ( i = 0, j = 0; i < sizeof(PayData.Electron_data.Ec.Card_ID); i++ ) {
			if ( PayData.Electron_data.Ec.Card_ID[i] == '\0' || PayData.Electron_data.Ec.Card_ID[i] == ' ' ) {
				continue;
			}
			switch ( i ) {
			case 4:
			case 8:
			case 12:
				buffer[j] = ' ';
				j++;
			}
			if ( 12 <= i ) {
				buffer[j] = PayData.Electron_data.Ec.Card_ID[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case 13:	// QUICPay
		// 01234567890123456789 -> **** **** **** **** 6789
		for ( i = 0, j = 0; i < sizeof(PayData.Electron_data.Ec.Card_ID); i++ ) {
			if ( PayData.Electron_data.Ec.Card_ID[i] == '\0' || PayData.Electron_data.Ec.Card_ID[i] == ' ' ) {
				continue;
			}
			switch ( i ) {
			case 4:
			case 8:
			case 12:
			case 16:
				buffer[j] = ' ';
				j++;
			}
			if ( 16 <= i ) {
				buffer[j] = PayData.Electron_data.Ec.Card_ID[i];
			} else {
				buffer[j] = '*';
			}
			j++;
		}
		break;
	case 16:	// QR
	default:
		break;
	}

	// カード決済 取引決済情報
	switch ( CardKind ) {
	case 1:		// クレジット
		// カード決済 端末区分
		pInfo->CardPaymentTerminalCategory = 1;

		// カード決済 取引ステータス
		if ( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ) {
			pInfo->CardPaymentTransactionStatus = 3;	// 未了支払不明
		}
		else if ( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ) {
			pInfo->CardPaymentTransactionStatus = 1;	// みなし決済
		}
		else {
			pInfo->CardPaymentTransactionStatus = 0;	// 支払い
		}

		if ( PayData.PayClass == 2 || PayData.PayClass == 3 ) {	// 中止
			// 会員番号
			memset(pInfo->card.credit.card_no, 0x20, sizeof(pInfo->card.credit.card_no));

			// 端末識別番号
			memset(pInfo->card.credit.cct_num, 0x20, sizeof(pInfo->card.credit.cct_num));

			// KIDコード
			memset(pInfo->card.credit.kid_code, 0x20, sizeof(pInfo->card.credit.kid_code));

			// 加盟店取引番号
			memset(pInfo->card.credit.ShopAccountNo, 0x20, sizeof(pInfo->card.credit.ShopAccountNo));
		}
		else {
			// 利用金額
			pInfo->card.credit.amount = PayData.credit.pay_ryo;

			// 会員番号
			if ( (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) &&
			     (PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0) ) {
				// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.を0x20h埋めする。
				// [*]印字みなし決済と同様の会員Noをセットする。
				memset(pInfo->card.credit.card_no, 0x20,
					sizeof(pInfo->card.credit.card_no));
			}
			else {
				memcpy(pInfo->card.credit.card_no, buffer, sizeof(pInfo->card.credit.card_no));
			}
// GG129004(S) R.Endo 2024/12/12 #7563 クレジット決済時のリアルタイム領収証データの会員番号が「0」でマスクされている
// 			// 上7桁目～下5桁目以外で'*'が格納されていたら'0'に置換
// 			change_CharInArray(pInfo->card.credit.card_no, sizeof(pInfo->card.credit.card_no), 7, 5, '*', '0');
// GG129004(E) R.Endo 2024/12/12 #7563 クレジット決済時のリアルタイム領収証データの会員番号が「0」でマスクされている

			// 端末識別番号
			memcpyFlushLeft(pInfo->card.credit.cct_num,
				(uchar *)PayData.credit.CCT_Num,
				sizeof(pInfo->card.credit.cct_num),
				sizeof(PayData.credit.CCT_Num));

			// KIDコード
			memcpyFlushLeft(pInfo->card.credit.kid_code,
				PayData.credit.kid_code,
				sizeof(pInfo->card.credit.kid_code),
				sizeof(PayData.credit.kid_code));

			// 承認番号
			pInfo->card.credit.app_no = PayData.credit.app_no;

			// センター処理通番
			pInfo->card.credit.center_oiban = 0;

			// 加盟店取引番号
//			memcpyFlushLeft(pInfo->card.credit.ShopAccountNo,
//				PayData.credit.ShopAccountNo,
//				sizeof(pInfo->card.credit.ShopAccountNo),
//				sizeof(PayData.credit.ShopAccountNo));
			memset(pInfo->card.credit.ShopAccountNo, 0x20, sizeof(pInfo->card.credit.ShopAccountNo));

			// 伝票番号
			pInfo->card.credit.slip_no = PayData.credit.slip_no;
		}

		break;
	case 2:		// 交通系電子マネー(SX-20)
		// 利用金額
		pInfo->card.emoney.amount = PayData.Electron_data.Suica.pay_ryo;						

		// カード番号
		memcpy(pInfo->card.emoney.card_id, buffer, sizeof(pInfo->card.emoney.card_id));

		// カード残額
		pInfo->card.emoney.card_zangaku = PayData.Electron_data.Suica.pay_after;

		break;
	case EC_KOUTSUU_USED_TEMP:	// 交通系 (既存と区別するため一時的にセットした値)
		CardKind = 2;
		// no break
	case 3:		// Edy
	case 4:		// PiTaPa
	case 5:		// WAON
	case 6:		// nanaco
	case 7:		// SAPICA
	case 12:	// iD
	case 13:	// QUICPay
		// カード決済 端末区分
		pInfo->CardPaymentTerminalCategory = 1;

		// カード決済 取引ステータス
		if ( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ) {
			pInfo->CardPaymentTransactionStatus = 3;	// 未了支払不明
		}
		else if ( PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END ) {
			pInfo->CardPaymentTransactionStatus = 2;	// 未了支払済み
		}
		else if ( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ) {
			pInfo->CardPaymentTransactionStatus = 1;	// みなし決済
		}
		else {
			pInfo->CardPaymentTransactionStatus = 0;	// 支払い
		}

		if ( PayData.PayClass == 2 || PayData.PayClass == 3 ) {	// 中止
			// カード番号
			memset(pInfo->card.emoney.card_id, 0x20, sizeof(pInfo->card.emoney.card_id));
			memset(pInfo->card.emoney.card_id, 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]));
			memset(pInfo->card.emoney.card_id, 'Z', 2);

			// 問い合わせ番号
			memset(pInfo->card.emoney.inquiry_num, 0x20, sizeof(pInfo->card.emoney.inquiry_num));

			// 物販端末ID
			memset(pInfo->card.emoney.terminal_id, 0x20, sizeof(pInfo->card.emoney.terminal_id));
		}
		else {
			// 利用金額
			pInfo->card.emoney.amount = PayData.Electron_data.Ec.pay_ryo;

			// カード番号
			if ( (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) &&
			     (PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0) ) {
				// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.をカード桁数分0x30埋めし左詰めでZZする。
				// [*]印字みなし決済と同様の会員Noをセットする。
				memset(pInfo->card.emoney.card_id, 0x20, sizeof(pInfo->card.emoney.card_id));
				memset(pInfo->card.emoney.card_id, 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]));
				memset(pInfo->card.emoney.card_id, 'Z', 2);
			} else {
				memcpy(pInfo->card.emoney.card_id, buffer, sizeof(pInfo->card.emoney.card_id));
			}

			// カード残額
			pInfo->card.emoney.card_zangaku = PayData.Electron_data.Ec.pay_after;

			// 問い合わせ番号
			memcpyFlushLeft(pInfo->card.emoney.inquiry_num,
				PayData.Electron_data.Ec.inquiry_num,
				sizeof(pInfo->card.emoney.inquiry_num),
				sizeof(PayData.Electron_data.Ec.inquiry_num));

			// 承認番号
			switch ( PayData.Electron_data.Ec.e_pay_kind ) {
			case EC_ID_USED:
				pInfo->card.emoney.approbal_no = astoinl(PayData.Electron_data.Ec.Brand.Id.Approval_No, 7);
				break;
			case EC_QUIC_PAY_USED:
				pInfo->card.emoney.approbal_no = astoinl(PayData.Electron_data.Ec.Brand.Quickpay.Approval_No, 7);
				break;
			case EC_PITAPA_USED:
				// オフライン承認決済時はオール'*'のため、'0'へ変換する
				if ( PayData.Electron_data.Ec.Brand.Pitapa.Approval_No[0] == '*' ) {
					pInfo->card.emoney.approbal_no = 0;
				}
				else {
					pInfo->card.emoney.approbal_no = astoinl(PayData.Electron_data.Ec.Brand.Pitapa.Approval_No, 8);
				}
				break;
			default:
				pInfo->card.emoney.approbal_no = 0;
				break;
			}

			// 物販端末ID
			switch ( PayData.Electron_data.Ec.e_pay_kind ) {
			case EC_WAON_USED:
				memcpyFlushLeft(pInfo->card.emoney.terminal_id,
					PayData.Electron_data.Ec.Brand.Waon.SPRW_ID,
					sizeof(pInfo->card.emoney.terminal_id),
					sizeof(PayData.Electron_data.Ec.Brand.Waon.SPRW_ID));
				break;
			default:
				memset(pInfo->card.emoney.terminal_id, 0x20, sizeof(pInfo->card.emoney.terminal_id));
				break;
			}
		}

		break;
	case 16:	// QR
		// カード決済 端末区分
		pInfo->CardPaymentTerminalCategory = 1;

		// カード決済 取引ステータス
		if ( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ) {
			pInfo->CardPaymentTransactionStatus = 3;	// 未了支払不明
		}
		else if ( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ) {
			pInfo->CardPaymentTransactionStatus = 1;	// みなし決済
		}
		else {
			pInfo->CardPaymentTransactionStatus = 0;	// 支払い
		}

		if ( PayData.PayClass == 2 || PayData.PayClass == 3 ) {	// 中止
			// Mch取引番号
			memset(pInfo->card.qr.MchTradeNo, 0x20, sizeof(pInfo->card.qr.MchTradeNo));

			if ( PayData.Electron_data.Ec.E_Flag.BIT.deemSettleCancal == 1 ) {
				// 中止であるが決済結果を受信できなかったので
				// それぞれの桁数分0x20埋めし、支払端末IDは13桁、取引番号は15桁、0x30埋めする

				// 支払端末ID
				memset(pInfo->card.qr.PayTerminalNo, 0x20, sizeof(pInfo->card.qr.PayTerminalNo));
				memset(pInfo->card.qr.PayTerminalNo, 0x30, 13);

				// 取引番号
				memset(pInfo->card.qr.DealNo, 0x20, sizeof(pInfo->card.qr.DealNo));
				memset(pInfo->card.qr.DealNo, 0x30, 15);
			}
			else {
				// 支払端末ID
				memcpyFlushLeft(pInfo->card.qr.PayTerminalNo,
					PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo,
					sizeof(pInfo->card.qr.PayTerminalNo),
					sizeof(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo));

				// 取引番号
				memcpyFlushLeft(pInfo->card.qr.DealNo,
					PayData.Electron_data.Ec.inquiry_num,
					sizeof(pInfo->card.qr.DealNo),
					sizeof(PayData.Electron_data.Ec.inquiry_num));
			}
		}
		else {
			// 利用金額
			pInfo->card.qr.amount = PayData.Electron_data.Ec.pay_ryo;

			if ( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ) {
				// Mch取引番号
				memset(pInfo->card.qr.MchTradeNo, 0x20, sizeof(pInfo->card.qr.MchTradeNo));

				// 支払端末ID
				memcpyFlushLeft(pInfo->card.qr.PayTerminalNo,
					PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo,
					sizeof(pInfo->card.qr.PayTerminalNo),
					sizeof(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo));

				// 取引番号
				memcpyFlushLeft(pInfo->card.qr.DealNo,
					PayData.Electron_data.Ec.inquiry_num,
					sizeof(pInfo->card.qr.DealNo),
					sizeof(PayData.Electron_data.Ec.inquiry_num));
			}
			else {
				// Mch取引番号
				memcpyFlushLeft(pInfo->card.qr.MchTradeNo,
					PayData.Electron_data.Ec.Brand.Qr.MchTradeNo,
					sizeof(pInfo->card.qr.MchTradeNo),
					sizeof(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo));

				// 支払端末ID
				memset(pInfo->card.qr.PayTerminalNo, 0x20, sizeof(pInfo->card.qr.PayTerminalNo));

				// 取引番号
				memset(pInfo->card.qr.DealNo, 0x20, sizeof(pInfo->card.qr.DealNo));
			}
		}

		// 決済ブランド
		pInfo->card.qr.PayKind = PayData.Electron_data.Ec.Brand.Qr.PayKind;

		break;
	default:
// GG129004(S) R.Endo 2024/12/13 #7565 現金で精算完了しても領収証データのカード決済日時に「1980/3/3 00:00:00」がセットされる
		isCard = 0;
// GG129004(E) R.Endo 2024/12/13 #7565 現金で精算完了しても領収証データのカード決済日時に「1980/3/3 00:00:00」がセットされる
		break;
	}

	// カード決済 区分
	pInfo->CardPaymentCategory = CardKind;

	// カード決済 年月日時分秒
// GG129004(S) R.Endo 2024/12/13 #7565 現金で精算完了しても領収証データのカード決済日時に「1980/3/3 00:00:00」がセットされる
	if ( isCard ) {
// GG129004(E) R.Endo 2024/12/13 #7565 現金で精算完了しても領収証データのカード決済日時に「1980/3/3 00:00:00」がセットされる
	c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &wkClk);
	pInfo->dtCardPayment.shYear   = wkClk.year;
	pInfo->dtCardPayment.byMonth  = wkClk.mont;
	pInfo->dtCardPayment.byDay    = wkClk.date;
	pInfo->dtCardPayment.byHours  = wkClk.hour;
	pInfo->dtCardPayment.byMinute = wkClk.minu;
	pInfo->dtCardPayment.bySecond = wkClk.seco;
// GG129004(S) R.Endo 2024/12/13 #7565 現金で精算完了しても領収証データのカード決済日時に「1980/3/3 00:00:00」がセットされる
	}
// GG129004(E) R.Endo 2024/12/13 #7565 現金で精算完了しても領収証データのカード決済日時に「1980/3/3 00:00:00」がセットされる

	// 適格請求書 情報有無
	pInfo->EligibleInvoiceHasInfo = 1;

	// 適格請求書 登録番号
	memcpy(pInfo->EligibleInvoiceRegistrationNo, PayData.RegistNum, sizeof(PayData.RegistNum));

	// 適格請求書 課税対象額(税込み)
	pInfo->EligibleInvoiceTaxableAmount = PayData.WTaxPrice;

	// 適格請求書 適用税率
	pInfo->EligibleInvoiceTaxRate = PayData.WTaxRate;

	// 発券情報 RYOUSYUU_edit_hakkenj
	pInfo->EntryMachineNo = PayData.CMachineNo;	// 発券機番号 [Rev.No.1追加]
	pInfo->ParkingTicketNo = PayData.TicketNum;	// 駐車券番号 [Rev.No.1追加]

	// 現金領収額 RYOUSYUU_edit_content_sub
	if ( (PayData.WInPrice - PayData.WChgPrice) > 0 ) {
		pInfo->CashReceiptAmount = PayData.WInPrice - PayData.WChgPrice;	// 現金領収額 [Rev.No.1追加]
	}

	// 預り金額 RYOUSYUU_edit_content_sub
	pInfo->WInPrice = PayData.WInPrice;	// 預り金額 [Rev.No.1追加]

	// 釣り銭額 RYOUSYUU_edit_turisen
	pInfo->WChgPrice = PayData.WChgPrice;	// 釣り銭額 [Rev.No.1追加]

	// 釣り切れ情報 RYOUSYUU_edit_turikire
	pInfo->WFusoku = PayData.WFusoku;	// 払戻不足金額 [Rev.No.1追加]
}

//[]----------------------------------------------------------------------[]
///	@brief			QR領収証データ作成
//[]----------------------------------------------------------------------[]
///	@param[out]		data	QR領収証データの格納先
///	@param[in]		size	QR領収証データの格納先のサイズ
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2024/11/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2024 AMANO Corp.---[]
void	MakeQRCodeReceipt( char* data, size_t size )
{
	char encrypt[QRCODE_RECEIPT_ENCRYPT_SIZE + 1];
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	char buffer[QRCODE_RECEIPT_ENCRYPT_SIZE * 2];
	char buffer[15];	// 年月日時分秒が入るサイズ
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	char crc_wk[2];
	int i;

	// QR領収証データ＋CRCを格納できるかチェック
	if ( size < (QRCODE_RECEIPT_SIZE + 4) ) {
		return;
	}

	// クリア
	memset(data, '\0', size);

	// 非暗号化領域
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(data, "https://payment.parkingweb.jp/000000000000000000000000000000000000000000000000000000000000000000000/");	// URL
	if ( prm_get(COM_PRM, S_RTP, 58, 1, 6) == 1 ) {	// 17-0058① 電子領収証URL変更（デバッグ用）
		strcat(data, "https://payment.verif.parkingdev.com/00000000000000000000000000000000000000000000000000000000000000/");	// URL
	} else {
		strcat(data, "https://payment.parkingweb.jp/000000000000000000000000000000000000000000000000000000000000000000000/");	// URL
	}
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	strcat(data, "DCENTER_QR");	// 識別文字
	strcat(data, "00101");		// QRコードID
	strcat(data, "00001");		// QRコードフォーマットRev.No.
	strcat(data, "000001");		// リンクID
	strcat(data, "1");			// 暗号化方式
	strcat(data, "0");			// 予備

	// 暗号化領域
	memset(encrypt, '\0', sizeof(encrypt));

	sprintf(buffer, "%04d%02d%02d%02d%02d%02d",
		CLK_REC.year,
		CLK_REC.mont,
		CLK_REC.date,
		CLK_REC.hour,
		CLK_REC.minu,
		CLK_REC.seco);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// QR発行年月日時分秒
// 	strcat(encrypt, "0");		// 予備
	strcat(data, buffer);	// QR発行年月日時分秒
	strcat(data, "0");		// 予備
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%03d", CPrmSS[S_PKT][1]);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// 端末IPアドレス 第1オクテット
	strcat(data, buffer);	// 端末IPアドレス 第1オクテット
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%03d", CPrmSS[S_PKT][2]);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// 端末IPアドレス 第2オクテット
	strcat(data, buffer);	// 端末IPアドレス 第2オクテット
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%03d", CPrmSS[S_PKT][3]);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// 端末IPアドレス 第3オクテット
	strcat(data, buffer);	// 端末IPアドレス 第3オクテット
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%03d", CPrmSS[S_PKT][4]);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// 端末IPアドレス 第4オクテット
	strcat(data, buffer);	// 端末IPアドレス 第4オクテット
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%06d", CPrmSS[S_SYS][1]);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// 駐車場№
	strcat(data, buffer);	// 駐車場№
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%03d", RTReceipt_Data.receipt_info.ModelCode);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// 機種コード
	strcat(data, buffer);	// 機種コード
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%04d", RTReceipt_Data.receipt_info.MachineNo);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// 機械№
	strcat(data, buffer);	// 機械№
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%010d", RTReceipt_Data.CenterOiban);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// センター追番
	strcat(data, buffer);	// センター追番
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	sprintf(buffer, "%04d%02d%02d%02d%02d%02d",
		RTReceipt_Data.receipt_info.dtTimeYtoSec.shYear,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.byMonth,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.byDay,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.byHours,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.byMinute,
		RTReceipt_Data.receipt_info.dtTimeYtoSec.bySecond);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	strcat(encrypt, buffer);	// 処理年月日時分秒
// 	strcat(encrypt, "000000000000000");	// 暗号化調整バイト
	strcat(data, buffer);	// 処理年月日時分秒
	strcat(data, "0000000000000000");	// 暗号化調整バイト
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない

	// 暗号化
	EncryptWithKeyAndIV_NoPadding(
		CRYPT_KEY_QRRECEIPT,
		(uchar *)&data[QRCODE_RECEIPT_HEADER_SIZE],
		(uchar *)encrypt,
		sizeof(encrypt));

	// ASCII変換
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	for ( i = 0; i < QRCODE_RECEIPT_ENCRYPT_SIZE; i++ ) {
// 		hxtoas((uchar*)&buffer[i*2], (uchar)encrypt[i]);
// 	}
// 	memcpy(&data[QRCODE_RECEIPT_HEADER_SIZE], buffer, (QRCODE_RECEIPT_ENCRYPT_SIZE * 2));
	for ( i = 0; i < QRCODE_RECEIPT_ENCRYPT_SIZE; i++ ) {
		hxtoas((uchar*)&data[QRCODE_RECEIPT_HEADER_SIZE+(i*2)], (uchar)encrypt[i]);
	}
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない

	// CRC算出
	crc_ccitt((ushort)QRCODE_RECEIPT_SIZE, (uchar *)data, (uchar *)crc_wk, (uchar)R_SHIFT);
// GG129004(S) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
// 	sprintf(buffer, "%02x%02x", crc_wk[1], crc_wk[0]);
	sprintf(buffer, "%02X%02X", crc_wk[1], crc_wk[0]);
// GG129004(E) R.Endo 2024/12/17 #7567 電子領収証の暗号化部分に何も情報がセットされていない
	memcpy(&data[QRCODE_RECEIPT_SIZE], buffer, 4);
}
// GG129004(E) R.Endo 2024/12/10 電子領収証対応

// MH810100(S) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
//[]----------------------------------------------------------------------[]
///	@brief			入庫情報で受信した精算情報セット処理
//[]----------------------------------------------------------------------[]
///	@param			none
//[]----------------------------------------------------------------------[]
///	@return			none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void	Set_PayInfo_from_InCarInfo( void )
{
	ushort	cnt;
	uchar	card_info[16];
// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
	stMasterInfo_t* pMaster;
	stZaishaInfo_t* pZaisha;
	ushort	index;
// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更

	// 駐車券機械№セット
	OpeNtnetAddedInfo.CMachineNo = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.MachineNo;	// 駐車券機械№ = 入庫端末機械№
	// NT-NET電文上、駐車券機械№は1Byteなので256以上は0とする
	if( OpeNtnetAddedInfo.CMachineNo <= 255 ){
		PayData.CMachineNo	= (uchar)OpeNtnetAddedInfo.CMachineNo;	// 駐車券機械№
	}else{
		PayData.CMachineNo	= 0;									// 駐車券機械№
	}
	NTNET_Data152Save((void *)&PayData.CMachineNo, NTNET_152_CMACHINENO);

	PayData.MediaKind1 = 0;	// 精算媒体種別クリア
	memset( PayData.MediaCardNo1, 0, sizeof(PayData.MediaCardNo1) );
	memset( card_info, 0, sizeof(card_info) );
// GG129000(S) T.Nagai 2023/01/16 ゲート式車番チケットレスシステム対応（発券情報印字対応）
	PayData.TicketNum = 0;
	for( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ){
		if( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType == CARD_TYPE_PARKING_TKT ){	// 精算媒体種別 = 駐車券
			PayData.MediaKind1 = CARD_TYPE_PARKING_TKT;
			memcpy( PayData.MediaCardNo1, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo, sizeof(PayData.MediaCardNo1) );
			PayData.TicketNum = astoinl(&PayData.MediaCardNo1[3], 6);
// GG129000(S) H.Fujinaga 2023/02/18 ゲート式車番チケットレスシステム対応（QR駐車券対応）
			memcpy( PayData.QrTicketInfo.EntryMachineNo, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo, sizeof(PayData.QrTicketInfo.EntryMachineNo) );
			memcpy( PayData.QrTicketInfo.ParkingTicketNo, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo[sizeof(PayData.QrTicketInfo.EntryMachineNo)], sizeof(PayData.QrTicketInfo.ParkingTicketNo) );
			PayData.QrTicketInfo.TicketLessMode[0] = 0;
// GG129000(E) H.Fujinaga 2023/02/18 ゲート式車番チケットレスシステム対応（QR駐車券対応）
			break;
		}
	}
// GG129000(E) T.Nagai 2023/01/16 ゲート式車番チケットレスシステム対応（発券情報印字対応）
	if( PayData.teiki.id ){	// 定期券精算
		// 定期券関連媒体情報セット
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:225)対応
//		for( cnt = 0; cnt < ONL_MAX_MEDIA_NUM; cnt++ ){
		for( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ){
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:225)対応
			if( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType == CARD_TYPE_PARKING_TKT ){	// 精算媒体種別 = 駐車券
				PayData.MediaKind1 = CARD_TYPE_PASS;
// GG129000(S) T.Nagai 2023/01/16 ゲート式車番チケットレスシステム対応（発券情報印字対応）
				memset( PayData.MediaCardNo1, 0, sizeof(PayData.MediaCardNo1) );
// GG129000(E) T.Nagai 2023/01/16 ゲート式車番チケットレスシステム対応（発券情報印字対応）
				intoasl( PayData.MediaCardNo1, PayData.teiki.id, 5 );	// カード番号(定期券ID(5桁、0～12000))
				intoas( card_info, PayData.teiki.syu, 2 );				// カード情報(定期券種別(2桁、1～15))
				break;
			}
		}
// GG129000(S) T.Nagai 2023/01/16 ゲート式車番チケットレスシステム対応（発券情報印字対応）
//	}else{
//		// 駐車券関連媒体情報セット
//// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:226,227)対応
////		for( cnt = 0; cnt < ONL_MAX_MEDIA_NUM; cnt++ ){
//		for( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ){
//// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:226,227)対応
//			if( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType == CARD_TYPE_PARKING_TKT ){	// 精算媒体種別 = 駐車券
//				PayData.MediaKind1 = CARD_TYPE_PARKING_TKT;
//				memcpy( PayData.MediaCardNo1, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo, sizeof(PayData.MediaCardNo1) );
//				break;
//			}
//		}
// GG129000(E) T.Nagai 2023/01/16 ゲート式車番チケットレスシステム対応（発券情報印字対応）
	}
	NTNET_Data152Save( (void *)&PayData.MediaKind1, NTNET_152_MEDIAKIND1 );
	NTNET_Data152Save( (void *)PayData.MediaCardNo1, NTNET_152_MEDIACARDNO1 );
	NTNET_Data152Save( (void *)card_info, NTNET_152_MEDIACARDINFO1 );
	// 車番関連媒体情報セット
	PayData.MediaKind2 = 0;	// 精算媒体種別クリア
	memset( PayData.MediaCardNo2, 0, sizeof(PayData.MediaCardNo2) );
	memset( card_info, 0, sizeof(card_info) );
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:228,229)対応
//	for( cnt = 0; cnt < ONL_MAX_MEDIA_NUM; cnt++ ){
	for( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ){
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:228,229)対応
		if( lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType == CARD_TYPE_CAR_NUM ){	// 精算媒体種別 = 車番
			PayData.MediaKind2 = CARD_TYPE_CAR_NUM;
			NTNET_Data152Save( (void *)&PayData.MediaKind2, NTNET_152_MEDIAKIND2 );
			memcpy( PayData.MediaCardNo2, &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[cnt].byCardNo, sizeof(PayData.MediaCardNo2) );
			NTNET_Data152Save( (void *)PayData.MediaCardNo2, NTNET_152_MEDIACARDNO2 );
			NTNET_Data152Save( (void *)card_info, NTNET_152_MEDIACARDINFO2 );
			break;
		}
	}

// GG124100(S) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
	// 車番情報
	memcpy(&PayData.ShabanInfo, &lcdbm_rsp_in_car_info_main.ShabanInfo, sizeof(SHABAN_INFO));
// GG124100(E) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット

// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
	// NT-NET精算データの駐車券/定期券は一時利用フラグに準拠する為、
	// 必要なカード情報を常に保持しておく。
	pMaster = &(lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo);
	pZaisha = &(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo);

	// 一時利用フラグ
	ntNet_56_SaveData.NormalFlag = pZaisha->NormalFlag;

	// 駐車券カード情報
	for ( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ) {
		if ( pMaster->stCardDataInfo[cnt].CardType == CARD_TYPE_PARKING_TKT ) {	// 駐車券
			// 駐車券機械№(入庫機械№)(上位3桁、0～255)、駐車券番号(下位6桁、0～999999)
			memcpy(ntNet_56_SaveData.MachineTktNo,
				&(pMaster->stCardDataInfo[cnt].byCardNo),
				sizeof(ntNet_56_SaveData.MachineTktNo));

			break;
		}
	}

	// 定期券カード情報
	for ( cnt = 0; cnt < ONL_MAX_CARDNUM; cnt++ ) {
		if ( (pMaster->stCardDataInfo[cnt].CardType == CARD_TYPE_PASS) &&		// 定期券
			 (pMaster->stCardDataInfo[cnt].ParkingLotNo) ) {					// 定期券駐車場№あり
			for ( index = 0; index < 4; index++ ) {
				// 定期券駐車場№が基本/拡張駐車場№と一致するか確認する。
				if ( pMaster->stCardDataInfo[cnt].ParkingLotNo == CPrmSS[S_SYS][index+1] ) {
					// 定期券駐車場№
					ntNet_56_SaveData.ParkingNo = pMaster->stCardDataInfo[cnt].ParkingLotNo;

					// 定期券ID(5桁、0～12000)
					ntNet_56_SaveData.id = astoin(pMaster->stCardDataInfo[cnt].byCardNo, 5);

					// 定期券種別(2桁、1～15)
					ntNet_56_SaveData.syu = pMaster->SeasonKind;

					break;
				}
			}

			break;
		}
	}
// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更

// GG132000(S) 精算完了／中止時のNT-NET通信の精算データにデータ項目を追加
	ntNet_56_SaveData.MaxFeeApplyFlg	= lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.MaxFeeApplyFlg;
	ntNet_56_SaveData.MaxFeeApplyCnt	= lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.MaxFeeApplyCnt;
	ntNet_56_SaveData.MaxFeeSettingFlg	= lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.MaxFeeSettingFlg;
// GG132000(E) 精算完了／中止時のNT-NET通信の精算データにデータ項目を追加
}
// MH810100(E) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)

/*[]----------------------------------------------------------------------[]*/
/*| 待機                                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod00( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = 他処理へ切換                              |*/
/*|                       : -1 = ﾓｰﾄﾞ切替                                  |*/
/*|                       : 10 = 取消(中止)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod00( void )
{
	short	ret = 0;
	ushort	msg = 0;
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス(不要処理削除)
//	uchar	key_chk;
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス(不要処理削除)
	short	tim2_mov = 0;
	uchar	uc_buf_size;
	uchar	reci_sw_flg = 0;
// MH810103(s) 電子マネー対応 静的解析
//	ushort	e_pram_set;												// 電子ﾏﾈｰ使用設定
	ushort	e_pram_set = 0;												// 電子ﾏﾈｰ使用設定
// MH810103(e) 電子マネー対応 静的解析
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//	uchar	MntLevel = 0;
//	uchar	PassLevel = 0;
//	uchar	KakariNo = 0;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
	short	ans;
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//	short	wk_lev;
//	char	f_Card = 0;												// 0:不正係員ｶｰﾄﾞ 1:係員ｶｰﾄﾞ受け -1:係員ｶｰﾄﾞNG
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	uchar	rd_err_count = 0;
	ushort	tim_mandet;												// 人体検出信号マスク時間
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//	ushort	lcdGuardTm = 0;
//	ushort	lcdGuardInterval = prm_get(COM_PRM, S_PAY, 62, 2, 1);	// スクリーンセーバ実行間隔(分)
//	ushort	lcdGuardSpan = 25;										// スクリーンセーバ実行長500ms(x20ms)
	ushort	res_wait_time;

	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// データ受信監視タイマ
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// 範囲外は5秒とする
	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH321800(S) Y.Tanizaki ICクレジット対応
#if (6 == AUTO_PAYMENT_PROGRAM)
	ushort	auto_pay_timer = (ushort)prm_get(COM_PRM, S_ECR, 100, 3, 1);
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応

	OPECTL.ChkPassSyu = 0;											// 定期券問合せ中券種ｸﾘｱ
	RD_Credit_kep = 0;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// 未了タイムアウトフラグクリア
	OPECTL.op_faz = 0;												// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ
	an_vol_flg = 0;
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
// MH810105(S) MH364301 インボイス対応
	f_reci_ana = 0;
// MH810105(E) MH364301 インボイス対応
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
	GyouCnt_All_r = 0;
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	GyouCnt_All_j = 0;
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129001(S) インボイス対応（フラグのクリア処理）
	OPECTL.f_RctErrDisp = 0;
// GG129001(E) インボイス対応（フラグのクリア処理）
	if( OPECTL.Pay_mod != 2 ){										// 修正精算処理?
		OpeLcd( 1 );												// 待機画面表示
	}
	rd_shutter();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
	OPECTL.remote_wait_flg = 0;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
	OPECTL.f_rtm_remote_pay_flg = 0;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	LaneStsSeqNoUpdate();
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

	Lagtim( OPETCBNO, 18, RD_ERR_INTERVAL );						// ﾀｲﾏｰ18(10s)ｼｬｯﾀｰ異常判定用ﾀｲﾏｰ

#if (4 == AUTO_PAYMENT_PROGRAM)
	OPECTL.Seisan_Chk_mod = ON;
#endif

	if( OPECTL.Seisan_Chk_mod == ON ){								// 精算ﾁｪｯｸﾓｰﾄﾞ？
// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
	// 入庫状態にしない
#else
		op_indata_set();											// 入庫状態ｾｯﾄ
#endif
	}
// MH321800(S) Y.Tanizaki ICクレジット対応
#if (6 == AUTO_PAYMENT_PROGRAM)
	is_auto_pay = 0;
	// 自動精算開始タイマー
	if(auto_pay_timer == 0) {
		auto_pay_timer = 5;
	}
	auto_pay_timer *= 50;
	Lagtim( OPETCBNO, TIMERNO_AUTO_PAYMENT_TIMEOUT, auto_pay_timer );
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応
	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
	}

	if( MIFARE_CARD_DoesUse ){										// Mifareが有効な場合
		op_MifareStop_with_LED();								 	// Mifareを停止する
	}

	if( OPECTL.NtnetTimRec ){										// NT-NET時計受信ﾌﾗｸﾞON
		NTNET_Snd_Data104(NTNET_MANDATA_CLOCK);						// NT-NET管理ﾃﾞｰﾀ要求作成
		OPECTL.NtnetTimRec = 0;										// NT-NET時計受信ﾌﾗｸﾞOFF
	}

	key_num = 0;													// ｷｰ入力値ｸﾘｱ
	if( CPrmSS[S_TYP][81] ){										// 区画使用設定有り?
		key_sec = 1;												// 区画を[Ａ]とする(区画使用する)
	}else{
		key_sec = 0;												// 区画入力値ｸﾘｱ(区画使用しない)
	}

	Lagtim( OPETCBNO, 1, 10*50 );									// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	tim1_mov = 1;													// ﾀｲﾏｰ1起動中
	if(( OPECTL.RECI_SW == (char)-1 )&&( RECI_SW_Tim == 0 )){		// 待機時領収証ﾎﾞﾀﾝ可?
		RECI_SW_Tim = (30 * 2) + 1;									// 領収証受付時間は30s固定(500msにてｶｳﾝﾄ)
	}
	Lagtim( OPETCBNO, 4, 1*50 );									// ﾀｲﾏｰ4(1s)起動(不正強制出庫監視用)初回のみ1sとする
	Lagtim( OPETCBNO, 26, 3*50 );									// ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ(3s)START
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//	dspErrMsg = 0;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

	OPECTL.Ent_Key_Sts = OFF;										// 登録ｷｰ押下状態：ＯＦＦ
	OPECTL.Can_Key_Sts = OFF;										// 取消ｷｰ押下状態：ＯＦＦ
	OPECTL.EntCan_Sts  = OFF;										// 登録ｷｰ＆取消ｷｰ同時押下状態：ＯＦＦ
	OPECTL.Apass_off_seisan = 0;									// 強制ｱﾝﾁﾊﾟｽOFFでの定期利用ﾌﾗｸﾞOFF

	Chu_Syu_Status = 0;
	memset( DspWorkerea, 0, sizeof( DspWorkerea ));							// ﾜｰｸｴﾘｱの初期化
	memset( DspChangeTime, 0, sizeof( DspChangeTime ));						// ﾜｰｸｴﾘｱの初期化
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
	memset( &g_PipCtrl, 0, sizeof(g_PipCtrl) );
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
// MH810100(S) 2020/06/17 #4497【連動評価指摘事項】Ai-Ⅴより入庫時刻あり割引ありの遠隔精算を行うと、不正な（前回精算時に適応した）割引を再適応して精算してしまう
	memset( &lcdbm_rsp_in_car_info_main, 0, sizeof(lcdbm_rsp_in_car_info_t) );
// MH810100(E) 2020/06/17 #4497【連動評価指摘事項】Ai-Ⅴより入庫時刻あり割引ありの遠隔精算を行うと、不正な（前回精算時に適応した）割引を再適応して精算してしまう
// MH810105(S) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
	memset( &RTPay_Data, 0, sizeof(RTPay_log) );					// RT精算データクリア
// MH810105(E) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	memset( &RTReceipt_Data, 0, sizeof(RTReceipt_log) );			// RT領収証データクリア
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

// MH810103(s) 電子マネー対応 不要機能削除
//// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////	Card_Status_Prev = 0;
////	if( ((e_pram_set = (ushort)prm_get(COM_PRM, S_PAY, 24, 1, 3)) == 1 ) ||
////		((e_pram_set = (ushort)prm_get(COM_PRM, S_PAY, 24, 1, 4)) == 1 )) {
//	if( (e_pram_set = (ushort)prm_get(COM_PRM, S_PAY, 24, 1, 3)) == 1 ) {
//// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		Ope_EleUseDsp();
//	}
//// MH321800(S) hosoda ICクレジット対応
//	else if(isEC_USE()) {
// MH810105(S) MH364301 QRコード決済対応 GT-4100
	e_pram_set = 0;
// MH810105(E) MH364301 QRコード決済対応 GT-4100
	if(isEC_USE()) {
// MH810103(e) 電子マネー対応 不要機能削除
		e_pram_set = 1;
		Ope_EcEleUseDsp();
// MH810103(s) 電子マネー対応 #5413 精算不可画面または警告画面表示時に、VPが残高照会ボタンを表示してしまっている
		// mod00ため通知
		LcdBackLightCtrl(ON);
// MH810103(e) 電子マネー対応 #5413 精算不可画面または警告画面表示時に、VPが残高照会ボタンを表示してしまっている
	}
// MH321800(E) hosoda ICクレジット対応
// MH810104(S) MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
	// まだ直前取引データ処理をしていなければ処理する
	if( ac_flg.ec_recv_deemed_fg != 0 ){
		EcRecvDeemed_RegistPri();
	}
// MH810104(E) MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
	if( OPECTL.Pay_mod == 2 ){									// 修正精算処理?
		OPECTL.Ope_mod = 11;									// 修正精算駐車位置番号入力処理
		return(1);
	}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//	if(( key_num = Key_Event_Get() )){
//		queset( OPETCBNO, DUMMY_KEY_EVENT, 0, NULL );
//	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
	// NOTE:人体検知信号検出処理を加える場合はこれ以降に加えること
	tim_mandet = (ushort)prm_get(COM_PRM, S_PAY, 32, 2, 3);			// 人体検出信号マスク時間
	if(tim_mandet != 0 && f_MandetMask == 1 ){
		Lagtim( OPETCBNO, TIMER_MANDETMASK, (ushort)(tim_mandet*50) );	// 人体検知センサーマスクタイマー開始
	}else{
		f_MandetMask = 0;
	}
	
	if(0 == f_MandetMask && 1 == FNT_MAN_DET) {					// マスクOFFかつ人体検知 ON
		goto	op_mod00_MANDET_ON;
	}
// MH810100(S) K.Onodera 2020/04/09 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
	// 精算状態通知
	dspCyclicErrMsgRewrite();
// MH810100(E) K.Onodera 2020/04/09 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
// MH810104(S) nanaco・iD・QUICPay対応2
//// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//	// まだ直前取引データ処理をしていなければ処理する
//	if( ac_flg.ec_recv_deemed_fg != 0 ){
//		EcRecvDeemed_RegistPri();
//	}
//// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// MH810104(E) nanaco・iD・QUICPay対応2
// GG129000(S) H.Fujinaga 2022/12/09 ゲート式車番チケットレスシステム対応（#6411 電子マネー設定なしの設定でサービス券(QR)で精算完了時に領収証発行ボタンがタッチ可能になっている）
	// 電子ジャーナル接続状態を通知
	lcdbm_notice_ope(LCDBM_OPCD_EJA_USE,isEJA_USE());
// GG129000(E) H.Fujinaga 2022/12/09 ゲート式車番チケットレスシステム対応（#6411 電子マネー設定なしの設定でサービス券(QR)で精算完了時に領収証発行ボタンがタッチ可能になっている）

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		if( Op_Event_Disable( msg ) )								// イベント抑止条件チェック
			continue;
		Ope_ArmClearCheck( msg );

		switch( msg ){
			case KEY_DOOR1:											// Door1 key event
			case KEY_DOOR1_DUMMY:									// Edy自動センター通信中のドア閉イベント
				if( OPECTL.on_off == 0 ){							// OFF(Door Close)
					if( DoorCloseChk() < 100 ){						// ﾄﾞｱ閉でﾁｪｯｸ NG
						ret = -1;
						break;
					}
				}
				break;
			case CLOCK_CHG:											// 時計更新
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( edy_auto_com ){
//					queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL ); // 営休業変化通知登録
//				}else{
				{
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					// インデント調整の為、以下の処理にタブを入れる。
					if( opncls() == 2 ){								// 休業?
						ret = -1;
						break;
					}
				}
				dspclk(0, COLOR_DARKSLATEBLUE);						// 時計表示
				if( tim1_mov == 0 ){								// ﾀｲﾏｰ1未起動
					LcdBackLightCtrl( OFF );						// back light OFF
				}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//				if(lcdGuardInterval != 0){
//					lcdGuardTm++;
//					if( lcdGuardTm >= lcdGuardInterval ){			// スクリーンセーバ実行間隔
//						dispclr();
//						lcdGuardTm = 0;
//						Lagtim( OPETCBNO, 28, lcdGuardSpan );		// スクリーンセーバ実行長
//					}
//				}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
				break;

			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}else{
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
				}
				ret = -1;
				fus_subcal_flg = 0;
				break;

			case OPE_OPNCLS_EVT:									// 強制営休業 event
				if( opncls() == 2 ){								// 休業?
					ret = -1;
				}
// MH810105(S) MH364301 インボイス対応
				if (f_reci_ana) {
					// 領収証ボタン押下済みであれば、
					// レシート印字失敗判定を行う
					op_rct_failchk();
				}
// MH810105(E) MH364301 インボイス対応
				break;

			case MID_STSCHG_CHGEND:									// 釣り切れｱﾗｰﾑ変化
			case MID_STSCHG_R_PNEND:								// ﾚｼｰﾄ紙切れ変化
			case MID_STSCHG_J_PNEND:								// ｼﾞｬｰﾝﾙ紙切れ変化
				if( DspSts == LCD_WMSG_OFF ){						// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示状態がＯＦＦ
					dspCyclicErrMsgRewrite();
				}
			case IBK_LPR_ERR_REC:									// ﾗﾍﾞﾙﾌﾟﾘﾝﾀｴﾗｰﾃﾞｰﾀ受信
				LCDNO = (ushort)-1;
				OpeLcd( 1 );										// 待機画面再表示
				break;

// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(不要処理削除)
//			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
//			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
//			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
//			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
//			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
//			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
//			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
//			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
//			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
//			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
//			case DUMMY_KEY_EVENT:									// 精算完了画面からのキー入力
//				if (tim2_mov) {
//					tim2_mov = 0;									// F1ｷｰが離されたらﾌﾗｸﾞをｸﾘｱ
//					Lagcan( OPETCBNO, 2 );							// F1ｷｰ押下監視ﾀｲﾏ
//				}
//				if( auto_syu_prn == 2 || 							// 自動集計中？
//// MH322914 (s) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
////					(AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) ){	// 精算ログ書き込み中
//					(AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) || // 精算ログ書き込み中
//					( ryo_inji != 0 ) ){	 						// 領収証印字中
//// MH322914 (e) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
//					// 自動集計中は精算操作不可とする為、ﾃﾝｷｰの入力を規制する
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				LcdBackLightCtrl( ON );								// back light ON
//				if( msg != DUMMY_KEY_EVENT )
//				key_num = msg - KEY_TEN;
//				OPECTL.Ope_mod = 1;									// 駐車位置番号入力処理へ
//				ret = 1;											// 次操作
//				break;
//
//			case KEY_TEN_CL:										// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
//				if( auto_syu_prn == 2 || 							// 自動集計中
//					OPECTL.Pay_mod == 1 ||							// 暗証番号忘れ精算ﾓｰﾄﾞ時
//// MH322914 (s) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
////					(AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) ){	// 精算ログ書き込み中
//					(AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) || // 精算ログ書き込み中
//					( ryo_inji != 0 ) ){	 						// 領収証印字中
//// MH322914 (e) kasiyama 2016/07/13 領収証印字中は精算開始させない[共通バグNo.1275](MH341106)
//					// 自動集計中は精算操作不可とする為、ﾃﾝｷｰの入力を規制する
//					BUZPIPI();
//					break;
//				}
//				if( key_sec ){										// 区画使用する?
//					OPECTL.Ope_mod = 1;								// 駐車位置番号入力処理へ
//					BUZPI();
//					LcdBackLightCtrl( ON );							// back light ON
//					ret = 1;										// 次操作
//					break;
//				}
//				if ((CPrmSS[S_PAY][41] % 10) != 0) {
//					if( tim2_mov == 1 ){
//						BUZPI();
//						LcdBackLightCtrl( ON );						// back light ON
//						OPECTL.Ope_mod = 200;						// 暗証番号(Btype)入力操作へ
//						ret = 1;
//						break;
//					}
//				}
//
//#if	UPDATE_A_PASS
//				if( CPrmSS[S_KOU][1] ){								// 定期券更新機能あり?
//					if( !( LprnErrChk( 1 ) && prm_get( COM_PRM,S_KOU,26,1,1 ) ) ){
//						BUZPI();
//						LcdBackLightCtrl( ON );						// back light ON
//						// ｴﾗｰなしかｴﾗｰ時でも精算可の設定の時のみ定期券更新を許可する
//						OPECTL.Ope_mod = 21;						// 定期券更新 券挿入処理へ
//						ret = 1;									// 次操作
//						break;
//					}
//				}
//#endif	// UPDATE_A_PASS
//				BUZPIPI();
//				LcdBackLightCtrl( ON );								// back light ON
//				Lagtim( OPETCBNO, 1, 10*50 );						// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				tim1_mov = 1;										// ﾀｲﾏｰ1起動中
//				break;
//
//			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
//				if (tim2_mov) {
//					tim2_mov = 0;									// F1ｷｰが離されたらﾌﾗｸﾞをｸﾘｱ
//					Lagcan( OPETCBNO, 2 );							// F1ｷｰ押下監視ﾀｲﾏ
//				}
//				Lagtim( OPETCBNO, 1, 10*50 );						// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				tim1_mov = 1;										// ﾀｲﾏｰ1起動中
//				if( OPECTL.RECI_SW == (char)-1 ){					// 待機時領収証ﾎﾞﾀﾝ可?
//					BUZPI();
//					LcdBackLightCtrl( ON );								// back light ON
//					ryo_isu( 0 );									// 領収証発行
//					OPECTL.RECI_SW = 0;								// 領収書ﾎﾞﾀﾝ未使用
//
//					RECI_SW_Tim = 0;								// 領収証受付時間ｸﾘｱ
//					reci_sw_flg = 1;								// 領収書発行不可
//// MH810105(S) MH364301 インボイス対応
//					f_reci_ana = 1;									// 延長中に領収証発行
//// MH810105(E) MH364301 インボイス対応
//				}
//				else {
//					BUZPIPI();
//					LcdBackLightCtrl( ON );							// back light ON
//				}
//				break;
//			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
//			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
//			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
//				if (tim2_mov) {
//					tim2_mov = 0;									// F1ｷｰが離されたらﾌﾗｸﾞをｸﾘｱ
//					Lagcan( OPETCBNO, 2 );							// F1ｷｰ押下監視ﾀｲﾏ
//				}
//				if ((CPrmSS[S_PAY][41] % 10) != 0) {
//					if( msg == KEY_TEN_F5 ){		// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
//						//暗証番号(Btype)出庫を使用する設定であればﾀｲﾏｽﾀｰﾄ
//						Lagtim( OPETCBNO, 2, 3*50 );					// ﾀｲﾏｰ2(3s)起動(F1ｷｰ3s押下監視用)
//						tim2_mov = 1;
//					}
//				}
//				BUZPIPI();
//				LcdBackLightCtrl( ON );								// back light ON
//				Lagtim( OPETCBNO, 1, 10*50 );						// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				tim1_mov = 1;										// ﾀｲﾏｰ1起動中
//				break;
//			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
//				fus_subcal_flg = 0;
//				if (tim2_mov) {
//					tim2_mov = 0;									// F1ｷｰが離されたらﾌﾗｸﾞをｸﾘｱ
//					Lagcan( OPETCBNO, 2 );							// F1ｷｰ押下監視ﾀｲﾏ
//				}
//				if( OPECTL.Pay_mod == 1 ){							// ﾊﾟｽﾜｰﾄﾞ忘れ処理?
//					BUZPI();
//					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
//					OPECTL.Pay_mod = 0;								// 通常精算
//					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					ret = -1;
//				}else{
//					BUZPIPI();
//					LcdBackLightCtrl( ON );							// back light ON
//					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//					tim1_mov = 1;									// ﾀｲﾏｰ1起動中
//				}
//				StackCardEject(3);									/* カード詰まり時の排出処理 */
//				break;
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(不要処理削除)

// MH810100(S) Y.Yamauchi 2020/03/05 #3901 バックライトの点灯/消灯が設定通りに動作しない
			case KEY_TEN_CL:
				if( OPECTL.on_off == 1 ){
					LcdBackLightCtrl( ON );
					Lagtim( OPETCBNO, 1, 10*50 );						// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					tim1_mov = 1;										// ﾀｲﾏｰ1起動中
				}
				break;
// MH810100(E) Y.Yamauchi 2020/03/05 #3901 バックライトの点灯/消灯が設定通りに動作しない

			case KEY_MANDET:										// 人体検知ｾﾝｻｰ入力 ｲﾍﾞﾝﾄ
				if( f_MandetMask ){
					break;											// マスク中はイベント無視
				} else if(tim_mandet != 0){// 人体検知センサーマスク時間有
					Lagtim( OPETCBNO, TIMER_MANDETMASK, (ushort)(tim_mandet*50) );	// 人体検知センサーマスクタイマー開始
					f_MandetMask = 1;
				}
op_mod00_MANDET_ON:
				LcdBackLightCtrl( ON );								// back light ON
				Lagtim( OPETCBNO, 1, 10*50 );						// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				tim1_mov = 1;										// ﾀｲﾏｰ1起動中
				ope_anm( AVM_AUX );									// 人体検知時(ﾃﾝｷｰ以外を押下時)ｱﾅｳﾝｽ
				break;

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
//				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
//				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
//				if (((m_kakari *)&MAGred[MAG_ID_CODE])->kkr_type == 0x20) {		// 係員カード
//				f_Card = Kakariin_card();
//				if( 1 == f_Card ) {
//					if(CPrmSS[S_PAY][17]) {							// 電磁ロックあり？
//						Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
//						// 電磁ロック解除レベル以上の係員カード挿入時はロック解除
//						if(PassLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
//							LcdBackLightCtrl( ON );					// ﾊﾞｯｸﾗｲﾄ ON
//							OPE_SIG_OUT_DOORNOBU_LOCK(0);			// ﾄﾞｱﾉﾌﾞのﾛｯｸ解除
//							wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ﾓﾆﾀ登録(アンロック)
//							Ope2_WarningDisp( 6*2, sercret_Str[3]);	// ｱﾝﾛｯｸｶﾞｲﾀﾞﾝｽ表示
//							if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰ値が有効範囲内
//								DoorLockTimer = 1;
//								Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｽﾀｰﾄ
//							}
//						}
//						// 電磁ロック解除レベル未満の場合は何もしないで待機画面のまま
//						continue;
//					}
//				}
//				else if( 0 == f_Card ) {							// 不正カード（未登録）
//					Ope2_WarningDisp( 6*2, ERR_CHR[3]);				// 不正ｶﾞｲﾀﾞﾝｽ表示
//				}
//				else {												// カードNG
//					Ope2_WarningDisp( 6*2, ERR_CHR[1]);				// 不正ｶﾞｲﾀﾞﾝｽ表示
//				}
//				}
//				else {
//				wk_lev = hojuu_card();
//				if( NG != wk_lev && UsMnt_mnyctl_chk() ){			// 補充ｶｰﾄﾞﾁｪｯｸ
//					// 補充ｶｰﾄﾞの場合
//					LcdBackLightCtrl( ON );							// ﾊﾞｯｸﾗｲﾄ ON
//					OPECTL.Mnt_mod = 4;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ←補充ｶｰﾄﾞﾒﾝﾃﾅﾝｽﾓｰﾄﾞ
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					OPECTL.Mnt_lev = (char)wk_lev;
//					Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
//					OPECTL.Kakari_Num = KakariNo;					// 係員No.をセット
//					ret = -1;										// ﾓｰﾄﾞ切替
//				}
//				}
//				break;
//
//			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
//				Ope2_WarningDispEnd();								// ｶﾞｲﾀﾞﾝｽ消去
//				if( RD_mod < 9 ){
//					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
//				}
//				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				fus_subcal_flg = 0;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( edy_dsp.BIT.edy_zangaku_dsp )					// 残額表示中の場合
//					break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				tim1_mov = 0;										// ﾀｲﾏｰ1未起動
				LcdBackLightCtrl( OFF );							// back light OFF
				LedReq( CN_TRAYLED, LED_OFF );						// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED消灯
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( Edy_Rec.edy_status.BIT.ZAN_SW ){
//					Edy_StopAndLedOff();							// ｶｰﾄﾞ検知停止＆LED OFF
//					Edy_Rec.edy_status.BIT.ZAN_SW = 0;				// 残額照会ﾎﾞﾀﾝ押下済みﾘｾｯﾄ
//					DspWorkerea[3] = 0;								// 6行目に空白表示する
//					grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]] );	// 6行目に画面切替用表示をする
//				}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				break;
			case TIMEOUT2:										// ﾀｲﾏｰ2ﾀｲﾑｱｳﾄ(領収証釦受付監視用)
				if ((CPrmSS[S_PAY][41] % 10) != 0) {					// 解除番号あり
					//暗証番号(Btype)出庫を使用する設定であればﾀｲﾏｽﾀｰﾄ
					if (tim2_mov == 1) {
						// F1監視ﾀｲﾏ起動中であれば再起動
						Lagtim( OPETCBNO, 2, 3*50 );					// ﾀｲﾏｰ2(3s)起動(F1ｷｰ3s押下監視用)
					}
				}
				break;
			case MID_RECI_SW_TIMOUT:								// 領収証発行可能ﾀｲﾏｰﾀｲﾑｱｳﾄ通知
				OPECTL.RECI_SW = 0;									// 領収書ﾎﾞﾀﾝ未使用
				reci_sw_flg = 1;									// 領収書発行不可
				break;

			case TIMEOUT4:											// ﾀｲﾏｰ4ﾀｲﾑｱｳﾄ
				Lagtim( OPETCBNO, 4, 5*50 );						// ﾀｲﾏｰ4(5s)起動(不正強制出庫監視用)
				break;

// MH810100(S) K.Onodera 2019/10/17 車番チケットレス(不要処理削除)
//			case TIMEOUT6:											// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ：「登録」ｷｰ＆「取消」ｷｰ同時押し状態監視ﾀｲﾏｰ
//
//				if( READ_SHT_flg == 1 ){							// ｼｬｯﾀｰ状態？
//					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
//						// 磁気リーダなしの場合はシャッター開しない
//						break;
//					}
//					Lagtim( OPETCBNO, 7, 30*50 );					// ﾀｲﾏｰ7（ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ）起動（３０秒）
//					read_sht_opn();									// 閉の場合：開する
//				}
//				else{
//					read_sht_cls();									// 開の場合：閉する
//				}
//				break;
//			case TIMEOUT7:											// ﾀｲﾏｰ7ﾀｲﾑｱｳﾄ：ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ
//				rd_shutter();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
//				break;
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス(不要処理削除)
			case TIMEOUT11:														// Suica用ﾀｲﾏｰ
				Ope_TimeOut_11( OPECTL.Ope_mod,  e_pram_set );
				break;

			case TIMEOUT10:														// 画面切替用ﾀｲﾏｰとして使用
				Ope_TimeOut_10( OPECTL.Ope_mod,  e_pram_set );
				break;

// MH321800(S) G.So ICクレジット対応
			case EC_CYCLIC_DISP_TIMEOUT:										// 画面切替用ﾀｲﾏｰとして使用
				Ope_TimeOut_Cyclic_Disp( OPECTL.Ope_mod,  e_pram_set );
				break;

			case EC_RECEPT_SEND_TIMEOUT:										// ec用ﾀｲﾏｰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,  e_pram_set );
				break;
// MH321800(E) G.So ICクレジット対応

			case KEY_RESSW1:															// 残額照会ﾎﾞﾀﾝｲﾍﾞﾝﾄ
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( !Edy_Rec.edy_status.BIT.ZAN_SW ){
//					if( Dsp_Prm_Setting == 10 ){										// Edy設定のみかつEdy関連ｴﾗｰ未発生の場合
//						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ﾀｲﾏｰ1(03-0067)延長起動(ｵﾍﾟﾚｰｼｮﾝ制御用) // 07-01-19変更 //
//						tim1_mov = 1;													// ﾀｲﾏｰ1起動中 // 07-01-16追加 //
//						LcdBackLightCtrl( ON );											// back light ON
//						Edy_Rec.edy_status.BIT.ZAN_SW = 1;								// 残額照会ﾎﾞﾀﾝ押下済みｾｯﾄ
//						Edy_SndData01();												// ｶｰﾄﾞ検知開始指示送信
//						op_mod01_dsp_sub();												// 残額照会可能媒体表示検索
//						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]] );					// 6行目に画面切替用表示をする
//						DspWorkerea[0] = 1;
//					}
//					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );				// 画面切替用ﾀｲﾏｰ起動(Timer10)
//				}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			case IBK_EDY_RCV:														// Edyﾓｼﾞｭｰﾙからの受信ﾃﾞｰﾀあり
//				ret = Ope_Edy_Event( msg, OPECTL.Ope_mod );
//				break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			case TIMEOUT18:
				shutter_err_chk( &rd_err_count );										// 磁気ﾘｰﾀﾞｰ異常状態ﾁｪｯｸ
				break;
			case TIMEOUT26:
				if( DspSts == LCD_WMSG_OFF ){						// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示状態がＯＦＦ
					dspCyclicErrMsgRewrite();
				}
				Lagtim( OPETCBNO, 26, 3*50 );						// ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ(3s)
				break;
			case TIMEOUT27:
				f_MandetMask = 0;
				break;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//			case TIMEOUT28:
//				OpeLcd( 1 );								// 待機画面表示
//				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			case OPE_REQ_CALC_FEE:							// 料金計算要求
				// 料金計算
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);		// 全車室情報テーブル送信
				break;

// MH322914(S) K.Onodera 2016/08/18 AI-V対応：振替精算
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//			// 振替対象情報取得要求
//			case OPE_REQ_FURIKAE_TARGET_INFO:
//				ans = carchk( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, 0 );	// 駐車位置番号チェック
//				if( ans != 0 ){
//					// 出庫状態 or NG
//					if( ans != 1 && ans != 4 ){
//						// NG応答
//						NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_DST );	// 振替先条件エラー
//						break;
//					}
//				}
//				// フラグクリア
//				cm27();
//				init_ryocalsim();
//				// 料金計算
//				ans = (short)Ope_PipFurikaeCalc();
//				ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
//				NTNET_Snd_Data16_04( (ushort)ans );
//				break;
//
//			// 振替要求
//			case OPE_REQ_FURIKAE_GO:
//				// 振替元車室ラグタイマ停止
//				if( ope_Furikae_start(vl_frs.lockno) ){
//					;
//				} else {
//					// NG応答
//					NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_SRC );	// 振替元条件エラー
//					break;
//				}
//				ans = carchk( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, 0 );	// 駐車位置番号チェック
//				if( ans != 0 ){
//					// 出庫状態 or NG
//					if( ans != 1 && ans != 4 ){
//						// NG応答
//						NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_DST );	// 振替先条件エラー
//						break;
//					}
//				}
//				op_mod01_Init();								// op_mod01() ｽｷｯﾌﾟのためｴﾘｱのみｸﾘｱ
//				OPECTL.Ope_mod = 2;								// 料金表示画面へ
//				PiP_FurikaeStart();
//				memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ
//				cm27();											// ﾌﾗｸﾞｸﾘｱ
//				LcdBackLightCtrl( ON );
//				ret = 1;										// 次操作
//				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH322914(E) K.Onodera 2016/08/18 AI-V対応：振替精算
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)
			// 遠隔精算料金計算要求
			case OPE_REQ_REMOTE_CALC_TIME_PRE:

// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算（リアルタイム）開始受付済み
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
				// クレジットHOST接続中？
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_BUSY );		// NG応答
					break;
				}

				// フラグクリア
				cm27();
				init_ryocalsim();
				// 料金計算
				ans = (short)Ope_PipRemoteCalcTimePre();
				NTNET_Snd_Data16_08( (ushort)ans );
				break;

			// 遠隔精算入庫時刻指定精算要求
			case OPE_REQ_REMOTE_CALC_TIME:

// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算（リアルタイム）開始受付済み
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
				// クレジットHOST接続中？
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_BUSY );		// NG応答
					break;
				}
// MH810100(S) K.Onodera  2019/12/25 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//				ans = carchk( g_PipCtrl.stRemoteTime.Area, g_PipCtrl.stRemoteTime.ulNo, 0 );	// 駐車位置番号チェック
//				if( ans > 2 ){
//					// NG応答
//					switch( ans ){
//						case 3:			// 3 = 休業状態(ｺｲﾝﾒｯｸ、紙幣ﾘｰﾀﾞｰ通信異常)
//						case 4:			// 4 = ラグタイム中 
//						case 10:		// 10 = NG(ﾛｯｸ装置状態が規定外等) 
//							NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_DENY);
//							break;
//						default:
//							NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_NO_CAR);
//							break;
//					}
//					break;
//				}
// MH810100(E) K.Onodera  2019/12/25 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//				Ope_Set_tyudata();							// 料金計算用に駐車券ﾃﾞｰﾀをセットする。
//				read_sht_cls();									// ﾘｰﾀﾞｰｼｬｯﾀｰ閉・LED OFF
//				op_mod01_Init();								// op_mod01() ｽｷｯﾌﾟのためｴﾘｱのみｸﾘｱ
//				memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ
//				OPECTL.Ope_mod = 2;								// 料金表示画面へ
//				OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;	// 料金計算開始要因=駐車券ﾃﾞｰﾀ受信
//				cm27();											// ﾌﾗｸﾞｸﾘｱ
//				vl_now = V_CHM;
//				LcdBackLightCtrl( ON );
//				ret = 1;										// 次操作
//				break;
				// 遠隔精算要求の応答待ちじゃない？
				if( OPECTL.remote_wait_flg == 0 ){
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
					if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
						// 入庫時刻指定遠隔精算開始
						lcdbm_remote_time_start();
					} else {					// 通常料金計算モード
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

					// 遠隔精算開始送信
					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
					}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

					// 遠隔精算開始応答待ちフラグON
					OPECTL.remote_wait_flg = 1;
					// 応答待ちタイマスタート
					Lagtim( OPETCBNO, TIMERNO_REMOTE_PAY_RESP, (res_wait_time * 50) );
				}
				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)

// MH322914 (s) kasiyama 2016/07/15 AI-V対応
			case OPE_REQ_REMOTE_CALC_FEE:							// 遠隔精算金額指定精算要求

// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算（リアルタイム）開始受付済み
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
				// クレジットHOST接続中？
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG応答
					break;
				}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//				// 後日精算
//				if( !g_PipCtrl.stRemoteFee.Type ){
//// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//// MH810100(S) K.Onodera 2019/12/24 車番チケットレス（ParkiPRO対応）
////					ans = carchk( g_PipCtrl.stRemoteFee.Area, g_PipCtrl.stRemoteFee.ulNo, 0 );	// 駐車位置番号チェック
////					if( ans > 2 ){
////						// NG応答
////						switch( ans ){
////							case 3:			// 3 = 休業状態(ｺｲﾝﾒｯｸ、紙幣ﾘｰﾀﾞｰ通信異常)
////							case 4:			// 4 = ラグタイム中 
////							case 10:		// 10 = NG(ﾛｯｸ装置状態が規定外等) 
////								NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_DENY);
////								break;
////							default:
////								NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_NO_CAR);
////								break;
////						}
////						break;
////					}
//					OPECTL.Pr_LokNo = 1;
//// MH810100(E) K.Onodera 2019/12/24 車番チケットレス（ParkiPRO対応）
//// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//				}
//				// 出庫精算
//				else{
//					OPECTL.Pr_LokNo = 0;
//// MH810100(S) K.Onodera 2019/12/24 車番チケットレス（ParkiPRO対応）
////					OPECTL.Op_LokNo = (g_PipCtrl.stRemoteFee.Area*10000L + g_PipCtrl.stRemoteFee.ulNo);
//// MH810100(E) K.Onodera 2019/12/24 車番チケットレス（ParkiPRO対応）
//				}
//// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//
//				// 紛失精算チェック
//				if( Ope_FunCheck(1) ){
//// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//					// 操作通知(遠隔精算開始)送信	0固定
//					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );
//// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//
//					read_sht_cls();									// ﾘｰﾀﾞｰｼｬｯﾀｰ閉・LED OFF
//					op_mod01_Init();								// op_mod01() ｽｷｯﾌﾟのためｴﾘｱのみｸﾘｱ
//					memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ
//					OPECTL.Ope_mod = 2;								// 料金表示画面へ
//					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_FEE;		// 料金計算開始要因=紛失精算
//					cm27();											// ﾌﾗｸﾞｸﾘｱ
//					vl_now = V_FUN;
//					LcdBackLightCtrl( ON );
//					ret = 1;										// 次操作
//				}
				// 遠隔精算要求の応答待ちじゃない？
				if( OPECTL.remote_wait_flg == 0 ){
					// 遠隔精算開始送信
					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );
					// 遠隔精算開始応答待ちフラグON
					OPECTL.remote_wait_flg = 2;
					// 応答待ちタイマスタート
					Lagtim( OPETCBNO, TIMERNO_REMOTE_PAY_RESP, (res_wait_time * 50) );
				}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
				break;
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH810105(S) MH364301 インボイス対応
			case PRIEND_PREQ_RYOUSYUU:
				if (IS_INVOICE) {
					if (OPECTL.Pri_Kind == J_PRI) {
						// ジャーナル印字
						if (OPECTL.f_ReIsuType) {
							// 短い領収証（現金精算など）印字で印字中にフタを開けた場合、
							// 印字完了→印字失敗という順で通知される場合がある
							// そのため、印字完了受信後にWaitタイマを動作させて
							// タイマ動作中に印字失敗を受信した場合は印字失敗として扱う
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
//							LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, JNL_PRI_WAIT_TIME, op_jnl_failchk);
							LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, jnl_pri_wait_Timer, op_jnl_failchk);
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
						}
					}
				}
				else {
					if (f_reci_ana && OPECTL.Pri_Result == PRI_NML_END) {
						f_reci_ana = 0;
					}
				}
				break;
// MH810105(E) MH364301 インボイス対応
// MH321800(S) Y.Tanizaki ICクレジット対応
#if (6 == AUTO_PAYMENT_PROGRAM)
			case AUTO_PAYMENT_TIMEOUT:
				BUZPI();
				LcdBackLightCtrl( ON );
				is_auto_pay = 1;
				key_num = auto_payment_locksts_set();
				OPECTL.Ope_mod = 1;
				ret = 1;
				break;
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
			// 入庫情報
			case LCD_IN_CAR_INFO:
// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
//				if ( OPECTL.remote_wait_flg == 1 ) {	// 遠隔精算(入庫時刻指定)
				if ( OPECTL.remote_wait_flg == 1 ||		// 遠隔精算(入庫時刻指定)
					 OPECTL.f_rtm_remote_pay_flg != 0) {	// 遠隔精算（リアルタイム）
// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
					// タイマ停止
					Lagcan(OPETCBNO, TIMERNO_REMOTE_PAY_RESP);

					// 入庫情報(ｶｰﾄﾞ情報)のﾁｪｯｸと設定処理
					lcdbm_rsp_in_car_info_proc();

					// ｵﾍﾟﾚｰｼｮﾝﾀｽｸ内で1精算終了後で行うﾌﾗｸﾞｸﾘｱ
					cm27();

					// 料金計算用に駐車券データをセットする
// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
//					Ope_Set_tyudata();
					if (OPECTL.remote_wait_flg == 1) {	// 遠隔精算(入庫時刻指定)
// GG129004(S) M.Fujikawa 2024/12/09 個別ROMDB指摘事項対応（2024/11/27 大木）
//						OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;
						Ope_Set_tyudata();
// GG129004(E) M.Fujikawa 2024/12/09 個別ROMDB指摘事項対応（2024/11/27 大木）
					}
					else {								// 車番チケットレス精算(入庫時刻指定)
// GG129004(S) M.Fujikawa 2024/12/09 個別ROMDB指摘事項対応（2024/11/27 大木）
//						OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_LCD_IN_TIME;
						Ope_Set_tyudata_Card();
// GG129004(E) M.Fujikawa 2024/12/09 個別ROMDB指摘事項対応（2024/11/27 大木）
					}
// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）

					// op_mod02(料金表示、入金処理)
					OPECTL.Ope_mod = 2;

					// 料金計算用にセット
					OPECTL.Pr_LokNo = 1;

					// 精算開始トリガーセット
// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
//					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;
					if (OPECTL.remote_wait_flg == 1) {	// 遠隔精算(入庫時刻指定)
						OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;
					}
					else {								// 車番チケットレス精算(入庫時刻指定)
						OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_LCD_IN_TIME;
					}
					OPECTL.f_rtm_remote_pay_flg = 0;
// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）

					// 現在VLカードデータ = 精算前券
					vl_now = V_CHM;
// GG129003(S) M.Fujikawa 2024/11/22 標準不具合修正　GM858400(S)[マージ] クラウド料金計算対応(改善連絡7、8:遠隔精算時の情報セット漏れ)
					LcdBackLightCtrl( ON );
// GG129003(E) M.Fujikawa 2024/11/22 標準不具合修正　GM858400(S)[マージ] クラウド料金計算対応(改善連絡7、8:遠隔精算時の情報セット漏れ)

					ret = 1;
				}

				break;
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			// QRデータ
			case LCD_QR_DATA:
				// QR応答データ
				lcdbm_QR_data_res( 1 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
				break;

			// 操作通知
			case LCD_OPERATION_NOTICE:
				// 操作コード
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// 精算中止要求
					case LCDBM_OPCD_PAY_STP:
// GG129004(S) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
//// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
//						if (OPECTL.f_rtm_remote_pay_flg != 0) {
//							// 遠隔精算（リアルタイム）開始受付済み
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
//							break;
//						}
//// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
						if(OPECTL.f_rtm_remote_pay_flg != 0){
							OPECTL.f_rtm_remote_pay_flg = 0;
						}
// GG129004(E) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
						// 操作通知(精算中止応答(OK))送信	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
						break;

					// 精算開始要求
					case LCDBM_OPCD_PAY_STA:
						// 自動集計中？ or 精算ログ書き込み中 or 領収証印字中 or 遠隔精算要求の応答待ち？
// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
//						if( auto_syu_prn == 2 || (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) || ( ryo_inji != 0 ) || OPECTL.remote_wait_flg ){
						if( auto_syu_prn == 2 || (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0) ||
							( ryo_inji != 0 ) || OPECTL.remote_wait_flg ||
							OPECTL.f_rtm_remote_pay_flg != 0 ){
// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );	// NG
// MH810100(S) Y.Yamauchi 2020/03/05 #3901 バックライトの点灯/消灯が設定通りに動作しない
							LcdBackLightCtrl( ON );
							Lagtim( OPETCBNO, 1, 10*50 );						// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
							tim1_mov = 1;										// ﾀｲﾏｰ1起動中
// MH810100(E) Y.Yamauchi 2020/03/05 #3901 バックライトの点灯/消灯が設定通りに動作しない
						}else{
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );	// OK
							OPECTL.Ope_mod = 1;		// op_mod01(車番入力) = ｶｰﾄﾞ情報待ち
							ret = 1;
// MH810100(S) Y.Yamauchi 2020/03/13 #3901 バックライトの点灯/消灯が設定通りに動作しない
							LcdBackLightCtrl( ON );
// MH810100(E) Y.Yamauchi 2020/03/13 #3901 バックライトの点灯/消灯が設定通りに動作しない
						}
						break;

					// 遠隔精算開始応答
					case LCDBM_OPCD_RMT_PAY_STA_RES:
						// タイマ停止
						Lagcan( OPETCBNO, TIMERNO_REMOTE_PAY_RESP );

						// ＯＫ？
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							// 入庫時刻指定
							if( OPECTL.remote_wait_flg == 1 ){
								OPECTL.Pr_LokNo = 1;
								cm27();											// ﾌﾗｸﾞｸﾘｱ
								Ope_Set_tyudata();								// 料金計算用に駐車券ﾃﾞｰﾀをセットする。
								op_mod01_Init();								// op_mod01() ｽｷｯﾌﾟのためｴﾘｱのみｸﾘｱ
								memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ
								OPECTL.Ope_mod = 2;								// 料金表示画面へ
								OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;	// 料金計算開始要因=駐車券ﾃﾞｰﾀ受信
								vl_now = V_CHM;
								LcdBackLightCtrl( ON );
								ret = 1;										// 次操作
							}
							// 金額指定
							else if( OPECTL.remote_wait_flg == 2 ){
								// 出庫精算
								if( !g_PipCtrl.stRemoteFee.Type ){
									OPECTL.Pr_LokNo = 1;
								}
								// 後日精算
								else{
									OPECTL.Pr_LokNo = 0;
								}
								Ope_FunCheck(1);
								op_mod01_Init();								// op_mod01() ｽｷｯﾌﾟのためｴﾘｱのみｸﾘｱ
								memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ
								OPECTL.Ope_mod = 2;								// 料金表示画面へ
								OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_FEE;		// 料金計算開始要因=紛失精算
								cm27();											// ﾌﾗｸﾞｸﾘｱ
								vl_now = V_FUN;
								LcdBackLightCtrl( ON );
								ret = 1;										// 次操作
							}
						}
						// ＮＧ？
						else{
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 							OPECTL.remote_wait_flg = 0;	// 問合せ中フラグクリア
// 							NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG応答
							if ( OPECTL.remote_wait_flg == 1 ) {		// 遠隔精算(入庫時刻指定)
								OPECTL.remote_wait_flg = 0;									// 問合せ中フラグクリア
								NTNET_Snd_Data16_08(LcdRecv.lcdbm_rsp_notice_ope.status);	// NG応答
							} else if( OPECTL.remote_wait_flg == 2 ) {	// 遠隔精算(金額指定)
								OPECTL.remote_wait_flg = 0;									// 問合せ中フラグクリア
								NTNET_Snd_Data16_09(LcdRecv.lcdbm_rsp_notice_ope.status);	// NG応答
							}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
						}
						break;
// GG129000(S) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）
					// 遠隔精算（リアルタイム）開始要求
					case LCDBM_OPCD_RTM_REMOTE_PAY_REQ:
// GG129004(S) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
//						if (OPECTL.remote_wait_flg != 0 ||
//							OPECTL.f_rtm_remote_pay_flg != 0) {
						if (OPECTL.remote_wait_flg != 0) {
// GG129004(E) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
							// 遠隔精算（PIP）開始要求中
							// 遠隔精算（リアルタイム）開始NG
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 1 );
						}
						else {
							OPECTL.f_rtm_remote_pay_flg = 1;
							// 遠隔精算（リアルタイム）開始OK
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
						}
						break;
					// 遠隔精算（リアルタイム）受付中止
					case LCDBM_OPCD_RTM_REMOTE_PAY_CAN:
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							OPECTL.f_rtm_remote_pay_flg = 0;
						}
						break;
// GG129000(E) T.Nagai 2023/10/04 ゲート式車番チケットレスシステム対応（遠隔精算対応）（待機状態でも受付ける）

					// 起動通知
					case LCDBM_OPCD_STA_NOT:
						// 起動ﾓｰﾄﾞ = 設定ｱｯﾌﾟﾛｰﾄﾞ起動
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(起動中)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// 起動ﾓｰﾄﾞ = 通常起動
						else {
							// op_mod00(待機)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;
// MH810100(S) K.Onodera 2020/03/13 車番チケットレス(起動完了通知受信考慮漏れ修正)
					// 起動完了通知
					case LCDBM_OPCD_STA_CMP_NOT:
						ope_idle_transit_common();	// 待機状態遷移共通処理
						OPECTL.init_sts = 1;
						OPECTL.Ope_mod = 0;
						ret = -1;
						break;

					default:
						break;
// MH810100(E) K.Onodera 2020/03/13 車番チケットレス(起動完了通知受信考慮漏れ修正)
				}
				break;

			// 遠隔精算開始応答待ちタイムアウト
			case TIMEOUT_REMOTE_PAY_RESP:
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 				OPECTL.remote_wait_flg = 0;	// 問合せ中フラグクリア
// 				NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG応答
				if ( OPECTL.remote_wait_flg == 1 ) {		// 遠隔精算(入庫時刻指定)
					OPECTL.remote_wait_flg = 0;						// 問合せ中フラグクリア
					NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_OTHER);	// NG応答
				} else if( OPECTL.remote_wait_flg == 2 ) {	// 遠隔精算(金額指定)
					OPECTL.remote_wait_flg = 0;						// 問合せ中フラグクリア
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_OTHER);	// NG応答
				}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(起動中)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// 初期化未完了状態とする
				ret = -1;
				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

			default:
				break;
		}

// MH810100(S) K.Onodera 2019/10/17 車番チケットレス(不要処理削除)
//		key_chk = op_key_chek();									// 「登録」キー＆「取消」キーの同時押し状態監視
//		switch( key_chk ){
//			case	1:												// 同時押し状態開始
//				Lagtim( OPETCBNO, 6, 3*50 );						// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）起動（３秒）
//				break;
//			case	2:												// 同時押し状態解除
//				Lagcan( OPETCBNO, 6 );								// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）ﾘｾｯﾄ
//				break;
//		}
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス(不要処理削除)
		if (tim2_mov) {
			if (OPECTL.on_off == 0) {
				// F1ｷｰ押下解除
				Lagcan( OPETCBNO, 2 );								// ﾀｲﾏｰ2（F1ｷｰ押下監視ﾀｲﾏｰ）ﾘｾｯﾄ
				tim2_mov = 0;
			}
		}
		lto_syuko();												// ﾌﾗｯﾌﾟ上昇、ロック閉ﾀｲﾏ内出庫
		if(	fus_subcal_flg == 0 ){
			if( fus_kyo() == 2 ){									// 不正・強制集計処理(1=強制出庫,2=不正出庫)
				if( prm_get(COM_PRM,S_NTN,63,1,5) == 0 ){	// 即時送信しない設定の場合
					// この時点でFROMへの書込みは終わっている or 書込み要求なし
					ntautoReqToSend( NTNET_BUFCTRL_REQ_SALE );	// 精算データ送信要求
				}
			}
		}
		mc10exec();
	}

	if((reci_sw_flg) || ( OPECTL.RECI_SW == (char)-1 )){
		if(Flap_Sub_Flg == 1){
			if(Flap_Sub_Num < 9){
				uc_buf_size = (uchar)(9 - Flap_Sub_Num);
				memset(&FLAPDT_SUB[Flap_Sub_Num],0,sizeof(flp_com_sub));			// 詳細中止ｴﾘｱ(精算完了)ｸﾘｱ
				memmove(&FLAPDT_SUB[Flap_Sub_Num],&FLAPDT_SUB[Flap_Sub_Num+1],sizeof(flp_com_sub)*uc_buf_size);
			}
			if(Flap_Sub_Num == 10){
				memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));							// 終端詳細中止ｴﾘｱｸﾘｱ
			}else{
				memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// 終端詳細中止ｴﾘｱｸﾘｱ
			}
			Flap_Sub_Flg = 0;														// 詳細中止ｴﾘｱ使用フラグOFF
			OPECTL.RECI_SW = 0;														// 待機画面を抜けたら領収書発行不可
		}
	}

	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 2 );											// F1ｷｰ押下監視ﾀｲﾏ-
	Lagcan( OPETCBNO, 4 );											// ﾀｲﾏｰ4ﾘｾｯﾄ(不正強制出庫監視用)
	Lagcan( OPETCBNO, 6 );											// ﾀｲﾏｰ6ﾘｾｯﾄ
	Lagcan( OPETCBNO, 7 );											// ﾀｲﾏｰ7ﾘｾｯﾄ
	Lagcan( OPETCBNO, 10 );											// Suica残高表示用ﾀｲﾏｰﾘｾｯﾄ
	Lagcan( OPETCBNO, 11 );											// 画面切替用ﾀｲﾏｰﾘｾｯﾄ
	Lagcan( OPETCBNO, 18 );											// ﾀｲﾏｰ18(10s)ｼｬｯﾀｰ異常判定用ﾀｲﾏｰSTOP
	Lagcan( OPETCBNO, 26 );											// ﾀｲﾏｰ26(3s)釣銭不足、用紙不足のｻｲｸﾘｯｸﾀｲﾏｰSTOP
	Lagcan( OPETCBNO, TIMER_MANDETMASK );							// 人体検知マスク時間リセット (27)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	LagCan500ms( LAG500_EDY_LED_RESET_TIMER );						// Edy関連ﾀｲﾏｰ消去
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) G.So ICクレジット対応
	Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );					// ecリサイクル表示用ﾀｲﾏｰﾘｾｯﾄ
// MH321800(E) G.So ICクレジット対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	Lagcan( OPETCBNO, TIMERNO_REMOTE_PAY_RESP );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH810105(S) MH364301 インボイス対応
	LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);							// ジャーナル印字完了待ちタイマ
	LagCan500ms(LAG500_RECEIPT_MISS_DISP_TIMER);					// 領収証失敗表示タイマ
// MH810105(E) MH364301 インボイス対応
// MH321800(S) Y.Tanizaki ICクレジット対応
#if (6 == AUTO_PAYMENT_PROGRAM)
	Lagcan( OPETCBNO, TIMERNO_AUTO_PAYMENT_TIMEOUT );
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_StopAndLedOff();											// ｶｰﾄﾞ検知停止指示開始＆LEDOFF
//	Edy_Rec.edy_status.BIT.ZAN_SW = 0;								// 残額照会ﾎﾞﾀﾝ押下済みﾘｾｯﾄ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	op_SuicaFusokuOff();											// 6・7行目のｱﾗｰﾑを消去
	ope_anm( AVM_STOP );

	if( OPECTL.Mnt_mod != 4 ){										// 「補充ｶｰﾄﾞ受付完了」以外？
		rd_shutter();												// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
	}

	LedReq( CN_TRAYLED, LED_OFF );									// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED消灯
	f_MandetMask = 0;

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 駐車位置番号入力処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod01( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = 他処理へ切換                              |*/
/*|                       : -1 = ﾓｰﾄﾞ切替                                  |*/
/*|                       : 10 = 取消(中止)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod01( void )
{
	short	ret;
	short	ans;
	ushort	msg = 0;
// GG129000(S) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）
	ushort result = 0;
// GG129000(E) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	stParkKindNum_t	stQRTicket;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//// MH322914(S) K.Onodera 2016/12/22 [静的解析-583]未初期化
////	char	setwk;
//	char	setwk = 0;
//// MH322914(E) K.Onodera 2016/12/22 [静的解析-583]未初期化
//	char	pri_time_flg;
//	ushort	key_num_wk = key_num;			// 暗証番号出庫用キー入力値(4桁)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	OpeLcd( 2 );													// 駐車位置番号表示
//
//	ope_anm( AVM_ICHISEL );											// 車室番号選択時ｱﾅｳﾝｽ
	ushort	res_wait_time;

	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// データ受信監視タイマ
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// 範囲外は5秒とする
	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

// MH322914(S) K.Onodera 2016/09/21 AI-V対応
//	OPECTL.multi_lk = 0L;											// ﾏﾙﾁ精算問合せ中車室№
//	OPECTL.ChkPassSyu = 0;											// 定期券問合せ中券種ｸﾘｱ
//	OPECTL.ChkPassPkno = 0L;										// 定期券問合せ中駐車場№ｸﾘｱ
//	OPECTL.ChkPassID = 0L;											// 定期券問合せ中定期券IDｸﾘｱ
//// MH322914 (s) kasiyama 2016/07/15 AI-V対応
//	OPECTL.CalStartTrigger = 0;
//// MH322914 (e) kasiyama 2016/07/15 AI-V対応
	op_mod01_Init();
// MH322914(E) K.Onodera 2016/09/21 AI-V対応

// MH321800(S) Y.Tanizaki ICクレジット対応
#if (6 == AUTO_PAYMENT_PROGRAM)
	if(is_auto_pay) {
		msg = KEY_TEN_F1;
		goto AUTO_PAYMENT_PROGRAM_START;
	}
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス（OPE対応）
//	Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス（OPE対応）

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( Edy_Rec.edy_status.BIT.INITIALIZE ){						// Edyﾓｼﾞｭｰﾙの初期化が完了している？
//		Edy_StopAndLedOff();										// ｶｰﾄﾞ検知停止指示開始＆LEDOFF
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
// MH321800(S) Y.Tanizaki ICクレジット対応
#if (6 == AUTO_PAYMENT_PROGRAM)
AUTO_PAYMENT_PROGRAM_START:
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応
		switch( msg ){

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//			// op_mod01()のloop内処理でKEY_MODECHGでﾒﾝﾃﾅﾝｽに移行しないようにする
//			case KEY_MODECHG:										// Mode change key
//				if( OPECTL.on_off == 1 ){							// key ON
//					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
//					OPECTL.Pay_mod = 0;								// 通常精算
//					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//				}else{
//					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
//					OPECTL.Pay_mod = 0;								// 通常精算
//				}
//				ret = -1;
//				break;
//
//			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
//			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
//			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
//			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
//			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
//			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
//			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
//			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
//			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
//			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
//				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中はｷｰは利かない
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				key_num_wk = (( key_num_wk % 1000 ) * 10 )+( msg - KEY_TEN );
//				key_num = (( key_num % knum_ket ) * 10 )+( msg - KEY_TEN );
//				teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				break;
//			case KEY_TEN_CL:										// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
//				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中はｷｰは利かない
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				key_num = 0;
//				key_num_wk = 0;
//				teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				break;
//			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
//				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中はｷｰは利かない
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				if(key_num_wk != 0 && key_num_wk == CPrmSS[39][6]){
//					OPECTL.Ope_mod = 200;
//					key_num = 0;
//					key_num_wk = 0;									// 暗証番号出庫用キー入力値
//					ret = 1;
//					break;
//				}
//				ans = carchk( key_sec, key_num, 0 );				// 駐車位置番号ﾁｪｯｸ
//				setwk = (char)Carkind_Param(FLP_ROCK_INTIME, (char)(LockInfo[OPECTL.Pr_LokNo-1].ryo_syu),1,1);
//				if(( ans == -1 )||( ans == 10 )||( ans == 2 )||		// 精算不可能 or 未設定 or 出庫状態
//				   (( ans == 1 )&&( setwk == 0 ))){					// ﾛｯｸ装置閉開始待ち and ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に精算しない設定
//					if( ans == -1						&&				// 精算不可能
//						( OPECTL.Pay_mod != 1 ) 		&&				// パスワード忘れ出庫では無い
//						_is_ntnet_normal()				&&				// NT-NET接続？
//						prm_get( COM_PRM,S_NTN,26,1,1 )) 				// 車室問合せする?
//					{
//						if( !ERR_CHK[mod_ntibk][1] ){				// NTNET IBK 通信正常?
//							OPECTL.multi_lk = (ulong)(( key_sec * 10000L ) + key_num );
//							NTNET_Snd_Data01( OPECTL.multi_lk );	// NTNET車室問合せﾃﾞｰﾀ作成
//							grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[70]);		// "      只今、問合せ中です      "
//							grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);		// "                              "
//							blink_reg(6, 5, 20, 0, COLOR_DARKSLATEBLUE, &OPE_CHR[7][5] );		// "     しばらくお待ち下さい     "
//							grachr(7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0]);		// "                              "
//							Lagtim( OPETCBNO, 2, (ushort)(prm_get( COM_PRM,S_NTN,30,2,3 )*50+1) );	// ﾀｲﾏｰ2(XXs)起動(ﾏﾙﾁ精算問合せ監視)
//						}else{
//							ope_anm( AVM_BTN_NG );					// 車室番号間違い時ｱﾅｳﾝｽ
//							key_num = 0;
//							key_num_wk = 0;							// 暗証番号出庫用キー入力値
//							teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
//							grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );		// "                              "
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[7] );		// "  他の精算機で精算して下さい  "
//							grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );		// "                              "
//							Lagtim( OPETCBNO, 4, 5*50 );			// ﾀｲﾏｰ4(5s)起動(ｴﾗｰ表示用)
//						}
//					}else{
//						ope_anm( AVM_BTN_NG );											// 車室番号間違い時ｱﾅｳﾝｽ
//						key_num = 0;
//						teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
//						key_num_wk = 0;													// 暗証番号出庫用キー入力値
//					}
//					break;
//				}
//				if(( ans == 0 )||									// OK or 休業状態(ﾒｯｸ,紙幣ﾘｰﾀﾞｰ通信異常) or
//				   (( ans == 1 )&&( setwk ))){						// ﾛｯｸ装置閉開始待ち and ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に精算する設定
//					if(( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd != 0 )&&	// ﾊﾟｽﾜｰﾄﾞ有り?
//					   ( OPECTL.Pay_mod != 1 )){					// ﾊﾟｽﾜｰﾄﾞ忘れ処理以外?
//						/*** ﾊﾟｽﾜｰﾄﾞ登録有り(通常精算のみ) ***/
//						OPECTL.Ope_mod = 70;						// ﾊﾟｽﾜｰﾄﾞ入力処理(精算時)へ
//					}else{
//						/*** ﾊﾟｽﾜｰﾄﾞ登録無し ***/
//						if( OPECTL.InLagTimeMode ) {				// ラグタイム延長処理実行？
//							OPECTL.Ope_mod = 220;					// ラグタイム延長処理へ
//						}else{
//							OPECTL.Ope_mod = 2;						// 料金表示,入金処理へ
//						}
//					}
//					cm27();											// ﾌﾗｸﾞｸﾘｱ
//					ret = 1;
//				}
//				if( ans == 4  )										// ラグタイム中
//				{
//					OPECTL.Ope_mod = 230;							// 精算済み案内処理へ
//					cm27();											// ﾌﾗｸﾞｸﾘｱ
//					ret = 1;
//				}
//				break;
//
//			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
//				BUZPIPI();
//				break;
//			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
//				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中はｷｰは利かない
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				if( prm_get( COM_PRM,S_TYP,62,1,3 ) ){				// 駐車証明書発行可能枚数設定あり
//					setwk = (char)prm_get( COM_PRM,S_TYP,68,1,2 );	// ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に受付券発行しない/する
//					ans = carchk( key_sec, key_num, 12);			// 駐車位置番号ﾁｪｯｸ
//					if(( ans == -1 )||( ans == 10 )||( ans == 2 )||	// 精算不可能 or 未設定 or 出庫状態
//					   (( ans == 1 )&&( setwk == 0 ))){				// ﾛｯｸ装置閉開始待ち and ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に受付券発行しない設定
//						ope_anm( AVM_BTN_NG );						// 車室番号間違い時ｱﾅｳﾝｽ
//						key_num = 0;
//						teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
//						key_num_wk = 0;											// 暗証番号出庫用キー入力値
//						break;
//					}
//				pri_time_flg = pri_time_chk(OPECTL.Pr_LokNo,0);
//				if(pri_time_flg == 0){
//					//エラーメッセージ
//					BUZPIPI();
//					OPECTL.Ope_mod = 91;						// 受付券発行処理へ
//					ret = 1;
//					break;
//				}
//				else if(Ope_isPrinterReady() == 0){
//					BUZPIPI();
//					OPECTL.Ope_mod = 92;						// 受付券発行処理へ
//					shomei_errnum = 1;
//					ret = 1;
//					break;
//				}else{
//					if(( ans == 0 )||( ans == 3 )||					// OK or 休業状態(ﾒｯｸ,紙幣ﾘｰﾀﾞｰ通信異常) or
//					   (( ans == 1 )&&( setwk ))){					// ﾛｯｸ装置閉開始待ち and ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に受付券発行する設定
//						OPECTL.Ope_mod = 90;						// 受付券発行処理へ
//						ret = 1;
//					}
//				}
//				}
//				break;
//			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
//				BUZPI();
//				ret = 10;											// 待機へ戻る
//				break;
//			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
//				if( OPECTL.multi_lk || OPECTL.Pay_mod == 1 ){		// ﾏﾙﾁ精算問合せ中または暗証番号忘れ精算ﾓｰﾄﾞ時はｷｰは無効とする
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				if( prm_get( COM_PRM,S_TYP,63,1,1 ) ){
//					ans = carchk( key_sec, key_num, 3 );			// 駐車位置番号ﾁｪｯｸ
//					if(( ans == -1 )||( ans == 10 )||( ans == 2 )){	// 精算不可能 or 未設定 or 出庫状態
//						ope_anm( AVM_BTN_NG );						// 車室番号間違い時ｱﾅｳﾝｽ
//						key_num = 0;
//						teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
//						key_num_wk = 0;											// 暗証番号出庫用キー入力値
//						break;
//					}
//				pri_time_flg = pri_time_chk(OPECTL.Pr_LokNo,1);
//				if(pri_time_flg == 0){
//					//エラーメッセージ
//					BUZPIPI();
//					OPECTL.Ope_mod = 81;						// 受付券発行処理へ
//					ret = 1;
//					break;
//				}else{
//					if(( ans == 0 )||( ans == 1 )||( ans == 3 )){	// OK or ﾛｯｸ装置閉開始待ち or 休業状態(ﾒｯｸ,紙幣ﾘｰﾀﾞｰ通信異常)
//						OPECTL.Ope_mod = 80;						// ﾊﾟｽﾜｰﾄﾞ登録処理(入庫時)へ
//						ret = 1;
//					}
//				}
//				}
//				break;
//
//			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
//				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
//				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
//				break;
//			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
//				if( RD_mod < 9 ){
//					Ope2_WarningDispEnd();							// ｴﾗｰ表示消し
//					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
//				}
//				break;
//
//			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
//				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中?
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				}else{
//					ret = 10;
//				}
//				break;
//
//			case IBK_NTNET_LOCKMULTI:								// NTNET ﾏﾙﾁ精算用ﾃﾞｰﾀ(車室問合せ結果ﾃﾞｰﾀ)受信
//				if( LOCKMULTI.Answer == 0 && OPECTL.multi_lk )		// 結果が正常
//				{
//					ans = carchk_ml( key_sec, key_num );			// 駐車位置番号ﾁｪｯｸ
//					if(( ans == 0 )||								// OK or 休業状態(ﾒｯｸ,紙幣ﾘｰﾀﾞｰ通信異常) or
//					   (( ans == 1 )&&( setwk ))){					// ﾛｯｸ装置閉開始待ち and ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に精算する設定
//						if(( LOCKMULTI.lock_mlt.passwd != 0 )&&		// ﾊﾟｽﾜｰﾄﾞ有り?
//						   ( OPECTL.Pay_mod != 1 )){				// ﾊﾟｽﾜｰﾄﾞ忘れ処理以外?
//							/*** ﾊﾟｽﾜｰﾄﾞ登録有り(通常精算のみ) ***/
//							OPECTL.Ope_mod = 70;					// ﾊﾟｽﾜｰﾄﾞ入力処理(精算時)へ
//						}else{
//							/*** ﾊﾟｽﾜｰﾄﾞ登録無し ***/
//							OPECTL.Ope_mod = 2;						// 料金表示,入金処理へ
//						}
//						OPECTL.Op_LokNo = LOCKMULTI.LockNo;
//						OPECTL.Pr_LokNo = 0xffff;
//						cm27();										// ﾌﾗｸﾞｸﾘｱ
//						ret = 1;
//						break;
//					}
//					else{
//						LOCKMULTI.Answer = 1;						// 結果＝正常／車室状態＝駐車中以外
//					}
//				}
//				else{
//					if( LOCKMULTI.Answer == 1 ){					// ID=02（結果ﾃﾞｰﾀ）受信	結果＝未接続（該当なし）？
//						LOCKMULTI.Answer = 2;						// 結果＝未接続（該当なし）
//					}
//				}
//
//				// no break
//			case TIMEOUT2:											// ﾀｲﾏｰ2ﾀｲﾑｱｳﾄ
//				if( LOCKMULTI.Answer == 0 ){						// 車室問合せ結果ﾃﾞｰﾀ待ちﾀｲﾑｱｳﾄ？
//					LOCKMULTI.Answer = 3;							// Yes：通信障害中
//				}
//				if( OPECTL.multi_lk ){
//					OPECTL.multi_lk = 0;
//					ope_anm( AVM_BTN_NG );							// 車室番号間違い時ｱﾅｳﾝｽ
//
//					key_num = 0;
//					blink_end();									// 点滅終了
//					teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
//					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );				// "                              "
//					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );				// "                              "
//					key_num_wk = 0;																	// 暗証番号出庫用キー入力値
//
//					switch( LOCKMULTI.Answer ){
//						case	1:									// 車室状態＝駐車中以外
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[6] );		// "  車室番号が正しくありません  "
//							break;
//						case	2:									// 未接続
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[12] );		// "   該当する車室はありません   "
//							break;
//						case	3:									// 通信障害中
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[7] );		// "  他の精算機で精算して下さい  "
//							break;
//						case	4:									// 手動ﾓｰﾄﾞ中
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[13] );		// "     係員に連絡して下さい     "
//							break;
//						default:									// その他
//							grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[7] );		// "  他の精算機で精算して下さい  "
//// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
//							break;
//// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
//					}
//					Lagtim( OPETCBNO, 4, 5*50 );					// ﾀｲﾏｰ4(5s)起動(ｴﾗｰ表示用)
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//				}
//				break;
//
//			case TIMEOUT4:											// ﾀｲﾏｰ4ﾀｲﾑｱｳﾄ
//				if( OPECTL.multi_lk == 0 ){
//					LCDNO = (ushort)-1;
//					OpeLcd( 2 );									// 駐車位置番号表示(再表示)
//				}
//				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

			case MID_RECI_SW_TIMOUT:								// 領収証発行可能ﾀｲﾏｰﾀｲﾑｱｳﾄ通知
				OPECTL.RECI_SW = 0;									// 領収書ﾎﾞﾀﾝ未使用
				break;

			case OPE_REQ_CALC_FEE:									// 料金計算要求
				// 料金計算
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);				// 全車室情報テーブル送信
				break;

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
//
//			// 振替対象情報取得要求
//			case OPE_REQ_FURIKAE_TARGET_INFO:
//				ans = carchk( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, 0 );	// 駐車位置番号チェック
//				if( ans != 0 ){
//					// 出庫状態 or NG
//					if( ans != 1 && ans != 4 ){
//						// NG応答
//						NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_SRC );	// 振替元条件エラー
//						break;
//					}
//				}
//				// フラグクリア
//				cm27();
//				init_ryocalsim();
//				// 料金計算
//				ans = (short)Ope_PipFurikaeCalc();
//				ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
//				NTNET_Snd_Data16_04( (ushort)ans );
//				break;
//
//			// 振替要求
//			case OPE_REQ_FURIKAE_GO:
//				// 振替元車室ラグタイマ停止
//				if( ope_Furikae_start(vl_frs.lockno) ){
//					;
//				} else {
//					// NG応答
//					NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_SRC );	// 振替元条件エラー
//					break;
//				}
//				ans = carchk( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, 0 );	// 駐車位置番号チェック
//				if( ans != 0 ){
//					// 出庫状態 or NG
//					if( ans != 1 && ans != 4 ){
//						// NG応答
//						NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_FURIKAE_DST );	// 振替先条件エラー
//						break;
//					}
//				}
//				OPECTL.Ope_mod = 2;								// 料金表示画面へ
//				PiP_FurikaeStart();
//				cm27();											// ﾌﾗｸﾞｸﾘｱ
//				LcdBackLightCtrl( ON );
//				ret = 1;										// 次操作
//				break;
//// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH322914(S) K.Onodera 2016/09/15 AI-V対応：遠隔精算
			// 遠隔精算料金計算要求
			case OPE_REQ_REMOTE_CALC_TIME_PRE:

// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算（リアルタイム）開始受付済み
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				// クレジットHOST接続中？
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_BUSY );		// NG応答
					break;
				}

				// フラグクリア
				cm27();
				init_ryocalsim();
				// 料金計算
				ans = (short)Ope_PipRemoteCalcTimePre();
				NTNET_Snd_Data16_08( (ushort)ans );
				break;

			// 遠隔精算入庫時刻指定精算要求
			case OPE_REQ_REMOTE_CALC_TIME:

				// クレジットHOST接続中？
				if( OPECTL.InquiryFlg ){
					NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_BUSY );		// NG応答
					break;
				}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//				ans = carchk( g_PipCtrl.stRemoteTime.Area, g_PipCtrl.stRemoteTime.ulNo, 0 );	// 駐車位置番号チェック
//				if( ans > 2 ){
//					// NG応答
//					switch( ans ){
//						case 3:			// 3 = 休業状態(ｺｲﾝﾒｯｸ、紙幣ﾘｰﾀﾞｰ通信異常)
//						case 4:			// 4 = ラグタイム中 
//						case 10:		// 10 = NG(ﾛｯｸ装置状態が規定外等) 
//							NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_DENY);
//							break;
//						default:
//							NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_NO_CAR);
//							break;
//					}
//					break;
//				}
//				Ope_Set_tyudata();							// 料金計算用に駐車券ﾃﾞｰﾀをセットする。
//				OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;			// 料金計算開始要因=駐車券ﾃﾞｰﾀ受信
//				read_sht_cls();									// ﾘｰﾀﾞｰｼｬｯﾀｰ閉・LED OFF
//				OPECTL.Ope_mod = 2;
//				cm27();											// ﾌﾗｸﾞｸﾘｱ
//				vl_now = V_CHM;
//				LcdBackLightCtrl( ON );
//				ret = 1;										// 次操作
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算（リアルタイム）開始受付済み
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				// 遠隔精算要求の応答待ちじゃない？
				if( OPECTL.remote_wait_flg == 0 ){
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
					if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
						// 入庫時刻指定遠隔精算開始
						lcdbm_remote_time_start();
					} else {					// 通常料金計算モード
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

					// 遠隔精算開始送信
					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
					}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

					// 遠隔精算開始応答待ちフラグON
					OPECTL.remote_wait_flg = 1;
					// 応答待ちタイマスタート
					Lagtim( OPETCBNO, TIMERNO_REMOTE_PAY_RESP, (res_wait_time * 50) );
				}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
				break;
// MH322914(E) K.Onodera 2016/09/15 AI-V対応：遠隔精算
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
			// 遠隔精算金額指定精算要求
			case OPE_REQ_REMOTE_CALC_FEE:
			
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算（リアルタイム）開始受付済み
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if( OPECTL.InquiryFlg ){								// クレジットHOST接続中は受け付けない
					// NG応答
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_BUSY);
					break;
				}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//				if( !g_PipCtrl.stRemoteFee.Type ){
//// MH810100(S) K.Onodera 2019/12/24 車番チケットレス（ParkiPRO対応）
////					ans = carchk( g_PipCtrl.stRemoteFee.Area, g_PipCtrl.stRemoteFee.ulNo, 0 );	// 駐車位置番号ﾁｪｯｸ
////					if( ans > 2 ){
////						// NG応答
////						switch(ans){
////						case 3:			// 3 = 休業状態(ｺｲﾝﾒｯｸ、紙幣ﾘｰﾀﾞｰ通信異常)
////						case 4:			// 4 = ラグタイム中 
////						case 10:		// 10 = NG(ﾛｯｸ装置状態が規定外等) 
////							NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_DENY);
////							break;
////						default:
////							NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_NO_CAR);
////							break;
////						}
////						break;
////					}
//					OPECTL.Pr_LokNo = 1;
//// MH810100(E) K.Onodera 2019/12/24 車番チケットレス（ParkiPRO対応）
//				}else{
//					OPECTL.Pr_LokNo = 0;
//// MH810100(S) K.Onodera 2019/12/24 車番チケットレス（ParkiPRO対応）
////					OPECTL.Op_LokNo = (g_PipCtrl.stRemoteFee.Area*10000L + g_PipCtrl.stRemoteFee.ulNo);
//// MH810100(E) K.Onodera 2019/12/24 車番チケットレス（ParkiPRO対応）
//				}
//				if( Ope_FunCheck(1) ){								// 紛失精算ﾁｪｯｸ (OK)
//					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_FEE;			// 料金計算開始要因=紛失精算
//					read_sht_cls();								// ﾘｰﾀﾞｰｼｬｯﾀｰ閉・LED OFF
//					OPECTL.Ope_mod = 2;								// 料金表示,入金処理へ
//					cm27();											// ﾌﾗｸﾞｸﾘｱ
//					vl_now = V_FUN;
//					LcdBackLightCtrl( ON );
//					ret = 1;										// 次操作
//				}
				// 遠隔精算要求の応答待ちじゃない？
				if( OPECTL.remote_wait_flg == 0 ){
					// 遠隔精算開始送信
					lcdbm_notice_ope( LCDBM_OPCD_RMT_PAY_STA, 0 );
					// 遠隔精算開始応答待ちフラグON
					OPECTL.remote_wait_flg = 2;
					// 応答待ちタイマスタート
					Lagtim( OPETCBNO, TIMERNO_REMOTE_PAY_RESP, (res_wait_time * 50) );
				}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
				break;
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH321800(S) 受付許可が送信されない不具合修正
			case EC_RECEPT_SEND_TIMEOUT:							// ec用ﾀｲﾏｰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,0 );
				break;
// MH321800(E) 受付許可が送信されない不具合修正
// MH810105(S) MH364301 インボイス対応
			case PRIEND_PREQ_RYOUSYUU:
				if (IS_INVOICE) {
					if (OPECTL.Pri_Kind == J_PRI) {
						// ジャーナル印字
						if (OPECTL.f_ReIsuType) {
							// 短い領収証（現金精算など）印字で印字中にフタを開けた場合、
							// 印字完了→印字失敗という順で通知される場合がある
							// そのため、印字完了受信後にWaitタイマを動作させて
							// タイマ動作中に印字失敗を受信した場合は印字失敗として扱う
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
//							LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, JNL_PRI_WAIT_TIME, op_jnl_failchk);
							LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, jnl_pri_wait_Timer, op_jnl_failchk);
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
						}
					}
				}
				else {
					if (f_reci_ana && OPECTL.Pri_Result == PRI_NML_END) {
						f_reci_ana = 0;
					}
				}
				break;
// MH810105(E) MH364301 インボイス対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

			// 入庫情報
			case LCD_IN_CAR_INFO:

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
				if ( OPECTL.remote_wait_flg == 1 ) {	// 遠隔精算(入庫時刻指定)
					// タイマ停止
					Lagcan(OPETCBNO, TIMERNO_REMOTE_PAY_RESP);
				}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

				// 入庫情報(ｶｰﾄﾞ情報)のﾁｪｯｸと設定処理
				lcdbm_rsp_in_car_info_proc();

				// ｵﾍﾟﾚｰｼｮﾝﾀｽｸ内で1精算終了後で行うﾌﾗｸﾞｸﾘｱ
				cm27();

				// 料金計算用に駐車券データをセットする
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 				Ope_Set_tyudata_Card();
				if ( OPECTL.remote_wait_flg == 1 ) {	// 遠隔精算(入庫時刻指定)
					Ope_Set_tyudata();
				} else {								// 車番チケットレス精算(入庫時刻指定)
					Ope_Set_tyudata_Card();
				}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

				// op_mod02(料金表示、入金処理)
				OPECTL.Ope_mod = 2;

				// 料金計算用にセット
				OPECTL.Pr_LokNo = 1;

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 				// 精算開始トリガーセット = 車番チケットレス精算
// 				OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_LCD_IN_TIME;
				// 精算開始トリガーセット
				if ( OPECTL.remote_wait_flg == 1 ) {	// 遠隔精算(入庫時刻指定)
					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;
				} else {								// 車番チケットレス精算(入庫時刻指定)
					OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_LCD_IN_TIME;
				}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				OPECTL.f_rtm_remote_pay_flg = 0;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）

				// 現在VLカードデータ = 精算前券
				vl_now = V_CHM;
				ret = 1;
				break;

			// QRデータ
			case LCD_QR_DATA:
// GG129000(S) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）
//				// QR応答データ
//				lcdbm_QR_data_res( 1 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
//				break;
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算（リアルタイム）開始受付済み
					lcdbm_QR_data_res( 1 );	// 結果(1:NG(排他))
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				// mod01ではQR駐車券のみ受信
				if( ope_ParseBarcodeData( &MediaDetail ) ){
					result = ope_CanUseQRTicket( &MediaDetail );
					if( result == 1 ){
						lcdbm_QR_data_res( 4 );	// 結果(4:フォーマット不正)
						break;
					}
				}
				// 最終読取QRの保持(メンテナンス用)
				push_ticket( &MediaDetail,(ushort)result);

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				// QR駐車券の駐車券番号を保持
				memset( &stQRTicket, 0, sizeof(stQRTicket));
				stQRTicket.ParkingLotNo = MediaDetail.Barcode.QR_data.TicketType.ParkingNo;
				stQRTicket.CardType = CARD_TYPE_PARKING_TKT;
				intoas(&stQRTicket.byCardNo[0], MediaDetail.Barcode.QR_data.TicketType.EntryMachineNo, 3);
				intoasl(&stQRTicket.byCardNo[3], MediaDetail.Barcode.QR_data.TicketType.ParkingTicketNo, 6);
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

				// QR応答データ
				lcdbm_QR_data_res( 0 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
				break;
// GG129000(E) H.Fujinaga 2022/12/27 ゲート式車番チケットレスシステム対応（QR読取画面対応）

			// 操作通知
			case LCD_OPERATION_NOTICE:

				// 操作コード
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// 精算中止要求
					case LCDBM_OPCD_PAY_STP:
// GG129004(S) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
//// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
//						if (OPECTL.f_rtm_remote_pay_flg != 0) {
//							// 遠隔精算（リアルタイム）開始受付済み
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
//							break;
//						}
//// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
// GG129004(E) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
						// 精算中止応答(OK)送信
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
						// レーンモニタデータ登録(精算中止)
// GG129000(S) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.51)（GM803000流用）
						if(OPECTL.f_searchtype != 0xff) {				// 検索タイプセット後か
							ope_MakeLaneLog(LM_PAY_STOP);
						}
// GG129000(E) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.51)（GM803000流用）
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

						// op_mod00(待機)
						OPECTL.Ope_mod = 0;
						ret = -1;
						break;

					// 精算開始要求
					case LCDBM_OPCD_PAY_STA:
						// 遠隔精算要求の応答待ち？
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
//						if( OPECTL.remote_wait_flg ){
						if( OPECTL.remote_wait_flg || OPECTL.f_rtm_remote_pay_flg != 0 ){
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );	// NG
						}else{
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );	// OK
						}
						break;

					// 遠隔精算開始応答
					case LCDBM_OPCD_RMT_PAY_STA_RES:
						// タイマ停止
						Lagcan( OPETCBNO, TIMERNO_REMOTE_PAY_RESP );

						// ＯＫ？
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							// 入庫時刻指定
							if( OPECTL.remote_wait_flg == 1 ){
								OPECTL.Pr_LokNo = 1;
								cm27();										// ﾌﾗｸﾞｸﾘｱ
								Ope_Set_tyudata();							// 料金計算用に駐車券ﾃﾞｰﾀをセットする。
								OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_TIME;			// 料金計算開始要因=駐車券ﾃﾞｰﾀ受信
								OPECTL.Ope_mod = 2;
								vl_now = V_CHM;
								LcdBackLightCtrl( ON );
								ret = 1;										// 次操作
							}
							// 金額指定
							else if( OPECTL.remote_wait_flg == 2 ){
								// 出庫精算
								if( !g_PipCtrl.stRemoteFee.Type ){
									OPECTL.Pr_LokNo = 1;
								}
								// 後日精算
								else{
									OPECTL.Pr_LokNo = 0;
								}
								Ope_FunCheck(1);
								OPECTL.CalStartTrigger = (uchar)CAL_TRIGGER_REMOTECALC_FEE;			// 料金計算開始要因=紛失精算
								OPECTL.Ope_mod = 2;								// 料金表示,入金処理へ
								cm27();											// ﾌﾗｸﾞｸﾘｱ
								vl_now = V_FUN;
								LcdBackLightCtrl( ON );
								ret = 1;										// 次操作
							}
						}
						// ＮＧ？
						else{
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 							OPECTL.remote_wait_flg = 0;	// 問合せ中フラグクリア
// 							NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG応答
							if ( OPECTL.remote_wait_flg == 1 ) {		// 遠隔精算(入庫時刻指定)
								OPECTL.remote_wait_flg = 0;									// 問合せ中フラグクリア
								NTNET_Snd_Data16_08(LcdRecv.lcdbm_rsp_notice_ope.status);	// NG応答
							} else if( OPECTL.remote_wait_flg == 2 ) {	// 遠隔精算(金額指定)
								OPECTL.remote_wait_flg = 0;									// 問合せ中フラグクリア
								NTNET_Snd_Data16_09(LcdRecv.lcdbm_rsp_notice_ope.status);	// NG応答
							}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
						}
						break;

					// 起動通知
					case LCDBM_OPCD_STA_NOT:
						// 起動ﾓｰﾄﾞ = 設定ｱｯﾌﾟﾛｰﾄﾞ起動
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(起動中)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// 起動ﾓｰﾄﾞ = 通常起動
						else {
							// op_mod00(待機)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					// 検索タイプ通知
					case LCDBM_OPCD_SEARCH_TYPE:
						// 検索タイプセット
						OPECTL.f_searchtype = LcdRecv.lcdbm_rsp_notice_ope.status;

						// レーンモニタデータ送信(問合せ中)
						switch(LcdRecv.lcdbm_rsp_notice_ope.status){
							// 車番検索
							case SEARCH_TYPE_NO:
								// レーンモニタデータ登録
								ope_MakeLaneLog(LM_INQ_NO);
								break;

							// 時刻検索
							case SEARCH_TYPE_TIME:
								// レーンモニタデータ登録
								ope_MakeLaneLog(LM_INQ_TIME);
								break;

							// QR検索
							case SEARCH_TYPE_QR_TICKET:
								SetLaneMedia(stQRTicket.ParkingLotNo,
												stQRTicket.CardType,
												stQRTicket.byCardNo);

								// レーンモニタデータ登録
								ope_MakeLaneLog(LM_INQ_QR_TICKET);
								break;
							default:
								break;
						}
						break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
					// 遠隔精算（リアルタイム）開始要求
					case LCDBM_OPCD_RTM_REMOTE_PAY_REQ:
// GG129004(S) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
//						if (OPECTL.remote_wait_flg != 0 ||
//							OPECTL.f_rtm_remote_pay_flg != 0) {
						if (OPECTL.remote_wait_flg != 0) {
// GG129004(E) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
							// 遠隔精算（PIP）開始要求中
							// 遠隔精算（リアルタイム）開始要求受付済み
							// 遠隔精算（リアルタイム）開始NG
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 1 );
						}
						else {
							OPECTL.f_rtm_remote_pay_flg = 1;
							// 遠隔精算（リアルタイム）開始OK
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
						}
						break;
					// 遠隔精算（リアルタイム）受付中止
					case LCDBM_OPCD_RTM_REMOTE_PAY_CAN:
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							OPECTL.f_rtm_remote_pay_flg = 0;
						}
						break;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				}
				break;

			// 遠隔精算開始応答待ちタイムアウト
			case TIMEOUT_REMOTE_PAY_RESP:
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 				OPECTL.remote_wait_flg = 0;	// 問合せ中フラグクリア
// 				NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_BUSY );		// NG応答
				if ( OPECTL.remote_wait_flg == 1 ) {		// 遠隔精算(入庫時刻指定)
					OPECTL.remote_wait_flg = 0;						// 問合せ中フラグクリア
					NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_OTHER);	// NG応答
				} else if( OPECTL.remote_wait_flg == 2 ) {	// 遠隔精算(金額指定)
					OPECTL.remote_wait_flg = 0;						// 問合せ中フラグクリア
					NTNET_Snd_Data16_09(PIP_RES_RESULT_NG_OTHER);	// NG応答
				}
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(起動中)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// 初期化未完了状態とする
				ret = -1;
				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

			default:
				break;
		}
		mc10exec();
	}
   	if(OPECTL.Ope_mod != 2){					// 次精算フェーズじゃない
   		ope_anm( AVM_STOP );
   	}

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
//	Lagcan( OPETCBNO, 2 );											// ﾀｲﾏｰ2ﾘｾｯﾄ(ﾏﾙﾁ精算問合せ監視)
//	Lagcan( OPETCBNO, 4 );											// ﾀｲﾏｰ4ﾘｾｯﾄ(ｴﾗｰ表示用)
//	blink_end();													// 点滅終了
	Lagcan( OPETCBNO, TIMERNO_REMOTE_PAY_RESP );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH810105(S) MH364301 インボイス対応
	LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);							// ジャーナル印字完了待ちタイマ
	LagCan500ms(LAG500_RECEIPT_MISS_DISP_TIMER);					// 領収証失敗表示タイマ
// MH810105(E) MH364301 インボイス対応

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金表示,入金処理                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod02( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret(OPECTL.Fin_mod)                                     |*/
/*|                  1 = 精算完了(釣り無し) 精算完了処理へ切換             |*/
/*|                  2 = 精算完了(釣り有り) 精算完了処理へ切換             |*/
/*|                  3 = ﾒｯｸｴﾗｰ発生         精算完了処理へ切換             |*/
/*|                 10 = 精算中止           待機へ戻る                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 2007-02-26                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod02( void )
{
	short	ret;
	short	r_zero = -1;
	ushort	msg = 0;
	ushort	wk_MsgNo;
	uchar	Mifare_LastReadCardID[4];			// 最後に読んだｶｰﾄﾞIDを記憶しておき同一ｶｰﾄﾞ時は処理しない様にする
	uchar	syu_bk;

// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	short	credit_result = 0;
//	short	credit_turi;
//	short	credit_nyukin = 0;
//	uchar	credit_error_ari = 0;
//	uchar	credit_Timeout8Flag = 0;	// 入金開始から、一定時間再度入金が無い場合、自動中止する場合のクレジットとのからみ
//										// 入金途中であっても、クレジット問い合わせ中のタイムアップは無視し、
//										// 問い合わせＮＧで、タイムアップ済の場合に従来の処理を行うためのフラグ.
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	uchar	set03_98;					// 修正精算機能設定
//	char	pri_ann_flg = 0;
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	uchar	ryo_announce_flg = 0;	// 料金読上げ済みフラグ
	uchar	first_announce_flag = 0;
	uchar	nyukin_flag = 0;
// MH321800(S) D.Inaba ICクレジット対応
	uchar	announce_flag = 0;			// 取消押下時のｶｰﾄﾞ抜き取りｱﾅｳﾝｽ中フラグ
	uchar	announce_end = 0;			// 取消押下時のｶｰﾄﾞ抜き取りｱﾅｳﾝｽｴﾝﾄﾞフラグ
	ushort	ec_removal_wait_time;
// MH321800(E) D.Inaba ICクレジット対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	ushort cnt = 0;
	uchar type = 0;
	OPE_RESULT result = RESULT_NO_ERROR;
	ushort	res_wait_time;	// データ受信監視タイマ
// MH810100(S) K.Onodera 2020/2/18 #3870 初期画面に戻るまで精算できなくなってしまう
	uchar require_cancel_res = 0;
// MH810100(E) K.Onodera 2020/2/18 #3870 初期画面に戻るまで精算できなくなってしまう
// MH810100(S) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
	short retOnline = 0;
// MH810100(E) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	uchar	carno[36];
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

// MH810100(S) K.Onodera  2020/03/06 車番チケットレス(イベント発生の度に残高変化通知/操作通知が送信される不具合修正)
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
//	uchar cmp_send = 0;
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
// MH810100(E) K.Onodera  2020/03/06 車番チケットレス(イベント発生の度に残高変化通知/操作通知が送信される不具合修正)
// MH810102(S) 電子マネー対応 #5476 【市場不具合】精算ボタンを押しても「精算を開始できませんでした」となり開始できない
	uchar flag_stop = 0;		// 1:中止中
// MH810102(E) 電子マネー対応 #5476 【市場不具合】精算ボタンを押しても「精算を開始できませんでした」となり開始できない
// MH810105(S) MH364301 インボイス対応
	OPECTL.f_DelayRyoIsu = 0;
	OPECTL.f_CrErrDisp = 0;
// MH810105(E) MH364301 インボイス対応

// MH810101(S) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
	season_chk_result = 0;
// MH810101(E) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
	cmp_send = 0;
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102流用）（GM803003流用）
	r_zero_call = 0;												// 0円精算による表示短縮フラグ初期化
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102流用）（GM803003流用）

	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// データ受信監視タイマ
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// 範囲外は5秒とする
	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	FurikaeCancelFlg = 0;		// 振替精算失敗フラグ
//	FurikaeMotoSts = 0;		// 振替元状態フラグ 0:チェック不要 / 1:車あり / 2:車なし
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	OPECTL.InquiryFlg = 0;		// 外部照会中フラグOFF
	MifStat = MIF_WAITING;				// Mifare処理状態

	creInfoInit();		// クレジット情報クリア

	delay_count = 0;												// 未了発生中のﾒｯｾｰｼﾞｶｳﾝﾄのｸﾘｱ
	memset( nyukin_delay,0,sizeof( nyukin_delay ));					// 未了発生中のﾒｯｾｰｼﾞを保持領域のｸﾘｱ
	memset( &Settlement_Res,0,sizeof( Settlement_Res ));
// MH321800(S) hosoda ICクレジット対応
	coin_err_flg = 0;
	note_err_flg = 0;
	ec_flag_clear(0);												// 決済リーダ関連フラグクリア
// MH321800(E) hosoda ICクレジット対応

    edy_dsp.BYTE = 0;												// Edy画面表示用領域初期化
	cansel_status.BYTE = 0;											// 取消中の電子媒体停止状態初期化
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Pay_Work = 0;												// Edy決済額ﾜｰｸ領域
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	CCT_Cansel_Status.BYTE = 0;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ使用時の各電子媒体の停止状況
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	time_out = 0;
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// 未了タイムアウトフラグクリア
	Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;

	Suica_Rec.Data.BIT.PAY_CTRL = 0;								// 電子媒体決済済み判定ﾌﾗｸﾞｸﾘｱ※念のため
	Suica_rcv_split_flag_clear();

	Flap_Sub_Flg = 0;												// 詳細中止エリア使用フラグOFF
	Pay_Flg = 0;		//プリペイド、回数券使用ﾌﾗｸﾞｸﾘｱ

	NgCard = 0;														// NGカード読取内容クリア

	CreditReqResult = 0;											// クレジットカード問い合わせ結果情報変数初期化
	OPECTL.credit_in_mony = 0;
// MH810100(S) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
//	f_al_card_wait = 0;												// 電子媒体停止中のカード挿入フラグクリア
// MH810100(E) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 0;							// 未了無応答判定フラグクリア
	Product_Select_Data = 0;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Rec.edy_status.BIT.PRI_NG = 0;								// ジャーナルプリンタ使用不可ﾌﾗｸﾞｸﾘｱ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	Suica_Rec.Data.BIT.PRI_NG = 0;									// ジャーナルプリンタ使用不可ﾌﾗｸﾞｸﾘｱ

// MH321800(S) D.Inaba ICクレジット対応
	ec_removal_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 13, 3, 1);
	if(ec_removal_wait_time == 0) {									// 0の時、360をｾｯﾄ
		ec_removal_wait_time = 360;
	} else if( ec_removal_wait_time < 30 ){
		ec_removal_wait_time = 30;
	}
	ec_removal_wait_time *= 50;										// Xs * 1000ms / 20ms
// MH321800(E) D.Inaba ICクレジット対応

// MH810100(S) S.Nishimoto 2020/09/11 車番チケットレス (#4607 精算中に決済リーダのエラーが発生しても精算画面に「クレジット利用不可」が表示されない)
	Lagtim( OPETCBNO, 26, 3*50 );									// ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ(3s)START
// MH810100(E) S.Nishimoto 2020/09/11 車番チケットレス (#4607 精算中に決済リーダのエラーが発生しても精算画面に「クレジット利用不可」が表示されない)

	if( f_NTNET_RCV_MC10_EXEC ){									// mc10()更新が必要な設定が通信で更新された
		mc10();
		f_NTNET_RCV_MC10_EXEC = 0;
	}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	ryo_buf.lag_tim_over = 0;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH810100(S) 2020/06/17 #4497【連動評価指摘事項】Ai-Ⅴより入庫時刻あり割引ありの遠隔精算を行うと、不正な（前回精算時に適応した）割引を再適応して精算してしまう
	FLAPDT.flp_data[0].lag_to_in.BIT.LAGIN = 0x00;
// MH810100(E) 2020/06/17 #4497【連動評価指摘事項】Ai-Ⅴより入庫時刻あり割引ありの遠隔精算を行うと、不正な（前回精算時に適応した）割引を再適応して精算してしまう
// MH810100(S) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
	lcdbm_Flag_QRuse_ICCuse = 0;	// QRフラグクリア
// MH810100(E) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
	lcdbm_Flag_RemotePay_ICCuse = 0;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）

// MH322914 (s) kasiyama 2016/07/15 AI-V対応
//	/*** 入出庫時刻条理ﾁｪｯｸ ***/
//	if( set_tim( OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){				// 入出庫時刻NG?(Y)
//		ope_anm(AVM_STOP);											// 放送停止
//		BUZPIPI();
//		return( 10 );												// 精算中止(精算不可)とする。待機へ戻る
//	}
//	ryo_buf.credit.pay_ryo = 0;
//
//	ryo_cal( 0, OPECTL.Pr_LokNo );									// 料金計算

// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
// // MH810105(S) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
// 	// 精算開始時刻保持
// 	SetPayStartTime( &CLK_REC );
// // MH810105(E) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

// GG124100(S) R.Endo 2022/08/03 車番チケットレス3.0 #6343 クラウド料金計算対応
	if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
		ret = 0;

		switch ( OPECTL.CalStartTrigger ) {
		case CAL_TRIGGER_LCD_IN_TIME:		// 車番チケットレス精算(入庫時刻指定)
// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
			// 精算開始時刻保持
			SetPayStartTimeInCarInfo();
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

			// 入庫情報チェック(料金)
			retOnline = cal_cloud_fee_check();

			// 入庫時刻、出庫時刻(現在時間)、再計算用入車時刻をセット
			if ( !retOnline ) {
				if ( set_tim_only_out_card(1) ) {
					retOnline = 1;	// 精算不可
				}
			}

			// 結果判定
			switch ( retOnline ) {
			case 0:		// 処理成功
				// 車番チケットレス精算(入庫時刻指定)設定
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// 				in_time_set();
				in_time_set(1);	// 初回
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる

				// 入力された駐車位置番号
				key_num = (ushort)OPECTL.Op_LokNo;

				break;
			case 2:		// 精算済み
				lcdbm_pay_rem_chg(2);	// 精算残高変化通知(精算済み)送信
				OPECTL.Ope_mod = 230;	// 精算済み案内処理
				return 0;	// 継続
// GG124100(S) R.Endo 2022/09/28 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
			case 3:		// 精算済み(中止)
				lcdbm_pay_rem_chg(4);	// 精算残高変化通知(精算済(中止))送信
// GG129000(S) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.51(再修正))（GM803000流用）
				// 精算中のレーンモニタを出力していないので、ここで車番をセットする
				// 車番情報（UTF-8）（予備を抜いてコピーする）
				// 陸運支局名、分類番号
				memcpy(&carno[0], lcdbm_rsp_in_car_info_main.ShabanInfo.LandTransOfficeName, (size_t)(12+9));
				// 用途文字
				memcpy(&carno[(12+9)], lcdbm_rsp_in_car_info_main.ShabanInfo.UsageCharacter, (size_t)3);
				// 一連番号
				memcpy(&carno[(12+9+3)], lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber, (size_t)12);
				SetLaneFreeStr(carno, sizeof(carno));
				ope_MakeLaneLog(LM_PAY_STOP);
// GG129000(E) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.51(再修正))（GM803000流用）
				return 10;	// 精算中止(待機へ戻る)
// GG124100(E) R.Endo 2022/09/28 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
			case 1:		// 精算不可
			default:	// その他
				lcdbm_pay_rem_chg(3);	// 精算残高変化通知(精算不可)送信
				return 10;	// 精算中止(待機へ戻る)
			}

			break;
		case CAL_TRIGGER_REMOTECALC_FEE:	// 遠隔精算(金額指定)
// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
			// 精算開始時刻保持
			SetPayStartTime(&CLK_REC);
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
			// 入庫時刻、出庫時刻(現在時間)、再計算用入車時刻をセット
			set_tim_Lost(OPECTL.CalStartTrigger, OPECTL.Pr_LokNo, GetPayStartTime(), 0);	// エラーはない

			// 遠隔精算履歴登録
			wopelg(OPLOG_PARKI_ENKAKU, 0, 0);

			// 料金設定
			cal_cloud_fee_set_remote_fee();

			// 割引設定
			if ( g_PipCtrl.stRemoteFee.Discount ) {	// 割引金額あり
				cal_cloud_discount_set_remote_fee();
			}

			// 入力された駐車位置番号
			key_num = (ushort)OPECTL.Op_LokNo;

			// 遠隔精算金額指定応答
			NTNET_Snd_Data16_09(PIP_RES_RESULT_OK);

			break;
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
		case CAL_TRIGGER_REMOTECALC_TIME:	// 遠隔精算(入庫時刻指定)
// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
			// 精算開始時刻保持
			SetPayStartTimeInCarInfo();
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
			// 入庫時刻、出庫時刻(現在時間)、再計算用入車時刻をセット
			if ( set_tim_only_out(OPECTL.Pr_LokNo, GetPayStartTime(), 1) == 1 ) {
				// 放送停止
				ope_anm(AVM_STOP);

				// ブザー
				BUZPIPI();

				// 遠隔精算入庫時刻指定指定応答
				NTNET_Snd_Data16_08(PIP_RES_RESULT_NG_OTHER);

				// 精算残高変化通知(精算不可)送信
				lcdbm_pay_rem_chg(3);

				return 10;	// 精算中止(待機へ戻る)
			}

			// 遠隔精算履歴登録
			wopelg(OPLOG_PARKI_ENKAKU, 0, 0);

			// 料金設定
			cal_cloud_fee_set_remote_time();

			// 割引設定
			if ( g_PipCtrl.stRemoteTime.Discount ) {	// 割引金額あり
				cal_cloud_discount_set_remote_time();
			}

			// 入力された駐車位置番号
			key_num = (ushort)OPECTL.Op_LokNo;

			// 遠隔精算入庫時刻指定指定応答
			NTNET_Snd_Data16_08(PIP_RES_RESULT_OK);

			break;
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
		default:
			break;
		}
	} else {					// 通常料金計算モード
// GG124100(E) R.Endo 2022/08/03 車番チケットレス3.0 #6343 クラウド料金計算対応

	// 遠隔精算(金額指定)
	if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE ){
// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
		// 精算開始時刻保持
		SetPayStartTime(&CLK_REC);
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

		// 入出庫時刻条理チェック
		if( set_tim_Lost(OPECTL.CalStartTrigger, OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){		// 入出庫時刻NG?(Y)
			ope_anm( AVM_STOP );			// 放送停止
			BUZPIPI();
			NTNET_Snd_Data16_09( PIP_RES_RESULT_NG_CHANGE_NEXT );
			return( 10 );					// 精算中止(精算不可)とする。待機へ戻る
		}
		wopelg(OPLOG_PARKI_ENKAKU, 0, 0);	// 履歴登録
		ryo_buf.credit.pay_ryo = 0;
		ryo_cal( 7, OPECTL.Pr_LokNo );		// 料金計算
		// 割引情報あり？
		if( g_PipCtrl.stRemoteFee.Discount ){
			// 割引>残額
			if( g_PipCtrl.stRemoteFee.Discount > ryo_buf.zankin ){
				g_PipCtrl.stRemoteFee.Discount = ryo_buf.zankin;
			}
			vl_now = V_DIS_FEE;
			ryo_cal( 3, OPECTL.Pr_LokNo );		// サービス券として計算
			ret = in_mony(OPE_REQ_REMOTE_CALC_FEE, 0);
		}
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//		OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算
		// 出庫精算？
		if( !g_PipCtrl.stRemoteFee.Type ){
			OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算
		}
		// 後日精算？
		else{
			OpeNtnetAddedInfo.PayMethod = 13;	// 後日精算
		}
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
		OpeNtnetAddedInfo.PayMode = 4;
// 不具合修正(S) K.Onodera 2016/10/12 #1518 振替精算時と通常精算時で料金アナウンスの文言が異なる
		key_num = OPECTL.Op_LokNo;
// 不具合修正(E) K.Onodera 2016/10/12 #1518 振替精算時と通常精算時で料金アナウンスの文言が異なる
// 仕様変更(S) K.Onodera 2016/11/07 遠隔精算フォーマット変更対応
//		g_PipCtrl.stRemoteFee.RyoSyu = ryo_buf.syubet + 1;
// 仕様変更(E) K.Onodera 2016/11/07 遠隔精算フォーマット変更対応
		NTNET_Snd_Data16_09( PIP_RES_RESULT_OK );
	}
	// 遠隔精算(入庫時刻指定)
	else if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_TIME ){
// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
		// 精算開始時刻保持
		SetPayStartTime(&CLK_REC);
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

		// 入出庫時刻条理チェック
		if( set_tim_only_out(OPECTL.Pr_LokNo, &CLK_REC, 1) == 1 ){
			ope_anm( AVM_STOP );			// 放送停止
			BUZPIPI();
			NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_CHANGE_NEXT );
			return( 10 );					// 精算中止(精算不可)とする。待機へ戻る
		}
		wopelg(OPLOG_PARKI_ENKAKU, 0, 0);	// 履歴登録
		ryo_buf.credit.pay_ryo = 0;
		ryo_cal( 0, OPECTL.Pr_LokNo );		// 料金計算
		if( g_PipCtrl.stRemoteTime.Discount ){
			if( g_PipCtrl.stRemoteTime.Discount > ryo_buf.zankin ){
				g_PipCtrl.stRemoteTime.Discount = ryo_buf.zankin;
			}
			vl_now = V_DIS_TIM;
			ryo_cal( 3, OPECTL.Pr_LokNo );		// サービス券として計算
			ret = in_mony(OPE_REQ_REMOTE_CALC_TIME, 0);
		}
		OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算
		OpeNtnetAddedInfo.PayMode = 4;
// 不具合修正(S) K.Onodera 2016/10/12 #1518 振替精算時と通常精算時で料金アナウンスの文言が異なる
		key_num = OPECTL.Op_LokNo;
// 不具合修正(E) K.Onodera 2016/10/12 #1518 振替精算時と通常精算時で料金アナウンスの文言が異なる
		// 返信用情報セット
		g_PipCtrl.stRemoteTime.RyoSyu = ryo_buf.syubet + 1;
		g_PipCtrl.stRemoteTime.Price = ryo_buf.tyu_ryo;
		NTNET_Snd_Data16_08( PIP_RES_RESULT_OK );
	}

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_for_LCD_IN_CAR_INFO_料金計算
	// ====================== //
	// LCDから入庫情報受信
	// ====================== //
	else if( OPECTL.CalStartTrigger == CAL_TRIGGER_LCD_IN_TIME ){
// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
		// 精算開始時刻保持
		SetPayStartTimeInCarInfo();
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

		// 精算処理区分チェック
		FLAPDT.flp_data[0].lag_to_in.BIT.LAGIN = 0x00;

		if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1) ||		// 1=精算
			(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2) ){		// 2=再精算
			// 精算処理月日あり？
			if( (lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth > 0) &&
				(lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay > 0) ){

// MH810100(S) 2020/06/16 #4231 【連動評価指摘事項】ラグタイムオーバ後に出庫しているが、未精算出庫の再精算が無料になってしまう(No.02-0027)
//				// ラグタイム内？（精算時間 + ﾗｸﾞﾀｲﾑ時間 >= 現在時刻）
//// MH810100(S) K.Onodera 2020/03/05 #3962/#3912 未払い出庫後、再未払い出庫しようとしたときに有料になる
////				if( Is_in_lagtim() ){
//				// ラグタイム内？ or 未払い精算？
//				if( Is_in_lagtim() || lcdbm_rsp_in_car_info_main.shubetsu == 1 ){
//// MH810100(E) K.Onodera 2020/03/05 #3962/#3912 未払い出庫後、再未払い出庫しようとしたときに有料になる
//// MH810100(S) K.Onodera 2020/2/18 #3869 ラグタイム内の再精算実施時、精算済み案内表示されずに初期画面に遷移してしまう
//					lcdbm_pay_rem_chg(2);							// 残高変化通知（精算済み）送信
//// MH810100(S) K.Onodera  2020/03/05 車番チケットレス(精算完了画面表示時間が設定値に従わない不具合修正)
////					lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 2 );	// 精算完了通知(精算済み案内)
//// MH810100(E) K.Onodera  2020/03/05 車番チケットレス(精算完了画面表示時間が設定値に従わない不具合修正)
//// MH810100(E) K.Onodera 2020/2/18 #3869 ラグタイム内の再精算実施時、精算済み案内表示されずに初期画面に遷移してしまう
//					// op_mod230(精算済み案内)
//					OPECTL.Ope_mod = 230;
//					return(0);
//				}
//				// ラグタイムオーバー？（精算時間 + ﾗｸﾞﾀｲﾑ時間 < 現在時刻）
//				else {
//					// ryo_cal()内で
//					//		if( FLAPDT.flp_data[num-1].lag_to_in.BIT.LAGIN == ON ){ ryo_buf.lag_tim_over = 1; }しているので
//					//		以下のbitをsetする(num == OPECTL.Pr_LokNoの値)
//					FLAPDT.flp_data[0].lag_to_in.BIT.LAGIN = 0x01;
//				}

				// ラグタイム内？（精算時間(出庫時刻がある場合は出庫時刻と比較する) + ﾗｸﾞﾀｲﾑ時間 >= 現在時刻）
				if( Is_in_lagtim() ){
					lcdbm_pay_rem_chg(2);							// 残高変化通知（精算済み）送信
					// op_mod230(精算済み案内)
					OPECTL.Ope_mod = 230;
					return(0);
				}
				// ラグタイムオーバー？（精算時間(出庫時刻がある場合は出庫時刻と比較する)+ ﾗｸﾞﾀｲﾑ時間 < 現在時刻）
				else {
					// ryo_cal()内で
					//		if( FLAPDT.flp_data[num-1].lag_to_in.BIT.LAGIN == ON ){ ryo_buf.lag_tim_over = 1; }しているので
					//		以下のbitをsetする(num == OPECTL.Pr_LokNoの値)
					FLAPDT.flp_data[0].lag_to_in.BIT.LAGIN = 0x01;
				}
// MH810100(E) 2020/06/16 #4231 【連動評価指摘事項】ラグタイムオーバ後に出庫しているが、未精算出庫の再精算が無料になってしまう(No.02-0027)
			}
		}

// MH810105(S) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
// 		// 精算開始時刻保持
// 		SetPayStartTime( &CLK_REC );
// MH810105(E) R.Endo 2021/11/04 車番チケットレス3.0 #6147 出庫済み精算後に遠隔精算した場合、NT-NET精算データの処理日時と入庫日時が不正になる
// MH810100(S) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応
// MH810100(S) 2020/08/19 車番チケットレス(#4744 【検証課指摘事項】QR割引券(1h)が料金投入後の種別切替で無効になる(No56))
		// ↓のryo_calでクリアされるがここでもしておく。IsDupSyubetuwariで参照するため
		ryo_buf.nyukin = 0;
// MH810100(E) 2020/08/19 車番チケットレス(#4744 【検証課指摘事項】QR割引券(1h)が料金投入後の種別切替で無効になる(No56))
// MH810100(S) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
		g_checkKirikae = 0;								// 種別切替チェック済みフラグ
// MH810100(E) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
		// 複数の種別割引があるかどうかチェック
		if( IsDupSyubetuwari() == TRUE){
			// error時
			lcdbm_pay_rem_chg(3);		// 残高変化通知（精算不可）送信
			lcdbm_notice_dsp( POP_UNABLE_PAY, 0 );			// 8:精算不可
			return (10);
		}
// MH810100(E) 2020/08/03 #4563【連動評価指摘事項】種別割引の順番対応

		// 入庫時刻, 出庫時刻(現在時間), 再計算用入車時刻をセット
		if( set_tim_only_out_card( 1 ) == 1 ){
			// error時
			lcdbm_pay_rem_chg(3);		// 残高変化通知（精算不可）送信
			lcdbm_notice_dsp( POP_UNABLE_PAY, 0 );			// 8:精算不可
			return (10);
		}

		// ryo_cal()内で車番精算の時に参照する
		ryo_buf.credit.pay_ryo = 0;

		// ----------------------------------------------- //
		// ■料金計算
		//   ryo_cal()では車番精算は
		//				ec09()	req_rkn.param = RY_TSA
		//				et02() -> et40()		の順番でcallする
		// ----------------------------------------------- //
		// ryo_cal()内で以下のﾗｸﾞﾀｲﾑｵｰﾊﾞのﾁｪｯｸを行う
		//		if( FLAPDT.flp_data[num-1].lag_to_in.BIT.LAGIN == ON ){ ryo_buf.lag_tim_over = 1; }

		// 車番精算として計算
		ryo_cal( 8, OPECTL.Pr_LokNo );		// Pr_LokNo : 内部処理用駐車位置番号(1～324))
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:車種切換後の精算中止時、駐車料金/料金種別を切換前に戻す)
		RTPay_Data.crd_info.dtZaishaInfo.ParkingFee = ryo_buf.tyu_ryo;	// 初回料金計算時の駐車料金を格納
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:車種切換後の精算中止時、駐車料金/料金種別を切換前に戻す)
		ret = in_mony( OPE_REQ_LCD_CALC_IN_TIME, 0 );

		// ryo_cal()内でラグタイムオーバー検出？
		if( ryo_buf.lag_tim_over == 1 ){
			// ラグタイムオーバー後駐車料金 > 現金売上額
			if( ryo_buf.tyu_ryo > lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.GenkinFee ){
				// 入庫時刻から再計算した駐車料金から、現金売上額を引く
				ryo_buf.zankin = ryo_buf.tyu_ryo - lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.GenkinFee;
				// さらに、決済金額を引く
				for( cnt=0; cnt<ONL_MAX_SETTLEMENT; cnt++ ){
					// 決済区分あり？
					if( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeType ){
						if( ryo_buf.zankin > lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeFee ){
							ryo_buf.zankin -= lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeFee;
						}else{
							ryo_buf.zankin = 0;
							break;
						}
					}
				}
			}else{
				ryo_buf.zankin = 0;
			}
			// 表示料金と課税対象額にも同様の金額をセット
			ryo_buf.dsp_ryo = ryo_buf.kazei = ryo_buf.zankin;
// MH810100(S) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
//			// 前回精算で支払った現金/決済額をセット
//			ryo_buf.zenkai = (ryo_buf.tyu_ryo - ryo_buf.dsp_ryo);
// MH810100(E) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
		}

		// 未精算出庫精算？
		if( lcdbm_rsp_in_car_info_main.shubetsu == 1 ){
			OpeNtnetAddedInfo.PayMethod = 13;	// 後日精算
		}
		// 事前精算？
		else{
			OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算
		}

		OpeNtnetAddedInfo.PayMode = 0;		// 精算モード = 自動精算
		if(( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 1 ) ||	// 精算
		   ( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 2 ) ||	// 再精算
		   ( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType == 4 )){	// 再精算中止
			OpeNtnetAddedInfo.PayClass = 1;	// 処理区分 = 再精算
		}

		// 表示料金あり？
// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//		if( ryo_buf.dsp_ryo ){
		if( 1 ){
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応

			// ----------------------------------------------- //
			// 定期
			// ----------------------------------------------- //
			// 定期車？
// GG129000(S) ゲート式車番チケットレスシステム対応（改善連絡No.79)
//			if( isSeasonCardType( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo ) ){
			if( isSeasonCardType( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo ) ){
// GG129000(E) ゲート式車番チケットレスシステム対応（改善連絡No.79)
				// 受信定期データチェック（CRD_DATへの追加も行う）
// MH810101(S) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
//				if( CheckSeasonCardData( &lcdbm_rsp_in_car_info_main ) == SEASON_CHK_OK ){
				season_chk_result = CheckSeasonCardData( &lcdbm_rsp_in_car_info_main );
				if( season_chk_result == SEASON_CHK_OK ){
// MH810101(E) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
					al_pasck_set();							// PayDataに定期情報を書き込み
// MH810101(S) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応
					if ( OPECTL.PassNearEnd == 1 ) {
						season_chk_result = 100;
					}
// MH810101(E) R.Endo 2021/01/22 車番チケットレス フェーズ2 定期対応

					ryo_cal( 1, OPECTL.Pr_LokNo );			// 料金計算
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(仕様変更:車種切換後の精算中止時、駐車料金/料金種別を切換前に戻す)
					RTPay_Data.crd_info.dtZaishaInfo.ParkingFee = ryo_buf.tei_ryo;	// 格納した初回料金計算時の駐車料金を定期料金で上書き
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(仕様変更:車種切換後の精算中止時、駐車料金/料金種別を切換前に戻す)

					if( OPECTL.op_faz == 0 ){
						OPECTL.op_faz = 1;					// 入金中
					}

					ac_flg.cycl_fg = 10;					// 入金

					ret = in_mony( OPE_REQ_LCD_CALC_IN_TIME, 0 );
				}
			}
		}
// MH810100(S) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
		Set_PayInfo_from_InCarInfo();	// 入庫情報で受信した精算情報をセット
// MH810100(E) K.Onodera  2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)

		// 表示料金あり？
// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//		if( ryo_buf.dsp_ryo ){
		// サービスタイム内でも割引が乗ってきたら適応させる
		if( 1 ){
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応

			// ----------------------------------------------- //
			// 割引
			// ----------------------------------------------- //
			OnlineDiscount();

			key_num = OPECTL.Op_LokNo;			// ｷｰ入力値 <- 駐車位置番号
		}
// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
		// サービスタイム内でも乗せるのでここはコメントアウトOnlineDiscountでCOPYされる
//		else{
//			// ここでCOPYしておく（↑でOnlineDiscountでCOPYされないときに）
//			CopyDiscountCalcInfoToZaisha();
//		}
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_for_LCD_IN_CAR_INFO_料金計算

// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	// 振替精算
//	else if( PiP_GetFurikaeSts() ){
//		// 入出庫時刻条理チェック
//		if( set_tim( OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){
//			// 入出庫時刻NG?(Y)
//			ope_anm( AVM_STOP );			// 放送停止
//			BUZPIPI();
//			NTNET_Snd_Data16_04( PIP_RES_RESULT_OK_STOP );
//			return( 10 );					// 精算中止(精算不可)とする。待機へ戻る
//		}
//		wopelg(OPLOG_PARKI_FURIKAE, 0, 0);	// 履歴登録
//		ryo_buf.credit.pay_ryo = 0;
//		ryo_cal( 0, OPECTL.Pr_LokNo );		// 料金計算
//		// 振替開始処理
//		FurikaeMotoSts = 1;					// 振替元状態フラグ 1:車あり
//		OpeNtnetAddedInfo.PayMethod = 12;	// 精算方法=12:振替精算
//		// 定期有/無判定
//		if( vl_frs.antipassoff_req ){
//			// 定期利用有りなら定期投入後
//			ryo_buf.apass_off = 1;
//		}else{
//			ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
//			vl_now = V_FRK;						// 振替精算
//			ryo_cal( 3, OPECTL.Pr_LokNo );		// サービス券として計算
//		}
//// 不具合修正(S) K.Onodera 2016/10/12 #1518 振替精算時と通常精算時で料金アナウンスの文言が異なる
//		key_num = OPECTL.Op_LokNo;
//// 不具合修正(E) K.Onodera 2016/10/12 #1518 振替精算時と通常精算時で料金アナウンスの文言が異なる
//		// 返信用情報セット
//		g_PipCtrl.stFurikaeInfo.DestFeeKind = ryo_buf.syubet + 1;
//		g_PipCtrl.stFurikaeInfo.DestFee = ryo_buf.tyu_ryo;
//		// 正常応答
//		NTNET_Snd_Data16_04( PIP_RES_RESULT_OK );
//	}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	// その他
	else{
// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
		// 精算開始時刻保持
		SetPayStartTime(&CLK_REC);
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

		// 入出庫時刻情理チェック
		if( set_tim( OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){									// 入出庫時刻NG?(Y)
			ope_anm( AVM_STOP );			// 放送停止
			BUZPIPI();
			return( 10 );					// 精算中止(精算不可)とする。待機へ戻る
		}
		ryo_buf.credit.pay_ryo = 0;
		ryo_cal( 0, OPECTL.Pr_LokNo );		// 料金計算
	}
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応
	}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応

// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
	if (OPECTL.Seisan_Chk_mod == ON) {			// 精算ﾁｪｯｸﾓｰﾄﾞON
		// 精算額を強制的に百円にする
// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
//		ryo_buf.dsp_ryo = 100;
		autopay_coin_cnt = 0;					// 100円投入枚数クリア
		if( ryo_buf.svs_tim == 1 ){		// サービスタイム内の場合
			ryo_buf.dsp_ryo = 0;
		}
		else{
			ryo_buf.dsp_ryo = AP_PARKING_FEE;
			Lagtim( OPETCBNO, 28, AP_CHARGE_INTERVAL_SEC * 50);			// 100円投入タイマ
		}
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算
	}
#endif

	ac_flg.cycl_fg = 1;												// 1:料金表示中
	OPECTL.op_faz = 0;												// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ
	OPECTL.RECI_SW = 0;												// 領収書ﾎﾞﾀﾝ未使用
	RECI_SW_Tim = 0;												// 領収証受付時間ｸﾘｱ
	OPECTL.CAN_SW = 0;												// 取消ﾎﾞﾀﾝ
	OPECTL.PassNearEnd = 0;											// 期限切れ間近ﾌﾗｸﾞ
	OPECTL.LastUsePCardBal = 0L;									// 最後に利用したﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞの残額
	OPECTL.PriUsed = 0;
	ryodsp = 0;
	OPECTL.f_KanSyuu_Cmp = 0;										// 完了集計未実施
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	OPECTL.f_eReceiptReserve = 0;									// 電子領収証予約クリア
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	OPECTL.InLagTimeMode = 0;

// 仕様変更(S) K.Onodera 2016/11/04 振替精算時の領収証自動発行の設定を設ける
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
////	if( OPECTL.Pay_mod != 2 ){
////		ope_imf_Start();
////	}
//	// ParkingWeb経由の振替精算中？
//	if( PiP_GetFurikaeSts() ){
//		// 領収書 自動発行する？
//		if( prm_get( COM_PRM, S_CEN, 33, 1, 1 ) == 1 ){
//			// 領収書ﾎﾞﾀﾝ未使用？
//			if( OPECTL.RECI_SW == 0 ){
//				OPECTL.RECI_SW = 1;						// 領収書ﾎﾞﾀﾝ使用
//			}
//		}
//	}
//	// ParkingWeb経由の振替精算中でない？
//	else{
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	if( OPECTL.Pay_mod != 2 ){
		ope_imf_Start();
	}
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
// 仕様変更(E) K.Onodera 2016/11/04 振替精算時の領収証自動発行の設定を設ける
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
	// 遠隔精算？
	if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE || 
		OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_TIME ){
		// 領収書 自動発行する？
		if( prm_get( COM_PRM, S_CEN, 34, 1, 1 ) == 1 ){
			// 領収書ﾎﾞﾀﾝ未使用？
			if( OPECTL.RECI_SW == 0 ){
				OPECTL.RECI_SW = 1;						// 領収書ﾎﾞﾀﾝ使用
// MH810100(S) S.Fujii 2020/07/01 車番チケットレス(遠隔精算時の領収証自動発行)
				PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );	// 操作通知送信
// MH810100(E) S.Fujii 2020/07/01 車番チケットレス(遠隔精算時の領収証自動発行)
			}
		}
	}
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター

	/* 駐輪精算開始登録 */
	IoLog_write(IOLOG_EVNT_AJAST_STA, (ushort)LockInfo[OPECTL.Pr_LokNo - 1].posi, 0, 0);

	Ope_Last_Use_Card = 0;											// 最後に利用したカード初期化
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Card_Status_Prev = 0;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	dsp_change = 0;
	w_settlement = 0;


#if (4 == AUTO_PAYMENT_PROGRAM)						// 試験用に料金を変更する
	if( CPrmSS[S_SYS][5] != 0 )
	ryo_buf.tyu_ryo = ryo_buf.dsp_ryo = ryo_buf.zankin = ryo_buf.kazei = CPrmSS[S_SYS][5];
#endif																	// 中止券内 サ券・掛売券 割引額 get

	if( ryo_buf.pkiti != 0xffff ){
		syu_bk = (uchar)FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].bk_syu;
	}else{
		syu_bk = (uchar)LOCKMULTI.lock_mlt.bk_syu;
	}
// MH322914(S) K.Onodera 2016/09/13 AI-V対応 振替精算
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	// システムID16：振替精算？ 且つ、振替額が駐車料金以上？
//	if( PiP_GetFurikaeSts() && ryo_buf.dsp_ryo == 0 ){
//		ret = in_mony(OPE_REQ_FURIKAE_GO, 0);
//	}
//	else{
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
// MH322914(E) K.Onodera 2016/09/13 AI-V対応 振替精算
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	// 車番情報（UTF-8）（予備を抜いてコピーする）
	// 陸運支局名、分類番号
	memcpy(&carno[0], lcdbm_rsp_in_car_info_main.ShabanInfo.LandTransOfficeName, (size_t)(12+9));
	// 用途文字
	memcpy(&carno[(12+9)], lcdbm_rsp_in_car_info_main.ShabanInfo.UsageCharacter, (size_t)3);
	// 一連番号
	memcpy(&carno[(12+9+3)], lcdbm_rsp_in_car_info_main.ShabanInfo.SeqDesignNumber, (size_t)12);
	SetLaneFreeStr(carno, sizeof(carno));
	SetLaneFeeKind((ushort)(ryo_buf.syubet+1));
// GG129000(S) 改善連絡No.01 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
//	SetLaneFreeNum(ryo_buf.dsp_ryo);
	SetLaneFreeNum(lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo.ParkingFee);
// GG129000(E) 改善連絡No.01 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
// GG129000(S) H.Fujinaga 2023/01/26 ゲート式車番チケットレスシステム対応（遠隔精算対応）
	if(lcdbm_rsp_in_car_info_main.shubetsu != 2){
// GG129000(E) H.Fujinaga 2023/01/26 ゲート式車番チケットレスシステム対応（遠隔精算対応）
	switch( OPECTL.f_searchtype ){
	case SEARCH_TYPE_NO:		// 車番検索
	case SEARCH_TYPE_TIME:		// 時刻検索
		// レーンモニタデータ登録
		ope_MakeLaneLog(LM_PAY_NO_TIME);
		break;
	case SEARCH_TYPE_QR_TICKET:	// QR検索
		// レーンモニタデータ登録
		ope_MakeLaneLog(LM_PAY_QR_TICKET);
		break;
	default:				
		break;
	}
// GG129000(S) H.Fujinaga 2023/01/26 ゲート式車番チケットレスシステム対応（遠隔精算対応）
	}
	else {
		// 精算中変更データ適用時
		ope_MakeLaneLog(LM_PAY_CHANGE);
	}
// GG129000(E) H.Fujinaga 2023/01/26 ゲート式車番チケットレスシステム対応（遠隔精算対応）
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	if( ryo_buf.nyukin || ryo_buf.waribik || c_pay || syu_bk || e_pay){

		// 精算完了判定invcrd()と同様の処理とする
		if( ryo_buf.nyukin < ryo_buf.dsp_ryo ){						// 入金額 < 駐車料金?
			ryo_buf.zankin = ryo_buf.dsp_ryo - ryo_buf.nyukin;
		}
// MH321800(S) G.So ICクレジット対応
		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
		} else {
// MH321800(E) G.So ICクレジット対応
			Lagcan( OPETCBNO, 10 );									// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
// MH321800(S) G.So ICクレジット対応
		}
// MH321800(E) G.So ICクレジット対応
		ryodsp = 1;													// 料金表示
		OpeLcd( 4 );												// 料金表示
	}else{
		suica_fusiku_flg = 0;										// 利用可能媒体ｻｲｸﾘｯｸ表示有効
// MH322914 (s) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
		PrinterCheckForSuica();
// MH322914 (e) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
// MH810103 GG119202(S) コイン・紙幣のエラーチェック追加
		if (cn_errst[0] != 0) {
			coin_err_flg = 1;
		}
		if (cn_errst[1] != 0) {
			note_err_flg = 1;
		}
// MH810103 GG119202(E) コイン・紙幣のエラーチェック追加
		OpeLcd( 3 );												// 料金表示(初回)
		// サイクリック表示タイマ起動
		if (dspIsCyclicMsg()) {		// サイクリック判別/初期化
			DspChangeTime[0] = (ushort)(prm_get(COM_PRM, S_DSP, 30, 1, 1)); // 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ値取得
			if(DspChangeTime[0] <= 0) {
				DspChangeTime[0] = 2;
			}
// MH321800(S) G.So ICクレジット対応
			if (isEC_USE() != 0) {
				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
			} else {
// MH321800(E) G.So ICクレジット対応
				mode_Lagtim10 = 0;										// Lagtimer10の使用状態 0:利用可能媒体ｻｲｸﾘｯｸ表示
				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
// MH321800(S) G.So ICクレジット対応
			}
// MH321800(E) G.So ICクレジット対応
		}
	}

// GG129000(S) 改善連絡No.01 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
		if( ryo_buf.nyukin || ryo_buf.waribik || c_pay || e_pay ){
			// 入庫情報で事前精算等による割引や前回領収額がある場合は「精算中（事前精算：入金済み（割引含む））」を送信する
			SetLaneFeeKind(ryo_buf.syubet+1);
			SetLaneFreeNum(ryo_buf.zankin);
			ope_MakeLaneLog(LM_PAY_MNY_IN);
		}
// GG129000(E) 改善連絡No.01 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）

	// 0円精算
	if( ryo_buf.dsp_ryo == 0 ){
		r_zero = 0;													// 0円精算
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102流用）（GM803003流用）
		r_zero_call = 1;											// 0円精算による表示短縮フラグON
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102流用）（GM803003流用）
		ope_anm(AVM_STOP);											// 放送停止
// GG129000(S) 富士急ハイランド機能改善（画面表示時間短縮仕様の強化）（GM804102流用）（GM803003流用）
//		Lagtim( OPETCBNO, 1, 3*50 );								// ﾀｲﾏｰ1(3s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
		Lagtim( OPETCBNO, 1, 1*50 );								// ﾀｲﾏｰ1(1s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
// GG129000(E) 富士急ハイランド機能改善（画面表示時間短縮仕様の強化）（GM804102流用）（GM803003流用）
		OPE_red = 4;												// 精算完了
// MH810100(S) S.Takahashi 2020/03/02 割引額が遅れて表示される
		lcdbm_pay_rem_chg(1);							// 残高変化通知(精算完了)
// MH810100(E) S.Takahashi 2020/03/02 割引額が遅れて表示される
// GG129000(S) M.Fujikawa 2023/09/11 ゲート式車番チケットレスシステム対応（現金領収0円時は領収証を発行しない）
		QRIssueChk(0);
// GG129000(E) M.Fujikawa 2023/09/11 ゲート式車番チケットレスシステム対応（現金領収0円時は領収証を発行しない）
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
		lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 0 );				// 精算完了通知(0円表示)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	}
	// 0円以外精算
	else{
		r_zero = -1;
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102流用）（GM803003流用）
		r_zero_call = 0;											// 0円精算による表示短縮フラグOFF
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102流用）（GM803003流用）

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
		// 精算残高変化通知の送信処理
		lcdbm_pay_rem_chg(0);		// 精算ｽﾃｰﾀｽ(0:精算中,1:精算完了,2:精算済,3:精算不可)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// GG129000(S) 改善連絡No.01 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
//// GG129000(S) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.53)（GM803000流用）
//		if( ryo_buf.nyukin || ryo_buf.waribik || c_pay || e_pay ){
//			// 入庫情報で事前精算等による割引や前回領収額がある場合は「精算中（事前精算：入金済み（割引含む））」を送信する
//			SetLaneFeeKind(ryo_buf.syubet+1);
//			SetLaneFreeNum(ryo_buf.zankin);
//			ope_MakeLaneLog(LM_PAY_MNY_IN);
//		}
//// GG129000(E) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.53)（GM803000流用）
// GG129000(E) 改善連絡No.01 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）

		if( !SyuseiEndKeyChk() ){
			cn_stat( 1, 2 );									// 紙幣 & COIN入金可
		}

		nmicler( &SFV_DAT.refval, 8 );								// 不足額ｸﾘｱ

		OPE_red = 3;												// 精算中
		if(( RD_mod != 10 )&&( RD_mod != 11 )){
			opr_snd( 3 );											// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
		}

		if( SyuseiEndKeyChk() != 2 ){
			if( MIFARE_CARD_DoesUse ){									// Mifareが有効な場合
				op_MifareStart();										// Mifareを有効化
			}
		}


		if( SyuseiEndKeyChk() != 2 ){
			read_sht_opn();											// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
		}


		if( !SyuseiEndKeyChk() ){

// MH322914 (s) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
//				if( Ope_isJPrinterReady() != 1 || IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)){		// ジャーナル使用不可時
//						Suica_Rec.Data.BIT.PRI_NG = 1;
//						Edy_Rec.edy_status.BIT.PRI_NG = 1;
//				}
			PrinterCheckForSuica();
// MH322914 (e) kasiyama 2016/07/13 電子マネー利用可能表示修正[共通バグNo.1253](MH341106)
			Suica_Rec.Data.BIT.ADJUSTOR_START = 1;						// 精算開始フラグセット
			if( !Suica_Rec.Data.BIT.CTRL ){								// 受付不可状態？
// MH321800(S) T.Nagai ICクレジット対応
				if (isSX10_USE()) {
// MH321800(E) T.Nagai ICクレジット対応
				Suica_Ctrl( S_CNTL_DATA, 0x01 );										// Suica利用を可にする
// MH321800(S) T.Nagai ICクレジット対応
				}
				else {
					Ope_EcPayStart();									// 決済リーダ利用を可にする
				}
// MH321800(E) T.Nagai ICクレジット対応
			}else{
// MH321800(S) G.So ICクレジット対応
				if( isSX10_USE() ) {									// SX-10接続あり
// MH321800(E) G.So ICクレジット対応
				Suica_Ctrl( S_SELECT_DATA, 0 );							// 駐車料金を商品選択ﾃﾞｰﾀとして送信する
// MH321800(S) G.So ICクレジット対応
				}else{													// EC有効 and 複数決済有効
					Ope_EcPayStart();									// 決済リーダ利用を可にする
				}
// MH321800(E) G.So ICクレジット対応
			}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( Edy_Rec.edy_status.BIT.INITIALIZE ){				// Edyﾓｼﾞｭｰﾙの初期化が完了している？
//					Edy_SndData01();									// ｶｰﾄﾞ検知開始
//				}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		}

		w_settlement = 0;											// Suica残高不即時の精算額（０時は残高不足でないを意味する）
		ope_anm(AVM_SHASHITU);										// 車室読み上げ
		// 料金読み上げ処理
		if(ryo_announce_flg == 0){
			announceFee = ryo_buf.dsp_ryo;
			ope_anm(AVM_RYOUKIN);									// 「料金は○○○○○○円です」
			ryo_announce_flg = 1;									// 料金読上げ
		}
		if( OPECTL.Pay_mod != 2 ){									// 修正精算でない
			if( Ex_portFlag[EXPORT_CHGNEND] == 0 ){						// 0:釣銭切れ予告無し/1:釣銭切れ予告有り
				ope_anm( AVM_TURIARI );									// 料金表示時(釣銭あり状態)ｱﾅｳﾝｽ
			}else{
				ope_anm( AVM_TURINASI );								// 料金表示時(釣銭なし状態)ｱﾅｳﾝｽ
			}
		}
// MH810103 GG119202(S) 電子マネーシングル設定で案内放送を行う
		op_EcEmoney_Anm();
// MH810103 GG119202(E) 電子マネーシングル設定で案内放送を行う
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_削除
//			Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][65]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_削除
	}
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH322914(S) K.Onodera 2016/09/13 AI-V対応 振替精算
//	}
//// MH322914(E) K.Onodera 2016/09/13 AI-V対応 振替精算
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

	OPECTL.coin_syukei = 0;											// ｺｲﾝ金庫集計未出力とする
	OPECTL.note_syukei = 0;											// 紙幣金庫集計未出力とする

	dsp_fusoku = 0;													// ｱﾗｰﾑﾒｯｾｰｼﾞ表示ﾌﾗｸﾞﾘｾｯﾄ
	suica_fusiku_flg = 0;											//
// MH810100(S) K.Onodera 2020/03/11 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
	lcdbm_Flag_ResetRequest = 0;
// MH810100(E) K.Onodera 2020/03/11 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
// MH810103(s) 電子マネー対応 #5424 精算状態通知をOpe_modにより切り替える
	// 精算状態通知(Ope_modにより切り替わるので、mod02時点で送信しておく)Ope_EcPayStartの後でないと、利用不可となってしまう
	dspCyclicErrMsgRewrite();
// MH810103(e) 電子マネー対応 #5424精算状態通知をOpe_modにより切り替える


	for( ret = 0; ; ){
		msg = GetMessage();
		if( Ope_MiryoEventCheck( msg ) == 1 )
			continue;

		if( Ope_ArmClearCheck( msg ) == 1 )
			continue;
_MSG_RETRY:

		switch( msg ){
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(不要処理削除)
//			case KEY_TEN_F3:											// [強制精算完了]ｲﾍﾞﾝﾄ
//				if(	OPECTL.Pay_mod == 2 ){								// 修正精算の時
//					if( prm_get( COM_PRM,S_TYP,98,1,3 ) ){				// 強制精算完了ｷｰ有効
//						BUZPI();
//						cn_stat( 2, 2 );								// 入金不可
//						PayData.MMTwari = ryo_buf.dsp_ryo;
//						PayData.Zengakufg |= 0x02;						// 支払い掛け処理
//						ret = 1;
//					}
//				}
//				break;
//			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
//// MH810105(S) MH364301 インボイス対応
////				if(Ope_isPrinterReady() == 0){
//				if (Ope_isPrinterReady() == 0 ||
//					(IS_INVOICE && Ope_isJPrinterReady() == 0)) {		// インボイス設定時はレシートとジャーナルが片方でも動かなかった場合
//// MH810105(E) MH364301 インボイス対応
//					if( pri_ann_flg == 0 && OPECTL.InquiryFlg == 0){		// 問い合わせ中はアナウンスしない
//						pri_ann_flg = 1;
//						ope_anm( AVM_RYOUSYUU_NG );				// 只今、領収証の発行は出来ません
//					}
//					break;
//				}
//// MH810105(S) MH364301 QRコード決済対応
//				if( isEC_USE() && ECCTL.Ec_FailureContact_Sts != 0 ){	// 1:障害連絡票発行処理中／2:障害連絡票発行後
//					if (ECCTL.Ec_FailureContact_Sts == 2) {
//						// 障害連絡票発行済みの場合、領収証ボタンを受け付けない
//						break;
//					}
//					EcFailureContactPri();								// 障害連絡票発行 ※レシートのみ
//					op_SuicaFusokuOff();
//					// 6秒間表示する
//					lcd_wmsg_dsp_elec(1, ERR_CHR[78], ERR_CHR[79], 0, 0, COLOR_RED, LCD_BLINK_OFF);
//					LagTim500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER, 13, lcd_Recv_FailureContactData_edit);
//					ECCTL.Ec_FailureContact_Sts = 2;					// 2:障害連絡票発行後
//					if( OPECTL.RECI_SW == 0 ){							// 領収書ﾎﾞﾀﾝ未使用?
//						OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
//					}
//					break;
//				}
// MH810105(E) MH364301 QRコード決済対応
//// MH321800(S) hosoda ICクレジット対応
//				if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
//				// 未了確定後なら受け付けない
//					break;
//				}
//				if( announce_flag ){
//				// ｶｰﾄﾞ抜き取りｱﾅｳﾝｽ中なら受け付けない
//					break;
//				}
//// MH321800(E) hosoda ICクレジット対応
//				if( ReceiptChk() == 1 ){							// ﾚｼｰﾄ発行不要設定時
//					break;
//				}
//				if(	(OPECTL.Pay_mod == 2)&&							// 修正精算
//					(!prm_get(COM_PRM, S_PRN, 32, 1, 6))){			// 領収証の発行なし
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				if( OPECTL.RECI_SW == 0 ){							// 領収書ﾎﾞﾀﾝ未使用?
//					if((( OPECTL.ChkPassSyu == 0 )&&				// 問合せ中ではない?
//					   ( OPECTL.op_faz != 0 || ryodsp ))			// 待機状態以外?
//					   &&
//// MH321800(S) D.Inaba ICクレジット対応
////					   (ryo_buf.credit.pay_ryo == 0))				// クレジット問い合わせ中?
//							ec_MessagePtnNum == 0 ) 					// 決済リーダ関連文言表示中ではない？
//// MH321800(E) D.Inaba ICクレジット対応
//					{
//						wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
//						grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      領収証を発行します      "
//// MH810105(S) MH364301 インボイス対応
//						OPECTL.f_CrErrDisp = 0;						// エラー非表示
//// MH810105(E) MH364301 インボイス対応
//					}
//					OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
//				}
//				break;
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(不要処理削除)
			case COIN_EVT:											// Coin Mech event
				if( OPECTL.op_faz == 4 ){							// ﾀｲﾑｱｳﾄ
					if(( OPECTL.CN_QSIG == 7 )&&( ryo_buf.nyukin == 0 )){	// 払出可かつ入金額がなかったら
						if( !Op_Cansel_Wait_sub( 0xff ) )				// 全デバイスのキャンセル済み？
							Op_Cansel_Wait_sub( 2 );					// メックの停止
						break;
					}else{
						OPECTL.op_faz = 0;
						cn_stat( 3, 2 );							// 入金可
						Lagcan( OPETCBNO, 2 );						// ﾒｯｸ監視ﾀｲﾏｰｷｬﾝｾﾙ
					}
				}
				if (( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 ) ){
					nyukin_flag = 1;
				}
			case NOTE_EVT:											// Note Reader event
				if(( r_zero == 0 )&&( OPECTL.cnsend_flg == 1 )){	// 0円精算＆入金可能状態
					r_zero = -1;
					OPECTL.op_faz = 1;								// 入金中
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				}
				if ( (msg == NOTE_EVT )&&( OPECTL.NT_QSIG == 1 ) ){
					nyukin_flag = 1;
				}
			case COIN_RJ_EVT:										// 精算中止
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ﾘｼﾞｪｸﾄﾚﾊﾞｰ_COIN_RJ_EVT
// MH810100(S) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
//				// ﾘｼﾞｪｸﾄﾚﾊﾞｰ_COIN_RJ_EVT
//				if( msg == COIN_RJ_EVT ){
			case TIMEOUT8:
				// リジェクトレバー or 入金後戻り判定タイムアウト 
				if( msg == COIN_RJ_EVT || msg == TIMEOUT8 ){
// MH810100(E) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
// MH810100(S) K.Onodera  2020/02/28 #3974 リジェクトレバーで中止後、精算開始できない
//					// 決済リーダのカード処理中？
//					if( isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ){
//						break;
//					}
//// MH810100(S) K.Onodera  2020/02/27 #3945 満額時のリジェクトレバー操作で初期画面へ遷移してしまう
//					if( OPECTL.op_faz < 2 ){
//// MH810100(E) K.Onodera  2020/02/27 #3945 満額時のリジェクトレバー操作で初期画面へ遷移してしまう
//						// 中止応答待ちじゃない？
//						if( stp_wait_flg == 0 ){
//							// 精算中止応答OK待ちﾘﾄﾗｲｶｳﾝﾀ
//							lcdbm_Counter_PayStopRetry = 0;
//							// 精算中止要求送信
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP, 0 );
//							// 精算中止応答待ちタイマ
//							Lagtim( OPETCBNO, TIMERNO_PAY_STOP_RESP, (res_wait_time * 50) );
//							stp_wait_flg = 1;
//						}
//// MH810100(S) K.Onodera  2020/02/27 #3945 満額時のリジェクトレバー操作で初期画面へ遷移してしまう
//					}
//// MH810100(E) K.Onodera  2020/02/27 #3945 満額時のリジェクトレバー操作で初期画面へ遷移してしまう
// MH810105(S) MH364301 QRコード決済対応 GT-4100 #6398 電子マネー、QRコード読取時の処理中ポップアップが時間経過でとじてしまう
//					// リジェクトレバー通知
//					PKTcmd_notice_ope( LCDBM_OPCD_REJECT_LEVER, (ushort)0 );
//// MH810100(E) K.Onodera  2020/02/28 #3974 リジェクトレバーで中止後、精算開始できない
//					break;
					if( msg == TIMEOUT8 ){								// 入金後戻り判定ﾀｲﾏｰﾀｲﾑｱｳﾄ?
						if(OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8 ){ 			//定期通信チェック中か電子媒体停止中
							Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
							break;											//キャンセル無効
						}
						if(ECCTL.Ec_FailureContact_Sts == 1){
							// 障害連絡票発行中はとりけし操作を受け付けない
							break;
						}
						// 決済リーダのカード処理中は取消操作を受け付けない
						if(isEC_STS_CARD_PROC()) {
							break;
						}
						if (isEC_MIRYO_TIMEOUT()) {
							// 未了残高照会タイムアウト後は無操作タイムアウトを受け付けない
							break;
						}
// MH810105(S) QRコード決済対応 GT-4100 #6591 精算中、待機画面復帰タイマー経過後もLCD画面が料金表示したままとなる
//						msg = KEY_TEN_F4;							// 自動的に精算中止とする
//						PayInfo_Class = 2;							// 精算情報データの処理区分を精算中止にする
//						SetNoPayData();								// 精算中止データを設定する
//						Cansel_Ele_Start();
//						time_out = 1;									// ﾀｲﾑｱｳﾄ発生ﾌﾗｸﾞON 07-01-22追加
//					}else{
//						// リジェクトレバー通知
//						PKTcmd_notice_ope( LCDBM_OPCD_REJECT_LEVER, (ushort)0 );
//						break;
//					}
						// リジェクトレバー通知
						PKTcmd_notice_ope( LCDBM_OPCD_REJECT_LEVER, (ushort)0 );
						break;
					}else{
						if(OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8 ){ 			//定期通信チェック中か電子媒体停止中
							break;											//キャンセル無効
						}
						if( OPECTL.op_faz == 9 ) {
							break;
						}
						if(ECCTL.Ec_FailureContact_Sts == 1){
							// 障害連絡票発行中はとりけし操作を受け付けない
							break;
						}
						// 決済リーダのカード処理中は取消操作を受け付けない
						if(isEC_STS_CARD_PROC()) {
							break;
						}
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							break;
						}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
							// リジェクトレバー通知
						PKTcmd_notice_ope( LCDBM_OPCD_REJECT_LEVER, (ushort)0 );
						break;
					}
// MH810105(E) QRコード決済対応 GT-4100 #6591 精算中、待機画面復帰タイマー経過後もLCD画面が料金表示したままとなる
// MH810105(E) MH364301 QRコード決済対応 GT-4100 #6398 電子マネー、QRコード読取時の処理中ポップアップが時間経過でとじてしまう
				}
				// no break;

			// op_Payment_Cancel(新規作成rev.1100)の処理を行わないで従来の処理を行う
			case LCD_LCDBM_OPCD_PAY_STP:			// 精算中止要求受信
			case LCD_LCDBM_OPCD_STA_NOT:			// 起動通知受信時
			case LCD_LCD_DISCONNECT:				// リセット通知/切断検知
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ﾘｼﾞｪｸﾄﾚﾊﾞｰ_COIN_RJ_EVT
// MH810100(S) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
//			case TIMEOUT8:											// 入金後戻り判定ﾀｲﾏｰﾀｲﾑｱｳﾄ
// MH810100(E) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
// MH810100(S) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
//			case CANCEL_EVENT:										// キャンセル発生
// MH810100(E) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
				fus_subcal_flg = 1;
// MH322914 (s) kasiyama 2016/07/11 入金＋クレジットの取消で画面ロック対策[共通バグNo.1002](MH324709)
//				if( msg == KEY_TEN_F4 && creSeisanInfo.amount != 0L){// 取消ﾎﾞﾀﾝ ON and クレジット精算中
//					break;
//				}
// MH322914 (e) kasiyama 2016/07/11 入金＋クレジットの取消で画面ロック対策[共通バグNo.1002](MH324709)
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//				if( msg == KEY_TEN_F4 || msg == CANCEL_EVENT|| msg == COIN_RJ_EVT){	// 取消ﾎﾞﾀﾝ?(Y) or キャンセル or 精算中止
// MH810100(S) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
//				if((  msg == KEY_TEN_F4 || msg == CANCEL_EVENT|| msg == COIN_RJ_EVT) ||
				if( ( msg == KEY_TEN_F4 || msg == COIN_RJ_EVT) ||
// MH810100(E) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
					( msg == LCD_LCDBM_OPCD_PAY_STP ) 		||		// 精算中止要求受信時
					( msg == LCD_LCDBM_OPCD_STA_NOT ) 		||		// 起動通知受信時
					( msg == LCD_LCD_DISCONNECT ) ){				// リセット通知/切断検知
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH322914 (s) kasiyama 2016/07/11 入金＋クレジットの取消で画面ロック対策[共通バグNo.1002](MH324709)
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//					if( creSeisanInfo.amount != 0L){// クレジット精算中
//						break;
//					}
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// MH810105(S) MH364301 QRコード決済対応
					if(ECCTL.Ec_FailureContact_Sts == 1){
						// 障害連絡票発行中はとりけし操作を受け付けない
						break;
					}
// MH810105(E) MH364301 QRコード決済対応
// MH321800(S) Y.Tanizaki ICクレジット対応
					// 決済リーダのカード処理中は取消操作を受け付けない
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//					if(isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC) {
					if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
						break;
					}
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH322914 (e) kasiyama 2016/07/11 入金＋クレジットの取消で画面ロック対策[共通バグNo.1002](MH324709)
					if(OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8){ 		//定期通信チェック中か電子媒体停止中
						break;											//キャンセル無効
					}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					if( dsp_fusoku || edy_dsp.BIT.edy_dsp_Warning ){
					if( dsp_fusoku ){
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) G.So ICクレジット対応
						if (isEC_USE() != 0) {
							Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );
// MH321800(S) Y.Tanizaki ICクレジット対応(共通改善No.1362/精算中止と同時に電子マネータッチで、カード引去りするが精算中止してしまう不具合対策)
							Target_MsgGet_delete1( OPETCBNO, EC_CYCLIC_DISP_TIMEOUT );
// MH321800(E) Y.Tanizaki ICクレジット対応(共通改善No.1362/精算中止と同時に電子マネータッチで、カード引去りするが精算中止してしまう不具合対策)
						} else {
// MH321800(E) G.So ICクレジット対応
							Lagcan( OPETCBNO, 10 );
// MH321800(S) Y.Tanizaki ICクレジット対応(共通改善No.1362/精算中止と同時に電子マネータッチで、カード引去りするが精算中止してしまう不具合対策)
							Target_MsgGet_delete1( OPETCBNO, TIMEOUT10 );
// MH321800(E) Y.Tanizaki ICクレジット対応(共通改善No.1362/精算中止と同時に電子マネータッチで、カード引去りするが精算中止してしまう不具合対策)
// MH321800(S) G.So ICクレジット対応
						}
// MH321800(E) G.So ICクレジット対応
					}
					if( OPECTL.op_faz == 9 ) {
						break;
					}
					if (msg == KEY_TEN_F4) {
						BUZPI();
					} else {
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//						// 振替元車出庫の場合、以下の処理は取消ボタン処理と同じ
//						msg = KEY_TEN_F4;
						if (( msg != LCD_LCDBM_OPCD_PAY_STP ) 		&&		// 精算中止要求受信時
							( msg != LCD_LCDBM_OPCD_STA_NOT ) 		&&		// 起動通知受信時
							( msg != LCD_LCD_DISCONNECT ) ){ 				// リセット通知/切断検知
								// 振替元車出庫の場合、以下の処理は取消ボタン処理と同じ
								msg = KEY_TEN_F4;
						}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
					}
					PayInfo_Class = 2;								// 精算情報データの処理区分を精算中止にする
					SetNoPayData();								// 精算中止データを設定する
					ope_anm( AVM_STOP );							// 放送停止ｱﾅｳﾝｽ
					Cansel_Ele_Start();
				}
// MH810100(S) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
//				if( msg == TIMEOUT8 ){								// 入金後戻り判定ﾀｲﾏｰﾀｲﾑｱｳﾄ?
//					if(OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8 ){ 			//定期通信チェック中か電子媒体停止中
//						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
//						break;											//キャンセル無効
//					}
//// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
////					if ( creSeisanInfo.amount != 0L ){ 				// ちなみに、クレジット精算開始時は入金不可にすることで、
////						credit_Timeout8Flag = 1;					// NOTE_EVT が発生するので、 CPrmSS[S_TYP][66] 秒 Lagtimされる.
////					}else{
//					{
//// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//						msg = KEY_TEN_F4;							// 自動的に精算中止とする
//						PayInfo_Class = 2;							// 精算情報データの処理区分を精算中止にする
//						SetNoPayData();								// 精算中止データを設定する
//					}
//					Cansel_Ele_Start();
//					time_out = 1;									// ﾀｲﾑｱｳﾄ発生ﾌﾗｸﾞON 07-01-22追加
//				}
// MH810100(E) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
				if( r_zero == 0 ){
					if( OPECTL.Pay_mod != 2 ){
						break;
					}
				}
				if( (first_announce_flag == 0)&&(nyukin_flag == 1)) { // 未アナウンス、入金あり
					first_announce_flag = 1;

					if (OPECTL.op_faz == 0){
						OPECTL.op_faz = 1;								// 入金中
					}
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) hosoda ICクレジット対応
////					if( OPECTL.Pay_mod != 2 ){						// 修正精算でない
//					if ((OPECTL.Pay_mod != 2)						// 修正精算でない
//					&&	(Suica_Rec.Data.BIT.MIRYO_TIMEOUT == 0)) {	// 未了確定していない
//// MH321800(E) hosoda ICクレジット対応
//							ope_anm( AVM_RYOUSYUU );						// 現金受付時及び領収証ｱﾅｳﾝｽ
//					}											// 修正精算でない
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				}
// MH810104 GG119202(S) みなし休業で自動とりけしされない
				if (msg == KEY_TEN_F4 || msg == COIN_RJ_EVT) {
// MH810104 GG119202(E) みなし休業で自動とりけしされない
// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
				LagCan500ms(LAG500_EC_AUTO_CANCEL_TIMER);		// アラーム取引後の精算自動キャンセルタイマリセット
// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
// MH810104 GG119202(S) みなし休業で自動とりけしされない
				}
// MH810104 GG119202(E) みなし休業で自動とりけしされない

				/*** 入金後XXsで精算中止とする ***/
				Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
				ret = in_mony( msg, 0 );							// 入金処理ｻﾌﾞ

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ｺｲﾝ_紙幣投入
				switch(msg) {
					// 精算中止要求受信時
					case LCD_LCDBM_OPCD_PAY_STP:
// MH810100(S) K.Onodera 2020/2/18 #3870 初期画面に戻るまで精算できなくなってしまう
//						// 中止中
//						if (OPECTL.op_faz == 3) {
//							// 操作通知(精算中止応答(OK))送信	0=OK/1=NG
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
//						}
//						// OPECTL.op_faz != 3
//						else {
//							// 操作通知(精算中止応答(NG))送信	0=OK/1=NG
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
//						}
						require_cancel_res = 1;
// MH810100(E) K.Onodera 2020/2/18 #3870 初期画面に戻るまで精算できなくなってしまう
						break;

					// 起動通知受信時
					case LCD_LCDBM_OPCD_STA_NOT:
						// 中止中
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
//// MH810100(S) K.Onodera  2020/02/25 #3858 車番チケットレス(起動待機状態無限ループ防止)
////						if (OPECTL.op_faz == 3) {
//						if((OPECTL.op_faz == 3) && ( OPECTL.Ope_mod != 0 )){	// 精算中止処理中でop_mod00への遷移待ちではない？
//// MH810100(E) K.Onodera  2020/02/25 #3858 車番チケットレス(起動待機状態無限ループ防止)
//							// 0=起動
//							if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
//								OPECTL.init_sts = 0xff;	// 初期化未完了(起動通知受信済み)状態とする
//							}
//// MH810100(S) K.Onodera  2020/02/25 #3858 車番チケットレス(起動待機状態無限ループ防止)
//							// 1=通常
//							else{
//								OPECTL.init_sts = 0xfe;	// 初期化完了済み(起動通知受信済み)状態とする
//							}
//// MH810100(E) K.Onodera  2020/02/25 #3858 車番チケットレス(起動待機状態無限ループ防止)
						if((OPECTL.op_faz == 3) && ( OPECTL.init_sts != 1 )){	// 精算中止処理中で初期化完了済みではない？
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
							OPECTL.Ope_mod = 255;
						}
						break;

					// LCD_DISCONNECT受信時にICｸﾚｼﾞｯﾄ非問合せ中の時に発行
					case LCD_LCD_DISCONNECT:
						// 中止中
						if( OPECTL.op_faz == 3 ){
							// op_init00(起動中)
							OPECTL.Ope_mod = 255;
							OPECTL.init_sts = 0;	// 初期化未完了状態とする
						}
						break;

					default:
						break;
				}		// switch(msg) {
				// 以下は共通処理

				// ｺｲﾝ_紙幣投入
				if (( msg == COIN_EVT ) || ( msg == NOTE_EVT )) {
					// 精算完了？
					if( !ryo_buf.zankin ){
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
//// MH810100(S) K.Onodera  2020/03/06 車番チケットレス(イベント発生の度に残高変化通知/操作通知が送信される不具合修正)
//						if( !cmp_send ){	// 残高変化通知(精算完了)/操作通知(精算完了通知(精算済み案内))未送信
//// MH810100(E) K.Onodera  2020/03/06 車番チケットレス(イベント発生の度に残高変化通知/操作通知が送信される不具合修正)
//// MH810100(S) S.Takahashi 2020/03/02 割引額が遅れて表示される
//							lcdbm_pay_rem_chg(1);							// 残高変化通知(精算完了)
//// MH810100(E) S.Takahashi 2020/03/02 割引額が遅れて表示される
//							// 操作通知(精算完了通知(精算済み案内))送信
//							// 0=0円表示/1=ﾗｸﾞﾀｲﾑ延長画面/2=精算済み案内
//							lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 2 );
//// MH810100(S) K.Onodera  2020/03/06 車番チケットレス(イベント発生の度に残高変化通知/操作通知が送信される不具合修正)
//							cmp_send = 1;
//						}
//// MH810100(E) K.Onodera  2020/03/06 車番チケットレス(イベント発生の度に残高変化通知/操作通知が送信される不具合修正)
						if ( cmp_send != 1 ) {		// 同種の残高変化通知(精算完了)未送信
							lcdbm_pay_rem_chg(1);	// 残高変化通知(精算完了)
							cmp_send = 1;
						}
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
					}else{
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(#3995 精算中止すると、精算確認画面の音声メッセージが再生されてしまう)
// MH810100(S) K.Onodera 2020/03/18 車番チケットレス(#4067 紙幣挿入で残高変化通知が送信されない)
//						if( ( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 ) ){
						if( (( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 ))  ||
							( (OPECTL.NT_QSIG == 1 )||( OPECTL.NT_QSIG == 5 )) ){
// MH810100(E) K.Onodera 2020/03/18 車番チケットレス(#4067 紙幣挿入で残高変化通知が送信されない)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(#3995 精算中止すると、精算確認画面の音声メッセージが再生されてしまう)
							lcdbm_pay_rem_chg(0);		// 残高変化通知(精算中)
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(#3995 精算中止すると、精算確認画面の音声メッセージが再生されてしまう)
// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
							Lagtim( OPETCBNO, 28, AP_CHARGE_INTERVAL_SEC * 50);	// 100円投入タイマ
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算
						}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(#3995 精算中止すると、精算確認画面の音声メッセージが再生されてしまう)
					}
				}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ｺｲﾝ_紙幣投入
				break;

// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
			case AUTO_CANCEL:
// MH810100(S) K.Onodera 2020/03/11 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
				if( lcdbm_Flag_ResetRequest ){	// 精算中止要因発生？
					// 精算中止処理後、起動待機状態に遷移する(OPECTL.init_stsは要因に応じた値に変更済み)
					// 但し、意図的に待機状態に遷移させる場合は起動待機状態には遷移しない
					if( OPECTL.Ope_mod != 0 ){
						OPECTL.Ope_mod = 255;
					}
				}
// MH810100(E) K.Onodera 2020/03/11 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
				msg = KEY_TEN_F4;
				goto _MSG_RETRY;
// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			case IBK_EDY_RCV:															// Edyﾓｼﾞｭｰﾙからの受信ﾃﾞｰﾀ？
//				Ope_Edy_Event( msg, OPECTL.Ope_mod );
//				break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			case ELE_EVT_STOP:											// 電子媒体停止完了受信
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( edy_dsp.BIT.edy_Miryo_Loss || 
//				  ( Suica_Rec.Data.BIT.MIRYO_ARM_DISP && !SUICA_CM_BV_RD_STOP ) ) // 引去り失敗後のﾒｯｾｰｼﾞ表示中？
				if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP && !SUICA_CM_BV_RD_STOP ) // 引去り失敗後のﾒｯｾｰｼﾞ表示中？
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					op_SuicaFusokuOff();								// ブリンク表示とかぶるのでﾒｯｾｰｼﾞ消去
				switch( CCT_Cansel_Status.BIT.STOP_REASON ){			// 停止要因
					case REASON_PAY_END:								// 精算完了時の電子媒体停止待ち合わせ
// 仕様変更(S) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
//						if ( ryo_buf.turisen == 0 ){
						if ( ryo_buf.turisen == 0 && ryo_buf.kabarai == 0 ){
// 仕様変更(E) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
							ret = 1;												// 精算終了(釣無し)
						}else{
							ret = 2;												// 精算終了(釣有り)
						}
// MH810103(s) 電子マネー対応 未了通知
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
//						lcdbm_notice_ope(LCDBM_OPCD_PAY_CMP_NOT, 2);	// 0=0円表示/1=ﾗｸﾞﾀｲﾑ延長画面/2=精算済み案内
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
// MH810103(e) 電子マネー対応 未了通知
						#if (3 == AUTO_PAYMENT_PROGRAM)
						BUZPIPIPI();
						#endif
						break;
					case REASON_CARD_READ:								// カード挿入時の待ち合わせ処理時
						if( (Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
								  OPECTL.op_faz == 3 || 
								  ryo_buf.zankin == 0 ){
							opr_snd( 13 );						// 前排出
						}else{
							msg = ARC_CR_R_EVT;
							CCT_Cansel_Status.BYTE = 0;						// 電子媒体停止管理領域の初期化
							goto OPMOD_ARC_CR_R_EVT_DUMMY;
						}
						break;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
					// QRﾃﾞｰﾀ受信時にICｸﾚｼﾞｯﾄ停止したいという理由
					case REASON_QR_USE:
// MH810103(s) 電子マネー対応
//						// ■ICｸﾚｼﾞｯﾄ設定あり
//						// 02-0024 ①:EC決済ﾘｰﾀﾞ	0=接続なし/1=接続あり
//						if (prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1 ) {
						if(isEC_USE()) {
// MH810103(e) 電子マネー対応
							// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ停止
							Lagcan(OPETCBNO, TIMERNO_ICCREDIT_STOP);

							// QRﾌﾗｸﾞ== 利用確認中	(0 = idle, 1 = 利用確認中, 2 = 利用中)
							if (lcdbm_Flag_QRuse_ICCuse == 1) {
								// QRﾃﾞｰﾀ応答(OK))送信	結果(0:OK,1:NG(排他),2:NG(枚数上限))
								lcdbm_QR_data_res( 0 );							// 結果(0:OK,1:NG(排他),2:NG(枚数上限))

// MH810100(S) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
								// QRﾃﾞｰﾀ受信 & QRﾃﾞｰﾀ応答(OK)送信後のLCD_IN_CAR_INFO待ち
								lcdbm_QRans_InCar_status = 1;
// MH810100(E) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる

								// QRﾌﾗｸﾞ←利用中
								lcdbm_Flag_QRuse_ICCuse = 2;
							}
						}
						// ICｸﾚｼﾞｯﾄ設定なし
						else {
							// nothing todo
						}
						// REASON_CARD_READと同等の処理を行う
						if( (Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
								  OPECTL.op_faz == 3 || 
								  ryo_buf.zankin == 0 ){
							opr_snd( 13 );						// 前排出
						}else{
// MH810100(S) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
//							msg = ARC_CR_R_EVT;
//							CCT_Cansel_Status.BYTE = 0;						// 電子媒体停止管理領域の初期化
//							goto OPMOD_ARC_CR_R_EVT_DUMMY;
							CCT_Cansel_Status.BYTE = 0;						// 電子媒体停止管理領域の初期化
// MH810100(E) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
						}
						break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
					case REASON_RTM_REMOTE_PAY:							// 遠隔精算開始
						// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ停止
						Lagcan(OPETCBNO, TIMERNO_ICCREDIT_STOP);
						// 決済リーダ停止確認中
						if (lcdbm_Flag_RemotePay_ICCuse == 1) {
							// 受付済み
							OPECTL.f_rtm_remote_pay_flg = 1;
							// 遠隔精算（リアルタイム）開始OK
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
							// 遠隔精算開始OK
							lcdbm_Flag_RemotePay_ICCuse = 2;
						}
						// REASON_CARD_READと同等の処理を行う
						if( (Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
								OPECTL.op_faz == 3 || 
								ryo_buf.zankin == 0 ){
						}else{
							CCT_Cansel_Status.BYTE = 0;					// 電子媒体停止管理領域の初期化
						}
						break;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
					case REASON_MIF_READ:								// Mifareタッチ時の待ち合わせ処理時
						if( (Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
								  OPECTL.op_faz == 3 || 
								  ryo_buf.zankin == 0 ){
							op_MifareStop_with_LED();					// Mifare停止
						}else{
							msg = IBK_MIF_A2_OK_EVT;
							CCT_Cansel_Status.BYTE = 0;						// 電子媒体停止管理領域の初期化
							goto OPMOD_MIF_A2_OK_DUMMY;
						}
						break;
					case REASON_MIF_WRITE_LOSS:							// Mifare書込み失敗
						OpMif_snd( 0xA2, 0 );							// Mifareｶｰﾄﾞｾﾝｽ&ﾃﾞｰﾀ読出し再要求
						LagTim500ms( LAG500_MIF_LED_ONOFF, 1, op_IccLedOnOff );
						read_sht_cls();									// 強制的にｼｬｯﾀｰを閉じる
						cn_stat( 2, 1 );								// 紙幣入金不可
						break;
					default:
						break;
				}
				CCT_Cansel_Status.BYTE = 0;						// 電子媒体停止管理領域の初期化
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除

// MH321800(S) T.Nagai ICクレジット対応
				// 決済リーダ関連文言表示中はカード吐き出し
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if( ec_MessagePtnNum != 0 ){
				if( isEC_MSG_DISP() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					if( OPECTL.other_machine_card ) {
						opr_snd( 13 );								// 保留位置からでも戻す
					}
					else {
						opr_snd( 2 );								// 前排出
					}
					break;
				}
// MH321800(E) T.Nagai ICクレジット対応
				if( OPECTL.op_faz > 2 ){							// 精算中では無い?
					if( OPE_red != 2 ){
						if( OPECTL.other_machine_card )
							opr_snd( 13 );									// 保留位置からでも戻す
						else
							opr_snd( 2 );									// 前排出				
					}
				}else if( ryo_buf.zankin == 0 ){					// 残額0円
					if( OPE_red != 2 ){
						if( OPECTL.other_machine_card )
							opr_snd( 13 );									// 保留位置からでも戻す
						else
							opr_snd( 2 );									// 前排出				
					}
				}else{
					if(( OPECTL.op_faz == 0 )&&( r_zero != 0 )){	//
						Lagcan( OPETCBNO, 1 );						// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
					Lagcan( OPETCBNO, 7 );							// ﾀｲﾏｰ7ﾘｾｯﾄ(読取ｴﾗｰ表示用ﾀｲﾏｰ)
					Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
					if( !Op_StopModuleWait( REASON_CARD_READ ) ){
// MH810100(S) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
//						f_al_card_wait = 1;							// 1=磁気カードが入れられ、al_card() 実施前に電子マネーリーダなどの停止待ちをしている
// MH810100(E) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
						break;
					}
OPMOD_ARC_CR_R_EVT_DUMMY:
// MH810100(S) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
//					f_al_card_wait = 0;								// al_card() 実施のためクリア
// MH810100(E) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
					ret = al_card( msg , 0 );						// ｶｰﾄﾞ処理
					if (ret >= 0x100) {
						ret = 0;		// その他の処理保留状態
						break;
					}
				}
				break;

			case ELE_EVENT_CANSEL:									// 全デバイスの停止完了イベント
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) D.Inaba ICクレジット対応
//				if( !announce_flag ) {								// ｶｰﾄﾞ抜き取りｱﾅｳﾝｽ中であれば放送停止はしない
//// MH321800(E) D.Inaba ICクレジット対応
//					ope_anm( AVM_STOP );							// 放送停止ｱﾅｳﾝｽ
//// MH321800(S) D.Inaba ICクレジット対応
//				}
//// MH321800(E) D.Inaba ICクレジット対応
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
				ret = 10;											// 待機に戻る
				break;

			case ARC_CR_E_EVT:										// 終了ｺﾏﾝﾄﾞ受信
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				break;
			case TIMEOUT7:											// 読取ｴﾗｰ表示用ﾀｲﾏｰﾀｲﾑｱｳﾄ
			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( msg == ARC_CR_EOT_EVT ){
// MH321800(S) T.Nagai ICクレジット対応
					// 決済リーダ関連文言表示中はカード吐き出し
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//					if( ec_MessagePtnNum != 0 ){
					if( isEC_MSG_DISP() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH810100(S) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
//						f_al_card_wait = 0;							// al_card()実施前に券返却したためクリア
// MH810100(E) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						if( ec_MessagePtnNum == 98 &&				// 決済NGメッセージ表示中
//							!Suica_Rec.Data.BIT.CTRL_CARD ){		// ｶｰﾄﾞ抜取り済み
						if( isEC_NG_MSG_DISP() &&					// 決済NGメッセージ表示中
							!isEC_CARD_INSERT() ){					// ｶｰﾄﾞ抜取り済み
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							// 決済NGメッセージ表示中にクレジットカード抜いてから
							// 磁気カード抜き取りをすると磁気リーダーシャッターを
							// 閉めてしまうため、ここで抜ける
							break;
						}
						read_sht_cls();
						break;
					}
// MH321800(E) T.Nagai ICクレジット対応
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//					if( !Suica_Rec.Data.BIT.MIRYO_TIMEOUT){
//						creMessageAnaOnOff( 0, 0 );						// ｴﾗｰ表示消し
//					}
//					if ( credit_error_ari != 0 ){
//						credit_error_ari = 0;
//						if( OPECTL.op_faz != 3 ){
//							cn_stat( 1, 1 );							// 紙幣入金可
//							cn_stat( 3, 0);								// 硬貨入金可
//						}
//						else {											// 取消
//							cn_stat( 2, 2 );							// 入金不可
//						}
//					}
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
					Ope2_WarningDispEnd();							// ｴﾗｰ表示消し
				}
				if( Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_MIRYO_AFTER_STOP ){	// 未了ﾀｲﾑｱｳﾄ後のSuica停止設定の場合は表示を消さない
					if( SUICA_CM_BV_RD_STOP && !(( RD_mod >= 10 )&&( RD_mod <= 13 )) ){// 未了ﾀｲﾑｱｳﾄ後は全ての媒体を使用不可とする場合で移動中・抜き取り待ちではない場合
						read_sht_cls();
					}
				}else if( OPECTL.ChkPassSyu ){										// 定期(Mifare)の問い合わせ中のカード吐き出し
					read_sht_cls();
				}
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) D.Inaba ICクレジット対応
//				// 決済リーダ関連文言表示中ではない？
//				if( ec_MessagePtnNum == 0 ){
//// MH321800(E) D.Inaba ICクレジット対応
//				// 領収証再表示
//				if( (OPECTL.PriUsed == 0 || OPECTL.CR_ERR_DSP != 0) && OPECTL.ChkPassSyu == 0 ){
//					// ﾌﾟﾘﾍﾟ未使用 か ｴﾗｰ表示を行っていない 且つ 定期券問い合わせ中ではない
//					if( ryodsp ){
//						Lcd_Receipt_disp();
//					}else{
//						dsp_intime( 7, OPECTL.Pr_LokNo );			// 入庫時刻表示
//					}
//// MH810105(S) MH364301 インボイス対応
//					OPECTL.f_CrErrDisp = 0;							// エラー非表示
//// MH810105(E) MH364301 インボイス対応
//				}
//// MH321800(S) D.Inaba ICクレジット対応
//				}
//// MH321800(E) D.Inaba ICクレジット対応
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
				OPECTL.PassNearEnd = 0;								// 期限切れ間近
				if( SyuseiEndKeyChk() == 2 ){
					read_sht_cls();										/* 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｸﾛｰｽﾞ */
				}
				break;
			case IBK_MIF_A2_OK_EVT:									// Mifareﾃﾞｰﾀ読出し完了
				if( OPECTL.InquiryFlg )								// クレジットHOST接続中は受け付けない
					break;
				if(( OPECTL.op_faz > 2 )||( ryo_buf.zankin == 0 )){	// 精算中では無い or 残額0円
					break;
				}else{
					if(( OPECTL.op_faz == 0 )&&( r_zero != 0 )){	//
						Lagcan( OPETCBNO, 1 );						// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
					Lagcan( OPETCBNO, 7 );							// ﾀｲﾏｰ7ﾘｾｯﾄ(読取ｴﾗｰ表示用ﾀｲﾏｰ)
					Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
					if( !Op_StopModuleWait( REASON_MIF_READ ) )
						break;
OPMOD_MIF_A2_OK_DUMMY:
					// Mifare書込み状態チェック
					if( ( MifStat == MIF_WRITING) ||					// 書込み完了待ち
						( MifStat == MIF_WROTE_CMPLT) ||				// 書込み完了OK後（ｶｰﾄﾞ置きっぱなし）
						( MifStat == MIF_DATA_ERR) ){					// データエラー
						break;
					}
					else if (MifStat == MIF_WROTE_FAIL) {				// 書込みNG後、同一ｶｰﾄﾞﾀｯﾁ待ち
						if( memcmp( &MIF_LastReadCardData.Sid,  &MIF_CARD_SID[0], 4 ) != 0 ){
							break;										// 同一ｶｰﾄﾞではない場合、ｶｰﾄﾞが無くなるのを待つ
						}
						// 各情報初期化（初めて読出した場合と同等の操作にする）
						LagCan500ms( LAG500_MIF_WRITING_FAIL_BUZ );		// NGﾌﾞｻﾞｰ停止
						// ｴﾗｰ消去
						op_RestoreErrDsp(ryodsp);
					}

					memcpy( &Mifare_LastReadCardID[0], &MIF_CARD_SID[0], 4 );	// ｶｰﾄﾞID保存
					LagTim500ms( LAG500_MIF_LED_ONOFF, (1*2), op_IccLedOff );	// 1秒後消灯

					ret = al_iccard( msg , 0 );							// ICｶｰﾄﾞ処理

					if( MifStat == MIF_WROTE_FAIL )					// 書込み失敗から再度タッチされた場合
						MifStat = MIF_WAITING;					// ｶｰﾄﾞのﾁｪｯｸを行なった後でステータスを戻す

					if (OPECTL.MIF_CR_ERR) {							// Mifare券ﾃﾞｰﾀｴﾗｰ
						MifStat = MIF_DATA_ERR;							// status = data error
						OPECTL.MIF_CR_ERR = 0;							// ｴﾗｰﾌﾗｸﾞOFF

						LagTim500ms( LAG500_MIF_LED_ONOFF, (1*2), op_IccLedOff2 );	// ICC LED再点滅

					}
					else {
						MifStat = MIF_WRITING;
					}
				}
				break;
			case	TIMEOUT12:
				if (MIF_ENDSTS.sts1 == 0)
					break;			// カードのエラーを検出していなければ何もしない
		// NTNET問い合わせ中にカード無しになると、カード検出を一時停止するので
		// このタイマーでIBK_MIF_A2_EN_EVTの代用をする

			case IBK_MIF_A2_NG_EVT:									// Mifareﾃﾞｰﾀ読出しNG
				switch (MifStat) {
				case MIF_DATA_ERR:
					if( MIF_ENDSTS.sts1 == 0x30 ){					// ｶｰﾄﾞ無し
						op_RestoreErrDsp(ryodsp);					// 領収証再表示
						OPECTL.PassNearEnd = 0;						// 期限切れ間近ﾌﾗｸﾞ
						MifStat = MIF_WAITING;
					}
				case MIF_WAITING:
				case MIF_WROTE_FAIL:
					if (OPECTL.ChkPassSyu == 0) {					// 問合せ中は無視
				/* 07.01.31 NOTE
					 問合せ中にカード交換されても、元カードがOKなら精算できてしまうので
					 問合せ完了まで、カード検出を停止する
						結果OK→A4送信でエラーとなる（その後再検出開始）
						結果NG→カードエラー表示後、タイマー12でA2 NG処理を実行
				*/
						OpMif_snd( 0xA2, 0 );						// A2再送信
					}
					break;
				case MIF_WRITING:
				case MIF_WROTE_CMPLT:
					break;
				default:
					break;
				}
				break;
			case IBK_MIF_A4_OK_EVT:									// Mifareﾃﾞｰﾀ書き込み完了
				if( MifStat == MIF_WRITING ){						// Mifare書込み完了待ち
					BUZPI();										// OK音
					op_RestoreErrDsp(ryodsp);						// エラー消去
					
					// ｶｰﾄﾞ書込み失敗ｴﾘｱに登録されていれば削除
					Mifare_WrtNgDataUpdate( 2, (void *)&MIF_LastReadCardData );
					ret = al_iccard( msg , 0 );						// 料金計算処理
					op_MifareStop();

					MifStat = MIF_WROTE_CMPLT;						// Mifare書込み完了
				}
				break;
			
			case IBK_MIF_A4_NG_EVT:									// Mifareﾃﾞｰﾀ書き込みNG
				if( MifStat == MIF_WRITING ){						// Mifare書込み完了待ち
					BUZPIPI();										// NG音
					grachr( 7, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[14] );				// " 再度カードをタッチして下さい "

					LagTim500ms( LAG500_MIF_WRITING_FAIL_BUZ, (1*2), op_Cycle_BUZPIPI );	// 1秒後 再ｴﾗｰﾌﾞｻﾞｰ音
					MifStat = MIF_WROTE_FAIL;						// 書込み失敗（ｷｬﾝｾﾙ）

					/* ｶｰﾄﾞ書込み失敗ｴﾘｱに Read時のﾃﾞｰﾀを登録 */
					Mifare_WrtNgDataUpdate( 3, (void *)&MIF_LastReadCardData );	// 登録

					if( Op_StopModuleWait( REASON_MIF_WRITE_LOSS ) == 1 ){
					OpMif_snd( 0xA2, 0 );							// Mifareｶｰﾄﾞｾﾝｽ&ﾃﾞｰﾀ読出し再要求
					LagTim500ms( LAG500_MIF_LED_ONOFF, 1, op_IccLedOnOff );
																	// ICC LED再点滅
																	// 既に1秒後 ICC LED消灯指示しているので上書きする
					read_sht_cls();									// 強制的にｼｬｯﾀｰを閉じる
					cn_stat( 2, 1 );								// 紙幣入金不可
					}
				}
				break;

			case IBK_NTNET_CHKPASS:									// 定期問合せ結果ﾃﾞｰﾀ受信
			case TIMEOUT6:											// 定期券問合せ監視ﾀｲﾏｰﾀｲﾑｱｳﾄ
				if(( OPECTL.op_faz > 2 )||( ryo_buf.zankin == 0 )){	// 精算中では無い or 残額0円
					break;
				}else{
					if( OPECTL.ChkPassSyu == 1 ){					// APS定期券
						if( msg == TIMEOUT6 ){
							OPECTL.ChkPassSyu = 0xff;
						}
						msg = ARC_CR_R_EVT;
						ret = al_card( msg , 0 );					// ｶｰﾄﾞ処理

						if( OPECTL.CR_ERR_DSP ){
						read_sht_opn();								// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰｵｰﾌﾟﾝ 
						if( ryo_buf.zankin != 0 ) {						// 残額があれば
							cn_stat( 3, 2 );						// 紙幣 & COIN再入金可
							if( OPECTL.CR_ERR_DSP == 4 )
								read_sht_opn();
						}
						}
					}
					else if( OPECTL.ChkPassSyu == 2 ){				// Mifare定期券
						if( msg == TIMEOUT6 ){
							OPECTL.ChkPassSyu = 0xff;
						}
						msg = IBK_MIF_A2_OK_EVT;
						ret = al_iccard( msg , 0 );					// ICｶｰﾄﾞ処理
						if ((OPECTL.MIF_CR_ERR) && 					// Mifare券ﾃﾞｰﾀｴﾗｰ
							(OPECTL.CR_ERR_DSP != 99)) {			// 問合せ中でない
							MifStat = MIF_DATA_ERR;					// status = data error
							OPECTL.MIF_CR_ERR = 0;					// ｴﾗｰﾌﾗｸﾞOFF

							LagTim500ms( LAG500_MIF_LED_ONOFF, (1*2), op_IccLedOff2 );		// ICC LED再点滅
							read_sht_opn();
							cn_stat( 3, 2 );						// 紙幣 & COIN再入金可
						}
						else {
							MifStat = MIF_WRITING;
						}
					}
				}
				break;

			case TIMEOUT1:
				if( r_zero == 0 ){									// 0円精算
					cn_stat( 2, 2 );								// 入金不可
					ret = 1;										// 精算完了処理へ
					break;
				}
				fus_subcal_flg = 1;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( ( OPECTL.op_faz == 0 && Suica_Rec.Data.BIT.CTRL || Edy_Rec.edy_status.BIT.CTRL) ){
				if( ( OPECTL.op_faz == 0 && Suica_Rec.Data.BIT.CTRL) ){
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					time_out = 1;									// ﾀｲﾑｱｳﾄ発生ﾌﾗｸﾞON
				}
				cansel_status.BIT.INITIALIZE = 1;					// 電子媒体停止ｽﾀｰﾄ

				if(( OPECTL.op_faz == 9 || OPECTL.op_faz == 3 || ( OPECTL.op_faz == 0 && time_out == 1))){		// 取消し処理実行中の場合
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					if( Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL ){	// Suicaが有効かつ最後に送信したのが受付許可の場合
						Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
					}
					else{													// Suicaがすでに受付不可状態（通常ありえない）
						if( !Suica_Rec.Data.BIT.CTRL )
							Op_Cansel_Wait_sub( 0 );						// Suica停止済みﾌﾗｸﾞを立てる
					}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					if( Edy_Rec.edy_status.BIT.CTRL )
//						Edy_StopAndLedOff();
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

					LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 20, Op_ReturnLastTimer );	// 残額表示用ﾀｲﾏｰを一時的に使用する
				}

				if( OPECTL.op_faz >= 3 ){
					if( OPECTL.op_faz == 8 && CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){
						cn_stat( 2, 2 );									// 入金不可
						Lagcan( OPETCBNO, 1 );								// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
						Lagtim( OPETCBNO, 2, 10*50 );						// ﾒｯｸ監視ﾀｲﾏｰ起動							
						break;
					}

					if( !Op_Cansel_Wait_sub( 0xff ) )				// 全デバイスのキャンセル済み？
						Op_Cansel_Wait_sub( 2 );					// メックの停止
					break;
				}
				cn_stat( 2, 2 );									// 入金不可
				PayInfo_Class = 2;									// 精算情報データの処理区分を精算中止にする
				SetNoPayData();										// 精算中止データを設定する
				OPECTL.op_faz = 4;									// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ ﾀｲﾑｱｳﾄ
				Lagcan( OPETCBNO, 1 );								// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				Lagtim( OPETCBNO, 2, 10*50 );						// ﾒｯｸ監視ﾀｲﾏｰ起動
				break;

			case TIMEOUT2:											// ﾒｯｸ監視ﾀｲﾏｰﾀｲﾑｱｳﾄ
				switch( OPECTL.op_faz ){
					case 2:											// 完了
						if( ryo_buf.turisen ){						//
							SFV_DAT.reffal = ryo_buf.turisen;		//
							ret = 3;								// 精算完了処理へ
						}else{
							ret = 1;								// 精算完了処理へ
						}
						break;
					case 3:											// 中止
					case 4:											// ﾀｲﾑｱｳﾄ
					case 9:											// ﾀｲﾑｱｳﾄ
						if( ryo_buf.nyukin ){						//
							ryo_buf.turisen = ryo_buf.nyukin;		//
							if( SFV_DAT.reffal ){					// 不足あり?
								ryo_buf.fusoku += SFV_DAT.reffal;	// 支払い不足額
							}
							if( SFV_DAT.nt_escrow ){				// 紙幣戻しｴﾗｰ
								ryo_buf.fusoku += 1000L;			// 支払い不足額
							}
							chu_isu();								// 中止集計
							Lagtim( OPETCBNO, 1, 10*50 );			// ﾀｲﾏｰ1起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
						}else{
// MH810105(S) MH364301 QRコード決済対応（精算中止データ送信対応）
//							if( carduse() || Suica_Rec.Data.BIT.PAY_CTRL ){	// ｶｰﾄﾞ使用 かSuica使用時?
							if( carduse() || Suica_Rec.Data.BIT.PAY_CTRL ||	// ｶｰﾄﾞ使用 かSuica使用時?
								isEC_PAY_CANCEL() ){				// 決済リーダの決済精算中止？
// MH810105(E) MH364301 QRコード決済対応（精算中止データ送信対応）
								chu_isu();							// 中止処理
								svs_renzoku();						// ｻｰﾋﾞｽ券連続挿入
							}
							if( !Suica_Rec.suica_err_event.BYTE ){					// Suicaｴﾗｰが発生していない
								if( Suica_Rec.Data.BIT.CTRL ){
									Suica_Ctrl( S_CNTL_DATA, 0 );				// Suica利用を不可にする	
									OPECTL.op_faz = 9;
									LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 20, Op_ReturnLastTimer );	// 残額表示用ﾀｲﾏｰを一時的に使用する
									break;
								}
							}
							// 入金がなく、応答が無い場合は、１秒後TIMEOUT1を発生させる
							// （返金等がないので、上記のように１０秒も待つ必要がない）
							Lagtim( OPETCBNO, 1, 50 );				// ﾀｲﾏｰ1起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
						}
						break;
					case 8:											// 精算完了待ち？
						if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// 精算完了時の電子媒体停止待ち合わせ
							// ｺｲﾝﾒｯｸが応答を返さない場合、Suicaも応答がない可能性があるため、
							// 停止要求から20秒経っても終了しない場合は、Suica・ｺｲﾝﾒｯｸともに完了したとみなす
							if( !CCT_Cansel_Status.BIT.SUICA_END ){	// Suicaが終了していない場合
								CCT_Cansel_Status.BIT.SUICA_END = 1;// Suica終了
							}
							if( !Op_StopModuleWait_sub( 0xff ) ){
								Op_StopModuleWait_sub( 3 );				// ｺｲﾝﾒｯｸ終了
							}
						}
						break;
					default:
						ret = 10;									// 待機に戻る
						break;
				}
				break;

			case TIMEOUT11:											// Suica「ｶｰﾄﾞ認証ｴﾗｰ/精算未了」解消待ちのﾀｲﾑｱｳﾄ通知
				Ope_TimeOut_11( OPECTL.Ope_mod,  0 );
				break;

			case TIMEOUT10:											// 「Suica停止後の受付許可送信待ち」or「料金画面表示切替」のﾀｲﾑｱｳﾄ通知
				Ope_TimeOut_10( OPECTL.Ope_mod,  0 );
				break;
// MH321800(S) G.So ICクレジット対応
			case EC_CYCLIC_DISP_TIMEOUT:							// 「ec停止後の受付許可送信待ち」or「料金画面表示切替」のﾀｲﾑｱｳﾄ通知
				Ope_TimeOut_Cyclic_Disp( OPECTL.Ope_mod,  0 );
				break;

			case EC_RECEPT_SEND_TIMEOUT:							// ec「ｶｰﾄﾞ認証ｴﾗｰ/精算未了」解消待ちのﾀｲﾑｱｳﾄ通知ｰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,  0 );
				break;
// MH321800(E) G.So ICクレジット対応
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//			case	TIMEOUT9:
//				if( cre_ctl.Status == CRE_STS_IDLE ){				// 応答待ちﾀｲﾑｱｳﾄ
//					if( cre_ctl.Timeout_Announce != 0 ){			// 04, 06 タイムアウトのときだけアナウンスする
//						//再送信失敗.or.リトライ不可能な TimeOutだった場合
//						credit_result = -2;	//只今、お取り扱いができません
//					}
//				}
//				break;
//			case	CRE_EVT_04_OK:
//				vl_now = V_CRE;	//料金計算で参照.
//				// vl_now : を参照して処理 > Ryo_Cal
//				ryo_cal( 0, OPECTL.Pr_LokNo );
//
//				if( prm_get( COM_PRM, S_CRE, 3, 1, 1 ) == 1 ){		//利用明細書/領収書 自動発行する
//					if( OPECTL.RECI_SW == 0 ){						// 領収書ﾎﾞﾀﾝ未使用?
//						OPECTL.RECI_SW = 1;							// 領収書ﾎﾞﾀﾝ使用
//					}
//				}
//
//				ac_flg.cycl_fg = 10;								// 入金
//				in_mony( msg, 0 );
//
//				credit_result = 1;
//
//				// 本当は in_mony の先の invcrd の戻り値を使いたいが、他のイベントと共通化しているため
//				// 影響が出ない様に↓でもう一度チェックする. クレジットで、 B3正常の場合は必ず、精算は完了する.
//				if ( ryo_buf.turisen == 0 ){
//					credit_turi = 1;
//				}else{
//					credit_turi = 2;
//				}
//				if (ryo_buf.nyukin != 0){
//					credit_nyukin = 1;								// クレジット精算前の入金あり
//				}
//
//				OPECTL.InquiryFlg = 0;
//
//				cre_ctl.SalesKind = CRE_KIND_PAYMENT;				// 送信要因に精算動作を設定
//
//				if( creCtrl( CRE_EVT_SEND_SALES ) != 0 ){			// 売上依頼データ(05)の初回送信
//					;	// 該当処理なし
//				}else{
//					;	// 該当処理なし
//				}
//				break;
//			case	CRE_EVT_02_OK:
//			case	CRE_EVT_06_OK:
//			case	CRE_EVT_08_OK:
//			case	CRE_EVT_0A_OK:
//				{
//					;	// 該当処理なし
//				}
//				break;
//			case	CRE_EVT_02_NG:
//			case	CRE_EVT_08_NG:
//			case	CRE_EVT_06_TOUT:
//				//ＮＧ応答受信
//				if(OPECTL.InquiryFlg == 1) {						// 与信問い合せ時
//					credit_result = -2;										//  只今、お取り扱いができません 
//					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ
//
//					OPECTL.InquiryFlg = 0;
//					creSeisanInfo.amount = 0;
//					ryo_buf.credit.pay_ryo = 0;
//				}
//				break;
//			case	CRE_EVT_0A_NG:
//				{
//					;	// 該当処理なし
//				}
//				break;
//			case	CRE_EVT_04_NG1:
//			case	CRE_EVT_04_NG2:
//				//ＮＧ応答受信
//				if( msg == CRE_EVT_04_NG1 ){
//					credit_result = -3;									// このカードは使えません
//				}else if( msg == CRE_EVT_04_NG2 ){
//					credit_result = -2;									// 只今、お取り扱いができません
//				}
//				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ
//
//				OPECTL.InquiryFlg = 0;
//				creSeisanInfo.amount = 0;
//				ryo_buf.credit.pay_ryo = 0;
//				break;
//			case	CRE_EVT_06_NG:
//				//ＮＧ応答受信
//				;	// ここでは該当処理なし
//				break;
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// MH321800(S) G.So ICクレジット対応
			case EC_EVT_CANCEL_PAY_OFF:							// 精算キャンセルトリガ発生
// MH810103 GG119202(S) カード処理中タイムアウトでみなし決済扱いとする
//			case EC_INQUIRY_WAIT_TIMEOUT:						// 問合せ(処理中)待ちタイムアウト
// MH810103 GG119202(E) カード処理中タイムアウトでみなし決済扱いとする
			// 問合せタイムアウトで精算をキャンセルする。（みなし決済とはしない）
			// （精算キャンセルはタイムアウトと同じ処理をする）
				OPECTL.InquiryFlg = 0;							// カード処理中フラグOFF
				Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 0;		// 決済処理中受信フラグ初期化
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				Ec_Settlement_Sts = EC_SETT_STS_NONE;			// 決済状態をOFFにする
				ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;		// 決済状態をOFFにする
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
				ec_MessageAnaOnOff( 0, 0 );						// 処理中の文言を元に戻す
				ope_anm(AVM_STOP);								// しばらくお待ちくださいの音声停止
// MH810103 GG119202(S) 電子マネー決済時の決済結果受信待ちタイマ追加
				ac_flg.ec_deemed_fg = 0;						// みなし決済復電用フラグOFF
// MH810103 GG119202(E) 電子マネー決済時の決済結果受信待ちタイマ追加
				// no break
			case	EC_EVT_CRE_PAID_NG:
			// 外部照会NG時、入金可にする。カード抜き取り後とする。
// MH810100(S) S.Nishimoto 2020/08/19 #4606 車番チケットレス(無用なポップアップが表示されてしまう)
//// MH810100(S) K.Onodera 2020/3/10 車番チケットレス(ポップアップ)
//				lcdbm_notice_dsp( POP_INQUIRY_NG, 0 );	// 照会NGポップアップ
//// MH810100(E) K.Onodera 2020/3/10 車番チケットレス(ポップアップ)
// MH810100(E) S.Nishimoto 2020/08/19 #4606 車番チケットレス(無用なポップアップが表示されてしまう)
				if( ryo_buf.nyukin && OPECTL.credit_in_mony ){	// 入金ありでｸﾚｼﾞｯﾄとのすれ違い入金の場合
					// in_mony()内でcn_crdt()を実行してryo_buf.nyukinを更新してつり銭計算を行う
					// 一度、cn_crdt()を実行すれば、紙幣入金分も加算されるので
					// in_mony(COIN_EVT)だけを実行する
					in_mony ( COIN_EVT, 0 );					// ｺｲﾝｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
					OPECTL.credit_in_mony = 0;					// すれ違いﾌﾗｸﾞｸﾘｱ
// MH810100(S) K.Onodera 2020/2/27 車番チケットレス(残高変化通知)
					if( ryo_buf.zankin ){
						lcdbm_pay_rem_chg(0);					// 残高変化通知(精算中)
					}
// MH810100(E) K.Onodera 2020/2/27 車番チケットレス(残高変化通知)
				}

				// 残りが0円でない時
				if( ryo_buf.zankin ){
					OPECTL.holdPayoutForEcStop = 0;						// 保留中を解除
// MH810100(S) K.Onodera 2020/03/11 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
					if( lcdbm_Flag_ResetRequest ){	// 精算中止要因発生？
						// 精算中止処理後、起動待機状態に遷移する(OPECTL.init_stsは要因に応じた値に変更済み)
						// 但し、意図的に待機状態に遷移させる場合は起動待機状態には遷移しない
						if( OPECTL.Ope_mod != 0 ){
							OPECTL.Ope_mod = 255;
						}
						msg = KEY_TEN_F4;
						goto _MSG_RETRY;
					}
// MH810100(E) K.Onodera 2020/03/11 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
					// EC_EVT_CRE_PAID_NG、再入金可能にする
					if( msg == EC_EVT_CRE_PAID_NG ){
						// ｶｰﾄﾞ差込なし？ または、決済リーダ関連文言表示中ではない？
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						if( !Suica_Rec.Data.BIT.CTRL_CARD || ec_MessagePtnNum == 0 ){
						if( !isEC_CARD_INSERT() || !isEC_MSG_DISP() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							read_sht_opn();
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
							if (check_enable_multisettle() <= 1 ||
// MH810105(S) MH364301 QRコード決済対応
//								!isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
//								// マルチブランド以外、または、電子マネーブランド未選択の場合は
//								// 再入金可とする
								!EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
								// シングルブランド、または、電子マネー、QRコード決済以外の場合は
								// 再入金可とする
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
							cn_stat( 3, 2 );								// 紙幣 & COIN再入金可
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
							}
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
							if( MIFARE_CARD_DoesUse ){						// Mifareが有効な場合
								op_MifareStart();							// Mifare有効
							}
						}
					}
					// EC_INQUIRY_WAIT_TIMEOUT、EC_EVT_CANCEL_PAY_OFF、自動取消発生
					else {
						// 自動取消(取消ボタン)トリガ発生
// MH810100(S) S.Nishimoto 2020/07/27 車番チケットレス(電子決済端末対応)
//						msg = KEY_TEN_F4;
//						goto _MSG_RETRY;
// MH810100(S) S.Nishimoto 2020/07/29 車番チケットレス(電子決済端末対応)
						// 操作通知(カード決済状態通知)
// MH810103(s) 電子マネー対応 未了通知
//						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x07);	// クレジットカードキャンセル
						if(isEcBrandNoEMoney(RecvBrandResTbl.no, 0) ){
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x08);	// 電子マネーキャンセル
						}else{
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x07);	// クレジットカードキャンセル
						}
// MH810103(e) 電子マネー対応 未了通知
// MH810100(E) S.Nishimoto 2020/07/29 車番チケットレス(電子決済端末対応)
						// リジェクトレバー通知
						PKTcmd_notice_ope(LCDBM_OPCD_REJECT_LEVER, (ushort)0);
// MH810100(E) S.Nishimoto 2020/07/27 車番チケットレス(電子決済端末対応)
					}
				}
				else {
			//※zankinなし（精算完了）時のすれ違いに対してはcredit_in_monyで処理されるため、払出し可は無視して良い
					OPECTL.holdPayoutForEcStop = 0;						// 保留中を解除
				}
				break;
// MH321800(E) G.So ICクレジット対応
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//			case	IFMPAY_GENGAKU:
//			// 残額が一致すること && 減額≦残額であること
//				if (vl_ggs.zangaku == ryo_buf.zankin &&
//					!OPECTL.InquiryFlg &&	// ｸﾚｼﾞｯﾄHOST接続中及び電子決済の未了中に1になる
//					vl_ggs.waribikigaku <= ryo_buf.zankin) {
//					ope_imf_Answer(1);					// 正常通知
//					wopelg(OPLOG_PARKI_GENGAKU, 0, vl_ggs.waribikigaku);	// 履歴登録
//					vl_now = V_GNG;
//					ryo_cal(3, OPECTL.Pr_LokNo);		// サービス券として計算
//					ret = in_mony(msg, 0);
//				}
//				else {
//					ope_imf_Answer(0);					// 拒否通知
//				}
//				break;
//
//			case	IFMPAY_FURIKAE:
//				// 振替元車室ラグタイマ停止
//				if (ope_Furikae_start(vl_frs.lockno) &&
//					!OPECTL.InquiryFlg ){	// ｸﾚｼﾞｯﾄHOST接続中及び電子決済の未了中に1になる
//					ope_ifm_GetFurikaeGaku(&vl_frs);
//					ope_imf_Answer(1);				// 正常通知
//				} else {
//					ope_imf_Answer(0);				// 異常通知
//					break;
//				}
//				wopelg(OPLOG_PARKI_FURIKAE, 0, 0);	// 履歴登録
//				FurikaeMotoSts = 1;			// 振替元状態フラグ 1:車あり
//				OpeNtnetAddedInfo.PayMethod = 11;	// 精算方法=11:修正精算
//				// 定期有/無判定
//				if (vl_frs.antipassoff_req) {
//					// 定期利用有りなら定期投入後
//					ryo_buf.apass_off = 1;
//				} else {
//					// 定期無しなら振替実行
//					ope_ifm_FurikaeCalc( 0 );				// 定期精算で振替完了なら振替額＝０円
//					ret = in_mony(IFMPAY_FURIKAE, 0);
//				}
//				break;
//
//			case	SYU_SEI_SEISAN:
//				OpeNtnetAddedInfo.PayMethod = 11;	// 精算方法=11:修正精算
//				// 定期有/無判定
//				if (vl_frs.antipassoff_req) {		// 修正元で定期券使用
//					// 定期利用有りなら定期投入後
//					ryo_buf.apass_off = 1;			// 強制的にアンチパスをOFFにする
//					
//					set03_98 = (uchar)prm_get(COM_PRM, S_TYP, 98, 1, 4);
//					if( (set03_98 == 2)||(set03_98 == 3)){		// 修正元で使った定期券情報により精算
//						if(	!al_pasck_syusei() ){				// 定期券使用可能?
//							ryo_cal( 1, OPECTL.Pr_LokNo );
//						}
//					}
//				}
//				ope_ifm_FurikaeCalc( 1 );			// 定期精算で振替完了なら振替額＝０円
//				ret = in_mony(IFMPAY_FURIKAE, 0);
//				break;
//
//			case CAR_FURIKAE_OUT:									// 振替元の車が出庫
//				if (FurikaeMotoSts != 0) {
//					// 振替元状態フラグ 0:チェック不要 でなければ"2:車なし"に
//					FurikaeMotoSts = 2;			// 振替元状態フラグ 2:車なし
//				}
//				break;
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

			case OPE_REQ_CALC_FEE:						// 料金計算要求
				// 料金計算
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);	// 全車室情報テーブル送信
				break;

// MH321800(S) Y.Tanizaki ICクレジット対応
// MH810103 GG119202(S) カード処理中タイムアウトでみなし決済扱いとする
			case EC_INQUIRY_WAIT_TIMEOUT:							// 問合せ(処理中)待ちタイムアウト
				err_chk( (char)jvma_setup.mdl, ERR_EC_RCV_SETT_FAIL, 1, 0, 0 );
				// 決済結果受信失敗
				Suica_Rec.suica_err_event.BIT.SETT_RCV_FAIL = 1;
// MH810103 GG119202(S) JVMAリセット処理不具合
				jvma_trb(ERR_SUICA_RECEIVE);
// MH810103 GG119202(E) JVMAリセット処理不具合
// MH810103 GG119202(S) ハウスカードによるカード処理中タイムアウトは精算中止とする
// MH810103 GG119201(S) ハウスカードブランド判定処理不具合
//				if (!IsSettlementBrand(RecvBrandResTbl.no)) {
// MH810105(S) MH364301 QRコード決済対応
//				if (!isEcBrandNoEMoney(RecvBrandResTbl.no, 0) &&
				if (!EcUseKindCheck(convert_brandno(RecvBrandResTbl.no)) &&
// MH810105(E) MH364301 QRコード決済対応
					RecvBrandResTbl.no != BRANDNO_CREDIT) {
// MH810103 GG119201(E) ハウスカードブランド判定処理不具合
					// 決済なしブランド／後決済ブランドは精算中止とする
					queset(OPETCBNO, EC_EVT_CANCEL_PAY_OFF, 0, 0);
					break;
				}
// MH810103 GG119202(E) ハウスカードによるカード処理中タイムアウトは精算中止とする
				// no break
// MH810103 GG119202(E) カード処理中タイムアウトでみなし決済扱いとする
			case EC_EVT_DEEMED_SETTLEMENT:
				// みなし決済トリガ発生
				EcDeemedSettlement(&OPECTL.Ope_mod);
				break;
			case EC_EVT_CARD_STS_UPDATE:
				// カード処理中なら破棄
				if( OPECTL.InquiryFlg != 0 ){
					break;
				}
				LagCan500ms(LAG500_EC_START_REMOVAL_TIMER);			// ｶｰﾄﾞ抜き取りタイマ起動タイマー停止
				// 操作通知(カード抜去通知)
				lcdbm_notice_ope(LCDBM_OPCD_CARD_EJECTED, 0x00);
				// 取消押下時のｶｰﾄﾞ抜き取りｱﾅｳﾝｽ中？
				if ( announce_flag ) {
				// announce_endはセットせず、Suica_Rec.Data.BIT.CTRL_CARD次第でリトライする
					ret = 10;										// 待機画面へ
				}
				else {
// MH321800(S) カード差込状態更新通知ととりけしの競合により預かり証が発行される不具合修正
					// 決済結果NG後のｶｰﾄﾞ抜き取り待ち中？or決済結果NG表示中？
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//					if( ec_MessagePtnNum == 4 || ec_MessagePtnNum == 98 ){
					if( isEC_REMOVE_MSG_DISP() || isEC_NG_MSG_DISP() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
						queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);	// opeへ通知(再度決済NG受信の処理を行う。入金を再開させる)
					}
// MH321800(E) カード差込状態更新通知ととりけしの競合により預かり証が発行される不具合修正
					// 決済結果NG後のｶｰﾄﾞ抜き取り待ち中？
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//					if( ec_MessagePtnNum == 4 ){
					if( isEC_REMOVE_MSG_DISP() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
						ope_anm( AVM_STOP );						// 放送停止ｱﾅｳﾝｽ
						ec_MessageAnaOnOff( 0, 0 );					// 「カードをお取り下さい」表示をもとに戻す
					}
// MH321800(S) カード差込状態更新通知ととりけしの競合により預かり証が発行される不具合修正
//					queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);		// opeへ通知(再度決済NG受信の処理を行う。入金を再開させる)
// MH321800(E) カード差込状態更新通知ととりけしの競合により預かり証が発行される不具合修正
				}
				break;
			case EC_REMOVAL_WAIT_TIMEOUT:
				// 取消押下時のｶｰﾄﾞ抜き取りｱﾅｳﾝｽ中？
				if ( announce_flag ) {
				// 以後Suica_Rec.Data.BIT.CTRL_CARDは見ずに終了
					announce_end = 1;								// ｱﾅｳﾝｽｴﾝﾄﾞフラグON
					ret = 10;										// 待機画面へ
				}
				// 決済結果NG後のｶｰﾄﾞ抜き取り待ち中？
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				else if( ec_MessagePtnNum == 4 ){
				else if( isEC_REMOVE_MSG_DISP() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);		// opeへ通知(再度決済NG受信の処理を行う。入金を再開させる)
				}
				ope_anm( AVM_STOP );								// 放送停止ｱﾅｳﾝｽ
				ec_MessageAnaOnOff( 0, 0 );							// 「カードをお取り下さい」表示をもとに戻す
				break;
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
		// 決済リーダーでのブランド選択に対応
//			case EC_BRAND_RESULT_TCARD:		// 決済リーダ対応 ブランド選択結果=Tカード
//				AcceptTCard();
//				EnableNyukin();
//				break;				
//			case EC_BRAND_RESULT_HOUJIN:	// 決済リーダ対応 ブランド選択結果=法人カード
//				if(InquireHoujin()) {		// 法人カード問い合わせ
//					// 法人カードNG
//					EnableNyukin();		
//				}
//				break;				
			case EC_BRAND_RESULT_UNSELECTED:
				BrandResultUnselected();
			// no break
			case EC_BRAND_RESULT_CANCELED:
				Ope_EcPayStart();			// ブランド選択データ
// MH810103(s) 電子マネー対応 電子マネーブランド選択時の案内放送(キャンセル)
				lcdbm_notice_ope(LCDBM_OPCD_BRAND_SELECT_CANCEL,0);
// MH810103(e) 電子マネー対応 電子マネーブランド選択時の案内放送(キャンセル)
				break;
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
			case EC_EVT_ENABLE_NYUKIN:
				if (isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
					// 電子マネーブランド選択状態の場合は再入金可処理しない
					break;
				}
// MH810105(S) MH364301 QRコード決済対応
				if( RecvBrandResTbl.no == BRANDNO_QR ){
					// QRコード選択状態の場合は再入金可処理しない
					break;
				}
// MH810105(E) MH364301 QRコード決済対応
				if (Suica_Rec.suica_err_event.BIT.SETT_RCV_FAIL != 0) {
					// 決済結果受信失敗時はみなし決済となるため、再入金可としない
					break;
				}

				cn_stat( 3, 2 );			// 紙幣 & COIN再入金可
				if (LCDNO == 3) {
					// 「硬貨または紙幣で精算して下さい」再表示のため再描画
					OpeLcd(3);
				}
				break;
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
// MH810105(S) MH364301 インボイス対応
			case OPE_OPNCLS_EVT:							// 強制営休業 event
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//				if (!IS_INVOICE) {
//					break;
//				}
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）

				statusChange_DispUpdate();					// 画面更新
				break;
// MH810105(E) MH364301 インボイス対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応_変更))
			/****************************************************************/
			/* 入庫情報(ｶｰﾄﾞ情報) = op_mod02								*/
			/*		  1 = 精算完了(釣り無し) 	精算完了処理(op_mod03)へ	*/
			/*		  2 = 精算完了(釣り有り) 	精算完了処理(op_mod03)へ	*/
			/*		  3 = ﾒｯｸｴﾗｰ発生         	精算完了処理(op_mod03)へ	*/
			/*		 10 = 精算中止           	待機(op_mod00)へ戻る		*/
			/****************************************************************/
			case LCD_IN_CAR_INFO:

// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
				Lagcan(OPETCBNO, 28);				//100円投入タイマ
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算

				// 入庫情報(ｶｰﾄﾞ情報)のﾁｪｯｸと設定処理
				lcdbm_rsp_in_car_info_proc();

				// カード情報待ち状態
				switch( lcdbm_QRans_InCar_status ){
					// QRデータ未受信
					case 0:
						break;

					// QRデータ応答(OK)送信後のカード情報待ち
					case 1:
						type = lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup;
						// 割引重複利用チェックＯＫ？
						if( type < 2 ){
							lcdbm_QRans_InCar_status = 0;
						}else{
							// 割引重複あり
							if( type == 2 ){
								lcdbm_notice_dsp( POP_UNABLE_QR, 0 );	// 重複
							}
							// データ不正
							else if( type == 3 ){
								lcdbm_notice_dsp( POP_UNABLE_QR, 1 );	// データ不正
							}
							// その他異常
							else{
								lcdbm_notice_dsp( POP_UNABLE_QR, 2 );	// その他異常
							}

// GG124100(S) R.Endo 2022/08/25 車番チケットレス3.0 #6343 クラウド料金計算対応
// // MH810100(S) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
// 							// 入庫情報NG応答
// 							lcdbm_notice_ope( LCDBM_OPCD_IN_INFO_NG, (ushort)0 );
// // MH810100(E) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
// GG124100(E) R.Endo 2022/08/25 車番チケットレス3.0 #6343 クラウド料金計算対応
						}
						break;

					// カード情報待ち中に現金投入あり
					case 2:
						// 割引精算の再計算を行う
						lcdbm_QRans_InCar_status = 3;	// 割引精算後 0にする
						break;

					default:
						break;
				}
// MH810100(S) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）
				// センター問い合わせ結果をDC用の情報にセット
				SetQRMediabyQueryResult();
// MH810100(E) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
				if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
					// 入庫情報チェック(割引)
					retOnline = cal_cloud_discount_check();

					// 結果判定
					switch ( retOnline ) {
					case 0:		// 処理成功
						// 車番チケットレス精算(入庫時刻指定)設定
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// 						in_time_set();
						in_time_set(0);	// 二回目以降
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる

// GG129000(S) H.Fujinaga 2023/02/24 ゲート式車番チケットレスシステム対応（遠隔精算対応）
						if (PayInfoChange_StateCheck() == 1){
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
							// 入金されている場合を考慮して再計算
							cn_crdt();
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
							// 遠隔精算開始時であれば抜ける
							break;
						}
// GG129000(E) H.Fujinaga 2023/02/24 ゲート式車番チケットレスシステム対応（遠隔精算対応）
						// 使用済媒体登録
						ope_AddUseMedia(&MediaDetail);

						// QR割引OK応答
						lcdbm_notice_ope(LCDBM_OPCD_QR_DISCOUNT_OK, 0);

						break;
					case 2:		// 入庫情報NG(QR取消なし)
						// 入庫情報NG応答
						lcdbm_notice_ope(LCDBM_OPCD_IN_INFO_NG, 0);

						break;
					case 1:		// 入庫情報NG
					default:	// その他
						// QR取消データ即時登録
						ope_CancelRegist(&MediaDetail);

						// 入庫情報NG応答
						lcdbm_notice_ope(LCDBM_OPCD_IN_INFO_NG, 0);

						break;
					}
				} else {					// 通常料金計算モード
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

				// 割引処理実行
// MH810100(S) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
//				if( !OnlineDiscount() ){
				retOnline = OnlineDiscount();
				if(!retOnline){
// MH810100(E) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
					// 使用済媒体登録
					ope_AddUseMedia( &MediaDetail );
					// QR割引OK応答
					lcdbm_notice_ope( LCDBM_OPCD_QR_DISCOUNT_OK, (ushort)0 );
				}
// MH810100(S) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)
				// 割引処理失敗？ ※前回からの差分が失敗か成功か
				else{
					// 入庫情報NG応答
// MH810100(S) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
//					lcdbm_notice_ope( LCDBM_OPCD_IN_INFO_NG, (ushort)0 );
					lcdbm_notice_ope( LCDBM_OPCD_IN_INFO_NG, (ushort)retOnline );
					if( retOnline < 100){
						// センターがわでNGになったもの以外（RXMでNGとしたもの)はキャンセルデータを送る
						ope_CancelRegist(&MediaDetail );
					}
// MH810100(E) 2020/07/11 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
				}
// MH810100(E) K.Onodera  2020/04/01 #4040 車番チケットレス(入庫情報NGでポップアップ表示しない場合がある)

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応
				}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応

// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
				// 入金で精算完了条件を満たしたが割引待ちで精算未完了と判定した場合のために
				// 割引NGであれば入金処理サブを呼び再判定を行う
				if ( retOnline ) {
					// 入金処理サブ：車番チケットレス精算(入庫時刻指定)
					in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);
				}
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041

// MH810100(S) K.Onodera  2020/04/10 #4127 車番チケットレス(再度QRコードを翳した場合に表示されない)
//				// QR利用時の割引データ比較用にバックアップ
//				memcpy( DiscountBackUpArea, lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo, sizeof(DiscountBackUpArea) );
// MH810100(E) K.Onodera  2020/04/10 #4127 車番チケットレス(再度QRコードを翳した場合に表示されない)

				key_num = OPECTL.Op_LokNo;			// ｷｰ入力値 <- 駐車位置番号
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					OPECTL.f_rtm_remote_pay_flg = 0;
					if (ryo_buf.nyukin) {
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
						OPECTL.CN_QSIG = 1;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
						in_mony(COIN_EVT, 0);			// 入金再計算
					}
					if (OPECTL.op_faz < 2) {
						cn_stat( 3, 2 );				// 紙幣 & COIN再入金可
					}
				}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）

				// 割引金額 + 投入金額 >= 駐車料金
				if( (ryo_buf.waribik + ryo_buf.nyukin) >= ryo_buf.tyu_ryo ){
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
//// MH810100(S) S.Takahashi 2020/03/02 割引額が遅れて表示される
//					lcdbm_pay_rem_chg(1);							// 残高変化通知(精算完了)
//// MH810100(E) S.Takahashi 2020/03/02 割引額が遅れて表示される
//					lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT,2 );	// 精算完了通知
					if ( cmp_send != 2 ) {		// 同種の残高変化通知(精算完了)未送信
						lcdbm_pay_rem_chg(1);	// 残高変化通知(精算完了)
						cmp_send = 2;
					}
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041

// MH810100(S) K.Onodera 2020/03/23 #4043 車番チケットレス(割引と入金同時で割引勝ち精算完了時、入金分が払い出されない不具合修正)
//					// 釣り銭なし
//					if (ryo_buf.turisen <= 0) {
//						ret = 1;				// 精算完了(釣り無し) 	精算完了処理(op_mod03)へ
//					}
//					// 釣り銭あり(入金された後の割引金額 >= 駐車料金 = 入金は全額払戻す)
//					else {
// MH810100(E) K.Onodera 2020/03/23 #4043 車番チケットレス(割引と入金同時で割引勝ち精算完了時、入金分が払い出されない不具合修正)
// MH810103(s) 電子マネー対応 QRサービス券で精算完了した場合に、領収証の発行が遅くなる不具合修正
						// OnlineDiscount内のin_monyで同じことを実施しているし、決済リーダがある場合はop_fazを8にして停止待ちするためここで書き換えてはダメ
//						ac_flg.cycl_fg = 10;				// 入金中
//						cn_crdt();
//						ac_flg.cycl_fg = 11;				// 精算完了
//						if( OPECTL.op_faz != 2 ){
//							svs_renzoku();					// ｻｰﾋﾞｽ券連続挿入
//							cn_stat( 2, 2 );				// 入金不可
//							Lagtim( OPETCBNO, 2, 10*50 );	// ﾒｯｸ監視10s = TIMEOUT2:
//							OPECTL.op_faz = 2;
//						}
// MH810103(e) 電子マネー対応 QRサービス券で精算完了した場合に、領収証の発行が遅くなる不具合修正
// MH810100(S) K.Onodera 2020/03/23 #4043 車番チケットレス(割引と入金同時で割引勝ち精算完了時、入金分が払い出されない不具合修正)
//					}
// MH810100(E) K.Onodera 2020/03/23 #4043 車番チケットレス(割引と入金同時で割引勝ち精算完了時、入金分が払い出されない不具合修正)
				}
				// 割引金額 + 投入金額 < 駐車料金 = 割引後料金が発生
				else{
					lcdbm_pay_rem_chg(0);							// 残高変化通知(精算中)

					// QRフラグﾞ== 利用中	(0 = idle, 1 = 利用確認中, 2 = 利用中)
					if (lcdbm_Flag_QRuse_ICCuse == 2) {
						// ICｸﾚｼﾞｯﾄ開始要求
// MH810103(s) 電子マネー対応 #5403 クレジット精算限度額からQR割引で限度額以下になった場合、クレジット精算が行えるようにならない
//						Suica_Ctrl( S_CNTL_DATA, 0x01 );
						// 決済リーダ金額変更
						if( isEC_USE()){						// 決済リーダで
							if(!Suica_Rec.Data.BIT.PAY_CTRL	&&	// 決済結果データを受信しておらず		// ★注意レビュー中
							Suica_Rec.Data.BIT.CTRL   			// 決済リーダ有効で
							) {
								if (EcCheckBrandSelectTable(TRUE)){			// ブランド選択データに変更がある(ブランド選択中でない場合は常にTRUE)
									Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// 受付禁止送信(金額変更)
								}
							}

							// 受付許可送信可否判定はここで行う
							if(( OPECTL.op_faz < 2 ) &&						// 精算未完了
							( !Suica_Rec.Data.BIT.PAY_CTRL ) &&			// 決済結果データを受信しておらず
							( !Suica_Rec.Data.BIT.CTRL )){				// 決済リーダ無効
								EcSendCtrlEnableData();						// 受付許可送信
							}
						}
// MH810103(s) 電子マネー対応 #5403 クレジット精算限度額からQR割引で限度額以下になった場合、クレジット精算が行えるようにならない
						// QRフラグクリア
						lcdbm_Flag_QRuse_ICCuse = 0;
					}
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
					if (lcdbm_Flag_RemotePay_ICCuse == 2) {
						// 決済リーダ再活性化
						Op_StopModuleWait_sub(2);
					}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				}
				lcdbm_QRans_InCar_status = 0;
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				lcdbm_Flag_RemotePay_ICCuse = 0;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
				Lagtim( OPETCBNO, 28, AP_CHARGE_INTERVAL_SEC * 50);	// 100円投入タイマ
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算
				break;

			// QR確定・取消データ応答(lcdbm_rsp_QR_conf_can_res_t)
			case LCD_QR_CONF_CAN_RES:
				break;

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			// レーンモニタデータ応答(lcdbm_rsp_LANE_res_t)
			case LCD_LANE_DATA_RES:
				break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

			// QRデータ
			case LCD_QR_DATA:
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算（リアルタイム）開始受付済み
					lcdbm_QR_data_res( 1 );	// 結果(1:NG(排他))
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）

				if( ope_ParseBarcodeData( &MediaDetail ) ){
					result = ope_CanUseBarcode( &MediaDetail );
				}
				else{
					result = RESULT_BAR_FORMAT_ERR;
				}
				// 最終読取QRの保持(メンテナンス用)
				push_ticket( &MediaDetail,(ushort)result);

				// エラーあり？
				if( result != RESULT_NO_ERROR ){
					switch( result ){
						case RESULT_QR_INQUIRYING:			// 問合せ中
							type = 1;	// 排他
							break;
						case RESULT_DISCOUNT_TIME_MAX:		// 割引上限
// MH810100(S) K.Onodera 2020/03/04 #3994 同じQRコードを翳すと「QRの利用枚数を超えています。」と案内する
							type = 6;	// 割引上限
							break;
// MH810100(E) K.Onodera 2020/03/04 #3994 同じQRコードを翳すと「QRの利用枚数を超えています。」と案内する
						case RESULT_BAR_USED:				// 使用済のバーコード
// MH810100(S) K.Onodera 2020/03/04 #3994 同じQRコードを翳すと「QRの利用枚数を超えています。」と案内する
							type = 7;	// 使用済のバーコード
							break;
// MH810100(E) K.Onodera 2020/03/04 #3994 同じQRコードを翳すと「QRの利用枚数を超えています。」と案内する
						case RESULT_BAR_READ_MAX:			// バーコード枚数上限
							type = 2;	// 枚数上限
							break;
						case RESULT_BAR_EXPIRED:			// 期限切れバーコード
							type = 3;	// 有効期限切れ
							break;
						case RESULT_BAR_FORMAT_ERR:			// フォーマットエラー
							type = 4;	// フォーマット不正
							break;
						case RESULT_BAR_NOT_USE_CHANGEKIND:	// 入金・割引済み後の車種切換
							type = 8;	// 入金・割引済み後の車種切換
							break;
						case RESULT_BAR_ID_ERR:				// IDエラー
						default:
							type = 5;	// 対象外
							break;
					}
					// QR応答データ
					lcdbm_QR_data_res( type );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
					break;
				}
// MH810102(S) 電子マネー対応 #5476 【市場不具合】精算ボタンを押しても「精算を開始できませんでした」となり開始できない
				if(	flag_stop){		// 1:中止中
					// QR応答データ
					lcdbm_QR_data_res( 1 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
					break;
				}
// MH810102(E) 電子マネー対応 #5476 【市場不具合】精算ボタンを押しても「精算を開始できませんでした」となり開始できない

				// ICｸﾚｼﾞｯﾄ設定あり
// MH810103(s) 電子マネー対応
//				if( prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1  ){
				if(isEC_USE()) {
// MH810103(e) 電子マネー対応
// MH810100(S) S.Fujii 2020/08/27 #4764 クレジット利用不可の状態でQRコードが利用できない
//					// ICｸﾚｼﾞｯﾄ停止要求
//					Op_StopModuleWait( REASON_QR_USE );
//					// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ開始		48-0021	ﾃﾞｰﾀ受信監視ﾀｲﾏ	(問合せ)	⑤⑥:受信監視ﾀｲﾏ(問合せ) 1～99=1秒～99秒
//					Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, (res_wait_time * 50) );
//					// QRﾌﾗｸﾞ←利用確認中	0 = idle, 1 = 利用確認中, 2 = 利用中
//					lcdbm_Flag_QRuse_ICCuse = 1;
// MH810100(S) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
//					if( Suica_Rec.Data.BIT.CTRL &&				// 受付可で
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//					if( Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ) {	// カード処理中？
					if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
						// QR応答データ
						lcdbm_QR_data_res( 1 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
					} else if( Suica_Rec.Data.BIT.CTRL &&				// 受付可で
// MH810100(E) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
						!Suica_Rec.suica_err_event.BYTE) {		// エラーなし
						// ICｸﾚｼﾞｯﾄ停止要求
// MH810103(s) 電子マネー対応 QR利用
//						Op_StopModuleWait( REASON_QR_USE );
//// MH810100(S) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
////						// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ開始		48-0021	ﾃﾞｰﾀ受信監視ﾀｲﾏ	(問合せ)	⑤⑥:受信監視ﾀｲﾏ(問合せ) 1～99=1秒～99秒
////						Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, (res_wait_time * 50) );
//						// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ開始		受付不可受信待ちタイマー(5秒) + 1秒
//						Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, ((EC_DISABLE_WAIT_TIME+1) * 50) );
//// MH810100(E) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
//						// QRﾌﾗｸﾞ←利用確認中	0 = idle, 1 = 利用確認中, 2 = 利用中
//						lcdbm_Flag_QRuse_ICCuse = 1;
						if(Op_StopModuleWait( REASON_QR_USE )){
							// QR応答データ
							lcdbm_QR_data_res( 0 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
							// QRﾃﾞｰﾀ受信 & QRﾃﾞｰﾀ応答(OK)送信後のLCD_IN_CAR_INFO待ち
							lcdbm_QRans_InCar_status = 1;
							// QRﾌﾗｸﾞ←利用中
							lcdbm_Flag_QRuse_ICCuse = 2;
						}else{
							// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ開始		受付不可受信待ちタイマー(5秒) + 1秒
							Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, ((EC_DISABLE_WAIT_TIME+1) * 50) );
							// QRﾌﾗｸﾞ←利用確認中	0 = idle, 1 = 利用確認中, 2 = 利用中
							lcdbm_Flag_QRuse_ICCuse = 1;
						}
// MH810103(e) 電子マネー対応 QR利用
					} else {
						// QR応答データ
						lcdbm_QR_data_res( 0 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))

						// QRﾃﾞｰﾀ受信 & QRﾃﾞｰﾀ応答(OK)送信後のLCD_IN_CAR_INFO待ち
						lcdbm_QRans_InCar_status = 1;

						// QRﾌﾗｸﾞ←利用中
						lcdbm_Flag_QRuse_ICCuse = 2;
					}
// MH810100(E) S.Fujii 2020/08/27 #4764 クレジット利用不可の状態でQRコードが利用できない
// MH810100(S) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
//				}
//				// QR応答データ
//				lcdbm_QR_data_res( 0 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
//				
//				// QRﾃﾞｰﾀ受信 & QRﾃﾞｰﾀ応答(OK)送信後のLCD_IN_CAR_INFO待ち
//				lcdbm_QRans_InCar_status = 1;
				} else {
					// QR応答データ
					lcdbm_QR_data_res( 0 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
				
					// QRﾃﾞｰﾀ受信 & QRﾃﾞｰﾀ応答(OK)送信後のLCD_IN_CAR_INFO待ち
					lcdbm_QRans_InCar_status = 1;
				}
// MH810100(E) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
				break;

			// 操作通知(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// 操作通知		操作ｺｰﾄﾞ
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// 精算中止要求
					case LCDBM_OPCD_PAY_STP:
						// 決済リーダのカード処理中？
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						if( isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ){
// MH810105(S) QRコード決済対応 GT-4100 #6591 精算中、待機画面復帰タイマー経過後もLCD画面が料金表示したままとなる
//						if(isEC_STS_CARD_PROC()) {
						if(isEC_STS_CARD_PROC() || 								// 決済リーダのカード処理中は取消操作を受け付けない
						(ECCTL.Ec_FailureContact_Sts == 1) ||					// 障害連絡票発行中はとりけし操作を受け付けない
// GG129004(S) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
//// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
//						(OPECTL.f_rtm_remote_pay_flg != 0) ||					// 遠隔精算開始受付済み
//// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
// GG129004(E) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
						 (OPECTL.ChkPassSyu != 0 || OPECTL.op_faz == 8)){ 		// 定期通信チェック中か電子媒体停止中
// MH810105(E) QRコード決済対応 GT-4100 #6591 精算中、待機画面復帰タイマー経過後もLCD画面が料金表示したままとなる
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							// 操作通知(精算中止応答(NG))送信	0=OK/1=NG
							lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						}
						// ICｸﾚｼﾞｯﾄ非問合せ中
						else {
// MH810102(S) 電子マネー対応 #5476 【市場不具合】精算ボタンを押しても「精算を開始できませんでした」となり開始できない
//							// 中止イベント送信
//							queset( OPETCBNO, LCD_LCDBM_OPCD_PAY_STP, 0, NULL );
							if(lcdbm_QRans_InCar_status){
								// カード情報待ち状態なので、このタイミングで来たらNG応答
								// 操作通知(精算中止応答(NG))送信	0=OK/1=NG
								lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
							}else{
								flag_stop = 1;		// 1:中止中
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
								// レーンモニタデータ登録(精算中止)
// GG129000(S) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.54)（GM803000流用）
//								SetLaneFreeNum(ryo_buf.zankin);
								SetLaneFreeNum(0);								// 残額クリア
								SetLaneFeeKind(0);								// 料金種別クリア
// GG129000(E) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.54)（GM803000流用）
								ope_MakeLaneLog(LM_PAY_STOP);
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
								// 中止イベント送信
								queset( OPETCBNO, LCD_LCDBM_OPCD_PAY_STP, 0, NULL );
							}
// MH810102(E) 電子マネー対応 #5476 【市場不具合】精算ボタンを押しても「精算を開始できませんでした」となり開始できない
						}
						break;

					// 精算開始要求
					case LCDBM_OPCD_PAY_STA:
						// 操作通知(精算開始応答(NG))送信	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
// MH810102(S) 電子マネー対応 #5476 【市場不具合】精算ボタンを押しても「精算を開始できませんでした」となり開始できない
						// フェールセーフ処理(op_mod02に開始通知を受ける時点で、フェーズずれが生じている。中止しているはずだが何かしらでひかかっている場合)
						if(flag_stop){
							// 待機に戻る
							queset( OPETCBNO, ELE_EVENT_CANSEL, 0, NULL );					// オペに電子媒体の停止を通知					
						}
// MH810102(E) 電子マネー対応 #5476 【市場不具合】精算ボタンを押しても「精算を開始できませんでした」となり開始できない
						break;

					// 領収証発行
					case LCDBM_OPCD_RCT_ISU:
// GG129000(S) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
						// 領収証発行ボタン押下後の領収証印字不可で必ず領収証発行失敗表示するため必ずフラグを立てる
						while ( OPECTL.RECI_SW == 0 ) {							// 領収書ﾎﾞﾀﾝ未使用?
							OPECTL.RECI_SW = 1;									// 領収書ﾎﾞﾀﾝ使用
// GG129000(E) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
// MH810105(S) MH364301 インボイス対応 GT-4100
//						if( Ope_isPrinterReady() == 0 ){
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 						if (Ope_isPrinterReady() == 0 ||
						if ((!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) ||
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// GG129000(S) M.Fujikawa 2023/11/08 ゲート式車番チケットレスシステム対応　不具合#7151
//							(IS_INVOICE && Ope_isJPrinterReady() == 0)) {		// インボイス設定時はレシートとジャーナルが片方でも動かなかった場合
							(IS_INVOICE && Ope_isJPrinterReady() == 0 && ECCTL.Ec_FailureContact_Sts == 0)) {		// インボイス設定時はレシートとジャーナルが片方でも動かなかった場合
// GG129000(E) M.Fujikawa 2023/11/08 ゲート式車番チケットレスシステム対応　不具合#7151
// MH810105(E) MH364301 インボイス対応 GT-4100
							break;
						}
// MH810105(S) MH364301 QRコード決済対応 GT-4100
						if( isEC_USE() && ECCTL.Ec_FailureContact_Sts != 0 ){	// 1:障害連絡票発行処理中／2:障害連絡票発行後
							if (ECCTL.Ec_FailureContact_Sts == 2) {
								// 障害連絡票発行済みの場合、領収証ボタンを受け付けない
								break;
							}
							EcFailureContactPri();								// 障害連絡票発行 ※レシートのみ
							op_SuicaFusokuOff();
							// 6秒間表示する
							lcd_wmsg_dsp_elec(1, ERR_CHR[78], ERR_CHR[79], 0, 0, COLOR_RED, LCD_BLINK_OFF);
							LagTim500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER, 13, lcd_Recv_FailureContactData_edit);
							// LCDに通知
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0E);	// 14=障害連絡票発行済み(コード決済)
							
							ECCTL.Ec_FailureContact_Sts = 2;					// 2:障害連絡票発行後
							if( OPECTL.RECI_SW == 0 ){							// 領収書ﾎﾞﾀﾝ未使用?
								OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
							}
							break;
						}
// MH810105(E) MH364301 QRコード決済対応 GT-4100
						if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
						// 未了確定後なら受け付けない
							break;
						}
						if( announce_flag ){
						// ｶｰﾄﾞ抜き取りｱﾅｳﾝｽ中なら受け付けない
							break;
						}
						if( ReceiptChk() == 1 ){							// ﾚｼｰﾄ発行不要設定時
							break;
						}
						if(	(OPECTL.Pay_mod == 2)&&							// 修正精算
							(!prm_get(COM_PRM, S_PRN, 32, 1, 6))){			// 領収証の発行なし
							BUZPIPI();
							break;
						}
						BUZPI();
// GG129000(S) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
// 						if( OPECTL.RECI_SW == 0 ){							// 領収書ﾎﾞﾀﾝ未使用?
// GG129000(E) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
							if((( OPECTL.ChkPassSyu == 0 )&&				// 問合せ中ではない?
							   ( OPECTL.op_faz != 0 || ryodsp ))			// 待機状態以外?
							   &&
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//								Ec_Settlement_Sts != EC_SETT_STS_CARD_PROC ) // クレジット処理中？
								!isEC_STS_CARD_PROC() )
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							{
								wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
								grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      領収証を発行します      "
// MH810105(S) MH364301 インボイス対応
								OPECTL.f_CrErrDisp = 0;						// エラー非表示
// MH810105(E) MH364301 インボイス対応
							}
// GG129000(S) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
// 							OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
// GG129000(E) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
						}
						break;

					// 起動通知
					case LCDBM_OPCD_STA_NOT:
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
						// 精算完了処理実施後、OPECTL.init_stsに従ってop_init00()へ遷移
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							OPECTL.init_sts = 0xff;	// 初期化未完了(起動通知受信済み)状態とする
						}
						// 1=通常
						else{
							OPECTL.init_sts = 0xfe;	// 初期化完了済み(起動通知受信済み)状態とする
						}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
						// ICクレジット問合せ中
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						if( isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ){
						if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							// リセット要求有無フラグON
							lcdbm_Flag_ResetRequest = 2;
							// op_mod03(精算完了)移行時のloop前で -> op_init00(起動中)へ移行する
						}
						// ICクレジット非問合せ中
						else {
							// 精算中止処理メッセージ送信
							queset( OPETCBNO, LCD_LCDBM_OPCD_STA_NOT, 0, NULL );
						}
						break;

					// 起動完了通知
					case LCDBM_OPCD_STA_CMP_NOT:
// MH810100(S) K.Onodera  2020/02/25 #3858 車番チケットレス(起動待機状態無限ループ防止)
//						if(( OPECTL.Ope_mod == 255 ) && ( OPECTL.init_sts == 1 )){	// 初期化完了状態で起動待機に遷移？
//							// 起動通知(1)受信後、精算中止処理完了前に起動完了通知を受信した場合
//							// FTPは不要なので精算中止処理が完了したらop_mod00()に戻る
// MH810100(S) K.Onodera 2020/03/11 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
//						if( OPECTL.Ope_mod == 255 ){	// 起動待機に遷移？
						if(( OPECTL.init_sts == 0xfe ) || ( OPECTL.init_sts == 0xff )){	// 起動通知受信済み？
// MH810100(E) K.Onodera 2020/03/11 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
							// op_mod02()の状態でFTPしていないのに起動完了通知を受信するということは
							// LCDはFTP不要との判断なので精算中止処理が完了したらop_mod00()に戻る
							OPECTL.init_sts = 1;
// MH810100(S) K.Onodera 2020/03/13 車番チケットレス(待機状態遷移処理共通化)
//							// 起動完了通知受信時の起動完了状態更新
//							PktStartUpStsChg( 2 );
// MH810100(E) K.Onodera 2020/03/13 車番チケットレス(待機状態遷移処理共通化)
// MH810100(E) K.Onodera  2020/02/25 #3858 車番チケットレス(起動待機状態無限ループ防止)
							OPECTL.Ope_mod = 0;
						}
						break;

					// QR停止応答			0=OK／1=NG
					// QR開始応答			0=OK／1=NG
					case LCDBM_OPCD_QR_STP_RES:
					case LCDBM_OPCD_QR_STA_RES:
						// 一旦応答待ちタイマ停止
						Lagcan( OPETCBNO, TIMERNO_QR_START_RESP );
						Lagcan( OPETCBNO, TIMERNO_QR_STOP_RESP );

						// 0=OK
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
						}
						// 1=NG
						else {
							// QRﾘｰﾀﾞ開始/停止応答OK待ちﾘﾄﾗｲｶｳﾝﾀ
							lcdbm_Counter_QR_StartStopRetry++;

							// リトライ回数内？
							if( lcdbm_Counter_QR_StartStopRetry < 3 ){
								if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_QR_STA_RES ){
									// QR開始要求送信
									lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
									// 応答待ちタイマースタート
									Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
								}else{
									// QR停止要求送信
									lcdbm_notice_ope( LCDBM_OPCD_QR_STP_REQ, 0 );
									// 応答待ちタイマースタート
									Lagtim( OPETCBNO, TIMERNO_QR_STOP_RESP, (res_wait_time * 50) );
								}
							}
							// リトライオーバー？
							else {
								lcdbm_Counter_QR_StartStopRetry = 0;
								// 精算継続
							}
						}
						break;
// MH810100(S) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
					// 待機画面通知
					case LCDBM_OPCD_CHG_IDLE_DISP_NOT:
						OPECTL.chg_idle_disp_rec = 1;
						break;
// MH810100(E) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
// MH810100(S) 2020/06/08 #4205【連動評価指摘事項】センターとの通信断時にQR読込→精算完了すると画面が固まる(No.02-0009)
					case LCDBM_OPCD_QR_NO_RESPONSE:			// 37:QR応答後の入庫情報は来ない通知
					 	// QRﾃﾞｰﾀを受信していない状態に戻す
					 	lcdbm_QRans_InCar_status = 0;
// MH810100(S) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
						if (isEC_USE()) {
							// QRフラグクリア
							lcdbm_Flag_QRuse_ICCuse = 0;
							
							// 受付許可送信
							EcSendCtrlEnableData();
						}
// MH810100(E) S.Fujii 2020/08/25 #4753 QRコード読み取り後に入庫情報受信前にICクレジット受付可になる
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
						// 入金で精算完了条件を満たしたが割引待ちで精算未完了と判定した場合のために
						// センター問合せNGであれば入金処理サブを呼び再判定を行う
						// 入金処理サブ：車番チケットレス精算(入庫時刻指定)
						in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);

						// QR取消データ即時登録
						ope_CancelRegist(&MediaDetail);
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
// GG129000(S) T.Nagai 2023/10/17 #7173 QRのセンター問合せタイムアウトすると精算完了できなくなる
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							OPECTL.f_rtm_remote_pay_flg = 0;
							if (ryo_buf.nyukin) {
								in_mony(COIN_EVT, 0);			// 入金再計算
							}
							if (OPECTL.op_faz < 2) {
								cn_stat( 3, 2 );				// 紙幣 & COIN再入金可
							}
							lcdbm_Flag_RemotePay_ICCuse = 0;
						}
// GG129000(E) T.Nagai 2023/10/17 #7173 QRのセンター問合せタイムアウトすると精算完了できなくなる
					 	break;
// MH810100(E) 2020/06/08 #4205【連動評価指摘事項】センターとの通信断時にQR読込→精算完了すると画面が固まる(No.02-0009)
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
					// 遠隔精算（リアルタイム）開始要求
					case LCDBM_OPCD_RTM_REMOTE_PAY_REQ:
// GG129004(S) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
//						if (OPECTL.f_rtm_remote_pay_flg != 0 ||		// 遠隔精算（リアルタイム）開始受付済み
//							OPECTL.op_faz > 1 ||					// 入金中
//							card_use[USE_N_SVC] > 0 ||				// 今回割引あり
//							isEC_PAY_CANCEL() ||					// 決済精算中止中
//							ryo_buf.nyukin > 0 ||					// 入金中
//							Suica_Rec.Data.BIT.CTRL_MIRYO != 0 ||	// 未了中
//							Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0){	// 未了残高照会タイムアウト
						if (OPECTL.op_faz > 1 ||					// 入金中
							card_use[USE_N_SVC] > 0 ||				// 今回割引あり
							isEC_PAY_CANCEL() ||					// 決済精算中止中
							ryo_buf.nyukin > 0 ||					// 入金中
							Suica_Rec.Data.BIT.CTRL_MIRYO != 0 ||	// 未了中
							Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0){	// 未了残高照会タイムアウト
// GG129004(E) M.Fujikawa 2024/11/13 AMS改善要望 遠隔精算開始時に車両を確認させる
							// 遠隔精算（リアルタイム）開始NG
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 1 );
							break;
						}

						if (isEC_USE()) {
							// 決済リーダあり

							if (isEC_STS_CARD_PROC()) {
								// 遠隔精算（リアルタイム）開始NG
								lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 1 );
							}
							else {
								// 入金禁止
								cn_stat(2, 2);

								if (Op_StopModuleWait(REASON_RTM_REMOTE_PAY)) {
									// 受付済み
									OPECTL.f_rtm_remote_pay_flg = 1;
									// 遠隔精算（リアルタイム）開始OK
									lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
									// 遠隔精算開始OK
									lcdbm_Flag_RemotePay_ICCuse = 2;
								}
								else {
									// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ開始		受付不可受信待ちタイマー(5秒) + 1秒
									Lagtim( OPETCBNO, TIMERNO_ICCREDIT_STOP, ((EC_DISABLE_WAIT_TIME+1) * 50) );
									// 停止確認中
									lcdbm_Flag_RemotePay_ICCuse = 1;
								}
							}
						}
						else {
							// 決済リーダなし

							// 受付済み
							OPECTL.f_rtm_remote_pay_flg = 1;
							// 入金禁止
							cn_stat(2, 2);
							// 遠隔精算（リアルタイム）開始OK
							lcdbm_notice_ope( LCDBM_OPCD_RTM_REMOTE_PAY_RES, 0 );
						}
						break;
					// 遠隔精算（リアルタイム）受付中止
					case LCDBM_OPCD_RTM_REMOTE_PAY_CAN:
						if (OPECTL.f_rtm_remote_pay_flg != 0) {
							OPECTL.f_rtm_remote_pay_flg = 0;
// GG129004(S) M.Fujikawa 2024/11/19 AMS改善要望 遠隔精算開始時に車両を確認させる
//							if (ryo_buf.nyukin) {
//								in_mony(COIN_EVT, 0);			// 入金再計算
//							}
//							if (OPECTL.op_faz < 2) {
//								cn_stat( 3, 2 );				// 紙幣 & COIN再入金可
//							}
//							if (lcdbm_Flag_RemotePay_ICCuse == 2) {
//								// 決済リーダ再活性化
//								Op_StopModuleWait_sub(2);
//							}
//							lcdbm_Flag_RemotePay_ICCuse = 0;
// GG129004(E) M.Fujikawa 2024/11/19 AMS改善要望 遠隔精算開始時に車両を確認させる
						}
						break;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				}
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// ICクレジット問合せ中
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if( isEC_USE() && Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC ){
				if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					// リセット要求有無フラグON
					lcdbm_Flag_ResetRequest = 1;
					// op_mod03(精算完了)移行時のloop前で -> op_init00(起動中)へ移行する
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
					OPECTL.init_sts = 0;	// 初期化未完了状態とする
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
				}
				// ICクレジット非問合せ中
				else {
					// 精算中止処理メッセージ送信
					queset( OPETCBNO, LCD_LCD_DISCONNECT, 0, NULL );
				}
				break;

			// ICｸﾚｼﾞｯﾄ_受付不可はELE_EVT_STOP(電子媒体停止完了受信)ｲﾍﾞﾝﾄ受信時の
			// REASON_QR_USE(QRﾃﾞｰﾀ受信時にICｸﾚｼﾞｯﾄ停止したいという理由)受信の箇所で処理を行う

			// ICｸﾚｼﾞｯﾄ_処理中(処理開始)
			// 決済状態ﾃﾞｰﾀ(From_ICC)(lcdbm_ICC_Settlement_Status)
			case LCD_ICC_SETTLEMENT_STS:
				// ｶｰﾄﾞ処理中
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if( lcdbm_ICC_Settlement_Status == EC_SETT_STS_CARD_PROC ){
				if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					lcdbm_Counter_QR_StartStopRetry = 0;
					// 操作通知(QR停止要求)送信	0固定
					lcdbm_notice_ope( LCDBM_OPCD_QR_STP_REQ, 0 );
// MH810100(S) S.Nishimoto 2020/07/14 車番チケットレス(電子決済端末対応)
					// 操作通知(カード決済状態通知)
// MH810105(S) MH364301 QRコード決済対応 GT-4100
//// MH810103(s) 電子マネー対応 未了通知
////					lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x01);	// クレジットカード処理中
//					if(isEcBrandNoEMoney(RecvBrandResTbl.no, 0) ){
					if (EcUseKindCheck(convert_brandno((ushort)RecvBrandResTbl.no))) {
// MH810105(E) MH364301 QRコード決済対応 GT-4100
// MH810105(S) 2021/12/03 iD対応
//						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x02);	// 電子マネー処理中
						if(RecvBrandResTbl.no == BRANDNO_ID){
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0b);	// 電子マネー処理中(iD)
// MH810105(S) MH364301 QRコード決済対応 GT-4100
						}else if(RecvBrandResTbl.no ==BRANDNO_QR){
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0b);	// 電子マネー処理中(QRもiDと同じ)
// MH810105(E) MH364301 QRコード決済対応 GT-4100
						}else{
							lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x02);	// 電子マネー処理中(iD以外)
						}
// MH810105(E) 2021/12/03 iD対応
					}else{
						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x01);	// クレジットカード処理中
					}
// MH810103(s) 電子マネー対応 未了通知
// MH810100(E) S.Nishimoto 2020/07/14 車番チケットレス(電子決済端末対応)
// MH810100(S) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
					// QRﾌﾗｸﾞ== 利用確認中	(0 = idle, 1 = 利用確認中, 2 = 利用中)
					if( lcdbm_Flag_QRuse_ICCuse == 1 ){
						// ICクレジット停止待ちタイマ停止
						Lagcan( OPETCBNO, TIMERNO_ICCREDIT_STOP );
						// QRﾃﾞｰﾀ応答(NG))送信	結果(0:OK,1:NG(排他),2:NG(枚数上限))
						lcdbm_QR_data_res( 1 );		// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
						// QRフラグクリア
						lcdbm_Flag_QRuse_ICCuse = 0;
					}
// MH810100(E) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
					// LCDBM_OPCD_QR_STP_REQ(QR停止要求), LCDBM_OPCD_QR_STA_REQ(QR開始要求)の応答待ちﾀｲﾏ
					// QRﾘｰﾀﾞ開始/停止応答OK待ちﾀｲﾏ	48-0021	ﾃﾞｰﾀ受信監視ﾀｲﾏ	(問合せ)	⑤⑥:受信監視ﾀｲﾏ(問合せ) 1～99=1秒～99秒
					Lagtim( OPETCBNO, TIMERNO_QR_STOP_RESP, (res_wait_time * 50) );
				}
				break;

			// ICクレジット決済結果データ(成功/失敗)
			case LCD_ICC_SETTLEMENT_RSLT:
				// ICクレジット接続あり？
// MH810103(s) 電子マネー対応
//				if( prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1 ){
				if(isEC_USE()) {
// MH810103(e) 電子マネー対応
// MH810100(S) S.Nishimoto 2020/08/18 #4608 車番チケットレス(クレジット決済後、カード抜けタイムアウトより前に待機に戻ってしまう)
//// MH810100(S) S.Nishimoto 2020/07/14 車番チケットレス(電子決済端末対応)
//					// 操作通知(カード決済状態通知)
//					lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x00);	// ポップアップ消去(正常終了)
//// MH810100(E) S.Nishimoto 2020/07/14 車番チケットレス(電子決済端末対応)
// MH810100(E) S.Nishimoto 2020/08/18 #4608 車番チケットレス(クレジット決済後、カード抜けタイムアウトより前に待機に戻ってしまう)
// MH810103(s) 電子マネー対応 #5469 E3262(決済不良(取引指示額と取引額に差異あり))発生で精算完了してしまう
					if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR){
						// 受信した決済額と受信した選択商品データとで差異があった
						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=未了(再精算案内)
					}else
// MH810103(e) 電子マネー対応 #5469 E3262(決済不良(取引指示額と取引額に差異あり))発生で精算完了してしまう
					// 決済結果OK
// MH810103(s) 電子マネー対応
//					if( (lcdbm_ICC_Settlement_Result == EPAY_RESULT_OK) ||				// OK
//						(lcdbm_ICC_Settlement_Result == EPAY_RESULT_MIRYO_AFTER_OK) ){	// 未了確定後決済OK	
					if( (lcdbm_ICC_Settlement_Result == EPAY_RESULT_OK)  ){				// OK
// MH810103(e) 電子マネー対応
// MH810100(S) S.Nishimoto 2020/08/18 #4608 車番チケットレス(クレジット決済後、カード抜けタイムアウトより前に待機に戻ってしまう)
						// 操作通知(カード決済状態通知)
						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x00);	// ポップアップ消去(決済OK)
// MH810100(E) S.Nishimoto 2020/08/18 #4608 車番チケットレス(クレジット決済後、カード抜けタイムアウトより前に待機に戻ってしまう)
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
//						lcdbm_pay_rem_chg(1);							// 残高変化通知(精算完了)
//// MH810100(E) S.Takahashi 2020/03/02 割引額が遅れて表示される
//						// 操作通知(精算完了通知(精算済み案内))送信
//						// 0=0円表示/1=ﾗｸﾞﾀｲﾑ延長画面/2=精算済み案内
//						lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 2 );
						if ( cmp_send != 3 ) {		// 同種の残高変化通知(精算完了)未送信
							lcdbm_pay_rem_chg(1);	// 残高変化通知(精算完了)
							cmp_send = 3;
						}
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041

// MH810105(S) #6469 コインメック異常時に、精算完了するとフェーズずれが起こる
//// MH810100(S) Y.Watanabe 2020/03/25 車番チケットレス_ICクレジット決済と現金投入の同時発生の修正
////						// 釣り銭(釣銭額)
////						if( ryo_buf.turisen == 0 ){
////							ret = 1;				// 精算完了(釣り無し) 	精算完了処理(op_mod03)へ
////						}
////						else {
////							ret = 2;				// 精算完了(釣り有り) 	精算完了処理(op_mod03)へ
////						}
						// OnlineDiscount内のin_monyで同じことを実施しているし、決済リーダがある場合はop_fazを8にして停止待ちするためここで書き換えてはダメ
//						ac_flg.cycl_fg = 10;				// 入金中
//						cn_crdt();
//						ac_flg.cycl_fg = 11;				// 精算完了
//						if( OPECTL.op_faz != 2 ){
//							cn_stat( 2, 2 );				// 入金不可
//							Lagtim( OPETCBNO, 2, 10*50 );	// ﾒｯｸ監視10s
//							OPECTL.op_faz = 2;
//						}
//// MH810100(E) Y.Watanabe 2020/03/25 車番チケットレス_ICクレジット決済と現金投入の同時発生の修正
// MH810105(E) #6469 コインメック異常時に、精算完了するとフェーズずれが起こる
					}
					// 決済結果NG
					else {
// MH810100(S) S.Nishimoto 2020/08/18 #4608 車番チケットレス(クレジット決済後、カード抜けタイムアウトより前に待機に戻ってしまう)
						// 操作通知(カード決済状態通知)
						lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x09);	// ポップアップ消去(決済NG)
// MH810100(E) S.Nishimoto 2020/08/18 #4608 車番チケットレス(クレジット決済後、カード抜けタイムアウトより前に待機に戻ってしまう)
						lcdbm_Counter_QR_StartStopRetry = 0;
// MH810105(S) #6275 処理未了残高照会タイムアウト後にQRコードリーダーが読み取り状態になる。
//						// 操作通知(QR開始要求)	送信	0固定
//						lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
//						// LCDBM_OPCD_QR_STP_REQ(QR停止要求), LCDBM_OPCD_QR_STA_REQ(QR開始要求)の応答待ちﾀｲﾏ
//						// QRﾘｰﾀﾞ開始/停止応答OK待ちﾀｲﾏ	48-0021	ﾃﾞｰﾀ受信監視ﾀｲﾏ	(問合せ)	⑤⑥:受信監視ﾀｲﾏ(問合せ) 1～99=1秒～99秒
//						Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
						// 完了した場合は、QRを開始しない
						switch(lcdbm_ICC_Settlement_Result){
						case EPAY_RESULT_MIRYO:				// 未了確定
						case EPAY_RESULT_MIRYO_ZANDAKA_END:	// 未了残高照会完了
						case EPAY_RESULT_PAY_MIRYO:			// 支払未了（決済OK）
							break;
						default:
							// 操作通知(QR開始要求)	送信	0固定
							lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
							// LCDBM_OPCD_QR_STP_REQ(QR停止要求), LCDBM_OPCD_QR_STA_REQ(QR開始要求)の応答待ちﾀｲﾏ
							// QRﾘｰﾀﾞ開始/停止応答OK待ちﾀｲﾏ	48-0021	ﾃﾞｰﾀ受信監視ﾀｲﾏ	(問合せ)	⑤⑥:受信監視ﾀｲﾏ(問合せ) 1～99=1秒～99秒
							Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
							break;
						}
// MH810105(E) #6275 処理未了残高照会タイムアウト後にQRコードリーダーが読み取り状態になる。

						// ICクレジット問合せ中により、精算中止を保留していた場合にここで要求を行う
						if( lcdbm_Flag_ResetRequest == 1 ){
							// 切断による中止要求
							queset( OPETCBNO, LCD_LCD_DISCONNECT, 0, NULL );
						}else if( lcdbm_Flag_ResetRequest == 2 ){
							// 初期化通知による中止要求
							queset( OPETCBNO, LCD_LCDBM_OPCD_STA_NOT, 0, NULL );
						}
					}
				}
				// ICｸﾚｼﾞｯﾄ設定なし
				else {
					// nothing todo
				}
				break;

			// ICクレジット停止待ちタイムアウト
			case TIMEOUT_ICCREDIT_STOP:
				// ICクレジット接続あり
// MH810103(s) 電子マネー対応
//				if( prm_get( COM_PRM, S_PAY, 24, 1, 6 ) == 1 ){
				if(isEC_USE()) {
// MH810103(e) 電子マネー対応
					// ICクレジット停止待ちタイマ停止
					Lagcan( OPETCBNO, TIMERNO_ICCREDIT_STOP );

					// QRﾌﾗｸﾞ== 利用確認中	(0 = idle, 1 = 利用確認中, 2 = 利用中)
					if( lcdbm_Flag_QRuse_ICCuse == 1 ){
							// QRﾃﾞｰﾀ応答(NG))送信	結果(0:OK,1:NG(排他),2:NG(枚数上限))
							lcdbm_QR_data_res( 1 );		// 結果(0:OK,1:NG(排他),2:NG(枚数上限))

// MH810100(S) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
						// QRフラグクリア
						lcdbm_Flag_QRuse_ICCuse = 0;
// MH810100(E) S.Fujii 2020/08/31 #4778 QR利用確認タイムアウト修正
					}
				}
				// ICｸﾚｼﾞｯﾄ設定なし
				else {
					// nothing todo
				}
 				break;

			// QRﾘｰﾀﾞ開始/停止応答OK待ちﾀｲﾏ
			case TIMEOUT_QR_START_RESP:
			case TIMEOUT_QR_STOP_RESP:
				// QRﾘｰﾀﾞ開始/停止応答OK待ちﾘﾄﾗｲｶｳﾝﾀ
				lcdbm_Counter_QR_StartStopRetry++;

				// リトライ回数内？
				if( lcdbm_Counter_QR_StartStopRetry < 3 ){
					if( msg == TIMEOUT_QR_START_RESP ){
						// QR開始要求送信
						lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
						// 応答待ちタイマースタート
						Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
					}else{
						// QR停止要求送信
						lcdbm_notice_ope( LCDBM_OPCD_QR_STP_REQ, 0 );
						// 応答待ちタイマースタート
						Lagtim( OPETCBNO, TIMERNO_QR_STOP_RESP, (res_wait_time * 50) );
					}
				}
				// リトライオーバー？
				else {
					lcdbm_Counter_QR_StartStopRetry = 0;
					// 精算継続
				}
				break;
// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
			case TIMEOUT28:
				ryo_buf.in_coin[2] = ++autopay_coin_cnt;
				queset( OPETCBNO, COIN_IN_EVT, 0, NULL );	// 入金あり
				queset( OPETCBNO, COIN_EN_EVT, 0, NULL );	// 払出し可状態
				Lagcan(OPETCBNO, 28);
				break;
#endif // (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算

// MH810100(S) S.Nishimoto 2020/09/11 車番チケットレス (#4607 精算中に決済リーダのエラーが発生しても精算画面に「クレジット利用不可」が表示されない)
			case TIMEOUT26:
				if( DspSts == LCD_WMSG_OFF ){						// ﾜｰﾆﾝｸﾞﾒｯｾｰｼﾞ表示状態がＯＦＦ
					dspCyclicErrMsgRewrite();
				}
				Lagtim( OPETCBNO, 26, 3*50 );						// ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ(3s)
				break;
// MH810100(E) S.Nishimoto 2020/09/11 車番チケットレス (#4607 精算中に決済リーダのエラーが発生しても精算画面に「クレジット利用不可」が表示されない)

			default:
				break;
		}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応_変更))

		if( ret == 10 ){											// 取消(中止)?
// MH321800(S) hosoda ICクレジット対応
			// ｱﾅｳﾝｽｴﾝﾄﾞフラグOFFの時
			if( !announce_end ){
				// ｶｰﾄﾞ差込中
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if( Suica_Rec.Data.BIT.CTRL_CARD ){
				if( isEC_CARD_INSERT() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					// ｱﾅｳﾝｽ中でない時、ｶｰﾄﾞ抜き取りｱﾅｳﾝｽON
					if (!announce_flag) {
						// 抜き取りﾀｲﾏｰ起動
						Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );
						announce_flag = 1;		// ｱﾅｳﾝｽフラグON
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//						// 「カードをお取り下さい」表示とアナウンスをする
//						ec_MessageAnaOnOff( 1, 3 );
//						// 磁気リーダに何も無ければシャッター閉する
//						if( RD_mod < 9 ){
//							rd_shutter();
//						}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
					}
					// カード抜き取り、または、タイムアウトを待つため、retをクリアする
					ret = 0;
					continue;
				}
				// ｶｰﾄﾞ抜き取った
				else {
					ope_anm( AVM_STOP );						// 放送停止ｱﾅｳﾝｽ
					// 「カードをお取り下さい」表示をもとに戻す
					ec_MessageAnaOnOff( 0, 0 );
					announce_end = 1;							// ｱﾅｳﾝｽｴﾝﾄﾞフラグON
					announce_flag = 0;							// ｱﾅｳﾝｽフラグOFF
				}
			}
// MH810103 GG119202(S) みなし決済扱い時の動作
//			if (ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0) {	// WAON決済異常が発生している
			if (ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0 ||	// WAON決済異常が発生している
				ERR_CHK[mod_ec][ERR_EC_SETTLE_ABORT] != 0) {		// 決済処理失敗が発生している
// MH810103 GG119202(E) みなし決済扱い時の動作
				// 券はリーダー内に保留するので抜き取りの待ち合わせはしない
				// 未了タイムアウトによる休業の場合、券を戻さない
				if (RD_mod == 8) {
					opr_snd(91);		// read中なら停止
				}
			}
// MH810100(S) K.Onodera 2020/2/18 #3870 初期画面に戻るまで精算できなくなってしまう
			// 精算中の応答が必要？
			if( require_cancel_res ){
				// 精算中止応答(OK)
				lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
			}
// MH810100(E) K.Onodera 2020/2/18 #3870 初期画面に戻るまで精算できなくなってしまう
// MH321800(E) hosoda ICクレジット対応
			_di();
			CN_escr_timer = 0; 										// ｴｽｸﾛﾀｲﾏｰｷｬﾝｾﾙ
			_ei();
			safecl( 7 );											// 金庫枚数算出
// MH810100(S) K.Onodera 2020/03/13 車番チケットレス(待機状態遷移処理共通化)
			if(( OPECTL.Ope_mod == 0 ) && ( OPECTL.init_sts == 1 )){
				// 起動完了通知受信で待機状態に遷移
				ope_idle_transit_common();	// 待機状態遷移共通処理
			}
// MH810100(E) K.Onodera 2020/03/13 車番チケットレス(待機状態遷移処理共通化)
		}
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//		if ( credit_result < 0 )	//精算異常
//		{
//			creInfoInit();			// クレジット情報クリア
//			memset ( &ryo_buf.credit, 0, sizeof(credit_use));
//			read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
//			opr_snd( 2 );										// ｶｰﾄﾞ排出
//			creMessageAnaOnOff( 1, (short)(credit_result * -1) );
//			// クレジットカードエラー時、サイクリック表示有効なら再開
//			if (dspIsCyclicMsg()) {		// サイクリック判別
//				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
//			}
//			credit_result = 0;
//			credit_error_ari = 1;
//			if ( credit_Timeout8Flag == 1 ){
//				in_mony ( KEY_TEN_F4, 0 );
//				credit_result = 999;								// 完了
//			}
//			OPECTL.InquiryFlg = 0;		// 外部照会中フラグOFF
//			Suica_Ctrl( S_CNTL_DATA, 0x01 );							// Suica利用を不可にする	
//// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////			Edy_SndData01();								// Edy利用復活（設定あれば）
//// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( MIFARE_CARD_DoesUse ){						// Mifareが有効な場合
//				op_MifareStart();							// Mifareを利用復活（再開）
//			}
//			if( ryo_buf.nyukin && OPECTL.credit_in_mony ){	// 入金ありでｸﾚｼﾞｯﾄとのすれ違い入金の場合
//				in_mony ( COIN_EVT, 0 );					// ｺｲﾝｲﾍﾞﾝﾄ動作をさせて、入金処理(ryo_bufｴﾘｱの更新)を行う
//				OPECTL.credit_in_mony = 0;					// すれ違いﾌﾗｸﾞｸﾘｱ
//			}
//		}
//		else if ( credit_result == 1 )	//精算成功
//		{
//			read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
//			opr_snd( 2 );										//ｶｰﾄﾞ排出
//			creMessageAnaOnOff( 0, 0 );
//			credit_result = 999;								// 完了
//			FurikaeMotoSts = 0;			// 振替元状態フラグ 0:チェック不要
//		}
//		// クレジット関連イベント待ち中にすれ違いで入金が完了するﾀｲﾐﾝｸﾞがあるため
//		// 
//		if( ryo_buf.credit.pay_ryo != 0L && ryo_buf.credit.cre_type == CREDIT_CARD){
//			if( credit_result == 999 && ret != 0 ){
//				if (credit_nyukin == 1){					// クレジット精算前の入金あり
//					ret = credit_turi;
//				}
//				else{
//					if(ryo_buf.nyukin != 0){				// クレジット精算後の入金あり
//						ryo_buf.turisen = ryo_buf.nyukin - ryo_buf.dsp_ryo;
//						ret = 2;
//					}
//					else{
//						ret = credit_turi;
//					}
//				}
//				break;
//			}
//		}else{
//			if( ret != 0 ){
//				// ｸﾚｼﾞｯﾄｶｰﾄﾞ挿入済み＆結果が正常以外＆ｸﾚｼﾞｯﾄHOST問い合わせ動作開始＆電子媒体での精算がない
//				if( RD_Credit_kep && credit_result <= 0 && OPECTL.InquiryFlg && !e_incnt ){
//					opr_snd( 2 );										//ｶｰﾄﾞ排出					
//				}
//				break;
//			}
//		}
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
		if ( ((ret == 1) || (ret == 2)) && (r_zero != 0) ) {	// 精算完了＆0円以外精算
			lcdbm_notice_ope(LCDBM_OPCD_PAY_CMP_NOT, 2);	// 精算完了通知
		}
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
		// 精算完了/とりけし(1=釣銭なし,2=釣銭あり,3=ﾒｯｸｴﾗｰ発生,10=とりけし)
		if (ret != 0) {
			break;
		}
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
		if (OPECTL.InquiryFlg == 0) {		// 外部照会中フラグOFF
// MH810100(S) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
//			if (FurikaeMotoSts == 2) {		// 振替元状態フラグ 2:車なし
//				msg = CANCEL_EVENT;
//				FurikaeMotoSts = 0;
//				FurikaeCancelFlg = 1;
//				goto _MSG_RETRY;	// ※先にキャンセル処理を行いたいため、quesetではなくgotoする
//			}
// MH810100(E) K.Onodera 2020/03/09 車番チケットレス（精算中止処理変更）
		}
	}


	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 2 );											// ﾀｲﾏｰ2ﾘｾｯﾄ(ﾒｯｸ監視用)
	Lagcan( OPETCBNO, 6 );											// ﾀｲﾏｰ6ﾘｾｯﾄ(定期券問合せ監視ﾀｲﾏｰ)
	Lagcan( OPETCBNO, 7 );											// ﾀｲﾏｰ7ﾘｾｯﾄ(読取ｴﾗｰ表示用ﾀｲﾏｰ)
	Lagcan( OPETCBNO, 8 );											// ﾀｲﾏｰ8ﾘｾｯﾄ(入金後戻り判定ﾀｲﾏｰ)
	Lagcan( OPETCBNO, 10 );											// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
	Lagcan( OPETCBNO, 11 );										
	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );				// Suica関連ﾀｲﾏｰｸﾘｱ
	LagCan500ms( LAG500_MIF_WRITING_FAIL_BUZ );						// NGﾌﾞｻﾞｰ
// MH321800(S) G.So ICクレジット対応
	LagCan500ms(LAG500_EC_START_REMOVAL_TIMER);						// 決済結果NG受信時のｶｰﾄﾞ抜き取りタイマ起動ﾀｲﾏｰﾘｾｯﾄ
	Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );						// ecリサイクル表示用ﾀｲﾏｰﾘｾｯﾄ
	Lagcan( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT );					// カード抜き取り待ちﾀｲﾏｰﾘｾｯﾄ
// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
	Lagcan(OPETCBNO, 28);											//100円投入タイマ
#endif	// (1 == AUTO_PAYMENT_PROGRAM)
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算

	// 決済リーダ関連文言がまだ表示されていたら元に戻す
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if( ec_MessagePtnNum != 0){
//		if( ec_MessagePtnNum == 98 ){
	if( isEC_MSG_DISP() ){
		if( isEC_NG_MSG_DISP() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
			Ope2_WarningDispEnd();
		} else {
			ec_MessageAnaOnOff( 0, 0 );
		}
	}
// MH321800(E) G.So ICクレジット対応
	if( MifStat != MIF_WROTE_CMPLT) {								// 停止済みでなければ
		op_MifareStop();
	}
	OPECTL.InquiryFlg = 0;		// 外部照会中フラグOFF

	OPECTL.Fin_mod = ret;
	if( ret != 10 ){
		OPECTL.Ope_mod = 3;											// 精算完了処理へ
// MH810100(S) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
//		if( f_al_card_wait == 1 ){		// 1=磁気カードが入れられ、al_card() 実施前に電子マネーリーダなどの停止待ちをしている時に精算完了になった
//			// 前方排出等のコマンド送信時にRD_mod = 10となり、送信完了をリーダーから受信すると、RD_mod = 11となる。
//			// 10,11中は、他で排出要求させている可能性があるので、ここでは戻さない（空回り対策）
//			if((RD_mod != 10) && (RD_mod != 11) ){
//				opr_snd( 13 );			// 保留位置にあるカードでも前方排出
//				if( pas_kep == (char)-1 ){
//					// 上記で戻すのでpas_kep=2として、op_mod03でカード戻し判定しないようにする。
//					pas_kep = 2;		// 定期返却中
//				}
//			}
//		}else{
//			if( pas_kep && ( RD_mod != 10 && RD_mod != 11 )){		// 精算完了しているにも関わらず、定期が保留位置に残っている場合
//				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
//				opr_snd( 2 );		//前方排出
//			}
//		}
// MH810100(E) K.Onodera 2020/02/18 車番チケットレス(不要処理削除)
	}
	else{
		cn_stat( 8, 0 );	// 保有枚数送信
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//		Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );	// 中止時の金銭管理ログデータ作成
//		Log_regist( LOG_MONEYMANAGE_NT );							// 金銭管理ログ登録
		if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {	// 中止時の金銭管理ログデータ作成
			Log_regist( LOG_MONEYMANAGE_NT );							// 金銭管理ログ登録
		}
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
		LagCan500ms( LAG500_MIF_LED_ONOFF );							// ICC ｶﾞｲﾄﾞLED点滅
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// 不具合修正(S) K.Onodera 2016/10/13 #1521 振替元のフラップが下降したままになってしまう
////		if (ope_imf_GetStatus() == 2) {
//		if( ope_imf_GetStatus() == 2 || PiP_GetFurikaeSts() ){
//// 不具合修正(E) K.Onodera 2016/10/13 #1521 振替元のフラップが下降したままになってしまう
//			ope_Furikae_stop(vl_frs.lockno, 0);			// 振替元車室復元中止
//			if (FurikaeCancelFlg) {
//			// 車がなくなったことによる精算中止なら、振替精算失敗を通知
//				ope_imf_Answer(0);			// 振替精算失敗
//			}
//		}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
		if(OPECTL.ChkPassSyu)
			blink_end();									// 点滅終了
		if(OPECTL.Pay_mod == 2){										// 精算の種類(通常精算)
			OPECTL.Pay_mod = 0;
			wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
			OPECTL.Kakari_Num = 0;										// 係員№ｸﾘｱ
		}
			f_MandetMask = 1;
	}
	ope_imf_End();
// MH322914(S) K.Onodera 2016/09/12 AI-V対応
	PiP_FurikaeEnd();
// MH322914(E) K.Onodera 2016/09/12 AI-V対応
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	Suica_Rec.Data.BIT.ADJUSTOR_START = 0;							// 精算開始フラグセット
	PayDataErrDispClr();

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 残額不足ﾒｯｾｰｼﾞ表示                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_SuicaFusoku_Msg			                           |*/
/*| PARAMETER    : dat	：表示金額	                                       |*/
/*| 			 : buff	：表示ﾃﾞｰﾀ格納領域                                 |*/
/*| RETURN VALUE : void				　                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : suzuki                                                 |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	op_SuicaFusoku_Msg( ulong dat,	uchar *buff, uchar use_kind )
{
	unsigned char	w_buf[5];
	unsigned char	d_buf[10];
	unsigned char	w_Msg[30];
	short			i;
	unsigned long	bb;
	ushort siz = 5;

	if( siz != 0 ) {
		for( bb = 1l,i = 0; i < siz; i++ ) {
			bb = bb * 10l;
		}
	}
	dat = ( dat % bb );
	bb = bb / 10l;
	for( i = 0;i < siz; i++ ) {
		w_buf[i] = (unsigned char)(( dat / bb ) | 0x30);
		dat = ( dat % bb );
		bb = bb / 10l;
	}
	zrschr( w_buf, (uchar)siz );
	as1chg( w_buf, d_buf, (uchar)siz );
	if( use_kind == 0 )
		memcpy(&w_Msg[0], "  残額不足（残額", 16);
	else
		memcpy(&w_Msg[0], "  残高不足（残高", 16);
	memcpy(&w_Msg[16], d_buf, 10);
	memcpy(&w_Msg[26], "円）", 4);
	memcpy(buff, w_Msg, sizeof(w_Msg));
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	// ポップアップ表示要求(残高不足)
// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
//	lcdbm_notice_dsp( POP_SHORT_BALANCE, 0 );
	lcdbm_notice_dsp2( POP_INQUIRY_NG, 6, dat );
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| 6・7行目に表示したｱﾗｰﾑ消去                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_SuicaFusokuOff			                           |*/
/*| PARAMETER    : void				                                       |*/
/*| RETURN VALUE : void				　                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : suzuki                                                 |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	op_SuicaFusokuOff( void )
{
	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );	// ﾀｲﾏｰｸﾘｱ
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( edy_dsp.BIT.edy_Pay_Retry || edy_dsp.BIT.edy_Pay_Retry_Sub ){ // 残額不足画面表示中（引落可)かｻﾌﾞﾌﾗｸﾞがｾｯﾄ済み？
//		if( !PayData.Electron_data.Edy.pay_ryo )		// Edy未決済
//			Edy_StopAndStart();							// ｶｰﾄﾞ検知停止＆開始指示送信
//		edy_dsp.BIT.edy_Pay_Retry_Sub = 0;				// 残額引落し画面表示ｻﾌﾞﾌﾗｸﾞをｸﾘｱ
//	}
//	if( edy_dsp.BIT.edy_dsp_change || edy_dsp.BIT.edy_dsp_Warning ){// ｱﾗｰﾑﾒｯｾｰｼﾞが表示したままの場合
//		edy_dsp.BIT.edy_dsp_change = 0;					// ｱﾗｰﾑﾒｯｾｰｼﾞﾌﾗｸﾞを初期化
//		edy_dsp.BIT.edy_dsp_Warning = 0;				// ｱﾗｰﾑﾒｯｾｰｼﾞﾌﾗｸﾞを初期化
//		if(!edy_dsp.BIT.edy_Miryo_Loss){				// Edy減算失敗ﾀｲﾑｱｳﾄ後のｱﾗｰﾑ表示中？
//			if( PayData.Electron_data.Edy.pay_ryo != EDY_USED && OPECTL.op_faz != 8 && // 07-01-22変更 //
//				(OPECTL.op_faz != 3 || OPECTL.op_faz != 4 ))	// 取消＆ﾀｲﾑｱｳﾄ時はｶｰﾄﾞ検知開始指示を出さない 07-01-22追加 //
//				Edy_StopAndStart();						// ｶｰﾄﾞ検知停止＆開始指示送信
//		}
//		if( OPECTL.Ope_mod == 0 && !Edy_Rec.edy_status.BIT.ZAN_SW )	// 駐車券挿入待ち？
//			Lagtim( OPETCBNO, 11, 2 );					// 残高照会画面切替ﾀｲﾏｰを再ｽﾀｰﾄ
//		else if( (OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 13 || OPECTL.Ope_mod == 22 ) && OPECTL.op_faz != 8 )
//			Lagtim( OPETCBNO, 10, 2 );					// ｻｲｸﾘｯｸ表示ﾀｲﾏｰを再ｽﾀｰﾄ
//	}
//	else if( edy_dsp.BIT.edy_Miryo_Loss ){				// Edy減算失敗ﾀｲﾑｱｳﾄ後のｱﾗｰﾑ表示中？
//		edy_dsp.BIT.edy_Miryo_Loss = 0;					// Edy減算失敗ﾀｲﾑｱｳﾄ後のｱﾗｰﾑ表示ﾌﾗｸﾞｸﾘｱ
//	}
//	else if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){		// Suica減算失敗ﾀｲﾑｱｳﾄ後のｱﾗｰﾑ表示中？
	if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){			// Suica減算失敗ﾀｲﾑｱｳﾄ後のｱﾗｰﾑ表示中？
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) hosoda ICクレジット対応
		Ope2_ErrChrCyclicDispStop();					// ワーニング サイクリック表示停止
// MH321800(E) hosoda ICクレジット対応
		Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;			// ｱﾗｰﾑ表示ﾌﾗｸﾞｸﾘｱ
	}
	Lcd_WmsgDisp_OFF2();
	Lcd_WmsgDisp_OFF();
	dsp_fusoku = 0;
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//// GG116200(S) D.Inaba ICクレジット対応
//	ec_MessagePtnNum = 0;								// LCD表示パターンクリア
//// GG116200(E) D.Inaba ICクレジット対応
	ECCTL.ec_MessagePtnNum = 0;							// LCD表示パターンクリア
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
}

/*[]----------------------------------------------------------------------[]*/
/*| 保留中のｲﾍﾞﾝﾄ再Post関数                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nyukin_delay_check			                           |*/
/*| PARAMETER    : 					                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2006-10-17                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	nyukin_delay_check( ushort *postdata, uchar count )
{
	uchar i;
	for( i=0; i<count; i++ ){
		if( postdata[i] ){
			queset( OPETCBNO, postdata[i], 0, NULL );		// 保留中の紙幣ｲﾍﾞﾝﾄを通知
		}
	}
	memset( nyukin_delay,0,sizeof( nyukin_delay ));			// ｲﾍﾞﾝﾄｴﾘｱの初期化
	delay_count = 0;										// ｶｳﾝﾄ数の初期化
}

/*[]----------------------------------------------------------------------[]*/
/*| 待機復帰用最終ﾀｲﾏｰ                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_return_last_timer			                           |*/
/*| PARAMETER    : 					                                       |*/
/*| 備考         : 待機画面に遷移させるための最終ﾀｲﾏｰで本関数がｺｰﾙされた　 |*/
/*| 			 : 場合はSuicaの状態に関わらず待機画面に遷移する        　 |*/
/*| 			 : 待機に戻るためのﾛｼﾞｯｸとしてはTIMEOUT2を使用する         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2006-10-11                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	Op_ReturnLastTimer( void )
{
	Suica_Rec.Data.BIT.CTRL = 0;			// 待機用にSuicaを不可状態に設定しておく
	Lagtim( OPETCBNO, 2, 25 );				// ﾀｲﾏｰ2ｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)(500ms)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ){	// Edy設定があれば
//		Op_Cansel_Wait_sub( 1 );			// Edy終了済みﾌﾗｸﾞをｾｯﾄする
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
}

/*[]----------------------------------------------------------------------[]*/
/*| 入金処理ｻﾌﾞ                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : in_mony( msg )                                          |*/
/*| PARAMETER    : msg    : ﾒｯｾｰｼﾞ                                         |*/
/*|                paymod :  0 = 通常, 1 = 修正                            |*/
/*| RETURN VALUE : ret    :  0 = 継続                                      |*/
/*|                       :  1 = 精算完了(釣り無し)                        |*/
/*|                       :  2 = 精算完了(釣り有り)                        |*/
/*|                       : 10 = 精算中止(待機へ戻る)                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	in_mony( ushort msg, ushort paymod )
{
	short	ret = 0;
	char	w_op_faz = OPECTL.op_faz;
// MH321800(S) Y.Tanizaki ICクレジット対応
	ushort	turi_wk = 0;
// MH321800(E) Y.Tanizaki ICクレジット対応

	switch( msg ){
		case KEY_TEN_F4:											// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
			if( OPECTL.op_faz == 2 ){
				break;
			}
			if( OPECTL.CAN_SW == 0 ){								// 再入力でない
				OPECTL.CAN_SW = 1;
				cn_stat( 2, 2 );									// 入金不可
				Lagtim( OPETCBNO, 2, 10*50 );						// ﾒｯｸ監視10s
				OPECTL.op_faz = 3;									// 中止中
				OPE_red = 5;										// 精算中止
				if( ryo_buf.tei_ryo ){								// 定期使用(定期券使用後に残額有り)?
					PassIdBackupTim = 0;							// 定期n分ﾁｪｯｸを解除する
				}
			}
			break;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
		case LCD_LCDBM_OPCD_PAY_STP:			// 精算中止要求受信時
		case LCD_LCDBM_OPCD_STA_NOT:			// 起動通知受信時
		case LCD_LCD_DISCONNECT:				// リセット通知/切断検知
			// invcrd( paymod )後に精算完了なら ---> op_faz <- 2で精算完了
			if( OPECTL.op_faz == 2 ){
				break;
			}
			// 精算未完了 ⇒ 精算完了発生
			else {
// MH810100(S) K.Onodera  2020/02/25 #3858 車番チケットレス(全額払出し後に預り証が発行される不具合修正)
				if( OPECTL.op_faz != 3 ){	// 精算中止中ではない
// MH810100(E) K.Onodera  2020/02/25 #3858 車番チケットレス(全額払出し後に預り証が発行される不具合修正)
					cn_stat( 2, 2 );								// 入金不可
					Lagtim( OPETCBNO, 2, 10*50 );					// ﾒｯｸ監視10s
					OPECTL.op_faz = 3;								// 中止中
					OPE_red = 5;									// 精算中止
// MH810100(S) K.Onodera  2020/02/25 #3858 車番チケットレス(全額払出し後に預り証が発行される不具合修正)
				}
// MH810100(E) K.Onodera  2020/02/25 #3858 車番チケットレス(全額払出し後に預り証が発行される不具合修正)
			}
			break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

		case NOTE_EVT:												// Note Reader event
			if( OPECTL.NT_QSIG == 1 ){
// MH321800(S) Y.Tanizaki ICクレジット対応
				if(isEC_USE()) {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//					if( !Suica_Rec.Data.BIT.PAY_CTRL &&							// 決済結果データを受信しておらず
//						Suica_Rec.Data.BIT.CTRL &&								// 決済リーダ有効で
//						in_credit_check() ){									// 残額がある場合
//						// 入金による受付禁止の場合は金額変更ビットをセットする
//						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// 受付禁止送信(金額変更)
//					}
					// 受付禁止(金額変更)はブランド選択データの変更判定に更新後のryo_buf.nyukinを使用するためcn_crdt()後に移動
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
				} else
// MH321800(E) Y.Tanizaki ICクレジット対応
				if( !Suica_Rec.Data.BIT.PAY_CTRL && !ryo_buf.nyukin &&	// 精算開始後最初の現金投入時は受付不可を送信する
					Suica_Rec.Data.BIT.CTRL && in_credit_check() ){		// Suica有効の時で残額がある場合
					Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
				}					
				ac_flg.cycl_fg = 10;								// 入金中
				cn_crdt();
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
				// 決済リーダ金額変更
				if( isEC_USE()						&&	// 決済リーダで
				    !Suica_Rec.Data.BIT.PAY_CTRL	&&	// 決済結果データを受信しておらず		// ★注意レビュー中
				    Suica_Rec.Data.BIT.CTRL   		&&	// 決済リーダ有効で
					(ryo_buf.nyukin < ryo_buf.dsp_ryo)) {	// 残額あり
				    if (EcCheckBrandSelectTable(TRUE)){			// ブランド選択データに変更がある(ブランド選択中でない場合は常にTRUE)
						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// 受付禁止送信(金額変更)
					}
				}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH321800(S) hosoda ICクレジット対応
				if ((Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)
				||	(Suica_Rec.Data.BIT.CTRL_MIRYO != 0)) {
				// 未了発生～確定までの入金は処理しない(OPECTL.op_faz:2～3の間なので)
				// →取消キー押下で返金する
					break;											// 未了確定後?
				}
// MH321800(E) hosoda ICクレジット対応
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算受付とのすれ違い入金時は入庫情報を受信するまで入金処理しない
					// 入庫情報受信後に再計算するため
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if( OPECTL.op_faz == 3 ){
					break;											// 中止?
				}
				else if( OPECTL.op_faz == 4 ){
					OPECTL.op_faz = 3;								// 精算中止処理を行う。
					break;
				}
				if( invcrd( paymod ) != 0 ){						// 精算完了?
					ac_flg.cycl_fg = 11;							// 精算完了
					if( OPECTL.op_faz != 2 ){						//
						svs_renzoku();								// ｻｰﾋﾞｽ券連続挿入
						cn_stat( 2, 2 );							// 入金不可
						Lagtim( OPETCBNO, 2, 10*50 ); 				// ﾒｯｸ監視10s
						OPECTL.op_faz = 2;
					}
				}
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
				else{												// 精算未完了
					// ryo_buf.zankinはinvcrd()で更新されるので
					// 受付許可送信可否判定はここで行う
					if( isEC_USE() ){								// 決済リーダ接続設定あり
						if(( !Suica_Rec.Data.BIT.PAY_CTRL ) &&		// 決済結果データを受信しておらず
						   ( !Suica_Rec.Data.BIT.CTRL )){			// 決済リーダ無効
							EcSendCtrlEnableData();					// 受付許可送信
						}
					}
				}
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
			}
			if( OPECTL.NT_QSIG == 5 ){								// 後続入金あり?
				cn_stat( 1, 1 );									// 紙幣収金
			}
			break;

		case COIN_EVT:												// Coin Mech event
// MH321800(S) Y.Tanizaki ICクレジット対応
//			if(( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 )){	// 入金ｱﾘ
			if(( OPECTL.CN_QSIG == 1 )||( OPECTL.CN_QSIG == 5 )||	// 入金ｱﾘ
			   ( OPECTL.credit_in_mony != 0 )){
				if(isEC_USE()) {
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//					if( !Suica_Rec.Data.BIT.PAY_CTRL &&							// 決済結果データを受信しておらず
//						Suica_Rec.Data.BIT.CTRL &&								// 決済リーダ有効で
//						in_credit_check() ){									// 残額がある場合
//						// 入金による受付禁止の場合は金額変更ビットをセットする
//						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// 受付禁止送信(金額変更)
//					}
					// 受付禁止(金額変更)はブランド選択データの変更判定に更新後のryo_buf.nyukinを使用するためcn_crdt()後に移動
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
				} else
// MH321800(E) Y.Tanizaki ICクレジット対応
				if( !Suica_Rec.Data.BIT.PAY_CTRL && !ryo_buf.nyukin &&	// 精算開始後最初の現金投入時は受付不可を送信する
					Suica_Rec.Data.BIT.CTRL && in_credit_check() ){		// Suica有効の時で残額がある場合
					Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
				}					
				ac_flg.cycl_fg = 10;								// 入金中
				cn_crdt();
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
				// 決済リーダ金額変更
				if( isEC_USE()						&&	// 決済リーダで
				    !Suica_Rec.Data.BIT.PAY_CTRL	&&	// 決済結果データを受信しておらず		// ★注意レビュー中
				    Suica_Rec.Data.BIT.CTRL   		&&	// 決済リーダ有効で
					(ryo_buf.nyukin < ryo_buf.dsp_ryo)) {	// 残額あり
				    if (EcCheckBrandSelectTable(TRUE)){			// ブランド選択データに変更がある(ブランド選択中でない場合は常にTRUE)
						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// 受付禁止送信(金額変更)
					}
				}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH321800(S) hosoda ICクレジット対応
				if ((Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)
				||	(Suica_Rec.Data.BIT.CTRL_MIRYO != 0)) {
				// 未了発生～確定までの入金は処理しない(OPECTL.op_faz:2～3の間なので)
				// →取消キー押下で返金する
// MH810105 MH321800(S) 現金投入と同時にカードタッチ→再タッチ待ち中のみなし決済受信で画面がロックする
//					break;											// 未了確定後?
					if( OPECTL.credit_in_mony == 0 ){
					// 現金入金と競合である場合は入金処理をするためbreakしない
						break;										// 未了確定後?
					}
// MH810105 MH321800(E) 現金投入と同時にカードタッチ→再タッチ待ち中のみなし決済受信で画面がロックする
				}
// MH321800(E) hosoda ICクレジット対応
// MH810104 GG119201(S) 「カード処理中です」表示ががちらつく
				if ((OPECTL.credit_in_mony == 1) && isEC_STS_CARD_PROC()) {
					// カード処理中とのすれ違い入金時は決済結果受信するまで入金処理しない
					// 決済結果受信後に再計算するため
					break;
				}
// MH810104 GG119201(E) 「カード処理中です」表示ががちらつく
				if( OPECTL.op_faz == 3 ){
					break;											// 中止?
				}
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if (OPECTL.f_rtm_remote_pay_flg != 0) {
					// 遠隔精算受付とのすれ違い入金時は入庫情報を受信するまで入金処理しない
					// 入庫情報受信後に再計算するため
					break;
				}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
				if( invcrd( paymod ) != 0 ){						// 精算終了?
					ac_flg.cycl_fg = 11;							// 精算完了
					if( OPECTL.op_faz != 2 ){						//
						svs_renzoku();								// ｻｰﾋﾞｽ券連続挿入
						cn_stat( 2, 2 );							// 入金不可
						Lagtim( OPETCBNO, 2, 10*50 );				// ﾒｯｸ監視10s
						OPECTL.op_faz = 2;							//
					}
				}else if( OPECTL.CN_QSIG == 5 ){ 					// Coin MAX ? NJ
					alm_chk( ALMMDL_SUB, ALARM_CNM_IN_AMOUNT_MAX, 2 );
					cn_stat( 5, 0 ); 								// Continue
				}
// MH810103 GG119202(S) クレジットカード精算限度額設定対応
				if( isEC_USE() ){									// 決済リーダ接続設定あり
					// ryo_buf.zankinはinvcrd()で更新されるので
					// 受付許可送信可否判定はここで行う
					if(( OPECTL.op_faz < 2 ) &&						// 精算未完了
					   ( !Suica_Rec.Data.BIT.PAY_CTRL ) &&			// 決済結果データを受信しておらず
					   ( !Suica_Rec.Data.BIT.CTRL )){				// 決済リーダ無効
						EcSendCtrlEnableData();						// 受付許可送信
					}
				}
// MH810103 GG119202(E) クレジットカード精算限度額設定対応
			}
			else if( OPECTL.CN_QSIG == 7 ){							// 払出可(入金不可状態)
				Lagcan( OPETCBNO, 2 );
// MH321800(S) T.Nagai ICクレジット対応
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				if (Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC) {	// カード処理中？
				if (isEC_STS_CARD_PROC()) {							// カード処理中？
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
					OPECTL.holdPayoutForEcStop = 1;					// 払出可イベントを保留する
					break;
				}
// MH321800(E) T.Nagai ICクレジット対応
				switch( OPECTL.op_faz ){
					case 3:											// 中止
// MH321800(S) T.Nagai ICクレジット対応
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						if (isEC_USE() &&
//							Suica_Rec.Data.BIT.CTRL &&				// 受付可で
//							!Suica_Rec.suica_err_event.BYTE) {		// エラーなし
						if (isEC_CTRL_ENABLE()) {					// 決済リーダが受付可
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							OPECTL.holdPayoutForEcStop = 1;			// 払出可イベントを保留する
							break;
						}
// MH321800(E) T.Nagai ICクレジット対応
						if( ryo_buf.nyukin ){						// 入金ｱﾘ
							ac_flg.cycl_fg = 21;					// 精算中止
							if( refund( modoshi() ) == 0 ){
								/*** ｺｲﾝ払出し ***/
								LedReq( CN_TRAYLED, LED_ON );		// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED点灯
// MH321800(S) Y.Tanizaki ICクレジット対応
//								Lagtim( OPETCBNO, 2, (ushort)((60*12)*50) );	// ﾒｯｸ監視12min
								// 払出金額からﾒｯｸ監視時間を算出（最大12min）
								if(((ryo_buf.nyukin / 10) + 5) > 720) {
									turi_wk = 720;
								} else {
									turi_wk = (ushort)((ryo_buf.nyukin / 10) + 5);
								}
								Lagtim( OPETCBNO, 2, (ushort)(turi_wk*50) );
// MH321800(E) Y.Tanizaki ICクレジット対応
							}else{
								Lagtim( OPETCBNO, 2, 10*50 );		// ﾒｯｸ監視10s
							}
							svs_renzoku();							// ｻｰﾋﾞｽ券連続挿入
						}else{
// MH810105(S) MH364301 QRコード決済対応（精算中止データ送信対応）
//							if( carduse() ){						// ｶｰﾄﾞ使用?
							if( carduse() ||						// ｶｰﾄﾞ使用?
								isEC_PAY_CANCEL() ){				// 決済リーダの決済精算中止？
// MH810105(E) MH364301 QRコード決済対応（精算中止データ送信対応）
								chu_isu();							// 中止
								svs_renzoku();						// ｻｰﾋﾞｽ券連続挿入
							}
// MH810100(S) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
							else{
								chu_isu();							// 中止
							}
// MH810100(E) 2020/07/20 車番チケットレス(仕様変更 #4541 RXMでセンター問合せ後に、NGとしたQRデータの取消データ（DC-NET）送信)
							Lagtim( OPETCBNO, 1, 1 );				// T1outから待機へ移行する
						}
						break;
					case 2:											// 完了
// MH321800(S) T.Nagai ICクレジット対応
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						if (isEC_USE() &&
//							Suica_Rec.Data.BIT.CTRL &&				// 受付可で
//							!Suica_Rec.suica_err_event.BYTE) {		// エラーなし
						if (isEC_CTRL_ENABLE()) {					// 決済リーダが受付可
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							OPECTL.holdPayoutForEcStop = 1;			// 払出可イベントを保留する
							break;
						}
// MH321800(E) T.Nagai ICクレジット対応
						ret = invcrd( paymod );						// 精算完了判別
						Op_StopModuleWait_sub(3);					// 精算完了（精算ユニット停止完了）時は 
						ret = 0;									// "ELE_EVT_STOP"メールで通知する
						break;
					case 8:												// 電子媒体停止処理中？
// MH321800(S) T.Nagai ICクレジット対応
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//						if (isEC_USE() &&
//							Suica_Rec.Data.BIT.CTRL &&				// 受付可で
//							!Suica_Rec.suica_err_event.BYTE) {		// エラーなし
						if (isEC_CTRL_ENABLE()) {					// 決済リーダが受付可
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
							OPECTL.holdPayoutForEcStop = 1;			// 払出可イベントを保留する
							break;
						}
// MH321800(E) T.Nagai ICクレジット対応
						if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// 精算完了時の電子決済終了待ち中？
							Op_StopModuleWait_sub(3);					// ｺｲﾝﾒｯｸ停止処理実施
							ret = 0;
						}
						break;
					default:
						break;
				}
			}
			else if( OPECTL.CN_QSIG == 2 ){ 						// 払出完了
				Lagcan( OPETCBNO, 2 );								// ﾒｯｸ監視ﾀｲﾏｰｷｬﾝｾﾙ
				Op_Cansel_Wait_sub( 2 );
				if(( SFV_DAT.reffal )||( ryo_buf.fusoku )){			//
					ryo_buf.fusoku += SFV_DAT.reffal;				// 支払い不足額
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					ret = 0;
				}
				ac_flg.cycl_fg = 23;								// 精算中止払い出し完了
				chu_isu();											// 中止集計
			}
			else if( OPECTL.CN_QSIG == 9 ){							// 払出ｴﾗｰ
				if( OPECTL.op_faz == 3 ){							// 中止?
					ac_flg.cycl_fg = 23;							// 精算中止払い出し完了
					refalt();	 									// 不足分算出
					ryo_buf.fusoku += SFV_DAT.reffal;				// 支払い不足額
					chu_isu();										// 中止集計
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					set_escrowtimer();
				}
			}
			break;

		case ARC_CR_R_EVT:											// ｶｰﾄﾞIN
		case IBK_MIF_A2_OK_EVT:										// Mifareﾃﾞｰﾀ読出し完了
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//		case CRE_EVT_04_OK:											// クレジット与信問合せ結果ＯＫ
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// MH321800(S) hosoda ICクレジット対応
		case EC_EVT_CRE_PAID:										// 決済リーダでのクレジット決済OK
// MH321800(E) hosoda ICクレジット対応
			if( paymod == 0 ){
			}else{
				if( ryo_buf.dsp_ryo > ryo_buf.mis_ryo ){
					ryo_buf.dsp_ryo -= ryo_buf.mis_ryo;
				}else{
					ryo_buf.dsp_ryo = 0;
				}
			}
			if( invcrd( paymod ) != 0 ){							// 精算終了? NJ
				if( paymod ){										// 修正精算?
					ac_flg.cycl_fg = 71;							// 修正精算完了
				}else{
					ac_flg.cycl_fg = 11;							// 精算完了
				}
				if( OPECTL.op_faz != 2 ){
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//					if( msg == CRE_EVT_04_OK ) {					// クレジット精算（与信問い合せOK）
//						cre_uriage.UmuFlag	= ON;					// 売上依頼データ送信ﾌﾗｸﾞON（06受信でOFF）
//					}
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// MH321800(S) Y.Tanizaki ICクレジット対応
					if(msg == EC_EVT_CRE_PAID) {
						if( OPECTL.op_faz == 8 && 
							CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ ){	// STOP_REASONがカード挿入の場合はｶｰﾄﾞの吐き戻しを行う
							opr_snd( 13 );							// 前排出(プリンタブロックからでも吐き出す)
						}
						Op_StopModuleWait_sub(4);					// 決済リーダから結果をもらっているため停止済みにする
						OPECTL.credit_in_mony = 0;
					}
// MH321800(E) Y.Tanizaki ICクレジット対応
					OPE_red = 4;									// 精算完了
					cn_stat( 2, 2 );								// 入金不可
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					OPECTL.op_faz = 2;
					if( msg == ARC_CR_R_EVT ){						// ｶｰﾄﾞIN
						if( MAGred[MAG_ID_CODE] == 0x2d ){					// ｻｰﾋﾞｽ券? NJ
							rd_shutter();							// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
						}
						if( (msg == ARC_CR_R_EVT) &&					// カードイベントのとき
							(MAGred[MAG_ID_CODE] == 0x0e) ||						// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ
							(MAGred[MAG_ID_CODE] == 0x2c) ){						// 回数券
							OPECTL.PriUsed = 1;
						}
					}
				}
			}else{
				if( msg == ARC_CR_R_EVT && MAGred[MAG_ID_CODE] == 0x0e ){		// ｶｰﾄﾞIN & ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ? NJ
					OPECTL.PriUsed = 2;
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[68] );				// "        （残額            円）"
					opedpl( 7, 16, OPECTL.LastUsePCardBal, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
				}
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
// ブランド数に変化があった場合はブランド選択を行う
// 決済リーダ金額変更
				if( isEC_USE()	&&						// 決済リーダで
					Suica_Rec.Data.BIT.CTRL ) {  		// 受付可（リーダー停止していないとき）
				    if (EcCheckBrandSelectTable(FALSE)){			// ブランド選択データに変更がある(ブランド選択中でない場合は常にTRUE)
						Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// 受付禁止送信(金額変更)
						break;
				    }
				}
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
				Op_StopModuleWait_sub( 2 );
			}
			break;
		case SUICA_EVT:													// Suica(Sx-10)からの受信ﾃﾞｰﾀ
// MH810105(S) MH364301 QRコード決済対応
		case EC_EVT_QR_PAID:											// 決済リーダでのQRコード決済OK
// MH810105(E) MH364301 QRコード決済対応
			if( PayData.Electron_data.Suica.pay_ryo ) {					// Suica決済結果ﾃﾞｰﾀ受信 
				if( paymod != 0 ){											// 修正精算時は
					if( ryo_buf.dsp_ryo > ryo_buf.mis_ryo ){				// 正＞誤であれば（不足あり）
						ryo_buf.dsp_ryo -= ryo_buf.mis_ryo;					// 誤で支払った分を引く
					}else{													// 正<or=誤であれば（不足なし）
						ryo_buf.dsp_ryo = 0;								// 0をset
					}
				}
				if( invcrd( paymod ) != 0 ){								// 精算終了? NJ
					if( OPECTL.op_faz == 8 && 
						CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ ){	// STOP_REASONがカード挿入の場合はｶｰﾄﾞの吐き戻しを行う
							opr_snd( 13 );											// 前排出(プリンタブロックからでも吐き出す)
					}
					Op_StopModuleWait_sub(4);									
// MH810103 GG119202(S) 現金投入と同時に交通系のカード処理中受信したとき、精算完了後、現金が戻らない。
					if (isEC_USE() ){
						OPECTL.credit_in_mony = 0;
					}
// MH810103 GG119202(E) 現金投入と同時に交通系のカード処理中受信したとき、精算完了後、現金が戻らない。
					ac_flg.cycl_fg = 11;									// 精算完了
					OPE_red = 4;											// 精算完了
					if( OPECTL.PriUsed != 1 && RD_Credit_kep )
					svs_renzoku();											// ｻｰﾋﾞｽ券連続挿入
					cn_stat( 2, 2 );										// 入金不可
					Lagtim( OPETCBNO, 1, 10*50 );							// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					OPECTL.op_faz = 2;										// 精算完了
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					Edy_StopAndLedOff();							// ｶｰﾄﾞ検知停止＆UI LED消灯指示送信
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
				}
				else{
					if( OPECTL.Ope_mod != 13 ){								// 修正精算時は残高表示はしない
						grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[68] );						// "      （残額            円）  "
						opedpl( 7, 14, (ulong)e_zandaka, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
					}
				}
				Suica_Rec.Data.BIT.PAY_CTRL = 1;
				if( OPECTL.op_faz == 9 ){								// 精算と取り消しﾎﾞﾀﾝの同時押しで取り消しﾎﾞﾀﾝが保留中の場合
					OPECTL.op_faz = 1;									// 元のﾌｪｰｽﾞに戻しておく
				}
			}
			break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			case IBK_EDY_RCV:											// Edy（決済結果：完了ﾃﾞｰﾀ受信のみ呼出される）
//				if(Edy_Rec.rcv_kind == R_SUBTRACTION) {	// Suica決済結果ﾃﾞｰﾀ受信 
//					if( paymod != 0 ){											// 修正精算時は
//						if( ryo_buf.dsp_ryo > ryo_buf.mis_ryo ){				// 正＞誤であれば（不足あり）
//							ryo_buf.dsp_ryo -= ryo_buf.mis_ryo;					// 誤で支払った分を引く
//						}else{													// 正<or=誤であれば（不足なし）
//							ryo_buf.dsp_ryo = 0;								// 0をset
//						}
//					}
//					if( invcrd( paymod ) != 0 ){								// 精算終了? NJ
//						if( OPECTL.op_faz == 8 && 
//							CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ ){	// STOP_REASONがカード挿入の場合はｶｰﾄﾞの吐き戻しを行う
//								opr_snd( 13 );											// 前排出
//						}
//						Op_StopModuleWait_sub(5);								// Edy決済時(決済後は必ずEdyも停止状態)
//						ac_flg.cycl_fg = 11;									// 精算完了
//						OPE_red = 4;											// 精算完了
//						if( OPECTL.PriUsed != 1 && RD_Credit_kep )
//						svs_renzoku();											// ｻｰﾋﾞｽ券連続挿入
//						cn_stat( 2, 2 );										// 入金不可
//						Lagtim( OPETCBNO, 1, 10*50 );							// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//						OPECTL.op_faz = 2;										// 精算完了
//					}
//					Edy_SndData04();									// EMｽﾃｰﾀｽ読出しのために先行してｶｰﾄﾞ検知終了ｺﾏﾝﾄﾞ送信
//					Edy_SndData11();									// EMｽﾃｰﾀｽ読出し指示送信
//					if( !Ex_portFlag[EXPORT_R_PEND] && OPECTL.RECI_SW == 0 && prm_get(COM_PRM, S_SCA, 54, 1, 1) == 1 )
//						OPECTL.RECI_SW = 1;								// 券なし（紛失・手動）時は、ﾚｼｰﾄ印字
//				}
//			break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

		case	IFMPAY_GENGAKU:
		case	IFMPAY_FURIKAE:
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
		case OPE_REQ_REMOTE_CALC_FEE:
		case OPE_REQ_REMOTE_CALC_TIME:
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応_for_LCD_IN_CAR_INFO_op_mod00))
		case OPE_REQ_LCD_CALC_IN_TIME:			// 車番チケットレス精算(入庫時刻指定)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応_for_LCD_IN_CAR_INFO_op_mod00))

// MH322914(S) K.Onodera 2016/09/05 AI-V対応：振替精算
		case OPE_REQ_FURIKAE_GO:
// MH322914(E) K.Onodera 2016/09/05 AI-V対応：振替精算
			if( invcrd( paymod ) != 0 ){						// 精算完了?
				ac_flg.cycl_fg = 11;							// 精算完了
				if( OPECTL.op_faz != 2 ){
					svs_renzoku();								// ｻｰﾋﾞｽ券連続挿入
					cn_stat( 2, 2 );							// 入金不可
					Lagtim( OPETCBNO, 2, 10*50 ); 				// ﾒｯｸ監視10s
					OPECTL.op_faz = 2;
				}
			}
			else{
// MH321800(S) D.Inaba ICクレジット対応
				// 決済リーダは制御データ(利用不可)を送信する
				if( isEC_USE() ){
// MH810100(S) 遠隔精算でクレジット精算ができない
                    // 選択商品データ送信済みであれば、受付禁止を送信する
                    if (Suica_Rec.Data.BIT.ADJUSTOR_NOW) {
// MH810100(E) 遠隔精算でクレジット精算ができない
						Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );
// MH810100(S) 遠隔精算でクレジット精算ができない
                    }
// MH810100(E) 遠隔精算でクレジット精算ができない
				} else {
// MH321800(E) D.Inaba ICクレジット対応
				if( !ryo_buf.nyukin ){
					Suica_Ctrl( S_SELECT_DATA, 0 );							// 駐車料金を商品選択ﾃﾞｰﾀとして送信する
				}
// MH321800(S) D.Inaba ICクレジット対応
				}
// MH321800(E) D.Inaba ICクレジット対応
			}
			break;
		default:
			break;
	}

	if( OPECTL.op_faz == 2 && (OpeImfStatus != OPEIMF_STS_GGK_DONE && OpeImfStatus != OPEIMF_STS_FRK_DONE ) ){
		switch(ope_imf_GetStatus()) {
		case	1:
			ope_imf_Answer(1);			// 減額精算完了
			break;
		case	2:
			if( is_paid_remote(&PayData) < 0 ){
			// 振替精算未実施（定期利用指示されたが現金精算）ならここで振替額のみ計上
				ope_ifm_FurikaeCalc( 0 );
			}
			ope_Furikae_stop(vl_frs.lockno, 1);			// 振替元車室復元実行
			ope_imf_Answer(1);			// 振替精算成功
			break;
		default:
			break;
		}
	}
// MH322914(S) K.Onodera 2016/09/05 AI-V対応：振替精算
	if( OPECTL.op_faz == 2 && PiP_GetFurikaeSts() ){
		if (is_ParkingWebFurikae(&PayData) < 0) {
			ope_PiP_GetFurikaeGaku( &g_PipCtrl.stFurikaeInfo );
			vl_now = V_FRK;						// 振替精算
			ryo_cal( 3, OPECTL.Pr_LokNo );		// サービス券として計算
		}
		ope_Furikae_stop( vl_frs.lockno, 1 );			// 振替元車室復元実行
		PiP_FurikaeEnd();
	}
// MH322914(E) K.Onodera 2016/09/05 AI-V対応：振替精算

	if( OPECTL.op_faz == 2 ){ 		// 精算完了時
		// 精算完了時に他の要因による電子媒体停止中の場合は、精算完了の停止要因を優先させる
		// ここでは、INITIALIZEをクリアし、Op_StopModuleWait()内で精算完了要因のフラグを上書きする
		if( CCT_Cansel_Status.BIT.STOP_REASON != REASON_PAY_END ){
			CCT_Cansel_Status.BIT.INITIALIZE = 0;
		}
		Op_StopModuleWait( REASON_PAY_END );		// 電子媒体停止処理
		if(OPECTL.ChkPassSyu)
			blink_end();												// 点滅終了
	}
	if( w_op_faz == 8 && ( w_op_faz != OPECTL.op_faz && StopStatusGet( 0 ) != 0x07 )){	// 電子媒体停止待ち合わせ中でフェーズが変わるような場合
		OPECTL.op_faz = w_op_faz;						// 再度、電子媒体停止待ち合わせフェーズに戻す
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算完了                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod03( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = 正常完了                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod03( void )
{
	short	ret;
	short	ryo_en;
	short	safe_que = 0;
	ushort	msg = 0;
	short	ret_bak = 0;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//	ushort	wk_MsgNo;
	uchar	Discon_flg = 0;
	ushort	res_wait_time;
	uchar	idle_req_flg = 0;	// 0=精算案内中 1=精算案内終了応答待ち 2=精算案内終了応答受信済み
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	uchar	buf_size;
	uchar	ryo_isu_zumi=0;
	ulong	turi_wk;
// MH810105(S) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
	uchar	print_wait_return =0;
// MH810105(E) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
// MH321800(S) D.Inaba ICクレジット対応
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	uchar	announce_flag = 0;			//  ｶｰﾄﾞ抜き取りｱﾅｳﾝｽ中フラグ
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	ushort	ec_removal_wait_time;
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(E) D.Inaba ICクレジット対応
//// MH322914(S) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//	short	nDelayAnnounce = 0;
//// MH322914(E) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
	uchar	dummyNotce_Wait = 0;		// 印字処理完了待ちフラグ
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
	Cnm_End_PayTimOut		= 0;
// MH810105(S) MH364301 インボイス対応
	OPECTL.f_RctErrDisp = 0;
// MH810105(E) MH364301 インボイス対応
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	ryo_stock = 0;						// ストックなし
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// MH321800(S) D.Inaba ICクレジット対応
	ec_removal_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 13, 3, 1);
	if(ec_removal_wait_time == 0) {
		ec_removal_wait_time = 360;
	} else if( ec_removal_wait_time < 30 ){
		ec_removal_wait_time = 30;
	}
	ec_removal_wait_time *= 50;
// MH321800(E) D.Inaba ICクレジット対応
// MH810100(S) K.Onodera 2020/02/27 車番チケットレス（精算終了処理修正）
	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// データ受信監視タイマ
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// 範囲外は5秒
	}
// MH810100(E) K.Onodera 2020/02/27 車番チケットレス（精算終了処理修正）
// GG129000(S) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
	OPECTL.f_req_paystop = 0xff;
// GG129000(E) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52

	// クレジットカード売上げ送信

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( edy_dsp.BIT.edy_Miryo_Loss || edy_dsp.BIT.edy_dsp_change || dsp_change || Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
	if( dsp_change || Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		op_SuicaFusokuOff();
	}

	if( Suica_Rec.Data.BIT.OPE_CTRL == 1 ){							// 最後にOpeが送信したデータが受付可の場合は
		// 本条件は、精算なし出庫及び、０円精算時には精算処理を経ずに精算完了ﾌｪｰｽﾞに移行するため、
		// ここでSuicaの停止を行う
		Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
	}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( PayData.Electron_data.Edy.e_pay_kind != EDY_USED ){
//		Edy_StopAndLedOff();
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	memset( DspChangeTime, 0, sizeof( DspChangeTime ));				// ﾜｰｸｴﾘｱの初期化
// MH810103(s) 電子マネー対応 精算状態通知をOpe_modにより切り替える
	// 精算状態通知(領収証が使えるようになっている可能性があるため、念のため)
	dspCyclicErrMsgRewrite();
// MH810103(e) 電子マネー対応 精算状態通知をOpe_modにより切り替える

// MH810100(S) K.Onodera 2020/01/06 車番チケットレス（QR確定・取消データ対応）
//	IoLog_wrrite(IOLOG_EVNT_AJAST_FIN, (ushort)LockInfo[OPECTL.Pr_LokNo - 1].posi, 0, 0);
//	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出
//
//	if( OPECTL.PriUsed != 1 && RD_Credit_kep == 0){
//		rd_shutter();												// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
//	}
// MH810100(E) K.Onodera 2020/01/06 車番チケットレス（QR確定・取消データ対応）

// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//	if( !OPECTL.multi_lk ){											// ﾏﾙﾁ精算問合せ中以外?
	if( !OPECTL.multi_lk && (!g_PipCtrl.stRemoteFee.Type || g_PipCtrl.stRemoteFee.ReqKind != UNI_KIND_REMOTE_FEE) ){	// ﾏﾙﾁ精算問合せ中でも、後日精算でもない？
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
//		queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof( OPECTL.Pr_LokNo ), &OPECTL.Pr_LokNo );	// 精算完了ｷｭｰｾｯﾄ(ﾌﾗｯﾊﾟｰ下降)
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_year = car_ot_f.year;// 精算時刻	（年）をｾｯﾄ
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_mont = car_ot_f.mon;	// 			（月）をｾｯﾄ
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_date = car_ot_f.day;	// 			（日）をｾｯﾄ
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_hour = car_ot_f.hour;// 			（時）をｾｯﾄ
		FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].s_minu = car_ot_f.min;	// 			（分）をｾｯﾄ
// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//		LongTermParkingRel( OPECTL.Pr_LokNo );						// 長期駐車解除(精算)
		LongTermParkingRel( OPECTL.Pr_LokNo, LONGPARK_LOG_PAY ,&FLAPDT.flp_data[OPECTL.Pr_LokNo - 1]);	// 長期駐車解除(精算)
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応

// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
//		if( OPECTL.Pay_mod == 2 ){
//			queset( FLPTCBNO, FLAP_UP_SND_SS, sizeof(OPECTL.MPr_LokNo), &OPECTL.MPr_LokNo );		// 修正元ﾌﾗｯﾌﾟ上昇
//		}
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// 不具合修正(S) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
//		// 振替精算？
//		if( g_PipCtrl.stFurikaeInfo.ReqKind == UNI_KIND_FURIKAE ){
//			// 振替先未下降？
//			if( FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].nstat.bits.b01 == 1 ){
//				FurikaeDestFlapNo = OPECTL.Pr_LokNo;
//			}
//		}
//// 不具合修正(E) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	}
	PayEndSig();													// 精算完了信号出力
// GG129000(S) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5684：領収証必ず発行設定で領収証発行した場合もNT-NET精算データの領収証発行有無が0＝領収証なしになっている）
	QRIssueChk(0);
// GG129000(E) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5684：領収証必ず発行設定で領収証発行した場合もNT-NET精算データの領収証発行有無が0＝領収証なしになっている）

	ryo_en = 0;														// 領ﾎﾞﾀﾝ不可
	switch( OPECTL.Fin_mod ){
		case 1:														// 釣り無し?
		default:
			ac_flg.cycl_fg = 13;									// 払い出し完了(仮)
			cn_stat( 6, 0 );										// 投入金ｸﾘｱ(ｴｽｸﾛｺｲﾝを落とす)
			ryo_en = 1;												// 領ﾎﾞﾀﾝ可
			if( OPECTL.RECI_SW == 1 ){								// 領収書ﾎﾞﾀﾝ使用?
				ryo_isu( 0 );										// 領収書(釣銭不足無し)発行
// MH810105(S) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
				print_wait_return = 1;								// 領収証発行あり
				if (IS_INVOICE &&
					OPECTL.f_DelayRyoIsu != 2) {
				}else{
					ryo_isu_zumi = 1;								// 領収証発行済み
				}
// MH810105(E) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
// GG129000(S) R.Endo 2023/02/22 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
				// 領収証発行ボタン押下後の領収証印字不可で領収証発行失敗表示するため
				// 領収証印字不可時のレシート印字失敗判定をここでも行う
				op_rct_failchk();
// GG129000(E) R.Endo 2023/02/22 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
			}
// MH810105(S) MH364301 インボイス対応（精算完了集計のタイミング変更）
//			if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )			// 精算完了通信電文送信タイミング=精算完了直後
			// ゲート式と同様に精算完了集計のタイミングを早める
// MH810105(E) MH364301 インボイス対応（精算完了集計のタイミング変更）
			{
				if( !OPECTL.f_KanSyuu_Cmp ){						// 完了集計未実施
					PayData_set( 0, 0 );							// 1精算情報ｾｯﾄ
					kan_syuu();										// 精算完了集計
				}
			}
			break;

		case 2:														// 釣り有り?
			turi_wk = 0;
			if( ryo_buf.turisen != 0 ){
				turi_wk = ryo_buf.turisen;
			}else if( ryo_buf. mis_tyu != 0 ){
				turi_wk = ryo_buf.mis_tyu;
			}
// 仕様変更(S) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
			if( ryo_buf.kabarai != 0 ){
				turi_wk += ryo_buf.kabarai;
			}
// 仕様変更(E) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
			if( refund( (long)turi_wk ) != 0 ){				// 払出起動
				/*** 払出NG ***/
				ac_flg.cycl_fg = 13;								// 払い出し完了(仮)
				ryo_en = 1;											// 領ﾎﾞﾀﾝ可
				ryo_buf.fusoku = SFV_DAT.reffal;					// 支払い不足額
				set_escrowtimer();
				Print_Condition_select();							// 領収書(釣銭不足有り) or 預り証発行
// MH810105(S) MH364301 インボイス対応（精算完了集計のタイミング変更）
//				if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )		// 精算完了通信電文送信タイミング=精算完了直後
				// ゲート式と同様に精算完了集計のタイミングを早める
// MH810105(E) MH364301 インボイス対応（精算完了集計のタイミング変更）
				{
					if( !OPECTL.f_KanSyuu_Cmp ){					// 完了集計未実施
						PayData_set( 0, 0 );						// 1精算情報ｾｯﾄ
						kan_syuu();									// 精算完了集計
					}
				}
			}else{
				if( (CN_refund & 0x01) == 0 ){
					// ｺｲﾝの払い出しが”なし”のとき
					cn_stat( 6, 0 );									// 投入金ｸﾘｱ(ｴｽｸﾛｺｲﾝを落とす)
				}
				else{												// 払い出しある
					Lagtim( OPETCBNO, 20, (ushort)(((turi_wk/10)+5)*50));		// ｺﾝﾒｯｸ払い出し完了待ちﾀｲﾏｰSTART
					Cnm_End_PayTimOut = 1;							// 払い出し監視ｽﾀｰﾄ
				}
			}
			LedReq( CN_TRAYLED, LED_ON );							// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED点灯
			break;

		case 3:														// ﾒｯｸｴﾗｰ(釣り有り)?
			ac_flg.cycl_fg = 13;									// 払い出し完了(仮)
			ryo_en = 1;												// 領ﾎﾞﾀﾝ可
			ryo_buf.fusoku = SFV_DAT.reffal;						// 支払い不足額
			set_escrowtimer();
			Print_Condition_select();								// 領収書(釣銭不足有り) or 預り証発行
// MH810105(S) MH364301 インボイス対応（精算完了集計のタイミング変更）
//			if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )			// 精算完了通信電文送信タイミング=精算完了直後
			// ゲート式と同様に精算完了集計のタイミングを早める
// MH810105(E) MH364301 インボイス対応（精算完了集計のタイミング変更）
			{
				if( !OPECTL.f_KanSyuu_Cmp ){						// 完了集計未実施
					PayData_set( 0, 0 );							// 1精算情報ｾｯﾄ
					kan_syuu();										// 精算完了集計
				}
			}
			break;
	}
// GG129000(S) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
//// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	// レーンモニタデータ登録
//	SetLaneFreeNum(0);
//// GG129000(S) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.54)（GM803000流用）
//	SetLaneFeeKind(0);								// 料金種別クリア
//// GG129000(E) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.54)（GM803000流用）
//	ope_MakeLaneLog(LM_PAY_CMP);
//// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129000(E) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
// GG129000(S) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5684：領収証必ず発行設定で領収証発行した場合もNT-NET精算データの領収証発行有無が0＝領収証なしになっている）
//// GG129000(S) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）
//	QRIssueChk(0);
//// GG129000(E) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129000(E) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5684：領収証必ず発行設定で領収証発行した場合もNT-NET精算データの領収証発行有無が0＝領収証なしになっている）
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//	DspChangeTime[0] = (ushort)(prm_get(COM_PRM, S_DSP, 30, 1, 1)); // 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ値取得
//	if(DspChangeTime[0] <= 0) {
//		DspChangeTime[0] = 2;
//	}
//// MH321800(S) G.So ICクレジット対応
//	if (isEC_USE() != 0) {
//		Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
//	} else {
//// MH321800(E) G.So ICクレジット対応
//		mode_Lagtim10 = 0;											// Lagtimer10の使用状態 0:利用可能媒体ｻｲｸﾘｯｸ表示
//		suica_fusiku_flg = 0;										// 利用可能媒体ｻｲｸﾘｯｸ表示有効
//		Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );		// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTART
//// MH321800(S) G.So ICクレジット対応
//	}
//// MH321800(E) G.So ICクレジット対応
//	OpeLcd( 5 );													// 精算完了表示

// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
//	// 精算残高変化通知の送信処理
//	lcdbm_pay_rem_chg(1);	// 1=精算完了
	if ( !cmp_send ) {			// 残高変化通知(精算完了)未送信
		lcdbm_pay_rem_chg(1);	// 残高変化通知(精算完了)
		cmp_send = 0;
	}
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

// GG129000(S) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
	// レーンモニタデータ登録
	SetLaneFreeNum(0);
	ope_MakeLaneLog(LM_PAY_CMP);
	SetLaneFeeKind(0);								// 料金種別クリア
// GG129000(E) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）

	cn_stat( 4, 1 );												// 紙幣取込

	PayDataErrDisp();
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH322914(S) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//	if( (cr_dat_n == RID_SAV) && ( 1 == IsMagReaderRunning() ) ){	// サービス券使用後でリーダー処理中？
//		nDelayAnnounce = 1;
//	}
//// MH321800(S) Y.Tanizaki ICクレジット対応
//	else if( Suica_Rec.Data.BIT.CTRL_CARD ) {						// 決済リーダ接続でカード差込状態あり
//		// カード抜き取り待ちタイマー起動
//		Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );
//		announce_flag = 1;
//		// 「カードをお取り下さい」表示とアナウンスをする
//		ec_MessageAnaOnOff( 1, 3 );
//	}
//// MH321800(E) Y.Tanizaki ICクレジット対応
//	else{
//// MH322914(E) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//	if( !OPECTL.multi_lk ){											// ﾏﾙﾁ精算問合せ中以外?
//		ope_anm( AVM_BGM );
//		if( OPECTL.Pr_LokNo >= LOCK_START_NO ){
//			ope_anm( AVM_KANRYO );									// 精算完了時ｱﾅｳﾝｽ(ロック)
//		}else{
//			ope_anm( AVM_KANRYO2 );									// 精算完了時ｱﾅｳﾝｽ(フラップ)
//		}
//	}
//	else{	// マルチ精算時はロック式の音声を流す（defaultでは「ありがとうございました」）
//		ope_anm( AVM_KANRYO );										// 精算完了時ｱﾅｳﾝｽ(ロック)
//	}
//// MH322914(S) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//	}
//// MH322914(E) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
	// 決済リーダ接続でカード差込状態あり
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if( Suica_Rec.Data.BIT.CTRL_CARD ) {						// 決済リーダ接続でカード差込状態あり
	if( isEC_CARD_INSERT() ) {									// 決済リーダ接続でカード差込状態あり
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		// カード抜き取り待ちタイマー起動
		Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );
	}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

// MH321800(S) D.Inaba ICクレジット対応
	// 決済リーダ接続設定でない||カードを抜き取っていたらﾀｲﾏｰ1(10s)起動
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if( !isEC_USE() || !Suica_Rec.Data.BIT.CTRL_CARD ){
	if( !isEC_CARD_INSERT() ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
// MH321800(E) D.Inaba ICクレジット対応
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
//	Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	if( r_zero_call ){
		Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );		// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)：短縮用
	}else{
		Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );		// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	}
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
// MH321800(S) D.Inaba ICクレジット対応
	}
// MH321800(E) D.Inaba ICクレジット対応
	Lagcan( OPETCBNO, 2 );											// Timer Cancel
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
	if( OPECTL.init_sts != 1 ){	// 初期化未完了？(op_mod02()で起動待機状態遷移イベント発生？)
		Discon_flg = 1;
	}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
// MH810105(S) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
	if( OPECTL.RECI_SW == 1 && print_wait_return == 0){		// 領収書ﾎﾞﾀﾝ押下済み？
		print_wait_return = 1;								// 領収証発行あり
		OPECTL.RECI_SW = 0;								// いったん戻してメッセージ通知
		queset( OPETCBNO, OPE_DELAY_RYO_PRINT, 0, NULL );
	}
// MH810105(E) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
// MH810100(S) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
	if( OPECTL.chg_idle_disp_rec ){	// 待機画面通知受信済み
		queset( OPETCBNO, LCD_CHG_IDLE_DISP, 0, NULL );
	}
// MH810100(E) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case COIN_EVT:											// Coin Mech event
				ryo_en = 1;											// 領ﾎﾞﾀﾝ可
				if( OPECTL.CN_QSIG == 2 || OPECTL.CN_QSIG == 9 ){
					Lagcan( OPETCBNO, 20 );
					Cnm_End_PayTimOut = 0;
				}
				if( OPECTL.CN_QSIG == 2 ){							// 払出完了
					ac_flg.cycl_fg = 13;							// 払い出し完了
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );		// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					if( r_zero_call ){
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)：短縮用
					}else{
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
					if( SFV_DAT.reffal ){
						ryo_buf.fusoku += SFV_DAT.reffal;			// 支払い不足額
						Print_Condition_select();					// 領収書(釣銭不足有り) or 預り証発行
					}
					else if( OPECTL.RECI_SW ){						// 領収書ﾎﾞﾀﾝ状態
						if( !ryo_isu_zumi ){
						ryo_isu( 0 );								// 領収書(釣銭不足無し)発行
					    OPECTL.RECI_SW = 1;							// 領収書ﾎﾞﾀﾝ使用
// MH810105(S) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
						print_wait_return = 1;								// 領収証発行あり
						if (IS_INVOICE &&
							OPECTL.f_DelayRyoIsu != 2) {
						}else{
							ryo_isu_zumi = 1;								// 領収証発行済み
						}
// MH810105(E) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
						}
					}
// MH810105(S) MH364301 インボイス対応（精算完了集計のタイミング変更）
//					if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )	// 精算完了通信電文送信タイミング=精算完了直後
					// ゲート式と同様に精算完了集計のタイミングを早める
// MH810105(E) MH364301 インボイス対応（精算完了集計のタイミング変更）
					{
						if( !OPECTL.f_KanSyuu_Cmp ){				// 完了集計未実施
							PayData_set( 0, 0 );					// 1精算情報ｾｯﾄ
							kan_syuu();								// 精算完了集計
						}
					}
				}
				else if( OPECTL.CN_QSIG == 9 ){						// 払出ｴﾗｰ
					ac_flg.cycl_fg = 13;							// 払い出し完了(仮)
					refalt();										// 不足分算出
					ryo_buf.fusoku += SFV_DAT.reffal;				// 支払い不足額
					Print_Condition_select();						// 領収書(釣銭不足有り) or 預り証発行
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					if( r_zero_call ){
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)：短縮用
					}else{
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
					set_escrowtimer();
// MH810105(S) MH364301 インボイス対応（精算完了集計のタイミング変更）
//					if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )	// 精算完了通信電文送信タイミング=精算完了直後
					// ゲート式と同様に精算完了集計のタイミングを早める
// MH810105(E) MH364301 インボイス対応（精算完了集計のタイミング変更）
					{
						if( !OPECTL.f_KanSyuu_Cmp ){				// 完了集計未実施
							PayData_set( 0, 0 );					// 1精算情報ｾｯﾄ
							kan_syuu();								// 精算完了集計
						}
					}
				}
// MH810100(S) K.Onodera 2020/02/27 車番チケットレス（精算終了処理修正）
//				if( ret_bak ) ret = ret_bak;
// MH810100(S) K.Onodera 2020/03/02 #3964 領収証を印字しない
//				ret = 10;
				if( ret_bak ){
					ret = ret_bak;
				}
// MH810100(E) K.Onodera 2020/03/02 #3964 領収証を印字しない
				// 切断していたら、応答を受信したことにする
				if( Discon_flg ){
					idle_req_flg = 2;
				}
// MH810100(E) K.Onodera 2020/02/27 車番チケットレス（精算終了処理修正）
				break;

// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(不要処理削除)
//			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
//			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
//			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
//			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
//			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
//			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
//			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
//			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
//			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
//			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
//// MH322914(S) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//				if( 1 == IsMagReaderRunning() ){					// リーダー処理中？
//					break;
//				}
//// MH322914(E) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//				if( ryo_en ){
//					Key_Event_Set( msg );
//				}
//			case KEY_TEN_CL:										// ﾃﾝｷｰ[区画] ｲﾍﾞﾝﾄ
//// MH322914(S) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//				if( 1 == IsMagReaderRunning() ){					// リーダー処理中？
//					break;
//				}
//// MH322914(E) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//				BUZPI();
//// MH321800(S) D.Inaba ICクレジット対応
//				// カード抜き取りアナウンス中は[KEY_TEN]イベントを発生させない
//				if( announce_flag == 1 ){
//					break;
//				}
//// MH321800(E) D.Inaba ICクレジット対応
//				// NOTE:キーを押下時点で音声を止め、精算データ7件ごとにFROMにライトアクセスするのでその時間を早めに確保し精算動作の遅延を軽減する
//				ope_anm( AVM_STOP );								// 放送停止ｱﾅｳﾝｽ
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(不要処理削除)
			case TIMEOUT1:											// Time Over ?
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) D.Inaba ICクレジット対応
//				// カード抜き取りアナウンス中は[TIMEOUT1]イベントを発生させない
//				if( announce_flag == 1 ){
//					break;
//				}
//// MH321800(E) D.Inaba ICクレジット対応
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
				if( ryo_en == 0 ){
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					if( r_zero_call ){
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );		// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)：短縮用
					}else{
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );		// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
					break;
				}
				if( SFV_DAT.safe_cal_do != 0 ){						// 金庫計算未終了?
					if( msg == TIMEOUT1 ){
						safe_que++;
						if( safe_que < 10 ){
							Lagtim( OPETCBNO, 1, 1*50 );				// ﾀｲﾏｰ1(1s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
							break;
						}
						ex_errlg( ERRMDL_MAIN, ERR_MAIN_NOSAFECLOPERATE, 2, 0 );	// 金庫枚数算出なしに抜けた
					}else{
						break;
					}
				}
				ret = 10;
				break;

// MH810105(S) MH364301 インボイス対応
			case OPE_DELAY_RYO_PRINT:								// op_jnl_failchk後の領収証ｲﾍﾞﾝﾄ
				goto _LCDBM_OPCD_RCT_ISU;	// 操作通知の領収証発行に飛ばす
				break;
// MH810105(E) MH364301 インボイス対応
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
//				if( ReceiptChk() == 1 || Ope_isPrinterReady() == 0){	// ﾚｼｰﾄ発行不要設定時
//					break;
//				}
//				if(	(OPECTL.Pay_mod == 2)&&
//					(!prm_get(COM_PRM, S_PRN, 32, 1, 6))){
//					BUZPIPI();
//					break;
//				}
//
//				BUZPI();
//				if( OPECTL.RECI_SW == 0 ){							// 領収書ﾎﾞﾀﾝ未使用?
//					OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
//// MH321800(S) D.Inaba ICクレジット対応
//					// カード抜き取りアナウンス中は[F2]押下後「領収証をお取り下さい」を表示させない
//					if( announce_flag != 1 ){
//// MH321800(E) D.Inaba ICクレジット対応
//					wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
//					grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);		// "     領収証をお取り下さい     "
//// MH321800(S) D.Inaba ICクレジット対応
//					}
//// MH321800(E) D.Inaba ICクレジット対応
//					if( ryo_en == 0 ){
//						break;
//					}
//					ryo_isu( 0 );									// 領収書(釣銭不足無し)発行
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//					OPECTL.PassNearEnd = 0;							// 期限切れ間近
//					ryo_isu_zumi = 1;								// 領収証発行済み
//				}
//				break;
//
//			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
//// MH321800(S) D.Inaba ICクレジット対応
//				// カード抜き取りアナウンス中は[取消]イベントを発生させない
//				if( announce_flag == 1 ){
//					break;
//				}
//// MH321800(E) D.Inaba ICクレジット対応
//				BUZPI();
//				// NOTE:キーを押下時点で音声を止め、精算データ7件ごとにFROMにライトアクセスするのでその時間を早めに確保し精算動作の遅延を軽減する
//				ope_anm( AVM_STOP );								// 放送停止ｱﾅｳﾝｽ
//				if(( ryo_en != 0 )&&( SFV_DAT.safe_cal_do == 0 )){	// 払出完了で金庫計算終了なら
//					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
//					ret = 10;
//				}
//				break;
//
//			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
//				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
//				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
//				break;
//
//			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
//				if( RD_mod < 9 ){
//					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
//				}
//				OPECTL.PassNearEnd = 0;								// 期限切れ間近
//				break;
//
//			case IBK_MIF_A2_NG_EVT:									// Mifareﾃﾞｰﾀ読出しNG
//				if( MIF_ENDSTS.sts1 == 0x30 ){						// ｶｰﾄﾞ無し
//					OPECTL.PassNearEnd = 0;							// 期限切れ間近ﾌﾗｸﾞ
//				}
//				break;
//
//// MH322914(S) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//			case ARC_CR_E_EVT:
//				if( (1 == nDelayAnnounce) && ( !IsMagReaderRunning() ) ){
//					nDelayAnnounce = 0;
//// MH321800(S) Y.Tanizaki ICクレジット対応
//					// カードを抜き取っていたらﾀｲﾏｰ1(10s)起動
//					if( !Suica_Rec.Data.BIT.CTRL_CARD ){
//						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//					}
//					else {
//						// カード抜き取り待ちタイマー起動
//						Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );
//						announce_flag = 1;
//						// 「カードをお取り下さい」表示とアナウンスをする
//						ec_MessageAnaOnOff( 1, 3 );
//						break;
//					}
//// MH321800(E) Y.Tanizaki ICクレジット対応
//					if( !OPECTL.multi_lk ){							// ﾏﾙﾁ精算問合せ中以外?
//						ope_anm( AVM_BGM );
//						if( OPECTL.Pr_LokNo >= LOCK_START_NO ){
//							ope_anm( AVM_KANRYO );					// 精算完了時ｱﾅｳﾝｽ(ロック)
//						}else{
//							ope_anm( AVM_KANRYO2 );					// 精算完了時ｱﾅｳﾝｽ(フラップ)
//						}
//					}
//					else{	// マルチ精算時はロック式の音声を流す（defaultでは「ありがとうございました」）
//						ope_anm( AVM_KANRYO );						// 精算完了時ｱﾅｳﾝｽ(ロック)
//					}
//				}
//				break;
//// MH322914(E) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
//
//			case TIMEOUT10:											// 「料金画面表示切替」のﾀｲﾑｱｳﾄ通知
//				dspCyclicMsgRewrite(1);	// サイクリック表示更新
//				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
//				break;
//// MH321800(S) G.So ICクレジット対応
//			case EC_CYCLIC_DISP_TIMEOUT:							// 「料金画面表示切替」のﾀｲﾑｱｳﾄ通知
//				dspCyclicMsgRewrite(1);	// サイクリック表示更新
//				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
//				break;
//// MH321800(E) G.So ICクレジット対応
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
			case TIMEOUT20:											// 払い出し可能BITON待ちﾀｲﾑｱｳﾄ通知
				Cnm_End_PayTimOut = 0x02;
				break;
			case OPE_REQ_CALC_FEE:									// 料金計算要求
				// 料金計算
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);				// 全車室情報テーブル送信
				break;
// MH321800(S) Y.Tanizaki ICクレジット対応
			case EC_REMOVAL_WAIT_TIMEOUT:
				// カード抜き取り待ちﾀｲﾏｰ発生
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//				Suica_Rec.Data.BIT.CTRL_CARD = 0;					// カードが抜き取られた事にする
				if (isEC_USE()) {
					Suica_Rec.Data.BIT.CTRL_CARD = 0;				// カードが抜き取られた事にする
				}
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
				// no break
			case EC_EVT_CARD_STS_UPDATE:
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//				// ｶｰﾄﾞ抜き取りｱﾅｳﾝｽ中
//				if( announce_flag ){
//					// カード抜き取り待ちﾀｲﾏｰﾘｾｯﾄ
//					Lagcan( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT );
//					ec_MessageAnaOnOff( 0, 0 );
//					if(!Suica_Rec.Data.BIT.CTRL_CARD) {				// カード差込なし
//						if(!nDelayAnnounce) {
//							Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );			// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//							if( !OPECTL.multi_lk ){											// ﾏﾙﾁ精算問合せ中以外?
//								ope_anm( AVM_BGM );
//								if( OPECTL.Pr_LokNo >= LOCK_START_NO ){
//									ope_anm( AVM_KANRYO );									// 精算完了時ｱﾅｳﾝｽ(ロック)
//								}else{
//									ope_anm( AVM_KANRYO2 );									// 精算完了時ｱﾅｳﾝｽ(フラップ)
//								}
//							}
//							else{	// マルチ精算時はロック式の音声を流す（defaultでは「ありがとうございました」）
//								ope_anm( AVM_KANRYO );										// 精算完了時ｱﾅｳﾝｽ(ロック)
//							}
//							announce_flag = 0;
//						}
//					}
//				}
				// カード抜き取り待ちタイマーリセット
				Lagcan( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT );
				// カード差込なし
				if( !Suica_Rec.Data.BIT.CTRL_CARD ){
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
//					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );			// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					if( r_zero_call ){
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)：短縮用
					}else{
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
				}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
				break;
// MH321800(E) Y.Tanizaki ICクレジット対応

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			// QRデータ
			case LCD_QR_DATA:
				// QR応答データ
				lcdbm_QR_data_res( 1 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
				break;

			// 操作通知(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// 操作通知		操作ｺｰﾄﾞ
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// 精算中止要求
					case LCDBM_OPCD_PAY_STP:
						// 操作通知(精算中止応答(NG))送信	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						break;

					// 精算開始要求
					case LCDBM_OPCD_PAY_STA:
// MH810100(S) K.Onodera 2020/02/18 #3883 「別車両の精算」押下すると車番検索画面に遷移してしまう
//						// 操作通知(精算開始応答(OK))送信	0=OK/1=NG
//						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );
//
//						// 変数初期化漏れ防止の為、op_mod00()を通してop_mod01()に遷移する
//						OPECTL.Ope_mod = 0;			// op_mod00(待機)
//						OPECTL.rec_pay_start = 1;	// 精算開始要求受信フラグON
//						ret = 1;
						// 精算開始応答送信(NG)
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
						break;

					// 待機画面通知
					case LCDBM_OPCD_CHG_IDLE_DISP_NOT:
// MH810105(S) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
//						ret = 10;
						if(print_wait_return){								// 領収証発行あり
							queset( OPETCBNO, LCD_CHG_IDLE_DISP, 0, NULL );
						}else{
							ret = 10;
						}
// MH810105(E) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
// MH810100(E) K.Onodera 2020/02/18 #3883 「別車両の精算」押下すると車番検索画面に遷移してしまう
						break;

// MH810105(S) MH364301 インボイス対応 GT-4100
_LCDBM_OPCD_RCT_ISU:
// MH810105(E) MH364301 インボイス対応 GT-4100
					// 領収証発行
					case LCDBM_OPCD_RCT_ISU:
// GG129000(S) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
						// 領収証発行ボタン押下後の領収証印字不可で必ず領収証発行失敗表示するため必ずフラグを立てる
						// ※各処理のエラー時のbreakでLCDBM_OPCD_RCT_ISUのケースを抜けずに
						// 　レシート印字失敗判定を行う為、if文ではなくwhile文を用いる。
						while ( OPECTL.RECI_SW == 0 ) {						// 領収書ﾎﾞﾀﾝ未使用?
							OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
// GG129000(E) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
// GG129000(S) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）
//						if( ReceiptChk() == 1 || Ope_isPrinterReady() == 0){	// ﾚｼｰﾄ発行不要設定時
						if( (QRIssueChk(1) == 0 && ReceiptChk() == 1) ||	// QR駐車券発行なし、かつ、レシート発行不可設定
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 							Ope_isPrinterReady() == 0){						// ﾚｼｰﾄ発行不可
							(!IS_ERECEIPT && (Ope_isPrinterReady() == 0))){	// ﾚｼｰﾄ発行不可
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// GG129000(E) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）
							break;
						}
// MH810105(S) MH364301 インボイス対応
						if (IS_INVOICE && Ope_isJPrinterReady() == 0) {		// ｲﾝﾎﾞｲｽ設定時はｼﾞｬｰﾅﾙ発行不可でも
							break;											// 印字しない
						}
// MH810105(E) MH364301 インボイス対応
						if(	(OPECTL.Pay_mod == 2)&&
							(!prm_get(COM_PRM, S_PRN, 32, 1, 6))){
							BUZPIPI();
							break;
						}
// GG129000(S) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
// 						if( OPECTL.RECI_SW == 0 ){							// 領収書ﾎﾞﾀﾝ未使用?
// 							OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
// GG129000(E) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
							if( ryo_en == 0 ){
								break;
							}
// MH810105(S) MH364301 インボイス対応
// MH810105(S) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
							print_wait_return = 1;								// 領収証発行あり
// MH810105(E) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
							if (IS_INVOICE &&
								OPECTL.f_DelayRyoIsu != 2) {
								break;
							}
							if (ryo_isu_zumi) {
								// 領収証発行済み
								break;
							}
// MH810105(E) MH364301 インボイス対応
							ryo_isu( 0 );									// 領収書(釣銭不足無し)発行
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
//							Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
							if( r_zero_call ){
								Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][129]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)：短縮用
							}else{
								Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][128]*50) );	// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
							}
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
							OPECTL.PassNearEnd = 0;							// 期限切れ間近
							ryo_isu_zumi = 1;								// 領収証発行済み
						}
// GG129000(S) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
						// 領収証印字不可後の領収証発行ボタン押下で領収証発行失敗表示するため
						// 領収証印字不可時のレシート印字失敗判定をここでも行う
						op_rct_failchk();
// GG129000(E) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
						break;

					// 起動通知
					case LCDBM_OPCD_STA_NOT:
						Discon_flg = 1;		// 終了後、起動中に遷移する
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							OPECTL.init_sts = 0xff;	// 初期化未完了(起動通知受信済み)状態とする
						}
						// 1=通常
						else{
							OPECTL.init_sts = 0xfe;	// 初期化完了済み(起動通知受信済み)状態とする
						}
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
						break;

// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
					// 起動完了通知
					case LCDBM_OPCD_STA_CMP_NOT:
						if( OPECTL.init_sts != 1 ){	// 初期化未完了？
							// op_mod03()の状態でFTPしていないのに起動完了通知を受信するということは
							// LCDはFTP不要との判断なので精算完了処理後はop_mod00()に戻る
							OPECTL.init_sts = 1;
							OPECTL.Ope_mod = 0;
							Discon_flg = 2;			// 終了後、待機に遷移する
						}
						break;

// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
					// 精算案内終了応答
					case LCDBM_OPCD_PAY_GUIDE_END_RES:
						Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );
						// 応答受信フラグON
						idle_req_flg = 2;
						break;
				}
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				Discon_flg = 1;		// 終了後、起動中に遷移する
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
				OPECTL.init_sts = 0;	// 初期化未完了状態とする
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(ICクレジット問合せ中の起動通知/切断通知処理見直し)
				// 払出し完了？
				if( ryo_en != 0 ){
					idle_req_flg = 2;	// 応答を受信したことにしておく
					ret = 10;
				}
				break;

			// 精算案内終了応答タイムアウト
			case TIMEOUT_MNT_RESTART:
				// 精算案内終了通知
				lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
				// 応答待ちタイマー再開
				Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

// MH810100(S) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
			// 待機画面通知受信済み
			case LCD_CHG_IDLE_DISP:
// MH810105(S) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
//				OPECTL.chg_idle_disp_rec = 0;
//				ret = 10;
				if(print_wait_return && ryo_isu_zumi == 0){					// 領収証発行ありの場合はまたせる
					queset( OPETCBNO, LCD_CHG_IDLE_DISP, 0, NULL );
				}else{
					OPECTL.chg_idle_disp_rec = 0;
					ret = 10;
				}
// GG129000(S) A.Shirai 2023/10/2 ゲート式車番チケットレスシステム対応（詳細設計#5688：釣銭切れで精算完了後トップに戻るボタンを押下してもトップ画面に戻らない）
				if (Ope_isJPrinterReady() == 0 || (paperchk2() != 0)) {
					OPECTL.chg_idle_disp_rec = 0;
					ret = 10;
				}
// GG129000(E) A.Shirai 2023/10/2 ゲート式車番チケットレスシステム対応（詳細設計#5688：釣銭切れで精算完了後トップに戻るボタンを押下してもトップ画面に戻らない）
// MH810105(E) MH364301 QRコード決済対応 #6424 精算完了後、すぐに精算完了画面から待機画面に遷移すると領収証が印字されない
				break;
// MH810100(E) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
// MH810104(S) MH321800(S) 制御データ（受付許可）送信タイマ動作中フラグクリア不具合
			case EC_RECEPT_SEND_TIMEOUT:
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,  0 );
				break;
// MH810104(E) MH321800(E) 制御データ（受付許可）送信タイマ動作中フラグクリア不具合
// MH810105(S) MH364301 インボイス対応
			case PRIEND_PREQ_RYOUSYUU:
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//				if (!IS_INVOICE) {
//// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
//					if (OPECTL.Pri_Kind == R_PRI) {
//						// レシート印字完了待ちタイマー起動
//						rpt_pri_wait_Timer = GyouCnt_All_r + GyouCnt_r;							// 送信済み印字ﾃﾞｰﾀ全行数取得
//						Lagtim(OPETCBNO, TIMERNO_RPT_PRINT_WAIT, (rpt_pri_wait_Timer * 5));		// 1行/100ms(20ms*5)
//					}
//// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
//					break;
//				}
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）

				if (OPECTL.Pri_Kind == J_PRI) {
					// ジャーナル印字

// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//					// 短い領収証（現金精算など）印字で印字中にフタを開けた場合、
//					// 印字完了→印字失敗という順で通知される場合がある
//					// そのため、印字完了受信後にWaitタイマを動作させて
//					// タイマ動作中に印字失敗を受信した場合は印字失敗として扱う
//// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
////					LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, JNL_PRI_WAIT_TIME, op_jnl_failchk);
//// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
////					LagTim500ms(LAG500_JNL_PRI_WAIT_TIMER, jnl_pri_wait_Timer, op_jnl_failchk);
//					jnl_pri_wait_Timer = GyouCnt_All_j + GyouCnt_j;								// 送信済み印字ﾃﾞｰﾀ全行数取得
//// GG129000(S) M.Fujikawa 2023/10/24 ゲート式車番チケットレスシステム対応　不具合#7170
//					if(jnl_pri_wait_Timer == 0){
//						jnl_pri_wait_Timer = 1;
//					}
//// GG129000(E) M.Fujikawa 2023/10/24 ゲート式車番チケットレスシステム対応　不具合#7170
//// GG129000(S) M.Fujikawa 2023/10/26 ゲート式車番チケットレスシステム対応　印字完了タイミングを早める
////					LagTim20ms(LAG20_JNL_PRI_WAIT_TIMER, (jnl_pri_wait_Timer *5), op_jnl_failchk);		// 1行/100ms(20ms*5)
//					LagTim20ms(LAG20_JNL_PRI_WAIT_TIMER, (jnl_pri_wait_Timer *3), op_jnl_failchk);		// 1行/60ms(20ms*3)
//// GG129000(E) M.Fujikawa 2023/10/26 ゲート式車番チケットレスシステム対応　印字完了タイミングを早める
//// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
//// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/領収証発行タイミング変更）
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
				}
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
				else {
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//					if( OPECTL.f_DelayRyoIsu == 2 && ryo_isu_zumi == 1 ){
					if( ryo_isu_zumi == 1 ){
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
						// レシート印字完了待ちタイマー起動
						rpt_pri_wait_Timer = GyouCnt_All_r + GyouCnt_r;							// 送信済み印字ﾃﾞｰﾀ全行数取得
						Lagtim(OPETCBNO, TIMERNO_RPT_PRINT_WAIT, (rpt_pri_wait_Timer * 5));		// 1行/100ms(20ms*5)
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
						ryo_isu_zumi = 0xff;						// レシート印字完了
						if( dummyNotce_Wait == 1 ) {				// すでに印字処理完了待ち
							// ﾀﾞﾐｰ通知で待機へ遷移する
							// 印字処理完了待ちではない場合、[領収証] ｲﾍﾞﾝﾄのﾀｲﾏｰ1で待機へ遷移する
							queset(OPETCBNO, OPE_DUMMY_NOTICE, 0, 0);
						}
					}
				}
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
				break;
			case OPE_OPNCLS_EVT:					// 強制営休業 event
// GG129000(S) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
// 				if (!IS_INVOICE) {
// 					break;
// 				}
// 
// 				statusChange_DispUpdate();			// 画面更新
				if ( IS_INVOICE ) {
					statusChange_DispUpdate();		// 画面更新
				}
// GG129000(E) R.Endo 2023/02/20 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる

				if (OPECTL.RECI_SW == 1) {
					// 領収証ボタン押下済みであれば、
					// レシート印字失敗判定を行う
					op_rct_failchk();
				}
// GG129000(S) D.Inaba 2023/10/03 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
				else{
					// 押下済みではない場合はフラグをOFFする
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 					if( (Ope_isPrinterReady() == 0) ||					// ﾚｼｰﾄ発行不可
					if( (!IS_ERECEIPT && (Ope_isPrinterReady() == 0)) ||	// ﾚｼｰﾄ発行不可
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
						(IS_INVOICE && Ope_isJPrinterReady() == 0) ){	// ｲﾝﾎﾞｲｽ設定時はｼﾞｬｰﾅﾙ発行不可
						OPECTL.f_DelayRyoIsu = 0;
						if( ryo_stock == 1 ){							// 印字データストック中
							MsgSndFrmPrn(PREQ_STOCK_CLEAR, R_PRI, 0);	// 印字データクリア
							ryo_stock = 2;								// ストックデータ破棄
						}
					}
				}
// GG129000(E) D.Inaba 2023/10/03 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
				break;
// MH810105(E) MH364301 インボイス対応
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
			case RPT_PRINT_WAIT_TIMEOUT:							// ﾚｼｰﾄ印字完了待ちﾀｲﾏｰ
					// 領収証POP削除要求
					lcdbm_notice_del(1,1);
				break;
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
			case TIMEOUT_QR_RECEIPT_DISP:							// QRコード発行案内表示タイマー
				lcdbm_notice_del(1, 1);	// 領収証POP, 印字完了
				break;
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/精算完了時の領収証発行中に待機状態に戻らないようにする）
			case OPE_DUMMY_NOTICE:									// ﾀﾞﾐｰ通知
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//				if( !IS_INVOICE ){									// インボイスでない
//					break;
//				}
//				if( OPECTL.f_DelayRyoIsu == 1 ){					// ジャーナル印字中
//					break;
//				}
//				if( OPECTL.f_DelayRyoIsu == 2 && ryo_isu_zumi == 1 ){ // レシート印字中
//					break;
//				}
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
				if( OPECTL.f_RctErrDisp == 1 ){						// 領収証発行失敗案内表示中
					break;
				}
				if( dummyNotce_Wait == 1 ) {						// 印字処理完了待ち
					ret = 10;										// 待機へ
				}
				break;
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/精算完了時の領収証発行中に待機状態に戻らないようにする）
			default:
				break;
		}
// MH810100(S) K.Onodera 2020/02/27 車番チケットレス（精算終了処理修正）
//		// 釣銭の払出しが未完了で抜けようとしたら、払出しが完了するまで待つ
//		if(( ryo_en == 0 )&&( ret )){								// 払出未完了?
//			ret_bak = ret;											// Yes..Save
//			ret = 0;
//		}
		if( ret ){
			if( ryo_en != 0 ){
				if( idle_req_flg == 0 ){
					// 精算案内終了通知
					lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
					// 精算案内終了応答待ち
					idle_req_flg = 1;
					// 応答待ちタイマー開始
					Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				}
			}
// MH810100(S) K.Onodera 2020/03/02 #3964 領収証を印字しない
			else{
				ret_bak = ret;
			}
// MH810100(E) K.Onodera 2020/03/02 #3964 領収証を印字しない
			ret = 0;
		}
		// 精算案内終了応答受信済み
		if( idle_req_flg == 2 ){
			ret = 10;	// 終了へ
		}
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/精算完了時の領収証発行中に待機状態に戻らないようにする）
		// 印字関連処理中で抜けようとしたら、完了するまで待つ
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//		if( ret == 10 && IS_INVOICE ){								// インボイス
		if( ret == 10 ){
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
			if( OPECTL.f_DelayRyoIsu == 1 ||						// ジャーナル印字中
				OPECTL.f_RctErrDisp == 1 ||							// 領収証発行失敗案内表示中
				(OPECTL.f_DelayRyoIsu == 2 && ryo_isu_zumi == 1) ){ // レシート印字中
				dummyNotce_Wait = 1;								// 印字処理完了待ち
				ret = 0;											// 処理継続
			}
		}
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/精算完了時の領収証発行中に待機状態に戻らないようにする）
// MH810100(E) K.Onodera 2020/02/27 車番チケットレス（精算終了処理修正）
	}
// MH321800(S) Y.Tanizaki ICクレジット対応
	ope_anm( AVM_STOP );
// MH321800(E) Y.Tanizaki ICクレジット対応
	Lagcan( OPETCBNO, 10 );											// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
// MH321800(S) G.So ICクレジット対応
	Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );						// ecリサイクル表示用ﾀｲﾏｰﾘｾｯﾄ
	Lagcan( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT );					// カード抜き取り待ちﾀｲﾏｰﾘｾｯﾄ
// MH321800(E) G.So ICクレジット対応
// MH810105(S) MH364301 インボイス対応
	LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);							// ジャーナル印字完了待ちタイマ
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
	Lagcan( OPETCBNO, TIMERNO_RPT_PRINT_WAIT );						// レシート印字完了待ちタイマリセット
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
	LagCan500ms(LAG500_RECEIPT_MISS_DISP_TIMER);					// 領収証失敗表示タイマ
// MH810105(E) MH364301 インボイス対応
// MH810100(S) K.Onodera 2020/02/27 車番チケットレス（精算終了処理修正）
	Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );
// MH810100(E) K.Onodera 2020/02/27 車番チケットレス（精算終了処理修正）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	LagCan20ms( LAG20_JNL_PRI_WAIT_TIMER );							// ジャーナル印字完了待ちタイマ
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
	Lagcan(OPETCBNO, TIMERNO_QR_RECEIPT_DISP);						// QRコード発行案内表示タイマー
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
	if( OPECTL.RECI_SW == 1 ){										// 領収書ﾎﾞﾀﾝ使用?
		OPECTL.RECI_SW = 0;											// 領収書ﾎﾞﾀﾝ未使用
	}else{
// MH810105(S) MH364301 インボイス対応（精算完了集計のタイミング変更）
		// ゲート式と同様に精算完了集計のタイミングを早めたため、
		// ここでの精算完了集計処理をやめる
//		if( !(uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )				// 精算完了通信電文送信タイミング=待機へ戻る時
//		{
//			PayData_set( 0, 0 );									// 1精算情報ｾｯﾄ
//			kan_syuu();												// 精算完了集計
//		}
// MH810105(E) MH364301 インボイス対応（精算完了集計のタイミング変更）
		OPECTL.RECI_SW = (char)-1;									// 待機中領収証ﾎﾞﾀﾝ可
		if(OPECTL.Pay_mod == 2){									// 修正精算中
			OPECTL.RECI_SW = 0;
		}
	}
	// 金庫計算未完了（タイムアウト）で抜けた場合などの集計
// MH810105(S) MH364301 インボイス対応（精算完了集計のタイミング変更）
//	if( (uchar)prm_get(COM_PRM, S_NTN, 26, 1 ,1) )					// 精算完了通信電文送信タイミング=精算完了直後
// MH810105(E) MH364301 インボイス対応（精算完了集計のタイミング変更）
	{
		if( !OPECTL.f_KanSyuu_Cmp ){								// 完了集計未実施
			PayData_set( 0, 0 );									// 1精算情報ｾｯﾄ
			kan_syuu();												// 精算完了集計
		}
	}
	// FTは途中の画面の更新等はないので、画面を抜ける最後だけ表示チェックする
	PayDataErrDispClr();
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//	Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );					// 金銭管理ログデータ作成
//	Log_regist( LOG_MONEYMANAGE_NT );								// 金銭管理ログ登録
	if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {			// 金銭管理ログデータ作成
		Log_regist( LOG_MONEYMANAGE_NT );							// 金銭管理ログ登録
	}
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)

	if(OPECTL.RECI_SW != (char)-1){
	if(Flap_Sub_Flg == 1){
		if(Flap_Sub_Num < 9){
			buf_size = (uchar)(9 - Flap_Sub_Num);
			memset(&FLAPDT_SUB[Flap_Sub_Num],0,sizeof(flp_com_sub));			// 詳細中止ｴﾘｱ(精算完了)ｸﾘｱ
			memmove(&FLAPDT_SUB[Flap_Sub_Num],&FLAPDT_SUB[Flap_Sub_Num+1],sizeof(flp_com_sub)*buf_size);
		}
		if(Flap_Sub_Num == 10){
			memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));							// 終端詳細中止ｴﾘｱｸﾘｱ
		}else{
			memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// 終端詳細中止ｴﾘｱｸﾘｱ
		}
		Flap_Sub_Flg = 0;														// 詳細中止ｴﾘｱ使用フラグOFF
	}
	}

	cn_stat( 8, 0 );	// 保有枚数送信

	f_MandetMask = 1;

	e_inflg = 0;													// 今回の入金は「電子マネー」1:である0:ない をクリア
	e_incnt = 0;													// 精算内の「電子マネー」使用回数（含む中止）をクリア
	e_pay = 0;														// 電子マネー決済額をクリア
	e_zandaka = 0;													// 電子マネー残高をクリア
// MH810105(S) MH364301 QRコード決済対応
	q_inflg = 0;													// 今回の入金は「QRコード」1:である0:ない をクリア
// MH810105(E) MH364301 QRコード決済対応
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	if( ryo_stock == 1 ){											// 印字データストック中
		MsgSndFrmPrn(PREQ_STOCK_CLEAR, R_PRI, 0);					// 印字データクリア
	}
	ryo_stock = 0;													// ストックなし
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( Led_status[0] || Led_status[1] || Led_status [2] )			// LEDのどれかが点灯または点滅していた場合
//		EDY_LED_OFF();												// LED消去
//	if( Edy_Rec.edy_status.BIT.CTRL )								// Edyの状態がｶｰﾄﾞｾﾝｽ可能状態なら
//		Edy_SndData04();											// ｶｰﾄﾞ検知停止指示送信
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	if(OPECTL.Pay_mod == 2){										// 修正精算中
		OPECTL.Pay_mod = 0;
		wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
		OPECTL.Kakari_Num = 0;										// 係員№ｸﾘｱ
	}
	// 念の為フラグクリア処理を入れておく
	Cnm_End_PayTimOut = 0;

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	// 精算完了後、起動中に戻す？
	if( Discon_flg == 1 ){			// 終了後、起動中に遷移
		OPECTL.Ope_mod = 255;		// op_init00(起動中)
		ret = -1;
	}else if( Discon_flg == 2 ){	// 終了後、待機に遷移？
		ope_idle_transit_common();	// 待機状態遷移共通処理
		ret = -1;
	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 修正精算駐車位置番号入力処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod11( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = 他処理へ切換                              |*/
/*|                       : -1 = ﾓｰﾄﾞ切替                                  |*/
/*|                       : 10 = 取消(中止)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod11( void )
{
	short	ret;
	short	ans;
	ushort	msg = 0;
	ushort	inp_lin = 2;											// 区画,駐車位置番号の入力行
	char	setwk;

	Suica_Rec.Data.BIT.PAY_CTRL = 0;
	OPECTL.MOp_LokNo = 0L;											// 間違えた接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)ｸﾘｱ
	OPECTL.MPr_LokNo = 0;											// 間違えた内部処理用駐車位置番号(1～324)ｸﾘｱ
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// 未了タイムアウトフラグクリア
	Suica_rcv_split_flag_clear();

	Syusei_Select_Flg = 0;

	key_num = 0;													// ｷｰ入力値ｸﾘｱ
	if( CPrmSS[S_TYP][81] ){										// 区画使用設定有り?
		key_sec = 1;												// 区画を[Ａ]とする(区画使用する)
	}else{
		key_sec = 0;												// 区画入力値ｸﾘｱ(区画使用しない)
	}
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

	OpeLcd( 10 );													// 修正精算駐車位置番号入力画面

	rd_shutter();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出

	ope_anm( AVM_STOP );											// 放送停止ｱﾅｳﾝｽ

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( Edy_Rec.edy_status.BIT.INITIALIZE ){						// Edyﾓｼﾞｭｰﾙの初期化が完了している？
//		Edy_StopAndLedOff();										// ｶｰﾄﾞ検知停止＆LEDOFF開始
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 0 ){							// key OFF
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					OPECTL.Pay_mod = 0;								// 精算の種類(通常精算)
					wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
					OPECTL.Kakari_Num = 0;									// 係員№ｸﾘｱ
					ret = -1;
				}
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
				BUZPI();
				key_num = (( key_num % knum_ket ) * 10 )+( msg - KEY_TEN );
				opedsp( inp_lin, 22, key_num, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// 駐車位置番号表示書換え
				break;
			case KEY_TEN_CL:										// ﾃﾝｷｰ[区画] ｲﾍﾞﾝﾄ
				BUZPI();
#if	UPDATE_A_PASS
				if( !CPrmSS[S_KOU][1] ){							// 定期券更新精算なし?
#endif	// UPDATE_A_PASS
					if( key_sec == 0 )	break;
					if( key_sec > 0 )	key_sec++;
					if( key_sec > CPrmSS[S_TYP][81] )	key_sec = 1;
					grachr( inp_lin, 20, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );	// 区画表示切換
#if	UPDATE_A_PASS
				}
#endif	// UPDATE_A_PASS
				break;
			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
				BUZPI();
				if( inp_lin == 2 ){									// 間違えた駐車位置番号入力
					ans = carchk( key_sec, key_num, 1 );			// 駐車位置番号ﾁｪｯｸ
					if(( ans != 0 )||								// OK以外 or
					   ( OPECTL.MOp_LokNo == OPECTL.Op_LokNo )){	// 入力した接客用駐車位置番号が同じ
						ope_anm( AVM_BTN_NG );						// 車室番号間違い時ｱﾅｳﾝｽ
						key_num = 0;
						opedsp( inp_lin, 22, key_num, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	// 駐車位置番号表示書換え
						break;
					}
					if( ans == 0 ){									// OK
						OPECTL.MOp_LokNo = OPECTL.Op_LokNo;			// 間違えた接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)ｾｯﾄ
						OPECTL.MPr_LokNo = OPECTL.Pr_LokNo;			// 間違えた内部処理用駐車位置番号(1～324)ｾｯﾄ
						grachr( inp_lin, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );	// 間違った区画表示切換
						opedsp( inp_lin, 22, key_num, knum_len, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );			// 間違った駐車位置番号表示書換え
						inp_lin = 3;								// 正しい駐車位置番号へ切換
						if( CPrmSS[S_TYP][81] ){					// 区画使用設定有り?
							key_sec = 1;							// 区画を[Ａ]とする(区画使用する)
							grachr( inp_lin, 20, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );// 正しい区画表示切換
						}else{
							key_sec = 0;							// 区画入力値ｸﾘｱ(区画使用しない)
						}
						key_num = 0;
						opedsp( inp_lin, 22, key_num, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );			// 正しい駐車位置番号表示書換え
					}
				}else{
					ans = carchk( key_sec, key_num, 0 );			// 駐車位置番号ﾁｪｯｸ
					setwk = (char)Carkind_Param(FLP_ROCK_INTIME, (char)(LockInfo[OPECTL.Pr_LokNo-1].ryo_syu),1,1);
					if(( ans == -1 )||( ans == 10 )||				// 精算不可能 or 未設定 or
					   ( ans == 2 )||(( ans == 1 ) && ( setwk == 0 ))||	// 出庫状態 or ﾛｯｸ装置閉開始待ち and ﾛｯｸ装置閉開始ﾀｲﾏｰ起動中に精算しない設定 or
					   ( OPECTL.MOp_LokNo == OPECTL.Op_LokNo )){	// 入力した接客用駐車位置番号が同じ
						ope_anm( AVM_BTN_NG );						// 車室番号間違い時ｱﾅｳﾝｽ
						key_num = 0;
						opedsp( inp_lin, 22, key_num, knum_len, 1, 1 , COLOR_BLACK, LCD_BLINK_OFF);	// 駐車位置番号表示書換え
						break;
					}
					/*** 入出庫時刻条理ﾁｪｯｸ ***/
					if( set_tim( OPECTL.Pr_LokNo, &CLK_REC, 1 ) == 1 ){	// 入出庫時刻NG?(Y)
						ope_anm( AVM_BTN_NG );						// 車室番号間違い時ｱﾅｳﾝｽ
						key_num = 0;
						opedsp( inp_lin, 22, key_num, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	// 駐車位置番号表示書換え
						break;
					}
					OPECTL.Ope_mod = 12;								// 修正精算確認表示へ
					ret = 1;
				}
				break;
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				if(( key_num == 0 )&&( inp_lin == 2 )){
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
					OPECTL.Kakari_Num = 0;										// 係員№ｸﾘｱ
					ret = -1;
					break;
				}
				inp_lin = 2;										// 間違えた駐車位置番号入力
				OPECTL.MOp_LokNo = 0L;								// 間違えた接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)ｸﾘｱ
				OPECTL.MPr_LokNo = 0;								// 間違えた内部処理用駐車位置番号(1～324)ｸﾘｱ
				key_num = 0;										// ｷｰ入力値ｸﾘｱ
				if( CPrmSS[S_TYP][81] ){							// 区画使用設定有り?
					key_sec = 1;									// 区画を[Ａ]とする(区画使用する)
					grachr( 2, 20, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ key_sec * 2 ] );	// 区画表示
					opedsp( 2, 22, 0, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );				// 駐車位置番号0表示
					grachr( 3, 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &OPE_ENG[ 1 * 2 ] );		// 区画表示
					opedsp( 3, 22, 0, knum_len, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );				// 駐車位置番号0表示
				}else{
					key_sec = 0;									// 区画入力値ｸﾘｱ(区画使用しない)
					opedsp( 2, 22, 0, knum_len, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );				// 駐車位置番号0表示
					opedsp( 3, 22, 0, knum_len, 1, 0 , COLOR_BLACK, LCD_BLINK_OFF);				// 駐車位置番号0表示
				}
				break;
			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
				BUZPI();
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					Ope2_WarningDispEnd();							// ｴﾗｰ表示消し
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			default:
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 修正精算料金表示(間違った駐車位置の料金 > 正しい駐車位置の料金)        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod12( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 10=中止  1=戻さない  2=硬貨戻す                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*[]----------------------------------------------------------------------[]*/
/*| 修正精算料金表示(確認のため修正元車室の情報を表示)                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod12( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret :                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

short	op_mod12( void )
{
	short	ret;
	ushort	msg = 0;
	if( prm_get(COM_PRM, S_TYP, 98, 1, 1) == 2 ){					// 修正元現金入金額の払戻選択
		Syusei_Select_Flg = 1;
	}

	OpeLcd( 11 );													// 修正精算料金表示(違＞正)

	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 0 ){							// key OFF
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					OPECTL.Pay_mod = 0;								// 精算の種類(通常精算)
					wopelg( OPLOG_SYUSEI_END, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
					OPECTL.Kakari_Num = 0;										// 係員№ｸﾘｱ
					ret = -1;
				}
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
			case KEY_TEN_CL:										// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
				BUZPI();
				break;
			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
				BUZPI();
				if( prm_get(COM_PRM, S_TYP, 98, 1, 1) == 2 ){		// 修正元現金入金額の払戻選択
					Syusei_Select_Flg = 2;							// 払戻しあり
				}
				SyuseiDataSet( OPECTL.MPr_LokNo );					// 
				ope_SyuseiStart( OPECTL.Pr_LokNo );					// 新規関数
				queset(OPETCBNO, SYU_SEI_SEISAN, 0, NULL);			// 修正精算開始イベントセット
				OPECTL.Ope_mod = 2;									// 料金・入金処理へ
				ret = 1;
				break;
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				ret = 10;
				break;
			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
				BUZPI();
				if( prm_get(COM_PRM, S_TYP, 98, 1, 1) == 2 ){		// 修正元現金入金額の払戻選択
					SyuseiDataSet( OPECTL.MPr_LokNo );				// 
					ope_SyuseiStart( OPECTL.Pr_LokNo );				// 新規関数
					queset(OPETCBNO, SYU_SEI_SEISAN, 0, NULL);		// 修正精算開始イベントセット
					Syusei_Select_Flg = 1;							// 払戻しなし
					OPECTL.Ope_mod = 2;								// 料金・入金処理へ
					ret = 1;
				}
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			default:
				break;
		}
	}
	return( ret );
}



#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
//----------------------------------------------------------------------------
// 新修正精算機能追加(新修正精算ではこの関数は使わない)
//----------------------------------------------------------------------------
/*[]----------------------------------------------------------------------[]*/
/*| 修正精算入金処理(間違った駐車位置の料金 < 正しい駐車位置の料金)        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod13( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 10=中止  1=戻さない  2=硬貨戻す                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod13( void )
{
	short	ret;
	ushort	msg = 0;
	ushort	loklen, Mloklen;
	ushort	wk_MsgNo;
	delay_count = 0;												// 未了発生中のﾒｯｾｰｼﾞｶｳﾝﾄのｸﾘｱ
	memset( nyukin_delay,0,sizeof( nyukin_delay ));					// 未了発生中のﾒｯｾｰｼﾞを保持領域のｸﾘｱ
	dsp_change = 0;
    edy_dsp.BYTE = 0;												// Edy画面表示用領域初期化
	cansel_status.BYTE = 0;											// 取消中の電子媒体停止状態初期化
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Pay_Work = 0;												// Edy決済額ﾜｰｸ領域
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	CCT_Cansel_Status.BYTE = 0;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ使用時の各電子媒体の停止状況
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_Rec.edy_status.BIT.MIRYO_LOSS = 0;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// 未了タイムアウトフラグクリア
	Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

	time_out = 0;
	Ope_Last_Use_Card = 0;											// 最後に利用したカード初期化
	w_settlement = 0;											// Suica残高不即時の精算額（０時は残高不足でないを意味する）

	OpeLcd( 12 );													// 修正精算料金表示(違＜正)

	OPECTL.op_faz = 0;												// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ
	OPECTL.RECI_SW = 0;												// 領収書ﾎﾞﾀﾝ未使用
	RECI_SW_Tim = 0;												// 領収証受付時間ｸﾘｱ
	OPECTL.CAN_SW = 0;												// 取消ﾎﾞﾀﾝ
	ryo_buf.dsp_ryo -= ryo_buf.mis_ryo;								// 表示＆料金減算ため表示料金を間違い料金を引いた額とする。

	cn_stat( 1, 2 );												// 紙幣 & COIN入金可

	nmicler( &SFV_DAT.refval, 8 );									// 不足額ｸﾘｱ

	OPE_red = 3;													// 精算中
	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
	}

	if( MIFARE_CARD_DoesUse ){										// Mifareが有効な場合
		op_MifareStart();											// Mifareを有効化
	}

	read_sht_opn();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
	OPECTL.coin_syukei = 0;											// ｺｲﾝ金庫集計未出力とする
	OPECTL.note_syukei = 0;											// 紙幣金庫集計未出力とする

	dsp_fusoku = 0;
	suica_fusiku_flg = 0;									

	for( ret = 0; ret == 0; ){
		msg = GetMessage();

		if( Ope_MiryoEventCheck( msg ) == 1 )
			continue;

		if( Ope_ArmClearCheck( msg ) == 1 )
			continue;

		switch( msg ){
			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
				BUZPI();
				break;

			case COIN_EVT:											// Coin Mech event
				if( OPECTL.op_faz == 4 ){							// ﾀｲﾑｱｳﾄ
					if( OPECTL.CN_QSIG == 7 ){						// 払出可
//						ret = 10;									// 待機に戻る
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						if( cansel_status.BYTE == 0x0d ){				// 終了ｽﾃｰﾀｽ：Suica&Edy終了済みの場合
						if( cansel_status.BYTE == 0x05 ){				// 終了ｽﾃｰﾀｽ：Suica終了済みの場合
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
							ret = 10;									// 待機に戻る
						}else{											// Suicaがまだ終了していない場合
							if( SUICA_USE_ERR ){
								cansel_status.BIT.SUICA_END = 1;		// Suica終了済みﾌﾗｸﾞをｾｯﾄ
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//								if( cansel_status.BYTE == 0x0d ){		// 終了ｽﾃｰﾀｽ：Suica&Edy終了済みの場合	
								if( cansel_status.BYTE == 0x05 ){		// 終了ｽﾃｰﾀｽ：Suica終了済みの場合	
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
									ret = 10;							// 待機に戻る
									break;
								}
							}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//							if( EDY_USE_ERR ){							// Edy関連のｴﾗｰが発生中
//								cansel_status.BIT.EDY_END = 1;			// Edy終了済みﾌﾗｸﾞをｾｯﾄ
//								if( cansel_status.BYTE == 0x0d ){		// 終了ｽﾃｰﾀｽ：Suica&Edy終了済みの場合
//									ret = 10;							// 待機に戻る
//									break;
//								}
//							}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
							cansel_status.BIT.CMN_END = 1;				// ﾒｯｸ終了済みﾌﾗｸﾞをｾｯﾄ
						}
						break;
					}else{
						OPECTL.op_faz = 0;
						cn_stat( 3, 2 );							// 入金可
					}
				}
			case NOTE_EVT:											// Note Reader event
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				if( msg == KEY_TEN_F4 ){							// 取消ﾎﾞﾀﾝ?(Y)
					if( !Suica_Rec.suica_err_event.BYTE && (!Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL) ) 	// 制御ﾃﾞｰﾀ送信後、ﾌﾗｸﾞが更新される前に取消しキーを押された場合
						break;
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					if( dsp_fusoku || edy_dsp.BIT.edy_dsp_Warning ){
					if( dsp_fusoku ){
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) G.So ICクレジット対応
						if (isEC_USE() != 0) {
							Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );
						} else {
// MH321800(E) G.So ICクレジット対応
						Lagcan( OPETCBNO, 10 );
// MH321800(S) G.So ICクレジット対応
						}
// MH321800(E) G.So ICクレジット対応
					}
					if( OPECTL.op_faz == 9 ) {
						break;
					}
					BUZPI();
					Cansel_Ele_Start();
				}
				if(( OPECTL.op_faz == 0 )&&( msg != KEY_TEN_F4 )){
					OPECTL.op_faz = 1;								// 入金中
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				}
				ret = in_mony( msg, 1 );							// 入金処理ｻﾌﾞ
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				if( OPECTL.op_faz > 2 ){							// 精算中ﾃﾞ ﾅｲ?
					opr_snd( 2 );									// 前排出
				}else if( ryo_buf.zankin == 0 ){					//
					break;
				}else{
//					if(( OPECTL.op_faz == 0 )&&( r_zero != 0 )){	//
					if( OPECTL.op_faz == 0 ){
						Lagcan( OPETCBNO, 1 );						// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
					ret = al_card( msg , 1 );						// ｶｰﾄﾞ処理
				}
				break;

			case ARC_CR_E_EVT:										// 終了ｺﾏﾝﾄﾞ受信
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り

				// 領収証再表示
				if( OPECTL.RECI_SW == 1 ){							// 領収証ﾎﾞﾀﾝ使用?
					wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
					grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);		// "      領収証を発行します      "
				}else{
					grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);			// " 必要な方は領収を押して下さい "
				}
				break;

			case IBK_MIF_A2_OK_EVT:									// Mifareﾃﾞｰﾀ読出し完了
				if(( OPECTL.op_faz > 2 )||( ryo_buf.zankin == 0 )){	// 精算中では無い or 残額0円
					break;
				}else{
					if( OPECTL.op_faz == 0 ){						//
						Lagcan( OPETCBNO, 1 );						// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
					ret = al_iccard( msg , 0 );						// ICｶｰﾄﾞ処理
				}
				break;

			case IBK_MIF_A2_NG_EVT:									// Mifareﾃﾞｰﾀ読出しNG
				if( MIF_ENDSTS.sts1 == 0x30 ){						// ｶｰﾄﾞ無し
					// 領収証再表示
					if( OPECTL.RECI_SW == 1 ){						// 領収証ﾎﾞﾀﾝ使用?
						wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
						grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      領収証を発行します      "
					}else{
						grachr(7, 0, 30, 0, COLOR_DARKGREEN, LCD_BLINK_OFF, OPE_CHR[20]);		// " 必要な方は領収を押して下さい "
					}
				}
				break;

			case TIMEOUT1:
				time_out = 1;									// ﾀｲﾑｱｳﾄ発生ﾌﾗｸﾞON
				cansel_status.BIT.INITIALIZE = 1;				// 電子媒体停止ｽﾀｰﾄ

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( OPECTL.op_faz == 9 || Suica_Rec.Data.BIT.CTRL || Edy_Rec.edy_status.BIT.CTRL ){		// 取消し処理実行中の場合
				if( OPECTL.op_faz == 9 || Suica_Rec.Data.BIT.CTRL ){		// 取消し処理実行中の場合
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					if( Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL ){	// Suicaが有効かつ最後に送信したのが受付許可の場合
						Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
					}

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					if( Edy_Rec.edy_status.BIT.CTRL )
//						Edy_StopAndLedOff();
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

					LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 20, Op_ReturnLastTimer );	// 残額表示用ﾀｲﾏｰを一時的に使用する
				}
				if( OPECTL.op_faz >= 3 ){
					if( carduse() ){								// ｶｰﾄﾞ使用?
						chu_isu();									// 中止処理
						svs_renzoku();								// ｻｰﾋﾞｽ券連続挿入
					}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					if( cansel_status.BYTE == 0x0d ){				// 終了ｽﾃｰﾀｽ：Suica&Edy終了済みの場合
					if( cansel_status.BYTE == 0x05 ){				// 終了ｽﾃｰﾀｽ：Suica終了済みの場合
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
						ret = 10;									// 待機に戻る
					}else{											// Suicaがまだ終了していない場合
						if( SUICA_USE_ERR ){
							cansel_status.BIT.SUICA_END = 1;		// Suica終了済みﾌﾗｸﾞをｾｯﾄ
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//							if( cansel_status.BYTE == 0x0d ){		// 終了ｽﾃｰﾀｽ：Suica&Edy終了済みの場合	
							if( cansel_status.BYTE == 0x05 ){		// 終了ｽﾃｰﾀｽ：Suica終了済みの場合	
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
								ret = 10;							// 待機に戻る
								break;
							}
						}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						if( EDY_USE_ERR ){							// Edy関連のｴﾗｰが発生中
//							cansel_status.BIT.EDY_END = 1;			// Edy終了済みﾌﾗｸﾞをｾｯﾄ
//							if( cansel_status.BYTE == 0x0d ){		// 終了ｽﾃｰﾀｽ：Suica&Edy終了済みの場合
//								ret = 10;							// 待機に戻る
//								break;
//							}
//						}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
						cansel_status.BIT.CMN_END = 1;				// ﾒｯｸ終了済みﾌﾗｸﾞをｾｯﾄ
					}
					break;
				}
				cn_stat( 2, 2 );									// 入金不可
				OPECTL.op_faz = 4;									// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ ﾀｲﾑｱｳﾄ
				Lagcan( OPETCBNO, 1 );								// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				Lagtim( OPETCBNO, 2, 10*50 );						// ﾒｯｸ監視ﾀｲﾏｰ起動
				break;

			case TIMEOUT2:											// ﾒｯｸ監視ﾀｲﾏｰﾀｲﾑｱｳﾄ
				switch( OPECTL.op_faz ){
					case 2:											// 完了
						if( ryo_buf.turisen ){						//
							SFV_DAT.reffal = ryo_buf.turisen;		//
							ret = 3;								// 精算完了処理へ
						}else{
							ret = 1;								// 精算完了処理へ
						}
						break;
					case 3:											// 中止
					case 4:											// ﾀｲﾑｱｳﾄ
					case 9:											// ﾀｲﾑｱｳﾄ
						if( ryo_buf.nyukin ){						//
							ryo_buf.turisen = ryo_buf.nyukin;		//
							if( SFV_DAT.reffal ){					// 不足あり?
								ryo_buf.fusoku += SFV_DAT.reffal;	// 支払い不足額
							}
							if( SFV_DAT.nt_escrow ){				// 紙幣戻しｴﾗｰ
								ryo_buf.fusoku += 1000L;			// 支払い不足額
							}
							chu_isu();								// 中止集計
							Lagtim( OPETCBNO, 1, 10*50 );			// ﾀｲﾏｰ1起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
						}else{
							if( !Suica_Rec.suica_err_event.BYTE ){					// Suicaｴﾗｰが発生していない
								if( Suica_Rec.Data.BIT.CTRL ){
									Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
									OPECTL.op_faz = 9;
									LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 20, Op_ReturnLastTimer );	// 残額表示用ﾀｲﾏｰを一時的に使用する
									break;
								}
							}
							ret = 10;								// 待機に戻る
						}
						break;
					default:
						ret = 10;									// 待機に戻る
						break;
				}
				break;
			case TIMEOUT11:											// Suica「ｶｰﾄﾞ認証ｴﾗｰ/精算未了」解消待ちのﾀｲﾑｱｳﾄ通知
				Ope_TimeOut_11( OPECTL.Ope_mod,  0 );
				break;

			case TIMEOUT10:											// 「Suica停止後の受付許可送信待ち」or「料金画面表示切替」のﾀｲﾑｱｳﾄ通知
				Ope_TimeOut_10( OPECTL.Ope_mod,  0 );
				break;

// MH321800(S) G.So ICクレジット対応
			case EC_CYCLIC_DISP_TIMEOUT:							// 「ec停止後の受付許可送信待ち」or「料金画面表示切替」のﾀｲﾑｱｳﾄ通知
				Ope_TimeOut_Cyclic_Disp( OPECTL.Ope_mod,  0 );
				break;

			case EC_RECEPT_SEND_TIMEOUT:							// ec「ｶｰﾄﾞ認証ｴﾗｰ/精算未了」解消待ちのﾀｲﾑｱｳﾄ通知ｰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,  0 );
				break;
// MH321800(E) G.So ICクレジット対応

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			case IBK_EDY_RCV:															// Edyﾓｼﾞｭｰﾙからの受信ﾃﾞｰﾀ？
//				ret = Ope_Edy_Event( msg, OPECTL.Ope_mod );
//				break;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			default:
				break;
		}
	}

	OPECTL.Fin_mod = ret;
	if( ret == 10 ){
		_di();
		CN_escr_timer = 0;											// ｴｽｸﾛﾀｲﾏｰｷｬﾝｾﾙ
		_ei();
		safecl( 7 );												// 金庫枚数算出
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//		Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );	// 中止時の金銭管理ログデータ作成
//		Log_regist( LOG_MONEYMANAGE_NT );							// 金銭管理ログ登録
		if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {	// 中止時の金銭管理ログデータ作成
			Log_regist( LOG_MONEYMANAGE_NT );						// 金銭管理ログ登録
		}
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
//		if( FLAPDT.flp_data[OPECTL.MPr_LokNo-1].mode != FLAP_CTRL_MODE4 ){		// 駐車中でない?
//			queset( FLPTCBNO, FLAP_DOWN_SND_SS, sizeof( OPECTL.MPr_LokNo ), &OPECTL.MPr_LokNo );	// 修正精算用ﾛｯｸ装置開(ﾌﾗｯﾌﾟ下降)
//		}
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
	}else{
		OPECTL.Ope_mod = 14;										// 修正精算精算完了へ
	}

	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 2 );											// ﾀｲﾏｰ2ﾘｾｯﾄ(ﾒｯｸ監視用)
	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );				// Suica関連ﾀｲﾏｰｸﾘｱ

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 修正精算完了                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod14( void                                          |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = 正常完了                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod14( void )
{
	short	ret;
	short	ryo_en;
	short	safe_que = 0;
	ushort	msg = 0;

	Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( PayData.Electron_data.Edy.e_pay_kind != EDY_USED ){
//		Edy_StopAndLedOff();
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	if( MIFARE_CARD_DoesUse ){										// Mifareが有効な場合
		op_MifareStop_with_LED();								 	// Mifareを停止する
	}
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
//	queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof( OPECTL.Pr_LokNo ), &OPECTL.Pr_LokNo );	// 精算完了ｷｭｰｾｯﾄ(ﾌﾗｯﾊﾟｰ下降)
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
	IoLog_write(IOLOG_EVNT_AJAST_FIN, (ushort)LockInfo[OPECTL.Pr_LokNo - 1].posi, 0, 0);

	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出

	rd_shutter();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御

	ryo_en = 0;														// 領ﾎﾞﾀﾝ不可

	switch( OPECTL.Fin_mod ){
		case 0:														// 同額?
			ac_flg.cycl_fg = 73; 									// （仮）払い出し完了
			ryo_en = 1;
			OpeLcd( 13 );											// 修正精算完了表示(同額)
			break;

		case 1:														// 釣り無し?
		default:
			ac_flg.cycl_fg = 73; 									// （仮）払い出し完了
			ryo_en = 1;
			OpeLcd( 14 );											// 修正精算完了表示(釣無し)
			break;

		case 2:														// 釣り有り?
		case 3:														// ﾒｯｸｴﾗｰ(釣り有り)?
			OpeLcd( 15 );											// 修正精算完了表示(釣有り)
			if(( OPECTL.Fin_mod == 3 )||
			   (( OPECTL.Fin_mod == 2 )&&
			    ( refund( (long)ryo_buf.turisen ) != 0 ))){			// 払出起動
				/*** 払出NG ***/
				ac_flg.cycl_fg = 73;								// （仮）払い出し完了
				ryo_en = 1;											// 領ﾎﾞﾀﾝ可
				ryo_buf.fusoku = SFV_DAT.reffal;					// 支払い不足額
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[29] );					// "払戻不足額                  円"
				opedpl( 3, 16, ryo_buf.fusoku, 6, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );
			}
			else{
				if( (CN_refund & 0x01) == 0 ){
					// ｺｲﾝの払い出しが”なし”のとき
					cn_stat( 6, 0 );									// 投入金ｸﾘｱ(ｴｽｸﾛｺｲﾝを落とす)
				}
			}
			LedReq( CN_TRAYLED, LED_ON );							// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED点灯
			break;
	}

	cn_stat( 4, 1 );												// 紙幣取込

	Lagtim( OPETCBNO, 1, 10*50 );									// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 2 );											// Timer Cancel

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case COIN_EVT:											// Coin Mech event
				ryo_en = 1;											// 領ﾎﾞﾀﾝ可
				if( OPECTL.CN_QSIG == 2 ){							// 払出完了
					ac_flg.cycl_fg = 73;							// 払い出し完了
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					if( SFV_DAT.reffal ){
						ryo_buf.fusoku += SFV_DAT.reffal;			// 支払い不足額
					}
				}
				else if( OPECTL.CN_QSIG == 9 ){						// 払出ｴﾗｰ
					ac_flg.cycl_fg = 73;							// 払い出し完了(仮)
					refalt();										// 不足分算出
					ryo_buf.fusoku += SFV_DAT.reffal;				// 支払い不足額
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				}
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
				BUZPI();
			case TIMEOUT1:											// Time Over ?
				if( ryo_en == 0 ){
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
//					ryo_en = 1;	払出が10秒以上かかると抜けてしまう
					break;
				}
				if( SFV_DAT.safe_cal_do != 0 ){						// 金庫計算未終了?
					if( msg == TIMEOUT1 ){
					safe_que++;
					if( safe_que < 10 ){
						Lagtim( OPETCBNO, 1, 1*50 );				// ﾀｲﾏｰ1(1s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
						break;
					}
					ex_errlg( ERRMDL_MAIN, ERR_MAIN_NOSAFECLOPERATE, 2, 0 );	// 金庫枚数算出なしに抜けた
					}else{
						break;
					}
				}
				ret = 10;
				break;

			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
				BUZPI();
				break;

			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				if(( ryo_en != 0 )&&( SFV_DAT.safe_cal_do == 0 )){	// 払出完了で金庫計算終了なら
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					ret = 10;
				}
				break;

			default:
				break;
		}
	}
	syu_syuu();														// 修正精算完了集計
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//	Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );					// 金銭管理ログデータ作成
//	Log_regist( LOG_MONEYMANAGE_NT );								// 金銭管理ログ登録
	if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {			// 金銭管理ログデータ作成
		Log_regist( LOG_MONEYMANAGE_NT );							// 金銭管理ログ登録
	}
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
	e_inflg = 0;													// 今回の入金は「電子マネー」1:である0:ない をクリア
	e_incnt = 0;													// 精算内の「電子マネー」使用回数（含む中止）をクリア
	e_pay = 0;														// 電子マネー決済額をクリア
	e_zandaka = 0;													// 電子マネー残高をクリア

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( Led_status[0] || Led_status[1] || Led_status [2] )			// LEDのどれかが点灯または点滅していた場合
//		EDY_LED_OFF();												// LED消去
//	if( Edy_Rec.edy_status.BIT.CTRL )								// Edyの状態がｶｰﾄﾞｾﾝｽ可能状態なら
//		Edy_SndData04();											// ｶｰﾄﾞ検知停止指示送信
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	return( ret );
}

#endif		// SYUSEI_PAYMENT


#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| 定期券更新 券挿入待ち処理                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod21( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret    :  1 = 他処理へ切換                              |*/
/*|                       : -1 = ﾓｰﾄﾞ切替                                  |*/
/*|                       : 10 = 取消(中止)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	op_mod21( void )
{
	short	ret;
	short	ans;
	ushort	msg = 0;
	char	setwk;

	OpeLcd( 20 );													// 定期券更新 券挿入待ち画面表示

	OPECTL.multi_lk = 0L;											// ﾏﾙﾁ精算問合せ中車室№
	OPECTL.ChkPassSyu = 0;											// 定期券問合せ中券種ｸﾘｱ
	OPECTL.ChkPassPkno = 0L;										// 定期券問合せ中駐車場№ｸﾘｱ
	OPECTL.ChkPassID = 0L;											// 定期券問合せ中定期券IDｸﾘｱ
	OPECTL.PassNearEnd = 0;											// 期限切れ間近ﾌﾗｸﾞ
	OPECTL.PriUsed = 0;
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

	OPE_red = 3;													// 精算中
	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
	}
	read_sht_opn();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅

	Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}else{
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
				}
				ret = -1;
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
				BUZPI();
				break;
			case KEY_TEN_CL:										// ﾃﾝｷｰ[更新] ｲﾍﾞﾝﾄ
				BUZPI();
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				break;
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				ret = 10;											// 待機へ戻る
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				memset( &PayData, 0, sizeof( Receipt_data ) );		// 精算情報,領収証印字ﾃﾞｰﾀ
// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
				memset( &ntNet_56_SaveData, 0, sizeof(ntNet_56_SaveData) );	// 精算データ保持
// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
				memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );		// NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ
				memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );	// 精算ﾃﾞｰﾀ割引情報作成ｴﾘｱｸﾘｱ
				memset( card_use, 0, sizeof( card_use ) );			// 1精算ｶｰﾄﾞ毎使用枚数ﾃｰﾌﾞﾙ
				CardUseSyu = 0;										// 1精算の割引種類の件数ｸﾘｱ
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				if( ret = al_card_renewal( msg , 0 ) ){				// ｶｰﾄﾞ判定
					OPECTL.Ope_mod = 22;							// 定期券更新 料金表示,入金処理へ
					set_tim2( &CLK_REC );							// 精算時刻ｾｯﾄ
				}
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				// ｴﾗｰ表示削除
				grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );					// "                              "
				OPECTL.PassNearEnd = 0;								// 期限切れ間近
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
				ret = 10;
				break;

			case MID_RECI_SW_TIMOUT:								// 領収証発行可能ﾀｲﾏｰﾀｲﾑｱｳﾄ通知
				OPECTL.RECI_SW = 0;									// 領収書ﾎﾞﾀﾝ未使用
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券更新 料金表示,入金処理                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod22( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret(OPECTL.Fin_mod)                                     |*/
/*|                  1 = 精算完了(釣り無し) 精算完了処理へ切換             |*/
/*|                  2 = 精算完了(釣り有り) 精算完了処理へ切換             |*/
/*|                  3 = ﾒｯｸｴﾗｰ発生         精算完了処理へ切換             |*/
/*|                 10 = 精算中止           待機へ戻る                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	op_mod22( void )
{
	short	ret;
	short	r_zero = -1;
	ushort	msg = 0;
	short	err = 0;
	ushort	loklen;

	ac_flg.cycl_fg = 1;												// 1:料金表示中
	OPECTL.op_faz = 0;												// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ
	OPECTL.RECI_SW = 0;												// 領収書ﾎﾞﾀﾝ未使用
	RECI_SW_Tim = 0;												// 領収証受付時間ｸﾘｱ
	OPECTL.CAN_SW = 0;												// 取消ﾎﾞﾀﾝ
	OPECTL.LastUsePCardBal = 0L;									// 最後に利用したﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞの残額
	OPECTL.PriUsed = 0;
	ryodsp = 0;
	OPECTL.f_KanSyuu_Cmp = 0;										// 完了集計未実施
	OpeNtnetAddedInfo.PayMethod = 5;								// 精算方法=5:定期券更新
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;							// 未了タイムアウトフラグクリア
	Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;
	Suica_Rec.Data.BIT.PAY_CTRL = 0;								// 電子媒体決済済み判定ﾌﾗｸﾞｸﾘｱ※念のため
	Suica_rcv_split_flag_clear();

	cm27();															// ﾌﾗｸﾞｸﾘｱ
	memset( &ryo_buf, 0, sizeof( struct RYO_BUF ) );				// 料金ﾊﾞｯﾌｧｸﾘｱｰ
	ryo_buf.pkiti = 9999;											// 車室№はﾀﾞﾐｰとして9999を入れておく
	ryo_buf.tyu_ryo = 
	ryo_buf.dsp_ryo = RenewalFee;									// 更新料金

	if( ryo_buf.nyukin ){
		OpeLcd( 4 );												// 料金表示(入金又は割引後)
	}else{
		OpeLcd( 21 );												// 定期券更新 料金表示(初回)
	}
	if( ryo_buf.dsp_ryo == 0 ){										// 表示料金0円
		r_zero = 0;													// 0円精算
		Lagtim( OPETCBNO, 1, 3*50 );								// ﾀｲﾏｰ1(3s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
		OPE_red = 4;												// 精算完了
	}else{
		r_zero = -1;												// 0円以外精算
				cn_stat( 1, 2 );									// 紙幣 & COIN入金可
		nmicler( &SFV_DAT.refval, 8 );								// 不足額ｸﾘｱ

		rd_shutter();												// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
		Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][65]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	}

	OPECTL.coin_syukei = 0;											// ｺｲﾝ金庫集計未出力とする
	OPECTL.note_syukei = 0;											// 紙幣金庫集計未出力とする

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
				BUZPI();
				if( OPECTL.RECI_SW == 0 ){							// 領収書ﾎﾞﾀﾝ未使用?
					if(( OPECTL.ChkPassSyu == 0 )&&					// 問合せ中ではない?
					   ( OPECTL.op_faz != 0 || ryodsp )){			// 待機状態以外?
						wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
						grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "      領収証を発行します      "
					}
					OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
				}
				break;

			case COIN_EVT:											// Coin Mech event
				if( OPECTL.op_faz == 4 ){							// ﾀｲﾑｱｳﾄ
					if( OPECTL.CN_QSIG == 7 ){ 						// 払出可
						ret = 10;									// 待機に戻る
						break;
					}else{
						OPECTL.op_faz = 0;
						cn_stat( 3, 2 );							// 入金可
					}
				}
			case NOTE_EVT:											// Note Reader event
				if(( r_zero == 0 )&&( OPECTL.cnsend_flg == 1 )){	// 0円精算＆入金可能状態
					r_zero = -1;
					OPECTL.op_faz = 1;								// 入金中
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				}
			case TIMEOUT8:											// 入金後戻り判定ﾀｲﾏｰﾀｲﾑｱｳﾄ
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				if( msg == KEY_TEN_F4 ){							// 取消ﾎﾞﾀﾝ?(Y)
					BUZPI();
					ope_anm( AVM_STOP );							// 放送停止ｱﾅｳﾝｽ
				}
				if( msg == TIMEOUT8 ){								// 入金後戻り判定ﾀｲﾏｰﾀｲﾑｱｳﾄ?
					msg = KEY_TEN_F4;								// 自動的に精算中止とする
				}
				if( r_zero == 0 ){
					break;
				}
				if(( OPECTL.op_faz == 0 )&&( msg != KEY_TEN_F4 )){
					OPECTL.op_faz = 1;								// 入金中
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				}

				/*** 入金後XXsで精算中止とする ***/
				Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ﾀｲﾏｰ8(XXs)起動(入金後戻り判定ﾀｲﾏｰ)
				ret = in_mony( msg, 0 );							// 入金処理ｻﾌﾞ
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
			case ARC_CR_E_EVT:										// 終了ｺﾏﾝﾄﾞ受信
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				break;

			case TIMEOUT1:
				if( r_zero == 0 ){									// 0円精算
					cn_stat( 2, 2 );								// 入金不可
					ret = 1;										// 精算完了処理へ
					break;
				}
				if( OPECTL.op_faz >= 3 ){
					ret = 10;										// 待機に戻る
					break;
				}
				cn_stat( 2, 2 );									// 入金不可
				OPECTL.op_faz = 4;									// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ ﾀｲﾑｱｳﾄ
				Lagcan( OPETCBNO, 1 );								// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				Lagtim( OPETCBNO, 2, 10*50 );						// ﾒｯｸ監視ﾀｲﾏｰ起動
				break;

			case TIMEOUT2:											// ﾒｯｸ監視ﾀｲﾏｰﾀｲﾑｱｳﾄ
				switch( OPECTL.op_faz ){
					case 2:											// 完了
						if( ryo_buf.turisen ){						//
							SFV_DAT.reffal = ryo_buf.turisen;		//
							ret = 3;								// 精算完了処理へ
						}else{
							ret = 1;								// 精算完了処理へ
						}
						break;
					case 3:											// 中止
					case 4:											// ﾀｲﾑｱｳﾄ
						if( ryo_buf.nyukin ){						//
							ryo_buf.turisen = ryo_buf.nyukin;		//
							if( SFV_DAT.reffal ){					// 不足あり?
								ryo_buf.fusoku += SFV_DAT.reffal;	// 支払い不足額
							}
							if( SFV_DAT.nt_escrow ){				// 紙幣戻しｴﾗｰ
								ryo_buf.fusoku += 1000L;			// 支払い不足額
							}
							chu_isu();								// 中止集計
							Lagtim( OPETCBNO, 1, 10*50 );			// ﾀｲﾏｰ1起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
						}else{
							if( carduse() ){						// ｶｰﾄﾞ使用?
								chu_isu();							// 中止処理
								svs_renzoku();						// ｻｰﾋﾞｽ券連続挿入
							}
							ret = 10;								// 待機に戻る
						}
						break;
					default:
						ret = 10;									// 待機に戻る
						break;
				}
				break;

			default:
				break;
		}
		if( ret == 10 ){											// 取消(中止)?
			_di();
			CN_escr_timer = 0; 										// ｴｽｸﾛﾀｲﾏｰｷｬﾝｾﾙ
			_ei();
			safecl( 7 );											// 金庫枚数算出
		}
	}

	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 2 );											// ﾀｲﾏｰ2ﾘｾｯﾄ(ﾒｯｸ監視用)
	Lagcan( OPETCBNO, 8 );											// ﾀｲﾏｰ8ﾘｾｯﾄ(入金後戻り判定ﾀｲﾏｰ)

	OPECTL.Fin_mod = ret;
	if( ret != 10 ){
		OPECTL.Ope_mod = 23;										// 定期券更新 精算完了処理へ
	}
	else{
		cn_stat( 8, 0 );	// 保有枚数送信
		read_sht_opn();		// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
		opr_snd( 13 );		// 券取出口移動
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//		Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );	// 中止時の金銭管理ログデータ作成
//		Log_regist( LOG_MONEYMANAGE_NT );							// 金銭管理ログ登録
		if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {	// 中止時の金銭管理ログデータ作成
			Log_regist( LOG_MONEYMANAGE_NT );						// 金銭管理ログ登録
		}
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 定期券更新 精算完了                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod23( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = 正常完了                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	op_mod23( void )
{
	short	ret;
	short	ryo_en;
	short	safe_que = 0;
	ushort	msg = 0;
	short	ret_bak = 0;
	ushort	wk_MsgNo;

	read_sht_opn();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅

	al_mkpas_renewal( (m_apspas *)MAGred, &CLK_REC );

	if( CPrmSS[S_TIK][10] == 0 ){									// 定期券ﾍﾞﾘﾌｧｲしない設定?
		opr_snd( 16 );												// 定期券ﾃﾞｰﾀﾗｲﾄ(ﾍﾞﾘﾌｧｲ無し)
	}else{
		opr_snd( 9 );												// 定期券ﾃﾞｰﾀﾗｲﾄ(ﾍﾞﾘﾌｧｲ有り)
	}


	if( prm_get( COM_PRM,S_PAY,25,1,2 ) && !LprnErrChk( 1 ) ){		// ﾗﾍﾞﾙ設定ありでﾗﾍﾞﾙ関連ｴﾗｰなし時のみﾗﾍﾞﾙ発行する
		LedReq( LB_TRAYLED, LED_ON );								// ﾗﾍﾞﾙ取出し口ｶﾞｲﾄﾞLED点灯

		PayData.TOutTime.Year = car_ot_f.year;						// 出庫 年
		PayData.TOutTime.Mon  = car_ot_f.mon;						//      月
		PayData.TOutTime.Day  = car_ot_f.day;						//      日

		OpLpr_snd( &PayData, 0 );									// ﾗﾍﾞﾙ出力
		LabelCountDown();											// ﾗﾍﾞﾙ発行枚数減算
	}

	ryo_en = 0;														// 領ﾎﾞﾀﾝ不可
	switch( OPECTL.Fin_mod ){
		case 1:														// 釣り無し?
		default:
			ac_flg.cycl_fg = 13;									// 払い出し完了(仮)
			cn_stat( 6, 0 );										// 投入金ｸﾘｱ(ｴｽｸﾛｺｲﾝを落とす)
			ryo_en = 1;												// 領ﾎﾞﾀﾝ可
			if( OPECTL.RECI_SW == 1 ){								// 領収書ﾎﾞﾀﾝ使用?
				ryo_isu( 0 );										// 領収書(釣銭不足無し)発行
			}
			break;

		case 2:														// 釣り有り?
			if( refund( (long)ryo_buf.turisen ) != 0 ){				// 払出起動
				/*** 払出NG ***/
				ac_flg.cycl_fg = 13;								// 払い出し完了(仮)
				ryo_en = 1;											// 領ﾎﾞﾀﾝ可
				ryo_buf.fusoku = SFV_DAT.reffal;					// 支払い不足額
				set_escrowtimer();									// ｴｽｸﾛﾀｲﾏｰｾｯﾄ
				Print_Condition_select();							// 領収書(釣銭不足有り) or 預り証発行
			}else{
				if( (CN_refund & 0x01) == 0 ){
					// ｺｲﾝの払い出しが”なし”のとき
					cn_stat( 6, 0 );									// 投入金ｸﾘｱ(ｴｽｸﾛｺｲﾝを落とす)
				}
			}
			LedReq( CN_TRAYLED, LED_ON );							// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED点灯
			break;

		case 3:														// ﾒｯｸｴﾗｰ(釣り有り)?
			ac_flg.cycl_fg = 13;									// 払い出し完了(仮)
			ryo_en = 1;												// 領ﾎﾞﾀﾝ可
			ryo_buf.fusoku = SFV_DAT.reffal;						// 支払い不足額
			set_escrowtimer();										// ｴｽｸﾛﾀｲﾏｰｾｯﾄ
			Print_Condition_select();								// 領収書(釣銭不足有り) or 預り証発行
			break;
	}

	OpeLcd( 23 );													// 定期券更新 精算完了表示

	cn_stat( 4, 1 );												// 紙幣取込

	Lagtim( OPETCBNO, 1, 10*50 );									// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 2 );											// Timer Cancel

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case COIN_EVT:											// Coin Mech event
				ryo_en = 1;											// 領ﾎﾞﾀﾝ可
				if( OPECTL.CN_QSIG == 2 ){							// 払出完了
					ac_flg.cycl_fg = 13;							// 払い出し完了
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					if( SFV_DAT.reffal ){
						ryo_buf.fusoku += SFV_DAT.reffal;			// 支払い不足額
						Print_Condition_select();					// 領収書(釣銭不足有り) or 預り証発行
					}
					else if( OPECTL.RECI_SW ){						// 領収書ﾎﾞﾀﾝ状態
						ryo_isu( 0 );								// 領収書(釣銭不足無し)発行
					    OPECTL.RECI_SW = 1;							// 領収書ﾎﾞﾀﾝ使用
					}
				}
				else if( OPECTL.CN_QSIG == 9 ){						// 払出ｴﾗｰ
					ac_flg.cycl_fg = 13;							// 払い出し完了(仮)
					refalt();										// 不足分算出
					ryo_buf.fusoku += SFV_DAT.reffal;				// 支払い不足額
					Print_Condition_select();						// 領収書(釣銭不足有り) or 預り証発行
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					set_escrowtimer();								// ｴｽｸﾛﾀｲﾏｰｾｯﾄ
				}
				if( ret_bak ) ret = ret_bak;
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
			case KEY_TEN_CL:										// ﾃﾝｷｰ[区画] ｲﾍﾞﾝﾄ
				BUZPI();
			case TIMEOUT1:											// Time Over ?
				if( ryo_en == 0 ){
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					break;
				}
				if( SFV_DAT.safe_cal_do != 0 ){						// 金庫計算未終了?
					if( msg == TIMEOUT1 ){
					safe_que++;
					if( safe_que < 10 ){
						Lagtim( OPETCBNO, 1, 1*50 );				// ﾀｲﾏｰ1(1s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
						break;
					}
					ex_errlg( ERRMDL_MAIN, ERR_MAIN_NOSAFECLOPERATE, 2, 0 );	// 金庫枚数算出なしに抜けた
					}else{
						break;
					}
				}
				ret = 10;
				break;

			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
				BUZPI();
				if( OPECTL.RECI_SW == 0 ){							// 領収書ﾎﾞﾀﾝ未使用?
					OPECTL.RECI_SW = 1;								// 領収書ﾎﾞﾀﾝ使用
					wk_MsgNo = Ope_Disp_Receipt_GetMsgNo();
					grachr(7, 0, 30, 0, COLOR_CRIMSON, LCD_BLINK_OFF, OPE_CHR[wk_MsgNo]);	// "     領収証をお取り下さい     "
					if( ryo_en == 0 ){
						break;
					}
					ryo_isu( 0 );									// 領収書(釣銭不足無し)発行
					Lagtim( OPETCBNO, 1, 10*50 );					// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					OPECTL.PassNearEnd = 0;							// 期限切れ間近
				}
				break;

			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				if(( ryo_en != 0 )&&( SFV_DAT.safe_cal_do == 0 )){	// 払出完了で金庫計算終了なら
					Lagcan( OPETCBNO, 1 );							// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
					ret = 10;
				}
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				OPECTL.PassNearEnd = 0;								// 期限切れ間近
				break;

			case IBK_MIF_A2_NG_EVT:									// Mifareﾃﾞｰﾀ読出しNG
				if( MIF_ENDSTS.sts1 == 0x30 ){						// ｶｰﾄﾞ無し
					OPECTL.PassNearEnd = 0;							// 期限切れ間近ﾌﾗｸﾞ
				}
				break;

			case IBK_LPR_B1_REC_NG:									// ﾗﾍﾞﾙﾌﾟﾘﾝﾄ失敗
				grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR2[8] );					// "ラベルの発行が出来ませんでした"反転表示
				// no break
			case IBK_LPR_B1_REC_OK:									// ﾗﾍﾞﾙﾌﾟﾘﾝﾄ成功
				break;

			default:
				break;
		}
		// 釣銭の払出しが未完了で抜けようとしたら、払出しが完了するまで待つ
		if(( ryo_en == 0 )&&( ret )){								// 払出未完了?
			ret_bak = ret;											// Yes..Save
			ret = 0;
		}
	}
	if( OPECTL.RECI_SW == 1 ){										// 領収書ﾎﾞﾀﾝ使用?
		OPECTL.RECI_SW = 0;											// 領収書ﾎﾞﾀﾝ未使用
	}else{
		PayData_set( 0, 0 );										// 1精算情報ｾｯﾄ
		kan_syuu();													// 精算完了集計
		OPECTL.RECI_SW = -1;										// 待機中領収証ﾎﾞﾀﾝ可
	}
	cn_stat( 8, 0 );	// 保有枚数送信
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//	Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );					// 金銭管理ログデータ作成
//	Log_regist( LOG_MONEYMANAGE_NT );								// 金銭管理ログ登録
	if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {			// 金銭管理ログデータ作成
		Log_regist( LOG_MONEYMANAGE_NT );							// 金銭管理ログ登録
	}
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)

	return( ret );
}
#endif	// UPDATE_A_PASS

/*[]----------------------------------------------------------------------[]*/
/*| 駐車位置の状態ﾁｪｯｸ                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : carchk( snum, pnum, paymod )                            |*/
/*| PARAMETER    : snum   : 区画                                           |*/
/*|                pnum   : 接客用駐車位置番号                             |*/
/*|                paymod : ﾓｰﾄﾞ 0=通常 1=修正                             |*/
/*|                		  :      2=受付券発行時 3=暗証番号登録時           |*/
/*| RETURN VALUE : ret    :  0 = OK                                        |*/
/*|                       :  1 = ﾛｯｸ装置閉開始待ち,ﾛｯｸ装置閉待ち           |*/
/*|                       :  2 = 出庫状態                                  |*/
/*|                       :  3 = 休業状態(ｺｲﾝﾒｯｸ、紙幣ﾘｰﾀﾞｰ通信異常)       |*/
/*|                       :  4 = ラグタイム中                              |*/
/*|                       : 10 = NG(ﾛｯｸ装置状態が規定外等)                 |*/
/*|                       : -1 = NG(未設定等により本精算機では精算不可能)  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	carchk( uchar snum, ushort pnum, uchar paymod )
{
	short	ret;
	ushort	posi;

	ret = -1;
	OPECTL.InLagTimeMode = OFF;										// ラグタイム延長モードＯＦＦ

	if(( snum > 26 )||
	   (( pnum < 1 )||( pnum > 9999 ))){							// 区画,駐車位置番号が対象外?
		return( 10 );
	}

	if( LKopeGetLockNum( snum, pnum, &posi ) ){
		OPECTL.Op_LokNo = (ulong)(( snum * 10000L ) + pnum );		// 接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)
		OPECTL.Pr_LokNo = posi;										// 内部処理用駐車位置番号(1～324)
		pnum = posi-1;
		ret = 10;
	}

	if( ret == 10 ){
		for( ; ; ){
			if( FlpSetChk( pnum ) != 1 ){							// Flap Settei Check
				break;
			}

			if( paymod == 1 ){										// 修正精算?
				if( syusei[pnum].sei == 1 ){
					ret = 0;
				}
				break;
			}
			else if(paymod / 10)									// 10の桁が0以外(駐車証明書発行有効)
			{
				paymod = (uchar)(paymod % 10);						// 1の桁のみ残す
			}

			switch( FLAPDT.flp_data[pnum].mode ){
				case FLAP_CTRL_MODE2:								// ﾛｯｸ装置閉開始待ち
				case FLAP_CTRL_MODE3:								// ﾛｯｸ装置閉完了待ち
					ret = 1;
					break;
				case FLAP_CTRL_MODE4:								// 駐車中
					ret = 0;
					break;
				case FLAP_CTRL_MODE1:								// 待機状態
				case FLAP_CTRL_MODE5:								// ﾛｯｸ装置開完了待ち
				case FLAP_CTRL_MODE6:								// 出庫待ち
					if (Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[pnum].ryo_syu, 1, 4)) {		// ラグタイム延長処理あり
						if ((FLAPDT.flp_data[pnum].mode == FLAP_CTRL_MODE5 || FLAPDT.flp_data[pnum].mode == FLAP_CTRL_MODE6) &&	// ロック装置開完了待ち or 出庫待ち中
							(FLAPDT.flp_data[pnum].timer > 0) &&																// ラグタイマー動作中？
							(OPECTL.Ope_mod == 1) &&																			// 駐車位置番号入力処理中
							(paymod < 2)) {																						// テンキー[精算] イベント(一応修正精算時も可能にする)
							if (!prm_get(COM_PRM, S_TYP, 80, 1, 2) ||		// ラグタイム延長制限なし
								!prm_get(COM_PRM, S_TYP, 80, 1, 1) ||		// ラグタイム延長判定なし
								Flp_LagExtCnt[pnum] < prm_get(COM_PRM, S_TYP, 80, 1, 1)) {
																			// 制限回数内
								OPECTL.InLagTimeMode = ON;					// ラグタイム延長モードＯＮ
								ret = 0;
								break;
							} else {
								if (prm_get(COM_PRM, S_DSP, 20, 1, 1)) {	// ラグタイム中車室の接客案内する
									ret = 4;
									break;
								}
							}
						}
					} else {
						if ((prm_get(COM_PRM, S_DSP, 20, 1, 1)) &&			// ラグタイム中車室の接客案内する
							(FLAPDT.flp_data[pnum].mode == FLAP_CTRL_MODE5 || FLAPDT.flp_data[pnum].mode == FLAP_CTRL_MODE6) &&	// ロック装置開完了待ち or 出庫待ち中
							(FLAPDT.flp_data[pnum].timer > 0) &&																// ラグタイマー動作中？
							(OPECTL.Ope_mod == 1) &&																			// 駐車位置番号入力処理中
							(paymod < 2)) {																						// テンキー[精算] イベント(一応修正精算時も可能にする)
							ret = 4;
							break;
						}
					}
					ret = 2;
					break;
				default:
					ex_errlg( ERRMDL_MAIN, ERR_MAIN_LOCKSTATUSNG, 2, 0 );	// ﾛｯｸ装置状態規定外
					break;
			}
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 車室番号からﾌﾗｯﾊﾟ設定をﾁｪｯｸする処理                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FlpSetChk( pnum )                                       |*/
/*| PARAMETER    : num   : 車室№-1                                        |*/
/*| RETURN VALUE : ret   : 0 = 設定無し 1 = 設定有り                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	FlpSetChk( ushort pnum )
{
	if(( LockInfo[pnum].lok_syu == 0 )||							// ﾒｰｶｰ設定無し
	   ( LockInfo[pnum].ryo_syu == 0 )){							// 料金種別無し
		return( 0 );												// 有効設定無し
	}
	return( 1 );													// 有効設定有り
}

/*[]----------------------------------------------------------------------[]*/
/*| 駐車位置の状態ﾁｪｯｸ                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : carchk_ml( snum, pnum )                                 |*/
/*| PARAMETER    : snum   : 区画                                           |*/
/*|                pnum   : 接客用駐車位置番号                             |*/
/*| RETURN VALUE : ret    :  0 = OK                                        |*/
/*|                       :  1 = ﾛｯｸ装置閉開始待ち,ﾛｯｸ装置閉待ち           |*/
/*|                       :  2 = 出庫状態                                  |*/
/*|                       :  3 = 休業状態(ｺｲﾝﾒｯｸ、紙幣ﾘｰﾀﾞｰ通信異常)       |*/
/*|                       : 10 = NG(ﾛｯｸ装置状態が規定外等)                 |*/
/*|                       : -1 = NG(未設定等により本精算機では精算不可能)  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	carchk_ml( uchar snum, ushort pnum )
{
	short	ret;

	ret = -1;

	if(( snum > 26 )||
	   (( pnum < 1 )||( pnum > 9999 ))){							// 区画,駐車位置番号が対象外?
		return( 10 );
	}

	for( ; ; ){
		if( prm_get( COM_PRM,S_SHA,(short)(1+6*(LOCKMULTI.lock_mlt.ryo_syu-1)),2,5 ) == 0 ){
			// 料金計算する種別が設定されていない。
			break;
		}
		if( LOCKMULTI.lock_mlt.bk_syu ){
			if( prm_get( COM_PRM,S_SHA,(short)(1+6*(LOCKMULTI.lock_mlt.bk_syu-1)),2,5 ) == 0 ){
				// 料金計算する種別が設定されていない。
				break;
			}
		}

		switch( LOCKMULTI.lock_mlt.mode ){
			case 1:												// ﾛｯｸ装置閉開始待ち
			case 2:												// ﾛｯｸ装置閉完了待ち
				ret = 1;
				break;
			case 3:												// 駐車中
				ret = 0;
				break;
			case 0:												// 待機状態
			case 4:												// ﾛｯｸ装置開完了待ち
			case 5:												// 出庫待ち
				ret = 2;
				break;
			default:
				ex_errlg( ERRMDL_MAIN, ERR_MAIN_LOCKSTATUSNG, 2, 0 );	// ﾛｯｸ装置状態規定外
				break;
		}
		break;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 入出庫時刻ｾｯﾄ                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : set_tim( num, clk, err_mode )                           |*/
/*| PARAMETER    : num      : 車室№                                       |*/
/*|              : clk      : 現在時刻                                     |*/
/*|              : err_mode : ｴﾗｰ有無 0:無し / 1:有り                      |*/
/*| RETURN VALUE : ret      : 入出庫時刻判定(1980～2079年以内か?)0:OK 1:NG |*/
/*[]----------------------------------------------------------------------[]*/
/*| car_in, car_ot, car_in_f, car_ot_fに入出庫時刻をセットする             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	set_tim( ushort num, struct clk_rec *clk_para, short err_mode )
{
	flp_com	*frp;
	ushort	in_tim, out_tim;

	if( OPECTL.multi_lk ){											// ﾏﾙﾁ精算問合せ中?
		frp = &LOCKMULTI.lock_mlt;
	}else{
		frp = &FLAPDT.flp_data[num-1];
	}

	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					// 入車時刻ｸﾘｱ
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// 出車時刻ｸﾘｱ
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// 入車時刻Fixｸﾘｱ
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// 出車時刻Fixｸﾘｱ
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// 再計算用入車時刻ｸﾘｱ

	car_in.year = frp->year;										// 入車 年
	car_in.mon  = frp->mont;										//      月
	car_in.day  = frp->date;										//      日
	car_in.hour = frp->hour;										//      時
	car_in.min  = frp->minu;										//      分
	car_in.week = (char)youbiget( car_in.year,						//      曜日
								(short)car_in.mon,
								(short)car_in.day );
// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
	// システムID16：振替精算？
	if( PiP_GetFurikaeSts() ){
		// 入庫時刻保持
		g_PipCtrl.stFurikaeInfo.DestInTime.Year		= car_in.year;	// Year
		g_PipCtrl.stFurikaeInfo.DestInTime.Mon		= car_in.mon;	// Month
		g_PipCtrl.stFurikaeInfo.DestInTime.Day		= car_in.day;	// Day
		g_PipCtrl.stFurikaeInfo.DestInTime.Hour		= car_in.hour;	// Hour
		g_PipCtrl.stFurikaeInfo.DestInTime.Min		= car_in.min;	// Minute
	}
// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算

	car_ot.year = clk_para->year;									// 出庫 年
	car_ot.mon  = clk_para->mont;									//      月
	car_ot.day  = clk_para->date;									//      日
	car_ot.hour = clk_para->hour;									//      時
	car_ot.min  = clk_para->minu;									//      分
	car_ot.week = (char)youbiget( car_ot.year,						//      曜日
								(short)car_ot.mon,
								(short)car_ot.day );
// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
	// システムID16：振替精算？
	if( PiP_GetFurikaeSts() ){
		// 料金計算時刻保持
		g_PipCtrl.stFurikaeInfo.DestPayTime.Year	= clk_para->year;	// Year
		g_PipCtrl.stFurikaeInfo.DestPayTime.Mon		= clk_para->mont;	// Month
		g_PipCtrl.stFurikaeInfo.DestPayTime.Day		= clk_para->date;	// Day
		g_PipCtrl.stFurikaeInfo.DestPayTime.Hour	= clk_para->hour;	// Hour
		g_PipCtrl.stFurikaeInfo.DestPayTime.Min		= clk_para->minu;	// Minute
		g_PipCtrl.stFurikaeInfo.DestPayTime.Sec		= clk_para->seco;	// Second
	}
// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算

	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// 入車時刻Fix
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			// 出車時刻Fix
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		// 再計算用入車時刻Fix

	if( err_mode == 0 ){
		return( 0 );
	}

	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// 入庫時刻規定外?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// 出庫時刻規定外?
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTTIMENG, 2, 0 );		// 入出庫時刻規定外
		return( 1 );
	}
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					// (入庫日+1年)normlize
					(short)car_in.mon,
					(short)car_in.day );
	out_tim = dnrmlzm( car_ot.year,									// 出庫日normlize
					(short)car_ot.mon,
					(short)car_ot.day );
	if( in_tim <= out_tim ){
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_OVER1YEARCAL, 2, 0 );		// 1年以上の料金計算を行った
	}
	in_tim = dnrmlzm((short)( car_in.year ),						/* (入庫日)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){											/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTGYAKU, 2, 1 );			/* 入出庫逆転					*/
		return( 1 );												/*								*/
	}																/*								*/
	return( 0 );
}

#if	UPDATE_A_PASS
/*[]----------------------------------------------------------------------[]*/
/*| 精算時刻(入出庫時刻)ｾｯﾄ                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : set_tim2(  clk_para )                                   |*/
/*| PARAMETER    : clk_para : 現在時刻                                     |*/
/*| RETURN VALUE : ret      : 入出庫時刻判定(1980～2079年以内か?)0:OK 1:NG |*/
/*[]----------------------------------------------------------------------[]*/
/*| car_in, car_ot, car_in_f, car_ot_fに入出庫時刻をセットする             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2006-01-25                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	set_tim2( struct clk_rec *clk_para )
{
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					// 入車時刻ｸﾘｱ
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// 出車時刻ｸﾘｱ
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// 入車時刻Fixｸﾘｱ
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// 出車時刻Fixｸﾘｱ
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// 再計算用入車時刻ｸﾘｱ

	car_in.year = clk_para->year;									// 入車 年
	car_in.mon  = clk_para->mont;									//      月
	car_in.day  = clk_para->date;									//      日
	car_in.hour = clk_para->hour;									//      時
	car_in.min  = clk_para->minu;									//      分
	car_in.week = (char)youbiget( car_in.year,						//      曜日
								(short)car_in.mon,
								(short)car_in.day );

	memcpy( &car_ot, &car_in, sizeof( struct CAR_TIM ) );			// 出車時刻
	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// 入車時刻Fix
	memcpy( &car_ot_f, &car_in, sizeof( struct CAR_TIM ) );			// 出車時刻Fix
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		// 再計算用入車時刻Fix

	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// 入庫時刻規定外?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// 出庫時刻規定外?
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTTIMENG, 2, 0 );		// 入出庫時刻規定外
		return( 1 );
	}
	return( 0 );
}
#endif	// UPDATE_A_PASS
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : set_tim( num, clk, err_mode )                           |*/
/*| PARAMETER    : trig     : ﾄﾘｶﾞｰ                                       |*/
/*|              : num      : 車室№                                       |*/
/*|              : clk      : 現在時刻                                     |*/
/*|              : err_mode : ｴﾗｰ有無 0:無し / 1:有り                      |*/
/*| RETURN VALUE : ret      : 入出庫時刻判定(1980～2079年以内か?)0:OK 1:NG |*/
/*[]----------------------------------------------------------------------[]*/
/*| car_in, car_ot, car_in_f, car_ot_fに入出庫時刻をセットする             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	set_tim_Lost( ushort trig, ushort num, struct clk_rec *clk_para, short err_mode )
{
	flp_com	*frp;

	if( OPECTL.multi_lk ){											// ﾏﾙﾁ精算問合せ中?
		frp = &LOCKMULTI.lock_mlt;
	}else{
		frp = &FLAPDT.flp_data[num-1];
	}
	// クリア
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					// 入車時刻ｸﾘｱ
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// 出車時刻ｸﾘｱ
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// 入車時刻Fixｸﾘｱ
	NTNET_Data152Save( (void *)&car_in_f, NTNET_152_CARINTIME );
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// 出車時刻Fixｸﾘｱ
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// 再計算用入車時刻ｸﾘｱ

	// 金額指定遠隔精算？
	if( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE ){
		// 入庫時刻がある？
		if( frp->mode > FLAP_CTRL_MODE3 ){
			car_in.year = frp->year;								// 入車 年
			car_in.mon  = frp->mont;								//      月
			car_in.day  = frp->date;								//      日
			car_in.hour = frp->hour;								//      時
			car_in.min  = frp->minu;								//      分
			car_in.week = (char)youbiget( car_in.year,				//      曜日
										(short)car_in.mon,
										(short)car_in.day );
		}
		// 入庫時刻なし？
		else{
			// 精算時刻をセットする
			car_in.year = clk_para->year;							// 入車 年
			car_in.mon  = clk_para->mont;							//      月
			car_in.day  = clk_para->date;							//      日
			car_in.hour = clk_para->hour;							//      時
			car_in.min  = clk_para->minu;							//      分
			car_in.week = (char)youbiget( car_in.year,				//      曜日
										(short)car_in.mon,
										(short)car_in.day );
		}
	}

	ryo_buf.fumei_fg = 2;											// 入車時刻不明情報set
	// 紛失券or紛失ボタン
	ryo_buf.fumei_reason = fumei_LOST;								// 不明詳細理由:紛失精算

	car_ot.year = clk_para->year;									// 出庫 年
	car_ot.mon  = clk_para->mont;									//      月
	car_ot.day  = clk_para->date;									//      日
	car_ot.hour = clk_para->hour;									//      時
	car_ot.min  = clk_para->minu;									//      分
	car_ot.week = (char)youbiget( car_ot.year,						//      曜日
								(short)car_ot.mon,
								(short)car_ot.day );

	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// 入車時刻Fix
	NTNET_Data152Save( (void *)&car_in_f, NTNET_152_CARINTIME );
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			// 出車時刻Fix
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		// 再計算用入車時刻Fix

	return( 0 );
}
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)
//[]----------------------------------------------------------------------[]
///	@brief		入庫時刻強制セット済み時の入出庫時刻セット
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/03<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
short set_tim_only_out( ushort num, struct clk_rec *clk_para, short err_mode )
{
	ushort	in_tim, out_tim;
// MH810100(S) K.Onodera 2020/03/03 #3989 入庫時刻指定の遠隔精算で入場日時が表示されない
	DATE_YMDHMS paytime;
// MH810100(E) K.Onodera 2020/03/03 #3989 入庫時刻指定の遠隔精算で入場日時が表示されない

	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					// 出車時刻ｸﾘｱ
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				// 入車時刻Fixｸﾘｱ
	NTNET_Data152Save( (void *)&car_in_f, NTNET_152_CARINTIME );
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				// 出車時刻Fixｸﾘｱ
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );			// 再計算用入車時刻ｸﾘｱ

	car_ot.year = clk_para->year;									// 出庫 年
	car_ot.mon  = clk_para->mont;									//      月
	car_ot.day  = clk_para->date;									//      日
	car_ot.hour = clk_para->hour;									//      時
	car_ot.min  = clk_para->minu;									//      分
	car_ot.week = (char)youbiget( car_ot.year,						//      曜日
								(short)car_ot.mon,
								(short)car_ot.day );
	// 料金計算時刻保持
	g_PipCtrl.stRemoteTime.OutTime.Year	= clk_para->year;			// Year
	g_PipCtrl.stRemoteTime.OutTime.Mon	= clk_para->mont;			// Month
	g_PipCtrl.stRemoteTime.OutTime.Day	= clk_para->date;			// Day
	g_PipCtrl.stRemoteTime.OutTime.Hour	= clk_para->hour;			// Hour
	g_PipCtrl.stRemoteTime.OutTime.Min	= clk_para->minu;			// Minute
	g_PipCtrl.stRemoteTime.OutTime.Sec	= clk_para->seco;			// Second

// MH810100(S) K.Onodera 2020/03/03 #3989 入庫時刻指定の遠隔精算で入場日時が表示されない
	// LCDに通知するために精算時刻を保持する(残高変化通知にて送信する)
	memset( &paytime, 0, sizeof(paytime) );
	paytime.Year = car_in.year;
	paytime.Mon  = car_in.mon;
	paytime.Day  = car_in.day;
	paytime.Hour = car_in.hour;
	paytime.Min  = car_in.min;
	paytime.Sec  = (uchar)g_PipCtrl.stRemoteTime.InTime.Sec;

	SetPayTargetTime( &paytime );
// MH810100(E) K.Onodera 2020/03/03 #3989 入庫時刻指定の遠隔精算で入場日時が表示されない

	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			// 入車時刻Fix
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			// 出車時刻Fix
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );		// 再計算用入車時刻Fix

	if( err_mode == 0 ){
		return( 0 );
	}

	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// 入庫時刻規定外?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// 出庫時刻規定外?
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTTIMENG, 2, 0 );		// 入出庫時刻規定外
		return( 1 );
	}
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					// (入庫日+1年)normlize
					(short)car_in.mon,
					(short)car_in.day );
	out_tim = dnrmlzm( car_ot.year,									// 出庫日normlize
					(short)car_ot.mon,
					(short)car_ot.day );
	if( in_tim <= out_tim ){
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_OVER1YEARCAL, 2, 0 );		// 1年以上の料金計算を行った
	}
	in_tim = dnrmlzm((short)( car_in.year ),						/* (入庫日)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){											/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTGYAKU, 2, 1 );			/* 入出庫逆転					*/
		return( 1 );												/*								*/
	}																/*								*/
	return( 0 );
}

// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//[]----------------------------------------------------------------------[]
///	@brief		入出庫時刻セット
//[]----------------------------------------------------------------------[]
///	@param		err_mode : errorﾁｪｯｸの有無
///	@return		0=正常/1=異常
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2019/11/15<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static short set_tim_only_out_card( short err_mode )
{
	ushort	in_tim, out_tim;
	stDatetTimeYtoSec_t* pExitDate = NULL;
	stDateParkTime_t* pExit = NULL;
	DATE_YMDHMS paytime;
	struct clk_rec *pTime;

	// 出庫時刻
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );
	memset( &car_ot_f, 0, sizeof( car_ot_f ) );
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）

	// 入庫時刻
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );
	memset( &car_in_f, 0, sizeof( car_in_f ) );
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）

	NTNET_Data152Save( (void *)&car_in_f, NTNET_152_CARINTIME );

	// 再計算用入車時刻
	memset( &recalc_carin, 0, sizeof( struct CAR_TIM ) );

	// 未払精算チェック
	pExit = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime;

	// 未精算出庫精算？
	if( lcdbm_rsp_in_car_info_main.shubetsu == 1 ){
		if( pExit->ParkingNo && pExit->MachineKind && 
			pExit->dtTimeYtoSec.shYear && pExit->dtTimeYtoSec.byMonth && pExit->dtTimeYtoSec.byDay ){
			pExitDate = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtExitDateTime.dtTimeYtoSec;
			// 出庫時刻情報 <- 在車情報_出庫時刻
			car_ot.year = pExitDate->shYear;		// 出庫_年	2000～2099(処理日時)
			car_ot.mon  = pExitDate->byMonth;		// 出庫_月	1～12(処理日時)
			car_ot.day  = pExitDate->byDay;			// 出庫_日	1～31(処理日時)
			car_ot.hour = pExitDate->byHours;		// 出庫_時	0～23(処理日時)
			car_ot.min  = pExitDate->byMinute;		// 出庫_分	0～59(処理日時)
			// 在車情報_精算 年月日時分秒
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear		= pExitDate->shYear;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth		= pExitDate->byMonth;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay		= pExitDate->byDay;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours		= pExitDate->byHours;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute	= pExitDate->byMinute;
			lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond	= pExitDate->bySecond;
		}
		else{
			// 未精算出庫精算だが、出庫情報が正しくセットされていない
			return( 1 );
		}
	}
	// 事前精算？
	else {
		// 精算開始時刻を取得
		pTime = GetPayStartTime();

		// 出庫時刻情報 <- parameterの現在時刻
		car_ot.year = pTime->year;									// 出庫 年
		car_ot.mon  = pTime->mont;									//      月
		car_ot.day  = pTime->date;									//      日
		car_ot.hour = pTime->hour;									//      時
		car_ot.min  = pTime->minu;									//      分
		// 在車情報_精算 年月日時分秒
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.shYear		= pTime->year;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMonth		= pTime->mont;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byDay		= pTime->date;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byHours		= pTime->hour;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.byMinute	= pTime->minu;
		lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond	= pTime->seco;
	}
	// 曜日取得
	car_ot.week = (char)youbiget( car_ot.year, (short)car_ot.mon, (short)car_ot.day );

	// LCDに通知するために精算時刻を保持する(残高変化通知にて送信する)
	memcpy( &paytime, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec, sizeof(paytime) );
	SetPayTargetTime( &paytime );

	// 入車時刻Fix <- 本関数call前に設定した入庫時刻情報
	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
	car_in_f.sec = (char)lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.bySecond;
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）

	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);

	// 出車時刻Fix <- 出庫時刻情報 <- parameterの現在時刻
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
	car_ot_f.sec = (char)lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtPaymentDateTime.dtTimeYtoSec.bySecond;
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）

	// 再計算用入車時刻Fix <- 本関数call前に設定した入庫時刻情報
	memcpy( &recalc_carin, &car_in, sizeof( struct CAR_TIM ) );

	// errorﾁｪｯｸの有無 = 無
	if( err_mode == 0 ){
		return( 0 );
	}

	// err_mode == 1なのでerrorﾁｪｯｸを行う
	// car_in情報 = Ope_Set_tyudata_Card()でset	--->	vl_lcd_tikchg();
	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	// 入庫時刻規定外?
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	// 出庫時刻規定外?
			ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTTIMENG, 2, 0 );		// 入出庫時刻規定外
			return( 1 );
	}

	// (入庫日+1年)normlize
	in_tim = dnrmlzm( (short)( car_in.year + 1 ), (short)car_in.mon, (short)car_in.day );

	// 出庫日normlize
	out_tim = dnrmlzm( car_ot.year, (short)car_ot.mon, (short)car_ot.day );

	// (入庫日+1年)normlize <= 出庫日normlize
	if( in_tim <= out_tim ){
		// 1年以上の料金計算を行った
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_OVER1YEARCAL, 2, 0 );
		return( 1 );
	}

	// (入庫日)normlize
	in_tim = dnrmlzm( (short)( car_in.year ), (short)car_in.mon, (short)car_in.day );

	// 入庫 > 出庫 ⇒ 入出庫逆転
	if( in_tim > out_tim ){
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_INOUTGYAKU, 2, 1 );
		return( 1 );
	}

	return( 0 );
}

//[]----------------------------------------------------------------------[]
///	@brief		精算開始時刻情報設定
//[]----------------------------------------------------------------------[]
///	@return		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void SetPayStartTime( struct clk_rec *pTime )
{
// MH810100(S) 2020/09/09 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
//	memcpy( &PayStartTime, pTime, sizeof(*pTime) );
	memcpy( &g_PayStartTime, pTime, sizeof(*pTime) );
// MH810100(E) 2020/09/09 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
}

//[]----------------------------------------------------------------------[]
///	@brief		精算開始時刻取得
//[]----------------------------------------------------------------------[]
///	@return		券挿入時刻情報ポインタ
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
struct clk_rec* GetPayStartTime( void )
{
// MH810100(S) 2020/09/09 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
//	return &PayStartTime;
	return &g_PayStartTime;
// MH810100(E) 2020/09/09 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
}

//[]----------------------------------------------------------------------[]
///	@brief		精算時刻設定
//[]----------------------------------------------------------------------[]
///	@return		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void SetPayTargetTime( DATE_YMDHMS *pTime )
{
	memcpy( &PayTargetTime, pTime, sizeof(*pTime) );
}

//[]----------------------------------------------------------------------[]
///	@brief		精算時刻取得
//[]----------------------------------------------------------------------[]
///	@return		券挿入時刻情報ポインタ
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static DATE_YMDHMS* GetPayTargetTime( void )
{
	return &PayTargetTime;
}

/********************************************************************/
/* ■QR/ICC/現金の同時使用											*/
/*		lcdbm_QRans_InCar_status									*/
/* 0 =	op_mod2()のloop内処理でQRﾃﾞｰﾀを受信していない状態			*/
/* 1 =	QRﾃﾞｰﾀ受信 & QRﾃﾞｰﾀ応答(OK)送信後のLCD_IN_CAR_INFO待ち		*/
/* 2 =	1の状態で現金(COIN_EVT: NOTE_EVT:)投入されて				*/
/*		料金計算を行いryo_bufの更新は行うが精算完了としない状態		*/
/*		invcrd()でret == 1 or 2	で精算完了とする時					*/
/*		精算完了としないで(ret = 0)	⇒ =op_mod02を抜けない			*/
/*		lcdbm_QRans_InCar_status <- 2とする							*/
/* 3	2の状態でop_mod02でLCD_IN_CAR_INFO受信した状態				*/
/*		この状態で割引精算の再計算を行う							*/
/*		割引精算後 <- 0にする										*/
/********************************************************************/
// invcrd()からreturnする前にcallされてinvcrd()のret変数をcheckする
// invcrd()では精算完了の有無のみ判定しているのでinvcrd()のcall元のin_mony()ではなくinvcrd()で行う
// ret == 1(精算終了(釣無し)) or 2(精算終了(釣有り)) && lcdbm_QRans_InCar_status == 1なら
// ret <- 0 & lcdbm_QRans_InCar_status <- 2にして精算完了としない
void lcdbm_QRans_InCar_DoNotPayCompCheck(short *ret)
{
	// 何らかの入金がある時
	if (ryo_buf.nyukin != 0) {
		// QRﾃﾞｰﾀ受信 & QRﾃﾞｰﾀ応答(OK)送信後のLCD_IN_CAR_INFO待ち
		if (lcdbm_QRans_InCar_status == 1) {
			// LCD_IN_CAR_INFO受信待ち
			// この状態で割引精算の再計算を行う
			// 割引精算後 <- 0にする
			lcdbm_QRans_InCar_status = 2;
		}

// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
//		if (lcdbm_QRans_InCar_status > 1) {
		if ( lcdbm_QRans_InCar_status == 2 ) {
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　QR割引のセンター問合せ中に現金で精算完了するとフェーズずれが発生する　不具合#7041
			// 1(精算終了(釣無し)) or 2(精算終了(釣有り))
			if ((*ret == 1) || (*ret == 2)) {
				// 精算未終了 <- 1(精算終了(釣無し)) or 2(精算終了(釣有り)
				*ret = 0;
			}
		}
	}
}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

/*[]----------------------------------------------------------------------[]*/
/*| 精算完了判別                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : invcrd( void )                                          |*/
/*| PARAMETER    : paymod : 0 = 通常, 1 = 修正                             |*/
/*| RETURN VALUE : ret    : 0 = 精算未終了                                 |*/
/*|                       : 1 = 精算終了(釣無し)                           |*/
/*|                       : 2 = 精算終了(釣有り)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	invcrd( ushort paymod )
{
	short	ret;

	ret = 0;
// MH321800(S) T.Nagai ICクレジット対応
//	if( ryo_buf.credit.pay_ryo != 0 ){
//		if ( ryo_buf.zankin == 0 ){
//			if ( ryo_buf.turisen == 0 ){
//				ret = 1;											// 精算終了(釣無し)
//			}else{
//				ret = 2;											// 精算終了(釣有り)
//			}
//		}
//		if( ryo_buf.credit.pay_ryo != 0 ){
//			creMessageAnaOnOff( 0, 0 );
//		}
//
//		Lagcan( OPETCBNO, 10 );										// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
//		OpeLcd( 4 );												// 料金表示(入金又は割引後)
//
//		if( ret == 0 ){
//			creMessageAnaOnOff( 1, 1 );
//			//クレジットカード挿入後にお金入った、すれ違いの場合.
//			OPECTL.credit_in_mony = 1;
//		}
//		return ret;
//	}
	// カード処理中
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if (Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC) {
	if (isEC_USE()) {
	if (isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		ec_MessageAnaOnOff( 0, 0 );

		OpeLcd( 4 );												// 料金表示(入金又は割引後)

// MH810103 GG119202(S) ブランド選択シーケンス変更
//		if(EcDeemedLog.EcDeemedBrandNo == BRANDNO_CREDIT) {
		if(RecvBrandResTbl.no == BRANDNO_CREDIT) {
// MH810103 GG119202(E) ブランド選択シーケンス変更
			ec_MessageAnaOnOff( 1, 1 );			// クレジットカード処理中
		} else {
			ec_MessageAnaOnOff( 1, 2 );			// カード処理中
		}
		//クレジットカード挿入後にお金入った、すれ違いの場合.
		OPECTL.credit_in_mony = 1;

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	// lcdbm_QRans_InCar_statusの更新check
	// invcrd()からreturnする前にcallされてinvcrd()のret変数をcheckする
	lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
		return ret;
	}
	// クレジット決済後
	else if (ryo_buf.credit.pay_ryo != 0) {
		if ( ryo_buf.turisen == 0 ){
			ret = 1;												// 精算終了(釣無し)
		}else{
			ret = 2;												// 精算終了(釣有り)
		}

		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
		} else {
			Lagcan( OPETCBNO, 10 );									// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
		}
		OpeLcd( 4 );												// 料金表示(入金又は割引後)

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
		// lcdbm_QRans_InCar_statusの更新check
		// invcrd()からreturnする前にcallされてinvcrd()のret変数をcheckする
		lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
		return ret;
	}
// MH321800(E) T.Nagai ICクレジット対応
// MH810103 GG119202(S) 現金投入と同時に交通系のカード処理中受信したとき、精算完了後、現金が戻らない。
	else if (PayData.Electron_data.Ec.pay_ryo != 0) {
		if ( ryo_buf.turisen == 0 ){
			ret = 1;											// 精算終了(釣無し)
		}else{
			ret = 2;											// 精算終了(釣有り)
		}

		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );			// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
		} else {
			Lagcan( OPETCBNO, 10 );								// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
		}
		OpeLcd( 4 );											// 料金表示(入金又は割引後)
// MH810103(s) 電子マネー対応 
		// lcdbm_QRans_InCar_statusの更新check
		// invcrd()からreturnする前にcallされてinvcrd()のret変数をcheckする
		lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810103(e) 電子マネー対応 
		return ret;
	}
// MH810103 GG119202(E) 現金投入と同時に交通系のカード処理中受信したとき、精算完了後、現金が戻らない。
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
	}
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し

	// ※ryo_buf.dsp_ryo は ryo_buf.zenkai を考慮（計算）済み
	if( ryo_buf.nyukin < ryo_buf.dsp_ryo ){ 						// 入金額 < 駐車料金?
		ryo_buf.zankin = ryo_buf.dsp_ryo - ryo_buf.nyukin;
	}else{
		ryo_buf.zankin = 0;
// MH322914(S) K.Onodera 2016/09/16 AI-V対応：振替精算
// 不具合修正(S) K.Onodera 2016/10/13 #1514 振替先で定期を利用すると通知した上で定期を使用せずに精算した時の釣銭が正しくない
//		if( PiP_GetFurikaeSts() && ryo_buf.dsp_ryo == 0 ){
		if( PiP_GetFurikaeSts() && (ryo_buf.dsp_ryo == 0 ) ){
// 不具合修正(E) K.Onodera 2016/10/13 #1514 振替先で定期を利用すると通知した上で定期を使用せずに精算した時の釣銭が正しくない
			ret = 2;												// 精算終了(釣有り)※釣銭はryo_calで算出
// MH321800(S) G.So ICクレジット対応
			if (isEC_USE() != 0) {
				Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );
			} else {
// MH321800(E) G.So ICクレジット対応
			Lagcan( OPETCBNO, 10 );
// MH321800(S) G.So ICクレジット対応
			}
// MH321800(E) G.So ICクレジット対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			// lcdbm_QRans_InCar_statusの更新check
			// invcrd()からreturnする前にcallされてinvcrd()のret変数をcheckする
			lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			return ret;
// MH810102(S) 電子マネー対応 #5500 現金投入と同時にQR割引で精算完了すると現金が返却されない
//// MH810100(S) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
//		}else if( ryo_buf.dsp_ryo == 0 ){
//			ret = 2;												// 精算終了(釣有り)※釣銭はryo_calで算出
//			if (isEC_USE() != 0) {
//				Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );
//			} else {
//				Lagcan( OPETCBNO, 10 );
//			}
//			lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
//			return ret;
//// MH810100(E) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
// MH810102(E) 電子マネー対応 #5500 現金投入と同時にQR割引で精算完了すると現金が返却されない
		}else{
// MH322914(E) K.Onodera 2016/09/16 AI-V対応：振替精算
		if(( ryo_buf.turisen = ryo_buf.nyukin - ryo_buf.dsp_ryo ) == 0 ){
			ret = 1;												// 精算終了(釣無し)
			if( OPECTL.Pay_mod == 2 ){								// 修正精算の時
				ret = SyuseiModosiPay( ret );
			}
		}else{
			ret = 2;												// 精算終了(釣有り)
		}
// MH322914(S) K.Onodera 2016/09/16 AI-V対応：振替精算
		}
// MH322914(E) K.Onodera 2016/09/16 AI-V対応：振替精算
	}
	// 駐車料金(残額)表示
	if( paymod == 0 ){												// 通常精算
// MH321800(S) G.So ICクレジット対応
		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
		} else {
// MH321800(E) G.So ICクレジット対応
		Lagcan( OPETCBNO, 10 );										// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
// MH321800(S) G.So ICクレジット対応
		}
// MH321800(E) G.So ICクレジット対応
// MH321800(S) D.Inaba ICクレジット対応
//		if( ryo_buf.credit.pay_ryo != 0 ){
//			creMessageAnaOnOff( 0, 0 );
//		}
// MH321800(E) D.Inaba ICクレジット対応

// MH321800(S) G.So ICクレジット対応
		if (isEC_USE() != 0) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
		} else {
// MH321800(E) G.So ICクレジット対応
		Lagcan( OPETCBNO, 10 );										// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
// MH321800(S) G.So ICクレジット対応
		}
// MH321800(E) G.So ICクレジット対応
		OpeLcd( 4 );												// 料金表示(入金又は割引後)

// MH321800(S) D.Inaba ICクレジット対応
//		if( ryo_buf.credit.pay_ryo != 0 ){
//			creMessageAnaOnOff( 1, 1 );
//		}
// MH321800(E) D.Inaba ICクレジット対応
		ryodsp = 1;
	}else{																				// 修正精算
		opedpl( 5, 16, ryo_buf.nyukin, 6, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 入金額表示
		opedpl( 6, 16, ryo_buf.zankin, 6, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );		// 残額表示
	}

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	// lcdbm_QRans_InCar_statusの更新check
	// invcrd()からreturnする前にcallされてinvcrd()のret変数をcheckする
	lcdbm_QRans_InCar_DoNotPayCompCheck(&ret);
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 中止時、戻し金額の算出                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : modoshi( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 戻し金額                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
long	modoshi( void )
{
	ulong	set, not_vl, ret;

	ret = ryo_buf.turisen = ryo_buf.nyukin;

	set = CPrmSS[S_SYS][46];										// 入金に対する最大払戻額

	if( set ){
		if( ryo_buf.in_coin[4] != 0 ){								// 紙幣入金有り?
			not_vl = ryo_buf.in_coin[4] * 1000L;
			if( not_vl > set ){
				ryo_buf.fusoku = not_vl - set;
				ret = ryo_buf.nyukin - ryo_buf.fusoku;
			}
		}
	}
	ryo_buf.modoshi = ret;											// 中止戻し額をｾｯﾄ
	return( (long)ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾊﾟｽﾜｰﾄﾞ入力処理(精算時)                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod70( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  :  1 = 他処理(料金表示,入金処理)へ切換             |*/
/*|                     : 10 = 取消(中止),待機へ戻る                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const uchar		KuroMaru[3] = { "●" };

short	op_mod70( void )
{
	short	ret;
	ushort	msg = 0;
	ushort	us_pass = 0;
	uchar	pushcnt = 0;
	ushort	Lk_pass;


	OpeLcd( 9 );													// 暗証番号入力画面表示

	ope_anm( AVM_P_INPUT );											// 暗証番号入力時ｱﾅｳﾝｽ

	if( OPECTL.multi_lk ){											// ﾏﾙﾁ精算問合せ中?
		Lk_pass = LOCKMULTI.lock_mlt.passwd;
	}else{
		Lk_pass = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd;
	}

	Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );		// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
				BUZPI();
				us_pass = (( us_pass % 1000 ) * 10 )+( msg - KEY_TEN );
				if( pushcnt == 0 ){									// 1文字目入力
					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[4] );				// "      暗証番号  ○○○○      "
				}
				if( pushcnt < 4 ){
					grachr(5, (ushort)(16+(pushcnt*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ●表示
					pushcnt++;										// ﾊﾟｽﾜｰﾄﾞ表示入力桁数(MAX4桁)
				}
				if( pushcnt == 4 ){									// 4文字目入力
					if( Lk_pass == us_pass ){						// ﾊﾟｽﾜｰﾄﾞは正しい?
						if( OPECTL.InLagTimeMode ) {				// ラグタイム延長処理実行？
							OPECTL.Ope_mod = 220;					// ラグタイム延長処理へ
						}else{
							ope_anm(AVM_SHASHITU);					// 車室読み上げ
							OPECTL.Ope_mod = 2;						// 料金表示,入金処理へ
						}
						ret = 1;
						break;
					}else{
						pushcnt = 0;
						grachr(7, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[42]);			// "   暗証番号が間違っています   "
						ope_anm( AVM_P_INPNG );						// 暗証番号間違い時ｱﾅｳﾝｽ
						Lagtim( OPETCBNO, 2, 3*50 );				// ﾀｲﾏｰ2(3s)起動(ｴﾗｰ表示用)
					}
				}
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				break;

			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
				BUZPIPI();
				break;

			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				ret = 10;											// 待機へ戻る
				break;

			case KEY_TEN_CL:										// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
				BUZPI();
				if( us_pass ){
					us_pass = 0;
					pushcnt = 0;
					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[4]);				// "      暗証番号  ○○○○      "
				}
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
				ret = 10;											// 待機へ戻る
				break;

			case TIMEOUT2:											// ﾀｲﾏｰ2ﾀｲﾑｱｳﾄ
				displclr( 7 );										// Display Line Clear
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 2 );											// ﾀｲﾏｰ2ﾘｾｯﾄ(ｴﾗｰ表示用)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾊﾟｽﾜｰﾄﾞ登録処理(入庫時)                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod80( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = 取消(中止),待機へ戻る                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod80( void )
{
	short	ret;
	ushort	msg = 0;
	ushort	us_pass = 0;
	uchar	pushcnt = 0;
	uchar	set;
	char	pri_time_flg;

	if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd == 0 ){			// ﾊﾟｽﾜｰﾄﾞ未登録
		OpeLcd( 8 );												// ﾊﾟｽﾜｰﾄﾞ登録画面表示
		ope_anm( AVM_P_ENTRY );										// 暗証番号登録時ｱﾅｳﾝｽ
		Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	}else{															// ﾊﾟｽﾜｰﾄﾞ登録済み
		OpeLcd( 19 );												// ﾊﾟｽﾜｰﾄﾞ登録済み画面表示
		ope_anm( AVM_STOP );										// 放送停止ｱﾅｳﾝｽ
		Lagtim( OPETCBNO, 1, 5*50 );								// ﾀｲﾏｰ1(5s)起動(ｵﾍﾟﾚｰｼｮﾝ用)
	}

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
				if(pushcnt <= 3 ){
					BUZPI();
					if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd == 0 ){		// ﾊﾟｽﾜｰﾄﾞ未登録
						us_pass = (( us_pass % 1000 ) * 10 )+( msg - KEY_TEN );
						pushcnt++;
						set = (uchar)prm_get( COM_PRM,S_TYP,63,1,1 );
						if( ( set == 3 )||( set == 4 ) ){
							blindpasword( us_pass, 4, 3, 16 ,pushcnt);					// ﾊﾟｽﾜｰﾄﾞ非表示
						}else{
							teninb_pass(us_pass, 4, 3, 16 ,pushcnt, COLOR_FIREBRICK);	// ﾊﾟｽﾜｰﾄﾞ表示
						}
						Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
					break;
				}else{
					BUZPIPI();
					break;
				}

			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
				if(pushcnt < 4 ){					//テンキーを4回押されないと受付券は発行しません
					BUZPIPI();
					break;
				}
				set = (uchar)prm_get( COM_PRM,S_TYP,62,1,3 );		// 駐車証明書発行可能枚数取得
				pri_time_flg = pri_time_chk(OPECTL.Pr_LokNo,0);
				if( ( us_pass == 0 ) || ( set == 0 ) ||	( pri_time_flg == 0 ) ){
					// ﾊﾟｽﾜｰﾄﾞ無し(0000)　または　受付券発行機能なし設定　または受付券発行時間オーバーの場合、登録＆発行不可 
					BUZPIPI();
					break;
				}

				/*** ﾊﾟｽﾜｰﾄﾞ登録 & 受付券発行 ***/
				BUZPI();

				if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd == 0 ){	// ﾊﾟｽﾜｰﾄﾞ未登録
					FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd = us_pass;// ﾊﾟｽﾜｰﾄﾞ格納

					displclr( 6 );									// Line Clear
					if( PriRctCheck() != OK ) {						// ﾚｼｰﾄ印字処理可／不可ﾁｪｯｸ
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[143] );	// "只今、受付券は発行できません  "
					} else {
					if( uke_isu( OPECTL.Op_LokNo, OPECTL.Pr_LokNo, OFF ) == 0 ){	// 受付券発行（通常）
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[74] );			// " 受付券をお取り下さい"
						ope_anm( AVM_UKE_PRN );						// 受付券発行ｱﾅｳﾝｽ
					}else{
						grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[73] );			// " 受付券は発行済みです"
						ope_anm( AVM_UKE_SUMI );					// 受付券発行済みｱﾅｳﾝｽ
					}
					}
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[39] );				// "  暗証番号登録が完了しました  "
					Lagtim( OPETCBNO, 1, 5*50 );					// ﾀｲﾏｰ1(5s)起動(ｵﾍﾟﾚｰｼｮﾝ用)
				}
				break;

			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
				if(pushcnt < 4 ){					//テンキーを4回押されないとパスワードの登録はしません
					BUZPIPI();
					break;
				}
				if( us_pass == 0 ){									// ﾊﾟｽﾜｰﾄﾞ無し(0000)は登録不可
					BUZPIPI();
					break;
				}

				/*** ﾊﾟｽﾜｰﾄﾞ登録 ***/
				BUZPI();

				if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd == 0 ){	// ﾊﾟｽﾜｰﾄﾞ未登録
					FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd = us_pass;// ﾊﾟｽﾜｰﾄﾞ格納

					displclr( 6 );									// Line Clear
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[39] );				// "  暗証番号登録が完了しました  "
					Lagtim( OPETCBNO, 1, 5*50 );					// ﾀｲﾏｰ1(5s)起動(ｵﾍﾟﾚｰｼｮﾝ用)
				}
				break;

			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				ret = 10;											// 待機へ戻る
				break;
			case KEY_TEN_CL:										// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
				BUZPI();
				if( FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd != 0 ){	// ﾊﾟｽﾜｰﾄﾞ登録あり
					break;
				}
				if( us_pass == 0 && pushcnt == 0){
				}else{
					pushcnt = 0;									//pushカウンター
					us_pass = 0;									// ﾊﾟｽﾜｰﾄﾞｸﾘｱ
					teninb_pass(us_pass, 4, 3, 16, pushcnt, COLOR_FIREBRICK);		// ﾊﾟｽﾜｰﾄﾞ表示
				}
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
				ret = 10;											// 待機へ戻る
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 受付券発行処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod90( void )                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = 取消(中止),待機へ戻る                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod90( void )
{
	short	ret;
	ushort	msg = 0;

	if( PriRctCheck() != OK ) {										// ﾚｼｰﾄ印字処理可／不可ﾁｪｯｸ
		OpeLcd( 29 );												// 受付券発行NG
	} else {
	if( uke_isu( OPECTL.Op_LokNo, OPECTL.Pr_LokNo, OFF ) ){			// 受付券発行（通常）
		/*** 発行済みNG ***/
		OpeLcd( 7 );												// 受付券発行NG
		ope_anm( AVM_UKE_SUMI );									// 受付券発行済みｱﾅｳﾝｽ
	}else{
		/*** 発行OK ***/
		OpeLcd( 6 );												// 受付券発行OK
		ope_anm( AVM_UKE_PRN );										// 受付券発行ｱﾅｳﾝｽ
	}
	}

	Lagtim( OPETCBNO, 1, 5*50 );									// ﾀｲﾏｰ1(5s)起動(ｵﾍﾟﾚｰｼｮﾝ用)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				ret = 10;											// 待機へ戻る
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
				ret = 10;											// 待機へ戻る
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)

	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| 受付券発行,ﾊﾟｽﾜｰﾄﾞ登録NG表示                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod81( void )                                        |*/
/*| PARAMETER    : mod : 81 = 暗証番号, 91 = 受付券                        |*/
/*| RETURN VALUE : ret  : 10 = 取消(中止),待機へ戻る                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.Sekiguchi                                             |*/
/*| Date         : 2006-09-26                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod81( uchar mod )
{
	short	ret;
	ushort	msg = 0;

	if( mod == 81 ){			// 受付券発行（通常）
		/*** 発行済みNG ***/
		OpeLcd( 26 );												// ﾊﾟｽﾜｰﾄﾞ登録NG
	}else if(mod == 91){
		/*** 発行OK ***/
		OpeLcd( 27 );												// 受付券発行NG
	}
	else if(mod == 92){
		OpeLcd( 28 );
	}

	Lagtim( OPETCBNO, 1, 5*50 );									// ﾀｲﾏｰ1(5s)起動(ｵﾍﾟﾚｰｼｮﾝ用)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				ret = 10;											// 待機へ戻る
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
				ret = 10;											// 待機へ戻る
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)

	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| ｶｰﾄﾞ使用有無判定処理                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : carduse( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 0 = 使用無し                                     |*/
/*|                     : 1 = 使用有り                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	carduse( void )
{
	short	ret;

	ret = 0;
	if( card_use[USE_PAS] +											// 定期券使用枚数
	    card_use[USE_PPC] +											// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用枚数
	    card_use[USE_NUM] +											// 回数券使用枚数
	    card_use[USE_N_SVC] )										// 新規ｻｰﾋﾞｽ券,掛売券,割引券使用枚数
	{
		ret = 1;
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ｻｰﾋﾞｽ券連続挿入処理処理                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : svs_renzoku( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ｻｰﾋﾞｽ券連続投入のため、保留している券(SNS6,7)を、精算完了または        |*/
/*| 精算中止時に、後方廃券し、自動排出ﾓｰﾄﾞへ遷移させるための処理           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nakayama, modified by Hara                            |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	svs_renzoku( void )
{
	if(( CPrmSS[S_DIS][3] == 1 )&&									// ｻｰﾋﾞｽ券取り込み
	   ( CPrmSS[S_DIS][2] == 0 )){									// 廃券ﾏｰｸ印字しない
		/*** 連続挿入許可する ***/
		if( cr_service_holding == 1 ){								// ｻｰﾋﾞｽ券の保留解除
			opr_snd( 10 );											// ｶｰﾄﾞ後方排出
			cr_service_holding = 0;									// ｻｰﾋﾞｽ券連続処理 保留解除
		}
	}else{
		/*** 連続挿入許可しない ***/
		OPE_red = 2;												// ﾘｰﾀﾞｰ自動排出
	}
	rd_shutter();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
}

/*[]----------------------------------------------------------------------[]*/
/*| 休業処理                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod100( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : -1 = ﾓｰﾄﾞ切替                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod100( void )
{
	short	ret;
	ushort	msg = 0;
	uchar	key_chk;

	short	wk_lev;
	char	f_Card = 0;												// 0:不正係員ｶｰﾄﾞ 1:係員ｶｰﾄﾞ受け -1:係員ｶｰﾄﾞNG
	uchar	MntLevel = 0;
	uchar	PassLevel = 0;
	uchar	KakariNo = 0;
	uchar	rd_err_count = 0;

// GG129001(S) インボイス対応（フラグのクリア処理）
	f_reci_ana = 0;
	OPECTL.f_RctErrDisp = 0;
// GG129001(E) インボイス対応（フラグのクリア処理）
// MH810103 GG119202(S) 未了残高照会タイムアウト画面が消えない
	if (isEC_USE()) {
		Ope_EcEleUseDsp();
	}
	else {
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない
	if( Suica_Rec.Data.BIT.CTRL ){								// 受付可状態？
		Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
	}
// MH810103 GG119202(S) 未了残高照会タイムアウト画面が消えない
	}
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( Edy_Rec.edy_status.BIT.INITIALIZE ){					// Edyﾓｼﾞｭｰﾙの初期化が完了している？
//		Edy_StopAndLedOff();									// ｶｰﾄﾞ検知停止指示開始＆LEDOFF
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	memset( DspChangeTime, 0, sizeof( DspChangeTime ));				// ﾜｰｸｴﾘｱの初期化
//	
//	DspChangeTime[0] = (ushort)(prm_get(COM_PRM, S_DSP, 30, 1, 1)); // 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ値取得
//	if(DspChangeTime[0] <= 0) {
//		DspChangeTime[0] = 2;
//	}
//// MH321800(S) G.So ICクレジット対応
//	if (isEC_USE() != 0) {
//		Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTART
//	} else {
//// MH321800(E) G.So ICクレジット対応
//	mode_Lagtim10 = 0;										// Lagtimer10の使用状態 0:利用可能媒体ｻｲｸﾘｯｸ表示
//	suica_fusiku_flg = 0;									// 利用可能媒体ｻｲｸﾘｯｸ表示有効
//	Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );		// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTART <--★06/09/15 Suzuki（ｻｲｸﾘｯｸ表示しない）
//// MH321800(S) G.So ICクレジット対応
//	}
//// MH321800(E) G.So ICクレジット対応
//
//	OpeLcd( 17 );													// 休業中画面表示
//
//	ope_anm( AVM_STOP );											// 放送停止ｱﾅｳﾝｽ
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	if( OPECTL.NtnetTimRec ){										// NT-NET時計受信ﾌﾗｸﾞON
		NTNET_Snd_Data104(NTNET_MANDATA_CLOCK);						// NT-NET管理ﾃﾞｰﾀ要求作成
		OPECTL.NtnetTimRec = 0;										// NT-NET時計受信ﾌﾗｸﾞOFF
	}
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	rd_shutter();													// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
//	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	Lagtim( OPETCBNO, 18, RD_ERR_INTERVAL );						// ﾀｲﾏｰ18(10s)ｼｬｯﾀｰ異常判定用ﾀｲﾏｰ

	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
	}

	if( MIFARE_CARD_DoesUse ){										// Mifareが有効な場合
		op_MifareStop_with_LED();								 	// Mifareを停止する
	}

	Lagtim( OPETCBNO, 1, 10*50 );									// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	tim1_mov = 1;													// ﾀｲﾏｰ1起動中
	if(( OPECTL.RECI_SW == (char)-1 )&&( RECI_SW_Tim == 0 )){		// 待機時領収証ﾎﾞﾀﾝ可?
		RECI_SW_Tim = (30 * 2) + 1;									// 領収証受付時間は30s固定(500msにてｶｳﾝﾄ)
	}
	Lagtim( OPETCBNO, 4, 1*50 );									// ﾀｲﾏｰ4(1s)起動(不正強制出庫監視用)初回のみ1sとする

	OPECTL.Ent_Key_Sts = OFF;										// 登録ｷｰ押下状態：ＯＦＦ
	OPECTL.Can_Key_Sts = OFF;										// 取消ｷｰ押下状態：ＯＦＦ
	OPECTL.EntCan_Sts  = OFF;										// 登録ｷｰ＆取消ｷｰ同時押下状態：ＯＦＦ

// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
	// まだ直前取引データ処理をしていなければ処理する
	if( ac_flg.ec_recv_deemed_fg != 0 ){
		EcRecvDeemed_RegistPri();
	}
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		if( Op_Event_Disable(msg) )									// イベント抑止チェック
			continue;												// 無効にする
		switch( msg ){
			case CLOCK_CHG:											// 時計更新
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				if( edy_auto_com ){
//					queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL ); // 営休業変化通知登録
//				}else{
				{
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
					// インデント調整の為、以下の処理にタブを入れる。
					if( opncls() == 1 ){								// 営業?
						ret = -1;
						break;
					}
					else {
						// 休業理由再表示
						if( (0 == (uchar)prm_get(COM_PRM, S_DSP, 35, 1, 1)) ||	// 休業表示する or 
							(2 == CLOSE_stat) ){								// 営業時間外(これは常に表示)
							grachr(0, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, CLSMSG[CLOSE_stat]);	// 休業理由表示
						}
					}
				}
				dspclk(7, COLOR_WHITE);								// 時計表示
				if( tim1_mov == 0 ){								// ﾀｲﾏｰ1未起動
					LcdBackLightCtrl( OFF );						// back light OFF
				}
				break;

			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_削除
//					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_削除
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					ret = -1;
				}
				break;

			case OPE_OPNCLS_EVT:									// 強制営休業 event
				if( opncls() == 1 ){								// 営業?
					ret = -1;
				} else {
					// 休業理由再表示
					if( (0 == (uchar)prm_get(COM_PRM, S_DSP, 35, 1, 1)) ||	// 休業表示する or 
						(2 == CLOSE_stat) ){								// 営業時間外(これは常に表示)
						grachr(0, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, CLSMSG[CLOSE_stat]);	// 休業理由表示
					}
				}
// MH810105(S) MH364301 インボイス対応
				if (f_reci_ana) {
					// 領収証ボタン押下済みであれば、
					// レシート印字失敗判定を行う
					op_rct_failchk();
				}
// MH810105(E) MH364301 インボイス対応
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
			case KEY_TEN_CL:										// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
				BUZPIPI();
				LcdBackLightCtrl( ON );								// back light ON
				Lagtim( OPETCBNO, 1, 10*50 );						// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				tim1_mov = 1;										// ﾀｲﾏｰ1起動中
				break;

			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
				LcdBackLightCtrl( ON );								// back light ON
				Lagtim( OPETCBNO, 1, 10*50 );						// ﾀｲﾏｰ1(10s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				tim1_mov = 1;										// ﾀｲﾏｰ1起動中
				if( OPECTL.RECI_SW == (char)-1 ){					// 待機時領収証ﾎﾞﾀﾝ可?
					BUZPI();
					ryo_isu( 0 );									// 領収証発行
					OPECTL.RECI_SW = 0;								// 領収書ﾎﾞﾀﾝ未使用
					RECI_SW_Tim = 0;								// 領収証受付時間ｸﾘｱ
// MH810105(S) MH364301 インボイス対応
					f_reci_ana = 1;
// MH810105(E) MH364301 インボイス対応
				}else{
					BUZPIPI();
				}
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				if (((m_kakari *)&MAGred[MAG_ID_CODE])->kkr_type == 0x20) {		// 係員カード
					f_Card = Kakariin_card();
					if( 1 == f_Card ) {
						if(CPrmSS[S_PAY][17]) {							// 電磁ロックあり？
							Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
							// 電磁ロック解除レベル以上の係員カード挿入時はロック解除
							if(PassLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
								LcdBackLightCtrl( ON );					// ﾊﾞｯｸﾗｲﾄ ON
								OPE_SIG_OUT_DOORNOBU_LOCK(0);			// ﾄﾞｱﾉﾌﾞのﾛｯｸ解除
								wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ﾓﾆﾀ登録(アンロック)
								Ope2_WarningDisp( 6*2, sercret_Str[3]);	// ｱﾝﾛｯｸｶﾞｲﾀﾞﾝｽ表示
								if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰ値が有効範囲内
									DoorLockTimer = 1;
									Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｽﾀｰﾄ
								}
							}
						}
					}
					else if( 0 == f_Card ) {							// 不正カード（未登録）
						Ope2_WarningDisp( 6*2, ERR_CHR[3]);				// 不正ｶﾞｲﾀﾞﾝｽ表示
					}
					else {												// カードNG
						Ope2_WarningDisp( 6*2, ERR_CHR[1]);				// 不正ｶﾞｲﾀﾞﾝｽ表示
					}
				}
				else {
				wk_lev = hojuu_card();
				if( NG != wk_lev && UsMnt_mnyctl_chk() ){			// 補充ｶｰﾄﾞﾁｪｯｸ
					// 補充ｶｰﾄﾞの場合
					LcdBackLightCtrl( ON );							// ﾊﾞｯｸﾗｲﾄ ON
					OPECTL.Mnt_mod = 4;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ←補充ｶｰﾄﾞﾒﾝﾃﾅﾝｽﾓｰﾄﾞ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					OPECTL.Mnt_lev = (char)wk_lev;
					Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
					OPECTL.Kakari_Num = KakariNo;					// 係員No.をセット
					ret = -1;										// ﾓｰﾄﾞ切替
				}
				}
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					Ope2_WarningDispEnd();							// ｴﾗｰ表示消し
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				tim1_mov = 0;										// ﾀｲﾏｰ1未起動
				LcdBackLightCtrl( OFF );							// back light OFF
				LedReq( CN_TRAYLED, LED_OFF );						// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED消灯
				break;

			case MID_RECI_SW_TIMOUT:								// 領収証発行可能ﾀｲﾏｰﾀｲﾑｱｳﾄ通知
				OPECTL.RECI_SW = 0;									// 領収書ﾎﾞﾀﾝ未使用
				break;

			case TIMEOUT4:											// ﾀｲﾏｰ4ﾀｲﾑｱｳﾄ
				Lagtim( OPETCBNO, 4, 5*50 );						// ﾀｲﾏｰ4(5s)起動(不正強制出庫監視用)
				break;

			case TIMEOUT6:											// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ：「登録」ｷｰ＆「取消」ｷｰ同時押し状態監視ﾀｲﾏｰ

				if( READ_SHT_flg == 1 ){							// ｼｬｯﾀｰ状態？
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// 磁気リーダなしの場合はシャッター開しない
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ﾀｲﾏｰ7（ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ）起動（３０秒）
					read_sht_opn();									// 閉の場合：開する
				}
				else{
					read_sht_cls();									// 開の場合：閉する
				}
				break;

			case TIMEOUT7:											// ﾀｲﾏｰ7ﾀｲﾑｱｳﾄ：ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ

				rd_shutter();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
				break;
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//			case TIMEOUT10:								// 「Suica停止後の受付許可送信待ち」or「料金画面表示切替」のﾀｲﾑｱｳﾄ通知
//				dspCyclicMsgRewrite(18);	// サイクリック表示更新
//				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
//				break;
//// MH321800(S) G.So ICクレジット対応
//			case EC_CYCLIC_DISP_TIMEOUT:								// 「ec停止後の受付許可送信待ち」or「料金画面表示切替」のﾀｲﾑｱｳﾄ通知
//				dspCyclicMsgRewrite(18);								// サイクリック表示更新
//				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰ1000mswait
//				break;
//// MH321800(E) G.So ICクレジット対応
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
			case TIMEOUT18:
				shutter_err_chk( &rd_err_count );					// 磁気ﾘｰﾀﾞｰ異常状態ﾁｪｯｸ
				break;
			case OPE_REQ_CALC_FEE:									// 料金計算要求
				// 料金計算
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);				// 全車室情報テーブル送信
				break;

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			// QR確定・取消データ応答(lcdbm_rsp_QR_conf_can_res_t)
			case LCD_QR_CONF_CAN_RES:
				break;

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			// レーンモニタデータ応答(lcdbm_rsp_LANE_res_t)
			case LCD_LANE_DATA_RES:
				break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

			// QRデータ
			case LCD_QR_DATA:
				// QR応答データ
				lcdbm_QR_data_res( 1 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
				break;

			// 決済状態ﾃﾞｰﾀ(lcdbm_ICC_Settlement_Status)
			case LCD_ICC_SETTLEMENT_STS:
				break;

			// 決済結果情報(lcdbm_ICC_Settlement_Result)
			case LCD_ICC_SETTLEMENT_RSLT:
				break;

			// 操作通知(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// 操作通知		操作ｺｰﾄﾞ
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// 精算中止要求
					case LCDBM_OPCD_PAY_STP:
						// 操作通知(精算中止応答(NG))送信	0=OK/1=NG
// MH810103(s) 電子マネー対応 休業時は中止要求はOKを返却する
//						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 0 );
// MH810103(e) 電子マネー対応 休業時は中止要求はOKを返却する
						break;

					// 精算中止応答		0=OK/1=NG
					case LCDBM_OPCD_PAY_STP_RES:
						break;

					// 精算開始要求			0固定
					case LCDBM_OPCD_PAY_STA:
						// 操作通知(精算開始応答(NG))送信	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
						break;

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 					// 遠隔精算開始応答		0=OK/1=NG
// 					case LCDBM_OPCD_RMT_PAY_STA_RES:
// 						break;
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

					// 起動通知
					case LCDBM_OPCD_STA_NOT:
						// 起動ﾓｰﾄﾞ = 設定ｱｯﾌﾟﾛｰﾄﾞ起動
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(起動中)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// 起動ﾓｰﾄﾞ = 通常起動
						else {
							// op_mod00(待機)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;
				}
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(起動中)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// 初期化未完了状態とする
				ret = -1;
				break;

			// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ
			case TIMEOUT_ICCREDIT_STOP:		// TIMERNO_ICCREDIT_STOP
				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// MH810103 GG119202(S) 未了残高照会タイムアウト画面が消えない
			case EC_RECEPT_SEND_TIMEOUT:							// ec用ﾀｲﾏｰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,0 );
				break;
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない
// MH810105(S) MH364301 インボイス対応
			case PRIEND_PREQ_RYOUSYUU:
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
// // GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
// //			if (IS_INVOICE) {
// 				if (!IS_INVOICE) {
// // GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//					break;
//				}
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
				if (f_reci_ana && OPECTL.Pri_Result == PRI_NML_END) {
					f_reci_ana = 0;
				}
				break;
// MH810105(E) MH364301 インボイス対応
			default:
				break;
		}
		key_chk = op_key_chek();									// 「登録」キー＆「取消」キーの同時押し状態監視
		switch( key_chk ){
			case	1:												// 同時押し状態開始
				Lagtim( OPETCBNO, 6, 3*50 );						// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）起動（３秒）
				break;
			case	2:												// 同時押し状態解除
				Lagcan( OPETCBNO, 6 );								// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）ﾘｾｯﾄ
				break;
		}

		lto_syuko();												// ﾌﾗｯﾌﾟ上昇、ロック閉ﾀｲﾏ内出庫
		fus_kyo();													// 不正・強制集計処理
		mc10exec();
	}

	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 4 );											// ﾀｲﾏｰ4ﾘｾｯﾄ(不正強制出庫監視用)
	Lagcan( OPETCBNO, 6 );											// ﾀｲﾏｰ6ﾘｾｯﾄ
	Lagcan( OPETCBNO, 7 );											// ﾀｲﾏｰ7ﾘｾｯﾄ
	Lagcan( OPETCBNO, 10 );											// 利用可能媒体ｻｲｸﾘｯｸ表示用ﾀｲﾏｰSTOP
	Lagcan( OPETCBNO, 18 );											// ﾀｲﾏｰ18(10s)ｼｬｯﾀｰ異常判定用ﾀｲﾏｰSTOP
// MH321800(S) G.So ICクレジット対応
	Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );					// ecリサイクル表示用ﾀｲﾏｰﾘｾｯﾄ
// MH321800(E) G.So ICクレジット対応
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
	LagCan500ms(LAG500_RECEIPT_MISS_DISP_TIMER);					// 領収証失敗表示タイマ
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）

	if( OPECTL.Mnt_mod != 4 ){										// 「補充ｶｰﾄﾞ受付完了」以外？
		rd_shutter();												// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾄﾞｱ閉時ｱﾗｰﾑ表示処理                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod110( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : -1 = ﾓｰﾄﾞ切替                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod110( void )
{
	short	ret;
	ushort	msg = 0;
	char	f_Card = 0;												// 0:不正係員ｶｰﾄﾞ 1:係員ｶｰﾄﾞ受け -1:係員ｶｰﾄﾞNG
	uchar	MntLevel = 0;
	uchar	PassLevel = 0;
	uchar	KakariNo = 0;
	uchar	key_chk;
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//	pcard_shtter_ctl = 0;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	LcdBackLightCtrl( ON );											// back light ON
// MH321800(S) hosoda ICクレジット対応
	Suica_Ctrl( S_CNTL_DATA, 0x00 );								// Suica利用を不可にする
// MH321800(E) hosoda ICクレジット対応
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	OpeLcd( 18 );													// ﾄﾞｱ閉時のｱﾗｰﾑ表示
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)

	if( OPECTL.NtnetTimRec ){										// NT-NET時計受信ﾌﾗｸﾞON
		NTNET_Snd_Data104(NTNET_MANDATA_CLOCK);						// NT-NET管理ﾃﾞｰﾀ要求作成
		OPECTL.NtnetTimRec = 0;										// NT-NET時計受信ﾌﾗｸﾞOFF
	}
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出
//	if(( RD_mod != 10 )&&( RD_mod != 11 )){
//		opr_snd( 3 );												// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
//	}
//	ope_anm( AVM_STOP );											// 放送停止ｱﾅｳﾝｽ
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
	Lagtim( OPETCBNO, 1, 2*50 );									// ﾀｲﾏｰ1(2s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	BUZPIPIPI();

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					ret = -1;
				}
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
			case KEY_TEN_CL:										// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
				BUZPIPI();
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				f_Card = Kakariin_card();
				if( 1 == f_Card ) {
					if(CPrmSS[S_PAY][17]) {							// 電磁ロックあり？
						Ope_KakariCardInfoGet(&MntLevel, &PassLevel, &KakariNo);
						// 電磁ロック解除レベル以上の係員カード挿入時はロック解除
						if(PassLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
							LcdBackLightCtrl( ON );					// ﾊﾞｯｸﾗｲﾄ ON
							OPE_SIG_OUT_DOORNOBU_LOCK(0);			// ﾄﾞｱﾉﾌﾞのﾛｯｸ解除
							wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ﾓﾆﾀ登録(アンロック)
							Ope2_WarningDisp( 6*2, sercret_Str[3]);	// ｱﾝﾛｯｸｶﾞｲﾀﾞﾝｽ表示
							if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰ値が有効範囲内
								DoorLockTimer = 1;
								Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｽﾀｰﾄ
							}
						}
					}
				}
				else if( 0 == f_Card ) {							// 不正カード（未登録）
					Ope2_WarningDisp( 6*2, ERR_CHR[3]);				// 不正ｶﾞｲﾀﾞﾝｽ表示
				}
				else {												// カードNG
					Ope2_WarningDisp( 6*2, ERR_CHR[1]);				// 不正ｶﾞｲﾀﾞﾝｽ表示
				}
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					Ope2_WarningDispEnd();							// ｴﾗｰ表示消し
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			case KEY_DOOR1:
				if( OPECTL.on_off == 0 ){							// OFF(Door Close)
					if( DoorCloseChk() == 100 ){
						OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
						ret = -1;
						break;
					}
					OpeLcd( 18 );									// ﾄﾞｱ閉時のｱﾗｰﾑ再表示
				}
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
				if( DoorCloseChk() == 100 ){						// ﾄﾞｱ閉でﾁｪｯｸ OK(異常復旧)
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					ret = -1;
					break;
				}
				Lagtim( OPETCBNO, 1, 2*50 );						// ﾀｲﾏｰ1(2s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				BUZPIPIPI();
				break;
			case TIMEOUT6:											// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ：「登録」ｷｰ＆「取消」ｷｰ同時押し状態監視ﾀｲﾏｰ

				if( READ_SHT_flg == 1 ){							// ｼｬｯﾀｰ状態？
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// 磁気リーダなしの場合はシャッター開しない
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ﾀｲﾏｰ7（ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ）起動（３０秒）
					read_sht_opn();									// 閉の場合：開する
				}
				else{
					read_sht_cls();									// 開の場合：閉する
				}
				break;
			case TIMEOUT7:											// ﾀｲﾏｰ7ﾀｲﾑｱｳﾄ：ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ

				rd_shutter();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
				break;
			case OPE_REQ_CALC_FEE:									// 料金計算要求
				// 料金計算
				ryo_cal_sim();
				NTNET_Snd_Data245(RyoCalSim.GroupNo);				// 全車室情報テーブル送信
				break;

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
			// QR確定・取消データ応答(lcdbm_rsp_QR_conf_can_res_t)
			case LCD_QR_CONF_CAN_RES:
				break;

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			// レーンモニタデータ応答(lcdbm_rsp_LANE_res_t)
			case LCD_LANE_DATA_RES:
				break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

			// QRデータ
			case LCD_QR_DATA:
				// QR応答データ
				lcdbm_QR_data_res( 1 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
				break;

			// 決済状態ﾃﾞｰﾀ(lcdbm_ICC_Settlement_Status)
			case LCD_ICC_SETTLEMENT_STS:
				break;

			// 決済結果情報(lcdbm_ICC_Settlement_Result)
			case LCD_ICC_SETTLEMENT_RSLT:
				break;

			// 操作通知(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// 操作通知		操作ｺｰﾄﾞ
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// 精算中止要求
					case LCDBM_OPCD_PAY_STP:
						// 操作通知(精算中止応答(NG))送信	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						break;

					// 精算開始要求			0固定
					case LCDBM_OPCD_PAY_STA:
						// 操作通知(精算開始応答(NG))送信
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
						break;

					// 起動通知
					case LCDBM_OPCD_STA_NOT:
						// 起動ﾓｰﾄﾞ = 設定ｱｯﾌﾟﾛｰﾄﾞ起動
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(起動中)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// 起動ﾓｰﾄﾞ = 通常起動
						else {
							// op_mod00(待機)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;
				}
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(起動中)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// 初期化未完了状態とする
				ret = -1;
				break;

			// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ
			case TIMEOUT_ICCREDIT_STOP:		// TIMERNO_ICCREDIT_STOP
				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

			default:
				break;
		}
		key_chk = op_key_chek();									// 「登録」キー＆「取消」キーの同時押し状態監視
		switch( key_chk ){
			case	1:												// 同時押し状態開始
				Lagtim( OPETCBNO, 6, 3*50 );						// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）起動（３秒）
				break;
			case	2:												// 同時押し状態解除
				Lagcan( OPETCBNO, 6 );								// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）ﾘｾｯﾄ
				break;
		}
	}

	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| 暗証番号(Btype)出庫操作                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod200( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : -1 = ﾓｰﾄﾞ切替                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Nishizato                                             |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod200( void )
{
	short	ret;
	ushort	msg = 0;
	short	ans;
	uchar	key_chk;

	LcdBackLightCtrl( ON );											// back light ON
	OpeLcd( 24 );													// 暗証番号出庫操作 車室入力画面 表示

	OPECTL.multi_lk = 0L;											// ﾏﾙﾁ精算問合せ中車室№
	OPECTL.ChkPassSyu = 0;											// 定期券問合せ中券種ｸﾘｱ
	OPECTL.ChkPassPkno = 0L;										// 定期券問合せ中駐車場№ｸﾘｱ
	OPECTL.ChkPassID = 0L;											// 定期券問合せ中定期券IDｸﾘｱ

	// TODO:暗証番号強制出庫待機画面復帰タイマーは標準用の共通パラメータを取得すること
	Lagtim( OPETCBNO, 1, 60*50+1 );								// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中はｷｰは利かない
					BUZPIPI();
					break;
				}
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}else{
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
				}
				ret = -1;
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中はｷｰは利かない
					BUZPIPI();
					break;
				}
				BUZPI();
				key_num = (( key_num % knum_ket ) * 10 )+( msg - KEY_TEN );
				teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				break;

			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
			case KEY_TEN_CL:										// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
				BUZPIPI();
				break;

			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				if( key_num == 0 ){
					ret = 10;										// 待機へ戻る
				}else{
					key_num = 0;									// 駐車位置番号ｸﾘｱ
					teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
				}
				break;

			case KEY_TEN_F1:										// ﾃﾝｷｰ[確認] ｲﾍﾞﾝﾄ
				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中はｷｰは利かない
					BUZPIPI();
					break;
				}
				BUZPI();
				Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)

				ans = carchk( key_sec, key_num, 0 );				// 駐車位置番号ﾁｪｯｸ
				if (((ans == 10) || (ans == -1)) ||
					 ((ans == 2) && (FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mode == FLAP_CTRL_MODE1))) {	// 規定外 or 精算不可能 or (出庫状態 and 待機状態)
					key_num = 0;
					teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示書換え
					break;
				}

				//暗証番号(Btype)入力操作へ遷移
				OPECTL.Ope_mod = 210;								// 暗証番号(Btype)入力操作へ
				ret = 1;

				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
				if( OPECTL.multi_lk ){								// ﾏﾙﾁ精算問合せ中?
					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				}else{
					ret = 10;
				}
				break;

			case IBK_NTNET_LOCKMULTI:								// NTNET ﾏﾙﾁ精算用ﾃﾞｰﾀ(車室問合せ結果ﾃﾞｰﾀ)受信
				if( LOCKMULTI.Answer == 0 && OPECTL.multi_lk )		// 結果が正常
				{
					ans = carchk_ml( key_sec, key_num );			// 駐車位置番号ﾁｪｯｸ
					if ((ans != 10) && (ans != -1)) {	// 規定外 以外 and 精算不可 以外
						OPECTL.Ope_mod = 210;						// 暗証番号(Btype)入力操作へ
						ret = 1;
						break;
					}
				}
				// no break
			case TIMEOUT2:											// ﾀｲﾏｰ2ﾀｲﾑｱｳﾄ
				if( OPECTL.multi_lk ){
					OPECTL.multi_lk = 0;

					key_num = 0;
					blink_end();									// 点滅終了
					teninb(key_num, knum_len, 2, Ope_Disp_LokNo_GetFirstCol(), COLOR_FIREBRICK);	// 駐車位置番号表示
					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );				// "                              "
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );				// "                              "
					if( LOCKMULTI.Answer ){							// 結果が未接続(該当無し)
						grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[6] );			// "  車室番号が正しくありません  "
					}else{
						grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR3[7] );			// "  他の精算機で操作して下さい  "
					}
					Lagtim( OPETCBNO, 4, 5*50 );					// ﾀｲﾏｰ4(5s)起動(ｴﾗｰ表示用)
					Lagtim( OPETCBNO, 1, (ushort)(CPrmSS[S_TYP][67]*50+1) );	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				}
				break;

			case TIMEOUT4:											// ﾀｲﾏｰ4ﾀｲﾑｱｳﾄ
				if( OPECTL.multi_lk == 0 ){
					LCDNO = (ushort)-1;
					OpeLcd( 2 );									// 駐車位置番号表示(再表示)
				}
				break;
			case TIMEOUT6:											// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ：「登録」ｷｰ＆「取消」ｷｰ同時押し状態監視ﾀｲﾏｰ

				if( READ_SHT_flg == 1 ){							// ｼｬｯﾀｰ状態？
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// 磁気リーダなしの場合はシャッター開しない
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ﾀｲﾏｰ7（ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ）起動（３０秒）
					read_sht_opn();									// 閉の場合：開する
				}
				else{
					read_sht_cls();									// 開の場合：閉する
				}
				break;
			case TIMEOUT7:											// ﾀｲﾏｰ7ﾀｲﾑｱｳﾄ：ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ

				rd_shutter();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
				break;

			default:
				break;
		}
		key_chk = op_key_chek();									// 「登録」キー＆「取消」キーの同時押し状態監視
		switch( key_chk ){
			case	1:												// 同時押し状態開始
				Lagtim( OPETCBNO, 6, 3*50 );						// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）起動（３秒）
				break;
			case	2:												// 同時押し状態解除
				Lagcan( OPETCBNO, 6 );								// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）ﾘｾｯﾄ
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ｵﾍﾟﾚｰｼｮﾝ制御用)
	Lagcan( OPETCBNO, 2 );											// ﾀｲﾏｰ2ﾘｾｯﾄ(ﾏﾙﾁ精算問合せ監視)
	Lagcan( OPETCBNO, 4 );											// ﾀｲﾏｰ4ﾘｾｯﾄ(ｴﾗｰ表示用)
	blink_end();													// 点滅終了

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 暗証番号(Btype)入力操作                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod210( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : -1 = ﾓｰﾄﾞ切替                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Nishizato                                             |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	op_mod210( void )
{
	uchar	visitor_code[6];		//お客様ｺｰﾄﾞ格納ｴﾘｱ
	short	ret;
	ushort	loklen;
	ushort	msg = 0;
	ushort	pass_num = 0;
	ushort	work = 0;
	uchar	pushcnt = 0;
	uchar	endflg = 0;
	uchar	key_chk;
	struct CAR_TIM wk_tm1,wk_tm2;

	lcd_backlight( ON );											// back light ON
	OpeLcd( 25 );													// 暗証番号出庫操作 暗証番号入力画面 表示

	//車室番号表示
	loklen = binlen((ulong)key_num);
	opedsp(1, 12, key_num, loklen, 1, 0, COLOR_CRIMSON, LCD_BLINK_OFF);	// 車室番号表示

	//お客様ｺｰﾄﾞ作成、表示、表示時間ﾀｲﾏｾｯﾄ
	MakeVisitorCode(visitor_code);									// お客様ｺｰﾄﾞ作成(4桁+1桁)
																	// visitor_code[0]～[3]：乱数
																	//            [4]：桁
																	//            [5]：\0
	grachr( 6, 16, 5, 0, COLOR_BLACK, LCD_BLINK_OFF, visitor_code);	// "  お客様コード nnnnn          "
	//お客様ｺｰﾄﾞ表示時間取得
	work = (ushort)CPrmSS[S_PAY][41];
	work = (work % 10000) / 10;
	if (work != 0) {
		Lagtim( OPETCBNO, 1,  (ushort)(work*50));	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
	}

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 1 ){							// key ON
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
					OPECTL.Mnt_mod = 1;								// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				}else{
					OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
					OPECTL.Pay_mod = 0;								// 通常精算
				}
				ret = -1;
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
				if(pushcnt <= 3 ){
					BUZPI();
					pass_num = (( pass_num % 1000 ) * 10 )+( msg - KEY_TEN );
					pushcnt++;	
					teninb_pass(pass_num, 4, 3, 16 ,pushcnt, COLOR_FIREBRICK);	// ﾊﾟｽﾜｰﾄﾞ表示
					if (work != 0) {
						Lagtim( OPETCBNO, 1,  (ushort)(work*50));	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
					break;
				}else{
					BUZPIPI();
					break;
				}
			case KEY_TEN_F2:										// ﾃﾝｷｰ[領収証] ｲﾍﾞﾝﾄ
			case KEY_TEN_F3:										// ﾃﾝｷｰ[受付券] ｲﾍﾞﾝﾄ
			case KEY_TEN_F5:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
				BUZPIPI();
				break;
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				if (endflg == 1) {									// 強制出庫処理が完了した時
					endflg = 0;										// 処理完了フラグクリア
				}
				ret = 10;											// 待機へ戻る
				break;
			case KEY_TEN_CL:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				if( endflg == 1 ){									// 暗証番号ｸﾘｱ、強制出庫処理が完了した時
					break;
				}
				if( pass_num == 0 && pushcnt == 0){
				}else{
					pass_num = 0;									// 暗証番号ｸﾘｱ
					pushcnt = 0;									//pushカウンター
					teninb_pass(0, 4, 3, 16, 0, COLOR_FIREBRICK);
					if (work != 0) {
						Lagtim( OPETCBNO, 1,  (ushort)(work*50));	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
				}
				break;
			case KEY_TEN_F1:										// ﾃﾝｷｰ[確認] ｲﾍﾞﾝﾄ
				//暗証番号(Btype)ﾁｪｯｸ
				if (CheckVisitorCode(visitor_code, pass_num) == 0) {
					//暗証番号(Btype)が正しくなかったら
					BUZPIPI();
					pass_num = 0;									// 暗証番号ｸﾘｱ
					pushcnt = 0;									//pushカウンター
					teninb_pass(0, 4, 3, 16, 0, COLOR_FIREBRICK);
					if (work != 0) {
						Lagtim( OPETCBNO, 1,  (ushort)(work*50));	// ﾀｲﾏｰ1(XXs)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
					}
					break;
				}
				BUZPI();
				work = 0;												// 精算案内
				if ((prm_get( COM_PRM,S_TYP,63,1,1 ))&&					// 暗証番号登録あり かつ
					(FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd != 0))	// 暗証番号が登録されている
				{
					if(prm_get( COM_PRM,S_PAY,41,1,5 ) == 1 ){			// 無条件解除番号強制出庫
						work = 1;										// 強制出庫
					}else{
						work = 2;										// 暗証番号解除
					}
					endflg = 1;											// 暗証番号ｸﾘｱ完了通知
				}
				else
				{
					if(prm_get( COM_PRM,S_PAY,41,1,5 ) == 1 ){			// 無条件解除番号強制出庫
						work = 1;										// 強制出庫
					}else{
						wk_tm1.year = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].year;
						wk_tm1.mon  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mont;
						wk_tm1.day  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].date;
						wk_tm1.hour = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].hour;
						wk_tm1.min  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].minu;

						wk_tm2.year = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_year;
						wk_tm2.mon  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_mont;
						wk_tm2.day  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_date;
						wk_tm2.hour = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_hour;
						wk_tm2.min  = FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_minu;
						if(ec64(&wk_tm1,&wk_tm2) != -1 ){				// 入庫時刻＜出庫時刻
						// 精算済の場合->強制出庫
							work = 1;									// 強制出庫
						}
					}
					endflg = 1;											// 強制出庫完了通知
				}
				if( work == 1 ){	// 強制出庫
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
//					queset( FLPTCBNO, FLAP_DOWN_SND, sizeof( OPECTL.Pr_LokNo ), &OPECTL.Pr_LokNo );	// 修正精算用ﾛｯｸ装置開(ﾌﾗｯﾌﾟ下降)
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[3] );					// "  強制出庫を行います          "
					// ロック装置
					wopelg(OPLOG_LOCK_OPEN, 0, (ulong)OPECTL.Pr_LokNo);
					endflg = 1;										// 強制出庫完了通知
				}else if( work == 2 ){	// 暗証番号クリア～精算案内
					FLAPDT.flp_data[OPECTL.Pr_LokNo-1].passwd = 0;	// 暗証番号(Atype)ｸﾘｱ
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[4] );					// "  精算してください            "
					wopelg(OPLOG_ANSHOU_B_CLR, 0, (ulong)OPECTL.Pr_LokNo);
					endflg = 1;										// 強制出庫完了通知
				}else{		// 精算案内
					// 未精算の場合
					grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[4] );					// "  精算してください            "
					endflg = 1;										// 強制出庫完了通知
				}
				//5秒で待機状態へ遷移
				Lagtim( OPETCBNO, 2, 5*50 );						// ﾀｲﾏｰ2(5s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
				pass_num = 0;										//次回CLｷｰ押下ですぐに待機状態へ戻るための処理
				break;
			case TIMEOUT6:											// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ：「登録」ｷｰ＆「取消」ｷｰ同時押し状態監視ﾀｲﾏｰ
				if( READ_SHT_flg == 1 ){							// ｼｬｯﾀｰ状態？
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// 磁気リーダなしの場合はシャッター開しない
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ﾀｲﾏｰ7（ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ）起動（３０秒）
					read_sht_opn();									// 閉の場合：開する
				}
				else{
					read_sht_cls();									// 開の場合：閉する
				}
				break;
			case TIMEOUT7:											// ﾀｲﾏｰ7ﾀｲﾑｱｳﾄ：ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ
				rd_shutter();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
				break;
			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
			case TIMEOUT2:											// ﾀｲﾏｰ2ﾀｲﾑｱｳﾄ
				ret = 10;											// 待機へ戻る
				break;
			default:
				break;
		}
		key_chk = op_key_chek();									// 「登録」キー＆「取消」キーの同時押し状態監視
		switch( key_chk ){
			case	1:												// 同時押し状態開始
				Lagtim( OPETCBNO, 6, 3*50 );						// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）起動（３秒）
				break;
			case	2:												// 同時押し状態解除
				Lagcan( OPETCBNO, 6 );								// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）ﾘｾｯﾄ
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ
	Lagcan( OPETCBNO, 2 );											// ﾀｲﾏｰ2ﾘｾｯﾄ

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| ラグタイム延長処理                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod220( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = 取消(中止),待機へ戻る                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Nagashima                                             |*/
/*| Date         : 2010-10-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
short	op_mod220( void )
{
	short	ret;
	ushort	msg = 0;
	uchar	timeout1 = OFF;

	FLAPDT.flp_data[OPECTL.Pr_LokNo-1].timer = LagTimer( OPECTL.Pr_LokNo );
																	// Lag time restart

	if( prm_get(COM_PRM, S_TYP, 68, 1, 3) == 2 &&					// ラグタイムタイムアップ後の再入庫時刻＝精算完了時刻
	    prm_get(COM_PRM, S_TYP, 80, 1, 3) == 1 ){					// 時刻更新する
		// ラグタイム延長操作時刻（現在時刻）を精算時刻に設定する。
		memcpy( &FLAPDT.flp_data[OPECTL.Pr_LokNo-1].s_year, &CLK_REC, 6 );
	}

	Flp_LagExtCnt[OPECTL.Pr_LokNo-1] += 1;							// 車室毎ラグタイム延長回数＋１

//	wptoff();
	sky.tsyuk.Lag_extension_cnt += 1;								// 集計－ラグタイム延長回数＋１
//	wpton();

	wmonlg(OPMON_LAG_EXT, NULL, OPECTL.Op_LokNo);					// モニタ登録（ラグタイム延長：R0504-xxxx）

	if( prm_get(COM_PRM, S_TYP, 80, 1, 2) ){						// ラグタイム延長制限あり
		if( Flp_LagExtCnt[OPECTL.Pr_LokNo-1] >= prm_get(COM_PRM, S_TYP, 80, 1, 1)){	// ラグタイム延長判定回数到達
			alm_chk2(ALMMDL_MAIN, ALARM_LAG_EXT_OVER, 2, 2, 1, &OPECTL.Op_LokNo);	// アラーム登録（ラグタイム延長操作規定回数オーバー：A0030-xxxx）
		}
	}else{															// ラグタイム延長制限なし
		if( prm_get(COM_PRM, S_TYP, 80, 1, 1) &&					// ラグタイム延長判定あり
		    Flp_LagExtCnt[OPECTL.Pr_LokNo-1] >= prm_get(COM_PRM, S_TYP, 80, 1, 1)){	// ラグタイム延長判定回数到達
			alm_chk2(ALMMDL_MAIN, ALARM_LAG_EXT_OVER, 2, 2, 1, &OPECTL.Op_LokNo);	// アラーム登録（ラグタイム延長操作規定回数オーバー：A0030-xxxx）
			Flp_LagExtCnt[OPECTL.Pr_LokNo-1] = 0;					// ラグタイム延長回数クリア
		}
	}

	OpeLcd( 30 );													// 出庫案内
	if( OPECTL.Pr_LokNo >= LOCK_START_NO ){
		ope_anm( AVM_KANRYO );										// 精算完了時アナウンス（ロック）
	}else{
		ope_anm( AVM_KANRYO2 );										// 精算完了時アナウンス（フラップ）
	}

	// 以下、「受付券発行処理(op_mod90())」より流用
	Lagtim( OPETCBNO, 1, 5*50 );									// ﾀｲﾏｰ1(5s)起動(ﾃﾝｷｰ受付不可時間用)
	Lagtim( OPETCBNO, 2, 10*50 );									// ﾀｲﾏｰ2(10s)起動(出庫案内表示用)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
			case KEY_TEN_CL:										// ﾃﾝｷｰ[区画] ｲﾍﾞﾝﾄ
				if( timeout1 == OFF ) {								// ﾀｲﾏｰ1ﾀ起動中？
					break;											// ﾃﾝｷｰ受付不可
				}
				// no break

			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();
				ret = 10;											// 待機へ戻る
				break;

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
				timeout1 = ON;
				break;

			case TIMEOUT2:											// ﾀｲﾏｰ2ﾀｲﾑｱｳﾄ
				ret = 10;											// 待機へ戻る
				break;

			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				cr_service_holding = 0;								// ｻｰﾋﾞｽ券の保留解除
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				break;

			case ARC_CR_EOT_EVT:									// ｶｰﾄﾞ抜き取り
				if( RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;

			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ﾃﾝｷｰ受付不可時間用)
	Lagcan( OPETCBNO, 2 );											// ﾀｲﾏｰ2ﾘｾｯﾄ(出庫案内表示用)

	OPECTL.InLagTimeMode = OFF;										// ラグタイム延長モードＯＦＦ

	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| 精算済み案内処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_mod230( void )                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret  : 10 = 待機へ戻る                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.Suzuki                                                |*/
/*| Date         : 2011-06-24                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
short	op_mod230( void )
{
	short	ret;
	ushort	msg = 0;
// MH810100(S) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
	ushort	res_wait_time;
	uchar	end_req = 0;
// MH810100(E) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
// MH810100(S) K.Onodera  2020/03/05 車番チケットレス(精算完了画面表示時間が設定値に従わない不具合修正)
	ushort	dsp_tm;
// MH810100(E) K.Onodera  2020/03/05 車番チケットレス(精算完了画面表示時間が設定値に従わない不具合修正)

// MH810100(S) K.Onodera  2020/03/05 車番チケットレス(精算完了画面表示時間が設定値に従わない不具合修正)
//	OpeLcd( 31 );													// 精算済み案内
	lcdbm_notice_ope( LCDBM_OPCD_PAY_CMP_NOT, 2 );					// 精算完了通知(精算済み案内)
// MH810100(E) K.Onodera  2020/03/05 車番チケットレス(精算完了画面表示時間が設定値に従わない不具合修正)

// MH810100(S) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// データ受信監視タイマ
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// 範囲外は5秒
	}
// MH810100(E) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
// MH810100(S) K.Onodera  2020/03/05 車番チケットレス(精算完了画面表示時間が設定値に従わない不具合修正)
//	Lagtim( OPETCBNO, 1, 5*50 );									// ﾀｲﾏｰ1(5s)起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
// GG129000(S) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
//	dsp_tm = (ushort)prm_get( COM_PRM, S_TYP, 128, 4, 1 );			// 精算完了案内タイマー
	// 本処理はラグタイム内精算専用=0円精算確定のため、タイマーには必ず個別の設定値をセットする
	dsp_tm = (ushort)prm_get( COM_PRM, S_TYP, 129, 2, 2 );			// 精算完了案内タイマー
// GG129000(E) 富士急ハイランド機能改善（0円精算時の精算完了画面表示時間を短くする（GM804102参考）（GM803003流用）
// GG129000(S) 富士急ハイランド機能改善（画面表示時間短縮仕様の強化）（GM804102流用）（GM803003流用）
	// ラグタイム内精算時は固定タイマー(1s)が作動しないため、その他の0円精算時と表示時間が同じになるようタイマー値を補正する。
	dsp_tm = dsp_tm + 1;
// GG129000(E) 富士急ハイランド機能改善（画面表示時間短縮仕様の強化）（GM804102流用）（GM803003流用）
	Lagtim( OPETCBNO, 1, dsp_tm*50 );								// ﾀｲﾏｰ1起動(ｵﾍﾟﾚｰｼｮﾝ制御用)
// MH810100(E) K.Onodera  2020/03/05 車番チケットレス(精算完了画面表示時間が設定値に従わない不具合修正)

	for( ret = 0; ret == 0; ){
		msg = GetMessage();
		switch( msg ){
			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				BUZPI();

			case TIMEOUT1:											// ﾀｲﾏｰ1ﾀｲﾑｱｳﾄ
// MH810100(S) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
//				ret = 10;											// 待機へ戻る
				// 精算案内終了通知
				lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
				// 応答待ちタイマー開始
				Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				end_req = 1;
// MH810100(E) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
				break;

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

			// QRデータ
			case LCD_QR_DATA:
				// QR応答データ
				lcdbm_QR_data_res( 1 );	// 結果(0:OK,1:NG(排他),2:NG(枚数上限))
				break;

			// 操作通知(lcdbm_rsp_notice_ope_t)
			case LCD_OPERATION_NOTICE:
				// 操作通知		操作ｺｰﾄﾞ
				switch( LcdRecv.lcdbm_rsp_notice_ope.ope_code) {
					// 精算中止要求
					case LCDBM_OPCD_PAY_STP:
						// 操作通知(精算中止応答(NG))送信	0=OK/1=NG
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STP_RES, 1 );
						break;

					// 精算開始要求			0固定
					case LCDBM_OPCD_PAY_STA:
// MH810100(S) K.Onodera 2020/02/18 #3883 「別車両の精算」押下すると車番検索画面に遷移してしまう
//						// 操作通知(精算開始応答(OK))送信
//						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );
//
//						// op_mod01(車番入力) = ｶｰﾄﾞ情報待ち
//						OPECTL.Ope_mod = 1;
//						ret = -1;
						// 操作通知(精算開始応答(NG)
						lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 1 );
						break;

					// 待機画面通知
					case LCDBM_OPCD_CHG_IDLE_DISP_NOT:
// MH810100(S) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
//						ret = 10;
						if( !end_req ){
							// 精算案内終了通知
							lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
							// 応答待ちタイマー開始
							Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
							end_req = 1;
						}
// MH810100(E) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
// MH810100(E) K.Onodera 2020/02/18 #3883 「別車両の精算」押下すると車番検索画面に遷移してしまう
						break;

					// 起動通知
					case LCDBM_OPCD_STA_NOT:
						// 起動ﾓｰﾄﾞ = 設定ｱｯﾌﾟﾛｰﾄﾞ起動
						if (LcdRecv.lcdbm_rsp_notice_ope.status == 0) {
							// op_init00(起動中)
							OPECTL.Ope_mod = 255;
							ret = -1;
						}
						// 起動ﾓｰﾄﾞ = 通常起動
						else {
							// op_mod00(待機)
							OPECTL.Ope_mod = 0;
							ret = -1;
						}
						break;

// MH810100(S) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
					// 精算案内終了応答
					case LCDBM_OPCD_PAY_GUIDE_END_RES:
						ret = 10;	// 待機へ戻る
						break;
// MH810100(E) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
				}
				break;

			// 精算案内終了応答タイムアウト
			case TIMEOUT_MNT_RESTART:
				// 精算案内終了通知
				lcdbm_notice_ope( LCDBM_OPCD_PAY_GUIDE_END_NOT, 0 );
				// 応答待ちタイマー再開
				Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				break;

			// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
			case LCD_DISCONNECT:
				// LcdRecv.lcdbm_lcd_disconnect.MsgId
				// op_init00(起動中)
				OPECTL.Ope_mod = 255;
				OPECTL.init_sts = 0;	// 初期化未完了状態とする
				ret = -1;
				break;

			// ICｸﾚｼﾞｯﾄ停止待ちﾀｲﾏ
			case TIMEOUT_ICCREDIT_STOP:		// TIMERNO_ICCREDIT_STOP
				break;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

			default:
				break;
		}
	}

	Lagcan( OPETCBNO, 1 );											// ﾀｲﾏｰ1ﾘｾｯﾄ(ﾃﾝｷｰ受付不可時間用)
// MH810100(S) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)
	Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );						// ﾀｲﾏｰ1ﾘｾｯﾄ(ﾃﾝｷｰ受付不可時間用)
// MH810100(E) K.Onodera  2020/03/05 #3931 車番チケットレス(ラグタイム内0円精算時、トップに戻る押下で待機に戻らない不具合修正)

	OPECTL.InLagTimeMode = OFF;										// ラグタイム延長モードＯＦＦ

	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MntMain( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	MntMain( void )
{
	uchar	pushcnt = 0;
	uchar	wpass[9];
	ushort	msg = 0;
	ushort	usMtSelEvent;
	char	f_Card = 0;												// 0:不正係員ｶｰﾄﾞ 1:係員ｶｰﾄﾞ受け -1:係員ｶｰﾄﾞNG
	uchar	menu_page = 0;											// 表示中ファンクションページ
	uchar	key_chk;
	uchar	tenkey_f5=0;
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
	uchar	ucReq = 0;	// 0=OK/1=NG/2=OK(再起動不要)
	ushort	mode = 0;
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)


	TimeAdjustCtrl(2);			// 自動時刻補正停止解除

	DP_CP[0] = DP_CP[1] = 0;

	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出

	if(( RD_mod != 10 )&&( RD_mod != 11 )){
		opr_snd( 3 );												// ﾘｰﾄﾞｺﾏﾝﾄﾞ(駐車券保留ｱﾘ)
	}

// GG124100(S) R.Endo 2022/09/28 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
// // GG120600(S) // Phase9 設定変更通知対応
// 	if(mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE){
// 		// parkingWeb接続あり
// 		if(_is_ntnet_remote()) {
// 			// 端末で設定パラメータ変更
// 			rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
// 			// 端末側で設定が更新されたため、予約が入っていたらキャンセルを行う
// 			remotedl_cancel_setting();
// 		}
// 		mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
// 	}
// // GG120600(E) // Phase9 設定変更通知対応
// GG124100(E) R.Endo 2022/09/28 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]		
	memset( wpass, 0x00, sizeof( wpass ) );							// Password clear
	LcdBackLightCtrl( ON );								// back light ON
	if( DOWNLOADING() ){
// MH810100(S) K.Onodera 2020/03/26 車番チケットレス(メンテナンス)
//		if( remotedl_disp() == MOD_CHG ){
//			return;
//		}
// GG120600(S) // Phase9 #5040 プログラムダウンロード実施中(FTP接続成功受信後)に精算機の電源OFF/ONすると、通信中の画面が表示されてメンテナンス画面に遷移できなくなる
		// task_statusにも反映
		remotedl_status_set(remotedl_status_get());
// GG120600(E) // Phase9 #5040 プログラムダウンロード実施中(FTP接続成功受信後)に精算機の電源OFF/ONすると、通信中の画面が表示されてメンテナンス画面に遷移できなくなる
		mode = remotedl_disp();
		if( mode == MOD_CHG || mode == MOD_CUT ){
			return;
		}
// MH810100(E) K.Onodera 2020/03/26 車番チケットレス(メンテナンス)
	}
// MH810100(S) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
	if( mnt_GetFtpFlag() != FTP_REQ_NONE ){
		ucReq = 1;	// 失敗
		if( 0 == lcdbm_setting_upload_FTP() ){		// LCDﾓｼﾞｭｰﾙに対するﾊﾟﾗﾒｰﾀｱｯﾌﾟﾛｰﾄﾞを行う
			// 成功
			switch( mnt_GetFtpFlag() ){
				// 1:FTP要求あり
				case FTP_REQ_NORMAL:
					ucReq = 2;	// OK(再起動不要)
					break;
				// 2:FTP要求あり(電断要)
				case FTP_REQ_WITH_POWEROFF:
					ucReq = 0;	// OK(再起動要)
					break;
				default:
					break;
			}
		}
		lcdbm_notice_ope( LCDBM_OPCD_PRM_UPL_NOT, (ushort)ucReq );	// 設定アップロード通知送信
		mnt_SetFtpFlag( FTP_REQ_NONE );								// FTP更新フラグ（更新終了）をセット
	}
// MH810100(E) Y.Yamauchi 2020/1/9 車番チケットレス(メンテナンス)
// GG124100(S) R.Endo 2022/09/28 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
	// 先にLCDへのパラメータアップロードを行ってから遠隔監視データを送信する
	// (LCDへのパラメータアップロード中に遠隔メンテナンス要求が届くのを防止するため)
	if ( mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE ) {
		// parkingWeb接続あり
		if ( _is_ntnet_remote() ) {
			// 端末で設定パラメータ変更
			rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
			// 端末側で設定が更新されたため、予約が入っていたらキャンセルを行う
			remotedl_cancel_setting();
		}
		mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
	}
// GG124100(E) R.Endo 2022/09/28 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]

	dsp_background_color(COLOR_WHITE);
	dispclr();														// Display All Clear

	if( OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4){										// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙ3(技術員操作)?
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[33] );							// "＜メンテナンス選択＞          "

		usMtSelEvent = Menu_Slt( SLMENU, MNT_SEL_TBL, (char)MNT_SEL_MAX, (char)1 );
																	// "１．ユーザーメンテナンス      "
																	// "２．システムメンテナンス      "
		if( usMtSelEvent == USR_MNT ){
			OPECTL.Mnt_mod = 2;										// 係員,管理者ﾒﾝﾃﾅﾝｽﾓｰﾄﾞへ移行
		}else if( usMtSelEvent == SYS_MNT ){
			OPECTL.Mnt_mod = 3;										// 技術員ﾒﾝﾃﾅﾝｽﾓｰﾄﾞへ移行
		}else if( usMtSelEvent == MOD_EXT ){						// F5 Key ( End Key )
			OPECTL.Mnt_lev = (char)-1;								// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
			OPECTL.PasswordLevel = (char)-1;
		}else if( usMtSelEvent == MOD_CHG ){						// Mode change key
// MH810100(S) K.Onodera 2020/2/19 #3887 メンテナンス選択画面に戻ってからドア閉にすると、初期画面へ遷移できなくなる
//			OPECTL.Mnt_mod = 0;										// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
//			OPECTL.Mnt_lev = (char)-1;								// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
//			OPECTL.PasswordLevel = (char)-1;
//		}
			op_wait_mnt_close();									// メンテナンス終了処理
		}
		else if( usMtSelEvent == MOD_CUT ){
			OPECTL.Ope_mod = 255;									// 初期化状態へ
			OPECTL.init_sts = 0;									// 初期化未完了状態とする
			OPECTL.Pay_mod = 0;										// 通常精算
			OPECTL.Mnt_mod = 0;										// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
			OPECTL.Mnt_lev = (char)-1;								// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
			OPECTL.PasswordLevel = (char)-1;
		}
// MH810100(E) K.Onodera 2020/2/19 #3887 メンテナンス選択画面に戻ってからドア閉にすると、初期画面へ遷移できなくなる
		Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
		return;
	}else{
		/* ﾒﾝﾃﾅﾝｽ画面から戻ってきた場合はﾒﾝﾃ終了操作LOG登録 */
		if( 0 != OPECTL.Kakari_Num ){									// ﾒﾝﾃﾅﾝｽ画面からの戻り
			wopelg( OPLOG_MNT_END, 0L, (ulong)OPECTL.Kakari_Num );		// 係員操作終了操作ﾛｸﾞ登録
			OPECTL.Kakari_Num = 0;										// 係員№ｸﾘｱ
			OPECTL.PasswordLevel = (char)-1;
		}
		else{
			msg = 0xffff;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//			Key_Event_Get();
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
		}
		OPECTL.Mnt_lev = (char)-1;									// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
		OPECTL.PasswordLevel = (char)-1;
		if( Err_onf ){
			grachr( 0, 8, 16, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[58]+8 );					// "       －エラー発生中－       "
		}else if( Alm_onf ){
			grachr( 0, 6, 18, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[59]+6 );					// "     －アラーム発生中－       "
		}
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[144] );						// "  パスワードを入力して下さい  "
		grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );							// "     パスワード  ○○○○     "
		if( prm_get( COM_PRM,S_TYP,63,1,1 ) ){						// 暗証番号登録あり?
			Fun_Dsp(FUNMSG[107]);
		}
		else{														// 暗証番号登録なし
			Fun_Dsp(FUNMSG[106]);
		}
		if( msg == 0xffff ){
// MH810103 GG119202(S) 未了残高照会タイムアウト画面が消えない
			if (isEC_USE()) {
				Ope_EcEleUseDsp();
// MH810104 GG119201(S) 駐車券挿入待ち→メンテ画面で直前取引内容が印字されない
				// メンテナンス画面へ遷移する際に
				// まだ直前取引データ処理をしていなければ処理する
				if( ac_flg.ec_recv_deemed_fg != 0 ){
					EcRecvDeemed_RegistPri();
				}
// MH810104 GG119201(E) 駐車券挿入待ち→メンテ画面で直前取引内容が印字されない
			}
			else {
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない
			Suica_Ctrl( S_CNTL_DATA, 0 );							// Suica利用を不可にする	
// MH810103 GG119202(S) 未了残高照会タイムアウト画面が消えない
			}
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			if( Edy_Rec.edy_status.BIT.INITIALIZE ){			// Edyの初期設定が完了済み？
//				Edy_StopAndLedOff();							// ｶｰﾄﾞ検知停止＆UI LED消灯指示送信  
//			}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			msg = 0;
		}
	}

	OPE_red = 2;													// ﾘｰﾀﾞｰ自動排出

	for( ;; ){
		msg = GetMessage();
		switch( msg ){
// MH810100(S) K.Onodera 2019/12/17 車番チケットレス（切断検知）
			case LCD_DISCONNECT:
				OPECTL.Ope_mod = 255;							// 初期化状態へ
				OPECTL.init_sts = 0;							// 初期化未完了状態とする
				OPECTL.Pay_mod = 0;								// 通常精算
				OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
				OPECTL.PasswordLevel = (char)-1;
				return;
				break;
// MH810100(E) K.Onodera 2019/12/17 車番チケットレス（切断検知）

			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 0 ){							// key OFF
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス（メンテナンス終了処理）
//					OPECTL.Pay_mod = 0;								// 通常精算
//					OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
//					OPECTL.PasswordLevel = (char)-1;
//					return;
					// メンテナンス終了処理
					mode = op_wait_mnt_close();
					if( mode == MOD_CHG || mode == MOD_CUT ){
						return;
					}
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス（メンテナンス終了処理）
				}
				break;

			case KEY_TEN0:											// ﾃﾝｷｰ[0] ｲﾍﾞﾝﾄ
			case KEY_TEN1:											// ﾃﾝｷｰ[1] ｲﾍﾞﾝﾄ
			case KEY_TEN2:											// ﾃﾝｷｰ[2] ｲﾍﾞﾝﾄ
			case KEY_TEN3:											// ﾃﾝｷｰ[3] ｲﾍﾞﾝﾄ
			case KEY_TEN4:											// ﾃﾝｷｰ[4] ｲﾍﾞﾝﾄ
			case KEY_TEN5:											// ﾃﾝｷｰ[5] ｲﾍﾞﾝﾄ
			case KEY_TEN6:											// ﾃﾝｷｰ[6] ｲﾍﾞﾝﾄ
			case KEY_TEN7:											// ﾃﾝｷｰ[7] ｲﾍﾞﾝﾄ
			case KEY_TEN8:											// ﾃﾝｷｰ[8] ｲﾍﾞﾝﾄ
			case KEY_TEN9:											// ﾃﾝｷｰ[9] ｲﾍﾞﾝﾄ
				BUZPI();
				if(f_Card) {
					// 係員カード挿入後、パスワード画面のままテンキーが入力された場合はクリアする
					OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
					OPECTL.PasswordLevel = (char)-1;
					f_Card = 0;
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );		// "     パスワード  ○○○○     "
				}
				if( OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4){										// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙ3(技術員操作)?
					;
				}else{
					/*--------------------------------------------------------------*/
					/* ●は左から表示するが、内部ﾊﾟｽﾜｰﾄﾞは右詰めでﾊﾞｯﾌｧﾘﾝｸﾞする。	*/
					/* 左へ1ﾊﾞｲﾄｼﾌﾄしてから新しい入力値を格納する。					*/
					/* 設定と比較されるのはwpass[4]～wpass[7]の4ﾊﾞｲﾄとなる。		*/
					/* wpass[0]～wpass[7]の8ﾊﾞｲﾄは[C4011441]用。					*/
					/*--------------------------------------------------------------*/
					if( pushcnt < 4 ){
						grachr( 4, (ushort)(17+(pushcnt*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru );	// ●表示
						pushcnt++;									// ﾊﾟｽﾜｰﾄﾞ表示入力桁数(MAX4桁)
					}
					memmove( &wpass[0], &wpass[1], 8 );				// 入力値を左へ1ﾊﾞｲﾄ移動
					wpass[8] = (uchar)( msg - KEY_TEN );
				}
				break;

			case KEY_TEN_F1:										// ﾃﾝｷｰ[精算] ｲﾍﾞﾝﾄ
				if( !prm_get( COM_PRM,S_TYP,62,1,2 ) ) {
					BUZPIPI();
					break;
				}
				if (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0){	// 精算ログ書き込み中
					BUZPIPI();
					break;
				}
				if( ope_imf_GetStatus() ){ 
					BUZPIPI();
					break;
				}
				if( prm_get(COM_PRM, S_TYP, 98, 1, 5) ){
					if( mtpass_get( pushcnt, wpass ) < 1 ){				// ﾒﾝﾃﾅﾝｽの役割(ﾚﾍﾞﾙ)取得
						BUZPIPI();
						break;
					}
				}
				wopelg( OPLOG_SYUSEI_START, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
				BUZPI();
				OPECTL.Pay_mod = 2;									// 修正精算処理
				OPECTL.Mnt_mod = 0;									// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
				OPECTL.PasswordLevel = (char)-1;
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;							// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
				OPECTL.PasswordLevel = (char)-1;
				return;

			case KEY_TEN_F4:										// ﾃﾝｷｰ[取消] ｲﾍﾞﾝﾄ
				/*** ﾊﾟｽﾜｰﾄﾞ確定処理 ***/
				if( (OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4) && f_Card != 1){			// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙ3(技術員操作)?
					;
				}else{
					if(f_Card != 1) {
						OPECTL.Mnt_lev = mtpass_get( pushcnt, wpass );	// ﾒﾝﾃﾅﾝｽの役割(ﾚﾍﾞﾙ)取得
					}
					else {
						f_Card = 0;
					}

// MH810104(S) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加
					// 料金計算テストの注意画面を表示する
					OPECTL.RT_show_attention = 1;
// MH810104(E) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加

					switch( OPECTL.Mnt_lev ){
						case 1:										// 係員操作
						case 2:										// 管理者操作
							if( read_rotsw() != 0 ) {
								BUZPIPI();
								grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ﾛｰﾀﾘｽｲｯﾁを0に戻して下さい   "
								OPECTL.Mnt_lev = (char)-1;			// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
								OPECTL.PasswordLevel = (char)-1;
								break;
							}
							BUZPI();
							OPECTL.Mnt_mod = 2;						// 係員,管理者ﾒﾝﾃﾅﾝｽﾓｰﾄﾞへ移行
							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
							wopelg( OPLOG_MNT_START_PASS, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
							if(CPrmSS[S_PAY][17]) {							// 電磁ロックあり？
								// 電磁ロック解除レベル以上の係員カード挿入時はロック解除
								if(OPECTL.PasswordLevel != (char)-1 && OPECTL.PasswordLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
									if(SD_EXIOPORT & 0x8000) {
									OPE_SIG_OUT_DOORNOBU_LOCK(0);			// ﾄﾞｱﾉﾌﾞのﾛｯｸ解除
									wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ﾓﾆﾀ登録(アンロック)
									if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰ値が有効範囲内
										DoorLockTimer = 1;
										Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｽﾀｰﾄ
									}
									}
								}
							}
							return;

						case 3:										// 技術員操作
						case 4:										// 開発者操作
							if( read_rotsw() != 0 ) {
								BUZPIPI();
								grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ﾛｰﾀﾘｽｲｯﾁを0に戻して下さい   "
								OPECTL.Mnt_lev = (char)-1;			// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
								OPECTL.PasswordLevel = (char)-1;
								break;
							}
							BUZPI();
							wopelg( OPLOG_MNT_START_PASS, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
							if(CPrmSS[S_PAY][17]) {							// 電磁ロックあり？
								// 電磁ロック解除レベル以上の係員カード挿入時はロック解除
								if(OPECTL.PasswordLevel != (char)-1 && OPECTL.PasswordLevel >=  (char)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
									if(SD_EXIOPORT & 0x8000) {
									OPE_SIG_OUT_DOORNOBU_LOCK(0);			// ﾄﾞｱﾉﾌﾞのﾛｯｸ解除
									wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ﾓﾆﾀ登録(アンロック)
									if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰ値が有効範囲内
										DoorLockTimer = 1;
										Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｽﾀｰﾄ
									}
									}
								}
							}
							dispclr();								// Display All Clear
							grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[33] );		// "＜メンテナンス選択＞          "

							usMtSelEvent = Menu_Slt( SLMENU, MNT_SEL_TBL, (char)MNT_SEL_MAX, (char)1 );
																	// "１．ユーザーメンテナンス      "
																	// "２．システムメンテナンス      "
							if( usMtSelEvent == USR_MNT ){
								OPECTL.Mnt_mod = 2;					// 係員,管理者ﾒﾝﾃﾅﾝｽﾓｰﾄﾞへ移行
							}else if( usMtSelEvent == SYS_MNT ){
								OPECTL.Mnt_mod = 3;					// 技術員ﾒﾝﾃﾅﾝｽﾓｰﾄﾞへ移行
							}else if( usMtSelEvent == MOD_EXT ){	// F5 Key ( End Key )
								OPECTL.Mnt_lev = (char)-1;			// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
								OPECTL.PasswordLevel = (char)-1;
							}else if( usMtSelEvent == MOD_CHG ){	// Mode change key
// MH810100(S) K.Onodera 2020/01/17 車番チケットレス（メンテナンス終了処理）
//								OPECTL.Mnt_mod = 0;					// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
//								OPECTL.Mnt_lev = (char)-1;			// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
//								OPECTL.PasswordLevel = (char)-1;
//							}
								op_wait_mnt_close();				// メンテナンス終了処理
							}
							else if( usMtSelEvent == MOD_CUT ){
								OPECTL.Ope_mod = 255;				// 初期化状態へ
								OPECTL.init_sts = 0;				// 初期化未完了状態とする
								OPECTL.Pay_mod = 0;					// 通常精算
								OPECTL.Mnt_mod = 0;					// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
								OPECTL.Mnt_lev = (char)-1;			// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
								OPECTL.PasswordLevel = (char)-1;
							}
// MH810100(E) K.Onodera 2020/01/17 車番チケットレス（メンテナンス終了処理）
							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
							return;

						default:
							BUZPIPI();
							OPECTL.Mnt_lev = (char)-1;				// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
							OPECTL.PasswordLevel = (char)-1;
							OPECTL.Kakari_Num = 0;					// 係員№ｸﾘｱ
							pushcnt = 0;							// ﾊﾟｽﾜｰﾄﾞ表示入力桁数ｸﾘｱ
							memset( wpass, 0x00, sizeof( wpass ) );	// Password clear
							grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );		// "     パスワード  ○○○○     "
							break;
					}
				}
				break;

			case KEY_TEN_F2:										// ﾃﾝｷｰ[登録] ｲﾍﾞﾝﾄ
				if( tenkey_f5 == 1 && menu_page == 1 ){					// 券無開のテンキー機能は２面目固定
					if(( OPECTL.Mnt_lev = mtpass_get( pushcnt, wpass )) >= 1){	// ﾒﾝﾃﾅﾝｽの役割(ﾚﾍﾞﾙ)取得(係員以上)
						if( read_rotsw() != 0 ) {
							BUZPIPI();
							grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ﾛｰﾀﾘｽｲｯﾁを0に戻して下さい   "
							break;
						}
						OPECTL.Mnt_mod = 2;								// 係員,管理者ﾒﾝﾃﾅﾝｽﾓｰﾄﾞへ移行
						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
						wopelg( OPLOG_MNT_START_PASS, 0L, (ulong)OPECTL.Kakari_Num );	// ﾊﾟｽﾜｰﾄﾞ係員操作開始操作ﾛｸﾞ登録
						
						if( Cardress_DoorOpen() == MOD_EXT ){
							if( OPECTL.Mnt_lev == 3 || OPECTL.Mnt_lev == 4 ){							// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙ3(技術員操作)?
								if( read_rotsw() != 0 ) {
									BUZPIPI();
									grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ﾛｰﾀﾘｽｲｯﾁを0に戻して下さい   "
									break;
								}
								BUZPI();
								OPECTL.Mnt_mod = 1;						// メンテナンス選択画面へ
							}else{
								if( read_rotsw() != 0 ) {
									BUZPIPI();
									grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ﾛｰﾀﾘｽｲｯﾁを0に戻して下さい   "
									break;
								}
								BUZPI();
								OPECTL.Mnt_mod = 2;						// ﾊﾟｽﾜｰﾄﾞ画面へ移行
							}
						}else{
							/* MOD_CHG */
							if( OPECTL.on_off == 0 ){					// key OFF
								BUZPI();
								OPECTL.Pay_mod = 0;						// 通常精算
								OPECTL.Mnt_mod = 0;						// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
								OPECTL.Mnt_lev = (char)-1;				// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
								OPECTL.PasswordLevel = (char)-1;
							}else{										// key ON
								if( read_rotsw() != 0 ) {
									BUZPIPI();
									grachr( 6, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[146] );	// 	"  ﾛｰﾀﾘｽｲｯﾁを0に戻して下さい   "
									break;
								}
								BUZPI();
								OPECTL.Pay_mod = 0;						// 通常精算
								OPECTL.Mnt_mod = 1;						// ﾒﾝﾃﾅﾝｽﾓｰﾄﾞ選択処理へ
								OPECTL.Mnt_lev = (char)-1;				// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
								OPECTL.PasswordLevel = (char)-1;
							}
						}
						Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
						return;
					}else{
						BUZPIPI();
						OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
						OPECTL.PasswordLevel = (char)-1;
						OPECTL.Kakari_Num = 0;							// 係員№ｸﾘｱ
						pushcnt = 0;									// ﾊﾟｽﾜｰﾄﾞ表示入力桁数ｸﾘｱ
						memset( wpass, 0x00, sizeof( wpass ) );			// Password clear
						grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );				// "     パスワード  ○○○○     "
					}
					break;
				}
				if( prm_get( COM_PRM,S_TYP,63,1,1 ) ){				// 暗証番号登録あり?
					BUZPI();
					OPECTL.Pay_mod = 1;								// ﾊﾟｽﾜｰﾄﾞ忘れ処理
					OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
					OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
					OPECTL.PasswordLevel = (char)-1;
					return;
				}else{
					BUZPIPI();
				}
				break;

			case KEY_TEN_CL:										// ﾃﾝｷｰCL ｲﾍﾞﾝﾄ
					BUZPI();
					pushcnt = 0;									// ﾊﾟｽﾜｰﾄﾞ表示入力桁数ｸﾘｱ
					memset( wpass, 0x00, sizeof( wpass ) );			// Password clear
					wpass[8] = 'C';
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[57] );				// "     パスワード  ○○○○     "
					grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );	// 	<--- Clear
					OPECTL.Mnt_lev = (char)-1;									// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
					OPECTL.PasswordLevel = (char)-1;
					break;
			case KEY_TEN_F3:										// ﾃﾝｷｰF3 ｲﾍﾞﾝﾄ
// テスト用ログ作成(S)
#ifdef TEST_LOGFULL
				BUZPI();
				grachr( 6, 10, 10, 0, COLOR_RED, LCD_BLINK_ON, "ログ出力中" );
				taskchg(IDLETSKNO);
				Test_Ope_LogFull();
				grachr( 6, 10, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, "　　　　　" );
#endif	// TEST_LOGFULL
// テスト用ログ作成(E)
// MH810100(S) テスト用入庫ログ作成
#ifdef TEST_ENTER_LOG
			BUZPI();
			Test_Ope_EnterLog();
#endif
// MH810100(E) テスト用入庫ログ作成
			case KEY_TEN_F5:										// ﾃﾝｷｰF5 ｲﾍﾞﾝﾄ
				BUZPIPI();
				break;
			case MID_STSCHG_ANY_ERROR:								// ｴﾗｰ 有無に変化あり
			case MID_STSCHG_ANY_ALARM:								// ｱﾗｰﾑ有無に変化あり
				if( Err_onf ){
					displclr( 0 );									// Line Clear
					grachr( 0, 8, 16, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[58]+8 );			// "       －エラー発生中－       "
				}else if( Alm_onf ){
					displclr( 0 );									// Line Clear
					grachr( 0, 6, 18, 1, COLOR_RED, LCD_BLINK_OFF, OPE_CHR[59]+6 );			// "     －アラーム発生中－       "
				}else{
					displclr( 0 );									// Line Clear
				}
				break;
			case ARC_CR_R_EVT:										// ｶｰﾄﾞIN
				read_sht_opn();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
				f_Card = Kakariin_card();
				if( 1 == f_Card ) {

					BUZPI();
					// レベルクリア
					OPECTL.Mnt_lev = (char)-1;
					OPECTL.PasswordLevel = (char)-1;

					f_Card = 1;
					grachr( 4, (ushort)17, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ●表示
					grachr( 4, (ushort)19, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ●表示
					grachr( 4, (ushort)21, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ●表示
					grachr( 4, (ushort)23, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, KuroMaru);		// ●表示
					Ope_KakariCardInfoGet((uchar*)&OPECTL.Mnt_lev, (uchar*)&OPECTL.PasswordLevel, &OPECTL.Kakari_Num);
					if(CPrmSS[S_PAY][17]) {							// 電磁ロックあり？
						// 電磁ロック解除レベル以上の係員カード挿入時はロック解除
						if(OPECTL.PasswordLevel >=  (uchar)prm_get(COM_PRM, S_PAY, 16, 1, 1)) {
							OPE_SIG_OUT_DOORNOBU_LOCK(0);			// ﾄﾞｱﾉﾌﾞのﾛｯｸ解除
							wmonlg( OPMON_LOCK_OPEN, 0, 0 );		// ﾓﾆﾀ登録(アンロック)
							if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){	// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰ設定あり
								DoorLockTimer = 1;
								Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｽﾀｰﾄ
							}
							grachr(6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, sercret_Str[3]);				// " <<ドアロックを解除しました>> "
						}
					}
				}
				else if( 0 == f_Card ) {							// 不正カード（未登録）
					grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[3] );	// このカードは無効です
				}
				else {												// カードNG
					grachr( 6, 0, 30, 1, COLOR_RED, LCD_BLINK_OFF, ERR_CHR[1] );	// このカードは使えません
				}
				break;
			case TIMEOUT4:
			case ARC_CR_EOT_EVT:
				displclr(6);
				if( msg == ARC_CR_EOT_EVT && RD_mod < 9 ){
					rd_shutter();									// 500msecﾃﾞﾚｲ後にｼｬｯﾀｰ閉する
				}
				break;
			case TIMEOUT6:											// ﾀｲﾏｰ6ﾀｲﾑｱｳﾄ：「登録」ｷｰ＆「取消」ｷｰ同時押し状態監視ﾀｲﾏｰ

				if( READ_SHT_flg == 1 ){							// ｼｬｯﾀｰ状態？
					if(0 == prm_get( COM_PRM,S_PAY,21,1,3 )) {
						// 磁気リーダなしの場合はシャッター開しない
						break;
					}
					Lagtim( OPETCBNO, 7, 30*50 );					// ﾀｲﾏｰ7（ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ）起動（３０秒）
					read_sht_opn();									// 閉の場合：開する
				}
				else{
					read_sht_cls();									// 開の場合：閉する
				}
				break;
			case TIMEOUT7:											// ﾀｲﾏｰ7ﾀｲﾑｱｳﾄ：ｼｬｯﾀｰ開状態継続監視ﾀｲﾏｰ

				rd_shutter();										// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ閉制御
				break;
// MH810103 GG119202(S) 未了残高照会タイムアウト画面が消えない
			case EC_RECEPT_SEND_TIMEOUT:							// ec用ﾀｲﾏｰ
				Ope_TimeOut_Recept_Send( OPECTL.Ope_mod,0 );
				break;
// MH810103 GG119202(E) 未了残高照会タイムアウト画面が消えない
			default:
				break;
		}
		key_chk = op_key_chek();									// 「登録」キー＆「取消」キーの同時押し状態監視
		switch( key_chk ){
			case	1:												// 同時押し状態開始
				Lagtim( OPETCBNO, 6, 3*50 );						// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）起動（３秒）
				break;
			case	2:												// 同時押し状態解除
				Lagcan( OPETCBNO, 6 );								// ﾀｲﾏｰ6（同時押し監視ﾀｲﾏｰ）ﾘｾｯﾄ
				break;
		}
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算動作ﾁｪｯｸ用入庫状態ｾｯﾄ処理                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_indata_set( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*|                                                                        |*/
/*|  接続設定されているﾛｯｸ装置の駐車位置を入庫状態とする                   |*/
/*|  （入庫日時は、現在年月日の５：００とする）                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-10-04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	op_indata_set( void )
{
	ushort	i;
	uchar	work;
// MH321800(S) hosoda ICクレジット対応
	ushort	normalize;
	short	date[3];
// MH321800(E) hosoda ICクレジット対応

#if (4 == AUTO_PAYMENT_PROGRAM)
	for( i=0 ; i < 24 ; i++ ){

			FLAPDT.flp_data[i].mode = 3;						// 入庫状態ｾｯﾄ
			FLAPDT.flp_data[i].year = (ushort)CLK_REC.year;		// 現在（年）ｾｯﾄ
			FLAPDT.flp_data[i].mont = (uchar)CLK_REC.mont;		// 現在（月）ｾｯﾄ
			FLAPDT.flp_data[i].date = (uchar)CLK_REC.date;		// 現在（日）ｾｯﾄ
			FLAPDT.flp_data[i].hour = 5;						// ０５時
			FLAPDT.flp_data[i].minu = 0;						// ００分
			memcpy( &LockInfo[i+1],&LockInfo[i],sizeof( LockInfo[0] ));
			LockInfo[i+1].posi = (LockInfo[i].posi+1);
			LockInfo[i+1].lok_no = (uchar)(LockInfo[i].lok_no+1);
			IoLog_write(IOLOG_EVNT_ENT_GARAGE, i, 0, (ulong)FLAPDT.flp_data[i].hour);	/* MH702200-T00 Y.Ise ログ情報記録デバッグ用 */
	}

#else
	work = GetCarInfoParam();
	for( i=0 ; i < LOCK_MAX ; i++ ){
		WACDOG;													// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		if( i < INT_CAR_START_INDEX ) {
			if( !( work & 0x02 ) ) {
				continue;
			}
		} else if( i < BIKE_START_INDEX ) {
			if( !( work & 0x04 ) ) {
				continue;
			}
		} else {
			if( !( work & 0x01 ) ) {
				continue;
			}
		}

		if( LockInfo[i].lok_syu != 0 ){
			// 接続設定あり
			FLAPDT.flp_data[i].mode = 3;						// 入庫状態ｾｯﾄ
			if (_is_pip()) {
				if ((i+1) == ryo_buf.pkiti) {
					FLAPDT.flp_data[i].mode = 5;					// 入庫状態ｾｯﾄ
				}
			}
			FLAPDT.flp_data[i].year = (ushort)CLK_REC.year;		// 現在（年）ｾｯﾄ
			FLAPDT.flp_data[i].mont = (uchar)CLK_REC.mont;		// 現在（月）ｾｯﾄ
			FLAPDT.flp_data[i].date = (uchar)CLK_REC.date;		// 現在（日）ｾｯﾄ
			if (_is_pip()) {
				if (i % 2) {
					FLAPDT.flp_data[i].hour = 9;					// 09時
				} else {
					FLAPDT.flp_data[i].hour = 5;					// ０５時
				}
			} else {
				FLAPDT.flp_data[i].hour = 5;						// ０５時
			}
// MH321800(S) hosoda ICクレジット対応
			if (CLK_REC.hour <= FLAPDT.flp_data[i].hour) {
			// 現在時刻 <= ５時（９時）→日にちを前日にする
				memset(date, 0, sizeof(date));
				date[0] = FLAPDT.flp_data[i].year;
				date[1] = FLAPDT.flp_data[i].mont;
				date[2] = FLAPDT.flp_data[i].date;
				normalize = dnrmlzm(date[0], date[1], date[2]);
				normalize--;
				idnrmlzm(normalize, &date[0], &date[1], &date[2]);
				FLAPDT.flp_data[i].year = (ushort)date[0];
				FLAPDT.flp_data[i].mont = (uchar)date[1];
				FLAPDT.flp_data[i].date = (uchar)date[2];
			}
// MH321800(E) hosoda ICクレジット対応
			FLAPDT.flp_data[i].minu = 0;						// ００分
			IoLog_write(IOLOG_EVNT_ENT_GARAGE, i, 0, (ulong)FLAPDT.flp_data[i].hour);	/* MH702200-T00 Y.Ise ログ情報記録デバッグ用 */
		}
	}
#endif
}

/*[]----------------------------------------------------------------------[]*/
/*| 登録ｷｰ、取消ｷｰの押下状態ﾁｪｯｸ処理                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_key_chek( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : uchar ret = ｷｰ状態                                      |*/
/*|                          0:無効（ｷｰ状態変化なし）                      |*/
/*|                          1:登録ｷｰ、取消ｷｰ同時押し開始                  |*/
/*|                          2:登録ｷｰ、取消ｷｰ同時押し解除                  |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2005-09-13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	op_key_chek( void )
{
	uchar	ret = 0;	// 戻り値

	if( PPrmSS[S_P01][1] == 0 ){

		// シャッター開放モードでない場合

		if( (OPECTL.Comd_knd == 0x01) && (OPECTL.Comd_cod == (0x00FF & KEY_TEN_F5)) ){
			// 「登録キー」イベント
			if( OPECTL.on_off == 0 ){
				// ＯＦＦ
				OPECTL.Ent_Key_Sts = OFF;				// 登録ｷｰ状態：ＯＦＦ
				if( OPECTL.EntCan_Sts == ON ){			// 同時押し状態だった？
					OPECTL.EntCan_Sts = OFF;			// 同時押し状態：ＯＦＦ
					ret = 2;							// 同時押し状態解除
				}
			}
			else{
				// ＯＮ
				OPECTL.Ent_Key_Sts = ON;				// 登録ｷｰ状態：ＯＮ
				if( OPECTL.Can_Key_Sts == ON ){			// 取消ｷｰ状態ＯＮ？
					OPECTL.EntCan_Sts = ON;				// 同時押し状態：ＯＮ
					ret = 1;							// 同時押し状態開始
				}
			}
		}
		else if( (OPECTL.Comd_knd == 0x01) && (OPECTL.Comd_cod == (0x00FF & KEY_TEN_F4)) ){
			// 「取消キー」イベント
			if( OPECTL.on_off == 0 ){
				// ＯＦＦ
				OPECTL.Can_Key_Sts = OFF;				// 取消ｷｰ状態：ＯＦＦ
				if( OPECTL.EntCan_Sts == ON ){			// 同時押し状態だった？
					OPECTL.EntCan_Sts = OFF;			// 同時押し状態：ＯＦＦ
					ret = 2;							// 同時押し状態解除
				}
			}
			else{
				// ＯＮ
				OPECTL.Can_Key_Sts = ON;				// 取消ｷｰ状態：ＯＮ
				if( OPECTL.Ent_Key_Sts == ON ){			// 登録ｷｰ状態ＯＮ？
					OPECTL.EntCan_Sts = ON;				// 同時押し状態：ＯＮ
					ret = 1;							// 同時押し状態開始
				}
			}
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*|  パラメータによって預り証・領収書の印字を行う						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Print_Condition_select( void )		                   |*/
/*| PARAMETER    : NONE													   |*/
/*| RETURN VALUE : NONE													   |*/
/*| ＜備考＞                                                               |*/
/*|    共通ﾊﾟﾗﾒｰﾀ01-0044に設定されている値を参照し、印字条件を変更する     |*/
/*|    0：預り証を自動発行								                   |*/
/*|    1：領収書に払出不足額を印字して自動発行					           |*/
/*|    2：自動発行しない								                   |*/
/*|    注：今回の対応では領収書ボタンを押下している場合は、OPECTL.RECI_SW  |*/
/*|    が１に設定されている認識であり、設定方法が変わった場合は修正の必要が|*/
/*|    あります。										                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.NAMIOKA                                               |*/
/*| Date         : 2006-02-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void Print_Condition_select( void )
{
	switch(CPrmSS[S_SYS][44]){							// 0:自動的に預り証の発行 1:自動的に領収書に預り内容を印字して発行 2:自動発行しない
		case 0:
			chu_isu();									// 預り書発行
			if(OPECTL.RECI_SW == 1)
				ryo_isu( 1 );							// 領収書(預り内容あり)発行
			break;
		case 1:
// MH810100(S) K.Onodera 2020/03/31 車番チケットレス(#3941 領収証自動発行後、ボタンが押下できてしまう)
			PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );	// 操作通知送信
// MH810100(E) K.Onodera 2020/03/31 車番チケットレス(#3941 領収証自動発行後、ボタンが押下できてしまう)
			ryo_isu( 1 );								// 領収書(預り内容あり)発行
// MH810100(S) 2020/05/27 車番チケットレス(#4181)
			if(OPECTL.op_faz == 3){											// 取消ボタンが押下された場合
				azukari_popup(1);
			}else{
				azukari_popup(0);
			}
// MH810100(E) 2020/05/27 車番チケットレス(#4181)
			OPECTL.RECI_SW = 1;							// 領収書ﾎﾞﾀﾝ使用不可状態
			break;
		case 2:
			if(OPECTL.RECI_SW == 1)
				ryo_isu( 1 );							// 領収書(預り内容あり)発行
			break;
		default:
			break;
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| お客様ｺｰﾄﾞ作成                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MakeVisitorCode(void)                                   |*/
/*| PARAMETER    : code お客様ｺｰﾄﾞ格納ｴﾘｱへのﾎﾟｲﾝﾀ                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Nishizato                                             |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void MakeVisitorCode(uchar *code)
{
	ushort wk, month, denom;
	short i;

	static unsigned int seed = 0;
	if (seed++ >= 0xefff) seed = 1;		// seed == 0 だと乱数が生成されない
	srand((unsigned int)CLK_REC.minu + seed);	//現在分をSeedとして乱数生成
	wk = (ushort)rand();
	wk %= 10000;

	//乱数ｾｯﾄ
	denom = 1000;
	for (i = 3; i >= 0; i--) {
		*code = (uchar)((wk / denom) + 0x30);
		wk %= denom;
		code++;
		denom /= 10;
	}

	//桁ｾｯﾄ
	if ((CPrmSS[S_PAY][41] % 10) == 1) {
		// 暗証番号(Btype)乱数表開始桁の更新は行わない
		wk = 1;
	} else {
		// 開始桁計算に当月の下1桁を使用する
		month = (ushort)CLK_REC.mont;
		wk = month % 10;
	}
	*code = (uchar)(wk + 0x30);
}

/*[]----------------------------------------------------------------------[]*/
/*| 暗証番号(Btype)ﾁｪｯｸ                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CheckVisitorCode(void)                                  |*/
/*| PARAMETER    : code お客様ｺｰﾄﾞ格納ｴﾘｱへのﾎﾟｲﾝﾀ                         |*/
/*|                val  入力暗証番号                                       |*/
/*| RETURN VALUE : 0:暗証番号不一致 / 1:暗証番号一致                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Nishizato                                             |*/
/*| Date         : 2006-08-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort CheckVisitorCode(uchar *code, ushort val)
{
	ulong pass1, pass2, denom;
	ushort wk, kt;
	short i, j;
	ulong pass[10];

	//乱数表取得
	pass1 = (ulong)CPrmSS[S_PAY][42];
	pass2 = (ulong)CPrmSS[S_PAY][43];
	denom = 100000;
	for (i = 0; i < 10; i ++) {
		if (i < 6) {
			pass[i] = pass1 / denom;
			if (i == 5) {
				denom = 100000*10;
			}
			pass1 %= denom;
		} else {
			pass[i] = pass2 / denom;
			pass2 %= denom;
		}
		denom /= 10;
	}

	//開始桁の取得
	kt = code[4] - 0x30;
	if (kt == 0) kt = 10;

	//値取得
	wk = 0;
	denom = 1000;
	for (i = 3; i >= 0; i--) {
		j = (kt-1)+(code[3-i]-0x30);
		if (j == 0) j = 10;	// 0(桁目)なら10桁目を参照する
		if (j > 10) j -= 10;
		wk += (ushort)(pass[j-1] * denom);
		denom /= 10;
	}
	//値一致ﾁｪｯｸ
	if (wk == val) {
		return 1;
	} else {
		return 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算領域へのﾃﾞｰﾀの格納		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ElectronSet_PayData						               |*/
/*| PARAMETER    : ppc_tiket *buf : ﾃﾞｰﾀ格納バッファ                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2007-02-26                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	ElectronSet_PayData( void *buf, uchar data_kind )
{

	#if (3 == AUTO_PAYMENT_PROGRAM)									// 試験用にカード残額をごまかす場合（ﾃｽﾄ用）
		ulong	wkul;
	#endif

	if( SuicaUseKindCheck( data_kind ) ){									// 決算がsuicaで行われた場合
		PayData.Electron_data.Suica.e_pay_kind = data_kind;	// 決算種別をｾｯﾄ
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.e_pay_kind), NTNET_152_ECARDKIND);
		memcpy( &PayData.Electron_data.Suica.pay_ryo,&((SUICA_SETTLEMENT_RES*)buf)->settlement_data, 28); // 決済金額・残高・ｶｰﾄﾞIDをｾｯﾄ
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.pay_ryo), NTNET_152_EPAYRYO);
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.Card_ID[0]), NTNET_152_ECARDID);
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.pay_after), NTNET_152_EPAYAFTER);

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	}else if( data_kind == EDY_USED ){						// 決算がEdyで行われた場合
//		memset( &PayData.Electron_data.Edy, 0, sizeof( PayData.Electron_data.Edy ));
//		PayData.Electron_data.Edy.e_pay_kind = data_kind;	// 決算種別をｾｯﾄ
//		// Edy情報のｾｯﾄを以下で行う
//		PayData.Electron_data.Edy.pay_ryo		=	PayChengeToBIN(((R_SUBTRACTION_DATA*)buf)->Pay_Ryo);    	// 決済金額をｾｯﾄ
//		PayData.Electron_data.Edy.pay_befor		=	PayChengeToBIN(((R_SUBTRACTION_DATA*)buf)->Pay_Befor);		// 精算前残額をｾｯﾄ
//		PayData.Electron_data.Edy.pay_after		=	PayChengeToBIN(((R_SUBTRACTION_DATA*)buf)->Pay_After);		// 精算後残額をｾｯﾄ
//
//		/** ﾃｽﾄ用 ｶｰﾄﾞ残額ごまかし処理 **/
//		#if (3 == AUTO_PAYMENT_PROGRAM)									// 試験用にカード残額をごまかす場合（ﾃｽﾄ用）
//			if( 0L != (unsigned long)CPrmSS[S_SYS][5] ){				// 残額ごまかし設定あれば
//				wkul = CPrmSS[S_SYS][5];								// ｶｰﾄﾞ内残額変更
//				if( PayData.Electron_data.Edy.pay_befor > wkul ) {		// 実ｶｰﾄﾞ残額 > ごまかし額（実ｶｰﾄﾞ残額より大きな値にはしない）
//					PayData.Electron_data.Edy.pay_befor = wkul;			// 減算前ｶｰﾄﾞ残額ごまかし
//					PayData.Electron_data.Edy.pay_after = PayData.Electron_data.Edy.pay_befor - PayData.Electron_data.Edy.pay_ryo;
//																		// 減算後ｶｰﾄﾞ残額ごまかし
//				}
//			}
//		#endif
//
//		if( prm_get(COM_PRM, S_SCA, 55, 1, 1) != 1 ){															// ﾃｽﾄ用ｶｰﾄﾞ使用設定時
//			PayData.Electron_data.Edy.deal_no		=	((R_SUBTRACTION_DATA*)buf)->Edy_Deal_num;				// Edy取引通番をｾｯﾄ
//			PayData.Electron_data.Edy.card_deal_no	=	((R_SUBTRACTION_DATA*)buf)->Card_Deal_num;				// ｶｰﾄﾞ取引通番をｾｯﾄ
//			memcpy( PayData.Electron_data.Edy.Terminal_ID, &((R_SUBTRACTION_DATA*)buf)->High_Terminal_ID,4);
//			memcpy( PayData.Electron_data.Edy.Card_ID, ((R_SUBTRACTION_DATA*)buf)->Edy_No, sizeof( PayData.Electron_data.Edy.Card_ID ));
//		}
//#endif
//	}else if( data_kind == 99 ){							// 決算がEdyﾃｽﾄﾓｰﾄﾞで行われた場合
//		memset( &PayData.Electron_data.Edy, 0, sizeof( PayData.Electron_data.Edy ));
//		PayData.Electron_data.Edy.e_pay_kind = EDY_USED;													// 決算種別をｾｯﾄ
//		PayData.Electron_data.Edy.pay_befor		=	Edy_Settlement_Res.Pay_Befor;							// 精算前残額をｾｯﾄ
//		PayData.Electron_data.Edy.pay_ryo		=	Edy_Settlement_Res.Pay_Ryo;								// 決済金額をｾｯﾄ
//		PayData.Electron_data.Edy.pay_after		=	Edy_Settlement_Res.Pay_Befor;							// 精算前残額をｾｯﾄ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

	}else{
		;													// ﾊﾟｽﾈｯﾄ対応予定時にﾛｼﾞｯｸを作成する
	}

}

// MH321800(S) hosoda ICクレジット対応
// MH810103 GG119202(S) 電子マネー対応
const unsigned char EcBrandEmoney_Digit[] = {
	// 電子マネーのブランドのカード番号桁数
	// EC_KOUTSUU_USED等の順番に合わせる
	0,		// EC_UNKNOWN_USED [11]不明ブランド
	0,		// EC_ZERO_USED [12]ブランド番号0
	16,		// EC_EDY_USED [13]Edy
	16,		// EC_NANACO_USED [14]nanaco
// MHUT40XX(S) Edy・WAON対応
//	17,		// EC_WAON_USED [15]WAON
	16,		// EC_WAON_USED [15]WAON
// MHUT40XX(E) Edy・WAON対応
	0,		// EC_SAPICA_USED [16]SAPICA
	17,		// EC_KOUTSUU_USED [17]交通系ICカード
	16,		// EC_ID_USED [18]iD
	20,		// EC_QUIC_PAY_USED [19]QuicPay
	0,		// EC_CREDIT_USED [20]クレジット
	0,		// EC_HOUJIN_USED [21]法人
// MH810105(S) MH364301 QRコード決済対応
	0,		// EC_HOUJIN_USED [22]法人
	0,		// EC_HOUSE_USED [23]ハウスカード
	0,		// EC_QR_USED [24]QRコード決済
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
	17,		// EC_PITAPA_USED [25]PiTaPa
// MH810105(E) MH364301 PiTaPa対応
};
// MH810103 GG119202(E) 電子マネー対応
/*[]----------------------------------------------------------------------[]*/
/*| 精算領域へのﾃﾞｰﾀの格納 (決済リーダでの電子マネー決済)                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcElectronSet_PayData					               |*/
/*| PARAMETER    : *buf		: 取引ﾃﾞｰﾀ				                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : hosoda                                                  |*/
/*| Date         : 2019-02-06                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	EcElectronSet_PayData( void *buf )
{
	EC_SETTLEMENT_RES	*p;
// MH810103 GG119202(S) 未了残高照会完了時のカード番号はマスクして印字する
	ushort	i, max;
// MH810103 GG119202(E) 未了残高照会完了時のカード番号はマスクして印字する
// MH810105(S) MH364301 QRコード決済対応
//// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//	uchar	dummy_Card_ID[20];
//	uchar	dummy_inquiry_num[16];
//// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
// MH810105(E) MH364301 QRコード決済対応

	p = (EC_SETTLEMENT_RES*)buf;
	if(p->Column_No == 0xFF) {
		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;									// みなし決済
	}
	PayData.Electron_data.Ec.pay_ryo	= p->settlement_data;											// 決済金額
	PayData.Electron_data.Ec.pay_befor	= p->settlement_data_before;									// 決済前残高
	PayData.Electron_data.Ec.pay_after	= p->settlement_data_after;										// 決済後残高
	memcpy(PayData.Electron_data.Ec.Card_ID, p->Card_ID, sizeof(PayData.Electron_data.Ec.Card_ID));		// ｶｰﾄﾞID
	memcpy(PayData.Electron_data.Ec.inquiry_num, p->inquiry_num, sizeof(PayData.Electron_data.Ec.inquiry_num));// 問合せ番号
	PayData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);						// 決済日時(Normalize)
// MH810105 GG119202(S) 処理未了取引集計仕様改善
	PayData.EcResult = p->Result;																		// 決済結果
// MH810105 GG119202(E) 処理未了取引集計仕様改善
// MH810105(S) MH364301 不要変数削除
//// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
//	memcpy(PayData.Electron_data.Ec.Termserial_No, p->Termserial_No, sizeof(PayData.Electron_data.Ec.Termserial_No));// 端末識別番号
//// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
// MH810105(E) MH364301 不要変数削除

	switch (p->brand_no) {
	case	BRANDNO_KOUTSUU:
		PayData.Electron_data.Ec.e_pay_kind					= EC_KOUTSUU_USED;							// 決算種別
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
		memcpy(PayData.Electron_data.Ec.Brand.Koutsuu.SPRW_ID, p->Brand.Koutsuu.SPRW_ID,
				sizeof(PayData.Electron_data.Ec.Brand.Koutsuu.SPRW_ID));								// SPRWID
		memcpy(PayData.Electron_data.Ec.Brand.Koutsuu.Kamei, p->Brand.Koutsuu.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Koutsuu.Kamei));									// 加盟店名
		PayData.Electron_data.Ec.Brand.Koutsuu.TradeKind = p->Brand.Koutsuu.TradeKind;					// 取引種別
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
		break;
	case	BRANDNO_EDY:
		PayData.Electron_data.Ec.e_pay_kind					= EC_EDY_USED;								// 決算種別
// MH810103 MHUT40XX(S) Edy・WAON対応
//		PayData.Electron_data.Ec.Brand.Edy.DealNo			= p->Brand.Edy.DealNo;						// 取引通番
//		PayData.Electron_data.Ec.Brand.Edy.CardDealNo		= p->Brand.Edy.CardDealNo;					// カード取引通番
		memcpy(PayData.Electron_data.Ec.Brand.Edy.DealNo, &p->Brand.Edy.DealNo, 10);					// Edy取引通番
		memcpy(PayData.Electron_data.Ec.Brand.Edy.CardDealNo, &p->Brand.Edy.CardDealNo, 5);				// カード取引通番
		memcpy(PayData.Electron_data.Ec.Brand.Edy.Kamei, p->Brand.Edy.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Edy.Kamei));										// 加盟店名
// MH810103 MHUT40XX(E) Edy・WAON対応
		memcpy(PayData.Electron_data.Ec.Brand.Edy.TerminalNo, &p->Brand.Edy.TerminalNo, 8);				// 端末番号
		break;
	case	BRANDNO_NANACO:
		PayData.Electron_data.Ec.e_pay_kind					= EC_NANACO_USED;							// 決算種別
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//		PayData.Electron_data.Ec.Brand.Nanaco.DealNo		= p->Brand.Nanaco.DealNo;					// 上位取引通番
//		memcpy(PayData.Electron_data.Ec.Brand.Nanaco.TerminalNo, &p->Brand.Nanaco.TerminalNo, 12);		// 上位端末ID
		memcpy(PayData.Electron_data.Ec.Brand.Nanaco.Kamei, p->Brand.Nanaco.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Nanaco.Kamei));									// 加盟店名
		memcpy(PayData.Electron_data.Ec.Brand.Nanaco.DealNo, &p->Brand.Nanaco.DealNo, 6);				// 端末取引通番
		memcpy(PayData.Electron_data.Ec.Brand.Nanaco.TerminalNo, &p->Brand.Nanaco.TerminalNo, 20);		// 上位端末ID
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
		break;
	case	BRANDNO_WAON:
		PayData.Electron_data.Ec.e_pay_kind					= EC_WAON_USED;								// 決算種別
		PayData.Electron_data.Ec.Brand.Waon.point_status	= p->Brand.Waon.PointStatus;				// 利用可否
		PayData.Electron_data.Ec.Brand.Waon.point			= p->Brand.Waon.GetPoint;					// 今回ポイント
// MH810103 MHUT40XX(S) Edy・WAON対応
//		PayData.Electron_data.Ec.Brand.Waon.point_total		= p->Brand.Waon.TotalPoint;					// 累計ポイント
//		PayData.Electron_data.Ec.Brand.Waon.period_point	= p->Brand.Waon.PeriodPoint;				// ２年前までに獲得したポイント
//		PayData.Electron_data.Ec.Brand.Waon.period			= p->Brand.Waon.Period;						// ２年前までに獲得したポイントの有効期限
//		PayData.Electron_data.Ec.Brand.Waon.card_type		= p->Brand.Waon.CardType;					// カード分類
//		PayData.Electron_data.Ec.Brand.Waon.deal_code		= p->Brand.Waon.DealCode;					// 取引種別コード
		memcpy(PayData.Electron_data.Ec.Brand.Waon.point_total, p->Brand.Waon.TotalPoint,
				sizeof(PayData.Electron_data.Ec.Brand.Waon.point_total));								// 累計ポイント
		memcpy(PayData.Electron_data.Ec.Brand.Waon.Kamei, p->Brand.Waon.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Waon.Kamei));										// 加盟店名
// MH810103 MHUT40XX(E) Edy・WAON対応
		memcpy(PayData.Electron_data.Ec.Brand.Waon.SPRW_ID, &p->Brand.Waon.SPRW_ID, sizeof(PayData.Electron_data.Ec.Brand.Waon.SPRW_ID));	// SPRWID
		break;
	case	BRANDNO_SAPICA:
		PayData.Electron_data.Ec.e_pay_kind					= EC_SAPICA_USED;							// 決算種別
		PayData.Electron_data.Ec.Brand.Sapica.Details_ID	= p->Brand.Sapica.Details_ID;				// 明細ID
		memcpy(PayData.Electron_data.Ec.Brand.Sapica.Terminal_ID, &p->Brand.Sapica.Terminal_ID, sizeof(PayData.Electron_data.Ec.Brand.Sapica.Terminal_ID));	// 物販端末ID
		break;
	case	BRANDNO_ID:
		PayData.Electron_data.Ec.e_pay_kind					= EC_ID_USED;								// 決算種別
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//		PayData.Electron_data.Ec.Brand.Id.Details_ID
//		PayData.Electron_data.Ec.Brand.Id.Terminal_ID
		memcpy(PayData.Electron_data.Ec.Brand.Id.Kamei, p->Brand.Id.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Id.Kamei));										// 加盟店名
		memcpy(PayData.Electron_data.Ec.Brand.Id.Approval_No, &p->Brand.Id.Approval_No, 7);				// 承認番号
		memcpy(PayData.Electron_data.Ec.Brand.Id.TerminalNo, &p->Brand.Id.TerminalNo, 13);				// 上位端末ID
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
		break;
	case	BRANDNO_QUIC_PAY:
		PayData.Electron_data.Ec.e_pay_kind					= EC_QUIC_PAY_USED;							// 決算種別
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//		PayData.Electron_data.Ec.Brand.Id.Details_ID
//		PayData.Electron_data.Ec.Brand.Id.Terminal_ID
		memcpy(PayData.Electron_data.Ec.Brand.Quickpay.Kamei, p->Brand.QuicPay.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Quickpay.Kamei));									// 加盟店名
		memcpy(PayData.Electron_data.Ec.Brand.Quickpay.Approval_No, &p->Brand.QuicPay.Approval_No, 7);	// 承認番号
		memcpy(PayData.Electron_data.Ec.Brand.Quickpay.TerminalNo, &p->Brand.QuicPay.TerminalNo, 13);	// 上位端末ID
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
		break;
// MH810105(S) MH364301 PiTaPa対応
	case	BRANDNO_PITAPA:
		PayData.Electron_data.Ec.e_pay_kind					= EC_PITAPA_USED;							// 決算種別
		memcpy(PayData.Electron_data.Ec.Brand.Pitapa.Kamei, p->Brand.Pitapa.Kamei,
				sizeof(PayData.Electron_data.Ec.Brand.Pitapa.Kamei));									// 加盟店名
		memcpy(PayData.Electron_data.Ec.Brand.Pitapa.Slip_No, &p->Brand.Pitapa.Slip_No, 5);				// 伝票番号
		memcpy(PayData.Electron_data.Ec.Brand.Pitapa.TerminalNo, &p->Brand.Pitapa.TerminalNo, 13);		// 上位端末ID
		memcpy(PayData.Electron_data.Ec.Brand.Pitapa.Approval_No, &p->Brand.Pitapa.Approval_No, 8);		// 承認番号
		break;
// MH810105(E) MH364301 PiTaPa対応
	default:
		break;
	}

// MH810103 GG119202(S) 電子マネー対応
	// みなし決済？
	if( PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 1 ){
		// 20byteを0x20埋め
		memset( &PayData.Electron_data.Ec.Card_ID[0], 0x20, sizeof(PayData.Electron_data.Ec.Card_ID) );
		if( EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED] ) {
			// それぞれのカード桁数分0x30埋め
			memset( &PayData.Electron_data.Ec.Card_ID[0], 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]) );
			// 左詰めでZZ
			memset( &PayData.Electron_data.Ec.Card_ID[0], 'Z', 2 );
		}
	}
// MH810103 GG119202(E) 電子マネー対応
// MH810103 GG119202(S) 未了残高照会完了時のカード番号はマスクして印字する
	else if (p->Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
		// 未了残高照会完了時の決済結果データにはマスクされないカード番号が通知されるため、
		// ここでカード番号をマスクする
		switch (p->brand_no) {
		case BRANDNO_KOUTSUU:
			max = ECARDID_SIZE_KOUTSUU - 4;
			for (i = 2; i < max; i++) {
				PayData.Electron_data.Ec.Card_ID[i] = '*';		// 上位2桁、下位4桁以外をマスク
			}
			break;
// MH810103 MHUT40XX(S) Edy・WAON対応
		case BRANDNO_EDY:
// MH810104(S) nanaco・iD・QUICPay対応2
//// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//		case BRANDNO_NANACO:
//		case BRANDNO_ID:
//// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
// MH810104(E) nanaco・iD・QUICPay対応2
			memset(&PayData.Electron_data.Ec.Card_ID[0], '*', 12);	// 下位4桁以外をマスク
			break;
		case BRANDNO_WAON:
			// WAONはマスクされて通知される
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//			// 未了残高照会完了時はポイントメッセージを印字しない仕様のため、
//			// 0xFFをセットしておく
//			PayData.Electron_data.Ec.Brand.Waon.point_status = 0xFF;
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810104 GG119202(S) nanaco・iD・QUICPay対応
		case BRANDNO_NANACO:
			// nanacoはマスクされて通知される
// MH810104 GG119202(E) nanaco・iD・QUICPay対応
			break;
// MH810103 MHUT40XX(E) Edy・WAON対応
// MH810104(S) nanaco・iD・QUICPay対応2
//// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//		case BRANDNO_QUIC_PAY:
//			memset(&PayData.Electron_data.Ec.Card_ID[0], '*', 16);	// 下位4桁以外をマスク
//			break;
//// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
		case BRANDNO_ID:
		case BRANDNO_QUIC_PAY:
			// iD, QUICPayは印字時にカード番号をマスクする
			break;
// MH810104(E) nanaco・iD・QUICPay対応2
		default:
			break;
		}
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
		// 決済結果データを保存する
		memcpy(&EcAlarm.Ec_Res, p, sizeof(*p));
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
	}
// MH810103 GG119202(E) 未了残高照会完了時のカード番号はマスクして印字する
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
	if (p->Result == EPAY_RESULT_MIRYO ||
		p->Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
		// 未了確定、未了残高照会完了
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//		if (p->brand_no != BRANDNO_ID && p->brand_no != BRANDNO_QUIC_PAY) {
//			PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
//		}
// MH810105 GG119202(E) 処理未了取引集計仕様改善
		if (p->brand_no == BRANDNO_WAON) {
			// 未了確定時はポイントメッセージを印字しないため、0xFFをセットしておく
			PayData.Electron_data.Ec.Brand.Waon.point_status = 0xFF;
		}
	}
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810105(S) MH364301 QRコード決済対応（精算中止データ送信対応）
	if (PayData.EcResult == EPAY_RESULT_MIRYO ||
		PayData.EcResult == EPAY_RESULT_NG) {
		// 決済結果＝未了確定、決済NGの場合は精算中止データ送信対象とする
		PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
	}
// MH810105(E) MH364301 QRコード決済対応（精算中止データ送信対応）

// MH810105(S) MH364301 QRコード決済対応
//	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.e_pay_kind), NTNET_152_ECARDKIND);
//	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.pay_ryo), NTNET_152_EPAYRYO);
//// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
////	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.Card_ID[0]), NTNET_152_ECARDID);
////	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
//	if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
//		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
//		// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.をカード桁数分0x30埋めし左詰めでZZする。
//		// [*]印字みなし決済と同様の会員Noをセットする。
//		memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
//		// それぞれのカード桁数分0x30埋め
//		memset( &dummy_Card_ID[0], 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]) );
//		// 左詰めでZZ
//		memset( &dummy_Card_ID[0], 'Z', 2 );
//		NTNET_Data152Save((void *)(&dummy_Card_ID[0]), NTNET_152_ECARDID);
//
//		// 問合せ番号は0x20h埋めする。
//		memset( &dummy_inquiry_num[0], 0x20, sizeof(dummy_inquiry_num) );
//		NTNET_Data152Save((void *)(&dummy_inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
//	}
//	else {
//		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.Card_ID[0]), NTNET_152_ECARDID);
//		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
//	}
//// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//	NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.pay_after), NTNET_152_EPAYAFTER);
	Ec_Data152Save();
// MH810105(E) MH364301 QRコード決済対応
}

/*[]----------------------------------------------------------------------[]*/
/*| 精算領域へのﾃﾞｰﾀの格納 (決済リーダでのクレジット決済)                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcCreUpdatePayData					                   |*/
/*| PARAMETER    : *buf		: 取引ﾃﾞｰﾀ				                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : hosoda                                                  |*/
/*| Date         : 2019-02-18                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	EcCreUpdatePayData( void *buf )
{
	EC_SETTLEMENT_RES	*p;
// MH810105(S) MH364301 QRコード決済対応
//// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//	char	dummy_card_no[20];
//// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
// MH810105(E) MH364301 QRコード決済対応

	p = (EC_SETTLEMENT_RES*)buf;

	//集計・領収書 用 (クレジット)
	memset( &ryo_buf.credit, 0, sizeof(credit_use) );

	memcpy( &ryo_buf.credit.card_name[0], &p->Brand.Credit.Credit_Company[0], sizeof(ryo_buf.credit.card_name) );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社略号
	memcpy( &ryo_buf.credit.card_no[0], &p->Card_ID[0], sizeof(ryo_buf.credit.card_no) );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員番号（右詰め）
	ryo_buf.credit.slip_no				= (ulong)p->Brand.Credit.Slip_No;					// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	ryo_buf.credit.pay_ryo				= (ulong)p->settlement_data;						// クレジット決済額
	memset( &ryo_buf.credit.ShopAccountNo[0], 0x20, sizeof(ryo_buf.credit.ShopAccountNo) );	// 加盟店取引番号, 決済リーダでは無し

	ryo_buf.credit.app_no = astoinl(&p->Brand.Credit.Approval_No[0], sizeof(p->Brand.Credit.Approval_No));	// 承認番号

	memcpy( &ryo_buf.credit.CCT_Num[0], &p->Brand.Credit.Id_No[0], sizeof(p->Brand.Credit.Id_No) );			// 端末識別番号
	memcpy( &ryo_buf.credit.kid_code[0], &p->Brand.Credit.KID_Code[0], sizeof(ryo_buf.credit.kid_code) );	// KID コード
	ryo_buf.credit.Identity_Ptrn = p->Brand.Credit.Identity_Ptrn;											// 本人確認パターン

	//集計・領収書 用 (電子決済)
	if(p->Column_No == 0xFF) {
		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;									// みなし決済
	}
	PayData.Electron_data.Ec.e_pay_kind	= EC_CREDIT_USED;												// 決算種別
	memcpy(PayData.Electron_data.Ec.Card_ID, p->Card_ID, sizeof(PayData.Electron_data.Ec.Card_ID));		// ｶｰﾄﾞID
	PayData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);						// 決済日時(Normalize)
// MH810105 GG119202(S) 処理未了取引集計仕様改善
	PayData.EcResult = p->Result;																		// 決済結果
// MH810105 GG119202(E) 処理未了取引集計仕様改善
// MH810105(S) MH364301 QRコード決済対応（精算中止データ送信対応）
	if (PayData.EcResult == EPAY_RESULT_NG) {
		// 決済結果＝決済NGの場合は精算中止データ送信対象とする
		PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
	}
// MH810105(E) MH364301 QRコード決済対応（精算中止データ送信対応）

// MH810105(S) MH364301 QRコード決済対応
//	//精算情報データ 用
//	NTNET_Data152Save((void *)(&ryo_buf.credit.pay_ryo), NTNET_152_CPAYRYO);				// クレジット決済額
//// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
////	NTNET_Data152Save((void *)(&ryo_buf.credit.card_no[0]), NTNET_152_CCARDNO);				// クレジットカード会員番号
//	if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
//		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
//		// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.を0x20h埋めする。
//		// [*]印字みなし決済と同様の会員Noをセットする。
//		memset( &dummy_card_no[0], 0x20, sizeof(dummy_card_no) );
//		NTNET_Data152Save((void *)(&dummy_card_no[0]), NTNET_152_CCARDNO);					// クレジットカード会員番号
//	}
//	else {
//		NTNET_Data152Save((void *)(&ryo_buf.credit.card_no[0]), NTNET_152_CCARDNO);			// クレジットカード会員番号
//	}
//// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//	NTNET_Data152Save((void *)(&ryo_buf.credit.CCT_Num[0]), NTNET_152_CCCTNUM);				// 端末識別番号
//	NTNET_Data152Save((void *)(&ryo_buf.credit.kid_code[0]), NTNET_152_CKID);				// ＫＩＤコード
//	NTNET_Data152Save((void *)(&ryo_buf.credit.app_no), NTNET_152_CAPPNO);					// 承認番号
//	NTNET_Data152Save((void *)(&ryo_buf.credit.ShopAccountNo[0]), NTNET_152_CTRADENO);		// 加盟店取引番号, 決済リーダでは無し
//	NTNET_Data152Save((void *)(&ryo_buf.credit.slip_no), NTNET_152_SLIPNO);					// 伝票番号
	Ec_Data152Save();
// MH810105(E) MH364301 QRコード決済対応
}
// MH321800(E) hosoda ICクレジット対応

/*[]----------------------------------------------------------------------[]*/
/*|	Mifare ｶｰﾄﾞ検知開始													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2005-11-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	op_MifareStart( void )
{
	if( (MIFARE_CARD_DoesUse) &&									// Mifare使う設定
		(!IsErrorOccuerd(ERRMDL_MIFARE,1)) &&
		( OPECTL.Ope_mod != 22 )){									// 定期更新精算以外
		OpMif_snd( 0xA2, 0 );										// Mifareｶｰﾄﾞｾﾝｽ&ﾃﾞｰﾀ読出し開始
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|	Mifare処理中止（停止）												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2005-11-22                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	op_MifareStop( void )
{
	OpMif_snd( 0xA3, 0 );											// Mifare処理中止
}

void	op_MifareStop_with_LED( void )
{
	OpMif_snd( 0xA3, 0 );											// Mifare処理中止
}

void	op_Cycle_BUZPIPI( void )
{
	BUZPIPI();
	LagTim500ms( LAG500_MIF_WRITING_FAIL_BUZ, (1*2), op_Cycle_BUZPIPI );	// 1秒後 再ｴﾗｰﾌﾞｻﾞｰ音
}

void	op_IccLedOff( void )
{
}

void	op_IccLedOff2( void )
{
	queset(OPETCBNO, TIMEOUT12, 0, NULL);
	LagTim500ms( LAG500_MIF_LED_ONOFF, (2*2), op_IccLedOnOff );		// 2秒後 ICC LED再点滅
}

void	op_IccLedOnOff( void )
{
}

// エラー表示を元の表示に戻す
void op_RestoreErrDsp( uchar dsp ) {
	if( dsp ){
		Lcd_Receipt_disp();
	}else{
		dsp_intime( 7, OPECTL.Pr_LokNo );			// 入庫時刻表示
	}
// MH810105(S) MH364301 インボイス対応
	OPECTL.f_CrErrDisp = 0;							// エラー非表示
// MH810105(E) MH364301 インボイス対応
}

/*[]----------------------------------------------------------------------[]*/
/*| 新修正精算（syuseiﾊﾞｯﾌｧに格納されているﾃﾞｰﾀをvl_frsにｾｯﾄする）         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SyuseiDataSet(void)                                     |*/
/*| PARAMETER    : mot_lkno : 修正元フラップ№                             |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Akiba                                                 |*/
/*| Date         : 2008-03-07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SyuseiDataSet( ushort mot_lkno )
{
	cm27();
	memset(&vl_frs, 0, sizeof(vl_frs));

	vl_frs.lockno	= mot_lkno;										// 修正元・フラップ／ロック装置No.(1-324)
	vl_frs.price	= syusei[mot_lkno-1].ryo;						// 修正元・支払額（料金＋外税）
	vl_frs.in_price = syusei[mot_lkno-1].gen  						// 修正元・現金
								+ syusei[mot_lkno-1].sy_wari;		// 修正元・料金割引,プリペイド,回数券,クレジット,Eマネー入金額
	vl_frs.syubetu	= syusei[mot_lkno-1].syubetu;					// 修正元料金種別

	vl_frs.seisan_oiban = syusei[mot_lkno-1].oiban;					// 修正元精算追い番
	memcpy( &vl_frs.seisan_time.Year, &syusei[mot_lkno-1].oyear, 6 );

	// 定期券が使われたか分かるためここでチェックする。
	if( syusei[mot_lkno-1].tei_syu ){
		vl_frs.antipassoff_req = 1;									// アンチパス強制OFF要求
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| 新修正精算（修正元現金入金額の払戻判定）                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SyuseiModosiPay(void)                                   |*/
/*| PARAMETER    : rt  ：戻り値                                            |*/
/*| RETURN VALUE : ret ：2=釣銭あり                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Akiba                                                 |*/
/*| Date         : 2008-03-07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	SyuseiModosiPay( short rt )
{
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//short	ret;
//ulong	tyu_zan,tmp_ryo;
//ulong	disp_waribik;
//uchar	end_flg = 0;
//
//	ret = rt;
//	if( (ryo_buf.nyukin == 0)&&(ryo_buf.mis_ryo != 0) ){		// 入金なしで修正元現金入金あり
//		ryo_buf.mis_tyu = 0;
//		tmp_ryo = ryo_buf.tyu_ryo;
//		if( tki_flg ){
//			tmp_ryo = ryo_buf.tei_ryo;
//		}
//
//		disp_waribik = ryo_buf.dis_fee + ryo_buf.dis_tim;
//		if( tmp_ryo < disp_waribik ){
//			end_flg = 1;
//		}else{
//			tmp_ryo -= disp_waribik;
//		}
//		if( tmp_ryo < ryo_buf.mis_wari ){						// 駐車料金>修正元割引額
//			end_flg = 1;
//		}
//
//		if( !end_flg ){
//			tyu_zan = tmp_ryo - ryo_buf.mis_wari;				// 駐車料金-修正元割引額（残り駐車料金）
//			if( tyu_zan <= ryo_buf.mis_ryo ){					// 残り駐車料金<修正元現金入金額
//				PayData.MMTwari = ryo_buf.mis_ryo - tyu_zan;
//				if( (prm_get(COM_PRM, S_TYP, 98, 1, 1) == 1)||	// 修正元現金入金額の払戻あり設定
//					(Syusei_Select_Flg == 2) ){					// 修正元現金入金額の払戻あり選択
//					ryo_buf.mis_tyu = ryo_buf.mis_ryo - tyu_zan;// 修正元現金入金額-残り駐車料金（修正払戻額）
//					ret = 2;									// 精算終了(釣有り)
//				}else{
//					PayData.Zengakufg |= 0x04;
//				}
//			}
//		}else{
//			PayData.MMTwari = ryo_buf.mis_ryo;
//			if( (prm_get(COM_PRM, S_TYP, 98, 1, 1) == 1)||		// 修正元現金入金額の払戻あり設定
//				(Syusei_Select_Flg == 2) ){						// 修正元現金入金額の払戻あり選択
//				ryo_buf.mis_tyu = ryo_buf.mis_ryo;
//				ret = 2;
//			}else{
//				PayData.Zengakufg |= 0x04;
//			}
//		}
//	}
//	return ret;
	return rt;
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
}

uchar	SyuseiEndKeyChk( void )
{
uchar	ret,set;

	ret = 0;
	set = (uchar)prm_get(COM_PRM, S_TYP, 98, 1, 3);
	if( set ){													// 強制完了ｷｰあり
		if( OPECTL.Pay_mod == 2 ){								// 修正精算中
			ret = 2;											// ｺｲﾝ、紙幣、電子決済、磁気ﾘｰﾀﾞ、Mifareだめ
			if( (set == 1)&&									// 定期の挿入あり
				(vl_frs.antipassoff_req)&&(tki_flg==OFF) ){		// 修正元で定期未使用又は定期券使用済み
				ret = 1;										// ｺｲﾝ、紙幣、電子決済だめ
			}
		}
	}
	return(ret);
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 通知番号を計算する																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: op_Calc_Notice( ushort stimer, struct clk_rec *pDate, short sParkNo )			   |*/
/*| PARAMETER	: ushort stimer,		: 10msごとのタイマー値									   |*/
/*|               struct clk_rec *pDate : 日時													   |*/
/*|               short sParkNo         : 駐車場番号											   |*/
/*| RETURN VALUE: ret                   : 通知番号												   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																		   |*/
/*| Date		: 2006-02-21																	   |*/
/*| Update		:																				   |*/
/*| 備考		:NT4500EXからの流用ロジック（変更あり） 										   |*/
/*[]------------------------------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static long	op_Calc_Notice( ushort stimer, struct clk_rec *pDate, long sParkNo )/*								*/
{																				/*								*/
	long motodat1, motodat2, worknum, hash;										/*								*/
	long nen, tuki, hi, ji, fun;												/*								*/
	long chunum, timer;															/*								*/
																				/*								*/
	nen = (long)( pDate->year % GETA_10_2 );									/*								*/
	tuki = (long)pDate->mont;													/*								*/
	hi = (long)pDate->date;														/*								*/
	ji = (long)pDate->hour;														/*								*/
	fun = (long)pDate->minu;													/*								*/
	chunum = (long)sParkNo;														/*								*/
	timer  = (long)stimer;														/*								*/
																				/*								*/
    motodat1 = (nen * GETA_10_6 + timer * GETA_10_3 + chunum) ;								/* '8桁データ1作成	*/
    motodat2 = ((hi + fun) * GETA_10_6 + ji * GETA_10_5 + tuki * GETA_10_2 + timer + fun) ;	/* '8桁データ2作成	*/
																				/*								*/
    worknum = (motodat1 ^ motodat2);											/*								*/
    hash = (worknum % MAXVAL);													/*								*/
																				/*								*/
	return( hash );																/*								*/
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 暗証番号を計算する																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: op_Calc_Ansyou( long lNoticeNo )												   |*/
/*| PARAMETER	: long lNoticeNo : 通知番号														   |*/
/*| RETURN VALUE: ret            : 暗証番号														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																		   |*/
/*| Date		: 2006-02-21																	   |*/
/*| Update		:																				   |*/
/*| 備考		:NT4500EXからの流用ロジック（変更あり）	     									   |*/
/*[]------------------------------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static long	op_Calc_Ansyou( long lNoticeNo )									/*								*/
{																				/*								*/
	long ansyo, val_1, Setting[4];												/*								*/
	long seed1, seed2, seed3, shift1, shift2, shift3;							/*								*/
	ushort temp=GETA_10_3,i;
	memset(Setting,0,sizeof(Setting));											/*								*/
	val_1 = lNoticeNo;

	for(i=0;i<4;i++){
		Setting[i] = ONE_CHARACTER_CHECK(decode_number,temp);
		temp/=10;
	}																			/*								*/
																				/*								*/
	shift1 = (long)Setting[0];													/* '１桁目データ				*/
	shift2 = (long)Setting[1];													/* '２桁目データ				*/
																				/*								*/
	shift3 = (shift1 + shift2);													/* データ合成					*/
																				/*								*/
	seed1 = (long)Setting[2];													/* '３桁目データ				*/
	seed2 = (long)Setting[3];													/* '４桁目データ				*/

	seed3 = (seed1 + seed2);													/* データ合成					*/

	shift1 = ( ( shift1 % 4L ) + 1L );											/* 'データがオーバーフローしないように変数を1～4のデータに変換 */
	shift2 = ( ( shift2 % 4L ) + 1L );											/*								*/
	shift3 = ( ( shift3 % 4L ) + 1L );											/*								*/
	seed1 = ( ( seed1 % 4L ) + 1L );											/*								*/
	seed2 = ( ( seed2 % 4L ) + 1L );											/*								*/
	seed3 = ( ( seed3 % 4L ) + 1L );											/*								*/
																				/*								*/ 
	ansyo = ( ( ( val_1 << shift1 ) * op_Calc_BekiJyo( 10, seed1 ) ) ^ ( ( val_1 << shift2 ) * op_Calc_BekiJyo( 10, seed2 ) ) ^ ( ( val_1 << shift3 ) * op_Calc_BekiJyo( 10, seed3 ) ) ^
	          ( ( val_1 << 4 ) * GETA_10_4 ) ^ ( ( val_1 << 1 ) * GETA_10_3 ) ^ ( ( val_1 << 3 ) * GETA_10_2 ) );


	ansyo = ( ( ( val_1 << shift1 ) * op_Calc_BekiJyo( 10, seed1 ) ) ^ ( ( val_1 << shift2 ) * op_Calc_BekiJyo( 10, seed2 ) ) ^ ( ( val_1 << shift3 ) * op_Calc_BekiJyo( 10, seed3 ) ) ^
	          ( ( val_1 << 4 ) * GETA_10_4 ) ^ ( ( val_1 << 1 ) * GETA_10_3 ) ^ ( ( val_1 << 3 ) * GETA_10_2 ) );

																				/* '通知番号を暗証番号に加工	*/
																				/*								*/
																				/*								*/
	ansyo = (ansyo % MAXVAL);													/* '暗証番号を4桁に変換			*/
																				/*								*/
																				/*								*/
	seed1 = ansyo / GETA_10_3;													/* 1000の桁						*/
																				/*								*/
	seed2 = (ansyo / GETA_10_2 ) % GETA_10_1;									/* 100の桁						*/
																				/*								*/
	seed3 = (ansyo / GETA_10_1 ) % GETA_10_1;									/* 10の桁						*/
																				/*								*/
	shift1 = ansyo % GETA_10_1;													/* 10の桁						*/
																				/*								*/
	if( seed1 == 0){															/* 0?(Y)						*/
		seed1 = ZeroToMitsu;													/* 0->3							*/
	}																			/*								*/
	if( seed2 == 0){															/* 0?(Y)						*/
		seed2 = ZeroToMitsu;													/* 0->3							*/
	}																			/*								*/
	if( seed3 == 0){															/* 0?(Y)						*/
		seed3 = ZeroToMitsu;													/* 0->3							*/
	}																			/*								*/
	if( shift1 == 0){															/* 0?(Y)						*/
		shift1 = ZeroToMitsu;													/* 0->3							*/
	}																			/*								*/
																				/*								*/
	ansyo = seed1*GETA_10_3 + seed2*GETA_10_2 + seed3*GETA_10_1 + shift1;		/*	暗証番号生成　　			*/
																				/*								*/
	return( ansyo );															/*								*/
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| べき乗を計算する																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: op_Calc_BekiJyo( long Val1, long Val2 )										   |*/
/*| PARAMETER	: long Val1 : べき乗される値													   |*/
/*|				: long Val2 : べき乗															   |*/
/*| RETURN VALUE: ret       : 答え																   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																		   |*/
/*| Date		: 2006-02-21																	   |*/
/*| Update		:																				   |*/
/*| 備考		:NT4500EXからの流用ロジック														   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static long	op_Calc_BekiJyo( long Val1, long Val2 )								/*								*/
{																				/*								*/
	long Result;																/*								*/
	long i;																		/*								*/
																				/*								*/
	if ( Val2 <= 0 ) {															/*								*/
		Result = 1L;															/* ？の０乗は１なので、１で初期化 */
	} else {																	/*								*/
		Result = Val1;															/*								*/
		for ( i = 1L ; i < Val2 ; i++ ) {										/*								*/
			Result = ( Result * Val1 );											/*								*/
		}																		/*								*/
	}																			/*								*/
	return Result;																/*								*/
}
unsigned short	Cardress_DoorOpen( void )
{
	long advice_No,ansyou_No;
	short	Save_OPE_red;
	ushort	ret;
	ushort	msg=0;
	ushort	input=0;
	uchar	input_cnt = 0;
	char	exitflag=OFF;
	
	dispclr();												/* Display All Clear */

	// TODO:解読番号は標準用の共通パラメータを取得すること
	decode_number = 4134;

	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[0] );				/* ＜電磁ロック解除＞ */
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[1] );				/* "   ドア開番号を入力し、       " */
// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け
//	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[2] );				/* "   最後に精算を押して下さい   " */
//	Fun_Dsp(FUNMSG[8]);										// "　                       終了 "
	grachr( 4, 0, 9, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[2] );				/* "   最後に                     " */
	PKTcmd_text_1_backgroundcolor( COLOR_F1BLUE );
	grachr( 4, 9, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[4] );				/* " 確定                         " */
	PKTcmd_text_1_backgroundcolor( COLOR_WHITE );
	grachr( 4, 15, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, sercret_Str[5] );				/* "を押して下さい                " */
	Fun_Dsp(FUNMSG[125]);									// " 確定                    終了 "
// MH810100(E) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け
	advice_No = (op_Calc_Notice((ushort)LifeTimGet(),&CLK_REC,(long)CPrmSS[S_SYS][1])%GETA_10_4);		/* 通知番号の生成 */
	ansyou_No = op_Calc_Ansyou(advice_No);						/* 暗証番号の生成 */
	
// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
//	teninb((ushort)advice_No,4,1,9, COLOR_BLACK);			/* 通知番号の表示 */
	teninb((ushort)advice_No,4,1,11, COLOR_BLACK);			/* 通知番号の表示 */
// MH810100(E) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）

	Save_OPE_red = OPE_red;
	OPE_red = 2;										// ﾘｰﾀﾞｰ自動排出

	for ( ret = 0; ret == 0; ) {
		msg = StoF( GetMessage(), 1 );
		if (msg == KEY_MODECHG){
			BUZPI();
			ret = MOD_CHG;
		}
		switch (KEY_TEN0to9(msg)) {
			case KEY_TEN_F5:														/* 終了(F5) */
				if(exitflag == OFF){
					BUZPI();
					ret = MOD_EXT;
				}
				break;
			case KEY_TEN_F3:														/* 書込(F4) */
			case KEY_TEN_F2:														/* －(F2) */
				break;																
			case KEY_TEN_F1:														/* ＋(F1) */
				if (ansyou_No != input) {
					//暗証番号(Btype)が正しくなかったら
					BUZPIPI();
					input = 0;
					input_cnt = 0;
// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
//					teninb_pass2( input, 4, 5, 9, input_cnt );		// 番号表示
					teninb_pass2( input, 4, 5, 11, input_cnt );		// 番号表示
// MH810100(E) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
					break;
				}
				BUZPI();
				Lagcan( OPETCBNO, 16 );									// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸﾀｲﾏｰﾘｾｯﾄ
				Lagcan( OPETCBNO, 17 );									// ﾄﾞｱ閉ﾉﾌﾞ閉後 ﾉﾌﾞﾛｯｸ待ちﾀｲﾏｰﾘｾｯﾄ
				grachr(3, 0, 30, 0, COLOR_RED, LCD_BLINK_OFF, sercret_Str[3]);					/* "<<ドアロックを解除しました>>" */
				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLSMSG[0]);						/* "                            " */
				Lagtim( OPETCBNO, 6, 3*50 );								/* タイマ－設定 */
				exitflag = ON;
				OPE_SIG_OUT_DOORNOBU_LOCK(0);								// ﾄﾞｱﾉﾌﾞのﾛｯｸ解除
				wmonlg( OPMON_LOCK_OPEN, 0, 0 );							// ﾓﾆﾀ登録
				attend_no = 0;
				wopelg2( OPLOG_NO_CARD_OPEN, 0L, (ulong)attend_no );		// 操作履歴登録(券無開時)
				if(prm_get(COM_PRM, S_PAY, 17, 3, 1)){						// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰ値が有効範囲内
					DoorLockTimer = 1;
					Lagtim( OPETCBNO, 16, (ushort)(prm_get(COM_PRM, S_PAY, 17, 3, 1)*50) );	// ﾄﾞｱ開ﾉﾌﾞ 再ﾛｯｸ時間ﾀｲﾏｰｽﾀｰﾄ
				}
				break;

			case KEY_TEN:			/* 数字(テンキー) */
				BUZPI();
				input =  ((input%1000) * 10 )+( msg - KEY_TEN );
				input_cnt++;
				if(input_cnt > 4){
					input_cnt = 4;
				}
// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
//				teninb_pass2( input, 4, 5, 9, input_cnt);	// 表示書換え
				teninb_pass2( input, 4, 5, 11, input_cnt);	// 表示書換え
// MH810100(E) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
				break;

			case KEY_TEN_CL:		// ﾃﾝｷｰ[C] ｲﾍﾞﾝﾄ
				BUZPI();
				input = 0;
				input_cnt = 0;
// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
//				teninb_pass2( input, 4, 5, 9, input_cnt);	// 駐車位置番号表示
				teninb_pass2( input, 4, 5, 11, input_cnt);	// 駐車位置番号表示
// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
				break;
			case TIMEOUT6:															/* 4桁入力後かつ画面遷移用ﾀｲﾑｱｳﾄ */
//				return 0;
				ret = MOD_EXT;
			default:
				break;
		}
	}

	OPE_red = Save_OPE_red;
	return	ret;
}

static void	teninb_pass2( ushort nu, ushort in_k, ushort lin, ushort col, uchar pas_cnt)
{
	uchar	c_bf[4];
	ushort	wcol;
	short	i;
	uchar	p_cnt;

	nu %= 10000;
	if( in_k > 4 )	in_k = 4;

	intoas( c_bf, nu, in_k );

// MH810100(S) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
//	for( i=0, wcol=0; i<in_k; i++, wcol+=3 ){
	for( i=0, wcol=0; i<in_k; i++, wcol+=2 ){
// MH810100(E) S.Takahashi 2020/02/12 #3853 電磁ロック解除画面で文字化け（暗証番号が表示されない）
		if(pas_cnt == 0){
			//空白表示
			p_cnt = 0;
			c_bf[p_cnt] = ' ';
			numchr(lin, (ushort)(col+wcol), COLOR_FIREBRICK, LCD_BLINK_OFF, c_bf[p_cnt]);
		}else{
			p_cnt = (uchar)(4 - pas_cnt);
			numchr(lin, (ushort)(col+wcol), COLOR_FIREBRICK, LCD_BLINK_OFF, c_bf[p_cnt]);
			pas_cnt--;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*| 磁気リーダ内のカード詰まり発生処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : req = 要求動作											|*/
/*|							0:初期化処理									|*/
/*|							1:精算完了or精算中止時の処理					|*/
/*|							2:排出動作後まだ詰まっていたらタイマ開始		|*/
/*|							3:連続動作抑止時間経過							|*/
/*|							4:取り消しボタンによるカード排出処理			|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Ise                                                      |*/
/*| Date         : 2009-06-18                                               |*/
/*| Update       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void StackCardEject(uchar req)
{
	uchar RetryMax;																/* 自動排出回数設定値 */

	uchar retry_wk = CardStackRetry;
	RetryMax = (uchar)(prm_get(COM_PRM, S_SYS, 14, 2, 1));						/* 自動排出回数設定取得 */
	switch(req)																	/* 要求毎に処理を分岐 */
	{
	case 1:																		/* 精算中止or精算完了 */
		if(IS_ERR_RED_STACK && RetryMax != 0){
			if(OutputRelease != 0 || RetryMax <= EjectActionCnt){
				EjectActionCnt = 0;
				CardStackStatus = 0;
			if(!rd_mov(1))														/* カード排出要求 */
			{
				wmonlg(OPMON_CARDSTACK_OCC, 0, 0);								/* 排出動作開始モニタログ登録 */
				OutputRelease = 0;												/* 次のエラー解消ログ登録あり */
				CardStackRetry = 1;												/* 排出動作開始 */
			}
			}
		}
		else if(!IS_ERR_RED_STACK && CardStackRetry)																	/* カード詰まり解除中 */
		{
			if(!OutputRelease)													/* エラー解除によるログ登録あり */
			{
				wmonlg(OPMON_CARDSTACK_REL, 0, 0);								/* 排出動作完了モニタログ登録 */
				OutputRelease = 1;												/* 次からエラー解除ログ登録なし */
			}
			else																/* エラー解除ログ登録なし */
			{
				OutputRelease = 2;												/* ログタスクからのカード詰まり通知待ち */
			}
			CardStackRetry = 0;													/* 排出動作動作終了 */
		}
		break;																	/* 分岐処理終了 */
	case 2:																		/* 排出動作後の磁気リーダ通知 */
		if(!CardStackRetry){
			/* 排出動作中以外の動作完了はリードコマンド出さない */
			break;
		}
		if(IS_ERR_RED_STACK)													/* カード詰まり発生中 */
		{
			if(!CardStackStatus){											// ﾀｲﾏｰ起動していない
				EjectActionCnt++;
			if((RetryMax == 99) || (EjectActionCnt < RetryMax))					/* 自動排出動作設定回数分繰り返す */
			{
				Lagtim(OPETCBNO, 3, CARD_EJECT_AUTOLAG);						/* 自動排出動作タイマー開始 */
			}
			else																/* 自動排出動作が設定回数実行済み */
			{
				EjectSafetyTime = LifeTimGet();									/* 取り消しボタンでの排出操作開始 */
				CardStackRetry = 0;												/* 排出動作終了*/
			}
			CardStackStatus = 1;												/* 排出有効タイマ開始 */
			}
		}
		else																	/* カード詰まりが解消 */
		{
			EjectActionCnt = 0;													/* 排出動作回数初期化 */
			CardStackStatus = 0;												/* 初回排出動作可能 */
			CardStackRetry = 0;													/* 排出動作終了 */
			if(!OutputRelease)													/* エラー解除によるログ登録あり */
			{
				wmonlg(OPMON_CARDSTACK_REL, 0, 0);								/* 排出動作完了モニタログ登録 */
				OutputRelease = 1;												/* 次からエラー解除ログ登録なし */
			}
		}
		break;																	/* 分岐処理終了 */
	case 3:																		/* カード詰まり中の取り消しボタン */
		if(CardStackStatus == 1)												/* 排出動作有効タイマ動作中 */
		{
			if((EjectActionCnt >= RetryMax) && (RetryMax != 99 && RetryMax != 0))	/* 設定回数分の自動排出を実行した */
			{
				if(CARD_EJECT_MANULAG <= LifePastTimGet(EjectSafetyTime))		/* 排出操作無効時間経過 */
				{
					if(!rd_mov(1))												/* カード排出要求 */
					{
						CardStackStatus = 0;									/* 排出動作要求済み */
						wmonlg(OPMON_CARDSTACK_OCC, 0, 0);								/* 排出動作開始モニタログ登録 */
						OutputRelease = 0;												/* 次のエラー解消ログ登録あり */
						EjectActionCnt = 0;
						CardStackRetry = 1;										/* 排出動作開始 */
					}
				}
			}
		}
		break;																	/* 分岐処理終了 */
	case 4:																		/* 自動排出動作タイマ割り込み */
		if(CardStackStatus == 1)												/* 排出動作有効タイマ動作中 */
		{
			if((EjectActionCnt < RetryMax) || (RetryMax == 99))					/* 自動排出を設定回数分繰り返す */
			{
				if(!rd_mov(1))													/* カード排出要求 */
				{
					CardStackStatus = 0;										/* 排出動作要求済み */
				}
			}
		}
		break;																	/* 分岐処理終了 */
	case 5:																		/* ログタスクよりカード詰まり通知 */
		if(IS_ERR_RED_STACK && (RetryMax > EjectActionCnt)){					/* カード詰まり発生中 */
			if(EjectActionCnt == 0 && CardStackRetry == 0){
				wmonlg(OPMON_CARDSTACK_OCC, 0, 0);								/* 排出動作開始モニタログ登録 */
				if(!rd_mov(1))													/* カード排出要求 */
				{
					CardStackRetry = 1;											/* 排出動作中*/
					OutputRelease = 0;											/* 次のエラー解消ログ登録あり */
				}
			}
		}
		else if(!OutputRelease)													/* エラー解除によるログ登録あり */
		{
			if(!IS_ERR_RED_STACK){
				EjectActionCnt = 0;													/* 排出動作回数初期化 */
				CardStackStatus = 0;												/* 初回排出動作可能 */
				wmonlg(OPMON_CARDSTACK_REL, 0, 0);								/* 排出動作完了モニタログ登録 */
				OutputRelease = 1;												/* 次からエラー解除ログ登録なし */
				CardStackRetry = 0;
			}
		}
		break;																	/* 分岐処理終了 */
	default:																	/* 上記以外 */
		break;																	/* 規定外はなにもしない */
	}
	if(retry_wk && !CardStackRetry && !IS_ERR_RED_STACK){					// 今回カード詰まりが解消した
		opr_snd( 3 );
	}

}
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
///*[]----------------------------------------------------------------------[]*/
///*| テンキー入力保持関数                                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : Key_Event_Set( msg )         	                       |*/
///*| PARAMETER    : msg:ｲﾍﾞﾝﾄ種別                                           |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       : namioka                                                 |*/
///*| Date         : 2009-10-26                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
//void	Key_Event_Set( ushort msg )
//{
//	uchar	i;
//	uchar	param;
//	
//	param = (uchar)prm_get(COM_PRM, S_TYP, 97, 1, 1);
//	
//	switch( param ){
//		case	1:
//			break;
//		case	2:
//			if( SFV_DAT.safe_cal_do != 0 )
//				return;
//			break;
//		case	0:
//		default:
//			return;
//	}
//		
//	if( KEY_TEN0to9(msg) == KEY_TEN ){
//		for( i=0; i < TBL_CNT(key_event_keep); i++ ){
//			if( !key_event_keep[i] ){
//				key_event_keep[i] = msg;
//				break;
//			}
//		}
//		
//		if( i == TBL_CNT(key_event_keep) ){
//			memmove( &key_event_keep[0], &key_event_keep[1], (size_t)(sizeof( key_event_keep )-1));
//			key_event_keep[i-1] = msg;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*| テンキー入力復元関数                                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : Key_Event_Get( msg )         	                       |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN		 : ret:変換後の車室番号                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       : namioka                                                 |*/
///*| Date         : 2009-10-26                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
//ushort	Key_Event_Get( void )
//{
//	uchar	i;
//	ushort	ret=0;
//	
//	for( i=0; i < TBL_CNT(key_event_keep); i++ ){
//		if( key_event_keep[i] ){
//			ret = (ushort)(( ret % knum_ket ) * 10 )+( key_event_keep[i] - KEY_TEN );
//		}
//	}
//	memset( key_event_keep, 0, sizeof( key_event_keep ));
//	return ret;
//}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_

//[]----------------------------------------------------------------------[]
///	@brief			問い合わせ中などのイベント抑止チェック
//[]----------------------------------------------------------------------[]
///	@return			ret 0:抑止なし 1:抑止
///	@author			
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/10/27<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char	Op_Event_Disable( ushort msg )
{
	char ret=0;
	char i;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	if( edy_auto_com ){
//		switch( msg ){
//			case CLOCK_CHG:						// 時計は例外で時刻の更新を行わせる為に処理を行う
//				break;
//			case ARC_CR_R_EVT:					// ｶｰﾄﾞIN
//				read_sht_opn();					// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ開, 磁気ﾘｰﾀﾞｰｶﾞｲﾄﾞLED点滅
//				if( OPE_red != 2 ){				// 自動排出モードではない場合
//					opr_snd( 13 );				// 保留位置から取出し口へ
//				}
//				ope_anm( AVM_CARD_ERR1 );	// 音声｢このカードは使えません｣
//				ret = 2;
//				break;
//			case ARC_CR_E_EVT:					
//				rd_shutter();					// シャッターを閉める
//				break;
//			case KEY_DOOR1:				// ドア開閉通知（ドアが開いているのか、閉まっているのかはDoorCloseChk()で判断する）
//				msg = KEY_DOOR1_DUMMY;
//				ret = 1;						// イベント保持して、センター通信後に行う
//				break;
//
//			case TIMEOUT1:						// バックライト、画面制御用タイマー（多目的タイマー）
//			case TIMEOUT4:						// 不正強制出庫監視用タイマー
//			case TIMEOUT7:						// シャッター開状態継続監視タイマー
//			case TIMEOUT27:						// 人体検出信号マスク時間
//			case OPE_OPNCLS_EVT:				// 休業状態変化通知
//			case MID_RECI_SW_TIMOUT:			// 領収証発行可能ﾀｲﾏｰﾀｲﾑｱｳﾄ通知
//			// KEY_INSIG_LC1やKEY_MODECHG等は通信中にON→OFF、OFF→ONされる可能性もあるので
//			// イベント保持せず、タイムアウト後に状態を見て判断する
//				ret = 1;						// イベント保持して、センター通信後に行う
//				break;
//			default:
//				ret = 2;						// その他のイベントは受け捨てる
//				break;
//		}
//	}
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	if( ret == 1 ){											// ｲﾍﾞﾝﾄ登録条件？
		for( i=0;i<KEEP_EVT_MAX && Keep_Evt_Buf[i];i++ );
		if(i >= KEEP_EVT_MAX){
			i = KEEP_EVT_MAX -1;// 範囲外へのアクセスブロック
		}
		Keep_Evt_Buf[i] = msg;								// ｲﾍﾞﾝﾄ保持
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			センター通信中などに抑止したイベントチェック
//[]----------------------------------------------------------------------[]
///	@return			0:要求なし 1:画面遷移条件あり
///	@author			T.Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/12/08<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char	Op_Event_enable( void )
{
	ushort evt=0;
	char i;
	
	while( 1 ){

		if( CP_MODECHG == 1 ){
			evt = KEY_MODECHG;							// ﾒﾝﾃ鍵 ON
			break;
		}
		// 他の条件を付け加える場合はここに記載↓

		// 他の条件を付け加える場合はここに記載↑
		break;
	}
	
	if( evt ){
		OPECTL.on_off = 1;
		queset( OPETCBNO, evt, sizeof(OPECTL.on_off), &OPECTL.on_off );		// 保留中のｲﾍﾞﾝﾄを通知
	}
	
	for( i=0; i<KEEP_EVT_MAX; i++ ){
		if( Keep_Evt_Buf[i] ){
			queset( OPETCBNO, Keep_Evt_Buf[i], 0, 0 );		// 保留中のｲﾍﾞﾝﾄを通知
		}else{
			break;
		}
	}
	memset( Keep_Evt_Buf, 0, sizeof( Keep_Evt_Buf ));
	
	return(char)(evt != 0);
}

/*[]----------------------------------------------------------------------[]*/
/*| RTCへのフラグレジスタ取得要求関数                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_RtcFlagRegisterRead     	  	                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN		 : void 				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : tanaka                                                  |*/
/*| Date         : 2012-1-13                                               |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	op_RtcFlagRegisterRead( void )
{
	I2C_REQUEST 			request;
	ushort					ret;

	request.TaskNoTo	 			= I2C_TASK_OPE;							// opeタスクへI2CREQUEST処理完了を通知
	request.DeviceNo 				= I2C_DN_RTC;							// RTCへの要求
	request.RequestCode    			= I2C_RC_RTC_STATUS_RCV;				// 
	request.I2cReqDataInfo.RWCnt	= RTC_FR_SIZE;							// 
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.flag_reg;					// 
	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );					// 
	if( ret != RIIC_RSLT_OK ){
		// ここでエラー登録等はやりますかね？
	}

	if( (RTC_CLOCK.flag_reg & 0x03) != 0x00 )
	{
		//VLF(発振回路電圧低下)の場合
		if((RTC_CLOCK.flag_reg & 0x02) == 0x02)
		{
			err_chk( ERRMDL_MAIN, ERR_MAIN_RTCCPU_VOLDWN, 2, 0, 0);			/* VLF(発振回路電圧低下	*/
		}
		//VDET(温度補償回路電圧低下)の場合
		if(( RTC_CLOCK.flag_reg & 0x01) == 0x01 )
		{
			err_chk( ERRMDL_MAIN, ERR_MAIN_RTCTMPREC_VOLDWN, 2, 0, 0);		/* VDET(温度補償回路電圧低下) */
		}
		
		request.RequestCode 			= I2C_RC_RTC_EX_FLAG_CTL_SND;		// レジスタ（3種類）初期化
		request.I2cReqDataInfo.RWCnt	= RTC_EX_FR_CR_SIZE;				// Write/ReadするByte数(3)
		RTC_CLOCK.ex_reg				= RTC_EX_INIT;						// TEST		WADA	USEL	TE		FSEL1	FSEL0	TSEL1	TSEL0(0x20)
		RTC_CLOCK.flag_reg	 			= RTC_FR_INIT;						// ---		---		UF		TF		AF		EVF		VLF		VDET(0x00)
		RTC_CLOCK.ctrl_reg	 			= RTC_CR_INIT;						// CSEL1	CSEL0	UIE		TIE		AIE		EIE		---		RESET(0x60)
		request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.ex_reg;				// Write/ReadするBufferへのPointer
	}else{
		request.RequestCode    			=	I2C_RC_RTC_FLAG_SND;			// flag レジスタ初期化
		request.I2cReqDataInfo.RWCnt	=	RTC_FR_SIZE;					// Write/ReadするByte数(1)
		RTC_CLOCK.flag_reg				=	RTC_FR_INIT;					// flag register
		request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.flag_reg;			// Write/ReadするBufferへのPointer
	}

	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );					// 
	if( ret != RIIC_RSLT_OK ){
		// ここでエラー登録等はやりますかね？
	}
	
	return;
}
/*[]----------------------------------------------------------------------[]*/
/*| RTCのレジスタ初期化関数  		                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_RtcRegisterInit     	    	                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN		 : void 				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : tanaka                                                  |*/
/*| Date         : 2012-1-13                                               |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	op_RtcRegisterInit( void )
{
	I2C_REQUEST 			request;
	ushort					ret;

	request.TaskNoTo	 			= I2C_TASK_OPE;					// opeタスクへI2CREQUEST処理完了を通知
	request.DeviceNo 				= I2C_DN_RTC;					// RTCへの要求
	request.RequestCode    			= I2C_RC_RTC_STATUS_RCV;		// 
	request.I2cReqDataInfo.RWCnt	= RTC_FR_SIZE;					// 
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.flag_reg;			// 

	//FLAGレジスタエラー判定処理( VLF(発振回路電圧低下) orVDET(温度補償回路電圧低下))の場合
	if( (RTC_CLOCK.flag_reg & 0x03) != 0x00 )
	{
		//VLF(発振回路電圧低下)の場合
		if((RTC_CLOCK.flag_reg & 0x02) == 0x02)
		{
			err_chk( ERRMDL_MAIN, ERR_MAIN_RTCCPU_VOLDWN, 2, 0, 0);				/* VLF(発振回路電圧低下	*/
		}
		//VDET(温度補償回路電圧低下)の場合
		if(( RTC_CLOCK.flag_reg & 0x01) == 0x01 )
		{
			err_chk( ERRMDL_MAIN, ERR_MAIN_RTCTMPREC_VOLDWN, 2, 0, 0);			/* VDET(温度補償回路電圧低下) */
		}		
		request.RequestCode   			=	I2C_RC_RTC_EX_FLAG_CTL_SND;		//レジスタ（3種類）初期化
		request.I2cReqDataInfo.RWCnt	=	RTC_EX_FR_CR_SIZE;				// Write/ReadするByte数(1)
		RTC_CLOCK.ex_reg				=	RTC_EX_INIT;					// ex register
		RTC_CLOCK.flag_reg	 			=	RTC_FR_INIT;					// flag register
		RTC_CLOCK.ctrl_reg	 			=	RTC_CR_INIT;					// ctrl register
		request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.ex_reg;				// Write/ReadするBufferへのPointer
	}
	//エラーなしの場合
	else
	{
		request.RequestCode    			=	I2C_RC_RTC_FLAG_SND;			// flag レジスタ初期化
		request.I2cReqDataInfo.RWCnt	=	RTC_FR_SIZE;					// Write/ReadするByte数(1)
		RTC_CLOCK.flag_reg				=	RTC_FR_INIT;					// flag register
		request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.flag_reg;				// Write/ReadするBufferへのPointer
	}
	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );					// 

	if( ret != RIIC_RSLT_OK ){
		// ここでエラー登録等はやりますかね？
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*|            timset( struct clk_rec * )                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*|            set time                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Argument : struct clk_rec *                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Return : 0: OK / -1: NG                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Y.Nonaka                                                |*/
/*| Date        :  2000- 7-29                                              |*/
/*| Update      :  2001-11-27 Y.Takahashi A1H                              |*/
/*[]------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/

unsigned char	timset( struct clk_rec *irq1_clk )
{

	I2C_REQUEST 			request;
	ushort					ret;
	uchar					year;
	
	request.TaskNoTo	 			= I2C_TASK_OPE;					// opeタスクへI2CREQUEST処理完了を通知
	request.DeviceNo 				= I2C_DN_RTC;					// RTCへの要求
	request.RequestCode    			= I2C_RC_RTC_TIME_SND;			// 
	request.I2cReqDataInfo.RWCnt	= (RTC_RW_SIZE - 1);			// 

	RTC_CLOCK.freq100sec	= 0;
	RTC_CLOCK.sec    = binbcd( irq1_clk->seco );					// Second (BCD)
	RTC_CLOCK.min    = binbcd( irq1_clk->minu );					// Minute (BCD)
	RTC_CLOCK.hour   = binbcd( irq1_clk->hour );					// Hour (BCD)
	RTC_CLOCK.week   = 0;
	RTC_CLOCK.day    = binbcd( irq1_clk->date );					// Day (BCD)
	RTC_CLOCK.month  = binbcd( irq1_clk->mont );					// Month (BCD)
	year = (unsigned char)( irq1_clk->year - 2000 );				//00～99年までしか設定できないので-2000年
	RTC_CLOCK.year   = binbcd( year );								// Year (BCD)
	request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.sec;				// Write/ReadするBufferへのPointer

	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );			// 

	if( ret != RIIC_RSLT_OK ){
		// ここでエラー登録等はやりますかね？
	}
	
	// 各種レジスタの初期化・リセット
	request.RequestCode    			=	I2C_RC_RTC_FLAG_SND;	//フラグレジスタ書き込み要求
	RTC_CLOCK.flag_reg				=	RTC_FR_INIT;			//フラグレジスタ初期化
	RTC_CLOCK.ctrl_reg				=	(RTC_CR_INIT | RTC_CR_RESET);			//CTRLレジスタRESET
	request.I2cReqDataInfo.pRWData	=	&RTC_CLOCK.flag_reg;		
	request.I2cReqDataInfo.RWCnt	=	RTC_FR_CR_SIZE;			// Write/ReadするByte数(2)
	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );			// 

	if( ret != RIIC_RSLT_OK ){
		// ここでエラー登録等はやりますかね？
	}

	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*|            timerd( void )                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Argument : struct clk_rec *                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Return : 0: OK / -1: NG                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Y.Nonaka                                                |*/
/*| Date        :  2000- 7-29                                              |*/
/*| Update      :  2001-11-27 Y.Takahashi A2H -> D2H                       |*/
/*[]------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/
unsigned char	timerd( struct clk_rec *irq1_clk )
{
	return (uchar)(RTC_ClkDataReadReq( RTC_READ_NON_QUE ));	// 時計データ読み込み要求
}

/*[]----------------------------------------------------------------------[]*/
/*| オンラインからの要求受信時の実行可能状態チェック					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : CheckOpeCondit( void )								   |*/
/*| RETURN VALUE : 0=OK、1=NG											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : imai													   |*/
/*| Date		 : 2014-11-12											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
int CheckOpeCondit(void)
{
	if (OPECTL.Mnt_mod != 0) {
	// メンテナンス中
		return 1;
	}
	if ((OPECTL.Ope_mod != 0)
	&&	(OPECTL.Ope_mod != 1)
	&&	(OPECTL.Ope_mod != 2)
	&&	(OPECTL.Ope_mod != 3)
	&&	(OPECTL.Ope_mod != 100)
	&&	(OPECTL.Ope_mod != 110)) {
	// 待機or駐車位置番号入力or精算中or精算完了ではない
	// 休業orﾄﾞｱ閉時ｱﾗｰﾑ表示処理でない
		return 1;
	}
	if (auto_syu_prn == 2) {
	// 待機 && 自動集計実行中
		return 1;
	}
	if (DOWNLOADING()) {
	// 遠隔ダウンロード中
		return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| オンラインでの料金計算要求受信時の車室チェック						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : CheckReqCalcData( void )								   |*/
/*| RETURN VALUE : 0=OK、0!=NG											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : T.Nagai												   |*/
/*| Date		 : 2015-02-26											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
int CheckReqCalcData(void)
{
	int				i;
	int				j = 0;
	int				ret = 0;
	flp_com			*flp;
	RYO_INFO		*ryo_info;
	struct clk_rec	wk_CLK_REC;
	ushort			wk_CLK_REC_msec;
	ushort	sts_msk = 0xFFFF;
	extern	uchar	SetCarInfoSelect( short index );

	// 料金計算可能状態か？
	if (CheckOpeCondit()) {
		// 料金計算不可
		ret = 1;
	}


	// 出庫時間取得
	c_Now_CLK_REC_ms_Read(&wk_CLK_REC, &wk_CLK_REC_msec);

	ryo_info = &RyoCalSim.RyoInfo[0];
	for (i = 0; i < OLD_LOCK_MAX; i++) {
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行

		if (FlpSetChk((ushort)i)) {
			// 有効な車室情報のみセット
			if( !SetCarInfoSelect(i) ){
				continue;
			}
			if( j >= OLD_LOCK_MAX ){
				break;
			}
			//  区画情報セット
			ryo_info[j].op_lokno = (ulong)((LockInfo[i].area * 10000L) + LockInfo[i].posi);	// 区画情報
			ryo_info[j].pr_lokno = i+1;

			// 料金種別セット
			ryo_info[j].kind = LockInfo[i].ryo_syu;			// 料金種別

			flp = &FLAPDT.flp_data[i];
			// 車両あり and (フラップ上昇中 or フラップ上昇完了 or 駐車中)
			if (flp->nstat.bits.b00 == 1 && (flp->mode == FLAP_CTRL_MODE2 ||
				flp->mode == FLAP_CTRL_MODE3 || flp->mode == FLAP_CTRL_MODE4)) {
				ryo_info[j].indate.year	= flp->year;		// 入庫日時
				ryo_info[j].indate.mont	= flp->mont;
				ryo_info[j].indate.date	= flp->date;
				ryo_info[j].indate.hour	= flp->hour;
				ryo_info[j].indate.minu	= flp->minu;
				ryo_info[j].indate.week	= (uchar)youbiget((short)ryo_info[j].indate.year,	// 曜日
													(short)ryo_info[j].indate.mont,
													(short)ryo_info[j].indate.date);
			}
			else {
				// 入庫時刻を0クリアする
				memset(&ryo_info[j].indate, 0, sizeof(ryo_info[j].indate));
			}

			// 出庫時刻セット
			memcpy(&ryo_info[j].outdate, &wk_CLK_REC, sizeof(wk_CLK_REC));

			// 現在ステータス
			ryo_info[j].nstat = flp->nstat.word & sts_msk;
			if (ret == 1) {
				// 駐車料金不明(bit10)をセット
				ryo_info[j].nstat |= (1 << 10);
			}

			j++;
		}
	}


	// 料金計算不可状態
	if (ret == 1) {
		// 全車室情報テーブルを送信
		NTNET_Snd_Data245(RyoCalSim.GroupNo);
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金計算処理														   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : ryo_cal_sim( void )									   |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : T.Nagai												   |*/
/*| Date		 : 2015-02-26											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
static void	ryo_cal_sim(void)
{
	int			i;
	ulong		wk_Credit;		// クレジット利用額
	uchar		wk_key_sec;		// 区画入力値
	uchar		ryo_syu;		// 料金種別(A=0・・・)
	short		ans;
	ushort		wk_Pr_LokNo;	// 内部処理用駐車位置番号(1～324)
	ulong		wk_Op_LokNo;	// 接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)
	uchar		wk_InLagTime;	// ラグタイム延長処理実行
	RYO_INFO	*ryo_info;
	flp_com		*flp;

	// 領収証発行延長用に、ワークエリアにPayData、PPC_Data_Detail、クレジット利用額を退避させる
	memcpy(&SysMnt_Work[0], &PayData, sizeof(PayData));
	memcpy(&SysMnt_Work[sizeof(PayData)], &PPC_Data_Detail, sizeof(PPC_Data_Detail));
	wk_Credit = ryo_buf.credit.pay_ryo;
	// 現在精算中の場合にセット済みのデータを退避させる
	wk_Op_LokNo = OPECTL.Op_LokNo;							// 接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)
	wk_Pr_LokNo = OPECTL.Pr_LokNo;							// 内部処理用駐車位置番号(1～324)
	wk_InLagTime = OPECTL.InLagTimeMode;					// ラグタイム延長処理実行

	if( CPrmSS[S_TYP][81] ){								// 区画使用設定有り?
		wk_key_sec = 1;										// 区画を[Ａ]とする(区画使用する)
	}else{
		wk_key_sec = 0;										// 区画入力値ｸﾘｱ(区画使用しない)
	}

	// 有効車室分ループする
	i = 0;
	ryo_info = &RyoCalSim.RyoInfo[0];
	while (ryo_info[i].pr_lokno) {
		WACDOG;											// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行

		ans = carchk(wk_key_sec, ryo_info[i].op_lokno, 0);	// 駐車位置番号ﾁｪｯｸ
		if ((ans == -1)										// 精算不可能
		||	(ans == 10)) {									// 未設定
		// 車室№エラー
			i++;
			continue;
		}

		ryo_syu = ryo_info[i].kind - 1;
		if (prm_get(COM_PRM, S_SHA, (short)(1+ryo_syu*6), 2, 5) == 0) {
		// 車室№の車種未使用設定
			i++;
			continue;
		}
		flp = &FLAPDT.flp_data[ryo_info[i].pr_lokno-1];
		// 精算中or精算完了？
		if (OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3) {
			// 精算中の車室のみ駐車料金をセットする
			if (ryo_info[i].pr_lokno == wk_Pr_LokNo) {
				// フラップ状態チェック
				switch (flp->mode) {
				case FLAP_CTRL_MODE1:						// 待機状態
					ryo_info[i].nstat |= (1 << 10);			// 駐車料金不明(bit10)をセット
					// no break
				case FLAP_CTRL_MODE5:						// ロック装置開完了待ち
				case FLAP_CTRL_MODE6:						// 出庫待ち中
					ryo_info[i].fee = 0;					// 0円をセット
					break;
				default:
					// 精算中か？
					if (OPECTL.Ope_mod == 2) {
						ryo_info[i].fee = ryo_buf.tyu_ryo;	// 計算済みの駐車料金をセット
					}
					else {
						ryo_info[i].fee = 0;				// 0円をセット
						ryo_info[i].nstat |= (1 << 10);		// 駐車料金不明(bit10)をセット
					}
					break;
				}
			}
			else {
				ryo_info[i].fee = 0;						// 0円をセット
				ryo_info[i].nstat |= (1 << 10);				// 駐車料金不明(bit10)をセット
			}
		}
		// 出庫状態ではない or 下降済み(車両なし)ではない
		else if (!chkdate2(ryo_info[i].indate.year, (short)ryo_info[i].indate.mont, (short)ryo_info[i].indate.date)) {
			// 料金計算処理
			cm27();												// ﾌﾗｸﾞｸﾘｱ
			init_ryocalsim();									// ﾌﾗｸﾞｸﾘｱ
			set_tim_ryocalsim(ryo_info[i].pr_lokno, &ryo_info[i].indate, &ryo_info[i].outdate);	// 入出庫時刻セット
			ryo_cal(110, ryo_info[i].pr_lokno);					// 料金計算(料金計算シミュレータ用)
			ryo_info[i].fee = ryo_buf.dsp_ryo;					// 算出料金
		}
		else {
			ryo_info[i].fee = 0;						// 0円をセット
			ryo_info[i].nstat |= (1 << 10);				// 駐車料金不明(bit10)をセット
		}

		i++;
		if (i >= OLD_LOCK_MAX) {
			break;
		}
	}

	// 退避させたPayData、PPC_Data_Detail、クレジット利用額を戻す
	memcpy(&PayData, &SysMnt_Work[0], sizeof(PayData));
	memcpy(&PPC_Data_Detail, &SysMnt_Work[sizeof(PayData)], sizeof(PPC_Data_Detail));
	ryo_buf.credit.pay_ryo = wk_Credit;
	// 退避した値を戻す
	OPECTL.Op_LokNo = wk_Op_LokNo;
	OPECTL.Pr_LokNo = wk_Pr_LokNo;
	OPECTL.InLagTimeMode = wk_InLagTime;
}

/*[]----------------------------------------------------------------------[]*/
/*| 料金計算フラグクリア												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : init_ryocalsim( void )								   |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : imai													   |*/
/*| Date		 : 2014-10-10											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
static void	init_ryocalsim(void)
{
	Flap_Sub_Flg = 0;											// 詳細中止エリア使用フラグOFF

	if (f_NTNET_RCV_MC10_EXEC) {								// mc10()更新が必要な設定が通信で更新された
		mc10();
		f_NTNET_RCV_MC10_EXEC = 0;
	}

	ryo_buf.credit.pay_ryo = 0;

	OPECTL.InLagTimeMode = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 入出庫時刻ｾｯﾄ														   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME	 : set_tim_ryocalsim( num, *indate, *outdate )			   |*/
/*| PARAMETER	 : num	 	: 車室№									   |*/
/*| PARAMETER	 : *indate	: 入庫予定日時								   |*/
/*| PARAMETER	 : *outdate	: 出庫予定日時								   |*/
/*| RETURN VALUE : void													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| car_in, car_ot, car_in_f, car_ot_fに入出庫時刻をセットする             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author		 : imai													   |*/
/*| Date		 : 2014-10-10											   |*/
/*| Update		 : 														   |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
static void	set_tim_ryocalsim(ushort num, struct clk_rec *indate, struct clk_rec *outdate)
{
	memset(&car_in, 		0, sizeof(struct CAR_TIM));				// 入車時刻ｸﾘｱ
	memset(&car_ot, 		0, sizeof(struct CAR_TIM));				// 出車時刻ｸﾘｱ
	memset(&car_in_f,		0, sizeof(struct CAR_TIM));				// 入車時刻Fixｸﾘｱ
	memset(&car_ot_f,		0, sizeof(struct CAR_TIM));				// 出車時刻Fixｸﾘｱ
	memset(&recalc_carin,	0, sizeof(struct CAR_TIM));				// 再計算用入車時刻ｸﾘｱ

	car_in.year = indate->year;										// 入車 年
	car_in.mon  = indate->mont;										//      月
	car_in.day  = indate->date;										//      日
	car_in.hour = indate->hour;										//      時
	car_in.min  = indate->minu;										//      分
	car_in.week = (char)youbiget( car_in.year,						//      曜日
								(short)car_in.mon,
								(short)car_in.day );
	car_ot.year = outdate->year;									// 出庫 年
	car_ot.mon  = outdate->mont;									//      月
	car_ot.day  = outdate->date;									//      日
	car_ot.hour = outdate->hour;									//      時
	car_ot.min  = outdate->minu;									//      分
	car_ot.week = (char)youbiget( car_ot.year,						//      曜日
								(short)car_ot.mon,
								(short)car_ot.day );

	memcpy(&car_in_f,		&car_in, sizeof(struct CAR_TIM));		// 入車時刻Fix
	memcpy(&car_ot_f,		&car_ot, sizeof(struct CAR_TIM));		// 出車時刻Fix
	memcpy(&recalc_carin,	&car_in, sizeof(struct CAR_TIM));		// 再計算用入車時刻Fix
}
// MH322914(S) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
short	IsMagReaderRunning()
{
	short nRet = 0;
	
	if(( RD_mod == 12 )||( RD_mod == 13 )){							// リーダー動作中？
		nRet = 1;
	}
	
	return nRet;
}
// MH322914(E) K.Onodera 2016/12/07 サービス券使用時に動作不安定となることの対策[共通バグNo.1341](GG103200)
// MH322914(S) K.Onodera 2016/09/21 AI-V対応
//[]----------------------------------------------------------------------[]
///	@brief		OPE01開始時クリア処理
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/09/21<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void op_mod01_Init( void )
{
	OPECTL.multi_lk = 0L;							// ﾏﾙﾁ精算問合せ中車室№
	OPECTL.ChkPassSyu = 0;							// 定期券問合せ中券種ｸﾘｱ
	OPECTL.ChkPassPkno = 0L;						// 定期券問合せ中駐車場№ｸﾘｱ
	OPECTL.ChkPassID = 0L;							// 定期券問合せ中定期券IDｸﾘｱ
	OPECTL.CalStartTrigger = 0;

// MH810100(S) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
	OPECTL.chg_idle_disp_rec = 0;
// MH810100(E) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
// MH810100(S) K.Onodera 2019/12/27 車番チケットレス（ID対応）
	OPECTL.remote_wait_flg = 0;
	op_ClearSequenceID();
// MH810100(E) K.Onodera 2019/12/27 車番チケットレス（ID対応）
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
	OPECTL.f_rtm_remote_pay_flg = 0;
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
// MH810100(S) K.Onodera  2020/01/22 車番チケットレス(QR確定・取消データ対応)
	memset( &g_UseMediaData, 0, sizeof( t_UseMedia ) );
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// 	memset( DiscountBackUpArea, 0, sizeof(DiscountBackUpArea) );
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// MH810100(E) K.Onodera  2020/01/22 車番チケットレス(QR確定・取消データ対応)
// GG129000(S) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.51)（GM803000流用）
// GG129000(S) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
//	OPECTL.f_searchtype = 0xff;						// 0に意味があるので0xffで初期化
	if(OPECTL.f_req_paystop == 0){
		OPECTL.f_searchtype = 0xff;						// 0に意味があるので0xffで初期化
	}
// GG129000(E) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
// GG129000(E) ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）(改善連絡No.51)（GM803000流用）
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
	GyouCnt_All_r = 0;
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	GyouCnt_All_j = 0;
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
}
// MH322914(E) K.Onodera 2016/09/21 AI-V対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
//// MH322914(S) K.Onodera 2016/08/08 AI-V対応：振替精算
////[]----------------------------------------------------------------------[]
/////	@brief		振替先料金計算
////[]----------------------------------------------------------------------[]
/////	@param		none
/////	@return		0=OK/!0=NG
////[]----------------------------------------------------------------------[]
/////	@date		Create	: 2016/08/25<br>
/////				Update	: 
////[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
//static int Ope_PipFurikaeCalc( void )
//{
//	ushort			posi;
//	flp_com			*flp;
//	RYO_INFO		*ryo_info;
//	struct clk_rec	wk_CLK_REC;
//	ushort			wk_CLK_REC_msec;
//	ushort	sts_msk = 0xFFFF;
//	extern	uchar	SetCarInfoSelect( short index );
//
//	// ワークへ出庫時間取得
//	c_Now_CLK_REC_ms_Read( &wk_CLK_REC, &wk_CLK_REC_msec );
//
//	ryo_info = &RyoCalSim.RyoInfo[0];
//
//	// 該当車室あり？
//	if( LKopeGetLockNum( g_PipCtrl.stFurikaeInfo.DestArea, g_PipCtrl.stFurikaeInfo.DestNo, &posi ) ){
//
//		// 現在ステータス
//		flp = &FLAPDT.flp_data[posi-1];
//		ryo_info[0].nstat = flp->nstat.word & sts_msk;
//		g_PipCtrl.stFurikaeInfo.DestStatus = ryo_info[0].nstat;
//
//		//  区画情報セット
//		ryo_info[0].op_lokno = (ulong)( (g_PipCtrl.stFurikaeInfo.DestArea * 10000L) + g_PipCtrl.stFurikaeInfo.DestNo );
//		ryo_info[0].pr_lokno = posi;
//
//		// 料金種別セット
//		ryo_info[0].kind = LockInfo[posi-1].ryo_syu;
//		g_PipCtrl.stFurikaeInfo.DestFeeKind = ryo_info[0].kind;
//
//		// 入庫時刻セット
//		ryo_info[0].indate.year = flp->year;										// 入車 年
//		ryo_info[0].indate.mont = flp->mont;										//      月
//		ryo_info[0].indate.date = flp->date;										//      日
//		ryo_info[0].indate.hour = flp->hour;										//      時
//		ryo_info[0].indate.minu = flp->minu;										//      分
//		ryo_info[0].indate.week = (char)youbiget( ryo_info[0].indate.year,			//      曜日
//									(short)ryo_info[0].indate.mont,
//									(short)ryo_info[0].indate.date );
//
//		// 応答用データにもセット
//		g_PipCtrl.stFurikaeInfo.DestInTime.Year = ryo_info[0].indate.year;
//        g_PipCtrl.stFurikaeInfo.DestInTime.Mon	= ryo_info[0].indate.mont;
//		g_PipCtrl.stFurikaeInfo.DestInTime.Day	= ryo_info[0].indate.date;
//		g_PipCtrl.stFurikaeInfo.DestInTime.Hour	= ryo_info[0].indate.hour;
//		g_PipCtrl.stFurikaeInfo.DestInTime.Min	= ryo_info[0].indate.minu;
//		g_PipCtrl.stFurikaeInfo.DestInTime.Sec	= ryo_info[0].indate.seco;
//
//		// 出庫時刻セット
//		memcpy( &ryo_info[0].outdate, &wk_CLK_REC, sizeof(wk_CLK_REC) );
//		// 応答用データにもセット
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Year	= ryo_info[0].outdate.year;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Mon		= ryo_info[0].outdate.mont;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Day		= ryo_info[0].outdate.date;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Hour	= ryo_info[0].outdate.hour;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Min		= ryo_info[0].outdate.minu;
//		g_PipCtrl.stFurikaeInfo.DestPayTime.Sec		= ryo_info[0].outdate.seco;
//
//	}else{
//		return PIP_RES_RESULT_NG_NO_CAR;
//	}
//
//	set_tim_ryocalsim( ryo_info[0].pr_lokno, &ryo_info[0].indate, &ryo_info[0].outdate );	// 入出庫時刻セット
//	ryo_cal( 111, ryo_info[0].pr_lokno );				// 料金計算(料金計算シミュレータ用)
//	ryo_info[0].fee = ryo_buf.dsp_ryo;					// 算出料金
//	g_PipCtrl.stFurikaeInfo.DestFee = ryo_info[0].fee;
//	g_PipCtrl.stFurikaeInfo.DestFeeDiscount = ryo_buf.waribik;
//
//	return PIP_RES_RESULT_OK;
//}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_PiP_GetFurikaeGaku
 *[]----------------------------------------------------------------------[]
 *| summary	: 振替金額計算
 *| return	: void
 *| NOTE	: 振替精算券データポインタ
 *[]----------------------------------------------------------------------[]*/
void	ope_PiP_GetFurikaeGaku( PIP_FURIKAE_INFO *st )
{
	uchar	wari_tim;

	wari_tim = (uchar)prm_get(COM_PRM, S_CEN, 32, 1, 1);

	vl_frs.zangaku = ryo_buf.zankin;
	// 振替元と振替先の料金種別が同じ or 料金種別が異なってもサービス券/掛売り券振替する？
	if( st->SrcFeeKind == (char)(ryo_buf.syubet+1) ||	wari_tim == 1 ){
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//		st->DestFeeDiscount = vl_frs.price;
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
		st->Remain = ryo_buf.zankin - vl_frs.price;
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//		vl_frs.furikaegaku = vl_frs.price;
		vl_frs.wari_furikaegaku = ( vl_frs.price - (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku) );
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
	}else{
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//		st->DestFeeDiscount = vl_frs.in_price;
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
		st->Remain = ryo_buf.zankin - vl_frs.in_price;
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//		vl_frs.furikaegaku = vl_frs.in_price;
		vl_frs.wari_furikaegaku = ( vl_frs.in_price - (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku) );
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
	}
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
//	// furikaegakuは、振替可能な額のみ(釣りとする分は含まない)
//	if( vl_frs.furikaegaku > vl_frs.zangaku ){
//		vl_frs.furikaegaku = vl_frs.zangaku;
//	}
	// 振替割引が残額以上？※振替割引は実際に適用した額のため
	if( vl_frs.wari_furikaegaku > vl_frs.zangaku ){
		vl_frs.wari_furikaegaku = vl_frs.zangaku;
	}
	// 振替額 = 振替額(現金、割引、カード決済の合算)
	vl_frs.furikaegaku = (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku + vl_frs.wari_furikaegaku);
	if( vl_frs.furikaegaku > vl_frs.zangaku ){
		vl_frs.kabarai = (vl_frs.furikaegaku - vl_frs.zangaku);
	}
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
}
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：振替精算
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)
//[]----------------------------------------------------------------------[]
///	@brief		遠隔精算用料金計算
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		0=OK/!0=NG
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static int Ope_PipRemoteCalcTimePre( void )
{
	ushort			posi;
	flp_com			*flp;
	RYO_INFO		*ryo_info;
	struct clk_rec	wk_CLK_REC;
	ushort			wk_CLK_REC_msec;
	ushort	sts_msk = 0xFFFF;
	extern	uchar	SetCarInfoSelect( short index );

	// ワークへ出庫時間取得
	c_Now_CLK_REC_ms_Read( &wk_CLK_REC, &wk_CLK_REC_msec );
	// 料金計算時刻保持
	g_PipCtrl.stRemoteTime.OutTime.Year	= wk_CLK_REC.year;		// Year
	g_PipCtrl.stRemoteTime.OutTime.Mon	= wk_CLK_REC.mont;		// Month
	g_PipCtrl.stRemoteTime.OutTime.Day	= wk_CLK_REC.date;		// Day
	g_PipCtrl.stRemoteTime.OutTime.Hour	= wk_CLK_REC.hour;		// Hour
	g_PipCtrl.stRemoteTime.OutTime.Min	= wk_CLK_REC.minu;		// Minute
	g_PipCtrl.stRemoteTime.OutTime.Sec	= wk_CLK_REC.seco;		// Second

	ryo_info = &RyoCalSim.RyoInfo[0];

	// 該当車室あり？
	if( LKopeGetLockNum( g_PipCtrl.stRemoteTime.Area, g_PipCtrl.stRemoteTime.ulNo, &posi ) ){
		//  区画情報セット
		ryo_info[0].op_lokno = (ulong)( (g_PipCtrl.stRemoteTime.Area * 10000L) + g_PipCtrl.stRemoteTime.ulNo );
		ryo_info[0].pr_lokno = posi;

		// 料金種別セット
		ryo_info[0].kind = LockInfo[posi-1].ryo_syu;			// 料金種別
		g_PipCtrl.stRemoteTime.RyoSyu = ryo_info[0].kind;

		// 入庫日時
		ryo_info[0].indate.year	= g_PipCtrl.stRemoteTime.InTime.Year;
		ryo_info[0].indate.mont	= g_PipCtrl.stRemoteTime.InTime.Mon;
		ryo_info[0].indate.date	= g_PipCtrl.stRemoteTime.InTime.Day;
		ryo_info[0].indate.hour	= g_PipCtrl.stRemoteTime.InTime.Hour;
		ryo_info[0].indate.minu	= g_PipCtrl.stRemoteTime.InTime.Min;
		ryo_info[0].indate.week	= (uchar)youbiget((short)ryo_info[0].indate.year,	// 曜日
												  (short)ryo_info[0].indate.mont,
												  (short)ryo_info[0].indate.date);

		// 出庫時刻セット
		memcpy( &ryo_info[0].outdate, &wk_CLK_REC, sizeof(wk_CLK_REC) );
	}else{
// MH810100(S) 2020/06/11 #4222【連動評価指摘事項】遠隔精算要求データ「要求種別：1=料金計算」受信時に遠隔精算入庫時刻指定精算応答データの結果が「13=失敗（指定車室無し）」となってしまう
//		return PIP_RES_RESULT_NG_NO_CAR;

		syashu = g_PipCtrl.stRemoteTime.RyoSyu;				// 料金種別

		ryo_info[0].pr_lokno = 1;							// 1固定

		// 入庫日時
		ryo_info[0].indate.year	= g_PipCtrl.stRemoteTime.InTime.Year;
		ryo_info[0].indate.mont	= g_PipCtrl.stRemoteTime.InTime.Mon;
		ryo_info[0].indate.date	= g_PipCtrl.stRemoteTime.InTime.Day;
		ryo_info[0].indate.hour	= g_PipCtrl.stRemoteTime.InTime.Hour;
		ryo_info[0].indate.minu	= g_PipCtrl.stRemoteTime.InTime.Min;
		ryo_info[0].indate.week	= (uchar)youbiget((short)ryo_info[0].indate.year,	// 曜日
												  (short)ryo_info[0].indate.mont,
												  (short)ryo_info[0].indate.date);

		// 出庫時刻セット
		memcpy( &ryo_info[0].outdate, &wk_CLK_REC, sizeof(wk_CLK_REC) );
// MH810100(E) 2020/06/11 #4222【連動評価指摘事項】遠隔精算要求データ「要求種別：1=料金計算」受信時に遠隔精算入庫時刻指定精算応答データの結果が「13=失敗（指定車室無し）」となってしまう
	}

	// 現在ステータス
	flp = &FLAPDT.flp_data[0];
	ryo_info[0].nstat = flp->nstat.word & sts_msk;

	set_tim_ryocalsim( ryo_info[0].pr_lokno, &ryo_info[0].indate, &ryo_info[0].outdate );	// 入出庫時刻セット
	ryo_cal( 111, ryo_info[0].pr_lokno );				// 料金計算(料金計算シミュレータ用)
	ryo_info[0].fee = ryo_buf.dsp_ryo;					// 算出料金
	// 割引額が算出料金を超える場合、割引額は料金に合わせる
	if( ryo_buf.dsp_ryo < g_PipCtrl.stRemoteTime.Discount ){
		g_PipCtrl.stRemoteTime.Discount = ryo_buf.dsp_ryo;
	}
	g_PipCtrl.stRemoteTime.Price = ryo_info[0].fee;

	return PIP_RES_RESULT_OK;
}
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(入庫時刻指定)

// MH321800(S) Y.Tanizaki ICクレジット対応
/*[]----------------------------------------------------------------------[]*/
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
///*| 精算領域へのﾃﾞｰﾀの格納 (決済リーダで復決済)              		       |*/
/*| 精算領域へのﾃﾞｰﾀの格納 (決済リーダで直前取引データ受信)              		       |*/
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcElectronSet_DeemedData					               |*/
/*| PARAMETER    : *buf		: 取引ﾃﾞｰﾀ				                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Inaba                                                   |*/
/*| Date         : 2019-02-06                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	EcElectronSet_DeemedData( void *buf )
{
	EC_SETTLEMENT_RES	*p;
	p = (EC_SETTLEMENT_RES*)buf;
	
	//集計・領収書 用 (クレジット) 初期化
	memset( &EcRecvDeemedData, 0, sizeof(Receipt_data) );

	// 復決済データにIDを付与
	EcRecvDeemedData.ID = 22;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		EcRecvDeemedData.ID = 56;
	}

	// みなし決済結果データ受信フラグ
	EcRecvDeemedData.Electron_data.Ec.E_Status.BIT.deemed_receive = 1;
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//	// 決済時刻をセット
//	EcRecvDeemedData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);						// 決済日時(Normalize)
//	// 出庫時刻を決済リーダから送られてきた時刻をセット
//	memcpy(&EcRecvDeemedData.TOutTime, &p->settlement_time, sizeof(date_time_rec));
	// 決済時刻をセット
	if( datetimecheck(&p->settlement_time) == 0){
		// 不条理チェックがNGであれば決済時刻をセットしない
		EcRecvDeemedData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);						// 決済日時(Normalize)
	}
	// 決済結果をセット
	EcRecvDeemedData.EcResult = (uchar)p->Result;
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う

	// ｸﾚｼﾞｯﾄﾃﾞｰﾀｾｯﾄ
	if( p->brand_no == BRANDNO_CREDIT ){
		memcpy( &EcRecvDeemedData.credit.card_name[0], &p->Brand.Credit.Credit_Company[0], sizeof(EcRecvDeemedData.credit.card_name) );	// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社略号
		memcpy( &EcRecvDeemedData.credit.card_no[0], &p->Card_ID[0], sizeof(EcRecvDeemedData.credit.card_no) );							// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員番号（右詰め）
		EcRecvDeemedData.credit.slip_no				= (ulong)p->Brand.Credit.Slip_No;													// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
		EcRecvDeemedData.credit.pay_ryo				= (ulong)p->settlement_data;														// クレジット決済額
		memset( &EcRecvDeemedData.credit.ShopAccountNo[0], 0x20, sizeof(EcRecvDeemedData.credit.ShopAccountNo) );						// 加盟店取引番号, 決済リーダでは無し

		EcRecvDeemedData.credit.app_no = astoinl(&p->Brand.Credit.Approval_No[0], sizeof(p->Brand.Credit.Approval_No));					// 承認番号
		memcpy( &EcRecvDeemedData.credit.CCT_Num[0], &p->Brand.Credit.Id_No[0], sizeof(p->Brand.Credit.Id_No) );						// 端末識別番号
		memcpy( &EcRecvDeemedData.credit.kid_code[0], &p->Brand.Credit.KID_Code[0], sizeof(EcRecvDeemedData.credit.kid_code) );			// KID コード

		EcRecvDeemedData.Electron_data.Ec.e_pay_kind	= EC_CREDIT_USED;																// 決算種別
		memcpy(EcRecvDeemedData.Electron_data.Ec.Card_ID, p->Card_ID, sizeof(EcRecvDeemedData.Electron_data.Ec.Card_ID));				// ｶｰﾄﾞID
	}
// MH810105(S) MH364301 QRコード決済対応
	else if( p->brand_no == BRANDNO_QR ){
		EcRecvDeemedData.Electron_data.Ec.pay_ryo	= p->settlement_data;											// 決済金額
		EcRecvDeemedData.Electron_data.Ec.pay_after	= p->settlement_data_after;										// 残高

		EcRecvDeemedData.Electron_data.Ec.Brand.Qr.PayKind = p->Brand.Qr.PayKind;									// 支払種別
		EcRecvDeemedData.Electron_data.Ec.e_pay_kind	= EC_QR_USED;												// 決算種別

		memcpy(EcRecvDeemedData.Electron_data.Ec.QR_Kamei, p->QR_Kamei,
				sizeof(EcRecvDeemedData.Electron_data.Ec.QR_Kamei));												// 加盟店名
		memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Qr.PayTerminalNo, p->Brand.Qr.PayTerminalNo,
				sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Qr.PayTerminalNo));									// 支払端末ID
		memcpy(EcRecvDeemedData.Electron_data.Ec.inquiry_num, p->inquiry_num,
				sizeof(EcRecvDeemedData.Electron_data.Ec.inquiry_num));												// 取引番号
		memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Qr.MchTradeNo, p->Brand.Qr.MchTradeNo,
				sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Qr.MchTradeNo));										// Mch取引番号
	}
// MH810105(E) MH364301 QRコード決済対応
	// 電子ﾏﾈｰﾃﾞｰﾀｾｯﾄ
	else {
		EcRecvDeemedData.Electron_data.Ec.pay_ryo	= p->settlement_data;											// 決済金額
		EcRecvDeemedData.Electron_data.Ec.pay_befor	= p->settlement_data_before;									// 決済前残高
		EcRecvDeemedData.Electron_data.Ec.pay_after	= p->settlement_data_after;										// 決済後残高
		memcpy(EcRecvDeemedData.Electron_data.Ec.Card_ID, p->Card_ID, sizeof(EcRecvDeemedData.Electron_data.Ec.Card_ID));		// ｶｰﾄﾞID
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
		memcpy(EcRecvDeemedData.Electron_data.Ec.inquiry_num, p->inquiry_num, sizeof(EcRecvDeemedData.Electron_data.Ec.inquiry_num)); //問い合わせ番号
// MH810105(S) MH364301 不要変数削除
//		memcpy(EcRecvDeemedData.Electron_data.Ec.Termserial_No, p->Termserial_No, sizeof(EcRecvDeemedData.Electron_data.Ec.Termserial_No)); //端末識別番号
// MH810105(E) MH364301 不要変数削除
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正

		switch (p->brand_no) {
		case	BRANDNO_KOUTSUU:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_KOUTSUU_USED;						// 決算種別
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.SPRW_ID, p->Brand.Koutsuu.SPRW_ID,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.SPRW_ID));								// SPRWID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.Kamei, p->Brand.Koutsuu.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.Kamei));									// 加盟店名
			EcRecvDeemedData.Electron_data.Ec.Brand.Koutsuu.TradeKind = p->Brand.Koutsuu.TradeKind;					// 取引種別
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
			break;
		case	BRANDNO_EDY:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_EDY_USED;							// 決算種別
// MH810103 MHUT40XX(S) Edy・WAON対応
//			EcRecvDeemedData.Electron_data.Ec.Brand.Edy.DealNo			= p->Brand.Edy.DealNo;						// 取引通番
//			EcRecvDeemedData.Electron_data.Ec.Brand.Edy.CardDealNo		= p->Brand.Edy.CardDealNo;					// カード取引通番
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.DealNo, &p->Brand.Edy.DealNo, 10);					// Edy取引通番
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.CardDealNo, &p->Brand.Edy.CardDealNo, 5);			// カード取引通番
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.Kamei, p->Brand.Edy.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.Kamei));										// 加盟店名
// MH810103 MHUT40XX(E) Edy・WAON対応
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Edy.TerminalNo, &p->Brand.Edy.TerminalNo, 8);				// 端末番号
			break;
		case	BRANDNO_NANACO:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_NANACO_USED;						// 決算種別
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//			EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.DealNo		= p->Brand.Nanaco.DealNo;					// 上位取引通番
//			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.TerminalNo, &p->Brand.Nanaco.TerminalNo, 12);		// 上位端末ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.Kamei, p->Brand.Nanaco.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.Kamei));									// 加盟店名
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.DealNo, &p->Brand.Nanaco.DealNo, 6);				// 端末取引通番
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Nanaco.TerminalNo, &p->Brand.Nanaco.TerminalNo, 20);		// 上位端末ID
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
			break;
		case	BRANDNO_WAON:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_WAON_USED;							// 決算種別
			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point_status	= p->Brand.Waon.PointStatus;				// 利用可否
			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point			= p->Brand.Waon.GetPoint;					// 今回ポイント
// MH810103 MHUT40XX(S) Edy・WAON対応
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point_total		= p->Brand.Waon.TotalPoint;				// 累計ポイント
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.period_point	= p->Brand.Waon.PeriodPoint;				// ２年前までに獲得したポイント
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.period			= p->Brand.Waon.Period;						// ２年前までに獲得したポイントの有効期限
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.card_type		= p->Brand.Waon.CardType;					// カード分類
//			EcRecvDeemedData.Electron_data.Ec.Brand.Waon.deal_code		= p->Brand.Waon.DealCode;					// 取引種別コード
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point_total, p->Brand.Waon.TotalPoint,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.point_total));								// 累計ポイント
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.Kamei, p->Brand.Waon.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.Kamei));									// 加盟店名
// MH810103 MHUT40XX(E) Edy・WAON対応
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.SPRW_ID, &p->Brand.Waon.SPRW_ID, sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Waon.SPRW_ID));	// SPRWID
			break;
		case	BRANDNO_SAPICA:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_SAPICA_USED;						// 決算種別
			EcRecvDeemedData.Electron_data.Ec.Brand.Sapica.Details_ID	= p->Brand.Sapica.Details_ID;				// 明細ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Sapica.Terminal_ID, &p->Brand.Sapica.Terminal_ID, sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Sapica.Terminal_ID));	// 物販端末ID
			break;
		case	BRANDNO_ID:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_ID_USED;							// 決算種別
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//			PayData.Electron_data.Ec.Brand.Id.Details_ID
//			PayData.Electron_data.Ec.Brand.Id.Terminal_ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Id.Kamei, p->Brand.Id.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Id.Kamei));										// 加盟店名
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Id.Approval_No, &p->Brand.Id.Approval_No, 7);			// 承認番号
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Id.TerminalNo, &p->Brand.Id.TerminalNo, 13);				// 上位端末ID
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
			break;
		case	BRANDNO_QUIC_PAY:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_QUIC_PAY_USED;						// 決算種別
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//			PayData.Electron_data.Ec.Brand.Id.Details_ID
//			PayData.Electron_data.Ec.Brand.Id.Terminal_ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Quickpay.Kamei, p->Brand.QuicPay.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Quickpay.Kamei));								// 加盟店名
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Quickpay.Approval_No, &p->Brand.QuicPay.Approval_No, 7);	// 承認番号
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Quickpay.TerminalNo, &p->Brand.QuicPay.TerminalNo, 13);	// 上位端末ID
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
			break;
// MH810105(S) MH364301 PiTaPa対応
		case	BRANDNO_PITAPA:
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_PITAPA_USED;						// 決算種別
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.Kamei, p->Brand.Pitapa.Kamei,
					sizeof(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.Kamei));									// 加盟店名
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.Slip_No, &p->Brand.Pitapa.Slip_No, 5);			// 伝票番号
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.TerminalNo, &p->Brand.Pitapa.TerminalNo, 13);		// 上位端末ID
			memcpy(EcRecvDeemedData.Electron_data.Ec.Brand.Pitapa.Approval_No, &p->Brand.Pitapa.Approval_No, 8);	// 承認番号
			break;
// MH810105(E) MH364301 PiTaPa対応
		default:	// 不明ブランド
			EcRecvDeemedData.Electron_data.Ec.e_pay_kind					= EC_UNKNOWN_USED;
			break;
		}
	}
}

#if (6 == AUTO_PAYMENT_PROGRAM)
ushort	auto_payment_locksts_set( void )
{
	ulong	work_data_nml;
	short	car_index_bk = car_index;
	short	i;
	short	end = INT_CAR_START_INDEX + INT_CAR_LOCK_MAX;
	date_time_rec work_date;
	memset(&work_date, 0, sizeof(work_date));

	// 入庫時刻を算出
	work_date.Year = (ushort)CLK_REC.year;
	work_date.Mon = (uchar)CLK_REC.mont;
	work_date.Day = (uchar)CLK_REC.date;
	work_date.Hour = (uchar)CLK_REC.hour;
	work_date.Min = (uchar)CLK_REC.minu;
	work_data_nml = Nrm_YMDHM(&work_date);
	work_data_nml -= 0x10000;
	UnNrm_YMDHM(&work_date, work_data_nml);

	// 車室状態をセット
	FLAPDT.flp_data[car_index].mode = 3;					// 入庫
	FLAPDT.flp_data[car_index].year = work_date.Year;		// 年
	FLAPDT.flp_data[car_index].mont = work_date.Mon;		// 月
	FLAPDT.flp_data[car_index].date = work_date.Day;		// 日
	FLAPDT.flp_data[car_index].hour = work_date.Hour;		// 時
	FLAPDT.flp_data[car_index].minu = work_date.Min;		// 分
	FLAPDT.flp_data[car_index].nstat.bits.b00 = 1;			// 車両あり
	FLAPDT.flp_data[car_index].nstat.bits.b01 = 1;			// 上昇済み
	FLAPDT.flp_data[car_index].nstat.bits.b02 = 1;			// 上昇動作

	// 次の車室のインデックスを算出
	for(i = car_index+1; i < end; i++) {
		WACDOG;
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			car_index = i;
			break;
		}
	}
	if(car_index == car_index_bk ) {
		car_index = INT_CAR_START_INDEX;
	}
	return (ushort)LockInfo[car_index_bk].posi;
}
#endif
// MH321800(E) Y.Tanizaki ICクレジット対応

// MH810103 GG119202(S) 電子マネーシングル設定で案内放送を行う
//[]----------------------------------------------------------------------[]
///	@brief			決済リーダの案内放送処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/04/26
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void op_EcEmoney_Anm(void)
{
	if (!isEC_USE() ||
		OPECTL.op_faz >= 2) {
		return;
	}

	if (check_enable_multisettle() == 1) {
		// 電子マネーシングル設定の場合は案内放送を行う
		if (isEcEmoneyEnabled(1, 0) &&
			ECCTL.anm_ec_emoney != 0xFF) {
			ope_anm(AVM_SELECT_EMONEY);			// 「音が鳴るまでしっかりタッチしてください」
			ECCTL.anm_ec_emoney = 0xFF;			// 放送済み
// MH810103(s) 電子マネー対応 #5555【WAONシングル対応】精算開始で「音が鳴るまでしっかりタッチして下さい」を案内放送する
			lcdbm_notice_ope(LCDBM_OPCD_EMONEY_SINGLE,0);
// MH810103(e) 電子マネー対応 #5555【WAONシングル対応】精算開始で「音が鳴るまでしっかりタッチして下さい」を案内放送する
		}
		else if (!SUICA_USE_ERR && Suica_Rec.Data.BIT.CTRL == 0 &&
				 ECCTL.anm_ec_emoney != 0xFF) {
			// 受付不可状態の場合は受付可受信時に案内放送する
			ECCTL.anm_ec_emoney = 1;			// 受付可待ち
		}
	}
}
// MH810103 GG119202(E) 電子マネーシングル設定で案内放送を行う
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//// MH810100(S) テスト用入庫ログ作成
//#ifdef TEST_ENTER_LOG
//void Test_Ope_EnterLog(void)
//{
//	FLAPDT.flp_data[1].year = CLK_REC.year;
//	FLAPDT.flp_data[1].mont = CLK_REC.mont;
//	FLAPDT.flp_data[1].date = CLK_REC.date;
//	FLAPDT.flp_data[1].hour = CLK_REC.hour;
//	FLAPDT.flp_data[1].minu = CLK_REC.minu;
//	Make_Log_Enter(1);
//	Log_regist(LOG_ENTER);
//	taskchg(IDLETSKNO);
//}
//#endif
//// MH810100(E) テスト用入庫ログ作成
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)

// MH810100(S) K.Onodera 2019/11/18 車番チケットレス（メンテナンス終了処理）
//[]----------------------------------------------------------------------[]
///	@brief			メンテナンス開始応答待ち
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			MOD_EXT / MOD_CHG
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort op_wait_mnt_start( void )
{
	ushort	msg = 0;
	ushort	res_wait_time = prm_get(COM_PRM, S_PKT, 21, 2, 1);

	if( !res_wait_time ){
		res_wait_time = 5;	// 範囲外は5秒
	}

	// 操作通知(メンテナンス開始要求)送信(→LCD)
	lcdbm_notice_ope( LCDBM_OPCD_MNT_STA_REQ, 0 );
	// メンテナンス終了応答待ちタイマー開始
	Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );

	while(1){
		msg = GetMessage();
		switch( msg ){

			case KEY_MODECHG:			// Mode change key
				if( !OPECTL.on_off ){	// key OFF
					// メンテナンス開始応答待ちタイマー停止
					Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );
					return (MOD_EXT);	// OPE継続
				}
				break;

			case LCD_DISCONNECT:
				OPECTL.Ope_mod = 255;							// 初期化状態へ
				OPECTL.init_sts = 0;							// 初期化未完了状態とする
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
				OPECTL.PasswordLevel = (char)-1;
				return MOD_CUT;
				break;

			// メンテナンス開始応答待ちタイムアウト
			case TIMEOUT_MNT_RESTART:
				 // ドア開？
				if( !CP_MODECHG ){
					// 操作通知(メンテナンス開始要求)送信(→LCD)
					lcdbm_notice_ope( LCDBM_OPCD_MNT_STA_REQ, 0 );
					// メンテナンス開始応答待ちタイマー再開
					Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
				}
				break;

			// 操作通知
			case LCD_OPERATION_NOTICE:
				// メンテナンス開始応答
				if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_MNT_STA_RES ){
					 // ドア開？
					if( !CP_MODECHG ){
						// 0=OK？
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							OPECTL.Ope_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞｸﾘｱ
							OPECTL.Pay_mod = 0;								// 通常精算
							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
							OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
							OPECTL.PasswordLevel = (char)-1;
							return (MOD_CHG);	// メンテナンス終了
						}
						// 1=NG？
						else{
							// リトライオーバー等設けるとフェーズずれが発生するので無限リトライ
							// 操作通知(メンテナンス開始通知)再送信(→LCD)
							lcdbm_notice_ope( LCDBM_OPCD_MNT_STA_REQ, 0 );
							// メンテナンス開始応答待ちタイマー再開
							Lagtim( OPETCBNO, TIMERNO_MNT_RESTART, (res_wait_time * 50) );
						}
					}
				}
				// 精算開始要求
				else if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_PAY_STA ){
					// 操作通知(精算開始応答(OK))送信
					lcdbm_notice_ope( LCDBM_OPCD_PAY_STA_RES, 0 );
					// ﾒﾝﾃﾅﾝｽ開始再送ﾀｲﾏ停止
					Lagcan( OPETCBNO, TIMERNO_MNT_RESTART );
					// op_mod01(車番入力) = ｶｰﾄﾞ情報待ち
					OPECTL.Ope_mod = 1;
					return (MOD_EXT);	// OPE継続
				}
				break;

			// 入庫情報(ｶｰﾄﾞ情報)
			case LCD_IN_CAR_INFO:
				// TODO_Onodera：必要？？
				break;

			// 遠隔精算→ドア開ならやらないのでOK
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			メンテナンス終了処理
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			MOD_EXT / MOD_CHG
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort op_wait_mnt_close( void )
{
	ushort	msg = 0;
	ushort	res_wait_time = prm_get(COM_PRM, S_PKT, 21, 2, 1);

	if( !res_wait_time ){
		res_wait_time = 5;	// 範囲外は5秒
	}

	// 操作通知(メンテナンス終了通知)送信(→LCD)
	lcdbm_notice_ope( LCDBM_OPCD_MNT_END_REQ, lcdbm_alarm_check() );
	// メンテナンス終了応答待ちタイマー開始
	Lagtim( OPETCBNO, TIMERNO_MNT_RESTOP, (res_wait_time * 50) );

	while(1){
		msg = GetMessage();
		switch( msg ){

			case KEY_MODECHG:			// Mode change key
				if( OPECTL.on_off ){	// key ON
					// メンテナンス終了応答待ちタイマー停止
					Lagcan( OPETCBNO, TIMERNO_MNT_RESTOP );
					return (MOD_EXT);	// メンテナンス継続
				}
				break;

			case LCD_DISCONNECT:
				OPECTL.Ope_mod = 255;							// 初期化状態へ
				OPECTL.init_sts = 0;							// 初期化未完了状態とする
				OPECTL.Pay_mod = 0;								// 通常精算
				OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
				OPECTL.PasswordLevel = (char)-1;
				return MOD_CUT;
				break;

			// メンテナンス終了応答待ちタイムアウト
			case TIMEOUT_MNT_RESTOP:
				 // ドア閉？
				if( CP_MODECHG ){
					// 操作通知(メンテナンス終了通知)再送信(→LCD)
					lcdbm_notice_ope( LCDBM_OPCD_MNT_END_REQ, lcdbm_alarm_check() );
					// メンテナンス終了応答待ちタイマー再開
					Lagtim( OPETCBNO, TIMERNO_MNT_RESTOP, (res_wait_time * 50) );
				}
				break;

			// 操作通知
			case LCD_OPERATION_NOTICE:
				// メンテナンス終了応答
				if( LcdRecv.lcdbm_rsp_notice_ope.ope_code == LCDBM_OPCD_MNT_END_RES ){
					 // ドア閉？
					if( CP_MODECHG ){
						// 0=OK？
						if( LcdRecv.lcdbm_rsp_notice_ope.status == 0 ){
							OPECTL.Pay_mod = 0;								// 通常精算
							OPECTL.Mnt_mod = 0;								// ｵﾍﾟﾚｰｼｮﾝﾓｰﾄﾞ処理へ
							Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
							OPECTL.Mnt_lev = (char)-1;						// ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
							OPECTL.PasswordLevel = (char)-1;
							return (MOD_CHG);	// メンテナンス終了
						}
						// 1=NG？
						else{
							// リトライオーバー等設けるとフェーズずれが発生するので無限リトライ
							// 操作通知(メンテナンス終了通知)再送信(→LCD)
							lcdbm_notice_ope( LCDBM_OPCD_MNT_END_REQ, lcdbm_alarm_check() );
							// メンテナンス終了応答待ちタイマー再開
							Lagtim( OPETCBNO, TIMERNO_MNT_RESTOP, (res_wait_time * 50) );
						}
					}
				}
				break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ID(入庫から精算完了までを管理するID)セット
//[]----------------------------------------------------------------------[]
///	@param			id：入庫から精算完了までを管理するID
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void op_SetSequenceID( ulong id )
{
	g_OpeSequenceID = id;
}

//[]----------------------------------------------------------------------[]
///	@brief			ID(入庫から精算完了までを管理するID)ゲット
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			id：入庫から精算完了までを管理するID
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ulong op_GetSequenceID( void )
{
	return g_OpeSequenceID;
}

//[]----------------------------------------------------------------------[]
///	@brief			ID(入庫から精算完了までを管理するID)クリア
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void op_ClearSequenceID( void )
{
	g_OpeSequenceID = 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			定期種別がセットされているかをチェックする
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// GG129000(S) ゲート式車番チケットレスシステム対応（改善連絡No.79)
//static BOOL isSeasonCardType( stMasterInfo_t* pstMasterInfo )
static BOOL isSeasonCardType( stMasterInfo_t* pstMasterInfo, stZaishaInfo_t* pstZaishaInfo )
// GG129000(E) ゲート式車番チケットレスシステム対応（改善連絡No.79)
{
	ushort	cnt;
	BOOL	bRet = FALSE;

// GG129000(S) ゲート式車番チケットレスシステム対応（改善連絡No.79)
	if( pstZaishaInfo && pstZaishaInfo->NormalFlag != 0 ) {		// 定期利用以外
		return bRet;
	}
// GG129000(E) ゲート式車番チケットレスシステム対応（改善連絡No.79)

	// 定期種別あり？
	if( pstMasterInfo->SeasonKind ){

		for( cnt=0; cnt<ONL_MAX_CARDNUM; cnt++ ){
			// カード番号種別が定期券？
			if( pstMasterInfo->stCardDataInfo[cnt].CardType == CARD_TYPE_PASS ){
				bRet = TRUE;
				break;
			}
		}
	}

	return bRet;
}

//[]----------------------------------------------------------------------[]
///	@brief			カード情報として受信した定期情報のチェック(実処理)
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static eSEASON_CHK_RESULT CheckSeasonCardData_main( lcdbm_rsp_in_car_info_t* pstCardData, ushort no )
{
	char	c_prm;
	short	wk;
// MH810100(S) 2020/06/01 期限切れ定期券で精算時の料金が、入庫日時から精算日時で計算されてしまう
	short	wk2;
	uchar	KigenCheckResult2;
// MH810100(E) 2020/06/01 期限切れ定期券で精算時の料金が、入庫日時から精算日時で計算されてしまう
	short	w_syasyu;
	long	div_id;
	ushort	index = 0;
	ushort	pass_id;
	ushort	us_day;
// MH810100(S) m.saito 2020/05/22 車番チケットレス（#4179 定期券及びサービス券の種別毎の使用不可設定が適応されない）
	short	data_adr;		// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ
	char	data_pos;		// 使用可能料金種別のﾃﾞｰﾀ位置
// MH810100(E) m.saito 2020/05/22 車番チケットレス（#4179 定期券及びサービス券の種別毎の使用不可設定が適応されない）

	stDatetTimeYtoSec_t processTime;
	stMasterInfo_t* pMaster = &pstCardData->crd_info.dtMasterInfo;
	stZaishaInfo_t* pZaisha = &pstCardData->crd_info.dtZaishaInfo;
	stPassInfo_t* pPass 	= &pstCardData->crd_info.dtPassInfo;
	stParkKindNum_t* pCard 	= &pMaster->stCardDataInfo[no];

	short	wksy, wkey;
	uchar	uc_GtAlmPrm[ALM_LOG_ASC_DAT_SIZE];
	uchar	KigenCheckResult;

	memset( &CRD_DAT.PAS, 0, sizeof(pas_rcc) );
	memset( uc_GtAlmPrm, 0x00, ALM_LOG_ASC_DAT_SIZE );	// 0クリア
	memcpy( &uc_GtAlmPrm[0], &pCard->ParkingLotNo, 4 );	// 駐車場Noセット
	pass_id = astoin( pCard->byCardNo, 5 );
	memcpy( &uc_GtAlmPrm[4], &pass_id, 2 );				// 定期券IDセット

	// ------------------- //
	// 駐車場№チェック
	// ------------------- //
	if( pCard->ParkingLotNo ){
		for( index=0; index<4; index++ ){
			// 端末とカード情報の駐車場№一致？
			if( pCard->ParkingLotNo == prm_get( COM_PRM, S_SYS, (1 + index), 6, 1 ) ){
				break;
			}
		}
		if( index >= 4 ){
			alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			return SEASON_CHK_INVALID_PARKINGLOT;	// 駐車場№エラー
		}
	}

	// ------------------- //
	// カード番号チェック
	// ------------------- //
	if( pass_id < 1 ){
		return SEASON_CHK_INVALID_PASS_ID;		// 定期IDエラー
	}
	if( (ulong)CPrmSS[S_SYS][65] == pCard->ParkingLotNo ){
		if( CPrmSS[S_SYS][61] ){	// 定期テーブル分割①設定あり
			div_id = CPrmSS[S_SYS][61];
		}else{
			div_id = PAS_MAX + 1;
		}
		if( pass_id > (ushort)( div_id - 1 ) ){							// ﾃｰﾌﾞﾙ①の範囲外?
			return SEASON_CHK_INVALID_PASS_ID;	// 定期IDエラー
		}
	}
	else if( (ulong)CPrmSS[S_SYS][66] == pCard->ParkingLotNo ){
		if( CPrmSS[S_SYS][62] ){	// 定期テーブル分割②設定あり
			div_id = CPrmSS[S_SYS][62];
		}else{
			div_id = PAS_MAX + 1;
		}
		if( pass_id > (ushort)( div_id - CPrmSS[S_SYS][61] ) ){			// ﾃｰﾌﾞﾙ②の範囲外?
			return SEASON_CHK_INVALID_PASS_ID;	// 定期IDエラー
		}
	}
	else if( (ulong)CPrmSS[S_SYS][67] == pCard->ParkingLotNo ){
		if( CPrmSS[S_SYS][63] ){	// 定期テーブル分割③設定あり
			div_id = CPrmSS[S_SYS][63];
		}else{
			div_id = PAS_MAX + 1;
		}
		if( pass_id > (ushort)( div_id - CPrmSS[S_SYS][62] ) ){			// ﾃｰﾌﾞﾙ③の範囲外?
			return SEASON_CHK_INVALID_PASS_ID;	// 定期IDエラー
		}
	}
	else if( (ulong)CPrmSS[S_SYS][68] == pCard->ParkingLotNo ){
		if( pass_id > (ushort)( PAS_MAX + 1 - CPrmSS[S_SYS][63] ) ){	// ﾃｰﾌﾞﾙ④の範囲外?
			return SEASON_CHK_INVALID_PASS_ID;	// 定期IDエラー
		}
	}else{
		return SEASON_CHK_NO_MASTER_PARKINGLOT;	// 親機の駐車場№一致なしエラー
	}

	// ------------------- //
	// 有効無効チェック
	// ------------------- //
	if( pPass->CardStatus == 2 ){
		alm_chk2( ALMMDL_SUB2, ALARM_GT_MUKOU_PASS_USE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
		return SEASON_CHK_INVALID_SEASONDATA;		// 無効定期エラー
	}

	// ------------------- //
	// 初回定期チェック
	// ------------------- //
	// 初回定期読まない？
	if( prm_get( COM_PRM, S_TIK, 8, 1, 1 ) == 0 ){
		// 入出庫ステータス = 0:初期状態？
		if( pMaster->InOutStatus == 0 ){
			return SEASON_CHK_FIRST_NG;		// 初回定期エラー
		}
	}

	// ------------------- //
	// 販売ステータスチェック
	// ------------------- //
	if( pPass->SaleStatus == 9 ){
		return SEASON_CHK_PRE_SALE_NG;		// 販売前定期エラー
	}

	// ------------------- //
	// 定期種別範囲チェック
	// ------------------- //
	if( pMaster->SeasonKind < 1 || pMaster->SeasonKind > 15 ){
		alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
		return SEASON_CHK_INVALID_TYPE;		// 定期種別範囲外エラー
	}

// MH810100(S) m.saito 2020/05/22 車番チケットレス（#4179 定期券及びサービス券の種別毎の使用不可設定が適応されない）
	if( ryo_buf.syubet < 6 ){
		// 精算対象の料金種別A～F(0～5)
		data_adr = 10*(pMaster->SeasonKind-1)+9;				// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
		data_pos = (char)(6-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
	}
	else{
		// 精算対象の料金種別G～L(6～11)
		data_adr = 10*(pMaster->SeasonKind-1)+10;				// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
		data_pos = (char)(12-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
	}
	if( prm_get( COM_PRM, S_PAS, data_adr, 1, data_pos ) ){		// 使用不可設定？
		return SEASON_CHK_UNUSED_RYO_SYU;	// 料金種別未使用エラー
	}
// MH810100(E) m.saito 2020/05/22 車番チケットレス（#4179 定期券及びサービス券の種別毎の使用不可設定が適応されない）

	// ------------------- //
	// 定期種別使用可否チェック
	// ------------------- //
	w_syasyu = 0;
	c_prm = (char)prm_get( COM_PRM, S_PAS, 1 + (10 * (pMaster->SeasonKind - 1)), 2, 1 );	// 使用目的設定Get
	if( !rangechk( 1, 14, c_prm ) ){			// 定期使用目的設定範囲外
		if( c_prm == 0 ){
			return SEASON_CHK_UNUSED_TYPE;		// 定期種別未使用エラー
		}else{
			return SEASON_CHK_OUT_OF_RNG_TYPE;	// 定期使用目的設定範囲外エラー
		}
	}
	if( rangechk( 3, 14, c_prm ) ){													// 定期車種切換?
		w_syasyu = c_prm - 2;														// 車種切換用車種セット
		if ( prm_get( COM_PRM, S_SHA, (short)(1+6*(w_syasyu-1)), 2, 5 ) == 0L ) {	// 車種設定なし
			return SEASON_CHK_UNUSED_RYO_SYU;										// 料金種別未使用エラー
		}
		if(( ryo_buf.waribik )|| ( ryo_buf.zankin == 0 )){							// 割引済みor残金0円?
			return SEASON_CHK_ORDER_OF_USE;											// 精算順番エラー
		}
	}

	// ------------------- //
	// 有効期限チェック
	// ------------------- //
	CRD_DAT.PAS.knd = pMaster->SeasonKind;		// 種別ｾｯﾄ

	if( (short)prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,1 ) == 1 ){	// 期限切れ時受付する設定
		c_prm = (char)prm_get( COM_PRM,S_PAS,(short)(5+10*(CRD_DAT.PAS.knd-1)),1,2 );	// 無期限設定Get
	}else{
		c_prm = 0;
	}

	wksy = (short)pMaster->StartDate.shYear;
	if( c_prm == 1 || c_prm == 2 ){									// 無期限 or 開始日無期限
		CRD_DAT.PAS.std = dnrmlzm( 1980, 3, 1 );					// 1980年3月1日
	}else{
		CRD_DAT.PAS.std = 											// 有効開始日ｾｯﾄ
		dnrmlzm( wksy, (short)pMaster->StartDate.byMonth, (short)pMaster->StartDate.byDay );
	}

	CRD_DAT.PAS.std_end[0] = (char)(pMaster->StartDate.shYear % 100);
	CRD_DAT.PAS.std_end[1] = pMaster->StartDate.byMonth;;
	CRD_DAT.PAS.std_end[2] = pMaster->StartDate.byDay;

	wkey = (short)pMaster->EndDate.shYear;
	if( c_prm == 1 || c_prm == 3 ){									// 無期限 or 終了日無期限
		CRD_DAT.PAS.end = dnrmlzm( 2079, 12, 31 );					// 2079年12月31日
	}else{
		CRD_DAT.PAS.end = 											// 有効終了日ｾｯﾄ
		dnrmlzm( wkey, (short)pMaster->EndDate.byMonth, (short)pMaster->EndDate.byDay );
	}

	CRD_DAT.PAS.std_end[3] = (char)(pMaster->EndDate.shYear % 100);
	CRD_DAT.PAS.std_end[4] = pMaster->EndDate.byMonth;
	CRD_DAT.PAS.std_end[5] = pMaster->EndDate.byDay;

// MH810100(S) 2020/06/01 期限切れ定期券で精算時の料金が、入庫日時から精算日時で計算されてしまう
//	KigenCheckResult = Ope_PasKigenCheck( CRD_DAT.PAS.std, CRD_DAT.PAS.end, CRD_DAT.PAS.knd, CLK_REC.ndat, CLK_REC.nmin );
//										// 定期有効期限チェック（戻り：0=期限開始日より前，1=有効期限内，2=期限終了日より後）
//
//	if(( 1 != KigenCheckResult ) &&								// 期限切れ
//	   ( prm_get( COM_PRM,S_PAS, (short)(5+10*(CRD_DAT.PAS.knd-1)), 1, 1 ) == 0 )){	// 期限切れ時受付しない設定?
//
//		/*** 期限切れで期限切れ定期受付しない設定の時 ***/
//		if( 0 == KigenCheckResult ){							// 期限前
//			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
//			return SEASON_CHK_BEFORE_VALID;						// 期限前エラー
//		}
//		else{													// 期限切れ
//			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
//			return SEASON_CHK_AFTER_VALID;						// 期限切れエラー
//		}
//	}

	if( prm_get( COM_PRM,S_PAS, (short)(5+10*(CRD_DAT.PAS.knd-1)), 1, 1 ) == 0 ){	// 期限切れ時受付しない設定?
		
		//                   *-----------* 定期有効期限
		//                                    *-------- 入庫をチェック
		// 入庫時刻をチェック
		wk = dnrmlzm( car_in_f.year, (short)car_in_f.mon, (short)car_in_f.day );	// 入庫年月日ノーマライズ
		wk2 = tnrmlz( 0, 0, (short)car_in_f.hour, (short)car_in_f.min );			// 入庫時間ノーマライズ
	
		KigenCheckResult = Ope_PasKigenCheck( CRD_DAT.PAS.std, CRD_DAT.PAS.end, CRD_DAT.PAS.knd, wk, wk2 );
												// 定期有効期限チェック（戻り：0=期限開始日より前，1=有効期限内，2=期限終了日より後）
		
// MH810101 フェーズ2 (S) Takei 2021/01/20 　期限切れ時受付しない設定の場合に有効期限が途中半端な場合もNGとする。									　
//		if( KigenCheckResult == 2){
//			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
//			return SEASON_CHK_AFTER_VALID;						// 期限切れエラー
//		}										

		if( KigenCheckResult == 2){
			//入庫時刻が有効期限終了日より後
			//入庫日が定期の有効期間Bよりあと　　		A●-----------------●B　	入庫日●　　→時間
			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			return SEASON_CHK_AFTER_VALID;						// 期限切れエラー
		}
		else if( KigenCheckResult == 0){
			//入庫時刻が有効期限開始日より前　	入庫時発券機がなくて定期をNGで弾く端末がないため、精算時にチェックする必要がある
			//入庫日が有効開始Aより前　　		入庫日●	A●------●精算日-----------●B　●（精算日がここでもよい）	→時間
			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			return SEASON_CHK_BEFORE_VALID;					// 期限前エラー
		}
// MH810101 フェーズ2 (E) Takei 2021/01/20 　期限切れ時受付しない設定の場合に有効期限が途中半端な場合もNGとする。	

		//                                *-----------* 定期有効期限
		//       精算をチェック    ---*
		// 精算時刻をチェック
		wk = dnrmlzm( car_ot_f.year, (short)car_ot_f.mon, (short)car_ot_f.day );	// 精算年月日ノーマライズ
		wk2 = tnrmlz( 0, 0, (short)car_ot_f.hour, (short)car_ot_f.min );			// 精算時刻ノーマライズ
	
		KigenCheckResult2 = Ope_PasKigenCheck( CRD_DAT.PAS.std, CRD_DAT.PAS.end, CRD_DAT.PAS.knd, wk, wk2 );
											// 定期有効期限チェック（戻り：0=期限開始日より前，1=有効期限内，2=期限終了日より後）
											
// MH810101 フェーズ2 (S) Takei 2021/01/20 　期限切れ時受付しない設定の場合に有効期限が途中半端な場合もNGとする。
//		if( KigenCheckResult2 == 0){
//			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
//			return SEASON_CHK_BEFORE_VALID;						// 期限切れエラー
//		}
		
		if( KigenCheckResult2 == 0){
			//精算時刻が有効期限開始日より前
			//精算日が有効開始Aより前　　		●入庫日 ●精算日	A●-----------------●B　→時間
			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			return SEASON_CHK_BEFORE_VALID;						// 期限切れエラー
// MH810101 フェーズ2 (S) Endo 2021/01/22 　期限切れ時受付しない設定の場合に有効期限が途中半端な場合もNGとする。
//		}else if( KigenCheckResult == 2){
		}else if( KigenCheckResult2 == 2){
// MH810101 フェーズ2 (E) Endo 2021/01/22 　期限切れ時受付しない設定の場合に有効期限が途中半端な場合もNGとする。
			//精算時刻が有効期限終了日より後
			//精算日が有効開始Bより前　　		A●-------●入庫日----------●B　●精算日	→時間
			//これをOKにしたいのならば、07-00X5 = 1 定期X 無期限／期限切れ受付を1にすればOK。
			alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			return SEASON_CHK_AFTER_VALID;						// 期限切れエラー
		}
// MH810101 フェーズ2 (E) Takei 2021/01/20 　期限切れ時受付しない設定の場合に有効期限が途中半端な場合もNGとする。

	}

// MH810100(E) 2020/06/01 期限切れ定期券で精算時の料金が、入庫日時から精算日時で計算されてしまう

	// ------------------- //
	// アンチパスチェック
	// ------------------- //
	memset( &processTime, 0, sizeof(processTime) );
	switch( pMaster->InOutStatus ){
		// 出庫中
		case 1:
			memcpy( &processTime, &pZaisha->dtPaymentDateTime.dtTimeYtoSec, sizeof(processTime) );
			break;
		// 入庫中
		case 2:
			memcpy( &processTime, &pZaisha->dtEntryDateTime.dtTimeYtoSec, sizeof(processTime) );
			break;
		default:
			break;
	}

	if( prm_get( COM_PRM, S_PAS, (short)(2+10*(pMaster->SeasonKind - 1)), 1, 1 ) ){	// 入出チェックする設定？
		us_day = dnrmlzm( (short)processTime.shYear, (short)processTime.byMonth, (short)processTime.byDay );	// カード情報_処理年月日ノーマライズ
		wk = dnrmlzm( car_in_f.year, (short)car_in_f.mon, (short)car_in_f.day );								// 入庫年月日ノーマライズ
		// ｶｰﾄﾞ月日 > 入庫月日?？
		if( us_day > wk ){
			return SEASON_CHK_NTIPASS_ERROR;			// 入出エラー
		}
		if( us_day == wk ){
			wk = tnrmlz( 0, 0, (short)car_in_f.hour, (short)car_in_f.min );
			us_day = tnrmlz( 0, 0, (short)processTime.byHours, (short)processTime.byMinute );
			if( us_day > wk ){
				return SEASON_CHK_NTIPASS_ERROR;		// 入出エラー
			}
		}
	}

	if( w_syasyu ){
		vl_now = V_SYU;													// 種別切換
		syashu = (char)w_syasyu;										// 車種
	}else{																// 車種切換でない
		vl_now = V_TSC;													// 定期券(駐車券併用有り)
	}

	// 定期データを共通エリアにセーブ（vl_paschg()関数をコールするため）
	CRD_DAT.PAS.pno = (long)pCard->ParkingLotNo;						// 駐車場№
	CRD_DAT.PAS.GT_flg = 1;												// GTフォーマットフラグ
	CRD_DAT.PAS.cod = astoin( pCard->byCardNo, 5 );						// 定期ID
	CRD_DAT.PAS.sts = pMaster->InOutStatus;								// 入出庫ステータス
	CRD_DAT.PAS.trz[0] = processTime.byMonth;							// 処理月
	CRD_DAT.PAS.trz[1] = processTime.byDay;								// 処理日
	CRD_DAT.PAS.trz[2] = processTime.byHours;							// 処理時
	CRD_DAT.PAS.trz[3] = processTime.byMinute;							// 処理分
	CRD_DAT.PAS.typ = index;											// 定期区分

	InTeiki_PayData_Tmp.syu 		 = (uchar)CRD_DAT.PAS.knd;			// 定期券種別
	InTeiki_PayData_Tmp.status 		 = (uchar)CRD_DAT.PAS.sts;			// 定期券ｽﾃｰﾀｽ(読取時)
	InTeiki_PayData_Tmp.id 			 = CRD_DAT.PAS.cod;					// 定期券id
	InTeiki_PayData_Tmp.pkno_syu 	 = CRD_DAT.PAS.typ;					// 定期券駐車場Ｎｏ．種別 (0-3:基本,拡張1-3)
	InTeiki_PayData_Tmp.update_mon 	 = 0;								// 更新月数
	InTeiki_PayData_Tmp.s_year 		 = pMaster->StartDate.shYear;		// 有効期限（開始：年）
	InTeiki_PayData_Tmp.s_mon 		 = pMaster->StartDate.byMonth;		// 有効期限（開始：月）
	InTeiki_PayData_Tmp.s_day 		 = pMaster->StartDate.byDay;		// 有効期限（開始：日）
	InTeiki_PayData_Tmp.e_year 		 = pMaster->EndDate.shYear;			// 有効期限（終了：年）
	InTeiki_PayData_Tmp.e_mon 		 = pMaster->EndDate.byMonth;		// 有効期限（終了：月）
	InTeiki_PayData_Tmp.e_day 		 = pMaster->EndDate.byDay;			// 有効期限（終了：日）
	memcpy( InTeiki_PayData_Tmp.t_tim, CRD_DAT.PAS.trz, 4 );			// 処理月日時分
	InTeiki_PayData_Tmp.update_rslt1 = 0;								// 定期更新精算時の更新結果			（　OK：更新成功　／　NG：更新失敗　）
	InTeiki_PayData_Tmp.update_rslt2 = 0;								// 定期更新精算時のラベル発行結果	（　OK：正常発行　／　NG：発行不良　）

	OpeNtnetAddedInfo.PayMethod = 2;	// 精算方法=2:定期券精算

	return SEASON_CHK_OK;	// エラーなし
}

//[]----------------------------------------------------------------------[]
///	@brief			カード情報として受信した定期情報のチェック
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2020/01/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static eSEASON_CHK_RESULT CheckSeasonCardData( lcdbm_rsp_in_car_info_t* pstCardData )
{
	ushort	cnt;
	eSEASON_CHK_RESULT	result = SEASON_CHK_OK;

	for( cnt=0; cnt<ONL_MAX_CARDNUM; cnt++ ){

		if( pstCardData->crd_info.dtMasterInfo.stCardDataInfo[cnt].CardType != CARD_TYPE_PASS ){	// 媒体種別が定期でない
			continue;
		}
		// チェック実処理
		result = CheckSeasonCardData_main( pstCardData, cnt );
		break;
	}

	return result;
}

// MH810100(E) K.Onodera 2019/11/18 車番チケットレス（メンテナンス終了処理）

// MH810105(S) MH364301 QRコード決済対応
/*[]----------------------------------------------------------------------[]*/
/*| 精算領域へのﾃﾞｰﾀの格納 (決済リーダでのQRコード決済)			               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcQrSet_PayData					     	              |*/
/*| PARAMETER    : *buf		: 取引ﾃﾞｰﾀ				                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2021-07-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]*/
void	EcQrSet_PayData( void *buf )
{
	EC_SETTLEMENT_RES	*p;

	p = (EC_SETTLEMENT_RES*)buf;
	if(p->Column_No == 0xFF) {
		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;								// みなし決済
	}
	PayData.Electron_data.Ec.pay_ryo	= p->settlement_data;										// 決済金額
	PayData.Electron_data.Ec.pay_after	= p->settlement_data_after;									// 残高
	PayData.Electron_data.Ec.pay_datetime	= c_Normalize_sec(&p->settlement_time);					// 決済日時(Normalize)

	PayData.Electron_data.Ec.Brand.Qr.PayKind = p->Brand.Qr.PayKind;								// 支払種別
	PayData.Electron_data.Ec.e_pay_kind	= EC_QR_USED;												// 決算種別
	PayData.EcResult = p->Result;

	memcpy(PayData.Electron_data.Ec.QR_Kamei, &p->QR_Kamei,
			sizeof(PayData.Electron_data.Ec.QR_Kamei));												// 加盟店名
	memcpy(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo, &p->Brand.Qr.PayTerminalNo,
			sizeof(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo));								// 支払端末ID
	memcpy(PayData.Electron_data.Ec.inquiry_num, &p->inquiry_num,
			sizeof(PayData.Electron_data.Ec.inquiry_num));											// 取引番号
	memcpy(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo, &p->Brand.Qr.MchTradeNo,
			sizeof(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo));									// Mch取引番号

	// みなし決済？
	if (PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 1) {
		// 支払端末IDを'0'埋め（13桁）
		memset(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo, 0x20,
				sizeof(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo));
		memset(PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo, 0x30, 13);
		// 取引番号を'0'埋め（15桁）
		memset(PayData.Electron_data.Ec.inquiry_num, 0x30,
				sizeof(PayData.Electron_data.Ec.inquiry_num));
		// Mch取引番号をスペース埋め（32桁）
		memset(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo, 0x20,
				sizeof(PayData.Electron_data.Ec.Brand.Qr.MchTradeNo));
	}

// MH810105(S) MH364301 QRコード決済対応（精算中止データ送信対応）
	if (PayData.EcResult == EPAY_RESULT_NG) {
		// 決済結果＝決済NGの場合は精算中止データ送信対象とする
		PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
		// 精算中止ﾌﾗｸﾞをdeemSettleCancalにｾｯﾄ（精算データ送信時にこのﾌﾗｸﾞを使用する）
		// 決済状態＝「みなし決済」は deemSettleCancal=0, それ以外はdeemSettleCancal=1となる。
		PayData.Electron_data.Ec.E_Flag.BIT.deemSettleCancal = p->E_Flag.BIT.deemSettleCancal;
	}
// MH810105(E) MH364301 QRコード決済対応（精算中止データ送信対応）
	Ec_Data152Save();
}

//[]----------------------------------------------------------------------[]
///	@brief			障害連絡票処理中モノクロLCD表示切替
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			t.sato
///	@note			障害連絡票処理中モノクロLCD表示切替<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/01/28<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void lcd_Recv_FailureContactData_edit(void)
{
	lcd_wmsg_dsp_elec(1, ERR_CHR[78], ERR_CHR[85], 0, 0, COLOR_RED, LCD_BLINK_OFF);
}

//[]----------------------------------------------------------------------[]
///	@brief			ID152精算情報データの電子端末決済情報保存
//[]----------------------------------------------------------------------[]
///	@param[in]		wk_paydata 	: PayData
///	@return			void
///	@note			使用した電子端末カード情報をID152へ保存<br>
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/21<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void Ec_Data152Save(void)
{
	ulong	zangaku;
	ulong	app_no;
	ulong	slip_no;
	uchar	trans_sts;
	char	dummy_kid[6];
	char	dummy_CCTNum[13];
	uchar	dummy_Card_ID[20];
	uchar	dummy_inquiry_num[16];
	uchar	dummy_MchTradeNo[32];
	uchar	dummy_PayTermID[16];

	// 各ブランド毎にID152へデータを保存する
	switch (PayData.Electron_data.Ec.e_pay_kind) {
	// クレジット
	case EC_CREDIT_USED:

		if( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ){
			// 会員番号
			memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
			// 端末識別番号
			memset( &dummy_CCTNum[0], 0x20, sizeof(dummy_CCTNum) );
			// KIDコード
			memset( &dummy_kid[0], 0x20, sizeof(dummy_kid) );
			// 承認番号
			app_no = 0;
			// 伝票番号
			slip_no = 0;
		}
		else{
			// 会員番号
			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
				PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
				// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.を0x20h埋めする。
				// [*]印字みなし決済と同様の会員Noをセットする。
				memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
			}
			else {
				memcpy( &dummy_Card_ID[0], &ryo_buf.credit.card_no[0], sizeof(ryo_buf.credit.card_no) );
			}
			// 端末識別番号
			memcpy( &dummy_CCTNum[0], &ryo_buf.credit.CCT_Num[0], sizeof(dummy_CCTNum) );
			// KIDコード
			memcpy( &dummy_kid[0], &ryo_buf.credit.kid_code, sizeof(dummy_kid) );
			// 承認番号
			app_no = ryo_buf.credit.app_no;
			// 伝票番号
			slip_no = ryo_buf.credit.slip_no;
		}
		// 利用金額
		NTNET_Data152Save((void *)(&ryo_buf.credit.pay_ryo), NTNET_152_CPAYRYO);
		// 会員番号
		NTNET_Data152Save((void *)(&dummy_Card_ID[0]), NTNET_152_CCARDNO);
		// 端末識別番号
		NTNET_Data152Save((void *)(&dummy_CCTNum[0]), NTNET_152_CCCTNUM);
		// KIDコード
		NTNET_Data152Save((void *)(&dummy_kid[0]), NTNET_152_CKID);
		// 承認番号
		NTNET_Data152Save((void *)(&app_no), NTNET_152_CAPPNO);
		// 伝票番号
		NTNET_Data152Save((void *)(&slip_no), NTNET_152_SLIPNO);
		break;

	// QR
	case EC_QR_USED:

		if( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ){
			// Mch取引番号
			memset(&dummy_MchTradeNo[0], 0x20, sizeof(dummy_MchTradeNo));

			if (PayData.Electron_data.Ec.E_Flag.BIT.deemSettleCancal == 1) {
				// 決済結果を受信できなかったので
				// それぞれの桁数分0x20埋めし、支払端末IDは13桁、取引番号は15桁、0x30埋めする
				// 支払端末ID
				memset(&dummy_PayTermID, 0x20, sizeof(dummy_PayTermID));
				memset(&dummy_PayTermID, 0x30, 13);
				// 取引番号
				memset(&dummy_inquiry_num, 0x20, sizeof(dummy_inquiry_num));
				memset(&dummy_inquiry_num, 0x30, 15);
			}
			else {
				// 支払端末ID
				memcpy(&dummy_PayTermID[0], &PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo[0], 16);
				// 取引番号
				memset(&dummy_inquiry_num, 0x20, sizeof(dummy_inquiry_num));
				memcpy(&dummy_inquiry_num[0], &PayData.Electron_data.Ec.inquiry_num[0], 15);
			}
		}
		else{
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// Mch取引番号
				memset(&dummy_MchTradeNo[0], 0x20, sizeof(dummy_MchTradeNo));
				// 支払端末ID
				memcpy(&dummy_PayTermID[0], &PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo[0], 16);
				// 取引番号
				memset(&dummy_inquiry_num, 0x20, sizeof(dummy_inquiry_num));
				memcpy(&dummy_inquiry_num[0], &PayData.Electron_data.Ec.inquiry_num[0], 15);
			}
			else{
				// Mch取引番号
				memcpy(&dummy_MchTradeNo[0], &PayData.Electron_data.Ec.Brand.Qr.MchTradeNo[0], sizeof(dummy_MchTradeNo));
				// 支払端末ID
				memset(&dummy_PayTermID, 0x20, sizeof(dummy_PayTermID));
// MH810105(S) MH364301 QRコード決済対応 #6381 QRコード決済で通常精算時の精算データで決済情報の支払端末ID、取引番号が 0となってしまう
//				memset(&dummy_PayTermID, 0x30, 13);
// MH810105(E) MH364301 QRコード決済対応 #6381 QRコード決済で通常精算時の精算データで決済情報の支払端末ID、取引番号が 0となってしまう
				// 取引番号
				memset(&dummy_inquiry_num, 0x20, sizeof(dummy_inquiry_num));
// MH810105(S) MH364301 QRコード決済対応 #6381 QRコード決済で通常精算時の精算データで決済情報の支払端末ID、取引番号が 0となってしまう
//				memset(&dummy_inquiry_num, 0x30, 15);
// MH810105(E) MH364301 QRコード決済対応 #6381 QRコード決済で通常精算時の精算データで決済情報の支払端末ID、取引番号が 0となってしまう
			}
		}
		// カード決済区分
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.e_pay_kind), NTNET_152_ECARDKIND);
		// 利用金額
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.pay_ryo), NTNET_152_EPAYRYO);
		// Mch取引番号
		NTNET_Data152Save((void *)(&dummy_MchTradeNo[0]), NTNET_152_QR_MCH_TRADE_NO);
		// 支払端末ID
		NTNET_Data152Save((void *)(&dummy_PayTermID[0]), NTNET_152_QR_PAY_TERM_ID);
		// 取引番号
		NTNET_Data152Save((void *)(&dummy_inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
		// 決済ブランド
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.Brand.Qr.PayKind), NTNET_152_QR_PAYKIND);
		break;

// MH810105(S) MH364301 SX20統合対応
	case SUICA_USED:
		if( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ){
			// カード番号
			memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
			memset( &dummy_Card_ID[0], 0x30, ECARDID_SIZE_SUICA );
			memset( &dummy_Card_ID[0], 'Z', 2 );
			// カード残額
			zangaku = 0;
		}
		else{
			// カード番号
			memcpyFlushLeft(&dummy_Card_ID[0],
							&PayData.Electron_data.Suica.Card_ID[0],
							sizeof(dummy_Card_ID),
							sizeof(PayData.Electron_data.Suica.Card_ID));
			// カード残額
			zangaku = PayData.Electron_data.Suica.pay_after;
		}
		// 問い合わせ番号
		memset( &dummy_inquiry_num[0], 0x20, sizeof(dummy_inquiry_num) );
		// 承認番号
		app_no = 0;

		// カード決済区分
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.e_pay_kind), NTNET_152_ECARDKIND);
		// 利用金額
		NTNET_Data152Save((void *)(&PayData.Electron_data.Suica.pay_ryo), NTNET_152_EPAYRYO);
		// カード番号
		NTNET_Data152Save((void *)(&dummy_Card_ID[0]), NTNET_152_ECARDID);
		// 問合せ番号
		NTNET_Data152Save((void *)(&dummy_inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
		// カード残額
		NTNET_Data152Save((void *)(&zangaku), NTNET_152_EPAYAFTER);
		// 承認番号
		NTNET_Data152Save((void*)(&app_no), NTNET_152_CAPPNO);
		break;
// MH810105(E) MH364301 SX20統合対応

	// その他（電子マネー）
	default:

		if( PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1 ){
			// カード番号
			memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
			memset( &dummy_Card_ID[0], 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]) );
			memset( &dummy_Card_ID[0], 'Z', 2 );
			// カード残額
			zangaku = 0;
			// 問い合わせ番号
			memset( &dummy_inquiry_num[0], 0x20, sizeof(dummy_inquiry_num) );
			// 承認番号
			app_no = 0;
		}
		else{
			// カード番号
			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
				PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
				// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.をカード桁数分0x30埋めし左詰めでZZする。
				// [*]印字みなし決済と同様の会員Noをセットする。
				memset( &dummy_Card_ID[0], 0x20, sizeof(dummy_Card_ID) );
				memset( &dummy_Card_ID[0], 0x30, (size_t)(EcBrandEmoney_Digit[PayData.Electron_data.Ec.e_pay_kind - EC_USED]) );
				memset( &dummy_Card_ID[0], 'Z', 2 );
			}
			else{
				memcpy( &dummy_Card_ID[0], &PayData.Electron_data.Ec.Card_ID[0], sizeof(dummy_Card_ID) );
			}
			// カード残額
			zangaku = PayData.Electron_data.Ec.pay_after;
			// 問い合わせ番号
			memcpy( &dummy_inquiry_num[0], &PayData.Electron_data.Ec.inquiry_num[0], sizeof(PayData.Electron_data.Ec.inquiry_num) );
			// 承認番号
			switch(PayData.Electron_data.Ec.e_pay_kind){
				case EC_ID_USED:
					app_no = astoinl(PayData.Electron_data.Ec.Brand.Id.Approval_No, 7);
					break;
				case EC_QUIC_PAY_USED:
					app_no = astoinl(PayData.Electron_data.Ec.Brand.Quickpay.Approval_No, 7);
					break;
				case EC_PITAPA_USED:
					app_no = astoinl(PayData.Electron_data.Ec.Brand.Pitapa.Approval_No, 8);
					break;
				default:
					app_no = 0;
					break;
			}
		}
		// カード決済区分
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.e_pay_kind), NTNET_152_ECARDKIND);
		// 利用金額
		NTNET_Data152Save((void *)(&PayData.Electron_data.Ec.pay_ryo), NTNET_152_EPAYRYO);
		// カード番号
		NTNET_Data152Save((void *)(&dummy_Card_ID[0]), NTNET_152_ECARDID);
		// 問合せ番号
		NTNET_Data152Save((void *)(&dummy_inquiry_num[0]), NTNET_152_ECINQUIRYNUM);
		// カード残額
		NTNET_Data152Save((void *)(&zangaku), NTNET_152_EPAYAFTER);
		// 承認番号
		NTNET_Data152Save((void*)(&app_no), NTNET_152_CAPPNO);
		break;
	}

	// 取引ステータス
	if (PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
		trans_sts = 3;		// 未了支払不明
	}
	else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
		trans_sts = 2;		// 未了支払済み
	}
	else if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
		trans_sts = 1;		// みなし決済
	}
	else {
		trans_sts = 0;		// 支払い
	}
	NTNET_Data152Save((void*)(&trans_sts), NTNET_152_TRANS_STS);
}
// MH810105(E) MH364301 QRコード決済対応

// MH810105(S) MH364301 インボイス対応
//[]----------------------------------------------------------------------[]
///	@brief			領収証再発行（インボイス対応）
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/22
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void ryo_reisu(void)
{
	T_FrmReceipt	rec_data;

	memset(&rec_data, 0, sizeof(rec_data));
	rec_data.prn_kind = R_PRI;										// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
	rec_data.prn_data = &Cancel_pri_work;							// 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ
	if (OPECTL.f_ReIsuType == 1) {
		rec_data.kakari_no = 99;									// 係員No.99（固定）
	}
	else {
		rec_data.kakari_no = 0;										// 係員No.0（固定）
	}
	rec_data.reprint = ON;											// 再発行ﾌﾗｸﾞｾｯﾄ（再発行）
	memcpy( &rec_data.PriTime, &CLK_REC, sizeof(date_time_rec) );	// 再発行日時（現在日時）ｾｯﾄ
	queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
	wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );						// 操作履歴登録
	LedReq( CN_TRAYLED, LED_ON );									// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON
	LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );

	OPECTL.f_ReIsuType = 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			レシート印字失敗判定
//[]----------------------------------------------------------------------[]
///	@param[in]		prm	: 引数
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/05/06
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void op_rct_failchk(void)
{
	if (OPECTL.Ope_mod == 3) {
// GG129000(S) R.Endo 2023/02/22 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
// 		if (Ope_isPrinterReady() &&
// 			paperchk() != -1 &&
// 			Ope_isJPrinterReady() &&
// 			paperchk2() != -1) {
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 		if ( ( IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1)) &&
// 		                     (Ope_isJPrinterReady() && (paperchk2() != -1))) ||	
// 		     (!IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1))) ) {	// インボイスではない場合はレシートのみ判定
		// インボイスではない場合はレシートのみ判定、電子領収証の場合は常にレシート出力可
		if ( ( IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1))) &&
		                     (                Ope_isJPrinterReady() && (paperchk2() != -1) )) ||	
		     (!IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1)))) ) {
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// GG129000(E) R.Endo 2023/02/22 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
			// レシート・ジャーナル印字可能であれば、印字成功と判断する
		}
		else {
			// レシート・ジャーナルのどちらか印字不可であれば、印字失敗と判断する

			LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);
			OPECTL.f_DelayRyoIsu = 0;
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
			if( ryo_stock == 1 ){							// 印字データストック中
				MsgSndFrmPrn(PREQ_STOCK_CLEAR, R_PRI, 0);	// 印字データクリア
				ryo_stock = 2;								// ストックデータ破棄
			}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

			// 領収証ボタンLED消灯など
			op_ReciptLedOff();
			// 領収証発行失敗表示
			receipt_output_error();
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
			// レシート印字完了待ちタイマリセット
			Lagcan( OPETCBNO, TIMERNO_RPT_PRINT_WAIT );
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
		}
	}
	else {
		if (f_reci_ana) {
			// 延長後の領収証ボタン押下
// GG129000(S) R.Endo 2023/02/22 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
// 			if (Ope_isPrinterReady() &&
// 				paperchk() != -1 &&
// 				Ope_isJPrinterReady() &&
// 				paperchk2() != -1) {
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 			if ( ( IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1)) &&
// 			                     (Ope_isJPrinterReady() && (paperchk2() != -1))) ||	
// 			     (!IS_INVOICE && (Ope_isPrinterReady()  && (paperchk()  != -1))) ) {	// インボイスではない場合はレシートのみ判定
			// インボイスではない場合はレシートのみ判定、電子領収証の場合は常にレシート出力可
			if ( ( IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1))) &&
			                                     (Ope_isJPrinterReady() && (paperchk2() != -1)) ) ||
			     (!IS_INVOICE && (IS_ERECEIPT || (Ope_isPrinterReady()  && (paperchk()  != -1)))) ) {
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// GG129000(E) R.Endo 2023/02/22 車番チケットレス4.0 #6927 精算完了画面でジャーナル紙切れ、レシート紙切れを発生させても領収証ボタン押下可能のままになる
				// レシート・ジャーナル印字可能であれば、印字成功と判断する

				// 延長後の領収証（レシート）印字完了
				f_reci_ana = 0;
			}
			else {
				// レシート・ジャーナルのどちらか印字不可であれば、印字失敗と判断する

				// 領収証ボタンLED消灯など
				LagCan500ms(LAG500_RECEIPT_LEDOFF_DELAY);
				op_ReciptLedOff();
				// 領収証発行失敗表示
				receipt_output_error();
				f_reci_ana = 0;
			}
		}
		else {
			// 領収証再発行要求受信
			LagCan500ms(LAG500_JNL_PRI_WAIT_TIMER);
			OPECTL.f_ReIsuType = 0;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ジャーナル印字失敗判定
//[]----------------------------------------------------------------------[]
///	@param[in]		prm	: 引数
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/05/06
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void op_jnl_failchk(void)
{

	if (OPECTL.Ope_mod == 3) {
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 		if (Ope_isPrinterReady() &&
// 			paperchk() != -1 &&
		if ((IS_ERECEIPT || (Ope_isPrinterReady() && (paperchk() != -1))) &&
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
			Ope_isJPrinterReady() &&
			paperchk2() != -1) {
			// レシート・ジャーナル印字可能であれば、印字成功と判断する

			// 領収証（ジャーナル）印字完了
			OPECTL.f_DelayRyoIsu = 2;

			if (OPECTL.RECI_SW == 1) {
				// 領収証（レシート）を印字する
				OPECTL.RECI_SW = 0;
				queset(OPETCBNO, OPE_DELAY_RYO_PRINT, 0, NULL);
			}
		}
		else {
			// レシート・ジャーナルのどちらか印字不可であれば、印字失敗と判断する

			OPECTL.f_DelayRyoIsu = 0;
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
			if( ryo_stock == 1 ){							// 印字データストック中
				MsgSndFrmPrn(PREQ_STOCK_CLEAR, R_PRI, 0);	// 印字データクリア
				ryo_stock = 2;								// ストックデータ破棄
			}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）

			// 領収証ボタンLED消灯など
			op_ReciptLedOff();

			// 画面更新
			statusChange_DispUpdate();

			if (OPECTL.RECI_SW == 1) {
				// 領収証発行失敗表示
				receipt_output_error();
			}
		}
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/精算完了時の領収証発行中に待機状態に戻らないようにする）
		// ﾀﾞﾐｰにてｼﾞｬｰﾅﾙ印字完了（失敗）を通知
		queset(OPETCBNO, OPE_DUMMY_NOTICE, 0, 0);
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/精算完了時の領収証発行中に待機状態に戻らないようにする）
	}
	else {
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
// 		if (Ope_isPrinterReady() &&
// 			paperchk() != -1 &&
		if ((IS_ERECEIPT || (Ope_isPrinterReady() && (paperchk() != -1))) &&
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
			Ope_isJPrinterReady() &&
			paperchk2() != -1) {
			// レシート・ジャーナル印字可能であれば、印字成功と判断する

			if (OPECTL.f_ReIsuType) {
				// 領収証再発行要求受信後の領収証（ジャーナル）印字完了
				ryo_reisu();
			}
		}
		else {
			// レシート・ジャーナルのどちらか印字不可であれば、印字失敗と判断する
			OPECTL.f_ReIsuType = 0;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			領収証出力エラー時ポップアップ
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/26
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void receipt_output_error(void)
{

	short	time = 6*2;										// 領収証失敗表示時間
	uchar	wk = 0;

	// タイマー起動中？
	if( LagTim500ms_Is_Counting(LAG500_RECEIPT_MISS_DISP_TIMER) ){
		return;
	}

// MH810105(S) MH364301 インボイス対応 GT-4100
//	wk = 86;												// "　領収証の発行に失敗しました　"
//	if (prm_get(COM_PRM, S_RTP, 2, 1, 3) == 1) {			// 領収証表題印字(2)(17-0002④)
//		wk = 87;											// "　領収書の発行に失敗しました　"
//	}
// MH810105(E) MH364301 インボイス対応 GT-4100

	switch (OPECTL.f_RctErrDisp)
	{
		case 0:												// 領収証失敗ﾒｯｾｰｼﾞ未表示
			if( f_reci_ana ){								// 領収証延長中？
				Ope2_WarningDisp( 6*2, ERR_CHR[wk] );		// 領収証出力ワーニング表示 (6sec)
			}
			else{
				grachr( 7, 0, 30, 1, COLOR_RED,  LCD_BLINK_OFF, ERR_CHR[wk] );
				// 領収証発行失敗表示タイマー起動
				LagTim500ms( LAG500_RECEIPT_MISS_DISP_TIMER, (short)(time + 1), receipt_output_error );
// MH810105(S) MH364301 インボイス対応 GT-4100
				lcdbm_notice_dsp( POP_RECIPT_OUT_ERROR, 0 );	// 11:領収証発行失敗
// MH810105(E) MH364301 インボイス対応 GT-4100
			}
			OPECTL.f_RctErrDisp = 1;						// 領収証失敗ﾒｯｾｰｼﾞ表示中
			break;
		case 1:												// 領収証失敗ﾒｯｾｰｼﾞ表示中？
			Lcd_Receipt_disp();

			if (OPECTL.Ope_mod == 3 && isEC_CARD_INSERT()) {
				// カード抜き取り待ち継続中であれば、再表示する
				ec_MessageAnaOnOff( 1, 3 );
			}
			OPECTL.f_RctErrDisp = 0xff;						// 領収証失敗ﾒｯｾｰｼﾞ表示終了
			break;
		default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			状態変化による画面更新処理
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/05/16
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void statusChange_DispUpdate(void)
{
	ushort	wk_ec_MessagePtnNum = 0;

	switch (OPECTL.Ope_mod)
	{
	case 2:											// 精算中
		// 券エラー表示中？
		if( OPECTL.f_CrErrDisp != 0 ){
			// 券抜き取り後に７行目を再表示するのでここでは表示しない
			break;
		}

		// 料金表示(初回)でない
		if( ryodsp ){
			// (ｴﾗｰ以外の)EC関連ﾒｯｾｰｼﾞ表示中？
			if( isEC_MSG_DISP() && ECCTL.ec_MessagePtnNum != 98 ){
				// ﾒｯｾｰｼﾞ番号を仮ﾒｯｾｰｼﾞ番号へ保持
				wk_ec_MessagePtnNum = ECCTL.ec_MessagePtnNum;
				ec_MessageAnaOnOff( 0, 0 );
			}
			else{
				Lcd_Receipt_disp();
			}

			// 仮ﾒｯｾｰｼﾞ番号に値を保持していたら再度ﾒｯｾｰｼﾞを表示する
			if( wk_ec_MessagePtnNum != 0 ){
				// ｶｰﾄﾞ抜き取り表示？
				ec_MessageAnaOnOff( 1, (short)wk_ec_MessagePtnNum );
			}
		}
		break;
	case 3:											// 精算完了
		// 失敗表示中は無視
		if( OPECTL.f_RctErrDisp == 1 ){
			break;
		}
		// 表示完了後でec関連ﾒｯｾｰｼﾞ表示中は無視（ec関連ﾒｯｾｰｼﾞ優先）
		if( isEC_MSG_DISP() && OPECTL.f_RctErrDisp == 0xff ){
			break;
		}

		// (ｴﾗｰ以外の)EC関連ﾒｯｾｰｼﾞ表示中？
		if( isEC_MSG_DISP() ){
			// ﾒｯｾｰｼﾞ番号を仮ﾒｯｾｰｼﾞ番号へ保持
			wk_ec_MessagePtnNum = ECCTL.ec_MessagePtnNum;
			ec_MessageAnaOnOff( 0, 0 );
		}
		else{
			Lcd_Receipt_disp();
		}

		// 仮ﾒｯｾｰｼﾞ番号に値を保持していたら再表示する
		if( wk_ec_MessagePtnNum != 0 ){
			// 領収証ボタン未押下？
			if( OPECTL.RECI_SW == 0 ){
				ec_MessageAnaOnOff( 1, (short)wk_ec_MessagePtnNum );
			}
			else{
				// 領収証ボタン押下済の場合はreceipt_output_errorで画面表示する
				// ここでは何もしない
			}
		}
		break;
	default:
		break;
	}
}
// MH810105(E) MH364301 インボイス対応

// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)
//[]----------------------------------------------------------------------[]
///	@brief			精算開始時刻情報設定(入庫情報から設定)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void SetPayStartTimeInCarInfo()
{
	struct clk_rec PayDateTime;

	PayDateTime.year = lcdbm_rsp_in_car_info_main.PayDateTime.Year;
	PayDateTime.mont = lcdbm_rsp_in_car_info_main.PayDateTime.Mon;
	PayDateTime.date = lcdbm_rsp_in_car_info_main.PayDateTime.Day;
	PayDateTime.hour = lcdbm_rsp_in_car_info_main.PayDateTime.Hour;
	PayDateTime.minu = lcdbm_rsp_in_car_info_main.PayDateTime.Min;
	PayDateTime.seco = lcdbm_rsp_in_car_info_main.PayDateTime.Sec;
	PayDateTime.ndat = dnrmlzm(
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Year,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Mon,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Day);
	PayDateTime.nmin = tnrmlz((short)0, (short)0,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Hour,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Min);
	PayDateTime.week = (uchar)youbiget(
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Year,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Mon,
		(short)lcdbm_rsp_in_car_info_main.PayDateTime.Day);
	SetPayStartTime(&PayDateTime);
}
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(LCD-RXM精算開始日時統一)

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
//[]----------------------------------------------------------------------[]
///	@brief			車番チケットレス精算(入庫時刻指定)設定
//[]----------------------------------------------------------------------[]
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// ///	@param[in]		void
///	@param[in]		uchar	: 初回フラグ<br>
///							  0 = 二回目以降<br>
///							  1 = 初回<br>
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
///	@return			ret		: 料金設定結果<br>
///							  0 = 処理成功<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// static short in_time_set() {
static short in_time_set(uchar firstFlg) {
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
	short ret;

	// 料金設定
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// 	ret = cal_cloud_fee_set();
	ret = cal_cloud_fee_set(firstFlg);
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
	if ( ret != 0 ) {
		return ret;
	}

	// 定期有無チェック
// GG129000(S) ゲート式車番チケットレスシステム対応（改善連絡No.79)
//	if ( isSeasonCardType( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo ) ) {
	if ( isSeasonCardType( &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo, &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo ) ) {
// GG129000(E) ゲート式車番チケットレスシステム対応（改善連絡No.79)
		// 定期チェック
		season_chk_result = (uchar)cal_cloud_season_check();
		if( season_chk_result == SEASON_CHK_OK ){
			// 定期設定
			ret = cal_cloud_season_set();
			if ( ret != 0 ) {
				return ret;
			}
			if ( OPECTL.PassNearEnd == 1 ) {
				season_chk_result = 100;
			}
		}
	}

	// 入庫情報で受信した精算情報をセット
	Set_PayInfo_from_InCarInfo();

	// 割引設定
	ret = cal_cloud_discount_set();

	return ret;
}
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

// GG129000(S) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）
/**
 * @brief QR駐車券付き領収証の発行条件チェック
 * 
 * @param[in]		check_only	: 1=チェックのみ
 * @return 0:QR駐車券発行なし 1:QR駐車券強制発行 2:QR駐車券発行あり 3:QR駐車券強制発行(正常)
 * 
 * @date			2023/02/10
 */
uchar QRIssueChk(uchar check_only)
{
	uchar	ret = 0;
	uchar	qriss_flg1, qriss_flg2;
	long	prm;

	// 領収証は強制発行する設定?
	qriss_flg1 = 0;
// GG129000(S) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
//	if( prm_get(COM_PRM, S_LCD, 139, 1, 2) != 0 )
	if( prm_get(COM_PRM, S_RTP, 57, 1, 2) != 0 )
// GG129000(E) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
	{
		// 強制発行しない
		qriss_flg1 = 0;
	}
	else
	{
// GG129000(S) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
//		if( prm_get(COM_PRM, S_LCD, 139, 1, 1) != 3 )
		if( prm_get(COM_PRM, S_RTP, 57, 1, 1) != 3 )
// GG129000(E) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
		{
			// QRコード領収証発行しない設定以外の時
			// 強制発行する
			qriss_flg1 = 1;
		}
	}

	// 今回の精算はQR駐車券発行条件に当て嵌まったか確認する。
	qriss_flg2 = 0;
	// 領収証発行条件
// GG129000(S) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
//	if( prm_get(COM_PRM, S_LCD, 139, 1, 1) == 0 )
	if( prm_get(COM_PRM, S_RTP, 57, 1, 1) == 0 )
// GG129000(E) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
	{
		qriss_flg2 = 2;
	}

// GG129000(S) A.Shirai 2023/9/22 ゲート式車番チケットレスシステム対応（詳細設計#5696：#7043入庫時刻検索で精算完了しても領収証が自動発行されない）
// GG129000(S) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
//	if (prm_get(COM_PRM, S_LCD, 139, 1, 1) == 1)
	if (prm_get(COM_PRM, S_RTP, 57, 1, 1) == 1)
// GG129000(E) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
	{
		if (OPECTL.f_searchtype == SEARCH_TYPE_TIME) {
			qriss_flg2 = 1;
		}
	}
// GG129000(E) A.Shirai 2023/9/22 ゲート式車番チケットレスシステム対応（詳細設計#5696：#7043入庫時刻検索で精算完了しても領収証が自動発行されない）
	// 車番認識失敗した車両で精算完了した場合はQR発行を行う。
// GG129000(S) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
//	if( prm_get(COM_PRM, S_LCD, 139, 1, 1) == 2 )
	if( prm_get(COM_PRM, S_RTP, 57, 1, 1) == 2 )
// GG129000(E) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
	{
		// 入庫時車番情報取得に成功していた場合、QRコードを印字しない
		qriss_flg2 = CarNumChk();
	}

	// QR駐車券使用時はQR印字対象外
	if( PayData.CarSearchFlg == 2 &&
		lcdbm_rsp_in_car_info_main.shubetsu == 0)
	{
		qriss_flg2 = 0;
	}

	// 設定範囲外があった場合はQR印字対象外
	// 領収証の自動発行
// GG129000(S) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
//	prm = prm_get(COM_PRM, S_LCD, 139, 1, 2);
	prm = prm_get(COM_PRM, S_RTP, 57, 1, 2);
// GG129000(E) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
	if ( 1 < prm )
	{
		// 範囲外
		qriss_flg2 = 0;
		ret = 0;
	}

	// 領収証の発行条件
// GG129000(S) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
//	prm = prm_get(COM_PRM, S_LCD, 139, 1, 1);
	prm = prm_get(COM_PRM, S_RTP, 57, 1, 1);
// GG129000(E) M.Fujikawa 2023/09/28 ゲート式車番チケットレスシステム対応　QR駐車券のパラメータ設定
// GG129000(S) A.Shirai 2023/9/22 ゲート式車番チケットレスシステム対応（詳細設計#5696：#7043入庫時刻検索で精算完了しても領収証が自動発行されない）
//	if ( (3 < prm) || (prm == 1) )
	if (3 < prm)
// GG129000(E) A.Shirai 2023/9/22 ゲート式車番チケットレスシステム対応（詳細設計#5696：#7043入庫時刻検索で精算完了しても領収証が自動発行されない）
	{
		// 範囲外
		qriss_flg2 = 0;
		ret = 0;
	}

	// 強制発行設定あり&QR発行条件合致
	if( (qriss_flg1 == 1) && (qriss_flg2 == 1) )
	{
		ret = 1;
	}
	// 強制発行設定なし&QR発行条件合致
	else if( (qriss_flg1 == 0) && (qriss_flg2 == 1) )
	{
		ret = 2;
	}
	// 強制発行設定あり&QR発行条件合致
	else if( (qriss_flg1 == 1) && (qriss_flg2 == 2) )
	{
		ret = 3;
	}

	if (check_only) {
		return ret;
	}

	switch (ret) {
	case 1:
		// 領収証ボタンが押されていなければ強制発行処理を行う
// GG129000(S) M.Fujikawa 2023/10/26 ゲート式車番チケットレスシステム対応　不具合#7199対応
// GG129000(S) M.Fujikawa 2023/10/20 ゲート式車番チケットレスシステム対応　不具合#7170
////		if( OPECTL.RECI_SW == 0 ){											// 領収書ﾎﾞﾀﾝ未使用?
//		if( OPECTL.RECI_SW == 0 && 
//		  ((!IS_INVOICE && Ope_isPrinterReady() == 1) || 
//		  (IS_INVOICE && (Ope_isJPrinterReady() == 1 && paperchk2() == 0 && Ope_isPrinterReady() == 1)))){
// GG129000(E) M.Fujikawa 2023/10/20 ゲート式車番チケットレスシステム対応　不具合#7170
		if( OPECTL.RECI_SW == 0 ){											// 領収書ﾎﾞﾀﾝ未使用?
// GG129000(E) M.Fujikawa 2023/10/26 ゲート式車番チケットレスシステム対応　不具合#7199対応
			OPECTL.RECI_SW = 1;												// 領収書ﾎﾞﾀﾝ使用
// GG129000(S) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5684：領収証必ず発行設定で領収証発行した場合もNT-NET精算データの領収証発行有無が0＝領収証なしになっている）
			ryo_buf.ryos_fg = 1;
// GG129000(E) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5684：領収証必ず発行設定で領収証発行した場合もNT-NET精算データの領収証発行有無が0＝領収証なしになっている）
			PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );		// 操作通知送信
		}
		// 領収証受け取りポップアップは必ず表示する
		lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQR, 0 );						// 領収証受け取り表示(発行ボタン無し)
		break;
	case 2:
		// ボタンが押されていない場合は、ボタン付きポップアップを表示させる。
		if( OPECTL.RECI_SW == 0 ){											// 領収書ﾎﾞﾀﾝ未使用?
			lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQRBUTTON, 0 );				// 領収証受け取り表示(発行ボタン有り)
		}
		else
		{
			// 既にボタンが押されている場合はボタン無しのポップアップを表示する。
			lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQR, 0 );					// 領収証受け取り表示(発行ボタン無し)
		}
		break;
	case 3:
		// 領収証ボタンが押されていなければ強制発行処理を行う
// GG129000(S) M.Fujikawa 2023/10/26 ゲート式車番チケットレスシステム対応　不具合#7199対応
// GG129000(S) M.Fujikawa 2023/10/20 ゲート式車番チケットレスシステム対応　不具合#7170
////		if( OPECTL.RECI_SW == 0 ){											// 領収書ﾎﾞﾀﾝ未使用?
//		if( OPECTL.RECI_SW == 0 && 
//		  ((!IS_INVOICE && Ope_isPrinterReady() == 1) || 
//		  (IS_INVOICE && (Ope_isJPrinterReady() == 1 && paperchk2() == 0 && Ope_isPrinterReady() == 1)))){
// GG129000(E) M.Fujikawa 2023/10/20 ゲート式車番チケットレスシステム対応　不具合#7170
		if( OPECTL.RECI_SW == 0 ){											// 領収書ﾎﾞﾀﾝ未使用?
// GG129000(E) M.Fujikawa 2023/10/26 ゲート式車番チケットレスシステム対応　不具合#7199対応
			OPECTL.RECI_SW = 1;												// 領収書ﾎﾞﾀﾝ使用
// GG129000(S) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5684：領収証必ず発行設定で領収証発行した場合もNT-NET精算データの領収証発行有無が0＝領収証なしになっている）
			ryo_buf.ryos_fg = 1;
// GG129000(E) A.Shirai 2023/9/26 ゲート式車番チケットレスシステム対応（詳細設計#5684：領収証必ず発行設定で領収証発行した場合もNT-NET精算データの領収証発行有無が0＝領収証なしになっている）
			PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );		// 操作通知送信
		}
		// 領収証受け取りポップアップは必ず表示する
		// 車番情報取得有無
		if ( CarNumChk() ){
			// 取得NG
			lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQR, 0 );					// 領収証受け取り表示(発行ボタン無し)
		}else{
			// 取得OK
			lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQRNORMAL, 0 );				// 領収証受け取り表示(正常)
		}
		break;
	case 0:
		// QR発行条件を満たしていないため、表示なしを通知する(案内放送パターン分けに使用)
		lcdbm_notice_ope( LCDBM_OPCD_POP_PAYQRNONE, 0 );					// 領収証受け取り表示しない
		break;
	}

	return ret;
}

uchar	CarNumChk(void)
{
	uchar	ret = 0;	// 0:取得OK,1:取得NG

	// 入庫時車番情報取得に成功していた場合、QRコードを印字しない
	if( PayData.MediaKind2 == CARD_TYPE_CAR_NUM )
	{
		// 車番情報ありの場合、各項目の値が0になっていないか確認する
		// すべて値があれば情報取得成功と判断し、印字しない
		if( ((PayData.MediaCardNo2[0] != 0x30) || (PayData.MediaCardNo2[1] != 0x30)) &&	// 陸運支局名
			((PayData.MediaCardNo2[2] != 0x30) || (PayData.MediaCardNo2[3] != 0x30) ||	// 分類番号
			 (PayData.MediaCardNo2[4] != 0x30)) &&
			((PayData.MediaCardNo2[5] != 0x30) || (PayData.MediaCardNo2[6] != 0x30)) && // 用途文字
			((PayData.MediaCardNo2[7] != 0x30) || (PayData.MediaCardNo2[8] != 0x30) ||	// 一連番号
			 (PayData.MediaCardNo2[9] != 0x30) || (PayData.MediaCardNo2[10] != 0x30)) )
		{
			ret = 0;
		}
		else
		{
			ret = 1;
		}
	}
	else
	{
		ret = 1;
	}
	
	return ret;
}
// GG129000(E) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）

// GG129000(S) H.Fujinaga 2023/02/24 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//[]----------------------------------------------------------------------[]
///	@brief			遠隔精算(精算中変更)
//[]----------------------------------------------------------------------[]
///	@param[in]		void		: 
///	@return			ret			: 0=対象外(遠隔精算ではない)
///								: 1=初回(遠隔精算開始)
///								: 2=2回目以降(QR割引)
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2023/02/24
///					Update	:
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
uchar	PayInfoChange_StateCheck(void)
{
	uchar ret = 0;
	uchar shubetsu = lcdbm_rsp_in_car_info_main.shubetsu;
	uchar state = lcdbm_rsp_in_car_info_main.data.PayInfoChange.state;

	if(shubetsu == 2 && state == 1){
		// 遠隔精算かつ初回
		ret = 1;
	}else if(shubetsu == 2 && state == 2){
		// 遠隔精算かつ2回目以降(QR割引)
		ret = 2;
	}

	return ret;
}
// GG129000(E) H.Fujinaga 2023/02/24 ゲート式車番チケットレスシステム対応（遠隔精算対応）