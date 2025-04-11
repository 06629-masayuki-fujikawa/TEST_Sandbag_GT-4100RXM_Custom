#include	"rau.h"
#include	"raudef.h"
/*------------------------------------------------------------------------------*/
#pragma	section	_UNINIT3		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:�A�h���X���0x06039000-0x060FFFFF(1MB)

RAU_TIME_AREA	RAU_Tm_DoPaAtCom;				// DoPa��������
RAU_TIME_AREA	RAU_Tm_No_Action;				// ���ʐM�^�C�}(������ɂ��_�~�[�f�[�^���M�Ԋu)
RAU_TIME_AREA	RAU_Tm_Port_Watchdog;			// �������ʐM�Ď��^�C�}(�������̂s�b�o�ڑ������ؒf����)
RAU_TIME_AREA	RAU_Tm_TCP_CNCT;				// TCP�R�l�N�V�����ڑ��҂��^�C�}�[
RAU_TIME_AREA	RAU_Tm_DISCONNECT;				// �R�l�N�V�����ؒf�҂��^�C�}�[
RAU_TIME_AREA	RAU_Tm_TCP_TIME_WAIT;			// �s�b�o�R�l�N�V�����s�h�l�d�Q�v�`�h�s�����҂��^�C�}
RAU_TIME_AREA	RAU_Tm_data_rty;				// �f�[�^�đ��҂��^�C�}�[(��) 100ms�x�[�X��1min�^�C�}�[�g�p
RAU_TIME_AREA	RAU_Tm_TCP_DISCNCT;				// TCP�R�l�N�V�����ؒf�҂��^�C�}�[

RAU_TIME_AREA	CRE_tm_TCP_TIME_WAIT;			// �s�b�o�R�l�N�V�����s�h�l�d�Q�v�`�h�s�����҂��^�C�}
unsigned long	RAU_x_1mLifeTime;				/* timer counter. up to every 10ms */

t_DPA_Ctrl	DPA2_Snd_Ctrl;						// �������̑��M�L���[
t_DPA2_Ctrl	DPA2_Rcv_Ctrl;						// ������̎�M�L���[

t_DPA_Ctrl	DPA_Snd_Ctrl;						// ������̑��M�L���[
t_DPA_Ctrl	DPA_Rcv_Ctrl;						// �������̎�M�L���[

uchar		RAU_temp_data[970];					// �e��ʒm�p�̃e���|�����o�b�t�@�i�PNT�u���b�N���j


/* DataTable.c */
T_RAU_TABLE_DATA		RAU_table_data;
RAU_DATA_TABLE_INFO		rau_data_table_info[RAU_TABLE_MAX];
int						rau_table_crear_flag;
uchar					*RAU_table_top[RAU_TABLE_MAX];
/* Host.c */
uchar					RAU_ucSendSeqFlag;						// ���M�V�[�P���X�t���O
uchar					RAU_ucReceiveSeqFlag;					// ��M�V�[�P���X�t���O
uchar					RAU_ucTcpBlockSts;						// TCP�u���b�N�X�e�[�^�X
uchar					RAU_ucTcpBlockSts2;						// ���������M�f�[�^�p�s�b�o�u���b�N�X�e�[�^�X
ushort					RAU_uiTcpBlockNo;						// TCP�u���b�N�i���o�[
ushort					RAU_uiTcpBlockNo2;						// ���������M�f�[�^�p�s�b�o�u���b�N�ԍ�
RAU_SEND_REQUEST		RAUhost_SendRequest;					// ���M�v���f�[�^
uchar					RAUhost_SendRequestFlag;				// ���M�v���f�[�^����t���O
uchar					RAUhost_Error01;						// �G���[�R�[�h01������
T_SEND_NT_DATA			RAUhost_SendNtDataInfo;					// ���M�Ώۂ̃e�[�u���o�b�t�@����擾����NT�f�[�^�̏��
T_SEND_DATA_ID_INFO		*pRAUhost_SendIdInfo;					// ���M�Ώۂ̃e�[�u���o�b�t�@���̎Q��
T_SEND_NT_DATA			RAUhost_SendNtThroughInfo;				// ���������瑗�M����X���[�f�[�^�e�[�u���̂m�s�f�[�^���
T_SEND_DATA_ID_INFO		*pRAUhost_SendThroughInfo;				// ���������瑗�M����X���[�f�[�^�e�[�u���̏��
uchar					RAU_f_RcvSeqCnt_rq;						// ��M�V�[�P���X�^�C�}�[�t���O
ushort					RAU_Tm_RcvSeqCnt;						// ��M�V�[�P���X�t���O�L�����Z���^�C�}�[
uchar					RAU_f_SndSeqCnt_rq;						// ��M�V�[�P���X�^�C�}�[�t���O
ushort					RAU_Tm_SndSeqCnt;						// ��M�V�[�P���X�t���O�L�����Z���^�C�}�[
/* IDproc.c */
uchar					RAU_special_table_send_buffer[RAU_SPECIAL_SEND_BUFF_SIZE];		// �s���v����K�Ǘ��f�[�^�̉��u�o�b���M�p����o�b�t�@
RAU_COMMUNICATION_TEST	RemoteCheck_Buffer100_101;				// �ʐM�`�F�b�N�p�o�b�t�@(�h�c�P�O�O�E�P�O�P)
RAU_COMMUNICATION_TEST	RemoteCheck_Buffer116_117;				// �ʐM�`�F�b�N�p�o�b�t�@(�h�c�P�P�U�E�P�P�V)
uchar					RauCT_SndReqID;							// �ʐM�`�F�b�N�f�[�^���M�v���h�c(0:�v���Ȃ�, 100:�h�c�P�O�O���M�v��, 117:�h�c�P�P�V���M�v��)
uchar					Rau_SedEnable;							// ���M����
uchar					RAU_ucLastNtBlockNum;					// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
ushort					RAU_uiArcReceiveLen;					// ��M�����m�s�u���b�N���̍��v
ushort					RAU_uiLastTcpBlockNum;					// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
ushort					RAU_uiDopaReceiveLen;					// ��M�����m�s�u���b�N���̍��v
/* Que.c */
T_SEND_DATA_ID_INFO		RAUque_SendDataInfo[RAU_SEND_TABLE_MAX];	// ���M���ׂ��e�e�[�u���̏��(�X���[�f�[�^(������)�܂�)
ushort					RAUque_CurrentSendData;
/* RAUinRam.c */
ushort					RAU_Tm_Reset_t;							// �@�탊�Z�b�g�o�̓^�C�}
/* RAUmain.c */
uchar					RAU_f_TmStart;							// 1=Start, 0=don't do process
/* Terminal.c */
T_RAU_ARC_SNDBLK		RAUarc_SndNtBlk;
/* ram.h */
uchar					RAU_uc_mode_h;							//	�iHOST�ʐM���j�ʐM��ԃ��[�h
uchar					RAU_uc_mode_h2;							// �������̒ʐM��ԃ��[�h(S_H_IDLE:�A�C�h��, S_H_ACK:�`�b�j�҂�)
uchar					RAU_uc_retrycnt_h;						//	�iHOST�ʐM���j���M���g���C�J�E���^
uchar					RAU_uc_retrycnt_h2;						// �������p�̑��M���g���C�J�E���^
uchar					RAU_uc_txdata_h_keep[26];				//	�iHOST�ʐM���j�O�񑗐M�����f�[�^��ۑ�
uchar					RAU_uc_txdata_h_keep2[26];				// ���������瑗�M�����f�[�^�̂c���o���w�b�_������ێ�����o�b�t�@
uchar					RAU_uc_rcvretry_cnt;					// ����������̓���f�[�^��M�񐔃J�E���^
ushort					RAU_ui_txdlength_t;						//	�i�[���ʐM���j���M�f�[�^���i���C���p�j
ushort					RAU_ui_data_length_h;					//	�iHOST�ʐM���j��M�o�C�g���J�E���^
ushort					RAU_ui_txdlength_h;						//	�iHOST�ʐM���j���M�f�[�^���i���C���p�j
ushort					RAU_ui_txdlength_h2;					// �������֑��M����f�[�^�̒���
ushort					RAU_ui_txdlength_h_keep;
ushort					RAU_ui_txdlength_h_keep2;
ulong					RAU_ui_seq_bak;							// �V�[�P���V�������o�b�N�A�b�v
uchar					RAU_huc_rcv_work_t[RAU_RCV_MAX_T];		//	�i�[���ʐM���j��M�e�L�X�g���[�N�o�b�t�@
uchar					RAU_huc_txdata_h[RAU_SEND_MAX_H];		//	�iHOST�ʐM���j���M�d���o�b�t�@
uchar					RAU_huc_txdata_h2[RAU_SEND_MAX_H];		// �������̑��M�d���o�b�t�@
uchar					RAU_huc_rcv_work_h[RAU_RCV_MAX_H];		//	�iHOST�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
uchar					RAU_huc_rcv_tmp[RAU_RCV_MAX_H];			//	�iHOST�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
uchar					RAU_huc_rcv_buf[3][RAU_RCV_MAX_H];		//	�iHOST�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
ushort					RAU_huc_rcv_len[3];						//	�iHOST�ʐM���j��M�o�C�g���J�E���^
RAU_TIME_AREA			RAU_Tm_CommuTest;						// �ʐM�`�F�b�N�p�^�C�}
RAU_TIME_AREA			RAU_Tm_Ackwait;							// ���������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
RAU_TIME_AREA			RAU_Tm_Ackwait2;						// ����������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
uchar					RAU_uc_retrycnt_reconnect_h;			//	�iHOST�ʐM���j�Đڑ����g���C�J�E���^
RAU_TIME_AREA			RAU_Tm_Ackwait_reconnect;				// ���������f�[�^���M����Ƃ��̍Đڑ��҂��^�C�}
t_DPA_Ctrl	DPA_Credit_SndCtrl;						// �N���W�b�g����̑��M�L���[
t_DPA_Ctrl	DPA_Credit_RcvCtrl;						// �N���W�b�g����̎�M�L���[

uchar	Credit_Snd_Buf[RAU_BLKSIZE];			// (*T_SEND_DATA_ID_INFO)->pbuff_info.pc_AreaTop�Ɏw�肷��ꏊ
uchar	Credit_Rcv_Buf[RAU_BLKSIZE];
T_RAU_ARC_SNDBLK		Credit_SndNtBlk;

uchar					Cre_ucLastNtBlockNum;				// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
ushort					Cre_uiArcReceiveLen;					// ��M�����m�s�u���b�N���̍��v
ushort					Cre_uiLastTcpBlockNum;				// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
ushort					Cre_uiDopaReceiveLen;				// ��M�����m�s�u���b�N���̍��v
uchar					RAU_uc_mode_CRE;						//	�i�N���W�b�g�ʐM���j�ʐM��ԃ��[�h
uchar					RAU_uc_retrycnt_CRE;					//	�i�N���W�b�g�ʐM���j���M���g���C�J�E���^
uchar					Credit_nakretry_cnt;
uchar					RAU_uc_txdata_CRE_keep[26];				//	�i�N���W�b�g�ʐM���j�O�񑗐M�����f�[�^��ۑ�
uchar					RAU_ui_txdlength_CRE_keep;
ushort					RAU_ui_data_length_CRE;					//	�i�N���W�b�g�ʐM���j��M�o�C�g���J�E���^
ushort					RAU_ui_txdlength_CRE;					//	�i�N���W�b�g�ʐM���j���M�f�[�^���i���C���p�j
uchar					RAU_huc_txdata_CRE[RAU_SEND_MAX_H];		//	�i�N���W�b�g�ʐM���j���M�d���o�b�t�@
uchar					RAU_huc_rcv_work_CRE[RAU_RCV_MAX_H];	//	�i�N���W�b�g�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
RAU_TIME_AREA			RAU_Tm_Ackwait_CRE;							// ���������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
uchar					RAU_uc_retrycnt_disconnected_CRE;		//	�i�N���W�b�g�ʐM���jTCP�ؒf�����M���g���C�J�E���^
RAU_TIME_AREA			RAU_Tm_Ackwait_reconnect_CRE;			// �N���W�b�g�������f�[�^���M����Ƃ��̂`�b�j�҂����̍Đڑ��^�C�}

ulong					Credit_seq_bak;							// �V�[�P���V�������o�b�N�A�b�v
uchar					Credit_SeqFlag;					// ��M�V�[�P���X�t���O
uchar					Credit_TcpBlockSts;						// TCP�u���b�N�X�e�[�^�X
ushort					Credit_TcpBlockNo;						// TCP�u���b�N�i���o�[
uchar					Credit_Error01;						// �G���[�R�[�h01������
T_SEND_NT_DATA			Credit_SendNtDataInfo;				// ���M�Ώۂ̃e�[�u���o�b�t�@����擾����NT�f�[�^�̏��
T_SEND_DATA_ID_INFO		Credit_SendIdInfo;					// ���M���ׂ��N���W�b�g�f�[�^�̏��
RAU_DATA_TABLE_INFO		Credit_SndBuf_info;					// �N���W�b�g�f�[�^�̃o�b�t�@�[���
RAU_DATA_TABLE_INFO		Credit_RcvBuf_info;					// �N���W�b�g�f�[�^�̃o�b�t�@�[���
ushort					Credit_RAUDPA_seq;						// �N���W�b�g�p���M���V�[�P���V�����ԍ�
uchar					Credit_rcvretry_cnt;					// ����������̓���f�[�^��M�񐔃J�E���^
uchar					Credit_SeqReset_rq;						// �V�[�P���X�^�C�}�[�t���O
ushort					Credit_SeqReset_tm;						// �V�[�P���X�t���O�L�����Z���^�C�}�[
/* IDProc.h */
ushort					RAU_ui_RAUDPA_seq;						// ���uDopa�p���M���V�[�P���V�����ԍ�
ushort					RAU_ui_RAUDPA_seq2;						// ���������M�f�[�^�p�c���o���p�P�b�g�V�[�P���V�����ԍ�

_tsum_man				RAU_tsum_man;
_centerterminfo_man		RAU_centerterminfo_man;
