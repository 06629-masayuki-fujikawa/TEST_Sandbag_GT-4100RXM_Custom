/************************************************************************************************/
/*																								*/
/*	Ӽޭ�ٖ���	:ksgRauModemData.c	:---------------------------------------------------------: */
/*	Ӽޭ�ټ����	:					: ���� �ް��֘A											  : */
/*																								*/
/************************************************************************************************/
#include	"ksgRauModem.h"												/* ���ъ֘A�֐��S			*/
#include	"ksgRauModemData.h"											/* ���ъ֘A�ް���`			*/
#include	"ksgmac.h"													/* MAC ���ڋ@�\ͯ��			*/
																		/*							*/
TIME_AREA		KSG_Tm_ReDial;											/* ��޲�����				*/
TIME_AREA		KSG_Tm_Gurd;											/* �޲�ٶް�����			*/
TIME_AREA		KSG_Tm_DialConn;										/* ����ڑ��҂����			*/
TIME_AREA		KSG_Tm_AtCom;											/* AT��������				*/
TIME_AREA_L		KSG_Tm_No_Action;										/* ���ʐM���				*/
TIME_AREA		KSG_Tm_TCP_CNCT;										/* TCP�ȸ��ݐڑ��҂����		*/
TIME_AREA		KSG_Tm_DISCONNECT;										/* �ȸ��ݐؒf�҂����		*/
TIME_AREA		KSG_Tm_DataRetry;										/* �ް��đ��҂����(��)		*/
TIME_AREA		KSG_Tm_TCP_DISCNCT;										/* TCP�ȸ��ݐؒf�҂����		*/
TIME_AREA		KSG_Tm_ReDialTime;										/* ��޲�ْ����				*/
TIME_AREA		KSG_Tm_Ackwait;											/* ACK�҂����				*/
unsigned short	KSG_Tm_Reset_t;											/* �@��ؾ�ďo�����			*/
unsigned short	KSG_Tm_ResetPulse;										/* �@��ؾ�ăp���X�o�����	*/
unsigned char	KSG_line_stat_h;										/* ν� ײݽð��				*/
unsigned char	KSG_wait_CS;											/*							*/
unsigned char	KSG_RauModemOnline;										/* νĉ������׸�			*/
unsigned char	KSG_mdm_cmd_len;										/* ���Ѻ���ޑ��M�ް���		*/
unsigned char	KSG_mdm_cmd_ptr;										/* ���Ѻ���ޑ��M�ް��߲��	*/
unsigned char	KSG_code_sts;											/* CR,LF ���o�ð��			*/
unsigned char	KSG_mdm_r_buf[KSG_RCV_MAX_SCI7];						/* SIO0 ��Mү�����ޯ̧		*/
unsigned char	*KSG_ptr_rx_save;										/* SIO0 ��M�ް��i�[�߲��	*/
unsigned char	KSG_rx_save_len;										/* SIO0 ��M�ް��i�[��		*/
unsigned char	KSG_mdm_cmd_buf[128];									/* ���Ѻ�����ޯ̧			*/
unsigned char	KSG_sent_h;												/* �߹�đ��M�I���׸�		*/
unsigned char	KSG_DSR;												/* ��������DSR�l 		*/
unsigned char	KSG_CD;													/* ��������CD�l  		*/
unsigned char	KSG_CI;													/* ��������RI�l 			*/
unsigned char	KSG_CTS;												/* ��������CTS�l 		*/
unsigned char	KSG_Chatt_DSR;											/* ������p ���ް��ۑ���	*/
unsigned char	KSG_Chatt_CTS;											/* ������p ���ް��ۑ���	*/
unsigned char	KSG_Chatt_CD;											/* ������p ���ް��ۑ���	*/
unsigned char	KSG_Chatt_CI;											/* ������p ���ް��ۑ���	*/
																		/*							*/
DIAL_STATS		KSG_dials;												/* �޲�ٽð��				*/
unsigned short	KSG_net_rev_cmd;										/* USNET ��M�����			*/
unsigned short	KSG_net_send_cmd;										/* USNET ���M�����			*/
unsigned char	KSG_dial_go;											/* �޲�يJ�n				*/
unsigned char	KSG_net_online;											/* ��ײ��׸�				*/
unsigned char	KSG_modem_req;											/* ���ѐڑ�ظ����׸�		*/
unsigned char	KSG_mdm_mode;											/* ����Ӱ��(0:�����,1:�ް�)	*/
unsigned char	KSG_mdm_status;											/* ���ѽð��				*/
unsigned char	KSG_mdm_init_sts;										/* ���я������ð��			*/
unsigned char	KSG_mdm_ant_sts;										/* �������َ擾�ð��		*/
unsigned short	KSG_Cnt_Retry;											/* �ڑ���ײ����				*/
unsigned char	KSG_f_init_repeat;										/* ��������				*/
																		/*							*/
BITS			KSG_mdm_Start;											/* start flag				*/
BITS			KSG_mdm_ActFunc;										/* active function flag		*/
BITS			KSG_mdm_flag;											/* �����׸�					*/
BITS			KSG_mdm_e_flag;											/* ���Ѵװ�׸�				*/
																		/*							*/
t_MODEM_Ctrl	KSG_mdm_Snd_Ctrl;										/*							*/
t_MODEM_Ctrl	KSG_mdm_Rcv_Ctrl;										/*							*/
unsigned char	KSG_mdm_Condit;											/* task condition			*/
unsigned char	KSG_txdata_mdm[KSG_SEND_MAX_MDM];						/* ���M�d���ޯ̧			*/
unsigned int	KSG_txdlen_mdm;											/* ���M�ް���				*/
unsigned char	KSG_txdch_mdm;											/* ���M����(UDP�̂�)		*/
unsigned char	KSG_txdata_bak[KSG_SEND_MAX_MDM];						/* ���M�d���ޯ̧			*/
unsigned int	KSG_txdlen_bak;											/* ���M�ް���				*/
unsigned char	KSG_txdch_bak;											/* ���M����(UDP�̂�)		*/
unsigned char	KSG_rxdata_mdm[KSG_RCV_MAX_MDM];						/* ��M�d���ޯ̧			*/
unsigned int	KSG_rxdlen_mdm;											/* ��M�ް���				*/
unsigned char	KSG_mdm_f_TmStart;										/* 							*/
unsigned char	KSG_connect_retry_flg;									/* PPP�Đڑ��v��			*/
unsigned char	KSG_mdm_cut_req;										/* modem�ؒfظ����׸�		*/
unsigned char	KSG_mdm_TCP_CNCT;										/* TCP�ȸ��ݏ���׸�		*/
unsigned char	KSG_disconect_flg;										/* TCP�ȸ��ݐؒf�׸�		*/
unsigned char	KSG_mdm_TCPcnct_req;									/* TCP�ȸ���ظ����׸�		*/
unsigned char	KSG_TCP_conct_cnt;										/* TCP�ȸ��ݐڑ���ײ��	*/
  signed char	KSG_vccs_authsel;										/* PPP�F�ؕ��@				*/
unsigned char	KSG_tcp_inout;											/* TCP���M���M�N�����		*/
unsigned char	KSG_tcp_in_flg;											/* TCP���M�ڑ����ؒf�׸�	*/
unsigned char	KSG_ppp_info;											/* PPP�̏��				*/
TIME_AREA_L		KSG_Tm_DopaReset;										/*  HOST����������DOPAؾ��	*/
TIME_AREA		KSG_Tm_ERDR_TIME;										// ER->DR HIGH �Ď���ϰ
unsigned char	KSG_uc_FomaFlag;										// 0=Dopa / 1=Foma
																		// ���ʐM�T�[�r�X�̃^�C�v���Z�b�g
unsigned char	KSG_uc_AdapterType;										// �T�[�r�X���g�p����@��^�C�v���Z�b�g
unsigned char	KSG_uc_AtRetryCnt;										// 3 times retury counter
unsigned char	KSG_uc_AtResultWait;									// 0:-- 1:Waiting for Result Code
unsigned char	KSG_uc_ConnAftWait;										// CONNECT ( 2s ) -> PPP
unsigned char	KSG_uc_Foma_APN[32];									// Access Point Name
unsigned char	KSG_uc_FomaWaitDRFlag;									// 100msec. ER ON ��
unsigned char	KSG_uc_FomaPWRphase;									// �d���Ǘ��t�F�[�Y 0=IDLE, 1=CS off �Ď�, 2=MODEM OFF�v����
unsigned char	KSG_uc_MdmInitRetry;									// ���f���d�����䃊�g���C �d��ON���s�����g���C�J�E���^

