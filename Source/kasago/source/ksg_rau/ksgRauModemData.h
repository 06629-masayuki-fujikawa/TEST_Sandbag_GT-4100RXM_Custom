/************************************************************************************************/
/*																								*/
/*	ﾓｼﾞｭｰﾙ名称	:ksgRauModemData.h	:---------------------------------------------------------: */
/*	ﾓｼﾞｭｰﾙｼﾝﾎﾞﾙ	:					:ﾓﾃﾞﾑﾃﾞｰﾀ関連											  : */
/*																								*/
/************************************************************************************************/
#ifndef	_KSG_RAU_MODEMDATA_H
#define	_KSG_RAU_MODEMDATA_H
																		/*							*/
#include	"ksgmac.h"													/* MAC 搭載機能ﾍｯﾀﾞ			*/
																		/*							*/
#define		KSG_P_TCP					0								/* 							*/
#define		KSG_P_UDP					1								/* 							*/
#define		KSG_SET						1								/*  						*/
#define		KSG_CLR						0								/* 							*/
#define		KSG_CS_SIG					1								/*							*/
#define		KSG_DR_SIG					2								/* 							*/
#define		KSG_CD_SIG					4								/*							*/
#define		KSG_CI_SIG					8								/*							*/
#define		KSG_LF						0x0a							/*							*/
#define		KSG_CR						0x0d							/*							*/
#define		KSG_RSSIG_CHT_BITCNT		0x1F							/* ﾁｬﾀ取り判定回数:LSBからのﾋﾞｯﾄ数回 (1-8)*/
#define		KSG_SEND_MAX_MDM			128								/* 送信ｴﾘｱMAXｻｲｽﾞ			*/
#define		KSG_RCV_MAX_MDM				128								/* 受信ｴﾘｱMAXｻｲｽﾞ			*/
#define		KSG_Tm_Gurd_COUNT			-100							/*  （100msﾍﾞｰｽ） 10秒		*/
#define		KSG_Tm_AtCom_3s				-30								/* 3.0s ﾀｲﾏ値				*/
#define		KSG_Tm_AtCom_2s				-20								/* 2.0s ﾀｲﾏ値				*/
#define		KSG_Tm_AtCom_02s			-2								/* 0.2s ﾀｲﾏ値				*/
#define		KSG_Tm_PPPcnct_COUNT		-600							/*  （100msﾍﾞｰｽ） 60秒		*/
#define		KSG_Tm_TCPcnct_COUNT		-300							/*  （100msﾍﾞｰｽ） 30秒		*/
#define		KSG_Tm_discnnect_COUNT		-300							/*  （100msﾍﾞｰｽ） 30秒		*/
#define		KSG_RCV_MAX_SCI7			18								/* 受信最大文字数			*/
																		/*							*/
#define		KSG_f_mdm_init_err			KSG_mdm_e_flag.BIT.B0			/* ﾓﾃﾞﾑ初期化要求ｴﾗｰ		*/
#define		KSG_f_mdm_kind_err			KSG_mdm_e_flag.BIT.B1			/* 処理区分設定				*/
#define		KSG_f_mdm_conn_err			KSG_mdm_e_flag.BIT.B2			/* ﾎｽﾄ接続ｴﾗｰ				*/
#define		KSG_f_ReDial_rq				KSG_Tm_ReDial.BIT.f_rq			/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ ﾄﾘｶﾞ		*/
#define		KSG_f_ReDial_ov				KSG_Tm_ReDial.BIT.f_ov			/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ ｵｰﾊﾞｰﾌﾛｰ	*/
#define		KSG_f_Gurd_rq				KSG_Tm_Gurd.BIT.f_rq			/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏ ﾄﾘｶﾞ		*/
#define		KSG_f_Gurd_ov				KSG_Tm_Gurd.BIT.f_ov			/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏ ｵｰﾊﾞｰﾌﾛｰ	*/
#define		KSG_f_DialConn_rq			KSG_Tm_DialConn.BIT.f_rq		/* 							*/
#define		KSG_f_DialConn_ov			KSG_Tm_DialConn.BIT.f_ov		/* 							*/
#define		KSG_f_No_Action_rq			KSG_Tm_No_Action.BIT.f_rq		/* 							*/
#define		KSG_f_No_Action_ov			KSG_Tm_No_Action.BIT.f_ov		/* 							*/
#define		KSG_f_TCPcnct_rq			KSG_Tm_TCP_CNCT.BIT.f_rq		/* 							*/
#define		KSG_f_TCPcnct_ov			KSG_Tm_TCP_CNCT.BIT.f_ov		/* 							*/
#define		KSG_f_discnnect_rq			KSG_Tm_DISCONNECT.BIT.f_rq		/* ｺﾈｸｼｮﾝ切断待ﾀｲﾏ ﾄﾘｶﾞ		*/
#define		KSG_f_discnnect_ov			KSG_Tm_DISCONNECT.BIT.f_ov		/* ｺﾈｸｼｮﾝ切断待ﾀｲﾏ ｵｰﾊﾞｰﾌﾛｰ	*/
#define		KSG_f_data_rty_rq			KSG_Tm_DataRetry.BIT.f_rq		/* 							*/
#define		KSG_f_data_rty_ov			KSG_Tm_DataRetry.BIT.f_ov		/* 							*/
#define		KSG_f_TCPdiscnct_rq			KSG_Tm_TCP_DISCNCT.BIT.f_rq		/* 							*/
#define		KSG_f_TCPdiscnct_ov			KSG_Tm_TCP_DISCNCT.BIT.f_ov		/* 							*/
#define		KSG_f_ReDialTime_rq			KSG_Tm_ReDialTime.BIT.f_rq		/* ｻｰﾊﾞｰ復旧待ﾀｲﾏ  ﾄﾘｶﾞ		*/
#define		KSG_f_ReDialTime_ov			KSG_Tm_ReDialTime.BIT.f_ov		/* ｻｰﾊﾞｰ復旧待ﾀｲﾏ  ｵｰﾊﾞｰﾌﾛｰ	*/
#define		KSG_f_Ackwait_rq			KSG_Tm_Ackwait.BIT.f_rq			/* ﾘｸｴｽﾄ					*/
#define		KSG_f_Ackwait_ov			KSG_Tm_Ackwait.BIT.f_ov			/* ﾀｲﾑｵｰﾊﾞｰ					*/
																		/*							*/
#define		KSG_MDM_DATA_MAX			128								/*							*/
#define		KSG_f_host_conn_req			KSG_mdm_flag.BIT.B0				/* ﾎｽﾄ接続要求				*/
#define		KSG_Tm_FomaMdmDRwait		-50								// 5s ﾀｲﾏ //
																		/*							*/
typedef volatile union {												/*  時間管理				*/
	struct	{															/*  	Bitｱｸｾｽ				*/
		unsigned short		f_ov	: 1  ;								/*  	 bit 15				*/
		unsigned short		f_rq	: 1  ;								/*  	 bit 14				*/
		unsigned short		bit0 	: 14 ;								/*  	 bit 0-13			*/
	} BIT;																/*							*/
	unsigned short			tm;											/*   Byteｱｸｾｽ				*/
}	TIME_AREA;															/*							*/
																		/*							*/
typedef volatile union {												/*  時間管理				*/
	struct	{															/*  	Bitｱｸｾｽ				*/
		unsigned long		f_ov	: 1  ;								/*  	 bit 31				*/
		unsigned long		f_rq	: 1  ;								/*  	 bit 30				*/
		unsigned long		bit0 	: 30 ;								/*  	 bit 0-29			*/
	} BIT;																/*							*/
	unsigned long			tm;											/*   Byteｱｸｾｽ				*/
}	TIME_AREA_L;														/*							*/
																		/*							*/
typedef volatile struct {												/*							*/
	unsigned short			Len;										/*							*/
	unsigned char			ch;											/*							*/
	unsigned char			Dat[KSG_MDM_DATA_MAX];						/*							*/
} t_MODEM_DATA;															/*							*/
																		/*							*/
typedef volatile struct {												/*							*/
	unsigned char			Count;										/*							*/
	unsigned char			ReadIdx;									/*							*/
	unsigned char			WriteIdx;									/*							*/
	unsigned char			dummy;										/*							*/
	t_MODEM_DATA			modem_data[4];								/*							*/
} t_MODEM_Ctrl;															/*							*/
																		/*							*/
typedef volatile union {												/*							*/
		struct	{														/*							*/
			unsigned char 	dmy	:5;										/*							*/
			unsigned char 	ing	:1;										/* ﾀﾞｲｱﾙ中					*/
			unsigned char 	in	:1;										/* 着信						*/
			unsigned char 	out	:1;										/* 発信						*/
		} BIT;															/*							*/
		unsigned char		BYTE;										/*							*/
} DIAL_STATS;															/*							*/
																		/*							*/
extern  signed short		KSG_SCI0_CHR_TIM;							/* 文字間監視ﾀｲﾏ			*/
extern	unsigned char		KSG_line_stat_h;							/* ﾎｽﾄ ﾗｲﾝｽﾃｰﾀｽ				*/
extern	unsigned char		KSG_wait_CS;								/* 							*/
extern	unsigned char		KSG_RauModemOnline;							/* ﾎｽﾄ回線状態ﾌﾗｸﾞ			*/
extern	unsigned char		KSG_mdm_cmd_len;							/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信ﾃﾞｰﾀ長		*/
extern	unsigned char		KSG_mdm_cmd_ptr;							/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信ﾃﾞｰﾀﾎﾟｲﾝﾀ	*/
extern	unsigned char		KSG_code_sts;								/* CR,LF 検出ｽﾃｰﾀｽ			*/
extern	unsigned char		KSG_mdm_r_buf[];							/* SIO0 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ		*/
extern	unsigned char		*KSG_ptr_rx_save;							/* SIO0 受信ﾃﾞｰﾀ格納ﾎﾟｲﾝﾀ	*/
extern	unsigned char		KSG_rx_save_len;							/* SIO0 受信ﾃﾞｰﾀ格納数		*/
extern	unsigned char		KSG_mdm_cmd_buf[];							/* ﾓﾃﾞﾑｺﾏﾝﾄﾞﾊﾞｯﾌｧ			*/
extern	unsigned char		KSG_sent_h;									/* ﾊﾟｹｯﾄ送信終了ﾌﾗｸﾞ		*/
extern	unsigned char		KSG_DSR;									/* ﾁｬﾀ取り後のDSR値 		*/
extern	unsigned char		KSG_CD;										/* ﾁｬﾀ取り後のCD値  		*/
extern	unsigned char		KSG_CI;										/* ﾁｬﾀ取り後のRI値 			*/
extern	unsigned char		KSG_CTS;									/* ﾁｬﾀ取り後のCTS値 		*/
extern unsigned char		KSG_Chatt_DSR;								/* ﾁｬﾀ取り用 生ﾃﾞｰﾀ保存域	*/
extern unsigned char		KSG_Chatt_CTS;								/* ﾁｬﾀ取り用 生ﾃﾞｰﾀ保存域	*/
extern unsigned char		KSG_Chatt_CD;								/* ﾁｬﾀ取り用 生ﾃﾞｰﾀ保存域	*/
extern unsigned char		KSG_Chatt_CI;								/* ﾁｬﾀ取り用 生ﾃﾞｰﾀ保存域	*/
extern	unsigned char		KSG_TCP_conct_cnt;							/* TCPｺﾈｸｼｮﾝ接続ﾘﾄﾗｲ回数	*/
extern	TIME_AREA			KSG_Tm_ReDial;								/* ﾘﾀﾞｲｱﾙﾀｲﾏ				*/
extern	TIME_AREA			KSG_Tm_Gurd;								/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏ			*/
extern	TIME_AREA			KSG_Tm_DialConn;							/* 回線接続待ちﾀｲﾏ			*/
extern	TIME_AREA			KSG_Tm_AtCom;								/* ATｺﾏﾝﾄﾞﾀｲﾏ				*/
extern	TIME_AREA_L			KSG_Tm_No_Action;							/* 無通信ﾀｲﾏ(秒)			*/
extern	TIME_AREA			KSG_Tm_TCP_CNCT;							/* TCPｺﾈｸｼｮﾝ接続待ちﾀｲﾏ		*/
extern	TIME_AREA			KSG_Tm_DISCONNECT;							/* ｺﾈｸｼｮﾝ切断待ちﾀｲﾏ		*/
extern	TIME_AREA			KSG_Tm_DataRetry;							/* ﾃﾞｰﾀ再送待ちﾀｲﾏ(分) 		*/
extern	TIME_AREA			KSG_Tm_TCP_DISCNCT;							/* TCPｺﾈｸｼｮﾝ切断待ちﾀｲﾏ		*/
extern	TIME_AREA			KSG_Tm_ReDialTime;							/* ﾘﾀﾞｲﾔﾙ中ﾀｲﾏ				*/
extern	TIME_AREA			KSG_Tm_Ackwait;								/* ACK待ちﾀｲﾏ				*/
extern	unsigned short		KSG_Tm_Reset_t;								/* 機器ﾘｾｯﾄ出力ﾀｲﾏ			*/
extern	unsigned short		KSG_Tm_ResetPulse;							/* 機器ﾘｾｯﾄパルス出力ﾀｲﾏ	*/
extern	DIAL_STATS			KSG_dials;									/* ﾀﾞｲｱﾙｽﾃｰﾀｽ				*/
extern	unsigned short		KSG_net_rev_cmd;							/* USNET 受信ｺﾏﾝﾄﾞ			*/
extern	unsigned short		KSG_net_send_cmd;							/* USNET 送信ｺﾏﾝﾄﾞ			*/
extern	unsigned char		KSG_dial_go;								/* ﾀﾞｲｱﾙ開始(0:切断,1:開始)	*/
extern	unsigned char		KSG_net_online;								/* ｵﾝﾗｲﾝﾌﾗｸﾞ				*/
extern	unsigned char		KSG_mdm_mode;								/* ﾓﾃﾞﾑﾓｰﾄﾞ(0:ｺﾏﾝﾄﾞ,1:ﾃﾞｰﾀ)	*/
extern	unsigned char		KSG_mdm_init_sts;							/* ﾓﾃﾞﾑ初期化ｽﾃｰﾀｽ			*/
extern	unsigned char		KSG_mdm_ant_sts;							/* ｱﾝﾃﾅﾚﾍﾞﾙ取得ｽﾃｰﾀｽ		*/
extern	unsigned short		KSG_Cnt_Retry;								/* 接続ﾘﾄﾗｲｶｳﾝﾀ				*/
extern	unsigned char		KSG_f_init_repeat;							/* 初期化回数				*/
extern	BITS				KSG_mdm_Start;								/* Dopa start flag			*/
extern	BITS				KSG_mdm_ActFunc;							/* active function flag		*/
extern	BITS				KSG_mdm_e_flag;								/* ﾓﾃﾞﾑｴﾗｰﾌﾗｸﾞ				*/
extern	unsigned char		KSG_mdm_Condit;								/* task condition			*/
extern	unsigned char		KSG_txdata_mdm[KSG_SEND_MAX_MDM];			/* 送信電文ﾊﾞｯﾌｧ			*/
extern	unsigned int		KSG_txdlen_mdm;								/* 送信ﾃﾞｰﾀ長				*/
extern	unsigned char		KSG_txdch_mdm;								/* 送信ｿｹｯﾄ(UDPのみ)		*/
extern	unsigned char		KSG_txdata_bak[KSG_SEND_MAX_MDM];			/* 送信電文ﾊﾞｯﾌｧ			*/
extern	unsigned int		KSG_txdlen_bak;								/* 送信ﾃﾞｰﾀ長				*/
extern	unsigned char		KSG_txdch_bak;								/* 送信ｿｹｯﾄ(UDPのみ)		*/
extern	unsigned char		KSG_rxdata_mdm[KSG_RCV_MAX_MDM];			/* 受信電文ﾊﾞｯﾌｧ			*/
extern	unsigned int		KSG_rxdlen_mdm;								/* 受信ﾃﾞｰﾀ長				*/
extern	unsigned char		KSG_mdm_f_TmStart;							/* 							*/
extern	unsigned char		KSG_connect_retry_flg;						/* PPP再接続要求			*/
extern	unsigned char		KSG_disconect_flg;							/* TCPｺﾈｸｼｮﾝ切断ﾌﾗｸﾞ		*/
extern	  signed char		KSG_vccs_authsel;							/* PPP認証方法				*/
extern	unsigned char		KSG_mdm_status;								/* ﾓﾃﾞﾑｽﾃｰﾀｽ				*/
extern	BITS				KSG_mdm_flag;								/* ﾓﾃﾞﾑﾌﾗｸﾞ					*/
extern	unsigned char		KSG_modem_req;								/* ﾓﾃﾞﾑ接続ﾘｸｴｽﾄﾌﾗｸﾞ		*/
extern	unsigned char		KSG_mdm_cut_req;							/* ﾓﾃﾞﾑ切断ﾘｸｴｽﾄﾌﾗｸﾞ		*/
extern	unsigned char		KSG_mdm_TCP_CNCT;							/* TCPｺﾈｸｼｮﾝ状態ﾌﾗｸﾞ		*/
extern	unsigned char		KSG_mdm_TCPcnct_req;						/* TCPｺﾈｸｼｮﾝﾘｸｴｽﾄﾌﾗｸﾞ		*/
extern	unsigned char		KSG_tcp_inout;								/* TCP発信着信起動状態		*/
extern	unsigned char		KSG_tcp_in_flg;								/* TCP着信接続時切断ﾌﾗｸﾞ	*/
extern	unsigned char		KSG_ppp_info;								/* PPPの状態				*/
extern	t_MODEM_Ctrl		KSG_mdm_Snd_Ctrl;							/*							*/
extern	t_MODEM_Ctrl		KSG_mdm_Rcv_Ctrl;							/*							*/
extern  TIME_AREA_L			KSG_Tm_DopaReset;							/*  HOST無応答時のDOPAﾘｾｯﾄ	*/

extern TIME_AREA		KSG_Tm_ERDR_TIME;								// ER->DR HIGH 監視ﾀｲﾏｰ
extern unsigned char	KSG_uc_FomaFlag;								// 0=Dopa / 1=Foma
extern unsigned char	KSG_uc_AdapterType;								// サービス毎使用する機器タイプをセット
extern unsigned char	KSG_uc_AtRetryCnt;								// 3 times retury counter
extern unsigned char	KSG_uc_AtResultWait;							// 0:-- 1:Waiting for Result Code
extern unsigned char	KSG_uc_ConnAftWait;								// CONNECT ( 2s ) -> PPP
extern unsigned char	KSG_uc_Foma_APN[32];							// Access Point Name
extern unsigned char	KSG_uc_FomaWaitDRFlag;							// 100msec. ER ON 中
extern unsigned char	KSG_uc_FomaPWRphase;							// 電源管理フェーズ 0=IDLE, 1=CS off 監視, 2=MODEM OFF要求中
extern unsigned char	KSG_uc_MdmInitRetry;							// モデム電源制御リトライ 電源ON失敗時リトライカウンタ

#define		KSG_f_ERDRtime_ov 			KSG_Tm_ERDR_TIME.BIT.f_ov
#define		KSG_f_ERDRtime_rq 			KSG_Tm_ERDR_TIME.BIT.f_rq

#define		KSG_Tm_FomaResetToGurd		-100			// MODEM 電源ON から Dial 可能状態までのｳｪｲﾄ

#define		KSG_FomaModemPwrPhase1		600				// Foma Modem 電源Off 時 CD 監視時間(60秒)
#define		KSG_FomaModemPwrPhase2		200				// Foma Modem 電源 PWR HIGHの時間(20秒)

#define		KSG_FomaModemPwrPhase3		50				// Foma Modem 電源 ON までのｲﾝﾀｰﾊﾞﾙ 5秒[GuidLine] //

#endif		//_KSG_RAU_MODEMDATA_H
