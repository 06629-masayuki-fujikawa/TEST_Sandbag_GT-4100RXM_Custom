#ifndef	___PIPDEFH___
#define	___PIPDEFH___

//--------------------------------------------------------------------------------
// Park i Pro  (PIP) 
// 各種型定義 / define ファイル
//
//--------------------------------------------------------------------------------
#define	R_BUF_MAX_H				2060
#define	SEND_MAX_H				1448	// ＨＯＳＴ送信エリアＭＡＸサイズ
#define	PIP_HOSTSEND_LEN_MAX	256

#define	PIP_PACKET_SIZE_MAX	(5+PIP_HOSTSEND_LEN_MAX+3)	// ＳＯＨ～ＢＣＣまでの最大サイズ(パケット一本分のサイズ)
#define	PIP_PACKET_DATA_MAX	(PIP_HOSTSEND_LEN_MAX+3)	// ＳＴＸ～ＢＣＣまでの最大サイズ(テーブル１件分のサイズ)
#define	NH_RBUF_MAX			(6+PIP_HOSTSEND_LEN_MAX+2)
#define	NH_SBUF_MAX			(6+PIP_HOSTSEND_LEN_MAX+2)

#define		PIP_ERR_DATLEN_IDB0		24		// エラー通知データ(共通)

typedef struct {
	unsigned char	COUNT;							// パケット数(０～８０)
	unsigned char	CIND;							// 送信パケット位置(０～７９，８０:送信完了)
	struct {
		unsigned short	LEN;						// パケット長(ＳＴＸ～ＢＣＣ)
		unsigned char	BUFF[PIP_PACKET_DATA_MAX];	// データバッファ
	} DATA[80];										// ８０件
} t_PIP_SCISendQue;									// シリアル送信キュー

union PIP_B0_54_CTRL {
	struct {
		unsigned short  dmy   : 7;
		unsigned short	ON_54 : 1;	// [共通]メモリ破損通知(1:メモリ破損検出, 0:メモリ正常)
		unsigned short	B0_01 : 1;	// [ゲート式]外部接続機器未応答(1:発生中, 0:復帰)
		unsigned short	B0_02 : 1;	// [ゲート式]精算データ送信ＮＧ(1:発生中, 0:復帰)
		unsigned short	B0_03 : 1;	// [ゲート式]エラー発生データ送信ＮＧ(1:発生中, 0:復帰)
		unsigned short	B0_04 : 1;	// [ゲート式]エラー解除データ送信ＮＧ(1:発生中, 0:復帰)
		unsigned short	B0_05 : 1;	// [ゲート式]精算可否判定データ送信ＮＧ(1:発生中, 0:復帰)
		unsigned short	B0_06 : 1;	// [ゲート式]領収証再発行可否判定データ送信ＮＧ(1:発生中, 0:復帰)
		unsigned short	B0_10 : 1;	// [ゲート式]ＡＲＣＮＥＴシーケンシャル№異常(1:発生中, 0:復帰)
		unsigned short	B0_11 : 1;	// [ゲート式]受信ＲＡＵ電文異常(1:発生中, 0:復帰)
	} bits;
	unsigned short		WordData;
};
union	pip_bits_reg {
	struct	pip_bit_tag {
		unsigned char	bit_7	: 1 ;
		unsigned char	bit_6	: 1 ;
		unsigned char	bit_5	: 1 ;
		unsigned char	bit_4	: 1 ;
		unsigned char	bit_3	: 1 ;
		unsigned char	bit_2	: 1 ;
		unsigned char	bit_1	: 1 ;
		unsigned char	bit_0	: 1 ;
	} bits;
	unsigned char	byte;
};
typedef union pip_bits_reg	PIP_BITS;

//==============================================================================
//	ＰａｒｋｉＰＲＯ制御用変数(モジュールローカル)
//
//	@attention	構造体の先頭アドレスを保持しているポインタ(PIPRAM)以外は実体がない。
//
//	@note	ＰａｒｋｉＰＲＯモジュールが(共用)外部ＲＡＭ空間に定義している変数群。
//
//======================================== Copyright(C) 2007 AMANO Corp. =======

typedef struct {
	char			PIP_RamMark[16];					// ＲＡＭ破損検出用キーワード"ＰＩＰＴＡＳＫＲＡＭ＿ＳＴＡＲＴ" (破損していたら＿ＥＮＤまでＡＬＬクリア)

	unsigned short	initialize_start;					// ＲＡＭの破損がないとき、ここから un_initialize_start までクリア

	unsigned char	Err_sendbuf[40];					// パラメータ不良エラー送信バッファ

	char			uc_r_buf_h[R_BUF_MAX_H];			// シリアル受信バッファ
	unsigned short	ui_write_h;							// シリアル受信バッファ(１キャラクタ)書き込み位置
	unsigned short	ui_read_h;							// シリアル受信バッファ読み込み位置
	unsigned short	ui_count_h;							// シリアル受信バッファデータ取得数(バイト数)
	unsigned char	uc_rcv_byte_h;						// シリアル受信バッファ１バイト受信ワーク

	unsigned short	ui_txdlength_i_h;					// シリアル送信バッファ送信データ長(割り込み用)
	unsigned short	ui_txpointer_h;						// シリアル送信バッファ送信データポインタ
	unsigned char	huc_txdata_i_h[SEND_MAX_H];			// シリアル送信バッファ(割り込み用)
	unsigned char	uc_send_end_h;						// シリアル送信バッファ送信完了フラグ

	unsigned long	NH_StartTime;						/* count of timer start */
	
	unsigned short	NH_Rct1;							/* NAK retry counter */
	unsigned short	NH_Rct2;							/* Pol error counter */
	unsigned short	NH_Rct3;							/* Sel error counter */
	
	unsigned char	NH_f_ComErr;						// シリアル送信エラーフラグ(0:送信可, 1:エラーにより送信不可)
	
	unsigned char	NH_PrevRecvChar;					/* prevous received character */
	unsigned char	NH_f_NxAck;							/* next send/receive ACK (0x30=ACK0, 0x31=ACK1) */
	
	unsigned char	NH_Rbuf[NH_RBUF_MAX+2];				/* received character save area (temp) */
	unsigned short	NH_RbufCt;							/* received character count */
	
	unsigned char	NH_Sbuf[NH_SBUF_MAX+2];				// シリアル送信バッファ
	unsigned short	NH_SbufLen;							// シリアル送信バッファ内のパケット長

	PIP_BITS		pip_uc_txerr_h;						// シリアル通信エラーフラグ
	unsigned char	pip_uc_mode_h ;						// ＰａｒｋｉＰＲＯとの通信処理ステータス
														// 0=IDLE
														// 1=POL(/NAK)送信完
														// 2=SOH/STX受信 (ETX待ち)
														// 3=ETX受信 (BCC待ち)
														// 4=ACK送信完  (EOT.etc..待ち)
														// 5=SEL 送信完 (ACK.etc..待ち)
														// 6=電文送信完 (ACK.etc..待ち)

	unsigned char	i_time_setting;						// ポーリング間隔時間("01"～"99"×100ms+300ms)

	union 	PIP_B0_54_CTRL	Ctrl_B0_54;					// ＩＢＫがＢ０または５４でＭＡＩＮに通知するすべてのエラーの発生状況を保持する変数
	unsigned char	ARCs_B0_54_SendEdit[PIP_ERR_DATLEN_IDB0];	// 種別Ｂ０＆５４のＡＲＣＮＥＴパケット作成用バッファ

	unsigned char	wkuc[8];							// ＰＯＬ，ＡＣＫ，ＮＡＫ，ＥＯＴパケット作成用バッファ

	t_PIP_SCISendQue		SCI_S_QUE;					// シリアル送信キュー

	unsigned short	un_initialize_start;				// ＲＡＭの破損がないときはここから＿ＥＮＤまでクリアしない

	char	PIP_RamEnd[16];								// ＲＡＭ破損検出用キーワード"ＰＩＰＴＡＳＫＲＡＭ＿ＥＮＤ０１"
} t_PIPRAM;

#define	pip_f_txerr_h_dr	PIPRAM.pip_uc_txerr_h.bits.bit_6	// データ値範囲外エラー
#define	pip_f_txerr_h_dl	PIPRAM.pip_uc_txerr_h.bits.bit_5	// データ長エラー(データ長オーバー、８の倍数でないなど)
#define	pip_f_txerr_h_BCC	PIPRAM.pip_uc_txerr_h.bits.bit_4	// BCCエラー
#define	pip_f_txerr_h_txt	PIPRAM.pip_uc_hxerr_h.bits.bit_3	// テキストエラー(受信中に新たなSTXが来た)
#define	pip_f_txerr_h_fr	PIPRAM.pip_uc_txerr_h.bits.bit_2	// フレーミングエラー
#define	pip_f_txerr_h_p		PIPRAM.pip_uc_txerr_h.bits.bit_1	// パリティエラー
#define	pip_f_txerr_h_ovrun	PIPRAM.pip_uc_txerr_h.bits.bit_0	// オーバーランエラー

extern	t_PIPRAM	PIPRAM;								// 

extern	void			PIP_ChangeData(unsigned short Length, unsigned char *pData);		// シリアル受信データをIFMのバッファへ変換
extern	void			NH_Main( void );													// IFmodule通信処理メイン（状態管理部）
extern	unsigned char	Mt_BccCal(unsigned char *pSet, unsigned short Length);
extern	unsigned short	Mh_get_receive(void);
extern	void			NH_TimerSet(unsigned char f_Start);
extern	void			sci_init_h(void);
extern	void			sci_tx_start_h(void);
extern	void			sci_stop_h(void);
extern	void			PIP_DTRsigOut(unsigned char f_Lebel);
extern	unsigned char	PIP_BcdAscToBin_2(unsigned char *asc_dat);
extern	void			PIP_ARCsB0Regist(unsigned char err_code, unsigned char on_off);
extern	unsigned short	PIP_SciSendQue_insert(unsigned char *pData, unsigned short length);	// シリアル送信キューへのデータ追加
extern	void IFM_RcdBufClrAll();

extern	void			NH_Init( void );
#endif //___PIPDEFH__
