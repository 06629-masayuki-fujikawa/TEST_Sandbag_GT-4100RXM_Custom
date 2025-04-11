/*[]----------------------------------------------------------------------[]*/
/*| cnmtaskﾃﾞｰﾀ定義                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 2001-11-01                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	"cnm_def.h"

unsigned char 	cnm_sts = 0;				/* Send Status for Coinmech 	*/
unsigned char 	Cnm_End_PayTimOut;			/*  	*/
unsigned char	ntr_sts = 0;				/* Send Status for Note 		*/
unsigned short 	sfstat = 0;					/* Status for Safe cal. 		*/
unsigned short 	cn_polcnt = 0;				/* Status for Safe cal. 		*/
unsigned char	cn_errst[2] = { 0 };		/* Communication  Status 		*/
unsigned char	cn_com = 0;					/* Output data Flg to Coin Mech */
unsigned char	nt_com = 0;					/* Output data Flg to Note Reader */
// MH810103 GG119202(S) JVMAリセット処理変更
unsigned char	cn_com_bak = 0;				// cn_comのバックアップ
unsigned char	nt_com_bak = 0;				// nt_comのバックアップ
// MH810103 GG119202(E) JVMAリセット処理変更
unsigned char	csd_buf[32] = { 0 };		/* Send Buffer (Temporary) 		*/
short	csd_siz = 0;						/* Number of Send Char. 		*/
unsigned char	crd_buf[R_BUFSIZ] = { 0 };	/* Receive Buffer (Temporary) 	*/
short	crd_siz = 0;						/* Number of Receive Char. 		*/
short	cnm_ope = 0;						/* Init Coin mech & Note Reader */
short	tn_save	= 0;						/* Com. stat =0:Coin Mech  =1:Note Reader */

/*	<<< Coin Mech Communication Control Data >>>	*/
											/*------------------------------*/
short			TXSDCM = -1;				/* Send Out Command       		*/
											/*------------------------------*/
// MH321800(S) G.So ICクレジット対応
//unsigned char	TXBUFF[32] = { 0 };			/* Send buffer					*/
unsigned char	TXBUFF[T_BUFSIZ] = { 0 };	/* Send buffer					*/
// MH321800(E) G.So ICクレジット対応
short			TXCONT = 0;					/* Number of Send Data Size    	*/
short			TXRPTR = 0;					/* Send buffer pointer			*/
short			TXRSLT = 0;					/* Send Result					*/
short			TXWAIT = -1;				/* Send flg						*/
											/*------------------------------*/
unsigned char	RXBUFF[R_BUFSIZ] = { 0 };	/* Recive buffer				*/
short			RXCONT = 0;					/* Number of Recive Data Size	*/
short			RXWAIT = -1;				/* Send flg						*/
// MH321800(S) G.So ICクレジット対応
volatile short	TXRXWAIT = -1;				/* Send flg						*/
// MH321800(E) G.So ICクレジット対応
											/*------------------------------*/
union	RXRSLT_rec	RXRSLT = { 0 };			/* Recive Result				*/
char			RXDTCT = 0;					/* 受信すべきﾃﾞｰﾀﾊﾞｲﾄ数			*/
											/*------------------------------*/


/*	<<< Coin Mech Communication Data >>>	*/

											/*------------------------------*/
struct	CNM_REC_REC CN_RDAT = { 0 };		/* Coin Mech Recive data		*/
											/*------------------------------*/
struct	CNM_SND_REC CN_SDAT = { 0 };		/* Coin Mech Send data			*/
											/* -----------------------------*/
short	CN_SUB_SET[3] = { 0 };				/* ｻﾌﾞﾁｭｰﾌﾞ、予蓄金種設定情報	*/
											/* -----------------------------*/


/*	<<< Note Reader Communication Data >>> 	*/

struct	NTR_REC_REC	NT_RDAT = { 0 };
struct	NTR_SND_REC	NT_SDAT = { 0 };

/*	<<< C/M,N/R Que Contents >>>	*/

short	CN_tsk_timer = 0;					/* cnmtask Control Timer			*/

short	CN_escr_timer = 0;
short	Tubu_cnt_set = 0;
short	CN_refund = 0;

unsigned char	CNM_SYN_Time = 0;
unsigned char Cnm_Force_Reset;
/* 金種定義 */

const short	coin_vl[] = { 10, 50, 100, 500 };

/* 入金最大枚数 */
const unsigned char	co_max[] = { 0x40, 0x18, 0x30, 0x19 };

const unsigned char sv_coin_max[4] = { 0x98, 0x98, 0x98, 0x98 };

/* ｺｲﾝﾒｯｸ ｺﾏﾝﾄﾞ */
const unsigned char	com_tbl[][2] = {
	{ 0x60, 0x9f },
	{ 0x61, 0x9e },
	{ 0x62, 0x9d },
	{ 0x63, 0x9c },
	{ 0x64, 0x9b },
	{ 0x65, 0x9a }};

/* 紙幣ﾘｰﾀﾞｰｺﾏﾝﾄﾞ */
const unsigned char	cnt_tbl[][2] = {
	{ 0x58, 0xa7 },
	{ 0x59, 0xa6 },
	{ 0x5a, 0xa5 },
	{ 0x5b, 0xa4 },
	{ 0x5c, 0xa3 },
	{ 0x5d, 0xa2 }};

/* 応答ｺｰﾄﾞ */
const unsigned char	ack_tbl[] = 
	{ 0x11, 0x22, 0x33, 0x44, 0x55, 0xee };

CNM_CTRL_INFO	cnm_ctrl_info;
