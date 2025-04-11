/*[]----------------------------------------------------------------------[]*/
/*|		RAU task                    									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  S.Takahashi                                             |*/
/*| Date        :  2012-09-05                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/

#include	<string.h>
#include    <trsocket.h>
#include    <trmacro.h>
#include    <trtype.h>
#include    <trproto.h>
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ope_def.h"
#include	"rau.h"
#include	"raudef.h"
#include	"rauIDproc.h"

/*----------------------------------*/
/*		function external define	*/
/*----------------------------------*/
extern	void	RAU_FuncStop( void );
extern	void	RAU_FuncMain( void );
extern	void	RAU_IniDatSave( void );
extern	void	RAU_IniDatUpdate( void );
extern	void	RAU_FuncStart( void );
extern	void	Credit_main(void);
/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	RAU_init( void );
void	RAU_ConfigInit( void );
void	RAU_main( void );
void	Credit_Init(void);

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_Get_Tm_Reset_t()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|     return value of "RAU_Tm_Reset_t"                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-12-20                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	RAU_Get_Tm_Reset_t(void)
{
	return RAU_Tm_Reset_t;
}

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_Set_Tm_Reset_t()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|     set "RAU_Tm_Reset_t"                                                   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAU_Set_Tm_Reset_t(ushort val)
{
	RAU_Tm_Reset_t = val;
}

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_init()  	                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|          RAU task initial routine                                      |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	RAU_init( void )
{
	short	Lno;								// ���OID
	short	target;								// ���O�^�[�Q�b�g
	RAU_ConfigInit();
	
	/* area initialize */

	/* DataTable.c */
	memset(&RAU_table_data, 0, sizeof(RAU_table_data));
	memset(&rau_data_table_info , 0, sizeof(rau_data_table_info));
	rau_table_crear_flag = 0;
	memset(&RAU_table_top, 0, sizeof(RAU_table_top));
	/* Host.c */
	RAU_ucSendSeqFlag = 0;						// ���M�V�[�P���X�t���O
	RAU_ucReceiveSeqFlag = 0;					// ��M�V�[�P���X�t���O
	RAU_ucTcpBlockSts = 0;						// TCP�u���b�N�X�e�[�^�X
	RAU_ucTcpBlockSts2 = 0;						// ���������M�f�[�^�p�s�b�o�u���b�N�X�e�[�^�X
	RAU_uiTcpBlockNo = 0;						// TCP�u���b�N�i���o�[
	RAU_uiTcpBlockNo2 = 0;						// ���������M�f�[�^�p�s�b�o�u���b�N�ԍ�
	memset(&RAUhost_SendRequest, 0, sizeof(RAUhost_SendRequest));					// ���M�v���f�[�^
	RAUhost_SendRequestFlag = 0;				// ���M�v���f�[�^����t���O
	RAUhost_Error01 = 0;						// �G���[�R�[�h01������
	memset(&RAUhost_SendNtDataInfo, 0, sizeof(RAUhost_SendNtDataInfo));				// ���M�Ώۂ̃e�[�u���o�b�t�@����擾����NT�f�[�^�̏��
	pRAUhost_SendIdInfo = NULL;					// ���M�Ώۂ̃e�[�u���o�b�t�@���̎Q��
	memset(&RAUhost_SendNtThroughInfo, 0, sizeof(RAUhost_SendNtThroughInfo));		// ���������瑗�M����X���[�f�[�^�e�[�u���̂m�s�f�[�^���
	pRAUhost_SendThroughInfo = NULL;				// ���������瑗�M����X���[�f�[�^�e�[�u���̏��
	RAU_f_RcvSeqCnt_rq = 0;						// ��M�V�[�P���X�^�C�}�[�t���O
	RAU_Tm_RcvSeqCnt = 0;						// ��M�V�[�P���X�t���O�L�����Z���^�C�}�[
	RAU_f_SndSeqCnt_rq = 0;						// ��M�V�[�P���X�^�C�}�[�t���O
	RAU_Tm_SndSeqCnt = 0;						// ��M�V�[�P���X�t���O�L�����Z���^�C�}�[
	/* IDproc.c */
	memset(&RAU_special_table_send_buffer, 0, sizeof(RAU_special_table_send_buffer));		// �s���v����K�Ǘ��f�[�^�̉��u�o�b���M�p����o�b�t�@
	memset(&RemoteCheck_Buffer100_101, 0, sizeof(RemoteCheck_Buffer100_101));				// �ʐM�`�F�b�N�p�o�b�t�@(�h�c�P�O�O�E�P�O�P)
	memset(&RemoteCheck_Buffer116_117, 0, sizeof(RemoteCheck_Buffer116_117));				// �ʐM�`�F�b�N�p�o�b�t�@(�h�c�P�P�U�E�P�P�V)
	RauCT_SndReqID = 0;							// �ʐM�`�F�b�N�f�[�^���M�v���h�c(0:�v���Ȃ�, 100:�h�c�P�O�O���M�v��, 117:�h�c�P�P�V���M�v��)
	Rau_SedEnable = 0;							// ���M����
	RAU_ucLastNtBlockNum = 0;					// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
	RAU_uiArcReceiveLen = 0;					// ��M�����m�s�u���b�N���̍��v
	RAU_uiLastTcpBlockNum = 0;					// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
	RAU_uiDopaReceiveLen = 0;					// ��M�����m�s�u���b�N���̍��v
	/* Que.c */
	memset(&RAUque_SendDataInfo, 0, sizeof(RAUque_SendDataInfo));				// ���M���ׂ��e�e�[�u���̏��(0:���Ƀe�[�u��, 1:�o�Ƀe�[�u��, ��� 14:�X���[�f�[�^�e�[�u��)
	RAUque_CurrentSendData = 0;
	/* RAUinRam.c */
	RAU_Tm_Reset_t = 0;							// �@�탊�Z�b�g�o�̓^�C�}
	/* RAUmain.c */
	RAU_f_TmStart = 0;							// 1=Start, 0=don't do process
	/* Terminal.c */
	memset(&RAUarc_SndNtBlk, 0, sizeof(RAUarc_SndNtBlk));
	/* ram.h */
	RAU_uc_mode_h = 0;							//	�iHOST�ʐM���j�ʐM��ԃ��[�h
	RAU_uc_mode_h2 = 0;							// �������̒ʐM��ԃ��[�h(S_H_IDLE:�A�C�h��, S_H_ACK:�`�b�j�҂�)
	RAU_uc_retrycnt_h = 0;						//	�iHOST�ʐM���j���M���g���C�J�E���^
	RAU_uc_retrycnt_h2 = 0;						// �������p�̑��M���g���C�J�E���^
	memset(&RAU_uc_txdata_h_keep, 0, sizeof(RAU_uc_txdata_h_keep));				//	�iHOST�ʐM���j�O�񑗐M�����f�[�^��ۑ�
	memset(&RAU_uc_txdata_h_keep2, 0, sizeof(RAU_uc_txdata_h_keep2));				// ���������瑗�M�����f�[�^�̂c���o���w�b�_������ێ�����o�b�t�@
	RAU_uc_rcvretry_cnt = 0;					// ����������̓���f�[�^��M�񐔃J�E���^
	RAU_ui_txdlength_t = 0;						//	�i�[���ʐM���j���M�f�[�^���i���C���p�j
	RAU_ui_data_length_h = 0;					//	�iHOST�ʐM���j��M�o�C�g���J�E���^
	RAU_ui_txdlength_h = 0;						//	�iHOST�ʐM���j���M�f�[�^���i���C���p�j
	RAU_ui_txdlength_h2 = 0;					// �������֑��M����f�[�^�̒���
	RAU_ui_txdlength_h_keep = 0;
	RAU_ui_txdlength_h_keep2 = 0;
	RAU_ui_seq_bak = 0;							// �V�[�P���V�������o�b�N�A�b�v
	memset(&RAU_huc_rcv_work_t, 0, sizeof(RAU_huc_rcv_work_t));		//	�i�[���ʐM���j��M�e�L�X�g���[�N�o�b�t�@
	memset(&RAU_huc_txdata_h, 0, sizeof(RAU_huc_txdata_h));			//	�iHOST�ʐM���j���M�d���o�b�t�@
	memset(&RAU_huc_txdata_h2, 0, sizeof(RAU_huc_txdata_h2));		// �������̑��M�d���o�b�t�@
	memset(&RAU_huc_rcv_work_h, 0, sizeof(RAU_huc_rcv_work_h));		//	�iHOST�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
	memset(&RAU_Tm_CommuTest, 0, sizeof(RAU_Tm_CommuTest));			// �ʐM�`�F�b�N�p�^�C�}
	memset(&RAU_Tm_Ackwait, 0, sizeof(RAU_Tm_Ackwait));				// ���������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
	memset(&RAU_Tm_Ackwait2, 0, sizeof(RAU_Tm_Ackwait2));			// ����������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
	RAU_uc_retrycnt_reconnect_h = 0;			//	�iHOST�ʐM���j�Đڑ����g���C�J�E���^
	memset(&RAU_Tm_Ackwait_reconnect, 0, sizeof(RAU_Tm_Ackwait_reconnect));				// ���������f�[�^���M����Ƃ��̍Đڑ��҂��^�C�}
	/* IDProc.h */
	RAU_ui_RAUDPA_seq = 0;						// ���uDopa�p���M���V�[�P���V�����ԍ�
	RAU_ui_RAUDPA_seq2 = 0;						// ���������M�f�[�^�p�c���o���p�P�b�g�V�[�P���V�����ԍ�

	memset(&RAU_tsum_man, 0, sizeof(RAU_tsum_man));

	RAU_ui_RAUDPA_seq = 0;					// ���uDopa�p�e�[�u���f�[�^���M���V�[�P���V�����ԍ��N���A
	RAU_ui_RAUDPA_seq2 = 0;					// ���uDopa�p�X���[�f�[�^���M���V�[�P���V�����ԍ��N���A
	RAU_ui_seq_bak = 0;						// �V�[�P���V�������o�b�N�A�b�v
	
	memset(&RauCtrl, 0, sizeof(RauCtrl));	// ����e�[�u���̃N���A
	memset(&CreditCtrl, 0, sizeof(CreditCtrl));	// ����e�[�u���̃N���A

	RAUarc_SndNtBlk.arcblknum		= RAUARC_NODATA;
	RAUarc_SndNtBlk.ntblknum		= RAUARC_NODATA;
	RAUarc_SndNtBlk.retry = 0;
	RAUarc_SndNtBlk.ntblkno_retry = 0;
//--	RAUarc_ES_OiBan = 0xFF;
	
	memset(&RauOpeSendNtDataBuf, 0, sizeof(RauOpeSendNtDataBuf));	// RAU�^�X�N���M�o�b�t�@�i��OPE�w�j
	memset(&RauOpeRecvNtDataBuf, 0, sizeof(RauOpeRecvNtDataBuf));	// RAU�^�X�N��M�o�b�t�@�i��OPE�w�j

	RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// ���M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)
	RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X�O(�ʏ���)
	RAUhost_ClearSendRequeat();
	
	memset(&DPA2_Snd_Ctrl, 0,sizeof(DPA2_Snd_Ctrl));	// �������̑��M�L���[
	memset(&DPA2_Rcv_Ctrl, 0, sizeof(DPA2_Rcv_Ctrl));	// ������̎�M�L���[
	memset(&DPA_Snd_Ctrl, 0, sizeof(DPA_Snd_Ctrl));		// ������̑��M�L���[
	memset(&DPA_Rcv_Ctrl, 0, sizeof(DPA_Rcv_Ctrl));		// �������̎�M�L���[
	
	// �^�C�}�֘A�̏�����
	RAU_Tm_No_Action.tm = 0;
	RAU_Tm_Port_Watchdog.tm = 0;
	RAU_Tm_DISCONNECT.tm = 0;
	RAU_Tm_TCP_TIME_WAIT.tm = 0;
	RAU_f_TCPtime_wait_ov = 1;
	RAU_Tm_data_rty.tm = 0;
	RAU_f_data_rty_ov = 1;
	RAU_Tm_TCP_DISCNCT.tm = 0;

	RAU_data_table_init();

	init_tsum((char*)&RAU_table_data.total, RAU_TOTAL_SIZE);
	init_center_term_info((char*)&RAU_table_data.center_term_info);

	RAU_f_TmStart = 1;						/* timer process start */
	// ���O�̃j�A�t���������l��ݒ�
	// ToDo: �b���RAU�^�X�N�ōs��
	for (Lno = 0; Lno < eLOG_MAX; ++Lno) {
		for (target = 0; target < eLOG_TARGET_MAX; ++target) {
			LOG_DAT[Lno].nearFull[target].NearFullMaximum = Ope_Log_GetNearFullCount(Lno);
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_ConfigInit
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ʃp�����[�^�֘A����ݒ������������
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_ConfigInit( void )
{
	short	s_work;
	ushort	us_work;
	char	ip[20];								// IP�A�h���X�ϊ��p
	
	memset(&RauConfig, 0, sizeof(RauConfig));
	
	RauConfig.id22_23_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 5);	/* �h�c�������M�t���O  �i���Z�f�[�^�j*/
	RauConfig.id30_41_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 4);	/* �h�c�������M�t���O  �i�W�v�f�[�^�j*/
	RauConfig.id121_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 2);	/* �h�c�������M�t���O  �i�A���[���f�[�^�j*/
	RauConfig.id123_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 6);	/* �h�c�������M�t���O  �i���샂�j�^�[�f�[�^�j*/
	RauConfig.id120_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 3);	/* �h�c�������M�t���O  �i�G���[�f�[�^�j*/
	RauConfig.id20_21_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 6);	/* �h�c�������M�t���O  �i���ɏo�Ƀf�[�^�j*/
	RauConfig.id122_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 1);	/* �h�c�������M�t���O  �i���j�^�[�f�[�^�j*/
	RauConfig.id131_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 5);	/* �h�c�������M�t���O  �i�R�C�����ɏW�v�f�[�^�j*/
	RauConfig.id133_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 4);	/* �h�c�������M�t���O  �i�������ɏW�v�f�[�^�j*/
	RauConfig.id236_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 3);	/* �h�c�������M�t���O  �i���ԑ䐔�f�[�^�j*/
	RauConfig.id237_tel_flg 	= 0;									/* �h�c�������M�t���O  �i���䐔�E���ԃf�[�^�j*/
	RauConfig.id126_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 1);	/* �h�c�������M�t���O  �i���K�Ǘ��f�[�^�j*/
	RauConfig.id135_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 2);	/* �h�c�������M�t���O  �i���K�Ǘ��W�v�f�[�^�j*/
	// @todo �ݒ肪�Ȃ����߁A1=�������M���Z�b�g
	RauConfig.id125_tel_flg 	= 1;									/* �h�c�������M�t���O  �i���u�Ď��f�[�^�j*/
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	// �������M�Œ�
	RauConfig.id61_tel_flg 	= 1;										/* �h�c�������M�t���O  �i�������ԏ��f�[�^�j*/
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)

	RauConfig.id22_23_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 5);	/* ���M�}�X�N�ݒ�  �i���Z�f�[�^�j*/
	RauConfig.id30_41_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 4);	/* ���M�}�X�N�ݒ�  �i�W�v�f�[�^�j*/
	RauConfig.id121_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 2);	/* ���M�}�X�N�ݒ�  �i�A���[���f�[�^�j*/
	RauConfig.id123_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 6);	/* ���M�}�X�N�ݒ�  �i���샂�j�^�[�f�[�^�j*/
	RauConfig.id120_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 3);	/* ���M�}�X�N�ݒ�  �i�G���[�f�[�^�j*/
	RauConfig.id20_21_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 6);	/* ���M�}�X�N�ݒ�  �i���ɏo�Ƀf�[�^�j*/
	RauConfig.id122_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 1);	/* ���M�}�X�N�ݒ�  �i���j�^�[�f�[�^�j*/
	RauConfig.id131_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 5);	/* ���M�}�X�N�ݒ�  �i�R�C�����ɏW�v�f�[�^�j*/
	RauConfig.id133_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 4);	/* ���M�}�X�N�ݒ�  �i�������ɏW�v�f�[�^�j*/
	RauConfig.id236_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 3);	/* ���M�}�X�N�ݒ�  �i���ԑ䐔�f�[�^�j*/
	RauConfig.id237_mask_flg 	= 0;									/* ���M�}�X�N�ݒ�  �i���䐔�E���ԃf�[�^�j*/
	RauConfig.id126_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 1);	/* ���M�}�X�N�ݒ�  �i���K�Ǘ��f�[�^�j*/
	RauConfig.id135_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 2);	/* ���M�}�X�N�ݒ�  �i�ޑK�Ǘ��W�v�f�[�^�j*/
	// @todo �ݒ肪�Ȃ����߁A0=���M������Z�b�g
	RauConfig.id125_mask_flg 	= 0;									/* ���M�}�X�N�ݒ�  �i���u�Ď��f�[�^�j*/
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	// ���M����Œ�
	RauConfig.id61_mask_flg 	= 0;									/* ���M�}�X�N�ݒ�  �i�������ԏ��f�[�^�j*/
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)

	RauConfig.alarm_send_level		= prm_get(COM_PRM, S_NTN, 37, 1, 4);
	RauConfig.error_send_level		= prm_get(COM_PRM, S_NTN, 37, 1, 3);
	RauConfig.opemonitor_send_level	= prm_get(COM_PRM, S_NTN, 37, 1, 2);
	RauConfig.monitor_send_level	= prm_get(COM_PRM, S_NTN, 37, 1, 1);
	// @todo 0=���x��0�ȏ�̃f�[�^�𑗐M�ɃZ�b�g
	RauConfig.rmon_send_level		= 0;
	RauConfig.serverTypeSendTsum	= prm_get(COM_PRM, S_NTN, 26, 1, 3);	// �T�[�o�[�^�C�v���̑��M 0:���Ȃ� 1:����

	RauConfig.deta_full_proc 	= prm_get(COM_PRM, S_CEN, 51, 1, 1);	/* �f�[�^�������@ */
	RauConfig.IBW_RespWtime = 20;										/* ���M���ʎ�M�����҂����ԁi�P�ʁ��b�j*/
	RauConfig.system_select = 0;										/* �V�X�e���I�� */
	RauConfig.modem_exist = (unsigned char)prm_get(COM_PRM, S_CEN, 51, 1, 3);
	if(prm_get(COM_PRM, S_PAY, 24, 1, 1) != 2){							// Rau�g�p�����擾
		if(RAU_Credit_Enabale == 2){
			RauConfig.modem_exist = 0;
		}
		else{
			RauConfig.modem_exist = 1;
		}
	}
	RauConfig.Dpa_proc_knd	 	= prm_get(COM_PRM, S_CEN, 51, 1, 2);	/* Dopa �����敪 [0]=�펞�ʐM�Ȃ�, [1]=�펞�ʐM���� */
	
	sprintf(ip, "%03d.%03d.%03d.%03d",
			prm_get(COM_PRM, S_CEN, 62, 3, 4),
			prm_get(COM_PRM, S_CEN, 62, 3, 1),
			prm_get(COM_PRM, S_CEN, 63, 3, 4),
			prm_get(COM_PRM, S_CEN, 63, 3, 1));
	RauConfig.Dpa_IP_h			= inet_addr(ip);						/* Dopa HOST��IP�A�h���X */

	RauConfig.Dpa_port_h		= prm_get(COM_PRM, S_CEN, 64, 5, 1);	/* Dopa HOST�ǃ|�[�g�ԍ� */

	if(RauConfig.modem_exist == 0) {									// ���f��
		sprintf(ip, "%03d.%03d.%03d.%03d",
				prm_get(COM_PRM, S_CEN, 65, 3, 4),
				prm_get(COM_PRM, S_CEN, 65, 3, 1),
				prm_get(COM_PRM, S_CEN, 66, 3, 4),
				prm_get(COM_PRM, S_CEN, 66, 3, 1));
	}
	else {																// Ethernet
		sprintf(ip, "%03d.%03d.%03d.%03d",
				prm_get(COM_PRM, S_MDL, 2, 3, 4),
				prm_get(COM_PRM, S_MDL, 2, 3, 1),
				prm_get(COM_PRM, S_MDL, 3, 3, 4),
				prm_get(COM_PRM, S_MDL, 3, 3, 1));
	}
	RauConfig.Dpa_IP_m 			= inet_addr(ip);						/* Dopa ����IP�A�h���X */
	
	RauConfig.Dpa_port_m		= prm_get(COM_PRM, S_CEN, 67, 5, 1);	/* Dopa ���ǃ|�[�g�ԍ� */
	RauConfig.Dpa_nosnd_tm		= 0 - (short)prm_get(COM_PRM, S_CEN, 54, 4, 1);	/* Dopa ���ʐM�^�C�}�[(�b) */
	RauConfig.Dpa_cnct_rty_tm	= prm_get(COM_PRM, S_CEN, 53, 3, 4);	/* Dopa �Ĕ��đ҂��^�C�}�[(�b) */
	RauConfig.Dpa_com_wait_tm	= 0 - (short)prm_get(COM_PRM, S_CEN, 68, 3, 1);	/* Dopa �R�}���h(ACK)�҂��^�C�}�[(�b) */
	us_work	= prm_get(COM_PRM, S_CEN, 75, 3, 1);						/* Dopa �f�[�^�đ��҂��^�C�}�[(��) */
	if(us_work <= 240) {
		RauConfig.Dpa_data_rty_tm	= 0 - us_work;
	}
	else {
		RauConfig.Dpa_data_rty_tm	= (ushort)(0 - 240);
	}
	
	// TCP�R�l�N�V�����ؒf�҂�(TIME_WAIT)�^�C�}�[(�b)
	s_work = (short)prm_get(COM_PRM, S_CEN, 69, 4, 1);
	if(RauConfig.modem_exist == 0) {									// ���f���̏ꍇ��10�b�ȏ�Ƃ���
		if(s_work < 10) {
			s_work = 10;
		}
	}
	RauConfig.Dpa_discnct_tm	= 0 - (s_work * 10);					// 100ms�P�ʂɕϊ�
	
	RauConfig.Dpa_cnct_rty_cn	= prm_get(COM_PRM, S_CEN, 53, 3, 1);	/* Dopa �Ĕ��ĉ� */
	RauConfig.Dpa_data_rty_cn	= prm_get(COM_PRM, S_CEN, 70, 1, 2);	/* Dopa �f�[�^�đ���(NAK,������) */
	RauConfig.Dpa_data_code		= 0;									/* Dopa HOST���d���R�[�h [0]=�����R�[�h, [1]=�o�C�i���R�[�h */
	RauConfig.Dpa_com_wait_reconnect_tm	= (ushort)( 0 - RAU_RECONNECT_INTERVAL );	/* Dopa �Đڑ��҂��^�C�}�[(�b) */
	RauConfig.Dpa_data_rty_cn_disconnected	= RAU_RECONNECT_MAX;		/* Dopa �Đڑ���(TCP�ؒf) */

	RauConfig.interrupt_proc = 0;										/* ���f�v�����̏��� */
	RauConfig.interrupt_proc_block = 0;									/* ���f�v����M���̔���u���b�N */
	RauConfig.Dpa_data_snd_rty_cnt = prm_get(COM_PRM, S_CEN, 73, 1, 2);	// �c���o���������p�P�b�g���M���g���C��(������)
	RauConfig.Dpa_ack_wait_tm	= 0 - (short)prm_get(COM_PRM, S_CEN, 72, 3, 1);	// �c���o���������p�P�b�g�����҂�����(�`�b�j/�m�`�j)
	RauConfig.Dpa_data_rcv_rty_cnt = prm_get(COM_PRM, S_CEN, 73, 1, 1);	// �c���o���������p�P�b�g��M���g���C��(�m�`�j)
	RauConfig.Dpa_port_watchdog_tm = 0 - (short)prm_get(COM_PRM, S_CEN, 72, 3, 4);	// �c���o���������ʐM�Ď��^�C�}(�b)
	RauConfig.Dpa_port_m2		= prm_get(COM_PRM, S_CEN, 71, 5, 1);	// �c���o�������� ���ǃ|�[�g�ԍ�
	if(2 == prm_get( COM_PRM,S_CEN,78,1,1 )) {							// Dopa/FOMA�ؑ�
		RauConfig.tcp_disconect_flag = 1;								// TCP�ؒf�Ȃ�
	}
	else {
		RauConfig.tcp_disconect_flag = 0;								// TCP�ؒf����
	}
	sprintf(ip, "%03d.%03d.%03d.%03d",
			prm_get(COM_PRM, S_CRE, 21, 3, 4),
			prm_get(COM_PRM, S_CRE, 21, 3, 1),
			prm_get(COM_PRM, S_CRE, 22, 3, 4),
			prm_get(COM_PRM, S_CRE, 22, 3, 1));
	RauConfig.Dpa_IP_Cre = inet_addr(ip);								/* �N���W�b�g�Z���^�[IP�A�h���X */
	RauConfig.Dpa_port_Cre = prm_get(COM_PRM, S_CRE, 23, 5, 1);		/* �N���W�b�g�Z���^�[�|�[�g�ԍ� */
	RauConfig.Dpa_port_m3 = prm_get(COM_PRM, S_CRE, 24, 5, 1);
	RauConfig.Credit_snd_rty_cnt = (uchar)prm_get(COM_PRM, S_CRE, 25, 1, 5);
	RauConfig.Credit_nak_rty_cnt = (uchar)prm_get(COM_PRM, S_CRE, 25, 1, 4);
	RauConfig.Credit_ack_wait_tm = 0 - (ushort)prm_get(COM_PRM, S_CRE, 25, 3, 1);
	RauConfig.Credit_snd_rty_cnt_disconnected = RAU_RECONNECT_MAX;
	RauConfig.Credit_ack_wait_reconnect_tm = (ushort)( 0 - RAU_RECONNECT_INTERVAL );
	s_work = (short)prm_get(COM_PRM, S_CRE, 26, 3, 1);
	RauConfig.Credit_discnct_tm	= 0 - (s_work * 10);
// GG129000(S) W.Hoshino 2023/03/23 #6986 �N���W�b�g�̃f�[�^�R�l�N�V�����ݒ莞�A�Q�Ɛ�̋��ʃp�����[�^�A�h���X���ԈႦ�Ă���[���ʉ��P���� No 1550]
//	RauConfig.Credit_cnct_rty_cnt = (uchar)prm_get(COM_PRM, S_CRE, 26, 2, 4);
	RauConfig.Credit_cnct_rty_cnt = (uchar)prm_get(COM_PRM, S_CRE, 26, 1, 4);
// GG129000(E) W.Hoshino 2023/03/23 #6986 �N���W�b�g�̃f�[�^�R�l�N�V�����ݒ莞�A�Q�Ɛ�̋��ʃp�����[�^�A�h���X���ԈႦ�Ă���[���ʉ��P���� No 1550]
}
void	Credit_Init(void)
{
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iSRAM�e�ʊm�ہj
//	memset(&DPA_Credit_SndCtrl, 0, sizeof(DPA_Credit_SndCtrl));	// �N���W�b�g����̑��M�L���[
//	memset(&DPA_Credit_RcvCtrl, 0, sizeof(DPA_Credit_RcvCtrl));	// �N���W�b�g����̎�M�L���[
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iSRAM�e�ʊm�ہj
	memset(&Credit_Snd_Buf, 0, sizeof(Credit_Snd_Buf));
	memset(&Credit_Rcv_Buf, 0, sizeof(Credit_Rcv_Buf));
	memset(&Credit_SndNtBlk, 0, sizeof(Credit_SndNtBlk));
	Cre_ucLastNtBlockNum = 0;					// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
	Cre_uiArcReceiveLen = 0;					// ��M�����m�s�u���b�N���̍��v
	Cre_uiLastTcpBlockNum = 0;					// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
	Cre_uiDopaReceiveLen = 0;					// ��M�����m�s�u���b�N���̍��v
	RAU_uc_mode_CRE = 0;						//	�i�N���W�b�g�ʐM���j�ʐM��ԃ��[�h
	RAU_uc_retrycnt_CRE = 0;					//	�i�N���W�b�g�ʐM���j���M���g���C�J�E���^
	Credit_nakretry_cnt = 0;
	memset(&RAU_uc_txdata_CRE_keep, 0, sizeof(RAU_uc_txdata_CRE_keep));				//	�i�N���W�b�g�ʐM���j�O�񑗐M�����f�[�^��ۑ�
	RAU_ui_txdlength_CRE_keep = 0;
	RAU_ui_data_length_CRE = 0;					//	�i�N���W�b�g�ʐM���j��M�o�C�g���J�E���^
	RAU_ui_txdlength_CRE = 0;					//	�i�N���W�b�g�ʐM���j���M�f�[�^���i���C���p�j
	memset(&RAU_huc_txdata_CRE, 0, sizeof(RAU_huc_txdata_CRE));		//	�i�N���W�b�g�ʐM���j���M�d���o�b�t�@
	memset(&RAU_huc_rcv_work_CRE, 0,sizeof(RAU_huc_rcv_work_CRE));	//	�i�N���W�b�g�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
	memset(&RAU_Tm_Ackwait_CRE, 0, sizeof(RAU_TIME_AREA));							// ���������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
	RAU_uc_retrycnt_disconnected_CRE = 0;		//	�i�N���W�b�g�ʐM���jTCP�ؒf�����M���g���C�J�E���^
	memset(&RAU_Tm_Ackwait_reconnect_CRE, 0, sizeof(RAU_TIME_AREA));				// �N���W�b�g�������f�[�^���M����Ƃ��̂`�b�j�҂����̍Đڑ��^�C�}

	Credit_seq_bak = 0;							// �V�[�P���V�������o�b�N�A�b�v
	Credit_SeqFlag = 0;							// ��M�V�[�P���X�t���O
	Credit_TcpBlockSts = 0;						// TCP�u���b�N�X�e�[�^�X
	Credit_TcpBlockNo = 0;						// TCP�u���b�N�i���o�[
	Credit_Error01 = 0;							// �G���[�R�[�h01������
	memset(&Credit_SendNtDataInfo, 0, sizeof(T_SEND_NT_DATA));				// ���M�Ώۂ̃e�[�u���o�b�t�@����擾����NT�f�[�^�̏��
	memset(&Credit_SendIdInfo, 0, sizeof(T_SEND_DATA_ID_INFO));

	Credit_RAUDPA_seq = 0;						// �N���W�b�g�p���M���V�[�P���V�����ԍ�
	
	CRE_tm_TCP_TIME_WAIT.tm = 0;
	Credit_TCPtime_wait_ov = 1;
	
	Credit_rcvretry_cnt = 0;
	Credit_SeqReset_rq = 0;
	Credit_SeqReset_tm = 0;
	
	/* �o�b�t�@�̃N���A */
	memset(&RecvCreditDt, 0, sizeof(CreNtDataBuf));
	memset(&RAU_huc_txdata_CRE, 0, sizeof(RAU_huc_txdata_CRE));
	memset(&RAU_huc_rcv_work_CRE,0, sizeof(RAU_huc_rcv_work_CRE));
	
	memset(&Credit_SndBuf_info, 0, sizeof(RAU_DATA_TABLE_INFO));
	Credit_SndBuf_info.pc_AreaTop				= &Credit_Snd_Buf[0];
	Credit_SndBuf_info.ul_AreaSize				= RAU_BLKSIZE;
	Credit_SndBuf_info.pc_ReadPoint				= Credit_SndBuf_info.pc_AreaTop;
	Credit_SndBuf_info.pc_WritePoint			= Credit_SndBuf_info.pc_AreaTop;
	
	memset(&Credit_RcvBuf_info, 0, sizeof(RAU_DATA_TABLE_INFO));
	Credit_RcvBuf_info.pc_AreaTop				= &Credit_Rcv_Buf[0];
	Credit_RcvBuf_info.ul_AreaSize				= RAU_BLKSIZE;
	Credit_RcvBuf_info.pc_ReadPoint				= Credit_RcvBuf_info.pc_AreaTop;
	Credit_RcvBuf_info.pc_WritePoint			= Credit_RcvBuf_info.pc_AreaTop;
}

/*[]----------------------------------------------------------------------[]*/
/*|             rautask() 	                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|          RAU task main routine                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	rautask( void )
{
	RAU_TimerInit();
	RAU_init();
	RAUid_Init();
	RAU_SockInit();				// �\�P�b�g���䏉����
	Credit_Init();

	for( ;; ){
		taskchg( IDLETSKNO );
		
		RAU_SockMain();			// �\�P�b�g���䃁�C������

		if(_is_ntnet_remote()) {
			RAU_FuncMain();			/* RAU process exec */
		}		
		Credit_main();			/* �N���W�b�g���� */
	}
}
