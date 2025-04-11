/************************************************************************************************/
/*																								*/
/*	Ӽޭ�ٖ���	:ksgRauModemData.h	:---------------------------------------------------------: */
/*	Ӽޭ�ټ����	:					:�����ް��֘A											  : */
/*																								*/
/************************************************************************************************/
#ifndef	_KSG_RAU_MODEMDATA_H
#define	_KSG_RAU_MODEMDATA_H
																		/*							*/
#include	"ksgmac.h"													/* MAC ���ڋ@�\ͯ��			*/
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
#define		KSG_RSSIG_CHT_BITCNT		0x1F							/* �����蔻���:LSB������ޯĐ��� (1-8)*/
#define		KSG_SEND_MAX_MDM			128								/* ���M�رMAX����			*/
#define		KSG_RCV_MAX_MDM				128								/* ��M�رMAX����			*/
#define		KSG_Tm_Gurd_COUNT			-100							/*  �i100ms�ް��j 10�b		*/
#define		KSG_Tm_AtCom_3s				-30								/* 3.0s ��ϒl				*/
#define		KSG_Tm_AtCom_2s				-20								/* 2.0s ��ϒl				*/
#define		KSG_Tm_AtCom_02s			-2								/* 0.2s ��ϒl				*/
#define		KSG_Tm_PPPcnct_COUNT		-600							/*  �i100ms�ް��j 60�b		*/
#define		KSG_Tm_TCPcnct_COUNT		-300							/*  �i100ms�ް��j 30�b		*/
#define		KSG_Tm_discnnect_COUNT		-300							/*  �i100ms�ް��j 30�b		*/
#define		KSG_RCV_MAX_SCI7			18								/* ��M�ő啶����			*/
																		/*							*/
#define		KSG_f_mdm_init_err			KSG_mdm_e_flag.BIT.B0			/* ���я������v���װ		*/
#define		KSG_f_mdm_kind_err			KSG_mdm_e_flag.BIT.B1			/* �����敪�ݒ�				*/
#define		KSG_f_mdm_conn_err			KSG_mdm_e_flag.BIT.B2			/* νĐڑ��װ				*/
#define		KSG_f_ReDial_rq				KSG_Tm_ReDial.BIT.f_rq			/* ��޲�يԊu��� �ض�		*/
#define		KSG_f_ReDial_ov				KSG_Tm_ReDial.BIT.f_ov			/* ��޲�يԊu��� ���ް�۰	*/
#define		KSG_f_Gurd_rq				KSG_Tm_Gurd.BIT.f_rq			/* �޲�ٶް����� �ض�		*/
#define		KSG_f_Gurd_ov				KSG_Tm_Gurd.BIT.f_ov			/* �޲�ٶް����� ���ް�۰	*/
#define		KSG_f_DialConn_rq			KSG_Tm_DialConn.BIT.f_rq		/* 							*/
#define		KSG_f_DialConn_ov			KSG_Tm_DialConn.BIT.f_ov		/* 							*/
#define		KSG_f_No_Action_rq			KSG_Tm_No_Action.BIT.f_rq		/* 							*/
#define		KSG_f_No_Action_ov			KSG_Tm_No_Action.BIT.f_ov		/* 							*/
#define		KSG_f_TCPcnct_rq			KSG_Tm_TCP_CNCT.BIT.f_rq		/* 							*/
#define		KSG_f_TCPcnct_ov			KSG_Tm_TCP_CNCT.BIT.f_ov		/* 							*/
#define		KSG_f_discnnect_rq			KSG_Tm_DISCONNECT.BIT.f_rq		/* �ȸ��ݐؒf����� �ض�		*/
#define		KSG_f_discnnect_ov			KSG_Tm_DISCONNECT.BIT.f_ov		/* �ȸ��ݐؒf����� ���ް�۰	*/
#define		KSG_f_data_rty_rq			KSG_Tm_DataRetry.BIT.f_rq		/* 							*/
#define		KSG_f_data_rty_ov			KSG_Tm_DataRetry.BIT.f_ov		/* 							*/
#define		KSG_f_TCPdiscnct_rq			KSG_Tm_TCP_DISCNCT.BIT.f_rq		/* 							*/
#define		KSG_f_TCPdiscnct_ov			KSG_Tm_TCP_DISCNCT.BIT.f_ov		/* 							*/
#define		KSG_f_ReDialTime_rq			KSG_Tm_ReDialTime.BIT.f_rq		/* ���ް���������  �ض�		*/
#define		KSG_f_ReDialTime_ov			KSG_Tm_ReDialTime.BIT.f_ov		/* ���ް���������  ���ް�۰	*/
#define		KSG_f_Ackwait_rq			KSG_Tm_Ackwait.BIT.f_rq			/* ظ���					*/
#define		KSG_f_Ackwait_ov			KSG_Tm_Ackwait.BIT.f_ov			/* ��ѵ��ް					*/
																		/*							*/
#define		KSG_MDM_DATA_MAX			128								/*							*/
#define		KSG_f_host_conn_req			KSG_mdm_flag.BIT.B0				/* νĐڑ��v��				*/
#define		KSG_Tm_FomaMdmDRwait		-50								// 5s ��� //
																		/*							*/
typedef volatile union {												/*  ���ԊǗ�				*/
	struct	{															/*  	Bit����				*/
		unsigned short		f_ov	: 1  ;								/*  	 bit 15				*/
		unsigned short		f_rq	: 1  ;								/*  	 bit 14				*/
		unsigned short		bit0 	: 14 ;								/*  	 bit 0-13			*/
	} BIT;																/*							*/
	unsigned short			tm;											/*   Byte����				*/
}	TIME_AREA;															/*							*/
																		/*							*/
typedef volatile union {												/*  ���ԊǗ�				*/
	struct	{															/*  	Bit����				*/
		unsigned long		f_ov	: 1  ;								/*  	 bit 31				*/
		unsigned long		f_rq	: 1  ;								/*  	 bit 30				*/
		unsigned long		bit0 	: 30 ;								/*  	 bit 0-29			*/
	} BIT;																/*							*/
	unsigned long			tm;											/*   Byte����				*/
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
			unsigned char 	ing	:1;										/* �޲�ْ�					*/
			unsigned char 	in	:1;										/* ���M						*/
			unsigned char 	out	:1;										/* ���M						*/
		} BIT;															/*							*/
		unsigned char		BYTE;										/*							*/
} DIAL_STATS;															/*							*/
																		/*							*/
extern  signed short		KSG_SCI0_CHR_TIM;							/* �����ԊĎ����			*/
extern	unsigned char		KSG_line_stat_h;							/* ν� ײݽð��				*/
extern	unsigned char		KSG_wait_CS;								/* 							*/
extern	unsigned char		KSG_RauModemOnline;							/* νĉ������׸�			*/
extern	unsigned char		KSG_mdm_cmd_len;							/* ���Ѻ���ޑ��M�ް���		*/
extern	unsigned char		KSG_mdm_cmd_ptr;							/* ���Ѻ���ޑ��M�ް��߲��	*/
extern	unsigned char		KSG_code_sts;								/* CR,LF ���o�ð��			*/
extern	unsigned char		KSG_mdm_r_buf[];							/* SIO0 ��Mү�����ޯ̧		*/
extern	unsigned char		*KSG_ptr_rx_save;							/* SIO0 ��M�ް��i�[�߲��	*/
extern	unsigned char		KSG_rx_save_len;							/* SIO0 ��M�ް��i�[��		*/
extern	unsigned char		KSG_mdm_cmd_buf[];							/* ���Ѻ�����ޯ̧			*/
extern	unsigned char		KSG_sent_h;									/* �߹�đ��M�I���׸�		*/
extern	unsigned char		KSG_DSR;									/* ��������DSR�l 		*/
extern	unsigned char		KSG_CD;										/* ��������CD�l  		*/
extern	unsigned char		KSG_CI;										/* ��������RI�l 			*/
extern	unsigned char		KSG_CTS;									/* ��������CTS�l 		*/
extern unsigned char		KSG_Chatt_DSR;								/* ������p ���ް��ۑ���	*/
extern unsigned char		KSG_Chatt_CTS;								/* ������p ���ް��ۑ���	*/
extern unsigned char		KSG_Chatt_CD;								/* ������p ���ް��ۑ���	*/
extern unsigned char		KSG_Chatt_CI;								/* ������p ���ް��ۑ���	*/
extern	unsigned char		KSG_TCP_conct_cnt;							/* TCP�ȸ��ݐڑ���ײ��	*/
extern	TIME_AREA			KSG_Tm_ReDial;								/* ��޲�����				*/
extern	TIME_AREA			KSG_Tm_Gurd;								/* �޲�ٶް�����			*/
extern	TIME_AREA			KSG_Tm_DialConn;							/* ����ڑ��҂����			*/
extern	TIME_AREA			KSG_Tm_AtCom;								/* AT��������				*/
extern	TIME_AREA_L			KSG_Tm_No_Action;							/* ���ʐM���(�b)			*/
extern	TIME_AREA			KSG_Tm_TCP_CNCT;							/* TCP�ȸ��ݐڑ��҂����		*/
extern	TIME_AREA			KSG_Tm_DISCONNECT;							/* �ȸ��ݐؒf�҂����		*/
extern	TIME_AREA			KSG_Tm_DataRetry;							/* �ް��đ��҂����(��) 		*/
extern	TIME_AREA			KSG_Tm_TCP_DISCNCT;							/* TCP�ȸ��ݐؒf�҂����		*/
extern	TIME_AREA			KSG_Tm_ReDialTime;							/* ��޲�ْ����				*/
extern	TIME_AREA			KSG_Tm_Ackwait;								/* ACK�҂����				*/
extern	unsigned short		KSG_Tm_Reset_t;								/* �@��ؾ�ďo�����			*/
extern	unsigned short		KSG_Tm_ResetPulse;							/* �@��ؾ�ăp���X�o�����	*/
extern	DIAL_STATS			KSG_dials;									/* �޲�ٽð��				*/
extern	unsigned short		KSG_net_rev_cmd;							/* USNET ��M�����			*/
extern	unsigned short		KSG_net_send_cmd;							/* USNET ���M�����			*/
extern	unsigned char		KSG_dial_go;								/* �޲�يJ�n(0:�ؒf,1:�J�n)	*/
extern	unsigned char		KSG_net_online;								/* ��ײ��׸�				*/
extern	unsigned char		KSG_mdm_mode;								/* ����Ӱ��(0:�����,1:�ް�)	*/
extern	unsigned char		KSG_mdm_init_sts;							/* ���я������ð��			*/
extern	unsigned char		KSG_mdm_ant_sts;							/* �������َ擾�ð��		*/
extern	unsigned short		KSG_Cnt_Retry;								/* �ڑ���ײ����				*/
extern	unsigned char		KSG_f_init_repeat;							/* ��������				*/
extern	BITS				KSG_mdm_Start;								/* Dopa start flag			*/
extern	BITS				KSG_mdm_ActFunc;							/* active function flag		*/
extern	BITS				KSG_mdm_e_flag;								/* ���Ѵװ�׸�				*/
extern	unsigned char		KSG_mdm_Condit;								/* task condition			*/
extern	unsigned char		KSG_txdata_mdm[KSG_SEND_MAX_MDM];			/* ���M�d���ޯ̧			*/
extern	unsigned int		KSG_txdlen_mdm;								/* ���M�ް���				*/
extern	unsigned char		KSG_txdch_mdm;								/* ���M����(UDP�̂�)		*/
extern	unsigned char		KSG_txdata_bak[KSG_SEND_MAX_MDM];			/* ���M�d���ޯ̧			*/
extern	unsigned int		KSG_txdlen_bak;								/* ���M�ް���				*/
extern	unsigned char		KSG_txdch_bak;								/* ���M����(UDP�̂�)		*/
extern	unsigned char		KSG_rxdata_mdm[KSG_RCV_MAX_MDM];			/* ��M�d���ޯ̧			*/
extern	unsigned int		KSG_rxdlen_mdm;								/* ��M�ް���				*/
extern	unsigned char		KSG_mdm_f_TmStart;							/* 							*/
extern	unsigned char		KSG_connect_retry_flg;						/* PPP�Đڑ��v��			*/
extern	unsigned char		KSG_disconect_flg;							/* TCP�ȸ��ݐؒf�׸�		*/
extern	  signed char		KSG_vccs_authsel;							/* PPP�F�ؕ��@				*/
extern	unsigned char		KSG_mdm_status;								/* ���ѽð��				*/
extern	BITS				KSG_mdm_flag;								/* �����׸�					*/
extern	unsigned char		KSG_modem_req;								/* ���ѐڑ�ظ����׸�		*/
extern	unsigned char		KSG_mdm_cut_req;							/* ���ѐؒfظ����׸�		*/
extern	unsigned char		KSG_mdm_TCP_CNCT;							/* TCP�ȸ��ݏ���׸�		*/
extern	unsigned char		KSG_mdm_TCPcnct_req;						/* TCP�ȸ���ظ����׸�		*/
extern	unsigned char		KSG_tcp_inout;								/* TCP���M���M�N�����		*/
extern	unsigned char		KSG_tcp_in_flg;								/* TCP���M�ڑ����ؒf�׸�	*/
extern	unsigned char		KSG_ppp_info;								/* PPP�̏��				*/
extern	t_MODEM_Ctrl		KSG_mdm_Snd_Ctrl;							/*							*/
extern	t_MODEM_Ctrl		KSG_mdm_Rcv_Ctrl;							/*							*/
extern  TIME_AREA_L			KSG_Tm_DopaReset;							/*  HOST����������DOPAؾ��	*/

extern TIME_AREA		KSG_Tm_ERDR_TIME;								// ER->DR HIGH �Ď���ϰ
extern unsigned char	KSG_uc_FomaFlag;								// 0=Dopa / 1=Foma
extern unsigned char	KSG_uc_AdapterType;								// �T�[�r�X���g�p����@��^�C�v���Z�b�g
extern unsigned char	KSG_uc_AtRetryCnt;								// 3 times retury counter
extern unsigned char	KSG_uc_AtResultWait;							// 0:-- 1:Waiting for Result Code
extern unsigned char	KSG_uc_ConnAftWait;								// CONNECT ( 2s ) -> PPP
extern unsigned char	KSG_uc_Foma_APN[32];							// Access Point Name
extern unsigned char	KSG_uc_FomaWaitDRFlag;							// 100msec. ER ON ��
extern unsigned char	KSG_uc_FomaPWRphase;							// �d���Ǘ��t�F�[�Y 0=IDLE, 1=CS off �Ď�, 2=MODEM OFF�v����
extern unsigned char	KSG_uc_MdmInitRetry;							// ���f���d�����䃊�g���C �d��ON���s�����g���C�J�E���^

#define		KSG_f_ERDRtime_ov 			KSG_Tm_ERDR_TIME.BIT.f_ov
#define		KSG_f_ERDRtime_rq 			KSG_Tm_ERDR_TIME.BIT.f_rq

#define		KSG_Tm_FomaResetToGurd		-100			// MODEM �d��ON ���� Dial �\��Ԃ܂ł̳���

#define		KSG_FomaModemPwrPhase1		600				// Foma Modem �d��Off �� CD �Ď�����(60�b)
#define		KSG_FomaModemPwrPhase2		200				// Foma Modem �d�� PWR HIGH�̎���(20�b)

#define		KSG_FomaModemPwrPhase3		50				// Foma Modem �d�� ON �܂ł̲������ 5�b[GuidLine] //

#endif		//_KSG_RAU_MODEMDATA_H
