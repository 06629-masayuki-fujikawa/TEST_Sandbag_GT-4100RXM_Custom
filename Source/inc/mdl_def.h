/*[]------------------------------------------------------------------------------[]*/
/*|	名称			:ﾓｼﾞｭｰﾙ間通信制御機器関連で使用するﾃﾞｰﾀ等の定義を行う		   |*/
/*|	ファイル名		:mdl_def.h								 					   |*/
/*|	ターゲットCPU	:H8S/2352								 					   |*/
/*|	Author    		:Tadashi_nakayama	Amano Co.,Ltd.		 					   |*/
/*|	Date    		:2004.5.19								 					   |*/
/*[]------------------------------------------------------------------------------[]*/
/*|	specifications											 					   |*/
/*|		keymag.hﾃﾞｰﾀ定義等を移植します						 					   |*/
/*|		MDLSUB.h 関数型宣言を移植します						 					   |*/
/*[]------------------------------------------------------------------------------[]*/
/*|		total_def.h , memdef.hの後に宣言して使用します		 					   |*/
/*[]------------------------------------------------------------------------------[]*/
#ifndef _MDL_DEF_H_
#define _MDL_DEF_H_

#include "SodiacAPI.h"

/*[]------------------------------------------------------------------------------[]*/
/*|		Reader , Anounce Machine												   |*/
/*[]------------------------------------------------------------------------------[]*/
#define		MTS_RED			0						/* ﾓｼﾞｭｰﾙ間通信磁気ﾘｰﾀﾞ			*/
#define		MTS_AVM			1						/* ﾓｼﾞｭｰﾙ間通信音声案内			*/
#define		MTS_MAX			(MTS_AVM+1)				/* ﾓｼﾞｭｰﾙ間通信最大接続数		*/
													/*								*/
													/*								*/
													/*------------------------------*/
struct	red_rec	{									/* Reader Data					*/
													/* ---------------------------- */
	unsigned char	idc1;							/* ID 1							*/
	unsigned char	idc2;							/* ID 2							*/
	unsigned char	rdat[250];						/* Read Data					*/
													/*------------------------------*/
};													/*								*/
													/*------------------------------*/
extern	struct	red_rec	RDT_REC;					/* Reader Read Data Buffer		*/
													/*------------------------------*/
													/*------------------------------*/
struct	rfn_rec	{									/* Reader End Data				*/
													/* ---------------------------- */
	unsigned char	idc1;							/* ID 1							*/
	unsigned char	idc2;							/* ID 2							*/
	unsigned char	ecod;							/* Kind							*/
	unsigned char	ercd;							/* Error Code					*/
	unsigned char	posi[2];						/* Head Position				*/
													/*------------------------------*/
};													/*								*/
													/*------------------------------*/
extern	struct	rfn_rec	RED_REC;					/* Reader End Command Buffer	*/
													/*------------------------------*/
													/*------------------------------*/
struct	rvr_rec	{									/* Module Version Data			*/
													/* ---------------------------- */
	unsigned char	idc1;							/* ID 1							*/
	unsigned char	idc2;							/* ID 2							*/
	unsigned char	kind;							/* Kind							*/
	unsigned char	vers[8];						/* Version No.					*/
													/*------------------------------*/
};													/*								*/
													/*------------------------------*/
extern	struct	rvr_rec	RVD_REC;					/* Reader Version No.			*/
													/*------------------------------*/
													/*------------------------------*/
extern	struct	rvr_rec	ANM_REC;					/* Anounce Machine Version No.	*/
													/*------------------------------*/
													/*								*/
struct	rds_rec	{									/* Dip Switch Data				*/
													/* ---------------------------- */
	unsigned char	result;							/* Result						*/
	unsigned char	state[4];						/* status						*/
													/*------------------------------*/
};													/*								*/
extern	struct	rds_rec	RDS_REC;					/* Dip Switch Data				*/
													/*------------------------------*/
struct	mdl_dat_rec {								/* Module Data Send Buffer		*/
													/*------------------------------*/
	short		mdl_size;							/* Send Data Size				*/
													/*------------------------------*/
	struct	red_rec	mdl_data;						/* Send Data Buffer				*/
													/*------------------------------*/
	short		mdl_endf;							/* ETX/ETB Flag					*/
													/*------------------------------*/
};													/*								*/
													/*								*/
extern	struct	mdl_dat_rec	MDLdata[MTS_MAX];		/*								*/
													/*								*/
													/*								*/
/*----------------------------------------------------------------------------------*/
/* Reader 																			*/
/*----------------------------------------------------------------------------------*/
													/*								*/
													/*								*/
extern char		MAGred[256];						/* Card Read Data Buffer		*/
													/*								*/
													/*								*/
extern char		MDW_buf[96];						/* Card Write Data Buffer		*/
													/*								*/
													/*								*/
extern short	MDW_siz;							/* Size							*/
													/*								*/
													/*								*/
extern char		MDP_buf[128];						/* Card Print&Write Data Buffer	*/
													/*								*/
													/*								*/
extern short	MDP_siz;							/* Size							*/
													/*								*/
													/*								*/
extern unsigned short	MDP_mag;					/* Write Data Size				*/
													/*								*/
													/*								*/
extern short	PAS_okng;							/* Pass Word OK/NG Flag			*/
													/* 0:OK / 1:NG					*/
													/*								*/
extern short	OPE_red;							/* ﾘｰﾀﾞｰ制御					*/
													/*								*/
#define		TIK_SYU_MAX		16						/* 券種　最大値					*/
#define		ID_SYU_MAX		6						/* ID種　最大値					*/
#define		MAG_GT_APS_TYPE	0						/* GT/APSフォーマット格納位置	*/
#define		MAG_EX_GT_PKNO	1						/* GT用に追加されたP10-P17格納位置*/
#define		MAG_ID_CODE		2						/* 磁気データ先頭番地			*/

extern	const	uchar	tik_id_tbl[TIK_SYU_MAX+1][ID_SYU_MAX];

typedef	struct {									/* 券種毎の最終読取券ID種保存域	*/
	uchar	pk_tik;									/* 駐車券						*/
	uchar	teiki;									/* 定期券						*/
	uchar	pripay;									/* プリペイドカード				*/
	uchar	kaisuu;									/* 回数券						*/
	uchar	svs_tik;								/* サービス券（掛売券・割引券）	*/
	uchar	kakari;									/* 係員カード					*/
} t_ReadIdSyu;

extern	t_ReadIdSyu	ReadIdSyu;						/* 券種毎の最終読取券ID種		*/
extern	uchar	MRD_VERSION[16];					// 磁気リーダーバージョン

/*----------------------------------------------------------------------------------*/
/* Anounce Machine																	*/
/*----------------------------------------------------------------------------------*/
struct	an_msg_def {								/* Anounce Machine				*/
	char	cnt;									/*								*/
	char	mod;									/*								*/
	char	msg[10];								/*								*/
};													/*								*/
													/*								*/
extern	struct an_msg_def MSG_def[5];				/*								*/
													/*								*/
													/*								*/
extern	short	MSG_str[9][4];						/*								*/
													/*								*/
													/*								*/
/*----------------------------------------------------------------------------------*/
/* Operation Data & Flag															*/
/*----------------------------------------------------------------------------------*/
													/*								*/
extern	unsigned short 	RD_mod ;					/* Reader Mode					*/
													/*								*/
													/*								*/
extern	unsigned char	RD_pos ;					/* Reader Head Position			*/
													/*								*/
													/*								*/
													/*								*/
													/*								*/
extern	char	Is_AN_reset;						/* AVM基盤リセット(した=1)		*/
													/*								*/
													/*								*/
extern	short	OP_MODUL;							/* Module Next Send Wait		*/
													/*								*/
extern	unsigned char	cr_service_holding;			/* 								*/
													/*								*/
extern	unsigned char AvmBusyFg;					/*								*/
													/*								*/
extern	char	rd_tik;								/* = 0 : ﾅｼ						*/
													/* = 1 : 駐車券保留				*/
													/* = 2 : 領発行抜き待ち			*/
													/*								*/
extern	char	pas_kep;							/* 定期保留ﾌﾗｸﾞ					*/
													/*								*/
extern	short	opr_bak;							/* 送信内容ﾊﾞｯｸｱｯﾌﾟ				*/
													/*								*/
extern	short	rd_faz;								/* ﾘｰﾀﾞｰﾌｪｰｽﾞ					*/
													/*								*/
extern	uchar	WaitForTicektRemove;				/* 抜き取り待ちﾌﾗｸﾞ				*/
													/*								*/
extern	uchar	RD_SendCommand;						/* 送信ｺﾏﾝﾄﾞｾｰﾌﾞ用				*/
													/*								*/
extern	uchar	Mag_LastSendCmd;					/* 磁気ﾘｰﾀﾞへ最後に送信したｺﾏﾝﾄﾞ*/
extern	uchar	RD_PrcWriteFlag;							/* プリペイドカード書き込み中フラグ */
extern	uchar	RD_Credit_kep;						/* 1=ｸﾚｼﾞｯﾄｶｰﾄﾞがﾘｰﾀﾞｰ内にある*/
extern	char	avm_alarm_flg;
extern	char pre_volume[2];
extern	ulong		announceFee;					// 精算料金（料金読み上げ時に使用）
#define	AN_MSGBUF_SIZE	10
extern	ushort		an_msgbuf[AN_MSGBUF_SIZE];					// 料金読み上げ用メッセージ編集バッファ
#define	SODIAC_ERR_NONE					0x00			/*	エラー無し				*/
#define	SODIAC_INIT_ERR					0x01			/*	Sodiac初期化エラー		*/
#define	SODIAC_AMP_ERR					0x02			/*	AMP登録エラー			*/
#define	SODIAC_EVEHOOK_ERR				0x04			/*	イベントフック登録エラー*/
#define	SODIAC_NOTSTOP_ERR				0x08			/*	再生終了通知無しエラー　*/

#define	AVM_REQ_BUFF_SIZE				20				/*	AVM要求バッファサイズ	*/
#define	AVM_REQ_RETRY_MAX				2				/*  要求リトライ最大回数	*/

typedef struct _AVM_SODIAC_REQ
{
	st_sodiac_param			req_prm;					/*	sodiac音声再生要求用	*/
	unsigned char			resend_count;				/*	繰り返し回数			*/
	unsigned char			wait;						/*	インターバル			*/
	unsigned char			message_num;				/*	メッセージ番号			*/
	unsigned char			dummy;						/*	ダミー					*/
}AVM_SODIAC_REQ;

typedef struct _AVM_SODIAC_CTRL
{
	AVM_SODIAC_REQ	sd_req[AVM_REQ_BUFF_SIZE];			/*  sodiac要求						*/
	unsigned char	write_ptr;							/*	AVM要求バッファライトポインタ	*/
	unsigned char	read_ptr;							/*	AVM要求バッファリードポインタ	*/
	unsigned char	retry_count;						/*  再送信カウンタ					*/
	unsigned char	resend_count_now;					/*	残り繰り返し回数				*/
	unsigned char	play_cmp;							/*  再生終了フラグ					*/
	unsigned char	message_num;						/*	メッセージ番号					*/
	unsigned char	stop;
	unsigned char	play_message_cmp;					/* 1メッセージ再生終了フラグ		*/
}AVM_SODIAC_CTRL;

extern		AVM_SODIAC_CTRL			AVM_Sodiac_Ctrl[2];
extern		st_sodiac_version_info 	Avm_Sodiac_Info;
extern		unsigned char			Avm_Sodiac_Info_end;
extern		unsigned char			Avm_Sodiac_Err_flg;
extern		unsigned char			Sodiac_FROM_Get;

/*----------------------------------------------------------------------------------*/
/* Module Communication Flag														*/
/*----------------------------------------------------------------------------------*/
extern short	mts_req;							/* ﾃﾞｰﾀ送信要求ｆｌｇ			*/
													/*								*/
/*----------------------------------------------------------------------------------*/
/* Module Status Buffer																*/
/*----------------------------------------------------------------------------------*/
													/*								*/
extern	unsigned char	RED_stat[4];				/* Reader Status				*/
													/*								*/
extern	unsigned char	w_stat1[3];					/* Reader EOT Status			*/
extern	unsigned char	w_stat2;					/* Reader EOT Status			*/
													/*								*/
/*----------------------------------------------------------------------------------*/
/* Module Status Buffer																*/
/*----------------------------------------------------------------------------------*/
													/*								*/
#define		M_R_INIT		0x49					/* Reader Initial Command		*/
#define		M_R_FONT		0x53					/*        Font					*/
#define		M_R_TEST		0x54					/*        Test					*/
#define		M_R_MOVE		0x4D					/*	  Card Move					*/
#define		M_R_READ		0x52					/*	  Read						*/
#define		M_R_WRIT		0x57					/*	  Write						*/
#define		M_R_PRNT		0x50					/*	  Print						*/
#define		M_R_PRWT		0x58					/*	  Print & Write				*/
#define		M_R_RECV		0x44					/*	  Read Data					*/
#define		M_R_FINE		0x45					/*	  Read Data					*/
#define		M_R_ANSW		0x09					/*	  Read Data					*/
													/*								*/
#define		M_A_MSAG		0x03					/* Anounce Machine Message		*/
#define		M_A_STOP		0x02					/*		   Stop					*/
#define		M_A_DEFN		0x05					/*		   Define				*/
#define		M_A_INIT		0x07					/*		   Initial				*/
#define		M_A_TEST		0x09					/*		   Test					*/
#define		M_A_ANSW		0x09					/*		   Test					*/
													/*								*/
#define		ANN_END			0						// 放送終了
/*----------------------------------------------------------------------------------*/
/* OPERED.C 																		*/
/*----------------------------------------------------------------------------------*/
#define		E_VERIFY		0x27					/*								*/
#define		E_START			0x21					/*								*/
#define		E_DATA			0x22					/*								*/
#define		E_PARITY		0x23					/*								*/
#define		E_VERIFY_30		30						/*								*/
/*----------------------------------------------------------------------------------*/
/* OPEANM.C 																		*/
/*----------------------------------------------------------------------------------*/
extern	void	ope_anm	( short );					/*								*/
extern	void	opa_ctl	( ushort );					/*								*/
													/*								*/
#define		AVM_TEST		0						/* ﾃｽﾄｺﾏﾝﾄﾞ4番 ﾊﾞｰｼﾞｮﾝ要求		*/
#define		AVM_STOP		1						/* 放送停止						*/
#define		AVM_AUX			2						/* 人体検知時					*/
#define		AVM_ICHISEL		3						/* 車室番号選択時				*/
#define		AVM_TURIARI		4						/* 料金表示時(釣銭あり状態)		*/
#define		AVM_TURINASI	5						/* 料金表示時(釣銭なし状態)		*/
#define		AVM_RYOUSYUU	6						/* 現金受付時及び領収証放送		*/
#define		AVM_KANRYO		7						/* 精算完了時					*/
#define		AVM_BTN_NG		8						/* 車室番号間違い時				*/
#define		AVM_P_INPUT		9						/* 暗証番号入力時				*/
#define		AVM_P_INPNG		10						/* 暗証番号間違い時				*/
#define		AVM_P_ENTRY		11						/* 暗証番号登録時				*/
#define		AVM_UKE_PRN		12						/* 受付券発行時					*/
#define		AVM_UKE_SUMI	13						/* 受付券発行済み時				*/
#define		AVM_KANRYO2		14						/* 精算完了時(フラップ)			*/
enum{
	_pre = AVM_KANRYO2,
	AVM_CREWAIT,			/* 15		ｸﾚｼﾞｯﾄ精算ｾﾝﾀｰ問合せ中ﾒｯｾｰｼﾞ */
	AVM_MAGCARD_ERR,		/* 16		磁気カードエラーアナウンス */
	AVM_BGM,				/* 17		リパークＢＧＭ */
	AVM_HOJIN,				/* 18		法人カード  */
	AVM_FORCE_ROCK,			/* 19		強制出庫（ロック） */
	AVM_FORCE_FLAP,			/* 20		強制出庫（フラップ） */
// MH810103 GG119202(S) 案内放送の電子マネー対応
//// MH321800(S) D.Inaba ICクレジット対応
//	AVM_CARD_ERR6,			// 21		カードの読み取りに失敗しました。
//	AVM_CARD_ERR7,			// 22		残高不足です。
//// MH321800(E) D.Inaba ICクレジット対応	
// MH810103 GG119202(E) 案内放送の電子マネー対応

	// 27	空き
	AVM_RYOUKIN = 28,		// 料金読み上げ(仮)
	AVM_SHASHITU = 29,		// 車室読み上げ(仮)
	
	AVM_CREJIT_NG = 32,		// 32		只今、クレジットカードはお取り扱い出来ません。
	AVM_CREJIT_MAXOVER,		// 33		クレジットカードで精算できる限度額を超えているためクレジットカードによる精算は出来ません。
	AVM_CARD_RETRY,			// 34		再度矢印の方向に入れて下さい。
	AVM_TCARD_NG,			// 35		只今、Tカードはお取り扱い出来ません。
	AVM_TCARD_OK,			// 36		Tカードを受け付けました。
	AVM_TCARD_DOUBLE,		// 37		Tカードは受付済みです。
	AVM_HOJIN_MAXOVER,		// 38		法人カードで精算できる限度額を超えているため法人カードによる精算は出来ません。
	AVM_CARD_ERR5,			// 39		只今、お取り扱いが出来ません。
	AVM_RYOUSYUU_NG,		// 40		只今、領収書の発行ができません
	AVM_KIGEN_END,			// 41		このカードは期限切れです。
	AVM_CARD_ERR3,			// 42		このカードは無効です。
	AVM_CARD_ERR1,			// 43		このカードは使えません。
	AVM_CARD_ERR2,			// 44		このカードは読めません。
	AVM_CARD_ERR4,			// 45		このカードはお取り扱い出来ません。
	AVM_KIGEN_NEAR_END,		// 46		このカードはまもなく期限切れとなります。
// MH810103 GG119202(S) 案内放送の電子マネー対応
//// MH321800(S) Y.Tanizaki ICクレジット対応
//	AVM_ICCARD_ERR1,		// 47		カードをお取りください。
//// MH321800(E) Y.Tanizaki ICクレジット対応
	AVM_ICCARD_ERR1,		// 47		カードを抜いてください。
	AVM_CARD_ERR6,			// 48		カードの読み取りに失敗しました。
	AVM_CARD_ERR7,			// 49		残高不足です。
// MH810103 GG119202(E) 案内放送の電子マネー対応
// MH810103 GG119202(S) 磁気リーダーにクレジットカード挿入したときの案内放送・表示
	AVM_CARD_ERR8,			// 50		決済リーダーで精算してください
// MH810103 GG119202(E) 磁気リーダーにクレジットカード挿入したときの案内放送・表示
// MH810103 GG119202(S) 電子マネーブランド選択時の案内放送
	AVM_SELECT_EMONEY,		// 51		電子マネーブランド選択時案内
// MH810103 GG119202(E) 電子マネーブランド選択時の案内放送
};

#define		AVM_RYOUKIN_HA		102			// 「料金は」のメッセージ№
#define		AVM_SHASHITU_BAN	105			// 「車室番号」のメッセージ№
#define		AVM_SHASHITU_HA		107			// 「車室番号は」のメッセージ№
#define		AVM_BANGOU_1		106			// 「番の料金は」(後に料金読み上げが)
#define		AVM_BANGOU_2		108			// ｎヒャク,バンデスと放送する場合の"バンデス"

/* 券エラーアナウンス */
/* 設定に持たないアナウンス番号(効果音) */
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#define		AVM_Edy_OK		120					/* Edy OK 音 */
//#define		AVM_Edy_NG		121					/* Edy NG 音 */
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
#define		AVM_BOO			122					/* ブザー音		  */
#define		AVM_IC_OK		123					/* ICカード検知音 */
#define		AVM_IC_NG1		124					/* NG音 */
#define		AVM_IC_NG2		125					/* NG音 */
#define		AVM_BGM_MSG		126					/* リパークBGM */

#define		AVM_AN_TEST		490						// 案内放送テスト

													/*								*/
/* 音声放送は0.5秒間はコマンドを送信しないこととする								*/
/* 音声案内放送後0.5秒以内															*/
/* 音声案内放送なしまたは、0.5秒経過												*/
#define		AVM_BUSY		1						/*								*/
#define		AVM_READY		0						/*								*/
#define		AVM_TIMER		5						/*								*/
#define		AVM_VOLMAX		31						/* 音量MAX値 					*/
extern unsigned short avm_test_no[2];
extern char	avm_test_ch;
extern short	avm_test_cnt;

/* ラグタイム読上げ用 */
#define		AVM_FLAP_BAN	185	//   "フラップ板が下がったことを確認後、"
#define		AVM_LOCK_BAN	186	//   "ロック板が下がったことを確認後、"
#define		AVM_1MINUTE		87	//   "1分以内に"
#define		AVM_SYUKKO		109	//   "出庫してください"

/* 車室・料金読み上げ用 */
#define		ANN_YEN_DESU	103	//   "エンデス"
#define		ANN_JUU			10	//   "ジュウ"
#define		ANN_HYAKU		37	//   "ヒャク"
#define		ANN_SEN			50	//   "セン"
#define		ANN_MAN			67	//   "マン"

// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(メンテナンス)
//// MH321800(S) T.Nagai ICクレジット対応
////#define		ANN_MSG_MAX		254	//   メッセージ番号MAX項目数
//#define		ANN_MSG_MAX		299	//   メッセージ番号MAX項目数
//// MH321800(E) T.Nagai ICクレジット対応
#define		ANN_MSG_MAX		999	//   メッセージ番号MAX項目数
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(メンテナンス)

/*----------------------------------------------------------------------------------*/
/* MDLSUB.C 																		*/
/*----------------------------------------------------------------------------------*/
extern	short	rd_mov( short );					/*								*/
extern	short	rd_init( short );					/*								*/
extern	short	rd_read( short );					/*								*/
extern	short	rd_FB7000_MntCommandSend( uchar, uchar, uchar );
extern	short	rd_test( short );					/*								*/
extern	short	rd_font( void );					/*								*/
extern	short	an_init( char );					/*								*/
extern	short	an_test( char );					/*								*/
extern	short	an_boo( void );						/*								*/
extern	short	an_defn( char, char, char * );		/*								*/
extern	void	md_pari( uchar *, ushort, char );	/*								*/
extern	void	md_pari2( uchar *, ushort, char );	/*								*/
extern	char	Is_AVM_connect(void);
extern	void	an_reset(void);
extern	short	an_msag( short *an_msgno, short msg_cnt, short cnt, short wat, char ch);
extern	void 	an_stop(char ch);
extern	short	msg_set( short *an_msgno, short prm );
extern	short	an_boo2(short	seg);
extern	void opa_chime(char onoff);
extern	short an_msag_edit_rxm(short mode);
extern	void	AVM_Sodiac_Init( void );			/*	Sodiac初期化処理			*/
extern	void	AVM_Sodiac_Execute( unsigned short ch );	/*  	*/
extern	void	AVM_Sodiac_Play_Wait_Tim( unsigned short ch );
extern	void	AVM_Sodiac_Err_Chk( D_SODIAC_E err, unsigned char notice_id );
extern	unsigned char	AVM_Sodiac_NextMessageNum( unsigned short ch,	unsigned char ptr);
extern	unsigned char	AVM_Sodiac_EqualReadPtr( unsigned short ch,	unsigned char  ptr, unsigned char message_num );
extern	void	AVM_Sodiac_EqualMsgBuffClear( unsigned short ch,	unsigned char  ptr, unsigned char message_num );
extern  void	AVM_Sodiac_ReadPtrInc( unsigned short ch,	unsigned char ptr);
extern	void	AVM_Sodiac_Play_WaitReq( unsigned char ch,  unsigned char ptr );
extern	void	AVM_Sodiac_TimeOut( void );
extern	void	AVM_Sodiac_SemFree( void );

#define AVM_NOT_CONNECT		0
#define AVM_CONNECT			1

/*------------------------------------------------------------------------------*/
/* CR_SND.C																		*/
/*------------------------------------------------------------------------------*/
extern 	void	cr_snd( void );							/* ﾘｰﾀﾞﾃﾞｰﾀ送信			*/
extern 	void	avm_snd( void );						/* AVMﾃﾞｰﾀ送信			*/
														/*						*/
#define		MTS_BCR1		0x8000						/* データ送信要求ビット	*/
#define		MTS_BCRWT1		0x0080						/* 挿入口ｶｰﾄﾞなし待ち要求ビット	*/
														/*						*/
#define		MTS_BAVM1		0x0800						/* AVM送信要求ビット	*/
														/*						*/
#define		TICKET_MOUTH	0x10						/* 挿入口券ありビット	*/
#define		R_CMD_RCV		0x20						/* ﾘｰﾄﾞｺﾏﾝﾄﾞ受信済みビット	*/
														/*						*/
/*------------------------------------------------------------------------------*/
/* CR_RCV.C																		*/
/*------------------------------------------------------------------------------*/
void	cr_rcv( void );									/*						*/
void	avm_rcv( void );								/*						*/
char	avm_queset( char);								/*						*/
														/*						*/
extern	char get_timeptn(void);
extern	char get_anavolume(char,char);

#endif	// _MDL_DEF_H_
