//[]----------------------------------------------------------------------[]
///	@brief			WAON-Suica Operation control
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/08<br>
///					Update
///	@file			ope_suica_ctrl.c
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
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

static short	RecvDspData( uchar* );
static short	RecvSettData( uchar* );
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//static short	RecvErrData( uchar* );
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
static short	RecvStatusData( uchar* );
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//static short	RecvTimeData( uchar* );
//static short	RecvFixData( uchar* );
//static short	RecvPointData( uchar* );
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) SX-20�����C��
static short	RecvErrData( uchar* );
static short	RecvTimeData( uchar* );
static short	RecvFixData( uchar* );
static short	RecvPointData( uchar* );
// MH810103 GG119202(E) SX-20�����C��

static void 	Log_data_edit( struct  suica_log_rec *data_cuf, uchar kind );
static void 	Log_data_edit2( struct  suica_log_rec *data_cuf, uchar kind );
static void 	Log_data_edit3( struct  suica_log_rec *data_cuf, ushort log_size, ushort loopcount );
void 	Suica_Log_regist( unsigned char* , ushort ,uchar);
static void 	time_snd_ope( struct clk_rec *sndbuf );
static uchar	Suica_Read_RcvQue( void );
// MH321800(S) G.So IC�N���W�b�g�Ή�
//static void 	miryo_timeout_after_disp( void );
//static void lcd_wmsg_dsp_elec( char type, const uchar *msg1, const uchar *msg2, uchar timer, uchar ann, ushort color, ushort blink );
//static void		Suica_Status_Chg( void );
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
extern void lcdbm_notice_dsp( ePOP_DISP_KIND kind, uchar DispStatus );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
#if (4 == AUTO_PAYMENT_PROGRAM)								// �����p�ɒʐM���O�擾������ύX����
void 	Suica_Log_regist_for_debug( unsigned char* , ushort ,uchar);
#endif
static void		No_Responce_Timeout( void );

typedef short (*CMD_FUNC)(uchar*);

typedef struct{
	unsigned char Command;
	CMD_FUNC Func;
}RCV_CMD_FUNC;

RCV_CMD_FUNC CommandTbl[] = {
//	{ ��M������ޯ�,�����֐�},			/* ������ */
	{ 0x02,			RecvSettData},		/* ���ό��ʃf�[�^ */
	{ 0x08,			RecvStatusData},	/* ��ԃf�[�^ */
	{ 0x01,			RecvDspData},		/* �\���˗��f�[�^ */
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//	{ 0x04,			RecvErrData},		/* �ُ�f�[�^ */
//	{ 0x16,			RecvTimeData},		/* ���������f�[�^ */
//	{ 0x32,			RecvFixData},		/* �Œ�f�[�^ */
//	{ 0x64,			RecvPointData},		/* �󎚈˗��f�[�^(�|�C���g�f�[�^) */
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) SX-20�����C��
	{ 0x04,			RecvErrData},		/* �ُ�f�[�^ */
	{ 0x16,			RecvTimeData},		/* ���������f�[�^ */
	{ 0x32,			RecvFixData},		/* �Œ�f�[�^ */
	{ 0x64,			RecvPointData},		/* �󎚈˗��f�[�^(�|�C���g�f�[�^) */
// MH810103 GG119202(E) SX-20�����C��
};

// MH321800(S) G.So IC�N���W�b�g�Ή�
//static uchar	suica_dsp_buff[30];						// ��ʕ\���p�\��������i�[�̈�
//static uchar	suica_Log_wbuff[S_BUF_MAXSIZE];			// �ʐM���O�p�ꎞ�ҏW�̈�
//static uchar	wrcvbuf[S_BUF_MAXSIZE];					// �f�[�^��M�p�̈ꎞ�ҏW�̈�
//static uchar	wpsnddata[S_BUF_MAXSIZE];
//
//#define WAR_MONEY_CHECK		(!Suica_Rec.Data.BIT.MONEY_IN && !ryo_buf.nyukin )
//
//#define	OPE_SUICA_MIRYO_TIME	prm_get(COM_PRM, S_SCA, 13, 3, 1)		// 255
//
//static	uchar	err_wk[31];
uchar	suica_dsp_buff[30];						// ��ʕ\���p�\��������i�[�̈�
uchar	suica_Log_wbuff[S_BUF_MAXSIZE];			// �ʐM���O�p�ꎞ�ҏW�̈�
uchar	wrcvbuf[S_BUF_MAXSIZE+1];				// �f�[�^��M�p�̈ꎞ�ҏW�̈�
uchar	wpsnddata[S_BUF_MAXSIZE];
uchar	err_wk[40];
uchar	moni_wk[10];
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
//// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
//const uchar msg_idx[4] = {
//	49, 50,					// 1�y�[�W��
//	51, 52,					// 2�y�[�W��
//};
//// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P

//[]----------------------------------------------------------------------[]
///	@brief			Suica Snd Data que Set & read
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/09/29<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Suica_Snd_regist( uchar *kind, uchar *snddata )
{
	uchar	count_status,Read_pt;
	uchar	wpkind;

	wpkind=*kind;										// auto�ϐ��Ɏ�ʂ�ێ� 
	memcpy( &wpsnddata,snddata,(size_t)S_BUF_MAXSIZE);	// auto�ϐ����ޯ̧�ް���ێ�

	if( *kind == 0){
		if( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind ){
			memcpy( snddata, Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].Snd_Buf, (size_t)S_BUF_MAXSIZE );
			*kind = Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind;
			return 1;
		}
	}else{

		count_status = 1;										// �ޯ̧�ð�����
		for( Read_pt=0;Read_pt<5;Read_pt++ ){					// �����M�ް��̌���
			if( !Suica_Snd_Buf.Suica_Snd_q[Read_pt].snd_kind ){	// �����M�ް��������
				count_status = 0;								// �ޯ̧�ð�����X�V
				break;
			}
		}
		if( count_status ){										// �ޯ̧���ق̏ꍇ
			for( Read_pt=0;Read_pt<BUF_MAX_DELAY_COUNT;Read_pt++ ){		// �����M�ް��̌���
				xPause( BUF_MAX_DELAY_TIME );					// �ޯ̧MAX���̑��M�����҂�
				if( !Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind ){	// �����M�ް�������Α��M�ް��̓o�^���s
					break;
				}
			}			
		}

		if( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind != 0 ){	// �ް������łɊi�[�ς݂̏ꍇ
			if( Suica_Snd_Buf.read_wpt > 3 )						// ذ���߲����MAX�ȏ�H
				Suica_Snd_Buf.read_wpt = 0;							// ����̐擪���w�� 
			else
				Suica_Snd_Buf.read_wpt++;							// ذ���߲������UP			
		}

		Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind = wpkind;
		memcpy( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].Snd_Buf, wpsnddata,(size_t)S_BUF_MAXSIZE );

		if( Suica_Snd_Buf.write_wpt > 3 )
			Suica_Snd_Buf.write_wpt = 0;
		else
			Suica_Snd_Buf.write_wpt++;
	}

	return 0;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica Snd Data Set
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Suica_Data_Snd( uchar kind, void *snddata )
{
	long tmp = 0;
    ulong paydata = 0;

	if( prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1) 					/* suica�g�p�H */
		return;

	if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR ){	// ��M�������ϊz�Ǝ�M�������i�I���f�[�^�Ƃō��ق�������
		return;											// �ȍ~���ׂĂ̓d�����M�����ɔ�����
	}

	if( OPECTL.Ope_mod == 13 )										/* ̪��ނ��C�����Z�������ꍇ�d�����M���Ȃ� */
		return;														
	memset( suica_work_buf,0,sizeof(suica_work_buf) );
	Suica_Rec.snd_kind = kind;										/* ���M��ʂ̾�� */

	switch( kind ){
		case S_CNTL_DATA:											/* �����ް��쐬 */
			memcpy(&suica_work_buf,snddata,sizeof(suica_work_buf[0]));
			if( suica_work_buf[0] == 0x80 && Suica_Rec.Data.BIT.SEND_CTRL80 ){		// ���M�Ώۂ�����I��(0x80)�ő��M�\��ԂłȂ��ꍇ
				return;																// ���M���Ȃ��Ŕ�����
			}
Suica_Data_Snd_10:
			if( (suica_work_buf[0] & 0x01 ) == 0 ){
				Suica_Rec.Data.BIT.OPE_CTRL = 0;
				if( suica_work_buf[0] == 0 ){
					tmp = (unsigned char)prm_get(COM_PRM, S_SCA, 16, 2, 3);
					LagTim500ms( LAG500_SUICA_NONE_TIMER, (short)((tmp * 2) + 1), snd_ctrl_timeout3 );			// ��t�s�����ް����M��������ϊJ�n(5s)
					LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );	/* ��t�����ް����M���������ؾ�� */
				}
			}else if( (suica_work_buf[0] & 0x01 ) == 1 ){
				if( OPECTL.op_faz == 8 || OPECTL.InquiryFlg || 						// �d�q�}�̒�~�����ڼޯ�HOST�ʐM����
					OPECTL.ChkPassSyu != 0 || Suica_Rec.Data.BIT.MIRYO_TIMEOUT )			// ����߽��������Suica�����^�C���A�E�g
					return;
				Suica_Rec.Data.BIT.OPE_CTRL = 1;
				tmp = (unsigned char)prm_get(COM_PRM, S_SCA, 15, 2, 3);
				LagTim500ms( LAG500_SUICA_NO_RESPONSE_TIMER, (short)(tmp*2+1), snd_ctrl_timeout2 );	// ��t�����ް����M��������ϊJ�n(500ms)
				LagCan500ms( LAG500_SUICA_NONE_TIMER );								// ��t�s�����ް����M��������ϊJ�n(5s)
			}
			Ope_Suica_Status = 1;
			break;
		case S_SELECT_DATA:											/* ���i�I���ް��쐬 */
			// Suica��~�����̏ꍇ�́A���i�I���f�[�^�𑗐M������Suica��~���������s����
			if( Suica_Rec.Data.BIT.MIRYO_TIMEOUT || 				// �����^�C���A�E�g������͏��i�I���f�[�^�𑗐M���Ȃ�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				Edy_Rec.edy_status.BIT.CTRL_MIRYO || 				// Edy�̈����薢����
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				OPECTL.InquiryFlg ||												// �ڼޯĂ�HOST�₢���킹��
				OPECTL.ChkPassSyu != 0 ||							// ����₢���킹��
				OPECTL.op_faz == 8 ){								// �d�q�}�̒�~������
				Suica_Rec.snd_kind = S_CNTL_DATA;
				memset( suica_work_buf,0,sizeof(suica_work_buf) );
				goto Suica_Data_Snd_10;
			}
			else if( !Suica_Rec.Data.BIT.ADJUSTOR_START ){			// ���i�I���f�[�^���M�s���
				return;												// ���M�����ɔ�����
			}
			else if( Suica_Rec.Data.BIT.PRI_NG ) { 						// �W���[�i���v�����^�g�p�s��
				return;
			}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(Suica���ψُ픭��(E6961)��͌��ϕs�Ƃ���)
			else if( Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR ) {	// Suica���ψُ픭��?
				return;													// Suica���ψُ픭�����͑��M�����ɔ�����
			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(Suica���ψُ픭��(E6961)��͌��ϕs�Ƃ���)
			tmp = 10000;
			suica_work_buf[0] = 0x01;
			memcpy( &paydata,snddata,sizeof(snddata));
			suica_work_buf[6] = binbcd( (unsigned char)(paydata/tmp) );
			paydata %= tmp;
			tmp/=100;
			suica_work_buf[5] = binbcd( (unsigned char)(paydata/tmp) );
			paydata %= tmp;
			suica_work_buf[4] = binbcd( (unsigned char)paydata );
			Product_Select_Data = *((long*)snddata);
			break;
		case S_INDIVIDUAL_DATA:										/* ���ް��쐬 */
			suica_work_buf[0] = 0x00;
			suica_work_buf[1] = 0x06;
			suica_work_buf[2] = 0x00;
			if( !CPrmSS[S_SCA][10] )
				time_set_snd(&suica_work_buf[3],snddata);
			suica_work_buf[11] = 0xFE;
			suica_work_buf[22] = (uchar)OPE_SUICA_MIRYO_TIME;

			break;
		case S_PAY_DATA:											/* ���Z�ް��쐬 */
			if( !CPrmSS[S_SCA][10] )
				time_set_snd(suica_work_buf,snddata);
			break;
		default:
			break;
	}

	Suica_Snd_regist( &Suica_Rec.snd_kind, suica_work_buf );
	CNMTSK_START = 1;												/* �^�X�N�N�� */	
}

//[]----------------------------------------------------------------------[]
///	@brief			��M�ް����ނ��Aү���޷����o�^����
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/02/19<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void suica_save( unsigned char *buf, short siz )
{
	unsigned short 	work;
	unsigned char	*pt;
	unsigned char	status_timer;
// MH321800(S) G.So IC�N���W�b�g�Ή�
//	unsigned char	moni_wk[10];
// MH321800(E) G.So IC�N���W�b�g�Ή�
	
	pt = buf + 1;	/* �ް����̐擪���ڽ(��ʂ̎�) */
	memset( wrcvbuf,0,sizeof( wrcvbuf ));
	memcpy( wrcvbuf, pt, (size_t)siz );
	Suica_Rec.suica_err_event.BIT.COMFAIL = 0;


	switch( *buf ){														/* ��M�ް����� */
		case S_DSP_DATA:	/* �\���˗��f�[�^ */
			pay_dsp = settlement_amount( wrcvbuf );						/* ��M�ް��ϊ� */

			if( Suica_Rec.Data.BIT.INITIALIZE )							/* Suica�������I���H */
				Suica_Rec.suica_rcv_event.BIT.DSP_DATA = 1;				/* �\���˗��ް���M�׸�ON */

			break;
		case S_SETTLEMENT_DATA:	/* ���ό��ʃf�[�^ */
			if( wrcvbuf[0] != 0 ){
				if( wrcvbuf[0] != 0x01 && wrcvbuf[0] != 0x03 ){			/* ��M�ް���ʂ�0x01�E0x03�ȊO�H */
					err_chk( ERRMDL_SUICA, ERR_SUICA_PAY_NG, 2, 0, 0 ); /* �װ۸ޓo�^�i�o�^�j*/
					break;
				}	
				if(wrcvbuf[0] == 0x01){											// ����f�[�^
					memcpy( &moni_wk[0],&wrcvbuf[12],(size_t)2);					// ���ޔԍ��擾(��2�A��4��)
					memcpy( &moni_wk[2],&wrcvbuf[24],(size_t)4);
					work = (ushort)settlement_amount(&wrcvbuf[6]);				// ���ϑO�c�z
					memcpy(&moni_wk[6],(uchar*)(&work),sizeof(short));
					work = (ushort)settlement_amount(&wrcvbuf[9]);				// ���ό�c�z
					memcpy(&moni_wk[8],(uchar*)(&work),sizeof(short));
					wmonlg( OPMON_SUICA_SETTLEMENT, moni_wk, 0 );				// ���j�^�o�^
				}

				if( Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1 &&		 /* ���łɌ��ό��ʎ�M�ς݂� */
				    Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0 ){   /* �܂�Ope���������s���Ă��Ȃ��ꍇ or */
				    Settlement_rcv_faze_err( wrcvbuf, 2 );						 /* �G���[�o�^ */
					break;														 /* �Ȍ�̏��������Ȃ��̂Ŕ����� */
				}else if( Suica_Rec.Data.BIT.PAY_CTRL == 1 ){					 /* ���łɂP�x�A�d�q���ς��s���Ă���ꍇ */
				    Settlement_rcv_faze_err( wrcvbuf, 3 );						 /* �G���[�o�^ */
					break;														 /* �Ȍ�̏��������Ȃ��̂Ŕ����� */
				}

				/* ��M�ް���� */
				memset( &Settlement_Res,0,sizeof( Settlement_Res ));
				memcpy( &Settlement_Res.Result,&wrcvbuf[0],(size_t)3);
				Settlement_Res.settlement_data = settlement_amount(&wrcvbuf[3]);
				Settlement_Res.settlement_data_before = settlement_amount(&wrcvbuf[6]);
				Settlement_Res.settlement_data_after = settlement_amount(&wrcvbuf[9]);
				memcpy( &Settlement_Res.Suica_ID,&wrcvbuf[12],sizeof(Settlement_Res.Suica_ID));

				#if (3 == AUTO_PAYMENT_PROGRAM)										/* �����p�Ɍ��ϊz�����܂����ꍇ�iýėp�j*/
					if( CPrmSS[S_SYS][9] ){
						Settlement_Res.settlement_data = CPrmSS[S_SYS][9];
						if(( Settlement_Res.settlement_data_after - CPrmSS[S_SYS][9]) >= 0){
							Settlement_Res.settlement_data_after = Settlement_Res.settlement_data_before - CPrmSS[S_SYS][9];
						}else{
							Settlement_Res.settlement_data_after = 0;
						}
					}
				#endif
				if( Settlement_Res.Result == 1 && Product_Select_Data != Settlement_Res.settlement_data ){
					Suica_Ctrl( S_CNTL_DATA, 0x80 );								// �����ް��i����I���j�𑗐M����
					Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR = 1;					// ��M�������ϊz�Ƒ��M�������i�I���f�[�^�ɍ��ق�����
					memset( err_wk, 0, sizeof( err_wk ));
					intoasl( err_wk, (ulong)Product_Select_Data, 5 );				// ���ϊz���Z�b�g
					err_wk[5] = '-';												// ��؂蕶���Z�b�g
					err_wk[6] = '>';												// ��؂蕶���Z�b�g
					err_wk[12] = ':';												// ��؂蕶���Z�b�g
					intoasl( &err_wk[7], (ulong)Settlement_Res.settlement_data, 5 );// ���ϊz���Z�b�g
					memcpy( &err_wk[13], Settlement_Res.Suica_ID, sizeof( Settlement_Res.Suica_ID ) );							// ��ʂS�����}�X�N����
					err_chk2( ERRMDL_SUICA, ERR_SUICA_PAY_ERR, 1, 1, 0, err_wk );/* �װ۸ޓo�^�i�o�^�j*/
					Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;							// �����^�C���A�E�g�������������Ƃɂ���
					Suica_Rec.Data.BIT.ADJUSTOR_START = 0;							// ���i�I���f�[�^���M�ۃt���O�N���A
					dsp_change = 0;													// �����\���׸ނ�������
					queset( OPETCBNO, SUICA_PAY_DATA_ERR, 0, NULL );				// ���ψُ��ʒm
					return;															// ���Ϗ����͂��Ȃ��i�������Ƃ���j
				}

				if( Suica_Rec.Data.BIT.INITIALIZE )						/* Suica�������I���H */
					Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;	/* ���ό����ް���M�׸�ON */

				if( Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1 && 
					Settlement_Res.Result == 0x01){						/* ���ό����ް���M�׸�ON? */
					// �����׸ނ�ر�������ݸނ͐��Z۸ދy�ѐ��Z���~۸ނɌ��σf�[�^��o�^���邩�A
					// �����Z���̐��Z���J�n�����ꍇ�Ƃ���B
					Suica_Rec.Data.BIT.LOG_DATA_SET = 1;				/* ���ό����ް���M�ς� */

					// ���̃t���O�͐��Z���J�n����O�y�ь��ς��s��ꂽ�ꍇ�ɃN���A���A
					// �Z�b�g����^�C�~���O�͐��Z�J�n��A���ԗ��������������ꍇ�Ƃ���
					Suica_Rec.Data.BIT.ADJUSTOR_START = 0;						/* ���i�I���f�[�^���M�ۃt���O�N���A */
				}

#if (3 == AUTO_PAYMENT_PROGRAM)											// �����p�ɗ��p�J�[�h��ʂ����܂����ꍇ�iýėp�j
				work = (short)prm_get(COM_PRM, S_SYS, 6, 1, 1);			// 01-0006�E�ɃJ�[�h��ʂ��Z�b�g����

				/** 01-0006�E=7���́A�t�@���N�V�����L�[�������Ȃ���Suica�^�b�`�ŃJ�[�h���؂�ւ��� **/
				if( work == 7 ){
					if( SHORT_KEY1 == 1 ){								// F1�L�[������
						work = 1;										// Suica�ɓǂݑւ�
					}
					else if( SHORT_KEY2 == 1 ){							// F2�L�[������
						work = 2;										// PASMO�ɓǂݑւ�
					}
					else if( SHORT_KEY3 == 1 ){							// F3�L�[������
						work = 4;										// ICOCA�ɓǂݑւ�
					}
					else if( SHORT_KEY4 == 1 ){							// F4�L�[������
						work = 9;										// AMANO�ɓǂݑւ�
					}
				}

				//   �J�[�h�ԍ��̐擪2����������������B
				//   1=Suica("JE")�A2=PASMO("PB")�A4=ICOCA("JW")�A9=���̑�("AM")
				switch( work ){
				case	1:												// Suica�ɓǂݑւ�
					memcpy( &Settlement_Res.Suica_ID, "JE", 2 );
					break;
				case	2:												// PASMO�ɓǂݑւ�
					memcpy( &Settlement_Res.Suica_ID, "PB", 2 );
					break;
				case	4:												// ICOCA�ɓǂݑւ�
					memcpy( &Settlement_Res.Suica_ID, "JW", 2 );
					break;
				case	9:												// AMANO�ɓǂݑւ�
					memcpy( &Settlement_Res.Suica_ID, "AM", 2 );
					break;
				}
#endif

			}
			#if (3 == AUTO_PAYMENT_PROGRAM)											// �����p�Ɏ�M�f�[�^�����܂����ꍇ�iýėp�j
			else{
						if( SHORT_KEY2 == 1 ){							// F2�L�[������
							STATUS_DATA.status_data = 1;							// ��t�s����M�������̗p�ɐU����
							goto DEBUG_SUICA_SAVE;						// ����ް���M�����ֈړ�
						}
			}
			#endif
			
			break;
		case S_ERR_DATA: /* �ُ�f�[�^ */
			err_suica_chk( wrcvbuf ,&err_data ,ERR_SUICA_RECEIVE );	/* �װ���� */

			err_data = wrcvbuf[0];
			work = (short)prm_get(COM_PRM, S_SCA, 14, 2, 3);			/* SX-10��ԊĎ���ϒl�擾 */
			if( err_data ){											/* �װ�����H */
				Suica_Rec.suica_err_event.BIT.ERR_RECEIVE = 1;			/* �ُ��ް���M�׸�ON */

				if( work )												/* SX-10��ԊĎ���ϒl������l�H */
					LagTim500ms( LAG500_SUICA_STATUS_TIMER, (short)(120*work), snd_ctrl_timeout );	// SX-10��ԊĎ���ϊJ�n
			}else{														/* �װ���� */
				Suica_Rec.suica_err_event.BIT.ERR_RECEIVE = 0;			/* �ُ��ް���M�׸�OFF */
				LagCan500ms( LAG500_SUICA_STATUS_TIMER );				/* SX-10��ԊĎ����ؾ�� */
			}
			break;
		case S_STATUS_DATA: /* ��ԃf�[�^ */
			#if (3 == AUTO_PAYMENT_PROGRAM)									/* �����p�ɖ����^�C�}�[�����܂����iýėp�j*/
				if( CPrmSS[S_SYS][7] ){											// ����������̐ݒ薳�����삠��H
					if( Suica_Rec.Data.BIT.CTRL_MIRYO ){						// ����������
						CPrmSS[S_SYS][7]--;										// ��񐧌�f�[�^�����񐔂��f�N�������g
						break;													// ��M�f�[�^���������Ȃ�
					}
				}
			#endif
			STATUS_DATA.status_data = wrcvbuf[0];									/* ��M�ް��ð����� */

			#if (3 == AUTO_PAYMENT_PROGRAM)											// �����p�Ɏ�M�f�[�^�����܂����ꍇ�iýėp�j
				if( SHORT_KEY1 == 1 ){								// F1�L�[������
					if( STATUS_DATA.status_data == 0x03 ){						// ����L�����Z������t�s��
						STATUS_DATA.status_data = 0x02;							// ����L�����Z����t�ɕύX
					}
				}else if( SHORT_KEY2 == 1 ){						// F2�L�[������
					if( Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA ){ // ���ό��ʂ���M�ς�
						break;										// �������������Ȃ��Ŕ�����
					}
				}
DEBUG_SUICA_SAVE:
			#endif

			Suica_Rec.Data.BIT.CTRL = STATUS_DATA.StatusInfo.ReceptStatus ^ 0x01;	/* ��M�ް���1Byte�ڂ��Q�Ƃ��Aذ�ް��ԂƂ��Ď擾���� */
			if( STATUS_DATA.StatusInfo.MiryoStatus && 				/* ����Bit���`�F�b�N */
				!Suica_Rec.Data.BIT.CTRL_MIRYO ){					/* �܂��������������Ă��Ȃ� */
				Suica_Rec.Data.BIT.CTRL_MIRYO = 1;					/* Suica�����ð����� */
				read_sht_cls();										/* ���Cذ�ް������۰�� */
				cn_stat( 2, 2 );									/* �����s�� */
				work = (ushort)OPE_SUICA_MIRYO_TIME;							/* ������ײ��ϰ�l�擾 */

			#if (3 == AUTO_PAYMENT_PROGRAM)									/* �����p�ɖ����^�C�}�[�����܂����iýėp�j*/
				if( CPrmSS[S_SYS][6] ){
					work = (ushort)CPrmSS[S_SYS][6];
				}
			#endif
				LagTim500ms( LAG500_SUICA_MIRYO_RESET_TIMER, (short)(2*(work+5)), miryo_timeout );	// ������ײ��ԊĎ���ϊJ�n
				OPECTL.InquiryFlg = 1;
				err_chk( ERRMDL_SUICA, ERR_SUICA_MIRYO_START, 2, 0, 0 ); 	/* �װ۸ޓo�^�i�����������j*/
			}else{
				switch( STATUS_DATA.status_data & 0x03 ){					/* ��2Bit�̏�Ԃ��`�F�b�N */
					case 0:		/* ��t�� */
						if( !Ope_Suica_Status )					/* ���Z�@������̑��M�v���Ȃ� */
							Suica_Rec.Data.BIT.OPE_CTRL = 1;	/* �Ō�ɐ��Z�@������̑��M�v���͎�t�Ƃ��� */

						if( Suica_Rec.Data.BIT.OPE_CTRL ){		/* �Ō�ɐ��Z�@������̑��M�v���͎�t�H */
							LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );		/* ��t�����ް����M���������ؾ�� */
							err_chk( ERRMDL_SUICA, ERR_SUICA_OPEN, 0, 0, 0 ); 	/* �װ۸ޓo�^�i�����j*/
							Suica_Rec.suica_err_event.BIT.OPEN = 0;				/* �J�ǈُ��׸ނ����� */
							Status_Retry_Count_OK = 0;							/* ��ײ���Đ��������� */
						}
						else{									/* �Ō�ɐ��Z�@������̑��M�v���͎�t�s�H */
							status_timer = (unsigned char)prm_get(COM_PRM, S_SCA, 16, 2, 3);
							LagTim500ms( LAG500_SUICA_NONE_TIMER, (short)((status_timer * 2) + 1), snd_ctrl_timeout3 );			// ��t�s�����ް����M��������ϊJ�n(5s)
						}
						break;
					case 0x01:	/* ��t�s�� */
						if( !Ope_Suica_Status )					/* ���Z�@������̑��M�v���Ȃ� */
							Suica_Rec.Data.BIT.OPE_CTRL = 0;    /* �Ō�ɐ��Z�@������̑��M�v���͎�t�s�Ƃ��� */
																
						if( !Suica_Rec.Data.BIT.INITIALIZE )				/* Suica�������I���H */
							break;
						if( !Suica_Rec.Data.BIT.OPE_CTRL ){		/* �Ō�ɐ��Z�@������̑��M�v���͎�t�s�H */
							LagCan500ms( LAG500_SUICA_NONE_TIMER );				/* ��t�s�����ް����M���������ؾ�� */
							if( Suica_Rec.Data.BIT.CTRL_MIRYO &&			/* ������Ԃ����t�s����M�H���� */ 
								Suica_Rec.Data.BIT.INITIALIZE ){			/* ���������I�����Ă���ꍇ */ 
								LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );	/* ������ԊĎ����ؾ�� */
								Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* ������ԉ��� */
							}
							Status_Retry_Count_NG = 0;							/* ��ײ���Đ��������� */
						}else{
							status_timer = (unsigned char)prm_get(COM_PRM, S_SCA, 15, 2, 3);
							LagTim500ms( LAG500_SUICA_NO_RESPONSE_TIMER, (short)(status_timer*2+1), snd_ctrl_timeout2 );	// ��t�����ް����M��������ϊJ�n(500ms)
						}
						break;
					case 0x02:	/* ����L�����Z����t */
					case 0x03:	/* ��t�s������L�����Z����t */
						LagCan500ms( LAG500_SUICA_NONE_TIMER );				/* ��t�s�����ް����M���������ؾ�� */
						if( Suica_Rec.Data.BIT.CTRL_MIRYO ){				/* ������ԁH */
							Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* ������ԉ��� */
							LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );	/* ������ײ���ؾ�� */
							if( !SUICA_CM_BV_RD_STOP && OPECTL.op_faz != 3 ){ /* ������̓���͑S���Z�}�̂��~���Ȃ��ꍇ �����Z���~�������ł͂Ȃ� */
								miryo_timeout_after_proc();
							}
							err_chk( ERRMDL_SUICA, ERR_SUICA_MIRYO, 2, 0, 0 ); 	/* �װ۸ޓo�^�i�o�^�j*/
							OPECTL.InquiryFlg = 0;		
						}
						Status_Retry_Count_NG = 0;							/* ��ײ���Đ��������� */
						break;
					default:
						break;
				}
			}

			if( Suica_Rec.Data.BIT.INITIALIZE )							/* Suica�������I���H */
				Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;			/* �����ް���M�׸�ON */
				
			Ope_Suica_Status = 0;	
			break;
		case S_TIME_DATA: /* ���ԓ����f�[�^ */
			if( !Suica_Rec.Data.BIT.INITIALIZE ){						/* Suica�������I���H */
				break;

			}
			if( CPrmSS[S_SCA][10] ){ 									/* SX-10���玞�v�f�[�^�ł̐�������H */
				time_get_rcv(&time_data,wrcvbuf);						/* ��M�ް��̕ϊ����� */
				timset( &time_data );									/* �����ް��ϊ��������{ */
				time_snd_ope( &time_data );								/* Ope�Ɏ����ް���ėv�� */
			}

			break;
	    case S_FIX_DATA: /* �Œ�f�[�^ */
			memcpy( fix_data,wrcvbuf,sizeof(fix_data));					/* ��M�ް�����ʕ\���p�ɕێ� */
			break;

	    default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica Set time
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void time_set_snd( unsigned char *sndbuf, void *snddata )
{
	unsigned char year_data;	

	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->seco) );		// �b
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->minu) );		// ��
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->hour) );		// ��
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->week) );		// �T
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->date) );		// ��
	*(sndbuf++) = binbcd( (unsigned char)(((struct clk_rec*)snddata)->mont) );		// ��

	year_data = binbcd( (unsigned char)(((struct clk_rec*)snddata)->year%100) );	// �N�E���ʂQ��
	memcpy( sndbuf,&year_data,sizeof(year_data));
	year_data = binbcd( (unsigned char)(((struct clk_rec*)snddata)->year/100) );	// �N�E��ʂQ��
	sndbuf++;
	memcpy( sndbuf,&year_data,sizeof(year_data));
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica Rcv time
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void time_get_rcv(  struct clk_rec *sndbuf, void *snddata)
{
	unsigned char *rcv;
	ushort	year_date;
	rcv = snddata;

	memset(sndbuf,0,sizeof(sndbuf));

	sndbuf->seco = bcdbin( *rcv );		// �b
	sndbuf->minu = bcdbin( *(++rcv) );	// ��
	sndbuf->hour = bcdbin( *(++rcv) );	// ��
	sndbuf->week = bcdbin( *(++rcv) );	// �T
	sndbuf->date = bcdbin( *(++rcv) );	// ��
	sndbuf->mont = bcdbin( *(++rcv) );	// ��
	sndbuf->year = bcdbin( *(++rcv) );	// �N�E��ʂQ��
	year_date 	= bcdbin( *(++rcv) );

	sndbuf->year += year_date*100; 		// �N�E���ʂQ��	
}

//[]----------------------------------------------------------------------[]
///	@brief			time_snd_ope
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
static void time_snd_ope( struct clk_rec *sndbuf )
{
	unsigned char snd_time[11];
	union {
		unsigned long	ul;
		unsigned char	uc[4];
	} u_LifeTime;
	memset(snd_time,0,sizeof(snd_time));
	snd_time[0] = binbcd( (unsigned char)( sndbuf->year / 100 ));	/* �N(��Q��) */
	snd_time[1] = binbcd( (unsigned char)( sndbuf->year % 100 ));	/* �N(���Q��) */
	snd_time[2] = binbcd( sndbuf->mont );							/* �� */
	snd_time[3] = binbcd( sndbuf->date );							/* �� */
	snd_time[4] = binbcd( sndbuf->hour );							/* ���� */
	snd_time[5] = binbcd( sndbuf->minu );							/* �� */
	u_LifeTime.ul = LifeTim2msGet();
	snd_time[7] = u_LifeTime.uc[0];
	snd_time[8] = u_LifeTime.uc[1];
	snd_time[9] = u_LifeTime.uc[2];
	snd_time[10] = u_LifeTime.uc[3];
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Edy_Rec.edy_status.BIT.TIME_SYNC_REQ = 1;						// ���������f�[�^���M�v��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	queset( OPETCBNO, CLOCK_CHG, 11, snd_time );						/* Send message to opration task */
}
//[]----------------------------------------------------------------------[]
///	@brief			�\���˗��f�[�^�E���ό��ʃf�[�^�ϊ�
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 07/02/29<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
long settlement_amount( unsigned char *buf )
{
	unsigned char count;
	long	res=0;
	long	pay=0;
	long	ofset=1;

	for( ;ofset<100000; ){		/* ���z�𐔒l�ɕϊ����� */
		count = bcdbin( (unsigned char)(*buf & 0x0f) );
		pay = count;
		res += pay*ofset;
		count = bcdbin( (unsigned char)(*buf >> 4) );
		pay = count;
		res += pay*(ofset*10);
		ofset *= 100;
		buf++;
	}

	return (res);	
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_ctrl_timeout2
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/11<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void snd_ctrl_timeout2( void )
{
	uchar	Retry_count = 0;
	LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );			/* ��t�����ް����M���������ؾ�� */

	// �Ō�ɑ��M�����f�[�^����t�s�̏ꍇ�̓^�C���A�E�g���������Ȃ�
	if( Suica_Rec.Data.BIT.OPE_CTRL == 0 ){
		return;
	}

	Retry_count = (uchar)prm_get(COM_PRM, S_SCA, 15, 2, 1);	/* �ݒ肩����ײ���Đ����擾 */
	if( Status_Retry_Count_OK < Retry_count ){				/* ���̶݂��Đ����ݒ�l�ȉ��H */
		if( !Suica_Rec.Data.BIT.CTRL ){
			Suica_Ctrl( S_CNTL_DATA, 0x80 );								/* �����ް��i����I���j�𑗐M */
			Suica_Ctrl( S_CNTL_DATA, 0x01 );								/* �����ް��i��t�j�𑗐M */
			Status_Retry_Count_OK++;						/* ��ײ���Ă�ݸ���� */
			return;
		}
	}			

	Status_Retry_Count_OK = 0;								/* ��ײ���Ă�ر */
	if( Suica_Rec.Data.BIT.CTRL ){							/* Suica�̏�Ԃ���t�̏ꍇ */
		Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;		/* �����ް���M�׸�ON */
		STATUS_DATA.StatusInfo.ReceptStatus = 0;			/* �����ް����e�Ɏ�t�¾�� */
		RecvStatusData( &OPECTL.Ope_mod );					/* ����f�[�^��M�������{ */
	}
	else{
		Suica_Rec.suica_err_event.BIT.OPEN = 1;				/* �J�ǈُ��׸ނ�� */
		err_chk( ERRMDL_SUICA, ERR_SUICA_OPEN, 1, 0, 0 );	/* �װ۸ޓo�^�i�o�^�j*/
	}	
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_ctrl_timeout3
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/08/10<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void snd_ctrl_timeout3( void )
{
	uchar	Retry_count = 0;

	LagCan500ms( LAG500_SUICA_NONE_TIMER );					/* ��t�s�����ް����M���������ؾ�� */
	if(	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER == 1 && STATUS_DATA.StatusInfo.Running ){		// Suica���牞�����Ȃ��A�����^�C���A�E�g�����ꍇ
		// ���Z�@�̃^�C�}�[�ɂĖ����^�C���A�E�g�ɂȂ�����̎�t�s�̉��������s���̏ꍇ�͉i�v�Ɏ�t�s���J��Ԃ��B
		// �������A��t�s�̉��������s��BIT�������Ȃ��f�[�^����M����΁A�ʏ�̃��g���C�����Ɉڍs����B
		STATUS_DATA.StatusInfo.Running = 0;													// �s�̉����Ŏ��s������M���A�ēx��t�s�𑗐M����ꍇ�͎��s���׸ނ𗎂Ƃ�
		Status_Retry_Count_NG = 0;															// ���s���ŕԂ��Ă����ꍇ�ɂ̓��g���C�J�E���g���N���A����B
		Suica_Ctrl( S_CNTL_DATA, 0 );														/* �����ް��i��t�s�j�𑗐M */
		return;
	}

	// �Ō�ɑ��M�����f�[�^����t�̏ꍇ�̓^�C���A�E�g���������Ȃ�
	if( Suica_Rec.Data.BIT.OPE_CTRL == 1 ){
		return;
	}

	if( Suica_Rec.Data.BIT.CTRL ){								/* Suica����t��ԁH */
		Retry_count = (uchar)prm_get(COM_PRM, S_SCA, 16, 2, 1);	/* �ݒ肩����ײ���Đ����擾 */
		if( Status_Retry_Count_NG < Retry_count ){				/* ���̶݂��Đ����ݒ�l�ȉ��H */
			Suica_Ctrl( S_CNTL_DATA, 0 );									/* �����ް��i��t�s�j�𑗐M */
			Status_Retry_Count_NG++;							/* ��ײ���Ă�ݸ���� */
			return;
		}else{
			Suica_Rec.Data.BIT.CTRL = 0;
			No_Responce_Timeout();
		}			
	}else{
		No_Responce_Timeout();
	}
	Status_Retry_Count_NG = 0;									/* ��ײ���Ă�ر */
	
}

//[]----------------------------------------------------------------------[]
///	@brief			No_Responce_Timeout(�����^�C���A�E�g�����������̓���)
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/06/16<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void No_Responce_Timeout( void )
{
	if(	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER && Suica_Rec.Data.BIT.CTRL_MIRYO ){			/* ������� */
		Suica_Rec.Data.BIT.CTRL_MIRYO = 0;					/* ������ԉ��� */
	}
	Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;			/* �����ް���M�׸�ON */	
	STATUS_DATA.StatusInfo.ReceptStatus = 1;				/* �����ް����e�Ɏ�t�s�¾�� */
	if( Suica_Rec.Data.BIT.ADJUSTOR_NOW ){					/* ���ϒ��̎�t�s�����������ꍇ */
		STATUS_DATA.StatusInfo.TradeCansel = 1;				/* �����ݾ��׸ނ�ON */
	}
	RecvStatusData( &OPECTL.Ope_mod );						/* ����f�[�^��M�������{ */
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica_Log_regist
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/26<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Suica_Log_regist( unsigned char* logdata, ushort log_size, uchar kind )
{
	ushort  i,k,wks = 0;

	memset( &time_data, 0, sizeof( time_data ));
	memset( &suica_Log_wbuff, 0, sizeof( suica_Log_wbuff ));									// ۸��ް��ҏW�̈�̏�����

	if( !SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start ){		// ۸��ް�������������Ă���ꍇ
		SUICA_LOG_REC.log_time_old = CLK_REC.ndat;					// �ŌÂ̓��t�ް����ޯ�����
		Log_data_edit( &SUICA_LOG_REC, 1 );							// ���t�ް��̕ҏW
	}
	if( CLK_REC.ndat != suica_work_time.ndat && SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start ){	// ���t���X�V����Ă���ꍇ
		Log_data_edit( &SUICA_LOG_REC, 1 );														// ���t�ް��̕ҏW
	}
	if( CLK_REC.nmin != suica_work_time.nmin || !SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start ){	// ���Ԃ��X�V����Ă���ꍇ
		Log_data_edit( &SUICA_LOG_REC, 0 );														// �����ް��̕ҏW
	}

	Log_data_edit2( &SUICA_LOG_REC,kind );														// �ް�ͯ�ނ̕ҏW(���M�ySD�z�E��M�yRD�z)

	if( (SUICA_LOG_REC.log_wpt+(log_size*2)) > SUICA_LOG_MAXSIZE-1){							// �ҏW�ް��̻�������
		wks = SUICA_LOG_MAXSIZE-SUICA_LOG_REC.log_wpt;											// �ޯ̧�̍Ō�܂Ŋi�[�ł��黲�ނ��Z�o
		
		for( i=0,k=0; i<log_size; i++){															// �ʐM�ް��̕ҏW
			hxtoas ( &suica_Log_wbuff[k],*(logdata+i) );												// ͷ��ް��𕶎���ɕϊ�
			k+=2;																				
		}
		memcpy( &SUICA_LOG_REC.log_Buf[SUICA_LOG_REC.log_wpt], suica_Log_wbuff, (size_t)wks );			// �ޯ̧�̍Ō�ɏ������߂镪�̒ʐM�ް���������
		SUICA_LOG_REC.log_wpt=7;																// �������߲���̍X�V

		Log_data_edit3( &SUICA_LOG_REC, (ushort)(SUICA_LOG_REC.log_wpt+k-wks), SUICA_LOG_REC.log_wpt );	// ���t�X�V����

		if( (k-wks) != 0 ){
			memcpy( &SUICA_LOG_REC.log_Buf[7], &suica_Log_wbuff[wks], (size_t)(k-wks) );				// �c����ް���擪���珑����
			SUICA_LOG_REC.log_wpt += (k-wks);													// �������߲���̍X�V
		}
		SUICA_LOG_REC.Suica_log_event.BIT.write_flag=1;											// ���t�X�V�����׸ލX�V

	}else{
		if( SUICA_LOG_REC.Suica_log_event.BIT.write_flag ){										// ���t�X�V�����׸�ON
			Log_data_edit3( &SUICA_LOG_REC, (ushort)(log_size+SUICA_LOG_REC.log_wpt), SUICA_LOG_REC.log_wpt );	// ���t�X�V����
		}
		for( i=0; i<log_size; i++){
			hxtoas ( &((uchar)SUICA_LOG_REC.log_Buf[SUICA_LOG_REC.log_wpt]),*(logdata+i) );		// �ʐM�ް��̕ϊ��E������
			SUICA_LOG_REC.log_wpt+=2;															// �������߲���̍X�V
		}
	}

	if( !SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start )									// �������׸ނ��O�̏ꍇ
		SUICA_LOG_REC.Suica_log_event.BIT.log_Write_Start = 1;									// �������׸ނ̍X�V

	suica_work_time = CLK_REC;																		
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Log_data_edit
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Log_data_edit( struct  suica_log_rec *data_cuf, uchar kind )
{
	ushort wks;
	memset( &suica_Log_wbuff, 0, sizeof( suica_Log_wbuff ));

	if( (data_cuf->log_wpt+7) > SUICA_LOG_MAXSIZE-1 ){									// ���������ް���������
		if( kind )																		// ���t�ް��H
			sprintf( (char*)suica_Log_wbuff,"[%02d/%02d]",CLK_REC.mont,CLK_REC.date );				// ���t��ݒ�
		else																			// �����ް��H
			sprintf( (char*)suica_Log_wbuff,"[%02d:%02d]",CLK_REC.hour,CLK_REC.minu );				// ������ݒ�

		wks = SUICA_LOG_MAXSIZE-data_cuf->log_wpt;										// �������ް����ނ��Z�o
		memcpy( &data_cuf->log_Buf[data_cuf->log_wpt],suica_Log_wbuff,(size_t)wks);			// �ް��̏�����
		data_cuf->log_wpt = 7;															// �������߲���̍X�V
		Log_data_edit3( data_cuf, (ushort)(7+data_cuf->log_wpt), data_cuf->log_wpt );	// ���t�X�V����
		if( (7-wks) != 0){
			memcpy( &data_cuf->log_Buf[data_cuf->log_wpt],&suica_Log_wbuff[wks],(size_t)(7-wks));	// �c����ް���擪���珑����
			data_cuf->log_wpt += (7-wks);														// �������߲���̍X�V
		}

		data_cuf->Suica_log_event.BIT.write_flag=1;										// ���t�X�V�����׸ލX�V

	}else{
		if(	data_cuf->Suica_log_event.BIT.write_flag )									// ���t�X�V�����׸�ON
			Log_data_edit3( data_cuf, (ushort)(7+data_cuf->log_wpt), data_cuf->log_wpt );// ���t�X�V����

		if( kind )																		// ���t�ް��H
			sprintf( (char*)suica_Log_wbuff,"[%02d/%02d]",CLK_REC.mont,CLK_REC.date );				// ���t��ݒ�H
		else																			// �����ް��H
			sprintf( (char*)suica_Log_wbuff,"[%02d:%02d]",CLK_REC.hour,CLK_REC.minu );				// ������ݒ�H

		memcpy( &data_cuf->log_Buf[data_cuf->log_wpt], suica_Log_wbuff,7 );					// �ҏW�ް��̐ݒ�
		data_cuf->log_wpt += 7;															// �������߲���̍X�V
	}
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			Log_data_edit2
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Log_data_edit2( struct  suica_log_rec *data_cuf, uchar kind )
{
	ushort wks;
	memset( &suica_Log_wbuff, 0, sizeof( suica_Log_wbuff ));

	if( (data_cuf->log_wpt+4) > SUICA_LOG_MAXSIZE-1 ){									// ���������ް���������
		if( kind )																		// ���M�ް��H
		     sprintf( (char*)suica_Log_wbuff,"[SD]" );												// �ySD�z���
		else																			// ��M�ް��H
		     sprintf( (char*)suica_Log_wbuff,"[RD]" );												// �yRD�z���

		wks = SUICA_LOG_MAXSIZE-data_cuf->log_wpt;										// �������ް����ނ��Z�o
		memcpy( &data_cuf->log_Buf[data_cuf->log_wpt],suica_Log_wbuff,(size_t)wks);			// �ް��̏�����
		data_cuf->log_wpt = 7;															// �������߲���̍X�V
		Log_data_edit3( data_cuf, (ushort)(4+data_cuf->log_wpt), data_cuf->log_wpt );// ���t�X�V����
		if( (4-wks) != 0){
			memcpy( &data_cuf->log_Buf[data_cuf->log_wpt],&suica_Log_wbuff[wks],(size_t)(4-wks));	// �c����ް���擪���珑����
			data_cuf->log_wpt += (4-wks);													// �������߲���̍X�V
		}
		data_cuf->Suica_log_event.BIT.write_flag=1;										// ���t�X�V�����׸ލX�V
	}else{			
		if(	data_cuf->Suica_log_event.BIT.write_flag )									// ���t�X�V�����׸�ON
			Log_data_edit3( data_cuf, (ushort)(4+data_cuf->log_wpt), data_cuf->log_wpt );// ���t�X�V����

		if( kind )																		// ���M�ް��H
		     sprintf( (char*)suica_Log_wbuff,"[SD]" );												// �ySD�z���
		else																			// ��M�ް��H
		     sprintf( (char*)suica_Log_wbuff,"[RD]" );												// �yRD�z���
	
		memcpy( &data_cuf->log_Buf[data_cuf->log_wpt], suica_Log_wbuff,4 );					// �ҏW�ް��̾��

		data_cuf->log_wpt += 4;															// �������߲���̍X�V
	}
	return;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Log_data_edit3
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
///	@file			ope_suica_ctrl.c
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Log_data_edit3( struct  suica_log_rec *data_cuf, ushort log_size, ushort loopcount )
{
	ushort i,time_year,time_work,siz;
	uchar	time_data[2];

	if( (log_size-data_cuf->log_wpt) < 15 ){			// �����ݻ��ނ�15Byte�ȉ��̏ꍇ�́A���t�ް������r���[�ɏ㏑������A�ŌÓ��t��
		if( (siz = data_cuf->log_wpt+15) > SUICA_LOG_MAXSIZE-1 ){	// �X�V������Ȃ����Ԃ�������邽�߂Ɍ���Byte���𑝂₷
			siz = SUICA_LOG_MAXSIZE-1;
		}
	}
	else
		siz = log_size;									// �������ސ����
	
	
	for( i=siz;i>loopcount;i-- ){						
		if( data_cuf->log_Buf[i] == 0x5d ){				// �y]�z�ł��邩�H
			if( data_cuf->log_Buf[i-3] == 0x2f && data_cuf->log_Buf[i-6] == 0x5b){	// �y/�z�E�y[�z�ł��邩�H 
				time_year = CLK_REC.year;											// �N���擾
				time_data[0] = (uchar)astoin( &((uchar)data_cuf->log_Buf[i-5]),2 );	// �ޯ̧������t���擾(��)
				time_data[1] = (uchar)astoin( &((uchar)data_cuf->log_Buf[i-2]),2 );	// �ޯ̧������t���擾(��)

				if( data_cuf->log_time_old < (time_work = dnrmlzm( (short)time_year, (short)time_data[0], (short)time_data[1] ))){ // �Ō��ް����ǂ�������
					sprintf( (char*)suica_Log_wbuff,"[%02d/%02d]",time_data[0],time_data[1] );	// ���t�ް��쐬
					memcpy( data_cuf->log_Buf, suica_Log_wbuff, (size_t)7);				// �ޯ̧�ɐݒ�
					data_cuf->log_time_old = time_work;								// �ŌÓ��t�̍X�V
					break;
				}
			}
		}		
	}
	return;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Log_Count_search
//[]----------------------------------------------------------------------[]
///	@return			data_count  ���O��̑���M����
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short Log_Count_search( uchar	search_kind )
{
	ushort i;// ���O�e�ʑ����ɂ��ύX
	short data_count = 0;

	struct	suica_log_rec *wlogbuf;
	
	if( search_kind ){
		wlogbuf = &SUICA_LOG_REC_FOR_ERR;
	}else{
		wlogbuf = &SUICA_LOG_REC;
	}
	
	for( i=0; i<SUICA_LOG_MAXSIZE-4; i++ ){					// �ʐM۸ނ̌���
		if( strncmp( &wlogbuf->log_Buf[i], "[RD]", 4 ) == 0 || strncmp( &wlogbuf->log_Buf[i], "[SD]", 4 ) == 0){	// �yRD�z�E�ySD�z������
			data_count++;									// ���ı���
			i+=3;											// �����߲���̍X�V
		}
	}
	return (data_count);									// ���Đ���߂�	
}

//[]----------------------------------------------------------------------[]
///	@brief			miryo_timeout
//[]----------------------------------------------------------------------[]
///	@return			data_count  ���O��̑���M����
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/08/10<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void miryo_timeout( void )
{
	LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );		/* ������ײ���ؾ�� */

	OPECTL.InquiryFlg = 0;								// FT4800�ł͂����׸ނ͖������Ƃ����Ӗ������ł��g�p����̂ł����ł��Ƃ�
	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;				// Suica���牞�����Ȃ��A�����^�C���A�E�g�����ꍇ
	Suica_Ctrl( S_CNTL_DATA, 0 );						// �����ް��i��t�s�j�𑗐M
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_Suica_Event
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			:getmassege�ɂĎ擾����ID 
///					ope_faze	:��ٌ���̪��
///	@return			ret			:���Z�I������
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/11/27<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short Ope_Suica_Event( ushort msg, uchar ope_faze )
{
	short	ret = 0;
	uchar	i;

// MH321800(S) G.So IC�N���W�b�g�Ή�
	if (isEC_USE()) {
		return Ope_Ec_Event(msg, ope_faze);
	}
// MH321800(E) G.So IC�N���W�b�g�Ή�
	for( ;suica_rcv_que.count != 0; ){								// ��M�ς݂̃f�[�^��S�ď�������܂ŉ�
		if( Suica_Read_RcvQue() == 1 )								// ��M�҂����킹���̏ꍇ��
			continue;												// �㑱�̃f�[�^��M������ׁA��͏����͂����Ȃ�

		for( i=0; i < TBL_CNT(CommandTbl); i++){					// ��M�f�[�^�e�[�u���������s
			if( CommandTbl[i].Command == (Suica_Rec.suica_rcv_event.BYTE & CommandTbl[i].Command) ){	// ��M�����f�[�^�������\�ȓd���̏ꍇ
				if(( ret = CommandTbl[i].Func(&ope_faze)) != 0 )	// �d�����̉�͏��������s
					break;										
			}
		}
	}
	
	return ret;	
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_TimeOut_10
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze		:��ٌ���̪���
///	@param[in]		e_pram_set		:�d�q���ώg�p�ݒ�
///	@return			ret				:���Z�I������
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/11/27<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short Ope_TimeOut_10( uchar ope_faze, ushort  e_pram_set )
{
	short	ret=0;

	switch( ope_faze ){
		case	0:
				if( e_pram_set == 1 ){											// �d�q�Ȱ�g�p�\�ݒ�H
					op_mod01_dsp_sub();
					grachr( 6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]] );				// 6�s�ڂɉ�ʐؑ֗p�\��������
					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );		// ��ʐؑ֗p��ϰ�N��(Timer10)
				}

				break;
		case	2:
				if( OPECTL.op_faz == 8 )								// �ڼޯĎg�p�œd�q�}�̒�~���̏ꍇ�͏����������ɔ�����
					break;
				if( First_Pay_Dsp() ){ 									// ����\���i�����\���j
					if( !Ex_portFlag[EXPORT_CHGNEND] ){						   // �`�����p�ł��܂��\�����ޑK�s���łȂ����̏ꍇ
						dspCyclicMsgRewrite(4);
						Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
					}
					if( OPECTL.op_faz == 3 || OPECTL.op_faz == 9 ){		// ������������s���̏ꍇ
						if( Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL){	// Suica���L�����Ō�ɑ��M�����̂���t���̏ꍇ
							Suica_Ctrl( S_CNTL_DATA, 0 );												// Suica���p��s�ɂ���
						}
					}
				} else {												// Lagtimer10�̎g�p���= 1:Suica��~��̎�t���đ��MWait�I��
					if( OPECTL.op_faz == 3 ){							// ��ϰ���s���Ɏ��������݉������ꂽ�ꍇ�ͷ�ݾُ������s
						Suica_Ctrl( S_CNTL_DATA, 0x80 );													// �����ް��i����I���j�𑗐M����
						Op_Cansel_Wait_sub( 0 );											// Suica��~�ςݏ������s
						break;
					}							
					if( ryo_buf.zankin == 0 ){							// ���}�̂ɂ�鐸�Z�������͎c���s�����o��̎�t�𑗐M���Ȃ�
						break;
					}

					if( OPECTL.op_faz == 9 )								// �����������͎�t�𑗐M���Ȃ�
						break;
					
					if( !Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ){			// Suica���󂯕t���s��Ԋ��A�Ō�ɑ��M���Ă���̂���t�s�v��
						Suica_Ctrl( S_CNTL_DATA, 0x01 );													// Suica���p���ɂ���
					}
					if(suica_fusiku_flg == 0) {							// Suica�c���s���ōă^�b�`�҂����ͻ��د��\�����ĊJ���Ȃ�
						mode_Lagtim10 = 0;									// Lagtimer10�̎g�p��� 0:���p�\�}�̻��د��\���ĊJ
						Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ1000mswait
					}
				}

				break;

	}
	return	ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_TimeOut_11
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze		:��ٌ���̪���
///	@param[in]		e_pram_set		:�d�q���ώg�p�ݒ�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/11/27<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Ope_TimeOut_11( uchar ope_faze, ushort  e_pram_set )
{
	switch( ope_faze ){
		case	0:
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( Suica_Rec.Data.BIT.CTRL == 1 || Edy_Rec.edy_status.BIT.CTRL == 1 ){	// �c���\���p����
			if( Suica_Rec.Data.BIT.CTRL == 1 ){	// �c���\���p����
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				if( edy_dsp.BIT.suica_zangaku_dsp ){						// Suica�̎c�z�\����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					Edy_StopAndStart();										// EdyStop��LED���� 
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					edy_dsp.BIT.suica_zangaku_dsp = 0;						// Edy�c�z�\���׸�OFF
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				}else if( edy_dsp.BIT.edy_zangaku_dsp ){					// Edy�̎c�z�\����
//					break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				}
				op_mod01_dsp_sub();
				grachr( 6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]] );			// 6�s�ڂɉ�ʐؑ֗p�\��������
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( DspChangeTime[1] && (!Suica_Rec.suica_err_event.BYTE || // ��ϰ�l������ɾ�Ă���Ă��邩��SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
//										 ( !EDY_USE_ERR ) ))				// Edy���Z���p�\
				if( DspChangeTime[1] && (!Suica_Rec.suica_err_event.BYTE) ) // ��ϰ�l������ɾ�Ă���Ă��邩��SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );	// ��ʐؑ֗p��ϰ�N��(Timer10)
			}else{															// ����L�����Z����̐����ް����M
				if( !Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL){
					Suica_Ctrl( S_CNTL_DATA, 0x01 );												// Suica���p���ɂ���
				}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( DspChangeTime[1] && (!Suica_Rec.suica_err_event.BYTE || // ��ϰ�l������ɾ�Ă���Ă��邩��SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
//										 ( !EDY_USE_ERR ) ))				// Edy���Z���p�\
				if( DspChangeTime[1] && (!Suica_Rec.suica_err_event.BYTE) ) // ��ϰ�l������ɾ�Ă���Ă��邩��SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );	// ��ʐؑ֗p��ϰ�N��(Timer10)
			}	
			break;
		case	2:
			if(!Suica_Rec.Data.BIT.CTRL_MIRYO	&&						// ���Z������ԂŖ����ꍇ�� ����
				!Suica_Rec.Data.BIT.CTRL 		&&						// �����ް����s�@����
				OPECTL.op_faz != 3 && ryo_buf.zankin ) {				// ������{�^���������Ŗ����@���� �c�z������ꍇ
				if( OPECTL.op_faz == 9 )
					break;
				if( OPECTL.op_faz == 8 || OPECTL.op_faz == 10 )			// �ڼޯĂł̐��Z�J�n(Host�����҂�)��Edy/Suica��~�҂����킹����
					break;
				if( ope_faze == 22 ){
				}else{
					if( MifStat == MIF_WROTE_FAIL ){						// Mifare�����ݎ��s
						break;
					}
				}
				Suica_Ctrl( S_CNTL_DATA, 0x01 );													// Suica���p���ɂ���
				w_settlement = 0;										// Suica�c���s�����̐��Z�z�i�O���͎c���s���łȂ����Ӗ�����j
			}
			break;
	}
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_MiryoEventCheck
//[]----------------------------------------------------------------------[]
///	@param[in]		msg				:getmassege�ɂĎ擾����ID
///	@return			ret				: 0=�����p�� 1=ٰ�ߏ�����Continue����
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char Ope_MiryoEventCheck( ushort msg )
{
	char	ret=0;
	uchar	setflag = 0;
	ushort	msgdata = 0;
   uchar	i;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( Suica_Rec.Data.BIT.CTRL_MIRYO || Edy_Rec.edy_status.BIT.CTRL_MIRYO ) // Suica/Edy���Z������ԁi�^�b�`���얢�����j�̏ꍇ
	if( Suica_Rec.Data.BIT.CTRL_MIRYO ) // Suica���Z������ԁi�^�b�`���얢�����j�̏ꍇ
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	{
		switch( msg ){
			case COIN_EVT:									// ����Ă����ү�����Ă̏ꍇ/
				if( OPECTL.CN_QSIG == 1 || OPECTL.CN_QSIG == 5 ){
					msgdata = COIN_IN_EVT;
				// ���ү����略���o�����󂯂����ɁA���łɐ��Z�������Ă��邩�A���Z���~���ɂͲ���ĕێ�
				}else if( OPECTL.CN_QSIG == 7 && (!ryo_buf.zankin || OPECTL.op_faz == 3) ){		// �����o����
					msgdata = COIN_EN_EVT;
				}
				if( msgdata != 0 )							// �o�^����f�[�^�������
					setflag = 1;								// ����Ĕ����׸ނ��
				break;
			case NOTE_EVT:									// ����Ă�����ذ�ް����Ă̏ꍇ
				if( OPECTL.NT_QSIG == 1 ){
					msgdata = NOTE_IN_EVT;
					setflag = 1;							// ����Ĕ����׸ނ��
				}
				break;
			case TIMEOUT1:									// ����Ă�TIMEOUT����Ă̏ꍇ
			case TIMEOUT2:
				if( !ryo_buf.zankin || OPECTL.op_faz == 3 ){	// �c�z������(���Z����)���A���Z���~���̏ꍇ
					for( i=0; i < DELAY_MAX; i++ ){				// ������ޯ̧������
						if( nyukin_delay[i] == COIN_EN_EVT )	// ү���~����Ă��������ꍇ
							break;								// ���[�v�𔲂���
					}
					if( i != DELAY_MAX ){						// ү���~����Ă���H
						break;									// TIMEOUT�n�̲���Ă͓o�^���Ȃ�
					}
					msgdata = msg;
					setflag = 1;							// ����Ĕ����׸ނ��
				}
				break;
			case ARC_CR_E_EVT:
				if( WaitForTicektRemove ){
					if(	Suica_Rec.Data.BIT.CTRL_MIRYO ||								// Suica������
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						Edy_Rec.edy_status.BIT.CTRL_MIRYO || 							// Edy�̈����薢����
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
						OPECTL.InquiryFlg ||											// �ڼޯĂ�HOST�₢���킹��
						OPECTL.ChkPassSyu ){											// ����₢���킹��
						read_sht_cls();
					}
				} 
				break;
			default:
				break;	
		}
		if( setflag ){
			nyukin_delay[delay_count] = msgdata;			// ����ĕێ�
			delay_count++;									// ����ĕێ��������X�V
		}
		if( msg == ARC_CR_R_EVT ){
			ope_anm( AVM_CARD_ERR5 );			// Card Error Announce(�������̃J�[�h�͂���舵���ł��܂���)
			if( 1 != rd_tik )								// ���Ԍ��ۗ��Ȃ�
				opr_snd( 13 );								// ���Ԍ��ۗ��ʒu����ł��O�r�o
			else
				opr_snd( 2 );								// �O�r�o
		}
		switch( msg ){										// �����薢�����ł��󂯕t������ү����case�ȉ��ɒǋL
			case SUICA_EVT:									// Suica�֘Aү����	
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			case IBK_EDY_RCV:								// Edy�֘Aү����
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			case CAR_FURIKAE_OUT:							// �U�֐��Z���̐U�֌����o�ɂ����ʒm
			case IFMPAY_GENGAKU:							// ���Z���Z�d����M�ʒm
			case IFMPAY_FURIKAE:							// �U�֐��Z�d����M�ʒm
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
			case EC_EVT_DEEMED_SETTLEMENT:					// �݂Ȃ����σg���K���� 
			case AUTO_CANCEL:								// �������̎����Ƃ肯������(���σ��[�_��薢���c�����~��M)
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
			case EC_INQUIRY_WAIT_TIMEOUT:									// �⍇��(������)�҂��^�C���A�E�g
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�	
				break;
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
			case COIN_RJ_EVT:								// ���Z���~(���o�[)�ɂ��[���]�C�x���g
			case KEY_TEN_F4:								// �ݷ�[���] �����
				// �����m�莞�͎���C�x���g�𔭐�������
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//				if(Suica_Rec.Data.BIT.MIRYO_CONFIRM){
//					ret = 0;
				if(isEC_CONF_MIRYO()){						// �����m��(��ԃf�[�^)��M�ς�
					Suica_Ctrl( S_CNTL_DATA, 0 );			// Suica���p��s�ɂ���	
					ret = 1;
// MH810103 GG119202(E) �����d�l�ύX�Ή�
				}
				// �����m��O�͎���C�x���g�𔭐������Ȃ�
				else {
					ret = 1;
				}
				break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�	
			default:
				ret = 1;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
	}

	if( msg == ARC_CR_R_EVT ){												// ����ذ�޲���Ă̏ꍇ
		i = 0;
		if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP && SUICA_CM_BV_RD_STOP ){		// �����^�C���A�E�g��̎g�p�s�ݒ莞
			i = 1;															// �}�����ꂽ�J�[�h��ԋp����
		}
		if( i == 1 ){
			ope_anm( AVM_CARD_ERR1 );			// Card Error Announce(���̃J�[�h�͎g���܂���)
			if( 1 != rd_tik )												// ���Ԍ��ۗ��Ȃ�
				opr_snd( 13 );												// ���Ԍ��ۗ��ʒu����ł��O�r�o
			else
				opr_snd( 2 );												// �O�r�o
			ret = 1;
		}
	}

	return	ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_ArmClearCheck
//[]----------------------------------------------------------------------[]
///	@param[in]		msg				:getmassege�ɂĎ擾����ID
///	@return			ret				: 0=�����p�� 1=ٰ�ߏ�����Continue����
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char Ope_ArmClearCheck( ushort msg )
{
	char	ret=0;
	char	exec=0;
// MH321800(S) G.So IC�N���W�b�g�Ή�
	if (isEC_USE() != 0) {
		return	Ope_EcArmClearCheck(msg);
	}
// MH321800(E) G.So IC�N���W�b�g�Ή�
	switch( msg ){												// Suica�c���s���̔��]ү���ޕ\������������
		case SUICA_EVT:											// Suica(Sx-10)����̎�M�ް�
			// ���ό����ް��ȊO���ް���M���A���ό��ʎ�M�Ŏc�z�s���̏ꍇ���A�����ް���M�̏ꍇ�͔�����
			if(Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA != 1 || Settlement_Res.Result == 0x03 || 
			   Suica_Rec.suica_rcv_event.BIT.STATUS_DATA ) {
				break;
			}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case IBK_EDY_RCV:										// EM�̎�M�ް�
//			// ���Z���ʒʒm�ȊO���ް���M���A�������{�����AEdyذ�ް��Ԃ��s��Ԃ̏ꍇ�͔�����
//			if(Edy_Rec.rcv_kind != R_SUBTRACTION || Edy_Rec.edy_status.BIT.CTRL_MIRYO || 
//			   !Edy_Rec.edy_status.BIT.CTRL ) {					
//				break;
//			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		case COIN_EVT:											// Coin Mech event
		case NOTE_EVT:											// Note Reader event
			if(( COIN_EVT == msg && !( OPECTL.CN_QSIG == 1 || OPECTL.CN_QSIG == 5 )) ||
   			   ( NOTE_EVT == msg && !( OPECTL.NT_QSIG == 1 || OPECTL.NT_QSIG == 5 )))
   			   	break;
		case ARC_CR_R_EVT:										// ����IN
		case KEY_TEN_F4:										// ������� ON
		case ELE_EVENT_CANSEL:									// �I���C�x���g�i���Z���~�j
		case ELE_EVT_STOP:										// ���Z�����C�x���g�i�d�q�}�̂̒�~�j
			if( msg == ELE_EVT_STOP && CCT_Cansel_Status.BIT.STOP_REASON != REASON_PAY_END )	// ���Z�������ȊO�͔�����
				break;
			if( msg == KEY_TEN_CL && OPECTL.CAN_SW == 1 )		// ������݂̍ĉ����̏ꍇ
				ret = 1;										// ��������������Ȃ��i����Ĕj���j

			if( OPECTL.InquiryFlg )			// �e�@�₢���킹�����N���W�b�g�₢���킹��
				break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			// Suica�c�z�s���\���� �� Edy�̎c�z�s���\���� �� Edy�̖�����ѱ�Č��ү���ޕ\���� �� Suica�̖�����ѱ�Č��ү���ޕ\����
//			if( dsp_fusoku ||
//				edy_dsp.BIT.edy_dsp_Warning ||
//				edy_dsp.BIT.edy_Miryo_Loss ){
			// Suica�c�z�s���\���� �� Suica�̖�����ѱ�Č��ү���ޕ\����
			if( dsp_fusoku ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				exec = 1;
			}else if( Suica_Rec.Data.BIT.MIRYO_ARM_DISP ){
				if(!SUICA_CM_BV_RD_STOP || msg == KEY_TEN_CL || (OPECTL.op_faz == 3 || OPECTL.op_faz == 9))
					exec = 1;
			}else if( DspSts == LCD_WMSG_ON ){
				exec = 1;
			}
			if( exec ){
			    LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );// �c�z�s���\�������p����ϰ��ݾ�
				op_SuicaFusokuOff();							// ���ݕ\������ү���ނ�����
				dsp_change = 0;
			}
	}

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_EleUseDsp
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Ope_EleUseDsp( void )
{
	e_incnt = 0;															// ���Z���́u�d�q�}�l�[�v�g�p�񐔁i�܂ޒ��~�j���N���A
	Suica_Ctrl( S_CNTL_DATA, 0x01 );												// Suica���p���ɂ���
	Suica_Rec.Data.BIT.PAY_CTRL = 0;										// Suica���Z�׸ނ�ؾ��
	DspChangeTime[0] = (ushort)CPrmSS[S_SCA][6];							// �c���\�����Ծ��

	DspChangeTime[1] = 2; 													// ��ʐ؂�ւ����Ծ��

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( Dsp_Prm_Setting != 10 )
//		Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );					// ��ʐؑ֗p��ϰ�N��(Timer10)
//	else{																	// 07-01-22�ǉ�
//		Edy_StopAndLedOff();												// ���ތ��m��~��LEDOFF�w�����M 07-01-22�ǉ�
//	}					
	Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[1]*50) );					// ��ʐؑ֗p��ϰ�N��(Timer10)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
}

//[]----------------------------------------------------------------------[]
///	@brief			���p�\�}�̂̕\���ؑ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	op_mod01_dsp_sub( void )
{
	disp_media_flg = 0;													// ��ʕ\���p�@���Z�}�̎g�p��(0)��(1)																		
	wk_media_Type = Ope_Disp_Media_Getsub(0);							// ��ʕ\���p�@���Z�}�̎�� �擾
	switch(wk_media_Type) {												// �u�c���Ɖ�i�^�s�j�v�pMsgNo��ܰ��ر�ɕێ�
		case OPE_DISP_MEDIA_TYPE_SUICA:
			if( Suica_Rec.suica_err_event.BYTE == 0 ){					// SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
				DspWorkerea[3] = 97;									// "�r���������̎c�z�Ɖ�ł��܂�"
			} else {													// SX-10�֘A�̴װ���������Ă���ꍇ
				DspWorkerea[3] = 96;									// "�����r���������͗��p�ł��܂���"
				disp_media_flg = 1;
			}
			break;

		case OPE_DISP_MEDIA_TYPE_PASMO:
			if( Suica_Rec.suica_err_event.BYTE == 0 ){					// SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
				DspWorkerea[3] = 117;									// "�o�`�r�l�n�̎c�z�Ɖ�ł��܂�"
			} else {													// SX-10�֘A�̴װ���������Ă���ꍇ
				DspWorkerea[3] = 101;									// "�����o�`�r�l�n�͗��p�ł��܂���"
				disp_media_flg = 1;
			}
			break;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case OPE_DISP_MEDIA_TYPE_EDY:
//			if( Dsp_Prm_Setting == 10 && !Edy_Rec.edy_status.BIT.ZAN_SW ){
//				DspWorkerea[3] = 0;
//				break;
//			}
//
//			if( !EDY_USE_ERR ){											// Edy���Z���p�\
//				DspWorkerea[3] = 98;									// "�d�����̎c���Ɖ�ł��܂�    "
//			} else {													// Edy�֘A�̴װ���������Ă���ꍇ
//				DspWorkerea[3] = 102;									// "�����d�����͗��p�ł��܂���    "
//				disp_media_flg = 1;
//			}
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		case OPE_DISP_MEDIA_TYPE_ICCARD:
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( Suica_Rec.suica_err_event.BYTE &&						// SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
//				EDY_USE_ERR ){											// Edy���Z���p�s�\
			if( Suica_Rec.suica_err_event.BYTE ){						// SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				DspWorkerea[3] = 103;									// �����h�b�J�[�h�͗��p�ł��܂���
				disp_media_flg = 1;
			}else{
				DspWorkerea[3] = 118;									// "�h�b�J�[�h�̎c�z�Ɖ�ł��܂�"
			}
			break;
		case OPE_DISP_MEDIA_TYPE_ICOCA:
			if( Suica_Rec.suica_err_event.BYTE == 0 ){					// SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
				DspWorkerea[3] = 124;									// "�h�b�n�b�`�̎c�z�Ɖ�ł��܂�"
			} else {													// SX-10�֘A�̴װ���������Ă���ꍇ
				DspWorkerea[3] = 123;									// "�����h�b�n�b�`�͗��p�ł��܂���"
				disp_media_flg = 1;
			}
			break;
		case OPE_DISP_MEDIA_TYPE_eMONEY:
			if( Suica_Rec.suica_err_event.BYTE == 0 ){					// SX-10�֘A�̴װ���������Ă��Ȃ��ꍇ
				DspWorkerea[3] = 130;									// "�d�q�}�l�[�̎c�z�Ɖ�ł��܂�"
			} else {													// SX-10�֘A�̴װ���������Ă���ꍇ
				DspWorkerea[3] = 129;									// "�����d�q�}�l�[�͗��p�ł��܂���"
				disp_media_flg = 1;
			}
			break;

// MH321800(S) hosoda IC�N���W�b�g�Ή�
		case OPE_DISP_MEDIA_TYPE_EC:
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//			if( isEcEnabled(EC_CHECK_EMONEY) ) {						// �d�q�}�l�[���L��
//				if( isEcReady(EC_CHECK_EMONEY) ) {						// �d�q�}�l�[�Ō��ω\
//					DspWorkerea[3] = 130;								// "�d�q�}�l�[�̎c�z�Ɖ�ł��܂�"
//				} else {												// �d�q�}�l�[�Ō��ϕs��
//					DspWorkerea[3] = 129;								// "�����d�q�}�l�[�͗��p�ł��܂���"
//					disp_media_flg = 1;
//				}
//			}
//			else {
//				DspWorkerea[3] = 0;										// �c���Ɖ���\���͍s��Ȃ�
//			}
// MH810103 GG119202(S) �J�ǁE�L�������ύX
//			if( isEcEmoneyEnabled(1, 0) ){								// �d�q�}�l�[���ω\
			{
// MH810103 GG119202(E) �J�ǁE�L�������ύX
				if( isEcEmoneyEnabled(1, 1) ){							// �d�q�}�l�[���ρE�c���Ɖ�\
// MH810103 GG119202(S) �ڋq��ʂ̓d�q�}�l�[�Ή�
//					DspWorkerea[3] = 130;								// "�d�q�}�l�[�̎c�z�Ɖ�ł��܂�"
					DspWorkerea[3] = 162;								// "�d�q�}�l�[�̎c���Ɖ�ł��܂�"
// MH810103 GG119202(E) �ڋq��ʂ̓d�q�}�l�[�Ή�
				}
				else {
					DspWorkerea[3] = 0;									// �c���Ɖ���\���͍s��Ȃ�
				}
			}
// MH810103 GG119202(S) �J�ǁE�L�������ύX
//			else {
//// GG119202(S) �ڋq��ʂ̓d�q�}�l�[�Ή�
////				DspWorkerea[3] = 129;									// "�����d�q�}�l�[�͗��p�ł��܂���"
//				DspWorkerea[3] = 0;										// �c���Ɖ���\���͍s��Ȃ�
//// GG119202(E) �ڋq��ʂ̓d�q�}�l�[�Ή�
//				disp_media_flg = 1;
//			}
// MH810103 GG119202(E) �J�ǁE�L�������ύX
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
			break;
// MH321800(E) hosoda IC�N���W�b�g�Ή�

		default:
			DspWorkerea[3] = 0;											// "							 "
			break;
	}	
}

//[]----------------------------------------------------------------------[]
///	@brief			���p�\�}�̂̕\��
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 06/07/28<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	op_mod01_dsp( void )
{
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Dsp_Prm_Setting = (uchar)prm_get( COM_PRM, S_PAY, 24, 2 ,3 );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	op_mod01_dsp_sub();
	grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]]);		// 6�s�ڂɉ�ʐؑ֗p�\��������
// MH321800(S) G.So IC�N���W�b�g�Ή�
//	if( !EDY_USE_ERR && Dsp_Prm_Setting == 11 ){
//		if( Edy_Rec.edy_status.BIT.CTRL )
//			Edy_SndData04();											// ���ތ��m��~�w�����M	 						
//		Edy_SndData01();												// ���ތ��m�w�����M	 
//	}
// MH321800(E) G.So IC�N���W�b�g�Ή�

	edy_dsp.BYTE = 0;															// Edy��ʕ\���n�̈�̏�����
	Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 0;										// Suica��ʕ\���׸ޏ�����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//	Elec_Data_Initialize();														// �d�q���ώg�p�̈�̏�����
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Op_StopModuleWait
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0�F�d�q�}�̒�~�J�n 1�F�d�q�}�̖���
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 07/10/12<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Op_StopModuleWait( unsigned char stop_kind )
{

// MH321800(S) G.So IC�N���W�b�g�Ή�
// ��~�҂��̊Ԃɑ��̗v������ēx��~���v�����ꂽ�Ƃ��̂��߂̕ی�
	if (OPECTL.op_faz == 8) {
	// ���ɒ�~���Ȃ�Ȃɂ����Ȃ�
		if (stop_kind == REASON_PAY_END) {
		// ��~���R�����Z�����͗D�悳����
			CCT_Cansel_Status.BIT.STOP_REASON = stop_kind;				// ��~���R(�v��)���Z�b�g
		}
		return 0;
	}
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	if (isEC_USE()) {
	// ���σ��[�_�[�́A���[�_�[�Ō��ς��Ă��Ȃ���Ύ~�߂Ȃ�
		if (OPECTL.Ope_mod == 2 &&										// ���Z����
			(Suica_Rec.Data.BIT.ADJUSTOR_NOW != 0 ||					// �I�����i�f�[�^���M�ς݁@��
			 Suica_Rec.Data.BIT.SELECT_SND != 0 ||						// �I�����i�f�[�^���M���@��
			 stop_kind == REASON_PAY_END)) {							// ��~���R�����Z����
			 ;															// ���[�_�[��~��������
		}
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
		else if (stop_kind == REASON_RTM_REMOTE_PAY) {
			// ���u���Z�J�n���͕K����~����
		}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
		else {
			return 1;
		}
	}
// MH810103 2GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( (Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR) || 				// Suica/Edy���L���̏ꍇ
//	    (Edy_Rec.edy_status.BIT.CTRL && !EDY_USE_ERR)){	
	if( (Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR) ){				// Suica���L���̏ꍇ
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

		if( (Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR) ){
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//			Suica_Ctrl( S_CNTL_DATA, 0 );										// Suica���p��s�ɂ���
			if (isEC_USE()) {
// MH810103 GG119202(S) ���Z�����������z�ύX�r�b�g���Z�b�g����
//				if (Suica_Rec.Data.BIT.ADJUSTOR_START != 0 &&			// ���Z�J�n�ς�
//					stop_kind != REASON_PAY_END) {						// ��~���R�����Z�����ȊO
//					// ���Z�����ɂ���t�֎~�ȊO�̏ꍇ�͋��z�ύX�r�b�g���Z�b�g����
//					Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );	// ��t�֎~���M(���z�ύX)
//				}
//				else {
//					Suica_Ctrl( S_CNTL_DATA, 0 );						// ��t�֎~���M
//				}
				Suica_Ctrl( S_CNTL_DATA, S_CNTL_PRICE_CHANGE_BIT );		// ��t�֎~���M(���z�ύX)
// MH810103 GG119202(E) ���Z�����������z�ύX�r�b�g���Z�b�g����
			}
			else {
				Suica_Ctrl( S_CNTL_DATA, 0 );							// ��t�֎~���M
			}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
		}
		else{														// ��~�v������Suica�����łɒ�~�ς݂̏ꍇ
			CCT_Cansel_Status.BIT.SUICA_END = 1;
			if( SUICA_USE_ERR ){									// Suica�n�̃G���[���������Ă���ꍇ
				Suica_Rec.Data.BIT.CTRL = 0;						// ��t�s��Ԃɂ��Ă���
			}
		}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		if( (Edy_Rec.edy_status.BIT.CTRL && !EDY_USE_ERR) ){		// Edy����t��ԂŁAEdy�֘A�̴װ���Ȃ��ꍇ
//			Edy_StopAndLedOff();									// ���ތ��m��~��UI LED�����w�����M				
//		}else{														// ��~�v������Edy�����łɒ�~�ς݂̏ꍇ
//			CCT_Cansel_Status.BIT.EDY_END = 1;						// Edy��~�ς��׸ނ��
//			if( EDY_USE_ERR ){										// Edy�̃G���[���������Ă���ꍇ
//				Edy_Rec.edy_status.BIT.CTRL = 0;					// ��t�s��Ԃɂ��Ă���
//			}
//		}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) G.So IC�N���W�b�g�Ή�
//		Lagcan( OPETCBNO, 10 );										// ��ϰ10ؾ��(���ڰ��ݐ���p)
		if(isEC_USE()) {
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );				// ���p�\�}�̻��د��\���p��ϰSTOP
		} else {
			Lagcan( OPETCBNO, 10 );									// ��ϰ10ؾ��(���ڰ��ݐ���p)
		}
// MH321800(E) G.So IC�N���W�b�g�Ή�
		CCT_Cansel_Status.BIT.INITIALIZE = 1;						// �d�q�}�̒�~�J�n
		CCT_Cansel_Status.BIT.STOP_REASON = stop_kind;				// ��~���R(�v��)���Z�b�g
		save_op_faz = OPECTL.op_faz;								// ���݂̃t�F�[�Y��ێ�
		OPECTL.op_faz = 8;											// �d�q�}�̒�~�t�F�[�Y���Z�b�g

		if( stop_kind != REASON_MIF_WRITE_LOSS ){					// Mifare�����ݎ��s����Mifare���~�����Ȃ�
			if( MIFARE_CARD_DoesUse ){									// Mifare���L���ȏꍇ
				op_MifareStop_with_LED();								// Mifare����
			}
		}
		
		LagCan500ms(LAG500_MIF_LED_ONOFF);
		Op_StopModuleWait_sub( 0xff );								// �S�f�o�C�X�̏I�����`�F�b�N����
		return 0;													// ��~�����J�n
	}

	if( stop_kind == REASON_PAY_END && !CCT_Cansel_Status.BIT.INITIALIZE ){		// ���Z�������ɂ͊�{�I�ɓd�q�}�͕̂s�ɂȂ��Ă���̂ł����ŏ�������
		CCT_Cansel_Status.BIT.INITIALIZE = 1;						// �d�q�}�̒�~�J�n
		CCT_Cansel_Status.BIT.STOP_REASON = stop_kind;				// ��~���R(�v��)���Z�b�g
		save_op_faz = OPECTL.op_faz;								// ���݂̃t�F�[�Y��ێ�
		OPECTL.op_faz = 8;											// �d�q�}�̒�~�t�F�[�Y���Z�b�g
		if( !Suica_Rec.Data.BIT.CTRL ){								// ��~�v������Suica�����łɒ�~�ς݂̏ꍇ
			CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica��~�ς��׸ނ��						
		}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		if( !Edy_Rec.edy_status.BIT.CTRL ){							// ��~�v������Edy�����łɒ�~�ς݂̏ꍇ
//			CCT_Cansel_Status.BIT.EDY_END = 1;						// Edy��~�ς��׸ނ��
//		}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		Op_StopModuleWait_sub( 0xff );								// �S�f�o�C�X�̏I�����`�F�b�N����
		return 0;													// ��~�����J�n		
	}
	return 1;						
}

#define STOP_BIT (32*CCT_Cansel_Status.BIT.STOP_REASON)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define Suica_STOP_BYTE 9
//#define EDY_STOP_BYTE	5
//
//#define Suica_CMN_STOP_BYTE 11
//#define EDY_CMN_STOP_BYTE 7
//#define Suica_EDY_STOP_BYTE 13
//#define ALL_STOP_BYTE 15
#define CMN_STOP_BYTE	0x03
#define Suica_STOP_BYTE	0x05
#define ALL_STOP_BYTE	0x07
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//[]----------------------------------------------------------------------[]
///	@brief			Op_StopModuleWait_sub
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret�F0�F��~�҂����킹�� 1�F��~����
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 07/10/12<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Op_StopModuleWait_sub( uchar kind )
{
	uchar ret = 0;
	
	switch( kind ){
		case 0:														// Suica�C�x���g
			if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// ���Z�������̓d�q�}�̒�~�҂����킹
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_CMN_STOP_BYTE ){	// ���ł�Edy�ƺ��ү�����~�ς݂̏ꍇ
//					ret = 1;											// 
//				}else{													// Edy�����ү����܂���~���Ă��Ȃ��ꍇ
//					if( EDY_USE_ERR ){
//						CCT_Cansel_Status.BIT.EDY_END = 1;				// Edy��~�ς��׸ނ��
//						if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_CMN_STOP_BYTE ){	// Edy�ݒ�Ȃ���Edy�֘A�װ���������ź��ү�����~�ς�
//							ret = 1;											// 
//						}
//					}
//				}
				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == CMN_STOP_BYTE ){	// ���łɺ��ү�����~�ς݂̏ꍇ
					ret = 1;
				}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica��~�ς��׸ނ��
				break;
			}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_STOP_BYTE || EDY_USE_ERR ){	// ���ł�Edy����~�ς݂�Edy�֘A�װ���������̏ꍇ
//				ret = 1;											// 
//				CCT_Cansel_Status.BIT.EDY_END = 1;					// Edy��~�ς��׸ނ��
//			}
			ret = 1;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica��~�ς��׸ނ��
			break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case 1:														// Edy�C�x���g
//			if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// ���Z�������̓d�q�}�̒�~�҂����킹
//				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == EDY_CMN_STOP_BYTE ){	// ���ł�Suica�ƺ��ү�����~�ς݂̏ꍇ
//					ret = 1;											// 
//				}else{													// Suica�����ү����܂���~���Ă��Ȃ��ꍇ
//					if( SUICA_USE_ERR ){
//						CCT_Cansel_Status.BIT.SUICA_END = 1;				// Suica��~�ς��׸ނ��
//						if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_CMN_STOP_BYTE ){	// Suica�ݒ�Ȃ���Suica�֘A�װ���������ź��ү�����~�ς�
//							ret = 1;											// 
//						}
//					}
//				}
//				CCT_Cansel_Status.BIT.EDY_END = 1;					// Edy��~�ς��׸ނ��
//				break;
//			}
//			if( (CCT_Cansel_Status.BYTE-STOP_BIT) == EDY_STOP_BYTE || SUICA_USE_ERR ){		// Suica����~�ς݂�Suica�֘A�װ��������
//				ret = 1;
//				CCT_Cansel_Status.BIT.SUICA_END = 1;				// Suica��~�ς��׸ނ��
//			}
//			CCT_Cansel_Status.BIT.EDY_END = 1;						// Edy��~�ς��׸޾��
//			break;
//		case 2:														// �d�q�}�̂̍Ċ����v��
//			if( !EDY_USE_ERR && !Edy_Rec.edy_status.BIT.CTRL ){					// Edy�֘A�̃G���[����������Edy��~��Ԃ̏ꍇ
//				Edy_SndData01();								// Edy���p�����i�ݒ肠��΁j
//			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		case 2:														// �d�q�}�̂̍Ċ����v��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( !EDY_USE_ERR && !Edy_Rec.edy_status.BIT.CTRL ){					// Edy�֘A�̃G���[����������Edy��~��Ԃ̏ꍇ
//				Edy_SndData01();								// Edy���p�����i�ݒ肠��΁j
//			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			if( !SUICA_USE_ERR && !Suica_Rec.Data.BIT.CTRL ){					// Suica�֘A�̃G���[����������Suica��~��Ԃ̏ꍇ
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
				if (isEC_USE()) {
					EcSendCtrlEnableData();							// ��t�����M
				}
				else {
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
				Suica_Ctrl( S_CNTL_DATA, 0x01 );									// Suica���p���ɂ���
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
				}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
			}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//			Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ
			if(isEC_USE()) {
				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ
			} else {
				Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ
			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
			break;
		case 3:														// ���ү��̒�~
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_EDY_STOP_BYTE ){	// ���ł�Suica��Edy����~�ς݂̏ꍇ
//				ret = 1;											// 
//			}else{													// Suica�����ү����܂���~���Ă��Ȃ��ꍇ
//				if( SUICA_USE_ERR || Suica_Rec.Data.BIT.MIRYO_TIMEOUT ){	// �װ���������Ă��邩�A����������(�K���s�ɂȂ邽��)�̏ꍇ
//					CCT_Cansel_Status.BIT.SUICA_END = 1;			// Suica��~�ς��׸ނ��
//					if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_EDY_STOP_BYTE ){	// Suica�ݒ�Ȃ���Suica�֘A�װ���������ź��ү�����~�ς�
//						ret = 1;											// 
//					}
//				}
//				if( EDY_USE_ERR || Edy_Rec.edy_status.BIT.MIRYO_LOSS ){
//					CCT_Cansel_Status.BIT.EDY_END = 1;				// Edy��~�ς��׸ނ��
//					if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_EDY_STOP_BYTE ){	// Edy�ݒ�Ȃ���Edy�֘A�װ���������ź��ү�����~�ς�
//						ret = 1;											// 
//					}
//				}
//			}
			if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_STOP_BYTE ){	// ���ł�Suica����~�ς݂̏ꍇ
				ret = 1;												// �S���޲���~����
			}else{														// Suica�����ү����܂���~���Ă��Ȃ��ꍇ
				if( SUICA_USE_ERR || Suica_Rec.Data.BIT.MIRYO_TIMEOUT ){	// �װ���������Ă��邩�A����������(�K���s�ɂȂ邽��)�̏ꍇ
					CCT_Cansel_Status.BIT.SUICA_END = 1;				// Suica��~�ς��׸ނ��
					ret = 1;											// �S���޲���~����
				}
			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			CCT_Cansel_Status.BIT.CMN_END = 1;					// Edy��~�ς��׸ނ��
			if( !CCT_Cansel_Status.BIT.SUICA_END ){					
				Lagtim( OPETCBNO, 2, 10*50 );						// ��ϰ2�N��(���ڰ��ݐ���p)
			}
			break;
		case 4:														// Suica���ςɂ�鐸�Z����
			CCT_Cansel_Status.BYTE = 0;
			CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica��~�ς��׸ނ��
			goto Op_StopModuleWait_sub_ALL;
			break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case 5:														// Edy���ςɂ�鐸�Z����
//			CCT_Cansel_Status.BYTE = 0;
//			// Edy���ώ���Suica�̒�~��҂����킹�Ă��������̂�Suica�͕K����~�ς݂ɂȂ��Ă��邽�߁A
//			// Edy���ώ���Suica�̏I���t���O���Z�b�g����
//			CCT_Cansel_Status.BIT.SUICA_END = 1;					// Suica��~�ς��׸ނ��
//			CCT_Cansel_Status.BIT.EDY_END = 1;						// Edy��~�ς��׸ނ��
//			goto Op_StopModuleWait_sub_ALL;
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		case 0xff:													// �S�Ẵf�o�C�X���������Ă��邩�ǂ����𔻒肷��
Op_StopModuleWait_sub_ALL:
			if( CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){	// ���Z�������̓��b�N�̒�~�܂ő҂����킹��
				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == ALL_STOP_BYTE ){	// ���łɂ��ׂẴf�o�C�X����~�ς�
					ret = 1;										// �S���޲���~����
				}
			}else{													// ����ȊO�̒�~�҂����킹����Suica��Edy�̑҂����킹���s��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_EDY_STOP_BYTE ){	// ���łɂ��ׂẴf�o�C�X����~�ς�
				if( (CCT_Cansel_Status.BYTE-STOP_BIT) == Suica_STOP_BYTE ){	// ���łɂ��ׂẴf�o�C�X����~�ς�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					ret = 1;										// �S���޲���~����
				}				
			}
			break;	
		default:
			break;
	}

	if( ret == 1 ){
		OPECTL.op_faz = save_op_faz;								// ���Z̪��ނ����ɖ߂�
		queset( OPETCBNO, ELE_EVT_STOP, 0, NULL );					// �I�y�ɓd�q�}�̂̒�~��ʒm					
	}

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			����\����ʔ���֐�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret �F0:����(�����ς�)��� 1:������
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/04/16<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char	First_Pay_Dsp( void )
{
	char	ret = 0;

	if((0 == ryo_buf.waribik) &&
	   (0 == c_pay) &&
	   (0 == e_pay) &&
	   (mode_Lagtim10 == 0) &&
	   ( !carduse() ) &&
	   ( vl_ggs.ryokin == 0 &&								// vl_ggs �F ���z���Z�̈� ��0�̎��ɻ��د��\���ĊJ
	   ( vl_frs.lockno == 0 || vl_frs.antipassoff_req) ) &&	// vl_frs �F �U�֐��Z�̈� ��0�A�������͒�����p�L��̎��ɻ��د��\���ĊJ	   														
	   (ryo_buf.nyukin == 0))
	   ret = 1;

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			���ό��ʃf�[�^��M̪��ވُ펞�̃G���[�o�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		*dat �F ���ό��ʃf�[�^
///	@param[in]		kind �F 1:���ϕs��̪��ނŌ��ό��ʂ���M
///				   		 �F 2:Ope����������O�ɍēx���ό��ʂ���M
///				   		 �F 3:Ope����x���ς����̂ɂ��ւ�炸�A�ēx
///				   		 	  ���ό��ʂ���M�����ꍇ
///				   		 �F 4:���ό��ʃf�[�^����M��A���ZLOG���ɓo�^
///				   			  ����O��̪��ނ��ς�����ꍇ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/05/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	Settlement_rcv_faze_err( uchar *dat, uchar kind )
{
// MH321800(S) G.So IC�N���W�b�g�Ή�(�ÓI���)
//	uchar	wkuc[24];
	uchar	wkuc[ERR_LOG_ASC_DAT_SIZE];
// MH321800(E) G.So IC�N���W�b�g�Ή�(�ÓI���)
	SUICA_SETTLEMENT_RES *work_set;
	long work;
	
	memset(wkuc, 0, sizeof(wkuc));
	if( kind == 1 || kind == 4 ){								// ���ό��ʃf�[�^�G���A�ɂ��łɃf�[�^���i�[����Ă���ꍇ�͂�����
		work_set = (SUICA_SETTLEMENT_RES *)dat;					// �f�[�^�Z�b�g
		intoasl( wkuc, (ulong)work_set->settlement_data, 5 );	// ���ϊz���Z�b�g
		wkuc[5] = wkuc[10] = ':';								//
		wkuc[6] = (uchar)(kind + 0x30);							// �o�^��ʂ��Z�b�g
		intoas( &wkuc[7], (ushort)OPECTL.Ope_mod, 3 );			// �o�^���̵��̪��ނ��Z�b�g
		memcpy( &wkuc[11], "****", 4 );							// ��ʂS�����}�X�N����
		memcpy( &wkuc[15], &work_set->Suica_ID[12], 4 );		// �J�[�h�ԍ��̉��S�����Z�b�g
	}else{														// ��M�f�[�^�����̂܂܎g���ꍇ�͂�����
		work = settlement_amount(&dat[3]);						// BCD�`���̌��ϊz��Bin�`���ɕϊ�
		intoasl( wkuc, (ulong)work, 5 );						// �����Ascii�`���ɕϊ����Ċi�[
		wkuc[5] = wkuc[10] = ':';								//
		wkuc[6] = (uchar)(kind + 0x30);							// �o�^��ʂ��Z�b�g
		intoas( &wkuc[7], (ushort)OPECTL.Ope_mod, 3 );			// �o�^���̵��̪��ނ��Z�b�g
		memcpy( &wkuc[11], "****", 4 );							// ��ʂS�����}�X�N����
		memcpy( &wkuc[15], &dat[24], 4 );                       // �J�[�h�ԍ��̉��S�����Z�b�g
	}
	err_chk2( (char)ERRMDL_SUICA, (char)61, (char)2, (char)1, (char)1, (void*)wkuc );
			// E6962�o�^�i���ϊz�p�����[�^����FASCII�j "E6961   08/03/17 14:38   ����/����"
			//											"        (99999:888:------------)  "
			//													  ��������z�ƁA���̎���OPmod
	#if (4 != AUTO_PAYMENT_PROGRAM)								// �����p�ɃJ�[�h�c�z�����܂����ꍇ�iýėp�j
	memcpy( &SUICA_LOG_REC_FOR_ERR, &SUICA_LOG_REC, sizeof( struct	suica_log_rec ));	// �ُ�۸ނƂ��Č��݂̒ʐM۸ނ�o�^����B
	#endif
	
	// �d���o�^��h�����߂ɂ��łɑ��̗v���ŃG���[�o�^����Ă����ꍇ�́Akind=4�̎���
	// �G���[�o�^�v���׸ނ��N���A����
	Suica_Rec.Data.BIT.LOG_DATA_SET = 0;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(���ψُ픭����͌��ς����Ȃ�)
	Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR = 1;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(���ψُ픭����͌��ς����Ȃ�)
}

//[]----------------------------------------------------------------------[]
///	@brief			��M�f�[�^�҂�����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret: 0:Continue	1:recv wait
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/06/10<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar	rcv_split_data_check( void )
{
	uchar ret = 0;
	t_Suica_rcv_event w_rcv_data;
	uchar rcv_judge=0;
	
	w_rcv_data.BYTE = Suica_Rec.suica_rcv_event.BYTE;							// ��M�f�[�^�������[�N�G���A�ɃZ�b�g
	if( !w_rcv_data.BYTE )														// ��M����Ă����������ꍇ
		return (ret=1);															// ��M��͏����͂��Ȃ�
	
	rcv_judge = 0x0B;
	w_rcv_data.BYTE &= rcv_judge;												// �\���˗��E���ό��ʁE��ԃf�[�^�̂ݒ��o
	if( !Suica_Rec.Data.BIT.DATA_RCV_WAIT ){									// �҂����킹���H
		if( Suica_Rec.Data.BIT.ADJUSTOR_NOW ){									// Suica���Z���H(���i�I���f�[�^���M���ɃZ�b�g�����)
			if(( w_rcv_data.BYTE == rcv_judge && Settlement_Res.Result == 0x01 )||	// ���ώ��̕K�v�ȃf�[�^���S�Ă�����Ă�H��
			   ( w_rcv_data.BYTE == rcv_judge && Settlement_Res.Result == 0x03 )||	// �c�z�s���J�[�h�^�b�`���̕K�v�ȃf�[�^���S�đ����Ă���
			   ( w_rcv_data.BIT.STATUS_DATA &&										// ��ԃf�[�^��M��
			   ( STATUS_DATA.StatusInfo.TradeCansel &&							// ����L�����Z����t�r�b�g���Z�b�g����Ă���
			    STATUS_DATA.StatusInfo.ReceptStatus ))){						// ��t�s�r�b�g���Z�b�g����Ă���
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;							// Suica���Z���t���O�N���A
				ret = 0;														// �����I��0���Z�b�g
			}else if( w_rcv_data.BYTE == rcv_judge && Settlement_Res.Result == 0x02 ){	// �g�p�s�J�[�h�^�b�`��
				ret = 0;														// �����I��0���Z�b�g
			}else if( w_rcv_data.BIT.STATUS_DATA &&								// ��ԃf�[�^��M��
				    ( STATUS_DATA.StatusInfo.MiryoStatus ||						// ����������
					  STATUS_DATA.StatusInfo.Running )){						// ���s���̏ꍇ�́A���Z��Ԃ͌p�����AOpeTask�ւ̒ʒm���s��
					ret = 0;													// �����I��0���Z�b�g
			}else if( w_rcv_data.BIT.STATUS_DATA  && 
					  (!STATUS_DATA.StatusInfo.ReceptStatus &&					// ��t������L�����Z����tBIT�������Ă��Ȃ��ꍇ�́A���Z��Ԃ͌p�����AOpeTask�ւ̒ʒm�͍s��Ȃ�
					   !STATUS_DATA.StatusInfo.TradeCansel) ){					
					ret = 0;													// �����I��0���Z�b�g
			}else{
				if( w_rcv_data.BIT.STATUS_DATA ){								// ��M�����f�[�^������f�[�^�ő҂����킹�̏ꍇ
					STATUS_DATA_WAIT.status_data = STATUS_DATA.status_data;		// �������҂����킹�悤�ɃZ�[�u
				}
				Suica_Rec.rcv_split_event.BYTE = w_rcv_data.BYTE;				// �҂����킹�p�̃G���A�ɏ�Ԃ�ۑ�
				Suica_Rec.Data.BIT.DATA_RCV_WAIT = 1;							// �f�[�^��M�҂��t���O�Z�b�g
				ret = 1;														// ��M�����͂��Ȃ��i�҂����킹���j
			}
		}
	}else{
		Suica_Rec.rcv_split_event.BYTE |= w_rcv_data.BYTE;						// ��M�����f�[�^�̏�Ԃ�҂����킹�G���A�ɔ��f

		if( w_rcv_data.BIT.STATUS_DATA ){										// ��M�����f�[�^������f�[�^�̏ꍇ
			STATUS_DATA.status_data |= STATUS_DATA_WAIT.status_data;			// �҂����킹�p�̃f�[�^���}�[�W����
		}
		
		if(( Suica_Rec.rcv_split_event.BYTE == rcv_judge && Settlement_Res.Result == 0x01 )|| // ���ώ��̕K�v�ȃf�[�^���S�Ă�����Ă�H��
		   ( Suica_Rec.rcv_split_event.BYTE == rcv_judge && Settlement_Res.Result == 0x03 )|| // �c�z�s���J�[�h�^�b�`���̕K�v�ȃf�[�^���S�đ����Ă���
		  ((( Suica_Rec.rcv_split_event.BIT.STATUS_DATA ) && 					// ��ԃf�[�^��M��
		  (STATUS_DATA.StatusInfo.TradeCansel  && 								// ����L�����Z����t�r�b�g���Z�b�g����Ă���
		   STATUS_DATA.StatusInfo.ReceptStatus)))){								// ��t�s�r�b�g���Z�b�g����Ă���
			Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;								// Suica���Z���t���O�N���A
			Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;								// �҂����킹�t���O�N���A
			Suica_Rec.suica_rcv_event.BYTE = Suica_Rec.rcv_split_event.BYTE;	// ��M����p�̃G���A���X�V
			STATUS_DATA.status_data = 0;										// ��ԃf�[�^�҂����킹�p�G���A�N���A
			ret = 0;															// ��M�������s��
		}else if( w_rcv_data.BYTE == rcv_judge && Settlement_Res.Result == 0x02 ){
			ret = 0;															// ��M�������s��
		}else{
			ret = 1;															// ��M�����͂��Ȃ��i�҂����킹���j
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			��M�f�[�^�҂������p�̃t���O���N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/06/11<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	Suica_rcv_split_flag_clear( void )
{
	Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;						// Suica���Z���t���O�N���A
	Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;						// �҂����킹�t���O�N���A	
	Suica_Rec.Data.BIT.SEND_CTRL80 = 0;							// ����I�����M���N���A
	STATUS_DATA.status_data = 0;								// �҂����킹���̏�ԃf�[�^�ێ��G���A�N���A
	Suica_Rec.Data.BIT.ADJUSTOR_START = 0;						// ���Z�J�n�t���O�Z�b�g
}

#if (4 == AUTO_PAYMENT_PROGRAM)								// �����p�ɒʐM���O�擾������ύX����
static uchar	d_work_buf[256];
//[]----------------------------------------------------------------------[]
///	@brief			Suica_Log_regist
//[]----------------------------------------------------------------------[]
///	@param[in]		logdata	�F�o�^���郍�O�f�[�^
///				    log_size�F�o�^���郍�O�T�C�Y
///				    kind	�F0�F��M�f�[�^ 1�F���M�f�[�^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/06/10<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void Suica_Log_regist_for_debug( unsigned char* logdata, ushort log_size, uchar kind )
{
	ushort  i,k,wks = 0;

	memset( &time_data, 0, sizeof( time_data ));
	memset( &d_work_buf, 0, sizeof( d_work_buf ));

	if( !SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start ){		// ۸��ް�������������Ă���ꍇ
		SUICA_LOG_REC_FOR_ERR.log_time_old = CLK_REC.ndat;					// �ŌÂ̓��t�ް����ޯ�����
		Log_data_edit( &SUICA_LOG_REC_FOR_ERR, 1 );							// ���t�ް��̕ҏW
	}
	if( CLK_REC.ndat != suica_work_time.ndat && SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start ){	// ���t���X�V����Ă���ꍇ
		Log_data_edit( &SUICA_LOG_REC_FOR_ERR, 1 );														// ���t�ް��̕ҏW
	}
	if( CLK_REC.nmin != suica_work_time.nmin || !SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start ){	// ���Ԃ��X�V����Ă���ꍇ
		Log_data_edit( &SUICA_LOG_REC_FOR_ERR, 0 );														// �����ް��̕ҏW
	}

	Log_data_edit2( &SUICA_LOG_REC_FOR_ERR,kind );														// �ް�ͯ�ނ̕ҏW(���M�ySD�z�E��M�yRD�z)

	if( (SUICA_LOG_REC_FOR_ERR.log_wpt+(log_size*2)) > SUICA_LOG_MAXSIZE-1){							// �ҏW�ް��̻�������
		wks = SUICA_LOG_MAXSIZE-SUICA_LOG_REC_FOR_ERR.log_wpt;											// �ޯ̧�̍Ō�܂Ŋi�[�ł��黲�ނ��Z�o

		for( i=0,k=0; i<log_size; i++){															// �ʐM�ް��̕ҏW
			hxtoas ( &d_work_buf[k],*(logdata+i) );												// ͷ��ް��𕶎���ɕϊ�
			k+=2;																				
		}
		memcpy( &SUICA_LOG_REC_FOR_ERR.log_Buf[SUICA_LOG_REC_FOR_ERR.log_wpt], d_work_buf, (size_t)wks );			// �ޯ̧�̍Ō�ɏ������߂镪�̒ʐM�ް���������
		SUICA_LOG_REC_FOR_ERR.log_wpt=7;																// �������߲���̍X�V

		Log_data_edit3( &SUICA_LOG_REC_FOR_ERR, (ushort)(SUICA_LOG_REC_FOR_ERR.log_wpt+k-wks), SUICA_LOG_REC_FOR_ERR.log_wpt );	// ���t�X�V����

		if( (k-wks) != 0 ){
			memcpy( &SUICA_LOG_REC_FOR_ERR.log_Buf[7], &d_work_buf[wks], (size_t)(k-wks) );				// �c����ް���擪���珑����
			SUICA_LOG_REC_FOR_ERR.log_wpt += (k-wks);													// �������߲���̍X�V
		}
		SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.write_flag=1;											// ���t�X�V�����׸ލX�V

	}else{
		if( SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.write_flag ){										// ���t�X�V�����׸�ON
			Log_data_edit3( &SUICA_LOG_REC_FOR_ERR, (ushort)(log_size+SUICA_LOG_REC_FOR_ERR.log_wpt), SUICA_LOG_REC_FOR_ERR.log_wpt );	// ���t�X�V����
		}
		for( i=0; i<log_size; i++){
			hxtoas ( &((uchar)SUICA_LOG_REC_FOR_ERR.log_Buf[SUICA_LOG_REC_FOR_ERR.log_wpt]),*(logdata+i) );		// �ʐM�ް��̕ϊ��E������
			SUICA_LOG_REC_FOR_ERR.log_wpt+=2;															// �������߲���̍X�V
		}
	}

	if( !SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start )									// �������׸ނ��O�̏ꍇ
		SUICA_LOG_REC_FOR_ERR.Suica_log_event.BIT.log_Write_Start = 1;									// �������׸ނ̍X�V

	suica_work_time = CLK_REC;																		
	
}
#endif

//[]----------------------------------------------------------------------[]
///	@brief			��M�L���[����d����Read����
//[]----------------------------------------------------------------------[]
///	@param[in]		recv_buf �F��M�L���[������o�����f�[�^�i�[�̈�
///					recv_size�F��M�f�[�^�T�C�Y
///	@return			ret 0�F�ް���ʖ��̉�͏������{ 1:��M�ް��҂����킹
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/10<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar	Suica_Read_RcvQue( void )
{
	uchar *pData = (uchar*)&suica_rcv_que.rcvdata[suica_rcv_que.readpt];	// ��M������߲����ܰ��̈�ݒ�
	uchar i,bc;
	uchar ret = 1;
	ushort	cnt;
	ushort	w_counter[3];
	
	memcpy( w_counter, &suica_rcv_que.readpt, sizeof( w_counter ) );		// �߲�������̈�̏�����

	for( i=0; (i<6 && *pData != ack_tbl[i]); i++);							// ��M���(ACK1�`NACK)����
	
    if( i==1 ){																// ACK2 �f�[�^��M ?
    	pData+=2;															// �ް��߲���̍X�V
		suica_save( pData, (short)*(pData-1) ); 							// ��M�ް��̉�͏���
		if( rcv_split_data_check() == 0 )									// �ް��҂����킹���菈��
			ret = 0;														// ��ʖ��̉�͏������{
    }else if( (i==2 )||(i==0) ){ 											// ACK3 or ACK1 �f�[�^��M?
    	bc = *(pData+1);													// ��M�ް�����ێ�
    	pData+=2;															// �ް��߲���̍X�V
		for( cnt=0 ;cnt < bc; ){											// �S�Ă̎�M�ް��̉�͂���������܂�ٰ��
			cnt += (*pData + 1);											// �ް���ʖ����ް����ނ����Z
			pData++;														// �ް��߲���̍X�V
			suica_save( pData,(short)*(pData-1) ); 							// ��M�ް��̉�͏���
			pData += *(pData-1);											// �ް��߲���̍X�V(�����ް��܂Ői�߂�)
		}
		if( rcv_split_data_check() == 0 )									// �ް��҂����킹���菈��
			ret = 0;                                                        // ��ʖ��̉�͏������{
    }
	
	if( ++w_counter[0] >= SUICA_QUE_MAX_COUNT ){							// ذ���߲����MAX�ɓ��B
		w_counter[0] = 0;													// ذ���߲����擪�Ɉړ�
	}
	if( w_counter[2] != 0 ){												// ��M�ް����Đ����O�łȂ��ꍇ
		w_counter[2]--;														// �޸���Ď��{
	}

	nmisave( &suica_rcv_que.readpt, w_counter, sizeof(w_counter) );			// �߲���̍X�V���d�ۏ�ōs��
	return ret;
	
}

//[]----------------------------------------------------------------------[]
///	@brief			�\���˗��f�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze �F���݂�Ope̪���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvDspData( uchar*	ope_faze )
{
	short ret = 0;
	ushort	wk_MsgNo=0;														// ��ʕ\���p�@���Z�}�̎�ʖ���ү����No.
	
	switch( *ope_faze ){
		case	0:
				if( OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod == 100 )
					break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				Edy_StopAndLedOff();										// EdyStop��LED����
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				edy_dsp.BIT.suica_zangaku_dsp = 1;							// �c�z�Ɖ�׸޾��

				LcdBackLightCtrl( ON );										// back light ON
				if( Is_SUICA_STYLE_OLD ){									// ���œ���
					wk_media_Type = Ope_Disp_Media_Getsub(0);				// ��ʕ\���p�@���Z�}�̎�� �擾
					switch(wk_media_Type) {
						case OPE_DISP_MEDIA_TYPE_SUICA:
							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[93] );			// "�r���������c�z�F          �~  "
							break;
						case OPE_DISP_MEDIA_TYPE_PASMO:
							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[104] );		// "�o�`�r�l�n�c�z�F          �~  "
							break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						case OPE_DISP_MEDIA_TYPE_EDY:
//							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[94] );			// "   �d�����c�z�F          �~   "
//							break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
						case OPE_DISP_MEDIA_TYPE_ICCARD:
							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[105] );		// "�h�b�J�[�h�c�z�F          �~  "
							break;
						case OPE_DISP_MEDIA_TYPE_ICOCA:
							grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[119] );		// "�h�b�n�b�`�c�z�F          �~  "
							break;
						default:
							break;
					}
					opedpl( 6, 16, pay_dsp, 5, 0, 0, COLOR_BLACK,  LCD_BLINK_OFF );					// ���z��\��
				}															// ���œ���(e)
				else{														// �V�œ���
					grachr( 6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[127] );					// "�@�@�@�c�z�F�@�@�@�@�@�~�@�@�@"

					opedpl( 6, 12, pay_dsp, 5, 0, 0, COLOR_FIREBRICK,  LCD_BLINK_OFF );						// ���z��\��
				}

				if( DspChangeTime[0] != 99 )								// Suica�c���\�����Ԃ�99�b�������ꍇ�͑��̲���Ăɂ���ʐؑւ܂ŕ\��
					Lagtim( OPETCBNO,11, (ushort)(DspChangeTime[0]*50) );	// IC���ގc���\���p��ϰ�N��(Timer9)

				Lagcan( OPETCBNO, 10 );										// ��ʐؑ֗p��ϰؾ��(Timer10)
				if( DspChangeTime[0] > 10 ){								// �c�z�\�����ԁ�10�r // 07-01-16�ǉ� //
					Lagtim( OPETCBNO, 1, (ushort)(DspChangeTime[0]*50) );   // �ޯ�ײē_�����Ԃ��c���\�����ԂƂ���
				}else{
					Lagtim( OPETCBNO, 1, 10*50 );							// �ޯ�ײē_�����Ԃ�10�Ƃ���
				}
				tim1_mov = 1;												// ��ϰ1�N���� // 07-01-16�ǉ� //
				break;
		case	2:
				if( Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA != 1){
					if( !WAR_MONEY_CHECK ){								// �������g�p�ς�
						wk_MsgNo = Ope_Disp_Media_GetMsgNo( 0, 3, 0 );
						lcd_wmsg_dsp_elec( 1, OPE_CHR[wk_MsgNo], OPE_CHR[100], 1, 0, COLOR_RED, LCD_BLINK_OFF );
						dsp_fusoku = 1;									// �װя����p�׸ނ��(�c���s�����̱װ��׸ނƋ��p)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						Restart_Timer();											// Timer1��ؾ�Ă���Timer8���N��	07-01-22�ǉ�
						Lagcan( OPETCBNO, 1 );										// ��ϰ1ؾ��(���ڰ��ݐ���p)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					}	
					else if(Suica_Rec.Data.BIT.PRI_NG){
						if( DspSts == LCD_WMSG_ON )								//�G���[���b�Z�[�W�\�����Ȃ�
							LagCan500ms(LAG500_ERROR_DISP_DELAY);				//�^�C�}�[�L�����Z��

						Lcd_WmsgDisp_ON( 1, COLOR_RED,  LCD_BLINK_OFF, OPE_CHR[136] );						// �u�����d�q�}�l�[�͎g�p�ł��܂���v
						
						LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 13, op_SuicaFusokuOff );	// 6sec��ɏ���
						
					
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						Restart_Timer();											// Timer1��ؾ�Ă���Timer8���N��
						Lagcan( OPETCBNO, 1 );										// ��ϰ1ؾ��(���ڰ��ݐ���p)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
					}
				}
				break;
		case	13:
		case	22:
				if( !WAR_MONEY_CHECK ){								// �������g�p�ς�
					wk_MsgNo = Ope_Disp_Media_GetMsgNo( 0, 3, 0 );
					lcd_wmsg_dsp_elec( 1, OPE_CHR[wk_MsgNo], OPE_CHR[100], 1, 0, COLOR_RED, LCD_BLINK_OFF );
					dsp_fusoku = 1;									// �װя����p�׸ނ��(�c���s�����̱װ��׸ނƋ��p)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					Restart_Timer();												// Timer1��ؾ�Ă���Timer8���N��	07-01-22�ǉ�
					Lagcan( OPETCBNO, 1 );										// ��ϰ1ؾ��(���ڰ��ݐ���p)
					Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				}	
				break;
		default:
				break;
	}
	
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			���ό��ʃf�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze �F���݂�Ope̪���
///	@return			ret		�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvSettData( uchar*	ope_faze )
{
	short ret = 0;
	
	if( (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 		// ���ό����ް�����M��
		(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) )		// ���ό����ް��������̏ꍇ
	{
		switch( OPECTL.Ope_mod ){
		case 2:											// �����\��,���������t�F�[�Y
			// ��ʌnIC�J�[�h�u���ό��ʃf�[�^�v�d���������ł���t�F�[�Y
			// �i�����ł�����M���Ȃ��͂��j
			break;
		default:	// ��ʌnIC�J�[�h�u���ό��ʃf�[�^�v�d�����������Ȃ������t�F�[�Y�Ŏ�M�����ꍇ
			Settlement_rcv_faze_err((uchar*)&Settlement_Res, 1 );		// �G���[�o�^����
			Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA = 1;	// ���ό��ʃf�[�^�����ςɂ���
			break;
		}
	}

	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				if( Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0 )			// ���ό����ް���M�ς݂Ŗ������̏ꍇ
				{
					Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA = 1;
					if( dsp_change ){													// ���Z����ү���ނ��\�������܂܂̏ꍇ
						op_SuicaFusokuOff();											// ү���ނ̏���
						dsp_change = 0;
					}
					Suica_Ctrl( S_CNTL_DATA, 0x80 );													// �����ް��i����I���j�𑗐M����
					
					if( Settlement_Res.Result == 0x02 ){								// ����NG(�g�p�s�¶��������)��M��
						Lcd_WmsgDisp_ON2( 1, COLOR_RED,  LCD_BLINK_OFF, suica_dsp_buff );
						Lcd_WmsgDisp_ON( 1, COLOR_RED,  LCD_BLINK_OFF, OPE_CHR[89] );					// 
						ope_anm( AVM_CARD_ERR1 );							// �ųݽ�u���̃J�[�h�͎g���܂���v
						LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 12, op_SuicaFusokuOff );	// 6sec��ɏ���
						if( !Suica_Rec.Data.BIT.MIRYO_TIMEOUT )							// ����������̃^�C���A�E�g������(�ȍ~����̂ݗL��)�łȂ����
							Lagtim( OPETCBNO, 11, 1*10 );								// ��t���M�^�C�}�[�X�^�[�g
					}else
					// ���ό��ʂ��c���s�������̎�
					if(Settlement_Res.Result == 0x03) {								
						if( OPECTL.op_faz == 8 && STATUS_DATA.StatusInfo.ReceptStatus ){// �d�q�}�̒�~�҂����킹��Suica�̏�Ԃ���t�s���
							Op_StopModuleWait_sub( 0 );									// Suica��~�ςݏ���
							ret = 0;													// ���Z���i�������j
							break;
						}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						if( OPECTL.op_faz == 10 && STATUS_DATA.StatusInfo.ReceptStatus ){	// Edy�ł̐��Z�J�n��Suica�̏�Ԃ���t�s���
//							OPECTL.op_faz = save_op_faz;								// �ۑ��ς݂�̪��ނ�߂�
//							ret = Edy_WarDsp_and_Pay();									// Edy�̌��Z�w�����M
//						}
//						Restart_Timer();											// Timer1��ؾ�Ă���Timer8���N�� 07-01-22�ǉ�
						Lagcan( OPETCBNO, 1 );										// ��ϰ1ؾ��(���ڰ��ݐ���p)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

 
						if( !dsp_fusoku ){									// �c�z�s���\��������Ă��Ȃ�
							#if (3 == AUTO_PAYMENT_PROGRAM)							// �����p�ɃJ�[�h�c�z�����܂����ꍇ�iýėp�j
								if( 0L != (unsigned long)CPrmSS[S_SYS][5] )			// �ݒ肠���
									op_SuicaFusoku_Msg( (ulong)CPrmSS[S_SYS][5], suica_dsp_buff, 0 );	// �w��l�ɓǂݑւ���
								else
									op_SuicaFusoku_Msg( (ulong)Settlement_Res.settlement_data_after, suica_dsp_buff, 0 );
							#else
								op_SuicaFusoku_Msg( (ulong)Settlement_Res.settlement_data_after, suica_dsp_buff, 0 );
							#endif
							lcd_wmsg_dsp_elec( 1, suica_dsp_buff, OPE_CHR[89], 1, 1, COLOR_RED, LCD_BLINK_OFF );
							dsp_fusoku = 1;							// �c�z�s��ү���ނ̕ҏW
						}

						if( OPECTL.op_faz == 9 ) {										// ���Z���~���݉�����Suica��~�҂����킹����
							OPECTL.op_faz = 1;											// ̪��ނ����ɖ߂��Ă���
							op_SuicaFusokuOff();										// �\������ү���ނ�����
							Op_Cansel_Wait_sub( 0 );									// Suica��~�ςݏ���

						}else{
							mode_Lagtim10 = 1;											// Lagtimer10�̎g�p��� 1:Suica��~��̎�t���đ��MWait�I��
							Lagtim( OPETCBNO, 10, 1*10 );								// Suica��~��̎�t�����M�҂���ϰ200mswait(Timer10)
						}
						break;
					}
					// ���ό��ʂ����������̎��i�c���s����̍ă^�b�`�����܂ށj
					else if(Settlement_Res.Result == 0x01) {
						Lagcan( OPETCBNO, 8 );									// ��ϰ8ؾ��(���ڰ��ݐ���p) 07-01-22�ǉ�
						Lagcan( OPETCBNO, 1 );									// ��ϰ10ؾ��
						Lagcan( OPETCBNO, 10 );									// ��ϰ10ؾ��
						nyukin_delay_check( nyukin_delay, delay_count);					// �ۗ����̲���Ă�����΍�post����
						e_inflg = 1;													// Suica���ςł���
						e_pay = Settlement_Res.settlement_data;							// Suica���ϊz��ۑ�
						e_zandaka = Settlement_Res.settlement_data_after;				// Suica�c����ۑ�
						if( Is_SUICA_STYLE_OLD ){										// ���œ���
							wk_media_Type = Ope_Disp_Media_Getsub(1);					// ��ʕ\���p�@���Z�}�̎�� �擾
							if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		
								ElectronSet_PayData( &Settlement_Res, PASMO_USED );		// PASMO�Ő��Z
							} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {
								ElectronSet_PayData( &Settlement_Res, SUICA_USED );		// Suica�Ő��Z
							} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICOCA) {
								ElectronSet_PayData( &Settlement_Res, ICOCA_USED );		// ICOCA�Ő��Z
							} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICCARD) {
								ElectronSet_PayData( &Settlement_Res, ICCARD_USED );	// IDCARD�Ő��Z
							}
						}																// ���œ���(e)
						else{															// �V�œ���
							ElectronSet_PayData( &Settlement_Res, SUICA_USED );		// Suica�Ő��Z
						}
						ret = al_emony( SUICA_EVT , 0 );								// �d�q�}�l�[���Z����
						if( ryo_buf.dsp_ryo ){
							Lagtim( OPETCBNO, 11, 1*10 );								// Suica���Z���ޔF�شװ������ϰ200mswait(Timer11)
							if( OPECTL.op_faz == 3 ){									// ��������̏ꍇ
								Op_Cansel_Wait_sub( 0 );								// Suica��~�ςݏ������s
							}
						}
						// ���ϊ�������in_mony()�ɂĐ��Z�������̏������s����
					}

					else {
						// ���ό��ʂ�01or03�ȊO�͂Ȃ�
					}
				}
				break;
		case	3:
				break;
	}
	
	return ret;	
}

// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
////[]----------------------------------------------------------------------[]
/////	@brief			�ُ�f�[�^����
////[]----------------------------------------------------------------------[]
/////	@param[in]		ope_faze �F���݂�Ope̪���
/////	@return			ret		�F0�Œ�
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 08/07/15<br>
/////					Update
////[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
//short	RecvErrData( uchar*	ope_faze )
//{
//	short ret = 0;
//	
//	switch( *ope_faze ){
//		case	1:
//				break;
//		case	2:
//				break;
//		case	3:
//				break;
//	}
//	
//	return ret;	
//}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) SX-20�����C��
//[]----------------------------------------------------------------------[]
///	@brief			�ُ�f�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze �F���݂�Ope̪���
///	@return			ret		�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvErrData( uchar*	ope_faze )
{
	short ret = 0;
	
	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				break;
		case	3:
				break;
	}
	
	return ret;	
}
// MH810103 GG119202(E) SX-20�����C��

//[]----------------------------------------------------------------------[]
///	@brief			��ԃf�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze �F���݂�Ope̪���
///	@return			ret		�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvStatusData( uchar*	ope_faze )
{
	short ret = 0;
	
	Suica_Status_Chg();
	switch( *ope_faze ){
		case	0:
				if( Suica_Rec.Data.BIT.CTRL == 0 && STATUS_DATA.StatusInfo.ReceptStatus ){
					if( OPECTL.op_faz == 8 ){ 										// �d�q�}�̒�~������
						Op_StopModuleWait_sub( 0 );									// Suica��~�ςݏ���
					} 
					Lagtim( OPETCBNO,11, 1*10 );									// ��t���M��ϰ�N��(Timer11)
				}
				break;
		case	2:
				if(!Suica_Rec.Data.BIT.CTRL_MIRYO) {								// ���Z������ԂŖ����ꍇ�̂ݏ��������{
					if(Suica_Rec.Data.BIT.CTRL) {									// �����ް�����Ԃ̎���
						if( OPECTL.op_faz == 9 || STATUS_DATA.StatusInfo.Running ) {// ���Z���~����Suica��~�҂����킹��(����ɒ�~���Ă��Ȃ��ꍇ)����M�X�e�[�^�X�����s��
							break;
						}
						if( OPECTL.op_faz == 8 || OPECTL.op_faz == 3 ) {			// �d�q�}�̒�~�҂����킹���H
							// �d�q�}�̒�~�҂����킹���ł̎�t��M���́A̪��ލ��킹�ׂ̈Ɏ�t�s�𑗐M����B
							Suica_Ctrl( S_CNTL_DATA, 0 );										// Suica���p��s�ɂ���
							break;
						}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						if( OPECTL.op_faz == 10 ) {									// Edy�ł̐��Z�J�n��Suica��~�҂����킹����
//							break;
//						}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

						if( OPECTL.op_faz != 3 && WAR_MONEY_CHECK && !e_incnt){		// ���������݉�������Ă��ꍇ�E����������ESuica�g�p��ȊO��
																					// �ꍇ�́A���i�I���ް����M����B
							if(w_settlement != 0) {									// �c���s�����̈�������
								Suica_Data_Snd( S_SELECT_DATA, &w_settlement);		// Suica�c���i10�~�����؎̂āj�����i�I���ް��Ƃ��đ��M����
							} else {												// �ʏ펞
								Suica_Ctrl( S_SELECT_DATA, 0 );						// ���ԗ��������i�I���ް��Ƃ��đ��M����
								w_settlement = 0;									// ���i�I���f�[�^�ҏW�̈��ر����
							}
						}
					}
					else {															// �����ް����s��Ԃ̎���
						if( STATUS_DATA.StatusInfo.ReceptStatus &&					// ��t�s�f�[�^�y��
							STATUS_DATA.StatusInfo.TradeCansel ) {					// �����ݾَ�t�f�[�^����M��
							if( dsp_change ){
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//								if( !e_incnt && !SUICA_CM_BV_RD_STOP ){				// �d�q���ϖ����{�̏ꍇ�������㐸�Z�\�ݒ�̏ꍇ
//									Edy_SndData01();								// ���ތ��m��~�w�����M
//								}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
								op_SuicaFusokuOff();								// ���Z�������̔��]ү���ޕ\������������
								dsp_change = 0;
								miryo_timeout_after_mif( 0 );
								nyukin_delay_check( nyukin_delay, delay_count);		// �ۗ����̲���Ă�����΍�post����
								miryo_timeout_after_disp();							// �����^�C���A�E�g��̕\�����s��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//								Restart_Timer();									// Timer1��ؾ�Ă���Timer8���N��	07-01-22�ǉ�
								Lagcan( OPETCBNO, 1 );								// ��ϰ1ؾ��(���ڰ��ݐ���p)
								Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
							}
							Suica_Ctrl( S_CNTL_DATA, 0x80 );										// �����ް��i����I���j�𑗐M
							if( !time_out )											// �������(�������)�ɂ���ѱ�Ĕ�����
								Lagtim( OPETCBNO, 11, 1*10 );						// Suica���Z���ޔF�شװ������ϰ200mswait(Timer11)
						}
						
						if( OPECTL.op_faz == 3 || OPECTL.op_faz == 9 || time_out ) {// ���Z���~���݉�����Suica��~�҂����킹����
							Op_Cansel_Wait_sub( 0 );								// Suica��~�ς݂Ƃ��Ē�~���f���s��
							break;
						}
						
						if( OPECTL.op_faz == 8 ) {									// �d�q�}�̒�~�҂����킹����
							if( Op_StopModuleWait_sub( 0 ) == 1 ){					// Suica��~�ς݂Ƃ��Ē�~���f���s��
								ret = 0;
								break;
							}
						}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						if( OPECTL.op_faz == 10 ) {									// Edy�ł̐��Z�J�n��Suica��~�҂����킹����
//							OPECTL.op_faz = save_op_faz;							// ���Z̪��ނ����ɖ߂�
//							ret = Edy_WarDsp_and_Pay();								// Edy�̐��Z���������s����B
//							break;
//						}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

						if( STATUS_DATA.StatusInfo.ReceptStatus && 					// Suicaذ�ް����t�s��ԂƂȂ����ꍇ��
							!Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA ){	// ���ό��ʃf�[�^�𖢏����̏ꍇ�́A�Ċ����������̃^�C�}�[���X�^�[�g
																					// ��t�d���𑗐M���邩�ǂ����̓^�C���A�E�g��Ŕ��肷��
							if( !time_out )											// �������(�������)�ɂ���ѱ�Ĕ�����
								Lagtim( OPETCBNO, 11, 1*10 );						// ��t���M�^�C�}�[�X�^�[�g
						}
					}
				}else{
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						Edy_StopAndLedOff();										// ���ތ��m��~��UI LED�����w�����M
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
						if( MIFARE_CARD_DoesUse  ){									// Mifare���L���ȏꍇ
							LagCan500ms( LAG500_MIF_LED_ONOFF );				// Mifare�ėL������ϰ�į��
							op_MifareStop_with_LED();								// Mifare����
						}
						if( First_Pay_Dsp() ) 					         			// �������(�����\��)
							Lagcan( OPETCBNO, 1 );									// ��ϰ1ؾ��
						else
							Lagcan( OPETCBNO, 8 );									// ��ϰ8ؾ��
						Lagcan( OPETCBNO, 10 );										// ��ϰ10ؾ��
						dsp_change = 1;												//  �����װѕ\�����׸޾��
						lcd_wmsg_dsp_elec( 1, OPE_CHR[91], OPE_CHR[92], 0, 0, COLOR_RED, LCD_BLINK_OFF );
						if( dsp_fusoku )											//  �s���A���[���\�����H
					    	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );		//  �s���A���[��������ϰ���~�߂�
				}
				break;
		case	3:
		case	14:
		case	23:
				Suica_Ctrl( S_CNTL_DATA, 0x80 );									// �����ް��i����I���j�𑗐M����
				break;
	}

	
	if( STATUS_DATA.StatusInfo.DetectionSwitch ){									// ���ߌ��m�y��
		Suica_Data_Snd( S_PAY_DATA,&CLK_REC);										// SX-10���ߏ������s
	}
	return ret;	
}

// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
////[]----------------------------------------------------------------------[]
/////	@brief			���������f�[�^����
////[]----------------------------------------------------------------------[]
/////	@param[in]		ope_faze �F���݂�Ope̪���
/////	@return			ret		�F0�Œ�
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 08/07/15<br>
/////					Update
////[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
//short	RecvTimeData( uchar*	ope_faze )
//{
//	short ret = 0;
//	
//	switch( *ope_faze ){
//		case	1:
//				break;
//		case	2:
//				break;
//		case	3:
//				break;
//	}
//	
//	return ret;	
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			�Œ�f�[�^��M����
////[]----------------------------------------------------------------------[]
/////	@param[in]		ope_faze �F���݂�Ope̪���
/////	@return			ret		�F0�Œ�
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 08/07/15<br>
/////					Update
////[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
//short	RecvFixData( uchar*	ope_faze ){
//	
//	short ret = 0;
//	
//	switch( *ope_faze ){
//		case	1:
//				break;
//		case	2:
//				break;
//		case	3:
//				break;
//	}
//	
//	return ret;	
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			�󎚈˗��f�[�^�i�|�C���g�j�f�[�^��M����
////[]----------------------------------------------------------------------[]
/////	@param[in]		ope_faze �F���݂�Ope̪���
/////	@return			ret		�F0�Œ�
////[]----------------------------------------------------------------------[]
/////	@author			T.Namioka
/////	@date			Create	: 08/07/15<br>
/////					Update
////[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
//short	RecvPointData( uchar*	ope_faze ){
//	
//	short ret = 0;
//	
//	switch( *ope_faze ){
//		case	1:
//				break;
//		case	2:
//				break;
//		case	3:
//				break;
//	}
//	
//	return ret;
//	
//}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) SX-20�����C��
//[]----------------------------------------------------------------------[]
///	@brief			���������f�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze �F���݂�Ope̪���
///	@return			ret		�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvTimeData( uchar*	ope_faze )
{
	short ret = 0;
	
	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				break;
		case	3:
				break;
	}
	
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			�Œ�f�[�^��M����
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze �F���݂�Ope̪���
///	@return			ret		�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvFixData( uchar*	ope_faze ){
	
	short ret = 0;
	
	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				break;
		case	3:
				break;
	}
	
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			�󎚈˗��f�[�^�i�|�C���g�j�f�[�^��M����
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze �F���݂�Ope̪���
///	@return			ret		�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
short	RecvPointData( uchar*	ope_faze ){
	
	short ret = 0;
	
	switch( *ope_faze ){
		case	1:
				break;
		case	2:
				break;
		case	3:
				break;
	}
	
	return ret;
	
}
// MH810103 GG119202(E) SX-20�����C��

/*[]----------------------------------------------------------------------[]*/
/*| �d�q�}�̏I���������s					                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Cansel_Ele_Start	                                       |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*| ���l		 : ���������ϰ���Ő��Z��ʂ��畜�A����ꍇ�ɓd�q�}�̂��~|*/
/*|				 : �����邽�߂̏����B									   |*/
/*|				 : �ڐA����GT4700�ɂ͖�����UT�͐��Z̪��ނ���ϰ�����I������ |*/
/*|				 : ���邽�ߖ{�֐��ɂċ��ʉ�����							   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-12-14                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void Cansel_Ele_Start( void )
{

// MH810103 GG119202(S) ����f�[�^��A���ő��M���Ă��܂�
//	if(( Suica_Rec.Data.BIT.CTRL || Suica_Rec.Data.BIT.OPE_CTRL )&& !SUICA_USE_ERR ) {			// �����ް�����Ԃ̎��܂��̓G���[���������Ă���ꍇ��
// MH810104(S) MH321800(S) ���撆�ɂƂ肯������Ɖ�ʂ��ł܂�
//	if (((!Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL) ||	// ��t�����M��
//		 (Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL)) &&	// ��t�֎~���M��
	// ���Ɏ�t�s��Ԃł���΁A��~��ԂƂ���
	if ((Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL) &&	// ��t�֎~���M��
// MH810104(E) MH321800(E) ���撆�ɂƂ肯������Ɖ�ʂ��ł܂�
		!SUICA_USE_ERR) {												// �G���[������
		// ��t���^�֎~���M���͏�ԃf�[�^��M��҂�
		// �P�D��t�����M���̏ꍇ��EcRecvStatusData()��
		// �@�@��ԃf�[�^�i��t�j��M����Ɛ���f�[�^�i��t�֎~�j�𑗐M����
		// �Q�D��t�֎~���M���̏ꍇ��EcRecvStatusData()��
		// �@�@��ԃf�[�^�i��t�s�j����M�����Op_Cansel_Wait_sub(0)��
		// �@�@���s����
		Lagtim( OPETCBNO, 2, 10*50 );							// ��ϰ2���(���ڰ��ݐ���p)(10S)
	}
	else if (Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR) {		// ��t��ԁA���A�G���[������
// MH810103 GG119202(E) ����f�[�^��A���ő��M���Ă��܂�
		Suica_Ctrl( S_CNTL_DATA, 0 );										// Suica���p��s�ɂ���
		Lagtim( OPETCBNO, 2, 10*50 );							// ��ϰ2���(���ڰ��ݐ���p)(10S)
	}else{														// Suica�����łɒ�~�ς݂Ȃ�
		cansel_status.BIT.SUICA_END = 1;						// ��~��ԂƂ���
	}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ){				// Edy�ݒ肪�L���̏ꍇ�AEdy�̒�~�҂��̍̏W�^�C�}�[�Ƃ��Ĉȉ����{
//		LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 120, Op_ReturnLastTimer );// �c�z�\���p��ϰ���ꎞ�I�Ɏg�p����(60�b)
//	}
//
//	if( !EDY_USE_ERR )											// Edy�֘A�̃G���[������
//	{
//		Edy_StopAndLedOff();									// ���ތ��m��~��UI LED�����w�����M
//		LagCan500ms( LAG500_EDY_LED_RESET_TIMER );				// ������ς��~����
//	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	cansel_status.BIT.INITIALIZE = 1;							// �d�q�}�̂̒�~�������X�^�[�g
	Op_Cansel_Wait_sub( 0xff );									// �S�f�o�C�X�̏I�����`�F�b�N����
	
	Suica_Rec.Data.BIT.ADJUSTOR_START = 0;						// ���Z���~���ɂ͏��i�I���f�[�^���M�ۃt���O���N���A����
}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define Suica_CMN_CANSEL_BYTE 0x07								// Suica�y�Ѻ��ү�����~�ς�
//#define EDY_CMN_CANSEL_BYTE 0x0b								// Edy�y�Ѻ��ү�����~�ς�
//#define Suica_EDY_CANSEL_BYTE 0x0d								// Suica�y�Ѻ��ү�����~�ς�
//#define ALL_CANSEL_BYTE 0x0f									// �S�Ă̔}�̂���~�ς�
#define Suica_CANSEL_BYTE	0x05								// Suica�y�Ѻ��ү�����~�ς�
#define CMN_CANSEL_BYTE		0x03								// Edy�y�Ѻ��ү�����~�ς�
#define ALL_CANSEL_BYTE		0x07								// �S�Ă̔}�̂���~�ς�
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

//[]----------------------------------------------------------------------[]
///	@brief			Op_Cansel_Wait_sub
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �C�x���g���
///	@return			ret		: ���Z�����I������
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/25<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Op_Cansel_Wait_sub( uchar kind )
{
	uchar ret = 0;

	switch( kind ){
		case 0:															// Suica�C�x���g
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( cansel_status.BYTE == EDY_CMN_CANSEL_BYTE ){			// �I���ð���FEdy&���ү��I���ς݂̏ꍇ
//				ret = 1;												// ���Z�������I������
//			}else{														// Edy�y��ү����܂��I�����Ă��Ȃ��ꍇ
//				if( EDY_USE_ERR ){										// Edy���g�p�s���װ���������Ă���ꍇ��
//					cansel_status.BIT.EDY_END = 1;						// Suica�I���ς��׸ނ��
//					if( cansel_status.BYTE == EDY_CMN_CANSEL_BYTE ){	// Edy&���ү��I���ς�
//						ret = 1;										// ���Z�������I������
//					}
//				}
//			}
			if( cansel_status.BYTE == CMN_CANSEL_BYTE ){				// �I���ð���F���ү��I���ς݂̏ꍇ
				ret = 1;												// ���Z�������I������
			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			cansel_status.BIT.SUICA_END = 1;							// Suica�I���ς��׸ނ��
			break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case 1:															// Edy�C�x���g
//			if( cansel_status.BYTE == Suica_CMN_CANSEL_BYTE ){			// �I���ð���FSuica&ү��I���ς݂̏ꍇ
//				ret = 1;												// ���Z�������I������
//			}else{														// Suica�y��ү����܂��I�����Ă��Ȃ��ꍇ
//				if( SUICA_USE_ERR ){									// suica���g�p�s���װ���������Ă���ꍇ��
//					cansel_status.BIT.SUICA_END = 1;					// Suica�I���ς��׸ނ��
//					if( cansel_status.BYTE == Suica_CMN_CANSEL_BYTE ){	// Suica&���ү��I���ς�
//						ret = 1;										// ���Z�������I������
//					}
//				}
//			}
//			cansel_status.BIT.EDY_END = 1;								// Edy�I���ς��׸ނ��
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		case 2:															// ���ү������
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			if( cansel_status.BYTE == Suica_EDY_CANSEL_BYTE ){			// �I���ð���FSuica��Edy�I���ς݂̏ꍇ
//				ret = 1;												// ���Z�������I������
//			}else{														// Suica�y��Edy���܂��I�����Ă��Ȃ��ꍇ
//				if( SUICA_USE_ERR ){									// Suica�֘A�̴װ���������Ă���ꍇ
//					cansel_status.BIT.SUICA_END = 1;					// Suica�I���ς��׸ނ��
//					if( cansel_status.BYTE == Suica_EDY_CANSEL_BYTE ){	// Suica&Edy�I���ς݁H
//						ret = 1;										// ���Z�������I������
//					}
//				}
//				if( EDY_USE_ERR ){										// Edy�֘A�װ���������Ă���ꍇ
//					cansel_status.BIT.EDY_END = 1;						// Edy�I���ς��׸ނ��
//					if( cansel_status.BYTE == Suica_EDY_CANSEL_BYTE ){	// Suica&Edy�I���ς݁H
//						ret = 1;										// ���Z�������I������
//					}
//				}
//			}
			if( cansel_status.BYTE == Suica_CANSEL_BYTE ){				// �I���ð���FSuica�I���ς݂̏ꍇ
				ret = 1;												// ���Z�������I������
			}else{														// Suica���܂��I�����Ă��Ȃ��ꍇ
				if( SUICA_USE_ERR ){									// Suica�֘A�̴װ���������Ă���ꍇ
					cansel_status.BIT.SUICA_END = 1;					// Suica�I���ς��׸ނ��
					ret = 1;											// ���Z�������I������
				}
			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			cansel_status.BIT.CMN_END = 1;								// ү��I���ς��׸ނ��
			break;
		case 0xff:														// �S�Ẵf�o�C�X���������Ă��邩�ǂ����𔻒肷��
			if( cansel_status.BYTE == ALL_CANSEL_BYTE ){				// ���łɂ��ׂẴf�o�C�X����~�ς�
				ret = 1;												// ���Z�������I������
			}
			break;	
		default:
			break;
	}
	
	if( ret == 1 && ryo_buf.zankin ){									// 
		queset( OPETCBNO, ELE_EVENT_CANSEL, 0, NULL );					// �I�y�ɓd�q�}�̂̒�~��ʒm					
	}
	
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica_fukuden_que_check	 
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/07/29<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	Suica_fukuden_que_check( void )
{
	uchar *pData;
	uchar i,bc;
	ushort	cnt;
	ushort	w_counter[3];

	if( !suica_rcv_que.count )											// �������f�[�^�������ꍇ�͔�����
		return;
		
	memcpy( w_counter, &suica_rcv_que.readpt, sizeof( w_counter ) );	// offset�֘A�����[�N�ɑޔ�
	
	for( ; w_counter[2]!=0; --w_counter[2] ){							// �������f�[�^�����[�v
		pData = (uchar*)&suica_rcv_que.rcvdata[w_counter[0]];			// ���������p�Ƀ��[�N�̈�ɓW�J
		
		for( i=0; (i<6 && *pData != ack_tbl[i]); i++);					// ��M��ʂ����
		
	    if( i==1 ){														// ACK2 �f�[�^��M ? 
	    	pData+=2;													// �|�C���^�ʒu��␳
// MH321800(S) G.So IC�N���W�b�g�Ή�
			if (isEC_USE() != 0) {
			// ���σ��[�_
				if (*pData == S_SETTLEMENT_DATA) {						// ���ό��ʃf�[�^
					EcSettlementPhaseError((pData+1), 5);				// �G���[�f�[�^�o�^���s
				}
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
				else if(*pData == S_RECEIVE_DEEMED)						// ���ϒ��O����f�[�^
				{
					// ac_flg.cycl_fg��57,58�ł���ꍇ�͂��ł�fukuden()�ŏ����ς̂͂�
					// �܂��Aac_flg.ec_recv_deemed_fg��ON�̏ꍇ�����ł�fukuden()�ŏ����ς̂͂�
					// ��ac_flg.cycl_fg��57,58�ł͂Ȃ��Aac_flg.ec_recv_deemed_fg=0�ł���A
					// suica_rcv_que.rcvdata�ɒ��O����f�[�^���Z�b�g����Ă����Ԃł̕��d���݂̂����ŏ�������B
					// EcRecvDeemedData.WFlag = 0�̏ꍇ�A���d�ɂĒ��O����f�[�^���܂����O�o�^�A�󎚏�������Ă��Ȃ��̂ł����ŏ�������
					if( EcRecvDeemedData.WFlag == 0 ){
						if( EcRecvDeemed_DataSet(pData+1) ){
							// ���O����f�[�^���Z�b�g�����烍�O�o�^�A�󎚏���������
							EcRecvDeemed_RegistPri();
						}
					}
				}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
			} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
			if( *pData == S_SETTLEMENT_DATA && *(pData+1) == 1 ){		// ���ό��ʃf�[�^�Ńf�[�^�̓��e���L���̏ꍇ
				Settlement_rcv_faze_err( (pData+1), 5 );				// �G���[�f�[�^�o�^���s
			}
// MH321800(S) G.So IC�N���W�b�g�Ή�
			}
// MH321800(E) G.So IC�N���W�b�g�Ή�
	    }else if( (i==2 )||(i==0) ){ 									// ACK3 or ACK1 �f�[�^��M?
	    	bc = *(pData+1);											// ��M�f�[�^�o�C�g����ݒ�
	    	pData+=2;													// �|�C���^�ʒu��␳
			for( cnt=0 ;cnt < bc; ){									// �S�f�[�^����͂���܂Ń��[�v
				cnt += (*pData + 1);									// �e�f�[�^���̃T�C�Y�����Z
				pData++;												// �|�C���^�ʒu��␳
// MH321800(S) G.So IC�N���W�b�g�Ή�
				if (isEC_USE() != 0) {
				// ���σ��[�_
					if (*pData == S_SETTLEMENT_DATA) {					// ���ό��ʃf�[�^
						EcSettlementPhaseError((pData+1), 5);			// �G���[�f�[�^�o�^���s
					}
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
					else if(*pData == S_RECEIVE_DEEMED)					// ���ϒ��O����f�[�^
					{
						// ac_flg.cycl_fg��57,58�ł���ꍇ�͂��ł�fukuden()�ŏ����ς̂͂�
						// �܂��Aac_flg.ec_recv_deemed_fg��ON�̏ꍇ�����ł�fukuden()�ŏ����ς̂͂�
						// ��ac_flg.cycl_fg��57,58�ł͂Ȃ��Aac_flg.ec_recv_deemed_fg=0�ł���A
						// suica_rcv_que.rcvdata�ɒ��O����f�[�^���Z�b�g����Ă����Ԃł̕��d���݂̂����ŏ�������B
						// EcRecvDeemedData.WFlag = 0�̏ꍇ�A���d�ɂĒ��O����f�[�^���܂����O�o�^�A�󎚏�������Ă��Ȃ��̂ł����ŏ�������
						if( EcRecvDeemedData.WFlag == 0 ){
							if( EcRecvDeemed_DataSet(pData+1) ){
								// ���O����f�[�^���Z�b�g�����烍�O�o�^�A�󎚏���������
								EcRecvDeemed_RegistPri();
							}
						}
					}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
				} else {
// MH321800(E) G.So IC�N���W�b�g�Ή�
				if( *pData == S_SETTLEMENT_DATA && *(pData+1) == 1 ){	// ���ό��ʃf�[�^�Ńf�[�^�̓��e���L���̏ꍇ
					Settlement_rcv_faze_err( (pData+1), 5 );			// �G���[�f�[�^�o�^���s
				}
// MH321800(S) G.So IC�N���W�b�g�Ή�
				}
// MH321800(E) G.So IC�N���W�b�g�Ή�
				pData += *(pData-1);									// �|�C���^�ʒu��␳
			}
	    }
		
		if( ++w_counter[0] >= SUICA_QUE_MAX_COUNT ){					// ���[�h�|�C���^���L���[�̍Ō�̏ꍇ
			w_counter[0] = 0;											// �擪�֖߂�
		}

		nmisave( &suica_rcv_que.readpt, w_counter, sizeof(w_counter) );	// �{�֐��������ɍēx��d�����ꍇ�ɔ�����offset�֘A���X�V
	}
	memset( &suica_rcv_que.readpt, 0, sizeof( w_counter ));				// ����������offset��������
}

//[]----------------------------------------------------------------------[]
///	@brief			�����^�C���A�E�g��̉�ʕ\������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/08/22<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void miryo_timeout_after_disp( void )
{
	uchar timer=0;
	const uchar *msg;
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
//// GG116200(S) G.So IC�N���W�b�g�Ή�
//	static const
//	uchar msg_idx[4] = {49, 50,							// 1�y�[�W��
//						51, 52};						// 2�y�[�W��
//// GG116200(E) G.So IC�N���W�b�g�Ή�
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���

// MH810103 GG118808_GG118908(S) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
	if (isEC_USE() != 0 && ryo_buf.zankin == 0) {
		// �c����0�ł���΁A�\�����Ȃ�
		return;
	}
// MH810103 GG118808_GG118908(E) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����

	if( SUICA_MIRYO_AFTER_STOP ){						// �����������Suica���g�p�s�ݒ�Ƃ���
		Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;			// �����^�C���A�E�g����
		mode_Lagtim10 = 0;								// Lagtimer10�̎g�p��� 0:���p�\�}�̻��د��\���ĊJ
		if( SUICA_ONLY_STOP ){							//	Suica�݂̂��g�p�s�Ƃ���ݒ�̏ꍇ
			msg = ERR_CHR[18];
			timer = 1;
		}else{											//  Suica�ECM�EBV�E���Cذ�ް���g�p�s�Ƃ���ݒ�̏ꍇ
			msg = ERR_CHR[22];
		}
		
// MH321800(S) G.So IC�N���W�b�g�Ή�
//		lcd_wmsg_dsp_elec( 1, ERR_CHR[20], msg, timer, 0, COLOR_RED, LCD_BLINK_OFF );
		if (isEC_USE() != 0) {
// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH810103 GG118809_GG118909(S) WAON�̖����c���Ɖ�Ŗ�����������m�F�����ꍇ�͂Ƃ肯���{�^�������҂��Ƃ���
////			if(Ec_Settlement_Res.brand_no == BRANDNO_WAON) {
//			if(Ec_Settlement_Res.brand_no == BRANDNO_WAON && (ECCTL.ec_Disp_No != 96)) {
//// MH810103 GG118809_GG118909(E) WAON�̖����c���Ɖ�Ŗ�����������m�F�����ꍇ�͂Ƃ肯���{�^�������҂��Ƃ���
//			// ���σ��[�_��WAON�̎�
//// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
////				Ope2_ErrChrCyclicDispStart(2000, msg_idx);	// ���[�j���O�T�C�N���b�N�\��(2�b�Ԋu)
//				Ope2_ErrChrCyclicDispStart(2000, msg_idx, 0);	// ���[�j���O�T�C�N���b�N�\��(2�b�Ԋu)
//// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
//				LagTim500ms(LAG500_EC_AUTO_CANCEL_TIMER, (short)((30 * 2) + 1), ec_auto_cancel_timeout);	// �A���[�������̐��Z�����L�����Z���^�C�}�J�n(500ms)
//// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
//			} else if (Ec_Settlement_Res.brand_no == BRANDNO_ID &&
//					   ECCTL.ec_Disp_No == 90){
			if (Ec_Settlement_Res.brand_no == BRANDNO_ID &&
				ECCTL.ec_Disp_No == 90){
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
			// ���σ��[�_��iD�̎�
				lcd_wmsg_dsp_elec( 1, ERR_CHR[61], ERR_CHR[59], timer, 0, COLOR_RED, LCD_BLINK_OFF );
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
			} else {
				lcd_wmsg_dsp_elec( 1, ERR_CHR[20], msg, timer, 0, COLOR_RED, LCD_BLINK_OFF );
			}
		} else {
			lcd_wmsg_dsp_elec( 1, ERR_CHR[20], msg, timer, 0, COLOR_RED, LCD_BLINK_OFF );
		}
// MH321800(E) G.So IC�N���W�b�g�Ή�
		
		Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 1;			// �װ�ү���ޕ\���p�׸޾��
		// ���������^�C���A�E�g��́A���i�I���f�[�^���M�ۃt���O�𗎂Ƃ��B
		Suica_Rec.Data.BIT.ADJUSTOR_START = 0;			// ���i�I���f�[�^���M�ۃt���O�N���A
	}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//	if(!SUICA_CM_BV_RD_STOP && OPECTL.op_faz != 3 )
//		Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ��	
	if(!SUICA_CM_BV_RD_STOP && OPECTL.op_faz != 3 ) {
		if(isEC_USE()) {
			Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ��	
		} else {
			Lagtim( OPETCBNO, 10, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ��	
		}
	}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
}

//[]----------------------------------------------------------------------[]
///	@brief			�����薢�����̔��菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		�F0�F�������łȂ�<br>
///							  1�F����������orSuica��ѱ�Č�̎g�p�s�ݒ�
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/08/22<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
uchar Miryo_Exec_check( void )
{
	uchar ret=0;

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//	if( Suica_Rec.Data.BIT.CTRL_MIRYO ||
//		(Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP) ||
//		Edy_Rec.edy_status.BIT.CTRL_MIRYO )
	// �����c���Ɖ�H
	if( Suica_Rec.Data.BIT.CTRL_MIRYO && !Suica_Rec.Data.BIT.MIRYO_CONFIRM ||
		(Suica_Rec.Data.BIT.MIRYO_TIMEOUT && SUICA_CM_BV_RD_STOP))
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

		ret = 1;
		
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			�������z�Z�o����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0:No balance 1:In the balance
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/09/11<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
char in_credit_check( void )
{
	char	i;
	long	crdt, wk;

	crdt = 0l;
	for( i = 0; i < 4; i++ ){
		wk = ( (long)ryo_buf.in_coin[i] );
		crdt += ( (long)( wk * coin_vl[i] ) );
	}
	crdt += ( (long)( ryo_buf.in_coin[4] * 1000l ) );
	
	i = 0;
	if( crdt < ryo_buf.dsp_ryo )
		i = 1;

	return( i );	
}

//[]----------------------------------------------------------------------[]
///	@brief			���]�\������
//[]----------------------------------------------------------------------[]
///	@param[in]		type �F0�F�ʏ�\�� 1�F���]�\��
///	@param[in]		msg1 �F�\��������(7�s��)	
///	@param[in]		msg2 �F�\��������(8�s��)
///	@param[in]		timer�F0�F������ϰ�g�p���Ȃ� 1�F������ϰ�g�p����(6�b)
///	@param[in]		ann  �F0�F�����ē����Ȃ� 1�F�����ē��g�p����(�g�p�s��)
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 08/09/19<br>
///					Update
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void lcd_wmsg_dsp_elec( char type, const uchar *msg1, const uchar *msg2, uchar timer, uchar ann, ushort color, ushort blink )
{
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�폜
// �߯�߱��ߕ\���v�� �� LCD�ɑ��M����LCD�ŕ\������(lcdbm_notice_dsp)
//	if( DspSts == LCD_WMSG_ON )								//�G���[���b�Z�[�W�\�����Ȃ�
//		LagCan500ms(LAG500_ERROR_DISP_DELAY);				//�^�C�}�[�L�����Z��
//
//	Lcd_WmsgDisp_ON2( (ushort)type, color, blink, msg1 );
//	Lcd_WmsgDisp_ON( (ushort)type, color, blink, msg2 );
//	
//	if( timer )
//		LagTim500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER, 13, op_SuicaFusokuOff );	// 6sec��ɏ���
//	
//	if( ann ){
//	}	
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�폜
}

//[]----------------------------------------------------------------------[]
///	@brief			�����^�C���A�E�g��̐��Z���A����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/01/14<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void miryo_timeout_after_proc( void )
{
	if( ryo_buf.zankin ){						// ����������Ɏc��������ꍇ��
		if( (( !pas_kep && !RD_Credit_kep ) || 
			(( RED_stat[2] & TICKET_MOUTH ) ==  TICKET_MOUTH ) ))	/* �����}�����ɂ��� */
			read_sht_opn();					/* ���Cذ�ް���������� */
		cn_stat( 3, 2 );					// ���� & COIN�ē�����
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�����^�C���A�E�g���mifare���A����
//[]----------------------------------------------------------------------[]
///	@param[in]		type: 0�FSuica / 1�FEdy
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/01/14<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void miryo_timeout_after_mif( uchar type )
{
	uchar ret = 0;
	
	if( MIFARE_CARD_DoesUse &&					// Mifare���L��
		OPECTL.op_faz != 8 ){					// �d�q�}�̒�~�v�����łȂ�
		if( type ){								// Edy����̃R�[��
			ret = 1;							// Mifare�̍Ċ�����OK
		}else{
			if( !SUICA_CM_BV_RD_STOP )			// ������ASuica�ECM�EBV�ERD���g�p�s�ݒ�ȊO
				ret = 1;						// Mifare�̍Ċ�����OK
		}
		if( ret )
			op_MifareStart();					// Mifare�Ċ�����
		
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�G���[�\���E�󎚂̕�����ϊ�����
//[]----------------------------------------------------------------------[]
///	@param[in]		ItemNum: �G���[������v�f�ԍ�
///	@return			wkErrNum�F�ϊ���̕�����v�f�ԍ�
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/01/14<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
ushort SuicaErrCharChange( ushort ItemNum )
{
	ushort wkErrNum = 0;
	ushort media_Type = 0;
	
	wkErrNum = ItemNum - 150;							// 0=�ʐM�s�ǁA1=�ُ�A2=��

	media_Type = Ope_Disp_Media_Getsub(0);				// ��ʕ\���p ���Z�}�̎�� �擾
	if( !Is_SUICA_STYLE_OLD ){							// �V�œ���ݒ�
		media_Type = OPE_DISP_MEDIA_TYPE_eMONEY;		// "�d�q�}�l�[���[�_�[..." �ɌŒ�
	}
	switch(media_Type) {
	case OPE_DISP_MEDIA_TYPE_SUICA:
		wkErrNum += 150;
		break;											// "Suica���[�_�[..."�\��
	case OPE_DISP_MEDIA_TYPE_PASMO:
		wkErrNum += 168;								// "PASMO���[�_�[..."�\��
		break;
	case OPE_DISP_MEDIA_TYPE_ICOCA:
		wkErrNum += 171;								// "ICOCA���[�_�[..."�\��
		break;
	case OPE_DISP_MEDIA_TYPE_ICCARD:
		wkErrNum += 174;								// "IC�J�[�h���[�_�[..."�\��
		break;
	case OPE_DISP_MEDIA_TYPE_eMONEY:
		wkErrNum += 177;								// "�d�q�}�l�[���[�_�[..."�\��
		break;
	default:
		break;
	}
	return wkErrNum;	
}

//[]----------------------------------------------------------------------[]
///	@brief			����f�[�^��Z�߂�
//[]----------------------------------------------------------------------[]
///	@param[in]		type: 0�F��t�s�� / 1�F��t�� / 0x80�F����I��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/02/17<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void Suica_Ctrl( ushort req, uchar type )
{
	uchar ctrl_work = type;

// MH321800(S) G.So IC�N���W�b�g�Ή�
	if (isEC_USE() != 0) {
		Ec_Pri_Data_Snd(req, type);
		return;
	}
// MH321800(E) G.So IC�N���W�b�g�Ή�
	
	switch( req ){
		case S_CNTL_DATA:
			Suica_Data_Snd( S_CNTL_DATA,&ctrl_work );			// �����ް����M
			break;
		case S_SELECT_DATA:
			if(e_incnt <= 0) {
				w_settlement = ryo_buf.dsp_ryo;
				if( w_settlement > PAY_RYO_MAX){
					w_settlement = 	PAY_RYO_MAX;
				}
				Suica_Data_Snd( S_SELECT_DATA, &w_settlement);		// ���ԗ��������i�I���ް��Ƃ��đ��M����
			}
			break;
	}	
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica�̏�Ԃ��ω��������Ƃ�Ope�ɒʒm����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/02/17<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void Suica_Status_Chg( void )
{
	ushort req = 0;
	
	if( Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL ){			// Suica���L�����Ō�ɑ��M�����̂���t���̏ꍇ
		req = SUICA_ENABLE_EVT;												// �I�y�Ɏ�t�̕ω���ʒm
	}else if( !Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ){	// Suica���������Ō�ɑ��M�����̂���t�s�̏ꍇ
		req = SUICA_DISABLE_EVT;											// �I�y�Ɏ�t�s�̕ω���ʒm
	}
	
	if( req )
		queset( OPETCBNO, req, 0, NULL );									// �I�y�ɃC�x���g��ʒm
	
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica�n�̃J�[�h���g�p���ꂽ���ǂ����̔�����s��
//[]----------------------------------------------------------------------[]
///	@param[in]		kind�F�g�p�J�[�h���
///	@return			ret �F1�F�g�p�ς� 0�F���g�p
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/03/18<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar SuicaUseKindCheck( uchar kind )
{
	uchar ret = 0;
	
	if( (kind == SUICA_USED) ||	// Suica�g�p��
	    (kind == PASMO_USED) ||	// PASMO�g�p��
	    (kind == ICOCA_USED) ||	// ICOCA�g�p��
	    (kind == ICCARD_USED)){	// IC-Card�g�p��
	    ret = 1;
	}
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			�d�q�}�̒�~���̃X�e�[�^�X���擾����
//[]----------------------------------------------------------------------[]
///	@param[in]		type�F0�����Z������ 1�����Z���~��
///	@return			ret �F0BIT��Suica��~�ς�<br>
///					    �F1BIT��Edy��~�ς�<br>
///					    �F2BIT�����ү���~�ς�<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/04/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar StopStatusGet( uchar type )
{
	uchar ret = 0;
	
	if( !type ){
		ret = CCT_Cansel_Status.BIT.SUICA_END;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		ret |= (uchar)(CCT_Cansel_Status.BIT.EDY_END << 1);
		ret |= (uchar)(1 << 1);
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		ret |= (uchar)(CCT_Cansel_Status.BIT.CMN_END << 2);
	}else{
		ret = cansel_status.BIT.SUICA_END;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		ret |= (uchar)(cansel_status.BIT.EDY_END << 1);
		ret |= (uchar)(1 << 1);
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		ret |= (uchar)(cansel_status.BIT.CMN_END << 2);
	}
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			�����W�v�p���ޯ����ߋy��ؽı�̏��������{����
//[]----------------------------------------------------------------------[]
///	@param[in]		type�F0=�o�b�N�A�b�v 1�����X�g�A<br>
///	@return			ret �F0���擾OK		 1���擾���s�i�ݒ�Ȃ��j<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 09/04/14<br>
///					Update
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar DailyAggregateDataBKorRES( uchar type )
{
	uchar ret = 0;
	
// MH321800(S) G.So IC�N���W�b�g�Ή�
	if (isEC_USE() != 0) {
		if( !type )
			memcpy( &bk_syu_dat_ec, &Syuukei_sp.ec_inf.now, sizeof( bk_syu_dat_ec ));
		else
			memcpy( &Syuukei_sp.ec_inf.now, &bk_syu_dat_ec, sizeof( bk_syu_dat_ec ));
	} else
// MH321800(E) G.So IC�N���W�b�g�Ή�
	if( prm_get(COM_PRM, S_PAY, 24, 1, 3) == 1 ){
		if( !type )
			memcpy( &bk_syu_dat, &Syuukei_sp.sca_inf.now, sizeof( bk_syu_dat ));
		else	
			memcpy( &Syuukei_sp.sca_inf.now, &bk_syu_dat, sizeof( bk_syu_dat ));
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	}else if( prm_get(COM_PRM, S_PAY, 24, 1, 4) == 1 ){
//		if( !type)
//			memcpy( &bk_syu_dat, &Syuukei_sp.edy_inf.now, sizeof( bk_syu_dat ));
//		else	
//			memcpy( &Syuukei_sp.edy_inf.now, &bk_syu_dat, sizeof( bk_syu_dat ));
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	}else{
		ret = 1;
	}
		
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			���ϊz�Ə��i�I���f�[�^�̋��z����v���Ȃ��ꍇ�̃A���[���\��
//[]----------------------------------------------------------------------[]
///	@param			None<br>
///	@return			None<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/06/23<br>
///					Update
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void	PayDataErrDisp( void )
{
// MH321800(S) G.So IC�N���W�b�g�Ή�
	if (isEC_USE() != 0) {
		Settlement_Res.settlement_data = Ec_Settlement_Res.settlement_data;
	}
// MH321800(E) G.So IC�N���W�b�g�Ή�

// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//	if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR &&				// ���ϊz�Ə��i�I���f�[�^�ō������鐸�Z���s�Ȃ�ꂽ
//		Settlement_Res.settlement_data ){ 							// ����̐��Z�Ō��ό��ʃf�[�^����M
	if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR ){				// ���ϊz(0�~�܂�)�Ə��i�I���f�[�^�ō������鐸�Z���s�Ȃ�ꂽ
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�

		switch( OPECTL.Ope_mod ){
			case 2:
				Lcd_WmsgDisp_ON2( 1, COLOR_RED,  LCD_BLINK_OFF, ERR_CHR[47] );
				Lcd_WmsgDisp_ON(  1, COLOR_RED,  LCD_BLINK_OFF, ERR_CHR[22] );
				nyukin_delay_check( nyukin_delay, delay_count);		// �ۗ����̲���Ă�����΍�post����
				Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 1;				// �װ�ү���ޕ\���p�׸޾��

				if( OPECTL.op_faz == 8 && CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ ){
					Op_StopModuleWait_sub( 0 );						// Suica��~�ςݏ���
				}else{
					read_sht_cls();									// ���Cذ�ް������۰��
				}
				cn_stat( 2, 2 );									// �����s��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//				Lagcan( OPETCBNO, 10 );								// ��ʐؑ֗p��ϰؾ��(Timer10)
				if(isEC_USE()) {
					Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );		// ���p�\�}�̻��د��\���p��ϰSTOP
				} else {
					Lagcan( OPETCBNO, 10 );							// ��ʐؑ֗p��ϰؾ��(Timer10)
				}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
				
				if( dsp_fusoku ){											//  �s���A���[���\�����H
			    	LagCan500ms( LAG500_SUICA_ZANDAKA_RESET_TIMER );		//  �s���A���[��������ϰ���~�߂�
			    	dsp_fusoku = 0;											//  �s���t���OOFF
			    }
				break;
			case 3:
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if (Suica_Rec.Data.BIT.CTRL_CARD) {
				if (isEC_CARD_INSERT()) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					// �J�[�h����͔������҂���D�悷��
					break;
				}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
				Lcd_WmsgDisp_ON(  1, COLOR_RED,  LCD_BLINK_OFF, ERR_CHR[47] );
				break;
			default:
				break;
			
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���ϊz�Ə��i�I���f�[�^�̋��z����v���Ȃ��ꍇ�̃A���[������
//[]----------------------------------------------------------------------[]
///	@param			None<br>
///	@return			None<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/06/23<br>
///					Update
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void	PayDataErrDispClr( void )
{
	if( Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR &&				// ���ψُ픭����
		(DspSts == LCD_WMSG_ON || DspSts2 == LCD_WMSG_ON )){		// �A���[���\����
		Lcd_WmsgDisp_OFF2();
		Lcd_WmsgDisp_OFF();
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Suica_Ctrl������R�[�������Suica�֘A�̃p�����[�^�擾����<br>
///					���������΍�
//[]----------------------------------------------------------------------[]
///	@param			None<br>
///	@return			0:Suica�L�� 1�FSuica����<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/07/04<br>
///					Update
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
short	SuicaParamGet( void )
{
	
	short	ret = 0;
	
// MH321800(S) G.So IC�N���W�b�g�Ή�
//	ret = ( prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1 );
	ret = (isEMoneyReader() != 0)? 0 : 1;
// MH321800(E) G.So IC�N���W�b�g�Ή�
	
	return ret;
}

// MH322914 (s) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
//[]----------------------------------------------------------------------[]
///	@brief			�W���[�i���g�p��ɂ����Suica���g�p�ł��邩�ǂ����𔻒肷��<br>
//[]----------------------------------------------------------------------[]
///	@param			None<br>
///	@return			None<br>
//[]----------------------------------------------------------------------[]
///	@author			T.Namioka
///	@date			Create	: 11/07/04<br>
///					Update
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void	PrinterCheckForSuica( void )
{
	
	if( Ope_isJPrinterReady() != 1 || IsErrorOccuerd((char)ERRMDL_PRINTER, (char)ERR_PRNT_J_HEADHEET)){		// �W���[�i���g�p�s��
		Suica_Rec.Data.BIT.PRI_NG = 1;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		Edy_Rec.edy_status.BIT.PRI_NG = 1;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	}
}
// MH322914 (e) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)

