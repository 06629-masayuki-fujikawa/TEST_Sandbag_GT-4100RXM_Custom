/*[]----------------------------------------------------------------------[]*/
/*| headder file for common cnmtask                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| DATE        : 95.07.31                                                 |*/
/*| UPDATE      : 2001-11-01                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#ifndef _CNM_DEF_H_
#define _CNM_DEF_H_

/*** EQU ***/

#define		SYN_OUT_TIM		51				/* 1.0sec SYN out time in module reset */
#ifdef	SUICA_EAON_DEBUG_MODE
#define		RCV_MON_TIM		26				/* 500ms RECIVE TIMEOUT TIMER */
#else
#define		RCV_MON_TIM		6				/* 10ms RECIVE TIMEOUT TIMER */
#endif

// MH321800(S) G.So ICクレジット対応
//#define		R_BUFSIZ	64					/* Receive buffer size */
#define		R_BUFSIZ	256					/* Receive buffer size */
#define		T_BUFSIZ	256					/* Send buffer size */

#define		CN_TX_TIMEOUT					((T_BUFSIZ * 2 * 14 + 100) / (200))
											// 送信バッファサイズ * 2ms(4800bps) * 1.4(40%マージン)
// MH321800(E) G.So ICクレジット対応

#define		CN_WAIT_S	26					/* 500～ 520ms 		*/
#define		T_RTRY		31					/* Time Out Retry 	*/
#define		R_RTRY		21					/* Recive Retry 	*/
#define		S_RTRY		21					/* Sel. Retry 		*/

#define		TUB_MAX10		85				/* 10YEN tube max 	*/
#define		TUB_MAX50		77				/* 50YEN tube max 	*/
#define		TUB_MAX100		75				/* 100YEN tube max 	*/
#define		TUB_MAX500		71				/* 500YEN tube max 	*/
#define		TUB_MAX10SUB	95				/* 10YENSUB tube max 	*/
#define		TUB_MAX100SUB	80				/* 100YENSUB tube max 	*/
#define		TUB_MAX10YO		150				/* 10YENYOCHIKU tube max 	*/
#define		TUB_MAX50YO		135				/* 50YENYOCHIKU tube max 	*/
#define		TUB_MAX100YO	140				/* 100YENYOCHIKU tube max 	*/

#define		TUB_CURMAX10		92			/* 10YEN tube max 	*/
#define		TUB_CURMAX50		82			/* 50YEN tube max 	*/
#define		TUB_CURMAX100		77			/* 100YEN tube max 	*/
#define		TUB_CURMAX500		70			/* 500YEN tube max 	*/
#define		TUB_CURMAX10SUB		100			/* 10YENSUB tube max 	*/
#define		TUB_CURMAX100SUB	87			/* 100YENSUB tube max 	*/
#define		TUB_CURMAX10YO		155			/* 10YENYOCHIKU tube max 	*/
#define		TUB_CURMAX50YO		135			/* 50YENYOCHIKU tube max 	*/
#define		TUB_CURMAX100YO		140			/* 100YENYOCHIKU tube max 	*/

#define		CN_ESCROW		4				/* 投入金ｸﾘｱし、金庫枚数算出までの時間 */
											/* 100円ｺｲﾝｴｽｸﾛのときは4秒にする */

/*** structure ***/

/*	<<< Rom Data >>>	*/

extern const short	coin_vl[];
extern const unsigned char	co_max[];
extern const unsigned char	com_tbl[][2];
extern const unsigned char	cnt_tbl[][2];
extern const unsigned char	ack_tbl[];
extern const unsigned char sv_coin_max[4];

/*	<<< Ram Data >>>	*/

extern	unsigned char 	cnm_sts;			/* Send Status for Coinmech 	*/
extern	unsigned char 	Cnm_End_PayTimOut;/* 支払い終了フラグ				*/
extern	unsigned char	ntr_sts;			/* Send Status for Note 		*/
extern	unsigned short 	sfstat;				/* Status for Safe cal. 		*/
extern	unsigned short 	cn_polcnt;			/* Status for Safe cal. 		*/
extern	unsigned char	cn_errst[2];		/* Communication  Status 		*/
extern	unsigned char	cn_com;				/* Output data Flg to Coin Mech */
extern	unsigned char	nt_com;				/* Output data Flg to Note Reader */
// MH810103 GG119202(S) JVMAリセット処理変更
extern	unsigned char	cn_com_bak;			// cn_comのバックアップ
extern	unsigned char	nt_com_bak;			// nt_comのバックアップ
// MH810103 GG119202(E) JVMAリセット処理変更
extern	unsigned char	csd_buf[32];		/* Send Buffer (Temporary) 		*/
extern	short	csd_siz;					/* Number of Send Char. 		*/
extern	unsigned char	crd_buf[R_BUFSIZ];	/* Receive Buffer (Temporary) 	*/
extern	short	crd_siz;					/* Number of Receive Char. 		*/
extern	short	cnm_ope;					/* Init Coin mech & Note Reader */
extern	short	tn_save;					/* Com. stat =0:Coin Mech  =1:Note Reader */

extern unsigned	char	cn_com;
extern unsigned	char	nt_com;

											/*------------------------------*/
extern		short TXSDCM;					/* Flg Send Out Command       	*/
											/*------------------------------*/
extern unsigned char TXBUFF[];				/* Send buffer					*/
extern		short TXCONT;					/* Number of Send Data Size		*/
extern		short TXRPTR;					/* Send buffer poshorter		*/
extern		short TXRSLT;					/* Send Result					*/
extern		short TXWAIT;					/* Send flg						*/
											/*------------------------------*/
extern unsigned char RXBUFF[];				/* Receive buffer				*/
extern		short RXCONT;    				/* Number of Receive Data Size	*/
extern		short RXWAIT;    				/* Send flg						*/
// MH321800(S) G.So ICクレジット対応
extern volatile short TXRXWAIT;				/* Send flg						*/
// MH321800(E) G.So ICクレジット対応
union RXRSLT_rec {
		unsigned char BYTE;
		struct {
			unsigned char B7:1;				/* Bit 7						*/
			unsigned char B6:1;				/* Bit 6						*/
			unsigned char B5:1;				/* Bit 5						*/
			unsigned char B4:1;				/* Bit 4						*/
			unsigned char B3:1;				/* Bit 3						*/
			unsigned char B2:1;				/* Bit 2						*/
			unsigned char B1:1;				/* Bit 1						*/
			unsigned char B0:1;				/* Bit 0						*/
		} BIT;
}; 				 							/* Receive Result				*/
extern	union RXRSLT_rec	RXRSLT;

extern		char	RXDTCT;					/* 受信すべきﾃﾞｰﾀﾊﾞｲﾄ数			*/

extern		short	CN_tsk_timer;			/* cnmtask Control Timer		*/

extern		short	CN_escr_timer;			/* cnmtask escrow Timer			*/
extern		short	Tubu_cnt_set;			/* 枚数設定あり／なし			*/
extern		short	CN_refund;				/* 払出中ﾌﾗｸﾞ					*/
extern	unsigned char	CNM_SYN_Time;

/*	<<< Coin Mech Communication Data >>>	*/
											/*------------------------------*/
struct	CNM_REC_REC {						/*								*/
											/*------------------------------*/
	unsigned char r_dat06[12];				/* Tube Full Numbers 			*/
	unsigned char r_dat07[12];				/* Tube Count					*/
	unsigned char r_dat08[4];				/* Credit value	 				*/
	unsigned char r_dat09[4];				/* Avalable Change				*/
	unsigned char r_dat0a[4];				/* Change refunded				*/
	unsigned char r_dat0b;					/* Reader status				*/
	unsigned char r_dat0c[3];				/* Reader error code			*/
	unsigned char r_dat0d[2];				/* Reader version				*/
											/*------------------------------*/
};
											/*------------------------------*/
extern	struct	CNM_REC_REC	CN_RDAT;		/* Receive Data from Coin Mech	*/
											/*------------------------------*/
struct	CNM_SND_REC {						/* Coin Mech Send data			*/
											/* -----------------------------*/
	unsigned char s_dat00;					/* Control						*/
	unsigned char s_dat01[3];				/* Refund Value					*/
	unsigned char s_dat02[4];
	unsigned char s_dat03[12];				/* Change Counter Settings		*/
	unsigned char s_dat04[12];				/* Tube Counts Initialize		*/
											/*------------------------------*/
};
											/*------------------------------*/
extern	struct	CNM_SND_REC	CN_SDAT;		/* Send Data to Coin Mech		*/
											/*------------------------------*/

											/*------------------------------*/
extern		short	CN_SUB_SET[3];			/* ｻﾌﾞﾁｭｰﾌﾞ、予蓄金種設定情報	*/
											/*------------------------------*/

/*	<<< Note Reader Communication Data >>>	*/
											/*------------------------------*/
struct	NTR_REC_REC {						/* Note reader Receive data		*/
											/* -----------------------------*/
	unsigned char r_dat18[4];				/* Credit value	 				*/
	unsigned char r_dat19[4];				/* Avalable Change				*/
	unsigned char r_dat1a[4];				/* Change refunded				*/
	unsigned char r_dat1b[3];				/* Reader status				*/
	unsigned char r_dat1c[2];				/* Reader error code			*/
	unsigned char r_dat1d[2];				/* Reader version				*/
	unsigned char r_dat1e[5];				/* Gift	Ticket					*/
											/*------------------------------*/
};
											/*------------------------------*/
extern	struct	NTR_REC_REC	NT_RDAT;		/* Receive Data from Note Reader*/
											/*------------------------------*/
struct	NTR_SND_REC {						/* Coin Mech Send data			*/
											/*------------------------------*/
	unsigned char s_dat10[2];				/* Control						*/
	unsigned char s_dat11[4];				/* Refund Value					*/
											/*------------------------------*/
};
											/*------------------------------*/
extern	struct	NTR_SND_REC	NT_SDAT;		/* Send Data to Note Reader		*/
											/*------------------------------*/
typedef struct	sfv_dat_rec {
											/*------------------------------*/
	unsigned char	r_dat07[12];			/* Tube Count(金庫枚数算出用)	*/
	unsigned char	r_dat0a[4];				/* Refund Count					*/
	short		in_dat07[4];				/* Tube Count(金庫枚数算出用)	*/
	short		out_dat07[4];				/* Tube Count(金庫枚数算出用)	*/
	short		r_add08[4];					/* Save Credit value(金庫枚数算出用) */
	short		r_add0a[4];					/* Save refund value(金庫枚数算出用) */
	short		safe_dt[4];					/* Coin Safe Count(金庫枚数)	*/
	short		safe_dt_wk[4];				/* safe_dtの復電用ﾜｰｸ			*/
	short		safe_inout;					/* 0:払出なし、1:払出あり		*/
	short		cn_escrow;					/* ｺｲﾝｴｽｸﾛ中ﾌﾗｸﾞ				*/
	short		safe_cal_do;				/* 金庫枚数算出指示ﾌﾗｸﾞ			*/
	short		nt_escrow;					/* 紙幣ｴｽｸﾛ中ﾌﾗｸﾞ				*/
	short		nt_safe_dt;					/* Note Safe Count				*/
	long		refval;						/* Refund Value					*/
	long		reffal;						/* Refund Fault Value			*/
	short		powoff;						/* Power off flg  				*/
	short		work[2];					/* work area					*/
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
}	sfv_rec;

extern	sfv_rec		SFV_DAT;
typedef union{
	unsigned char	BYTE;
	struct{
		unsigned char YOBI:6;
		unsigned char note:1;
		unsigned char coin:1;
	}BIT;
}t_SFV_SNDSTS;

extern	t_SFV_SNDSTS	SFV_SNDSTS;

typedef	union {
	unsigned char		CHAR;
	struct {
		unsigned char		yobi		:6;			// 予備
		unsigned char		standby_snd_after:1;	// スタンバイ送信フラグ(r_dat09_rcv_waitとセットで使用)
		unsigned char		r_dat09_rcv_wait:1;		// 起動時の払出可能枚数データ受信待ちフラグ
	} BIT;
}CNM_CTRL_INFO;
extern	CNM_CTRL_INFO	cnm_ctrl_info;
extern	unsigned char ref_coinsyu[4];
extern	unsigned char Cnm_Force_Reset;
/*** function prototype ***/

/* CNMTASK.C */
extern  void	cnmtask( void );
extern	short	cnmact( short );
extern	short	cnm_recv( unsigned char *, short );
extern	void	cnm_save( unsigned char * );
extern	void	coin_inout( short, unsigned char* );
extern	void	cn_init( void );
extern	void	cnm_set( void );
extern	void	ntr_set( void );
extern	void	count_set( short );
extern	short	refcal( void );
extern	void	cmn_ComErrorRegist( void );


/* CREFUND.C */
extern	short	refund( long );
extern	void	refalt( void );
extern	short	cn_stat( short, short );
extern	void	cn_stat_wait( void );
extern	void	safecl( short );
extern	long	cn_crdt( void );
extern	void	cnm_mem( short );
extern	void	set_escrowtimer( void );
extern	long	cn_GetTempCredit( void );

/* CNMDRV.C */
extern	short	CNM_CMD( const unsigned char*, short );
extern	short	CNM_SND( unsigned char*, short );
extern	short	CNM_RCV( unsigned char*, short* );
extern	void	cnwait( short );
extern	void	CN_RCLR( void );
extern	void	SYN_ENB( void );
extern	void	CN_reset( void );
#endif	// _CNM_DEF_H_
