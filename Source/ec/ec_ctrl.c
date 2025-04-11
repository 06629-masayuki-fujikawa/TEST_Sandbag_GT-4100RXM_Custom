/*[]----------------------------------------------------------------------[]*/
/*| EC CRW-MJA R/W Communication Control                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : G.So                                                     |*/
/*| Date        : 2019.02.07                                               |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
#include	<stdio.h>
#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"suica_def.h"
#include	"ec_def.h"
#include	"prm_tbl.h"

/*[]----------------------------------------------------------------------[]*/
/*| ���σ��[�_���䕔 ����                                                     |*/
/*[]----------------------------------------------------------------------[]*/
const	ver_rec		VERSNO_EC_COMM = {
// MH321800(S) ���ԕύX
//	'0',' ','G','G','1','1','6','8','0','0'				// ���σ��[�_�Ή�����
	'0',' ','M','H','3','4','3','7','0','0'				// ���σ��[�_�Ή�����
// MH321800(E) ���ԕύX
};

// MH810103 GG119202(S) �N���V�[�P���X�s��C��
extern	int		ModuleStartFlag;
// MH810103 GG119202(E) �N���V�[�P���X�s��C��

/*[]----------------------------------------------------------------------[]*/
/*| CRW-MJA Initialize                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : crwmja_init                                             |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void ec_init( void )
{
	short	ct;
//MH810103  GG119202(S) �N���V�[�P���X�s��C��
//// GG119200(S) �N���V�[�P���X��������������
////	short	max = 14;
//	short	max = 15;
//// GG119200(E) �N���V�[�P���X��������������
	short	max = 4;
// MH810103 GG119202(E) �N���V�[�P���X�s��C��

// setup
	jvma_setup.mdl = ERRMDL_EC;
	jvma_setup.recv = ec_recv;

// init.
	ec_flag_clear(1);								// �֘A�t���O������

	suica_errst = 0;
	memset( suica_err,0,sizeof(suica_err) );
	memset( &suica_work_time, 0, sizeof( suica_work_time ));

	Suica_Rec.snd_kind = 0;
	Ope_Suica_Status = 0;							// Ope������̑��M�ð��������
	memset( &Suica_Snd_Buf,0,sizeof( SUICA_SEND_BUFF ));

// initial seq.
	for( ct=0;ct<max; ct++ ){
		switch( ct ){
		case 0: 									/* Stand-by */
			Suica_Rec.Com_kind = 0;					
			Suica_Rec.Status = STANDBY_BEFORE;
			break;
		case 1: 									/* All req. */
			Suica_Rec.Com_kind = 1;					
			Suica_Rec.Status = STANDBY_SND_AFTER;
			break;
		case 3:										/* Out req. */
			if( Suica_Rec.Status == DATA_RCV ){
				Ec_Data_Snd( S_INDIVIDUAL_DATA,&CLK_REC );
				jvma_command_set();
			}
			break;
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//		case 12:									/* Out req. */
//			if( Suica_Rec.Status == DATA_RCV ){
//// GG119200(S) �N���V�[�P���X��������������
//				if( !isEcBrandStsDataRecv() ) {		// �S�u�����h��ԃf�[�^����M
//					break;
//				}
//// GG119200(E) �N���V�[�P���X��������������
//				SetEcVolume();
//				Ec_Data_Snd( S_VOLUME_CHG_DATA, &VolumeTbl );
//				jvma_command_set();
//			}
//			break;
//		case 2: 									/* Input req. */
//		case 4:										/* wait for brand status 1-01 */
//		case 6:										/* wait for brand status 1-02 */
//		case 8:										/* wait for brand status 2-01 */
//		case 10:									/* wait for brand status 2-02 */
//		case 13:									/* wait for volume result */
		case 2: 									/* Input req. */
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
			Suica_Rec.Com_kind = 2;
			break;
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//													/* Out req. */
//		case 5:										/* send brand setting 1-01 */
//		case 7:										/* send brand setting 1-02 */
//		case 9:										/* send brand setting 2-01 */
//		case 11:									/* send brand setting 3-02 */
//			// �ʏ��ec_recv()����SUICA_EVT��opetask�ɒʒm�����
//			// Ope_Ec_Event()���Ă΂�邪�A�ی��̂��߂��̃^�C�~���O�ł�
//			// Ope_Ec_Event()���Ă�Ŏ�M�������s��
//			Ope_Ec_Event(0, 0);
//			jvma_command_set();
//			if(ct == 7 && ECCTL.brand_num <= EC_BRAND_MAX) {
//				// �u�����h�l�S�V�G�[�V�����P�������Ƀu�����h��ԃf�[�^�P�Œʒm���ꂽ
//				// �u�����h����10���𒴂���ꍇ�̂݃u�����h�l�S�V�G�[�V�����Q�����{
//				// �u�����h����10�������̏ꍇ�͉��ʎ��ԑѐݒ�f�[�^(DC=31H)�𑗐M���邽��
//				// ct��+4����12�Ƃ���(for����+1����邽�߁A�����ł�+4�Ƃ���)
//				ct += 4;
//			}
//			break;
//// GG119200(S) �N���V�[�P���X��������������
//		case 14:									// ���ʕύX���ʃf�[�^��M���Ɏ��{
//			Ope_Ec_Event(0, 0);
//			break;
//// GG119200(E) �N���V�[�P���X��������������
// MH810103 GG119202(E) �N���V�[�P���X�s��C��

		default:
			break;
		}
		if( 4 == jvma_act() ){						// ACK4 receive
			// �������V�[�P���X����ACK4��M�G���[��o�^���Ȃ�
			return;									// ����POL�_�@�ŃX�^���o�C���M(�C�j�V��������)
		}
		suica_errst = suica_err[0];
		if( suica_errst )
			return;
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
//		cnwait( 5 );								/* 100msec wait */
		cnwait( 2 );								/* 40msec wait */
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
		if (ModuleStartFlag != 0) {
			Ope_Ec_Event(0, 0);
		}
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
	}

// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//// GG119200(S) �N���V�[�P���X��������������
////	if( !Suica_Rec.Data.BIT.BRAND_STS_RCV ){
////		// �u�����h��ԃf�[�^����M���ĂȂ� = �N���V�[�P���X���s���Ă��Ȃ��Ƃ݂Ȃ�
////		return;
////	}
//	if( ( !Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.VOL_RES_RCV ) ) {	// ���ʕύX���ʃf�[�^����M
//		return;
//	}
//// GG119200(E) �N���V�[�P���X��������������
//
//	Suica_Rec.Data.BIT.INITIALIZE = 1;				/* Finished Initial */
//	ECCTL.phase = EC_PHASE_PAY;						// ������������
//	ECCTL.step = 0;
	ECCTL.phase = EC_PHASE_BRAND;				// �u�����h���킹��
	ECCTL.step = 0;

	// �ŏ��̃u�����h��ԃf�[�^��M�҂�
	Lagtim(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT, EC_BRAND_NEGO_WAIT_TIME);	// ��M�҂��^�C�}�J�n
// MH810103 GG119202(E) �N���V�[�P���X�s��C��

// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//	LagCan500ms(LAG500_EC_WAIT_BOOT_TIMER);			// �N�������҂��^�C�}��~
//	// E3210����
//	err_chk( ERRMDL_EC, ERR_EC_UNINITIALIZED, 0, 0, 0 );
//
//	queset( OPETCBNO, SUICA_INITIAL_END, 0, NULL );	// OpeTask��Suica�̏�����������ʒm
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
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
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short ec_recv( unsigned char *buf,short siz )
{
	short	i, rtn, bc;
	unsigned char	*ack;

	// ���ό��ʃf�[�^�����͏����ςŖ��������M���������Ȃ�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if( (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 		// ���ό����ް�����M��
//		(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) ){	// ���ό����ް��������̏ꍇ
//		Suica_Rec.suica_rcv_event.BYTE = 0;
//		Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;				// ���ό��ʃf�[�^�����͏����Ȃ�
//	}
//	else if( (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 	// ���ό����ް�����M��
//			(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 1) ){// ���ό����ް������ς݂̏ꍇ
//		Suica_Rec.suica_rcv_event.BYTE = 0;
//	}
//	else if( Suica_Rec.suica_rcv_event.BIT.SETTSTS_DATA == 1 ){
	if( (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1) && 			// ���ό����ް�����M��
		(Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 0) ){		// ���ό����ް��������̏ꍇ
		Suica_Rec.suica_rcv_event.BYTE = 0;
		Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;				// ���ό��ʃf�[�^�����͏����Ȃ�
	}
	else if( (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1) && 	// ���ό����ް�����M��
			(Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 1) ){	// ���ό����ް������ς݂̏ꍇ
		Suica_Rec.suica_rcv_event.BYTE = 0;
	}
	else if( Suica_Rec.suica_rcv_event.EC_BIT.SETTSTS_DATA == 1 ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
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
				ex_errlg( jvma_setup.mdl, ERR_EC_DATANUM, 2, 0 );	// Recv packet size error
				i = 9;			/* Size Error 	*/
				break;
		    }
		    if( (unsigned char)(*(buf+bc+1)) != bcccal( (char*)buf, (short)(bc+1) ) ){ /* LRC Ok ? YJ */
				ex_errlg( jvma_setup.mdl, ERR_EC_DATABCC, 2, 0 );	// BCC error
				i = 9;			/* LRC Error */
				break;
		    }

			if( Suica_Rec.Status == STANDBY_SND_AFTER ){		// ����޲���M����ް���M�̂����ް��j��
				SUICA_RCLR();	/* Recieve Inf. Clear */
				Suica_Rec.Status = DATA_RCV;
				return (i);
			}

			if( i >= 1 && i <= 3 ){
				jvma_setup.log_regist( (uchar*)(buf+(i!=2?1:0)),(ushort)(i!=2?bc:bc+1), 0 );
				buf--;											// ��M�f�[�^�̐擪�o�C�g���w��
				Ec_recv_data_regist( buf,(ushort)(bc+2) );		// ��M�L���[�֊i�[
				queset( OPETCBNO, SUICA_EVT, 0, NULL );			// OpeTask�֎�M�ʒm
				Suica_Rec.Status = DATA_RCV;
			}
		    break;
		}
	}else if( siz == 1 ){
		if( *buf == 0x11 ){
			Suica_Rec.Status = DATA_RCV;
		}
	}
	SUICA_RCLR();	/* Recieve Inf. Clear */
	rtn = i;
	return( (short)rtn );
}

//[]----------------------------------------------------------------------[]
///	@brief			��M�ް��o�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		buf		:�o�^�����ް��߲��
///	@param[in]		size	:�o�^�����ް�����<br>
///	@return			void		:
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 2019.02.07<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void 	Ec_recv_data_regist(uchar *buf , ushort size)
{
	uchar	*wbuf = suica_rcv_que.rcvdata[suica_rcv_que.writept];					// ���񏑍��ގ�M�ޯ̧�߲�����
	uchar	wsize;																	// �����p�ر
	ushort	i;																		// ٰ�ߗp�ϐ�
	ushort	w_counter[3];															// �̾�ĕҏW�ر

	memcpy(w_counter, &suica_rcv_que.readpt, sizeof(w_counter));					// �̾�Ă�ܰ��̈�ɓW�J
	if ((w_counter[2] == SUICA_QUE_MAX_COUNT) && (w_counter[1] == w_counter[0])) {	// �ޯ̧���ُ�Ԃ̏ꍇ

		for( i=0; (i<6 && *wbuf != ack_tbl[i]); i++);								// ��M���(ACK1�`NACK)����

		if( i == 1 ){																// �P���ް��̏ꍇ
			if (*(wbuf+2) == S_SETTLEMENT_DATA) {									// ���ό��ʃf�[�^
				EcSettlementPhaseError(wbuf+3, 6);									// �װ�o�^(E3259)
				if(Suica_Rec.Data.BIT.SETTLMNT_STS_RCV) {							// ���Ϗ������ɒʐM�ُ킪�����H
					queset( OPETCBNO, EC_EVT_DEEMED_SETTLEMENT, 0, 0 );				// ope�֒ʒm
				}
			}
		}else{																		// �����ް��̏ꍇ
			wsize = *(wbuf+1);														// �ް����ނ�ݒ�
			wbuf++;																	// �ް��߲�����X�V
			for( i=0; i<wsize; ){													// �S�ް�����������܂�ٰ��
				if (*(wbuf+2) == S_SETTLEMENT_DATA) {								// ���ό��ʃf�[�^
					EcSettlementPhaseError(wbuf+3, 6);								// �װ�o�^(E3259)
					if(Suica_Rec.Data.BIT.SETTLMNT_STS_RCV) {						// ���Ϗ������ɒʐM�ُ킪�����H
						queset( OPETCBNO, EC_EVT_DEEMED_SETTLEMENT, 0, 0 );			// ope�֒ʒm
					}
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
