// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/*[]----------------------------------------------------------------------[]
 *|	filename: NTComComdr.c
 *[]----------------------------------------------------------------------[]
 *| summary	: SCI�h���C�o(1�p�P�b�g���o�@�\)
 *| author	: machida.k
 *| date	: 2005-06-10
 *| update	:
 *|				2005-12-20			machida.k	RAM���L��
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"



/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
static	void	NTComComdr_PortReset(void);
static	void	NTComComdr_Reset(void);

static	void	int_scircv(uchar rcvdata);
static	void	int_scierr(uchar errtype);
static	void	int_comdrtimer_c2c(void);
static	void	int_comdrtimer_sndcmp(void);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: NTComComdr���W���[��������
 *| param	: timeout_c2c - �L�����N�^�ԃ^�C���A�E�g�l [msec]
 *|			  speed (bps) 0=1200, 1=2400
 *|			  dbit  (Data bits length) 0=8bits, 1=7bits
 *|			  sbit  (Stop bits length) 0=1bit , 1=2bits
 *|			  pbit  (Parity bit kind ) 0=none , 1=odd  ,2=even
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComComdr_Init(ushort timeout_c2c, uchar port, uchar speed, uchar dbit, uchar sbit, uchar pbit)
{
	/* SCI�C���^�t�F�[�X������(���荞�ݖ���) */
	NTComSci_Init(int_scircv, int_scierr, port, speed, dbit, sbit, pbit);
	/* �^�C���A�E�g�l�ۑ� */
	NTComComdr_Ctrl.timer_c2c.timeout	= (timeout_c2c + 1) / NT_C2C_TIMINT;
	/* ����f�[�^������ */
	NTComComdr_Reset();
	/* �^�C�}�[�N���G�C�g���X�^�[�g */
	NTComComdr_Ctrl.timer_c2c.timerid	= NTComTimer_Create(1, NT_C2C_TIMINT, int_comdrtimer_c2c, TRUE);
	NTComTimer_Start(NTComComdr_Ctrl.timer_c2c.timerid);
	NTComComdr_Ctrl.timer_sndcmp.timerid	= NTComTimer_Create(10, 10, int_comdrtimer_sndcmp, TRUE);
	NTComTimer_Start(NTComComdr_Ctrl.timer_sndcmp.timerid);
	/* SCI���荞�ݗL���� */
	NTComSci_Start();
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_Reset()
 *[]----------------------------------------------------------------------[]
 *| summary	: NTComComdr���W���[�����Z�b�g
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComComdr_Reset(void)
{
	volatile unsigned long	ist;			// ���݂̊�����t���

	ist = _di2();
	NTComComdr_Ctrl.status			= NT_COMDR_NO_PKT;
	NTComComdr_Ctrl.r_rcvlen			= 0;
	NTComComdr_Ctrl.r_datalen			= 0;
	NTComComdr_Ctrl.timer_c2c.count	= -1;
	NTComComdr_Ctrl.timer_sndcmp.count	= 0;
	_ei2(ist);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_GetRcvPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�p�P�b�g�擾
 *| param	: buf - ��M�p�P�b�g�o�b�t�@	<OUT>
 *|					(�߂�l��NTCOM_SCI_PKT_RCVD�̂Ƃ��L��)
 *| return	: COMDR�X�e�[�^�X
 *[]----------------------------------------------------------------------[]*/
eNT_COMDR_STS	NTComComdr_GetRcvPacket(T_NT_BLKDATA *buf)
{
	eNT_COMDR_STS	ret;

	ret = (eNT_COMDR_STS)NTComComdr_Ctrl.status;

	switch (ret) {
	case NT_COMDR_NO_PKT:					/* �A�C�h�� */
		break;
	case NT_COMDR_PKT_RCVING:				/* �p�P�b�g��M�� */
		if (NTComComdr_Ctrl.timer_c2c.count >= NTComComdr_Ctrl.timer_c2c.timeout) {
			/* �L�����N�^�ԃ^�C���A�E�g���� */
			ret = NT_COMDR_ERR_TIMEOUT;
			NTComComdr_Reset();
		}
		break;
	case NT_COMDR_PKT_RCVD:					/* �p�P�b�g��M���� */
		memcpy(buf->data, &NTComComdr_Ctrl.r_buf, NTComComdr_Ctrl.r_rcvlen);
		buf->len = NTComComdr_Ctrl.r_rcvlen;
		NTComComdr_Reset();
		break;
	default:								/* ��M�G���[���� */
		NTComComdr_Reset();
		break;
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_SendPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�P�b�g���M(���M�������^�C���A�E�g�܂Ń��^�[�����Ȃ�))
 *| param	: buf - ���M�p�P�b�g	<IN>
 *|			  wait - ���M�J�n�҂�����				[msec]
 *|			  timeout - ���M�����҂��^�C���A�E�g	[msec]
 *| return	: ���M����
 *[]----------------------------------------------------------------------[]*/
eNT_COMDR_SND	NTComComdr_SendPacket(T_NT_BLKDATA *buf, int wait, int timeout)
{
	/* ���M�J�n�҂� */
	xPause1ms(wait);

	/* �h���C�o�ɑ��M��v�� */
	if (!NTComSci_SndReq(buf->data, buf->len)) {
		NTComComdr_PortReset();
		return NT_COMDR_SND_ERR;			/* ���M�s�� */
	}
	/* �^�C���A�E�g�ݒ� */
	NTComComdr_Ctrl.timer_sndcmp.count	= _To10msVal(timeout);

	/* ���M�����҂� */
	while (NTComComdr_Ctrl.timer_sndcmp.count > 0) {
		taskchg( IDLETSKNO );
		NTCom_TimoutChk();// �^�C�}�[�`�F�b�N����
		if (NTComSci_IsSndCmp()){
			NTComComdr_Ctrl.timer_sndcmp.count = 0;
			return NT_COMDR_SND_NORMAL;		/* ���M���� */
		}
	}
	NTComComdr_PortReset();

	return NT_COMDR_SND_TIMEOUT;			/* �^�C���A�E�g */
}

/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_PortReset()
 *[]----------------------------------------------------------------------[]
 *| summary	: SCI�|�[�g���Z�b�g
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComComdr_PortReset(void)
{
	/* SCI���荞�ݖ����� */
	NTComSci_Stop();
	/* COMDR����f�[�^������ */
	NTComComdr_Reset();
	/* SCI�|�[�g���Z�b�g�����荞�ݗL���� */
	NTComSci_Start();
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
	/* �L�����N�^�ԃ^�C���A�E�g�Ď��J�n */
	NTComComdr_Ctrl.timer_c2c.count = 0;

	if (NTComComdr_Ctrl.status == NT_COMDR_NO_PKT) {
		NTComComdr_Ctrl.status = NT_COMDR_PKT_RCVING;
	}

	if (NTComComdr_Ctrl.status == NT_COMDR_PKT_RCVING) {

		if (NTComComdr_Ctrl.r_rcvlen < 7) {
		/* �p�P�b�g�擪7�o�C�g(5�o�C�g�̃V�O�l�`���{2�o�C�g�̃f�[�^��)��M�t�F�[�Y */
			if (NTComComdr_Ctrl.r_rcvlen < 5) {
			/* �d���擪�̃V�O�l�`���`�F�b�N */
				if (rcvdata != NT_PKT_SIG[NTComComdr_Ctrl.r_rcvlen]) {
					/* ��M�p�P�b�g��1���猟�o���Ȃ��� */
					if (rcvdata == NT_PKT_SIG[0]) {
						/* �p�P�b�g�̐擪�f�[�^��������Ȃ��̂� */
						NTComComdr_Ctrl.r_buf[0] = rcvdata;
						NTComComdr_Ctrl.r_rcvlen = 1;
					}
					else {
						NTComComdr_Ctrl.r_rcvlen = 0;
					}
					return;
				}
			}
			NTComComdr_Ctrl.r_buf[NTComComdr_Ctrl.r_rcvlen++] = rcvdata;
			if (NTComComdr_Ctrl.r_rcvlen >= 7) {
			/* �f�[�^���擾 */
				NTComComdr_Ctrl.r_datalen = NT_MakeWord(&NTComComdr_Ctrl.r_buf[5]);
				if (NTComComdr_Ctrl.r_datalen >= NT_BLKSIZE - NT_PKT_CRC_SIZE) {
					NTComComdr_Ctrl.status = NT_COMDR_ERR_INVALID_LEN;
				}
				/* �p�P�b�g�f�[�^&CRC�f�[�^��M�t�F�[�Y�� */
			}
		}
		else {
		/* �p�P�b�g�f�[�^&CRC�f�[�^��M�t�F�[�Y */
			NTComComdr_Ctrl.r_buf[NTComComdr_Ctrl.r_rcvlen++] = rcvdata;
			if (NTComComdr_Ctrl.r_rcvlen >= NTComComdr_Ctrl.r_datalen + NT_PKT_CRC_SIZE) {
				/* 1�p�P�b�g��M���� */
				NTComComdr_Ctrl.timer_c2c.count = -1;
				NTComComdr_Ctrl.status = NT_COMDR_PKT_RCVD;
			}
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
	case NT_SCI_ERR_OVERRUN:
		NTCom_ComErr.OVRcnt++;
		break;
	case NT_SCI_ERR_FRAME:
		NTCom_ComErr.FRMcnt++;
		break;
	case NT_SCI_ERR_PARITY:
		NTCom_ComErr.PRYcnt++;
	default:
		break;
	}
	NTComComdr_Ctrl.status = NT_COMDR_ERR_SCI;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: int_comdrtimer_c2c()
 *[]----------------------------------------------------------------------[]
 *| summary	: �L�����N�^�[�ԃ^�C���A�E�g�Ď��p�^�C�}�[���荞�݃n���h��
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_comdrtimer_c2c(void)
{
	if (NTComComdr_Ctrl.timer_c2c.count >= 0) {
		if (NTComComdr_Ctrl.timer_c2c.count < 0x7fffffff) {
			NTComComdr_Ctrl.timer_c2c.count++;
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: int_comdrtimer_sndcmp()
 *[]----------------------------------------------------------------------[]
 *| summary	: 100ms�^�C�}�[���荞�݃n���h��
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_comdrtimer_sndcmp(void)
{
	if (NTComComdr_Ctrl.timer_sndcmp.count > 0) {
		NTComComdr_Ctrl.timer_sndcmp.count--;
	}
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
