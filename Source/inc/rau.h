#ifndef	___RAUH___
#define	___RAUH___
/*[]----------------------------------------------------------------------[]
 *|	filename: rau.h
 *[]----------------------------------------------------------------------[]
 *| summary	: RAU���W���[�� RAM���L���p���b�p�[��`
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/

#include	"raudef.h"
#include	"rauIDproc.h"

#define RAU_IBW_TEMP_BUFF_SIZE	31044		// 970 * 32 + 4 (NT-DATA * 32 + Len + CRC)
#define RAU_HOST_TEMP_BUFF_SIZE	25224		// 970 * 26 + 4 (NT-DATA * 26 + Len + CRC)
#define RAU_SPECIAL_SEND_BUFF_SIZE	16494	// 970 * 17 + 4 (NT-DATA * 17 + Len + CRC) ���s���v��s���v����K�Ǘ��f�[�^�̉��u�o�b���M�p����o�b�t�@�̕K�v�T�C�Y
#define RAU_TABLE_DATA_MAX	31				// 25����31�֕ύX #001

#define		RAU_IN_PARKING_SIZE		0			// ���Ƀf�[�^			�iID20�j
#define		RAU_OUT_PARKING_SIZE	0			// �o�Ƀf�[�^			�iID21�j
#define		RAU_PAID_DATA22_SIZE	0			// ���Z�f�[�^			�iID22�j
#define		RAU_PAID_DATA23_SIZE	0			// ���Z�f�[�^			�iID23�j
#define		RAU_TOTAL_SIZE			37000		// T���v�W�v�f�[�^		�iID30�`38�E41�j
#define		RAU_ERROR_SIZE			0			// �G���[�f�[�^			�iID120�j
#define		RAU_ALARM_SIZE			0			// �A���[���f�[�^		�iID121�j
#define		RAU_MONITOR_SIZE		0			// ���j�^�[�f�[�^		�iID122�j
#define		RAU_OPE_MONITOR_SIZE	0			// ���샂�j�^�[�f�[�^	�iID123�j
#define		RAU_COIN_TOTAL_SIZE		0			// �R�C�����ɏW�v�f�[�^	�iID131�j
#define		RAU_MONEY_TORAL_SIZE	0			// �������ɏW�v�f�[�^	�iID133�j
#define		RAU_PARKING_NUM_SIZE	0			// ���ԑ䐔�E���ԃf�[�^	�iID236�j
#define		RAU_AREA_NUM_SIZE		0			// ���䐔�E���ԃf�[�^	�iID237�j
#define		RAU_MONEY_MANAGE_SIZE	0			// ���K�Ǘ��f�[�^		�iID126�j(176 + 7 + 4) * 32
#define		RAU_TURI_MANAGE_SIZE	0			// �ޑK�Ǘ��W�v�f�[�^	 (ID135)
#define		RAU_REMOTE_MONITOR_SIZE	0			// ���u�Ď��f�[�^		�iID125�j
// GG120600(S) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
//#define		RAU_CENTER_TERM_INFO_SIZE		492	// �Z���^�[�p�[�����f�[�^	�iID65�jsizeof(_ctib)=246 * 2
#define		RAU_CENTER_TERM_INFO_SIZE		582	// �Z���^�[�p�[�����f�[�^	�iID65�jsizeof(_ctib)=292 * 2
// GG120600(E) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
#define		RAU_LONG_PARKING_INFO_SIZE		0	// �������ԏ��f�[�^	�iID61�j
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
#define		RAU_SEND_THROUGH_DATA_SIZE		31168	// �X���[�f�[�^	�i970 + 4�j�~ 32
#define		RAU_RECEIVE_THROUGH_DATA_SIZE	25324	// �X���[�f�[�^	�i970 + 4�j�~ 26

#define		RAU_NTDATA_MAX			25222	//(970 * 26) + 2

#define		RAU_LOG_BUFF_SIZE		20000

/****************************************************************/
/*				 �Pbyte�̃O���[�o���ϐ���`						*/
/****************************************************************/

#define		RAU_f_CommuTest_rq	RAU_Tm_CommuTest.bits0.f_rq	// �ʐM�`�F�b�N�p�^�C�}�̋N���t���O
#define		RAU_f_CommuTest_ov	RAU_Tm_CommuTest.bits0.f_ov	// �ʐM�`�F�b�N�p�^�C�}�̃^�C���A�E�g�t���O

#define		RAU_f_Ackwait_rq	RAU_Tm_Ackwait.bits0.f_rq	// ���������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}�N���t���O
#define		RAU_f_Ackwait_ov	RAU_Tm_Ackwait.bits0.f_ov	// ���������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}�^�C���A�E�g�t���O
#define		RAU_f_Ackwait_reconnect_rq	RAU_Tm_Ackwait_reconnect.bits0.f_rq	// ���������f�[�^���M����Ƃ��̍Đڑ��^�C�}�N���t���O
#define		RAU_f_Ackwait_reconnect_ov	RAU_Tm_Ackwait_reconnect.bits0.f_ov	// ���������f�[�^���M����Ƃ��̍Đڑ��^�C�}�^�C���A�E�g�t���O

#define		RAU_f_Ackwait2_rq	RAU_Tm_Ackwait2.bits0.f_rq	// ����������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}�N���t���O
#define		RAU_f_Ackwait2_ov	RAU_Tm_Ackwait2.bits0.f_ov	// ����������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}�^�C���A�E�g�t���O
#define		Credit_Ackwait_rq	RAU_Tm_Ackwait_CRE.bits0.f_rq
#define		Credit_Ackwait_ov	RAU_Tm_Ackwait_CRE.bits0.f_ov
#define		Credit_Ackwait_reconnect_rq	RAU_Tm_Ackwait_reconnect_CRE.bits0.f_rq
#define		Credit_Ackwait_reconnect_ov	RAU_Tm_Ackwait_reconnect_CRE.bits0.f_ov

enum {
	RAU_IN_PARKING_TABLE,			// ���Ƀf�[�^			�iID20�j
	RAU_OUT_PARKING_TABLE,			// �o�Ƀf�[�^			�iID21�j
	RAU_PAID_DATA22_TABLE,			// ���Z�f�[�^			�iID22�j
	RAU_PAID_DATA23_TABLE,			// ���Z�f�[�^			�iID23�j
	RAU_ERROR_TABLE,				// �G���[�f�[�^			�iID120�j
	RAU_ALARM_TABLE,				// �A���[���f�[�^		�iID121�j
	RAU_MONITOR_TABLE,				// ���j�^�[�f�[�^		�iID122�j
	RAU_OPE_MONITOR_TABLE,			// ���샂�j�^�[�f�[�^	�iID123�j
	RAU_COIN_TOTAL_TABLE,			// �R�C�����ɏW�v�f�[�^	�iID131�j
	RAU_MONEY_TORAL_TABLE,			// �������ɏW�v�f�[�^	�iID133�j
	RAU_PARKING_NUM_TABLE,			// ���ԑ䐔�E���ԃf�[�^	�iID236�j
	RAU_AREA_NUM_TABLE,				// ���䐔�E���ԃf�[�^	�iID237�j
	RAU_TURI_MANAGE_TABLE,			// �ޑK�Ǘ��W�v�f�[�^	�iID135�j
	RAU_TOTAL_TABLE,				// T���v�W�v�f�[�^		�iID30�`38�E41�j
	RAU_GTOTAL_TABLE,				// GT���v�W�v�f�[�^		�iID42,43,44,45,46,48�j
	RAU_MONEY_MANAGE_TABLE,			// ���K�Ǘ��f�[�^		�iID126�j
	RAU_REMOTE_MONITOR_TABLE,		// ���u�Ď��f�[�^		�iID125�j
	RAU_CENTER_TERM_INFO_TABLE,		// �Z���^�[�p�[�����f�[�^�iID65�j
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	RAU_LONG_PARKING_INFO_TABLE,	// �������ԏ��f�[�^	(ID61�j
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	RAU_SEND_THROUGH_DATA_TABLE,	// ���M�X���[�f�[�^		�iID �s��j
	RAU_RECEIVE_THROUGH_DATA_TABLE,	// ��M�X���[�f�[�^		�iID �s��j
	RAU_TABLE_MAX
};
#define	RAU_SEND_TABLE_MAX			(RAU_SEND_THROUGH_DATA_TABLE+1)		// ���M���ׂ��e�e�[�u���̏��(�X���[�f�[�^(������)�܂�)

enum {
	RAU_ID20_IN_PARKING,			// ID20		���Ƀf�[�^
	RAU_ID21_OUT_PARKING,			// ID21		�o�Ƀf�[�^
	RAU_ID22_PAID_DATA22,			// ID22		���Z�f�[�^
	RAU_ID23_PAID_DATA23,			// ID23		���Z�f�[�^
	RAU_ID54_IN_PARKING,			// ID54		���Ƀf�[�^
	RAU_ID55_OUT_PARKING,			// ID55		�o�Ƀf�[�^
	RAU_ID56_PAID_DATA22,			// ID56		���Z�f�[�^
	RAU_ID57_PAID_DATA23,			// ID57		���Z�f�[�^
	RAU_ID120_ERROR,				// ID120	�G���[�f�[�^
	RAU_ID121_ALARM,				// ID121	�A���[���f�[�^
	RAU_ID122_MONITOR,				// ID122	���j�^�f�[�^
	RAU_ID123_OPE_MONITOR,			// ID123	���샂�j�^�f�[�^
	RAU_ID131_COIN_TOTAL,			// ID131	�R�C�����ɏW�v���v�f�[�^
	RAU_ID133_MONEY_TORAL,			// ID133	�������ɏW�v���v�f�[�^
	RAU_ID236_PARKING_NUM,			// ID236	���ԑ䐔�f�[�^
	RAU_ID58_PARKING_NUM,			// ID58		Web�p���ԑ䐔�f�[�^
	RAU_ID237_AREA_NUM,				// ID237	���䐔�E���ԃf�[�^
	RAU_ID30_TOTAL,					// ID30		T���v�W�v�f�[�^
	RAU_ID31_TOTAL,					// ID31		T���v�W�v�f�[�^
	RAU_ID32_TOTAL,					// ID32		T���v�W�v�f�[�^
	RAU_ID33_TOTAL,					// ID33		T���v�W�v�f�[�^
	RAU_ID34_TOTAL,					// ID34		T���v�W�v�f�[�^
	RAU_ID35_TOTAL,					// ID35		T���v�W�v�f�[�^
	RAU_ID36_TOTAL,					// ID36		T���v�W�v�f�[�^
	RAU_ID37_TOTAL,					// ID37		T���v�W�v�f�[�^
	RAU_ID38_TOTAL,					// ID38		T���v�W�v�f�[�^
	RAU_ID41_TOTAL,					// ID41		T���v�W�v�f�[�^
	RAU_ID42_TOTAL,					// ID42		�t�H�[�}�b�gRevNo.10 �W�v��{�f�[�^
	RAU_ID43_TOTAL,					// ID43		�t�H�[�}�b�gRevNo.10 ������ʖ��W�v�f�[�^
	RAU_ID45_TOTAL,					// ID44		�t�H�[�}�b�gRevNo.10 �����W�v�f�[�^
	RAU_ID46_TOTAL,					// ID45		�t�H�[�}�b�gRevNo.10 ����W�v�f�[�^
	RAU_ID49_TOTAL,					// ID46		�t�H�[�}�b�gRevNo.10 �^�C�����W�W�v�f�[�^
	RAU_ID53_TOTAL,					// ID48		�t�H�[�}�b�gRevNo.10 �W�v�I���ʒm�f�[�^
	RAU_ID126_MONEY_MANAGE,			// ID126	���K�Ǘ��f�[�^
	RAU_ID135_TURI_MANAGE,			// ID135	�ޑK�Ǘ��W�v�f�[�^
	RAU_ID59_REMOTE_MONITOR,		// ID125	���u�Ď��f�[�^
	RAU_ID65_CENTER_TERM_INFO,		// ID65		�Z���^�[�p�[�����f�[�^
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	RAU_ID61_LONG_PARKING_INFO,		// ID61		�������ԏ��f�[�^
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	RAU_ID0_SEND_THROUGH_DATA,		// ���M�X���[�f�[�^		�iID �s��j
	RAU_ID0_RECEIVE_THROUGH_DATA,	// ��M�X���[�f�[�^		�iID �s��j
	RAU_ID_BUFF_MAX,
	RAU_ID_BUFF_NUM
};

typedef struct {
	uchar total[RAU_TOTAL_SIZE];						// T���v�W�v�f�[�^		�iID30�`38�E41�j
	uchar center_term_info[RAU_CENTER_TERM_INFO_SIZE];	// �Z���^�[�p�[�����f�[�^�iID65�j
	uchar send_through_data[RAU_SEND_THROUGH_DATA_SIZE];		// ���M�X���[�f�[�^		�iID �s��j�����M�̂�32block�Ɋg�� 2006.08.31:m-onouchi
	uchar receive_through_data[RAU_RECEIVE_THROUGH_DATA_SIZE];	// ��M�X���[�f�[�^		�iID �s��j
}T_RAU_TABLE_DATA;

// �^�C�}��`
union RAU_time_area
{
	struct	tim_tag0
	{
		ushort	f_ov	 : 1  ;		/* �r�b�g�@�P�T		*/
		ushort	f_rq	 : 1  ;		/* �r�b�g�@�P�S		*/
		ushort	bit_0_13 : 14 ;		/* �r�b�g�O�`�P�R	*/

	} bits0;
	ushort	tm;
};
typedef	union	RAU_time_area	RAU_TIME_AREA;	/* �^�C�}�E�G���A		*/

typedef struct {
	ulong	size;
	uchar	id;
} RAU_ID_INIT_INFO;

typedef struct _tsumm {
	struct _ntman	man;
	struct {
		char	*sendtop;
	} dirty;
} _tsum_man;

typedef struct _ctim {
	char	*top;		/* center term info buffer top */
	char	*bottom;	/* center term info buffer bottom */
	char	*freeq;		/* free buffer queue */
	char	*sendq;		/* send buffer queue */
	char	*readq;		/* send buffer queue */
} _centerterminfo_man;

/* �\�[�X��V���{�����̉��� */
/* DataTable.c */
extern		T_RAU_TABLE_DATA		RAU_table_data;
extern		RAU_DATA_TABLE_INFO		rau_data_table_info[RAU_TABLE_MAX];
extern		int						rau_table_crear_flag;
extern		uchar					*RAU_table_top[RAU_TABLE_MAX];
/* Host.c */
extern		uchar					RAU_ucSendSeqFlag;						// ���M�V�[�P���X�t���O
extern		uchar					RAU_ucReceiveSeqFlag;					// ��M�V�[�P���X�t���O
extern		uchar					RAU_ucTcpBlockSts;						// TCP�u���b�N�X�e�[�^�X
extern		uchar					RAU_ucTcpBlockSts2;						// ���������M�f�[�^�p�s�b�o�u���b�N�X�e�[�^�X
extern		ushort					RAU_uiTcpBlockNo;						// TCP�u���b�N�i���o�[
extern		ushort					RAU_uiTcpBlockNo2;						// ���������M�f�[�^�p�s�b�o�u���b�N�ԍ�
extern		RAU_SEND_REQUEST		RAUhost_SendRequest;					// ���M�v���f�[�^
extern		uchar					RAUhost_SendRequestFlag;				// ���M�v���f�[�^����t���O
extern		uchar					RAUhost_Error01;						// �G���[�R�[�h01������
extern		T_SEND_NT_DATA			RAUhost_SendNtDataInfo;					// ���M�Ώۂ̃e�[�u���o�b�t�@����擾����NT�f�[�^�̏��
extern		T_SEND_DATA_ID_INFO		*pRAUhost_SendIdInfo;					// ���M�Ώۂ̃e�[�u���o�b�t�@���̎Q��
extern		T_SEND_NT_DATA			RAUhost_SendNtThroughInfo;				// ���������瑗�M����X���[�f�[�^�e�[�u���̂m�s�f�[�^���
extern		T_SEND_DATA_ID_INFO		*pRAUhost_SendThroughInfo;				// ���������瑗�M����X���[�f�[�^�e�[�u���̏��
extern		uchar					RAU_f_RcvSeqCnt_rq;						// ��M�V�[�P���X�^�C�}�[�t���O
extern		ushort					RAU_Tm_RcvSeqCnt;						// ��M�V�[�P���X�t���O�L�����Z���^�C�}�[
extern		uchar					RAU_f_SndSeqCnt_rq;						// ��M�V�[�P���X�^�C�}�[�t���O
extern		ushort					RAU_Tm_SndSeqCnt;						// ��M�V�[�P���X�t���O�L�����Z���^�C�}�[
/* IDproc.c */
extern		uchar					RAU_special_table_send_buffer[RAU_SPECIAL_SEND_BUFF_SIZE];		// �s���v����K�Ǘ��f�[�^�̉��u�o�b���M�p����o�b�t�@
extern		RAU_COMMUNICATION_TEST	RemoteCheck_Buffer100_101;				// �ʐM�`�F�b�N�p�o�b�t�@(�h�c�P�O�O�E�P�O�P)
extern		RAU_COMMUNICATION_TEST	RemoteCheck_Buffer116_117;				// �ʐM�`�F�b�N�p�o�b�t�@(�h�c�P�P�U�E�P�P�V)
extern		uchar					RauCT_SndReqID;							// �ʐM�`�F�b�N�f�[�^���M�v���h�c(0:�v���Ȃ�, 100:�h�c�P�O�O���M�v��, 117:�h�c�P�P�V���M�v��)
extern		uchar					Rau_SedEnable;							// ���M����
extern		uchar					RAU_ucLastNtBlockNum;					// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
extern		ushort					RAU_uiArcReceiveLen;					// ��M�����m�s�u���b�N���̍��v
extern		ushort					RAU_uiLastTcpBlockNum;					// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
extern		ushort					RAU_uiDopaReceiveLen;					// ��M�����m�s�u���b�N���̍��v
/* Que.c */
extern		T_SEND_DATA_ID_INFO		RAUque_SendDataInfo[RAU_SEND_TABLE_MAX];	// ���M���ׂ��e�e�[�u���̏��(�X���[�f�[�^(������)�܂�)
extern		ushort					RAUque_CurrentSendData;
/* RAUinRam.c */
extern		ushort					RAU_Tm_Reset_t;							// �@�탊�Z�b�g�o�̓^�C�}
/* RAUmain.c */
extern		uchar					RAU_f_TmStart;							// 1=Start, 0=don't do process
/* Terminal.c */
extern		T_RAU_ARC_SNDBLK		RAUarc_SndNtBlk;
/* ram.h */
extern		uchar					RAU_uc_mode_h;							//	�iHOST�ʐM���j�ʐM��ԃ��[�h
extern		uchar					RAU_uc_mode_h2;							// �������̒ʐM��ԃ��[�h(S_H_IDLE:�A�C�h��, S_H_ACK:�`�b�j�҂�)
extern		uchar					RAU_uc_retrycnt_h;						//	�iHOST�ʐM���j���M���g���C�J�E���^
extern		uchar					RAU_uc_retrycnt_h2;						// �������p�̑��M���g���C�J�E���^
extern		uchar					RAU_uc_txdata_h_keep[26];				//	�iHOST�ʐM���j�O�񑗐M�����f�[�^��ۑ�
extern		uchar					RAU_uc_txdata_h_keep2[26];				// ���������瑗�M�����f�[�^�̂c���o���w�b�_������ێ�����o�b�t�@
extern		uchar					RAU_uc_rcvretry_cnt;					// ����������̓���f�[�^��M�񐔃J�E���^
extern		ushort					RAU_ui_txdlength_t;						//	�i�[���ʐM���j���M�f�[�^���i���C���p�j
extern		ushort					RAU_ui_data_length_h;					//	�iHOST�ʐM���j��M�o�C�g���J�E���^
extern		ushort					RAU_ui_txdlength_h;						//	�iHOST�ʐM���j���M�f�[�^���i���C���p�j
extern		ushort					RAU_ui_txdlength_h2;					// �������֑��M����f�[�^�̒���
extern		ushort					RAU_ui_txdlength_h_keep;
extern		ushort					RAU_ui_txdlength_h_keep2;
extern		ulong					RAU_ui_seq_bak;							// �V�[�P���V�������o�b�N�A�b�v
extern		uchar					RAU_huc_rcv_work_t[RAU_RCV_MAX_T];		//	�i�[���ʐM���j��M�e�L�X�g���[�N�o�b�t�@
extern		uchar					RAU_huc_txdata_h[RAU_SEND_MAX_H];		//	�iHOST�ʐM���j���M�d���o�b�t�@
extern		uchar					RAU_huc_txdata_h2[RAU_SEND_MAX_H];		// �������̑��M�d���o�b�t�@
extern		uchar					RAU_huc_rcv_work_h[RAU_RCV_MAX_H];		//	�iHOST�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
extern		uchar					RAU_huc_rcv_tmp[RAU_RCV_MAX_H];			//	�iHOST�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
extern		uchar					RAU_huc_rcv_buf[3][RAU_RCV_MAX_H];		//	�iHOST�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
extern		ushort					RAU_huc_rcv_len[3];						//	�iHOST�ʐM���j��M�o�C�g���J�E���^
extern		RAU_TIME_AREA			RAU_Tm_CommuTest;						// �ʐM�`�F�b�N�p�^�C�}
extern		RAU_TIME_AREA			RAU_Tm_Ackwait;							// ���������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
extern		RAU_TIME_AREA			RAU_Tm_Ackwait2;						// ����������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
extern		uchar					RAU_uc_retrycnt_reconnect_h;			//	�iHOST�ʐM���j�Đڑ����g���C�J�E���^
extern		RAU_TIME_AREA			RAU_Tm_Ackwait_reconnect;				// ���������f�[�^���M����Ƃ��̍Đڑ��^�C�}
/* IDProc.h */
extern		ushort					RAU_ui_RAUDPA_seq;						// ���uDopa�p���M���V�[�P���V�����ԍ�
extern		ushort					RAU_ui_RAUDPA_seq2;						// ���������M�f�[�^�p�c���o���p�P�b�g�V�[�P���V�����ԍ�

extern		_tsum_man				RAU_tsum_man;
extern		_centerterminfo_man		RAU_centerterminfo_man;

extern		RAU_TIME_AREA			RAU_Tm_No_Action;				// ���ʐM�^�C�}(������ɂ��_�~�[�f�[�^���M�Ԋu)
#define		RAU_f_No_Action_rq		RAU_Tm_No_Action.bits0.f_rq
#define		RAU_f_No_Action_ov		RAU_Tm_No_Action.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_DISCONNECT;				// �s�b�o�ؒf�҂��^�C�}
#define		RAU_f_discnnect_rq		RAU_Tm_DISCONNECT.bits0.f_rq
#define		RAU_f_discnnect_ov		RAU_Tm_DISCONNECT.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_Port_Watchdog;			// �������ʐM�Ď��^�C�}(�������̂s�b�o�ڑ������ؒf����)
#define		RAU_f_Port_Watchdog_rq	RAU_Tm_Port_Watchdog.bits0.f_rq
#define		RAU_f_Port_Watchdog_ov	RAU_Tm_Port_Watchdog.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_TCP_TIME_WAIT;			// �s�b�o�R�l�N�V�����s�h�l�d�Q�v�`�h�s�����҂��^�C�}
#define		RAU_f_TCPtime_wait_rq	RAU_Tm_TCP_TIME_WAIT.bits0.f_rq
#define		RAU_f_TCPtime_wait_ov	RAU_Tm_TCP_TIME_WAIT.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_data_rty;				// �f�[�^�đ��҂��^�C�}�[(��) 100ms�x�[�X��1min�^�C�}�[�g�p
#define		RAU_f_data_rty_rq		RAU_Tm_data_rty.bits0.f_rq
#define		RAU_f_data_rty_ov		RAU_Tm_data_rty.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_TCP_DISCNCT;				// TCP�R�l�N�V�����ؒf�҂��^�C�}�[
#define		RAU_f_TCPdiscnct_rq		RAU_Tm_TCP_DISCNCT.bits0.f_rq
#define		RAU_f_TCPdiscnct_ov		RAU_Tm_TCP_DISCNCT.bits0.f_ov

extern		uchar	RAU_temp_data[970];								// �e��ʒm�p�̃e���|�����o�b�t�@�i�PNT�u���b�N���j
extern uchar				RAU_LogData[RAU_LOG_BUFF_SIZE];

extern	t_DPA_Ctrl	DPA_Credit_SndCtrl;						// �N���W�b�g����̑��M�L���[
extern	t_DPA_Ctrl	DPA_Credit_RcvCtrl;						// �N���W�b�g����̎�M�L���[

extern	uchar	Credit_Snd_Buf[RAU_BLKSIZE];			// (*T_SEND_DATA_ID_INFO)->pbuff_info.pc_AreaTop�Ɏw�肷��ꏊ
extern	uchar	Credit_Rcv_Buf[RAU_BLKSIZE];			// �P�u���b�N�T�C�Y�~�S����
extern	T_RAU_ARC_SNDBLK		Credit_SndNtBlk;

extern	uchar					RAU_uc_mode_CRE;						//	�i�N���W�b�g�ʐM���j�ʐM��ԃ��[�h
extern	uchar					RAU_uc_retrycnt_CRE;					//	�i�N���W�b�g�ʐM���j���M���g���C�J�E���^
extern	uchar					Credit_nakretry_cnt;
extern	uchar					Credit_rcvretry_cnt;
extern	uchar					RAU_uc_txdata_CRE_keep[26];				//	�i�N���W�b�g�ʐM���j�O�񑗐M�����f�[�^��ۑ�
extern	uchar					RAU_ui_txdlength_CRE_keep;
extern	ushort					RAU_ui_data_length_CRE;					//	�i�N���W�b�g�ʐM���j��M�o�C�g���J�E���^
extern	ushort					RAU_ui_txdlength_CRE;					//	�i�N���W�b�g�ʐM���j���M�f�[�^���i���C���p�j
extern	uchar					RAU_huc_txdata_CRE[RAU_SEND_MAX_H];		//	�i�N���W�b�g�ʐM���j���M�d���o�b�t�@
extern	uchar					RAU_huc_rcv_work_CRE[RAU_RCV_MAX_H];	//	�i�N���W�b�g�ʐM���j��M�e�L�X�g���[�N�o�b�t�@
extern	RAU_TIME_AREA			RAU_Tm_Ackwait_CRE;							// ���������f�[�^���M����Ƃ��̂`�b�j�҂��^�C�}
extern	uchar					RAU_uc_retrycnt_disconnected_CRE;		//	�i�N���W�b�g�ʐM���jTCP�ؒf�����M���g���C�J�E���^
extern	RAU_TIME_AREA			RAU_Tm_Ackwait_reconnect_CRE;			// �N���W�b�g�������f�[�^���M����Ƃ��̂`�b�j�҂����̍Đڑ��^�C�}

extern	uchar					Cre_ucLastNtBlockNum;				// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
extern	ushort					Cre_uiArcReceiveLen;					// ��M�����m�s�u���b�N���̍��v
extern	ushort					Cre_uiLastTcpBlockNum;				// �Ō�Ɏ�M�����m�s�f�[�^�u���b�N�i���o�[
extern	ushort					Cre_uiDopaReceiveLen;				// ��M�����m�s�u���b�N���̍��v

extern	ulong					Credit_seq_bak;							// �V�[�P���V�������o�b�N�A�b�v
extern	uchar					Credit_SeqFlag;					// ��M�V�[�P���X�t���O
extern	uchar					Credit_TcpBlockSts;						// TCP�u���b�N�X�e�[�^�X
extern	ushort					Credit_TcpBlockNo;						// TCP�u���b�N�i���o�[
extern	uchar					Credit_Error01;						// �G���[�R�[�h01������
extern	T_SEND_NT_DATA			Credit_SendNtDataInfo;				// ���M�Ώۂ̃e�[�u���o�b�t�@����擾����NT�f�[�^�̏��
extern	T_SEND_DATA_ID_INFO		Credit_SendIdInfo;					// ���M���ׂ��N���W�b�g�f�[�^�̏��
extern	RAU_DATA_TABLE_INFO		Credit_SndBuf_info;					// �N���W�b�g�f�[�^�̃o�b�t�@�[���
extern	RAU_DATA_TABLE_INFO		Credit_RcvBuf_info;					// �N���W�b�g�f�[�^�̃o�b�t�@�[���
extern	ushort					Credit_RAUDPA_seq;						// �N���W�b�g�p���M���V�[�P���V�����ԍ�
extern		RAU_TIME_AREA			CRE_tm_TCP_TIME_WAIT;			// �s�b�o�R�l�N�V�����s�h�l�d�Q�v�`�h�s�����҂��^�C�}
#define		Credit_TCPtime_wait_rq	CRE_tm_TCP_TIME_WAIT.bits0.f_rq
#define		Credit_TCPtime_wait_ov	CRE_tm_TCP_TIME_WAIT.bits0.f_ov

extern	uchar					Credit_SeqReset_rq;					// �V�[�P���X�^�C�}�[�t���O
extern	ushort					Credit_SeqReset_tm;					// �V�[�P���X�t���O�L�����Z���^�C�}�[

#endif/* ___RAUH___ */
