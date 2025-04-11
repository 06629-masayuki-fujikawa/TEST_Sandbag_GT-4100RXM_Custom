/*[]----------------------------------------------------------------------[]*/
/*| Suica control                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Namioka                                                |*/
/*| Date        : 06-07-12                                                 |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"irq1.h"
#include	"suica_def.h"
#include	"tbl_rkn.h"
#include	"lcd_def.h"
#include	"mdl_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

static short	suica_01( void );
static short	suica_02( void );
static short	suica_03( void );
static short	suica_04( void );
static uchar 	Suica_que_check( void );

static void 	Suica_recv_data_regist( uchar* , ushort );

// �����������@�ySX-10���䕔�z(start)�@���������� 
/*[]----------------------------------------------------------------------[]*/
/*| SX-10���䕔		                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|�E��ʌnIC�J�[�h���[�_�[�iSX-10�j�̐��䕔�ŕ��ł����B  	           |*/
/*|�@"�ySX-10���䕔�z" (start)�`(end) �܂ł̊Ԃ�SX-10���䕔�ł���B �@�@�@ |*/
/*|�E���̊Ԃ̃v���O������ύX�����ꍇ�́AJREM�l�֐\�����s�����ƂƂ���B    |*/
/*|�ESX-10���䕔�Ƃ��ĕ��ł������A�ύX�̗L���𖾊m�ɂ���B                 |*/
/*|�E���ٖ��m���̂��߁A���̊Ԃ̃R�����g���͐�������B					   |*/
/*[]----------------------------------------------------------------------[]*/

/*[]----------------------------------------------------------------------[]*/
/*| SX-10���䕔 ����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
const	ver_rec		VERSNO_SX10CTRL = {
	'0',' ','M','H','3','2','8','8','0','0'		// �d�q���ϑΉ�����(Suica�P�v�΍���)FT4000�p
};

/*[]----------------------------------------------------------------------[]*/
/*| Suica Initialize                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_init                                              |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void suica_init( void )
{
	short	ct;
	
	if( SuicaParamGet() ){		// if( prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1) �𔻒�
		return;					// �ݒ�ُ�
	}

	Suica_Rec.Data.BIT.INITIALIZE = 0;				/* Init Suica Reader */
	suica_errst = 0;								
	memset( suica_err,0,sizeof(suica_err) );
	memset( &suica_work_time, 0, sizeof( suica_work_time ));

    Suica_Rec.snd_kind = 0;
	Ope_Suica_Status = 0;							// Ope������̑��M�ð��������
	memset( &Suica_Snd_Buf,0,sizeof( SUICA_SEND_BUFF ));

	for( ct=0;ct<5; ct++){
		switch( ct ){
		    case 0: 								/* Stand-by */
				Suica_Rec.Com_kind = 0;					
				Suica_Rec.Status = STANDBY_BEFORE;
				break;
		    case 1: 								/* All req. */
				Suica_Rec.Com_kind = 1;					
				Suica_Rec.Status = STANDBY_SND_AFTER;
				break;
		    case 3:									/* Out req. */
				if( Suica_Rec.Status == DATA_RCV ){
					Suica_Data_Snd( S_INDIVIDUAL_DATA,&CLK_REC );
					suica_command_set();
					Suica_Rec.Com_kind = 3;
				}
				break;
		    case 2: 								/* Input req. */
		    case 4:
				Suica_Rec.Com_kind = 2;	
				break;

		    default:
				break;
		}
		if( 4 == suicaact() ){						// ACK4 receive
			ex_errlg( ERRMDL_SUICA, ERR_SUICA_ACK4RECEIVE, 2, 0 ); /* �װ۸ޓo�^(ACK4��M) */
			return;									// ����POL�_�@�ŃX�^���o�C���M(�C�j�V��������)
		}
		suica_errst = suica_err[0];
		if( suica_errst )
			return;
		cnwait( 5 );								/* 100msec wait */
	}
	Suica_Rec.Data.BIT.INITIALIZE = 1;				/* Finished Initial */
	
	queset( OPETCBNO, SUICA_INITIAL_END, 0, NULL );	// OpeTask��Suica�̏�����������ʒm
	
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Command Set                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_event                                             |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void suica_event( void )
{
		if( !Suica_Rec.Data.BIT.INITIALIZE ){
			suica_init();
			return;
		}
		suica_command_set();							/* ���s�������ނ�I������ */
		switch( suicaact()){							/* SuicaҲ�ۼޯ� */
			case 1: /* OK */
				break;
			case 2: 									/* Retry Err */
				suica_errst = suica_err[0];
				break;
			case 4: /* ACK4 */
				ex_errlg( ERRMDL_SUICA, ERR_SUICA_ACK4RECEIVE, 2, 0 ); /* �װ۸ޓo�^(ACK4��M) */
				suica_errst = suica_err[0];
				cnwait( 50 );							/* 1sec wait */
				suica_init();							/* Excute Initial */
				break;
			case -1: 									/* Receive Time Out */
			case -9: 									/* Send Time Out */
				if( (suica_errst & 0xc0 ) == 0 ){		/* Already Error? YJ */
					suica_errst = suica_err[0];
				}
				break;
			default:
				break;
		}
			
		return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Command Set                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_command_set                                       |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2007-02-19                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void suica_command_set( void )
{

	uchar	snd_kind = 0;
	memset( suica_work_buf,0,sizeof( suica_work_buf ));
	Suica_Snd_regist( &snd_kind , suica_work_buf );
	Suica_Rec.snd_kind = snd_kind;

	switch( Suica_Rec.snd_kind ){
		case S_CNTL_DATA: 					/* �����ް��̑��M�v�� */
			Suica_Rec.Snd_Size = 3;
			break;
		case S_SELECT_DATA:					/* ���i�I���ް��̑��M�v�� */
			Suica_Rec.Snd_Size = 9;
			break;
		case S_INDIVIDUAL_DATA:				/* ���ް��̑��M�v�� */
			Suica_Rec.Snd_Size = 25;
			break;
		case S_PAY_DATA:					/* ���Z�ް��̑��M�v�� */
			Suica_Rec.Snd_Size = 10;
			break;
		default:							/* ����ޑ��M�v���Ȃ� */
			Suica_Rec.Com_kind = 2;			/* Polling���{ */
			return;
			break;
	}

	Suica_Rec.Snd_Buf[0] = (unsigned char)(Suica_Rec.Snd_Size-1);	/* ���M�ް����{��ʂ̍��v���� */
	Suica_Rec.Snd_Buf[1] = Suica_Rec.snd_kind;						/* ���M��� */
	Suica_Rec.Com_kind = 3;												/* �ް����M�v�� */
	memcpy( &Suica_Rec.Snd_Buf[2],suica_work_buf,((sizeof(Suica_Rec.Snd_Buf[2]))*(Suica_Rec.Snd_Size-2)));	/* ���M�ޯ̧���ް���� */
	Suica_Rec.Snd_Buf[Suica_Rec.Snd_Size] = bcccal( (char*)Suica_Rec.Snd_Buf, Suica_Rec.Snd_Size );		/* BCC�ް��t�� */
	Suica_Rec.Snd_Size = Suica_Rec.Snd_Size+1;															/* ���M�ް���İ�ٻ��޾�� */

}


/*[]----------------------------------------------------------------------[]*/
/*| Suica-reader Communication Main  				                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suicaact                                                |*/
/*| PARAMETER	 : None										               |*/
/*| RETURN VALUE : short		| 1:OK	<>1:Error                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka	                                           |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short suicaact( void )
{
	short	rtn;

	rtn = 0;
	
	suica_errst = 0;								
	memset(suica_err,0,sizeof(suica_err));			/* �װ�i�[�ر�̏����� */
	Suica_Rec.faze = 1;								/* ����ޑ��M̪��ވڍs */

	for( ; rtn==0 ; ){
		switch( Suica_Rec.faze ){
			case 1:									/* ����ޑ��M */
				rtn = suica_01();
				break;
			case 2:									/* ����ޑ��M���ʎ擾̪��� */
				rtn = suica_02();
				break;
			case 3:									/* �ް����M̪��� */
				rtn = suica_03();
				break;
			case 4:									/* �ް����M���ʎ�M̪��� */
				rtn = suica_04();
				break;
			default:
				break;
		}
		switch( rtn ) {
			case 0:	 								/* Continue */
				continue;
				break;
			case 4:  								/* ACK4 */
				break;
			case -9: 								/* ���MTimeOut */
				if( suica_errst & 0x80 ){  			/* Already Timeout?  */
					suica_err[0] |= 0x80;			
					break;
				}
				suica_err[2]++;						/* Time Out Retry Counter +1 */
				if( suica_err[2] <= SUICA_RTRY_COUNT_31 ){  /* ��ײ��31�����̏ꍇ */
					Suica_Rec.faze = 1;                     /* ��ײ�����{ */
					Suica_Rec.Com_kind = 5;
					rtn = 0;
				}else{
					suica_err[0] |= 0x80;			/*�װ��� */		
				}
				break;
			case -1: 								/* ��MTimeOut */
				if( Suica_Rec.Status == STANDBY_SND_AFTER ){ /* �ð��������޲����ޑ��M�� */
					Suica_Rec.Com_kind = 0;					 /* �ēx�A����޲����ނ𑗐M���� */
					Suica_Rec.faze = 1;
					suica_err[0] |= 0x40;			/*�װ��� */
					break;
				}
					
				if( suica_errst & 0x40 ){  			/* Already Timeout?  */
					suica_err[0] |= 0x40;			
					break;
				}
				suica_err[1]++;						/* Time Out Retry Counter +1 */
				if( suica_err[1] <= SUICA_RTRY_COUNT_31 ){  /* ��ײ��31�����̏ꍇ */
					Suica_Rec.faze = 1;                     /* ��ײ�����{ */
					rtn = 0;
				}else{
					suica_err[0] |= 0x40;			/*�װ��� */		
				}
				break;
			default:
				break;
		}
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 1...Send Command                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_01		                                           |*/
/*| PARAMETER	 : None									                   |*/
/*| RETURN VALUE : short 	| 0:Continue -9:Send Timeout                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static short suica_01()
{
	short	sig;
	unsigned char	*scp;

	sig = Suica_Rec.Com_kind;			/* ���M����ނ�ܰ��ر�ɐݒ� */
	scp = (unsigned char *)&suica_tbl[sig][0];			/* ���M�����ID��ݒ� */

	SUICA_RCLR();						// ��M�o�b�t�@�N���A
	if( CNM_CMD( scp, sig ) ) {			/* Send COMMAND */
		return( -9 );					/* TIME OUT */
	}
	Suica_Rec.Status = (unsigned char)(sig+1); /* ���݂̏�Ԃ�ۑ� */

	switch( sig ){
		case 2:
		case 0:
		case 1:
		case 4:
			Suica_Rec.faze = 2;			/* ����ޑ��M���ʎ擾̪��ނɈڍs */
			break;
		case 3:
		case 5:
			Suica_Rec.faze = 3;			/* �ް����M̪��ނɈڍs */
			break;
		default:
			break;
	}
	if( sig == 4 ){
		err_chk( ERRMDL_SUICA, ERR_SUICA_REINPUT, 2, 0, 0 );		
	}else if( sig == 5 ){
		err_chk( ERRMDL_SUICA, ERR_SUICA_REOUTPUT, 2, 0, 0 );
	}
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 2...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_02                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short    | 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka	                                           |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static short suica_02( void )
{
	short	rtn;
	unsigned char	status;
	
	status = Suica_Rec.Status;												/* �ð���̕ێ� */

	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* �ް���M */
		// 0�ȊO��-1�i�^�C���A�E�g�j
suica_02_10:
		rtn = -1;															// �^�C���A�E�g�ŏ�������
		if( 2 == Suica_Rec.Com_kind ){										// ���͗v�����M��̖�����
			Suica_Rec.Com_kind = 4;											// ���͍ėv���R�}���h���M
		}																	// �ꊇ�v���̖������͍ēx�ꊇ�v�����s�����ƂƂ���
		return( rtn );
	}

	switch( rtn = suica_recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){	/* ��M�ް���͏��� */
		case 1: /* ACK1 */
			if( ( status == STANDBY_SND_AFTER ||							/* ����ޑ��M��A�ް����M�v��������ꍇ�A   */
				  status == INPUT_SND_AFTER   ||							/* �o�͗v������ޑ��M��A�ް����M���s�� */
				  status == REINPUT_SND_AFTER )) {
				  if( Suica_que_check() ){
					suica_command_set();
					Suica_Rec.faze = 1;
					Suica_Rec.Com_kind = 3;										/* �o�͗v������ޑ��M�������s�� */
					rtn = 0;
					break;
				  }
			}
			rtn = 1;														/* ���͗v�����s�� */
			break;
		case 2: /* ACK2 */
		case 3: /* ACK3 */
			if( (Suica_Rec.Com_kind == 0) ||								// ����޲����ޑ��M��
				(Suica_Rec.Com_kind == 1) ){								// �ꊇ�v�����M��
				goto suica_02_10;											// ����
			}
			rtn = 1;														
			break;
		case 6: /* NAK */
			if( (Suica_Rec.Com_kind == 0) ||								// ����޲����ޑ��M��
				(Suica_Rec.Com_kind == 1) ){								// �ꊇ�v�����M��
				goto suica_02_10;											// �đ�
			}
			rtn = 1;
			Suica_Rec.Status = DATA_RCV;									// ��UNIT�ւ�POL�Ɉڂ�
			break;
		case 4: /* ACK4 */
			Suica_Rec.Com_kind = 0;											/* ����޲����ޑ��M */
			Suica_Rec.faze = 1;
			Suica_Rec.Status = DATA_RCV;									// �������ݽ���ł���΁A��UNIT�ւ�POL�Ɉڂ莟�̓X�^���o�C
																			// �������ݽ���łȂ���΁A�����X�^���o�C���M
			break;
		case 5: /* ACK5 */
			goto suica_02_10;
			break;

		case 9:	/* LRC ERROR */												// PHY error�A�����O�X�ُ���܂�
			if( Suica_Rec.Com_kind == 0 ){									// ����޲���M��
				goto suica_02_10;											// �����i����޲����ނ̍đ��j
			}
			if( suica_errst & 0x10 ){  										/* Already RetryErr?  */
				suica_err[0] |= 0x10;
				rtn = 2;
				break;
			}
			suica_err[4]++;													/* ��ײ�񐔂��ı��� */
			if( suica_err[4] <= SUICA_RTRY_COUNT_31 ){						// ��ײ���ް�łȂ�
				if( 2 == Suica_Rec.Com_kind ){								// ���͗v�����M��̍đ��i�ꊇ�v�����͍đ����ꊇ�j
					Suica_Rec.Com_kind = 4;									// ���͍ėv���R�}���h���M
				}
				Suica_Rec.faze = 1;
				rtn = 0;	/* Retry */
			}else{															/* ��ײ�񐔵��ް */
				suica_err[0] |= 0x10;										/* �װ��� */
				rtn = 2;
			}
			break;
		default:
			goto suica_02_10;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 3...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_03                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short	| 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static short suica_03( void )
{
	short	rtn;
	unsigned char	status;
	
	status = Suica_Rec.Status;												/* �ð���̕ێ� */

	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* �ް���M */
suica_03_10:
		rtn = -1;															// �^�C���A�E�g�ŏ�������
		return( rtn );
	}

	switch( rtn = suica_recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){	/* ��M�ް���͏��� */
		case 1: /* ACK1 */
			if( CNM_SND(Suica_Rec.Snd_Buf, Suica_Rec.Snd_Size) == 0 ){ 		/* �ް����M */
				Suica_Rec.snd_kind = 0;
				Suica_Log_regist( &Suica_Rec.Snd_Buf[1],(ushort)(Suica_Rec.Snd_Size-2), 1 );	/* �ʐM���O�o�^ */	
				
				#if (4 == AUTO_PAYMENT_PROGRAM)								// �����p�ɒʐM���O�擾������ύX����
				Suica_Log_regist_for_debug( &Suica_Rec.Snd_Buf[1],(ushort)(Suica_Rec.Snd_Size-2), 1 );	/* �ʐM���O�o�^ */	
				#endif
				

				Suica_Rec.faze = 4;								/* �ް����M��A���M����̪��ނɈڍs */
				rtn = 0;
				break;
			}else{
				rtn = -9;
				break;
			}
		case 2: /* ACK2 */
		case 3: /* ACK3 */
			rtn = 1;											/* OK */
			break;
		case 4: /* ACK4 */										/* �o�͗v���E�o�͍ėv������ޑ��M���ACK4��M�� */				
			Suica_Rec.faze = 1;								    /* ����޲����ޑ��M */ 			
			Suica_Rec.Com_kind = 0;												
			break;
		case 5: /* ACK5 */
			if( status == REOUTPUT_SND_AFTER ){					/* �o�͍Ďw�ߑ��M�� */
				Suica_Rec.faze = 4;								// ���M�o�����Ɣ��f���A���M�f�[�^����
				if( Suica_que_check() ){							/* ����ޑ��M�v��������ꍇ */
					suica_command_set();
					Suica_Rec.faze = 1;								
					Suica_Rec.Com_kind = 3;							/* �o�͎w�ߑ��M */
					rtn = 0;		
				}
				else{
					Suica_Rec.faze = 3;							// ���M�f�[�^�����̂��߂ɕς����l�����ɖ߂�
					rtn = 1;									// ���̎����ɏo�͗v������(ACK5���̓f�[�^�̍đ������Ȃ�)
				}
			}
			else{												// �o�͎w��
				goto suica_03_10;								// �đ�
			}
			break;
		case 6: /* NAK */
			ex_errlg( ERRMDL_SUICA, ERR_SUICA_NAKRECEIVE, 2, 0 ); /* �װ۸ޓo�^(NAK��M) */
			if( suica_errst & 0x08 ){  							/* Already RetryErr?  */
				suica_err[0] |= 0x08;							/* �װ���  */
				rtn = 2;
				break;
			}
			suica_err[3]++;										/* ��ײ�񐔂��ı��� */
			if( suica_err[3] <= SUICA_RTRY_COUNT_21 ){			/* ��ײ�񐔂�21�񖢖� */
				Suica_Rec.faze = 1;								/* ��ײ���{ */
				wait2us( 2500L ); 								/* 5ms wait */
				rtn = 0;
			}else{
				suica_err[0] |= 0x08;							/* �װ��� */
				rtn = 2;
			}
			break;
		default:
			break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 4...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_04                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short | 0:Continue 1:OK ETC:Error                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka	                                           |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static short suica_04( void )
{
	short	rtn;
	
	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* �ް���M */
		rtn = -1;															// �^�C���A�E�g�ŏ�������
		Suica_Rec.Com_kind = 5;												// ���͏o�͍Ďw�ߑ��M
		return( rtn );
	}
	switch( rtn = suica_recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){
		case 1: /* ACK1 */
			if( Suica_Rec.Snd_Buf[1] == S_SELECT_DATA ){
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 1;						// Suica���Z���t���OON�I
				Suica_Rec.Data.BIT.SEND_CTRL80 = 0;
			}
			else if(( Suica_Rec.Snd_Buf[1] == S_CNTL_DATA && Suica_Rec.Snd_Buf[2] == 0x80 ) && 
					  !Suica_Rec.Data.BIT.SEND_CTRL80 ){
				Suica_Rec.Data.BIT.SEND_CTRL80 = 1;
			}
		case 5: /* ACK5 */
			if( Suica_que_check() ){
				suica_command_set();
				Suica_Rec.faze = 1;								     
				rtn = 0;		
			}else 	rtn = 1;		

			break;
		case 2: /* ACK2 */
		case 3: /* ACK3 */
			rtn = 1; /* OK */
			break;
		case 4: /* ACK4 */										/* �o�͗v���E�o�͍ėv������ޑ��M���ACK4��M�� */				
			Suica_Rec.faze = 1;								    /* ����޲����ޑ��M */ 			
			Suica_Rec.Com_kind = 0;												
			break;
		case 6: /* NAK */
			ex_errlg( ERRMDL_SUICA, ERR_SUICA_NAKRECEIVE, 2, 0 ); 			/* �װ۸ޓo�^(NAK��M) */
			if( suica_errst & 0x08 ){  										/* Already RetryErr?  */
				suica_err[0] |= 0x08;
				rtn = 2;
				break;
			}
			suica_err[4]++;
			if( suica_err[4] <= SUICA_RTRY_COUNT_3 ){
				Suica_Rec.faze = 1;
				Suica_Rec.Com_kind = 5;							// �o�͍Ďw��
				rtn = 0;
			}else{
				suica_err[0] |= 0x08;
				rtn = 2;
			}
			break;

		case 9:	/* LRC ERROR */
		default:
			if( suica_errst & 0x10 ){  										/* Already RetryErr?  */
				suica_err[0] |= 0x10;
				rtn = 2;
				break;
			}
			suica_err[4]++;
			if( suica_err[4] <= SUICA_RTRY_COUNT_3 ){
				Suica_Rec.Com_kind = 5;										// ���͏o�͍Ďw��
				Suica_Rec.faze = 1;
				rtn = 0;	/* Retry */
			}else{
				suica_err[0] |= 0x10;
				rtn = 2;
			}
			break;
	}
	return( rtn );
}


/*[]----------------------------------------------------------------------[]*/
/*| Suica Snd Data que Check                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Suica_que_check                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-09-29                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar Suica_que_check( void )
{
	uchar i;

	if( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind && Suica_Rec.faze == 4 ){
		Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind = 0;

		if( Suica_Snd_Buf.read_wpt > 3 )
			Suica_Snd_Buf.read_wpt = 0;
		else
			Suica_Snd_Buf.read_wpt++;
	}

	for( i=0;i<5;i++ ){
		if( Suica_Snd_Buf.Suica_Snd_q[i].snd_kind ){
			return 1;
		}
	}
	if( Suica_Snd_Buf.read_wpt != Suica_Snd_Buf.write_wpt )		// �����M�ް����Ȃ��ARpt��Wpt������Ă���ꍇ
		Suica_Snd_Buf.read_wpt = Suica_Snd_Buf.write_wpt;		// Rpt��Wpt�ɂ��킹�A���M�̖������Ȃ���

	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| Get Receive Charactor Inf.                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_recv                                              |*/
/*| PARAMETER	 : char *buf    : Receive Buffer Address                   |*/
/*|              : short   siz    : number of Receive Charactor            |*/
/*| RETURN VALUE : short ret;     : 1:ACK1+DATA                            |*/
/*|              : 2:ACK2+DATA                                             |*/
/*|              : 3:ACK3+DATA                                             |*/
/*|              : 4:ACK4                                                  |*/
/*|              : 5:ACK5                                                  |*/
/*|              : 6:NAK                                                   |*/
/*|              : 7:ETC. Char.                                            |*/
/*|              : 9:LRC ERROR                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short suica_recv( unsigned char *buf,short siz )
{
	short	i, rtn, bc;
	unsigned char	*ack;

	// ���ό��ʃf�[�^�����͏����ςŖ��������M���������Ȃ�
	if( (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 		// ���ό����ް�����M��
		(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) )		// ���ό����ް��������̏ꍇ
	{
		Suica_Rec.suica_rcv_event.BYTE = 0;
		Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;				// ���ό��ʃf�[�^�����͏����Ȃ�
	}
	else{
		Suica_Rec.suica_rcv_event.BYTE = 0;
	}
	Suica_Rec.suica_rcv_event_sumi.BYTE = 0;

	rtn = 0;
	ack = (unsigned char *)ack_tbl;
	for( i=1; i<7; i++,ack++){
		if(*buf == *ack) break;
	}

	if(( siz > 1 ) && ( i < 4 )) {		// ACK2,3 + �f�[�^�d����M
		buf++;
		bc = (short)(*buf);	/* bc <- Total Byte */
		for( ; ; ){
		    if( siz != (bc+3) ){	/* Recieve Size OK ? YJ */
				ex_errlg( ERRMDL_SUICA, ERR_COIN_RECEIVESIZENG, 2, 0 );	// Recv packet size error
				i = 9;			/* Size Error 	*/
				break;
		    }
		    if( (unsigned char)(*(buf+bc+1)) != bcccal( (char*)buf, (short)(bc+1) ) ){ /* LRC Ok ? YJ */
				ex_errlg( ERRMDL_SUICA, ERR_COIN_RECEIVEBCCNG, 2, 0 );	// BCC error
				i = 9;			/* LRC Error */
				break;
		    }

			if( Suica_Rec.Status == STANDBY_SND_AFTER ){		// ����޲���M����ް���M�̂����ް��j��
				SUICA_RCLR();	/* Recieve Inf. Clear */
				return (i);
			}

			if( i >= 1 || i <= 3 ){
				Suica_Log_regist( (uchar*)(buf+(i!=2?1:0)),(ushort)(i!=2?bc:bc+1), 0 );
				buf--;											// ��M�f�[�^�̐擪�o�C�g���w��
				Suica_recv_data_regist( buf,(ushort)(bc+2) );	// ��M�L���[�֊i�[
				queset( OPETCBNO, SUICA_EVT, 0, NULL );			// OpeTask�֎�M�ʒm
				Suica_Rec.Status = DATA_RCV;
			}

		    break;
		}
	}else if( siz == 1 ){
		if( *buf == 0x11 )
			Suica_Rec.Status = DATA_RCV;
	}
	SUICA_RCLR();	/* Recieve Inf. Clear */
	rtn = i;
	return( (short)rtn );
}


/*[]----------------------------------------------------------------------[]*/
/*| Suica Trouble   	                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suicatrb                                                |*/
/*| PARAMETER	 : err_kind:�װ���							               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void suicatrb( unsigned char err_kind )
{

	if( err_kind == ERR_SUICA_COMFAIL ){									/* �װ��ʂ��ʐM�s�ǂ̏ꍇ */
		err_suica_chk(  &suica_errst ,&suica_errst_bak,ERR_SUICA_COMFAIL );	/* �װ���� */
		Suica_Rec.suica_err_event.BIT.COMFAIL = 1;							/* �ʐM�s�Ǵװ�׸�ON */
		suica_errst_bak = suica_errst;										/* �װ��Ԃ��ޯ����� */
	}

	if( !Suica_Rec.Data.BIT.RESET_COUNT ){									/* �����������s�H */
		_di();																/* �������������s */
		Sci2Init();
		_ei();
		Suica_reset();
		suica_init();
		Suica_Rec.Data.BIT.RESET_COUNT = 1;									/* �������ς��׸޾�� */
		Suica_Rec.Data.BIT.EVENT ^= 0x01;									/* Suica����Đ؂�ւ�(���ү���������ڍs) */

	}

}

/*[]----------------------------------------------------------------------[]*/
/*| RECEIVE FLG RESET                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SUICA_RCLR                                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void SUICA_RCLR( void )
{
	RXRSLT.BYTE = 0;
	RXCONT = 0;
	RXDTCT = 0;
	RXWAIT = -1;
	memset( Suica_Rec.Rcv_Buf, 0, sizeof( Suica_Rec.Rcv_Buf ) );
}

/*[]----------------------------------------------------------------------[]*/
/*| SUICA Reder Hard Reset                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CN_reset                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-07-11                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void Suica_reset( void )
{
	CP_CN_SYN = 0;			/* SYM Enable	*/
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	CP_CN_SYN = 1;			/* SYM Disnable	*/
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
	cnwait( SYN_OUT_TIM );	/* Wait 1.0sec */
}


/*[]----------------------------------------------------------------------[]*/
/*| snd_ctrl_timeout                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : snd_ctrl_timeout                                        |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2007-02-26                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void snd_ctrl_timeout( void )
{
	unsigned char wks = 0;

	wks = (uchar)prm_get(COM_PRM, S_SCA, 14, 2, 1);		/* �ď�������ײ�񐔎擾 */	
	LagCan500ms( LAG500_SUICA_STATUS_TIMER );			/* SX-10��ԊĎ����ؾ�� */
	if( Retrycount < wks ){								/* �ď�������ײ�񐔵��ް�H */
		suicatrb( ERR_SUICA_RECEIVE );					/* �������������s */
		Retrycount++;									/* ��ײ�����̍X�V */
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			��M�ް��o�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		buf		:�o�^�����ް��߲��
///	@param[in]		size	:�o�^�����ް�����<br>
///	@return			void		:
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 2008/07/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void 	Suica_recv_data_regist( uchar *buf , ushort size )
{
	uchar	*wbuf = suica_rcv_que.rcvdata[suica_rcv_que.writept];					// ���񏑍��ގ�M�ޯ̧�߲�����
	uchar	wsize;																	// �����p�ر
	ushort	i;																		// ٰ�ߗp�ϐ�
	ushort	w_counter[3];															// �̾�ĕҏW�ر

	memcpy( w_counter, &suica_rcv_que.readpt, sizeof( w_counter ) );				// �̾�Ă�ܰ��̈�ɓW�J
	if( (w_counter[2] == SUICA_QUE_MAX_COUNT) && (w_counter[1] == w_counter[0]) ){	// �ޯ̧���ُ�Ԃ̏ꍇ

		for( i=0; (i<6 && *wbuf != ack_tbl[i]); i++);								// ��M���(ACK1�`NACK)����

		if( i == 1 ){																// �P���ް��̏ꍇ
			if( *(wbuf+2) == S_SETTLEMENT_DATA && *(wbuf+3) == 0x01 ){				// �L���Ȍ��ό����ް��̏ꍇ
				Settlement_rcv_faze_err( wbuf+3, 6 );								// �װ�o�^(E6961)
			}			
		}else{																		// �����ް��̏ꍇ
			wsize = *(wbuf+1);														// �ް����ނ�ݒ�
			wbuf++;																	// �ް��߲�����X�V
			for( i=0; i<wsize; ){													// �S�ް�����������܂�ٰ��
				if( *(wbuf+2) == S_SETTLEMENT_DATA && *(wbuf+3) == 0x01 ){			// �L���Ȍ��ό����ް�
					Settlement_rcv_faze_err( wbuf+3, 6 );							// �װ�o�^(E6961)
				}
				i+=*(wbuf+1)+1;														// �����ς݂��ް����ނ��X�V
				wbuf+=*(wbuf+1)+1;													// �ް��߲�����X�V���A�����ް����Q��
			}			
		}

		if( ++w_counter[0] >= SUICA_QUE_MAX_COUNT ){								// �Ǎ��ݵ̾�čX�V�����ɓǍ��޵̾�Ă�MAX�z���̏ꍇ
			w_counter[0] = 0;														// �̾�Ă�擪�Ɉړ�
		}
	}
	memcpy( suica_rcv_que.rcvdata[w_counter[1]], buf, (size_t)size );				// ��M���Ă����ް�����M����ɓo�^
	if( ++w_counter[1] >= SUICA_QUE_MAX_COUNT ){									// �����ݵ̾�čX�V�����ɓǍ��޵̾�Ă�MAX�z���̏ꍇ
		w_counter[1] = 0;															// �̾�Ă�擪�Ɉړ�
	}
	
	if( w_counter[2] < SUICA_QUE_MAX_COUNT ){										// �ް����Ă�MAX�łȂ����
		w_counter[2]++;																// �ް����Ă�ݸ����
	}
	nmisave( &suica_rcv_que.readpt, w_counter, sizeof(w_counter));					// �̾�Ă��d�ۏ�ōX�V

}
