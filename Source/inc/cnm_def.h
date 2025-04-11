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

// MH321800(S) G.So IC�N���W�b�g�Ή�
//#define		R_BUFSIZ	64					/* Receive buffer size */
#define		R_BUFSIZ	256					/* Receive buffer size */
#define		T_BUFSIZ	256					/* Send buffer size */

#define		CN_TX_TIMEOUT					((T_BUFSIZ * 2 * 14 + 100) / (200))
											// ���M�o�b�t�@�T�C�Y * 2ms(4800bps) * 1.4(40%�}�[�W��)
// MH321800(E) G.So IC�N���W�b�g�Ή�

#define		CN_WAIT_S	26					/* 500�` 520ms 		*/
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

#define		CN_ESCROW		4				/* ������ر���A���ɖ����Z�o�܂ł̎��� */
											/* 100�~��ݴ��ۂ̂Ƃ���4�b�ɂ��� */

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
extern	unsigned char 	Cnm_End_PayTimOut;/* �x�����I���t���O				*/
extern	unsigned char	ntr_sts;			/* Send Status for Note 		*/
extern	unsigned short 	sfstat;				/* Status for Safe cal. 		*/
extern	unsigned short 	cn_polcnt;			/* Status for Safe cal. 		*/
extern	unsigned char	cn_errst[2];		/* Communication  Status 		*/
extern	unsigned char	cn_com;				/* Output data Flg to Coin Mech */
extern	unsigned char	nt_com;				/* Output data Flg to Note Reader */
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
extern	unsigned char	cn_com_bak;			// cn_com�̃o�b�N�A�b�v
extern	unsigned char	nt_com_bak;			// nt_com�̃o�b�N�A�b�v
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
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
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern volatile short TXRXWAIT;				/* Send flg						*/
// MH321800(E) G.So IC�N���W�b�g�Ή�
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

extern		char	RXDTCT;					/* ��M���ׂ��ް��޲Đ�			*/

extern		short	CN_tsk_timer;			/* cnmtask Control Timer		*/

extern		short	CN_escr_timer;			/* cnmtask escrow Timer			*/
extern		short	Tubu_cnt_set;			/* �����ݒ肠��^�Ȃ�			*/
extern		short	CN_refund;				/* ���o���׸�					*/
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
extern		short	CN_SUB_SET[3];			/* �������ށA�\�~����ݒ���	*/
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
	unsigned char	r_dat07[12];			/* Tube Count(���ɖ����Z�o�p)	*/
	unsigned char	r_dat0a[4];				/* Refund Count					*/
	short		in_dat07[4];				/* Tube Count(���ɖ����Z�o�p)	*/
	short		out_dat07[4];				/* Tube Count(���ɖ����Z�o�p)	*/
	short		r_add08[4];					/* Save Credit value(���ɖ����Z�o�p) */
	short		r_add0a[4];					/* Save refund value(���ɖ����Z�o�p) */
	short		safe_dt[4];					/* Coin Safe Count(���ɖ���)	*/
	short		safe_dt_wk[4];				/* safe_dt�̕��d�pܰ�			*/
	short		safe_inout;					/* 0:���o�Ȃ��A1:���o����		*/
	short		cn_escrow;					/* ��ݴ��ے��׸�				*/
	short		safe_cal_do;				/* ���ɖ����Z�o�w���׸�			*/
	short		nt_escrow;					/* �������ے��׸�				*/
	short		nt_safe_dt;					/* Note Safe Count				*/
	long		refval;						/* Refund Value					*/
	long		reffal;						/* Refund Fault Value			*/
	short		powoff;						/* Power off flg  				*/
	short		work[2];					/* work area					*/
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
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
		unsigned char		yobi		:6;			// �\��
		unsigned char		standby_snd_after:1;	// �X�^���o�C���M�t���O(r_dat09_rcv_wait�ƃZ�b�g�Ŏg�p)
		unsigned char		r_dat09_rcv_wait:1;		// �N�����̕��o�\�����f�[�^��M�҂��t���O
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
