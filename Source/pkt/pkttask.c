// MH810100(S) K.Onodera  2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"message.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"aes_sub.h"
#include	"pkt_def.h"
#include	"pkt_com.h"
#include	"pkt_buffer.h"
#include	"pktctrl.h"
#include	"lcdbm_ctrl.h"
#include	"trsocket.h"
#include	"trsntp.h"
// MH810105(S) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
#include	"trmacro.h"
#include	"trproto.h"
#include	"trglobal.h"
// MH810105(E) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��


// ------------------------------------- //
// Declaration
// ------------------------------------- //
int		PKT_Socket = -1;							// �\�P�b�g
// MH810105(S) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
int		SocketOpenErrorCount = 0;					// �\�P�b�g�I�[�v���G���[�̃J�E���^�[
// MH810105(E) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��

// ------------------------------------- //
// Definition
// ------------------------------------- //
#define TIMEOUT_CONNECT				TIMEOUT1		// �ڑ��R�}���h�J�n�^�C�}�^�C���A�E�g�C�x���g
#define TIMEOUT_CON_RES_WT			TIMEOUT2		// �R�l�N�V���������҂�����
#define TIMEOUT_CON_INTVL			TIMEOUT3		// �R�l�N�V�������M�Ԋu�^�C���A�E�g�C�x���g
#define TIMEOUT_CON_DIS_WT			TIMEOUT4		// �R�l�N�V�����ؒf�҂����ԃ^�C���A�E�g�C�x���g
#define TIMEOUT_RES_WT				TIMEOUT5		// �p�P�b�g�����҂����ԃ^�C���A�E�g�C�x���g
#define TIMEOUT_KEEPALIVE			TIMEOUT6		// �L�[�v�A���C�u�p�^�C���A�E�g�C�x���g
#define TIMEOUT_RTPAY_RES			TIMEOUT7		// ���Z�����f�[�^�҂��^�C�}�^�C���A�E�g�C�x���g
#define TIMEOUT_DC_QR_RES			TIMEOUT8		// QR�m��E��������f�[�^�҂��^�C�}�^�C���A�E�g�C�x���g
#define TIMEOUT_RTPAY_RESND			TIMEOUT9		// �đ��E�F�C�g�^�C�}�^�C���A�E�g�C�x���g
#define TIMEOUT_DC_QR_RESND			TIMEOUT10		// �đ��E�F�C�g�^�C�}�^�C���A�E�g�C�x���g
#define TIMEOUT_DISCON_WT			TIMEOUT11		// �R�l�N�V�����ؒf���o�҂��^�C�}�^�C���A�E�g�C�x���g
#define TIMEOUT_ERR_GUARD			TIMEOUT12		// �ڑ��G���[�K�[�h�^�C�}�^�C���A�E�g�C�x���g
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
#define TIMEOUT_DC_LANE_RES			TIMEOUT13		// ���[�����j�^�����f�[�^�҂��^�C�}�^�C���A�E�g�C�x���g
#define TIMEOUT_DC_LANE_RESND		TIMEOUT14		// �đ��E�F�C�g�^�C�}�^�C���A�E�g�C�x���g
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
#define TIMEOUT_RTRECEIPT_RES		TIMEOUT15		// �̎��؃f�[�^�����҂��^�C�}�^�C���A�E�g�C�x���g
#define TIMEOUT_RTRECEIPT_RESND		TIMEOUT16		// �đ��E�F�C�g�^�C�}�^�C���A�E�g�C�x���g
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

#define RESET_KEEPALIVE_COUNT		pktctrl.keepalive_timer = 0

#define RES_ACK						0x06
#define RES_NAK						0x15

#define PKT_RES_SIZE				28				// ACK/NAK�f�[�^�T�C�Y

typedef enum{
	TARGET_NORMAL_DATA,
	TARGET_RT_PAY,
	TARGET_DC_QR,
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	TARGET_DC_LANE,
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	TARGET_RT_RECEIPT,
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	TARGET_MAX
} eSEND_TARGET;

enum{
	LOG_PHASE_IDLE,
	LOG_PHASE_WAIT_ACK,			// 1:���O����(ACK/NAK)�҂�
	LOG_PHASE_RESEND_CANCEL,	// 2:���O�f�[�^�đ����f��
	LOG_PHASE_WAIT_RES_CMD,		// 3:���O�����f�[�^�҂�
};

// ------------------------------------------------------------------------------------------------
// PKT Ctrl
static struct{
	int		ready;
	char	buf_type;			// S:���M�o�b�t�@�CB:�đ��o�b�t�@(�x�[�X)�CA:�đ��o�b�t�@(�ǉ�)

	struct{
		ushort		flag;		// ��M���t���O
		ushort		usNackRetryNum;
		PKT_RCVRES	buff;
	}recv;

	struct{
		ushort		commited;	// �R�}���h�o�b�t�@�[�ɂ���f�[�^��
		ushort		sent;		// �\�P�b�g�̑��M�ς݃o�b�t�@�[��
		PKT_SNDCMD	buff;
		uchar		res_wait;	// �����҂�
		ushort		wait_seqno;	// �����҂���SEQ
		uchar		rtry_cnt;	// ���M���g���C��
		uchar		RT_LogSend;	// RT���O�f�[�^���M�t���O
		uchar		QR_LogSend;	// QR���O�f�[�^���M�t���O
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		uchar		LANE_LogSend;	// ���[�����j�^�f�[�^���M�t���O
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		uchar		RECEIPT_LogSend;	// RT�̎��؃��O�f�[�^���M�t���O
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	}send;

	ushort	recv_port;			// ���ڂɃ\�P�b�gID�ŕۑ�
	uchar	open_mode;			// PKT_SERVER:�T�[�o�[���[�h�CPKT_CLIENT:�N���C�A���g���[�h
	uchar	start_up;			// �N�������ʒm��M�t���O(0:RT���Z�f�[�^���M�s��/1:���M��)
	uchar	reset_req;			// ���Z�b�g�v���t���O(0:���Z�b�g�v���s�v/1:�v)
	short	keepalive_timer;
	uchar	f_wait_discon;		// ���Z�b�g�ʒm��M�|�R�l�N�V�����ؒf���o�҂��^�C�}�N�����t���O

	uchar	sckt_sts;			// �\�P�b�g�|�[�g���(�ʐM������̃X�e�[�^�X�ʒm�ōX�V)

	ushort	snd_seqno;			// ���M�V�[�P���X��
	ushort	rcv_seqno;			// ��M�V�[�P���X��

	ulong	ipa;				// �ݒ臂48-0001�`0004�FLCD�p�l���h�o�A�h���X�@�`�C
	ushort	port;				// �ݒ臂48-0005�FLCD�|�[�g�ԍ�
	uchar	mac[6];				// �ʐM��MAC�A�h���X

	ulong	ipa_m;				// �ݒ臂41-0002�`0003�F����IP�A�h���X
	ushort	port_m;				// �ݒ臂48-0007�F���ǃ|�[�g

	uchar	err_cnt;			// �ݒ臂48-0010�F����G���[�����
	uchar	con_res_tm;			// �ݒ臂48-0011�F�R�l�N�V���������҂�����(�b)
	ushort	con_snd_intvl;		// �ݒ臂48-0012�F�R�l�N�V�������M�Ԋu(�b)
	uchar	con_dis_wt_tm;		// �ݒ臂48-0013�F�R�l�N�V�����ؒf�҂�����(�b)
	uchar	rsnd_rtry_ov;		// �ݒ臂48-0014�F���g���C�I�[�o�[���̍đ��M
	uchar	snd_rtry_cnt;		// �ݒ臂48-0015�F�p�P�b�g���M���g���C��(������)
	uchar	res_wt_tm;			// �ݒ臂48-0016�F�p�P�b�g�����҂�����(ACK/NAK)(�b)
	uchar	rcv_rtry_cnt;		// �ݒ臂48-0017�F�p�P�b�g��M���g���C��(NAK)
	ushort	kpalv_snd_intvl;	// �ݒ臂48-0019�F�L�[�v�A���C�u���M�Ԋu(�b)
	uchar	kpalv_rtry_cnt;		// �ݒ臂48-0020�F�L�[�v�A���C�u���g���C��
	uchar	RTPay_res_tm;		// �ݒ臂48-0023�F���Z�����f�[�^�҂��^�C�}(�b)
	uchar	RTPay_resnd_tm;		// �ݒ臂48-0024�F�đ��E�F�C�g�^�C�}(�b)
	ushort	discon_wait_tm;		// �ݒ臂48-0025�F�R�l�N�V�����ؒf���o�҂��^�C�}(�b)
	ushort	err_guard_tm;		// �ݒ臂48-0027�F�ڑ��G���[�K�[�h�^�C�}(�b)
	uchar	EncDecrypt;			// �ݒ臂48-0040�F�Í����ʐM(0:�Í����Ȃ��C1:�Í�������)

} pktctrl;

typedef	union {
	struct {
		uchar	buff[16];
		ushort	wp;
	} string;
	struct {
		ulong	data_size;		// �p�P�b�g�f�[�^��(data_size�`crc16h)
		uchar	identifier[12];	// �w�b�_�[�R�[�h("AMANO_LCDPKT")
		ushort	dummy;
	} packet;
} RCV_DATA_ROLLER;

static PKT_TRAILER			res_trailer;	// ACK/NAK���M�p�o�b�t�@
static PKT_TRAILER			snd_trailer;	// �R�}���h���M�p���M�o�b�t�@
static PKT_TRAILER			rcv_trailer;	// �R�}���h��M�p�o�b�t�@
static lcdbm_cmd_pydt_0pd_t	log_pay_wk;		// RT���Z�f�[�^���O���o���pwork
static lcdbm_cmd_qrdt_0pd_t	log_qr_wk;		// QR�m�����f�[�^���O���o���pwork
static uchar EncDecrypt_buff[DECRYPT_COM];	// �Í���/�����pwork
static ulong				Pay_Data_ID;	// �����`�F�b�N�pRT���Z�f�[�^ID
static ulong				QR_Data_ID;		// �����`�F�b�N�pQR�m��E����f�[�^ID
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
static ulong				LANE_Data_ID;		// �����`�F�b�N�p���[�����j�^�f�[�^ID
static lcdbm_cmd_lanedt_0pd_t	log_lane_wk;	// ���[�����j�^�f�[�^���O���o���pwork
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
static lcdbm_cmd_receiptdt_0pd_t	log_receipt_wk;		// RT�̎��؃f�[�^���O���o���pwork
static ulong						Receipt_Data_ID;	// �����`�F�b�N�pRT�̎��؃f�[�^ID
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

static uchar	restore_unsent_flg = 1;		// �����f�[�^�����M�t���O

static short rcv_data_size;					// ���d���̎�M�T�C�Y(�p�P�b�g�ׂ��̏ꍇ���܂�)
static short prc_data_size_bf;				// �p�P�b�g���ׂ��ł���܂Ŏ�M�������d���̃T�C�Y(���p�P�b�g������)
static short prc_data_size_nw;				// 1�p�P�b�g���̉�͍ς݂̓d���T�C�Y
static RCV_DATA_ROLLER rcv_data_roller;
static uchar pkt_id_found_flg = 0;			// �p�P�b�g���ʎq�����t���O

static ushort PKTrcv_Receive_Packet( ushort rcv_seqno, PKT_TRAILER *p_trailer );
static void pkt_tenkey_status_check( uchar *tenkey );
static ulong pkt_ksg_err_getreg( int fid, char code, char knd );
static void pkt_can_all_com_err( void );
// MH810105(S) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
#if DEBUG_JOURNAL_PRINT
static void pkt_socket_debug_print( void );
#endif
// MH810105(E) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��


// ------------------------------------------------------------------------------------------------

#define	_ref_sckt		(pktctrl.sckt_sts)
#define	_ref_recv		(pktctrl.recv)
#define	_ref_send		(pktctrl.send)
#define	_ref_buf_type	(pktctrl.buf_type)

void PktSocCbk( int sock, int event );

//[]----------------------------------------------------------------------[]
///	@brief		�N���ʒm��ԍX�V
//[]----------------------------------------------------------------------[]
///	@param[in]	faze	0:�N���ʒm��M�s��(OPE�^�X�N������������/RT���Z�f�[�^���M�s��)
///						1:�N���ʒm��M��(OPE�^�X�N����������/RT���Z�f�[�^���M�s��)
///						2:�N�������ʒm��M�ς�(OPE�^�X�N����������/RT���Z�f�[�^���M��)
///	@return		none
/// @note		���Z�@�d��OFF/ON�ALCD�Ƃ̃R�l�N�V�����ؒf���ART���Z�f�[�^
///				���M�s��ԂƂ��ALCD����̋N�������ʒm��M�ő��M�Ƃ���
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PktStartUpStsChg( uchar faze )
{
	pktctrl.start_up = faze;
}

//[]----------------------------------------------------------------------[]
///	@brief		�N���ʒm��Ԏ擾
//[]----------------------------------------------------------------------[]
///	@param[in]	none	
///	@return		faze	0:�N���ʒm��M�s��(OPE�^�X�N������������/RT���Z�f�[�^���M�s��)
///						1:�N���ʒm��M��(OPE�^�X�N����������/RT���Z�f�[�^���M�s��)
///						2:�N�������ʒm��M�ς�(OPE�^�X�N����������/RT���Z�f�[�^���M��)
/// @note		�N��������Ԃ��擾����
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar PktStartUpStsGet( void )
{
	return pktctrl.start_up;
}

//[]----------------------------------------------------------------------[]
///	@brief		���Z�b�g�v���t���O�Z�b�g����
//[]----------------------------------------------------------------------[]
///	@param[in]	rst_req		0:���Z�b�g�v���s�v
///							1:���Z�b�g�v���v
///	@return		none
/// @note		LCD�ɑ΂��郊�Z�b�g�v���t���O���Z�b�g����
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PktResetReqFlgSet( uchar rst_req )
{
	pktctrl.reset_req = rst_req;
}

//[]----------------------------------------------------------------------[]
///	@brief		���Z�b�g�v���t���O�擾
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		reset_req	0:���Z�b�g�v���s�v
///							1:���Z�b�g�v���v
/// @note		LCD�ɑ΂��郊�Z�b�g�v���t���O���擾����
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PktResetReqFlgRead( void )
{
	return pktctrl.reset_req;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ݒ�擾
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/10/31<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void GetSetting( void )
{
	char	ip[20];					// IP�A�h���X�ϊ��p

	// �ڑ���(LCD)IP�A�h���X
// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:13,14,15,16)�Ή�
//	sprintf( ip, "%03d.%03d.%03d.%03d",
	sprintf( ip, "%03ld.%03ld.%03ld.%03ld",
// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:13,14,15,16)�Ή�
			prm_get( COM_PRM, S_PKT, 1, 3, 1 ),
			prm_get( COM_PRM, S_PKT, 2, 3, 1 ),
			prm_get( COM_PRM, S_PKT, 3, 3, 1 ),
			prm_get( COM_PRM, S_PKT, 4, 3, 1 )
	);
	pktctrl.ipa = htonl( inet_addr(ip) );

	// �ڑ���(LCD)�|�[�g
	pktctrl.port = (ushort)prm_get(COM_PRM, S_PKT, 5, 5, 1 );

	// ����IP�A�h���X
// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:17,18,19,20)�Ή�
//	sprintf( ip, "%03d.%03d.%03d.%03d",
	sprintf( ip, "%03ld.%03ld.%03ld.%03ld",
// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:17,18,19,20)�Ή�
			prm_get( COM_PRM, S_MDL, 2, 3, 4 ),
			prm_get( COM_PRM, S_MDL, 2, 3, 1 ),
			prm_get( COM_PRM, S_MDL, 3, 3, 4 ),
			prm_get( COM_PRM, S_MDL, 3, 3, 1 )
	);
	pktctrl.ipa_m = htonl( inet_addr(ip) );

	// ���ǃ|�[�g
	pktctrl.port_m = (ushort)prm_get( COM_PRM, S_PKT, 7, 5, 1 );

	// ����G���[�����
	pktctrl.err_cnt = (ushort)prm_get( COM_PRM, S_PKT, 10, 2, 1 );
	if( 99 < pktctrl.err_cnt ){
		pktctrl.err_cnt = 1;
	}

	// �R�l�N�V���������҂�����(�b)
	pktctrl.con_res_tm = (uchar)prm_get( COM_PRM, S_PKT, 11, 2, 1 );
	if( pktctrl.con_res_tm < 1 || 20 < pktctrl.con_res_tm ){
		pktctrl.con_res_tm = 2;
	}

	// �R�l�N�V�������M�Ԋu(�b)
	pktctrl.con_snd_intvl = (ushort)prm_get( COM_PRM, S_PKT, 12, 3, 1 );
// MH810100(S) K.Onodera  2020/03/25 #4093 �Ԕԃ`�P�b�g���X(�R�l�N�V�������M�Ԋu��0�b���w�肷��ƁALCD���N�����Ȃ��Ȃ�s��Ή�)
//	if( 999 < pktctrl.con_snd_intvl ){
	if( pktctrl.con_snd_intvl < 1 || 999 < pktctrl.con_snd_intvl ){
// MH810100(E) K.Onodera  2020/03/25 #4093 �Ԕԃ`�P�b�g���X(�R�l�N�V�������M�Ԋu��0�b���w�肷��ƁALCD���N�����Ȃ��Ȃ�s��Ή�)
		pktctrl.con_snd_intvl = 3;
	}

	// �R�l�N�V�����ؒf�҂�����(�b)
	pktctrl.con_dis_wt_tm = (uchar)prm_get( COM_PRM, S_PKT, 13, 2, 1 );
// MH810100(S) K.Onodera  2020/03/25 �Ԕԃ`�P�b�g���X(#4093�Ɠ��l�Ɂu0�v�͖����l�Ƃ���)
//	if( 99 < pktctrl.con_dis_wt_tm ){
	if( pktctrl.con_dis_wt_tm < 1 || 99 < pktctrl.con_dis_wt_tm ){
// MH810100(E) K.Onodera  2020/03/25 �Ԕԃ`�P�b�g���X(#4093�Ɠ��l�Ɂu0�v�͖����l�Ƃ���)
		pktctrl.con_dis_wt_tm = 3;
	}

	// ���g���C�I�[�o�[���̍đ��M
	pktctrl.rsnd_rtry_ov = (uchar)prm_get( COM_PRM, S_PKT, 14, 1, 1 );
	if( 1 < pktctrl.rsnd_rtry_ov ){
		pktctrl.rsnd_rtry_ov = 1;
	}

	// �p�P�b�g���M���g���C��
	pktctrl.snd_rtry_cnt = (uchar)prm_get( COM_PRM, S_PKT, 15, 2, 1 );
	if( 99 < pktctrl.snd_rtry_cnt ){
		pktctrl.snd_rtry_cnt = 1;
	}

	// �p�P�b�g�����҂�����(ACK/NAK)(�b)
	pktctrl.res_wt_tm = (uchar)prm_get( COM_PRM, S_PKT, 16, 2, 1 );
// MH810100(S) K.Onodera  2020/03/25 �Ԕԃ`�P�b�g���X(#4093�Ɠ��l�Ɂu0�v�͖����l�Ƃ���)
//	if( 99 < pktctrl.res_wt_tm ){
	if( pktctrl.res_wt_tm < 1 || 99 < pktctrl.res_wt_tm ){
// MH810100(E) K.Onodera  2020/03/25 �Ԕԃ`�P�b�g���X(#4093�Ɠ��l�Ɂu0�v�͖����l�Ƃ���)
		pktctrl.res_wt_tm = 3;
	}

	// �p�P�b�g��M���g���C��(NAK)
	pktctrl.rcv_rtry_cnt = (uchar)prm_get( COM_PRM, S_PKT, 17, 2, 1 );
	if( 99 < pktctrl.rcv_rtry_cnt ){
		pktctrl.rcv_rtry_cnt = 3;
	}

	// �L�[�v�A���C�u���M�Ԋu(�b)
	// ���ۂɂ�1�b�^�C�}��(�ݒ臂48-0019)��^�C���A�E�g�ő��M�Ԋu���B�Ƃ݂Ȃ�
	pktctrl.kpalv_snd_intvl = (ushort)prm_get( COM_PRM, S_PKT, 19, 3, 1 );
	if( 999 < pktctrl.kpalv_snd_intvl ){
		pktctrl.kpalv_snd_intvl = 10;
	}

	// �L�[�v�A���C�u���g���C��
	pktctrl.kpalv_rtry_cnt = (uchar)prm_get( COM_PRM, S_PKT, 20, 2, 1 );
	if( pktctrl.kpalv_rtry_cnt < 1 || 10 < pktctrl.kpalv_rtry_cnt ){
		pktctrl.kpalv_rtry_cnt = 3;
	}

	// ���Z�����f�[�^�҂��^�C�}(�b)
	pktctrl.RTPay_res_tm = (uchar)prm_get( COM_PRM, S_PKT, 23, 2, 1 );
	if( pktctrl.RTPay_res_tm < 1 || 99 < pktctrl.RTPay_res_tm ){
		pktctrl.RTPay_res_tm = 15;
	}

	// �đ��E�F�C�g�^�C�}(�b)
	pktctrl.RTPay_resnd_tm = (uchar)prm_get( COM_PRM, S_PKT, 24, 2, 1 );
	if( pktctrl.RTPay_resnd_tm < 1 || 99 < pktctrl.RTPay_resnd_tm ){
		pktctrl.RTPay_resnd_tm = 10;
	}

	// �R�l�N�V�����ؒf���o�҂��^�C�}(�b)
	pktctrl.discon_wait_tm = (ushort)prm_get( COM_PRM, S_PKT, 25, 3, 1 );
	if( pktctrl.discon_wait_tm < 1 || 999 < pktctrl.discon_wait_tm ){
		pktctrl.discon_wait_tm = 300;
	}

	// �ڑ��G���[�K�[�h�^�C�}(�b)
	pktctrl.err_guard_tm = (ushort)prm_get( COM_PRM, S_PKT, 27, 3, 1 );
	if( pktctrl.err_guard_tm < 1 || 999 < pktctrl.err_guard_tm ){
		pktctrl.err_guard_tm = 90;
	}

	// �Í����ʐM(0:�Í����Ȃ��C1:�Í�������)
	pktctrl.EncDecrypt = (uchar)prm_get( COM_PRM, S_PKT, 40, 1, 1 );
	//if( 1 < pktctrl.EncDecrypt ){
	//	pktctrl.EncDecrypt = 1;
	//}

}

//[]----------------------------------------------------------------------[]
///	@brief			�\�P�b�g�ݒ�
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int PktSocCfg( void )
{
	int		sock = -1;				// �\�P�b�g�f�B�X�N���v�^
	int		on = 1;					// �I�v�V�����L��
	int		maxrt;
	struct sockaddr_in	myAddr;		// ���ǃA�h���X�E�|�[�g�ԍ�

	// �\�P�b�g�쐬
	sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( sock == -1 ){
// MH810105(S) R.Endo 2021/12/21 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
// 		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_SOC, 2 );
		if ( SocketOpenErrorCount < 50 ) {
			// 50��܂ł̓G���[�o�^
			pkt_ksg_err_getreg(sock, ERR_TKLSLCD_KSG_SOC, 2);
#if DEBUG_JOURNAL_PRINT
			// 10�񖈂Ƀf�o�b�O��
			if ( (SocketOpenErrorCount % 10) == 0 ) {
				pkt_socket_debug_print();
			}
#endif
		} else {
			// 50��𒴂�����ҋ@���x�Ƃ���������Ԃł���΃��Z�b�g
			if ( (OPECTL.Mnt_mod == 0) && ((OPECTL.Ope_mod == 0) || (OPECTL.Ope_mod == 100) || (OPECTL.Ope_mod == 255)) ) {
				System_reset();	// Main CPU reset (It will not retrun from this function)
			}
		}
		SocketOpenErrorCount++;
// MH810105(E) R.Endo 2021/12/21 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
		return sock;
	}
// MH810105(S) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
	SocketOpenErrorCount = 0;
// MH810105(E) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��

	// �m���u���b�L���O
	if( -1 == tfBlockingState( sock, TM_BLOCKING_OFF ) ){
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_BLK_ST, 2 );
		if( -1 == tfClose(sock) ){
			pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_CLS, 2 );
		}
		return -1;
	}

	// TIME_WAIT�ł��|�[�g�̃I�[�v�����\�ɂ���
	if( -1 == setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) ){
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_SOC_OP, 2 );
		if( -1 == tfClose(sock) ){
			pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_CLS, 2 );
		}
		return -1;
	}

	// connect�̃^�C���A�E�g���Ԃ�ύX����
	maxrt = PKT_CONN_TIMEOUT;
	if( -1 == setsockopt( sock, IP_PROTOTCP, TCP_MAXRT, (char*)&maxrt, sizeof(maxrt) ) ){
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_SOC_OP, 2 );
		if( -1 == tfClose( sock ) ){
			pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_CLS, 2 );
		}
		return -1;
	}

	// �\�P�b�g�C�x���g�R�[���o�b�N�֐��̓o�^
	if( -1 == tfRegisterSocketCB( sock, PktSocCbk,
						TM_CB_RECV | TM_CB_RECV_OOB |								// �f�[�^��M
						TM_CB_REMOTE_CLOSE | TM_CB_SOCKET_ERROR |					// ���肩��̐ؒf�A�G���[
						TM_CB_RESET | TM_CB_CLOSE_COMPLT | TM_CB_CONNECT_COMPLT )){	// ���肩��̃��Z�b�g�A�N���[�Y�����A�ڑ�����
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_SOC_CB, 2 );
	}

	// ���ǃ|�[�g�����蓖�Ă�
	memset( &myAddr, 0x00, sizeof(myAddr) );
	myAddr.sin_family      = AF_INET;
	myAddr.sin_port        = pktctrl.port_m;	// ���ǃ|�[�g�ԍ�
	myAddr.sin_addr.s_addr = pktctrl.ipa_m;		// ����IP�A�h���X

	if( -1 == bind( sock, (struct sockaddr *)&myAddr, sizeof(myAddr) ) ){
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_BND, 2 );
		if( -1 == tfClose( sock ) ){
			pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_CLS, 2 );
		}
		sock = -1;
	}

	return sock;
}

//[]----------------------------------------------------------------------[]
///	@brief			TCP���M����
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
int PktTcpSend( uchar *data, ulong len )
{
	int iRet;

	iRet = send( PKT_Socket, (char*)data, len, 0 );
	// ���M���s�H
	if( iRet <= 0 ){
		pkt_ksg_err_getreg( PKT_Socket, ERR_TKLSLCD_KSG_SND, 2 );
	}
	taskchg( IDLETSKNO );

	return iRet;
}

//[]----------------------------------------------------------------------[]
///	@brief			TCP��M����
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PktTcpRecv( int fid )
{
	int er;
	ushort i;
	ushort iDiff;
	int		sockError;
	ushort	err_cnt = 0;
	uchar	rst_cause;

	if( !_ref_recv.flag ){	// ��M�p�P�b�g�������ł͂Ȃ�
		rcv_data_size = 0;
		prc_data_size_bf = 0;
		prc_data_size_nw = 0;
		memset(&rcv_trailer, 0x00, sizeof(rcv_trailer));	// ��M�d���i�[�̈�N���A
		pkt_id_found_flg = 0;			// �p�P�b�g���ʎq�����t���O
	}

	while(1) {
		er = recv(fid, (char*)&_ref_recv.buff.rcv.rcv_data, PKT_DATA_MAX, 0);
		if( er > 0 ){
			// ��M�f�[�^����
			if( !rcv_data_size ){	// �����M����
				if( er < 28 ){	// �w�b�_�T�C�Y(26Byte)�{CRC16������M�f�[�^�����Z��
					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_LENGTH, 2, 0, 0, NULL );
					return;	// ��M�f�[�^�j��
				}
			}
			_ref_recv.flag = 1;

			_ref_recv.buff.com.kind = PKT_RCV_NOTIFY;
			_ref_recv.buff.rcv.data_size = er;
			pktctrl.recv_port = (ushort)fid;				// fid��ۑ�

			_ref_sckt = PKT_PORT_ESTABLISH;
			err_cnt = 0;
		}
		else{
			sockError = (int)pkt_ksg_err_getreg( fid, ERR_TKLSLCD_KSG_RCV, 2 );
			if( sockError == TM_EWOULDBLOCK ){
				// �m���u���b�L���O���[�h(�G���[�łȂ�)
				if( _ref_recv.flag == 1 ){	// ��M��
					if( ++err_cnt < 10 ){
						taskchg( IDLETSKNO );
						continue;
					}
				}
			}
			else{
				// �G���[����(TCP�ؒf)
				rst_cause = ERR_TKLSLCD_KSG_RCV;
				queset( PKTTCBNO, PKT_RST_PRT, 1, &rst_cause );
			}
			return;
		}

		// �p�P�b�g��M����
		if (_ref_recv.buff.rcv.data_size) {
			if ((PKT_RCV_NOTIFY == _ref_recv.buff.com.kind)) {
				if( !rcv_data_size ){	// �����M����
					// �w�b�_���i�[�G���A�N���A
					memset(&rcv_data_roller, 0x00, sizeof(rcv_data_roller));
				}	// �������ꂽ�㑱�f�[�^��M���̓w�b�_��񂪊i�[�ς݂Ȃ̂ŃN���A���Ȃ�
				iDiff = 0;
				i = 0;
				while(1)
				{
					if( pkt_id_found_flg == 0){
						// iDiff�́A0�͎�M�����ŏ��B����ȊO�͍����M�����c��̉��
						for (i=iDiff; i<_ref_recv.buff.rcv.data_size; i++) {	// �����M�T�C�Y�����[�v
							WACDOG;	// �E�H�b�`�h�b�N���Z�b�g���s
							rcv_data_roller.string.buff[rcv_data_roller.string.wp++] = _ref_recv.buff.rcv.rcv_data[i];
							if (rcv_data_roller.string.wp == 16) {
								if (memcmp(rcv_data_roller.packet.identifier, "AMANO_LCDPKT", 12) == 0) {
									memcpy(rcv_trailer.data, rcv_data_roller.string.buff, 16);
									rcv_data_size = 16;				// 16�ɕύX
									pkt_id_found_flg = 1;			// �p�P�b�g���ʎq�����t���O
									++i;							// 
									break;
								}else{
									// ������Ȃ��ꍇ�́A1�������炵�ĒT���Ă���
									memmove(&rcv_data_roller.string.buff[0], &rcv_data_roller.string.buff[1], 15);
									rcv_data_roller.string.wp--;
								}
							}
						}
					}
					if( pkt_id_found_flg == 1){
						// ��M�����T�C�Y or 1�d���̃T�C�Y�̏���������COPY
//						er = min(_ref_recv.buff.rcv.data_size - i,rcv_trailer.part.data_size - rcv_data_size);
						er = _ref_recv.buff.rcv.data_size - i;
// MH810100(S) 2020/06/26 #4508�y�G�[�W���O�zDCNET�̊����F�ؓo�^�E����v���f�[�^���đ�����邱�Ƃ�����
//						if( er > (rcv_trailer.part.data_size)){
//							er = rcv_trailer.part.data_size;
//						}
						if( er > (rcv_trailer.part.data_size - rcv_data_size)){
							er = rcv_trailer.part.data_size - rcv_data_size;
						}
// MH810100(S) 2020/06/26 #4508�y�G�[�W���O�zDCNET�̊����F�ؓo�^�E����v���f�[�^���đ�����邱�Ƃ�����
						memcpy(&rcv_trailer.data[rcv_data_size],&_ref_recv.buff.rcv.rcv_data[i],er);
						rcv_data_size = rcv_data_size + er;

						if( rcv_data_size >= rcv_trailer.part.data_size ){	// 1�d����M����
							ushort seqTemp = PKTrcv_Receive_Packet(pktctrl.rcv_seqno, &rcv_trailer);
							if(seqTemp != 0xFFFF) {
								pktctrl.rcv_seqno = seqTemp;
							}
							// �����M�p�P�b�g���ɖ������̓d������H(�����M���T�C�Y - (�����͓d���T�C�Y(���p�P�b�g�̂�) - ��͍ςݓd���T�C�Y))
							if( (short)_ref_recv.buff.rcv.data_size - ((rcv_data_size - prc_data_size_bf) + prc_data_size_nw) > 0 ){
								prc_data_size_nw += rcv_data_size - prc_data_size_bf;		// ��͍ςݓd���T�C�Y�ɍ����͓d���T�C�Y(���p�P�b�g�̂�)�����Z
								iDiff += rcv_data_size;
								rcv_data_size = 0;											// ���d����ׂ͂̈Ɏ�M�T�C�Y�N���A
								prc_data_size_bf = 0;										// 1�d����M���������̂Ńp�P�b�g�ׂ��̎�M�d���T�C�Y�N���A
								memset(&rcv_trailer, 0x00, sizeof(rcv_trailer));			// ��M�d���i�[�̈�N���A
								memset(&rcv_data_roller, 0x00, sizeof(rcv_data_roller));	// �w�b�_�i�[�̈�N���A
								pkt_id_found_flg = 0;			// �p�P�b�g���ʎq�����t���O
								continue;	// �����M�p�P�b�g�̎��d����M�E���͂�
							}else{	// �������d���Ȃ�
								break;	// ���[�v�𔲂���(��M����)
							}
						}
					}
					// ������Ȃ��ꍇ�͔�����
					break;
				}
				// �����M�T�C�Y���̏�������
				if( rcv_data_size >= rcv_trailer.part.data_size ){
					// �����M�T�C�Y���������A���d���T�C�Y����������
					rcv_data_size = 0;
					prc_data_size_bf = 0;
					prc_data_size_nw = 0;
					_ref_recv.flag = 0;
					pkt_id_found_flg = 0;			// �p�P�b�g���ʎq�����t���O
					break;
				}else{
					// �p�P�b�g���ʎq���������Ă��Ȃ��ꍇ�́A�j���B�������Ă���ꍇ�͈��p��
					if(pkt_id_found_flg == 1){
						prc_data_size_bf = rcv_data_size;	// ����܂ł̎�M�T�C�Y���o�b�N�A�b�v
						prc_data_size_nw = 0;				// 1�p�P�b�g���̉�͍ς݂̓d���T�C�Y�N���A
					}else{
						rcv_data_size = 0;
						prc_data_size_bf = 0;
						prc_data_size_nw = 0;
						_ref_recv.flag = 0;
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)

switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 30 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:���O����(ACK/NAK)�҂�
wmonlg( OPMON_TEST, 0, 31 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:���O�f�[�^�đ����f��
wmonlg( OPMON_TEST, 0, 32 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:���O�����f�[�^�҂�
wmonlg( OPMON_TEST, 0, 33 );
	break;
default:
wmonlg( OPMON_TEST, 0, 34 );
	break;
}
#endif
// �����p (e)
						break;
					}
				}
			}
		}
		taskchg( IDLETSKNO );
	}
}

static void pkt_send_req_sndAckNack(uchar ucKind, uchar ucDetail, short SndSeqNo)
{
	PKT_TRAILER*	p_trailer = &res_trailer;
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} crc16;

	memset(p_trailer, 0, sizeof(PKT_TRAILER));

	p_trailer->part.data_size = 28;		// CRC16(2byte)
	memcpy(&p_trailer->part.identifier, "AMANO_LCDPKT", sizeof(p_trailer->part.identifier));
	intoas(p_trailer->part.seqno, SndSeqNo, sizeof(p_trailer->part.seqno));
	p_trailer->part.id = 0x00;
	p_trailer->part.kind = 0x10;
	p_trailer->part.blkno_h = ucKind;		// Ack Nak
	p_trailer->part.blkno_l = ucDetail;	// Detail
	p_trailer->part.block_sts = 0;
	p_trailer->part.encrypt = 0;
	p_trailer->part.pad_size = 0;

	crc_ccitt((ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16.us, (uchar)R_SHIFT);	// CRC16�Z�o(little endian)
	p_trailer->data[p_trailer->part.data_size-2] = crc16.uc.high;	// little endian�̂܂܊i�[
	p_trailer->data[p_trailer->part.data_size-1] = crc16.uc.low;	//

	PktTcpSend((uchar*)p_trailer, (ulong)p_trailer->part.data_size);
}

//[]----------------------------------------------------------------------[]
///	@brief			AMANO_LCDPKT�̎�M����
//[]----------------------------------------------------------------------[]
///	@param[in]		rcv_seqno	: �O���M�V�[�P���X��(0�`999)
///	@param[out]		*p_trailer	: ��M�����p�P�b�g�f�[�^
///	@return			��M�V�[�P���X��(0�`999)
///	@note			����ȃp�P�b�g�����`�F�b�N�������Ǝ�M�o�b�t�@�Ɋi�[���܂��B<br>
///					�܂��R�}���h�P�ʂŃI�y���[�V�����^�X�N�ɒʒm���܂��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2009/09/29<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
static ushort PKTrcv_Receive_Packet(ushort rcv_seqno, PKT_TRAILER *p_trailer)
{
	ushort	seqno;
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} crc16;
	ushort	crc16us;
	uchar	*rcv;
	long	len;
	ushort	size;
	ushort	rp;
	ushort	msg;
	ushort	cmd_sz;
	uchar	cmd_dat[MSGBUFSZ];
	int		insize;
	ulong	id;
	uchar	result;

	seqno = astoin(p_trailer->part.seqno, sizeof(p_trailer->part.seqno));
	if(_ref_send.res_wait) {
		crc16.us = crc16us = 0;
		crc16.uc.high = p_trailer->part.data[p_trailer->part.data_size-28];	// little endian�̂܂ܕ]��
		crc16.uc.low = p_trailer->part.data[p_trailer->part.data_size-27];	//
		crc_ccitt((ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16us, (uchar)R_SHIFT);	// CRC16�Z�o(little endian)
		if(( p_trailer->part.id == 0x00 ) &&		// �f�[�^���R�[�h
		   ( p_trailer->part.kind == 0x10 ) &&		// �d�����ʃR�[�h
		   ( crc16.us == crc16us ) &&
		   ( p_trailer->part.blkno_h == RES_ACK || p_trailer->part.blkno_h == RES_NAK))		// 06H=ACK,15H=NAK
		{
			// ---------------------------------------------------
			// ACK / NACK��M������
			// ---------------------------------------------------
			// Ack/Nak���Ԃ��Ă����̂ŏ�����
			// �L�[�v�A���C�u�^�C�}���Z�b�g
			Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
			RESET_KEEPALIVE_COUNT;
			// �p�P�b�g�����҂����ԃ^�C�}��~
			Lagcan( PKTTCBNO, PKT_TIM_RES_WT );
			_ref_send.res_wait = 0;

			// ACK��M�H
			if( p_trailer->part.blkno_h == RES_ACK ){
				_ref_send.rtry_cnt = 0;	// ���g���C�J�E���^�N���A
				// ���M�����̂�RT���O�f�[�^�H
				if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
					// ����ACK�H
					if( p_trailer->part.blkno_l == 0x00 ){
						Lagtim( PKTTCBNO, PKT_TIM_RTPAY_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// �p�P�b�g�����f�[�^�҂��^�C�}�J�n
						pktctrl.send.RT_LogSend = LOG_PHASE_WAIT_RES_CMD;		// ���O�����f�[�^�҂�
					}
					// ����ACK����(���g���C�I�[�o�[)
					else{
						Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
						pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
						_ref_buf_type = 'S';
					}
				}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				// ���M�����̂�RT���O�f�[�^�H
				else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ) {
					// ����ACK�H
					if ( p_trailer->part.blkno_l == 0x00 ) {
						Lagtim(PKTTCBNO, PKT_TIM_RTRECEIPT_RES, (ushort)(pktctrl.RTPay_res_tm*50));	// �p�P�b�g�����f�[�^�҂��^�C�}�J�n
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_WAIT_RES_CMD;	// ���O�����f�[�^�҂�
					}
					// ����ACK����(���g���C�I�[�o�[)
					else {
						Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
						_ref_buf_type = 'S';
					}
				}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				// ���M�����̂�QR���O�f�[�^�H
				else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
					// ����ACK�H
					if( p_trailer->part.blkno_l == 0x00 ){
						Lagtim( PKTTCBNO, PKT_TIM_DC_QR_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// �p�P�b�g�����f�[�^�҂��^�C�}�J�n
						pktctrl.send.QR_LogSend = LOG_PHASE_WAIT_RES_CMD;		// ���O�����f�[�^�҂�
					}
					// ����ACK����(���g���C�I�[�o�[)
					else{
						// ���O�f�[�^�͋���ACK������M�ł��j�����Ȃ�
						Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
						pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
						_ref_buf_type = 'S';
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 2 );
#endif
// �����p (e)
					}
				}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				// ���M�����̂̓��[�����j�^���O�f�[�^�H
				else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
					// ����ACK�H
					if( p_trailer->part.blkno_l == 0x00 ){
						Lagtim( PKTTCBNO, PKT_TIM_DC_QR_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// �p�P�b�g�����f�[�^�҂��^�C�}�J�n
						pktctrl.send.LANE_LogSend = LOG_PHASE_WAIT_RES_CMD;		// ���O�����f�[�^�҂�
					}
					// ����ACK����(���g���C�I�[�o�[)
					else{
						// ���O�f�[�^�͋���ACK������M�ł��j�����Ȃ�
						Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
						pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
						_ref_buf_type = 'S';
					}
				}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				// �ʏ�f�[�^�ɑ΂��鉞���H
				else{
					if( PktResetReqFlgRead() ){	// ���Z�b�g�v���ɑ΂���ACK�H
						PktResetReqFlgSet( 0 );	// ���Z�b�g�v���t���O�N���A
					}
					// ����ACK�������܂߂đ��M�����Ƃ��Ď��f�[�^���M�Ƃ���
					_ref_send.sent += 1;
					if( _ref_send.sent > 9999 ){
						_ref_send.sent = 0;
					}
					// ���O�f�[�^�đ����f���H
					if( pktctrl.send.RT_LogSend == LOG_PHASE_RESEND_CANCEL ){
						pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�đ����f����
						_ref_buf_type = 'S';
					}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_RESEND_CANCEL ) {
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�đ����f����
						_ref_buf_type = 'S';
					}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					else if( pktctrl.send.QR_LogSend == LOG_PHASE_RESEND_CANCEL ){
						pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�đ����f����
						_ref_buf_type = 'S';
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 13 );
#endif
// �����p (e)
					}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					else if( pktctrl.send.LANE_LogSend == LOG_PHASE_RESEND_CANCEL ){
						pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�đ����f����
						_ref_buf_type = 'S';
					}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				}
				if( p_trailer->part.blkno_l == 0x01 ){	// ����ACK����(���g���C�I�[�o�[)��M
					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_FORCED_ACK, 2, 0, 0, NULL );
				}
				// �đ��o�b�t�@���N���A���Ď��f�[�^�𑗐M
				PKTbuf_ClearReSendCommand();
				queset( PKTTCBNO, PKT_SND_PRT, 0, NULL );
			}
			// NAK��M�H
			else{
				if( ++_ref_send.rtry_cnt > pktctrl.snd_rtry_cnt ){	// ���M���g���C�I�[�o�[
					_ref_send.rtry_cnt = 0;	// ���g���C�J�E���^�N���A
					// ���M�����̂�RT���Z���O�f�[�^�H
					if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
						// �đ��𒆒f���ă��O�f�[�^�ȊO�̑��M���ɍs��
						PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
						pktctrl.send.RT_LogSend = LOG_PHASE_RESEND_CANCEL;	// ���O�f�[�^�đ����f
						return 0xFFFF;
					}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					// ���M�����̂�RT�̎��؃��O�f�[�^�H
					else if( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ){
						// �đ��𒆒f���ă��O�f�[�^�ȊO�̑��M���ɍs��
						PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_RESEND_CANCEL;	// ���O�f�[�^�đ����f
						return 0xFFFF;
					}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					// ���M�����̂�QR�m��E������O�f�[�^�H
					else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
						// �đ��𒆒f���ă��O�f�[�^�ȊO�̑��M���ɍs��
						PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
						pktctrl.send.QR_LogSend = LOG_PHASE_RESEND_CANCEL;	// ���O�f�[�^�đ����f
						return 0xFFFF;
					}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					// ���M�����̂̓��[�����j�^���O�f�[�^�H
					else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
						// �đ��𒆒f���ă��O�f�[�^�ȊO�̑��M���ɍs��
						PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
						pktctrl.send.LANE_LogSend = LOG_PHASE_RESEND_CANCEL;	// ���O�f�[�^�đ����f
						return 0xFFFF;
					}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					// ���M�����̂͒ʏ�f�[�^�H
					else{
						if( ( pktctrl.rsnd_rtry_ov ) &&	// ���g���C�I�[�o�[���̍đ��M����
							( PKTbuf_CheckReSendCommand() != LCDBM_CMD_NOTIFY_CONFIG )){	// ��{�ݒ�v���łȂ�
							;	// �đ��M(��{�ݒ�v���̓��g���C�I�[�o�[���̍đ��M�ΏۊO�Ƃ���)
						}else{
							err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_RTRY_OV, 2, 0, 0, NULL );
							_ref_send.sent += 1;	// ���M�ς݂Ƃ���
							PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
							// RT���Z���O�f�[�^�đ����f���H
							if( pktctrl.send.RT_LogSend == LOG_PHASE_RESEND_CANCEL ){
								// RT���Z�f�[�^�đ����f����������RT���Z�f�[�^�đ��J�n
								pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;
								_ref_buf_type = 'S';
							}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
							// RT�̎��؃��O�f�[�^�đ����f���H
							else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_RESEND_CANCEL ) {
								// RT�̎��؃f�[�^�đ����f����������RT�̎��؃f�[�^�đ��J�n
								pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;
								_ref_buf_type = 'S';
							}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
							// QR�m��E������O�f�[�^�đ����f���H
							else if( pktctrl.send.QR_LogSend == LOG_PHASE_RESEND_CANCEL ){
								// QR�m��E����f�[�^�đ����f����������QR�m��E����f�[�^�đ��J�n
								pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;
								_ref_buf_type = 'S';
							}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
							// ���[�����j�^���O�f�[�^�đ����f���H
							else if( pktctrl.send.LANE_LogSend == LOG_PHASE_RESEND_CANCEL ){
								// ���[�����j�^�f�[�^�đ����f���������ă��[�����j�^�f�[�^�đ��J�n
								pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;
								_ref_buf_type = 'S';
							}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
							return 0xFFFF;
						}
					}
				}
				// �đ�(�����ɗ���̂̓��g���C�I�[�o�O�ƃ��g���C�I�[�o�����đ�����ꍇ)

				// RT�ʐM�H
				if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// RT���Z�f�[�^�����M��Ԃɖ߂�
					_ref_buf_type = 'S';
				}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				// RT�ʐM(�̎��؃f�[�^)�H
				else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ) {
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// RT�̎��؃f�[�^�����M��Ԃɖ߂�
					_ref_buf_type = 'S';
				}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				// DC-NET�ʐM�H
				else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// QR�m��E����f�[�^�����M��Ԃɖ߂�
					_ref_buf_type = 'S';
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 3 );
#endif
// �����p (e)
				}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				// DC-NET�ʐM(���[�����j�^�f�[�^)�H
				else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���[�����j�^�f�[�^�����M��Ԃɖ߂�
					_ref_buf_type = 'S';
				}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				// �ʏ�f�[�^
				else{
					_ref_buf_type = 'A';
				}
				queset( PKTTCBNO, PKT_SND_PRT, 0, NULL );
			}
			return 0xFFFF;
		}
		if(( p_trailer->part.id != 0x00 ) ||		// �f�[�^���R�[�h
		   ( p_trailer->part.kind != 0x10)){		// �d�����ʃR�[�h
			err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_HDR, 2, 0, 0, NULL );
		}else if( crc16.us != crc16us ){
			err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_CRC, 2, 0, 0, NULL );
		}
	}
	else {
		if(p_trailer->part.data_size == PKT_RES_SIZE) {
			// TCP�đ����ɂ�艞���҂��ȊO�ŉ����f�[�^����M�����ꍇ�͔j������
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 8 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:���O����(ACK/NAK)�҂�
wmonlg( OPMON_TEST, 0, 9 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:���O�f�[�^�đ����f��
wmonlg( OPMON_TEST, 0, 10 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:���O�����f�[�^�҂�
wmonlg( OPMON_TEST, 0, 11 );
	break;
default:
wmonlg( OPMON_TEST, 0, 12 );
	break;
}
#endif
// �����p (e)
			return 0xFFFF;
		}
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
else{
switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_WAIT_ACK:			// 1:���O����(ACK/NAK)�҂�
wmonlg( OPMON_TEST, 0, 45 );
}
}
#endif
// �����p (e)
	}


	if((seqno == 0) || (seqno != rcv_seqno)){
		crc16.us = crc16us = 0;
		crc16.uc.high = p_trailer->part.data[p_trailer->part.data_size-28];	// little endian�̂܂ܕ]��
		crc16.uc.low = p_trailer->part.data[p_trailer->part.data_size-27];	//
		// CRC�͈Í�������Ă���ꍇ�����̂܂܎Z�o
		crc_ccitt((ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16us, (uchar)R_SHIFT);	// CRC16�Z�o(little endian)
		if(( p_trailer->part.id == 0x00 )	&&	// �f�[�^���R�[�h(00H �Œ�)
		   ( p_trailer->part.kind == 0x10 ) &&	// �d�����ʃR�[�h(10H=�p�P�b�g�g���C���i�Œ�j)
		   ( p_trailer->part.blkno_l == 1 ) &&	// �u���b�NNo.(1�Œ�)
		   ( p_trailer->part.block_sts == 1 )){	// �u���b�N�X�e�[�^�X(1�Œ�)
			if( crc16.us == crc16us ){
				rp = 0;
				if( p_trailer->part.encrypt == 1 ){	// �Í�������=AES
					// ����������
					insize = (int)(p_trailer->part.data_size-26-2);	// �f�[�^���T�C�Y���p�P�b�g�f�[�^���|�w�b�_�T�C�Y�|CRC16�T�C�Y
					if( insize > DECRYPT_COM ){	// ��M�d�������������p�G���A�T�C�Y
						insize = 0;	// ������NG�Ƃ���
					}else{
						insize = DecryptWithKeyAndIV_NoPadding( CRYPT_KEY_RXMLCDPK, (uchar*)&p_trailer->part.data, EncDecrypt_buff, insize );
					}
					if( !insize ){	// ������NG
						// ��M�f�[�^�������G���[(�����^��������)
						err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_DCRYPT_FL, 2, 0, 0, NULL );
						_ref_recv.usNackRetryNum++;
						if(_ref_recv.usNackRetryNum > pktctrl.rcv_rtry_cnt ) {
							// NAK���g���C�I�[�o�[(ACK:�����ڍ�=01H(���g���C�I�[�o�[�̈׋���ACK����))
							err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_FORCED_ACK, 2, 0, 0, NULL );
							pkt_send_req_sndAckNack(RES_ACK, 0x01, seqno);

							// RT���O�����f�[�^�҂��H
							if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
								Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  ���O�����f�[�^�҂��^�C�}��~
								Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
								pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
								// �ʏ�f�[�^���M���̉\��������̂�_ref_buf_type�͕ύX���Ȃ�
							}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
							// RT���O�����f�[�^(�̎��؃f�[�^)�҂��H
							if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
								Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	//  ���O�����f�[�^�҂��^�C�}��~
								Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
								pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
								// �ʏ�f�[�^���M���̉\��������̂�_ref_buf_type�͕ύX���Ȃ�
							}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
							// DC���O�����f�[�^�҂��H
							if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
								Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  ���O�����f�[�^�҂��^�C�}��~
								Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
								pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
								// �ʏ�f�[�^���M���̉\��������̂�_ref_buf_type�͕ύX���Ȃ�
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 14 );
#endif
// �����p (e)
							}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
							// DC���O�����f�[�^(���[�����j�^�f�[�^)�҂��H
							if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
								Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  ���O�����f�[�^�҂��^�C�}��~
								Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
								pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
								// �ʏ�f�[�^���M���̉\��������̂�_ref_buf_type�͕ύX���Ȃ�
							}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
						} else {
							pkt_send_req_sndAckNack(RES_NAK, 0x11, seqno);
						}
						return 0xFFFF;
					}
					rcv = EncDecrypt_buff;
				}else{	// �Í����Ȃ�
					rcv = p_trailer->part.data;
				}
				// �L�[�v�A���C�u�^�C�}���Z�b�g
				Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
				RESET_KEEPALIVE_COUNT;
				_ref_recv.usNackRetryNum = 0;
				pkt_send_req_sndAckNack(RES_ACK, 0x00,seqno);
				len = *(ushort *)&rcv[rp];				// �ŏ���2byte�̓p�P�b�g�̃f�[�^�����i�[����G���A
				rp += 2;

				while(0<len) {
					taskchg(IDLETSKNO);
					size = *(ushort *)&rcv[rp];
					rp += 2;
					// �R�}���hID/�T�u�R�}���hID���`�F�b�N���A���b�Z�[�W�ԍ��ɕϊ�
					// (�ȉ��́u==�v�ł͂Ȃ��߂�l(msg)���u0�v���ۂ������Ă��܂�)
					if (msg = lcdbm_check_message(*(ulong *)&rcv[rp])) {
						cmd_sz = size;
						if ( cmd_sz > MSGBUFSZ ) {
							cmd_sz = MSGBUFSZ;
						}
						memcpy( cmd_dat, &rcv[rp], cmd_sz );
						queset( OPETCBNO, msg, cmd_sz, cmd_dat );
					} else {
						switch( *(ulong *)&rcv[rp] ){
							case LCDBM_RSP_NOTIFY_CONFIG:	// ��{�ݒ艞��
								// ��{�ݒ艞���̎�M����ۑ�(ope�^�X�N�ւ̒ʒm�Ȃ�)
								lcdbm_receive_config((lcdbm_rsp_config_t *)&rcv[rp]);
								// �L�[�v�A���C�u���g���C�J�E���^���Z�b�g
								lcdbm_reset_keepalive_status();

								// �����f�[�^���M�v(�N����)�H
								if( pkt_get_restore_unsent_flg() == 1 ){
									// ���A���^�C�����Z�f�[�^����H
									if( Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) ){
										result = 1;		// �����M�L��
									}
									// �f�[�^�Ȃ��H
									else{
										result = 0;		// �S�f�[�^���M����
									}
									PKTcmd_notice_ope( LCDBM_OPCD_RESTORE_NOTIFY, (ushort)result );
									pkt_set_restore_unsent_flg(0);
								}
								break;
							case LCDBM_RSP_TENKEY_KEYDOWN:	// �e���L�[�������
								// ��M�{�^����Ԃ̕ω����`�F�b�N(�֐�����ope�^�X�N��queset())
								pkt_tenkey_status_check(&rcv[rp+4]);
								break;
							case LCDBM_RSP_NOTICE_OPE:	// ����ʒm
								// �����f�[�^�v����M
								if( rcv[rp+4] == LCDBM_OPCD_RESTORE_REQUEST ){
									// ���A���^�C�����Z�f�[�^����H
									if( Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) ){
										result = 1;		// �����M�L��
									}
									// �f�[�^�Ȃ��H
									else{
										result = 0;		// �S�f�[�^���M����
									}
									PKTcmd_notice_ope( LCDBM_OPCD_RESTORE_RESULT, (ushort)result );
									pkt_set_restore_unsent_flg(0);
									break;
								}
								// ���Z�b�g�ʒm
								if( rcv[rp+4] == LCDBM_OPCD_RESET_NOT ){
									// ���Z�b�g�ʒm�Ȃ�ؒf���o�҂��^�C�}�J�n(���̊Ԃ̐ؒf���o�̓G���[�o�^�ΏۊO)
									Lagtim( PKTTCBNO, PKT_TIM_DISCON_WAIT, pktctrl.discon_wait_tm*50 );	// �R�l�N�V�����ؒf���o�҂��^�C�}�J�n
									pktctrl.f_wait_discon = 1;	// �R�l�N�V�����ؒf���o�҂��^�C�}�N�����t���OON
								}
								cmd_sz = size;
								if ( cmd_sz > MSGBUFSZ ) {
									cmd_sz = MSGBUFSZ;
								}
								memcpy( cmd_dat, &rcv[rp], cmd_sz );
								queset( OPETCBNO, LCD_OPERATION_NOTICE, cmd_sz, cmd_dat );
								break;
							case LCDBM_RSP_IN_INFO:			// ���ɏ��
								// ���ɏ��̎�M����ۑ�(�֐�����ope�^�X�N��queset())
								lcdbm_receive_in_car_info((lcdbm_rsp_in_car_info_t *)&rcv[rp]);
								break;
							case LCDBM_RSP_QR_DATA:			// QR�f�[�^
								// QR�f�[�^�̎�M����ۑ�(�֐�����ope�^�X�N��queset())
								lcdbm_receive_QR_data((lcdbm_rsp_QR_data_t *)&rcv[rp]);
								break;
							case LCDBM_RSP_PAY_DATA_RES:	// ���Z�����f�[�^
								// ���Z�����f�[�^�҂��H
								if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
									// ��M�f�[�^���
									lcdbm_receive_RTPay_res( (lcdbm_rsp_pay_data_res_t *)&rcv[rp], &id, &result );
									if( id == Pay_Data_ID ){	// ���Z�f�[�^��ID�ƈ�v
										Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  ���O�f�[�^�����f�[�^�҂��^�C�}��~
										if( result == 0 ){	// ����OK
											Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���

											// ���A���^�C�����Z�f�[�^�c�Ȃ��H
											if( 0 == Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) ){
												// �����f�[�^���M�v(������)�H
												if( pkt_get_restore_unsent_flg() == 2 ){
													// �����f�[�^�ʒm(�S�f�[�^���M����)
													PKTcmd_notice_ope( LCDBM_OPCD_RESTORE_RESULT, (ushort)0 );
													pkt_set_restore_unsent_flg(0);
												}
											}
										}else{	// ����NG
											// �đ��E�F�C�g�^�C�}��ɍđ�
											Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
											Lagtim( PKTTCBNO, PKT_TIM_RTPAY_RESND, pktctrl.RTPay_resnd_tm*50 );	// �đ��E�F�C�g�^�C�}�J�n
										}
										pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M
										// ���O�f�[�^�҂����͑��f�[�^���M���̉\��������̂�_ref_buf_type�͕ύX���Ȃ�
									}	// ID�s��v�̎��͎󂯎̂�(�����҂��^�C�}�p��)
								}	// �����҂��łȂ����͎󂯎̂�

// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
else{
switch(pktctrl.send.RT_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 40 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:���O����(ACK/NAK)�҂�
wmonlg( OPMON_TEST, 0, 41 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:���O�f�[�^�đ����f��
wmonlg( OPMON_TEST, 0, 42 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:���O�����f�[�^�҂�
wmonlg( OPMON_TEST, 0, 43 );
	break;
default:
wmonlg( OPMON_TEST, 0, 44 );
	break;
}
switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 35 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:���O����(ACK/NAK)�҂�
wmonlg( OPMON_TEST, 0, 36 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:���O�f�[�^�đ����f��
wmonlg( OPMON_TEST, 0, 37 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:���O�����f�[�^�҂�
wmonlg( OPMON_TEST, 0, 38 );
	break;
default:
wmonlg( OPMON_TEST, 0, 39 );
	break;
}

}
#endif
// �����p (e)
								break;
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
							case LCDBM_RSP_RECEIPT_DATA_RES:	// �̎��؃f�[�^����
								// �̎��؃f�[�^�����҂��H
								if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
									// ��M�f�[�^���
									lcdbm_receive_RTReceipt_res( (lcdbm_rsp_receipt_data_res_t *)&rcv[rp], &id, &result );
									if ( id == Receipt_Data_ID ) {	// �̎��؃f�[�^��ID�ƈ�v
										Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	//  ���O�f�[�^�����f�[�^�҂��^�C�}��~
										if ( result == 0 ) {	// ����OK
											Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
										} else {				// ����NG
											// �đ��E�F�C�g�^�C�}��ɍđ�
											Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );		// ��ǂ݂����ɖ߂�
											Lagtim( PKTTCBNO, PKT_TIM_RTRECEIPT_RESND, pktctrl.RTPay_resnd_tm*50 );	// �đ��E�F�C�g�^�C�}�J�n
										}
										pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M
										// ���O�f�[�^�҂����͑��f�[�^���M���̉\��������̂�_ref_buf_type�͕ύX���Ȃ�
									}	// ID�s��v�̎��͎󂯎̂�(�����҂��^�C�}�p��)
								}	// �����҂��łȂ����͎󂯎̂�
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
							case LCDBM_RSP_QR_CONF_CAN_RES:	// QR�m��E����f�[�^����
								// QR�m���������f�[�^�҂��H
								if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
									// ��M�f�[�^���
									lcdbm_receive_DC_QR_res( (lcdbm_rsp_QR_conf_can_res_t *)&rcv[rp], &id, &result );
									if( id == QR_Data_ID ){	// QR�m��E����f�[�^��ID�ƈ�v
										Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  ���O�f�[�^�����f�[�^�҂��^�C�}��~
// MH810100(S) K.Onodera  2020/03/16 �Ԕԃ`�P�b�g���X(#4034 DC-NET�ő��M�f�[�^������~����M���Ă��f�[�^���đ��������Ă��܂�)
//										if( result == 0 ){	// ����OK
										// ����OK or Nack99�ő��M�ς�
										if( result == 0 || result == 1 ){
// MH810100(E) K.Onodera  2020/03/16 �Ԕԃ`�P�b�g���X(#4034 DC-NET�ő��M�f�[�^������~����M���Ă��f�[�^���đ��������Ă��܂�)
											Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
										}else{	// ����NG
											// �đ��E�F�C�g�^�C�}��ɍđ�
											Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
											Lagtim( PKTTCBNO, PKT_TIM_DC_QR_RESND, pktctrl.RTPay_resnd_tm*50 );	// �đ��E�F�C�g�^�C�}�J�n
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 4 );
#endif
// �����p (e)
										}
										pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M
										// ���O�f�[�^�҂����͑��f�[�^���M���̉\��������̂�_ref_buf_type�͕ύX���Ȃ�
									}	// ID�s��v�̎��͎󂯎̂�(�����҂��^�C�}�p��)
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
else{
	wmonlg( OPMON_TEST, 0, 15 );
}
#endif
// �����p (e)
								}	// �����҂��łȂ����͎󂯎̂�
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
else{
//	wmonlg( OPMON_TEST, 0, 16 );

switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 25 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:���O����(ACK/NAK)�҂�
wmonlg( OPMON_TEST, 0, 26 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:���O�f�[�^�đ����f��
wmonlg( OPMON_TEST, 0, 27 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:���O�����f�[�^�҂�
wmonlg( OPMON_TEST, 0, 28 );
	break;
default:
wmonlg( OPMON_TEST, 0, 29 );
	break;
}

}
#endif
// �����p (e)
								break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
							case LCDBM_RSP_LANE_RES:	// ���[�����j�^�f�[�^����
								// ���[�����j�^�f�[�^�����҂��H
								if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){	
									// ��M�f�[�^���
									lcdbm_receive_DC_LANE_res( (lcdbm_rsp_LANE_res_t *)&rcv[rp], &id, &result );
									if( id == LANE_Data_ID ){	// ���[�����j�^�f�[�^��ID�ƈ�v
										Lagcan( PKTTCBNO, PKT_TIM_DC_LANE_RES );	//  ���O�f�[�^�����f�[�^�҂��^�C�}��~
										// ����OK or Nack99�ő��M�ς�
										if( result == 0 || result == 1 ){
											Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
										}else{	// ����NG
											// �đ��E�F�C�g�^�C�}��ɍđ�
											Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
											Lagtim( PKTTCBNO, PKT_TIM_DC_LANE_RESND, pktctrl.RTPay_resnd_tm*50 );	// �đ��E�F�C�g�^�C�}�J�n
										}
										pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M
										// ���O�f�[�^�҂����͑��f�[�^���M���̉\��������̂�_ref_buf_type�͕ύX���Ȃ�
									}	// ID�s��v�̎��͎󂯎̂�(�����҂��^�C�}�p��)
								}	// �����҂��łȂ����͎󂯎̂�
								break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
							case LCDBM_RSP_MNT_QR_CTRL_RES:	// QR���[�_���䉞��
								// QR���[�_���䉞���̎�M����ۑ�(�֐�����ope�^�X�N��queset())
								lcdbm_receive_QR_ctrl_res((lcdbm_rsp_QR_ctrl_res_t *)&rcv[rp]);
								break;
							case LCDBM_RSP_MNT_QR_RD_RSLT:	// QR�ǎ挋��
								// QR�ǎ挋�ʂ̎�M����ۑ�(�֐�����ope�^�X�N��queset())
								lcdbm_receive_QR_rd_rslt((lcdbm_rsp_QR_rd_rslt_t *)&rcv[rp]);
								break;
							default:	// �s���R�}���h�͔j��
								break;
						}
					}
					rp += size;
					len -= (size + 2);
				}
			}
			// CRC�`�F�b�NNG
			else{
				// ��M�f�[�^CRC�G���[(�����^��������)
				err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_CRC, 2, 0, 0, NULL );
				_ref_recv.usNackRetryNum++;
				if(_ref_recv.usNackRetryNum > pktctrl.rcv_rtry_cnt ) {
					// Nak���g���C�I�[�o�[(ACK:�����ڍ�=01H(���g���C�I�[�o�[�̈׋���ACK����))
					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_FORCED_ACK, 2, 0, 0, NULL );
					pkt_send_req_sndAckNack(RES_ACK, 0x01, seqno);

					// RT���O�����f�[�^�҂��H
					if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  RT���Z�����f�[�^�҂��^�C�}��~
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
						pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
						// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
						// _ref_buf_type�͕ύX���Ȃ�
					}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					// RT�̎��؃��O�����f�[�^�҂��H
					if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
						Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	// RT�̎��؃f�[�^�����҂��^�C�}��~
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
						// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
						// _ref_buf_type�͕ύX���Ȃ�
					}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					// QR���O�����f�[�^�҂��H
					if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  RT���Z�����f�[�^�҂��^�C�}��~
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
						pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
						// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
						// _ref_buf_type�͕ύX���Ȃ�
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 17 );
#endif
// �����p (e)
					}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					// ���[�����j�^���O�����f�[�^�҂��H
					if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  RT���Z�����f�[�^�҂��^�C�}��~
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
						pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
						// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
						// _ref_buf_type�͕ύX���Ȃ�
					}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				} else {
					pkt_send_req_sndAckNack(RES_NAK, 0x11, seqno);
				}
				return 0xFFFF;
			}
		}
		else{
			// ��M�f�[�^�w�b�_�G���[(�����^��������)
			err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_HDR, 2, 0, 0, NULL );
		}
	}
	else {	// �����SEQ
		// ��M�f�[�^�V�[�P���X���G���[(�����^��������)
		err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_SEQ_NO, 2, 0, 0, NULL );
		_ref_recv.usNackRetryNum++;
		if(_ref_recv.usNackRetryNum > pktctrl.rcv_rtry_cnt ) {
			// Nak���g���C�I�[�o�[(ACK:�����ڍ�=01H(���g���C�I�[�o�[�̈׋���ACK����))
			err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_FORCED_ACK, 2, 0, 0, NULL );
			pkt_send_req_sndAckNack(RES_ACK, 0x1, seqno);

			// RT���O�����f�[�^�҂��H
			if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
				Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  RT���Z�����f�[�^�҂��^�C�}��~
				Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
				pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// RT���Z�f�[�^�����M��Ԃɖ߂�
				// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
				// _ref_buf_type�͕ύX���Ȃ�
			}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
			// RT�̎��؃��O�����f�[�^�҂��H
			if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
				Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	// RT�̎��؃f�[�^�����҂��^�C�}��~
				Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
				pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// RT�̎��؃f�[�^�����M��Ԃɖ߂�
				// �̎��؃f�[�^�����҂����͑��f�[�^���M���̉\��������̂�
				// _ref_buf_type�͕ύX���Ȃ�
			}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
			// QR���O�����f�[�^�҂��H
			if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
				Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  QR�m��E��������f�[�^�҂��^�C�}��~
				Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
				pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// QR�m��E����f�[�^�����M��Ԃɖ߂�
				// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
				// _ref_buf_type�͕ύX���Ȃ�
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 18 );
#endif
// �����p (e)
			}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			// ���[�����j�^���O�����f�[�^�҂��H
			if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
				Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  QR�m��E��������f�[�^�҂��^�C�}��~
				Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
				pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���[�����j�^�f�[�^�����M��Ԃɖ߂�
				// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
				// _ref_buf_type�͕ύX���Ȃ�
			}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		} else {
			pkt_send_req_sndAckNack(RES_NAK, 0x11, seqno);
		}
		return 0xFFFF;
	}

	memset(p_trailer, 0, sizeof(PKT_TRAILER));
	return(seqno);
}

//[]----------------------------------------------------------------------[]
///	@brief			HIF�f�[�^���M����
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
///	@note			HIF���M�o�b�t�@��HIF�f�[�^���i�[���܂��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void pkt_send_start( void )
{
	uchar	rst_cause;

	int iRet = PktTcpSend( (uchar*)&_ref_send.buff.snd.snd_data, (ulong)_ref_send.buff.snd.data_size );

	// ���M�����i�v���T�C�Y�Ƒ��M�ς݃T�C�Y����v�j
	if( iRet == _ref_send.buff.snd.data_size ){
		// �p�P�b�g�����҂��^�C�}���J�n���ĉ����҂���ԂƂ���
		Lagtim( PKTTCBNO, PKT_TIM_RES_WT, (ushort)(pktctrl.res_wt_tm*50) );
		_ref_send.res_wait = 1;
		_ref_send.wait_seqno = pktctrl.snd_seqno;
	}
	// ���M���s�H
	else {
		// �̏ᔭ�����邽�߁A�\�P�b�g���Z�b�g
		rst_cause = ERR_TKLSLCD_KSG_SND;
		queset( PKTTCBNO, PKT_RST_PRT, 1, &rst_cause );
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 19 );
#endif
// �����p (e)
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			TCP Connect�v��
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int pkt_send_req_con( void )
{
	struct sockaddr_in	serverAddr;			// HOST�A�h���X�E�|�[�g�ԍ�
	int sockError;							// �\�P�b�g�G���[�R�[�h
	int ret = -1;

	if( pktctrl.ipa > 0 && pktctrl.port > 0 ){
		memset( &serverAddr, 0, sizeof(serverAddr) );
		serverAddr.sin_family      = AF_INET;
		serverAddr.sin_port        = pktctrl.port;		// �ڑ���(LCD)�|�[�g�ԍ�
		serverAddr.sin_addr.s_addr = pktctrl.ipa;		// �ڑ���(LCD)IP�A�h���X

		ret = connect( PKT_Socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr) );
		if( ret == -1 ){
			sockError = (int)pkt_ksg_err_getreg( PKT_Socket, ERR_TKLSLCD_KSG_CON, 2 );
			if( sockError == TM_EINPROGRESS ||			// �m���u���b�L���O�Ȃ̂Őڑ����̓G���[����Ȃ�
				sockError == TM_EISCONN ){				// ���ɐڑ����̓G���[�ł͂Ȃ�(connect�`GetSockError�ԂŐڑ�����)
				ret = 0;								// �G���[�Ƃ��Ȃ�
			}
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�p�P�b�g���M�v��
//[]----------------------------------------------------------------------[]
///	@param[in]		fid		: �@�\ID(FID_PT1�`FID_PT4)
///	@param[in]		size	: �f�[�^�T�C�Y
///	@param[in]		data	: �f�[�^
///	@param[in]		push	: Push�t���O(�T�uCPU�̑��M�I�v�V����)
///	@return			�Ȃ�
///	@attention		Push�t���O��1�ɂ����ꍇ�͑����p�P�b�g������đ��M���܂��B
///	@note			�w�肳�ꂽ�f�[�^�Ƀf�[�^�w�b�_(10byte)��t�����đ��M����B<br>
///					(HIF�w�b�_��HIF���M�^�X�N���t�����܂�)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void pkt_send_req_snd( ushort size, uchar *data, uchar push )
{
	memset( &(_ref_send.buff), 0, sizeof(PKT_SND_CMD) );

	_ref_send.buff.snd.kind = PKT_PKT_SND_REQ;
	// ���M�f�[�^���ʔԍ�
	_ref_send.buff.snd.snd_no = 0;
	// �u���b�N�X�e�[�^�X
	_ref_send.buff.snd.block_sts = push;
	// �f�[�^�T�C�Y
	_ref_send.buff.snd.data_size = size;
	// �f�[�^
	memcpy( _ref_send.buff.snd.snd_data, data, (size_t)(_ref_send.buff.snd.data_size) );

	pkt_send_start();
}

//[]----------------------------------------------------------------------[]
///	@brief			�m���u���b�L���O�̃R�[���o�b�N
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PktSocCbk( int sock, int event )
{
	uchar	rst_cause;

// MH810105(S) R.Endo 2021/12/13 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
#if DEBUG_JOURNAL_PRINT
	T_FrmDebugData frm_debug_data;
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	struct clk_rec	wk_CLK_REC_debug;
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	// �N���[�Y�����A���肩��̃��Z�b�g�A���肩��̐ؒf�A�G���[
	if ( (event & TM_CB_CLOSE_COMPLT) ||
		 (event & TM_CB_RESET) ||
		 (event & TM_CB_REMOTE_CLOSE) ||
		 (event & TM_CB_SOCKET_ERROR) ) {
		// �N���A
		memset(&frm_debug_data, 0x00, sizeof(frm_debug_data));

		// ���ݓ���
		c_Now_CLK_REC_ms_Read(
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//			&frm_debug_data.wk_CLK_REC,
			&wk_CLK_REC_debug,
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
			&frm_debug_data.wk_CLK_REC_msec);
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		// 7�o�C�g�̂�COPY
		memcpy( &frm_debug_data.wk_CLK_YMDHMS, &wk_CLK_REC_debug, sizeof(DATE_YMDHMS) );
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

		// �W���[�i����
		frm_debug_data.prn_kind = J_PRI;

		// 1�s��
		frm_debug_data.first_line = TRUE;
		sprintf(frm_debug_data.data, "cur_sock:%-2d sock:%-2d event:%04x", PKT_Socket, sock, event);
		queset(PRNTCBNO, PREQ_DEBUG, sizeof(T_FrmDebugData), &frm_debug_data);
	}
#endif
// MH810105(E) R.Endo 2021/12/13 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��

	// �\�P�b�g���قȂ�܂��͊��ɃN���[�Y�ς݁H
	if( PKT_Socket != sock ){
		return;
	}

	// ���肩���Reset / ���肩��̐ؒf / �G���[����
	if( (event & TM_CB_RESET) || (event & TM_CB_REMOTE_CLOSE) || (event & TM_CB_SOCKET_ERROR) ){
		if( PKT_Socket != -1 ){
			if( event & TM_CB_RESET ){
				rst_cause = ERR_TKLSLCD_RMT_RST;
			}else if( event & TM_CB_REMOTE_CLOSE ){
				rst_cause = ERR_TKLSLCD_RMT_DSC;
			}else{
				rst_cause = ERR_TKLSLCD_ERR_OCC;
			}
			// �\�P�b�g�N���[�Y�̓^�X�N�ŃC�x���g����M���ɍs��
			queset( PKTTCBNO, PKT_RST_PRT, 1, &rst_cause );
		}
	}
	// �ڑ�����
	else if( event & TM_CB_CONNECT_COMPLT ){
		queset( PKTTCBNO, PKT_SOC_CON, 0, NULL );
	}
	// �f�[�^��M
	else if( (event & TM_CB_RECV) || (event & TM_CB_RECV_OOB) ){
		queset( PKTTCBNO, PKT_SOC_RCV, 0, NULL );
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�p�P�b�g�ʐM�@�\������
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/10/31<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void pkt_init( void )
{
	// �ȉ��A0�N���A����Ă��Ă������I�ɏ����l��������悤�ɋL�q���Ă���
	// ���N�����A�ȉ������삷��O��opetask()���烊�Z�b�g�v���t���O������
	//   ���݂ɗ���̂ŁA������pktctrl��memset()��all�N���A����͕̂s��
	_ref_sckt = PKT_PORT_DISCONNECT;// �\�P�b�g�|�[�g���
	_ref_buf_type = 'S';			// S:���M�o�b�t�@�CB:�đ��o�b�t�@(�x�[�X)�CA:�đ��o�b�t�@(�ǉ�)
	_ref_send.commited = 0;			// �R�}���h�o�b�t�@�[�ɂ���f�[�^��
	_ref_send.sent = 0;				// �\�P�b�g�̑��M�ς݃o�b�t�@�[��
	_ref_send.res_wait = 0;			// �����҂�
	_ref_send.wait_seqno = 0;		// �����҂���SEQ
	_ref_recv.usNackRetryNum = 0;
	_ref_recv.flag = 0;
	pktctrl.start_up = 0;			// �N��������(RT���Z�f�[�^���M�s��)
	pktctrl.send.RT_LogSend = 0;	// RT���Z�f�[�^���M�t���O
	pktctrl.send.QR_LogSend = 0;	// QR�m��E����f�[�^���M�t���O
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	pktctrl.send.LANE_LogSend = 0;	// ���[�����j�^�f�[�^���M�t���O
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	pktctrl.send.RECEIPT_LogSend = 0;	// RT�̎��؃��O�f�[�^���M�t���O
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	pktctrl.f_wait_discon = 0;		// ���Z�b�g�ʒm��M�|�R�l�N�V�����ؒf���o�҂��^�C�}�N�����t���O
	lcdbm_reset_keepalive_status();	// �L�[�v�A���C�u���g���C�J�E���^���Z�b�g

	// PKT�o�b�t�@������
	PKTbuf_init();

	pktctrl.ready = 1;			// HIF�d����M����
}

//[]----------------------------------------------------------------------[]
///	@brief			AMANO_LCDPKT�̐���
//[]----------------------------------------------------------------------[]
///	@param[in]		snd_seqno	: ���M�V�[�P���X��(0�`999)
///	@param[out]		*p_trailer	: ���������p�P�b�g�f�[�^
///	@return			�p�P�b�g�̃f�[�^��
///	@note			�o�b�t�@�����O����Ă���R�}���h����p�P�b�g�f�[�^�𐶐����܂��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort PKTsnd_Create_Packet( ushort snd_seqno, PKT_TRAILER *p_trailer )
{
	ushort	len = 2;	// �ŏ���2byte�̓p�P�b�g�̃f�[�^�����i�[����G���A
	ushort	size = 0;
	ushort	limit = PKT_CMD_MAX - 2;
	ushort	num = 0;
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} crc16;

	memset( p_trailer, 0, sizeof(PKT_TRAILER) );
	memset( EncDecrypt_buff, 0, sizeof(EncDecrypt_buff) );

	do {
		len += size;
		limit -= size;

		switch( _ref_buf_type ){
			case 'A':
				size = PKTbuf_ReadReSendCommand( num++, &EncDecrypt_buff[len] );
				break;
			default:	// 'S'
				size = PKTbuf_ReadSendCommand( &EncDecrypt_buff[len], limit );
				if( size ){
					PKTbuf_SetReSendCommand( &EncDecrypt_buff[len], size );
				}
				break;
		}
	} while(size);

	len -= 2;
	// ���[�h�f�[�^����
	if( len ){
		*(ushort *)&EncDecrypt_buff[0] = len;
		len += 2;

		memcpy( &p_trailer->part.identifier, "AMANO_LCDPKT", sizeof(p_trailer->part.identifier) );
		intoas( p_trailer->part.seqno, snd_seqno, sizeof(p_trailer->part.seqno) );
		p_trailer->part.id = 0x00;
		p_trailer->part.kind = 0x10;
		p_trailer->part.blkno_h = 0;
		p_trailer->part.blkno_l = 1;	// �u���b�N����2byte(���E�̂��߂��̂悤�ɥ��)
		p_trailer->part.block_sts = 1;
		// �Í�������H
		if( pktctrl.EncDecrypt ){
			p_trailer->part.encrypt = 1;
			// �p�f�B���O�K�v�H
			if( len % 16 ){
				p_trailer->part.pad_size = (16 - (len % 16));
			}
			len += p_trailer->part.pad_size;
			// �Í������ăf�[�^�R�s�[
			len = EncryptWithKeyAndIV_NoPadding( CRYPT_KEY_RXMLCDPK, EncDecrypt_buff, (uchar*)p_trailer->part.data, (int)len );
			if( !len ){	// �Í���NG
				return 0;
			}
		}else{
			memcpy( p_trailer->part.data, EncDecrypt_buff, len );
		}

		// �d�����Z�o(�Í������K�v�ȏꍇ�A�Í�����ɎZ�o)
		p_trailer->part.data_size = ( (uchar *)&p_trailer->part.data[0] - (uchar *)&p_trailer->part.data_size );	// �w�b�_�T�C�Y
		p_trailer->part.data_size += len;	// �d����(�d�������g���܂�(�Í������̓p�f�B���O�T�C�Y���܂�))
		p_trailer->part.data_size += 2;		// CRC16(2byte)

		// CRC16�Z�o(little endian) ��little endian�̂܂܊i�[
		crc_ccitt( (ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16.us, (uchar)R_SHIFT );
		p_trailer->data[p_trailer->part.data_size-2] = crc16.uc.high;
		p_trailer->data[p_trailer->part.data_size-1] = crc16.uc.low;
	}

	return ((ushort)p_trailer->part.data_size);
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�f�[�^�pAMANO_LCDPKT�̐���
//[]----------------------------------------------------------------------[]
///	@param[in]		target		: �Ώێ��(1=RT���Z/1=DC_QR)
///	@param[in]		snd_seqno	: ���M�V�[�P���X��(0�`999)
///	@param[out]		*p_trailer	: ���������p�P�b�g�f�[�^
///	@return			�p�P�b�g�̃f�[�^��
///	@note			���O�f�[�^(RT���Z/DC_QR)����p�P�b�g�f�[�^�𐶐����܂��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/20<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort PKTsnd_Create_LogData_Packet( eSEND_TARGET target, ushort snd_seqno, PKT_TRAILER *p_trailer )
{
// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:24)�Ή�
//	int	len;
	int	len = 0;
// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:24)�Ή�
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} crc16;

	memset( p_trailer, 0, sizeof(PKT_TRAILER) );

	memcpy( &p_trailer->part.identifier, "AMANO_LCDPKT", sizeof(p_trailer->part.identifier) );
	intoas( p_trailer->part.seqno, snd_seqno, sizeof(p_trailer->part.seqno) );
	p_trailer->part.id = 0x00;			// �f�[�^���R�[�h(00H�Œ�)
	p_trailer->part.kind = 0x10;		// �d�����ʃR�[�h(10H=�p�P�b�g�g���C��(�Œ�))
	p_trailer->part.blkno_h = 0;		// �u���b�NNo.(���)
	p_trailer->part.blkno_l = 1;		// �u���b�NNo.(����)(1�Œ�)
	p_trailer->part.block_sts = 1;		// �u���b�N�X�e�[�^�X(1�Œ�)

	// �Í����O�̃R�}���h�쐬
	switch( target ){
		case TARGET_RT_PAY:
			memset( &log_pay_wk, 0, sizeof(lcdbm_cmd_pydt_0pd_t) );
			log_pay_wk.data_len = sizeof(lcdbm_cmd_pay_data_t);
			log_pay_wk.pay_dat.command.length = (ushort)(sizeof(lcdbm_cmd_pay_data_t) - 2);	// �R�}���h��(2Byte)����������
			log_pay_wk.pay_dat.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
			log_pay_wk.pay_dat.command.subid = LCDBM_CMD_SUBID_PAY_DATA;
			Ope_Log_TargetDataVoidRead( eLOG_RTPAY, &log_pay_wk.pay_dat.id, eLOG_TARGET_LCD, TRUE );
			Pay_Data_ID = log_pay_wk.pay_dat.id;	// ������M���̃`�F�b�N�p��ID��ۑ�

			// �Í�������H
			if( pktctrl.EncDecrypt ){
				// �Í������ăf�[�^�R�s�[
				len = (ushort)(2 + log_pay_wk.data_len + PADSIZE);	// �d����(2Byte)�{RT���Z�f�[�^���{�p�f�B���O�T�C�Y
				// ����:log_pay_wk/�o��:p_trailer->part.data
				len = EncryptWithKeyAndIV_NoPadding(CRYPT_KEY_RXMLCDPK, (uchar*)&log_pay_wk, (uchar*)p_trailer->part.data, (int)len);
				if( !len ){	// �Í���NG
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					return 0;
				}
				p_trailer->part.encrypt = 1;	// �Í�������
				p_trailer->part.pad_size = PADSIZE;
			}
			// �Í����Ȃ��H
			else{
				len = (ushort)(2 + log_pay_wk.data_len);
				memcpy( p_trailer->part.data, &log_pay_wk, len );
			}
			break;

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		case TARGET_RT_RECEIPT:
			memset( &log_receipt_wk, 0, sizeof(lcdbm_cmd_receiptdt_0pd_t) );
			log_receipt_wk.data_len = sizeof(lcdbm_cmd_receipt_data_t);
			log_receipt_wk.receipt_dat.command.length = (ushort)(sizeof(lcdbm_cmd_receipt_data_t) - 2);	// �R�}���h��(2Byte)����������
			log_receipt_wk.receipt_dat.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
			log_receipt_wk.receipt_dat.command.subid = LCDBM_CMD_SUBID_RECEIPT_DATA;
			Ope_Log_TargetDataVoidRead( eLOG_RTRECEIPT, &log_receipt_wk.receipt_dat.id, eLOG_TARGET_LCD, TRUE );
			Receipt_Data_ID = log_receipt_wk.receipt_dat.id;	// ������M���̃`�F�b�N�p��ID��ۑ�

			// �Í�������H
			if ( pktctrl.EncDecrypt ) {
				// �Í������ăf�[�^�R�s�[
				len = (ushort)(2 + log_receipt_wk.data_len + RECEIPT_PADSIZE);	// �d����(2Byte)�{RT�̎��؃f�[�^���{�p�f�B���O�T�C�Y
				// ����:log_receipt_wk/�o��:p_trailer->part.data
				len = EncryptWithKeyAndIV_NoPadding(CRYPT_KEY_RXMLCDPK, (uchar*)&log_receipt_wk, (uchar*)p_trailer->part.data, (int)len);
				if ( !len ) {	// �Í���NG
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					return 0;
				}
				p_trailer->part.encrypt = 1;	// �Í�������
				p_trailer->part.pad_size = RECEIPT_PADSIZE;
			}
			// �Í����Ȃ��H
			else {
				len = (ushort)(2 + log_receipt_wk.data_len);
				memcpy(p_trailer->part.data, &log_receipt_wk, len);
			}
			break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		case TARGET_DC_QR:
			memset( &log_qr_wk, 0, sizeof(lcdbm_cmd_QR_conf_can_t) );
			log_qr_wk.data_len = sizeof(lcdbm_cmd_QR_conf_can_t);
			log_qr_wk.qr_dat.command.length = (ushort)(sizeof(lcdbm_cmd_QR_conf_can_t) - 2);	// �R�}���h��(2Byte)����������
			log_qr_wk.qr_dat.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
			log_qr_wk.qr_dat.command.subid = LCDBM_CMD_SUBID_QR_CONF_CAN;
			Ope_Log_TargetDataVoidRead( eLOG_DC_QR, &log_qr_wk.qr_dat.id, eLOG_TARGET_LCD, TRUE );
			QR_Data_ID = log_qr_wk.qr_dat.id;	// ������M���̃`�F�b�N�p��ID��ۑ�

			// �Í�������H
			if( pktctrl.EncDecrypt ){
				// �Í������ăf�[�^�R�s�[
				len = (ushort)(2 + log_qr_wk.data_len + QR_PADSIZE);	// �d����(2Byte)�{RT���Z�f�[�^���{�p�f�B���O�T�C�Y
				// ����:log_qr_wk/�o��:p_trailer->part.data
				len = EncryptWithKeyAndIV_NoPadding(CRYPT_KEY_RXMLCDPK, (uchar*)&log_qr_wk, (uchar*)p_trailer->part.data, (int)len);
				if( !len ){	// �Í���NG
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 5 );
#endif
// �����p (e)
					return 0;
				}
				p_trailer->part.encrypt = 1;	// �Í�������
				p_trailer->part.pad_size = QR_PADSIZE;
			}
			// �Í����Ȃ��H
			else{
				len = (ushort)(2 + log_qr_wk.data_len);
				memcpy( p_trailer->part.data, &log_qr_wk, len );
			}
			break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		case TARGET_DC_LANE:
			memset( &log_lane_wk, 0, sizeof(lcdbm_cmd_lane_t) );
			log_lane_wk.data_len = sizeof(lcdbm_cmd_lane_t);
			log_lane_wk.lane_dat.command.length = (ushort)(sizeof(lcdbm_cmd_lane_t) - 2);	// �R�}���h��(2Byte)����������
			log_lane_wk.lane_dat.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
			log_lane_wk.lane_dat.command.subid = LCDBM_CMD_SUBID_LANE;
			Ope_Log_TargetDataVoidRead( eLOG_DC_LANE, &log_lane_wk.lane_dat.id, eLOG_TARGET_LCD, TRUE );
			LANE_Data_ID = log_lane_wk.lane_dat.id;	// ������M���̃`�F�b�N�p��ID��ۑ�

			// �Í�������H
			if( pktctrl.EncDecrypt ){
				// �Í������ăf�[�^�R�s�[
				len = (ushort)(2 + log_lane_wk.data_len + LANE_PADSIZE);	// �d����(2Byte)�{RT���Z�f�[�^���{�p�f�B���O�T�C�Y
				// ����:log_lane_wk/�o��:p_trailer->part.data
				len = EncryptWithKeyAndIV_NoPadding(CRYPT_KEY_RXMLCDPK, (uchar*)&log_lane_wk, (uchar*)p_trailer->part.data, (int)len);
				if( !len ){	// �Í���NG
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					return 0;
				}
				p_trailer->part.encrypt = 1;	// �Í�������
				p_trailer->part.pad_size = LANE_PADSIZE;
			}
			// �Í����Ȃ��H
			else{
				len = (ushort)(2 + log_lane_wk.data_len);
				memcpy( p_trailer->part.data, &log_lane_wk, len );
			}
			break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

		default:
			break;
	}

	// �p�P�b�g�f�[�^�����p�P�b�g�f�[�^���`CRC16(�f�[�^���͈Í�����̒���)
	p_trailer->part.data_size = 26;		// �w�b�_���T�C�Y
	p_trailer->part.data_size += len;	// �Í�����̃f�[�^���T�C�Y���Z
	p_trailer->part.data_size += 2;		// CRC16(2byte)�T�C�Y���Z

	// CRC16�Z�o(little endian) ��little endian�̂܂܊i�[
	crc_ccitt( (ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16.us, (uchar)R_SHIFT );
	p_trailer->data[p_trailer->part.data_size-2] = crc16.uc.high;
	p_trailer->data[p_trailer->part.data_size-1] = crc16.uc.low;

	return ((ushort)p_trailer->part.data_size);
}

//[]----------------------------------------------------------------------[]
///	@brief			LCD�ʐM����Ҳݏ���
//[]----------------------------------------------------------------------[]
///	@param			�Ȃ�
///	@return			�Ȃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/10/31<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	PktTask( void )
{
	ushort	data_size;
	MsgBuf	*msb;		// ��Mү�����ޯ̧�߲��
	MsgBuf	msg;		// ��Mү���ފi�[�ޯ̧
	uchar	rst_cause;
	char	err_code;

	// ������
	pkt_init();

	// �ݒ�擾(IP/�|�[�g)
	GetSetting();

	// �ڑ��R�}���h�J�n�^�C�}���N��
	Lagtim( PKTTCBNO, PKT_TIM_CONNECT, 3*50 );	// 3�b

	// �ڑ��G���[�K�[�h�^�C�}���N��
	Lagtim( PKTTCBNO, PKT_TIM_ERR_GUARD, pktctrl.err_guard_tm*50 );

	// ================ //
	// ���C�����[�v
	// ================ //
	while( 1 ){
		taskchg( IDLETSKNO );

		if( ( msb = GetMsg( PKTTCBNO ) ) == NULL ){
			continue;		// ��Mү���ނȂ��� continue�i���������ؑցj
		}

		// ��M���b�Z�[�W����
		memcpy( &msg ,msb ,sizeof(MsgBuf) );				// ����ү���ށ���Mү����
		FreeBuf( msb );										// ��Mү�����ޯ̧�J��

		// ------------------ //
		// �C�x���g������
		// ------------------ //
		switch( msg.msg.command ){

			// �ڑ�����(�񓯊�)
			case PKT_SOC_CON:
				// �R�l�N�V���������҂��^�C�}���~
				Lagcan( PKTTCBNO, PKT_TIM_CON_RES_WT );
				// �ڑ��G���[�K�[�h�^�C�}���~
				Lagcan( PKTTCBNO, PKT_TIM_ERR_GUARD );
				// �܂��͊�{�ݒ�v���𑗐M����
				PKTcmd_request_config();
				// �����ł����Ă����M�������s�����߂�1�b�^�C�}�J�n����
				Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
				// �L�[�v�A���C�u�^�C�}�J�E���^���Z�b�g
				RESET_KEEPALIVE_COUNT;
				// �L�[�v�A���C�u���g���C�J�E���^���Z�b�g
				lcdbm_reset_keepalive_status();
				// �ʐM��Ԃ��X�V
				_ref_sckt = PKT_PORT_ESTABLISH;
				// �p�P�b�g�����҂����ԃ^�C�}��~
				Lagcan( PKTTCBNO, PKT_TIM_RES_WT );
				_ref_send.res_wait = 0;
				// �ʐM�s�Ǌ֘A�G���[�S����
				pkt_can_all_com_err();
				break; // ���̂܂ܑ��M�����ɂ��邽�߂�break

			// �ڑ��R�}���h�J�n�^�C�}�^�C���A�E�g
			case TIMEOUT_CONNECT:
				if( PktStartUpStsGet() < 1 ){	// OPE�^�X�N������������
					// �ڑ��R�}���h�J�n�^�C�}���ċN��(OPE�^�X�N�����������܂ő҂����킹��)
					Lagtim( PKTTCBNO, PKT_TIM_CONNECT, 50 );	// 1�b
					continue;
				}
				// no break
			// �R�l�N�V�������M�Ԋu�^�C�}�^�C���A�E�g
			case TIMEOUT_CON_INTVL:
				// ���ɐڑ���
				if( _ref_sckt == PKT_PORT_CONNECTING ){
					;
				}
				else {
					if( PKT_Socket != -1 ){
						// �\�P�b�g�����
						if( -1 == tfClose( PKT_Socket ) ){
							pkt_ksg_err_getreg( PKT_Socket, ERR_TKLSLCD_KSG_CLS, 2 );
						}
						PKT_Socket = -1;
					}
					// �\�P�b�g�ݒ�
					PKT_Socket = PktSocCfg();
					// �\�P�b�g�ݒ�OK�H
					if( PKT_Socket != -1 ){
						// �R�l�N�V�����ڑ��v��OK
						if( -1 != pkt_send_req_con() ){
							_ref_sckt = PKT_PORT_CONNECTING;			// �ڑ���
							// �R�l�N�V���������҂��^�C�}���N��
							Lagtim( PKTTCBNO, PKT_TIM_CON_RES_WT, (ushort)pktctrl.con_res_tm*50 );
						}
						// �R�l�N�V�����ڑ��v��NG
						else{
							// �R�l�N�V�������M�Ԋu�^�C�}���N��
							Lagtim( PKTTCBNO, PKT_TIM_CON_INTVL, (ushort)pktctrl.con_snd_intvl*50 );
							err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_COMM_FAIL, 1, 0, 0, NULL );
							queset( OPETCBNO, LCD_COMMUNICATION_CUTTING, 0, NULL );
						}
					}
					// �\�P�b�g�ݒ�NG�H
					else{
						// �R�l�N�V�������M�Ԋu�^�C�}���N��
						Lagtim( PKTTCBNO, PKT_TIM_CON_INTVL, (ushort)pktctrl.con_snd_intvl*50 );
						err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_COMM_FAIL, 1, 0, 0, NULL );
						queset( OPETCBNO, LCD_COMMUNICATION_CUTTING, 0, NULL );
					}
				}
				continue;
				break;

			// �L�[�v�A���C�u�^�C���A�E�g
			// 1�b���Ƀ`�F�b�N���s���A�L�[�v�A���C�u���M�Ԋu(�ݒ臂48-0019)�ɓ��B�������{�ݒ�v�����M
			// �L�[�v�A���C�u���M�Ԋu�����B�����̓L�[�v�A���C�u����(�ݒ臂48-0019=0)���͑��M�������s��
			// �A��1�b���̃`�F�b�N���Ƀ��Z�b�g�v�����������ꍇ�̓��Z�b�g�v����D�悵�čs��
			case TIMEOUT_KEEPALIVE:
				if( PktResetReqFlgRead() ){	// ���Z�b�g�v������
					PKTcmd_notice_ope( LCDBM_OPCD_RST_REQ, 0 );
					Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
					RESET_KEEPALIVE_COUNT;
					// �L�[�v�A���C�u���g���C�J�E���^���Z�b�g
					lcdbm_reset_keepalive_status();
					break;	// ���M������
				}
				// �L�[�v�A���C�u�L���H
				if( pktctrl.kpalv_snd_intvl ){
					// �L�[�v�A���C�u���s�H
				if( ++pktctrl.keepalive_timer >= pktctrl.kpalv_snd_intvl ){
						// ���g���C�`�F�b�NOK�H
						if( lcdbm_check_keepalive_status( pktctrl.kpalv_rtry_cnt ) ){
							// ��{�ݒ�v�����M�v��
							PKTcmd_request_config();
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
							RESET_KEEPALIVE_COUNT;
						}
						// ���g���C�I�[�o�[�H
						else{
							// �L�[�v�A���C�u�^�C�}�[���~
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
							// �L�[�v�A���C�u���g���C�J�E���^���Z�b�g
							lcdbm_reset_keepalive_status();
							// �đ��o�t�@�N���A
							PKTbuf_ClearReSendCommand();
							rst_cause = ERR_TKLSLCD_KPALV_RTY_OV;
							queset( PKTTCBNO, PKT_RST_PRT, 1, &rst_cause );
						}
						continue;
					}
				}
				// �����ł����Ă����M�������s�����߂�1�b�^�C�}�ĊJ����
				Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
				break;	// ���M������

			// �|�[�g���Z�b�g�v��
			case PKT_RST_PRT:
				_ref_recv.flag = 0;
				if( PktStartUpStsGet() >= 2 ){	// �N�������ʒm��M�ς�(OPE�^�X�N����������/RT���Z�f�[�^���M��)
					PktStartUpStsChg( 1 );	// �N���ʒm��M��(OPE�^�X�N����������/RT���Z�f�[�^���M�s��)
				}
				// no break;
			// �R�l�N�V���������҂��^�C�}�^�C���A�E�g
			case TIMEOUT_CON_RES_WT:
				if( msg.msg.command == PKT_RST_PRT ){
					err_code = (char)msg.msg.data[0];
				}else if( msg.msg.command == TIMEOUT_CON_RES_WT ){
					err_code = (char)ERR_TKLSLCD_CON_RES_TO;
				}
				if(( msg.msg.command == PKT_RST_PRT ) &&		// �|�[�g���Z�b�g�v��
				   (( err_code == ERR_TKLSLCD_RMT_RST ) ||		// ���肩���Reset
				    ( err_code == ERR_TKLSLCD_RMT_DSC )) &&		// ���肩��̐ؒf
				    ( pktctrl.f_wait_discon )){					// ���Z�b�g�ʒm��M��
					// ���Z�b�g�ʒm��M��̃R�l�N�V�����ؒf�̓G���[�o�^���Ȃ�
					Lagcan( PKTTCBNO, PKT_TIM_DISCON_WAIT );	// �R�l�N�V�����ؒf���o�҂��^�C�}��~
					pktctrl.f_wait_discon = 0;					// �R�l�N�V�����ؒf���o�҂��^�C�}�N�����t���OOFF
				}else{
					if( !LagChkExe( PKTTCBNO, PKT_TIM_ERR_GUARD ) ){	// �ڑ��G���[�K�[�h�^�C�}��~���H
						// �ڑ��G���[�K�[�h�^�C�}���쒆�̓G���[�o�^���Ȃ�
						if(( err_code != ERR_TKLSLCD_KSG_SND ) &&	// ����M�G���[�͌��o���ɓo�^�ς�
						   ( err_code != ERR_TKLSLCD_KSG_RCV )){
							err_chk2( ERRMDL_TKLSLCD, err_code, 1, 0, 0, NULL );
// MH810105(S) R.Endo 2021/12/13 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
#if DEBUG_JOURNAL_PRINT
							if ( err_code == ERR_TKLSLCD_RMT_DSC ) {
								// E2603�������_�̃f�o�b�O��
								pkt_socket_debug_print();
							}
#endif
// MH810105(E) R.Endo 2021/12/13 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
						}
						err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_COMM_FAIL, 1, 0, 0, NULL );
					}
				}
				// OPE�ɐؒf��ʒm
				queset( OPETCBNO, LCD_COMMUNICATION_CUTTING, 0, NULL );
				// �L�[�v�A���C�u�^�C�}�[���~
				Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
				if( _ref_send.res_wait ){	// �p�P�b�g�����҂�
					// �p�P�b�g�����҂����ԃ^�C�}��~
					Lagcan( PKTTCBNO, PKT_TIM_RES_WT );
					_ref_send.res_wait = 0;
				}
				if( pktctrl.send.RT_LogSend ){	// ���O�f�[�^������
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					//  ���O�����f�[�^�҂�
					if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  RT���Z�����f�[�^�҂��^�C�}��~
					}
					pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
				}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				if ( pktctrl.send.RECEIPT_LogSend ) {	// ���O�f�[�^������
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					//  ���O�����f�[�^�҂�
					if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
						Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	// RT�̎��؃f�[�^�����҂��^�C�}��~
					}
					pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
				}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				if( pktctrl.send.QR_LogSend ){	// ���O�f�[�^������
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 6 );
#endif
// �����p (e)
					//  ���O�����f�[�^�҂�
					if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  QR�m��E��������f�[�^�҂��^�C�}��~
					}
					pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
				}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				if( pktctrl.send.LANE_LogSend ){	// ���O�f�[�^������
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
					//  ���O�����f�[�^�҂�
					if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_DC_LANE_RES );	//  QR�m��E��������f�[�^�҂��^�C�}��~
					}
					pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
				}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				if( LagChkExe(PKTTCBNO, PKT_TIM_RTPAY_RESND) ){	//  �đ��E�F�C�g�^�C�}���쒆
					// �đ��E�F�C�g�^�C�}���쒆�́upktctrl.send.LogSend = 0�v���u���O�������M�v
					Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RESND );	//  �đ��E�F�C�g�^�C�}��~
				}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				if ( LagChkExe(PKTTCBNO, PKT_TIM_RTRECEIPT_RESND) ) {	//  �đ��E�F�C�g�^�C�}���쒆
					// �đ��E�F�C�g�^�C�}���쒆�́upktctrl.send.LogSend = 0�v���u���O�������M�v
					Lagcan(PKTTCBNO, PKT_TIM_RTRECEIPT_RESND);	//  �đ��E�F�C�g�^�C�}��~
				}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				if( LagChkExe(PKTTCBNO, PKT_TIM_DC_QR_RESND) ){	//  �đ��E�F�C�g�^�C�}���쒆
					// �đ��E�F�C�g�^�C�}���쒆�́upktctrl.send.LogSend = 0�v���u���O�������M�v
					Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RESND );	//  �đ��E�F�C�g�^�C�}��~
				}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				if( LagChkExe(PKTTCBNO, PKT_TIM_DC_LANE_RESND) ){	//  �đ��E�F�C�g�^�C�}���쒆
					// �đ��E�F�C�g�^�C�}���쒆�́upktctrl.send.LogSend = 0�v���u���O�������M�v
					Lagcan( PKTTCBNO, PKT_TIM_DC_LANE_RESND );	//  �đ��E�F�C�g�^�C�}��~
				}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				if( PKT_Socket != -1 ){
					// �\�P�b�g�����
					if( -1 == tfClose( PKT_Socket ) ){
						pkt_ksg_err_getreg( PKT_Socket, ERR_TKLSLCD_KSG_CLS, 2 );
					}
					PKT_Socket = -1;
				}
				// �R�l�N�V�������M�Ԋu�^�C�}���N��
				Lagtim( PKTTCBNO, PKT_TIM_CON_INTVL, (ushort)pktctrl.con_snd_intvl*50 );
				_ref_sckt = PKT_PORT_DISCONNECT;
				continue;
				break;

			// �R�}���h���M�v��
			case PKT_SND_PRT:
				if (_ref_send.commited == _ref_send.sent) {
					// �f�[�^���M���I�����Ƃ��A���̑��M�����s
					_ref_send.commited += 1;
					if( _ref_send.commited > 9999 ){
						_ref_send.commited = 0;
					}
				}
				break;	// ���M������

			// �񓯊���M�C�x���g
			case PKT_SOC_RCV:
				if (0 == pktctrl.ready) {	// not ready�Ȃ�ۑ�������OK��Ԃ��i�ǂݎ̂āj
					// while()�ɓ���O��pkt_init()�ŃZ�b�g���Ă���̂Œʂ�Ȃ��͂�
					break;	// ���M������
				}
				PktTcpRecv( PKT_Socket );
				break;	// ���M������

			// �p�P�b�g�����҂����ԃ^�C���A�E�g
			case TIMEOUT_RES_WT:
				if( _ref_send.res_wait ){	// �����҂��H
					_ref_send.res_wait = 0;
					if( ++_ref_send.rtry_cnt > pktctrl.snd_rtry_cnt ){	// ���M���g���C�I�[�o�[
						_ref_send.rtry_cnt = 0;	// ���g���C�J�E���^�N���A
						// ���M�����̂�RT���O�f�[�^�H
						if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
							// �đ��𒆒f����RT���Z�f�[�^�ȊO�̑��M���ɍs��
							PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
							Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
							pktctrl.send.RT_LogSend = LOG_PHASE_RESEND_CANCEL;	// ���O�f�[�^�đ����f
							break;	// ���M������
						}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
						// ���M�����̂�RT�̎��؃��O�f�[�^�H
						else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ) {
							// �đ��𒆒f����RT�̎��؃f�[�^�ȊO�̑��M���ɍs��
							PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
							Ope_Log_UndoTargetDataVoidRead(eLOG_RTRECEIPT, eLOG_TARGET_LCD);	// ��ǂ݂����ɖ߂�
							pktctrl.send.RECEIPT_LogSend = LOG_PHASE_RESEND_CANCEL;	// ���O�f�[�^�đ����f
							break;	// ���M������
						}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
						// ���M�����̂�QR���O�f�[�^�H
						else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
							// �đ��𒆒f����RT���Z�f�[�^�ȊO�̑��M���ɍs��
							PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
							Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
							pktctrl.send.QR_LogSend = LOG_PHASE_RESEND_CANCEL;	// ���O�f�[�^�đ����f
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 7 );
#endif
// �����p (e)
							break;	// ���M������
						}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
						// ���M�����̂̓��[�����j�^���O�f�[�^�H
						else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
							// �đ��𒆒f����RT���Z�f�[�^�ȊO�̑��M���ɍs��
							PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
							Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
							pktctrl.send.LANE_LogSend = LOG_PHASE_RESEND_CANCEL;	// ���O�f�[�^�đ����f
							break;	// ���M������
						}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
						// ���M�����̂͒ʏ�f�[�^�H
						else{
							if( ( pktctrl.rsnd_rtry_ov ) &&	// ���g���C�I�[�o�[���̍đ��M����
								( PKTbuf_CheckReSendCommand() != LCDBM_CMD_NOTIFY_CONFIG )){	// ��{�ݒ�v���łȂ�
								;	// �đ��M(��{�ݒ�v���̓��g���C�I�[�o�[���̍đ��M�ΏۊO�Ƃ���)
							}else{
								err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_RTRY_OV, 2, 0, 0, NULL );
								_ref_send.sent += 1;	// ���M�ς݂Ƃ���
								PKTbuf_ClearReSendCommand();	// �đ��o�b�t�@�N���A
								// RT���Z�f�[�^�đ����f���H
								if( pktctrl.send.RT_LogSend == LOG_PHASE_RESEND_CANCEL ){
									// RT���Z�f�[�^�đ����f����������RT���Z�f�[�^�đ��J�n
									pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;
									_ref_buf_type = 'S';
								}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
								// RT�̎��؃f�[�^�đ����f���H
								else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_RESEND_CANCEL ) {
									// RT�̎��؃f�[�^�đ����f����������RT�̎��؃f�[�^�đ��J�n
									pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;
									_ref_buf_type = 'S';
								}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
								// QR�m��E����f�[�^�đ����f���H
								else if( pktctrl.send.QR_LogSend == LOG_PHASE_RESEND_CANCEL ){
									// QR�m�����f�[�^�đ����f����������QR�m��E����f�[�^�đ��J�n
									pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;
									_ref_buf_type = 'S';
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 21 );
#endif
// �����p (e)
								}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
								// ���[�����j�^�f�[�^�đ����f���H
								else if( pktctrl.send.LANE_LogSend == LOG_PHASE_RESEND_CANCEL ){
									// QR�m�����f�[�^�đ����f���������ă��[�����j�^�f�[�^�đ��J�n
									pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;
									_ref_buf_type = 'S';
								}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
								break;	// ���M������
							}
						}
					}
					// �đ�(�����ɗ���̂̓��g���C�I�[�o�O�ƃ��g���C�I�[�o�����đ�����ꍇ)

					// ���M�����̂�RT���Z�f�[�^�H
					if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
						Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
						pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// RT���Z�f�[�^�����M��Ԃɖ߂�
						_ref_buf_type = 'S';
					}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					// ���M�����̂�RT�̎��؃f�[�^�H
					else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ) {
						Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// RT�̎��؃f�[�^�����M��Ԃɖ߂�
						_ref_buf_type = 'S';
					}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					// ���M�����̂�QR�m��E����f�[�^�H
					else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
						Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
						pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// QR�m��E����f�[�^�����M��Ԃɖ߂�
						_ref_buf_type = 'S';
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 1 );
#endif
// �����p (e)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					}
					// ���M�����̂̓��[�����j�^�f�[�^�H
					else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
						Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ��ǂ݂����ɖ߂�
						pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ���[�����j�^�f�[�^�����M��Ԃɖ߂�
						_ref_buf_type = 'S';
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					}else{
						_ref_buf_type = 'A';
					}
					break;	// ���M������
				}
				// �����҂��łȂ���Ζ߂�
				continue;
				break;

			// ���Z�����f�[�^�҂��^�C�}�^�C���A�E�g
			case TIMEOUT_RTPAY_RES:
				// ���O�����f�[�^�҂����H
				if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
// MH810100(S) K.Onodera  2020/04/27 #4135 �Ԕԃ`�P�b�g���X(DC-NET�ʐM�������ɕ����̊����F�ؓo�^�E����v���f�[�^�𑗐M���Ă��܂�)
//					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_PAY_RES_TO, 2, 0, 0, NULL );	// RT���Z�����f�[�^�҂��G���[(������������)
//					Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );		// ��ǂ݂����ɖ߂�
//					pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
//					// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
//					// _ref_buf_type�͕ύX���Ȃ�
					Lagtim( PKTTCBNO, PKT_TIM_RTPAY_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// �p�P�b�g�����f�[�^�҂��^�C�}�J�n
// MH810100(E) K.Onodera  2020/04/27 #4135 �Ԕԃ`�P�b�g���X(DC-NET�ʐM�������ɕ����̊����F�ؓo�^�E����v���f�[�^�𑗐M���Ă��܂�)
					break;	// ���M������
				}
				// �����҂��łȂ���Ζ߂�
				continue;
				break;

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
			// �̎��؃f�[�^�����҂��^�C�}�^�C���A�E�g
			case TIMEOUT_RTRECEIPT_RES:
				// ���O�����f�[�^�҂����H
				if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
					Lagtim(PKTTCBNO, PKT_TIM_RTRECEIPT_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// �p�P�b�g�����f�[�^�҂��^�C�}�J�n
					break;	// ���M������
				}
				// �����҂��łȂ���Ζ߂�
				continue;
				break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

			// QR�m���������f�[�^�҂��^�C�}�^�C���A�E�g
			case TIMEOUT_DC_QR_RES:
				// ���O�����f�[�^�҂����H
				if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
// MH810100(S) K.Onodera  2020/04/27 #4135 �Ԕԃ`�P�b�g���X(DC-NET�ʐM�������ɕ����̊����F�ؓo�^�E����v���f�[�^�𑗐M���Ă��܂�)
//					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_QR_RES_TO, 2, 0, 0, NULL );	// QR�m���������f�[�^�҂��G���[(������������)
//					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );		// ��ǂ݂����ɖ߂�
//					pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ���O�f�[�^�����M��Ԃɖ߂�
//					// ���Z�����f�[�^�҂����͑��f�[�^���M���̉\��������̂�
//					// _ref_buf_type�͕ύX���Ȃ�
					Lagtim( PKTTCBNO, PKT_TIM_DC_QR_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// �p�P�b�g�����f�[�^�҂��^�C�}�J�n
// MH810100(E) K.Onodera  2020/04/27 #4135 �Ԕԃ`�P�b�g���X(DC-NET�ʐM�������ɕ����̊����F�ؓo�^�E����v���f�[�^�𑗐M���Ă��܂�)
					break;	// ���M������
				}
				// �����҂��łȂ���Ζ߂�
				continue;
				break;

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			// ���[�����j�^�����f�[�^�҂��^�C�}�^�C���A�E�g
			case TIMEOUT_DC_LANE_RES:
				// ���O�����f�[�^�҂����H
				if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
					Lagtim( PKTTCBNO, PKT_TIM_DC_LANE_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// �p�P�b�g�����f�[�^�҂��^�C�}�J�n
					break;	// ���M������
				}
				// �����҂��łȂ���Ζ߂�
				continue;
				break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

			// �đ��E�F�C�g�^�C�}�^�C���A�E�g
			case TIMEOUT_RTPAY_RESND:
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
			case TIMEOUT_RTRECEIPT_RESND:
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
			case TIMEOUT_DC_QR_RESND:
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			case TIMEOUT_DC_LANE_RESND:
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				;	// ��ǂ݁E���O�f�[�^�����M��Ԗ߂��ς�
				break;	// ���M������

			// �R�l�N�V�����ؒf���o�҂��^�C�}�^�C���A�E�g
			case TIMEOUT_DISCON_WT:
				pktctrl.f_wait_discon = 0;	// �R�l�N�V�����ؒf���o�҂��^�C�}�N�����t���OOFF
				break;	// ���M������

			// �ڑ��G���[�K�[�h�^�C�}�^�C���A�E�g
			case TIMEOUT_ERR_GUARD:
				break;	// ���M������

			default:
				continue;
				break;

		}

		// -------------------- //
		// �p�P�b�g���M����
		// -------------------- //

		// ���M�\��ԁH
		if( (_ref_sckt == PKT_PORT_ESTABLISH) &&
		    (_ref_send.res_wait == 0)	// ACK/NAK�҂�����Ȃ��H
		){
			taskchg( IDLETSKNO );
			// ***************
			// ���M�f�[�^����
			// ***************

			// ���O�f�[�^�̑��M
			if( ( PktStartUpStsGet() >= 2 )	&&	// �N�������ʒm��M�ς�(OPE�^�X�N����������/���O�f�[�^���M��)
				( _ref_buf_type != 'A' )	){	// �p�P�b�g�đ��łȂ�

			   // RT���Z�f�[�^�����M����H
				if( Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) && ( pktctrl.send.RT_LogSend == LOG_PHASE_IDLE ) && !LagChkExe(PKTTCBNO, PKT_TIM_RTPAY_RESND) ){
					data_size = PKTsnd_Create_LogData_Packet( TARGET_RT_PAY, pktctrl.snd_seqno, &snd_trailer );

					// �p�P�b�g����OK�H
					if( data_size ){
						// ***************
						//    ���M�v��
						// ***************
						pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
						// ���O�f�[�^�̉����҂��^�C�}��ACK��M�ŊJ�n
						pktctrl.send.RT_LogSend = LOG_PHASE_WAIT_ACK;

						// ���M���Ă���C���N�������g����
						pktctrl.snd_seqno++;
						if( pktctrl.snd_seqno > 999 ){
							pktctrl.snd_seqno = 1;
						}
						if( pktctrl.kpalv_snd_intvl ){	// �L�[�v�A���C�u�L��
							// �L�[�v�A���C�u�^�C�}���Z�b�g
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
							RESET_KEEPALIVE_COUNT;
						}else{	// �L�[�v�A���C�u����(�ȉ��A�ی�)
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
						}
					}
					// �p�P�b�g����NG�H
					else{
						// �p�P�b�g����NG�̃��O�͑��M�ς݂Ƃ���(���̃��[�g�͒ʂ�Ȃ��͂�)
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
						Pay_Data_ID = 0;
						QR_Data_ID = 0;
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
						Receipt_Data_ID = 0;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					}
					continue;	// RT���Z�f�[�^���M��D�悳����ׁA�p�P�b�g����NG����continue
				}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				// RT�̎��؃f�[�^�����M����H
				else if ( Ope_Log_UnreadCountGet( eLOG_RTRECEIPT, eLOG_TARGET_LCD ) && (pktctrl.send.RECEIPT_LogSend == LOG_PHASE_IDLE) && !LagChkExe(PKTTCBNO, PKT_TIM_RTRECEIPT_RESND) ) {
					data_size = PKTsnd_Create_LogData_Packet( TARGET_RT_RECEIPT, pktctrl.snd_seqno, &snd_trailer );

					// �p�P�b�g����OK�H
					if ( data_size ) {
						// ***************
						//    ���M�v��
						// ***************
						pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
						// ���O�f�[�^�̉����҂��^�C�}��ACK��M�ŊJ�n
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_WAIT_ACK;

						// ���M���Ă���C���N�������g����
						pktctrl.snd_seqno++;
						if ( pktctrl.snd_seqno > 999 ) {
							pktctrl.snd_seqno = 1;
						}
						if ( pktctrl.kpalv_snd_intvl ) {	// �L�[�v�A���C�u�L��
							// �L�[�v�A���C�u�^�C�}���Z�b�g
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
							RESET_KEEPALIVE_COUNT;
						} else {	// �L�[�v�A���C�u����(�ȉ��A�ی�)
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
						}
					}
					// �p�P�b�g����NG�H
					else {
						// �p�P�b�g����NG�̃��O�͑��M�ς݂Ƃ���(���̃��[�g�͒ʂ�Ȃ��͂�)
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
						Pay_Data_ID = 0;
						QR_Data_ID = 0;
						Receipt_Data_ID = 0;
					}
					continue;	// RT�̎��؃f�[�^���M��D�悳����ׁA�p�P�b�g����NG����continue
				}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				// QR�m�����f�[�^�����M����H
				else if( Ope_Log_UnreadCountGet( eLOG_DC_QR, eLOG_TARGET_LCD ) && ( pktctrl.send.QR_LogSend == LOG_PHASE_IDLE ) && !LagChkExe(PKTTCBNO, PKT_TIM_DC_QR_RESND) ){
			    	// �p�P�b�g����
					data_size = PKTsnd_Create_LogData_Packet( TARGET_DC_QR, pktctrl.snd_seqno, &snd_trailer );

					// �p�P�b�g����OK�H
					if( data_size ){
						// ***************
						//    ���M�v��
						// ***************
						pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
						// ���O�f�[�^�̉����҂��^�C�}��ACK��M�ŊJ�n
						pktctrl.send.QR_LogSend = LOG_PHASE_WAIT_ACK;

						// ���M���Ă���C���N�������g����
						pktctrl.snd_seqno++;
						if( pktctrl.snd_seqno > 999 ){
							pktctrl.snd_seqno = 1;
						}
						if( pktctrl.kpalv_snd_intvl ){	// �L�[�v�A���C�u�L��
							// �L�[�v�A���C�u�^�C�}���Z�b�g
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
							RESET_KEEPALIVE_COUNT;
						}else{	// �L�[�v�A���C�u����(�ȉ��A�ی�)
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
						}
					}
					// �p�P�b�g����NG�H
					else{
						// �p�P�b�g����NG�̃��O�͑��M�ς݂Ƃ���(���̃��[�g�͒ʂ�Ȃ��͂�)
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
						Pay_Data_ID = 0;
						QR_Data_ID = 0;
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
						Receipt_Data_ID = 0;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 20 );
#endif
// �����p (e)
					}
					continue;	// RT���Z�f�[�^���M��D�悳����ׁA�p�P�b�g����NG����continue
			    }
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				// ���[�����j�^�f�[�^�����M����H
				else if( Ope_Log_UnreadCountGet( eLOG_DC_LANE, eLOG_TARGET_LCD ) && ( pktctrl.send.LANE_LogSend == LOG_PHASE_IDLE ) && !LagChkExe(PKTTCBNO, PKT_TIM_DC_LANE_RESND) ){
			    	// �p�P�b�g����
					data_size = PKTsnd_Create_LogData_Packet( TARGET_DC_LANE, pktctrl.snd_seqno, &snd_trailer );

					// �p�P�b�g����OK�H
					if( data_size ){
						// ***************
						//    ���M�v��
						// ***************
						pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
						// ���O�f�[�^�̉����҂��^�C�}��ACK��M�ŊJ�n
						pktctrl.send.LANE_LogSend = LOG_PHASE_WAIT_ACK;

						// ���M���Ă���C���N�������g����
						pktctrl.snd_seqno++;
						if( pktctrl.snd_seqno > 999 ){
							pktctrl.snd_seqno = 1;
						}
						if( pktctrl.kpalv_snd_intvl ){	// �L�[�v�A���C�u�L��
							// �L�[�v�A���C�u�^�C�}���Z�b�g
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
							RESET_KEEPALIVE_COUNT;
						}else{	// �L�[�v�A���C�u����(�ȉ��A�ی�)
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
						}
					}
					// �p�P�b�g����NG�H
					else{
						// �p�P�b�g����NG�̃��O�͑��M�ς݂Ƃ���(���̃��[�g�͒ʂ�Ȃ��͂�)
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// ���M�ς݂Ƃ���
						LANE_Data_ID = 0;
					}
					continue;	// RT���Z�f�[�^���M��D�悳����ׁA�p�P�b�g����NG����continue
				}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			}

			// ���O�f�[�^�ȊO�̃R�}���h�̑��M
			// (���O�f�[�^�̓R�}���h�o�b�t�@�ɃZ�b�g���Ȃ��̂�_ref_send.sent/commited�Ǘ��ΏۊO)
			if( _ref_send.sent != _ref_send.commited ){
				data_size = PKTsnd_Create_Packet( pktctrl.snd_seqno, &snd_trailer );
				switch( _ref_buf_type ){
					case 'A':
						_ref_buf_type = 'S';
						break;
					default:
						break;
				}
				// �f�[�^����OK
				if( data_size ){
					// ***************
					//    ���M�v��
					// ***************
					pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
					// ���M���Ă���C���N�������g����
					pktctrl.snd_seqno++;
					if( pktctrl.snd_seqno > 999 ){
						pktctrl.snd_seqno = 1;
					}
					if( pktctrl.kpalv_snd_intvl ){	// �L�[�v�A���C�u�L��
						// �L�[�v�A���C�u�^�C�}���Z�b�g
						Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
						RESET_KEEPALIVE_COUNT;
					}else{	// �L�[�v�A���C�u����(�ȉ��A�ی�)
						Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
					}
				}
				// ���M�o�b�t�@�Ƀf�[�^�Ȃ��H
				else{
					_ref_send.sent = _ref_send.commited;	// �S�đ��M�ς݂�(�t�F�C���Z�[�t)
				}
			}
		}
	}
}

const unsigned short tenkey_event_table[KeyDatMaxLCD][8] = {
	{KEY_TEN_CL,KEY_TEN_F5,	KEY_TEN_F4,	KEY_TEN_F3,	KEY_TEN_F2,	KEY_TEN_F1,	KEY_TEN9,	KEY_TEN8},
	{KEY_TEN7,	KEY_TEN6,	KEY_TEN5,	KEY_TEN4,	KEY_TEN3,	KEY_TEN2,	KEY_TEN1,	KEY_TEN0},
};
//[]----------------------------------------------------------------------[]
///	@brief		�e���L�[�{�^����ԃ`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]	tenkey		�e���L�[�{�^����Ԃւ̃|�C���^
///	@return		none
/// @note		LCD�����M�����e���L�[�{�^����Ԃ��`�F�b�N����
///				�ω��̂������e���L�[�{�^����ope�^�X�N�ɒʒm����
//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
static void pkt_tenkey_status_check( uchar *tenkey )
{
	uchar i,j;
	uchar wkbit;
	uchar on_off;

	for (i = 0; i < KeyDatMaxLCD; i++) {
		wkbit = 0x80;
		for (j = 0; j < 8; j++, wkbit>>=1) {
			if ((tenkey[i] & wkbit) != (key_dat_LCD[i].BYTE & wkbit)) {	// �ω�����
				// �ω��̂������e���L�[�{�^����Ԃ�ope�^�X�N�ɒʒm����
				if (tenkey[i] & wkbit) {
					on_off = 1;
				} else {
					on_off = 0;
				}
				queset( OPETCBNO, tenkey_event_table[i][j], 1, &on_off );
			}
// MH810100(S) Y.Yamauchi 2020/03/05 #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�
			// C�L�[��ON�͓_���p�ɏ�ɑ��M
			else if( (tenkey[i] & wkbit) && (tenkey_event_table[i][j] == KEY_TEN_CL) ){
				on_off = 1;
				queset( OPETCBNO, tenkey_event_table[i][j], 1, &on_off );
			}
// MH810100(E) Y.Yamauchi 2020/03/05 #3901 �o�b�N���C�g�̓_��/�������ݒ�ʂ�ɓ��삵�Ȃ�
		}
		key_dat_LCD[i].BYTE = tenkey[i];	// key_dat_LCD�X�V
	}

}

//[]----------------------------------------------------------------------[]
///	@brief		kasago�N���G���[���擾/�o�^����
//[]----------------------------------------------------------------------[]
///	@param[in]	fid		�\�P�b�g�f�B�X�N���v�^
///	@param[in]	code	�G���[�R�[�h(�e�֐�����ope_def.h�Œ�`����Ă���l)
///	@param[in]	knd		0:����/1:����/2:��������
///	@return		error	�G���[���
/// @note		kasago�N���G���[�������A�G���[���(bin)���擾���ēo�^����
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ulong pkt_ksg_err_getreg( int fid, char code, char knd )
{
	ulong	error;

	if( code == ERR_TKLSLCD_KSG_SOC ){	// socket()�G���[�H
		fid = TM_SOCKET_ERROR;
	}
	error = (ulong)tfGetSocketError( fid );		// �G���[���擾
	if(( code == ERR_TKLSLCD_KSG_CON ) &&		// connect()�̃G���[
	   (( error == TM_EINPROGRESS ) ||			// �m���u���b�L���O�����݂̐ڑ����s���܂��������Ă��Ȃ�
	    ( error == TM_EISCONN ))){				// tfGetSocketError���R�[�������܂ł̊Ԃɐڑ������������ꍇ
	    ;	// �G���[�����Ƃ��Ȃ��̂œo�^���Ȃ�
	}else if(( code == ERR_TKLSLCD_KSG_RCV ) &&	// recv()�̃G���[
			 ( error == TM_EWOULDBLOCK )){		// �m���u���b�L���O�œǂݍ��݉\�ȃf�[�^���Ȃ�
	    ;	// �G���[�����Ƃ��Ȃ��̂œo�^���Ȃ�
	}else{
		// �߂�l���G���[���(bin)�Ƃ��Ă��̂܂ܓo�^����
		err_chk2( ERRMDL_TKLSLCD, code, knd, 2, 0, &error );
	}

	return error;
}

//[]----------------------------------------------------------------------[]
///	@brief		�ʐM�s�Ǌ֘A�G���[�S��������
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		none
/// @note		�ʐM�s�Ǌ֘A�G���[��S�ĉ�������
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void pkt_can_all_com_err( void )
{
	char	i;

	for( i = 0; i < PKT_COMM_FAIL_MAX; i++ ){
		err_chk2( ERRMDL_TKLSLCD, i, 0, 0, 0, NULL );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		�����f�[�^�����M�t���O�擾
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		�����f�[�^�����M�t���O
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar pkt_get_restore_unsent_flg( void )
{
	return restore_unsent_flg;
}

//[]----------------------------------------------------------------------[]
///	@brief		�����f�[�^�����M�t���O�Z�b�g
//[]----------------------------------------------------------------------[]
///	@param[in]	�t���O���<br>
///				(0=�����f�[�^���M�ς� / 1=�����f�[�^���M�v(�N����) / 2=�����f�[�^���M�v(������))
///	@return		none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void pkt_set_restore_unsent_flg( uchar flg )
{
	restore_unsent_flg = flg;
}
// MH810100(E) K.Onodera  2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)

// MH810105(S) R.Endo 2021/12/13 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
#if DEBUG_JOURNAL_PRINT
//[]----------------------------------------------------------------------[]
///	@brief		socket�f�o�b�O�󎚏���
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		none
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void pkt_socket_debug_print( void )
{
	const int row_max = 4;

	T_FrmDebugData frm_debug_data;
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	struct clk_rec	wk_CLK_REC_debug;
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	ttSocketEntryPtr socket_entry_ptr;
	int i;
	int data_index;
	int null_count;

	// �N���A
	memset(&frm_debug_data, 0x00, sizeof(frm_debug_data));

	// ���ݓ���
	c_Now_CLK_REC_ms_Read(
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//		&frm_debug_data.wk_CLK_REC,
		&wk_CLK_REC_debug,
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		&frm_debug_data.wk_CLK_REC_msec);
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	// 7�o�C�g�̂�COPY
	memcpy( &frm_debug_data.wk_CLK_YMDHMS, &wk_CLK_REC_debug, sizeof(DATE_YMDHMS) );
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	// �W���[�i����
	frm_debug_data.prn_kind = J_PRI;

	// 1�s��(�\�P�b�g�I�[�v�����̌��ݒl�ƍő�l)
	frm_debug_data.first_line = TRUE;
	sprintf(frm_debug_data.data, "sock:%u/%u tcp_sock:%u/%u",
		tm_context(tvNumberOpenSockets), tm_context(tvMaxNumberSockets),
		tm_context(tvTcpVectAllocCount), tm_context(tvMaxTcpVectAllocCount));

	// 2�s�ڈȍ~(�e�\�P�b�g�̃X�e�[�^�X�t���O)
	data_index = 0;
	null_count = 0;
	for ( i = 0; i < tm_context(tvMaxNumberSockets); i++ ) {
		if ( (i % row_max) == 0 ) {
			if ( null_count != row_max ) {
				queset(PRNTCBNO, PREQ_DEBUG, sizeof(T_FrmDebugData), &frm_debug_data);
			}
			data_index = 0;
			null_count = 0;
			memset(frm_debug_data.data, 0x00, sizeof(frm_debug_data.data));
			frm_debug_data.first_line = FALSE;
		}
		socket_entry_ptr = tfSocketCheckValidLock(i);
		if ( socket_entry_ptr == TM_SOCKET_ENTRY_NULL_PTR ) {
			data_index += sprintf(&frm_debug_data.data[data_index], "%2d:---- ", i);
			null_count++;
		} else {
			data_index += sprintf(&frm_debug_data.data[data_index], "%2d:%04x ", i, socket_entry_ptr->socFlags);
		}
	}
	if ( null_count != row_max ) {
		queset(PRNTCBNO, PREQ_DEBUG, sizeof(T_FrmDebugData), &frm_debug_data);
	}
}
#endif
// MH810105(E) R.Endo 2021/12/13 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
