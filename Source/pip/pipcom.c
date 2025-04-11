/*[]----------------------------------------------------------------------[]*/
/*| IFmodule�Ό� ����	(GW828700�ǉ�)                                     |*/
/*|																		   |*/
/*| Host �ʐM����v���O�����iHost.c �̑�s�j				               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : M.Okuda                                                  |*/
/*| Date        : 2003.01.06                                               |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
#define		GLOBAL extern
#include	<string.h>

#include	"pip_def.h"
#include	"system.h"
#include	"ope_def.h"
#include	"mem_def.h"
#include	"ifm_ctrl.h"
#include	"prm_tbl.h"

/*==========================================================================*/

/** timer number **/
#define	TIM1_NUM		(unsigned char)1
#define	TIM2_NUM		(unsigned char)2
#define	TIM3_NUM		(unsigned char)3
#define	TIM4_NUM		(unsigned char)4
#define	TIM_ACC_PAY_NUM	(unsigned char)5	// ���Z�f�[�^�~�Ϗ����p
#define	TIM_ACC_ERR_NUM	(unsigned char)6	// �G���[�f�[�^�~�Ϗ����p

/** timer value **/
#define	TIM1_VAL		(unsigned long)(30+1)	/* 300ms */
#define	TIM2_VAL		(unsigned long)(30+1)	/* 300ms */
#define	TIM3_VAL		(unsigned long)( 1+1)	/*  10ms */

#define	RCT1_COUNT	2						/* NAK retry */
#define	RCT2_COUNT	15						/* Pol retry */
#define	RCT3_COUNT	2						/* Sel retry */

#define	CHAR_ACK0	'0'
#define	CHAR_ACK1	'1'
#define	CHAR_DLE	0x10

#define	SOH	(unsigned char)( 0x01 )	// �w�b�_�[�J�n(Start Of Heading)
#define	STX	(unsigned char)( 0x02 )	// �e�L�X�g�J�n(Start Of Text)
#define	ETX	(unsigned char)( 0x03 )	// �e�L�X�g�I��(End Of Text)
#define	EOT	(unsigned char)( 0x04 )	// �`���I��(End Of Transmission)
#define	ENQ	(unsigned char)( 0x05 )	// �₢���킹(Enquiry)
#define	ACK	(unsigned char)( 0x06 )	// �m�艞��(Acknowledge)
#define	NAK	(unsigned char)( 0x15 )	// �ے艞��(Negative Acknowledge)
#define	SYN	(unsigned char)( 0x16 )	// �e�[�u���f�[�^��؂�R�[�h
#define	ETB	(unsigned char)( 0x17 )	// �`���u���b�N�I��


#define	S0_IDLE			0	// IDEL
#define	S1_Pol_Response	1	// Pol�����҂�
#define	S2_Receiv_TEXT	2	// EXT��M�҂�
#define	S3_Receiv_BCC	3	// BCC��M�҂�
#define	S4_Receiv_EOT	4	// ACK���M��� EOT��M�҂�
#define	S5_Sel_Response	5	// Sel ���M�㉞���҂�
#define	S6_Send_TEXT	6	// Sel �d�����M�� �����҂�

/*==========================================================================*/
/*		�G���A��`															*/
/*==========================================================================*/
// ==> PIPRAM (PIP_Ram.h)

/*==========================================================================*/
/*		�v���O����															*/
/*==========================================================================*/

#pragma 	section

/* external */
extern	unsigned long	term_send_que_push( unsigned long );
extern	void	sci_tx_start_h( void );
extern	void	NH_ScuRestart( void );

/* plottype */
void			NH_Init(void);
void			NH_Main(void);
void			NH_S0(void);
void			NH_S1(void);
void			NH_S2(void);
void			NH_S3(void);
void			NH_S4(void);
void			NH_S5(void);
void			NH_S6(void);
void			NH_TimerSet(unsigned char f_Start);
void			NH_Send(unsigned char *pData, unsigned short Length);
unsigned short	NH_SData_Read(unsigned char *pSData);
unsigned short	NH_Is_SData(void);
void			NH_SData_Del(void);
void			NH_PolSel_Send(unsigned char f_Sel);
void			NH_CtrlCmd_Send(unsigned char CmdKind);
void			NH_RDataSet_toTemp(unsigned char Rchar);
void			NH_TimoutProc(unsigned char Phase);
void			NH_PolErrProc(unsigned char f_Occurred);
void			NH_SelErrProc(unsigned char f_Occurred, unsigned char *pSData);
unsigned char	NH_IsTimeout(unsigned char TimNum);
void			NH_SendReqCancel(void);

unsigned short PIP_SciSendQue_insert(unsigned char *pData, unsigned short length);

unsigned char	NH_RemoteErrProc(void);	// �f�[�^�ʐMNG�`�F�b�N

/*[]----------------------------------------------------------------------[]*/
/*| NH_Init		                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PowerON������Call�����C�j�V��������  		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-06                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_Init( void )
{
	PIPRAM.NH_Rct1 = PIPRAM.NH_Rct2 = PIPRAM.NH_Rct3 = 0;	/* COM error counter */
	PIPRAM.NH_f_ComErr = 0;					/* 1= COM error */
	PIPRAM.NH_PrevRecvChar = 0;				/* prevous received character */

	NH_TimerSet( 1 );						/* timer start */
	PIPRAM.pip_uc_mode_h = S0_IDLE;			/* same as "S_H_IDLE" */
	PIPRAM.i_time_setting = ( unsigned char )prm_get(COM_PRM, S_CEN, 27, 2, 1);	// �|�[�����O�Ԋu�̐ݒ�
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_Main			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| IFmodule�ʐM�������C���i��ԊǗ����j   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-06                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_Main( void )
{
	switch( PIPRAM.pip_uc_mode_h ){
	case	S0_IDLE:
	default:
		NH_S0();		//IDLE
		break;

	case	S1_Pol_Response:
		NH_S1();		//Pol�����҂�
		break;

	case	S2_Receiv_TEXT:	
		NH_S2();		//EXT��M�҂�
		break;

	case	S3_Receiv_BCC:
		NH_S3();		//BCC��M�҂�
		break;

	case	S4_Receiv_EOT:
		NH_S4();		//ACK���M��� EOT��M�҂�
		break;

	case	S5_Sel_Response:
		NH_S5();		//Sel ���M�㉞���҂�
		break;

	case	S6_Send_TEXT:
		NH_S6();		//�d�����M�� �����҂�
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S0			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| ��ԂO�i�A�C�h���j����				   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S0( void )
{
	unsigned	char	f_Sel;						/* 0=Pol send, 1=Sel send */

	if( 0 == NH_IsTimeout( TIM1_NUM ) ){			/* T1 timeout (N) */
		if( 0 != NH_Is_SData() ){					/* send data exist (Y) */
			if( 1 == NH_IsTimeout( TIM4_NUM ) ){	/* T4 timeout (N) */
				goto NH_S0_10;
			}
		}
		return;
	}
NH_S0_10:

	_di();
	PIPRAM.ui_write_h = PIPRAM.ui_count_h = PIPRAM.ui_read_h = 0;	/* receive buffer clear */
	_ei();

	PIPRAM.NH_RbufCt = 0;							/* receive character for temporary buffer clear */
	PIPRAM.NH_PrevRecvChar = 0;						/* previous received character buffer clear */
	PIPRAM.NH_f_NxAck = CHAR_ACK0;					/* next send/receive ACK is ACK0 */

	f_Sel = 0;
	if( 1 != PIPRAM.NH_f_ComErr ){					/* comunication error is happening (N) */
		PIPRAM.NH_SbufLen = NH_SData_Read( &PIPRAM.NH_Sbuf[ 5 ] );	// �V���A�����M�L���[����P�p�P�b�g���o��
		if( 0 != PIPRAM.NH_SbufLen ){				/* send data exist (Y) */
			f_Sel = 1;								/* Sel send req info set */
		}
	}

	NH_PolSel_Send( f_Sel );			/* Pol or Sel send */
	NH_TimerSet( 1 );					/* timer start */

	if( 1 == f_Sel ){					/* Sel sended (Y) */
		PIPRAM.pip_uc_mode_h = S5_Sel_Response;
	}
	else{								/* Pol sended */
		PIPRAM.pip_uc_mode_h = S1_Pol_Response;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S1			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| ��ԂP�iPol���M�� �����҂��j����	   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S1( void )
{
	if( 1 == Mh_get_receive() ) {		/* character received (Y) */
		switch( PIPRAM.uc_rcv_byte_h ){		/* branch by received character */
		case	EOT:					/* frame none */
			PIPRAM.NH_f_ComErr = 0;			/* error repaier */
			PIPRAM.NH_Rct2 = 0;
			NH_PolErrProc( 0 );

			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// �f�[�^�ʐMNG�`�F�b�N
			return;

		case	SOH:					/* frame start */
			PIPRAM.NH_RbufCt = 0;				/* receive character for temporary buffer clear */
			NH_RDataSet_toTemp( PIPRAM.uc_rcv_byte_h );	/* received character save */
			NH_TimerSet( 1 );			/* T3 start */
			PIPRAM.pip_uc_mode_h = S2_Receiv_TEXT;
			return;
		}
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM2_NUM ) ){		/* T2 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S2			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| ��ԂQ�i�d����M�� ETX�҂��j����	   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S2( void )
{
	if( 1 == Mh_get_receive() ) {			/* character received (Y) */
		switch( PIPRAM.uc_rcv_byte_h ){		/* branch by received character */
		case	EOT:						/* frame none */
			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// �f�[�^�ʐMNG�`�F�b�N
			return;

		default:
			NH_RDataSet_toTemp( PIPRAM.uc_rcv_byte_h );	/* received character save */
			NH_TimerSet( 1 );			/* T3 start */

			if( ETX == PIPRAM.uc_rcv_byte_h ){	/* ETX received (Y) */
				PIPRAM.pip_uc_mode_h = S3_Receiv_BCC;
			}
			return;
		}
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM3_NUM ) ){		/* T3 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S3			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| ��ԂR�i�d����M�� BCC�҂��j����	   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S3( void )
{
	unsigned char	Bcc;
	unsigned char	f_Ok;
	short			Length;
	unsigned char			ReqID;		// �v���f�[�^�h�c
	unsigned char			ReqCnt;		// �v������
	#define	PIP_ID51_PKT_LEN	32		// ���v�X�V�v���d�� PIP�t���[����

	if( 1 == Mh_get_receive() ) {		/* character received (Y) */

		NH_RDataSet_toTemp( PIPRAM.uc_rcv_byte_h );	/* received character save */
		f_Ok = 0;
		if( ( 13 < PIPRAM.NH_RbufCt ) &&		/* header & length OK (Y) */
			( 0 == (PIPRAM.NH_RbufCt & 7) )&&	/* multiple of 8 (also SOH+Header+STX+ETX+BCC are 8 bytes) */
			( '0' == PIPRAM.NH_Rbuf[1] ) &&
			( '0' == PIPRAM.NH_Rbuf[2] ) &&
			( '0' == PIPRAM.NH_Rbuf[3] ) &&
			( '1' == PIPRAM.NH_Rbuf[4] ) &&
			( STX == PIPRAM.NH_Rbuf[5] ) ){

			Bcc = Mt_BccCal( &PIPRAM.NH_Rbuf[6], (PIPRAM.NH_RbufCt - 6 - 1) );
			if( Bcc == PIPRAM.uc_rcv_byte_h ){		/* BCC OK (Y) */
				f_Ok = 1;
			}
		}
		if( 1 == NH_RemoteErrProc() ){
			f_Ok = 0;				// �f�[�^�ʐMNG���������Ă���ꍇ��NAK��Ԃ�
		}

		/** NG frame received process **/
		if( 0 == f_Ok ){							/* received data NG (Y) */
			NH_CtrlCmd_Send( NAK );					/* NAK send */
			NH_TimerSet( 1 );						/* T2 start */
			PIPRAM.pip_uc_mode_h = S1_Pol_Response;	/* frame receive wait again */
			return;
		}

		NH_CtrlCmd_Send( ACK );		/* ACK send (0/1) */
		PIPRAM.NH_f_ComErr = 0;		/* COM error repaier */
		PIPRAM.NH_Rct2 = 0;
		NH_PolErrProc( 0 );

		Length = PIPRAM.NH_RbufCt;	//Length = SOH�`BCC�̃T�C�Y
		Length -= 8;				//SOH,HD1,HD2(1.2.3),STX, , ETX,BCC

		if( Length >= 6 ){
			switch( PIP_BcdAscToBin_2(&PIPRAM.NH_Rbuf[10]) ){	// �f�[�^���
			case 31:	// �f�[�^�v���Q(�t���b�v�E���b�N��)
				ReqID = PIP_BcdAscToBin_2( &PIPRAM.NH_Rbuf[24] );		// �v���f�[�^�h�c
				ReqCnt = PIP_BcdAscToBin_2( &PIPRAM.NH_Rbuf[26] );		// �v������
				if( (ReqCnt == 0)||(ReqCnt > 20) ){						// �P�`�Q�O�͈͓̔��łȂ���΂m�f
					ReqID = 0;
				}
				switch( ReqID ){
				case 67:	// ���Z�f�[�^(�t���b�v�E���b�N��)
					IFM_Snd_AccumulatePayment( ReqCnt );
					break;
				case 68:	// �G���[�f�[�^(�t���b�v�E���b�N��)
					IFM_Snd_AccumulateError( ReqCnt );
					break;
				default:	// ����:�p�����[�^�s��("�O�Q")
					memcpy(&PIPRAM.Err_sendbuf[0], &PIPRAM.NH_Rbuf[6], 40);
					PIPRAM.Err_sendbuf[4] = '6';
					PIPRAM.Err_sendbuf[5] = '1';		// �U�P:�f�[�^�v���Q�m�f����
					PIPRAM.Err_sendbuf[16] = '0';
					PIPRAM.Err_sendbuf[17] = '2';		// �O�Q:�p�����[�^�s��
					PIP_SciSendQue_insert(&PIPRAM.Err_sendbuf[0], 40);	// �V���A�����M�L���[�ւ̃f�[�^�ǉ�
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				}
				break;
			default:	// �X���[�f�[�^
				PIP_ChangeData(Length, &PIPRAM.NH_Rbuf[6]);	// �V���A����M�f�[�^��IFM�̃o�b�t�@�֕ϊ�
				break;
			}
		}

		NH_TimerSet( 1 );			/* T2 start */
		PIPRAM.pip_uc_mode_h = S4_Receiv_EOT;
		return;
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM3_NUM ) ){		/* T3 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S4			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| ��ԂS�i�d����M ACK���M�� EOT�҂��j����	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S4( void )
{
	if( 1 == Mh_get_receive() ) {			/* character received (Y) */
		switch( PIPRAM.uc_rcv_byte_h ){		/* branch by received character */
		case	EOT:						/* frame none */
			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// �f�[�^�ʐMNG�`�F�b�N
			return;

		case	STX:
			PIPRAM.NH_RbufCt = 5;				/* data set point set */
			NH_RDataSet_toTemp( PIPRAM.uc_rcv_byte_h );	/* received character save */
			NH_TimerSet( 1 );			/* T3 start */
			PIPRAM.pip_uc_mode_h = S2_Receiv_TEXT;
			NH_RemoteErrProc();			// �f�[�^�ʐMNG�`�F�b�N
			return;
		}
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM2_NUM ) ){		/* T2 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S5			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| ��ԂT�iSel���M�� �����҂��j����			                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S5( void )
{
	if( 1 == Mh_get_receive() ) {		/* character received (Y) */
		if( EOT == PIPRAM.uc_rcv_byte_h ){	/* EOT receive (Y) */
			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// �f�[�^�ʐMNG�`�F�b�N
			return;
		}

		if( NAK == PIPRAM.uc_rcv_byte_h ){		/* NAK receive (Y) */
			NH_TimoutProc( PIPRAM.pip_uc_mode_h );
			return;
		}

		if( (CHAR_DLE == PIPRAM.NH_PrevRecvChar) && (PIPRAM.NH_f_NxAck == PIPRAM.uc_rcv_byte_h) ){	/* OK ACK receive (Y) */
			PIPRAM.NH_Rct1 = 0;

			/* ���M����d���� "PIPRAM.NH_Sbuf[]" �ɓ����Ă���B�A���w�b�_�[���� */
			PIPRAM.NH_Sbuf[ 0 ] = SOH;
			PIPRAM.NH_Sbuf[ 1 ] = '0';
			PIPRAM.NH_Sbuf[ 2 ] = '0';
			PIPRAM.NH_Sbuf[ 3 ] = '0';
			PIPRAM.NH_Sbuf[ 4 ] = '1';
			NH_Send( &PIPRAM.NH_Sbuf[ 0 ], (PIPRAM.NH_SbufLen + 5) );	/* send */

			PIPRAM.NH_f_NxAck ^= 1;
			PIPRAM.NH_PrevRecvChar = 0;

			NH_TimerSet( 1 );				/* T2 start */
			PIPRAM.pip_uc_mode_h = S6_Send_TEXT;
			NH_RemoteErrProc();			// �f�[�^�ʐMNG�`�F�b�N
			return;
		}

		PIPRAM.NH_PrevRecvChar = PIPRAM.uc_rcv_byte_h;	/* previous character update */
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM2_NUM ) ){		/* T2 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_S6			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| ��ԂU�i�d�����M�� ACK�҂��j����			                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_S6( void )
{
	if( 1 == Mh_get_receive() ) {		/* character received (Y) */
		if( EOT == PIPRAM.uc_rcv_byte_h ){		/* EOT receive (Y) */
			NH_TimerSet( 1 );				/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;	/* phase to IDLE */
			NH_RemoteErrProc();				// �f�[�^�ʐMNG�`�F�b�N
			return;
		}

		if( NAK == PIPRAM.uc_rcv_byte_h ){		/* NAK receive (Y) */
			if( RCT1_COUNT <= PIPRAM.NH_Rct1 ){			/* retry over (Y) */
				PIPRAM.NH_f_ComErr = 1;

				/* ���M���悤�Ƃ����f�[�^�� "PIPRAM.NH_Sbuf[]" �ɓ����Ă��� */
				NH_SelErrProc( 1, &PIPRAM.NH_Sbuf[0] );	/* error inform */
				NH_SendReqCancel();						/* send request cancel (for error) */

				PIPRAM.NH_Rct1 = 0;

				NH_CtrlCmd_Send( EOT );
				NH_TimerSet( 1 );					/* T1 start */
				PIPRAM.pip_uc_mode_h = S0_IDLE;		/* phase to IDLE */
			}
			else{
				++PIPRAM.NH_Rct1;
				NH_Send( &PIPRAM.NH_Sbuf[ 0 ], (PIPRAM.NH_SbufLen + 5) );	/* frame re-send */
				NH_TimerSet( 1 );					/* T2 start */
			}
			return;
		}

		if( (CHAR_DLE == PIPRAM.NH_PrevRecvChar) && (PIPRAM.NH_f_NxAck == PIPRAM.uc_rcv_byte_h) ){	/* OK ACK receive (Y) */

			/* ���M�����f�[�^�� "PIPRAM.NH_Sbuf[]" �ɓ����Ă��� */
			NH_SData_Del();								// �V���A�����M�L���[���̑��M�f�[�^�|�C���g�ړ�
			PIPRAM.NH_f_ComErr = 0;
			PIPRAM.NH_Rct1 = 0;							/* NAK received retry counter clear */
			PIPRAM.NH_Rct3 = 0;							/* Sel retry counter clear */
			NH_CtrlCmd_Send( EOT );
			NH_TimerSet( 1 );						/* T1 start */
			PIPRAM.pip_uc_mode_h = S0_IDLE;			/* phase to IDLE */
			NH_RemoteErrProc();			// �f�[�^�ʐMNG�`�F�b�N
			return;
		}

		PIPRAM.NH_PrevRecvChar = PIPRAM.uc_rcv_byte_h;		/* previous character update */
	}

	/* timeout check */
	if( 1 == NH_IsTimeout( TIM2_NUM ) ){		/* T2 timeout (Y) */
		NH_TimoutProc( PIPRAM.pip_uc_mode_h );
	}
}



/*==========================================================================*/
/*		UTYLITY subroutines													*/
/*==========================================================================*/

/*[]----------------------------------------------------------------------[]*/
/*| NH_TimerSet		                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| timer start/stop control			  		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : f_Start : 1=Start, 0=Stop							   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-06                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_TimerSet( unsigned char f_Start )
{
	if( 1 == f_Start ){
		PIPRAM.NH_StartTime = LifeTimGet();
	}
	else{
		PIPRAM.NH_StartTime = 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_Send			                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| data send to serial port.			  		                           |*/
/*| This return after send complete 									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : pData = pointer for send string						   |*/
/*|				   Length = send data length							   |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-06                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_Send( unsigned char *pData, unsigned short Length )
{
	memcpy( &PIPRAM.huc_txdata_i_h[0], pData, Length );
	PIPRAM.ui_txdlength_i_h = Length;
	PIPRAM.ui_txpointer_h = 0;					/* send pointer clear */

	NH_TimerSet( 1 );						/* T1 start */
	do{
		taskchg( IDLETSKNO );
	}while( 0 == NH_IsTimeout( TIM3_NUM ) );

	sci_tx_start_h();					/* TxI enable (send start) */

	/* wait until send end */
	do{
		taskchg( IDLETSKNO );
	}while( 0 == PIPRAM.uc_send_end_h );
}

//==============================================================================
//	�V���A�����M�L���[����̃p�P�b�g���o��
//
//	@argument	*pSData		�o���������o�q�n�ɑ��M����p�P�b�g
//
//	@return		�o���������o�q�n�ɑ��M����p�P�b�g�̒���(�r�s�w�`�a�b�b)
//
//	@image	���M��
//			  �����M�ʒu                  [�V���A�����M�L���[]
//			  ��   ��  ����������������������������������������������������������
//			�p��   ������������STX ��ID1 ��ID2 ��ID3 ���c�f�[�^�c��ETX ��BCC ����
//			�P��   ��  �����������������������������������������������������������l
//			�b��   ������������STX ��ID1 ��ID2 ��ID3 ���c�f�[�^�c��ETX ��BCC �����`
//			�g��   ��  �����������������������������������������������������������w
//			����   ������ �F �� �F �� �F �� �F �� �F ��    �F    �� �F �� �F �����W
//			  ��   ��  �����������������������������������������������������������O
//			  ��   ������������STX ��ID1 ��ID2 ��ID3 ���c�f�[�^�c��ETX ��BCC ������
//		STOP ��������������������������������������������������������������������
//			           ��    ��    ��    ��    ��    ��          ��    ��    ����
//			           ����������������������������������������������������������
//			           ������������������  �Q�T�X��������  ��������������������
//	@end
//
//	@attention	COUNT 0, CIND 0 �͑��M�L���[����̏�Ԃ��Ӗ�����B
//
//	@note		�V���A�����M�L���[�Ɋi�[����Ă���(�r�s�w����a�b�b�܂ł�)
//				�����M�p�P�b�g���L���[�̏�����T���o���p�P�b�g�ƃp�P�b�g����Ԃ��B
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
unsigned short NH_SData_Read(unsigned char *pSData)
{
	if( PIPRAM.SCI_S_QUE.COUNT == 0 )	return( 0 );			// �V���A�����M�L���[�Ƀf�[�^�Ȃ�
	if( PIPRAM.SCI_S_QUE.CIND >= PIPRAM.SCI_S_QUE.COUNT ){		// ���M�p�P�b�g�ʒu���p�P�b�g���Ɠ���ɂȂ�ƑS���M�I��
		PIPRAM.SCI_S_QUE.COUNT = 0;								// �p�P�b�g���O
		PIPRAM.SCI_S_QUE.CIND = 0;								// [����]���M�p�P�b�g�ʒu���C���N�������g�����̂́u���푗�M���v
		return( 0 );											// �V���A�����M�L���[�Ƀf�[�^�Ȃ�
	}

	memcpy( pSData, &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.CIND].BUFF[0], PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.CIND].LEN );
	return( PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.CIND].LEN );
}

//==============================================================================
//	�V���A�����M�L���[���̖����M�p�P�b�g���擾
//
//	@argument	�Ȃ�
//
//	@return		�����M�p�P�b�g��
//
//	@attention	COUNT 0, CIND 0 �͑��M�L���[����̏�Ԃ��Ӗ�����B
//
//	@note		�V���A�����M�L���[�Ɋi�[����Ă���(�r�s�w����a�b�b�܂ł�)
//				�����M�p�P�b�g����Ԃ��B
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
unsigned short NH_Is_SData(void)
{
	if( PIPRAM.SCI_S_QUE.CIND > PIPRAM.SCI_S_QUE.COUNT ){	// �{�����蓾�Ȃ�
		PIPRAM.SCI_S_QUE.COUNT = 0;							// �p�P�b�g���O
		PIPRAM.SCI_S_QUE.CIND = 0;							// [����]���M�p�P�b�g�ʒu���C���N�������g�����̂́u���푗�M���v
	}
	return(unsigned short)( PIPRAM.SCI_S_QUE.COUNT - PIPRAM.SCI_S_QUE.CIND );
}

//==============================================================================
//	�V���A�����M�f�[�^�|�C���g�ړ�
//
//	@argument	�Ȃ�
//
//	@return		�Ȃ�
//
//	@attention	COUNT 0, CIND 0 �͑��M�L���[����̏�Ԃ��Ӗ�����B
//
//	@note		�V���A�����M�L���[�Ɋi�[����Ă���p�P�b�g�P���̑��M�������
//				�I�������Ƃ��ɃR�[������A�L���[���̎��̃f�[�^���|�C���g����B
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void NH_SData_Del(void)
{
	if( PIPRAM.SCI_S_QUE.COUNT == 0 ){	// �V���A�����M�L���[�ɂ͂��Ƃ��ƃf�[�^�������ĂȂ�
		PIPRAM.SCI_S_QUE.CIND = 0;
		return;
	}

	if( PIPRAM.SCI_S_QUE.CIND > PIPRAM.SCI_S_QUE.COUNT ){		// �{�����蓾�Ȃ�
		PIPRAM.SCI_S_QUE.COUNT = 0;
		PIPRAM.SCI_S_QUE.CIND = 0;
		return;
	}

	if( PIPRAM.SCI_S_QUE.CIND == PIPRAM.SCI_S_QUE.COUNT ){				// ���M�p�P�b�g�ʒu���p�P�b�g���Ɠ���ɂȂ�ƑS���M�I��
	} else {
		PIPRAM.SCI_S_QUE.CIND++;				// ���̃p�P�b�g���|�C���g
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_PolSel_Send		                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Pol or Sel send						  		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : f_Sel  0=Pol, 1=Sel									   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_PolSel_Send( unsigned char f_Sel )
{
	PIPRAM.wkuc[ 0 ] = EOT;
	PIPRAM.wkuc[ 1 ] = '0';
	PIPRAM.wkuc[ 2 ] = '0';
	PIPRAM.wkuc[ 3 ] = '0';
	PIPRAM.wkuc[ 4 ] = ( 1 == f_Sel ) ? '1' : '0';
	PIPRAM.wkuc[ 5 ] = ENQ;

	NH_Send( &PIPRAM.wkuc[ 0 ], (unsigned short)6 );
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_CtrlCmd_Send			                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| control character send 				  		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : CmdKind  send control character (see source)			   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_CtrlCmd_Send( unsigned char CmdKind )
{
	switch( CmdKind ){
	case	ACK:
		PIPRAM.wkuc[ 0 ] = CHAR_DLE;
		PIPRAM.wkuc[ 1 ] = PIPRAM.NH_f_NxAck;
		NH_Send( &PIPRAM.wkuc[ 0 ], (unsigned short)2 );
		PIPRAM.NH_f_NxAck ^= 1;
		break;

	case	NAK:
		PIPRAM.wkuc[ 0 ] = NAK;
		NH_Send( &PIPRAM.wkuc[ 0 ], (unsigned short)1 );
		break;

	case	EOT:
		PIPRAM.wkuc[ 0 ] = EOT;
		NH_Send( &PIPRAM.wkuc[ 0 ], (unsigned short)1 );
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_RDataSet_toTemp	                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| received character set to temporary buffer	                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : Rchar  received character							   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_RDataSet_toTemp( unsigned char Rchar )
{
	if( NH_RBUF_MAX > PIPRAM.NH_RbufCt ){
		PIPRAM.NH_Rbuf[ PIPRAM.NH_RbufCt ] = Rchar;
		++PIPRAM.NH_RbufCt;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_TimoutProc		                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| timeout happened process					                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : Phase  Process phase when timeout occuerd 			   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_TimoutProc( unsigned char Phase )
{
	/** Pol phase process **/
	if( ( 1 <= Phase) && (Phase <= 3) ){			/* Pol phase (Y) */
		if( RCT2_COUNT <= PIPRAM.NH_Rct2 ){				/* retry over (Y) */
			NH_ScuRestart();						/* SCU restart */

			PIPRAM.NH_f_ComErr = 1;
			NH_PolErrProc( 1 );
			PIPRAM.NH_Rct2 = 0;
		}
		else{
			++PIPRAM.NH_Rct2;
		}
	}

	/** Sel phase process **/
	else if( ( 5 <= Phase) && (Phase <= 6) ){		/* Sel phase (Y) */
		if( RCT3_COUNT <= PIPRAM.NH_Rct3 ){				/* retry over (Y) */
			PIPRAM.NH_f_ComErr = 1;

			/* ���M���悤�Ƃ����f�[�^�� "PIPRAM.NH_Sbuf[]" �ɓ����Ă��� */
			NH_SelErrProc( 1, &PIPRAM.NH_Sbuf[0] );	/* error inform */
			NH_SendReqCancel();						/* send request cancel (for error) */

			PIPRAM.NH_Rct3 = 0;
		}
		else{
			++PIPRAM.NH_Rct3;
		}
	}

	NH_CtrlCmd_Send( EOT );
	NH_TimerSet( 1 );								/* T1 start */
	PIPRAM.pip_uc_mode_h = S0_IDLE;					/* phase to IDLE */
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_PolErrProc		                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| When Pol send retry over occurred or released process                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : f_Occerd  1=occurred, 0=released						   |*/
/*| RETURN VALUE : none					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NH_PolErrProc( unsigned char f_Occurred )
{
	if( 1 == f_Occurred ){
		PIP_ARCsB0Regist(1, 1);	// �G���[�ʒm:�O���ڑ��@�햢����(����)
	}
	else{
		PIP_ARCsB0Regist(1, 0);	// �G���[�ʒm:�O���ڑ��@�햢����(����)
	}
}

//==============================================================================
//	�f�[�^���M�m�f�n�̃G���[�ʒm�d������
//
//	@argument	f_Occurred	�G���[��(0:����, 1:����)
//	@argument	*pSData		�V���A�����M����/���悤�Ƃ����p�P�b�g�f�[�^
//	@return		�Ȃ�
//
//	@note		�r�d�k�p�P�b�g�̑��M�ɑ΂��ĉ���(�`�b�j/�m�`�j)���R�O�O�����ȓ���
//				��M�ł��Ȃ��A�܂��͂m�`�j�������A�����ĂR�񑱂����Ƃ��A�����
//				�s�d�w�s�̑��M�ɑ΂��ĉ���(�`�b�j)���R�O�O�����ȓ��Ɏ�M�ł��Ȃ��Ƃ���
//				�R�[������A�h�a�j���ێ����Ă���G���[�̔����󋵂ɕω��������
//				�G���[�̔����܂��͉������a�n�Œʒm����B
//				����A�G���[�������������Ȃ̂ŁA�����͕s�v�����A�d�g�݂͎c��
//
//	@author		2012/10/19 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void NH_SelErrProc( unsigned char f_Occurred, unsigned char *pSData )
{
	unsigned char	ErrCode;

	switch( PIP_BcdAscToBin_2(&pSData[10]) ){		// �h�c�R(�f�[�^���)
	case 67:	// ���M�p�P�b�g�͐��Z�f�[�^(�t���b�v�E���b�N��)
		ErrCode = 40;
		break;
	case 68:	// ���M�p�P�b�g�̓G���[�f�[�^(�t���b�v�E���b�N��)
		ErrCode = 41;
		break;
	case 60:
	case 61:
	case 62:
	case 63:
	case 64:
	case 65:
	case 66:
	case 70:
	case 71:
	case 72:
	case 73:
	case 74:
	case 75:
	case 76:
		ErrCode = 42;
		break;
	default:
		return;
	}

	if( 1 == f_Occurred ){
		PIP_ARCsB0Regist(ErrCode, 1);	// �G���[�ʒm:�S�O�`�S�Q(����/������������)
	} else {
		PIP_ARCsB0Regist(ErrCode, 0);	// �G���[�ʒm:���񖢎g�p(����)
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NH_IsTimeout	                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| timeout check						   		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : TimNum (1-3)											   |*/
/*| RETURN VALUE : 1=timeout, 0=doesn't timeout                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2003-01-07                                              |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
unsigned char	NH_IsTimeout( unsigned char TimNum )
{
	unsigned char	TOsts;
	unsigned long	TimVal = 0;

	TOsts = 0;

	switch( TimNum ){
	case	TIM1_NUM:
		TimVal = TIM1_VAL + (unsigned long)(PIPRAM.i_time_setting * 10);
		break;
	case	TIM2_NUM:
		TimVal = TIM2_VAL;
		break;
	case	TIM3_NUM:
		TimVal = TIM3_VAL;
		break;
	case	TIM4_NUM:
		TimVal = TIM1_VAL;
		break;
	}
	if( LifePastTimGet( PIPRAM.NH_StartTime ) >= TimVal ){
		TOsts = 1;
	}
	return	TOsts;
}
//==============================================================================
//	�V���A�����M�f�[�^�|�C���g�ړ�
//
//	@argument	�Ȃ�
//
//	@return		�Ȃ�
//
//	@attention	COUNT 0, CIND 0 �͑��M�L���[����̏�Ԃ��Ӗ�����B
//
//	@note		�r�d�k�ɑ΂��Ė������A�܂��͂m�`�j�������A�����ĂR�񑱂����Ƃ���
//				�ɃR�[������A�V���A�����M�L���[�ƃX���[�f�[�^�o�b�t�@�̃f�[�^��
//				�폜����B
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void NH_SendReqCancel(void)
{
	PIPRAM.SCI_S_QUE.COUNT = 0;		// ���M�L���[�N���A
	PIPRAM.SCI_S_QUE.CIND = 0;
}

/****************************************************************/
/*	�@�\	=	BCC���Z����										*/
/*	return	=	BCC�l											*/
/****************************************************************/
unsigned char	Mt_BccCal( unsigned char *pSet, unsigned short Length)
{
	unsigned char	bcc;
	unsigned short	i;

	bcc = 0;
	for ( i=0; i<Length; i++ ) {
		bcc = bcc ^ pSet[i];
	}
	return	bcc;
}
/****************************************************************/
/*	�֐���	=	Mh_get_receive									*/
/*	�@�\	=	HOST����M�f�[�^�擾							*/
/*	���l	=	Host����̃L�����N�^��M�L�����`�F�b�N���A		*/
/*				��M���Ă���� uc_rcv_byte_h �ɃZ�b�g����B		*/
/*	return	=	1=��M�L�����N�^����A0=�Ȃ�					*/
/****************************************************************/
unsigned short	Mh_get_receive(void)
{
	/* �V���Ȏ�M�f�[�^�����邩�`�F�b�N */
	if ( PIPRAM.ui_count_h == 0 )					/* �f�[�^�Ȃ� */
	{
		return((unsigned short)0);
	}
	else									/* �f�[�^���� */
	{
		PIPRAM.uc_rcv_byte_h = PIPRAM.uc_r_buf_h[PIPRAM.ui_read_h];	/* �����ݏ����̎�M�o�b�t�@����P�L�����N�^get */
		if ( PIPRAM.ui_read_h == R_BUF_MAX_H-1 )		/* �ǂݏo���|�C���^�X�V */
		{
			PIPRAM.ui_read_h = 0;
		}
		else
		{
			PIPRAM.ui_read_h++;
		}
		_di();
		PIPRAM.ui_count_h--;
		_ei();
		return((unsigned short)1);
	}
}
//==============================================================================
//	�f�[�^�ʐMNG�`�F�b�N
//
//	@argument	�Ȃ�
//	@argument	�Ȃ�
//	@return		�G���[������ �F 0 = �G���[�Ȃ��A1 = �G���[����
//	@note		�G���[���荞�ݔ����t���O�Ŕ��f���A���ꂼ��̃G���[�𐶐����A
//				�G���[������A�t���O����������
//
//	@author		2012/10/11 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
unsigned char NH_RemoteErrProc()
{
	unsigned char error_result;

	error_result = 0;
	if( 1 == pip_f_txerr_h_ovrun ){	// �I�[�o�[�����G���[����
		PIP_ARCsB0Regist(ERR_REMOTE_SCI_OVERRUN, 1);	// �G���[�ʒm : 62 �I�[�o�[�����G���[
		pip_f_txerr_h_ovrun = 0;						// �G���[��ԉ���
		error_result = 1;
	}
	if( 1 == pip_f_txerr_h_fr ){	// �t���[�~���O�G���[����
		PIP_ARCsB0Regist(ERR_REMOTE_SCI_FRAME, 1);		// �G���[�ʒm : 61 �t���[�~���O�G���[
		pip_f_txerr_h_fr = 0;							// �G���[��ԉ���
		error_result = 1;
	}
	if( 1 == pip_f_txerr_h_p ){		// �p���e�B�G���[����
		PIP_ARCsB0Regist(ERR_REMOTE_SCI_PARITY, 1);		// �G���[�ʒm : 60 �p���e�B�G���[
		pip_f_txerr_h_p = 0;							// �G���[��ԉ���
		error_result = 1;
	}
	return error_result;
}
/****************************************************************/
/*	�֐���		=	MH_ScuRestart								*/
/*	�@�\		=	HOST�ʐM���@�V���A���R���g���[�����X�^�[�g	*/
/*	���l		=												*/
/****************************************************************/
void	NH_ScuRestart( void )
{
	sci_stop_h();

	PIPRAM.ui_txdlength_i_h = 0;		/* ���M�����ݏ����p �c���M�����O�X �N���A 	*/
	PIPRAM.ui_txpointer_h = 0;			/* ���M�����ݏ����p �����M�f�[�^Index�N���A */

	PIPRAM.pip_uc_mode_h = S0_IDLE;		/*	��ԃ��[�h���j���[�g�����ɂ���	*/

	sci_init_h();				/* �g�n�r�s����M�N�� */
}

/****************************************************************************************************/
/*	�֐���		:  PIP_BcdAscToBin_2 ()																	*/
/*	�@�\		:  2byte BCD�A�X�L�[���o�C�i���[�ɕϊ�����											*/
/****************************************************************************************************/
unsigned char	PIP_BcdAscToBin_2 ( unsigned char *asc_dat)
{
	unsigned char	wuc_bcd;
	unsigned char	i;

	wuc_bcd = 0;
	for( i=0; i < 2; i++ ){
		wuc_bcd *= 10;
		if ( asc_dat[i] >= '0' && asc_dat[i] <= '9' ){
			wuc_bcd += ( asc_dat[i] - '0' );
		}else{
			wuc_bcd = 0;
		}
	}
	return( wuc_bcd );
}
//--------------------------------------------------------------------------------
// �o�C�i���[���Q�o�C�gBCD�����ɕϊ�����
//
// BinDat(in) : Binary Data
// TextDat(out):BCD ASCII
//--------------------------------------------------------------------------------
void	PIP_BinToBcdAsc_2 ( unsigned char BinDat, unsigned char * TextDat )
{
	unsigned char	uwi;
	uwi = BinDat / 10;
	TextDat[0] = uwi + 0x30;
	uwi = BinDat % 10;
	TextDat[1] = uwi + 0x30;
}
//==============================================================================
//	�G���[�ʒm�d���̐���
//
//	@argument	err_code	�G���[�R�[�h
//	@argument	on_off		����/����(1:����, 0:����)
//
//	@return		�Ȃ�
//
//	@attention	�t���b�v�E���b�N���̏ꍇ�G���[�R�[�h�O�P�ȊO�A
//				����(�O)�ł̓G���[�d���𐶐����܂���B
//
//	@note		�G���[�ʒm�d���𐶐����`�q�b�m�d�s���M�L���[�Ɋi�[����B
//
//	@author		2007.03.01:m-onouchi
//======================================== Copyright(C) 2006 AMANO Corp. =======
void PIP_ARCsB0Regist(unsigned char err_code, unsigned char on_off)
{
	unsigned char	module_code;
	unsigned char	c_work[2];

	switch( err_code ){
	case 1:
		module_code = 55;
		if( PIPRAM.Ctrl_B0_54.bits.B0_01 == on_off ) return;		// �������̔����A�������̉����͎󂯕t���Ȃ�
		PIPRAM.Ctrl_B0_54.bits.B0_01 = on_off;
		on_off = on_off?1:2;										// 1:�����C2:����
		break;
	case 40:
	case 41:
	case 42:
		if( on_off == 0 )	return;			// �������������Ȃ̂ŉ����̂Ƃ��ɂ͐������Ȃ��B
		module_code = 55;
		on_off = 3;							// �t���b�v�E���b�N���̏ꍇ(3:������������)
		break;
	case ERR_REMOTE_SCI_PARITY:		// �V���A���p���e�B�G���[
	case ERR_REMOTE_SCI_FRAME:		// �V���A���t���[�~���O�G���[
	case ERR_REMOTE_SCI_OVERRUN:	// �V���A���I�[�o�[�����G���[
		module_code = 55;
		break;
	default:
		return;
	}
	memset(&PIPRAM.ARCs_B0_54_SendEdit[0], 0x30, PIP_ERR_DATLEN_IDB0);
	memcpy(&PIPRAM.ARCs_B0_54_SendEdit[0],"0101B00100", 10);	// �h�c�P�`�@��R�[�h

	PIP_BinToBcdAsc_2(module_code, c_work);						// ���W���[���R�[�h
	PIPRAM.ARCs_B0_54_SendEdit[10] = c_work[0];
	PIPRAM.ARCs_B0_54_SendEdit[11] = c_work[1];

	PIP_BinToBcdAsc_2(err_code, c_work);						// �G���[�R�[�h
	PIPRAM.ARCs_B0_54_SendEdit[12] = c_work[0];
	PIPRAM.ARCs_B0_54_SendEdit[13] = c_work[1];

	PIP_BinToBcdAsc_2(on_off, c_work);							// �����E����
	PIPRAM.ARCs_B0_54_SendEdit[14] = c_work[0];
	PIPRAM.ARCs_B0_54_SendEdit[15] = c_work[1];

	PIP_ChangeData(16, &PIPRAM.ARCs_B0_54_SendEdit[0]);		// �V���A����M�f�[�^��IFM�̃o�b�t�@�֕ϊ�
}

//==============================================================================
//	�V���A�����M�L���[�ւ̃f�[�^�ǉ�
//
//	@argument	*pData		IFM�Ő������������f�[�^�p�P�b�g(�h�c�P�`)
//	@argument	length		�����f�[�^�p�P�b�g�f�[�^��
//
//	@return		�V���A�����M�L���[�֒ǉ������r�s�w����a�b�b�܂ł̃f�[�^��
//
//	@attention	���� length �̒l�� PIP_HOSTSEND_LEN_MAX �o�C�g�ȓ��ł��邱�ƁB
//
//	@note		IFM�Ő������������f�[�^�p�P�b�g���V���A�����M�L���[�ɓ����B
//
//	@author		2012/10/1 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
unsigned short PIP_SciSendQue_insert(unsigned char *pData, unsigned short length)
{
	typedef struct {
		unsigned char	_1;	// �h�c�P(�f�[�^�u���b�N��)
		unsigned char	_2;	// �h�c�Q(�u���b�N�X�e�[�^�X)
		unsigned char	_3;	// �h�c�R(��ʃR�[�h)
	} t_ID;
	t_ID ID_now;

	ID_now._1 = PIP_BcdAscToBin_2( &pData[0] );		// �h�c�P
	ID_now._2 = PIP_BcdAscToBin_2( &pData[2] );		// �h�c�Q
	ID_now._3 = PIP_BcdAscToBin_2( &pData[4] );		// �h�c�R

	if( 1 < ID_now._1 ) {								// �擪�u���b�N�łȂ��ꍇ�A���܂łƓ���̎�ʂł���K�v������B
		t_ID ID_before;
		ID_before._1 = PIP_BcdAscToBin_2( &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT - 1 ].BUFF[1] );	// �O��̂h�c�P
		ID_before._3 = PIP_BcdAscToBin_2( &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT - 1 ].BUFF[5] );	// �O��̂h�c�R
		if( ID_now._3 != ID_before._3){					// �ʎ�ʂ̓r���u���b�N
			return( 0 );								// ��M�p�P�b�g�p��
		}
		if( ID_now._1 != ID_before._1 + 1 ){			// �f�[�^�u���b�N�����A�Ԃł���K�v������B
			return( 0 );								// ��M�p�P�b�g�p��
		}
	}
	if( (ID_now._2 == 0)&&(length != PIP_HOSTSEND_LEN_MAX) ){	// �r���u���b�N�̏ꍇ�f�[�^���͕K���ő�łȂ��Ƃ����Ȃ��B
		return( 0 );									// ��M�p�P�b�g�p��
	}
	if( PIPRAM.SCI_S_QUE.COUNT >= 79 ){
		if( PIPRAM.SCI_S_QUE.COUNT == 79 ){				// �ۑ��ʒu���W�O�p�P�b�g��
			if( ID_now._2 != 1 ){						// �K���I�[�p�P�b�g�łȂ��Ƃ����Ȃ��B
				return( 0 );							// ��M�p�P�b�g�p��
			}
		} else {
			return( 0 );								// ��M�p�P�b�g�p��
		}
	}

	memset( &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT], 0, sizeof( PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT] ) );					// �V�f�[�^�`����0�N���A
	PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[0] = STX;								// �e�L�X�g�u���b�N�̐擪 �ǉ�
	length = (length < PIP_HOSTSEND_LEN_MAX ? length : PIP_HOSTSEND_LEN_MAX);
	memcpy( &PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[1], pData, (size_t)length );	// �h�c�P���� �e�L�X�g�u���b�N���R�s�[
	length++;																					// �r�s�w���v���X

	PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[length] = ETX;							// �e�L�X�g�u���b�N�̖��� �ǉ�
	PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[length+1] = Mt_BccCal( (unsigned char*)&PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].BUFF[1], length );	// BCC�v�Z���ABBC��ǉ�
	length += 2;																				// �d�s�w�Ƃa�b�b���v���X
	PIPRAM.SCI_S_QUE.DATA[PIPRAM.SCI_S_QUE.COUNT].LEN += length;								// �r�s�w�`�a�b�b�܂ł̒���
	PIPRAM.SCI_S_QUE.COUNT++;																	// �p�P�b�g�}���ʒu�ύX

	return( length );
}

