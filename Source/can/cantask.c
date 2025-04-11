/*[]----------------------------------------------------------------------[]*/
/*| can task control                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                             |*/
/*| Date         : 2012-01-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/

/*--- Include -------------------*/
#include	<string.h>
#include	<machine.h>
#include	"system.h"
#include	"mem_def.h"
#include	"message.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"can_api.h"
#include	"can_def.h"
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
#include	"mnt_def.h"
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
/*--- Prototype -----------------*/
static void can_ic_init(void);
static void canchk(MsgBuf *pmsg);
static void canrcv(void);
static void cansnd(void);
static void get_can_key_event(can_std_frame_t *dataframe);

/*--- Define --------------------*/
#define _GET_CAN()		&can_ctr

#define CAN_TERM_PODR(x, y)			CAN_TERM_PODR_PREPROC(x, y)
#define CAN_TERM_PODR_PREPROC(x, y)	(PORT ## x .PODR.BIT.B ## y)

#define	CAN_RES_TIME						(100/10)			// CAN_RES�M��High�p������:100ms
#define	CAN_RESP_TIME						(1000/10)			// �����҂�����:1s
#define	CAN_HEARTBEAT_TIME					10					// �n�[�g�r�[�g����:10s
#define	CAN_START_KEEP_ALIVE_TIMER(nid, t)	Lagtim(CANTCBNO, (unsigned char)(nid-NID_RXM), (unsigned short)((t)/20))	// �L�[�v�A���C�u�^�C�}�N��('t'ms)
#define	CAN_STOP_KEEP_ALIVE_TIMER(nid)		Lagcan(CANTCBNO, (unsigned char)(nid-NID_RXM))								// �L�[�v�A���C�u�^�C�}��~
#define	CAN_TIMEOUT_KEEP_ALIVE_02			(TIMEOUT+NID_RXF-NID_RXM)	// �L�[�v�A���C�u�^�C�}�^�C���A�E�g�C�x���g(NID_RXF)
#define ECSR_DELAYTIM	5										// ECSR�̃f�B���C�^�C���A�E�g�l100ms
#define CAN_TIMER_ECSR_DELAY	2										// ECSR���o�f�B���C�^�C�}��ID
#define	CAN_TIMEOUT_ECSR_DELAY			(TIMEOUT+CAN_TIMER_ECSR_DELAY)	// ECSR���o�f�B���C�^�C���A�E�g

typedef union{								// data[0]���͗p�\����
	unsigned char	byte;
	struct{
		unsigned char	command		:3;		// �R�}���h
		unsigned char	t			:1;		// �g�O���r�b�g(segment���M�̓x�Ƀr�b�g�𔽓])
		unsigned char	undefined	:2;		// ����`
		unsigned char	e			:1;		// 0:�V���O���]��, 1:�u���b�N�]��
		unsigned char	c			:1;		// 0:��segment����, 1:�ŏIsegment
	} bit;
} t_data0;

typedef struct {
	unsigned char	data[_MAX_LENGTH_TRANSFERT];	// �f�B�X�v���C���M�o�b�t�@
	unsigned short	count;
	unsigned short	countmax;
	unsigned short	readidx;
	unsigned short	writeidx;
} t_can_dispbuf;

/*--- Gloval Value --------------*/
t_can_transfer	transfer[SIDMAX];					// ����M�Ǘ����

static  struct _canctr {							// ���[�N�̈�
	t_can_quectrl	sndque[3];						// ���M�L���[
	t_can_quectrl	rcvque;							// ��M�L���[
	can_std_frame_t	tx_dataframe[32];				// ���M�f�[�^�t���[��(����)
	can_std_frame_t	ctrl_tx_dataframe[16];			// ���M�f�[�^�t���[��(CONTROL)
	can_std_frame_t	disp_tx_dataframe[0x7F+2];		// ���M�f�[�^�t���[��(DISPLAY)
	can_std_frame_t	rx_dataframe[128];				// ��M�f�[�^�t���[��
	t_can_dispbuf	dispbuf;						// �f�B�X�v���C���M�o�b�t�@
	unsigned char	can_net_stat[NID_CNT];			// �l�b�g���[�N���(CAN_STS_INIT:���������, CAN_STS_OPE:�I�y���[�V���i�����, CAN_STS_STOP:��~���)
} can_ctr;

static const struct {
	unsigned short	sid;
	t_can_quectrl	*sndque;
	can_std_frame_t	*dataframe;
	unsigned char	countmax;
} t_can_transfer_Initializer[SIDMAX] = {
	{0x00,	&can_ctr.sndque[0],	&can_ctr.tx_dataframe[0],		32},	// CAN_NETWORK
	{0x80,	NULL,				NULL,							0},		// CAN_EMERGENCY
	{0x100,	NULL,				NULL,							0},		// CAN_EVENT
	{0x180,	&can_ctr.sndque[1],	&can_ctr.ctrl_tx_dataframe[0],	16},	// CAN_CONTROL
	{0x200,	&can_ctr.sndque[2],	&can_ctr.disp_tx_dataframe[0],	0x7F+2},// CAN_DISPLAY
	{0x700,	&can_ctr.sndque[0],	&can_ctr.tx_dataframe[0],		32},	// CAN_HEARTBEAT
};

const unsigned short scan_event_table[KeyDatMax][8] = {
	{FNT_DIPSW1,FNT_DIPSW2,	FNT_DIPSW3,	FNT_DIPSW4,	FNT_YOBI1,	FNT_YOBI2,	FNT_YOBI3,	FNT_YOBI4},
	{KEY_TEN0,	KEY_TEN1,	KEY_TEN2,	KEY_TEN3,	KEY_TEN4,	KEY_TEN5,	KEY_TEN6,	KEY_TEN7},
	{KEY_TEN8,	KEY_TEN9,	KEY_TEN_F1,	KEY_TEN_F2,	KEY_TEN_F3,	KEY_TEN_F4,	KEY_TEN_F5,	KEY_TEN_CL},
	{KEY_CNDRSW,KEY_NOTESF,	KEY_COINSF,	KEY_NTFALL,	KEY_VIBSNS,	KEY_MANDET,	KEY_RESSW1,	KEY_RESSW2},
};

unsigned char	g_ucCAN1Dispatch;		// CAN1�f�B�X�p�b�`�v��
unsigned char	g_ucCAN1RxFlag;			// CAN1��M�����t���O
unsigned char	g_ucCAN1SendFlag;		// CAN1���M���t���O
unsigned char	g_ucCAN1ErrInfo;		// CAN�G���[���
unsigned char	g_ucCAN1LostErr;		// CAN���b�Z�[�W���X�g���
unsigned char	g_ucCAN1StaffErr;		// CAN�X�^�b�t�G���[���
unsigned char	g_ucCAN1FormErr;		// CAN�t�H�[���G���[���
unsigned char	g_ECSR_Delay_f;			// ECSR�f�B���C�t���O�N���A
unsigned char	g_ucCAN1RcvSensFlag;	// CAN1�Z���T�[����M�����t���O

/*--- Extern --------------------*/


//[]----------------------------------------------------------------------[]
///	@brief			CAN�ʐM�^�X�N���C��
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void cantask( void )
{
	MsgBuf	*pmsg;
	MsgBuf	msg;

	LagTim500ms(LAG500_CAN_HEARTBEAT_TIMER, 4, can_snd_data14);	// �n�[�g�r�[�g����(����̂�2s)

	for( ; ; ){
		taskchg(IDLETSKNO);

		pmsg = GetMsg(CANTCBNO);
		if (pmsg != NULL) {
			memcpy(&msg, pmsg, sizeof(msg));
			FreeBuf(pmsg);
			pmsg = &msg;
		}

		canchk(pmsg);	// error check
		canrcv();		// receive
		cansnd();		// send
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			�ڑ���̃n�[�h�E�F�A���Z�b�g
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			m.onouchi
///	@note			CAN�Őڑ�����Ă��鑼�̃m�[�h�����Z�b�g����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/03<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void can_ic_init(void)
{
	unsigned long	StartTime;

	CP_CAN_RES = 1;		// CAN Reset ���s
	StartTime = LifeTimGet();

	do {
		g_ucCAN1Dispatch = 1;
		taskchg(IDLETSKNO);
	} while(LifePastTimGet(StartTime) < CAN_RES_TIME);

	CP_CAN_RES = 0;		// CAN Reset ����
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN������
//[]----------------------------------------------------------------------[]
///	@return			R_CAN_OK				: ����I��<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_BAD_ACTION_TYPE:	: ����`�̐ݒ�w��<br>
///					R_CAN_SW_RST_ERR		: CAN�y���t�F���������Z�b�g���[�h�ɂȂ�Ȃ�<br>
///					R_CAN_SW_WAKEUP_ERR		: CAN�y���t�F�������E�F�C�N�A�b�v���Ȃ�<br>
///					R_CAN_SW_SLEEP_ERR		: CAN�y���t�F�������X���[�v���[�h�ɂȂ���<br>
///					R_CAN_SW_HALT_ERR		: CAN�y���t�F�������z���g���[�h�ɂȂ�Ȃ�
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/20<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned int caninit(void)
{
	unsigned int	api_status;
	struct _canctr	*ctr = _GET_CAN();
	char			i;

	memset(ctr, 0, sizeof(*ctr));						// ���[�N�̈�N���A

	ctr->can_net_stat[NID_RXM] = CAN_STS_INIT;			// �l�b�g���[�N���(CAN_STS_INIT:���������)

	/* CAN�ʐM�ݒ� */
	if (ExIOSignalrd(INSIG_DPSW3)) {					// DPSW3:CAN�I�[�ݒ�
		CAN_TERM_PODR(5, 6) = 0;						// P56:CAN�I�[��ROFF
	} else {
		CAN_TERM_PODR(5, 6) = 1;						// P56:CAN�I�[��RON
	}

	MSTP_CAN1 = 0;										// CAN�̃��W���[���X�g�b�v��Ԃ�����

	api_status = R_CAN_Create(1);						// CAN�y���t�F����������

	R_CAN_PortSet(1, ENABLE);							// CAN�g�����V�[�o�|�[�g�̒[�q�ݒ�

	memset(&ctr->dispbuf, 0, sizeof(ctr->dispbuf));
	for (i = 0; i < SIDMAX; i++) {						// ���M�Ǘ���񏉊���
		transfer[i].sid = t_can_transfer_Initializer[i].sid;
		transfer[i].sndque = t_can_transfer_Initializer[i].sndque;
		transfer[i].sndque->dataframe = t_can_transfer_Initializer[i].dataframe;
		transfer[i].sndque->countmax = t_can_transfer_Initializer[i].countmax;
	}
	ctr->rcvque.dataframe = &ctr->rx_dataframe[0];
	ctr->rcvque.countmax = 128;

	g_ucCAN1Dispatch = 1;								// CAN1�f�B�X�p�b�`�v��
	g_ucCAN1RxFlag = 0;									// CAN1��M�����t���O
	g_ucCAN1SendFlag = 0;								// CAN1���M���t���O
	g_ucCAN1ErrInfo = 0;								// CAN�G���[���
	g_ucCAN1LostErr = 0;								// CAN���b�Z�[�W���X�g���
	g_ucCAN1StaffErr = 0;								// CAN�X�^�b�t�G���[���
	g_ucCAN1FormErr = 0;								// CAN�t�H�[���G���[���
	g_ECSR_Delay_f = 0;									// ECSR�f�B���C�t���O�N���A
	g_ucCAN1RcvSensFlag = 0;							// CAN1�Z���T�[����M�����t���O

	R_CAN_RxSet(1, CANBOX_RX, 0x002, DATA_FRAME);		// ���b�Z�[�W��M�w��
	R_CAN_RxSetMask(1, CANBOX_RX, 0x07F);				// ���b�Z�[�W�̎󂯓���}�X�N�ݒ�

	return (api_status);
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN�^�X�N�C�x���g�m�F
//[]----------------------------------------------------------------------[]
///	@return			0:�C�x���g�Ȃ�, 1:�C�x���g����
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
short canevent( void )
{
	struct _canctr	*ctr = _GET_CAN();

	// �����f�B�X�p�b�`�v���`�F�b�N
	if (g_ucCAN1Dispatch) {
		g_ucCAN1Dispatch = 0;
		return 1;
	}

	// ��M�f�[�^�`�F�b�N
	if (g_ucCAN1RxFlag) {	// ��M�f�[�^����
		return 1;
	}

	// ���M�v���`�F�b�N
	if (transfer[CAN_NETWORK].sndque->count) {
		return 1;
	}
	if (transfer[CAN_CONTROL].state == SDO_RESET) {
		if (0 != transfer[CAN_CONTROL].sndque->count) {
			return 1;
		}
	}
	if (transfer[CAN_DISPLAY].state == SDO_RESET) {
		if (0 != ctr->dispbuf.count) {
			return 1;
		}
	}

	return 0;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN ���M�f�[�^�t���[���i�[
//[]----------------------------------------------------------------------[]
///	@param[in]		*sndque	: ���M�L���[
///	@param[in]		*frame_p: �f�[�^�t���[��
///	@return			ret		: void
///	@author			m.onouchi
///	@note			���M�������f�[�^�t���[���𑗐M�p�����O�o�b�t�@�Ɋi�[����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2011/12/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void can_snd_queset(t_can_quectrl *sndque, can_std_frame_t *frame_p)
{
	volatile unsigned long	ist;			// ���݂̊�����t���

	ist = _di2();
	memcpy(&sndque->dataframe[sndque->writeidx], frame_p, sizeof(can_std_frame_t));

	++sndque->writeidx;
	if (sndque->writeidx >= sndque->countmax) {
		sndque->writeidx = 0;
	}

	if (sndque->count < sndque->countmax) {
		++sndque->count;
	} else {
		++sndque->readidx;
		if (sndque->readidx >= sndque->countmax) {
			sndque->readidx = 0;
		}
	}
	_ei2(ist);
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN ��M�f�[�^�t���[���i�[
//[]----------------------------------------------------------------------[]
///	@param[in]		*frame_p: �f�[�^�t���[��
///	@return			ret		: void
///	@author			m.onouchi
///	@note			��M�����f�[�^�t���[������M�p�����O�o�b�t�@�Ɋi�[����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2011/12/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void can_rcv_queset(can_std_frame_t *frame_p)
{
	struct _canctr	*ctr = _GET_CAN();

	memcpy(&ctr->rcvque.dataframe[ctr->rcvque.writeidx], frame_p, sizeof(can_std_frame_t));

	++ctr->rcvque.writeidx;
	if (ctr->rcvque.writeidx >= ctr->rcvque.countmax) {
		ctr->rcvque.writeidx = 0;
	}

	if (ctr->rcvque.count < ctr->rcvque.countmax) {
		++ctr->rcvque.count;
	} else {
		++ctr->rcvque.readidx;
		if (ctr->rcvque.readidx >= ctr->rcvque.countmax) {
			ctr->rcvque.readidx = 0;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			CAN�G���[�Ď�����
//[]----------------------------------------------------------------------[]
///	@param[in]		*pmsg	: ���b�Z�[�W
///	@return			ret		: void
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/16<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void canchk(MsgBuf *pmsg)
{
	unsigned char	err;
	unsigned char	lost;
	unsigned long	ist;		// ���݂̊�����t���

	volatile struct st_can __evenaccess * can_block_p;

	can_block_p = (struct st_can __evenaccess *) 0x91200;	// CAN1

	ist = _di2();
	err = g_ucCAN1ErrInfo;
	g_ucCAN1ErrInfo = 0;
	lost = g_ucCAN1LostErr;
	g_ucCAN1LostErr = 0;
	_ei2(ist);

	switch (pmsg->msg.command) {
	case CAN_TIMEOUT_KEEP_ALIVE_02:			// RXF
		err_chk(ERRMDL_CAN, ERR_CAN_NOANSWER_02, 1, 0, 0);	// E8612:CAN �m�[�h2�����Ȃ�(����)
		can_ic_init();	// �ڑ���̃n�[�h�E�F�A���Z�b�g
		break;
	case CAN_TIMEOUT_ECSR_DELAY:			// ECSR�f�B���C�^�C���A�E�g
		g_ECSR_Delay_f = 0;// ECSR�f�B���C�t���O�N���A
		break;
	default:
		break;
	}

	if (err & 0x08) {						// BOEIF:�o�X�I�t�J�n���o
		err_chk(ERRMDL_CAN, ERR_CAN_COMFAIL, 1, 0, 0);		// E8601:CAN �ʐM�s��(����)
// MH810100(S) K.Onodera 2020/01/08 �Ԕԃ`�P�b�g���X(�x���ʒm)
		lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) K.Onodera 2020/01/08 �Ԕԃ`�P�b�g���X(�x���ʒm)
	}
	if (err & 0x10) {						// BORIF:�o�X�I�t���A���o
		err_chk(ERRMDL_CAN, ERR_CAN_COMFAIL, 0, 0, 0);		// E8601:CAN �ʐM�s��(����)
// MH810100(S) K.Onodera 2020/01/08 �Ԕԃ`�P�b�g���X(�x���ʒm)
		lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) K.Onodera 2020/01/08 �Ԕԃ`�P�b�g���X(�x���ʒm)
	}
	if(g_ECSR_Delay_f == 0) {
		if (can_block_p->ECSR.BIT.SEF) {	// �X�^�b�t�G���[���o
			if(g_ucCAN1StaffErr == 0) {	// �X�^�b�t�G���[�����o
				err_chk(ERRMDL_CAN, ERR_CAN_STUFF_ERR, 1, 0, 0);	// E8605:CAN �X�^�b�t�G���[(����)
				g_ucCAN1StaffErr = 1;// CAN�X�^�b�t�G���[���Z�b�g
// MH810100(S) K.Onodera 2020/01/26 �Ԕԃ`�P�b�g���X (#3884 �x���ʒm_E8601�̖��\���s��C��)
				lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) K.Onodera 2020/01/26 �Ԕԃ`�P�b�g���X (#3884 �x���ʒm_E8601�̖��\���s��C��)
			}
		} else {
			if(g_ucCAN1StaffErr != 0) {	// �X�^�b�t�G���[���o��
				err_chk(ERRMDL_CAN, ERR_CAN_STUFF_ERR, 0, 0, 0);	// E8605:CAN �X�^�b�t�G���[(����)
// MH810100(S) Y.Watanabe 2020/01/26 �Ԕԃ`�P�b�g���X (#3884 �x���ʒm_E8601�̖��\���s��C��)
				lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2020/01/26 �Ԕԃ`�P�b�g���X (#3884 �x���ʒm_E8601�̖��\���s��C��)
			}
			g_ucCAN1StaffErr = 0;// CAN�X�^�b�t�G���[���N���A
		}
		if (can_block_p->ECSR.BIT.FEF) {	// �t�H�[���G���[���o
			if(g_ucCAN1FormErr == 0) {	// �t�H�[���G���[�����o
				err_chk(ERRMDL_CAN, ERR_CAN_FORM_ERR, 1, 0, 0);		// E8606:CAN �t�H�[���G���[(����)
				g_ucCAN1FormErr = 1;// CAN�t�H�[���G���[���Z�b�g
// MH810100(S) Y.Watanabe 2020/01/26 �Ԕԃ`�P�b�g���X (#3884 �x���ʒm_E8601�̖��\���s��C��)
				lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2020/01/26 �Ԕԃ`�P�b�g���X (#3884 �x���ʒm_E8601�̖��\���s��C��)
			}
		} else {
			if(g_ucCAN1FormErr != 0) {	// �t�H�[���G���[���o��
				err_chk(ERRMDL_CAN, ERR_CAN_FORM_ERR, 0, 0, 0);		// E8606:CAN �t�H�[���G���[(����)
// MH810100(S) Y.Watanabe 2020/01/26 #3884 �x���ʒm_E8601�̖��\���s��C��
				lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2020/01/26 #3884 �x���ʒm_E8601�̖��\���s��C��
			}
			g_ucCAN1FormErr = 0;// CAN�t�H�[���G���[���N���A
		}
		if (can_block_p->ECSR.BIT.CEF) {	// CRC�G���[���o
			err_chk(ERRMDL_CAN, ERR_CAN_CRC_ERR, 2, 0, 0);			// E8607:CAN CRC�G���[(������������)
		}
		if (can_block_p->ECSR.BYTE) {
			can_block_p->ECSR.BYTE = 0x00;							// �G���[�R�[�h�i�[���W�X�^�N���A
			// NOTE:�{�֐���ECSR���N���A���Ă��玟��ECSR��Ԃ��m�肷��܂Ŏ኱���Ԃ��K�v�Ȃ��߂Ƀf�B���C���m�ۂ���K�v������
			// �������Ȃ���RXM��RXF���CAN�P�[�u������������ԂŃX�^�b�t�G���[�����^��������������
			Lagtim(CANTCBNO, CAN_TIMER_ECSR_DELAY, ECSR_DELAYTIM);// ECSR�f�B���C�^�C�}�X�^�[�g
			g_ECSR_Delay_f = 1;// ECSR�f�B���C�t���O�Z�b�g
		}
	}
	if (lost) {
		// �I�[�o�[���[�h�i���b�Z�[�W���X�g�j�G���[�̓o�^
		err_chk(ERRMDL_CAN, ERR_CAN_LOST_ERR, 2, 0, 0);				// E8608:CAN �I�[�o���[�h�G���[(������������)
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN��M�f�[�^�Ď�����
//[]----------------------------------------------------------------------[]
///	@return			ret		: void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void canrcv(void)
{
	volatile unsigned long	ist;			// ���݂̊�����t���
	struct _canctr	*ctr = _GET_CAN();
	t_data0			*p_dt0;
	unsigned short	length;
	unsigned char	dlc;
	int				i;
	int				j;
	int				seqno;
	union {
		unsigned short	us;
		struct {
			unsigned char	high;
			unsigned char	low;
		} uc;
	} crc16;
	can_std_frame_t	tx_dataframe;
	can_std_frame_t	rx_dataframe;

	/* ��M���� */
	if (g_ucCAN1RxFlag) {						// ��M�L���[�Ɏ�M���b�Z�[�W����
		memset(&rx_dataframe, 0, sizeof(can_std_frame_t));
		if (0 != ctr->rcvque.count) {
			ist = _di2();
			memcpy(&rx_dataframe, &ctr->rcvque.dataframe[ctr->rcvque.readidx], sizeof(can_std_frame_t));
			--ctr->rcvque.count;
			++ctr->rcvque.readidx;
			if (ctr->rcvque.readidx >= ctr->rcvque.countmax) {
				ctr->rcvque.readidx = 0;
			}
			if (0 == ctr->rcvque.count) {
				g_ucCAN1RxFlag = 0;
			}
			_ei2(ist);
		}

		for (i = 0; i < SIDMAX; i++) {
			if (transfer[i].sid == (rx_dataframe.id & 0x780)) {
				switch (i) {
				case CAN_EMERGENCY:				// �G�}�[�W�F���V�[��M
					queset(OPETCBNO, CAN_EMG_EVT, rx_dataframe.dlc, rx_dataframe.data);
					break;
				case CAN_EVENT:					// �C�x���g��M
					get_can_key_event(&rx_dataframe);
					break;
				case CAN_CONTROL:				// �R���g���[����M
					p_dt0 = (t_data0*)&rx_dataframe.data;
					switch (p_dt0->bit.command) {
					case 3:						// initiate �� response
						if (transfer[i].state == SDO_INITIATE_DOWNLOAD_REQUEST) {
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_RESET;
							--transfer[CAN_CONTROL].sndque->count;
							++transfer[CAN_CONTROL].sndque->readidx;
							if (transfer[CAN_CONTROL].sndque->readidx >= transfer[CAN_CONTROL].sndque->countmax) {
								transfer[CAN_CONTROL].sndque->readidx = 0;
							}
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
						} else {
							// �G���[����
						}
						break;
					case 4:						// abort
						if (transfer[i].state == SDO_INITIATE_DOWNLOAD_REQUEST) {
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_RESET;
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
						}
						break;
					default:
						break;
					}
					break;
				case CAN_DISPLAY:				// �f�B�X�v���C��M
					p_dt0 = (t_data0*)&rx_dataframe.data;
					switch (p_dt0->bit.command) {
					case 3:						// initiate �� response
						if (transfer[i].state == SDO_INITIATE_DOWNLOAD_REQUEST) {
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_BLOCK_DOWNLOAD_IN_PROGRESS;
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
							/* segment */
							p_dt0 = (t_data0*)&tx_dataframe.data;
							length = transfer[i].count;
							for (seqno = 1; seqno <= transfer[i].blksize; seqno++) {
								memset(&tx_dataframe, 0, sizeof(can_std_frame_t));
								tx_dataframe.id = transfer[i].sid | NID_RXM;	// ID�Z�b�g
								if (length <= 7) {
									tx_dataframe.dlc = length;
								} else {
									tx_dataframe.dlc = 7;
								}
								length -= tx_dataframe.dlc;
								p_dt0->bit.command = 0;
								p_dt0->bit.t = transfer[i].toggle = !(transfer[i].toggle & 1);
								if (seqno == transfer[i].blksize) {
									p_dt0->bit.c = 1;
								}
								tx_dataframe.dlc += 1;
								for (dlc = 1; dlc < tx_dataframe.dlc; dlc++) {
									transfer[i].crc ^= tx_dataframe.data[dlc] = ctr->dispbuf.data[transfer[i].offset];
									for (j = 0; j < CHAR_BIT; j++) {
										if (transfer[i].crc & 1) {
											transfer[i].crc = (transfer[i].crc >> 1) ^ CRCPOLY2;
										} else {
											transfer[i].crc >>= 1;
										}
									}
									transfer[i].offset++;
									if (transfer[i].offset >= _MAX_LENGTH_TRANSFERT) {
										transfer[i].offset = 0;
									}
								}
								can_snd_queset(transfer[i].sndque, &tx_dataframe);
							}
							/* end */
							memset(&tx_dataframe, 0, sizeof(can_std_frame_t));
							tx_dataframe.id = transfer[i].sid | NID_RXM;	// ID�Z�b�g
							tx_dataframe.dlc = 3;
							p_dt0->bit.command = 6;

							crc16.uc.high = (unsigned char)(transfer[i].crc & 0x00ff);
							crc16.uc.low = (unsigned char)((transfer[i].crc & 0xff00) >> CHAR_BIT);
							transfer[i].crc = crc16.us;
							tx_dataframe.data[1] = crc16.uc.high;
							tx_dataframe.data[2] = crc16.uc.low;
							can_snd_queset(transfer[i].sndque, &tx_dataframe);
						}
						break;
					case 4:						// abort
						if ((transfer[i].state == SDO_INITIATE_DOWNLOAD_REQUEST)
						||  (transfer[i].state == SDO_BLOCK_DOWNLOAD_IN_PROGRESS)) {
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_RESET;
							transfer[i].toggle = 0;
							transfer[i].count = 0;
							transfer[i].blksize = 0;
							transfer[i].crc = 0;
							transfer[i].offset = 0;
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
						}
						break;
					case 5:						// end �� response
						if (transfer[i].state == SDO_END_DOWNLOAD_REQUEST) {
							for (length = 0; length < transfer[i].count; length++) {
								--ctr->dispbuf.count;
								++ctr->dispbuf.readidx;
								if (ctr->dispbuf.readidx >= _MAX_LENGTH_TRANSFERT) {
									ctr->dispbuf.readidx = 0;
								}
							}
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_RESET;
							transfer[i].toggle = 0;
							transfer[i].count = 0;
							transfer[i].blksize = 0;
							transfer[i].crc = 0;
							transfer[i].offset = 0;
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
						}
						break;
					}
					break;
				case CAN_HEARTBEAT:				// �n�[�g�r�[�g��M
					switch (rx_dataframe.id & 0x7F) {
					case NID_RXF:
						ctr->can_net_stat[rx_dataframe.id & 0x7F] = rx_dataframe.data[0];
						if (rx_dataframe.data[0] == CAN_STS_INIT) {			// �l�b�g���[�N���(CAN_STS_INIT:���������)
							ctr->can_net_stat[NID_RXM] = CAN_STS_OPE;		// �l�b�g���[�N���(CAN_STS_OPE:�I�y���[�V���i�����)
							can_snd_data0((unsigned char)(rx_dataframe.id & 0x7F), CAN_STS_OPE);
						}
						if (ERR_CHK[mod_can][ERR_CAN_NOANSWER_02]) {			// E8612:CAN �m�[�h2�����Ȃ� ������
							err_chk(ERRMDL_CAN, ERR_CAN_NOANSWER_02, 0, 0, 0);	// E8612:CAN �m�[�h2�����Ȃ�(����)
						}
						if (rx_dataframe.dlc == 7) {
							memset(&RXF_VERSION[0], 0, sizeof(RXF_VERSION));
							for (dlc = 1, j = 0; dlc < 4; dlc++) {
								if (rx_dataframe.data[dlc] >= 0x41 && rx_dataframe.data[dlc] <= 0x5A) {	// �`�`�y
									RXF_VERSION[j] = rx_dataframe.data[dlc];
									j++;
								}
							}
							BCDtoASCII(&rx_dataframe.data[dlc], &RXF_VERSION[j], 3);
						}
						CAN_START_KEEP_ALIVE_TIMER(NID_RXF, (CAN_HEARTBEAT_TIME+2)*1000*3);	// �L�[�v�A���C�u�^�C�}���X�^�[�g
						break;
					default:
						break;
					}
					break;
				default:
					break;
				}
			}
		}
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN���M�f�[�^�Ď�����
//[]----------------------------------------------------------------------[]
///	@return			ret		: void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void cansnd(void)
{
	volatile unsigned long	ist;			// ���݂̊�����t���
	struct _canctr	*ctr = _GET_CAN();
	can_std_frame_t	tx_dataframe;
	t_data0			*p_dt0;

	// �����҂�
	if (transfer[CAN_CONTROL].state == SDO_INITIATE_DOWNLOAD_REQUEST) {
		if (LifePastTimGet(transfer[CAN_CONTROL].timer) >= CAN_RESP_TIME) {
			transfer[CAN_CONTROL].state = SDO_RESET;
			transfer[CAN_CONTROL].toggle = 0;
			transfer[CAN_CONTROL].count = 0;
			transfer[CAN_CONTROL].blksize = 0;
			transfer[CAN_CONTROL].crc = 0;
			transfer[CAN_CONTROL].offset = 0;
		}
	}
	if ((transfer[CAN_DISPLAY].state == SDO_INITIATE_DOWNLOAD_REQUEST)
	||  (transfer[CAN_DISPLAY].state == SDO_END_DOWNLOAD_REQUEST)) {
		if (LifePastTimGet(transfer[CAN_DISPLAY].timer) >= CAN_RESP_TIME) {
			ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
			transfer[CAN_DISPLAY].state = SDO_RESET;
			transfer[CAN_DISPLAY].toggle = 0;
			transfer[CAN_DISPLAY].count = 0;
			transfer[CAN_DISPLAY].blksize = 0;
			transfer[CAN_DISPLAY].crc = 0;
			transfer[CAN_DISPLAY].offset = 0;
			_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
		}
	}

	/* ���M���� */
	if (g_ucCAN1SendFlag == 0) {						// ���M���łȂ�
		if (!ERR_CHK[mod_can][ERR_CAN_COMFAIL]) {		// E8601:CAN �ʐM�s�� �����Ȃ�
			if (0 != transfer[CAN_NETWORK].sndque->count) {
				g_ucCAN1SendFlag = 1;		// ���M���ɐݒ�
				R_CAN_TxSet(1, CANBOX_TX, &transfer[CAN_NETWORK].sndque->dataframe[transfer[CAN_NETWORK].sndque->readidx], DATA_FRAME);	// ���M���[���{�b�N�X�̐ݒ�
				--transfer[CAN_NETWORK].sndque->count;
				++transfer[CAN_NETWORK].sndque->readidx;
				if (transfer[CAN_NETWORK].sndque->readidx >= transfer[CAN_NETWORK].sndque->countmax) {
					transfer[CAN_NETWORK].sndque->readidx = 0;
				}
				goto _FIN;
			}
			if (transfer[CAN_CONTROL].state == SDO_RESET) {
				if (0 != transfer[CAN_CONTROL].sndque->count) {
					if (ctr->can_net_stat[NID_RXM] == CAN_STS_OPE) {	// �l�b�g���[�N���(CAN_STS_OPE:�I�y���[�V���i�����)
						g_ucCAN1SendFlag = 1;		// ���M���ɐݒ�
						transfer[CAN_CONTROL].state = SDO_INITIATE_DOWNLOAD_REQUEST;
						R_CAN_TxSet(1, CANBOX_TX, &transfer[CAN_CONTROL].sndque->dataframe[transfer[CAN_CONTROL].sndque->readidx], DATA_FRAME);	// ���M���[���{�b�N�X�̐ݒ�
						transfer[CAN_CONTROL].timer = LifeTimGet();							// �����҂��^�C�}�Z�b�g
						goto _FIN;
					}
				}
			}
			switch (transfer[CAN_DISPLAY].state) {
			case SDO_RESET:
				if (ctr->can_net_stat[NID_RXM] == CAN_STS_OPE) {	// �l�b�g���[�N���(CAN_STS_OPE:�I�y���[�V���i�����)
					if (0 != ctr->dispbuf.count) {
						ist = _di2();
						g_ucCAN1SendFlag = 1;	// ���M���ɐݒ�
						memset(&tx_dataframe, 0, sizeof(can_std_frame_t));
						transfer[CAN_DISPLAY].state = SDO_INITIATE_DOWNLOAD_REQUEST;
						transfer[CAN_DISPLAY].toggle = 1;
						transfer[CAN_DISPLAY].count = ctr->dispbuf.count;
						transfer[CAN_DISPLAY].blksize = (unsigned char)(transfer[CAN_DISPLAY].count / 7);
						if (transfer[CAN_DISPLAY].count % 7) {
							transfer[CAN_DISPLAY].blksize++;
						}
						transfer[CAN_DISPLAY].crc = 0;
						transfer[CAN_DISPLAY].offset = ctr->dispbuf.readidx;
						_ei2( ist );

						/* initiate */
						tx_dataframe.id = transfer[CAN_DISPLAY].sid | NID_RXM;				// ID�Z�b�g
						tx_dataframe.dlc = 2;												// �f�[�^��
						p_dt0 = (t_data0*)&tx_dataframe.data;
						p_dt0->bit.command = 1;												// �R�}���h�Z�b�g
						p_dt0->bit.e = 1;													// �u���b�N�]��
						tx_dataframe.data[1] = transfer[CAN_DISPLAY].blksize;				// �Z�O�����g��

						R_CAN_TxSet(1, CANBOX_TX, &tx_dataframe, DATA_FRAME);
						transfer[CAN_DISPLAY].timer = LifeTimGet();							// �����҂��^�C�}�Z�b�g
					}
				}
				break;
			case SDO_BLOCK_DOWNLOAD_IN_PROGRESS:
			case SDO_ABORTED_INTERNAL:
				if (0 != transfer[CAN_DISPLAY].sndque->count) {
					if (ctr->can_net_stat[NID_RXM] == CAN_STS_OPE) {	// �l�b�g���[�N���(CAN_STS_OPE:�I�y���[�V���i�����)
						ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
						g_ucCAN1SendFlag = 1;		// ���M���ɐݒ�
						R_CAN_TxSet(1, CANBOX_TX, &transfer[CAN_DISPLAY].sndque->dataframe[transfer[CAN_DISPLAY].sndque->readidx], DATA_FRAME);	// ���M���[���{�b�N�X�̐ݒ�
						--transfer[CAN_DISPLAY].sndque->count;
						++transfer[CAN_DISPLAY].sndque->readidx;
						if (transfer[CAN_DISPLAY].sndque->readidx >= transfer[CAN_DISPLAY].sndque->countmax) {
							transfer[CAN_DISPLAY].sndque->readidx = 0;
						}
						_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData end
					}
				}
				break;
			default:
				break;
			}
		}
	}
_FIN:
	return;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN �l�b�g���[�N���M(00h + NID_RXM)
//[]----------------------------------------------------------------------[]
///	@param[in]		d_node	: ���M��m�[�h
///	@param[in]		sts		: 0:Initialization, 1:Operational, 2:Stopped
///	@return			ret		: 0:���MOK, 1:���MNG
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2011/01/23<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void can_snd_data0(unsigned char d_node, unsigned char sts)
{
	can_std_frame_t tx_dataframe;

	memset(&tx_dataframe, 0, sizeof(can_std_frame_t));

	// �d������
	tx_dataframe.id = transfer[CAN_NETWORK].sid | NID_RXM;
	tx_dataframe.dlc = 2;
	tx_dataframe.data[0] = d_node;	// ���M��m�[�h
	tx_dataframe.data[1] = sts;		// ���

	can_snd_queset(transfer[CAN_NETWORK].sndque, &tx_dataframe);
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN �R���g���[�����M(180h + NID_RXM)
//[]----------------------------------------------------------------------[]
///	@param[in]		ctr_cd	: �R���g���[���R�[�h<br>
///							:	0x00:����`<br>
///							:	0x10-0x1F:�u�U�[��(�s�b��)����<br>
///							:	0x20-0x2F:�u�U�[��(�s�s�b��)����<br>
///							:	0x30-0x3F:�u�U�[��(�s�s�s�b��)����<br>
///							:	0x40-0x4F:LCD�o�b�N���C�g�P�x�l�ύX
///	@param[in]		out1	: �o�͂P<br>
///							:	b7:�ޑK��o���K�C�hLED(0:����, 1:�_��)<br>
///							:	b6:�e���L�[�K�C�hLED(0:����, 1:�_��)<br>
///							:	b5:�ƌ��������{�^���P(0:����, 1:�_��)<br>
///							:	b4:�ƌ��������{�^���P(0:����, 1:�_��)
///	@return			ret		: void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void can_snd_data3(char ctr_cd, char out1)
{
	can_std_frame_t	tx_dataframe;
	t_data0			*p_dt0;

	memset(&tx_dataframe, 0, sizeof(can_std_frame_t));

	tx_dataframe.id = transfer[CAN_CONTROL].sid | NID_RXM;
	tx_dataframe.dlc = 4;
	p_dt0 = (t_data0*)&tx_dataframe.data;
	p_dt0->bit.command = 1;					// �R�}���h�Z�b�g
	p_dt0->bit.e = 0;						// �V���O���]��
	tx_dataframe.data[1] = 0;				// �Z�O�����g��
	tx_dataframe.data[2] = ctr_cd;			// �R���g���[���R�[�h
	tx_dataframe.data[3] = out1;			// �o�͂P

	can_snd_queset(transfer[CAN_CONTROL].sndque, &tx_dataframe);
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN �f�B�X�v���C���M(200h + NID_RXM)
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: ��M�f�[�^
///	@param[in]		length	: �f�[�^�T�C�Y
///	@return			ret		: void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/27<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void can_snd_data4(void *data, unsigned short length)
{
	struct _canctr	*ctr = _GET_CAN();
	unsigned short	first_half;
	unsigned short	second_half;

	if (ctr->dispbuf.writeidx >= ctr->dispbuf.readidx) {
		first_half = _MAX_LENGTH_TRANSFERT - ctr->dispbuf.writeidx;
		second_half = ctr->dispbuf.readidx;
		if (ctr->dispbuf.count == _MAX_LENGTH_TRANSFERT) {
			first_half = second_half = 0;
		}
	} else {
		first_half = ctr->dispbuf.readidx - ctr->dispbuf.writeidx;
		second_half = 0;
	}

	if ((first_half + second_half) >= length) {
		if (length > first_half) {
			second_half = length - first_half;
		} else {
			first_half = length;
			second_half = 0;
		}
		memcpy(&ctr->dispbuf.data[ctr->dispbuf.writeidx], data, first_half);
		ctr->dispbuf.writeidx += first_half;
		if (ctr->dispbuf.writeidx == _MAX_LENGTH_TRANSFERT) {
			ctr->dispbuf.writeidx = 0;
		}
		if (second_half) {
			memcpy(&ctr->dispbuf.data[0], (unsigned char *)data + first_half, second_half);
			ctr->dispbuf.writeidx = second_half;
		}
		ctr->dispbuf.count += length;
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN �n�[�g�r�[�g���M(700h + NID_RXM)
//[]----------------------------------------------------------------------[]
///	@return			ret		: void
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/02<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void can_snd_data14(void)
{
	volatile struct st_can __evenaccess * can_block_p;
	struct _canctr	*ctr = _GET_CAN();
	can_std_frame_t tx_dataframe;

	LagTim500ms(LAG500_CAN_HEARTBEAT_TIMER, CAN_HEARTBEAT_TIME*2, can_snd_data14);	// �n�[�g�r�[�g����

	can_block_p = (struct st_can __evenaccess *) 0x91200;	// CAN1
	if (can_block_p->STR.BIT.BOST) {		// �o�X�I�t���
		return;
	}

	memset(&tx_dataframe, 0, sizeof(can_std_frame_t));

	tx_dataframe.id = transfer[CAN_HEARTBEAT].sid | NID_RXM;
	tx_dataframe.dlc = 1;
	tx_dataframe.data[0] = ctr->can_net_stat[NID_RXM];	// �l�b�g���[�N���

	can_snd_queset(transfer[CAN_HEARTBEAT].sndque, &tx_dataframe);
}

//[]----------------------------------------------------------------------[]
///	@brief			���͐M���ω��C�x���g�̒ʒm
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: ��M�f�[�^
///	@return			ret		: void
///	@author			k.tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void get_can_key_event(can_std_frame_t *dataframe)
{
	unsigned char	i;
	unsigned char	j;
	unsigned char	wkbit;
	unsigned char	on_off;

	for(i = 0; i < dataframe->dlc; ++i) {
		wkbit = 0x80;
		for (j = 0; j < 8; ++j, wkbit>>=1) {
			if ((dataframe->data[i] & wkbit) != (key_dat[i].BYTE & wkbit)) {	// �ω�����
				if (dataframe->data[i] & wkbit) {
					on_off = 1;
				} else {
					on_off = 0;
				}
				queset(OPETCBNO, scan_event_table[i][j], 1, &on_off);
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
				// �l�̌��m�H
				if( scan_event_table[i][j] == KEY_MANDET ){
					lcdbm_notice_ope( LCDBM_OPCD_MAN_DTC, on_off );
				}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
			}
		}
		key_dat[i].BYTE = dataframe->data[i];	// key_dat�X�V
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
// �x���ʒm(lcdbm_notice_alm)��call���̒ǉ�
		// �x���ʒm	����	0=�����1=����(�x���ʒm���ω��������̂ݑ��M����)
		//		��݋���			key_dat[3].BIT.B5
		//		���ү��̌�		key_dat[3].BIT.B7
		//		����ذ�ް�̌�	key_dat[3].BIT.B6
		lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ύX))
	}
	if (g_ucCAN1RcvSensFlag == 0) {
		g_ucCAN1RcvSensFlag = 1;							// CAN1�Z���T�[����M�����t���OON
	}
}
