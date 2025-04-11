// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/*[]----------------------------------------------------------------------[]
 *|	filename: ntcomsci.c
 *[]----------------------------------------------------------------------[]
 *| summary	: SCI�h���C�o
 *| author	: machida.k
 *| date	: 2005.05.31
 *| update	:
 *|		2005-12-20			machida.k	RAM���L��
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"


/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/
#define	NTCom_Enable_RS485Driver(bEnable)	(CP_NT_RTS=!bEnable)

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/


/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: �|�[�g������
 *|			  (�|�[�g�̓�����J�n������ɂ͕ʓrNTComSci_Start()�̃R�[�����K�v)
 *| param	: RcvFunc - �f�[�^��M�ʒm�p�̃R�[���o�b�N�֐��|�C���^
 *|			  StsFunc - �f�[�^��M�G���[�ʒm�p�R�[���o�b�N�֐��|�C���^
 *|			  Speed (bps) 0=1200, 1=2400
 *|			  Dbits (Data bits length) 0=8bits, 1=7bits
 *|			  Sbits (Stop bits length) 0=1bit , 1=2bits
 *|			  Pbit  (Parity bit kind ) 0=none , 1=odd  ,2=even
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComSci_Init(T_NT_SCI_CALLBACK RcvFunc, T_NT_SCI_CALLBACK StsFunc,
				uchar Port, uchar Speed, uchar Dbits, uchar Sbits, uchar Pbit)
{

	/* �Ώۃ|�[�g��SCI�����~ */
	NTComSci_Stop();

	NTCom_Enable_RS485Driver(0);				// (���MRS485�h���C�o)�g�����X�~�b�^���o�͋֎~

	/* �p�����[�^�ۑ� */
	NTComSci_Ctrl.port_no = Port;
	NTComSci_Ctrl.speed	= Speed;
	NTComSci_Ctrl.databit	= Dbits;
	NTComSci_Ctrl.stopbit	= Sbits;
	NTComSci_Ctrl.parity	= Pbit;
	NTComSci_Ctrl.rcvfunc = RcvFunc;
	NTComSci_Ctrl.stsfunc = StsFunc;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_Start()
 *[]----------------------------------------------------------------------[]
 *| summary	: �|�[�g����J�n
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComSci_Start(void)
{
	volatile unsigned char	i;
	uchar	ists;

	ists = _di2();												/* disable interrupt */

	/* �R���g���[���f�[�^�̏����� */
	NTComSci_Ctrl.slen		= 0;
	NTComSci_Ctrl.scomplete	= TRUE;

	//  SCI initial
	MSTP_SCI9 = 0;							// SCI9 module stop mode release(SYSTEM.MSTPCRB.BIT.MSTPB27)
	SCI9.SEMR.BIT.ABCS = 0x01;			/* ����������{�N���b�N�Z���N�gBIT	*/
	SCI9.SCR.BYTE = 0x00;				// �V���A�� �R���g���[�� ���W�X�^ �������N���A(���荞�݋֎~)

	_ei2(ists);												/* enable interrupt */

	SCI9.SMR.BIT.CM = 0;					// Async
	SCI9.SMR.BIT.MP = 0;					// Multi pro func : 0=Nouse, 1=Use

	/* �f�[�^�r�b�g�� */
    if( NTComSci_Ctrl.databit == 1 ) {
        SCI9.SMR.BIT.CHR  = 1;    /* 7 bits */
    }
    else {
        SCI9.SMR.BIT.CHR  = 0;    /* 8 bits */
    }

    /* �X�g�b�v�r�b�g�� */
    if( NTComSci_Ctrl.stopbit == 1 ) {
        SCI9.SMR.BIT.STOP = 1;    /* 2 bits */
    }
    else {
        SCI9.SMR.BIT.STOP = 0;    /* 1 bit */
    }

    /* �p���e�B */
    if( NTComSci_Ctrl.parity == 0 ) {
        SCI9.SMR.BIT.PE = 0;			// �p���e�B�Ȃ�
    }
    else {
        SCI9.SMR.BIT.PE = 1;			// �p���e�B�L��
        if( NTComSci_Ctrl.parity == 1 ) {
            SCI9.SMR.BIT.PM = 1;    // �
        }
        else {
            SCI9.SMR.BIT.PM = 0;    // ����
        }
    }

	/* �`�����x */
	switch( NTComSci_Ctrl.speed ){
	case 0:		/* 38400 bps */
		/* no break */
	default:
		SCI9.SMR.BIT.CKS = 0;
		SCI9.BRR = SCI_38400BPS_n0;
		break;
	case 1:		/* 19200 bps */
		SCI9.SMR.BIT.CKS = 0;
		SCI9.BRR = SCI_19200BPS_n1;
		break;
	case 2:		/* 9600 bps */
		SCI9.SMR.BIT.CKS = 0;
		SCI9.BRR = SCI_9600BPS_n1;
		break;
	case 3:		/* 4800 bps */
		SCI9.SMR.BIT.CKS = 0;
		SCI9.BRR = SCI_4800BPS_n1;
		break;
	}

	wait2us( 125L );						// 1bit time over wait
	i = SCI9.SSR.BYTE;					// Dummy Read
	SCI9.SSR.BYTE = 0xC4;				// Status register <- 0
	dummy_Read = SCI9.SSR.BYTE;			// Dummy Read
	SCI9.SCR.BYTE = 0x70;				// RIE&TE&RE Enable(RIE=1&����M����)

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_SndReq()
 *[]----------------------------------------------------------------------[]
 *| summary	: �V���A�����M���J�n����
 *|			  ���M������NTComSci_IsSndCmp()�Ŏ擾�ł���
 *| param	: buf - ���M�f�[�^
 *|			  len - ���M�f�[�^��
 *| return	: TRUE as success, FALSE as ���M�r�W�[
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComSci_SndReq(uchar *buf, ushort len)
{
	SCI9.SCR.BIT.TE = 1;
	if (SCI9.SSR.BIT.TEND) {
		/* ���M�\ */
		NTCom_Enable_RS485Driver(1);				/* (���MRS485�h���C�o)�g�����X�~�b�^���o�͋��� */

		NTComSci_Ctrl.scomplete			= FALSE;	/* ���M�����t���O������ */
		NTComSci_Ctrl.sdata				= buf;		/* ���M�f�[�^ */
		NTComSci_Ctrl.scnt				= 0;		/* ���M�����f�[�^�� */
		NTComSci_Ctrl.slen				= len;		/* ���M�f�[�^�� */

		SCI9.TDR			= buf[0];				/* ���M�f�[�^1byte�ڂ��Z�b�g */
		SCI9.SSR.BIT.TEND	= 0;
		dummy_Read = SCI9.SSR.BYTE;					// Dummy Read
		SCI9.SCR.BIT.TIE	= 1;					/* ���M���荞�݃C�l�[�u�� */

		return TRUE;
	}
	else {
		/* ���M�s�� */
		SCI9.SCR.BIT.TEIE	= 0;
		SCI9.SCR.BIT.TIE	= 0;					/* ���M���荞�݃f�B�Z�[�u�� */
		dummy_Read = SCI9.SSR.BYTE;					// Dummy Read
		NTComSci_Ctrl.scomplete	= TRUE;				/* �����I�ɑ��M�����Ƃ��� */

		return FALSE;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_IsSndCmp()
 *[]----------------------------------------------------------------------[]
 *| summary	: �V���A�����M�������擾����
 *| param	: none
 *| return	: TRUE as ���M����
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComSci_IsSndCmp(void)
{
	return NTComSci_Ctrl.scomplete;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_Stop()
 *[]----------------------------------------------------------------------[]
 *| summary	: �V���A������M��~
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComSci_Stop(void)
{
	_di();
	NTCom_Enable_RS485Driver(0);	/* (���MRS485�h���C�o)�g�����X�~�b�^���o�͋֎~ */

	SCI9.SCR.BIT.TIE  = 0;			/* ���M�f�[�^�G���v�e�B(TXI)���荞�݋֎~ */
	SCI9.SCR.BIT.RIE  = 0;			/* ��M�����݋֎~ */
	SCI9.SCR.BIT.TE   = 0;			/* ���M����֎~ */
	SCI9.SCR.BIT.RE   = 0;			/* ��M����֎~ */
	SCI9.SCR.BIT.TEIE = 0;			/* ���M�I��(TEI)���荞�݋֎~ */
	_ei();
}


/*==================================================================================[ISR (RX)]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Int_RXI()
 *[]----------------------------------------------------------------------[]
 *| summary	: �P�o�C�g��M�������荞�ݏ���
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Int_RXI(void)
{
	if((SCI9.SSR.BIT.ORER==0) && (SCI9.SSR.BIT.FER==0) && (SCI9.SSR.BIT.PER==0)) {
		NTComSci_Ctrl.rcvfunc(SCI9.RDR);
	}

	/* �f�[�^��M�t���O�N���A */
	SCI9.SSR.BYTE = 0;
}

/*==============================================================================[ISR (RX ERR)]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Int_ERI()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�G���[���荞�ݏ���
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Int_ERI(void)
{
    /* �I�[�o�[�����G���[ */
    if (SCI9.SSR.BIT.ORER) {
        NTComSci_Ctrl.stsfunc(NT_SCI_ERR_OVERRUN);
    }

    /* �t���[�~���O�G���[ */
    if (SCI9.SSR.BIT.FER) {
        NTComSci_Ctrl.stsfunc(NT_SCI_ERR_FRAME);
    }

    /* �p���e�B�G���[ */
    if (SCI9.SSR.BIT.PER) {
        NTComSci_Ctrl.stsfunc(NT_SCI_ERR_PARITY);
    }

    /* �G���[���N���A */
    SCI9.SSR.BYTE &= 0xC7;
    dummy_Read = SCI9.SSR.BYTE;			// Dummy Read
}

/*==================================================================================[ISR (TX)]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Int_TXI()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�f�[�^�G���v�e�B���荞�ݏ���
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Int_TXI(void)
{
	if (++NTComSci_Ctrl.scnt < NTComSci_Ctrl.slen) {
	/* ���M�f�[�^�L�� */
		SCI9.TDR			= NTComSci_Ctrl.sdata[NTComSci_Ctrl.scnt];
		SCI9.SSR.BIT.ORER= 0;							/* send start					*/
		SCI9.SSR.BIT.FER= 0;								/* 								*/
		SCI9.SSR.BIT.PER= 0;								/* 								*/
		dummy_Read = SCI9.SSR.BYTE;						// Dummy Read
	}
	else {
	/* �S�f�[�^���M���� */
		SCI9.SCR.BIT.TIE	= 0;		/* ���荞�݃f�B�Z�[�u�� */
		SCI9.SCR.BIT.TEIE = 1;				// TEND���荞�݋���
	}
}

/*==============================================================================[ISR (TX END)]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Int_TEI()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�I�����荞�ݏ���
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Int_TEI(void)
{
	NTCom_Enable_RS485Driver(0);				/* (���MRS485�h���C�o)�g�����X�~�b�^���o�͋֎~ */

	SCI9.SCR.BIT.TEIE = 0;						/* ���荞�݃f�B�Z�[�u�� */
	SCI9.SCR.BIT.TE = 0;
	NTComSci_Ctrl.scomplete			= TRUE;		/* ���M�����t���OON */
}

// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

