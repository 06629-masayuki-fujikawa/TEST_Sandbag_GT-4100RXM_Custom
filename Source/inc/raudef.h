/*[]----------------------------------------------------------------------[]
 *|	filename: raudef.h
 *[]----------------------------------------------------------------------[]
 *| summary	: RAU�^�X�N��`�f�[�^�w�b�_
 *| date	: 2012-09-11
 *| update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/

#ifndef	RAUDEF_H
#define	RAUDEF_H

#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ntnet.h"
#include	"rauconstant.h"

#define		RAU_NTDATABUF_COUNT		20
#define		RAU_BLKSIZE				1024					// 1�p�P�b�g��MAX

#define		RAU_SET				1							// SET
#define		RAU_CLR				0							// CLR

#define		COMMON_CMD			0							// ���ʃR�}���h
#define		UPLINE				1							// �����
#define		DOWNLINE			2							// ������
#define		CREDIT				3							// �N���W�b�g���
#define		RAU_ACK				0x06						// ACK
#define		RAU_NAK				0x15						// NAK

#define		RAU_DUMMY_STR_LEN	16							// �_�~�[�f�[�^�T�C�Y

#define		RAU_CONN_RETRY_MAX	3							// �R�l�N�V�����m�����g���C��
#define		RAU_CONN_TIMEOUT	40							// �^�C���A�E�g����(�b)
#define		RAU_TCP_CLOSE_TIME	5 * 100						// �A���e�i���x���`�F�b�N����TCP�ؒf�҂�����(10ms�P��)

#define		RAU_RECONNECT_MAX	9							// ACK�҂����̐ؒf���̍Đڑ���
#define		RAU_RECONNECT_INTERVAL	3						// ACK�҂����̐ؒf���̍Đڑ��Ԋu

#define	NET_STA_IDLE			0	// �A�C�h��
#define	NET_STA_PPP_OPENNING	1	// �o�o�o�ڑ���
#define	NET_STA_PPP_OPEN		2	// �o�o�o�ڑ��m����
#define	NET_STA_TCP_OPENNING	3	// �s�b�o�ڑ���
#define	NET_STA_TCP_OPEN		4	// �s�b�o�ڑ��m����
#define	NET_STA_TCP_CLOSING		5	// �s�b�o�ؒf��
#define	NET_STA_TCP_CLOSE		6	// �s�b�o�̐ؒf�m�F�ς�
#define	NET_STA_PPP_CLOSING		7	// �o�o�o�ؒf��
#define	NET_STA_PPP_CLOSE		8	// �o�o�o�̐ؒf�m�F�ς�

extern	ulong	RAU_x_1mLifeTime;			/* timer counter. up to every 1ms */
#define		RAU_DECIMAL_32BIT	4294967					// 32BIT�̏����_���̌v�Z�Ɏg���l (2^32 / 1000)
#define		RAU_TIMEZONE		32400;					// �^�C���]�[�� ���{ +9:00 9*60*60 = 32400�b

// �\�P�b�g�ʐM���
typedef enum {
	RAU_SOCK_INIT,											// ������
	RAU_SOCK_OPEN,											// �\�P�b�g�I�[�v��
	RAU_SOCK_CONNECTING,									// TCP�m����
	RAU_SOCK_CONNECTED,										// TCP�m��
	RAU_SOCK_LISTENING,										// TCP�ڑ��҂�
	RAU_SOCK_IDLE,											// TCP�ڑ���
	RAU_SOCK_CLOSING,										// TCP�ؒf��
	RAU_SOCK_CLOSED,										// TCP�ؒf
} RAU_SOCK_CONDITION;

typedef enum {
	RAU_LINE_NO_ERROR,										// ����
	RAU_LINE_SEND,											// �f�[�^���M���s
	RAU_LINE_ERROR,											// �G���[����
} RAU_LINE_STATE;

typedef enum {
	RAU_NET_IDLE,											// ���ڑ�
	RAU_NET_PPP_OPENED,										// PPP�m����
	RAU_NET_TCP_CONNECTIED,									// TCP�m����
} RAU_NET_STATE;

// �r�b�g�Q�Ɨp
union	rau_bits_reg {
	struct	rau_bit_tag {
		uchar	bit_7	: 1 ;
		uchar	bit_6	: 1 ;
		uchar	bit_5	: 1 ;
		uchar	bit_4	: 1 ;
		uchar	bit_3	: 1 ;
		uchar	bit_2	: 1 ;
		uchar	bit_1	: 1 ;
		uchar	bit_0	: 1 ;
	} bits;
	uchar	byte;
};
typedef union rau_bits_reg	RAU_BITS;

union _BF2
{
	struct {
		uchar	reserve		:5;
		uchar	credit		:1;
		uchar	upload		:1;
		uchar	download	:1;
	} port;
	uchar	both;
};
typedef	union _BF2	BF2;

// RAU�^�X�N��M�o�b�t�@�i��OPE�w�j
typedef struct {
	uchar	ntDataBuf[RAU_NTDATABUF_COUNT][RAU_BLKSIZE];	// NT-NET�f�[�^�o�b�t�@
	ushort	readIndex;										// �Ǎ��݃C���f�b�N�X
	ushort	writeIndex;										// �����݃C���f�b�N�X
	ushort	count;											// �i�[�f�[�^��
} RauNtDataBuf;

extern	RauNtDataBuf	RauOpeSendNtDataBuf;				// RAU�^�X�N���M�o�b�t�@�i��OPE�w�j
extern	RauNtDataBuf	RauOpeRecvNtDataBuf;				// RAU�^�X�N��M�o�b�t�@�i��OPE�w�j

// RAU�^�X�N����e�[�u��
typedef struct {
	BF2		tcpConnectFlag;									// TCP�R�l�N�V������ԃt���O
	uchar	TCPcnct_req;									// TCP�R�l�N�V�������N�G�X�g�t���O
	uchar	upConnecting;									// ���R�l�N�V�����ڑ����t���O 0:���ڑ� 1:�m���� 2:�ڑ���
	uchar	DPA_TCP_DummySend;								// �_�~�[�p�P�b�g���M�t���O
	uchar	upNetState;										// ��������
	uchar	downNetState;									// ���������
	uchar	reserved1;
	ushort	connRetryCount;									// �R�l�N�V�������g���C�J�E���^
	T_RAU_BLKDATA	mainRecvData;							// ���ݏ������̃��C������̎�M�f�[�^
} tRauCtrl;
extern	tRauCtrl RauCtrl;

// RAU�ݒ�e�[�u��
typedef struct  {
	char		id22_23_tel_flg;		/* �h�c�������M�t���O  �i���Z�f�[�^�j*/
	char		id30_41_tel_flg;		/* �h�c�������M�t���O  �i�W�v�f�[�^�j*/
	char		id121_tel_flg;			/* �h�c�������M�t���O  �i�A���[���f�[�^�j*/
	char		id123_tel_flg;			/* �h�c�������M�t���O  �i���샂�j�^�[�f�[�^�j*/
	char		id120_tel_flg;			/* �h�c�������M�t���O  �i�G���[�f�[�^�j*/
	char		id20_21_tel_flg;		/* �h�c�������M�t���O  �i���ɏo�Ƀf�[�^�j*/
	char		id122_tel_flg;			/* �h�c�������M�t���O  �i���j�^�[�f�[�^�j*/
	char		id131_tel_flg;			/* �h�c�������M�t���O  �i�R�C�����ɏW�v�f�[�^�j*/
	char		id133_tel_flg;			/* �h�c�������M�t���O  �i�������ɏW�v�f�[�^�j*/
	char		id236_tel_flg;			/* �h�c�������M�t���O  �i���ԑ䐔�f�[�^�j*/
	char		id237_tel_flg;			/* �h�c�������M�t���O  �i���䐔�E���ԃf�[�^�j*/
	char		id126_tel_flg;			/* �h�c�������M�t���O  �i���K�Ǘ��f�[�^�j*/
	char		id135_tel_flg;			/* �h�c�������M�t���O  �i�ޑK�Ǘ��W�v�f�[�^�j*/
	char		id125_tel_flg;			/* �h�c�������M�t���O  �i���u�Ď��f�[�^�j*/
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	char		id61_tel_flg;			/* �h�c�������M�t���O  �i�������ԏ��f�[�^�j*/
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	char		id22_23_mask_flg;		/* ���M�}�X�N�ݒ�  �i���Z�f�[�^�j*/
	char		id30_41_mask_flg;		/* ���M�}�X�N�ݒ�  �i�W�v�f�[�^�j*/
	char		id121_mask_flg;			/* ���M�}�X�N�ݒ�  �i�A���[���f�[�^�j*/
	char		id123_mask_flg;			/* ���M�}�X�N�ݒ�  �i���샂�j�^�[�f�[�^�j*/
	char		id120_mask_flg;			/* ���M�}�X�N�ݒ�  �i�G���[�f�[�^�j*/
	char		id20_21_mask_flg;		/* ���M�}�X�N�ݒ�  �i���ɏo�Ƀf�[�^�j*/
	char		id122_mask_flg;			/* ���M�}�X�N�ݒ�  �i���j�^�[�f�[�^�j*/
	char		id131_mask_flg;			/* ���M�}�X�N�ݒ�  �i�R�C�����ɏW�v�f�[�^�j*/
	char		id133_mask_flg;			/* ���M�}�X�N�ݒ�  �i�������ɏW�v�f�[�^�j*/
	char		id236_mask_flg;			/* ���M�}�X�N�ݒ�  �i���ԑ䐔�f�[�^�j*/
	char		id237_mask_flg;			/* ���M�}�X�N�ݒ�  �i���䐔�E���ԃf�[�^�j*/
	char		id126_mask_flg;			/* ���M�}�X�N�ݒ�  �i���K�Ǘ��f�[�^�j*/
	char		id135_mask_flg;			/* ���M�}�X�N�ݒ�  �i�ޑK�Ǘ��W�v�f�[�^�j*/
	char		id125_mask_flg;			/* ���M�}�X�N�ݒ�  �i���u�Ď��f�[�^�j*/
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	char		id61_mask_flg;			/* ���M�}�X�N�ݒ�  �i�������ԏ��f�[�^�j*/
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	char		alarm_send_level;		/* �A���[���f�[�^�̑��M���x��(34-0037)		*/
	char		error_send_level;		/* �G���[�f�[�^�̑��M���x��(34-0037)		*/
	char		opemonitor_send_level;	/* ���샂�j�^�[�f�[�^�̑��M���x��(34-0037)	*/
	char		monitor_send_level;		/* ���j�^�[�f�[�^�̑��M���x��(34-0037)		*/
	char		rmon_send_level;		/* ���u�Ď��f�[�^�̑��M���x��				*/
	char		serverTypeSendTsum;		/* �T�[�o�[�^�C�v���̃f�[�^���M 34-0026�C	*/
	char		deta_full_proc;			/* �f�[�^�������@ */
	ushort		IBW_RespWtime;			/* ���M���ʎ�M�����҂����ԁi�P�ʁ��b�j*/
	uchar		system_select;			/* �V�X�e���I�� */
	uchar		modem_exist;			/* ���f���̗L�� */
	uchar		Dpa_proc_knd;			/* Dopa �����敪 [0]=�펞�ʐM�Ȃ�, [1]=�펞�ʐM���� */
	ulong		Dpa_IP_h;				/* Dopa HOST��IP�A�h���X */
	ushort		Dpa_port_h;				/* Dopa HOST�ǃ|�[�g�ԍ� */
	ulong		Dpa_IP_m;				/* Dopa ����IP�A�h���X */
	ushort		Dpa_port_m;				/* Dopa ���ǃ|�[�g�ԍ� */
	ushort		Dpa_nosnd_tm;			/* Dopa ���ʐM�^�C�}�[(�b) */
	ushort		Dpa_cnct_rty_tm;		/* Dopa �Ĕ��đ҂��^�C�}�[(�b) */
	ushort		Dpa_com_wait_tm;		/* Dopa �R�}���h(ACK)�҂��^�C�}�[(�b) */
	ushort		Dpa_data_rty_tm;		/* Dopa �f�[�^�đ��҂��^�C�}�[(��) */
	ushort		Dpa_discnct_tm;			/* Dopa TCP�R�l�N�V�����ؒf�҂��^�C�}�[(�b) */
	ushort		Dpa_cnct_rty_cn;		/* Dopa �Ĕ��ĉ� */
	uchar		Dpa_data_rty_cn;		/* Dopa �f�[�^�đ���(NAK,������) */
	uchar		Dpa_data_code;			/* Dopa HOST���d���R�[�h [0]=�����R�[�h, [1]=�o�C�i���R�[�h */
	ushort		Dpa_com_wait_reconnect_tm;		/* Dopa �Đڑ��҂��^�C�}�[(�b) */
	uchar		Dpa_data_rty_cn_disconnected;		/* Dopa �f�[�^�đ���(TCP�ؒf) */

	uchar		tcp_disconect_flag;		// TCP�ؒf����^�Ȃ� 0=���� 1=�Ȃ�
	uchar		interrupt_proc;			/* ���f�v�����̏��� */
	ushort		interrupt_proc_block;	/* ���f�v����M���̔���u���b�N */

	uchar		Dpa_data_snd_rty_cnt;	// �c���o���������p�P�b�g���M���g���C��(������)
	ushort		Dpa_ack_wait_tm;		// �c���o���������p�P�b�g�����҂�����(�`�b�j/�m�`�j)
	uchar		Dpa_data_rcv_rty_cnt;	// �c���o���������p�P�b�g��M���g���C��(�m�`�j)
	ushort		Dpa_port_watchdog_tm;	// �c���o���������ʐM�Ď��^�C�}(�b)
	ushort		Dpa_port_m2;			// �c���o�������� ���ǃ|�[�g�ԍ�
	ulong		Dpa_IP_Cre;				/* �N���W�b�g�Z���^�[IP�A�h���X */
	ushort		Dpa_port_Cre;			/* �N���W�b�g�Z���^�[�|�[�g�ԍ� */
	ushort		Dpa_port_m3;			/* �N���W�b�g�p���ǃ|�[�g�ԍ� */
	uchar		Credit_snd_rty_cnt;		// �N���W�b�g����p�P�b�g���M���g���C��(������)
	ushort		Credit_ack_wait_tm;		// �N���W�b�g����p�P�b�g�����҂�����(�`�b�j/�m�`�j)
	uchar		Credit_nak_rty_cnt;		// �N���W�b�g����p�P�b�g��M���g���C��(�m�`�j)
	ushort		Credit_discnct_tm;		// TCP�R�l�N�V�����ؒf�҂��^�C�}�[(�b) 
	uchar		Credit_cnct_rty_cnt;	// �R�l�N�V�������g���C��
	uchar		Credit_snd_rty_cnt_disconnected;		// �N���W�b�g����p�P�b�g���M���g���C��(TCP�ؒf)
	ushort		Credit_ack_wait_reconnect_tm;	// �N���W�b�g����p�P�b�g�����҂����ؒf�̍Đڑ�����
} tRauConfig;

extern	tRauConfig	RauConfig;


typedef struct {
	ushort		Len;
	uchar		Dat[RAU_DATA_MAX];
} t_DPA_DATA;

typedef struct {						// ����M�L���[
	uchar		Count;
	uchar		ReadIdx;
	uchar		WriteIdx;
	uchar		dummy;
	t_DPA_DATA	dpa_data[4];
} t_DPA_Ctrl;

typedef struct {						// ����M�L���[
	uchar		Count;
	uchar		ReadIdx;
	uchar		WriteIdx;
	uchar		dummy;
	uchar		dpa_data[4][30];
} t_DPA2_Ctrl;

extern	t_DPA_Ctrl	DPA2_Snd_Ctrl;		// �������̑��M�L���[
extern	t_DPA2_Ctrl	DPA2_Rcv_Ctrl;		// ������̎�M�L���[

extern	t_DPA_Ctrl	DPA_Snd_Ctrl;		// ������̑��M�L���[
extern	t_DPA_Ctrl	DPA_Rcv_Ctrl;		// �������̎�M�L���[


extern	uchar	RAU_SetSendNtData(const uchar* pData, ushort size);
extern	uchar*	RAU_GetSendNtData(void);
extern	uchar	RAU_SetSendTransactionData(const uchar* pData, ushort size);
extern	uchar*	RAU_GetSendTransactionData(void);

extern	void	RAU_err_chk(char code, char kind, char f_data, char err_ctl, void *pData);

extern	void	RAU_SockMain(void);
extern	void	RAU_SockInit(void);
extern	uchar	RAU_ReceiveUpData(uchar* pBuffer, ushort* pSize);
extern	uchar	RAU_ReceiveDownData(uchar* pBuffer, ushort* pSize);
extern	RAU_NET_STATE	RAU_GetUpNetState(void);
extern	RAU_NET_STATE	RAU_GetDownNetState(void);
extern	void	SetUpNetState(uchar state);
extern	uchar	GetUpNetState(void);
extern	void	SetDownNetState(uchar state);
extern	uchar	GetDownNetState(void);
extern	void	RAU_SetAntennaLevelState(uchar state);
extern	uchar	RAU_GetAntennaLevelState(void);

extern	void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );
extern	void	RAU_Word2Byte(unsigned char *data, ushort uShort);
extern	ushort	RAU_Byte2Word(unsigned char *data);
extern	ulong	RAU_Byte2Long(unsigned char *data);

extern	uchar	RAUhost_GetSndSeqFlag(void);
extern	uchar	RAUhost_GetRcvSeqFlag(void);
extern	BOOL	RAUdata_CanSendTableData(short elogID);
extern	void	RAU_RequestCheckNearfull(void);
extern	ulong	RAU_GetCenterSeqNo(RAU_SEQNO_TYPE type);
extern	void	RAU_UpdateCenterSeqNo(RAU_SEQNO_TYPE type);
extern	void	RAU_SNTPStart( uchar onoff ,ushort msg, ulong now_sec, ushort now_msec );
extern	void	RAU_SNTPGetPacket( void *voidPtr );

// MH322915(S) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
extern	void	RAU_Cancel_RetryTimer();
// MH322915(E) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��

										// ���M�V�[�P���X�t���O�̃Z�b�g
extern	void	RAUhost_SetSndSeqFlag(uchar ucSndSeqFlag);
										// ��M�V�[�P���X�t���O�̃Z�b�g
extern	void	RAUhost_SetRcvSeqFlag(uchar ucRcvSeqFlag);
										// ���M�v������������
extern	void	RAUhost_ClearSendRequeat(void);
										// �f�[�^�e�[�u���̃T�C�Y�ɂ��擪�|�C���^�����߂�
extern	void	RAU_data_table_init( void );
										// T-sum data
extern	int		init_tsum(char *buff, long size);
extern	int		init_center_term_info(char *buff);
extern	void	RAU_TimerInit( void );	// �S�^�C�}��~�i�����オ�莞��Call�����j
extern	void	RAUid_Init(void);		// ������
										// TCP Connection �v�����s���̃N���A����
extern	void	RAUhost_TcpConnReq_Clear ( void );
										// �������ʐM�Ď��^�C�}�[���^�C���A�E�g�������̏���
extern	void	RAUhost_DwonLine_common_timeout(void);
										// �W�v�f�[�^�̍쐬�Atsum�i�[(�t�H�[�}�b�gRevNo.10�p)
extern	void	RAUhost_CreateTsumData_r10(ushort type);
										// �W�v�f�[�^�̍쐬�Atsum�i�[
extern	void	RAUhost_CreateTsumData(void);
										// �e�[�u���f�[�^�̃j�A�t���ʒm���s��
extern	void	RAUdata_CheckNearFullLogData(void);
extern	eRAU_TABLEDATA_SET	RAUid_SetNtBlockData(t_RemoteNtNetBlk *pBlockData, ushort size);
										// �z�X�g�����M����NT-NET�f�[�^�̃u���b�N�����擾
extern	int		RAUdata_GetRcvNtDataBlkNum(void);
										// �z�X�g�����M����NT-NET�f�[�^���擾
extern	ushort	RAUdata_GetRcvNtBlkData(uchar *data, uchar blkno);
										// �z�X�g�����M����NT-NET�f�[�^���폜
extern	void	RAUdata_DelRcvNtData(void);
										// Timer start value get. (return 1ms counter)
extern	ulong	RAU_c_1mTim_Start( void );
										// Dopa command read from receive queue.
extern	uchar	RAU_DpaRcvQue_Read(uchar *pData, ushort *len, int port);
										// Dopa command set to send queue.
extern	void	DpaSndQue_Set(uchar *pData, ushort len, int port);
extern	void	reset_tsum(char *buff, long size);
extern	int		write_tsum(char *ntdata, int length);
										// ���Ѵװ����/�����֐�
extern	void	RauSysErrSet( unsigned char n , unsigned char s );
extern	void	RAU_StopDownline(void);

extern	uchar	RAU_Credit_Enabale;				// �N���W�b�g�g�p�ݒ�
extern	uchar	Ntnet_Remote_Comm;
#define	_is_credit_only()	(Ntnet_Remote_Comm != 2 && RAU_Credit_Enabale != 0)
extern	uchar	Cre_SetSendNtData(const uchar* pData, ushort size);
extern	uchar*	Cre_GetSendNtData(void);
extern	uchar*	Cre_GetRcvNtData(void);
extern	eRAU_TABLEDATA_SET	Credit_SetSendNtBlockData(t_RemoteNtNetBlk *pBlockData, ushort size);
extern	eRAU_TABLEDATA_SET	Credit_SetRcvNtData(uchar *pData, ushort uiDataLen, ushort uiTcpBlkNo, uchar ucTcpBlkSts);
extern	eRAU_TABLEDATA_SET	Credit_SetThroughData(uchar *pData, ushort wDataLen, uchar ucDirection);
extern	void	Credit_DelRcvNtData(void);
extern	int		Credit_GetRcvNtDataBlkNum(void);
extern	ushort	Credit_GetRcvNtBlkData(uchar *data, uchar blkno);
extern	RAU_NET_STATE	RAU_GetCreditNetState(void);
extern	void	Credit_SetSeqFlag(uchar ucRcvSeqFlag);
extern	uchar	Credit_GetSeqFlag(void);

typedef struct {
	uchar	TCPcnct_req;									// TCP�R�l�N�V�������N�G�X�g�t���O
	uchar	CenterConnecting;								// �ڑ����t���O 0:���ڑ� 1:�m���� 2:�ڑ���(���g�p)
	uchar	NetState;										// ������
	uchar	TCPdiscnct_req;									// TCP�ؒf�v���t���O
	ushort	connRetryCount;									// �R�l�N�V�������g���C�J�E���^
//	T_RAU_BLKDATA	mainRecvData;							// ���ݏ������̃��C������̎�M�f�[�^
} tCreditCtrl;
extern	tCreditCtrl CreditCtrl;

// �N���W�b�g�p��M�o�b�t�@�i��OPE�w�j
#define CREDIT_DATABUF_COUNT	3
typedef struct {
	uchar	ntDataBuf[CREDIT_DATABUF_COUNT][RAU_BLKSIZE];	// NT-NET�f�[�^�o�b�t�@
	ushort	readIndex;										// �Ǎ��݃C���f�b�N�X
	ushort	writeIndex;										// �����݃C���f�b�N�X
	ushort	count;											// �i�[�f�[�^��
} CreNtDataBuf;

extern	CreNtDataBuf	CreOpeSendNtDataBuf;				// RAU�^�X�N���M�o�b�t�@�i��OPE�w�j
extern	CreNtDataBuf	RecvCreditDt;							/* NE-NET��M�ޯ̧(ܰ�)			*/

#endif	// RAUDEF_H
