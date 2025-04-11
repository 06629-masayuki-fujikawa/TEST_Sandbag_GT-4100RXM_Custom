/************************************************************************************************/
/*																								*/
/*	ﾓｼﾞｭｰﾙ名称	:ksgRauModemData.c	:---------------------------------------------------------: */
/*	ﾓｼﾞｭｰﾙｼﾝﾎﾞﾙ	:					: ﾓﾃﾞﾑ ﾃﾞｰﾀ関連											  : */
/*																								*/
/************************************************************************************************/
#include	"ksgRauModem.h"												/* ﾓﾃﾞﾑ関連関数郡			*/
#include	"ksgRauModemData.h"											/* ﾓﾃﾞﾑ関連ﾃﾞｰﾀ定義			*/
#include	"ksgmac.h"													/* MAC 搭載機能ﾍｯﾀﾞ			*/
																		/*							*/
TIME_AREA		KSG_Tm_ReDial;											/* ﾘﾀﾞｲｱﾙﾀｲﾏ				*/
TIME_AREA		KSG_Tm_Gurd;											/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏ			*/
TIME_AREA		KSG_Tm_DialConn;										/* 回線接続待ちﾀｲﾏ			*/
TIME_AREA		KSG_Tm_AtCom;											/* ATｺﾏﾝﾄﾞﾀｲﾏ				*/
TIME_AREA_L		KSG_Tm_No_Action;										/* 無通信ﾀｲﾏ				*/
TIME_AREA		KSG_Tm_TCP_CNCT;										/* TCPｺﾈｸｼｮﾝ接続待ちﾀｲﾏ		*/
TIME_AREA		KSG_Tm_DISCONNECT;										/* ｺﾈｸｼｮﾝ切断待ちﾀｲﾏ		*/
TIME_AREA		KSG_Tm_DataRetry;										/* ﾃﾞｰﾀ再送待ちﾀｲﾏ(分)		*/
TIME_AREA		KSG_Tm_TCP_DISCNCT;										/* TCPｺﾈｸｼｮﾝ切断待ちﾀｲﾏ		*/
TIME_AREA		KSG_Tm_ReDialTime;										/* ﾘﾀﾞｲﾔﾙ中ﾀｲﾏ				*/
TIME_AREA		KSG_Tm_Ackwait;											/* ACK待ちﾀｲﾏ				*/
unsigned short	KSG_Tm_Reset_t;											/* 機器ﾘｾｯﾄ出力ﾀｲﾏ			*/
unsigned short	KSG_Tm_ResetPulse;										/* 機器ﾘｾｯﾄパルス出力ﾀｲﾏ	*/
unsigned char	KSG_line_stat_h;										/* ﾎｽﾄ ﾗｲﾝｽﾃｰﾀｽ				*/
unsigned char	KSG_wait_CS;											/*							*/
unsigned char	KSG_RauModemOnline;										/* ﾎｽﾄ回線状態ﾌﾗｸﾞ			*/
unsigned char	KSG_mdm_cmd_len;										/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信ﾃﾞｰﾀ長		*/
unsigned char	KSG_mdm_cmd_ptr;										/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信ﾃﾞｰﾀﾎﾟｲﾝﾀ	*/
unsigned char	KSG_code_sts;											/* CR,LF 検出ｽﾃｰﾀｽ			*/
unsigned char	KSG_mdm_r_buf[KSG_RCV_MAX_SCI7];						/* SIO0 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ		*/
unsigned char	*KSG_ptr_rx_save;										/* SIO0 受信ﾃﾞｰﾀ格納ﾎﾟｲﾝﾀ	*/
unsigned char	KSG_rx_save_len;										/* SIO0 受信ﾃﾞｰﾀ格納数		*/
unsigned char	KSG_mdm_cmd_buf[128];									/* ﾓﾃﾞﾑｺﾏﾝﾄﾞﾊﾞｯﾌｧ			*/
unsigned char	KSG_sent_h;												/* ﾊﾟｹｯﾄ送信終了ﾌﾗｸﾞ		*/
unsigned char	KSG_DSR;												/* ﾁｬﾀ取り後のDSR値 		*/
unsigned char	KSG_CD;													/* ﾁｬﾀ取り後のCD値  		*/
unsigned char	KSG_CI;													/* ﾁｬﾀ取り後のRI値 			*/
unsigned char	KSG_CTS;												/* ﾁｬﾀ取り後のCTS値 		*/
unsigned char	KSG_Chatt_DSR;											/* ﾁｬﾀ取り用 生ﾃﾞｰﾀ保存域	*/
unsigned char	KSG_Chatt_CTS;											/* ﾁｬﾀ取り用 生ﾃﾞｰﾀ保存域	*/
unsigned char	KSG_Chatt_CD;											/* ﾁｬﾀ取り用 生ﾃﾞｰﾀ保存域	*/
unsigned char	KSG_Chatt_CI;											/* ﾁｬﾀ取り用 生ﾃﾞｰﾀ保存域	*/
																		/*							*/
DIAL_STATS		KSG_dials;												/* ﾀﾞｲｱﾙｽﾃｰﾀｽ				*/
unsigned short	KSG_net_rev_cmd;										/* USNET 受信ｺﾏﾝﾄﾞ			*/
unsigned short	KSG_net_send_cmd;										/* USNET 送信ｺﾏﾝﾄﾞ			*/
unsigned char	KSG_dial_go;											/* ﾀﾞｲｱﾙ開始				*/
unsigned char	KSG_net_online;											/* ｵﾝﾗｲﾝﾌﾗｸﾞ				*/
unsigned char	KSG_modem_req;											/* ﾓﾃﾞﾑ接続ﾘｸｴｽﾄﾌﾗｸﾞ		*/
unsigned char	KSG_mdm_mode;											/* ﾓﾃﾞﾑﾓｰﾄﾞ(0:ｺﾏﾝﾄﾞ,1:ﾃﾞｰﾀ)	*/
unsigned char	KSG_mdm_status;											/* ﾓﾃﾞﾑｽﾃｰﾀｽ				*/
unsigned char	KSG_mdm_init_sts;										/* ﾓﾃﾞﾑ初期化ｽﾃｰﾀｽ			*/
unsigned char	KSG_mdm_ant_sts;										/* ｱﾝﾃﾅﾚﾍﾞﾙ取得ｽﾃｰﾀｽ		*/
unsigned short	KSG_Cnt_Retry;											/* 接続ﾘﾄﾗｲｶｳﾝﾀ				*/
unsigned char	KSG_f_init_repeat;										/* 初期化回数				*/
																		/*							*/
BITS			KSG_mdm_Start;											/* start flag				*/
BITS			KSG_mdm_ActFunc;										/* active function flag		*/
BITS			KSG_mdm_flag;											/* ﾓﾃﾞﾑﾌﾗｸﾞ					*/
BITS			KSG_mdm_e_flag;											/* ﾓﾃﾞﾑｴﾗｰﾌﾗｸﾞ				*/
																		/*							*/
t_MODEM_Ctrl	KSG_mdm_Snd_Ctrl;										/*							*/
t_MODEM_Ctrl	KSG_mdm_Rcv_Ctrl;										/*							*/
unsigned char	KSG_mdm_Condit;											/* task condition			*/
unsigned char	KSG_txdata_mdm[KSG_SEND_MAX_MDM];						/* 送信電文ﾊﾞｯﾌｧ			*/
unsigned int	KSG_txdlen_mdm;											/* 送信ﾃﾞｰﾀ長				*/
unsigned char	KSG_txdch_mdm;											/* 送信ｿｹｯﾄ(UDPのみ)		*/
unsigned char	KSG_txdata_bak[KSG_SEND_MAX_MDM];						/* 送信電文ﾊﾞｯﾌｧ			*/
unsigned int	KSG_txdlen_bak;											/* 送信ﾃﾞｰﾀ長				*/
unsigned char	KSG_txdch_bak;											/* 送信ｿｹｯﾄ(UDPのみ)		*/
unsigned char	KSG_rxdata_mdm[KSG_RCV_MAX_MDM];						/* 受信電文ﾊﾞｯﾌｧ			*/
unsigned int	KSG_rxdlen_mdm;											/* 受信ﾃﾞｰﾀ長				*/
unsigned char	KSG_mdm_f_TmStart;										/* 							*/
unsigned char	KSG_connect_retry_flg;									/* PPP再接続要求			*/
unsigned char	KSG_mdm_cut_req;										/* modem切断ﾘｸｴｽﾄﾌﾗｸﾞ		*/
unsigned char	KSG_mdm_TCP_CNCT;										/* TCPｺﾈｸｼｮﾝ状態ﾌﾗｸﾞ		*/
unsigned char	KSG_disconect_flg;										/* TCPｺﾈｸｼｮﾝ切断ﾌﾗｸﾞ		*/
unsigned char	KSG_mdm_TCPcnct_req;									/* TCPｺﾈｸｼｮﾝﾘｸｴｽﾄﾌﾗｸﾞ		*/
unsigned char	KSG_TCP_conct_cnt;										/* TCPｺﾈｸｼｮﾝ接続ﾘﾄﾗｲ回数	*/
  signed char	KSG_vccs_authsel;										/* PPP認証方法				*/
unsigned char	KSG_tcp_inout;											/* TCP発信着信起動状態		*/
unsigned char	KSG_tcp_in_flg;											/* TCP着信接続時切断ﾌﾗｸﾞ	*/
unsigned char	KSG_ppp_info;											/* PPPの状態				*/
TIME_AREA_L		KSG_Tm_DopaReset;										/*  HOST無応答時のDOPAﾘｾｯﾄ	*/
TIME_AREA		KSG_Tm_ERDR_TIME;										// ER->DR HIGH 監視ﾀｲﾏｰ
unsigned char	KSG_uc_FomaFlag;										// 0=Dopa / 1=Foma
																		// ↑通信サービスのタイプをセット
unsigned char	KSG_uc_AdapterType;										// サービス毎使用する機器タイプをセット
unsigned char	KSG_uc_AtRetryCnt;										// 3 times retury counter
unsigned char	KSG_uc_AtResultWait;									// 0:-- 1:Waiting for Result Code
unsigned char	KSG_uc_ConnAftWait;										// CONNECT ( 2s ) -> PPP
unsigned char	KSG_uc_Foma_APN[32];									// Access Point Name
unsigned char	KSG_uc_FomaWaitDRFlag;									// 100msec. ER ON 中
unsigned char	KSG_uc_FomaPWRphase;									// 電源管理フェーズ 0=IDLE, 1=CS off 監視, 2=MODEM OFF要求中
unsigned char	KSG_uc_MdmInitRetry;									// モデム電源制御リトライ 電源ON失敗時リトライカウンタ

