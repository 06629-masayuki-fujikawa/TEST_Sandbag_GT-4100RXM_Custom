/*[]----------------------------------------------------------------------[]
 *|	filename: sysmntcomdr.c
 *[]----------------------------------------------------------------------[]
 *| summary	: SCI�h���C�o(1�p�P�b�g���o�@�\)
 *| author	: machida.k
 *| date	: 2005-07-22
 *| update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/

#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"sysmnt_def.h"

/*--------------------------------------------------------------------------*/
/*			E X T E R N														*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/

/* timer counters */
ushort	SYSMNTcomdr_c2c_timer;
ushort	SYSMNTcomdr_sndcmp_timer;

/* COMDR����f�[�^ */
static struct {
	eSYSMNT_COMDR_STS	status;		/* COMDR�X�e�[�^�X */
	T_SYSMNT_SCI_PACKET	r_packet;	/* ��M�o�b�t�@ */
	T_SYSMNT_SCI_PACKET	s_packet;	/* ���M�o�b�t�@ */
}SYSMNTcomdr_Ctrl;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

static	void	port_reset(void);

static	BOOL	pktchk(const T_SYSMNT_SCI_PACKET *rcvpkt);
static	BOOL	bccchk(const T_SYSMNT_SCI_PACKET *rcvpkt);
static	void	pkt2data(T_SYSMNT_PKT *pkt_bin, const T_SYSMNT_SCI_PACKET *pkt_ascii);
static	void	data2pkt(T_SYSMNT_SCI_PACKET *pkt, const T_SYSMNT_PKT *send_data, int type);

static	BOOL	send(uchar *data, int len);

static	void	int_scircv(uchar rcvdata);
static	void	int_scierr(uchar errtype);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_Open()
 *[]----------------------------------------------------------------------[]
 *| summary	: SYSMNTcomdr���W���[��������
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	SYSMNTcomdr_Open(void)
{

/* SCI�C���^�t�F�[�X������(���荞�ݖ���) */
	SYSMNTsci_Init(int_scircv, int_scierr);
/* ����f�[�^������ */
	SYSMNTcomdr_Reset();
/* SCI���荞�ݗL���� */
	SYSMNTsci_Start();
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_Reset()
 *[]----------------------------------------------------------------------[]
 *| summary	: SYSMNTcomdr���W���[�����Z�b�g
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	SYSMNTcomdr_Reset(void)
{
	ulong	ists;
	
	ists = _di2();
	SYSMNTcomdr_Ctrl.status			= SYSMNT_COMDR_NO_PKT;
	SYSMNTcomdr_Ctrl.r_packet.len	= 0;
	SYSMNTcomdr_sndcmp_timer		= SYSMNT_TIMER_STOP;
	SYSMNTcomdr_c2c_timer			= SYSMNT_TIMER_STOP;
	_ei2(ists);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_GetRcvPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�p�P�b�g�擾
 *| param	: buf - ��M�p�P�b�g�o�b�t�@	<OUT>
 *|					(�߂�l��SYSMNT_SCI_PKT_RCVD�̂Ƃ��L��)
 *| return	: COMDR�X�e�[�^�X
 *[]----------------------------------------------------------------------[]*/
eSYSMNT_COMDR_STS	SYSMNTcomdr_GetRcvPacket(T_SYSMNT_PKT *buf)
{
	eSYSMNT_COMDR_STS ret;
	
	ret = SYSMNTcomdr_Ctrl.status;
	
	switch (ret) {
	case SYSMNT_COMDR_NO_PKT:					/* �p�P�b�g����M */
		if (SYSMNTcomdr_Ctrl.r_packet.len > 0) {
			/* ��M�� */
			if (SYSMNTcomdr_c2c_timer <= 0) {
				SYSMNTcomdr_Ctrl.status = SYSMNT_COMDR_PKT_RCVD;
				/* �L�����N�^�ԃ^�C���A�E�g������1�p�P�b�g��M���� */
				if (pktchk(&SYSMNTcomdr_Ctrl.r_packet)) {
					if (SYSMNTcomdr_Ctrl.r_packet.data[0] == STX) {
						if (!bccchk(&SYSMNTcomdr_Ctrl.r_packet)) {
						/* BCC�G���[ */
							ret = SYSMNT_COMDR_BCC_ERR;
						}
					}
					if (ret != SYSMNT_COMDR_BCC_ERR) {
					/* ��M�p�P�b�g���� */
						/* ACII�˃o�C�i���ϊ� */
						pkt2data(buf, &SYSMNTcomdr_Ctrl.r_packet);
						ret = SYSMNT_COMDR_PKT_RCVD;
					}
				}
				else {
				/* ��M�p�P�b�g�ُ� */
					ret = SYSMNT_COMDR_PKT_ERR;
				}
				SYSMNTcomdr_Reset();
			}
		}
		break;
	default:								/* ��M�G���[���� */
		SYSMNTcomdr_Reset();
		break;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_SendPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�P�b�g���M(���M�������^�C���A�E�g�܂Ń��^�[�����Ȃ�))
 *| param	: buf - ���M�p�P�b�g	<IN>
 *|			  type - ���M�p�P�b�g�^�C�v(SYSMNT_TYPE_SEL,SYSMNT_TYPE_POL,SYSMNT_TYPE_STX)
 *| return	: TRUE - ���M����
 *[]----------------------------------------------------------------------[]*/
BOOL	SYSMNTcomdr_SendPacket(T_SYSMNT_PKT *buf, int type)
{
	T_SYSMNT_SCI_PACKET *sbuf = &SYSMNTcomdr_Ctrl.s_packet;
	
	data2pkt(sbuf, buf, type);
	
	if (!send(sbuf->data, sbuf->len)) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_SendByte()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�C�g���M(���M�������^�C���A�E�g�܂Ń��^�[�����Ȃ�))
 *| param	: c - ���M�f�[�^
 *| return	: TRUE - ���M����
 *[]----------------------------------------------------------------------[]*/
BOOL	SYSMNTcomdr_SendByte(uchar c)
{
	if (!send(&c, 1)) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_IsSendCmp()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�����`�F�b�N
 *| param	: timeout - TRUE=�^�C���A�E�g����				<OUT>
 *| return	: TRUE=���M����or�^�C���A�E�g����
 *[]----------------------------------------------------------------------[]*/
BOOL	SYSMNTcomdr_IsSendCmp(BOOL *timeout)
{
	*timeout = FALSE;
	
	if (SYSMNTsci_IsSndCmp()){
		SYSMNTcomdr_sndcmp_timer = SYSMNT_TIMER_STOP;
		return TRUE;		/* ���M���� */
	}
	
	if (SYSMNTcomdr_sndcmp_timer <= 0) {
		*timeout = TRUE;
		port_reset();
		return TRUE;		/* ���M�^�C���A�E�g */
	}
	
	return FALSE;				/* ���M������ */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_Close()
 *[]----------------------------------------------------------------------[]
 *| summary	: �V���A�����荞�ݒ�~
 *| param	: none
 *| return	: none
 *|-----------------------------------------------------------------------[]
 *| remark	: �{�֐���A�V���A�����荞�ݍĊJ�� SYSMNTcomdr_Open() �ōs��
 *[]----------------------------------------------------------------------[]*/
void	SYSMNTcomdr_Close(void)
{
	/* SCI���荞�ݒ�~ */
	SYSMNTsci_Stop();
}

/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: port_reset()
 *[]----------------------------------------------------------------------[]
 *| summary	: SCI�|�[�g���Z�b�g
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	port_reset(void)
{
	/* SCI���荞�ݒ�~ */
	SYSMNTsci_Stop();
	/* COMDR����f�[�^������ */
	SYSMNTcomdr_Reset();
	/* SCI�|�[�g���Z�b�g�����荞�ݗL���� */
	SYSMNTsci_Start();
}

/*[]----------------------------------------------------------------------[]
 *|	name	: pktchk()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�p�P�b�g�̃t�H�[�}�b�g�`�F�b�N
 *| param	: rcvpkt - ��M�p�P�b�g
 *| return	: TRUE - �p�P�b�g����
 *[]----------------------------------------------------------------------[]*/
BOOL	pktchk(const T_SYSMNT_SCI_PACKET *rcvpkt)
{
	switch (rcvpkt->data[0]) {
	case ACK:	/* no break */
	case NAK:	/* no break */
	case EOT:
		if (rcvpkt->len != 1) {
			return FALSE;
		}
		break;
	case STX:
		if (rcvpkt->len <= 10) {
			return FALSE;		/* �f�[�^���Ȃ� */
		}
		/* �f�[�^�u���b�NNo. */
		if (rcvpkt->data[1] < '0' && '9' < rcvpkt->data[1]) {
			return FALSE;
		}
		if (rcvpkt->data[2] < '0' && '9' < rcvpkt->data[2]) {
			return FALSE;
		}
		/* ����/�ŏI�u���b�N�t���O */
		if (rcvpkt->data[3] != '0') {
			return FALSE;
		}
		if (rcvpkt->data[4] != '0' && rcvpkt->data[4] != '1') {
			return FALSE;
		}
		/* �f�[�^��� */
		if (rcvpkt->data[5] != 'B') {
			return FALSE;
		}
		if (rcvpkt->data[6] != '1') {
			return FALSE;
		}
		/* ETX */
		if (rcvpkt->data[rcvpkt->len-3] != ETX) {
			return FALSE;
		}
		break;
	default:
		return FALSE;		/* �v���g�R���ŃT�|�[�g���Ȃ�����R�[�h */
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: bccchk()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�p�P�b�g��BCC�`�F�b�N
 *| param	: rcvpkt - ��M�p�P�b�g
 *| return	: TRUE - BCC����
 *[]----------------------------------------------------------------------[]*/
BOOL	bccchk(const T_SYSMNT_SCI_PACKET *rcvpkt)
{
	uchar bcc = bcccal((char*)&rcvpkt->data[1], (short)(rcvpkt->len-(1+2)));	/* 1+2 = STX+BCC */
	
	if (bcc != astohx((void*)(&rcvpkt->data[rcvpkt->len-2]))) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: pkt2data()
 *[]----------------------------------------------------------------------[]
 *| summary	: ASCII�t�H�[�}�b�g�̎�M�p�P�b�g���o�C�i���t�H�[�}�b�g�ɕϊ�����
 *| param	: rcv_data - �o�C�i���t�H�[�}�b�g�̃p�P�b�g		<OUT>
 *|			  pkt - ASCII�t�H�[�}�b�g�̃p�P�b�g		<IN>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	pkt2data(T_SYSMNT_PKT *rcv_data, const T_SYSMNT_SCI_PACKET *pkt)
{
	int i, len;
	
	/* �f�[�^���o�C�i���ϊ� */
	rcv_data->type	= pkt->data[0];
	if (rcv_data->type == STX) {
		/* ID1 */
		rcv_data->header.blkno		= (uchar)astoin((void*)(&pkt->data[1]), 2);
		/* ID2 */
		rcv_data->header.blksts		= astohx((void*)(&pkt->data[3]));
		/* ID3 */
		rcv_data->header.data_kind	= astohx((void*)(&pkt->data[5]));
		/* ID4 */
		rcv_data->header.data_format	= astohx((void*)(&pkt->data[7]));
		/* DATA */
		len = pkt->len - SYSMNT_SCI_HEADER_SIZE - 1 - 2;	/* "-1-2"=etx+bcc */
		if (rcv_data->header.data_format & _SYSMNT_DATA_VISIBLE) {
			memcpy(rcv_data->data.data, &pkt->data[SYSMNT_SCI_HEADER_SIZE], (size_t)len);
		}
		else {
			for (i = 0; i < len; i += 2) {
				rcv_data->data.data[i/2] = astohx((void*)(&pkt->data[SYSMNT_SCI_HEADER_SIZE+i]));
			}
			len /= 2;
		}
		/* �f�[�^�� */
		rcv_data->data.len = len;
	}
	else {
		rcv_data->data.len = 0;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: data2pkt()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�f�[�^���p�P�b�g�ɕϊ�����
 *| param	: pkt - �ϊ����ASCII�p�P�b�g		<OUT>
 *|			  send_data - �ϊ��O�̃o�C�i���p�P�b�g	<IN>
 *|			  type - ���M�p�P�b�g�^�C�v(SYSMNT_TYPE_SEL,SYSMNT_TYPE_POL,SYSMNT_TYPE_STX)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	data2pkt(T_SYSMNT_SCI_PACKET *pkt, const T_SYSMNT_PKT *send_data, int type)
{
	int i, len;
	uchar bcc;
	
	len = 0;
	if (type == SYSMNT_TYPE_STX) {
		/* STX */
		pkt->data[len] = STX;
		len++;
		/* ID1 */
		intoas(&pkt->data[len], (ushort)send_data->header.blkno, 2);
		len += 2;
		/* ID2 */
		hxtoas(&pkt->data[len], send_data->header.blksts);
		len += 2;
		/* ID3 */
		hxtoas(&pkt->data[len], send_data->header.data_kind);
		len += 2;
		/* ID4 */
		hxtoas(&pkt->data[len], send_data->header.data_format);
		len += 2;
		/* DATA */
		if (send_data->header.data_format & _SYSMNT_DATA_VISIBLE) {
			for (i = 0; i < send_data->data.len; i++) {
				pkt->data[len++] = send_data->data.data[i];
			}
		}
		else {
			for (i = 0; i < send_data->data.len; i++) {
				hxtoas(&pkt->data[len], send_data->data.data[i]);
				len += 2;
			}
		}
		/* ETX */
		pkt->data[len] = ETX;
		len++;
		/* BCC�Z�b�g */
		bcc = bcccal((char*)&pkt->data[1], (short)(len-1));
		hxtoas(&pkt->data[len], bcc);
		len += 2;
	}
	else if (type == SYSMNT_TYPE_SEL || type == SYSMNT_TYPE_POL) {
		pkt->data[0]	= EOT;
		pkt->data[1]	= '0';
		pkt->data[2]	= '0';
		pkt->data[3]	= '0';
		if (type == SYSMNT_TYPE_POL) {
			pkt->data[4] = '0';
		}
		else {	/* type == SYSMNT_TYPE_SEL */
			pkt->data[4] = '1';
		}
		pkt->data[5]	= ENQ;
		len = 6;
	}
	/* �p�P�b�g�� */
	pkt->len = len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send()
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^���M
 *| param	: data - ���M�f�[�^		<IN>
 *| 		  len - ���M�f�[�^��
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
BOOL	send(uchar *data, int len)
{
	/* �h���C�o�ɑ��M��v�� */
	if (!SYSMNTsci_SndReq(data, len)) {
		port_reset();
		return FALSE;			/* ���M�s�� */
	}
	
	/* �^�C���A�E�g�ݒ� */
	SYSMNTcomdr_sndcmp_timer = _To20msVal(_SYSMNT_COMM_TIMEOUT);
	
	return TRUE;
}

/*====================================================================================[INTERRUPTS]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: int_scircv()
 *[]----------------------------------------------------------------------[]
 *| summary	: 1�o�C�g��M���荞�݃n���h��
 *| param	: rcvdata - ��M����1�o�C�g
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_scircv(uchar rcvdata)
{
	if (SYSMNTcomdr_Ctrl.status == SYSMNT_COMDR_NO_PKT) {
		/* �L�����N�^�ԃ^�C���A�E�g�Ď��J�n */
		SYSMNTcomdr_c2c_timer = _To2msVal(SYSMNT_SCI_C2C_TIMEOUT);
		/* ��M�f�[�^�o�b�t�@�����O */
		if (SYSMNTcomdr_Ctrl.r_packet.len < sizeof(SYSMNTcomdr_Ctrl.r_packet.data)) {
			SYSMNTcomdr_Ctrl.r_packet.data[SYSMNTcomdr_Ctrl.r_packet.len++] = rcvdata;
		}
		else {
			SYSMNTcomdr_Ctrl.status = SYSMNT_COMDR_ERR_INVALID_LEN;		/* �p�P�b�g���ُ� */
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: int_scierr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �ʐM�G���[���荞�݃n���h��
 *| param	: errtype - ���������ʐM�G���[
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_scierr(uchar errtype)
{
	switch (errtype) {
	case SYSMNT_SCI_ERR_OVERRUN:
		break;
	case SYSMNT_SCI_ERR_FRAME:
		break;
	case SYSMNT_SCI_ERR_PARITY:
	default:
		break;
	}
	SYSMNTcomdr_Ctrl.status = SYSMNT_COMDR_ERR_SCI;
}

