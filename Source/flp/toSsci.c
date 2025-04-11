/*[]----------------------------------------------------------------------[]*/
/*|		�Ύq�@�V���A���ʐM�p�h���C�o									   |*/
/*|		�ESCI CH-4 ���g�p���܂��B										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"toS.h"
#include	"toSdata.h"

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/
#define	IFMCOM_ERROR_COUNT_LIMIT	(ushort)9999	/* server no answer */


/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
#define	toS_Enable_RS485Driver(bEnable)	(CP_IF_RTS=!bEnable)

/***************************/
/***   receive process	 ***/
/***************************/
/*[]----------------------------------------------------------------------[]*/
/*|  [API]      toSsci_Init                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�ΐe�@�ʐM�|�[�g������                                             |*/
/*|		�ESCI ch-4�̏�����                                                 |*/
/*|		�E����M����  ������                                               |*/
/*|		�E��M���荞�݁�����                                               |*/
/*|		�E���M���荞�݁��֎~�i���̓s�x�A����/�֎~�𐧌䂷��j              |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : Speed (�`�����x)         4=2400,  2=9600, 1=19200, 0=38400bps |*/
/*|			 Dbits (�f�[�^�r�b�g��)   0=8bits, 1=7bits                     |*/
/*|          Sbits (�X�g�b�v�r�b�g��) 0=1bit , 1=2bits                     |*/
/*|          Pbit  (�����p���e�B)     0=���� , 1=�, 2=����              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-21                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSsci_Init(unsigned char Speed,
					unsigned char Dbits,
					unsigned char Sbits,
					unsigned char Pbit)
{

	volatile unsigned char	i;
	unsigned long	ists;
	
	toS_Enable_RS485Driver(0);				// (���MRS485�h���C�o)�g�����X�~�b�^���o�͋֎~

	// ��M�Ǘ�����������
	toS_RcvCtrl.RcvCnt      = 0;
	toS_RcvCtrl.OvfCount    = 0;
	toS_RcvCtrl.ComerrStatus = 0;

	// ���M�Ǘ�����������
	toS_SndCtrl.SndReqCnt = 0;
	toS_SndCtrl.SndCmpCnt = 0;

	// ��M�G���[�Ǘ�����������
	toS_RcvErr.usOVRcnt = 0;				// �I�[�o�[�����G���[�����񐔂��N���A
	toS_RcvErr.usFRMcnt = 0;				// �t���[�~���O�G���[�����񐔂��N���A
	toS_RcvErr.usPRYcnt = 0;				//     �p���e�B�G���[�����񐔂��N���A

	ists = _di2();							// ���荞�݋֎~

	//  SCI initial 
	MSTP_SCI4 = 0;							// SCI4 module stop mode release(SYSTEM.MSTPCRB.BIT.MSTPB27)
	P_SCI_L.SEMR.BIT.ABCS = 0x01;			/* ����������{�N���b�N�Z���N�gBIT	*/
	P_SCI_L.SCR.BYTE = 0x00;				// �V���A�� �R���g���[�� ���W�X�^ �������N���A(���荞�݋֎~)

	_ei2( ists );
	
	P_SCI_L.SMR.BIT.CM = 0;					// Async
	P_SCI_L.SMR.BIT.MP = 0;					// Multi pro func : 0=Nouse, 1=Use

	// �f�[�^��(0:8Bit, 1:7Bit) 
	if( Dbits == 1 ){
		P_SCI_L.SMR.BIT.CHR  = 1;			// 7 bits 
	}else{
		P_SCI_L.SMR.BIT.CHR  = 0;			// 8 bits 
	}

	// �X�g�b�v�r�b�g��(0:1Bit, 1:2Bit) 
	if( Sbits == 1 ){
		P_SCI_L.SMR.BIT.STOP = 1;			// 2 bits
	}else{
		P_SCI_L.SMR.BIT.STOP = 0;			// 1 bit
	}

	// �p���e�B(0:NONE, 1:Odd, 2:Even) 
	switch (Pbit) {
		case 0:
		default:
			P_SCI_L.SMR.BIT.PE = 0;			// �p���e�B�Ȃ�
			break;
		case 1:
			P_SCI_L.SMR.BIT.PE = 1;			// �p���e�B�L��
			P_SCI_L.SMR.BIT.PM = 1;			// �
			break;
		case 2:
			P_SCI_L.SMR.BIT.PE = 1;			// �p���e�B�L��
			P_SCI_L.SMR.BIT.PM = 0;			// ����
			break;
	}

	// �`�����x(bps) 
	switch (Speed) {
		case 1:
			P_SCI_L.SMR.BIT.CKS = 1;		// Clock select : 0=�����Ȃ�, 1=����/4
			P_SCI_L.BRR = SCI_19200BPS_n1;
			break;
		case 2:
			P_SCI_L.SMR.BIT.CKS = 1;		// Clock select : 0=�����Ȃ�, 1=����/4
			P_SCI_L.BRR = SCI_9600BPS_n1;
			break;
		default:// 0
			P_SCI_L.SMR.BIT.CKS = 0;		// Clock select : 0=�����Ȃ�, 1=����/4
			P_SCI_L.BRR = SCI_38400BPS_n0;
			break;
	}

	wait2us( 125L );						// 1bit time over wait
	i = P_SCI_L.SSR.BYTE;					// Dummy Read
	P_SCI_L.SSR.BYTE = 0xC4;				// Status register <- 0
	dummy_Read = P_SCI_L.SSR.BYTE;			// Dummy Read
	P_SCI_L.SCR.BYTE = 0x70;				// RIE&TE&RE Enable(RIE=1&����M����)
}

/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toS_Int_RXI2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|     RXI���荞�݃n���h��                                                |*/
/*|		�E��M�f�[�^���W�X�^�E�t�����荞��                                 |*/
/*|		�ESYS\API_cstm.c �ɂ���{���̃n���h���Fx_Int_RXI2()����Ăяo�����|*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_Int_RXI2( void )
{
	unsigned char   rcvdat;

	if((P_SCI_L.SSR.BIT.ORER==0) && (P_SCI_L.SSR.BIT.FER==0) && (P_SCI_L.SSR.BIT.PER==0)) {
		rcvdat = P_SCI_L.RDR;
toS_Sci_Rcv_10:
		if (TOS_SCI_RCVBUF_SIZE > toS_RcvCtrl.RcvCnt) {
			toS_RcvBuf[toS_RcvCtrl.RcvCnt++] = rcvdat;		// ��M�o�b�t�@�֓]����A��M�f�[�^���J�E���g�A�b�v
		} else {											// ��M�o�b�t�@�̓I�[�o�[�t���[
			toS_RcvBuf[0] = rcvdat;							// ��M�f�[�^�́A�o�b�t�@�̐擪��
			toS_RcvCtrl.RcvCnt     = 1;						// ��M�f�[�^�����`����
			if (0xffff != toS_RcvCtrl.OvfCount) {
				++toS_RcvCtrl.OvfCount;
			}
		}
		toScom_2msTimerStart(toS_RcvCtrl.usBetweenChar); 	// �����ԊĎ��^�C�}�[�J�n�i�ĊJ�j
		P_SCI_L.SSR.BYTE = 0;								// ���荞�ݗv���̃N���A
		dummy_Read = P_SCI_L.SSR.BYTE;						// Dummy Read
	}
	else{													// RDRF=0, ORER=1, FER=0, PER=0
		rcvdat = 0xff;
		goto toS_Sci_Rcv_10;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toS_Int_ERI2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|     ��M�G���[���荞�݃n���h��                                         |*/
/*|		�ESYS\API_cstm.c �ɂ���{���̃n���h���Fx_Int_ERI2()����Ăяo�����|*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_Int_ERI2(void)
{
/*
*	�I�[�o�[�����G���[�F
*		�E�u�V�t�g���W�X�^�v�֎�M�����f�[�^���u�f�[�^���W�X�^�v�֓]������O�ɁA
*		  ���̎�M�f�[�^���u�V�t�g���W�X�^�v�֓����Ă����B
*		�E�u�f�[�^���W�X�^�v�ւ̓]����A��M�f�[�^�t�����荞�݂𔭐�������̂�
*		  ���̊��荞�݃n���h�������܂��@�\���Ă��Ȃ��ꍇ�ɔ�������B
*
*	�t���[�~���O�G���[�F
*		�E�X�^�[�g�r�b�g�{�f�[�^���{�p���e�B�{�X�g�b�v�r�b�g���t���[���A�����
*		  �r�b�g���[�g���A���M���Ǝ�M���ň�v���Ă��Ȃ��ꍇ�ɔ�������B
*
*	�p���e�B�G���[�F
*		�ESCI���s�������p���e�B���o�ɂĈُ킪������(�ʐM�ُ�)��A
*		  �p���e�B���A���M���Ǝ�M���ň�v���Ă��Ȃ��ꍇ�ɔ�������B
*/
	if( P_SCI_L.SSR.BIT.ORER ) {
		toS_RcvCtrl.ComerrStatus |= 0x01;	/* over run set	*/
		if( toS_RcvErr.usOVRcnt < IFMCOM_ERROR_COUNT_LIMIT ){
			toS_RcvErr.usOVRcnt++;			/* over run counter inc */
		}
	}
	if( P_SCI_L.SSR.BIT.FER ) {
		toS_RcvCtrl.ComerrStatus |= 0x02;	/* frame error set		*/
		if( toS_RcvErr.usFRMcnt < IFMCOM_ERROR_COUNT_LIMIT ){
			toS_RcvErr.usFRMcnt++;			/* frame error counter inc */
		}
	}
	if( P_SCI_L.SSR.BIT.PER ) {
		toS_RcvCtrl.ComerrStatus |= 0x04;	/* parity error set		*/
		if( toS_RcvErr.usPRYcnt < IFMCOM_ERROR_COUNT_LIMIT ){
			toS_RcvErr.usPRYcnt++;			/* parity error counter inc */
		}
	}

	/*�i�S���܂Ƃ߂āj�G���[�t���O�N���A */
	P_SCI_L.SSR.BYTE &= 0xc7;
	dummy_Read = P_SCI_L.SSR.BYTE;			// Dummy Read
}


/***************************/
/***	send process	 ***/
/***************************/
/*[]----------------------------------------------------------------------[]*/
/*|  [API]      toSsci_SndReq                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���M�J�n                                                           |*/
/*|		�E���M�������f�[�^�́AtoM_SndBuf[]�֊��ɃZ�b�g����Ă���O��ł��B |*/
/*[]----------------------------------------------------------------------[]*/
/*| ���� :	 ���M�������f�[�^�����n����܂��B							   |*/
/*| �߂�l : ���M�J�n����												   |*/
/*|				1=����I���i���M�J�n�ł����j                               |*/
/*|				0=�ُ�I���i���M�J�n�ł��Ȃ������j                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-05-12   �����E����                                 |*/
/*| 				�E�߂�l��Boolean�^�ɂ���                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toSsci_SndReq( unsigned short Length )
{
	/*�u�O�o�C�g����I�v�ŗv�����ꂽ�Ƃ��̃Z�[�t�e�B�@�\ */
	if (Length == 0) {
		toS_SndCtrl.SndCmpFlg = 1;				/* ���M���Ă��Ȃ����Ǒ��M��������(�����t���O��ON) */
		return (uchar)1;						/* ����I���i���M�J�n�ł����j��Ԃ� */
	}

	P_SCI_L.SCR.BIT.TE = 1;
	/* ���M�ł���H�i���M�f�[�^���W�X�^�E�G���v�e�B�H�j*/
	if( 0 != P_SCI_L.SSR.BIT.TEND ){
		toS_Enable_RS485Driver(1);				/* (���MRS485�h���C�o)�g�����X�~�b�^���o�͋��� */

		toS_SndCtrl.SndCmpFlg = 0;				/* ���M�J�n(�����t���O��OFF) */
		toS_SndCtrl.SndReqCnt = Length;			/* ���M�v���f�[�^�����Z�b�g */
		toS_SndCtrl.SndCmpCnt = 0;				/* ���M�����f�[�^�������Z�b�g */
		toS_SndCtrl.ReadIndex = 1;				/* �ŏ��̂P�����]�������ꂩ����̂� */

		P_SCI_L.TDR = toS_SndBuf[0];			// �ŏ��̂P�����𑗐M�o�b�t�@����]��
		dummy_Read = P_SCI_L.SSR.BYTE;			// Dummy Read
		P_SCI_L.SCR.BIT.TIE = 1;				// TXI���荞�݋���
		return (uchar)1;						/* ����I���i���M�J�n�ł����j��Ԃ� */
	}
	/* ���M�ł��Ȃ��i���M�f�[�^���W�X�^����ł͂Ȃ��j*/
	else {
		toS_SndCtrl.SndCmpFlg = 1;				/* ���M�ł��Ȃ����Ǒ��M��������(�����t���O��ON) */
		P_SCI_L.SCR.BIT.TEIE = 0;				// TEND���荞�݋֎~ 
		P_SCI_L.SCR.BIT.TIE  = 0;				//  TXI���荞�݋֎~ 
		dummy_Read = P_SCI_L.SSR.BYTE;			// Dummy Read
		return (uchar)0;                        /* �ُ�I���i���M�J�n�ł��Ȃ������j��Ԃ� */
	}
	return (uchar)1;
}


/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toS_Int_TXI2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���M�i���M�f�[�^���W�X�^��j���荞�݃n���h��                       |*/
/*|		�ESYS\API_cstm.c �ɂ���{���̃n���h���Fx_Int_TXI2()����Ăяo�����|*/
/*|		�E���M�f�[�^���W�X�^�֓]���\�ƂȂ����Ƃ��A�Ăяo����܂��B       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_Int_TXI2( void )
{
	++toS_SndCtrl.SndCmpCnt;					/* ���M�����f�[�^���J�E���g�i�A�b�v�j*/

	/* �܂����M�f�[�^���c���Ă���H*/
	if (toS_SndCtrl.SndReqCnt > toS_SndCtrl.SndCmpCnt) {
		P_SCI_L.TDR = toS_SndBuf[ toS_SndCtrl.ReadIndex++];	// ���M�o�b�t�@����]�� 
		P_SCI_L.SSR.BIT.ORER= 0;							/* send start					*/
		P_SCI_L.SSR.BIT.FER= 0;								/* 								*/
		P_SCI_L.SSR.BIT.PER= 0;								/* 								*/
		dummy_Read = P_SCI_L.SSR.BYTE;						// Dummy Read
	}
	/* �����S�đ��M�f�[�^��]�������B���Ǐo���؂����󂶂�Ȃ� */
	else {
		P_SCI_L.SCR.BIT.TIE  = 0;				//�i���g�́jTXI���荞�݋֎~ 
		P_SCI_L.SCR.BIT.TEIE = 1;				// TEND���荞�݋��� 

/* �������邱�Ƃŏo���؂�������͎��́uTEND���荞�݁v�ɔC���� */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toS_Int_TEI2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���M�I���iTEND=�g�����X�~�b�g�E�G���h�j���荞�݃n���h��            |*/
/*|		�E�V�t�g���W�X�^����O���֑��M���؂����Ƃ��A�Ăяo����܂��B       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_Int_TEI2( void )
{
	/* �����ɗ�����u�S�đ��M�f�[�^���o���؂����v�Ƃ������� */
	toS_Enable_RS485Driver(0);				/* (���MRS485�h���C�o)�g�����X�~�b�^���o�͋֎~ */
	P_SCI_L.SCR.BIT.TEIE = 0;				//�i���g�́jTEND���荞�݋֎~
	P_SCI_L.SCR.BIT.TE = 0;
	toS_SndCtrl.SndCmpFlg = 1;				/* ���M��������(�����t���O��ON) */
}


/*[]----------------------------------------------------------------------[]*/
/*|  [API]      toSsci_IsSndCmp                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*|		���M��������                                                       |*/
/*|		�E���M�����������H�ۂ���Ԃ��܂��B                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| �߂�l : ���M�������ʁiBoolean�j									   |*/
/*|				1=���M����												   |*/
/*|				0=���M������											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	toSsci_IsSndCmp( void )
{
	if (1 == toS_SndCtrl.SndCmpFlg) {
		return (unsigned char)1;
	}
	return (unsigned char)0;
}


/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toSsci_Stop		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|         SCI send & receive stop			                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSsci_Stop( void )
{
	_di();
	toS_Enable_RS485Driver(0);		/* (���MRS485�h���C�o)�g�����X�~�b�^���o�͋֎~ */
	P_SCI_L.SCR.BYTE = 0;			// �S�r�b�g���Z�b�g
	dummy_Read = P_SCI_L.SSR.BYTE;	// Dummy Read
	_ei();
}

