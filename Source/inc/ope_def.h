#ifndef	_OPE_DEF_H_
#define	_OPE_DEF_H_
/*[]----------------------------------------------------------------------[]*/
/*| ｵﾍﾟﾚｰｼｮﾝﾀｽｸ共通定義                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-02-01                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	"common.h"
#include	"pri_def.h"
#include	"rkn_cal.h"

/*** structure ***/

/*------------------*/
/* ｵﾍﾟﾚｰｼｮﾝ関連定義 */
/*------------------*/

typedef union{
	uchar	BYTE;											// Byte
	struct{
		uchar	MC10_EXEC:1;								// Bit 7 = 1:mc10()実施要因発生
		uchar	YOBI2:1;									// Bit 6 = 予備
		uchar	MNTTIMCHG:1;								// Bit 5 = ﾒﾝﾃﾅﾝｽ現在時刻変更
		uchar	TEIKIEXTIM:1;								// Bit 4 = 定期券出庫時刻ﾃﾞｰﾀ
		uchar	TEIKITHUSI:1;								// Bit 3 = 定期券精算中止設定ﾃﾞｰﾀ
		uchar	KENKIGEN:1;									// Bit 2 = 券期限設定ﾃﾞｰﾀ
		uchar	SPLDATE:1;									// Bit 1 = 特別日設定ﾃﾞｰﾀ
		uchar	USERSET:1;									// Bit 0 = ﾕｰｻｰ設定ﾃﾞｰﾀ
	} BIT;
} t_COM_UPDATE_DATA;

#define	f_NTNET_RCV_MC10_EXEC	(OPECTL.NTNET_RCV_DATA.BIT.MC10_EXEC)	// mc10()実施要因発生ﾌﾗｸﾞ
#define	f_NTNET_RCV_MNTTIMCHG	(OPECTL.NTNET_RCV_DATA.BIT.MNTTIMCHG)	// ﾒﾝﾃﾅﾝｽ現在時刻変更ﾌﾗｸﾞ
#define	f_NTNET_RCV_KENKIGEN	(OPECTL.NTNET_RCV_DATA.BIT.KENKIGEN)	// 券期限設定ﾃﾞｰﾀ更新ﾌﾗｸﾞ
#define	f_NTNET_RCV_SPLDATE		(OPECTL.NTNET_RCV_DATA.BIT.SPLDATE)		// 特別日設定ﾃﾞｰﾀ更新ﾌﾗｸﾞ
#define	f_NTNET_RCV_USERSET		(OPECTL.NTNET_RCV_DATA.BIT.USERSET)		// ﾕｰｻｰ設定ﾃﾞｰﾀ更新ﾌﾗｸﾞ
#define	f_NTNET_RCV_TEIKITHUSI	(OPECTL.NTNET_RCV_DATA.BIT.TEIKITHUSI)	// 定期券精算中止設定ﾃﾞｰﾀ更新ﾌﾗｸﾞ
#define	f_NTNET_RCV_TEIKIEXTIM	(OPECTL.NTNET_RCV_DATA.BIT.TEIKIEXTIM)	// 定期券出庫時刻ﾃﾞｰﾀ更新ﾌﾗｸﾞ


/*** ｵﾍﾟﾚｰｼｮﾝ制御関連定義 ***/
struct	OPECTL_rec {
	char	Mnt_mod;										// 0:通常(待機,精算)
															// 1:ﾊﾟｽﾜｰﾄﾞ,修正,ﾊﾟｽﾜｰﾄﾞ忘れの選択
															// 2:係員,管理者ﾒﾝﾃﾅﾝｽ
															// 3:技術員ﾒﾝﾃﾅﾝｽ
															// 4:補充ｶｰﾄﾞﾒﾝﾃﾅﾝｽﾓｰﾄﾞ
															// 5:工場検査ﾓｰﾄﾞ
	char	Mnt_lev;										//  1:ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙ1(係員操作)
															//  2:ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙ2(管理者操作)
															//  3:ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙ3(技術員操作)
															// -1:ﾒﾝﾃﾅﾝｽﾚﾍﾞﾙﾊﾟｽﾜｰﾄﾞ無し
	uchar	Kakari_Num;										// ﾒﾝﾃﾅﾝｽ係員番号(0=不明、1～10=ﾊﾟｽﾜｰﾄﾞ入力した係員番号、99=ｱﾏﾉ技術員)
	uchar	Ope_mod;										// ｵﾍﾟﾚｰｼｮﾝ状態№ OpeMain()参照
	uchar	Pay_mod;										// 0:通常精算
															// 1:ﾊﾟｽﾜｰﾄﾞ忘れ精算
															// 2:修正精算
	uchar	Comd_knd;										// ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ種別
	uchar	Comd_cod;										// ﾒｯｾｰｼﾞｺﾏﾝﾄﾞｺｰﾄﾞ
	char	on_off;											// 0:KEY OFF
															// 1:KEY ON
	ulong	Op_LokNo;										// 接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)
	ushort	Pr_LokNo;										// 内部処理用駐車位置番号(1～324)
	ulong	MOp_LokNo;										// 間違えた接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)
	ushort	MPr_LokNo;										// 間違えた内部処理用駐車位置番号(1～324)
	char	op_faz;											// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ
															// 0:入金可送信
															// 1:入金中(入金有り時にｾｯﾄ)
															// 2:精算完了(入金不可送信後ｾｯﾄ)
															// 3:中止中(取消後に入金不可送信後ｾｯﾄ
															// 4:ﾀｲﾑｱｳﾄ(ﾀｲﾑｱｳﾄし入金不可送信後ｾｯﾄ
															// 8:電子媒体停止待ち合わせﾌｪｰｽﾞ（詳細はSTOP_REASONで検索）
															// 9:精算中止時のSuica停止待ち合わせﾌｪｰｽﾞ(正常に停止しなかった場合用)
															//10:Edy決済時のSuica停止完了待ちﾌｪｰｽﾞ
	char	RECI_SW;										//  0:領収書ﾎﾞﾀﾝ未使用
															//  1:領収書ﾎﾞﾀﾝ使用
															// -1:待機中領収証ﾎﾞﾀﾝ可
	char	CAN_SW;											// 0:中止ｽｲｯﾁOFF
															// 1:中止ｽｲｯﾁON
	uchar	opncls_eigyo;									// 強制営業信号 0:シグナルOFF  1:シグナルON
	uchar	opncls_kyugyo;									// 強制休業信号 0:シグナルOFF  1:シグナルON
	uchar	CN_QSIG;										// ｺｲﾝﾒｯｸｺﾏﾝﾄﾞ
	uchar	NT_QSIG;										// 紙幣ﾘｰﾀﾞｰｺﾏﾝﾄﾞ
	char	Ope_Mnt_flg;									// ｵﾍﾟﾚｰｼｮﾝﾒﾝﾃﾅﾝｽﾌﾗｸﾞ
															//  1:ｷｰ入力ﾁｪｯｸ
															//  2:LCDﾁｪｯｸ
															//  3:LEDﾁｪｯｸ
															//  4:紙幣ﾘｰﾀﾞｰﾁｪｯｸ
															//  5:ｺｲﾝﾒｯｸﾁｪｯｸ
															//  6:SWﾁｪｯｸ
															//  7:ｼｬｯﾀｰﾁｪｯｸ
															//  8:入出力信号ﾁｪｯｸ
															//  9:ﾌﾟﾘﾝﾀﾁｪｯｸ
															// 10:磁気ﾘｰﾀﾞｰﾁｪｯｸ
															// 11:ｱﾅｳﾝｽﾁｪｯｸ
															// 12:ﾒﾓﾘｰﾁｪｯｸ
															// 13:IF盤ﾁｪｯｸ
															// 14:ARCNETﾁｪｯｸ
															// 15:NTNETﾁｪｯｸ
	char	Ope_err;										// ｵﾍﾟﾚｰｼｮﾝｴﾗｰﾌﾗｸﾞ
	char	coin_syukei;									// ｺｲﾝ金庫集計ﾌﾗｸﾞ
	char	note_syukei;									// 紙幣金庫集計ﾌﾗｸﾞ
	char	nyukin_flg;										// ｺｲﾝ紙幣入金ﾌﾗｸﾞ
	char	cnsend_flg;										// ｺｲﾝ紙幣入金可送信済みﾌﾗｸﾞ
	char	flp_recover;									// 入庫情報復帰ﾌﾗｸﾞ
	short	Fin_mod;										// 1:精算完了釣り無し
															// 2:精算完了釣り有り
															// 3:ﾒｯｸｴﾗｰ発生
	uchar	LastCard;										// 最後に使用されたカード
															// (OPE_LAST_RCARD_NONE/OPE_LAST_RCARD_MAG/OPE_LAST_RCARD_MIF)
	short	LastCardInfo;									// 券読み取り情報(券ﾃﾞｰﾀ表示用)
	short	CR_ERR_DSP;										// 券ｴﾗｰ情報
	short	MIF_CR_ERR;										// Mifare券ｴﾗｰ
	T_FrmEnd	PriEndMsg[2];								// プリンタ異常時表示用
	uchar	Pri_Result;										// 印字終了時の印字結果	（印字終了ﾒｯｾｰｼﾞ：BModeを格納）
	uchar	Pri_Kind;										// 印字を終了したﾌﾟﾘﾝﾀ	（印字終了ﾒｯｾｰｼﾞ：BPrikindを格納）
	char	NtnetTimRec;									// NT-NET時計受信ﾌﾗｸﾞ
	uchar	Seisan_Chk_mod;									// 精算ﾁｪｯｸﾓｰﾄﾞ（OFF:通常／ON:精算ﾁｪｯｸﾓｰﾄﾞ）
	uchar	Ent_Key_Sts;									// 登録ｷｰ押下状態（OFF／ON）
	uchar	Can_Key_Sts;									// 取消ｷｰ押下状態（OFF／ON）
	uchar	EntCan_Sts;										// 登録ｷｰ＆取消ｷｰ同時押下状態（OFF／ON）
	uchar	PriUsed;										// <>0:ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞを使用
	ulong	LastUsePCardBal;								// 最後に利用したﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞの残額
	ulong	multi_lk;										// ﾏﾙﾁ精算問合せ中車室№
	uchar	Apass_off_seisan;								// 1=強制ｱﾝﾁﾊﾟｽOFF設定（定期利用時）
	short	ChkPassSyu;										// 定期券問合せ中券種0=問合せなし,1=APS定期券,2=Mifare,0xfe=通信不良時,0xff=問合せﾀｲﾑｱｳﾄ
	ulong	ChkPassPkno;									// 定期券問合せ中駐車場№
	ulong	ChkPassID;										// 定期券問合せ中定期券ID
	short	PassNearEnd;									// 1=期限切れ間近
	uchar	f_KanSyuu_Cmp;									// 1=完了集計実施済み
	ushort	op_mod02_dispCnt[3];							// 料金画面での利用可能媒体切替表示用カウンタ
	t_COM_UPDATE_DATA	NTNET_RCV_DATA;						// 通信でﾃﾞｰﾀ更新した情報
	uchar	InquiryFlg;		// 外部照会中フラグ
	uchar	credit_in_mony;									// ｸﾚｼﾞｯﾄすれ違い入金あり
	uchar	other_machine_card;								// 他の精算機のｶｰﾄﾞ
	uchar	InLagTimeMode;									// ラグタイム延長処理実行
	char	PasswordLevel;									// パスワード or 係員カードレベル 0～4:レベル, -1:パスワード未設定
	uchar	f_DoorSts;										// ドア状態（0:閉、1:開）
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
	uchar	CalStartTrigger;								// 料金計算ﾄﾘｶﾞｰ
															// 0x14: 紛失精算(ﾎﾞﾀﾝ)による精算	（自動精算）
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH321800(S) Y.Tanizaki ICクレジット対応
	uchar	holdPayoutForEcStop;							// 払出可保留フラグ
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810100(S) K.Onodera 2019/12/09 車番チケットレス(起動時処理)
	uchar	init_sts;										// 起動ステータス
	char	sended_opcls;									// 送信済みの営休業状態
	uchar	remote_wait_flg;								// 遠隔精算要求後の応答待ちフラグ
// MH810100(E) K.Onodera 2019/12/09 車番チケットレス(起動時処理)
// MH810100(S) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
	uchar	chg_idle_disp_rec;								// 待機画面通知受信フラグ
// MH810100(E) K.Onodera 2020/03/26 #4065 車番チケットレス（精算終了画面遷移後、音声案内が完了するまで「トップに戻る」を押下しても初期画面に遷移できない不具合対応）
// MH810100(S) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
	uchar	lcd_prm_update;									// LCDへのパラメータアップロード中
// MH810100(E) K.Onodera  2020/03/31 #4098 車番チケットレス(プログラムダウンロード中の電源OFF/ONで再開失敗)
// MH810103(s) 電子マネー対応 待機画面での残高照会可否
	uchar	lcd_query_onoff;								// 待機画面での残高照会可否(0:NG/1:OK)
// MH810103(e) 電子マネー対応 待機画面での残高照会可否
// MH810104 GG119201(S) 電子ジャーナル対応
	uchar	EJAClkSetReq;									// 電子ジャーナルへの時計設定要求
// MH810104 GG119201(E) 電子ジャーナル対応
// MH810104(S) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加
	uchar	RT_show_attention;								// 料金計算テストの注意画面表示(0:表示しない/1:表示する)
// MH810104(E) R.Endo 2021/09/29 車番チケットレス フェーズ2.3 #6031 料金計算テストの注意画面追加
// MH810105(S) MH364301 インボイス対応
	uchar	f_DelayRyoIsu;									// レシート印字遅延フラグ 1:ジャーナル印字完了待ち、2:ジャーナル印字完了
	uchar	f_ReIsuType;									// 再発行種別 1:PWeb, 2:PIP
	uchar	f_RctErrDisp;									// 領収証発行失敗表示フラグ
	uchar	f_CrErrDisp;									// 券エラー表示中フラグ 0:非表示, 1:表示中
// MH810105(E) MH364301 インボイス対応
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	ushort	f_searchtype;									// 検索タイプフラグ 0:車番検索,1:時刻検索,2:QR検索
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
// GG129000(S) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
	uchar	f_req_paystop;									// LCDからの精算中止要求(0:とりけしボタン、1:戻るボタン)
// GG129000(E) M.Fujikawa 2023/09/19 ゲート式車番チケットレスシステム対応　改善連絡No.52
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
	uchar	f_rtm_remote_pay_flg;							// 遠隔精算（リアルタイム）開始受付フラグ
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	uchar	f_eReceiptReserve;								// 電子領収証予約フラグ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
};
extern	struct OPECTL_rec	OPECTL;

// MH322914(S) K.Onodera 2016/10/05 AI-V対応
#define		CAL_TRIGGER_REMOTECALC_TIME		0x55			// 遠隔精算(入庫時刻指定)
#define		CAL_TRIGGER_REMOTECALC_FEE		0x56			// 遠隔精算(金額指定)
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加)_for_LCD_IN_CAR_INFO_op_mod00))
// 精算としては入庫時刻指定固定で精算を行う
#define		CAL_TRIGGER_LCD_IN_TIME			0x65			// 車番チケットレス精算(入庫時刻指定)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加)_for_LCD_IN_CAR_INFO_op_mod00))
// MH322914(E) K.Onodera 2016/10/05 AI-V対応

typedef struct {
	ulong			op_lokno;								// 接客用駐車位置番号(上位2桁A～Z,下位4桁1～9999,計6桁)
	ushort			pr_lokno;								// 内部処理用駐車位置番号(1～324)
	uchar			kind;									// 料金種別(車種)
	uchar			resv;
	ulong			fee;									// 料金計算結果
	ushort			nstat;									// 現在ステータス
	struct clk_rec	indate;									// 入庫日時
	struct clk_rec	outdate;								// 出庫日時
} RYO_INFO;

typedef struct {
	ushort			GroupNo;								// グループNo
	RYO_INFO		RyoInfo[OLD_LOCK_MAX];					// 料金情報
} RYOCALSIM_CTRL;
extern	RYOCALSIM_CTRL		RyoCalSim;

// MH322914 (s) kasiyama 2016/07/15 AI-V対応
#pragma pack
typedef struct{
	uchar			ReqKind;
	uchar			OpenClose;				// 営休業状況
	uchar			MntMode;				// 精算機動作モード状況(0=通常/1=メンテ)
	uchar			ErrOccur;				// エラー発生状況(0=なし/1=あり)
	uchar			AlmOccur;				// アラーム発生状況(0=なし/1=あり)
	ushort			PayState;				// 精算処理状態
	ushort			PassUse;				// 振替元定期利用有無
	ushort			SrcArea;				// 振替元区画
	ulong			SrcNo;					// 振替元車室
	date_time_rec2	SrcPayTime;				// 振替元精算年月日時分秒
	date_time_rec2	SrcInTime;				// 振替元入庫年月日時分秒
	ushort			SrcFeeKind;				// 振替元料金種別
	ulong			SrcFee;					// 振替元駐車料金
	ulong			SrcFeeDiscount;			// 振替元割引金額
	ushort			SrcStatus;				// 振替元ステータス
	ushort			DestArea;				// 振替先区画
	ulong			DestNo;					// 振替先車室
	date_time_rec2	DestInTime;				// 振替先入庫年月日時分秒
	date_time_rec2	DestPayTime;			// 振替先精算年月日時分秒
	ushort			DestFeeKind;			// 振替先料金種別
	ulong			DestFee;				// 振替先駐車料金
	ulong			DestFeeDiscount;		// 振替先割引金額
	ushort			DestStatus;				// 振替先ステータス
	ulong			PassParkingNo;			// 定期券駐車場番号
	ulong			PassID;					// 定期券ID
	ushort			PassKind;				// 定期券種別
	long			Remain;					// 支払残額
}PIP_FURIKAE_INFO;
typedef struct{
	uchar			ReqKind;
	ushort			Area;					// 区画
	ulong			ulNo;					// 車室番号
	date_time_rec2	InTime;					// 入庫年月日時分秒
	date_time_rec2	OutTime;				// 料金計算年月日時分秒
	ulong			ulPno;					// 駐車場№
	ushort			RyoSyu;					// 料金種別
	ulong			Price;					// 駐車場金
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
	ulong			FutureFee;				// 後日精算予定額
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
	ushort			DiscountKind;			// 割引種別
	ushort			DiscountType;			// 割引区分
	ushort			DiscountCnt;			// 割引使用枚数
	ulong			Discount;				// 割引金額
	ulong			DiscountInfo1;			// 割引情報１
	ulong			DiscountInfo2;			// 割引情報２
}PIP_REMOTE_TIME;
typedef struct{
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	uchar			ReqKind;
	uchar			Type;					// 遠隔精算種別
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	ushort			Area;					// 区画
	ulong			ulNo;					// 車室番号
	ulong			Price;					// 駐車料金			0～
// 仕様変更(S) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
	ulong			FutureFee;				// 後日精算予定額
// 仕様変更(E) K.Onodera 2016/11/04 遠隔精算フォーマット変更対応
	ushort			RyoSyu;					// 料金種別
	ushort			DiscountKind;			// 割引種別
	ushort			DiscountType;			// 割引区分
	ushort			DiscountCnt;			// 割引使用枚数
	ulong			Discount;				// 割引金額
	ulong			DiscountInfo1;			// 割引情報１
	ulong			DiscountInfo2;			// 割引情報２
}PIP_REMOTE_FEE;

typedef union {
	PIP_FURIKAE_INFO	stFurikaeInfo;						// 振替対象情報
	PIP_REMOTE_TIME		stRemoteTime;						// 遠隔精算時刻指定
	PIP_REMOTE_FEE		stRemoteFee;						// 遠隔精算金額指定
} PIP_CTRL;
extern	PIP_CTRL		g_PipCtrl;
#pragma unpack
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

enum {
	OPE_LAST_RCARD_NONE = 0,
	OPE_LAST_RCARD_MAG,		// 磁気カード
};

/*** ｼｬｯﾀｰ制御関連定義 ***/
typedef union{
	uchar	BYTE;											// Byte
	struct{
		uchar	YOBI2:2;									// Bit 6,7 = 予備
		uchar	YOBI1:2;									// Bit 4,5 = 予備
		uchar	COIN:2;										// Bit 2,3 = ｺｲﾝ投入口ｼｬｯﾀｰ動作ﾘｸｴｽﾄ(0=ﾘｸｴｽﾄ無し,1=ｼｬｯﾀｰ閉,2=ｼｬｯﾀｰ開)
		uchar	READ:2;										// Bit 0,1 = 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ動作ﾘｸｴｽﾄ(0=ﾘｸｴｽﾄ無し,1=ｼｬｯﾀｰ閉,2=ｼｬｯﾀｰ開)
	} BIT;
} SHT_RQ;

extern	SHT_RQ			SHT_REQ;							// ｼｬｯﾀｰ制御
extern	SHT_RQ			SHT_CTRL;							// ｼｬｯﾀｰ動作中ﾌﾗｸﾞ
extern	char			READ_SHT_flg;						// 0=初期,1=ｼｬｯﾀｰ閉,2=ｼｬｯﾀｰ開
extern	char			COIN_SHT_flg;						// 0=初期,1=ｼｬｯﾀｰ閉,2=ｼｬｯﾀｰ開

extern	uchar	Kakari_Numu[LOCK_MAX];											// ﾒﾝﾃﾅﾝｽ係員番号(0=不明、1～10=ﾊﾟｽﾜｰﾄﾞ入力した係員番号、99=ｱﾏﾉ技術員)
extern	uchar	Flp_LagExtCnt[LOCK_MAX];					/* ラグタイム延長回数			*/

#define RECODE_SIZE		(0x1000-6)	// 一次格納サイズ/項目(FLT_LOGOFS_RECORD=5)
#define RECODE_SIZE2	(0x2000-6)	// 一次格納サイズ/項目(FLT_LOGOFS_RECORD=5)

/*** ﾛｸﾞ関連定義 ***/
enum {	// ログ種別
	eLOG_PAYMENT = 0,		// 精算
// MH810100(S) K.Onodera 2019/12/15 車番チケットレス(RT精算データ対応)
//	eLOG_ENTER,				// 入庫
	eLOG_RTPAY,				// RT精算データ
// MH810100(E) K.Onodera 2019/12/15 車番チケットレス(RT精算データ対応)
	eLOG_TTOTAL,			// 集計
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	eLOG_LCKTTL,			// 車室毎集計
	eLOG_RTRECEIPT,			// RT領収証データ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	eLOG_ERROR,				// エラー
	eLOG_ALARM,				// アラーム
	eLOG_OPERATE,			// 操作
	eLOG_MONITOR,			// モニタ
	eLOG_ABNORMAL,			// 不正強制出庫
	eLOG_MONEYMANAGE,		// 釣銭管理
	eLOG_PARKING,			// 駐車台数データ
	eLOG_NGLOG,				// 不正情報
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
//	eLOG_IOLOG,				// 入出庫履歴
	eLOG_DC_QR,				// DC-NET QR確定・取消データ
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
	eLOG_CREUSE,			// クレジット利用
	eLOG_iDUSE,				// ID利用
	eLOG_HOJIN_USE,			// 法人カード利用
	eLOG_REMOTE_SET,		// 遠隔料金設定
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//	eLOG_LONGPARK,			// 長期駐車
	eLOG_LONGPARK_PWEB,		// 長期駐車(ParkingWeb用)
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	eLOG_RISMEVENT,			// RISMイベント
	eLOG_DC_LANE,			// DC-NET レーンモニタデータ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	eLOG_GTTOTAL,			// GT合計
	eLOG_REMOTE_MONITOR,	// 遠隔監視
	eLOG_COINBOX,			// コイン金庫集計(ramのみ)
	eLOG_NOTEBOX,			// 紙幣金庫集計(ramのみ)
	eLOG_EDYARM,			// Edyアラーム(ramのみ)
	eLOG_EDYSHIME,			// Edy締め(ramのみ)
	eLOG_POWERON,			// 復電(ramのみ)
	eLOG_MNYMNG_SRAM,		// 金銭管理(SRAM)
	eLOG_MAX				// terminator
};

enum {	// ログターゲット種別
	eLOG_TARGET_NTNET = 0,	// NT-NET
	eLOG_TARGET_RISM,		// RISM
	eLOG_TARGET_PARKIPRO,	// Park i pro
	eLOG_TARGET_REMOTE,		// 遠隔NT-NET
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(RT精算データ対応)
//	eLOG_TARGET_YOBI,		// 予備
	eLOG_TARGET_LCD,		// LCD通信
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(RT精算データ対応)
	eLOG_TARGET_MAX			// terminator
};

typedef	struct {						// ニアフル状態管理
	ushort	NearFullMaximum;			// ニアフルアラーム上限値(件数)
	ushort	NearFullMinimum;			// ニアフルアラーム解除値(件数)
	ushort	NearFullStatus;				// ニアフル発生状態
	ushort	NearFullStatusBefore;		// ニアフル発生状態（前回）
} near_full;

// MH322917(S) A.Iiizumi 2018/11/22 注意事項のコメント追加
// 注意事項：log_record構造体はRAM上に保持しているログの管理領域のためメンバを変更しないこと
// 変更が必要な場合はメモリ配置が換わるためバージョンアップでログをログバージョン「LOG_VERSION」をアップデートし、
// opetask()の中でLOG_VERSIONにあった全ログクリア処理を追加すること
// MH322917(E) A.Iiizumi 2018/11/22 注意事項のコメント追加
struct log_record {	// 履歴レコード
	short	kind;						// 種別(0/1面)
	short	count[2];					// 件数(RAM)
	ulong	wtp;						// ﾗｲﾄﾎﾟｲﾝﾀ(RAM)
	union{								// ﾊﾞｯﾌｧ(RAM)
		uchar	s1[2][RECODE_SIZE];
		uchar	s2[RECODE_SIZE2];
	} dat;
	short	Fcount;						// FLASH書込み済みｾｸﾀ件数
	short	Fwtp;						// FLASH書込みｾｸﾀ№
	ushort	unread[eLOG_TARGET_MAX];	// 下記解説参照
	ushort	f_unread[eLOG_TARGET_MAX];	// 未読レコード数(RAMのみにデータを持つ場合使用)
	ushort	void_read[eLOG_TARGET_MAX];	// 空読みレコード件数
	near_full	nearFull[eLOG_TARGET_MAX];	// ニアフル状態
	ushort	overWriteUnreadCount[eLOG_TARGET_MAX];	// バッファフル発生によりFROM書込み時に上書きした未送信データ数
	uchar	writeLogFlag[eLOG_TARGET_MAX];	// ログ書込みフラグ
	uchar	writeFullFlag[eLOG_TARGET_MAX];	// データフル(FROMセクタ書込み)フラグ
};
extern struct log_record	LOG_DAT[eLOG_MAX];
// MH322917(S) A.Iiizumi 2018/11/22 注意事項のコメント追加
// 注意事項：log_recover構造体はログ書き込みの停電リカバリ用エリアのためメンバを変更しないこと
// (ログ書き込み中にバージョンアップを伴う停電するとデータが壊れるため)
// 変更が必要な場合はメモリ配置が換わるためバージョンアップでログをログバージョン「LOG_VERSION」をアップデートし、
// opetask()の中でLOG_VERSIONにあった全ログクリア処理を追加すること
// MH322917(E) A.Iiizumi 2018/11/22 注意事項のコメント追加
struct log_recover {// ログ管理復電処理の構造体
	ushort	f_recover;		// 現在の処理状態
	short	Lno;			// LOG種別
	short	stat_kind;		// 種別(0/1面)
	short	stat_count;		//  件数(RAM)
	ulong	stat_wtp;		// ﾗｲﾄﾎﾟｲﾝﾀ(RAM)
	void	*dat_p;			// LOGデータのポインタ
	BOOL	strage;			// SRAMフラグ
	ushort	f_unread[eLOG_TARGET_MAX];// 未読レコード数(RAMのみにデータを持つ場合使用)
	uchar	dummy[20];		// 予備領域
};
extern struct log_recover	log_bakup;// ログ管理復電処理の退避エリア
extern	const ushort LogDatMax[][2];
#define LOG_SECORNUM(x)	(LogDatMax[(x)][1]/LogDatMax[(x)][0])

enum {	// ログ種別
	LOG_PAYMENT = 0,											// 個別精算
	LOG_PAYSTOP,												// 精算中止
	LOG_ABNORMAL,												// 不正・強制出庫
	LOG_TTOTAL,													// Ｔ合計
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	LOG_LCKTTL,													// 車室毎集計
	LOG_RTRECEIPT,												// RT領収証データ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	LOG_MONEYMANAGE,											// 金銭管理
	LOG_COINBOX,												// COIN金庫集計
	LOG_NOTEBOX,												// 紙幣金庫集計
	LOG_POWERON,												// 停復電
	LOG_ERROR,													// ｴﾗｰ
	LOG_ALARM,													// ｱﾗｰﾑ
	LOG_OPERATE,												// 操作履歴
	LOG_MONITOR,												// モニタ
	LOG_CREUSE,													// クレジット利用明細ログ
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	LOG_EDYARM,													// Edyアラーム取引ログ
//	LOG_EDYSHIME,												// Edy締め記録ログ
//	LOG_EDYMEISAI,												// カード精算(Ｅｄｙ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	LOG_SCAMEISAI,												// カード精算(Ｓｕｉｃａ)
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	LOG_EDYSYUUKEI,												// カード精算(Ｅｄｙ)
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	LOG_SCASYUUKEI,												// カード精算(Ｓｕｉｃａ)
	LOG_NGLOG,													// 不正ログ
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応）
//	LOG_IOLOG,													// 入出庫ログ
	LOG_DC_QR,													// QR確定・取消データ
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応）
	LOG_HOJIN_USE,												// 法人カード利用
	LOG_REMOTE_SET,												// 遠隔料金設定
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(RT精算データ対応)
//	LOG_ENTER,													// 入庫
	LOG_RTPAY,													// RT精算データ
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(RT精算データ対応)
	LOG_PARKING,												// 駐車台数データ
	LOG_LONGPARK,												// 長期駐車
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	LOG_RISMEVENT,												// RISMイベント
	LOG_DC_LANE,												// レーンモニタデータ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	LOG_MONEYMANAGE_NT,											// 金銭管理データ(NT-NET)
	LOG_MNYMNG_SRAM,											// 金銭管理(SRAM)
	LOG_RTPAYMENT,												// 料金テスト
	LOG_GTTOTAL,												// GT合計
	LOG_REMOTE_MONITOR,											// 遠隔監視
// MH321800(S) G.So ICクレジット対応
	LOG_ECSYUUKEI,												// カード精算(決済リーダ)
	LOG_ECMEISAI,												// カード精算(決済リーダ)
// MH321800(E) G.So ICクレジット対応
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
	LOG_ECMINASHI,												// みなし決済(決済リーダ)
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// MH810105(S) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
	LOG_PAYSTOP_FU,												// 復電時の中止データ
// MH810105(E) #6207 未了再タッチ待ち中に電源OFF→ON後、NTNET精算データを送信しない
	LOG_DEFINE_MAX												// terminator
};

union log_rec {												// 登録用ﾜｰｸｴﾘｱ
	Pon_log			pon;
	Err_log			err;
	Arm_log			arm;
	Ope_log			ope;
	flp_log			flp;
//	Pay_log			pay;
//	Can_log			can;
	Mny_log			mny;
//	Fryo_log		Fryo;
	Receipt_data	receipt;
};

extern	union log_rec	logwork;

struct logwork_rec {
	short		wor_count;									// 件数ﾜｰｸ
	short		wor_wtp;									// ﾗｲﾄﾎﾟｲﾝﾀﾜｰｸ
};
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
// バーコード結果
typedef enum {
	RESULT_NO_ERROR,						// 使用可能なバーコード
	RESULT_QR_INQUIRYING,					// 問合せ中
	RESULT_DISCOUNT_TIME_MAX,				// 割引上限
	RESULT_BAR_USED,						// 使用済のバーコード
	RESULT_BAR_EXPIRED,						// 期限切れバーコード
	RESULT_BAR_READ_MAX,					// バーコード枚数上限
	RESULT_BAR_FORMAT_ERR,					// フォーマットエラー
	RESULT_BAR_ID_ERR,						// 対象外
// MH810100(S) 2020/06/19 #4503【連動評価指摘事項】サービス券QR適用後の種別切替の認証取消データが送信されてこない
	RESULT_BAR_NOT_USE_CHANGEKIND,			// 入金・割引済み後の車種切換
// MH810100(E) 2020/06/19 #4503【連動評価指摘事項】サービス券QR適用後の種別切替の認証取消データが送信されてこない
} OPE_RESULT;

// QR確定・取消処理区分
typedef enum {
	CERTIF_COMMIT,							// 確定
	CERTIF_ROLLBACK,						// 取消
} eCertifKind;
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)

/*** 定期券問合せ結果定義 ***/
typedef	struct {
	// NTNETのDATA_KIND_143と同様とする事。
	ushort			Reserve;								// 予備
	ulong			ParkingNo;								// 駐車場№
	ulong			PassID;									// 定期券ID
	uchar			PassState;								// 定期券ｽﾃｰﾀｽ
	uchar			RenewalState;							// 更新ｽﾃｰﾀｽ
	date_time_rec	OutTime;								// 出庫年月日時分
	ulong			UseParkingNo;							// 利用中駐車場№
} PASSCHK;

extern	PASSCHK		PassChk;								// 定期券問合せ結果ﾃﾞｰﾀ
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
// センター問い合わせ応答の定期情報チェック結果
typedef enum{
	SEASON_CHK_OK,					// エラーなし
	SEASON_CHK_INVALID_PARKINGLOT,	// 駐車場№エラー
	SEASON_CHK_INVALID_PASS_ID,		// 定期IDエラー
	SEASON_CHK_NO_MASTER_PARKINGLOT,// 親機の駐車場№一致なしエラー
	SEASON_CHK_INVALID_SEASONDATA,	// 無効定期エラー
	SEASON_CHK_FIRST_NG,			// 初回定期エラー
	SEASON_CHK_PRE_SALE_NG,			// 販売前定期エラー
	SEASON_CHK_INVALID_TYPE,		// 定期種別範囲外エラー
	SEASON_CHK_UNUSED_TYPE,			// 定期種別未使用エラー
	SEASON_CHK_OUT_OF_RNG_TYPE,		// 定期使用目的設定範囲外エラー
	SEASON_CHK_UNUSED_RYO_SYU,		// 料金種別未使用エラー
	SEASON_CHK_ORDER_OF_USE,		// 精算順番エラー
	SEASON_CHK_BEFORE_VALID,		// 期限前エラー
	SEASON_CHK_AFTER_VALID,			// 期限切れエラー
	SEASON_CHK_NTIPASS_ERROR,		// アンチパスエラー
} eSEASON_CHK_RESULT;
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)

/*** LED関連定義 ***/

/*** LedReq() への第1引数 ***/
// MH810100(S) Y.Yamauchi 2019/10/04 車番チケットレス(メンテナンス)
//#define		RD_SHUTLED			0							// 磁気リーダーシャッタLED
//#define		CN_TRAYLED			1							// 釣銭取出し口LED
//#define		LEDCT_MAX			2							// 上記引数の個数
#define		CN_TRAYLED			0							// 釣銭取出し口LED
#define		LEDCT_MAX			1							// 上記引数の個数
// MH810100(E) Y.Yamauchi 2019/10/04 車番チケットレス(メンテナンス)

/*** LedReq() への第2引数 ***/
#define		LED_OFF				0							// OFF
#define		LED_ON				1							// ON
#define		LED_ONOFF			2							// 点滅

/*** LedCtrl.Phase ***/
#define		LED_ON_TIME			25							// ON 時間 (500ms : 20ms*25)
#define		LED_OFF_TIME		25							// OFF時間 (500ms : 20ms*25)

/*** LED制御用 ***/
typedef struct {
	unsigned char	Request[LEDCT_MAX];						// 制御要求
	unsigned char	Phase;									// 点滅ﾌｪｰｽﾞ (0=現在点滅要求なし, 1=あり)
	unsigned char	OnOff;									// 点滅ﾌｪｰｽﾞ (0=現在OFF, 1=現在ON)
	unsigned char	Count;									// 点滅時間ｶｳﾝﾄ
} t_LedCtrl;

extern	t_LedCtrl	LedCtrl;								// LED制御

/*** 設定関連 ***/


/*** LCD関連 ***/
extern	ushort	LCDNO;										// 現在表示中の画面

extern	ushort	PowonSts_Param;								// 起動時のパラメータデータ状況
extern	ushort	PowonSts_LockParam;							// 起動時の車室パラメータデータ状況

/*** 入出力信号制御関連定義 ***/
#define		SIG_OFF				0							// OFF
#define		SIG_ON				1							// ON
#define		SIG_ONOFF			2							// ON-OFF(ﾜﾝｼｮｯﾄ)

#define		OutPortMax			7							// 出力信号数
// MH322914 (s) kasiyama 2016/07/11 運用設定プリントの出力信号数修正[共通バグNo.1266](MH341106)
#define		InPortMax			1							// 入力信号数
// MH322914 (e) kasiyama 2016/07/11 運用設定プリントの出力信号数修正[共通バグNo.1266](MH341106)
#define		INOUTMax			5							// ﾜﾝｼｮｯﾄ信号数(予備3つ)

#define		SIG_OUTCNT1			1							// 精算完了信号1(出庫信号1)
#define		SIG_OUTCNT2			2							// 精算完了信号2(出庫信号2)
#define		SIG_GATEOPN			3							// ｹﾞｰﾄ開信号
#define		SIG_GATECLS			4							// ｹﾞｰﾄ閉信号
#define		SIG_J_PEND			10							// ｼﾞｬｰﾅﾙ紙切れ
#define		SIG_J_PNEND			11							// ｼﾞｬｰﾅﾙ紙切れ予告
#define		SIG_R_PEND			12							// ﾚｼｰﾄ紙切れ
#define		SIG_R_PNEND			13							// ﾚｼｰﾄ紙切れ予告
#define		SIG_JR_PEND			14							// ｼﾞｬｰﾅﾙ又はﾚｼｰﾄ紙切れ
#define		SIG_JR_PNEND		15							// ｼﾞｬｰﾅﾙ又はﾚｼｰﾄ紙切れ予告
#define		SIG_TROUBLE			16							// ﾄﾗﾌﾞﾙ
#define		SIG_PAYMENT			17							// 精算中
#define		SIG_CARFUL1			18							// 満車1
#define		SIG_CARFUL2			19							// 満車2
#define		SIG_CARNFUL1		20							// ほぼ満車1
#define		SIG_CARNFUL2		21							// ほぼ満車2
#define		SIG_OUTALM			22							// 出庫警報
#define		SIG_CLOSE			24							// 休業中
#define		SIG_SECURITY		25							// 防犯センサー
#define		SIG_COINNFUL		26							// ｺｲﾝ金庫満杯予告
#define		SIG_NOTENFUL		27							// 紙幣金庫満杯予告
#define		SIG_SAFENFUL		28							// ｺｲﾝ金庫又は紙幣金庫満杯予告
#define		SIG_CHGEND			49							// 釣銭切れ
#define		SIG_CHGNEND			50							// 釣銭切れ予告
#define		SIG_COINFUL			60							// ｺｲﾝ金庫満杯
#define		SIG_NOTEFUL			61							// 紙幣金庫満杯
#define		SIG_SAFEFUL			62							// ｺｲﾝ金庫又は紙幣金庫満杯
#define		SIG_LPR_PEND		68							// ﾗﾍﾞﾙ紙切れ又はﾘﾎﾞﾝ切れ
#define		SIG_LPR_PNEND		69							// ﾗﾍﾞﾙ紙切れ又はﾘﾎﾞﾝ切れ予告
#define		SIG_CARFUL3			73							// 満車3
#define		SIG_CAREMPTY1		74							// 空車1
#define		SIG_CAREMPTY2		75							// 空車2
#define		SIG_CAREMPTY3		76							// 空車3

extern	unsigned char	sig_port[INOUTMax];					// ﾜﾝｼｮｯﾄ出力している信号の出力ﾎﾟｰﾄを保持
															// 出力信号の制御に使用

#define		EXPORTSIGNALNUM		27

#define		EXPORT_J_PEND		0							// ｼﾞｬｰﾅﾙ紙切れ
#define		EXPORT_J_PNEND		1							// ｼﾞｬｰﾅﾙ紙切れ予告
#define		EXPORT_R_PEND		2							// ﾚｼｰﾄ紙切れ
#define		EXPORT_R_PNEND		3							// ﾚｼｰﾄ紙切れ予告
#define		EXPORT_JR_PEND		4							// ｼﾞｬｰﾅﾙ又はﾚｼｰﾄ紙切れ
#define		EXPORT_JR_PNEND		5							// ｼﾞｬｰﾅﾙ又はﾚｼｰﾄ紙切れ予告
#define		EXPORT_TROUBLE		6							// ﾄﾗﾌﾞﾙ
#define		EXPORT_CARFUL1		7							// 満車1
#define		EXPORT_CARFUL2		8							// 満車2
#define		EXPORT_CLOSE		9							// 休業中
#define		EXPORT_COINNFUL		10							// ｺｲﾝ金庫満杯予告
#define		EXPORT_NOTENFUL		11							// 紙幣金庫満杯予告
#define		EXPORT_SAFENFUL		12							// ｺｲﾝ金庫又は紙幣金庫満杯予告
#define		EXPORT_COINFUL		13							// ｺｲﾝ金庫満杯
#define		EXPORT_NOTEFUL		14							// 紙幣金庫満杯
#define		EXPORT_SAFEFUL		15							// ｺｲﾝ金庫又は紙幣金庫満杯
#define		EXPORT_CHGEND		16							// 釣銭切れ
#define		EXPORT_CHGNEND		17							// 釣銭切れ予告
#define		EXPORT_LPR_PEND		18							// ﾗﾍﾞﾙ紙切れ又はﾘﾎﾞﾝ切れ
#define		EXPORT_LPR_PNEND	19							// ﾗﾍﾞﾙ紙切れ又はﾘﾎﾞﾝ切れ予告
#define		EXPORT_SECURITY		20							// 防犯センサー
#define		EXPORT_CARFUL3		21							// 満車3
#define		EXPORT_CAREMPTY1	22							// 空車1
#define		EXPORT_CAREMPTY2	23							// 空車2
#define		EXPORT_CAREMPTY3	24							// 空車3

#define		EXPORT_M_LD0		0							// ｽﾃｰﾀｽLED0
#define		EXPORT_M_LD1		1							// ｽﾃｰﾀｽLED1
#define		EXPORT_M_LD2		2							// ｽﾃｰﾀｽLED2
#define		EXPORT_M_LD3		3							// ｽﾃｰﾀｽLED3
#define		EXPORT_JP_RES		4							// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀﾘｾｯﾄ信号
#define		EXPORT_CF_RES		5							// ｸﾚｼﾞｯﾄFOMAﾘｾｯﾄ信号
#define		EXPORT_URES			6							// CPUﾓﾆﾀ用LED
#define		EXPORT_M_OUT		7							// RXM-1基板予備出力ﾎﾟｰﾄ
#define		EXPORT_RXI_OUT7		8							// RXI-1汎用出力ﾎﾟｰﾄ7
#define		EXPORT_RXI_OUT6		9							// RXI-1汎用出力ﾎﾟｰﾄ6
#define		EXPORT_RXI_OUT5		10							// RXI-1汎用出力ﾎﾟｰﾄ5
#define		EXPORT_RXI_OUT4		11							// RXI-1汎用出力ﾎﾟｰﾄ4
#define		EXPORT_RXI_OUT3		12							// RXI-1汎用出力ﾎﾟｰﾄ3
#define		EXPORT_RXI_OUT2		13							// RXI-1汎用出力ﾎﾟｰﾄ2
#define		EXPORT_RXI_OUT1		14							// RXI-1汎用出力ﾎﾟｰﾄ1
#define		EXPORT_LOCK			15							// RXI-1基板電磁ﾛｯｸ制御

extern	unsigned char	Ex_portFlag[EXPORTSIGNALNUM];		// ﾜｰｸ
extern	unsigned char	Ex_OutFlag[EXPORTSIGNALNUM];		// 出力状態 0:未発生,1:発生中
															//  0:ｼﾞｬｰﾅﾙ紙切れ
															//  1:ｼﾞｬｰﾅﾙ紙切れ予告
															//  2:ﾚｼｰﾄ紙切れ
															//  3:ﾚｼｰﾄ紙切れ予告
															//  4:ｼﾞｬｰﾅﾙ又はﾚｼｰﾄ紙切れ
															//  5:ｼﾞｬｰﾅﾙ又はﾚｼｰﾄ紙切れ予告
															//  6:ﾄﾗﾌﾞﾙ
															//  7:満車1
															//  8:満車2
															//  9:休業中
															// 10:ｺｲﾝ金庫満杯予告
															// 11:紙幣金庫満杯予告
															// 12:ｺｲﾝ又は紙幣金庫満杯予告
															// 13:ｺｲﾝ金庫満杯
															// 14:紙幣金庫満杯
															// 15:ｺｲﾝ又は紙幣金庫満杯
															// 16:釣銭切れ
															// 17:釣銭切れ予告
															// 18:ﾗﾍﾞﾙ紙切れ又はﾘﾎﾞﾝ切れ
															// 19:ﾗﾍﾞﾙ紙切れ又はﾘﾎﾞﾝ切れ予告
															// 20:防犯センサー
															// 21:満車3
															// 22:空車1
															// 23:空車2
															// 24:空車3


extern	uchar	f_OpeSig_1shot;								// 出力信号 1shot出力要求ﾌﾗｸﾞ（1=要求あり）
extern	ushort	OpeSig_1shotInfo[OutPortMax];				// 出力信号 1shot出力要求情報（残ON時間：x20ms値）

#define		INSIG_GATEST		1							// ｹﾞｰﾄ閉状態
#define		INSIG_INCNT1		2							// 入庫1
#define		INSIG_INCNT2		3							// 入庫2
#define		INSIG_OUTCNT1		4							// 出庫1
#define		INSIG_OUTCNT2		5							// 出庫2
#define		INSIG_OPOPEN		6							// 強制営業
#define		INSIG_OPCLOSE		7							// 強制休業
#define		INSIG_SYUCHG1		8							// 種別切替信号1
#define		INSIG_SYUCHG2		9							// 種別切替信号2
#define		INSIG_FRCEXE		10							// 強制出庫信号

#define		INSIG_RTSW0			0							// モード設定SW
#define		INSIG_RTSW1			1							// モード設定SW
#define		INSIG_RTSW2			2							// モード設定SW
#define		INSIG_RTSW3			3							// モード設定SW
#define		INSIG_DPSW0			4							// Rism, Cappi通信Ethernet設定
#define		INSIG_DPSW1			5							// 設定SW_予備
#define		INSIG_DPSW2			6							// 設定SW_予備
#define		INSIG_DPSW3			7							// 設定SW_CAN終端設定
#define		INSIG_DUMMY1		8							// 未使用
#define		INSIG_DUMMY2		9							// 未使用
#define		INSIG_FANSTOP		10							// FAN停止温度検出信号
#define		INSIG_FANSTART		11							// FAN駆動開始温度検出信号
#define		INSIG_SWMODE		12							// 設定キー状態信号(予備)
#define		INSIG_DOOR			13							// ドアノブキー状態信号
#define		INSIG_RXMIN			14							// RXM-1基板予備入力ポート
#define		INSIG_RXIIN			15							// RXI-1基板汎用入力ポート

#define		ALL_COUNT			1							// 通し追番
#define		PAYMENT_COUNT		2							// 精算追番
#define		CANCEL_COUNT		3							// 精算中止追番
#define		DEPOSIT_COUNT		4							// 預り証追番
#define		T_TOTAL_COUNT		5							// T合計追番
#define		GT_TOTAL_COUNT		6							// GT合計追番
#define		F_TOTAL_COUNT		7							// 複数T合計追番
#define		COINMECK_COUNT		8							// ｺｲﾝﾒｯｸ追番
#define		TURIKAN_COUNT		9							// 釣銭管理合計追番
#define		COIN_SAFE_COUNT		10							// ｺｲﾝ金庫追番
#define		NOTE_SAFE_COUNT		11							// 紙幣金庫追番
#define		LOST_TIK_COUNT		12							// 紛失券発行追番
#define		INFO_COUNT			13							// 受付券発行追番
#define		KIYOUSEI_COUNT		14							// 強制出庫追番
#define		FUSEI_COUNT			15							// 不正出庫追番

enum{
	__Prev = FUSEI_COUNT,
	CRE_CAPPI_COUNT,
	SET_PRI_COUNT,											// 運用設定発行追番
	COUNT_MAX
	
};

#define		CLEAR_COUNT_ALL		0xF0						// 追番ｸﾘｱ(全て)
#define		CLEAR_COUNT_T		0xF1						// 追番ｸﾘｱ(T合計完了時)
#define		CLEAR_COUNT_GT		0xF2						// 追番ｸﾘｱ(GT合計完了時)
#define		CLEAR_COUNT_MT		0xF3						// 追番ｸﾘｱ(MT合計完了時)


/*** ｶｰﾄﾞ使用枚数 ***/
#define		USE_TIK				0							// 駐車券
#define		USE_PAS				1							// 定期券
#define		USE_PPC				2							// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ
#define		USE_NUM				3							// 回数券
#define		USE_SVC				4							// ｻｰﾋﾞｽ券,掛売券,割引券(中止時に覚えていた枚数も含む)
#define		USE_N_SVC			5							// ｻｰﾋﾞｽ券,掛売券,割引券(新規使用した枚数)
#define		USE_MAX				6							// ｶｰﾄﾞ毎使用枚数ﾃｰﾌﾞﾙ数MAX

extern	uchar	card_use[USE_MAX];							// ｶｰﾄﾞ毎使用枚数ﾃｰﾌﾞﾙ
extern	uchar	CardUseSyu;									// 1精算の割引種類の件数
extern	uchar	card_use2[15];							// ｻｰﾋﾞｽ券毎使用枚数ﾃｰﾌﾞﾙ

extern	const unsigned char		OPE_CHR[][31];
extern	const unsigned char		OPE_CHR2[][31];
extern	const unsigned char		OPE_CHR3[][31];
extern	const unsigned char		OPE_OKNG[][3];
extern	const unsigned char		OPE_CHR_CRE[][31];
extern	const unsigned char		ERR_CHR[][31];
extern	const unsigned char		BIG_CHR[][21];
extern	const unsigned char		BIG_OPE[];
extern	const unsigned char		OPE_ENG[];
extern	const unsigned char		SKEY_CHR[][7];
extern	const unsigned char		CLSMSG[][31];
extern	const unsigned char		DRCLS_ALM[][31];
extern	const unsigned short	WAKU_DSP[][30];
#define	MAX_MSG		40
// MH321800(S) 文字列の追加箇所を変更
//// MH321800(S) D.Inaba ICクレジット対応
////#define	MAX_EXMSG	4
//#define	MAX_EXMSG	5
//// MH321800(E) D.Inaba ICクレジット対応
#define	MAX_EXMSG	4
// MH321800(E) 文字列の追加箇所を変更
extern const unsigned char		OPE_CHR_G_SALE_MSG[MAX_MSG+MAX_EXMSG][31];
extern const unsigned char		OPE_CHR_CYCLIC_MSG[][17];
extern const unsigned char MAGREADERRMSG[][5];
extern  const unsigned char	sercret_Str[][31];
extern const unsigned char shomei_errstr[][31];

extern	ushort			key_num;							// 入力された駐車位置番号
extern	uchar			key_sec;							// 区画入力値
extern	ushort			knum_len;							// 入力可能な駐車位置番号の桁数
extern	ushort			knum_ket;							// 入力可能な駐車位置番号の桁
extern	struct logwork_rec	Log_Work;
extern	char			pas_NG;
extern	char			Err_onf;							// 現在発生ｴﾗｰ 0=無し、1=有り
extern	char			Alm_onf;							// 現在発生ｱﾗｰﾑ 0=無し、1=有り
extern	char			Err_trb;							// ﾄﾗﾌﾞﾙ信号 0=無し、1=有り(ｴﾗｰ用)
extern	char			Alm_trb;							// ﾄﾗﾌﾞﾙ信号 0=無し、1=有り(ｱﾗｰﾑ用)
extern	char			Err_cls;							// 休業要因ｴﾗｰ  0=無し、1=有り
extern	char			Alm_cls;							// 休業要因ｱﾗｰﾑ 0=無し、1=有り
extern	char			Security_out;						// 防犯センサ 0=無し、1=有り
extern	char			NgCard;								// NGカード読取内容
extern	unsigned short	bundat1[24];
extern	unsigned short	bundat2[24];
extern	unsigned short	bunbak1[24];
extern	unsigned short	bunbak2[24];
extern	char			bundat_onf;
extern	struct clk_rec	CLK_BAK;
extern	char			CLOSE_stat;							// 休業ｽﾃｰﾀｽ
extern	char			OPEN_stat;							// 営業ｽﾃｰﾀｽ
#define		OPEN_NTNET		1
#define		OPEN_TIMESET	2
extern	uchar			auto_syu_prn;						// 自動集計状態
extern	ushort			auto_syu_ndat;						// 自動集計開始日時
extern	uchar			coin_kinko_evt;						// コイン金庫イベント
extern	uchar			note_kinko_evt;						// 紙幣金庫イベント
extern	uchar			CARFULL_stat[4];					// 満空ｽﾃｰﾀｽ(Auto) 0=空車 1=満車
extern	ulong			PassPkNoBackup;						// 定期券駐車場番号ﾊﾞｯｸｱｯﾌﾟ(n分ﾁｪｯｸ用)
extern	ushort			PassIdBackup;						// 定期券idﾊﾞｯｸｱｯﾌﾟ(n分ﾁｪｯｸ用)
extern	ulong			PassIdBackupTim;					// n分ﾁｪｯｸ用ﾀｲﾏｰ
extern	unsigned short	StartingFunctionKey;				// 起動時に押されていたファンクションキー
extern	ushort			RECI_SW_Tim;						// 領収証発行可能ﾀｲﾏｰ

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//typedef	union {
//	ushort		SHORT;
//	struct {
//		uchar		yobi		:6;			// 予備
//		uchar		blink_status:1;			// ブリンクステータス
//		uchar		comm_type	:1;			// 締め有無フラグ
//		uchar		disp_type	:2;			// 画面表示有無フラグ
//		uchar		exec_status	:2;			// 通信実行中ステータス
//											// 1：センター通信開始
//											// 2：センター通信開始応答受信(通信中)
//											// 3：センター通信完了
//		uchar		status		:2;			// auto_cnt_prn(1-3)
//											// 	0:空き（開始時刻待ち）
//											// 	1:通信開始待ち
//											// 	2:通信印字中
//											// 	3:通信処理完了
//		uchar		wait_kind	:2;			// 次回通信種別
//											// 1：センター通信のみ
//											// 2：自動T合計後の締め＋センター通信
//	} BIT;
//}EDY_CNT_CTRL;
//extern	EDY_CNT_CTRL	edy_cnt_ctrl;
//#define	auto_cnt_prn	edy_cnt_ctrl.BIT.status
//#define	edy_auto_com	edy_cnt_ctrl.BIT.exec_status
//extern	ulong 			auto_cnt_ndat;						// 最終センター通信開始日時
//extern	Receipt_data	PayData_save;						// 停電前の精算情報,領収証印字ﾃﾞｰﾀ
//extern	uchar			fg_autocnt_fstchk;					/* 立上げ時のEdy自動センター通信要否確認フラグ（0:未/1:済） */
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

extern	char			RAMCLR;								// RAMｸﾘｱ実施ﾌﾗｸﾞ

extern	teiki_use		InTeiki_PayData_Tmp;				// 挿入定期ﾃﾞｰﾀ一時保存ｴﾘｱ
// ユーザーメンテナンスで設定変更した
typedef union{
	unsigned char	BYTE;
	struct{
		unsigned char yobi:3;			// B5-7 = 予備
		unsigned char cpara:1;			// B4 = 車室パラメータ
		unsigned char bpara:1;			// B3 = 共通パラメータ
		unsigned char splday:1;			// B2 = 特別日／特別期間
		unsigned char tickvalid:1;		// B1 = 券期限
		unsigned char other:1;			// B0 = その他
	}BIT;
} f_PARAUPDATE;
extern	f_PARAUPDATE	f_ParaUpdate;

// ユーザーメンテナンスの券期限の編集で使用するワークエリア
typedef struct {
	long	kind;				// 種別
	long	data;				// 内容
	long	date[4];			// [0]=変更前開始年月日、[1]=変更前終了年月日、[2]=変更後開始年月日、[3]=変更後終了年月日
} TIC_PRM;

typedef struct {
	char	no;					// 1=券期限１を編集、2=券期限２を編集、3=券期限３を編集
	TIC_PRM	tic_prm;			// 券期限の編集データ
} TICK_VALID_DATA;
extern	TICK_VALID_DATA	tick_valid_data;
extern	ushort	sntp_prm_before;

/* NT-NET精算ﾃﾞｰﾀ 補足情報 */
typedef struct {
	ushort	PayMethod;										// 精算方法（0=券無し精算、1=駐車券精算、2=定期券精算、3=定期券併用精算、4=紛失精算）
	ushort	PayClass;										// 処理区分（0=精算、1=再精算、2=精算中止、3=再精算中止）
	ushort	PayMode;										// 精算ﾓｰﾄﾞ（0=自動精算、1=半自動精算、2=手動精算、3=精算なし）
	ushort	CardType;										// 駐車券ﾀｲﾌﾟ（磁気ﾃﾞｰﾀID）
	ushort	CMachineNo;										// 駐車券発券機機械№（0～20）
	ulong	CardNo;											// 駐車券番号(発券追い番)（0～99999）
	ushort	KakariNo;										// 係員№（常に0）
	ushort	CountSet;										// 在車ｶｳﾝﾄ（0=する+1、1=しない、2=する-1）
	ushort	PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ（0=ﾁｪｯｸON,1=ﾁｪｯｸOFF,2=強制OFF）
	ushort	ReadMode;										// 定期券ﾗｲﾄﾓｰﾄﾞ（0=ﾘｰﾄﾞﾗｲﾄ、1=ﾘｰﾄﾞｵﾝﾘｰ）
	uchar	f_ChuusiCard;									// 中止券ﾌﾗｸﾞ（1=APS互換中止券, 2=割引券ﾌｫｰﾏｯﾄ中止券）
	uchar	f_ChgOver;										// 払戻上限額ｵｰﾊﾞｰ発生ﾌﾗｸﾞ(1=発生)
	ulong	BeforeTwariTime;								// 前回割引時間（分）
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	uchar	CreditDate[2];									// クレジットカード有効期限 [0]=年 [1]=月
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
} t_OpeNtnetAddedInfo;
extern	t_OpeNtnetAddedInfo	OpeNtnetAddedInfo;
extern	const uchar		Shunbun_Day[100];
extern	const uchar		Shuubun_Day[100];
extern	ushort			RenewalMonth;						// 定期券更新月数
extern	ulong			RenewalFee;							// 定期券更新料金
extern	uchar	Syusei_Select_Flg;
#define		APSF_PKNO_LOWER				1					// APSフォーマット 駐車場№下限
#define		APSF_PKNO_UPPER				999					// APSフォーマット 駐車場№上限
#define		GTF_PKNO_LOWER				1001				// GTフォーマット 駐車場№下限
#define		GTF_PKNO_UPPER				260001L				// GTフォーマット 駐車場№上限
extern	char	an_vol_flg;
extern	char	shomei_errnum;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
//extern	ushort	dspErrMsg;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
extern	unsigned char	Pri_Open_Status_R;			// プリンタカバー状態
extern	unsigned char	Pri_Open_Status_J;			// プリンタカバー状態

/*[]----------------------------------------------------------------------[]*/
/*| ｴﾗｰ制御                                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| ｴﾗｰｺｰﾄﾞ表 ﾓｼﾞｭｰﾙｺｰﾄﾞ定義                                               |*/
/*| err_chk(md,,)の第１引数mdとして使用する                                |*/
/*| moj_no[modno_note] == mod_noteとなる                                   |*/
/*[]----------------------------------------------------------------------[]*/
#define		ERRMDL_MAIN					0					/* ﾒｲﾝ制御ﾓｼﾞｭｰﾙｺｰﾄﾞ			*/
#define		ERRMDL_READER				1					/* 磁気ﾘｰﾀﾞｰﾓｼﾞｭｰﾙｺｰﾄﾞ			*/
#define		ERRMDL_PRINTER				2					/* ﾚｼｰﾄ,ｼﾞｬｰﾅﾙﾓｼﾞｭｰﾙｺｰﾄﾞ		*/
#define		ERRMDL_COIM					3					/* ｺｲﾝ識別循環ﾓｼﾞｭｰﾙｺｰﾄﾞ		*/
#define		ERRMDL_NOTE					4					/* 紙幣ﾘｰﾀﾞｰﾓｼﾞｭｰﾙｺｰﾄﾞ			*/
#define		ERRMDL_IFROCK				10					/* IF盤,ﾛｯｸ装置ﾓｼﾞｭｰﾙｺｰﾄﾞ		*/
#define		ERRMDL_IFFLAP				11					/* IF盤,ﾌﾗｯﾌﾟ装置ﾓｼﾞｭｰﾙｺｰﾄﾞ		*/
#define		ERRMDL_CRRFLAP				15					/* CRR基板,ﾌﾗｯﾌﾟ装置ﾓｼﾞｭｰﾙｺｰﾄﾞ	*/
#define		ERRMDL_FLAP_CRB				16					// 簡易フラップ／ロック装置（フラップ装置、CRB基板）モジュールコード
// MH810104 GG119201(S) 電子ジャーナル対応
#define		ERRMDL_EJA					22					// 電子ジャーナル
// MH810104 GG119201(E) 電子ジャーナル対応
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
#define		ERRMDL_CAL_CLOUD			24					// クラウド版料金計算エンジン
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
#define		ERRMDL_SODIAC				25					/* Sodiac（音声合成）ﾓｼﾞｭｰﾙｺｰﾄ	*/
// MH810100(S) K.Onodera 2019/12/12 車番チケットレス(LCD_IF対応)
#define		ERRMDL_TKLSLCD				26					/* 車番チケットレスLCDﾓｼﾞｭｰﾙｺｰﾄﾞ		*/
#define		ERRMDL_DC_NET				28					// DC-NET通信
#define		ERRMDL_BARCODE				30					// バーコードリーダ
// MH810103(s) 電子マネー対応 再送時のエラー種別がおかしい不具合修正
#define		ERRMDL_SD					31					// SDカード
// MH810103(e) 電子マネー対応 再送時のエラー種別がおかしい不具合修正
#define		ERRMDL_REALTIME				37					// リアルタイム通信
// MH810100(E) K.Onodera 2019/12/12 車番チケットレス(LCD_IF対応)
#define		ERRMDL_I2C					40					/* I2C Error情報	*/
// MH321800(S) G.So ICクレジット対応 不要機能削除(CCT)
//#define		ERRMDL_CCT					54					/* IBK(CCT)関連エラー */
// MH321800(E) G.So ICクレジット対応 不要機能削除(CCT)
#define		ERRMDL_REMOTE				55					/* 遠隔ﾓｼﾞｭｰﾙｺｰﾄﾞ				*/
#define		ERRMDL_MIFARE				59					/* Mifearﾓｼﾞｭｰﾙｺｰﾄﾞ				*/
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		ERRMDL_EDY					62					/* Edyﾓｼﾞｭｰﾙｺｰﾄﾞ				*/
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		ERRMDL_NTNETIBK				63					/* NT-NET IBKﾓｼﾞｭｰﾙｺｰﾄﾞ			*/
#define		ERRMDL_LABEL				64					/* ﾗﾍﾞﾙﾌﾟﾘﾝﾀﾓｼﾞｭｰﾙｺｰﾄﾞ			*/
#define		ERRMDL_NTNETDOPAMAF			70					/* NT-NET-DOPA MAFﾓｼﾞｭｰﾙｺｰﾄﾞ	*/
#define		ERRMDL_NTNETDOPA			ERRMDL_NTNETDOPAMAF		/* NT-NET-DOPA ﾓｼﾞｭｰﾙｺｰﾄﾞ		*/
#define		ERRMDL_NTNET				66					/* NT-NETﾓｼﾞｭｰﾙｺｰﾄﾞ				*/
#define		ERRMDL_SUICA				69					/* Suicaﾓｼﾞｭｰﾙｺｰﾄﾞ				*/

#define		ERRMDL_APSNETIBC			ERRMDL_NTNETIBK		/* APS-NET IBCﾓｼﾞｭｰﾙｺｰﾄﾞ		*/
#define		ERRMDL_APSNET				ERRMDL_NTNET		/* APS-NETﾓｼﾞｭｰﾙｺｰﾄﾞ			*/
#define		ERRMDL_LANIBK				72					/* 遠隔：２ポートLAN－IBK		*/
#define		ERRMDL_DOPAIBK				73					/* 遠隔：２ポートDoPa－IBK		*/
#define		ERRMDL_CAPPI				74					/* Cappi関連エラー */
#define		ERRMDL_IDC					75					/* iD関連エラー */
#define		ERRMDL_CARD					76					/* 特殊カードエラー */
#define		ERRMDL_FOMAIBK				77					/* 遠隔：FOMA－IBK		*/
#define		ERRMDL_SUBCPU				80					/* ｻﾌﾞCPUﾓｼﾞｭｰﾙｺｰﾄﾞ				*/
#define		ERRMDL_CAN					86					/* CANﾓｼﾞｭｰﾙｺｰﾄﾞ		*/
#define		ERRMDL_CREDIT				88					/* クレジット		*/
// MH321800(S) G.So ICクレジット対応
#define		ERRMDL_EC					32					/* 決済リーダﾓｼﾞｭｰﾙｺｰﾄﾞ	*/
// MH321800(E) G.So ICクレジット対応

//--------------------------------------------------------------------------
// 各ﾓｼﾞｭｰﾙごとのｴﾗｰｺｰﾄﾞ
//--------------------------------------------------------------------------

// ﾒｲﾝﾓｼﾞｭｰﾙ
#define		ERR_MAIN_MEMORY				1					// Memory  Error 'AMANO' Clash
#define		ERR_MAIN_BOOTINFO_MAIN		2					// ﾒｲﾝCPU運用面異常
#define		ERR_MAIN_BOOTINFO_SUB		3					// ｻﾌﾞCPU運用面異常
#define		ERR_MAIN_SETTEIRAM			5					// RAM上の設定ｴﾘｱ異常		（発生/解除）
#define		ERR_MAIN_SETTEIFLASH		6					// FLASH上の設定ｴﾘｱ異常		（発生/解除）
#define		ERR_MAIN_CLOCKFAIL			7					// 時計ﾃﾞｰﾀ異常
#define		ERR_MAIN_SUBCPU				8					// ３分間 分歩進なし
#define		ERR_MAIN_ARCNETBUFFULL		10					// ARCNETﾊﾞｯﾌｧﾌﾙ
#define		ERR_MAIN_MESSAGEFULL		11					// ﾒｯｾｰｼﾞｷｭｰﾌﾙ				（解除なし）
#define		ERR_MAIN_FLASH_ERASE_ERR	13					// FlashROM消去失敗
#define		ERR_MAIN_FLASH_WRITE_ERR	14					// FlashROM書き込み失敗
#define		ERR_MAIN_LOG_WRITE_LOCK		15					// ログ書込みのロック失敗
#define		ERR_MAIN_ARCNETICFAILUER	16					// ARCNETﾄﾞﾗｲﾊﾞ異常（初期化失敗）
#define		ERR_MAIN_READER_ARCNETFAIL	17					// 磁気ﾘｰﾀﾞｰ通信不良(ARCNET)
#define		ERR_MAIN_ANOUNCE_ARCNETFAIL	18					// ｱﾅｳﾝｽﾏｼﾝ通信不良(ARCNET)
#define		ERR_MAIN_IBC_ARCNETFAIL		23					// APSNETモジュール通信不良
#define		ERR_MAIN_IBKCOMFAIL_213		25					// IBKﾓｼﾞｭｰﾙ通信不良（ARCNET Node：213）
#define		ERR_MAIN_IBKCOMFAIL_214		26					// IBKﾓｼﾞｭｰﾙ通信不良（ARCNET Node：214）
#define		ERR_MAIN_IBKCOMFAIL_215		27					// IBKﾓｼﾞｭｰﾙ通信不良（ARCNET Node：215）
#define		ERR_MAIN_IBKCOMFAIL_216		28					// IBKﾓｼﾞｭｰﾙ通信不良（ARCNET Node：216）
#define		ERR_MAIN_ANNOUNCECOMFAIL	31					// ｱﾅｳﾝｽﾏｼﾝ通信不良
#define		ERR_MAIN_FLAPIBCCOM			35					// ﾌﾗｯﾌﾟ用IBCﾓｼﾞｭｰﾙ通信不良
#define		ERR_MAIN_BASICPARAMETER		40					// 設定値異常
#define		ERR_MAIN_CALPARAMETER_NG	50					// 運用設定プリント異常
#define		ERR_MAIN_CALPARAMETER_OK	51					// 運用設定プリント正常
#define		ERR_MAIN_ILLEGAL_INSTRUCTION 52					// 未定義命令例外割り込み発生
#define		ERR_MAIN_ILLEGAL_SUPERVISOR_INST	53			// 特権命令例外発生
#define		ERR_MAIN_ILLEGAL_FLOATINGPOINT		54			// 浮動小数点例外発生
#define		ERR_MAIN_UNKOWNVECT			55					// 未登録ﾍﾞｸﾀｱｸｾｽ
#define		ERR_MAIN_CLOCKREADFAIL		57					// 時計読み込み失敗
#define		ERR_MAIN_MACHINEREADFAIL	58					// 機器情報読み込み失敗
// MH322914 (s) kasiyama 2016/07/13 ログ保存できない状態を改善[共通バグNo.1225](MH341106)
#define		ERR_MAIN_SRAM_LOG_BUFFER_FULL	59				// SRAMログバッファフル
// MH322914 (e) kasiyama 2016/07/13 ログ保存できない状態を改善[共通バグNo.1225](MH341106)
#define		ERR_MAIN_OVER1YEARCAL		60					// 1年以上の料金計算を行った
#define		ERR_MAIN_INOUTTIMENG		61					// 入出庫時刻規定外
#define		ERR_MAIN_LOCKSTATUSNG		63					// ﾛｯｸ装置状態規定外
#define		ERR_MAIN_LOCKNUMNG			65					// 不正強制でﾛｯｸ装置番号が規定外
#define		ERR_MAIN_FKOVER1YEARCAL		66					// 1年以上の料金計算を行った(不正・強制)
#define		ERR_MAIN_FKINOUTTIMENG		67					// 入出庫時刻規定外(不正・強制)
#define		ERR_MAIN_NOSAFECLOPERATE	68					// 金庫枚数算出なしで抜けた
#define		ERR_MAIN_INOUTGYAKU			71					// 入出庫逆転で料金計算を行おうとした
#define		ERR_MAIN_FKINOUTGYAKU		72					// 入出庫逆転で料金計算を行おうとした(不正・強制)
#define		ERR_MAIN_FUSEI_RYOCAL		73					// 料金計算で無限ﾙｰﾌﾟ		（発生/解除）
#define		ERR_MAIN_SNTP_FAIL			83					// センター時刻同期失敗
#define		ERR_MAIN_RTCCPU_VOLDWN		85					// 時計IC発信回路電圧低下エラー		（発生/解除）
#define		ERR_MAIN_RTCTMPREC_VOLDWN	86					// 時計IC温度補償回路電圧低下エラー	（発生/解除）
#define		ERR_MAIN_BOOTINFO_WAVE		87					// 音声データ運用面異常
#define		ERR_MAIN_BOOTINFO_PARM		88					// 共通パラメータ運用面異常
#define		ERR_MAIN_MAFCOMFAIL_224		96					// MAFﾓｼﾞｭｰﾙ通信不良（ARCNET Node：224）
#define		ERR_MAIN_MAFCOMFAIL_225		97					// MAFﾓｼﾞｭｰﾙ通信不良（ARCNET Node：225）
#define		ERR_MAIN_MAFCOMFAIL_226		98					// MAFﾓｼﾞｭｰﾙ通信不良（ARCNET Node：226）予約
#define		ERR_MAIN_MAFCOMFAIL_227		99					// MAFﾓｼﾞｭｰﾙ通信不良（ARCNET Node：227）予約

// 磁気ﾘｰﾀﾞｰﾓｼﾞｭｰﾙ
#define		ERR_RED_COMFAIL				1					// 通信不良
#define		ERR_RED_MAGTYPE				2					// 磁気リーダータイプ異常
#define		ERR_RED_STACK_R				10					// ｶｰﾄﾞ詰まり(ﾘｰﾀﾞｰ内)
#define		ERR_RED_STACK_P				11					// ｶｰﾄﾞ詰まり(ﾌﾟﾘﾝﾀ内)
#define		ERR_RED_LOCSENSOR1			13					// 位置ｾﾝｻｰ1不良
#define		ERR_RED_LOCSENSOR2			14					// 位置ｾﾝｻｰ2不良
#define		ERR_RED_LOCSENSOR3			15					// 位置ｾﾝｻｰ3不良
#define		ERR_RED_LOCSENSOR4			16					// 位置ｾﾝｻｰ4不良
#define		ERR_RED_LOCSENSOR5			17					// 位置ｾﾝｻｰ5不良
#define		ERR_RED_LOCSENSOR6			18					// 位置ｾﾝｻｰ6不良
#define		ERR_RED_LOCSENSOR7			19					// 位置ｾﾝｻｰ7不良
#define		ERR_RED_PRNLOCSENSOR		20					// ﾌﾟﾘﾝﾀ位置ｾﾝｻｰ不良
#define		ERR_RED_START				21					// スタート符号エラー
#define		ERR_RED_DATA				22					// データエラー
#define		ERR_RED_PARITY				23					// パリティエラー
#define		ERR_RED_HPSENSOR1			24					// HPｾﾝｻｰ1不良
#define		ERR_RED_HPSENSOR2			25					// HPｾﾝｻｰ2不良
#define		ERR_RED_AFTERWRITE			30					// 書込後のｴﾗｰ
#define		ERR_RED_OTHERS				31					// その他のｴﾗｰ
#define		ERR_RED_MEMORY				32					// ﾒﾓﾘ不良
#define		ERR_RED_VERIFY				33					// ﾍﾞﾘﾌｧｲｴﾗｰ
#define		ERR_RED_SCI_PARITY			40					// シリアルパリティエラー
#define		ERR_RED_SCI_FRAME			41					// シリアルフレーミングエラー
#define		ERR_RED_SCI_OVERRUN			42					// シリアルオーバーランエラー

// ﾌﾟﾘﾝﾀｰ
#define		ERR_PRNT_R_PRINTCOM			1					// Printer Error
#define		ERR_PRNT_R_HEADHEET			3					// Head Heet Up Error
#define		ERR_PRNT_R_CUTTER			4					// Cutter       Error
#define		ERR_PRNT_J_PRINTCOM			20					// Printer Error
#define		ERR_PRNT_J_HEADHEET			22					// Head Heet Up Error
#define		ERR_PRNT_DATA_ERR			51					// 印字要求ﾃﾞｰﾀｴﾗｰ
#define		ERR_PRNT_LOG_ERR			52					// ﾛｸﾞﾃﾞｰﾀｴﾗｰ
#define		ERR_PRNT_YEAR_ERR			53					// 日付（年）ﾃﾞｰﾀｴﾗｰ
#define		ERR_PRNT_MON_ERR			54					// 日付（月）ﾃﾞｰﾀｴﾗｰ
#define		ERR_PRNT_DAY_ERR			55					// 日付（日）ﾃﾞｰﾀｴﾗｰ
#define		ERR_PRNT_HOUR_ERR			56					// 日付（時）ﾃﾞｰﾀｴﾗｰ
#define		ERR_PRNT_MIN_ERR			57					// 日付（分）ﾃﾞｰﾀｴﾗｰ
#define		ERR_PRNT_ERR_IRQ			61					// ﾌﾟﾘﾝﾀｴﾗｰ割込み発生
#define		ERR_PRNT_BUFF_FULL			62					// ﾌﾟﾘﾝﾀ送信ﾊﾞｯﾌｧｵｰﾊﾞｰﾌﾛｰ発生
#define		ERR_PRNT_INIT_ERR			63					// ﾌﾟﾘﾝﾀ初期化失敗
#define		ERR_PRNT_PARITY_ERR_R		80					// ﾚｼｰﾄﾌﾟﾘﾝﾀﾊﾟﾘﾃｨｴﾗｰ
#define		ERR_PRNT_FRAMING_ERR_R		81					// ﾚｼｰﾄﾌﾟﾘﾝﾀﾌﾚｰﾐﾝｸﾞｴﾗｰ
#define		ERR_PRNT_OVERRUN_ERR_R		82					// ﾚｼｰﾄﾌﾟﾘﾝﾀｵｰﾊﾞｰﾗﾝｴﾗｰ
#define		ERR_PRNT_PARITY_ERR_J		83					// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀﾊﾟﾘﾃｨｴﾗｰ
#define		ERR_PRNT_FRAMING_ERR_J		84					// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀﾌﾚｰﾐﾝｸﾞｴﾗｰ
#define		ERR_PRNT_OVERRUN_ERR_J		85					// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀｵｰﾊﾞｰﾗﾝｴﾗｰ

// MH810104 GG119201(S) 電子ジャーナル対応
#define		ERR_EJA_COMFAIL				1					// [01]通信不良
#define		ERR_EJA_SD_UNAVAILABLE		2					// [02]SDカード使用不可
#define		ERR_EJA_WRITE_ERR			3					// [03]書込み失敗
#define		ERR_EJA_UNCONNECTED			4					// [04]未接続
#define		ERR_EJA_ERR_IRQ				40					// [40]エラー割込み発生(FT-4000では発生しない)
#define		ERR_EJA_PARITY_ERR			41					// [41]パリティエラー
#define		ERR_EJA_FRAMING_ERR			42					// [42]フレーミングエラー
#define		ERR_EJA_OVERRUN_ERR			43					// [43]オーバーランエラー
// MH810104 GG119201(E) 電子ジャーナル対応

// ｺｲﾝﾒｯｸ
#define		ERR_COIN_COMFAIL			1					// 通信不良
#define		ERR_COIN_ACCEPTER			5					// ｱｸｾﾌﾟﾀ  Error
#define		ERR_COIN_REJECTSW			6					// ﾘｼﾞｪｸﾄ  Error
#define		ERR_COIN_DISPENCEFAIL		7					// 払出  Error
#define		ERR_COIN_10EMPTYSW			12					//  10 ｴﾝﾌﾟﾃｨSW Error
#define		ERR_COIN_50EMPTYSW			13					//  50 ｴﾝﾌﾟﾃｨSW Error
#define		ERR_COIN_100EMPTYSW			14					// 100 ｴﾝﾌﾟﾃｨSW Error
#define		ERR_COIN_500EMPTYSW			15					// 500 ｴﾝﾌﾟﾃｨSW Error
#define		ERR_COIN_10OVFSENSOR		18					//  10 ｵｰﾊﾞｰﾌﾛｰｾﾝｻｰ Error
#define		ERR_COIN_50VFSENSOR			19					//  50 ｵｰﾊﾞｰﾌﾛｰｾﾝｻｰ Error
#define		ERR_COIN_100VFSENSOR		20					// 100 ｵｰﾊﾞｰﾌﾛｰｾﾝｻｰ Error
#define		ERR_COIN_500VFSENSOR		21					// 500 ｵｰﾊﾞｰﾌﾛｰｾﾝｻｰ Error
#define		ERR_COIN_ACK4RECEIVE		51					// ACK4受信
#define		ERR_COIN_NAKRECEIVE			52					// NAK受信
#define		ERR_COIN_RECEIVEBUFFULL		55					// 受信ﾊﾞｯﾌｧﾌﾙ
#define		ERR_COIN_PARITYERROR		56					// ﾊﾟﾘﾃｨｴﾗｰ
#define		ERR_COIN_OVERRUNERROR		57					// ｵｰﾊﾞｰﾗﾝｴﾗｰ
#define		ERR_COIN_FLAMERROR			58					// ﾌﾚ-ﾐﾝｸﾞｴﾗｰ
#define		ERR_COIN_RECEIVESIZENG		59					// 受信文字数不一致
#define		ERR_COIN_RECEIVEBCCNG		60					// 受信ﾃﾞｰﾀBCCｴﾗｰ
#define		ERR_COIN_CASETTOUT			99					// Casett    OUT
#define		ERR_COIN_SAFE				30

// 紙幣ﾘｰﾀﾞｰ
#define		ERR_NOTE_COMFAIL			1					// 通信不良
#define		ERR_NOTE_JAM				5					// Note Jam  Error
#define		ERR_NOTE_SHIKIBETU			6					// Read Div  Error
#define		ERR_NOTE_STACKER			7					// Stacker   Error
#define		ERR_NOTE_REJECT				8					// Reject    Error
#define		ERR_NOTE_ACK4RECEIVE		51					// ACK4受信
#define		ERR_NOTE_NAKRECEIVE			52					// NAK受信
#define		ERR_NOTE_SAFE				30

// IF盤、ﾛｯｸ装置
#define		ERR_IFMASTERCOMFAIL			1					// IF盤親機通信不良
#define		ERR_IFSLAVECOMFAIL			20					// IF盤子機通信不良
#define		ERR_LOCKCLOSEFAIL			38					// ﾛｯｸ装置閉ﾛｯｸ
#define		ERR_LOCKOPENFAIL			39					// ﾛｯｸ装置開ﾛｯｸ
#define		ERR_LOCK_STATUSNG			52					// ﾛｯｸ装置状態が規定外
#define		ERR_LOCK_EVENTOUTOFRULE		53					// ｲﾍﾞﾝﾄ番号規定外
#define		ERR_FLAPIFMASTERCOMFAIL		1					// IF盤親機通信不良
#define		ERR_FLAPCLOSEFAIL			38					// ﾌﾗｯﾌﾟ装置閉ﾛｯｸ
#define		ERR_FLAPOPENFAIL			39					// ﾌﾗｯﾌﾟ装置開ﾛｯｸ
#define		ERR_FLAPDOWNRETRYOVER		40					// ｎ分ｍ回ﾘﾄﾗｲｵｰﾊﾞｰ
#define		ERR_FLAPDOWNRETRY			41					// ｎ分ｍ回ﾘﾄﾗｲ動作
#define		ERR_FLAP_NOTPROCESSED		50					// IBC前回ﾃﾞｰﾀ未処理
#define		ERR_FLAP_STATUSNG			52					// ﾌﾗｯﾌﾟ装置状態が規定外
#define		ERR_FLAP_EVENTOUTOFRULE		53					// ｲﾍﾞﾝﾄ番号規定外
#define		ERR_FLAP_AUTO_UPRETRYOVER	45					// フラップ上昇モーター制御リトライオーバー
#define		ERR_FLAP_AUTO_DOWNRETRYOVER	46					// フラップ下降モーター制御リトライオーバー

// Mifare
#define		ERR_MIF_WRT_VERIFY			10					// Write Verify NG
#define		ERR_MIF_WRT_NG				11					// Write NG(Verify以外)

// NT-NET 子機	(66)
// 順番入れ替え、コメント追記
#define		ERR_NTNET_PRIOR_RCVBUF		6					// 優先データ受信バッファフルによるデータ削除発生
#define		ERR_NTNET_NORMAL_RCVBUF		7					// 通常データ受信バッファフルによるデータ削除発生
#define		ERR_NTNET_BROADCAST_RCVBUF	8					// 同報データ受信バッファフルによるデータ削除発生
#define		ERR_NTNET_FREE_RCVBUF		9					// FREEデータ受信バッファフルによるデータ削除発生
#define		ERR_NTNET_OVERWRITE_SENDBUF	12					// 送信バッファ上書き
#define		ERR_NTNET_DISCARD_SENDDATA	18					// ブロック番号異常でデータ破棄
#define		ERR_NTNET_NTBLKNO_VALID		19					// ブロック番号異常(今回データ有効)
#define		ERR_NTNET_NTBLKNO_INVALID	20					// ブロック番号異常(今回データ無効)
#define		ERR_NTNET_ID22_BUFNFULL		24					// 精算データ送信バッファニアフル
#define		ERR_NTNET_ID22_BUFFULL		25					// 精算データ送信バッファフル
#define		ERR_NTNET_ID20_BUFFULL		26					// 入庫データ送信バッファフル
#define		ERR_NTNET_ID21_BUFFULL		27					// 出庫データ送信バッファフル			※現在処理なし
#define		ERR_NTNET_TTOTAL_BUFFULL	28					// T合計データ送信バッファフル			※現在処理なし
#define		ERR_NTNET_ID22_SENDBUF		30					// 精算データ送信バッファ上書き
#define		ERR_NTNET_ID20_SENDBUF		31					// 入庫データ送信バッファ上書き
#define		ERR_NTNET_ID21_SENDBUF		32					// 出庫データ送信バッファ上書き			※現在処理なし
#define		ERR_NTNET_TTOTAL_SENDBUF	33					// T合計データ送信バッファ上書き		※現在処理なし
#define		ERR_NTNET_ARCBLKNO_INVALID	48					// 途中ブロック異常
#define		ERR_NTNET_RECV_BLK_OVER		50					// Arcnet受信ブロックオーバー
#define		ERR_NTNET_BUFFULL_RETRY		60					// バッファFULLによる再送回数オーバー
#define		ERR_NTNETDOPA_SAVE_SENDBUF_FULL			85		// 保持送信データバッファフルによるデータ削除発生

#define		ERR_NTNETDOPA_SAVE_SENDBUF_NFULL		30		// 保持送信データバッファニアフル					＊未使用(番号は仮値)
#define		ERR_NTNETDOPA_NOT_SAVE_SENDBUF_FULL		31		// 非保持送信データバッファフルによるデータ削除発生	＊未使用(番号は仮値)
#define		ERR_NTNETDOPA_NOT_SAVE_SENDBUF_NFULL	32		// 非保持送信データバッファニアフル					＊未使用(番号は仮値)
#define		ERR_NTNETDOPA_RECVBUF_FULL				33		// 受信データバッファフルによるデータ削除発生		＊未使用(番号は仮値)
#define		ERR_NTNETDOPA_RECVBUF_NFULL				34		// 受信データバッファニアフル						＊未使用(番号は仮値)
#define		ERR_NTNETDOPA_PARITY_ERR		40				// パリティエラー
#define		ERR_NTNETDOPA_FRAMING_ERR		41				// フレーミングエラー
#define		ERR_NTNETDOPA_OVERRUN_ERR		42				// オーバーランエラー

#define		ERR_NTNETDOPA_DISCARD_SENDDATA	86				// ブロック番号異常でデータ破棄
#define		ERR_NTNETDOPA_NTBLKNO_VALID		87				// ブロック番号異常(今回データ有効)
#define		ERR_NTNETDOPA_ARCBLKNO_INVALID	88				// 途中ブロック異常
#define		ERR_NTNETDOPA_BUFFULL_RETRY		89				// バッファFULLによる再送回数オーバー

#define		ERR_APSNET_ARCBLKNO_INVALID_RCV	48				// 6348：APS受信電文 ブロック番号異常 					（1shot）
#define		ERR_APSNET_COMMFAIL				1				// 6301：前回送信データ処理未完了 再送ｵｰﾊﾞｰ

// ﾗﾍﾞﾙﾌﾟﾘﾝﾀ()
#define		ERR_LPRN_COMFAIL			1					// 通信タイムアウト
#define		ERR_LPRN_CUTTER				10					// カッターエラー
#define		ERR_LPRN_HEADOPEN			11					// ヘッドオープン
#define		ERR_LPRN_HEAD				12					// ヘッドエラー
#define		ERR_LPRN_SENSOR				13					// センサエラー
#define		ERR_LPRN_BUFF_FULL			14					// バッファオーバー
#define		ERR_LPRN_MEDIA				15					// メディアエラー
#define		ERR_LPRN_OTHER				16					// その他のエラー
#define		ERR_LPRN_OFFLINE			20					// 通信オフライン

#define		ERR_SUICA_COMFAIL			1					// 通信不良
#define		ERR_SUICA_RECEIVE			2					// 異常ﾃﾞｰﾀ受信
#define		ERR_SUICA_OPEN				4					// 開局処理異常
#define		ERR_SUICA_MIRYO				5					// 未了発生後にﾀｲﾑｱｳﾄ
#define		ERR_SUICA_PAY_NG			6					// Suica決算NG

#define		ERR_SUICA_ACK4RECEIVE		51					// ACK4受信
#define		ERR_SUICA_NAKRECEIVE		52					// NAK受信

#define		ERR_SUICA_REINPUT			62					// 入力再要求送信
#define		ERR_SUICA_REOUTPUT			63					// 出力再指令送信
#define		ERR_SUICA_MIRYO_START		64					// 未了発生
#define		ERR_SUICA_PAY_ERR			65					// 電子マネーリーダー決済不良

// MH321800(S) G.So ICクレジット対応
// 決済リーダ
#define		ERR_EC_COMFAIL				ERR_SUICA_COMFAIL	// [01]通信不良
#define		ERR_EC_ERR_HARD				2					// [02]リーダハードウェア異常(異常データ受信時)
#define		ERR_EC_ERR_DRIVER			3					// [03]JVMAドライバ異常(異常データ受信時)
#define		ERR_EC_ERR_SYSTEM			4					// [04]ソフトウェア異常(異常データ受信時)
#define		ERR_EC_ERR_ENCRYPTION		5					// [05]暗号化モード設定失敗(異常データ受信時)
#define		ERR_EC_ERR_CONNECTION		6					// [06]リーダ接続異常(異常データ受信時)
#define		ERR_EC_OPEN					9					// [09]リーダNotReady
#define		ERR_EC_UNINITIALIZED		10					// [10]決済リーダ起動未完了
// MH810103 GG119202(S) 異常データフォーマット変更
#define		ERR_EC_ERR_LTE_FAIL			11					// [11]決済リーダ起動時LTE接続失敗
#define		ERR_EC_ERR_SETTING_FAIL		12					// [12]決済リーダ起動時設定異常
// MH810103 GG119202(E) 異常データフォーマット変更
// MH810103 GG119202(S) 決済リーダからの再起動要求を記録する
#define		ERR_EC_REBOOT				13					// [13]決済リーダ再起動
// MH810103 GG119202(E) 決済リーダからの再起動要求を記録する
#define		ERR_EC_RCV_SETT_ERR_100		20					// [20]決済リーダ系詳細エラーコード100受信(ハードウェア異常)
#define		ERR_EC_RCV_SETT_ERR_101		21					// [21]決済リーダ系詳細エラーコード101受信(JVMAドライバ異常)
#define		ERR_EC_RCV_SETT_ERR_200		22					// [22]決済リーダ系詳細エラーコード200受信(決済リーダH/W異常)
#define		ERR_EC_RCV_SETT_ERR_201		23					// [23]決済リーダ系詳細エラーコード201受信(選択商品データフォーマット不整合)
#define		ERR_EC_RCV_SETT_ERR_300		24					// [24]決済リーダ系詳細エラーコード300受信(ソウトウェア異常)
#define		ERR_EC_RCV_SETT_ERR_301		25					// [25]決済リーダ系詳細エラーコード301受信(センター通信異常)
#define		ERR_EC_RCV_SETT_ERR_304		26					// [26]決済リーダ系詳細エラーコード304受信(リーダ接続失敗)
#define		ERR_EC_RCV_SETT_ERR_305		27					// [27]決済リーダ系詳細エラーコード305受信(暗号化モード設定失敗)
#define		ERR_EC_RCV_SETT_ERR_306		28					// [28]決済リーダ系詳細エラーコード306受信(カード読み取り失敗)
#define		ERR_EC_RCV_SETT_ERR_307		29					// [29]決済リーダ系詳細エラーコード307受信(ICカード読み取り失敗)
#define		ERR_EC_RCV_SETT_ERR_308		30					// [30]決済リーダ系詳細エラーコード308受信(カード読み取りタイムアウト)
#define		ERR_EC_RCV_SETT_ERR_309		31					// [31]決済リーダ系詳細エラーコード309受信(カード取引拒否)
#define		ERR_EC_RCV_SETT_ERR_311		33					// [33]決済リーダ系詳細エラーコード311受信(決済完了前カード抜き去り)
// MH810103 GG119202(S) カード処理中タイムアウトでみなし決済扱いとする
#define		ERR_EC_RCV_SETT_FAIL		34					// [34]決済結果受信失敗
// MH810103 GG119202(E) カード処理中タイムアウトでみなし決済扱いとする
// MH810105(S) MH364301 みなし決済データ受信エラー追加
#define		ERR_EC_RCV_DEEMED_SETTLEMENT	35				// [35]みなし決済データ受信
// MH810105(E) MH364301 みなし決済データ受信エラー追加
#define		ERR_EC_RCV_SETT_ERR_DES		40					// [40]センター系詳細エラーコードDES受信
#define		ERR_EC_RCV_SETT_ERR_CKP		41					// [41]センター系詳細エラーコードCKP受信
#define		ERR_EC_RCV_SETT_ERR_G		42					// [42]センター系詳細エラーコードG受信
#define		ERR_EC_RCV_SETT_ERR_UNREG	43					// [43]未登録詳細エラーコード受信
// MH810103 GG119202(S) 電子マネー系詳細エラーコード追加
#define		ERR_EC_RCV_EMONEY_CRITICAL_ERR	44				// [44]センター・電子マネー系重大詳細エラーコード受信
#define		ERR_EC_RCV_EMONEY_ERR		45					// [45]センター・電子マネー系詳細エラーコード受信
// MH810103 GG119202(E) 電子マネー系詳細エラーコード追加	
// MH810103 GG119202(S) 決済停止エラーの登録
#define		ERR_EC_PAY_STOP				46					// [46]決済停止
// MH810103 GG119202(E) 決済停止エラーの登録
// MH810103 GG119202(S) みなし決済扱い時の動作
#define		ERR_EC_SETTLE_ABORT			47					// [47]決済処理失敗
// MH810103 GG119202(E) みなし決済扱い時の動作
// MH810103 GG119202(S) JVMAリセットをエラー登録する
#define		ERR_EC_JVMA_RESET			48					// [48]JVMAリセット
// MH810103 GG119202(E) JVMAリセットをエラー登録する
// MH321800(S) フェーズエラー発生時に決済OK以外はリーダー切り離ししない
// MH810105(S) MH364301 E3249を発生／解除別に変更
//#define		ERR_EC_NOTOK_SETTLE_ERR		49					// [49]決済結果受信エラー
#define		ERR_EC_NOTOK_SETTLE_ERR		49					// [49]決済結果受信エラー１
// MH810105(E) MH364301 E3249を発生／解除別に変更
// MH321800(E) フェーズエラー発生時に決済OK以外はリーダー切り離ししない
// MH321800(S) みなし決済発生時にエラー登録する
#define		ERR_EC_DEEMED_SETTLEMENT	50					// [50]みなし決済
// MH321800(E) みなし決済発生時にエラー登録する
#define		ERR_EC_ACK4RECEIVE			51					// [51]ACK4受信
#define		ERR_EC_NAKRECEIVE			52					// [52]NAK受信
#define		ERR_EC_BUF_FULL				53					// [53]受信バッファフル
#define		ERR_EC_PARITY				54					// [54]受信データパリティエラー
#define		ERR_EC_OVERRUN				55					// [55]受信データオーバーランエラー
#define		ERR_EC_FLAMING				56					// [56]受信データフレーミングエラー
#define		ERR_EC_DATANUM				57					// [57]受信文字数不一致エラー
#define		ERR_EC_DATABCC				58					// [58]受信データBCCチェックエラー
// MH810105(S) MH364301 E3249を発生／解除別に変更
//#define		ERR_EC_SETTLE_ERR			59					// [59]決済異常発生
#define		ERR_EC_SETTLE_ERR			59					// [59]決済結果受信エラー２
// MH810105(E) MH364301 E3249を発生／解除別に変更
#define		ERR_EC_REINPUT				60					// [60]入力再要求送信
#define		ERR_EC_REOUTPUT				61					// [61]出力再指令送信
#define		ERR_EC_PAY_ERR				62					// [62]決済リーダ決済不良
#define		ERR_EC_PAYBRAND_ERR			63					// [63]未設定ブランド取引データ受信
// MH810103 GG119202(S) 使用マネー選択設定(50-0001,2)を参照しない
//#define		ERR_EC_SETTEI_BRAND_NG		64					// [64]決済リーダ設定不良（使用マネー選択）
// MH810103 GG119202(E) 使用マネー選択設定(50-0001,2)を参照しない
#define		ERR_EC_BRANDNO_ZERO			65					// [65]ブランド番号0データ受信
#define		ERR_EC_ALARM_TRADE			66					// [66]引き去り処理未了タイムアウト発生(全種カード共通)
#define		ERR_EC_ALARM_TRADE_WAON		67					// [67]引き去り処理未了タイムアウト発生(WAON)
#define		ERR_EC_SND_QUE_GAP			68					// [68]送信キューの参照・書込みインデックスずれ
#define		ERR_EC_RCV_IGL_STS_DATA		69					// [69]想定外のMVT状態データ受信
#define		ERR_EC_KOUTU_ABNORMAL		70					// [70]ブランド異常(交通系電子マネー)
#define		ERR_EC_EDY_ABNORAML			71					// [71]ブランド異常(Edy)
#define		ERR_EC_WAON_ABNORMAL		72					// [72]ブランド異常(WAON)
#define		ERR_EC_NANACO_ABNORMAL		73					// [73]ブランド異常(nanaco)
#define		ERR_EC_ID_ABNORMAL			74					// [74]ブランド異常(iD)
#define		ERR_EC_QUICPAY_ABNORMAL		75					// [75]ブランド異常(QuicPay)
#define		ERR_EC_CREDIT_ABNORMAL		76					// [76]ブランド異常(クレジット)
// MH321800(E) G.So ICクレジット対応
// MH810105(S) MH364301 PiTaPa対応
#define		ERR_EC_PITAPA_ABNORMAL		77					// [77]ブランド異常(PiTaPa)
// MH810105(E) MH364301 PiTaPa対応
// MH810105(S) MH364301 QRコード決済対応
#define		ERR_EC_QR_ABNORMAL			78					// [78]ブランド異常(QRコード)
#define		ERR_EC_SUB_BRAND_FAILURE	79					// [79]サブブランド異常
#define		ERR_EC_PROCESS_FAILURE		80					// [80]決済処理失敗
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
#define		ERR_EC_HOUSE_ABNORAML		0					// エラーになることがない
#define		ERR_EC_TCARD_ABNORAML		0					// エラーになることがない
#define		ERR_EC_HOUJIN_ABNORAML		0					// エラーになることがない
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）

#define		ERR_SUBCPU_CPUERR			1					// 8001:通信CPU異常
#define		ERR_SUBCPU_HIFSENDERR		2					// 8002:HIFデータ送信エラー
#define		ERR_SUBCPU_HIFRECVDELAY		3					// 8003:HIFデータ遅延受信
#define		ERR_SUBCPU_LOADERR			4					// 8004:ローダーロードエラー
#define		ERR_SUBCPU_APPENVERR		5					// 8005:アプリ環境ロードエラー
#define		ERR_SUBCPU_APPERR			6					// 8006:アプリケーションロードエラー
#define		ERR_SUBCPU_CHECKSUMERR		7					// 8007:チェックサムエラー
#define		ERR_SUBCPU_RTRYERR			8					// 8008:ロードリトライエラー
#define		ERR_SUBCPU_BKMEMERR			9					// 8009:バックアップメモリ異常
#define		ERR_SUBCPU_FTP_COM_TIMEOUT	15					// 8015:FTP通信タイムアウト時間
#define		ERR_SUBCPU_NTPNOANS			20					// 8020:NTPサーバー通信不良
#define		ERR_SUBCPU_SNTPTIMOUT		21					// 8021:SNTPリクエストタイムアウト
#define		ERR_SUBCPU_FTPSVRFAIL		30					// 8030:FTPサーバー接続失敗
#define		ERR_CARD_T					2					// Ｔカードエラー
#define		ERR_CARD_H					3					// 法人カードエラー
#define		ERR_FLASH_WRITE				4					// フラッシュ書き込みエラー
#define		ERR_TCARD_CHK_NG			5					// Tカード判定NG
#define		ERR_CAN_COMFAIL				1					// E8601:CAN 通信不良
#define		ERR_CAN_STUFF_ERR			5					// E8605:CAN スタッフエラー
#define		ERR_CAN_FORM_ERR			6					// E8606:CAN フォームエラー
#define		ERR_CAN_CRC_ERR				7					// E8607:CAN CRCエラー
#define		ERR_CAN_LOST_ERR			8					// E8608:CAN オーバーロード（メッセージロスト）発生エラー
#define		ERR_CAN_NOANSWER_02			12					// E8612:CAN ノード2応答なし

#define		ERR_FLAPLOCK_COMFAIL		1					// 送信リトライオーバー（通信異常）
#define		ERR_FLAPLOCK_LOCKCLOSEFAIL	38					// ロック装置閉／フラップ板上昇動作でロック
#define		ERR_FLAPLOCK_LOCKOPENFAIL	39					// ロック装置開／フラップ板下降動作でロック
#define		ERR_FLAPLOCK_DOWNRETRYOVER	40					// フラップ板下降動作でロック時の保護処理リトライオーバー
#define		ERR_FLAPLOCK_DOWNRETRY		41					// フラップ板下降動作でロック時の保護処理開始
#define		ERR_FLAPLOCK_STATENG		52					// フラップ／ロック状態規定外
#define		ERR_FLAPLOCK_EVENTOUTOFRULE	53					// フラップ／ロックイベント規定外
#define		ERR_FLAPLOCK_PARITYERROR	60					// パリティエラー
#define		ERR_FLAPLOCK_FLAMERROR		61					// フレーミングエラー
#define		ERR_FLAPLOCK_OVERRUNERROR	62					// オーバーランエラー
#define		ERR_FLAPLOCK_HEADERERROR	63					// 電文異常（ヘッダ文字）
#define		ERR_FLAPLOCK_LENGTHERROR	64					// 電文異常（データ長NG）
#define		ERR_FLAPLOCK_IDERROR		65					// 電文異常（データIDNG）
#define		ERR_FLAPLOCK_NAKRYOVER		66					// NAKリトライオーバー
#define		ERR_FLAPLOCK_CRC_ERR		67					// CRC異常
#define		ERR_FLAPLOCK_NOHEADER		68					// 電文異常（ヘッダ部未受信）
#define		ERR_FLAPLOCK_PLOTOCOLERR	69					// プロトコル異常
#define		ERR_FLAPLOCK_RESERR			70					// 相手違い
#define		ERR_FLAPLOCK_QUEFULL1		71					// キューフル（子機状態リスト）
#define		ERR_FLAPLOCK_QUEFULL2		72					// キューフル（ロック動作待ちリスト）
#define		ERR_FLAPLOCK_QUEFULL3		73					// キューフル（子機セレクティング待ちリスト）
#define		ERR_FLAPLOCK_DATAERR		74					// 制御データ異常
#define		ERR_FLAPLOCK_TYPEDISCODE	75					// 装置種別不一致

// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
#define		ERR_CC_PAYDATE_INVALID			21				// 精算日時無効
#define		ERR_CC_ENTRYDATE_INVALID		22				// 入庫日時無効（初回）
#define		ERR_CC_ENTRYDATE_INVALID_OLD	23				// 入庫日時無効（再精算）
#define		ERR_CC_PAYDATE_INVALID_OLD		24				// 前回精算日時無効（再精算）
#define		ERR_CC_OVER1YEARCAL				41				// 1年以上料金計算（入庫）
#define		ERR_CC_OVER1YEARCAL_OLD			42				// 1年以上料金計算（前回精算）
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応

#define		ERR_SODIAC_STOP_ERR			5					//異常終了
#define		ERR_SODIAC_DATA_ERR			6					//データ異常
#define		ERR_SODIAC_MEM_ERR			7					//メモリ異常
#define		ERR_SODIAC_PRM_ERR			8					//パラメータ異常
#define		ERR_SODIAC_STS_ERR			9					//状態異常
#define		ERR_SODIAC_NOTSP_ERR		10					//未サポート異常
#define		ERR_SODIAC_OTHER_ERR		11					//その他異常
#define		ERR_SODIAC_RETRY_ERR		15					//送信リトライオーバー				
//Sodiac notice id
#define		ERR_SOIDAC_STOP_ID			0					//再生停止
#define		ERR_SOIDAC_NEXT_ID			1					//次回フレーズ要求
#define		ERR_SOIDAC_ERROR_ID			2					// エラー
#define		ERR_SOIDAC_PWMON_ID 		3					// PWM 出力開始
#define		ERR_SOIDAC_PWMOFF_ID 		4					// PWM 出力停止
#define		ERR_SOIDAC_MUTEOFF_ID 		5					// ミュート解除
#define		ERR_SOIDAC_MUTEON_ID 		6					// ミュート開始
#define		ERR_SOIDAC_NOT_ID 			10					// 通知なし
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(LCD_IF対応)
#define		ERR_TKLSLCD_COMM_FAIL		1					// 精算機－LCD間通信不良(発生：コネクション切断／解除：コネクション接続)
#define		ERR_TKLSLCD_RMT_RST			2					// 相手からのリセット(発生：TM_CB_RESET検出／解除：コネクション接続)
#define		ERR_TKLSLCD_RMT_DSC			3					// 相手からの切断(発生：TM_CB_REMOTE_CLOSE検出／解除：コネクション接続)
#define		ERR_TKLSLCD_ERR_OCC			4					// エラー発生(発生：TM_CB_SOCKET_ERROR検出／解除：コネクション接続)
#define		ERR_TKLSLCD_CON_RES_TO		5					// コネクション応答待ちタイムアウト(発生：コネクション応答待ちタイムアウト検出／解除：コネクション接続)
#define		ERR_TKLSLCD_KPALV_RTY_OV	6					// キープアライブリトライオーバー(発生：キープアライブリトライオーバー発生／解除：コネクション接続)
#define		ERR_TKLSLCD_KSG_SOC			10					// kasagoエラー(socket())(発生／解除同時)
#define		ERR_TKLSLCD_KSG_BLK_ST		11					// kasagoエラー(tfBlockingState())(発生／解除同時)
#define		ERR_TKLSLCD_KSG_SOC_OP		12					// kasagoエラー(setsockopt())(発生／解除同時)
#define		ERR_TKLSLCD_KSG_SOC_CB		13					// kasagoエラー(tfRegisterSocketCB())(発生／解除同時)
#define		ERR_TKLSLCD_KSG_BND			14					// kasagoエラー(bind())(発生／解除同時)
#define		ERR_TKLSLCD_KSG_CON			15					// kasagoエラー(Connect())(発生／解除同時)
#define		ERR_TKLSLCD_KSG_SND			16					// kasagoエラー(送信エラー(send())(発生／解除同時)
#define		ERR_TKLSLCD_KSG_RCV			17					// kasagoエラー(受信エラー(recv())(発生／解除同時)
#define		ERR_TKLSLCD_KSG_CLS			18					// kasagoエラー(tfClose()）(発生／解除同時)
#define		ERR_TKLSLCD_SND_FORCED_ACK	30					// 強制ACK送信(受信データ破棄(発生／解除同時)
#define		ERR_TKLSLCD_SND_RTRY_OV		31					// データ再送リトライオーバー(発生／解除同時)
#define		ERR_TKLSLCD_SND_BUF_RPT		32					// 送信バッファリードポインタ異常(発生／解除同時)
#define		ERR_TKLSLCD_SND_BUF_WPT		33					// 送信バッファライトポインタ異常(発生／解除同時)
#define		ERR_TKLSLCD_SND_BUF_FUL		34					// 送信バッファフル(発生／解除同時)
#define		ERR_TKLSLCD_PAY_RES_TO		40					// 精算応答データ待ちタイムアウト(発生／解除同時)
#define		ERR_TKLSLCD_QR_RES_TO		41					// QR確定・取消応答データ待ちタイムアウト(発生／解除同時)
#define		ERR_TKLSLCD_RCV_FORCED_ACK	50					// 強制ACK受信(送信データ破棄(発生／解除同時)
#define		ERR_TKLSLCD_RCV_HDR			51					// 受信データヘッダエラー(発生／解除同時)
#define		ERR_TKLSLCD_RCV_CRC			52					// 受信データCRCエラー(発生／解除同時)
#define		ERR_TKLSLCD_RCV_LENGTH		53					// 受信データ長エラー(発生／解除同時)
#define		ERR_TKLSLCD_RCV_SEQ_NO		54					// 受信データシーケンス№エラー(発生／解除同時)
#define		ERR_TKLSLCD_RCV_DCRYPT_FL	55					// 受信データ復号化エラー(発生／解除同時)
#define		ERR_TKLSLCD_PRM_UPLD_FL		70					// パラメータアップロード失敗

// DC-NET通信(28)
#define		ERR_BATCH_LAN_UP_HOST_COMM_ERROR		1		// 【上り回線】HOST側通信エラー
#define		ERR_BATCH_LAN_UP_REMOTE_CRC16_ERROR		10		// 【上り回線】受信データ異常
#define		ERR_BATCH_LAN_UP_SEND_RETRY_OVER		11		// 【上り回線】パケット送信リトライオーバー
#define		ERR_BATCH_LAN_UP_NTNET_BLOCK_ERROR		13		// 【上り回線】ＮＴ－ＮＥＴブロック番号異常

// バーコード(30)
#define		ERR_BARCODE_COMM_FAIL		5					// バーコードスキャナ通信不良
#define		ERR_BARCODE_PARAM_FAIL		10					// パラメータ設定失敗
#define		ERR_BARCODE_OVER_RUN		15					// シリアル通信：オーバーランエラー(発生／解除同時)
#define		ERR_BARCODE_FLAMING_ERR		16					// シリアル通信：フレーミングエラー(発生／解除同時)
#define		ERR_BARCODE_PARITY_ERR		17					// シリアル通信：パリティエラー(発生／解除同時)
#define		ERR_BARCODE_BUFF_OVER_FLOW	18					// シリアル通信：バッファオーバーフロー(発生／解除同時)

// リアルタイム通信(37)
#define		ERR_RELT_UP_COM_FAIL				1			//【上り回線】HOST側通信エラー
#define		ERR_RELT_UP_RECV_NACK99				3			//【上り回線】データ削除応答受信
#define		ERR_RELT_UP_NACK10_RETRY_OVER		4			//【上り回線】データ削除応答送信（NACK10リトライオーバー）
#define		ERR_RELT_UP_NACK12_RETRY_OVER		5			//【上り回線】データ削除応答送信（NACK12リトライオーバー）
#define		ERR_RELT_UP_NACK13_RETRY_OVER		6			//【上り回線】データ削除応答送信（NACK13リトライオーバー）
#define		ERR_RELT_UP_SEND_RETRY_OVER			14			//【上り回線】パケット送信リトライオーバー
#define		ERR_RELT_DOWN_NO_ACT_TIMEOUT		51			//【下り回線】受信監視タイムアウト
#define		ERR_RELT_DOWN_RECV_NACK99			52			//【下り回線】データ削除応答受信
#define		ERR_RELT_DOWN_NACK10_RETRY_OVER		53			//【下り回線】データ削除応答送信（NACK10リトライオーバー）
#define		ERR_RELT_DOWN_NACK12_RETRY_OVER		54			//【下り回線】データ削除応答送信（NACK12リトライオーバー）
#define		ERR_RELT_DOWN_NACK13_RETRY_OVER		55			//【下り回線】データ削除応答送信（NACK13リトライオーバー）
#define		ERR_RELT_DOWN_SEND_RETRY_OVER		58			//【下り回線】パケット送信リトライオーバー

// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(LCD_IF対応)
#define		ERR_I2C_R_W_MODE			4		// Read/Writeモードエラー
#define		ERR_I2C_CLOCK_LEVEL			5		// クロックレベルエラー
#define		ERR_I2C_SEND_DATA_LEVEL		6		// 送信データレベルエラー
#define		ERR_I2C_BUS_BUSY			7		// バスBUSY
#define		ERR_I2C_PREV_TOUT			8		// 前回ステータス変化待ちタイムアウト
//-----------------------------------------------
#define		ERR_I2C_BFULL_SEND_RP		10		// レシートプリンタ送信バッファフル
#define		ERR_I2C_BFULL_RECV_RP		11		// レシートプリンタ受信バッファフル
#define		ERR_I2C_BFULL_SEND_JP		12		// ジャーナルプリンタ送信バッファフル
#define		ERR_I2C_BFULL_RECV_JP		13		// ジャーナルプリンタ受信バッファフル
#define		ERR_I2C_BFULL_SEND_CAPPI	14		// 予約エリア（CAPPI送信バッファフル
#define		ERR_I2C_BFULL_RECV_CAPPI	15		// 予約エリア（CAPPI受信バッファフル
#define		ERR_I2C_BFULL_SEND_PIP		16		// 予約エリア（PIP送信バッファフル）
#define		ERR_I2C_BFULL_RECV_PIP		17		// 予約エリア（PIP受信バッファフル）

#define		ERR_CAPPI_PARITY_ERR		50		// パリティエラー
#define		ERR_CAPPI_FRAMING_ERR		51		// フレーミングエラー
#define		ERR_CAPPI_OVERRUN_ERR		52		// オーバーランエラー

#define		ERR_REMOTE_SCI_PARITY		60		// シリアルパリティエラー
#define		ERR_REMOTE_SCI_FRAME		61		// シリアルフレーミングエラー
#define		ERR_REMOTE_SCI_OVERRUN		62		// シリアルオーバーランエラー

//RAU
#define		ERR_RAU_HOST_COMMUNICATION		1				// エラーコード０１(上り回線通信エラー)
#define		ERR_RAU_RCV_SEQ_TIM_OVER		10				// エラーコード１０(受信シーケンスフラグレスポンスタイムアウト)
#define		ERR_RAU_REQ_COUNT_TIMEOUT		24				// 現在駐車台数要求タイムアウト(HOSTへの情報送信に使用)
#define		ERR_RAU_FMA_ANTENALOW1			31				// (31)接続時のｱﾝﾃﾅﾚﾍﾞﾙが 1
#define		ERR_RAU_FMA_ANTENALOW2			32				// (32)接続時のｱﾝﾃﾅﾚﾍﾞﾙが 2
#define		ERR_RAU_DPA_RECV_CRC			54				// エラーコード５４(上り回線受信データＣＲＣエラー)
#define		ERR_RAU_DPA_RECV_LONGER			55				// エラーコード５５(上り回線受信データ長異常)
#define		ERR_RAU_DPA_RECV_SHORTER		56				// エラーコード５６(上り回線受信データ長異常)
#define		ERR_RAU_TCP_CONNECT				91				// エラーコード９１(上り回線ＴＣＰコネクションタイムアウト)
#define		ERR_RAU_DPA_SEND_LEN			92				// エラーコード９２(上り回線送信データ長オーバー)
#define		ERR_RAU_DPA_RECV_LEN			93				// エラーコード９３(上り回線受信データ長オーバー)
#define		ERR_RAU_DPA_CALL_RETRY_OVER		94				// エラーコード９４(再発呼回数オーバー)
#define		ERR_RAU_DPA_SEND_RETRY_OVER		95				// エラーコード９５(上り回線データ再送回数オーバー)
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(ログ登録)
#define		ERR_RAU_LONGPARK_BUFERFULL		57				// 長期駐車データバッファフル
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(ログ登録)
#define		ERR_RAU_TURI_BUFERFULL			58				// 釣銭管理集計データバッファフル
#define		ERR_RAU_GTTOTAL_BUFERFULL		59				// GT合計データバッファフル
#define		ERR_RAU_ENTRY_BUFERFULL			60				// 入庫データバッファフル
#define		ERR_RAU_PAYMENT_BUFERFULL		63				// 精算（出口）データバッファフル
#define		ERR_RAU_TOTAL_BUFERFULL			64				// T合計データバッファフル
#define		ERR_RAU_ERROR_BUFERFULL			65				// エラーデータバッファフル
#define		ERR_RAU_ALARM_BUFERFULL			66				// アラームデータバッファフル
#define		ERR_RAU_MONITOR_BUFERFULL		67				// モニタデータバッファフル
#define		ERR_RAU_OPE_MONITOR_BUFERFULL	68				// 操作モニタデータバッファフル
#define		ERR_RAU_COIN_BUFERFULL			69				// コイン金庫庫データバッファフル
#define		ERR_RAU_NOTE_BUFERFULL			70				// 紙幣金庫データバッファフル
#define		ERR_RAU_PARK_CNT_BUFERFULL		71				// 駐車台数データバッファフル
#define		ERR_RAU_RMON_BUFERFULL			72				// 遠隔監視データバッファフル
#define		ERR_RAU_FMA_MODEMERR_DR			74				// (74)DR信号が 3秒経過してもONにならない(MODEM ON時) //
#define		ERR_RAU_FMA_MODEMERR_CD			75				// (75)CD信号が60秒経過してもOFFにならない(MODEM OFF時) //
#define		ERR_RAU_FMA_PPP_TIMEOUT 		76				// (76)PPPコネクションタイムアウト //
#define		ERR_RAU_FMA_RESTRICTION			77				// (77)RESTRICTION(規制中)を受信 //
#define		ERR_RAU_FMA_MODEMPOWOFF			78				// (78)MODEM 電源OFF //
#define		ERR_RAU_FMA_ANTENALOW			79				// (79)接続時のｱﾝﾃﾅﾚﾍﾞﾙが 0 //
#define		ERR_RAU_DPA_RECV_NAK99			97				// エラーコード97(上り回線送信データ強制停止受信)
// MH322914 (s) kasiyama 2016/07/12 遠隔NT-NETLAN接続時のエラーコードをFOMA接続時のエラーコードに合わせる[共通バグNo.1200](MH341106)
//#define		ERR_RAU_DPA_BLK_VALID_D			151				// エラーコード151(下り回線受信データブロック番号異常[今回有効])
//#define		ERR_RAU_DPA_BLK_INVALID_D		152				// エラーコード152(下り回線受信データブロック番号異常[今回無効])
//#define		ERR_RAU_DPA_RECV_CRC_D			154				// エラーコード154(下り回線受信データＣＲＣエラー)
//#define		ERR_RAU_DPA_RECV_LONGER_D		155				// エラーコード155(下り回線受信データ長異常)
//#define		ERR_RAU_DPA_RECV_SHORTER_D		156				// エラーコード156(下り回線受信データ長異常)
//#define		ERR_RAU_DPA_SEND_LEN_D			192				// エラーコード192(下り回線送信データ長オーバー)
//#define		ERR_RAU_DPA_RECV_LEN_D			193				// エラーコード193(下り回線受信データ長オーバー)
//#define		ERR_RAU_DPA_SEND_RETRY_OVER_D	195				// エラーコード195(下り回線データ再送回数オーバー)
//#define		ERR_RAU_DPA_RECV_NAK99_D		197				// エラーコード197(下り回線送信データ強制停止受信)
//#define		ERR_RAU_FMA_BLK_VALID_D			80				// FOMAエラーコード８０(下り回線受信データブロック番号異常[今回有効])
//#define		ERR_RAU_FMA_BLK_INVALID_D		81				// FOMAエラーコード８１(下り回線受信データブロック番号異常[今回無効])
//#define		ERR_RAU_FMA_RECV_CRC_D			82				// FOMAエラーコード８２(下り回線受信データＣＲＣエラー)
//#define		ERR_RAU_FMA_RECV_LONGER_D		83				// FOMAエラーコード８３(下り回線受信データ長異常)
//#define		ERR_RAU_FMA_RECV_SHORTER_D		84				// FOMAエラーコード８４(下り回線受信データ長異常)
//#define		ERR_RAU_FMA_SEND_LEN_D			85				// FOMAエラーコード８５(下り回線送信データ長オーバー)
//#define		ERR_RAU_FMA_RECV_LEN_D			86				// FOMAエラーコード８６(下り回線受信データ長オーバー)
//#define		ERR_RAU_FMA_SEND_RETRY_OVER_D	87				// FOMAエラーコード８７(下り回線データ再送回数オーバー)
//#define		ERR_RAU_FMA_RECV_NAK99_D		88				// FOMAエラーコード８８(下り回線送信データ強制停止受信)
//#define		ERR_RAU_FMA_RECV_NAK99			97				// FOMAエラーコード９７(上り回線送信データ強制停止受信)
#define		ERR_RAU_DPA_BLK_VALID_D			80				// エラーコード８０(下り回線受信データブロック番号異常[今回有効])
#define		ERR_RAU_DPA_BLK_INVALID_D		81				// エラーコード８１(下り回線受信データブロック番号異常[今回無効])
#define		ERR_RAU_DPA_RECV_CRC_D			82				// エラーコード８２(下り回線受信データＣＲＣエラー)
#define		ERR_RAU_DPA_RECV_LONGER_D		83				// エラーコード８３(下り回線受信データ長異常)
#define		ERR_RAU_DPA_RECV_SHORTER_D		84				// エラーコード８４(下り回線受信データ長異常)
#define		ERR_RAU_DPA_SEND_LEN_D			85				// エラーコード８５(下り回線送信データ長オーバー)
#define		ERR_RAU_DPA_RECV_LEN_D			86				// エラーコード８６(下り回線受信データ長オーバー)
#define		ERR_RAU_DPA_SEND_RETRY_OVER_D	87				// エラーコード８７(下り回線データ再送回数オーバー)
#define		ERR_RAU_DPA_RECV_NAK99_D		88				// エラーコード８８(下り回線送信データ強制停止受信)
// MH322914 (e) kasiyama 2016/07/12 遠隔NT-NETLAN接続時のエラーコードをFOMA接続時のエラーコードに合わせる[共通バグNo.1200](MH341106)
#define		ERR_CREDIT_HOST_COMMUNICATION		1			// クレジット回線通信エラー(HOST異常)
#define		ERR_CREDIT_SND_BUFFERFULL			57			// 送信データバッファフル
#define		ERR_CREDIT_OPEN_ERR					60			// 開局異常
#define		ERR_CREDIT_NO_ACCEPT				61			// 売上拒否データ
#define		ERR_CREDIT_CONNECTCHECK_ERR			62			// 生存確認失敗
#define		ERR_CREDIT_DPA_RECV_CRC				82			// エラーコード５４(回線受信データＣＲＣエラー)
#define		ERR_CREDIT_DPA_RECV_LONGER			83			// エラーコード５５(回線受信データ長異常)
#define		ERR_CREDIT_DPA_RECV_SHORTER			84			// エラーコード５６(回線受信データ長異常)
#define		ERR_CREDIT_FMA_SEND_LEN				85			// FOMAエラーコード８５(回線送信データ長オーバー)
#define		ERR_CREDIT_FMA_RECV_LEN				86			// FOMAエラーコード８６(回線受信データ長オーバー)
#define		ERR_CREDIT_FMA_SEND_RETRY_OVER		87			// FOMAエラーコード８７(回線データ再送回数オーバー)
#define		ERR_CREDIT_FMA_RECV_NAK99			88			// FOMAエラーコード８８(回線送信データ強制停止受信)
#define		ERR_CREDIT_NAK_RETRYOVER			89			// NAK受信によるリトライオーバー
#define		ERR_CREDIT_TCP_CONNECT				91			// エラーコード９１(上り回線ＴＣＰコネクションタイムアウト)


/*[]----------------------------------------------------------------------[]*/
/*| ｴﾗｰｺｰﾄﾞ表ﾓｼﾞｭｰﾙｺｰﾄﾞに対応したｲﾝﾃﾞｯｸｽ                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Used By static const char moj_no[99] & ERR_CHK[ Index No. ]            |*/
/*|                                                                        |*/
/*| ERR_CHK[mod_note][]                                                    |*/
/*| mod_noteは moj_no[ｴﾗｰｺｰﾄﾞ表ﾓｼﾞｭｰﾙｺｰﾄﾞ]に格納されている                 |*/
/*|                                                                        |*/
/*| ｴﾗｰｺｰﾄﾞ表ﾓｼﾞｭｰﾙｺｰﾄﾞから対応するERR_CHKを導く場合                       |*/
/*| ERR_CHK[moj_no[ｴﾗｰｺｰﾄﾞ表ﾓｼﾞｭｰﾙｺｰﾄﾞ]][]となる                           |*/
/*[]----------------------------------------------------------------------[]*/
#define		mod_main		0								/* ﾒｲﾝ制御						*/
#define		mod_read		1								/* 磁気ﾘｰﾀﾞｰ					*/
#define		mod_prnt		2								/* ﾚｼｰﾄ,ｼﾞｬｰﾅﾙ					*/
#define		mod_coin		3								/* ｺｲﾝ識別循環					*/
#define		mod_note		4								/* 紙幣ﾘｰﾀﾞｰ					*/
#define		mod_ifrock		5								/* IF盤･ﾛｯｸ装置					*/
#define		mod_mifr		6								/* Mifear						*/
#define		mod_ntnet		7								/* NT-NET						*/
#define		mod_ntibk		8								/* NT-NET IBK					*/
#define		mod_lprn		9								/* ﾗﾍﾞﾙﾌﾟﾘﾝﾀ					*/
#define		mod_ifflap		10								/* IF盤･ﾌﾗｯﾌﾟ装置					*/
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(CCT)
//#define		mod_cct			11								/* IBK(CCT)	(ModCode=54)*/
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(CCT)
enum {
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(CCT)
//	mod_lanibk = mod_cct+1,							/* IBK(LAN) (ModCode=72)*/
	mod_lanibk = mod_ifflap+1,						/* IBK(LAN) (ModCode=72)*/
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(CCT)
	mod_dopaibk,									/* IBK(Dopa)(ModCode=73)*/
	// 今後 新モジュールはここに定義を追加
	mod_Suica,										/* Suica (ModCode=69)		*/
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	mod_Edy,  										/* Edy (ModCode=62)			*/
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	mod_remote,
	mod_fomaibk,									/* IBK(FOMA)(ModCode=77)*/

	mod_ntmf,										/* NTNET-DOPA子機MAF(ModCode=65)*/
	mod_ntdp,										/* NTNET-DOPA子機	(ModCode=67)*/
	mod_cappi,										/* Cappi	(ModCode=74)*/
	mod_idc,										/* iD		(ModCode=75)*/
	mod_crrflap,									/* CRR基板・フラップ装置(ModCode=15)*/
	mod_card,
	mod_flapcrb,
	mod_can,								// CAN通信
	mod_sodiac,								// 音声合成（sodiac)
	mod_I2c,
// MH321800(S) G.So ICクレジット対応
	mod_ec,									// 決済リーダ
// MH321800(E) G.So ICクレジット対応
// MH810100(S) K.Onodera 2019/12/12 車番チケットレス(LCD_IF対応)
	mod_tklslcd,							// 車番チケットレスLCD通信
	mod_barcode,							// バーコード
	mod_dc_net,								// DC-NET通信
	mod_realtime,							// リアルタイム通信
// MH810100(E) K.Onodera 2019/12/12 車番チケットレス(LCD_IF対応)
// MH810100(S) S.Fujii 2020/08/24 #4609 SDカードエラー対応
	mod_sd,
// MH810100(E) S.Fujii 2020/08/24 #4609 SDカードエラー対応
// MH810104 GG119201(S) 電子ジャーナル対応
	mod_eja,								// 電子ジャーナル
// MH810104 GG119201(E) 電子ジャーナル対応
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
	mod_cc,									// クラウド版料金計算エンジン
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応

	mod_SubCPU,								// ｻﾌﾞCPU機能
	mod_Credit,
	ERR_MOD_ALL,
	mod_lanibk2 = ERR_MOD_ALL,
	mod_dopaibk2,
	ERR_MOD_MAX,
};

#define		ERR_NUM_MAX		100								/* ｴﾗｰｺｰﾄﾞ数MAX					*/
#define		HASSEI_MAX		33

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加)_変更)))
//#define		ALM_MOD_MAX		3								/* ｱﾗｰﾑﾓｼﾞｭｰﾙ数MAX				*/
enum{
	alm_mod_main,
	alm_mod_sub1,
	alm_mod_sub2,
	alm_mod_auth,
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
	alm_mod_cc,
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// GG124100(S) R.Endo 2022/08/29 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
#ifdef DEBUG_ALARM_ACT
	alm_mod_debug,
#endif
// GG124100(E) R.Endo 2022/08/29 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
	ALM_MOD_MAX
};
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加)_変更))
#define		ALM_NUM_MAX		100								/* ｱﾗｰﾑｺｰﾄﾞ数MAX				*/

extern	char			ERR_CHK[ERR_MOD_MAX][ERR_NUM_MAX];	// ｴﾗｰ状況
extern	char			ALM_CHK[ALM_MOD_MAX][ALM_NUM_MAX];	// ｱﾗｰﾑ状況

typedef struct{													/* エラー・アラーム対象用構造体 */
	int		kind;												/* エラー・アラーム種別 */
	uchar	code;												/* 種別毎コード */
}CERTIFY_ERR_ARM;

// MH322914(S) K.Onodera 2016/09/07 AI-V対応：エラーアラーム
extern ulong	ERR_INFO[ERR_MOD_MAX][ERR_NUM_MAX];		// 発生中エラー情報
extern ulong	ALM_INFO[ALM_MOD_MAX][ALM_NUM_MAX];		// 発生中アラーム情報
extern ulong	ERR_LOCK_INFO[LOCK_MAX][4];				// 上昇/下降エラー情報;
// MH322914(E) K.Onodera 2016/09/07 AI-V対応：エラーアラーム

/*[]----------------------------------------------------------------------[]*/
/*| ｱﾗｰﾑ制御                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| ｱﾗｰﾑｺｰﾄﾞ表 ﾓｼﾞｭｰﾙｺｰﾄﾞ定義                                              |*/
/*| alm_chk(md,,)の第１引数mdとして使用する                                |*/
/*[]----------------------------------------------------------------------[]*/
#define		ALMMDL_MAIN					0					/* ﾒｲﾝ制御ﾓｼﾞｭｰﾙｺｰﾄﾞ			*/
#define		ALMMDL_SUB					1					/* ｻﾌﾞ制御ﾓｼﾞｭｰﾙｺｰﾄﾞ			*/
#define		ALMMDL_SUB2					2					/* ｻﾌﾞ制御ﾓｼﾞｭｰﾙｺｰﾄﾞ2			*/
// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// // MH810100(S)
// #define		ALMMDL_AUTH					5					// 認証機関連
// // MH810100(E)
#define		ALMMDL_CAL_CLOUD			8					// クラウド版料金計算エンジン
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応

//------------------
// ｱﾗｰﾑ関連定義
//------------------

// Used by Function alm_chk( #define, ...

// ﾓｼﾞｭｰﾙｺｰﾄﾞ0
#define		ALARM_DOORALARM			1						// ﾄﾞｱ警報
#define		ALARM_VIBSNS			5						// 振動ｾﾝｻ
#define		ALARM_NOTERE_REMOVE		6						// 紙幣ﾘｰﾀﾞｰ脱検知
#define		ALARM_LAG_EXT_OVER		30						// ラグタイム延長操作規定回数オーバー
// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
#define		ALARM_LONG_PARKING		31						// 長期駐車検出
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応

// ﾓｼﾞｭｰﾙｺｰﾄﾞ1
// 並べ替えと修正
#define		ALARM_JPAPEREND			1						// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ用紙ｴﾝﾄﾞ
#define		ALARM_RPAPEREND			2						// ﾚｼｰﾄﾌﾟﾘﾝﾀ用紙ｴﾝﾄﾞ
//#define	ALARM_RPAPEROPEN		3						// ﾚｼｰﾄﾌﾟﾘﾝﾀｵｰﾌﾟﾝ
#define		ALARM_NOTESAFEFULL		6						// 紙幣金庫満杯
#define		ALARM_COINSAFEFULL		7						// ｺｲﾝ金庫満杯
//#define	ALARM_DOOROPEN			9						// ﾄﾞｱ開
//#define	ALARM_NOTESAFEOPEN		11						// 紙幣金庫開
//#define	ALARM_COINSAFEOPEN		12						// ｺｲﾝ金庫開
#define		ALARM_500COIN_EMPTY		15						// 500円釣切れ
#define		ALARM_100COIN_EMPTY		16						// 100円釣切れ
#define		ALARM_50COIN_EMPTY		17						// 50円釣切れ
#define		ALARM_10COIN_EMPTY		18						// 10円釣切れ
#define		ALARM_500COIN_NEMPTY	21						// 500円釣切れ予告
#define		ALARM_100COIN_NEMPTY	22						// 100円釣切れ予告
#define		ALARM_50COIN_NEMPTY		23						// 50円釣切れ予告
#define		ALARM_10COIN_NEMPTY		24						// 10円釣切れ予告
#define		ALARM_TURIKAN_REV		26						// 釣銭管理枚数補正
#define		ALARM_JPAPERNEAREND		28						// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ用紙ﾆｱｴﾝﾄﾞ
#define		ALARM_RPAPERNEAREND		29						// ﾚｼｰﾄﾌﾟﾘﾝﾀ用紙ﾆｱｴﾝﾄﾞ
#define		ALARM_COINSAFENFULL		31						// ｺｲﾝ金庫満杯予告
#define		ALARM_NOTESAFENFULL		32						// 紙幣金庫満杯予告
#define		ALARM_SALELOG_FULL		43						// 精算ﾛｸﾞFULLにてﾃﾞｰﾀ上書き	※現在処理なし（今回未対応）
#define		ALARM_TOTALLOG_FULL		44						// 集計ﾛｸﾞFULLにてﾃﾞｰﾀ上書き	※現在処理なし（今回未対応）
// MH810104 GG119201(S) 電子ジャーナル対応
#define		ALARM_SD_EXPIRATION		45						// SDカード使用期限超過
// MH810104 GG119201(E) 電子ジャーナル対応
#define		ALARM_READERSENSOR		53						// カードリーダセンサ（レベル低）
// MH810104 GG119201(S) 電子ジャーナル対応
#define		ALARM_SD_END			55						// SDカードエンド
#define		ALARM_SD_NEAR_END		56						// SDカードニアエンド
// MH810104 GG119201(E) 電子ジャーナル対応
#define		ALARM_LBPAPEREND		46						// ﾗﾍﾞﾙﾌﾟﾘﾝﾀ紙切れ
#define		ALARM_LBRIBBONEND		47						// ﾗﾍﾞﾙﾌﾟﾘﾝﾀﾘﾎﾞﾝ切れ
#define		ALARM_LBPAPERNEAREND	48						// ﾗﾍﾞﾙﾌﾟﾘﾝﾀ紙切れ予告
#define		ALARM_LBRIBBONNEAREND	49						// ﾗﾍﾞﾙﾌﾟﾘﾝﾀﾘﾎﾞﾝ切れ予告
#define		ALARM_RPAPEROPEN		60						// ﾚｼｰﾄﾌﾟﾘﾝﾀｵｰﾌﾟﾝ		(登録するがAlarm項目としない)
#define		ALARM_JPAPEROPEN		61						// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀｵｰﾌﾟﾝ		(登録するがAlarm項目としない)
// MH810105 GG119202(S) リーダから直取中の状態データ受信した際の新規アラームを設ける
#define		ALARM_EC_LASTSETTERUN	84						// 決済リーダ利用不可
// MH810105 GG119202(E) リーダから直取中の状態データ受信した際の新規アラームを設ける
// MH321800(S) Y.Tanizaki ICクレジット対応
#define		ALARM_EC_SETTLEMENT_TIME_GAP	89				// 決済時刻と精算機の時刻に1日以上のズレがある
// MH321800(E) Y.Tanizaki ICクレジット対応
#define		ALARM_CNM_IN_AMOUNT_MAX	90						// 投入金最大枚数オーバー時の入金額クリア実施
// MH321800(S) G.So ICクレジット対応
#define		ALARM_EC_OPEN			93						// リーダ受付可制御失敗
// MH321800(E) G.So ICクレジット対応

// ﾓｼﾞｭｰﾙｺｰﾄﾞ2
#define		ALARM_MUKOU_PASS_USE	1						// 無効登録定期券使用
#define		ALARM_PARKING_NO_NG		2						// 駐車場NG定期券
#define		ALARM_ANTI_PASS_NG		3						// アンチパスNG定期券
#define		ALARM_VALIDITY_TERM_OUT	4						// 有効期間外の定期券
#define		ALARM_NOT_USE_TICKET	7						// 使用不可定期券
#define		ALARM_N_MINUTE_RULE		10						// n分ルール定期券
#define		ALARM_GT_MUKOU_PASS_USE		40						// 無効登録定期券使用
#define		ALARM_GT_PARKING_NO_NG		41						// 駐車場NG定期券
#define		ALARM_GT_ANTI_PASS_NG		42						// アンチパスNG定期券
#define		ALARM_GT_VALIDITY_TERM_OUT	43						// 有効期間外の定期券
#define		ALARM_GT_NOT_USE_TICKET		46						// 使用不可定期券
#define		ALARM_GT_N_MINUTE_RULE		49						// n分ルール定期券
#define		ALARM_GT_MIFARE_READ_CRC_ERR	54					// Mifareデータ不良（読込みCRCエラー）
#define		ALARM_OUTSIDE_PERIOD_PRC	85						// 有効期間外のﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ
#define		ALARM_RED_AFTERWRITE_PRC	86						// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ書込み不良
#define		ALARM_WRITE_CRC_ERR			97						// 磁気データ書込みCRCエラー
#define		ALARM_READ_CRC_ERR			98						// 磁気データ読込みCRCエラー

// GG124100(S) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応
// // ﾓｼﾞｭｰﾙｺｰﾄﾞ5
// #define		ALARM_USED_RECEIPT			30						// 割引利用済みレシート
// #define		ALARM_NOT_USE_RECEIPT		31						// 対象外レシート
// #define		ALARM_EXPIRED_RECEIPT		32						// 有効期限切れレシート
// #define		ALARM_USE_COUNT_OVER		33						// レシート読み込み上限
// #define		ALARM_DIS_PRM_NOT_SET		34						// 対象外バーコード
// ﾓｼﾞｭｰﾙｺｰﾄﾞ8
#define		ALMMDL_CC_PAY_SET_ERR			1				// 料金設定異常
#define		ALMMDL_CC_PASS_SET_ERR			2				// 定期券設定異常
#define		ALMMDL_CC_PAYPATTERN_SET_ERR	3				// 料金パターン設定異常
#define		ALMMDL_CC_MAXCHARGE_SET_ERR		4				// 駐車時間最大料金設定異常
#define		ALMMDL_CC_TIMEZONE_SET_ERR		5				// 時間帯設定異常
#define		ALMMDL_CC_UNITTIME_SET_ERR		6				// 単位時間設定異常
#define		ALMMDL_CC_SVS_SET_ERR			7				// サービス券設定異常
#define		ALMMDL_CC_KAK_SET_ERR			8				// 店割引設定異常
#define		ALMMDL_CC_TKAK_SET_ERR			9				// 多店舗割引設定異常
#define		ALMMDL_CC_PMCSVS_SET_ERR		10				// PMCサービス券設定異常
#define		ALMMDL_CC_SYUBET_SET_ERR		11				// 車種割引設定異常
#define		ALMMDL_CC_WRI_SET_ERR			12				// 割引券設定異常
#define		ALMMDL_CC_UNSUPPORTED_DISC_USE	64				// 未対応割引使用
// GG124100(E) R.Endo 2022/09/12 車番チケットレス3.0 #6343 クラウド料金計算対応

//------------------
// 操作履歴関連定義
//------------------
#define		OPLOG_ALL_INITIAL		1						// 全イニシャライズ起動(スーパーイニシャル)
#define		OPLOG_DATA_INITIAL		2						// データイニシャル起動(ログイニシャル)
#define		OPLOG_DOOROPEN			101						// ドア開操作履歴登録
#define		OPLOG_DOORCLOSE			102						// ドア閉操作履歴登録
#define		OPLOG_COINMECHSWOPEN	103						// コインメックSW開の操作履歴登録
#define		OPLOG_COINMECHSWCLOSE	104						// コインメックSW閉の操作履歴登録
#define		OPLOG_NOTERDSWOPEN		105						// 紙幣ﾘｰﾀﾞｰSW開の操作履歴登録
#define		OPLOG_NOTERDSWCLOSE		106						// 紙幣ﾘｰﾀﾞｰSW閉の操作履歴登録
#define		OPLOG_COINKINKO			107						// コイン金庫開の操作履歴登録
#define		OPLOG_NOTEKINKO			108						// 紙幣金庫開の操作履歴登録
#define		OPLOG_MNT_START_PASS	111						// 係員操作開始（パスワードによる開始）
#define		OPLOG_MNT_START_CARD	112						// 係員操作開始（係員カードによる開始）
#define		OPLOG_MNT_END			115						// 係員操作終了
#define		OPLOG_ATTRNDANT_CARD	120						// ﾘﾊﾟｰｸ係員ｶｰﾄﾞを使用しﾄﾞｱﾛｯｸ解除
#define		OPLOG_NO_CARD_OPEN		121						// 券無開機能を使用しﾄﾞｱﾛｯｸ解除
#define		OPLOG_DOORNOBU_LOCK		122						// ﾄﾞｱﾉﾌﾞﾛｯｸ
#define		OPLOG_SYUSEI_START		128						// 修正精算操作開始
#define		OPLOG_SYUSEI_END		129						// 修正精算操作終了

#define		OPLOG_TURIHOJU			1101					// 補充操作記録
#define		OPLOG_TURIHENKO			1102					// 枚数変更操作記録
#define		OPLOG_TURIAUTO			1103					// AUTO操作記録
#define		OPLOG_TURICASETTE		1104					// ｶｾｯﾄ着操作記録
#define		OPLOG_INVENTRY			1105					// ｲﾝﾍﾞﾝﾄﾘ操作記録
#define		OPLOG_INVBUTTON			1106					// ｲﾝﾍﾞﾝﾄﾘ操作記録(ｲﾝﾍﾞﾝﾄﾘﾎﾞﾀﾝ)
#define		OPLOG_SHOKIMAISUHENKO	1107					// 初期枚数変更操作記録
#define		OPLOG_TURIKAN_SHOKEI	1108					// 釣銭管理小計操作履歴登録

#define		OPLOG_T_SHOKEI			1201					// Ｔ小計操作履歴登録
#define		OPLOG_T_GOKEI			1202					// Ｔ合計操作履歴登録
#define		OPLOG_T_ZENGOKEI		1203					// Ｔ合計操作履歴登録（再発行）
#define		OPLOG_GT_SHOKEI			1204					// ＧＴ小計操作履歴登録
#define		OPLOG_GT_GOKEI			1205					// ＧＴ合計操作履歴登録
#define		OPLOG_GT_ZENGOKEI		1206					// ＧＴ合計操作履歴登録（再発行）
#define		OPLOG_MT_SHOKEI			1207					// ＭＴ小計操作履歴登録
#define		OPLOG_MT_GOKEI			1208					// ＭＴ合計操作履歴登録
#define		OPLOG_MT_ZENGOKEI		1209					// ＭＴ合計操作履歴登録（再発行）
#define		OPLOG_F_SHOKEI			1210					// 複数小計操作履歴登録
#define		OPLOG_F_GOKEI			1211					// 複数合計操作履歴登録
#define		OPLOG_F_ZENGOKEI		1212					// 複数合計操作履歴登録（再発行）
#define		OPLOG_COIN_SHOKEI		1220					// コイン金庫小計操作履歴登録
#define		OPLOG_COIN_ZENGOUKEI	1221					// コイン金庫合計操作履歴登録（再発行）
#define		OPLOG_NOTE_SHOKEI		1222					// 紙幣金庫小計操作履歴登録
#define		OPLOG_NOTE_ZENGOUKEI	1223					// 紙幣金庫合計操作履歴登録（再発行）

#define		OPLOG_CAN_T_SUM			1240					// T合計中止
#define		OPLOG_CAN_GT_SUM		1241					// GT合計中止
#define		OPLOG_CAN_MT_SUM		1242					// MT合計中止

#define		OPLOG_TOKEISET			1301					// 時計操作履歴登録
#define		OPLOG_TOKEISET2			1302					// 時計合わせ（手動）
#define		OPLOG_LOCK_OPEN			1501					// ロック装置開
#define		OPLOG_A_LOCK_OPEN		1502					// 全ロック装置開
#define		OPLOG_FLAP_DOWN			1511					// フラップ下降
#define		OPLOG_A_FLAP_DOWN		1512					// 全フラップ下降
#define		OPLOG_FLAP_SENSOR_OFF	1521					// フラップ　車両検知センサー強制OFF

#define		OPLOG_LOCK_CLOSE		1601					// ロック装置閉
#define		OPLOG_A_LOCK_CLOSE		1602					// 全ロック装置閉
#define		OPLOG_FLAP_UP			1611					// フラップ上昇
#define		OPLOG_A_FLAP_UP			1612					// 全フラップ上昇
#define		OPLOG_FLAP_SENSOR_ON	1621					// フラップ　車両検知センサー強制ON

#define		OPLOG_CHUSHADAISU		1701					// 駐車台数履歴登録
#define		OPLOG_MANSYACONTROL		1702					// 満車コントロール履歴登録
#define		OPLOG_RYOSHUSAIHAKKO	1703					// 領収証再発行履歴登録
#define		OPLOG_UKETUKESAIHAKKO	1704					// 受付券再発行履歴登録
#define		OPLOG_EIKYUGYOKIRIKAE	1705					// 営休業切換履歴登録
#define		OPLOG_SHUTTERKAIHO		1706					// シャッター開放履歴登録
#define		OPLOG_SERVICETIME		1707					// サービスタイム履歴登録
#define		OPLOG_ROCKTIMER			1708					// ロック装置閉タイマー履歴登録
#define		OPLOG_TEIKICHKSURU		1710					// 操作履歴登録(定期／入出力チェックする)
#define		OPLOG_TEIKICHKSHINAI	1711					// 操作履歴登録(定期／入出力チェックしない)
#define		OPLOG_TEIKIYUKOMUKO		1712					// 操作履歴登録(定期／定期有効・無効)
#define		OPLOG_TEIKIZENYUKO		1713					// 操作履歴登録(定期／全定期有効)
#define		OPLOG_TEIKIZENMUKO		1714					// 操作履歴登録(定期／全定期無効)
#define		OPLOG_TEIKIINOUT		1715					// 操作履歴登録(定期／入出庫)
#define		OPLOG_TEIKIZENSHOKI		1716					// 操作履歴登録(定期／全初期)
#define		OPLOG_TEIKIMUKOYOUBI	1717					// 定期無効曜日操作履歴登録
#define		OPLOG_TOKUBETUBI		1718					// 特別日操作履歴登録
#define		OPLOG_TOKUBETUKIKAN		1719					// 特別期間操作履歴登録
#define		OPLOG_TOKUBETUHAPPY		1720					// ハッピーマンデー操作履歴登録
#define		OPLOG_KAKARIINPASS		1721					// 係員パスワード操作履歴登録
#define		OPLOG_BACKLIGHT			1722					// バックライト点灯操作履歴登録
#define		OPLOG_CONTRAST			1723					// コントラスト調整操作履歴登録
#define		OPLOG_KEYVOLUME			1724					// キー音量調整操作履歴登録
#define		OPLOG_KENKIGEN			1725					// 券期限操作履歴登録
#define		OPLOG_EIGYOKAISHI		1726					// 営業開始操作履歴登録
#define		OPLOG_TEIKICHUSI_DEL	1750					// 定期券中止データ１件削除 操作履歴登録
#define		OPLOG_TEIKICHUSI_ALLDEL	1751					// 定期券中止データ全削除 操作履歴登録
#define		OPLOG_PEXTIM_DEL		1752					// 定期券出庫時刻データ１件削除 操作履歴登録
#define		OPLOG_PEXTIM_ALLDEL		1753					// 定期券出庫時刻データ全削除   操作履歴登録
#define		OPLOG_TOKUBETUWEEKLY	1767					// 特別曜日設定(Y.Ise:暫定)
#define		OPLOG_LPRNSAIHAKKO		1768					// ラベル再発行操作履歴登録
#define		OPLOG_LPRPAPERSET		1769					// ラベル残枚数操作履歴登録
#define		OPLOG_LPRKOSINARI		1770					// ラベル発行不可時の更新あり操作登録
#define		OPLOG_LPRKOSINNASI		1771					// ラベル発行不可時の更新なし操作登録
#define		OPLOG_CRE_CAN_DEL		1772					// クレジット取消情報・データ削除
#define		OPLOG_CRE_CAN_PRN		1773					// 取消情報・印刷
#define		OPLOG_CRE_CONECT		1774					// 接続確認
#define		OPLOG_CRE_USE_DEL		1775					// 利用明細・削除

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		OPLOG_EDY_ARM_PRN		1776					// Edyｱﾗｰﾑ取引情報ﾌﾟﾘﾝﾄ
//#define		OPLOG_EDY_ARM_CLR		1777					// Edyｱﾗｰﾑ取引情報ｸﾘｱ
//#define		OPLOG_EDY_PRM_CHG		1778					// 関連設定変更
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

#define		OPLOG_VLSWTIME			1779					// 音量切替時刻履歴登録
#define		OPLOG_TOKUBETUYOBI		1780					// 特別曜日操作履歴登録
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		OPLOG_EDY_SHIME_PRN		1781					// Edy締め記録情報ﾌﾟﾘﾝﾄ
//#define		OPLOG_EDY_SHIME_CLR		1782					// Edy締め記録情報ｸﾘｱ
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		OPLOG_CARDINFO_UPDATE	1783					// カード情報データ要求	
#define		OPLOG_PRG_DL			1790					// プログラムダウンロード実施
#define		OPLOG_CPARAM_DL			1791					// 共通パラメータダウンロード実施
#define		OPLOG_MAINPRG_RESERVE	1792					// メインプログラム運用面切替予約実施
#define		OPLOG_SUBPRG_RESERVE	1793					// サブプログラム運用面切替予約実施
#define		OPLOG_CPARAM_RESERVE	1794					// 共通パラメーター運用面切替予約実施
#define		OPLOG_CHARGE_PRINT		1795					// 料金設定プリント
#define		OPLOG_SCASYU_CLR		1797					// Suica日毎集計消去
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		OPLOG_EDYSYU_CLR		1798					// Edy日毎集計消去
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		OPLOG_HOJIN_USEDEL		1799

#define		OPLOG_SAIDAIRYOKIN		1801					// 最大料金操作履歴登録
#define		OPLOG_SERVICEYAKUWARI	1802					// サービス券役割操作履歴登録
#define		OPLOG_MISEKUWARI		1803					// 店役割操作履歴登録
#define		OPLOG_SHOHIZEI			1804					// 消費税の税率操作履歴登録
#define		OPLOG_ALYAKUWARI		1805					// Ａ～Ｌ種役割操作履歴登録
#define		OPLOG_TANIJIKANRYO		1806					// 単位時間料金操作履歴登録
#define		OPLOG_CAN_SET_PRI		1830					// 運用設定プリント中止操作履歴登録

#define		OPLOG_KYOTUPARAWT		2001					// 共通ﾊﾟﾗﾒｰﾀｰ操作履歴登録
#define		OPLOG_SHASHITUPARAWT	2002					// 車室ﾊﾟﾗﾒｰﾀｰ操作履歴登録
#define		OPLOG_ROCKPARAWT		2003					// ﾛｯｸﾊﾟﾗﾒｰﾀｰ操作履歴登録
#define		OPLOG_KYOTUPARADF		2010					// 共通ﾊﾟﾗﾒｰﾀｰ操作履歴登録
#define		OPLOG_SHASHITUPARADF	2011					// 車室ﾊﾟﾗﾒｰﾀｰ操作履歴登録
#define		OPLOG_ROCKPARADF		2012					// ﾛｯｸﾊﾟﾗﾒｰﾀｰ操作履歴登録
#define		OPLOG_T_HOUJIN_DEF		2020					// T・法人カードデフォルトテーブルセット

#define		OPLOG_SWCHK				2301					// 動作ﾁｪｯｸ(SWﾁｪｯｸ)履歴登録
#define		OPLOG_KEYCHK			2302					// 動作ﾁｪｯｸ(ｷｰﾁｪｯｸ)履歴登録
#define		OPLOG_LCDCHK			2303					// 動作ﾁｪｯｸ(LCDﾁｪｯｸ)履歴登録
#define		OPLOG_LEDCHK			2304					// 動作ﾁｪｯｸ(LEDﾁｪｯｸ)履歴登録
#define		OPLOG_SHUTTERCHK		2305					// 動作ﾁｪｯｸ(ｼｬｯﾀｰﾁｪｯｸ)履歴登録
#define		OPLOG_SIGNALCHK			2306					// 動作ﾁｪｯｸ(入出力信号ﾁｪｯｸ)履歴登録
#define		OPLOG_COINMECHCHK		2307					// 動作ﾁｪｯｸ(ｺｲﾝﾒｯｸﾁｪｯｸ)履歴登録
#define		OPLOG_NOTEREADERCHK		2308					// 動作ﾁｪｯｸ(紙幣ﾘｰﾀﾞｰﾁｪｯｸ)履歴登録
#define		OPLOG_PRINTERCHK		2309					// 動作ﾁｪｯｸ(ﾌﾟﾘﾝﾀｰﾁｪｯｸ)履歴登録
#define		OPLOG_ANNAUNCECHK		2310					// 動作ﾁｪｯｸ(ｱﾅｳﾝｽﾁｪｯｸ)履歴登録
#define		OPLOG_MEMORYCHK			2311					// 動作ﾁｪｯｸ(ﾒﾓﾘﾁｪｯｸ)履歴登録
#define		OPLOG_IFBOARDCHK		2312					// 動作ﾁｪｯｸ(IF盤ﾁｪｯｸ)履歴登録
#define		OPLOG_ARCNETCHK			2313					// 動作ﾁｪｯｸ(ARCNETﾁｪｯｸ)履歴登録
#define		OPLOG_READERCHK			2314					// 動作ﾁｪｯｸ(磁気ﾘｰﾀﾞｰﾁｪｯｸ)履歴登録
#define		OPLOG_NYUKINCHK			2318					// 動作ﾁｪｯｸ(ｺｲﾝ入金ﾁｪｯｸ)履歴登録
#define		OPLOG_HARAIDASHICHK		2319					// 動作ﾁｪｯｸ(ｺｲﾝ出金ﾁｪｯｸ)履歴登録
#define		OPLOG_NOTENYUCHK		2320					// 動作ﾁｪｯｸ(紙幣入金ﾁｪｯｸ)履歴登録
#define		OPLOG_DOUSACOUNT		2323					// 動作ﾁｪｯｸ(動作ｶｳﾝﾄ)履歴登録
#define		OPLOG_DOUSACOUNTCLR		2324					// 動作ﾁｪｯｸ(動作ｶｳﾝﾄ-ｸﾘｱ)履歴登録
#define		OPLOG_LOCKDCNT			2325					// 動作ﾁｪｯｸ(ﾛｯｸ装置動作ｶｳﾝﾄ)	履歴登録
#define		OPLOG_LOCKDCNTCLR		2347					// 動作ﾁｪｯｸ(ﾛｯｸ装置動作ｶｳﾝﾄ-ｸﾘｱ)履歴登録
#define		OPLOG_FLAPDCNT			2348					// 動作ﾁｪｯｸ(ﾌﾗｯﾌﾟ動作ｶｳﾝﾄ)		履歴登録
#define		OPLOG_FLAPDCNTCLR		2349					// 動作ﾁｪｯｸ(ﾌﾗｯﾌﾟ動作ｶｳﾝﾄ-ｸﾘｱ)	履歴登録
#define		OPLOG_ALLBACKUP			2326					// 全ﾃﾞｰﾀﾊﾞｯｸｱｯﾌﾟ履歴登録
#define		OPLOG_ALLRESTORE		2327					// 全ﾃﾞｰﾀﾘｽﾄｱ履歴登録
#define		OPLOG_SHASHITUDTSET		2328					// 車室情報調整
#define		OPLOG_NTNETCHK			2338					// 動作ﾁｪｯｸ(NTNETﾁｪｯｸ)履歴登録
#define		OPLOG_VERSIONCHK		2399					// 動作ﾁｪｯｸ(ﾊﾞｰｼﾞｮﾝﾁｪｯｸ)履歴登録
#define		OPLOG_MIFARECHK			2339					// 動作ﾁｪｯｸ(MIFAREﾁｪｯｸ)履歴登録
#define		OPLOG_CCOMCHK			2346					// 動作ﾁｪｯｸ(ｾﾝﾀｰ通信ﾁｪｯｸ)履歴登録
#define		OPLOG_SUICACHK			2344					// 動作ﾁｪｯｸ(Suicaﾁｪｯｸ)履歴登録 
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		OPLOG_EDYCHK			2345					// 動作ﾁｪｯｸ(Edyﾁｪｯｸ)履歴登録 
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		OPLOG_PIPCHK			2342					// 動作ﾁｪｯｸ(ParkiProﾁｪｯｸ)履歴登録
#define		OPLOG_MAFCHK			2343					// 動作ﾁｪｯｸ(MAFﾁｪｯｸ)履歴登録
#define		OPLOG_CAPPICHK			2350					// 動作ﾁｪｯｸ(Cappiﾁｪｯｸ)履歴登録
#define		OPLOG_SCICHK			2360					// 動作ﾁｪｯｸ(汎用通信ﾗｲﾝﾁｪｯｸ)履歴登録 
#define		OPLOG_CCOMCHK_APN		2361					// 動作ﾁｪｯｸ(ｾﾝﾀｰ通信ﾁｪｯｸ APN変更)履歴登録
#define		OPLOG_CCOMCHK_ANT		2362					// 動作ﾁｪｯｸ(ｾﾝﾀｰ通信ﾁｪｯｸ 電波受信状態確認)履歴登録
#define		OPLOG_CCRVERCHK			2370					// 動作ﾁｪｯｸ(CRRバージョンチェック)履歴登録
#define		OPLOG_CCRCOMCHK			2371					// 動作ﾁｪｯｸ(CRR折り返しテスト)履歴登録
#define		OPLOG_RISMCOMCHK		2372					// 動作ﾁｪｯｸ(Rismサーバー接続確認)履歴登録
#define		OPLOG_CAR_FUNC_SELECT	2373					// 動作ﾁｪｯｸ(車室機能選択)履歴登録
#define		OPLOG_FTP_PARAM_UPLD	2374					// 動作ﾁｪｯｸ(FTP接続確認 パラメータアップロード実行)履歴登録
#define		OPLOG_FTP_CONNECT_CHK	2375					// 動作ﾁｪｯｸ(FTP接続確認 FTP接続確認実行)履歴登録
#define		OPLOG_FLA_LOOP_DATA		2376					// フラップループデータ確認
#define		OPLOG_FLA_SENSOR_CTRL	2377					// フラップ車両検知センサー操作
#define		OPLOG_FANOPE_CHK		2378					// FAN動作チェック履歴登録
#define		OPLOG_LAN_CONNECT_CHK	2379					// 動作ﾁｪｯｸ(LAN接続確認)履歴登録
#define		OPLOG_CHK_PRINT			2380					// 動作ﾁｪｯｸ(工場確認プリント)履歴登録
// MH321800(S) G.So ICクレジット対応
#define		OPLOG_CHK_EC			2383					// 動作ﾁｪｯｸ(決済リーダチェック)履歴登録
// MH321800(E) G.So ICクレジット対応
// MH810100(S) Y.Yamauchi 20191015 車番チケットレス(メンテナンス)
#define		OPLOG_QRCHK				2440					// 動作ﾁｪｯｸ(ＱＲリーダーチェック)履歴登録
#define		OPLOG_REALCHK			2441					// 動作ﾁｪｯｸ(リアルタイム通信チェック)履歴登録
#define		OPLOG_DCLCHK			2442					// 動作ﾁｪｯｸ(DC-NET通信チェック)履歴登録
// MH810100(E) Y.Yamauchi 20191015 車番チケットレス(メンテナンス)
#define		OPLOG_WAVEDATA_RESERVE	2502					// 音声データ運用面切替予約実施
#define		OPLOG_CARINFO_DL		2520					// 車室情報ダウンロード実施
#define		OPLOG_CCRFAIL_ON		2521					// 車室故障(なし⇒あり)履歴登録
#define		OPLOG_CCRFAIL_OFF		2522					// 車室故障(あり⇒なし)履歴登録
#define		OPLOG_SHOUMEISETTEI		2523					// 駐車証明書発行設定
#define		OPLOG_CRE_UNSEND_SEND	2570					// 未送信売上再送
#define		OPLOG_CRE_UNSEND_DEL	2571					// 未送信売上削除
#define		OPLOG_CRE_SALE_NG_DEL	2572					// 売上拒否データ削除
#define		OPLOG_CRE_CON_CHECK		2573					// 接続確認

/* リモートメンテナンス操作関連 */
#define		OPLOG_SET_TIME				8001				// 時計セット
#define		OPLOG_SET_MANSYACTRL		8002				// 満車コントロール変更
#define		OPLOG_SET_EIKYUGYO			8005				// 営休業状態変更
#define		OPLOG_SET_SHUTTER			8006				// シャッター状態変更		※現在処理なし（今回未対応）
#define		OPLOG_TELNET_LOGIN			8009				// TELNETログイン
#define		OPLOG_SET_NYUSYUTUCHK_ON	8010				// 定期入出力チェック「する」
#define		OPLOG_SET_NYUSYUTUCHK_OFF	8011				// 定期入出力チェック「しない」
#define		OPLOG_SET_TEIKI_YUKOMUKO	8012				// 定期有効/無効登録
#define		OPLOG_SET_TEIKI_ALLYUKO		8013				// 全定期有効登録
#define		OPLOG_SET_TEIKI_ALLMUKO		8014				// 全定期無効登録
#define		OPLOG_GT_SET_TEIKI_YUKOMUKO	8016				// 定期有効/無効登録(GTフォーマット)
#define		OPLOG_GT_SET_STATUS_CHENGE	8017				// 定期券ステータス変更(GTフォーマット)
#define		OPLOG_SET_TEIKI_TYUSI		8028				// 定期中止データ
#define		OPLOG_SET_LOCK_OPEN			8040				// ロック装置開
#define		OPLOG_SET_LOCK_CLOSE		8041				// ロック装置閉
#define		OPLOG_SET_FLAP_DOWN			8042				// フラップ下降
#define		OPLOG_SET_FLAP_UP			8043				// フラップ上昇
#define		OPLOG_SET_A_LOCK_OPEN		8044				// 全ロック装置開
#define		OPLOG_SET_A_LOCK_CLOSE		8045				// 全ロック装置閉
#define		OPLOG_SET_A_FLAP_DOWN		8046				// 全フラップ下降
#define		OPLOG_SET_A_FLAP_UP			8047				// 全フラップ上昇
#define		OPLOG_ANSHOU_B_CLR			8048				// 暗証番号消去
#define		OPLOG_SET_CTRL_RESET		8049				// 制御データリセット指示
#define		OPLOG_PARKI_GENGAKU			8051				// 減額精算
#define		OPLOG_PARKI_FURIKAE			8052				// 振替精算
#define		OPLOG_PARKI_UKETUKEHAKKO	8053				// 受付件発行
#define		OPLOG_PARKI_RYOSHUSAIHAKKO	8054				// 領収書再発行
// MH322914(S) K.Onodera 2016/09/16 AI-V対応：遠隔精算
#define		OPLOG_PARKI_ENKAKU			8055				// 遠隔精算
// MH322914(E) K.Onodera 2016/09/16 AI-V対応：遠隔精算
#define		OPLOG_SET_STATUS_CHENGE		8015				// 定期券ステータス変更
#define		OPLOG_SET_STATUS_TBL_UPDATE	8030				// 定期券ステータステーブル更新
#define		OPLOG_SET_TOKUBETU_UPDATE	8031				// 特別日設定データ更新
#define		OPLOG_SET_TIME2_COM			8033				// NT-NET/APS-NETによる時計セット
#define		OPLOG_SET_TIME2_PIP			8034				// ParkiPRO
#define		OPLOG_SET_TIME2_SNTP		8035				// SNTP 自動時計更新機能による時計セット
#define		OPLOG_SET_SNTP_SYNCHROTIME	8036				// SNTP 同期時刻変更(遠隔)
#define		OPLOG_REMOTE_DL_REQ			8084				// 遠隔ダウンロード要求結果
#define		OPLOG_REMOTE_DISCONNECT		8085				// Rism通信切断
#define		OPLOG_REMOTE_DL_END			8086				// 遠隔ダウンロード終了
#define		OPLOG_REMOTE_SW_END			8087				// プログラム更新終了
#define		OPLOG_REMOTE_RES_UPLD		8088				// 遠隔ダウンロード・プログラム更新結果アップロード終了
#define		OPLOG_REMOTE_CONNECT		8089				// Rism通信復旧
#define		OPLOG_SET_ATE_INV			8090				// 係員無効データ
#define		OPLOG_SET_PASSWORD			8091				// 係員無効データ
#define		OPLOG_SET_W_BINCODE			8092				// WカードBINコード設定
#define		OPLOG_SET_H_BINCODE			8093				// 法人カードBINコード設定
#define		OPLOG_SET_TPOINT_BASE		8094				// Ｔカード基本レート設定
#define		OPLOG_SET_TPOINT_TIME		8095				// Ｔカード期間レート設定
#define		OPLOG_SET_TCARD_USE			8096				// Ｔカード使用可否設定
#define		OPLOG_SET_ATE_INV_MAX		8097				// 係員無効データ登録上限

/* リモートパラメータ設定関連 */
#define		OPLOG_SET_KYOTUPARAM		8101				// 共通パラメータ変更
#define		OPLOG_SET_SYASITUPARAM		8102				// 車室パラメータ変更
#define		OPLOG_SET_LOCKPARAM			8103				// ロック種別パラメータ変更

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//------------------
// レーンモニタ関連定義
//------------------
// レーンモニタ（状態種別・コード）
// 問合せフェーズ
#define LM_INQ_NO							2421	// 車番検索で在車問合せ中
#define LM_INQ_TIME							2422	// 時刻検索で在車問合せ中
#define LM_INQ_QR_TICKET					2423	// QR駐車券で在車問合せ中

// 精算フェーズ
#define LM_PAY_NO_TIME						2721	// 精算中(車番/時刻検索)
#define LM_PAY_QR_TICKET					2722	// 精算中(QR駐車券)
#define LM_PAY_CHANGE						2724	// 精算中(精算中変更)
#define LM_PAY_MNY_IN						2725	// 精算中(入金済み(割引含む))
#define LM_PAY_CMP							2726	// 精算完了
#define	LM_PAY_STOP							2727	// 精算中止

// レーンモニタ（検索タイプ）
enum {
	SEARCH_TYPE_NO = 0,								// 車番検索
	SEARCH_TYPE_TIME,								// 時刻検索
	SEARCH_TYPE_QR_TICKET,							// QR検索
};

// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

//------------------
// モニタ関連定義
//------------------
enum {
	// 遠隔設定 -->
		OPMON_RSETUP_REFOK		= 80,			// 設定参照OK
		OPMON_RSETUP_REFNG,						// 同NG
		OPMON_RSETUP_PREOK,						// 設定変更準備OK
		OPMON_RSETUP_PRENG,						// 同NG
		OPMON_RSETUP_RCVOK,						// 設定データ受信OK
		OPMON_RSETUP_RCVNG,						// 同NG
		OPMON_RSETUP_EXEOK,						// 設定実行OK
		OPMON_RSETUP_EXENG,						// 同NG
		OPMON_RSETUP_CHGOK,						// 設定変更終了OK
		OPMON_RSETUP_CHGNG,						// 設定変更終了NG
		OPMON_RSETUP_CANOK,						// 設定変更キャンセルOK
		OPMON_RSETUP_CANNG,						// 同NG
		OPMON_RSETUP_NOREQ,						// 設定変更要求なし
		_OPMON_RSETUP_MAX
};

//------------------
// モニタ関連定義
//------------------
#define		OPMON_DOOROPEN				101					// ドア開
#define		OPMON_DOORCLOSE				102					// ドア閉
#define		OPMON_COINMECHSWOPEN		103					// コインメックSW開
#define		OPMON_COINMECHSWCLOSE		104					// コインメックSW閉
#define		OPMON_NOTERDSWOPEN			105					// 紙幣ﾘｰﾀﾞｰSW開
#define		OPMON_NOTERDSWCLOSE			106					// 紙幣ﾘｰﾀﾞｰSW閉
#define		OPMON_LOCK_OPEN				107					// ロック開
#define		OPMON_LOCK_CLOSE			108					// ロック閉
#define		OPMON_COINKINKO_UNSET		111					// コイン金庫未セット
#define		OPMON_COINKINKO_SET			112					// コイン金庫セット
#define		OPMON_NOTEKINKO_UNSET		113					// 紙幣金庫開
#define		OPMON_NOTEKINKO_SET			114					// 紙幣金庫閉
#define		OPMON_OPEN					121					// 営業中
#define		OPMON_CLOSW					122					// 休業中
#define		OPMON_CAR1_UNFULL			123					// 空車中１
#define		OPMON_CAR1_FULL				124					// 満車中１
#define		OPMON_CAR2_UNFULL			125					// 空車中２
#define		OPMON_CAR2_FULL				126					// 満車中２
#define		OPMON_CAR3_UNFULL			127					// 空車中３
#define		OPMON_CAR3_FULL				128					// 満車中３

#define		OPMON_CARDSTACK_OCC			145					// 磁気リーダーカード詰まり発生
#define		OPMON_CARDSTACK_REL			146					// 磁気リーダーカード詰まり解除

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		OPMON_EDY_FSTCON_START		150					// 初回通信開始
//#define		OPMON_EDY_FSTCON_STOP		151					// 初回通信終了
//#define		OPMON_EDY_MT_CEN_START		152					// 手動Ｔ合計によるセンター通信開始
//#define		OPMON_EDY_AT_CEN_START		153					// 自動Ｔ合計によるセンター通信開始
//#define		OPMON_EDY_M_CEN_START		154					// 手動センター通信開始（メンテ操作）
//#define		OPMON_EDY_A_CEN_START		155					// 自動センター通信開始
//#define		OPMON_EDY_CEN_STOP			156					// センター通信終了
//#define		OPMON_EDY_SNCKEY			157					// リーダー相互認証鍵更新
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		OPMON_TIME_AUTO_REVISE		170					// 時刻自動補正
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
#define		OPMON_SYUUKEI_IRGCLR1		171					// 集計クリア異常(①印字集計のヘッダーと前回集計のヘッダーの比較)
#define		OPMON_SYUUKEI_IRGCLR2		172					// 集計クリア異常(②現在集計の追番、今回集計日時、前回集計日時と 印字集計の追番、今回集計日時、前回集計日時の比較)
#define		OPMON_SYUUKEI_IRGCLR3		173					// 集計クリア異常(③現在集計項目の値と印字集計項目の値の比較)
#define		OPMON_SYUUKEI_IRGCLR4		174					// 集計クリア異常(④現在集計の今回集計日時が0クリアされているか確認)
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
// MH321800(S) G.So ICクレジット対応
#define		OPMON_EC_MIRYO_START		175					// 電子マネー処理未了発生
#define		OPMON_EC_MIRYO_TIMEOUT		176					// 電子マネー処理未了タイムアウト
// MH321800(E) G.So ICクレジット対応
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
#define		OPMON_TURIKAN_IRGCLR1		177					// 釣銭管理クリア異常(④現在集計の今回集計日時が0クリアされているか確認)
#define		OPMON_TURIKAN_IRGCLR2		178					// 釣銭管理クリア異常(⑤現在の追番、係員№、機械№が0クリア去れている場合はクリアキャンセル)
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
#define		OPMON_CRE_SEND_COMMAND		185					// クレジットコマンド送信
#define		OPMON_CRE_RECV_RESPONSE		186					// クレジットコマンド応答受信
#define		OPMON_CRE_COMMAND_TIMEOUT	187					// クレジットコマンドタイムアウト
#define		OPMON_SUICA_SETTLEMENT		250
// MH321800(S) G.So ICクレジット対応
#define		OPMON_EC_SETTLEMENT			252					// 電子マネー引去り結果電文受信
#define		OPMON_EC_CRE_SETTLEMENT		253					// クレジット決済結果電文受信
// MH321800(E) G.So ICクレジット対応
#define		OPMON_FLAPDOWN				510					// ﾌﾗｯﾌﾟ装置開
#define		OPMON_FLAPUP				511					// ﾌﾗｯﾌﾟ装置閉
#define		OPMON_LOCKDOWN				505					// ﾛｯｸ装置開
#define		OPMON_LOCKUP				506					// ﾛｯｸ装置閉
#define		OPMON_FLAP_MMODE_ON			515					// ﾌﾗｯﾌﾟ装置手動ﾓｰﾄﾞ発生
#define		OPMON_FLAP_MMODE_OFF		516					// ﾌﾗｯﾌﾟ装置手動ﾓｰﾄﾞ解除
#define		OPMON_LOCK_MMODE_ON			517					// ﾛｯｸ装置手動ﾓｰﾄﾞ発生
#define		OPMON_LOCK_MMODE_OFF		518					// ﾛｯｸ装置手動ﾓｰﾄﾞ解除
#define		OPMON_CAR_IN				520					// 入庫
#define		OPMON_CAR_OUT				521					// 出庫
#define		OPMON_LAG_EXT				504					// ラグタイム延長

// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
#define		OPMON_TEST					9901				// 調査用
#endif
// 調査用 (e)
// 端末機能番号
#define		RMON_FUNC_REMOTEDL					 1			// 遠隔メンテナンス機能

// 処理番号・状況番号
#define		RMON_PRG_DL_START_OK				 1000100		// ダウンロード開始正常
#define		RMON_PRG_DL_START_MNT_NG			 1000901		// メンテナンス中NG
#define		RMON_PRG_DL_START_RETRY_OVER		 1000201		// リトライオーバー
// GG120600(S) // Phase9 (一定時間経過)を追加
#define		RMON_PRG_DL_START_OVER_ELAPSED_TIME	 1000202		// 一定時間経過
// GG120600(E) // Phase9 (一定時間経過)を追加

#define		RMON_PRG_DL_END_OK					 1010100		// ダウンロード終了正常
#define		RMON_PRG_DL_END_COMM_NG				 1010201		// 通信異常
#define		RMON_PRG_DL_END_FILE_NG				 1010202		// プログラムファイルエラー
#define		RMON_PRG_DL_END_FLASH_WRITE_NG		 1010203		// Flash書込み失敗
#define		RMON_PRG_DL_END_PROG_FILE_NONE		 1010204		// プログラムファイルなし
#define		RMON_PRG_DL_END_SUM_NG				 1010205		// プログラムSUM値エラー
#define		RMON_PRG_DL_END_RETRY_OVER			 1010206		// リトライオーバー

#define		RMON_PRG_SW_START_OK				 2000100		// 更新開始正常
#define		RMON_PRG_SW_START_STATUS_NG			 2000901		// 状態NG
#define		RMON_PRG_SW_START_COMM_NG			 2000902		// 通信中
#define		RMON_PRG_SW_START_FLAP_NG			 2000903		// フラップ動作中
#define		RMON_PRG_SW_START_DOOR_OPEN_NG		 2000904		// ドア開
#define		RMON_PRG_SW_START_RETRY_OVER		 2000201		// リトライオーバー
// GG120600(S) // Phase9 (一定時間経過)を追加
#define		RMON_PRG_SW_START_OVER_ELAPSED_TIME	 2000202		// 一定時間経過
// GG120600(E) // Phase9 (一定時間経過)を追加

#define		RMON_PRG_SW_END_OK					 2010100		// 更新正常
#define		RMON_PRG_SW_END_BACKUP_NG			 2010201		// 全バックアップ失敗
#define		RMON_PRG_SW_END_PROG_FILE_NONE		 2010202		// 待機面にプログラムファイルなし
#define		RMON_PRG_SW_END_FLASH_WRITE_NG		 2010203		// プログラム書込み失敗
#define		RMON_PRG_SW_END_RESTORE_NG			 2010204		// リストア失敗
#define		RMON_PRG_SW_END_RESET_NG			 2010205		// 更新中にリセット
#define		RMON_PRG_SW_END_UNSENT_DATA_NG		 2010206		// 未送信データの送信失敗

#define		RMON_FTP_LOGIN_OK					 3000100		// ログイン正常
#define		RMON_FTP_LOGIN_ID_PW_NG				 3000201		// ID or PW不正
#define		RMON_FTP_LOGIN_TIMEOUT_NG			 3000202		// 接続タイムアウト
#define		RMON_FTP_FILE_TRANS_NG				 3000203		// ファイル転送失敗
#define		RMON_FTP_FILE_DEL_NG				 3000204		// 削除失敗
#define		RMON_FTP_LOGIN_RETRY_OVER			 3000205		// リトライオーバー

#define		RMON_FTP_LOGOUT_OK					 4010100		// ログアウト成功
#define		RMON_FTP_LOGOUT_NG					 4010200		// ログアウト失敗

#define		RMON_PRM_DL_START_OK				 5000100		// ダウンロード開始正常
#define		RMON_PRM_DL_START_MNT_NG			 5000901		// メンテナンス中NG
#define		RMON_PRM_DL_START_RETRY_OVER		 5000201		// リトライオーバー
// GG120600(S) // Phase9 (一定時間経過)を追加
#define		RMON_PRM_DL_START_OVER_ELAPSED_TIME	 5000202		// 一定時間経過
// GG120600(E) // Phase9 (一定時間経過)を追加
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_DL_START_CANCEL			 5000301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 設定変更通知対応

#define		RMON_PRM_DL_END_OK					 5010100		// ダウンロード終了正常
#define		RMON_PRM_DL_END_COMM_NG				 5010201		// 通信異常
#define		RMON_PRM_DL_END_FILE_NG				 5010202		// 設定ファイルエラー
#define		RMON_PRM_DL_END_FLASH_WRITE_NG		 5010203		// Flash書込み失敗
#define		RMON_PRM_DL_END_PARAM_FILE_NONE		 5010204		// 設定ファイルなし
#define		RMON_PRM_DL_END_RETRY_OVER			 5010205		// リトライオーバー
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_DL_END_CANCEL			     5010301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 設定変更通知対応

#define		RMON_PRM_SW_START_OK				 6000100		// 更新開始正常
#define		RMON_PRM_SW_START_STATUS_NG			 6000901		// 状態NG
#define		RMON_PRM_SW_START_COMM_NG			 6000902		// 通信中
#define		RMON_PRM_SW_START_FLAP_NG			 6000903		// フラップ動作中
#define		RMON_PRM_SW_START_DOOR_OPEN_NG		 6000904		// ドア開
#define		RMON_PRM_SW_START_RETRY_OVER		 6000201		// リトライオーバー
// GG120600(S) // Phase9 (一定時間経過)を追加
#define		RMON_PRM_SW_START_OVER_ELAPSED_TIME	 6000202		// 一定時間経過
// GG120600(E) // Phase9 (一定時間経過)を追加
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_SW_START_CANCEL			 6000301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 設定変更通知対応

#define		RMON_PRM_SW_END_OK					 6010100		// 更新正常
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_SW_END_OK_MNT				 6010101		// 更新正常(端末で設定パラメータ変更)
// GG120600(E) // Phase9 設定変更通知対応
#define		RMON_PRM_SW_END_PARAM_FILE_NONE		 6010201		// 待機面に設定ファイルなし
#define		RMON_PRM_SW_END_FLASH_WRITE_NG		 6010202		// 設定書込み失敗
#define		RMON_PRM_SW_END_RESET_NG			 6010203		// 更新中にリセット
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_SW_END_CANCEL				 6010301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 設定変更通知対応

#define		RMON_PRM_UP_START_OK				 7000100		// 設定要求開始正常
#define		RMON_PRM_UP_START_MNT_NG			 7000901		// メンテナンス中NG
#define		RMON_PRM_UP_START_RETRY_OVER		 7000201		// リトライオーバー
// GG120600(S) // Phase9 (一定時間経過)を追加
#define		RMON_PRM_UP_START_OVER_ELAPSED_TIME	 7000202		// 一定時間経過
// GG120600(E) // Phase9 (一定時間経過)を追加
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_UP_START_CANCEL			 7000301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 設定変更通知対応

#define		RMON_PRM_UP_END_OK					 7010100		// 設定要求終了正常
#define		RMON_PRM_UP_END_COMM_NG				 7010201		// 通信異常
#define		RMON_PRM_UP_END_UNCOMP_NG			 7010202		// 圧縮失敗
#define		RMON_PRM_UP_END_RETRY_OVER			 7010203		// リトライオーバー
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_UP_END_CANCEL				 7010301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 設定変更通知対応

#define		RMON_RESET_START_1MIN_BEFORE		 8000101		// 1分前通知
#define		RMON_RESET_START_OK					 8000100		// リセット開始正常
#define		RMON_RESET_START_STATUS_NG			 8000901		// 状態NG
#define		RMON_RESET_START_COMM_NG			 8000902		// 通信中
#define		RMON_RESET_START_FLAP_NG			 8000903		// フラップ動作中
#define		RMON_RESET_START_DOOR_OPEN_NG		 8000904		// ドア開
#define		RMON_RESET_START_RETRY_OVER			 8000201		// リトライオーバー
// GG120600(S) // Phase9 (一定時間経過)を追加
#define		RMON_RESET_START_OVER_ELAPSED_TIME	 8000202		// 一定時間経過
// GG120600(E) // Phase9 (一定時間経過)を追加

#define		RMON_RESET_END_OK					 8010100		// リセット終了正常
	
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)
#define		RMON_PRM_DL_DIF_START_OK			 9000100		// ダウンロード開始正常
#define		RMON_PRM_DL_DIF_START_MNT_NG		 9000901		// メンテナンス中NG
#define		RMON_PRM_DL_DIF_START_RETRY_OVER	 9000201		// リトライオーバー
// GG120600(S) // Phase9 (一定時間経過)を追加
#define		RMON_PRM_DL_DIF_START_OVER_ELAPSED_TIME	 9000202		// 一定時間経過
// GG120600(E) // Phase9 (一定時間経過)を追加
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_DL_DIF_START_CANCEL		 9000301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 設定変更通知対応

#define		RMON_PRM_DL_DIF_END_OK				 9010100		// ダウンロード終了正常
#define		RMON_PRM_DL_DIF_END_COMM_NG			 9010201		// 通信異常
#define		RMON_PRM_DL_DIF_END_FILE_NG			 9010202		// 設定ファイルエラー
#define		RMON_PRM_DL_DIF_END_FLASH_WRITE_NG	 9010203		// Flash書込み失敗
#define		RMON_PRM_DL_DIF_END_PARAM_FILE_NONE	 9010204		// 設定ファイルなし
#define		RMON_PRM_DL_DIF_END_RETRY_OVER		 9010205		// リトライオーバー
// GG120600(S) // Phase9 設定変更通知対応
#define		RMON_PRM_DL_DIF_END_CANCEL			 9010301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 設定変更通知対応

#define		RMON_PRM_SW_DIF_START_OK			10000100		// 更新開始正常
#define		RMON_PRM_SW_DIF_START_STATUS_NG		10000901		// 状態NG
#define		RMON_PRM_SW_DIF_START_COMM_NG		10000902		// 通信中
#define		RMON_PRM_SW_DIF_START_FLAP_NG		10000903		// フラップ動作中
#define		RMON_PRM_SW_DIF_START_DOOR_OPEN_NG	10000904		// ドア開
#define		RMON_PRM_SW_DIF_START_RETRY_OVER	10000201		// リトライオーバー
// GG120600(S) // Phase9 (一定時間経過)を追加
#define		RMON_PRM_SW_DIF_START_OVER_ELAPSED_TIME	 10000202		// 一定時間経過
// GG120600(E) // Phase9 (一定時間経過)を追加
// GG120600(S) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている
#define		RMON_PRM_SW_DIF_START_CANCEL		10000301		// [キャンセル]（端末で設定パラメータ変更）
// GG120600(E) // Phase9 #5078 【連動評価指摘事項】端末側でパラメータ変更後に送信される遠隔監視データの処理番号が間違っている

#define		RMON_PRM_SW_DIF_END_OK				10010100		// 更新正常
#define		RMON_PRM_SW_DIF_END_PARAM_FILE_NONE	10010201		// 待機面に設定ファイルなし
#define		RMON_PRM_SW_DIF_END_FLASH_WRITE_NG	10010202		// 設定書込み失敗
#define		RMON_PRM_SW_DIF_END_RESET_NG		10010203		// 更新中にリセット
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(遠隔ダウンロード)

#define		RMON_PRG_DL_REQ_OK					20000100		// 予約登録成功
#define		RMON_PRG_DL_REQ_NG					20000200		// 予約登録失敗
#define		RMON_PRG_DL_REQ_PRGNO_NG			20000201		// プログラム部番異常
#define		RMON_PRG_DL_CANCEL_OK				20010100		// 中止成功
#define		RMON_PRG_DL_CANCEL_NG				20010200		// 中止失敗
#define		RMON_PRG_DL_CHECK_OK				20020100		// 予約確認成功
#define		RMON_PRG_DL_CHECK_NG				20020200		// 予約確認失敗
#define		RMON_PRG_DL_REQ_RT_OK				20030100		// 即時実行登録成功
#define		RMON_PRG_DL_REQ_RT_NG				20030200		// 即時実行登録失敗
#define		RMON_PRG_DL_REQ_RT_PRGNO_NG			20030201		// プログラム部番異常

#define		RMON_PRM_DL_REQ_OK					20040100		// 予約登録成功
#define		RMON_PRM_DL_REQ_NG					20040200		// 予約登録失敗
#define		RMON_PRM_DL_CANCEL_OK				20050100		// 中止成功
#define		RMON_PRM_DL_CANCEL_NG				20050200		// 中止失敗
#define		RMON_PRM_DL_CHECK_OK				20060100		// 予約確認成功
#define		RMON_PRM_DL_CHECK_NG				20060200		// 予約確認失敗
#define		RMON_PRM_DL_REQ_RT_OK				20070100		// 即時実行登録成功
#define		RMON_PRM_DL_REQ_RT_NG				20070200		// 即時実行登録失敗

#define		RMON_PRM_UP_REQ_OK					20080100		// 予約登録成功
#define		RMON_PRM_UP_REQ_NG					20080200		// 予約登録失敗
#define		RMON_PRM_UP_CANCEL_OK				20090100		// 中止成功
#define		RMON_PRM_UP_CANCEL_NG				20090200		// 中止失敗
#define		RMON_PRM_UP_CHECK_OK				20100100		// 予約確認成功
#define		RMON_PRM_UP_CHECK_NG				20100200		// 予約確認失敗
#define		RMON_PRM_UP_REQ_RT_OK				20110100		// 即時実行登録成功
#define		RMON_PRM_UP_REQ_RT_NG				20110200		// 即時実行登録失敗

#define		RMON_RESET_REQ_OK					20120100		// 予約登録成功
#define		RMON_RESET_REQ_NG					20120200		// 予約登録失敗
#define		RMON_RESET_CANCEL_OK				20130100		// 中止成功
#define		RMON_RESET_CANCEL_NG				20130200		// 中止失敗
#define		RMON_RESET_CHECK_OK					20140100		// 予約確認成功
#define		RMON_RESET_CHECK_NG					20140200		// 予約確認失敗
#define		RMON_RESET_REQ_RT_OK				20150100		// 即時実行登録成功
#define		RMON_RESET_REQ_RT_NG				20150200		// 即時実行登録失敗

#define		RMON_PRG_SW_REQ_OK					20160100		// 予約登録成功
#define		RMON_PRG_SW_REQ_NG					20160200		// 予約登録失敗
#define		RMON_PRG_SW_REQ_PRG_NONE_NG			20160201		// プログラムデータなし
#define		RMON_PRG_SW_REQ_PRGNO_NG			20160202		// 部番不一致
#define		RMON_PRG_SW_CANCEL_OK				20170100		// 中止成功
#define		RMON_PRG_SW_CANCEL_NG				20170200		// 中止失敗
#define		RMON_PRG_SW_CHECK_OK				20180100		// 予約確認成功
#define		RMON_PRG_SW_CHECK_NG				20180200		// 予約確認失敗
#define		RMON_PRG_SW_REQ_RT_OK				20190100		// 即時実行登録成功
#define		RMON_PRG_SW_REQ_RT_NG				20190200		// 即時実行登録失敗
#define		RMON_PRG_SW_REQ_RT_PRG_NONE_NG		20190201		// プログラムデータなし
#define		RMON_PRG_SW_REQ_RT_PRGNO_NG			20190202		// 部番不一致

#define		RMON_FTP_CHG_OK						20200100		// 受付成功
#define		RMON_FTP_CHG_NG						20200200		// 受付不可

#define		RMON_FTP_TEST_RES_OK				20210100		// テスト成功
#define		RMON_FTP_TEST_RES_NG				20210200		// テスト失敗
#define		RMON_FTP_TEST_NG					20210201		// 受付不可

#define		RMON_ILLEGAL_REQ_KIND				20220200		// 不正要求種別
#define		RMON_ILLEGAL_PROC_KIND				20230200		// 不正処理種別

// MH810100(S) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない
#define		RMON_PRM_DIF_DL_REQ_OK				20240100		// 予約登録成功
#define		RMON_PRM_DIF_DL_REQ_NG				20240200		// 予約登録失敗
#define		RMON_PRM_DIF_DL_CANCEL_OK			20250100		// 中止成功
#define		RMON_PRM_DIF_DL_CANCEL_NG			20250200		// 中止失敗
#define		RMON_PRM_DIF_DL_CHECK_OK			20260100		// 予約確認成功
#define		RMON_PRM_DIF_DL_CHECK_NG			20260200		// 予約確認失敗
#define		RMON_PRM_DIF_DL_REQ_RT_OK			20270100		// 即時実行登録成功
#define		RMON_PRM_DIF_DL_REQ_RT_NG			20270200		// 即時実行登録失敗
// MH810100(E) S.Takahashi 2020/05/13 車番チケットレス #4162 差分設定予約確認のダウンロード日時、更新日時が取得できない

//-------------------------------
// バックアップ／リストア関連定義
//-------------------------------

/*** データ種別定義 ***/
enum {
	OPE_DTNUM_COMPARA = 0,									// 共通パラメータ
	OPE_DTNUM_PERPARA,										// 個別パラメータ
	OPE_DTNUM_LOCKINFO,										// 車室設定
	OPE_DTNUM_LOCKMAKER,									// ロック装置設定
	OPE_DTNUM_PCADPT,										// 定期テーブル
	OPE_DTNUM_FLAPDT,										// 入庫（駐車位置）情報
	OPE_DTNUM_MOVCNT,										// 動作カウント
	OPE_DTNUM_NTNET,										// NT-NETデータ
	OPE_DTNUM_COINSYU,										// コイン金庫集計
	OPE_DTNUM_NOTESYU,										// 紙幣金庫集計
	OPE_DTNUM_TURIKAN,										// 金銭管理
	OPE_DTNUM_SKY,											// 集計（T,GT,複数台）
	OPE_DTNUM_SKYBK,										// 前回集計
	OPE_DTNUM_LOKTL,										// 駐車位置別集計
	OPE_DTNUM_LOKTLBK,										// 前回駐車位置別集計
	OPE_DTNUM_CARCOUNT,										// 入出庫台数
	OPE_DTNUM_PRCADPT,										// 定期テーブル
	OPE_DTNUM_COIN_BK,										// コイン金庫合計（前回分）
	OPE_DTNUM_NOTE_BK,										// 紙幣金庫合計（前回分）
	OPE_DTNUM_CRE_CAN,										// クレジット取消情報ログ
	OPE_DTNUM_CRE_URI,										// クレジット売上収集チェックエリア
	OPE_DTNUM_MIF_NG,										// Mifare書込み失敗ｶｰﾄﾞﾃﾞｰﾀ	(Gate)
	OPE_DTNUM_SUICA_ERR_LOG,								// Suica異常通信ログ
	OPE_DTNUM_LOG_DCSYUUKEI,								// Edy,Suica集計情報
	OPE_DTNUM_ATEINV_LOG,									// 無効係員ﾃﾞｰﾀ
	OPE_DTNUM_IO_LOG,										// 入出庫ログ
	OPE_DTNUM_BKLOCK_INFO,									// 車室設定バックアップエリア（車室故障用）
	OPE_DTNUM_SETPRM_LOG,									// 設定更新履歴
	OPE_DTNUM_LONG_TERM_PARKING,							// 長期駐車データ前回発報時間
	OPE_DTNUM_CENTER_SEQNO,									// センター追番
	OPE_DTNUM_CREDIT_SLIPNO,								// クレジット端末処理通番
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	OPE_DTNUM_DATE_SYOUKEI,									//日付切替基準の小計(リアルタイムデータ用)
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// MH321800(S) G.So ICクレジット対応
	OPE_DTNUM_EDY_TERMINALNO,								// Edy上位端末ID
// MH810103 GG119202(S) 個別パラメータがリストアされない
//	OPE_DTNUM_EC_ALARM_TRADE_LOG,							// 決済リーダ アラーム取引ログ
	OPE_DTNUM_DUMMY,										// 後ろに移動するためダミー
// MH810103 GG119202(E) 個別パラメータがリストアされない
// MH321800(E) G.So ICクレジット対応
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
	OPE_DTNUM_DC_NET_SEQNO,									// DC-NET通信用センター追番
	OPE_DTNUM_REAL_SEQNO,									// リアルタイム通信用センター追番
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(バックアップ)
// MH810103 GG119202(S) 個別パラメータがリストアされない
	OPE_DTNUM_EC_ALARM_TRADE_LOG,							// 決済リーダ アラーム取引ログ
// MH810103 GG119202(E) 個別パラメータがリストアされない
// MH810103 GG119202(S) ブランドテーブルをバックアップ対象にする
	OPE_DTNUM_EC_BRAND_TBL,									// 決済リーダから受信したブランドテーブル
// MH810103 GG119202(E) ブランドテーブルをバックアップ対象にする
// MH810103 GG119202(S) 個別パラメータがリストアされない
	OPE_DTNUM_LOG_DCSYUUKEI_EC,								// SX,決済リーダ集計情報
// MH810103 GG119202(E) 個別パラメータがリストアされない
	OPE_DTNUM_MAX
};
extern	ulong	Ope_f_Sem_DataAccess;						// データアクセスセマフォーフラグ
															// bit毎に意味を持つ 1=使用中，0=未使用
															// bitとデータ種の関係は上記シンボルの通り
															// 例）b0=共通パラメータ

//------------------
// ｺｲﾝ関連定義
//------------------
#define		COIN_FULL			CPrmSS[S_MON][27]			// ｺｲﾝ金庫満杯ﾎﾟｲﾝﾄ
#define		COIN_NEAR_FULL		CPrmSS[S_MON][26]			// ｺｲﾝ金庫満杯予告ﾎﾟｲﾝﾄ
#define		NOTE_NEAR_FULL		CPrmSS[S_MON][28]			// 紙幣金庫満杯予告ﾎﾟｲﾝﾄ

#define		POINT_10			CPrmSS[S_MON][20]			// 10円玉ﾎﾟｲﾝﾄ数
#define		POINT_50			CPrmSS[S_MON][21]			// 50円玉ﾎﾟｲﾝﾄ数
#define		POINT_100			CPrmSS[S_MON][22]			// 100円玉ﾎﾟｲﾝﾄ数
#define		POINT_500   		CPrmSS[S_MON][23]			// 500円玉ﾎﾟｲﾝﾄ数

/* 画面表示用　精算媒体種別 */
#define		OPE_DISP_MEDIA_TYPE_SUICA		1
#define		OPE_DISP_MEDIA_TYPE_PASMO		2
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		OPE_DISP_MEDIA_TYPE_EDY			3
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		OPE_DISP_MEDIA_TYPE_ICOCA		4
#define		OPE_DISP_MEDIA_TYPE_eMONEY		5
#define		OPE_DISP_MEDIA_TYPE_ICCARD		10
// MH321800(S) hosoda ICクレジット対応
#define		OPE_DISP_MEDIA_TYPE_EC			11
// MH321800(E) hosoda ICクレジット対応
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define		EDY_USE_TIMER					6 // 500msﾀｲﾏｰ用(3秒)
//
//extern  const unsigned char EdyTestModeStr[][19];
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

//---------------------------------------
//	Macro
//---------------------------------------
/* 装置種別・運用形態など */
#define		MIFARE_CARD_DoesUse			( ((uchar)prm_get(COM_PRM, S_PAY, 25,1, 1)) == 1 && !card_use[USE_PAS] )		// 1=Mifare use & 定期券での精算を行っていない場合

#define		Is_SUICA_STYLE_OLD			((uchar)( prm_get(COM_PRM, S_SCA, 1, 1, 6) == 0 ))	// 旧動作指定判定（MH706305以前の動作）

#define		SUICA_MIRYO_AFTER_STOP		((uchar)( prm_get(COM_PRM, S_SCA, 5, 1, 6) >= 1 ))	// Suica処理未了発生後は使用不可とする設定
#define		SUICA_ONLY_STOP				((uchar)( prm_get(COM_PRM, S_SCA, 5, 1, 6) == 1 ))	// Suica処理未了発生後はSuicaのみ使用不可とする設定
#define		SUICA_CM_BV_RD_STOP			((uchar)( prm_get(COM_PRM, S_SCA, 5, 1, 6) == 2 ))	// Suica処理未了発生後はSuica/CM/BV/RDを使用不可とする設定
#define		OPE_SIG_DOOR_Is_OPEN			(1 == doorDat)							// 1= 前面扉開
#define		OPE_SIG_DOOR_and_NOBU_are_CLOSE	(!OPE_SIG_DOOR_Is_OPEN && !OPE_SIG_DOORNOBU_Is_OPEN)
#define		OPE_SIG_DOORNOBU_Is_OPEN		doornobuDat								// 1= 前面扉ドアノブ開
#define		OPE_SIG_OUT_DOORNOBU_LOCK(a)	(ExIOSignalwt( EXPORT_LOCK, (uchar)a ))	// ﾄﾞｱﾉﾌﾞﾛｯｸｿﾚﾉｲﾄﾞ制御（a:1=Lock, 0=UnLock）
#define		OPE_DOOR_KNOB_ALARM_START_TIMER		(ushort)(50 * prm_get(COM_PRM, S_PAY, 39, 3, 1))
#define		OPE_DOOR_KNOB_ALARM_STOP_TIMER		(ushort)(50 * prm_get(COM_PRM, S_PAY, 39, 3, 4))
#define		OPE_IS_EBABLE_MAG_LOCK		((uchar)((CPrmSS[S_PAY][17]) != 0))			// 1=ロックする 0=ロックしない
#define		DATE_CHK_ERR(y,m,d)	(( !y || !m || !d || chkdate( y, m, d ) != 0 ) ? 1 : 0)	// 1=Error
#define		OPE_Is_DOOR_ERRARM			((uchar)( prm_get(COM_PRM, S_CEN, 96, 1, 6) == 0 ))
// MH810100(S) Y.Watanabe 2020/02/13 車番チケットレス
#define		QR_READER_USE				((uchar)( prm_get(COM_PRM, S_PAY, 25, 1, 5) == 1))
// MH810100(E) Y.Watanabe 2020/02/13 車番チケットレス
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
//// MH810105(S) MH364301 インボイス対応 非対応
////// MH810105(S) MH364301 インボイス対応
////#define		IS_INVOICE					((uchar)( prm_get(COM_PRM, S_SYS, 16, 1, 6) == 0 ))
////// MH810105(E) MH364301 インボイス対応
//#define		IS_INVOICE					((uchar)( 0 ))
//// MH810105(E) MH364301 インボイス対応 非対応
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//#define		IS_INVOICE					((uchar)( prm_get(COM_PRM, S_SYS, 16, 1, 6) == 0 ))
#define		IS_INVOICE					(0)
// 01-0016①を参照する場合は下記のマクロを使用すること
#define		IS_INVOICE_PRM				((uchar)( prm_get(COM_PRM, S_SYS, 16, 1, 6) == 0 ))
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
// GG129004(S) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない
#define		IS_ERECEIPT					(prm_get(COM_PRM, S_RTP, 58, 1, 1) == 1)
// GG129004(E) R.Endo 2024/12/13 #7561 電子領収証を発行する設定でレシート紙を紙切れにすると、電子領収証が発行できない

//---------------------------------------
//	その他
//---------------------------------------
// MH810105(S) MH364301 インボイス対応
#define		PRIEND_PREQ_RYOUSYUU		(PREQ_RYOUSYUU | INNJI_ENDMASK)				// 領収書発行メッセージ
#define		JNL_PRI_WAIT_TIME			(3*2+1)										// ジャーナル印字完了待ちタイマ（3秒）（500ms単位）
// MH810105(E) MH364301 インボイス対応
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
#define		EJNL_PRI_WAIT_TIME			2											// 電子ジャーナル印字完了待ちタイマ（1秒）（500ms単位）
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）

//---------------------------------
// データSUM(現状共通パラメータと車室パラメータのみ使用)
//---------------------------------

typedef struct {
	ulong	Sum;
	ulong	Len;
}t_DataSum;
extern	t_DataSum	DataSum[OPE_DTNUM_MAX];

#define		FORCE_FULL		0x11	// 強制満車(本体)
#define		FORCE_VACANCY	0x21	// 強制空車(本体)
#define		NTNET_FULL		0x12	// 強制満車(通信)
#define		NTNET_VACANCY	0x22	// 強制空車(通信)
#define		AUTO_FULL		0x14	// 満車(自動)
#define		AUTO_VACANCY	0x24	// 空車(自動)
// MH322917(S) A.Iiizumi 2018/11/27 リアルタイム情報 フォーマットRev11対応
#define		DATE_URIAGE_NO_CHANGE		0
#define		DATE_URIAGE_CLR_TODAY		1
#define		DATE_URIAGE_CLR_BEFORE		2
#define		DATE_URIAGE_PRMCNG_TODAY	3
#define		DATE_URIAGE_PRMCNG_BEFORE	4

#define		DATE_URIAGE_PRMCNG_NOCLR	0
#define		DATE_URIAGE_PRMCNG_CLR		1
// MH322917(E) A.Iiizumi 2018/11/27 リアルタイム情報 フォーマットRev11対応
/*** function prototype ***/

/* opetask.c */
extern  void			opetask( void );
extern	void			mojule_wait( void );
extern	char			memclr( char );
extern	unsigned short	GetMessage( void );
extern	void			Ope_ErrAlmClear( void );
extern	void			auto_syuukei( void );
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	uchar			auto_centercomm( uchar execkind );
//extern	uchar			autocnt_execchk( uchar type );
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

extern	void			Ope_MsgBoxClear( void );
extern	void			Ope_InSigSts_Initial( void );

extern	void	ck_syuukei_ptr_zai( ushort LogKind );
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// アイドルサブタスクに移動
//-// MH322916(S) A.Iiizumi 2018/07/26 長期駐車検出機能対応
//extern	void	LongTermParkingCheck_Resend( void );
//-// MH322916(E) A.Iiizumi 2018/07/26 長期駐車検出機能対応
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH810104 GG119201(S) 電子ジャーナル対応
extern	void			EJA_TimeAdjust(void);
// MH810104 GG119201(E) 電子ジャーナル対応

#define	_AteGetDataTop()		(int)_offset(Attend_Invalid_table.wtp, _countof(Attend_Invalid_table.aino) - Attend_Invalid_table.count, _countof(Attend_Invalid_table.aino))
#define	_AteGetDataPos(ofs)		_offset(_AteGetDataTop(), ofs, _countof(Attend_Invalid_table.aino))
#define	_AteIdx2Ofs(idx)		_offset(Attend_Invalid_table.wtp, ADN_VAILD_MAX-Attend_Invalid_table.count+idx, ADN_VAILD_MAX)
#define	dispPos(top,i)			(((top+_AteGetDataPos(i)) > (ADN_VAILD_MAX-1)) ? ((top+_AteGetDataPos(i))-ADN_VAILD_MAX):(top+_AteGetDataPos(i)))
extern ushort	DoorLockTimer;
extern ulong	attend_no;							// Reparkｶｰﾄﾞから読み取った係員No
extern uchar	Repark_card_use;					// １：使用したｶｰﾄﾞがReparkｶｰﾄﾞ ０：別のｶｰﾄﾞ

/* logctrl.c */
extern	void			Log_regist( short );
extern	void			log_init( void );
extern	void			Log_Write(short Lno, void *dat, BOOL strage);
extern	void Log_Write_Pon(void);
extern	void Log_clear_log_bakupflag(void);
extern	void			Log_CheckBufferFull(BOOL occur, short Lno, short target);
extern	ushort			Ope_Log_TotalCountGet(short id);
extern	ushort			Ope_Log_UnreadCountGet(short id, short target);
extern	ushort			Ope_Log_UnreadToRead(short id, short target);
extern	ushort			Ope_Log_CheckNearFull(short id, short target);
extern	ushort			Ope_Log_GetNearFull( short id, short target );
extern	ushort			Ope_Log_GetNearFullCount( short id );
extern	uchar			Ope_Log_1DataGet(short id, ushort Index, void *pSetBuf);
extern	uchar			Ope_Log_TargetDataGet(short id, void *pSetBuf, short target, BOOL update);
extern	uchar			Ope_Log_TargetDataVoidRead(short id, void *pSetBuf, short target, BOOL start);
extern	void 			Ope_Log_UndoTargetDataVoidRead( short id, short target );
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// extern	uchar			Ope_Log_ShashitsuLogGet( SYUKEI *syukei, LCKTTL_LOG *lckttl );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
extern	void			Ope_Log_TargetReadPointerUpdate(short id, short target);
extern	ushort			Ope_Log_TargetVoidReadPointerUpdate(short id, short target);
extern	uchar			Log_SemGet( uchar DtNum );
extern	void			Log_SemFre( uchar DtNum );
extern	void			NgLog_write( ushort card_type, uchar *cardno, ushort cardno_len );
extern	void			IoLog_write( ushort event_type, ushort sub_type, ulong ticketnum, ulong flg );

/* Ledctrl.c */
extern	void			LedInit( void );
extern	void    		LedReq( unsigned char , unsigned char );
extern	unsigned char	IsLedReq( unsigned char );
extern	void			LedOnOffCtrl( void );

/* opemain.c */
extern	void			OpeMain( void );
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// 新規 起動中処理の追加
extern	short			op_init00( void );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern	short			op_mod00( void );
extern	short			op_mod01( void );
extern	short			op_mod02( void );
extern	short			op_mod03( void );
extern	short			op_mod11( void );
extern	short			op_mod12( void );
extern	short			op_mod13( void );
extern	short			op_mod14( void );
extern	short			op_mod21( void );
extern	short			op_mod22( void );
extern	short			op_mod23( void );
extern	short			carchk( uchar, ushort, uchar );
extern	short			carchk_ml( uchar, ushort );
extern	short			invcrd( ushort );
extern	short			in_mony( ushort, ushort );
extern	long			modoshi( void );
extern	short			set_tim( unsigned short, struct clk_rec *, short );
extern	short			set_tim2( struct clk_rec * );
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
extern	short			set_tim_Lost( ushort trig,ushort num, struct clk_rec *clk_para, short err_mode );
extern	short			set_tim_only_out( ushort num, struct clk_rec *clk_para, short err_mode );
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
extern	void			cl_7seg( char );
extern	void			Tim_7seg( void );
extern	char 			FlpSetChk( unsigned short );
extern	short			op_mod70( void );
extern	short			op_mod80( void );
extern	short			op_mod90( void );
extern	short			carduse( void );
extern	void			svs_renzoku( void );
extern	short			op_mod100( void );
extern	short			op_mod110( void );
extern	void			MntMain( void );
extern	void			op_indata_set( void );
extern	uchar			op_key_chek( void );
extern	void			Print_Condition_select( void );
extern	short			op_mod81( uchar );

extern	short			op_mod200( void );
extern	short			op_mod210( void );
extern	void			MakeVisitorCode(uchar *code);				//お客様ｺｰﾄﾞ作成
extern	ushort			CheckVisitorCode(uchar *code, ushort val);	//入力暗証番号(Btype)ﾁｪｯｸ
extern	short			op_mod220( void );	// ラグタイム延長処理
extern	short			op_mod230( void );	// 精算済み案内処理
// MH810100(S)
extern	void			SetPayStartTime( struct clk_rec *pTime );
extern	struct clk_rec*	GetPayStartTime( void );
// MH810100(E)

extern	char	save_op_faz;
extern	void	op_MifareStart( void );
extern	void	op_MifareStop( void );
extern	void	op_MifareStop_with_LED( void );

extern	void	ElectronSet_PayData( void *buf, uchar data_kind );
// MH321800(S) G.So ICクレジット対応
extern	void	EcElectronSet_PayData( void *buf );
extern	void	EcCreUpdatePayData( void *buf );
extern	void	EcElectronSet_DeemedData( void *buf );
// MH321800(E) G.So ICクレジット対応
// MH810103 GG119202(S) 電子マネーシングル設定で案内放送を行う
extern	void	op_EcEmoney_Anm(void);
// MH810103 GG119202(E) 電子マネーシングル設定で案内放送を行う
// MH810105(S) MH364301 QRコード決済対応
extern void		EcQrSet_PayData( void *buf );
extern void		Ec_Data152Save( void );
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 インボイス対応
extern	void	op_rct_failchk(void);
extern	void	op_jnl_failchk(void);
// MH810105(E) MH364301 インボイス対応

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern	void	Set_Pay_RTPay_Data( void );
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
extern	ushort	GetCardKind( void );
extern	void	Set_Pay_RTReceipt_Data( void );
extern	void	MakeQRCodeReceipt( char* data, size_t size );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
extern	void	Set_Cancel_RTPay_Data( void );
extern	BOOL	ope_SendCertifCommit( void );
extern	BOOL	ope_SendCertifCancel( void );
extern	ushort	op_wait_mnt_start( void );
extern	ushort	op_wait_mnt_close( void );
extern	ushort	lcdbm_alarm_check( void );
extern	void	lcdbm_notice_alm( ushort warning );
extern	void	lcdbm_ErrorReceiveProc( void );
extern	void	lcdbm_notice_ope( eOPE_NOTIF_CODE ope_code, ushort status );
extern	BOOL	ope_AddUseMedia( tMediaDetail* pMedia );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
extern	BOOL	ope_MakeLaneLog( ushort usStsNo );
extern	void	SetLaneMedia(ulong ulParkNo, ushort usMediaKind, uchar *pMediaNo);
extern	void	SetLaneFeeKind(ushort usFeeKind);
extern	void	SetLaneFreeNum(ulong ulNum);
extern	void	SetLaneFreeStr(uchar *pStr, uchar usStrSize);
extern	void	LaneStsSeqNoUpdate(void);
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129000(S) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
extern	BOOL	ope_MakeLaneLog_Check(uchar pay_sts);
// GG129000(E) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
// GG129000(S) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）
extern	uchar	QRIssueChk(uchar check_only);
extern	uchar	CarNumChk(void);
// GG129000(E) T.Nagai 2023/02/10 ゲート式車番チケットレスシステム対応（QR駐車券対応）


typedef union{
	unsigned char	BYTE;					// Byte
	struct{
		unsigned char   STOP_REASON:3;      // Bit 4-5 = 停止理由
											//     0：クレジット
											//     1：定期問合せ(磁気)
											//     3：定期問合せ(Mifare)
											//     5：精算完了時に電子媒体の停止を待ち合わせる
											//   6-7：予備
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		unsigned char	YOBI:1;				// Bit 4 = 予備
//		unsigned char	EDY_END:1;			// Bit 3 = Edy終了
		unsigned char	YOBI:2;				// Bit 3-4 = 予備
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		unsigned char	SUICA_END:1;		// Bit 2 = Suica終了
		unsigned char	CMN_END:1;			// Bit 1 = ﾒｯｸ終了
		unsigned char	INITIALIZE:1;		// Bit 0 = 初期状態
	} BIT;
} t_End_Status;

extern	t_End_Status	cansel_status;				// 精算中止用Edy・Suica・cmn終了待ち
extern	t_End_Status	CCT_Cansel_Status;			// ｸﾚｼﾞｯﾄｶｰﾄﾞ使用時のEdy・Suica停止待ち用
enum{
	REASON_PAY_END = 1,
	REASON_CARD_READ,
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// QRﾃﾞｰﾀ受信時にICｸﾚｼﾞｯﾄ停止したいという理由を新規追加
	REASON_QR_USE,
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
	REASON_MIF_READ,
	REASON_MIF_WRITE_LOSS,
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
	REASON_RTM_REMOTE_PAY,					// 遠隔精算開始
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
};

extern	uchar	Ope_Last_Use_Card;			// 次のカードのうち最後に使用したもの
											// 1=プリペ，2=回数券，3=電子決済カード
											// 0=上記まだ未使用

extern	short	tim1_mov;					// ﾀｲﾏｰ1起動ﾌﾗｸﾞ

extern	uchar	Chu_Syu_Status;				// 中止集計実施ｽﾃｰﾀｽ 0：未実施 1：実施済み
extern	Receipt_data	PayInfoData_SK;		// 精算情報データ用強制・不正出庫データ
extern	uchar	PayInfo_Class;				// 精算情報データ用処理区分

extern	uchar	SyuseiEndKeyChk( void );
extern void StackCardEject(uchar req);
extern uchar CardStackStatus;			// カード詰まり処理状態
extern uchar CardStackRetry;					// 排出動作中ﾌﾗｸﾞ
extern	char	Op_Event_Disable( ushort msg );
extern	char	Op_Event_enable( void );
// MH321800(S) T.Nagai ICクレジット対応
extern	uchar	coin_err_flg;
extern	uchar	note_err_flg;
// MH321800(E) T.Nagai ICクレジット対応
// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
extern const unsigned char EcBrandEmoney_Digit[];
// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう

/* fus_kyo.c */
extern	short			fus_kyo( void );

/* tokubet.c */
extern	char			tokubet( short, short, short );
extern	const uchar	Shunbun_Day[100];
extern  const uchar	Shuubun_Day[100];
extern  const uchar	MAY_6[100];

/* syuukei.c */
extern	void			kan_syuu( void );
extern	char			DoBunruiSyu( char Syubetu );
extern	void			cyu_syuu( void );
extern	void			mulkan_syu( ushort );
extern	void			mulchu_syu( ushort );
extern	short			fus_syuu( void );
extern	void			syu_syuu( void );
extern	void			lto_syuu(void);
extern	void			lto_syuko( void );
extern	void			toty_syu( void );
extern	SYUKEI			*syuukei_prn( int preq, SYUKEI *src );
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
extern	char			Check_syuukei_clr( char no );
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
extern	void			syuukei_clr( char );
extern	void			kinko_syu( char, char );
extern	void			kinko_clr( char );
extern	void			syu_init( void );
extern	void			disc_wari_conv( DISCOUNT_DATA *, wari_tiket * );
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
extern	void			disc_wari_conv_prev( DISCOUNT_DATA *, wari_tiket * );
extern	void			disc_wari_conv_all( DISCOUNT_DATA *, wari_tiket * );
// GG124100(S) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
extern	void			disc_wari_conv_new( DISCOUNT_DATA *, wari_tiket * );
// GG124100(E) R.Endo 2022/09/30 車番チケットレス3.0 #6631 再精算時に、前回精算で使用した割引の追加の割引分が集計に反映されない
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
extern	void			disc_Detail_conv( DETAIL_DATA *, wari_tiket * );
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
extern	uchar	date_uriage_use_chk( void );
extern	void	date_uriage_syoukei_judge( void );
extern	uchar	date_uriage_update_chk( void );
extern	void	date_uriage_syoukei_clr( uchar flg);
extern	void	date_uriage_prmcng_judge( void );
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//// MH321800(S) G.So ICクレジット対応
//extern	void			ec_alarm_syuu( uchar brand, ulong ryo );
//// MH321800(E) G.So ICクレジット対応
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
// MH810105 GG119202(S) T合計連動印字対応
extern	void			ec_linked_total_print(ushort pri_req, T_FrmSyuukei *pFrmSyuukei);
// MH810105 GG119202(E) T合計連動印字対応

/* turikan.c */
extern	void			turikan_proc( short );
extern	void			turikan_gen( void );
extern	void			turikan_pay( void );
extern	void			turikan_ini( void );
extern	void			turikan_prn( short );
extern	void			turikan_sfv( void );
// GG129001(S) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
extern	char			Check_turikan_clr( void );
// GG129001(E) T合計印字がマイナスにならないよう対策(共通改善No.1604)（MH364304流用）
extern	void			turikan_clr( void );
extern	void			turikan_clr_sub2( void );
extern	void			turikan_clr_sub( void );
extern	void			turikan_inventry( short );
extern	void			turikan_fuk( void );
extern	void			turiadd_hojyu_safe(void);
extern	void			turikan_subtube_set(void);

/* fukuden.c */
extern	void			fukuden( void );
extern	void			flp_infuku( void );

/* SystemMnt.c */
extern	void			SysMntMain( void );

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// sysmnt2.c
extern int ftp_auto_update_for_LCD( void );		// LCDﾓｼﾞｭｰﾙに対するﾊﾟﾗﾒｰﾀｱｯﾌﾟﾛｰﾄﾞ
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

/* UserMnt.c */
extern	void			UserMntMain( void );
// MH810100(S) Y.Yamauchi 20191212 車番チケットレス(遠隔ダウンロード)
//extern	void 			CardErrDsp( void );
extern	void 			CardErrDsp( short err );
extern	void			push_ticket( tMediaDetail *pMedia, ushort err );
// MH810100(S) Y.Yamauchi 20191212 車番チケットレス(遠隔ダウンロード)
/* shtctrl.c */
extern	void			shtctrl( void );
extern	void			rd_shutter( void );
extern	void			read_sht_opn( void );
extern	void			read_sht_cls( void );
extern	void			TpuReadShut( void );
extern	void			TpuCoinShut( void );
extern	void			shutter_err_chk( uchar * );
extern	void			start_rdsht_cls( void );

#define	RD_ERR_INTERVAL		(50*10)			// 10秒（ｼｬｯﾀｰ異常状態監視間隔）
#define	RD_SOLENOID_WAIT	(50*1)			// 1秒（ｼｬｯﾀｰｿﾚﾉｲﾄﾞ動作待ちﾀｲﾏｰ）
#define	RD_ERR_COUNT_MAX	3				// 3回（ｼｬｯﾀｰ異常状態監視回数）

/* SubCpuMnt.c */
extern	void			SubCpuMnt( void );

/* DspTask.c */
extern	void			DspTask( void );

/* opered.c */
extern	void			red_int( void );
extern	void			opr_ctl( ushort );
extern	void			opr_rcv( ushort );
extern	short			opr_snd( short );
extern	void			opr_int( void );
extern	short			al_svsck( m_gtservic * );
extern	ushort			CardSyuCntChk( ushort, short, short, short, short );
extern	short			al_pasck( m_gtapspas * );
extern	short			al_pasck_renewal( m_apspas * );
extern	void			al_mkpas_renewal( m_apspas *, struct clk_rec * );
extern	short			al_preck( m_gtprepid * );
extern	short			al_kasck( m_gtservic *mag );
extern	void			al_mkpas( m_gtapspas *, struct clk_rec * );
extern	void			al_mkpre( m_gtprepid *, pre_rec * );
extern	void			al_mkkas( m_gtkaisuutik *inpp, svs_rec *pre, struct clk_rec *ck );

extern	uchar			Read_Tik_Chk( uchar *, uchar *, uchar );
extern	ushort			PkNo_get( uchar *, uchar );
extern	void			NgPkNo_set( uchar *, uchar );
extern	uchar chk_for_inquiry(uchar type);
extern	uchar	Ope_PasKigenCheck( ushort StartKigen_ndat, ushort EndKigen_ndat, short TeikiSyu, 
						   ushort Target_ndat, ushort Target_nmin );

extern	void			al_pasck_set( void );
extern	short			al_pasck_syusei( void );

extern	uchar GT_Settei_flg;					// GTﾌｫｰﾏｯﾄ設定NGﾌﾗｸﾞ

/* opesub.c */
extern	short			al_card( ushort, ushort );
extern	short			al_card_renewal( ushort, ushort );
extern	void			date_renewal( ushort, ushort *, uchar *, uchar * );
extern	char			LprnErrChk( char );
extern	void			LabelCountDown( void );
extern	short			hojuu_card( void );
extern	short			al_iccard( ushort, ushort );
extern	void			PayData_set( uchar, uchar );
extern	void			SetAddPayData( void );
extern	void			PayData_set_SK( ushort, ushort, ushort, ushort, uchar );
extern	void			PayData_set_LO( ushort, ushort, ushort, ushort);
extern	short			Kakariin_card( void );
extern	void			Ope_KakariCardInfoGet( uchar *pMntLevel, uchar* pPassLevel, uchar *pKakariNum );
extern	void			ryo_isu( char );
extern	uchar			ReceiptChk( void );
extern	void			chu_isu( void );
extern	void			azukari_isu(uchar cancel);
extern	char			uke_isu( ulong, ushort, uchar );
extern	char			pri_time_chk( unsigned short pr_no, uchar type );
extern	char			mtpass_get( uchar, uchar* );
extern	char			PassExitTimeTblRead( ulong, ushort, date_time_rec * );
extern	void			PassExitTimeTblWrite( ulong, ushort, date_time_rec * );
extern	void			PassExitTimeTblDelete( ulong, ushort );
extern	short			opncls( void );
extern	void			PayEndSig( void );
extern	void			CountUp_Individual( uchar kind );
extern	ulong			CountRead_Individual( uchar kind );
extern	PAS_TBL	*GetPassData( ulong ParkingNo, ushort PassId);
extern	void			WritePassTbl( ulong ParkingNo, ushort PassId, ushort Status, ulong UseParking, ushort Valid );
extern	void			GetPassArea( ulong ParkingNo, ushort *Start, ushort *End);
extern	void			FillPassTbl( ulong ParkingNo, ushort Status, ulong UseParking, ushort Valid, ushort FillType );
extern	short			ReadPassTbl( ulong ParkingNo, ushort PassId, ushort *PassData );
extern	uchar			ck_jis_credit ( uchar , char * );

extern	PAS_RENEWAL    *GetPassRenewalData( ulong, ushort, char * );
extern	short			ReadPassRenewalTbl( ulong, ushort, ushort * );
extern	void			WritePassRenewalTbl( ulong, ushort, ushort );
extern	short			GetPassRenewalArea( ulong, ushort *, ushort *, ushort *, ushort * );
extern	void			FillPassRenewalTbl( ulong, uchar );

extern	void			Ope2_Log_NewestOldestDateGet( ushort LogSyu, date_time_rec *NewestDate, date_time_rec *OldestDate, ushort *RegCount );
extern	unsigned long	Nrm_YMDHM( date_time_rec *wk_CLK_REC );
extern	void			UnNrm_YMDHM( date_time_rec *wk_CLK_REC, ulong Nrm_YMDHM );
extern	ushort			Ope2_Log_CountGet_inDate( ushort LogSyu, ushort *Date, ushort *id );
extern	BOOL			Ope2_Log_Get_inDate( ushort LogSyu, ulong Nrm_Date, ushort *id, void *pBuf );
extern	ushort			Ope2_Log_CountGet( ushort LogSyu );
extern	void			DiditalCashe_NewestOldestDateGet( ushort LogSyu, date_time_rec *NewestDate, date_time_rec *OldestDate, ushort *RegCount );
extern	short			chkdate2(short yyyy, short mm, short dd);
extern	ulong 			time_nrmlz ( ushort norm_day, ushort norm_min );
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
extern	uchar			Ope_FunCheck( char f_Button );
extern	void			Ope_Set_tyudata( void );
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
extern	void			Ope_Set_tyudata_Card( void );
extern	ulong			DC_GetCenterSeqNo( DC_SEQNO_TYPE type );
extern	void			DC_UpdateCenterSeqNo( DC_SEQNO_TYPE type );
extern	void			DC_PushCenterSeqNo( DC_SEQNO_TYPE type );
extern	void			DC_PopCenterSeqNo( DC_SEQNO_TYPE type );
extern	void			RTPay_LogRegist_AddOiban(void);			// ﾘｱﾙﾀｲﾑ精算ﾃﾞｰﾀのｾﾝﾀｰ追番を更新してlogに書き込む処理
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
extern	void			RTReceipt_LogRegist_AddOiban(void);
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// MH810100(S) 2020/09/02 再精算時の複数枚対応
//// MH810100(S) Y.Watanabe2020/02/12 車番チケットレス(割引済み対応)
//extern	ulong			Ope_GetLastDisc( void );
//// MH810100(E) Y.Watanabe 2020/02/12 車番チケットレス(割引済み対応)
extern	ulong			Ope_GetLastDisc( long lDiscount );
// MH810100(E) 2020/09/02 再精算時の複数枚対応
// MH810100(S) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
extern	ulong			Ope_GetLastDiscOrg();
// MH810100(E) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
extern BOOL Ope_SyubetuWariCheck(DISCOUNT_DATA* pDisc);
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）

// MH810100(S) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
extern	void			CheckShoppingDisc( ulong pno, ushort *p_kind, ushort mno, ushort *p_info );
// MH810100(E) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)

// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//extern	void			LongTermParkingRel( ulong LockNo );
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
// MH810100(S) 2020/05/27 車番チケットレス(#4181)
extern void				azukari_popup(uchar cancel);
// MH810100(E) 2020/05/27 車番チケットレス(#4181)
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
extern void				pri_ryo_stock(uchar kind);
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
// GG129004(S) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）
extern void				QrReciptDisp( void );
// GG129004(E) R.Endo 2024/12/04 電子領収証対応（QRコード発行案内表示タイマー）

#define PASS_DATA_FILL_ALL			0
#define PASS_DATA_FILL_VALID		1
#define PASS_DATA_FILL_IN_FREE		2
#define PASS_DATA_FILL_OUT_FREE		3

extern	char			AteVaild_Update(ushort id, ushort status);
extern	short			AteVaild_Check(ushort id);
extern	char			MAFParamUpdate(ushort seg, ushort add, long data);
extern	void			Ope2_WarningDisp( short time, const uchar *pMessage );
extern	void			Ope2_WarningDispEnd( void );
// MH321800(S) G.So ICクレジット対応
typedef struct {			// ワーニング サイクリック表示管理構造体
	short	interval;
	uchar	page;			// 0:未使用、1～:表示ページ
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
	uchar	mode;			// 0:通常、1:反転
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
	uchar	err_page[2][2];
} t_CyclicDisp;
// MH810105 GG119202(S) 未了再タッチ待ちメッセージ表示対応
//extern	void			Ope2_ErrChrCyclicDispStart( short interval, const uchar err_num[] );
extern	void			Ope2_ErrChrCyclicDispStart( short interval, const uchar err_num[], uchar mode );
// MH810105 GG119202(E) 未了再タッチ待ちメッセージ表示対応
extern	void			Ope2_ErrChrCyclicDispStop( void );
extern	void			Ope2_ErrChrCyclicDisp( void );
// MH321800(E) G.So ICクレジット対応
extern	void			AteVaild_table_update( ushort *data, ushort size);
extern	void			SetNoPayData( void );
extern	int				subtube_use_check( int mny_type );
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
extern	void	EnableNyukin(void);
extern	void	BrandResultUnselected(void);
extern	ushort	GetBrandReasonCode(ushort msg);
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810105(S) MH364301 QRコード決済対応
extern	void			EcFailureContactPri( void );
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 インボイス対応
extern	long			invoice_prt_param_check( long param_jadge );
// MH810105(E) MH364301 インボイス対応
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
extern	uchar			cancelReceipt_chk( void );
extern	uchar			cancelReceipt_Waridata_chk( wari_tiket* );
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応）
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
extern	void			setting_taxableAdd( ulong*, uchar );
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/課税対象サービス券／店割引利用額の売上加算設定にパターンを追加する）
// GG129001(S) 領収証プリント設定の読み替え対応
extern	ulong			Billingcalculation(Receipt_data *data);
// GG129001(E) 領収証プリント設定の読み替え対応
// GG129001(S) データ保管サービス対応（課税対象額をセットする）
extern	ulong			TaxAmountcalculation(Receipt_data *data , uchar typ);
// GG129001(E) データ保管サービス対応（課税対象額をセットする）

/* Lcdctrl.c */
extern	void			opedsp( ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort );
extern	void			opedsp3( ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort );
extern	void			opedpl( ushort, ushort, ulong, ushort, ushort, ushort, ushort, ushort );
extern	void			opedpl3( ushort, ushort, ulong, ushort, ushort, ushort, ushort, ushort );
extern	void			opedpl2( ushort, ushort, ulong, ushort, ushort, ushort, ushort, ushort );
extern	void			opedsp5( ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort);
extern	void			opedsp6( ushort, ushort, ushort, ushort, ushort, ushort, ushort );
extern	void			pntchr( uchar *, uchar );
extern	void			as1chg( const uchar *, uchar *, uchar );
extern	void			wekchg( uchar, uchar * );
extern	void			dsp_background_color( ushort rgb );
extern	void			dsp_intime( ushort, ushort );
extern	void			zrschr( uchar *, uchar );
extern	ushort			teninb( ushort, ushort, ushort, ushort, ushort );
extern	ushort			teninb_pass( ushort, ushort, ushort, ushort, uchar, ushort );
extern	void			feedsp( ushort, ulong );
extern	void			LcdBackLightCtrl( char );
extern	void			OpeLcd( ushort );
extern	ushort			blindpasword( ushort, ushort, ushort, ushort ,uchar);
extern	void			MagReadErrCodeDisp(uchar ercd);
extern	ulong			teninb2( ulong, ushort, ushort, ushort, ushort );
extern	void			Lcd_Receipt_disp(void);
extern	ushort			Ope_Disp_LokNo_GetFirstCol(void);
// MH810100(S) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)
extern	void			clear_pay_status( void );
// MH810100(E) K.Onodera 2020/04/10 #4008 車番チケットレス(領収証印字中の用紙切れ動作)

/* werrlg.c */
extern	void			err_chk( char, char, char, char, char );
extern	void			err_chk2( char md, char no, char knd, char f_data, char err_ctl, void *pData );
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:222)対応
//extern	void			err_chk3( char md, char no, char knd, char f_data, char err_ctl, void *pData, date_time_rec* date );
extern	void			err_chk3( char md, char no, char knd, char f_data, char err_ctl, void *pData, unsigned int length, date_time_rec* date );
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:222)対応
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern	void			alm_chk( char, char, char );
extern	void			alm_chk2( char md, char no, char knd, char f_data, char err_ctl, void *pData );
// MH321800(S) G.So ICクレジット対応
extern	void			err_ec_chk(unsigned char *NEW, unsigned char *OLD);
// MH321800(E) G.So ICクレジット対応
// MH321800(S) 切り離しエラー発生後の待機状態でJVMAリセットを行う
extern	void			err_ec_clear(void);
// MH321800(E) 切り離しエラー発生後の待機状態でJVMAリセットを行う
// MH810103 GG119202(S) 異常データフォーマット変更
extern	void			err_ec_chk2(unsigned char *NEW, unsigned char *OLD);
// MH810103 GG119202(E) 異常データフォーマット変更
extern	void			err_cn_chk( uchar *, uchar * );
extern	char			err_cnm_chk(void);
extern	void			err_nt_chk( uchar *, uchar * );
extern	void			err_pr_chk( uchar, uchar );
extern	void			err_pr_chk2( uchar, uchar );
// MH810104 GG119201(S) 電子ジャーナル対応
extern	void			err_eja_chk(uchar NEW, uchar OLD);
// MH810104 GG119201(E) 電子ジャーナル対応
extern	void			memorychk( void );
extern	void			sig_chk( void );
extern	void			fulchk( void );
extern	void			wopelg( ushort no, ulong before, ulong after );
#define	wopelg2	wopelg
extern	void			wmonlg(ushort no, void *p_info, ulong ul_info);
extern	short			turick( void );
extern	void			Ex_outport( void );
extern	void			ExOutSignal( uchar, uchar );
// MH810105(S) MH364301 インボイス対応
extern	short			paperchk( void );
extern	short			paperchk2( void );
// MH810105(E) MH364301 インボイス対応
extern	short			cinful( void );
extern	short			notful( void );
extern	void			err_mod_chk( unsigned char *, unsigned char *, short );
extern	short			LabelPaperchk( void );
extern	char			DoorCloseChk( void );
extern	void			ex_errlg( uchar, uchar, uchar, uchar );
extern	uchar			IsErrorOccuerd( char md, char no );
extern	ushort			IsErrorInfoGet( char type, char md, char no );
extern	ushort	GetErrorOccuerdLevel( char md, char no );
extern	void	Ope_CenterDoorResend( void );
// GG120600(S) // Phase9 遠隔監視データ変更
//extern	void	wrmonlg(ushort FuncNo, ulong Code, ushort MonInfo, u_inst_no *pInstNo, date_time_rec *pTime1, date_time_rec *pTime2, uchar *pProgNo);
extern	void	wrmonlg(ushort FuncNo, ulong Code, ushort MonInfo, u_inst_no *pInstNo, date_time_rec *pTime1, date_time_rec *pTime2, uchar *pProgNo,uchar fromflag,uchar upflag);
// GG120600(E) // Phase9 遠隔監視データ変更

enum{
	ERR_INFO_ALL = 0,
	ERR_INFO_LOGREGIST,
	ERR_INFO_JOURNAL,
	ERR_INFO_CLOSE,
	ERR_INFO_NOTICETOHOST,
	ERR_INFO_TROUBLESIGOUT,
	ERR_INFO_BIGHORN,
	ERR_INFO_MUKOU,
	ERR_INFO_LEVEL,
};

extern char				getFullFactor(unsigned char);
// MH322914(S) K.Onodera 2016/08/30 AI-V対応：エラー・アラーム
extern ushort			getErrLevel( ushort no, ushort code );
extern ushort			getAlmLevel( ushort no, ushort code );
// MH322914(E) K.Onodera 2016/08/30 AI-V対応：エラー・アラーム
// 不具合修正(S) K.Onodera 2016/10/13 #1505 アラーム01-61(5000円札釣り切れ)発生を含むデータが応答されてしまう
extern BOOL 			isDefToErrAlmTbl( uchar type, uchar no, uchar code );
// 不具合修正(E) K.Onodera 2016/10/13 #1505 アラーム01-61(5000円札釣り切れ)発生を含むデータが応答されてしまう

/* sigctrl.c */
extern	void			Out1shotSig_Interval( void );
extern	void			OutSignalCtrl( uchar, uchar );
extern	uchar			InSignalCtrl( ushort );
extern	void			ExIOSignalwt( uchar, uchar );			// 拡張I/Oポート出力
extern	ushort			ExIOSignalrd( uchar );					// 拡張I/Oポート入力
extern	unsigned char	read_rotsw(void);

/* opemif.c */
extern	void			OpMif_ctl( ushort );
extern	void			OpMif_snd( uchar, uchar );
extern	void			OpMif_snd2( uchar cmdid, uchar stat );
extern	short			MifareDataChk( void );
extern	void			MifareDataWrt( void );
extern	void			MifareDataWrt2( void );
extern	void			Mifare_WrtNgDataUpdate( uchar Request, void *pData );
extern	void			Mifare_WrtNgDataUpdate_Exec( void );
extern	ushort			Mifare_WrtNgDataSearch( uchar pSid[] );

/* bunrui.c */
extern	void			bunrui( ushort, ulong ,char);
// 仕様変更(S) K.Onodera 2016/12/13 #1674 集計の精算料金に対する振替分の加算方法を変更する
extern	void			bunrui_Erace( Receipt_data* pay, ulong back, char syu );
// 仕様変更(E) K.Onodera 2016/12/13 #1674 集計の精算料金に対する振替分の加算方法を変更する
extern	void			bunrui_syusei( ushort pos, ulong ryo );

/* opelpr.c */
extern	void			OpLpr_init(void);
extern	ushort			OpLpr_ctl(ushort msg, uchar *data);
extern	void			OpLpr_snd( Receipt_data *lpn, char test_flg );

/* ope_setup.c */
typedef struct {
	uchar		ProcMode;										// 処理区分
	uchar		Cmnprm;											// 共通設定ﾃﾞｰﾀ
	uchar		Psnprm;											// 個別設定ﾃﾞｰﾀ
	uchar		Usrprm;											// ﾕｰｻﾞｰﾊﾟﾗﾒｰﾀﾃﾞｰﾀ
	uchar		PriHdr;											// 領収証ﾍｯﾀﾞｰﾃﾞｰﾀ
	uchar		PriPrm;											// 領収証ﾌｯﾀｰﾃﾞｰﾀ
	uchar		PriLg;											// ﾛｺﾞ印字ﾃﾞｰﾀ
	uchar		FlpPrm;											// 車室ﾊﾟﾗﾒｰﾀ設定
	uchar		LckPrm;											// ﾛｯｸ装置ﾊﾟﾗﾒｰﾀ設定
} SETUP_NTNETDATA;

extern	uchar	OPESETUP_StartSetup(SETUP_NTNETDATA *Data);
extern	void	OPESETUP_SetupDataCancel(SETUP_NTNETDATA *Data);
extern	uchar	OPESETUP_SetupDataChk(uchar ID, ushort Status);

extern	short	AN_buf;

extern	void	dspCyclicMsgInit(void);
extern	uchar	dspIsCyclicMsg(void);
extern	void	dspCyclicMsgRewrite(uchar);
extern	void	dspCyclicErrMsgRewrite();

extern	void	err_suica_chk( uchar *, uchar *, uchar );
extern	short	al_emony( ushort msg, ushort paymod );
// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) G.So ICクレジット対応
//extern	short	al_emony_sousai(void);
//// MH321800(E) G.So ICクレジット対応
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
extern	ushort	Ope_Disp_Media_Getsub( uchar mode );
extern	ushort	Ope_Disp_Media_GetMsgNo( uchar mode, uchar type1, uchar type2 );
extern	ushort	Ope_Disp_Receipt_GetMsgNo(void);
extern	ushort	SysMnt_Log_CntDsp2( ushort LogSyu, ushort LogCnt, const uchar *title, ushort PreqCmd, ushort *NewOldDate );
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	void	Edy_CentComm_sub( uchar disp_type, uchar comm_type );		// センター通信実施ルーチン
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define	OPE_RECIPT_LED_DELAY	(10*2)								// 領収証発行延長・中止領収証機能での領収証発行時の取出し口点滅時間(10S,*500msec)
extern	void	op_ReciptLedOff( void );
extern void		add_month ( struct clk_rec *clk_date, unsigned short add_val );
extern int		al_preck_Kigen ( m_gtprepid* mag, struct clk_rec *clk_Kigen );
extern void		GetPrcKigenStr  ( char *PccUkoKigenStr );

extern  uchar	MifStat;
// Mifare書込み状態
enum {
	MIF_WAITING = 0,		// 0=書込みしていない
	MIF_WRITING,			// 1=書き込み処理中
	MIF_WROTE_CMPLT,		// 2=書込み完了（正常）
	MIF_DATA_ERR,			// 3=データエラー
	MIF_WROTE_FAIL,			// 4=書込み失敗（同一ｶｰﾄﾞﾀｯﾁ待ち）
};

extern	ushort	Ope2_Log_CountGet_inDateTime( ushort LogSyu, ushort *NewDate, ushort *OldDate, ushort NewTime, ushort OldTime );
extern	ushort	SysMnt_Log_CntDsp3(ushort LogSyu, const uchar *title, ushort PreqCmd);
extern	ushort	Ope2_Log_CountGet_inDateTime_AttachOffset( ushort LogSyu, ushort *NewDate, ushort *OldDate, ushort NewTime, ushort OldTime, ushort *FirstIndex, ushort *LastIndex );
extern	ushort	Ope2_Log_NewestOldestDateGet_AttachOffset(ushort LogSyu, date_time_rec *NewestDate, date_time_rec *OldestDate, ushort *FirstIndex, ushort *LastIndex );
extern	int		Ope_isPrinterReady(void);
extern	int		Ope_isJPrinterReady(void);
extern	unsigned short	Cardress_DoorOpen( void );

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern	void	Edy_CentComm_Recv( uchar type );
//extern	void	Edy_CentComm_Blink( void );
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

/* oiban.c */
extern	ulong	CountSel( ST_OIBAN *oiban );
extern	int		CountCheck( void );
extern	void	CountFree( uchar kind );

/* define */
#define		SETDIFFLOG_BAK				200		// １履歴情報に格納可能な差分データ数（255 以下のこと)
#define		SETDIFFLOG_MAX				1010	// 履歴バッファ全体数 ( 個別ﾍｯﾀﾞｰとﾀｲﾑｽﾀﾝﾌﾟも含まれる、よってこの値が格納可能全件数ではない )
#define		REM_SET_CALSCH_ADDRLEN		3		// 遠隔料金設定　設定可能アドレス指定の設定値の数（開始、終了１セットとしたセット数)

/* enum */
// データ個別ヘッダー、変更種別
enum{
	SETDIFFLOG_SYU_NONE = 0,					// 変更種別なし
	SETDIFFLOG_SYU_DEFLOAD,						// デフォルトロード
	SETDIFFLOG_SYU_SYSMNT,						// ｼｽﾃﾑﾒﾝﾃﾅﾝｽにて変更
	SETDIFFLOG_SYU_USRMNT,						// ﾕｰｻﾞｰﾒﾝﾃﾅﾝｽにて変更
	SETDIFFLOG_SYU_PCTOOL,						// PC設定ﾂｰﾙにて変更
	SETDIFFLOG_SYU_REMSET,						// 遠隔設定にて料金・パスワード変更
	SETDIFFLOG_SYU_ROLLBACK,					// 前回運用値戻しにて変更
	SETDIFFLOG_SYU_FCSCH,						// 料金設定予約（SysMnt)にて変更
	SETDIFFLOG_SYU_PCDEFLOAD,					// PC設定ﾂｰﾙからﾃﾞﾌｫﾙﾄﾛｰﾄﾞ指示
	SETDIFFLOG_SYU_RBACKSCH,					// 前回運用値戻しにて変更(SysMnt)
	SETDIFFLOG_SYU_INVALID,						// 固定データ変更にて強制戻し
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス (#4014 RSW=5が設定更新情報印字に記録されない)
	SETDIFFLOG_SYU_IP_DEF,						// RSW=5起動によるIP，ポートデフォルトセット
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス (#4014 RSW=5が設定更新情報印字に記録されない)
	SETDIFFLOG_SYU_MAXV							// SetDiffPrnStr_1の配列数(ログ種別数)
};
// 処理ﾌｪｰｽﾞﾌﾗｸﾞ
enum {
	SETDIFFLOG_CYCL_IDLE = 0,					// IDLE / 格納終了
	SETDIFFLOG_CYCL_MIRROR,						// 共通パラメータのミラー領域複写
	SETDIFFLOG_CYCL_ACTIVE,						// ヘッダー格納中
	SETDIFFLOG_CYCL_DIFFING,					// 差分情報格納中
	SETDIFFLOG_CYCL_BOTTOM
};

/* struct */
// バッファヘッダー
typedef struct {
	unsigned short		UseInfoCnt;				// 使用済データ数 Info の使用数
	unsigned short		InPon;					// 入力ポインター Info[n]の n
	unsigned short		OlderPon;				// 最古のポインター Info[n] の n
} TD_SETDIFFLOG_INDX;
// 個別データヘッダー(１件目 : 12Byte)
typedef struct {
	unsigned char	hed_mark[2];				// ﾍｯﾀﾞｰﾏｰｸ 'H','D'
	unsigned short	zen_cnt;					// 差分情報件数
	unsigned char	count;						// 差分データ格納件数( 0 -- SETDIFFLOG_BAK_MAX ):ｺﾍﾞﾂﾍｯﾀﾞｰﾄ,ﾀｲﾑｽﾀﾝﾌﾟﾊ,ﾌｸﾏｽﾞ
												// エリア数定義の格納件数がﾊﾞｯﾌｧ全件数より小さい場合は zen_cnt > count も有り得る.
	unsigned char	syu;						// 種別
	unsigned short	year;						// 日時情報(年)
	unsigned char	mont;						// 日時情報(月)
	unsigned char	date;						// 日時情報(日)
	unsigned char	hour;						// 日時情報(時)
	unsigned char	minu;						// 日時情報(分)
} TD_SETDIFFLOG_HEAD;
// 個別データ差分情報(２～ｍ件目 : 12Byte)
typedef struct {
	short	ses;								// 共通パラメータ(セッション)
	short	add;								// 共通パラメータ(アドレス)
	long	bef;								// 変更前設定値
	long	aft;								// 変更後設定値
} TD_SETDIFFLOG_DIFF;
// 履歴情報 UNION(全て 12 Byteであること)
typedef union {
	TD_SETDIFFLOG_HEAD	head;					// 個別データヘッダー
	TD_SETDIFFLOG_DIFF	diff;					// 個別データ差分情報
}TD_SETDIFFLOG_INFO;
// 設定変更履歴バッファ
typedef struct {
	TD_SETDIFFLOG_INDX	Header;					// ログインデックス
	TD_SETDIFFLOG_INFO	Info[SETDIFFLOG_MAX];	// 個別データ([0]=ヘッダー/[1～m]=差分情報)
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
} TD_SETDIFFLOG;
// 復電処理用保存
typedef struct {
	unsigned char		cycl;					// see 'SETDIFFLOG_CYCL_XXX' 
	TD_SETDIFFLOG_INDX	Header;					// ログインデックス
	TD_SETDIFFLOG_INFO	Info[SETDIFFLOG_BAK+1];	// 1=header
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は最後尾に追加すること!!!
	//	   途中にメンバを挿入しないこと
} TD_BCKDIFFLOG;

/* extern */
extern TD_SETDIFFLOG	SetDiffLogBuff;			// 設定値変更履歴バッファ
extern TD_BCKDIFFLOG	SetDiffLogBack;			// 復電時ﾘｶﾊﾞﾘ用

/* function program / subroutine prottype */
extern void SetSetDiff ( unsigned char syubetu  );
extern void SetDiffLogBackUp ( void );
extern void SetDiffLogRecoverCheck_Go ( void );
extern void SetDiffLogReset( uchar ope_mode );
extern unsigned short SetDiff_UpdatePon ( unsigned short OldPon, unsigned short AddPon );

/* ---------------------------------------------------------------------------------------------------------
   設定値変更履歴バッファレイアウト全体像                          7                  0
                                                               +-- +------------------+
  7                           0                                |   |ﾍｯﾀﾞｰﾏｰｸ 1 'H'    | 固定文字
  +---------------------------+                                |   +------------------+
  | 使用済情報数              |                                |   |ﾍｯﾀﾞｰﾏｰｸ 2 'D'    | 固定文字
  +--                       --+                                |   +------------------+
  | 0 -- SETDIFFLOG_MAX       |                                |   |差分データ全件数  |
  +---------------------------+                                |   +--              --+
  | 現在入力ポインタ          |                                |   | 0 -- 1699        |
  +--                       --+                                |   +------------------+
  | 0 -- SETDIFFLOG_MAX-1     |                                |   |差分ﾃﾞｰﾀ格納件数  | 0 -- 200
  +---------------------------+        7               0       |   +------------------+
  | 最古のデータポインタ      |    +-- +---------------+       |   |変更種別          | SETDIFFLOG_SYU_XXXXXXX
  +--                       --+    |   | 個別ヘッダー  | ------+-- +------------------+
  | 0 -- SETDIFFLOG_MAX-1     |    |   +---------------+       |   | 西暦年           |
  +---------------------------+    |   | 差分ﾃﾞｰﾀ１件目| --+   |   +--              --+
  |  履歴情報 その１          |    |   +---------------+   |   |   |                  |
  | (個別ヘッダー + ﾀｲﾑｽﾀﾝﾌﾟ) | ---+   |               |   |   |   +------------------+
  | + (12 Byte x ｍ)          |    |  ==       ↓      ==  |   |   | 月               |
  +---------------------------+    |   |               |   |   |   +------------------+
  |                           |    |   +---------------+   |   |   | 日               |
  ==           ↓            ===   |   | 差分ﾃﾞｰﾀｍ件目|   |   |   +------------------+
  |                           |    +-- +---------------+   |   |   | 時               |
  +---------------------------+                            |   |   +------------------+
  |  履歴情報 そのＮ          |                            |   |   | 分               |
 =|                           |                            |   +-- +------------------+
  |                           |                            |                           
  +---------------------------+                            |       7                  0
                                                           +---+-- +------------------+
                                                               |   | 設定アドレス     |
                                                               |   +--              --+
                                                               |   |                  |
                                                               |   +------------------+
                                                               |   | 変更前データ     |
                                                               |   +--              --+
                                                               |   |                  |
                                                               |   +------------------+
                                                               |   | 変更後データ     |
                                                               |   +--              --+
                                                               |   |                  |
・使用済み情報数は 6Byte ﾃﾞ-ﾀ使用数                            +-- +------------------+
・履歴情報の最初は個別ヘッダー情報、最初＋１はﾀｲﾑｽﾀﾝﾌﾟ固定(最低12Byte)
・現在入力ポインタ、最古ポインタはそれぞれの個別ヘッダーの位置を示す
・それぞれの履歴情報( 12 x m byte )のサイズは 差分差分ﾃﾞｰﾀ格納件数 x 6 + (12 x 2)
・ﾊﾞｯﾌｧのﾎﾞﾄﾑまで使用した場合は先頭にまわりこむ（最古情報数分消える)=(ﾍｯﾀﾞｰ + 差分ﾃﾞｰﾀが消える)
・変更種別によっては、は差分ﾃﾞｰﾀなしで個別ﾍｯﾀﾞｰのみがありえる
・格納件数と全件数が != の場合がありえる、格納は 上限で切る
--------------------------------------------------------------------------------------------------------- */

#define		PRC_GENDOGAKU_MIN		10			// プリペイドカード精算限度額１０円.
extern  void	op_RtcFlagRegisterRead( void );
extern  void	op_RtcRegisterInit( void );
extern	uchar	timset( struct clk_rec * );
extern	uchar	timerd( struct clk_rec * );
extern	ushort	RTC_ClkDataReadReq( short );	// 時計データ読み込み要求
extern	void	I2C_Event_Analize( void );	// 時計データ読み込み要求
extern	void	I2C_BusError_Analize( void );
typedef struct {
	ushort		key;							// リピートキー
	uchar		enable;							// リピート有効/無効
	uchar		repeat;
} t_OpeKeyRepeat;

extern	void	Ope_KeyRepeatEnable(char mnt_mode);
extern	void 	ChekModeMain( void );
extern 	void	Ope_StartDoorKnobTimer(void);
extern 	void	Ope_StopDoorKnobTimer(void);
extern	void	Ope_EnableDoorKnobChime(void);
extern	void	Ope_DisableDoorKnobChime(void);
extern	uchar	Ope_IsEnableDoorKnobChime(void);
extern	void	Ope_CenterDoorTimer( char );

// 料金計算シミュレータ
extern	int		CheckOpeCondit(void);
extern	int		CheckReqCalcData(void);

// MH322914(S) K.Onodera 2016/09/14 AI-V対応：振替精算
extern void	ope_PiP_GetFurikaeGaku( PIP_FURIKAE_INFO *st );
// MH322914(E) K.Onodera 2016/09/14 AI-V対応：振替精算

// MH810103 GG119202 (s) #5320 モニタコードの「R0252」、「R0253」が通信データで送信されてしまう					
extern uchar chk_mon_send_ntnet( uchar MonKind,uchar MonCode);
extern uchar chk_opemon_send_ntnet( uchar OpeMonKind,uchar OpeMonCode);
// MH810103 GG119202 (e) #5320 モニタコードの「R0252」、「R0253」が通信データで送信されてしまう					
// MH810103(s) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない
extern void RTPay_pre_sousai(void);
extern void RTPay_set_sousai(void);
// MH810103(e) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない

// テスト用ログ作成(S)
//#define TEST_LOGFULL
#ifdef TEST_LOGFULL
extern void Test_Ope_LogFull(void);
#endif	// TEST_LOGFULL
// テスト用ログ作成(E)

#endif	// _OPE_DEF_H_

